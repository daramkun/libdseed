#define USE_WASAPI_NATIVE_OBJECT
#include <dseed.h>

#if PLATFORM_MICROSOFT
#	include <Audioclient.h>
#	include "../libs/WAVHelper.hxx"

#	define REFTIMES_PER_SEC									10000000
#	define REFTIMES_PER_MILLISEC							10000

class __wasapi_audiorequester : public dseed::audio::audiorequester
{
public:
	__wasapi_audiorequester (IAudioClient* audioClient, IAudioCaptureClient* audioCaptureClient, HANDLE hWakeUp, WAVEFORMATEX* wf)
		: _refCount (1), audioClient (audioClient), audioCaptureClient (audioCaptureClient), hWakeUp (hWakeUp), wf (wf)
		, silentNull (true)
	{
		audioClient->GetBufferSize (&bufferFrameSize);
		byteArray.resize (bufferFrameSize * wf->nAvgBytesPerSec);
	}
	~__wasapi_audiorequester ()
	{
		CoTaskMemFree (wf);
		CloseHandle (hWakeUp);
	}

public:
	virtual int32_t retain () override { return ++_refCount; }
	virtual int32_t release () override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual dseed::error_t native_object (void** nativeObject) override
	{
		if (nativeObject == nullptr)
			return dseed::error_invalid_args;

		auto no = reinterpret_cast<dseed::audio::wasapi_audiorequester_nativeobject*>(nativeObject);
		audioClient.As<IAudioClient> (&no->audioClient);
		audioCaptureClient.As<IAudioCaptureClient> (&no->audioCaptureClient);

		return dseed::error_good;
	}

public:
	virtual dseed::error_t format (dseed::media::audioformat* format) override
	{
		WAVEFORMATEX* wf;
		if (FAILED (audioClient->GetMixFormat (&wf)))
			return dseed::error_fail;

		convert_from_waveformatex (wf, format);

		CoTaskMemFree (wf);

		return dseed::error_good;
	}

public:
	virtual dseed::error_t start () override
	{
		if (FAILED (audioClient->Start ()))
			return dseed::error_fail;
		return dseed::error_good;
	}
	virtual dseed::error_t stop () override
	{
		if (FAILED (audioClient->Stop ()))
			return dseed::error_fail;
		return dseed::error_good;
	}

public:
	virtual bool silent_null () override { return silentNull; }
	virtual void set_silent_null (bool sn) override { silentNull = sn; }

public:
	virtual dseed::error_t request (void* buffer, size_t bufferMaxSize, size_t* dataSize) override
	{
		UINT32 packetLength = 0;
		DWORD totalLength = 0;

		HRESULT hr;
		hr = audioCaptureClient->GetNextPacketSize (&packetLength);

		if (byteArray.size () < bufferMaxSize)
			byteArray.resize (bufferMaxSize);

		void* retBuffer = byteArray.data ();
		while (SUCCEEDED (hr) && packetLength > 0)
		{
			BYTE* pData;
			UINT32 nNumFramesToRead;
			DWORD dwFlags;

			if (FAILED (hr = audioCaptureClient->GetBuffer (&pData, &nNumFramesToRead, &dwFlags, NULL, NULL)))
			{
				*dataSize = -1;
				return dseed::error_fail;
			}

			if (AUDCLNT_BUFFERFLAGS_SILENT == dwFlags)
			{
				fprintf (stderr, "Silent(nNumFramesToRead: %d).\n", nNumFramesToRead);
				if (silentNull)
				{
					audioCaptureClient->ReleaseBuffer (nNumFramesToRead);
					*dataSize = -1;
					return dseed::error_good;
				}
			}
			else if (AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY == dwFlags)
			{
				fprintf (stderr, "Discontinuity(nNumFramesToRead: %d).\n", nNumFramesToRead);
			}
			else if (AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR == dwFlags)
			{
				fprintf (stderr, "Timestamp Error(nNumFramesToRead: %d).\n", nNumFramesToRead);
			}
			else
			{
				fprintf (stderr, "Maybe Discontinuity(nNumFramesToRead: %d).\n", nNumFramesToRead);
			}

			if (0 == nNumFramesToRead)
			{
				//*bufferLength = 0;
				//pCaptureClient->ReleaseBuffer ( nNumFramesToRead );
				//return nullptr;
			}

			LONG lBytesToWrite = nNumFramesToRead * wf->nBlockAlign;
			memcpy (byteArray.data () + totalLength, pData, lBytesToWrite);
			totalLength += lBytesToWrite;

			if (FAILED (hr = audioCaptureClient->ReleaseBuffer (nNumFramesToRead)))
				return dseed::error_fail;

			hr = audioCaptureClient->GetNextPacketSize (&packetLength);
		}

		DWORD dwWaitResult = WaitForMultipleObjects (1, &hWakeUp, FALSE, INFINITE);

		memcpy (buffer, byteArray.data (), dseed::minimum (byteArray.size (), bufferMaxSize));
		*dataSize = totalLength;

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;

	Microsoft::WRL::ComPtr<IAudioClient> audioClient;
	Microsoft::WRL::ComPtr<IAudioCaptureClient> audioCaptureClient;

	bool silentNull;
	HANDLE hWakeUp;

	WAVEFORMATEX* wf;
	std::vector<uint8_t> byteArray;
	UINT bufferFrameSize;
};

#endif

dseed::error_t dseed::audio::create_wasapi_audiorequester (dseed::audio::audioadapter* adapter, dseed::audio::audiorequester** requester) noexcept
{
	if (requester == nullptr)
		return dseed::error_invalid_args;

#if PLATFORM_MICROSOFT
	Microsoft::WRL::ComPtr<IMMDevice> mmDevice;

	if (adapter != nullptr)
	{
		dseed::audio::wasapi_audioadapter_nativeobject adapterNativeObject;
		adapter->native_object ((void**)&adapterNativeObject);
		mmDevice = adapterNativeObject.mmDevice;
	}
	else
	{
		Microsoft::WRL::ComPtr<IMMDeviceEnumerator> deviceEnumerator;
		if (FAILED (CoCreateInstance (__uuidof (MMDeviceEnumerator), nullptr, CLSCTX_ALL,
			__uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator)))
			return dseed::error_fail;

		if(FAILED(deviceEnumerator->GetDefaultAudioEndpoint (eCapture, eConsole, &mmDevice)))
			return dseed::error_fail;
	}

	Microsoft::WRL::ComPtr<IAudioClient> audioClient;
	if (FAILED (mmDevice->Activate (__uuidof (IAudioClient), CLSCTX_ALL, nullptr, (void**)&audioClient)))
		return dseed::error_fail;

	WAVEFORMATEX* wf;
	audioClient->GetMixFormat (&wf);
	switch (wf->wFormatTag)
	{
		case WAVE_FORMAT_IEEE_FLOAT:
			wf->wFormatTag = WAVE_FORMAT_PCM;
			wf->wBitsPerSample = 16;
			wf->nBlockAlign = wf->nChannels * wf->wBitsPerSample / 8;
			wf->nAvgBytesPerSec = wf->nBlockAlign * wf->nSamplesPerSec;
			break;

		case WAVE_FORMAT_EXTENSIBLE:
			{
				PWAVEFORMATEXTENSIBLE pEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(wf);
				if (IsEqualGUID (KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat))
				{
					pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
					pEx->Samples.wValidBitsPerSample = 16;
					wf->wBitsPerSample = 16;
					wf->nBlockAlign = wf->nChannels * wf->wBitsPerSample / 8;
					wf->nAvgBytesPerSec = wf->nBlockAlign * wf->nSamplesPerSec;
				}
			}
			break;
	}

	REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
	if (FAILED (audioClient->Initialize (AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, wf, NULL)))
	{
		CoTaskMemFree (wf);
		return dseed::error_fail;
	}

	REFERENCE_TIME hnsDefaultDevicePeriod;
	if (FAILED (audioClient->GetDevicePeriod (&hnsDefaultDevicePeriod, NULL)))
	{
		CoTaskMemFree (wf);
		return dseed::error_fail;
	}

	Microsoft::WRL::ComPtr<IAudioCaptureClient> audioCaptureClient;
	if (FAILED (audioClient->GetService (__uuidof(IAudioCaptureClient), (void**)&audioCaptureClient)))
	{
		CoTaskMemFree (wf);
		return dseed::error_fail;
	}

	HANDLE hWakeUp = CreateWaitableTimer (nullptr, FALSE, nullptr);
	if (hWakeUp == INVALID_HANDLE_VALUE || hWakeUp == 0)
	{
		CoTaskMemFree (wf);
		return dseed::error_fail;
	}

	LARGE_INTEGER liFirstFire;
	liFirstFire.QuadPart = -hnsDefaultDevicePeriod / 2;
	LONG lTimeBetweenFires = (LONG)hnsDefaultDevicePeriod / 2 / (10 * 1000);
	BOOL bOK = SetWaitableTimer (
		hWakeUp,
		&liFirstFire,
		lTimeBetweenFires,
		NULL, NULL, FALSE
	);

	*requester = new __wasapi_audiorequester (audioClient.Get (), audioCaptureClient.Get (), hWakeUp, wf);
	if (*requester == nullptr)
	{
		CoTaskMemFree (wf);
		CloseHandle (hWakeUp);
		return dseed::error_out_of_memory;
	}

	return dseed::error_good;
#else
	return dseed::error_not_support;
#endif
}
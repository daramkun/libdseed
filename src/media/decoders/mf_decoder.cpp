#include <dseed.h>

#if PLATFORM_MICROSOFT
#	include <wrl.h>
#	include <mfapi.h>
#	include <mfidl.h>
#	include <mfreadwrite.h>
#	include <mftransform.h>
#	include <cassert>

#	include "../../libs/IStreamHelper.hxx"
#	include "../../libs/MicrosoftHelper.hxx"

class MFSupportChecker
{
public:
	HRESULT hr;
	MFSupportChecker () { hr = MFStartup (MF_VERSION); }
	~MFSupportChecker () { MFShutdown (); }
	bool IsSupport () { return SUCCEEDED (hr); }
} g_mfsupportchecker;

class __mf_sample : public dseed::media::media_sample
{
public:
	__mf_sample (dseed::media::sampletype sampletype, IMFSample* sample)
		: _refCount (1), _sample (sample), _sampletype (sampletype)
	{
		HRESULT hr = SUCCEEDED (_sample->ConvertToContiguousBuffer (&_buffer));
		assert (hr);
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
	virtual dseed::media::sampletype type () override { return _sampletype; }

public:
	virtual dseed::timespan position () override
	{
		dseed::timespan ret;
		_sample->GetSampleTime ((LONGLONG*)&ret);
		return ret;
	}
	virtual dseed::timespan duration () override
	{
		dseed::timespan ret;
		_sample->GetSampleDuration ((LONGLONG*)&ret);
		return ret;
	}

public:
	virtual dseed::error_t copy_sample (void* buffer) override
	{

		BYTE* mfBufferInternal;
		DWORD currentLength;
		if (FAILED (_buffer->Lock (&mfBufferInternal, nullptr, &currentLength)))
			return dseed::error_fail;

		memcpy (buffer, mfBufferInternal, sample_size ());

		_buffer->Unlock ();

		return dseed::error_good;
	}

	virtual size_t sample_size () override
	{
		DWORD len;
		if (FAILED (_buffer->GetCurrentLength (&len)))
			return 0;
		return len;
	}

private:
	std::atomic<int32_t> _refCount;
	Microsoft::WRL::ComPtr<IMFSample> _sample;
	Microsoft::WRL::ComPtr<IMFMediaBuffer> _buffer;
	dseed::media::sampletype _sampletype;
};

class __mf_decoder : public dseed::media::media_decoder
{
public:
	__mf_decoder (IMFSourceReader* reader, const std::vector<dseed::media::sampletype>& sampleTypes)
		: _refCount (1), _reader (reader), _sampleTypes (sampleTypes)
	{

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
	virtual dseed::media::sampletype type (size_t streamIndex) override
	{
		if (streamIndex >= _sampleTypes.size ())
			return dseed::media::sampletype_out_of_range;
		return _sampleTypes[streamIndex];
	}
	virtual dseed::error_t format (size_t streamIndex, void* buffer, size_t bufferSize) override
	{
		if (streamIndex >= _sampleTypes.size ())
			return dseed::error_out_of_range;
		if (buffer == nullptr)
			return dseed::error_invalid_args;

		switch (_sampleTypes[streamIndex] & 0xffff)
		{
			case dseed::media::sampletype_audio:
				if (bufferSize != sizeof (dseed::media::audioformat))
					return dseed::error_invalid_args;
				else
				{
					Microsoft::WRL::ComPtr<IMFMediaType> type;
					if (FAILED (_reader->GetCurrentMediaType ((DWORD)streamIndex, &type)))
						return dseed::error_fail;

					GUID audioType;
					type->GetGUID (MF_MT_SUBTYPE, &audioType);
					if (!(audioType == MFAudioFormat_PCM || audioType == MFAudioFormat_Float))
						return dseed::error_not_support;

					UINT32 channels, bitsPerSample, sampleRate;
					type->GetUINT32 (MF_MT_AUDIO_NUM_CHANNELS, &channels);
					type->GetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample);
					type->GetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate);

					*((dseed::media::audioformat*)buffer) = dseed::media::audioformat (channels, bitsPerSample, sampleRate,
						((audioType == MFAudioFormat_PCM)
							? dseed::media::pulseformat::pcm
							: dseed::media::pulseformat::ieee_float));
				}

				break;

			case dseed::media::sampletype_image:
				if (bufferSize != sizeof (dseed::media::imageformat))
					return dseed::error_invalid_args;
				else
				{
					Microsoft::WRL::ComPtr<IMFMediaType> type;
					if (FAILED (_reader->GetCurrentMediaType ((DWORD)streamIndex, &type)))
						return dseed::error_fail;

					dseed::media::imageformat* af = (dseed::media::imageformat*)buffer;
					GUID imageType;
					type->GetGUID (MF_MT_SUBTYPE, &imageType);

					dseed::color::pixelformat pf;
					if (imageType == MFVideoFormat_ARGB32 || imageType == MFVideoFormat_RGB32)
						pf = dseed::color::pixelformat::bgra8;
					else if (imageType == MFVideoFormat_RGB24)
						pf = dseed::color::pixelformat::bgr8;
					else if (imageType == MFVideoFormat_RGB565)
						pf = dseed::color::pixelformat::bgr565;
					else if (imageType == MFVideoFormat_AYUV)
						pf = dseed::color::pixelformat::yuva8;
					else if (imageType == MFVideoFormat_NV12)
						pf = dseed::color::pixelformat::nv12;
					else if (imageType == MFVideoFormat_YVYU)
						pf = dseed::color::pixelformat::yuyv8;
					else return dseed::error_not_support;

					UINT32 width, height;
					MFGetAttributeSize (type.Get (), MF_MT_FRAME_SIZE, &width, &height);
					UINT32 num, denom;
					MFGetAttributeRatio (type.Get (), MF_MT_FRAME_RATE, &num, &denom);

					*((dseed::media::imageformat*)buffer) = dseed::media::imageformat (dseed::size2i (width, height),
						pf, dseed::fraction (num, denom));
				}

				break;

			default:
				return dseed::error_not_support;
		}

		return dseed::error_good;
	}

public:
	virtual dseed::timespan duration () override
	{
		PROPVARIANT var;
		if (FAILED (_reader->GetPresentationAttribute (MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var)))
			return dseed::error_fail;
		return var.hVal.QuadPart;
	}

public:
	virtual dseed::timespan reading_position () override
	{
		return _currentTime;
	}
	virtual dseed::error_t set_reading_position (dseed::timespan pos) override
	{
		if (_reader == nullptr)
			return dseed::error_invalid_op;

		PROPVARIANT p;
		p.vt = VT_I8;
		p.hVal.QuadPart = pos.ticks ();
		if (FAILED (_reader->SetCurrentPosition (GUID_NULL, p)))
			return dseed::error_fail;
		_currentTime = pos;
		return dseed::error_t ();
	}

public:
	virtual dseed::error_t decode (size_t streamIndex, dseed::media::media_sample** sample) override
	{
		if (_reader == nullptr)
			return dseed::error_invalid_op;
		if (streamIndex == 0xffffffff)
			streamIndex = MF_SOURCE_READER_ALL_STREAMS;

		Microsoft::WRL::ComPtr<IMFSample> mfSample;
		DWORD flags;
		DWORD actualStream;
		if (FAILED (_reader->ReadSample ((DWORD)streamIndex, 0, &actualStream, &flags, nullptr, &mfSample)))
			return dseed::error_fail;

		if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
			return dseed::error_end_of_file;

		if (mfSample == nullptr || FAILED (mfSample->GetSampleTime ((LONGLONG*)&_currentTime)))
			return dseed::error_fail;

		*sample = new __mf_sample (_sampleTypes[actualStream], mfSample.Get ());
		if (*sample == nullptr)
			return dseed::error_out_of_memory;

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;

	Microsoft::WRL::ComPtr<IMFSourceReader> _reader;

	dseed::timespan _currentTime;

	std::vector<dseed::media::sampletype> _sampleTypes;
};
#endif

dseed::error_t dseed::media::create_media_foundation_media_decoder (dseed::io::stream* stream, dseed::media::media_decoder** decoder)
{
#if PLATFORM_WINDOWS
	if (!g_mfsupportchecker.IsSupport ())
		return dseed::error_not_support;
	if (stream == nullptr || decoder == nullptr)
		return dseed::error_invalid_args;

	Microsoft::WRL::ComPtr<IStream> istream;
	ImpledIStream::Create (stream, &istream);
	Microsoft::WRL::ComPtr<IMFByteStream> mfStream;
	if (FAILED (MFCreateMFByteStreamOnStreamEx (istream.Get (), &mfStream)))
		return dseed::error_fail;

	Microsoft::WRL::ComPtr<IMFAttributes> attributes;
	if (FAILED (MFCreateAttributes (&attributes, 0)))
		return dseed::error_fail;
	attributes->SetUINT32 (MF_LOW_LATENCY, TRUE);

	Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader;
	if (FAILED (MFCreateSourceReaderFromByteStream (mfStream.Get (), attributes.Get (), &sourceReader)))
		return dseed::error_fail;

	Microsoft::WRL::ComPtr<IMFMediaType> readingAudioMediaType;
	if (FAILED (MFCreateMediaType (&readingAudioMediaType)))
		return dseed::error_fail;
	if (FAILED (readingAudioMediaType->SetGUID (MF_MT_MAJOR_TYPE, MFMediaType_Audio)))
		return dseed::error_fail;
	if (FAILED (readingAudioMediaType->SetGUID (MF_MT_SUBTYPE, MFAudioFormat_PCM)))
		return dseed::error_fail;

	if (FAILED (sourceReader->SetCurrentMediaType (MF_SOURCE_READER_FIRST_AUDIO_STREAM,
		nullptr, readingAudioMediaType.Get ())))
		return dseed::error_fail;

	readingAudioMediaType.ReleaseAndGetAddressOf ();

	if (FAILED (sourceReader->GetCurrentMediaType (MF_SOURCE_READER_FIRST_AUDIO_STREAM, &readingAudioMediaType)))
		return dseed::error_fail;

	std::vector<dseed::media::sampletype> sampleTypes;
	for (int i = 0; i < 256; ++i)
	{
		Microsoft::WRL::ComPtr<IMFMediaType> type;
		if (FAILED (sourceReader->GetCurrentMediaType (i, &type)))
			break;

		sampleTypes.resize (i + 1);

		GUID sampleType;
		if (FAILED (type->GetGUID (MF_MT_MAJOR_TYPE, &sampleType)))
		{
			sampleTypes[i] = dseed::media::sampletype_unknown;
			continue;
		}

		if (sampleType == MFMediaType_Audio)
		{
			sampleTypes[i] = dseed::media::sampletype_audio;

			Microsoft::WRL::ComPtr<IMFMediaType> newType;
			MFCreateMediaType (&newType);

			newType->SetGUID (MF_MT_MAJOR_TYPE, MFMediaType_Audio);
			newType->SetGUID (MF_MT_SUBTYPE, MFAudioFormat_PCM);

			sourceReader->SetCurrentMediaType (i, nullptr, newType.Get ());
		}
		else if (sampleType == MFMediaType_Video)
		{
			sampleTypes[i] = dseed::media::sampletype_image;

			Microsoft::WRL::ComPtr<IMFMediaType> newType;
			MFCreateMediaType (&newType);

			newType->SetGUID (MF_MT_MAJOR_TYPE, MFMediaType_Video);
			newType->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_RGB24);
			newType->SetUINT32 (MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);

			sourceReader->SetCurrentMediaType (i, nullptr, newType.Get ());
		}
		else
			sampleTypes[i] = dseed::media::sampletype_unknown;
	}

	*decoder = new __mf_decoder (sourceReader.Get (), sampleTypes);
	if (*decoder == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
#else
	return dseed::error_not_support;
#endif
}
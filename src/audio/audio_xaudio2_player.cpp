#define USE_XAUDIO2_NATIVE_OBJECT
#define USE_WASAPI_NATIVE_OBJECT
#include <dseed.h>

#if PLATFORM_MICROSOFT
#	include <initguid.h>
#	include <functiondiscoverykeys.h>
DEFINE_PROPERTYKEY (PKEY_AudioEndpoint_Path, 0x9c119480, 0xddc2, 0x4954, 0xa1, 0x50, 0x5b, 0xd2, 0x40, 0xd4, 0x54, 0xad, 1);
#	include <wrl.h>
#	include <xaudio2.h>
#	include <x3daudio.h>

class __xaudio2_audioplaybuffer : public dseed::audio::audioplaybuffer, public IXAudio2VoiceCallback
{
public:
	__xaudio2_audioplaybuffer (IXAudio2SourceVoice* sourceVoice)
		: _refCount (1), sourceVoice (sourceVoice)
	{
		bufferEndEvent = CreateEvent (nullptr, FALSE, FALSE, nullptr);
	}
	~__xaudio2_audioplaybuffer ()
	{
		CloseHandle (bufferEndEvent);
		sourceVoice->DestroyVoice ();
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

		auto& no = *reinterpret_cast<dseed::audio::xaudio2_audioplaybuffer_nativeobject*>(nativeObject);
		no.sourceVoice = sourceVoice;

		return dseed::error_good;
	}

public:
	virtual dseed::audio::audiobufferstate state () noexcept override
	{
		XAUDIO2_VOICE_STATE voiceState;
		sourceVoice->GetState (&voiceState);


	}

public:
	virtual float volume () noexcept override
	{
		float vol = 0;
		sourceVoice->GetVolume (&vol);
		return vol;
	}

	virtual dseed::error_t set_volume (float vol) noexcept override
	{
		if (FAILED (sourceVoice->SetVolume (vol)))
			return dseed::error_fail;
		return dseed::error_good;
	}

public:
	virtual dseed::error_t play () noexcept override
	{
		if (FAILED (sourceVoice->Start ()))
			return dseed::error_fail;
		return dseed::error_good;
	}

	virtual dseed::error_t stop () noexcept override
	{
		if (FAILED (sourceVoice->Stop ()))
			return dseed::error_fail;
		return dseed::error_good;
	}

public:
	virtual dseed::error_t buffering (const void* buffer, size_t bufferSize) noexcept override
	{
		if ((buffer == nullptr && bufferSize != 0) || (buffer != nullptr && bufferSize == 0))
			return dseed::error_invalid_args;

		XAUDIO2_BUFFER xaudioBuffer = {};
		xaudioBuffer.pAudioData = (const BYTE*)buffer;
		xaudioBuffer.AudioBytes = bufferSize;
		xaudioBuffer.Flags = (buffer == nullptr) ? XAUDIO2_END_OF_STREAM : 0;
		if (FAILED (sourceVoice->SubmitSourceBuffer (&xaudioBuffer)))
			return dseed::error_fail;

		return dseed::error_good;
	}

	virtual dseed::error_t wait_to_buffer_end () noexcept override
	{
		XAUDIO2_VOICE_STATE voiceState;
		if (sourceVoice->GetState (&voiceState), voiceState.BuffersQueued > 0)
			WaitForSingleObjectEx (bufferEndEvent, INFINITE, TRUE);
		return dseed::error_good;
	}

public:
	virtual dseed::error_t set_listener (const dseed::audio::audiolistener* emitter) noexcept override
	{
		return dseed::error_not_impl;
	}

	virtual dseed::error_t set_emitter (const dseed::audio::audioemitter* emitter) noexcept override
	{
		return dseed::error_not_impl;
	}

public:
	virtual void __stdcall OnVoiceProcessingPassStart (UINT32 BytesRequired) override { }
	virtual void __stdcall OnVoiceProcessingPassEnd (void) override { }
	virtual void __stdcall OnStreamEnd (void) override { }
	virtual void __stdcall OnBufferStart (void* pBufferContext) override { }
	virtual void __stdcall OnBufferEnd (void* pBufferContext) override { SetEvent (bufferEndEvent); }
	virtual void __stdcall OnLoopEnd (void* pBufferContext) override { }
	virtual void __stdcall OnVoiceError (void* pBufferContext, HRESULT Error) override { }

private:
	std::atomic<int32_t> _refCount;
	IXAudio2SourceVoice* sourceVoice;
	HANDLE bufferEndEvent;
};

class __xaudio2_audioplayer : public dseed::audio::audioplayer
{
public:
	__xaudio2_audioplayer (IXAudio2* xaudio2, IXAudio2MasteringVoice* masteringVoice)
		: _refCount (1), xaudio2 (xaudio2), masteringVoice (masteringVoice)
	{

	}
	~__xaudio2_audioplayer ()
	{
		masteringVoice->DestroyVoice ();
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

		auto& no = *reinterpret_cast<dseed::audio::xaudio2_audioplayer_nativeobject*>(nativeObject);
		xaudio2.As<IXAudio2> (&no.xaudio2);
		no.masteringVoice = masteringVoice;

		return dseed::error_good;
	}

public:
	virtual float volume () noexcept override
	{
		float vol = 0;
		masteringVoice->GetVolume (&vol);
		return vol;
	}

	virtual dseed::error_t set_volume (float vol) noexcept override
	{
		if (FAILED (masteringVoice->SetVolume (vol)))
			return dseed::error_fail;
		return dseed::error_good;
	}

public:
	virtual dseed::error_t create_buffer (const dseed::media::audioformat* format, dseed::audio::audioplaybuffer** buffer) noexcept override
	{
		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	Microsoft::WRL::ComPtr<IXAudio2> xaudio2;
	IXAudio2MasteringVoice* masteringVoice;

	X3DAUDIO_HANDLE x3dAudioHandle;
};

dseed::error_t dseed::audio::create_xaudio2_audioplayer (dseed::audio::audioadapter* adapter, dseed::audio::audioplayer** player) noexcept
{
	if (player == nullptr)
		return dseed::error_invalid_args;
	if (adapter != nullptr && adapter->type () != dseed::audio::audioadapter_type::speaker)
		return dseed::error_invalid_args;

	WCHAR deviceId[128] = { 0, };
	if (adapter != nullptr)
	{
		dseed::audio::wasapi_audioadapter_nativeobject adapterNativeObject;
		adapter->native_object ((void**)&adapterNativeObject);

		Microsoft::WRL::ComPtr<IPropertyStore> propStore;
		if (FAILED (adapterNativeObject.mmDevice->OpenPropertyStore (STGM_READ, &propStore)))
			return dseed::error_fail;

		PROPVARIANT pv;
		if (FAILED (propStore->GetValue (PKEY_AudioEndpoint_Path, &pv)))
			return dseed::error_fail;

		if (pv.vt != VT_LPWSTR)
		{
			PropVariantClear (&pv);
			return dseed::error_fail;
		}

		wcscpy_s (deviceId, pv.bstrVal);

		PropVariantClear (&pv);
	}

	DWORD xaudioFlag = 0;
#if defined (_DEBUG) || !defined (_NDEBUG)
	xaudioFlag = XAUDIO2_DEBUG_ENGINE;
#endif

	Microsoft::WRL::ComPtr<IXAudio2> xaudio2;
	if (FAILED (XAudio2Create (&xaudio2, xaudioFlag)))
	{
		if (xaudioFlag & XAUDIO2_DEBUG_ENGINE)
		{
			xaudioFlag &= ~XAUDIO2_DEBUG_ENGINE;
			if (FAILED (XAudio2Create (&xaudio2, xaudioFlag)))
				return dseed::error_fail;
		}
		else return dseed::error_fail;
	}

	if (xaudioFlag & XAUDIO2_DEBUG_ENGINE)
	{
		XAUDIO2_DEBUG_CONFIGURATION debugConfig = {};
		debugConfig.TraceMask = XAUDIO2_LOG_WARNINGS;
		debugConfig.BreakMask = XAUDIO2_LOG_ERRORS;
		xaudio2->SetDebugConfiguration (&debugConfig);
	}

	IXAudio2MasteringVoice* masteringVoice;
	if (FAILED (xaudio2->CreateMasteringVoice (&masteringVoice, 0, 0, 0,
		deviceId[0] == '\0' ? nullptr : deviceId,
		nullptr, AudioCategory_GameEffects)))
		return dseed::error_fail;

	*player = new __xaudio2_audioplayer (xaudio2.Get (), masteringVoice);
	if (*player == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}

#endif
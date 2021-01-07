#define USE_XAUDIO2_NATIVE_OBJECT
#define USE_WASAPI_NATIVE_OBJECT
#include <dseed.h>

#if PLATFORM_MICROSOFT
#	include <initguid.h>
#	include <functiondiscoverykeys.h>
DEFINE_PROPERTYKEY(PKEY_AudioEndpoint_Path, 0x9c119480, 0xddc2, 0x4954, 0xa1, 0x50, 0x5b, 0xd2, 0x40, 0xd4, 0x54, 0xad, 1);
#	include <wrl.h>
#	include <xaudio2.h>
#	include <x3daudio.h>

#	include "../libs/WAVHelper.hxx"

class __xaudio2_backgroundaudio : public dseed::audio::backgroundaudio, public IXAudio2VoiceCallback
{
public:
	__xaudio2_backgroundaudio(IXAudio2SourceVoice* voice, dseed::media::audio_stream* stream)
		: _refCount(1), sourceVoice(voice), sourceStream(stream), isPlaying(false), isPaused(false), currentBuffer(0)
	{
		dseed::media::audioformat af;
		sourceStream->format(&af);

		bytesPerSec = af.bytes_per_sec;
		buffers[0].resize(bytesPerSec / 100);
		buffers[1].resize(bytesPerSec / 100);
		buffers[2].resize(bytesPerSec / 100);
	}
	~__xaudio2_backgroundaudio()
	{
		sourceVoice->DestroyVoice();
	}

public:
	virtual int32_t retain() override { return ++_refCount; }
	virtual int32_t release() override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual dseed::error_t format(dseed::media::audioformat* fmt) noexcept override
	{
		return sourceStream->format(fmt);
	}
	virtual dseed::audio::audiostate state() noexcept override
	{
		if (!isPlaying && !isPaused) return dseed::audio::audiostate::stopped;
		if (isPlaying && isPaused) return dseed::audio::audiostate::paused;

		XAUDIO2_VOICE_STATE state;
		sourceVoice->GetState(&state);

		if (state.BuffersQueued == 0)
			return dseed::audio::audiostate::stopped;

		return dseed::audio::audiostate::playing;
	}

public:
	virtual dseed::error_t play() noexcept override
	{
		if (state() != dseed::audio::audiostate::playing)
		{
			Buffering();
			if (FAILED(sourceVoice->Start()))
				return dseed::error_fail;

			isPlaying = true;
			isPaused = false;
		}
		return dseed::error_good;
	}
	virtual dseed::error_t stop() noexcept override
	{
		if (!isPlaying)
			return dseed::error_good;

		if (dseed::failed(sourceStream->seek(dseed::io::seekorigin::begin, 0)))
			return dseed::error_fail;

		sourceVoice->Stop();
		sourceVoice->FlushSourceBuffers();

		isPlaying = false;
		isPaused = false;
		return dseed::error_good;
	}
	virtual dseed::error_t pause() noexcept override
	{
		if (!isPlaying)
			return dseed::error_good;

		sourceVoice->Stop();
		sourceVoice->FlushSourceBuffers();

		isPaused = true;
		return dseed::error_good;
	}

public:
	virtual dseed::timespan position() noexcept override
	{
		return dseed::timespan::from_seconds(sourceStream->position() / (float)bytesPerSec);
	}
	virtual dseed::timespan duration() noexcept override
	{
		return dseed::timespan::from_seconds(sourceStream->length() / (float)bytesPerSec);
	}

public:
	virtual float volume() noexcept override
	{
		float vol = 0;
		sourceVoice->GetVolume(&vol);
		return vol;
	}
	virtual dseed::error_t set_volume(float vol) noexcept override
	{
		if (FAILED(sourceVoice->SetVolume(vol)))
			return dseed::error_fail;
		return dseed::error_good;
	}

public:
	virtual void __stdcall OnVoiceProcessingPassStart(UINT32 BytesRequired) override { }
	virtual void __stdcall OnVoiceProcessingPassEnd(void) override { }
	virtual void __stdcall OnStreamEnd(void) override { }
	virtual void __stdcall OnBufferStart(void* pBufferContext) override { }
	virtual void __stdcall OnBufferEnd(void* pBufferContext) override
	{
		if (!Buffering())
		{
			sourceStream->seek(dseed::io::seekorigin::begin, 0);
			isPlaying = false;
			isPaused = false;
		}
	}
	virtual void __stdcall OnLoopEnd(void* pBufferContext) override { }
	virtual void __stdcall OnVoiceError(void* pBufferContext, HRESULT Error) override { }

private:
	bool Buffering()
	{
		size_t read = sourceStream->read(buffers[currentBuffer].data(), buffers[currentBuffer].size());
		if (read == 0)
		{
			XAUDIO2_BUFFER buffer = {};
			buffer.Flags = XAUDIO2_END_OF_STREAM;
			sourceVoice->SubmitSourceBuffer(&buffer);

			return false;
		}

		XAUDIO2_BUFFER buffer = {};
		buffer.pAudioData = buffers[currentBuffer].data();
		buffer.AudioBytes = (UINT32)read;
		if (FAILED(sourceVoice->SubmitSourceBuffer(&buffer)))
			return false;;

		if (++currentBuffer >= 3)
			currentBuffer = 0;

		return true;
	}

private:
	std::atomic<int32_t> _refCount;

	IXAudio2SourceVoice* sourceVoice;
	dseed::autoref<dseed::media::audio_stream> sourceStream;

	bool isPlaying;
	bool isPaused;

	std::vector<uint8_t> buffers[3];
	uint32_t currentBuffer, bytesPerSec;
};

class __xaudio2_effectaudio : public dseed::audio::effectaudio
{
public:
	__xaudio2_effectaudio(IXAudio2* xaudio, const dseed::media::audioformat* fmt)
		: _refCount(1), xaudio2(xaudio), sourceFormat(*fmt), effects(32), frontEmptyEffect(0), frontEmptyInstance(0), effectVolumes(1)
	{

	}
	~__xaudio2_effectaudio()
	{

	}

public:
	dseed::error_t initialize(uint8_t destChannels)
	{
		if (sourceFormat.channels == 1)
		{
			if (FAILED(X3DAudioInitialize(destChannels, 1, x3dAudioHandle)))
				return dseed::error_fail;
		}

		return dseed::error_good;
	}

public:
	virtual int32_t retain() override { return ++_refCount; }
	virtual int32_t release() override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual dseed::error_t format(dseed::media::audioformat* fmt) noexcept override
	{
		*fmt = sourceFormat;
		return dseed::error_good;
	}

public:
	virtual dseed::error_t load_effectid(dseed::media::audio_stream* stream, dseed::audio::effectid_t* eid) noexcept override
	{
		dseed::media::audioformat af;
		stream->format(&af);

		if (sourceFormat.channels != af.channels || sourceFormat.bits_per_sample != af.bits_per_sample ||
			sourceFormat.samples_per_sec != af.samples_per_sec)
			return dseed::error_invalid_args;

		if (effects[frontEmptyEffect] != nullptr)
		{
			bool found = false;
			for (size_t i = frontEmptyEffect; i < effects.size(); ++i)
			{
				if (effects[frontEmptyEffect] == nullptr)
				{
					frontEmptyEffect = i;
					found = true;
					break;
				}
			}

			if (found == false)
			{
				frontEmptyEffect = effects.size();
				effects.resize(effects.size() * 2);
			}
		}

		std::vector<int8_t> buf(stream->length());
		stream->seek(dseed::io::seekorigin::begin, 0);
		stream->read(buf.data(), buf.size());

		dseed::autoref<dseed::blob> blob;
		if (dseed::failed(dseed::create_buffered_blob(buf.data(), buf.size(), &blob)))
			return dseed::error_fail;

		effects[*eid = (dseed::audio::effectid_t)frontEmptyEffect] = blob;
		++frontEmptyEffect;

		return dseed::error_good;
	}
	virtual dseed::error_t unload_effectid(dseed::audio::effectid_t eid) noexcept override
	{
		if (effects[eid] != nullptr)
		{
			effects[eid].release();
			if (frontEmptyEffect < eid)
				frontEmptyEffect = eid;
		}
		return dseed::error_good;
	}
	virtual dseed::error_t unload_all_effects() noexcept override
	{
		for (size_t i = 0; i < effects.size(); ++i)
			effects[i].release();
		frontEmptyEffect = 0;

		return dseed::error_good;
	}

public:
	virtual dseed::error_t create_instance(dseed::audio::effectid_t eid, dseed::audio::instanceid_t* iid) noexcept override
	{
		IXAudio2SourceVoice* sourceVoice;
		if (!instanceQueue.empty())
		{
			sourceVoice = instanceQueue.front();
			instanceQueue.pop();
		}
		else
		{
			WAVEFORMATEX wfex;
			convert_to_waveformatex(&sourceFormat, &wfex);

			if (FAILED(xaudio2->CreateSourceVoice(&sourceVoice, &wfex)))
				return dseed::error_fail;
		}

		if (std::get<IXAudio2SourceVoice*>(instances[frontEmptyInstance]) != nullptr)
		{
			bool found = false;
			for (size_t i = frontEmptyInstance; i < instances.size(); ++i)
			{
				if (std::get<IXAudio2SourceVoice*>(instances[frontEmptyInstance]) == nullptr)
				{
					frontEmptyInstance = i;
					found = true;
					break;
				}
			}

			if (found == false)
			{
				frontEmptyInstance = instances.size();
				instances.resize(instances.size() * 2);
			}
		}

		sourceVoice->SetVolume(effectVolumes);
		instances[*iid = frontEmptyInstance] = std::tuple<IXAudio2SourceVoice*, dseed::audio::effectid_t>(sourceVoice, eid);
		++frontEmptyInstance;

		return dseed::error_good;
	}
	virtual dseed::error_t release_instance(dseed::audio::instanceid_t iid) noexcept override
	{
		auto voice = std::get<IXAudio2SourceVoice*>(instances[iid]);
		if (voice == nullptr)
		{
			voice->DestroyVoice();
			instances[iid] = std::tuple<IXAudio2SourceVoice*, dseed::audio::effectid_t>(nullptr, dseed::audio::invalid_effectid);
			if (frontEmptyInstance < iid)
				frontEmptyInstance = iid;
		}
		return dseed::error_good;
	}
	virtual dseed::error_t release_all_instances() noexcept override
	{
		for (size_t i = 0; i < instances.size(); ++i)
		{
			auto voice = std::get<IXAudio2SourceVoice*>(instances[i]);
			if (voice == nullptr)
			{
				voice->DestroyVoice();
				instances[i] = std::tuple<IXAudio2SourceVoice*, dseed::audio::effectid_t>(nullptr, dseed::audio::invalid_effectid);
			}
		}
		frontEmptyInstance = 0;

		return dseed::error_good;
	}

public:
	virtual dseed::error_t set_listener(dseed::audio::instanceid_t iid, const dseed::audio::audiolistener* listener) noexcept override
	{
		if (sourceFormat.channels > 1)
			return dseed::error_invalid_op;

		if (std::get<IXAudio2SourceVoice*>(instances[iid]) == nullptr)
			return dseed::error_invalid_args;

		X3DAUDIO_LISTENER x3dListener = { };
		// TODO

		return dseed::error_good;
	}
	virtual dseed::error_t set_emitter(dseed::audio::instanceid_t iid, const dseed::audio::audioemitter* emitter) noexcept override
	{
		if (sourceFormat.channels > 1)
			return dseed::error_invalid_op;

		if (std::get<IXAudio2SourceVoice*>(instances[iid]) == nullptr)
			return dseed::error_invalid_args;

		X3DAUDIO_EMITTER x3dEmitter = { };
		// TODO

		return dseed::error_good;
	}

public:
	virtual dseed::error_t play(dseed::audio::instanceid_t iid) noexcept override
	{
		if (std::get<IXAudio2SourceVoice*>(instances[iid]) == nullptr)
			return dseed::error_invalid_args;
		std::get<IXAudio2SourceVoice*>(instances[iid])->Stop();
		Buffering(instances[iid]);
		std::get<IXAudio2SourceVoice*>(instances[iid])->Start();
		return dseed::error_good;
	}
	virtual dseed::error_t stop(dseed::audio::instanceid_t iid) noexcept override
	{
		if (std::get<IXAudio2SourceVoice*>(instances[iid]) == nullptr)
			return dseed::error_invalid_args;
		std::get<IXAudio2SourceVoice*>(instances[iid])->Stop();
		return dseed::error_good;
	}

public:
	virtual dseed::audio::audiostate state(dseed::audio::instanceid_t iid) noexcept override
	{
		auto voice = std::get<IXAudio2SourceVoice*>(instances[iid]);
		if (voice == nullptr)
			return dseed::audio::audiostate::stopped;

		XAUDIO2_VOICE_STATE state;
		voice->GetState(&state);

		if (state.BuffersQueued == 0)
			return dseed::audio::audiostate::stopped;
		return dseed::audio::audiostate::playing;
	}

public:
	virtual float volume() noexcept override { return effectVolumes; }
	virtual dseed::error_t set_volume(float vol) noexcept override
	{
		effectVolumes = vol;

		for (auto& i = instances.begin(); i != instances.end(); ++i)
		{
			auto voice = std::get<IXAudio2SourceVoice*>(*i);
			if (voice == nullptr)
				continue;
			voice->SetVolume(vol);
		}

		return dseed::error_good;
	}

private:
	void Buffering(std::tuple<IXAudio2SourceVoice*, dseed::audio::effectid_t>& instance) noexcept
	{
		auto voice = std::get<IXAudio2SourceVoice*>(instance);
		auto effectid = std::get<dseed::audio::effectid_t>(instance);

		XAUDIO2_BUFFER buf = { };
		buf.pAudioData = (const BYTE*)effects[effectid]->ptr();
		buf.AudioBytes = (UINT32)effects[effectid]->size();

		voice->FlushSourceBuffers();
		voice->SubmitSourceBuffer(&buf);
	}

private:
	std::atomic<int32_t> _refCount;

	Microsoft::WRL::ComPtr<IXAudio2> xaudio2;
	X3DAUDIO_HANDLE x3dAudioHandle;

	dseed::media::audioformat sourceFormat;
	float effectVolumes;

	std::vector<dseed::autoref<dseed::blob>> effects;
	size_t frontEmptyEffect;

	std::vector<std::tuple<IXAudio2SourceVoice*, dseed::audio::effectid_t>> instances;
	size_t frontEmptyInstance;
	std::queue<IXAudio2SourceVoice*> instanceQueue;
};

class __xaudio2_audioplayer : public dseed::audio::audioplayer
{
public:
	__xaudio2_audioplayer(IXAudio2* xaudio2, IXAudio2MasteringVoice* masteringVoice)
		: _refCount(1), xaudio2(xaudio2), masteringVoice(masteringVoice)
	{
		XAUDIO2_VOICE_DETAILS details;
		masteringVoice->GetVoiceDetails(&details);

		masterChannels = details.InputChannels;
	}
	~__xaudio2_audioplayer()
	{
		masteringVoice->DestroyVoice();
	}

public:
	virtual int32_t retain() override { return ++_refCount; }
	virtual int32_t release() override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual dseed::error_t native_object(void** nativeObject) override
	{
		if (nativeObject == nullptr)
			return dseed::error_invalid_args;

		auto& no = *reinterpret_cast<dseed::audio::xaudio2_audioplayer_nativeobject*>(nativeObject);
		xaudio2.As<IXAudio2>(&no.xaudio2);
		no.masteringVoice = masteringVoice;

		return dseed::error_good;
	}

public:
	virtual float volume() noexcept override
	{
		float vol = 0;
		masteringVoice->GetVolume(&vol);
		return vol;
	}

	virtual dseed::error_t set_volume(float vol) noexcept override
	{
		if (FAILED(masteringVoice->SetVolume(vol)))
			return dseed::error_fail;
		return dseed::error_good;
	}

public:
	virtual dseed::error_t create_backgroundaudio(dseed::media::audio_stream* stream, dseed::audio::backgroundaudio** audio) noexcept
	{
		if (stream == nullptr || audio == nullptr)
			return dseed::error_invalid_args;

		dseed::media::audioformat af;
		stream->format(&af);

		WAVEFORMATEX wf;
		convert_to_waveformatex(&af, &wf);

		IXAudio2SourceVoice* voice;
		if (FAILED(xaudio2->CreateSourceVoice(&voice, &wf)))
			return dseed::error_fail;

		*audio = new __xaudio2_backgroundaudio(voice, stream);
		if (*audio == nullptr)
			return dseed::error_out_of_memory;

		return dseed::error_good;
	}
	virtual dseed::error_t create_effectaudio(dseed::media::audioformat* format, dseed::audio::effectaudio** audio) noexcept
	{
		if (format == nullptr || audio == nullptr)
			return dseed::error_invalid_args;

		*audio = new __xaudio2_effectaudio(xaudio2.Get(), format);
		if (*audio == nullptr)
			return dseed::error_out_of_memory;

		if (dseed::failed(dynamic_cast<__xaudio2_effectaudio*>(*audio)->initialize(masterChannels)))
		{
			(*audio)->release();
			*audio = nullptr;
			return dseed::error_fail;
		}

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	Microsoft::WRL::ComPtr<IXAudio2> xaudio2;
	IXAudio2MasteringVoice* masteringVoice;

	uint8_t masterChannels;
};

dseed::error_t dseed::audio::create_xaudio2_audioplayer(dseed::audio::audioadapter* adapter, dseed::audio::audioplayer** player) noexcept
{
	if (player == nullptr)
		return dseed::error_invalid_args;
	if (adapter != nullptr && adapter->type() != dseed::audio::audioadapter_type::speaker)
		return dseed::error_invalid_args;

	WCHAR deviceId[128] = { 0, };
	if (adapter != nullptr)
	{
		dseed::audio::wasapi_audioadapter_nativeobject adapterNativeObject;
		adapter->native_object((void**)&adapterNativeObject);

		Microsoft::WRL::ComPtr<IPropertyStore> propStore;
		if (FAILED(adapterNativeObject.mmDevice->OpenPropertyStore(STGM_READ, &propStore)))
			return dseed::error_fail;

		PROPVARIANT pv;
		if (FAILED(propStore->GetValue(PKEY_AudioEndpoint_Path, &pv)))
			return dseed::error_fail;

		if (pv.vt != VT_LPWSTR)
		{
			PropVariantClear(&pv);
			return dseed::error_fail;
		}

		wcscpy_s(deviceId, pv.bstrVal);

		PropVariantClear(&pv);
	}

	DWORD xaudioFlag = 0;
#if defined (_DEBUG) || !defined (_NDEBUG)
	xaudioFlag = XAUDIO2_DEBUG_ENGINE;
#endif

	Microsoft::WRL::ComPtr<IXAudio2> xaudio2;
	if (FAILED(XAudio2Create(&xaudio2, xaudioFlag)))
	{
		if (xaudioFlag & XAUDIO2_DEBUG_ENGINE)
		{
			xaudioFlag &= ~XAUDIO2_DEBUG_ENGINE;
			if (FAILED(XAudio2Create(&xaudio2, xaudioFlag)))
				return dseed::error_fail;
		}
		else return dseed::error_fail;
	}

	if (xaudioFlag & XAUDIO2_DEBUG_ENGINE)
	{
		XAUDIO2_DEBUG_CONFIGURATION debugConfig = {};
		debugConfig.TraceMask = XAUDIO2_LOG_WARNINGS;
		debugConfig.BreakMask = XAUDIO2_LOG_ERRORS;
		xaudio2->SetDebugConfiguration(&debugConfig);
	}

	IXAudio2MasteringVoice* masteringVoice;
	if (FAILED(xaudio2->CreateMasteringVoice(&masteringVoice, 0, 0, 0,
		deviceId[0] == '\0' ? nullptr : deviceId,
		nullptr, AudioCategory_GameEffects)))
		return dseed::error_fail;

	*player = new __xaudio2_audioplayer(xaudio2.Get(), masteringVoice);
	if (*player == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}

#endif
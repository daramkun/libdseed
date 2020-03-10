#define USE_WASAPI_NATIVE_OBJECT
#include <dseed.h>

#if PLATFORM_MICROSOFT
#	include <initguid.h>
#	include <mmdeviceapi.h>
#	include <functiondiscoverykeys.h>
#	include <Audioclient.h>

#	include "../libs/WAVHelper.hxx"

class __wasapi_audioadapter : public dseed::audio::audioadapter
{
public:
	__wasapi_audioadapter (IMMDevice* device, dseed::audio::audioadapter_type type)
		: _refCount (1), device (device), _type (type)
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
	virtual dseed::error_t native_object (void** nativeObject) override
	{
		if (nativeObject == nullptr)
			return dseed::error_invalid_args;

		auto& no = *reinterpret_cast<dseed::audio::wasapi_audioadapter_nativeobject*>(nativeObject);
		device.As<IMMDevice> (&no.mmDevice);

		return dseed::error_good;
	}

public:
	virtual dseed::error_t name (char* name, size_t maxNameCount) noexcept override
	{
		if (name == nullptr || maxNameCount == 0)
			return dseed::error_invalid_args;

		Microsoft::WRL::ComPtr<IPropertyStore> propStore;
		device->OpenPropertyStore (STGM_READ, &propStore);

		PROPVARIANT pv;
		if (FAILED (propStore->GetValue (PKEY_Device_FriendlyName, &pv)))
			return dseed::error_fail;

		if (pv.vt != VT_LPWSTR)
			return dseed::error_fail;
		
		std::string str = dseed::utf16_to_utf8 ((char16_t*)pv.bstrVal);
		strcpy_s (name, maxNameCount, str.c_str ());

		PropVariantClear (&pv);

		return dseed::error_good;
	}

	virtual dseed::error_t info (dseed::media::audioformat* info) noexcept override
	{
		if (info == nullptr)
			return dseed::error_invalid_args;

		Microsoft::WRL::ComPtr<IPropertyStore> propStore;
		device->OpenPropertyStore (STGM_READ, &propStore);

		PROPVARIANT pv;
		if (FAILED (propStore->GetValue (PKEY_AudioEngine_DeviceFormat, &pv)))
			return dseed::error_fail;

		if (pv.vt != VT_BLOB)
		{
			PropVariantClear (&pv);
			return dseed::error_fail;
		}

		auto wf = reinterpret_cast<WAVEFORMATEX*>(pv.blob.pBlobData);
		convert_from_waveformatex (wf, info);

		PropVariantClear (&pv);

		return dseed::error_good;
	}
	virtual dseed::audio::audioadapter_type type () noexcept override { return _type; }

private:
	std::atomic<int32_t> _refCount;
	Microsoft::WRL::ComPtr<IMMDevice> device;
	dseed::audio::audioadapter_type _type;
};

class __wasapi_audioadapter_enumerator : public dseed::audio::audioadapter_enumerator
{
public:
	__wasapi_audioadapter_enumerator (IMMDeviceEnumerator* deviceEnumerator, dseed::audio::audioadapter_type type)
		: _refCount (1), deviceEnumerator (deviceEnumerator),  type (type)
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

	virtual dseed::error_t audioadapter (size_t index, dseed::audio::audioadapter** adapter) noexcept override
	{
		if (adapter == nullptr) return dseed::error_invalid_args;
		if (index >= audioadapter_count ()) return dseed::error_out_of_range;

		Microsoft::WRL::ComPtr<IMMDevice> device;
		if (FAILED (deviceCollection->Item ((UINT)index, &device)))
			return dseed::error_fail;

		*adapter = new __wasapi_audioadapter (device.Get (), type);
		if (*adapter == nullptr)
			return dseed::error_out_of_memory;

		return dseed::error_good;
	}

	virtual size_t audioadapter_count () noexcept override
	{
		UINT count;
		deviceCollection->GetCount (&count);
		return count;
	}

public:
	HRESULT refresh () noexcept
	{
		HRESULT hr;

		deviceCollection.ReleaseAndGetAddressOf ();
		if (FAILED (hr = deviceEnumerator->EnumAudioEndpoints (
			(type == dseed::audio::audioadapter_type::speaker) ? eRender : eCapture
			, DEVICE_STATE_ACTIVE, &deviceCollection)))
			return hr;

		return S_OK;
	}

private:
	std::atomic<int32_t> _refCount;
	Microsoft::WRL::ComPtr<IMMDeviceEnumerator> deviceEnumerator;
	dseed::audio::audioadapter_type type;
	Microsoft::WRL::ComPtr<IMMDeviceCollection> deviceCollection;
};

#endif

dseed::error_t dseed::audio::create_wasapi_audiooadapter_enumerator (dseed::audio::audioadapter_type type,
	dseed::audio::audioadapter_enumerator** enumerator) noexcept
{
#if PLATFORM_MICROSOFT
	HRESULT hr;
	Microsoft::WRL::ComPtr<IMMDeviceEnumerator> deviceEnumerator;
	if (FAILED (hr = CoCreateInstance (__uuidof (MMDeviceEnumerator), nullptr, CLSCTX_ALL,
		__uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator)))
		return dseed::error_fail;

	dseed::autoref<__wasapi_audioadapter_enumerator> wasapiAudioAdapterEnumerator;
	*&wasapiAudioAdapterEnumerator = new __wasapi_audioadapter_enumerator (deviceEnumerator.Get (), type);
	if (wasapiAudioAdapterEnumerator == nullptr)
		return dseed::error_out_of_memory;

	if (FAILED (wasapiAudioAdapterEnumerator->refresh ()))
	{
		wasapiAudioAdapterEnumerator.release ();
		return dseed::error_fail;
	}

	(*enumerator = wasapiAudioAdapterEnumerator)->retain ();

	return dseed::error_good;
#else
	return dseed::error_not_support;
#endif
}
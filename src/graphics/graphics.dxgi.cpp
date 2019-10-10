#include <dseed.h>

#if PLATFORM_MICROSOFT
#	include <VersionHelpers.h>
#	include <dxgi.h>
#	include <atlbase.h>

class __dxgi_display : public dseed::display
{
public:
	__dxgi_display (IDXGIOutput* output)
		: _refCount(1), _dxgiOutput(output)
	{
		refresh ();
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
		if (nativeObject == nullptr) return dseed::error_invalid_args;
		_dxgiOutput->QueryInterface (__uuidof(IDXGIOutput), nativeObject);
		return dseed::error_good;
	}

public:
	virtual dseed::error_t name (char* name, size_t maxNameCount) override
	{
		if (name == nullptr || maxNameCount == 0)
			return dseed::error_invalid_args;

		DXGI_OUTPUT_DESC desc;
		_dxgiOutput->GetDesc (&desc);

		MONITORINFOEXA monitorInfoEx = {};
		monitorInfoEx.cbSize = sizeof (MONITORINFOEXA);
		GetMonitorInfoA (desc.Monitor, &monitorInfoEx);

		DISPLAY_DEVICEA displayDevice = {};
		displayDevice.cb = sizeof (DISPLAY_DEVICEA);
		EnumDisplayDevicesA (monitorInfoEx.szDevice, 0, &displayDevice, 0);

		strcpy_s (name, maxNameCount, displayDevice.DeviceString);

		return dseed::error_good;
	}

public:
	virtual dseed::error_t displaymode (int index, dseed::displaymode* mode) override
	{
		if (mode == nullptr || index >= _displayModes.size () || index < 0)
			return dseed::error_invalid_args;

		*mode = _displayModes[index];

		return dseed::error_good;
	}
	virtual size_t displaymode_count () override { return _displayModes.size (); }

public:
	virtual dseed::error_t refresh () override
	{
		_displayModes.clear ();

		UINT numModes = 0;
		if (FAILED (_dxgiOutput->GetDisplayModeList (DXGI_FORMAT_B8G8R8A8_UNORM, 0, &numModes, nullptr)))
			return dseed::error_fail;
		std::shared_ptr<DXGI_MODE_DESC[]> modes (new DXGI_MODE_DESC[numModes]);
		_displayModes.resize (numModes);
		if (FAILED (_dxgiOutput->GetDisplayModeList (DXGI_FORMAT_B8G8R8A8_UNORM, 0, &numModes, &modes[0])))
			return dseed::error_fail;

		for (size_t i = 0; ; ++i)
		{
			DXGI_MODE_DESC& mode = modes[i];
			dseed::displaymode dm = {};
			dm.format = dseed::pixelformat::bgra8888;
			dm.resolution = dseed::size2i (mode.Width, mode.Height);
			dm.refresh_rate = dseed::fraction (mode.RefreshRate.Numerator, mode.RefreshRate.Denominator);
			_displayModes.push_back (dm);
		}

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	CComPtr<IDXGIOutput> _dxgiOutput;

	std::vector<dseed::displaymode> _displayModes;
};

class __dxgi_vga_adapter : public dseed::vga_adapter
{
public:
	__dxgi_vga_adapter (IDXGIAdapter* adapter)
		: _refCount (1), _dxgiAdapter (adapter)
	{
		refresh ();
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
		if (nativeObject == nullptr) return dseed::error_invalid_args;
		_dxgiAdapter->QueryInterface (__uuidof(IDXGIAdapter), nativeObject);
		return dseed::error_good;
	}

public:
	virtual dseed::error_t name (char* name, size_t maxNameCount) override
	{
		if (name == nullptr || maxNameCount == 0)
			return dseed::error_invalid_args;

		DXGI_ADAPTER_DESC desc;
		if (FAILED (_dxgiAdapter->GetDesc (&desc)))
			return dseed::error_fail;

		dseed::utf16_to_utf8 ((const char16_t*)desc.Description, name, maxNameCount);

		return dseed::error_good;
	}

public:
	virtual dseed::error_t display (int index, dseed::display** display) override
	{
		if (display == nullptr || index >= _displays.size () || index < 0)
			return dseed::error_invalid_args;

		(*display = _displays[index])->retain ();		

		return dseed::error_good;
	}

	virtual size_t display_count () override { return _displays.size (); }

public:
	virtual dseed::error_t refresh () override
	{
		_displays.clear ();

		for (size_t i = 0; ; ++i)
		{
			CComPtr<IDXGIOutput> output;
			if (FAILED (_dxgiAdapter->EnumOutputs (i, &output)))
				break;

			_displays.push_back (new __dxgi_display (output));
		}

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	CComPtr<IDXGIAdapter> _dxgiAdapter;

	std::vector<dseed::auto_object<dseed::display>> _displays;
};

class __dxgi_vga_adapter_enumerator : public dseed::vga_adapter_enumerator
{
public:
	__dxgi_vga_adapter_enumerator (IDXGIFactory* factory)
		: _refCount (1), _dxgiFactory (factory)
	{
		refresh ();
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
	virtual dseed::error_t vga_adapter (int index, dseed::vga_adapter** adapter) override
	{
		if (adapter == nullptr || index >= _adapters.size () || index < 0)
			return dseed::error_invalid_args;

		(*adapter = _adapters[index])->retain ();

		return dseed::error_good;
	}
	virtual size_t vga_adapter_count () override { return _adapters.size (); }

public:
	virtual dseed::error_t refresh () override
	{
		_adapters.clear ();

		for (size_t i = 0; ; ++i)
		{
			CComPtr<IDXGIAdapter> adapter;
			if (IsWindows8OrGreater ())
			{
				CComQIPtr<IDXGIFactory1> factory1 = _dxgiFactory;
				if (FAILED (factory1->EnumAdapters1 (i, (IDXGIAdapter1 * *)& adapter)))
					break;
			}
			else
			{
				if (FAILED (_dxgiFactory->EnumAdapters (i, &adapter)))
					break;
			}

			_adapters.push_back (new __dxgi_vga_adapter (adapter));
		}

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	CComPtr<IDXGIFactory> _dxgiFactory;

	std::vector<dseed::auto_object<dseed::vga_adapter>> _adapters;
};
#endif

dseed::error_t dseed::create_dxgi_vga_adapter_enumerator (dseed::vga_adapter_enumerator** e)
{
	if (e == nullptr)
		return dseed::error_invalid_args;

	CComPtr<IDXGIFactory> factory;

	if (IsWindows8OrGreater ())
	{
		if (FAILED (CreateDXGIFactory (__uuidof(IDXGIFactory1), (void**)& factory)))
			return dseed::error_fail;
	}
	else
	{
		if (FAILED (CreateDXGIFactory (__uuidof(IDXGIFactory), (void**)& factory)))
			return dseed::error_fail;
	}

	*e = new __dxgi_vga_adapter_enumerator (factory);
	if (*e == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}
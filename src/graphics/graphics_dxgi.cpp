#include <dseed.h>

#if PLATFORM_MICROSOFT

#include "common_dxgi.hxx"

////////////////////////////////////////////////////////////////////////////////////////////
// Display
////////////////////////////////////////////////////////////////////////////////////////////
__dxgi_display::__dxgi_display (IDXGIOutput* output)
	: _refCount (1), _dxgiOutput (output)
{
	refresh ();
}

dseed::error_t __dxgi_display::name (char* name, size_t maxNameCount)
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

dseed::error_t __dxgi_display::displaymode (int index, dseed::graphics::displaymode* mode)
{
	if (mode == nullptr || index >= _displayModes.size () || index < 0)
		return dseed::error_invalid_args;

	*mode = _displayModes[index];

	return dseed::error_good;
}

size_t __dxgi_display::displaymode_count () { return _displayModes.size (); }

dseed::error_t __dxgi_display::area (dseed::rect2i* area)
{
	if (area == nullptr)
		return dseed::error_invalid_args;

	DXGI_OUTPUT_DESC desc;
	_dxgiOutput->GetDesc (&desc);

	MONITORINFOEXA monitorInfoEx = {};
	monitorInfoEx.cbSize = sizeof (MONITORINFOEXA);
	GetMonitorInfoA (desc.Monitor, &monitorInfoEx);

	*area = dseed::rect2i (
		monitorInfoEx.rcMonitor.left,
		monitorInfoEx.rcMonitor.top,
		monitorInfoEx.rcMonitor.right - monitorInfoEx.rcMonitor.left,
		monitorInfoEx.rcMonitor.bottom - monitorInfoEx.rcMonitor.top
	);

	return dseed::error_good;
}

dseed::error_t __dxgi_display::refresh ()
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
		dseed::graphics::displaymode dm = {};
		dm.format = dseed::color::pixelformat::bgra8;
		dm.resolution = dseed::size2i (mode.Width, mode.Height);
		dm.refresh_rate = dseed::fraction (mode.RefreshRate.Numerator, mode.RefreshRate.Denominator);
		_displayModes.push_back (dm);
	}

	return dseed::error_good;
}

////////////////////////////////////////////////////////////////////////////////////////////
// VGA Adapter
////////////////////////////////////////////////////////////////////////////////////////////
__dxgi_vga_adapter::__dxgi_vga_adapter (IDXGIAdapter* adapter)
	: _refCount (1), _dxgiAdapter (adapter)
{
	refresh ();
}

dseed::error_t __dxgi_vga_adapter::name (char* name, size_t maxNameCount)
{
	if (name == nullptr || maxNameCount == 0)
		return dseed::error_invalid_args;

	DXGI_ADAPTER_DESC desc;
	if (FAILED (_dxgiAdapter->GetDesc (&desc)))
		return dseed::error_fail;

	dseed::utf16_to_utf8 ((const char16_t*)desc.Description, name, maxNameCount);

	return dseed::error_good;
}

dseed::error_t __dxgi_vga_adapter::display (int index, dseed::graphics::display** display)
{
	if (display == nullptr || index >= _displays.size () || index < 0)
		return dseed::error_invalid_args;

	(*display = _displays[index])->retain ();

	return dseed::error_good;
}

size_t __dxgi_vga_adapter::display_count () { return _displays.size (); }

dseed::error_t __dxgi_vga_adapter::refresh ()
{
	_displays.clear ();

	for (size_t i = 0; ; ++i)
	{
		Microsoft::WRL::ComPtr<IDXGIOutput> output;
		if (FAILED (_dxgiAdapter->EnumOutputs ((UINT)i, &output)))
			break;

		_displays.push_back (new __dxgi_display (output.Get ()));
	}

	return dseed::error_good;
}

////////////////////////////////////////////////////////////////////////////////////////////
// VGA Adapter Enumerator
////////////////////////////////////////////////////////////////////////////////////////////
__dxgi_vga_adapter_enumerator::__dxgi_vga_adapter_enumerator (IDXGIFactory* factory)
	: _refCount (1), _dxgiFactory (factory)
{
	refresh ();
}

dseed::error_t __dxgi_vga_adapter_enumerator::adapter (int index, dseed::graphics::vgaadapter** adapter)
{
	if (adapter == nullptr || index >= _adapters.size () || index < 0)
		return dseed::error_invalid_args;

	(*adapter = _adapters[index])->retain ();

	return dseed::error_good;
}

size_t __dxgi_vga_adapter_enumerator::adapter_count () { return _adapters.size (); }

dseed::error_t __dxgi_vga_adapter_enumerator::refresh ()
{
	_adapters.clear ();

	for (size_t i = 0; ; ++i)
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
		if (IsWindows8OrGreater ())
		{
			Microsoft::WRL::ComPtr<IDXGIFactory1> factory1;
			_dxgiFactory.As<IDXGIFactory1> (&factory1);

			Microsoft::WRL::ComPtr<IDXGIAdapter1> __adapter;
			if (FAILED (factory1->EnumAdapters1 ((UINT)i, &__adapter)))
				break;

			__adapter.As<IDXGIAdapter> (&adapter);
		}
		else
		{
			if (FAILED (_dxgiFactory->EnumAdapters ((UINT)i, &adapter)))
				break;
		}

		_adapters.push_back (new __dxgi_vga_adapter (adapter.Get ()));
	}

	return dseed::error_good;
}

#endif

dseed::error_t dseed::graphics::create_dxgi_vgaadapter_enumerator (vgaadapter_enumerator** enumerator) noexcept
{
#if PLATFORM_MICROSOFT
	if (enumerator == nullptr)
		return dseed::error_invalid_args;

	Microsoft::WRL::ComPtr<IDXGIFactory> factory;

	if (IsWindows8OrGreater ())
	{
		if (FAILED (CreateDXGIFactory (__uuidof(IDXGIFactory1), (void**)&factory)))
			return dseed::error_fail;
	}
	else
	{
		if (FAILED (CreateDXGIFactory (__uuidof(IDXGIFactory), (void**)&factory)))
			return dseed::error_fail;
	}

	*enumerator = new __dxgi_vga_adapter_enumerator (factory.Get ());

#else
	return dseed::error_not_support;
#endif
	return dseed::error_good;
}
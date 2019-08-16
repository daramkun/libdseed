#include <dseed.h>

static dseed::application* g_sharedApp;

dseed::error_t dseed::application::shared_app (dseed::application** app)
{
	if (app == nullptr) return dseed::error_invalid_args;
	*app = g_sharedApp;
	return dseed::error_good;
}

class dummy_handler : public dseed::event_handler
{
public:
	dummy_handler () : _refCount (1) { }

public:
	virtual int32_t retain () override { return ++_refCount; }
	virtual int32_t release () override
	{
		auto ret = --_refCount;
		if (ret == 0) delete this;
		return ret;
	}

public:
	virtual void activated () override { }
	virtual void deactivated () override { }
	virtual void paused () override { }
	virtual void resumed () override { }
	virtual void resized () override { }
	virtual void shown () override { }
	virtual void closing (bool& cancel) override { }
	virtual void closed () override { }
	virtual void next_frame (dseed::timespan_t delta) override { }

private:
	std::atomic<int32_t> _refCount;
};

class inputdevice_internal : public dseed::inputdevice
{
public:
	virtual dseed::error_t update () = 0;
	virtual dseed::error_t cleanup () = 0;
};

#if PLATFORM_WINDOWS
constexpr LPCWSTR WINDOW_CLASSNAME = L"libdseed";
constexpr DWORD WINDOW_STYLE_WINDOWED = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
constexpr DWORD WINDOW_STYLE_FRAMELESS_FULLSCREEN = WS_VISIBLE | WS_POPUP | WS_CHILDWINDOW | WS_SYSMENU;

bool update_rawinput (HWND hWnd, bool no_legacy)
{
	DWORD flags = (DWORD)((no_legacy ? RIDEV_NOLEGACY : 0) | RIDEV_INPUTSINK);
	RAWINPUTDEVICE rawInput[2] = {
		{ 0x01, 0x06, flags, hWnd },						// Keyboard
		{ 0x01, 0x02, flags, hWnd },						// Mouse
	};
	return (bool)RegisterRawInputDevices (rawInput, 2, sizeof (RAWINPUTDEVICE));
}

class __win32_application : public dseed::application
{
public:
	__win32_application ()
		: _refCount (1), _hWnd (nullptr), _handler (new dummy_handler ())
	{

	}

public:
	virtual int32_t retain () override { return ++_refCount; }
	virtual int32_t release () override
	{
		auto ret = --_refCount;
		if (ret == 0)
		{
			delete this;
			g_sharedApp = nullptr;
		}
		return ret;
	}

public:
	virtual dseed::error_t native_object (void** nativeObject) override
	{
		if (_hWnd == nullptr)
			return dseed::error_invalid_op;
		if (nativeObject == nullptr)
			return dseed::error_invalid_args;
		*nativeObject = _hWnd;
		return dseed::error_good;
	}

public:
	virtual dseed::error_t input_device (dseed::inputdevice_t type, dseed::inputdevice** device) override
	{
		if (_hWnd == nullptr)
			return dseed::error_invalid_op;

		if (type > dseed::inputdevice_headtracker || type < dseed::inputdevice_keyboard)
			return dseed::error_invalid_args;

		if (_inputDevices[(int)type] == nullptr)
			return dseed::error_not_support;

		*device = _inputDevices[(int)type];

		return dseed::error_good;
	}

public:
	virtual dseed::error_t set_title (const char* title) override
	{
		if (_hWnd == nullptr) return dseed::error_invalid_op;
		if (title == nullptr) title = "";
		if (!::SetWindowTextW (_hWnd, (LPCWSTR)dseed::utf8_to_utf16 (std::string (title)).c_str ()))
			return dseed::error_fail;
		return dseed::error_good;
	}

public:
	virtual dseed::error_t client_size (dseed::size2i* size) override
	{
		if (_hWnd == nullptr) return dseed::error_invalid_op;
		if (size == nullptr) return dseed::error_invalid_args;

		RECT rect;
		if (!::GetClientRect (_hWnd, &rect))
			return dseed::error_fail;

		size->width = rect.right - rect.left;
		size->height = rect.bottom - rect.top;

		return dseed::error_good;
	}

	virtual dseed::error_t set_client_size (const dseed::size2i& size) override
	{
		if (_hWnd == nullptr) return dseed::error_invalid_op;

		DWORD style = ::GetWindowLong (_hWnd, GWL_STYLE);

		RECT rect = { 0, 0, size.width, size.height };
		if (!::AdjustWindowRect (&rect, style, FALSE))
			return dseed::error_fail;

		if (!::SetWindowPos (_hWnd, NULL, 0, 0,
			rect.right - rect.left, rect.bottom - rect.top,
			SWP_NOMOVE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER | SWP_NOZORDER))
			return dseed::error_fail;

		return dseed::error_good;
	}

public:
	virtual dseed::error_t window_mode (dseed::windowmode_t* mode) override
	{
		if (_hWnd == nullptr) return dseed::error_invalid_op;

		DWORD style = ::GetWindowLong (_hWnd, GWL_STYLE);

		if ((style & WINDOW_STYLE_WINDOWED) == WINDOW_STYLE_WINDOWED)
			*mode = dseed::windowmode_windowed;
		else if ((style & WINDOW_STYLE_FRAMELESS_FULLSCREEN) == WINDOW_STYLE_FRAMELESS_FULLSCREEN)
			*mode = dseed::windowmode_borderless_fullscreen;
		else
			*mode = dseed::windowmode_unknown;

		return dseed::error_good;
	}

	virtual dseed::error_t set_window_mode (dseed::windowmode_t mode) override
	{
		if (_hWnd == nullptr) return dseed::error_invalid_op;

		switch (mode)
		{
		case dseed::windowmode_windowed:
			::SetWindowLong (_hWnd, GWL_STYLE, WINDOW_STYLE_WINDOWED);
			break;

		case dseed::windowmode_borderless_fullscreen:
			::SetWindowLong (_hWnd, GWL_STYLE, WINDOW_STYLE_FRAMELESS_FULLSCREEN);
			break;

		default:
			return dseed::error_fail;
		}

		return dseed::error_good;
	}

public:
	virtual dseed::error_t event_handler (dseed::event_handler** handler)
	{
		if (handler == nullptr) return dseed::error_invalid_args;
		*handler = _handler;
		return dseed::error_good;
	}
	virtual dseed::error_t set_event_handler (dseed::event_handler* handler)
	{
		_handler = handler;
		if (_handler == nullptr)
			_handler = new dummy_handler ();
		return dseed::error_good;
	}

public:
	virtual dseed::error_t run (dseed::event_handler* handler) override
	{
		set_event_handler (handler);

		RECT clientRect = { 0, 0, 640, 360 };
		AdjustWindowRect (&clientRect, WINDOW_STYLE_WINDOWED, FALSE);

		int width = clientRect.right - clientRect.left,
			height = clientRect.bottom - clientRect.top,
			x = GetSystemMetrics (SM_CXSCREEN) / 2 - width / 2,
			y = GetSystemMetrics (SM_CYSCREEN) / 2 - height / 2;

		_hWnd = CreateWindowW (WINDOW_CLASSNAME, L"DSeed Window", WINDOW_STYLE_WINDOWED,
			x, y, width, height, nullptr, nullptr, GetModuleHandle (nullptr), nullptr);

		if (_hWnd == nullptr)
			return dseed::error_fail;

		if (!update_rawinput (_hWnd, false))
			return dseed::error_fail;

		if (!::ShowWindow (_hWnd, SW_SHOW))
			return dseed::error_fail;

		MSG msg; bool running = true;
		dseed::timespan_t lastTime = dseed::timespan_t::current_ticks (), currentTime;
		do
		{
			while (::PeekMessage (&msg, nullptr, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					running = false;
					break;
				}

				::TranslateMessage (&msg);
				::DispatchMessage (&msg);
			}

			for (dseed::auto_object<inputdevice_internal>& inputdevice : _inputDevices)
				inputdevice->update ();

			currentTime = dseed::timespan_t::current_ticks ();
			handler->next_frame (currentTime - lastTime);

			for (dseed::auto_object<inputdevice_internal>& inputdevice : _inputDevices)
				inputdevice->cleanup ();

			lastTime = currentTime;

			::Sleep (0);
		} while (running);

		return dseed::error_good;
	}

private:
	static LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{

	}

private:
	std::atomic<int32_t> _refCount;

	HWND _hWnd;
	dseed::auto_object<inputdevice_internal> _inputDevices [8];

	dseed::auto_object<dseed::event_handler> _handler;
};

using __application = __win32_application;
#else

#endif

dseed::error_t dseed::create_application (dseed::application** app)
{
	if (g_sharedApp != nullptr) return dseed::error_invalid_op;

	if (app == nullptr) return dseed::error_invalid_args;

	*app = g_sharedApp = new __application ();
	if (*app == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}
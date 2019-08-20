#include <dseed.h>

extern dseed::application* g_sharedApp;

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
	inputdevice_internal () : _refCount (1) { }

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
	virtual void update () noexcept { }
	virtual void cleanup () noexcept { }

private:
	std::atomic<int32_t> _refCount;
};

template<typename TState, dseed::inputdevice_t device_type, size_t maximum_player_index = 1, bool device_conn_default = false>
class inputdevice_internal_template1 : public inputdevice_internal
{
public:
	inputdevice_internal_template1 ()
	{
		memset (_state, 0, sizeof (_state));
		memset (_deviceConnected, device_conn_default, sizeof(_deviceConnected));
	}

public:
	virtual dseed::inputdevice_t type () override { return device_type; }

public:
	virtual dseed::error_t state (void* state, size_t state_size, int32_t player_index = 0) override
	{
		if ((player_index < 0 || player_index > maximum_player_index)
			|| state_size != sizeof (TState) || state == nullptr)
			return dseed::error_invalid_args;
		if (!_deviceConnected[player_index])
			return dseed::error_device_disconn;
		*(reinterpret_cast<TState*> (state)) = this->_state[player_index];
		return dseed::error_good;
	}

	virtual dseed::error_t send_feedback (const void* feedback, size_t feedback_size, int32_t player_index = 0) override { return dseed::error_not_support; }

public:
	TState _state[maximum_player_index];
	bool _deviceConnected[maximum_player_index];
};

template<typename TState, typename TFeedback, dseed::inputdevice_t device_type, size_t maximum_player_index = 1, bool device_conn_default = false>
class inputdevice_internal_template2 : public inputdevice_internal_template1<TState, device_type, maximum_player_index, device_conn_default>
{
public:
	virtual dseed::error_t send_feedback (const void* feedback, size_t feedbackSize, int32_t playerIndex = 0) override
	{
		if (playerIndex < 0 || playerIndex > maximum_player_index
			|| feedbackSize != sizeof (TFeedback) || feedback == nullptr)
			return dseed::error_invalid_args;
		return do_feedback (reinterpret_cast<const TFeedback*> (feedback), playerIndex);
	}

private:
	virtual dseed::error_t do_feedback (const TFeedback* feedback, int32_t player_index) = 0;
};

class keyboard_common : public inputdevice_internal_template1<dseed::keyboard_state, dseed::inputdevice_keyboard, 1, true> { };
class mouse_common : public inputdevice_internal_template2<dseed::mouse_state, dseed::mouse_feedback, dseed::inputdevice_mouse, 1, true>
{
public:
	virtual dseed::error_t do_feedback (const dseed::mouse_feedback* feedback, int32_t player_index) noexcept
	{
		return dseed::error_not_impl;
	}
};
class gamepad_common : public inputdevice_internal_template2<dseed::gamepad_state, dseed::gamepad_feedback, dseed::inputdevice_gamepad, 8>
{
public:
	virtual dseed::error_t do_feedback (const dseed::gamepad_feedback* feedback, int32_t player_index) noexcept
	{
		return dseed::error_not_impl;
	}
};
class touchpanel_common : public inputdevice_internal_template1<dseed::touchpanel_state, dseed::inputdevice_touchpanel>
{
public:
	virtual void cleanup () noexcept override
	{
		if (_state[0].pointerCount == 0)
			return;

		std::vector<dseed::touchpointer> pointers (&_state[0].pointers[0], &_state[0].pointers[_state[0].pointerCount]);
		std::vector<std::vector<dseed::touchpointer>::iterator> removeList;
		for (auto i = pointers.begin (); i != pointers.end (); ++i)
		{
			dseed::touchpointer& p = *i;
			if (p.state == dseed::touchstate_pressed)
				p.state = dseed::touchstate_moved;
			else if (p.state == dseed::touchstate_released)
				removeList.push_back (i);
		}

		for (auto i : removeList)
			pointers.erase (i);

		std::copy (pointers.begin (), pointers.end (), _state[0].pointers);
		memset (&_state[0].pointers[pointers.size ()], 0, sizeof (dseed::touchpointer) * (256 - pointers.size () - 1));
		_state[0].pointerCount = pointers.size ();
	}

public:
	dseed::touchpointer* pointer (int32_t pid)
	{
		for (int i = 0; i < _state[0].pointerCount; ++i)
			if (_state[0].pointers[i].pointerId == pid)
				return &(_state[0].pointers[i]);
		return nullptr;
	}
};
class accelerometer_common : public inputdevice_internal_template1<dseed::accelerometer_state, dseed::inputdevice_accelerometer> { };
class gyroscope_common : public inputdevice_internal_template1<dseed::gyroscope_state, dseed::inputdevice_gyroscope> { };
class gps_common : public inputdevice_internal_template1<dseed::gps_state, dseed::inputdevice_gps> { };

#if PLATFORM_WINDOWS
constexpr LPCWSTR WINDOW_CLASSNAME = L"libdseed";
constexpr DWORD WINDOW_STYLE_WINDOWED = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
constexpr DWORD WINDOW_STYLE_SIZABLE_WINDOWED = WS_OVERLAPPEDWINDOW;
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

#include <XInput.h>
#include <initguid.h>
#include <SensorsApi.h>
#include <Sensors.h>
#include <atlbase.h>

class mouse_win32 : public mouse_common
{
public:
	virtual dseed::error_t do_feedback (const dseed::mouse_feedback* feedback, int32_t player_index) noexcept override
	{
		dseed::auto_object<dseed::application> app;
		dseed::application::shared_app (&app);
		HWND hWnd;
		app->native_object ((void**)& hWnd);

		if (feedback->lock_in_window)
		{
			RECT rect;
			::GetClientRect (hWnd, &rect);

			POINT p1 = { rect.left, rect.top }, p2 = { rect.right, rect.bottom };
			::ClientToScreen (hWnd, &p1);
			::ClientToScreen (hWnd, &p2);
			rect = { p1.x, p1.y, p2.x, p2.y };

			::ClipCursor (&rect);
		}
		else ::ClipCursor (nullptr);

		::ShowCursor (!feedback->hide_cursor);
		if (_state->is_relative != feedback->set_relative)
		{
			if (feedback->set_relative)
			{
				update_rawinput (hWnd, true);
			}
			else
			{
				update_rawinput (hWnd, false);
			}
			_state->is_relative = feedback->set_relative;
		}

		if (feedback->move_position.x == INT_MIN && feedback->move_position.y == INT_MIN)
		{
			POINT p = { feedback->move_position.x, feedback->move_position.y };
			::ClientToScreen (hWnd, &p);
			::SetCursorPos (p.x, p.y);
		}

		return dseed::error_good;
	}
};

class gamepad_win32 : public gamepad_common
{
public:
	virtual void update () noexcept override
	{
		for (int i = 0; i < 8; ++i)
		{
			XINPUT_STATE s;
			_deviceConnected[i] = ::XInputGetState (i, &s) != ERROR_DEVICE_NOT_CONNECTED;
			if (_deviceConnected[i] == ERROR_SUCCESS)
			{
				_state[i].left_thumbstick = dseed::float2 (
					s.Gamepad.sThumbLX / 32767.0f, s.Gamepad.sThumbLY / 32767.0f
				);
				_state[i].right_thumbstick = dseed::float2 (
					s.Gamepad.sThumbRX / 32767.0f, s.Gamepad.sThumbRY / 32767.0f
				);
				_state[i].left_trigger = s.Gamepad.bLeftTrigger / 255.0f;
				_state[i].right_trigger = s.Gamepad.bRightTrigger / 255.0f;

				int32_t buttons = 0;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_A) buttons |= dseed::gamepadbuttons_a;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_B) buttons |= dseed::gamepadbuttons_b;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_X) buttons |= dseed::gamepadbuttons_x;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_Y) buttons |= dseed::gamepadbuttons_y;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) buttons |= dseed::gamepadbuttons_dpad_left;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) buttons |= dseed::gamepadbuttons_dpad_right;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) buttons |= dseed::gamepadbuttons_dpad_up;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) buttons |= dseed::gamepadbuttons_dpad_down;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) buttons |= dseed::gamepadbuttons_left_bumper;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) buttons |= dseed::gamepadbuttons_right_thumbstick;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) buttons |= dseed::gamepadbuttons_left_thumbstick;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) buttons |= dseed::gamepadbuttons_right_thumbstick;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) buttons |= dseed::gamepadbuttons_back;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_START) buttons |= dseed::gamepadbuttons_start;
				_state[i].buttons = (dseed::gamepadbuttons_t) buttons;
			}
		}
	}

public:
	virtual dseed::error_t do_feedback (const dseed::gamepad_feedback* feedback, int32_t player_index) noexcept override
	{
		XINPUT_VIBRATION vib;
		vib.wLeftMotorSpeed = (WORD)(feedback->left_motor_speed * 65535);
		vib.wRightMotorSpeed = (WORD)(feedback->right_motor_speed * 65535);
		if (ERROR_SUCCESS != XInputSetState (player_index, &vib))
			return dseed::error_fail;
		return dseed::error_good;
	}
};

class accelerometer_win32 : public accelerometer_common
{
public:
	accelerometer_win32 (ISensor* sensor)
		: _sensor (sensor)
	{ }

public:
	virtual void update () noexcept override
	{
		CComPtr<ISensorDataReport> report;
		if (SUCCEEDED (_sensor->GetData (&report)))
		{
			PROPVARIANT px = {}, py = {}, pz = {};
			report->GetSensorValue (SENSOR_DATA_TYPE_ACCELERATION_X_G, &px);
			report->GetSensorValue (SENSOR_DATA_TYPE_ACCELERATION_Y_G, &py);
			report->GetSensorValue (SENSOR_DATA_TYPE_ACCELERATION_Z_G, &pz);

			_state[0].accelerometer = dseed::float3 ((float)px.dblVal, (float)py.dblVal, (float)pz.dblVal);
		}
		else _state[0].accelerometer = dseed::float3 ();
	}

private:
	CComPtr<ISensor> _sensor;
};

class gyroscope_win32 : public gyroscope_common
{
public:
	gyroscope_win32 (ISensor* sensor)
		: _sensor (sensor)
	{ }

public:
	virtual void update () noexcept override
	{
		CComPtr<ISensorDataReport> report;
		if (SUCCEEDED (_sensor->GetData (&report)))
		{
			PROPVARIANT px = {}, py = {}, pz = {};
			report->GetSensorValue (SENSOR_DATA_TYPE_ANGULAR_VELOCITY_X_DEGREES_PER_SECOND, &px);
			report->GetSensorValue (SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Y_DEGREES_PER_SECOND, &py);
			report->GetSensorValue (SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Z_DEGREES_PER_SECOND, &pz);

			_state[0].gyroscope = dseed::float3 ((float)px.dblVal, (float)py.dblVal, (float)pz.dblVal);
		}
		else _state[0].gyroscope = dseed::float3 ();
	}

private:
	CComPtr<ISensor> _sensor;
};

class gps_win32 : public gps_common
{
public:
	gps_win32 (ISensor* sensor)
		: _sensor (sensor)
	{ }

public:
	virtual void update () noexcept override
	{
		CComPtr<ISensorDataReport> report;
		if (SUCCEEDED (_sensor->GetData (&report)))
		{
			PROPVARIANT plg = {}, plt = {};
			report->GetSensorValue (SENSOR_DATA_TYPE_LONGITUDE_DEGREES, &plg);
			report->GetSensorValue (SENSOR_DATA_TYPE_LATITUDE_DEGREES, &plt);

			_state[0].longitude = plg.dblVal;
			_state[0].latitude = plt.dblVal;
		}
		else _state[0].longitude = _state[0].latitude = 0;
	}

private:
	CComPtr<ISensor> _sensor;
};

class __win32_application : public dseed::application
{
public:
	__win32_application ()
		: _refCount (1), _hWnd (nullptr), _handler (new dummy_handler ())
	{
		g_sharedApp = this;

		CComPtr<ISensorManager> sensorManager;
		CComPtr<ISensor> accelerometerSensor, gyroSensor, gpsSensor;
		if (SUCCEEDED (CoCreateInstance (CLSID_SensorManager, NULL, CLSCTX_INPROC_SERVER,
			__uuidof(ISensorManager), (void**) &sensorManager)))
		{
			CComPtr<ISensorCollection> sensors;
			ULONG count;

			if (SUCCEEDED (sensorManager->GetSensorsByType (SENSOR_TYPE_LOCATION_GPS, &sensors)))
			{
				if (SUCCEEDED (sensors->GetCount (&count)))
				{
					if (count > 0)
					{
						sensors->GetAt (0, &gpsSensor);
						gpsSensor->SetEventSink (nullptr);
					}
				}
				sensors = nullptr;
			}

			if (SUCCEEDED (sensorManager->GetSensorsByType (SENSOR_TYPE_ACCELEROMETER_3D, &sensors)))
			{
				if (SUCCEEDED (sensors->GetCount (&count)))
				{
					if (count > 0)
					{
						sensors->GetAt (0, &accelerometerSensor);
						accelerometerSensor->SetEventSink (nullptr);
					}
				}
				sensors = nullptr;
			}

			if (SUCCEEDED (sensorManager->GetSensorsByType (SENSOR_TYPE_GYROMETER_3D, &sensors)))
			{
				if (SUCCEEDED (sensors->GetCount (&count)))
				{
					if (count > 0)
					{
						sensors->GetAt (0, &gyroSensor);
						gyroSensor->SetEventSink (nullptr);
					}
				}
				sensors = nullptr;
			}
		}

		*&_inputDevices[dseed::inputdevice_keyboard] = new keyboard_common ();
		*&_inputDevices[dseed::inputdevice_mouse] = new mouse_win32 ();
		*&_inputDevices[dseed::inputdevice_gamepad] = new gamepad_win32 ();
		*&_inputDevices[dseed::inputdevice_touchpanel] = (GetSystemMetrics (SM_MAXIMUMTOUCHES) > 0) ? new touchpanel_common () : nullptr;
		*&_inputDevices[dseed::inputdevice_accelerometer] = (accelerometerSensor != nullptr) ? new accelerometer_win32 (accelerometerSensor) : nullptr;
		*&_inputDevices[dseed::inputdevice_gyroscope] = (gyroSensor != nullptr) ? new gyroscope_win32 (gyroSensor) : nullptr;
		*&_inputDevices[dseed::inputdevice_gps] = (gpsSensor != nullptr) ? new gps_win32 (gpsSensor) : nullptr;
	}
	~__win32_application ()
	{
		UnregisterClassW (WINDOW_CLASSNAME, GetModuleHandle (nullptr));
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

		if (type > dseed::inputdevice_gps || type < dseed::inputdevice_keyboard)
			return dseed::error_invalid_args;

		if (_inputDevices[(int)type] == nullptr)
			return dseed::error_not_support;

		*device = _inputDevices[(int)type];
		(*device)->retain ();

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
			* mode = dseed::windowmode_windowed;
		else if ((style & WINDOW_STYLE_SIZABLE_WINDOWED) == WINDOW_STYLE_SIZABLE_WINDOWED)
			* mode = dseed::windowmode_sizable_windowed;
		else if ((style & WINDOW_STYLE_FRAMELESS_FULLSCREEN) == WINDOW_STYLE_FRAMELESS_FULLSCREEN)
			* mode = dseed::windowmode_borderless_fullscreen;
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

		case dseed::windowmode_sizable_windowed:
			::SetWindowLong (_hWnd, GWL_STYLE, WINDOW_STYLE_SIZABLE_WINDOWED);
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
		WNDCLASSW wndClass =
		{
			NULL, __win32_application::WndProc, 0, 0, GetModuleHandle (nullptr),
			LoadIcon (NULL, IDI_APPLICATION), LoadCursor (NULL, IDC_ARROW),
			NULL, nullptr, WINDOW_CLASSNAME
		};
		if (RegisterClassW (&wndClass) == INVALID_ATOM)
			return dseed::error_fail;

		set_event_handler (handler);

		RECT clientRect = { 0, 0, 800, (int)(800 * (360.0f / 640)) };
		AdjustWindowRect (&clientRect, WINDOW_STYLE_WINDOWED, FALSE);

		int width = clientRect.right - clientRect.left,
			height = clientRect.bottom - clientRect.top,
			x = GetSystemMetrics (SM_CXSCREEN) / 2 - width / 2,
			y = GetSystemMetrics (SM_CYSCREEN) / 2 - height / 2;

		_hWnd = CreateWindowW (WINDOW_CLASSNAME, L"DSeed Window", WINDOW_STYLE_SIZABLE_WINDOWED,
			x, y, width, height, nullptr, nullptr, GetModuleHandle (nullptr), nullptr);

		if (_hWnd == nullptr)
			return dseed::error_fail;

		if (!update_rawinput (_hWnd, false))
			return dseed::error_fail;

		::ShowWindow (_hWnd, SW_SHOW);

		MSG msg; bool running = true;
		dseed::timespan_t lastTime = dseed::timespan_t::current_ticks (), currentTime;
		do
		{
			while (::PeekMessageW (&msg, nullptr, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					running = false;
					break;
				}

				::TranslateMessage (&msg);
				::DispatchMessageW (&msg);
			}

			for (dseed::auto_object<inputdevice_internal>& inputdevice : _inputDevices)
				if(inputdevice != nullptr)
					inputdevice->update ();

			currentTime = dseed::timespan_t::current_ticks ();
			_handler->next_frame (currentTime - lastTime);

			for (dseed::auto_object<inputdevice_internal>& inputdevice : _inputDevices)
				if (inputdevice != nullptr)
					inputdevice->cleanup ();

			lastTime = currentTime;

			::Sleep (0);
		} while (running);

		return dseed::error_good;
	}

private:
	static LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		dseed::auto_object<__win32_application> app;
		dseed::application::shared_app (reinterpret_cast<application * *> (&app));

		switch (uMsg)
		{
		case WM_SHOWWINDOW:
			if (wParam == TRUE)
				app->_handler->shown ();
			break;

		case WM_CLOSE:
		{
			bool cancel = false;
			app->_handler->closing (cancel);
			if (cancel)
				return 0;
			return DefWindowProcW (hWnd, uMsg, wParam, lParam);
		}
		break;

		case WM_DESTROY:
			app->_handler->closed ();
			PostQuitMessage (0);
			break;

		case WM_ACTIVATE:
		{
			if (wParam == WA_INACTIVE)
				app->_handler->deactivated ();
			else
				app->_handler->activated ();
		}
		break;

		case WM_SIZE:
			app->_handler->resized ();
			break;

		case WM_INPUT:
		{
			dseed::auto_object<keyboard_common> keyboard;
			app->input_device (dseed::inputdevice_keyboard, reinterpret_cast<dseed::inputdevice * *>(&keyboard));
			dseed::auto_object<mouse_win32> mouse;
			app->input_device (dseed::inputdevice_mouse, reinterpret_cast<dseed::inputdevice * *>(&mouse));

			UINT dwSize = 0;
			GetRawInputData ((HRAWINPUT)lParam, RID_INPUT, nullptr, &dwSize, sizeof (RAWINPUTHEADER));

			std::shared_ptr<BYTE[]> lpb (new BYTE[dwSize]);
			RAWINPUT* raw = (RAWINPUT*)& lpb[0];
			ZeroMemory (raw, dwSize);

			if (raw->header.dwType == RIM_TYPEMOUSE)
				raw->data.mouse.usFlags = mouse->_state[0].is_relative ? MOUSE_MOVE_RELATIVE : (MOUSE_MOVE_ABSOLUTE | MOUSE_VIRTUAL_DESKTOP);

			if (GetRawInputData ((HRAWINPUT)lParam, RID_INPUT, &lpb[0], &dwSize, sizeof (RAWINPUTHEADER)) != dwSize)
				break;

			if (raw->header.dwType == RIM_TYPEKEYBOARD)
				keyboard->_state[0].key_states[raw->data.keyboard.VKey] = ((raw->data.keyboard.Flags & RI_KEY_BREAK) == 0);
			else if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				POINT p = { raw->data.mouse.lLastX, raw->data.mouse.lLastY };
				if (!mouse->_state[0].is_relative)
					ScreenToClient (hWnd, &p);
				mouse->_state[0].position = dseed::point2i (p.x, p.y);

				int8_t buttons = mouse->_state[0].buttons;
				if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)	buttons |= dseed::mousebuttons_left;
				if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)	buttons &= ~dseed::mousebuttons_left;
				if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)	buttons |= dseed::mousebuttons_right;
				if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)	buttons &= ~dseed::mousebuttons_right;
				if (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)buttons |= dseed::mousebuttons_middle;
				if (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)	buttons &= ~dseed::mousebuttons_middle;
				if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)		buttons |= dseed::mousebuttons_x_button1;
				if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP)		buttons &= ~dseed::mousebuttons_x_button1;
				if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)		buttons |= dseed::mousebuttons_x_button2;
				if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP)		buttons &= ~dseed::mousebuttons_x_button2;
				mouse->_state[0].buttons = (dseed::mousebuttons_t)buttons;

				if (raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
					mouse->_state[0].wheel = dseed::float2 (0, raw->data.mouse.usButtonData / (float)WHEEL_DELTA);
				if (raw->data.mouse.usButtonFlags & RI_MOUSE_HWHEEL)
					mouse->_state[0].wheel = dseed::float2 (raw->data.mouse.usButtonData / (float)WHEEL_DELTA, 0);
			}
			else
				DefRawInputProc (&raw, dwSize, sizeof (RAWINPUTHEADER));
		}
		break;

		case WM_TOUCH:
		{
			dseed::auto_object<touchpanel_common> touchpanel;
			if (dseed::failed (app->input_device (dseed::inputdevice_touchpanel, reinterpret_cast<dseed::inputdevice * *>(&touchpanel)))
				|| touchpanel == nullptr)
				return DefWindowProcW (hWnd, uMsg, wParam, lParam);
			
			UINT touchInputsCount = LOWORD (wParam);
			std::shared_ptr<TOUCHINPUT[]> touchInputs (new TOUCHINPUT[touchInputsCount]);
			if (GetTouchInputInfo ((HTOUCHINPUT)lParam, touchInputsCount, &touchInputs[0], sizeof (TOUCHINPUT)))
			{
				for (UINT i = 0; i < touchInputsCount; ++i)
				{
					TOUCHINPUT& ti = touchInputs[i];
					if (ti.dwFlags & TOUCHEVENTF_DOWN)
					{
						dseed::touchpointer p = { (int32_t)ti.dwID, dseed::point2i (ti.x, ti.y), dseed::touchstate_pressed };
						touchpanel->_state[0].pointers[touchpanel->_state[0].pointerCount++] = p;
					}
					else
					{
						auto p = touchpanel->pointer (ti.dwID);
						if (p)
						{
							p->state = (ti.dwFlags & TOUCHEVENTF_UP) ? dseed::touchstate_released : dseed::touchstate_moved;
							p->position = dseed::point2i (ti.x, ti.y);
						}
					}
				}
				CloseTouchInputHandle ((HTOUCHINPUT)lParam);
			}
			else return DefWindowProcW (hWnd, uMsg, wParam, lParam);
		}
		break;

		default: return DefWindowProcW (hWnd, uMsg, wParam, lParam);
		}

		return 0;
	}

private:
	std::atomic<int32_t> _refCount;

	HWND _hWnd;
	dseed::auto_object<inputdevice_internal> _inputDevices[8];

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
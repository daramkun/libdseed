#include <dseed.h>

extern static dseed::application* g_sharedApp;

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
	virtual dseed::error_t update () = 0;
	virtual dseed::error_t cleanup () = 0;

private:
	std::atomic<int32_t> _refCount;
};

template<typename TState, dseed::inputdevice_t device_type, size_t maximum_player_index = 1>
class inputdevice_internal_template1 : public inputdevice_internal
{
public:
	inputdevice_internal_template1 () { memset (state, 0, sizeof (state)); }

public:
	virtual inputdevice_t type () override { return device_type; }

public:
	virtual dseed::error_t state (void* state, size_t state_size, int32_t player_index = 0) override
	{
		if ((player_index < 0 || player_index > maximum_player_index)
			|| state_size != sizeof (TState) || state == nullptr)
			return dseed::error_invalid_args;
		*(reinterpret_cast<TState*> (state)) = this->state[player_index];
		return dseed::error_good;
	}

	virtual error_t send_feedback (const void* feedback, size_t feedback_size, int32_t player_index = 0) override { return KB_NOT_SUPPORT; }

public:
	TState state[maximum_player_index];
};

template<typename TState, typename TFeedback, dseed::inputdevice_t device_type, size_t maximum_player_index = 1>
class inputdevice_internal_template2 : public inputdevice_internal_template1<TState, device_type, maximum_player_index>
{
public:
	virtual error_t send_feedback (const void* feedback, size_t feedbackSize, int32_t playerIndex = 0) override
	{
		if (playerIndex < 0 || playerIndex > maximum_player_index
			|| feedbackSize != sizeof (TFeedback) || feedback == nullptr)
			return dseed::error_invalid_args;
		return do_feedback (reinterpret_cast<const TFeedback*> (feedback), playerIndex);
	}

private:
	virtual error_t do_feedback (const TFeedback* feedback, int32_t player_index) = 0;
};

class keyboard_common : public inputdevice_internal_template1<dseed::keyboard_state, dseed::inputdevice_keyboard> { };
class mouse_common : public inputdevice_internal_template2<dseed::mouse_state, dseed::mouse_feedback, dseed::inputdevice_mouse>
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
	virtual void cleanup () override
	{
		if (state[0].pointerCount == 0)
			return;

		std::vector<touchpointer> pointers (&state[0].pointers[0], &state[0].pointers[state[0].pointerCount]);
		std::vector<std::vector<touchpointer>::iterator> removeList;
		for (auto i = pointers.begin (); i != pointers.end (); ++i)
		{
			touchpointer& p = *i;
			if (p.state == dseed::touchstate_pressed)
				p.state = dseed::touchstate_moved;
			else if (p.state == dseed::touchstate_released)
				removeList.push_back (i);
		}

		for (auto i : removeList)
			pointers.erase (i);

		std::copy (pointers.begin (), pointers.end (), state[0].pointers);
		memset (&state[0].pointers[pointers.size ()], 0, sizeof (dseed::touchpointer) * (256 - pointers.size () - 1));
		state[0].pointerCount = pointers.size ();
	}

public:
	dseed::touchpointer* pointer (int32_t pid)
	{
		for (int i = 0; i < state[0].pointerCount; ++i)
			if (state[0].pointers[i].pointerId == pid)
				return &(state[0].pointers[i]);
		return nullptr;
	}
};
class accelerometer_common : public inputdevice_internal_template1<dseed::accelerometer_state, dseed::inputdevice_accelerometer> { };
class gyroscope_common : public inputdevice_internal_template1<dseed::gyroscope_state, dseed::inputdevice_gyroscope> { };

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

#include <XInput.h>

class mouse_win32 : public mouse_common
{
public:
	virtual dseed::error_t do_feedback (const dseed::mouse_feedback* feedback, int32_t player_index) override
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
		if (state->is_relative != feedback->set_relative)
		{
			if (feedback->set_relative)
			{
				update_rawinput (hWnd, true);
			}
			else
			{
				update_rawinput (hWnd, false);
			}
			state->is_relative = feedback->set_relative;
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
	virtual void update () override
	{
		for (int i = 0; i < 8; ++i)
		{
			XINPUT_STATE s;
			_deviceConnected[i] = ::XInputGetState (i, &s);
			if (_deviceConnected[i] == ERROR_SUCCESS)
			{
				state[i].left_thumbstick = kkaebi::float2 (
					s.Gamepad.sThumbLX / 32767.0f, s.Gamepad.sThumbLY / 32767.0f
				);
				state[i].right_thumbstick = kkaebi::float2 (
					s.Gamepad.sThumbRX / 32767.0f, s.Gamepad.sThumbRY / 32767.0f
				);
				state[i].left_trigger = s.Gamepad.bLeftTrigger / 255.0f;
				state[i].right_trigger = s.Gamepad.bRightTrigger / 255.0f;

				int32_t buttons = 0;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_A) buttons |= kkaebi::input::gamepadbuttons_t_a;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_B) buttons |= kkaebi::input::gamepadbuttons_t_b;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_X) buttons |= kkaebi::input::gamepadbuttons_t_x;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_Y) buttons |= kkaebi::input::gamepadbuttons_t_y;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) buttons |= kkaebi::input::gamepadbuttons_t_dpad_left;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) buttons |= kkaebi::input::gamepadbuttons_t_dpad_right;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) buttons |= kkaebi::input::gamepadbuttons_t_dpad_up;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) buttons |= kkaebi::input::gamepadbuttons_t_dpad_down;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) buttons |= kkaebi::input::gamepadbuttons_t_left_bumper;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) buttons |= kkaebi::input::gamepadbuttons_t_right_thumbstick;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) buttons |= kkaebi::input::gamepadbuttons_t_left_thumbstick;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) buttons |= kkaebi::input::gamepadbuttons_t_right_thumbstick;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) buttons |= kkaebi::input::gamepadbuttons_t_back;
				if (s.Gamepad.wButtons & XINPUT_GAMEPAD_START) buttons |= kkaebi::input::gamepadbuttons_t_start;
				state[i].buttons = (kkaebi::input::gamepadbuttons_t) buttons;
			}
		}
	}

public:
	virtual dseed::error_t do_feedback (const dseed::gamepad_feedback* feedback, int32_t player_index) override
	{
		XINPUT_VIBRATION vib;
		vib.wLeftMotorSpeed = (WORD)(feedback->left_motor_speed * 65535);
		vib.wRightMotorSpeed = (WORD)(feedback->right_motor_speed * 65535);
		if (ERROR_SUCCESS != XInputSetState (player_index, &vib))
			return dseed::error_fail;
		return dseed::error_good;
	}

private:
	DWORD _deviceConnected[8];
};

class __win32_application : public dseed::application
{
public:
	__win32_application ()
		: _refCount (1), _hWnd (nullptr), _handler (new dummy_handler ())
	{
		*&_inputDevices[dseed::inputdevice_keyboard] = new keyboard_common ();
		*&_inputDevices[dseed::inputdevice_mouse] = new mouse_win32 ();
		*&_inputDevices[dseed::inputdevice_gamepad] = new gamepad_win32 ();
		*&_inputDevices[dseed::inputdevice_touchpanel] = new touchpanel_common ();
		*&_inputDevices[dseed::inputdevice_accelerometer] = new accelerometer_common ();
		*&_inputDevices[dseed::inputdevice_gyroscope] = new gyroscope_common ();
		*&_inputDevices[dseed::inputdevice_gps] = nullptr;
		*&_inputDevices[dseed::inputdevice_headtracker] = nullptr;
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
			* mode = dseed::windowmode_windowed;
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
			return DefWindowProc (hWnd, uMsg, wParam, lParam);
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
				raw->data.mouse.usFlags = mouse->state[0].is_relative ? MOUSE_MOVE_RELATIVE : (MOUSE_MOVE_ABSOLUTE | MOUSE_VIRTUAL_DESKTOP);

			if (GetRawInputData ((HRAWINPUT)lParam, RID_INPUT, &lpb[0], &dwSize, sizeof (RAWINPUTHEADER)) != dwSize)
				break;

			if (raw->header.dwType == RIM_TYPEKEYBOARD)
				keyboard->state[0].key_states[raw->data.keyboard.VKey] = ((raw->data.keyboard.Flags & RI_KEY_BREAK) == 0);
			else if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				POINT p = { raw->data.mouse.lLastX, raw->data.mouse.lLastY };
				if (!mouse->state[0].is_relative)
					ScreenToClient (hWnd, &p);
				mouse->state[0].position = dseed::point2i (p.x, p.y);

				int8_t buttons = mouse->state[0].buttons;
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
				mouse->state[0].buttons = (dseed::mousebuttons_t)buttons;

				if (raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
					mouse->state[0].wheel = dseed::float2 (0, raw->data.mouse.usButtonData / (float)WHEEL_DELTA);
				if (raw->data.mouse.usButtonFlags & RI_MOUSE_HWHEEL)
					mouse->state[0].wheel = dseed::float2 (raw->data.mouse.usButtonData / (float)WHEEL_DELTA, 0);
			}
			else
				DefRawInputProc (&raw, dwSize, sizeof (RAWINPUTHEADER));
		}
		break;

		case WM_TOUCH:
		{
			UINT touchInputsCount = LOWORD (wParam);
			std::shared_ptr<TOUCHINPUT[]> touchInputs (new TOUCHINPUT[touchInputsCount]);
			if (GetTouchInputInfo ((HTOUCHINPUT)lParam, touchInputsCount, &touchInputs[0], sizeof (TOUCHINPUT)))
			{
				dseed::auto_object<touchpanel_common> touchpanel;
				app->input_device (dseed::inputdevice_touchpanel, reinterpret_cast<dseed::inputdevice * *>(&touchpanel));

				for (UINT i = 0; i < touchInputsCount; ++i)
				{
					TOUCHINPUT& ti = touchInputs[i];
					if (ti.dwFlags & TOUCHEVENTF_DOWN)
					{
						dseed::touchpointer p = { (int32_t)ti.dwID, dseed::point2i (ti.x, ti.y), dseed::touchstate_pressed };
						touchpanel->state[0].pointers[touchpanel->state[0].pointerCount++] = p;
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
			else return DefWindowProc (hWnd, uMsg, wParam, lParam);
		}
		break;

		default: return DefWindowProc (hWnd, uMsg, wParam, lParam);
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
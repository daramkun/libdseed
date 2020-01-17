#ifndef __DSEED_PLATFORM_COMMON_HXX__
#define __DSEED_PLATFORM_COMMON_HXX__

extern dseed::platform::application* g_sharedApp;

class dummy_handler : public dseed::platform::event_handler
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
	virtual void next_frame (dseed::timespan delta) override { }

private:
	std::atomic<int32_t> _refCount;
};

class inputdevice_internal : public dseed::io::inputdevice
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

template<typename TState, dseed::io::inputdevicetype device_type, size_t maximum_player_index = 1, bool device_conn_default = false>
class inputdevice_internal_template1 : public inputdevice_internal
{
public:
	inputdevice_internal_template1 ()
	{
		memset (_state, 0, sizeof (_state));
		memset (_deviceConnected, device_conn_default, sizeof (_deviceConnected));
	}

public:
	virtual dseed::io::inputdevicetype type () override { return device_type; }

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

template<typename TState, typename TFeedback, dseed::io::inputdevicetype device_type, size_t maximum_player_index = 1, bool device_conn_default = false>
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

class keyboard_common : public inputdevice_internal_template1<dseed::io::keyboard_state, dseed::io::inputdevicetype_keyboard, 1, true> { };
class mouse_common : public inputdevice_internal_template2<dseed::io::mouse_state, dseed::io::mouse_feedback, dseed::io::inputdevicetype_mouse, 1, true>
{
public:
	virtual dseed::error_t do_feedback (const dseed::io::mouse_feedback* feedback, int32_t player_index) noexcept
	{
		return dseed::error_not_impl;
	}
};
class gamepad_common : public inputdevice_internal_template2<dseed::io::gamepad_state, dseed::io::gamepad_feedback, dseed::io::inputdevicetype_gamepad, 8>
{
public:
	virtual dseed::error_t do_feedback (const dseed::io::gamepad_feedback* feedback, int32_t player_index) noexcept
	{
		return dseed::error_not_impl;
	}
};
class touchpanel_common : public inputdevice_internal_template1<dseed::io::touchpanel_state, dseed::io::inputdevicetype_touchpanel>
{
public:
	virtual void cleanup () noexcept override
	{
		if (_state[0].pointerCount == 0)
			return;

		std::vector<dseed::io::touchpointer> pointers (&_state[0].pointers[0], &_state[0].pointers[_state[0].pointerCount]);
		std::vector<std::vector<dseed::io::touchpointer>::iterator> removeList;
		for (auto i = pointers.begin (); i != pointers.end (); ++i)
		{
			dseed::io::touchpointer& p = *i;
			if (p.state == dseed::io::touchstate::pressed)
				p.state = dseed::io::touchstate::moved;
			else if (p.state == dseed::io::touchstate::released)
				removeList.push_back (i);
		}

		for (auto i : removeList)
			pointers.erase (i);

		std::copy (pointers.begin (), pointers.end (), _state[0].pointers);
		memset (&_state[0].pointers[pointers.size ()], 0, sizeof (dseed::io::touchpointer) * (256 - pointers.size () - 1));
		_state[0].pointerCount = pointers.size ();
	}

public:
	dseed::io::touchpointer* pointer (int32_t pid)
	{
		for (int i = 0; i < _state[0].pointerCount; ++i)
			if (_state[0].pointers[i].pointerId == pid)
				return &(_state[0].pointers[i]);
		return nullptr;
	}
};
class accelerometer_common : public inputdevice_internal_template1<dseed::io::accelerometer_state, dseed::io::inputdevicetype_accelerometer> { };
class gyroscope_common : public inputdevice_internal_template1<dseed::io::gyroscope_state, dseed::io::inputdevicetype_gyroscope> { };
class gps_common : public inputdevice_internal_template1<dseed::io::gps_state, dseed::io::inputdevicetype_gps> { };

#endif
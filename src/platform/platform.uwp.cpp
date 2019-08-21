#include <dseed.h>

#if PLATFORM_UWP

#	include "platform.internal.h"

namespace dseed
{
	namespace uwp
	{
		ref class DSeedFrameworkView : public Windows::ApplicationModel::Core::IFrameworkView
		{
		public:
			virtual void Initialize (Windows::ApplicationModel::Core::CoreApplicationView^ appView)
			{

			}

			virtual void SetWindow (Windows::UI::Core::CoreWindow^ window)
			{

			}

			virtual void Load (Platform::String^ entryPoint) { }

			virtual void Run ()
			{

			}

			virtual void Uninitialize ()
			{

			}

		public:

		};

		ref class DSeedFrameworkViewSource : public Windows::ApplicationModel::Core::IFrameworkViewSource
		{
		public:
			virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView ()
			{

			}
		};
	}
}

class __uwp_application : public dseed::application
{
public:
	__uwp_application ()
		: _refCount (1)
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
	virtual dseed::error_t input_device (dseed::inputdevice_t type, dseed::inputdevice** device) override
	{

	}

public:
	virtual dseed::error_t set_title (const char* title) override
	{

	}

public:
	virtual dseed::error_t client_size (dseed::size2i* size) override
	{

	}
	virtual dseed::error_t set_client_size (const dseed::size2i& size) override
	{

	}

public:
	virtual dseed::error_t window_mode (dseed::windowmode_t* mode) override
	{

	}
	virtual dseed::error_t set_window_mode (dseed::windowmode_t mode) override
	{

	}

public:
	virtual dseed::error_t event_handler (dseed::event_handler** handler) override
	{

	}
	virtual dseed::error_t set_event_handler (dseed::event_handler* handler) override
	{

	}

public:
	virtual dseed::error_t run (dseed::event_handler* handler) override
	{

	}

private:
	std::atomic<int32_t> _refCount;
};

dseed::error_t dseed::create_application (dseed::application** app)
{
	if (g_sharedApp != nullptr) return dseed::error_invalid_op;
	if (app == nullptr) return dseed::error_invalid_args;


}

#endif
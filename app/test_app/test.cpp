#include <dseed.h>

class own_handler : public dseed::event_handler
{
public:
	own_handler () : _refCount (1) { }

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
	virtual void activated () override
	{
	}

	virtual void deactivated () override
	{
	}

public:
	virtual void paused () override
	{
	}

	virtual void resumed () override
	{
	}

public:
	virtual void resized () override
	{
	}

public:
	virtual void shown () override
	{
		dseed::auto_object<dseed::application> app;
		dseed::application::shared_app (&app);

		app->set_title (u8"애플리케이션");

		if(dseed::failed(dseed::create_d3d11_vga_device (nullptr, &_vgaDevice)))
			app->exit ();
		if (dseed::failed (dseed::create_d3d11_vga_swapchain (app, _vgaDevice, &_vgaSwapChain)))
			app->exit ();
	}

	virtual void closing (bool& cancel) override
	{

	}

	virtual void closed () override
	{
	}

public:
	virtual void next_frame (dseed::timespan_t delta) override
	{


		_vgaSwapChain->present ();
	}

private:
	std::atomic<int32_t> _refCount;

	dseed::auto_object<dseed::vga_device> _vgaDevice;
	dseed::auto_object<dseed::vga_swapchain> _vgaSwapChain;
};

ENTRYPOINT_ATTRIBUTE
ENTRYPOINT_RETURNTYPE ENTRYPOINT_CALLTYPE ENTRYPOINT_NAME (ENTRYPOINT_ARGUMENTS)
{
	dseed::auto_object<dseed::application> app;
	if (dseed::failed (dseed::create_application (&app)))
		return -1;

	dseed::auto_object<dseed::event_handler> handler;
	*&handler = new own_handler ();

	if (dseed::failed (app->run (handler)))
		return -2;

	return 0;
}
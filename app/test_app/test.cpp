#include <dseed.h>

class own_handler : public dseed::platform::event_handler
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

	virtual void paused () override
	{
	}

	virtual void resumed () override
	{
	}

	virtual void resized () override
	{
	}

public:
	virtual void shown () override
	{
		dseed::autoref<dseed::platform::application> app;
		dseed::platform::application::shared_app (&app);

		if (dseed::failed (dseed::graphics::create_d3d11_vgadevice (app, nullptr, &vgaDevice)))
			app->exit ();
	}

	virtual void closing (bool& cancel) override
	{
	}

	virtual void closed () override
	{
	}

public:
	virtual void next_frame (dseed::timespan delta) override
	{
		if (vgaDevice == nullptr)
			return;

		dseed::autoref<dseed::graphics::sprite_render> spriteRender;
		vgaDevice->sprite_render ((dseed::graphics::vgarender**) & spriteRender);



		vgaDevice->present ();
	}

private:
	std::atomic<int32_t> _refCount;

	dseed::autoref<dseed::graphics::vgadevice> vgaDevice;
};

#include <d3d11.h>

ENTRYPOINT_ATTRIBUTE
ENTRYPOINT_RETURNTYPE ENTRYPOINT_CALLTYPE ENTRYPOINT_NAME (ENTRYPOINT_ARGUMENTS)
{
	{
		dseed::autoref<dseed::platform::application> app;
		if (dseed::failed (dseed::platform::create_application (&app)))
			return -1;

		dseed::autoref<dseed::platform::event_handler> handler;
		*&handler = new own_handler ();

		app->run (handler);
	}

	ENTRYPOINT_RETURN (0);
}
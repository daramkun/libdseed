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
		{
			app->exit ();
			return;
		}

		if (dseed::failed (vgaDevice->sprite_render ((dseed::graphics::vgarender**) & spriteRender)))
		{
			app->exit ();
			return;
		}

		dseed::autoref<dseed::io::stream> stream;
		if (dseed::failed (dseed::io::create_native_filestream (u8"../../../sample/bitmaps/sample5.gif", false, &stream)))
		{
			app->exit ();
			return;
		}

		dseed::autoref<dseed::bitmaps::bitmap_array> decoder;
		if (dseed::failed (dseed::bitmaps::detect_bitmap_decoder (stream, &decoder)))
		{
			app->exit ();
			return;
		}

		dseed::autoref<dseed::bitmaps::bitmap> bitmap;
		if (dseed::failed (decoder->at (0, &bitmap)))
		{
			app->exit ();
			return;
		}

		if (dseed::failed (spriteRender->create_atlas (bitmap, nullptr, 0, &spriteAtlas)))
		{
			app->exit ();
			return;
		}

		if (dseed::failed (spriteRender->create_pipeline (nullptr, nullptr, dseed::graphics::sprite_texfilter::nearest, &spritePipeline)))
		{
			app->exit ();
			return;
		}
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
		static dseed::f32x4 color (1, 1, 1, 1), clearColor (0, 0, 0, 1);

		if (vgaDevice == nullptr)
			return;

		spriteRender->clear_rendertarget (nullptr, clearColor);

		dseed::float4x4 transform = dseed::float4x4::identity ();
		spriteRender->begin (dseed::graphics::rendermethod::forward, transform);

		dseed::graphics::sprite_rendertarget* renderTarget = nullptr;
		spriteRender->set_rendertarget (&renderTarget, 1);
		spriteRender->set_atlas (&spriteAtlas, 1);
		spriteRender->set_pipeline (spritePipeline);
		
		transform = dseed::float4x4::translate (800 / 2, 450 / 2, 0);
		spriteRender->draw (0, transform, color);

		spriteRender->end ();

		vgaDevice->present ();
	}

private:
	std::atomic<int32_t> _refCount;

	dseed::autoref<dseed::graphics::vgadevice> vgaDevice;
	dseed::autoref<dseed::graphics::sprite_render> spriteRender;
	dseed::autoref<dseed::graphics::sprite_atlas> spriteAtlas;
	dseed::autoref<dseed::graphics::pipeline> spritePipeline;
};

#include <d3d11.h>

ENTRYPOINT_ATTRIBUTE
ENTRYPOINT_RETURNTYPE ENTRYPOINT_CALLTYPE ENTRYPOINT_NAME (ENTRYPOINT_ARGUMENTS)
{
	dseed::autoref<dseed::platform::application> app;
	if (dseed::failed (dseed::platform::create_application (&app)))
		return -1;

	dseed::autoref<dseed::platform::event_handler> handler;
	*&handler = new own_handler ();

	app->run (handler);

	ENTRYPOINT_RETURN (0);
}
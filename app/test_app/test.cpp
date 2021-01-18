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
		if (vgaDevice != nullptr)
		{
			dseed::autoref<dseed::platform::application> app;
			dseed::platform::application::shared_app(&app);

			dseed::graphics::displaymode dm = {};
			app->client_size(&dm.resolution);
			dm.format = dseed::color::pixelformat::bgra8;
			vgaDevice->set_displaymode(&dm, false);
		}
	}

public:
	virtual void shown () override
	{
		dseed::autoref<dseed::platform::application> app;
		dseed::platform::application::shared_app (&app);
		app->set_client_size (dseed::size2i (1280, 720));

		if (dseed::failed (dseed::graphics::create_d3d11_vgadevice (app, nullptr, &vgaDevice)))
		{
			app->exit ();
			return;
		}
		//vgaDevice->set_vsync (true);
		
		if (dseed::failed (vgaDevice->sprite_render ((dseed::graphics::vgarender**) & spriteRender)))
		{
			app->exit ();
			return;
		}

		dseed::autoref<dseed::io::stream> stream;
		if (dseed::failed (dseed::io::create_native_filestream (u8"../../../sample/bitmaps/sample3.png", false, &stream)))
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

		if (dseed::failed (spriteRender->create_pipeline (nullptr, nullptr, dseed::graphics::sprite_texfilter::linear, &spritePipeline)))
		{
			app->exit ();
			return;
		}

		dseed::autoref<dseed::io::stream> fileStream;
		if (dseed::failed (dseed::io::create_native_filestream (u8"../../../sample/audios/sample1.mp3", false, &fileStream)))
		{
			app->exit ();
			return;
		}

		dseed::autoref<dseed::media::media_decoder> audioDecoder;
		if (dseed::failed (dseed::media::detect_media_decoder (fileStream, &audioDecoder)))
		{
			app->exit ();
			return;
		}

		dseed::autoref<dseed::media::audio_stream> audioStream;
		if (dseed::failed (dseed::media::create_audio_buffered_stream (audioDecoder, &audioStream)))
		{
			app->exit ();
			return;
		}

		if (dseed::failed (dseed::audio::create_xaudio2_audioplayer (nullptr, &audioPlayer)))
		{
			app->exit ();
			return;
		}

		if (dseed::failed (audioPlayer->create_backgroundaudio (audioStream, &bgm)))
		{
			app->exit ();
			return;
		}

		if (dseed::failed(bgm->play()))
		{
			app->exit();
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
		static dseed::f32x4_t color (1, 1, 1, 1), clearColor (0, 0, 0, 1);

		if (vgaDevice == nullptr)
			return;

		_frameMeasurer.update (delta);

		dseed::autoref<dseed::platform::application> app;
		dseed::platform::application::shared_app (&app);
		char title[256];
		sprintf (title, "FPS: %lf", _frameMeasurer.fps ());
		app->set_title (title);
		dseed::size2i clientSize;
		app->client_size(&clientSize);

		spriteRender->clear_rendertarget (nullptr, clearColor);

		dseed::float4x4 transform = dseed::float4x4::identity ();
		spriteRender->begin (dseed::graphics::rendermethod::deferred, transform);

		dseed::graphics::sprite_rendertarget* renderTarget = nullptr;
		spriteRender->set_rendertarget (&renderTarget, 1);
		spriteRender->set_atlas (&spriteAtlas, 1);
		spriteRender->set_pipeline (spritePipeline);
		
		for (int y = -2; y <= 2; ++y)
		{
			for (int x = -5; x <= 5; ++x)
			{
				transform = dseed::multiply ((dseed::f32x4x4_t)dseed::float4x4::scale (0.25f, 0.25f, 1), 
					(dseed::f32x4x4_t)dseed::float4x4::translate (clientSize.width / 2 + (x * 150), clientSize.height / 2 + (y * 150), 0));
				spriteRender->draw (0, transform, color);
			}
		}

		spriteRender->end ();

		vgaDevice->present ();
	}

private:
	std::atomic<int32_t> _refCount;
	dseed::framemeasurer _frameMeasurer;

	dseed::autoref<dseed::graphics::vgadevice> vgaDevice;
	dseed::autoref<dseed::graphics::sprite_render> spriteRender;
	dseed::autoref<dseed::graphics::sprite_atlas> spriteAtlas;
	dseed::autoref<dseed::graphics::pipeline> spritePipeline;

	dseed::autoref<dseed::audio::audioplayer> audioPlayer;
	dseed::autoref<dseed::audio::backgroundaudio> bgm;
};

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
#ifndef __DSEED_GRAPHICS_H__
#define __DSEED_GRAPHICS_H__

namespace dseed
{
	struct DSEEDEXP displaymode
	{
		size2i resolution;
		pixelformat format;
		fraction refresh_rate;
	};

	class DSEEDEXP display : public object, public wrapped
	{
	public:
		virtual error_t name (char* name, size_t maxNameCount) = 0;

	public:
		virtual error_t displaymode (int index, displaymode* mode) = 0;
		virtual size_t displaymode_count () = 0;

	public:
		virtual error_t refresh () = 0;
	};

	class DSEEDEXP vga_adapter : public object, public wrapped
	{
	public:
		virtual error_t name (char* name, size_t maxNameCount) = 0;

	public:
		virtual error_t display (int index, display** display) = 0;
		virtual size_t display_count () = 0;

	public:
		virtual error_t refresh () = 0;
	};

	class DSEEDEXP vga_adapter_enumerator : public object
	{
	public:
		virtual error_t vga_adapter (int index, vga_adapter** adapter) = 0;
		virtual size_t vga_adapter_count () = 0;

	public:
		virtual error_t refresh () = 0;
	};

	class DSEEDEXP texture : public object, public wrapped
	{
	public:
		virtual dseed::pixelformat format () = 0;
		virtual size_t mip_level () = 0;
	};

	class DSEEDEXP texture1d : public texture
	{
	public:
		virtual int32_t size () = 0;
	};

	class DSEEDEXP texture2d : public texture
	{
	public:
		virtual dseed::size2i size () = 0;
	};

	class DSEEDEXP texture3d : public texture
	{
	public:
		virtual dseed::size3i size () = 0;
	};

	class DSEEDEXP texturecube : public texture
	{
		virtual dseed::size2i size () = 0;
	};

	class DSEEDEXP rendertarget2d : public object, public wrapped
	{
	public:
		virtual error_t texture (texture2d** texture) = 0;
	};

	class DSEEDEXP depthstencil : public object, public wrapped
	{
	public:
		virtual error_t texture (texture2d** texture) = 0;
	};

	class DSEEDEXP vga_commandlist : public object, public wrapped
	{
	public:

	};

	class DSEEDEXP vga_commandqueue : public object, public wrapped
	{
	public:
		virtual error_t make_commandlist (vga_commandlist** list) = 0;

	public:

	};

	class DSEEDEXP vga_device : public object, public wrapped
	{
	public:
		virtual error_t adapter (vga_adapter** adapter) = 0;

	public:
		virtual bool is_support_format (pixelformat pf) = 0;
		virtual bool is_support_parallel_render () = 0;

	public:
		virtual error_t create_commandqueue (vga_commandqueue** commandqueue) = 0;

	public:
		virtual error_t do_commandlist (vga_commandlist* commandlist) = 0;

	public:
		virtual error_t create_texture2d (dseed::size2i size, dseed::pixelformat pf, int mipLevel, texture2d** texture) = 0;
		virtual error_t create_texture2d (dseed::bitmap* bitmap, texture2d** texture) = 0;
		virtual error_t create_texture2d (dseed::bitmap_decoder* decoder, texture2d** texture) = 0;

	public:
		virtual error_t create_rendertarget (texture2d* texture, rendertarget2d** rt) = 0;
		virtual error_t create_depthstencil (texture2d* texture, depthstencil** ds) = 0;

	public:
		virtual error_t copy_resource (texture2d* texture, dseed::bitmap* copy_to, int mipLevel = 0) = 0;
		virtual error_t copy_resource (dseed::bitmap* bmp, texture2d* copy_to, int mipLevel = 0) = 0;
	};

	class DSEEDEXP vga_swapchain : public object, public wrapped
	{
	public:
		virtual error_t backbuffer (texture2d** texture) = 0;

	public:
		virtual bool vsync () = 0;
		virtual error_t set_vsync (bool vsync) = 0;

	public:
		virtual error_t present () = 0;
	};
}

#include "graphics.dxgi.h"
#include "graphics.d3d11.h"
#include "graphics.vk.h"

#endif
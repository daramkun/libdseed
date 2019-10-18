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

	class DSEEDEXP vga_device : public object, public wrapped
	{
	public:
		virtual error_t adapter (vga_adapter** adapter) = 0;

	public:
		virtual bool is_support_format (pixelformat pf) = 0;
		virtual bool is_support_parallel_render () = 0;
	};

	class DSEEDEXP vga_swapchain : public object, public wrapped
	{
	public:
		virtual error_t present () = 0;
	};
}

#include "graphics.dxgi.h"
#include "graphics.d3d11.h"

#endif
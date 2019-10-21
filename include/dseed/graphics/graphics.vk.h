#ifndef __DSEED_GRAPHICS_VULKAN_H__
#define __DSEED_GRAPHICS_VULKAN_H__

namespace dseed
{
	DSEEDEXP error_t create_vulkan_vga_adapter_enumerator (vga_adapter_enumerator** e);
	DSEEDEXP error_t create_vulkan_vga_device (vga_adapter* adapter, vga_device** device);
}

#endif
#ifndef __MICROSOFT_ICO_CUR_COMMON_HXX__
#define __MICROSOFT_ICO_CUR_COMMON_HXX__

#define ICO_CUR_IMAGE_TYPE_ICON								1
#define ICO_CUR_IMAGE_TYPE_CURSOR							2

#if COMPILER_MSVC
#	pragma pack (push, 1)
#else
#	pragma pack (1)
#endif
struct IcoCurHeader
{
	uint16_t reserved;
	uint16_t image_type;
	uint16_t images;
};

struct IcoCurEntry
{
	uint8_t width;
	uint8_t height;
	uint8_t palettes;
	uint8_t reserved;
	union {
		uint16_t ico_planes;
		uint16_t cur_hotspot_x;
	};
	union {
		uint16_t ico_bits_per_pixels;
		uint16_t cur_hotspot_y;
	};
	uint32_t bytes;
	uint32_t offset;
};
#if COMPILER_MSVC
#	pragma pack (pop)
#else
#	pragma pack ()
#endif

#endif
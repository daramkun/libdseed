#include <dseed.h>

#include <map>

dseed::bitmap_filter_mask::bitmap_filter_mask (float* mask, size_t width, size_t height)
	: width (width), height (height)
{
	memcpy (this->mask, mask, width * height * sizeof (float));
}

dseed::bitmap_filter_mask::bitmap_filter_mask (float mask, size_t width, size_t height)
	: width (width), height (height)
{
	for (size_t i = 0; i < width * height; ++i)
		this->mask[i] = mask;
}

void dseed::bitmap_filter_mask::identity_3 (bitmap_filter_mask* mask)
{
	static float __identity[] = { 
		0, 0, 0,
		0, 1, 0,
		0, 0, 0
	};

	if (mask == nullptr) return;
	*mask = bitmap_filter_mask (__identity, 3, 3);
}

void dseed::bitmap_filter_mask::edge_detection_3 (bitmap_filter_mask* mask)
{
	static float __identity[] = {
		-1, -1, -1,
		-1, +8, -1, 
		-1, -1, -1
	};

	if (mask == nullptr) return;
	*mask = bitmap_filter_mask (__identity, 3, 3);
}

void dseed::bitmap_filter_mask::sharpen_3 (bitmap_filter_mask* mask)
{
	static float __identity[] = {
		+0, -1, +0,
		-1, +5, -1,
		+0, -1, +0
	};

	if (mask == nullptr) return;
	*mask = bitmap_filter_mask (__identity, 3, 3);
}

void dseed::bitmap_filter_mask::gaussian_blur_3 (bitmap_filter_mask* mask)
{
	static float __identity[] = {
		+1, +2, +1,
		+2, +4, +2,
		+1, +2, +1
	};
	static float __factor = 1 / 16.0f;

	if (mask == nullptr) return;
	*mask = bitmap_filter_mask (__identity, 3, 3);
	mask->operator*=(__factor);
}

void dseed::bitmap_filter_mask::gaussian_blur_5 (bitmap_filter_mask* mask)
{
	static float __identity[] = {
		1,  4,  6,  4, 1,
		4, 16, 24, 16, 4,
		6, 24, 36, 24, 6,
		4, 16, 24, 16, 4,
		1,  4,  6,  4, 1
	};
	static float __factor = 1 / 256.0f;

	if (mask == nullptr) return;
	*mask = bitmap_filter_mask (__identity, 5, 5);
	mask->operator*=(__factor);
}

void dseed::bitmap_filter_mask::unsharpmask_5 (bitmap_filter_mask* mask)
{
	static float __identity[] = {
		1,  4,    6,  4, 1,
		4, 16,   24, 16, 4,
		6, 24, -476, 24, 6,
		4, 16,   24, 16, 4,
		1,  4,    6,  4, 1
	};
	static float __factor = -1 / 256.0f;

	if (mask == nullptr) return;
	*mask = bitmap_filter_mask (__identity, 5, 5);
	mask->operator*=(__factor);
}

using ftfn = std::function<bool (uint8_t*, const uint8_t*, const dseed::size3i&, const dseed::bitmap_filter_mask&)>;

template<class TPixel>
inline bool filter_bitmap (uint8_t* dest, const uint8_t* src, const dseed::size3i& size, const dseed::bitmap_filter_mask& mask) noexcept
{
	size_t stride = dseed::get_bitmap_stride (dseed::type2format<TPixel> (), size.width);
	size_t depth = dseed::get_bitmap_plane_size (dseed::type2format<TPixel> (), size.width, size.height);

	for (size_t z = 0; z < size.depth; ++z)
	{
		size_t depthZ = z * depth;
		for (size_t y = 0; y < size.height; ++y)
		{
			size_t strideY = y * stride;
			TPixel* destPtr = (TPixel*)(dest + depthZ + strideY);

			for (size_t x = 0; x < size.width; ++x)
			{
				dseed::color4f sum;
				for (int fy = 0; fy < (int)mask.height; ++fy)
				{
					for (int fx = 0; fx < (int)mask.width; ++fx)
					{
						size_t cx = dseed::clamp<int> ((int)x + (fx - (int)mask.width / 2), size.width - 1);
						size_t cy = dseed::clamp<int> ((int)y + (fy - (int)mask.height / 2), size.height - 1);

						dseed::color4f color = *((TPixel*)(src + depthZ + (stride * cy)) + cx);
						color = color * mask.get_mask (fx, fy);
						sum += color;
					}
				}
				sum.restore_alpha (*((TPixel*)(src + depthZ + (strideY)) + x));

				TPixel* destPtrX = destPtr + x;
				*destPtrX = sum;
			}
		}
	}

	return true;
}

std::map<dseed::pixelformat, ftfn> g_resizes = {
	{ dseed::pixelformat::rgba8888, filter_bitmap<dseed::rgba> },
	{ dseed::pixelformat::rgb888, filter_bitmap<dseed::rgb> },
	{ dseed::pixelformat::rgbaf, filter_bitmap<dseed::rgbaf> },
	{ dseed::pixelformat::bgra8888, filter_bitmap<dseed::bgra> },
	{ dseed::pixelformat::bgr888, filter_bitmap<dseed::bgr> },
	{ dseed::pixelformat::bgra4444, filter_bitmap<dseed::bgra4> },
	{ dseed::pixelformat::bgr565, filter_bitmap<dseed::bgr565> },
	{ dseed::pixelformat::grayscale8, filter_bitmap<dseed::grayscale> },
	{ dseed::pixelformat::grayscalef, filter_bitmap<dseed::grayscalef> },
	{ dseed::pixelformat::yuva8888, filter_bitmap<dseed::yuva> },
	{ dseed::pixelformat::yuv888, filter_bitmap<dseed::yuv> },
	{ dseed::pixelformat::hsva8888, filter_bitmap<dseed::hsva> },
	{ dseed::pixelformat::hsv888, filter_bitmap<dseed::hsv> },
};

dseed::error_t dseed::filter_bitmap (dseed::bitmap* original, const dseed::bitmap_filter_mask& mask, dseed::bitmap** bitmap)
{
	if (original == nullptr || bitmap == nullptr)
		return dseed::error_invalid_args;
	if (mask.width % 2 == 0 || mask.height % 2 == 0)
		return dseed::error_invalid_args;

	dseed::auto_object<dseed::bitmap> temp;
	if (dseed::failed (dseed::create_bitmap (original->type (), original->size (), original->format (), nullptr, &temp)))
		return dseed::error_fail;

	uint8_t* destPtr, * srcPtr;
	original->pixels_pointer ((void**)& srcPtr);
	temp->pixels_pointer ((void**)& destPtr);

	auto found = g_resizes.find (original->format ());
	if (found == g_resizes.end ())
		return dseed::error_not_support;

	if (!found->second (destPtr, srcPtr, original->size (), mask))
		return dseed::error_not_support;

	*bitmap = temp.detach ();

	return dseed::error_good;
}
#include <dseed.h>

#include <map>

using namespace dseed::color;

dseed::bitmaps::bitmap_filter_mask::bitmap_filter_mask (float* mask, size_t width, size_t height)
	: width (width), height (height)
{
	memcpy (this->mask, mask, width * height * sizeof (float));
}

dseed::bitmaps::bitmap_filter_mask::bitmap_filter_mask (float mask, size_t width, size_t height)
	: width (width), height (height)
{
	for (size_t i = 0; i < width * height; ++i)
		this->mask[i] = mask;
}

void dseed::bitmaps::bitmap_filter_mask::identity_3 (bitmap_filter_mask* mask)
{
	static float __identity[] = {
		0, 0, 0,
		0, 1, 0,
		0, 0, 0
	};

	if (mask == nullptr) return;
	*mask = bitmap_filter_mask (__identity, 3, 3);
}

void dseed::bitmaps::bitmap_filter_mask::edge_detection_3 (bitmap_filter_mask* mask)
{
	static float __identity[] = {
		-1, -1, -1,
		-1, +8, -1,
		-1, -1, -1
	};

	if (mask == nullptr) return;
	*mask = bitmap_filter_mask (__identity, 3, 3);
}

void dseed::bitmaps::bitmap_filter_mask::sharpen_3 (bitmap_filter_mask* mask)
{
	static float __identity[] = {
		+0, -1, +0,
		-1, +5, -1,
		+0, -1, +0
	};

	if (mask == nullptr) return;
	*mask = bitmap_filter_mask (__identity, 3, 3);
}

void dseed::bitmaps::bitmap_filter_mask::gaussian_blur_3 (bitmap_filter_mask* mask)
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

void dseed::bitmaps::bitmap_filter_mask::gaussian_blur_5 (bitmap_filter_mask* mask)
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

void dseed::bitmaps::bitmap_filter_mask::unsharpmask_5 (bitmap_filter_mask* mask)
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

using ftfn = std::function<bool (uint8_t*, const uint8_t*, const dseed::size3i&, const dseed::bitmaps::bitmap_filter_mask&)>;

template<class TPixel>
inline bool filter_bitmap (uint8_t* dest, const uint8_t* src, const dseed::size3i& size, const dseed::bitmaps::bitmap_filter_mask& mask) noexcept
{
	size_t stride = calc_bitmap_stride (type2format<TPixel> (), size.width);
	size_t depth = calc_bitmap_plane_size (type2format<TPixel> (), dseed::size2i (size.width, size.height));

	for (size_t z = 0; z < size.depth; ++z)
	{
		size_t depthZ = z * depth;
		for (size_t y = 0; y < size.height; ++y)
		{
			size_t strideY = y * stride;
			TPixel* destPtr = (TPixel*)(dest + depthZ + strideY);

			for (size_t x = 0; x < size.width; ++x)
			{
				colorv sum;
				for (int fy = 0; fy < (int)mask.height; ++fy)
				{
					for (int fx = 0; fx < (int)mask.width; ++fx)
					{
						size_t cx = dseed::clamp<int> ((int)x + (fx - (int)mask.width / 2), size.width - 1);
						size_t cy = dseed::clamp<int> ((int)y + (fy - (int)mask.height / 2), size.height - 1);

						colorv color = *((TPixel*)(src + depthZ + (stride * cy)) + cx);
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

std::map<pixelformat, ftfn> g_resizes = {
	{ pixelformat::rgba8, filter_bitmap<rgba8> },
	{ pixelformat::rgb8, filter_bitmap<rgb8> },
	{ pixelformat::rgbaf, filter_bitmap<rgbaf> },
	{ pixelformat::bgra8, filter_bitmap<bgra8> },
	{ pixelformat::bgr8, filter_bitmap<bgr8> },
	{ pixelformat::bgra4, filter_bitmap<bgra4> },
	{ pixelformat::bgr565, filter_bitmap<bgr565> },
	{ pixelformat::r8, filter_bitmap<r8> },
	{ pixelformat::rf, filter_bitmap<rf> },
	{ pixelformat::yuva8, filter_bitmap<yuva8> },
	{ pixelformat::yuv8, filter_bitmap<yuv8> },
	{ pixelformat::hsva8, filter_bitmap<hsva8> },
	{ pixelformat::hsv8, filter_bitmap<hsv8> },
};

dseed::error_t dseed::bitmaps::filter_bitmap (dseed::bitmaps::bitmap* original, const dseed::bitmaps::bitmap_filter_mask& mask, dseed::bitmaps::bitmap** bitmap)
{
	if (original == nullptr || bitmap == nullptr)
		return dseed::error_invalid_args;
	if (mask.width % 2 == 0 || mask.height % 2 == 0)
		return dseed::error_invalid_args;

	dseed::autoref<dseed::bitmaps::bitmap> temp;
	if (dseed::failed (dseed::bitmaps::create_bitmap (original->type (), original->size (), original->format (), nullptr, &temp)))
		return dseed::error_fail;

	uint8_t* destPtr, * srcPtr;
	original->lock ((void**)&srcPtr);
	temp->lock ((void**)&destPtr);

	auto found = g_resizes.find (original->format ());
	if (found == g_resizes.end ())
		return dseed::error_not_support;

	if (!found->second (destPtr, srcPtr, original->size (), mask))
		return dseed::error_not_support;

	temp->unlock ();
	original->unlock ();

	*bitmap = temp.detach ();

	return dseed::error_good;
}
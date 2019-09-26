#include <dseed.h>

#include <map>

using namespace dseed;

template<class TPixel> inline TPixel add (const TPixel& p1, const TPixel& p2) noexcept { return p1 + p2; }
template<class TPixel> inline TPixel subtract (const TPixel& p1, const TPixel& p2) noexcept { return p1 - p2; }
template<class TPixel> inline TPixel multiply (const TPixel& p1, const TPixel& p2) noexcept { return p1 * p2; }
template<class TPixel> inline TPixel divide (const TPixel& p1, const TPixel& p2) noexcept { return p1 / p2; }
template<class TPixel> inline TPixel and (const TPixel& p1, const TPixel& p2) noexcept { return p1 & p2; }
template<class TPixel> inline TPixel or (const TPixel& p1, const TPixel& p2) noexcept { return p1 | p2; }
template<class TPixel> inline TPixel xor (const TPixel& p1, const TPixel& p2) noexcept { return p1 ^ p2; }
template<class TPixel>
using Operator2 = TPixel (*)(const TPixel&, const TPixel&);
template<class TPixel> inline TPixel negate (const TPixel& p) noexcept { return -p; }
template<class TPixel> inline TPixel not (const TPixel& p) noexcept { return ~p; }
template<class TPixel> inline TPixel invert (const TPixel& p) noexcept { return TPixel::max_color () - p; }
template<class TPixel>
using Operator1 = TPixel (*)(const TPixel&);

using binopfn = std::function<bool (uint8_t*, const uint8_t*, const uint8_t*, const dseed::size3i&)>;
using binoptp = std::tuple<dseed::pixelformat_t, dseed::binary_operator_t>;

template<class TPixel, Operator2<TPixel> op>
inline bool binary_operation (uint8_t* dest, const uint8_t* src1, const uint8_t* src2, const dseed::size3i& size) noexcept
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
			const TPixel* src1Ptr = (const TPixel*)(src1 + depthZ + strideY);
			const TPixel* src2Ptr = (const TPixel*)(src2 + depthZ + strideY);

			for (size_t x = 0; x < size.width; ++x)
			{
				const TPixel& pixel1 = *(src1Ptr + x);
				const TPixel& pixel2 = *(src2Ptr + x);

				TPixel result = op (pixel1, pixel2);
				*(destPtr + x) = result;
			}
		}
	}

	return true;
}

std::map<binoptp, binopfn> g_binops = {
	{ binoptp (pixelformat_rgba8888, binary_operator_add), binary_operation<rgba, add<rgba>> },
	{ binoptp (pixelformat_rgb888, binary_operator_add), binary_operation<rgb, add<rgb>> },
	{ binoptp (pixelformat_rgbaf, binary_operator_add), binary_operation<rgbaf, add<rgbaf>> },
	{ binoptp (pixelformat_bgra8888, binary_operator_add), binary_operation<bgra, add<bgra>> },
	{ binoptp (pixelformat_bgr888, binary_operator_add), binary_operation<bgr, add<bgr>> },
	{ binoptp (pixelformat_bgra4444, binary_operator_add), binary_operation<bgra4, add<bgra4>> },
	{ binoptp (pixelformat_bgr565, binary_operator_add), binary_operation<bgr565, add<bgr565>> },
	{ binoptp (pixelformat_grayscale8, binary_operator_add), binary_operation<grayscale, add<grayscale>> },
	{ binoptp (pixelformat_grayscalef, binary_operator_add), binary_operation<grayscalef, add<grayscalef>> },
	{ binoptp (pixelformat_yuva8888, binary_operator_add), binary_operation<yuva, add<yuva>> },
	{ binoptp (pixelformat_yuv888, binary_operator_add), binary_operation<yuv, add<yuv>> },
	{ binoptp (pixelformat_hsva8888, binary_operator_add), binary_operation<hsva, add<hsva>> },
	{ binoptp (pixelformat_hsv888, binary_operator_add), binary_operation<hsv, add<hsv>> },

	{ binoptp (pixelformat_rgba8888, binary_operator_subtract), binary_operation<rgba, subtract<rgba>> },
	{ binoptp (pixelformat_rgb888, binary_operator_subtract), binary_operation<rgb, subtract<rgb>> },
	{ binoptp (pixelformat_rgbaf, binary_operator_subtract), binary_operation<rgbaf, subtract<rgbaf>> },
	{ binoptp (pixelformat_bgra8888, binary_operator_subtract), binary_operation<bgra, subtract<bgra>> },
	{ binoptp (pixelformat_bgr888, binary_operator_subtract), binary_operation<bgr, subtract<bgr>> },
	{ binoptp (pixelformat_bgra4444, binary_operator_subtract), binary_operation<bgra4, subtract<bgra4>> },
	{ binoptp (pixelformat_bgr565, binary_operator_subtract), binary_operation<bgr565, subtract<bgr565>> },
	{ binoptp (pixelformat_grayscale8, binary_operator_subtract), binary_operation<grayscale, subtract<grayscale>> },
	{ binoptp (pixelformat_grayscalef, binary_operator_subtract), binary_operation<grayscalef, subtract<grayscalef>> },
	{ binoptp (pixelformat_yuva8888, binary_operator_subtract), binary_operation<yuva, subtract<yuva>> },
	{ binoptp (pixelformat_yuv888, binary_operator_subtract), binary_operation<yuv, subtract<yuv>> },
	{ binoptp (pixelformat_hsva8888, binary_operator_subtract), binary_operation<hsva, subtract<hsva>> },
	{ binoptp (pixelformat_hsv888, binary_operator_subtract), binary_operation<hsv, subtract<hsv>> },

	{ binoptp (pixelformat_rgba8888, binary_operator_multiply), binary_operation<rgba, multiply<rgba>> },
	{ binoptp (pixelformat_rgb888, binary_operator_multiply), binary_operation<rgb, multiply<rgb>> },
	{ binoptp (pixelformat_rgbaf, binary_operator_multiply), binary_operation<rgbaf, multiply<rgbaf>> },
	{ binoptp (pixelformat_bgra8888, binary_operator_multiply), binary_operation<bgra, multiply<bgra>> },
	{ binoptp (pixelformat_bgr888, binary_operator_multiply), binary_operation<bgr, multiply<bgr>> },
	{ binoptp (pixelformat_bgra4444, binary_operator_multiply), binary_operation<bgra4, multiply<bgra4>> },
	{ binoptp (pixelformat_bgr565, binary_operator_multiply), binary_operation<bgr565, multiply<bgr565>> },
	{ binoptp (pixelformat_grayscale8, binary_operator_multiply), binary_operation<grayscale, multiply<grayscale>> },
	{ binoptp (pixelformat_grayscalef, binary_operator_multiply), binary_operation<grayscalef, multiply<grayscalef>> },
	{ binoptp (pixelformat_yuva8888, binary_operator_multiply), binary_operation<yuva, multiply<yuva>> },
	{ binoptp (pixelformat_yuv888, binary_operator_multiply), binary_operation<yuv, multiply<yuv>> },
	{ binoptp (pixelformat_hsva8888, binary_operator_multiply), binary_operation<hsva, multiply<hsva>> },
	{ binoptp (pixelformat_hsv888, binary_operator_multiply), binary_operation<hsv, multiply<hsv>> },

	{ binoptp (pixelformat_rgba8888, binary_operator_divide), binary_operation<rgba, divide<rgba>> },
	{ binoptp (pixelformat_rgb888, binary_operator_divide), binary_operation<rgb, divide<rgb>> },
	{ binoptp (pixelformat_rgbaf, binary_operator_divide), binary_operation<rgbaf, divide<rgbaf>> },
	{ binoptp (pixelformat_bgra8888, binary_operator_divide), binary_operation<bgra, divide<bgra>> },
	{ binoptp (pixelformat_bgr888, binary_operator_divide), binary_operation<bgr, divide<bgr>> },
	{ binoptp (pixelformat_bgra4444, binary_operator_divide), binary_operation<bgra4, divide<bgra4>> },
	{ binoptp (pixelformat_bgr565, binary_operator_divide), binary_operation<bgr565, divide<bgr565>> },
	{ binoptp (pixelformat_grayscale8, binary_operator_divide), binary_operation<grayscale, divide<grayscale>> },
	{ binoptp (pixelformat_grayscalef, binary_operator_divide), binary_operation<grayscalef, divide<grayscalef>> },
	{ binoptp (pixelformat_yuva8888, binary_operator_divide), binary_operation<yuva, divide<yuva>> },
	{ binoptp (pixelformat_yuv888, binary_operator_divide), binary_operation<yuv, divide<yuv>> },
	{ binoptp (pixelformat_hsva8888, binary_operator_divide), binary_operation<hsva, divide<hsva>> },
	{ binoptp (pixelformat_hsv888, binary_operator_divide), binary_operation<hsv, divide<hsv>> },

	{ binoptp (pixelformat_rgba8888, binary_operator_and), binary_operation<rgba, and<rgba>> },
	{ binoptp (pixelformat_rgb888, binary_operator_and), binary_operation<rgb, and<rgb>> },
	{ binoptp (pixelformat_bgra8888, binary_operator_and), binary_operation<bgra, and<bgra>> },
	{ binoptp (pixelformat_bgr888, binary_operator_and), binary_operation<bgr, and<bgr>> },
	{ binoptp (pixelformat_bgra4444, binary_operator_and), binary_operation<bgra4, and<bgra4>> },
	{ binoptp (pixelformat_bgr565, binary_operator_and), binary_operation<bgr565, and<bgr565>> },
	{ binoptp (pixelformat_grayscale8, binary_operator_and), binary_operation<grayscale, and<grayscale>> },
	{ binoptp (pixelformat_yuva8888, binary_operator_and), binary_operation<yuva, and<yuva>> },
	{ binoptp (pixelformat_yuv888, binary_operator_and), binary_operation<yuv, and<yuv>> },
	{ binoptp (pixelformat_hsva8888, binary_operator_and), binary_operation<hsva,and <hsva>> },
	{ binoptp (pixelformat_hsv888, binary_operator_and), binary_operation<hsv,and <hsv>> },

	{ binoptp (pixelformat_rgba8888, binary_operator_or), binary_operation<rgba, or<rgba>> },
	{ binoptp (pixelformat_rgb888, binary_operator_or), binary_operation<rgb, or<rgb>> },
	{ binoptp (pixelformat_bgra8888, binary_operator_or), binary_operation<bgra, or<bgra>> },
	{ binoptp (pixelformat_bgr888, binary_operator_or), binary_operation<bgr, or<bgr>> },
	{ binoptp (pixelformat_bgra4444, binary_operator_or), binary_operation<bgra4, or<bgra4>> },
	{ binoptp (pixelformat_bgr565, binary_operator_or), binary_operation<bgr565, or<bgr565>> },
	{ binoptp (pixelformat_grayscale8, binary_operator_or), binary_operation<grayscale, or<grayscale>> },
	{ binoptp (pixelformat_yuva8888, binary_operator_or), binary_operation<yuva, or<yuva>> },
	{ binoptp (pixelformat_yuv888, binary_operator_or), binary_operation<yuv, or<yuv>> },
	{ binoptp (pixelformat_hsva8888, binary_operator_or), binary_operation<hsva, or <hsva>> },
	{ binoptp (pixelformat_hsv888, binary_operator_or), binary_operation<hsv, or <hsv>> },

	{ binoptp (pixelformat_rgba8888, binary_operator_xor), binary_operation<rgba, xor<rgba>> },
	{ binoptp (pixelformat_rgb888, binary_operator_xor), binary_operation<rgb, xor<rgb>> },
	{ binoptp (pixelformat_bgra8888, binary_operator_xor), binary_operation<bgra, xor<bgra>> },
	{ binoptp (pixelformat_bgr888, binary_operator_xor), binary_operation<bgr, xor<bgr>> },
	{ binoptp (pixelformat_bgra4444, binary_operator_xor), binary_operation<bgra4, xor<bgra4>> },
	{ binoptp (pixelformat_bgr565, binary_operator_xor), binary_operation<bgr565, xor<bgr565>> },
	{ binoptp (pixelformat_grayscale8, binary_operator_xor), binary_operation<grayscale, xor<grayscale>> },
	{ binoptp (pixelformat_yuva8888, binary_operator_xor), binary_operation<yuva, xor<yuva>> },
	{ binoptp (pixelformat_yuv888, binary_operator_xor), binary_operation<yuv, xor<yuv>> },
	{ binoptp (pixelformat_hsva8888, binary_operator_xor), binary_operation<hsva,xor <hsva>> },
	{ binoptp (pixelformat_hsv888, binary_operator_xor), binary_operation<hsv,xor <hsv>> },
};

dseed::error_t dseed::bitmap_binary_operation (dseed::bitmap* b1, dseed::bitmap* b2, dseed::binary_operator_t op, dseed::bitmap** bitmap)
{
	if (b1 == nullptr || b2 == nullptr)
		return dseed::error_invalid_args;

	size3i b1Size = b1->size ();
	size3i b2Size = b2->size ();

	if (b1->type () != b2->type ()
		|| b1Size.width != b2Size.width || b1Size.height != b2Size.height || b1Size.depth != b2Size.depth
		|| b1->format () != b2->format ())
		return dseed::error_invalid_args;

	dseed::auto_object<dseed::bitmap> temp;
	if (bitmap != nullptr)
	{
		if (dseed::failed (dseed::create_bitmap (b1->type (), b1->size (), b1->format (), nullptr, &temp)))
			return dseed::error_fail;
	}
	else
		temp = b1;

	uint8_t* destPtr, * src1Ptr, * src2Ptr;
	b1->pixels_pointer ((void**)& src1Ptr);
	b2->pixels_pointer ((void**)& src2Ptr);
	temp->pixels_pointer ((void**)& destPtr);

	auto found = g_binops.find (binoptp(b1->format (), op));
	if (found == g_binops.end ())
		return dseed::error_not_support;

	if (!found->second (destPtr, src1Ptr, src2Ptr, b1->size ()))
		return dseed::error_fail;

	*bitmap = temp.detach ();

	return dseed::error_good;
}

using unopfn = std::function<bool (uint8_t*, const uint8_t*, const dseed::size3i&)>;
using unoptp = std::tuple<dseed::pixelformat_t, dseed::unary_operator_t>;

template<class TPixel, Operator1<TPixel> op>
inline bool unary_operation (uint8_t* dest, const uint8_t* src, const dseed::size3i& size) noexcept
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
			const TPixel* srcPtr = (const TPixel*)(src + depthZ + strideY);

			for (size_t x = 0; x < size.width; ++x)
			{
				const TPixel& pixel = *(srcPtr + x);
				TPixel result = op (pixel);
				*(destPtr + x) = result;
			}
		}
	}

	return true;
}

std::map<unoptp, unopfn> g_unops = {
	{ unoptp (pixelformat_rgbaf, unary_operator_negate), unary_operation<rgbaf, negate<rgbaf>> },
	{ unoptp (pixelformat_grayscalef, unary_operator_negate), unary_operation<grayscalef, negate<grayscalef>> },

	{ unoptp (pixelformat_rgba8888, unary_operator_not), unary_operation<rgba, not<rgba>> },
	{ unoptp (pixelformat_rgb888, unary_operator_not), unary_operation<rgb, not<rgb>> },
	{ unoptp (pixelformat_bgra8888, unary_operator_not), unary_operation<bgra, not<bgra>> },
	{ unoptp (pixelformat_bgr888, unary_operator_not), unary_operation<bgr, not<bgr>> },
	{ unoptp (pixelformat_bgra4444, unary_operator_not), unary_operation<bgra4, not<bgra4>> },
	{ unoptp (pixelformat_bgr565, unary_operator_not), unary_operation<bgr565, not<bgr565>> },
	{ unoptp (pixelformat_grayscale8, unary_operator_not), unary_operation<grayscale, not<grayscale>> },
	{ unoptp (pixelformat_yuva8888, unary_operator_not), unary_operation<yuva, not<yuva>> },
	{ unoptp (pixelformat_yuv888, unary_operator_not), unary_operation<yuv, not<yuv>> },
	{ unoptp (pixelformat_hsva8888, unary_operator_not), unary_operation<hsva, not<hsva>> },
	{ unoptp (pixelformat_hsv888, unary_operator_not), unary_operation<hsv, not<hsv>> },

	{ unoptp (pixelformat_rgba8888, unary_operator_invert), unary_operation<rgba, invert<rgba>> },
	{ unoptp (pixelformat_rgb888, unary_operator_invert), unary_operation<rgb, invert<rgb>> },
	{ unoptp (pixelformat_rgbaf, unary_operator_invert), unary_operation<rgbaf, invert<rgbaf>> },
	{ unoptp (pixelformat_bgra8888, unary_operator_invert), unary_operation<bgra, invert<bgra>> },
	{ unoptp (pixelformat_bgr888, unary_operator_invert), unary_operation<bgr, invert<bgr>> },
	{ unoptp (pixelformat_bgra4444, unary_operator_invert), unary_operation<bgra4, invert<bgra4>> },
	{ unoptp (pixelformat_bgr565, unary_operator_invert), unary_operation<bgr565, invert<bgr565>> },
	{ unoptp (pixelformat_grayscale8, unary_operator_invert), unary_operation<grayscale, invert<grayscale>> },
	{ unoptp (pixelformat_grayscalef, unary_operator_invert), unary_operation<grayscalef, invert<grayscalef>> },
	{ unoptp (pixelformat_yuva8888, unary_operator_invert), unary_operation<yuva, invert<yuva>> },
	{ unoptp (pixelformat_yuv888, unary_operator_invert), unary_operation<yuv, invert<yuv>> },
	{ unoptp (pixelformat_hsva8888, unary_operator_invert), unary_operation<hsva, invert<hsva>> },
	{ unoptp (pixelformat_hsv888, unary_operator_invert), unary_operation<hsv, invert<hsv>> },
};

dseed::error_t dseed::bitmap_unary_operation (dseed::bitmap* b, dseed::unary_operator_t op, dseed::bitmap** bitmap)
{
	if (b == nullptr)
		return dseed::error_invalid_args;

	dseed::auto_object<dseed::bitmap> temp;
	if (bitmap != nullptr)
	{
		if (dseed::failed (dseed::create_bitmap (b->type (), b->size (), b->format (), nullptr, &temp)))
			return dseed::error_fail;
	}
	else
		temp = b;

	uint8_t* destPtr, * srcPtr;
	b->pixels_pointer ((void**)& srcPtr);
	temp->pixels_pointer ((void**)& destPtr);

	auto found = g_unops.find (unoptp (b->format (), op));
	if (found == g_unops.end ())
		return dseed::error_not_support;

	if (!found->second (destPtr, srcPtr, b->size ()))
		return dseed::error_not_support;

	*bitmap = temp.detach ();

	return error_good;
}
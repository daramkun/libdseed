#include <dseed.h>

#include <map>

using namespace dseed;

template<class TPixel> inline TPixel padd (const TPixel& p1, const TPixel& p2) noexcept { return p1 + p2; }
template<class TPixel> inline TPixel psubtract (const TPixel& p1, const TPixel& p2) noexcept { return p1 - p2; }
template<class TPixel> inline TPixel pmultiply (const TPixel& p1, const TPixel& p2) noexcept { return p1 * p2; }
template<class TPixel> inline TPixel pdivide (const TPixel& p1, const TPixel& p2) noexcept { return p1 / p2; }
template<class TPixel> inline TPixel pand (const TPixel& p1, const TPixel& p2) noexcept { return p1 & p2; }
template<class TPixel> inline TPixel por (const TPixel& p1, const TPixel& p2) noexcept { return p1 | p2; }
template<class TPixel> inline TPixel pxor (const TPixel& p1, const TPixel& p2) noexcept { return p1 ^ p2; }
template<class TPixel>
using Operator2 = TPixel (*)(const TPixel&, const TPixel&);
template<class TPixel> inline TPixel pnegate (const TPixel& p) noexcept { return -p; }
template<class TPixel> inline TPixel pnot (const TPixel& p) noexcept { return ~p; }
template<class TPixel> inline TPixel pinvert (const TPixel& p) noexcept { return TPixel::max_color () - p; }
template<class TPixel>
using Operator1 = TPixel (*)(const TPixel&);

using binopfn = std::function<bool (uint8_t*, const uint8_t*, const uint8_t*, const dseed::size3i&)>;
using binoptp = std::tuple<dseed::pixelformat, dseed::binary_operator>;

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
	{ binoptp (pixelformat::rgba8888, binary_operator::add), binary_operation<rgba, padd<rgba>> },
	{ binoptp (pixelformat::rgb888, binary_operator::add), binary_operation<rgb, padd<rgb>> },
	{ binoptp (pixelformat::rgbaf, binary_operator::add), binary_operation<rgbaf, padd<rgbaf>> },
	{ binoptp (pixelformat::bgra8888, binary_operator::add), binary_operation<bgra, padd<bgra>> },
	{ binoptp (pixelformat::bgr888, binary_operator::add), binary_operation<bgr, padd<bgr>> },
	{ binoptp (pixelformat::bgra4444, binary_operator::add), binary_operation<bgra4, padd<bgra4>> },
	{ binoptp (pixelformat::bgr565, binary_operator::add), binary_operation<bgr565, padd<bgr565>> },
	{ binoptp (pixelformat::grayscale8, binary_operator::add), binary_operation<grayscale, padd<grayscale>> },
	{ binoptp (pixelformat::grayscalef, binary_operator::add), binary_operation<grayscalef, padd<grayscalef>> },
	{ binoptp (pixelformat::yuva8888, binary_operator::add), binary_operation<yuva, padd<yuva>> },
	{ binoptp (pixelformat::yuv888, binary_operator::add), binary_operation<yuv, padd<yuv>> },
	{ binoptp (pixelformat::hsva8888, binary_operator::add), binary_operation<hsva, padd<hsva>> },
	{ binoptp (pixelformat::hsv888, binary_operator::add), binary_operation<hsv, padd<hsv>> },

	{ binoptp (pixelformat::rgba8888, binary_operator::subtract), binary_operation<rgba, psubtract<rgba>> },
	{ binoptp (pixelformat::rgb888, binary_operator::subtract), binary_operation<rgb, psubtract<rgb>> },
	{ binoptp (pixelformat::rgbaf, binary_operator::subtract), binary_operation<rgbaf, psubtract<rgbaf>> },
	{ binoptp (pixelformat::bgra8888, binary_operator::subtract), binary_operation<bgra, psubtract<bgra>> },
	{ binoptp (pixelformat::bgr888, binary_operator::subtract), binary_operation<bgr, psubtract<bgr>> },
	{ binoptp (pixelformat::bgra4444, binary_operator::subtract), binary_operation<bgra4, psubtract<bgra4>> },
	{ binoptp (pixelformat::bgr565, binary_operator::subtract), binary_operation<bgr565, psubtract<bgr565>> },
	{ binoptp (pixelformat::grayscale8, binary_operator::subtract), binary_operation<grayscale, psubtract<grayscale>> },
	{ binoptp (pixelformat::grayscalef, binary_operator::subtract), binary_operation<grayscalef, psubtract<grayscalef>> },
	{ binoptp (pixelformat::yuva8888, binary_operator::subtract), binary_operation<yuva, psubtract<yuva>> },
	{ binoptp (pixelformat::yuv888, binary_operator::subtract), binary_operation<yuv, psubtract<yuv>> },
	{ binoptp (pixelformat::hsva8888, binary_operator::subtract), binary_operation<hsva, psubtract<hsva>> },
	{ binoptp (pixelformat::hsv888, binary_operator::subtract), binary_operation<hsv, psubtract<hsv>> },

	{ binoptp (pixelformat::rgba8888, binary_operator::multiply), binary_operation<rgba, pmultiply<rgba>> },
	{ binoptp (pixelformat::rgb888, binary_operator::multiply), binary_operation<rgb, pmultiply<rgb>> },
	{ binoptp (pixelformat::rgbaf, binary_operator::multiply), binary_operation<rgbaf, pmultiply<rgbaf>> },
	{ binoptp (pixelformat::bgra8888, binary_operator::multiply), binary_operation<bgra, pmultiply<bgra>> },
	{ binoptp (pixelformat::bgr888, binary_operator::multiply), binary_operation<bgr, pmultiply<bgr>> },
	{ binoptp (pixelformat::bgra4444, binary_operator::multiply), binary_operation<bgra4, pmultiply<bgra4>> },
	{ binoptp (pixelformat::bgr565, binary_operator::multiply), binary_operation<bgr565, pmultiply<bgr565>> },
	{ binoptp (pixelformat::grayscale8, binary_operator::multiply), binary_operation<grayscale, pmultiply<grayscale>> },
	{ binoptp (pixelformat::grayscalef, binary_operator::multiply), binary_operation<grayscalef, pmultiply<grayscalef>> },
	{ binoptp (pixelformat::yuva8888, binary_operator::multiply), binary_operation<yuva, pmultiply<yuva>> },
	{ binoptp (pixelformat::yuv888, binary_operator::multiply), binary_operation<yuv, pmultiply<yuv>> },
	{ binoptp (pixelformat::hsva8888, binary_operator::multiply), binary_operation<hsva, pmultiply<hsva>> },
	{ binoptp (pixelformat::hsv888, binary_operator::multiply), binary_operation<hsv, pmultiply<hsv>> },

	{ binoptp (pixelformat::rgba8888, binary_operator::divide), binary_operation<rgba, pdivide<rgba>> },
	{ binoptp (pixelformat::rgb888, binary_operator::divide), binary_operation<rgb, pdivide<rgb>> },
	{ binoptp (pixelformat::rgbaf, binary_operator::divide), binary_operation<rgbaf, pdivide<rgbaf>> },
	{ binoptp (pixelformat::bgra8888, binary_operator::divide), binary_operation<bgra, pdivide<bgra>> },
	{ binoptp (pixelformat::bgr888, binary_operator::divide), binary_operation<bgr, pdivide<bgr>> },
	{ binoptp (pixelformat::bgra4444, binary_operator::divide), binary_operation<bgra4, pdivide<bgra4>> },
	{ binoptp (pixelformat::bgr565, binary_operator::divide), binary_operation<bgr565, pdivide<bgr565>> },
	{ binoptp (pixelformat::grayscale8, binary_operator::divide), binary_operation<grayscale, pdivide<grayscale>> },
	{ binoptp (pixelformat::grayscalef, binary_operator::divide), binary_operation<grayscalef, pdivide<grayscalef>> },
	{ binoptp (pixelformat::yuva8888, binary_operator::divide), binary_operation<yuva, pdivide<yuva>> },
	{ binoptp (pixelformat::yuv888, binary_operator::divide), binary_operation<yuv, pdivide<yuv>> },
	{ binoptp (pixelformat::hsva8888, binary_operator::divide), binary_operation<hsva, pdivide<hsva>> },
	{ binoptp (pixelformat::hsv888, binary_operator::divide), binary_operation<hsv, pdivide<hsv>> },

	{ binoptp (pixelformat::rgba8888, binary_operator::and), binary_operation<rgba, pand<rgba>> },
	{ binoptp (pixelformat::rgb888, binary_operator::and), binary_operation<rgb, pand<rgb>> },
	{ binoptp (pixelformat::bgra8888, binary_operator::and), binary_operation<bgra, pand<bgra>> },
	{ binoptp (pixelformat::bgr888, binary_operator::and), binary_operation<bgr, pand<bgr>> },
	{ binoptp (pixelformat::bgra4444, binary_operator::and), binary_operation<bgra4, pand<bgra4>> },
	{ binoptp (pixelformat::bgr565, binary_operator::and), binary_operation<bgr565, pand<bgr565>> },
	{ binoptp (pixelformat::grayscale8, binary_operator::and), binary_operation<grayscale, pand<grayscale>> },
	{ binoptp (pixelformat::yuva8888, binary_operator::and), binary_operation<yuva, pand<yuva>> },
	{ binoptp (pixelformat::yuv888, binary_operator::and), binary_operation<yuv, pand<yuv>> },
	{ binoptp (pixelformat::hsva8888, binary_operator::and), binary_operation<hsva, pand<hsva>> },
	{ binoptp (pixelformat::hsv888, binary_operator::and), binary_operation<hsv, pand<hsv>> },

	{ binoptp (pixelformat::rgba8888, binary_operator::or), binary_operation<rgba, por<rgba>> },
	{ binoptp (pixelformat::rgb888, binary_operator::or), binary_operation<rgb, por<rgb>> },
	{ binoptp (pixelformat::bgra8888, binary_operator::or), binary_operation<bgra, por<bgra>> },
	{ binoptp (pixelformat::bgr888, binary_operator::or), binary_operation<bgr, por<bgr>> },
	{ binoptp (pixelformat::bgra4444, binary_operator::or), binary_operation<bgra4, por<bgra4>> },
	{ binoptp (pixelformat::bgr565, binary_operator::or), binary_operation<bgr565, por<bgr565>> },
	{ binoptp (pixelformat::grayscale8, binary_operator::or), binary_operation<grayscale, por<grayscale>> },
	{ binoptp (pixelformat::yuva8888, binary_operator::or), binary_operation<yuva, por<yuva>> },
	{ binoptp (pixelformat::yuv888, binary_operator::or), binary_operation<yuv, por<yuv>> },
	{ binoptp (pixelformat::hsva8888, binary_operator::or), binary_operation<hsva, por <hsva>> },
	{ binoptp (pixelformat::hsv888, binary_operator::or), binary_operation<hsv, por <hsv>> },

	{ binoptp (pixelformat::rgba8888, binary_operator::xor), binary_operation<rgba, pxor<rgba>> },
	{ binoptp (pixelformat::rgb888, binary_operator::xor), binary_operation<rgb, pxor<rgb>> },
	{ binoptp (pixelformat::bgra8888, binary_operator::xor), binary_operation<bgra, pxor<bgra>> },
	{ binoptp (pixelformat::bgr888, binary_operator::xor), binary_operation<bgr, pxor<bgr>> },
	{ binoptp (pixelformat::bgra4444, binary_operator::xor), binary_operation<bgra4, pxor<bgra4>> },
	{ binoptp (pixelformat::bgr565, binary_operator::xor), binary_operation<bgr565, pxor<bgr565>> },
	{ binoptp (pixelformat::grayscale8, binary_operator::xor), binary_operation<grayscale, pxor<grayscale>> },
	{ binoptp (pixelformat::yuva8888, binary_operator::xor), binary_operation<yuva, pxor<yuva>> },
	{ binoptp (pixelformat::yuv888, binary_operator::xor), binary_operation<yuv, pxor<yuv>> },
	{ binoptp (pixelformat::hsva8888, binary_operator::xor), binary_operation<hsva, pxor<hsva>> },
	{ binoptp (pixelformat::hsv888, binary_operator::xor), binary_operation<hsv, pxor<hsv>> },
};

dseed::error_t dseed::bitmap_binary_operation (dseed::bitmap* b1, dseed::bitmap* b2, dseed::binary_operator op, dseed::bitmap** bitmap)
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
using unoptp = std::tuple<dseed::pixelformat, dseed::unary_operator>;

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
	{ unoptp (pixelformat::rgbaf, unary_operator::negate), unary_operation<rgbaf, pnegate<rgbaf>> },
	{ unoptp (pixelformat::grayscalef, unary_operator::negate), unary_operation<grayscalef, pnegate<grayscalef>> },

	{ unoptp (pixelformat::rgba8888, unary_operator::not), unary_operation<rgba, pnot<rgba>> },
	{ unoptp (pixelformat::rgb888, unary_operator::not), unary_operation<rgb, pnot<rgb>> },
	{ unoptp (pixelformat::bgra8888, unary_operator::not), unary_operation<bgra, pnot<bgra>> },
	{ unoptp (pixelformat::bgr888, unary_operator::not), unary_operation<bgr, pnot<bgr>> },
	{ unoptp (pixelformat::bgra4444, unary_operator::not), unary_operation<bgra4, pnot<bgra4>> },
	{ unoptp (pixelformat::bgr565, unary_operator::not), unary_operation<bgr565, pnot<bgr565>> },
	{ unoptp (pixelformat::grayscale8, unary_operator::not), unary_operation<grayscale, pnot<grayscale>> },
	{ unoptp (pixelformat::yuva8888, unary_operator::not), unary_operation<yuva, pnot<yuva>> },
	{ unoptp (pixelformat::yuv888, unary_operator::not), unary_operation<yuv, pnot<yuv>> },
	{ unoptp (pixelformat::hsva8888, unary_operator::not), unary_operation<hsva, pnot<hsva>> },
	{ unoptp (pixelformat::hsv888, unary_operator::not), unary_operation<hsv, pnot<hsv>> },

	{ unoptp (pixelformat::rgba8888, unary_operator::invert), unary_operation<rgba, pinvert<rgba>> },
	{ unoptp (pixelformat::rgb888, unary_operator::invert), unary_operation<rgb, pinvert<rgb>> },
	{ unoptp (pixelformat::rgbaf, unary_operator::invert), unary_operation<rgbaf, pinvert<rgbaf>> },
	{ unoptp (pixelformat::bgra8888, unary_operator::invert), unary_operation<bgra, pinvert<bgra>> },
	{ unoptp (pixelformat::bgr888, unary_operator::invert), unary_operation<bgr, pinvert<bgr>> },
	{ unoptp (pixelformat::bgra4444, unary_operator::invert), unary_operation<bgra4, pinvert<bgra4>> },
	{ unoptp (pixelformat::bgr565, unary_operator::invert), unary_operation<bgr565, pinvert<bgr565>> },
	{ unoptp (pixelformat::grayscale8, unary_operator::invert), unary_operation<grayscale, pinvert<grayscale>> },
	{ unoptp (pixelformat::grayscalef, unary_operator::invert), unary_operation<grayscalef, pinvert<grayscalef>> },
	{ unoptp (pixelformat::yuva8888, unary_operator::invert), unary_operation<yuva, pinvert<yuva>> },
	{ unoptp (pixelformat::yuv888, unary_operator::invert), unary_operation<yuv, pinvert<yuv>> },
	{ unoptp (pixelformat::hsva8888, unary_operator::invert), unary_operation<hsva, pinvert<hsva>> },
	{ unoptp (pixelformat::hsv888, unary_operator::invert), unary_operation<hsv, pinvert<hsv>> },
};

dseed::error_t dseed::bitmap_unary_operation (dseed::bitmap* b, dseed::unary_operator op, dseed::bitmap** bitmap)
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
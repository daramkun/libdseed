#include <dseed.h>

#include <map>
#include <tuple>

using namespace dseed::color;
using size2i = dseed::size2i;

template<class TPixel> inline TPixel padd(const TPixel& p1, const TPixel& p2) noexcept { return p1 + p2; }
template<class TPixel> inline TPixel psubtract(const TPixel& p1, const TPixel& p2) noexcept { return p1 - p2; }
template<class TPixel> inline TPixel pmultiply(const TPixel& p1, const TPixel& p2) noexcept { return p1 * p2; }
template<class TPixel> inline TPixel pdivide(const TPixel& p1, const TPixel& p2) noexcept { return p1 / p2; }
template<class TPixel> inline TPixel pand(const TPixel& p1, const TPixel& p2) noexcept { return p1 & p2; }
template<class TPixel> inline TPixel por(const TPixel& p1, const TPixel& p2) noexcept { return p1 | p2; }
template<class TPixel> inline TPixel pxor(const TPixel& p1, const TPixel& p2) noexcept { return p1 ^ p2; }
template<class TPixel>
using Operator2 = TPixel(*)(const TPixel&, const TPixel&);
template<class TPixel> inline TPixel pnegate(const TPixel& p) noexcept { return -p; }
template<class TPixel> inline TPixel pnot(const TPixel& p) noexcept { return ~p; }
template<class TPixel> inline TPixel pinvert(const TPixel& p) noexcept { return TPixel::max_color() - p; }
template<class TPixel>
using Operator1 = TPixel(*)(const TPixel&);

using binopfn = std::function<bool(uint8_t*, const uint8_t*, const uint8_t*, const dseed::size3i&)>;
using binoptp = std::tuple<pixelformat, dseed::binary_operator>;

template<class TPixel, Operator2<TPixel> op>
inline bool binary_operation(uint8_t* dest, const uint8_t* src1, const uint8_t* src2, const dseed::size3i& size) noexcept
{
	size_t stride = calc_bitmap_stride(type2format<TPixel>(), size.width);
	size_t depth = calc_bitmap_plane_size(type2format<TPixel>(), size2i(size.width, size.height));

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

				TPixel result = op(pixel1, pixel2);
				*(destPtr + x) = result;
			}
		}
	}

	return true;
}

std::map<binoptp, binopfn> g_binops = {
	{ binoptp(pixelformat::rgba8, dseed::binary_operator::add), binary_operation<rgba8, padd<rgba8>> },
	{ binoptp(pixelformat::rgb8, dseed::binary_operator::add), binary_operation<rgb8, padd<rgb8>> },
	{ binoptp(pixelformat::rgbaf, dseed::binary_operator::add), binary_operation<rgbaf, padd<rgbaf>> },
	{ binoptp(pixelformat::bgra8, dseed::binary_operator::add), binary_operation<bgra8, padd<bgra8>> },
	{ binoptp(pixelformat::bgr8, dseed::binary_operator::add), binary_operation<bgr8, padd<bgr8>> },
	{ binoptp(pixelformat::bgra4, dseed::binary_operator::add), binary_operation<bgra4, padd<bgra4>> },
	{ binoptp(pixelformat::bgr565, dseed::binary_operator::add), binary_operation<bgr565, padd<bgr565>> },
	{ binoptp(pixelformat::r8, dseed::binary_operator::add), binary_operation<r8, padd<r8>> },
	{ binoptp(pixelformat::rf, dseed::binary_operator::add), binary_operation<rf, padd<rf>> },
	{ binoptp(pixelformat::yuva8, dseed::binary_operator::add), binary_operation<yuva8, padd<yuva8>> },
	{ binoptp(pixelformat::yuv8, dseed::binary_operator::add), binary_operation<yuv8, padd<yuv8>> },
	{ binoptp(pixelformat::hsva8, dseed::binary_operator::add), binary_operation<hsva8, padd<hsva8>> },
	{ binoptp(pixelformat::hsv8, dseed::binary_operator::add), binary_operation<hsv8, padd<hsv8>> },

	{ binoptp(pixelformat::rgba8, dseed::binary_operator::subtract), binary_operation<rgba8, psubtract<rgba8>> },
	{ binoptp(pixelformat::rgb8, dseed::binary_operator::subtract), binary_operation<rgb8, psubtract<rgb8>> },
	{ binoptp(pixelformat::rgbaf, dseed::binary_operator::subtract), binary_operation<rgbaf, psubtract<rgbaf>> },
	{ binoptp(pixelformat::bgra8, dseed::binary_operator::subtract), binary_operation<bgra8, psubtract<bgra8>> },
	{ binoptp(pixelformat::bgr8, dseed::binary_operator::subtract), binary_operation<bgr8, psubtract<bgr8>> },
	{ binoptp(pixelformat::bgra4, dseed::binary_operator::subtract), binary_operation<bgra4, psubtract<bgra4>> },
	{ binoptp(pixelformat::bgr565, dseed::binary_operator::subtract), binary_operation<bgr565, psubtract<bgr565>> },
	{ binoptp(pixelformat::r8, dseed::binary_operator::subtract), binary_operation<r8, psubtract<r8>> },
	{ binoptp(pixelformat::rf, dseed::binary_operator::subtract), binary_operation<rf, psubtract<rf>> },
	{ binoptp(pixelformat::yuva8, dseed::binary_operator::subtract), binary_operation<yuva8, psubtract<yuva8>> },
	{ binoptp(pixelformat::yuv8, dseed::binary_operator::subtract), binary_operation<yuv8, psubtract<yuv8>> },
	{ binoptp(pixelformat::hsva8, dseed::binary_operator::subtract), binary_operation<hsva8, psubtract<hsva8>> },
	{ binoptp(pixelformat::hsv8, dseed::binary_operator::subtract), binary_operation<hsv8, psubtract<hsv8>> },

	{ binoptp(pixelformat::rgba8, dseed::binary_operator::multiply), binary_operation<rgba8, pmultiply<rgba8>> },
	{ binoptp(pixelformat::rgb8, dseed::binary_operator::multiply), binary_operation<rgb8, pmultiply<rgb8>> },
	{ binoptp(pixelformat::rgbaf, dseed::binary_operator::multiply), binary_operation<rgbaf, pmultiply<rgbaf>> },
	{ binoptp(pixelformat::bgra8, dseed::binary_operator::multiply), binary_operation<bgra8, pmultiply<bgra8>> },
	{ binoptp(pixelformat::bgr8, dseed::binary_operator::multiply), binary_operation<bgr8, pmultiply<bgr8>> },
	{ binoptp(pixelformat::bgra4, dseed::binary_operator::multiply), binary_operation<bgra4, pmultiply<bgra4>> },
	{ binoptp(pixelformat::bgr565, dseed::binary_operator::multiply), binary_operation<bgr565, pmultiply<bgr565>> },
	{ binoptp(pixelformat::r8, dseed::binary_operator::multiply), binary_operation<r8, pmultiply<r8>> },
	{ binoptp(pixelformat::rf, dseed::binary_operator::multiply), binary_operation<rf, pmultiply<rf>> },
	{ binoptp(pixelformat::yuva8, dseed::binary_operator::multiply), binary_operation<yuva8, pmultiply<yuva8>> },
	{ binoptp(pixelformat::yuv8, dseed::binary_operator::multiply), binary_operation<yuv8, pmultiply<yuv8>> },
	{ binoptp(pixelformat::hsva8, dseed::binary_operator::multiply), binary_operation<hsva8, pmultiply<hsva8>> },
	{ binoptp(pixelformat::hsv8, dseed::binary_operator::multiply), binary_operation<hsv8, pmultiply<hsv8>> },

	{ binoptp(pixelformat::rgba8, dseed::binary_operator::divide), binary_operation<rgba8, pdivide<rgba8>> },
	{ binoptp(pixelformat::rgb8, dseed::binary_operator::divide), binary_operation<rgb8, pdivide<rgb8>> },
	{ binoptp(pixelformat::rgbaf, dseed::binary_operator::divide), binary_operation<rgbaf, pdivide<rgbaf>> },
	{ binoptp(pixelformat::bgra8, dseed::binary_operator::divide), binary_operation<bgra8, pdivide<bgra8>> },
	{ binoptp(pixelformat::bgr8, dseed::binary_operator::divide), binary_operation<bgr8, pdivide<bgr8>> },
	{ binoptp(pixelformat::bgra4, dseed::binary_operator::divide), binary_operation<bgra4, pdivide<bgra4>> },
	{ binoptp(pixelformat::bgr565, dseed::binary_operator::divide), binary_operation<bgr565, pdivide<bgr565>> },
	{ binoptp(pixelformat::r8, dseed::binary_operator::divide), binary_operation<r8, pdivide<r8>> },
	{ binoptp(pixelformat::rf, dseed::binary_operator::divide), binary_operation<rf, pdivide<rf>> },
	{ binoptp(pixelformat::yuva8, dseed::binary_operator::divide), binary_operation<yuva8, pdivide<yuva8>> },
	{ binoptp(pixelformat::yuv8, dseed::binary_operator::divide), binary_operation<yuv8, pdivide<yuv8>> },
	{ binoptp(pixelformat::hsva8, dseed::binary_operator::divide), binary_operation<hsva8, pdivide<hsva8>> },
	{ binoptp(pixelformat::hsv8, dseed::binary_operator::divide), binary_operation<hsv8, pdivide<hsv8>> },

	{ binoptp(pixelformat::rgba8, dseed::binary_operator::and), binary_operation<rgba8, pand<rgba8>> },
	{ binoptp(pixelformat::rgb8, dseed::binary_operator::and), binary_operation<rgb8, pand<rgb8>> },
	{ binoptp(pixelformat::bgra8, dseed::binary_operator::and), binary_operation<bgra8, pand<bgra8>> },
	{ binoptp(pixelformat::bgr8, dseed::binary_operator::and), binary_operation<bgr8, pand<bgr8>> },
	{ binoptp(pixelformat::bgra4, dseed::binary_operator::and), binary_operation<bgra4, pand<bgra4>> },
	{ binoptp(pixelformat::bgr565, dseed::binary_operator::and), binary_operation<bgr565, pand<bgr565>> },
	{ binoptp(pixelformat::r8, dseed::binary_operator::and), binary_operation<r8, pand<r8>> },
	{ binoptp(pixelformat::yuva8, dseed::binary_operator::and), binary_operation<yuva8, pand<yuva8>> },
	{ binoptp(pixelformat::yuv8, dseed::binary_operator::and), binary_operation<yuv8, pand<yuv8>> },
	{ binoptp(pixelformat::hsva8, dseed::binary_operator::and), binary_operation<hsva8, pand<hsva8>> },
	{ binoptp(pixelformat::hsv8, dseed::binary_operator::and), binary_operation<hsv8, pand<hsv8>> },

	{ binoptp(pixelformat::rgba8, dseed::binary_operator:: or), binary_operation<rgba8, por<rgba8>> },
	{ binoptp(pixelformat::rgb8, dseed::binary_operator:: or), binary_operation<rgb8, por<rgb8>> },
	{ binoptp(pixelformat::bgra8, dseed::binary_operator:: or), binary_operation<bgra8, por<bgra8>> },
	{ binoptp(pixelformat::bgr8, dseed::binary_operator:: or), binary_operation<bgr8, por<bgr8>> },
	{ binoptp(pixelformat::bgra4, dseed::binary_operator:: or), binary_operation<bgra4, por<bgra4>> },
	{ binoptp(pixelformat::bgr565, dseed::binary_operator:: or), binary_operation<bgr565, por<bgr565>> },
	{ binoptp(pixelformat::r8, dseed::binary_operator:: or), binary_operation<r8, por<r8>> },
	{ binoptp(pixelformat::yuva8, dseed::binary_operator:: or), binary_operation<yuva8, por<yuva8>> },
	{ binoptp(pixelformat::yuv8, dseed::binary_operator:: or), binary_operation<yuv8, por<yuv8>> },
	{ binoptp(pixelformat::hsva8, dseed::binary_operator:: or), binary_operation<hsva8, por <hsva8>> },
	{ binoptp(pixelformat::hsv8, dseed::binary_operator:: or), binary_operation<hsv8, por <hsv8>> },

	{ binoptp(pixelformat::rgba8, dseed::binary_operator::xor), binary_operation<rgba8, pxor<rgba8>> },
	{ binoptp(pixelformat::rgb8, dseed::binary_operator::xor), binary_operation<rgb8, pxor<rgb8>> },
	{ binoptp(pixelformat::bgra8, dseed::binary_operator::xor), binary_operation<bgra8, pxor<bgra8>> },
	{ binoptp(pixelformat::bgr8, dseed::binary_operator::xor), binary_operation<bgr8, pxor<bgr8>> },
	{ binoptp(pixelformat::bgra4, dseed::binary_operator::xor), binary_operation<bgra4, pxor<bgra4>> },
	{ binoptp(pixelformat::bgr565, dseed::binary_operator::xor), binary_operation<bgr565, pxor<bgr565>> },
	{ binoptp(pixelformat::r8, dseed::binary_operator::xor), binary_operation<r8, pxor<r8>> },
	{ binoptp(pixelformat::yuva8, dseed::binary_operator::xor), binary_operation<yuva8, pxor<yuva8>> },
	{ binoptp(pixelformat::yuv8, dseed::binary_operator::xor), binary_operation<yuv8, pxor<yuv8>> },
	{ binoptp(pixelformat::hsva8, dseed::binary_operator::xor), binary_operation<hsva8, pxor<hsva8>> },
	{ binoptp(pixelformat::hsv8, dseed::binary_operator::xor), binary_operation<hsv8, pxor<hsv8>> },
};

dseed::error_t dseed::bitmaps::bitmap_binary_operation(dseed::bitmaps::bitmap* b1, dseed::bitmaps::bitmap* b2, dseed::binary_operator op, dseed::bitmaps::bitmap** bitmap)
{
	if (b1 == nullptr || b2 == nullptr)
		return dseed::error_invalid_args;

	size3i b1Size = b1->size();
	size3i b2Size = b2->size();

	if (b1->type() != b2->type()
		|| b1Size.width != b2Size.width || b1Size.height != b2Size.height || b1Size.depth != b2Size.depth
		|| b1->format() != b2->format())
		return dseed::error_invalid_args;

	dseed::autoref<dseed::bitmaps::bitmap> temp;
	if (bitmap != nullptr)
	{
		if (dseed::failed(dseed::bitmaps::create_bitmap(b1->type(), b1->size(), b1->format(), nullptr, &temp)))
			return dseed::error_fail;
	}
	else
		temp = b1;

	uint8_t* destPtr, * src1Ptr, * src2Ptr;
	b1->lock((void**)&src1Ptr);
	b2->lock((void**)&src2Ptr);
	temp->lock((void**)&destPtr);

	auto found = g_binops.find(binoptp(b1->format(), op));
	if (found == g_binops.end())
		return dseed::error_not_support;

	if (!found->second(destPtr, src1Ptr, src2Ptr, b1->size()))
		return dseed::error_fail;

	temp->unlock();
	b2->unlock();
	b1->unlock();

	*bitmap = temp.detach();

	return dseed::error_good;
}

using unopfn = std::function<bool(uint8_t*, const uint8_t*, const dseed::size3i&)>;
using unoptp = std::tuple<pixelformat, dseed::unary_operator>;

template<class TPixel, Operator1<TPixel> op>
inline bool unary_operation(uint8_t* dest, const uint8_t* src, const dseed::size3i& size) noexcept
{
	size_t stride = calc_bitmap_stride(type2format<TPixel>(), size.width);
	size_t depth = calc_bitmap_plane_size(type2format<TPixel>(), size2i(size.width, size.height));

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
				TPixel result = op(pixel);
				*(destPtr + x) = result;
			}
		}
	}

	return true;
}

std::map<unoptp, unopfn> g_unops = {
	{ unoptp(pixelformat::rgbaf, dseed::unary_operator::negate), unary_operation<rgbaf, pnegate<rgbaf>> },
	{ unoptp(pixelformat::rf, dseed::unary_operator::negate), unary_operation<rf, pnegate<rf>> },

	{ unoptp(pixelformat::rgba8, dseed::unary_operator::not), unary_operation<rgba8, pnot<rgba8>> },
	{ unoptp(pixelformat::rgb8, dseed::unary_operator::not), unary_operation<rgb8, pnot<rgb8>> },
	{ unoptp(pixelformat::bgra8, dseed::unary_operator::not), unary_operation<bgra8, pnot<bgra8>> },
	{ unoptp(pixelformat::bgr8, dseed::unary_operator::not), unary_operation<bgr8, pnot<bgr8>> },
	{ unoptp(pixelformat::bgra4, dseed::unary_operator::not), unary_operation<bgra4, pnot<bgra4>> },
	{ unoptp(pixelformat::bgr565, dseed::unary_operator::not), unary_operation<bgr565, pnot<bgr565>> },
	{ unoptp(pixelformat::r8, dseed::unary_operator::not), unary_operation<r8, pnot<r8>> },
	{ unoptp(pixelformat::yuva8, dseed::unary_operator::not), unary_operation<yuva8, pnot<yuva8>> },
	{ unoptp(pixelformat::yuv8, dseed::unary_operator::not), unary_operation<yuv8, pnot<yuv8>> },
	{ unoptp(pixelformat::hsva8, dseed::unary_operator::not), unary_operation<hsva8, pnot<hsva8>> },
	{ unoptp(pixelformat::hsv8, dseed::unary_operator::not), unary_operation<hsv8, pnot<hsv8>> },

	{ unoptp(pixelformat::rgba8, dseed::unary_operator::invert), unary_operation<rgba8, pinvert<rgba8>> },
	{ unoptp(pixelformat::rgb8, dseed::unary_operator::invert), unary_operation<rgb8, pinvert<rgb8>> },
	{ unoptp(pixelformat::rgbaf, dseed::unary_operator::invert), unary_operation<rgbaf, pinvert<rgbaf>> },
	{ unoptp(pixelformat::bgra8, dseed::unary_operator::invert), unary_operation<bgra8, pinvert<bgra8>> },
	{ unoptp(pixelformat::bgr8, dseed::unary_operator::invert), unary_operation<bgr8, pinvert<bgr8>> },
	{ unoptp(pixelformat::bgra4, dseed::unary_operator::invert), unary_operation<bgra4, pinvert<bgra4>> },
	{ unoptp(pixelformat::bgr565, dseed::unary_operator::invert), unary_operation<bgr565, pinvert<bgr565>> },
	{ unoptp(pixelformat::r8, dseed::unary_operator::invert), unary_operation<r8, pinvert<r8>> },
	{ unoptp(pixelformat::rf, dseed::unary_operator::invert), unary_operation<rf, pinvert<rf>> },
	{ unoptp(pixelformat::yuva8, dseed::unary_operator::invert), unary_operation<yuva8, pinvert<yuva8>> },
	{ unoptp(pixelformat::yuv8, dseed::unary_operator::invert), unary_operation<yuv8, pinvert<yuv8>> },
	{ unoptp(pixelformat::hsva8, dseed::unary_operator::invert), unary_operation<hsva8, pinvert<hsva8>> },
	{ unoptp(pixelformat::hsv8, dseed::unary_operator::invert), unary_operation<hsv8, pinvert<hsv8>> },
};

dseed::error_t dseed::bitmaps::bitmap_unary_operation(dseed::bitmaps::bitmap* b, dseed::unary_operator op, dseed::bitmaps::bitmap** bitmap)
{
	if (b == nullptr)
		return dseed::error_invalid_args;

	dseed::autoref<dseed::bitmaps::bitmap> temp;
	if (bitmap != nullptr)
	{
		if (dseed::failed(dseed::bitmaps::create_bitmap(b->type(), b->size(), b->format(), nullptr, &temp)))
			return dseed::error_fail;
	}
	else
		temp = b;

	uint8_t* destPtr, * srcPtr;
	b->lock((void**)&srcPtr);
	temp->lock((void**)&destPtr);

	auto found = g_unops.find(unoptp(b->format(), op));
	if (found == g_unops.end())
		return dseed::error_not_support;

	if (!found->second(destPtr, srcPtr, b->size()))
		return dseed::error_not_support;

	temp->unlock();
	b->unlock();

	*bitmap = temp.detach();

	return error_good;
}
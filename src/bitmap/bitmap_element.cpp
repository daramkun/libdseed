#include <dseed.h>

dseed::error_t dseed::bitmaps::bitmap_split_rgb_elements(bitmap* original, bitmap** r, bitmap** g, bitmap** b, bitmap** a)
{
	const auto bitmaptype = original->type();
	const auto format = original->format();
	const auto size = original->size();
	const auto stride = dseed::color::calc_bitmap_stride(format, size.width);

	dseed::color::pixelformat outputFormat = dseed::color::pixelformat::rf;
	auto bytesPerSampleOriginal = 0, bytesPerSampleSplit = 0;
	bool has_alpha = false;
	switch (format) {
	case dseed::color::pixelformat::rgb8:
	case dseed::color::pixelformat::bgr8:
	case dseed::color::pixelformat::yuv8:
	case dseed::color::pixelformat::hsv8:
		outputFormat = dseed::color::pixelformat::r8;
		bytesPerSampleOriginal = 3;
		bytesPerSampleSplit = 1;
		break;

	case dseed::color::pixelformat::bgr565:
	case dseed::color::pixelformat::bgra4:
		outputFormat = dseed::color::pixelformat::r8;
		bytesPerSampleOriginal = 2;
		bytesPerSampleSplit = 1;
		break;

	case dseed::color::pixelformat::rgba8:
	case dseed::color::pixelformat::bgra8:
	case dseed::color::pixelformat::yuva8:
	case dseed::color::pixelformat::hsva8:
		outputFormat = dseed::color::pixelformat::r8;
		bytesPerSampleOriginal = 4;
		bytesPerSampleSplit = 1;
		has_alpha = true;
		break;

	case dseed::color::pixelformat::rgbaf:
		outputFormat = dseed::color::pixelformat::rf;
		bytesPerSampleOriginal = 16;
		bytesPerSampleSplit = 4;
		has_alpha = true;
		break;

	default:
		return dseed::error_invalid_args;
	}

	auto strideSplit = dseed::color::calc_bitmap_stride(outputFormat, size.width);

	dseed::autoref<dseed::bitmaps::bitmap> tr, tg, tb, ta;
	auto rr =
		r != nullptr
		? dseed::bitmaps::create_bitmap(bitmaptype, size, outputFormat, nullptr, &tr)
		: dseed::error_good;
	auto rg =
		g != nullptr
		? dseed::bitmaps::create_bitmap(bitmaptype, size, outputFormat, nullptr, &tg)
		: dseed::error_good;
	auto rb =
		b != nullptr
		? dseed::bitmaps::create_bitmap(bitmaptype, size, outputFormat, nullptr, &tb)
		: dseed::error_good;
	auto ra =
		has_alpha && a != nullptr
		? dseed::bitmaps::create_bitmap(bitmaptype, size, outputFormat, nullptr, &ta)
		: dseed::error_good;

	if (rr != dseed::error_good || rg != dseed::error_good || rb != dseed::error_good || ra != dseed::error_good)
		return dseed::error_fail;

	void* originalPtr;
	original->lock(&originalPtr);

	void* pr = nullptr,
		* pg = nullptr,
		* pb = nullptr,
		* pa = nullptr;
	if (tr) tr->lock(&pr);
	if (tg) tg->lock(&pg);
	if (tb) tb->lock(&pb);
	if (ta) ta->lock(&pa);

	for (auto z = 0; z < size.depth; ++z)
	{
		for (auto y = 0; y < size.height; ++y)
		{
			const auto yStrideOriginal = y * stride;
			const auto yStride = y * strideSplit;
			for (auto x = 0; x < size.width; ++x)
			{
				const auto xBytesPerSample = x * bytesPerSampleSplit;
				void* offsetPtrOriginal = static_cast<int8_t*>(originalPtr) + yStrideOriginal + (x * bytesPerSampleOriginal);
				void* offsetPtrRed = pr ? static_cast<int8_t*>(pr) + yStride + xBytesPerSample : nullptr;
				void* offsetPtrGreen = pg ? static_cast<int8_t*>(pg) + yStride + xBytesPerSample : nullptr;
				void* offsetPtrBlue = pb ? static_cast<int8_t*>(pb) + yStride + xBytesPerSample : nullptr;
				void* offsetPtrAlpha = pa ? static_cast<int8_t*>(pa) + yStride + xBytesPerSample : nullptr;

				switch (format) {
				case dseed::color::pixelformat::rgb8:
				case dseed::color::pixelformat::bgr8:
				case dseed::color::pixelformat::yuv8:
				case dseed::color::pixelformat::hsv8:
				{
					const auto origin = static_cast<const dseed::color::rgb8*>(offsetPtrOriginal);
					if (offsetPtrRed) *static_cast<uint8_t*>(offsetPtrRed) = origin->r;
					if (offsetPtrGreen) *static_cast<uint8_t*>(offsetPtrGreen) = origin->g;
					if (offsetPtrBlue) *static_cast<uint8_t*>(offsetPtrBlue) = origin->b;
					break;
				}

				case dseed::color::pixelformat::bgr565:
				{
					const auto origin = static_cast<const dseed::color::bgr565*>(offsetPtrOriginal);
					const dseed::color::bgra8 bgra8 = *origin;
					if (offsetPtrRed) *static_cast<uint8_t*>(offsetPtrRed) = bgra8.r;
					if (offsetPtrGreen) *static_cast<uint8_t*>(offsetPtrGreen) = bgra8.g;
					if (offsetPtrBlue) *static_cast<uint8_t*>(offsetPtrBlue) = bgra8.b;
					break;
				}
					
				case dseed::color::pixelformat::bgra4:
				{
					const auto origin = static_cast<const dseed::color::bgra4*>(offsetPtrOriginal);
					const dseed::color::bgra8 bgra8 = *origin;
					if (offsetPtrRed) *static_cast<uint8_t*>(offsetPtrRed) = bgra8.r;
					if (offsetPtrGreen) *static_cast<uint8_t*>(offsetPtrGreen) = bgra8.g;
					if (offsetPtrBlue) *static_cast<uint8_t*>(offsetPtrBlue) = bgra8.b;
					if (offsetPtrAlpha) *static_cast<uint8_t*>(offsetPtrBlue) = bgra8.a;
					break;
				}

				case dseed::color::pixelformat::rgba8:
				case dseed::color::pixelformat::bgra8:
				case dseed::color::pixelformat::yuva8:
				case dseed::color::pixelformat::hsva8:
				{
					const auto origin = static_cast<const dseed::color::rgba8*>(offsetPtrOriginal);
					if (offsetPtrRed) *static_cast<uint8_t*>(offsetPtrRed) = origin->r;
					if (offsetPtrGreen) *static_cast<uint8_t*>(offsetPtrGreen) = origin->g;
					if (offsetPtrBlue) *static_cast<uint8_t*>(offsetPtrBlue) = origin->b;
					if (offsetPtrAlpha) *static_cast<uint8_t*>(offsetPtrAlpha) = origin->a;
					break;
				}

				case dseed::color::pixelformat::rgbaf:
				{
					const auto origin = static_cast<const dseed::color::rgbaf*>(offsetPtrOriginal);
					if (offsetPtrRed) *static_cast<float*>(offsetPtrRed) = origin->r;
					if (offsetPtrGreen) *static_cast<float*>(offsetPtrGreen) = origin->g;
					if (offsetPtrBlue) *static_cast<float*>(offsetPtrBlue) = origin->b;
					if (offsetPtrAlpha) *static_cast<float*>(offsetPtrAlpha) = origin->a;
					break;
				}

				default:
					return dseed::error_invalid_op;
				}
			}
		}
	}

	original->unlock();

	if (ta) ta->unlock();
	if (tb) tb->unlock();
	if (tg) tg->unlock();
	if (tr) tr->unlock();

	if (r) *r = tr.detach();
	if (g) *g = tg.detach();
	if (b) *b = tb.detach();
	if (a) *a = ta.detach();

	return dseed::error_good;
}

dseed::error_t dseed::bitmaps::bitmap_join_rgb_elements(bitmap* r, bitmap* g, bitmap* b, bitmap* a, bitmap** rgba)
{
	if (!r || !g || !b)
		return dseed::error_invalid_args;

	const auto bitmaptype = r->type();
	const auto format = r->format();
	const auto size = r->size();
	const auto stride = dseed::color::calc_bitmap_stride(format, size.width);
	int bytesPerSampleSplit = 0;
	int bytesPerSampleOriginal = 0;

	if (format != g->format() || format != b->format() || (format != dseed::color::pixelformat::r8 && format != dseed::color::pixelformat::rf))
		return dseed::error_invalid_args;
	if (bitmaptype != g->type() || bitmaptype != b->type())
		return dseed::error_invalid_args;
	if (size != g->size() || size != b->size())
		return dseed::error_invalid_args;

	auto outputFormat = dseed::color::pixelformat::unknown;

	if (a)
	{
		if (a->format() != format || a->type() != bitmaptype || a->size() != size)
			return dseed::error_invalid_args;

		if (format == dseed::color::pixelformat::r8)
		{
			outputFormat = dseed::color::pixelformat::rgba8;
			bytesPerSampleSplit = 1;
			bytesPerSampleOriginal = 4;
		}
		else if (format == dseed::color::pixelformat::rf)
		{
			outputFormat = dseed::color::pixelformat::rgbaf;
			bytesPerSampleSplit = 4;
			bytesPerSampleOriginal = 16;
		}
		else
			return dseed::error_invalid_args;
	}
	else
	{
		if (format == dseed::color::pixelformat::r8)
		{
			outputFormat = dseed::color::pixelformat::rgb8;
			bytesPerSampleSplit = 1;
		}
		else
			return dseed::error_invalid_args;

		bytesPerSampleOriginal = 3;
	}

	if (dseed::failed(dseed::bitmaps::create_bitmap(bitmaptype, size, outputFormat, nullptr, rgba)))
		return dseed::error_fail;

	const auto outputStride = dseed::color::calc_bitmap_stride(outputFormat, size.width);

	void* rgbaPtr;
	(*rgba)->lock(&rgbaPtr);

	void* pr = nullptr,
		* pg = nullptr,
		* pb = nullptr,
		* pa = nullptr;
	r->lock(&pr);
	g->lock(&pg);
	b->lock(&pb);
	if (a) a->lock(&pa);

	for (auto z = 0; z < size.depth; ++z)
	{
		for (auto y = 0; y < size.height; ++y)
		{
			auto yStride = outputStride * y;
			auto yStrideElement = stride * y;

			for (auto x = 0; x < size.width; ++x)
			{
				const auto xBytesPerSample = x * bytesPerSampleSplit;
				void* offsetPtrOriginal = static_cast<int8_t*>(rgbaPtr) + yStride + (x * bytesPerSampleOriginal);
				void* offsetPtrRed = static_cast<int8_t*>(pr) + yStrideElement + xBytesPerSample;
				void* offsetPtrGreen = static_cast<int8_t*>(pg) + yStrideElement + xBytesPerSample;
				void* offsetPtrBlue = static_cast<int8_t*>(pb) + yStrideElement + xBytesPerSample;
				void* offsetPtrAlpha = pa ? static_cast<int8_t*>(pa) + yStrideElement + xBytesPerSample : nullptr;

				switch (outputFormat)
				{
				case dseed::color::pixelformat::rgb8:
				{
					dseed::color::rgb8* r = static_cast<dseed::color::rgb8*>(offsetPtrOriginal);
					r->r = *((uint8_t*)offsetPtrRed);
					r->g = *((uint8_t*)offsetPtrGreen);
					r->b = *((uint8_t*)offsetPtrBlue);
				}
				break;

				case dseed::color::pixelformat::rgba8:
				{
					dseed::color::rgba8* r = static_cast<dseed::color::rgba8*>(offsetPtrOriginal);
					r->r = *((uint8_t*)offsetPtrRed);
					r->g = *((uint8_t*)offsetPtrGreen);
					r->b = *((uint8_t*)offsetPtrBlue);
					r->a = *((uint8_t*)offsetPtrAlpha);
				}
				break;

				case dseed::color::pixelformat::rgbaf:
				{
					dseed::color::rgbaf* r = static_cast<dseed::color::rgbaf*>(offsetPtrOriginal);
					r->r = *((float*)offsetPtrRed);
					r->g = *((float*)offsetPtrGreen);
					r->b = *((float*)offsetPtrBlue);
					r->a = *((float*)offsetPtrAlpha);
				}
				break;

				default:
					return dseed::error_invalid_op;
				}
			}
		}
	}

	if (a) a->unlock();
	b->unlock();
	g->unlock();
	r->unlock();

	(*rgba)->unlock();

	return dseed::error_good;
}
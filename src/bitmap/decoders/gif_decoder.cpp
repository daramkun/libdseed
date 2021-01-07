#include <dseed.h>

#include "common.hxx"

#if defined(USE_GIF)
#	include <gif_lib.h>
#endif

dseed::error_t dseed::bitmaps::create_gif_bitmap_decoder (dseed::io::stream* stream, dseed::bitmaps::bitmap_array** decoder) noexcept
{
#if defined(USE_GIF)
	int err;
	auto pgif = DGifOpen (stream, [](GifFileType* file, GifByteType* buf, int len) -> int
		{
			return (int) reinterpret_cast<dseed::io::stream*>(file->UserData)->read (buf, len);
		}, &err
	);
	if (pgif == nullptr)
		return dseed::error_fail;

	if (DGifSlurp (pgif) != GIF_OK)
		return dseed::error_fail;

	dseed::size3i size (pgif->SWidth, pgif->SHeight, 1);
	size_t imageCount = pgif->ImageCount;

	std::vector<dseed::color::bgra8> palette;
	if (pgif->SColorMap)
	{
		palette.resize (pgif->SColorMap->ColorCount);
		//memcpy (palette.data (), pgif->SColorMap->Colors, palette.size () * sizeof (dseed::bgra));
		for (int i = 0; i < palette.size (); ++i)
		{
			palette[i].b = pgif->SColorMap->Colors[i].Blue;
			palette[i].g = pgif->SColorMap->Colors[i].Green;
			palette[i].r = pgif->SColorMap->Colors[i].Red;
			palette[i].a = 255;
		}
	}

	for (int i = 0; i < pgif->ExtensionBlockCount; ++i)
	{
		if (pgif->ExtensionBlocks[i].Function == GRAPHICS_EXT_FUNC_CODE)
		{
			if (pgif->ExtensionBlocks[i].Bytes[3] != NO_TRANSPARENT_COLOR)
				palette[pgif->ExtensionBlocks[i].Bytes[3]].a = 0;
			break;
		}
	}

	dseed::autoref<dseed::bitmaps::palette> globalPaletteObj;
	if (dseed::failed (dseed::bitmaps::create_palette (palette.data (), 32, palette.size (), &globalPaletteObj)))
		return dseed::error_fail;

	std::vector<dseed::bitmaps::bitmap*> _bitmaps (imageCount);
	std::vector<dseed::timespan> _timespans (imageCount);
	std::vector<uint8_t> raster (size.width * size.height);
	for (int z = 0; z < imageCount; ++z)
	{
		auto& savedImage = pgif->SavedImages[z];

		dseed::autoref<dseed::bitmaps::palette> localPaletteObj = globalPaletteObj;

		std::vector<dseed::color::bgra8> innerPalette;
		dseed::color::bgra8* usingPalette = palette.data ();
		size_t usingPaletteSize = palette.size ();
		bool paletteChanged = false;
		if (pgif->SavedImages[z].ImageDesc.ColorMap)
		{
			innerPalette.resize (savedImage.ImageDesc.ColorMap->ColorCount);
			//memcpy (innerPalette.data (), savedImage.ImageDesc.ColorMap->Colors, innerPalette.size ());
			for (int i = 0; i < palette.size (); ++i)
			{
				innerPalette[i].b = savedImage.ImageDesc.ColorMap->Colors[i].Blue;
				innerPalette[i].g = savedImage.ImageDesc.ColorMap->Colors[i].Green;
				innerPalette[i].r = savedImage.ImageDesc.ColorMap->Colors[i].Red;
				innerPalette[i].a = 255;
			}
			usingPalette = innerPalette.data ();
			usingPaletteSize = innerPalette.size ();
			paletteChanged = true;
		}

		int transparent = -1;
		for (int i = 0; i < savedImage.ExtensionBlockCount; ++i)
		{
			if (savedImage.ExtensionBlocks[i].Function == GRAPHICS_EXT_FUNC_CODE)
			{
				int delayTime = (savedImage.ExtensionBlocks[i].Bytes[1] | (savedImage.ExtensionBlocks[i].Bytes[2] << 8)) * 10;
				_timespans[z] = dseed::timespan::from_milliseconds (delayTime);
				if (savedImage.ExtensionBlocks[i].Bytes[3] != NO_TRANSPARENT_COLOR)
				{
					usingPalette[transparent = savedImage.ExtensionBlocks[i].Bytes[3]].a = 0;
					paletteChanged = true;
				}
				int disposal = (savedImage.ExtensionBlocks[i].Bytes[0] >> 2) & 0x7;
				switch (disposal)
				{
				case DISPOSE_BACKGROUND:
					for (int y = 0; y < size.height; ++y)
					{
						for (int x = 0; x < size.width; ++x)
							*(raster.data () + (y * size.width) + x) = pgif->SBackGroundColor;
					}
					break;
				}
				break;
			}
		}

		for (int y = savedImage.ImageDesc.Top; y < savedImage.ImageDesc.Top + savedImage.ImageDesc.Height; ++y)
		{
			for (int x = savedImage.ImageDesc.Left; x < savedImage.ImageDesc.Left + savedImage.ImageDesc.Width; ++x)
			{
				uint8_t value = *(savedImage.RasterBits + ((y - savedImage.ImageDesc.Top) * savedImage.ImageDesc.Width) + (x - savedImage.ImageDesc.Left));
				if (!(/*value == pgif->SBackGroundColor || */value == transparent))
					*(raster.data () + (y * size.width) + x) = value;
			}
		}

		if (paletteChanged)
		{
			localPaletteObj.release ();
			if (dseed::failed (dseed::bitmaps::create_palette (usingPalette, 32, usingPaletteSize, &localPaletteObj)))
				return dseed::error_fail;
		}

		if (dseed::failed (dseed::bitmaps::create_bitmap (dseed::bitmaps::bitmaptype::bitmap2d, size, dseed::color::pixelformat::bgra8_indexed8,
			localPaletteObj, &_bitmaps[z])))
			return dseed::error_fail;

		void* ptr;
		if (dseed::failed (_bitmaps[z]->lock (&ptr)))
			return dseed::error_fail;

		memcpy (ptr, raster.data (), raster.size ());

		_bitmaps[z]->unlock ();
	}

	DGifCloseFile (pgif, &err);

	*decoder = new dseed::__common_bitmap_array (_bitmaps, _timespans);

	return dseed::error_good;
#else
	return dseed::error_not_support;
#endif
}
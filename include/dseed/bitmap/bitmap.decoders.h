#ifndef __DSEED_BITMAP_DECODERS_H__
#define __DSEED_BITMAP_DECODERS_H__

namespace dseed
{
	DSEEDEXP error_t create_windows_imaging_codec_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder);

	DSEEDEXP error_t create_dib_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder);
	DSEEDEXP error_t create_tga_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder);

	DSEEDEXP error_t create_dds_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder);
	DSEEDEXP error_t create_ktx_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder);
	DSEEDEXP error_t create_pkm_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder);

	DSEEDEXP error_t create_png_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder);
	DSEEDEXP error_t create_jpeg_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder);
	DSEEDEXP error_t create_jpeg2000_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder);
	DSEEDEXP error_t create_webp_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder);
	DSEEDEXP error_t create_tiff_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder);
	DSEEDEXP error_t create_gif_bitmap_decoder (dseed::stream* stream, dseed::bitmap_decoder** decoder);
}

#endif
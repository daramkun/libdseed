#include <dseed.h>

#if defined(USE_VORBIS)
#	include <vorbis/vorbisfile.h>
#	include <vector>

#	include "common.hxx"

class __vorbis_decoder : public dseed::media::media_decoder
{
public:
	__vorbis_decoder ()
		: _refCount (1), _file (), _byterate (0)
	{ }
	~__vorbis_decoder ()
	{

	}

public:
	virtual int32_t retain () override { return ++_refCount; }
	virtual int32_t release () override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	dseed::error_t initialize (dseed::io::stream* stream)
	{
		if (_file.datasource != nullptr)
			ov_clear (&_file);

		memset (&_file, 0, sizeof (OggVorbis_File));
		_stream = stream;

		int result = ov_open_callbacks (stream, &_file, nullptr, 0,
			{
				[] (void* ptr, size_t size, size_t nmemb, void* datasource) -> size_t
				{
					dseed::io::stream* stream = (dseed::io::stream*)datasource;
					int64_t readed = stream->read ((uint8_t*)ptr, size * nmemb);
					return (size_t)(readed / size);
				},
				[](void* datasource, ogg_int64_t offset, int whence) -> int
				{
					dseed::io::stream* stream = (dseed::io::stream*)datasource;
					return (int)stream->seek ((dseed::io::seekorigin)whence, offset);
				},
				[](void* datasource) -> int
				{
					return 0;
				},
				[](void* datasource) -> long
				{
					dseed::io::stream* stream = (dseed::io::stream*)datasource;
					return (long)stream->position ();
				}
			});

		if (result != 0)
		{
			ov_clear (&_file);
			switch (result)
			{
				case OV_EREAD: return dseed::error_io;
				case OV_ENOTVORBIS: return dseed::error_not_support;
				case OV_EVERSION: return dseed::error_not_support;
				case OV_EBADHEADER: return dseed::error_not_support;
				case OV_EFAULT: return dseed::error_fail;
				default: return dseed::error_fail;
			}
		}

		_byterate = (_file.vi->channels * 2 * _file.vi->rate);

		return dseed::error_good;
	}

public:
	virtual dseed::media::sampletype type (size_t streamIndex) override
	{
		if (streamIndex != 0)
			return dseed::media::sampletype_out_of_range;
		return dseed::media::sampletype_audio;
	}
	virtual dseed::error_t format (size_t streamIndex, void* buffer, size_t bufferSize) override
	{
		if (streamIndex != 0)
			return dseed::error_out_of_range;
		if (buffer == nullptr || bufferSize != sizeof (dseed::media::audioformat))
			return dseed::error_invalid_args;

		*((dseed::media::audioformat*)buffer) = dseed::media::audioformat (_file.vi->channels, 16, _file.vi->rate, dseed::media::pulseformat::pcm);

		return dseed::error_good;
	}

public:
	virtual dseed::timespan duration () override
	{
		return dseed::timespan::from_seconds (ov_time_total (&_file, -1));
	}

public:
	virtual dseed::timespan reading_position () override
	{
		return dseed::timespan::from_seconds (ov_time_tell (&_file));
	}
	virtual dseed::error_t set_reading_position (dseed::timespan pos) override
	{
		ov_time_seek (&_file, pos.total_seconds ());
		return dseed::error_good;
	}

public:
	virtual dseed::error_t decode (size_t streamIndex, dseed::media::media_sample** sample) override
	{
		if (streamIndex != 0)
			return dseed::error_out_of_range;
		if (sample == nullptr)
			return dseed::error_invalid_args;

		int length = _byterate / 100;
		std::vector<uint8_t> buffer (length);
		int bitstream;
		dseed::timespan current = dseed::timespan::from_seconds (ov_time_tell (&_file));
		long result = ov_read (&_file, (char*)buffer.data (), length, 0, 2, 1, &bitstream);

		if (result == 0)
			return dseed::error_end_of_file;
		else if (result == OV_HOLE || result == OV_EBADLINK || result == OV_EINVAL)
			return dseed::error_fail;

		dseed::timespan duration = dseed::timespan::from_seconds (length / (double)_byterate);

		*sample = new __common_media_sample (dseed::media::sampletype_audio, buffer.data (), current, duration, _byterate);

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;

	dseed::autoref<dseed::io::stream> _stream;

	OggVorbis_File _file;
	int _byterate;
};
#endif

dseed::error_t dseed::media::create_vorbis_media_decoder (dseed::io::stream* stream, dseed::media::media_decoder** decoder)
{
#if defined(USE_VORBIS)
	dseed::autoref<__vorbis_decoder> temp = new __vorbis_decoder ();
	if (temp == nullptr)
		return dseed::error_out_of_memory;

	dseed::error_t err;
	if (dseed::failed (err = temp->initialize (stream)))
		return err;

	*decoder = temp.detach ();

	return dseed::error_good;
#else
	return dseed::error_not_support;
#endif
}
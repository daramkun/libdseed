#include <dseed.h>

int main (int argc, char* argv[])
{
	dseed::auto_object<dseed::stream> inputStream;
	if (dseed::failed (dseed::create_native_filestream (u8"..\\..\\..\\sample\\audios\\sample1.mp3", false, &inputStream)))
		return -1;

	dseed::auto_object<dseed::media_decoder> decoder;
	if (dseed::failed (dseed::create_mp3_media_decoder (inputStream, &decoder)))
		return -2;

	size_t streamIndex;
	for (size_t i = 0;; ++i)
	{
		auto type = decoder->sample_type (i);
		if (type == dseed::sampletype_audio)
		{
			streamIndex = i;
			break;
		}
		else if (type == dseed::sampletype_out_of_range)
			return -3;
	}

	dseed::audioformat wf;
	decoder->sample_format (streamIndex, &wf, sizeof (dseed::audioformat));

	dseed::timespan_t duration = decoder->duration ();

	{
		dseed::auto_object<dseed::stream> outputStream;
		if (dseed::failed (dseed::create_native_filestream (u8"output.wav", true, &outputStream)))
			return -4;
		dseed::auto_object<dseed::media_encoder> encoder;
		if (dseed::failed (dseed::create_wav_media_encoder (outputStream, nullptr, &encoder)))
			return -5;
		if (dseed::failed (encoder->set_sample_format (0, dseed::sampletype_audio, &wf, sizeof (dseed::audioformat))))
			return -6;

		dseed::error_t err;
		while (true)
		{
			dseed::auto_object<dseed::media_sample> sample;
			if (dseed::failed (err = decoder->decode (streamIndex, &sample)))
			{
				if (err == dseed::error_end_of_file)
					break;
				else
					return -7;
			}

			if (dseed::failed (encoder->encode (0, sample)))
				break;

			dseed::timespan_t current = decoder->reading_position ();
			if (current > dseed::timespan_t::from_seconds (70))
				break;

			system ("cls");
			printf ("%02d:%02d:%02d.%03d/%02d:%02d:%02d.%03d",
				current.hours (), current.minutes (), current.seconds (), current.milliseconds (),
				duration.hours (), duration.minutes (), duration.seconds (), duration.milliseconds ());
		}

		if (dseed::failed (encoder->commit ()))
			return -8;
	}

	return 0;
}
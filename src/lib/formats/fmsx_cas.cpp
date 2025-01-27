// license:BSD-3-Clause
// copyright-holders:Sean Young

#include "fmsx_cas.h"

#include <cstring>


#define CAS_PERIOD        (16)
#define CAS_HEADER_PERIODS (4000)
#define CAS_EMPTY_PERIODS (1000)
static const uint8_t CasHeader[8] = { 0x1F,0xA6,0xDE,0xBA,0xCC,0x13,0x7D,0x74 };

static int cas_size;


/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/
static int fmsx_cas_to_wav_size (const uint8_t *casdata, int caslen)
{
	int     pos, size;

	if (caslen < 8) return -1;
	if (memcmp (casdata, CasHeader, sizeof (CasHeader) ) ) return -1;

	pos = size = 0;

	while (pos < caslen)
	{
		if ( (pos + 8) < caslen)
			if (!memcmp (casdata + pos, CasHeader, 8) )
			{
				size += (CAS_EMPTY_PERIODS + CAS_HEADER_PERIODS) * CAS_PERIOD;
				pos += 8;
				continue;
			}

		size += CAS_PERIOD * 12;
		pos++;
	}

	cas_size = caslen;

	return size;
}


/*******************************************************************
   Generate samples for the tape image
********************************************************************/
static int fmsx_cas_fill_wave(int16_t *buffer, int sample_count, const uint8_t *bytes)
{
	int cas_pos, bit, state = 1, samples_pos, size, n, i, p;

	cas_pos = 0;
	samples_pos = 0;

	while (samples_pos < sample_count && cas_pos < cas_size)
	{
		/* Check if we need to output a header */
		if ( cas_pos + 8 < cas_size )
		{
			if ( ! memcmp( bytes + cas_pos, CasHeader, 8 ) )
			{
				/* Write CAS_EMPTY_PERIODS of silence */
				n = CAS_EMPTY_PERIODS * CAS_PERIOD; while (n--) buffer[samples_pos++] = 0;

				/* Write CAS_HEADER_PERIODS of header (high frequency) */
				for (i=0;i<CAS_HEADER_PERIODS*4;i++)
				{
					for (n=0;n<CAS_PERIOD / 4;n++)
						buffer[samples_pos + n] = (state ? 32767 : -32767);

					samples_pos += CAS_PERIOD / 4 ;
					state = !state;
				}

				cas_pos += 8;
			}
		}

		for (i=0;i<=11;i++)
		{
			if (i == 0) bit = 0;
			else if (i < 9) bit = (bytes[cas_pos] & (1 << (i - 1) ) );
			else bit = 1;

			/* write this one bit */
			for (n=0;n<(bit ? 4 : 2);n++)
			{
				size = (bit ? CAS_PERIOD / 4 : CAS_PERIOD / 2);
				for (p=0;p<size;p++)
				{
					buffer[samples_pos + p] = (state ? 32767 : -32767);
				}
				state = !state;
				samples_pos += size;
			}
		}
		cas_pos++;
	}

	return samples_pos;
}


static const cassette_image::LegacyWaveFiller fmsx_legacy_fill_wave =
{
	fmsx_cas_fill_wave,                     /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	fmsx_cas_to_wav_size,                   /* chunk_sample_calc */
	22050,                                  /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};



static cassette_image::error fmsx_cas_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &fmsx_legacy_fill_wave);
}



static cassette_image::error fmsx_cas_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&fmsx_legacy_fill_wave);
}



static const cassette_image::Format fmsx_cas_format =
{
	"tap,cas",
	fmsx_cas_identify,
	fmsx_cas_load,
	nullptr
};



CASSETTE_FORMATLIST_START(fmsx_cassette_formats)
	CASSETTE_FORMAT(fmsx_cas_format)
CASSETTE_FORMATLIST_END

// license:BSD-3-Clause
// copyright-holders:Sean Young
#include <assert.h>

#include "svi_cas.h"

#define CAS_PERIOD_0        (37)
#define CAS_PERIOD_1        (18)
#define CAS_HEADER_PERIODS (1600)
#define CAS_EMPTY_SAMPLES (24220)
#define CAS_INIT_SAMPLES    (200)

static const UINT8 CasHeader[17] =
{
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x7f
};

#define SMPLO   -32768
#define SMPHI   32767

static int cas_size;

/*******************************************************************
   Generate samples for the tape image
********************************************************************/
static int svi_cas_fill_wave(INT16 *buffer, int sample_count, UINT8 *bytes)
{
	int cas_pos, samples_pos, n, i;

	cas_pos = 17;
	samples_pos = 0;

	/* write CAS_INIT_SAMPLES of silence */
	n = CAS_INIT_SAMPLES; while (n--) buffer[samples_pos++] = 0;

	while (samples_pos < sample_count && cas_pos < cas_size)
	{
		/* write CAS_HEADER_PERIODS of header */
		for (i=0;i<CAS_HEADER_PERIODS;i++)
		{
			/* write a "0" */
			n = !(i % 4) ? 21 : 18;
			while (n--) buffer[samples_pos++] = SMPHI;
			n = 19; while (n--) buffer[samples_pos++] = SMPLO;
			/* write a "1" */
			n = 9; while (n--) buffer[samples_pos++] = SMPHI;
			n = 9; while (n--) buffer[samples_pos++] = SMPLO;
		}

		/* write "0x7f" */
		/* write a "0" */
		n = 21; while (n--) buffer[samples_pos++] = SMPHI;
		n = 19; while (n--) buffer[samples_pos++] = SMPLO;

		for (i=0;i<7;i++)
		{
			/* write a "1" */
			n = 9; while (n--) buffer[samples_pos++] = SMPHI;
			n = 9; while (n--) buffer[samples_pos++] = SMPLO;
		}

		while (samples_pos < sample_count && cas_pos < cas_size)
		{
			n = 21; while (n--) buffer[samples_pos++] = SMPHI;
			n = 19; while (n--) buffer[samples_pos++] = SMPLO;

			for (i=0;i<8;i++)
			{
				int bit = (bytes[cas_pos] & (0x80 >> i) );

				/* write this one bit */
				if (bit)
				{
					/* write a "1" */
					n = 9; while (n--) buffer[samples_pos++] = SMPHI;
					n = 9; while (n--) buffer[samples_pos++] = SMPLO;
				}
				else
				{
					/* write a "0" */
					n = 18; while (n--) buffer[samples_pos++] = SMPHI;
					n = 19; while (n--) buffer[samples_pos++] = SMPLO;
				}
			}

			cas_pos++;

			/* check if we've hit a new header (or end of block) */
			if ( (cas_pos + 17) < cas_size)
			{
				if (!memcmp (bytes + cas_pos, CasHeader, 17) )
				{
					cas_pos += 17;

					/* end of block marker */
					n = CAS_EMPTY_SAMPLES; while (n--) buffer[samples_pos++] = SMPHI;

					break; /* falls back to loop above; plays header again */
				}
			}
		}
	}

	/* end of block marker */
	n = CAS_EMPTY_SAMPLES; while (n--) buffer[samples_pos++] = SMPHI;

	return samples_pos;
}


/*******************************************************************
   Calculate the number of samples needed for this tape image
********************************************************************/
static int svi_cas_to_wav_size(const UINT8 *casdata, int caslen)
{
	int cas_pos, samples_pos, size, i;

	if (caslen < 17) return -1;
	if (memcmp (casdata, CasHeader, sizeof (CasHeader) ) ) return -1;

	cas_size = caslen;

	cas_pos = 17;

	samples_pos = CAS_INIT_SAMPLES;

	while (cas_pos < caslen)
	{
		size = CAS_HEADER_PERIODS * ( CAS_PERIOD_0 + CAS_PERIOD_1 ) +
				( CAS_HEADER_PERIODS / 4 ) * 3;

		samples_pos += size;

		samples_pos += 21 + 19 + 7 * CAS_PERIOD_1;

		while (cas_pos < caslen)
		{
			samples_pos += 21;
			samples_pos += 19;

			for (i=0;i<8;i++)
			{
				int bit = (casdata[cas_pos] & (0x80 >> i) );

				samples_pos += bit ? CAS_PERIOD_1 : CAS_PERIOD_0;
			}

			cas_pos++;

			/* check if we've hit a new header (or end of block) */
			if ( (cas_pos + 17) < caslen)
			{
				if (!memcmp (casdata + cas_pos, CasHeader, 17) )
				{
					cas_pos += 17;

					/* end of block marker */
					samples_pos += CAS_EMPTY_SAMPLES;

					break; /* falls back to loop above; plays header again */
				}
			}
		}
	}

	/* end of block marker */
	samples_pos += CAS_EMPTY_SAMPLES;

	return samples_pos;
}


static const struct CassetteLegacyWaveFiller svi_legacy_fill_wave =
{
	svi_cas_fill_wave,                      /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	svi_cas_to_wav_size,                    /* chunk_sample_calc */
	44100,                                  /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};



static casserr_t svi_cas_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &svi_legacy_fill_wave);
}



static casserr_t svi_cas_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &svi_legacy_fill_wave);
}



static const struct CassetteFormat svi_cas_format =
{
	"cas",
	svi_cas_identify,
	svi_cas_load,
	nullptr
};



CASSETTE_FORMATLIST_START(svi_cassette_formats)
	CASSETTE_FORMAT(svi_cas_format)
CASSETTE_FORMATLIST_END

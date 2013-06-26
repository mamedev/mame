/* .APT tape images */


#include "formats/apf_apt.h"


#define WAVEENTRY_LOW  -32768
#define WAVEENTRY_HIGH  32767
#define WAVEENTRY_NULL  0

/* frequency of wave */
#define APF_WAV_FREQUENCY   11050

#define APF_APT_BIT_0_LENGTH ((int)(APF_WAV_FREQUENCY*0.0005))
#define APF_APT_BIT_1_LENGTH ((int)(APF_WAV_FREQUENCY*0.001))

struct apf_t
{
	int cassette_length;
};
static apf_t apf;


/* 500 microsecond of bit 0 and 1000 microsecond of bit 1 */

static INT16 *apf_emit_level(INT16 *p, int count, INT16 wave_state)
{
	int i;

	for (i=0; i<count; i++)
	{
		*(p++) = wave_state;
	}
	return p;
}

/* 4 periods at 1200Hz */
static INT16* apf_output_bit(INT16 *p, UINT8 b)
{
	if (b)
	{
		p = apf_emit_level(p, (APF_APT_BIT_1_LENGTH>>1), WAVEENTRY_HIGH);
		p = apf_emit_level(p, (APF_APT_BIT_1_LENGTH>>1), WAVEENTRY_LOW);
	}
	else
	{
		p = apf_emit_level(p, (APF_APT_BIT_0_LENGTH>>1), WAVEENTRY_HIGH);
		p = apf_emit_level(p, (APF_APT_BIT_0_LENGTH>>1), WAVEENTRY_LOW);
	}

	return p;
}

static int apf_get_bit_size_in_samples(UINT8 b)
{
	if (b)
	{
		return APF_APT_BIT_1_LENGTH;
	}

	return APF_APT_BIT_0_LENGTH;
}


/*************************************************************************************/

/* each bit in the data represents a "1" or "0" waveform */
static int apf_cassette_calculate_size_in_samples(const UINT8 *bytes, int length)
{
	unsigned i;
	int size;
	int b;
	UINT8 data;

	size = 0;
	apf.cassette_length = length;


	for (i=0; i<length; i++)
	{
		data = bytes[i];

		for (b=0; b<8; b++)
		{
			size += apf_get_bit_size_in_samples((data>>7) & 0x01);
			data = data<<1;
		}
	}

	return size;
}

/*************************************************************************************/

static int apf_cassette_fill_wave(INT16 *buffer, int length, UINT8 *bytes)
{
	int i;
	INT16 *p;
	UINT8 data;
	int b;

	p = buffer;

	for (i=0; i<apf.cassette_length; i++)
	{
		data = bytes[i];
		for (b=0; b<8; b++)
		{
			p = apf_output_bit(p,(data>>7) & 0x01);
			data = data<<1;
		}
	}
	return p - buffer;
}



static const struct CassetteLegacyWaveFiller apf_legacy_fill_wave =
{
	apf_cassette_fill_wave,                 /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	apf_cassette_calculate_size_in_samples, /* chunk_sample_calc */
	APF_WAV_FREQUENCY,                      /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};



static casserr_t apf_apt_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &apf_legacy_fill_wave);
}



static casserr_t apf_apt_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &apf_legacy_fill_wave);
}



static const struct CassetteFormat apf_apt_format =
{
	"apt",
	apf_apt_identify,
	apf_apt_load,
	NULL
};



CASSETTE_FORMATLIST_START(apf_cassette_formats)
	CASSETTE_FORMAT(apf_apt_format)
CASSETTE_FORMATLIST_END

// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
#include "formats/vt_cas.h"

/*********************************************************************
    vtech 1/2 agnostic
*********************************************************************/

#define SILENCE 8000

static int generic_fill_wave(int16_t *buffer, int length, const uint8_t *code, int bitsamples, int bytesamples, int lo, int16_t *(*fill_wave_byte)(int16_t *buffer, int byte))
{
	static int nullbyte;

	if( code == CODE_HEADER )
	{
		int i;

		if( length < SILENCE )
			return -1;
		for( i = 0; i < SILENCE; i++ )
			*buffer++ = 0;

		nullbyte = 0;

		return SILENCE;
	}

	if( code == CODE_TRAILER )
	{
		int i;

		/* silence at the end */
		for( i = 0; i < length; i++ )
			*buffer++ = 0;
		return length;
	}

	if( length < bytesamples )
		return -1;

	buffer = fill_wave_byte(buffer, *code);

	if( !nullbyte && *code == 0 )
	{
		int i;
		for( i = 0; i < 2*bitsamples; i++ )
			*buffer++ = lo;
		nullbyte = 1;
		return bytesamples + 2 * bitsamples;
	}

	return bytesamples;
}


/*********************************************************************
    vtech 1
*********************************************************************/

#define V1_LO   -32768
#define V1_HI   +32767

#define V1_BITSAMPLES   6
#define V1_BYTESAMPLES  8*V1_BITSAMPLES

static int16_t *vtech1_fill_wave_byte(int16_t *buffer, int byte)
{
	int i;

	for( i = 7; i >= 0; i-- )
	{
		*buffer++ = V1_HI;  /* initial cycle */
		*buffer++ = V1_LO;
		if( (byte >> i) & 1 )
		{
			*buffer++ = V1_HI; /* two more cycles */
			*buffer++ = V1_LO;
			*buffer++ = V1_HI;
			*buffer++ = V1_LO;
		}
		else
		{
			*buffer++ = V1_HI; /* one slow cycle */
			*buffer++ = V1_HI;
			*buffer++ = V1_LO;
			*buffer++ = V1_LO;
		}
	}
	return buffer;
}

static int vtech1_cassette_fill_wave(int16_t *buffer, int length, const uint8_t *code)
{
	return generic_fill_wave(buffer, length, code, V1_BITSAMPLES, V1_BYTESAMPLES, V1_LO, vtech1_fill_wave_byte);
}

static const cassette_image::LegacyWaveFiller vtech1_legacy_fill_wave =
{
	vtech1_cassette_fill_wave,  // fill_wave
	1,                          // chunk_size
	V1_BYTESAMPLES,             // chunk_samples
	nullptr,                    // chunk_sample_calc
	600*V1_BITSAMPLES,          // sample_frequency
	600*V1_BITSAMPLES,          // header_samples
	600*V1_BITSAMPLES           // trailer_samples
};

static cassette_image::error vtech1_cas_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &vtech1_legacy_fill_wave);
}

static cassette_image::error vtech1_cas_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&vtech1_legacy_fill_wave);
}

static const cassette_image::Format vtech1_cas_format =
{
	"cas",
	vtech1_cas_identify,
	vtech1_cas_load,
	nullptr
};

CASSETTE_FORMATLIST_START(vtech1_cassette_formats)
	CASSETTE_FORMAT(vtech1_cas_format)
CASSETTE_FORMATLIST_END

/*********************************************************************
    vtech 2
*********************************************************************/

#define VT2_LO  -20000
#define VT2_HI  +20000

#define VT2_BITSAMPLES  18
#define VT2_BYTESAMPLES 8*VT2_BITSAMPLES

static const int16_t vtech2_bit0[VT2_BITSAMPLES] =
{
	/* short cycle, long cycles */
	VT2_HI,VT2_HI,VT2_HI,VT2_LO,VT2_LO,VT2_LO,
	VT2_HI,VT2_HI,VT2_HI,VT2_HI,VT2_HI,VT2_HI,
	VT2_LO,VT2_LO,VT2_LO,VT2_LO,VT2_LO,VT2_LO
};

static const int16_t vtech2_bit1[VT2_BITSAMPLES] =
{
	/* three short cycle */
	VT2_HI,VT2_HI,VT2_HI,VT2_LO,VT2_LO,VT2_LO,
	VT2_HI,VT2_HI,VT2_HI,VT2_LO,VT2_LO,VT2_LO,
	VT2_HI,VT2_HI,VT2_HI,VT2_LO,VT2_LO,VT2_LO
};


static int16_t *vtech2_fill_wave_bit(int16_t *buffer, int bit)
{
	int i;
	if( bit )
	{
		for( i = 0; i < VT2_BITSAMPLES; i++ )
			*buffer++ = vtech2_bit1[i];
	}
	else
	{
		for( i = 0; i < VT2_BITSAMPLES; i++ )
			*buffer++ = vtech2_bit0[i];
	}
	return buffer;
}


static int16_t *vtech2_fill_wave_byte(int16_t *buffer, int byte)
{
	buffer = vtech2_fill_wave_bit(buffer, (byte >> 7) & 1);
	buffer = vtech2_fill_wave_bit(buffer, (byte >> 6) & 1);
	buffer = vtech2_fill_wave_bit(buffer, (byte >> 5) & 1);
	buffer = vtech2_fill_wave_bit(buffer, (byte >> 4) & 1);
	buffer = vtech2_fill_wave_bit(buffer, (byte >> 3) & 1);
	buffer = vtech2_fill_wave_bit(buffer, (byte >> 2) & 1);
	buffer = vtech2_fill_wave_bit(buffer, (byte >> 1) & 1);
	buffer = vtech2_fill_wave_bit(buffer, (byte >> 0) & 1);
	return buffer;
}

static int vtech2_cassette_fill_wave(int16_t *buffer, int length, const uint8_t *code)
{
	return generic_fill_wave(buffer, length, code, VT2_BITSAMPLES, VT2_BYTESAMPLES, VT2_LO, vtech2_fill_wave_byte);
}

static const cassette_image::LegacyWaveFiller vtech2_legacy_fill_wave =
{
	vtech2_cassette_fill_wave,  /* fill_wave */
	1,                          /* chunk_size */
	VT2_BYTESAMPLES,            /* chunk_samples */
	nullptr,                       /* chunk_sample_calc */
	600*VT2_BITSAMPLES,         /* sample_frequency */
	600*VT2_BITSAMPLES,         /* header_samples */
	600*VT2_BITSAMPLES          /* trailer_samples */
};

static cassette_image::error vtech2_cas_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &vtech2_legacy_fill_wave);
}

static cassette_image::error vtech2_cas_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&vtech2_legacy_fill_wave);
}

static const cassette_image::Format vtech2_cas_format =
{
	"cas",
	vtech2_cas_identify,
	vtech2_cas_load,
	nullptr
};

CASSETTE_FORMATLIST_START(vtech2_cassette_formats)
	CASSETTE_FORMAT(vtech2_cas_format)
CASSETTE_FORMATLIST_END

// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/********************************************************************

    Support for KC85 cassette images

    Supported formats:
    - kcc: raw cassette image without ID and checksum
    - tap: cassette image from KC-Emulator with head and ID
    - tp2: cassette image with ID and checksum (130 bytes block)
    - kcm: same as tp2 but without head
    - sss: BASIC data without head (miss the first 11 bytes)

********************************************************************/

#include "kc_cas.h"

#include <cstring>

#define SMPLO       -32768
#define SMPHI       32767
#define SILENCE     0

#define KC_WAV_FREQUENCY        44100

// from documentation
#define FREQ_BIT_0          2400
#define FREQ_BIT_1          1200
#define FREQ_SEPARATOR      600

// file formats
enum
{
	KC_IMAGE_KCC,
	KC_IMAGE_TP2,
	KC_IMAGE_TAP,
	KC_IMAGE_KCM
};

// image size
static int kc_image_size; // FIXME: global variable prevents multiple instances

/*******************************************************************
   Generate one high-low cycle of sample data
********************************************************************/
static inline int kc_cas_cycle(int16_t *buffer, int sample_pos, int len)
{
	int num_samples = KC_WAV_FREQUENCY / (len * 2);

	if (buffer)
	{
		for (int i=0; i<num_samples; i++)
			buffer[ sample_pos + i ] = SMPHI;

		for (int i=0; i<num_samples; i++)
			buffer[ sample_pos + num_samples + i ] = SMPLO;
	}

	return num_samples * 2;
}


/*******************************************************************
   Generate n samples of silence
********************************************************************/
static inline int kc_cas_silence(int16_t *buffer, int sample_pos, int len)
{
	int i = 0;

	if ( buffer )
		for( i = 0; i < len; i++)
			buffer[ sample_pos + i ] = SILENCE;

	return len;
}


/*******************************************************************
   Generate samples for 1 byte
********************************************************************/
static inline int kc_cas_byte(int16_t *buffer, int sample_pos, uint8_t data)
{
	int samples = 0;

	// write the byte
	for ( int i = 0; i < 8; i++ )
	{
		if ( data & 0x01 )
		{
			samples += kc_cas_cycle( buffer, sample_pos + samples, FREQ_BIT_1 );
		}
		else
		{
			samples += kc_cas_cycle( buffer, sample_pos + samples, FREQ_BIT_0 );
		}

		data >>= 1;
	}

	// byte separator
	samples += kc_cas_cycle( buffer, sample_pos + samples, FREQ_SEPARATOR);

	return samples;
}

static int kc_handle_cass(int16_t *buffer, const uint8_t *casdata, int type)
{
	int data_pos = (type == KC_IMAGE_KCC || type == KC_IMAGE_KCM) ? 0 : 16;
	int sample_count = 0;
	int block_id = 1;

	// 1 sec of silence at start
	sample_count += kc_cas_silence(buffer, sample_count, KC_WAV_FREQUENCY);

	// 8000 cycles of BIT_1 for synchronization
	for (int i=0; i<8000; i++)
		sample_count += kc_cas_cycle( buffer, sample_count, FREQ_BIT_1);

	// on the entire file
	while( data_pos < kc_image_size )
	{
		uint8_t checksum = 0;

		// 200 cycles of BIT_1 every block
		for (int i=0; i<200; i++)
			sample_count += kc_cas_cycle( buffer, sample_count, FREQ_BIT_1);

		// separator
		sample_count += kc_cas_cycle( buffer, sample_count, FREQ_SEPARATOR);

		// in TAP and TP2 file the first byte is the ID
		if (type == KC_IMAGE_TAP || type == KC_IMAGE_TP2 || type == KC_IMAGE_KCM)
			block_id = casdata[data_pos++];

		// is the last block ?
		if (data_pos + 128 >= kc_image_size && type == KC_IMAGE_KCC)
			block_id = 0xff;

		// write the block ID
		sample_count += kc_cas_byte( buffer, sample_count, block_id );

		// write the 128 bytes of the block
		for (int i=0; i<128; i++)
		{
			uint8_t data = 0;

			if (data_pos < kc_image_size)
				data = casdata[data_pos++];

			// calculate the checksum
			checksum += data;

			// write a byte
			sample_count += kc_cas_byte( buffer, sample_count, data );
		}

		// TP2 and KCM files also have the checksum byte
		if (type == KC_IMAGE_TP2 || type == KC_IMAGE_KCM)
			checksum = casdata[data_pos++];

		// 8bit checksum
		sample_count += kc_cas_byte( buffer, sample_count, checksum );

		// more TAP and TP2 can be combined into the same file
		if ((type == KC_IMAGE_TAP || type == KC_IMAGE_TP2) && block_id == 0xff && data_pos < kc_image_size)
		{
			if (casdata[data_pos] == 0xc3 || casdata[data_pos] == 0x4b)
			{
				sample_count += kc_cas_silence(buffer, sample_count, KC_WAV_FREQUENCY/10);

				data_pos += 16;
			}
		}

		block_id++;
	}

	sample_count += kc_cas_cycle( buffer, sample_count, FREQ_SEPARATOR);

	// 1 sec of silence
	sample_count += kc_cas_silence(buffer, sample_count, KC_WAV_FREQUENCY);

	return sample_count;
}


static int kc_handle_kcc(int16_t *buffer, const uint8_t *casdata)
{
	return kc_handle_cass(buffer, casdata, KC_IMAGE_KCC);
}


static int kc_handle_tap(int16_t *buffer, const uint8_t *casdata)
{
	if (!strncmp((const char *)(casdata + 1), "KC-TAPE by AF", 13))
	{
		return kc_handle_cass(buffer, casdata, KC_IMAGE_TAP);
	}
	else if (!strncmp((const char *)(casdata), "KC85", 4))
	{
		return kc_handle_cass(buffer, casdata, KC_IMAGE_TP2);
	}
	else if (casdata[0] == 0x01)
	{
		return kc_handle_cass(buffer, casdata, KC_IMAGE_KCM);
	}
	else
	{
		return (int)cassette_image::error::INVALID_IMAGE;
	}
}

static int kc_handle_sss(int16_t *buffer, const uint8_t *casdata)
{
	std::vector<uint8_t> sss(kc_image_size + 11);

	// tries to generate the missing head
	memset(&sss[0], 0xd3, 3);
	memset(&sss[3], 0x20, 8);
	memcpy(&sss[11], casdata, kc_image_size);

	// set an arbitrary filename
	sss[3] = 'A';

	int retval = kc_handle_cass(buffer, &sss[0], KC_IMAGE_KCC);

	return retval;
}



/*******************************************************************
   Generate samples for the tape image
********************************************************************/
static int kc_kcc_fill_wave(int16_t *buffer, int sample_count, const uint8_t *bytes)
{
	return kc_handle_kcc(buffer, bytes);
}


/*******************************************************************
   Calculate the number of samples needed for this tape image classical
********************************************************************/
static int kc_kcc_to_wav_size(const uint8_t *casdata, int caslen)
{
	kc_image_size = caslen;

	return kc_handle_kcc( nullptr, casdata );
}


static const cassette_image::LegacyWaveFiller kc_kcc_legacy_fill_wave =
{
	kc_kcc_fill_wave,                       /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	kc_kcc_to_wav_size,                     /* chunk_sample_calc */
	KC_WAV_FREQUENCY,                       /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static cassette_image::error kc_kcc_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &kc_kcc_legacy_fill_wave);
}


static cassette_image::error kc_kcc_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&kc_kcc_legacy_fill_wave);
}


static const cassette_image::Format kc_kcc_format =
{
	"kcc,kcb",
	kc_kcc_identify,
	kc_kcc_load,
	nullptr
};


/*******************************************************************
   Generate samples for the tape image
********************************************************************/
static int kc_tap_fill_wave(int16_t *buffer, int sample_count, const uint8_t *bytes)
{
	return kc_handle_tap(buffer, bytes);
}


/*******************************************************************
   Calculate the number of samples needed for this tape image classical
********************************************************************/
static int kc_tap_to_wav_size(const uint8_t *casdata, int caslen)
{
	kc_image_size = caslen;

	return kc_handle_tap( nullptr, casdata );
}


static const cassette_image::LegacyWaveFiller kc_tap_legacy_fill_wave =
{
	kc_tap_fill_wave,                       /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	kc_tap_to_wav_size,                     /* chunk_sample_calc */
	KC_WAV_FREQUENCY,                       /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static cassette_image::error kc_tap_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &kc_tap_legacy_fill_wave);
}


static cassette_image::error kc_tap_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&kc_tap_legacy_fill_wave);
}


static const cassette_image::Format kc_tap_format =
{
	"tap,853,854,855,tp2,kcm",
	kc_tap_identify,
	kc_tap_load,
	nullptr
};


/*******************************************************************
   Generate samples for the tape image
********************************************************************/
static int kc_sss_fill_wave(int16_t *buffer, int sample_count, const uint8_t *bytes)
{
	return kc_handle_sss(buffer, bytes);
}


/*******************************************************************
   Calculate the number of samples needed for this tape image classical
********************************************************************/
static int kc_sss_to_wav_size(const uint8_t *casdata, int caslen)
{
	kc_image_size = caslen;

	return kc_handle_sss( nullptr, casdata );
}


static const cassette_image::LegacyWaveFiller kc_sss_legacy_fill_wave =
{
	kc_sss_fill_wave,                       /* fill_wave */
	-1,                                     /* chunk_size */
	0,                                      /* chunk_samples */
	kc_sss_to_wav_size,                     /* chunk_sample_calc */
	KC_WAV_FREQUENCY,                       /* sample_frequency */
	0,                                      /* header_samples */
	0                                       /* trailer_samples */
};

static cassette_image::error kc_sss_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &kc_sss_legacy_fill_wave);
}


static cassette_image::error kc_sss_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&kc_sss_legacy_fill_wave);
}

static const cassette_image::Format kc_sss_format =
{
	"sss",
	kc_sss_identify,
	kc_sss_load,
	nullptr
};


CASSETTE_FORMATLIST_START(kc_cassette_formats)
	CASSETTE_FORMAT(kc_kcc_format)
	CASSETTE_FORMAT(kc_tap_format)
	CASSETTE_FORMAT(kc_sss_format)
CASSETTE_FORMATLIST_END

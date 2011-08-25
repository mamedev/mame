/* .PMD tape images */
#include "pmd_pmd.h"

#define WAVEENTRY_LOW  -32768
#define WAVEENTRY_HIGH  32767

#define PMD85_WAV_FREQUENCY	7200
#define PMD85_TIMER_FREQUENCY	1200
#define PMD85_BIT_LENGTH	(PMD85_WAV_FREQUENCY/PMD85_TIMER_FREQUENCY)
#define PMD85_PILOT_BITS	(PMD85_TIMER_FREQUENCY*3)
#define PMD85_PAUSE_BITS	(PMD85_TIMER_FREQUENCY/2)
#define PMD85_HEADER_BYTES	63
#define PMD85_BITS_PER_BYTE	11

static INT16 *pmd85_emit_level(INT16 *p, int count, int level)
{
	int i;

	for (i=0; i<count; i++)	*(p++) = level;

	return p;
}

static INT16* pmd85_output_bit(INT16 *p, UINT8 bit)
{
	if (bit)
	{
		p = pmd85_emit_level (p, PMD85_BIT_LENGTH/2, WAVEENTRY_LOW);
		p = pmd85_emit_level (p, PMD85_BIT_LENGTH/2, WAVEENTRY_HIGH);
	}
	else
	{
		p = pmd85_emit_level (p, PMD85_BIT_LENGTH/2, WAVEENTRY_HIGH);
		p = pmd85_emit_level (p, PMD85_BIT_LENGTH/2, WAVEENTRY_LOW);
	}
    	return p;
}

static INT16* pmd85_output_byte(INT16 *p, UINT8 byte)
{
	int i;

	/* start */
	p = pmd85_output_bit (p, 0);

	/* data */
	for (i=0; i<8; i++)
		p = pmd85_output_bit(p,(byte>>(7-i)) & 0x01);

	/* stop */
	p = pmd85_output_bit (p, 1);
	p = pmd85_output_bit (p, 1);

	return p;
}

static int pmd85_cassette_calculate_size_in_samples(const UINT8 *bytes, int length)
{
	return PMD85_BIT_LENGTH * (length * PMD85_BITS_PER_BYTE + PMD85_PILOT_BITS + PMD85_PAUSE_BITS);
}

static int pmd85_cassette_fill_wave(INT16 *buffer, int length, UINT8 *bytes)
{
	int i;
	INT16 * p = buffer;

	int data_size = ((length/PMD85_BIT_LENGTH-PMD85_PILOT_BITS-PMD85_PAUSE_BITS)/PMD85_BITS_PER_BYTE)-PMD85_HEADER_BYTES;

	/* pilot */
	for (i=0; i<PMD85_PILOT_BITS; i++)
		p = pmd85_output_bit(p, 1);

	/* header */
	for (i=0; i<PMD85_HEADER_BYTES; i++)
		p = pmd85_output_byte(p, bytes[i]);

	/* pause */
	for (i=0; i<PMD85_PAUSE_BITS; i++)
		p = pmd85_output_bit(p, 1);

	/* data */
	for (i=0; i<data_size; i++)
		p = pmd85_output_byte(p, bytes[i+PMD85_HEADER_BYTES]);

	return p - buffer;
}

static const struct CassetteLegacyWaveFiller pmd85_legacy_fill_wave =
{
	pmd85_cassette_fill_wave,			/* fill_wave */
	-1,											/* chunk_size */
	0,											/* chunk_samples */
	pmd85_cassette_calculate_size_in_samples,	/* chunk_sample_calc */
	PMD85_WAV_FREQUENCY,										/* sample_frequency */
	0,											/* header_samples */
	0											/* trailer_samples */
};

static casserr_t pmd85_pmd_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &pmd85_legacy_fill_wave);
}

static casserr_t pmd85_pmd_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &pmd85_legacy_fill_wave);
}

static const struct CassetteFormat pmd85_pmd_image_format =
{
	"pmd",
	pmd85_pmd_identify,
	pmd85_pmd_load,
	NULL
};

CASSETTE_FORMATLIST_START(pmd85_pmd_format)
	CASSETTE_FORMAT(pmd85_pmd_image_format)
CASSETTE_FORMATLIST_END

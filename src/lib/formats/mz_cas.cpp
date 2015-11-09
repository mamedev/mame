// license:BSD-3-Clause
// copyright-holders:Nathan Woods
#include <string.h>
#include <assert.h>
#include "mz_cas.h"

#ifndef VERBOSE
#define VERBOSE 0
#endif

//#define LOG(N,M,A)
//  if(VERBOSE>=N){ if( M )LOG_FORMATS("%11.6f: %-24s",machine.time().as_double(), (const char*)M ); LOG_FORMATS A; }

#define LOG(N,M,A)  \
	if(VERBOSE>=N){ if( M )printf("%-24s",(const char*)M ); printf A; }

#define LO  -32768
#define HI  +32767

#define SHORT_PULSE 2
#define LONG_PULSE  4

#define BYTE_SAMPLES (LONG_PULSE+8*LONG_PULSE)

#define SILENCE     8000

/* long gap and tape mark */
#define LGAP        22000
#define LTM_1       40
#define LTM_0       40
#define LTM_L       1

/* short gap and tape mark */
#define SGAP        11000
#define STM_1       20
#define STM_0       20
#define STM_L       1

static int fill_wave_1(INT16 *buffer, int offs)
{
	buffer[offs++] = HI;
	buffer[offs++] = HI;
	buffer[offs++] = LO;
	buffer[offs++] = LO;
	return LONG_PULSE;
}

static int fill_wave_0(INT16 *buffer, int offs)
{
	buffer[offs++] = HI;
	buffer[offs++] = LO;
	return SHORT_PULSE;
}

static int fill_wave_b(INT16 *buffer, int offs, int byte)
{
	int i, count = 0;

	/* data bits are preceded by a long pulse */
	count += fill_wave_1(buffer, offs + count);

	for( i = 7; i >= 0; i-- )
	{
		if( (byte >> i) & 1 )
			count += fill_wave_1(buffer, offs + count);
		else
			count += fill_wave_0(buffer, offs + count);
	}
	return count;
}

static int fill_wave(INT16 *buffer, int length, UINT8 *code)
{
	static INT16 *beg;
	static UINT16 csum = 0;
	static int header = 1, bytecount = 0;
	int count = 0;

	if( code == CODE_HEADER )
	{
		int i;

		/* is there insufficient space for the LGAP? */
		if( count + LGAP * SHORT_PULSE > length )
			return -1;
		LOG(1,"mz700 fill_wave",("LGAP %d samples\n", LGAP * SHORT_PULSE));
		/* fill long gap */
		for( i = 0; i < LGAP; i++ )
			count += fill_wave_0(buffer, count);

		/* make a long tape mark */
		/* is there insufficient space for the LTM 1? */
		if( count + LTM_1 * LONG_PULSE > length )
			return -1;
		LOG(1,"mz700_fill_wave",("LTM 1 %d samples\n", LTM_1 * LONG_PULSE));
		for( i = 0; i < LTM_1; i++ )
			count += fill_wave_1(buffer, count);

		/* is there insufficient space for the LTM 0? */
		if( count + LTM_0 * SHORT_PULSE > length )
			return -1;
		LOG(1,"mz700_fill_wave",("LTM 0 %d samples\n", LTM_0 * SHORT_PULSE));
		for( i = 0; i < LTM_0; i++ )
			count += fill_wave_0(buffer, count);

		/* is there insufficient space for the L? */
		if( count + LTM_L * LONG_PULSE > length )
			return -1;
		LOG(1,"mz700_fill_wave",("L %d samples\n", LONG_PULSE));
		count += fill_wave_1(buffer, count);

		/* reset header, bytecount and checksum */
		header = 1;
		bytecount = 0;
		csum = 0;

		/* HDR begins here */
		beg = buffer + count;

		return count;
	}

	if( code == CODE_TRAILER )
	{
		int i, file_length;
		INT16 *end = buffer;

		/* is there insufficient space for the CHKF? */
		if( count + 2 * BYTE_SAMPLES > length )
			return -1;
		LOG(1,"mz700_fill_wave",("CHKF 0x%04X\n", csum));
		count += fill_wave_b(buffer, count, csum >> 8);
		count += fill_wave_b(buffer, count, csum & 0xff);

		/* is there insufficient space for the L */
		if( count + LTM_L * LONG_PULSE > length )
			return -1;
		LOG(1,"mz700_fill_wave",("L\n"));
		count += fill_wave_1(buffer, count);

		/* is there insufficient space for the 256S pulses? */
		if( count + 256 * SHORT_PULSE > length )
			return -1;
		LOG(1,"mz700_fill_wave",("256S\n"));
		for (i = 0; i < 256; i++)
			count += fill_wave_0(buffer, count);

		file_length = (int)(end - beg) / sizeof(INT16);
		/* is there insufficient space for the FILEC ? */
		if( count + file_length > length )
			return -1;
		LOG(1,"mz700_fill_wave",("FILEC %d samples\n", file_length));
		memcpy(buffer + count, beg, file_length * sizeof(INT16));
		count += file_length;

		/* is there insufficient space for the CHKF ? */
		if( count + 2 * BYTE_SAMPLES > length )
			return -1;
		LOG(1,"mz700_fill_wave",("CHKF 0x%04X\n", csum));
		count += fill_wave_b(buffer, count, csum >> 8);
		count += fill_wave_b(buffer, count, csum & 0xff);

		/* is there insufficient space for the L ? */
		if( count + STM_L * LONG_PULSE > length )
			return -1;
		LOG(1,"mz700_fill_wave",("L %d samples\n", LONG_PULSE));
		count += fill_wave_1(buffer, count);

		LOG(1,"mz700_fill_wave",("silence %d samples\n", SILENCE));
		/* silence at the end */
		for( i = 0; i < SILENCE; i++ )
			buffer[count++] = 0;

		return count;
	}

	if( header == 1 && bytecount == 128 )
	{
		int i, hdr_length;
		INT16 *end = buffer;

		/* is there insufficient space for the CHKH ? */
		if( count + 2 * BYTE_SAMPLES > length )
			return -1;
		LOG(1,"mz700_fill_wave",("CHKH 0x%04X\n", csum & 0xffff));
		count += fill_wave_b(buffer, count, (csum >> 8) & 0xff);
		count += fill_wave_b(buffer, count, csum & 0xff);

		/* is there insufficient space for the L ? */
		if( count + LONG_PULSE > length )
			return -1;
		LOG(1,"mz700_fill_wave",("L %d samples\n", LONG_PULSE));
		count += fill_wave_1(buffer, count);

		/* is there insufficient space for the 256S ? */
		if( count + 256 * SHORT_PULSE > length )
			return -1;
		LOG(1,"mz700_fill_wave",("256S\n"));
		for (i = 0; i < 256; i++)
			count += fill_wave_0(buffer, count);

		hdr_length = (int)(end - beg) / sizeof(INT16);
		/* is there insufficient space for the HDRC ? */
		if( count + hdr_length > length )
			return -1;
		LOG(1,"mz700_fill_wave",("HDRC %d samples\n", hdr_length));
		memcpy(buffer + count, beg, hdr_length * sizeof(INT16));
		count += hdr_length;

		/* is there insufficient space for CHKH ? */
		if( count + 2 * BYTE_SAMPLES > length )
			return -1;
		LOG(1,"mz700_fill_wave",("CHKH 0x%04X\n", csum & 0xffff));
		count += fill_wave_b(buffer, count, (csum >> 8) & 0xff);
		count += fill_wave_b(buffer, count, csum & 0xff);

		/* is there insufficient space for the L ? */
		if( count + LONG_PULSE > length )
			return -1;
		LOG(1,"mz700_fill_wave",("L %d samples\n", LONG_PULSE));
		count += fill_wave_1(buffer, count);

		/* is there sufficient space for the SILENCE? */
		if( count + SILENCE > length )
			return -1;
		LOG(1,"mz700_fill_wave",("SILENCE %d samples\n", SILENCE));
		/* fill silence */
		for( i = 0; i < SILENCE; i++ )
			buffer[count++] = 0;

		/* is there sufficient space for the SGAP? */
		if( count + SGAP * SHORT_PULSE > length )
			return -1;
		LOG(1,"mz700_fill_wave",("SGAP %d samples\n", SGAP * SHORT_PULSE));
		/* fill short gap */
		for( i = 0; i < SGAP; i++ )
			count += fill_wave_0(buffer, count);

		/* make a short tape mark */

		/* is there sufficient space for the STM 1? */
		if( count + STM_1 * LONG_PULSE > length )
			return -1;
		LOG(1,"mz700_fill_wave",("STM 1 %d samples\n", STM_1 * LONG_PULSE));
		for( i = 0; i < STM_1; i++ )
			count += fill_wave_1(buffer, count);

		/* is there sufficient space for the STM 0? */
		if( count + STM_0 * SHORT_PULSE > length )
			return -1;
		LOG(1,"mz700_fill_wave",("STM 0 %d samples\n", STM_0 * SHORT_PULSE));
		for( i = 0; i < STM_0; i++ )
			count += fill_wave_0(buffer, count);

		/* is there sufficient space for the L? */
		if( count + STM_L * LONG_PULSE > length )
			return -1;
		LOG(1,"mz700_fill_wave",("L %d samples\n", LONG_PULSE));
		count += fill_wave_1(buffer, count);

		bytecount = 0;
		header = 0;
		csum = 0;

		/* FILE begins here */
		beg = buffer + count;
	}

	if( length < BYTE_SAMPLES )
		return -1;

	if (*code & 0x01) csum++;
	if (*code & 0x02) csum++;
	if (*code & 0x04) csum++;
	if (*code & 0x08) csum++;
	if (*code & 0x10) csum++;
	if (*code & 0x20) csum++;
	if (*code & 0x40) csum++;
	if (*code & 0x80) csum++;

	bytecount++;

	count += fill_wave_b(buffer, count, *code);

	return count;
}

#define MZ700_WAVESAMPLES_HEADER    (   \
		LGAP * SHORT_PULSE +            \
		LTM_1 * LONG_PULSE +            \
		LTM_0 * SHORT_PULSE +           \
		LTM_L * LONG_PULSE +            \
		2 * 2 * BYTE_SAMPLES +          \
	SILENCE +                           \
	SGAP * SHORT_PULSE +                \
		STM_1 * LONG_PULSE +            \
		STM_0 * SHORT_PULSE +           \
		STM_L * LONG_PULSE +            \
		2 * 2 * BYTE_SAMPLES)




static const struct CassetteLegacyWaveFiller mz700_legacy_fill_wave =
{
	fill_wave,                  /* fill_wave */
	1,                          /* chunk_size */
	2 * BYTE_SAMPLES,           /* chunk_samples */
	NULL,                       /* chunk_sample_calc */
	4400,                       // sample_frequency (tested ok with MZ-80K, MZ-80A, MZ-700, MZ-800, MZ-1500)
	MZ700_WAVESAMPLES_HEADER,   /* header_samples */
	1                           /* trailer_samples */
};



static casserr_t mz700_cas_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	return cassette_legacy_identify(cassette, opts, &mz700_legacy_fill_wave);
}



static casserr_t mz700_cas_load(cassette_image *cassette)
{
	return cassette_legacy_construct(cassette, &mz700_legacy_fill_wave);
}



static const struct CassetteFormat mz700_cas_format =
{
	"m12,mzf,mzt",
	mz700_cas_identify,
	mz700_cas_load,
	NULL
};



CASSETTE_FORMATLIST_START(mz700_cassette_formats)
	CASSETTE_FORMAT(mz700_cas_format)
CASSETTE_FORMATLIST_END

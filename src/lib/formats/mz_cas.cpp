// license:BSD-3-Clause
// copyright-holders:Nathan Woods
#include "mz_cas.h"

#include "util/coretmpl.h" // BIT

#include <cstring>

#ifndef VERBOSE
#define VERBOSE 0
#endif

//#define LOG(N,M,A)
//  if (VERBOSE>=N) { if( M )LOG_FORMATS("%11.6f: %-24s",machine.time().as_double(), (const char*)M ); LOG_FORMATS A; }

#define LOG(N,M,A)  \
	do { if (VERBOSE >= (N)) { if (M) printf("%-24s", (const char*)M); printf A; } } while (false)

namespace {

struct mz_cass_params
{
	int SHORT_PULSE, LONG_PULSE;
	int SILENCE;
	int LGAP, LTM_1, LTM_0, LTM_L;
	int SGAP, STM_1, STM_0, STM_L;

	constexpr int byte_samples() const { return LONG_PULSE + (8 * LONG_PULSE); }
	constexpr int wavesamples_header() const
	{
		return
				LGAP * SHORT_PULSE +
				LTM_1 * LONG_PULSE +
				LTM_0 * SHORT_PULSE +
				LTM_L * LONG_PULSE +
				2 * 2 * byte_samples() +
				SILENCE +
				SGAP * SHORT_PULSE +
				STM_1 * LONG_PULSE +
				STM_0 * SHORT_PULSE +
				STM_L * LONG_PULSE +
				2 * 2 * byte_samples();
	}
};

constexpr int LO = -32768;
constexpr int HI = +32767;

static constexpr mz_cass_params MZ700_PARAMS
{
	2,      // SHORT_PULSE
	4,      // LONG_PULSE

	8000,   // SILENCE

	// long gap and tape mark
	22000,  // LGAP
	40,     // LTM_1
	40,     // LTM_0
	1,      // LTM_L

	11000,  // SGAP
	20,     // STM_1
	20,     // STM_0
	1       // STM_L
};

// MZ-80B @ 48 kHz, LGAP 10k, SGAP 5k, 16/32 sample pulses for sharp edges
// 16 samples = 333µs, 32 samples = 666µs (8x MZ-700 resolution)
static constexpr mz_cass_params MZ80B_PARAMS
{
	16,     // SHORT_PULSE
	32,     // LONG_PULSE

	8000,   // SILENCE

	// long gap and tape mark
	10000,  // LGAP - MZ-80B uses 10,000 pulses (vs 22,000 for MZ-700)
	40,     // LTM_1
	40,     // LTM_0
	1,      // LTM_L

	5000,   // SGAP - MZ-80B uses 5,000 pulses (proportional to LGAP)
	20,     // STM_1
	20,     // STM_0
	1       // STM_L
};

template <const mz_cass_params &Params>
int fill_wave_0(int16_t *buffer, int offs)
{
	for (int i = 0; i < Params.SHORT_PULSE / 2; i++)
		buffer[offs + i] = HI;
	for (int i = Params.SHORT_PULSE / 2; i < Params.SHORT_PULSE; i++)
		buffer[offs + i] = LO;
	return Params.SHORT_PULSE;
}

template <const mz_cass_params &Params>
int fill_wave_1(int16_t *buffer, int offs)
{
	for (int i = 0; i < Params.LONG_PULSE / 2; i++)
		buffer[offs + i] = HI;
	for (int i = Params.LONG_PULSE / 2; i < Params.LONG_PULSE; i++)
		buffer[offs + i] = LO;
	return Params.LONG_PULSE;
}

template <const mz_cass_params &Params>
int fill_wave_b(int16_t *buffer, int offs, int byte)
{
	int count = 0;

	/* data bits are preceded by a long pulse */
	count += fill_wave_1<Params>(buffer, offs + count);

	for (int i = 7; i >= 0; i--)
	{
		if (util::BIT(byte, i))
			count += fill_wave_1<Params>(buffer, offs + count);
		else
			count += fill_wave_0<Params>(buffer, offs + count);
	}
	return count;
}

template <const mz_cass_params &Params>
int fill_wave(int16_t *buffer, int length, const uint8_t *code, int)
{
	const int BYTE_SAMPLES = Params.byte_samples();

	static int16_t *beg;
	static uint16_t csum = 0;
	static int header = 1, bytecount = 0;
	int count = 0;

	if (code == CODE_HEADER)
	{
		/* is there insufficient space for the LGAP? */
		if (count + Params.LGAP * Params.SHORT_PULSE > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("LGAP %d samples\n", Params.LGAP * Params.SHORT_PULSE));
		/* fill long gap */
		for (int i = 0; i < Params.LGAP; i++)
			count += fill_wave_0<Params>(buffer, count);

		/* make a long tape mark */
		/* is there insufficient space for the LTM 1? */
		if (count + Params.LTM_1 * Params.LONG_PULSE > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("LTM 1 %d samples\n", Params.LTM_1 * Params.LONG_PULSE));
		for (int i = 0; i < Params.LTM_1; i++)
			count += fill_wave_1<Params>(buffer, count);

		/* is there insufficient space for the LTM 0? */
		if (count + Params.LTM_0 * Params.SHORT_PULSE > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("LTM 0 %d samples\n", Params.LTM_0 * Params.SHORT_PULSE));
		for (int i = 0; i < Params.LTM_0; i++)
			count += fill_wave_0<Params>(buffer, count);

		/* is there insufficient space for the L? */
		if (count + Params.LTM_L * Params.LONG_PULSE > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("L %d samples\n", Params.LONG_PULSE));
		count += fill_wave_1<Params>(buffer, count);

		/* reset header, bytecount and checksum */
		header = 1;
		bytecount = 0;
		csum = 0;

		/* HDR begins here */
		beg = buffer + count;

		return count;
	}

	if (code == CODE_TRAILER)
	{
		int16_t *end = buffer;

		/* is there insufficient space for the CHKF? */
		if (count + 2 * BYTE_SAMPLES > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("CHKF 0x%04X\n", csum));
		count += fill_wave_b<Params>(buffer, count, csum >> 8);
		count += fill_wave_b<Params>(buffer, count, csum & 0xff);

		/* is there insufficient space for the L */
		if (count + Params.LTM_L * Params.LONG_PULSE > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("L\n"));
		count += fill_wave_1<Params>(buffer, count);

		/* is there insufficient space for the 256S pulses? */
		if (count + 256 * Params.SHORT_PULSE > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("256S\n"));
		for (int i = 0; i < 256; i++)
			count += fill_wave_0<Params>(buffer, count);

		const int file_length = int(end - beg) / sizeof(int16_t);
		/* is there insufficient space for the FILEC ? */
		if (count + file_length > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("FILEC %d samples\n", file_length));
		memcpy(buffer + count, beg, file_length * sizeof(int16_t));
		count += file_length;

		/* is there insufficient space for the CHKF ? */
		if (count + 2 * BYTE_SAMPLES > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("CHKF 0x%04X\n", csum));
		count += fill_wave_b<Params>(buffer, count, csum >> 8);
		count += fill_wave_b<Params>(buffer, count, csum & 0xff);

		/* is there insufficient space for the L ? */
		if (count + Params.STM_L * Params.LONG_PULSE > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("L %d samples\n", Params.LONG_PULSE));
		count += fill_wave_1<Params>(buffer, count);

		LOG(1,"mz_cass fill_wave",("silence %d samples\n", Params.SILENCE));
		/* silence at the end */
		for (int i = 0; i < Params.SILENCE; i++)
			buffer[count++] = 0;

		return count;
	}

	if (header == 1 && bytecount == 128)
	{
		int16_t *end = buffer;

		/* is there insufficient space for the CHKH ? */
		if (count + 2 * BYTE_SAMPLES > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("CHKH 0x%04X\n", csum & 0xffff));
		count += fill_wave_b<Params>(buffer, count, (csum >> 8) & 0xff);
		count += fill_wave_b<Params>(buffer, count, csum & 0xff);

		/* is there insufficient space for the L ? */
		if (count + Params.LONG_PULSE > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("L %d samples\n", Params.LONG_PULSE));
		count += fill_wave_1<Params>(buffer, count);

		/* is there insufficient space for the 256S ? */
		if (count + 256 * Params.SHORT_PULSE > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("256S\n"));
		for (int i = 0; i < 256; i++)
			count += fill_wave_0<Params>(buffer, count);

		const int hdr_length = int(end - beg) / sizeof(int16_t);
		/* is there insufficient space for the HDRC ? */
		if (count + hdr_length > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("HDRC %d samples\n", hdr_length));
		memcpy(buffer + count, beg, hdr_length * sizeof(int16_t));
		count += hdr_length;

		/* is there insufficient space for CHKH ? */
		if (count + 2 * BYTE_SAMPLES > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("CHKH 0x%04X\n", csum & 0xffff));
		count += fill_wave_b<Params>(buffer, count, (csum >> 8) & 0xff);
		count += fill_wave_b<Params>(buffer, count, csum & 0xff);

		/* is there insufficient space for the L ? */
		if (count + Params.LONG_PULSE > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("L %d samples\n", Params.LONG_PULSE));
		count += fill_wave_1<Params>(buffer, count);

		/* is there sufficient space for the SILENCE? */
		if (count + Params.SILENCE > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("SILENCE %d samples\n", Params.SILENCE));
		/* fill silence */
		for (int i = 0; i < Params.SILENCE; i++)
			buffer[count++] = 0;

		/* is there sufficient space for the SGAP? */
		if (count + Params.SGAP * Params.SHORT_PULSE > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("SGAP %d samples\n", Params.SGAP * Params.SHORT_PULSE));
		/* fill short gap */
		for (int i = 0; i < Params.SGAP; i++)
			count += fill_wave_0<Params>(buffer, count);

		/* make a short tape mark */

		/* is there sufficient space for the STM 1? */
		if (count + Params.STM_1 * Params.LONG_PULSE > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("STM 1 %d samples\n", Params.STM_1 * Params.LONG_PULSE));
		for (int i = 0; i < Params.STM_1; i++)
			count += fill_wave_1<Params>(buffer, count);

		/* is there sufficient space for the STM 0? */
		if (count + Params.STM_0 * Params.SHORT_PULSE > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("STM 0 %d samples\n", Params.STM_0 * Params.SHORT_PULSE));
		for (int i = 0; i < Params.STM_0; i++)
			count += fill_wave_0<Params>(buffer, count);

		/* is there sufficient space for the L? */
		if (count + Params.STM_L * Params.LONG_PULSE > length)
			return -1;
		LOG(1,"mz_cass fill_wave",("L %d samples\n", Params.LONG_PULSE));
		count += fill_wave_1<Params>(buffer, count);

		bytecount = 0;
		header = 0;
		csum = 0;

		/* FILE begins here */
		beg = buffer + count;
	}

	if (length < BYTE_SAMPLES)
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

	count += fill_wave_b<Params>(buffer, count, *code);

	return count;
}

} // anonymous namespace




// MZ-700/MZ-80A/MZ-700/MZ-800: 1200 baud Sharp PWM
static const cassette_image::LegacyWaveFiller mz700_legacy_fill_wave =
{
	fill_wave<MZ700_PARAMS>,            /* fill_wave */
	1,                                  /* chunk_size */
	2 * MZ700_PARAMS.byte_samples(),                   /* chunk_samples */
	nullptr,                            /* chunk_sample_calc */
	4400,                               /* sample_frequency (1200 baud) */
	MZ700_PARAMS.wavesamples_header(),  /* header_samples */
	1                                   /* trailer_samples */
};

// MZ-80B / MZ-2000: 1800 baud @ 48 kHz (Logic 0=~333µs, Logic 1=~667µs; CPU read @ 255µs)
// 48 kHz avoids upsampling jitter; 16/32 sample pulses give sharp edges for edge detection
static const cassette_image::LegacyWaveFiller mz80b_legacy_fill_wave =
{
	fill_wave<MZ80B_PARAMS>,            /* fill_wave: LGAP 10000, SGAP 5000, 16/32 sample pulses */
	1,                                  /* chunk_size */
	2 * MZ80B_PARAMS.byte_samples(),    /* chunk_samples */
	nullptr,                            /* chunk_sample_calc */
	48000,                              /* sample_frequency: native MAME rate, sharp edges */
	MZ80B_PARAMS.wavesamples_header(),  /* header_samples */
	16                                  /* trailer_samples: scaled for 48 kHz */
};



static cassette_image::error mz700_cas_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &mz700_legacy_fill_wave);
}



static cassette_image::error mz700_cas_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&mz700_legacy_fill_wave);
}



static cassette_image::error mz80b_cas_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	return cassette->legacy_identify(opts, &mz80b_legacy_fill_wave);
}



static cassette_image::error mz80b_cas_load(cassette_image *cassette)
{
	return cassette->legacy_construct(&mz80b_legacy_fill_wave);
}



static const cassette_image::Format mz700_cas_format =
{
	"m12,mzf,mzt",
	mz700_cas_identify,
	mz700_cas_load,
	nullptr
};



static const cassette_image::Format mz80b_cas_format =
{
	"m12,mzf,mzt",
	mz80b_cas_identify,
	mz80b_cas_load,
	nullptr
};



CASSETTE_FORMATLIST_START(mz700_cassette_formats)
	CASSETTE_FORMAT(mz700_cas_format)
CASSETTE_FORMATLIST_END

CASSETTE_FORMATLIST_START(mz80b_cassette_formats)
	CASSETTE_FORMAT(mz80b_cas_format)
CASSETTE_FORMATLIST_END

// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*

    Tape support for C16 / C64 / VIC20 TAP format


    Credits to:

        - Peter Schepers for the informations on C64 formats
        - Vice Team for the source of their very complete emulator
        - Attila G. for tap2wav (both the source and the actual tool)


    TODO:
    * to verify and fix (if needed) support for .TAP v2
    * to implement reading (and logging) the remaining part of the header
    * to verify if it is more accurate to use a different HIGH_WAVE value
      when the pulse corresponds to a 0x00 in the .TAP file
    (...)
    * (far away in the future) can this code be merged with TZX code?

*/

/* Info based on http://ist.uwaterloo.ca/~schepers/formats/TAP.TXT      */
/* Please refer to the webpage for the latest version and for a very
   complete listing of various cart types and their bankswitch tricks   */
/*

  Designed by Per Hakan Sundell (author of the CCS64 C64 emulator) in 1997,
this format attempts to duplicate the data stored on a C64  cassette  tape,
bit for bit. Since it is simply a representation of  the  raw  serial  data
from a tape, it should handle *any* custom tape loaders that exist.

  The TAP images are generally very large, being a minimum of eight  times,
and up to sixteen times as large as what a raw PRG file would be.  This  is
due to the way the data is stored, with each bit of the original  file  now
being one byte large in the TAP file. The layout is fairly simple,  with  a
small 14-byte header followed by file data.

    Bytes: $0000-000B: File signature "C64-TAPE-RAW"
                 000C: TAP version (see below for description)
                        $00 - Original layout
                         01 - Updated
            000D-000F: Future expansion
            0010-0013: File  data  size  (not  including  this  header,  in
                       LOW/HIGH format).
            0014-xxxx: File data

  In TAP version $00 files, each data byte in the file data area represents
the length of a pulse, when the C64's hardware  will  trigger  again.  This
pulse length is determined by the following formula:

    pulse length (in seconds) = (8 * data byte) / (clock cycles)

  Therefore, a data value of $2F (47 in decimal) would be:

    (47 * 8) / 985248 = .00038975 seconds.

  A data value of $00 represents an "overflow" condition, any pulse  length
which is more that 255 * 8 in length.

  The value of "clock cycles" from above  (985248)  is  based  on  the  PAL
value.  Since  this  file  format  was  developed  in  Europe,   which   is
predominantly PAL video, this is only logical.  The  NTSC  value  would  be
1022730, which is very close to  the  PAL,  and  therefore  won't  cause  a
compatibility problem converting European and NTSC tapes. I would stick  to
using the PAL value just in case.


  In TAP version $01 files, the data value of  $00  has  been  re-coded  to
represent values greater than 255 * 8. When a  $00  is  encountered,  three
bytes will follow which are the actual time (in cycles) of a pulse, and the
above formula does not apply.  The  three  bytes  are  stored  in  LOW/HIGH
format.


  The actual interpretation of the serial data takes a little more work  to
explain.  The  typical  ROM  tape  loader  (and  the  turbo  loaders)  will
initialize a timer with a specified value and start it  counting  down.  If
either the tape data changes or the timer runs out, an IRQ will occur.  The
loader will determine which condition caused the  IRQ.  If  the  tape  data
changed before the timer ran out, we have a short pulse, or a "0"  bit.  If
the timer ran out first, we have a long pulse, or a  "1"  bit.  Doing  this
continuously and we decode the entire file.


[ Additional notes on v.2:

  I found no documents about this around, but it seems an expansion of the
format specifically thought for C16 tapes. In a .TAP version 2, each byte
only stores informations on half of the wave...
Unfortunately, I have no such a .tap file to test, so my implementation
below could be not working.  FP ]
*/

#include "cbm_tap.h"
#include "imageutl.h"

#include "multibyte.h"


#define CBM_WAV_FREQUENCY   44100

/* Systems */
#define C64     0
#define VIC20   1
#define C16     2

/* Video standards */
#define PAL     0
#define NTSC    1

/* Frequencies in [Hz] to determine the length of each pulse */
#define C64_PAL     123156      /*  985248 / 8 */
#define C64_NTSC    127841      /* 1022727 / 8 */
#define VIC20_PAL   138551      /* 1108405 / 8 */
#define VIC20_NTSC  127841      /* 1022727 / 8 */
#define C16_PAL     110840      /*  886724 / 8 */
#define C16_NTSC    111860      /*  894886 / 8 */

#define PAUSE (CBM_WAV_FREQUENCY / 50)      /* tap2wav uses this value for 0x00 in .TAP v0, instead of 0x100 */

/* These values do not really matter, as long as the produced pulses
  go above & below 0. However, for documentation purpose it would be
  nice to find out which values were used by Commodore tapes. I was
  not able to find any reference on the subject. */
#define WAVE_HIGH       (0x5a9e >> 1)
#define WAVE_LOW        -(0x5a9e >> 1)
#define WAVE_PAUSE      0x80

#define CBM_HEADER_SIZE 20

static int16_t    wave_data = 0;
static int      len;


/* This in fact gives the number of samples for half of the pulse */
static inline int tap_data_to_samplecount(int data, int frequency)
{
//  return (int) (0.5 * (0.5 + (((double)CBM_WAV_FREQUENCY / frequency) * (double)data)));      // MAME TZX formula
	return (int) (0.5 * (((double)CBM_WAV_FREQUENCY / frequency) * (double)((data) + 0.5)));    // tap2wav formula
}

/* The version with parameters could be handy if we decide to implement a
different values for byte == 0x00 below (the WAVE_PAUSE) as tap2wav does. */
#if 0
static void toggle_wave_data(int low, int high)
{
	wave_data = (wave_data == low) ? high : low;
}
#endif

static void toggle_wave_data(void )
{
	wave_data = (wave_data == WAVE_HIGH) ? WAVE_LOW : WAVE_HIGH;
}

static void cbm_output_wave( int16_t **buffer, int length )
{
	if (buffer == nullptr)
		return;

	for( ; length > 0; length-- )
	{
		**buffer = wave_data;
		*buffer = *buffer + 1;
	}
}


static int cbm_tap_do_work( int16_t **buffer, int length, const uint8_t *data )
{
	int i, j = 0;
	int size = 0;

	int version, system, video_standard;
	int tap_frequency = 0;

	int byte_samples = 0;
	uint8_t over_pulse_bytes[3] = {0 , 0, 0 };
	int over_pulse_length = 0;
	/* These waveamp_* values are currently stored but not used.
	  Further investigations are needed to find real pulse amplitude
	  in Commodore tapes. Implementation here would follow */
	/* int waveamp_high, waveamp_low; */

	/* is the .tap file corrupted? */
	if ((data == nullptr) || (length <= CBM_HEADER_SIZE))
		return -1;

	version = data[0x0c];
	system = data[0x0d];
	video_standard = data[0x0e];

	/* Log .TAP info but only once */
	if (!(buffer == nullptr))
	{
		LOG_FORMATS("TAP version    : %d\n", version);
		LOG_FORMATS("Machine type   : %d\n", system);
		LOG_FORMATS("Video standard : %d\n", video_standard);
		LOG_FORMATS("Tape frequency : %d\n", (tap_frequency) << 3);
	}


	/* is this a supported version? */
	if ((version < 0) || (version > 2))
	{
		LOG_FORMATS("Unsupported .tap version: %d \n", version);
		return -1;
	}


	/* read the frequency from the .tap header */
	switch (system)
	{
		case VIC20:
			tap_frequency = (video_standard == NTSC) ? VIC20_NTSC : VIC20_PAL;
			break;

		case C16:
			tap_frequency = (video_standard == NTSC) ? C16_NTSC : C16_PAL;
			break;

		case C64:
		default:
			tap_frequency = (video_standard == NTSC) ? C64_NTSC : C64_PAL;
			break;
	}


	for (i = CBM_HEADER_SIZE; i < length; i++)
	{
		uint8_t byte = data[i];

		/* .TAP v0 */
		/* Here is simple:
		  if byte is != 0 -> length = byte
		  otherwise -> length =  0x100 (i.e. 0xff + 1) */
		if (!version)
		{
			if (byte != 0x00)
			{
				byte_samples = tap_data_to_samplecount(byte, tap_frequency);
				/* waveamp_high = WAVE_HIGH; */
			}
			else
			{
				byte_samples = tap_data_to_samplecount(PAUSE, tap_frequency);   // tap2wav value
//              byte_samples = tap_data_to_samplecount(0x100, tap_frequency);   // vice value
				/* waveamp_high = WAVE_PAUSE; */
			}
			/* waveamp_low = WAVE_LOW; */
		}

		/* .TAP v1 & v2 */
		/* Here is a bit more complicate:
		  if byte is != 0 -> length = byte
		  otherwise -> the length of the pulse is stored as a 24bit value in the 3 bytes after the 0.
		  See below for comments on the implementation of this mechanism */
		if (version)
		{
			if ((byte != 0x00) && !j)
			{
				byte_samples = tap_data_to_samplecount(byte, tap_frequency);
				/* waveamp_high = WAVE_HIGH; */
			}
			else
			{
				/* If we have a long pulse close to the end of the .TAP, check that bytes still
				  to be read are enough to complete it. */
				if (length - i + j >= 4)
				{
					/* Here we read the 3 following bytes, using an index j
					  j = 0 -> The 0x00 byte: we simply skip everything and go on
					  j = 1,2 -> The 1st and 2nd bytes after 0x00: we store them and go on
					  j = 3 -> The final byte of the pulse length: we store it,
					  and then we pass to finally output the wave */
					if (j > 0)
					{
						over_pulse_bytes[j-1] = byte;
						j += 1;

						if (j >= 4)
						{
							over_pulse_length = get_u24le(over_pulse_bytes) >> 3;
							byte_samples = tap_data_to_samplecount(over_pulse_length, tap_frequency);
							/* waveamp_high = WAVE_PAUSE; */
							j = 0;
						}
					}
					else
					{
						j += 1;
						LOG_FORMATS("Found a 00 byte close to the end of the .tap file.\n");
						LOG_FORMATS("This is not allowed by the format specs. \n");
						LOG_FORMATS("Check if your .tap file got corrupted when you created it!\n");
					}
				}
				else j = 1;
			}
			/* waveamp_low = WAVE_LOW; */
		}

		if (j == 0)
		{
			cbm_output_wave( buffer, byte_samples );
			size += byte_samples;
//          toggle_wave_data(waveamp_low, waveamp_high);
			toggle_wave_data();
			if (version < 2)
			{
				cbm_output_wave( buffer, byte_samples );
				size += byte_samples;
//              toggle_wave_data(waveamp_low, waveamp_high);
				toggle_wave_data();
			}
		}
	}

	return size;
}


static int cbm_tap_to_wav_size( const uint8_t *tapdata, int taplen )
{
	int size = cbm_tap_do_work(nullptr, taplen, tapdata);
	len = taplen;

	return size;
}

static int cbm_tap_fill_wave( int16_t *buffer, int length, const uint8_t *bytes )
{
	int16_t *p = buffer;

	return cbm_tap_do_work(&p, len, (const uint8_t *)bytes);
}




static const cassette_image::LegacyWaveFiller cbm_legacy_fill_wave = {
	cbm_tap_fill_wave,      /* fill_wave */
	-1,                     /* chunk_size */
	0,                      /* chunk_samples */
	cbm_tap_to_wav_size,    /* chunk_sample_calc */
	CBM_WAV_FREQUENCY,      /* sample_frequency */
	0,                      /* header_samples */
	0                       /* trailer_samples */
};


static cassette_image::error cbm_cassette_identify( cassette_image *cassette, cassette_image::Options *opts )
{
	return cassette->legacy_identify( opts, &cbm_legacy_fill_wave );
}


static cassette_image::error cbm_cassette_load( cassette_image *cassette )
{
	return cassette->legacy_construct( &cbm_legacy_fill_wave );
}


static const cassette_image::Format cbm_tap_cassette_format = {
	"tap",
	cbm_cassette_identify,
	cbm_cassette_load,
	nullptr
};


CASSETTE_FORMATLIST_START(cbm_cassette_formats)
	CASSETTE_FORMAT(cbm_tap_cassette_format)
CASSETTE_FORMATLIST_END

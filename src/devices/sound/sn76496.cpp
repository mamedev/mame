// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  sn76496.c
  by Nicola Salmoria
  with contributions by others

  Routines to emulate the:
  Texas Instruments SN76489, SN76489A, SN76494/SN76496
  ( Also known as, or at least compatible with, the TMS9919 and SN94624.)
  and the Sega 'PSG' used on the Master System, Game Gear, and Megadrive/Genesis
  This chip is known as the Programmable Sound Generator, or PSG, and is a 4
  channel sound generator, with three squarewave channels and a noise/arbitrary
  duty cycle channel.

  Noise emulation for all verified chips should be accurate:

  ** SN76489 uses a 15-bit shift register with taps on bits D and E, output on E,
  XOR function.
  It uses a 15-bit ring buffer for periodic noise/arbitrary duty cycle.
  Its output is inverted.
  ** SN94624 is the same as SN76489 but lacks the /8 divider on its clock input.
  ** SN76489A uses a 15-bit shift register with taps on bits D and E, output on F,
  XOR function.
  It uses a 15-bit ring buffer for periodic noise/arbitrary duty cycle.
  Its output is not inverted.
  ** SN76494 is the same as SN76489A but lacks the /8 divider on its clock input.
  ** SN76496 is identical in operation to the SN76489A, but the audio input on pin 9 is
  documented.
  All the TI-made PSG chips have an audio input line which is mixed with the 4 channels
  of output. (It is undocumented and may not function properly on the sn76489, 76489a
  and 76494; the sn76489a input is mentioned in datasheets for the tms5200)
  All the TI-made PSG chips act as if the frequency was set to 0x400 if 0 is
  written to the frequency register.
  ** Sega Master System III/MD/Genesis PSG uses a 16-bit shift register with taps
  on bits C and F, output on F
  It uses a 16-bit ring buffer for periodic noise/arbitrary duty cycle.
  (whether it uses an XOR or XNOR needs to be verified, assumed XOR)
  (whether output is inverted or not needs to be verified, assumed to be inverted)
  ** Sega Game Gear PSG is identical to the SMS3/MD/Genesis one except it has an
  extra register for mapping which channels go to which speaker.
  The register, connected to a z80 port, means:
  for bits 7  6  5  4  3  2  1  0
           L3 L2 L1 L0 R3 R2 R1 R0
  Noise is an XOR function, and audio output is negated before being output.
  All the Sega-made PSG chips act as if the frequency was set to 0 if 0 is written
  to the frequency register.
  ** NCR7496 (as used on the Tandy 1000) is similar to the SN76489 but with a
  different noise LFSR patttern: taps on bits A and E, output on E
  It uses a 15-bit ring buffer for periodic noise/arbitrary duty cycle.
  (all this chip's info needs to be verified)

  28/03/2005 : Sebastien Chevalier
  Update th SN76496Write func, according to SN76489 doc found on SMSPower.
   - On write with 0x80 set to 0, when LastRegister is other then TONE,
   the function is similar than update with 0x80 set to 1

  23/04/2007 : Lord Nightmare
  Major update, implement all three different noise generation algorithms and a
  set_variant call to discern among them.

  28/04/2009 : Lord Nightmare
  Add READY line readback; cleaned up struct a bit. Cleaned up comments.
  Add more TODOs. Fixed some unsaved savestate related stuff.

  04/11/2009 : Lord Nightmare
  Changed the way that the invert works (it now selects between XOR and XNOR
  for the taps), and added R->OldNoise to simulate the extra 0 that is always
  output before the noise LFSR contents are after an LFSR reset.
  This fixes SN76489/A to match chips. Added SN94624.

  14/11/2009 : Lord Nightmare
  Removed STEP mess, vastly simplifying the code. Made output bipolar rather
  than always above the 0 line, but disabled that code due to pending issues.

  16/11/2009 : Lord Nightmare
  Fix screeching in regulus: When summing together four equal channels, the
  size of the max amplitude per channel should be 1/4 of the max range, not
  1/3. Added NCR7496.

  18/11/2009 : Lord Nightmare
  Modify Init functions to support negating the audio output. The gamegear
  psg does this. Change gamegear and sega psgs to use XOR rather than XNOR
  based on testing. Got rid of R->OldNoise and fixed taps accordingly.
  Added stereo support for game gear.

  15/01/2010 : Lord Nightmare
  Fix an issue with SN76489 and SN76489A having the wrong periodic noise periods.
  Note that properly emulating the noise cycle bit timing accurately may require
  extensive rewriting.

  24/01/2010: Lord Nightmare
  Implement periodic noise as forcing one of the XNOR or XOR taps to 1 or 0 respectively.
  Thanks to PlgDavid for providing samples which helped immensely here.
  Added true clock divider emulation, so sn94624 and sn76494 run 8x faster than
  the others, as in real life.

  15/02/2010: Lord Nightmare & Michael Zapf (additional testing by PlgDavid)
  Fix noise period when set to mirror channel 3 and channel 3 period is set to 0 (tested on hardware for noise, wave needs tests) - MZ
  Fix phase of noise on sn94624 and sn76489; all chips use a standard XOR, the only inversion is the output itself - LN, Plgdavid
  Thanks to PlgDavid and Michael Zapf for providing samples which helped immensely here.

  23/02/2011: Lord Nightmare & Enik
  Made it so the Sega PSG chips have a frequency of 0 if 0 is written to the
  frequency register, while the others have 0x400 as before. Should fix a bug
  or two on sega games, particularly Vigilante on Sega Master System. Verified
  on SMS hardware.

  27/06/2012: Michael Zapf
  Converted to modern device, legacy devices were gradually removed afterwards.

  16/09/2015: Lord Nightmare
  Fix PSG chips to have volume reg inited on reset to 0x0 based on tests by
  ValleyBell. Made Sega PSG chips start up with register 0x3 selected (volume
  for channel 2) based on hardware tests by Nemesis.

  TODO: * Implement the TMS9919 - any difference to sn94624?
        * Implement the T6W28; has registers in a weird order, needs writes
          to be 'sanitized' first. Also is stereo, similar to game gear.
        * Test the NCR7496; Smspower says the whitenoise taps are A and E,
          but this needs verification on real hardware.
        * Factor out common code so that the SAA1099 can share some code.

***************************************************************************/

#include "emu.h"
#include "sn76496.h"

#define MAX_OUTPUT 0x7fff


sn76496_base_device::sn76496_base_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		int feedbackmask,
		int noisetap1,
		int noisetap2,
		bool negate,
		bool stereo,
		int clockdivider,
		bool sega,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_ready_handler(*this)
	, m_feedback_mask(feedbackmask)
	, m_whitenoise_tap1(noisetap1)
	, m_whitenoise_tap2(noisetap2)
	, m_negate(negate)
	, m_stereo(stereo)
	, m_clock_divider(clockdivider)
	, m_sega_style_psg(sega)
{
}

sn76496_device::sn76496_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sn76496_base_device(mconfig, SN76496, tag, 0x10000, 0x04, 0x08, false, false, 8, true, owner, clock)
{
}

u8106_device::u8106_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sn76496_base_device(mconfig, U8106, tag, 0x4000, 0x01, 0x02, true, false, 8, true, owner, clock)
{
}

y2404_device::y2404_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sn76496_base_device(mconfig, Y2404, tag, 0x10000, 0x04, 0x08, false, false, 8, true, owner, clock)
{
}

sn76489_device::sn76489_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sn76496_base_device(mconfig, SN76489, tag, 0x4000, 0x01, 0x02, true, false, 8, true, owner, clock)
{
}

sn76489a_device::sn76489a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sn76496_base_device(mconfig, SN76489A, tag, 0x10000, 0x04, 0x08, false, false, 8, true, owner, clock)
{
}

sn76494_device::sn76494_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sn76496_base_device(mconfig, SN76494, tag, 0x10000, 0x04, 0x08, false, false, 1, true, owner, clock)
{
}

sn94624_device::sn94624_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sn76496_base_device(mconfig, SN94624, tag, 0x4000, 0x01, 0x02, true, false, 1, true, owner, clock)
{
}

ncr7496_device::ncr7496_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sn76496_base_device(mconfig, NCR7496, tag, 0x8000, 0x02, 0x20, false, false, 8, true, owner, clock)
{
}

gamegear_device::gamegear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sn76496_base_device(mconfig, GAMEGEAR, tag, 0x8000, 0x01, 0x08, true, true, 8, false, owner, clock)
{
}

segapsg_device::segapsg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sn76496_base_device(mconfig, SEGAPSG, tag, 0x8000, 0x01, 0x08, true, false, 8, false, owner, clock)
{
}


void sn76496_base_device::device_start()
{
	int sample_rate = clock()/2;
	int i;
	double out;
	int gain;

	m_ready_handler.resolve_safe();

	m_sound = machine().sound().stream_alloc(*this, 0, (m_stereo? 2:1), sample_rate);

	for (i = 0; i < 4; i++) m_volume[i] = 0;

	m_last_register = m_sega_style_psg?3:0; // Sega VDP PSG defaults to selected period reg for 2nd channel
	for (i = 0; i < 8; i+=2)
	{
		m_register[i] = 0;
		m_register[i + 1] = 0x0;   // volume = 0x0 (max volume) on reset; this needs testing on chips other than SN76489A and Sega VDP PSG
	}

	for (i = 0; i < 4; i++)
	{
		m_output[i] = 0;
		m_period[i] = 0;
		m_count[i] = 0;
	}

	m_RNG = m_feedback_mask;
	m_output[3] = m_RNG & 1;

	m_cycles_to_ready = 1;          // assume ready is not active immediately on init. is this correct?
	m_stereo_mask = 0xFF;           // all channels enabled
	m_current_clock = m_clock_divider-1;

	// set gain
	gain = 0;

	gain &= 0xff;

	// increase max output basing on gain (0.2 dB per step)
	out = MAX_OUTPUT / 4; // four channels, each gets 1/4 of the total range
	while (gain-- > 0)
		out *= 1.023292992; // = (10 ^ (0.2/20))

	// build volume table (2dB per step)
	for (i = 0; i < 15; i++)
	{
		// limit volume to avoid clipping
		if (out > MAX_OUTPUT / 4) m_vol_table[i] = MAX_OUTPUT / 4;
		else m_vol_table[i] = out;

		out /= 1.258925412; /* = 10 ^ (2/20) = 2dB */
	}
	m_vol_table[15] = 0;

	m_ready_state = true;

	register_for_save_states();
}

WRITE8_MEMBER( sn76496_base_device::stereo_w )
{
	m_sound->update();
	if (m_stereo) m_stereo_mask = data;
	else fatalerror("sn76496_base_device: Call to stereo write with mono chip!\n");
}

void sn76496_base_device::write(uint8_t data)
{
	int n, r, c;

	// update the output buffer before changing the registers
	m_sound->update();

	// set number of cycles until READY is active; this is always one
	// 'sample', i.e. it equals the clock divider exactly
	m_cycles_to_ready = 1;

	if (data & 0x80)
	{
		r = (data & 0x70) >> 4;
		m_last_register = r;
		m_register[r] = (m_register[r] & 0x3f0) | (data & 0x0f);
	}
	else
	{
		r = m_last_register;
	}

	c = r >> 1;
	switch (r)
	{
		case 0: // tone 0: frequency
		case 2: // tone 1: frequency
		case 4: // tone 2: frequency
			if ((data & 0x80) == 0) m_register[r] = (m_register[r] & 0x0f) | ((data & 0x3f) << 4);
			if ((m_register[r] != 0) || (!m_sega_style_psg)) m_period[c] = m_register[r];
			else m_period[c] = 0x400;

			if (r == 4)
			{
				// update noise shift frequency
				if ((m_register[6] & 0x03) == 0x03) m_period[3] = m_period[2]<<1;
			}
			break;
		case 1: // tone 0: volume
		case 3: // tone 1: volume
		case 5: // tone 2: volume
		case 7: // noise: volume
			m_volume[c] = m_vol_table[data & 0x0f];
			if ((data & 0x80) == 0) m_register[r] = (m_register[r] & 0x3f0) | (data & 0x0f);
			break;
		case 6: // noise: frequency, mode
			{
				if ((data & 0x80) == 0) logerror("sn76496_base_device: write to reg 6 with bit 7 clear; data was %03x, new write is %02x! report this to LN!\n", m_register[6], data);
				if ((data & 0x80) == 0) m_register[r] = (m_register[r] & 0x3f0) | (data & 0x0f);
				n = m_register[6];
				// N/512,N/1024,N/2048,Tone #3 output
				m_period[3] = ((n&3) == 3)? (m_period[2]<<1) : (1 << (5+(n&3)));
				m_RNG = m_feedback_mask;
			}
			break;
	}
}

WRITE8_MEMBER( sn76496_base_device::write )
{
	write(data);
}

inline bool sn76496_base_device::in_noise_mode()
{
	return ((m_register[6] & 4)!=0);
}

void sn76496_base_device::countdown_cycles()
{
	if (m_cycles_to_ready > 0)
	{
		m_cycles_to_ready--;
		if (m_ready_state==true) m_ready_handler(CLEAR_LINE);
		m_ready_state = false;
	}
	else
	{
		if (m_ready_state==false) m_ready_handler(ASSERT_LINE);
		m_ready_state = true;
	}
}

void sn76496_base_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int i;
	stream_sample_t *lbuffer = outputs[0];
	stream_sample_t *rbuffer = (m_stereo)? outputs[1] : nullptr;

	int16_t out;
	int16_t out2 = 0;

	while (samples > 0)
	{
		// clock chip once
		if (m_current_clock > 0) // not ready for new divided clock
		{
			m_current_clock--;
		}
		else // ready for new divided clock, make a new sample
		{
			m_current_clock = m_clock_divider-1;
			// decrement Cycles to READY by one
			countdown_cycles();

			// handle channels 0,1,2
			for (i = 0; i < 3; i++)
			{
				m_count[i]--;
				if (m_count[i] <= 0)
				{
					m_output[i] ^= 1;
					m_count[i] = m_period[i];
				}
			}

			// handle channel 3
			m_count[3]--;
			if (m_count[3] <= 0)
			{
				// if noisemode is 1, both taps are enabled
				// if noisemode is 0, the lower tap, whitenoisetap2, is held at 0
				// The != was a bit-XOR (^) before
				if (((m_RNG & m_whitenoise_tap1)!=0) != (((m_RNG & m_whitenoise_tap2)!=0) && in_noise_mode()))
				{
					m_RNG >>= 1;
					m_RNG |= m_feedback_mask;
				}
				else
				{
					m_RNG >>= 1;
				}
				m_output[3] = m_RNG & 1;

				m_count[3] = m_period[3];
			}
		}

		if (m_stereo)
		{
			out = ((((m_stereo_mask & 0x10)!=0) && (m_output[0]!=0))? m_volume[0] : 0)
				+ ((((m_stereo_mask & 0x20)!=0) && (m_output[1]!=0))? m_volume[1] : 0)
				+ ((((m_stereo_mask & 0x40)!=0) && (m_output[2]!=0))? m_volume[2] : 0)
				+ ((((m_stereo_mask & 0x80)!=0) && (m_output[3]!=0))? m_volume[3] : 0);

			out2= ((((m_stereo_mask & 0x1)!=0) && (m_output[0]!=0))? m_volume[0] : 0)
				+ ((((m_stereo_mask & 0x2)!=0) && (m_output[1]!=0))? m_volume[1] : 0)
				+ ((((m_stereo_mask & 0x4)!=0) && (m_output[2]!=0))? m_volume[2] : 0)
				+ ((((m_stereo_mask & 0x8)!=0) && (m_output[3]!=0))? m_volume[3] : 0);
		}
		else
		{
			out= ((m_output[0]!=0)? m_volume[0]:0)
				+((m_output[1]!=0)? m_volume[1]:0)
				+((m_output[2]!=0)? m_volume[2]:0)
				+((m_output[3]!=0)? m_volume[3]:0);
		}

		if (m_negate) { out = -out; out2 = -out2; }

		*(lbuffer++) = out;
		if (m_stereo) *(rbuffer++) = out2;
		samples--;
	}
}

void sn76496_base_device::register_for_save_states()
{
	save_item(NAME(m_vol_table));
	save_item(NAME(m_register));
	save_item(NAME(m_last_register));
	save_item(NAME(m_volume));
	save_item(NAME(m_RNG));
//  save_item(NAME(m_clock_divider));
	save_item(NAME(m_current_clock));
//  save_item(NAME(m_feedback_mask));
//  save_item(NAME(m_whitenoise_tap1));
//  save_item(NAME(m_whitenoise_tap2));
//  save_item(NAME(m_negate));
//  save_item(NAME(m_stereo));
	save_item(NAME(m_stereo_mask));
	save_item(NAME(m_period));
	save_item(NAME(m_count));
	save_item(NAME(m_output));
	save_item(NAME(m_cycles_to_ready));
//  save_item(NAME(m_sega_style_psg));
}

DEFINE_DEVICE_TYPE(SN76496,  sn76496_device,   "sn76496",      "SN76496")
DEFINE_DEVICE_TYPE(U8106,    u8106_device,     "u8106",        "U8106")
DEFINE_DEVICE_TYPE(Y2404,    y2404_device,     "y2404",        "Y2404")
DEFINE_DEVICE_TYPE(SN76489,  sn76489_device,   "sn76489",      "SN76489")
DEFINE_DEVICE_TYPE(SN76489A, sn76489a_device,  "sn76489a",     "SN76489A")
DEFINE_DEVICE_TYPE(SN76494,  sn76494_device,   "sn76494",      "SN76494")
DEFINE_DEVICE_TYPE(SN94624,  sn94624_device,   "sn94624",      "SN94624")
DEFINE_DEVICE_TYPE(NCR7496,  ncr7496_device,   "ncr7496",      "NCR7496")
DEFINE_DEVICE_TYPE(GAMEGEAR, gamegear_device,  "gamegear_psg", "Game Gear PSG")
DEFINE_DEVICE_TYPE(SEGAPSG,  segapsg_device,   "segapsg",      "Sega VDP PSG")

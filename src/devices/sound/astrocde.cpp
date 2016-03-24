// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Frank Palazzolo
/***********************************************************

    Astrocade custom 'IO' chip sound chip driver
    Aaron Giles
    based on original work by Frank Palazzolo

************************************************************

    Register Map
    ============

    Register 0:
        D7..D0: Master oscillator frequency

    Register 1:
        D7..D0: Tone generator A frequency

    Register 2:
        D7..D0: Tone generator B frequency

    Register 3:
        D7..D0: Tone generator C frequency

    Register 4:
        D7..D6: Vibrato speed
        D5..D0: Vibrato depth

    Register 5:
            D5: Noise AM enable
            D4: Mux source (0=vibrato, 1=noise)
        D3..D0: Tone generator C volume

    Register 6:
        D7..D4: Tone generator B volume
        D3..D0: Tone generator A volume

    Register 7:
        D7..D0: Noise volume

***********************************************************/

#include "emu.h"
#include "astrocde.h"


// device type definition
const device_type ASTROCADE = &device_creator<astrocade_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  astrocade_device - constructor
//-------------------------------------------------

astrocade_device::astrocade_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ASTROCADE, "Astrocade", tag, owner, clock, "astrocade", __FILE__),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_master_count(0),
		m_vibrato_clock(0),
		m_noise_clock(0),
		m_noise_state(0),
		m_a_count(0),
		m_a_state(0),
		m_b_count(0),
		m_b_state(0),
		m_c_count(0),
		m_c_state(0)
{
	memset(m_reg, 0, sizeof(UINT8)*8);
	memset(m_bitswap, 0, sizeof(UINT8)*256);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void astrocade_device::device_start()
{
	int i;

	/* generate a bitswap table for the noise */
	for (i = 0; i < 256; i++)
		m_bitswap[i] = BITSWAP8(i, 0,1,2,3,4,5,6,7);

	/* allocate a stream for output */
	m_stream = stream_alloc(0, 1, clock());

	/* reset state */
	device_reset();
	state_save_register();
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void astrocade_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *dest = outputs[0];
	UINT16 noise_state;
	UINT8 master_count;
	UINT8 noise_clock;

	/* load some locals */
	master_count = m_master_count;
	noise_clock = m_noise_clock;
	noise_state = m_noise_state;

	/* loop over samples */
	while (samples > 0)
	{
		stream_sample_t cursample = 0;
		int samples_this_time;
		int samp;

		/* compute the number of cycles until the next master oscillator reset */
		/* or until the next noise boundary */
		samples_this_time = MIN(samples, 256 - master_count);
		samples_this_time = MIN(samples_this_time, 64 - noise_clock);
		samples -= samples_this_time;

		/* sum the output of the tone generators */
		if (m_a_state)
			cursample += m_reg[6] & 0x0f;
		if (m_b_state)
			cursample += m_reg[6] >> 4;
		if (m_c_state)
			cursample += m_reg[5] & 0x0f;

		/* add in the noise if it is enabled, based on the top bit of the LFSR */
		if ((m_reg[5] & 0x20) && (noise_state & 0x4000))
			cursample += m_reg[7] >> 4;

		/* scale to max and output */
		cursample = cursample * 32767 / 60;
		for (samp = 0; samp < samples_this_time; samp++)
			*dest++ = cursample;

		/* clock the noise; a 2-bit counter clocks a 4-bit counter which clocks the LFSR */
		noise_clock += samples_this_time;
		if (noise_clock >= 64)
		{
			/* update the noise state; this is a 15-bit LFSR with feedback from */
			/* the XOR of the top two bits */
			noise_state = (noise_state << 1) | (~((noise_state >> 14) ^ (noise_state >> 13)) & 1);
			noise_clock -= 64;

			/* the same clock also controls the vibrato clock, which is a 13-bit counter */
			m_vibrato_clock++;
		}

		/* clock the master oscillator; this is an 8-bit up counter */
		master_count += samples_this_time;
		if (master_count == 0)
		{
			/* reload based on mux value -- the value from the register is negative logic */
			master_count = ~m_reg[0];

			/* mux value 0 means reload based on the vibrato control */
			if ((m_reg[5] & 0x10) == 0)
			{
				/* vibrato speed (register 4 bits 6-7) selects one of the top 4 bits */
				/* of the 13-bit vibrato clock to use (0=highest freq, 3=lowest) */
				if (!((m_vibrato_clock >> (m_reg[4] >> 6)) & 0x0200))
				{
					/* if the bit is clear, we add the vibrato volume to the counter */
					master_count += m_reg[4] & 0x3f;
				}
			}

			/* mux value 1 means reload based on the noise control */
			else
			{
				/* the top 8 bits of the noise LFSR are ANDed with the noise volume */
				/* register and added to the count */
				master_count += m_bitswap[(noise_state >> 7) & 0xff] & m_reg[7];
			}

			/* clock tone A */
			if (++m_a_count == 0)
			{
				m_a_state ^= 1;
				m_a_count = ~m_reg[1];
			}

			/* clock tone B */
			if (++m_b_count == 0)
			{
				m_b_state ^= 1;
				m_b_count = ~m_reg[2];
			}

			/* clock tone C */
			if (++m_c_count == 0)
			{
				m_c_state ^= 1;
				m_c_count = ~m_reg[3];
			}
		}
	}

	/* put back the locals */
	m_master_count = master_count;
	m_noise_clock = noise_clock;
	m_noise_state = noise_state;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void astrocade_device::device_reset()
{
	memset(m_reg, 0, sizeof(m_reg));

	m_master_count = 0;
	m_vibrato_clock = 0;

	m_noise_clock = 0;
	m_noise_state = 0;

	m_a_count = 0;
	m_a_state = 0;

	m_b_count = 0;
	m_b_state = 0;

	m_c_count = 0;
	m_c_state = 0;
}


//-------------------------------------------------
//  Save state registration
//-------------------------------------------------

void astrocade_device::state_save_register()
{
	save_item(NAME(m_reg));

	save_item(NAME(m_master_count));
	save_item(NAME(m_vibrato_clock));

	save_item(NAME(m_noise_clock));
	save_item(NAME(m_noise_state));

	save_item(NAME(m_a_count));
	save_item(NAME(m_a_state));

	save_item(NAME(m_b_count));
	save_item(NAME(m_b_state));

	save_item(NAME(m_c_count));
	save_item(NAME(m_c_state));
}


/*************************************
 *
 *  Sound write accessors
 *
 *************************************/

WRITE8_MEMBER( astrocade_device::astrocade_sound_w )
{
	if ((offset & 8) != 0)
		offset = (offset >> 8) & 7;
	else
		offset &= 7;

	/* update */
	m_stream->update();

	/* stash the new register value */
	m_reg[offset & 7] = data;
}

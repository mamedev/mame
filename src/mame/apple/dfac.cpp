// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

   dfac.cpp - Apple Digitally Filtered Audio Chip, which controls input and
              output gain and filtering.

        DFAC uses a 3-wire serial interface, where it caches the last
        8 bits seen over a data/clock pair and writes that byte to the
        working space when a low/high transition occurs on a third
        "latch" line.

           ________________________
         /  4  3  2  1  28  27  26 |
        |            *             |
        | 5                     25 |
        | 6     344S1033        24 |
        | 7                     23 |
        | 8     (C)'90 APPLE    22 |
        | 9                     21 |
        | 10                    20 |
        | 11                    19 |
        |____12_13_14_15_16_17_18__|

        1 - Vcom - AC ground reference output, 1/2 of Vdd
        2 - Bypass - Connect 10 uF electrolytic here for Vcom stablity
        3 - Vdd2 - Chip's Vdd supply, 8 volts DC
        4 - Vdd1 - "       "    "     "   "   "
        5 - IV out - Current-to-voltage op-amp output
        6 - IV- - inverted output of current-to-voltage op-amp
        7 - NFin - Noise Filter op-amp non-inverting input
        8 - AmpOut - Output for either the record gain or playback PWM.
            Typically connected to SCFin.
        9 - /REDUCEGAIN - Open-drain pulldown digital output used to decrease the gain
            of an external op-amp.
        10 - SoundOut - Analog audio output
        11 - SCFin - Switched Capacitor Filter input
        12 - SCFout - output of Switched Capacitor Filter
        13 - ADin - A/D converter input.  Goes through volume control attenuator and
             ends up at the pin 10 output.
        14 - /RESET
        15 - SerDOut - A/D converter serial digital output
        16 - SampleClk - Sample clock
        17 - SerClk - Serial clock, used to clock individual bits of the sample
        18 - VssD - Digital ground
        19 - ConfigClk - Configuration bit clock
        20 - ConfigData - Configuration bit data.  Latched on the rising edge of ConfigClk.
        21 - ConfigLE - Configuration Latch Enable.  Causes the last 8 bits seen on the
             rising edge of ConfigClk to become the new configuraiton byte.
        22 - PWMin - PWM-encoded input pulse.  TTL compatible.
        23 - PWMout - Level shifted version of PWMin.  Output swings between Vss and Vdd.
        24 - LPFin - Low Pass Filter input.  Used to remove harmonics from PWMout.
        25 - LPFout - Output from LPFin.  Normally connected to SCFin.
        26 - VssA - Analog ground
        27 - Vlow - A/D converter's low reference voltage (1 volt less than Vcom)
        28 - Vhigh - A/D converter's high reference voltage (1 volt more than Vcom)

        The control byte is as follows:
        7654 3210
        VVV        - volume
            T      - playthrough enable (input is mixed with output)
             GG    - 0 = /REDUCE_GAIN low (enabled), 2 = /REDUCE_GAIN reacts to a comparitor
                     on the input.  If 1 or 3, /REDUCE_GAIN is tristated.
               L  - LPF In to mix with Noise input and go to Amp Out (0=Off, 1=On)
                N - Noise input enable: 0 = enabled, 1 = disabled

        Signal flow through the chip:
        PWM In -> PWM Out (level shifts the PWM signal from TTL to the chip's voltage range)
        LPF In -> LPF Out (no change) and to (switch on config bit 1) -> Amp Out
        NF In  -> (switch on config bit 0)                            -> Amp Out
        SCF In -> switched capacitor filter                           -> SCF Out
        AD In  -> volume attenuator (top 3 bits of config byte)       -> Sound Out

        Standard hookup verified on schematics for LC/LC II/LC III/Classic II:
        PWM from system ASIC -> PWM In, PWM Out -> LPF In, Amp Out -> SCF In, SCF Out -> AD In.

        TODO: SCF and external 2-pole RC lowpass filter.


***************************************************************************/

#include "emu.h"
#include "dfac.h"

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(APPLE_DFAC, dfac_device, "apldfac", "Apple Digitally Filtered Audio Chip")

static constexpr double atten_table[8] =
{
	0.0,                    // -infinite
	0.125892541179417,      // -18dB
	0.177827941003892,      // -15dB
	0.251188643150958,      // -12dB
	0.354813389233575,      // -9dB
	0.501187233627272,      // -6dB
	0.707945784384138,      // -3dB
	1.0                     // No attenuation
};

dfac_device::dfac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, APPLE_DFAC, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  m_stream(nullptr),
	  m_data(false),
	  m_clock(false),
	  m_dfaclatch(false),
	  m_settings_byte(0),
	  m_latch_byte(0)
{
}

void dfac_device::device_start()
{
	m_stream = stream_alloc(8, 2, clock(), STREAM_SYNCHRONOUS);

	save_item(NAME(m_clock));
	save_item(NAME(m_data));
	save_item(NAME(m_dfaclatch));
	save_item(NAME(m_settings_byte));
}

void dfac_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	if (BIT(m_settings_byte, 1))    // LPF In can go through to Amp Out
	{
		for (int i = 0; i < inputs[0].samples(); i++)
		{
			stream_buffer::sample_t l = inputs[0].get(i);
			stream_buffer::sample_t r = inputs[1].get(i);
			outputs[0].put(i, l * atten_table[m_settings_byte >> 5]);
			outputs[1].put(i, r * atten_table[m_settings_byte >> 5]);
		}
	}
	else
	{
		for (int i = 0; i < inputs[0].samples(); i++)
		{
			outputs[0].put(i, 0.0);
			outputs[1].put(i, 0.0);
		}
	}
}

void dfac_device::clock_write(int state)
{
	// take a bit on the rising edge of SCL
	if (state && !m_clock)
	{
		m_latch_byte >>= 1;
		m_latch_byte |= m_data ? 0x80 : 0;
	}

	m_clock = state;
}

void dfac_device::latch_write(int state)
{
	// commit settings byte on the rising edge of the latch
	if (state && !m_dfaclatch)
	{
		m_settings_byte = m_latch_byte;
		LOG("DFAC: applying new settings byte %02x (volume %d, atten %f)\n", m_settings_byte, m_settings_byte >> 5, atten_table[m_settings_byte >> 5]);
	}

	m_dfaclatch = state;
}


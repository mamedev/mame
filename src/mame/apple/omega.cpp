// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

   omega.cpp - Apple PLL clock synthesizer, used with Sonora and Ardbeg
        based machines to generate the system clock and video dot clock.

        Omega is intended to share the 3-wire clock/data/latch serial bus
        with DFAC.  On the rising edge of the latch, it takes note of the
        next 16 bits.  The first 7 are the N parameter, the next 7 bits are
        the D parameter, and the final 2 are the P parameter.  The actual
        formula is similar to the Gazelle chip and the Sierra Semiconductor
        SC11412 but deviates quickly for higher pixel clocks.

        Pixel clock = (31.3344 MHz * (N / D)) / P;

           ________________________
         /  4  3  2  1  28  27  26 |
        |            *             |
        | 5                     25 |
        | 6     344S1060-A N    24 |
        | 7                     23 |
        | 8   (C)1991 APPLE     22 |
        | 9                     21 |
        | 10                    20 |
        | 11                    19 |
        |____12_13_14_15_16_17_18__|

        1, 7, 13, 16, 19, 20 = DGND, digital ground
        2, 28 = AGND, analog ground
        3 = DOTFLTR, filter for the DOTCLK output
        4, 26 = AVDD, analog +5 volt supply (connected to VDD in LC 520)
        5 - OESYSCLK - 1 to enable the SYSCLK output on pin 15
        6 - OEALL - 1 to enable both the pixel and system clocks?
        8 - SLATCH - serial latch
        9 - SDATA - serial data
        10 - SCLK - serial bit clock
        11, 14, 17, 23 = VDD, main +5 volt supply
        12 - DOTCLK - video dot clock output
        15 - SYSCLK - system clock output
        18 - C32M - outputs a mirror of the reference crystal clock
        21 - XTALOUT
        22 - XTALIN - connect the 31.3344 MHz reference crystal between XTALIN and XTALOUT
        24 - S1 - selects one of the canned system clock freqencies (24 is VDD, 25 is GND in LC III = 25 MHz)
        25 - S0 - "       "   "  "   "      "      "     "
        27 - SYSFLTR - filter for the SYSCLK output

        SYSFLTR and DOTFLTR are connected like this:

                        604 ohms?  0.1uF
        (pin 3 or 27) ----/\/\/\----||-----|
                        |                  |-----GND
                        |---||-------------|
                          0.002uF

***************************************************************************/

#include "emu.h"
#include "omega.h"

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(APPLE_OMEGA, omega_device, "aplomega", "Apple Omega PLL Clock Synthesizer")

omega_device::omega_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, APPLE_OMEGA, tag, owner, clock),
	  m_write_pclock(*this),
	  m_data(false),
	  m_clock(false),
	  m_latch(false),
	  m_latch_byte(0),
	  m_N(0),
	  m_D(0),
	  m_P(0),
	  m_bit(0)
{
}

void omega_device::device_start()
{
	save_item(NAME(m_clock));
	save_item(NAME(m_data));
	save_item(NAME(m_latch));
	save_item(NAME(m_latch_byte));
	save_item(NAME(m_N));
	save_item(NAME(m_D));
	save_item(NAME(m_P));
	save_item(NAME(m_bit));
}

void omega_device::clock_write(int state)
{
	// take a bit on the rising edge of SCL
	if (state && !m_clock)
	{
		m_latch_byte <<= 1;
		m_latch_byte |= m_data & 1;

		switch (m_bit)
		{
			case 6:
				m_N = m_latch_byte;
				m_latch_byte = 0;
				break;

			case 13:
				m_D = m_latch_byte;
				m_latch_byte = 0;
				break;

			case 15:
				m_P = m_latch_byte & 3;

				const u32 pclock = (u32)(31334400.0f * ((double)m_N / (double)m_D)) / (double)m_P;
				m_write_pclock(pclock);

				LOG("%s: N = %d, D = %d, P = %d, pixel clock = %d\n", tag(), m_N, m_D, m_P, pclock);
				break;
		}

		m_bit++;
	}

	m_clock = state;
}

void omega_device::latch_write(int state)
{
	// start counting bits on the falling edge of the latch
	if (state && !m_latch)
	{
		LOG("%s: latch rising edge, resetting bit count\n", tag());
		m_bit = 0;
		m_latch_byte = 0;
	}

	m_latch = state;
}


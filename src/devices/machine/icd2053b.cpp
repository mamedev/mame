// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

   icd2053b.cpp - Cypress ICD2053A Programmable Clock Generator

   This is a serially programmed PLL clock generator rated for a range
   from 391 kHz to 100 MHz at TTL levels or 90 MHz at CMOS levels.

   Bits are shifted in LSB first.  If 4 1 bits in a row are shifted in,
   that's considered to be a command word of the form 0x1exx, where the
   low 8 bits set the control register.  Setting bit 0 of the command
   register to 1 enables a write of the next 22 bits to the program register.

   In the event that the program register contains 4 or more 1 bits in a row,
   you can escape them by inserting a 0 bit after every group of 3 1 bits.
   IC Designs/Cypress calls this "bit-stuffing".  The stuffed bit is NOT
   inserted into the shift register and the detection of 3 1 bits is reset
   so that you can have another 3 1 bits after an escape bit before you
   need another escape bit.

   To program:     1111   0111   1110    111111
   Send:          10111  00111  01110  01110111

   The control register is as follows:
   Bits 7, 6, and 4 are reserved and unused.
   Bit 5 reduces the duty cycle by 0.7 nanoseconds if 1.
   Bit 3 controls if pin 7 is OE (output enable) (0) or MUXREF (1).
   Bit 2 determines if CLKOUT is the VCO frequency (0) or a passthrough of
         the input clock (1).
   Bit 1 controls OE (output enable), which tri-states the output clock
         if 1 or operates normally if 0.
   Bit 0 is the write enable for the program word.  Set to "1" to enable
         writing a 22-bit program word.

   The program word is as follows:

   Bits 21-15 are the 7-bit P parameter.
   Bit  14 increases the duty cycle by 0.7 nanoseconds if 1.  Cypress
           recommends always setting it to 1.
   Bits 13-11 are the 3-bit MUX parameter.  This sets a post-VCO divider
        of the form (1 << MUX), so 0 divides by 1 (no divide), 1 divides by
        2, 2 divides by 4, and so on.
   Bits 10-4 are the 7-bit Q parameter.
   Bits 0-3 pre-set the VCO to an appropriate range. 0 is 50 to 80 MHz,
            and a value of 8 is 80-150 MHz.  Note that these frequencies
            are prior to the MUX divider, so a divide by 2 to output
            30 MHz would have a VCO frequency of 60 MHz.

    The VCO rate fVCO is (Input clock * 2) * ((P+3) / (Q+2)), and the
    output clock is (fVCO / (1 << MUX)).

    For an input clock of 31.3344 MHz with P = 80, Q = 41, and MUX = 1,
    the output is therefore ((31.3344 * 2) * (83 / 43)) / (2) = 30.241339 MHz.

   Apple used a Motorola clone/second source labeled XCR2115805 (possibly
   also XCR2115808, some photos are inconclusive), which they called
   "Clifton", in the Quadra 660AV and Duo Dock.

***************************************************************************/

#include "emu.h"
#include "icd2053b.h"

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(ICD2053B, icd2053b_device, "icd2053b", "Cypress Semiconductor ICD2053B Programmable Clock Generator")

icd2053b_device::icd2053b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ICD2053B, tag, owner, clock),
	m_clkout_changed_cb(*this),
	m_shifter(0),
	m_shifter_last3(0),
	m_shift_pos(0),
	m_pll_control(0),
	m_prev_shift_clock(CLEAR_LINE),
	m_shifter_expected(22),
	m_datalatch(0)
{
}

void icd2053b_device::device_start()
{
	save_item(NAME(m_shifter));
	save_item(NAME(m_shifter_last3));
	save_item(NAME(m_shift_pos));
	save_item(NAME(m_pll_control));
	save_item(NAME(m_prev_shift_clock));
	save_item(NAME(m_shifter_expected));
	save_item(NAME(m_datalatch));
}

void icd2053b_device::data_w(int state)
{
	m_datalatch = state;
}

void icd2053b_device::clk_w(int state)
{
	// rising clock edge?
	if (state && !m_prev_shift_clock)
	{
		LOG("%s: Got bit %d, shifter %08x, bits left %d\n", tag(), m_datalatch, m_shifter, m_shifter_expected);

		// were the previous 3 bits 1s?  If so this is a command word
		if (m_shift_pos >= 3)
		{
			if (m_shifter_last3 == 7)
			{
				// each time we see an escape, forget the last 3 bits (example on page 2 of the ICD2053B datasheet)
				m_shifter_last3 = 0;

				if (m_datalatch)
				{
					m_shifter_expected = 2;
				}
				else // escaped bit, skip
				{
					LOG("%s: 0111 escape sequence!\n", tag());
					return;
				}
			}
		}

		m_shifter_last3 >>= 1;
		m_shifter_last3 |= (m_datalatch << 2);

		m_shifter &= ~(1 << m_shift_pos);
		m_shifter |= (m_datalatch << m_shift_pos);
		m_shift_pos++;

		if (m_shifter_expected > 0)
		{
			m_shifter_expected--;
		}

		if (m_shifter_expected == 0)
		{
			if ((m_shifter & 0xff00) == 0x1e00)
			{
				LOG("%s: Command word is %04x\n", tag(), m_shifter & 0x7fff);
				m_pll_control = m_shifter & 0xff;
				if (BIT(m_pll_control, 0))
				{
					m_shifter_expected = 22;
				}
				else
				{
					m_shifter_expected = -1;
				}
			}
			else
			{
				LOG("%s: Program word is %08x\n", tag(), m_shifter);
				int P = ((m_shifter >> 15) & 0x7f) + 3;
				int Q = ((m_shifter >> 4) & 0x7f) + 2;

				double ratio = (double)((double)P / (double)Q);
				double pclock = ratio * (double)clock() * 2.0f;

				int divider = 1 << ((m_shifter >> 11) & 7);
				pclock /= (double)divider;

				LOG("%s: Pixel clock is %f Hz from P %d Q %d ratio %f, divider %d (div factor %d)\n", tag(), pclock, P, Q, ratio, divider, (m_shifter >> 11) & 7);
				m_clkout_changed_cb((u32)pclock);

				m_shifter_expected = -1;
			}
			m_shift_pos = 0;
			m_shifter = 0;
		}
	}

	m_prev_shift_clock = state;
}



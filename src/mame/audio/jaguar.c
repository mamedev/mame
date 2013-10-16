// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Jaguar hardware

****************************************************************************

    ------------------------------------------------------------
    JERRY REGISTERS
    ------------------------------------------------------------
    F10000-F13FFF   R/W   xxxxxxxx xxxxxxxx   Jerry
    F10000            W   xxxxxxxx xxxxxxxx   JPIT1 - timer 1 pre-scaler
    F10004            W   xxxxxxxx xxxxxxxx   JPIT2 - timer 1 divider
    F10008            W   xxxxxxxx xxxxxxxx   JPIT3 - timer 2 pre-scaler
    F1000C            W   xxxxxxxx xxxxxxxx   JPIT4 - timer 2 divider
    F10010            W   ------xx xxxxxxxx   CLK1 - processor clock divider
    F10012            W   ------xx xxxxxxxx   CLK2 - video clock divider
    F10014            W   -------- --xxxxxx   CLK3 - chroma clock divider
    F10020          R/W   ---xxxxx ---xxxxx   JINTCTRL - interrupt control register
                      W   ---x---- --------      (J_SYNCLR - clear synchronous serial intf ints)
                      W   ----x--- --------      (J_ASYNCLR - clear asynchronous serial intf ints)
                      W   -----x-- --------      (J_TIM2CLR - clear timer 2 [tempo] interrupts)
                      W   ------x- --------      (J_TIM1CLR - clear timer 1 [sample] interrupts)
                      W   -------x --------      (J_EXTCLR - clear external interrupts)
                    R/W   -------- ---x----      (J_SYNENA - enable synchronous serial intf ints)
                    R/W   -------- ----x---      (J_ASYNENA - enable asynchronous serial intf ints)
                    R/W   -------- -----x--      (J_TIM2ENA - enable timer 2 [tempo] interrupts)
                    R/W   -------- ------x-      (J_TIM1ENA - enable timer 1 [sample] interrupts)
                    R/W   -------- -------x      (J_EXTENA - enable external interrupts)
    F10030          R/W   -------- xxxxxxxx   ASIDATA - asynchronous serial data
    F10032            W   -x------ -xxxxxxx   ASICTRL - asynchronous serial control
                      W   -x------ --------      (TXBRK - transmit break)
                      W   -------- -x------      (CLRERR - clear error)
                      W   -------- --x-----      (RINTEN - enable receiver interrupts)
                      W   -------- ---x----      (TINTEN - enable transmitter interrupts)
                      W   -------- ----x---      (RXIPOL - receiver input polarity)
                      W   -------- -----x--      (TXOPOL - transmitter output polarity)
                      W   -------- ------x-      (PAREN - parity enable)
                      W   -------- -------x      (ODD - odd parity select)
    F10032          R     xxx-xxxx x-xxxxxx   ASISTAT - asynchronous serial status
                    R     x------- --------      (ERROR - OR of PE,FE,OE)
                    R     -x------ --------      (TXBRK - transmit break)
                    R     --x----- --------      (SERIN - serial input)
                    R     ----x--- --------      (OE - overrun error)
                    R     -----x-- --------      (FE - framing error)
                    R     ------x- --------      (PE - parity error)
                    R     -------x --------      (TBE - transmit buffer empty)
                    R     -------- x-------      (RBF - receive buffer full)
                    R     -------- ---x----      (TINTEN - enable transmitter interrupts)
                    R     -------- ----x---      (RXIPOL - receiver input polarity)
                    R     -------- -----x--      (TXOPOL - transmitter output polarity)
                    R     -------- ------x-      (PAREN - parity enable)
                    R     -------- -------x      (ODD - odd parity)
    F10034          R/W   xxxxxxxx xxxxxxxx   ASICLK - asynchronous serial interface clock
    ------------------------------------------------------------
    F14000-F17FFF   R/W   xxxxxxxx xxxxxxxx   Joysticks and GPIO0-5
    F14000          R     xxxxxxxx xxxxxxxx   JOYSTICK - read joystick state
    F14000            W   x------- xxxxxxxx   JOYSTICK - latch joystick output
                      W   x------- --------      (enable joystick outputs)
                      W   -------- xxxxxxxx      (joystick output data)
    F14002          R     xxxxxxxx xxxxxxxx   JOYBUTS - button register
    F14800-F14FFF   R/W   xxxxxxxx xxxxxxxx   GPI00 - reserved
    F15000-F15FFF   R/W   xxxxxxxx xxxxxxxx   GPI01 - reserved
    F16000-F16FFF   R/W   xxxxxxxx xxxxxxxx   GPI02 - reserved
    F17000-F177FF   R/W   xxxxxxxx xxxxxxxx   GPI03 - reserved
    F17800-F17BFF   R/W   xxxxxxxx xxxxxxxx   GPI04 - reserved
    F17C00-F17FFF   R/W   xxxxxxxx xxxxxxxx   GPI05 - reserved
    ------------------------------------------------------------
    F18000-F1FFFF   R/W   xxxxxxxx xxxxxxxx   Jerry DSP
    F1A100          R/W   xxxxxxxx xxxxxxxx   D_FLAGS - DSP flags register
                    R/W   x------- --------      (DMAEN - DMA enable)
                    R/W   -x------ --------      (REGPAGE - register page)
                      W   --x----- --------      (D_EXT0CLR - clear external interrupt 0)
                      W   ---x---- --------      (D_TIM2CLR - clear timer 2 interrupt)
                      W   ----x--- --------      (D_TIM1CLR - clear timer 1 interrupt)
                      W   -----x-- --------      (D_I2SCLR - clear I2S interrupt)
                      W   ------x- --------      (D_CPUCLR - clear CPU interrupt)
                    R/W   -------x --------      (D_EXT0ENA - enable external interrupt 0)
                    R/W   -------- x-------      (D_TIM2ENA - enable timer 2 interrupt)
                    R/W   -------- -x------      (D_TIM1ENA - enable timer 1 interrupt)
                    R/W   -------- --x-----      (D_I2SENA - enable I2S interrupt)
                    R/W   -------- ---x----      (D_CPUENA - enable CPU interrupt)
                    R/W   -------- ----x---      (IMASK - interrupt mask)
                    R/W   -------- -----x--      (NEGA_FLAG - ALU negative)
                    R/W   -------- ------x-      (CARRY_FLAG - ALU carry)
                    R/W   -------- -------x      (ZERO_FLAG - ALU zero)
    F1A102          R/W   -------- ------xx   D_FLAGS - upper DSP flags
                    R/W   -------- ------x-      (D_EXT1ENA - enable external interrupt 1)
                    R/W   -------- -------x      (D_EXT1CLR - clear external interrupt 1)
    F1A104            W   -------- ----xxxx   D_MTXC - matrix control register
                      W   -------- ----x---      (MATCOL - column/row major)
                      W   -------- -----xxx      (MATRIX3-15 - matrix width)
    F1A108            W   ----xxxx xxxxxx--   D_MTXA - matrix address register
    F1A10C            W   -------- -----x-x   D_END - data organization register
                      W   -------- -----x--      (BIG_INST - big endian instruction fetch)
                      W   -------- -------x      (BIG_IO - big endian I/O)
    F1A110          R/W   xxxxxxxx xxxxxxxx   D_PC - DSP program counter
    F1A114          R/W   xxxxxxxx xx-xxxxx   D_CTRL - DSP control/status register
                    R     xxxx---- --------      (VERSION - DSP version code)
                    R/W   ----x--- --------      (BUS_HOG - hog the bus!)
                    R/W   -----x-- --------      (D_EXT0LAT - external interrupt 0 latch)
                    R/W   ------x- --------      (D_TIM2LAT - timer 2 interrupt latch)
                    R/W   -------x --------      (D_TIM1LAT - timer 1 interrupt latch)
                    R/W   -------- x-------      (D_I2SLAT - I2S interrupt latch)
                    R/W   -------- -x------      (D_CPULAT - CPU interrupt latch)
                    R/W   -------- ---x----      (SINGLE_GO - single step one instruction)
                    R/W   -------- ----x---      (SINGLE_STEP - single step mode)
                    R/W   -------- -----x--      (FORCEINT0 - cause interrupt 0 on GPU)
                    R/W   -------- ------x-      (CPUINT - send GPU interrupt to CPU)
                    R/W   -------- -------x      (DSPGO - enable DSP execution)
    F1A116          R/W   -------- -------x   D_CTRL - upper DSP control/status register
                    R/W   -------- -------x      (D_EXT1LAT - external interrupt 1 latch)
    F1A118-F1A11B     W   xxxxxxxx xxxxxxxx   D_MOD - modulo instruction mask
    F1A11C-F1A11F   R     xxxxxxxx xxxxxxxx   D_REMAIN - divide unit remainder
    F1A11C            W   -------- -------x   D_DIVCTRL - divide unit control
                      W   -------- -------x      (DIV_OFFSET - 1=16.16 divide, 0=32-bit divide)
    F1A120-F1A123   R     xxxxxxxx xxxxxxxx   D_MACHI - multiply & accumulate high bits
    F1A148            W   xxxxxxxx xxxxxxxx   R_DAC - right transmit data
    F1A14C            W   xxxxxxxx xxxxxxxx   L_DAC - left transmit data
    F1A150            W   -------- xxxxxxxx   SCLK - serial clock frequency
    F1A150          R     -------- ------xx   SSTAT
                    R     -------- ------x-      (left - no description)
                    R     -------- -------x      (WS - word strobe status)
    F1A154            W   -------- --xxxx-x   SMODE - serial mode
                      W   -------- --x-----      (EVERYWORD - interrupt on MSB of every word)
                      W   -------- ---x----      (FALLING - interrupt on falling edge)
                      W   -------- ----x---      (RISING - interrupt of rising edge)
                      W   -------- -----x--      (WSEN - enable word strobes)
                      W   -------- -------x      (INTERNAL - enables serial clock)
    ------------------------------------------------------------
    F1B000-F1CFFF   R/W   xxxxxxxx xxxxxxxx   Local DSP RAM
    ------------------------------------------------------------
    F1D000          R     xxxxxxxx xxxxxxxx   ROM_TRI - triangle wave
    F1D200          R     xxxxxxxx xxxxxxxx   ROM_SINE - full sine wave
    F1D400          R     xxxxxxxx xxxxxxxx   ROM_AMSINE - amplitude modulated sine wave
    F1D600          R     xxxxxxxx xxxxxxxx   ROM_12W - sine wave and second order harmonic
    F1D800          R     xxxxxxxx xxxxxxxx   ROM_CHIRP16 - chirp
    F1DA00          R     xxxxxxxx xxxxxxxx   ROM_NTRI - triangle wave with noise
    F1DC00          R     xxxxxxxx xxxxxxxx   ROM_DELTA - spike
    F1DE00          R     xxxxxxxx xxxxxxxx   ROM_NOISE - white noise
    ------------------------------------------------------------
    F20000-FFFFFF   R     xxxxxxxx xxxxxxxx   Bootstrap ROM

****************************************************************************/

#include "emu.h"
#include "includes/jaguar.h"
#include "cpu/jaguar/jaguar.h"


/* Jerry registers */
enum
{
	JPIT1,      DSP0,       JPIT2,      DSP1,
	JPIT3,      DSP2,       JPIT4,      DSP3,
	CLK1,       CLK2,       CLK3,
	JINTCTRL = 0x20/2,
	ASIDATA = 0x30/2,   ASICTRL,    ASICLK,
	DSP_REGS
};



/*************************************
 *
 *  Jerry -> GPU interrupts
 *
 *************************************/

void jaguar_state::update_gpu_irq()
{
	if (m_gpu_irq_state & m_dsp_regs[JINTCTRL] & 0x1f)
	{
		m_gpu->set_input_line(1, ASSERT_LINE);
		gpu_resume();
	}
	else
		m_gpu->set_input_line(1, CLEAR_LINE);
}


WRITE_LINE_MEMBER( jaguar_state::external_int )
{
	if (state != CLEAR_LINE)
		m_gpu_irq_state |= 1;
	else
		m_gpu_irq_state &= ~1;
	update_gpu_irq();
}



/*************************************
 *
 *  Sound initialization
 *
 *************************************/

void jaguar_state::sound_start()
{
	m_serial_timer = timer_alloc(TID_SERIAL);

#if ENABLE_SPEEDUP_HACKS
	if (m_hacks_enabled)
		m_dsp->space(AS_PROGRAM).install_write_handler(0xf1a100, 0xf1a103, write32_delegate(FUNC(jaguar_state::dsp_flags_w), this));
#endif
}



/*************************************
 *
 *  Jerry 16-bit register access
 *
 *************************************/

READ16_MEMBER( jaguar_state::jerry_regs_r )
{
	if (offset != JINTCTRL && offset != JINTCTRL+2)
		logerror("%08X:jerry read register @ F10%03X\n", space.device().safe_pcbase(), offset * 2);

	switch (offset)
	{
		case JINTCTRL:
			return m_gpu_irq_state;
		case ASICTRL:
			return (m_dsp_regs[offset] & 0xfeff) | 0x100; // assume fifo empty
	}

	return m_dsp_regs[offset];
}


WRITE16_MEMBER( jaguar_state::jerry_regs_w )
{
	COMBINE_DATA(&m_dsp_regs[offset]);

	switch (offset)
	{
		case JINTCTRL:
			m_gpu_irq_state &= ~(m_dsp_regs[JINTCTRL] >> 8);
			update_gpu_irq();
			break;
	}

	if (offset != JINTCTRL && offset != JINTCTRL+2 && offset != ASICTRL)
		logerror("%08X:jerry write register @ F10%03X = %04X\n", space.device().safe_pcbase(), offset * 2, data);
}



/*************************************
 *
 *  Speedier sound hack
 *
 *************************************/

#if ENABLE_SPEEDUP_HACKS

WRITE32_MEMBER( jaguar_state::dsp_flags_w )
{
	/* write the data through */
	m_dsp->ctrl_w(space, offset, data, mem_mask);

	/* if they were clearing the A2S interrupt, see if we are headed for the spin */
	/* loop with R22 != 0; if we are, just start spinning again */
	if (&space.device() == m_dsp && ACCESSING_BITS_8_15 && (data & 0x400))
	{
		/* see if we're going back to the spin loop */
		if (!(data & 0x04000) && m_dsp->state_int(JAGUAR_R22) != 0)
		{
			UINT32 r30 = m_dsp->state_int(JAGUAR_R30) & 0xffffff;
			if (r30 >= 0xf1b124 && r30 <= 0xf1b126)
				dsp_suspend();
		}
	}
}

#endif



/*************************************
 *
 *  Serial port interrupt generation
 *
 *************************************/

void jaguar_state::serial_update()
{
	if (m_hacks_enabled)
	{
		/* assert the A2S IRQ on CPU #2 (DSP) */
		m_dsp->set_input_line(1, ASSERT_LINE);
		dsp_resume();

		/* fix flaky code in interrupt handler which thwarts our speedup */
		if ((m_dsp_ram[0x3e/4] & 0xffff) == 0xbfbc &&
			(m_dsp_ram[0x42/4] & 0xffff) == 0xe400)
		{
			/* move the store r28,(r29) into the branch delay slot, swapping it with */
			/* the nop that's currently there */
			m_dsp_ram[0x3e/4] = (m_dsp_ram[0x3e/4] & 0xffff0000) | 0xe400;
			m_dsp_ram[0x42/4] = (m_dsp_ram[0x42/4] & 0xffff0000) | 0xbfbc;
		}
	}
	else
	{
		/* assert the A2S IRQ on CPU #2 (DSP) */
		m_dsp->set_input_line(1, ASSERT_LINE);
		dsp_resume();
	}
}



/*************************************
 *
 *  Serial port handlers
 *
 *************************************/

READ32_MEMBER( jaguar_state::serial_r )
{
	logerror("%08X:jaguar_serial_r(%X)\n", space.device().safe_pcbase(), offset);
	return 0;
}


WRITE32_MEMBER( jaguar_state::serial_w )
{
	switch (offset)
	{
		/* right DAC */
		case 2:
			m_dac2->write_signed16((data & 0xffff) ^ 0x8000);
			break;

		/* left DAC */
		case 3:
			m_dac1->write_signed16((data & 0xffff) ^ 0x8000);
			break;

		/* frequency register */
		case 4:
			m_serial_frequency = data & 0xffff;
			break;

		/* control register -- only very specific modes supported */
		case 5:
			if ((data & 0x3f) != 0x15)
				logerror("Unexpected write to SMODE = %X\n", data);
			if ((data & 0x3f) == 0x15)
			{
				attotime rate = attotime::from_hz(26000000) * (32 * 2 * (m_serial_frequency + 1));
				m_serial_timer->adjust(rate, 0, rate);
			}
			break;

		default:
			logerror("%08X:jaguar_serial_w(%X,%X)\n", space.device().safe_pcbase(), offset, data);
			break;
	}
}

// license:BSD-3-Clause
// copyright-holders:Quench
/****************************************************************************
 *  Twin Cobra                                                              *
 *  Communications and memory functions between shared CPU memory spaces    *
 ****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms32010/tms32010.h"
#include "twincobr.h"

#define VERBOSE (0)
#include "logmacro.h"


void twincobr_state::twincobr_vblank_irq(int state)
{
	if (state && m_intenable)
		m_maincpu->set_input_line(M68K_IRQ_4, ASSERT_LINE);
}


void twincobr_state::twincobr_dsp_addrsel_w(u16 data)
{
	/* This sets the main CPU RAM address the DSP should */
	/*  read/write, via the DSP IO port 0 */
	/* Top three bits of data need to be shifted left 3 places */
	/*  to select which memory bank from main CPU address */
	/*  space to use */
	/* Lower thirteen bits of this data is shifted left one position */
	/*  to move it to an even address word boundary */

	m_main_ram_seg = ((data & 0xe000) << 3);
	m_dsp_addr_w   = ((data & 0x1fff) << 1);

	LOG("DSP PC:%04x IO write %04x (%08x) at port 0\n", m_dsp->pcbase(), data, m_main_ram_seg + m_dsp_addr_w);
}

u16 twincobr_state::twincobr_dsp_r()
{
	/* DSP can read data from main CPU RAM via DSP IO port 1 */

	u16 input_data = 0;
	switch (m_main_ram_seg)
	{
	case 0x30000:
	case 0x40000:
	case 0x50000:
		{
			address_space &mainspace = m_maincpu->space(AS_PROGRAM);
			input_data = mainspace.read_word(m_main_ram_seg + m_dsp_addr_w);
			break;
		}
	default:
		if (!machine().side_effects_disabled())
			logerror("DSP PC:%04x Warning !!! IO reading from %08x (port 1)\n", m_dsp->pcbase(), m_main_ram_seg + m_dsp_addr_w);
		break;
	}
	if (!machine().side_effects_disabled())
		LOG("DSP PC:%04x IO read %04x at %08x (port 1)\n", m_dsp->pcbase(), input_data, m_main_ram_seg + m_dsp_addr_w);
	return input_data;
}

void twincobr_state::twincobr_dsp_w(u16 data)
{
	/* Data written to main CPU RAM via DSP IO port 1 */
	m_dsp_execute = false;
	switch (m_main_ram_seg)
	{
	case 0x30000:
		if ((m_dsp_addr_w < 3) && (data == 0)) m_dsp_execute = true;
		[[fallthrough]];
	case 0x40000:
	case 0x50000:
		{
			address_space &mainspace = m_maincpu->space(AS_PROGRAM);
			mainspace.write_word(m_main_ram_seg + m_dsp_addr_w, data);
			break;
		}
	default:
		logerror("DSP PC:%04x Warning !!! IO writing to %08x (port 1)\n", m_dsp->pcbase(), m_main_ram_seg + m_dsp_addr_w);
		break;
	}
	LOG("DSP PC:%04x IO write %04x at %08x (port 1)\n", m_dsp->pcbase(), data, m_main_ram_seg + m_dsp_addr_w);
}

void twincobr_state::wardner_dsp_addrsel_w(u16 data)
{
	/* This sets the main CPU RAM address the DSP should */
	/*  read/write, via the DSP IO port 0 */
	/* Lower twelve bits of this data is shifted left one position */
	/*  to move it to an even address boundary */

	m_main_ram_seg =  (data & 0xe000);
	m_dsp_addr_w   = ((data & 0x07ff) << 1);

	if (m_main_ram_seg == 0x6000) m_main_ram_seg = 0x7000;

	LOG("DSP PC:%04x IO write %04x (%08x) at port 0\n", m_dsp->pcbase(), data, m_main_ram_seg + m_dsp_addr_w);
}

u16 twincobr_state::wardner_dsp_r()
{
	/* DSP can read data from main CPU RAM via DSP IO port 1 */

	u16 input_data = 0;
	switch (m_main_ram_seg)
	{
	case 0x7000:
	case 0x8000:
	case 0xa000:
		{
			address_space &mainspace = m_maincpu->space(AS_PROGRAM);
			input_data =  mainspace.read_byte(m_main_ram_seg + (m_dsp_addr_w + 0))
						| (mainspace.read_byte(m_main_ram_seg + (m_dsp_addr_w + 1)) << 8);
			break;
		}
	default:
		if (!machine().side_effects_disabled())
			logerror("DSP PC:%04x Warning !!! IO reading from %08x (port 1)\n", m_dsp->pcbase(), m_main_ram_seg + m_dsp_addr_w);
		break;
	}
	if (!machine().side_effects_disabled())
		LOG("DSP PC:%04x IO read %04x at %08x (port 1)\n", m_dsp->pcbase(), input_data, m_main_ram_seg + m_dsp_addr_w);
	return input_data;
}

void twincobr_state::wardner_dsp_w(u16 data)
{
	/* Data written to main CPU RAM via DSP IO port 1 */
	m_dsp_execute = false;
	switch (m_main_ram_seg)
	{
	case 0x7000:
		if ((m_dsp_addr_w < 3) && (data == 0)) m_dsp_execute = true;
		[[fallthrough]];
	case 0x8000:
	case 0xa000:
		{
			address_space &mainspace = m_maincpu->space(AS_PROGRAM);
			mainspace.write_byte(m_main_ram_seg + (m_dsp_addr_w + 0), (data & 0xff));
			mainspace.write_byte(m_main_ram_seg + (m_dsp_addr_w + 1), ((data >> 8) & 0xff));
			break;
		}
	default:
		logerror("DSP PC:%04x Warning !!! IO writing to %08x (port 1)\n", m_dsp->pcbase(), m_main_ram_seg + m_dsp_addr_w);
		break;
	}
	LOG("DSP PC:%04x IO write %04x at %08x (port 1)\n", m_dsp->pcbase(), data, m_main_ram_seg + m_dsp_addr_w);
}

void twincobr_state::twincobr_dsp_bio_w(u16 data)
{
	/* data 0xffff  means inhibit BIO line to DSP and enable */
	/*              communication to main processor */
	/*              Actually only DSP data bit 15 controls this */
	/* data 0x0000  means set DSP BIO line active and disable */
	/*              communication to main processor*/
	LOG("DSP PC:%04x IO write %04x at port 3\n", m_dsp->pcbase(), data);
	if (BIT(data, 15))
	{
		m_dsp_bio = CLEAR_LINE;
	}
	if (data == 0)
	{
		if (m_dsp_execute)
		{
			LOG("Turning the main CPU on\n");
			m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			m_dsp_execute = false;
		}
		m_dsp_bio = ASSERT_LINE;
	}
}

u16 twincobr_state::fsharkbt_dsp_r()
{
	/* IO Port 2 used by Flying Shark bootleg */
	/* DSP reads data from an extra MCU (8741) at IO port 2 */
	/* Port is read three times during startup. First and last data */
	/*   read must equal, but second data read must be different */
	m_fsharkbt_8741 += 1;
	LOG("DSP PC:%04x IO read %04x from 8741 MCU (port 2)\n", m_dsp->pcbase(), BIT(m_fsharkbt_8741, 3));
	return BIT(m_fsharkbt_8741, 0);
}

void twincobr_state::fsharkbt_dsp_w(u16 data)
{
	/* Flying Shark bootleg DSP writes data to an extra MCU (8741) at IO port 2 */
#if 0
	logerror("DSP PC:%04x IO write from DSP RAM:%04x to 8741 MCU (port 2)\n", m_dsp->pcbase(), m_fsharkbt_8741);
#endif
}

int twincobr_state::twincobr_bio_r()
{
	return m_dsp_bio;
}


void twincobr_state::int_enable_w(int state)
{
	m_intenable = state;
	if (!state)
		m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
}

void twincobr_state::dsp_int_w(int state)
{
	if (state)
	{
		// assert the INT line to the DSP
		LOG("Turning DSP on and main CPU off\n");
		m_dsp->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_dsp->set_input_line(0, ASSERT_LINE); // TMS32010 INT
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
	else
	{
		// inhibit the INT line to the DSP
		LOG("Turning DSP off\n");
		m_dsp->set_input_line(0, CLEAR_LINE); // TMS32010 INT
		m_dsp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
}


void twincobr_state::coin_counter_1_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

void twincobr_state::coin_counter_2_w(int state)
{
	machine().bookkeeping().coin_counter_w(1, state);
}

void twincobr_state::coin_lockout_1_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, !state);
}

void twincobr_state::coin_lockout_2_w(int state)
{
	machine().bookkeeping().coin_counter_w(1, !state);
}


u8 twincobr_state::twincobr_sharedram_r(offs_t offset)
{
	return m_sharedram[offset];
}

void twincobr_state::twincobr_sharedram_w(offs_t offset, u8 data)
{
	m_sharedram[offset] = data;
}

void twincobr_state::machine_start()
{
	save_item(NAME(m_intenable));
	save_item(NAME(m_dsp_addr_w));
	save_item(NAME(m_main_ram_seg));
	save_item(NAME(m_dsp_bio));
	save_item(NAME(m_dsp_execute));
	save_item(NAME(m_fsharkbt_8741));
}

void twincobr_state::machine_reset()
{
	m_dsp_addr_w = 0;
	m_main_ram_seg = 0;
	m_dsp_execute = false;
	m_dsp_bio = CLEAR_LINE;
	m_fsharkbt_8741 = -1;
}

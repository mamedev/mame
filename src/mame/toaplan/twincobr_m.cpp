// license:BSD-3-Clause
// copyright-holders:Quench
/****************************************************************************
 *  Twin Cobra                                                              *
 *  Communications and memory functions between shared CPU memory spaces    *
 ****************************************************************************/

#include "emu.h"
#include "twincobr.h"

#include "cpu/m68000/m68000.h"

#define VERBOSE (0)
#include "logmacro.h"


void twincobr_state::twincobr_vblank_irq(int state)
{
	if (state && m_intenable)
		m_maincpu->set_input_line(M68K_IRQ_4, ASSERT_LINE);
}


void twincobr_state::dsp_host_addr_cb(u16 data, u32 &seg, u32 &addr)
{
	/* This sets the main CPU RAM address the DSP should */
	/*  read/write, via the DSP IO port 0 */
	/* Top three bits of data need to be shifted left 3 places */
	/*  to select which memory bank from main CPU address */
	/*  space to use */
	/* Lower thirteen bits of this data is shifted left one position */
	/*  to move it to an even address word boundary */

	seg  = ((data & 0xe000) << 3);
	addr = ((data & 0x1fff) << 1);
}

u16 twincobr_state::dsp_host_read_cb(u32 seg, u32 addr)
{
	u16 input_data = 0;
	switch (seg)
	{
	case 0x30000:
	case 0x40000:
	case 0x50000:
		{
			address_space &mainspace = m_maincpu->space(AS_PROGRAM);
			input_data = mainspace.read_word(seg + addr);
			break;
		}
	default:
		if (!machine().side_effects_disabled())
			logerror("%s: Warning !!! IO reading from %08x (port 1)\n", machine().describe_context(), seg + addr);
		break;
	}
	return input_data;
}

bool twincobr_state::dsp_host_write_cb(u32 seg, u32 addr, u16 data)
{
	bool execute = false;
	switch (seg)
	{
	case 0x30000:
		if ((addr < 3) && (data == 0)) execute = true;
		[[fallthrough]];
	case 0x40000:
	case 0x50000:
		{
			address_space &mainspace = m_maincpu->space(AS_PROGRAM);
			mainspace.write_word(seg + addr, data);
			break;
		}
	default:
		logerror("%s: Warning !!! IO writing to %08x (port 1)\n", machine().describe_context(), seg + addr);
		break;
	}
	return execute;
}

u16 twincobr_state::fsharkbt_dsp_r()
{
	/* IO Port 2 used by Flying Shark bootleg */
	/* DSP reads data from an extra MCU (8741) at IO port 2 */
	/* Port is read three times during startup. First and last data */
	/*   read must equal, but second data read must be different */
	if (!machine().side_effects_disabled())
	{
		m_fsharkbt_8741 += 1;
		LOG("%s: IO read %04x from 8741 MCU (port 2)\n", machine().describe_context(), BIT(m_fsharkbt_8741, 3));
	}
	return BIT(m_fsharkbt_8741, 0);
}

void twincobr_state::fsharkbt_dsp_w(u16 data)
{
	/* Flying Shark bootleg DSP writes data to an extra MCU (8741) at IO port 2 */
#if 0
	logerror("%s: IO write from DSP RAM:%04x to 8741 MCU (port 2)\n", machine().describe_context(), m_fsharkbt_8741);
#endif
}


void twincobr_state::int_enable_w(int state)
{
	m_intenable = state;
	if (!state)
		m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
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
	save_item(NAME(m_fsharkbt_8741));
}

void twincobr_state::machine_reset()
{
	m_fsharkbt_8741 = -1;
}

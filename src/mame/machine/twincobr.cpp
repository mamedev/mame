// license:BSD-3-Clause
// copyright-holders:Quench
/****************************************************************************
 *  Twin Cobra                                                              *
 *  Communications and memory functions between shared CPU memory spaces    *
 ****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms32010/tms32010.h"
#include "includes/twincobr.h"


#define LOG_DSP_CALLS 0
#define LOG(x) do { if (LOG_DSP_CALLS) logerror x; } while (0)


WRITE_LINE_MEMBER(twincobr_state::twincobr_vblank_irq)
{
	if (state && m_intenable)
		m_maincpu->set_input_line(M68K_IRQ_4, ASSERT_LINE);
}


void twincobr_state::twincobr_dsp_addr_cb(u32 &main_ram_seg, u32 &dsp_addr, u16 data)
{
	// Top three bits of data need to be shifted left 3 places
	//  to select which memory bank from main CPU address
	//  space to use
	// Lower thirteen bits of this data is shifted left one position
	//  to move it to an even address word boundary

	main_ram_seg = ((data & 0xe000) << 3);
	dsp_addr     = ((data & 0x1fff) << 1);
}


bool twincobr_state::twincobr_dsp_read_cb(u32 main_ram_seg, u32 dsp_addr, u16 &data)
{
	bool res = false;

	switch (main_ram_seg)
	{
	case 0x30000:
	case 0x40000:
	case 0x50000:
		{
			address_space &mainspace = m_maincpu->space(AS_PROGRAM);
			data = mainspace.read_word(main_ram_seg + dsp_addr);
			res = true;
			break;
		}
	default:
		break;
	}

	return res;
}


bool twincobr_state::twincobr_dsp_write_cb(u32 main_ram_seg, u32 dsp_addr, bool &dsp_execute, u16 data)
{
	bool res = false;

	switch (main_ram_seg)
	{
	case 0x30000:
		if ((dsp_addr < 3) && (data == 0))
			dsp_execute = true;
		[[fallthrough]];
	case 0x40000:
	case 0x50000:
		{
			address_space &mainspace = m_maincpu->space(AS_PROGRAM);
			mainspace.write_word(main_ram_seg + dsp_addr, data);
			res = true;
			break;
		}
	default:
		break;
	}

	return res;
}


void twincobr_state::wardner_dsp_addr_cb(u32 &main_ram_seg, u32 &dsp_addr, u16 data)
{
	// Lower twelve bits of this data is shifted left one position
	//  to move it to an even address boundary

	main_ram_seg =  (data & 0xe000);
	dsp_addr     = ((data & 0x07ff) << 1);

	if (main_ram_seg == 0x6000) main_ram_seg = 0x7000;
}


bool twincobr_state::wardner_dsp_read_cb(u32 main_ram_seg, u32 dsp_addr, u16 &data)
{
	bool res = false;

	switch (main_ram_seg)
	{
	case 0x7000:
	case 0x8000:
	case 0xa000:
		{
			address_space &mainspace = m_maincpu->space(AS_PROGRAM);
			data =  mainspace.read_byte(main_ram_seg + (dsp_addr + 0))
				| (mainspace.read_byte(main_ram_seg + (dsp_addr + 1)) << 8);
			res = true;
			break;
		}
	default:
		break;
	}

	return res;
}


bool twincobr_state::wardner_dsp_write_cb(u32 main_ram_seg, u32 dsp_addr, bool &dsp_execute, u16 data)
{
	bool res = false;

	switch (main_ram_seg)
	{
	case 0x7000:
		if ((dsp_addr < 3) && (data == 0)) dsp_execute = true;
		[[fallthrough]];
	case 0x8000:
	case 0xa000:
		{
			address_space &mainspace = m_maincpu->space(AS_PROGRAM);
			mainspace.write_byte(main_ram_seg + (dsp_addr + 0), (data & 0xff));
			mainspace.write_byte(main_ram_seg + (dsp_addr + 1), ((data >> 8) & 0xff));
			res = true;
			break;
		}
	default:
		break;
	}

	return res;
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
		LOG(("DSP PC:%04x IO read %04x from 8741 MCU (port 2)\n",m_dsp->pcbase(),(m_fsharkbt_8741 & 0x08)));
	}
	return (m_fsharkbt_8741 & 1);
}


void twincobr_state::fsharkbt_dsp_w(u16 data)
{
	/* Flying Shark bootleg DSP writes data to an extra MCU (8741) at IO port 2 */
#if 0
	logerror("DSP PC:%04x IO write from DSP RAM:%04x to 8741 MCU (port 2)\n",m_dsp->pcbase(),m_fsharkbt_8741);
#endif
}


WRITE_LINE_MEMBER(twincobr_state::int_enable_w)
{
	m_intenable = state;
	if (!state)
		m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
}


WRITE_LINE_MEMBER(twincobr_state::coin_counter_1_w)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

WRITE_LINE_MEMBER(twincobr_state::coin_counter_2_w)
{
	machine().bookkeeping().coin_counter_w(1, state);
}

WRITE_LINE_MEMBER(twincobr_state::coin_lockout_1_w)
{
	machine().bookkeeping().coin_counter_w(0, !state);
}

WRITE_LINE_MEMBER(twincobr_state::coin_lockout_2_w)
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


void twincobr_state::machine_reset()
{
	m_fsharkbt_8741 = -1;
}

void twincobr_state::driver_savestate()
{
	save_item(NAME(m_intenable));
	save_item(NAME(m_fsharkbt_8741));
}

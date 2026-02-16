// license:BSD-3-Clause
// copyright-holders:Darren Olafson, Quench,Stephane Humbert
/***************************************************************************
                ToaPlan game hardware from 1988-1991
                ------------------------------------
 ***************************************************************************/

#include "emu.h"
#include "toaplan1.h"


void toaplan1_state::interrupt()
{
	if (m_intenable)
		m_maincpu->set_input_line(4, HOLD_LINE);
}

void toaplan1_state::intenable_w(u8 data)
{
	m_intenable = data;
}


void toaplan1_demonwld_state::dsp_host_addr_cb(u16 data, u32 &seg, u32 &addr)
{
	/* This sets the main CPU RAM address the DSP should */
	/*  read/write, via the DSP IO port 0 */
	/* Top three bits of data need to be shifted left 9 places */
	/*  to select which memory bank from main CPU address */
	/*  space to use */
	/* Lower thirteen bits of this data is shifted left one position */
	/*  to move it to an even address word boundary */

	seg  = ((data & 0xe000) << 9);
	addr = ((data & 0x1fff) << 1);
}

u16 toaplan1_demonwld_state::dsp_host_read_cb(u32 seg, u32 addr)
{
	u16 input_data = 0;

	switch (seg)
	{
		case 0xc00000:
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

bool toaplan1_demonwld_state::dsp_host_write_cb(u32 seg, u32 addr, u16 data)
{
	bool execute = false;

	switch (seg)
	{
		case 0xc00000:
		{
			if ((addr < 3) && (data == 0)) execute = true;
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

void toaplan1_demonwld_state::dsp_ctrl_w(u8 data)
{
#if 0
	logerror("%s: Writing %02x to $e0000b.\n", machine().describe_context(), data);
#endif

	if (data & ~0x01)
		logerror("%s: Writing unknown command %02x to $e0000b\n", machine().describe_context(), data);
	else
		m_dsp->dsp_int_w(BIT(~data, 0));
}


u8 toaplan1_samesame_state::port_6_word_r()
{
	/* Bit 0x80 is secondary CPU (HD647180) ready signal */
	return (m_soundlatch->pending_r() ? 0 : 0x80) | m_tjump_io->read();
}

u8 toaplan1_state::shared_r(offs_t offset)
{
	return m_sharedram[offset];
}

void toaplan1_state::shared_w(offs_t offset, u8 data)
{
	m_sharedram[offset] = data;
}


void toaplan1_state::reset_sound()
{
	/* Reset the secondary CPU and sound chip */
	/* rallybik, truxton, hellfire, demonwld write to a port to cause a reset */
	/* zerowing, fireshrk, outzone, vimana use a RESET instruction instead */
	m_ymsnd->reset();
	m_audiocpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

void toaplan1_samesame_state::reset_sound()
{
	toaplan1_state::reset_sound();
	m_soundlatch->acknowledge_w();
}

void toaplan1_state::reset_sound_w(u8 data)
{
	if (data == 0) reset_sound();
}


void toaplan1_rallybik_state::coin_counter_1_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

void toaplan1_rallybik_state::coin_counter_2_w(int state)
{
	machine().bookkeeping().coin_counter_w(1, state);
}

void toaplan1_rallybik_state::coin_lockout_1_w(int state)
{
	machine().bookkeeping().coin_lockout_w(0, !state);
}

void toaplan1_rallybik_state::coin_lockout_2_w(int state)
{
	machine().bookkeeping().coin_lockout_w(1, !state);
}


void toaplan1_state::coin_w(u8 data)
{
	// Upper 4 bits are junk (normally 1110 or 0000, which are artifacts of sound command processing)
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
	machine().bookkeeping().coin_lockout_w(0, !BIT(data, 2));
	machine().bookkeeping().coin_lockout_w(1, !BIT(data, 3));
}

void toaplan1_state::reset_callback(int state)
{
	reset_sound();
}

void toaplan1_state::machine_reset()
{
	m_intenable = 0;
	machine().bookkeeping().coin_lockout_global_w(0);
}

void toaplan1_state::machine_start()
{
	save_item(NAME(m_intenable));
}

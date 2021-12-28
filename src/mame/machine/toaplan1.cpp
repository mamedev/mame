// license:BSD-3-Clause
// copyright-holders:Darren Olafson, Quench,Stephane Humbert
/***************************************************************************
                ToaPlan game hardware from 1988-1991
                ------------------------------------
 ***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/tms32010/tms32010.h"
#include "sound/ymopl.h"
#include "includes/toaplan1.h"


void toaplan1_state::interrupt()
{
	if (m_intenable)
		m_maincpu->set_input_line(4, HOLD_LINE);
}

void toaplan1_state::intenable_w(u8 data)
{
	m_intenable = data;
}


void toaplan1_demonwld_state::dsp_addr_cb(u32 &main_ram_seg, u32 &dsp_addr, u16 data)
{
	// Top three bits of data need to be shifted left 9 places
	//  to select which memory bank from main CPU address
	//  space to use
	// Lower thirteen bits of this data is shifted left one position
	//  to move it to an even address word boundary

	main_ram_seg = ((data & 0xe000) << 9);
	dsp_addr     = ((data & 0x1fff) << 1);
}


bool toaplan1_demonwld_state::dsp_read_cb(u32 main_ram_seg, u32 dsp_addr, u16 &data)
{
	bool res = false;

	switch (main_ram_seg)
	{
	case 0xc00000:
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


bool toaplan1_demonwld_state::dsp_write_cb(u32 main_ram_seg, u32 dsp_addr, bool &dsp_execute, u16 data)
{
	bool res = false;

	switch (main_ram_seg)
	{
	case 0xc00000:
		{
			if ((dsp_addr < 3) && (data == 0)) dsp_execute = true;
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


void toaplan1_demonwld_state::dsp_ctrl_w(u8 data)
{
#if 0
	logerror("68000:%08x  Writing %02x to $e0000b.\n",m_maincpu->pc() ,data);
#endif

	switch (data)
	{
		case 0x00:  m_gxl->dsp_int_w(ASSERT_LINE); break;  /* Enable the INT line to the DSP */
		case 0x01:  m_gxl->dsp_int_w(CLEAR_LINE);  break;  /* Inhibit the INT line to the DSP */
		default:    logerror("68000:%08x  Writing unknown command %02x to $e0000b\n",m_maincpu->pcbase() ,data); break;
	}
}


u8 toaplan1_samesame_state::port_6_word_r()
{
	/* Bit 0x80 is secondary CPU (HD647180) ready signal */
	logerror("PC:%08x Warning !!! IO reading from $14000b\n",m_maincpu->pcbase());
	return (0x80 | m_tjump_io->read()) & 0xff;
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

void toaplan1_state::reset_sound_w(u8 data)
{
	if (data == 0) reset_sound();
}


WRITE_LINE_MEMBER(toaplan1_rallybik_state::coin_counter_1_w)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

WRITE_LINE_MEMBER(toaplan1_rallybik_state::coin_counter_2_w)
{
	machine().bookkeeping().coin_counter_w(1, state);
}

WRITE_LINE_MEMBER(toaplan1_rallybik_state::coin_lockout_1_w)
{
	machine().bookkeeping().coin_lockout_w(0, !state);
}

WRITE_LINE_MEMBER(toaplan1_rallybik_state::coin_lockout_2_w)
{
	machine().bookkeeping().coin_lockout_w(1, !state);
}


void toaplan1_state::coin_w(u8 data)
{
	logerror("Z80 writing %02x to coin control\n",data);
	/* This still isnt too clear yet. */
	/* Coin C has no coin lock ? */
	/* Are some outputs for lights ? (no space on JAMMA for it though) */

	switch (data)
	{
		case 0xee: machine().bookkeeping().coin_counter_w(1,1); machine().bookkeeping().coin_counter_w(1,0); break; /* Count slot B */
		case 0xed: machine().bookkeeping().coin_counter_w(0,1); machine().bookkeeping().coin_counter_w(0,0); break; /* Count slot A */
	/* The following are coin counts after coin-lock active (faulty coin-lock ?) */
		case 0xe2: machine().bookkeeping().coin_counter_w(1,1); machine().bookkeeping().coin_counter_w(1,0); machine().bookkeeping().coin_lockout_w(1,1); break;
		case 0xe1: machine().bookkeeping().coin_counter_w(0,1); machine().bookkeeping().coin_counter_w(0,0); machine().bookkeeping().coin_lockout_w(0,1); break;

		case 0xec: machine().bookkeeping().coin_lockout_global_w(0); break;  /* ??? count games played */
		case 0xe8: break;   /* ??? Maximum credits reached with coin/credit ratio */
		case 0xe4: break;   /* ??? Reset coin system */

		case 0x0c: machine().bookkeeping().coin_lockout_global_w(0); break;  /* Unlock all coin slots */
		case 0x08: machine().bookkeeping().coin_lockout_w(2,0); break;   /* Unlock coin slot C */
		case 0x09: machine().bookkeeping().coin_lockout_w(0,0); break;   /* Unlock coin slot A */
		case 0x0a: machine().bookkeeping().coin_lockout_w(1,0); break;   /* Unlock coin slot B */

		case 0x02: machine().bookkeeping().coin_lockout_w(1,1); break;   /* Lock coin slot B */
		case 0x01: machine().bookkeeping().coin_lockout_w(0,1); break;   /* Lock coin slot A */
		case 0x00: machine().bookkeeping().coin_lockout_global_w(1); break;  /* Lock all coin slots */
		default:   logerror("PC:%04x  Writing unknown data (%04x) to coin count/lockout port\n",m_audiocpu->pcbase(),data); break;
	}
}

WRITE_LINE_MEMBER(toaplan1_state::reset_callback)
{
	reset_sound();
}

void toaplan1_state::machine_reset()
{
	m_intenable = 0;
	machine().bookkeeping().coin_lockout_global_w(0);
}

/* zerowing, fireshrk, outzone */
MACHINE_RESET_MEMBER(toaplan1_state,zerowing)
{
	machine_reset();
	m_maincpu->set_reset_callback(*this, FUNC(toaplan1_state::reset_callback));
}


void toaplan1_state::machine_start()
{
	save_item(NAME(m_intenable));
}

void toaplan1_samesame_state::machine_start()
{
	toaplan1_state::machine_start();
	save_item(NAME(m_to_mcu));
	save_item(NAME(m_cmdavailable));
}

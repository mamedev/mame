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


void toaplan1_demonwld_state::dsp_addrsel_w(u16 data)
{
	/* This sets the main CPU RAM address the DSP should */
	/*  read/write, via the DSP IO port 0 */
	/* Top three bits of data need to be shifted left 9 places */
	/*  to select which memory bank from main CPU address */
	/*  space to use */
	/* Lower thirteen bits of this data is shifted left one position */
	/*  to move it to an even address word boundary */

	m_main_ram_seg = ((data & 0xe000) << 9);
	m_dsp_addr_w   = ((data & 0x1fff) << 1);
	logerror("DSP PC:%04x IO write %04x (%08x) at port 0\n", m_dsp->pcbase(), data, m_main_ram_seg + m_dsp_addr_w);
}

u16 toaplan1_demonwld_state::dsp_r()
{
	/* DSP can read data from main CPU RAM via DSP IO port 1 */

	u16 input_data = 0;

	switch (m_main_ram_seg)
	{
		case 0xc00000: {address_space &mainspace = m_maincpu->space(AS_PROGRAM);
						input_data = mainspace.read_word(m_main_ram_seg + m_dsp_addr_w);
						break;}
		default:
			if (!machine().side_effects_disabled())
				logerror("DSP PC:%04x Warning !!! IO reading from %08x (port 1)\n", m_dsp->pcbase(), m_main_ram_seg + m_dsp_addr_w);
			break;
	}
	if (!machine().side_effects_disabled())
		logerror("DSP PC:%04x IO read %04x at %08x (port 1)\n", m_dsp->pcbase(), input_data, m_main_ram_seg + m_dsp_addr_w);
	return input_data;
}

void toaplan1_demonwld_state::dsp_w(u16 data)
{
	/* Data written to main CPU RAM via DSP IO port 1 */
	m_dsp_execute = false;
	switch (m_main_ram_seg)
	{
		case 0xc00000: {if ((m_dsp_addr_w < 3) && (data == 0)) m_dsp_execute = true;
						address_space &mainspace = m_maincpu->space(AS_PROGRAM);
						mainspace.write_word(m_main_ram_seg + m_dsp_addr_w, data);
						break;}
		default:        logerror("DSP PC:%04x Warning !!! IO writing to %08x (port 1)\n", m_dsp->pcbase(), m_main_ram_seg + m_dsp_addr_w);
	}
	logerror("DSP PC:%04x IO write %04x at %08x (port 1)\n", m_dsp->pcbase(), data, m_main_ram_seg + m_dsp_addr_w);
}

void toaplan1_demonwld_state::dsp_bio_w(u16 data)
{
	/* data 0xffff  means inhibit BIO line to DSP and enable */
	/*              communication to main processor */
	/*              Actually only DSP data bit 15 controls this */
	/* data 0x0000  means set DSP BIO line active and disable */
	/*              communication to main processor*/

	logerror("DSP PC:%04x IO write %04x at port 3\n", m_dsp->pcbase(), data);
	if (BIT(data, 15))
		m_dsp_bio = CLEAR_LINE;

	if (data == 0)
	{
		if (m_dsp_execute)
		{
			logerror("Turning 68000 on\n");
			m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			m_dsp_execute = false;
		}
		m_dsp_bio = ASSERT_LINE;
	}
}

int toaplan1_demonwld_state::bio_r()
{
	return m_dsp_bio;
}


void toaplan1_demonwld_state::dsp_int_w(int enable)
{
	m_dsp_on = enable;
	if (enable)
	{
		logerror("Turning DSP on and 68000 off\n");
		m_dsp->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_dsp->set_input_line(0, ASSERT_LINE); /* TMS32010 INT */
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
	else
	{
		logerror("Turning DSP off\n");
		m_dsp->set_input_line(0, CLEAR_LINE); /* TMS32010 INT */
		m_dsp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
}

void toaplan1_demonwld_state::device_post_load()
{
	dsp_int_w(m_dsp_on);
}

void toaplan1_demonwld_state::dsp_ctrl_w(u8 data)
{
#if 0
	logerror("68000:%08x  Writing %02x to $e0000b.\n", m_maincpu->pc(), data);
#endif

	switch (data)
	{
		case 0x00:  dsp_int_w(1); break;  /* Enable the INT line to the DSP */
		case 0x01:  dsp_int_w(0); break;  /* Inhibit the INT line to the DSP */
		default:    logerror("68000:%08x  Writing unknown command %02x to $e0000b\n", m_maincpu->pcbase(), data); break;
	}
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

void toaplan1_demonwld_state::machine_reset()
{
	toaplan1_state::machine_reset();
	m_dsp_addr_w = 0;
	m_main_ram_seg = 0;
	m_dsp_execute = false;
}


void toaplan1_state::machine_start()
{
	save_item(NAME(m_intenable));
}

void toaplan1_demonwld_state::machine_start()
{
	toaplan1_state::machine_start();
	save_item(NAME(m_dsp_on));
	save_item(NAME(m_dsp_addr_w));
	save_item(NAME(m_main_ram_seg));
	save_item(NAME(m_dsp_bio));
	save_item(NAME(m_dsp_execute));
}

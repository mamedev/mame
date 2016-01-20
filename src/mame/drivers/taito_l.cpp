// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*
  Taito L-System

  Monoprocessor games (1 main TC0090LVC (z80 core), no sound z80)
  - Plotting
  - Puzznic
  - Palamedes
  - Cachat / Tube-It
  - American Horseshoes
  - Play Girls
  - Play Girls 2
  - LA Girl
  - Cuby Bop

  Dual processor games (1 main TC0090LVC (z80 core), 1 sound z80)
  - Kuri Kinton
  - Evil Stone

  Triple processor games (1 main TC0090LVC (z80 core), 1 slave z80, 1 sound z80)
  - Fighting hawk
  - Raimais
  - Champion Wrestler

Notes:
- the system uses RAM based characters, which aren't really supported by the
  TileMap system, so we have to tilemap_mark_all_tiles_dirty() to compensate
- kurikinta has some debug dip switches (invulnerability, slow motion) so might
  be a prototype. It also doesn't have service mode (or has it disabled).

TODO:
- plgirls doesn't work without a kludge because of an interrupt issue. This
  happens because the program enables interrupts before setting IM2, so the
  interrupt vector is interpreted as IM0, which is obviously bogus.
- The  puzznic protection is worked around,  but I'm not happy with it
  (the 68705-returned values are wrong, I'm sure of that).
- A bunch of control registers are simply ignored
- The source of   irqs 0 and  1 is  unknown, while  2 is vblank  (0 is
  usually   ignored  by the  program,    1   leads  to  reading    the
  ports... maybe vbl-in, vbl-out and hblank ?).
- Text Plane colours are only right in Cuby Bop once you've started a game
  & reset
- Scrolling in Cuby Bop's Game seems incorrect.
- Repeated SFXs in Evil Stone (with previous hack, it was used to die at level 1 boss)
- Evil Stone audio NMI source is unknown.

puzznici note
- this set is a bootleg, it uses a converted board without the MCU and has
  a hacked copyright message.  The tilemap data for one of the girls appears
  to be corrupt, however this is correct, the bootleggers overwrote part of
  the data with the expected response sequence from the MCU in order to simulate
  it.

*/

#include "emu.h"
#include "includes/taitoipt.h"
#include "cpu/z80/z80.h"
#include "audio/taitosnd.h"
#include "sound/2203intf.h"
#include "sound/2610intf.h"
#include "sound/msm5205.h"
#include "includes/taito_l.h"

static const char * const bankname[] = { "bank2", "bank3", "bank4", "bank5" };

static const struct
{
	void (taitol_state::*notifier)(int);
	UINT32 offset;
} rambank_modify_notifiers[12] =
{
	{ &taitol_state::taitol_chardef14_m, 0x0000 }, // 14
	{ &taitol_state::taitol_chardef15_m, 0x1000 }, // 15
	{ &taitol_state::taitol_chardef16_m, 0x2000 }, // 16
	{ &taitol_state::taitol_chardef17_m, 0x3000 }, // 17

	{ &taitol_state::taitol_bg18_m, 0x8000 },      // 18
	{ &taitol_state::taitol_bg19_m, 0x9000 },      // 19
	{ &taitol_state::taitol_char1a_m, 0xa000 },    // 1a
	{ &taitol_state::taitol_obj1b_m, 0xb000 },     // 1b

	{ &taitol_state::taitol_chardef1c_m, 0x4000 }, // 1c
	{ &taitol_state::taitol_chardef1d_m, 0x5000 }, // 1d
	{ &taitol_state::taitol_chardef1e_m, 0x6000 }, // 1e
	{ &taitol_state::taitol_chardef1f_m, 0x7000 }, // 1f
};


void taitol_state::palette_notifier(int addr)
{
	UINT8 *p = m_palette_ram + (addr & ~1);
	UINT8 byte0 = *p++;
	UINT8 byte1 = *p;

	//  addr &= 0x1ff;

	if (addr > 0x200)
	{
		logerror("%s:Large palette ? %03x\n", machine().describe_context(), addr);
	}
	else
	{
		//      r = g = b = ((addr & 0x1e) != 0)*255;
		m_palette->set_pen_color(addr / 2, pal4bit(byte0), pal4bit(byte0 >> 4), pal4bit(byte1));
	}
}

static const UINT8 puzznic_mcu_reply[] = { 0x50, 0x1f, 0xb6, 0xba, 0x06, 0x03, 0x47, 0x05, 0x00 };

void taitol_state::state_register(  )
{
	save_item(NAME(m_irq_adr_table));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_cur_rambank));
	save_item(NAME(m_cur_rombank));
	save_item(NAME(m_cur_rombank2));

	save_item(NAME(m_adpcm_pos));
	save_item(NAME(m_adpcm_data));
	save_item(NAME(m_trackx));
	save_item(NAME(m_tracky));
	save_item(NAME(m_mux_ctrl));
	save_item(NAME(m_extport));
	save_item(NAME(m_last_irq_level));
	save_item(NAME(m_high));
	save_item(NAME(m_high2));

	save_item(NAME(m_mcu_pos));
	save_item(NAME(m_mcu_reply_len));
	save_item(NAME(m_last_data_adr));
	save_item(NAME(m_last_data));
	save_item(NAME(m_cur_bank));

	save_item(NAME(m_bankc));
	save_item(NAME(m_horshoes_gfxbank));
	save_item(NAME(m_cur_ctrl));
	save_item(NAME(m_flipscreen));
}

MACHINE_START_MEMBER(taitol_state,taito_l)
{
	save_item(NAME(m_rambanks));
	save_item(NAME(m_palette_ram));
	save_item(NAME(m_empty_ram));

	state_register();
}

void taitol_state::taito_machine_reset()
{
	int i;

	for (i = 0; i < 3; i++)
		m_irq_adr_table[i] = 0;

	m_irq_enable = 0;

	for (i = 0; i < 4; i++)
	{
		m_cur_rambank[i] = 0x80;
		m_current_notifier[i] = &taitol_state::palette_notifier;
		m_current_base[i] = m_palette_ram;
		membank(bankname[i])->set_base(m_current_base[i]);
	}

	m_cur_rombank = m_cur_rombank2 = 0;
	membank("bank1")->set_base(memregion("maincpu")->base());

	m_gfxdecode->gfx(2)->set_source(m_rambanks);

	m_adpcm_pos = 0;
	m_adpcm_data = -1;
	m_trackx = m_tracky = 0;
	m_mux_ctrl = 0;
	m_extport = 0;
	m_last_irq_level = 0;
	m_high = 0;
	m_high2 = 0;

	m_mcu_reply = puzznic_mcu_reply;

	m_mcu_pos = m_mcu_reply_len = 0;
	m_last_data_adr = m_last_data = 0;
	m_cur_bank = 1;

	/* video related */
	m_bankc[0] = m_bankc[1] = m_bankc[2] = m_bankc[3] = 0;
	m_horshoes_gfxbank = 0;
	m_cur_ctrl = 0;
	m_flipscreen = 0;
}


MACHINE_RESET_MEMBER(taitol_state,fhawk)
{
	taito_machine_reset();
	m_porte0_tag = nullptr;
	m_porte1_tag = nullptr;
	m_portf0_tag = nullptr;
	m_portf1_tag = nullptr;
}

MACHINE_RESET_MEMBER(taitol_state,raimais)
{
	taito_machine_reset();
	m_porte0_tag = nullptr;
	m_porte1_tag = nullptr;
	m_portf0_tag = nullptr;
	m_portf1_tag = nullptr;
}

MACHINE_RESET_MEMBER(taitol_state,champwr)
{
	taito_machine_reset();
	m_porte0_tag = nullptr;
	m_porte1_tag = nullptr;
	m_portf0_tag = nullptr;
	m_portf1_tag = nullptr;
}


MACHINE_RESET_MEMBER(taitol_state,kurikint)
{
	taito_machine_reset();
	m_porte0_tag = nullptr;
	m_porte1_tag = nullptr;
	m_portf0_tag = nullptr;
	m_portf1_tag = nullptr;
}

MACHINE_RESET_MEMBER(taitol_state,evilston)
{
	taito_machine_reset();
	m_porte0_tag = nullptr;
	m_porte1_tag = nullptr;
	m_portf0_tag = nullptr;
	m_portf1_tag = nullptr;
}

MACHINE_RESET_MEMBER(taitol_state,puzznic)
{
	taito_machine_reset();
	m_porte0_tag = "DSWA";
	m_porte1_tag = "DSWB";
	m_portf0_tag = "IN0";
	m_portf1_tag = "IN1";
}

MACHINE_RESET_MEMBER(taitol_state,plotting)
{
	taito_machine_reset();
	m_porte0_tag = "DSWA";
	m_porte1_tag = "DSWB";
	m_portf0_tag = "IN0";
	m_portf1_tag = "IN1";
}

MACHINE_RESET_MEMBER(taitol_state,palamed)
{
	taito_machine_reset();
	m_porte0_tag = "DSWA";
	m_porte1_tag = nullptr;
	m_portf0_tag = "DSWB";
	m_portf1_tag = nullptr;
}

MACHINE_RESET_MEMBER(taitol_state,cachat)
{
	taito_machine_reset();
	m_porte0_tag = "DSWA";
	m_porte1_tag = nullptr;
	m_portf0_tag = "DSWB";
	m_portf1_tag = nullptr;
}

MACHINE_RESET_MEMBER(taitol_state,horshoes)
{
	taito_machine_reset();
	m_porte0_tag = "DSWA";
	m_porte1_tag = "DSWB";
	m_portf0_tag = "IN0";
	m_portf1_tag = "IN1";
}


IRQ_CALLBACK_MEMBER(taitol_state::irq_callback)
{
	return m_irq_adr_table[m_last_irq_level];
}

TIMER_DEVICE_CALLBACK_MEMBER(taitol_state::vbl_interrupt)
{
	int scanline = param;

	/* kludge to make plgirls boot */
	if (m_maincpu->state_int(Z80_IM) != 2)
		return;

	// What is really generating interrupts 0 and 1 is still to be found

	if (scanline == 120 && (m_irq_enable & 1))
	{
		m_last_irq_level = 0;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
	else if (scanline == 0 && (m_irq_enable & 2))
	{
		m_last_irq_level = 1;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
	else if (scanline == 240 && (m_irq_enable & 4))
	{
		m_last_irq_level = 2;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

WRITE8_MEMBER(taitol_state::irq_adr_w)
{
	//logerror("irq_adr_table[%d] = %02x\n", offset, data);
	m_irq_adr_table[offset] = data;
}

READ8_MEMBER(taitol_state::irq_adr_r)
{
	return m_irq_adr_table[offset];
}

WRITE8_MEMBER(taitol_state::irq_enable_w)
{
	//logerror("irq_enable = %02x\n",data);
	m_irq_enable = data;

	// fix Plotting test mode
	if ((m_irq_enable & (1 << m_last_irq_level)) == 0)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

READ8_MEMBER(taitol_state::irq_enable_r)
{
	return m_irq_enable;
}


WRITE8_MEMBER(taitol_state::rombankswitch_w)
{
	if (m_cur_rombank != data)
	{
		if (data > m_high)
		{
			m_high = data;
			logerror("New rom size : %x\n", (m_high + 1) * 0x2000);
		}

		//logerror("robs %d, %02x (%04x)\n", offset, data, space.device().safe_pc());
		m_cur_rombank = data;
		membank("bank1")->set_base(memregion("maincpu")->base() + 0x2000 * m_cur_rombank);
	}
}

WRITE8_MEMBER(taitol_state::rombank2switch_w)
{
	data &= 0xf;

	if (m_cur_rombank2 != data)
	{
		if (data > m_high2)
		{
			m_high2 = data;
			logerror("New rom2 size : %x\n", (m_high2 + 1) * 0x4000);
		}

		//logerror("robs2 %02x (%04x)\n", data, space.device().safe_pc());

		m_cur_rombank2 = data;
		membank("bank6")->set_base(memregion("slave")->base() + 0x4000 * m_cur_rombank2);
	}
}

READ8_MEMBER(taitol_state::rombankswitch_r)
{
	return m_cur_rombank;
}

READ8_MEMBER(taitol_state::rombank2switch_r)
{
	return m_cur_rombank2;
}

WRITE8_MEMBER(taitol_state::rambankswitch_w)
{
	if (m_cur_rambank[offset] != data)
	{
		m_cur_rambank[offset] = data;
//logerror("rabs %d, %02x (%04x)\n", offset, data, space.device().safe_pc());
		if (data >= 0x14 && data <= 0x1f)
		{
			data -= 0x14;
			m_current_notifier[offset] = rambank_modify_notifiers[data].notifier;
			m_current_base[offset] = m_rambanks + rambank_modify_notifiers[data].offset;
		}
		else if (data == 0x80)
		{
			m_current_notifier[offset] = &taitol_state::palette_notifier;
			m_current_base[offset] = m_palette_ram;
		}
		else
		{
			logerror("unknown rambankswitch %d, %02x (%04x)\n", offset, data, space.device().safe_pc());
			m_current_notifier[offset] = nullptr;
			m_current_base[offset] = m_empty_ram;
		}
		membank(bankname[offset])->set_base(m_current_base[offset]);
	}
}

READ8_MEMBER(taitol_state::rambankswitch_r)
{
	return m_cur_rambank[offset];
}

void taitol_state::bank_w(address_space &space, offs_t offset, UINT8 data, int banknum )
{
	if (m_current_base[banknum][offset] != data)
	{
		m_current_base[banknum][offset] = data;
		if (m_current_notifier[banknum])
			(this->*m_current_notifier[banknum])(offset);
	}
}

WRITE8_MEMBER(taitol_state::bank0_w)
{
	bank_w(space, offset, data, 0);
}

WRITE8_MEMBER(taitol_state::bank1_w)
{
	bank_w(space, offset, data, 1);
}

WRITE8_MEMBER(taitol_state::bank2_w)
{
	bank_w(space, offset, data, 2);
}

WRITE8_MEMBER(taitol_state::bank3_w)
{
	bank_w(space, offset, data, 3);
}

WRITE8_MEMBER(taitol_state::control2_w)
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x08);
}

READ8_MEMBER(taitol_state::portA_r)
{
	return ioport((m_extport == 0) ? m_porte0_tag : m_porte1_tag)->read();
}

READ8_MEMBER(taitol_state::portB_r)
{
	return ioport((m_extport == 0) ? m_portf0_tag : m_portf1_tag)->read();
}

READ8_MEMBER(taitol_state::extport_select_and_ym2203_r)
{
	ym2203_device *ym2203 = machine().device<ym2203_device>("ymsnd");
	m_extport = (offset >> 1) & 1;
	return ym2203->read(space, offset & 1);
}

WRITE8_MEMBER(taitol_state::mcu_data_w)
{
	m_last_data = data;
	m_last_data_adr = space.device().safe_pc();
//  logerror("mcu write %02x (%04x)\n", data, space.device().safe_pc());
	switch (data)
	{
	case 0x43:
		m_mcu_pos = 0;
		m_mcu_reply_len = ARRAY_LENGTH(puzznic_mcu_reply);
		break;
	}
}

WRITE8_MEMBER(taitol_state::mcu_control_w)
{
//  logerror("mcu control %02x (%04x)\n", data, space.device().safe_pc());
}

READ8_MEMBER(taitol_state::mcu_data_r)
{
//  logerror("mcu read (%04x) [%02x, %04x]\n", space.device().safe_pc(), last_data, last_data_adr);
	if (m_mcu_pos == m_mcu_reply_len)
		return 0;

	return m_mcu_reply[m_mcu_pos++];
}

READ8_MEMBER(taitol_state::mcu_control_r)
{
//  logerror("mcu control read (%04x)\n", space.device().safe_pc());
	return 0x1;
}

#if 0
WRITE8_MEMBER(taitol_state::sound_w)
{
	logerror("Sound_w %02x (%04x)\n", data, space.device().safe_pc());
}
#endif

READ8_MEMBER(taitol_state::mux_r)
{
	switch (m_mux_ctrl)
	{
	case 0:
		return ioport("DSWA")->read();
	case 1:
		return ioport("DSWB")->read();
	case 2:
		return ioport("IN0")->read();
	case 3:
		return ioport("IN1")->read();
	case 7:
		return ioport("IN2")->read();
	default:
		logerror("Mux read from unknown port %d (%04x)\n", m_mux_ctrl, space.device().safe_pc());
		return 0xff;
	}
}

WRITE8_MEMBER(taitol_state::mux_w)
{
	switch (m_mux_ctrl)
	{
	case 4:
		control2_w(space, 0, data);
		break;
	default:
		logerror("Mux write to unknown port %d, %02x (%04x)\n", m_mux_ctrl, data, space.device().safe_pc());
	}
}

WRITE8_MEMBER(taitol_state::mux_ctrl_w)
{
	m_mux_ctrl = data;
}


WRITE_LINE_MEMBER(taitol_state::champwr_msm5205_vck)
{
	if (m_adpcm_data != -1)
	{
		m_msm->data_w(m_adpcm_data & 0x0f);
		m_adpcm_data = -1;
	}
	else
	{
		m_adpcm_data = memregion("adpcm")->base()[m_adpcm_pos];
		m_adpcm_pos = (m_adpcm_pos + 1) & 0x1ffff;
		m_msm->data_w(m_adpcm_data >> 4);
	}
}

WRITE8_MEMBER(taitol_state::champwr_msm5205_lo_w)
{
	m_adpcm_pos = (m_adpcm_pos & 0xff00ff) | (data << 8);
}

WRITE8_MEMBER(taitol_state::champwr_msm5205_hi_w)
{
	m_adpcm_pos = ((m_adpcm_pos & 0x00ffff) | (data << 16)) & 0x1ffff;
}

WRITE8_MEMBER(taitol_state::champwr_msm5205_start_w)
{
	m_msm->reset_w(0);
}

WRITE8_MEMBER(taitol_state::champwr_msm5205_stop_w)
{
	m_msm->reset_w(1);
	m_adpcm_pos &= 0x1ff00;
}

WRITE8_MEMBER(taitol_state::champwr_msm5205_volume_w)
{
	m_msm->set_output_gain(0, data / 255.0);
}

READ8_MEMBER(taitol_state::horshoes_tracky_reset_r)
{
	/* reset the trackball counter */
	m_tracky = ioport("AN0")->read();
	return 0;
}

READ8_MEMBER(taitol_state::horshoes_trackx_reset_r)
{
	/* reset the trackball counter */
	m_trackx = ioport("AN1")->read();
	return 0;
}

READ8_MEMBER(taitol_state::horshoes_tracky_lo_r)
{
	return (ioport("AN0")->read() - m_tracky) & 0xff;
}

READ8_MEMBER(taitol_state::horshoes_tracky_hi_r)
{
	return (ioport("AN0")->read() - m_tracky) >> 8;
}

READ8_MEMBER(taitol_state::horshoes_trackx_lo_r)
{
	return (ioport("AN1")->read() - m_trackx) & 0xff;
}

READ8_MEMBER(taitol_state::horshoes_trackx_hi_r)
{
	return (ioport("AN1")->read() - m_trackx) >> 8;
}


#define COMMON_BANKS_MAP \
	AM_RANGE(0x0000, 0x5fff) AM_ROM         \
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank1")            \
	AM_RANGE(0xc000, 0xcfff) AM_ROMBANK("bank2") AM_WRITE(bank0_w) \
	AM_RANGE(0xd000, 0xdfff) AM_ROMBANK("bank3") AM_WRITE(bank1_w) \
	AM_RANGE(0xe000, 0xefff) AM_ROMBANK("bank4") AM_WRITE(bank2_w) \
	AM_RANGE(0xf000, 0xfdff) AM_ROMBANK("bank5") AM_WRITE(bank3_w) \
	AM_RANGE(0xfe00, 0xfe03) AM_READWRITE(taitol_bankc_r, taitol_bankc_w)       \
	AM_RANGE(0xfe04, 0xfe04) AM_READWRITE(taitol_control_r, taitol_control_w)   \
	AM_RANGE(0xff00, 0xff02) AM_READWRITE(irq_adr_r, irq_adr_w)         \
	AM_RANGE(0xff03, 0xff03) AM_READWRITE(irq_enable_r, irq_enable_w)       \
	AM_RANGE(0xff04, 0xff07) AM_READWRITE(rambankswitch_r, rambankswitch_w) \
	AM_RANGE(0xff08, 0xff08) AM_READWRITE(rombankswitch_r, rombankswitch_w)

#define COMMON_SINGLE_MAP \
	AM_RANGE(0xa000, 0xa003) AM_READ(extport_select_and_ym2203_r) AM_DEVWRITE("ymsnd", ym2203_device, write) \
	AM_RANGE(0x8000, 0x9fff) AM_RAM



static ADDRESS_MAP_START( fhawk_map, AS_PROGRAM, 8, taitol_state )
	COMMON_BANKS_MAP
	AM_RANGE(0x8000, 0x9fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xa000, 0xbfff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( fhawk_2_map, AS_PROGRAM, 8, taitol_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank6")
	AM_RANGE(0xc000, 0xc000) AM_WRITE(rombank2switch_w)
	AM_RANGE(0xc800, 0xc800) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_device, master_port_w)
	AM_RANGE(0xc801, 0xc801) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w)
	AM_RANGE(0xd000, 0xd000) AM_READ_PORT("DSWA") AM_WRITENOP   // Direct copy of input port 0
	AM_RANGE(0xd001, 0xd001) AM_READ_PORT("DSWB")
	AM_RANGE(0xd002, 0xd002) AM_READ_PORT("IN0")
	AM_RANGE(0xd003, 0xd003) AM_READ_PORT("IN1")
	AM_RANGE(0xd004, 0xd004) AM_WRITE(control2_w)
	AM_RANGE(0xd005, 0xd006) AM_WRITENOP    // Always 0
	AM_RANGE(0xd007, 0xd007) AM_READ_PORT("IN2")
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("share1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( fhawk_3_map, AS_PROGRAM, 8, taitol_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank7")
	AM_RANGE(0x8000, 0x9fff) AM_RAM
	AM_RANGE(0xe000, 0xe000) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_device, slave_port_w)
	AM_RANGE(0xe001, 0xe001) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, slave_comm_r, slave_comm_w)
	AM_RANGE(0xf000, 0xf001) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
ADDRESS_MAP_END


static ADDRESS_MAP_START( raimais_map, AS_PROGRAM, 8, taitol_state )
	COMMON_BANKS_MAP
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x8800, 0x8800) AM_READWRITE(mux_r, mux_w)
	AM_RANGE(0x8801, 0x8801) AM_WRITE(mux_ctrl_w) AM_READNOP    // Watchdog or interrupt ack (value ignored)
	AM_RANGE(0x8c00, 0x8c00) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_device, master_port_w)
	AM_RANGE(0x8c01, 0x8c01) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w)
	AM_RANGE(0xa000, 0xbfff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( raimais_2_map, AS_PROGRAM, 8, taitol_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_SHARE("share1")
ADDRESS_MAP_END


WRITE8_MEMBER(taitol_state::sound_bankswitch_w)
{
	UINT8 *RAM = memregion("audiocpu")->base();
	int banknum = data & 0x03;

	membank ("bank7")->set_base (&RAM [(banknum * 0x4000)]);
}

static ADDRESS_MAP_START( raimais_3_map, AS_PROGRAM, 8, taitol_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank7")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE("ymsnd", ym2610_device, read, write)
	AM_RANGE(0xe200, 0xe200) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_device, slave_port_w)
	AM_RANGE(0xe201, 0xe201) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, slave_comm_r, slave_comm_w)
	AM_RANGE(0xe400, 0xe403) AM_WRITENOP /* pan */
	AM_RANGE(0xe600, 0xe600) AM_WRITENOP /* ? */
	AM_RANGE(0xee00, 0xee00) AM_WRITENOP /* ? */
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP /* ? */
	AM_RANGE(0xf200, 0xf200) AM_WRITE(sound_bankswitch_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( champwr_map, AS_PROGRAM, 8, taitol_state )
	COMMON_BANKS_MAP
	AM_RANGE(0x8000, 0x9fff) AM_RAM
	AM_RANGE(0xa000, 0xbfff) AM_RAM AM_SHARE("share1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( champwr_2_map, AS_PROGRAM, 8, taitol_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank6")
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xe000, 0xe000) AM_READ_PORT("DSWA") AM_WRITENOP   // Watchdog
	AM_RANGE(0xe001, 0xe001) AM_READ_PORT("DSWB")
	AM_RANGE(0xe002, 0xe002) AM_READ_PORT("IN0")
	AM_RANGE(0xe003, 0xe003) AM_READ_PORT("IN1")
	AM_RANGE(0xe004, 0xe004) AM_WRITE(control2_w)
	AM_RANGE(0xe007, 0xe007) AM_READ_PORT("IN2")
	AM_RANGE(0xe008, 0xe00f) AM_READNOP
	AM_RANGE(0xe800, 0xe800) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_device, master_port_w)
	AM_RANGE(0xe801, 0xe801) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w)
	AM_RANGE(0xf000, 0xf000) AM_READWRITE(rombank2switch_r, rombank2switch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( champwr_3_map, AS_PROGRAM, 8, taitol_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank7")
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9001) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_device, slave_port_w)
	AM_RANGE(0xa001, 0xa001) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, slave_comm_r, slave_comm_w)
	AM_RANGE(0xb000, 0xb000) AM_WRITE(champwr_msm5205_hi_w)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(champwr_msm5205_lo_w)
	AM_RANGE(0xd000, 0xd000) AM_WRITE(champwr_msm5205_start_w)
	AM_RANGE(0xe000, 0xe000) AM_WRITE(champwr_msm5205_stop_w)
ADDRESS_MAP_END



static ADDRESS_MAP_START( kurikint_map, AS_PROGRAM, 8, taitol_state )
	COMMON_BANKS_MAP
	AM_RANGE(0x8000, 0x9fff) AM_RAM
	AM_RANGE(0xa000, 0xa7ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xa800, 0xa800) AM_READWRITE(mux_r, mux_w)
	AM_RANGE(0xa801, 0xa801) AM_WRITE(mux_ctrl_w) AM_READNOP    // Watchdog or interrupt ack (value ignored)
ADDRESS_MAP_END

static ADDRESS_MAP_START( kurikint_2_map, AS_PROGRAM, 8, taitol_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xe800, 0xe801) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
#if 0
	AM_RANGE(0xc000, 0xc000) AM_WRITE(rombank2switch_w)
	AM_RANGE(0xd000, 0xd000) AM_READ_PORT("DSWA")
	AM_RANGE(0xd001, 0xd001) AM_READ_PORT("DSWB")
	AM_RANGE(0xd002, 0xd002) AM_READ_PORT("IN0")
	AM_RANGE(0xd003, 0xd003) AM_READ_PORT("IN1")
	AM_RANGE(0xd007, 0xd007) AM_READ_PORT("IN2")
#endif
ADDRESS_MAP_END



static ADDRESS_MAP_START( puzznic_map, AS_PROGRAM, 8, taitol_state )
	COMMON_BANKS_MAP
	COMMON_SINGLE_MAP
	AM_RANGE(0xa800, 0xa800) AM_READNOP // Watchdog
	AM_RANGE(0xb000, 0xb7ff) AM_RAM     // Wrong, used to overcome protection
	AM_RANGE(0xb800, 0xb800) AM_READWRITE(mcu_data_r, mcu_data_w)
	AM_RANGE(0xb801, 0xb801) AM_READWRITE(mcu_control_r, mcu_control_w)
	AM_RANGE(0xbc00, 0xbc00) AM_WRITENOP    // Control register, function unknown
ADDRESS_MAP_END

/* bootleg, doesn't have the MCU */
static ADDRESS_MAP_START( puzznici_map, AS_PROGRAM, 8, taitol_state )
	COMMON_BANKS_MAP
	COMMON_SINGLE_MAP
	AM_RANGE(0xa800, 0xa800) AM_READNOP // Watchdog
	AM_RANGE(0xb000, 0xb7ff) AM_RAM     // Wrong, used to overcome protection
//  AM_RANGE(0xb800, 0xb800) AM_READWRITE(mcu_data_r, mcu_data_w)
	AM_RANGE(0xb801, 0xb801) AM_READ(mcu_control_r)
//  AM_RANGE(0xb801, 0xb801) AM_WRITE(mcu_control_w)
	AM_RANGE(0xbc00, 0xbc00) AM_WRITENOP    // Control register, function unknown
ADDRESS_MAP_END


static ADDRESS_MAP_START( plotting_map, AS_PROGRAM, 8, taitol_state )
	COMMON_BANKS_MAP
	COMMON_SINGLE_MAP
	AM_RANGE(0xa800, 0xa800) AM_WRITENOP    // Watchdog or interrupt ack
	AM_RANGE(0xb800, 0xb800) AM_WRITENOP    // Control register, function unknown
ADDRESS_MAP_END


static ADDRESS_MAP_START( palamed_map, AS_PROGRAM, 8, taitol_state )
	COMMON_BANKS_MAP
	COMMON_SINGLE_MAP
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("IN0")
	AM_RANGE(0xa801, 0xa801) AM_READ_PORT("IN1")
	AM_RANGE(0xa802, 0xa802) AM_READ_PORT("IN2")
	AM_RANGE(0xa803, 0xa803) AM_WRITENOP    // Control register, function unknown
	AM_RANGE(0xb000, 0xb000) AM_WRITENOP    // Control register, function unknown (copy of 8822)
	AM_RANGE(0xb001, 0xb001) AM_READNOP // Watchdog or interrupt ack
ADDRESS_MAP_END


static ADDRESS_MAP_START( cachat_map, AS_PROGRAM, 8, taitol_state )
	COMMON_BANKS_MAP
	COMMON_SINGLE_MAP
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("IN0")
	AM_RANGE(0xa801, 0xa801) AM_READ_PORT("IN1")
	AM_RANGE(0xa802, 0xa802) AM_READ_PORT("IN2")
	AM_RANGE(0xa803, 0xa803) AM_WRITENOP    // Control register, function unknown
	AM_RANGE(0xb000, 0xb000) AM_WRITENOP    // Control register, function unknown
	AM_RANGE(0xb001, 0xb001) AM_READNOP // Watchdog or interrupt ack (value ignored)
	AM_RANGE(0xfff8, 0xfff8) AM_READWRITE(rombankswitch_r, rombankswitch_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( horshoes_map, AS_PROGRAM, 8, taitol_state )
	COMMON_BANKS_MAP
	COMMON_SINGLE_MAP
	AM_RANGE(0xa800, 0xa800) AM_READ(horshoes_tracky_lo_r)
	AM_RANGE(0xa802, 0xa802) AM_READ(horshoes_tracky_reset_r)
	AM_RANGE(0xa803, 0xa803) AM_READ(horshoes_trackx_reset_r)
	AM_RANGE(0xa804, 0xa804) AM_READ(horshoes_tracky_hi_r)
	AM_RANGE(0xa808, 0xa808) AM_READ(horshoes_trackx_lo_r)
	AM_RANGE(0xa80c, 0xa80c) AM_READ(horshoes_trackx_hi_r)
	AM_RANGE(0xb801, 0xb801) AM_READNOP // Watchdog or interrupt ack
	AM_RANGE(0xb802, 0xb802) AM_WRITE(horshoes_bankg_w)
	AM_RANGE(0xbc00, 0xbc00) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( evilston_map, AS_PROGRAM, 8, taitol_state )
	COMMON_BANKS_MAP
	AM_RANGE(0x8000, 0x9fff) AM_RAM
	AM_RANGE(0xa000, 0xa7ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("DSWA") AM_WRITENOP   //watchdog ?
	AM_RANGE(0xa801, 0xa801) AM_READ_PORT("DSWB")
	AM_RANGE(0xa802, 0xa802) AM_READ_PORT("IN0")
	AM_RANGE(0xa803, 0xa803) AM_READ_PORT("IN1")
	AM_RANGE(0xa804, 0xa804) AM_WRITENOP    //coin couters/locks ?
	AM_RANGE(0xa807, 0xa807) AM_READ_PORT("IN2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( evilston_2_map, AS_PROGRAM, 8, taitol_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xe800, 0xe801) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0xf000, 0xf7ff) AM_ROMBANK("bank7")
ADDRESS_MAP_END



/***********************************************************
             INPUT PORTS, DIPs
***********************************************************/

#define TAITO_L_SYSTEM_INPUT( type, impulse ) \
	PORT_START("IN2")\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT ) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) \
	PORT_BIT( 0x04, type, IPT_COIN1 ) PORT_IMPULSE(impulse) \
	PORT_BIT( 0x08, type, IPT_COIN2 ) PORT_IMPULSE(impulse) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

static INPUT_PORTS_START( fhawk )
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )        /* Listed as Unused */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )        /* Listed as Unused */
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as Unused */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as Unused */

	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS( 2 )

	TAITO_L_SYSTEM_INPUT( IP_ACTIVE_LOW, 4 )
INPUT_PORTS_END

static INPUT_PORTS_START( fhawkj )
	PORT_INCLUDE( fhawk )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( raimais )
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "80k and 160k" )
	PORT_DIPSETTING(    0x0c, "80k only" )
	PORT_DIPSETTING(    0x04, "160k only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS( 1 )


	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS( 2 )

	TAITO_L_SYSTEM_INPUT( IP_ACTIVE_HIGH, 1 )
INPUT_PORTS_END

static INPUT_PORTS_START( raimaisj )
	PORT_INCLUDE( raimais )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( champwr )
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)  // all 2 in manual
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, "Time" )          PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "2 minutes" )
	PORT_DIPSETTING(    0x0c, "3 minutes" )
	PORT_DIPSETTING(    0x04, "4 minutes" )
	PORT_DIPSETTING(    0x00, "5 minutes" )
	PORT_DIPNAME( 0x30, 0x30, "'1 minute' Lasts:" )     PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "30 sec" )
	PORT_DIPSETTING(    0x10, "40 sec" )
	PORT_DIPSETTING(    0x30, "50 sec" )
	PORT_DIPSETTING(    0x20, "60 sec" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as Unused */

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( champwrj )
	PORT_INCLUDE( champwr )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( champwru )
	PORT_INCLUDE( champwr )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( kurikint )
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )        /* Listed as Unused */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )        /* Listed as Unused */
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        /* Listed as Unused */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as Unused */
	PORT_DIPNAME( 0x40, 0x40, "Bosses' messages" )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "5 Times" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS( 2 )

	TAITO_L_SYSTEM_INPUT( IP_ACTIVE_HIGH, 4 )
INPUT_PORTS_END

static INPUT_PORTS_START( kurikintj )
	PORT_INCLUDE( kurikint )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( kurikinta )
	PORT_INCLUDE( kurikint )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2") /* Oposite of most Taito settings. IE: Off "means" off */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "Level Select (Cheat)")   PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Bosses' messages" )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Slow Motion (Cheat)")    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( puzznic )
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SW1)
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )    /* There is no Coin B in the Manual */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )    /* There is no Coin B in the Manual */

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)           /* Difficulty controls the Timer Speed (how many seconds are there in a minute) */
	PORT_DIPNAME( 0x0c, 0x0c, "Retries" )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPNAME( 0x10, 0x10, "Bombs" )     PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "0" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x20, 0x20, "Girls" )     PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Terms of Replay" )   PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x40, "Stage one step back/Timer continuous" )
	PORT_DIPSETTING(    0xc0, "Stage reset to start/Timer continuous" )
	PORT_DIPSETTING(    0x80, "Stage reset to start/Timer reset to start" )
//  PORT_DIPSETTING(    0x00, "No Use" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("IN2") /* Not read yet. There is no Coin_B in manual */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( plotting )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "Play Mode" )         PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "1 Player" )
	PORT_DIPSETTING(    0x01, "2 Player" )
	TAITO_DSWA_BITS_1_TO_3_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )        /* Listed as Unused and "Must Remain Off" */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )        /* Listed as Unused and "Must Remain Off" */
	PORT_DIPNAME( 0x30, 0x30, "Wild Blocks" )       PORT_DIPLOCATION("SW2:5,6") /* Number of allowed misses */
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as Unused and "Must Remain Off" */

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( palamed )
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)               /* Difficulty controls how fast the dice lines fall*/
	PORT_DIPNAME( 0x0c, 0x0c, "Games for VS Victory" )  PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "1 Game" )
	PORT_DIPSETTING(    0x0c, "2 Games" )
	PORT_DIPSETTING(    0x04, "3 Games" )
	PORT_DIPSETTING(    0x00, "4 Games" )
	PORT_DIPNAME( 0x30, 0x30, "Dice Appear at" )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "500 Lines" )
	PORT_DIPSETTING(    0x30, "1000 Lines" )
	PORT_DIPSETTING(    0x10, "2000 Lines" )
	PORT_DIPSETTING(    0x00, "3000 Lines" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x80, 0x80, "Versus Mode" )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( cachat )
	PORT_START("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2") /* Oposite of most Taito settings. IE: Off "means" off */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	TAITO_DSWA_BITS_2_TO_3_LOC(SW1)
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( tubeit )
	PORT_START("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2") /* Oposite of most Taito settings. IE: Off "means" off */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	TAITO_DSWA_BITS_2_TO_3_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( horshoes )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "Beer Frame Message" )    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Break Time" )
	PORT_DIPSETTING(    0x00, "Beer Frame" )
	TAITO_DSWA_BITS_1_TO_3_LOC(SW1)
	TAITO_COINAGE_US_LOC(SW1)               /* According to the "United States Version" manual listing */

	PORT_START("DSWB")
	/* Not for sure, the CPU seems to play better when set to Hardest */
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, "Time" )          PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "20 sec" )
	PORT_DIPSETTING(    0x0c, "30 sec" )
	PORT_DIPSETTING(    0x04, "40 sec" )
	PORT_DIPSETTING(    0x00, "60 sec" )
	PORT_DIPNAME( 0x10, 0x10, "Innings" )           PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "3 per Credit" )
	PORT_DIPSETTING(    0x00, "9 per Credit" )
	PORT_DIPNAME( 0x20, 0x20, "Bonus Advantage" )       PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Scoring Speed" )     PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x80, 0x80, "Grip/Angle Select" )     PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "2 Buttons" )
	PORT_DIPSETTING(    0x00, "1 Button" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("AN0")
	PORT_BIT( 0xffff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_REVERSE

	PORT_START("AN1")
	PORT_BIT( 0xffff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30)
INPUT_PORTS_END

static INPUT_PORTS_START( plgirls )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:4,5,6") /* Manual shows same coinage as Play Girls 2 */
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
//  PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)               /* Difficulty controls the Ball Speed */
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x80, "P1+P2 Start to Clear Round (Cheat)" )    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( plgirls2 )
	PORT_START("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW1:1" )        /* Listed as Not Used */
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x08, "Coin Mode" )         PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "Mode A" )
	PORT_DIPSETTING(    0x00, "Mode B" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSWA", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSWA", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) ) PORT_CONDITION("DSWA", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSWA", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) ) PORT_CONDITION("DSWA", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) ) PORT_CONDITION("DSWA", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSWA", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) ) PORT_CONDITION("DSWA", 0x08, EQUALS, 0x00)
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSWA", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSWA", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) ) PORT_CONDITION("DSWA", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSWA", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) ) PORT_CONDITION("DSWA", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) ) PORT_CONDITION("DSWA", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSWA", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) ) PORT_CONDITION("DSWA", 0x08, EQUALS, 0x00)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)               /* Difficulty controls the number of hits requiered to destroy enemies */
	PORT_DIPNAME( 0x04, 0x04, "Time" )          PORT_DIPLOCATION("SW2:3") /* Simply listed as "Time", what exactly does it refer to? */
	PORT_DIPSETTING(    0x04, "2 Seconds" )
	PORT_DIPSETTING(    0x00, "3 Seconds" )
	PORT_DIPNAME( 0x18, 0x18, "Lives for Joe/Lady/Jack" )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x10, "3/2/3" )
	PORT_DIPSETTING(    0x18, "4/3/4" )
	PORT_DIPSETTING(    0x08, "5/4/5" )
	PORT_DIPSETTING(    0x00, "6/5/6" )
	PORT_DIPNAME( 0x20, 0x20, "Character Speed" )       PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as Not Used */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as Not Used */

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( cubybop )
	PORT_START("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( evilston )
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x80, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( English ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Japanese ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2  )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(4)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
INPUT_PORTS_END




static const gfx_layout bg2_layout =
{
	8, 8,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*8*4
};


#define O 8*8*4
#define O2 2*O
static const gfx_layout sp2_layout =
{
	16, 16,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16, O+3, O+2, O+1, O+0, O+19, O+18, O+17, O+16 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, O2+0*32, O2+1*32, O2+2*32, O2+3*32, O2+4*32, O2+5*32, O2+6*32, O2+7*32 },
	8*8*4*4
};
#undef O
#undef O2

static const gfx_layout char_layout =
{
	8, 8,
	1024,
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*8*4
};



static GFXDECODE_START( taito_l )
	GFXDECODE_ENTRY( "gfx1", 0, bg2_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, sp2_layout, 0, 16 )
	GFXDECODE_ENTRY( nullptr,           0, char_layout,  0, 16 )  // Ram-based
GFXDECODE_END



WRITE_LINE_MEMBER(taitol_state::irqhandler)
{
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER(taitol_state::portA_w)
{
	if (m_cur_bank != (data & 0x03))
	{
		int bankaddress;
		UINT8 *RAM = memregion("audiocpu")->base();

		m_cur_bank = data & 0x03;
		bankaddress = m_cur_bank * 0x4000;
		membank("bank7")->set_base(&RAM[bankaddress]);
		//logerror ("YM2203 bank change val=%02x  pc=%04x\n", m_cur_bank, space.device().safe_pc() );
	}
}


static MACHINE_CONFIG_START( fhawk, taitol_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_13_33056MHz/2)    /* verified freq on pin122 of TC0090LVC cpu */
	MCFG_CPU_PROGRAM_MAP(fhawk_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(taitol_state,irq_callback)

	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", taitol_state, vbl_interrupt, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_12MHz/3)     /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(fhawk_3_map)

	MCFG_CPU_ADD("slave", Z80, XTAL_12MHz/3)        /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(fhawk_2_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", taitol_state, irq0_line_hold)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_MACHINE_START_OVERRIDE(taitol_state,taito_l)
	MCFG_MACHINE_RESET_OVERRIDE(taitol_state,fhawk)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(taitol_state, screen_update_taitol)
	MCFG_SCREEN_VBLANK_DRIVER(taitol_state, screen_eof_taitol)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", taito_l)
	MCFG_PALETTE_ADD("palette", 256)

	MCFG_VIDEO_START_OVERRIDE(taitol_state,taitol)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_12MHz/4)       /* verified on pcb */
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(taitol_state,irqhandler))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(taitol_state, portA_w))
	MCFG_SOUND_ROUTE(0, "mono", 0.20)
	MCFG_SOUND_ROUTE(1, "mono", 0.20)
	MCFG_SOUND_ROUTE(2, "mono", 0.20)
	MCFG_SOUND_ROUTE(3, "mono", 0.80)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("slave")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( champwr, fhawk )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(champwr_map)

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(champwr_3_map)

	MCFG_CPU_MODIFY("slave")
	MCFG_CPU_PROGRAM_MAP(champwr_2_map)

	MCFG_MACHINE_RESET_OVERRIDE(taitol_state,champwr)

	/* sound hardware */
	MCFG_SOUND_MODIFY("ymsnd")
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(taitol_state,irqhandler))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(taitol_state, portA_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(taitol_state, champwr_msm5205_volume_w))

	MCFG_SOUND_ADD("msm", MSM5205, XTAL_384kHz)
	MCFG_MSM5205_VCLK_CB(WRITELINE(taitol_state, champwr_msm5205_vck)) /* VCK function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8 kHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END



static MACHINE_CONFIG_DERIVED( raimais, fhawk )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(raimais_map)

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(raimais_3_map)

	MCFG_CPU_MODIFY("slave")
	MCFG_CPU_PROGRAM_MAP(raimais_2_map)

	MCFG_MACHINE_RESET_OVERRIDE(taitol_state,raimais)

	/* sound hardware */
	MCFG_SOUND_REPLACE("ymsnd", YM2610, XTAL_8MHz)      /* verified on pcb (8Mhz OSC is also for the 2nd z80) */
	MCFG_YM2610_IRQ_HANDLER(WRITELINE(taitol_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)
	MCFG_SOUND_ROUTE(2, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( kurikint, taitol_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_13_33056MHz/2)    /* verified freq on pin122 of TC0090LVC cpu */
	MCFG_CPU_PROGRAM_MAP(kurikint_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(taitol_state,irq_callback)

	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", taitol_state, vbl_interrupt, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_12MHz/3)        /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(kurikint_2_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", taitol_state, irq0_line_hold)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_MACHINE_START_OVERRIDE(taitol_state,taito_l)
	MCFG_MACHINE_RESET_OVERRIDE(taitol_state,kurikint)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(taitol_state, screen_update_taitol)
	MCFG_SCREEN_VBLANK_DRIVER(taitol_state, screen_eof_taitol)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", taito_l)
	MCFG_PALETTE_ADD("palette", 256)

	MCFG_VIDEO_START_OVERRIDE(taitol_state,taitol)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_12MHz/4)       /* verified on pcb */
	MCFG_SOUND_ROUTE(0, "mono", 0.20)
	MCFG_SOUND_ROUTE(1, "mono", 0.20)
	MCFG_SOUND_ROUTE(2, "mono", 0.20)
	MCFG_SOUND_ROUTE(3, "mono", 0.80)
MACHINE_CONFIG_END





static MACHINE_CONFIG_START( plotting, taitol_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_13_33056MHz/2)    /* verified freq on pin122 of TC0090LVC cpu */
	MCFG_CPU_PROGRAM_MAP(plotting_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(taitol_state,irq_callback)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", taitol_state, vbl_interrupt, "screen", 0, 1)

	MCFG_MACHINE_START_OVERRIDE(taitol_state,taito_l)
	MCFG_MACHINE_RESET_OVERRIDE(taitol_state,plotting)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(taitol_state, screen_update_taitol)
	MCFG_SCREEN_VBLANK_DRIVER(taitol_state, screen_eof_taitol)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", taito_l)
	MCFG_PALETTE_ADD("palette", 256)

	MCFG_VIDEO_START_OVERRIDE(taitol_state,taitol)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_13_33056MHz/4) /* verified on pcb */
	MCFG_AY8910_PORT_A_READ_CB(READ8(taitol_state, portA_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(taitol_state, portB_r))
	MCFG_SOUND_ROUTE(0, "mono", 0.20)
	MCFG_SOUND_ROUTE(1, "mono", 0.20)
	MCFG_SOUND_ROUTE(2, "mono", 0.20)
	MCFG_SOUND_ROUTE(3, "mono", 0.80)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( puzznic, plotting )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(puzznic_map)

	MCFG_MACHINE_RESET_OVERRIDE(taitol_state,puzznic)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( puzznici, plotting )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(puzznici_map)

	MCFG_MACHINE_RESET_OVERRIDE(taitol_state,puzznic)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( horshoes, plotting )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(horshoes_map)

	MCFG_MACHINE_RESET_OVERRIDE(taitol_state,horshoes)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( palamed, plotting )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(palamed_map)

	MCFG_MACHINE_RESET_OVERRIDE(taitol_state,palamed)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cachat, plotting )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(cachat_map)

	MCFG_MACHINE_RESET_OVERRIDE(taitol_state,cachat)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( evilston, taitol_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_13_33056MHz/2)    /* not verified */
	MCFG_CPU_PROGRAM_MAP(evilston_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(taitol_state,irq_callback)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", taitol_state, vbl_interrupt, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_12MHz/3)     /* not verified */
	MCFG_CPU_PROGRAM_MAP(evilston_2_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", taitol_state, irq0_line_hold)
	MCFG_CPU_PERIODIC_INT_DRIVER(taitol_state, nmi_line_pulse, 60)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_MACHINE_START_OVERRIDE(taitol_state,taito_l)
	MCFG_MACHINE_RESET_OVERRIDE(taitol_state,evilston)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(taitol_state, screen_update_taitol)
	MCFG_SCREEN_VBLANK_DRIVER(taitol_state, screen_eof_taitol)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", taito_l)
	MCFG_PALETTE_ADD("palette", 256)

	MCFG_VIDEO_START_OVERRIDE(taitol_state,taitol)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_12MHz/4)       /* not verified */
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 0.25)
	MCFG_SOUND_ROUTE(2, "mono", 0.25)
	MCFG_SOUND_ROUTE(3, "mono", 0.80)
MACHINE_CONFIG_END


ROM_START( raimais )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "b36-11-1.bin", 0x00000, 0x20000, CRC(f19fb0d5) SHA1(ba7187dfa5b4a08cebf236913a80066dafbbc59f) )
	ROM_LOAD( "b36-09.bin",   0x20000, 0x20000, CRC(9c466e43) SHA1(2466a3f1f8124323008c9925f90e9a1d2edf1564) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b36-06.bin",   0x00000, 0x10000, CRC(29bbc4f8) SHA1(39a68729c6180c5f6cdf604e692018e7d6bf5591) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "b36-07.bin",   0x00000, 0x10000, CRC(4f3737e6) SHA1(ff5f5d4ca5485441d03c8cb01a6a096941ab02eb) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b36-01.bin",   0x00000, 0x80000, CRC(89355cb2) SHA1(433e929fe8b488af84e88486d9679468a3d9677a) )
	ROM_LOAD( "b36-02.bin",   0x80000, 0x80000, CRC(e71da5db) SHA1(aa47ae02c359264c0a1f09ecc583eefd1ef1dfa4) )

	ROM_REGION( 0x80000, "ymsnd", 0 )
	ROM_LOAD( "b36-03.bin",   0x00000, 0x80000, CRC(96166516) SHA1(a6748218188cbd1b037f6c0845416665c0d55a7b) )
ROM_END

ROM_START( raimaisj )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "b36-08-1.bin", 0x00000, 0x20000, CRC(6cc8f79f) SHA1(17b4903f87e6d5447c8557c2baca1728f86245dc) )
	ROM_LOAD( "b36-09.bin",   0x20000, 0x20000, CRC(9c466e43) SHA1(2466a3f1f8124323008c9925f90e9a1d2edf1564) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b36-06.bin",   0x00000, 0x10000, CRC(29bbc4f8) SHA1(39a68729c6180c5f6cdf604e692018e7d6bf5591) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "b36-07.bin",   0x00000, 0x10000, CRC(4f3737e6) SHA1(ff5f5d4ca5485441d03c8cb01a6a096941ab02eb) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b36-01.bin",   0x00000, 0x80000, CRC(89355cb2) SHA1(433e929fe8b488af84e88486d9679468a3d9677a) )
	ROM_LOAD( "b36-02.bin",   0x80000, 0x80000, CRC(e71da5db) SHA1(aa47ae02c359264c0a1f09ecc583eefd1ef1dfa4) )

	ROM_REGION( 0x80000, "ymsnd", 0 )
	ROM_LOAD( "b36-03.bin",   0x00000, 0x80000, CRC(96166516) SHA1(a6748218188cbd1b037f6c0845416665c0d55a7b) )
ROM_END

ROM_START( raimaisjo )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "b36-08.bin", 0x00000, 0x20000, CRC(f40b9178) SHA1(ccf5afcf08cac0d5b2d6ba74abd62d35412f0265) )
	ROM_LOAD( "b36-09.bin", 0x20000, 0x20000, CRC(9c466e43) SHA1(2466a3f1f8124323008c9925f90e9a1d2edf1564) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b36-06.bin",   0x00000, 0x10000, CRC(29bbc4f8) SHA1(39a68729c6180c5f6cdf604e692018e7d6bf5591) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "b36-07.bin",   0x00000, 0x10000, CRC(4f3737e6) SHA1(ff5f5d4ca5485441d03c8cb01a6a096941ab02eb) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b36-01.bin",   0x00000, 0x80000, CRC(89355cb2) SHA1(433e929fe8b488af84e88486d9679468a3d9677a) )
	ROM_LOAD( "b36-02.bin",   0x80000, 0x80000, CRC(e71da5db) SHA1(aa47ae02c359264c0a1f09ecc583eefd1ef1dfa4) )

	ROM_REGION( 0x80000, "ymsnd", 0 )
	ROM_LOAD( "b36-03.bin",   0x00000, 0x80000, CRC(96166516) SHA1(a6748218188cbd1b037f6c0845416665c0d55a7b) )
ROM_END

ROM_START( fhawk )
	ROM_REGION( 0xa0000, "maincpu", 0 )
	ROM_LOAD( "b70-11.ic3", 0x00000, 0x20000, CRC(7d9f7583) SHA1(d8fa7c66a81fb356fa9c72f377bfc31b1837eafb) )
	ROM_LOAD( "b70-03.ic2", 0x20000, 0x80000, CRC(42d5a9b8) SHA1(10714fe95c372cec12376e615a9abe213aff12bc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b70-09.ic31", 0x00000, 0x10000, CRC(85cccaa2) SHA1(5459cd8df9d94e1938008cfc17d4ebac98004bfc) )

	ROM_REGION( 0x20000, "slave", 0 )
	ROM_LOAD( "b70-08.ic12", 0x00000, 0x20000, CRC(4d795f48) SHA1(58040d8ccbd0572cf6aef6ea9dd646b9338a03a0) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b70-01.ic1", 0x00000, 0x80000, CRC(fcdf67e2) SHA1(08a6a04a45c4adb4f5b4b0b83e90b2e5fe5cb0b1) )
	ROM_LOAD( "b70-02.ic2", 0x80000, 0x80000, CRC(35f7172e) SHA1(f257e9db470bb6dcca491b89cb666ef6d2546887) )
ROM_END

ROM_START( fhawkj )
	ROM_REGION( 0xa0000, "maincpu", 0 )
	ROM_LOAD( "b70-07.ic3", 0x00000, 0x20000, CRC(939114af) SHA1(66218536dcb3b34ffa01d3c9c2fee365d91cfe00) )
	ROM_LOAD( "b70-03.ic2", 0x20000, 0x80000, CRC(42d5a9b8) SHA1(10714fe95c372cec12376e615a9abe213aff12bc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b70-09.ic31", 0x00000, 0x10000, CRC(85cccaa2) SHA1(5459cd8df9d94e1938008cfc17d4ebac98004bfc) )

	ROM_REGION( 0x20000, "slave", 0 )
	ROM_LOAD( "b70-08.ic12", 0x00000, 0x20000, CRC(4d795f48) SHA1(58040d8ccbd0572cf6aef6ea9dd646b9338a03a0) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b70-01.ic1", 0x00000, 0x80000, CRC(fcdf67e2) SHA1(08a6a04a45c4adb4f5b4b0b83e90b2e5fe5cb0b1) )
	ROM_LOAD( "b70-02.ic2", 0x80000, 0x80000, CRC(35f7172e) SHA1(f257e9db470bb6dcca491b89cb666ef6d2546887) )
ROM_END

ROM_START( champwr )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "c01-13.rom", 0x00000, 0x20000, CRC(7ef47525) SHA1(79789fa3bcaeb6666c108d2e4b69a1f9341b2f4a) )
	ROM_LOAD( "c01-04.rom", 0x20000, 0x20000, CRC(358bd076) SHA1(beb20a09370d05de719dde596eadca8fecb14ce5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c01-08.rom", 0x00000, 0x10000, CRC(810efff8) SHA1(dd4fc046095e0e815e8e1fd96d258da0d6bba298) )

	ROM_REGION( 0x20000, "slave", 0 )
	ROM_LOAD( "c01-07.rom", 0x00000, 0x20000, CRC(5117c98f) SHA1(16b3a443eb113d2591833884a1b0ff297d8c00a4) )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "c01-01.rom", 0x000000, 0x80000, CRC(f302e6e9) SHA1(456b046932c1ee29c890b8e87d417c4bb508c06a) )
	ROM_LOAD( "c01-02.rom", 0x080000, 0x80000, CRC(1e0476c4) SHA1(b7922e5196990ad4382f367ec80b5c72e75f9d35) )
	ROM_LOAD( "c01-03.rom", 0x100000, 0x80000, CRC(2a142dbc) SHA1(5d0e40ec266d3abcff4237c5c609355c65b4fa33) )

	ROM_REGION( 0x20000, "adpcm", 0 )   /* ADPCM samples */
	ROM_LOAD( "c01-05.rom", 0x00000, 0x20000, CRC(22efad4a) SHA1(54fb33dfba5059dee16fa8b5a33b0b2d62a78373) )
ROM_END

ROM_START( champwru )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "c01-12.rom", 0x00000, 0x20000, CRC(09f345b3) SHA1(f3f9a7dab0b3f87b6919a7b37cb52245e112cb08) )
	ROM_LOAD( "c01-04.rom", 0x20000, 0x20000, CRC(358bd076) SHA1(beb20a09370d05de719dde596eadca8fecb14ce5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c01-08.rom", 0x00000, 0x10000, CRC(810efff8) SHA1(dd4fc046095e0e815e8e1fd96d258da0d6bba298) )

	ROM_REGION( 0x20000, "slave", 0 )
	ROM_LOAD( "c01-07.rom", 0x00000, 0x20000, CRC(5117c98f) SHA1(16b3a443eb113d2591833884a1b0ff297d8c00a4) )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "c01-01.rom", 0x000000, 0x80000, CRC(f302e6e9) SHA1(456b046932c1ee29c890b8e87d417c4bb508c06a) )
	ROM_LOAD( "c01-02.rom", 0x080000, 0x80000, CRC(1e0476c4) SHA1(b7922e5196990ad4382f367ec80b5c72e75f9d35) )
	ROM_LOAD( "c01-03.rom", 0x100000, 0x80000, CRC(2a142dbc) SHA1(5d0e40ec266d3abcff4237c5c609355c65b4fa33) )

	ROM_REGION( 0x20000, "adpcm", 0 )   /* ADPCM samples */
	ROM_LOAD( "c01-05.rom", 0x00000, 0x20000, CRC(22efad4a) SHA1(54fb33dfba5059dee16fa8b5a33b0b2d62a78373) )
ROM_END

ROM_START( champwrj )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "c01-06.bin", 0x00000, 0x20000, CRC(90fa1409) SHA1(7904488d567ce5d8705b2d2c8a4b4aae310cc28b) )
	ROM_LOAD( "c01-04.rom", 0x20000, 0x20000, CRC(358bd076) SHA1(beb20a09370d05de719dde596eadca8fecb14ce5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c01-08.rom", 0x00000, 0x10000, CRC(810efff8) SHA1(dd4fc046095e0e815e8e1fd96d258da0d6bba298) )

	ROM_REGION( 0x20000, "slave", 0 )
	ROM_LOAD( "c01-07.rom", 0x00000, 0x20000, CRC(5117c98f) SHA1(16b3a443eb113d2591833884a1b0ff297d8c00a4) )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "c01-01.rom", 0x000000, 0x80000, CRC(f302e6e9) SHA1(456b046932c1ee29c890b8e87d417c4bb508c06a) )
	ROM_LOAD( "c01-02.rom", 0x080000, 0x80000, CRC(1e0476c4) SHA1(b7922e5196990ad4382f367ec80b5c72e75f9d35) )
	ROM_LOAD( "c01-03.rom", 0x100000, 0x80000, CRC(2a142dbc) SHA1(5d0e40ec266d3abcff4237c5c609355c65b4fa33) )

	ROM_REGION( 0x20000, "adpcm", 0 )   /* ADPCM samples */
	ROM_LOAD( "c01-05.rom", 0x00000, 0x20000, CRC(22efad4a) SHA1(54fb33dfba5059dee16fa8b5a33b0b2d62a78373) )
ROM_END


ROM_START( kurikint )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "b42-09.ic2",  0x00000, 0x20000, CRC(e97c4394) SHA1(fdeb15315166f7615d4039d5dc9c28d53cee86f2) )
	ROM_LOAD( "b42-06.ic6",  0x20000, 0x20000, CRC(fa15fd65) SHA1(a810d7315878212e4e5344a24addf117ea6baeab) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b42-07.ic22", 0x00000, 0x10000, CRC(0f2719c0) SHA1(f870335a75f236f0059522f9a577dee7ca3acb2f) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b42-01.ic1",  0x00000, 0x80000, CRC(7d1a1fec) SHA1(28311b07673686c18988400d0254533a454f07f4) )
	ROM_LOAD( "b42-02.ic5",  0x80000, 0x80000, CRC(1a52e65c) SHA1(20a1fc4d02b5928fb01444079692e23d178c6297) )
ROM_END

ROM_START( kurikintu )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "b42-08.ic2",  0x00000, 0x20000, CRC(7075122e) SHA1(55f5f0cf3b91b7b408f9c05c91f9839c43b49c5f) )
	ROM_LOAD( "b42-06.ic6",  0x20000, 0x20000, CRC(fa15fd65) SHA1(a810d7315878212e4e5344a24addf117ea6baeab) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b42-07.ic22", 0x00000, 0x10000, CRC(0f2719c0) SHA1(f870335a75f236f0059522f9a577dee7ca3acb2f) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b42-01.ic1",  0x00000, 0x80000, CRC(7d1a1fec) SHA1(28311b07673686c18988400d0254533a454f07f4) )
	ROM_LOAD( "b42-02.ic5",  0x80000, 0x80000, CRC(1a52e65c) SHA1(20a1fc4d02b5928fb01444079692e23d178c6297) )
ROM_END

ROM_START( kurikintj )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "b42-05.ic2",  0x00000, 0x20000, CRC(077222b8) SHA1(953fb3444f6bb0dbe0323a0fd8fc3067b106a4f6) )
	ROM_LOAD( "b42-06.ic6",  0x20000, 0x20000, CRC(fa15fd65) SHA1(a810d7315878212e4e5344a24addf117ea6baeab) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b42-07.ic22", 0x00000, 0x10000, CRC(0f2719c0) SHA1(f870335a75f236f0059522f9a577dee7ca3acb2f) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b42-01.ic1",  0x00000, 0x80000, CRC(7d1a1fec) SHA1(28311b07673686c18988400d0254533a454f07f4) )
	ROM_LOAD( "b42-02.ic5",  0x80000, 0x80000, CRC(1a52e65c) SHA1(20a1fc4d02b5928fb01444079692e23d178c6297) )
ROM_END

ROM_START( kurikinta )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "kk_ic2.ic2",  0x00000, 0x20000, CRC(908603f2) SHA1(f810f2501458224e9264a984f22547cc8ccc2b0e) )
	ROM_LOAD( "kk_ic6.ic6",  0x20000, 0x20000, CRC(a4a957b1) SHA1(bbdb5b71ab613a8c89f7a0300abd85408951dc7e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b42-07.ic22", 0x00000, 0x10000, CRC(0f2719c0) SHA1(f870335a75f236f0059522f9a577dee7ca3acb2f) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "kk_1-1l.rom", 0x00000, 0x20000, CRC(df1d4fcd) SHA1(300cad3636ea9648595c3f4bba3ca737f95f7354) )
	ROM_LOAD16_BYTE( "kk_2-2l.rom", 0x40000, 0x20000, CRC(fca7f647) SHA1(0571e8fc2eda9f139e81d6d191368fb99764f797) )
	ROM_LOAD16_BYTE( "kk_5-3l.rom", 0x80000, 0x20000, CRC(d080fde1) SHA1(e5011cdf35bf5d39f4786e6d60d2b35a79560dfa) )
	ROM_LOAD16_BYTE( "kk_7-4l.rom", 0xc0000, 0x20000, CRC(f5bf6829) SHA1(4c1b4c6f451ed823730762f67c2e716789cddb10) )
	ROM_LOAD16_BYTE( "kk_3-1h.rom", 0x00001, 0x20000, CRC(71af848e) SHA1(1e4d050c9191a8645f324de84767662ed80165b6) )
	ROM_LOAD16_BYTE( "kk_4-2h.rom", 0x40001, 0x20000, CRC(cebb5bac) SHA1(6c1e3cdea353bd835b49b95af0bb718e2b46ecfe) )
	ROM_LOAD16_BYTE( "kk_6-3h.rom", 0x80001, 0x20000, CRC(322e3752) SHA1(7592b5dc7945c96f53aeb5c328c54c0dcba3809a) )
	ROM_LOAD16_BYTE( "kk_8-4h.rom", 0xc0001, 0x20000, CRC(117bde99) SHA1(fe0f56b6c840e35870639c4de129443e14720a7b) )
ROM_END

/************************************************************************

  Plotting / Flipull rom numbering listed:

   B96-01 - Japanese main program rom
   B96-02 -  Original graphics rom
   B96-03 -  Original graphics rom
   B96-04 - PAL 16L8BCJ
   B96-05 - US main program rom
   B96-06 - Original World main program rom
   B96-07 -  Revised graphics rom
   B96-08 -  Revised graphics rom
   B96-09 - Later World main program rom??
   B96-10 - Later World main program rom??


PCB number info:
 K1100439A FLIPULL
 K1100441A PLOTTING
 K1100466A (US Plotting PCB ID#?)

  +--------------------------+
 _|    PAL           4 4 4 4 |
|                    3 3 3 3 |
|       VOL          2 2 2 2 |
|                    5 5 5 5 |
|                    6 6 6 6 |
|                            |
|J               +---------+ |
|A          OSC  |         | |
|M  YM3014       |TC0090LVC| |
|M       MB3771  |         | |
|A               +---------+ |
|                            |
|    +-------+      B  B  B  |
|    |YM2203C|      9  9  9  |
|    +-------+      6  6  6  |
|_                  0  0  0  |
  | DSB DSA         1  7  8  |
  +--------------------------+

OSC 13.33056MHz
RAM uPD43256
PAL 16L8BCJ (labeled as B96-04)
CPU TC0090LVC (All in one Z80 & system controller??)

************************************************************************/

ROM_START( plotting ) /* Likely B96-10 or higher by Taito's rom numbering system, demo mode is 1 player */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic10",       0x00000, 0x10000, CRC(be240921) SHA1(f29f3a49b563f24aa6e3187ac4da1a8100cb02b5) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b96-07.ic9", 0x00000, 0x10000, CRC(0713a387) SHA1(0fc1242ce02a56279fa1d5270c905bba7cdcd072) )
	ROM_LOAD16_BYTE( "b96-08.ic8", 0x00001, 0x10000, CRC(55b8e294) SHA1(14405638f751adfadb022bf7a0123a3972d4a617) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8-b86-04.bin", 0x0000, 0x0117, CRC(bf8c0ea0) SHA1(e0a00f1f6363fb79650202f90a56329990876d49) )  /* derived, but verified  Pal Stamped B86-04 */
ROM_END


ROM_START( plottinga ) /* B96-09 or higher by Taito's rom numbering system, demo mode is 2 players */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "plot01.ic10", 0x00000, 0x10000, CRC(5b30bc25) SHA1(df8839a90da9e5122d75b6faaf97f59499dbd316) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b96-02.ic9", 0x00000, 0x10000, CRC(6e0bad2a) SHA1(73996688cd058a2f56f61ea60144b9c673919a58) )
	ROM_LOAD16_BYTE( "b96-03.ic8", 0x00001, 0x10000, CRC(fb5f3ca4) SHA1(0c335acceea50133a6899f9e368cff5f61b55a96) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8-b86-04.bin", 0x0000, 0x0117, CRC(bf8c0ea0) SHA1(e0a00f1f6363fb79650202f90a56329990876d49) )  /* derived, but verified  Pal Stamped B86-04 */
ROM_END

ROM_START( plottingb ) /* The first (earliest) "World" version by Taito's rom numbering system, demo mode is 2 players */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b96-06.ic10",0x00000, 0x10000, CRC(f89a54b1) SHA1(19757b5fb61acdd6f5ae8e32a38ae54bfda0c522) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b96-02.ic9", 0x00000, 0x10000, CRC(6e0bad2a) SHA1(73996688cd058a2f56f61ea60144b9c673919a58) )
	ROM_LOAD16_BYTE( "b96-03.ic8", 0x00001, 0x10000, CRC(fb5f3ca4) SHA1(0c335acceea50133a6899f9e368cff5f61b55a96) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8-b86-04.bin", 0x0000, 0x0117, CRC(bf8c0ea0) SHA1(e0a00f1f6363fb79650202f90a56329990876d49) )  /* derived, but verified  Pal Stamped B86-04 */
ROM_END

ROM_START( plottingu ) /* The demo mode is 2 players */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b96-05.ic10",0x00000, 0x10000, CRC(afb99d1f) SHA1(a5cabc182d4f1d5709e6835d8b0a481dd0f9a563) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b96-02.ic9", 0x00000, 0x10000, CRC(6e0bad2a) SHA1(73996688cd058a2f56f61ea60144b9c673919a58) )
	ROM_LOAD16_BYTE( "b96-03.ic8", 0x00001, 0x10000, CRC(fb5f3ca4) SHA1(0c335acceea50133a6899f9e368cff5f61b55a96) )

	ROM_REGION( 0x0200, "plds", 0 ) // PAL16L8
	ROM_LOAD( "b96-04.ic12", 0x0000, 0x0104, CRC(9390a782) SHA1(9e68948ed15d96c1998e5d5cd99b823676e555e7) )  /* Confirmed/Matches U.S. set */
ROM_END

ROM_START( flipull ) /* The demo mode is 1 player */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b96-01.ic10",0x00000, 0x10000, CRC(65993978) SHA1(d14dc70f1b5e72b96ccc3fab61d7740f627bfea2) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b96-07.ic9", 0x00000, 0x10000, CRC(0713a387) SHA1(0fc1242ce02a56279fa1d5270c905bba7cdcd072) )
	ROM_LOAD16_BYTE( "b96-08.ic8", 0x00001, 0x10000, CRC(55b8e294) SHA1(14405638f751adfadb022bf7a0123a3972d4a617) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8-b86-04.bin", 0x0000, 0x0117, CRC(bf8c0ea0) SHA1(e0a00f1f6363fb79650202f90a56329990876d49) )  /* derived, but verified  Pal Stamped B86-04 */
ROM_END

ROM_START( puzznic )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "c20-09.ic11", 0x00000, 0x20000, CRC(156d6de1) SHA1(c247936b62ef354851c9bace76a7a0aa14194d5f) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "mc68705p3.ic4", 0x0000, 0x0800, CRC(085f68b4) SHA1(2dbc7e2c015220dc59ee1f1208540744e5b9b7cc) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "c20-07.ic10", 0x00000, 0x10000, CRC(be12749a) SHA1(c67d1a434486843a6776d89e905362b7db595d8d) )
	ROM_LOAD16_BYTE( "c20-06.ic9",  0x00001, 0x10000, CRC(ac85a9c5) SHA1(2d72dae86a191ccdac9648980aca832fb9886544) )

	ROM_REGION( 0x0800, "pals", 0 )
	ROM_LOAD( "mmipal20l8.ic3", 0x0000, 0x0800, NO_DUMP )
ROM_END

ROM_START( puzznicj )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "c20-04.ic11",  0x00000, 0x20000, CRC(a4150b6c) SHA1(27719b8993735532cd59f4ed5693ff3143ee2336) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "mc68705p3.ic4", 0x0000, 0x0800, CRC(085f68b4) SHA1(2dbc7e2c015220dc59ee1f1208540744e5b9b7cc) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "c20-03.ic10",  0x00000, 0x20000, CRC(4264056c) SHA1(d2d8a170ae0f361093a5384935238605a59e5938) )
	ROM_LOAD16_BYTE( "c20-02.ic9",   0x00001, 0x20000, CRC(3c115f8b) SHA1(8d518be01b7c4d6d993d5d9b62aab719a5c8baca) )

	ROM_REGION( 0x0200, "pals", 0 ) // PAL20L8
	ROM_LOAD( "c20-05.ic3", 0x0000, 0x0144, CRC(f90e5594) SHA1(6181bb25b77028bb150c84bdc073f0457efd7eaa) ) // Confirmed/Matches Japan Set
ROM_END

ROM_START( puzznici ) /* bootleg (original main board, bootleg sub-board without MCU) */
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1.ic11",  0x00000, 0x20000, CRC(4612f5e0) SHA1(dc07a365414666568537d31ef01b58f2362cadaf) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "u10.ic10",  0x00000, 0x20000, CRC(4264056c) SHA1(d2d8a170ae0f361093a5384935238605a59e5938) )
	ROM_LOAD16_BYTE( "3.ic9",     0x00001, 0x20000, CRC(2bf5232a) SHA1(a8fc06bb8bae2ca6bd21e3a96c9ed38bb356d5d7) )
ROM_END

ROM_START( puzznicb ) /* bootleg (original main board, bootleg sub-board without MCU) */
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ic11.bin",  0x00000, 0x20000, CRC(2510df4d) SHA1(534327e3d7f847b6c0effc5fd0fb9f5da9b0d3b1) )

	ROM_REGION( 0x20000, "gfx1", 0 ) // this has the bad line in tile 1 fixed (unused I believe) are we sure the roms used in the original sets are a good dump?
	ROM_LOAD16_BYTE( "ic10.bin",  0x00000, 0x10000, CRC(be12749a) SHA1(c67d1a434486843a6776d89e905362b7db595d8d) )
	ROM_LOAD16_BYTE( "ic9.bin",   0x00001, 0x10000, CRC(0f183340) SHA1(9eef7de801eb9763313f55a38e567b92fca3bfa6) )
ROM_END

ROM_START( puzznicba ) /* bootleg (original main board, bootleg sub-board without MCU) - marked PUZZNIC-2 008900 42 */
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "18.ic10",  0x00000, 0x20000, CRC(8349eb3b) SHA1(589dc99a22b3d7623b1ea6c1053f3b3dfe520547) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "19.ic9",  0x00000, 0x20000, CRC(4264056c) SHA1(d2d8a170ae0f361093a5384935238605a59e5938) )
	ROM_LOAD16_BYTE( "20.ic8",  0x00001, 0x20000, CRC(3c115f8b) SHA1(8d518be01b7c4d6d993d5d9b62aab719a5c8baca) )
ROM_END
/*

Taito's Horse Shoe

Main PCB is a L System board with a SUB PCB for roms.

Main (M4300189A / K1100589A):
   CPU: TC0090LVC (Embedded Z80 core)
 Sound: Yamaha YM2203C / Y3014B
   RAM: Four 43256 compatible type ram (Toshiba TC55257APL-10)
   OSC: 13.33056MHz
  DIPS: Two 8-way dipswitch blocks

SUB PCB (K9100282A / J9100220A)
 5 Rom chips type M27C1001 labeled C47 01 through C47 05
 Pal 20L8BCNS
 NEC uPD4701AC

*/

ROM_START( horshoes )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "c47-03.ic6",  0x00000, 0x20000, CRC(37e15b20) SHA1(85baa0ee553e4c9fed38294ba8912f18f519e62f) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "c47-02.ic5",  0x00000, 0x10000, CRC(35f96526) SHA1(e7f9b33d82b050aff49f991aa12db436421caa5b) ) /* silkscreened CH0-L */
	ROM_CONTINUE (           0x40000, 0x10000 )
	ROM_LOAD16_BYTE( "c47-01.ic11", 0x20000, 0x10000, CRC(031c73d8) SHA1(deef972fbf226701f9a6469ae3934129dc52ce9c) ) /* silkscreened CH1-L */
	ROM_CONTINUE (           0x60000, 0x10000 )
	ROM_LOAD16_BYTE( "c47-04.ic4",  0x00001, 0x10000, CRC(aeac7121) SHA1(cf67688cde14d452da6d9cbd7a81593f4048ce77) ) /* silkscreened CH0-H */
	ROM_CONTINUE (           0x40001, 0x10000 )
	ROM_LOAD16_BYTE( "c47-05.ic10", 0x20001, 0x10000, CRC(b2a3dafe) SHA1(5ffd3e296272ef3f31432005c827f057aac79497) ) /* silkscreened CH1-H */
	ROM_CONTINUE (           0x60001, 0x10000 )

	ROM_REGION( 0x0200, "plds", 0 ) // PAL20L8BCNS
	ROM_LOAD( "c47-06.ic12", 0x0000, 0x0144, CRC(4342ca6c) SHA1(9c798a6f1508b03004b76577eb823f004df7298d) )

ROM_END

ROM_START( palamed )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "c63.02", 0x00000, 0x20000, CRC(55a82bb2) SHA1(f157ad770351d4b8d8f8c061c4e330d6391fc624) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "c63.04", 0x00000, 0x20000, CRC(c7bbe460) SHA1(1c1f186d0b0b2e383f82c53ae93b975a75f50f9c) )
	ROM_LOAD16_BYTE( "c63.03", 0x00001, 0x20000, CRC(fcd86e44) SHA1(bdd0750ed6e93cc49f09f4ccb05b0c4a44cb9c23) )
ROM_END

ROM_START( cachat )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cac6",  0x00000, 0x20000, CRC(8105cf5f) SHA1(e6dd22165436c247db887a04c3e69c9e2505bb33) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "cac9",  0x00000, 0x20000, CRC(bc462914) SHA1(3eede8940cabadf563acb63059bfc2d13253b29f) )
	ROM_LOAD16_BYTE( "cac10", 0x40000, 0x20000, CRC(ecc64b31) SHA1(04ce97cdcdbdbd38602011f5ed27fe9182fb500a) )
	ROM_LOAD16_BYTE( "cac7",  0x00001, 0x20000, CRC(7fb71578) SHA1(34cfa1383ea1f3cbf45eaf6b989a1248cdef1bb9) )
	ROM_LOAD16_BYTE( "cac8",  0x40001, 0x20000, CRC(d2a63799) SHA1(71b024b239834ef068b7fc20cd49aae7853e0f7c) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal20l8b-c63-01.14", 0x0000, 0x0144, CRC(14a7dd2a) SHA1(2a39ca6069bdac553d73c34db6f50f880559113c) )
ROM_END

ROM_START( tubeit ) /* Title changed. Year, copyright and manufacture removed */
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "t-i_02.6", 0x00000, 0x20000, CRC(54730669) SHA1(a44ebd31a8588a133a7552a39fa8d52ba1985e45) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "t-i_03.7", 0x00001, 0x40000, CRC(e1c3fed0) SHA1(cd68dbf61ed820f4aa50c630e7cb778aafb433c2) )
	ROM_LOAD16_BYTE( "t-i_04.9", 0x00000, 0x40000, CRC(b4a6e31d) SHA1(e9abab8f19c78207f25a62104bcae1e391cbd2c0) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal20l8b-c63-01.14", 0x0000, 0x0144, CRC(14a7dd2a) SHA1(2a39ca6069bdac553d73c34db6f50f880559113c) )
ROM_END

ROM_START( cubybop )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "cb06.6", 0x00000, 0x40000, CRC(66b89a85) SHA1(2ba26d71fd1aa8e64584a5908a1d797666718d49) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "cb09.9",  0x00000, 0x40000, CRC(5f831e59) SHA1(db319a6c1058200274d687163b4df2f78a2bf879) )
	ROM_LOAD16_BYTE( "cb10.10", 0x80000, 0x40000, CRC(430510fc) SHA1(95c0a0ebd0485a15090f302e5d2f4da8204baf7c) )
	ROM_LOAD16_BYTE( "cb07.7",  0x00001, 0x40000, CRC(3582de99) SHA1(51620cc9044aef8e5ed0335b7d5d6d67a7857005) )
	ROM_LOAD16_BYTE( "cb08.8",  0x80001, 0x40000, CRC(09e18a51) SHA1(18db47d1d84f9be892bc796116c7ef7d0c1ee59f) )
ROM_END

ROM_START( plgirls )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "pg03.ic6", 0x00000, 0x40000, CRC(6ca73092) SHA1(f5679f047a29b936046c0d3677489df553ad7b41) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "pg02.ic9", 0x00000, 0x40000, CRC(3cf05ca9) SHA1(502c45a5330dda1b2fbf7d3d0c9bc6e889ff07d8) )
	ROM_LOAD16_BYTE( "pg01.ic7", 0x00001, 0x40000, CRC(79e41e74) SHA1(aa8efbeeee47f84e19b639821a89a7bcd67fe7a9) )
ROM_END

ROM_START( plgirls2 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "pg2_1j.ic6", 0x00000, 0x40000, CRC(f924197a) SHA1(ecaaefd1b3715ba60608e05d58be67e3c71f653a) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "cho-l.ic9",  0x00000, 0x80000, CRC(956384ec) SHA1(94a2b95f340e96bdccbeafd373f0dea90b8328dd) )
	ROM_LOAD16_BYTE( "cho-h.ic7",  0x00001, 0x80000, CRC(992f99b1) SHA1(c79f1014d73654740f7823812f92376d65d6b15d) )
ROM_END

ROM_START( plgirls2b )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "playgirls2b.d1", 0x00000, 0x40000, CRC(d58159fa) SHA1(541c6ca5f12c38b5a08f90048f52c31d27bb9233) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "playgirls2b.d8",   0x00003, 0x40000, CRC(22df48b5) SHA1(be51dbe55f84dd1b7c30da0e4d98c874b0803382) )
	ROM_LOAD32_BYTE( "playgirls2b.d4",   0x00001, 0x40000, CRC(bc9e2192) SHA1(7bc7f46295166a84c849e9ea82428e653375d9d6) )
	ROM_LOAD32_BYTE( "playgirls2b.b6",   0x00000, 0x40000, CRC(aac6c90b) SHA1(965cea2fb5f3aaabb4378fc24899af53de745ff3) )
	ROM_LOAD32_BYTE( "playgirls2b.d3",   0x00002, 0x40000, CRC(75d82fab) SHA1(4eb9ee416944a36f016e7d353f79884915da2730) )
ROM_END


ROM_START( evilston )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "c67-03.ic2",  0x00000, 0x20000, CRC(53419982) SHA1(ecc338e2237d26c5ff25b756d371b26b23beed1e) )
	ROM_LOAD( "c67-04.ic6",  0x20000, 0x20000, CRC(55d57e19) SHA1(8815bcaafe7ee056314b4131e3fb7963854dd6ba) )

	ROM_REGION( 0x80000, "audiocpu", 0 )
	ROM_LOAD( "c67-05.ic22", 0x00000, 0x20000, CRC(94d3a642) SHA1(af20aa5bb60a45c05eb1deba23ba30e6640ca235) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "c67-01.ic1",  0x00000, 0x80000, CRC(2f351bf4) SHA1(0fb37abf3413cd11baece1c9bbca5a51b0f28938) )
	ROM_LOAD( "c67-02.ic5",  0x80000, 0x80000, CRC(eb4f895c) SHA1(2c902572fe5a5d4442e4dd29e8a85cb40c384140) )
ROM_END

/*

LA Girl
Clearly a bootleg / hack of Play Girls by Hot-B, reportedly by Ta Ta Electronics, 1993

PCB Layout
----------

|------------------------------------------|
|VOL 4558 YM3014                 ROM4      |
|UPC1241              2018            ROM5 |
|     YM2203             TPC1020           |
|            DSW1(8)                       |
|            DSW2(8)                       |
|                         6264    ROM3     |
|J                                ROM2     |
|A  27.2109MHz                             |
|M                                         |
|M                                PAL      |
|A                                PAL      |
|        6264            TPC1020  PAL      |
|            ROM1                          |
|                                  6264    |
|                                  6264    |
|              PAL       TPC1020           |
|        Z80B                              |
|                 44256           44256    |
|PAL              44256           44256    |
|------------------------------------------|
Notes:
      Z80 - clock 6.802725MHz [27.2109/4]
   YM2203 - clocks 3.4013625 [27/2109/8] & 1.1337875 [27.2109/24]
    VSync - 55.8268Hz  \ possibly sync/PCB fault, had to adjust
    HSync - 14.7739kHz / h/v syncs on monitor to get a stable picture

*/

ROM_START( lagirl )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "rom1",  0x00000, 0x40000, CRC(ba1acfdb) SHA1(ff1093c2d0887287ce451417bd373e00f2881ce7) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "rom2",   0x00003, 0x20000, CRC(4c739a30) SHA1(4426f51aac9bb39f5d1a7616d183ff6c76749dc2) )
	ROM_LOAD32_BYTE( "rom3",   0x00001, 0x20000, CRC(4cf22a4b) SHA1(1c933ccbb6a5b8a6795385d7970db5f7138e572e) )
	ROM_LOAD32_BYTE( "rom4",   0x00002, 0x20000, CRC(7dcd6696) SHA1(8f3b1fe669520142668af6dc2d04f13767048989) )
	ROM_LOAD32_BYTE( "rom5",   0x00000, 0x20000, CRC(b1782816) SHA1(352663974886e1e4358e55b87c8bf0cdb979f177) )
ROM_END



// bits 7..0 => bits 0..7
DRIVER_INIT_MEMBER(taitol_state,plottinga)
{
	UINT8 tab[256];
	UINT8 *p;
	int i;

	for (i = 0; i < 256; i++)
	{
		int j, v = 0;
		for (j = 0; j < 8; j++)
			if (i & (1 << j))
				v |= 1 << (7 - j);
		tab[i] = v;
	}
	p = memregion("maincpu")->base();
	for (i = 0; i < 0x10000; i++)
	{
		*p = tab[*p];
		p++;
	}
}


GAME( 1988, raimais,   0,        raimais,   raimais,   driver_device, 0,         ROT0,   "Taito Corporation Japan", "Raimais (World)", 0 )
GAME( 1988, raimaisj,  raimais,  raimais,   raimaisj,  driver_device, 0,         ROT0,   "Taito Corporation", "Raimais (Japan)", 0 )
GAME( 1988, raimaisjo, raimais,  raimais,   raimaisj,  driver_device, 0,         ROT0,   "Taito Corporation", "Raimais (Japan, first revision)", 0 )

GAME( 1988, fhawk,     0,        fhawk,     fhawk,     driver_device, 0,         ROT270, "Taito Corporation Japan", "Fighting Hawk (World)", 0 )
GAME( 1988, fhawkj,    fhawk,    fhawk,     fhawkj,    driver_device, 0,         ROT270, "Taito Corporation", "Fighting Hawk (Japan)", 0 )

GAME( 1989, champwr,   0,        champwr,   champwr,   driver_device, 0,         ROT0,   "Taito Corporation Japan", "Champion Wrestler (World)", MACHINE_IMPERFECT_SOUND )
GAME( 1989, champwru,  champwr,  champwr,   champwru,  driver_device, 0,         ROT0,   "Taito America Corporation", "Champion Wrestler (US)", MACHINE_IMPERFECT_SOUND )
GAME( 1989, champwrj,  champwr,  champwr,   champwrj,  driver_device, 0,         ROT0,   "Taito Corporation", "Champion Wrestler (Japan)", MACHINE_IMPERFECT_SOUND )

GAME( 1988, kurikint,  0,        kurikint,  kurikint,  driver_device, 0,         ROT0,   "Taito Corporation Japan", "Kuri Kinton (World)", 0 )
GAME( 1988, kurikintu, kurikint, kurikint,  kurikintj, driver_device, 0,         ROT0,   "Taito America Corporation", "Kuri Kinton (US)", 0 )
GAME( 1988, kurikintj, kurikint, kurikint,  kurikintj, driver_device, 0,         ROT0,   "Taito Corporation", "Kuri Kinton (Japan)", 0 )
GAME( 1988, kurikinta, kurikint, kurikint,  kurikinta, driver_device, 0,         ROT0,   "Taito Corporation Japan", "Kuri Kinton (World, prototype?)", 0 )

GAME( 1989, plotting,  0,        plotting,  plotting,  driver_device, 0,         ROT0,   "Taito Corporation Japan", "Plotting (World set 1)", 0 )
GAME( 1989, plottinga, plotting, plotting,  plotting,  taitol_state,  plottinga, ROT0,   "Taito Corporation Japan", "Plotting (World set 2, protected)", 0 )
GAME( 1989, plottingb, plotting, plotting,  plotting,  driver_device, 0,         ROT0,   "Taito Corporation Japan", "Plotting (World set 3, earliest version)", 0 )
GAME( 1989, plottingu, plotting, plotting,  plotting,  driver_device, 0,         ROT0,   "Taito America Corporation", "Plotting (US)", 0 )
GAME( 1989, flipull,   plotting, plotting,  plotting,  driver_device, 0,         ROT0,   "Taito Corporation", "Flipull (Japan)", 0 )

GAME( 1989, puzznic,   0,        puzznic,   puzznic,   driver_device, 0,         ROT0,   "Taito Corporation Japan", "Puzznic (World)", 0 )
GAME( 1989, puzznicj,  puzznic,  puzznic,   puzznic,   driver_device, 0,         ROT0,   "Taito Corporation", "Puzznic (Japan)", 0 )
GAME( 1989, puzznici,  puzznic,  puzznici,  puzznic,   driver_device, 0,         ROT0,   "bootleg", "Puzznic (Italian bootleg)", 0 )
GAME( 1989, puzznicb,  puzznic,  puzznici,  puzznic,   driver_device, 0,         ROT0,   "bootleg", "Puzznic (bootleg, set 1)", 0 )
GAME( 1989, puzznicba, puzznic,  puzznici,  puzznic,   driver_device, 0,         ROT0,   "bootleg", "Puzznic (bootleg, set 2)", 0 )

GAME( 1990, horshoes,  0,        horshoes,  horshoes,  driver_device, 0,         ROT270, "Taito America Corporation", "American Horseshoes (US)", 0 )

GAME( 1990, palamed,   0,        palamed,   palamed,   driver_device, 0,         ROT0,   "Taito Corporation", "Palamedes (Japan)", 0 )

GAME( 1993, cachat,    0,        cachat,    cachat,    driver_device, 0,         ROT0,   "Taito Corporation", "Cachat (Japan)", 0 )
GAME( 1993, tubeit,    cachat,   cachat,    tubeit,    driver_device, 0,         ROT0,   "bootleg", "Tube-It", 0 ) // No (c) message

GAME( 199?, cubybop,   0,        cachat,    cubybop,   driver_device, 0,         ROT0,   "Hot-B", "Cuby Bop (location test)", 0 ) // No (c) message, but Hot-B company logo in tile gfx

GAME( 1992, plgirls,   0,        cachat,    plgirls,   driver_device, 0,         ROT270, "Hot-B", "Play Girls", 0 )
GAME( 1992, lagirl,    plgirls,  cachat,    plgirls,   driver_device, 0,         ROT270, "bootleg", "LA Girl", 0 ) // bootleg hardware with changed title & backgrounds

GAME( 1993, plgirls2,  0,        cachat,    plgirls2,  driver_device, 0,         ROT270, "Hot-B", "Play Girls 2", 0 )
GAME( 1993, plgirls2b, plgirls2, cachat,    plgirls2,  driver_device, 0,         ROT270, "bootleg", "Play Girls 2 (bootleg)", MACHINE_IMPERFECT_GRAPHICS ) // bootleg hardware (regular Z80 etc. instead of TC0090LVC, but acts almost the same - scroll offset problems)

GAME( 1990, evilston,  0,        evilston,  evilston,  driver_device, 0,         ROT270, "Spacy Industrial, Ltd.", "Evil Stone", MACHINE_IMPERFECT_SOUND ) // not Taito PCB, just uses TC0090LVC

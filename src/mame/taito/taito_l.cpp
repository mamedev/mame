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
- Of the several multi-processor games that use the MB8421 dual-port RAM for
  communications, Evil Stone seems to be the only one to use its special
  interrupt feature.

TODO:
- plgirls doesn't work without a kludge because of an interrupt issue. This
  happens because the program enables interrupts before setting IM2, so the
  interrupt vector is interpreted as IM0, which is obviously bogus.
- A bunch of control registers are simply ignored
- The source of   irqs 0 and  1 is  unknown, while  2 is vblank  (0 is
  usually   ignored  by the  program,    1   leads  to  reading    the
  ports... maybe vbl-in, vbl-out and hblank ?).
- Text Plane colours are only right in Cuby Bop once you've started a game
  & reset
- Scrolling in Cuby Bop's Game seems incorrect.

puzznici note
- this set is a bootleg, it uses a converted board without the MCU and has
  a hacked copyright message.  The tilemap data for one of the girls appears
  to be corrupt, however this is correct, the bootleggers overwrote part of
  the data with the expected response sequence from the MCU in order to simulate
  it.

*/

#include "emu.h"
#include "taito_l.h"

#include "taitoio.h"
#include "taitoipt.h"
#include "taitosnd.h"

#include "taito68705.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/mb8421.h"
#include "sound/msm5205.h"
#include "sound/ymopn.h"

#include "screen.h"
#include "speaker.h"


void taitol_state::machine_start()
{
	save_item(NAME(m_last_irq_level));
}

void taitol_2cpu_state::machine_start()
{
	taitol_state::machine_start();

	if (m_audio_bnk.found())
		m_audio_bnk->configure_entries(0, m_audio_prg->bytes()/0x4000, m_audio_prg->base(), 0x4000);
}

void fhawk_state::machine_start()
{
	taitol_2cpu_state::machine_start();

	m_slave_bnk->configure_entries(0, m_slave_prg->bytes()/0x4000, m_slave_prg->base(), 0x4000);

	save_item(NAME(m_slave_rombank));
}

void champwr_state::machine_start()
{
	fhawk_state::machine_start();

	save_item(NAME(m_adpcm_pos));
	save_item(NAME(m_adpcm_data));
}


void taitol_state::machine_reset()
{
	m_last_irq_level = 0;
}

void fhawk_state::machine_reset()
{
	taitol_2cpu_state::machine_reset();

	m_slave_rombank = 0;
	m_slave_bnk->set_entry(m_slave_rombank);

	m_audio_bnk->set_entry(1);
}

void champwr_state::machine_reset()
{
	fhawk_state::machine_reset();

	m_adpcm_pos = 0;
	m_adpcm_data = -1;
}


IRQ_CALLBACK_MEMBER(taitol_state::irq_callback)
{
	m_main_cpu->set_input_line(0, CLEAR_LINE);
	return m_main_cpu->irq_vector(m_last_irq_level);
}

TIMER_DEVICE_CALLBACK_MEMBER(taitol_state::vbl_interrupt)
{
	int scanline = param;

	/* kludge to make plgirls boot */
	if (m_main_cpu->state_int(Z80_IM) != 2)
		return;

	// What is really generating interrupts 0 and 1 is still to be found

	if (scanline == 120 && (m_main_cpu->irq_enable() & 1))
	{
		m_last_irq_level = 0;
		m_main_cpu->set_input_line(0, ASSERT_LINE);
	}
	else if (scanline == 0 && (m_main_cpu->irq_enable() & 2))
	{
		m_last_irq_level = 1;
		m_main_cpu->set_input_line(0, ASSERT_LINE);
	}
	else if (scanline == 240 && (m_main_cpu->irq_enable() & 4))
	{
		m_last_irq_level = 2;
		m_main_cpu->set_input_line(0, ASSERT_LINE);
	}
}

void taitol_state::irq_enable_w(u8 data)
{
	//logerror("irq_enable = %02x\n",data);
	m_main_cpu->irq_enable_w(data);

	// fix Plotting test mode
	if ((m_main_cpu->irq_enable() & (1 << m_last_irq_level)) == 0)
		m_main_cpu->set_input_line(0, CLEAR_LINE);
}


void fhawk_state::slave_rombank_w(u8 data)
{
	m_slave_rombank = data & 0xf;
	m_slave_bnk->set_entry(m_slave_rombank);
}

u8 fhawk_state::slave_rombank_r()
{
	return m_slave_rombank;
}

void taitol_state::coin_control_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x08);
}

u8 taitol_1cpu_state::extport_select_and_ym2203_r(offs_t offset)
{
	for (auto &mux : m_mux)
		mux->select_w((offset >> 1) & 1);
	return m_ymsnd->read(offset & 1);
}

void taitol_state::mcu_control_w(u8 data)
{
//  logerror("mcu control %02x (%04x)\n", data, m_main_cpu->pc());
}

u8 taitol_state::mcu_control_r()
{
//  logerror("mcu control read (%04x)\n", m_main_cpu->pc());
	return 0x1;
}

void champwr_state::msm5205_vck(int state)
{
	if (m_adpcm_data != -1)
	{
		m_msm->data_w(m_adpcm_data & 0x0f);
		m_adpcm_data = -1;
	}
	else
	{
		m_adpcm_data = m_adpcm_rgn[m_adpcm_pos];
		m_adpcm_pos = (m_adpcm_pos + 1) & 0x1ffff;
		m_msm->data_w(m_adpcm_data >> 4);
	}
}

void champwr_state::msm5205_lo_w(u8 data)
{
	m_adpcm_pos = (m_adpcm_pos & 0xff00ff) | (data << 8);
}

void champwr_state::msm5205_hi_w(u8 data)
{
	m_adpcm_pos = ((m_adpcm_pos & 0x00ffff) | (data << 16)) & 0x1ffff;
}

void champwr_state::msm5205_start_w(u8 data)
{
	m_msm->reset_w(0);
}

void champwr_state::msm5205_stop_w(u8 data)
{
	m_msm->reset_w(1);
	m_adpcm_pos &= 0x1ff00;
}

void champwr_state::msm5205_volume_w(u8 data)
{
	m_msm->set_output_gain(0, data / 255.0);
}


void taitol_state::common_banks_map(address_map &map)
{
	map(0xfe00, 0xfeff).rw(m_main_cpu, FUNC(tc0090lvc_device::vregs_r), FUNC(tc0090lvc_device::vregs_w));
	map(0xff00, 0xff02).mirror(0x00f0).rw(m_main_cpu, FUNC(tc0090lvc_device::irq_vector_r), FUNC(tc0090lvc_device::irq_vector_w));
	map(0xff03, 0xff03).mirror(0x00f0).r(m_main_cpu, FUNC(tc0090lvc_device::irq_enable_r)).w(FUNC(taitol_state::irq_enable_w));
	map(0xff04, 0xff07).mirror(0x00f0).rw(m_main_cpu, FUNC(tc0090lvc_device::ram_bank_r), FUNC(tc0090lvc_device::ram_bank_w));
	map(0xff08, 0xff08).mirror(0x00f0).rw(m_main_cpu, FUNC(tc0090lvc_device::rom_bank_r), FUNC(tc0090lvc_device::rom_bank_w));
}

void fhawk_state::fhawk_map(address_map &map)
{
	common_banks_map(map);
	map(0x8000, 0x9fff).ram().share("share1");
	map(0xa000, 0xbfff).ram();
}

void fhawk_state::fhawk_2_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("slavebank");
	map(0xc000, 0xc000).w(FUNC(fhawk_state::slave_rombank_w));
	map(0xc800, 0xc800).nopr().w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0xc801, 0xc801).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	map(0xd000, 0xd007).rw("tc0220ioc", FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write));
	map(0xe000, 0xffff).ram().share("share1");
}

void fhawk_state::fhawk_3_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr("audiobank");
	map(0x8000, 0x9fff).ram();
	map(0xe000, 0xe000).nopr().w("ciu", FUNC(pc060ha_device::slave_port_w));
	map(0xe001, 0xe001).rw("ciu", FUNC(pc060ha_device::slave_comm_r), FUNC(pc060ha_device::slave_comm_w));
	map(0xf000, 0xf001).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}


void taitol_2cpu_state::raimais_map(address_map &map)
{
	common_banks_map(map);
	map(0x8000, 0x87ff).rw("dpram", FUNC(mb8421_device::right_r), FUNC(mb8421_device::right_w));
	map(0x8800, 0x8801).rw("tc0040ioc", FUNC(tc0040ioc_device::read), FUNC(tc0040ioc_device::write));
	map(0x8c00, 0x8c00).nopr().w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x8c01, 0x8c01).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0xa000, 0xbfff).ram();
}

void taitol_2cpu_state::raimais_2_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe7ff).rw("dpram", FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w));
}


void taitol_2cpu_state::sound_bankswitch_w(u8 data)
{
	m_audio_bnk->set_entry(data & 0x03);
}

void taitol_2cpu_state::raimais_3_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr("audiobank");
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe003).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0xe200, 0xe200).nopr().w("tc0140syt", FUNC(tc0140syt_device::slave_port_w));
	map(0xe201, 0xe201).rw("tc0140syt", FUNC(tc0140syt_device::slave_comm_r), FUNC(tc0140syt_device::slave_comm_w));
	map(0xe400, 0xe403).nopw(); // pan
	map(0xe600, 0xe600).nopw(); // ?
	map(0xee00, 0xee00).nopw(); // ?
	map(0xf000, 0xf000).nopw(); // ?
	map(0xf200, 0xf200).w(FUNC(taitol_2cpu_state::sound_bankswitch_w));
}


void champwr_state::champwr_map(address_map &map)
{
	common_banks_map(map);
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xbfff).ram().share("share1");
}

void champwr_state::champwr_2_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("slavebank");
	map(0xc000, 0xdfff).ram().share("share1");
	map(0xe000, 0xe007).rw("tc0220ioc", FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write));
	map(0xe008, 0xe00f).nopr();
	map(0xe800, 0xe800).nopr().w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0xe801, 0xe801).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	map(0xf000, 0xf000).rw(FUNC(champwr_state::slave_rombank_r), FUNC(champwr_state::slave_rombank_w));
}

void champwr_state::champwr_3_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr("audiobank");
	map(0x8000, 0x8fff).ram();
	map(0x9000, 0x9001).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xa000, 0xa000).nopr().w("ciu", FUNC(pc060ha_device::slave_port_w));
	map(0xa001, 0xa001).rw("ciu", FUNC(pc060ha_device::slave_comm_r), FUNC(pc060ha_device::slave_comm_w));
	map(0xb000, 0xb000).w(FUNC(champwr_state::msm5205_hi_w));
	map(0xc000, 0xc000).w(FUNC(champwr_state::msm5205_lo_w));
	map(0xd000, 0xd000).w(FUNC(champwr_state::msm5205_start_w));
	map(0xe000, 0xe000).w(FUNC(champwr_state::msm5205_stop_w));
}



void taitol_2cpu_state::kurikint_map(address_map &map)
{
	common_banks_map(map);
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xa7ff).rw("dpram", FUNC(mb8421_device::right_r), FUNC(mb8421_device::right_w));
	map(0xa800, 0xa801).rw("tc0040ioc", FUNC(tc0040ioc_device::read), FUNC(tc0040ioc_device::write));
}

void taitol_2cpu_state::kurikint_2_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe7ff).rw("dpram", FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w));
	map(0xe800, 0xe801).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}



void taitol_1cpu_state::puzznic_map(address_map &map)
{
	common_banks_map(map);
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xa003).r(FUNC(taitol_1cpu_state::extport_select_and_ym2203_r)).w(m_ymsnd, FUNC(ym2203_device::write));
	map(0xa800, 0xa800).nopr(); // Watchdog
	map(0xb800, 0xb800).rw("mcu", FUNC(arkanoid_68705p3_device::data_r), FUNC(arkanoid_68705p3_device::data_w));
	map(0xb801, 0xb801).rw(FUNC(taitol_1cpu_state::mcu_control_r), FUNC(taitol_1cpu_state::mcu_control_w));
	map(0xbc00, 0xbc00).nopw(); // Control register, function unknown
}

/* bootleg, doesn't have the MCU */
void taitol_1cpu_state::puzznici_map(address_map &map)
{
	common_banks_map(map);
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xa003).r(FUNC(taitol_1cpu_state::extport_select_and_ym2203_r)).w(m_ymsnd, FUNC(ym2203_device::write));
	map(0xa800, 0xa800).nopr(); // Watchdog
	map(0xb801, 0xb801).r(FUNC(taitol_1cpu_state::mcu_control_r));
//  map(0xb801, 0xb801).w(FUNC(taitol_1cpu_state::mcu_control_w));
	map(0xbc00, 0xbc00).nopw(); // Control register, function unknown
}


void taitol_1cpu_state::plotting_map(address_map &map)
{
	common_banks_map(map);
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xa003).r(FUNC(taitol_1cpu_state::extport_select_and_ym2203_r)).w(m_ymsnd, FUNC(ym2203_device::write));
	map(0xa800, 0xa800).nopw(); // Watchdog or interrupt ack
	map(0xb800, 0xb800).nopw(); // Control register, function unknown
}


void taitol_1cpu_state::palamed_map(address_map &map)
{
	common_banks_map(map);
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xa003).rw(m_ymsnd, FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xa800, 0xa803).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xb000, 0xb000).nopw(); // Control register, function unknown (copy of 8822)
	map(0xb001, 0xb001).nopr(); // Watchdog or interrupt ack (value ignored in cachat)
}


void horshoes_state::horshoes_map(address_map &map)
{
	common_banks_map(map);
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xa003).r(FUNC(horshoes_state::extport_select_and_ym2203_r)).w(m_ymsnd, FUNC(ym2203_device::write));
	map(0xa800, 0xa800).select(0x000c).lr8(NAME([this](offs_t offset) { return m_upd4701->read_xy(offset >> 2); }));
	map(0xa802, 0xa802).r(m_upd4701, FUNC(upd4701_device::reset_x_r));
	map(0xa803, 0xa803).r(m_upd4701, FUNC(upd4701_device::reset_y_r));
	map(0xb801, 0xb801).nopr(); // Watchdog or interrupt ack
	map(0xb802, 0xb802).w(FUNC(horshoes_state::bankg_w));
	map(0xbc00, 0xbc00).nopw();
}

void taitol_2cpu_state::evilston_map(address_map &map)
{
	common_banks_map(map);
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xa7ff).rw("dpram", FUNC(mb8421_device::right_r), FUNC(mb8421_device::right_w));
	map(0xa800, 0xa807).rw("tc0510nio", FUNC(tc0510nio_device::read), FUNC(tc0510nio_device::write));
}

void taitol_2cpu_state::evilston_2_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe7ff).rw("dpram", FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w));
	map(0xe800, 0xe801).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xf000, 0xf7ff).bankr("audiobank");
}



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
	PORT_DIPSETTING(    0x08, "Every 80k" )
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

static INPUT_PORTS_START( plottingu )
	PORT_INCLUDE(plotting)
	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW1)
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
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_REVERSE

	PORT_START("AN1")
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30)
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
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)

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


void fhawk_state::portA_w(u8 data)
{
	m_audio_bnk->set_entry(data & 0x03);
	//logerror ("YM2203 bank change val=%02x  %s\n", data & 0x03, machine().describe_context() );
}

void taitol_state::l_system_video(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	screen.set_screen_update(m_main_cpu, FUNC(tc0090lvc_device::screen_update));
	screen.screen_vblank().set(FUNC(taitol_state::screen_vblank_taitol));
	screen.set_palette("maincpu:palette");

	TIMER(config, "scantimer").configure_scanline(FUNC(taitol_state::vbl_interrupt), "screen", 0, 1);
}

void fhawk_state::fhawk(machine_config &config)
{
	/* basic machine hardware */
	TC0090LVC(config, m_main_cpu, XTAL(13'330'560)/2);    /* verified freq on pin122 of TC0090LVC cpu */
	m_main_cpu->set_addrmap(AS_PROGRAM, &fhawk_state::fhawk_map);
	m_main_cpu->set_irq_acknowledge_callback(FUNC(taitol_state::irq_callback));

	Z80(config, m_audio_cpu, 12_MHz_XTAL/3);        /* verified on pcb */
	m_audio_cpu->set_addrmap(AS_PROGRAM, &fhawk_state::fhawk_3_map);

	z80_device &slave(Z80(config, "slave", 12_MHz_XTAL/3)); /* verified on pcb */
	slave.set_addrmap(AS_PROGRAM, &fhawk_state::fhawk_2_map);
	slave.set_vblank_int("screen", FUNC(taitol_state::irq0_line_hold));

	config.set_perfect_quantum(m_main_cpu);

	tc0220ioc_device &tc0220ioc(TC0220IOC(config, "tc0220ioc", 0));
	tc0220ioc.read_0_callback().set_ioport("DSWA");
	tc0220ioc.read_1_callback().set_ioport("DSWB");
	tc0220ioc.read_2_callback().set_ioport("IN0");
	tc0220ioc.read_3_callback().set_ioport("IN1");
	tc0220ioc.write_4_callback().set(FUNC(taitol_state::coin_control_w));
	tc0220ioc.read_7_callback().set_ioport("IN2");

	/* video hardware */
	l_system_video(config);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 12_MHz_XTAL/4));       /* verified on pcb */
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.port_a_write_callback().set(FUNC(fhawk_state::portA_w));
	ymsnd.add_route(0, "mono", 0.20);
	ymsnd.add_route(1, "mono", 0.20);
	ymsnd.add_route(2, "mono", 0.20);
	ymsnd.add_route(3, "mono", 0.80);

	pc060ha_device &ciu(PC060HA(config, "ciu", 0));
	ciu.nmi_callback().set_inputline(m_audio_cpu, INPUT_LINE_NMI);
	ciu.reset_callback().set_inputline(m_audio_cpu, INPUT_LINE_RESET);
}

void champwr_state::champwr(machine_config &config)
{
	fhawk(config);

	/* basic machine hardware */
	m_main_cpu->set_addrmap(AS_PROGRAM, &champwr_state::champwr_map);

	m_audio_cpu->set_addrmap(AS_PROGRAM, &champwr_state::champwr_3_map);

	subdevice<cpu_device>("slave")->set_addrmap(AS_PROGRAM, &champwr_state::champwr_2_map);

	/* sound hardware */
	subdevice<ym2203_device>("ymsnd")->port_b_write_callback().set(FUNC(champwr_state::msm5205_volume_w));

	MSM5205(config, m_msm, 384_kHz_XTAL);
	m_msm->vck_legacy_callback().set(FUNC(champwr_state::msm5205_vck)); /* VCK function */
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);  /* 8 kHz */
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.80);
}

void taitol_2cpu_state::raimais(machine_config &config)
{
	TC0090LVC(config, m_main_cpu, 13330560/2);    // needs verification from pin122 of TC0090LVC
	m_main_cpu->set_addrmap(AS_PROGRAM, &taitol_2cpu_state::raimais_map);
	m_main_cpu->set_irq_acknowledge_callback(FUNC(taitol_state::irq_callback));

	Z80(config, m_audio_cpu, 12000000/3);   // not verified
	m_audio_cpu->set_addrmap(AS_PROGRAM, &taitol_2cpu_state::raimais_3_map);

	z80_device &slave(Z80(config, "slave", 12000000/3));    // not verified
	slave.set_addrmap(AS_PROGRAM, &taitol_2cpu_state::raimais_2_map);
	slave.set_vblank_int("screen", FUNC(taitol_state::irq0_line_hold));

	config.set_perfect_quantum(m_main_cpu);

	tc0040ioc_device &tc0040ioc(TC0040IOC(config, "tc0040ioc", 0));
	tc0040ioc.read_0_callback().set_ioport("DSWA");
	tc0040ioc.read_1_callback().set_ioport("DSWB");
	tc0040ioc.read_2_callback().set_ioport("IN0");
	tc0040ioc.read_3_callback().set_ioport("IN1");
	tc0040ioc.write_4_callback().set(FUNC(taitol_state::coin_control_w));
	tc0040ioc.read_7_callback().set_ioport("IN2");

	MB8421(config, "dpram");

	/* video hardware */
	l_system_video(config);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 8_MHz_XTAL)); /* verified on pcb (8Mhz OSC is also for the 2nd z80) */
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 1.0);
	ymsnd.add_route(2, "mono", 1.0);

	tc0140syt_device &tc0140syt(TC0140SYT(config, "tc0140syt", 0));
	tc0140syt.nmi_callback().set_inputline(m_audio_cpu, INPUT_LINE_NMI);
	tc0140syt.reset_callback().set_inputline(m_audio_cpu, INPUT_LINE_RESET);
}

void taitol_2cpu_state::kurikint(machine_config &config)
{
	/* basic machine hardware */
	TC0090LVC(config, m_main_cpu, XTAL(13'330'560)/2);    /* verified freq on pin122 of TC0090LVC cpu */
	m_main_cpu->set_addrmap(AS_PROGRAM, &taitol_2cpu_state::kurikint_map);
	m_main_cpu->set_irq_acknowledge_callback(FUNC(taitol_state::irq_callback));

	Z80(config, m_audio_cpu, 12_MHz_XTAL/3);        /* verified on pcb */
	m_audio_cpu->set_addrmap(AS_PROGRAM, &taitol_2cpu_state::kurikint_2_map);
	m_audio_cpu->set_vblank_int("screen", FUNC(taitol_state::irq0_line_hold));

	config.set_maximum_quantum(attotime::from_hz(6000));

	tc0040ioc_device &tc0040ioc(TC0040IOC(config, "tc0040ioc", 0));
	tc0040ioc.read_0_callback().set_ioport("DSWA");
	tc0040ioc.read_1_callback().set_ioport("DSWB");
	tc0040ioc.read_2_callback().set_ioport("IN0");
	tc0040ioc.read_3_callback().set_ioport("IN1");
	tc0040ioc.write_4_callback().set(FUNC(taitol_state::coin_control_w));
	tc0040ioc.read_7_callback().set_ioport("IN2");

	MB8421(config, "dpram");

	/* video hardware */
	l_system_video(config);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 12_MHz_XTAL/4));       /* verified on pcb */
	ymsnd.add_route(0, "mono", 0.20);
	ymsnd.add_route(1, "mono", 0.20);
	ymsnd.add_route(2, "mono", 0.20);
	ymsnd.add_route(3, "mono", 0.80);
}

void taitol_1cpu_state::add_muxes(machine_config &config)
{
	LS157_X2(config, m_mux[0], 0);
	m_mux[0]->a_in_callback().set_ioport("DSWA");
	m_mux[0]->b_in_callback().set_ioport("DSWB");

	LS157_X2(config, m_mux[1], 0);
	m_mux[1]->a_in_callback().set_ioport("IN0");
	m_mux[1]->b_in_callback().set_ioport("IN1");
}

void taitol_1cpu_state::base(machine_config &config)
{
	/* basic machine hardware */
	TC0090LVC(config, m_main_cpu, XTAL(13'330'560)/2);    /* verified freq on pin122 of TC0090LVC cpu */
	m_main_cpu->set_addrmap(AS_PROGRAM, &taitol_1cpu_state::plotting_map);
	m_main_cpu->set_irq_acknowledge_callback(FUNC(taitol_state::irq_callback));

	/* video hardware */
	l_system_video(config);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2203(config, m_ymsnd, XTAL(13'330'560)/4); /* verified on pcb */
	m_ymsnd->port_a_read_callback().set("dswmux", FUNC(ls157_x2_device::output_r));
	m_ymsnd->port_b_read_callback().set("inmux", FUNC(ls157_x2_device::output_r));
	m_ymsnd->add_route(0, "mono", 0.20);
	m_ymsnd->add_route(1, "mono", 0.20);
	m_ymsnd->add_route(2, "mono", 0.20);
	m_ymsnd->add_route(3, "mono", 0.80);
}

void taitol_1cpu_state::plotting(machine_config &config)
{
	base(config);
	add_muxes(config);
}

void taitol_1cpu_state::puzznic(machine_config &config)
{
	base(config);
	add_muxes(config);
	m_main_cpu->set_addrmap(AS_PROGRAM, &taitol_1cpu_state::puzznic_map);

	ARKANOID_68705P3(config, "mcu", 3_MHz_XTAL);
}

void taitol_1cpu_state::puzznici(machine_config &config)
{
	base(config);
	add_muxes(config);
	m_main_cpu->set_addrmap(AS_PROGRAM, &taitol_1cpu_state::puzznici_map);
}

void horshoes_state::machine_start()
{
	taitol_1cpu_state::machine_start();

	save_item(NAME(m_horshoes_gfxbank));
}

void horshoes_state::machine_reset()
{
	taitol_1cpu_state::machine_reset();

	m_horshoes_gfxbank = 0;
}

void horshoes_state::horshoes(machine_config &config)
{
	base(config);
	add_muxes(config);

	m_main_cpu->set_addrmap(AS_PROGRAM, &horshoes_state::horshoes_map);
	m_main_cpu->set_tile_callback(FUNC(horshoes_state::horshoes_tile_cb));

	UPD4701A(config, m_upd4701, 0);
	m_upd4701->set_portx_tag("AN0");
	m_upd4701->set_porty_tag("AN1");
}


void taitol_1cpu_state::palamed(machine_config &config)
{
	plotting(config);

	/* basic machine hardware */
	m_main_cpu->set_addrmap(AS_PROGRAM, &taitol_1cpu_state::palamed_map);

	i8255_device &ppi(I8255(config, "ppi", 0)); // Toshiba TMP8255AP-5
	ppi.in_pa_callback().set_ioport("IN0");
	ppi.in_pb_callback().set_ioport("IN1");
	ppi.in_pc_callback().set_ioport("IN2");

	m_ymsnd->port_a_read_callback().set_ioport("DSWA");
	m_ymsnd->port_b_read_callback().set_ioport("DSWB");
}


void taitol_1cpu_state::cachat(machine_config &config)
{
	palamed(config);

	// NEC D70155C for inputs, instead TMP8255AP-5
}

void taitol_2cpu_state::evilston(machine_config &config)
{
	/* basic machine hardware */
	TC0090LVC(config, m_main_cpu, XTAL(13'330'560)/2);    /* not verified */
	m_main_cpu->set_addrmap(AS_PROGRAM, &taitol_2cpu_state::evilston_map);
	m_main_cpu->set_irq_acknowledge_callback(FUNC(taitol_state::irq_callback));

	Z80(config, m_audio_cpu, 12_MHz_XTAL/3);        /* not verified */
	m_audio_cpu->set_addrmap(AS_PROGRAM, &taitol_2cpu_state::evilston_2_map);
	m_audio_cpu->set_vblank_int("screen", FUNC(taitol_state::irq0_line_hold));

	config.set_maximum_quantum(attotime::from_hz(6000));

	tc0510nio_device &tc0510nio(TC0510NIO(config, "tc0510nio", 0));
	tc0510nio.read_0_callback().set_ioport("DSWA");
	tc0510nio.read_1_callback().set_ioport("DSWB");
	tc0510nio.read_2_callback().set_ioport("IN0");
	tc0510nio.read_3_callback().set_ioport("IN1");
	tc0510nio.write_4_callback().set(FUNC(taitol_state::coin_control_w));
	tc0510nio.read_7_callback().set_ioport("IN2");

	mb8421_device &dpram(MB8421(config, "dpram"));
	dpram.intl_callback().set_inputline("audiocpu", INPUT_LINE_NMI);

	/* video hardware */
	l_system_video(config);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 12_MHz_XTAL/4)); /* not verified */
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 0.25);
	ymsnd.add_route(2, "mono", 0.25);
	ymsnd.add_route(3, "mono", 0.80);
}


ROM_START( raimais )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "b36_11-1.ic7", 0x00000, 0x20000, CRC(f19fb0d5) SHA1(ba7187dfa5b4a08cebf236913a80066dafbbc59f) )
	ROM_LOAD( "b36_09.ic13",  0x20000, 0x20000, CRC(9c466e43) SHA1(2466a3f1f8124323008c9925f90e9a1d2edf1564) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b36_06.ic24",  0x00000, 0x10000, CRC(29bbc4f8) SHA1(39a68729c6180c5f6cdf604e692018e7d6bf5591) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "b36_07.ic2",   0x00000, 0x10000, CRC(4f3737e6) SHA1(ff5f5d4ca5485441d03c8cb01a6a096941ab02eb) )

	ROM_REGION( 0x100000, "maincpu:gfx", 0 )
	ROM_LOAD( "b36-01.ic6",   0x00000, 0x80000, CRC(89355cb2) SHA1(433e929fe8b488af84e88486d9679468a3d9677a) ) // mask ROM
	ROM_LOAD( "b36-02.ic12",  0x80000, 0x80000, CRC(e71da5db) SHA1(aa47ae02c359264c0a1f09ecc583eefd1ef1dfa4) ) // mask ROM

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "b36-03.ic28",  0x00000, 0x80000, CRC(96166516) SHA1(a6748218188cbd1b037f6c0845416665c0d55a7b) ) // mask ROM

	ROM_REGION( 0x022e, "plds", 0 )
	ROM_LOAD( "b36-04.ic3",     0x0000, 0x0117, CRC(59847b86) SHA1(8a861cc0eeb3ea5f39b7fd4d4b1e44c3555dc2da) ) // PAL16L8 - bruteforced
	ROM_LOAD( "b36-05.ic11",    0x0117, 0x0117, CRC(57342384) SHA1(549ac36668692b5839f59d6915712c48240af21e) ) // PAL16L8 - bruteforced
ROM_END

ROM_START( raimaisj )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "b36_08-1.ic7", 0x00000, 0x20000, CRC(6cc8f79f) SHA1(17b4903f87e6d5447c8557c2baca1728f86245dc) )
	ROM_LOAD( "b36_09.ic13",  0x20000, 0x20000, CRC(9c466e43) SHA1(2466a3f1f8124323008c9925f90e9a1d2edf1564) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b36_06.ic24",  0x00000, 0x10000, CRC(29bbc4f8) SHA1(39a68729c6180c5f6cdf604e692018e7d6bf5591) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "b36_07.ic2",   0x00000, 0x10000, CRC(4f3737e6) SHA1(ff5f5d4ca5485441d03c8cb01a6a096941ab02eb) )

	ROM_REGION( 0x100000, "maincpu:gfx", 0 )
	ROM_LOAD( "b36-01.ic6",   0x00000, 0x80000, CRC(89355cb2) SHA1(433e929fe8b488af84e88486d9679468a3d9677a) ) // mask ROM
	ROM_LOAD( "b36-02.ic12",  0x80000, 0x80000, CRC(e71da5db) SHA1(aa47ae02c359264c0a1f09ecc583eefd1ef1dfa4) ) // mask ROM

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "b36-03.ic28",  0x00000, 0x80000, CRC(96166516) SHA1(a6748218188cbd1b037f6c0845416665c0d55a7b) ) // mask ROM

	ROM_REGION( 0x022e, "plds", 0 )
	ROM_LOAD( "b36-04.ic3",     0x0000, 0x0117, CRC(59847b86) SHA1(8a861cc0eeb3ea5f39b7fd4d4b1e44c3555dc2da) ) // PAL16L8 - bruteforced
	ROM_LOAD( "b36-05.ic11",    0x0117, 0x0117, CRC(57342384) SHA1(549ac36668692b5839f59d6915712c48240af21e) ) // PAL16L8 - bruteforced
ROM_END

ROM_START( raimaisjo )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "b36_08.ic7",   0x00000, 0x20000, CRC(f40b9178) SHA1(ccf5afcf08cac0d5b2d6ba74abd62d35412f0265) )
	ROM_LOAD( "b36_09.ic13",  0x20000, 0x20000, CRC(9c466e43) SHA1(2466a3f1f8124323008c9925f90e9a1d2edf1564) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b36_06.ic24",  0x00000, 0x10000, CRC(29bbc4f8) SHA1(39a68729c6180c5f6cdf604e692018e7d6bf5591) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "b36_07.ic2",   0x00000, 0x10000, CRC(4f3737e6) SHA1(ff5f5d4ca5485441d03c8cb01a6a096941ab02eb) )

	ROM_REGION( 0x100000, "maincpu:gfx", 0 )
	ROM_LOAD( "b36-01.ic6",   0x00000, 0x80000, CRC(89355cb2) SHA1(433e929fe8b488af84e88486d9679468a3d9677a) ) // mask ROM
	ROM_LOAD( "b36-02.ic12",  0x80000, 0x80000, CRC(e71da5db) SHA1(aa47ae02c359264c0a1f09ecc583eefd1ef1dfa4) ) // mask ROM

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "b36-03.ic28",  0x00000, 0x80000, CRC(96166516) SHA1(a6748218188cbd1b037f6c0845416665c0d55a7b) ) // mask ROM

	ROM_REGION( 0x022e, "plds", 0 )
	ROM_LOAD( "b36-04.ic3",     0x0000, 0x0117, CRC(59847b86) SHA1(8a861cc0eeb3ea5f39b7fd4d4b1e44c3555dc2da) ) // PAL16L8 - bruteforced
	ROM_LOAD( "b36-05.ic11",    0x0117, 0x0117, CRC(57342384) SHA1(549ac36668692b5839f59d6915712c48240af21e) ) // PAL16L8 - bruteforced
ROM_END

ROM_START( fhawk )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "b70-11.ic3", 0x00000, 0x20000, CRC(7d9f7583) SHA1(d8fa7c66a81fb356fa9c72f377bfc31b1837eafb) )
	ROM_LOAD( "b70-03.ic2", 0x20000, 0x80000, CRC(42d5a9b8) SHA1(10714fe95c372cec12376e615a9abe213aff12bc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b70-09.ic31", 0x00000, 0x10000, CRC(85cccaa2) SHA1(5459cd8df9d94e1938008cfc17d4ebac98004bfc) )

	ROM_REGION( 0x20000, "slave", 0 )
	ROM_LOAD( "b70-08.ic12", 0x00000, 0x20000, CRC(4d795f48) SHA1(58040d8ccbd0572cf6aef6ea9dd646b9338a03a0) )

	ROM_REGION( 0x100000, "maincpu:gfx", 0 )
	ROM_LOAD( "b70-01.ic1", 0x00000, 0x80000, CRC(fcdf67e2) SHA1(08a6a04a45c4adb4f5b4b0b83e90b2e5fe5cb0b1) )
	ROM_LOAD( "b70-02.ic2", 0x80000, 0x80000, CRC(35f7172e) SHA1(f257e9db470bb6dcca491b89cb666ef6d2546887) )
ROM_END

ROM_START( fhawkj )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "b70-07.ic3", 0x00000, 0x20000, CRC(939114af) SHA1(66218536dcb3b34ffa01d3c9c2fee365d91cfe00) )
	ROM_LOAD( "b70-03.ic2", 0x20000, 0x80000, CRC(42d5a9b8) SHA1(10714fe95c372cec12376e615a9abe213aff12bc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b70-09.ic31", 0x00000, 0x10000, CRC(85cccaa2) SHA1(5459cd8df9d94e1938008cfc17d4ebac98004bfc) )

	ROM_REGION( 0x20000, "slave", 0 )
	ROM_LOAD( "b70-08.ic12", 0x00000, 0x20000, CRC(4d795f48) SHA1(58040d8ccbd0572cf6aef6ea9dd646b9338a03a0) )

	ROM_REGION( 0x100000, "maincpu:gfx", 0 )
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

	ROM_REGION( 0x180000, "maincpu:gfx", 0 )
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

	ROM_REGION( 0x180000, "maincpu:gfx", 0 )
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

	ROM_REGION( 0x180000, "maincpu:gfx", 0 )
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

	ROM_REGION( 0x100000, "maincpu:gfx", 0 )
	ROM_LOAD( "b42-01.ic1",  0x00000, 0x80000, CRC(7d1a1fec) SHA1(28311b07673686c18988400d0254533a454f07f4) )
	ROM_LOAD( "b42-02.ic5",  0x80000, 0x80000, CRC(1a52e65c) SHA1(20a1fc4d02b5928fb01444079692e23d178c6297) )

	ROM_REGION( 0x022e, "plds", 0 )
	ROM_LOAD( "gal16v8-b42-03.ic4.bin",  0x0000, 0x0117, CRC(f7150d37) SHA1(10f9190b89c802e0722fd6ba0f17ba97d463f434) )  /* derived, but verified, PAL16L8 Stamped B42-03 */
	ROM_LOAD( "gal16v8-b42-04.ic21.bin", 0x0117, 0x0117, CRC(b57b806c) SHA1(04cbb008256b7317ebf366327dfd38ead8eaf94e) )  /* derived, but verified, PAL16L8 Stamped B42-04 */
ROM_END

ROM_START( kurikintw )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "b42-10.ic2",  0x00000, 0x20000, CRC(87460109) SHA1(78d0726f5d344673828191bf2e56e9741e977350) )
	ROM_LOAD( "b42-06.ic6",  0x20000, 0x20000, CRC(fa15fd65) SHA1(a810d7315878212e4e5344a24addf117ea6baeab) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b42-07.ic22", 0x00000, 0x10000, CRC(0f2719c0) SHA1(f870335a75f236f0059522f9a577dee7ca3acb2f) )

	ROM_REGION( 0x100000, "maincpu:gfx", 0 )
	ROM_LOAD( "b42-01.ic1",  0x00000, 0x80000, CRC(7d1a1fec) SHA1(28311b07673686c18988400d0254533a454f07f4) )
	ROM_LOAD( "b42-02.ic5",  0x80000, 0x80000, CRC(1a52e65c) SHA1(20a1fc4d02b5928fb01444079692e23d178c6297) )

	ROM_REGION( 0x022e, "plds", 0 )
	ROM_LOAD( "gal16v8-b42-03.ic4.bin",  0x0000, 0x0117, CRC(f7150d37) SHA1(10f9190b89c802e0722fd6ba0f17ba97d463f434) )  /* derived, but verified, PAL16L8 Stamped B42-03 */
	ROM_LOAD( "gal16v8-b42-04.ic21.bin", 0x0117, 0x0117, CRC(b57b806c) SHA1(04cbb008256b7317ebf366327dfd38ead8eaf94e) )  /* derived, but verified, PAL16L8 Stamped B42-04 */
ROM_END

ROM_START( kurikintu )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "b42-08.ic2",  0x00000, 0x20000, CRC(7075122e) SHA1(55f5f0cf3b91b7b408f9c05c91f9839c43b49c5f) )
	ROM_LOAD( "b42-06.ic6",  0x20000, 0x20000, CRC(fa15fd65) SHA1(a810d7315878212e4e5344a24addf117ea6baeab) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b42-07.ic22", 0x00000, 0x10000, CRC(0f2719c0) SHA1(f870335a75f236f0059522f9a577dee7ca3acb2f) )

	ROM_REGION( 0x100000, "maincpu:gfx", 0 )
	ROM_LOAD( "b42-01.ic1",  0x00000, 0x80000, CRC(7d1a1fec) SHA1(28311b07673686c18988400d0254533a454f07f4) )
	ROM_LOAD( "b42-02.ic5",  0x80000, 0x80000, CRC(1a52e65c) SHA1(20a1fc4d02b5928fb01444079692e23d178c6297) )

	ROM_REGION( 0x022e, "plds", 0 )
	ROM_LOAD( "gal16v8-b42-03.ic4.bin",  0x0000, 0x0117, CRC(f7150d37) SHA1(10f9190b89c802e0722fd6ba0f17ba97d463f434) )  /* derived, but verified, PAL16L8 Stamped B42-03 */
	ROM_LOAD( "gal16v8-b42-04.ic21.bin", 0x0117, 0x0117, CRC(b57b806c) SHA1(04cbb008256b7317ebf366327dfd38ead8eaf94e) )  /* derived, but verified, PAL16L8 Stamped B42-04 */
ROM_END

ROM_START( kurikintj )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "b42-05.ic2",  0x00000, 0x20000, CRC(077222b8) SHA1(953fb3444f6bb0dbe0323a0fd8fc3067b106a4f6) )
	ROM_LOAD( "b42-06.ic6",  0x20000, 0x20000, CRC(fa15fd65) SHA1(a810d7315878212e4e5344a24addf117ea6baeab) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b42-07.ic22", 0x00000, 0x10000, CRC(0f2719c0) SHA1(f870335a75f236f0059522f9a577dee7ca3acb2f) )

	ROM_REGION( 0x100000, "maincpu:gfx", 0 )
	ROM_LOAD( "b42-01.ic1",  0x00000, 0x80000, CRC(7d1a1fec) SHA1(28311b07673686c18988400d0254533a454f07f4) )
	ROM_LOAD( "b42-02.ic5",  0x80000, 0x80000, CRC(1a52e65c) SHA1(20a1fc4d02b5928fb01444079692e23d178c6297) )

	ROM_REGION( 0x022e, "plds", 0 )
	ROM_LOAD( "gal16v8-b42-03.ic4.bin",  0x0000, 0x0117, CRC(f7150d37) SHA1(10f9190b89c802e0722fd6ba0f17ba97d463f434) )  /* derived, but verified, PAL16L8 Stamped B42-03 */
	ROM_LOAD( "gal16v8-b42-04.ic21.bin", 0x0117, 0x0117, CRC(b57b806c) SHA1(04cbb008256b7317ebf366327dfd38ead8eaf94e) )  /* derived, but verified, PAL16L8 Stamped B42-04 */
ROM_END

ROM_START( kurikinta )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "kk_ic2.ic2",  0x00000, 0x20000, CRC(908603f2) SHA1(f810f2501458224e9264a984f22547cc8ccc2b0e) )
	ROM_LOAD( "kk_ic6.ic6",  0x20000, 0x20000, CRC(a4a957b1) SHA1(bbdb5b71ab613a8c89f7a0300abd85408951dc7e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b42-07.ic22", 0x00000, 0x10000, CRC(0f2719c0) SHA1(f870335a75f236f0059522f9a577dee7ca3acb2f) )

	ROM_REGION( 0x100000, "maincpu:gfx", 0 )
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

	ROM_REGION( 0x20000, "maincpu:gfx", 0 )
	ROM_LOAD16_BYTE( "b96-07.ic9", 0x00000, 0x10000, CRC(0713a387) SHA1(0fc1242ce02a56279fa1d5270c905bba7cdcd072) )
	ROM_LOAD16_BYTE( "b96-08.ic8", 0x00001, 0x10000, CRC(55b8e294) SHA1(14405638f751adfadb022bf7a0123a3972d4a617) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8-b86-04.bin", 0x0000, 0x0117, CRC(bf8c0ea0) SHA1(e0a00f1f6363fb79650202f90a56329990876d49) )  /* derived, but verified  Pal Stamped B86-04 */
ROM_END


ROM_START( plottinga ) /* B96-09 or higher by Taito's rom numbering system, demo mode is 2 players */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "plot01.ic10", 0x00000, 0x10000, CRC(5b30bc25) SHA1(df8839a90da9e5122d75b6faaf97f59499dbd316) )

	ROM_REGION( 0x20000, "maincpu:gfx", 0 )
	ROM_LOAD16_BYTE( "b96-02.ic9", 0x00000, 0x10000, CRC(6e0bad2a) SHA1(73996688cd058a2f56f61ea60144b9c673919a58) )
	ROM_LOAD16_BYTE( "b96-03.ic8", 0x00001, 0x10000, CRC(fb5f3ca4) SHA1(0c335acceea50133a6899f9e368cff5f61b55a96) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8-b86-04.bin", 0x0000, 0x0117, CRC(bf8c0ea0) SHA1(e0a00f1f6363fb79650202f90a56329990876d49) )  /* derived, but verified  Pal Stamped B86-04 */
ROM_END

ROM_START( plottingb ) /* The first (earliest) "World" version by Taito's rom numbering system, demo mode is 2 players */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b96-06.ic10",0x00000, 0x10000, CRC(f89a54b1) SHA1(19757b5fb61acdd6f5ae8e32a38ae54bfda0c522) )

	ROM_REGION( 0x20000, "maincpu:gfx", 0 )
	ROM_LOAD16_BYTE( "b96-02.ic9", 0x00000, 0x10000, CRC(6e0bad2a) SHA1(73996688cd058a2f56f61ea60144b9c673919a58) )
	ROM_LOAD16_BYTE( "b96-03.ic8", 0x00001, 0x10000, CRC(fb5f3ca4) SHA1(0c335acceea50133a6899f9e368cff5f61b55a96) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8-b86-04.bin", 0x0000, 0x0117, CRC(bf8c0ea0) SHA1(e0a00f1f6363fb79650202f90a56329990876d49) )  /* derived, but verified  Pal Stamped B86-04 */
ROM_END

ROM_START( plottingu ) /* The demo mode is 2 players */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b96-05.ic10",0x00000, 0x10000, CRC(afb99d1f) SHA1(a5cabc182d4f1d5709e6835d8b0a481dd0f9a563) )

	ROM_REGION( 0x20000, "maincpu:gfx", 0 )
	ROM_LOAD16_BYTE( "b96-02.ic9", 0x00000, 0x10000, CRC(6e0bad2a) SHA1(73996688cd058a2f56f61ea60144b9c673919a58) )
	ROM_LOAD16_BYTE( "b96-03.ic8", 0x00001, 0x10000, CRC(fb5f3ca4) SHA1(0c335acceea50133a6899f9e368cff5f61b55a96) )

	ROM_REGION( 0x0200, "plds", 0 ) // PAL16L8
	ROM_LOAD( "b96-04.ic12", 0x0000, 0x0104, CRC(9390a782) SHA1(9e68948ed15d96c1998e5d5cd99b823676e555e7) )  /* Confirmed/Matches U.S. set */
ROM_END

ROM_START( flipull ) /* The demo mode is 1 player */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b96-01.ic10",0x00000, 0x10000, CRC(65993978) SHA1(d14dc70f1b5e72b96ccc3fab61d7740f627bfea2) )

	ROM_REGION( 0x20000, "maincpu:gfx", 0 )
	ROM_LOAD16_BYTE( "b96-07.ic9", 0x00000, 0x10000, CRC(0713a387) SHA1(0fc1242ce02a56279fa1d5270c905bba7cdcd072) )
	ROM_LOAD16_BYTE( "b96-08.ic8", 0x00001, 0x10000, CRC(55b8e294) SHA1(14405638f751adfadb022bf7a0123a3972d4a617) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8-b86-04.bin", 0x0000, 0x0117, CRC(bf8c0ea0) SHA1(e0a00f1f6363fb79650202f90a56329990876d49) )  /* derived, but verified  Pal Stamped B86-04 */
ROM_END

ROM_START( puzznic )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "c20-09.ic11", 0x00000, 0x20000, CRC(156d6de1) SHA1(c247936b62ef354851c9bace76a7a0aa14194d5f) )

	ROM_REGION( 0x0800, "mcu:mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "c20-01.ic4", 0x0000, 0x0800, CRC(085f68b4) SHA1(2dbc7e2c015220dc59ee1f1208540744e5b9b7cc) )

	ROM_REGION( 0x20000, "maincpu:gfx", 0 )
	ROM_LOAD16_BYTE( "c20-07.ic10", 0x00000, 0x10000, CRC(be12749a) SHA1(c67d1a434486843a6776d89e905362b7db595d8d) )
	ROM_LOAD16_BYTE( "c20-06.ic9",  0x00001, 0x10000, CRC(ac85a9c5) SHA1(2d72dae86a191ccdac9648980aca832fb9886544) )

	ROM_REGION( 0x0800, "pals", 0 )
	ROM_LOAD( "mmipal20l8.ic3", 0x0000, 0x0800, NO_DUMP )
ROM_END

ROM_START( puzznicu )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "c20-10.ic11", 0x00000, 0x20000, CRC(3522d2e5) SHA1(2428663d1d71f2920c69cd2cd907f0750c22af77) )

	ROM_REGION( 0x0800, "mcu:mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "c20-01.ic4", 0x0000, 0x0800, CRC(085f68b4) SHA1(2dbc7e2c015220dc59ee1f1208540744e5b9b7cc) )

	ROM_REGION( 0x40000, "maincpu:gfx", 0 )
	ROM_LOAD16_BYTE( "c20-03.ic10",  0x00000, 0x20000, CRC(4264056c) SHA1(d2d8a170ae0f361093a5384935238605a59e5938) )
	ROM_LOAD16_BYTE( "c20-02.ic9",   0x00001, 0x20000, CRC(3c115f8b) SHA1(8d518be01b7c4d6d993d5d9b62aab719a5c8baca) )

	ROM_REGION( 0x0800, "pals", 0 )
	ROM_LOAD( "mmipal20l8.ic3", 0x0000, 0x0800, NO_DUMP )
ROM_END

ROM_START( puzznicj )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "c20-04.ic11",  0x00000, 0x20000, CRC(a4150b6c) SHA1(27719b8993735532cd59f4ed5693ff3143ee2336) )

	ROM_REGION( 0x0800, "mcu:mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "c20-01.ic4", 0x0000, 0x0800, CRC(085f68b4) SHA1(2dbc7e2c015220dc59ee1f1208540744e5b9b7cc) )

	ROM_REGION( 0x40000, "maincpu:gfx", 0 )
	ROM_LOAD16_BYTE( "c20-03.ic10",  0x00000, 0x20000, CRC(4264056c) SHA1(d2d8a170ae0f361093a5384935238605a59e5938) )
	ROM_LOAD16_BYTE( "c20-02.ic9",   0x00001, 0x20000, CRC(3c115f8b) SHA1(8d518be01b7c4d6d993d5d9b62aab719a5c8baca) )

	ROM_REGION( 0x0200, "pals", 0 ) // PAL20L8
	ROM_LOAD( "c20-05.ic3", 0x0000, 0x0144, CRC(f90e5594) SHA1(6181bb25b77028bb150c84bdc073f0457efd7eaa) ) // Confirmed/Matches Japan Set
ROM_END

ROM_START( puzznici ) /* bootleg (original main board, bootleg sub-board without MCU) */
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1.ic11",  0x00000, 0x20000, CRC(4612f5e0) SHA1(dc07a365414666568537d31ef01b58f2362cadaf) )

	ROM_REGION( 0x40000, "maincpu:gfx", 0 )
	ROM_LOAD16_BYTE( "u10.ic10",  0x00000, 0x20000, CRC(4264056c) SHA1(d2d8a170ae0f361093a5384935238605a59e5938) )
	ROM_LOAD16_BYTE( "3.ic9",     0x00001, 0x20000, CRC(2bf5232a) SHA1(a8fc06bb8bae2ca6bd21e3a96c9ed38bb356d5d7) )
ROM_END

ROM_START( puzznicb ) /* bootleg (original main board, bootleg sub-board without MCU) */
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ic11.bin",  0x00000, 0x20000, CRC(2510df4d) SHA1(534327e3d7f847b6c0effc5fd0fb9f5da9b0d3b1) )

	ROM_REGION( 0x20000, "maincpu:gfx", 0 ) // this has the bad line in tile 1 fixed (unused I believe) are we sure the roms used in the original sets are a good dump?
	ROM_LOAD16_BYTE( "ic10.bin",  0x00000, 0x10000, CRC(be12749a) SHA1(c67d1a434486843a6776d89e905362b7db595d8d) )
	ROM_LOAD16_BYTE( "ic9.bin",   0x00001, 0x10000, CRC(0f183340) SHA1(9eef7de801eb9763313f55a38e567b92fca3bfa6) )
ROM_END

ROM_START( puzznicba ) /* bootleg (original main board, bootleg sub-board without MCU) - marked PUZZNIC-2 008900 42 */
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "18.ic10",  0x00000, 0x20000, CRC(8349eb3b) SHA1(589dc99a22b3d7623b1ea6c1053f3b3dfe520547) )

	ROM_REGION( 0x40000, "maincpu:gfx", 0 )
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

	ROM_REGION( 0x80000, "maincpu:gfx", 0 )
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

ROM_START( palamed ) /* Prototype or location test?? - Line 5 of notice screen says "Territory" later sets say "Territories" */
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "palamedes_prg_ic6.ic6", 0x00000, 0x20000, CRC(ee957b0e) SHA1(cca9db673026f769776cb86734a6503692676fbe) ) /* hand labeled as PALAMEDEStm [PRG] IC6 */

	ROM_REGION( 0x40000, "maincpu:gfx", 0 )
	ROM_LOAD16_BYTE( "chr-l_ic9.ic9", 0x00000, 0x20000, CRC(c7bbe460) SHA1(1c1f186d0b0b2e383f82c53ae93b975a75f50f9c) ) /* hand labeled as CHR-L IC9 */
	ROM_LOAD16_BYTE( "chr-h_ic7.ic7", 0x00001, 0x20000, CRC(fcd86e44) SHA1(bdd0750ed6e93cc49f09f4ccb05b0c4a44cb9c23) ) /* hand labeled as CHR-H IC7 */
ROM_END

ROM_START( palamedj )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "c63-02.ic6", 0x00000, 0x20000, CRC(55a82bb2) SHA1(f157ad770351d4b8d8f8c061c4e330d6391fc624) )

	ROM_REGION( 0x40000, "maincpu:gfx", 0 )
	ROM_LOAD16_BYTE( "c63-04.ic9", 0x00000, 0x20000, CRC(c7bbe460) SHA1(1c1f186d0b0b2e383f82c53ae93b975a75f50f9c) )
	ROM_LOAD16_BYTE( "c63-03.ic7", 0x00001, 0x20000, CRC(fcd86e44) SHA1(bdd0750ed6e93cc49f09f4ccb05b0c4a44cb9c23) )
ROM_END

ROM_START( cachat )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cac6",  0x00000, 0x20000, CRC(8105cf5f) SHA1(e6dd22165436c247db887a04c3e69c9e2505bb33) )

	ROM_REGION( 0x80000, "maincpu:gfx", 0 )
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

	ROM_REGION( 0x80000, "maincpu:gfx", 0 )
	ROM_LOAD16_BYTE( "t-i_03.7", 0x00001, 0x40000, CRC(e1c3fed0) SHA1(cd68dbf61ed820f4aa50c630e7cb778aafb433c2) )
	ROM_LOAD16_BYTE( "t-i_04.9", 0x00000, 0x40000, CRC(b4a6e31d) SHA1(e9abab8f19c78207f25a62104bcae1e391cbd2c0) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal20l8b-c63-01.14", 0x0000, 0x0144, CRC(14a7dd2a) SHA1(2a39ca6069bdac553d73c34db6f50f880559113c) )
ROM_END

ROM_START( cubybop )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "cb06.6", 0x00000, 0x40000, CRC(66b89a85) SHA1(2ba26d71fd1aa8e64584a5908a1d797666718d49) )

	ROM_REGION( 0x100000, "maincpu:gfx", 0 )
	ROM_LOAD16_BYTE( "cb09.9",  0x00000, 0x40000, CRC(5f831e59) SHA1(db319a6c1058200274d687163b4df2f78a2bf879) )
	ROM_LOAD16_BYTE( "cb10.10", 0x80000, 0x40000, CRC(430510fc) SHA1(95c0a0ebd0485a15090f302e5d2f4da8204baf7c) )
	ROM_LOAD16_BYTE( "cb07.7",  0x00001, 0x40000, CRC(3582de99) SHA1(51620cc9044aef8e5ed0335b7d5d6d67a7857005) )
	ROM_LOAD16_BYTE( "cb08.8",  0x80001, 0x40000, CRC(09e18a51) SHA1(18db47d1d84f9be892bc796116c7ef7d0c1ee59f) )
ROM_END

ROM_START( plgirls )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "pg03.ic6", 0x00000, 0x40000, CRC(6ca73092) SHA1(f5679f047a29b936046c0d3677489df553ad7b41) )

	ROM_REGION( 0x80000, "maincpu:gfx", 0 )
	ROM_LOAD16_BYTE( "pg02.ic9", 0x00000, 0x40000, CRC(3cf05ca9) SHA1(502c45a5330dda1b2fbf7d3d0c9bc6e889ff07d8) )
	ROM_LOAD16_BYTE( "pg01.ic7", 0x00001, 0x40000, CRC(79e41e74) SHA1(aa8efbeeee47f84e19b639821a89a7bcd67fe7a9) )
ROM_END

ROM_START( plgirls2 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "pg2_1j.ic6", 0x00000, 0x40000, CRC(f924197a) SHA1(ecaaefd1b3715ba60608e05d58be67e3c71f653a) )

	ROM_REGION( 0x100000, "maincpu:gfx", 0 )
	ROM_LOAD16_BYTE( "cho-l.ic9",  0x00000, 0x80000, CRC(956384ec) SHA1(94a2b95f340e96bdccbeafd373f0dea90b8328dd) )
	ROM_LOAD16_BYTE( "cho-h.ic7",  0x00001, 0x80000, CRC(992f99b1) SHA1(c79f1014d73654740f7823812f92376d65d6b15d) )
ROM_END

ROM_START( plgirls2b )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "playgirls2b.d1", 0x00000, 0x40000, CRC(d58159fa) SHA1(541c6ca5f12c38b5a08f90048f52c31d27bb9233) )

	ROM_REGION( 0x100000, "maincpu:gfx", 0 )
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

	ROM_REGION( 0x100000, "maincpu:gfx", 0 )
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

	ROM_REGION( 0x80000, "maincpu:gfx", 0 )
	ROM_LOAD32_BYTE( "rom2",   0x00003, 0x20000, CRC(4c739a30) SHA1(4426f51aac9bb39f5d1a7616d183ff6c76749dc2) )
	ROM_LOAD32_BYTE( "rom3",   0x00001, 0x20000, CRC(4cf22a4b) SHA1(1c933ccbb6a5b8a6795385d7970db5f7138e572e) )
	ROM_LOAD32_BYTE( "rom4",   0x00002, 0x20000, CRC(7dcd6696) SHA1(8f3b1fe669520142668af6dc2d04f13767048989) )
	ROM_LOAD32_BYTE( "rom5",   0x00000, 0x20000, CRC(b1782816) SHA1(352663974886e1e4358e55b87c8bf0cdb979f177) )
ROM_END



// bits 7..0 => bits 0..7
void taitol_1cpu_state::init_plottinga()
{
	u8 tab[256];
	u8 *RAM = m_main_prg->base();

	for (unsigned i = 0; i < sizeof(tab); i++)
		tab[i] = bitswap<8>(i, 0, 1, 2, 3, 4, 5, 6, 7);

	for (unsigned i = 0; i < m_main_prg->bytes(); i++)
		RAM[i] = tab[RAM[i]];
}


GAME( 1988, raimais,   0,        raimais,   raimais,   taitol_2cpu_state, empty_init,     ROT0,   "Taito Corporation Japan", "Raimais (World, rev 1)", 0 )
GAME( 1988, raimaisj,  raimais,  raimais,   raimaisj,  taitol_2cpu_state, empty_init,     ROT0,   "Taito Corporation", "Raimais (Japan, rev 1)", 0 )
GAME( 1988, raimaisjo, raimais,  raimais,   raimaisj,  taitol_2cpu_state, empty_init,     ROT0,   "Taito Corporation", "Raimais (Japan)", 0 )

GAME( 1988, fhawk,     0,        fhawk,     fhawk,     fhawk_state,       empty_init,     ROT270, "Taito Corporation Japan", "Fighting Hawk (World)", 0 )
GAME( 1988, fhawkj,    fhawk,    fhawk,     fhawkj,    fhawk_state,       empty_init,     ROT270, "Taito Corporation", "Fighting Hawk (Japan)", 0 )

GAME( 1989, champwr,   0,        champwr,   champwr,   champwr_state,     empty_init,     ROT0,   "Taito Corporation Japan", "Champion Wrestler (World)", MACHINE_IMPERFECT_SOUND )
GAME( 1989, champwru,  champwr,  champwr,   champwru,  champwr_state,     empty_init,     ROT0,   "Taito America Corporation", "Champion Wrestler (US)", MACHINE_IMPERFECT_SOUND )
GAME( 1989, champwrj,  champwr,  champwr,   champwrj,  champwr_state,     empty_init,     ROT0,   "Taito Corporation", "Champion Wrestler (Japan)", MACHINE_IMPERFECT_SOUND )

GAME( 1988, kurikint,  0,        kurikint,  kurikint,  taitol_2cpu_state, empty_init,     ROT0,   "Taito Corporation Japan", "Kuri Kinton (World)", 0 )
GAME( 1988, kurikintw, kurikint, kurikint,  kurikintj, taitol_2cpu_state, empty_init,     ROT0,   "Taito Corporation (World Games, Inc. license)", "Kuri Kinton (US, World Games license)", 0 )
GAME( 1988, kurikintu, kurikint, kurikint,  kurikintj, taitol_2cpu_state, empty_init,     ROT0,   "Taito America Corporation", "Kuri Kinton (US)", 0 )
GAME( 1988, kurikintj, kurikint, kurikint,  kurikintj, taitol_2cpu_state, empty_init,     ROT0,   "Taito Corporation", "Kuri Kinton (Japan)", 0 )
GAME( 1988, kurikinta, kurikint, kurikint,  kurikinta, taitol_2cpu_state, empty_init,     ROT0,   "Taito Corporation Japan", "Kuri Kinton (World, prototype?)", 0 )

GAME( 1989, plotting,  0,        plotting,  plotting,  taitol_1cpu_state, empty_init,     ROT0,   "Taito Corporation Japan", "Plotting (World set 1)", 0 )
GAME( 1989, plottinga, plotting, plotting,  plotting,  taitol_1cpu_state, init_plottinga, ROT0,   "Taito Corporation Japan", "Plotting (World set 2, protected)", 0 )
GAME( 1989, plottingb, plotting, plotting,  plotting,  taitol_1cpu_state, empty_init,     ROT0,   "Taito Corporation Japan", "Plotting (World set 3, earliest version)", 0 )
GAME( 1989, plottingu, plotting, plotting,  plottingu, taitol_1cpu_state, empty_init,     ROT0,   "Taito America Corporation", "Plotting (US)", 0 )
GAME( 1989, flipull,   plotting, plotting,  plotting,  taitol_1cpu_state, empty_init,     ROT0,   "Taito Corporation", "Flipull (Japan)", 0 )

GAME( 1989, puzznic,   0,        puzznic,   puzznic,   taitol_1cpu_state, empty_init,     ROT0,   "Taito Corporation Japan", "Puzznic (World)", 0 )
GAME( 1989, puzznicu,  puzznic,  puzznic,   puzznic,   taitol_1cpu_state, empty_init,     ROT0,   "Taito America Corporation", "Puzznic (US)", 0 )
GAME( 1989, puzznicj,  puzznic,  puzznic,   puzznic,   taitol_1cpu_state, empty_init,     ROT0,   "Taito Corporation", "Puzznic (Japan)", 0 )
GAME( 1989, puzznici,  puzznic,  puzznici,  puzznic,   taitol_1cpu_state, empty_init,     ROT0,   "bootleg", "Puzznic (Italian bootleg)", 0 )
GAME( 1989, puzznicb,  puzznic,  puzznici,  puzznic,   taitol_1cpu_state, empty_init,     ROT0,   "bootleg", "Puzznic (bootleg, set 1)", 0 )
GAME( 1989, puzznicba, puzznic,  puzznici,  puzznic,   taitol_1cpu_state, empty_init,     ROT0,   "bootleg", "Puzznic (bootleg, set 2)", 0 )

GAME( 1990, horshoes,  0,        horshoes,  horshoes,  horshoes_state,    empty_init,     ROT270, "Taito America Corporation", "American Horseshoes (US)", 0 )

GAME( 1990, palamed,   0,        palamed,   palamed,   taitol_1cpu_state, empty_init,     ROT0,   "Hot-B Co., Ltd.", "Palamedes (US)", 0 ) // Prototype or location test
GAME( 1990, palamedj,  palamed,  palamed,   palamed,   taitol_1cpu_state, empty_init,     ROT0,   "Taito Corporation", "Palamedes (Japan)", 0 )

GAME( 1993, cachat,    0,        cachat,    cachat,    taitol_1cpu_state, empty_init,     ROT0,   "Taito Corporation", "Cachat (Japan)", 0 )
GAME( 1993, tubeit,    cachat,   cachat,    tubeit,    taitol_1cpu_state, empty_init,     ROT0,   "bootleg", "Tube-It", 0 ) // No (c) message

GAME( 199?, cubybop,   0,        cachat,    cubybop,   taitol_1cpu_state, empty_init,     ROT0,   "Hot-B Co., Ltd.", "Cuby Bop (location test)", 0 ) // No (c) message, but Hot-B company logo in tile gfx

GAME( 1992, plgirls,   0,        cachat,    plgirls,   taitol_1cpu_state, empty_init,     ROT270, "Hot-B Co., Ltd.", "Play Girls", 0 )
GAME( 1992, lagirl,    plgirls,  cachat,    plgirls,   taitol_1cpu_state, empty_init,     ROT270, "bootleg", "LA Girl", 0 ) // bootleg hardware with changed title & backgrounds

GAME( 1993, plgirls2,  0,        cachat,    plgirls2,  taitol_1cpu_state, empty_init,     ROT270, "Hot-B Co., Ltd.", "Play Girls 2", 0 )
GAME( 1993, plgirls2b, plgirls2, cachat,    plgirls2,  taitol_1cpu_state, empty_init,     ROT270, "bootleg", "Play Girls 2 (bootleg)", MACHINE_IMPERFECT_GRAPHICS ) // bootleg hardware (regular Z80 etc. instead of TC0090LVC, but acts almost the same - scroll offset problems)

GAME( 1990, evilston,  0,        evilston,  evilston,  taitol_2cpu_state, empty_init,     ROT270, "Spacy Industrial, Ltd.", "Evil Stone", 0 ) // Taiwanese publisher, unknown Japanese developer

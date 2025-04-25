// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*******************************************

  Seta custom ST-0016 chip based games.
    driver by Tomasz Slanina

  this is for 'simple' games using the chip
  where the chip is providing the maincpu
  video, and sound functionality of the game
  rather than acting as a sub-cpu

********************************************

Todo:
- find NMI source, and NMI enable/disable (timer ? video hw ?)

Dips verified for Neratte Chu (nratechu) from manual
*/

#include "emu.h"

#include "st0016.h"

#include "cpu/v810/v810.h"
#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "sound/st0016.h"

#include "screen.h"
#include "speaker.h"


namespace {

class st0016_state : public driver_device
{
public:
	st0016_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_subcpu(*this, "sub"),
		m_screen(*this, "screen"),
		m_mainbank(*this, "mainbank"),
		m_system(*this, "SYSTEM"),
		m_dsw(*this, "DSW%u", 1U)
	{ }

	void st0016(machine_config &config) ATTR_COLD;
	void renju(machine_config &config) ATTR_COLD;
	void mayjinsn(machine_config &config) ATTR_COLD;

	void init_crownpkr() ATTR_COLD;
	void init_nratechu() ATTR_COLD;
	void init_mayjinsn() ATTR_COLD;
	void init_mayjisn2() ATTR_COLD;
	void init_renju() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	uint8_t m_mux_port;
	uint32_t m_latches[8];

	required_device<st0016_cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
	required_device<screen_device> m_screen;

	required_memory_bank m_mainbank;

	required_ioport m_system;
	required_ioport_array<2> m_dsw;

	uint8_t mux_r();
	void mux_select_w(uint8_t data);
	uint32_t latch32_r(offs_t offset);
	void latch32_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t latch8_r(offs_t offset);
	void latch8_w(offs_t offset, uint8_t data);

	void rom_bank_w(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void renju_mem(address_map &map) ATTR_COLD;
	void st0016_io(address_map &map) ATTR_COLD;
	void st0016_m2_io(address_map &map) ATTR_COLD;
	void st0016_mem(address_map &map) ATTR_COLD;
	void v810_mem(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Machine's structure ST0016
 *
 *************************************/

void st0016_state::machine_start()
{
	m_mainbank->configure_entries(0, 256, memregion("maincpu")->base(), 0x4000);

	m_mux_port = 0;
	std::fill(std::begin(m_latches), std::end(m_latches), 0);

	save_item(NAME(m_mux_port));
	save_item(NAME(m_latches));
}

void st0016_state::st0016_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xe000, 0xe7ff).ram();
	map(0xe800, 0xe87f).ram(); // common ram
	map(0xf000, 0xffff).ram(); // work ram
}

void st0016_state::renju_mem(address_map &map)
{
	st0016_mem(map);
	map(0x0000, 0x7fff).rom().region("maincpu", 0x200000);
}


uint8_t st0016_state::mux_r()
{
/*
    76543210
        xxxx - input port #2
    xxxx     - dip switches (2x8 bits) (multiplexed)
*/
	int retval = m_system->read() & 0x0f;

	switch(m_mux_port & 0x30)
	{
		case 0x00: retval |= ((m_dsw[0]->read() & 1) << 4) | ((m_dsw[0]->read() & 0x10) << 1)
								| ((m_dsw[1]->read() & 1) << 6) | ((m_dsw[1]->read() & 0x10) <<3); break;
		case 0x10: retval |= ((m_dsw[0]->read() & 2) << 3) | ((m_dsw[0]->read() & 0x20)   )
								| ((m_dsw[1]->read() & 2) << 5) | ((m_dsw[1]->read() & 0x20) <<2); break;
		case 0x20: retval |= ((m_dsw[0]->read() & 4) << 2) | ((m_dsw[0]->read() & 0x40) >> 1)
								| ((m_dsw[1]->read() & 4) << 4) | ((m_dsw[1]->read() & 0x40) <<1); break;
		case 0x30: retval |= ((m_dsw[0]->read() & 8) << 1) | ((m_dsw[0]->read() & 0x80) >> 2)
								| ((m_dsw[1]->read() & 8) << 3) | ((m_dsw[1]->read() & 0x80)    ); break;
	}

	return retval;
}

void st0016_state::mux_select_w(uint8_t data)
{
	m_mux_port = data;
}

void st0016_state::rom_bank_w(uint8_t data)
{
	m_mainbank->set_entry(data);
}

void st0016_state::st0016_io(address_map &map)
{
	map.global_mask(0xff);
	map(0xc0, 0xc0).portr("P1").w(FUNC(st0016_state::mux_select_w));
	map(0xc1, 0xc1).portr("P2").nopw();
	map(0xc2, 0xc2).r(FUNC(st0016_state::mux_r)).nopw();
	map(0xc3, 0xc3).portr("P2").nopw();
	map(0xe0, 0xe0).nopw(); // renju = $40, neratte = 0
	map(0xe1, 0xe1).w(FUNC(st0016_state::rom_bank_w));
	map(0xe6, 0xe6).nopw(); // banking ? ram bank ? shared rambank ?
	map(0xe7, 0xe7).nopw(); // watchdog
}


/*************************************
 *
 *  Machine's structure ST0016 + V810
 *
 *************************************/

uint32_t st0016_state::latch32_r(offs_t offset)
{
	if(!offset)
		m_latches[2] &= ~2;
	return m_latches[offset];
}

void st0016_state::latch32_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if(!offset)
		m_latches[2] |= 1;
	COMBINE_DATA(&m_latches[offset]);
	machine().scheduler().synchronize();
}

uint8_t st0016_state::latch8_r(offs_t offset)
{
	if(!offset)
		m_latches[2] &= ~1;
	return m_latches[offset];
}

void st0016_state::latch8_w(offs_t offset, uint8_t data)
{
	if(!offset)
		m_latches[2] |= 2;
	m_latches[offset] = data;
	machine().scheduler().synchronize();
}

void st0016_state::v810_mem(address_map &map)
{
	map(0x00000000, 0x0001ffff).ram();
	map(0x80000000, 0x8001ffff).ram();
	map(0xc0000000, 0xc001ffff).ram();
	map(0x40000000, 0x4000000f).r(FUNC(st0016_state::latch32_r)).w(FUNC(st0016_state::latch32_w));
	map(0xfff80000, 0xffffffff).rom().region("sub", 0);
}

void st0016_state::st0016_m2_io(address_map &map)
{
	map.global_mask(0xff);
	map(0xc0, 0xc3).r(FUNC(st0016_state::latch8_r)).w(FUNC(st0016_state::latch8_w));
	map(0xd0, 0xd0).portr("P1").w(FUNC(st0016_state::mux_select_w));
	map(0xd1, 0xd1).portr("P2").nopw();
	map(0xd2, 0xd2).r(FUNC(st0016_state::mux_r)).nopw();
	map(0xd3, 0xd3).portr("P2").nopw();
	map(0xe0, 0xe0).nopw();
	map(0xe1, 0xe1).w(FUNC(st0016_state::rom_bank_w));
	map(0xe6, 0xe6).nopw(); // banking ? ram bank ? shared rambank ?
	map(0xe7, 0xe7).nopw(); // watchdog
}

/*************************************
 *
 *  Generic port definitions
 *
 *************************************/
static INPUT_PORTS_START( st0016 )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW)

	PORT_START("UNK")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused ?

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( renju )
	PORT_INCLUDE( st0016 )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW1") // Dip switch A
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_MODIFY("DSW2") // Dip switch B
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
INPUT_PORTS_END

static INPUT_PORTS_START( koikois )
	PORT_INCLUDE( st0016 )

	PORT_MODIFY("P1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW1") // Dip switch A
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Crt Mode" ) PORT_DIPLOCATION("SW1:2") // flip screen ?
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x40, 0x40,  DEF_STR( Controls ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "Majyan Panel" )
	PORT_DIPSETTING(    0x40, DEF_STR( Joystick ) )

	PORT_MODIFY("DSW2") // Dip switch B
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
INPUT_PORTS_END

static INPUT_PORTS_START( nratechu )
	PORT_INCLUDE( st0016 )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW1") // Dip switch A
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPNAME( 0x40, 0x40, "How To Play" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )

	PORT_MODIFY("DSW2") // Dip switch B
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2") //  speed / time..
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ))
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0C, 0x0c, "VS Round" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "First one to win" )
	PORT_DIPSETTING(    0x04, "Best 4 out of 7" )
	PORT_DIPSETTING(    0x08, "Best 3 out of 5" )
	PORT_DIPSETTING(    0x0C, "Best 2 out of 3" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6") // Manual has this Defaulted OFF
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( crownpkr )
	PORT_INCLUDE( st0016 )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME( "Hold 3 / Low" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) // TODO: hopper
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) // TODO: hopper full
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME( "Hold 2 / High" ) // also Meter Key in test mode
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) // also Reset Key in test mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) // Last Game Key in test mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(10) // TODO: simulate coin drop sensor
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW ) // only works if pressed during boot
INPUT_PORTS_END

static INPUT_PORTS_START( mayjisn2 )
	PORT_INCLUDE( st0016 )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW1") // Dip switch A
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x18, "Timer" ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, "6:00" )
	PORT_DIPSETTING(    0x08, "5:00" )
	PORT_DIPSETTING(    0x18, "4:00" )
	PORT_DIPSETTING(    0x10, "3:00" )

	PORT_MODIFY("DSW2") // Dip switch B
	PORT_DIPNAME( 0x18, 0x18, "Music in Game"  ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, "Remixed" )
	PORT_DIPSETTING(    0x18, "Only Intro" )
	PORT_DIPSETTING(    0x10, "Classic" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Position of Title" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, "B" )
	PORT_DIPSETTING(    0x40, "A" )
INPUT_PORTS_END


static INPUT_PORTS_START( gostop )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("OUT")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("WAIT")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )


	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Gift 1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Gift 2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("Gift 3")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM") // IP3 - bits 5-8
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("Reset")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED ) // don't show up in test menu


	PORT_START("UNK")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) // nothing?

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" ) // shows up as IP3 bit 3
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" ) // shows up as IP3 bit 2
	PORT_DIPNAME( 0x20, 0x20, "Init Ram" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("DSW2") // no dsw2 listed in test mode
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" ) // shows up as IP3 bit 1
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" ) // shows up as IP3 bit 0
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END


TIMER_DEVICE_CALLBACK_MEMBER(st0016_state::interrupt)
{
	int scanline = param;

	if (scanline == 240)
		m_maincpu->set_input_line(0, HOLD_LINE);
	else if ((scanline % 64) == 0)
		if (m_maincpu->state_int(Z80_IFF1)) // dirty hack ...
			m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}




/*************************************
 *
 *  Machine driver(s)
 *
 *************************************/

void st0016_state::st0016(machine_config &config)
{
	// basic machine hardware
	ST0016_CPU(config, m_maincpu, XTAL(48'000'000) / 6); // 8 MHz (48 MHz / 6) verified from nratechu (https://www.youtube.com/watch?v=scKF95t4-lU)
	m_maincpu->set_addrmap(AS_PROGRAM, &st0016_state::st0016_mem);
	m_maincpu->set_addrmap(AS_IO, &st0016_state::st0016_io);
	m_maincpu->set_screen(m_screen);

	TIMER(config, "scantimer").configure_scanline(FUNC(st0016_state::interrupt), "screen", 0, 1);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(48*8, 48*8);
	m_screen->set_visarea(0*8, 48*8-1, 0*8, 48*8-1);
	m_screen->set_screen_update(m_maincpu, FUNC(st0016_cpu_device::screen_update));
	m_screen->set_palette("maincpu:palette");

	// TODO: Mono?
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	m_maincpu->add_route(0, "lspeaker", 1.0);
	m_maincpu->add_route(1, "rspeaker", 1.0);
}

void st0016_state::mayjinsn(machine_config &config)
{
	st0016(config);

	m_maincpu->set_addrmap(AS_IO, &st0016_state::st0016_m2_io);

	V810(config, m_subcpu, 10000000); //25 Mhz ?
	m_subcpu->set_addrmap(AS_PROGRAM, &st0016_state::v810_mem);

	config.set_maximum_quantum(attotime::from_hz(60));
}

void st0016_state::renju(machine_config &config)
{
	st0016(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &st0016_state::renju_mem);
}

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

/*
KIRA KIRA 五目ならべ 連珠貴族 (KIRA KIRA Gomoku Narabe Renju Kizoku)
Visco, 1994

PCB Layout

E51-00001-A
|------------------------------------|
|AMP       UPD6376    424400  62256  |
|    VOL      RESET   424400  62256  |
|                                    |
| TD62064 74273                      |
|J        74273                      |
|A        74273            42.9545MHz|
|M                                   |
|M        74245        *ST-0016      |
|A        74245         TC6210AF     |
|         74245                      |
|         74245                 48MHz|
|                                    |
|  74138  74138                      |
|  74253  74253       RNJ2           |
|                                    |
|  DSW1   DSW2        RENJYU-1  6264 |
|------------------------------------|

Note:
 *ST-0016: was surface scratched to hide its identity on PCB E51-00001-A
           On similar PCBs, the chip was unaltered
*/

ROM_START( renju ) // PCB E51-00001-A
	ROM_REGION( 0x280000, "maincpu", 0 )
	ROM_LOAD( "renjyu-1.u31", 0x000000, 0x200000, CRC(e0fdbe9b) SHA1(52d31024d1a88b8fcca1f87366fcaf80e3c387a1) )
	ROM_LOAD( "rnj2.u32",     0x200000, 0x080000, CRC(2015289c) SHA1(5223b6d3dbe4657cd63cf5b527eaab84cf23587a) )
ROM_END

// ねらってチュー (Neratte Chū) / NERATTE CHU
ROM_START( nratechu ) // PCB E56-00002 (almost identical to above). "1.10 1996/05/25 21:05 Programming by ITEC" string
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "sx012-01.u31", 0x000000, 0x080000, CRC(6ca01d57) SHA1(065848f19ecf2dc1f7bbc7ddd87bca502e4b8b16) )
	ROM_LOAD( "sx012-02.u32", 0x100000, 0x100000, CRC(40a4e354) SHA1(8120ce8deee6805050a5b083a334c3743c09566b) )
	// U33 not populated
	// U34 not populated
ROM_END

ROM_START( crownpkr ) // PCB E56-00002. "1.20 1997/05/30 19:00 Programming by K&S string"
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "cpu120.u31", 0x000000, 0x080000, CRC(9c47920b) SHA1(bdc3f16cc7bf84102a24e6f58a1fb329bf90920b) ) // hand written label NEC D27C4000D / AMD 27C400
	// U32 not populated
	// U33 not populated
	// U34 not populated
ROM_END

ROM_START( dcrown ) // PCB E51-00001 (almost identical to above)
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "dc1.u31", 0x000000, 0x80000, CRC(e55200b8) SHA1(20a968dc895bb636b064c29b4b53c6ffa49fea36) )
	ROM_LOAD( "dc2.u32", 0x080000, 0x80000, CRC(05b6192f) SHA1(6af6e7b2c681f2791a7f89a528a95eb976c8ba84) )
	ROM_LOAD( "dc3.u33", 0x100000, 0x80000, CRC(f23c1975) SHA1(118d6054922a733d23363c53bb331d84c78e50ad) )
	ROM_LOAD( "dc4.u34", 0x180000, 0x80000, CRC(0d1c2c61) SHA1(7e4dc20ab683ce0f61dd939cfd9b17714ba2343a) )
ROM_END

ROM_START( dcrowna ) // PCB E51-00001 (almost identical to above)
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "dcn-0.u31", 0x000000, 0x080000, CRC(5dd0615d) SHA1(b859994bd79229da4c687deefe1997313724b26e) ) // U31 @ 1C
	ROM_LOAD( "dcn-1.u32", 0x080000, 0x080000, CRC(6c6f14e7) SHA1(2a3474e44420cc78e3ead777eb91481c4bb46eef) ) // U32 @ 1D
	ROM_LOAD( "dcn-2.u33", 0x100000, 0x080000, CRC(e9401a5e) SHA1(db24ebe5a0073c7c1c2da957772e223545f3c778) ) // U33 @ 1E
	ROM_LOAD( "dcn-3.u34", 0x180000, 0x080000, CRC(ec2e88bc) SHA1(2a8deee63e123dae411e2b834eca69be6f646d66) ) // U34 @ 1F
ROM_END

// 韓国花札 GO-STOP (Kankoku Hanafuda GO-STOP)
ROM_START( gostop )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "go-stop_rom1.u31",0x000000, 0x80000, CRC(93decaa2) SHA1(a30958b76dfe5752a341ecdc950119c10e864586) )
	ROM_LOAD( "go-stop_rom2.u32",0x080000, 0x80000, CRC(3c5402ff) SHA1(bdc38922b5cbad0150adf9c6cc0fefc5705a16a2) )
ROM_END


/*
恋こいしましょ - スーパーリアル花札 (Koi Koi Shimasho - Super Real Hanafuda)
Visco

PCB Layout
----------

E63-00001
|---------------------------------------|
|VOL     RESET     TC514400   62256     |
|        UPD6376   TC514400   62256     |
|MJM2904                                |
|                                       |
|M                            42.9545MHz|
|A                   |----------|       |
|H                   |          |       |
|J                   | ST-0016  |       |
|O                   |          |  48MHz|
|N                   |          |       |
|G                   |          |       |
|5                   |----------|       |
|6                          BATTERY     |
|                                       |
|                                       |
|                              KOI-5    |
|   KOI-4   KOI-2   KOI-3  KOI-1    6264|
|---------------------------------------|

*/

ROM_START( koikois )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "koi-2.6c", 0x000001, 0x080000, CRC(2722be71) SHA1(1aa3d819eef01db042ee04a01c1b18c4d9dae65e) )
	ROM_LOAD16_BYTE( "koi-1.4c", 0x000000, 0x080000, CRC(c79e2b43) SHA1(868174f7ab8e68e31d3302ae94dd742048deed9f) )
	ROM_LOAD16_BYTE( "koi-4.8c", 0x100001, 0x080000, CRC(ace236df) SHA1(4bf56affe5b6d0ba3cc677eaa91f9be77f26c654) )
	ROM_LOAD16_BYTE( "koi-3.5c", 0x100000, 0x080000, CRC(6fd88149) SHA1(87b1be32770232eb041e3ef9d1da45282af8a5d4) )
	ROM_LOAD(        "koi-5.2c", 0x200000, 0x200000, CRC(561e12c8) SHA1(a7aedf549bc3141fc01bc4a10c235af265ba4ee9) )
ROM_END


/*
Mayjinsen (JPN Ver.)
(c)1994 Seta

CPU:    UPD70732-25 V810 ?
Sound:  Custom (ST-0016 ?)

sx003.01    main prg
sx003.02
sx003.03
sx003.04    /

sx003.05d   chr
sx003.06
sx003.07d   /

-----------

Mayjinsen II
Seta, 1994

This game runs on Seta hardware. The game is similar to Shougi.

PCB Layout
----------

E52-00001
|----------------------------------------------------|
|                  62256    62256    62256    62256  |
| D70732GD-25                                        |
| NEC 1991 V810    62256    62256    62256    62256  |
|                                                    |
|                  62256    62256    62256    62256  |
|                                                    |
|                SX007-01 SX007-02  SX007-03 SX007-04|
|                                                    |
|                                   6264             |
|                                                    |
|                   62256      42.9545MHz  48MHz     |
|      PAL                                           |
|                   62256                            |
| 46MHz                          ST-0016   SX007-05  |
|                                TC6210AF            |
|                                                    |
|                   TC514800                         |
|                                                    |
|                                           DSW1-8   |
|                                                    |
|                                           DSW2-8   |
|                           JAMMA                    |
|----------------------------------------------------|
*/

ROM_START(mayjinsn )
	ROM_REGION( 0x180000, "maincpu", 0 )
	ROM_LOAD( "sx003.05d", 0x000000, 0x80000, CRC(2be6d620) SHA1(113db888fb657d45be55708bbbf9a9ac159a9636) )
	ROM_LOAD( "sx003.06",  0x080000, 0x80000, CRC(f0553386) SHA1(8915cb3ce03b9a12612694caec9bbec6de4dd070) )
	ROM_LOAD( "sx003.07d", 0x100000, 0x80000, CRC(8db281c3) SHA1(f8b488dd28010f01f789217a4d62ba2116e06e94) )

	ROM_REGION32_LE( 0x20000*4, "sub", 0 ) // V810 code
	ROM_LOAD32_BYTE( "sx003.04", 0x00003, 0x20000, CRC(fa15459f) SHA1(4163ab842943705c550f137abbdd2cb51ba5390f) )
	ROM_LOAD32_BYTE( "sx003.03", 0x00002, 0x20000, CRC(71a438ea) SHA1(613bab6a59aa1bced2ab37177c61a0fd7ce7e64f) )
	ROM_LOAD32_BYTE( "sx003.02", 0x00001, 0x20000, CRC(61911eed) SHA1(1442b3867b85120ba652ec8205d74332addffb67) )
	ROM_LOAD32_BYTE( "sx003.01", 0x00000, 0x20000, CRC(d210bfe5) SHA1(96d9f2b198d98125df4bd6b15705646d472a8a87) )
ROM_END

ROM_START(mayjisn2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "sx007-05.8b", 0x00000, 0x100000, CRC(b13ea605) SHA1(75c067df02c988f170c24153d3852c472355fc9d) )

	ROM_REGION32_LE( 0x20000*4, "sub", 0 ) // V810 code
	ROM_LOAD32_BYTE( "sx007-04.4b", 0x00003, 0x20000, CRC(fa15459f) SHA1(4163ab842943705c550f137abbdd2cb51ba5390f) )
	ROM_LOAD32_BYTE( "sx007-03.4j", 0x00002, 0x20000, CRC(71a438ea) SHA1(613bab6a59aa1bced2ab37177c61a0fd7ce7e64f) )
	ROM_LOAD32_BYTE( "sx007-02.4m", 0x00001, 0x20000, CRC(61911eed) SHA1(1442b3867b85120ba652ec8205d74332addffb67) )
	ROM_LOAD32_BYTE( "sx007-01.4s", 0x00000, 0x20000, CRC(d210bfe5) SHA1(96d9f2b198d98125df4bd6b15705646d472a8a87) )
ROM_END

/*************************************
 *
 *  Game-specific driver inits
 *
 *************************************/

void st0016_state::init_renju()
{
	m_maincpu->set_game_flag(0);
}

void st0016_state::init_nratechu()
{
	m_maincpu->set_game_flag(1);
}

void st0016_state::init_crownpkr()
{
	m_maincpu->set_game_flag(2);
}

void st0016_state::init_mayjinsn()
{
	m_maincpu->set_game_flag(4 /*| 0x80*/);
}

void st0016_state::init_mayjisn2()
{
	m_maincpu->set_game_flag(4);
}

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1994, renju,      0,      renju,    renju,    st0016_state, init_renju,    ROT0, "Visco",            "Renju Kizoku - Kira Kira Gomoku Narabe (ver. 1.0)",  MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1996, nratechu,   0,      st0016,   nratechu, st0016_state, init_nratechu, ROT0, "Seta",             "Neratte Chu (ver. 1.10)",                            MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1994, mayjisn2,   0,      mayjinsn, mayjisn2, st0016_state, init_mayjisn2, ROT0, "Seta",             "Mayjinsen 2",                                        MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1995, koikois,    0,      st0016,   koikois,  st0016_state, init_renju,    ROT0, "Visco",            "Koi Koi Shimasho - Super Real Hanafuda",             MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 2001, gostop,     0,      st0016,   gostop,   st0016_state, init_renju,    ROT0, "Visco",            "Kankoku Hanafuda Go-Stop",                           MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

// Not working
GAME( 1994, mayjinsn,   0,      mayjinsn, st0016,   st0016_state, init_mayjinsn, ROT0, "Seta",             "Mayjinsen",               MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1997, crownpkr,   0,      st0016,   crownpkr, st0016_state, init_crownpkr, ROT0, "<unknown>",        "Crown Poker (ver. 1.20)", MACHINE_NOT_WORKING | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // coining in doesn't work
GAME( 1994, dcrown,     0,      st0016,   crownpkr, st0016_state, init_crownpkr, ROT0, "Nippon Data Kiki", "Dream Crown (set 1)",     MACHINE_NOT_WORKING | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // (c) 1994 Nippon Data Kiki is uploaded near the Japanese Insert coin text
GAME( 1994, dcrowna,    dcrown, st0016,   crownpkr, st0016_state, init_crownpkr, ROT0, "Nippon Data Kiki", "Dream Crown (set 2)",     MACHINE_NOT_WORKING | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // the Insert Coin text has been translated to English and no (c) is uploaded

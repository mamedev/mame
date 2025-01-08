// license:GPL-2.0+
// copyright-holders:Norbert Kehrer
/***************************************************************************

Super Tank
(c) 1981 Video Games GmbH

Reverse-engineering and MAME driver by Norbert Kehrer (December 2003).

****************************************************************************


Hardware:
---------

CPU     TMS9980 (Texas Instruments)
Sound chip  AY-3-8910


Memory map:
-----------

>0000 - >07ff   Fixed ROM
>0800 - >17ff   Bank-switched ROM
>1800 - >1bff   RAM
>1efc       Input port 1
>1efd       Input port 2
>1efe       Read:  DIP switch port 1
        Write: Control port of AY-3-8910 sound chip
>1eff       Read: DIP switch port 2
        Write: Data port of AY-3-8910
>2000 - >3fff   Video RAM (CPU view of it, in reality there are 24 KB on the video board)


Input ports:
------------

Input port 0, mapped to memory address 0x1efc:
7654 3210
0          Player 2 Right
 0         Player 2 Left
  0        Player 2 Down
   0       Player 2 Up
     0     Player 1 Right
      0    Player 1 Left
       0   Player 1 Down
        0  Player 1 Up

Input port 1, mapped to memory address 0x1efd:
7654 3210
0          Player 2 Fire
 0         Player 1 Fire
  0        ??
   0       ??
     0     Start 1 player game
      0    Start 2 players game
       0   Coin (strobe)
        0  ??


DIP switch ports:
-----------------

DIP switch port 1, mapped to memory address 0x1efe:
7654 3210
0          Not used (?)
 0         Not used (?)
  0        Tanks per player: 1 = 5 tanks, 0 = 3 tanks
   0       Extra tank: 1 = at 10,000, 0 = at 15,000 pts.
     000   Coinage
        0  Not used (?)

DIP switch port 2, mapped to memory address 0x1eff:
7654 3210
1          ??
 1         ??
  1        ??
   1       ??
     1     ??
      1    ??
       1   ??
        1  ??


CRU lines:
----------

>400    Select bitplane for writes into video RAM (bit 0)
>401    Select bitplane for writes into video RAM (bit 1)
>402    ROM bank selector (bit 0)
>404    ROM bank selector (bit 1)
>406    Interrupt acknowledge (clears interrupt line)
>407    Watchdog reset (?)
>b12    Unknown, maybe some special-hardware sound effect or lights blinking (?)
>b13    Unknown, maybe some special-hardware sound effect or lights blinking (?)

XTAL (on CPU board) is marked 20.790 on one PCB, 22.118 on another. The
former value is correct according to the parts list in the service manual.

***************************************************************************/


#include "emu.h"

#include "cpu/tms9900/tms9980a.h"
#include "machine/74259.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"

#include "screen.h"
#include "speaker.h"


namespace {

class supertnk_state : public driver_device
{
public:
	supertnk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_videoram(*this, "videoram%u", 0U, 0x2000U, ENDIANNESS_BIG)
		, m_prgbank(*this, "prgbank")
	{ }

	void supertnk(machine_config &config);

	void init_supertnk();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void bankswitch_0_w(int state);
	void bankswitch_1_w(int state);
	void interrupt_enable_w(int state);
	void videoram_w(offs_t offset, uint8_t data);
	uint8_t videoram_r(offs_t offset);
	void bitplane_select_0_w(int state);
	void bitplane_select_1_w(int state);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank_interrupt(int state);

	void io_map(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;

	// the color PROM is 32 bytes, but it is repeating every 8 bytes
	static constexpr uint8_t NUM_PENS = 8;

	uint8_t m_rom_bank = 0;
	uint8_t m_bitplane_select = 0;
	pen_t m_pens[NUM_PENS];
	bool m_interrupt_enable = false;

	required_device<cpu_device> m_maincpu;
	memory_share_array_creator<uint8_t, 3> m_videoram;
	required_memory_bank m_prgbank;
};


void supertnk_state::machine_start()
{
	m_prgbank->configure_entries(0, 4, memregion("maincpu")->base() + 0x800, 0x1000);

	save_item(NAME(m_rom_bank));
	save_item(NAME(m_bitplane_select));
	save_item(NAME(m_interrupt_enable));
}


/*************************************
 *
 *  Memory banking
 *
 *************************************/

void supertnk_state::bankswitch_0_w(int state)
{
	m_rom_bank = (m_rom_bank & 0x02) | (state ? 0x01 : 0x00);
	m_prgbank->set_entry(m_rom_bank);
}


void supertnk_state::bankswitch_1_w(int state)
{
	m_rom_bank = (m_rom_bank & 0x01) | (state ? 0x02 : 0x00);
	m_prgbank->set_entry(m_rom_bank);
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

void supertnk_state::vblank_interrupt(int state)
{
	if (state && m_interrupt_enable)
		m_maincpu->set_input_line(INT_9980A_LEVEL4, ASSERT_LINE);
}


void supertnk_state::interrupt_enable_w(int state)
{
	m_interrupt_enable = state;
	if (!state)
		m_maincpu->set_input_line(INT_9980A_LEVEL4, CLEAR_LINE);
}



/*************************************
 *
 *  Video system
 *
 *************************************/

void supertnk_state::video_start()
{
	const uint8_t *prom = memregion("proms")->base();

	for (offs_t i = 0; i < NUM_PENS; i++)
	{
		uint8_t data = prom[i];

		m_pens[i] = rgb_t(pal1bit(data >> 2), pal1bit(data >> 5), pal1bit(data >> 6));
	}
}


void supertnk_state::videoram_w(offs_t offset, uint8_t data)
{
	if (m_bitplane_select > 2)
	{
		m_videoram[0][offset] = 0;
		m_videoram[1][offset] = 0;
		m_videoram[2][offset] = 0;
	}
	else
	{
		m_videoram[m_bitplane_select][offset] = data;
	}
}


uint8_t supertnk_state::videoram_r(offs_t offset)
{
	uint8_t ret = 0x00;

	if (m_bitplane_select < 3)
		ret = m_videoram[m_bitplane_select][offset];

	return ret;
}


void supertnk_state::bitplane_select_0_w(int state)
{
	m_bitplane_select = (m_bitplane_select & 0x02) | (state ? 0x01 : 0x00);
}


void supertnk_state::bitplane_select_1_w(int state)
{
	m_bitplane_select = (m_bitplane_select & 0x01) | (state ? 0x02 : 0x00);
}


uint32_t supertnk_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < 0x2000; offs++)
	{
		uint8_t y = offs >> 5;
		uint8_t x = offs << 3;

		uint8_t data0 = m_videoram[0][offs];
		uint8_t data1 = m_videoram[1][offs];
		uint8_t data2 = m_videoram[2][offs];

		for (int i = 0; i < 8; i++)
		{
			uint8_t color = ((data0 & 0x80) >> 5) | ((data1 & 0x80) >> 6) | ((data2 & 0x80) >> 7);
			bitmap.pix(y, x) = m_pens[color];

			data0 <<= 1;
			data1 <<= 1;
			data2 <<= 1;

			x++;
		}
	}

	return 0;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void supertnk_state::prg_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
	map(0x0800, 0x17ff).bankr(m_prgbank);
	map(0x1800, 0x1bff).ram();
	map(0x1efc, 0x1efc).portr("JOYS");
	map(0x1efd, 0x1efd).portr("INPUTS");
	map(0x1efe, 0x1eff).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x1efe, 0x1efe).portr("DSW");
	map(0x1eff, 0x1eff).portr("UNK");
	map(0x2000, 0x3fff).rw(FUNC(supertnk_state::videoram_r), FUNC(supertnk_state::videoram_w));
}



/*************************************
 *
 *  Port handlers
 *
 *************************************/

void supertnk_state::io_map(address_map &map)
{
	map(0x0000, 0x0001).nopw();
	map(0x0800, 0x080f).w("outlatch", FUNC(ls259_device::write_d0));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( supertnk )
	PORT_START("JOYS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "at 15,000 points" )
	PORT_DIPSETTING(    0x10, "at 10,000 points" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("UNK")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void supertnk_state::supertnk(machine_config &config)
{
	// CPU TMS9980A; no line connections
	TMS9980A(config, m_maincpu, 20.79_MHz_XTAL / 2); // divider not verified (possibly should be /3)
	m_maincpu->set_addrmap(AS_PROGRAM, &supertnk_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &supertnk_state::io_map);

	WATCHDOG_TIMER(config, "watchdog");

	ls259_device &outlatch(LS259(config, "outlatch")); // on CPU board near 2114 SRAM
	outlatch.q_out_cb<0>().set(FUNC(supertnk_state::bitplane_select_0_w));
	outlatch.q_out_cb<1>().set(FUNC(supertnk_state::bitplane_select_1_w));
	outlatch.q_out_cb<2>().set(FUNC(supertnk_state::bankswitch_0_w));
	outlatch.q_out_cb<4>().set(FUNC(supertnk_state::bankswitch_1_w));
	outlatch.q_out_cb<6>().set("watchdog", FUNC(watchdog_timer_device::watchdog_enable)).invert();
	outlatch.q_out_cb<7>().set(FUNC(supertnk_state::interrupt_enable_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(20.79_MHz_XTAL / 4, 330, 0, 32*8, 315, 0, 32*8); // parameters guessed
	screen.set_screen_update(FUNC(supertnk_state::screen_update));
	screen.screen_vblank().set(FUNC(supertnk_state::vblank_interrupt));

	// audio hardware
	SPEAKER(config, "mono").front_center();

	AY8910(config, "aysnd", 2000000).add_route(ALL_OUTPUTS, "mono", 0.50);
}



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( supertnk )
	ROM_REGION( 0x4800, "maincpu", 0 ) // 64k for TMS9980 code + 16k of ROM
	ROM_LOAD( "supertan.2d",  0x0000, 0x0800, CRC(1656a2c1) SHA1(1d49945aed105003a051cfbf646af7a4be1b7e86) )
	ROM_LOAD( "supertnk.3d",  0x1000, 0x0800, CRC(8b023a9a) SHA1(1afdc8d75f2ca04153bac20c0e3e123e2a7acdb7) )
	ROM_CONTINUE(             0x0800, 0x0800)
	ROM_LOAD( "supertnk.4d",  0x2000, 0x0800, CRC(b8249e5c) SHA1(ef4bb714b0c1b97890a067f05fc50ab3426ce37f) )
	ROM_CONTINUE(             0x1800, 0x0800)
	ROM_LOAD( "supertnk.8d",  0x3000, 0x0800, CRC(d8175a4f) SHA1(cba7b426773ac86c81a9eac81087a2db268cd0f9) )
	ROM_CONTINUE(             0x2800, 0x0800)
	ROM_LOAD( "supertnk.9d",  0x4000, 0x0800, CRC(a34a494a) SHA1(9b7f0560e9d569ee25eae56f31886d50a3153dcc) )
	ROM_CONTINUE(             0x3800, 0x0800)

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "supertnk.clr",  0x0000, 0x0020, CRC(9ae1faee) SHA1(19de4bb8bc389d98c8f8e35c755fad96e1a6a0cd) )   // color PROM
	ROM_LOAD( "supertnk.s",    0x0020, 0x0020, CRC(91722fcf) SHA1(f77386014b459cc151d2990ac823b91c04e8d319) )   // unknown - sync?
	ROM_LOAD( "supertnk.t",    0x0040, 0x0020, CRC(154390bd) SHA1(4dc0fd7bd8999d2670c8d93aaada835d2a84d4db) )   // unknown - sync?
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

void supertnk_state::init_supertnk()
{
	// decode the TMS9980 ROMs
	uint8_t *rom = memregion("maincpu")->base();
	size_t len = memregion("maincpu")->bytes();

	for (offs_t offs = 0; offs < len; offs++)
	{
		rom[offs] = bitswap<8>(rom[offs], 0, 1, 2, 3, 4, 5, 6, 7);
	}
}

} // anonymous namespace


GAME( 1981, supertnk, 0, supertnk, supertnk, supertnk_state, init_supertnk, ROT90, "Video Games GmbH", "Super Tank", MACHINE_SUPPORTS_SAVE )

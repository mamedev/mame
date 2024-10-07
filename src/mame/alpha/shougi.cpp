// license:GPL-2.0+
// copyright-holders:Jarek Burczynski, Tomasz Slanina
/***************************************************************************

Driver by Jarek Burczynski, started by Tomasz Slanina
Lots of hardware info from Guru

memory map :
0000 - 3fff rom
4000 - 43ff RAM for main CPU
4800 - 480f  control area (connected to 74LS259 8-bit addressable latch; lines A0,A1,A2 - address input, line A3 - data input)
    4800,4808 - control shared RAM access between main CPU/sub CPU. 4800=access for sub CPU, 4808=access for main CPU
    4801,4809 - NMI disable/enable
    4802,480a -
    4803,480b -
    4804,480c -
    4807,480f -?

4800 - 4800 read: input
5000 - 5000 read: input
5800 - 5800 read: input

5800 - 5800 write: watchdog

6000 - AY port write
6800 - AY data write

7000 - 73ff RAM shared: main CPU/ALPHA MCU
7800 - 7bff RAM shared: between main CPU/sub CPU

8000 - bfff colorram
c000 - ffff videoram



Both games use custom MCU: ALPHA 8201 (42 pin DIP).
It's connected to the RAM that is shared with the first CPU.

Shougi
Alpha Electronics Co. Ltd., 198x

PCB No: 57F1
CPU   : Z80A (x2)
SOUND : AY-3-8910
XTAL  : 10.000MHz
RAM   :
 2114 (x6), one RAM: 0x400 x 4bits = 0x400 bytes (x 3)
mapped at:
1: 0x4000 - 0x43ff (main CPU - stack also here, so it is work RAM)
2: 0x7000 - 0x73ff (main CPU - shared with ALPHA-8201)
3: 0x6000 - 0x63ff (sub CPU)


 4116 (x16), one RAM: 16K x 1bit  = 16K x 8bits *2 = 32K x 8bits
mapped at:
0x8000 - 0xffff



CUSTOM: ALPHA 8201 (42 pin DIP)
DIPSW : 6 position (x1)
       Positions 1, 5 & 6 not used

    4    3    2
    ------------------------------
    OFF  OFF  OFF  1 minutes (time for the opponent to make his decision)
    OFF  OFF  ON   2
    OFF  ON   OFF  3
    OFF  ON   ON   4
    ON   OFF  OFF  5
    ON   OFF  ON   10
    ON   ON   OFF  20
    ON   ON   ON   30

ROMs  : All type 2732
PROM  : Type MB7051

**************************************************************************/

#include "emu.h"

#include "alpha8201.h"

#include "machine/74259.h"
#include "machine/watchdog.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class shougi_state : public driver_device
{
public:
	shougi_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_alpha_8201(*this, "alpha_8201"),
		m_videoram(*this, "videoram")
	{ }

	void shougi(machine_config &config);

private:
	void nmi_enable_w(int state);
	uint8_t semaphore_r();

	void shougi_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_nmi);

	virtual void machine_start() override ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void readport_sub(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<alpha_8201_device> m_alpha_8201;

	required_shared_ptr<uint8_t> m_videoram;

	uint8_t m_nmi_enabled = 0;
	int m_r = 0;
};

void shougi_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_nmi_enabled));
	save_item(NAME(m_r));
}



/***************************************************************************

  Video

***************************************************************************/

/***************************************************************************

  Convert the color PROMs into a more useable format.

  bit 0 -- 1000 ohm resistor--\
  bit 1 -- 470 ohm resistor --+--+--> RED
  bit 2 -- 220 ohm resistor --/  \---------------1000 ohm resistor---\
  bit 3 -- 1000 ohm resistor--\                                      |
  bit 4 -- 470 ohm resistor --+--+--> GREEN                          |
  bit 5 -- 220 ohm resistor --/  \---------------1000 ohm resistor---+--- GND
  bit 6 -- 470 ohm resistor --+--+--> BLUE                           |
  bit 7 -- 220 ohm resistor --/  \---------------1000 ohm resistor---/

***************************************************************************/

void shougi_state::shougi_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static const int resistances_b[2]  = { 470, 220 };
	static const int resistances_rg[3] = { 1000, 470, 220 };
	double weights_r[3], weights_g[3], weights_b[2];

	compute_resistor_weights(0, 255, -1.0,
			3,  resistances_rg, weights_r,  1000, 0,
			3,  resistances_rg, weights_g,  1000, 0,
			2,  resistances_b,  weights_b,  1000, 0);

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0,bit1,bit2;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		int const r = combine_weights(weights_r, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		int const g = combine_weights(weights_g, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		int const b = combine_weights(weights_b, bit0, bit1);

		palette.set_pen_color(i,rgb_t(r,g,b));
	}
}


uint32_t shougi_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < 0x4000; offs++)
	{
		int sx, sy, x, data1, data2, color, data;

		sx = offs >> 8;     /*00..0x3f (64*4=256)*/
		sy = offs & 0xff;   /*00..0xff*/

		data1 = m_videoram[offs];           /* color */
		data2 = m_videoram[0x4000 + offs];  /* pixel data */

		for (x=0; x<4; x++) /*4 pixels per byte (2 bitplanes in 2 nibbles: 1st=bits 7-4, 2nd=bits 3-0)*/
		{
			color= ((data1>>x) & 1) | (((data1>>(4+x)) & 1)<<1);
			data = ((data2>>x) & 1) | (((data2>>(4+x)) & 1)<<1);

			bitmap.pix(255-sy, 255-(sx*4 + x)) = color*4 + data;
		}
	}

	return 0;
}



/***************************************************************************

  I/O

***************************************************************************/

// maincpu side

void shougi_state::nmi_enable_w(int state)
{
	m_nmi_enabled = state;

	// NMI lines are tied together on both CPUs and connected to the LS74 /Q output
	if (!m_nmi_enabled)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		m_subcpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
}

INTERRUPT_GEN_MEMBER(shougi_state::vblank_nmi)
{
	if (m_nmi_enabled)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_subcpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
}


void shougi_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).ram(); /* 2114 x 2 (0x400 x 4bit each) */
	map(0x4800, 0x480f).w("mainlatch", FUNC(ls259_device::write_a3));
	map(0x4800, 0x4800).portr("DSW");
	map(0x5000, 0x5000).portr("P1");
	map(0x5800, 0x5800).portr("P2").w("watchdog", FUNC(watchdog_timer_device::reset_w)); /* game won't boot if watchdog doesn't work */
	map(0x6000, 0x6000).w("aysnd", FUNC(ay8910_device::address_w));
	map(0x6800, 0x6800).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x7000, 0x73ff).rw(m_alpha_8201, FUNC(alpha_8201_device::ext_ram_r), FUNC(alpha_8201_device::ext_ram_w));
	map(0x7800, 0x7bff).ram().share("sharedram"); /* 2114 x 2 (0x400 x 4bit each) */
	map(0x8000, 0xffff).ram().share("videoram"); /* 4116 x 16 (32K) */
}


// subcpu side

uint8_t shougi_state::semaphore_r()
{
	// d0: waits for it to be set before handling NMI routine
	// hmm it must be a signal from maincpu, but what?
	m_r ^= 1;
	return m_r;
}


void shougi_state::sub_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x63ff).ram().share("sharedram"); /* 2114 x 2 (0x400 x 4bit each) */
}

void shougi_state::readport_sub(address_map &map)
{
	map.global_mask(0x00ff);
	map(0x00, 0x00).r(FUNC(shougi_state::semaphore_r));
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( shougi )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP  ) PORT_PLAYER(2)

	// dip switch order is not sequential. Only 2,3,4, and 5 identified.
	// 1 and 6 missing, with three possible positions (the third available
	// bit is not a dip switch)
	PORT_START("DSW")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x32, 0x32, "Time" ) PORT_DIPLOCATION("SW:!3,!4,!2")
	PORT_DIPSETTING(    0x00, "1 Minute" )
	PORT_DIPSETTING(    0x20, "2 Minutes" )
	PORT_DIPSETTING(    0x02, "3 Minutes" )
	PORT_DIPSETTING(    0x22, "4 Minutes" )
	PORT_DIPSETTING(    0x10, "5 Minutes" )
	PORT_DIPSETTING(    0x30, "10 Minutes" )
	PORT_DIPSETTING(    0x12, "20 Minutes" )
	PORT_DIPSETTING(    0x32, "30 Minutes" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW:!5")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( shougi2 )
	PORT_INCLUDE(shougi)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void shougi_state::shougi(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(10'000'000)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &shougi_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(shougi_state::vblank_nmi));

	Z80(config, m_subcpu, XTAL(10'000'000)/4);
	m_subcpu->set_addrmap(AS_PROGRAM, &shougi_state::sub_map);
	m_subcpu->set_addrmap(AS_IO, &shougi_state::readport_sub);

	ALPHA_8201(config, m_alpha_8201, XTAL(10'000'000)/4/8);

	ls259_device &mainlatch(LS259(config, "mainlatch"));
	mainlatch.q_out_cb<0>().set_nop(); // 0: sharedram = sub, 1: sharedram = main (TODO!)
	mainlatch.q_out_cb<1>().set(FUNC(shougi_state::nmi_enable_w));
	mainlatch.q_out_cb<2>().set_nop(); // ?
	mainlatch.q_out_cb<3>().set(m_alpha_8201, FUNC(alpha_8201_device::mcu_start_w)); // start/halt ALPHA-8201
	mainlatch.q_out_cb<4>().set(m_alpha_8201, FUNC(alpha_8201_device::bus_dir_w)).invert(); // ALPHA-8201 shared RAM bus direction: 0: mcu, 1: maincpu
	mainlatch.q_out_cb<7>().set_nop(); // nothing? connected to +5v via resistor

	config.set_perfect_quantum(m_maincpu);

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 0x10); // assuming it's the same as champbas

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 255, 0, 255);
	screen.set_screen_update(FUNC(shougi_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(shougi_state::shougi_palette), 32);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, "aysnd", XTAL(10'000'000)/8).add_route(ALL_OUTPUTS, "mono", 0.30);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( shougi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.3a",    0x0000, 0x1000, CRC(b601303f) SHA1(ed07fb09053e15be49f4cb66e8916d1bdff48336) )
	ROM_LOAD( "3.3c",    0x1000, 0x1000, CRC(2b8c7314) SHA1(5d21e425889f8dc118fcd2ba8cfc6fb8f94ddc5f) )
	ROM_LOAD( "2.3b",    0x2000, 0x1000, CRC(09cb831f) SHA1(5a83a22d9245f980fe6a495433e51437d1f95644) )
	ROM_LOAD( "4.3d",    0x3000, 0x1000, CRC(ad1a642a) SHA1(d12b10f94a568d1126384e14af4b53c5e5b1a0d0) )

	ROM_REGION( 0x10000, "sub", ROMREGION_ERASE00 )
	ROM_LOAD( "5.3e",    0x0000, 0x1000, CRC(ff1f07d0) SHA1(ae5bab09916b6d4ad8d3568ea39501850bdc6991) )
	ROM_LOAD( "8.3j",    0x1000, 0x1000, CRC(6230c4c1) SHA1(0b2c81bb02c270ed3bb5b42c4bd4eb25023090cb) )
	ROM_LOAD( "6.3f",    0x2000, 0x1000, CRC(d5a91b16) SHA1(1d21295667c3eb186f9e7f867763f2f2697fd350) )
	ROM_LOAD( "9.3k",    0x3000, 0x1000, CRC(dbbfa66e) SHA1(fcf23fcc65e8253325937acaf7aad4253be5e6df) )
	ROM_LOAD( "7.3h",    0x4000, 0x1000, CRC(7ea8ec4a) SHA1(d3b999a683f49c911871d0ae6bb2022e73e3cfb8) )
	/* shougi has one socket empty */

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8201_44801a75_2f25.bin", 0x0000, 0x2000, CRC(b77931ac) SHA1(405b02585e80d95a2821455538c5c2c31ce262d1) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr.2l",   0x0000, 0x0020, CRC(cd3559ff) SHA1(a1291b06a8a337943660b2ef62c94c49d58a6fb5) )
ROM_END


ROM_START( shougi2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1-2.3a",    0x0000, 0x1000, CRC(16d75306) SHA1(2d090396abd1fe2b31cb8450cc5d2fbde75e0230) )
	ROM_LOAD( "3-2.3c",    0x1000, 0x1000, CRC(35b6d98b) SHA1(fc125acd4d504d9c883e685b9c6e5a509dc75c69) )
	ROM_LOAD( "2-2.3b",    0x2000, 0x1000, CRC(b38affed) SHA1(44529233358923f114285533270b2a3c078b70f4) )
	ROM_LOAD( "4-2.3d",    0x3000, 0x1000, CRC(1abdb6bf) SHA1(9c7630c0e4bcaa4296a442b0e9828b96d91da77f) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "5-2.3e",    0x0000, 0x1000, CRC(0ba89dd4) SHA1(d4d3b7bccccf3b7e07e2d9d776426a22b4ff422e) )
	ROM_LOAD( "8-2.3j",    0x1000, 0x1000, CRC(0ae0c8c1) SHA1(91f6f88d38c96c793137e7aaa763cab1b769e098) )
	ROM_LOAD( "6-2.3f",    0x2000, 0x1000, CRC(d98abcae) SHA1(f280b627f81f2c727268b9694d833e487ff6b08d) )
	ROM_LOAD( "9-2.3k",    0x3000, 0x1000, CRC(4e0e6c90) SHA1(b8462eec0a13d8bdf7d314eb285b5bd27d40631c) )
	ROM_LOAD( "7-2.3h",    0x4000, 0x1000, CRC(5f37ebc6) SHA1(2e5c4c2f455979e2ad2c66c5aa9f4d92194796af) )
	ROM_LOAD( "10-2.3l",   0x5000, 0x1000, CRC(a26385fd) SHA1(2adb21bb4f67a378014bc1edda48daca349d17e1) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8201_44801a75_2f25.bin", 0x0000, 0x2000, CRC(b77931ac) SHA1(405b02585e80d95a2821455538c5c2c31ce262d1) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr.2l",   0x0000, 0x0020, CRC(cd3559ff) SHA1(a1291b06a8a337943660b2ef62c94c49d58a6fb5) )
ROM_END

} // anonymous namespace


/*    YEAR  NAME     PARENT  MACHINE  INPUT    STATE         INIT        MONITOR  COMPANY                              FULLNAME          FLAGS */
GAME( 1982, shougi,  0,      shougi,  shougi,  shougi_state, empty_init, ROT0,    "Alpha Denshi Co. (Tehkan license)", "Shougi",         MACHINE_SUPPORTS_SAVE )
GAME( 1982, shougi2, 0,      shougi,  shougi2, shougi_state, empty_init, ROT0,    "Alpha Denshi Co. (Tehkan license)", "Shougi Part II", MACHINE_SUPPORTS_SAVE )

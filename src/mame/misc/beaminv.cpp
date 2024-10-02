// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    Tekunon Kougyou(Teknon Kogyo) Beam Invader hardware

    driver by Zsolt Vasvari

    Games supported:
        * Beam Invader (2 sets)

    Known issues:
        * Port 0 might be an analog port select


Stephh's notes (based on the games Z80 code and some tests) :

  - The min/max values for the controllers might not be accurate, but I have no infos at all.
    So I put the min/max values from what I see in the Z80 code (see below).
  - Is the visible area correct ? The invaders and the ship don't reach the left part of the screen !

1) 'beaminv'

  - Routine to handle the analog inputs at 0x0521.
    Contents from 0x3400 (IN2) is compared with contents from 0x1d25 (value in RAM).
    Contents from 0x3400 is not limited but contents from 0x1d25 range is the following :
      . player 1 : min = 0x1c - max = 0xd1
      . player 2 : min = 0x2d - max = 0xe2
    This is why sometimes the ship moves even if you don't do anything !
  - Screen flipping is internally handled (no specific write to memory or out to a port).
  - I can't tell if controller select is handled with a out to port 0 but I haven't found
    any other write to memory or out to another port.
  - Player's turn is handled by multiple reads from 0x1839 in RAM :
      . 1 player  game : [0x1839] = 0x00
      . 2 players game : [0x1839] = 0xaa (player 1) or 0x55 (player 2)
  - Credits are stored at address 0x1837 (BCD coded, range 0x00-0x99)

2) 'pacominv'

  - Routine to handle the analog inputs at 0x04bd.
    Contents from 0x3400 (IN2) is compared with contents from 0x1d05 (value in RAM).
    Contents from 0x3400 is limited to range 0x35-0x95 but contents from 0x1d05 range is the following :
      . player 1 : min = 0x1c - max = 0xd1
      . player 2 : min = 0x2d - max = 0xe2
    This is why sometimes the ship moves even if you don't do anything !
  - Screen flipping is internally handled (no specific write to memory or out to a port).
  - I can't tell if controller select is handled with a out to port 0 but I haven't found
    any other write to memory or out to another port.
  - Player's turn is handled by multiple reads from 0x1838 in RAM :
      . 1 player  game : [0x1838] = 0x00
      . 2 players game : [0x1838] = 0xaa (player 1) or 0x55 (player 2)
  - Credits are stored at address 0x1836 (BCD coded, range 0x00-0x99)

  Additional notes:
  - The only difference with pacominva is that it has some coded added
    at the end of ROM 5, however it appears to be unused

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "screen.h"

#include "beaminv.lh"


namespace {

class beaminv_state : public driver_device
{
public:
	beaminv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;

	/* misc */
	emu_timer  *m_interrupt_timer = nullptr;

	/* input-related */
	uint8_t      m_controller_select = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	uint8_t v128_r();
	void controller_select_w(uint8_t data);
	uint8_t controller_r();
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint32_t screen_update_beaminv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(interrupt_callback);
	void create_interrupt_timer();
	void start_interrupt_timer();
	void beaminv(machine_config &config);
	void ctainv(machine_config &config);
	void main_io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void ctainv_map(address_map &map) ATTR_COLD;
};



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

/* the interrupt scanlines are a guess */

#define INTERRUPTS_PER_FRAME    (2)

static const int interrupt_lines[INTERRUPTS_PER_FRAME] = { 0x00, 0x80 };


TIMER_CALLBACK_MEMBER(beaminv_state::interrupt_callback)
{
	int interrupt_number = param;
	int next_interrupt_number;
	int next_vpos;

	m_maincpu->set_input_line(0, HOLD_LINE);

	/* set up for next interrupt */
	next_interrupt_number = (interrupt_number + 1) % INTERRUPTS_PER_FRAME;
	next_vpos = interrupt_lines[next_interrupt_number];

	m_interrupt_timer->adjust(m_screen->time_until_pos(next_vpos), next_interrupt_number);
}


void beaminv_state::create_interrupt_timer()
{
	m_interrupt_timer = timer_alloc(FUNC(beaminv_state::interrupt_callback), this);
}


void beaminv_state::start_interrupt_timer()
{
	int vpos = interrupt_lines[0];
	m_interrupt_timer->adjust(m_screen->time_until_pos(vpos));
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

void beaminv_state::machine_start()
{
	create_interrupt_timer();


	/* setup for save states */
	save_item(NAME(m_controller_select));
}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

void beaminv_state::machine_reset()
{
	start_interrupt_timer();

	m_controller_select = 0;
}



/*************************************
 *
 *  Video system
 *
 *************************************/

uint32_t beaminv_state::screen_update_beaminv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_videoram.bytes(); offs++)
	{
		uint8_t y = offs;
		uint8_t x = offs >> 8 << 3;
		uint8_t data = m_videoram[offs];

		for (int i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x01) ? rgb_t::white() : rgb_t::black();
			bitmap.pix(y, x) = pen;

			data >>= 1;
			x++;
		}
	}

	return 0;
}


uint8_t beaminv_state::v128_r()
{
	return (m_screen->vpos() >> 7) & 0x01;
}



/*************************************
 *
 *  Controller
 *
 *************************************/

#define P1_CONTROL_PORT_TAG ("CONTP1")
#define P2_CONTROL_PORT_TAG ("CONTP2")


void beaminv_state::controller_select_w(uint8_t data)
{
	/* 0x01 (player 1) or 0x02 (player 2) */
	m_controller_select = data;
}


uint8_t beaminv_state::controller_r()
{
	return ioport((m_controller_select == 1) ? P1_CONTROL_PORT_TAG : P2_CONTROL_PORT_TAG)->read();
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void beaminv_state::main_map(address_map &map)
{
	map(0x0000, 0x17ff).rom();
	map(0x1800, 0x1fff).ram();
	map(0x2400, 0x2400).mirror(0x03ff).portr("DSW");
	map(0x2800, 0x2800).mirror(0x03ff).portr("INPUTS");
	map(0x3400, 0x3400).mirror(0x03ff).r(FUNC(beaminv_state::controller_r));
	map(0x3800, 0x3800).mirror(0x03ff).r(FUNC(beaminv_state::v128_r));
	map(0x4000, 0x5fff).ram().share("videoram");
}


void beaminv_state::ctainv_map(address_map &map)
{
	map(0x0000, 0x17ff).rom();
	map(0x1800, 0x1fff).ram();
	map(0x2000, 0x23ff).rom().region("maincpu", 0x1800);
	map(0x2400, 0x2400).mirror(0x03ff).portr("DSW");
	map(0x2800, 0x2800).mirror(0x03ff).portr("INPUTS");
	map(0x2c00, 0x2c00).mirror(0x03ff).r(FUNC(beaminv_state::controller_r));
	map(0x3800, 0x3800).mirror(0x03ff).r(FUNC(beaminv_state::v128_r));
	map(0x4000, 0x5fff).ram().share("videoram");
}



/*************************************
 *
 *  Port handlers
 *
 *************************************/

void beaminv_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(beaminv_state::controller_select_w)); /* to be confirmed */
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( beaminv )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "2000" )
	PORT_DIPSETTING(    0x08, "3000" )
	PORT_DIPSETTING(    0x0c, "4000" )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x60, 0x00, "Faster Bombs At" )
	PORT_DIPSETTING(    0x00, "49 Enemies" )
	PORT_DIPSETTING(    0x20, "39 Enemies" )
	PORT_DIPSETTING(    0x40, "29 Enemies" )
	PORT_DIPSETTING(    0x60, "Never" )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_HIGH )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START(P1_CONTROL_PORT_TAG)
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_START(P2_CONTROL_PORT_TAG)
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( pacominv )
	PORT_INCLUDE( beaminv )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_DIPSETTING(    0x04, "2000" )
	PORT_DIPSETTING(    0x08, "2500" )
	PORT_DIPSETTING(    0x0c, "3000" )
	PORT_DIPNAME( 0x60, 0x00, "Faster Bombs At" )
	PORT_DIPSETTING(    0x00, "44 Enemies" )
	PORT_DIPSETTING(    0x20, "39 Enemies" )
	PORT_DIPSETTING(    0x40, "34 Enemies" )
	PORT_DIPSETTING(    0x40, "29 Enemies" )

	PORT_MODIFY(P1_CONTROL_PORT_TAG)
	PORT_BIT( 0xff, 0x65, IPT_PADDLE ) PORT_MINMAX(0x35,0x95) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_MODIFY(P2_CONTROL_PORT_TAG)
	PORT_BIT( 0xff, 0x65, IPT_PADDLE ) PORT_MINMAX(0x35,0x95) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( ctainv )
	PORT_INCLUDE( pacominv )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "1500" )
	PORT_DIPSETTING(    0x08, "2000" )
	PORT_DIPSETTING(    0x0c, "2500" )
	PORT_DIPNAME( 0x60, 0x00, "Faster Bombs At" ) // table at 0x1738
	PORT_DIPSETTING(    0x00, "29 Enemies" )
	PORT_DIPSETTING(    0x20, "24 Enemies" )
	PORT_DIPSETTING(    0x40, "19 Enemies" )
	PORT_DIPSETTING(    0x40, "14 Enemies" )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void beaminv_state::beaminv(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 2000000);   /* 2 MHz ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &beaminv_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &beaminv_state::main_io_map);


	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 247, 16, 231);
	m_screen->set_refresh_hz(60);
	m_screen->set_screen_update(FUNC(beaminv_state::screen_update_beaminv));
}


void beaminv_state::ctainv(machine_config &config)
{
	beaminv(config);

	m_maincpu->set_clock(9.732_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &beaminv_state::ctainv_map);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( beaminv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0a", 0x0000, 0x0400, CRC(17503086) SHA1(18c789216e5c4330dba3eeb24919dae636bf803d) )
	ROM_LOAD( "1a", 0x0400, 0x0400, CRC(aa9e1666) SHA1(050e2bd169f1502f49b7e6f5f2df9dac0d8107aa) )
	ROM_LOAD( "2a", 0x0800, 0x0400, CRC(ebaa2fc8) SHA1(b4ff1e1bdfe9efdc08873bba2f0a30d24678f9d8) )
	ROM_LOAD( "3a", 0x0c00, 0x0400, CRC(4f62c2e6) SHA1(4bd7d5e4f18d250003c7d771f1cdab08d699a765) )
	ROM_LOAD( "4a", 0x1000, 0x0400, CRC(3eebf757) SHA1(990eebda80ec52b7e3a36912c6e9230cd97f9f25) )
	ROM_LOAD( "5a", 0x1400, 0x0400, CRC(ec08bc1f) SHA1(e1df6704298e470a77158740c275fdca105e8f69) )
ROM_END


ROM_START( pacominv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom_0", 0x0000, 0x0400, CRC(67e100dd) SHA1(5f58e2ed3da14c48f7c382ee6091a59caf8e0609) )
	ROM_LOAD( "rom_1", 0x0400, 0x0400, CRC(442bbe98) SHA1(0e0382d4f6491629449759747019bd453a458b66) )
	ROM_LOAD( "rom_2", 0x0800, 0x0400, CRC(5d5d2f68) SHA1(e363f9445bbba1492188efe1830cae96f6078878) )
	ROM_LOAD( "rom_3", 0x0c00, 0x0400, CRC(527906b8) SHA1(9bda7da653db64246597ca386adab4cbab319189) )
	ROM_LOAD( "rom_4", 0x1000, 0x0400, CRC(920bb3f0) SHA1(3b9897d31c551e0b9193f775a6be65376b4a8c34) )
	ROM_LOAD( "rom_5", 0x1400, 0x0400, CRC(3f6980e4) SHA1(cb73cbc474677e6e302cb3842f32923ef2cdc98d) )
ROM_END


ROM_START( pacominva )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0.bin", 0x0000, 0x0400, CRC(67e100dd) SHA1(5f58e2ed3da14c48f7c382ee6091a59caf8e0609) )
	ROM_LOAD( "1.bin", 0x0400, 0x0400, CRC(442bbe98) SHA1(0e0382d4f6491629449759747019bd453a458b66) )
	ROM_LOAD( "2.bin", 0x0800, 0x0400, CRC(5d5d2f68) SHA1(e363f9445bbba1492188efe1830cae96f6078878) )
	ROM_LOAD( "3.bin", 0x0c00, 0x0400, CRC(527906b8) SHA1(9bda7da653db64246597ca386adab4cbab319189) )
	ROM_LOAD( "4.bin", 0x1000, 0x0400, CRC(920bb3f0) SHA1(3b9897d31c551e0b9193f775a6be65376b4a8c34) )
	ROM_LOAD( "5.bin", 0x1400, 0x0400, CRC(72972e81) SHA1(1e5f820eb7b30f2f1b63d077eab786cb2b836e26) )
ROM_END


ROM_START( ctainv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0.bin", 0x0000, 0x0400, CRC(363cd088) SHA1(125dfa97b80369da7c2e8fd84caa14bdf7df80dc) )
	ROM_LOAD( "1.bin", 0x0400, 0x0400, CRC(04057d07) SHA1(749acd65f01cd408d3d452cba902580fc2e4cd49) )
	ROM_LOAD( "2.bin", 0x0800, 0x0400, CRC(569a3fe9) SHA1(ed51bd5a950c821531f8a40219339df6882e7d26) )
	ROM_LOAD( "3.bin", 0x0c00, 0x0400, CRC(772db93e) SHA1(65d0a7528c86b4c3377e8cfda82eae51ee078238) )
	ROM_LOAD( "4.bin", 0x1000, 0x0400, CRC(c9f671d9) SHA1(27e8ded5afc92f1eef9968a6af4b8c5af482904f) )
	ROM_LOAD( "5.bin", 0x1400, 0x0400, CRC(a028342f) SHA1(32fd83b0ac8215935032a22e3bfd9fcf1b8402c0) )
	ROM_LOAD( "6.bin", 0x1800, 0x0400, CRC(06dcb63c) SHA1(4d5260b3785e2c215dd0b3c9f8457cf4a557a452) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAMEL( 1979, beaminv,   0,       beaminv, beaminv,  beaminv_state, empty_init, ROT270, "Teknon Kogyo",      "Beam Invader",          MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE, layout_beaminv )
GAMEL( 1979, pacominv,  beaminv, beaminv, pacominv, beaminv_state, empty_init, ROT270, "Pacom Corporation", "Pacom Invader (set 1)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE, layout_beaminv )
GAMEL( 19??, pacominva, beaminv, beaminv, pacominv, beaminv_state, empty_init, ROT270, "Pacom Corporation", "Pacom Invader (set 2)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE, layout_beaminv )
GAME ( 19??, ctainv,    beaminv, ctainv,  ctainv,   beaminv_state, empty_init, ROT270, "CTA Corporation",   "CTA Invader",           MACHINE_WRONG_COLORS | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

// license:BSD-3-Clause
// copyright-holders:Steve Baines, Frank Palazzolo
/***************************************************************************

    Atari Star Wars hardware

    driver by Steve Baines and Frank Palazzolo

    This file is Copyright Steve Baines.
    Modified by Frank Palazzolo for sound support

    Games supported:
        * Star Wars
        * The Empire Strikes Back
        * TomCat prototype on Star Wars hardware

    Known bugs:
        * the monitor "overdrive" effect is not simulated when you
          get hit by enemy fire

****************************************************************************

    Memory map (TBA)

***************************************************************************/

#include "emu.h"
#include "starwars.h"

#include "cpu/m6809/m6809.h"
#include "machine/74259.h"
#include "machine/adc0808.h"
#include "machine/watchdog.h"
#include "video/avgdvg.h"
#include "video/vector.h"
#include "screen.h"
#include "speaker.h"


#define MASTER_CLOCK (12.096_MHz_XTAL)
#define CLOCK_3KHZ   (MASTER_CLOCK / 4096)



/*************************************
 *
 *  Machine init
 *
 *************************************/

void starwars_state::machine_reset()
{
	/* reset the matrix processor */
	starwars_mproc_reset();
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

void starwars_state::irq_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

uint8_t starwars_state::starwars_main_ready_flag_r()
{
	/* only upper two flag bits mapped */
	return (m_soundlatch->pending_r() << 7) | (m_mainlatch->pending_r() << 6);
}

void starwars_state::starwars_soundrst_w(uint8_t data)
{
	m_soundlatch->acknowledge_w();
	m_mainlatch->acknowledge_w();

	/* reset sound CPU here  */
	m_audiocpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

void starwars_state::main_map(address_map &map)
{
	map(0x0000, 0x2fff).ram();
	map(0x3000, 0x3fff).rom().region("vectorrom", 0);
	map(0x4300, 0x431f).portr("IN0");
	map(0x4320, 0x433f).portr("IN1");
	map(0x4340, 0x435f).portr("DSW0");
	map(0x4360, 0x437f).portr("DSW1");
	map(0x4380, 0x439f).r("adc", FUNC(adc0809_device::data_r));
	map(0x4400, 0x4400).r(m_mainlatch, FUNC(generic_latch_8_device::read));
	map(0x4400, 0x4400).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x4401, 0x4401).r(FUNC(starwars_state::starwars_main_ready_flag_r));
	map(0x4500, 0x45ff).rw("x2212", FUNC(x2212_device::read), FUNC(x2212_device::write));
	map(0x4600, 0x461f).w("avg", FUNC(avg_starwars_device::go_w));
	map(0x4620, 0x463f).w("avg", FUNC(avg_starwars_device::reset_w));
	map(0x4640, 0x465f).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x4660, 0x467f).w(FUNC(starwars_state::irq_ack_w));
	map(0x4680, 0x4687).nopr().mirror(0x0018).w("outlatch", FUNC(ls259_device::write_d7));
	map(0x46a0, 0x46bf).w(FUNC(starwars_state::starwars_nstore_w));
	map(0x46c0, 0x46c3).w("adc", FUNC(adc0809_device::address_offset_start_w));
	map(0x46e0, 0x46e0).w(FUNC(starwars_state::starwars_soundrst_w));
	map(0x4700, 0x4707).w(FUNC(starwars_state::starwars_math_w));
	map(0x4700, 0x4700).r(FUNC(starwars_state::starwars_div_reh_r));
	map(0x4701, 0x4701).r(FUNC(starwars_state::starwars_div_rel_r));
	map(0x4703, 0x4703).r(FUNC(starwars_state::starwars_prng_r));           /* pseudo random number generator */
	map(0x4800, 0x4fff).ram();                             /* CPU and Math RAM */
	map(0x5000, 0x5fff).ram().share("mathram"); /* CPU and Math RAM */
	map(0x6000, 0x7fff).bankr("bank1");                        /* banked ROM */
	map(0x8000, 0xffff).rom();                             /* rest of main_rom */
}

void starwars_state::esb_main_map(address_map &map)
{
	main_map(map);
	map(0x8000, 0x9fff).bankr(m_slapstic_bank);
	map(0xa000, 0xffff).bankr("bank2");
}


/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

void starwars_state::quad_pokeyn_w(offs_t offset, uint8_t data)
{
	int pokey_num = (offset >> 3) & ~0x04;
	int control = (offset & 0x20) >> 2;
	int pokey_reg = (offset % 8) | control;

	m_pokey[pokey_num]->write(pokey_reg, data);
}

void starwars_state::sound_map(address_map &map)
{
	map(0x0000, 0x07ff).w(m_mainlatch, FUNC(generic_latch_8_device::write));
	map(0x0800, 0x0fff).r(m_soundlatch, FUNC(generic_latch_8_device::read)); /* SIN Read */
	map(0x1000, 0x107f).m(m_riot, FUNC(mos6532_device::ram_map));
	map(0x1080, 0x109f).m(m_riot, FUNC(mos6532_device::io_map));
	map(0x1800, 0x183f).w(FUNC(starwars_state::quad_pokeyn_w));
	map(0x2000, 0x27ff).ram();                         /* program RAM */
	map(0x4000, 0x7fff).rom();                         /* sound roms */
	map(0xb000, 0xffff).rom();                         /* more sound roms */
}



/*************************************
 *
 *  Port definitions
 *
 *  Dips Manual Verified and Defaults
 *  set for starwars and esb - 06/2009
 *
 *************************************/

static INPUT_PORTS_START( starwars )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Diagnostic Step") // mentioned in schematics, but N/C?
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	/* Bit 6 is VG_HALT */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("avg", FUNC(avg_starwars_device::done_r))
	/* Bit 7 is MATH_RUN - see machine/starwars.c */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(starwars_state::matrix_flag_r))

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x00, "Starting Shields" )  PORT_DIPLOCATION("10D:1,2")
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x02, "8" )
	PORT_DIPSETTING(    0x03, "9" )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("10D:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, "Moderate" )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x10, "Bonus Shields" )  PORT_DIPLOCATION("10D:5,6")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("10D:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )  PORT_DIPLOCATION("10D:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ) )  PORT_DIPLOCATION("10EF:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
		/* Manual shows Coin_B (Right) as Bit 4,5 - actually Bit 3,4 */
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )  PORT_DIPLOCATION("10EF:3,4")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x04, "*4" )
	PORT_DIPSETTING(    0x08, "*5" )
	PORT_DIPSETTING(    0x0c, "*6" )
		/* Manual shows Coin_A (Left) as Bit 3 - actually Bit 5 */
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coin_A ) )  PORT_DIPLOCATION("10EF:5")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x10, "*2" )
	PORT_DIPNAME( 0xe0, 0x00, "Bonus Coin Adder" )  PORT_DIPLOCATION("10EF:6,7,8")
	PORT_DIPSETTING(    0x20, "2 gives 1" )
	PORT_DIPSETTING(    0x60, "4 gives 2" )
	PORT_DIPSETTING(    0xa0, "3 gives 1" )
	PORT_DIPSETTING(    0x40, "4 gives 1" )
	PORT_DIPSETTING(    0x80, "5 gives 1" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	/* 0xc0 and 0xe0 None */

	PORT_START("STICKY")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30)

	PORT_START("STICKX")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30)
INPUT_PORTS_END


static INPUT_PORTS_START( esb )
	PORT_INCLUDE( starwars )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x03, 0x03, "Starting Shields" )  PORT_DIPLOCATION("10D:1,2")
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("10D:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, "Moderate" )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, "Jedi-Letter Mode" )  PORT_DIPLOCATION("10D:5,6")
	PORT_DIPSETTING(    0x00, "Level Only" )
	PORT_DIPSETTING(    0x10, "Level" )
	PORT_DIPSETTING(    0x20, "Increment Only" )
	PORT_DIPSETTING(    0x30, "Increment" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("10D:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) // "No Music In Attract Mode" switch 'on'
	PORT_DIPSETTING(    0x40, DEF_STR( On ) ) // "Music In Attract Mode" switch 'off' 
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void starwars_state::starwars(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, MASTER_CLOCK / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &starwars_state::main_map);
	m_maincpu->set_periodic_int(FUNC(starwars_state::irq0_line_assert), attotime::from_hz(CLOCK_3KHZ / 12));

	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_hz(CLOCK_3KHZ / 128));

	MC6809E(config, m_audiocpu, MASTER_CLOCK / 8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &starwars_state::sound_map);

	adc0809_device &adc(ADC0809(config, "adc", MASTER_CLOCK / 16)); // designated as "137243-001" on parts list and "157249-120" on schematics
	adc.in_callback<0>().set_ioport("STICKY"); // pitch
	adc.in_callback<1>().set_ioport("STICKX"); // yaw
	adc.in_callback<2>().set_constant(0); // thrust (unused)

	MOS6532(config, m_riot, MASTER_CLOCK / 8);
	m_riot->pa_wr_callback<0>().set(m_tms, FUNC(tms5220_device::wsq_w));
	m_riot->pa_wr_callback<1>().set(m_tms, FUNC(tms5220_device::rsq_w));
	m_riot->pa_rd_callback<2>().set(m_tms, FUNC(tms5220_device::readyq_r));
	m_riot->pa_wr_callback<3>().set_nop(); // hold main CPU in reset? + enable delay circuit?
	m_riot->pa_rd_callback<4>().set_constant(1); // not sound self test
	m_riot->pa_wr_callback<5>().set_nop(); // TMS5220 VDD
	m_riot->pb_rd_callback().set(m_tms, FUNC(tms5220_device::status_r));
	m_riot->pb_wr_callback().set(m_tms, FUNC(tms5220_device::data_w));
	m_riot->irq_wr_callback().set_inputline(m_audiocpu, M6809_IRQ_LINE);

	X2212(config, "x2212").set_auto_save(true); /* nvram */

	ls259_device &outlatch(LS259(config, "outlatch")); // 9L/M
	outlatch.q_out_cb<0>().set(FUNC(starwars_state::coin1_counter_w)); // Coin counter 1
	outlatch.q_out_cb<1>().set(FUNC(starwars_state::coin2_counter_w)); // Coin counter 2
	outlatch.q_out_cb<2>().set_output("led2").invert(); // LED 3
	outlatch.q_out_cb<3>().set_output("led1").invert(); // LED 2
	outlatch.q_out_cb<4>().set_membank("bank1"); // bank switch
	outlatch.q_out_cb<5>().set(FUNC(starwars_state::prng_reset_w)); // reset PRNG
	outlatch.q_out_cb<6>().set_output("led0").invert(); // LED 1
	outlatch.q_out_cb<7>().set(FUNC(starwars_state::recall_w)); // NVRAM array recall

	/* video hardware */
	VECTOR(config, "vector", 0);
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_VECTOR));
	screen.set_refresh_hz(CLOCK_3KHZ / 12 / 6);
	screen.set_size(400, 300);
	screen.set_visarea(0, 250, 0, 280);
	screen.set_screen_update("vector", FUNC(vector_device::screen_update));

	avg_device &avg(AVG_STARWARS(config, "avg", 0));
	avg.set_vector("vector");
	avg.set_memory(m_maincpu, AS_PROGRAM, 0x0000);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	POKEY(config, m_pokey[0], MASTER_CLOCK / 8).add_route(ALL_OUTPUTS, "mono", 0.20);
	POKEY(config, m_pokey[1], MASTER_CLOCK / 8).add_route(ALL_OUTPUTS, "mono", 0.20);
	POKEY(config, m_pokey[2], MASTER_CLOCK / 8).add_route(ALL_OUTPUTS, "mono", 0.20);
	POKEY(config, m_pokey[3], MASTER_CLOCK / 8).add_route(ALL_OUTPUTS, "mono", 0.20);

	TMS5220(config, m_tms, MASTER_CLOCK/2/9).add_route(ALL_OUTPUTS, "mono", 0.50);

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set(m_riot, FUNC(mos6532_device::pa_bit_w<7>));
	m_soundlatch->data_pending_callback().append([this](int state) { if (state) machine().scheduler().perfect_quantum(attotime::from_usec(100)); });

	GENERIC_LATCH_8(config, m_mainlatch);
	m_mainlatch->data_pending_callback().set(m_riot, FUNC(mos6532_device::pa_bit_w<6>));
	m_mainlatch->data_pending_callback().append([this](int state) { if (state) machine().scheduler().perfect_quantum(attotime::from_usec(100)); });
}


void starwars_state::esb(machine_config &config)
{
	starwars(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &starwars_state::esb_main_map);

	SLAPSTIC(config, m_slapstic, 101);
	m_slapstic->set_range(m_maincpu, AS_PROGRAM, 0x8000, 0x9fff, 0);
	m_slapstic->set_bank(m_slapstic_bank);

	subdevice<ls259_device>("outlatch")->q_out_cb<4>().append_membank("bank2");
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( starwars )
	ROM_REGION( 0x12000, "maincpu", 0 )     /* 2 64k ROM spaces */
	ROM_LOAD( "136021.214.1f", 0x6000, 0x2000, CRC(04f1876e) SHA1(c1d3637cb31ece0890c25f6122d6bcd27e6ffe0c) ) /* ROM 0 bank pages 0 and 1 */
	ROM_CONTINUE(              0x10000, 0x2000 )
	ROM_LOAD( "136021.102.1hj",0x8000, 0x2000, CRC(f725e344) SHA1(f8943b67f2ea032ab9538084756ba86f892be5ca) ) /*  8k ROM 1 bank */
	ROM_LOAD( "136021.203.1jk",0xa000, 0x2000, CRC(f6da0a00) SHA1(dd53b643be856787bbc4da63e5eb132f98f623c3) ) /*  8k ROM 2 bank */
	ROM_LOAD( "136021.104.1kl",0xc000, 0x2000, CRC(7e406703) SHA1(981b505d6e06d7149f8bcb3e81e4d0c790f2fc86) ) /*  8k ROM 3 bank */
	ROM_LOAD( "136021.206.1m", 0xe000, 0x2000, CRC(c7e51237) SHA1(4960f4446271316e3f730eeb2531dbc702947395) ) /*  8k ROM 4 bank */

	ROM_REGION( 0x1000, "vectorrom", 0 )
	ROM_LOAD( "136021-105.1l", 0x0000, 0x1000, CRC(538e7d2f) SHA1(032c933fd94a6b0b294beee29159a24494ae969b) ) /* 3000-3fff is 4k vector rom */

	/* Sound ROMS */
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "136021-107.1jk",0x4000, 0x2000, CRC(dbf3aea2) SHA1(c38661b2b846fe93487eef09ca3cda19c44f08a0) ) /* Sound ROM 0 */
	ROM_RELOAD(                0xc000, 0x2000 )
	ROM_LOAD( "136021-208.1h", 0x6000, 0x2000, CRC(e38070a8) SHA1(c858ae1702efdd48615453ab46e488848891d139) ) /* Sound ROM 0 */
	ROM_RELOAD(                0xe000, 0x2000 )

	ROM_REGION( 0x100, "avg:prom", 0)
	ROM_LOAD( "136021-109.4b", 0x0000, 0x0100, CRC(82fc3eb2) SHA1(184231c7baef598294860a7d2b8a23798c5c7da6) ) /* AVG PROM */

	/* Mathbox PROMs */
	ROM_REGION( 0x1000, "user2", 0 )
	ROM_LOAD( "136021-110.7h", 0x0000, 0x0400, CRC(810e040e) SHA1(d247cbb0afb4538d5161f8ce9eab337cdb3f2da4) ) /* PROM 0 */
	ROM_LOAD( "136021-111.7j", 0x0400, 0x0400, CRC(ae69881c) SHA1(f3420c6e15602956fd94982a5d8d4ddd015ed977) ) /* PROM 1 */
	ROM_LOAD( "136021-112.7k", 0x0800, 0x0400, CRC(ecf22628) SHA1(4dcf5153221feca329b8e8d199bd4fc00b151d9c) ) /* PROM 2 */
	ROM_LOAD( "136021-113.7l", 0x0c00, 0x0400, CRC(83febfde) SHA1(e13541b09d1724204fdb171528e9a1c83c799c1c) ) /* PROM 3 */
ROM_END

ROM_START( starwars1 )
	ROM_REGION( 0x12000, "maincpu", 0 )     /* 2 64k ROM spaces */
	ROM_LOAD( "136021.114.1f", 0x6000, 0x2000, CRC(e75ff867) SHA1(3a40de920c31ffa3c3e67f3edf653b79fcc5ddd7) ) /* ROM 0 bank pages 0 and 1 */
	ROM_CONTINUE(              0x10000, 0x2000 )
	ROM_LOAD( "136021.102.1hj",0x8000, 0x2000, CRC(f725e344) SHA1(f8943b67f2ea032ab9538084756ba86f892be5ca) ) /*  8k ROM 1 bank */
	ROM_LOAD( "136021.203.1jk",0xa000, 0x2000, CRC(f6da0a00) SHA1(dd53b643be856787bbc4da63e5eb132f98f623c3) ) /*  8k ROM 2 bank */
	ROM_LOAD( "136021.104.1kl",0xc000, 0x2000, CRC(7e406703) SHA1(981b505d6e06d7149f8bcb3e81e4d0c790f2fc86) ) /*  8k ROM 3 bank */
	ROM_LOAD( "136021.206.1m", 0xe000, 0x2000, CRC(c7e51237) SHA1(4960f4446271316e3f730eeb2531dbc702947395) ) /*  8k ROM 4 bank */

	ROM_REGION( 0x1000, "vectorrom", 0 )
	ROM_LOAD( "136021-105.1l", 0x0000, 0x1000, CRC(538e7d2f) SHA1(032c933fd94a6b0b294beee29159a24494ae969b) ) /* 3000-3fff is 4k vector rom */

	/* Sound ROMS */
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "136021-107.1jk",0x4000, 0x2000, CRC(dbf3aea2) SHA1(c38661b2b846fe93487eef09ca3cda19c44f08a0) ) /* Sound ROM 0 */
	ROM_RELOAD(                0xc000, 0x2000 )
	ROM_LOAD( "136021-208.1h", 0x6000, 0x2000, CRC(e38070a8) SHA1(c858ae1702efdd48615453ab46e488848891d139) ) /* Sound ROM 0 */
	ROM_RELOAD(                0xe000, 0x2000 )

	ROM_REGION( 0x100, "avg:prom", 0)
	ROM_LOAD( "136021-109.4b", 0x0000, 0x0100, CRC(82fc3eb2) SHA1(184231c7baef598294860a7d2b8a23798c5c7da6) ) /* AVG PROM */

	/* Mathbox PROMs */
	ROM_REGION( 0x1000, "user2", 0)
	ROM_LOAD( "136021-110.7h", 0x0000, 0x0400, CRC(810e040e) SHA1(d247cbb0afb4538d5161f8ce9eab337cdb3f2da4) ) /* PROM 0 */
	ROM_LOAD( "136021-111.7j", 0x0400, 0x0400, CRC(ae69881c) SHA1(f3420c6e15602956fd94982a5d8d4ddd015ed977) ) /* PROM 1 */
	ROM_LOAD( "136021-112.7k", 0x0800, 0x0400, CRC(ecf22628) SHA1(4dcf5153221feca329b8e8d199bd4fc00b151d9c) ) /* PROM 2 */
	ROM_LOAD( "136021-113.7l", 0x0c00, 0x0400, CRC(83febfde) SHA1(e13541b09d1724204fdb171528e9a1c83c799c1c) ) /* PROM 3 */
ROM_END

ROM_START( starwarso )
	ROM_REGION( 0x12000, "maincpu", 0 )     /* 2 64k ROM spaces */
	ROM_LOAD( "136021-114.1f", 0x6000, 0x2000, CRC(e75ff867) SHA1(3a40de920c31ffa3c3e67f3edf653b79fcc5ddd7) ) /* ROM 0 bank pages 0 and 1 */
	ROM_CONTINUE(              0x10000, 0x2000 )
	ROM_LOAD( "136021-102.1hj",0x8000, 0x2000, CRC(f725e344) SHA1(f8943b67f2ea032ab9538084756ba86f892be5ca) ) /*  8k ROM 1 bank */
	ROM_LOAD( "136021-103.1jk",0xa000, 0x2000, CRC(3fde9ccb) SHA1(8d88fc7a28ac8f189f8aba08598732ac8c5491aa) ) /*  8k ROM 2 bank */
	ROM_LOAD( "136021-104.1kl",0xc000, 0x2000, CRC(7e406703) SHA1(981b505d6e06d7149f8bcb3e81e4d0c790f2fc86) ) /*  8k ROM 3 bank */
	ROM_LOAD( "136021-206.1m", 0xe000, 0x2000, CRC(c7e51237) SHA1(4960f4446271316e3f730eeb2531dbc702947395) ) /*  8k ROM 4 bank */

	ROM_REGION( 0x1000, "vectorrom", 0 )
	ROM_LOAD( "136021-105.1l", 0x0000, 0x1000, CRC(538e7d2f) SHA1(032c933fd94a6b0b294beee29159a24494ae969b) ) /* 3000-3fff is 4k vector rom */

	/* Sound ROMS */
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "136021-107.1jk",0x4000, 0x2000, CRC(dbf3aea2) SHA1(c38661b2b846fe93487eef09ca3cda19c44f08a0) ) /* Sound ROM 0 */
	ROM_RELOAD(                0xc000, 0x2000 )
	ROM_LOAD( "136021-208.1h", 0x6000, 0x2000, CRC(e38070a8) SHA1(c858ae1702efdd48615453ab46e488848891d139) ) /* Sound ROM 0 */
	ROM_RELOAD(                0xe000, 0x2000 )

	ROM_REGION( 0x100, "avg:prom", 0)
	ROM_LOAD( "136021-109.4b", 0x0000, 0x0100, CRC(82fc3eb2) SHA1(184231c7baef598294860a7d2b8a23798c5c7da6) ) /* AVG PROM */

	/* Mathbox PROMs */
	ROM_REGION( 0x1000, "user2", 0 )
	ROM_LOAD( "136021-110.7h", 0x0000, 0x0400, CRC(810e040e) SHA1(d247cbb0afb4538d5161f8ce9eab337cdb3f2da4) ) /* PROM 0 */
	ROM_LOAD( "136021-111.7j", 0x0400, 0x0400, CRC(ae69881c) SHA1(f3420c6e15602956fd94982a5d8d4ddd015ed977) ) /* PROM 1 */
	ROM_LOAD( "136021-112.7k", 0x0800, 0x0400, CRC(ecf22628) SHA1(4dcf5153221feca329b8e8d199bd4fc00b151d9c) ) /* PROM 2 */
	ROM_LOAD( "136021-113.7l", 0x0c00, 0x0400, CRC(83febfde) SHA1(e13541b09d1724204fdb171528e9a1c83c799c1c) ) /* PROM 3 */
ROM_END



ROM_START( tomcatsw )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "tc6.1f",        0x6000, 0x2000, CRC(56e284ff) SHA1(a5fda9db0f6b8f7d28a4a607976fe978e62158cf) )
	ROM_LOAD( "tc8.1hj",       0x8000, 0x2000, CRC(7b7575e3) SHA1(bdb838603ffb12195966d0ce454900253bc0f43f) )
	ROM_LOAD( "tca.1jk",       0xa000, 0x2000, CRC(a1020331) SHA1(128745a2ec771ac818a8fbba59a08f0cf5f28e8f) )
	ROM_LOAD( "tce.1m",        0xe000, 0x2000, CRC(4a3de8a3) SHA1(e48fc17201326358317f6b428e583ecaa3ecb881) )

	ROM_REGION( 0x1000, "vectorrom", 0 )
	ROM_LOAD( "tcavg3.1l",     0x0000, 0x1000, CRC(27188aa9) SHA1(5d9a978a7ac1913b57586e81045a1b955db27b48) )

	/* Sound ROMS */
	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "avg:prom", 0)
	ROM_LOAD( "136021-109.4b", 0x0000, 0x0100, CRC(82fc3eb2) SHA1(184231c7baef598294860a7d2b8a23798c5c7da6) ) /* AVG PROM */

	/* Mathbox PROMs */
	ROM_REGION( 0x1000, "user2", 0 )
	ROM_LOAD( "136021-110.7h", 0x0000, 0x0400, CRC(810e040e) SHA1(d247cbb0afb4538d5161f8ce9eab337cdb3f2da4) ) /* PROM 0 */
	ROM_LOAD( "136021-111.7j", 0x0400, 0x0400, CRC(ae69881c) SHA1(f3420c6e15602956fd94982a5d8d4ddd015ed977) ) /* PROM 1 */
	ROM_LOAD( "136021-112.7k", 0x0800, 0x0400, CRC(ecf22628) SHA1(4dcf5153221feca329b8e8d199bd4fc00b151d9c) ) /* PROM 2 */
	ROM_LOAD( "136021-113.7l", 0x0c00, 0x0400, CRC(83febfde) SHA1(e13541b09d1724204fdb171528e9a1c83c799c1c) ) /* PROM 3 */
ROM_END


ROM_START( esb )
	ROM_REGION( 0x22000, "maincpu", 0 )     /* 64k for code and a buttload for the banked ROMs */
	ROM_LOAD( "136031-101.1f", 0x6000, 0x2000, CRC(ef1e3ae5) SHA1(d228ff076faa7f9605badeee3b827adb62593e0a) )
	ROM_CONTINUE(              0x10000, 0x2000 )
	/* $8000 - $9fff : slapstic page */
	ROM_LOAD( "136031-102.1jk",0xa000, 0x2000, CRC(62ce5c12) SHA1(976256acf4499dc396542a117910009a8808f448) )
	ROM_CONTINUE(              0x1c000, 0x2000 )
	ROM_LOAD( "136031-203.1kl",0xc000, 0x2000, CRC(27b0889b) SHA1(a13074e83f0f57d65096d7f49ae78f33ab00c479) )
	ROM_CONTINUE(              0x1e000, 0x2000 )
	ROM_LOAD( "136031-104.1m", 0xe000, 0x2000, CRC(fd5c725e) SHA1(541cfd004b1736b6cec13836dfa813f00eedeed0) )
	ROM_CONTINUE(              0x20000, 0x2000 )

	ROM_LOAD( "136031-105.3u", 0x14000, 0x4000, CRC(ea9e4dce) SHA1(9363fd5b1fce62c2306b448a7766eaf7ec97cdf5) ) /* slapstic 0, 1 */
	ROM_LOAD( "136031-106.2u", 0x18000, 0x4000, CRC(76d07f59) SHA1(44dd018b406f95e1512ce92923c2c87f1458844f) ) /* slapstic 2, 3 */

	ROM_REGION( 0x1000, "vectorrom", 0 )
	ROM_LOAD( "136031-111.1l", 0x0000, 0x1000, CRC(b1f9bd12) SHA1(76f15395c9fdcd80dd241307a377031a1f44e150) ) /* 3000-3fff is 4k vector rom */

	/* Sound ROMS */
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "136031-113.1jk",0x4000, 0x2000, CRC(24ae3815) SHA1(b1a93af76de79b902317eebbc50b400b1f8c1e3c) ) /* Sound ROM 0 */
	ROM_CONTINUE(              0xc000, 0x2000 )
	ROM_LOAD( "136031-112.1h", 0x6000, 0x2000, CRC(ca72d341) SHA1(52de5b82bb85d7c9caad2047e540d0748aa93ba5) ) /* Sound ROM 1 */
	ROM_CONTINUE(              0xe000, 0x2000 )

	ROM_REGION( 0x100, "avg:prom", 0)
	ROM_LOAD( "136021-109.4b", 0x0000, 0x0100, CRC(82fc3eb2) SHA1(184231c7baef598294860a7d2b8a23798c5c7da6) ) /* AVG PROM */

	/* Mathbox PROMs */
	ROM_REGION( 0x1000, "user2", 0 )
	ROM_LOAD( "136031-110.7h", 0x0000, 0x0400, CRC(b8d0f69d) SHA1(c196f1a592bd1ac482a81e23efa224d9dfaefc0a) ) /* PROM 0 */
	ROM_LOAD( "136031-109.7j", 0x0400, 0x0400, CRC(6a2a4d98) SHA1(cefca71f025f92a193c5a7d8b5ab8be10db2fd44) ) /* PROM 1 */
	ROM_LOAD( "136031-108.7k", 0x0800, 0x0400, CRC(6a76138f) SHA1(9ef7af898a3e29d03f35045901023615a6a55205) ) /* PROM 2 */
	ROM_LOAD( "136031-107.7l", 0x0c00, 0x0400, CRC(afbf6e01) SHA1(0a6438e6c106d98e5d67a019751e1584324f5e5c) ) /* PROM 3 */
ROM_END



/*************************************
 *
 *  Driver init
 *
 *************************************/

void starwars_state::init_starwars()
{
	/* prepare the mathbox */
	starwars_mproc_init();

	/* initialize banking */
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0x6000, 0x10000 - 0x6000);
	membank("bank1")->set_entry(0);
}


void starwars_state::init_esb()
{
	uint8_t *rom = memregion("maincpu")->base();

	/* init the slapstic */
	m_slapstic_bank->configure_entries(0, 4, memregion("maincpu")->base() + 0x14000, 0x2000);

	/* prepare the matrix processor */
	starwars_mproc_init();

	/* initialize banking */
	membank("bank1")->configure_entries(0, 2, rom + 0x6000, 0x10000 - 0x6000);
	membank("bank1")->set_entry(0);
	membank("bank2")->configure_entries(0, 2, rom + 0xa000, 0x1c000 - 0xa000);
	membank("bank2")->set_entry(0);
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, starwars, 0,        starwars, starwars, starwars_state, init_starwars, ROT0, "Atari", "Star Wars (set 1)", 0 ) // newest
GAME( 1983, starwars1,starwars, starwars, starwars, starwars_state, init_starwars, ROT0, "Atari", "Star Wars (set 2)", 0 )
GAME( 1983, starwarso,starwars, starwars, starwars, starwars_state, init_starwars, ROT0, "Atari", "Star Wars (set 3)", 0 ) // oldest
// is there an even older starwars set with 136021-106.1m ?

GAME( 1983, tomcatsw, tomcat,   starwars, starwars, starwars_state, init_starwars, ROT0, "Atari", "TomCat (Star Wars hardware, prototype)", MACHINE_NO_SOUND )

GAME( 1985, esb,      0,        esb,      esb,      starwars_state, init_esb,      ROT0, "Atari Games", "The Empire Strikes Back", 0 )

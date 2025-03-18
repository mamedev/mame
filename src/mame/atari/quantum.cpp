// license:BSD-3-Clause
// copyright-holders:Hedley Rainnie, Aaron Giles, Couriersud, Paul Forgey
/***************************************************************************

    Atari Quantum hardware

    driver by Paul Forgey, with some help from Aaron Giles

    Games supported:
        * Quantum

    Known bugs:
        * none at this time

NOTE: The Atari 136002-125 PROM in the sets below wasn't dumped from an actual
      Quantum PCB. It is assumed all Atari 136002-125 PROMs are the same data.

****************************************************************************

    Memory map

****************************************************************************

    QUANTUM MEMORY MAP (per schem):

    000000-003FFF   ROM0
    004000-004FFF   ROM1
    008000-00BFFF   ROM2
    00C000-00FFFF   ROM3
    010000-013FFF   ROM4

    018000-01BFFF   RAM0
    01C000-01CFFF   RAM1

    940000          TRACKBALL
    948000          SWITCHES
    950000          COLORRAM
    958000          CONTROL (LED and coin control)
    960000-970000   RECALL (nvram read)
    968000          VGRST (vector reset)
    970000          VGGO (vector go)
    978000          WDCLR (watchdog)
    900000          NVRAM (nvram write)
    840000          I/OS (sound and dip switches)
    800000-801FFF   VMEM (vector display list)
    940000          I/O (shematic label really - covered above)
    900000          DTACK1

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/watchdog.h"
#include "machine/x2212.h"
#include "sound/discrete.h"
#include "sound/pokey.h"
#include "video/avgdvg.h"
#include "video/vector.h"

#include "screen.h"
#include "speaker.h"


namespace {

class quantum_state : public driver_device
{
public:
	quantum_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_avg(*this, "avg"),
		m_nvram(*this, "nvram"),
		m_leds(*this, "led%u", 0U)
	{ }

	void quantum(machine_config &config);

protected:
	virtual void machine_start() override { m_leds.resolve(); }

private:
	uint16_t trackball_r();
	void led_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void nvram_recall_w(uint16_t data);
	uint8_t input_1_r(offs_t offset);
	uint8_t input_2_r(offs_t offset);
	void main_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<avg_quantum_device> m_avg;
	required_device<x2212_device> m_nvram;
	output_finder<2> m_leds;
};


static constexpr XTAL MASTER_CLOCK = 12.096_MHz_XTAL;
static constexpr XTAL CLOCK_3KHZ   = MASTER_CLOCK / 4096;


/*************************************
 *
 *  Inputs
 *
 *************************************/

uint16_t quantum_state::trackball_r()
{
	return (ioport("TRACKY")->read() << 4) | ioport("TRACKX")->read();
}


uint8_t quantum_state::input_1_r(offs_t offset)
{
	return (ioport("DSW0")->read() << (7 - (offset - pokey_device::POT0_C))) & 0x80;
}


uint8_t quantum_state::input_2_r(offs_t offset)
{
	return (ioport("DSW1")->read() << (7 - (offset - pokey_device::POT0_C))) & 0x80;
}



/*************************************
 *
 *  LEDs/coin counters
 *
 *************************************/

void quantum_state::led_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bits 0 and 1 are coin counters */
		machine().bookkeeping().coin_counter_w(0, data & 2);
		machine().bookkeeping().coin_counter_w(1, data & 1);

		m_nvram->store(BIT(data, 2));

		/* bit 3 = select second trackball for cocktail mode? */

		/* bits 4 and 5 are LED controls */
		m_leds[0] = BIT(data, 4);
		m_leds[1] = BIT(data, 5);

		/* bits 6 and 7 flip screen */
		m_avg->set_flip_x(data & 0x40);
		m_avg->set_flip_y(data & 0x80);
	}
}

void quantum_state::nvram_recall_w(uint16_t data)
{
	m_nvram->recall(1);
	m_nvram->recall(0);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void quantum_state::main_map(address_map &map)
{
	map(0x000000, 0x013fff).rom();
	map(0x018000, 0x01cfff).ram();
	map(0x800000, 0x801fff).ram(); // vector RAM
	map(0x840000, 0x84001f).rw("pokey1", FUNC(pokey_device::read), FUNC(pokey_device::write)).umask16(0x00ff);
	map(0x840020, 0x84003f).rw("pokey2", FUNC(pokey_device::read), FUNC(pokey_device::write)).umask16(0x00ff);
	map(0x900000, 0x9001ff).rw("nvram", FUNC(x2212_device::read), FUNC(x2212_device::write)).umask16(0x00ff);
	map(0x940000, 0x940001).r(FUNC(quantum_state::trackball_r)); // trackball
	map(0x948000, 0x948001).portr("SYSTEM");
	map(0x950000, 0x95001f).writeonly().share("avg:colorram");
	map(0x958000, 0x958001).w(FUNC(quantum_state::led_w));
	map(0x960000, 0x960001).w(FUNC(quantum_state::nvram_recall_w));
	map(0x968000, 0x968001).w(m_avg, FUNC(avg_quantum_device::reset_word_w));
	map(0x970000, 0x970001).w(m_avg, FUNC(avg_quantum_device::go_word_w));
	map(0x978000, 0x978001).nopr().w("watchdog", FUNC(watchdog_timer_device::reset16_w));
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( quantum )
	PORT_START("SYSTEM")
	/* YHALT here MUST BE ALWAYS 0  */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("avg", FUNC(avg_quantum_device::done_r)) // vg YHALT
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* first POKEY is SW2, second is SW1 -- more confusion! */
	PORT_START("DSW0")
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x30, 0x00, "Right Coin" )
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x20, "*4" )
	PORT_DIPSETTING(    0x10, "*5" )
	PORT_DIPSETTING(    0x30, "*6" )
	PORT_DIPNAME( 0x08, 0x00, "Left Coin" )
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x08, "*2" )
	PORT_DIPNAME( 0x07, 0x00, "Bonus Coins" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x01, "1 each 5" )
	PORT_DIPSETTING(    0x02, "1 each 4" )
	PORT_DIPSETTING(    0x05, "1 each 3" )
	PORT_DIPSETTING(    0x06, "2 each 4" )

	PORT_START("DSW1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("TRACKX")
	PORT_BIT( 0x0f, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("TRACKY")
	PORT_BIT( 0x0f, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(10)
INPUT_PORTS_END


/*************************************
 *
 *  Discrete Sound Blocks
 *
 *************************************/

static discrete_op_amp_filt_info pokey1_info = {
		RES_K(220), 0, 0, 0, /* r1 .. r4 */
		RES_K(220),          /* rF */
		CAP_U(0.022),        /* C1 */
		CAP_U(0.1),          /* C2 */
		0,                   /* C3 */
		0.0,                 /* vRef */
		15.0,                /* vP */
		-15.0,               /* vN */
};

static discrete_mixer_desc quantum_mixer = {
		DISC_MIXER_IS_OP_AMP, /* type */
		{ RES_K(220), RES_K(220) }, /* r{} */
		{},                         /* r_node */
		{ CAP_U(0.1), CAP_U(0.1) }, /* c{} */
		0,                          /* rI  */
		RES_K(220),                 /* rF  */
		0,                          /* cF  */
		0,                          /* cAmp */
		0,                          /* vRef */
		1.0                         /* gain */
};

static DISCRETE_SOUND_START(quantum_discrete)

	/************************************************/
	/* FINAL MIX                                    */
	/************************************************/

	/* Convert Pokey output to 5V Signal */
	DISCRETE_INPUTX_STREAM(NODE_100, 0, 5.0 / 32768, 5.0)   /* Add VRef again */
	DISCRETE_INPUTX_STREAM(NODE_110, 1, 5.0 / 32768, 5.0)   /* Add VRef again */

	DISCRETE_OP_AMP_FILTER(NODE_150, 1, NODE_100, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1, &pokey1_info)

	DISCRETE_MIXER2(NODE_290, 1, NODE_150, NODE_110, &quantum_mixer)
	DISCRETE_OUTPUT(NODE_290, 8192)

DISCRETE_SOUND_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void quantum_state::quantum(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, MASTER_CLOCK / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &quantum_state::main_map);
	m_maincpu->set_periodic_int(FUNC(quantum_state::irq1_line_hold), attotime::from_hz(CLOCK_3KHZ / 12));

	X2212(config, "nvram"); // "137288-001" in parts list and schematic diagram

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	VECTOR(config, "vector");
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_VECTOR));
	screen.set_refresh_hz(60);
	screen.set_size(400, 300);
	screen.set_visarea(0, 900, 0, 600);
	screen.set_screen_update("vector", FUNC(vector_device::screen_update));

	AVG_QUANTUM(config, m_avg, 0);
	m_avg->set_vector("vector");
	m_avg->set_memory(m_maincpu, AS_PROGRAM, 0x800000);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	pokey_device &pokey1(POKEY(config, "pokey1", 600000));
	pokey1.pot_r<0>().set(FUNC(quantum_state::input_1_r));
	pokey1.pot_r<1>().set(FUNC(quantum_state::input_1_r));
	pokey1.pot_r<2>().set(FUNC(quantum_state::input_1_r));
	pokey1.pot_r<3>().set(FUNC(quantum_state::input_1_r));
	pokey1.pot_r<4>().set(FUNC(quantum_state::input_1_r));
	pokey1.pot_r<5>().set(FUNC(quantum_state::input_1_r));
	pokey1.pot_r<6>().set(FUNC(quantum_state::input_1_r));
	pokey1.pot_r<7>().set(FUNC(quantum_state::input_1_r));
	pokey1.set_output_opamp(RES_K(1), 0.0, 5.0);
	pokey1.add_route(0, "discrete", 1.0, 0);

	pokey_device &pokey2(POKEY(config, "pokey2", 600000));
	pokey2.pot_r<0>().set(FUNC(quantum_state::input_2_r));
	pokey2.pot_r<1>().set(FUNC(quantum_state::input_2_r));
	pokey2.pot_r<2>().set(FUNC(quantum_state::input_2_r));
	pokey2.pot_r<3>().set(FUNC(quantum_state::input_2_r));
	pokey2.pot_r<4>().set(FUNC(quantum_state::input_2_r));
	pokey2.pot_r<5>().set(FUNC(quantum_state::input_2_r));
	pokey2.pot_r<6>().set(FUNC(quantum_state::input_2_r));
	pokey2.pot_r<7>().set(FUNC(quantum_state::input_2_r));
	pokey2.set_output_opamp(RES_K(1), 0.0, 5.0);
	pokey2.add_route(0, "discrete", 1.0, 1);

	DISCRETE(config, "discrete", quantum_discrete).add_route(ALL_OUTPUTS, "mono", 0.50);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( quantum )
	ROM_REGION( 0x014000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "136016-201.2e",   0x000000, 0x002000, CRC(7e7be63a) SHA1(11b2d0168cdbaa7a48656b77abc0bcbe9408fe84) )
	ROM_LOAD16_BYTE( "136016-206.3e",   0x000001, 0x002000, CRC(2d8f5759) SHA1(54b0388ef44b5d34e621b48b465566aa16887e8f) )
	ROM_LOAD16_BYTE( "136016-102.2f",   0x004000, 0x002000, CRC(408d34f4) SHA1(9a30debd1240b9c103134701943c94d6b48b926d) )
	ROM_LOAD16_BYTE( "136016-107.3f",   0x004001, 0x002000, CRC(63154484) SHA1(c098cdbc339c9ea291c4c4fb203c60b3284e894a) )
	ROM_LOAD16_BYTE( "136016-203.2hj",  0x008000, 0x002000, CRC(bdc52fad) SHA1(c8ede54a4f7f555adffa5b4bfea6bf646a0d02d4) )
	ROM_LOAD16_BYTE( "136016-208.3hj",  0x008001, 0x002000, CRC(dab4066b) SHA1(dbb82df8e6de4e0f9f6e7ddd5f07618864fce8f9) )
	ROM_LOAD16_BYTE( "136016-104.2k",   0x00C000, 0x002000, CRC(bf271e5c) SHA1(012edb947f1437932b9283e49d025a7794c45669) )
	ROM_LOAD16_BYTE( "136016-109.3k",   0x00C001, 0x002000, CRC(d2894424) SHA1(5390025136b677b66d948c8cf6ea5e20203a4bae) )
	ROM_LOAD16_BYTE( "136016-105.2l",   0x010000, 0x002000, CRC(13ec512c) SHA1(22a0395135b83ba47eacb5129f34fc97aa1b70a1) )
	ROM_LOAD16_BYTE( "136016-110.3l",   0x010001, 0x002000, CRC(acb50363) SHA1(9efa9ca88efdd2d5e212bd537903892b67b4fe53) )
	/* AVG PROM */
	ROM_REGION( 0x100, "avg:prom", 0 )
	ROM_LOAD( "136002-125.6h",   0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "cf2038n.1b",   0x0000, 0x00eb, CRC(b372fa4f) SHA1(a60b51849e9f691b412ae4c4afc834ff93d8a30f) ) /* Original chip is a 82S153, schematics refer to this chip as 137290-001 */
ROM_END


ROM_START( quantum1 )
	ROM_REGION( 0x014000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "136016-101.2e",   0x000000, 0x002000, CRC(5af0bd5b) SHA1(f6e46fbebbf52294e78ae240fe2628c6b29b8dea) )
	ROM_LOAD16_BYTE( "136016-106.3e",   0x000001, 0x002000, CRC(f9724666) SHA1(1bb073135029c92bef9afc9ccd910e0ab3302c8a) )
	ROM_LOAD16_BYTE( "136016-102.2f",   0x004000, 0x002000, CRC(408d34f4) SHA1(9a30debd1240b9c103134701943c94d6b48b926d) )
	ROM_LOAD16_BYTE( "136016-107.3f",   0x004001, 0x002000, CRC(63154484) SHA1(c098cdbc339c9ea291c4c4fb203c60b3284e894a) )
	ROM_LOAD16_BYTE( "136016-103.2hj",  0x008000, 0x002000, CRC(948f228b) SHA1(878ac96173a793997cc88be469ec1ccdf833a7e8) )
	ROM_LOAD16_BYTE( "136016-108.3hj",  0x008001, 0x002000, CRC(e4c48e4e) SHA1(caaf9d20741fcb961d590b634250a44a166cc33a) )
	ROM_LOAD16_BYTE( "136016-104.2k",   0x00C000, 0x002000, CRC(bf271e5c) SHA1(012edb947f1437932b9283e49d025a7794c45669) )
	ROM_LOAD16_BYTE( "136016-109.3k",   0x00C001, 0x002000, CRC(d2894424) SHA1(5390025136b677b66d948c8cf6ea5e20203a4bae) )
	ROM_LOAD16_BYTE( "136016-105.2l",   0x010000, 0x002000, CRC(13ec512c) SHA1(22a0395135b83ba47eacb5129f34fc97aa1b70a1) )
	ROM_LOAD16_BYTE( "136016-110.3l",   0x010001, 0x002000, CRC(acb50363) SHA1(9efa9ca88efdd2d5e212bd537903892b67b4fe53) )
	/* AVG PROM */
	ROM_REGION( 0x100, "avg:prom", 0 )
	ROM_LOAD( "136002-125.6h",   0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "cf2038n.1b",   0x0000, 0x00eb, CRC(b372fa4f) SHA1(a60b51849e9f691b412ae4c4afc834ff93d8a30f) ) /* Original chip is a 82S153, schematics refer to this chip as 137290-001 */
ROM_END


ROM_START( quantump )
	ROM_REGION( 0x014000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "quantump.2e",  0x000000, 0x002000, CRC(176d73d3) SHA1(b887ee50af5db6f6d43cc6ba57451173f996dedc) )
	ROM_LOAD16_BYTE( "quantump.3e",  0x000001, 0x002000, CRC(12fc631f) SHA1(327a44da897199536f43e5f792cb4a18d9055ac4) )
	ROM_LOAD16_BYTE( "quantump.2f",  0x004000, 0x002000, CRC(b64fab48) SHA1(d5a77a367d4f652261c381e6bdd55c2175ace857) )
	ROM_LOAD16_BYTE( "quantump.3f",  0x004001, 0x002000, CRC(a52a9433) SHA1(33787adb04864efebb04483353bbc96c966ec607) )
	ROM_LOAD16_BYTE( "quantump.2hj", 0x008000, 0x002000, CRC(5b29cba3) SHA1(e83b68907bc397994ed51a39dfa241430a0adb0c) )
	ROM_LOAD16_BYTE( "quantump.3hj", 0x008001, 0x002000, CRC(c64fc03a) SHA1(ab6cd710d01bc85432cc52021f27fd8f2a5e3168) )
	ROM_LOAD16_BYTE( "quantump.2k",  0x00C000, 0x002000, CRC(854f9c09) SHA1(d908b8c7f6837e511004cbd45a8883c6c7b155dd) )
	ROM_LOAD16_BYTE( "quantump.3k",  0x00C001, 0x002000, CRC(1aac576c) SHA1(28bdb5fcbd8cccc657d6e00ace3c083c21015564) )
	ROM_LOAD16_BYTE( "quantump.2l",  0x010000, 0x002000, CRC(1285b5e7) SHA1(0e01e361da2d9cf1fac1896f8f44c4c2e75a3061) )
	ROM_LOAD16_BYTE( "quantump.3l",  0x010001, 0x002000, CRC(e19de844) SHA1(cb4f9d80807b26d6b95405b2d830799984667f54) )
	/* AVG PROM */
	ROM_REGION( 0x100, "avg:prom", 0 )
	ROM_LOAD( "136002-125.6h",   0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "cf2038n.1b",   0x0000, 0x00eb, CRC(b372fa4f) SHA1(a60b51849e9f691b412ae4c4afc834ff93d8a30f) ) /* Original chip is a 82S153, schematics refer to this chip as 137290-001 */
ROM_END

} // anonoymous namespace



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1982, quantum,  0,       quantum, quantum, quantum_state, empty_init, ROT270, "General Computer Corporation (Atari license)", "Quantum (rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, quantum1, quantum, quantum, quantum, quantum_state, empty_init, ROT270, "General Computer Corporation (Atari license)", "Quantum (rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, quantump, quantum, quantum, quantum, quantum_state, empty_init, ROT270, "General Computer Corporation (Atari license)", "Quantum (prototype)", MACHINE_SUPPORTS_SAVE )

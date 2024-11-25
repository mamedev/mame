// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Bally/Midway Jr. Pac-Man

    Games supported:
        * Jr. Pac-Man

    Known issues:
        * none

****************************************************************************

    Jr. Pac Man memory map (preliminary)

    0000-3fff ROM
    4000-47ff Video RAM (also color RAM)
    4800-4fff RAM
    8000-dfff ROM

    memory mapped ports:

    read:
    5000      P1
    5040      P2
    5080      DSW

    *
     * IN0 (all bits are inverted)
     * bit 7 : CREDIT
     * bit 6 : COIN 2
     * bit 5 : COIN 1
     * bit 4 : RACK TEST
     * bit 3 : DOWN player 1
     * bit 2 : RIGHT player 1
     * bit 1 : LEFT player 1
     * bit 0 : UP player 1
     *
    *
     * IN1 (all bits are inverted)
     * bit 7 : TABLE or UPRIGHT cabinet select (1 = UPRIGHT)
     * bit 6 : START 2
     * bit 5 : START 1
     * bit 4 : TEST SWITCH
     * bit 3 : DOWN player 2 (TABLE only)
     * bit 2 : RIGHT player 2 (TABLE only)
     * bit 1 : LEFT player 2 (TABLE only)
     * bit 0 : UP player 2 (TABLE only)
     *
    *
     * DSW1 (all bits are inverted)
     * bit 7 :  ?
     * bit 6 :  difficulty level
     *                       1 = Normal  0 = Harder
     * bit 5 :\ bonus pac at xx000 pts
     * bit 4 :/ 00 = 10000  01 = 15000  10 = 20000  11 = 30000
     * bit 3 :\ nr of lives
     * bit 2 :/ 00 = 1  01 = 2  10 = 3  11 = 5
     * bit 1 :\ play mode
     * bit 0 :/ 00 = free play   01 = 1 coin 1 credit
     *          10 = 1 coin 2 credits   11 = 2 coins 1 credit
     *

    write:
    4ff2-4ffd 6 pairs of two bytes:
              the first byte contains the sprite image number (bits 2-7), Y flip (bit 0),
              X flip (bit 1); the second byte the color
    5000      interrupt enable
    5001      sound enable
    5002      unused
    5003      flip screen
    5004      unused
    5005      unused
    5006      unused
    5007      coin counter
    5040-5044 sound voice 1 accumulator (nibbles) (used by the sound hardware only)
    5045      sound voice 1 waveform (nibble)
    5046-5049 sound voice 2 accumulator (nibbles) (used by the sound hardware only)
    504a      sound voice 2 waveform (nibble)
    504b-504e sound voice 3 accumulator (nibbles) (used by the sound hardware only)
    504f      sound voice 3 waveform (nibble)
    5050-5054 sound voice 1 frequency (nibbles)
    5055      sound voice 1 volume (nibble)
    5056-5059 sound voice 2 frequency (nibbles)
    505a      sound voice 2 volume (nibble)
    505b-505e sound voice 3 frequency (nibbles)
    505f      sound voice 3 volume (nibble)
    5062-506d Sprite coordinates, x/y pairs for 6 sprites
    5070      palette bank
    5071      colortable bank
    5073      background priority over sprites
    5074      char gfx bank
    5075      sprite gfx bank
    5080      scroll
    50c0      Watchdog reset

    I/O ports:
    OUT on port $0 sets the interrupt vector

***************************************************************************/

#include "emu.h"
#include "pacman.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "screen.h"
#include "speaker.h"


namespace {

class jrpacman_state : public pacman_state
{
public:
	jrpacman_state(const machine_config &mconfig, device_type type, const char *tag)
		: pacman_state(mconfig, type, tag) { }

	void jrpacman(machine_config &config);

	void init_jrpacman();

private:
	void main_map(address_map &map) ATTR_COLD;
	void port_map(address_map &map) ATTR_COLD;
};



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void jrpacman_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram().w(FUNC(jrpacman_state::jrpacman_videoram_w)).share("videoram");
	map(0x4800, 0x4fef).ram();
	map(0x4ff0, 0x4fff).ram().share("spriteram");
	map(0x5000, 0x503f).portr("P1");
	map(0x5000, 0x5007).w("latch1", FUNC(ls259_device::write_d0));
	map(0x5040, 0x507f).portr("P2");
	map(0x5040, 0x505f).w(m_namco_sound, FUNC(namco_device::pacman_sound_w));
	map(0x5060, 0x506f).writeonly().share("spriteram2");
	map(0x5070, 0x5077).w("latch2", FUNC(ls259_device::write_d0));
	map(0x5080, 0x50bf).portr("DSW");
	map(0x5080, 0x5080).w(FUNC(jrpacman_state::jrpacman_scroll_w));
	map(0x50c0, 0x50c0).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x8000, 0xdfff).rom();
}


void jrpacman_state::port_map(address_map &map)
{
	map.global_mask(0xff);
	map(0, 0).w(FUNC(jrpacman_state::pacman_interrupt_vector_w));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( jrpacman )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x10, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x30, "30000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout tilelayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ 0, 4 },
	{ STEP4(8*8,1), STEP4(0*8,1) },
	{ STEP8(0*8,8) },
	16*8
};


static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ 0, 4 },
	{ STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1), STEP4(0*8,1) },
	{ STEP8(0*8,8), STEP8(32*8,8) },
	64*8
};


static GFXDECODE_START( gfx_jrpacman )
	GFXDECODE_ENTRY( "gfx1", 0x0000, tilelayout,   0, 128 )
	GFXDECODE_ENTRY( "gfx1", 0x2000, spritelayout, 0, 128 )
GFXDECODE_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void jrpacman_state::jrpacman(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 18432000/6);    /* 3.072 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &jrpacman_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &jrpacman_state::port_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(jrpacman_state::interrupt_vector_r));

	ls259_device &latch1(LS259(config, "latch1")); // 5P
	latch1.q_out_cb<0>().set(FUNC(jrpacman_state::irq_mask_w));
	latch1.q_out_cb<1>().set("namco", FUNC(namco_device::sound_enable_w));
	latch1.q_out_cb<3>().set(FUNC(jrpacman_state::flipscreen_w));
	latch1.q_out_cb<7>().set(FUNC(jrpacman_state::coin_counter_w));

	ls259_device &latch2(LS259(config, "latch2")); // 1H
	latch2.q_out_cb<0>().set(FUNC(jrpacman_state::pengo_palettebank_w));
	latch2.q_out_cb<1>().set(FUNC(jrpacman_state::pengo_colortablebank_w));
	latch2.q_out_cb<3>().set(FUNC(jrpacman_state::jrpacman_bgpriority_w));
	latch2.q_out_cb<4>().set(FUNC(jrpacman_state::jrpacman_charbank_w));
	latch2.q_out_cb<5>().set(FUNC(jrpacman_state::jrpacman_spritebank_w));

	WATCHDOG_TIMER(config, m_watchdog);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60.606060);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(36*8, 28*8);
	screen.set_visarea(0*8, 36*8-1, 0*8, 28*8-1);
	screen.set_screen_update(FUNC(jrpacman_state::screen_update_pacman));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(jrpacman_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_jrpacman);
	PALETTE(config, m_palette, FUNC(jrpacman_state::pacman_palette), 128 * 4, 32);

	MCFG_VIDEO_START_OVERRIDE(jrpacman_state,jrpacman)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	NAMCO(config, m_namco_sound, 3072000/32);
	m_namco_sound->set_voices(3);
	m_namco_sound->add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

/*

Jr. Pac-Man (11/9/83)

Label format:
+------------+
| JR.PAC-MAN |
|     8D     |
|   11/9/83  |
| @BALLY/MDWY|  <-- the "@" is actually the copyright "circled C"
+------------+

*/
ROM_START( jrpacman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jr.pac-man_8d_11-9-83.8d",    0x0000, 0x2000, CRC(e3fa972e) SHA1(5ea34621213c649ca2848ab31aab2cbe751723d4) )
	ROM_LOAD( "jr.pac-man_8e_11-9-83.8e",    0x2000, 0x2000, CRC(ec889e94) SHA1(8294e9e79f8fd19a419431fa690e6ac4a1302f58) )
	ROM_LOAD( "jr.pac-man_8h_11-9-83.8h",    0x8000, 0x2000, CRC(35f1fc6e) SHA1(b84b34560b9aae18b24274712b052283faa01730) )
	ROM_LOAD( "jr.pac-man_8j_11-9-83.8j",    0xa000, 0x2000, CRC(9737099e) SHA1(07d912a61824323c8fc1b8bd0da89172d4f70b91) )
	ROM_LOAD( "jr.pac-man_8k_11-9-83.8k",    0xc000, 0x2000, CRC(5252dd97) SHA1(18bd4d5381656120e4242811006c20776774de4d) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "jr.pac-man_2c_11-9-83.2c",    0x0000, 0x2000, CRC(0527ff9b) SHA1(37fe3176b0d125b7d629e108e7ebdc1196e4a132) ) /* tiles (bank 1 & 2) */
	ROM_LOAD( "jr.pac-man_2e_11-9-83.2e",    0x2000, 0x2000, CRC(73477193) SHA1(f00a488958ea0438642d345693787bdf771219ad) ) /* sprites (bank 1 & 2) */

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD_NIB_LOW ( "a290-27axv-bxhd.9e", 0x0000, 0x0100, CRC(029d35c4) SHA1(d9aa2dc442e9ac36cf3c346b9fb1aa745eaf3cb8) ) /* color palette (low bits) */
	ROM_LOAD_NIB_HIGH( "a290-27axv-cxhd.9f", 0x0000, 0x0100, CRC(eee34a79) SHA1(7561f8ccab2af85c111af6a02af6986eb67503e5) ) /* color palette (high bits) */
	ROM_LOAD( "a290-27axv-axhd.9p",          0x0020, 0x0100, CRC(9f6ea9d8) SHA1(62cf15513934d34641433c891a7f73bef82e2fb1) ) /* color lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "a290-27axv-dxhd.7p",          0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) /* waveform */
	ROM_LOAD( "a290-27axv-exhd.5s",          0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */
ROM_END

ROM_START( jrpacmanf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fast_jr.8d",                  0x0000, 0x2000, CRC(461e8b57) SHA1(42e25d384e653efb95a97bd64f55a8c3b3f71239) ) // only 1 byte difference
	ROM_LOAD( "jr.pac-man_8e_11-9-83.8e",    0x2000, 0x2000, CRC(ec889e94) SHA1(8294e9e79f8fd19a419431fa690e6ac4a1302f58) )
	ROM_LOAD( "jr.pac-man_8h_11-9-83.8h",    0x8000, 0x2000, CRC(35f1fc6e) SHA1(b84b34560b9aae18b24274712b052283faa01730) )
	ROM_LOAD( "jr.pac-man_8j_11-9-83.8j",    0xa000, 0x2000, CRC(9737099e) SHA1(07d912a61824323c8fc1b8bd0da89172d4f70b91) )
	ROM_LOAD( "jr.pac-man_8k_11-9-83.8k",    0xc000, 0x2000, CRC(5252dd97) SHA1(18bd4d5381656120e4242811006c20776774de4d) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "jr.pac-man_2c_11-9-83.2c",    0x0000, 0x2000, CRC(0527ff9b) SHA1(37fe3176b0d125b7d629e108e7ebdc1196e4a132) ) /* tiles (bank 1 & 2) */
	ROM_LOAD( "jr.pac-man_2e_11-9-83.2e",    0x2000, 0x2000, CRC(73477193) SHA1(f00a488958ea0438642d345693787bdf771219ad) ) /* sprites (bank 1 & 2) */

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD_NIB_LOW ( "a290-27axv-bxhd.9e", 0x0000, 0x0100, CRC(029d35c4) SHA1(d9aa2dc442e9ac36cf3c346b9fb1aa745eaf3cb8) ) /* color palette (low bits) */
	ROM_LOAD_NIB_HIGH( "a290-27axv-cxhd.9f", 0x0000, 0x0100, CRC(eee34a79) SHA1(7561f8ccab2af85c111af6a02af6986eb67503e5) ) /* color palette (high bits) */
	ROM_LOAD( "a290-27axv-axhd.9p",          0x0020, 0x0100, CRC(9f6ea9d8) SHA1(62cf15513934d34641433c891a7f73bef82e2fb1) ) /* color lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "a290-27axv-dxhd.7p",          0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) /* waveform */
	ROM_LOAD( "a290-27axv-exhd.5s",          0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void jrpacman_state::init_jrpacman()
{
	/* The encryption PALs garble bits 0, 2 and 7 of the ROMs. The encryption */
	/* scheme is complex (basically it's a state machine) and can only be */
	/* faithfully emulated at run time. To avoid the performance hit that would */
	/* cause, here we have a table of the values which must be XORed with */
	/* each memory region to obtain the decrypted bytes. */
	/* Decryption table provided by David Caldwell (david@indigita.com) */
	/* For an accurate reproduction of the encryption, see jrcrypt.c */
	static const struct {
		int count;
		int value;
	} table[] =
	{
		{ 0x00C1, 0x00 },{ 0x0002, 0x80 },{ 0x0004, 0x00 },{ 0x0006, 0x80 },
		{ 0x0003, 0x00 },{ 0x0002, 0x80 },{ 0x0009, 0x00 },{ 0x0004, 0x80 },
		{ 0x9968, 0x00 },{ 0x0001, 0x80 },{ 0x0002, 0x00 },{ 0x0001, 0x80 },
		{ 0x0009, 0x00 },{ 0x0002, 0x80 },{ 0x0009, 0x00 },{ 0x0001, 0x80 },
		{ 0x00AF, 0x00 },{ 0x000E, 0x04 },{ 0x0002, 0x00 },{ 0x0004, 0x04 },
		{ 0x001E, 0x00 },{ 0x0001, 0x80 },{ 0x0002, 0x00 },{ 0x0001, 0x80 },
		{ 0x0002, 0x00 },{ 0x0002, 0x80 },{ 0x0009, 0x00 },{ 0x0002, 0x80 },
		{ 0x0009, 0x00 },{ 0x0002, 0x80 },{ 0x0083, 0x00 },{ 0x0001, 0x04 },
		{ 0x0001, 0x01 },{ 0x0001, 0x00 },{ 0x0002, 0x05 },{ 0x0001, 0x00 },
		{ 0x0003, 0x04 },{ 0x0003, 0x01 },{ 0x0002, 0x00 },{ 0x0001, 0x04 },
		{ 0x0003, 0x01 },{ 0x0003, 0x00 },{ 0x0003, 0x04 },{ 0x0001, 0x01 },
		{ 0x002E, 0x00 },{ 0x0078, 0x01 },{ 0x0001, 0x04 },{ 0x0001, 0x05 },
		{ 0x0001, 0x00 },{ 0x0001, 0x01 },{ 0x0001, 0x04 },{ 0x0002, 0x00 },
		{ 0x0001, 0x01 },{ 0x0001, 0x04 },{ 0x0002, 0x00 },{ 0x0001, 0x01 },
		{ 0x0001, 0x04 },{ 0x0002, 0x00 },{ 0x0001, 0x01 },{ 0x0001, 0x04 },
		{ 0x0001, 0x05 },{ 0x0001, 0x00 },{ 0x0001, 0x01 },{ 0x0001, 0x04 },
		{ 0x0002, 0x00 },{ 0x0001, 0x01 },{ 0x0001, 0x04 },{ 0x0002, 0x00 },
		{ 0x0001, 0x01 },{ 0x0001, 0x04 },{ 0x0001, 0x05 },{ 0x0001, 0x00 },
		{ 0x01B0, 0x01 },{ 0x0001, 0x00 },{ 0x0002, 0x01 },{ 0x00AD, 0x00 },
		{ 0x0031, 0x01 },{ 0x005C, 0x00 },{ 0x0005, 0x01 },{ 0x604E, 0x00 },
		{ 0,0 }
	};

	uint8_t *RAM = memregion("maincpu")->base();
	for (int i = 0, A = 0; table[i].count; i++)
		for (int j = 0; j < table[i].count; j++)
			RAM[A++] ^= table[i].value;
}

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, jrpacman,  0,        jrpacman, jrpacman, jrpacman_state, init_jrpacman, ROT90, "Bally Midway", "Jr. Pac-Man (11/9/83)",      MACHINE_SUPPORTS_SAVE )
GAME( 1983, jrpacmanf, jrpacman, jrpacman, jrpacman, jrpacman_state, init_jrpacman, ROT90, "hack",         "Jr. Pac-Man (speedup hack)", MACHINE_SUPPORTS_SAVE )

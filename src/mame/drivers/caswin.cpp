// license:BSD-3-Clause
// copyright-holders:Chris Hardy, Angelo Salese
/*******************************************************************************************

Royal Casino (c) 1984 Dyna
Casino Winner (c) 1985 Aristocrat

driver by Chris Hardy & Angelo Salese
original rcasino.c driver by Curt Coder

TODO:
-Cherry-type subgames appears to have wrong graphics alignment,maybe it's some fancy window
 effect?
-Add lamps support;
-p1 & p2 inputs are tied to the same port...maybe they are mux-ed with the flip screen bit?

============================================================================================
    ----------------------------------------
    Casino Royal by Dyna Electronics CO. LTD
    ----------------------------------------

    Location    Device      File ID     Checksum
    --------------------------------------------
    18B          2764        RI-W1        C62D
    16B          2764        RI-W2        AC85
    15B          2732        RI-W3        70B7
    11B          2732        RI-W4        0C21
    9B           2764        RI-W5        EB59
    8B           2764        RI-W6        C934
    6B           2732        RI-W7        4130
    9E         82S123      PROM1.BPR      0F29
    8E         82S123      PROM2.BPR      0EE5

    Notes: PCB No. D-2608208A1-2

    Brief hardware overview
    -----------------------

    Main processor  - Z80
    Sound           - AY-3-8910

    ----------------------------------------
    Casino Royal by Dyna Electronics CO. LTD
    ----------------------------------------

    PCB No. D-2608208A1-1 (Has angled corner cut on the pcb next to this identifier.)

    Brief hardware overview
    -----------------------

    Xtal            - 6.000 MHz @ 13L
    Main processor  - Z80A
    Sound           - AY-3-8910

    ----------------------------------------
    Casino Royal by Dyna Electronics CO. LTD
    ----------------------------------------

    PCB No. D-2608208A1-1 (Labeled with Nagoya Japan)

    Brief hardware overview
    -----------------------

    Xtal            - 6.000 MHz @ 2G
    Main processor  - Z80A
    Sound           - AY-3-8910

*******************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"


class caswin_state : public driver_device
{
public:
	caswin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_sc0_vram(*this, "sc0_vram"),
		m_sc0_attr(*this, "sc0_attr"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	required_shared_ptr<UINT8> m_sc0_vram;
	required_shared_ptr<UINT8> m_sc0_attr;
	tilemap_t *m_sc0_tilemap;
	DECLARE_WRITE8_MEMBER(sc0_vram_w);
	DECLARE_WRITE8_MEMBER(sc0_attr_w);
	DECLARE_WRITE8_MEMBER(vvillage_scroll_w);
	DECLARE_WRITE8_MEMBER(vvillage_vregs_w);
	DECLARE_READ8_MEMBER(vvillage_rng_r);
	DECLARE_WRITE8_MEMBER(vvillage_output_w);
	DECLARE_WRITE8_MEMBER(vvillage_lamps_w);
	TILE_GET_INFO_MEMBER(get_sc0_tile_info);
	virtual void video_start();
	DECLARE_PALETTE_INIT(caswin);
	UINT32 screen_update_vvillage(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};



TILE_GET_INFO_MEMBER(caswin_state::get_sc0_tile_info)
{
	int tile = (m_sc0_vram[tile_index] | ((m_sc0_attr[tile_index] & 0x70)<<4)) & 0x7ff;
	int colour = m_sc0_attr[tile_index] & 0xf;

	SET_TILE_INFO_MEMBER(0,
			tile,
			colour,
			0);
}

void caswin_state::video_start()
{
	m_sc0_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(caswin_state::get_sc0_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32,32);
}

UINT32 caswin_state::screen_update_vvillage(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_sc0_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

WRITE8_MEMBER(caswin_state::sc0_vram_w)
{
	m_sc0_vram[offset] = data;
	m_sc0_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(caswin_state::sc0_attr_w)
{
	m_sc0_attr[offset] = data;
	m_sc0_tilemap->mark_tile_dirty(offset);
}

/*These two are tested during the two cherry sub-games.I really don't know what is supposed to do...*/
WRITE8_MEMBER(caswin_state::vvillage_scroll_w)
{
	//...
}

/*---- --x- window effect? */
/*---- ---x flip screen */
WRITE8_MEMBER(caswin_state::vvillage_vregs_w)
{
	flip_screen_set(data & 1);
}

/**********************
*
* End of Video Hardware
*
**********************/

READ8_MEMBER(caswin_state::vvillage_rng_r)
{
	return machine().rand();
}

WRITE8_MEMBER(caswin_state::vvillage_output_w)
{
	coin_counter_w(machine(), 0,data & 1);
	coin_counter_w(machine(), 1,data & 1);
	// data & 4 payout counter
	coin_lockout_w(machine(), 0,data & 0x20);
	coin_lockout_w(machine(), 1,data & 0x20);
}

WRITE8_MEMBER(caswin_state::vvillage_lamps_w)
{
	/*
	---x ---- lamp button 5
	---- x--- lamp button 4
	---- -x-- lamp button 3
	---- --x- lamp button 2
	---- ---x lamp button 1
	*/
	set_led_status(machine(), 0, data & 0x01);
	set_led_status(machine(), 1, data & 0x02);
	set_led_status(machine(), 2, data & 0x04);
	set_led_status(machine(), 3, data & 0x08);
	set_led_status(machine(), 4, data & 0x10);
}

static ADDRESS_MAP_START( vvillage_mem, AS_PROGRAM, 8, caswin_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_READ(vvillage_rng_r) //accessed by caswin only
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xf000, 0xf3ff) AM_RAM_WRITE(sc0_vram_w) AM_SHARE("sc0_vram")
	AM_RANGE(0xf800, 0xfbff) AM_RAM_WRITE(sc0_attr_w) AM_SHARE("sc0_attr")
ADDRESS_MAP_END

static ADDRESS_MAP_START( vvillage_io, AS_IO, 8, caswin_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01,0x01) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0x02,0x03) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE(0x10,0x10) AM_READ_PORT("IN0")
	AM_RANGE(0x11,0x11) AM_READ_PORT("IN1")
	AM_RANGE(0x10,0x10) AM_WRITE(vvillage_scroll_w)
	AM_RANGE(0x11,0x11) AM_WRITE(vvillage_vregs_w)
	AM_RANGE(0x12,0x12) AM_WRITE(vvillage_lamps_w)
	AM_RANGE(0x13,0x13) AM_WRITE(vvillage_output_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( vvillage )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Note Acceptor")// Note
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Hopper Payout")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )                        // Hopper Micro
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )                          // 1P FlipFlop
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )                          // 2P FlipFlop
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Poker Available" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Black Jack Available" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "Hi & Low Available" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Five Line Available" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "Super Conti Available" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "BlackJack, Even Rule" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Poker, Royal Flush Rule" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Poker, Jack or Better Rule") PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Enable Hopper Payout" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Enable Hopper Win Payout") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Hi Lo, Royal Flush" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Game Bet") PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "Normal Game")
	PORT_DIPSETTING(    0x00, "Double Game")
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPNAME( 0x80, 0x80, "Analyzer" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ 0,RGN_FRAC(1,2) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( vvillage )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

PALETTE_INIT_MEMBER(caswin_state, caswin)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int bit0, bit1, bit2 , r, g, b;
	int i;

	for (i = 0; i < 0x40; ++i)
	{
		bit0 = 0;
		bit1 = (color_prom[0] >> 0) & 0x01;
		bit2 = (color_prom[0] >> 1) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[0] >> 2) & 0x01;
		bit1 = (color_prom[0] >> 3) & 0x01;
		bit2 = (color_prom[0] >> 4) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[0] >> 5) & 0x01;
		bit1 = (color_prom[0] >> 6) & 0x01;
		bit2 = (color_prom[0] >> 7) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}


static MACHINE_CONFIG_START( vvillage, caswin_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,4000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(vvillage_mem)
	MCFG_CPU_IO_MAP(vvillage_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", caswin_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(caswin_state, screen_update_vvillage)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", vvillage)
	MCFG_PALETTE_ADD("palette", 0x40)
	MCFG_PALETTE_INIT_OWNER(caswin_state, caswin)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 4000000 / 4)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END

ROM_START( caswin )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "cw_v5_0_1.26", 0x0000, 0x4000, CRC(ae3d2cf0) SHA1(268572730389f12cf962782008690305fad1ac1b) )
	ROM_LOAD( "cw_v5_0_2.24", 0x4000, 0x4000, CRC(2855b3b8) SHA1(f5cc0bbeee6c1fb0dc6aebc2e3af09dccdb248ad) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "cw_4.19", 0x00000, 0x4000, CRC(d2deab75) SHA1(12cf3fd02dbad9a40cfa6cece0cb66ce2c4dc315) )
	ROM_LOAD( "cw_3.22", 0x04000, 0x4000, CRC(7e79966c) SHA1(39190ee8cd7f3b8f895b32327f3a5555a0713315) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "clr1.bin", 0x00, 0x20, CRC(52e31046) SHA1(71a95a72b591ae7b75af4adff526fca9ae055c5b) )
	ROM_LOAD( "clr2.bin", 0x20, 0x20, CRC(2b5c7826) SHA1(c0de392aebd6982e5846c12aeb2e871358be60d7) )
ROM_END

ROM_START( rcasino )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ri-w1.18b", 0x0000, 0x2000, CRC(ed105d69) SHA1(951697e1050f72967f0710155aa8ff72db73fce1) )
	ROM_LOAD( "ri-w2.16b", 0x2000, 0x2000, CRC(a1a80b33) SHA1(2f969713cae288de1985d7baa70cad50c4148970) )
	ROM_LOAD( "ri-w3.15b", 0x4000, 0x1000, CRC(acf77a36) SHA1(599470e461a261130e942d174051648459f37a37) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "ri-w6.8b",  0x0000, 0x2000, CRC(b2dd4e1e) SHA1(323dcfb26653c17951db65ce2ced3325d35489e4) )
	ROM_LOAD( "ri-w7.6b",  0x2000, 0x1000, CRC(8e0d3b9c) SHA1(c5211d834b0db488839a5c53d00435a0b59cd4ca) )
	ROM_LOAD( "ri-w5.9b",  0x3000, 0x2000, CRC(81d20577) SHA1(50a1e0231400c106539ffa78deb3e0e6c8afc3f5) )
	ROM_LOAD( "ri-w4.11b", 0x5000, 0x1000, CRC(7ca0e78c) SHA1(163cfd1f76ecbd14219146963d1abc4c09c0ac8c) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "prom1.9e",  0x0000, 0x0020, CRC(93312432) SHA1(3c7abc165e6bc7e0c56ca97d89b0b5e06323b82e) )
	ROM_LOAD( "prom2.8e",  0x0020, 0x0020, CRC(2b5c7826) SHA1(c0de392aebd6982e5846c12aeb2e871358be60d7) )
ROM_END

ROM_START( rcasino1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mrdu1.f1",  0x0000, 0x2000, CRC(ed105d69) SHA1(951697e1050f72967f0710155aa8ff72db73fce1) )
	ROM_LOAD( "mrdu2.f2",  0x2000, 0x2000, CRC(a1a80b33) SHA1(2f969713cae288de1985d7baa70cad50c4148970) )
	ROM_LOAD( "mrdu3.f4",  0x4000, 0x1000, CRC(acf77a36) SHA1(599470e461a261130e942d174051648459f37a37) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "6.e11",     0x0000, 0x2000, CRC(b2dd4e1e) SHA1(323dcfb26653c17951db65ce2ced3325d35489e4) )
	ROM_LOAD( "v7.e13",    0x2000, 0x1000, CRC(c7ff4ce3) SHA1(51d4bafddcaef355571bd32b16753b1cee54368d) )
	ROM_LOAD( "h.e9",      0x3000, 0x2000, CRC(645c7cbf) SHA1(5790422b86a59531764233dd3c9488fdbad476bc) )
	ROM_LOAD( "mrdu4.e10", 0x5000, 0x1000, CRC(7ca0e78c) SHA1(163cfd1f76ecbd14219146963d1abc4c09c0ac8c) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "v1.a11",    0x0000, 0x0020, CRC(93312432) SHA1(3c7abc165e6bc7e0c56ca97d89b0b5e06323b82e) ) /* TBP18S030 */
	ROM_LOAD( "v2.a12",    0x0020, 0x0020, CRC(2b5c7826) SHA1(c0de392aebd6982e5846c12aeb2e871358be60d7) ) /* TBP18S030 */
ROM_END

ROM_START( rcasinoo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mrdu1.b18", 0x0000, 0x2000, CRC(ed105d69) SHA1(951697e1050f72967f0710155aa8ff72db73fce1) )
	ROM_LOAD( "mrdu2.b16", 0x2000, 0x2000, CRC(a1a80b33) SHA1(2f969713cae288de1985d7baa70cad50c4148970) )
	ROM_LOAD( "mrdu3.b15", 0x4000, 0x1000, CRC(acf77a36) SHA1(599470e461a261130e942d174051648459f37a37) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "5.b8",      0x0000, 0x2000, CRC(b2dd4e1e) SHA1(323dcfb26653c17951db65ce2ced3325d35489e4) )
	ROM_LOAD( "6.b6",      0x2000, 0x1000, CRC(c7ff4ce3) SHA1(51d4bafddcaef355571bd32b16753b1cee54368d) )
	ROM_LOAD( "3.b9",      0x3000, 0x2000, CRC(81d20577) SHA1(50a1e0231400c106539ffa78deb3e0e6c8afc3f5) )
	ROM_LOAD( "mrdu4.b11", 0x5000, 0x1000, CRC(7ca0e78c) SHA1(163cfd1f76ecbd14219146963d1abc4c09c0ac8c) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "prom2.e9",  0x0000, 0x0020, CRC(93312432) SHA1(3c7abc165e6bc7e0c56ca97d89b0b5e06323b82e) ) /* MB7051 */
	ROM_LOAD( "prom1.e8",  0x0020, 0x0020, CRC(2b5c7826) SHA1(c0de392aebd6982e5846c12aeb2e871358be60d7) ) /* MB7051 */
ROM_END

GAME( 1984, rcasino,  0,       vvillage, vvillage, driver_device, 0, ROT270, "Dyna Electronics", "Royal Casino (D-2608208A1-2)",                MACHINE_IMPERFECT_GRAPHICS )
GAME( 1984, rcasino1, rcasino, vvillage, vvillage, driver_device, 0, ROT270, "Dyna Electronics", "Royal Casino (D-2608208A1-1, Larger Board)",  MACHINE_IMPERFECT_GRAPHICS )
GAME( 1984, rcasinoo, rcasino, vvillage, vvillage, driver_device, 0, ROT270, "Dyna Electronics", "Royal Casino (D-2608208A1-1, Smaller Board)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1985, caswin,   rcasino, vvillage, vvillage, driver_device, 0, ROT270, "Aristocrat",       "Casino Winner",                               MACHINE_IMPERFECT_GRAPHICS )

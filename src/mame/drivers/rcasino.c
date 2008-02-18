/*
"Royal Casino"

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

    ---

    Driver by Curt Coder

TODO:

Get correct data for hopper on and lockout.
*/

#include "driver.h"
#include "sound/ay8910.h"

static tilemap *bg_tilemap;
static int pulse;
static int hopper;
static PALETTE_INIT( rcasino )
{
	int i;

	for (i = 0; i < machine->config->total_colors; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		// red component

		bit0 = (*color_prom >> 7) & 0x01;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;

		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component

		bit0 = (*color_prom >> 4) & 0x01;
		bit1 = (*color_prom >> 3) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;

		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component

		bit0 = 0;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 0) & 0x01;

		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));

		color_prom++;
	}
}

static WRITE8_HANDLER( rcasino_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static WRITE8_HANDLER( rcasino_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = colorram[tile_index];
	int bank = (attr & 0x40) >> 6;
	int code = videoram[tile_index] + ((attr & 0x30) << 4);
	int color = attr & 0x0f;

	SET_TILE_INFO(bank, code, color, 0);
}

static VIDEO_START(rcasino)
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		8, 8, 32, 32);
}

static VIDEO_UPDATE(rcasino)
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}

static WRITE8_HANDLER( rcasino_port_10_w )
{
//  popmessage("port 10 data %u", data);
	// write several values when the wheels are spinning
}

static WRITE8_HANDLER( rcasino_port_11_w )
{
	/*
        write 0 when "P1 FlipFlop" (start1) is pressed
        write 1 when "P2 FlipFlop" (start2) is pressed
        write 2 when in "Super Conti" game

        In royalmah.c games this is an input port selector, but
        there are only 2 input ports in this game and this write
        seems to have no effect
    */
}

static WRITE8_HANDLER( rcasino_lamp_w )
{
	// button lamps
	set_led_status(0, data & 0x01);
	set_led_status(1, data & 0x02);
	set_led_status(2, data & 0x04);
	set_led_status(3, data & 0x08);
	set_led_status(4, data & 0x10);
}

static WRITE8_HANDLER( rcasino_coin_counter_w )
{
	// coin counter
	coin_counter_w(0, data & 0x01);

	// payout counter
	coin_counter_w(1, data & 0x04);

	// activate hopper
	//hopper = data & 0x08;

	// write 0x80 if payout fails and "call dealer" appears on screen, is it a coin lockout?
	//coin_lockout_w(0, data & 0x80);
//Probably, most of these systems have some protection to stop clueless people from clogging up a broken hopper.
}

static READ8_HANDLER( rcasino_port_11_r )
{
   if (hopper)
   {
       if (pulse)
       pulse = 0;
       else pulse = 0x04;
   }
   return readinputport(1) + pulse;
}

static ADDRESS_MAP_START( rcasino_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x4fff) AM_ROM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM
	AM_RANGE(0xf000, 0xf3ff) AM_RAM AM_WRITE(rcasino_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0xf800, 0xfbff) AM_RAM AM_WRITE(rcasino_colorram_w) AM_BASE(&colorram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( rcasino_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x01, 0x01) AM_READ(AY8910_read_port_0_r)
	AM_RANGE(0x02, 0x02) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x10, 0x10) AM_READWRITE(input_port_0_r, rcasino_port_10_w)
//  AM_RANGE(0x11, 0x11) AM_READWRITE(input_port_1_r, rcasino_port_11_w)
	AM_RANGE(0x11, 0x11) AM_READWRITE(rcasino_port_11_r, rcasino_port_11_w)
	AM_RANGE(0x12, 0x12) AM_WRITE(rcasino_lamp_w)
	AM_RANGE(0x13, 0x13) AM_WRITE(rcasino_coin_counter_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( rcasino )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_A)	// SW1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_S)	// SW2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_D)	// SW3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_F)	// SW4
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_G)	// SW5
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )                           // Coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Note Acceptor")// Note
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("Payout")// Pay Out
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )                        // Hopper Micro
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )                          // 1P FlipFlop
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )                          // 2P FlipFlop
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Poker available?" )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Black Jack available?")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Hi and Low available?" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Five Line (Slot) available?")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Super Continental available?")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Rule Change (BlackJack, even)" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Rule Change (Poker, Royal Flush)" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Rule Change (Poker, Jack or Better)")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Enable Hopper Payout" )	// enables Payout button
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Enable Hopper Win Payout")	// enables Payout button
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Hi Lo, Royal Flush" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Game Bet")
	PORT_DIPSETTING(    0x08, "Normal Game")
	PORT_DIPSETTING(    0x00, "Double Game")
	PORT_DIPNAME( 0x10, 0x10, "Always Off" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Always Off")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, "Analyzer" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,2),
	2,
	{ 0, 0x2000*8 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout charlayout2 =
{
	8, 8,
	RGN_FRAC(1,2),
	2,
	{ 0, 0x1000*8 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( rcasino )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, charlayout,  0, 16 )
	GFXDECODE_ENTRY( REGION_GFX2, 0x0000, charlayout2, 0, 16 )
GFXDECODE_END

static const struct AY8910interface ay8910_interface =
{
	input_port_2_r,	// DSW1
	input_port_3_r,	// DSW2
	0,
	0
};

static MACHINE_DRIVER_START( rcasino )
	// basic machine hardware
	MDRV_CPU_ADD(Z80, 8000000/2)	// ???
	MDRV_CPU_PROGRAM_MAP(rcasino_map, 0)
	MDRV_CPU_IO_MAP(rcasino_io_map, 0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold, 1)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	// video hardware
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(rcasino)
	MDRV_PALETTE_LENGTH(64)

	MDRV_PALETTE_INIT(rcasino)
	MDRV_VIDEO_START(rcasino)
	MDRV_VIDEO_UPDATE(rcasino)

	// sound hardware
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 18432000/12)
	MDRV_SOUND_CONFIG(ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

MACHINE_DRIVER_END

ROM_START( rcasino )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "ri-w1.18b", 0x0000, 0x2000, CRC(ed105d69) SHA1(951697e1050f72967f0710155aa8ff72db73fce1) )
	ROM_LOAD( "ri-w2.16b", 0x2000, 0x2000, CRC(a1a80b33) SHA1(2f969713cae288de1985d7baa70cad50c4148970) )
	ROM_LOAD( "ri-w3.15b", 0x4000, 0x1000, CRC(acf77a36) SHA1(599470e461a261130e942d174051648459f37a37) )

	ROM_REGION( 0x4000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ri-w5.9b",  0x0000, 0x2000, CRC(81d20577) SHA1(50a1e0231400c106539ffa78deb3e0e6c8afc3f5) )
	ROM_LOAD( "ri-w6.8b",  0x2000, 0x2000, CRC(b2dd4e1e) SHA1(323dcfb26653c17951db65ce2ced3325d35489e4) )

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "ri-w4.11b", 0x0000, 0x1000, CRC(7ca0e78c) SHA1(163cfd1f76ecbd14219146963d1abc4c09c0ac8c) )
	ROM_LOAD( "ri-w7.6b",  0x1000, 0x1000, CRC(8e0d3b9c) SHA1(c5211d834b0db488839a5c53d00435a0b59cd4ca) )

	ROM_REGION( 0x40, REGION_PROMS, 0 )
	ROM_LOAD( "prom1.9e",  0x0000, 0x0020, CRC(93312432) SHA1(3c7abc165e6bc7e0c56ca97d89b0b5e06323b82e) )
	ROM_LOAD( "prom2.8e",  0x0020, 0x0020, CRC(2b5c7826) SHA1(c0de392aebd6982e5846c12aeb2e871358be60d7) )
ROM_END

GAME( 1984, rcasino, 0, rcasino, rcasino, 0, ROT270, "Dyna Electronics", "Royal Casino", GAME_IMPERFECT_COLORS )

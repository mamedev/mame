/*

        Lucky 8 Line
        Falcon 1989

        G14           6116  9
        G13   D13           8
              D12
        6116                 Z80
        6116                 8255
        7                    8255
        6            SW1     8255
 12MHz  5            SW2     8910
        4  6116      SW4
        3  6116      SW3
        2  6116
        1  6116

    ---

    Driver by Curt Coder

*/

#include "driver.h"
#include "sound/ay8910.h"

/* video */

static tilemap *bg_tilemap;

static PALETTE_INIT( lucky8 )
{
	int i;

	for (i = 0; i < machine->config->total_colors; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		UINT8 pen = color_prom[0x20 + i] & 0x0f;

		/* red component */
		bit0 = (color_prom[pen] >> 0) & 0x01;
		bit1 = (color_prom[pen] >> 1) & 0x01;
		bit2 = (color_prom[pen] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[pen] >> 3) & 0x01;
		bit1 = (color_prom[pen] >> 4) & 0x01;
		bit2 = (color_prom[pen] >> 5) & 0x01;

		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[pen] >> 6) & 0x01;
		bit2 = (color_prom[pen] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

static WRITE8_HANDLER( lucky8_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static WRITE8_HANDLER( lucky8_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index] + ((attr & 0xf0) << 4);
	int color = attr & 0x0f;

	SET_TILE_INFO(0, code, color, 0);
}

static VIDEO_START(lucky8)
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		8, 8, 64, 32);
}

static VIDEO_UPDATE(lucky8)
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}

static ADDRESS_MAP_START( lucky8_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8fff) AM_RAM_WRITE(lucky8_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x9000, 0x97ff) AM_RAM	AM_WRITE(lucky8_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x9800, 0x99ff) AM_RAM
	AM_RANGE(0xa000, 0xa1ff) AM_RAM
	AM_RANGE(0xa800, 0xa9ff) AM_RAM
	AM_RANGE(0xb000, 0xb1ff) AM_RAM
	AM_RANGE(0xb800, 0xb800) AM_READ(input_port_0_r)
	AM_RANGE(0xb801, 0xb801) AM_READ(input_port_1_r)
	AM_RANGE(0xb802, 0xb802) AM_READ(input_port_2_r)
	AM_RANGE(0xb803, 0xb803) AM_WRITENOP
	AM_RANGE(0xb810, 0xb810) AM_READ(input_port_3_r)
	AM_RANGE(0xb811, 0xb811) AM_READ(input_port_4_r)
	AM_RANGE(0xb812, 0xb812) AM_READ(input_port_5_r)
	AM_RANGE(0xb813, 0xb813) AM_WRITENOP
	AM_RANGE(0xb820, 0xb820) AM_READ(input_port_6_r)
	AM_RANGE(0xb821, 0xb821) AM_WRITENOP
	AM_RANGE(0xb822, 0xb822) AM_WRITENOP
	AM_RANGE(0xb823, 0xb823) AM_WRITENOP
	AM_RANGE(0xb830, 0xb830) AM_READWRITE(AY8910_read_port_0_r, AY8910_write_port_0_w)
	AM_RANGE(0xb840, 0xb840) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0xb850, 0xb850) AM_WRITENOP
	AM_RANGE(0xb860, 0xb860) AM_WRITENOP
	AM_RANGE(0xb870, 0xb870) AM_WRITENOP
ADDRESS_MAP_END

static INPUT_PORTS_START( lucky8 )
	PORT_START_TAG("B800")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 0") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 1") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 2") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 3") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 4") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 5") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 6") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 7") PORT_CODE(KEYCODE_I)

	PORT_START_TAG("B801")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 0") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 1") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 2") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 3") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 4") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 5") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 6") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 7") PORT_CODE(KEYCODE_K)

	PORT_START_TAG("B802")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 0") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 1") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 2") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 3") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 4") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 5") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 6") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 7") PORT_CODE(KEYCODE_L)

	PORT_START_TAG("B810")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_START_TAG("B811")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE3 )

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Main Game" )
	PORT_DIPSETTING(    0x07, "80%" )
	PORT_DIPSETTING(    0x06, "83%" )
	PORT_DIPSETTING(    0x05, "86%" )
	PORT_DIPSETTING(    0x04, "89%" )
	PORT_DIPSETTING(    0x03, "92%" )
	PORT_DIPSETTING(    0x02, "95%" )
	PORT_DIPSETTING(    0x01, "98%" )
	PORT_DIPSETTING(    0x00, "101%" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW4")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8, 8,
	4096,
	3,
	{ 0, 0x8000*8, 0x10000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const UINT32 spritelayout_xoffset[96] =
{
	  0+0*32*8, 1+0*32*8, 2+0*32*8, 3+0*32*8, 4+0*32*8, 5+0*32*8, 6+0*32*8, 7+0*32*8,
	  0+1*32*8, 1+1*32*8, 2+1*32*8, 3+1*32*8, 4+1*32*8, 5+1*32*8, 6+1*32*8, 7+1*32*8,
	  0+2*32*8, 1+2*32*8, 2+2*32*8, 3+2*32*8, 4+2*32*8, 5+2*32*8, 6+2*32*8, 7+2*32*8,
	  0+3*32*8, 1+3*32*8, 2+3*32*8, 3+3*32*8, 4+3*32*8, 5+3*32*8, 6+3*32*8, 7+3*32*8,
	  0+4*32*8, 1+4*32*8, 2+4*32*8, 3+4*32*8, 4+4*32*8, 5+4*32*8, 6+4*32*8, 7+4*32*8,
	  0+5*32*8, 1+5*32*8, 2+5*32*8, 3+5*32*8, 4+5*32*8, 5+5*32*8, 6+5*32*8, 7+5*32*8,
	  0+6*32*8, 1+6*32*8, 2+6*32*8, 3+6*32*8, 4+6*32*8, 5+6*32*8, 6+6*32*8, 7+6*32*8,
	  0+7*32*8, 1+7*32*8, 2+7*32*8, 3+7*32*8, 4+7*32*8, 5+7*32*8, 6+7*32*8, 7+7*32*8,
	  0+8*32*8, 1+8*32*8, 2+8*32*8, 3+8*32*8, 4+8*32*8, 5+8*32*8, 6+8*32*8, 7+8*32*8,
	  0+9*32*8, 1+9*32*8, 2+9*32*8, 3+9*32*8, 4+9*32*8, 5+9*32*8, 6+9*32*8, 7+9*32*8,
	  0+10*32*8, 1+10*32*8, 2+10*32*8, 3+10*32*8, 4+10*32*8, 5+10*32*8, 6+10*32*8, 7+10*32*8,
	  0+11*32*8, 1+11*32*8, 2+11*32*8, 3+11*32*8, 4+11*32*8, 5+11*32*8, 6+11*32*8, 7+11*32*8
};

static const gfx_layout spritelayout =
{
	96, 32,
	10,
	4,
	{ 0, 0x2000*8, 0x4000*8, 0x6000*8 },
	EXTENDED_XOFFS,
	{
		   0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,  8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
		  16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8, 24*8, 25*8, 26*8, 27*8, 28*8, 29*8, 30*8, 31*8
	},
	12*32*8,
	spritelayout_xoffset,
	NULL
};

static const gfx_layout spritelayout2 =
{
	64, 32,
	5,
	4,
	{ 0, 0x2000*8, 0x4000*8, 0x6000*8 },
	EXTENDED_XOFFS,
	{
		   0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,  8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
		  16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8, 24*8, 25*8, 26*8, 27*8, 28*8, 29*8, 30*8, 31*8
	},
	8*32*8,
	spritelayout_xoffset,
	NULL
};

static GFXDECODE_START( lucky8 )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, charlayout,	   0, 64 )
	GFXDECODE_ENTRY( REGION_GFX2, 0x0000, spritelayout,  0, 64 )
	GFXDECODE_ENTRY( REGION_GFX2, 0x1000, spritelayout2, 0, 64 )
GFXDECODE_END

static const struct AY8910interface ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	input_port_7_r,	// DSW3
	input_port_8_r,	// DSW4
	NULL,
	NULL
};

static MACHINE_DRIVER_START( lucky8 )
	// basic machine hardware
	MDRV_CPU_ADD_TAG("main", Z80, 12000000/4)	// ??? Runs far too fast
	MDRV_CPU_PROGRAM_MAP(lucky8_map, 0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	// video hardware

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(lucky8)
	MDRV_PALETTE_LENGTH(256)

	MDRV_PALETTE_INIT(lucky8)
	MDRV_VIDEO_START(lucky8)
	MDRV_VIDEO_UPDATE(lucky8)

	// sound hardware
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay", AY8910, 12000000/12)
	MDRV_SOUND_CONFIG(ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END

ROM_START( lucky8 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "8",	 0x0000, 0x4000, CRC(a187573e) SHA1(864627502025dbc83a0049fc98505655cec7b181) )
	ROM_LOAD( "9",   0x4000, 0x4000, CRC(6f62672e) SHA1(05662ef1a70f93b09e48de497b049a282f070735) )

	ROM_REGION( 0x18000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "5",  0x00000, 0x8000, CRC(59026af3) SHA1(3d7f7e78968ca26275635aeaa0e994468a3da575) )
	ROM_LOAD( "6",  0x08000, 0x8000, CRC(67a073c1) SHA1(36194d57d0dc0601fa1fdf2e6806f11b2ea6da36) )
	ROM_LOAD( "7",  0x10000, 0x8000, CRC(c415b9d0) SHA1(fd558fe8a116c33bbd712a639224d041447a45c1) )

	ROM_REGION( 0x8000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "1",   0x0000, 0x2000, CRC(29d6f197) SHA1(1542ca457594f6b7fe8f28f7d78023edd7021bc8) )
	ROM_LOAD( "2",   0x2000, 0x2000, CRC(5f812e65) SHA1(70d9ea82f9337936bf21f82b6961768d436f3a6f) )
	ROM_LOAD( "3",   0x4000, 0x2000, CRC(898b9ed5) SHA1(11b7d1cfcf425d00d086c74e0dbcb72068dda9fe) )
	ROM_LOAD( "4",   0x6000, 0x2000, CRC(4f7cfb35) SHA1(0617cf4419be00d9bacc78724089cb8af4104d68) )

	ROM_REGION( 0x240, REGION_PROMS, 0 )
	ROM_LOAD( "d13", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )
	ROM_LOAD( "d12", 0x0020, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	ROM_LOAD( "g13", 0x0120, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
	ROM_LOAD( "g14", 0x0140, 0x0100, CRC(bd48de71) SHA1(e4fa1e774af1499bc568be5b2deabb859d8c8172) )
ROM_END

static DRIVER_INIT( lucky8 )
{
	// patch rst $10 timing routine that gets stuck in an endless loop
	UINT8 *ROM = memory_region(machine, REGION_CPU1);
	ROM[0x0010] = 0xc9;
}

GAME( 1989, lucky8, 0, lucky8, lucky8, lucky8, ROT0, "Falcon", "Lucky 8 Lines", GAME_NOT_WORKING )

/*******************************************************************************************

86 Lions (c) 1985 Aristocrat

driver by Chris Hardy,Angelo Salese & Roberto Fresca


NOTES:

  - 2 graphics ROMS by each R, G, B.
  - 6 bit buffer, goes to 2 resistors each to 3 transistors... 32 colors.
  - CRTC is using 4 pixels by memory address.
  - Xtal and crtc CLK are accurate.
  - Seems to be 1 DSW bank tied to VIA, and another to ay8910.
  - Maybe this is "Aristocrat Mark-III" HW?

TODO:
  - Understand inputs / via mapping properly;
  - Finish the mc6845 conversion;

Changes 02/06/2009 - Palindrome
- Fixed VIA address map to 5000 - 0x501f ( now generates required FIRQ_LINE timer interrupt,
  call attendant msg no longer displayed)


*******************************************************************************************/

#define MAIN_CLOCK	XTAL_12MHz	/* guess */

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/mc6845.h"
#include "sound/ay8910.h"
#include "machine/6522via.h"

static UINT8 *lions_vram;

static VIDEO_START(lions)
{
}

static VIDEO_UPDATE(lions)
{
	const gfx_element *gfx = screen->machine->gfx[0];
	int count = 0;

	int y,x;

	for (y=0;y<27;y++)
	{
		for (x=0;x<38;x++)
		{
			int tile = lions_vram[count+1]|lions_vram[count]<<8;
			tile&=0x1ff;
			//int colour = tile>>12;
			drawgfx_opaque(bitmap,cliprect,gfx,tile,0,0,0,x*8,y*8);

			count+=2;
		}
	}
	return 0;
}


/**********************
*
* End of Video Hardware
*
**********************/

static READ8_HANDLER( test_r )
{
	return mame_rand(space->machine);
}


static READ8_HANDLER(lions_via_r)
{
	running_device *via_0 = devtag_get_device(space->machine, "via6522_0");
	return via_r(via_0, offset);
}

static WRITE8_HANDLER(lions_via_w)
{
	running_device *via_0 = devtag_get_device(space->machine, "via6522_0");
	via_w(via_0, offset, data);
}


static ADDRESS_MAP_START( lions_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE(&lions_vram)
	AM_RANGE(0x0800, 0x0fff) AM_RAM
	AM_RANGE(0x1800, 0x1800) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x1801, 0x1801) AM_DEVREADWRITE("crtc", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0x5000, 0x500f) AM_MIRROR(0x0010) AM_READWRITE(lions_via_r, lions_via_w)
	AM_RANGE(0x5300, 0x5300) AM_READ(test_r)//AM_READ_PORT("IN0")
	AM_RANGE(0x5382, 0x5383) AM_DEVWRITE("aysnd", ay8910_data_address_w)
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( lions )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-2") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-3") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-4") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-5") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-6") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-7") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-8") PORT_CODE(KEYCODE_8)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-1") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-2") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-3") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-4") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-5") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-6") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-7") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-8") PORT_CODE(KEYCODE_I)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-7") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-8") PORT_CODE(KEYCODE_K)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-6") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-7") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-8") PORT_CODE(KEYCODE_L)

//  PORT_START("DSW1")
//  PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
//  PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
//  PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
//  PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
//  PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
//  PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )

//  PORT_START("DSW2")
//  PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
//  PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
//  PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
//  PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
//  PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
//  PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout layout8x8x6 =
{
	8,8,
	RGN_FRAC(1,6),
	6,
	{ RGN_FRAC(3,6),
	  RGN_FRAC(0,6),
	  RGN_FRAC(4,6),
	  RGN_FRAC(1,6),
	  RGN_FRAC(5,6),
	  RGN_FRAC(2,6)
	},
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( lions )
	GFXDECODE_ENTRY( "gfx1", 0, layout8x8x6, 0, 1 )
GFXDECODE_END

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("IN2"),	/* DSW? */
	DEVCB_INPUT_PORT("IN3"),	/* DSW? */
	DEVCB_NULL,
	DEVCB_NULL
};

//static READ8_DEVICE_HANDLER( input_a )
//{
//return input_port_read(machine, "IN0");
//  return mame_rand(device->machine);
//return 0xff;
//}

//static READ8_DEVICE_HANDLER( input_b )
//{
//return input_port_read(machine, "IN1");
//      return mame_rand(device->machine);
//return 0xff;
//}

static READ8_DEVICE_HANDLER( input_ca1 )
{
//      return mame_rand(device->machine);
	return 0x00;
}

static READ8_DEVICE_HANDLER( input_cb1 )
{
//      return mame_rand(device->machine);
	return 0x00;
}

static READ8_DEVICE_HANDLER( input_ca2 )
{
//      return mame_rand(device->machine);

	return 0x00;
}

static READ8_DEVICE_HANDLER( input_cb2 )
{
//      return mame_rand(device->machine);

	return 0x00;
}

static WRITE8_DEVICE_HANDLER( output_a )
{
	popmessage("VIA outa: %02X", data);
	// ...
}

static WRITE8_DEVICE_HANDLER( output_b )
{
	popmessage("VIA outb: %02X", data);
	// ...
}


static WRITE8_DEVICE_HANDLER( output_ca1 )
{
	// ...
}

static WRITE8_DEVICE_HANDLER( output_cb1 )
{
	// ...
}

static WRITE8_DEVICE_HANDLER( output_ca2 )
{
	// ...
}

static WRITE8_DEVICE_HANDLER( output_cb2 )
{
	// ...
}

static const via6522_interface via_interface =
{
	/*inputs : A/B         */ DEVCB_INPUT_PORT("IN0"), DEVCB_INPUT_PORT("IN1"),
	/*inputs : CA/B1,CA/B2 */ DEVCB_HANDLER(input_ca1), DEVCB_HANDLER(input_cb1), DEVCB_HANDLER(input_ca2), DEVCB_HANDLER(input_cb2),
	/*outputs: A/B         */ DEVCB_HANDLER(output_a), DEVCB_HANDLER(output_b),
	/*outputs: CA/B1,CA/B2 */ DEVCB_HANDLER(output_ca1), DEVCB_HANDLER(output_cb1), DEVCB_HANDLER(output_ca2), DEVCB_HANDLER(output_cb2),
	/*irq                  */ DEVCB_CPU_INPUT_LINE("maincpu", M6809_FIRQ_LINE)
};

static INTERRUPT_GEN( lions_irq )
{
}

static const mc6845_interface mc6845_intf =
{
	/* in fact is a mc6845 driving 4 pixels by memory address.
       that's why the big horizontal parameters */

	"screen",	/* screen we are acting on */
	4,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};

/* same as Aristocrat Mark-IV HW color offset 7 */
static PALETTE_INIT( lions )
{
	int i;

	for (i = 0;i < machine->config->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		bit0 = (i >> 0) & 0x01;
		bit1 = (i >> 1) & 0x01;
		b = 0x4f * bit0 + 0xa8 * bit1;
		bit0 = (i >> 1) & 0x01;
		bit1 = (i >> 2) & 0x01;
		bit2 = (i >> 3) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (i >> 3) & 0x01;
		bit1 = (i >> 4) & 0x01;
		bit2 = (i >> 5) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

static MACHINE_DRIVER_START( lions )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6809, MAIN_CLOCK/4)		 /* 3 MHz.(guess) */
	MDRV_CPU_PROGRAM_MAP(lions_map)
	MDRV_CPU_VBLANK_INT("screen", lions_irq )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(304, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 304-1, 0, 216-1)	/* from the crtc registers... updated by crtc */

	MDRV_GFXDECODE(lions)
	MDRV_PALETTE_LENGTH(64)
	MDRV_PALETTE_INIT(lions)

	MDRV_VIDEO_START(lions)
	MDRV_VIDEO_UPDATE(lions)

	MDRV_VIA6522_ADD("via6522_0", MAIN_CLOCK/12, via_interface)	/* 1 MHz.(only 1 or 2 MHz.are valid) */

	MDRV_MC6845_ADD("crtc", MC6845, MAIN_CLOCK/8, mc6845_intf)	/* 1.5 MHz.(logical guess to get a decent 59.6374 Hz.) */

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/8)	/* 1.5 MHz.(guess) */
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_DRIVER_END


ROM_START( 86lions )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lion_std.u9", 0xe000, 0x2000, CRC(994842b0) SHA1(72fc31c577ee70b07ce9a4f2e864fe113d32affe) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "rd0.u13", 0x00000, 0x1000, CRC(38c57504) SHA1(cc3ac1df644abc4586fc9f0e88531ba146b86b48) )
	ROM_LOAD( "gn0.u10", 0x01000, 0x1000, CRC(80dce6f4) SHA1(bf953eba9cb270297b0d0efffe15b926e94dfbe7) )
	ROM_LOAD( "bl0.u8",  0x02000, 0x1000, CRC(00ef4724) SHA1(714fafd035e2befbb35c6d00df52845745e58a93) )
	ROM_LOAD( "rd1.u12", 0x03000, 0x1000, CRC(350dd017) SHA1(ba273d4231e7e4c44922898cf5a70e8b1d6e2f9d) )
	ROM_LOAD( "gn1.u11", 0x04000, 0x1000, CRC(80dce6f4) SHA1(bf953eba9cb270297b0d0efffe15b926e94dfbe7) )
	ROM_LOAD( "bl1.u9",  0x05000, 0x1000, CRC(675e164a) SHA1(99346ca70bfe673b31d71dc6b3bbc3b8f961e87f) )

//  ROM_REGION( 0x20, "proms", 0 )
//  ROM_LOAD( "prom.x", 0x00, 0x20, NO_DUMP )
ROM_END


GAME( 1985, 86lions, 0, lions, lions, 0, ROT0, "Aristocrat",  "86 Lions", GAME_NOT_WORKING )

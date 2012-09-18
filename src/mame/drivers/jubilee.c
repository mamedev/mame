/******************************************************************************

  Jubilee Double-Up Poker
  -----------------------

  Driver by Roberto Fresca.


  Games running on this hardware:

  * Double-Up Poker,  198?,  Jubilee.


*******************************************************************************


  Hardware Notes:
  ---------------

  1x TMS9980
  1x M6845

  Unknown Xtal


*******************************************************************************


  *** Game Notes ***

  Nothing, yet...


*******************************************************************************

  --------------------
  ***  Memory Map  ***
  --------------------

  0x0000 - 0x2FFF    ; ROM space.
  0x???? - 0x????    ; Video RAM.


*******************************************************************************

  TMS9980A memory map:


  0000-0003 ---> Reset
  0004-0007 ---> Level 1
  0008-000B ---> Level 2
  000C-000F ---> Level 3
  0010-0013 ---> Level 4

  3FFC-3FFF ---> Load


*******************************************************************************


  DRIVER UPDATES:


  [2010-09-05]

  - Initial release.
  - Decoded graphics.
  - Proper tile colors.
  - Hooked the correct CPU.
  - Preliminary memory map.
  - Added technical notes.


  TODO:

  - Improve the memory map.
  - Find the correct video RAM offset.
  - Hook the CRT controller.
  - Sound.
  - Check clocks on a PCB (if someday appear!)


*******************************************************************************/


#define MASTER_CLOCK	XTAL_8MHz	/* guess */

#include "emu.h"
#include "cpu/tms9900/tms9900l.h"
#include "video/mc6845.h"


class jubilee_state : public driver_device
{
public:
	jubilee_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"){ }

	required_shared_ptr<UINT8> m_videoram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(jubileep_videoram_w);
	DECLARE_READ8_MEMBER(unk_r);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_jubileep(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(jubileep_interrupt);
};


/*************************
*     Video Hardware     *
*************************/


WRITE8_MEMBER(jubilee_state::jubileep_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(jubilee_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index];

	SET_TILE_INFO_MEMBER( 0, code, 0, 0);
}



void jubilee_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(jubilee_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


UINT32 jubilee_state::screen_update_jubileep(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}


void jubilee_state::palette_init()
{

}


/**************************
*  Read / Write Handlers  *
**************************/

INTERRUPT_GEN_MEMBER(jubilee_state::jubileep_interrupt)
{
	/* doesn't seems to work properly. need to set level1 interrupts */
	device.execute().set_input_line_and_vector(0, ASSERT_LINE, 3);//2=nmi  3,4,5,6
}


/*************************
* Memory Map Information *
*************************/
//59a
static ADDRESS_MAP_START( jubileep_map, AS_PROGRAM, 8, jubilee_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x3000, 0x30ff) AM_WRITE(jubileep_videoram_w) AM_SHARE("videoram")	/* wrong... just placed somewhere */
	AM_RANGE(0x3100, 0x3fff) AM_RAM
ADDRESS_MAP_END

/*

  WRAM = 3400-37ff
  VRAM = 3800-3bff?

  RAM = 3c00-3fff

*/


READ8_MEMBER(jubilee_state::unk_r)
{
	return (machine().rand() & 0xff);
}

static ADDRESS_MAP_START( jubileep_cru_map, AS_IO, 8, jubilee_state )
//  AM_RANGE(0x0000, 0xffff) AM_READ(unk_r)
//  AM_RANGE(0x00, 0x00) AM_DEVREADWRITE_LEGACY("crtc",  mc6845_status_r, mc6845_address_w)
//  AM_RANGE(0x01, 0x01) AM_DEVREADWRITE_LEGACY("crtc", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0xc8, 0xc8) AM_READ(unk_r)
ADDRESS_MAP_END

/* I/O byte R/W


   -----------------

   unknown writes:


*/

/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( jubileep )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN0-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN0-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN0-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN0-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN0-8")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN1-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN1-8")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("IN2-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN2-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN2-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN2-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN2-8")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("IN3-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("IN3-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("IN3-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("IN3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("IN3-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("IN3-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN3-8")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN4-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("IN4-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("IN4-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN4-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("IN4-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("IN4-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("IN4-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("IN4-8")

	PORT_START("DSW1")
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

	PORT_START("DSW2")
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

	PORT_START("DSW3")
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

	PORT_START("DSW4")
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


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1,3),
	3,
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },    /* bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( jubileep )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0, 16 )
GFXDECODE_END


/***********************
*    CRTC Interface    *
************************/

static const mc6845_interface mc6845_intf =
{
	"screen",	/* screen we are acting on */
	8,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};


/*************************
*    Machine Drivers     *
*************************/

static MACHINE_CONFIG_START( jubileep, jubilee_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS9980L, MASTER_CLOCK/2)	/* guess */
	MCFG_CPU_PROGRAM_MAP(jubileep_map)
	MCFG_CPU_IO_MAP(jubileep_cru_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", jubilee_state,  jubileep_interrupt)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(jubilee_state, screen_update_jubileep)

	MCFG_GFXDECODE(jubileep)

	MCFG_PALETTE_LENGTH(256)


	MCFG_MC6845_ADD("crtc", MC6845, MASTER_CLOCK/4, mc6845_intf) /* guess */

MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( jubileep )
	ROM_REGION( 0x4000, "maincpu", 0 ) /* TMS9980 code */
	ROM_LOAD( "1_ic59.bin",	0x0000, 0x1000, CRC(534c81c2) SHA1(4ce1d4492de9cbbc37e5a946b1183d8e8b0ba989) )
	ROM_LOAD( "2_ic58.bin",	0x1000, 0x1000, CRC(69984028) SHA1(c919a5cb43f23a0d9e496107997c74799709b347) )
	ROM_LOAD( "3_ic57.bin",	0x2000, 0x1000, CRC(c9ae423d) SHA1(8321e3e6fd60d92202b0c7b47e2a333a567b5c22) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "ic49.bin",	0x0000, 0x2000, CRC(ec65d259) SHA1(9e82e4043cbea26b91965a19507a5f00dc3ba01a) )
	ROM_LOAD( "ic48.bin",	0x2000, 0x2000, CRC(74e9ffd9) SHA1(7349fea72a349a58014b795ec6c29647e7159d39) )
	ROM_LOAD( "ic47.bin",	0x4000, 0x2000, CRC(55dc8482) SHA1(53f22bd66e5fcad5e2397998bc58109c3c19af96) )
ROM_END


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME          PARENT  MACHINE   INPUT     INIT  ROT    COMPANY    FULLNAME                  FLAGS */
GAME( 198?, jubileep,     0,      jubileep, jubileep, driver_device, 0,    ROT0, "Jubilee", "Jubilee Double-Up Poker", GAME_NO_SOUND | GAME_NOT_WORKING )

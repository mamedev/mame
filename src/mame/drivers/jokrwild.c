/******************************************************************************

    JOKER'S WILD - SIGMA
    --------------------

    Preliminary driver by Roberto Fresca.


    Games running on this hardware:

    * Joker's Wild (encrypted).   1988, Sigma.


*******************************************************************************


    Hardware Notes (guessed):
    ------------------------

    CPU:    1x M6809
    Video:  1x M6845 CRTC or similar.
    I/O:    2x PIAs ?? (there is code to initialize PIAs at $8033)


*******************************************************************************


    *** Game Notes ***

    Game is trying to boot, and after approx. 90 seconds an error message appear
    on screen: "Random number generator is defective", then stuck here.

    See code at $9859

    PIAs are commented out just to see the R/W on error log.


*******************************************************************************

    --------------------
    ***  Memory Map  ***
    --------------------

    0x0000 - 0x07FF    ; Video RAM.
    0x2000 - 0x27FF    ; Color RAM.
    0x4004 - 0x4007    ; PIA?.
    0x4008 - 0x400B    ; PIA?.
    0x6000 - 0x6001    ; M6845 CRTC.
    0x8000 - 0xFFFF    ; ROM space.


    *** MC6545 Initialization ***
    ----------------------------------------------------------------------------------------------------------------------
    register:  R00   R01   R02   R03   R04   R05   R06   R07   R08   R09   R10   R11   R12   R13   R14   R15   R16   R17
    ----------------------------------------------------------------------------------------------------------------------
    value:     0x20  0x18  0x1B  0x64  0x20  0x07  0x1A  0x1D  0x00  0x07  0x00  0x00  0x00  0x00  0x00  0x00  0x00  0x00.


*******************************************************************************


    DRIVER UPDATES:


    [2008-10-30]

    - Fixed graphics to 2 bits per pixel.


    [2008-10-25]

    - Initial release.
    - ROMs load OK.
    - Proper ROMs decryption.
    - Added MC6845 CRTC.
    - Video RAM OK.
    - Added technical notes.


    TODO:

    - RND number generator.
    - Inputs
    - Sound.
    - A lot of work.


*******************************************************************************/


#define MASTER_CLOCK	XTAL_8MHz	/* guess */

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/mc6845.h"
#include "machine/6821pia.h"
#include "machine/nvram.h"


class jokrwild_state : public driver_device
{
public:
	jokrwild_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(jokrwild_videoram_w);
	DECLARE_WRITE8_MEMBER(jokrwild_colorram_w);
	DECLARE_READ8_MEMBER(rng_r);
	DECLARE_WRITE8_MEMBER(testa_w);
	DECLARE_WRITE8_MEMBER(testb_w);
};


/*************************
*     Video Hardware     *
*************************/


WRITE8_MEMBER(jokrwild_state::jokrwild_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(jokrwild_state::jokrwild_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


static TILE_GET_INFO( get_bg_tile_info )
{
	jokrwild_state *state = machine.driver_data<jokrwild_state>();
/*  - bits -
    7654 3210
    xx-- ----   bank select.
    ---- xxxx   color code.
*/
	int attr = state->m_colorram[tile_index];
	int code = state->m_videoram[tile_index] | ((attr & 0xc0) << 2);
	int color = (attr & 0x0f);

	SET_TILE_INFO( 0, code , color , 0);
}


static VIDEO_START( jokrwild )
{
	jokrwild_state *state = machine.driver_data<jokrwild_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 24, 26);
}


static SCREEN_UPDATE_IND16( jokrwild )
{
	jokrwild_state *state = screen.machine().driver_data<jokrwild_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}


static PALETTE_INIT( jokrwild )
{
	//missing proms
}


/*************************
*      Machine Init      *
*************************/


/*****************************
*    Read/Write  Handlers    *
*****************************/

READ8_MEMBER(jokrwild_state::rng_r)
{
	if(cpu_get_pc(&space.device()) == 0xab32)
		return (offset == 0) ? 0x9e : 0x27;

	if(cpu_get_pc(&space.device()) == 0xab3a)
		return (offset == 2) ? 0x49 : 0x92;

	return machine().rand() & 0xff;
}

/*************************
* Memory Map Information *
*************************/

static ADDRESS_MAP_START( jokrwild_map, AS_PROGRAM, 8, jokrwild_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM_WRITE(jokrwild_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x0400, 0x07ff) AM_RAM //FIXME: backup RAM
	AM_RANGE(0x2000, 0x23ff) AM_RAM_WRITE(jokrwild_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x2400, 0x27ff) AM_RAM //stack RAM
	AM_RANGE(0x4004, 0x4007) AM_DEVREADWRITE("pia0", pia6821_device, read, write)
	AM_RANGE(0x4008, 0x400b) AM_DEVREADWRITE("pia1", pia6821_device, read, write) //optical sensor is here
//  AM_RANGE(0x4010, 0x4010) AM_READNOP /* R ???? */
	AM_RANGE(0x6000, 0x6000) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x6001, 0x6001) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x6100, 0x6100) AM_READ_PORT("SW1")
	AM_RANGE(0x6200, 0x6203) AM_READ(rng_r)//another PIA?
	AM_RANGE(0x6300, 0x6300) AM_READ_PORT("SW2")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/* I/O byte R/W


   -----------------

   unknown writes:

  4004-400b R/W  ; 2x PIAs?
  4010      R    ; unknown.

  6100      R/W  ; unknown.
  6200-6203 R/W  ; extra PIA?
  6300      R    ; unknown.


*/

/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( jokrwild )
	PORT_START("IN0")
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

/*  PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-1") PORT_CODE(KEYCODE_1)
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-2") PORT_CODE(KEYCODE_2)
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-3") PORT_CODE(KEYCODE_3)
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-4") PORT_CODE(KEYCODE_4)
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-5") PORT_CODE(KEYCODE_5)
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-6") PORT_CODE(KEYCODE_6)
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-7") PORT_CODE(KEYCODE_7)
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-8") PORT_CODE(KEYCODE_8)
*/
	PORT_START("IN1")
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
/*  PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-1") PORT_CODE(KEYCODE_Q)
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-2") PORT_CODE(KEYCODE_W)
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-3") PORT_CODE(KEYCODE_E)
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-4") PORT_CODE(KEYCODE_R)
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-5") PORT_CODE(KEYCODE_T)
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-6") PORT_CODE(KEYCODE_Y)
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-7") PORT_CODE(KEYCODE_U)
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-8") PORT_CODE(KEYCODE_I)
*/
	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-7") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-8") PORT_CODE(KEYCODE_K)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-6") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-7") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-8") PORT_CODE(KEYCODE_L)

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "sw1" )
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

	PORT_START("SW2")
	PORT_DIPNAME( 0x01, 0x01, "sw2" )
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

static const gfx_layout charlayout =
{
	8, 8,
	0x400,	/* tiles */
	2,		/* 2 bpp */
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( jokrwild )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 16 )
GFXDECODE_END


/***********************
*    PIA Interfaces    *
***********************/

WRITE8_MEMBER(jokrwild_state::testa_w)
{
//  printf("%02x A\n",data);
}

WRITE8_MEMBER(jokrwild_state::testb_w)
{
//  printf("%02x B\n",data);
}

static const pia6821_interface pia0_intf =
{
	DEVCB_INPUT_PORT("IN0"),		/* port A in */
	DEVCB_INPUT_PORT("IN1"),		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DRIVER_MEMBER(jokrwild_state,testa_w),		/* port A out */
	DEVCB_DRIVER_MEMBER(jokrwild_state,testb_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

static const pia6821_interface pia1_intf =
{
	DEVCB_INPUT_PORT("IN2"),		/* port A in */
	DEVCB_INPUT_PORT("IN3"),		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};


/************************
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

static MACHINE_CONFIG_START( jokrwild, jokrwild_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/2)	/* guess */
	MCFG_CPU_PROGRAM_MAP(jokrwild_map)
	MCFG_CPU_VBLANK_INT("screen", nmi_line_pulse)

//  MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_PIA6821_ADD("pia0", pia0_intf)
	MCFG_PIA6821_ADD("pia1", pia1_intf)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE((32+1)*8, (32+1)*8)                  /* From MC6845, registers 00 & 04. (value-1) */
	MCFG_SCREEN_VISIBLE_AREA(0*8, 24*8-1, 0*8, 26*8-1)    /* From MC6845, registers 01 & 06 */
	MCFG_SCREEN_UPDATE_STATIC(jokrwild)

	MCFG_GFXDECODE(jokrwild)
	MCFG_PALETTE_INIT(jokrwild)
	MCFG_PALETTE_LENGTH(512)
	MCFG_VIDEO_START(jokrwild)

	MCFG_MC6845_ADD("crtc", MC6845, MASTER_CLOCK/16, mc6845_intf) /* guess */

MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/
/*
    One day I will write about HOW I got this dump... :)

   "Only two things are infinite: The universe and human stupidity...
    and I'm not sure about the universe."  (Albert Einstein)

*/
ROM_START( jokrwild )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jwild.7b",	0x8000, 0x4000, CRC(744cd029) SHA1(766faea330836344ffc6a1b4e1a64a679b9bf579) )
	ROM_LOAD( "jwild.7a",	0xc000, 0x4000, CRC(ca8e4f58) SHA1(a4f682980fe562dcd8743890ce94619719cd1153) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "jwild.2h",	0x0000, 0x0800, CRC(aed38e00) SHA1(9530078f6c22d67594606476c3698a75e052d1d6) )
	ROM_LOAD( "jwild.2g",	0x1000, 0x0800, CRC(d635f025) SHA1(f70d5a837797e2250a7e581b96e60a704da25511) )
	ROM_LOAD( "jwild.2f",	0x0800, 0x0800, CRC(9c1e057c) SHA1(23fd630aa20a4ffa5179d4a4fa32c6ee4b3f9c1b) )
	ROM_LOAD( "jwild.2e",	0x1800, 0x0800, CRC(a66ae0a1) SHA1(8e6bfcb169148fdbcc36f4f35747c4805762ddd7) )
	ROM_LOAD( "jwild.2d",	0x2000, 0x0800, CRC(76a0bcb4) SHA1(34c24ad63b1182166209074259e8f0aabe1ad331) )
	ROM_LOAD( "jwild.2c",	0x3000, 0x0800, CRC(8d5e0b8f) SHA1(da6692d0c2074427f801b3f1861e3d03075963a2) )
	ROM_LOAD( "jwild.2b",	0x2800, 0x0800, CRC(a264b0be) SHA1(a935dd3df8bbae9b7788d6c2a8c378fad07d2b43) )
	ROM_LOAD( "jwild.2a",	0x3800, 0x0800, CRC(8084d0c2) SHA1(370f30f0138e2f7743a97df92379c7b879d90aed) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "prom.x", 0x0000, 0x0100, NO_DUMP )
ROM_END


/**************************
*  Driver Initialization  *
**************************/

static DRIVER_INIT( jokrwild )
/*****************************************************************************

    Encryption was made by pages of 256 bytes.

    For each page, the value is XORed with a fixed value (0xCC),
    then XORed again with the offset of the original value inside its own page.

    Example:

    For encrypted value at offset 0x123A (0x89)...

    0x89 XOR 0xCC XOR 0x3A = 0x7F


*****************************************************************************/
{
	int i, offs;
	UINT8 *srcp = machine.root_device().memregion( "maincpu" )->base();

	for (i = 0x8000; i < 0x10000; i++)
	{
		offs = i & 0xff;
		srcp[i] = srcp[i] ^ 0xcc ^ offs;
	}
}


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     INIT      ROT    COMPANY  FULLNAME                   FLAGS */
GAME( 1988, jokrwild, 0,      jokrwild, jokrwild, jokrwild_state, jokrwild, ROT0, "Sigma", "Joker's Wild (encrypted)", GAME_NO_SOUND | GAME_NOT_WORKING )

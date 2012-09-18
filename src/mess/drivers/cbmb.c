/***************************************************************************
    commodore b series computer

    PeT mess@utanet.at

    documentation
     vice emulator
     www.funet.fi
***************************************************************************/

/*

2008 - Driver Updates
---------------------

(most of the informations are taken from http://www.zimmers.net/cbmpics/ )
(according to http://ca.geocities.com/sjgray@rogers.com/CBM/index.html there
are MANY more models in this family... but I haven't found confirmations, nor
dumps)

[CBM systems which belong to this driver]

* Commodore CBM-II Series (1983)

  Notice that, despite the Datasette port present on the board, the BASIC ROMs
miss the routines needed to actually load from a tape

  Various models:

U.S. name           EU name     RAM

B500 (*)            -           128k
B128                CBM 610     128k
B256                CBM 620     256k
B128-80HP (**)      CBM 710     128k
B256-80HP (**)      CBM 720     256k
BX256-80HP (***)    CBM 730     256k

(*) Prototype for the B128? As you can read at http://www.zimmers.net/cbmpics/cb500.html
    the naming of these machines is not clear: either they were B500/128 &
    B500/256 or B128 & B256. However, these names always refer to the low
    profile models.
(**) HP stands for High Profile. The other systems were the low profile models.
    HP machines had a detachable keyboard
(***) Additional 8088 CPU present

CPU: MOS 6509 (2 MHz)
RAM: 128 Kilobytes (Expandable to 256k internal, 704k external)
ROM: 24 Kilobytes
Video: MOS 6545 CTRC (Text: 80 columns, 25 rows)
Sound: MOS 6581 SID (3 voice stereo synthesizer/digital sound capabilities)
Ports: CSG 6551/6525x2/6526 (IEEE-488 port; CBM Datasette port; RS232 port;
    CBM Monitor port; CBM-II/PET-II expansion port; 1 RCA audio port; Power
    and reset switches
Keyboard: Full-sized 102 key QWERTY (19 key numeric keypad!; 4 direction
    cursor-pad)


* Commodore PET-II Series (1983)

  This series only features the P500 machine, which never even reached the
market. In this machine, the Datasette port can be used (the BASIC ROMs
contain the necessary routines). It is also probably know as C128-40.

CPU: CSG 6509 (1 MHz)
RAM: 128 kilobytes, expandable to 720k
ROM: 24 kilobytes
Video: CSG 6569 "VIC-II" (320 x 200 Hi-Resolution; 40 columns text; Palette
    of 16 colors)
Sound: CSG 6581 "SID" (3 voice stereo synthesizer/digital sound capabilities)
Ports: CSG 6551/6522 (2 Joystick/Mouse ports; IEEE-488 port; CBM Datasette
    port; RS232 port; CBM Monitor port; CBM-II/PET-II expansion port; 1 RCA
    audio port; Power and reset switches)
Keyboard: Full-sized 102 key QWERTY (19 key numeric keypad!; 4 direction
    cursor-pad)

[To Do]

* Support is still missing for: Sound, internal slots

* Emulate 8088 co-processor for BX-256HP and eventually add its European
    counterpart CBM 730

* Add better P500 emulation (almost everything: memory access, inputs,
    Datasette, etc.)

* Was CBM 710 / 720 monitor at 50Hz? If not remove MACHINE_CONFIG_START( cbm700pal, cbmb_state )
    and use the 60Hz version for the whole High Profile

* Find info about the following models (if ever existed):
    + BX128-80HP
    + CBM 700
    + CBM 505, CBM 510 (these seems proto as CBM 500 but with less RAM, 64
        & 128)
*/


#include "emu.h"
//#include "cpu/i86/i86.h"
#include "cpu/m6502/m6509.h"
#include "sound/sid6581.h"
#include "machine/6525tpi.h"
#include "machine/6526cia.h"

#include "formats/cbm_snqk.h"
#include "includes/cbm.h"
#include "includes/cbmb.h"
#include "machine/ieee488.h"
#include "machine/cbmipt.h"
#include "video/mos6566.h"
#include "video/mc6845.h"

#include "includes/cbmb.h"


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/


static ADDRESS_MAP_START(cbmb_mem , AS_PROGRAM, 8, cbmb_state )
	AM_RANGE(0x00000, 0x0ffff) AM_READONLY AM_WRITENOP
	AM_RANGE(0x10000, 0x4ffff) AM_RAM
	AM_RANGE(0x50002, 0x5ffff) AM_READONLY AM_WRITENOP
	AM_RANGE(0x60000, 0xf07ff) AM_RAM
#if 0
	AM_RANGE(0xf0800, 0xf0fff) AM_READ_LEGACY(SMH_ROM)
#endif
	AM_RANGE(0xf1000, 0xf1fff) AM_ROM	/* cartridges or ram */
	AM_RANGE(0xf2000, 0xf3fff) AM_ROM	/* cartridges or ram */
	AM_RANGE(0xf4000, 0xf5fff) AM_ROM
	AM_RANGE(0xf6000, 0xf7fff) AM_ROM
	AM_RANGE(0xf8000, 0xfbfff) AM_ROM AM_SHARE("basic")
	AM_RANGE(0xfd000, 0xfd7ff) AM_RAM AM_SHARE("videoram") /* VIDEORAM */
	AM_RANGE(0xfd800, 0xfd800) AM_MIRROR(0xfe) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xfd801, 0xfd801) AM_MIRROR(0xfe) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	/* disk units */
	AM_RANGE(0xfda00, 0xfdaff) AM_DEVREADWRITE("sid6581", sid6581_device, read, write)
	/* db00 coprocessor */
	AM_RANGE(0xfdc00, 0xfdcff) AM_DEVREADWRITE_LEGACY("cia", mos6526_r, mos6526_w)
	/* dd00 acia */
	AM_RANGE(0xfde00, 0xfdeff) AM_DEVREADWRITE_LEGACY("tpi6525_0", tpi6525_r, tpi6525_w)
	AM_RANGE(0xfdf00, 0xfdfff) AM_DEVREADWRITE_LEGACY("tpi6525_1", tpi6525_r, tpi6525_w)
	AM_RANGE(0xfe000, 0xfffff) AM_ROM AM_SHARE("kernal")
ADDRESS_MAP_END

static ADDRESS_MAP_START(p500_mem , AS_PROGRAM, 8, cbmb_state )
	AM_RANGE(0x00000, 0x1ffff) AM_RAM
	AM_RANGE(0x20000, 0x2ffff) AM_READONLY AM_WRITENOP
	AM_RANGE(0x30000, 0x7ffff) AM_RAM
	AM_RANGE(0x80000, 0x8ffff) AM_READONLY AM_WRITENOP
	AM_RANGE(0x90000, 0xf07ff) AM_RAM
#if 0
	AM_RANGE(0xf0800, 0xf0fff) AM_READ_LEGACY(SMH_ROM)
#endif
	AM_RANGE(0xf1000, 0xf1fff) AM_ROM	/* cartridges or ram */
	AM_RANGE(0xf2000, 0xf3fff) AM_ROM	/* cartridges or ram */
	AM_RANGE(0xf4000, 0xf5fff) AM_ROM
	AM_RANGE(0xf6000, 0xf7fff) AM_ROM
	AM_RANGE(0xf8000, 0xfbfff) AM_ROM AM_SHARE("basic")
	AM_RANGE(0xfd000, 0xfd3ff) AM_RAM AM_SHARE("videoram")		/* videoram */
	AM_RANGE(0xfd400, 0xfd7ff) AM_RAM_WRITE(cbmb_colorram_w) AM_SHARE("colorram")		/* colorram */
	AM_RANGE(0xfd800, 0xfd8ff) AM_DEVREADWRITE("vic6567", mos6566_device, read, write)
	/* disk units */
	AM_RANGE(0xfda00, 0xfdaff) AM_DEVREADWRITE("sid6581", sid6581_device, read, write)
	/* db00 coprocessor */
	AM_RANGE(0xfdc00, 0xfdcff) AM_DEVREADWRITE_LEGACY("cia", mos6526_r, mos6526_w)
	/* dd00 acia */
	AM_RANGE(0xfde00, 0xfdeff) AM_DEVREADWRITE_LEGACY("tpi6525_0", tpi6525_r, tpi6525_w)
	AM_RANGE(0xfdf00, 0xfdfff) AM_DEVREADWRITE_LEGACY("tpi6525_1", tpi6525_r, tpi6525_w)
	AM_RANGE(0xfe000, 0xfffff) AM_ROM AM_SHARE("kernal")
ADDRESS_MAP_END


/*************************************
 *
 *  Input Ports
 *
 *************************************/


static INPUT_PORTS_START( p500 )
	PORT_INCLUDE( cbmb_keyboard )			/* ROW0 -> ROW11 */

	PORT_INCLUDE( cbmb_special )			/* SPECIAL */

	PORT_INCLUDE( c64_controls )			/* CTRLSEL, JOY0, JOY1, PADDLE0 -> PADDLE3, TRACKX, TRACKY, LIGHTX, LIGHTY, OTHER */
INPUT_PORTS_END


static INPUT_PORTS_START( cbm600 )
	PORT_INCLUDE( p500 )

	PORT_MODIFY("ROW0")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Graph  Norm") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RVS  Off") PORT_CODE(KEYCODE_HOME)
INPUT_PORTS_END


static INPUT_PORTS_START( cbm600pal )
	PORT_INCLUDE( p500 )

	PORT_MODIFY("ROW0")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Graph  Norm") PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ASCII  DIN") PORT_CODE(KEYCODE_PGUP)
INPUT_PORTS_END


static INPUT_PORTS_START( cbm700 )
	PORT_INCLUDE( p500 )

	PORT_MODIFY("ROW0")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RVS  Off") PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Graph  Norm") PORT_CODE(KEYCODE_PGUP)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/


static const unsigned char cbm700_palette[] =
{
	0,0,0, /* black */
	0,0x80,0, /* green */
};

static const unsigned short cbmb_colortable[] = {
	0, 1
};

static const gfx_layout cbm600_charlayout =
{
	8,16,
	256,                                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes; 1 bit per pixel */
	/* x offsets */
	{ 0,1,2,3,4,5,6,7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	  8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8
	},
	8*16
};

static const gfx_layout cbm700_charlayout =
{
	9,16,
	256,                                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes; 1 bit per pixel */
	/* x offsets */
	{ 0,1,2,3,4,5,6,7,7 }, // 8.column will be cleared in cbm700_vh_start
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	  8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8
	},
	8*16
};

static GFXDECODE_START( cbm600 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, cbm600_charlayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, cbm600_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START( cbm700 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, cbm700_charlayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, cbm700_charlayout, 0, 1 )
GFXDECODE_END

PALETTE_INIT_MEMBER(cbmb_state,cbm700)
{
	int i;

	for ( i = 0; i < 2; i++ ) {
		palette_set_color_rgb(machine(), i, cbm700_palette[i*3], cbm700_palette[i*3+1], cbm700_palette[i*3+2]);
	}
}


static const mc6845_interface cbm600_crtc = {
	"screen",
	8 /*?*/,
	NULL,
	cbm600_update_row,
	NULL,
	DEVCB_LINE(cbmb_display_enable_changed),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};

static const mc6845_interface cbm700_crtc = {
	"screen",
	9 /*?*/,
	NULL,
	cbm700_update_row,
	NULL,
	DEVCB_LINE(cbmb_display_enable_changed),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};

/* p500 uses a VIC II chip */

READ8_MEMBER( cbmb_state::vic_lightpen_x_cb )
{
	return ioport("LIGHTX")->read() & ~0x01;
}

READ8_MEMBER( cbmb_state::vic_lightpen_y_cb )
{
	return ioport("LIGHTY")->read() & ~0x01;
}

READ8_MEMBER( cbmb_state::vic_lightpen_button_cb )
{
	return ioport("OTHER")->read() & 0x04;
}

READ8_MEMBER( cbmb_state::vic_dma_read )
{
	if (offset >= 0x1000)
		return m_videoram[offset & 0x3ff];
	else
		return m_chargen[offset & 0xfff];
}

READ8_MEMBER( cbmb_state::vic_dma_read_color )
{
	return m_colorram[offset & 0x3ff];
}

READ8_MEMBER( cbmb_state::vic_rdy_cb )
{
	return ioport("CTRLSEL")->read() & 0x08;
}

static ADDRESS_MAP_START( vic_videoram_map, AS_0, 8, cbmb_state )
	AM_RANGE(0x0000, 0x3fff) AM_READ(vic_dma_read)
ADDRESS_MAP_END

static ADDRESS_MAP_START( vic_colorram_map, AS_1, 8, cbmb_state )
	AM_RANGE(0x000, 0x3ff) AM_READ(vic_dma_read_color)
ADDRESS_MAP_END

static MOS6567_INTERFACE( vic_intf )
{
	"screen",
	"maincpu",
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(cbmb_state, vic_lightpen_x_cb),
	DEVCB_DRIVER_MEMBER(cbmb_state, vic_lightpen_y_cb),
	DEVCB_DRIVER_MEMBER(cbmb_state, vic_lightpen_button_cb),
	DEVCB_DRIVER_MEMBER(cbmb_state, vic_rdy_cb)
};

static const sid6581_interface sid_intf =
{
	DEVCB_NULL,
	DEVCB_NULL
};


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static const tpi6525_interface cbmb_tpi_0_intf =
{
	DEVCB_LINE(cbmb_irq),
	DEVCB_HANDLER(cbmb_tpi0_port_a_r),
	DEVCB_HANDLER(cbmb_tpi0_port_a_w),
	DEVCB_HANDLER(cbmb_tpi0_port_b_r),
	DEVCB_HANDLER(cbmb_tpi0_port_b_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_LINE(cbmb_change_font),
	DEVCB_NULL
};

static const tpi6525_interface cbmb_tpi_1_intf =
{
	DEVCB_NULL,
	DEVCB_HANDLER(cbmb_keyboard_line_a),
	DEVCB_HANDLER(cbmb_keyboard_line_select_a),
	DEVCB_HANDLER(cbmb_keyboard_line_b),
	DEVCB_HANDLER(cbmb_keyboard_line_select_b),
	DEVCB_HANDLER(cbmb_keyboard_line_c),
	DEVCB_HANDLER(cbmb_keyboard_line_select_c),
	DEVCB_NULL,
	DEVCB_NULL
};

static IEEE488_INTERFACE( ieee488_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static MACHINE_CONFIG_START( cbm600, cbmb_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6509, 7833600)        /* 7.8336 MHz */
	MCFG_CPU_PROGRAM_MAP(cbmb_mem)

	MCFG_MACHINE_RESET_OVERRIDE(cbmb_state, cbmb )

    /* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640 - 1, 0, 200 - 1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)

	MCFG_GFXDECODE( cbm600 )
	MCFG_PALETTE_LENGTH(ARRAY_LENGTH(cbm700_palette) / 3)
	MCFG_PALETTE_INIT_OVERRIDE(cbmb_state, cbm700 )

	MCFG_MC6845_ADD("crtc", MC6845, XTAL_18MHz / 8 /*?*/ /*  I do not know if this is correct, please verify */, cbm600_crtc)

	MCFG_VIDEO_START_OVERRIDE(cbmb_state, cbmb_crtc )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sid6581", SID6581, 1000000)
	MCFG_SOUND_CONFIG(sid_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", cbmb, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)

	/* cia */
	MCFG_MOS6526R1_ADD("cia", 7833600, 60, cbmb_cia)

	/* tpi */
	MCFG_TPI6525_ADD("tpi6525_0", cbmb_tpi_0_intf)
	MCFG_TPI6525_ADD("tpi6525_1", cbmb_tpi_1_intf)

	/* IEEE bus */
	MCFG_CBM_IEEE488_ADD(ieee488_intf, "c8250")

	MCFG_FRAGMENT_ADD(cbmb_cartslot)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cbm600pal, cbm600 )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cbm700, cbm600 )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(720, 350)
	MCFG_SCREEN_VISIBLE_AREA(0, 720 - 1, 0, 350 - 1)
	MCFG_GFXDECODE( cbm700 )

	MCFG_DEVICE_REMOVE("crtc")
	MCFG_MC6845_ADD("crtc", MC6845, XTAL_18MHz / 8 /*? I do not know if this is correct, please verify */, cbm700_crtc)

	MCFG_VIDEO_START_OVERRIDE(cbmb_state, cbm700 )
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cbm700pal, cbm700 )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( bx256hp, cbm700 )

//  MCFG_CPU_ADD("8088", I8088, /* ? */)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( p500, cbmb_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6509, VIC6567_CLOCK)        /* 7.8336 MHz */
	MCFG_CPU_PROGRAM_MAP(p500_mem)
	//MCFG_CPU_PERIODIC_INT_DRIVER(cbmb_state, vic2_raster_irq,  VIC6567_HRETRACERATE)

	MCFG_MACHINE_RESET_OVERRIDE(cbmb_state, cbmb )

	/* video hardware */
	MCFG_MOS6567_ADD("vic6567", "screen", VIC6567_CLOCK, vic_intf, vic_videoram_map, vic_colorram_map)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sid6581", SID6581, 1000000)
	MCFG_SOUND_CONFIG(sid_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_QUICKLOAD_ADD("quickload", p500, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)

	/* cia */
	MCFG_MOS6526R1_ADD("cia", VIC6567_CLOCK, 60, cbmb_cia)

	/* tpi */
	MCFG_TPI6525_ADD("tpi6525_0", cbmb_tpi_0_intf)
	MCFG_TPI6525_ADD("tpi6525_1", cbmb_tpi_1_intf)

	/* IEEE bus */
	MCFG_CBM_IEEE488_ADD(ieee488_intf, "c8250")

	MCFG_FRAGMENT_ADD(cbmb_cartslot)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/


/* Low Profile Series - CBM 600 Series */
/* chargen: 8x16 chars for 8x8 size; 128 ascii, 128 ascii graphics; inversion logic in hardware */

ROM_START( b500 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "901243-01.u59",  0xf8000, 0x2000, CRC(22822706) SHA1(901bbf59d8b8682b481be8b2de99b406fffa4bab) )
	ROM_LOAD( "901242-01a.u60", 0xfa000, 0x2000, CRC(ef13d595) SHA1(2fb72985d7d4ab69c5780179178828c931a9f5b0) )
	ROM_LOAD( "901244-01.u61",  0xfe000, 0x2000, CRC(93414213) SHA1(a54a593dbb420ae1ac39b0acde9348160f7840ff) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "901237-01.u25", 0x0000, 0x1000, CRC(1acf5098) SHA1(e63bf18da48e5a53c99ef127c1ae721333d1d102) )

	ROM_REGION( 0xf5, "pla", 0 )
	ROM_LOAD( "906114-04.bin", 0x00, 0xf5, CRC(ae3ec265) SHA1(334e0bc4b2c957ecb240c051d84372f7b47efba3) )
ROM_END

ROM_START( b128 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "default", "BASIC 4.0 r" )
	ROMX_LOAD( "901243-04a.u59", 0xf8000, 0x2000, CRC(b0dcb56d) SHA1(08d333208060ee2ce84d4532028d94f71c016b96), ROM_BIOS(1) )
	ROMX_LOAD( "901242-04a.u60", 0xfa000, 0x2000, CRC(de04ea4f) SHA1(7c6de17d46a3343dc597d9b9519cf63037b31908), ROM_BIOS(1) )
	ROMX_LOAD( "901244-04a.u61", 0xfe000, 0x2000, CRC(09a5667e) SHA1(abb26418b9e1614a8f52bdeee0822d4a96071439), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "older", "BASIC 4.0" )
	ROMX_LOAD( "901243-02b.u59", 0xf8000, 0x2000, CRC(9d0366f9) SHA1(625f7337ea972a8bce2bdf2daababc0ed0b3b69b), ROM_BIOS(2) )
	ROMX_LOAD( "901242-02b.u60", 0xfa000, 0x2000, CRC(837978b5) SHA1(56e8d2f86bf73ba36b3d3cb84dd75806b66c530a), ROM_BIOS(2) )
	ROMX_LOAD( "901244-03b.u61", 0xfe000, 0x2000, CRC(4276dbba) SHA1(a624899c236bc4458570144d25aaf0b3be08b2cd), ROM_BIOS(2) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "901237-01.u25", 0x0000, 0x1000, CRC(1acf5098) SHA1(e63bf18da48e5a53c99ef127c1ae721333d1d102) )

	ROM_REGION( 0xf5, "pla", 0 )
	ROM_LOAD( "906114-04.bin", 0x00, 0xf5, CRC(ae3ec265) SHA1(334e0bc4b2c957ecb240c051d84372f7b47efba3) )
ROM_END

ROM_START( b256 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "901241-03.u59", 0xf8000, 0x2000, CRC(5c1f3347) SHA1(2d46be2cd89594b718cdd0a86d51b6f628343f42) )
	ROM_LOAD( "901240-03.u60", 0xfa000, 0x2000, CRC(72aa44e1) SHA1(0d7f77746290afba8d0abeb87c9caab9a3ad89ce) )
	ROM_SYSTEM_BIOS( 0, "default", "BASIC 4.0 r" )
	ROMX_LOAD( "901244-04a.u61", 0xfe000, 0x2000, CRC(09a5667e) SHA1(abb26418b9e1614a8f52bdeee0822d4a96071439), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "older", "BASIC 4.0" )
	ROMX_LOAD( "901244-03b.u61", 0xfe000, 0x2000, CRC(4276dbba) SHA1(a624899c236bc4458570144d25aaf0b3be08b2cd), ROM_BIOS(2) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "901237-01.u25", 0x0000, 0x1000, CRC(1acf5098) SHA1(e63bf18da48e5a53c99ef127c1ae721333d1d102) )

	ROM_REGION( 0xf5, "pla", 0 )
	ROM_LOAD( "906114-04.bin", 0x00, 0xf5, CRC(ae3ec265) SHA1(334e0bc4b2c957ecb240c051d84372f7b47efba3) )
ROM_END


#define rom_cbm610	rom_b128
#define rom_cbm620	rom_b256


ROM_START( cbm620hu )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "610.u60", 0xf8000, 0x4000, CRC(8eed0d7e) SHA1(9d06c5c3c012204eaaef8b24b1801759b62bf57e) )
	ROM_LOAD( "kernhun.bin", 0xfe000, 0x2000, CRC(0ea8ca4d) SHA1(9977c9f1136ee9c04963e0b50ae0c056efa5663f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "charhun.bin", 0x0000, 0x2000, CRC(1fb5e596) SHA1(3254e069f8691b30679b19a9505b6afdfedce6ac) )

	ROM_REGION( 0xf5, "pla", 0 )
	ROM_LOAD( "906114-04.bin", 0x00, 0xf5, CRC(ae3ec265) SHA1(334e0bc4b2c957ecb240c051d84372f7b47efba3) )
ROM_END


/* High Profile Series - CBM 700 Series */
/* chargen: 8x16 chars for 9x14 size */

ROM_START( b128hp )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "default", "BASIC 4.0 r" )
	ROMX_LOAD( "901243-04a.u59", 0xf8000, 0x2000, CRC(b0dcb56d) SHA1(08d333208060ee2ce84d4532028d94f71c016b96), ROM_BIOS(1) )
	ROMX_LOAD( "901242-04a.u60", 0xfa000, 0x2000, CRC(de04ea4f) SHA1(7c6de17d46a3343dc597d9b9519cf63037b31908), ROM_BIOS(1) )
	ROMX_LOAD( "901244-04a.u61", 0xfe000, 0x2000, CRC(09a5667e) SHA1(abb26418b9e1614a8f52bdeee0822d4a96071439), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "older", "BASIC 4.0" )
	ROMX_LOAD( "901243-02b.u59", 0xf8000, 0x2000, CRC(9d0366f9) SHA1(625f7337ea972a8bce2bdf2daababc0ed0b3b69b), ROM_BIOS(2) )
	ROMX_LOAD( "901242-02b.u60", 0xfa000, 0x2000, CRC(837978b5) SHA1(56e8d2f86bf73ba36b3d3cb84dd75806b66c530a), ROM_BIOS(2) )
	ROMX_LOAD( "901244-03b.u61", 0xfe000, 0x2000, CRC(4276dbba) SHA1(a624899c236bc4458570144d25aaf0b3be08b2cd), ROM_BIOS(2) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "901232-01.u25", 0x0000, 0x1000, CRC(3a350bc3) SHA1(e7f3cbc8e282f79a00c3e95d75c8d725ee3c6287) )

	ROM_REGION( 0xf5, "pla", 0 )
	ROM_LOAD( "906114-05.bin", 0x00, 0xf5, CRC(ff6ba6b6) SHA1(45808c570eb2eda7091c51591b3dbd2db1ac646a) )
ROM_END

ROM_START( b256hp )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "901241-03.u59", 0xf8000, 0x2000, CRC(5c1f3347) SHA1(2d46be2cd89594b718cdd0a86d51b6f628343f42) )
	ROM_LOAD( "901240-03.u60", 0xfa000, 0x2000, CRC(72aa44e1) SHA1(0d7f77746290afba8d0abeb87c9caab9a3ad89ce) )
	ROM_SYSTEM_BIOS( 0, "default", "BASIC 4.0 r" )
	ROMX_LOAD( "901244-04a.u61", 0xfe000, 0x2000, CRC(09a5667e) SHA1(abb26418b9e1614a8f52bdeee0822d4a96071439), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "older", "BASIC 4.0" )
	ROMX_LOAD( "901244-03b.u61", 0xfe000, 0x2000, CRC(4276dbba) SHA1(a624899c236bc4458570144d25aaf0b3be08b2cd), ROM_BIOS(2) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "901232-01.u25", 0x0000, 0x1000, CRC(3a350bc3) SHA1(e7f3cbc8e282f79a00c3e95d75c8d725ee3c6287) )

	ROM_REGION( 0xf5, "pla", 0 )
	ROM_LOAD( "906114-05.bin", 0x00, 0xf5, CRC(ff6ba6b6) SHA1(45808c570eb2eda7091c51591b3dbd2db1ac646a) )
ROM_END

#define rom_cbm710	rom_b128hp
#define rom_cbm720	rom_b256hp


/* BX-256HP - only ROM loading added */
ROM_START( bx256hp )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "901241-03.u59", 0xf8000, 0x2000, CRC(5c1f3347) SHA1(2d46be2cd89594b718cdd0a86d51b6f628343f42) )
	ROM_LOAD( "901240-03.u60", 0xfa000, 0x2000, CRC(72aa44e1) SHA1(0d7f77746290afba8d0abeb87c9caab9a3ad89ce) )
	ROM_SYSTEM_BIOS( 0, "default", "BASIC 4.0 r" )
	ROMX_LOAD( "901244-04a.u61", 0xfe000, 0x2000, CRC(09a5667e) SHA1(abb26418b9e1614a8f52bdeee0822d4a96071439), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "older", "BASIC 4.0" )
	ROMX_LOAD( "901244-03b.u61", 0xfe000, 0x2000, CRC(4276dbba) SHA1(a624899c236bc4458570144d25aaf0b3be08b2cd), ROM_BIOS(2) )

	ROM_REGION( 0x1000, "8088", 0)
	ROM_LOAD( "8088.u14", 0x0000, 0x1000, CRC(195e0281) SHA1(ce8acd2a5fb6cbd70d837811d856d656544a1f97) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "901232-01.u25", 0x0000, 0x1000, CRC(3a350bc3) SHA1(e7f3cbc8e282f79a00c3e95d75c8d725ee3c6287) )

	ROM_REGION( 0xf5, "pla", 0 )
	ROM_LOAD( "906114-05.bin", 0x00, 0xf5, CRC(ff6ba6b6) SHA1(45808c570eb2eda7091c51591b3dbd2db1ac646a) )
ROM_END

ROM_START( cbm720se )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "901241-03.u59", 0xf8000, 0x2000, CRC(5c1f3347) SHA1(2d46be2cd89594b718cdd0a86d51b6f628343f42) )
	ROM_LOAD( "901240-03.u60", 0xfa000, 0x2000, CRC(72aa44e1) SHA1(0d7f77746290afba8d0abeb87c9caab9a3ad89ce) )
	ROM_LOAD( "swe-901244-03.u61", 0xfe000, 0x2000, CRC(87bc142b) SHA1(fa711f6082741b05a9c80744f5aee68dc8c1dcf4) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "901233-03.u25", 0x0000, 0x1000, CRC(09518b19) SHA1(2e28491e31e2c0a3b6db388055216140a637cd09) )

	ROM_REGION( 0xf5, "pla", 0 )
	ROM_LOAD( "906114-05.bin", 0x00, 0xf5, CRC(ff6ba6b6) SHA1(45808c570eb2eda7091c51591b3dbd2db1ac646a) )
ROM_END

ROM_START( p500 )
	ROM_REGION( 0x101000, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "default", "BASIC 4.0 new" )
	ROMX_LOAD( "901236-02.bin", 0xf8000, 0x2000, CRC(c62ab16f) SHA1(f50240407bade901144f7e9f489fa9c607834eca), ROM_BIOS(1) )
	ROMX_LOAD( "901235-02.bin", 0xfa000, 0x2000, CRC(20b7df33) SHA1(1b9a55f12f8cf025754d8029cc5324b474c35841), ROM_BIOS(1) )
	ROMX_LOAD( "901234-02.bin", 0xfe000, 0x2000, CRC(f46bbd2b) SHA1(097197d4d08e0b82e0466a5f1fbd49a24f3d2523), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "old", "BASIC 4.0 old" )
	ROMX_LOAD( "901236-01.bin", 0xf8000, 0x2000, CRC(33eb6aa2) SHA1(7e3497ae2edbb38c753bd31ed1bf3ae798c9a976), ROM_BIOS(2) )
	ROMX_LOAD( "901235-01.bin", 0xfa000, 0x2000, CRC(18a27feb) SHA1(951b5370dd7db762b8504a141f9f26de345069bb), ROM_BIOS(2) )
	ROMX_LOAD( "901234-01.bin", 0xfe000, 0x2000, CRC(67962025) SHA1(24b41b65c85bf30ab4e2911f677ce9843845b3b1), ROM_BIOS(2) )

	ROM_LOAD( "901225-01.bin", 0x100000, 0x1000, CRC(ec4272ee) SHA1(adc7c31e18c7c7413d54802ef2f4193da14711aa) )

	ROM_REGION( 0xf5, "pla1", 0 )
	ROM_LOAD( "906114-02.bin", 0x00, 0xf5, CRC(6436b20b) SHA1(57ebebe771791288051afd1abe9b7500bd2df847) )

	ROM_REGION( 0xf5, "pla2", 0 )
	ROM_LOAD( "906114-03.bin", 0x00, 0xf5, CRC(668c073e) SHA1(1115858bb2dc91ea9e2016ba2e23ec94239358b4) )
ROM_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME      PARENT  COMPAT  MACHINE    INPUT      INIT       COMPANY                             FULLNAME                                  FLAGS */
COMP( 1983,	b500,     cbm610, 0,      cbm600,    cbm600, cbmb_state,    cbm600,    "Commodore Business Machines",  "B500 (proto, 60Hz)",                     GAME_NOT_WORKING )
COMP( 1983,	b128,     cbm610, 0,      cbm600pal, cbm600pal, cbmb_state, cbm600pal, "Commodore Business Machines",  "B128 (60Hz)",                            GAME_NOT_WORKING )
COMP( 1983,	b256,     cbm610, 0,      cbm600pal, cbm600pal, cbmb_state, cbm600hu,  "Commodore Business Machines",  "B256 (60Hz)",                            GAME_NOT_WORKING )
COMP( 1983,	cbm610,   0,      0,      cbm600,    cbm600, cbmb_state,    cbm600,    "Commodore Business Machines",  "CBM 610 (50Hz)",                         GAME_NOT_WORKING )
COMP( 1983,	cbm620,   cbm610, 0,      cbm600pal, cbm600pal, cbmb_state, cbm600pal, "Commodore Business Machines",  "CBM 620 (50Hz)",                         GAME_NOT_WORKING )
COMP( 1983,	cbm620hu, cbm610, 0,      cbm600pal, cbm600pal, cbmb_state, cbm600hu,  "Commodore Business Machines",  "CBM 620 (Hungary, 50Hz)",                GAME_NOT_WORKING )

COMP( 1983, b128hp,   cbm610, 0,      cbm700,    cbm700, cbmb_state,    cbm700,    "Commodore Business Machines",  "B128-80HP (60Hz)",                       GAME_NOT_WORKING )
COMP( 1983, b256hp,   cbm610, 0,      cbm700,    cbm700, cbmb_state,    cbm700,    "Commodore Business Machines",  "B256-80HP (60Hz)",                       GAME_NOT_WORKING )
COMP( 1983, cbm710,   cbm610, 0,      cbm700pal, cbm700, cbmb_state,    cbm700,    "Commodore Business Machines",  "CBM 710 (50Hz)",                         GAME_NOT_WORKING )
COMP( 1983, cbm720,   cbm610, 0,      cbm700pal, cbm700, cbmb_state,    cbm700,    "Commodore Business Machines",  "CBM 720 (50Hz)",                         GAME_NOT_WORKING )
COMP( 1983, cbm720se, cbm610, 0,      cbm700pal, cbm700, cbmb_state,    cbm700,    "Commodore Business Machines",  "CBM 720 (Sweden/Finland, 50Hz)",         GAME_NOT_WORKING )

COMP( 1983,	bx256hp,  cbm610, 0,      bx256hp,   cbm700, cbmb_state,    cbm700,    "Commodore Business Machines",  "BX256-80HP (60Hz)",                      GAME_NOT_WORKING )

COMP( 1983,	p500,     0,      0,      p500,      p500, cbmb_state,      p500,      "Commodore Business Machines",  "P500 (proto, a.k.a. B128-40 or Pet-II)", GAME_NOT_WORKING )

/***************************************************************************

    commodore c65 home computer
    PeT mess@utanet.at

    documention
     www.funet.fi

***************************************************************************/

/*

2008 - Driver Updates
---------------------

(most of the informations are taken from http://www.zimmers.net/cbmpics/ )


[CBM systems which belong to this driver]

* Commodore 65 (1989)

Also known as C64 DX at early stages of the project. It was cancelled
around 1990-1991. Only few units survive (they were sold after Commodore
liquidation in 1994).

CPU: CSG 4510 (3.54 MHz)
RAM: 128 kilobytes, expandable to 8 megabytes
ROM: 128 kilobytes
Video: CSG 4569 "VIC-III" (6 Video modes; Resolutions from 320x200 to
    1280x400; 80 columns text; Palette of 4096 colors)
Sound: CSG 8580 "SID" x2 (6 voice stereo synthesizer/digital sound
    capabilities)
Ports: CSG 4510 (2 Joystick/Mouse ports; CBM Serial port; CBM 'USER'
    port; CBM Monitor port; Power and reset switches; C65 bus drive
    port; RGBI video port; 2 RCA audio ports; RAM expansion port; C65
    expansion port)
Keyboard: Full-sized 77 key QWERTY (12 programmable function keys;
    4 direction cursor-pad)
Additional Hardware: Built in 3.5" DD disk drive (1581 compatible)
Miscellaneous: Partially implemented Commodore 64 emulation

[Notes]

The datasette port was removed here. C65 supports an additional "dumb"
drive externally. It also features, in addition to the standard CBM
bus serial (available in all modes), a Fast and a Burst serial bus
(both available in C65 mode only)

*/


#include "emu.h"
#include "cpu/m6502/m4510.h"
#include "sound/sid6581.h"
#include "machine/6526cia.h"
#include "machine/cbmipt.h"
#include "video/vic4567.h"
#include "includes/cbm.h"
#include "formats/cbm_snqk.h"
#include "includes/c64_legacy.h"
#include "includes/c65.h"
#include "machine/cbmiec.h"
#include "machine/ram.h"

/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( c65_mem , AS_PROGRAM, 8, c65_state )
	AM_RANGE(0x00000, 0x07fff) AM_RAMBANK("bank11")
	AM_RANGE(0x08000, 0x09fff) AM_READ_BANK("bank1") AM_WRITE_BANK("bank12")
	AM_RANGE(0x0a000, 0x0bfff) AM_READ_BANK("bank2") AM_WRITE_BANK("bank13")
	AM_RANGE(0x0c000, 0x0cfff) AM_READ_BANK("bank3") AM_WRITE_BANK("bank14")
	AM_RANGE(0x0d000, 0x0d7ff) AM_READ_BANK("bank4") AM_WRITE_BANK("bank5")
	AM_RANGE(0x0d800, 0x0dbff) AM_READ_BANK("bank6") AM_WRITE_BANK("bank7")
	AM_RANGE(0x0dc00, 0x0dfff) AM_READ_BANK("bank8") AM_WRITE_BANK("bank9")
	AM_RANGE(0x0e000, 0x0ffff) AM_READ_BANK("bank10") AM_WRITE_BANK("bank15")
	AM_RANGE(0x10000, 0x1f7ff) AM_RAM
	AM_RANGE(0x1f800, 0x1ffff) AM_RAM AM_SHARE("colorram")

	AM_RANGE(0x20000, 0x23fff) AM_ROM /* &c65_dos,     maps to 0x8000    */
	AM_RANGE(0x24000, 0x28fff) AM_ROM /* reserved */
	AM_RANGE(0x29000, 0x29fff) AM_ROM AM_SHARE("chargen")
	AM_RANGE(0x2a000, 0x2bfff) AM_ROM AM_SHARE("basic")
	AM_RANGE(0x2c000, 0x2cfff) AM_ROM AM_SHARE("interface")
	AM_RANGE(0x2d000, 0x2dfff) AM_ROM AM_SHARE("chargen")
	AM_RANGE(0x2e000, 0x2ffff) AM_ROM AM_SHARE("kernal")

	AM_RANGE(0x30000, 0x31fff) AM_ROM /*&c65_monitor,     monitor maps to 0x6000    */
	AM_RANGE(0x32000, 0x37fff) AM_ROM /*&c65_basic, */
	AM_RANGE(0x38000, 0x3bfff) AM_ROM /*&c65_graphics, */
	AM_RANGE(0x3c000, 0x3dfff) AM_ROM /* reserved */
	AM_RANGE(0x3e000, 0x3ffff) AM_ROM /* &c65_kernal, */

	AM_RANGE(0x40000, 0x7ffff) AM_NOP
	/* 8 megabyte full address space! */
ADDRESS_MAP_END


/*************************************
 *
 *  Input Ports
 *
 *************************************/

static INPUT_PORTS_START( c65 )
	PORT_INCLUDE( common_cbm_keyboard )		/* ROW0 -> ROW7 */

	PORT_START("FUNCT")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F13 F14") PORT_CODE(KEYCODE_F11)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F11 F12") PORT_CODE(KEYCODE_F10)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F9 F10") PORT_CODE(KEYCODE_F9)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("HELP") PORT_CODE(KEYCODE_F12)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ALT") PORT_CODE(KEYCODE_F2)		/* non blocking */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("NO SCRL") PORT_CODE(KEYCODE_F4)

	PORT_INCLUDE( c65_special )				/* SPECIAL */

	PORT_INCLUDE( c64_controls )			/* CTRLSEL, JOY0, JOY1, PADDLE0 -> PADDLE3, TRACKX, TRACKY, LIGHTX, LIGHTY, OTHER */
INPUT_PORTS_END


static INPUT_PORTS_START( c65ger )
	PORT_INCLUDE( c65 )

	PORT_MODIFY( "ROW1" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Z  { Y }") PORT_CODE(KEYCODE_Z)					PORT_CHAR('Z')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3  #  { 3  Paragraph }") PORT_CODE(KEYCODE_3)		PORT_CHAR('3') PORT_CHAR('#')

	PORT_MODIFY( "ROW3" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Y  { Z }") PORT_CODE(KEYCODE_Y)					PORT_CHAR('Y')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7 ' { 7  / }") PORT_CODE(KEYCODE_7)				PORT_CHAR('7') PORT_CHAR('\'')

	PORT_MODIFY( "ROW4" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0  { = }") PORT_CODE(KEYCODE_0)					PORT_CHAR('0')

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(",  <  { ; }") PORT_CODE(KEYCODE_COMMA)			PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Paragraph  \xE2\x86\x91  { \xc3\xbc }") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00A7) PORT_CHAR(0x2191)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(":  [  { \xc3\xa4 }") PORT_CODE(KEYCODE_COLON)		PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(".  >  { : }") PORT_CODE(KEYCODE_STOP)				PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("-  { '  ` }") PORT_CODE(KEYCODE_EQUALS)			PORT_CHAR('-')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("+  { \xc3\x9f ? }") PORT_CODE(KEYCODE_MINUS)		PORT_CHAR('+')

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("/  ?  { -  _ }") PORT_CODE(KEYCODE_SLASH)			PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Sum Pi  { ]  \\ }") PORT_CODE(KEYCODE_DEL)		PORT_CHAR(0x03A3) PORT_CHAR(0x03C0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("=  { #  ' }") PORT_CODE(KEYCODE_BACKSLASH)		PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(";  ]  { \xc3\xb6 }") PORT_CODE(KEYCODE_QUOTE)		PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("*  `  { +  * }") PORT_CODE(KEYCODE_CLOSEBRACE)	PORT_CHAR('*') PORT_CHAR('`')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\\  { [ \xE2\x86\x91 }") PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('\xa3')

	PORT_MODIFY( "ROW7" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("_  { <  > }") PORT_CODE(KEYCODE_TILDE)			PORT_CHAR('_')

	PORT_MODIFY("SPECIAL") /* special keys */
	PORT_DIPNAME( 0x20, 0x00, "(C65) DIN ASC (switch)") PORT_CODE(KEYCODE_F3)
	PORT_DIPSETTING(	0x00, "ASC" )
	PORT_DIPSETTING(	0x20, "DIN" )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

PALETTE_INIT_MEMBER(c65_state,c65)
{
	int i;

	for ( i = 0; i < 0x100; i++ )
	{
		palette_set_color_rgb(machine(), i, 0, 0, 0);
	}
}


/*************************************
 *
 *  Sound definitions
 *
 *************************************/

READ8_MEMBER( c65_state::sid_potx_r )
{
	device_t *sid = machine().device("sid_r");

	return c64_paddle_read(sid, space, 0);
}

READ8_MEMBER( c65_state::sid_poty_r )
{
	device_t *sid = machine().device("sid_r");

	return c64_paddle_read(sid, space, 1);
}

static MOS6581_INTERFACE( sidr_intf )
{
	DEVCB_DRIVER_MEMBER(c65_state, sid_potx_r),
	DEVCB_DRIVER_MEMBER(c65_state, sid_poty_r)
};

static MOS6581_INTERFACE( sidl_intf )
{
	DEVCB_NULL,
	DEVCB_NULL
};


static CBM_IEC_INTERFACE( cbm_iec_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


/*************************************
 *
 *  VIC III interfaces
 *
 *************************************/

static SCREEN_UPDATE_IND16( c65 )
{
	device_t *vic3 = screen.machine().device("vic3");

	vic3_video_update(vic3, bitmap, cliprect);
	return 0;
}

static UINT8 c65_lightpen_x_cb( running_machine &machine )
{
	return machine.root_device().ioport("LIGHTX")->read() & ~0x01;
}

static UINT8 c65_lightpen_y_cb( running_machine &machine )
{
	return machine.root_device().ioport("LIGHTY")->read() & ~0x01;
}

static UINT8 c65_lightpen_button_cb( running_machine &machine )
{
	return machine.root_device().ioport("OTHER")->read() & 0x04;
}

static UINT8 c65_c64_mem_r( running_machine &machine, int offset )
{
	c65_state *state = machine.driver_data<c65_state>();
	return state->m_memory[offset];
}

static const vic3_interface c65_vic3_ntsc_intf = {
	"screen",
	"maincpu",
	VIC4567_NTSC,
	c65_lightpen_x_cb,
	c65_lightpen_y_cb,
	c65_lightpen_button_cb,
	c65_dma_read,
	c65_dma_read_color,
	c65_vic_interrupt,
	c65_bankswitch_interface,
	c65_c64_mem_r
};

static const vic3_interface c65_vic3_pal_intf = {
	"screen",
	"maincpu",
	VIC4567_PAL,
	c65_lightpen_x_cb,
	c65_lightpen_y_cb,
	c65_lightpen_button_cb,
	c65_dma_read,
	c65_dma_read_color,
	c65_vic_interrupt,
	c65_bankswitch_interface,
	c65_c64_mem_r
};

static INTERRUPT_GEN( vic3_raster_irq )
{
	device_t *vic3 = device->machine().device("vic3");

	vic3_raster_interrupt_gen(vic3);
}

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( c65, c65_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M4510, 3500000)  /* or VIC6567_CLOCK, */
	MCFG_CPU_PROGRAM_MAP(c65_mem)
	MCFG_CPU_VBLANK_INT("screen", c65_frame_interrupt)
	MCFG_CPU_PERIODIC_INT(vic3_raster_irq, VIC6567_HRETRACERATE)

	MCFG_MACHINE_START_OVERRIDE(c65_state, c65 )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(VIC6567_VRETRACERATE)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(525 * 2, 520 * 2)
	MCFG_SCREEN_VISIBLE_AREA(VIC6567_STARTVISIBLECOLUMNS ,(VIC6567_STARTVISIBLECOLUMNS + VIC6567_VISIBLECOLUMNS - 1) * 2, VIC6567_STARTVISIBLELINES, VIC6567_STARTVISIBLELINES + VIC6567_VISIBLELINES - 1)
	MCFG_SCREEN_UPDATE_STATIC( c65 )

	MCFG_PALETTE_LENGTH(0x100)
	MCFG_PALETTE_INIT_OVERRIDE(c65_state, c65 )

	MCFG_VIC3_ADD("vic3", c65_vic3_ntsc_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("sid_r", SID8580, 985248)
	MCFG_SOUND_CONFIG(sidr_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
	MCFG_SOUND_ADD("sid_l", SID8580, 985248)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_CONFIG(sidl_intf)

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", cbm_c65, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)

	/* cia */
	MCFG_MOS6526R1_ADD("cia_0", 3500000, 60, c65_cia0)
	MCFG_MOS6526R1_ADD("cia_1", 3500000, 60, c65_cia1)

	/* floppy from serial bus */
	MCFG_CBM_IEC_ADD(cbm_iec_intf, NULL)

	MCFG_FRAGMENT_ADD(c64_cartslot)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("640K,4224K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( c65pal, c65 )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(VIC6569_VRETRACERATE)
	MCFG_SCREEN_SIZE(625 * 2, 520 * 2)
	MCFG_SCREEN_VISIBLE_AREA(VIC6569_STARTVISIBLECOLUMNS, (VIC6569_STARTVISIBLECOLUMNS + VIC6569_VISIBLECOLUMNS - 1) * 2, VIC6569_STARTVISIBLELINES, VIC6569_STARTVISIBLELINES + VIC6569_VISIBLELINES - 1)

	MCFG_DEVICE_REMOVE("vic3")
	MCFG_VIC3_ADD("vic3", c65_vic3_pal_intf)

	/* sound hardware */
	MCFG_SOUND_REPLACE("sid_r", SID8580, 1022727)
	MCFG_SOUND_CONFIG(sidr_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
	MCFG_SOUND_REPLACE("sid_l", SID8580, 1022727)
	MCFG_SOUND_CONFIG(sidl_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)

	/* cia */
	MCFG_DEVICE_REMOVE("cia_0")
	MCFG_DEVICE_REMOVE("cia_1")
	MCFG_MOS6526R1_ADD("cia_0", 3500000, 50, c65_cia0)
	MCFG_MOS6526R1_ADD("cia_1", 3500000, 50, c65_cia1)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/


ROM_START( c65 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "910111", "V0.9.910111" )
	ROMX_LOAD( "910111.bin", 0x20000, 0x20000, CRC(c5d8d32e) SHA1(71c05f098eff29d306b0170e2c1cdeadb1a5f206), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "910523", "V0.9.910523" )
	ROMX_LOAD( "910523.bin", 0x20000, 0x20000, CRC(e8235dd4) SHA1(e453a8e7e5b95de65a70952e9d48012191e1b3e7), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "910626", "V0.9.910626" )
	ROMX_LOAD( "910626.bin", 0x20000, 0x20000, CRC(12527742) SHA1(07c185b3bc58410183422f7ac13a37ddd330881b), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "910828", "V0.9.910828" )
	ROMX_LOAD( "910828.bin", 0x20000, 0x20000, CRC(3ee40b06) SHA1(b63d970727a2b8da72a0a8e234f3c30a20cbcb26), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 4, "911001", "V0.9.911001" )
	ROMX_LOAD( "911001.bin", 0x20000, 0x20000, CRC(0888b50f) SHA1(129b9a2611edaebaa028ac3e3f444927c8b1fc5d), ROM_BIOS(5) )
ROM_END

ROM_START( c64dx )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "910429.bin", 0x20000, 0x20000, CRC(b025805c) SHA1(c3b05665684f74adbe33052a2d10170a1063ee7d) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY                         FULLNAME                                              FLAGS */

COMP( 1991, c65,    0,      0,      c65,    c65, c65_state,    c65,    "Commodore Business Machines",  "Commodore 65 Development System (Prototype, NTSC)", GAME_NOT_WORKING )
COMP( 1991, c64dx,  c65,    0,      c65pal, c65ger, c65_state, c65pal, "Commodore Business Machines",  "Commodore 64DX Development System (Prototype, PAL, German)", GAME_NOT_WORKING )

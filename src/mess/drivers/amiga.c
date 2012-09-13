/***************************************************************************
Commodore Amiga - (c) 1985, Commodore Business Machines Co.

Preliminary driver by:

Ernesto Corvi

Note 1: The 'fast-mem' memory detector in Kickstart 1.2 expects to
see the custom chips at the end of any present fast memory (mapped
from $C00000 onwards). I assume this is a bug, given that the routine
was entirely rewritten for Kickstart 1.3. So the strategy is, for
any machine that can run Kickstart 1.2 (a500,a1000 and a2000), we
map a mirror of the custom chips right after any fast-mem we mapped.
If we didn't map any, then we still put a mirror, but where fast-mem
would commence ($C00000).

***************************************************************************/

/* Core includes */
#include "emu.h"
#include "includes/amiga.h"

/* Components */
#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m6502.h"
#include "machine/6525tpi.h"
#include "machine/6526cia.h"
#include "machine/amigafdc.h"
#include "machine/amigakbd.h"
#include "machine/amigacd.h"
#include "machine/amigacrt.h"
#include "machine/msm6242.h"
#include "machine/ctronics.h"
#include "machine/nvram.h"
#include "sound/cdda.h"

/* Devices */
#include "imagedev/chd_cd.h"
#include "imagedev/cartslot.h"

class cdtv_state : public amiga_state
{
public:
	cdtv_state(const machine_config &mconfig, device_type type, const char *tag)
		: amiga_state(mconfig, type, tag) { }

	DECLARE_MACHINE_START(cdtv);
	DECLARE_MACHINE_RESET(cdtv);
};

static READ8_DEVICE_HANDLER( amiga_cia_0_portA_r );
static READ8_DEVICE_HANDLER( amiga_cia_0_cdtv_portA_r );
static WRITE8_DEVICE_HANDLER( amiga_cia_0_portA_w );

/***************************************************************************
  Battery Backed-Up Clock (MSM6264)
***************************************************************************/

static READ16_HANDLER( amiga_clock_r )
{
	msm6242_device *rtc = space->machine().device<msm6242_device>("rtc");
	return rtc->read(*space,offset / 2);
}


static WRITE16_HANDLER( amiga_clock_w )
{
	msm6242_device *rtc = space->machine().device<msm6242_device>("rtc");
	rtc->write(*space,offset / 2, data);
}


/***************************************************************************
    CENTRONICS PORT
***************************************************************************/

static READ8_DEVICE_HANDLER( amiga_cia_1_porta_r )
{
	centronics_device *centronics = device->machine().device<centronics_device>("centronics");
	UINT8 result = 0;

	/* centronics status is stored in PA0 to PA2 */
	result |= centronics->busy_r() << 0;
	result |= !centronics->pe_r() << 1;
	result |= centronics->vcc_r() << 2;

	/* PA3 to PA7 store the serial line status (not emulated) */

	return result;
}

static const centronics_interface amiga_centronics_config =
{
	DEVCB_DEVICE_LINE("cia_0", mos6526_flag_w),
	DEVCB_NULL,
	DEVCB_NULL
};


/***************************************************************************
  Address maps
***************************************************************************/

static ADDRESS_MAP_START(amiga_mem, AS_PROGRAM, 16, amiga_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x07ffff) AM_MIRROR(0x80000) AM_RAMBANK("bank1") AM_SHARE("chip_ram")
	AM_RANGE(0xbfd000, 0xbfefff) AM_READWRITE_LEGACY(amiga_cia_r, amiga_cia_w)
	AM_RANGE(0xc00000, 0xc7ffff) AM_RAM /* slow-mem */
	AM_RANGE(0xc80000, 0xcfffff) AM_READWRITE_LEGACY(amiga_custom_r, amiga_custom_w)	/* see Note 1 above */
	AM_RANGE(0xdf0000, 0xdfffff) AM_READWRITE_LEGACY(amiga_custom_r, amiga_custom_w) AM_SHARE("custom_regs")	/* Custom Chips */
	AM_RANGE(0xe80000, 0xe8ffff) AM_READWRITE_LEGACY(amiga_autoconfig_r, amiga_autoconfig_w)
	AM_RANGE(0xf80000, 0xffffff) AM_ROM AM_REGION("user1", 0)	/* System ROM - mirror */
ADDRESS_MAP_END

static ADDRESS_MAP_START(keyboard_mem, AS_PROGRAM, 8, amiga_state )
	AM_RANGE(0x0000, 0x003f) AM_RAM /* internal user ram */
	AM_RANGE(0x0040, 0x007f) AM_NOP /* unassigned */
	AM_RANGE(0x0080, 0x0080) AM_NOP /* port a */
	AM_RANGE(0x0081, 0x0081) AM_NOP /* port b */
	AM_RANGE(0x0082, 0x0082) AM_NOP /* port c */
	AM_RANGE(0x0083, 0x0083) AM_NOP /* port d */
	AM_RANGE(0x0084, 0x0085) AM_NOP /* latch */
	AM_RANGE(0x0086, 0x0087) AM_NOP /* count */
	AM_RANGE(0x0088, 0x0088) AM_NOP /* upper latch & transfer latch to counter */
	AM_RANGE(0x0089, 0x0089) AM_NOP /* clear pa0 pos edge detected */
	AM_RANGE(0x008a, 0x008a) AM_NOP /* clear pa1 neg edge detected */
	AM_RANGE(0x008b, 0x008e) AM_NOP /* unassigned */
	AM_RANGE(0x008f, 0x008f) AM_NOP /* control register */
	AM_RANGE(0x0090, 0x07ff) AM_NOP /* unassigned */
	AM_RANGE(0x0800, 0x0fff) AM_ROM AM_REGION("keyboard", 0) /* internal mask rom */
ADDRESS_MAP_END


/*
 * CDTV memory map (source: http://www.l8r.net/technical/cdtv-technical.html)
 *
 * 000000-0FFFFF Chip memory
 * 100000-1FFFFF Space for extra chip memory (Megachip)
 * 200000-9FFFFF Space for AutoConfig memory
 * A00000-BFFFFF CIA chips
 * C00000-C7FFFF Space for slow-fast memory
 * C80000-DBFFFF Space
 * DC0000-DC7FFF Power backed-up real time clock
 * DC8000-DC87FF Non-volatile RAM
 * DC8800-DCFFFF Space in non-volatile RAM decoded area
 * DD0000-DEFFFF Space
 * DF0000-DFFFFF Custom chips
 * E00000-E7FFFF Memory card address space for front panel memory card
 * E80000-E8FFFF AutoConfig configuration space
 * E90000-E9FFFF First AutoConfig device, used by DMAC
 * EA0000-EFFFFF Space for other AutoConfig devices
 * F00000-F3FFFF CDTV ROM
 * F40000-F7FFFF Space in CDTV ROM decoded area
 * F80000-FBFFFF Space in Kickstart ROM decoded area (used by Kickstart 2)
 * FC0000-FFFFFF Kickstart ROM
 *
 */

static ADDRESS_MAP_START(cdtv_mem, AS_PROGRAM, 16, amiga_state )
	AM_RANGE(0x000000, 0x0fffff) AM_RAMBANK("bank1") AM_SHARE("chip_ram")
	AM_RANGE(0xbfd000, 0xbfefff) AM_READWRITE_LEGACY(amiga_cia_r, amiga_cia_w)
	AM_RANGE(0xdc0000, 0xdc003f) AM_READWRITE_LEGACY(amiga_clock_r, amiga_clock_w)
	AM_RANGE(0xdc8000, 0xdc87ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xdf0000, 0xdfffff) AM_READWRITE_LEGACY(amiga_custom_r, amiga_custom_w) AM_SHARE("custom_regs")	/* Custom Chips */
	AM_RANGE(0xe80000, 0xe8ffff) AM_READWRITE_LEGACY(amiga_autoconfig_r, amiga_autoconfig_w)
	AM_RANGE(0xf00000, 0xffffff) AM_ROM AM_REGION("user1", 0)	/* CDTV & System ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START(cdtv_rcmcu_mem, AS_PROGRAM, 8, amiga_state )
	AM_RANGE(0x0000, 0x003f) AM_RAM /* internal user ram */
	AM_RANGE(0x0040, 0x007f) AM_NOP /* unassigned */
	AM_RANGE(0x0080, 0x0080) AM_NOP /* port a */
	AM_RANGE(0x0081, 0x0081) AM_NOP /* port b */
	AM_RANGE(0x0082, 0x0082) AM_NOP /* port c */
	AM_RANGE(0x0083, 0x0083) AM_NOP /* port d */
	AM_RANGE(0x0084, 0x0085) AM_NOP /* latch */
	AM_RANGE(0x0086, 0x0087) AM_NOP /* count */
	AM_RANGE(0x0088, 0x0088) AM_NOP /* upper latch & transfer latch to counter */
	AM_RANGE(0x0089, 0x0089) AM_NOP /* clear pa0 pos edge detected */
	AM_RANGE(0x008a, 0x008a) AM_NOP /* clear pa1 neg edge detected */
	AM_RANGE(0x008b, 0x008e) AM_NOP /* unassigned */
	AM_RANGE(0x008f, 0x008f) AM_NOP /* control register */
	AM_RANGE(0x0090, 0x07ff) AM_NOP /* unassigned */
	AM_RANGE(0x0800, 0x0fff) AM_ROM AM_REGION("rcmcu", 0) /* internal mask rom */
ADDRESS_MAP_END

static ADDRESS_MAP_START(a1000_mem, AS_PROGRAM, 16, amiga_state )
	AM_RANGE(0x000000, 0x03ffff) AM_MIRROR(0xc0000) AM_RAMBANK("bank1") AM_SHARE("chip_ram")
	AM_RANGE(0xbfd000, 0xbfefff) AM_READWRITE_LEGACY(amiga_cia_r, amiga_cia_w)
	AM_RANGE(0xc00000, 0xc3ffff) AM_READWRITE_LEGACY(amiga_custom_r, amiga_custom_w) /* See Note 1 above */
	AM_RANGE(0xdf0000, 0xdfffff) AM_READWRITE_LEGACY(amiga_custom_r, amiga_custom_w) AM_SHARE("custom_regs")	/* Custom Chips */
	AM_RANGE(0xe80000, 0xe8ffff) AM_READWRITE_LEGACY(amiga_autoconfig_r, amiga_autoconfig_w)
	AM_RANGE(0xf80000, 0xfbffff) AM_ROM AM_REGION("user1", 0)	/* Bootstrap ROM */
	AM_RANGE(0xfc0000, 0xffffff) AM_RAMBANK("bank2")	/* Writable Control Store RAM */
ADDRESS_MAP_END


/***************************************************************************
  Inputs
***************************************************************************/

static INPUT_PORTS_START( amiga_common )
	PORT_START("input")
	PORT_CONFNAME( 0x20, 0x00, "Input Port 0 Device")
	PORT_CONFSETTING( 0x00, "Mouse" )
	PORT_CONFSETTING( 0x20, DEF_STR(Joystick) )
	PORT_CONFNAME( 0x10, 0x10, "Input Port 1 Device")
	PORT_CONFSETTING( 0x00, "Mouse" )
	PORT_CONFSETTING( 0x10, DEF_STR(Joystick) )

	PORT_START("CIA0PORTA")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_SPECIAL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("JOY0DAT")
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, amiga_state,amiga_joystick_convert, "P1JOY")
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("JOY1DAT")
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, amiga_state,amiga_joystick_convert, "P2JOY")
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("POTGO")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xaaff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("P2JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("P0MOUSEX")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(1)

	PORT_START("P0MOUSEY")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(1)

	PORT_START("P1MOUSEX")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(2)

	PORT_START("P1MOUSEY")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(2)

	PORT_INCLUDE( amiga_us_keyboard )

INPUT_PORTS_END


static INPUT_PORTS_START( amiga )
	PORT_START("hardware")
	PORT_CONFNAME( 0x08, 0x08, "Battery backed-up RTC")
	PORT_CONFSETTING( 0x00, "Not Installed" )
	PORT_CONFSETTING( 0x08, "Installed" )

	PORT_INCLUDE( amiga_common )
INPUT_PORTS_END


/* TODO: Support for the CDTV remote control */
static INPUT_PORTS_START( cdtv )
	PORT_INCLUDE( amiga_common )
INPUT_PORTS_END

/***************************************************************************
  Machine drivers
***************************************************************************/

MACHINE_START_MEMBER(cdtv_state,cdtv)
{
	MACHINE_START_CALL_LEGACY( amigacd );
}


MACHINE_RESET_MEMBER(cdtv_state,cdtv)
{
	MACHINE_RESET_CALL_MEMBER( amiga );

	/* initialize the cdrom controller */
	MACHINE_RESET_CALL_LEGACY( amigacd );
}

static const mos6526_interface cia_0_ntsc_intf =
{
	DEVCB_DEVICE_LINE("cia_0", amiga_cia_0_irq),							/* irq_func */
	DEVCB_DEVICE_LINE_MEMBER("centronics", centronics_device, strobe_w),	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_HANDLER("cia_0", amiga_cia_0_portA_r),
	DEVCB_DEVICE_HANDLER("cia_0", amiga_cia_0_portA_w),						/* port A */
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER("centronics", centronics_device, write)	/* port B */
};

static const mos6526_interface cia_0_pal_intf =
{
	DEVCB_DEVICE_LINE("cia_0", amiga_cia_0_irq),							/* irq_func */
	DEVCB_DEVICE_LINE_MEMBER("centronics", centronics_device, strobe_w),	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_HANDLER("cia_0", amiga_cia_0_portA_r),
	DEVCB_DEVICE_HANDLER("cia_0", amiga_cia_0_portA_w),						/* port A */
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER("centronics", centronics_device, write)	/* port B */
};

static const mos6526_interface cia_1_intf =
{
	DEVCB_DEVICE_LINE("cia_1", amiga_cia_1_irq),							/* irq_func */
	DEVCB_NULL,												/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_HANDLER("centronics", amiga_cia_1_porta_r),
	DEVCB_NULL,												/* port A */
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER("fdc", amiga_fdc, ciaaprb_w)		/* port B */
};

static const mos6526_interface cia_0_cdtv_intf =
{
	DEVCB_DEVICE_LINE("cia_0", amiga_cia_0_irq),							/* irq_func */
	DEVCB_DEVICE_LINE_MEMBER("centronics", centronics_device, strobe_w),	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_HANDLER("cia_0", amiga_cia_0_cdtv_portA_r),
	DEVCB_DEVICE_HANDLER("cia_0", amiga_cia_0_portA_w),						/* port A */
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER("centronics", centronics_device, write)	/* port B */
};

static const mos6526_interface cia_1_cdtv_intf =
{
	DEVCB_DEVICE_LINE("cia_1", amiga_cia_1_irq),							/* irq_func */
	DEVCB_NULL,	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_HANDLER("centronics", amiga_cia_1_porta_r),
	DEVCB_NULL,												/* port A */
	DEVCB_NULL,
	DEVCB_NULL												/* port B */
};

static const tpi6525_interface cdtv_tpi_intf =
{
	DEVCB_DEVICE_LINE("tpi6525", amigacd_tpi6525_irq),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_HANDLER("tpi6525", amigacd_tpi6525_portb_w),
	DEVCB_DEVICE_HANDLER("tpi6525", amigacd_tpi6525_portc_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static SLOT_INTERFACE_START( amiga_floppies )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( amiga_cartslot )
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin")
	MCFG_CARTSLOT_NOT_MANDATORY
MACHINE_CONFIG_END

static MSM6242_INTERFACE( amiga_rtc_intf )
{
	DEVCB_NULL
};

static MACHINE_CONFIG_START( ntsc, amiga_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, AMIGA_68000_NTSC_CLOCK)
	MCFG_CPU_PROGRAM_MAP(amiga_mem)

	MCFG_CPU_ADD("keyboard", M6502, XTAL_1MHz) /* 1 MHz? */
	MCFG_CPU_PROGRAM_MAP(keyboard_mem)
	MCFG_DEVICE_DISABLE()

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.997)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))

	MCFG_MACHINE_RESET_OVERRIDE(amiga_state, amiga )

    /* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_SIZE(228*4, 262)
	MCFG_SCREEN_VISIBLE_AREA(214, (228*4)-1, 34, 262-1)
	MCFG_SCREEN_UPDATE_STATIC(amiga)

	MCFG_PALETTE_LENGTH(4096)
	MCFG_PALETTE_INIT_OVERRIDE(amiga_state, amiga )

	MCFG_VIDEO_START_OVERRIDE(amiga_state,amiga)

	/* devices */
	MCFG_MSM6242_ADD("rtc",amiga_rtc_intf)
	MCFG_CENTRONICS_PRINTER_ADD("centronics", amiga_centronics_config)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("amiga", AMIGA, 3579545)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)
	MCFG_SOUND_ROUTE(2, "rspeaker", 0.50)
	MCFG_SOUND_ROUTE(3, "lspeaker", 0.50)

	/* cia */
	MCFG_MOS8520_ADD("cia_0", AMIGA_68000_NTSC_CLOCK / 10, 60, cia_0_ntsc_intf)
	MCFG_MOS8520_ADD("cia_1", AMIGA_68000_NTSC_CLOCK, 0, cia_1_intf)

	/* fdc */
	MCFG_AMIGA_FDC_ADD("fdc", AMIGA_68000_NTSC_CLOCK)
	MCFG_FLOPPY_DRIVE_ADD("fd0", amiga_floppies, "35dd", 0, amiga_fdc::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd1", amiga_floppies, 0,      0, amiga_fdc::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd2", amiga_floppies, 0,      0, amiga_fdc::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd3", amiga_floppies, 0,      0, amiga_fdc::floppy_formats)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( a1000n, ntsc )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(a1000_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( a500n, ntsc )
	MCFG_FRAGMENT_ADD(amiga_cartslot)
MACHINE_CONFIG_END

struct cdrom_interface cdtv_cdrom =
{
	"cdrom",
	NULL
};

static MACHINE_CONFIG_DERIVED_CLASS( cdtv, ntsc, cdtv_state)
	MCFG_CPU_REPLACE("maincpu", M68000, CDTV_CLOCK_X1 / 4)
	MCFG_CPU_PROGRAM_MAP(cdtv_mem)

	MCFG_DEVICE_REMOVE("keyboard")

	MCFG_CPU_ADD("rcmcu", M6502, XTAL_1MHz) /* 1 MHz? */
	MCFG_CPU_PROGRAM_MAP(cdtv_rcmcu_mem)
	MCFG_DEVICE_DISABLE()

//  MCFG_CPU_ADD("lcd", LC6554, XTAL_4MHz) /* 4 MHz? */
//  MCFG_CPU_PROGRAM_MAP(cdtv_lcd_mem)

	MCFG_MACHINE_START_OVERRIDE(cdtv_state, cdtv )
	MCFG_MACHINE_RESET_OVERRIDE(cdtv_state, cdtv )

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 1.0 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 1.0 )

	/* cdrom */
	MCFG_CDROM_ADD( "cdrom", cdtv_cdrom)
	MCFG_SOFTWARE_LIST_ADD("cd_list", "cdtv")

	MCFG_TPI6525_ADD("tpi6525", cdtv_tpi_intf)

	/* cia */
	MCFG_DEVICE_REMOVE("cia_0")
	MCFG_DEVICE_REMOVE("cia_1")
	MCFG_MOS8520_ADD("cia_0", CDTV_CLOCK_X1 / 40, 0, cia_0_cdtv_intf)
	MCFG_MOS8520_ADD("cia_1", CDTV_CLOCK_X1 / 4, 0, cia_1_cdtv_intf)

	/* fdc */
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(CDTV_CLOCK_X1 / 4)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( pal, ntsc )

	/* adjust for PAL specs */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK( AMIGA_68000_PAL_CLOCK)

	// Change the FDC clock too?

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(228*4, 312)
	MCFG_SCREEN_VISIBLE_AREA(214, (228*4)-1, 34, 312-1)

	/* cia */
	MCFG_DEVICE_REMOVE("cia_0")
	MCFG_MOS8520_ADD("cia_0", AMIGA_68000_PAL_CLOCK / 10, 50, cia_0_pal_intf)

	/* fdc */
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(AMIGA_68000_PAL_CLOCK)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( a500p, pal )
	MCFG_FRAGMENT_ADD(amiga_cartslot)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( a1000p, pal )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(a1000_mem)
MACHINE_CONFIG_END

/***************************************************************************

  Amiga specific stuff

***************************************************************************/


static READ8_DEVICE_HANDLER( amiga_cia_0_portA_r )
{
	UINT8 ret = device->machine().root_device().ioport("CIA0PORTA")->read() & 0xc0;	/* Gameport 1 and 0 buttons */
	ret |= device->machine().device<amiga_fdc>("fdc")->ciaapra_r();
	return ret;
}


static READ8_DEVICE_HANDLER( amiga_cia_0_cdtv_portA_r )
{
	return device->machine().root_device().ioport("CIA0PORTA")->read() & 0xc0;	/* Gameport 1 and 0 buttons */
}


static WRITE8_DEVICE_HANDLER( amiga_cia_0_portA_w )
{
	amiga_state *state = device->machine().driver_data<amiga_state>();
	/* switch banks as appropriate */
	state->membank("bank1")->set_entry(data & 1);

	/* swap the write handlers between ROM and bank 1 based on the bit */
	if ((data & 1) == 0) {
		UINT32 mirror_mask = state->m_chip_ram.bytes();

		while( (mirror_mask<<1) < 0x100000 ) {
			mirror_mask |= ( mirror_mask << 1 );
		}

		/* overlay disabled, map RAM on 0x000000 */
		device->machine().device("maincpu")->memory().space(AS_PROGRAM)->install_write_bank(0x000000, state->m_chip_ram.bytes() - 1, 0, mirror_mask, "bank1");

		/* if there is a cart region, check for cart overlay */
		if (device->machine().root_device().memregion("user2")->base() != NULL)
			amiga_cart_check_overlay(device->machine());
	}
	else
		/* overlay enabled, map Amiga system ROM on 0x000000 */
		device->machine().device("maincpu")->memory().space(AS_PROGRAM)->unmap_write(0x000000, state->m_chip_ram.bytes() - 1);

	set_led_status( device->machine(), 0, ( data & 2 ) ? 0 : 1 ); /* bit 2 = Power Led on Amiga */
	output_set_value("power_led", ( data & 2 ) ? 0 : 1);
}

static UINT16 amiga_read_joy0dat(running_machine &machine)
{
	if ( machine.root_device().ioport("input")->read() & 0x20 ) {
		/* Joystick */
		return machine.root_device().ioport("JOY0DAT")->read_safe(0xffff);
	} else {
		/* Mouse */
		int input;
		input  = ( machine.root_device().ioport("P0MOUSEX")->read() & 0xff );
		input |= ( machine.root_device().ioport("P0MOUSEY")->read() & 0xff ) << 8;
		return input;
	}
}

static UINT16 amiga_read_joy1dat(running_machine &machine)
{
	if ( machine.root_device().ioport("input")->read() & 0x10 ) {
		/* Joystick */
		return machine.root_device().ioport("JOY1DAT")->read_safe(0xffff);
	} else {
		/* Mouse */
		int input;
		input  = ( machine.root_device().ioport("P1MOUSEX")->read() & 0xff );
		input |= ( machine.root_device().ioport("P1MOUSEY")->read() & 0xff ) << 8;
		return input;
	}
}

static void amiga_reset(running_machine &machine)
{
	if (machine.root_device().ioport("hardware")->read() & 0x08)
	{
		/* Install RTC */
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xdc0000, 0xdc003f, FUNC(amiga_clock_r), FUNC(amiga_clock_w));
	}
	else
	{
		/* No RTC support */
		machine.device("maincpu")->memory().space(AS_PROGRAM)->unmap_readwrite(0xdc0000, 0xdc003f);
	}
}

DRIVER_INIT_MEMBER(amiga_state,amiga)
{
	static const amiga_machine_interface amiga_intf =
	{
		ANGUS_CHIP_RAM_MASK,
		amiga_read_joy0dat,	amiga_read_joy1dat,  /* joy0dat_r & joy1dat_r */
		NULL,                                    /* potgo_w */
		NULL,                                    /* serdat_w */
		NULL,                                    /* scanline0_callback */
		amiga_reset,                             /* reset_callback */
		amiga_cart_nmi,                          /* nmi_callback */
		0                                        /* flags */
	};

	amiga_machine_config(machine(), &amiga_intf);

	/* set up memory */
	membank("bank1")->configure_entry(0, m_chip_ram);
	membank("bank1")->configure_entry(1, machine().root_device().memregion("user1")->base());

	/* initialize cartridge (if present) */
	amiga_cart_init(machine());

	/* initialize keyboard */
	amigakbd_init(machine());
}

#ifdef UNUSED_FUNCTION
DRIVER_INIT_MEMBER(apollo_state,amiga_ecs)
{
	static const amiga_machine_interface amiga_intf =
	{
		ECS_CHIP_RAM_MASK,
		amiga_cia_0_portA_r, NULL,               /* CIA0 port A & B read */
		amiga_cia_0_portA_w, NULL,               /* CIA0 port A & B write */
		NULL, NULL,                              /* CIA1 port A & B read */
		NULL, amiga_fdc_control_w,               /* CIA1 port A & B write */
		amiga_read_joy0dat,	amiga_read_joy1dat,  /* joy0dat_r & joy1dat_r */
		NULL,                                    /* potgo_w */
		NULL,                                    /* serdat_w */
		NULL,                                    /* scanline0_callback */
		amiga_reset,                             /* reset_callback */
		amiga_cart_nmi,                          /* nmi_callback */
		0                                        /* flags */
	};

	amiga_machine_config(machine(), &amiga_intf);

	/* set up memory */
	1.root_device().membank(0)->configure_entries(1, m_chip_ram, 0);
	1.root_device().membank(1)->configure_entries(1, machine().root_device().memregion("user1")->base(), 0);

	/* initialize Action Replay (if present) */
	amiga_cart_init(machine());

	/* initialize keyboard */
	amigakbd_init(machine());
}
#endif

DRIVER_INIT_MEMBER(amiga_state,cdtv)
{
	static const amiga_machine_interface amiga_intf =
	{
		ECS_CHIP_RAM_MASK,
		amiga_read_joy0dat,	amiga_read_joy1dat,  /* joy0dat_r & joy1dat_r */
		NULL,                                    /* potgo_w */
		NULL,                                    /* serdat_w */
		NULL,                                    /* scanline0_callback */
		NULL,                                    /* reset_callback */
		NULL,                                    /* nmi_callback */
		0                                        /* flags */
	};

	amiga_machine_config(machine(), &amiga_intf);

	/* set up memory */
	membank("bank1")->configure_entry(0, m_chip_ram);
	membank("bank1")->configure_entry(1, machine().root_device().memregion("user1")->base());

	/* initialize keyboard - in cdtv we can use a standard Amiga keyboard*/
	amigakbd_init(machine());
}


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( a500n )
	ROM_REGION16_BE(0x080000, "user1", 0)
	ROM_DEFAULT_BIOS("kick13")
	ROM_SYSTEM_BIOS(0, "kick12",  "Kickstart 1.2 (33.180)")
	ROMX_LOAD("315093-01.u6", 0x000000, 0x040000, CRC(a6ce1636) SHA1(11f9e62cf299f72184835b7b2a70a16333fc0d88), ROM_GROUPWORD | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "kick13",  "Kickstart 1.3 (34.5)")
	ROMX_LOAD("315093-02.u6", 0x000000, 0x040000, CRC(c4f0f55f) SHA1(891e9a547772fe0c6c19b610baf8bc4ea7fcb785), ROM_GROUPWORD | ROM_BIOS(2))
	ROM_COPY("user1", 0x000000, 0x040000, 0x040000)
	ROM_SYSTEM_BIOS(2, "kick204", "Kickstart 2.04 (37.175)")
	ROMX_LOAD("390979-01.u6", 0x000000, 0x080000, CRC(c3bdb240) SHA1(c5839f5cb98a7a8947065c3ed2f14f5f42e334a1), ROM_GROUPWORD | ROM_BIOS(3))	/* identical to 363968.01 */
	ROM_SYSTEM_BIOS(3, "kick31",  "Kickstart 3.1 (40.63)")
	ROMX_LOAD("kick40063.u6", 0x000000, 0x080000, CRC(fc24ae0d) SHA1(3b7f1493b27e212830f989f26ca76c02049f09ca), ROM_GROUPWORD | ROM_BIOS(4))	/* part number? */

	/* action replay cartridge */
	ROM_REGION16_BE(0x080000, "user2", ROMREGION_ERASEFF )
	ROM_CART_LOAD("cart", 0x0000, 0x080000, ROM_NOMIRROR | ROM_OPTIONAL)

	/* keyboard controller, mos 6500/1 mcu */
	ROM_REGION(0x800, "keyboard", 0)
	ROM_LOAD("328191-02.ic1", 0x000, 0x800, NO_DUMP)
ROM_END

#define rom_a500p    rom_a500n


ROM_START( a1000n )
	ROM_REGION16_BE(0x080000, "user1", 0)
	ROM_LOAD16_BYTE("252179-01.u5n", 0x000000, 0x001000, CRC(42553bc4) SHA1(8855a97f7a44e3f62d1c88d938fee1f4c606af5b))
	ROM_LOAD16_BYTE("252180-01.u5p", 0x000001, 0x001000, CRC(8e5b9a37) SHA1(d10f1564b99f5ffe108fa042362e877f569de2c3))

	/* keyboard controller, mos 6500/1 mcu */
	ROM_REGION(0x800, "keyboard", 0)
	ROM_LOAD("328191-01.bin", 0x000, 0x800, NO_DUMP)
ROM_END

#define rom_a1000p    rom_a1000n


ROM_START( cdtv )
	ROM_REGION16_BE(0x100000, "user1", 0)
	ROM_LOAD16_BYTE("391008-01.u34", 0x000000, 0x020000, CRC(791cb14b) SHA1(277a1778924496353ffe56be68063d2a334360e4))
	ROM_LOAD16_BYTE("391009-01.u35", 0x000001, 0x020000, CRC(accbbc2e) SHA1(41b06d1679c6e6933c3378b7626025f7641ebc5c))
	ROM_COPY("user1", 0x000000, 0x040000, 0x040000)
	ROMX_LOAD(      "315093-02.u13", 0x080000, 0x040000, CRC(c4f0f55f) SHA1(891e9a547772fe0c6c19b610baf8bc4ea7fcb785), ROM_GROUPWORD)
	ROM_COPY("user1", 0x080000, 0x0c0000, 0x040000)

	/* remote control input converter, mos 6500/1 mcu */
	ROM_REGION(0x800, "rcmcu", 0)
	ROM_LOAD("252609-02.u75", 0x000, 0x800, NO_DUMP)

	/* lcd controller, sanyo lc6554h */
	ROM_REGION(0x1000, "lcd", 0)
	ROM_LOAD("252608-01.u62", 0x000, 0x1000, NO_DUMP)
ROM_END

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY                             FULLNAME                 FLAGS */
COMP( 1985, a1000n, 0,      0,      a1000n, amiga, amiga_state,  amiga,  "Commodore Business Machines",  "Amiga 1000 (NTSC)",     GAME_IMPERFECT_GRAPHICS )
COMP( 1985, a1000p, a1000n, 0,      a1000p, amiga, amiga_state,  amiga,  "Commodore Business Machines",  "Amiga 1000 (PAL)",      GAME_IMPERFECT_GRAPHICS )
COMP( 1987, a500n,  0,      0,      a500n,  amiga, amiga_state,  amiga,  "Commodore Business Machines",  "Amiga 500 (NTSC, OCS)", GAME_IMPERFECT_GRAPHICS )
COMP( 1987, a500p,  a500n,  0,      a500p,  amiga, amiga_state,  amiga,  "Commodore Business Machines",  "Amiga 500 (PAL, OCS)",  GAME_IMPERFECT_GRAPHICS )
COMP( 1991, cdtv,   0,      0,      cdtv,   cdtv, amiga_state,   cdtv,   "Commodore Business Machines",  "CDTV (NTSC)",           GAME_IMPERFECT_GRAPHICS )

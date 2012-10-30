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
/*
    Amiga 1200

    Preliminary MAME driver by Mariusz Wojcieszek
    CD-ROM controller by Ernesto Corvi
    Borrowed by incog for MESS

    2009-05 Fabio Priuli:
    Amiga 1200 support is just sketched (I basically took cd32 and removed Akiko). I connected
    the floppy drive in the same way as in amiga.c but it seems to be not working, since I
    tried to load WB3.1 with no success. However, this problem may be due to anything: maybe
    the floppy code must be connected elsewhere, or the .adf image is broken, or I made some
    stupid mistake in the CIA interfaces.
    Later, it could be wise to re-factor this source and merge the non-AGA code with
    mess/drivers/amiga.c
*/










/* Core includes */
#include "emu.h"
#include "includes/amiga.h"
#include "includes/cd32.h"

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
#include "machine/i2cmem.h"

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



class a1200_state : public amiga_state
{
public:
	a1200_state(const machine_config &mconfig, device_type type, const char *tag)
		: amiga_state(mconfig, type, tag) { }

	UINT16 m_potgo_value;
	int m_cd32_shifter[2];
	int m_oldstate[2];
	DECLARE_WRITE32_MEMBER(aga_overlay_w);
	DECLARE_DRIVER_INIT(a1200);
	DECLARE_WRITE8_MEMBER(ami1200_cia_0_porta_w);
	DECLARE_READ8_MEMBER(ami1200_cia_0_portb_r);
	DECLARE_WRITE8_MEMBER(ami1200_cia_0_portb_w);
	DECLARE_READ8_MEMBER(a1200_cia_0_portA_r);
};



#define A1200PAL_XTAL_X1  XTAL_28_37516MHz
#define A1200PAL_XTAL_X2  XTAL_4_433619MHz




static DECLARE_READ8_DEVICE_HANDLER( amiga_cia_0_portA_r );
static DECLARE_READ8_DEVICE_HANDLER( amiga_cia_0_cdtv_portA_r );
static DECLARE_WRITE8_DEVICE_HANDLER( amiga_cia_0_portA_w );

/***************************************************************************
  Battery Backed-Up Clock (MSM6264)
***************************************************************************/

static READ16_HANDLER( amiga_clock_r )
{
	msm6242_device *rtc = space.machine().device<msm6242_device>("rtc");
	return rtc->read(space,offset / 2);
}


static WRITE16_HANDLER( amiga_clock_w )
{
	msm6242_device *rtc = space.machine().device<msm6242_device>("rtc");
	rtc->write(space,offset / 2, data);
}


/***************************************************************************
    CENTRONICS PORT
***************************************************************************/

static READ8_DEVICE_HANDLER( amiga_cia_1_porta_r )
{
	centronics_device *centronics = space.machine().device<centronics_device>("centronics");
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

static ADDRESS_MAP_START( a1200_map, AS_PROGRAM, 32, a1200_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_RAMBANK("bank1") AM_SHARE("chip_ram")
	AM_RANGE(0xbfa000, 0xbfa003) AM_WRITE(aga_overlay_w)
	AM_RANGE(0xbfd000, 0xbfefff) AM_READWRITE16_LEGACY(amiga_cia_r, amiga_cia_w, 0xffffffff)
	AM_RANGE(0xc00000, 0xdfffff) AM_READWRITE16_LEGACY(amiga_custom_r, amiga_custom_w, 0xffffffff) AM_SHARE("custom_regs")
	AM_RANGE(0xe80000, 0xe8ffff) AM_READWRITE16_LEGACY(amiga_autoconfig_r, amiga_autoconfig_w, 0xffffffff)
	AM_RANGE(0xf80000, 0xffffff) AM_ROM AM_REGION("user1", 0)	/* Kickstart */
ADDRESS_MAP_END

static ADDRESS_MAP_START( amiga_mem32, AS_PROGRAM, 32, a1200_state )
	ADDRESS_MAP_UNMAP_HIGH
//  ADDRESS_MAP_GLOBAL_MASK(0xffffff) // not sure
	AM_RANGE(0x000000, 0x1fffff) AM_RAMBANK("bank1") AM_SHARE("chip_ram")
	AM_RANGE(0xbfd000, 0xbfefff) AM_READWRITE16_LEGACY(amiga_cia_r, amiga_cia_w, 0xffffffff)
	AM_RANGE(0xc00000, 0xdfffff) AM_READWRITE16_LEGACY(amiga_custom_r, amiga_custom_w, 0xffffffff) AM_SHARE("custom_regs")
	AM_RANGE(0xe80000, 0xe8ffff) AM_READWRITE16_LEGACY(amiga_autoconfig_r, amiga_autoconfig_w, 0xffffffff)
	AM_RANGE(0xf80000, 0xffffff) AM_ROM AM_REGION("user1", 0)	/* Kickstart */
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


static void handle_cd32_joystick_cia(a1200_state *state, UINT8 pra, UINT8 dra);

WRITE32_MEMBER(a1200_state::aga_overlay_w)
{
	if (ACCESSING_BITS_16_23)
	{
		data = (data >> 16) & 1;

		/* switch banks as appropriate */
		membank("bank1")->set_entry(data & 1);

		/* swap the write handlers between ROM and bank 1 based on the bit */
		if ((data & 1) == 0)
			/* overlay disabled, map RAM on 0x000000 */
			space.install_write_bank(0x000000, 0x1fffff, "bank1");
		else
			/* overlay enabled, map Amiga system ROM on 0x000000 */
			space.unmap_write(0x000000, 0x1fffff);
	}
}

/*************************************
 *
 *  CIA-A port A access:
 *
 *  PA7 = game port 1, pin 6 (fire)
 *  PA6 = game port 0, pin 6 (fire)
 *  PA5 = /RDY (disk ready)
 *  PA4 = /TK0 (disk track 00)
 *  PA3 = /WPRO (disk write protect)
 *  PA2 = /CHNG (disk change)
 *  PA1 = /LED (LED, 0=bright / audio filter control)
 *  PA0 = MUTE
 *
 *************************************/

WRITE8_MEMBER(a1200_state::ami1200_cia_0_porta_w)
{
	device_t *device = machine().device("cia_0");

	/* bit 2 = Power Led on Amiga */
	set_led_status(machine(), 0, !BIT(data, 1));

	handle_cd32_joystick_cia(this, data, mos6526_r(device, space, 2));
}

/*************************************
 *
 *  CIA-A port B access:
 *
 *  PB7 = parallel data 7
 *  PB6 = parallel data 6
 *  PB5 = parallel data 5
 *  PB4 = parallel data 4
 *  PB3 = parallel data 3
 *  PB2 = parallel data 2
 *  PB1 = parallel data 1
 *  PB0 = parallel data 0
 *
 *************************************/




//int cd32_input_port_val = 0;
//int cd32_input_select = 0;
#if 0
static void cd32_potgo_w(running_machine &machine, UINT16 data)
{
	a1200_state *state = machine.driver_data<a1200_state>();
	int i;

	state->m_potgo_value = state->m_potgo_value & 0x5500;
	state->m_potgo_value |= data & 0xaa00;

	for (i = 0; i < 8; i += 2)
	{
		UINT16 dir = 0x0200 << i;
		if (data & dir)
		{
			UINT16 d = 0x0100 << i;
			state->m_potgo_value &= ~d;
			state->m_potgo_value |= data & d;
		}
	}
	for (i = 0; i < 2; i++)
	{
		UINT16 p5dir = 0x0200 << (i * 4); /* output enable P5 */
		UINT16 p5dat = 0x0100 << (i * 4); /* data P5 */
		if ((state->m_potgo_value & p5dir) && (state->m_potgo_value & p5dat))
			state->m_cd32_shifter[i] = 8;
	}
}
#endif

static void handle_cd32_joystick_cia(a1200_state *state, UINT8 pra, UINT8 dra)
{
	int i;

	for (i = 0; i < 2; i++)
	{
		UINT8 but = 0x40 << i;
		UINT16 p5dir = 0x0200 << (i * 4); /* output enable P5 */
		UINT16 p5dat = 0x0100 << (i * 4); /* data P5 */
		if (!(state->m_potgo_value & p5dir) || !(state->m_potgo_value & p5dat))
		{
			if ((dra & but) && (pra & but) != state->m_oldstate[i])
			{
				if (!(pra & but))
				{
					state->m_cd32_shifter[i]--;
					if (state->m_cd32_shifter[i] < 0)
						state->m_cd32_shifter[i] = 0;
				}
			}
		}
		state->m_oldstate[i] = pra & but;
	}
}



READ8_MEMBER(a1200_state::ami1200_cia_0_portb_r)
{
	/* parallel port */
	logerror("%s:CIA0_portb_r\n", machine().describe_context());
	return 0xff;
}

WRITE8_MEMBER(a1200_state::ami1200_cia_0_portb_w)
{
	/* parallel port */
	logerror("%s:CIA0_portb_w(%02x)\n", machine().describe_context(), data);
}



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

static const legacy_mos6526_interface cia_0_ntsc_intf =
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

static const legacy_mos6526_interface cia_0_pal_intf =
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

static const legacy_mos6526_interface cia_1_intf =
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

static const legacy_mos6526_interface cia_0_cdtv_intf =
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

static const legacy_mos6526_interface cia_1_cdtv_intf =
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

READ8_MEMBER(a1200_state::a1200_cia_0_portA_r)
{
	UINT8 ret = machine().root_device().ioport("CIA0PORTA")->read() & 0xc0;	/* Gameport 1 and 0 buttons */
	ret |= machine().device<amiga_fdc>("fdc")->ciaapra_r();
	return ret;
}


static const legacy_mos6526_interface a1200_cia_0_intf =
{
	DEVCB_DEVICE_LINE("cia_0", amiga_cia_0_irq),									/* irq_func */
	DEVCB_NULL,	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(a1200_state,a1200_cia_0_portA_r),
	DEVCB_DRIVER_MEMBER(a1200_state,ami1200_cia_0_porta_w),		/* port A */
	DEVCB_DRIVER_MEMBER(a1200_state,ami1200_cia_0_portb_r),
	DEVCB_DRIVER_MEMBER(a1200_state,ami1200_cia_0_portb_w)		/* port B */
};

static const legacy_mos6526_interface a1200_cia_1_intf =
{
	DEVCB_DEVICE_LINE("cia_1", amiga_cia_1_irq),									/* irq_func */
	DEVCB_NULL,	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,									/* port A */
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER("fdc", amiga_fdc, ciaaprb_w)			/* port B */
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
	MCFG_SCREEN_UPDATE_DRIVER(amiga_state, screen_update_amiga)

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
	MCFG_LEGACY_MOS8520_ADD("cia_0", AMIGA_68000_NTSC_CLOCK / 10, 60, cia_0_ntsc_intf)
	MCFG_LEGACY_MOS8520_ADD("cia_1", AMIGA_68000_NTSC_CLOCK, 0, cia_1_intf)

	/* fdc */
	MCFG_AMIGA_FDC_ADD("fdc", AMIGA_68000_NTSC_CLOCK)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", amiga_floppies, "35dd", 0, amiga_fdc::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", amiga_floppies, 0,      0, amiga_fdc::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:2", amiga_floppies, 0,      0, amiga_fdc::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:3", amiga_floppies, 0,      0, amiga_fdc::floppy_formats)

	MCFG_AMIGA_KEYBOARD_ADD("kbd")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( a1000ntsc, ntsc )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(a1000_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( a500ntsc, ntsc )
	MCFG_FRAGMENT_ADD(amiga_cartslot)
	MCFG_SOFTWARE_LIST_ADD("flop_common","amiga_flop")
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
	MCFG_LEGACY_MOS8520_ADD("cia_0", CDTV_CLOCK_X1 / 40, 0, cia_0_cdtv_intf)
	MCFG_LEGACY_MOS8520_ADD("cia_1", CDTV_CLOCK_X1 / 4, 0, cia_1_cdtv_intf)

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
	MCFG_LEGACY_MOS8520_ADD("cia_0", AMIGA_68000_PAL_CLOCK / 10, 50, cia_0_pal_intf)

	/* fdc */
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(AMIGA_68000_PAL_CLOCK)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( a500p, pal )
	MCFG_FRAGMENT_ADD(amiga_cartslot)
	MCFG_SOFTWARE_LIST_ADD("flop_common","amiga_flop")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( a1000p, pal )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(a1000_mem)
MACHINE_CONFIG_END

/* Machine definitions with Software List associations for system software */

/* Amiga 1000 */

static MACHINE_CONFIG_DERIVED( a1000, a1000p )
	MCFG_SOFTWARE_LIST_ADD("flop_list","amiga1000_flop")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( a1000n, a1000ntsc )
	MCFG_SOFTWARE_LIST_ADD("flop_list","amiga1000_flop")
MACHINE_CONFIG_END

/* Amiga 500 */

static MACHINE_CONFIG_DERIVED( a500, a500p )
	MCFG_SOFTWARE_LIST_ADD("flop_list","amiga500_flop")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( a500n, a500ntsc )
	MCFG_SOFTWARE_LIST_ADD("flop_list","amiga500_flop")
MACHINE_CONFIG_END

/* Amiga 500 Plus */

static MACHINE_CONFIG_DERIVED( a500pls, a500p )
	MCFG_SOFTWARE_LIST_ADD("flop_list","amiga500plus_flop")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( a500plsn, a500ntsc )
	MCFG_SOFTWARE_LIST_ADD("flop_list","amiga500plus_flop")
MACHINE_CONFIG_END

/* Amiga 600 */

static MACHINE_CONFIG_DERIVED( a600, a500p )
	MCFG_SOFTWARE_LIST_ADD("flop_list","amiga600_flop")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( a600n, a500ntsc )
	MCFG_SOFTWARE_LIST_ADD("flop_list","amiga600_flop")
MACHINE_CONFIG_END



static MACHINE_CONFIG_START( a1200n, a1200_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68EC020, AMIGA_68EC020_NTSC_CLOCK) /* 14.3 Mhz */
	MCFG_CPU_PROGRAM_MAP(a1200_map)

	MCFG_CPU_ADD("keyboard", M6502, XTAL_1MHz) /* 1 MHz? */
	MCFG_CPU_PROGRAM_MAP(keyboard_mem)
	MCFG_DEVICE_DISABLE()

	MCFG_MACHINE_RESET_OVERRIDE(amiga_state, amiga )

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.997)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512*2, 312)
	MCFG_SCREEN_VISIBLE_AREA((129-8-8)*2, (449+8-1+8)*2, 44-8, 300+8-1)
	MCFG_SCREEN_UPDATE_DRIVER(a1200_state, screen_update_amiga_aga)

	MCFG_VIDEO_START_OVERRIDE(a1200_state,amiga_aga)


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
	MCFG_LEGACY_MOS8520_ADD("cia_0", AMIGA_68EC020_NTSC_CLOCK /2 / 10, 60, cia_0_ntsc_intf)
	MCFG_LEGACY_MOS8520_ADD("cia_1", AMIGA_68EC020_NTSC_CLOCK /2, 0, cia_1_intf)

	/* fdc */
	MCFG_AMIGA_FDC_ADD("fdc", AMIGA_68EC020_NTSC_CLOCK / 2)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", amiga_floppies, "35dd", 0, amiga_fdc::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", amiga_floppies, 0,      0, amiga_fdc::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:2", amiga_floppies, 0,      0, amiga_fdc::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:3", amiga_floppies, 0,      0, amiga_fdc::floppy_formats)

	MCFG_AMIGA_KEYBOARD_ADD("kbd")

	MCFG_SOFTWARE_LIST_ADD("flop_common","amiga_flop")
	MCFG_SOFTWARE_LIST_ADD("flop_list","amiga1200_flop")
	MCFG_SOFTWARE_LIST_ADD("flop_aga","amigaaga_flop")


MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( a1200, a1200n )

	/* adjust for PAL specs */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(AMIGA_68EC020_PAL_CLOCK) /* 14.18758 MHz */

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(50)

	/* sound hardware */
	MCFG_SOUND_MODIFY("amiga")
	MCFG_SOUND_CLOCK(AMIGA_68EC020_PAL_CLOCK/4) /* 3.546895 MHz */

	/* cia */
	MCFG_DEVICE_MODIFY("cia_0")
	MCFG_DEVICE_CLOCK(AMIGA_68EC020_PAL_CLOCK/10/2)
	MCFG_DEVICE_MODIFY("cia_1")
	MCFG_DEVICE_CLOCK(AMIGA_68EC020_PAL_CLOCK/2)

	/* fdc */
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(AMIGA_68EC020_PAL_CLOCK/2)
MACHINE_CONFIG_END


// 16mhz and 25mhz versions were available
// 68030 / 68040 options available
#define A3000_XTAL	XTAL_25MHz

/* ToDo: proper A3000 clocks */

static MACHINE_CONFIG_START( a3000n, amiga_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68030, A3000_XTAL)
	MCFG_CPU_PROGRAM_MAP(amiga_mem32)

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
	MCFG_SCREEN_UPDATE_DRIVER(amiga_state, screen_update_amiga)

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
	MCFG_LEGACY_MOS8520_ADD("cia_0", AMIGA_68000_NTSC_CLOCK / 10, 60, cia_0_ntsc_intf)
	MCFG_LEGACY_MOS8520_ADD("cia_1", AMIGA_68000_NTSC_CLOCK, 0, cia_1_intf)

	/* fdc */
	MCFG_AMIGA_FDC_ADD("fdc", AMIGA_68000_NTSC_CLOCK)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", amiga_floppies, "35dd", 0, amiga_fdc::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", amiga_floppies, 0,      0, amiga_fdc::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:2", amiga_floppies, 0,      0, amiga_fdc::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:3", amiga_floppies, 0,      0, amiga_fdc::floppy_formats)

	MCFG_AMIGA_KEYBOARD_ADD("kbd")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( a3000, a3000n )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(228*4, 312)
	MCFG_SCREEN_VISIBLE_AREA(214, (228*4)-1, 34, 312-1)

	/* cia */
	MCFG_DEVICE_REMOVE("cia_0")
	MCFG_LEGACY_MOS8520_ADD("cia_0", AMIGA_68000_PAL_CLOCK / 10, 50, cia_0_pal_intf)

	/* fdc */
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(AMIGA_68000_PAL_CLOCK)
MACHINE_CONFIG_END

/***************************************************************************

  Amiga specific stuff

***************************************************************************/


static READ8_DEVICE_HANDLER( amiga_cia_0_portA_r )
{
	UINT8 ret = space.machine().root_device().ioport("CIA0PORTA")->read() & 0xc0;	/* Gameport 1 and 0 buttons */
	ret |= space.machine().device<amiga_fdc>("fdc")->ciaapra_r();
	return ret;
}


static READ8_DEVICE_HANDLER( amiga_cia_0_cdtv_portA_r )
{
	return space.machine().root_device().ioport("CIA0PORTA")->read() & 0xc0;	/* Gameport 1 and 0 buttons */
}


static WRITE8_DEVICE_HANDLER( amiga_cia_0_portA_w )
{
	amiga_state *state = space.machine().driver_data<amiga_state>();
	/* switch banks as appropriate */
	state->membank("bank1")->set_entry(data & 1);

	/* swap the write handlers between ROM and bank 1 based on the bit */
	if ((data & 1) == 0) {
		UINT32 mirror_mask = state->m_chip_ram.bytes();

		while( (mirror_mask<<1) < 0x100000 ) {
			mirror_mask |= ( mirror_mask << 1 );
		}

		/* overlay disabled, map RAM on 0x000000 */
		space.machine().device("maincpu")->memory().space(AS_PROGRAM).install_write_bank(0x000000, state->m_chip_ram.bytes() - 1, 0, mirror_mask, "bank1");

		/* if there is a cart region, check for cart overlay */
		if (space.machine().root_device().memregion("user2")->base() != NULL)
			amiga_cart_check_overlay(space.machine());
	}
	else
		/* overlay enabled, map Amiga system ROM on 0x000000 */
		space.machine().device("maincpu")->memory().space(AS_PROGRAM).unmap_write(0x000000, state->m_chip_ram.bytes() - 1);

	set_led_status( space.machine(), 0, ( data & 2 ) ? 0 : 1 ); /* bit 2 = Power Led on Amiga */
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
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_legacy_readwrite_handler(0xdc0000, 0xdc003f, FUNC(amiga_clock_r), FUNC(amiga_clock_w));
	}
	else
	{
		/* No RTC support */
		machine.device("maincpu")->memory().space(AS_PROGRAM).unmap_readwrite(0xdc0000, 0xdc003f);
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
}



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
}

DRIVER_INIT_MEMBER(a1200_state,a1200)
{
	static const amiga_machine_interface cd32_intf =
	{
		AGA_CHIP_RAM_MASK,
		amiga_read_joy0dat,	amiga_read_joy1dat,  /* joy0dat_r & joy1dat_r */
		NULL,				/* potgo_w */
		NULL,				/* serdat_w */
		NULL,				/* scanline0_callback */
		NULL,				/* reset_callback */
		NULL,				/* nmi_callback */
		FLAGS_AGA_CHIPSET | FLAGS_IS_32BIT	/* flags */
	};

	/* configure our Amiga setup */
	amiga_machine_config(machine(), &cd32_intf);

	/* set up memory */
	membank("bank1")->configure_entry(0, m_chip_ram);
	membank("bank1")->configure_entry(1, machine().root_device().memregion("user1")->base());
}

DRIVER_INIT_MEMBER(amiga_state,a3000)
{
	static const amiga_machine_interface a3000_intf =
	{
		ECS_CHIP_RAM_MASK,
		amiga_read_joy0dat,	amiga_read_joy1dat,  /* joy0dat_r & joy1dat_r */
		NULL,				/* potgo_w */
		NULL,				/* serdat_w */
		NULL,				/* scanline0_callback */
		NULL,				/* reset_callback */
		NULL,				/* nmi_callback */
		FLAGS_IS_32BIT,	/* flags */
	};

	/* configure our Amiga setup */
	amiga_machine_config(machine(), &a3000_intf);

	/* set up memory */
	membank("bank1")->configure_entry(0, m_chip_ram);
	membank("bank1")->configure_entry(1, machine().root_device().memregion("user1")->base());
}



/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( a1000 )
	ROM_REGION16_BE(0x080000, "user1", 0)
	ROM_LOAD16_BYTE("252179-01.u5n", 0x000000, 0x001000, CRC(42553bc4) SHA1(8855a97f7a44e3f62d1c88d938fee1f4c606af5b))
	ROM_LOAD16_BYTE("252180-01.u5p", 0x000001, 0x001000, CRC(8e5b9a37) SHA1(d10f1564b99f5ffe108fa042362e877f569de2c3))

	/* Kickstart needed to be loaded from floppy */

	/* keyboard controller, mos 6500/1 mcu */
	ROM_REGION(0x800, "keyboard", 0)
	ROM_LOAD("328191-01.bin", 0x000, 0x800, NO_DUMP)
ROM_END

#define rom_a1000n    rom_a1000



ROM_START( a500 )
	ROM_REGION16_BE(0x080000, "user1", 0)
	ROM_DEFAULT_BIOS("kick13")

	/* early models had Kickstart 1.2 */
	ROM_SYSTEM_BIOS(0, "kick12",  "Kickstart 1.2 (33.180)")
	ROMX_LOAD("315093-01.u6", 0x000000, 0x040000, CRC(a6ce1636) SHA1(11f9e62cf299f72184835b7b2a70a16333fc0d88), ROM_GROUPWORD | ROM_BIOS(1))
	/* most models had Kickstart 1.3 */
	ROM_SYSTEM_BIOS(1, "kick13",  "Kickstart 1.3 (34.5)")
	ROMX_LOAD("315093-02.u6", 0x000000, 0x040000, CRC(c4f0f55f) SHA1(891e9a547772fe0c6c19b610baf8bc4ea7fcb785), ROM_GROUPWORD | ROM_BIOS(2))
	ROM_COPY("user1", 0x000000, 0x040000, 0x040000)
	/* why would you run kick31 on an a500? */
	ROM_SYSTEM_BIOS(2, "kick31",  "Kickstart 3.1 (40.63)")
	ROMX_LOAD("kick40063.u6", 0x000000, 0x080000, CRC(fc24ae0d) SHA1(3b7f1493b27e212830f989f26ca76c02049f09ca), ROM_GROUPWORD | ROM_BIOS(4))	/* part number? */

	/* action replay cartridge */
	ROM_REGION16_BE(0x080000, "user2", ROMREGION_ERASEFF )
	ROM_CART_LOAD("cart", 0x0000, 0x080000, ROM_NOMIRROR | ROM_OPTIONAL)

	/* keyboard controller, mos 6500/1 mcu */
	ROM_REGION(0x800, "keyboard", 0)
	ROM_LOAD("328191-02.ic1", 0x000, 0x800, NO_DUMP)
ROM_END

#define rom_a500n    rom_a500

ROM_START( a500pl )
	ROM_REGION16_BE(0x080000, "user1", 0)
	ROM_DEFAULT_BIOS("kick204")

	ROM_SYSTEM_BIOS(0, "kick204", "Kickstart 2.04 (37.175)")
	ROMX_LOAD("390979-01.u6", 0x000000, 0x080000, CRC(c3bdb240) SHA1(c5839f5cb98a7a8947065c3ed2f14f5f42e334a1), ROM_GROUPWORD | ROM_BIOS(0))	/* identical to 363968.01 */

	/* action replay cartridge */
	ROM_REGION16_BE(0x080000, "user2", ROMREGION_ERASEFF )
	ROM_CART_LOAD("cart", 0x0000, 0x080000, ROM_NOMIRROR | ROM_OPTIONAL)

	/* keyboard controller, mos 6500/1 mcu */
	ROM_REGION(0x800, "keyboard", 0)
	ROM_LOAD("328191-02.ic1", 0x000, 0x800, NO_DUMP)
ROM_END

#define rom_a500pln    rom_a500pl

ROM_START( a600 )
	ROM_REGION16_BE(0x080000, "user1", 0)
	ROM_DEFAULT_BIOS("kick205")

	ROM_SYSTEM_BIOS(0, "kick205", "Kickstart 2.05 (37.299)")
	ROMX_LOAD("kickstart v2.05 r37.299 (1991)(commodore)(a600)[!].rom", 0x000000, 0x080000, CRC(83028fb5) SHA1(87508de834dc7eb47359cede72d2e3c8a2e5d8db), ROM_GROUPWORD | ROM_BIOS(0))

	// from A600HD (had HDD by default)
	ROM_SYSTEM_BIOS(1, "kick205a", "Kickstart 2.05 (37.300)")
	ROMX_LOAD("kickstart v2.05 r37.300 (1991)(commodore)(a600hd).rom",  0x000000, 0x080000, CRC(64466c2a) SHA1(f72d89148dac39c696e30b10859ebc859226637b), ROM_GROUPWORD | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "kick205b", "Kickstart 2.05 (37.300)")
	ROMX_LOAD("kickstart v2.05 r37.350 (1992)(commodore)(a600hd)[!].rom", 0x000000, 0x080000, CRC(43b0df7b) SHA1(02843c4253bbd29aba535b0aa3bd9a85034ecde4), ROM_GROUPWORD | ROM_BIOS(2))

	/* action replay cartridge */
	ROM_REGION16_BE(0x080000, "user2", ROMREGION_ERASEFF )
	ROM_CART_LOAD("cart", 0x0000, 0x080000, ROM_NOMIRROR | ROM_OPTIONAL)

	/* keyboard controller, mos 6500/1 mcu */
	ROM_REGION(0x800, "keyboard", 0)
	ROM_LOAD("328191-02.ic1", 0x000, 0x800, NO_DUMP)
ROM_END

#define rom_a600n    rom_a600


ROM_START( a1200 )
	ROM_REGION32_BE(0x080000, "user1", 0)
	ROM_DEFAULT_BIOS("kick31")
	ROM_SYSTEM_BIOS(0, "kick30", "Kickstart 3.0 (39.106)")
	ROMX_LOAD("391523-01.u6a", 0x000000, 0x040000, CRC(c742a412) SHA1(999eb81c65dfd07a71ee19315d99c7eb858ab186), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(1))
	ROMX_LOAD("391524-01.u6b", 0x000002, 0x040000, CRC(d55c6ec6) SHA1(3341108d3a402882b5ef9d3b242cbf3c8ab1a3e9), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(1))
//  ROMX_LOAD("kickstart v3.0 r39.106 (1992)(commodore)(a1200)[!].rom", 0x000000, 0x080000, CRC(6c9b07d2) SHA1(70033828182fffc7ed106e5373a8b89dda76faa5), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(1, "kick31", "Kickstart 3.1 (40.068)")
	ROMX_LOAD("391773-01.u6a", 0x000000, 0x040000, CRC(08dbf275) SHA1(b8800f5f909298109ea69690b1b8523fa22ddb37), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(2))
	ROMX_LOAD("391774-01.u6b", 0x000002, 0x040000, CRC(16c07bf8) SHA1(90e331be1970b0e53f53a9b0390b51b59b3869c2), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(2))

	// COMMODORE | 391508-01 REV0 | KEYBOARD MPU
	ROM_REGION(0x1040, "keyboard", 0)
	ROM_LOAD("391508-01.u13", 0x0000, 0x1040, NO_DUMP)

	// Amiga Tech REV1 Keyboard MPU
	ROM_REGION(0x2f40, "keyboard_rev1", 0)
	ROM_LOAD("391508-02.u13", 0x0000, 0x2f40, NO_DUMP)
ROM_END

#define rom_a1200n    rom_a1200

/* Note: I think those ROMs are correct, but they should be verified */
ROM_START( a3000 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_DEFAULT_BIOS("kick14")
	ROM_SYSTEM_BIOS(0, "kick14", "Kickstart 1.4 (36.16)")
	/* COPYRIGHT 1990 CAI // ALL RIGHTS RESERVED // ALPHA 5 ROM 0 CS=9713 */
	ROMX_LOAD("390629-02.u182", 0x00000, 0x40000, CRC(58327536) SHA1(d1713d7f31474a5948e6d488e33686061cf3d1e2), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(1))
	/* COPYRIGHT 1990 CAI // ALL RIGHTS RESERVED // ALPHA 5 ROM 1 CS=9B21 */
	ROMX_LOAD("390630-02.u183", 0x00002, 0x40000, CRC(fe2f7fb9) SHA1(c05c9c52d014c66f9019152b3f2a2adc2c678794), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "kick204", "Kickstart 2.04 (37.175)")
	ROMX_LOAD("390629-03.u182", 0x00000, 0x40000, CRC(a245dbdf) SHA1(83bab8e95d378b55b0c6ae6561385a96f638598f), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(2))
	ROMX_LOAD("390630-03.u183", 0x00002, 0x40000, CRC(7db1332b) SHA1(48f14b31279da6757848df6feb5318818f8f576c), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "kick31", "Kickstart 3.1 (40.68)")
	ROMX_LOAD("kick31.u182",    0x00000, 0x40000, CRC(286b9a0d) SHA1(6763a2258ec493f7408cf663110dae9a17803ad1), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(3))
	ROMX_LOAD("kick31.u183",    0x00002, 0x40000, CRC(0b8cde6a) SHA1(5f02e97b48ebbba87d516a56b0400c6fc3434d8d), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(3))

	ROM_REGION(0x1040, "keyboard", 0)
	ROM_LOAD("keyboard", 0x0000, 0x1040, NO_DUMP)
ROM_END

#define rom_a3000n    rom_a3000

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

/* High-end market line */

COMP( 1985, a1000,   0,      0,     a1000,  amiga, amiga_state,  amiga,  "Commodore Business Machines",  "Amiga 1000 (PAL)",      GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS )
COMP( 1985, a1000n,  a1000,  0,     a1000n, amiga, amiga_state,  amiga,  "Commodore Business Machines",  "Amiga 1000 (NTSC)",     GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS )



/* Low-end market line */

COMP( 1987, a500,    0,      0,      a500,      amiga, amiga_state,  amiga,  "Commodore Business Machines",  "Amiga 500 (PAL, OCS)",  GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS )
COMP( 1987, a500n,   a500,   0,      a500n,     amiga, amiga_state,  amiga,  "Commodore Business Machines",  "Amiga 500 (NTSC, OCS)", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS )

COMP( 1991, a500pl,  0,      0,      a500pls,   amiga, amiga_state,  amiga,  "Commodore Business Machines",  "Amiga 500+ (PAL, ECS)",  GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS )
COMP( 1991, a500pln, a500pl, 0,      a500plsn,  amiga, amiga_state,  amiga,  "Commodore Business Machines",  "Amiga 500+ (NTSC, ECS)", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS )

COMP( 1992, a600,    0,      0,      a600,      amiga, amiga_state,  amiga,  "Commodore Business Machines",  "Amiga 600 (PAL, ECS)",  GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS )
COMP( 1992, a600n,   a600,   0,      a600n,     amiga, amiga_state,  amiga,  "Commodore Business Machines",  "Amiga 600 (NTSC, ECS)", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS )

COMP( 1992, a1200,   0,      0,      a1200, 	amiga, a1200_state,a1200,  "Commodore Business Machines",  "Amiga 1200 (PAL, AGA)" , GAME_NOT_WORKING  )
COMP( 1992, a1200n,  a1200,  0,      a1200n,	amiga, a1200_state,a1200,  "Commodore Business Machines",  "Amiga 1200 (NTSC, AGA)" , GAME_NOT_WORKING )

COMP( 1992, a3000,   0,      0,      a3000, 	amiga, amiga_state,  a3000,  "Commodore Business Machines",  "Amiga 3000 (PAL, ECS, 68030)" , GAME_NOT_WORKING  )
COMP( 1992, a3000n,  a3000,  0,      a3000n,	amiga, amiga_state,  a3000,  "Commodore Business Machines",  "Amiga 3000 (NTSC, ECS, 68030)" , GAME_NOT_WORKING )



COMP( 1991, cdtv,   0,       0,      cdtv,  	cdtv,  amiga_state,   cdtv,   "Commodore Business Machines",  "CDTV (NTSC)",           GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS )

/* other official models */
/* Amiga 2000 - similar to 1000 */
/* Amiga 1500 - Amiga 2000 with two floppy drives (2nd replacing the HDD) */
/* Amiga 2500 - Amiga 2000 with 68020 accelerator card */
/* Amiga 4000 - AGA chipset, 68040 / 68060 CPU */


/* CD32 - see cd32.c */



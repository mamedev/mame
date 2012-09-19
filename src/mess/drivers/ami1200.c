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

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/6526cia.h"
#include "machine/i2cmem.h"
#include "machine/amigafdc.h"
#include "machine/amigakbd.h"

#include "includes/amiga.h"
#include "includes/cd32.h"



class ami1200_state : public amiga_state
{
public:
	ami1200_state(const machine_config &mconfig, device_type type, const char *tag)
		: amiga_state(mconfig, type, tag) { }

	UINT16 m_potgo_value;
	int m_cd32_shifter[2];
	int m_oldstate[2];
	DECLARE_WRITE32_MEMBER(aga_overlay_w);
	DECLARE_DRIVER_INIT(a1200);
};



#define A1200PAL_XTAL_X1  XTAL_28_37516MHz
#define A1200PAL_XTAL_X2  XTAL_4_433619MHz



static void handle_cd32_joystick_cia(ami1200_state *state, UINT8 pra, UINT8 dra);

WRITE32_MEMBER(ami1200_state::aga_overlay_w)
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

static WRITE8_DEVICE_HANDLER( ami1200_cia_0_porta_w )
{
	ami1200_state *state = device->machine().driver_data<ami1200_state>();

	/* bit 2 = Power Led on Amiga */
	set_led_status(device->machine(), 0, !BIT(data, 1));

	handle_cd32_joystick_cia(state, data, mos6526_r(device, space, 2));
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

static READ8_DEVICE_HANDLER( ami1200_cia_0_portb_r )
{
	/* parallel port */
	logerror("%s:CIA0_portb_r\n", device->machine().describe_context());
	return 0xff;
}

static WRITE8_DEVICE_HANDLER( ami1200_cia_0_portb_w )
{
	/* parallel port */
	logerror("%s:CIA0_portb_w(%02x)\n", device->machine().describe_context(), data);
}

static ADDRESS_MAP_START( a1200_map, AS_PROGRAM, 32, ami1200_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_RAMBANK("bank1") AM_SHARE("chip_ram")
	AM_RANGE(0xbfa000, 0xbfa003) AM_WRITE(aga_overlay_w)
	AM_RANGE(0xbfd000, 0xbfefff) AM_READWRITE16_LEGACY(amiga_cia_r, amiga_cia_w, 0xffffffff)
	AM_RANGE(0xc00000, 0xdfffff) AM_READWRITE16_LEGACY(amiga_custom_r, amiga_custom_w, 0xffffffff) AM_SHARE("custom_regs")
	AM_RANGE(0xe80000, 0xe8ffff) AM_READWRITE16_LEGACY(amiga_autoconfig_r, amiga_autoconfig_w, 0xffffffff)
	AM_RANGE(0xf80000, 0xffffff) AM_ROM AM_REGION("user1", 0)	/* Kickstart */
ADDRESS_MAP_END


//int cd32_input_port_val = 0;
//int cd32_input_select = 0;

static void cd32_potgo_w(running_machine &machine, UINT16 data)
{
	ami1200_state *state = machine.driver_data<ami1200_state>();
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

static void handle_cd32_joystick_cia(ami1200_state *state, UINT8 pra, UINT8 dra)
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


static INPUT_PORTS_START( a1200 )
	PORT_START("CIA0PORTA")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_SPECIAL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("CIA0PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JOY0DAT")
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, ami1200_state,amiga_joystick_convert, "P1JOY")
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("JOY1DAT")
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, ami1200_state,amiga_joystick_convert, "P2JOY")
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


/*************************************
 *
 *  Sound definitions
 *
 *************************************/

static READ8_DEVICE_HANDLER( a1200_cia_0_portA_r )
{
	UINT8 ret = device->machine().root_device().ioport("CIA0PORTA")->read() & 0xc0;	/* Gameport 1 and 0 buttons */
	ret |= device->machine().device<amiga_fdc>("fdc")->ciaapra_r();
	return ret;
}


static const mos6526_interface a1200_cia_0_intf =
{
	DEVCB_DEVICE_LINE("cia_0", amiga_cia_0_irq),									/* irq_func */
	DEVCB_NULL,	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_HANDLER("cia_0", a1200_cia_0_portA_r),
	DEVCB_DEVICE_HANDLER("cia_0", ami1200_cia_0_porta_w),		/* port A */
	DEVCB_DEVICE_HANDLER("cia_0", ami1200_cia_0_portb_r),
	DEVCB_DEVICE_HANDLER("cia_0", ami1200_cia_0_portb_w)		/* port B */
};

static const mos6526_interface a1200_cia_1_intf =
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


static MACHINE_CONFIG_START( a1200n, ami1200_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68EC020, AMIGA_68EC020_NTSC_CLOCK) /* 14.3 Mhz */
	MCFG_CPU_PROGRAM_MAP(a1200_map)

//  MCFG_CPU_ADD("keyboard_mpu", MC68HC05)

	MCFG_MACHINE_RESET_OVERRIDE(ami1200_state,amiga)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.997)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512*2, 312)
	MCFG_SCREEN_VISIBLE_AREA((129-8-8)*2, (449+8-1+8)*2, 44-8, 300+8-1)
	MCFG_SCREEN_UPDATE_DRIVER(ami1200_state, screen_update_amiga_aga)

	MCFG_VIDEO_START_OVERRIDE(ami1200_state,amiga_aga)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("amiga", AMIGA, XTAL_28_63636MHz/8)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(2, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(3, "lspeaker", 0.25)

	/* cia */
	MCFG_MOS8520_ADD("cia_0", AMIGA_68EC020_NTSC_CLOCK / 10, 0, a1200_cia_0_intf)
	MCFG_MOS8520_ADD("cia_1", AMIGA_68EC020_NTSC_CLOCK / 10, 0, a1200_cia_1_intf)

	/* fdc */
	MCFG_AMIGA_FDC_ADD("fdc", AMIGA_68EC020_NTSC_CLOCK/2)
	MCFG_FLOPPY_DRIVE_ADD("fd0", amiga_floppies, "35dd", 0, amiga_fdc::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd1", amiga_floppies, 0,      0, amiga_fdc::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd2", amiga_floppies, 0,      0, amiga_fdc::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd3", amiga_floppies, 0,      0, amiga_fdc::floppy_formats)
	
	MCFG_AMIGA_KEYBOARD_ADD("kbd")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( a1200p, a1200n )

	/* adjust for PAL specs */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(A1200PAL_XTAL_X1/2) /* 14.18758 MHz */

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(50)

	/* sound hardware */
	MCFG_SOUND_MODIFY("amiga")
	MCFG_SOUND_CLOCK(A1200PAL_XTAL_X1/8) /* 3.546895 MHz */

	/* cia */
	MCFG_DEVICE_MODIFY("cia_0")
	MCFG_DEVICE_CLOCK(A1200PAL_XTAL_X1/20)
	MCFG_DEVICE_MODIFY("cia_1")
	MCFG_DEVICE_CLOCK(A1200PAL_XTAL_X1/20)

	/* fdc */
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_DEVICE_CLOCK(A1200PAL_XTAL_X1/4)
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( a1200n )
	ROM_REGION32_BE(0x080000, "user1", 0)
	ROM_DEFAULT_BIOS("kick31")
	ROM_SYSTEM_BIOS(0, "kick30", "Kickstart 3.0 (39.106)")
	ROMX_LOAD("391523-01.u6a", 0x000000, 0x040000, CRC(c742a412) SHA1(999eb81c65dfd07a71ee19315d99c7eb858ab186), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(1))
	ROMX_LOAD("391524-01.u6b", 0x000002, 0x040000, CRC(d55c6ec6) SHA1(3341108d3a402882b5ef9d3b242cbf3c8ab1a3e9), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "kick31", "Kickstart 3.1 (40.068)")
	ROMX_LOAD("391773-01.u6a", 0x000000, 0x040000, CRC(08dbf275) SHA1(b8800f5f909298109ea69690b1b8523fa22ddb37), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(2))	// ROM_LOAD32_WORD_SWAP!
	ROMX_LOAD("391774-01.u6b", 0x000002, 0x040000, CRC(16c07bf8) SHA1(90e331be1970b0e53f53a9b0390b51b59b3869c2), ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(2))

	// COMMODORE | 391508-01 REV0 | KEYBOARD MPU
	ROM_REGION(0x1040, "keyboard_rev0", 0)
	ROM_LOAD("391508-01.u13", 0x0000, 0x1040, NO_DUMP)

	// Amiga Tech REV1 Keyboard MPU
	ROM_REGION(0x2f40, "keyboard_rev1", 0)
	ROM_LOAD("391508-02.u13", 0x0000, 0x2f40, NO_DUMP)
ROM_END

#define rom_a1200p    rom_a1200n


/***************************************************************************************************/

DRIVER_INIT_MEMBER(ami1200_state,a1200)
{
	static const amiga_machine_interface cd32_intf =
	{
		AGA_CHIP_RAM_MASK,
		NULL, NULL,			/* joy0dat_r & joy1dat_r */
		cd32_potgo_w,		/* potgo_w */
		NULL,				/* serdat_w */
		NULL,				/* scanline0_callback */
		NULL,				/* reset_callback */
		NULL,				/* nmi_callback */
		FLAGS_AGA_CHIPSET	/* flags */
	};

	/* configure our Amiga setup */
	amiga_machine_config(machine(), &cd32_intf);

	/* set up memory */
	membank("bank1")->configure_entry(0, m_chip_ram);
	membank("bank1")->configure_entry(1, machine().root_device().memregion("user1")->base());
}


/***************************************************************************************************/

/*    YEAR  NAME     PARENT   COMPAT  MACHINE INPUT   INIT      COMPANY       FULLNAME */
COMP( 1992, a1200n,  0,       0,      a1200n, a1200, ami1200_state,  a1200,  "Commodore Business Machines",  "Amiga 1200 (NTSC)" , GAME_NOT_WORKING )
COMP( 1992, a1200p,  a1200n,  0,      a1200p, a1200, ami1200_state,  a1200,  "Commodore Business Machines",  "Amiga 1200 (PAL)" , GAME_NOT_WORKING  )

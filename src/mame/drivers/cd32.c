/*
    Commodore Amiga CD-32 and related hardware
    (Cubo CD32 for arcades)

   Driver by Mariusz Wojcieszek
   CD-ROM controller by Ernesto Corvi
   Inputs by stephh

   This is basically a CD32 (Amiga 68020, AGA based games system) hacked up to run arcade games.
   see http://ninjaw.ifrance.com/cd32/cubo/ for a brief overview.

   Several of the games have Audio tracks, therefore the CRC / SHA1 information you get when
   reading your own CDs may not match those in the driver.  There is currently no 100% accurate
   way to rip the audio data with full sub-track and offset information.

   CD32 Hardware Specs (from Wikipedia, http://en.wikipedia.org/wiki/Amiga_CD32)
    * Main Processor: Motorola 68EC020 at 14.3 MHz
    * System Memory: 2 MB Chip RAM
    * 1 MB ROM with Kickstart ROM 3.1 and integrated cdfs.filesystem
    * 1KB of FlashROM for game saves
    * Graphics/Chipset: AGA Chipset
    * Akiko chip, which handles CD-ROM and can do Chunky to Planar conversion
    * Proprietary (MKE) CD-ROM drive at 2x speed
    * Expansion socket for MPEG cartridge, as well as 3rd party devices such as the SX-1 and SX32 expansion packs.
    * 4 8-bit audio channels (2 for left, 2 for right)
    * Gamepad, Serial port, 2 Gameports, Interfaces for keyboard

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/cdda.h"
#include "machine/6526cia.h"
#include "machine/i2cmem.h"
#include "includes/cd32.h"
#include "sound/cdda.h"
#include "imagedev/chd_cd.h"
#include "machine/amigafdc.h"

#define CD32PAL_XTAL_X1   XTAL_28_37516MHz
#define CD32PAL_XTAL_X2   XTAL_4_433619MHz
#define CD32PAL_XTAL_X3   XTAL_16_9344MHz


/* set to 0 to use control panel with only buttons (as in quiz games) - joy is default in dispenser setup */
#define MGPREM11_USE_JOY	1
#define MGNUMBER_USE_JOY	1


static void handle_cd32_joystick_cia(running_machine &machine, UINT8 pra, UINT8 dra);

static WRITE32_HANDLER( aga_overlay_w )
{
	if (ACCESSING_BITS_16_23)
	{
		data = (data >> 16) & 1;

		/* switch banks as appropriate */
		memory_set_bank(space->machine(), "bank1", data & 1);

		/* swap the write handlers between ROM and bank 1 based on the bit */
		if ((data & 1) == 0)
			/* overlay disabled, map RAM on 0x000000 */
			space->install_write_bank(0x000000, 0x1fffff, "bank1");
		else
			/* overlay enabled, map Amiga system ROM on 0x000000 */
			space->unmap_write(0x000000, 0x1fffff);
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

static WRITE8_DEVICE_HANDLER( cd32_cia_0_porta_w )
{
	/* bit 1 = cd audio mute */
	device->machine().device<cdda_device>("cdda")->set_output_gain( 0, ( data & 1 ) ? 0.0 : 1.0 );

	/* bit 2 = Power Led on Amiga */
	set_led_status(device->machine(), 0, (data & 2) ? 0 : 1);

	handle_cd32_joystick_cia(device->machine(), data, mos6526_r(device, 2));
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

static READ8_DEVICE_HANDLER( cd32_cia_0_portb_r )
{
	/* parallel port */
	logerror("%s:CIA0_portb_r\n", device->machine().describe_context());
	return 0xff;
}

static WRITE8_DEVICE_HANDLER( cd32_cia_0_portb_w )
{
	/* parallel port */
	logerror("%s:CIA0_portb_w(%02x)\n", device->machine().describe_context(), data);
}

static ADDRESS_MAP_START( cd32_map, AS_PROGRAM, 32, cd32_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_RAMBANK("bank1") AM_BASE_SIZE(m_chip_ram, m_chip_ram_size)
	AM_RANGE(0x800000, 0x800003) AM_READ_PORT("DIPSW1")
	AM_RANGE(0x800010, 0x800013) AM_READ_PORT("DIPSW2")
	AM_RANGE(0xb80000, 0xb8003f) AM_DEVREADWRITE_LEGACY("akiko", amiga_akiko32_r, amiga_akiko32_w)
	AM_RANGE(0xbfa000, 0xbfa003) AM_WRITE_LEGACY(aga_overlay_w)
	AM_RANGE(0xbfd000, 0xbfefff) AM_READWRITE16_LEGACY(amiga_cia_r, amiga_cia_w, 0xffffffff)
	AM_RANGE(0xc00000, 0xdfffff) AM_READWRITE16_LEGACY(amiga_custom_r, amiga_custom_w, 0xffffffff) AM_BASE(m_custom_regs)
	AM_RANGE(0xe00000, 0xe7ffff) AM_ROM AM_REGION("user1", 0x80000)	/* CD32 Extended ROM */
	AM_RANGE(0xa00000, 0xf7ffff) AM_NOP
	AM_RANGE(0xf80000, 0xffffff) AM_ROM AM_REGION("user1", 0x0)		/* Kickstart */
ADDRESS_MAP_END

/*************************************
 *
 *  Inputs
 *
 *************************************/

static void cd32_potgo_w(running_machine &machine, UINT16 data)
{
	cd32_state *state = machine.driver_data<cd32_state>();
	int i;

	if (state->m_input_hack != NULL)
		(*state->m_input_hack)(machine);

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

static void handle_cd32_joystick_cia(running_machine &machine, UINT8 pra, UINT8 dra)
{
	cd32_state *state = machine.driver_data<cd32_state>();
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

static UINT16 handle_joystick_potgor(running_machine &machine, UINT16 potgor)
{
	cd32_state *state = machine.driver_data<cd32_state>();
	static const char *const player_portname[] = { "P2", "P1" };
	int i;

	for (i = 0; i < 2; i++)
	{
		UINT16 p9dir = 0x0800 << (i * 4); /* output enable P9 */
		UINT16 p9dat = 0x0400 << (i * 4); /* data P9 */
		UINT16 p5dir = 0x0200 << (i * 4); /* output enable P5 */
		UINT16 p5dat = 0x0100 << (i * 4); /* data P5 */

		/* p5 is floating in input-mode */
		potgor &= ~p5dat;
		potgor |= state->m_potgo_value & p5dat;
		if (!(state->m_potgo_value & p9dir))
			potgor |= p9dat;
		/* P5 output and 1 -> shift register is kept reset (Blue button) */
		if ((state->m_potgo_value & p5dir) && (state->m_potgo_value & p5dat))
			state->m_cd32_shifter[i] = 8;
		/* shift at 1 == return one, >1 = return button states */
		if (state->m_cd32_shifter[i] == 0)
			potgor &= ~p9dat; /* shift at zero == return zero */
		if (state->m_cd32_shifter[i] >= 2 && (input_port_read(machine, player_portname[i]) & (1 << (state->m_cd32_shifter[i] - 2))))
			potgor &= ~p9dat;
	}
	return potgor;
}

static CUSTOM_INPUT(cubo_input)
{
	cd32_state *state = field.machine().driver_data<cd32_state>();
	return handle_joystick_potgor(field.machine(), state->m_potgo_value) >> 10;
}

static INPUT_PORTS_START( cd32 )
	PORT_START("CIA0PORTA")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("CIA0PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("JOY0DAT")
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(amiga_joystick_convert, "P2JOY")
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("JOY1DAT")
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(amiga_joystick_convert, "P1JOY")
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("POTGO")
	PORT_BIT( 0x4400, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(cubo_input, 0)
	PORT_BIT( 0xbbff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("P2JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)    /* BUTTON3 */
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1)    /* BUTTON4 */
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1)    /* BUTTON1 = START1 */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(1)    /* BUTTON2 */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)    /* BUTTON3 */
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2)    /* BUTTON4 */
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2)    /* BUTTON1 = START2 */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(2)    /* BUTTON2 */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(2)

	PORT_START("DIPSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1 1" )
	PORT_DIPSETTING(    0x01, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x02, 0x02, "DSW1 2" )
	PORT_DIPSETTING(    0x02, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x04, 0x04, "DSW1 3" )
	PORT_DIPSETTING(    0x04, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x08, 0x08, "DSW1 4" )
	PORT_DIPSETTING(    0x08, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x10, 0x10, "DSW1 5" )
	PORT_DIPSETTING(    0x10, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x20, 0x20, "DSW1 6" )
	PORT_DIPSETTING(    0x20, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x40, 0x40, "DSW1 7" )
	PORT_DIPSETTING(    0x40, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x80, 0x80, "DSW1 8" )
	PORT_DIPSETTING(    0x80, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )

	PORT_START("DIPSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2 1" )
	PORT_DIPSETTING(    0x01, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x02, 0x02, "DSW2 2" )
	PORT_DIPSETTING(    0x02, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x04, 0x04, "DSW2 3" )
	PORT_DIPSETTING(    0x04, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x08, 0x08, "DSW2 4" )
	PORT_DIPSETTING(    0x08, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x10, 0x10, "DSW2 5" )
	PORT_DIPSETTING(    0x10, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x20, 0x20, "DSW2 6" )
	PORT_DIPSETTING(    0x20, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x40, 0x40, "DSW2 7" )
	PORT_DIPSETTING(    0x40, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x80, 0x80, "DSW2 8" )
	PORT_DIPSETTING(    0x80, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )

INPUT_PORTS_END

static INPUT_PORTS_START( cndypuzl )
	PORT_INCLUDE( cd32 )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)    /* START1 and launch bubble */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)    /* START2 and launch bubble */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DIPSW1")
	PORT_BIT( 0x003f, IP_ACTIVE_HIGH, IPT_UNKNOWN )                   /* bits 0 to 5 must be set to 0 to insert coin */
	PORT_SERVICE( 0x0040, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_MODIFY("DIPSW2")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )                   /* not read at all */

INPUT_PORTS_END

static INPUT_PORTS_START( haremchl )
	PORT_INCLUDE( cd32 )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)    /* fire */
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)    /* START1 and draw */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)    /* retract */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)    /* fire */
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)    /* START2 and draw */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)    /* retract */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DIPSW1")
	PORT_BIT( 0x003f, IP_ACTIVE_HIGH, IPT_UNKNOWN )                   /* bits 0 to 5 must be set to 0 to insert coin */
	PORT_SERVICE( 0x0040, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_MODIFY("DIPSW2")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )                   /* not read at all */

INPUT_PORTS_END

static INPUT_PORTS_START( lsrquiz )
	PORT_INCLUDE( cd32 )

	PORT_MODIFY("P1JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)    /* "C" */
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)    /* "D" */
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)    /* START1 and "A" */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)    /* "B" */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)    /* "C" */
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)    /* "D" */
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)    /* START2 and "A" */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)    /* "B" */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DIPSW1")
	PORT_BIT( 0x003f, IP_ACTIVE_HIGH, IPT_UNKNOWN )                   /* bits 0 to 5 must be set to 0 to insert coin */
	PORT_SERVICE( 0x0040, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_MODIFY("DIPSW2")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )                   /* not read at all */

INPUT_PORTS_END

static INPUT_PORTS_START( lsrquiz2 )
	PORT_INCLUDE( cd32 )

	PORT_MODIFY("P1JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)    /* "C" */
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)    /* "D" */
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)    /* START1 and "A" */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)    /* "B" */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)    /* "C" */
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)    /* "D" */
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)    /* START2 and "A" */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)    /* "B" */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DIPSW1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(3)    /* "C" */
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(3)    /* "B" */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(3)    /* START3 and "A" */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_BUTTON4 ) PORT_PLAYER(3)    /* "D" */
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Free_Play ) )                  /* always set credits to 10 */
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Speed" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0020, "Ultra Turbo" )                         /* the game is unplayable !!! */
	PORT_SERVICE( 0x0040, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_MODIFY("DIPSW2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(4)    /* "C" */
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(4)    /* "B" */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(4)    /* START4 and "A" */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_BUTTON4 ) PORT_PLAYER(4)    /* "D" */
	PORT_BIT( 0x00f0, IP_ACTIVE_HIGH, IPT_UNKNOWN )                   /* bits 4 to 7 aren't tested */

INPUT_PORTS_END

static INPUT_PORTS_START( lasstixx )
	PORT_INCLUDE( cd32 )

	PORT_MODIFY("CIA0PORTA")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("P2JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)    /* draw */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)    /* retract */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DIPSW1")
	PORT_BIT( 0x003f, IP_ACTIVE_HIGH, IPT_UNKNOWN )                   /* bits 0 to 5 must be set to 0 to insert coin */
	PORT_SERVICE( 0x0040, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_MODIFY("DIPSW2")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )                   /* not read at all */

INPUT_PORTS_END

static INPUT_PORTS_START( mgnumber )
	PORT_INCLUDE( cd32 )

#if MGNUMBER_USE_JOY
	/* P1JOY, P2JOY, P1 and P2 inputs when control panel is set to "joystick" in the dispenser setup */
	PORT_MODIFY("P2JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME( "C (setup)" )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME( "Put Number" )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME( "Discard" )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME( "B (setup)" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
#else
	/* P1JOY, P2JOY, P1 and P2 inputs when control panel is set to "buttons" ("pulsanti") in the dispenser setup */
	/* P1JOY is still needed in the dispenser setup, so I don't remove it even if it isn't needed to play the game */
	PORT_MODIFY("P2JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME( "1 / C (setup)" )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME( "Discard" )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME( "B (setup)" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_NAME( "4" )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_PLAYER(1) PORT_NAME( "5" )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME( "2" )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME( "3" )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
#endif

	PORT_MODIFY("DIPSW1")
	PORT_DIPNAME( 0x01, 0x00, "Tokens" )                              /* Dip Switch or Input ? */
	PORT_DIPSETTING(    0x00, "OK" )
	PORT_DIPSETTING(    0x01, "ERROR!" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN1 )                     /* keep pressed to enter dispenser setup at start */
	PORT_BIT( 0x007e, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DIPSW2")
	PORT_DIPNAME( 0x01, 0x00, "Tickets" )                             /* Dip Switch or Input ? */
	PORT_DIPSETTING(    0x00, "OK" )
	PORT_DIPSETTING(    0x01, "ERROR!" )
	PORT_BIT( 0x00fe, IP_ACTIVE_HIGH, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START( mgprem11 )
	PORT_INCLUDE( cd32 )

#if MGPREM11_USE_JOY
	/* P1JOY, P2JOY, P1 and P2 inputs when control panel is set to "joystick" in the dispenser setup */
	PORT_MODIFY("P1JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3  ) PORT_PLAYER(1) PORT_NAME( "End Game / Abort / Confirm" )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON5  ) PORT_PLAYER(1) PORT_NAME( "C (setup)" )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1  ) PORT_PLAYER(1) PORT_NAME( "Put Card / Initialise / Continue Game / Cancel" )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2  ) PORT_PLAYER(1) PORT_NAME( "Discard / P1 A (setup)" )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4  ) PORT_PLAYER(1) PORT_NAME( "B (setup)" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN  )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON7  ) PORT_PLAYER(1) PORT_NAME( "P2 A (setup)" )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_PLAYER(1) PORT_NAME( "P2 B (setup)" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
#else
	/* P1JOY, P2JOY, P1 and P2 inputs when control panel is set to "buttons" ("pulsanti") in the dispenser setup */
	/* P1JOY is still needed in the dispenser setup, so I don't remove it even if it isn't needed to play the game */
	PORT_MODIFY("P1JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3  ) PORT_PLAYER(1) PORT_NAME( "End Game / Abort / Confirm" )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON5  ) PORT_PLAYER(1) PORT_NAME( "1 / C (setup)" )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1  ) PORT_PLAYER(1) PORT_NAME( "Initialise / Continue Game / Cancel" )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2  ) PORT_PLAYER(1) PORT_NAME( "Discard / P1 A (setup)" )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4  ) PORT_PLAYER(1) PORT_NAME( "B (setup)" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN  )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON8  ) PORT_PLAYER(1) PORT_NAME( "4" )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON9  ) PORT_PLAYER(1) PORT_NAME( "5" )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6  ) PORT_PLAYER(1) PORT_NAME( "2" )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON7  ) PORT_PLAYER(1) PORT_NAME( "3 / P2 A (setup)" )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_PLAYER(1) PORT_NAME( "P2 B (setup)" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
#endif

	PORT_MODIFY("DIPSW1")
	PORT_DIPNAME( 0x01, 0x00, "Tokens" )                              /* Dip Switch or Input ? */
	PORT_DIPSETTING(    0x00, "OK" )
	PORT_DIPSETTING(    0x01, "ERROR!" )
	PORT_DIPNAME( 0x50, 0x50, "Setup" )                               /* also affects payout values */
	PORT_DIPSETTING(    0x50, "Full Tick" )
//  PORT_DIPSETTING(    0x10, "Full Tick" )                           /* duplicated setting */
	PORT_DIPSETTING(    0x40, "104 & 105" )
	PORT_DIPSETTING(    0x00, "Full T+C" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )                    /* tested in dispenser setup when P1B3 is pressed */
	PORT_DIPSETTING(    0x20, "0x20" )
	PORT_DIPSETTING(    0x00, "0x00" )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN1 )                     /* keep pressed to enter dispenser setup at start */
	PORT_BIT( 0x000e, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DIPSW2")
	PORT_DIPNAME( 0x01, 0x00, "Tickets" )                             /* Dip Switch or Input ? */
	PORT_DIPSETTING(    0x00, "OK" )
	PORT_DIPSETTING(    0x01, "ERROR!" )
	PORT_BIT( 0x00fe, IP_ACTIVE_HIGH, IPT_UNKNOWN )

INPUT_PORTS_END

/*************************************
 *
 *  Sound definitions
 *
 *************************************/

static const mos6526_interface cia_0_intf =
{
	0,													/* tod_clock */
	DEVCB_LINE(amiga_cia_0_irq),									/* irq_func */
	DEVCB_NULL,	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_INPUT_PORT("CIA0PORTA"),
	DEVCB_HANDLER(cd32_cia_0_porta_w),		/* port A */
	DEVCB_HANDLER(cd32_cia_0_portb_r),
	DEVCB_HANDLER(cd32_cia_0_portb_w)		/* port B */
};

static const mos6526_interface cia_1_intf =
{
	0,													/* tod_clock */
	DEVCB_LINE(amiga_cia_1_irq),									/* irq_func */
	DEVCB_NULL,	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

#define	NVRAM_SIZE 1024
#define	NVRAM_PAGE_SIZE	16	/* max size of one write request */

static const i2cmem_interface i2cmem_interface =
{
	I2CMEM_SLAVE_ADDRESS, NVRAM_PAGE_SIZE, NVRAM_SIZE
};

static const microtouch_interface cd32_microtouch_config =
{
	DEVCB_DRIVER_MEMBER(cd32_state, microtouch_tx),
	NULL
};

static MACHINE_CONFIG_START( cd32base, cd32_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68EC020, AMIGA_68EC020_PAL_CLOCK) /* 14.3 Mhz */
	MCFG_CPU_PROGRAM_MAP(cd32_map)
	MCFG_DEVICE_ADD("akiko", AKIKO, 0)

	MCFG_MACHINE_RESET(amiga)

	MCFG_I2CMEM_ADD("i2cmem",i2cmem_interface)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512*2, 312)
	MCFG_SCREEN_VISIBLE_AREA((129-8-8)*2, (449+8-1+8)*2, 44-8, 300+8-1)
	MCFG_SCREEN_UPDATE_STATIC(amiga_aga)

	MCFG_VIDEO_START(amiga_aga)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("amiga", AMIGA, 3579545)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(2, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(3, "lspeaker", 0.25)

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 0.50 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 0.50 )

	/* cia */
	MCFG_MOS8520_ADD("cia_0", AMIGA_68EC020_PAL_CLOCK / 10, cia_0_intf)
	MCFG_MOS8520_ADD("cia_1", AMIGA_68EC020_PAL_CLOCK / 10, cia_1_intf)

	MCFG_MICROTOUCH_ADD( "microtouch", cd32_microtouch_config )

	/* fdc */
	MCFG_AMIGA_FDC_ADD("fdc", AMIGA_68000_NTSC_CLOCK)
MACHINE_CONFIG_END

struct cdrom_interface cd32_cdrom =
{
	"cd32_cdrom",
	NULL
};

static MACHINE_CONFIG_DERIVED( cd32, cd32base )
	MCFG_CDROM_ADD( "cdrom", cd32_cdrom )
	MCFG_SOFTWARE_LIST_ADD("cd_list","cd32")
MACHINE_CONFIG_END


#define ROM_LOAD16_WORD_BIOS(bios,name,offset,length,hash)     ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios+1))

#define CD32_BIOS \
	ROM_REGION32_BE(0x100000, "user1", 0 ) \
	ROM_SYSTEM_BIOS(0, "cd32", "Kickstart v3.1 rev 40.60 with CD32 Extended-ROM" ) \
	ROM_LOAD16_WORD_BIOS(0, "391640-03.u6a", 0x000000, 0x100000, CRC(d3837ae4) SHA1(06807db3181637455f4d46582d9972afec8956d9) ) \


ROM_START( cd32 )
	CD32_BIOS
ROM_END

/***************************************************************************************************/

static DRIVER_INIT( cd32 )
{
	cd32_state *state = machine.driver_data<cd32_state>();
	static const amiga_machine_interface cd32_intf =
	{
		AGA_CHIP_RAM_MASK,
		NULL, NULL, cd32_potgo_w,
		NULL,
		NULL, NULL,
		NULL,
		FLAGS_AGA_CHIPSET
	};

	/* configure our Amiga setup */
	amiga_machine_config(machine, &cd32_intf);

	/* set up memory */
	memory_configure_bank(machine, "bank1", 0, 1, state->m_chip_ram, 0);
	memory_configure_bank(machine, "bank1", 1, 1, machine.region("user1")->base(), 0);

	/* input hack */
	state->m_input_hack = NULL;
}

/* BIOS */
CONS( 1993, cd32,    0,       0,      cd32,   cd32,   cd32,   "Commodore Business Machines",  "Amiga CD32 (PAL)" , GAME_NOT_WORKING|GAME_IS_BIOS_ROOT )


/*

   Cubo CD32 (additional hardware and games by CD Express, Milan, Italy)

   The CuboCD32 is a stock retail CD32 unit with additional hardware to adapt it
   for JAMMA use

   Known Games:
   Title                | rev. | year
   ----------------------------------------------
   Candy Puzzle         |  1.0 | 1995
   Harem Challenge      |      | 1995
   Laser Quiz           |      | 1995
   Laser Quiz 2 "Italy" |  1.0 | 1995
   Laser Strixx 2       |      | 1995
   Magic Premium        |  1.1 | 1996
   Laser Quiz France    |  1.0 | 1995
   Odeon Twister        |      | 199x
   Odeon Twister 2      |202.19| 1999


   ToDo:
   - remove the hack needed to make inputs working


Stephh's notes (based on the game M68EC020 code and some tests) :


1) "Candy Puzzle"

settings (A5=0x059ac0) :

  - 051ba0.w (-$7f20,A5) : difficulty player 1 (0)
  - 051ba2.w (-$7f1e,A5) : difficulty player 2 (0)
  - 051ba4.w (-$7f1c,A5) : challenge rounds (3)
  - 051ba6.w (-$7f1a,A5) : death in 2P mode (0) - 0 = NO / 1 = YES
  - 051ba8.w (-$7f18,A5) : play x credit (1)
  - 051bac.w (-$7f14,A5) : coin x 1 play (1)
  - 051bb0.w (-$7f10,A5) : maxpalleattack (2)
  -                      : reset high score - NO* / YES
  - 051baa.w (-$7f16,A5) : 1 coin 2 players (1) - 0 = NO / 1 = YES

useful addresses :

  - 051c02.w (-$7ebe,A5) : must be 0x0000 instead of 0x0001 to accept coins !
  - 051c04.b (-$7ebc,A5) : credits
  - 051c10.l (-$7eb0,A5) : basic address for inputs = 0006fd18 :
      * BA + 1 x 6 + 4 : start 2 (bit 5)
      * BA + 1 x 6 + 0 : player 2 L/R (-1/0/1)
      * BA + 1 x 6 + 2 : player 2 U/D (-1/0/1)
      * BA + 0 x 6 + 4 : start 1 (bit 5)
      * BA + 0 x 6 + 0 : player 1 L/R (-1/0/1)
      * BA + 0 x 6 + 2 : player 1 U/D (-1/0/1)

routines :

  - 07acae : inputs read
  - 092c8a : buttons + coin read
      * D0 = 0 : read POTGO (player 2)
      * D0 = 1 : read POTGO (player 1)
  - 092baa : joy read - test bits 1 and 9 : 1 if bit 1, -1 if bit 9, 0 if none
      * D0 = 0 : read JOY0DAT (player 2)
      * D0 = 1 : read JOY1DAT (player 1)
  - 092bd4 : joy read - test bits 0 and 8 : 1 if bit 0, -1 if bit 8, 0 if none ("eor.w read, read >> 1" before test)
      * D0 = 0 : read JOY0DAT (player 2)
      * D0 = 1 : read JOY1DAT (player 1)

  - 07ada4 : coin verification
  - 07d0fe : start buttons verification


2) "Harem Challenge"

settings (A5=0x00a688 & A2=0x0028e8) :

  -                      : photo level - soft* / erotic / porno
  - 002906.b (A2 + 0x1e) : difficulty (2) - 1 = LOW / 2 = NORMAL / 3 = HIGH
  - 0028fc.b (A2 + 0x14) : deadly selftouch (0) - 0 = NO / 1 = YES
  - 0028ff.b (A2 + 0x17) : energy supply - 100%* / 0% / 50%
  - 002900.b (A2 + 0x18) : area to cover 1P - 75% / 80%* / 85%
  - 002901.b (A2 + 0x19) : area to cover 2P - 70% / 75%* / 80%
  - 002902.w (A2 + 0x1a) : level time (00C8) - 0096 = 1:00 / 00C8 = 1:20 / 00FA = 1:40
  - 002904.b (A2 + 0x1c) : tournament mode (2) - 0 = NO CHALLENGE / 2 = BEST OF 3 / 3 = BEST OF 5
  -                      : reset hi-score - NO* / YES
  - 0028fd.b (A2 + 0x15) : coin x 1 play (1) <=> number of credits used for each play
  - 0028fe.b (A2 + 0x16) : play x credit (1) <=> credits awarded for each coin
  -                      : demo photo level - soft* / erot

useful addresses :

  - 002907.b (-$7f00,A5 -> $1f,A2) : must be 0x00 instead of 0x01 to accept coins !
  - 00278c.w (-$7efc,A5) : player 2 inputs
  - 00278e.w (-$7efa,A5) : player 1 inputs
  - 002790.b (-$7ef8,A5) : credits

  - 0028e8.b (A2       ) : level - range (0x01 - 0x09 and 0x00 when in "attract mode")
  - 0028ea.w (A2 + 0x02) : time (not used for "continue play") - range (0x0000 - 0x0014 with 0x0000 being max)

  - 026fe8.l (-$7fa0,A5 -> A2 + $18 x 0 + 0x00) : player 1 energy (in MSW after swap) - range (0x00000000 - 0x00270000)
  - 026fec.l (-$7fa0,A5 -> A2 + $18 x 0 + 0x04) : player 1 score
  - 026ff8.w (-$7fa0,A5 -> A2 + $18 x 0 + 0x10) : player 1 girls
  - 027000.l (-$7fa0,A5 -> A2 + $18 x 1 + 0x00) : player 2 energy (in MSW after swap) - range (0x00000000 - 0x00270000)
  - 027004.l (-$7fa0,A5 -> A2 + $18 x 1 + 0x04) : player 2 score
  - 027010.w (-$7fa0,A5 -> A2 + $18 x 1 + 0x10) : player 2 girls

routines :

  - 04a5de : inputs read
  - 061928 : buttons + coin read
      * D0 = 0 : read POTGO (player 2)
      * D0 = 1 : read POTGO (player 1)

  - 04a692 : coin verification
  - 050b8c : start buttons verification


3) "Laser Quiz"

settings (A5=0x00a580 & A2=0x001e08) :

  - 001a54.w (see below) : numero di vite (2)
  - 001e08.w (A2       ) : primo bonus dom. (5)
  - 001e0a.w (A2 + 0x02) : max dom. facili (7)
  - 001e0c.w (A2 + 0x04) : incremento dom. (2)
  - 001e0e.w (A2 + 0x06) : inizio dom. diff. (10)
  - 001e10.w (A2 + 0x08) : risposte a video (4)
  - 001e12.w (A2 + 0x0a) : checkmark risp. (1) - 0 = CONTEMPORANEO / 1 = IMMEDIATO
  - 001e16.w (A2 + 0x0e) : vel. chronometro (32) - 3C = LENTA / 32 = NORMALE / 28 = VELOCE
  - 001e14.w (A2 + 0x0c) : volume musica (14) - 14 = NORMALE / 0A = BASSO / 00 = NO MUSICA
  - 001e18.b (A2 + 0x10) : vita premio bonus (1) - 0 = PER IL PRIMO / 1 = PER EMTRAMBI
  -                      : reset hi-score - NO* / SI
  - 001e19.b (A2 + 0x11) : coin x 1 play (1) <=> number of credits used for each play
  - 001e1a.b (A2 + 0x12) : foto erotiche (1) - 1 = SI / 2 = NO

useful addresses :

  - 001e1b.b (-$7fe0,A5 -> $13,A2) : must be 0x00 instead of 0x01 to accept coins !
  - 0026a4.w (-$7f0c,A5) : player 2 inputs
  - 0026a6.w (-$7f0a,A5) : player 1 inputs
  - 002606.b (-$7faa,A5) : credits

  - 001a54.w (-$7fe4,A5 -> $04,A2) : lives at start
  - 001a59.b (-$7fe4,A5 -> $09,A2) : bonus level (so lives aren't decremented) ? 0x00 = NO / 0x01 YES
  - 001a5a.b (-$7fe4,A5 -> $0a,A2) : level - range 0x00-0x63

  - 00c9b0.b (-$7fdc,A5 -> A2 + $06 x 0 + 0x00) : player 1 active ? 0x00 = NO / 0x01 YES / 0x02 = "ATTENDI"
  - 00c9b1.b (-$7fdc,A5 -> A2 + $06 x 0 + 0x01) : player 1 lives - range 0x00-0x63
  - 00c9b2.l (-$7fdc,A5 -> A2 + $06 x 0 + 0x02) : player 1 score
  - 00c9b6.b (-$7fdc,A5 -> A2 + $06 x 1 + 0x00) : player 2 active ? 0x00 = NO / 0x01 YES / 0x02 = "ATTENDI"
  - 00c9b7.b (-$7fdc,A5 -> A2 + $06 x 1 + 0x01) : player 2 lives - range 0x00-0x63
  - 00c9b8.l (-$7fdc,A5 -> A2 + $06 x 1 + 0x02) : player 2 score

  - 02327c.l (-$50,A4) : player 1 inputs (ingame)
  - 023280.l (-$4c,A4) : player 2 inputs (ingame)

routines :

  - 0cedf8 : inputs read
  - 0d8150 : buttons + coin read
      * D0 = 0 : read POTGO (player 2)
      * D0 = 1 : read POTGO (player 1)

  - 0cee42 : coin verification
  - 0cefc6 : start buttons verification

  - 0cb40c : inputs (ingame) read


4) "Laser Quiz 2"

settings (A5=0x0531e8 & A2=0x0460f0) :

  - 0460f0.w (A2       ) : numero di vite (2)
  - 0460f0.w (A2 + 0x02) : primo bonus dom. (5)
  - 0460f0.w (A2 + 0x04) : max dom. facili (7)
  - 0460f0.w (A2 + 0x06) : incremento dom. (2)
  - 0460f0.w (A2 + 0x08) : inizio dom. diff. (10)
  - 0460f0.w (A2 + 0x0a) : risposte a video (4)
  - 0460f0.w (A2 + 0x0c) : checkmark risp. (1) - 0 = CONTEMPORANEO / 1 = IMMEDIATO
  - 0460f0.w (A2 + 0x10) : vel. chronometro (5) - 5 = LENTA / 4 = NORMALE / 3 = VELOCE
  - 0460f0.w (A2 + 0x0e) : volume musica (3F) - 3F = NORMALE / 20 = BASSO / 00 = NO MUSICA
  - 0460f0.b (A2 + 0x12) : vita premio bonus (1) - 0 = PER IL PRIMO / 1 = PER TUTTI
  -                      : reset hi-score - NO* / SI
  - 0460f0.b (A2 + 0x13) : coin x 1 play (1) <=> number of credits used for each play
  - 0460f0.b (A2 + 0x14) : play x credit (1) <=> credits awarded for each coin
  - 0460f0.b (A2 + 0x15) : foto erotiche (1) - 1 = SI / 2 = NO
  - 0460f0.b (A2 + 0x16) : numero giocatori (0) - 0 = 2 / 1 = 4

useful addresses :

  - 046107.b (-$7fdc,A5 -> $17,A2) : must be 0x00 instead of 0x01 to accept coins !
  - 04b250.w (-$7f98,A5) : player 2 inputs
  - 04b252.w (-$7f96,A5) : player 1 inputs
  - 04b254.w (-$7f94,A5) : player 3 inputs
  - 04b256.w (-$7f92,A5) : player 4 inputs
  - 04b248.b (-$7fa0,A5) : credits

  - 04570d.b (-$7fe0,A5 -> $05,A2) : bonus level (so lives aren't decremented) ? 0x00 = NO / 0x01 YES
  - 04570e.b (-$7fe4,A5 -> $06,A2) : level - range 0x00-0x63

  - 051fc0.b (-$7fd8,A5 -> A2 + $12 x 0 + 0x00) : player 1 active ? 0x00 = NO / 0x01 = YES / 0x02 = "ATTENDI"
  - 051fc1.b (-$7fd8,A5 -> A2 + $12 x 0 + 0x01) : player 1 lives - range 0x00-0x63
  - 051fc2.l (-$7fd8,A5 -> A2 + $12 x 0 + 0x02) : player 1 score
  - 051fd2.b (-$7fd8,A5 -> A2 + $12 x 1 + 0x00) : player 2 active ? 0x00 = NO / 0x01 = YES / 0x02 = "ATTENDI"
  - 051fd3.b (-$7fd8,A5 -> A2 + $12 x 1 + 0x01) : player 2 lives - range 0x00-0x63
  - 051fd4.l (-$7fd8,A5 -> A2 + $12 x 1 + 0x02) : player 2 score
  - 051fe4.b (-$7fd8,A5 -> A2 + $12 x 2 + 0x00) : player 3 active ? 0x00 = NO / 0x01 = YES / 0x02 = "ATTENDI"
  - 051fe5.b (-$7fd8,A5 -> A2 + $12 x 2 + 0x01) : player 3 lives - range 0x00-0x63
  - 051fe6.l (-$7fd8,A5 -> A2 + $12 x 2 + 0x02) : player 3 score
  - 051ff6.b (-$7fd8,A5 -> A2 + $12 x 3 + 0x00) : player 4 active ? 0x00 = NO / 0x01 = YES / 0x02 = "ATTENDI"
  - 051ff7.b (-$7fd8,A5 -> A2 + $12 x 3 + 0x01) : player 4 lives - range 0x00-0x63
  - 051ff8.l (-$7fd8,A5 -> A2 + $12 x 3 + 0x02) : player 4 score

routines :

  - 0746d6 : inputs read
  - 08251a : buttons + coin read
      * D0 = 0 : read POTGO (player 2)
      * D0 = 1 : read POTGO (player 1)

  - 07472e : coin verification


5) "Laser Strixx 2"

settings (A5=0x00a688 & A2=0x0027f8) :

  -                      : photo level - soft* / erotic / porno
  - 002910.l (A2 + 0x18) : difficulty (00010000) - 00008000 = LOW / 00010000 = NORMAL / 00018000 = HIGH
  - 002818.b (A2 + 0x20) : deadly selftouch (0) - 0 = NO / 1 = YES
  - 00281b.b (A2 + 0x23) : energy supply - 100%* / 0% / 50%
  - 002819.b (A2 + 0x21) : coin x 1 play (1) <=> number of credits used for each play
  - 00281a.b (A2 + 0x22) : play x credit (1) <=> credits awarded for each coin

useful addresses :

  - 00281c.b (-$7fa2,A5 -> $24,A2) : must be 0x00 instead of 0x01 to accept coins !
  - 0026f4.l (-$7f94,A5) : player 1 inputs (in MSW after swap)
  - 0026ee.b (-$7f9a,A5) : credits

  - 0027f8.l (A2 + 0x00) : player 1 energy (in MSW after swap) - range (0x00000000 - 0x00270000)
  - 0027fc.b (A2 + 0x04) : level - range (0x01 - 0x09 and 0x00 when in "attract mode")
  - 0027fe.w (A2 + 0x06) : time (not used for "continue play") - range (0x0000 - 0x0014 with 0x0000 being max)
  - 002800.l (A2 + 0x08) : player 1 score
  - 00280a.b (A2 + 0x12) : mouth status (0x00 = not out - 0x01 = out - 0x02 = captured = LIVE STRIP at the end of level)
  - 00280b.b (A2 + 0x13) : strip phase (0x01 - n with 0x01 being last phase)
                             Ann : n = 0x05
                             Eva : n = 0x04
                             Jo  : n = 0x06
                             Sue : n = 0x04
                             Bob : n = 0x05

routines :

  - 04d1ae : inputs read
  - 058340 : buttons + coin read
      * D0 = 0 : read POTGO (player 2)
      * D0 = 1 : read POTGO (player 1)

  - 04d1da : coin verification


6) "Magic Number"

settings (A5=0x053e78 & A2=0x03f540) : TO DO !

useful addresses :

  - 04bfa0.l (-$7ed8,A5) : must be 0x00000000 instead of 0x00010000 to accept coins !
  - 04bf18.w (-$7f60,A5) : player 1 inputs (in MSW after swap)
  - 04bf78.l (-$7e00,A5) : player 2 inputs (in MSW after swap)
  - 03f547.b (-$7f92,A5 -> $7,A2) : credits

routines :

  - 07a480 : inputs read
  - 085bb8 : buttons + coin read
      * D0 = 0 : read POTGO (player 2)
      * D0 = 1 : read POTGO (player 1)

  - 07a50e : coin verification


7) "Magic Premium"

settings (A5=0x04ce48 & A2=0x0419b0) : TO DO !

useful addresses :

  - 044f7e.b (-$7eca,A5) : must be 0x00 instead of 0x01 to accept coins !
  - 044f02.w (-$7f94,A5) : player 1 inputs
  - 044f6a.l (-$7ede,A5) : player 2 inputs (in MSW after swap)
  - 0419b1.b (-$7fc0,A5 -> $1,A2) : credits - range 0x00-0x09

routines :

  - 0707d6 : inputs read
  - 07b3b2 : buttons + coin read
      * D0 = 0 : read POTGO (player 2)
      * D0 = 1 : read POTGO (player 1)

  - 070802 : coin verification

*/


ROM_START( cndypuzl )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "cndypuzl", 0, SHA1(5f41ed3521b3e05d233ac1245b78cb0b118b2b90) )
ROM_END

ROM_START( haremchl )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "haremchl", 0, SHA1(abbab347c0d7c5eef0465d0eee770754a452e874) )
ROM_END

ROM_START( lsrquiz )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "lsrquiz", 0, SHA1(41fb6cd0c9d36bd77e9c3db69d36801edc791e96) )
ROM_END

ROM_START( lsrquiz2 )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "lsrquiz2", 0, SHA1(78e261df1c548fa492e6cf37a9469640bb8816bf) )
ROM_END

ROM_START( mgprem11 )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "mgprem11", 0, SHA1(7808db33d5949f6c86d12b32bc388c12377e7038) )
ROM_END

ROM_START( lasstixx )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "lasstixx", 0, SHA1(b8f6138e1f1840c193e786c56dab03c512f3e21f) )
ROM_END

ROM_START( mgnumber )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "magicnumber", 0, SHA1(60e1fadc42694742d19cc0ac2b6e99e9e33faa3d) )
ROM_END

ROM_START( odeontw2 )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "odeontw2", 0, SHA1(f39e09f35b65a6ae9f1eba4a22f970626b7d3b71) )
ROM_END



/*************************************
 *
 *  Hacks (to allow coins to be inserted)
 *
 *************************************/

static void cndypuzl_input_hack(running_machine &machine)
{
	cd32_state *state = machine.driver_data<cd32_state>();

	if (cpu_get_pc(machine.device("maincpu")) < state->m_chip_ram_size)
	{
		//(*state->m_chip_ram_w)(0x051c02, 0x0000);

		UINT32 r_A5 = cpu_get_reg(machine.device("maincpu"), M68K_A5);
		(*state->m_chip_ram_w)(state, r_A5 - 0x7ebe, 0x0000);
	}
}

static DRIVER_INIT(cndypuzl)
{
	cd32_state *state = machine.driver_data<cd32_state>();
	DRIVER_INIT_CALL(cd32);
	state->m_input_hack = cndypuzl_input_hack;
}

static void haremchl_input_hack(running_machine &machine)
{
	cd32_state *state = machine.driver_data<cd32_state>();

	if (cpu_get_pc(machine.device("maincpu")) < state->m_chip_ram_size)
	{
		//amiga_chip_ram_w8(state, 0x002907, 0x00);

		UINT32 r_A5 = cpu_get_reg(machine.device("maincpu"), M68K_A5);
		UINT32 r_A2 = ((*state->m_chip_ram_r)(state, r_A5 - 0x7f00 + 0) << 16) | ((*state->m_chip_ram_r)(state, r_A5 - 0x7f00 + 2));
		amiga_chip_ram_w8(state, r_A2 + 0x1f, 0x00);
	}
}

static DRIVER_INIT(haremchl)
{
	cd32_state *state = machine.driver_data<cd32_state>();
	DRIVER_INIT_CALL(cd32);
	state->m_input_hack = haremchl_input_hack;
}

static void lsrquiz_input_hack(running_machine &machine)
{
	cd32_state *state = machine.driver_data<cd32_state>();

	if (cpu_get_pc(machine.device("maincpu")) < state->m_chip_ram_size)
	{
		//amiga_chip_ram_w8(state, 0x001e1b, 0x00);

		UINT32 r_A5 = cpu_get_reg(machine.device("maincpu"), M68K_A5);
		UINT32 r_A2 = ((*state->m_chip_ram_r)(state, r_A5 - 0x7fe0 + 0) << 16) | ((*state->m_chip_ram_r)(state, r_A5 - 0x7fe0 + 2));
		amiga_chip_ram_w8(state, r_A2 + 0x13, 0x00);
	}
}

static DRIVER_INIT(lsrquiz)
{
	cd32_state *state = machine.driver_data<cd32_state>();
	DRIVER_INIT_CALL(cd32);
	state->m_input_hack = lsrquiz_input_hack;
}

/* The hack isn't working if you exit the test mode with P1 button 2 ! */
static void lsrquiz2_input_hack(running_machine &machine)
{
	cd32_state *state = machine.driver_data<cd32_state>();

	if (cpu_get_pc(machine.device("maincpu")) < state->m_chip_ram_size)
	{
		//amiga_chip_ram_w8(state, 0x046107, 0x00);

		UINT32 r_A5 = cpu_get_reg(machine.device("maincpu"), M68K_A5);
		UINT32 r_A2 = ((*state->m_chip_ram_r)(state, r_A5 - 0x7fdc + 0) << 16) | ((*state->m_chip_ram_r)(state, r_A5 - 0x7fdc + 2));
		amiga_chip_ram_w8(state, r_A2 + 0x17, 0x00);
	}
}

static DRIVER_INIT(lsrquiz2)
{
	cd32_state *state = machine.driver_data<cd32_state>();
	DRIVER_INIT_CALL(cd32);
	state->m_input_hack = lsrquiz2_input_hack;
}

static void lasstixx_input_hack(running_machine &machine)
{
	cd32_state *state = machine.driver_data<cd32_state>();

	if (cpu_get_pc(machine.device("maincpu")) < state->m_chip_ram_size)
	{
		//amiga_chip_ram_w8(state, 0x00281c, 0x00);

		UINT32 r_A5 = cpu_get_reg(machine.device("maincpu"), M68K_A5);
		UINT32 r_A2 = ((*state->m_chip_ram_r)(state, r_A5 - 0x7fa2 + 0) << 16) | ((*state->m_chip_ram_r)(state, r_A5 - 0x7fa2 + 2));
		amiga_chip_ram_w8(state, r_A2 + 0x24, 0x00);
	}
}

static DRIVER_INIT(lasstixx)
{
	cd32_state *state = machine.driver_data<cd32_state>();
	DRIVER_INIT_CALL(cd32);
	state->m_input_hack = lasstixx_input_hack;
}

static void mgnumber_input_hack(running_machine &machine)
{
	cd32_state *state = machine.driver_data<cd32_state>();

	if (cpu_get_pc(machine.device("maincpu")) < state->m_chip_ram_size)
	{
		//(*state->m_chip_ram_w)(0x04bfa0, 0x0000);

		UINT32 r_A5 = cpu_get_reg(machine.device("maincpu"), M68K_A5);
		(*state->m_chip_ram_w)(state, r_A5 - 0x7ed8, 0x0000);
	}
}

static DRIVER_INIT(mgnumber)
{
	cd32_state *state = machine.driver_data<cd32_state>();
	DRIVER_INIT_CALL(cd32);
	state->m_input_hack = mgnumber_input_hack;
}

static void mgprem11_input_hack(running_machine &machine)
{
	cd32_state *state = machine.driver_data<cd32_state>();

	if (cpu_get_pc(machine.device("maincpu")) < state->m_chip_ram_size)
	{
		//amiga_chip_ram_w8(state, 0x044f7e, 0x00);

		UINT32 r_A5 = cpu_get_reg(machine.device("maincpu"), M68K_A5);
		amiga_chip_ram_w8(state, r_A5 - 0x7eca, 0x00);
	}
}

static DRIVER_INIT(mgprem11)
{
	cd32_state *state = machine.driver_data<cd32_state>();
	DRIVER_INIT_CALL(cd32);
	state->m_input_hack = mgprem11_input_hack;
}

static INPUT_PORTS_START( odeontw2 )
//  PORT_INCLUDE( cd32 )
	PORT_START("CIA0PORTA")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("CIA0PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )


	PORT_START("DIPSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1 1" )
	PORT_DIPSETTING(    0x01, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x02, 0x02, "DSW1 2" )
	PORT_DIPSETTING(    0x02, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x04, 0x04, "DSW1 3" )
	PORT_DIPSETTING(    0x04, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x08, 0x08, "DSW1 4" )
	PORT_DIPSETTING(    0x08, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x10, 0x10, "DSW1 5" )
	PORT_DIPSETTING(    0x10, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x20, 0x20, "DSW1 6" )
	PORT_DIPSETTING(    0x20, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x40, 0x40, "DSW1 7" )
	PORT_DIPSETTING(    0x40, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x80, 0x80, "DSW1 8" )
	PORT_DIPSETTING(    0x80, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )

	PORT_START("DIPSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2 1" )
	PORT_DIPSETTING(    0x01, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x02, 0x02, "DSW2 2" )
	PORT_DIPSETTING(    0x02, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x04, 0x04, "DSW2 3" )
	PORT_DIPSETTING(    0x04, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x08, 0x08, "DSW2 4" )
	PORT_DIPSETTING(    0x08, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x10, 0x10, "DSW2 5" )
	PORT_DIPSETTING(    0x10, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x20, 0x20, "DSW2 6" )
	PORT_DIPSETTING(    0x20, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x40, 0x40, "DSW2 7" )
	PORT_DIPSETTING(    0x40, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x80, 0x80, "DSW2 8" )
	PORT_DIPSETTING(    0x80, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )

INPUT_PORTS_END

static void serial_w(running_machine &machine, UINT16 data)
{
	cd32_state *state = machine.driver_data<cd32_state>();
	UINT8 data8 = data & 0xff;
	if ( data8 != 0x00 )
		state->m_microtouch->rx(*machine.memory().first_space(), 0, data8);
}

WRITE8_MEMBER (cd32_state::microtouch_tx)
{
	amiga_serial_in_w(machine(), data);
}

static DRIVER_INIT( odeontw2 )
{
	cd32_state *state = machine.driver_data<cd32_state>();
	static const amiga_machine_interface cd32_intf =
	{
		AGA_CHIP_RAM_MASK,
		NULL, NULL, cd32_potgo_w,
		serial_w,
		NULL, NULL,
		NULL,
		FLAGS_AGA_CHIPSET
	};

	/* configure our Amiga setup */
	amiga_machine_config(machine, &cd32_intf);

	/* set up memory */
	memory_configure_bank(machine, "bank1", 0, 1, state->m_chip_ram, 0);
	memory_configure_bank(machine, "bank1", 1, 1, machine.region("user1")->base(), 0);

	/* input hack */
	state->m_input_hack = NULL;
}

/***************************************************************************************************/

// these are clones of the cd32 SYSTEM because they run on a stock retail unit, with additional HW
GAME( 1995, cndypuzl, cd32, cd32base, cndypuzl, cndypuzl, ROT0, "CD Express", "Candy Puzzle (v1.0)",       GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1995, haremchl, cd32, cd32base, haremchl, haremchl, ROT0, "CD Express", "Harem Challenge",           GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1995, lsrquiz,  cd32, cd32base, lsrquiz,  lsrquiz,  ROT0, "CD Express", "Laser Quiz Italy",          GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )  /* no player 2 inputs (ingame) */
GAME( 1995, lsrquiz2, cd32, cd32base, lsrquiz2, lsrquiz2, ROT0, "CD Express", "Laser Quiz 2 Italy (v1.0)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1995, lasstixx, cd32, cd32base, lasstixx, lasstixx, ROT0, "CD Express", "Laser Strixx 2",            GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1995, mgnumber, cd32, cd32base, mgnumber, mgnumber, ROT0, "CD Express", "Magic Number",              GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1996, mgprem11, cd32, cd32base, mgprem11, mgprem11, ROT0, "CD Express", "Magic Premium (v1.1)",      GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1999, odeontw2, cd32, cd32base, odeontw2, odeontw2, ROT0, "CD Express", "Odeon Twister 2 (v202.19)", GAME_NOT_WORKING )

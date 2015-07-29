// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari System 1 hardware (available in TTL or LSI version)

    driver by Aaron Giles

    Games supported:
        * Marble Madness (1984) [5 sets, available in either a TTL or LSI version]
        * Peter Packrat (1984)
        * Indiana Jones & the Temple of Doom (1985) [6 sets]
        * Road Runner (1985) [3 sets]
        * Road Blasters (1987) [10 sets]

    Known bugs:
        * none at this time

****************************************************************************

    Atari System 1 TTL motherboard factory modifications.  (From TTL Rev 1 dump.)

    Location A2
      74S02 with no pins pulled up
      (On the solder side the traces for Pin 1 and 3 are severed and swapped by
      jumper wire on the parts side.)

        Pin 1 - Jumpered to C15
        Pin 3 - Jumpered to C16


    Location J16
      74LS164 piggybacked on top of existing chip with all pins pulled up
      except for VCC and GND

        Pin 1  - Jumpered to VCC
        Pin 2  - Jumpered to VCC
        Pin 3  - No connection
        Pin 4  - No connection
        Pin 5  - No connection
        Pin 6  - No connection
        Pin 7  - Ground
        Pin 8  - Jumpered to Pin 4 of 74LS244 at C17
        Pin 9  - Jumpered to Pin 11 of piggybacked 74LS00 at J17
        Pin 10 - No Connection
        Pin 11 - No Connection
        Pin 12 - No Connection
        Pin 13 - Jumpered to Pin 9 of piggybacked 74LS00 at J17
        Pin 14 - VCC


    Location J17
      74LS00 piggybacked on top of existing chip with all pins pulled up except
      for VCC and GND

        Pin 1  - Jumpered to Pin 2 of YM2151 at H16
        Pin 2  - Jumpered to Pin 6 of 74LS273 at E10
        Pin 3  - Jumpered to Pin 12 of piggybacked 74LS00
        Pin 4  - No connection
        Pin 5  - No connection
        Pin 6  - No connection
        Pin 7  - Ground
        Pin 8  - Jumpered to Pin 2 of 6502 at L19 (Pin 2 of 6502 pulled up)
        Pin 9  - Jumpered to Pin 13 of piggybacked 74LS164 at J16
        Pin 10 - Jumpered to Pin 4 of 74LS367 at D17
        Pin 11 - Jumpered to Pin 13 of piggybacked 74LS164 at J16
        Pin 12 - Jumpered to Pin 3 of piggybacked 74LS00
        Pin 13 - Jumpered to VCC
        Pin 14 - VCC

****************************************************************************

    Memory map

****************************************************************************

    ========================================================================
    MAIN CPU
    ========================================================================
    000000-07FFFF   R     xxxxxxxx xxxxxxxx   Program ROM
    080000-087FFF   R     xxxxxxxx xxxxxxxx   Slapstic-protected ROM
    2E0000          R     -------- x-------   Sprite interrupt state
    400000-401FFF   R/W   xxxxxxxx xxxxxxxx   Program RAM
    800000            W   -------x xxxxxxxx   Playfield X scroll
    820000            W   -------x xxxxxxxx   Playfield Y scroll
    840000            W   -------- xxxxxxxx   Playfield priority color mask
    860000            W   -------- xxxxxxxx   Audio/video control
                      W   -------- x-------      (Sound CPU reset)
                      W   -------- -x------      (Trackball test)
                      W   -------- --xxx---      (Motion object RAM bank select)
                      W   -------- -----x--      (Playfield tile bank select)
                      W   -------- ------x-      (Trackball resolution & test LED)
                      W   -------- -------x      (Alphanumerics tile bank select)
    880000            W   -------- --------   Watchdog reset
    8A0000            W   -------- --------   VBLANK IRQ acknowledge
    8C0000            W   -------- --------   EEPROM enable
    900000-9FFFFF   R/W   xxxxxxxx xxxxxxxx   Catridge external RAM/ROM
    A00000-A01FFF   R/W   xxxxxxxx xxxxxxxx   Playfield RAM (64x64 tiles)
                    R/W   x------- --------      (Horizontal flip)
                    R/W   -xxxxxxx --------      (Tile ROM & palette select)
                    R/W   -------- xxxxxxxx      (Tile index, 8 LSB)
    A02000-A02FFF   R/W   xxxxxxxx xxxxxxxx   Motion object RAM (8 banks x 64 entries x 4 words)
                    R/W   x------- --------      (0: X flip)
                    R/W   --xxxxxx xxx-----      (0: Y position)
                    R/W   -------- ----xxxx      (0: Number of Y tiles - 1)
                    R/W   xxxxxxxx --------      (64: Tile ROM & palette select)
                    R/W   -------- xxxxxxxx      (64: Tile index, 8 LSB)
                    R/W   --xxxxxx xxx-----      (128: X position)
                    R/W   -------- ----xxxx      (128: Number of X tiles - 1)
                    R/W   -------- --xxxxxx      (192: Link to the next object)
    A03000-A03FFF   R/W   --xxxxxx xxxxxxxx   Alphanumerics RAM
                    R/W   --x----- --------      (Opaque/transparent)
                    R/W   ---xxx-- --------      (Palette index)
                    R/W   ------xx xxxxxxxx      (Tile index)
    B00000-B001FF   R/W   xxxxxxxx xxxxxxxx   Alphanumerics palette RAM (256 entries)
                    R/W   xxxx---- --------      (Intensity)
                    R/W   ----xxxx --------      (Red)
                    R/W   -------- xxxx----      (Green)
                    R/W   -------- ----xxxx      (Blue)
    B00200-B003FF   R/W   xxxxxxxx xxxxxxxx   Motion object palette RAM (256 entries)
    B00400-B005FF   R/W   xxxxxxxx xxxxxxxx   Playfield palette RAM (256 entries)
    B00600-B007FF   R/W   xxxxxxxx xxxxxxxx   Translucency palette RAM (256 entries)
    F00000-F00FFF   R/W   -------- xxxxxxxx   EEPROM
    F20000-F20007   R     -------- xxxxxxxx   Analog inputs
    F40000-F4001F   R     -------- xxxxxxxx   Joystick inputs
    F40000-F4001F     W   -------- --------   Joystick IRQ enable
    F60000          R     -------- xxxxxxxx   Switch inputs
                    R     -------- x-------      (Command buffer full)
                    R     -------- -x------      (Self test)
                    R     -------- --x-xxxx      (Game-specific switches)
                    R     -------- ---x----      (VBLANK)
    FC0000          R     -------- xxxxxxxx   Sound response read
    FE0000            W   -------- xxxxxxxx   Sound command write
    ========================================================================
    Interrupts:
        IRQ2 = joystick interrupt
        IRQ3 = sprite-based interrupt
        IRQ4 = VBLANK
        IRQ6 = sound CPU communications
    ========================================================================


    ========================================================================
    SOUND CPU
    ========================================================================
    0000-0FFF   R/W   xxxxxxxx   Program RAM
    1000-100F   R/W   xxxxxxxx   M6522
    1000-1FFF   R/W   xxxxxxxx   Catridge external RAM/ROM
    1800-1801   R/W   xxxxxxxx   YM2151 communications
    1810        R     xxxxxxxx   Sound command read
    1810          W   xxxxxxxx   Sound response write
    1820        R     x--xxxxx   Sound status/input read
                R     x-------      (Self-test)
                R     ---x----      (Response buffer full)
                R     ----x---      (Command buffer full)
                R     -----x--      (Service coin)
                R     ------x-      (Left coin)
                R     -------x      (Right coin)
    1824-1825     W   -------x   LED control
    1826          W   -------x   Right coin counter
    1827          W   -------x   Left coin counter
    1870-187F   R/W   xxxxxxxx   POKEY communications
    4000-FFFF   R     xxxxxxxx   Program ROM
    ========================================================================
    Interrupts:
        IRQ = YM2151 interrupt
        NMI = latch on sound command
    ========================================================================


Game Identification (information sourced from internal Atari Vax emails)

All System I games are required to initialize a byte in the configuration area
(at 01006E specifically) to a certain value.  The utility system on the main
board interrogates this byte to determine if a new game cart has been installed
and clears the EEROM accordingly.

Here is the list, which includes confirmed (*) and unconfirmed values:

Diagnostic Cartridge:           255
Peter Packrat:                  000*
Marble Madness:                 001*
Indiana Jones - Temple of Doom: 002*
Road Runner:                    003*
Relief Pitcher (unreleased):    004
Off-Road (unreleased):          004
RoadBlasters (aka Future Vette):005*


****************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m6502.h"
#include "machine/atarigen.h"
#include "machine/6522via.h"
#include "sound/2151intf.h"
#include "sound/pokey.h"
#include "video/atarimo.h"
#include "includes/atarisy1.h"



/*************************************
 *
 *  Initialization & interrupts
 *
 *************************************/

void atarisy1_state::update_interrupts()
{
	m_maincpu->set_input_line(2, m_joystick_int && m_joystick_int_enable ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(3, m_scanline_int_state ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(4, m_video_int_state ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(6, m_sound_int_state ? ASSERT_LINE : CLEAR_LINE);
}


MACHINE_START_MEMBER(atarisy1_state,atarisy1)
{
	atarigen_state::machine_start();

	save_item(NAME(m_joystick_int));
	save_item(NAME(m_joystick_int_enable));
	save_item(NAME(m_joystick_value));
}


MACHINE_RESET_MEMBER(atarisy1_state,atarisy1)
{
	atarigen_state::machine_reset();

	/* reset the joystick parameters */
	m_joystick_value = 0;
	m_joystick_int = 0;
	m_joystick_int_enable = 0;
}



/*************************************
 *
 *  Joystick I/O
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(atarisy1_state::delayed_joystick_int)
{
	m_joystick_value = param;
	m_joystick_int = 1;
	update_interrupts();
}


READ16_MEMBER(atarisy1_state::joystick_r)
{
	int newval = 0xff;
	static const char *const portnames[] = { "IN0", "IN1" };

	/* digital joystick type */
	if (m_joystick_type == 1)
		newval = (ioport("IN0")->read() & (0x80 >> offset)) ? 0xf0 : 0x00;

	/* Hall-effect analog joystick */
	else if (m_joystick_type == 2)
		newval = ioport(portnames[offset & 1])->read();

	/* Road Blasters gas pedal */
	else if (m_joystick_type == 3)
		newval = ioport("IN1")->read();

	/* the A4 bit enables/disables joystick IRQs */
	m_joystick_int_enable = ((offset >> 3) & 1) ^ 1;

	/* clear any existing interrupt and set a timer for a new one */
	m_joystick_int = 0;
	m_joystick_timer->adjust(attotime::from_usec(50), newval);
	update_interrupts();

	return m_joystick_value;
}


WRITE16_MEMBER(atarisy1_state::joystick_w)
{
	/* the A4 bit enables/disables joystick IRQs */
	m_joystick_int_enable = ((offset >> 3) & 1) ^ 1;
}



/*************************************
 *
 *  Trackball I/O
 *
 *************************************/

READ16_MEMBER(atarisy1_state::trakball_r)
{
	int result = 0xff;

	/* Marble Madness trackball type -- rotated 45 degrees! */
	if (m_trackball_type == 1)
	{
		int player = (offset >> 1) & 1;
		int which = offset & 1;

		/* when reading the even ports, do a real analog port update */
		if (which == 0)
		{
			UINT8 posx,posy;

			if (player == 0)
			{
				posx = (INT8)ioport("IN0")->read();
				posy = (INT8)ioport("IN1")->read();
			}
			else
			{
				posx = (INT8)ioport("IN2")->read();
				posy = (INT8)ioport("IN3")->read();
			}

			m_cur[player][0] = posx + posy;
			m_cur[player][1] = posx - posy;
		}

		result = m_cur[player][which];
	}

	/* Road Blasters steering wheel */
	else if (m_trackball_type == 2)
		result = ioport("IN0")->read();

	return result;
}



/*************************************
 *
 *  Sound input
 *
 *************************************/

READ8_MEMBER(atarisy1_state::switch_6502_r)
{
	int temp = ioport("1820")->read();
	if (!(ioport("F60000")->read() & 0x0040)) temp ^= 0x80;
	return temp;
}



/*************************************
 *
 *  TMS5220 communications
 *
 *************************************/

/*
 *  All communication to the 5220 goes through an VIA6522.
 *
 *  The ports are hooked in like follows:
 *
 *  Port A, D0-D7 = TMS5220 data lines (i/o)
 *
 *  Port B, D0 =    Write strobe (out)
 *          D1 =    Read strobe (out)
 *          D2 =    Ready signal (in)
 *          D3 =    Interrupt signal (in)
 *          D4 =    TMS5220 frequency select (out)
 *          D5 =    LED (out)
 */

WRITE8_MEMBER(atarisy1_state::via_pa_w)
{
	m_tms->data_w(space, 0, data);
}


READ8_MEMBER(atarisy1_state::via_pa_r)
{
	return m_tms->status_r(space, 0);
}


WRITE8_MEMBER(atarisy1_state::via_pb_w)
{
	/* write strobe */
	m_tms->wsq_w(data & 1);

	/* read strobe */
	m_tms->rsq_w((data & 2)>>1);

	/* bit 4 is connected to an up-counter, clocked by SYCLKB */
	data = 5 | ((data >> 3) & 2);
	m_tms->set_frequency(ATARI_CLOCK_14MHz/2 / (16 - data));
}


READ8_MEMBER(atarisy1_state::via_pb_r)
{
	return (m_tms->readyq_r() << 2) | (m_tms->intq_r() << 3);
}



/*************************************
 *
 *  Sound LED handlers
 *
 *************************************/

WRITE8_MEMBER(atarisy1_state::led_w)
{
	set_led_status(machine(), offset, ~data & 1);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, atarisy1_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x087fff) AM_ROM /* slapstic maps here */
	AM_RANGE(0x2e0000, 0x2e0001) AM_READ(atarisy1_int3state_r)
	AM_RANGE(0x400000, 0x401fff) AM_RAM
	AM_RANGE(0x800000, 0x800001) AM_WRITE(atarisy1_xscroll_w) AM_SHARE("xscroll")
	AM_RANGE(0x820000, 0x820001) AM_WRITE(atarisy1_yscroll_w) AM_SHARE("yscroll")
	AM_RANGE(0x840000, 0x840001) AM_WRITE(atarisy1_priority_w)
	AM_RANGE(0x860000, 0x860001) AM_WRITE(atarisy1_bankselect_w) AM_SHARE("bankselect")
	AM_RANGE(0x880000, 0x880001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x8a0000, 0x8a0001) AM_WRITE(video_int_ack_w)
	AM_RANGE(0x8c0000, 0x8c0001) AM_DEVWRITE("eeprom", atari_eeprom_device, unlock_write)
	AM_RANGE(0x900000, 0x9fffff) AM_RAM
	AM_RANGE(0xa00000, 0xa01fff) AM_RAM_DEVWRITE("playfield", tilemap_device, write) AM_SHARE("playfield")
	AM_RANGE(0xa02000, 0xa02fff) AM_RAM_WRITE(atarisy1_spriteram_w) AM_SHARE("mob")
	AM_RANGE(0xa03000, 0xa03fff) AM_RAM_DEVWRITE("alpha", tilemap_device, write) AM_SHARE("alpha")
	AM_RANGE(0xb00000, 0xb007ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xf00000, 0xf00fff) AM_DEVREADWRITE8("eeprom", atari_eeprom_device, read, write, 0x00ff)
	AM_RANGE(0xf20000, 0xf20007) AM_READ(trakball_r)
	AM_RANGE(0xf40000, 0xf4001f) AM_READWRITE(joystick_r, joystick_w)
	AM_RANGE(0xf60000, 0xf60003) AM_READ_PORT("F60000")
	AM_RANGE(0xf80000, 0xf80001) AM_DEVWRITE8("soundcomm", atari_sound_comm_device, main_command_w, 0x00ff) /* used by roadbls2 */
	AM_RANGE(0xfc0000, 0xfc0001) AM_DEVREAD8("soundcomm", atari_sound_comm_device, main_response_r, 0x00ff)
	AM_RANGE(0xfe0000, 0xfe0001) AM_DEVWRITE8("soundcomm", atari_sound_comm_device, main_command_w, 0x00ff)
ADDRESS_MAP_END



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, atarisy1_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x100f) AM_DEVREADWRITE("via6522_0", via6522_device, read, write)
	AM_RANGE(0x1800, 0x1801) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x1810, 0x1810) AM_DEVREADWRITE("soundcomm", atari_sound_comm_device, sound_command_r, sound_response_w)
	AM_RANGE(0x1820, 0x1820) AM_READ(switch_6502_r)
	AM_RANGE(0x1824, 0x1825) AM_WRITE(led_w)
	AM_RANGE(0x1870, 0x187f) AM_DEVREADWRITE("pokey", pokey_device, read, write)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( marble )
	PORT_START("IN0")  /* F20000 */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("IN1")  /* F20002 */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_START("IN2")  /* F20004 */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("IN3")  /* F20006 */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(2)

	PORT_START("F60000")    /* F60000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_ATARI_COMM_MAIN_TO_SOUND_READY("soundcomm")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("1820")  /* 1820 (sound) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_ATARI_COMM_MAIN_TO_SOUND_READY("soundcomm")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_ATARI_COMM_SOUND_TO_MAIN_READY("soundcomm")
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL )
INPUT_PORTS_END


static INPUT_PORTS_START( peterpak )
	PORT_START("IN0")   /* F40000 */
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("IN1")   /* n/a */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")   /* n/a */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")   /* n/a */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("F60000")    /* F60000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Throw/P1 Start")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Right Throw/P2 Start")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Jump")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_ATARI_COMM_MAIN_TO_SOUND_READY("soundcomm")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("1820")  /* 1820 (sound) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_ATARI_COMM_MAIN_TO_SOUND_READY("soundcomm")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_ATARI_COMM_SOUND_TO_MAIN_READY("soundcomm")
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL )
INPUT_PORTS_END


static INPUT_PORTS_START( indytemp )
	PORT_START("IN0")   /* F40000 */
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("IN1")   /* n/a */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")   /* n/a */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")   /* n/a */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("F60000")    /* F60000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Whip/P1 Start")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Right Whip/P2 Start")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* freeze? */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_ATARI_COMM_MAIN_TO_SOUND_READY("soundcomm")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("1820")  /* 1820 (sound) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_ATARI_COMM_MAIN_TO_SOUND_READY("soundcomm")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_ATARI_COMM_SOUND_TO_MAIN_READY("soundcomm")
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL )
INPUT_PORTS_END


static INPUT_PORTS_START( indytemc )
	PORT_INCLUDE( indytemp )

	PORT_MODIFY("IN0")  /* F40000 */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
INPUT_PORTS_END


static INPUT_PORTS_START( roadrunn )
	PORT_START("IN0")   /* F40000 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("IN1")   /* F40002 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("IN2")   /* n/a */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")   /* n/a */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("F60000")    /* F60000 */
	/* Note that "P1 Button 1' and 'P2 Start' both act as "Hop' Buttons" in game"  */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Hop/P1 Start")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right Hop/P2 Start")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_ATARI_COMM_MAIN_TO_SOUND_READY("soundcomm")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("1820")  /* 1820 (sound) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_ATARI_COMM_MAIN_TO_SOUND_READY("soundcomm")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_ATARI_COMM_SOUND_TO_MAIN_READY("soundcomm")
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL )
INPUT_PORTS_END


static INPUT_PORTS_START( roadblst )
	PORT_START("IN0")   /* F20000 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("IN1")   /* F40000 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(64)

	PORT_START("IN2")   /* n/a */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")   /* n/a */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("F60000")    /* F60000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Special Weapon")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Lasers")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_ATARI_COMM_MAIN_TO_SOUND_READY("soundcomm")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("1820")  /* 1820 (sound) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_ATARI_COMM_MAIN_TO_SOUND_READY("soundcomm")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_ATARI_COMM_SOUND_TO_MAIN_READY("soundcomm")
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout anlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8, 9, 10, 11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};


static GFXDECODE_START( atarisy1 )
	GFXDECODE_ENTRY( "alpha", 0x00000, anlayout,       0, 64 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( atarisy1, atarisy1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68010, ATARI_CLOCK_14MHz/2)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", atarigen_state, video_int_gen)

	MCFG_SLAPSTIC_ADD("slapstic")

	MCFG_CPU_ADD("audiocpu", M6502, ATARI_CLOCK_14MHz/8)
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_MACHINE_START_OVERRIDE(atarisy1_state,atarisy1)
	MCFG_MACHINE_RESET_OVERRIDE(atarisy1_state,atarisy1)

	MCFG_ATARI_EEPROM_2804_ADD("eeprom")

	MCFG_TIMER_DRIVER_ADD("joystick_timer", atarisy1_state, delayed_joystick_int)
	MCFG_TIMER_DRIVER_ADD("scan_timer", atarisy1_state, atarisy1_int3_callback)
	MCFG_TIMER_DRIVER_ADD("int3off_timer", atarisy1_state, atarisy1_int3off_callback)
	MCFG_TIMER_DRIVER_ADD("yreset_timer", atarisy1_state, atarisy1_reset_yscroll_callback)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", atarisy1)

	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(IIIIRRRRGGGGBBBB)

	MCFG_TILEMAP_ADD_STANDARD("playfield", "gfxdecode", 2, atarisy1_state, get_playfield_tile_info, 8,8, SCAN_ROWS, 64,64)
	MCFG_TILEMAP_ADD_STANDARD_TRANSPEN("alpha", "gfxdecode", 2, atarisy1_state, get_alpha_tile_info, 8,8, SCAN_ROWS, 64,32, 0)

	MCFG_ATARI_MOTION_OBJECTS_ADD("mob", "screen", atarisy1_state::s_mob_config)
	MCFG_ATARI_MOTION_OBJECTS_GFXDECODE("gfxdecode")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	/* note: these parameters are from published specs, not derived */
	/* video timing comes from an 82S163 (H) and an 82S129 (V) */
	MCFG_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(atarisy1_state, screen_update_atarisy1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(atarisy1_state,atarisy1)

	/* sound hardware */
	MCFG_ATARI_SOUND_COMM_ADD("soundcomm", "audiocpu", WRITELINE(atarigen_state, sound_int_write_line))
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", ATARI_CLOCK_14MHz/4)
	MCFG_YM2151_IRQ_HANDLER(DEVWRITELINE("soundcomm", atari_sound_comm_device, ym2151_irq_gen))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.80)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.80)

	MCFG_SOUND_ADD("pokey", POKEY, ATARI_CLOCK_14MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.40)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.40)

	MCFG_SOUND_ADD("tms", TMS5220C, ATARI_CLOCK_14MHz/2/11)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	/* via */
	MCFG_DEVICE_ADD("via6522_0", VIA6522, 0)
	MCFG_VIA6522_READPA_HANDLER(READ8(atarisy1_state, via_pa_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(atarisy1_state, via_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(atarisy1_state, via_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(atarisy1_state, via_pb_w))
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

/*
    Atari System 1 Motherboard BIOS

    Initially Atari released the Atari System 1 motherboard in the TTL version and
    then later consolidated some of the chips into the LSI version.

    Motherboard Manuals:
        * The TTL Motherboard is referenced in the TM-277 and SP-277 manuals.
        * The LSI Motherboard is referenced in the TM-286 and SP-286 manuals.
*/

#define ROM_LOAD16_BYTE_BIOS(bios,name,offset,length,hash) \
	ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios+1) | ROM_SKIP(1)) /* Note '+1' */

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
	ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios+1)) /* Note '+1' */

#define MOTHERBOARD_BIOS                                                                                                       \
	ROM_SYSTEM_BIOS( 0, "ttl", "TTL Motherboard (Rev 2)" )                                                                     \
	ROM_LOAD16_BYTE_BIOS(0, "136032.205.l13", 0x00000, 0x04000, CRC(88d0be26) SHA1(d124045eccc562ff0423b23a240e27ad740fa0c9) ) \
	ROM_LOAD16_BYTE_BIOS(0, "136032.206.l12", 0x00001, 0x04000, CRC(3c79ef05) SHA1(20fdca7131478e1ee12691bdafd2d5bb74cbd16f) ) \
	ROM_SYSTEM_BIOS( 1, "ttl1", "TTL Motherboard (Rev 1)" )                                                                    \
	ROM_LOAD16_BYTE_BIOS(1, "136032.105.l13", 0x00000, 0x04000, CRC(690b37d3) SHA1(547372f1044a3442aa52fcd2b3546540aba59344) ) \
	ROM_LOAD16_BYTE_BIOS(1, "136032.106.l12", 0x00001, 0x04000, CRC(76ee86c4) SHA1(cbcd424510435a04e9041967a13781fd19b0f2c4) ) \
	ROM_SYSTEM_BIOS( 2, "lsi", "LSI Motherboard" )                                                                             \
	ROM_LOAD16_BYTE_BIOS(2, "136032.114.j11", 0x00000, 0x04000, CRC(195c54ad) SHA1(d7cda3cd3db4c6f77074ca05e96ae11b62e048b7) ) \
	ROM_LOAD16_BYTE_BIOS(2, "136032.115.j10", 0x00001, 0x04000, CRC(7275b4dc) SHA1(0896ab37ea832a1335046353612c1b4c86d8d040) )

#define MOTHERBOARD_ALPHA                                                                                              \
	ROM_LOAD_BIOS(0, "136032.104.f5", 0x00000, 0x02000, CRC(7a29dc07) SHA1(72ba464da01bd6d3a91b8d9997d5ac14b6f47aad) ) \
	ROM_LOAD_BIOS(1, "136032.104.f5", 0x00000, 0x02000, CRC(7a29dc07) SHA1(72ba464da01bd6d3a91b8d9997d5ac14b6f47aad) ) \
	ROM_LOAD_BIOS(2, "136032.107.b2", 0x00000, 0x02000, CRC(315e4bea) SHA1(a00ea23fbdbf075f8f3f184275be83387e8ac82b) ) /* is this bad? it just seems to be missing upper nibbles and hence some of the planes? */

#define MOTHERBOARD_PROMS                                                                                              \
	ROM_LOAD_BIOS(0, "136032.101.e3",     0x00000, 0x00100, CRC(7e84972a) SHA1(84d422b53547271e3a07342704a05ef481db3f99) ) \
	ROM_LOAD_BIOS(0, "136032.102.e5",     0x00000, 0x00100, CRC(ebf1e0ae) SHA1(2d327e78832edd67ca3909c25b8c8c839637a1ed) ) \
	ROM_LOAD_BIOS(0, "136032.103.f7.bin", 0x00000, 0x000eb, CRC(92d6a0b4) SHA1(0a42a4816c89447b16e1f3245409591efea98a4a) ) /* N82S153 */                                        \
	ROM_LOAD_BIOS(1, "136032.101.e3",     0x00000, 0x00100, CRC(7e84972a) SHA1(84d422b53547271e3a07342704a05ef481db3f99) ) \
	ROM_LOAD_BIOS(1, "136032.102.e5",     0x00000, 0x00100, CRC(ebf1e0ae) SHA1(2d327e78832edd67ca3909c25b8c8c839637a1ed) ) \
	ROM_LOAD_BIOS(1, "136032.103.f7.bin", 0x00000, 0x000eb, CRC(92d6a0b4) SHA1(0a42a4816c89447b16e1f3245409591efea98a4a) ) /* N82S153 */

ROM_START( atarisy1 )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASE00 )    /* 64k for 6502 code */

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x100000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by the TTL version.) */
	MOTHERBOARD_PROMS

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 ) /* graphics mapping PROMs */
ROM_END


/*
    Marble Madness

    Initially Atari released the TTL version and then later consolidated some
    of the chips into the LSI version.

    There are two different versions of the schematics and they are not compatible.
    The SP-276 manual is for the TTL version and the SP-276-A manual is for the LSI version.
*/

ROM_START( marble )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136033.623",   0x10000, 0x04000, CRC(284ed2e9) SHA1(a24d2fd587dffcc8536ef28fcbcf5c964a6b67a9) )
	ROM_LOAD16_BYTE( "136033.624",   0x10001, 0x04000, CRC(d541b021) SHA1(978b1565da746f7389eaf7646604990fb28d47ed) )
	ROM_LOAD16_BYTE( "136033.625",   0x18000, 0x04000, CRC(563755c7) SHA1(a444b72ff4cdecee3b9dd7e636d658c31ecc186c) )
	ROM_LOAD16_BYTE( "136033.626",   0x18001, 0x04000, CRC(860feeb3) SHA1(d6059c1fe13f28ada27f6586215a16e2117e3ecd) )
	ROM_LOAD16_BYTE( "136033.627",   0x20000, 0x04000, CRC(d1dbd439) SHA1(cefc0fa9c71512c961272fcf0f9c069f1396468e) )
	ROM_LOAD16_BYTE( "136033.628",   0x20001, 0x04000, CRC(957d6801) SHA1(b007d9e45a1442ab1c9ec1463f9f46ea85fb0659) )
	ROM_LOAD16_BYTE( "136033.229",   0x28000, 0x04000, CRC(c81d5c14) SHA1(0464ea183685de83e797b9d946b4acc409f4c451) )
	ROM_LOAD16_BYTE( "136033.630",   0x28001, 0x04000, CRC(687a09f7) SHA1(95e31acf29cd8d51beefa9b0e4acd92b81980c2f) )
	ROM_LOAD16_BYTE( "136033.107",   0x80000, 0x04000, CRC(f3b8745b) SHA1(4754eac5e6d8547b3ee00f3f48eaa560eb403862) )
	ROM_LOAD16_BYTE( "136033.108",   0x80001, 0x04000, CRC(e51eecaa) SHA1(37d51a9e9cb33d1156d02a312ac8e202a18d7c20) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136033.421",   0x8000, 0x4000, CRC(78153dc3) SHA1(d4e68226b87df8834dc3d6daa9d683f17896c32e) )
	ROM_LOAD( "136033.422",   0xc000, 0x4000, CRC(2e66300e) SHA1(49acb9443c5d2c1016cde7f489deab2575dd82ca) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x100000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136033.137",   0x00000, 0x04000, CRC(7a45f5c1) SHA1(b826a178660ff2e278558e4779586737751dca5e) )  /* bank 1, plane 0 */
	ROM_LOAD( "136033.138",   0x04000, 0x04000, CRC(7e954a88) SHA1(238a913529781f424a6f49c3fba1524684d0cbcf) )
	ROM_LOAD( "136033.139",   0x10000, 0x04000, CRC(1eb1bb5f) SHA1(987a8289fd4be06b6899bb8c620ddfa4c4b966b0) )  /* bank 1, plane 1 */
	ROM_LOAD( "136033.140",   0x14000, 0x04000, CRC(8a82467b) SHA1(25538e98e5fdadfa6de3bd57ee8658c7add0169d) )
	ROM_LOAD( "136033.141",   0x20000, 0x04000, CRC(52448965) SHA1(e2ce22f89304b2d6858d0c61040d8ff2ee33347f) )  /* bank 1, plane 2 */
	ROM_LOAD( "136033.142",   0x24000, 0x04000, CRC(b4a70e4f) SHA1(d8e4d6dbbef3be86558ab23646c77400793a5eb1) )
	ROM_LOAD( "136033.143",   0x30000, 0x04000, CRC(7156e449) SHA1(361e024e1173299d0e6b776a7c1be10767cfab0d) )  /* bank 1, plane 3 */
	ROM_LOAD( "136033.144",   0x34000, 0x04000, CRC(4c3e4c79) SHA1(642e469ce10067502ce3a920066184a9c73a44c5) )
	ROM_LOAD( "136033.145",   0x40000, 0x04000, CRC(9062be7f) SHA1(ae372433da441b69345d67f5e838b8479557517b) )  /* bank 1, plane 4 */
	ROM_LOAD( "136033.146",   0x44000, 0x04000, CRC(14566dca) SHA1(7134c233daba3b2510d1c08d9efde79c0eec6c20) )

	ROM_LOAD( "136033.149",   0x84000, 0x04000, CRC(b6658f06) SHA1(e719d956f4f9d703a12e2c5520cac0a2f47ea058) )  /* bank 2, plane 0 */
	ROM_LOAD( "136033.151",   0x94000, 0x04000, CRC(84ee1c80) SHA1(5192c0a2887f46b616d130bdbfffbbd5e394e9a3) )  /* bank 2, plane 1 */
	ROM_LOAD( "136033.153",   0xa4000, 0x04000, CRC(daa02926) SHA1(33c7a38c66fb4d67a6ee88ef2da2bba091439e0c) )  /* bank 2, plane 2 */

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136033.118",   0x000, 0x200, CRC(2101b0ed) SHA1(e4fb8dfa80ed78847c697f9de2bd8540b0c04889) )  /* remap */
	ROM_LOAD( "136033.119",   0x200, 0x200, CRC(19f6e767) SHA1(041f24cc03c9043c31c3294c9565dfda9bdada74) )  /* color */

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


ROM_START( marble2 )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136033.401",   0x10000, 0x08000, CRC(ecfc25a2) SHA1(45d98bea8d6bed5ecdcde1049c3a3f6e0fa6ee4c) )
	ROM_LOAD16_BYTE( "136033.402",   0x10001, 0x08000, CRC(7ce9bf53) SHA1(0a5761d4856cd055e8a58b36276945e06c01d08d) )
	ROM_LOAD16_BYTE( "136033.403",   0x20000, 0x08000, CRC(dafee7a2) SHA1(6e4aa9721b5fd4385422c146a84a5e271124abcb) )
	ROM_LOAD16_BYTE( "136033.404",   0x20001, 0x08000, CRC(b59ffcf6) SHA1(814d4c9d8112708abcab64a05ca62d3566ccc67e) )
	ROM_LOAD16_BYTE( "136033.107",   0x80000, 0x04000, CRC(f3b8745b) SHA1(4754eac5e6d8547b3ee00f3f48eaa560eb403862) )
	ROM_LOAD16_BYTE( "136033.108",   0x80001, 0x04000, CRC(e51eecaa) SHA1(37d51a9e9cb33d1156d02a312ac8e202a18d7c20) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136033.421",   0x8000, 0x4000, CRC(78153dc3) SHA1(d4e68226b87df8834dc3d6daa9d683f17896c32e) )
	ROM_LOAD( "136033.422",   0xc000, 0x4000, CRC(2e66300e) SHA1(49acb9443c5d2c1016cde7f489deab2575dd82ca) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x100000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136033.137",   0x00000, 0x04000, CRC(7a45f5c1) SHA1(b826a178660ff2e278558e4779586737751dca5e) )  /* bank 1, plane 0 */
	ROM_LOAD( "136033.138",   0x04000, 0x04000, CRC(7e954a88) SHA1(238a913529781f424a6f49c3fba1524684d0cbcf) )
	ROM_LOAD( "136033.139",   0x10000, 0x04000, CRC(1eb1bb5f) SHA1(987a8289fd4be06b6899bb8c620ddfa4c4b966b0) )  /* bank 1, plane 1 */
	ROM_LOAD( "136033.140",   0x14000, 0x04000, CRC(8a82467b) SHA1(25538e98e5fdadfa6de3bd57ee8658c7add0169d) )
	ROM_LOAD( "136033.141",   0x20000, 0x04000, CRC(52448965) SHA1(e2ce22f89304b2d6858d0c61040d8ff2ee33347f) )  /* bank 1, plane 2 */
	ROM_LOAD( "136033.142",   0x24000, 0x04000, CRC(b4a70e4f) SHA1(d8e4d6dbbef3be86558ab23646c77400793a5eb1) )
	ROM_LOAD( "136033.143",   0x30000, 0x04000, CRC(7156e449) SHA1(361e024e1173299d0e6b776a7c1be10767cfab0d) )  /* bank 1, plane 3 */
	ROM_LOAD( "136033.144",   0x34000, 0x04000, CRC(4c3e4c79) SHA1(642e469ce10067502ce3a920066184a9c73a44c5) )
	ROM_LOAD( "136033.145",   0x40000, 0x04000, CRC(9062be7f) SHA1(ae372433da441b69345d67f5e838b8479557517b) )  /* bank 1, plane 4 */
	ROM_LOAD( "136033.146",   0x44000, 0x04000, CRC(14566dca) SHA1(7134c233daba3b2510d1c08d9efde79c0eec6c20) )

	ROM_LOAD( "136033.149",   0x84000, 0x04000, CRC(b6658f06) SHA1(e719d956f4f9d703a12e2c5520cac0a2f47ea058) )  /* bank 2, plane 0 */
	ROM_LOAD( "136033.151",   0x94000, 0x04000, CRC(84ee1c80) SHA1(5192c0a2887f46b616d130bdbfffbbd5e394e9a3) )  /* bank 2, plane 1 */
	ROM_LOAD( "136033.153",   0xa4000, 0x04000, CRC(daa02926) SHA1(33c7a38c66fb4d67a6ee88ef2da2bba091439e0c) )  /* bank 2, plane 2 */

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136033.118",   0x000, 0x200, CRC(2101b0ed) SHA1(e4fb8dfa80ed78847c697f9de2bd8540b0c04889) )  /* remap */
	ROM_LOAD( "136033.119",   0x200, 0x200, CRC(19f6e767) SHA1(041f24cc03c9043c31c3294c9565dfda9bdada74) )  /* color */

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


ROM_START( marble3 )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136033.201",   0x10000, 0x08000, CRC(9395804d) SHA1(7cca2cc85a9678199c7a60c0976f3e0362f8538f) )
	ROM_LOAD16_BYTE( "136033.202",   0x10001, 0x08000, CRC(edd313f5) SHA1(f3ec6f5812287e187026446fe286f257b54c426e) )
	ROM_LOAD16_BYTE( "136033.403",   0x20000, 0x08000, CRC(dafee7a2) SHA1(6e4aa9721b5fd4385422c146a84a5e271124abcb) )
	ROM_LOAD16_BYTE( "136033.204",   0x20001, 0x08000, CRC(4d621731) SHA1(619d74abe750aac61c3425bcb584ffdafdfc809a) )
	ROM_LOAD16_BYTE( "136033.107",   0x80000, 0x04000, CRC(f3b8745b) SHA1(4754eac5e6d8547b3ee00f3f48eaa560eb403862) )
	ROM_LOAD16_BYTE( "136033.108",   0x80001, 0x04000, CRC(e51eecaa) SHA1(37d51a9e9cb33d1156d02a312ac8e202a18d7c20) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136033.121",   0x8000, 0x4000, CRC(73fe2b46) SHA1(09fceb60c831972f544c92a84c6d0cbc2481b9f5) )
	ROM_LOAD( "136033.122",   0xc000, 0x4000, CRC(03bf65c3) SHA1(4f4a4e4b5bd202da2d18c062f585a842082f08db) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x100000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136033.137",   0x00000, 0x04000, CRC(7a45f5c1) SHA1(b826a178660ff2e278558e4779586737751dca5e) )  /* bank 1, plane 0 */
	ROM_LOAD( "136033.138",   0x04000, 0x04000, CRC(7e954a88) SHA1(238a913529781f424a6f49c3fba1524684d0cbcf) )
	ROM_LOAD( "136033.139",   0x10000, 0x04000, CRC(1eb1bb5f) SHA1(987a8289fd4be06b6899bb8c620ddfa4c4b966b0) )  /* bank 1, plane 1 */
	ROM_LOAD( "136033.140",   0x14000, 0x04000, CRC(8a82467b) SHA1(25538e98e5fdadfa6de3bd57ee8658c7add0169d) )
	ROM_LOAD( "136033.141",   0x20000, 0x04000, CRC(52448965) SHA1(e2ce22f89304b2d6858d0c61040d8ff2ee33347f) )  /* bank 1, plane 2 */
	ROM_LOAD( "136033.142",   0x24000, 0x04000, CRC(b4a70e4f) SHA1(d8e4d6dbbef3be86558ab23646c77400793a5eb1) )
	ROM_LOAD( "136033.143",   0x30000, 0x04000, CRC(7156e449) SHA1(361e024e1173299d0e6b776a7c1be10767cfab0d) )  /* bank 1, plane 3 */
	ROM_LOAD( "136033.144",   0x34000, 0x04000, CRC(4c3e4c79) SHA1(642e469ce10067502ce3a920066184a9c73a44c5) )
	ROM_LOAD( "136033.145",   0x40000, 0x04000, CRC(9062be7f) SHA1(ae372433da441b69345d67f5e838b8479557517b) )  /* bank 1, plane 4 */
	ROM_LOAD( "136033.146",   0x44000, 0x04000, CRC(14566dca) SHA1(7134c233daba3b2510d1c08d9efde79c0eec6c20) )

	ROM_LOAD( "136033.149",   0x84000, 0x04000, CRC(b6658f06) SHA1(e719d956f4f9d703a12e2c5520cac0a2f47ea058) )  /* bank 2, plane 0 */
	ROM_LOAD( "136033.151",   0x94000, 0x04000, CRC(84ee1c80) SHA1(5192c0a2887f46b616d130bdbfffbbd5e394e9a3) )  /* bank 2, plane 1 */
	ROM_LOAD( "136033.153",   0xa4000, 0x04000, CRC(daa02926) SHA1(33c7a38c66fb4d67a6ee88ef2da2bba091439e0c) )  /* bank 2, plane 2 */

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136033.118",   0x000, 0x200, CRC(2101b0ed) SHA1(e4fb8dfa80ed78847c697f9de2bd8540b0c04889) )  /* remap */
	ROM_LOAD( "136033.119",   0x200, 0x200, CRC(19f6e767) SHA1(041f24cc03c9043c31c3294c9565dfda9bdada74) )  /* color */

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


ROM_START( marble4 )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136033.323",   0x10000, 0x04000, CRC(4dc2987a) SHA1(abe155f773dfa623aee6c3952941426adb4c8c9c) )
	ROM_LOAD16_BYTE( "136033.324",   0x10001, 0x04000, CRC(e22e6e11) SHA1(aedb18f25aad846b82690c70b917e97cdcfbd09e) )
	ROM_LOAD16_BYTE( "136033.225",   0x18000, 0x04000, CRC(743f6c5c) SHA1(b69b7dc9923f514b8de207895bbe0c2038dc519d) )
	ROM_LOAD16_BYTE( "136033.226",   0x18001, 0x04000, CRC(aeb711e3) SHA1(0fb0b55e7bd96d993b227db1ab64abbce5026a90) )
	ROM_LOAD16_BYTE( "136033.227",   0x20000, 0x04000, CRC(d06d2c22) SHA1(e75437e383a350b44e4bfe8b2aee80b7b49d1065) )
	ROM_LOAD16_BYTE( "136033.228",   0x20001, 0x04000, CRC(e69cec16) SHA1(d65ce830582dc780f5a73fda6a8a607b04de42b2) )
	ROM_LOAD16_BYTE( "136033.229",   0x28000, 0x04000, CRC(c81d5c14) SHA1(0464ea183685de83e797b9d946b4acc409f4c451) )
	ROM_LOAD16_BYTE( "136033.230",   0x28001, 0x04000, CRC(526ce8ad) SHA1(14f34d763a8a02df7488bfdd1677ec012ad316cd) )
	ROM_LOAD16_BYTE( "136033.107",   0x80000, 0x04000, CRC(f3b8745b) SHA1(4754eac5e6d8547b3ee00f3f48eaa560eb403862) )
	ROM_LOAD16_BYTE( "136033.108",   0x80001, 0x04000, CRC(e51eecaa) SHA1(37d51a9e9cb33d1156d02a312ac8e202a18d7c20) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136033.257",   0x8000, 0x4000, CRC(2e2e0df8) SHA1(e76f7297a3e78ebbc00e3a4c468149f9f7124b16) )
	ROM_LOAD( "136033.258",   0xc000, 0x4000, CRC(1b9655cd) SHA1(deb8728ee9620718e449f1e55a6fe115256c00c9) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x100000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136033.137",   0x00000, 0x04000, CRC(7a45f5c1) SHA1(b826a178660ff2e278558e4779586737751dca5e) )  /* bank 1, plane 0 */
	ROM_LOAD( "136033.138",   0x04000, 0x04000, CRC(7e954a88) SHA1(238a913529781f424a6f49c3fba1524684d0cbcf) )
	ROM_LOAD( "136033.139",   0x10000, 0x04000, CRC(1eb1bb5f) SHA1(987a8289fd4be06b6899bb8c620ddfa4c4b966b0) )  /* bank 1, plane 1 */
	ROM_LOAD( "136033.140",   0x14000, 0x04000, CRC(8a82467b) SHA1(25538e98e5fdadfa6de3bd57ee8658c7add0169d) )
	ROM_LOAD( "136033.141",   0x20000, 0x04000, CRC(52448965) SHA1(e2ce22f89304b2d6858d0c61040d8ff2ee33347f) )  /* bank 1, plane 2 */
	ROM_LOAD( "136033.142",   0x24000, 0x04000, CRC(b4a70e4f) SHA1(d8e4d6dbbef3be86558ab23646c77400793a5eb1) )
	ROM_LOAD( "136033.143",   0x30000, 0x04000, CRC(7156e449) SHA1(361e024e1173299d0e6b776a7c1be10767cfab0d) )  /* bank 1, plane 3 */
	ROM_LOAD( "136033.144",   0x34000, 0x04000, CRC(4c3e4c79) SHA1(642e469ce10067502ce3a920066184a9c73a44c5) )
	ROM_LOAD( "136033.145",   0x40000, 0x04000, CRC(9062be7f) SHA1(ae372433da441b69345d67f5e838b8479557517b) )  /* bank 1, plane 4 */
	ROM_LOAD( "136033.146",   0x44000, 0x04000, CRC(14566dca) SHA1(7134c233daba3b2510d1c08d9efde79c0eec6c20) )

	ROM_LOAD( "136033.149",   0x84000, 0x04000, CRC(b6658f06) SHA1(e719d956f4f9d703a12e2c5520cac0a2f47ea058) )  /* bank 2, plane 0 */
	ROM_LOAD( "136033.151",   0x94000, 0x04000, CRC(84ee1c80) SHA1(5192c0a2887f46b616d130bdbfffbbd5e394e9a3) )  /* bank 2, plane 1 */
	ROM_LOAD( "136033.153",   0xa4000, 0x04000, CRC(daa02926) SHA1(33c7a38c66fb4d67a6ee88ef2da2bba091439e0c) )  /* bank 2, plane 2 */

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136033.118",   0x000, 0x200, CRC(2101b0ed) SHA1(e4fb8dfa80ed78847c697f9de2bd8540b0c04889) )  /* remap */
	ROM_LOAD( "136033.119",   0x200, 0x200, CRC(19f6e767) SHA1(041f24cc03c9043c31c3294c9565dfda9bdada74) )  /* color */

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


ROM_START( marble5 ) /* LSI Cartridge */
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136033.201", 0x10000, 0x08000, CRC(9395804d) SHA1(7cca2cc85a9678199c7a60c0976f3e0362f8538f) ) /* Located at B10 */
	ROM_LOAD16_BYTE( "136033.202", 0x10001, 0x08000, CRC(edd313f5) SHA1(f3ec6f5812287e187026446fe286f257b54c426e) ) /* Located at A10 */
	ROM_LOAD16_BYTE( "136033.203", 0x20000, 0x08000, CRC(dafee7a2) SHA1(6e4aa9721b5fd4385422c146a84a5e271124abcb) ) /* Located at B12 */
	ROM_LOAD16_BYTE( "136033.204", 0x20001, 0x08000, CRC(4d621731) SHA1(619d74abe750aac61c3425bcb584ffdafdfc809a) ) /* Located at A12 */
	ROM_LOAD16_BYTE( "136033.107", 0x80000, 0x04000, CRC(f3b8745b) SHA1(4754eac5e6d8547b3ee00f3f48eaa560eb403862) ) /* Located at B16 */
	ROM_LOAD16_BYTE( "136033.108", 0x80001, 0x04000, CRC(e51eecaa) SHA1(37d51a9e9cb33d1156d02a312ac8e202a18d7c20) ) /* Located at A16 */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD(        "136033.121", 0x08000, 0x04000, CRC(73fe2b46) SHA1(09fceb60c831972f544c92a84c6d0cbc2481b9f5) ) /* Located at D14 */
	ROM_LOAD(        "136033.122", 0x0c000, 0x04000, CRC(03bf65c3) SHA1(4f4a4e4b5bd202da2d18c062f585a842082f08db) ) /* Located at D16 */

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x100000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD(        "136033.109", 0x00000, 0x08000, CRC(467208f4) SHA1(a050991bb684fa51720c0ac33b9bf68a373e8856) )  /* bank 1, plane 0 - located at B1 */
	ROM_LOAD(        "136033.110", 0x10000, 0x08000, CRC(b883ec76) SHA1(ec048e19395fd617f2cd0d3edea728799bdca037) )  /* bank 1, plane 1 - located at B6 */
	ROM_LOAD(        "136033.111", 0x20000, 0x08000, CRC(c208bd5e) SHA1(e75f7d33c66b58e3c72a83d0dc7a6427a57d0de9) )  /* bank 1, plane 2 - located at C1 */
	ROM_LOAD(        "136033.112", 0x30000, 0x08000, CRC(042673d4) SHA1(499c1edf82834d5ff6144ee4349b2349db7a57c2) )  /* bank 1, plane 3 - located at C6 */
	ROM_LOAD(        "136033.113", 0x40000, 0x08000, CRC(b390aef3) SHA1(2cd58071d1c09058af7876c35484c3b571774a06) )  /* bank 1, plane 4 - located at D1 */
	ROM_LOAD(        "136033.115", 0x84000, 0x04000, CRC(b6658f06) SHA1(e719d956f4f9d703a12e2c5520cac0a2f47ea058) )  /* bank 2, plane 0 - located at B2 */
	ROM_LOAD(        "136033.116", 0x94000, 0x04000, CRC(84ee1c80) SHA1(5192c0a2887f46b616d130bdbfffbbd5e394e9a3) )  /* bank 2, plane 1 - located at B7 */
	ROM_LOAD(        "136033.117", 0xa4000, 0x04000, CRC(daa02926) SHA1(33c7a38c66fb4d67a6ee88ef2da2bba091439e0c) )  /* bank 2, plane 2 - located at C7 */

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD(        "136033.118", 0x00000, 0x00200, CRC(2101b0ed) SHA1(e4fb8dfa80ed78847c697f9de2bd8540b0c04889) )  /* remap, located at A7 */
	ROM_LOAD(        "136033.159", 0x00200, 0x00200, CRC(19f6e767) SHA1(041f24cc03c9043c31c3294c9565dfda9bdada74) )  /* color, located at A5 */

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


/*
    Peter Pack-Rat
*/

ROM_START( peterpak )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136028.142",   0x10000, 0x04000, CRC(4f9fc020) SHA1(7e0d5660ea395ded7e0e4c9122bc41f9bb9a483d) )
	ROM_LOAD16_BYTE( "136028.143",   0x10001, 0x04000, CRC(9fb257cc) SHA1(5590fc2b4e79a071ae5ba04e23aadb3e35614a73) )
	ROM_LOAD16_BYTE( "136028.144",   0x18000, 0x04000, CRC(50267619) SHA1(7aec7e4c8b97313d18c8eb55e078157776e29842) )
	ROM_LOAD16_BYTE( "136028.145",   0x18001, 0x04000, CRC(7b6a5004) SHA1(f1fa069cb3148ba38722932f96f72098c800468a) )
	ROM_LOAD16_BYTE( "136028.146",   0x20000, 0x04000, CRC(4183a67a) SHA1(2c86701ab9f17e0bd0e2de5b9d802d6c1777bdd0) )
	ROM_LOAD16_BYTE( "136028.147",   0x20001, 0x04000, CRC(14e2d97b) SHA1(eb802e1c198a55d08eb52e9438dde8d23a14d9a4) )
	ROM_LOAD16_BYTE( "136028.148",   0x80000, 0x04000, CRC(230e8ba9) SHA1(decda85f3de92ab72bb2a3812a84a92c937a7409) )
	ROM_LOAD16_BYTE( "136028.149",   0x80001, 0x04000, CRC(0ff0c13a) SHA1(d1912d8e468ca04a930aedaece824f020d8e0357) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136028.101",   0x8000, 0x4000, CRC(ff712aa2) SHA1(07beeb51f41d406782965dddb6fe73fdc49529df) )
	ROM_LOAD( "136028.102",   0xc000, 0x4000, CRC(89ea21a1) SHA1(a7cb6eceb45d54406d6fc692f50a4e34564d6170) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x180000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136028.138",   0x000000, 0x08000, CRC(53eaa018) SHA1(3f9887d4996e6a7c894732e9c9ebe4408fb60192) )  /* bank 1, plane 0 */
	ROM_LOAD( "136028.139",   0x010000, 0x08000, CRC(354a19cb) SHA1(84a145d0f1624315923d389b4b746db5fbc672df) )  /* bank 1, plane 1 */
	ROM_LOAD( "136028.140",   0x020000, 0x08000, CRC(8d2c4717) SHA1(d7709d5859b5276d46799ff39a375929a48e34ae) )  /* bank 1, plane 2 */
	ROM_LOAD( "136028.141",   0x030000, 0x08000, CRC(bf59ea19) SHA1(2abef43de94845992f2a14ea66838351bdce9d85) )  /* bank 1, plane 3 */

	ROM_LOAD( "136028.150",   0x080000, 0x08000, CRC(83362483) SHA1(56bbf8d416ab0f6ee3c6593323a4a53246827ee0) )  /* bank 2, plane 0 */
	ROM_LOAD( "136028.151",   0x090000, 0x08000, CRC(6e95094e) SHA1(7b42398abf1bdf9790fd1abe12acf6145a4a71a8) )  /* bank 2, plane 1 */
	ROM_LOAD( "136028.152",   0x0a0000, 0x08000, CRC(9553f084) SHA1(0b6bdab2f36b6796c220765c035068a9f4e64b12) )  /* bank 2, plane 2 */
	ROM_LOAD( "136028.153",   0x0b0000, 0x08000, CRC(c2a9b028) SHA1(1e436462761fa791a1f15ba1f280f339cb0f3a29) )  /* bank 2, plane 3 */

	ROM_LOAD( "136028.105",   0x104000, 0x04000, CRC(ac9a5a44) SHA1(c5cdd09435ce76a4e98d858864becca33ed7a969) )  /* bank 3, plane 0 */
	ROM_LOAD( "136028.108",   0x114000, 0x04000, CRC(51941e64) SHA1(eb04d37a70d5362e947e3a6d53facc88e09b85bb) )  /* bank 3, plane 1 */
	ROM_LOAD( "136028.111",   0x124000, 0x04000, CRC(246599f3) SHA1(7902941ce29a784875c17a8eadb50288f1497604) )  /* bank 3, plane 2 */
	ROM_LOAD( "136028.114",   0x134000, 0x04000, CRC(918a5082) SHA1(0a9b83e0c9f6883bdca1b84805f8642fc660d09d) )  /* bank 3, plane 3 */

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136028.136",   0x000, 0x200, CRC(861cfa36) SHA1(d18ca5e28bf48df6506be6bc0283c996c6520ef4) )  /* remap */
	ROM_LOAD( "136028.137",   0x200, 0x200, CRC(8507e5ea) SHA1(a009a98fe02625a20f4a9d9ab1c70891bf4e45ec) )  /* color */

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


/*
    Indiana Jones and the Temple of Doom
*/

ROM_START( indytemp )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136036.432",   0x10000, 0x08000, CRC(d888cdf1) SHA1(c9cbc0d429755611b3749e018d3817d4d7042425) )
	ROM_LOAD16_BYTE( "136036.431",   0x10001, 0x08000, CRC(b7ac7431) SHA1(b062258746ca5f9c3f9cf8a6186eb3c370ac3a77) )
	ROM_LOAD16_BYTE( "136036.434",   0x20000, 0x08000, CRC(802495fd) SHA1(228f48ef067a758fe0d3bfe0508765a52bb579eb) )
	ROM_LOAD16_BYTE( "136036.433",   0x20001, 0x08000, CRC(3a914e5c) SHA1(af2e03ad8afa7fb262c145b73ddfec9cbab537a8) )
	ROM_LOAD16_BYTE( "136036.456",   0x30000, 0x04000, CRC(ec146b09) SHA1(302522f202154e0b56e25c881ec7541cdf179a6c) )
	ROM_LOAD16_BYTE( "136036.457",   0x30001, 0x04000, CRC(6628de01) SHA1(c951b5f37cf5f2c6c2b45ef1b75caa1e08cc6755) )
	ROM_LOAD16_BYTE( "136036.358",   0x80000, 0x04000, CRC(d9351106) SHA1(6a11cef7f93701d5bc08e0ce413999322a011d69) )
	ROM_LOAD16_BYTE( "136036.359",   0x80001, 0x04000, CRC(e731caea) SHA1(71c1445804b5696078a49855852420e4046d77bb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136036.153",   0x4000, 0x4000, CRC(95294641) SHA1(00f90a0d49d2c77d5288080036f81a74ad31f8bc) )
	ROM_LOAD( "136036.154",   0x8000, 0x4000, CRC(cbfc6adb) SHA1(ee132eced924435f1214e2997533e866a3e5364b) )
	ROM_LOAD( "136036.155",   0xc000, 0x4000, CRC(4c8233ac) SHA1(3d2bdb71b8f499a21f0b0e1686be7fe6f23efede) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x200000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136036.135",   0x000000, 0x08000, CRC(ffa8749c) SHA1(b0bec9b8364a15606d9f4d11f674dea2ec25279f) )  /* bank 1, plane 0 */
	ROM_LOAD( "136036.139",   0x010000, 0x08000, CRC(b682bfca) SHA1(21625ea29609560b31b28c669b2599cfcdab8ea7) )  /* bank 1, plane 1 */
	ROM_LOAD( "136036.143",   0x020000, 0x08000, CRC(7697da26) SHA1(546b95467ca61fab9ac853635c8972a2dedb6581) )  /* bank 1, plane 2 */
	ROM_LOAD( "136036.147",   0x030000, 0x08000, CRC(4e9d664c) SHA1(334c5f9e96e7a6fbf375bca58bd58fb18a68da6f) )  /* bank 1, plane 3 */

	ROM_LOAD( "136036.136",   0x080000, 0x08000, CRC(b2b403aa) SHA1(ead88544d798d07c03fefadf84c87ed104036cfa) )  /* bank 2, plane 0 */
	ROM_LOAD( "136036.140",   0x090000, 0x08000, CRC(ec0c19ca) SHA1(a787502e9db79a3c601964cd8c662ff7d0c2f636) )  /* bank 2, plane 1 */
	ROM_LOAD( "136036.144",   0x0a0000, 0x08000, CRC(4407df98) SHA1(67eae09121e594bec965b78c44a964a39a2c8772) )  /* bank 2, plane 2 */
	ROM_LOAD( "136036.148",   0x0b0000, 0x08000, CRC(70dce06d) SHA1(c50aa9ea9ea70e282ebed54cefa9e51da01d14ca) )  /* bank 2, plane 3 */

	ROM_LOAD( "136036.137",   0x100000, 0x08000, CRC(3f352547) SHA1(2a3439bb1ca9cdb80c0bd61cbd5fce60d6ee1fef) )  /* bank 3, plane 0 */
	ROM_LOAD( "136036.141",   0x110000, 0x08000, CRC(9cbdffd0) SHA1(ce36e7b9792e41d87f97901b8d0a473835017ae6) )  /* bank 3, plane 1 */
	ROM_LOAD( "136036.145",   0x120000, 0x08000, CRC(e828e64b) SHA1(8ad19ee40e6c4caa3a102f967fe93e823c766b5a) )  /* bank 3, plane 2 */
	ROM_LOAD( "136036.149",   0x130000, 0x08000, CRC(81503a23) SHA1(9a28ba78f8029395542067f03460d15cbcd7b315) )  /* bank 3, plane 3 */

	ROM_LOAD( "136036.138",   0x180000, 0x08000, CRC(48c4d79d) SHA1(c4cd258cd1effbfd62e3270b626271db66095dcb) )  /* bank 4, plane 0 */
	ROM_LOAD( "136036.142",   0x190000, 0x08000, CRC(7faae75f) SHA1(e3c3f77aa9ec91d4fcdd2fc5f0a22063eec1afe2) )  /* bank 4, plane 1 */
	ROM_LOAD( "136036.146",   0x1a0000, 0x08000, CRC(8ae5a7b5) SHA1(97ddf916c41d6ddd23501e27611a83c912ec9ad2) )  /* bank 4, plane 2 */
	ROM_LOAD( "136036.150",   0x1b0000, 0x08000, CRC(a10c4bd9) SHA1(3177f22aff9e18c0f003f3fd1fa4b258308eee07) )  /* bank 4, plane 3 */

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136036.152",   0x000, 0x200, CRC(4f96e57c) SHA1(271633a0aacd1d1efe2917728b73e90010c64d2c) )  /* remap */
	ROM_LOAD( "136036.151",   0x200, 0x200, CRC(7daf351f) SHA1(95c13d81a47440f847af7b19632cc032380b9ff4) )  /* color */

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


ROM_START( indytemp2 )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136036.470",   0x10000, 0x08000, CRC(7fac1dd8) SHA1(043daf709209e76115899a1e301f8fb262b3a749) )
	ROM_LOAD16_BYTE( "136036.471",   0x10001, 0x08000, CRC(e93272fb) SHA1(bb5741419a9d1a54ae16bfd6d2fcf64627e8a5c3) )
	ROM_LOAD16_BYTE( "136036.434",   0x20000, 0x08000, CRC(802495fd) SHA1(228f48ef067a758fe0d3bfe0508765a52bb579eb) )
	ROM_LOAD16_BYTE( "136036.433",   0x20001, 0x08000, CRC(3a914e5c) SHA1(af2e03ad8afa7fb262c145b73ddfec9cbab537a8) )
	ROM_LOAD16_BYTE( "136036.456",   0x30000, 0x04000, CRC(ec146b09) SHA1(302522f202154e0b56e25c881ec7541cdf179a6c) )
	ROM_LOAD16_BYTE( "136036.457",   0x30001, 0x04000, CRC(6628de01) SHA1(c951b5f37cf5f2c6c2b45ef1b75caa1e08cc6755) )
	ROM_LOAD16_BYTE( "136036.358",   0x80000, 0x04000, CRC(d9351106) SHA1(6a11cef7f93701d5bc08e0ce413999322a011d69) )
	ROM_LOAD16_BYTE( "136036.359",   0x80001, 0x04000, CRC(e731caea) SHA1(71c1445804b5696078a49855852420e4046d77bb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136036.153",   0x4000, 0x4000, CRC(95294641) SHA1(00f90a0d49d2c77d5288080036f81a74ad31f8bc) )
	ROM_LOAD( "136036.154",   0x8000, 0x4000, CRC(cbfc6adb) SHA1(ee132eced924435f1214e2997533e866a3e5364b) )
	ROM_LOAD( "136036.155",   0xc000, 0x4000, CRC(4c8233ac) SHA1(3d2bdb71b8f499a21f0b0e1686be7fe6f23efede) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x200000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136036.135",   0x000000, 0x08000, CRC(ffa8749c) SHA1(b0bec9b8364a15606d9f4d11f674dea2ec25279f) )  /* bank 1, plane 0 */
	ROM_LOAD( "136036.139",   0x010000, 0x08000, CRC(b682bfca) SHA1(21625ea29609560b31b28c669b2599cfcdab8ea7) )  /* bank 1, plane 1 */
	ROM_LOAD( "136036.143",   0x020000, 0x08000, CRC(7697da26) SHA1(546b95467ca61fab9ac853635c8972a2dedb6581) )  /* bank 1, plane 2 */
	ROM_LOAD( "136036.147",   0x030000, 0x08000, CRC(4e9d664c) SHA1(334c5f9e96e7a6fbf375bca58bd58fb18a68da6f) )  /* bank 1, plane 3 */

	ROM_LOAD( "136036.136",   0x080000, 0x08000, CRC(b2b403aa) SHA1(ead88544d798d07c03fefadf84c87ed104036cfa) )  /* bank 2, plane 0 */
	ROM_LOAD( "136036.140",   0x090000, 0x08000, CRC(ec0c19ca) SHA1(a787502e9db79a3c601964cd8c662ff7d0c2f636) )  /* bank 2, plane 1 */
	ROM_LOAD( "136036.144",   0x0a0000, 0x08000, CRC(4407df98) SHA1(67eae09121e594bec965b78c44a964a39a2c8772) )  /* bank 2, plane 2 */
	ROM_LOAD( "136036.148",   0x0b0000, 0x08000, CRC(70dce06d) SHA1(c50aa9ea9ea70e282ebed54cefa9e51da01d14ca) )  /* bank 2, plane 3 */

	ROM_LOAD( "136036.137",   0x100000, 0x08000, CRC(3f352547) SHA1(2a3439bb1ca9cdb80c0bd61cbd5fce60d6ee1fef) )  /* bank 3, plane 0 */
	ROM_LOAD( "136036.141",   0x110000, 0x08000, CRC(9cbdffd0) SHA1(ce36e7b9792e41d87f97901b8d0a473835017ae6) )  /* bank 3, plane 1 */
	ROM_LOAD( "136036.145",   0x120000, 0x08000, CRC(e828e64b) SHA1(8ad19ee40e6c4caa3a102f967fe93e823c766b5a) )  /* bank 3, plane 2 */
	ROM_LOAD( "136036.149",   0x130000, 0x08000, CRC(81503a23) SHA1(9a28ba78f8029395542067f03460d15cbcd7b315) )  /* bank 3, plane 3 */

	ROM_LOAD( "136036.138",   0x180000, 0x08000, CRC(48c4d79d) SHA1(c4cd258cd1effbfd62e3270b626271db66095dcb) )  /* bank 4, plane 0 */
	ROM_LOAD( "136036.142",   0x190000, 0x08000, CRC(7faae75f) SHA1(e3c3f77aa9ec91d4fcdd2fc5f0a22063eec1afe2) )  /* bank 4, plane 1 */
	ROM_LOAD( "136036.146",   0x1a0000, 0x08000, CRC(8ae5a7b5) SHA1(97ddf916c41d6ddd23501e27611a83c912ec9ad2) )  /* bank 4, plane 2 */
	ROM_LOAD( "136036.150",   0x1b0000, 0x08000, CRC(a10c4bd9) SHA1(3177f22aff9e18c0f003f3fd1fa4b258308eee07) )  /* bank 4, plane 3 */

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136036.152",   0x000, 0x200, CRC(4f96e57c) SHA1(271633a0aacd1d1efe2917728b73e90010c64d2c) )  /* remap */
	ROM_LOAD( "136036.151",   0x200, 0x200, CRC(7daf351f) SHA1(95c13d81a47440f847af7b19632cc032380b9ff4) )  /* color */

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


ROM_START( indytemp3 )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "232.10b",      0x10000, 0x08000, CRC(1e80108f) SHA1(663ef81b865c4d13fd73eca4f82ee64a4bdeaad5) )
	ROM_LOAD16_BYTE( "231.10a",      0x10001, 0x08000, CRC(8ae54c0c) SHA1(c7ab4a2e2af8f2336baa8c7b2a80af7c9bfd0435) )
	ROM_LOAD16_BYTE( "234.12b",      0x20000, 0x08000, CRC(86be7e07) SHA1(b6b0d9c6d81d446dab675c71d60f792f6a657c5b) )
	ROM_LOAD16_BYTE( "233.12a",      0x20001, 0x08000, CRC(bfcea7ae) SHA1(b5c93d38ccadb52cd55df96151b7c953d546176b) )
	ROM_LOAD16_BYTE( "256.15b",      0x30000, 0x04000, CRC(3a076fd2) SHA1(82186975237e2e7157cb62bf2cb03439d097e136) )
	ROM_LOAD16_BYTE( "257.15a",      0x30001, 0x04000, CRC(15293606) SHA1(80025378746a0c1281219a4b79723b58d2b553b8) )
	ROM_LOAD16_BYTE( "158.16b",      0x80000, 0x04000, CRC(10372888) SHA1(438a990dc7599325f203d57ffb0a94ae36a00cc7) )
	ROM_LOAD16_BYTE( "159.16a",      0x80001, 0x04000, CRC(50f890a8) SHA1(7dfdc4512de8fb0039b426e3b5be50e0095f39f7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136036.153",   0x4000, 0x4000, CRC(95294641) SHA1(00f90a0d49d2c77d5288080036f81a74ad31f8bc) )
	ROM_LOAD( "136036.154",   0x8000, 0x4000, CRC(cbfc6adb) SHA1(ee132eced924435f1214e2997533e866a3e5364b) )
	ROM_LOAD( "136036.155",   0xc000, 0x4000, CRC(4c8233ac) SHA1(3d2bdb71b8f499a21f0b0e1686be7fe6f23efede) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x200000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136036.135",   0x000000, 0x08000, CRC(ffa8749c) SHA1(b0bec9b8364a15606d9f4d11f674dea2ec25279f) )  /* bank 1, plane 0 */
	ROM_LOAD( "136036.139",   0x010000, 0x08000, CRC(b682bfca) SHA1(21625ea29609560b31b28c669b2599cfcdab8ea7) )  /* bank 1, plane 1 */
	ROM_LOAD( "136036.143",   0x020000, 0x08000, CRC(7697da26) SHA1(546b95467ca61fab9ac853635c8972a2dedb6581) )  /* bank 1, plane 2 */
	ROM_LOAD( "136036.147",   0x030000, 0x08000, CRC(4e9d664c) SHA1(334c5f9e96e7a6fbf375bca58bd58fb18a68da6f) )  /* bank 1, plane 3 */

	ROM_LOAD( "136036.136",   0x080000, 0x08000, CRC(b2b403aa) SHA1(ead88544d798d07c03fefadf84c87ed104036cfa) )  /* bank 2, plane 0 */
	ROM_LOAD( "136036.140",   0x090000, 0x08000, CRC(ec0c19ca) SHA1(a787502e9db79a3c601964cd8c662ff7d0c2f636) )  /* bank 2, plane 1 */
	ROM_LOAD( "136036.144",   0x0a0000, 0x08000, CRC(4407df98) SHA1(67eae09121e594bec965b78c44a964a39a2c8772) )  /* bank 2, plane 2 */
	ROM_LOAD( "136036.148",   0x0b0000, 0x08000, CRC(70dce06d) SHA1(c50aa9ea9ea70e282ebed54cefa9e51da01d14ca) )  /* bank 2, plane 3 */

	ROM_LOAD( "136036.137",   0x100000, 0x08000, CRC(3f352547) SHA1(2a3439bb1ca9cdb80c0bd61cbd5fce60d6ee1fef) )  /* bank 3, plane 0 */
	ROM_LOAD( "136036.141",   0x110000, 0x08000, CRC(9cbdffd0) SHA1(ce36e7b9792e41d87f97901b8d0a473835017ae6) )  /* bank 3, plane 1 */
	ROM_LOAD( "136036.145",   0x120000, 0x08000, CRC(e828e64b) SHA1(8ad19ee40e6c4caa3a102f967fe93e823c766b5a) )  /* bank 3, plane 2 */
	ROM_LOAD( "136036.149",   0x130000, 0x08000, CRC(81503a23) SHA1(9a28ba78f8029395542067f03460d15cbcd7b315) )  /* bank 3, plane 3 */

	ROM_LOAD( "136036.138",   0x180000, 0x08000, CRC(48c4d79d) SHA1(c4cd258cd1effbfd62e3270b626271db66095dcb) )  /* bank 4, plane 0 */
	ROM_LOAD( "136036.142",   0x190000, 0x08000, CRC(7faae75f) SHA1(e3c3f77aa9ec91d4fcdd2fc5f0a22063eec1afe2) )  /* bank 4, plane 1 */
	ROM_LOAD( "136036.146",   0x1a0000, 0x08000, CRC(8ae5a7b5) SHA1(97ddf916c41d6ddd23501e27611a83c912ec9ad2) )  /* bank 4, plane 2 */
	ROM_LOAD( "136036.150",   0x1b0000, 0x08000, CRC(a10c4bd9) SHA1(3177f22aff9e18c0f003f3fd1fa4b258308eee07) )  /* bank 4, plane 3 */

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136036.152",   0x000, 0x200, CRC(4f96e57c) SHA1(271633a0aacd1d1efe2917728b73e90010c64d2c) )  /* remap */
	ROM_LOAD( "136036.151",   0x200, 0x200, CRC(7daf351f) SHA1(95c13d81a47440f847af7b19632cc032380b9ff4) )  /* color */

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


ROM_START( indytemp4 )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136036.332",   0x10000, 0x08000, CRC(a5563773) SHA1(52701b53e62aae691f7b9483f5e843d805223a0a) )
	ROM_LOAD16_BYTE( "136036.331",   0x10001, 0x08000, CRC(7d562141) SHA1(a2143ed473e40a5a46679e7751fc37475768885f) )
	ROM_LOAD16_BYTE( "136036.334",   0x20000, 0x08000, CRC(e40828e5) SHA1(a5c50693bddcc394711859bdc36d9ff93d02b82e) )
	ROM_LOAD16_BYTE( "136036.333",   0x20001, 0x08000, CRC(96e1f1aa) SHA1(16495a8388492cd67f0cfafa53a1291c07fa3cba) )
	ROM_LOAD16_BYTE( "136036.356",   0x30000, 0x04000, CRC(5eba2ac7) SHA1(1ce5cb5f8e5d735aa55944712c850a0fbf2c7f30) )
	ROM_LOAD16_BYTE( "136036.357",   0x30001, 0x04000, CRC(26e84b5c) SHA1(18a18c38aea90940e0fa6cc99d3cf8a6eb6e8378) )
	ROM_LOAD16_BYTE( "136036.358",   0x80000, 0x04000, CRC(d9351106) SHA1(6a11cef7f93701d5bc08e0ce413999322a011d69) )
	ROM_LOAD16_BYTE( "136036.359",   0x80001, 0x04000, CRC(e731caea) SHA1(71c1445804b5696078a49855852420e4046d77bb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136036.153",   0x4000, 0x4000, CRC(95294641) SHA1(00f90a0d49d2c77d5288080036f81a74ad31f8bc) )
	ROM_LOAD( "136036.154",   0x8000, 0x4000, CRC(cbfc6adb) SHA1(ee132eced924435f1214e2997533e866a3e5364b) )
	ROM_LOAD( "136036.155",   0xc000, 0x4000, CRC(4c8233ac) SHA1(3d2bdb71b8f499a21f0b0e1686be7fe6f23efede) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x200000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136036.135",   0x000000, 0x08000, CRC(ffa8749c) SHA1(b0bec9b8364a15606d9f4d11f674dea2ec25279f) )  /* bank 1, plane 0 */
	ROM_LOAD( "136036.139",   0x010000, 0x08000, CRC(b682bfca) SHA1(21625ea29609560b31b28c669b2599cfcdab8ea7) )  /* bank 1, plane 1 */
	ROM_LOAD( "136036.143",   0x020000, 0x08000, CRC(7697da26) SHA1(546b95467ca61fab9ac853635c8972a2dedb6581) )  /* bank 1, plane 2 */
	ROM_LOAD( "136036.147",   0x030000, 0x08000, CRC(4e9d664c) SHA1(334c5f9e96e7a6fbf375bca58bd58fb18a68da6f) )  /* bank 1, plane 3 */

	ROM_LOAD( "136036.136",   0x080000, 0x08000, CRC(b2b403aa) SHA1(ead88544d798d07c03fefadf84c87ed104036cfa) )  /* bank 2, plane 0 */
	ROM_LOAD( "136036.140",   0x090000, 0x08000, CRC(ec0c19ca) SHA1(a787502e9db79a3c601964cd8c662ff7d0c2f636) )  /* bank 2, plane 1 */
	ROM_LOAD( "136036.144",   0x0a0000, 0x08000, CRC(4407df98) SHA1(67eae09121e594bec965b78c44a964a39a2c8772) )  /* bank 2, plane 2 */
	ROM_LOAD( "136036.148",   0x0b0000, 0x08000, CRC(70dce06d) SHA1(c50aa9ea9ea70e282ebed54cefa9e51da01d14ca) )  /* bank 2, plane 3 */

	ROM_LOAD( "136036.137",   0x100000, 0x08000, CRC(3f352547) SHA1(2a3439bb1ca9cdb80c0bd61cbd5fce60d6ee1fef) )  /* bank 3, plane 0 */
	ROM_LOAD( "136036.141",   0x110000, 0x08000, CRC(9cbdffd0) SHA1(ce36e7b9792e41d87f97901b8d0a473835017ae6) )  /* bank 3, plane 1 */
	ROM_LOAD( "136036.145",   0x120000, 0x08000, CRC(e828e64b) SHA1(8ad19ee40e6c4caa3a102f967fe93e823c766b5a) )  /* bank 3, plane 2 */
	ROM_LOAD( "136036.149",   0x130000, 0x08000, CRC(81503a23) SHA1(9a28ba78f8029395542067f03460d15cbcd7b315) )  /* bank 3, plane 3 */

	ROM_LOAD( "136036.138",   0x180000, 0x08000, CRC(48c4d79d) SHA1(c4cd258cd1effbfd62e3270b626271db66095dcb) )  /* bank 4, plane 0 */
	ROM_LOAD( "136036.142",   0x190000, 0x08000, CRC(7faae75f) SHA1(e3c3f77aa9ec91d4fcdd2fc5f0a22063eec1afe2) )  /* bank 4, plane 1 */
	ROM_LOAD( "136036.146",   0x1a0000, 0x08000, CRC(8ae5a7b5) SHA1(97ddf916c41d6ddd23501e27611a83c912ec9ad2) )  /* bank 4, plane 2 */
	ROM_LOAD( "136036.150",   0x1b0000, 0x08000, CRC(a10c4bd9) SHA1(3177f22aff9e18c0f003f3fd1fa4b258308eee07) )  /* bank 4, plane 3 */

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136036.152",   0x000, 0x200, CRC(4f96e57c) SHA1(271633a0aacd1d1efe2917728b73e90010c64d2c) )  /* remap */
	ROM_LOAD( "136036.151",   0x200, 0x200, CRC(7daf351f) SHA1(95c13d81a47440f847af7b19632cc032380b9ff4) )  /* color */

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


ROM_START( indytempc ) /* Dedicated boardset marked 'ATARI SYSTEM I LSI CARTRIDGE COCKTAIL A043310' */
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	ROM_LOAD16_BYTE( "136032.116",   0x00000, 0x04000, CRC(195c54ad) SHA1(d7cda3cd3db4c6f77074ca05e96ae11b62e048b7) )
	ROM_LOAD16_BYTE( "136032.117",   0x00001, 0x04000, CRC(9af9fe29) SHA1(1d5077662e4111ece9f8a5124394dad8b1abdc13) )
	ROM_LOAD16_BYTE( "136032.632",   0x10000, 0x08000, CRC(d3e1a611) SHA1(edbced6dd64ca44a59aff6a4acca0b3ddb233810) )
	ROM_LOAD16_BYTE( "136032.631",   0x10001, 0x08000, CRC(9ac96ba8) SHA1(d80adabb4198461b5976577bdd8cc4e650de1c8e) )
	ROM_LOAD16_BYTE( "136036.534",   0x20000, 0x08000, CRC(eae396be) SHA1(6d7a82a2fd43b91c6c1e2c07d33c81487b845bba) )
	ROM_LOAD16_BYTE( "136036.533",   0x20001, 0x08000, CRC(06c66335) SHA1(848cd7d8c2d8da4d07ddc908676155e154ae764a) )
	ROM_LOAD16_BYTE( "136036.568",   0x30000, 0x08000, CRC(2bbc16ed) SHA1(f20a07632679641f007caa44feb52272f8a2936d) )
	ROM_LOAD16_BYTE( "136036.569",   0x30001, 0x08000, CRC(39270ade) SHA1(9d2d029eacfe4565972ed40a9f96970f42875608) )
	ROM_LOAD16_BYTE( "136036.358",   0x80000, 0x04000, CRC(d9351106) SHA1(6a11cef7f93701d5bc08e0ce413999322a011d69) )
	ROM_LOAD16_BYTE( "136036.359",   0x80001, 0x04000, CRC(e731caea) SHA1(71c1445804b5696078a49855852420e4046d77bb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136036.153",   0x4000, 0x4000, CRC(95294641) SHA1(00f90a0d49d2c77d5288080036f81a74ad31f8bc) )
	ROM_LOAD( "136036.170",   0x8000, 0x8000, CRC(f318b321) SHA1(8fe9e88fa9f2104526f89926a7119b866051e4ef) )

	ROM_REGION( 0x4000, "alpha", 0 )
	ROM_LOAD( "136032.120",   0x00000, 0x04000, CRC(90a1950d) SHA1(fba32c255850312175d1e3c03d677ffb57e09e07) )  /* alpha font */

	ROM_REGION( 0x200000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136036.135",   0x000000, 0x08000, CRC(ffa8749c) SHA1(b0bec9b8364a15606d9f4d11f674dea2ec25279f) )  /* bank 1, plane 0 */
	ROM_LOAD( "136036.139",   0x010000, 0x08000, CRC(b682bfca) SHA1(21625ea29609560b31b28c669b2599cfcdab8ea7) )  /* bank 1, plane 1 */
	ROM_LOAD( "136036.143",   0x020000, 0x08000, CRC(7697da26) SHA1(546b95467ca61fab9ac853635c8972a2dedb6581) )  /* bank 1, plane 2 */
	ROM_LOAD( "136036.147",   0x030000, 0x08000, CRC(4e9d664c) SHA1(334c5f9e96e7a6fbf375bca58bd58fb18a68da6f) )  /* bank 1, plane 3 */

	ROM_LOAD( "136036.136",   0x080000, 0x08000, CRC(b2b403aa) SHA1(ead88544d798d07c03fefadf84c87ed104036cfa) )  /* bank 2, plane 0 */
	ROM_LOAD( "136036.140",   0x090000, 0x08000, CRC(ec0c19ca) SHA1(a787502e9db79a3c601964cd8c662ff7d0c2f636) )  /* bank 2, plane 1 */
	ROM_LOAD( "136036.144",   0x0a0000, 0x08000, CRC(4407df98) SHA1(67eae09121e594bec965b78c44a964a39a2c8772) )  /* bank 2, plane 2 */
	ROM_LOAD( "136036.148",   0x0b0000, 0x08000, CRC(70dce06d) SHA1(c50aa9ea9ea70e282ebed54cefa9e51da01d14ca) )  /* bank 2, plane 3 */

	ROM_LOAD( "136036.137",   0x100000, 0x08000, CRC(3f352547) SHA1(2a3439bb1ca9cdb80c0bd61cbd5fce60d6ee1fef) )  /* bank 3, plane 0 */
	ROM_LOAD( "136036.141",   0x110000, 0x08000, CRC(9cbdffd0) SHA1(ce36e7b9792e41d87f97901b8d0a473835017ae6) )  /* bank 3, plane 1 */
	ROM_LOAD( "136036.145",   0x120000, 0x08000, CRC(e828e64b) SHA1(8ad19ee40e6c4caa3a102f967fe93e823c766b5a) )  /* bank 3, plane 2 */
	ROM_LOAD( "136036.149",   0x130000, 0x08000, CRC(81503a23) SHA1(9a28ba78f8029395542067f03460d15cbcd7b315) )  /* bank 3, plane 3 */

	ROM_LOAD( "136036.138",   0x180000, 0x08000, CRC(48c4d79d) SHA1(c4cd258cd1effbfd62e3270b626271db66095dcb) )  /* bank 4, plane 0 */
	ROM_LOAD( "136036.142",   0x190000, 0x08000, CRC(7faae75f) SHA1(e3c3f77aa9ec91d4fcdd2fc5f0a22063eec1afe2) )  /* bank 4, plane 1 */
	ROM_LOAD( "136036.146",   0x1a0000, 0x08000, CRC(8ae5a7b5) SHA1(97ddf916c41d6ddd23501e27611a83c912ec9ad2) )  /* bank 4, plane 2 */
	ROM_LOAD( "136036.150",   0x1b0000, 0x08000, CRC(a10c4bd9) SHA1(3177f22aff9e18c0f003f3fd1fa4b258308eee07) )  /* bank 4, plane 3 */

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136036.152",   0x000, 0x200, CRC(4f96e57c) SHA1(271633a0aacd1d1efe2917728b73e90010c64d2c) )  /* remap */
	ROM_LOAD( "136036.160",   0x200, 0x200, CRC(88c65843) SHA1(81fef378b3dbf4d7228beb7427e2f75cae371808) )  /* color */
ROM_END


ROM_START( indytempd )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136036.462",   0x10000, 0x08000, CRC(317dc430) SHA1(563f09fb1b096bd40e1a73acb7f11d3809f9f19f) )
	ROM_LOAD16_BYTE( "136036.461",   0x10001, 0x08000, CRC(8c73f974) SHA1(023b55f1d54606f5c51b86b802d417099d775f14) )
	ROM_LOAD16_BYTE( "136036.464",   0x20000, 0x08000, CRC(3fcb199f) SHA1(11a8a17bf0100b5f16e9148669b2ec5bf81b3d62) )
	ROM_LOAD16_BYTE( "136036.463",   0x20001, 0x08000, CRC(d6bda19a) SHA1(67c928e2b5ae61279f8ca1b580638c196fc16ca1) )
	ROM_LOAD16_BYTE( "136036.466",   0x30000, 0x04000, CRC(faa7f23a) SHA1(0e235387ff100158e925641aaf3ddde5334f76fd) )
	ROM_LOAD16_BYTE( "136036.467",   0x30001, 0x04000, CRC(ee9fd91a) SHA1(63c5bf6137e5a328188914a36479a704a6a46404) )
	ROM_LOAD16_BYTE( "136036.358",   0x80000, 0x04000, CRC(d9351106) SHA1(6a11cef7f93701d5bc08e0ce413999322a011d69) )
	ROM_LOAD16_BYTE( "136036.359",   0x80001, 0x04000, CRC(e731caea) SHA1(71c1445804b5696078a49855852420e4046d77bb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136036.153",   0x4000, 0x4000, CRC(95294641) SHA1(00f90a0d49d2c77d5288080036f81a74ad31f8bc) )
	ROM_LOAD( "136036.154",   0x8000, 0x4000, CRC(cbfc6adb) SHA1(ee132eced924435f1214e2997533e866a3e5364b) )
	ROM_LOAD( "136036.155",   0xc000, 0x4000, CRC(4c8233ac) SHA1(3d2bdb71b8f499a21f0b0e1686be7fe6f23efede) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x200000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136036.135",   0x000000, 0x08000, CRC(ffa8749c) SHA1(b0bec9b8364a15606d9f4d11f674dea2ec25279f) )  /* bank 1, plane 0 */
	ROM_LOAD( "136036.139",   0x010000, 0x08000, CRC(b682bfca) SHA1(21625ea29609560b31b28c669b2599cfcdab8ea7) )  /* bank 1, plane 1 */
	ROM_LOAD( "136036.143",   0x020000, 0x08000, CRC(7697da26) SHA1(546b95467ca61fab9ac853635c8972a2dedb6581) )  /* bank 1, plane 2 */
	ROM_LOAD( "136036.147",   0x030000, 0x08000, CRC(4e9d664c) SHA1(334c5f9e96e7a6fbf375bca58bd58fb18a68da6f) )  /* bank 1, plane 3 */

	ROM_LOAD( "136036.136",   0x080000, 0x08000, CRC(b2b403aa) SHA1(ead88544d798d07c03fefadf84c87ed104036cfa) )  /* bank 2, plane 0 */
	ROM_LOAD( "136036.140",   0x090000, 0x08000, CRC(ec0c19ca) SHA1(a787502e9db79a3c601964cd8c662ff7d0c2f636) )  /* bank 2, plane 1 */
	ROM_LOAD( "136036.144",   0x0a0000, 0x08000, CRC(4407df98) SHA1(67eae09121e594bec965b78c44a964a39a2c8772) )  /* bank 2, plane 2 */
	ROM_LOAD( "136036.148",   0x0b0000, 0x08000, CRC(70dce06d) SHA1(c50aa9ea9ea70e282ebed54cefa9e51da01d14ca) )  /* bank 2, plane 3 */

	ROM_LOAD( "136036.137",   0x100000, 0x08000, CRC(3f352547) SHA1(2a3439bb1ca9cdb80c0bd61cbd5fce60d6ee1fef) )  /* bank 3, plane 0 */
	ROM_LOAD( "136036.141",   0x110000, 0x08000, CRC(9cbdffd0) SHA1(ce36e7b9792e41d87f97901b8d0a473835017ae6) )  /* bank 3, plane 1 */
	ROM_LOAD( "136036.145",   0x120000, 0x08000, CRC(e828e64b) SHA1(8ad19ee40e6c4caa3a102f967fe93e823c766b5a) )  /* bank 3, plane 2 */
	ROM_LOAD( "136036.149",   0x130000, 0x08000, CRC(81503a23) SHA1(9a28ba78f8029395542067f03460d15cbcd7b315) )  /* bank 3, plane 3 */

	ROM_LOAD( "136036.138",   0x180000, 0x08000, CRC(48c4d79d) SHA1(c4cd258cd1effbfd62e3270b626271db66095dcb) )  /* bank 4, plane 0 */
	ROM_LOAD( "136036.142",   0x190000, 0x08000, CRC(7faae75f) SHA1(e3c3f77aa9ec91d4fcdd2fc5f0a22063eec1afe2) )  /* bank 4, plane 1 */
	ROM_LOAD( "136036.146",   0x1a0000, 0x08000, CRC(8ae5a7b5) SHA1(97ddf916c41d6ddd23501e27611a83c912ec9ad2) )  /* bank 4, plane 2 */
	ROM_LOAD( "136036.150",   0x1b0000, 0x08000, CRC(a10c4bd9) SHA1(3177f22aff9e18c0f003f3fd1fa4b258308eee07) )  /* bank 4, plane 3 */

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136036.152",   0x000, 0x200, CRC(4f96e57c) SHA1(271633a0aacd1d1efe2917728b73e90010c64d2c) )  /* remap */
	ROM_LOAD( "136036.151",   0x200, 0x200, CRC(7daf351f) SHA1(95c13d81a47440f847af7b19632cc032380b9ff4) )  /* color */

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


/*
    Road Runner
*/

ROM_START( roadrunn )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136040-228.11c",  0x010000, 0x008000, CRC(b66c629a) SHA1(94dc13f2f151a4109ee50016bcc1392aef0d5a22) )
	ROM_LOAD16_BYTE( "136040-229.11a",  0x010001, 0x008000, CRC(5638959f) SHA1(486bd13ba151558b72bf29de9ecc14944e56435c) )
	ROM_LOAD16_BYTE( "136040-230.13c",  0x020000, 0x008000, CRC(cd7956a3) SHA1(d72aa90b74a77e5fa49cb34515383c8ab054d654) )
	ROM_LOAD16_BYTE( "136040-231.13a",  0x020001, 0x008000, CRC(722f2d3b) SHA1(b97f0a22377f3fc3d05290410a47f7c3b389d190) )
	ROM_LOAD16_BYTE( "136040-134.12c",  0x050000, 0x008000, CRC(18f431fe) SHA1(aa6500da9c77b34f179b725e6f71c08fa8c604dd) )
	ROM_LOAD16_BYTE( "136040-135.12a",  0x050001, 0x008000, CRC(cb06f9ab) SHA1(4191038027b179c9e62fe89dd0e13105ff044a8a) )
	ROM_LOAD16_BYTE( "136040-136.14c",  0x060000, 0x008000, CRC(8050bce4) SHA1(8ba76b1afdfb15367909ffa7380761b423730458) )
	ROM_LOAD16_BYTE( "136040-137.14a",  0x060001, 0x008000, CRC(3372a5cf) SHA1(ae9b960565a07a34e594b47315a22773bc07a35e) )
	ROM_LOAD16_BYTE( "136040-138.16c",  0x070000, 0x008000, CRC(a83155ee) SHA1(b6b08f289446ae881729970fa474ee79d043eef0) )
	ROM_LOAD16_BYTE( "136040-139.16a",  0x070001, 0x008000, CRC(23aead1c) SHA1(c63062a780ac969b7198ce7f2ed17722f416a138) )
	ROM_LOAD16_BYTE( "136040-140.17c",  0x080000, 0x004000, CRC(d1464c88) SHA1(0d600119a9137f4c7cad7f708e0c885da96af207) )
	ROM_LOAD16_BYTE( "136040-141.17a",  0x080001, 0x004000, CRC(f8f2acdf) SHA1(b9e31c1f96fab5368fb90768e059157f74cbc762) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136040-143.15e",  0x008000, 0x004000, CRC(62b9878e) SHA1(db6072a7c8600d85fb70dac3d940a200d0cb5d0a) )
	ROM_LOAD( "136040-144.17e",  0x00c000, 0x004000, CRC(6ef1b804) SHA1(176066c6946090c87945213145960754cd315acb) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x300000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136040-101.4b",   0x000000, 0x008000, CRC(26d9f29c) SHA1(e03a03484ede4ea209040faf64e88f0667a0ab40) )  /* bank 1, plane 0 */
	ROM_LOAD( "136040-107.9b",   0x010000, 0x008000, CRC(8aac0ba4) SHA1(67a6412c77d08eab6745db9dff3e32a1bb768901) )  /* bank 1, plane 1 */
	ROM_LOAD( "136040-113.4f",   0x020000, 0x008000, CRC(48b74c52) SHA1(d8328a9ff715247d841be02b1bb7aa043dcb77e1) )  /* bank 1, plane 2 */
	ROM_LOAD( "136040-119.9f",   0x030000, 0x008000, CRC(17a6510c) SHA1(ae24764ba90d48a6c44b165191a713b12004dc47) )  /* bank 1, plane 3 */

	ROM_LOAD( "136040-102.3b",   0x080000, 0x008000, CRC(ae88f54b) SHA1(b6519d6d0078b852ce23967c01afb772be3eda0e) )  /* bank 2, plane 0 */
	ROM_LOAD( "136040-108.8b",   0x090000, 0x008000, CRC(a2ac13d4) SHA1(f36625f7cd84c00a82debec107404a78568466c3) )  /* bank 2, plane 1 */
	ROM_LOAD( "136040-114.3f",   0x0a0000, 0x008000, CRC(c91c3fcb) SHA1(c76c351defe6db2aa762625dc292aa88043975e4) )  /* bank 2, plane 2 */
	ROM_LOAD( "136040-120.8f",   0x0b0000, 0x008000, CRC(42d25859) SHA1(421b40cab1fe527581be43a754fc050e9f3d5283) )  /* bank 2, plane 3 */

	ROM_LOAD( "136040-103.2b",   0x100000, 0x008000, CRC(f2d7ef55) SHA1(b76cf23293913d9171112627e4bb9aadede9bf52) )  /* bank 3, plane 0 */
	ROM_LOAD( "136040-109.7b",   0x110000, 0x008000, CRC(11a843dc) SHA1(9ca18c065cf2504657fe70238e28d08808a5012b) )  /* bank 3, plane 1 */
	ROM_LOAD( "136040-115.2f",   0x120000, 0x008000, CRC(8b1fa5bc) SHA1(65e1b390838b195f4e51f72fb8cae129345666d9) )  /* bank 3, plane 2 */
	ROM_LOAD( "136040-121.7f",   0x130000, 0x008000, CRC(ecf278f2) SHA1(325a593bdebae6f8cd9aa3335b18c8f3bc7056ad) )  /* bank 3, plane 3 */

	ROM_LOAD( "136040-104.1b",   0x180000, 0x008000, CRC(0203d89c) SHA1(1647d9b9d77a6c14859b22d54faf969fe949f1e3) )  /* bank 4, plane 0 */
	ROM_LOAD( "136040-110.6b",   0x190000, 0x008000, CRC(64801601) SHA1(928f8d38f3d3f363c1781048b6ce779273529098) )  /* bank 4, plane 1 */
	ROM_LOAD( "136040-116.1f",   0x1a0000, 0x008000, CRC(52b23a36) SHA1(fb868669aee0d0a959be4003fb7bd725623ef879) )  /* bank 4, plane 2 */
	ROM_LOAD( "136040-122.6f",   0x1b0000, 0x008000, CRC(b1137a9d) SHA1(d5acf1329bd9f559241e5e24c39412c62bab5fa0) )  /* bank 4, plane 3 */

	ROM_LOAD( "136040-105.4d",   0x200000, 0x008000, CRC(398a36f8) SHA1(547f01d01cac6ed1b428ebf70b8efbc59ef5b078) )  /* bank 5, plane 0 */
	ROM_LOAD( "136040-111.9d",   0x210000, 0x008000, CRC(f08b418b) SHA1(977c4d8e0b4cf5ed0437b5fd45ba0c164d40e5b7) )  /* bank 5, plane 1 */
	ROM_LOAD( "136040-117.2d",   0x220000, 0x008000, CRC(c4394834) SHA1(1805f8e41059649ce017cd2db45c8c72c0b18300) )  /* bank 5, plane 2 */
	ROM_LOAD( "136040-123.7d",   0x230000, 0x008000, CRC(dafd3dbe) SHA1(4c92d2306a5466490006305a6967ff124c01c6c0) )  /* bank 5, plane 3 */

	ROM_LOAD( "136040-106.3d",   0x280000, 0x008000, CRC(36a77bc5) SHA1(1bc73ab67ce213d2f9069328a8d90d6c7e916689) )  /* bank 6, plane 0 */
	ROM_LOAD( "136040-112.8d",   0x290000, 0x008000, CRC(b6624f3c) SHA1(a9ba46dc1f366d6cf257959e9b88cfb7819fffd9) )  /* bank 6, plane 1 */
	ROM_LOAD( "136040-118.1d",   0x2a0000, 0x008000, CRC(f489a968) SHA1(424ef3f11b30b152e03d40e604350098e0c3e9e0) )  /* bank 6, plane 2 */
	ROM_LOAD( "136040-124.6d",   0x2b0000, 0x008000, CRC(524d65f7) SHA1(3f2b0ee642fe7eebb578b00b44d9821369269791) )  /* bank 6, plane 3 */

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136040-126.7a",   0x000000, 0x000200, CRC(1713c0cd) SHA1(237ce1c53d8a17823df3341360f03b2b94cd91bb) )
	ROM_LOAD( "136040-125.5a",   0x000200, 0x000200, CRC(a9ca8795) SHA1(77583510e7a7179493f313e0c0b25d029dd6e583) )

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


ROM_START( roadrunn2 )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136040-x28.11c",  0x010000, 0x008000, CRC(fbd43085) SHA1(57f2f2f01e235f4525507583887d52b09f6267b4) )
	ROM_LOAD16_BYTE( "136040-x29.11a",  0x010001, 0x008000, CRC(f8d8819b) SHA1(dddb44290e3d2728845639611c3a66c3389c8f69) )
	ROM_LOAD16_BYTE( "136040-x30.13c",  0x020000, 0x008000, CRC(6a273375) SHA1(2e222ad4a4d53ec50f848386fca12f74e98814ed) )
	ROM_LOAD16_BYTE( "136040-x31.13a",  0x020001, 0x008000, CRC(eb5c4368) SHA1(ff22a9262c6c728fa5c316e993a2cc26b8c8b6fd) )
	ROM_LOAD16_BYTE( "136040-134.12c",  0x050000, 0x008000, CRC(18f431fe) SHA1(aa6500da9c77b34f179b725e6f71c08fa8c604dd) )
	ROM_LOAD16_BYTE( "136040-135.12a",  0x050001, 0x008000, CRC(cb06f9ab) SHA1(4191038027b179c9e62fe89dd0e13105ff044a8a) )
	ROM_LOAD16_BYTE( "136040-136.14c",  0x060000, 0x008000, CRC(8050bce4) SHA1(8ba76b1afdfb15367909ffa7380761b423730458) )
	ROM_LOAD16_BYTE( "136040-137.14a",  0x060001, 0x008000, CRC(3372a5cf) SHA1(ae9b960565a07a34e594b47315a22773bc07a35e) )
	ROM_LOAD16_BYTE( "136040-138.16c",  0x070000, 0x008000, CRC(a83155ee) SHA1(b6b08f289446ae881729970fa474ee79d043eef0) )
	ROM_LOAD16_BYTE( "136040-139.16a",  0x070001, 0x008000, CRC(23aead1c) SHA1(c63062a780ac969b7198ce7f2ed17722f416a138) )
	ROM_LOAD16_BYTE( "136040-140.17c",  0x080000, 0x004000, CRC(d1464c88) SHA1(0d600119a9137f4c7cad7f708e0c885da96af207) )
	ROM_LOAD16_BYTE( "136040-141.17a",  0x080001, 0x004000, CRC(f8f2acdf) SHA1(b9e31c1f96fab5368fb90768e059157f74cbc762) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136040-143.15e",  0x008000, 0x004000, CRC(62b9878e) SHA1(db6072a7c8600d85fb70dac3d940a200d0cb5d0a) )
	ROM_LOAD( "136040-144.17e",  0x00c000, 0x004000, CRC(6ef1b804) SHA1(176066c6946090c87945213145960754cd315acb) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x300000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136040-101.4b",   0x000000, 0x008000, CRC(26d9f29c) SHA1(e03a03484ede4ea209040faf64e88f0667a0ab40) )  /* bank 1, plane 0 */
	ROM_LOAD( "136040-107.9b",   0x010000, 0x008000, CRC(8aac0ba4) SHA1(67a6412c77d08eab6745db9dff3e32a1bb768901) )  /* bank 1, plane 1 */
	ROM_LOAD( "136040-113.4f",   0x020000, 0x008000, CRC(48b74c52) SHA1(d8328a9ff715247d841be02b1bb7aa043dcb77e1) )  /* bank 1, plane 2 */
	ROM_LOAD( "136040-119.9f",   0x030000, 0x008000, CRC(17a6510c) SHA1(ae24764ba90d48a6c44b165191a713b12004dc47) )  /* bank 1, plane 3 */

	ROM_LOAD( "136040-102.3b",   0x080000, 0x008000, CRC(ae88f54b) SHA1(b6519d6d0078b852ce23967c01afb772be3eda0e) )  /* bank 2, plane 0 */
	ROM_LOAD( "136040-108.8b",   0x090000, 0x008000, CRC(a2ac13d4) SHA1(f36625f7cd84c00a82debec107404a78568466c3) )  /* bank 2, plane 1 */
	ROM_LOAD( "136040-114.3f",   0x0a0000, 0x008000, CRC(c91c3fcb) SHA1(c76c351defe6db2aa762625dc292aa88043975e4) )  /* bank 2, plane 2 */
	ROM_LOAD( "136040-120.8f",   0x0b0000, 0x008000, CRC(42d25859) SHA1(421b40cab1fe527581be43a754fc050e9f3d5283) )  /* bank 2, plane 3 */

	ROM_LOAD( "136040-103.2b",   0x100000, 0x008000, CRC(f2d7ef55) SHA1(b76cf23293913d9171112627e4bb9aadede9bf52) )  /* bank 3, plane 0 */
	ROM_LOAD( "136040-109.7b",   0x110000, 0x008000, CRC(11a843dc) SHA1(9ca18c065cf2504657fe70238e28d08808a5012b) )  /* bank 3, plane 1 */
	ROM_LOAD( "136040-115.2f",   0x120000, 0x008000, CRC(8b1fa5bc) SHA1(65e1b390838b195f4e51f72fb8cae129345666d9) )  /* bank 3, plane 2 */
	ROM_LOAD( "136040-121.7f",   0x130000, 0x008000, CRC(ecf278f2) SHA1(325a593bdebae6f8cd9aa3335b18c8f3bc7056ad) )  /* bank 3, plane 3 */

	ROM_LOAD( "136040-104.1b",   0x180000, 0x008000, CRC(0203d89c) SHA1(1647d9b9d77a6c14859b22d54faf969fe949f1e3) )  /* bank 4, plane 0 */
	ROM_LOAD( "136040-110.6b",   0x190000, 0x008000, CRC(64801601) SHA1(928f8d38f3d3f363c1781048b6ce779273529098) )  /* bank 4, plane 1 */
	ROM_LOAD( "136040-116.1f",   0x1a0000, 0x008000, CRC(52b23a36) SHA1(fb868669aee0d0a959be4003fb7bd725623ef879) )  /* bank 4, plane 2 */
	ROM_LOAD( "136040-122.6f",   0x1b0000, 0x008000, CRC(b1137a9d) SHA1(d5acf1329bd9f559241e5e24c39412c62bab5fa0) )  /* bank 4, plane 3 */

	ROM_LOAD( "136040-105.4d",   0x200000, 0x008000, CRC(398a36f8) SHA1(547f01d01cac6ed1b428ebf70b8efbc59ef5b078) )  /* bank 5, plane 0 */
	ROM_LOAD( "136040-111.9d",   0x210000, 0x008000, CRC(f08b418b) SHA1(977c4d8e0b4cf5ed0437b5fd45ba0c164d40e5b7) )  /* bank 5, plane 1 */
	ROM_LOAD( "136040-117.2d",   0x220000, 0x008000, CRC(c4394834) SHA1(1805f8e41059649ce017cd2db45c8c72c0b18300) )  /* bank 5, plane 2 */
	ROM_LOAD( "136040-123.7d",   0x230000, 0x008000, CRC(dafd3dbe) SHA1(4c92d2306a5466490006305a6967ff124c01c6c0) )  /* bank 5, plane 3 */

	ROM_LOAD( "136040-106.3d",   0x280000, 0x008000, CRC(36a77bc5) SHA1(1bc73ab67ce213d2f9069328a8d90d6c7e916689) )  /* bank 6, plane 0 */
	ROM_LOAD( "136040-112.8d",   0x290000, 0x008000, CRC(b6624f3c) SHA1(a9ba46dc1f366d6cf257959e9b88cfb7819fffd9) )  /* bank 6, plane 1 */
	ROM_LOAD( "136040-118.1d",   0x2a0000, 0x008000, CRC(f489a968) SHA1(424ef3f11b30b152e03d40e604350098e0c3e9e0) )  /* bank 6, plane 2 */
	ROM_LOAD( "136040-124.6d",   0x2b0000, 0x008000, CRC(524d65f7) SHA1(3f2b0ee642fe7eebb578b00b44d9821369269791) )  /* bank 6, plane 3 */

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136040-126.7a",   0x000000, 0x000200, CRC(1713c0cd) SHA1(237ce1c53d8a17823df3341360f03b2b94cd91bb) )
	ROM_LOAD( "136040-125.5a",   0x000200, 0x000200, CRC(a9ca8795) SHA1(77583510e7a7179493f313e0c0b25d029dd6e583) )

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


ROM_START( roadrunn1 )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136040-128.11c",  0x010000, 0x008000, CRC(5e39d540) SHA1(69fae4c7db247062dffda0138a6e3bfb697f17bf) )
	ROM_LOAD16_BYTE( "136040-129.11a",  0x010001, 0x008000, CRC(d79bfea1) SHA1(eb265ff5f3f08bbdb3b19f9c7dece7e58dd2c6ba) )
	ROM_LOAD16_BYTE( "136040-130.13c",  0x020000, 0x008000, CRC(66453b37) SHA1(cb467a9c8109475bbc6d24b4a3559fa2f2edc887) )
	ROM_LOAD16_BYTE( "136040-131.13a",  0x020001, 0x008000, CRC(a8497cdc) SHA1(bee9127a0395291b0a096273d3b241016d3469b2) )
	ROM_LOAD16_BYTE( "136040-134.12c",  0x050000, 0x008000, CRC(18f431fe) SHA1(aa6500da9c77b34f179b725e6f71c08fa8c604dd) )
	ROM_LOAD16_BYTE( "136040-135.12a",  0x050001, 0x008000, CRC(cb06f9ab) SHA1(4191038027b179c9e62fe89dd0e13105ff044a8a) )
	ROM_LOAD16_BYTE( "136040-136.14c",  0x060000, 0x008000, CRC(8050bce4) SHA1(8ba76b1afdfb15367909ffa7380761b423730458) )
	ROM_LOAD16_BYTE( "136040-137.14a",  0x060001, 0x008000, CRC(3372a5cf) SHA1(ae9b960565a07a34e594b47315a22773bc07a35e) )
	ROM_LOAD16_BYTE( "136040-138.16c",  0x070000, 0x008000, CRC(a83155ee) SHA1(b6b08f289446ae881729970fa474ee79d043eef0) )
	ROM_LOAD16_BYTE( "136040-139.16a",  0x070001, 0x008000, CRC(23aead1c) SHA1(c63062a780ac969b7198ce7f2ed17722f416a138) )
	ROM_LOAD16_BYTE( "136040-140.17c",  0x080000, 0x004000, CRC(d1464c88) SHA1(0d600119a9137f4c7cad7f708e0c885da96af207) )
	ROM_LOAD16_BYTE( "136040-141.17a",  0x080001, 0x004000, CRC(f8f2acdf) SHA1(b9e31c1f96fab5368fb90768e059157f74cbc762) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136040-143.15e",  0x008000, 0x004000, CRC(62b9878e) SHA1(db6072a7c8600d85fb70dac3d940a200d0cb5d0a) )
	ROM_LOAD( "136040-144.17e",  0x00c000, 0x004000, CRC(6ef1b804) SHA1(176066c6946090c87945213145960754cd315acb) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x300000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136040-101.4b",   0x000000, 0x008000, CRC(26d9f29c) SHA1(e03a03484ede4ea209040faf64e88f0667a0ab40) )  /* bank 1, plane 0 */
	ROM_LOAD( "136040-107.9b",   0x010000, 0x008000, CRC(8aac0ba4) SHA1(67a6412c77d08eab6745db9dff3e32a1bb768901) )  /* bank 1, plane 1 */
	ROM_LOAD( "136040-113.4f",   0x020000, 0x008000, CRC(48b74c52) SHA1(d8328a9ff715247d841be02b1bb7aa043dcb77e1) )  /* bank 1, plane 2 */
	ROM_LOAD( "136040-119.9f",   0x030000, 0x008000, CRC(17a6510c) SHA1(ae24764ba90d48a6c44b165191a713b12004dc47) )  /* bank 1, plane 3 */

	ROM_LOAD( "136040-102.3b",   0x080000, 0x008000, CRC(ae88f54b) SHA1(b6519d6d0078b852ce23967c01afb772be3eda0e) )  /* bank 2, plane 0 */
	ROM_LOAD( "136040-108.8b",   0x090000, 0x008000, CRC(a2ac13d4) SHA1(f36625f7cd84c00a82debec107404a78568466c3) )  /* bank 2, plane 1 */
	ROM_LOAD( "136040-114.3f",   0x0a0000, 0x008000, CRC(c91c3fcb) SHA1(c76c351defe6db2aa762625dc292aa88043975e4) )  /* bank 2, plane 2 */
	ROM_LOAD( "136040-120.8f",   0x0b0000, 0x008000, CRC(42d25859) SHA1(421b40cab1fe527581be43a754fc050e9f3d5283) )  /* bank 2, plane 3 */

	ROM_LOAD( "136040-103.2b",   0x100000, 0x008000, CRC(f2d7ef55) SHA1(b76cf23293913d9171112627e4bb9aadede9bf52) )  /* bank 3, plane 0 */
	ROM_LOAD( "136040-109.7b",   0x110000, 0x008000, CRC(11a843dc) SHA1(9ca18c065cf2504657fe70238e28d08808a5012b) )  /* bank 3, plane 1 */
	ROM_LOAD( "136040-115.2f",   0x120000, 0x008000, CRC(8b1fa5bc) SHA1(65e1b390838b195f4e51f72fb8cae129345666d9) )  /* bank 3, plane 2 */
	ROM_LOAD( "136040-121.7f",   0x130000, 0x008000, CRC(ecf278f2) SHA1(325a593bdebae6f8cd9aa3335b18c8f3bc7056ad) )  /* bank 3, plane 3 */

	ROM_LOAD( "136040-104.1b",   0x180000, 0x008000, CRC(0203d89c) SHA1(1647d9b9d77a6c14859b22d54faf969fe949f1e3) )  /* bank 4, plane 0 */
	ROM_LOAD( "136040-110.6b",   0x190000, 0x008000, CRC(64801601) SHA1(928f8d38f3d3f363c1781048b6ce779273529098) )  /* bank 4, plane 1 */
	ROM_LOAD( "136040-116.1f",   0x1a0000, 0x008000, CRC(52b23a36) SHA1(fb868669aee0d0a959be4003fb7bd725623ef879) )  /* bank 4, plane 2 */
	ROM_LOAD( "136040-122.6f",   0x1b0000, 0x008000, CRC(b1137a9d) SHA1(d5acf1329bd9f559241e5e24c39412c62bab5fa0) )  /* bank 4, plane 3 */

	ROM_LOAD( "136040-105.4d",   0x200000, 0x008000, CRC(398a36f8) SHA1(547f01d01cac6ed1b428ebf70b8efbc59ef5b078) )  /* bank 5, plane 0 */
	ROM_LOAD( "136040-111.9d",   0x210000, 0x008000, CRC(f08b418b) SHA1(977c4d8e0b4cf5ed0437b5fd45ba0c164d40e5b7) )  /* bank 5, plane 1 */
	ROM_LOAD( "136040-117.2d",   0x220000, 0x008000, CRC(c4394834) SHA1(1805f8e41059649ce017cd2db45c8c72c0b18300) )  /* bank 5, plane 2 */
	ROM_LOAD( "136040-123.7d",   0x230000, 0x008000, CRC(dafd3dbe) SHA1(4c92d2306a5466490006305a6967ff124c01c6c0) )  /* bank 5, plane 3 */

	ROM_LOAD( "136040-106.3d",   0x280000, 0x008000, CRC(36a77bc5) SHA1(1bc73ab67ce213d2f9069328a8d90d6c7e916689) )  /* bank 6, plane 0 */
	ROM_LOAD( "136040-112.8d",   0x290000, 0x008000, CRC(b6624f3c) SHA1(a9ba46dc1f366d6cf257959e9b88cfb7819fffd9) )  /* bank 6, plane 1 */
	ROM_LOAD( "136040-118.1d",   0x2a0000, 0x008000, CRC(f489a968) SHA1(424ef3f11b30b152e03d40e604350098e0c3e9e0) )  /* bank 6, plane 2 */
	ROM_LOAD( "136040-124.6d",   0x2b0000, 0x008000, CRC(524d65f7) SHA1(3f2b0ee642fe7eebb578b00b44d9821369269791) )  /* bank 6, plane 3 */

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136040-126.7a",   0x000000, 0x000200, CRC(1713c0cd) SHA1(237ce1c53d8a17823df3341360f03b2b94cd91bb) )
	ROM_LOAD( "136040-125.5a",   0x000200, 0x000200, CRC(a9ca8795) SHA1(77583510e7a7179493f313e0c0b25d029dd6e583) )

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


/*
    Road Blasters

    Road Blasters was released in a cartridge version for the generic upright
    cabinet and a dedicated three board stack for the cockpit version.

    The cartridge version is referenced in the TM-298 and SP-298 manuals.

    The cockpit version is referenced in the TM-299 and SP-299 manuals.
*/

ROM_START( roadblst )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136048-1157.11c", 0x010000, 0x008000, CRC(6d9ad91e) SHA1(15815d3777be5377e053a39c1cef20eb101d9b92) )
	ROM_LOAD16_BYTE( "136048-1158.11a", 0x010001, 0x008000, CRC(7d4cf151) SHA1(979006149906638b942a3157b237112acc16183d) )
	ROM_LOAD16_BYTE( "136048-1159.13c", 0x020000, 0x008000, CRC(921c0e34) SHA1(bcc90a73baaecc551e1065037681a79e260905ff) )
	ROM_LOAD16_BYTE( "136048-1160.13a", 0x020001, 0x008000, CRC(8bf22f7d) SHA1(b20f68c037038259f60f7fb64da49780e9faf19f) )
	ROM_LOAD16_BYTE( "136048-2141.7l",  0x050000, 0x008000, CRC(054273b2) SHA1(4c820c00d3b67825c361edc9615c89c2a9a1c6d3) )
	ROM_LOAD16_BYTE( "136048-2142.8l",  0x050001, 0x008000, CRC(49181bec) SHA1(79e042e4f079a9806ef12c5c8dfdc2e6e4f90011) )
	ROM_LOAD16_BYTE( "136048-2143.7m",  0x060000, 0x008000, CRC(f63dc29a) SHA1(e54637b9d0b271aa9b58e89a442ac03ec812e1eb) )
	ROM_LOAD16_BYTE( "136048-2144.8m",  0x060001, 0x008000, CRC(b1fc5955) SHA1(b860213a9b5ae7547c258812045e71795129598f) )
	ROM_LOAD16_BYTE( "136048-2145.7n",  0x070000, 0x008000, CRC(c6d30d6f) SHA1(acb552976b2dcfa585097ea246ca88034549c8ab) )
	ROM_LOAD16_BYTE( "136048-2146.8n",  0x070001, 0x008000, CRC(16951020) SHA1(5e5a6ad4ae87723060232c7ecb837f5fc2a9be68) )
	ROM_LOAD16_BYTE( "136048-2147.7k",  0x080000, 0x004000, CRC(5c1adf67) SHA1(53838a2f5059797991aa337a7bec32f7e694610a) )
	ROM_LOAD16_BYTE( "136048-2148.8k",  0x080001, 0x004000, CRC(d9ac8966) SHA1(7d056c1eb8184b4261c5713b0d5799b2fd8bde2a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136048-1149.14e",   0x4000, 0x4000, CRC(2e54f95e) SHA1(5056ddec3c88384ada1d2ee9b1532b9ba9f34e08) )
	ROM_LOAD( "136048-1169.1516e", 0x8000, 0x4000, CRC(ee318052) SHA1(f66ff39499697b7439dc62567e727fec769c1505) )
	ROM_LOAD( "136048-1170.17e",   0xc000, 0x4000, CRC(75dfec33) SHA1(3092348b98419bb23181d21406733d5d21cd3d82) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x380000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136048-1101.2s",  0x000000, 0x008000, CRC(fe342d27) SHA1(72deac16ab9b6b811f49d70d700d6bc3a904f9d5) )  /* bank 1, plane 0 */
	ROM_LOAD( "136048-1102.2r",  0x010000, 0x008000, CRC(17c7e780) SHA1(675a2fe5ac55c97d54153eacabf863a525e4b71f) )  /* bank 1, plane 1 */
	ROM_LOAD( "136048-1103.2n",  0x020000, 0x008000, CRC(39688e01) SHA1(1b273e26b48a2f69b0895c29c9b3e762c686661d) )  /* bank 1, plane 2 */
	ROM_LOAD( "136048-1104.2m",  0x030000, 0x008000, CRC(c8f9bd8e) SHA1(452194efafd5f9562434dafcb14b72f400fc4a49) )  /* bank 1, plane 3 */
	ROM_LOAD( "136048-1105.2k",  0x040000, 0x008000, CRC(c69e439e) SHA1(c933637820c04bdbc56990423adfb9067e09fad7) )  /* bank 1, plane 4 */
	ROM_LOAD( "136048-1106.2j",  0x050000, 0x008000, CRC(4ee55796) SHA1(045635cd4f80a5f2c378cb4c55996e09c21b5859) )  /* bank 1, plane 5 */

	ROM_LOAD( "136048-1107.3s",  0x080000, 0x008000, CRC(02117c58) SHA1(e68eb9fbdbd473f217883a96ad581a94f1df53d9) )  /* bank 2/3, plane 0 */
	ROM_CONTINUE(                0x100000, 0x008000 )
	ROM_LOAD( "136048-1108.2p",  0x090000, 0x008000, CRC(1e148525) SHA1(613c408b352d2d9bac8091bbd5a3c60d09f81084) )  /* bank 2/3, plane 1 */
	ROM_CONTINUE(                0x110000, 0x008000 )
	ROM_LOAD( "136048-1109.3n",  0x0a0000, 0x008000, CRC(110ce07e) SHA1(0d165e18bad935713efc44b8e609c7fa7093f5c8) )  /* bank 2/3, plane 2 */
	ROM_CONTINUE(                0x120000, 0x008000 )
	ROM_LOAD( "136048-1110.2l",  0x0b0000, 0x008000, CRC(c00aa0f4) SHA1(a95e2717e725721b59969499720a8da0cf208e36) )  /* bank 2/3, plane 3 */
	ROM_CONTINUE(                0x130000, 0x008000 )

	ROM_LOAD( "136048-1111.4s",  0x180000, 0x008000, CRC(c951d014) SHA1(596fd0315a5da3d3f6131883d44770724d8e46e3) )  /* bank 4/5, plane 0 */
	ROM_CONTINUE(                0x200000, 0x008000 )
	ROM_LOAD( "136048-1112.3r",  0x190000, 0x008000, CRC(95c5a006) SHA1(0492404c0f092c1bb6239078e3a9ef45fc153923) )  /* bank 4/5, plane 1 */
	ROM_CONTINUE(                0x210000, 0x008000 )
	ROM_LOAD( "136048-1113.4n",  0x1a0000, 0x008000, CRC(f61f2370) SHA1(9f4b2aa91ac5dc0e543e5dfe1151fdea3579c739) )  /* bank 4/5, plane 2 */
	ROM_CONTINUE(                0x220000, 0x008000 )
	ROM_LOAD( "136048-1114.3m",  0x1b0000, 0x008000, CRC(774a36a8) SHA1(ebcf4363174abd834f3e77b0f1a2e29497f470b4) )  /* bank 4/5, plane 3 */
	ROM_CONTINUE(                0x230000, 0x008000 )

	ROM_LOAD( "136048-1115.4r",  0x300000, 0x008000, CRC(a47bc79d) SHA1(367f21c355c0af55629d2c5746cd5cb06272abc0) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x280000, 0x008000 )
	ROM_LOAD( "136048-1116.3p",  0x310000, 0x008000, CRC(b8a5c215) SHA1(2594f2a865fb73c8a7829e50ff19a6494a35c088) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x290000, 0x008000 )
	ROM_LOAD( "136048-1117.4m",  0x320000, 0x008000, CRC(2d1c1f64) SHA1(66141b9a7ae8d6dd3f1315d60cf0e99cd802d2e5) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2a0000, 0x008000 )
	ROM_LOAD( "136048-1118.3l",  0x330000, 0x008000, CRC(be879b8e) SHA1(dc1d1f7bdb511e922b650fac88307a08ab37ac4c) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2b0000, 0x008000 )

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136048-1174.12d", 0x000000, 0x000200, CRC(db4a4d53) SHA1(c5468f3585ec9bc23c9ee990b3ae3738b0309823) )
	ROM_LOAD( "136048-1173.2d",  0x000200, 0x000200, CRC(c80574af) SHA1(9a3dc83f70e79915ce0db3e6e69b5dcfee3acb6f) )

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


ROM_START( roadblstg )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136048-2257.11c", 0x010000, 0x008000, CRC(6e9de790) SHA1(348d4953b63f577b2b1ad747c6fb32c8ec55d310) )
	ROM_LOAD16_BYTE( "136048-2258.11a", 0x010001, 0x008000, CRC(5160c69e) SHA1(a4aa4ae0cf24f0d3c768186332b4b0f8e55d2700) )
	ROM_LOAD16_BYTE( "136048-2259.13c", 0x020000, 0x008000, CRC(62f10976) SHA1(e3c832304ad9163ed43d128aa68c321af655958d) )
	ROM_LOAD16_BYTE( "136048-2260.13a", 0x020001, 0x008000, CRC(528035ba) SHA1(f6f9ef60551d9bd59d9e729a09e1a7352f76e43f) )
	ROM_LOAD16_BYTE( "136048-2141.7l",  0x050000, 0x008000, CRC(054273b2) SHA1(4c820c00d3b67825c361edc9615c89c2a9a1c6d3) )
	ROM_LOAD16_BYTE( "136048-2142.8l",  0x050001, 0x008000, CRC(49181bec) SHA1(79e042e4f079a9806ef12c5c8dfdc2e6e4f90011) )
	ROM_LOAD16_BYTE( "136048-2143.7m",  0x060000, 0x008000, CRC(f63dc29a) SHA1(e54637b9d0b271aa9b58e89a442ac03ec812e1eb) )
	ROM_LOAD16_BYTE( "136048-2144.8m",  0x060001, 0x008000, CRC(b1fc5955) SHA1(b860213a9b5ae7547c258812045e71795129598f) )
	ROM_LOAD16_BYTE( "136048-2145.7n",  0x070000, 0x008000, CRC(c6d30d6f) SHA1(acb552976b2dcfa585097ea246ca88034549c8ab) )
	ROM_LOAD16_BYTE( "136048-2146.8n",  0x070001, 0x008000, CRC(16951020) SHA1(5e5a6ad4ae87723060232c7ecb837f5fc2a9be68) )
	ROM_LOAD16_BYTE( "136048-2147.7k",  0x080000, 0x004000, CRC(5c1adf67) SHA1(53838a2f5059797991aa337a7bec32f7e694610a) )
	ROM_LOAD16_BYTE( "136048-2148.8k",  0x080001, 0x004000, CRC(d9ac8966) SHA1(7d056c1eb8184b4261c5713b0d5799b2fd8bde2a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136048-1149.14e",   0x4000, 0x4000, CRC(2e54f95e) SHA1(5056ddec3c88384ada1d2ee9b1532b9ba9f34e08) )
	ROM_LOAD( "136048-1169.1516e", 0x8000, 0x4000, CRC(ee318052) SHA1(f66ff39499697b7439dc62567e727fec769c1505) )
	ROM_LOAD( "136048-1170.17e",   0xc000, 0x4000, CRC(75dfec33) SHA1(3092348b98419bb23181d21406733d5d21cd3d82) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x380000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136048-1101.2s",  0x000000, 0x008000, CRC(fe342d27) SHA1(72deac16ab9b6b811f49d70d700d6bc3a904f9d5) )  /* bank 1, plane 0 */
	ROM_LOAD( "136048-1102.2r",  0x010000, 0x008000, CRC(17c7e780) SHA1(675a2fe5ac55c97d54153eacabf863a525e4b71f) )  /* bank 1, plane 1 */
	ROM_LOAD( "136048-1103.2n",  0x020000, 0x008000, CRC(39688e01) SHA1(1b273e26b48a2f69b0895c29c9b3e762c686661d) )  /* bank 1, plane 2 */
	ROM_LOAD( "136048-1104.2m",  0x030000, 0x008000, CRC(c8f9bd8e) SHA1(452194efafd5f9562434dafcb14b72f400fc4a49) )  /* bank 1, plane 3 */
	ROM_LOAD( "136048-1105.2k",  0x040000, 0x008000, CRC(c69e439e) SHA1(c933637820c04bdbc56990423adfb9067e09fad7) )  /* bank 1, plane 4 */
	ROM_LOAD( "136048-1106.2j",  0x050000, 0x008000, CRC(4ee55796) SHA1(045635cd4f80a5f2c378cb4c55996e09c21b5859) )  /* bank 1, plane 5 */

	ROM_LOAD( "136048-1107.3s",  0x080000, 0x008000, CRC(02117c58) SHA1(e68eb9fbdbd473f217883a96ad581a94f1df53d9) )  /* bank 2/3, plane 0 */
	ROM_CONTINUE(                0x100000, 0x008000 )
	ROM_LOAD( "136048-1108.2p",  0x090000, 0x008000, CRC(1e148525) SHA1(613c408b352d2d9bac8091bbd5a3c60d09f81084) )  /* bank 2/3, plane 1 */
	ROM_CONTINUE(                0x110000, 0x008000 )
	ROM_LOAD( "136048-1109.3n",  0x0a0000, 0x008000, CRC(110ce07e) SHA1(0d165e18bad935713efc44b8e609c7fa7093f5c8) )  /* bank 2/3, plane 2 */
	ROM_CONTINUE(                0x120000, 0x008000 )
	ROM_LOAD( "136048-1110.2l",  0x0b0000, 0x008000, CRC(c00aa0f4) SHA1(a95e2717e725721b59969499720a8da0cf208e36) )  /* bank 2/3, plane 3 */
	ROM_CONTINUE(                0x130000, 0x008000 )

	ROM_LOAD( "136048-1111.4s",  0x180000, 0x008000, CRC(c951d014) SHA1(596fd0315a5da3d3f6131883d44770724d8e46e3) )  /* bank 4/5, plane 0 */
	ROM_CONTINUE(                0x200000, 0x008000 )
	ROM_LOAD( "136048-1112.3r",  0x190000, 0x008000, CRC(95c5a006) SHA1(0492404c0f092c1bb6239078e3a9ef45fc153923) )  /* bank 4/5, plane 1 */
	ROM_CONTINUE(                0x210000, 0x008000 )
	ROM_LOAD( "136048-1113.4n",  0x1a0000, 0x008000, CRC(f61f2370) SHA1(9f4b2aa91ac5dc0e543e5dfe1151fdea3579c739) )  /* bank 4/5, plane 2 */
	ROM_CONTINUE(                0x220000, 0x008000 )
	ROM_LOAD( "136048-1114.3m",  0x1b0000, 0x008000, CRC(774a36a8) SHA1(ebcf4363174abd834f3e77b0f1a2e29497f470b4) )  /* bank 4/5, plane 3 */
	ROM_CONTINUE(                0x230000, 0x008000 )

	ROM_LOAD( "136048-1115.4r",  0x300000, 0x008000, CRC(a47bc79d) SHA1(367f21c355c0af55629d2c5746cd5cb06272abc0) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x280000, 0x008000 )
	ROM_LOAD( "136048-1116.3p",  0x310000, 0x008000, CRC(b8a5c215) SHA1(2594f2a865fb73c8a7829e50ff19a6494a35c088) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x290000, 0x008000 )
	ROM_LOAD( "136048-1117.4m",  0x320000, 0x008000, CRC(2d1c1f64) SHA1(66141b9a7ae8d6dd3f1315d60cf0e99cd802d2e5) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2a0000, 0x008000 )
	ROM_LOAD( "136048-1118.3l",  0x330000, 0x008000, CRC(be879b8e) SHA1(dc1d1f7bdb511e922b650fac88307a08ab37ac4c) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2b0000, 0x008000 )

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136048-1174.12d", 0x000000, 0x000200, CRC(db4a4d53) SHA1(c5468f3585ec9bc23c9ee990b3ae3738b0309823) )
	ROM_LOAD( "136048-1173.2d",  0x000200, 0x000200, CRC(c80574af) SHA1(9a3dc83f70e79915ce0db3e6e69b5dcfee3acb6f) )

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END

/*
Road Blasters (German upright Rev xx)(Atari 1987)
Dumped from the original Atari
(System 1 motherboard Rev. C)
(System 1 daughterboard). "Road Blasters Cartridge 3 PCB Assy: A043972-01 Rev.A"
*/

// different IC positions and different GFX rom configuration.
ROM_START( roadblstgu )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	// expects bios roms 114/115, does not work properly with others (corrupt insert coin text)
	ROM_LOAD16_BYTE( "136032-114.j7", 0x00000, 0x04000, CRC(195c54ad) SHA1(d7cda3cd3db4c6f77074ca05e96ae11b62e048b7) )
	ROM_LOAD16_BYTE( "136032-115.j8", 0x00001, 0x04000, CRC(7275b4dc) SHA1(0896ab37ea832a1335046353612c1b4c86d8d040) )

	ROM_LOAD16_BYTE( "136048-1257.c11", 0x010000, 0x008000, CRC(604a5cc0) SHA1(a057a2e47ac7b7c727e9c1bfce28ba955ce75442) )
	ROM_LOAD16_BYTE( "136048-1258.a11", 0x010001, 0x008000, CRC(3d10929d) SHA1(aa4d568e5d5b62fb8ea11094bad78bb8f713404e) )
	ROM_LOAD16_BYTE( "136048-1259.c13", 0x020000, 0x008000, CRC(b9c807ac) SHA1(ca955790c98037045aa49425392581f21d33caa9) )
	ROM_LOAD16_BYTE( "136048-1260.a13", 0x020001, 0x008000, CRC(eaeb1196) SHA1(82bf14244b342c97adde893f19a050baab30ab1e) )

	ROM_LOAD16_BYTE( "136048-1163.c12",  0x050000, 0x008000, CRC(054273b2) SHA1(4c820c00d3b67825c361edc9615c89c2a9a1c6d3) )//
	ROM_LOAD16_BYTE( "136048-1164.a12",  0x050001, 0x008000, CRC(49181bec) SHA1(79e042e4f079a9806ef12c5c8dfdc2e6e4f90011) )//
	ROM_LOAD16_BYTE( "136048-1165.c14",  0x060000, 0x008000, CRC(f63dc29a) SHA1(e54637b9d0b271aa9b58e89a442ac03ec812e1eb) )//
	ROM_LOAD16_BYTE( "136048-1166.a14",  0x060001, 0x008000, CRC(b1fc5955) SHA1(b860213a9b5ae7547c258812045e71795129598f) )//
	ROM_LOAD16_BYTE( "136048-1167.c16",  0x070000, 0x008000, CRC(c6d30d6f) SHA1(acb552976b2dcfa585097ea246ca88034549c8ab) )//
	ROM_LOAD16_BYTE( "136048-1168.a16",  0x070001, 0x008000, CRC(16951020) SHA1(5e5a6ad4ae87723060232c7ecb837f5fc2a9be68) )//
	ROM_LOAD16_BYTE( "136048-2147.c17",  0x080000, 0x004000, CRC(5c1adf67) SHA1(53838a2f5059797991aa337a7bec32f7e694610a) )//
	ROM_LOAD16_BYTE( "136048-2148.a17",  0x080001, 0x004000, CRC(d9ac8966) SHA1(7d056c1eb8184b4261c5713b0d5799b2fd8bde2a) )//

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136048-1149.e14",   0x4000, 0x4000, CRC(2e54f95e) SHA1(5056ddec3c88384ada1d2ee9b1532b9ba9f34e08) )
	ROM_LOAD( "136048-1169.e15",   0x8000, 0x4000, CRC(ee318052) SHA1(f66ff39499697b7439dc62567e727fec769c1505) )//
	ROM_LOAD( "136048-1170.e17",   0xc000, 0x4000, CRC(75dfec33) SHA1(3092348b98419bb23181d21406733d5d21cd3d82) )//

	ROM_REGION( 0x2000, "alpha", 0 )
	ROM_LOAD( "136032.107.b2", 0x00000, 0x02000, CRC(7a29dc07) SHA1(72ba464da01bd6d3a91b8d9997d5ac14b6f47aad) )

	ROM_REGION( 0x380000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136048-1101.b4",  0x000000, 0x008000, CRC(fe342d27) SHA1(72deac16ab9b6b811f49d70d700d6bc3a904f9d5) )  /* bank 1, plane 0 *///
	ROM_LOAD( "136048-1102.b9",  0x010000, 0x008000, CRC(17c7e780) SHA1(675a2fe5ac55c97d54153eacabf863a525e4b71f) )  /* bank 1, plane 1 *///
	ROM_LOAD( "136048-1103.f4",  0x020000, 0x008000, CRC(39688e01) SHA1(1b273e26b48a2f69b0895c29c9b3e762c686661d) )  /* bank 1, plane 2 *///
	ROM_LOAD( "136048-1104.f9",  0x030000, 0x008000, CRC(c8f9bd8e) SHA1(452194efafd5f9562434dafcb14b72f400fc4a49) )  /* bank 1, plane 3 *///
	ROM_LOAD( "136048-1105.h4",  0x040000, 0x008000, CRC(c69e439e) SHA1(c933637820c04bdbc56990423adfb9067e09fad7) )  /* bank 1, plane 4 *///
	ROM_LOAD( "136048-1106.h9",  0x050000, 0x008000, CRC(4ee55796) SHA1(045635cd4f80a5f2c378cb4c55996e09c21b5859) )  /* bank 1, plane 5 *///

	ROM_LOAD( "136048-1119.b3",  0x080000, 0x008000, CRC(2533be39) SHA1(4dc5c85fe5388ed62806742c8bb27957b9ffb321) )  /* bank 2/3, plane 0 */
	ROM_LOAD( "136048-1120.b8",  0x090000, 0x008000, CRC(3de4f4db) SHA1(a3c19ef52f31b924b7eaa7bd864fe942d2e581f9) )  /* bank 2/3, plane 1 */
	ROM_LOAD( "136048-1121.f3",  0x0a0000, 0x008000, CRC(4a1b6b9f) SHA1(c0c6de17573b314954c02fdd57cf57c9cd59c783) )  /* bank 2/3, plane 2 */
	ROM_LOAD( "136048-1122.f8",  0x0b0000, 0x008000, CRC(680bdb7d) SHA1(6a66560a8ef983055b250d59022e3d9a2ea25861) )  /* bank 2/3, plane 3 */

	ROM_LOAD( "136048-1123.b2",  0x100000, 0x008000, CRC(a405d8bf) SHA1(c5bb7c54caf04e8f6a6534de7fd1127d9777dded) )  /* bank 2/3, plane 0 */
	ROM_LOAD( "136048-1124.b7",  0x110000, 0x008000, CRC(b9070c2e) SHA1(4b912cc35d0f4d445a6e619fc52e8c131bbb7899) )  /* bank 2/3, plane 1 */
	ROM_LOAD( "136048-1125.f2",  0x120000, 0x008000, CRC(5dfac572) SHA1(46eee972944766b7ba3a88937da3b3ac785c5741) )  /* bank 2/3, plane 2 */
	ROM_LOAD( "136048-1126.f7",  0x130000, 0x008000, CRC(a0416c6d) SHA1(bdaa2d828035206f0662c0c920b654cf4d314ca6) )  /* bank 2/3, plane 3 */

	ROM_LOAD( "136048-1127.b1",  0x180000, 0x008000, CRC(0138b391) SHA1(3d22c93c5ffb1a998f85a27eb9f58061f2ffe19d) )  /* bank 4/5, plane 0 */
	ROM_LOAD( "136048-1128.b6",  0x190000, 0x008000, CRC(5136fb4b) SHA1(0b2a8d96e573907e11d4fe7288069e9735c1d072) )  /* bank 4/5, plane 1 */
	ROM_LOAD( "136048-1129.f1",  0x1a0000, 0x008000, CRC(7d75bb12) SHA1(bbaf4c991bb21b32dc1c6fdc23673e27c33f4edf) )  /* bank 4/5, plane 2 */
	ROM_LOAD( "136048-1130.f6",  0x1b0000, 0x008000, CRC(81bb54d9) SHA1(134a6280183595b5b5989d8cc593b8fbd6a74577) )  /* bank 4/5, plane 3 */

	ROM_LOAD( "136048-1131.d4",  0x200000, 0x008000, CRC(72233889) SHA1(e5596c9f450a00947bd0ab4b7b7b2623d4ce8d89) )  /* bank 4/5, plane 0 */
	ROM_LOAD( "136048-1132.d9",  0x210000, 0x008000, CRC(6a82b8a7) SHA1(8b9f65617601cb39d37110074e99ab27fd5fa274) )  /* bank 4/5, plane 1 */
	ROM_LOAD( "136048-1133.d2",  0x220000, 0x008000, CRC(845dd347) SHA1(47ab870a808898c819d48454099b44ccf507c341) )  /* bank 4/5, plane 2 */
	ROM_LOAD( "136048-1134.d7",  0x230000, 0x008000, CRC(54e4c9e6) SHA1(cfd989b6237c95bac1717025a895eb4b1df28365) )  /* bank 4/5, plane 3 */

	ROM_LOAD( "136048-1115.d3",  0x300000, 0x008000, CRC(a47bc79d) SHA1(367f21c355c0af55629d2c5746cd5cb06272abc0) )  /* bank 7/6, plane 0 *///
	ROM_CONTINUE(                0x280000, 0x008000 )
	ROM_LOAD( "136048-1116.d8",  0x310000, 0x008000, CRC(b8a5c215) SHA1(2594f2a865fb73c8a7829e50ff19a6494a35c088) )  /* bank 7/6, plane 0 *///
	ROM_CONTINUE(                0x290000, 0x008000 )
	ROM_LOAD( "136048-1117.d1",  0x320000, 0x008000, CRC(2d1c1f64) SHA1(66141b9a7ae8d6dd3f1315d60cf0e99cd802d2e5) )  /* bank 7/6, plane 0 *///
	ROM_CONTINUE(                0x2a0000, 0x008000 )
	ROM_LOAD( "136048-1118.d6",  0x330000, 0x008000, CRC(be879b8e) SHA1(dc1d1f7bdb511e922b650fac88307a08ab37ac4c) )  /* bank 7/6, plane 0 *///
	ROM_CONTINUE(                0x2b0000, 0x008000 )

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136048-1174.a7", 0x000000, 0x000200, CRC(db4a4d53) SHA1(c5468f3585ec9bc23c9ee990b3ae3738b0309823) )//
	ROM_LOAD( "136048-1173.a5", 0x000200, 0x000200, CRC(c80574af) SHA1(9a3dc83f70e79915ce0db3e6e69b5dcfee3acb6f) )//

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END

ROM_START( roadblst3 )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136048-3157.11c", 0x010000, 0x008000, CRC(ce88fe34) SHA1(72a311e7e2e5f588226168e048905281f71f0aef) )
	ROM_LOAD16_BYTE( "136048-3158.11a", 0x010001, 0x008000, CRC(03bf2879) SHA1(27f2622b9e2a3f823557fd231a8b0e3234fea5cb) )
	ROM_LOAD16_BYTE( "136048-3159.13c", 0x020000, 0x008000, CRC(4305d74a) SHA1(618e0c638efb4110daf0954e89368c7b68a2a155) )
	ROM_LOAD16_BYTE( "136048-3160.13a", 0x020001, 0x008000, CRC(23304687) SHA1(3997504f7a961f74f4099a3cf339c190e0f4da4f) )
	ROM_LOAD16_BYTE( "136048-2141.7l",  0x050000, 0x008000, CRC(054273b2) SHA1(4c820c00d3b67825c361edc9615c89c2a9a1c6d3) )
	ROM_LOAD16_BYTE( "136048-2142.8l",  0x050001, 0x008000, CRC(49181bec) SHA1(79e042e4f079a9806ef12c5c8dfdc2e6e4f90011) )
	ROM_LOAD16_BYTE( "136048-2143.7m",  0x060000, 0x008000, CRC(f63dc29a) SHA1(e54637b9d0b271aa9b58e89a442ac03ec812e1eb) )
	ROM_LOAD16_BYTE( "136048-2144.8m",  0x060001, 0x008000, CRC(b1fc5955) SHA1(b860213a9b5ae7547c258812045e71795129598f) )
	ROM_LOAD16_BYTE( "136048-2145.7n",  0x070000, 0x008000, CRC(c6d30d6f) SHA1(acb552976b2dcfa585097ea246ca88034549c8ab) )
	ROM_LOAD16_BYTE( "136048-2146.8n",  0x070001, 0x008000, CRC(16951020) SHA1(5e5a6ad4ae87723060232c7ecb837f5fc2a9be68) )
	ROM_LOAD16_BYTE( "136048-2147.7k",  0x080000, 0x004000, CRC(5c1adf67) SHA1(53838a2f5059797991aa337a7bec32f7e694610a) )
	ROM_LOAD16_BYTE( "136048-2148.8k",  0x080001, 0x004000, CRC(d9ac8966) SHA1(7d056c1eb8184b4261c5713b0d5799b2fd8bde2a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136048-1149.14e",   0x4000, 0x4000, CRC(2e54f95e) SHA1(5056ddec3c88384ada1d2ee9b1532b9ba9f34e08) )
	ROM_LOAD( "136048-1169.1516e", 0x8000, 0x4000, CRC(ee318052) SHA1(f66ff39499697b7439dc62567e727fec769c1505) )
	ROM_LOAD( "136048-1170.17e",   0xc000, 0x4000, CRC(75dfec33) SHA1(3092348b98419bb23181d21406733d5d21cd3d82) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x380000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136048-1101.2s",  0x000000, 0x008000, CRC(fe342d27) SHA1(72deac16ab9b6b811f49d70d700d6bc3a904f9d5) )  /* bank 1, plane 0 */
	ROM_LOAD( "136048-1102.2r",  0x010000, 0x008000, CRC(17c7e780) SHA1(675a2fe5ac55c97d54153eacabf863a525e4b71f) )  /* bank 1, plane 1 */
	ROM_LOAD( "136048-1103.2n",  0x020000, 0x008000, CRC(39688e01) SHA1(1b273e26b48a2f69b0895c29c9b3e762c686661d) )  /* bank 1, plane 2 */
	ROM_LOAD( "136048-1104.2m",  0x030000, 0x008000, CRC(c8f9bd8e) SHA1(452194efafd5f9562434dafcb14b72f400fc4a49) )  /* bank 1, plane 3 */
	ROM_LOAD( "136048-1105.2k",  0x040000, 0x008000, CRC(c69e439e) SHA1(c933637820c04bdbc56990423adfb9067e09fad7) )  /* bank 1, plane 4 */
	ROM_LOAD( "136048-1106.2j",  0x050000, 0x008000, CRC(4ee55796) SHA1(045635cd4f80a5f2c378cb4c55996e09c21b5859) )  /* bank 1, plane 5 */

	ROM_LOAD( "136048-1107.3s",  0x080000, 0x008000, CRC(02117c58) SHA1(e68eb9fbdbd473f217883a96ad581a94f1df53d9) )  /* bank 2/3, plane 0 */
	ROM_CONTINUE(                0x100000, 0x008000 )
	ROM_LOAD( "136048-1108.2p",  0x090000, 0x008000, CRC(1e148525) SHA1(613c408b352d2d9bac8091bbd5a3c60d09f81084) )  /* bank 2/3, plane 1 */
	ROM_CONTINUE(                0x110000, 0x008000 )
	ROM_LOAD( "136048-1109.3n",  0x0a0000, 0x008000, CRC(110ce07e) SHA1(0d165e18bad935713efc44b8e609c7fa7093f5c8) )  /* bank 2/3, plane 2 */
	ROM_CONTINUE(                0x120000, 0x008000 )
	ROM_LOAD( "136048-1110.2l",  0x0b0000, 0x008000, CRC(c00aa0f4) SHA1(a95e2717e725721b59969499720a8da0cf208e36) )  /* bank 2/3, plane 3 */
	ROM_CONTINUE(                0x130000, 0x008000 )

	ROM_LOAD( "136048-1111.4s",  0x180000, 0x008000, CRC(c951d014) SHA1(596fd0315a5da3d3f6131883d44770724d8e46e3) )  /* bank 4/5, plane 0 */
	ROM_CONTINUE(                0x200000, 0x008000 )
	ROM_LOAD( "136048-1112.3r",  0x190000, 0x008000, CRC(95c5a006) SHA1(0492404c0f092c1bb6239078e3a9ef45fc153923) )  /* bank 4/5, plane 1 */
	ROM_CONTINUE(                0x210000, 0x008000 )
	ROM_LOAD( "136048-1113.4n",  0x1a0000, 0x008000, CRC(f61f2370) SHA1(9f4b2aa91ac5dc0e543e5dfe1151fdea3579c739) )  /* bank 4/5, plane 2 */
	ROM_CONTINUE(                0x220000, 0x008000 )
	ROM_LOAD( "136048-1114.3m",  0x1b0000, 0x008000, CRC(774a36a8) SHA1(ebcf4363174abd834f3e77b0f1a2e29497f470b4) )  /* bank 4/5, plane 3 */
	ROM_CONTINUE(                0x230000, 0x008000 )

	ROM_LOAD( "136048-1115.4r",  0x300000, 0x008000, CRC(a47bc79d) SHA1(367f21c355c0af55629d2c5746cd5cb06272abc0) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x280000, 0x008000 )
	ROM_LOAD( "136048-1116.3p",  0x310000, 0x008000, CRC(b8a5c215) SHA1(2594f2a865fb73c8a7829e50ff19a6494a35c088) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x290000, 0x008000 )
	ROM_LOAD( "136048-1117.4m",  0x320000, 0x008000, CRC(2d1c1f64) SHA1(66141b9a7ae8d6dd3f1315d60cf0e99cd802d2e5) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2a0000, 0x008000 )
	ROM_LOAD( "136048-1118.3l",  0x330000, 0x008000, CRC(be879b8e) SHA1(dc1d1f7bdb511e922b650fac88307a08ab37ac4c) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2b0000, 0x008000 )

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136048-1174.12d", 0x000000, 0x000200, CRC(db4a4d53) SHA1(c5468f3585ec9bc23c9ee990b3ae3738b0309823) )
	ROM_LOAD( "136048-1173.2d",  0x000200, 0x000200, CRC(c80574af) SHA1(9a3dc83f70e79915ce0db3e6e69b5dcfee3acb6f) )

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


ROM_START( roadblstg2 )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136048-1239.11c", 0x010000, 0x008000, CRC(3b2bb14b) SHA1(e5eac61a4249a644fbc27908b443a3830d4488e6) )
	ROM_CONTINUE(                       0x050000, 0x008000 )
	ROM_LOAD16_BYTE( "136048-1240.11a", 0x010001, 0x008000, CRC(2a5ab597) SHA1(3bdd27c67b05a426bd57e03a5c71948b6b57a40a) )
	ROM_CONTINUE(                       0x050001, 0x008000 )
	ROM_LOAD16_BYTE( "136048-1255.13c", 0x020000, 0x008000, CRC(1dcce3e6) SHA1(8f4c151aef4ad2b0f586f3b0fd413d80653d6369) )
	ROM_CONTINUE(                       0x060000, 0x008000 )
	ROM_LOAD16_BYTE( "136048-1256.13a", 0x020001, 0x008000, CRC(193eaf68) SHA1(de907a0975093706cfe5e1f557d1c06ffd67ee1b) )
	ROM_CONTINUE(                       0x060001, 0x008000 )
	ROM_LOAD16_BYTE( "136048-2145.7n",  0x070000, 0x008000, CRC(c6d30d6f) SHA1(acb552976b2dcfa585097ea246ca88034549c8ab) )
	ROM_LOAD16_BYTE( "136048-2146.8n",  0x070001, 0x008000, CRC(16951020) SHA1(5e5a6ad4ae87723060232c7ecb837f5fc2a9be68) )
	ROM_LOAD16_BYTE( "136048-2147.7k",  0x080000, 0x004000, CRC(5c1adf67) SHA1(53838a2f5059797991aa337a7bec32f7e694610a) )
	ROM_LOAD16_BYTE( "136048-2148.8k",  0x080001, 0x004000, CRC(d9ac8966) SHA1(7d056c1eb8184b4261c5713b0d5799b2fd8bde2a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136048-1149.14e",   0x4000, 0x4000, CRC(2e54f95e) SHA1(5056ddec3c88384ada1d2ee9b1532b9ba9f34e08) )
	ROM_LOAD( "136048-1169.1516e", 0x8000, 0x4000, CRC(ee318052) SHA1(f66ff39499697b7439dc62567e727fec769c1505) )
	ROM_LOAD( "136048-1170.17e",   0xc000, 0x4000, CRC(75dfec33) SHA1(3092348b98419bb23181d21406733d5d21cd3d82) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x380000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136048-1101.2s",  0x000000, 0x008000, CRC(fe342d27) SHA1(72deac16ab9b6b811f49d70d700d6bc3a904f9d5) )  /* bank 1, plane 0 */
	ROM_LOAD( "136048-1102.2r",  0x010000, 0x008000, CRC(17c7e780) SHA1(675a2fe5ac55c97d54153eacabf863a525e4b71f) )  /* bank 1, plane 1 */
	ROM_LOAD( "136048-1103.2n",  0x020000, 0x008000, CRC(39688e01) SHA1(1b273e26b48a2f69b0895c29c9b3e762c686661d) )  /* bank 1, plane 2 */
	ROM_LOAD( "136048-1104.2m",  0x030000, 0x008000, CRC(c8f9bd8e) SHA1(452194efafd5f9562434dafcb14b72f400fc4a49) )  /* bank 1, plane 3 */
	ROM_LOAD( "136048-1105.2k",  0x040000, 0x008000, CRC(c69e439e) SHA1(c933637820c04bdbc56990423adfb9067e09fad7) )  /* bank 1, plane 4 */
	ROM_LOAD( "136048-1106.2j",  0x050000, 0x008000, CRC(4ee55796) SHA1(045635cd4f80a5f2c378cb4c55996e09c21b5859) )  /* bank 1, plane 5 */

	ROM_LOAD( "136048-1107.3s",  0x080000, 0x008000, CRC(02117c58) SHA1(e68eb9fbdbd473f217883a96ad581a94f1df53d9) )  /* bank 2/3, plane 0 */
	ROM_CONTINUE(                0x100000, 0x008000 )
	ROM_LOAD( "136048-1108.2p",  0x090000, 0x008000, CRC(1e148525) SHA1(613c408b352d2d9bac8091bbd5a3c60d09f81084) )  /* bank 2/3, plane 1 */
	ROM_CONTINUE(                0x110000, 0x008000 )
	ROM_LOAD( "136048-1109.3n",  0x0a0000, 0x008000, CRC(110ce07e) SHA1(0d165e18bad935713efc44b8e609c7fa7093f5c8) )  /* bank 2/3, plane 2 */
	ROM_CONTINUE(                0x120000, 0x008000 )
	ROM_LOAD( "136048-1110.2l",  0x0b0000, 0x008000, CRC(c00aa0f4) SHA1(a95e2717e725721b59969499720a8da0cf208e36) )  /* bank 2/3, plane 3 */
	ROM_CONTINUE(                0x130000, 0x008000 )

	ROM_LOAD( "136048-1111.4s",  0x180000, 0x008000, CRC(c951d014) SHA1(596fd0315a5da3d3f6131883d44770724d8e46e3) )  /* bank 4/5, plane 0 */
	ROM_CONTINUE(                0x200000, 0x008000 )
	ROM_LOAD( "136048-1112.3r",  0x190000, 0x008000, CRC(95c5a006) SHA1(0492404c0f092c1bb6239078e3a9ef45fc153923) )  /* bank 4/5, plane 1 */
	ROM_CONTINUE(                0x210000, 0x008000 )
	ROM_LOAD( "136048-1113.4n",  0x1a0000, 0x008000, CRC(f61f2370) SHA1(9f4b2aa91ac5dc0e543e5dfe1151fdea3579c739) )  /* bank 4/5, plane 2 */
	ROM_CONTINUE(                0x220000, 0x008000 )
	ROM_LOAD( "136048-1114.3m",  0x1b0000, 0x008000, CRC(774a36a8) SHA1(ebcf4363174abd834f3e77b0f1a2e29497f470b4) )  /* bank 4/5, plane 3 */
	ROM_CONTINUE(                0x230000, 0x008000 )

	ROM_LOAD( "136048-1115.4r",  0x300000, 0x008000, CRC(a47bc79d) SHA1(367f21c355c0af55629d2c5746cd5cb06272abc0) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x280000, 0x008000 )
	ROM_LOAD( "136048-1116.3p",  0x310000, 0x008000, CRC(b8a5c215) SHA1(2594f2a865fb73c8a7829e50ff19a6494a35c088) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x290000, 0x008000 )
	ROM_LOAD( "136048-1117.4m",  0x320000, 0x008000, CRC(2d1c1f64) SHA1(66141b9a7ae8d6dd3f1315d60cf0e99cd802d2e5) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2a0000, 0x008000 )
	ROM_LOAD( "136048-1118.3l",  0x330000, 0x008000, CRC(be879b8e) SHA1(dc1d1f7bdb511e922b650fac88307a08ab37ac4c) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2b0000, 0x008000 )

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136048-1174.12d", 0x000000, 0x000200, CRC(db4a4d53) SHA1(c5468f3585ec9bc23c9ee990b3ae3738b0309823) )
	ROM_LOAD( "136048-1173.2d",  0x000200, 0x000200, CRC(c80574af) SHA1(9a3dc83f70e79915ce0db3e6e69b5dcfee3acb6f) )

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


ROM_START( roadblst2 )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136048-1139.11c", 0x010000, 0x008000, CRC(b73c1bd5) SHA1(c4de0267a75225db22c771bec14b8da2fc9f06bf) )
	ROM_CONTINUE(                       0x050000, 0x008000 )
	ROM_LOAD16_BYTE( "136048-1140.11a", 0x010001, 0x008000, CRC(6305429b) SHA1(c4180f6438a539ddc34c12529e5ac6d59c107728) )
	ROM_CONTINUE(                       0x050001, 0x008000 )
	ROM_LOAD16_BYTE( "136048-1155.13c", 0x020000, 0x008000, CRC(e95fc7d2) SHA1(1e43350ab8f7f21cdc8454a72aaf8a4adf326367) )
	ROM_CONTINUE(                       0x060000, 0x008000 )
	ROM_LOAD16_BYTE( "136048-1156.13a", 0x020001, 0x008000, CRC(727510f9) SHA1(810582ce50234720a44cd46b09f7febef4daa512) )
	ROM_CONTINUE(                       0x060001, 0x008000 )
	ROM_LOAD16_BYTE( "136048-2145.7n",  0x070000, 0x008000, CRC(c6d30d6f) SHA1(acb552976b2dcfa585097ea246ca88034549c8ab) )
	ROM_LOAD16_BYTE( "136048-2146.8n",  0x070001, 0x008000, CRC(16951020) SHA1(5e5a6ad4ae87723060232c7ecb837f5fc2a9be68) )
	ROM_LOAD16_BYTE( "136048-2147.7k",  0x080000, 0x004000, CRC(5c1adf67) SHA1(53838a2f5059797991aa337a7bec32f7e694610a) )
	ROM_LOAD16_BYTE( "136048-2148.8k",  0x080001, 0x004000, CRC(d9ac8966) SHA1(7d056c1eb8184b4261c5713b0d5799b2fd8bde2a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136048-1149.14e",   0x4000, 0x4000, CRC(2e54f95e) SHA1(5056ddec3c88384ada1d2ee9b1532b9ba9f34e08) )
	ROM_LOAD( "136048-1169.1516e", 0x8000, 0x4000, CRC(ee318052) SHA1(f66ff39499697b7439dc62567e727fec769c1505) )
	ROM_LOAD( "136048-1170.17e",   0xc000, 0x4000, CRC(75dfec33) SHA1(3092348b98419bb23181d21406733d5d21cd3d82) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x380000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136048-1101.2s",  0x000000, 0x008000, CRC(fe342d27) SHA1(72deac16ab9b6b811f49d70d700d6bc3a904f9d5) )  /* bank 1, plane 0 */
	ROM_LOAD( "136048-1102.2r",  0x010000, 0x008000, CRC(17c7e780) SHA1(675a2fe5ac55c97d54153eacabf863a525e4b71f) )  /* bank 1, plane 1 */
	ROM_LOAD( "136048-1103.2n",  0x020000, 0x008000, CRC(39688e01) SHA1(1b273e26b48a2f69b0895c29c9b3e762c686661d) )  /* bank 1, plane 2 */
	ROM_LOAD( "136048-1104.2m",  0x030000, 0x008000, CRC(c8f9bd8e) SHA1(452194efafd5f9562434dafcb14b72f400fc4a49) )  /* bank 1, plane 3 */
	ROM_LOAD( "136048-1105.2k",  0x040000, 0x008000, CRC(c69e439e) SHA1(c933637820c04bdbc56990423adfb9067e09fad7) )  /* bank 1, plane 4 */
	ROM_LOAD( "136048-1106.2j",  0x050000, 0x008000, CRC(4ee55796) SHA1(045635cd4f80a5f2c378cb4c55996e09c21b5859) )  /* bank 1, plane 5 */

	ROM_LOAD( "136048-1107.3s",  0x080000, 0x008000, CRC(02117c58) SHA1(e68eb9fbdbd473f217883a96ad581a94f1df53d9) )  /* bank 2/3, plane 0 */
	ROM_CONTINUE(                0x100000, 0x008000 )
	ROM_LOAD( "136048-1108.2p",  0x090000, 0x008000, CRC(1e148525) SHA1(613c408b352d2d9bac8091bbd5a3c60d09f81084) )  /* bank 2/3, plane 1 */
	ROM_CONTINUE(                0x110000, 0x008000 )
	ROM_LOAD( "136048-1109.3n",  0x0a0000, 0x008000, CRC(110ce07e) SHA1(0d165e18bad935713efc44b8e609c7fa7093f5c8) )  /* bank 2/3, plane 2 */
	ROM_CONTINUE(                0x120000, 0x008000 )
	ROM_LOAD( "136048-1110.2l",  0x0b0000, 0x008000, CRC(c00aa0f4) SHA1(a95e2717e725721b59969499720a8da0cf208e36) )  /* bank 2/3, plane 3 */
	ROM_CONTINUE(                0x130000, 0x008000 )

	ROM_LOAD( "136048-1111.4s",  0x180000, 0x008000, CRC(c951d014) SHA1(596fd0315a5da3d3f6131883d44770724d8e46e3) )  /* bank 4/5, plane 0 */
	ROM_CONTINUE(                0x200000, 0x008000 )
	ROM_LOAD( "136048-1112.3r",  0x190000, 0x008000, CRC(95c5a006) SHA1(0492404c0f092c1bb6239078e3a9ef45fc153923) )  /* bank 4/5, plane 1 */
	ROM_CONTINUE(                0x210000, 0x008000 )
	ROM_LOAD( "136048-1113.4n",  0x1a0000, 0x008000, CRC(f61f2370) SHA1(9f4b2aa91ac5dc0e543e5dfe1151fdea3579c739) )  /* bank 4/5, plane 2 */
	ROM_CONTINUE(                0x220000, 0x008000 )
	ROM_LOAD( "136048-1114.3m",  0x1b0000, 0x008000, CRC(774a36a8) SHA1(ebcf4363174abd834f3e77b0f1a2e29497f470b4) )  /* bank 4/5, plane 3 */
	ROM_CONTINUE(                0x230000, 0x008000 )

	ROM_LOAD( "136048-1115.4r",  0x300000, 0x008000, CRC(a47bc79d) SHA1(367f21c355c0af55629d2c5746cd5cb06272abc0) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x280000, 0x008000 )
	ROM_LOAD( "136048-1116.3p",  0x310000, 0x008000, CRC(b8a5c215) SHA1(2594f2a865fb73c8a7829e50ff19a6494a35c088) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x290000, 0x008000 )
	ROM_LOAD( "136048-1117.4m",  0x320000, 0x008000, CRC(2d1c1f64) SHA1(66141b9a7ae8d6dd3f1315d60cf0e99cd802d2e5) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2a0000, 0x008000 )
	ROM_LOAD( "136048-1118.3l",  0x330000, 0x008000, CRC(be879b8e) SHA1(dc1d1f7bdb511e922b650fac88307a08ab37ac4c) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2b0000, 0x008000 )

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136048-1174.12d", 0x000000, 0x000200, CRC(db4a4d53) SHA1(c5468f3585ec9bc23c9ee990b3ae3738b0309823) )
	ROM_LOAD( "136048-1173.2d",  0x000200, 0x000200, CRC(c80574af) SHA1(9a3dc83f70e79915ce0db3e6e69b5dcfee3acb6f) )

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


ROM_START( roadblstg1 )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136048-1251.11c", 0x010000, 0x008000, CRC(7e94d6a2) SHA1(985b7537dc484975b458a9ae5780e32a5f742d1c) )
	ROM_CONTINUE(                       0x050000, 0x008000 )
	ROM_LOAD16_BYTE( "136048-1252.11a", 0x010001, 0x008000, CRC(d7a66215) SHA1(f9797384f2d01b0dda2ff47c3560ca3489a7a38c) )
	ROM_CONTINUE(                       0x050001, 0x008000 )
	ROM_LOAD16_BYTE( "136048-1253.13c", 0x020000, 0x008000, CRC(342bf326) SHA1(04796f47627b835566153d106fbc521ce9f1476b) )
	ROM_CONTINUE(                       0x060000, 0x008000 )
	ROM_LOAD16_BYTE( "136048-1254.13a", 0x020001, 0x008000, CRC(db8d7495) SHA1(fdbca009ac184f11ac170dbfe0824c3182a0f4b3) )
	ROM_CONTINUE(                       0x060001, 0x008000 )
	ROM_LOAD16_BYTE( "136048-2145.7n",  0x070000, 0x008000, CRC(c6d30d6f) SHA1(acb552976b2dcfa585097ea246ca88034549c8ab) )
	ROM_LOAD16_BYTE( "136048-2146.8n",  0x070001, 0x008000, CRC(16951020) SHA1(5e5a6ad4ae87723060232c7ecb837f5fc2a9be68) )
	ROM_LOAD16_BYTE( "136048-2147.7k",  0x080000, 0x004000, CRC(5c1adf67) SHA1(53838a2f5059797991aa337a7bec32f7e694610a) )
	ROM_LOAD16_BYTE( "136048-2148.8k",  0x080001, 0x004000, CRC(d9ac8966) SHA1(7d056c1eb8184b4261c5713b0d5799b2fd8bde2a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136048-1149.14e",   0x4000, 0x4000, CRC(2e54f95e) SHA1(5056ddec3c88384ada1d2ee9b1532b9ba9f34e08) )
	ROM_LOAD( "136048-1169.1516e", 0x8000, 0x4000, CRC(ee318052) SHA1(f66ff39499697b7439dc62567e727fec769c1505) )
	ROM_LOAD( "136048-1170.17e",   0xc000, 0x4000, CRC(75dfec33) SHA1(3092348b98419bb23181d21406733d5d21cd3d82) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x380000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136048-1101.2s",  0x000000, 0x008000, CRC(fe342d27) SHA1(72deac16ab9b6b811f49d70d700d6bc3a904f9d5) )  /* bank 1, plane 0 */
	ROM_LOAD( "136048-1102.2r",  0x010000, 0x008000, CRC(17c7e780) SHA1(675a2fe5ac55c97d54153eacabf863a525e4b71f) )  /* bank 1, plane 1 */
	ROM_LOAD( "136048-1103.2n",  0x020000, 0x008000, CRC(39688e01) SHA1(1b273e26b48a2f69b0895c29c9b3e762c686661d) )  /* bank 1, plane 2 */
	ROM_LOAD( "136048-1104.2m",  0x030000, 0x008000, CRC(c8f9bd8e) SHA1(452194efafd5f9562434dafcb14b72f400fc4a49) )  /* bank 1, plane 3 */
	ROM_LOAD( "136048-1105.2k",  0x040000, 0x008000, CRC(c69e439e) SHA1(c933637820c04bdbc56990423adfb9067e09fad7) )  /* bank 1, plane 4 */
	ROM_LOAD( "136048-1106.2j",  0x050000, 0x008000, CRC(4ee55796) SHA1(045635cd4f80a5f2c378cb4c55996e09c21b5859) )  /* bank 1, plane 5 */

	ROM_LOAD( "136048-1107.3s",  0x080000, 0x008000, CRC(02117c58) SHA1(e68eb9fbdbd473f217883a96ad581a94f1df53d9) )  /* bank 2/3, plane 0 */
	ROM_CONTINUE(                0x100000, 0x008000 )
	ROM_LOAD( "136048-1108.2p",  0x090000, 0x008000, CRC(1e148525) SHA1(613c408b352d2d9bac8091bbd5a3c60d09f81084) )  /* bank 2/3, plane 1 */
	ROM_CONTINUE(                0x110000, 0x008000 )
	ROM_LOAD( "136048-1109.3n",  0x0a0000, 0x008000, CRC(110ce07e) SHA1(0d165e18bad935713efc44b8e609c7fa7093f5c8) )  /* bank 2/3, plane 2 */
	ROM_CONTINUE(                0x120000, 0x008000 )
	ROM_LOAD( "136048-1110.2l",  0x0b0000, 0x008000, CRC(c00aa0f4) SHA1(a95e2717e725721b59969499720a8da0cf208e36) )  /* bank 2/3, plane 3 */
	ROM_CONTINUE(                0x130000, 0x008000 )

	ROM_LOAD( "136048-1111.4s",  0x180000, 0x008000, CRC(c951d014) SHA1(596fd0315a5da3d3f6131883d44770724d8e46e3) )  /* bank 4/5, plane 0 */
	ROM_CONTINUE(                0x200000, 0x008000 )
	ROM_LOAD( "136048-1112.3r",  0x190000, 0x008000, CRC(95c5a006) SHA1(0492404c0f092c1bb6239078e3a9ef45fc153923) )  /* bank 4/5, plane 1 */
	ROM_CONTINUE(                0x210000, 0x008000 )
	ROM_LOAD( "136048-1113.4n",  0x1a0000, 0x008000, CRC(f61f2370) SHA1(9f4b2aa91ac5dc0e543e5dfe1151fdea3579c739) )  /* bank 4/5, plane 2 */
	ROM_CONTINUE(                0x220000, 0x008000 )
	ROM_LOAD( "136048-1114.3m",  0x1b0000, 0x008000, CRC(774a36a8) SHA1(ebcf4363174abd834f3e77b0f1a2e29497f470b4) )  /* bank 4/5, plane 3 */
	ROM_CONTINUE(                0x230000, 0x008000 )

	ROM_LOAD( "136048-1115.4r",  0x300000, 0x008000, CRC(a47bc79d) SHA1(367f21c355c0af55629d2c5746cd5cb06272abc0) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x280000, 0x008000 )
	ROM_LOAD( "136048-1116.3p",  0x310000, 0x008000, CRC(b8a5c215) SHA1(2594f2a865fb73c8a7829e50ff19a6494a35c088) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x290000, 0x008000 )
	ROM_LOAD( "136048-1117.4m",  0x320000, 0x008000, CRC(2d1c1f64) SHA1(66141b9a7ae8d6dd3f1315d60cf0e99cd802d2e5) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2a0000, 0x008000 )
	ROM_LOAD( "136048-1118.3l",  0x330000, 0x008000, CRC(be879b8e) SHA1(dc1d1f7bdb511e922b650fac88307a08ab37ac4c) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2b0000, 0x008000 )

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136048-1174.12d", 0x000000, 0x000200, CRC(db4a4d53) SHA1(c5468f3585ec9bc23c9ee990b3ae3738b0309823) )
	ROM_LOAD( "136048-1173.2d",  0x000200, 0x000200, CRC(c80574af) SHA1(9a3dc83f70e79915ce0db3e6e69b5dcfee3acb6f) )

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


ROM_START( roadblst1 )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136048-2151.11c", 0x010000, 0x008000, CRC(ea6b3060) SHA1(0786f2e528c6a77ad7422ae199d06b7261cb4f2c) )
	ROM_CONTINUE(                       0x050000, 0x008000 )
	ROM_LOAD16_BYTE( "136048-2152.11a", 0x010001, 0x008000, CRC(f5c1fbe0) SHA1(1917f2110a8021198b0cdadbaeab44c71b3bb0b5) )
	ROM_CONTINUE(                       0x050001, 0x008000 )
	ROM_LOAD16_BYTE( "136048-2153.13c", 0x020000, 0x008000, CRC(11c41698) SHA1(d3a6fe8c20b1142ed38cabbbad81a48ef15bebf7) )
	ROM_CONTINUE(                       0x060000, 0x008000 )
	ROM_LOAD16_BYTE( "136048-2154.13a", 0x020001, 0x008000, CRC(7b947d64) SHA1(d2ccca5d9a9bceb8729e944bce2bc91ff672d751) )
	ROM_CONTINUE(                       0x060001, 0x008000 )
	ROM_LOAD16_BYTE( "136048-2145.7n",  0x070000, 0x008000, CRC(c6d30d6f) SHA1(acb552976b2dcfa585097ea246ca88034549c8ab) )
	ROM_LOAD16_BYTE( "136048-2146.8n",  0x070001, 0x008000, CRC(16951020) SHA1(5e5a6ad4ae87723060232c7ecb837f5fc2a9be68) )
	ROM_LOAD16_BYTE( "136048-2147.7k",  0x080000, 0x004000, CRC(5c1adf67) SHA1(53838a2f5059797991aa337a7bec32f7e694610a) )
	ROM_LOAD16_BYTE( "136048-2148.8k",  0x080001, 0x004000, CRC(d9ac8966) SHA1(7d056c1eb8184b4261c5713b0d5799b2fd8bde2a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136048-1149.14e",   0x4000, 0x4000, CRC(2e54f95e) SHA1(5056ddec3c88384ada1d2ee9b1532b9ba9f34e08) )
	ROM_LOAD( "136048-1169.1516e", 0x8000, 0x4000, CRC(ee318052) SHA1(f66ff39499697b7439dc62567e727fec769c1505) )
	ROM_LOAD( "136048-1170.17e",   0xc000, 0x4000, CRC(75dfec33) SHA1(3092348b98419bb23181d21406733d5d21cd3d82) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x380000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136048-1101.2s",  0x000000, 0x008000, CRC(fe342d27) SHA1(72deac16ab9b6b811f49d70d700d6bc3a904f9d5) )  /* bank 1, plane 0 */
	ROM_LOAD( "136048-1102.2r",  0x010000, 0x008000, CRC(17c7e780) SHA1(675a2fe5ac55c97d54153eacabf863a525e4b71f) )  /* bank 1, plane 1 */
	ROM_LOAD( "136048-1103.2n",  0x020000, 0x008000, CRC(39688e01) SHA1(1b273e26b48a2f69b0895c29c9b3e762c686661d) )  /* bank 1, plane 2 */
	ROM_LOAD( "136048-1104.2m",  0x030000, 0x008000, CRC(c8f9bd8e) SHA1(452194efafd5f9562434dafcb14b72f400fc4a49) )  /* bank 1, plane 3 */
	ROM_LOAD( "136048-1105.2k",  0x040000, 0x008000, CRC(c69e439e) SHA1(c933637820c04bdbc56990423adfb9067e09fad7) )  /* bank 1, plane 4 */
	ROM_LOAD( "136048-1106.2j",  0x050000, 0x008000, CRC(4ee55796) SHA1(045635cd4f80a5f2c378cb4c55996e09c21b5859) )  /* bank 1, plane 5 */

	ROM_LOAD( "136048-1107.3s",  0x080000, 0x008000, CRC(02117c58) SHA1(e68eb9fbdbd473f217883a96ad581a94f1df53d9) )  /* bank 2/3, plane 0 */
	ROM_CONTINUE(                0x100000, 0x008000 )
	ROM_LOAD( "136048-1108.2p",  0x090000, 0x008000, CRC(1e148525) SHA1(613c408b352d2d9bac8091bbd5a3c60d09f81084) )  /* bank 2/3, plane 1 */
	ROM_CONTINUE(                0x110000, 0x008000 )
	ROM_LOAD( "136048-1109.3n",  0x0a0000, 0x008000, CRC(110ce07e) SHA1(0d165e18bad935713efc44b8e609c7fa7093f5c8) )  /* bank 2/3, plane 2 */
	ROM_CONTINUE(                0x120000, 0x008000 )
	ROM_LOAD( "136048-1110.2l",  0x0b0000, 0x008000, CRC(c00aa0f4) SHA1(a95e2717e725721b59969499720a8da0cf208e36) )  /* bank 2/3, plane 3 */
	ROM_CONTINUE(                0x130000, 0x008000 )

	ROM_LOAD( "136048-1111.4s",  0x180000, 0x008000, CRC(c951d014) SHA1(596fd0315a5da3d3f6131883d44770724d8e46e3) )  /* bank 4/5, plane 0 */
	ROM_CONTINUE(                0x200000, 0x008000 )
	ROM_LOAD( "136048-1112.3r",  0x190000, 0x008000, CRC(95c5a006) SHA1(0492404c0f092c1bb6239078e3a9ef45fc153923) )  /* bank 4/5, plane 1 */
	ROM_CONTINUE(                0x210000, 0x008000 )
	ROM_LOAD( "136048-1113.4n",  0x1a0000, 0x008000, CRC(f61f2370) SHA1(9f4b2aa91ac5dc0e543e5dfe1151fdea3579c739) )  /* bank 4/5, plane 2 */
	ROM_CONTINUE(                0x220000, 0x008000 )
	ROM_LOAD( "136048-1114.3m",  0x1b0000, 0x008000, CRC(774a36a8) SHA1(ebcf4363174abd834f3e77b0f1a2e29497f470b4) )  /* bank 4/5, plane 3 */
	ROM_CONTINUE(                0x230000, 0x008000 )

	ROM_LOAD( "136048-1115.4r",  0x300000, 0x008000, CRC(a47bc79d) SHA1(367f21c355c0af55629d2c5746cd5cb06272abc0) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x280000, 0x008000 )
	ROM_LOAD( "136048-1116.3p",  0x310000, 0x008000, CRC(b8a5c215) SHA1(2594f2a865fb73c8a7829e50ff19a6494a35c088) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x290000, 0x008000 )
	ROM_LOAD( "136048-1117.4m",  0x320000, 0x008000, CRC(2d1c1f64) SHA1(66141b9a7ae8d6dd3f1315d60cf0e99cd802d2e5) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2a0000, 0x008000 )
	ROM_LOAD( "136048-1118.3l",  0x330000, 0x008000, CRC(be879b8e) SHA1(dc1d1f7bdb511e922b650fac88307a08ab37ac4c) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2b0000, 0x008000 )

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136048-1174.12d", 0x000000, 0x000200, CRC(db4a4d53) SHA1(c5468f3585ec9bc23c9ee990b3ae3738b0309823) )
	ROM_LOAD( "136048-1173.2d",  0x000200, 0x000200, CRC(c80574af) SHA1(9a3dc83f70e79915ce0db3e6e69b5dcfee3acb6f) )

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


ROM_START( roadblstc )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136048-1179.7p",  0x010000, 0x008000, CRC(ef448f96) SHA1(238a6c435e317383a1983702ef0a1f2b757ebe8c) )
	ROM_LOAD16_BYTE( "136048-1180.8p",  0x010001, 0x008000, CRC(bdb368d5) SHA1(9c79b053708ff619a2733664f881abb849ae2371) )
	ROM_LOAD16_BYTE( "136048-1181.7r",  0x020000, 0x008000, CRC(d52581da) SHA1(d0a475f9d8199bc7c0ce6e6a663f214df9bffc39) )
	ROM_LOAD16_BYTE( "136048-1182.8r",  0x020001, 0x008000, CRC(847788c4) SHA1(8dafc59459f8fd59c242500b5e5f4989f95cc604) )
	ROM_LOAD16_BYTE( "136048-2141.7l",  0x050000, 0x008000, CRC(054273b2) SHA1(4c820c00d3b67825c361edc9615c89c2a9a1c6d3) )
	ROM_LOAD16_BYTE( "136048-2142.8l",  0x050001, 0x008000, CRC(49181bec) SHA1(79e042e4f079a9806ef12c5c8dfdc2e6e4f90011) )
	ROM_LOAD16_BYTE( "136048-2143.7m",  0x060000, 0x008000, CRC(f63dc29a) SHA1(e54637b9d0b271aa9b58e89a442ac03ec812e1eb) )
	ROM_LOAD16_BYTE( "136048-2144.8m",  0x060001, 0x008000, CRC(b1fc5955) SHA1(b860213a9b5ae7547c258812045e71795129598f) )
	ROM_LOAD16_BYTE( "136048-2145.7n",  0x070000, 0x008000, CRC(c6d30d6f) SHA1(acb552976b2dcfa585097ea246ca88034549c8ab) )
	ROM_LOAD16_BYTE( "136048-2146.8n",  0x070001, 0x008000, CRC(16951020) SHA1(5e5a6ad4ae87723060232c7ecb837f5fc2a9be68) )
	ROM_LOAD16_BYTE( "136048-2147.7k",  0x080000, 0x004000, CRC(5c1adf67) SHA1(53838a2f5059797991aa337a7bec32f7e694610a) )
	ROM_LOAD16_BYTE( "136048-2148.8k",  0x080001, 0x004000, CRC(d9ac8966) SHA1(7d056c1eb8184b4261c5713b0d5799b2fd8bde2a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136048-1149.14e",   0x4000, 0x4000, CRC(2e54f95e) SHA1(5056ddec3c88384ada1d2ee9b1532b9ba9f34e08) )
	ROM_LOAD( "136048-1169.1516e", 0x8000, 0x4000, CRC(ee318052) SHA1(f66ff39499697b7439dc62567e727fec769c1505) )
	ROM_LOAD( "136048-1170.17e",   0xc000, 0x4000, CRC(75dfec33) SHA1(3092348b98419bb23181d21406733d5d21cd3d82) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x380000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136048-1101.2s",  0x000000, 0x008000, CRC(fe342d27) SHA1(72deac16ab9b6b811f49d70d700d6bc3a904f9d5) )  /* bank 1, plane 0 */
	ROM_LOAD( "136048-1102.2r",  0x010000, 0x008000, CRC(17c7e780) SHA1(675a2fe5ac55c97d54153eacabf863a525e4b71f) )  /* bank 1, plane 1 */
	ROM_LOAD( "136048-1103.2n",  0x020000, 0x008000, CRC(39688e01) SHA1(1b273e26b48a2f69b0895c29c9b3e762c686661d) )  /* bank 1, plane 2 */
	ROM_LOAD( "136048-1104.2m",  0x030000, 0x008000, CRC(c8f9bd8e) SHA1(452194efafd5f9562434dafcb14b72f400fc4a49) )  /* bank 1, plane 3 */
	ROM_LOAD( "136048-1105.2k",  0x040000, 0x008000, CRC(c69e439e) SHA1(c933637820c04bdbc56990423adfb9067e09fad7) )  /* bank 1, plane 4 */
	ROM_LOAD( "136048-1106.2j",  0x050000, 0x008000, CRC(4ee55796) SHA1(045635cd4f80a5f2c378cb4c55996e09c21b5859) )  /* bank 1, plane 5 */

	ROM_LOAD( "136048-1107.3s",  0x080000, 0x008000, CRC(02117c58) SHA1(e68eb9fbdbd473f217883a96ad581a94f1df53d9) )  /* bank 2/3, plane 0 */
	ROM_CONTINUE(                0x100000, 0x008000 )
	ROM_LOAD( "136048-1108.2p",  0x090000, 0x008000, CRC(1e148525) SHA1(613c408b352d2d9bac8091bbd5a3c60d09f81084) )  /* bank 2/3, plane 1 */
	ROM_CONTINUE(                0x110000, 0x008000 )
	ROM_LOAD( "136048-1109.3n",  0x0a0000, 0x008000, CRC(110ce07e) SHA1(0d165e18bad935713efc44b8e609c7fa7093f5c8) )  /* bank 2/3, plane 2 */
	ROM_CONTINUE(                0x120000, 0x008000 )
	ROM_LOAD( "136048-1110.2l",  0x0b0000, 0x008000, CRC(c00aa0f4) SHA1(a95e2717e725721b59969499720a8da0cf208e36) )  /* bank 2/3, plane 3 */
	ROM_CONTINUE(                0x130000, 0x008000 )

	ROM_LOAD( "136048-1111.4s",  0x180000, 0x008000, CRC(c951d014) SHA1(596fd0315a5da3d3f6131883d44770724d8e46e3) )  /* bank 4/5, plane 0 */
	ROM_CONTINUE(                0x200000, 0x008000 )
	ROM_LOAD( "136048-1112.3r",  0x190000, 0x008000, CRC(95c5a006) SHA1(0492404c0f092c1bb6239078e3a9ef45fc153923) )  /* bank 4/5, plane 1 */
	ROM_CONTINUE(                0x210000, 0x008000 )
	ROM_LOAD( "136048-1113.4n",  0x1a0000, 0x008000, CRC(f61f2370) SHA1(9f4b2aa91ac5dc0e543e5dfe1151fdea3579c739) )  /* bank 4/5, plane 2 */
	ROM_CONTINUE(                0x220000, 0x008000 )
	ROM_LOAD( "136048-1114.3m",  0x1b0000, 0x008000, CRC(774a36a8) SHA1(ebcf4363174abd834f3e77b0f1a2e29497f470b4) )  /* bank 4/5, plane 3 */
	ROM_CONTINUE(                0x230000, 0x008000 )

	ROM_LOAD( "136048-1115.4r",  0x300000, 0x008000, CRC(a47bc79d) SHA1(367f21c355c0af55629d2c5746cd5cb06272abc0) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x280000, 0x008000 )
	ROM_LOAD( "136048-1116.3p",  0x310000, 0x008000, CRC(b8a5c215) SHA1(2594f2a865fb73c8a7829e50ff19a6494a35c088) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x290000, 0x008000 )
	ROM_LOAD( "136048-1117.4m",  0x320000, 0x008000, CRC(2d1c1f64) SHA1(66141b9a7ae8d6dd3f1315d60cf0e99cd802d2e5) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2a0000, 0x008000 )
	ROM_LOAD( "136048-1118.3l",  0x330000, 0x008000, CRC(be879b8e) SHA1(dc1d1f7bdb511e922b650fac88307a08ab37ac4c) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2b0000, 0x008000 )

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136048-1174.12d", 0x000000, 0x000200, CRC(db4a4d53) SHA1(c5468f3585ec9bc23c9ee990b3ae3738b0309823) )
	ROM_LOAD( "136048-1173.2d",  0x000200, 0x000200, CRC(c80574af) SHA1(9a3dc83f70e79915ce0db3e6e69b5dcfee3acb6f) )

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END

/*
Road Blasters (German Cockpit Rev xx)(Atari 1987)
Dumped from an original Atari (Dedicated System 1) cocktail board. "Road Blasters Cart. 5 PCB Assy: A044203-01 Rev.A"

Note: The text on the board says "cocktail" but I suppose this is the cockpit version.
      136048-1150.d7 is a sound rom.
*/

ROM_START( roadblstcg )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	ROM_LOAD16_BYTE( "136032-117-l9",  0x000001, 0x004000, CRC(9af9fe29) SHA1(1d5077662e4111ece9f8a5124394dad8b1abdc13) ) // alt bios roms? (to put in bios structure, or are they specific to this set?)
	ROM_LOAD16_BYTE( "136032-116.m9",  0x000000, 0x004000, CRC(195c54ad) SHA1(d7cda3cd3db4c6f77074ca05e96ae11b62e048b7) ) //

	ROM_LOAD16_BYTE( "136048-1235.7p",  0x010000, 0x008000, CRC(58b2998f) SHA1(7e9f4ca2b15cf60c61e0615f214f9fcc518cb194) )
	ROM_LOAD16_BYTE( "136048-1236.8p",  0x010001, 0x008000, CRC(02e23a40) SHA1(6525351669e95dab869c7adc7d992d12d9313aee) )
	ROM_LOAD16_BYTE( "136048-1237.7r",  0x020000, 0x008000, CRC(5e0a7c5d) SHA1(fb3688fbadc05f96980c67f5446ccb250d20b1a3) )
	ROM_LOAD16_BYTE( "136048-1238.8r",  0x020001, 0x008000, CRC(8c8f9523) SHA1(301481f1fc8169b7a8d51786424e65795a61abb9) )
	ROM_LOAD16_BYTE( "136048-2141.7l",  0x050000, 0x008000, CRC(054273b2) SHA1(4c820c00d3b67825c361edc9615c89c2a9a1c6d3) )
	ROM_LOAD16_BYTE( "136048-2142.8l",  0x050001, 0x008000, CRC(49181bec) SHA1(79e042e4f079a9806ef12c5c8dfdc2e6e4f90011) )
	ROM_LOAD16_BYTE( "136048-2143.7m",  0x060000, 0x008000, CRC(f63dc29a) SHA1(e54637b9d0b271aa9b58e89a442ac03ec812e1eb) )
	ROM_LOAD16_BYTE( "136048-2144.8m",  0x060001, 0x008000, CRC(b1fc5955) SHA1(b860213a9b5ae7547c258812045e71795129598f) )
	ROM_LOAD16_BYTE( "136048-2145.7n",  0x070000, 0x008000, CRC(c6d30d6f) SHA1(acb552976b2dcfa585097ea246ca88034549c8ab) )
	ROM_LOAD16_BYTE( "136048-2146.8n",  0x070001, 0x008000, CRC(16951020) SHA1(5e5a6ad4ae87723060232c7ecb837f5fc2a9be68) )
	ROM_LOAD16_BYTE( "136048-2147.7k",  0x080000, 0x004000, CRC(5c1adf67) SHA1(53838a2f5059797991aa337a7bec32f7e694610a) )
	ROM_LOAD16_BYTE( "136048-2148.8k",  0x080001, 0x004000, CRC(d9ac8966) SHA1(7d056c1eb8184b4261c5713b0d5799b2fd8bde2a) )


	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136048-1149.c8",   0x4000, 0x4000, CRC(2e54f95e) SHA1(5056ddec3c88384ada1d2ee9b1532b9ba9f34e08) ) //
	ROM_LOAD( "136048-1150.d7",   0x8000, 0x8000, CRC(e89e7fc8) SHA1(d944bc27de2f2f675680c62d999cacf84a08fd4b) )

	ROM_REGION( 0x4000, "alpha", 0 )
	ROM_LOAD( "136032-120.p1",   0x00000, 0x04000, CRC(90a1950d) SHA1(fba32c255850312175d1e3c03d677ffb57e09e07) ) //

	ROM_REGION( 0x380000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136048-1101.2s",  0x000000, 0x008000, CRC(fe342d27) SHA1(72deac16ab9b6b811f49d70d700d6bc3a904f9d5) )  /* bank 1, plane 0 */
	ROM_LOAD( "136048-1102.2r",  0x010000, 0x008000, CRC(17c7e780) SHA1(675a2fe5ac55c97d54153eacabf863a525e4b71f) )  /* bank 1, plane 1 */
	ROM_LOAD( "136048-1103.2n",  0x020000, 0x008000, CRC(39688e01) SHA1(1b273e26b48a2f69b0895c29c9b3e762c686661d) )  /* bank 1, plane 2 */
	ROM_LOAD( "136048-1104.2m",  0x030000, 0x008000, CRC(c8f9bd8e) SHA1(452194efafd5f9562434dafcb14b72f400fc4a49) )  /* bank 1, plane 3 */
	ROM_LOAD( "136048-1105.2k",  0x040000, 0x008000, CRC(c69e439e) SHA1(c933637820c04bdbc56990423adfb9067e09fad7) )  /* bank 1, plane 4 */
	ROM_LOAD( "136048-1106.2j",  0x050000, 0x008000, CRC(4ee55796) SHA1(045635cd4f80a5f2c378cb4c55996e09c21b5859) )  /* bank 1, plane 5 */

	ROM_LOAD( "136048-1107.3s",  0x080000, 0x008000, CRC(02117c58) SHA1(e68eb9fbdbd473f217883a96ad581a94f1df53d9) )  /* bank 2/3, plane 0 */
	ROM_CONTINUE(                0x100000, 0x008000 )
	ROM_LOAD( "136048-1108.2p",  0x090000, 0x008000, CRC(1e148525) SHA1(613c408b352d2d9bac8091bbd5a3c60d09f81084) )  /* bank 2/3, plane 1 */
	ROM_CONTINUE(                0x110000, 0x008000 )
	ROM_LOAD( "136048-1109.3n",  0x0a0000, 0x008000, CRC(110ce07e) SHA1(0d165e18bad935713efc44b8e609c7fa7093f5c8) )  /* bank 2/3, plane 2 */
	ROM_CONTINUE(                0x120000, 0x008000 )
	ROM_LOAD( "136048-1110.2l",  0x0b0000, 0x008000, CRC(c00aa0f4) SHA1(a95e2717e725721b59969499720a8da0cf208e36) )  /* bank 2/3, plane 3 */
	ROM_CONTINUE(                0x130000, 0x008000 )

	ROM_LOAD( "136048-1111.4s",  0x180000, 0x008000, CRC(c951d014) SHA1(596fd0315a5da3d3f6131883d44770724d8e46e3) )  /* bank 4/5, plane 0 */
	ROM_CONTINUE(                0x200000, 0x008000 )
	ROM_LOAD( "136048-1112.3r",  0x190000, 0x008000, CRC(95c5a006) SHA1(0492404c0f092c1bb6239078e3a9ef45fc153923) )  /* bank 4/5, plane 1 */
	ROM_CONTINUE(                0x210000, 0x008000 )
	ROM_LOAD( "136048-1113.4n",  0x1a0000, 0x008000, CRC(f61f2370) SHA1(9f4b2aa91ac5dc0e543e5dfe1151fdea3579c739) )  /* bank 4/5, plane 2 */
	ROM_CONTINUE(                0x220000, 0x008000 )
	ROM_LOAD( "136048-1114.3m",  0x1b0000, 0x008000, CRC(774a36a8) SHA1(ebcf4363174abd834f3e77b0f1a2e29497f470b4) )  /* bank 4/5, plane 3 */
	ROM_CONTINUE(                0x230000, 0x008000 )

	ROM_LOAD( "136048-1115.4r",  0x300000, 0x008000, CRC(a47bc79d) SHA1(367f21c355c0af55629d2c5746cd5cb06272abc0) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x280000, 0x008000 )
	ROM_LOAD( "136048-1116.3p",  0x310000, 0x008000, CRC(b8a5c215) SHA1(2594f2a865fb73c8a7829e50ff19a6494a35c088) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x290000, 0x008000 )
	ROM_LOAD( "136048-1117.4m",  0x320000, 0x008000, CRC(2d1c1f64) SHA1(66141b9a7ae8d6dd3f1315d60cf0e99cd802d2e5) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2a0000, 0x008000 )
	ROM_LOAD( "136048-1118.3l",  0x330000, 0x008000, CRC(be879b8e) SHA1(dc1d1f7bdb511e922b650fac88307a08ab37ac4c) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2b0000, 0x008000 )

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	// these 2 proms were present on this board, an earlier revision to the other games, however they don't give correct
	// gfx, so we load the regular ones over, might need different GFX rom loading? investigate further.
	// (the failure to use these roms properly might be the reason we have the IMPERFECT_GRAPHICS flag on this
	// set in the first place)
	ROM_LOAD( "135048-1172.d1", 0x000000, 0x000200, CRC(b79d1903) SHA1(8319fab8b39f708457eb1efe47789b26adc61249) )
	ROM_LOAD( "135048-1171.d2",  0x000200, 0x000200, CRC(29248a95) SHA1(c9467d495676d323121c92c6757e73b2ed3fb3d7) )
	
	ROM_LOAD( "136048-1174.12d", 0x000000, 0x000200, CRC(db4a4d53) SHA1(c5468f3585ec9bc23c9ee990b3ae3738b0309823) )
	ROM_LOAD( "136048-1173.2d",  0x000200, 0x000200, CRC(c80574af) SHA1(9a3dc83f70e79915ce0db3e6e69b5dcfee3acb6f) )

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END


ROM_START( roadblstc1 )
	ROM_REGION( 0x88000, "maincpu", 0 ) /* 8.5*64k for 68000 code & slapstic ROM */
	MOTHERBOARD_BIOS
	ROM_LOAD16_BYTE( "136048-2135.7p",  0x010000, 0x008000, CRC(c0ef86df) SHA1(7dd4d2acba55dc001e009c37fae5a97a53ea1e66) )
	ROM_LOAD16_BYTE( "136048-2136.8p",  0x010001, 0x008000, CRC(9637e2f0) SHA1(86257e1316356c1a7d86bcf7b57bcaff33ac3df5) )
	ROM_LOAD16_BYTE( "136048-2137.7r",  0x020000, 0x008000, CRC(5382ab85) SHA1(1511dfaf8537980e506e4180a23ffcfcfec81451) )
	ROM_LOAD16_BYTE( "136048-2138.8r",  0x020001, 0x008000, CRC(c2c75309) SHA1(b2fb6097a4d361ce863fedf2a98a05c06e3d3b36) )
	ROM_LOAD16_BYTE( "136048-2141.7l",  0x050000, 0x008000, CRC(054273b2) SHA1(4c820c00d3b67825c361edc9615c89c2a9a1c6d3) )
	ROM_LOAD16_BYTE( "136048-2142.8l",  0x050001, 0x008000, CRC(49181bec) SHA1(79e042e4f079a9806ef12c5c8dfdc2e6e4f90011) )
	ROM_LOAD16_BYTE( "136048-2143.7m",  0x060000, 0x008000, CRC(f63dc29a) SHA1(e54637b9d0b271aa9b58e89a442ac03ec812e1eb) )
	ROM_LOAD16_BYTE( "136048-2144.8m",  0x060001, 0x008000, CRC(b1fc5955) SHA1(b860213a9b5ae7547c258812045e71795129598f) )
	ROM_LOAD16_BYTE( "136048-2145.7n",  0x070000, 0x008000, CRC(c6d30d6f) SHA1(acb552976b2dcfa585097ea246ca88034549c8ab) )
	ROM_LOAD16_BYTE( "136048-2146.8n",  0x070001, 0x008000, CRC(16951020) SHA1(5e5a6ad4ae87723060232c7ecb837f5fc2a9be68) )
	ROM_LOAD16_BYTE( "136048-2147.7k",  0x080000, 0x004000, CRC(5c1adf67) SHA1(53838a2f5059797991aa337a7bec32f7e694610a) )
	ROM_LOAD16_BYTE( "136048-2148.8k",  0x080001, 0x004000, CRC(d9ac8966) SHA1(7d056c1eb8184b4261c5713b0d5799b2fd8bde2a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for 6502 code */
	ROM_LOAD( "136048-1149.14e",   0x4000, 0x4000, CRC(2e54f95e) SHA1(5056ddec3c88384ada1d2ee9b1532b9ba9f34e08) )
	ROM_LOAD( "136048-1169.1516e", 0x8000, 0x4000, CRC(ee318052) SHA1(f66ff39499697b7439dc62567e727fec769c1505) )
	ROM_LOAD( "136048-1170.17e",   0xc000, 0x4000, CRC(75dfec33) SHA1(3092348b98419bb23181d21406733d5d21cd3d82) )

	ROM_REGION( 0x2000, "alpha", 0 )
	MOTHERBOARD_ALPHA

	ROM_REGION( 0x380000, "tiles", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "136048-1101.2s",  0x000000, 0x008000, CRC(fe342d27) SHA1(72deac16ab9b6b811f49d70d700d6bc3a904f9d5) )  /* bank 1, plane 0 */
	ROM_LOAD( "136048-1102.2r",  0x010000, 0x008000, CRC(17c7e780) SHA1(675a2fe5ac55c97d54153eacabf863a525e4b71f) )  /* bank 1, plane 1 */
	ROM_LOAD( "136048-1103.2n",  0x020000, 0x008000, CRC(39688e01) SHA1(1b273e26b48a2f69b0895c29c9b3e762c686661d) )  /* bank 1, plane 2 */
	ROM_LOAD( "136048-1104.2m",  0x030000, 0x008000, CRC(c8f9bd8e) SHA1(452194efafd5f9562434dafcb14b72f400fc4a49) )  /* bank 1, plane 3 */
	ROM_LOAD( "136048-1105.2k",  0x040000, 0x008000, CRC(c69e439e) SHA1(c933637820c04bdbc56990423adfb9067e09fad7) )  /* bank 1, plane 4 */
	ROM_LOAD( "136048-1106.2j",  0x050000, 0x008000, CRC(4ee55796) SHA1(045635cd4f80a5f2c378cb4c55996e09c21b5859) )  /* bank 1, plane 5 */

	ROM_LOAD( "136048-1107.3s",  0x080000, 0x008000, CRC(02117c58) SHA1(e68eb9fbdbd473f217883a96ad581a94f1df53d9) )  /* bank 2/3, plane 0 */
	ROM_CONTINUE(                0x100000, 0x008000 )
	ROM_LOAD( "136048-1108.2p",  0x090000, 0x008000, CRC(1e148525) SHA1(613c408b352d2d9bac8091bbd5a3c60d09f81084) )  /* bank 2/3, plane 1 */
	ROM_CONTINUE(                0x110000, 0x008000 )
	ROM_LOAD( "136048-1109.3n",  0x0a0000, 0x008000, CRC(110ce07e) SHA1(0d165e18bad935713efc44b8e609c7fa7093f5c8) )  /* bank 2/3, plane 2 */
	ROM_CONTINUE(                0x120000, 0x008000 )
	ROM_LOAD( "136048-1110.2l",  0x0b0000, 0x008000, CRC(c00aa0f4) SHA1(a95e2717e725721b59969499720a8da0cf208e36) )  /* bank 2/3, plane 3 */
	ROM_CONTINUE(                0x130000, 0x008000 )

	ROM_LOAD( "136048-1111.4s",  0x180000, 0x008000, CRC(c951d014) SHA1(596fd0315a5da3d3f6131883d44770724d8e46e3) )  /* bank 4/5, plane 0 */
	ROM_CONTINUE(                0x200000, 0x008000 )
	ROM_LOAD( "136048-1112.3r",  0x190000, 0x008000, CRC(95c5a006) SHA1(0492404c0f092c1bb6239078e3a9ef45fc153923) )  /* bank 4/5, plane 1 */
	ROM_CONTINUE(                0x210000, 0x008000 )
	ROM_LOAD( "136048-1113.4n",  0x1a0000, 0x008000, CRC(f61f2370) SHA1(9f4b2aa91ac5dc0e543e5dfe1151fdea3579c739) )  /* bank 4/5, plane 2 */
	ROM_CONTINUE(                0x220000, 0x008000 )
	ROM_LOAD( "136048-1114.3m",  0x1b0000, 0x008000, CRC(774a36a8) SHA1(ebcf4363174abd834f3e77b0f1a2e29497f470b4) )  /* bank 4/5, plane 3 */
	ROM_CONTINUE(                0x230000, 0x008000 )

	ROM_LOAD( "136048-1115.4r",  0x300000, 0x008000, CRC(a47bc79d) SHA1(367f21c355c0af55629d2c5746cd5cb06272abc0) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x280000, 0x008000 )
	ROM_LOAD( "136048-1116.3p",  0x310000, 0x008000, CRC(b8a5c215) SHA1(2594f2a865fb73c8a7829e50ff19a6494a35c088) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x290000, 0x008000 )
	ROM_LOAD( "136048-1117.4m",  0x320000, 0x008000, CRC(2d1c1f64) SHA1(66141b9a7ae8d6dd3f1315d60cf0e99cd802d2e5) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2a0000, 0x008000 )
	ROM_LOAD( "136048-1118.3l",  0x330000, 0x008000, CRC(be879b8e) SHA1(dc1d1f7bdb511e922b650fac88307a08ab37ac4c) )  /* bank 7/6, plane 0 */
	ROM_CONTINUE(                0x2b0000, 0x008000 )

	ROM_REGION( 0x400, "proms", 0 ) /* graphics mapping PROMs */
	ROM_LOAD( "136048-1174.12d", 0x000000, 0x000200, CRC(db4a4d53) SHA1(c5468f3585ec9bc23c9ee990b3ae3738b0309823) )
	ROM_LOAD( "136048-1173.2d",  0x000200, 0x000200, CRC(c80574af) SHA1(9a3dc83f70e79915ce0db3e6e69b5dcfee3acb6f) )

	ROM_REGION( 0x201, "motherbrd_proms", 0) /* Motherboard PROM's (Only used by TTL version.) */
	MOTHERBOARD_PROMS
ROM_END






/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(atarisy1_state,marble)
{
	slapstic_configure(*m_maincpu, 0x080000, 0, 103);

	m_joystick_type = 0;    /* none */
	m_trackball_type = 1;   /* rotated */
}


DRIVER_INIT_MEMBER(atarisy1_state,peterpak)
{
	slapstic_configure(*m_maincpu, 0x080000, 0, 107);

	m_joystick_type = 1;    /* digital */
	m_trackball_type = 0;   /* none */
}


DRIVER_INIT_MEMBER(atarisy1_state,indytemp)
{
	slapstic_configure(*m_maincpu, 0x080000, 0, 105);

	m_joystick_type = 1;    /* digital */
	m_trackball_type = 0;   /* none */
}


DRIVER_INIT_MEMBER(atarisy1_state,roadrunn)
{
	slapstic_configure(*m_maincpu, 0x080000, 0, 108);

	m_joystick_type = 2;    /* analog */
	m_trackball_type = 0;   /* none */
}


DRIVER_INIT_MEMBER(atarisy1_state,roadb109)
{
	slapstic_configure(*m_maincpu, 0x080000, 0, 109);

	m_joystick_type = 3;    /* pedal */
	m_trackball_type = 2;   /* steering wheel */
}


DRIVER_INIT_MEMBER(atarisy1_state,roadb110)
{
	slapstic_configure(*m_maincpu, 0x080000, 0, 110);

	m_joystick_type = 3;    /* pedal */
	m_trackball_type = 2;   /* steering wheel */
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1984, atarisy1, 0,        atarisy1, peterpak, atarisy1_state, peterpak, ROT0, "Atari Games", "Atari System 1 BIOS", GAME_IS_BIOS_ROOT )

GAME( 1984, marble,   atarisy1, atarisy1, marble, atarisy1_state,   marble,   ROT0, "Atari Games", "Marble Madness (set 1)", 0 )
GAME( 1984, marble2,  marble,   atarisy1, marble, atarisy1_state,   marble,   ROT0, "Atari Games", "Marble Madness (set 2)", 0 )
GAME( 1984, marble3,  marble,   atarisy1, marble, atarisy1_state,   marble,   ROT0, "Atari Games", "Marble Madness (set 3)", 0 )
GAME( 1984, marble4,  marble,   atarisy1, marble, atarisy1_state,   marble,   ROT0, "Atari Games", "Marble Madness (set 4)", 0 )
GAME( 1984, marble5,  marble,   atarisy1, marble, atarisy1_state,   marble,   ROT0, "Atari Games", "Marble Madness (set 5 - LSI Cartridge)", 0 )

GAME( 1984, peterpak, atarisy1, atarisy1, peterpak, atarisy1_state, peterpak, ROT0, "Atari Games", "Peter Pack-Rat", 0 )

GAME( 1985, indytemp, atarisy1, atarisy1, indytemp, atarisy1_state, indytemp, ROT0, "Atari Games", "Indiana Jones and the Temple of Doom (set 1)", 0 )
GAME( 1985, indytemp2,indytemp, atarisy1, indytemp, atarisy1_state, indytemp, ROT0, "Atari Games", "Indiana Jones and the Temple of Doom (set 2)", 0 )
GAME( 1985, indytemp3,indytemp, atarisy1, indytemp, atarisy1_state, indytemp, ROT0, "Atari Games", "Indiana Jones and the Temple of Doom (set 3)", 0 )
GAME( 1985, indytemp4,indytemp, atarisy1, indytemp, atarisy1_state, indytemp, ROT0, "Atari Games", "Indiana Jones and the Temple of Doom (set 4)", 0 )
GAME( 1985, indytempd,indytemp, atarisy1, indytemp, atarisy1_state, indytemp, ROT0, "Atari Games", "Indiana Jones and the Temple of Doom (German)", 0 )
GAME( 1985, indytempc,indytemp, atarisy1, indytemc, atarisy1_state, indytemp, ROT0, "Atari Games", "Indiana Jones and the Temple of Doom (Cocktail)", GAME_IMPERFECT_GRAPHICS )

GAME( 1985, roadrunn, atarisy1, atarisy1, roadrunn, atarisy1_state, roadrunn, ROT0, "Atari Games", "Road Runner (rev 2)", 0 )
GAME( 1985, roadrunn2,roadrunn, atarisy1, roadrunn, atarisy1_state, roadrunn, ROT0, "Atari Games", "Road Runner (rev 1+)", 0 )
GAME( 1985, roadrunn1,roadrunn, atarisy1, roadrunn, atarisy1_state, roadrunn, ROT0, "Atari Games", "Road Runner (rev 1)", 0 )

GAME( 1987, roadblst,   atarisy1, atarisy1, roadblst, atarisy1_state, roadb110, ROT0, "Atari Games", "Road Blasters (upright, rev 4)", 0 )
GAME( 1987, roadblstg,  roadblst, atarisy1, roadblst, atarisy1_state, roadb109, ROT0, "Atari Games", "Road Blasters (upright, German, rev 3)", 0 )
GAME( 1987, roadblst3,  roadblst, atarisy1, roadblst, atarisy1_state, roadb109, ROT0, "Atari Games", "Road Blasters (upright, rev 3)", 0 )
GAME( 1987, roadblstg2, roadblst, atarisy1, roadblst, atarisy1_state, roadb110, ROT0, "Atari Games", "Road Blasters (upright, German, rev 2)", 0 )
GAME( 1987, roadblst2,  roadblst, atarisy1, roadblst, atarisy1_state, roadb110, ROT0, "Atari Games", "Road Blasters (upright, rev 2)", 0 )
GAME( 1987, roadblstg1, roadblst, atarisy1, roadblst, atarisy1_state, roadb109, ROT0, "Atari Games", "Road Blasters (upright, German, rev 1)", 0 )
GAME( 1987, roadblst1,  roadblst, atarisy1, roadblst, atarisy1_state, roadb109, ROT0, "Atari Games", "Road Blasters (upright, rev 1)", 0 )
GAME( 1987, roadblstc,  roadblst, atarisy1, roadblst, atarisy1_state, roadb110, ROT0, "Atari Games", "Road Blasters (cockpit, rev 2)", 0 )
GAME( 1987, roadblstcg, roadblst, atarisy1, roadblst, atarisy1_state, roadb109, ROT0, "Atari Games", "Road Blasters (cockpit, German, rev 1)", GAME_IMPERFECT_GRAPHICS )
GAME( 1987, roadblstc1, roadblst, atarisy1, roadblst, atarisy1_state, roadb109, ROT0, "Atari Games", "Road Blasters (cockpit, rev 1)", GAME_IMPERFECT_GRAPHICS )
GAME( 1987, roadblstgu, roadblst, atarisy1, roadblst, atarisy1_state, roadb109, ROT0, "Atari Games", "Road Blasters (upright, German, rev ?)", 0 )

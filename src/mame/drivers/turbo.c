// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn, Howie Cohen, Frank Palazzolo, Ernesto Corvi, Aaron Giles
/*************************************************************************

    Sega Z80-3D system

    driver by Alex Pasadyn, Howie Cohen, Frank Palazzolo, Ernesto Corvi,
    and Aaron Giles

    Games supported:
        * Turbo
        * Subroc 3D
        * Buck Rogers: Planet of Zoom

**************************************************************************
    TURBO
**************************************************************************

    Memory Map:  ( * not complete * )

    Address Range:  R/W:     Function:
    --------------------------------------------------------------------------
    0000 - 5fff     R        Program ROM
    a000 - a0ff     W        Sprite RAM
    a800 - a803     W        Lamps / Coin Meters
    b000 - b1ff     R/W      Collision RAM
    e000 - e7ff     R/W      character RAM
    f000 - f7ff     R/W      RAM
    f202                     coinage 2
    f205                     coinage 1
    f800 - f803     R/W      road drawing
    f900 - f903     R/W      road drawing
    fa00 - fa03     R/W      sound
    fb00 - fb03     R/W      x,DS2,x,x
    fc00 - fc01     R        DS1,x
    fc00 - fc01     W        score
    fd00            R        Coin Inputs, etc.
    fe00            R        DS3,x

    Switch settings:
    Notes:
        1) Facing the CPU board, with the two large IDC connectors at
           the top of the board, and the large and small IDC
           connectors at the bottom, DIP switch #1 is upper right DIP
           switch, DIP switch #2 is the DIP switch to the right of it.

        2) Facing the Sound board, with the IDC connector at the
           bottom of the board, DIP switch #3 (4 bank) can be seen.
    ----------------------------------------------------------------------------

    Option     (DIP Swtich #1) | SW1 | SW2 | SW3 | SW4 | SW5 | SW6 | SW7 | SW8 |
     --------------------------|-----|-----|-----|-----|-----|-----|-----|-----|
    1 Car On Extended Play     | ON  | ON  |     |     |     |     |     |     |
    2 Car On Extended Play     | OFF | ON  |     |     |     |     |     |     |
    3 Car On Extended Play     | ON  | OFF |     |     |     |     |     |     |
    4 Car On Extended Play     | OFF | OFF |     |     |     |     |     |     |
     --------------------------|-----|-----|-----|-----|-----|-----|-----|-----|
    Game Time Adjustable       |     |     | ON  |     |     |     |     |     |
    Game Time Fixed (55 Sec.)  |     |     | OFF |     |     |     |     |     |
     --------------------------|-----|-----|-----|-----|-----|-----|-----|-----|
    Hard Game Difficulty       |     |     |     | ON  |     |     |     |     |
    Easy Game Difficulty       |     |     |     | OFF |     |     |     |     |
     --------------------------|-----|-----|-----|-----|-----|-----|-----|-----|
    Normal Game Mode           |     |     |     |     | ON  |     |     |     |
    No Collisions (cheat)      |     |     |     |     | OFF |     |     |     |
     --------------------------|-----|-----|-----|-----|-----|-----|-----|-----|
    Initial Entry Off (?)      |     |     |     |     |     | ON  |     |     |
    Initial Entry On  (?)      |     |     |     |     |     | OFF |     |     |
     --------------------------|-----|-----|-----|-----|-----|-----|-----|-----|
    Not Used                   |     |     |     |     |     |     |  X  |  X  |
    ---------------------------------------------------------------------------

    Option     (DIP Swtich #2) | SW1 | SW2 | SW3 | SW4 | SW5 | SW6 | SW7 | SW8 |
    --------------------------|-----|-----|-----|-----|-----|-----|-----|-----|
    60 Seconds Game Time       | ON  | ON  |     |     |     |     |     |     |
    70 Seconds Game Time       | OFF | ON  |     |     |     |     |     |     |
    80 Seconds Game Time       | ON  | OFF |     |     |     |     |     |     |
    90 Seconds Game Time       | OFF | OFF |     |     |     |     |     |     |
     --------------------------|-----|-----|-----|-----|-----|-----|-----|-----|
    Slot 1   1 Coin  1 Credit  |     |     | ON  | ON  | ON  |     |     |     |
    Slot 1   1 Coin  2 Credits |     |     | OFF | ON  | ON  |     |     |     |
    Slot 1   1 Coin  3 Credits |     |     | ON  | OFF | ON  |     |     |     |
    Slot 1   1 Coin  6 Credits |     |     | OFF | OFF | ON  |     |     |     |
    Slot 1   2 Coins 1 Credit  |     |     | ON  | ON  | OFF |     |     |     |
    Slot 1   3 Coins 1 Credit  |     |     | OFF | ON  | OFF |     |     |     |
    Slot 1   4 Coins 1 Credit  |     |     | ON  | OFF | OFF |     |     |     |
    Slot 1   1 Coin  1 Credit  |     |     | OFF | OFF | OFF |     |     |     |
     --------------------------|-----|-----|-----|-----|-----|-----|-----|-----|
    Slot 2   1 Coin  1 Credit  |     |     |     |     |     | ON  | ON  | ON  |
    Slot 2   1 Coin  2 Credits |     |     |     |     |     | OFF | ON  | ON  |
    Slot 2   1 Coin  3 Credits |     |     |     |     |     | ON  | OFF | ON  |
    Slot 2   1 Coin  6 Credits |     |     |     |     |     | OFF | OFF | ON  |
    Slot 2   2 Coins 1 Credit  |     |     |     |     |     | ON  | ON  | OFF |
    Slot 2   3 Coins 1 Credit  |     |     |     |     |     | OFF | ON  | OFF |
    Slot 2   4 Coins 1 Credit  |     |     |     |     |     | ON  | OFF | OFF |
    Slot 2   1 Coins 1 Credit  |     |     |     |     |     | OFF | OFF | OFF |
    ---------------------------------------------------------------------------

    Option     (DIP Swtich #3) | SW1 | SW2 | SW3 | SW4 |
     --------------------------|-----|-----|-----|-----|
    Not Used                   |  X  |  X  |     |     |
     --------------------------|-----|-----|-----|-----|
    Digital (LED) Tachometer   |     |     | ON  |     |
    Analog (Meter) Tachometer  |     |     | OFF |     |
     --------------------------|-----|-----|-----|-----|
    Cockpit Sound System       |     |     |     | ON  |
    Upright Sound System       |     |     |     | OFF |
    ---------------------------------------------------

    Here is a complete list of the ROMs:

    Turbo ROMLIST - Frank Palazzolo
    Name     Loc             Function
    -----------------------------------------------------------------------------
    Images Acquired:
    EPR1262,3,4  IC76, IC89, IC103
    EPR1363,4,5
    EPR15xx             Program ROMS
    EPR1244             Character Data 1
    EPR1245             Character Data 2
    EPR-1125            Road ROMS
    EPR-1126
    EPR-1127
    EPR-1238
    EPR-1239
    EPR-1240
    EPR-1241
    EPR-1242
    EPR-1243
    EPR1246-1258        Sprite ROMS
    EPR1288-1300

    PR-1114     IC13    Color 1 (road, etc.)
    PR-1115     IC18    Road gfx
    PR-1116     IC20    Crash (collision detection?)
    PR-1117     IC21    Color 2 (road, etc.)
    PR-1118     IC99    256x4 Character Color PROM
    PR-1119     IC50    512x8 Vertical Timing PROM
    PR-1120     IC62    Horizontal Timing PROM
    PR-1121     IC29    Color PROM
    PR-1122     IC11    Pattern 1
    PR-1123     IC21    Pattern 2

    PA-06R      IC22    Mathbox Timing PAL
    PA-06L      IC90    Address Decode PAL

**************************************************************************/

#include "emu.h"
#include "includes/turbo.h"
#include "machine/i8279.h"
#include "machine/segacrpt.h"
#include "sound/samples.h"

#include "turbo.lh"
#include "subroc3d.lh"
#include "buckrog.lh"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define MASTER_CLOCK        (XTAL_19_968MHz)

#define PIXEL_CLOCK         (MASTER_CLOCK/4*TURBO_X_SCALE)

#define HTOTAL              (320*TURBO_X_SCALE)
#define HBEND               (0)
#define HBSTART             (256*TURBO_X_SCALE)

#define VTOTAL              (264)
#define VBEND               (0)
#define VBSTART             (224)







/*************************************
 *
 *  Machine init
 *
 *************************************/

MACHINE_RESET_MEMBER(turbo_state,buckrog)
{
	m_buckrog_command = 0x00;
}



/*************************************
 *
 *  Turbo 8255 PPI handling
 *
 *************************************/

/*
    chip index:
        0 = IC75 - CPU Board, Sheet 6, D7
        1 = IC32 - CPU Board, Sheet 6, D6
        2 = IC123 - CPU Board, Sheet 6, D4
        3 = IC6 - CPU Board, Sheet 5, D7
*/

WRITE8_MEMBER(turbo_state::turbo_ppi0a_w)
{
	/* bit0-7 = 0PA0-7 */
	m_turbo_opa = data;
}


WRITE8_MEMBER(turbo_state::turbo_ppi0b_w)
{
	/* bit0-7 = 0PB0-7 */
	m_turbo_opb = data;
}


WRITE8_MEMBER(turbo_state::turbo_ppi0c_w)
{
	/* bit0-7 = 0PC0-7 */
	m_turbo_opc = data;
}


WRITE8_MEMBER(turbo_state::turbo_ppi1a_w)
{
	/* bit0-7 = 1PA0-7 */
	m_turbo_ipa = data;
}


WRITE8_MEMBER(turbo_state::turbo_ppi1b_w)
{
	/* bit0-7 = 1PB0-7 */
	m_turbo_ipb = data;
}


WRITE8_MEMBER(turbo_state::turbo_ppi1c_w)
{
	/* bit0-7 = 1PC0-7 */
	m_turbo_ipc = data;
}


WRITE8_MEMBER(turbo_state::turbo_ppi3c_w)
{
	/* bit 0-3 = PLA0-3 */
	/* bit 4-6 = COL0-2 */
	/* bit   7 = n/c */
	m_turbo_fbpla = data & 0x0f;
	m_turbo_fbcol = (data >> 4) & 0x07;
}


/*************************************
 *
 *  Subroc 3D PPI handling
 *
 *************************************/

/*
    chip index:
        0 = IC117 - CPU Board, Sheet 3, A6
        1 = IC119 - CPU Board, Sheet 3, A5
*/

WRITE8_MEMBER(turbo_state::subroc3d_ppi0a_w)
{
	/* bit 0-3 = PLY0-3 */
	/* bit 4-7 = n/c */
	m_subroc3d_ply = data & 0x0f;
}


WRITE8_MEMBER(turbo_state::subroc3d_ppi0c_w)
{
	/* bit 0-3 = COL0-3 */
	m_subroc3d_col = data & 0x0f;
}


WRITE8_MEMBER(turbo_state::subroc3d_ppi0b_w)
{
	/* bit 0 = COM1 (COIN METER 1) */
	/* bit 1 = COM2 (COIN METER 2) */
	/* bit 2 = STLA (START LAMP) */
	/* bit 3 = NOUSE (n/c) */
	/* bit 4 = FLIP (not really flip, just offset) */
	coin_counter_w(machine(), 0, data & 0x01);
	coin_counter_w(machine(), 1, data & 0x02);
	set_led_status(machine(), 0, data & 0x04);
	m_subroc3d_flip = (data >> 4) & 1;
}

/*************************************
 *
 *  Buck Rogers PPI handling
 *
 *************************************/

WRITE8_MEMBER(turbo_state::buckrog_ppi0a_w)
{
	/* bit 0-7 = data to be read on the /IOREQ */
	m_buckrog_command = data;
}


WRITE8_MEMBER(turbo_state::buckrog_ppi0b_w)
{
	/* bit 0-5 = MOV0-5 */
	/* bit 6-7 = n/c */
	m_buckrog_mov = data & 0x3f;
}


WRITE8_MEMBER(turbo_state::buckrog_ppi0c_w)
{
	/* bit 0-2 = FCHG0 */
	/* bit 3-5 = n/c */
	/* bit   6 = /IOREQ on the 2nd CPU */
	/* bit   7 = /INT on the 2nd CPU */
	m_buckrog_fchg = data & 0x07;
	m_subcpu->set_input_line(0, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
}


WRITE8_MEMBER(turbo_state::buckrog_ppi1c_w)
{
	/* bit 0-2 = OBCH0-2 */
	/* bit   3 = n/c */
	/* bit   4 = COM1 (COIN METER 1) */
	/* bit   5 = COM2 (COIN METER 2) */
	/* bit   6 = STLA (START LAMP) */
	/* bit   7 = NOUSE (BODY SONIC) */
	m_buckrog_obch = data & 0x07;
	coin_counter_w(machine(), 0, data & 0x10);
	coin_counter_w(machine(), 1, data & 0x20);
	set_led_status(machine(), 0, data & 0x40);
}


/*************************************
 *
 *  8279 display/keyboard driver
 *
 *************************************/

WRITE8_MEMBER( turbo_state::scanlines_w )
{
	m_i8279_scanlines = data;
}

WRITE8_MEMBER( turbo_state::digit_w )
{
	static const UINT8 ls48_map[16] =
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };

	output_set_digit_value(m_i8279_scanlines * 2 + 0, ls48_map[data & 0x0f]);
	output_set_digit_value(m_i8279_scanlines * 2 + 1, ls48_map[(data>>4) & 0x0f]);
}

/*************************************
 *
 *  Misc Turbo inputs/outputs
 *
 *************************************/

READ8_MEMBER(turbo_state::turbo_collision_r)
{
	m_screen->update_partial(m_screen->vpos());
	return ioport("DSW3")->read() | (m_turbo_collision & 15);
}


WRITE8_MEMBER(turbo_state::turbo_collision_clear_w)
{
	m_screen->update_partial(m_screen->vpos());
	m_turbo_collision = 0;
}


READ8_MEMBER(turbo_state::turbo_analog_r)
{
	return ioport("DIAL")->read() - m_turbo_last_analog;
}


WRITE8_MEMBER(turbo_state::turbo_analog_reset_w)
{
	m_turbo_last_analog = ioport("DIAL")->read();
}


WRITE8_MEMBER(turbo_state::turbo_coin_and_lamp_w)
{
	switch (offset & 7)
	{
		case 0:
			coin_counter_w(machine(), 0, data & 1);
			break;
		case 1:
			coin_counter_w(machine(), 1, data & 1);
			break;
		case 3:
			set_led_status(machine(), 0, data & 1);
			break;
	}
}



/*************************************
 *
 *  Misc Buck Rogers inputs/outputs
 *
 *************************************/

READ8_MEMBER(turbo_state::buckrog_cpu2_command_r)
{
	/* assert ACK */
	m_i8255_0->pc6_w(CLEAR_LINE);
	return m_buckrog_command;
}


READ8_MEMBER(turbo_state::buckrog_port_2_r)
{
	int inp1 = ioport("DSW1")->read();
	int inp2 = ioport("DSW2")->read();

	return  (((inp2 >> 6) & 1) << 7) |
			(((inp2 >> 4) & 1) << 6) |
			(((inp2 >> 3) & 1) << 5) |
			(((inp2 >> 0) & 1) << 4) |
			(((inp1 >> 6) & 1) << 3) |
			(((inp1 >> 4) & 1) << 2) |
			(((inp1 >> 3) & 1) << 1) |
			(((inp1 >> 0) & 1) << 0);
}


READ8_MEMBER(turbo_state::buckrog_port_3_r)
{
	int inp1 = ioport("DSW1")->read();
	int inp2 = ioport("DSW2")->read();

	return  (((inp2 >> 7) & 1) << 7) |
			(((inp2 >> 5) & 1) << 6) |
			(((inp2 >> 2) & 1) << 5) |
			(((inp2 >> 1) & 1) << 4) |
			(((inp1 >> 7) & 1) << 3) |
			(((inp1 >> 5) & 1) << 2) |
			(((inp1 >> 2) & 1) << 1) |
			(((inp1 >> 1) & 1) << 0);
}


TIMER_CALLBACK_MEMBER(turbo_state::delayed_i8255_w)
{
	m_i8255_0->write(m_maincpu->space(AS_PROGRAM), param >> 8, param & 0xff);
}


WRITE8_MEMBER(turbo_state::buckrog_i8255_0_w)
{
	/* the port C handshaking signals control the sub CPU IRQ, */
	/* so we have to sync whenever we access this PPI */
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(turbo_state::delayed_i8255_w),this), ((offset & 3) << 8) | (data & 0xff));
}



/*************************************
 *
 *  Turbo CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( turbo_map, AS_PROGRAM, 8, turbo_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0xa000, 0xa0ff) AM_MIRROR(0x0700) AM_MASK(0x0f7) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xa800, 0xa807) AM_MIRROR(0x07f8) AM_WRITE(turbo_coin_and_lamp_w)
	AM_RANGE(0xb000, 0xb3ff) AM_MIRROR(0x0400) AM_RAM AM_SHARE("spritepos")
	AM_RANGE(0xb800, 0xbfff) AM_WRITE(turbo_analog_reset_w)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(turbo_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xe800, 0xefff) AM_WRITE(turbo_collision_clear_w)
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xf803) AM_MIRROR(0x00fc) AM_DEVREADWRITE("i8255_0", i8255_device, read, write)
	AM_RANGE(0xf900, 0xf903) AM_MIRROR(0x00fc) AM_DEVREADWRITE("i8255_1", i8255_device, read, write)
	AM_RANGE(0xfa00, 0xfa03) AM_MIRROR(0x00fc) AM_DEVREADWRITE("i8255_2", i8255_device, read, write)
	AM_RANGE(0xfb00, 0xfb03) AM_MIRROR(0x00fc) AM_DEVREADWRITE("i8255_3", i8255_device, read, write)
	AM_RANGE(0xfc00, 0xfc00) AM_MIRROR(0x00fe) AM_DEVREADWRITE("i8279", i8279_device, data_r, data_w )
	AM_RANGE(0xfc01, 0xfc01) AM_MIRROR(0x00fe) AM_DEVREADWRITE("i8279", i8279_device, status_r, cmd_w)
	AM_RANGE(0xfd00, 0xfdff) AM_READ_PORT("INPUT")
	AM_RANGE(0xfe00, 0xfeff) AM_READ(turbo_collision_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Subroc3D CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( subroc3d_map, AS_PROGRAM, 8, turbo_state )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xa3ff) AM_RAM AM_SHARE("spritepos")                           // CONT RAM
	AM_RANGE(0xa400, 0xa7ff) AM_RAM AM_SHARE("spriteram")                           // CONT RAM
	AM_RANGE(0xa800, 0xa800) AM_MIRROR(0x07fc) AM_READ_PORT("IN0")                  // INPUT 253
	AM_RANGE(0xa801, 0xa801) AM_MIRROR(0x07fc) AM_READ_PORT("IN1")                  // INPUT 253
	AM_RANGE(0xa802, 0xa802) AM_MIRROR(0x07fc) AM_READ_PORT("DSW2")                 // INPUT 253
	AM_RANGE(0xa803, 0xa803) AM_MIRROR(0x07fc) AM_READ_PORT("DSW3")                 // INPUT 253
	AM_RANGE(0xb000, 0xb7ff) AM_RAM                                                 // SCRATCH
	AM_RANGE(0xb800, 0xbfff)                                                        // HANDLE CL
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(turbo_videoram_w) AM_SHARE("videoram")    // FIX PAGE
	AM_RANGE(0xe800, 0xe803) AM_MIRROR(0x07fc) AM_DEVREADWRITE("i8255_0", i8255_device, read, write)
	AM_RANGE(0xf000, 0xf003) AM_MIRROR(0x07fc) AM_DEVREADWRITE("i8255_1", i8255_device, read, write)
	AM_RANGE(0xf800, 0xf800) AM_MIRROR(0x07fe) AM_DEVREADWRITE("i8279", i8279_device, data_r, data_w )
	AM_RANGE(0xf801, 0xf801) AM_MIRROR(0x07fe) AM_DEVREADWRITE("i8279", i8279_device, status_r, cmd_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Buck Rogers CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( buckrog_map, AS_PROGRAM, 8, turbo_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(turbo_videoram_w) AM_SHARE("videoram")    // FIX PAGE
	AM_RANGE(0xc800, 0xc803) AM_MIRROR(0x07fc) AM_DEVREAD("i8255_0", i8255_device, read) AM_WRITE(buckrog_i8255_0_w)    // 8255
	AM_RANGE(0xd000, 0xd003) AM_MIRROR(0x07fc) AM_DEVREADWRITE("i8255_1", i8255_device, read, write)            // 8255
	AM_RANGE(0xd800, 0xd800) AM_MIRROR(0x07fe) AM_DEVREADWRITE("i8279", i8279_device, data_r, data_w )
	AM_RANGE(0xd801, 0xd801) AM_MIRROR(0x07fe) AM_DEVREADWRITE("i8279", i8279_device, status_r, cmd_w)
	AM_RANGE(0xe000, 0xe3ff) AM_RAM AM_SHARE("spritepos")                           // CONT RAM
	AM_RANGE(0xe400, 0xe7ff) AM_RAM AM_SHARE("spriteram")                           // CONT RAM
	AM_RANGE(0xe800, 0xe800) AM_MIRROR(0x07fc) AM_READ_PORT("IN0")                  // INPUT
	AM_RANGE(0xe801, 0xe801) AM_MIRROR(0x07fc) AM_READ_PORT("IN1")
	AM_RANGE(0xe802, 0xe802) AM_MIRROR(0x07fc) AM_READ(buckrog_port_2_r)
	AM_RANGE(0xe803, 0xe803) AM_MIRROR(0x07fc) AM_READ(buckrog_port_3_r)
	AM_RANGE(0xf000, 0xf000)
	AM_RANGE(0xf800, 0xffff) AM_RAM                                                 // SCRATCH
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, turbo_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_SHARE("decrypted_opcodes")
ADDRESS_MAP_END

static ADDRESS_MAP_START( buckrog_cpu2_map, AS_PROGRAM, 8, turbo_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x0000, 0xdfff) AM_WRITE(buckrog_bitmap_w)
	AM_RANGE(0xe000, 0xe7ff) AM_MIRROR(0x1800) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( buckrog_cpu2_portmap, AS_IO, 8, turbo_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0xff) AM_READ(buckrog_cpu2_command_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( turbo )
	PORT_START("INPUT") /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )                /* ACCEL B */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )                /* ACCEL A */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_TOGGLE    /* SHIFT */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW1")  /* DSW 1 */
	PORT_DIPNAME( 0x03, 0x03, "Car On Extended Play" )  PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "Fixed (55 sec)" )
	PORT_DIPSETTING(    0x00, "Adjustable" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ))
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ))
	PORT_DIPNAME( 0x10, 0x10, "Game Mode" )             PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "No Collisions (cheat)" )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x20, 0x20, "Initial Entry" )         PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  /* DSW 2 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "60 seconds" )
	PORT_DIPSETTING(    0x01, "70 seconds" )
	PORT_DIPSETTING(    0x02, "80 seconds" )
	PORT_DIPSETTING(    0x03, "90 seconds" )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ))        PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x14, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ))
/*  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))*/
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ))
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ))        PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ))
/*  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))*/
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_6C ))

	PORT_START("DSW3")  /* Collision and DSW 3 */
	PORT_BIT( 0x0f,     0x00, IPT_SPECIAL ) /* Merged with collision bits */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Tachometer" )            PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x40, "Analog (Meter)")
	PORT_DIPSETTING(    0x00, "Digital (LED)")
	PORT_DIPNAME( 0x80, 0x80, "Sound System" )          PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, "Cockpit")

	PORT_START("DIAL")  /* IN0 */
	PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(10) PORT_KEYDELTA(30)

	/* this is actually a variable resistor */
	PORT_START("VR1")
	PORT_ADJUSTER(31, "Sprite scale offset")

	/* this is actually a variable resistor */
	PORT_START("VR2")
	PORT_ADJUSTER(91, "Sprite scale gain")
INPUT_PORTS_END


static INPUT_PORTS_START( subroc3d )
	PORT_START("IN0")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE_NO_TOGGLE( 0x10, 0x10 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW2")  /* DSW 2 */
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ))        PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x38, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0xc0, "5" )

	PORT_START("DSW3")  /* DSW 3 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x01, "40000" )
	PORT_DIPSETTING(    0x02, "60000" )
	PORT_DIPSETTING(    0x03, "80000" )
	PORT_DIPNAME( 0x04, 0x04, "Initial Entry" )         PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Motion" )                PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x00, "Stop" )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x40, 0x00, "Screen" )                PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Mono ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Stereo ) )
	PORT_DIPNAME( 0x80, 0x80, "Game" )                  PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x00, "Endless" )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )

	PORT_START("DSW1")  /* DSW 1 */                 /* Unused */
INPUT_PORTS_END


static INPUT_PORTS_START( buckrog )
	PORT_START("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) // Accel Hi
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) // Accel Lo
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )

	PORT_START("IN1")   /* Inputs */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE_NO_TOGGLE( 0x10, 0x10 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW1")  /* DSW 1 */
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ))        PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x38, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("DSW2")  /* DSW 2 */
	PORT_DIPNAME( 0x01, 0x00, "Collisions" )            PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Accel by" )              PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, "Pedal" )
	PORT_DIPSETTING(    0x02, "Button" )
	PORT_DIPNAME( 0x04, 0x00, "Best 5 Scores" )         PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Score Display" )         PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x60, "6" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, "Cockpit" )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static GFXDECODE_START( turbo )
	GFXDECODE_ENTRY( "fgtiles", 0, gfx_8x8x2_planar, 0, 64 )
GFXDECODE_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( turbo, turbo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(turbo_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", turbo_state,  irq0_line_hold)

	MCFG_DEVICE_ADD("i8255_0", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(turbo_state, turbo_ppi0a_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(turbo_state, turbo_ppi0b_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(turbo_state, turbo_ppi0c_w))

	MCFG_DEVICE_ADD("i8255_1", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(turbo_state, turbo_ppi1a_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(turbo_state, turbo_ppi1b_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(turbo_state, turbo_ppi1c_w))

	MCFG_DEVICE_ADD("i8255_2", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(turbo_state, turbo_sound_a_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(turbo_state, turbo_sound_b_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(turbo_state, turbo_sound_c_w))

	MCFG_DEVICE_ADD("i8255_3", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(turbo_state, turbo_analog_r))
	MCFG_I8255_IN_PORTB_CB(IOPORT("DSW2"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(turbo_state, turbo_ppi3c_w))

	MCFG_DEVICE_ADD("i8279", I8279, MASTER_CLOCK/4)    // unknown clock
	MCFG_I8279_OUT_SL_CB(WRITE8(turbo_state, scanlines_w))    // scan SL lines
	MCFG_I8279_OUT_DISP_CB(WRITE8(turbo_state, digit_w))      // display A&B
	MCFG_I8279_IN_RL_CB(IOPORT("DSW1"))                       // kbd RL lines

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", turbo)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(turbo_state,turbo)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(turbo_state, screen_update_turbo)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(turbo_state,turbo)

	/* sound hardware */
	MCFG_FRAGMENT_ADD(turbo_samples)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( subroc3d, turbo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(subroc3d_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", turbo_state,  irq0_line_hold)

	MCFG_DEVICE_ADD("i8255_0", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(turbo_state, subroc3d_ppi0a_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(turbo_state, subroc3d_ppi0b_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(turbo_state, subroc3d_ppi0c_w))

	MCFG_DEVICE_ADD("i8255_1", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(turbo_state, subroc3d_sound_a_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(turbo_state, subroc3d_sound_b_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(turbo_state, subroc3d_sound_c_w))

	MCFG_DEVICE_ADD("i8279", I8279, MASTER_CLOCK/4)    // unknown clock
	MCFG_I8279_OUT_SL_CB(WRITE8(turbo_state, scanlines_w))    // scan SL lines
	MCFG_I8279_OUT_DISP_CB(WRITE8(turbo_state, digit_w))      // display A&B
	MCFG_I8279_IN_RL_CB(IOPORT("DSW1"))                       // kbd RL lines

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", turbo)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(turbo_state,subroc3d)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(turbo_state, screen_update_subroc3d)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ORIENTATION_FLIP_X)

	MCFG_VIDEO_START_OVERRIDE(turbo_state,turbo)

	/* sound hardware */
	MCFG_FRAGMENT_ADD(subroc3d_samples)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( buckrog, turbo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(buckrog_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", turbo_state,  irq0_line_hold)

	MCFG_CPU_ADD("subcpu", Z80, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(buckrog_cpu2_map)
	MCFG_CPU_IO_MAP(buckrog_cpu2_portmap)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))
	MCFG_MACHINE_RESET_OVERRIDE(turbo_state,buckrog)

	MCFG_DEVICE_ADD("i8255_0", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(turbo_state, buckrog_ppi0a_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(turbo_state, buckrog_ppi0b_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(turbo_state, buckrog_ppi0c_w))

	MCFG_DEVICE_ADD("i8255_1", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(turbo_state, buckrog_sound_a_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(turbo_state, buckrog_sound_b_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(turbo_state, buckrog_ppi1c_w))

	MCFG_DEVICE_ADD("i8279", I8279, MASTER_CLOCK/4)    // unknown clock
	MCFG_I8279_OUT_SL_CB(WRITE8(turbo_state, scanlines_w))    // scan SL lines
	MCFG_I8279_OUT_DISP_CB(WRITE8(turbo_state, digit_w))      // display A&B
	MCFG_I8279_IN_RL_CB(IOPORT("DSW1"))                       // kbd RL lines

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", turbo)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_INIT_OWNER(turbo_state,buckrog)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(turbo_state, screen_update_buckrog)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(turbo_state,buckrog)

	/* sound hardware */
	MCFG_FRAGMENT_ADD(buckrog_samples)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( buckrogu, buckrog )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_DEVICE_REMOVE_ADDRESS_MAP(AS_DECRYPTED_OPCODES)
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( turbo )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "epr-1513.cpu-ic76",  0x0000, 0x2000, CRC(0326adfc) SHA1(d9f06f0bc78667fa58c4b8ab3a3897d0dd0bdfbf) )
	ROM_LOAD( "epr-1514.cpu-ic89",  0x2000, 0x2000, CRC(25af63b0) SHA1(9af4b3da83a4cef79b7dd0e9061132c499872c1c) )
	ROM_LOAD( "epr-1515.cpu-ic103", 0x4000, 0x2000, CRC(059c1c36) SHA1(ba870e6f45ff15aa148b2c2f213c879144aaacf0) )

	ROM_REGION( 0x20000, "sprites", 0 )    /* sprite data */
	ROM_LOAD( "epr-1246.prom-ic84", 0x00000, 0x2000, CRC(555bfe9a) SHA1(1e56385475eeff044dcd9b44a154991d3efe995e) ) /* level 0 */
	ROM_RELOAD(                     0x02000, 0x2000 )
	ROM_LOAD( "epr-1247.prom-ic86", 0x04000, 0x2000, CRC(c8c5e4d5) SHA1(da70297340ddea0cd7fe04f2d94ea65f8202d0e5) ) /* level 1 */
	ROM_RELOAD(                     0x06000, 0x2000 )
	ROM_LOAD( "epr-1248.prom-ic88", 0x08000, 0x2000, CRC(82fe5b94) SHA1(b96688ca0cfd90fdc4ee7c2e6c0b66726cc5713c) ) /* level 2 */
	ROM_RELOAD(                     0x0a000, 0x2000 )
	ROM_LOAD( "epr-1249.prom-ic90", 0x0c000, 0x2000, CRC(e258e009) SHA1(598d382db0f789ea2fde749b7467abed545de25a) ) /* level 3 */
	ROM_LOAD( "epr-1250.prom-ic108",0x0e000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1251.prom-ic92", 0x10000, 0x2000, CRC(292573de) SHA1(3ddc980d11478a6a6e4082c2f76c1ab82ffe2f36) ) /* level 4 */
	ROM_LOAD( "epr-1252.prom-ic110",0x12000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1253.prom-ic94", 0x14000, 0x2000, CRC(92783626) SHA1(13979eb964112436182d2a92f21803bcc28f4a4a) ) /* level 5 */
	ROM_LOAD( "epr-1254.prom-ic112",0x16000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1255.prom-ic32", 0x18000, 0x2000, CRC(485dcef9) SHA1(0f760ebb42cc2580a29758c72428a41d74477ce6) ) /* level 6 */
	ROM_LOAD( "epr-1256.prom-ic47", 0x1a000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1257.prom-ic34", 0x1c000, 0x2000, CRC(4ca984ce) SHA1(99f294fb203f23929b44baa2dd1825c67dde08a1) ) /* level 7 */
	ROM_LOAD( "epr-1258.prom-ic49", 0x1e000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )

	ROM_REGION( 0x1000, "fgtiles", 0 ) /* foreground data */
	ROM_LOAD( "epr-1244.cpu-ic111", 0x0000, 0x0800, CRC(17f67424) SHA1(6126562510f1509f3487faaa3b9d7470ab600a2c) )
	ROM_LOAD( "epr-1245.cpu-ic122", 0x0800, 0x0800, CRC(2ba0b46b) SHA1(5d4d4f19ad7a911c7b37db190a420faf665546b4) )

	ROM_REGION( 0x4800, "road", 0 ) /* road data */
	ROM_LOAD( "epr-1125.cpu-ic1",   0x0000, 0x0800, CRC(65b5d44b) SHA1(bbdd5db013c9d876e9666f17c48569c7531bfc08) )
	ROM_LOAD( "epr-1126.cpu-ic2",   0x0800, 0x0800, CRC(685ace1b) SHA1(99c8d36ac910169b27676d18c894433c2ba44853) )
	ROM_LOAD( "epr-1127.cpu-ic13",  0x1000, 0x0800, CRC(9233c9ca) SHA1(cbf9a0f564d8ace1ccd701c1769dbc001d465851) )
	ROM_LOAD( "epr-1238.cpu-ic14",  0x1800, 0x0800, CRC(d94fd83f) SHA1(1e3a68259d2ede623d5a7306fdf693a4eab301f0) )
	ROM_LOAD( "epr-1239.cpu-ic27",  0x2000, 0x0800, CRC(4c41124f) SHA1(d73a9441552c77fb3078553195794311a950d589) )
	ROM_LOAD( "epr-1240.cpu-ic28",  0x2800, 0x0800, CRC(371d6282) SHA1(f5902b357d976822d46aa6404b7bd30855d435a9) )
	ROM_LOAD( "epr-1241.cpu-ic41",  0x3000, 0x0800, CRC(1109358a) SHA1(27a5351a4e87309671e72115299420315a93dba6) )
	ROM_LOAD( "epr-1242.cpu-ic42",  0x3800, 0x0800, CRC(04866769) SHA1(1f9c0d53766fdaf8de57d3df05f291c2ca3dc5fb) )
	ROM_LOAD( "epr-1243.cpu-ic74",  0x4000, 0x0800, CRC(29854c48) SHA1(cab89bc30f83d9746931ddf6f95a6d0c8a517e5d) )

	ROM_REGION( 0x1020, "proms", 0 )    /* various PROMs */
	ROM_LOAD( "pr-1114.prom-ic13",  0x0000, 0x0020, CRC(78aded46) SHA1(c78afe804f8b8e837b0c502de5b8715a41fb92b9) )  /* road red/green color table */
	ROM_LOAD( "pr-1115.prom-ic18",  0x0020, 0x0020, CRC(5394092c) SHA1(129ff61104979ff6a3c3af8bf81c04ae9b133c9e) )  /* road collision/enable */
	ROM_LOAD( "pr-1116.prom-ic20",  0x0040, 0x0020, CRC(3956767d) SHA1(073aaf57175526660fcf7af2e16e7f1d1aaba9a9) )  /* collision detection */
	ROM_LOAD( "pr-1117.prom-ic21",  0x0060, 0x0020, CRC(f06d9907) SHA1(f11db7800f41b03e79f5eef8d7ef3ae0a6277518) )  /* road green/blue color table */
	ROM_LOAD( "pr-1118.cpu-ic99",   0x0100, 0x0100, CRC(07324cfd) SHA1(844abc2042d6810fa34d84ff1ed57744886c6ea6) )  /* background color table */
	ROM_LOAD( "pr-1119.cpu-ic50",   0x0200, 0x0200, CRC(57ebd4bc) SHA1(932649da3537666f95833a8a8aff506217bd9aa1) )  /* sprite Y scaling */
	ROM_LOAD( "pr-1120.cpu-ic62",   0x0400, 0x0200, CRC(8dd4c8a8) SHA1(e8d9cf08f115d57c44746fa0ff28f47b064b4193) )  /* video timing */
	ROM_LOAD( "pr-1121.prom-ic29",  0x0600, 0x0200, CRC(7692f497) SHA1(42468c0705df9928e15ff8deb7e793a6c0c04353) )  /* palette */
	ROM_LOAD( "pr-1122.prom-ic11",  0x0800, 0x0400, CRC(1a86ce70) SHA1(cab708b9a089b2e28f2298c1e4fae6e200923527) )  /* sprite priorities */
	ROM_LOAD( "pr-1123.prom-ic12",  0x0c00, 0x0400, CRC(02d2cb52) SHA1(c34d6b60355747ce20fcb8d322df0e188d187f10) )  /* sprite/road/background priorities */
	ROM_LOAD( "pr-1279.sound-ic40", 0x1000, 0x0020, CRC(b369a6ae) SHA1(dda7c6cf58ce5173f29a3084c85393c0c4587086) )  /* sound board PROM */
ROM_END


ROM_START( turboa )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "epr-1262.cpu-ic76",  0x0000, 0x2000, CRC(1951b83a) SHA1(31933676140db66281b7ca016a1b42cb985f44dd) )
	ROM_LOAD( "epr-1263.cpu-ic89",  0x2000, 0x2000, CRC(45e01608) SHA1(0a9812714c41904bef7a8777b4aae63b5a1dd633) )
	ROM_LOAD( "epr-1264.cpu-ic103", 0x4000, 0x2000, CRC(1802f6c7) SHA1(5c575821d849d955059868b3dd3167b4bef9a8c4) )

	ROM_REGION( 0x20000, "sprites", 0 )    /* sprite data */
	ROM_LOAD( "epr-1246.prom-ic84", 0x00000, 0x2000, CRC(555bfe9a) SHA1(1e56385475eeff044dcd9b44a154991d3efe995e) ) /* level 0 */
	ROM_RELOAD(                     0x02000, 0x2000 )
	ROM_LOAD( "epr-1247.prom-ic86", 0x04000, 0x2000, CRC(c8c5e4d5) SHA1(da70297340ddea0cd7fe04f2d94ea65f8202d0e5) ) /* level 1 */
	ROM_RELOAD(                     0x06000, 0x2000 )
	ROM_LOAD( "epr-1248.prom-ic88", 0x08000, 0x2000, CRC(82fe5b94) SHA1(b96688ca0cfd90fdc4ee7c2e6c0b66726cc5713c) ) /* level 2 */
	ROM_RELOAD(                     0x0a000, 0x2000 )
	ROM_LOAD( "epr-1249.prom-ic90", 0x0c000, 0x2000, CRC(e258e009) SHA1(598d382db0f789ea2fde749b7467abed545de25a) ) /* level 3 */
	ROM_LOAD( "epr-1250.prom-ic108",0x0e000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1251.prom-ic92", 0x10000, 0x2000, CRC(292573de) SHA1(3ddc980d11478a6a6e4082c2f76c1ab82ffe2f36) ) /* level 4 */
	ROM_LOAD( "epr-1252.prom-ic110",0x12000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1253.prom-ic94", 0x14000, 0x2000, CRC(92783626) SHA1(13979eb964112436182d2a92f21803bcc28f4a4a) ) /* level 5 */
	ROM_LOAD( "epr-1254.prom-ic112",0x16000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1255.prom-ic32", 0x18000, 0x2000, CRC(485dcef9) SHA1(0f760ebb42cc2580a29758c72428a41d74477ce6) ) /* level 6 */
	ROM_LOAD( "epr-1256.prom-ic47", 0x1a000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1257.prom-ic34", 0x1c000, 0x2000, CRC(4ca984ce) SHA1(99f294fb203f23929b44baa2dd1825c67dde08a1) ) /* level 7 */
	ROM_LOAD( "epr-1258.prom-ic49", 0x1e000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )

	ROM_REGION( 0x1000, "fgtiles", 0 ) /* foreground data */
	ROM_LOAD( "epr-1244.cpu-ic111", 0x0000, 0x0800, CRC(17f67424) SHA1(6126562510f1509f3487faaa3b9d7470ab600a2c) )
	ROM_LOAD( "epr-1245.cpu-ic122", 0x0800, 0x0800, CRC(2ba0b46b) SHA1(5d4d4f19ad7a911c7b37db190a420faf665546b4) )

	ROM_REGION( 0x4800, "road", 0 ) /* road data */
	ROM_LOAD( "epr-1125.cpu-ic1",   0x0000, 0x0800, CRC(65b5d44b) SHA1(bbdd5db013c9d876e9666f17c48569c7531bfc08) )
	ROM_LOAD( "epr-1126.cpu-ic2",   0x0800, 0x0800, CRC(685ace1b) SHA1(99c8d36ac910169b27676d18c894433c2ba44853) )
	ROM_LOAD( "epr-1127.cpu-ic13",  0x1000, 0x0800, CRC(9233c9ca) SHA1(cbf9a0f564d8ace1ccd701c1769dbc001d465851) )
	ROM_LOAD( "epr-1238.cpu-ic14",  0x1800, 0x0800, CRC(d94fd83f) SHA1(1e3a68259d2ede623d5a7306fdf693a4eab301f0) )
	ROM_LOAD( "epr-1239.cpu-ic27",  0x2000, 0x0800, CRC(4c41124f) SHA1(d73a9441552c77fb3078553195794311a950d589) )
	ROM_LOAD( "epr-1240.cpu-ic28",  0x2800, 0x0800, CRC(371d6282) SHA1(f5902b357d976822d46aa6404b7bd30855d435a9) )
	ROM_LOAD( "epr-1241.cpu-ic41",  0x3000, 0x0800, CRC(1109358a) SHA1(27a5351a4e87309671e72115299420315a93dba6) )
	ROM_LOAD( "epr-1242.cpu-ic42",  0x3800, 0x0800, CRC(04866769) SHA1(1f9c0d53766fdaf8de57d3df05f291c2ca3dc5fb) )
	ROM_LOAD( "epr-1243.cpu-ic74",  0x4000, 0x0800, CRC(29854c48) SHA1(cab89bc30f83d9746931ddf6f95a6d0c8a517e5d) )

	ROM_REGION( 0x1020, "proms", 0 )    /* various PROMs */
	ROM_LOAD( "pr-1114.prom-ic13",  0x0000, 0x0020, CRC(78aded46) SHA1(c78afe804f8b8e837b0c502de5b8715a41fb92b9) )  /* road red/green color table */
	ROM_LOAD( "pr-1115.prom-ic18",  0x0020, 0x0020, CRC(5394092c) SHA1(129ff61104979ff6a3c3af8bf81c04ae9b133c9e) )  /* road collision/enable */
	ROM_LOAD( "pr-1116.prom-ic20",  0x0040, 0x0020, CRC(3956767d) SHA1(073aaf57175526660fcf7af2e16e7f1d1aaba9a9) )  /* collision detection */
	ROM_LOAD( "pr-1117.prom-ic21",  0x0060, 0x0020, CRC(f06d9907) SHA1(f11db7800f41b03e79f5eef8d7ef3ae0a6277518) )  /* road green/blue color table */
	ROM_LOAD( "pr-1118.cpu-ic99",   0x0100, 0x0100, CRC(07324cfd) SHA1(844abc2042d6810fa34d84ff1ed57744886c6ea6) )  /* background color table */
	ROM_LOAD( "pr-1119.cpu-ic50",   0x0200, 0x0200, CRC(57ebd4bc) SHA1(932649da3537666f95833a8a8aff506217bd9aa1) )  /* sprite Y scaling */
	ROM_LOAD( "pr-1120.cpu-ic62",   0x0400, 0x0200, CRC(8dd4c8a8) SHA1(e8d9cf08f115d57c44746fa0ff28f47b064b4193) )  /* video timing */
	ROM_LOAD( "pr-1121.prom-ic29",  0x0600, 0x0200, CRC(7692f497) SHA1(42468c0705df9928e15ff8deb7e793a6c0c04353) )  /* palette */
	ROM_LOAD( "pr-1122.prom-ic11",  0x0800, 0x0400, CRC(1a86ce70) SHA1(cab708b9a089b2e28f2298c1e4fae6e200923527) )  /* sprite priorities */
	ROM_LOAD( "pr-1123.prom-ic12",  0x0c00, 0x0400, CRC(02d2cb52) SHA1(c34d6b60355747ce20fcb8d322df0e188d187f10) )  /* sprite/road/background priorities */
	ROM_LOAD( "pr-1279.sound-ic40", 0x1000, 0x0020, CRC(b369a6ae) SHA1(dda7c6cf58ce5173f29a3084c85393c0c4587086) )  /* sound board PROM */
ROM_END


ROM_START( turbob )
	ROM_REGION( 0x6000, "maincpu", 0 )
	// B revision label on 1st rom, A revision for 2nd / 3rd, 2nd rom was faulty, 3rd rom matched existing set, existing 2nd rom passes rom check.
	ROM_LOAD( "epr-1363_t5b.ic76",  0x0000, 0x2000, CRC(f7f28149) SHA1(08aec3edd6d756b14b2f10fe5abd22ac83c79fcc) ) /* CPU module stamped as 834-0128 */
	ROM_LOAD( "epr-1364_t5a.ic89",  0x2000, 0x2000, CRC(6a341693) SHA1(428927c4a14bf82225875012c255d25dcffaf2ab) )
	ROM_LOAD( "epr-1365_t5a.ic103", 0x4000, 0x2000, CRC(3b6b0dc8) SHA1(3ebfa3f9fabd444ee105591acb6984b6b3523725) )

	ROM_REGION( 0x20000, "sprites", 0 ) /* sprite data */
	ROM_LOAD( "epr-1246.prom-ic84", 0x00000, 0x2000, CRC(555bfe9a) SHA1(1e56385475eeff044dcd9b44a154991d3efe995e) ) /* level 0 */
	ROM_RELOAD(                     0x02000, 0x2000 )
	ROM_LOAD( "epr-1247.prom-ic86", 0x04000, 0x2000, CRC(c8c5e4d5) SHA1(da70297340ddea0cd7fe04f2d94ea65f8202d0e5) ) /* level 1 */
	ROM_RELOAD(                     0x06000, 0x2000 )
	ROM_LOAD( "epr-1248.prom-ic88", 0x08000, 0x2000, CRC(82fe5b94) SHA1(b96688ca0cfd90fdc4ee7c2e6c0b66726cc5713c) ) /* level 2 */
	ROM_RELOAD(                     0x0a000, 0x2000 )
	ROM_LOAD( "epr-1249.prom-ic90", 0x0c000, 0x2000, CRC(e258e009) SHA1(598d382db0f789ea2fde749b7467abed545de25a) ) /* level 3 */
	ROM_LOAD( "epr-1250.prom-ic108",0x0e000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1251.prom-ic92", 0x10000, 0x2000, CRC(292573de) SHA1(3ddc980d11478a6a6e4082c2f76c1ab82ffe2f36) ) /* level 4 */
	ROM_LOAD( "epr-1252.prom-ic110",0x12000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1253.prom-ic94", 0x14000, 0x2000, CRC(92783626) SHA1(13979eb964112436182d2a92f21803bcc28f4a4a) ) /* level 5 */
	ROM_LOAD( "epr-1254.prom-ic112",0x16000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1255.prom-ic32", 0x18000, 0x2000, CRC(485dcef9) SHA1(0f760ebb42cc2580a29758c72428a41d74477ce6) ) /* level 6 */
	ROM_LOAD( "epr-1256.prom-ic47", 0x1a000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1257.prom-ic34", 0x1c000, 0x2000, CRC(4ca984ce) SHA1(99f294fb203f23929b44baa2dd1825c67dde08a1) ) /* level 7 */
	ROM_LOAD( "epr-1258.prom-ic49", 0x1e000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )

	ROM_REGION( 0x1000, "fgtiles", 0 ) /* foreground data */
	ROM_LOAD( "epr-1244.cpu-ic111", 0x0000, 0x0800, CRC(17f67424) SHA1(6126562510f1509f3487faaa3b9d7470ab600a2c) )
	ROM_LOAD( "epr-1245.cpu-ic122", 0x0800, 0x0800, CRC(2ba0b46b) SHA1(5d4d4f19ad7a911c7b37db190a420faf665546b4) )

	ROM_REGION( 0x4800, "road", 0 ) /* road data */
	ROM_LOAD( "epr-1125.cpu-ic1",   0x0000, 0x0800, CRC(65b5d44b) SHA1(bbdd5db013c9d876e9666f17c48569c7531bfc08) )
	ROM_LOAD( "epr-1126.cpu-ic2",   0x0800, 0x0800, CRC(685ace1b) SHA1(99c8d36ac910169b27676d18c894433c2ba44853) )
	ROM_LOAD( "epr-1127.cpu-ic13",  0x1000, 0x0800, CRC(9233c9ca) SHA1(cbf9a0f564d8ace1ccd701c1769dbc001d465851) )
	ROM_LOAD( "epr-1238.cpu-ic14",  0x1800, 0x0800, CRC(d94fd83f) SHA1(1e3a68259d2ede623d5a7306fdf693a4eab301f0) )
	ROM_LOAD( "epr-1239.cpu-ic27",  0x2000, 0x0800, CRC(4c41124f) SHA1(d73a9441552c77fb3078553195794311a950d589) )
	ROM_LOAD( "epr-1240.cpu-ic28",  0x2800, 0x0800, CRC(371d6282) SHA1(f5902b357d976822d46aa6404b7bd30855d435a9) )
	ROM_LOAD( "epr-1241.cpu-ic41",  0x3000, 0x0800, CRC(1109358a) SHA1(27a5351a4e87309671e72115299420315a93dba6) )
	ROM_LOAD( "epr-1242.cpu-ic42",  0x3800, 0x0800, CRC(04866769) SHA1(1f9c0d53766fdaf8de57d3df05f291c2ca3dc5fb) )
	ROM_LOAD( "epr-1243.cpu-ic74",  0x4000, 0x0800, CRC(29854c48) SHA1(cab89bc30f83d9746931ddf6f95a6d0c8a517e5d) )

	ROM_REGION( 0x1020, "proms", 0 )    /* various PROMs */
	ROM_LOAD( "pr-1114.prom-ic13",  0x0000, 0x0020, CRC(78aded46) SHA1(c78afe804f8b8e837b0c502de5b8715a41fb92b9) )  /* road red/green color table */
	ROM_LOAD( "pr-1115.prom-ic18",  0x0020, 0x0020, CRC(5394092c) SHA1(129ff61104979ff6a3c3af8bf81c04ae9b133c9e) )  /* road collision/enable */
	ROM_LOAD( "pr-1116.prom-ic20",  0x0040, 0x0020, CRC(3956767d) SHA1(073aaf57175526660fcf7af2e16e7f1d1aaba9a9) )  /* collision detection */
	ROM_LOAD( "pr-1117.prom-ic21",  0x0060, 0x0020, CRC(f06d9907) SHA1(f11db7800f41b03e79f5eef8d7ef3ae0a6277518) )  /* road green/blue color table */
	ROM_LOAD( "pr-1118.cpu-ic99",   0x0100, 0x0100, CRC(07324cfd) SHA1(844abc2042d6810fa34d84ff1ed57744886c6ea6) )  /* background color table */
	ROM_LOAD( "pr-1119.cpu-ic50",   0x0200, 0x0200, CRC(57ebd4bc) SHA1(932649da3537666f95833a8a8aff506217bd9aa1) )  /* sprite Y scaling */
	ROM_LOAD( "pr-1120.cpu-ic62",   0x0400, 0x0200, CRC(8dd4c8a8) SHA1(e8d9cf08f115d57c44746fa0ff28f47b064b4193) )  /* video timing */
	ROM_LOAD( "pr-1121.prom-ic29",  0x0600, 0x0200, CRC(7692f497) SHA1(42468c0705df9928e15ff8deb7e793a6c0c04353) )  /* palette */
	ROM_LOAD( "pr-1122.prom-ic11",  0x0800, 0x0400, CRC(1a86ce70) SHA1(cab708b9a089b2e28f2298c1e4fae6e200923527) )  /* sprite priorities */
	ROM_LOAD( "pr-1123.prom-ic12",  0x0c00, 0x0400, CRC(02d2cb52) SHA1(c34d6b60355747ce20fcb8d322df0e188d187f10) )  /* sprite/road/background priorities */
	ROM_LOAD( "pr-1279.sound-ic40", 0x1000, 0x0020, CRC(b369a6ae) SHA1(dda7c6cf58ce5173f29a3084c85393c0c4587086) )  /* sound board PROM */
ROM_END

ROM_START( turboc )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "epr-1363_t5a.ic76",  0x0000, 0x2000, CRC(5c110fb6) SHA1(fdcdf488bd112db12aa22c4b7e9f34004185d4ce) )
	ROM_LOAD( "epr-1364_t5a.ic89",  0x2000, 0x2000, CRC(6a341693) SHA1(428927c4a14bf82225875012c255d25dcffaf2ab) )
	ROM_LOAD( "epr-1365_t5a.ic103", 0x4000, 0x2000, CRC(3b6b0dc8) SHA1(3ebfa3f9fabd444ee105591acb6984b6b3523725) )

	ROM_REGION( 0x20000, "sprites", 0 ) /* sprite data */
	ROM_LOAD( "epr-1246.prom-ic84", 0x00000, 0x2000, CRC(555bfe9a) SHA1(1e56385475eeff044dcd9b44a154991d3efe995e) ) /* level 0 */
	ROM_RELOAD(                     0x02000, 0x2000 )
	ROM_LOAD( "epr-1247.prom-ic86", 0x04000, 0x2000, CRC(c8c5e4d5) SHA1(da70297340ddea0cd7fe04f2d94ea65f8202d0e5) ) /* level 1 */
	ROM_RELOAD(                     0x06000, 0x2000 )
	ROM_LOAD( "epr-1248.prom-ic88", 0x08000, 0x2000, CRC(82fe5b94) SHA1(b96688ca0cfd90fdc4ee7c2e6c0b66726cc5713c) ) /* level 2 */
	ROM_RELOAD(                     0x0a000, 0x2000 )
	ROM_LOAD( "epr-1249.prom-ic90", 0x0c000, 0x2000, CRC(e258e009) SHA1(598d382db0f789ea2fde749b7467abed545de25a) ) /* level 3 */
	ROM_LOAD( "epr-1250.prom-ic108",0x0e000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1251.prom-ic92", 0x10000, 0x2000, CRC(292573de) SHA1(3ddc980d11478a6a6e4082c2f76c1ab82ffe2f36) ) /* level 4 */
	ROM_LOAD( "epr-1252.prom-ic110",0x12000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1253.prom-ic94", 0x14000, 0x2000, CRC(92783626) SHA1(13979eb964112436182d2a92f21803bcc28f4a4a) ) /* level 5 */
	ROM_LOAD( "epr-1254.prom-ic112",0x16000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1255.prom-ic32", 0x18000, 0x2000, CRC(485dcef9) SHA1(0f760ebb42cc2580a29758c72428a41d74477ce6) ) /* level 6 */
	ROM_LOAD( "epr-1256.prom-ic47", 0x1a000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1257.prom-ic34", 0x1c000, 0x2000, CRC(4ca984ce) SHA1(99f294fb203f23929b44baa2dd1825c67dde08a1) ) /* level 7 */
	ROM_LOAD( "epr-1258.prom-ic49", 0x1e000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )

	ROM_REGION( 0x1000, "fgtiles", 0 ) /* foreground data */
	ROM_LOAD( "epr-1244.cpu-ic111", 0x0000, 0x0800, CRC(17f67424) SHA1(6126562510f1509f3487faaa3b9d7470ab600a2c) )
	ROM_LOAD( "epr-1245.cpu-ic122", 0x0800, 0x0800, CRC(2ba0b46b) SHA1(5d4d4f19ad7a911c7b37db190a420faf665546b4) )

	ROM_REGION( 0x4800, "road", 0 ) /* road data */
	ROM_LOAD( "epr-1125.cpu-ic1",   0x0000, 0x0800, CRC(65b5d44b) SHA1(bbdd5db013c9d876e9666f17c48569c7531bfc08) )
	ROM_LOAD( "epr-1126.cpu-ic2",   0x0800, 0x0800, CRC(685ace1b) SHA1(99c8d36ac910169b27676d18c894433c2ba44853) )
	ROM_LOAD( "epr-1127.cpu-ic13",  0x1000, 0x0800, CRC(9233c9ca) SHA1(cbf9a0f564d8ace1ccd701c1769dbc001d465851) )
	ROM_LOAD( "epr-1238.cpu-ic14",  0x1800, 0x0800, CRC(d94fd83f) SHA1(1e3a68259d2ede623d5a7306fdf693a4eab301f0) )
	ROM_LOAD( "epr-1239.cpu-ic27",  0x2000, 0x0800, CRC(4c41124f) SHA1(d73a9441552c77fb3078553195794311a950d589) )
	ROM_LOAD( "epr-1240.cpu-ic28",  0x2800, 0x0800, CRC(371d6282) SHA1(f5902b357d976822d46aa6404b7bd30855d435a9) )
	ROM_LOAD( "epr-1241.cpu-ic41",  0x3000, 0x0800, CRC(1109358a) SHA1(27a5351a4e87309671e72115299420315a93dba6) )
	ROM_LOAD( "epr-1242.cpu-ic42",  0x3800, 0x0800, CRC(04866769) SHA1(1f9c0d53766fdaf8de57d3df05f291c2ca3dc5fb) )
	ROM_LOAD( "epr-1243.cpu-ic74",  0x4000, 0x0800, CRC(29854c48) SHA1(cab89bc30f83d9746931ddf6f95a6d0c8a517e5d) )

	ROM_REGION( 0x1020, "proms", 0 )    /* various PROMs */
	ROM_LOAD( "pr-1114.prom-ic13",  0x0000, 0x0020, CRC(78aded46) SHA1(c78afe804f8b8e837b0c502de5b8715a41fb92b9) )  /* road red/green color table */
	ROM_LOAD( "pr-1115.prom-ic18",  0x0020, 0x0020, CRC(5394092c) SHA1(129ff61104979ff6a3c3af8bf81c04ae9b133c9e) )  /* road collision/enable */
	ROM_LOAD( "pr-1116.prom-ic20",  0x0040, 0x0020, CRC(3956767d) SHA1(073aaf57175526660fcf7af2e16e7f1d1aaba9a9) )  /* collision detection */
	ROM_LOAD( "pr-1117.prom-ic21",  0x0060, 0x0020, CRC(f06d9907) SHA1(f11db7800f41b03e79f5eef8d7ef3ae0a6277518) )  /* road green/blue color table */
	ROM_LOAD( "pr-1118.cpu-ic99",   0x0100, 0x0100, CRC(07324cfd) SHA1(844abc2042d6810fa34d84ff1ed57744886c6ea6) )  /* background color table */
	ROM_LOAD( "pr-1119.cpu-ic50",   0x0200, 0x0200, CRC(57ebd4bc) SHA1(932649da3537666f95833a8a8aff506217bd9aa1) )  /* sprite Y scaling */
	ROM_LOAD( "pr-1120.cpu-ic62",   0x0400, 0x0200, CRC(8dd4c8a8) SHA1(e8d9cf08f115d57c44746fa0ff28f47b064b4193) )  /* video timing */
	ROM_LOAD( "pr-1121.prom-ic29",  0x0600, 0x0200, CRC(7692f497) SHA1(42468c0705df9928e15ff8deb7e793a6c0c04353) )  /* palette */
	ROM_LOAD( "pr-1122.prom-ic11",  0x0800, 0x0400, CRC(1a86ce70) SHA1(cab708b9a089b2e28f2298c1e4fae6e200923527) )  /* sprite priorities */
	ROM_LOAD( "pr-1123.prom-ic12",  0x0c00, 0x0400, CRC(02d2cb52) SHA1(c34d6b60355747ce20fcb8d322df0e188d187f10) )  /* sprite/road/background priorities */
	ROM_LOAD( "pr-1279.sound-ic40", 0x1000, 0x0020, CRC(b369a6ae) SHA1(dda7c6cf58ce5173f29a3084c85393c0c4587086) )  /* sound board PROM */
ROM_END


ROM_START( turbod )
	ROM_REGION( 0x6000, "maincpu", 0 )
	// no letter on rom labels, numbered 1363-1365, possibly the original 1363-1365 revision? - service mode still shows the older EPR-1262 through EPR-1264 tho?
	ROM_LOAD( "1363.ic76",  0x0000, 0x2000, CRC(b6329a00) SHA1(02ca3d7703607bc0390a14d838fafb01a3e3cdef) ) /* CPU module stamped as 834-0128 */
	ROM_LOAD( "1364.ic89",  0x2000, 0x2000, CRC(3192f83b) SHA1(78dabb75e38f5f8331bfc43dce852a4c4397f874) )
	ROM_LOAD( "1365.ic103", 0x4000, 0x2000, CRC(23a3303a) SHA1(bcc4ab9203060b4043d779b7a242abc583093dbb) )

	ROM_REGION( 0x20000, "sprites", 0 ) /* sprite data */
	ROM_LOAD( "epr-1246.prom-ic84", 0x00000, 0x2000, CRC(555bfe9a) SHA1(1e56385475eeff044dcd9b44a154991d3efe995e) ) /* level 0 */
	ROM_RELOAD(                     0x02000, 0x2000 )
	ROM_LOAD( "epr-1247.prom-ic86", 0x04000, 0x2000, CRC(c8c5e4d5) SHA1(da70297340ddea0cd7fe04f2d94ea65f8202d0e5) ) /* level 1 */
	ROM_RELOAD(                     0x06000, 0x2000 )
	ROM_LOAD( "epr-1248.prom-ic88", 0x08000, 0x2000, CRC(82fe5b94) SHA1(b96688ca0cfd90fdc4ee7c2e6c0b66726cc5713c) ) /* level 2 */
	ROM_RELOAD(                     0x0a000, 0x2000 )
	ROM_LOAD( "epr-1249.prom-ic90", 0x0c000, 0x2000, CRC(e258e009) SHA1(598d382db0f789ea2fde749b7467abed545de25a) ) /* level 3 */
	ROM_LOAD( "epr-1250.prom-ic108",0x0e000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1251.prom-ic92", 0x10000, 0x2000, CRC(292573de) SHA1(3ddc980d11478a6a6e4082c2f76c1ab82ffe2f36) ) /* level 4 */
	ROM_LOAD( "epr-1252.prom-ic110",0x12000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1253.prom-ic94", 0x14000, 0x2000, CRC(92783626) SHA1(13979eb964112436182d2a92f21803bcc28f4a4a) ) /* level 5 */
	ROM_LOAD( "epr-1254.prom-ic112",0x16000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1255.prom-ic32", 0x18000, 0x2000, CRC(485dcef9) SHA1(0f760ebb42cc2580a29758c72428a41d74477ce6) ) /* level 6 */
	ROM_LOAD( "epr-1256.prom-ic47", 0x1a000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "epr-1257.prom-ic34", 0x1c000, 0x2000, CRC(4ca984ce) SHA1(99f294fb203f23929b44baa2dd1825c67dde08a1) ) /* level 7 */
	ROM_LOAD( "epr-1258.prom-ic49", 0x1e000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )

	ROM_REGION( 0x1000, "fgtiles", 0 ) /* foreground data */
	ROM_LOAD( "epr-1244.cpu-ic111", 0x0000, 0x0800, CRC(17f67424) SHA1(6126562510f1509f3487faaa3b9d7470ab600a2c) )
	ROM_LOAD( "epr-1245.cpu-ic122", 0x0800, 0x0800, CRC(2ba0b46b) SHA1(5d4d4f19ad7a911c7b37db190a420faf665546b4) )

	ROM_REGION( 0x4800, "road", 0 ) /* road data */
	ROM_LOAD( "epr-1125.cpu-ic1",   0x0000, 0x0800, CRC(65b5d44b) SHA1(bbdd5db013c9d876e9666f17c48569c7531bfc08) )
	ROM_LOAD( "epr-1126.cpu-ic2",   0x0800, 0x0800, CRC(685ace1b) SHA1(99c8d36ac910169b27676d18c894433c2ba44853) )
	ROM_LOAD( "epr-1127.cpu-ic13",  0x1000, 0x0800, CRC(9233c9ca) SHA1(cbf9a0f564d8ace1ccd701c1769dbc001d465851) )
	ROM_LOAD( "epr-1238.cpu-ic14",  0x1800, 0x0800, CRC(d94fd83f) SHA1(1e3a68259d2ede623d5a7306fdf693a4eab301f0) )
	ROM_LOAD( "epr-1239.cpu-ic27",  0x2000, 0x0800, CRC(4c41124f) SHA1(d73a9441552c77fb3078553195794311a950d589) )
	ROM_LOAD( "epr-1240.cpu-ic28",  0x2800, 0x0800, CRC(371d6282) SHA1(f5902b357d976822d46aa6404b7bd30855d435a9) )
	ROM_LOAD( "epr-1241.cpu-ic41",  0x3000, 0x0800, CRC(1109358a) SHA1(27a5351a4e87309671e72115299420315a93dba6) )
	ROM_LOAD( "epr-1242.cpu-ic42",  0x3800, 0x0800, CRC(04866769) SHA1(1f9c0d53766fdaf8de57d3df05f291c2ca3dc5fb) )
	ROM_LOAD( "epr-1243.cpu-ic74",  0x4000, 0x0800, CRC(29854c48) SHA1(cab89bc30f83d9746931ddf6f95a6d0c8a517e5d) )

	ROM_REGION( 0x1020, "proms", 0 )    /* various PROMs */
	ROM_LOAD( "pr-1114.prom-ic13",  0x0000, 0x0020, CRC(78aded46) SHA1(c78afe804f8b8e837b0c502de5b8715a41fb92b9) )  /* road red/green color table */
	ROM_LOAD( "pr-1115.prom-ic18",  0x0020, 0x0020, CRC(5394092c) SHA1(129ff61104979ff6a3c3af8bf81c04ae9b133c9e) )  /* road collision/enable */
	ROM_LOAD( "pr-1116.prom-ic20",  0x0040, 0x0020, CRC(3956767d) SHA1(073aaf57175526660fcf7af2e16e7f1d1aaba9a9) )  /* collision detection */
	ROM_LOAD( "pr-1117.prom-ic21",  0x0060, 0x0020, CRC(f06d9907) SHA1(f11db7800f41b03e79f5eef8d7ef3ae0a6277518) )  /* road green/blue color table */
	ROM_LOAD( "pr-1118.cpu-ic99",   0x0100, 0x0100, CRC(07324cfd) SHA1(844abc2042d6810fa34d84ff1ed57744886c6ea6) )  /* background color table */
	ROM_LOAD( "pr-1119.cpu-ic50",   0x0200, 0x0200, CRC(57ebd4bc) SHA1(932649da3537666f95833a8a8aff506217bd9aa1) )  /* sprite Y scaling */
	ROM_LOAD( "pr-1120.cpu-ic62",   0x0400, 0x0200, CRC(8dd4c8a8) SHA1(e8d9cf08f115d57c44746fa0ff28f47b064b4193) )  /* video timing */
	ROM_LOAD( "pr-1121.prom-ic29",  0x0600, 0x0200, CRC(7692f497) SHA1(42468c0705df9928e15ff8deb7e793a6c0c04353) )  /* palette */
	ROM_LOAD( "pr-1122.prom-ic11",  0x0800, 0x0400, CRC(1a86ce70) SHA1(cab708b9a089b2e28f2298c1e4fae6e200923527) )  /* sprite priorities */
	ROM_LOAD( "pr-1123.prom-ic12",  0x0c00, 0x0400, CRC(02d2cb52) SHA1(c34d6b60355747ce20fcb8d322df0e188d187f10) )  /* sprite/road/background priorities */
	ROM_LOAD( "pr-1279.sound-ic40", 0x1000, 0x0020, CRC(b369a6ae) SHA1(dda7c6cf58ce5173f29a3084c85393c0c4587086) )  /* sound board PROM */
ROM_END


ROM_START( turbobl )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "ic76.bin",  0x0000, 0x2000, CRC(c208373b) SHA1(c1b8de41fe5cb20a262f8e8d1326deb9c8bdfe90) )
	ROM_LOAD( "ic89.bin",  0x2000, 0x2000, CRC(93ebc86a) SHA1(26b78ef19610c88ce7783f7b7749b6fb34e6287d) )
	ROM_LOAD( "ic103.bin", 0x4000, 0x2000, CRC(71876f74) SHA1(078b8b93971caa4e14f5e95fadf8c209b20d266e) )

	ROM_REGION( 0x20000, "sprites", 0 )    /* sprite data */
	ROM_LOAD( "a-ic84.bin", 0x00000, 0x2000, CRC(555bfe9a) SHA1(1e56385475eeff044dcd9b44a154991d3efe995e) ) /* level 0 */
	ROM_RELOAD(             0x02000, 0x2000 )
	ROM_LOAD( "b-ic86.bin", 0x04000, 0x2000, CRC(82fe5b94) SHA1(b96688ca0cfd90fdc4ee7c2e6c0b66726cc5713c) ) /* level 1 */ // ic86 and ic88 roms seem swapped compared to other sets, is it correct?
	ROM_RELOAD(             0x06000, 0x2000 )
	ROM_LOAD( "c-ic88.bin", 0x08000, 0x2000, CRC(95182020) SHA1(cd392a311da222727ce92801cb9d926ccdb08797) ) /* level 2 */
	ROM_RELOAD(             0x0a000, 0x2000 )
	ROM_LOAD( "e-ic90.bin", 0x0c000, 0x2000, CRC(0e857f82) SHA1(fbf0dcd11fd4fa09235c3f05d8e284b7dcc8f303) ) /* level 3 */
	ROM_LOAD( "d-ic99.bin", 0x0e000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "g-ic92.bin", 0x10000, 0x2000, CRC(292573de) SHA1(3ddc980d11478a6a6e4082c2f76c1ab82ffe2f36) ) /* level 4 */
	ROM_LOAD( "f-ic100.bin",0x12000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "k-ic94.bin", 0x14000, 0x2000, CRC(92783626) SHA1(13979eb964112436182d2a92f21803bcc28f4a4a) ) /* level 5 */
	ROM_LOAD( "h-ic101.bin",0x16000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "p-ic32.bin", 0x18000, 0x2000, CRC(485dcef9) SHA1(0f760ebb42cc2580a29758c72428a41d74477ce6) ) /* level 6 */
	ROM_LOAD( "n-ic47.bin", 0x1a000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )
	ROM_LOAD( "m-ic34.bin", 0x1c000, 0x2000, CRC(4ca984ce) SHA1(99f294fb203f23929b44baa2dd1825c67dde08a1) ) /* level 7 */
	ROM_LOAD( "l-ic49.bin", 0x1e000, 0x2000, CRC(aee6e05e) SHA1(99b9b1ec996746ddf713ed38192f350f1f32a847) )

	ROM_REGION( 0x1000, "fgtiles", 0 ) /* foreground data */
	ROM_LOAD( "ic111.bin", 0x0000, 0x0800, CRC(fab3899b) SHA1(7e869084ab9ec9902490f5f40b7902c6c1f6d7c9) )
	ROM_LOAD( "ic122.bin", 0x0800, 0x0800, CRC(e5fab290) SHA1(3c25e327105dcdba129a776bb73928683063818c) )

	ROM_REGION( 0x4800, "road", 0 ) /* road data */
	ROM_LOAD( "ic1.bin",   0x0000, 0x1000, CRC(c2f649a6) SHA1(a5b0ff6920187003fa2375ba2c5dfcd39382a9ed) )
	ROM_LOAD( "ic13.bin",  0x1000, 0x1000, CRC(fefcf3be) SHA1(d1b56a8826fec2fcb8e586b7ca4ae67e9b52c911) )
	ROM_LOAD( "ic27.bin",  0x2000, 0x1000, CRC(83195ee5) SHA1(482684e71db41234a31767763066dfc1c61de743) )
	ROM_LOAD( "ic41.bin",  0x3000, 0x1000, CRC(3158a549) SHA1(1bae6d90dc8c924ba467d617dee7870d73dd0bea) )
	ROM_LOAD( "ic74.bin",  0x4000, 0x0800, CRC(29854c48) SHA1(cab89bc30f83d9746931ddf6f95a6d0c8a517e5d) )

	ROM_REGION( 0x100, "unkproms", 0 )    /* various PROMs */
	ROM_LOAD( "ic90.bin",   0x0000, 0x0100, CRC(eb2fd7a2) SHA1(2c50ab05305bed4e336fc198f58dc3aa06a3bdfd) ) // bootleg specific?

	ROM_REGION( 0x1020, "proms", 0 )    /* various PROMs */
	ROM_LOAD( "74s288.ic13",  0x0000, 0x0020, CRC(78aded46) SHA1(c78afe804f8b8e837b0c502de5b8715a41fb92b9) )  /* road red/green color table */
	ROM_LOAD( "74s288.ic18",  0x0020, 0x0020, CRC(172d0835) SHA1(e2125f5025d69021b799d0c80b61e05af53ec633) )  /* road collision/enable */ // different
	ROM_LOAD( "74s288.ic20",  0x0040, 0x0020, CRC(3956767d) SHA1(073aaf57175526660fcf7af2e16e7f1d1aaba9a9) )  /* collision detection */
	ROM_LOAD( "74s288.ic21",  0x0060, 0x0020, CRC(f06d9907) SHA1(f11db7800f41b03e79f5eef8d7ef3ae0a6277518) )  /* road green/blue color table */
	ROM_LOAD( "ic99.bin",   0x0100, 0x0100, CRC(59f36e1c) SHA1(0d3ea6218f4ef1ac3181903e31eeea7786141f52) )  /* background color table */ // different

	// PROMs below weren't in this zip, missing?
	ROM_LOAD( "pr-1119.cpu-ic50",   0x0200, 0x0200, CRC(57ebd4bc) SHA1(932649da3537666f95833a8a8aff506217bd9aa1) )  /* sprite Y scaling */
	ROM_LOAD( "pr-1120.cpu-ic62",   0x0400, 0x0200, CRC(8dd4c8a8) SHA1(e8d9cf08f115d57c44746fa0ff28f47b064b4193) )  /* video timing */
	ROM_LOAD( "pr-1121.prom-ic29",  0x0600, 0x0200, CRC(7692f497) SHA1(42468c0705df9928e15ff8deb7e793a6c0c04353) )  /* palette */
	ROM_LOAD( "pr-1122.prom-ic11",  0x0800, 0x0400, CRC(1a86ce70) SHA1(cab708b9a089b2e28f2298c1e4fae6e200923527) )  /* sprite priorities */
	ROM_LOAD( "pr-1123.prom-ic12",  0x0c00, 0x0400, CRC(02d2cb52) SHA1(c34d6b60355747ce20fcb8d322df0e188d187f10) )  /* sprite/road/background priorities */
	ROM_LOAD( "pr-1279.sound-ic40", 0x1000, 0x0020, CRC(b369a6ae) SHA1(dda7c6cf58ce5173f29a3084c85393c0c4587086) )  /* sound board PROM */
ROM_END

ROM_START( subroc3d )
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "epr-1614a.cpu-ic88", 0x0000, 0x2000, CRC(0ed856b4) SHA1(c2f48170365a53bff312ca20df5b74466de6349a) )
	ROM_LOAD( "epr-1615.cpu-ic87",  0x2000, 0x2000, CRC(6281eb2e) SHA1(591d7f184f51f33fb583c916eddacf4581d612d7) )
	ROM_LOAD( "epr-1616.cpu-ic86",  0x4000, 0x2000, CRC(cc7b0c9b) SHA1(0b44c9a2421a51bdc16a2b590f24fbbfb47ef86f) )

	ROM_REGION( 0x40000, "sprites", 0 )    /* sprite data */
	ROM_LOAD( "epr-1417.prom-ic29",  0x00000, 0x2000, CRC(2aaff4e0) SHA1(4b4e4f65d63fb9648108c5f01248ffcb3b4bc54f) )    /* level 0 */
	ROM_LOAD( "epr-1418.prom-ic30",  0x02000, 0x2000, CRC(41ff0f15) SHA1(c441c5368a3faf2544d617e1ceb5cb8eac23017d) )
	ROM_LOAD( "epr-1419.prom-ic55",  0x08000, 0x2000, CRC(37ac818c) SHA1(26b15f410c6a6dcde498e20cece973d5ba23b0de) )    /* level 1 */
	ROM_LOAD( "epr-1420.prom-ic56",  0x0a000, 0x2000, CRC(41ff0f15) SHA1(c441c5368a3faf2544d617e1ceb5cb8eac23017d) )
	ROM_LOAD( "epr-1422.prom-ic81",  0x10000, 0x2000, CRC(0221db58) SHA1(8a157168610bf867a038229ad345de8f95741d04) )    /* level 2 */
	ROM_LOAD( "epr-1423.prom-ic82",  0x12000, 0x2000, CRC(08b1a4b8) SHA1(8e64228911863bf93fdf8a17a2ddca739fb20cd6) )
	ROM_LOAD( "epr-1421.prom-ic80",  0x16000, 0x2000, CRC(1db33c09) SHA1(1b2ec0c15fb178bed7cd2c877a6679ac6c59955c) )
	ROM_LOAD( "epr-1425.prom-ic107", 0x18000, 0x2000, CRC(0221db58) SHA1(8a157168610bf867a038229ad345de8f95741d04) )    /* level 3 */
	ROM_LOAD( "epr-1426.prom-ic108", 0x1a000, 0x2000, CRC(08b1a4b8) SHA1(8e64228911863bf93fdf8a17a2ddca739fb20cd6) )
	ROM_LOAD( "epr-1424.prom-ic106", 0x1e000, 0x2000, CRC(1db33c09) SHA1(1b2ec0c15fb178bed7cd2c877a6679ac6c59955c) )
	ROM_LOAD( "epr-1664.prom-ic116", 0x20000, 0x2000, CRC(6c93ece7) SHA1(b6523f08862f70743422283d7d46e226994add8c) )    /* level 4 */
	ROM_LOAD( "epr-1427.prom-ic115", 0x22000, 0x2000, CRC(2f8cfc2d) SHA1(1ee1b57cf7133aee5c12d654112883af36dff2fa) )
	ROM_LOAD( "epr-1429.prom-ic117", 0x26000, 0x2000, CRC(80e649c7) SHA1(433c847e05a072af8fd7a4d1f50ad856f569c0a6) )
	ROM_LOAD( "epr-1665.prom-ic90",  0x28000, 0x2000, CRC(6c93ece7) SHA1(b6523f08862f70743422283d7d46e226994add8c) )    /* level 5 */
	ROM_LOAD( "epr-1430.prom-ic89",  0x2a000, 0x2000, CRC(2f8cfc2d) SHA1(1ee1b57cf7133aee5c12d654112883af36dff2fa) )
	ROM_LOAD( "epr-1432.prom-ic91",  0x2e000, 0x2000, CRC(d9cd98d0) SHA1(4e1c135ea19375c6a97aac3d134572a45972c56a) )
	ROM_LOAD( "epr-1666.prom-ic64",  0x30000, 0x2000, CRC(6c93ece7) SHA1(b6523f08862f70743422283d7d46e226994add8c) )    /* level 6 */
	ROM_LOAD( "epr-1433.prom-ic63",  0x32000, 0x2000, CRC(2f8cfc2d) SHA1(1ee1b57cf7133aee5c12d654112883af36dff2fa) )
	ROM_LOAD( "epr-1436.prom-ic66",  0x34000, 0x2000, CRC(fc4ad926) SHA1(bf6659ac9eaf5e85bc73848ab4e0c6c7413b55a8) )
	ROM_LOAD( "epr-1435.prom-ic65",  0x36000, 0x2000, CRC(40662eef) SHA1(23bf268ea93288af90bd0e8d6f506a5b92490829) )
	ROM_LOAD( "epr-1438.prom-ic38",  0x38000, 0x2000, CRC(d563d4c1) SHA1(81ebb65c3c0a44aaddf6895a80533436b87a15c7) )    /* level 7 */
	ROM_LOAD( "epr-1437.prom-ic37",  0x3a000, 0x2000, CRC(18ba6aad) SHA1(b959f09739909b835d790928f35b7f7e6bd52c31) )
	ROM_LOAD( "epr-1440.prom-ic40",  0x3c000, 0x2000, CRC(3a0e659c) SHA1(51e64b2417cf3b599aa9ecc84457462a5dca2a61) )
	ROM_LOAD( "epr-1439.prom-ic39",  0x3e000, 0x2000, CRC(3d051668) SHA1(aa4f6152235f07ad39019c46dfacf69d70a7fdcc) )

	ROM_REGION( 0x01000, "fgtiles", 0 )    /* foreground data */
	ROM_LOAD( "epr-1618.cpu-ic82",  0x0000, 0x0800, CRC(a25fea71) SHA1(283efee3951d081119d756114f9f49c2996de5f2) )
	ROM_LOAD( "epr-1617.cpu-ic83",  0x0800, 0x0800, CRC(f70c678e) SHA1(1fabf0011fa4fefd29daf18d4ed6b2cbec14e7b7) )

	ROM_REGION( 0x0a00, "proms", 0 )    /* various PROMs */
	ROM_LOAD( "pr-1419.cpu-ic108", 0x00000, 0x0200, CRC(2cfa2a3f) SHA1(7e2ed2f4ef3324c41da153828c7976e7ba91af7c) )  /* color prom */
	ROM_LOAD( "pr-1620.cpu-ic62",  0x00200, 0x0100, CRC(0ab7ef09) SHA1(b89f8889e2c1220b381e1d6ecc4105cb4152e350) )  /* char color palette */
	ROM_LOAD( "pr-1449.cpu-ic5",   0x00300, 0x0200, CRC(5eb9ff47) SHA1(b8b1e7cfb8aa380663684df6090c48c7c57a6d50) )  /* sprite Y scaling */
	ROM_LOAD( "pr-1450.cpu-ic21",  0x00500, 0x0200, CRC(66bdb00c) SHA1(3956647b27a73770bd163eb7ad29fcd9243dac83) )  /* sprite priority */
	ROM_LOAD( "pr-1451.cpu-ic58",  0x00700, 0x0200, CRC(6a575261) SHA1(79f690db671e471153cbdf1939e733da74fcdc08) )  /* video timing */
	ROM_LOAD( "pr-1453.cpu-ic39",  0x00900, 0x0020, CRC(181c6d23) SHA1(4749b205cbaa513ee65a644946235d2cfe275648) )  /* sprite state machine */
	ROM_LOAD( "pr-1454.cpu-ic67",  0x00920, 0x0020, CRC(dc683440) SHA1(8469914d364dc8f9d0839cae3c864de3b2f3c8df) )  /* flipped tilemap addressing */
ROM_END


ROM_START( buckrog ) /* CPU BOARD Sega ID#  834-5158-01, ROM BOARD Sega ID# 834-5152-01 */
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "epr-5265.cpu-ic3", 0x0000, 0x4000, CRC(f0055e97) SHA1(f6ee2afd6fef710949087d1cb04cbc242d1fa9f5) ) /* encrypted Z80 code, SEGA 315-5014 CPU */
	ROM_LOAD( "epr-5266.cpu-ic4", 0x4000, 0x4000, CRC(7d084c39) SHA1(ef2c0a2a59e14d9e196fd3837139fc5acf0f63be) ) /* encrypted Z80 code, SEGA 315-5014 CPU */

	ROM_REGION( 0x2000, "subcpu", 0 )
	ROM_LOAD( "epr-5200.cpu-ic66", 0x0000, 0x1000, CRC(0d58b154) SHA1(9f3951eb7ea1fa9ff914738462e4b4f755d60802) )

	ROM_REGION( 0x40000, "sprites", 0 ) /* sprite data */
	ROM_LOAD( "epr-5216.prom-ic100", 0x00000, 0x2000, CRC(8155bd73) SHA1(b6814f03eafe16457655598685b4827456b86335) )    /* level 0 */
	ROM_LOAD( "epr-5213.prom-ic84",  0x08000, 0x2000, CRC(fd78dda4) SHA1(4328b5782cbe692765eac43a8eba40bdf2e41921) )    /* level 1 */
	ROM_LOAD( "epr-5262.prom-ic68",  0x10000, 0x4000, CRC(2a194270) SHA1(8d4e444bd8a4e2fa32099787849e6c02cffe49b0) )    /* level 2 */
	ROM_LOAD( "epr-5260.prom-ic52",  0x18000, 0x4000, CRC(b31a120f) SHA1(036cdf56cb43b892609a8f793d5ca66940bf128e) )    /* level 3 */
	ROM_LOAD( "epr-5259.prom-ic43",  0x20000, 0x4000, CRC(d3584926) SHA1(7ad410ad84447a3edba2c51c4ec4314a117fffe7) )    /* level 4 */
	ROM_LOAD( "epr-5261.prom-ic59",  0x28000, 0x4000, CRC(d83c7fcf) SHA1(4c4a590762ef87a3057a12e8d4310decbeb8613c) )    /* level 5 */
	ROM_LOAD( "epr-5208.prom-ic58",  0x2c000, 0x2000, CRC(d181fed2) SHA1(fd46e609b7e04d0661c84ad0faa616d75b8ba89f) )
	ROM_LOAD( "epr-5263.prom-ic75",  0x30000, 0x4000, CRC(1bd6e453) SHA1(472fbc7add05b96e368b961c5ef7ef27f3896216) )    /* level 6 */
	ROM_LOAD( "epr-5237.prom-ic74",  0x34000, 0x2000, CRC(c34e9b82) SHA1(9e69fe9dcc631783e43abe356657f3c6a6a533d8) )
	ROM_LOAD( "epr-5264.prom-ic91",  0x38000, 0x4000, CRC(221f4ced) SHA1(07498c9105c4c4589b19c2bc36abafb176de7bda) )    /* level 7 */
	ROM_LOAD( "epr-5238.prom-ic90",  0x3c000, 0x2000, CRC(7aff0886) SHA1(09ed9fa973257bb23b488e02ef9e02d867e4c366) )

	ROM_REGION( 0x01000, "fgtiles", 0 )    /* foreground data */
	ROM_LOAD( "epr-5201.cpu-ic102",  0x0000, 0x0800, CRC(7f21b0a4) SHA1(b6d784031ffecb36863ae1d81eeaaf8f76ab83df) )
	ROM_LOAD( "epr-5202.cpu-ic103",  0x0800, 0x0800, CRC(43f3e5a7) SHA1(2714943b6720311c5d226db3b6fe95d072677793) )

	ROM_REGION( 0x2000, "bgcolor", 0 ) /* background color data */
	ROM_LOAD( "epr-5203.cpu-ic91", 0x0000, 0x2000, CRC(631f5b65) SHA1(ce8b23cf97f7e08a13f426964ef140a20a884335) )

	ROM_REGION( 0x0b00, "proms", 0 )    /* various PROMs */
	ROM_LOAD( "pr-5194.cpu-ic39", 0x0000, 0x0020, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )  /* char layer X shift */
	ROM_LOAD( "pr-5195.cpu-ic53", 0x0020, 0x0020, CRC(181c6d23) SHA1(4749b205cbaa513ee65a644946235d2cfe275648) )  /* sprite state machine */
	ROM_LOAD( "pr-5196.cpu-ic10", 0x0100, 0x0200, CRC(04204bcf) SHA1(5636eb184463ac58fcfd20012d13d14fb0769124) )  /* sprite Y scaling */
	ROM_LOAD( "pr-5197.cpu-ic78", 0x0300, 0x0200, CRC(a42674af) SHA1(db3590dd0d0f8a85d4ba32ac4ee33f2f4ee4c348) )  /* video timing */
	ROM_LOAD( "pr-5198.cpu-ic93", 0x0500, 0x0200, CRC(32e74bc8) SHA1(dd2c812efd7b8f6b31a45e698d6453ea6bec132e) )  /* char color table */
	ROM_LOAD( "pr-5233.cpu-ic95", 0x0700, 0x0400, CRC(1cd08c4e) SHA1(fb3081548f157d705211a5f07261cf4ad1ebb453) )  /* sprite color table */
ROM_END


ROM_START( buckrogn )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "cpu-ic3.bin", 0x0000, 0x4000, CRC(7f1910af) SHA1(22d37750282676d8fd1f602e928c174f823245c9) )
	ROM_LOAD( "cpu-ic4.bin", 0x4000, 0x4000, CRC(5ecd393b) SHA1(d069f12326644f2c685e516d91d33b97ec162c56) )

	ROM_REGION( 0x2000, "subcpu", 0 )
	ROM_LOAD( "epr-5200.cpu-ic66", 0x0000, 0x1000, CRC(0d58b154) SHA1(9f3951eb7ea1fa9ff914738462e4b4f755d60802) )

	ROM_REGION( 0x40000, "sprites", 0 ) /* sprite data */
	ROM_LOAD( "epr-5216.prom-ic100",  0x00000, 0x2000, CRC(8155bd73) SHA1(b6814f03eafe16457655598685b4827456b86335) )   /* level 0 */
	ROM_LOAD( "epr-5213.prom-ic84",   0x08000, 0x2000, CRC(fd78dda4) SHA1(4328b5782cbe692765eac43a8eba40bdf2e41921) )   /* level 1 */
	ROM_LOAD( "epr-5262.prom-ic68",   0x10000, 0x4000, CRC(2a194270) SHA1(8d4e444bd8a4e2fa32099787849e6c02cffe49b0) )   /* level 2 */
	ROM_LOAD( "epr-5260.prom-ic52",   0x18000, 0x4000, CRC(b31a120f) SHA1(036cdf56cb43b892609a8f793d5ca66940bf128e) )   /* level 3 */
	ROM_LOAD( "epr-5259.prom-ic43",   0x20000, 0x4000, CRC(d3584926) SHA1(7ad410ad84447a3edba2c51c4ec4314a117fffe7) )   /* level 4 */
	ROM_LOAD( "epr-5261.prom-ic59",   0x28000, 0x4000, CRC(d83c7fcf) SHA1(4c4a590762ef87a3057a12e8d4310decbeb8613c) )   /* level 5 */
	ROM_LOAD( "epr-5208.prom-ic58",   0x2c000, 0x2000, CRC(d181fed2) SHA1(fd46e609b7e04d0661c84ad0faa616d75b8ba89f) )
	ROM_LOAD( "epr-5263.prom-ic75",   0x30000, 0x4000, CRC(1bd6e453) SHA1(472fbc7add05b96e368b961c5ef7ef27f3896216) )   /* level 6 */
	ROM_LOAD( "epr-5237.prom-ic74",   0x34000, 0x2000, CRC(c34e9b82) SHA1(9e69fe9dcc631783e43abe356657f3c6a6a533d8) )
	ROM_LOAD( "epr-5264.prom-ic91",   0x38000, 0x4000, CRC(221f4ced) SHA1(07498c9105c4c4589b19c2bc36abafb176de7bda) )   /* level 7 */
	ROM_LOAD( "epr-5238.prom-ic90",   0x3c000, 0x2000, CRC(7aff0886) SHA1(09ed9fa973257bb23b488e02ef9e02d867e4c366) )

	ROM_REGION( 0x01000, "fgtiles", 0 )    /* foreground data */
	ROM_LOAD( "epr-5201.cpu-ic102",  0x0000, 0x0800, CRC(7f21b0a4) SHA1(b6d784031ffecb36863ae1d81eeaaf8f76ab83df) )
	ROM_LOAD( "epr-5202.cpu-ic103",  0x0800, 0x0800, CRC(43f3e5a7) SHA1(2714943b6720311c5d226db3b6fe95d072677793) )

	ROM_REGION( 0x2000, "bgcolor", 0 ) /* background color data */
	ROM_LOAD( "epr-5203.cpu-ic91", 0x0000, 0x2000, CRC(631f5b65) SHA1(ce8b23cf97f7e08a13f426964ef140a20a884335) )

	ROM_REGION( 0x0b00, "proms", 0 )    /* various PROMs */
	ROM_LOAD( "pr-5194.cpu-ic39", 0x0000, 0x0020, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )  /* char layer X shift */
	ROM_LOAD( "pr-5195.cpu-ic53", 0x0020, 0x0020, CRC(181c6d23) SHA1(4749b205cbaa513ee65a644946235d2cfe275648) )  /* sprite state machine */
	ROM_LOAD( "pr-5196.cpu-ic10", 0x0100, 0x0200, CRC(04204bcf) SHA1(5636eb184463ac58fcfd20012d13d14fb0769124) )  /* sprite Y scaling */
	ROM_LOAD( "pr-5197.cpu-ic78", 0x0300, 0x0200, CRC(a42674af) SHA1(db3590dd0d0f8a85d4ba32ac4ee33f2f4ee4c348) )  /* video timing */
	ROM_LOAD( "pr-5198.cpu-ic93", 0x0500, 0x0200, CRC(32e74bc8) SHA1(dd2c812efd7b8f6b31a45e698d6453ea6bec132e) )  /* char color table */
	ROM_LOAD( "pr-5199.cpu-ic95", 0x0700, 0x0400, CRC(45e997a8) SHA1(023703b90b503310351b12157b1e732e61430fa5) )  /* sprite color table */
ROM_END

ROM_START( buckrogn2 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "epr-5204.cpu-ic3", 0x0000, 0x4000, CRC(c2d43741) SHA1(ad435278de101b32e931a2a1a6cdba9be7b7da73) )
	ROM_LOAD( "epr-5205.cpu-ic4", 0x4000, 0x4000, CRC(648f3546) SHA1(2eefdab44aea5fe6fa8e302032c725615b9fdb8a) )

	ROM_REGION( 0x2000, "subcpu", 0 )
	ROM_LOAD( "epr-5200.cpu-ic66", 0x0000, 0x1000, CRC(0d58b154) SHA1(9f3951eb7ea1fa9ff914738462e4b4f755d60802) )

	ROM_REGION( 0x40000, "sprites", 0 ) /* sprite data */
	ROM_LOAD( "epr-5216.prom-ic100",  0x00000, 0x2000, CRC(8155bd73) SHA1(b6814f03eafe16457655598685b4827456b86335) )   /* level 0 */
	ROM_LOAD( "epr-5213.prom-ic84",   0x08000, 0x2000, CRC(fd78dda4) SHA1(4328b5782cbe692765eac43a8eba40bdf2e41921) )   /* level 1 */
	ROM_LOAD( "epr-5210.prom-ic68",   0x10000, 0x4000, CRC(c25b7b9e) SHA1(4418ed056d3240279ce83a872d5887cce374c24e) )   /* level 2 */
	ROM_LOAD( "epr-5235.prom-ic52",   0x18000, 0x4000, CRC(0ba5dac1) SHA1(3a9ab6d3ad1e4bff216412c161e0dc8079c7167e) )   /* level 3 */
	ROM_LOAD( "epr-5234.prom-ic43",   0x20000, 0x4000, CRC(6b773a81) SHA1(5ebcdf8466e634e01e1dbb339c60387ffd471b1d) )   /* level 4 */
	ROM_LOAD( "epr-5236.prom-ic59",   0x28000, 0x4000, CRC(d11ce162) SHA1(c0c7645b2886e133506a203c3feb773d7dba5f2b) )   /* level 5 */
	ROM_LOAD( "epr-5208.prom-ic58",   0x2c000, 0x2000, CRC(d181fed2) SHA1(fd46e609b7e04d0661c84ad0faa616d75b8ba89f) )
	ROM_LOAD( "epr-5212.prom-ic75",   0x30000, 0x4000, CRC(9359ec4f) SHA1(4783527b9961df259e7fbbf8db0b599882dd1207) )   /* level 6 */
	ROM_LOAD( "epr-5237.prom-ic74",   0x34000, 0x2000, CRC(c34e9b82) SHA1(9e69fe9dcc631783e43abe356657f3c6a6a533d8) )
	ROM_LOAD( "epr-5215.prom-ic91",   0x38000, 0x4000, CRC(f5dacc53) SHA1(fe536d16ccb249c26a046f60dc804f5d3be430dc) )   /* level 7 */
	ROM_LOAD( "epr-5238.prom-ic90",   0x3c000, 0x2000, CRC(7aff0886) SHA1(09ed9fa973257bb23b488e02ef9e02d867e4c366) )

	ROM_REGION( 0x01000, "fgtiles", 0 )    /* foreground data */
	ROM_LOAD( "epr-5201.cpu-ic102",  0x0000, 0x0800, CRC(7f21b0a4) SHA1(b6d784031ffecb36863ae1d81eeaaf8f76ab83df) )
	ROM_LOAD( "epr-5202.cpu-ic103",  0x0800, 0x0800, CRC(43f3e5a7) SHA1(2714943b6720311c5d226db3b6fe95d072677793) )

	ROM_REGION( 0x2000, "bgcolor", 0 ) /* background color data */
	ROM_LOAD( "epr-5203.cpu-ic91", 0x0000, 0x2000, CRC(631f5b65) SHA1(ce8b23cf97f7e08a13f426964ef140a20a884335) )

	ROM_REGION( 0x0b00, "proms", 0 )    /* various PROMs */
	ROM_LOAD( "pr-5194.cpu-ic39", 0x0000, 0x0020, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )  /* char layer X shift */
	ROM_LOAD( "pr-5195.cpu-ic53", 0x0020, 0x0020, CRC(181c6d23) SHA1(4749b205cbaa513ee65a644946235d2cfe275648) )  /* sprite state machine */
	ROM_LOAD( "pr-5196.cpu-ic10", 0x0100, 0x0200, CRC(04204bcf) SHA1(5636eb184463ac58fcfd20012d13d14fb0769124) )  /* sprite Y scaling */
	ROM_LOAD( "pr-5197.cpu-ic78", 0x0300, 0x0200, CRC(a42674af) SHA1(db3590dd0d0f8a85d4ba32ac4ee33f2f4ee4c348) )  /* video timing */
	ROM_LOAD( "pr-5198.cpu-ic93", 0x0500, 0x0200, CRC(32e74bc8) SHA1(dd2c812efd7b8f6b31a45e698d6453ea6bec132e) )  /* char color table */
	ROM_LOAD( "pr-5233.cpu-ic95", 0x0700, 0x0400, CRC(1cd08c4e) SHA1(fb3081548f157d705211a5f07261cf4ad1ebb453) )  /* sprite color table */
ROM_END

/*
Zoom 909 (alternate title for Buck Rogers Planet of Zoom for a different region?)
Sega, 1982

PCB Layouts
-----------

Top
---
834-5122 (Sound Board)
|------------------------------------------------------|
|                                                      |
|             4066  555     LM324  LM324  LM324        |
|                                                      |
| 7404  7407  4066  555 555 MB4391                     |
|                                                      |
| 40175 40175 7438  7439    MB4391 MB4391 LM324        |
|                                                      |
| 53323   53323  IR3702     IR3702 LM324  LM324  P138MM|
|                                                5837N |
| 53323   53323  53323      IR3702                     |
|                                           LA4460 VOL |
|------------------------------------------------------|
Notes:
      All IC's shown.


Middle
------
834-5120  171-5011  CPU BOARD  (sticker 834-5142)
|------------------------------------------------------|
|D8279    D8255  EPR-5200  Z80   20MHz    SEGA         |
|    PR-5199     6116                     315-5014     |
|                                                      |
|    EPR-5203  8264                                    |
|              8264                       6116         |
|              8264                          EPR-5217B |
|                                                      |
|                                         *  EPR-5218B |
|2                                                     |
|2    PR-5198                                          |
|W                                                     |
|A                                              2148   |
|Y         6116                                        |
|                                               2148   |
|                          PR-5195  PR-5194            |
|               PR-5197                         2148   |
|      EPR-5201                                        |
|                                               2148   |
|      EPR-5202                                        |
|                                                      |
| DSW2  DSW1                                   PR-5196 |
|                                                      |
| D8255                                                |
|                                                      |
|------------------------------------------------------|
Notes:
      6116          : 2K  x8 SRAM
      8264          : 64K x4 DRAM
      2148          : 1K  x4 SRAM
      Z80 clock     : 5.000MHz
      315-5014 clock: measured 3.766MHz to 3.815MHz on pin6; moving slowly!? (NOTE! This is an encrypted Z80)
      VSync         : 60Hz
      VCO voltage   : 1.43 Volts
                      Note! This is guessed to make the sprites a reasonable size. A shot of the title screen
                      after coinup from a real machine would help to fix the real voltage and give the correct
                      sprite sizes. Note in MAME the Buck Rogers title isn't centered because of the guessed
                      voltage, though I'm sure it's very close.
                      Is it like that on the real machine?

      Label           ROM Type
      EPR-5200.66     2732
      EPR-5201.102    2716
      EPR-5202.103    2716
      EPR-5203.91     2764
      EPR-5217B.3     27128
      EPR-5218B.4     27128
      PR-5194.39      TBP18S030 (compatible with 82S123)
      PR-5195.53      TBP18S030 (compatible with 82S123)
      PR-5196.10      TBP28S46N (compatible with 82S141)
      PR-5197.78      TBP28S46N (compatible with 82S141)
      PR-5198.93      TBP28S46N (compatible with 82S141)
      PR-5199.95      82S181
      *               Empty socket


Bottom
------
(sticker 834-5151)  171-5012  ROM BOARD
|------------------------------------------------------|
|                                                      |
|                                                      |
| EPR-5214  EPR-5211  EPR-5208      *                  |
|                                                      |
| EPR-5215  EPR-5212  EPR-5209  EPR-5206               |
|                                                      |
|                                                      |
|                                                      |
|                                                      |
|                                          NEC         |
|                                          uPC624      |
|                                                      |
|                                               NEC    |
|                                               C159A  |
|                                                      |
|                                          TL084  TL084|
|                                                      |
|                                                 4066 |
|                                                 4066 |
| EPR-5216  EPR-5213  EPR-5231  EPR-5207               |
|                                                75365 |
|    *          *         *         *            75365 |
|                                                      |
|                                                      |
|------------------------------------------------------|
Notes:
      Label          ROM Type
      EPR-5206.43    27128
      EPR-5207.52    27128
      EPR-5208.58    2764
      EPR-5209.59    27128
      EPR-5211.74    2764
      EPR-5212.75    27128
      EPR-5213.84    2764
      EPR-5214.90    2764
      EPR-5215.91    27128
      EPR-5216.100   2764
      EPR-5231.68    27128
      *              Empty socket

      Note! On my PCB, ROMs 58 and 74 match. It could be
      wrong, but the PCB appears to work perfectly.
*/

ROM_START( zoom909 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "epr-5217b.cpu-ic3",  0x0000, 0x4000, CRC(1b56e7dd) SHA1(ccf638c318ebce754ac9628271d2064e05ced35c) )  /* encrypted Z80 code, SEGA 315-5014 CPU  */
	ROM_LOAD( "epr-5218b.cpu-ic4",  0x4000, 0x4000, CRC(77dfd911) SHA1(cc1d4aac863b2d6b52eff7de2b8233be21aac3c9) )  /* encrypted Z80 code, SEGA 315-5014 CPU  */

	ROM_REGION( 0x2000, "subcpu", 0 )
	ROM_LOAD( "epr-5200.cpu-ic66",  0x0000, 0x1000, CRC(0d58b154) SHA1(9f3951eb7ea1fa9ff914738462e4b4f755d60802) )

	ROM_REGION( 0x40000, "sprites", 0 ) /* sprite data */
	ROM_LOAD( "epr-5216.prom-ic100", 0x00000, 0x2000, CRC(8155bd73) SHA1(b6814f03eafe16457655598685b4827456b86335) )    /* level 0 */
	ROM_LOAD( "epr-5213.prom-ic84",  0x08000, 0x2000, CRC(fd78dda4) SHA1(4328b5782cbe692765eac43a8eba40bdf2e41921) )    /* level 1 */
	ROM_LOAD( "epr-5231.prom-ic68",  0x10000, 0x4000, CRC(f00385fc) SHA1(88f64159fdd9b0b8b6a26e7c52da74189f529eb4) )    /* level 2 */
	ROM_LOAD( "epr-5207.prom-ic52",  0x18000, 0x4000, CRC(644f29d8) SHA1(301b94a522bf7a79195d96ca7a4c2ec6f63d45d2) )    /* level 3 */
	ROM_LOAD( "epr-5206.prom-ic43",  0x20000, 0x4000, CRC(049dc998) SHA1(8184a92895b04ea140f073c2345284c23fba7fd4) )    /* level 4 */
	ROM_LOAD( "epr-5209.prom-ic59",  0x28000, 0x4000, CRC(0ff9ff71) SHA1(9038949b657269a3b3287ad526c0e14ebe87525a) )    /* level 5 */
	ROM_LOAD( "epr-5208.prom-ic58",  0x2c000, 0x2000, CRC(d181fed2) SHA1(fd46e609b7e04d0661c84ad0faa616d75b8ba89f) )
	ROM_LOAD( "epr-5212.prom-ic75",  0x30000, 0x4000, CRC(9359ec4f) SHA1(4783527b9961df259e7fbbf8db0b599882dd1207) )    /* level 6 */
	ROM_LOAD( "epr-5211.prom-ic74",  0x34000, 0x2000, CRC(d181fed2) SHA1(fd46e609b7e04d0661c84ad0faa616d75b8ba89f) )
	ROM_LOAD( "epr-5215.prom-ic91",  0x38000, 0x4000, CRC(f5dacc53) SHA1(fe536d16ccb249c26a046f60dc804f5d3be430dc) )    /* level 7 */
	ROM_LOAD( "epr-5214.prom-ic90",  0x3c000, 0x2000, CRC(68306dd6) SHA1(63644e38b36512d93464280d73344c97d9ec1f78) )

	ROM_REGION( 0x01000, "fgtiles", 0 )    /* foreground data */
	ROM_LOAD( "epr-5201.cpu-ic102", 0x0000, 0x0800, CRC(7f21b0a4) SHA1(b6d784031ffecb36863ae1d81eeaaf8f76ab83df) )
	ROM_LOAD( "epr-5202.cpu-ic103", 0x0800, 0x0800, CRC(43f3e5a7) SHA1(2714943b6720311c5d226db3b6fe95d072677793) )

	ROM_REGION( 0x2000, "bgcolor", 0 ) /* background color data */
	ROM_LOAD( "epr-5203.cpu-ic91",  0x0000, 0x2000, CRC(631f5b65) SHA1(ce8b23cf97f7e08a13f426964ef140a20a884335) )

	ROM_REGION( 0x0b00, "proms", 0 )    /* various PROMs */
	ROM_LOAD( "pr-5194.cpu-ic39", 0x0000, 0x0020, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )  /* char layer X shift */
	ROM_LOAD( "pr-5195.cpu-ic53", 0x0020, 0x0020, CRC(181c6d23) SHA1(4749b205cbaa513ee65a644946235d2cfe275648) )  /* sprite state machine */
	ROM_LOAD( "pr-5196.cpu-ic10", 0x0100, 0x0200, CRC(04204bcf) SHA1(5636eb184463ac58fcfd20012d13d14fb0769124) )  /* sprite Y scaling */
	ROM_LOAD( "pr-5197.cpu-ic78", 0x0300, 0x0200, CRC(a42674af) SHA1(db3590dd0d0f8a85d4ba32ac4ee33f2f4ee4c348) )  /* video timing */
	ROM_LOAD( "pr-5198.cpu-ic93", 0x0500, 0x0200, CRC(32e74bc8) SHA1(dd2c812efd7b8f6b31a45e698d6453ea6bec132e) )  /* char color table */
	ROM_LOAD( "pr-5199.cpu-ic95", 0x0700, 0x0400, CRC(45e997a8) SHA1(023703b90b503310351b12157b1e732e61430fa5) )  /* sprite color table */
ROM_END



/*************************************
 *
 *  Turbo ROM decoding
 *
 *************************************/

void turbo_state::turbo_rom_decode()
{
	/*
	 * The table is arranged this way (second half is mirror image of first)
	 *
	 *      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
	 *
	 * 0   00 00 00 00 01 01 01 01 02 02 02 02 03 03 03 03
	 * 1   04 04 04 04 05 05 05 05 06 06 06 06 07 07 07 07
	 * 2   08 08 08 08 09 09 09 09 0A 0A 0A 0A 0B 0B 0B 0B
	 * 3   0C 0C 0C 0C 0D 0D 0D 0D 0E 0E 0E 0E 0F 0F 0F 0F
	 * 4   10 10 10 10 11 11 11 11 12 12 12 12 13 13 13 13
	 * 5   14 14 14 14 15 15 15 15 16 16 16 16 17 17 17 17
	 * 6   18 18 18 18 19 19 19 19 1A 1A 1A 1A 1B 1B 1B 1B
	 * 7   1C 1C 1C 1C 1D 1D 1D 1D 1E 1E 1E 1E 1F 1F 1F 1F
	 * 8   1F 1F 1F 1F 1E 1E 1E 1E 1D 1D 1D 1D 1C 1C 1C 1C
	 * 9   1B 1B 1B 1B 1A 1A 1A 1A 19 19 19 19 18 18 18 18
	 * A   17 17 17 17 16 16 16 16 15 15 15 15 14 14 14 14
	 * B   13 13 13 13 12 12 12 12 11 11 11 11 10 10 10 10
	 * C   0F 0F 0F 0F 0E 0E 0E 0E 0D 0D 0D 0D 0C 0C 0C 0C
	 * D   0B 0B 0B 0B 0A 0A 0A 0A 09 09 09 09 08 08 08 08
	 * E   07 07 07 07 06 06 06 06 05 05 05 05 04 04 04 04
	 * F   03 03 03 03 02 02 02 02 01 01 01 01 00 00 00 00
	 *
	 */
	static const UINT8 xortable[][32]=
	{
		/* Table 0 */
		/* 0x0000-0x3ff */
		/* 0x0800-0xbff */
		/* 0x4000-0x43ff */
		/* 0x4800-0x4bff */
		{ 0x00,0x44,0x0c,0x48,0x00,0x44,0x0c,0x48,
			0xa0,0xe4,0xac,0xe8,0xa0,0xe4,0xac,0xe8,
			0x60,0x24,0x6c,0x28,0x60,0x24,0x6c,0x28,
			0xc0,0x84,0xcc,0x88,0xc0,0x84,0xcc,0x88 },

		/* Table 1 */
		/* 0x0400-0x07ff */
		/* 0x0c00-0x0fff */
		/* 0x1400-0x17ff */
		/* 0x1c00-0x1fff */
		/* 0x2400-0x27ff */
		/* 0x2c00-0x2fff */
		/* 0x3400-0x37ff */
		/* 0x3c00-0x3fff */
		/* 0x4400-0x47ff */
		/* 0x4c00-0x4fff */
		/* 0x5400-0x57ff */
		/* 0x5c00-0x5fff */
		{ 0x00,0x44,0x18,0x5c,0x14,0x50,0x0c,0x48,
			0x28,0x6c,0x30,0x74,0x3c,0x78,0x24,0x60,
			0x60,0x24,0x78,0x3c,0x74,0x30,0x6c,0x28,
			0x48,0x0c,0x50,0x14,0x5c,0x18,0x44,0x00 }, //0x00 --> 0x10 ?

		/* Table 2 */
		/* 0x1000-0x13ff */
		/* 0x1800-0x1bff */
		/* 0x5000-0x53ff */
		/* 0x5800-0x5bff */
		{ 0x00,0x00,0x28,0x28,0x90,0x90,0xb8,0xb8,
			0x28,0x28,0x00,0x00,0xb8,0xb8,0x90,0x90,
			0x00,0x00,0x28,0x28,0x90,0x90,0xb8,0xb8,
			0x28,0x28,0x00,0x00,0xb8,0xb8,0x90,0x90 },

		/* Table 3 */
		/* 0x2000-0x23ff */
		/* 0x2800-0x2bff */
		/* 0x3000-0x33ff */
		/* 0x3800-0x3bff */
		{ 0x00,0x14,0x88,0x9c,0x30,0x24,0xb8,0xac,
			0x24,0x30,0xac,0xb8,0x14,0x00,0x9c,0x88,
			0x48,0x5c,0xc0,0xd4,0x78,0x6c,0xf0,0xe4,
			0x6c,0x78,0xe4,0xf0,0x5c,0x48,0xd4,0xc0 }
	};

	static const int findtable[]=
	{
		0,1,0,1, /* 0x0000-0x0fff */
		2,1,2,1, /* 0x1000-0x1fff */
		3,1,3,1, /* 0x2000-0x2fff */
		3,1,3,1, /* 0x3000-0x3fff */
		0,1,0,1, /* 0x4000-0x4fff */
		2,1,2,1  /* 0x5000-0x5fff */
	};

	UINT8 *RAM = memregion("maincpu")->base();
	int offs, i, j;
	UINT8 src;

	for (offs = 0x0000; offs < 0x6000; offs++)
	{
		src = RAM[offs];
		i = findtable[offs >> 10];
		j = src >> 2;
		if (src & 0x80) j ^= 0x3f;
		RAM[offs] = src ^ xortable[i][j];
	}
}



/*************************************
 *
 *  Driver init
 *
 *************************************/

DRIVER_INIT_MEMBER(turbo_state,turbo_enc)
{
	turbo_rom_decode();
}


DRIVER_INIT_MEMBER(turbo_state,buckrog_enc)
{
	static const UINT8 convtable[32][4] =
	{
		/*       opcode                   data                     address      */
		/*  A    B    C    D         A    B    C    D                           */
		{ 0x80,0x00,0x88,0x08 }, { 0x28,0x20,0xa8,0xa0 },   /* ...0...0...0...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0xa0,0x80,0x20,0x00 },   /* ...0...0...0...1 */
		{ 0x28,0xa8,0x08,0x88 }, { 0xa8,0xa0,0x88,0x80 },   /* ...0...0...1...0 */
		{ 0x80,0x00,0x88,0x08 }, { 0x28,0x20,0xa8,0xa0 },   /* ...0...0...1...1 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0xa0,0x80,0x20,0x00 },   /* ...0...1...0...0 */
		{ 0x80,0x00,0x88,0x08 }, { 0x28,0x20,0xa8,0xa0 },   /* ...0...1...0...1 */
		{ 0x28,0xa8,0x08,0x88 }, { 0xa8,0xa0,0x88,0x80 },   /* ...0...1...1...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0xa0,0x80,0x20,0x00 },   /* ...0...1...1...1 */
		{ 0x28,0xa8,0x08,0x88 }, { 0xa8,0xa0,0x88,0x80 },   /* ...1...0...0...0 */
		{ 0x80,0x00,0x88,0x08 }, { 0x28,0x20,0xa8,0xa0 },   /* ...1...0...0...1 */
		{ 0x80,0x00,0x88,0x08 }, { 0x28,0x20,0xa8,0xa0 },   /* ...1...0...1...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0xa0,0x80,0x20,0x00 },   /* ...1...0...1...1 */
		{ 0x80,0x00,0x88,0x08 }, { 0x28,0x20,0xa8,0xa0 },   /* ...1...1...0...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0xa0,0x80,0x20,0x00 },   /* ...1...1...0...1 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0xa0,0x80,0x20,0x00 },   /* ...1...1...1...0 */
		{ 0x28,0xa8,0x08,0x88 }, { 0xa8,0xa0,0x88,0x80 }    /* ...1...1...1...1 */
	};
	sega_decode(memregion("maincpu")->base(), m_decrypted_opcodes, 0x8000, convtable);
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAMEL( 1981, turbo,    0,       turbo,     turbo,    driver_device, 0,           ROT270, "Sega", "Turbo (program 1513-1515)", MACHINE_IMPERFECT_SOUND , layout_turbo )
GAMEL( 1981, turboa,   turbo,   turbo,     turbo,    turbo_state,   turbo_enc,   ROT270, "Sega", "Turbo (encrypted, program 1262-1264)", MACHINE_IMPERFECT_SOUND , layout_turbo )
GAMEL( 1981, turbob,   turbo,   turbo,     turbo,    turbo_state,   turbo_enc,   ROT270, "Sega", "Turbo (encrypted, program 1363-1365 rev B)", MACHINE_IMPERFECT_SOUND , layout_turbo )
GAMEL( 1981, turboc,   turbo,   turbo,     turbo,    turbo_state,   turbo_enc,   ROT270, "Sega", "Turbo (encrypted, program 1363-1365 rev A)", MACHINE_IMPERFECT_SOUND , layout_turbo )
GAMEL( 1981, turbod,   turbo,   turbo,     turbo,    turbo_state,   turbo_enc,   ROT270, "Sega", "Turbo (encrypted, program 1363-1365)", MACHINE_IMPERFECT_SOUND , layout_turbo ) // but still reports 1262-1264 in the test mode?
GAMEL( 1981, turbobl,  turbo,   turbo,     turbo,    driver_device, 0,           ROT270, "bootleg", "Indianapolis (bootleg of Turbo)", MACHINE_IMPERFECT_SOUND , layout_turbo ) // decrypted bootleg of a 1262-1264 set

GAMEL( 1982, subroc3d, 0,       subroc3d,  subroc3d, driver_device, 0,           ORIENTATION_FLIP_X, "Sega", "Subroc-3D", MACHINE_IMPERFECT_SOUND , layout_subroc3d )

GAMEL( 1982, buckrog,  0,       buckrog,   buckrog,  turbo_state,   buckrog_enc, ROT0, "Sega", "Buck Rogers: Planet of Zoom", MACHINE_IMPERFECT_SOUND , layout_buckrog )
GAMEL( 1982, buckrogn, buckrog, buckrogu,  buckrog,  driver_device, 0,           ROT0, "Sega", "Buck Rogers: Planet of Zoom (not encrypted, set 1)", MACHINE_IMPERFECT_SOUND , layout_buckrog )
GAMEL( 1982, buckrogn2,buckrog, buckrogu,  buckrog,  driver_device, 0,           ROT0, "Sega", "Buck Rogers: Planet of Zoom (not encrypted, set 2)", MACHINE_IMPERFECT_SOUND , layout_buckrog )
GAMEL( 1982, zoom909,  buckrog, buckrog,   buckrog,  turbo_state,   buckrog_enc, ROT0, "Sega", "Zoom 909", MACHINE_IMPERFECT_SOUND, layout_buckrog )

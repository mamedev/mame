// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Sega Zaxxon hardware

    Games supported:
        * Zaxxon
        * Super Zaxxon
        * Future Spy
        * Razmatazz
        * Ixion
        * Congo Bongo

    DIP locations verified from manual for:
        * Zaxxon
        * Congo Bongo

    Known bugs:
        * discrete sound emulation

****************************************************************************

    Zaxxon memory map (preliminary)

    0000-1fff ROM 3
    2000-3fff ROM 2
    4000-4fff ROM 1
    6000-67ff RAM 1
    6800-6fff RAM 2
    8000-83ff Video RAM
    a000-a0ff sprites

    read:
    c000      IN0
    c001      IN1
    c002      DSW0
    c003      DSW1
    c100      IN2
    see the input_ports definition below for details on the input bits

    write:
    c000      coin A enable
    c001      coin B enable
    c002      coin aux enable
    c003-c004 coin counters
    c006      flip screen
    ff3c-ff3f sound (see below)
    fff0      interrupt enable
    fff1      character color bank (not used during the game, but used in test mode)
    fff8-fff9 background playfield position (11 bits)
    fffa      background color bank (0 = standard  1 = reddish)
    fffb      background enable

    interrupts:
    VBlank triggers IRQ, handled with interrupt mode 1
    NMI enters the test mode.

    Changes:
    25 Jan 98 LBO
        * Added crude support for samples based on Frank's info. As of yet, I don't have
          a set that matches the names - I need a way to edit the .wav files I have.
          Hopefully I'll be able to create a good set shortly. I also don't know which
          sounds "loop".
    26 Jan 98 LBO
        * Fixed the sound support. I lack explosion samples and the base missile sample so
          these are untested. I'm also unsure about the background noise. It seems to have
          a variable volume so I've tried to reproduce that via just 1 sample.

    12 Mar 98 ATJ
            * For the moment replaced Brad's version of the samples with mine from the Mame/P
              release. As yet, no variable volume, but I will be combining the features from
              Brad's driver into mine ASAP.

****************************************************************************

    Ixion Board Info

    Ixion
    Sega(prototype)  7/1/1983


    Board set is a modified Super Zaxxon, similar to Razzmatazz

    [G80 Sound board]



                            U51
                            U50

    [834-0214 ZAXXON-SOUNDII]

                                       315-5013  U27 U28 U29


             U68 U69      U72
                                            U98

    [         ZAXXON-VIDEOII]


    U77 U78 U79                                U90 U91 U92 U93
                              U111 U112 U113

****************************************************************************

    Congo Bongo memory map (preliminary)

    0000-1fff ROM 1
    2000-3fff ROM 2
    4000-5fff ROM 3
    6000-7fff ROM 4

    8000-87ff RAM 1
    8800-8fff RAM 2
    a000-a3ff Video RAM
    a400-a7ff Color RAM

    8400-8fff sprites

    read:
    c000      IN0
    c001      IN1
    c002      DSW1
    c003      DSW2
    c008      IN2

    write:
    c000-c002 ?
    c006      ?
    c018      coinAen, used to enable input on coin slot A
    c019      coinBen, used to enable input on coin slot B
    c01a      serven,  used to enable input on service sw
    c01b      counterA,coin counter for slot A
    c01c      counterB,coin counter for slot B
    c01d      background enable
    c01e      flip screen
    c01f      interrupt enable

    c028-c029 background playfield position
    c031      sprite enable ?
    c032      sprite start
    c033      sprite count


    interrupts:
    VBlank triggers IRQ, handled with interrupt mode 1
    NMI causes a ROM/RAM test.

    ----------------------------------------------------------------------

    There are two different Congo Bongo boardsets. One is a 3-stack (which I have)
    and the other is a 2-stack.

    The smaller third board is just a dedicated sound board, then later they started
    making 2-stack boardset versions with merged sound circuits built into the control
    board.

    The schematic name for the control board on the two-stack set is "Control Board II"
    my boardset uses control board 1.

    The ROMs that are dumped for MAME are from the 3-stack boardset.
    The ROMs on control board II are in different socket assignments than the ROMs
    found on control board 1.

    For example:
    MAME congo1.bin = ROM 1 at location U35 on the 3-stack control board 1
    MAME congo1.bin = ROM 1 at location U21 on the 2-stack control board II

    So the locations are different between the two boardsets but they appear to use
    the same ROM.

    The Video boards are exactly the same between the two boardset versions (bottom
    board) nothing changed with the video boards.

    Both the 2-stack control board and 3-stack control board use a PROM that's contents
    are identical between the two.  Apparently the 3-Stack Control Board prom was
    dumped as a TBP28L22, because that is how the 2-Stack Control board prom was dumped.

    Board                  Location  Label            PROM Type
    ---------------------  --------  ---------------  ---------
    3-Stack Control Board  U68       MR020 (PR-5308)  TBP28S42?
    2-Stack Control Board  U87       MR019 (PR-5315)  TBP28L22?

    For the 3-stack control board PROM sheet 2 of 6 for the 834-5212 board lists the pinouts
    and they match the physical board.  The PROM also has the TI logo and a date code? of J810A.

    ROM locations are different with ROMs 1-5 on control board II and a different
    numbered ROM in U87.

    ----ROM--NAMES-------------------------------------


    THREE STACK BOARDSET
    control board = 834-5166
    video board = 834-5167
    sound board = 834-5168


    Current MAME      Schematic Board
       Names       Names     Location (3-stack)
    ------------      ---------     -------------------

    congo.u68    =    MR020     U68  control board
    congo1.bin   =    ROM 1     U35  control board
    congo2.bin   =    ROM 2     U34  control board
    congo3.bin   =    ROM 3     U33  control board
    congo4.bin   =    ROM 4     U32  control board
    congo5.bin   =    ROM 5     U76  control board
    congo6.bin   =    ROM 6     U57  video board
    congo7.bin   =    ROM 7     U58  video board
    congo8.bin   =    ROM 8     U93  video board
    congo9.bin   =    ROM 9     U94  video board
    congo10.bin  =    ROM 10    U95  video board
    congo11.bin  =    ROM 11    U77  video board
    congo12.bin  =    ROM 12    U78  video board
    congo13.bin  =    ROM 13    U79  video board
    congo14.bin  =    ROM 14    U104 video board
    congo15.bin  =    ROM 15    U105 video board
    congo16.bin  =    ROM 16    U106 video board
    congo17.bin  =    ROM 17    U11  sound board



    TWO STACK BOARDSET
    control board II = 834-5212
    video board = 834-5167


    Current MAME      Schematic Board
       Names       Names     Location (2-stack)
    ------------      ---------     -------------------

    congo.u68    =    MR019     U87  control board
    congo1.bin   =    ROM 1     U21  control board
    congo2.bin   =    ROM 2     U22  control board
    congo3.bin   =    ROM 3     U23  control board
    congo4.bin   =    ROM 4     U24  control board
    congo5.bin   =    ROM 5     U77  control board
    congo6.bin   =    ROM 6     U57  video board
    congo7.bin   =    ROM 7     U58  video board
    congo8.bin   =    ROM 8     U93  video board
    congo9.bin   =    ROM 9     U94  video board
    congo10.bin  =    ROM 10    U95  video board
    congo11.bin  =    ROM 11    U77  video board
    congo12.bin  =    ROM 12    U78  video board
    congo13.bin  =    ROM 13    U79  video board
    congo14.bin  =    ROM 14    U104 video board
    congo15.bin  =    ROM 15    U105 video board
    congo16.bin  =    ROM 16    U106 video board
    congo17.bin  =    ROM 17    U19  control board

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "sound/samples.h"
#include "machine/segacrpt.h"
#include "machine/i8255.h"
#include "audio/segasnd.h"
#include "includes/zaxxon.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define MASTER_CLOCK        XTAL_48_66MHz
#define SOUND_CLOCK         XTAL_4MHz

#define PIXEL_CLOCK         (MASTER_CLOCK/8)

#define HTOTAL              (384)
#define HBEND               (0)
#define HBSTART             (256)

#define VTOTAL              (264)
#define VBEND               (16)
#define VBSTART             (240)



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

INPUT_CHANGED_MEMBER(zaxxon_state::service_switch)
{
	/* pressing the service switch sends an NMI */
	if (newval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


INTERRUPT_GEN_MEMBER(zaxxon_state::vblank_int)
{
	if (m_int_enabled)
		device.execute().set_input_line(0, ASSERT_LINE);
}


WRITE8_MEMBER(zaxxon_state::int_enable_w)
{
	m_int_enabled = data & 1;
	if (!m_int_enabled)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

void zaxxon_state::machine_start()
{
	/* register for save states */
	save_item(NAME(m_int_enabled));
	save_item(NAME(m_coin_status));
	save_item(NAME(m_coin_enable));
}



/*************************************
 *
 *  Input handlers
 *
 *************************************/

READ8_MEMBER(zaxxon_state::razmataz_counter_r)
{
	/* this behavior is really unknown; however, the code is using this */
	/* counter as a sort of timeout when talking to the sound board */
	/* it needs to be increasing at a reasonable rate but not too fast */
	/* or else the sound will mess up */
	return m_razmataz_counter++ >> 8;
}


CUSTOM_INPUT_MEMBER(zaxxon_state::razmataz_dial_r)
{
	int num = (FPTR)param;
	int res;

	int delta = m_dials[num]->read();

	if (delta < 0x80)
	{
		// right
		m_razmataz_dial_pos[num] -= delta;
		res = (m_razmataz_dial_pos[num] << 1) | 1;
	}
	else
	{
		// left
		m_razmataz_dial_pos[num] += delta;
		res = (m_razmataz_dial_pos[num] << 1);
	}

	return res;
}



/*************************************
 *
 *  Output handlers
 *
 *************************************/

WRITE8_MEMBER(zaxxon_state::zaxxon_coin_counter_w)
{
	coin_counter_w(machine(), offset, data & 0x01);
}


// There is no external coin lockout circuitry; instead, the pcb simply latches
// the coin input, which then needs to be explicitly cleared by the game.
WRITE8_MEMBER(zaxxon_state::zaxxon_coin_enable_w)
{
	m_coin_enable[offset] = data & 1;
	if (!m_coin_enable[offset])
		m_coin_status[offset] = 0;
}


INPUT_CHANGED_MEMBER(zaxxon_state::zaxxon_coin_inserted)
{
	if (newval)
	{
		m_coin_status[(int)(FPTR)param] = m_coin_enable[(int)(FPTR)param];
	}
}


CUSTOM_INPUT_MEMBER(zaxxon_state::zaxxon_coin_r)
{
	return m_coin_status[(int)(FPTR)param];
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

/* complete memory map derived from schematics */
static ADDRESS_MAP_START( zaxxon_map, AS_PROGRAM, 8, zaxxon_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x1c00) AM_RAM_WRITE(zaxxon_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xa000, 0xa0ff) AM_MIRROR(0x1f00) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xc000, 0xc000) AM_MIRROR(0x18fc) AM_READ_PORT("SW00")
	AM_RANGE(0xc001, 0xc001) AM_MIRROR(0x18fc) AM_READ_PORT("SW01")
	AM_RANGE(0xc002, 0xc002) AM_MIRROR(0x18fc) AM_READ_PORT("DSW02")
	AM_RANGE(0xc003, 0xc003) AM_MIRROR(0x18fc) AM_READ_PORT("DSW03")
	AM_RANGE(0xc100, 0xc100) AM_MIRROR(0x18ff) AM_READ_PORT("SW100")
	AM_RANGE(0xc000, 0xc002) AM_MIRROR(0x18f8) AM_WRITE(zaxxon_coin_enable_w)
	AM_RANGE(0xc003, 0xc004) AM_MIRROR(0x18f8) AM_WRITE(zaxxon_coin_counter_w)
	AM_RANGE(0xc006, 0xc006) AM_MIRROR(0x18f8) AM_WRITE(zaxxon_flipscreen_w)
	AM_RANGE(0xe03c, 0xe03f) AM_MIRROR(0x1f00) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0xe0f0, 0xe0f0) AM_MIRROR(0x1f00) AM_WRITE(int_enable_w)
	AM_RANGE(0xe0f1, 0xe0f1) AM_MIRROR(0x1f00) AM_WRITE(zaxxon_fg_color_w)
	AM_RANGE(0xe0f8, 0xe0f9) AM_MIRROR(0x1f00) AM_WRITE(zaxxon_bg_position_w)
	AM_RANGE(0xe0fa, 0xe0fa) AM_MIRROR(0x1f00) AM_WRITE(zaxxon_bg_color_w)
	AM_RANGE(0xe0fb, 0xe0fb) AM_MIRROR(0x1f00) AM_WRITE(zaxxon_bg_enable_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, zaxxon_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM AM_SHARE("decrypted_opcodes")
ADDRESS_MAP_END

/* derived from Zaxxon, different sound hardware */
static ADDRESS_MAP_START( ixion_map, AS_PROGRAM, 8, zaxxon_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x1c00) AM_RAM_WRITE(zaxxon_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xa000, 0xa0ff) AM_MIRROR(0x1f00) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xc000, 0xc000) AM_MIRROR(0x18fc) AM_READ_PORT("SW00")
	AM_RANGE(0xc001, 0xc001) AM_MIRROR(0x18fc) AM_READ_PORT("SW01")
	AM_RANGE(0xc002, 0xc002) AM_MIRROR(0x18fc) AM_READ_PORT("DSW02")
	AM_RANGE(0xc003, 0xc003) AM_MIRROR(0x18fc) AM_READ_PORT("DSW03")
	AM_RANGE(0xc100, 0xc100) AM_MIRROR(0x18ff) AM_READ_PORT("SW100")
	AM_RANGE(0xc000, 0xc002) AM_MIRROR(0x18f8) AM_WRITE(zaxxon_coin_enable_w)
	AM_RANGE(0xc003, 0xc004) AM_MIRROR(0x18f8) AM_WRITE(zaxxon_coin_counter_w)
	AM_RANGE(0xc006, 0xc006) AM_MIRROR(0x18f8) AM_WRITE(zaxxon_flipscreen_w)
	AM_RANGE(0xe03c, 0xe03c) AM_MIRROR(0x1f00) AM_DEVREADWRITE("usbsnd", usb_sound_device, status_r, data_w)
	AM_RANGE(0xe0f0, 0xe0f0) AM_MIRROR(0x1f00) AM_WRITE(int_enable_w)
	AM_RANGE(0xe0f1, 0xe0f1) AM_MIRROR(0x1f00) AM_WRITE(zaxxon_fg_color_w)
	AM_RANGE(0xe0f8, 0xe0f9) AM_MIRROR(0x1f00) AM_WRITE(zaxxon_bg_position_w)
	AM_RANGE(0xe0fa, 0xe0fa) AM_MIRROR(0x1f00) AM_WRITE(zaxxon_bg_color_w)
	AM_RANGE(0xe0fb, 0xe0fb) AM_MIRROR(0x1f00) AM_WRITE(zaxxon_bg_enable_w)
ADDRESS_MAP_END


/* complete memory map derived from schematics */
static ADDRESS_MAP_START( congo_map, AS_PROGRAM, 8, zaxxon_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0xa000, 0xa3ff) AM_MIRROR(0x1800) AM_RAM_WRITE(zaxxon_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xa400, 0xa7ff) AM_MIRROR(0x1800) AM_RAM_WRITE(congo_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xc000, 0xc000) AM_MIRROR(0x1fc4) AM_READ_PORT("SW00")
	AM_RANGE(0xc001, 0xc001) AM_MIRROR(0x1fc4) AM_READ_PORT("SW01")
	AM_RANGE(0xc002, 0xc002) AM_MIRROR(0x1fc4) AM_READ_PORT("DSW02")
	AM_RANGE(0xc003, 0xc003) AM_MIRROR(0x1fc4) AM_READ_PORT("DSW03")
	AM_RANGE(0xc008, 0xc008) AM_MIRROR(0x1fc7) AM_READ_PORT("SW100")
	AM_RANGE(0xc018, 0xc01a) AM_MIRROR(0x1fc0) AM_WRITE(zaxxon_coin_enable_w)
	AM_RANGE(0xc01b, 0xc01c) AM_MIRROR(0x1fc0) AM_WRITE(zaxxon_coin_counter_w)
	AM_RANGE(0xc01d, 0xc01d) AM_MIRROR(0x1fc0) AM_WRITE(zaxxon_bg_enable_w)
	AM_RANGE(0xc01e, 0xc01e) AM_MIRROR(0x1fc0) AM_WRITE(zaxxon_flipscreen_w)
	AM_RANGE(0xc01f, 0xc01f) AM_MIRROR(0x1fc0) AM_WRITE(int_enable_w)
	AM_RANGE(0xc021, 0xc021) AM_MIRROR(0x1fc0) AM_WRITE(zaxxon_fg_color_w)
	AM_RANGE(0xc023, 0xc023) AM_MIRROR(0x1fc0) AM_WRITE(zaxxon_bg_color_w)
	AM_RANGE(0xc026, 0xc026) AM_MIRROR(0x1fc0) AM_WRITE(congo_fg_bank_w)
	AM_RANGE(0xc027, 0xc027) AM_MIRROR(0x1fc0) AM_WRITE(congo_color_bank_w)
	AM_RANGE(0xc028, 0xc029) AM_MIRROR(0x1fc4) AM_WRITE(zaxxon_bg_position_w)
	AM_RANGE(0xc030, 0xc033) AM_MIRROR(0x1fc4) AM_WRITE(congo_sprite_custom_w)
	AM_RANGE(0xc038, 0xc03f) AM_MIRROR(0x1fc0) AM_WRITE(soundlatch_byte_w)
ADDRESS_MAP_END


/* complete memory map derived from schematics */
static ADDRESS_MAP_START( congo_sound_map, AS_PROGRAM, 8, zaxxon_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_MIRROR(0x1800) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x1fff) AM_DEVWRITE("sn1", sn76489a_device, write)
	AM_RANGE(0x8000, 0x8003) AM_MIRROR(0x1ffc) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x1fff) AM_DEVWRITE("sn2", sn76489a_device, write)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( zaxxon )
	PORT_START("SW00")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SW01")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SW100")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, zaxxon_state,zaxxon_coin_r, (void *)0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, zaxxon_state,zaxxon_coin_r, (void *)1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, zaxxon_state,zaxxon_coin_r, (void *)2)

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )    PORT_CHANGED_MEMBER(DEVICE_SELF, zaxxon_state,zaxxon_coin_inserted, (void *)0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )    PORT_CHANGED_MEMBER(DEVICE_SELF, zaxxon_state,zaxxon_coin_inserted, (void *)1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, zaxxon_state,zaxxon_coin_inserted, (void *)2)

	PORT_START("SERVICESW")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_HIGH ) PORT_CHANGED_MEMBER(DEVICE_SELF, zaxxon_state,service_switch, 0)

	PORT_START("DSW02")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x03, "10000" )
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x40, 0x40, "Sound" ) PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW03")
	PORT_DIPNAME( 0x0f, 0x03, DEF_STR ( Coin_B ) ) PORT_DIPLOCATION("SW2:!1,!2,!3,!4")
	PORT_DIPSETTING(    0x0f, DEF_STR ( 4C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR ( 3C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR ( 2C_1C ) )
	PORT_DIPSETTING(    0x06, "2C/1C 5C/3C 6C/4C" )
	PORT_DIPSETTING(    0x0a, "2C/1C 3C/2C 4C/3C" )
	PORT_DIPSETTING(    0x03, DEF_STR ( 1C_1C ) )
	PORT_DIPSETTING(    0x02, "1C/1C 5C/6C" )
	PORT_DIPSETTING(    0x0c, "1C/1C 4C/5C" )
	PORT_DIPSETTING(    0x04, "1C/1C 2C/3C" )
	PORT_DIPSETTING(    0x0d, DEF_STR ( 1C_2C ) )
	PORT_DIPSETTING(    0x08, "1C/2C 5C/11C" )
	PORT_DIPSETTING(    0x00, "1C/2C 4C/9C" )
	PORT_DIPSETTING(    0x05, DEF_STR ( 1C_3C ) )
	PORT_DIPSETTING(    0x09, DEF_STR ( 1C_4C ) )
	PORT_DIPSETTING(    0x01, DEF_STR ( 1C_5C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR ( 1C_6C ) )
	PORT_DIPNAME( 0xf0, 0x30, DEF_STR ( Coin_A ) ) PORT_DIPLOCATION("SW2:!5,!6,!7,!8")
	PORT_DIPSETTING(    0xf0, DEF_STR ( 4C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR ( 3C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR ( 2C_1C ) )
	PORT_DIPSETTING(    0x60, "2C/1C 5C/3C 6C/4C" )
	PORT_DIPSETTING(    0xa0, "2C/1C 3C/2C 4C/3C" )
	PORT_DIPSETTING(    0x30, DEF_STR ( 1C_1C ) )
	PORT_DIPSETTING(    0x20, "1C/1C 5C/6C" )
	PORT_DIPSETTING(    0xc0, "1C/1C 4C/5C" )
	PORT_DIPSETTING(    0x40, "1C/1C 2C/3C" )
	PORT_DIPSETTING(    0xd0, DEF_STR ( 1C_2C ) )
	PORT_DIPSETTING(    0x80, "1C/2C 5C/11C" )
	PORT_DIPSETTING(    0x00, "1C/2C 4C/9C" )
	PORT_DIPSETTING(    0x50, DEF_STR ( 1C_3C ) )
	PORT_DIPSETTING(    0x90, DEF_STR ( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR ( 1C_5C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR ( 1C_6C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( szaxxon )
	PORT_INCLUDE(zaxxon)

	PORT_MODIFY("DSW02")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( futspy )
	PORT_INCLUDE(zaxxon)

	PORT_MODIFY("SW00")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("SW01")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("DSW02")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR ( Coin_A ) )      PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x08, DEF_STR ( 4C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR ( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR ( 2C_1C ) )
	PORT_DIPSETTING(    0x0a, "2C/1C 5C/3C 6C/4C" )
	PORT_DIPSETTING(    0x0b, "2C/1C 4C/3C" )
	PORT_DIPSETTING(    0x00, DEF_STR ( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, "1C/1C 2C/3C" )
	PORT_DIPSETTING(    0x0d, "1C/1C 4C/5C" )
	PORT_DIPSETTING(    0x0c, "1C/1C 5C/6C" )
	PORT_DIPSETTING(    0x09, DEF_STR ( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR ( 1C_2C ) )
	PORT_DIPSETTING(    0x0f, "1C/2C 5C/11C" )
	PORT_DIPSETTING(    0x02, DEF_STR ( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR ( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR ( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR ( 1C_6C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR ( Coin_B ) )      PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x80, DEF_STR ( 4C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR ( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR ( 2C_1C ) )
	PORT_DIPSETTING(    0xa0, "2C/1C 5C/3C 6C/4C" )
	PORT_DIPSETTING(    0xb0, "2C/1C 4C/3C" )
	PORT_DIPSETTING(    0x00, DEF_STR ( 1C_1C ) )
	PORT_DIPSETTING(    0xe0, "1C/1C 2C/3C" )
	PORT_DIPSETTING(    0xd0, "1C/1C 4C/5C" )
	PORT_DIPSETTING(    0xc0, "1C/1C 5C/6C" )
	PORT_DIPSETTING(    0x90, DEF_STR ( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR ( 1C_2C ) )
	PORT_DIPSETTING(    0xf0, "1C/2C 5C/11C" )
	PORT_DIPSETTING(    0x20, DEF_STR ( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR ( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR ( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR ( 1C_6C ) )

	PORT_MODIFY("DSW03")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "20K 40K 60K" )
	PORT_DIPSETTING(    0x10, "30K 60K 90K" )
	PORT_DIPSETTING(    0x20, "40K 70K 100K" )
	PORT_DIPSETTING(    0x30, "40K 80K 120K" )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( razmataz )
	PORT_START("SW00")
	PORT_BIT( 0xff, 0x00, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, zaxxon_state,razmataz_dial_r, (void *)0)

	PORT_START("DIAL.0")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_RESET PORT_PLAYER(1)

	PORT_START("SW01")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW04")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x1e, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SW08")
	PORT_BIT( 0xff, 0x00, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, zaxxon_state,razmataz_dial_r, (void *)1)

	PORT_START("DIAL.1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_RESET PORT_PLAYER(2)

	PORT_START("SW0C")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x1e, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SW100")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, zaxxon_state,zaxxon_coin_r, (void *)0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, zaxxon_state,zaxxon_coin_r, (void *)1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, zaxxon_state,zaxxon_coin_r, (void *)2)

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )    PORT_CHANGED_MEMBER(DEVICE_SELF, zaxxon_state,zaxxon_coin_inserted, (void *)0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )    PORT_CHANGED_MEMBER(DEVICE_SELF, zaxxon_state,zaxxon_coin_inserted, (void *)1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, zaxxon_state,zaxxon_coin_inserted, (void *)2)

	PORT_START("SERVICESW")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_HIGH ) PORT_CHANGED_MEMBER(DEVICE_SELF, zaxxon_state,service_switch, 0)

	PORT_START("DSW02")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPSETTING(    0x01, "100000" )
	PORT_DIPSETTING(    0x02, "150000" )
	PORT_DIPSETTING(    0x03, "200000" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW03")
	PORT_DIPNAME( 0x07, 0x03, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR ( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR ( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR ( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR ( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR ( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR ( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR ( 1C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR ( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x18, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR ( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR ( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR ( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR ( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR ( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR ( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR ( 1C_4C ) )
	PORT_DIPSETTING(    0x38, DEF_STR ( 1C_5C ) )
	PORT_DIPNAME( 0x40, 0x40, "Test Flip Screen" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( ixion )
	PORT_INCLUDE(zaxxon)

	PORT_MODIFY("SW00")
	PORT_BIT( 0xff, 0x00, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, zaxxon_state,razmataz_dial_r, (void *)0)

	PORT_START("DIAL.0")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_RESET

	PORT_MODIFY("SW01")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("DSW02")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_DIPSETTING(    0x01, "60000" )
	PORT_DIPSETTING(    0x02, "80000" )
	PORT_DIPSETTING(    0x03, "100000" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x70, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easier ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x50, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x70, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_MODIFY("DSW03")
	PORT_DIPNAME( 0x07, 0x03, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR ( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR ( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR ( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR ( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR ( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR ( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR ( 1C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR ( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x18, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR ( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR ( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR ( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR ( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR ( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR ( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR ( 1C_4C ) )
	PORT_DIPSETTING(    0x38, DEF_STR ( 1C_5C ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( congo )
	PORT_INCLUDE(zaxxon)

	PORT_MODIFY("SW00")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_DIPNAME( 0x20, 0x00, "Test Back and Target" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("SW01")
	PORT_DIPNAME( 0x20, 0x00, "Test I/O and Dip SW" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("DSW02")
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout zaxxon_spritelayout =
{
	32,32,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			24*8+0, 24*8+1, 24*8+2, 24*8+3, 24*8+4, 24*8+5, 24*8+6, 24*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8,
			64*8, 65*8, 66*8, 67*8, 68*8, 69*8, 70*8, 71*8,
			96*8, 97*8, 98*8, 99*8, 100*8, 101*8, 102*8, 103*8 },
	128*8
};


static GFXDECODE_START( zaxxon )
	GFXDECODE_ENTRY( "gfx_tx", 0, gfx_8x8x2_planar,  0, 64*2 )  /* characters */
	GFXDECODE_ENTRY( "gfx_bg", 0, gfx_8x8x3_planar,  0, 32*2 )  /* background tiles */
	GFXDECODE_ENTRY( "gfx_spr", 0, zaxxon_spritelayout,  0, 32*2 )  /* sprites */
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( root, zaxxon_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/16)
	MCFG_CPU_PROGRAM_MAP(zaxxon_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", zaxxon_state,  vblank_int)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(zaxxon_state, zaxxon_sound_a_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(zaxxon_state, zaxxon_sound_b_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(zaxxon_state, zaxxon_sound_c_w))

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", zaxxon)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(zaxxon_state, zaxxon)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(zaxxon_state, screen_update_zaxxon)
	MCFG_SCREEN_PALETTE("palette")

MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( zaxxon, root )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_FRAGMENT_ADD(zaxxon_samples)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( szaxxon, zaxxon )

	/* encryption */
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( futspy, root )

	/* encryption */
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(zaxxon_state, screen_update_futspy)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_FRAGMENT_ADD(zaxxon_samples)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( razmataz, root )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ixion_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)

	MCFG_DEVICE_REMOVE("ppi8255")

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(zaxxon_state,razmataz)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(zaxxon_state, screen_update_razmataz)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SEGAUSBROM_ADD("usbsnd")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( congo, root )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(congo_map)

	MCFG_DEVICE_REMOVE("ppi8255")
	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(driver_device, soundlatch_byte_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(zaxxon_state, congo_sound_b_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(zaxxon_state, congo_sound_c_w))

	MCFG_CPU_ADD("audiocpu", Z80, SOUND_CLOCK)
	MCFG_CPU_PROGRAM_MAP(congo_sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(zaxxon_state, irq0_line_hold,  (double)SOUND_CLOCK/16/16/16/4)

	/* video hardware */
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(512)
	MCFG_PALETTE_INIT_OWNER(zaxxon_state, zaxxon)

	MCFG_VIDEO_START_OVERRIDE(zaxxon_state,congo)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(zaxxon_state, screen_update_congo)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SN76489A, SOUND_CLOCK) // schematic shows sn76489A
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("sn2", SN76489A, SOUND_CLOCK/4) // schematic shows sn76489A
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_FRAGMENT_ADD(congo_samples)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( zaxxon )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "zaxxon3.u27",  0x0000, 0x2000, CRC(6e2b4a30) SHA1(80ac53c554c84226b119cbe3cf3470bcdbcd5762) )
	ROM_LOAD( "zaxxon2.u28",  0x2000, 0x2000, CRC(1c9ea398) SHA1(0cd259be3fa80f3d53dfa76d5ca06773cdfe5945) )
	ROM_LOAD( "zaxxon1.u29",  0x4000, 0x1000, CRC(1c123ef9) SHA1(2588be06ea7baca6112d58c78a1eeb98aad8a02e) )

	ROM_REGION( 0x1000, "gfx_tx", 0 )
	ROM_LOAD( "zaxxon14.u68", 0x0000, 0x0800, CRC(07bf8c52) SHA1(425157a1625b1bd5169c3218b958010bf6af12bb) )
	ROM_LOAD( "zaxxon15.u69", 0x0800, 0x0800, CRC(c215edcb) SHA1(f1ded2173eb139f48d2ca86c5ef00acbe6c11cd3) )

	ROM_REGION( 0x6000, "gfx_bg", 0 )
	ROM_LOAD( "zaxxon6.u113", 0x0000, 0x2000, CRC(6e07bb68) SHA1(a002f3441b0f0044615ce71ecbd14edadba16270) )
	ROM_LOAD( "zaxxon5.u112", 0x2000, 0x2000, CRC(0a5bce6a) SHA1(a86543727389931244ba8a576b543d7ac05a2585) )
	ROM_LOAD( "zaxxon4.u111", 0x4000, 0x2000, CRC(a5bf1465) SHA1(a8cd27dfb4a606bae8bfddcf936e69e980fb1977) )

	ROM_REGION( 0x6000, "gfx_spr", 0 )
	ROM_LOAD( "zaxxon11.u77", 0x0000, 0x2000, CRC(eaf0dd4b) SHA1(194e2ca0a806e0cb6bb7cc8341d1fc6f2ea911f6) )
	ROM_LOAD( "zaxxon12.u78", 0x2000, 0x2000, CRC(1c5369c7) SHA1(af6a5984c3cedfa8c9efcd669f4f205b51a433b2) )
	ROM_LOAD( "zaxxon13.u79", 0x4000, 0x2000, CRC(ab4e8a9a) SHA1(4ac79cccc30e4adfa878b36101e97e20ac010438) )

	ROM_REGION( 0x8000, "tilemap_dat", 0 )
	ROM_LOAD( "zaxxon8.u91",  0x0000, 0x2000, CRC(28d65063) SHA1(e1f90716236c61df61bdc6915a8e390cb4dcbf15) )
	ROM_LOAD( "zaxxon7.u90",  0x2000, 0x2000, CRC(6284c200) SHA1(d26a9049541479b8b19f5aa0690cf4aaa787c9b5) )
	ROM_LOAD( "zaxxon10.u93", 0x4000, 0x2000, CRC(a95e61fd) SHA1(a0f8c15ff75affa3532abf8f340811cf415421fd) )
	ROM_LOAD( "zaxxon9.u92",  0x6000, 0x2000, CRC(7e42691f) SHA1(2124363be8f590b74e2b15dd3f90d77dd9ca9528) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "zaxxon.u98",   0x0000, 0x0100, CRC(6cc6695b) SHA1(01ae8450ccc302e1a5ae74230d44f6f531a962e2) )
	ROM_LOAD( "zaxxon.u72",   0x0100, 0x0100, CRC(deaa21f7) SHA1(0cf08fb62f77d93ff7cb883c633e0db35906e11d) )
ROM_END

ROM_START( zaxxon2 )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "zaxxon3a.u27", 0x0000, 0x2000, CRC(b18e428a) SHA1(d3ff077e37a3ed8a9cc32cba19e1694b79df6b30) )
	ROM_LOAD( "zaxxon2.u28",  0x2000, 0x2000, CRC(1c9ea398) SHA1(0cd259be3fa80f3d53dfa76d5ca06773cdfe5945) )
	ROM_LOAD( "zaxxon1a.u29", 0x4000, 0x1000, CRC(1977d933) SHA1(b0100a51a85928b8df3b07b27c9e7e4f929d7893) )

	ROM_REGION( 0x1000, "gfx_tx", 0 )
	ROM_LOAD( "zaxxon14.u68", 0x0000, 0x0800, CRC(07bf8c52) SHA1(425157a1625b1bd5169c3218b958010bf6af12bb) )
	ROM_LOAD( "zaxxon15.u69", 0x0800, 0x0800, CRC(c215edcb) SHA1(f1ded2173eb139f48d2ca86c5ef00acbe6c11cd3) )

	ROM_REGION( 0x6000, "gfx_bg", 0 )
	ROM_LOAD( "zaxxon6.u113", 0x0000, 0x2000, CRC(6e07bb68) SHA1(a002f3441b0f0044615ce71ecbd14edadba16270) )
	ROM_LOAD( "zaxxon5.u112", 0x2000, 0x2000, CRC(0a5bce6a) SHA1(a86543727389931244ba8a576b543d7ac05a2585) )
	ROM_LOAD( "zaxxon4.u111", 0x4000, 0x2000, CRC(a5bf1465) SHA1(a8cd27dfb4a606bae8bfddcf936e69e980fb1977) )

	ROM_REGION( 0x6000, "gfx_spr", 0 )
	ROM_LOAD( "zaxxon11.u77", 0x0000, 0x2000, CRC(eaf0dd4b) SHA1(194e2ca0a806e0cb6bb7cc8341d1fc6f2ea911f6) )
	ROM_LOAD( "zaxxon12.u78", 0x2000, 0x2000, CRC(1c5369c7) SHA1(af6a5984c3cedfa8c9efcd669f4f205b51a433b2) )
	ROM_LOAD( "zaxxon13.u79", 0x4000, 0x2000, CRC(ab4e8a9a) SHA1(4ac79cccc30e4adfa878b36101e97e20ac010438) )

	ROM_REGION( 0x8000, "tilemap_dat", 0 )
	ROM_LOAD( "zaxxon8.u91",  0x0000, 0x2000, CRC(28d65063) SHA1(e1f90716236c61df61bdc6915a8e390cb4dcbf15) )
	ROM_LOAD( "zaxxon7.u90",  0x2000, 0x2000, CRC(6284c200) SHA1(d26a9049541479b8b19f5aa0690cf4aaa787c9b5) )
	ROM_LOAD( "zaxxon10.u93", 0x4000, 0x2000, CRC(a95e61fd) SHA1(a0f8c15ff75affa3532abf8f340811cf415421fd) )
	ROM_LOAD( "zaxxon9.u92",  0x6000, 0x2000, CRC(7e42691f) SHA1(2124363be8f590b74e2b15dd3f90d77dd9ca9528) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "zaxxon.u98",   0x0000, 0x0100, CRC(6cc6695b) SHA1(01ae8450ccc302e1a5ae74230d44f6f531a962e2) )
	ROM_LOAD( "j214a2.72",    0x0100, 0x0100, CRC(a9e1fb43) SHA1(57dbcfe2438fd090c08594818549aeea6339eab2) )
ROM_END

ROM_START( zaxxon3 )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "zaxxon3_alt.u27", 0x0000, 0x2000, CRC(2f2f2b7c) SHA1(009b6f8943b35bf933c56e9538377be3e1db6a6d) )
	ROM_LOAD( "zaxxon2_alt.u28", 0x2000, 0x2000, CRC(ae7e1c38) SHA1(896f920e04c182eced3a3714b45a1d96e45b4473) )
	ROM_LOAD( "zaxxon1_alt.u29", 0x4000, 0x1000, CRC(cc67c097) SHA1(d258b2900a173b0f3f8acd94b6b636ce3324616d) )

	ROM_REGION( 0x1000, "gfx_tx", 0 )
	ROM_LOAD( "zaxxon14.u68", 0x0000, 0x0800, CRC(07bf8c52) SHA1(425157a1625b1bd5169c3218b958010bf6af12bb) )
	ROM_LOAD( "zaxxon15.u69", 0x0800, 0x0800, CRC(c215edcb) SHA1(f1ded2173eb139f48d2ca86c5ef00acbe6c11cd3) )

	ROM_REGION( 0x6000, "gfx_bg", 0 )
	ROM_LOAD( "zaxxon6.u113", 0x0000, 0x2000, CRC(6e07bb68) SHA1(a002f3441b0f0044615ce71ecbd14edadba16270) )
	ROM_LOAD( "zaxxon5.u112", 0x2000, 0x2000, CRC(0a5bce6a) SHA1(a86543727389931244ba8a576b543d7ac05a2585) )
	ROM_LOAD( "zaxxon4.u111", 0x4000, 0x2000, CRC(a5bf1465) SHA1(a8cd27dfb4a606bae8bfddcf936e69e980fb1977) )

	ROM_REGION( 0x6000, "gfx_spr", 0 )
	ROM_LOAD( "zaxxon11.u77", 0x0000, 0x2000, CRC(eaf0dd4b) SHA1(194e2ca0a806e0cb6bb7cc8341d1fc6f2ea911f6) )
	ROM_LOAD( "zaxxon12.u78", 0x2000, 0x2000, CRC(1c5369c7) SHA1(af6a5984c3cedfa8c9efcd669f4f205b51a433b2) )
	ROM_LOAD( "zaxxon13.u79", 0x4000, 0x2000, CRC(ab4e8a9a) SHA1(4ac79cccc30e4adfa878b36101e97e20ac010438) )

	ROM_REGION( 0x8000, "tilemap_dat", 0 )
	ROM_LOAD( "zaxxon8.u91",  0x0000, 0x2000, CRC(28d65063) SHA1(e1f90716236c61df61bdc6915a8e390cb4dcbf15) )
	ROM_LOAD( "zaxxon7.u90",  0x2000, 0x2000, CRC(6284c200) SHA1(d26a9049541479b8b19f5aa0690cf4aaa787c9b5) )
	ROM_LOAD( "zaxxon10.u93", 0x4000, 0x2000, CRC(a95e61fd) SHA1(a0f8c15ff75affa3532abf8f340811cf415421fd) )
	ROM_LOAD( "zaxxon9.u92",  0x6000, 0x2000, CRC(7e42691f) SHA1(2124363be8f590b74e2b15dd3f90d77dd9ca9528) )
	//ROM_LOAD( "zaxxon9_alt.u92",  0x6000, 0x2000, CRC(aef163fb) SHA1(b9e88b3a467926fac3931b891fb01b9b4add3809) ) // was in this set, but seems to have bad bits

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "zaxxon.u98",   0x0000, 0x0100, CRC(6cc6695b) SHA1(01ae8450ccc302e1a5ae74230d44f6f531a962e2) )
	ROM_LOAD( "j214a2.72",    0x0100, 0x0100, CRC(a9e1fb43) SHA1(57dbcfe2438fd090c08594818549aeea6339eab2) )
ROM_END

ROM_START( zaxxonj )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "zaxxon_rom3.u13",  0x0000, 0x2000, CRC(925168c7) SHA1(0f14b5072b5be252955ffb3a30fa6c76b3e6a6c4) )
	ROM_LOAD( "zaxxon_rom2.u12",  0x2000, 0x2000, CRC(c088df92) SHA1(c0c6cd8dcf6db65129980331fa9ecc3800b63436) )
	ROM_LOAD( "zaxxon_rom1.u11",  0x4000, 0x1000, CRC(f832dd79) SHA1(1d626f22630eaa53603637991a06aeb7401e9769) )

	ROM_REGION( 0x1000, "gfx_tx", 0 )
	ROM_LOAD( "zaxxon_rom14.u54", 0x0000, 0x0800, CRC(07bf8c52) SHA1(425157a1625b1bd5169c3218b958010bf6af12bb) )
	ROM_LOAD( "zaxxon_rom15.u55", 0x0800, 0x0800, CRC(c215edcb) SHA1(f1ded2173eb139f48d2ca86c5ef00acbe6c11cd3) )

	ROM_REGION( 0x6000, "gfx_bg", 0 )
	ROM_LOAD( "zaxxon_rom6.u70",  0x0000, 0x2000, CRC(6e07bb68) SHA1(a002f3441b0f0044615ce71ecbd14edadba16270) )
	ROM_LOAD( "zaxxon_rom5.u69",  0x2000, 0x2000, CRC(0a5bce6a) SHA1(a86543727389931244ba8a576b543d7ac05a2585) )
	ROM_LOAD( "zaxxon_rom4.u68",  0x4000, 0x2000, CRC(a5bf1465) SHA1(a8cd27dfb4a606bae8bfddcf936e69e980fb1977) )

	ROM_REGION( 0x6000, "gfx_spr", 0 )
	ROM_LOAD( "zaxxon_rom11.u59", 0x0000, 0x2000, CRC(eaf0dd4b) SHA1(194e2ca0a806e0cb6bb7cc8341d1fc6f2ea911f6) )
	ROM_LOAD( "zaxxon_rom12.u60", 0x2000, 0x2000, CRC(1c5369c7) SHA1(af6a5984c3cedfa8c9efcd669f4f205b51a433b2) )
	ROM_LOAD( "zaxxon_rom13.u61", 0x4000, 0x2000, CRC(ab4e8a9a) SHA1(4ac79cccc30e4adfa878b36101e97e20ac010438) )

	ROM_REGION( 0x8000, "tilemap_dat", 0 )
	ROM_LOAD( "zaxxon_rom8.u58",  0x0000, 0x2000, CRC(28d65063) SHA1(e1f90716236c61df61bdc6915a8e390cb4dcbf15) )
	ROM_LOAD( "zaxxon_rom7.u57",  0x2000, 0x2000, CRC(6284c200) SHA1(d26a9049541479b8b19f5aa0690cf4aaa787c9b5) )
	ROM_LOAD( "zaxxon_rom10.u60", 0x4000, 0x2000, CRC(a95e61fd) SHA1(a0f8c15ff75affa3532abf8f340811cf415421fd) )
	ROM_LOAD( "zaxxon_rom9.u59",  0x6000, 0x2000, CRC(7e42691f) SHA1(2124363be8f590b74e2b15dd3f90d77dd9ca9528) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mro_16.u76",   0x0000, 0x0100, CRC(6cc6695b) SHA1(01ae8450ccc302e1a5ae74230d44f6f531a962e2) )
	ROM_LOAD( "mro_17.u41",   0x0100, 0x0100, CRC(a9e1fb43) SHA1(57dbcfe2438fd090c08594818549aeea6339eab2) )
ROM_END

ROM_START( zaxxonb )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "zaxxonb3.u27", 0x0000, 0x2000, CRC(125bca1c) SHA1(f4160966d42e5282736cde8a276204ba8910ca61) )
	ROM_LOAD( "zaxxonb2.u28", 0x2000, 0x2000, CRC(c088df92) SHA1(c0c6cd8dcf6db65129980331fa9ecc3800b63436) )
	ROM_LOAD( "zaxxonb1.u29", 0x4000, 0x1000, CRC(e7bdc417) SHA1(209f0d259f60b984c84229bb31af1ef939adc73e) )

	ROM_REGION( 0x1000, "gfx_tx", 0 )
	ROM_LOAD( "zaxxon14.u68", 0x0000, 0x0800, CRC(07bf8c52) SHA1(425157a1625b1bd5169c3218b958010bf6af12bb) )
	ROM_LOAD( "zaxxon15.u69", 0x0800, 0x0800, CRC(c215edcb) SHA1(f1ded2173eb139f48d2ca86c5ef00acbe6c11cd3) )

	ROM_REGION( 0x6000, "gfx_bg", 0 )
	ROM_LOAD( "zaxxon6.u113", 0x0000, 0x2000, CRC(6e07bb68) SHA1(a002f3441b0f0044615ce71ecbd14edadba16270) )
	ROM_LOAD( "zaxxon5.u112", 0x2000, 0x2000, CRC(0a5bce6a) SHA1(a86543727389931244ba8a576b543d7ac05a2585) )
	ROM_LOAD( "zaxxon4.u111", 0x4000, 0x2000, CRC(a5bf1465) SHA1(a8cd27dfb4a606bae8bfddcf936e69e980fb1977) )

	ROM_REGION( 0x6000, "gfx_spr", 0 )
	ROM_LOAD( "zaxxon11.u77", 0x0000, 0x2000, CRC(eaf0dd4b) SHA1(194e2ca0a806e0cb6bb7cc8341d1fc6f2ea911f6) )
	ROM_LOAD( "zaxxon12.u78", 0x2000, 0x2000, CRC(1c5369c7) SHA1(af6a5984c3cedfa8c9efcd669f4f205b51a433b2) )
	ROM_LOAD( "zaxxon13.u79", 0x4000, 0x2000, CRC(ab4e8a9a) SHA1(4ac79cccc30e4adfa878b36101e97e20ac010438) )

	ROM_REGION( 0x8000, "tilemap_dat", 0 )
	ROM_LOAD( "zaxxon8.u91",  0x0000, 0x2000, CRC(28d65063) SHA1(e1f90716236c61df61bdc6915a8e390cb4dcbf15) )
	ROM_LOAD( "zaxxon7.u90",  0x2000, 0x2000, CRC(6284c200) SHA1(d26a9049541479b8b19f5aa0690cf4aaa787c9b5) )
	ROM_LOAD( "zaxxon10.u93", 0x4000, 0x2000, CRC(a95e61fd) SHA1(a0f8c15ff75affa3532abf8f340811cf415421fd) )
	ROM_LOAD( "zaxxon9.u92",  0x6000, 0x2000, CRC(7e42691f) SHA1(2124363be8f590b74e2b15dd3f90d77dd9ca9528) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "zaxxon.u98",   0x0000, 0x0100, CRC(6cc6695b) SHA1(01ae8450ccc302e1a5ae74230d44f6f531a962e2) )
	ROM_LOAD( "zaxxon.u72",   0x0100, 0x0100, CRC(deaa21f7) SHA1(0cf08fb62f77d93ff7cb883c633e0db35906e11d) )
ROM_END


ROM_START( szaxxon )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "1804e.u27",   0x0000, 0x2000, CRC(af7221da) SHA1(b5d3beb296d52ed69b4ceacf329c20a72e3a1dce) )
	ROM_LOAD( "1803e.u28",   0x2000, 0x2000, CRC(1b90fb2a) SHA1(afb2bd2ffee3f5e589064f59b6ac21ed915094df) )
	ROM_LOAD( "1802e.u29",   0x4000, 0x1000, CRC(07258b4a) SHA1(91e3a0c0df6c9cf66980d1ffcc3830ffdbef8c2f) )

	ROM_REGION( 0x1000, "gfx_tx", 0 )
	ROM_LOAD( "1815b.u68",   0x0000, 0x0800, CRC(bccf560c) SHA1(9f92bd15466048a5665bfc2ebc8c6504af9353eb) )
	ROM_LOAD( "1816b.u69",   0x0800, 0x0800, CRC(d28c628b) SHA1(42ab7dc0e4e0d09213054597373383cdb6a55699) )

	ROM_REGION( 0x6000, "gfx_bg", 0 )
	ROM_LOAD( "1807b.u113",  0x0000, 0x2000, CRC(f51af375) SHA1(8682217dc800f43b73cd5e8501dbf3b7cd136dc1) )
	ROM_LOAD( "1806b.u112",  0x2000, 0x2000, CRC(a7de021d) SHA1(a1bee07aa906366aa69866d1bdff38e2d90fafdd) )
	ROM_LOAD( "1805b.u111",  0x4000, 0x2000, CRC(5bfb3b04) SHA1(f898e42d6bc1fd3629c9caee3c2af27805969ac6) )

	ROM_REGION( 0x6000, "gfx_spr", 0 )
	ROM_LOAD( "1812e.u77",   0x0000, 0x2000, CRC(1503ae41) SHA1(d4085f15fcbfb9547a7f9e2cb7ce9276c4d6c08d) )
	ROM_LOAD( "1813e.u78",   0x2000, 0x2000, CRC(3b53d83f) SHA1(118e9d2b4f5daf96f5a38ccd92d0b046a470b0b2) )
	ROM_LOAD( "1814e.u79",   0x4000, 0x2000, CRC(581e8793) SHA1(2b3305dd55dc09d7394ed8ae691773972dba28b9) )

	ROM_REGION( 0x8000, "tilemap_dat", 0 )
	ROM_LOAD( "1809b.u91",   0x0000, 0x2000, CRC(dd1b52df) SHA1(8170dd9f81c41104694951a2c74405d0c6d8b9b6) )
	ROM_LOAD( "1808b.u90",   0x2000, 0x2000, CRC(b5bc07f0) SHA1(1e4d460ce8cca66b081ee8ec1a9adb6ef98274ec) )
	ROM_LOAD( "1811b.u93",   0x4000, 0x2000, CRC(68e84174) SHA1(b78c44d92078552835a20bcb7125fc9ca8af5048) )
	ROM_LOAD( "1810b.u92",   0x6000, 0x2000, CRC(a509994b) SHA1(51541ec78ab3f8241a5ddf7f99a46f5e44292992) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "pr-5168.u98", 0x0000, 0x0100, CRC(15727a9f) SHA1(42840e9ab303fb64102a1dbae03d66c9cf743a9f) ) /* TBP24S10N */
	ROM_LOAD( "pr-5167.u72", 0x0100, 0x0100, CRC(deaa21f7) SHA1(0cf08fb62f77d93ff7cb883c633e0db35906e11d) ) /* TBP28L22 */
ROM_END


ROM_START( futspy )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "fs_snd.u27",   0x0000, 0x2000, CRC(7578fe7f) SHA1(ab42bdf74b07c1ba5337c3d34647d3ee16f9db05) )
	ROM_LOAD( "fs_snd.u28",   0x2000, 0x2000, CRC(8ade203c) SHA1(f095f4019befff7af4203c886ef42357f79592a1) )
	ROM_LOAD( "fs_snd.u29",   0x4000, 0x1000, CRC(734299c3) SHA1(12acf71d9d00e0e0df29c4d8c397ad407266b364) )

	ROM_REGION( 0x1000, "gfx_tx", 0 )
	ROM_LOAD( "fs_snd.u68",   0x0000, 0x0800, CRC(305fae2d) SHA1(fbe89feff0fb2d4515000d1b73b7c91aac4e0b67) )
	ROM_LOAD( "fs_snd.u69",   0x0800, 0x0800, CRC(3c5658c0) SHA1(70ac44b9334b086cdecd73f5f7820a0bf8ae2629) )

	ROM_REGION( 0x6000, "gfx_bg", 0 )
	ROM_LOAD( "fs_vid.u113",  0x0000, 0x2000, CRC(36d2bdf6) SHA1(c27835055beedf61ba644070f8920b6008d99040) )
	ROM_LOAD( "fs_vid.u112",  0x2000, 0x2000, CRC(3740946a) SHA1(e7579dd91628a811a60a8d8a5b407728b74aa17e) )
	ROM_LOAD( "fs_vid.u111",  0x4000, 0x2000, CRC(4cd4df98) SHA1(3ae4b2d0a79069e0de81596805bcf1a9ae7912cf) )

	ROM_REGION( 0xc000, "gfx_spr", 0 )
	ROM_LOAD( "fs_vid.u77",   0x0000, 0x4000, CRC(1b93c9ec) SHA1(4b1d3b7e35d65cc3b96eb4f2e98c59e779bcb1c1) )
	ROM_LOAD( "fs_vid.u78",   0x4000, 0x4000, CRC(50e55262) SHA1(363acbde7b37a2358b3e53cfc08c9bd5dee73d55) )
	ROM_LOAD( "fs_vid.u79",   0x8000, 0x4000, CRC(bfb02e3e) SHA1(f53bcec46b8c7d26e9ab01c821a8d1578b85f786) )

	ROM_REGION( 0x8000, "tilemap_dat", 0 )
	ROM_LOAD( "fs_vid.u91",   0x0000, 0x2000, CRC(86da01f4) SHA1(954e4be1b0e24c8bc88c2b328e3a0e32005bb7b2) )
	ROM_LOAD( "fs_vid.u90",   0x2000, 0x2000, CRC(2bd41d2d) SHA1(efb74b4bce31c7868ab6438e07b02b0539d35120) )
	ROM_LOAD( "fs_vid.u93",   0x4000, 0x2000, CRC(b82b4997) SHA1(263f74aab47fc4e516b2111eaa94beea61c5fbe5) )
	ROM_LOAD( "fs_vid.u92",   0x6000, 0x2000, CRC(af4015af) SHA1(6ed01a42d395ada6f2442b68f901fe61b04c8e44) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "futrprom.u98", 0x0000, 0x0100, CRC(9ba2acaa) SHA1(20e0257ca531ddc398b3aab861c7b5c41b659d40) )
	ROM_LOAD( "futrprom.u72", 0x0100, 0x0100, CRC(f9e26790) SHA1(339f27e0126312d35211b5ce533f293b58851c1d) )
ROM_END


ROM_START( razmataz )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "u27",           0x0000, 0x2000, CRC(254f350f) SHA1(f8e84778b7ffc4da76e97992f01c742c212480cf) )
	ROM_LOAD( "u28",           0x2000, 0x2000, CRC(3a1eaa99) SHA1(d1f2a61a8548135c9754097aa468672616244710) )
	ROM_LOAD( "u29",           0x4000, 0x2000, CRC(0ee67e78) SHA1(c6c703000a4e0da8af65be53b2a6b2ef67860c30) )

	ROM_REGION( 0x1000, "usbcpu", 0 )
	ROM_LOAD( "1924.u51",      0x0000, 0x0800, CRC(a75e0011) SHA1(7d67ce2e8a2de471221b3b565a937ae1a35e1560) )
	ROM_LOAD( "1923.u50",      0x0800, 0x0800, CRC(59994a51) SHA1(57ccee24a989efe39f8ffc08aab7d72a1cdef3d1) )

	ROM_REGION( 0x1000, "gfx_tx", 0 )
	ROM_LOAD( "1921.u68",      0x0000, 0x0800, CRC(77f8ff5a) SHA1(d535109387559dd5b58dc6432a1eae6535442079) )
	ROM_LOAD( "1922.u69",      0x0800, 0x0800, CRC(cf63621e) SHA1(60452ad34f2b0e0afa0f09455d9aa84058c54fd5) )

	ROM_REGION( 0x6000, "gfx_bg", 0 )
	ROM_LOAD( "1934.u113",     0x0000, 0x2000, CRC(39bb679c) SHA1(0a384286dbfc8b35e4779119f62769b6cfc93a52) )
	ROM_LOAD( "1933.u112",     0x2000, 0x2000, CRC(1022185e) SHA1(874d796baea8ade2c642f3640ec7875a9f509a68) )
	ROM_LOAD( "1932.u111",     0x4000, 0x2000, CRC(c7a715eb) SHA1(8b04558c87c5a5f94a5bab9fbe198a0b8a84ebf4) )

	ROM_REGION( 0x6000, "gfx_spr", 0 )
	ROM_LOAD( "1925.u77",      0x0000, 0x2000, CRC(a7965437) SHA1(24ab3cc9b6d70e8cab4f0a20f84fb98682b321f5) )
	ROM_LOAD( "1926.u78",      0x2000, 0x2000, CRC(9a3af434) SHA1(0b5b1ac9cf8bee1c3830ef3baffcd7d3a05bf765) )
	ROM_LOAD( "1927.u79",      0x4000, 0x2000, CRC(0323de2b) SHA1(6f6ceafe6472d59bd0ffecb9dd2d401659157b50) )

	ROM_REGION( 0x8000, "tilemap_dat", 0 )
	ROM_LOAD( "1929.u91",      0x0000, 0x2000, CRC(55c7c757) SHA1(ad8d548eb965f343e88bad4b4ad1b5b226f21d71) )
	ROM_LOAD( "1928.u90",      0x2000, 0x2000, CRC(e58b155b) SHA1(dd6abeae66de69734b7aa5e133dbfb8f8a35578e) )
	ROM_LOAD( "1931.u93",      0x4000, 0x2000, CRC(55fe0f82) SHA1(391434b41b6235199a4f19a8873a523cbb417f70) )
	ROM_LOAD( "1930.u92",      0x6000, 0x2000, CRC(f355f105) SHA1(93067b7390c05b71020e77abdd9577b39e486d9f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "clr.u98",       0x0000, 0x0100, CRC(0fd671af) SHA1(7f26139398754dae7383c9375fca95b7970fcefb) )
	ROM_LOAD( "clr.u72",       0x0100, 0x0100, CRC(03233bc5) SHA1(30bd690da7eda4e13df90d7ee59dbf744b3541a4) )
ROM_END


ROM_START( ixion )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "1937d.u27",      0x0000, 0x2000, CRC(f447aac5) SHA1(f6ec02f20482649ba1765254e0e67a8593075092) )
	ROM_LOAD( "1938b.u28",      0x2000, 0x2000, CRC(17f48640) SHA1(d661e8ae0747c2c526360cb72e403deba7a98e71) )
	ROM_LOAD( "1955b.u29",      0x4000, 0x1000, CRC(78636ec6) SHA1(afca6418221e700749031cb3fa738907d77c1566) )

	ROM_REGION( 0x1000, "usbcpu", 0 )
	ROM_LOAD( "1944a.u51",      0x0000, 0x0800, CRC(88215098) SHA1(54bd1c71e7f10f20623e47f4e791f54ce698bc08) )
	ROM_LOAD( "1943a.u50",      0x0800, 0x0800, CRC(77e5a1f0) SHA1(00152ffb59ebac718b300fdf24314b456748ffbe) )

	ROM_REGION( 0x1000, "gfx_tx", 0 )
	ROM_LOAD( "1939a.u68",      0x0000, 0x0800, CRC(c717ddc7) SHA1(86fdef368f097a27aac6e05bf3208fcdaf7d9da7) )
	ROM_LOAD( "1940a.u69",      0x0800, 0x0800, CRC(ec4bb3ad) SHA1(8a38bc48cda59b5e76a5153d459bb2d01d6a56f3) )

	ROM_REGION( 0x6000, "gfx_bg", 0 )
	ROM_LOAD( "1952a.u113",     0x0000, 0x2000, CRC(ffb9b03d) SHA1(b7a900166a880ca4a71fec6ad02f5c0ecfc92df8) )
	ROM_LOAD( "1951a.u112",     0x2000, 0x2000, CRC(db743f1b) SHA1(a5d13d597fe999757137d96fb4bf7c7efc7a3245) )
	ROM_LOAD( "1950a.u111",     0x4000, 0x2000, CRC(c2de178a) SHA1(0347a751accb02576b9cd8b123b79018cc05268c) )

	ROM_REGION( 0x6000, "gfx_spr", 0 )
	ROM_LOAD( "1945a.u77",      0x0000, 0x2000, CRC(3a3fbfe7) SHA1(f3a503f476524f9de5b55de49009972124e58601) )
	ROM_LOAD( "1946a.u78",      0x2000, 0x2000, CRC(f2cb1b53) SHA1(8c2fb58ce7de7876c4d2f1a3d13c6a5efd06d354) )
	ROM_LOAD( "1947a.u79",      0x4000, 0x2000, CRC(d2421e92) SHA1(07000055ace2c6983c7add180904d6bc20e1bb3b) )

	ROM_REGION( 0x8000, "tilemap_dat", 0 )
	ROM_LOAD( "1948a.u91",      0x0000, 0x2000, CRC(7a7fcbbe) SHA1(76dffffcadbe446091ee98958873aa76f7b17213) )
	ROM_LOAD( "1953a.u90",      0x2000, 0x2000, CRC(6b626ea7) SHA1(7e02da0b031a42b077c173d85f15f75242e61e98) )
	ROM_LOAD( "1949a.u93",      0x4000, 0x2000, CRC(e7722d09) SHA1(c9a0fb4fac798454facd3d5dd02d2c05cfe8e3a6) )
	ROM_LOAD( "1954a.u92",      0x6000, 0x2000, CRC(a970f5ff) SHA1(0f1f8f329ceefcbd0725f8eeff1b01348f5c9374) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "1942a.u98",       0x0000, 0x0100, CRC(3a8e6f74) SHA1(c2d480f8c8a111c1e23cbf819dea807f8128208d) )
	ROM_LOAD( "1941a.u72",       0x0100, 0x0100, CRC(a5d0d97e) SHA1(2677508b44f9b7a6c6ee56e49a7b88073e80debe) )
ROM_END

/*
Extra notes about Congo Bongo:

There is known to be a version of Congo Bongo with official Sega "EPR" numbers. Sega game ID number for this set is 834-5197

3 board stack with 834-5181 stickered as 834-5207 with standard 834-5167 video board & 834-5168 sound board.
EPR-5309A.rom1 @ U35
EPR-5310A.rom2 @ U34
EPR-5311A.rom3 @ U33
EPR-5312A.rom4 @ U32

Uses the MR018 BPROM @ U68

Oddly, it seems most Congo Bongo board sets (2 & 3 board stacks) have hand written labels for the program roms. At least
all the PCB photos I found via Google as well as those included in a recent redump of both board set types.  And YES it's
actually "CONGO BONGO REV C ROM 2A" this is verified in several PCB photos.

*/

ROM_START( congo ) /* 2 board stack, Sega game ID number for this set is 834-5180 */
	ROM_REGION( 0x8000, "maincpu", 0 ) /* Located on 834-5181 PCB,  AKA Tip Top-CONTR II */
	ROM_LOAD( "congo_rev_c_rom1.u21",   0x0000, 0x2000, CRC(09355b5b) SHA1(0085ac7eb0035a88cb54cdd3dd6b2643141d39db) ) /* SUM16 written on label 044A */
	ROM_LOAD( "congo_rev_c_rom2a.u22",  0x2000, 0x2000, CRC(1c5e30ae) SHA1(7cc5420e0e7a2793a671b938c121ae4079f5b1b8) ) /* SUM16 written on label DD4D */
	ROM_LOAD( "congo_rev_c_rom3.u23",   0x4000, 0x2000, CRC(5ee1132c) SHA1(26294cd69ee43dfd29fc3642e8c04552dcdbaa49) ) /* SUM16 written on label C932 */
	ROM_LOAD( "congo_rev_c_rom4.u24",   0x6000, 0x2000, CRC(5332b9bf) SHA1(8440cc6f92918b3b467a5a0b86c9defeb0a7db0e) ) /* SUM16 written on label 5128 */

	ROM_REGION( 0x2000, "audiocpu", 0 ) /* Located on 834-5181 PCB,  AKA Tip Top-CONTR II */
	ROM_LOAD( "tip_top_rom_17.u19",  0x0000, 0x2000, CRC(5024e673) SHA1(6f846146a4e29bcdfd5bd1bc5f1211d344cd5afa) )

	ROM_REGION( 0x1000, "gfx_tx", 0 ) /* Located on 834-5181 PCB,  AKA Tip Top-CONTR II */
	ROM_LOAD( "tip_top_rom_5.u76",   0x00000, 0x1000, CRC(7bf6ba2b) SHA1(3a2bd21b0e0e55cbd737c7b075492b5e8f944150) )

	ROM_REGION( 0x6000, "gfx_bg", 0 ) /* Located on 834-5167 PCB, AKA Tip Tip-VIDEO */
	ROM_LOAD( "tip_top_rom_8.u93",   0x00000, 0x2000, CRC(db99a619) SHA1(499029197d26f9aea3ac15d66b5738ce7dea1f6c) )
	ROM_LOAD( "tip_top_rom_9.u94",   0x02000, 0x2000, CRC(93e2309e) SHA1(bd8a74332cac0cf85f319c1f35d04a4781c9d655) )
	ROM_LOAD( "tip_top_rom_10.u95",  0x04000, 0x2000, CRC(f27a9407) SHA1(d41c90c89ae28c92bf0c57927357d9b68ed7e0ef) )

	ROM_REGION( 0xc000, "gfx_spr", 0 ) /* Located on 834-5167 PCB, AKA Tip Tip-VIDEO */
	ROM_LOAD( "tip_top_rom_12.u78",  0x00000, 0x2000, CRC(15e3377a) SHA1(04a7fbfd58924359fae0ba76ed152f325f07beae) )
	ROM_LOAD( "tip_top_rom_13.u79",  0x02000, 0x2000, CRC(1d1321c8) SHA1(d12e156a24db105c5f941b7ef79f32181b616710) )
	ROM_LOAD( "tip_top_rom_11.u77",  0x04000, 0x2000, CRC(73e2709f) SHA1(14919facf08f6983c3a9baad031239a1b57c8202) )
	ROM_LOAD( "tip_top_rom_14.u104", 0x06000, 0x2000, CRC(bf9169fe) SHA1(303d68e38e9a47464f14dc5be6bff1be01b88bb6) )
	ROM_LOAD( "tip_top_rom_16.u106", 0x08000, 0x2000, CRC(cb6d5775) SHA1(b1f8ead6e6f8ad995baaeb7f8554d41ed2296fff) )
	ROM_LOAD( "tip_top_rom_15.u105", 0x0a000, 0x2000, CRC(7b15a7a4) SHA1(b1c05e60a1442e4dd56d197be8b768bcbf45e2d9) )

	ROM_REGION( 0x4000, "tilemap_dat", 0 ) /* Located on 834-5167 PCB, AKA Tip Tip-VIDEO */
	ROM_LOAD( "tip_top_rom_6.u57",   0x0000, 0x2000, CRC(d637f02b) SHA1(29127149924c5bfdeb9456d7df2a5a5d14098794) )
	ROM_LOAD( "tip_top_rom_7.u58",   0x2000, 0x2000, CRC(80927943) SHA1(4683520c241d209c6cabeaead9b363f046c30f70) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Located on 834-5181 PCB,  AKA Tip Top-CONTR II */
	ROM_LOAD( "mr019.u87",    0x0000, 0x100, CRC(b788d8ae) SHA1(9765180f3087140c75e5953409df841787558160) ) /* BPROM type is a TBP28L22 per the schematics */
	ROM_RELOAD(               0x0100, 0x100 )
ROM_END

ROM_START( congoa ) /* 3 board stack, Sega game ID number for this set is 834-5156 */
	ROM_REGION( 0x8000, "maincpu", 0 ) /* Located on 834-5166 PCB  AKA Tip Top-CONTR */
	ROM_LOAD( "congo_rev_c_rom1.u35",   0x0000, 0x2000, CRC(09355b5b) SHA1(0085ac7eb0035a88cb54cdd3dd6b2643141d39db) ) /* SUM16 written on label 044A */
	ROM_LOAD( "congo_rev_c_rom2a.u34",  0x2000, 0x2000, CRC(1c5e30ae) SHA1(7cc5420e0e7a2793a671b938c121ae4079f5b1b8) ) /* SUM16 written on label DD4D */
	ROM_LOAD( "congo_rev_c_rom3.u33",   0x4000, 0x2000, CRC(5ee1132c) SHA1(26294cd69ee43dfd29fc3642e8c04552dcdbaa49) ) /* SUM16 written on label C932 */
	ROM_LOAD( "congo_rev_c_rom4.u32",   0x6000, 0x2000, CRC(5332b9bf) SHA1(8440cc6f92918b3b467a5a0b86c9defeb0a7db0e) ) /* SUM16 written on label 5128 */

	ROM_REGION( 0x2000, "audiocpu", 0 ) /* Located on 834-5168 PCB  AKA Tip Top-SOUND */
	ROM_LOAD( "tip_top_rom_17.u11",  0x0000, 0x2000, CRC(5024e673) SHA1(6f846146a4e29bcdfd5bd1bc5f1211d344cd5afa) )

	ROM_REGION( 0x1000, "gfx_tx", 0 ) /* Located on 834-5166 PCB  AKA Tip Top-CONTR */
	ROM_LOAD( "tip_top_rom_5.u76",   0x00000, 0x1000, CRC(7bf6ba2b) SHA1(3a2bd21b0e0e55cbd737c7b075492b5e8f944150) )

	ROM_REGION( 0x6000, "gfx_bg", 0 ) /* Located on 834-5167 PCB, AKA Tip Tip-VIDEO */
	ROM_LOAD( "tip_top_rom_8.u93",   0x00000, 0x2000, CRC(db99a619) SHA1(499029197d26f9aea3ac15d66b5738ce7dea1f6c) )
	ROM_LOAD( "tip_top_rom_9.u94",   0x02000, 0x2000, CRC(93e2309e) SHA1(bd8a74332cac0cf85f319c1f35d04a4781c9d655) )
	ROM_LOAD( "tip_top_rom_10.u95",  0x04000, 0x2000, CRC(f27a9407) SHA1(d41c90c89ae28c92bf0c57927357d9b68ed7e0ef) )

	ROM_REGION( 0xc000, "gfx_spr", 0 ) /* Located on 834-5167 PCB, AKA Tip Tip-VIDEO */
	ROM_LOAD( "tip_top_rom_12.u78",  0x00000, 0x2000, CRC(15e3377a) SHA1(04a7fbfd58924359fae0ba76ed152f325f07beae) )
	ROM_LOAD( "tip_top_rom_13.u79",  0x02000, 0x2000, CRC(1d1321c8) SHA1(d12e156a24db105c5f941b7ef79f32181b616710) )
	ROM_LOAD( "tip_top_rom_11.u77",  0x04000, 0x2000, CRC(73e2709f) SHA1(14919facf08f6983c3a9baad031239a1b57c8202) )
	ROM_LOAD( "tip_top_rom_14.u104", 0x06000, 0x2000, CRC(bf9169fe) SHA1(303d68e38e9a47464f14dc5be6bff1be01b88bb6) )
	ROM_LOAD( "tip_top_rom_16.u106", 0x08000, 0x2000, CRC(cb6d5775) SHA1(b1f8ead6e6f8ad995baaeb7f8554d41ed2296fff) )
	ROM_LOAD( "tip_top_rom_15.u105", 0x0a000, 0x2000, CRC(7b15a7a4) SHA1(b1c05e60a1442e4dd56d197be8b768bcbf45e2d9) )

	ROM_REGION( 0x4000, "tilemap_dat", 0 ) /* Located on 834-5167 PCB, Tip Tip-VIDEO */
	ROM_LOAD( "tip_top_rom_6.u57",   0x0000, 0x2000, CRC(d637f02b) SHA1(29127149924c5bfdeb9456d7df2a5a5d14098794) )
	ROM_LOAD( "tip_top_rom_7.u58",   0x2000, 0x2000, CRC(80927943) SHA1(4683520c241d209c6cabeaead9b363f046c30f70) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Located on 834-5166 PCB  AKA Tip Top-CONTR */
	ROM_LOAD( "mr018.u68",    0x0000, 0x200, CRC(56b9f1ba) SHA1(32ae743087f7c2dfba6818df8e9d665d8d9a3ee7) ) /* dumped as TBP28S42, first 256 bytes match MR019 then 0xFF filled */
ROM_END

ROM_START( tiptop ) /* 3 board stack */
	ROM_REGION( 0x8000, "maincpu", 0 ) /* Located on 834-5166 PCB  AKA Tip Top-CONTR */
	ROM_LOAD( "tiptop1.u35",  0x0000, 0x2000, CRC(e19dc77b) SHA1(d3782dd55701e0f5cd426ad2771c1bd0264c366a) )
	ROM_LOAD( "tiptop2.u34",  0x2000, 0x2000, CRC(3fcd3b6e) SHA1(2898807ee36fca7fbc06616c9a070604beb782b9) )
	ROM_LOAD( "tiptop3.u33",  0x4000, 0x2000, CRC(1c94250b) SHA1(cb70a91d07b0a9c61a093f1b5d37f2e69d1345c1) )
	ROM_LOAD( "tiptop4.u32",  0x6000, 0x2000, CRC(577b501b) SHA1(5cad98a60a5241ba9467aa03fcd94c7490e6dbbb) )

	ROM_REGION( 0x2000, "audiocpu", 0 ) /* Located on 834-5168 PCB  AKA Tip Top-SOUND */
	ROM_LOAD( "tip_top_rom_17.u11",  0x0000, 0x2000, CRC(5024e673) SHA1(6f846146a4e29bcdfd5bd1bc5f1211d344cd5afa) )

	ROM_REGION( 0x1000, "gfx_tx", 0 ) /* Located on 834-5166 PCB  AKA Tip Top-CONTR */
	ROM_LOAD( "tip_top_rom_5.u76",   0x00000, 0x1000, CRC(7bf6ba2b) SHA1(3a2bd21b0e0e55cbd737c7b075492b5e8f944150) )

	ROM_REGION( 0x6000, "gfx_bg", 0 ) /* Located on 834-5167 PCB, AKA Tip Tip-VIDEO */
	ROM_LOAD( "tip_top_rom_8.u93",   0x00000, 0x2000, CRC(db99a619) SHA1(499029197d26f9aea3ac15d66b5738ce7dea1f6c) )
	ROM_LOAD( "tip_top_rom_9.u94",   0x02000, 0x2000, CRC(93e2309e) SHA1(bd8a74332cac0cf85f319c1f35d04a4781c9d655) )
	ROM_LOAD( "tip_top_rom_10.u95",  0x04000, 0x2000, CRC(f27a9407) SHA1(d41c90c89ae28c92bf0c57927357d9b68ed7e0ef) )

	ROM_REGION( 0xc000, "gfx_spr", 0 ) /* Located on 834-5167 PCB, AKA Tip Tip-VIDEO */
	ROM_LOAD( "tip_top_rom_12.u78",  0x00000, 0x2000, CRC(15e3377a) SHA1(04a7fbfd58924359fae0ba76ed152f325f07beae) )
	ROM_LOAD( "tip_top_rom_13.u79",  0x02000, 0x2000, CRC(1d1321c8) SHA1(d12e156a24db105c5f941b7ef79f32181b616710) )
	ROM_LOAD( "tip_top_rom_11.u77",  0x04000, 0x2000, CRC(73e2709f) SHA1(14919facf08f6983c3a9baad031239a1b57c8202) )
	ROM_LOAD( "tip_top_rom_14.u104", 0x06000, 0x2000, CRC(bf9169fe) SHA1(303d68e38e9a47464f14dc5be6bff1be01b88bb6) )
	ROM_LOAD( "tip_top_rom_16.u106", 0x08000, 0x2000, CRC(cb6d5775) SHA1(b1f8ead6e6f8ad995baaeb7f8554d41ed2296fff) )
	ROM_LOAD( "tip_top_rom_15.u105", 0x0a000, 0x2000, CRC(7b15a7a4) SHA1(b1c05e60a1442e4dd56d197be8b768bcbf45e2d9) )

	ROM_REGION( 0x4000, "tilemap_dat", 0 ) /* Located on 834-5167 PCB, AKA Tip Tip-VIDEO */
	ROM_LOAD( "tip_top_rom_6.u57",   0x0000, 0x2000, CRC(d637f02b) SHA1(29127149924c5bfdeb9456d7df2a5a5d14098794) )
	ROM_LOAD( "tip_top_rom_7.u58",   0x2000, 0x2000, CRC(80927943) SHA1(4683520c241d209c6cabeaead9b363f046c30f70) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Located on 834-5166 PCB  AKA Tip Top-CONTR */
	ROM_LOAD( "mr018.u68",    0x0000, 0x200, CRC(56b9f1ba) SHA1(32ae743087f7c2dfba6818df8e9d665d8d9a3ee7) ) /* dumped as TBP28S42, first 256 bytes match MR019 then 0xFF filled */
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(zaxxon_state,zaxxonj)
{
/*
    the values vary, but the translation mask is always laid out like this:

      0 1 2 3 4 5 6 7 8 9 a b c d e f
    0 A A B B A A B B C C D D C C D D
    1 A A B B A A B B C C D D C C D D
    2 E E F F E E F F G G H H G G H H
    3 E E F F E E F F G G H H G G H H
    4 A A B B A A B B C C D D C C D D
    5 A A B B A A B B C C D D C C D D
    6 E E F F E E F F G G H H G G H H
    7 E E F F E E F F G G H H G G H H
    8 H H G G H H G G F F E E F F E E
    9 H H G G H H G G F F E E F F E E
    a D D C C D D C C B B A A B B A A
    b D D C C D D C C B B A A B B A A
    c H H G G H H G G F F E E F F E E
    d H H G G H H G G F F E E F F E E
    e D D C C D D C C B B A A B B A A
    f D D C C D D C C B B A A B B A A

    (e.g. 0xc0 is XORed with H)
    therefore in the following tables we only keep track of A, B, C, D, E, F, G and H.
*/
	static const UINT8 data_xortable[2][8] =
	{
		{ 0x0a,0x0a,0x22,0x22,0xaa,0xaa,0x82,0x82 },    /* ...............0 */
		{ 0xa0,0xaa,0x28,0x22,0xa0,0xaa,0x28,0x22 },    /* ...............1 */
	};

	static const UINT8 opcode_xortable[8][8] =
	{
		{ 0x8a,0x8a,0x02,0x02,0x8a,0x8a,0x02,0x02 },    /* .......0...0...0 */
		{ 0x80,0x80,0x08,0x08,0xa8,0xa8,0x20,0x20 },    /* .......0...0...1 */
		{ 0x8a,0x8a,0x02,0x02,0x8a,0x8a,0x02,0x02 },    /* .......0...1...0 */
		{ 0x02,0x08,0x2a,0x20,0x20,0x2a,0x08,0x02 },    /* .......0...1...1 */
		{ 0x88,0x0a,0x88,0x0a,0xaa,0x28,0xaa,0x28 },    /* .......1...0...0 */
		{ 0x80,0x80,0x08,0x08,0xa8,0xa8,0x20,0x20 },    /* .......1...0...1 */
		{ 0x88,0x0a,0x88,0x0a,0xaa,0x28,0xaa,0x28 },    /* .......1...1...0 */
		{ 0x02,0x08,0x2a,0x20,0x20,0x2a,0x08,0x02 }     /* .......1...1...1 */
	};

	UINT8 *rom = memregion("maincpu")->base();

	for (int A = 0x0000; A < 0x6000; A++)
	{
		int i,j;
		UINT8 src;

		src = rom[A];

		/* pick the translation table from bit 0 of the address */
		i = A & 1;

		/* pick the offset in the table from bits 1, 3 and 5 of the source data */
		j = ((src >> 1) & 1) + (((src >> 3) & 1) << 1) + (((src >> 5) & 1) << 2);
		/* the bottom half of the translation table is the mirror image of the top */
		if (src & 0x80) j = 7 - j;

		/* decode the ROM data */
		rom[A] = src ^ data_xortable[i][j];

		/* now decode the opcodes */
		/* pick the translation table from bits 0, 4, and 8 of the address */
		i = ((A >> 0) & 1) + (((A >> 4) & 1) << 1) + (((A >> 8) & 1) << 2);
		m_decrypted_opcodes[A] = src ^ opcode_xortable[i][j];
	}
}


DRIVER_INIT_MEMBER(zaxxon_state,szaxxon)
{
	static const UINT8 convtable[32][4] =
	{
		/*       opcode                   data                     address      */
		/*  A    B    C    D         A    B    C    D                           */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x28,0x20,0xa8,0xa0 },   /* ...0...0...0...0 */
		{ 0x08,0x28,0x88,0xa8 }, { 0x88,0x80,0x08,0x00 },   /* ...0...0...0...1 */
		{ 0xa8,0x28,0xa0,0x20 }, { 0x20,0xa0,0x00,0x80 },   /* ...0...0...1...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x28,0x20,0xa8,0xa0 },   /* ...0...0...1...1 */
		{ 0x08,0x28,0x88,0xa8 }, { 0x88,0x80,0x08,0x00 },   /* ...0...1...0...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x28,0x20,0xa8,0xa0 },   /* ...0...1...0...1 */
		{ 0xa8,0x28,0xa0,0x20 }, { 0x20,0xa0,0x00,0x80 },   /* ...0...1...1...0 */
		{ 0x08,0x28,0x88,0xa8 }, { 0x88,0x80,0x08,0x00 },   /* ...0...1...1...1 */
		{ 0x08,0x28,0x88,0xa8 }, { 0x88,0x80,0x08,0x00 },   /* ...1...0...0...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x28,0x20,0xa8,0xa0 },   /* ...1...0...0...1 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x28,0x20,0xa8,0xa0 },   /* ...1...0...1...0 */
		{ 0xa8,0x28,0xa0,0x20 }, { 0x20,0xa0,0x00,0x80 },   /* ...1...0...1...1 */
		{ 0xa8,0x28,0xa0,0x20 }, { 0x20,0xa0,0x00,0x80 },   /* ...1...1...0...0 */
		{ 0xa8,0x28,0xa0,0x20 }, { 0x20,0xa0,0x00,0x80 },   /* ...1...1...0...1 */
		{ 0x08,0x28,0x88,0xa8 }, { 0x88,0x80,0x08,0x00 },   /* ...1...1...1...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x28,0x20,0xa8,0xa0 }    /* ...1...1...1...1 */
	};

	sega_decode(memregion("maincpu")->base(), m_decrypted_opcodes, 0x6000, convtable);
}


DRIVER_INIT_MEMBER(zaxxon_state,futspy)
{
	static const UINT8 convtable[32][4] =
	{
		/*       opcode                   data                     address      */
		/*  A    B    C    D         A    B    C    D                           */
		{ 0x28,0x08,0x20,0x00 }, { 0x28,0x08,0x20,0x00 },   /* ...0...0...0...0 */
		{ 0x80,0x00,0xa0,0x20 }, { 0x08,0x88,0x00,0x80 },   /* ...0...0...0...1 */
		{ 0x80,0x00,0xa0,0x20 }, { 0x08,0x88,0x00,0x80 },   /* ...0...0...1...0 */
		{ 0xa0,0x80,0x20,0x00 }, { 0x20,0x28,0xa0,0xa8 },   /* ...0...0...1...1 */
		{ 0x28,0x08,0x20,0x00 }, { 0x88,0x80,0xa8,0xa0 },   /* ...0...1...0...0 */
		{ 0x80,0x00,0xa0,0x20 }, { 0x08,0x88,0x00,0x80 },   /* ...0...1...0...1 */
		{ 0x80,0x00,0xa0,0x20 }, { 0x20,0x28,0xa0,0xa8 },   /* ...0...1...1...0 */
		{ 0x20,0x28,0xa0,0xa8 }, { 0x08,0x88,0x00,0x80 },   /* ...0...1...1...1 */
		{ 0x88,0x80,0xa8,0xa0 }, { 0x28,0x08,0x20,0x00 },   /* ...1...0...0...0 */
		{ 0x80,0x00,0xa0,0x20 }, { 0xa0,0x80,0x20,0x00 },   /* ...1...0...0...1 */
		{ 0x20,0x28,0xa0,0xa8 }, { 0x08,0x88,0x00,0x80 },   /* ...1...0...1...0 */
		{ 0x80,0x00,0xa0,0x20 }, { 0x20,0x28,0xa0,0xa8 },   /* ...1...0...1...1 */
		{ 0x88,0x80,0xa8,0xa0 }, { 0x88,0x80,0xa8,0xa0 },   /* ...1...1...0...0 */
		{ 0x80,0x00,0xa0,0x20 }, { 0x08,0x88,0x00,0x80 },   /* ...1...1...0...1 */
		{ 0x80,0x00,0xa0,0x20 }, { 0x28,0x08,0x20,0x00 },   /* ...1...1...1...0 */
		{ 0x20,0x28,0xa0,0xa8 }, { 0xa0,0x80,0x20,0x00 }    /* ...1...1...1...1 */
	};

	sega_decode(memregion("maincpu")->base(), m_decrypted_opcodes, 0x6000, convtable);
}


DRIVER_INIT_MEMBER(zaxxon_state,razmataz)
{
	// Note: same as nprinces
	static const UINT8 convtable[32][4] =
	{
		/*       opcode                   data                     address      */
		/*  A    B    C    D         A    B    C    D                           */
		{ 0x08,0x88,0x00,0x80 }, { 0xa0,0x20,0x80,0x00 },   /* ...0...0...0...0 */
		{ 0xa8,0xa0,0x28,0x20 }, { 0x88,0xa8,0x80,0xa0 },   /* ...0...0...0...1 */
		{ 0x88,0x80,0x08,0x00 }, { 0x28,0x08,0xa8,0x88 },   /* ...0...0...1...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x28,0x08,0xa8,0x88 },   /* ...0...0...1...1 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0xa0,0x20,0x80,0x00 },   /* ...0...1...0...0 */
		{ 0xa8,0xa0,0x28,0x20 }, { 0xa8,0xa0,0x28,0x20 },   /* ...0...1...0...1 */
		{ 0x88,0x80,0x08,0x00 }, { 0x88,0xa8,0x80,0xa0 },   /* ...0...1...1...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x88,0xa8,0x80,0xa0 },   /* ...0...1...1...1 */
		{ 0xa0,0x20,0x80,0x00 }, { 0xa0,0x20,0x80,0x00 },   /* ...1...0...0...0 */
		{ 0x08,0x88,0x00,0x80 }, { 0x28,0x08,0xa8,0x88 },   /* ...1...0...0...1 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x88,0x80,0x08,0x00 },   /* ...1...0...1...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x28,0x08,0xa8,0x88 },   /* ...1...0...1...1 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x88,0xa8,0x80,0xa0 },   /* ...1...1...0...0 */
		{ 0x88,0xa8,0x80,0xa0 }, { 0x88,0xa8,0x80,0xa0 },   /* ...1...1...0...1 */
		{ 0x88,0x80,0x08,0x00 }, { 0x88,0x80,0x08,0x00 },   /* ...1...1...1...0 */
		{ 0x08,0x88,0x00,0x80 }, { 0x28,0x08,0xa8,0x88 }    /* ...1...1...1...1 */
	};


	sega_decode(memregion("maincpu")->base(), m_decrypted_opcodes, 0x6000, convtable);

	address_space &pgmspace = m_maincpu->space(AS_PROGRAM);

	/* additional input ports are wired */
	pgmspace.install_read_port(0xc004, 0xc004, 0, 0x18f3, "SW04");
	pgmspace.install_read_port(0xc008, 0xc008, 0, 0x18f3, "SW08");
	pgmspace.install_read_port(0xc00c, 0xc00c, 0, 0x18f3, "SW0C");

	/* unknown behavior expected here */
	pgmspace.install_read_handler(0xc80a, 0xc80a, read8_delegate(FUNC(zaxxon_state::razmataz_counter_r),this));

	/* additional state saving */
	save_item(NAME(m_razmataz_dial_pos));
	save_item(NAME(m_razmataz_counter));
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

/* these games run on standard Zaxxon hardware */
GAME( 1982, zaxxon,   0,      zaxxon,   zaxxon, driver_device,   0,        ROT90,  "Sega",    "Zaxxon (set 1)",  GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1982, zaxxon2,  zaxxon, zaxxon,   zaxxon, driver_device,   0,        ROT90,  "Sega",    "Zaxxon (set 2)",  GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1982, zaxxon3,  zaxxon, zaxxon,   zaxxon, driver_device,   0,        ROT90,  "Sega",    "Zaxxon (set 3)",  GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1982, zaxxonj,  zaxxon, szaxxon,  zaxxon, zaxxon_state,    zaxxonj,  ROT90,  "Sega",    "Zaxxon (Japan)",  GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1982, zaxxonb,  zaxxon, zaxxon,   zaxxon, zaxxon_state,    zaxxonj,  ROT90,  "bootleg", "Jackson",         GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )

/* standard Zaxxon hardware but extra sound board plugged into 8255 PPI socket and encrypted cpu */
GAME( 1982, szaxxon,  0,      szaxxon,  szaxxon, zaxxon_state,   szaxxon,  ROT90,  "Sega",    "Super Zaxxon (315-5013)",  GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )

/* standard Zaxxon hardware? but encrypted cpu */
GAME( 1984, futspy,   0,      futspy,   futspy, zaxxon_state,    futspy,   ROT90,  "Sega",    "Future Spy (315-5061)",  GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )

/* these games run on modified Zaxxon hardware with no skewing, extra inputs, and a */
/* G-80 Universal Sound Board */
GAME( 1983, razmataz, 0,      razmataz, razmataz, zaxxon_state,  razmataz, ROT90,  "Sega",    "Razzmatazz", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1983, ixion,    0,      razmataz, ixion, zaxxon_state,     szaxxon,  ROT270, "Sega",    "Ixion (prototype)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE)

/* these games run on a slightly newer Zaxxon hardware with more ROM space and a */
/* custom sprite DMA chip */
GAME( 1983, congo,    0,      congo,    congo, driver_device,    0,        ROT90,  "Sega",    "Congo Bongo (Rev C, 2 board stack)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1983, congoa,   congo,  congo,    congo, driver_device,    0,        ROT90,  "Sega",    "Congo Bongo (Rev C, 3 board stack)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1983, tiptop,   congo,  congo,    congo, driver_device,    0,        ROT90,  "Sega",    "Tip Top (3 board stack)",     GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )

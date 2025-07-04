// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Paul Leaman
/***************************************************************************

    Cinematronics / Leland Cinemat System driver
    driver by Aaron Giles and Paul Leaman

    Games supported:
        * Cerberus
        * Mayhem 2002
        * Power Play
        * World Series: The Season
        * Alley Master
        * Up Your Alley
        * Danger Zone
        * Baseball The Season II
        * Super Baseball Double Play Home Run Derby
        * Strike Zone Baseball
        * Redline Racer
        * Quarterback
        * Viper
        * John Elway's Team Quarterback
        * All American Football
        * Ironman Stewart's Super Off-Road
        * Pigout

    Leland Ataxx-era
    Games supported:
        * Ataxx
        * World Soccer Finals
        * Danny Sullivan's Indy Heat
        * Brute Force
        * Asylum (prototype)

    Known bugs:
        * none at this time

****************************************************************************

Ivan 'Ironman' Stewart's Super Off-Road, Leland, 1989
Hardware info by Guru
---------------------

Most of these Cinematronics/Leland games use the same main board with different game-specific top boards.

Top PCB (for Off-Road)
-------

PCA # 81-18002-2
PART # 80-18002-01
|-\         /-----------------------------------|
|  \-------/        AD7524       LM324   J8     |
|                   AD7524  AD7533      LM324 J7|
|                   AD7524       J9 LM324       |
|                   AD7524              LM339   |
|                   AD7524              LM339   |
|                   AD7524                    J6|
|                                    U75 U82    |
| U9          16MHz     U44    76C88 U74 U81    |
|            80186      U43    76C88            |
| U8                    U42           ADC0820 J5|
| U7                           U59              |
| U6                   71054   U58              |
| U5     U15   U27     71054   U57            J4|
| U4     U14   U26             U56              |
| U3     U13   U25                              |
| U2     U12   U24             U55      U68   J3|
| Z80(2)76C88  76C88           U54      Z80(1)  |
|  J1               BATT       DS1221    J2     |
|-----------------------------------------------|
Notes:
      80186 - Siemens SAB80186-1-N x86 CPU. Clock input 16MHz, runs at 8MHz with an internal /2 divider.
        Z80 - Master(1) & Slave(2) Zilog Z0840006 CPU. Clock input 6.000MHz [both, 12/2]
      76C88 - Goldstar GM76C88-10 8kBx8-bit SRAM, equivalent to 6264 etc
       BATT - 3.6V AA Lithium Battery. Connected to the 2x 76C88 SRAMs just above U59. Used to hold the book-keeping data and high scores.
              Not required/essential for normal operation.
      71054 - NEC D71054 Programmable Timer. Compatible with Intel 8254
     J1, J2 - 40 pin flat cable joining to bottom board
         J3 - 10 pin connector (not used?)
         J4 - 10 pin connector (not used?)
         J5 - 12 pin connector for 3x foot pedals
         J6 - 10 pin connector for 3x steering wheels
         J7 - 10 pin connector for power output to bottom board. 5V also flows through the J1 & J2 cables.
         J8 - 10 pin connector for DC power input
         J9 - 2 pin connector for audio output from top board to bottom board
     DS1221 - Dallas DS1221 Non-Volatile Controller / Battery Management IC
      LM324 - ST Microelectronics LM324 Quad Operational Amplifier
      LM339 - Samsung LM339 Quad Differential Comparator
     AD7524 - Maxim AD7524 8-bit Multiplying D/A Converter. 6 chips: channel 0 U46 to channel 5 U51
     AD7533 - Maxim AD7533 10-bit Multiplying D/A Converter
    ADC0820 - Signetics ADC0820 8-bit A/D Converter
    U56-U59 - 27C512 program ROMs for master Z80
      U2-U8 - 27C512 program ROMs for slave Z80. U2 & U3 are empty but are used on the Track-Pak upgrade.
    U24-U27 - 27C512 program ROMs for 80186 CPU
    U12-U15 - 27C512 program ROMs for 80186 CPU
         U9 - PAL20L10 labelled '38-01'
        U42 - PAL16R6 labelled '37-01'
        U43 - PAL16R4 labelled '36-01'
        U44 - PAL16L8 labelled '35-01'
        U54 - PAL16L8 labelled '41-02'
        U55 - PLHS18P8 labelled '39-01'
        U68 - PLHS18P8 labelled '40-01'
        U74 - PAL16R6 labelled '19-01'
        U75 - PAL16R6 labelled '18-01'
        U82 - PAL16R6 labelled '21-01'
        U81 - Socket not populated


Bottom board
------------

CINEMATRONICS INC
PCB 80-10000-01 REV A
LELAND CORP 1985 MADE IN USA
PCB: 80-10000-01
ASSY: 81-12170-12
|----------------------------------------------------------|
|   P1         2148                   P2             7815  |
|              2148                     LM324    TDA2002 P3|
|                                         VOL              |
|                                 8912                  SW1|
| 6116  LS461  4464 4464        X 8910   6116              |
| 6116  LS461                            6116            P4|
|                               U85             93C46.U120 |
|        U21   4464 4464               U96                 |
|                                                        P5|
|        U19   LS461 LS461                                 |
|                                    U95          LS154    |
|        U17   U43                   U94                 P6|
|              U42                   U93                   |
|        U16         LS461                                 |
| U3                          U70  U92                     |
|          U27 U40 JP2        U69  U91  U101               |
| JP1      U26                U68  U90  JP3                |
|  U1    14.31818MHz          U67  U89   U99      12MHz    |
|----------------------------------------------------------|
Notes:
      U3 - 27C64 EPROM (first part of slave Z80 program)
  U1/U99 - 40 pin flat cable joining to top board
 U67-U70 - 27C128 EPROM (graphics). U68 & U70 not populated
 U89-U92 - 27C128 EPROM (graphics). U89 & U91 not populated
 U93-U95 - 27C256 EPROM (graphics)
 JP1/1/2 - Jumpers all set to 1-2
   LS461 - 74LS461 Octal Counter. These are said to be PALs programmed as LS461 but some appears to be actual LS461 logic chips.
   LS154 - 74LS154 4-Line to 16-Line Decoder/Demultiplexer
    6116 - 2kBx8-bit SRAM. Chips near P1 are for the slave Z80 program RAM. Chips near P2 are for the master Z80 program RAM.
    U101 - Empty socket
    4464 - 4464 64kBx4-bit DRAM (graphics RAM)
    2148 - 1kBx4-bit SRAM (color RAM)
    8910 - AY-3-8910 Programmable Sound Generator
    8912 - AY-3-8912 Programmable Sound Generator (not populated). The 8910 does not use the extra features so either this or the 8910 can be used.
       X - 2 wires connected to pins 5 & 6 of the not-populated 8912 chip and joining to J9 on the top board
   LM324 - ST Microelectronics LM324 Quad Operational Amplifier
     VOL - Volume Pot
    7815 - ST L7815 Voltage Regulator
 TDA2002 - ST TDA2002 Audio Power Amplifier
     SW1 - Reset Switch
      P1 - 10 pin connector for R,G,B and separate H & V sync outputs
      P2 - 2 pin connector for audio output to speakers
      P3 - 10 pin connector for power input from top board
      P4 - 10 pin connector for coin/service
      P5 - 10 pin connector for nitro buttons
      P6 - 10 pin connector (not used?)
     U96 - PAL16L8 labelled '01-01'
     U85 - PAL16L8 labelled '02-22'
     U16 - PAL16L8 labelled '03-01'
     U17 - PAL16L8 labelled '04-01'
     U21 - PAL20X10 labelled '05-01'
     U19 - PAL20X10 labelled '06-01'
     U43 - PAL16R8 labelled '07-01'
     U42 - PAL16R4 labelled '08-01'
     U40 - PAL16R8 labelled '09-01'
     U26 - PAL16R8 labelled '10-01'
     U27 - PAL16R8 labelled '11-01'
   HSync - 16.8822kHz \
   VSync - 65.9459Hz  / Yes really.... not a typo ;-)

****************************************************************************

    To enter service mode in most games, press 1P start and then press
    the service coin.

    For Redline Racer, hold the service coin down and reset the machine.

    For Super Offroad, press the blue nitro button (3P button 1) and then
    press the service switch (F2).

    For Pigout, press 1P start and then press the service switch (F2).

    To enter service mode in Ataxx and Brute Force, press 1P start and
    then press the service switch (F2).

    For World Soccer Finals, press the 1P button B and then press the
    service switch.

    For Indy Heat, press the red turbo button (1P button 1) and then
    press the service switch.

***************************************************************************/

#include "emu.h"
#include "leland.h"
#include "leland_a.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "speaker.h"


/* Master Clock2 is for Asylum, Ataxx, Brute Force, Danny Sullivan's Indy Heat, World Soccer Finals */
#define MASTER_CLOCK2       XTAL_28_63636MHZ
#define MASTER_CLOCK        XTAL(12'000'000)
#define VIDEO_CLOCK         XTAL(14'318'181)


/*************************************
 *
 *  Master CPU memory handlers
 *
 *************************************/

void leland_state::master_map_program(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x9fff).bankr(m_master_bankslot[0]);
	map(0xa000, 0xdfff).bankr(m_master_bankslot[1]).w(FUNC(leland_state::leland_battery_ram_w));
	map(0xe000, 0xefff).ram().share(m_mainram);
	map(0xf000, 0xf3ff).rw(FUNC(leland_state::gated_paletteram_r), FUNC(leland_state::gated_paletteram_w)).share("palette");
	map(0xf800, 0xf801).w(FUNC(leland_state::master_video_addr_w));
}

void leland_state::master_map_io(address_map &map)
{
	map.global_mask(0xff);
	map(0xf0, 0xf0).w(FUNC(leland_state::leland_master_alt_bankswitch_w));
	map(0xfd, 0xff).rw(FUNC(leland_state::master_analog_key_r), FUNC(leland_state::master_analog_key_w));
}

void redline_state::master_redline_map_io(address_map &map)
{
	map.global_mask(0xff);
	map(0xf0, 0xf0).w(FUNC(redline_state::redline_master_alt_bankswitch_w));
	map(0xf2, 0xf2).rw(m_sound, FUNC(leland_80186_sound_device::response_r), FUNC(leland_80186_sound_device::command_lo_w));
	map(0xf4, 0xf4).w(m_sound, FUNC(leland_80186_sound_device::command_hi_w));
	map(0xfd, 0xff).rw(FUNC(redline_state::master_analog_key_r), FUNC(redline_state::master_analog_key_w));
}


void ataxx_state::master_map_program_2(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x9fff).bankr(m_master_bankslot[0]);
	map(0xa000, 0xdfff).bankr(m_master_bankslot[1]).w(FUNC(ataxx_state::ataxx_battery_ram_w));
	map(0xe000, 0xf7ff).ram().share(m_mainram);
	map(0xf800, 0xffff).rw(FUNC(ataxx_state::paletteram_and_misc_r), FUNC(ataxx_state::paletteram_and_misc_w)).share("palette");
}


void ataxx_state::master_map_io_2(address_map &map)
{
	map.global_mask(0xff);
	map(0x04, 0x04).r(m_sound, FUNC(leland_80186_sound_device::response_r));
	map(0x05, 0x05).w(m_sound, FUNC(leland_80186_sound_device::command_hi_w));
	map(0x06, 0x06).w(m_sound, FUNC(leland_80186_sound_device::command_lo_w));
	map(0x0c, 0x0c).w(m_sound, FUNC(leland_80186_sound_device::ataxx_80186_control_w));
	map(0x20, 0x20).rw(FUNC(ataxx_state::eeprom_r), FUNC(ataxx_state::eeprom_w));
	map(0xd0, 0xef).rw(FUNC(ataxx_state::ataxx_mvram_port_r), FUNC(ataxx_state::ataxx_mvram_port_w));
	map(0xf0, 0xff).rw(FUNC(ataxx_state::ataxx_master_input_r), FUNC(ataxx_state::ataxx_master_output_w));
}



/*************************************
 *
 *  Slave CPU memory handlers
 *
 *************************************/

void leland_state::slave_small_map_program(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0xdfff).bankr(m_slave_bankslot);
	map(0xe000, 0xefff).ram();
	map(0xf800, 0xf801).w(FUNC(leland_state::slave_video_addr_w));
	map(0xf802, 0xf802).r(FUNC(leland_state::raster_r));
	map(0xf803, 0xf803).w(FUNC(leland_state::slave_small_banksw_w));
}


void redline_state::slave_large_map_program(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0xbfff).bankr(m_slave_bankslot);
	map(0xc000, 0xc000).w(FUNC(leland_state::slave_large_banksw_w));
	map(0xe000, 0xefff).ram();
	map(0xf800, 0xf801).w(FUNC(leland_state::slave_video_addr_w));
	map(0xf802, 0xf802).r(FUNC(leland_state::raster_r));
}


void leland_state::slave_map_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x1f).rw(FUNC(leland_state::leland_svram_port_r), FUNC(leland_state::leland_svram_port_w));
	map(0x40, 0x5f).rw(FUNC(leland_state::leland_svram_port_r), FUNC(leland_state::leland_svram_port_w));
}


void leland_state::slave_map_program(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x9fff).bankr(m_slave_bankslot);
	map(0xa000, 0xdfff).rom();
	map(0xe000, 0xefff).ram();
	map(0xfffc, 0xfffd).w(FUNC(leland_state::slave_video_addr_w));
	map(0xfffe, 0xfffe).r(FUNC(leland_state::raster_r));
	map(0xffff, 0xffff).w(FUNC(leland_state::ataxx_slave_banksw_w));
}

void leland_state::asylum_slave_map_program(address_map &map)
{
	slave_map_program(map);
	map(0xf000, 0xfffb).ram();
}

void ataxx_state::slave_map_io_2(address_map &map)
{
	map.global_mask(0xff);
	map(0x60, 0x7f).rw(FUNC(ataxx_state::ataxx_svram_port_r), FUNC(ataxx_state::ataxx_svram_port_w));
}

/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

/************************************************************************

Memory configurations:

    Redline Racer:
        FFDF7:80186 upper chip select = E03C        -> E0000-FFFFF, 128k long
        FFDF7:80186 lower chip select = 00FC        -> 00000-00FFF, 4k long
        FFDF7:80186 peripheral chip select = 013C   -> 01000, 01080, 01100, 01180, 01200, 01280, 01300
        FFDF7:80186 middle chip select = 81FC       -> 80000-C0000, 64k chunks, 256k total
        FFDF7:80186 middle P chip select = A0FC

    Quarterback, Team Quarterback, AAFB, Super Offroad, Track Pack, Pigout, Viper:
        FFDFA:80186 upper chip select = E03C        -> E0000-FFFFF, 128k long
        FFDFA:80186 peripheral chip select = 203C   -> 20000, 20080, 20100, 20180, 20200, 20280, 20300
        FFDFA:80186 middle chip select = 01FC       -> 00000-7FFFF, 128k chunks, 512k total
        FFDFA:80186 middle P chip select = C0FC

    Ataxx, Indy Heat, World Soccer Finals:
        FFD9D:80186 upper chip select = E03C        -> E0000-FFFFF, 128k long
        FFD9D:80186 peripheral chip select = 043C   -> 04000, 04080, 04100, 04180, 04200, 04280, 04300
        FFD9D:80186 middle chip select = 01FC       -> 00000-7FFFF, 128k chunks, 512k total
        FFD9D:80186 middle P chip select = C0BC

************************************************************************/


/*************************************
 *
 *  Port definitions
 *
 *************************************/

/* Helps document the input ports. */
#define IPT_SLAVEHALT   IPT_CUSTOM


static INPUT_PORTS_START( cerberus )        /* complete, verified from code */
	PORT_START("IN0")   /* 0x80 */
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("IN1")   /* 0x81 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")   /* 0x90 */
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START("IN3")   /* 0x91 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")   /* Analog joystick 1 */
	PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN1")
	PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_START("AN2")   /* Analog joystick 2 */
	PORT_START("AN3")
	PORT_START("AN4")   /* Analog joystick 3 */
	PORT_START("AN5")
INPUT_PORTS_END


static INPUT_PORTS_START( mayhem )      /* complete, verified from code */
	PORT_START("IN0")   /* 0xC0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("IN1")   /* 0xC1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")   /* 0xD0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("IN3")   /* 0xD1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")   /* Analog joystick 1 */
	PORT_START("AN1")
	PORT_START("AN2")   /* Analog joystick 2 */
	PORT_START("AN3")
	PORT_START("AN4")   /* Analog joystick 3 */
	PORT_START("AN5")
INPUT_PORTS_END


static INPUT_PORTS_START( wseries )     /* complete, verified from code */
	PORT_START("IN0")   /* 0x80 */
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Extra Base") PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Go Back") PORT_PLAYER(1)

	PORT_START("IN1")   /* 0x81 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")   /* 0x90 */
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Aim") PORT_PLAYER(1)

	PORT_START("IN3")   /* 0x91 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")   /* Analog joystick 1 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN2")   /* Analog joystick 2 */
	PORT_START("AN3")
	PORT_START("AN4")   /* Analog joystick 3 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_START("AN5")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( alleymas )        /* complete, verified from code */
	PORT_START("IN0")   /* 0xC0 */
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN1")   /* 0xC1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")   /* 0xD0 */
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
//  PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )        /* redundant inputs */
//  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )        /* redundant inputs */

	PORT_START("IN3")   /* 0xD1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")   /* Analog joystick 1 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0,224) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN2")   /* Analog joystick 2 */
	PORT_START("AN3")
	PORT_START("AN4")   /* Analog joystick 3 */
	PORT_START("AN5")
INPUT_PORTS_END


static INPUT_PORTS_START( upyoural )        /* complete, verified from code */
	PORT_START("IN0")   /* 0xC0 */
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN1")   /* 0xC1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")   /* 0xD0 */
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("IN3")   /* 0xD1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")   /* Analog joystick 1 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0,224) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN2")   /* Analog joystick 2 */
	PORT_START("AN3")
	PORT_START("AN4")   /* Analog joystick 3 */
	PORT_START("AN5")
INPUT_PORTS_END


static INPUT_PORTS_START( dangerz )     /* complete, verified from code */
	PORT_START("IN0")   /* 0x80 */
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")   /* 0x81 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")   /* 0x90 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")   /* 0x91 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")   /* Analog 1 */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN1")   /* Analog 2 */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( basebal2 )        /* complete, verified from code */
	PORT_START("IN0")   /* 0x40/C0 */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* read by strkzone, but never referenced */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Extra Base") PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Go Back") PORT_PLAYER(1)

	PORT_START("IN1")   /* 0x41/C1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")   /* 0x50/D0 */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("R Run/Steal") PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("L Run/Steal") PORT_PLAYER(1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Run/Aim") PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Run/Cutoff") PORT_PLAYER(1)

	PORT_START("IN3")   /* 0x51/D1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")   /* Analog joystick 1 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN2")   /* Analog joystick 2 */
	PORT_START("AN3")
	PORT_START("AN4")   /* Analog joystick 3 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_START("AN5")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( redline )     /* complete, verified in code */
	PORT_START("IN0")   /* 0xC0 */
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xe0, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(64) PORT_PLAYER(1)

	PORT_START("IN1")   /* 0xC1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* used, but for what purpose? */

	PORT_START("IN2")   /* 0xD0 */
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xe0, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(64) PORT_PLAYER(2)

	PORT_START("IN3")   /* 0xD1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")   /* Analog wheel 1 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN1")   /* Analog wheel 2 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( quarterb )        /* complete, verified in code */
	PORT_START("IN0")   /* 0x80 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")   /* 0x81 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")   /* 0x90 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("IN3")   /* 0x91 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")   /* Analog spring stick 1 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN1")   /* Analog spring stick 2 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN2")   /* Analog spring stick 3 */
	PORT_START("AN3")   /* Analog spring stick 4 */
	PORT_START("AN4")   /* Analog spring stick 5 */
	PORT_START("AN5")   /* Analog spring stick 6 */
INPUT_PORTS_END


static INPUT_PORTS_START( teamqb )      /* complete, verified in code */
	PORT_START("IN0")   /* 0x80 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")   /* 0x81 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")   /* 0x90 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("IN3")   /* 0x91 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")   /* Analog spring stick 1 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN1")   /* Analog spring stick 2 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN2")   /* Analog spring stick 3 */
	PORT_START("AN3")   /* Analog spring stick 4 */
	PORT_START("AN4")   /* Analog spring stick 5 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3)
	PORT_START("AN5")   /* Analog spring stick 6 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3)

	PORT_START("IN4")   /* 0x7C */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START("IN5")   /* 0x7F */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
INPUT_PORTS_END


static INPUT_PORTS_START( aafb2p )      /* complete, verified in code */
	PORT_START("IN0")   /* 0x80 */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")   /* 0x81 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")   /* 0x90 */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("IN3")   /* 0x91 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")   /* Analog spring stick 1 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN1")   /* Analog spring stick 2 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN2")   /* Analog spring stick 3 */
	PORT_START("AN3")   /* Analog spring stick 4 */
	PORT_START("AN4")   /* Analog spring stick 5 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_START("AN5")   /* Analog spring stick 6 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("IN4")   /* 0x7C */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN5")   /* 0x7F */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( offroad )     /* complete, verified from code */
	PORT_START("IN0")   /* 0xC0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* read */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* read */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* read */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")   /* 0xC1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")   /* 0xD0 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")   /* 0xD1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")   /* Analog pedal 1 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN1")   /* Analog pedal 2 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_START("AN2")   /* Analog pedal 3 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3)
	PORT_START("AN3")   /* Analog wheel 1 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN4")   /* Analog wheel 2 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_START("AN5")   /* Analog wheel 3 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3)
INPUT_PORTS_END


static INPUT_PORTS_START( offroadt2p )
	PORT_INCLUDE( offroad )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("AN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("AN2")   /* Analog pedal 2 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_MODIFY("AN4")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("AN5")   /* Analog wheel 2 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( pigout )      /* complete, verified from code */
	PORT_START("IN0")   /* 0x40 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN1")   /* 0x41 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* read, but never referenced */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")   /* 0x50 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN3")   /* 0x51 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")   /* 0x7F */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
INPUT_PORTS_END

static INPUT_PORTS_START( ataxx )
	PORT_START("IN0")       /* 0xF6 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* huh? affects trackball movement */
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("IN1")       /* 0xF7 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")       /* 0x20 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")       /* 0x00 - analog X */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN1")       /* 0x01 - analog Y */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN2")       /* 0x02 - analog X */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_START("AN3")       /* 0x03 - analog Y */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( wsf )
	PORT_START("IN0")       /* 0xF6 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)

	PORT_START("IN1")       /* 0xF7 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")       /* 0x20 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_P2")     /* 0x0D */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("P3_P4")     /* 0x0E */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)

	PORT_START("BUTTONS")   /* 0x0F */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )
INPUT_PORTS_END


static INPUT_PORTS_START( indyheat )
	PORT_START("IN0")       /* 0xF6 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(1)
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("IN1")       /* 0xF7 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")       /* 0x20 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")       /* Analog wheel 1 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN1")       /* Analog wheel 2 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_START("AN2")       /* Analog wheel 3 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3)
	PORT_START("AN3")       /* Analog pedal 1 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN4")       /* Analog pedal 2 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_START("AN5")       /* Analog pedal 3 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3)

	PORT_START("P1")        /* 0x0D */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")        /* 0x0E */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3")        /* 0x0F */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END


static INPUT_PORTS_START( brutforc )
	PORT_START("IN0")       /* 0xF6 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(1)
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )

	PORT_START("IN1")       /* 0xF7 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")       /* 0x20 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")        /* 0x0E */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")        /* 0x0D */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3")        /* 0x0F */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void leland_state::leland(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_master, MASTER_CLOCK/2);
	m_master->set_addrmap(AS_PROGRAM, &leland_state::master_map_program);
	m_master->set_addrmap(AS_IO, &leland_state::master_map_io);
	m_master->set_vblank_int("screen", FUNC(leland_state::leland_master_interrupt));

	Z80(config, m_slave, MASTER_CLOCK/2);
	m_slave->set_addrmap(AS_PROGRAM, &leland_state::slave_small_map_program);
	m_slave->set_addrmap(AS_IO, &leland_state::slave_map_io);

	EEPROM_93C46_16BIT(config, m_eeprom);
	NVRAM(config, "battery", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	leland_video(config);
	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	// only one of the AY sockets is populated
	AY8910(config, m_ay8910, 10000000/6);
	m_ay8910->set_flags(AY8910_SINGLE_OUTPUT);
	m_ay8910->set_resistors_load(1000, 0, 0);
	m_ay8910->port_a_read_callback().set(FUNC(leland_state::sound_port_r));
	m_ay8910->port_a_write_callback().set(FUNC(leland_state::sound_port_w));
	m_ay8910->add_route(ALL_OUTPUTS, "speaker", 0.25);

//  AY8912(config, m_ay8912, 10000000/6);
//  m_ay8912->set_flags(AY8910_SINGLE_OUTPUT);
//  m_ay8912->set_resistors_load(1000, 0, 0);
//  m_ay8912->port_a_read_callback().set(FUNC(leland_state::sound_port_r));
//  m_ay8912->port_a_write_callback().set(FUNC(leland_state::sound_port_w));
//  m_ay8912->add_route(ALL_OUTPUTS, "speaker", 0.25);

	DAC_8BIT_BINARY_WEIGHTED(config, m_dac[0], 0).add_route(ALL_OUTPUTS, "speaker", 0.0625); // ls374.u79 + r17-r23 (24k,12k,6.2k,3k,1.5k,750,390,180)
	DAC_8BIT_BINARY_WEIGHTED(config, m_dac[1], 0).add_route(ALL_OUTPUTS, "speaker", 0.0625); // ls374.u88 + r27-r34 (24k,12k,6.2k,3k,1.5k,750,390,180)
}


void redline_state::redline(machine_config &config)
{
	leland(config);

	/* basic machine hardware */
	m_master->set_addrmap(AS_IO, &redline_state::master_redline_map_io);

	/* sound hardware */
	REDLINE_80186(config, m_sound, 0).set_master_cpu_tag(m_master);
}


void redline_state::quarterb(machine_config &config)
{
	redline(config);

	/* sound hardware */
	LELAND_80186(config.replace(), m_sound, 0).set_master_cpu_tag(m_master);
}


void redline_state::lelandi(machine_config &config)
{
	quarterb(config);

	/* basic machine hardware */
	m_slave->set_addrmap(AS_PROGRAM, &redline_state::slave_large_map_program);
}


void ataxx_state::ataxx(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_master, 6000000);
	m_master->set_addrmap(AS_PROGRAM, &ataxx_state::master_map_program_2);
	m_master->set_addrmap(AS_IO, &ataxx_state::master_map_io_2);

	Z80(config, m_slave, 6000000);
	m_slave->set_addrmap(AS_PROGRAM, &ataxx_state::slave_map_program);
	m_slave->set_addrmap(AS_IO, &ataxx_state::slave_map_io_2);

	EEPROM_93C56_16BIT(config, m_eeprom, eeprom_serial_streaming::ENABLE);

	NVRAM(config, "battery", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	ataxx_video(config);

	/* sound hardware */
	ATAXX_80186(config, m_sound, 0).set_master_cpu_tag(m_master);
}


void ataxx_state::wsf(machine_config &config)
{
	ataxx(config);

	WSF_80186(config.replace(), m_sound, 0).set_master_cpu_tag(m_master);
}

void ataxx_state::asylum(machine_config &config)
{
	wsf(config);
	m_slave->set_addrmap(AS_PROGRAM, &ataxx_state::asylum_slave_map_program);
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( cerberus )
	ROM_REGION( 0x10000, "master", 0 )
	ROM_LOAD( "3-23u101", 0x00000, 0x02000, CRC(d78210df) SHA1(7557bc9da7d7347073cebcc080ff2040184ee77b) )
	ROM_LOAD( "3-23u102", 0x02000, 0x02000, CRC(eed121ef) SHA1(862c4fee6c4483569aec7969ce797a5c3fbae336) )
	ROM_LOAD( "3-23u103", 0x04000, 0x02000, CRC(45b82bf7) SHA1(ca239fcb96754c9e388d55eea4974824e6ce4d75) )
	ROM_LOAD( "3-23u104", 0x06000, 0x02000, CRC(e133d6bf) SHA1(7afe4883d7b072277fab8b383ad3a247c7045403) )
	ROM_LOAD( "3-23u105", 0x08000, 0x02000, CRC(a12c2c79) SHA1(1a36405a8f9bc4422f01c2bb1361061fb8d76b51) )
	ROM_LOAD( "3-23u106", 0x0a000, 0x02000, CRC(d64110d2) SHA1(3bd8cda21e848357c84f5064f38e0b9da35051db) )
	ROM_LOAD( "3-23u107", 0x0c000, 0x02000, CRC(24e41c34) SHA1(b38462593320bd004a24392e0cce7b36fe12434e) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "3-23u3",  0x00000, 0x02000, CRC(b0579138) SHA1(b79888d0c8cc4ecb015e3865df379859e02e2846) )
	ROM_LOAD( "3-23u4",  0x02000, 0x02000, CRC(ba0dc990) SHA1(836eef85e31b81a4b6f84529ecbe64167a5059dd) )
	ROM_LOAD( "3-23u5",  0x04000, 0x02000, CRC(f8d6cc5d) SHA1(5b82c722aa6a055d1955f654985b43e114792704) )
	ROM_LOAD( "3-23u6",  0x06000, 0x02000, CRC(42cdd393) SHA1(3d2a803cb90ec25af0b34de1ae549408fc0292c3) )
	ROM_LOAD( "3-23u7",  0x08000, 0x02000, CRC(c020148a) SHA1(5ed0211526f0dc04ed010b9103bb7992dc17766f) )
	ROM_LOAD( "3-23u8",  0x0a000, 0x02000, CRC(dbabdbde) SHA1(906ff8f91eaf01f0435d7ac1291af62073568d2f) )
	ROM_LOAD( "3-23u9",  0x0c000, 0x02000, CRC(eb992385) SHA1(0951d6fb5ff8508ef7184e9c26be6c20b85bad72) )

	ROM_REGION( 0x06000, "bg_gfx", 0 )
	ROM_LOAD( "3-23u93", 0x00000, 0x02000, CRC(14a1a4b0) SHA1(aad63e368a09497188f8112d1ca0ac0d0366ac61) )
	ROM_LOAD( "3-23u94", 0x02000, 0x02000, CRC(207a1709) SHA1(c7fbb80a83a5684b6b35750df68d51091e8747e4) )
	ROM_LOAD( "3-23u95", 0x04000, 0x02000, CRC(e9c86267) SHA1(c7f3a4725824da1e2793160409821017bd0bd956) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	ROM_LOAD( "3-23u70",  0x02000, 0x2000, CRC(96499983) SHA1(202c9d74fe4bbce7b93fcbb6352c35eb480d8297) )
	ROM_LOAD( "3-23_u92", 0x06000, 0x2000, CRC(497bb717) SHA1(748ac9f22d896b493cdf182ec9deb3e07e2ffb48) )
	ROM_LOAD( "3-23u69",  0x0a000, 0x2000, CRC(ebd14d9e) SHA1(8eb061d43eb60eea01b122e0b4e937bfc00146cc) )
	ROM_LOAD( "3-23u91",  0x0e000, 0x2000, CRC(b592d2e5) SHA1(bbacbd772b6fc683dfec4f13bdf9a1746f3ea1e6) )
	ROM_LOAD( "3-23u68",  0x12000, 0x2000, CRC(cfa7b8bf) SHA1(7f38f8148cddc93baedfaa28a8c72918eb5d3b98) )
	ROM_LOAD( "3-23u90",  0x16000, 0x2000, CRC(b7566f8a) SHA1(a0128b3bf4803947050a75df0607e4886f5ed931) )
	ROM_LOAD( "3-23u67",  0x1a000, 0x2000, CRC(02b079a8) SHA1(2ad76641831a391d9acefe8e42515e16dd056868) )
	ROM_LOAD( "3-23u89",  0x1e000, 0x2000, CRC(7e5e82bb) SHA1(ccbb583689d420a0b7413c0a221a3f57a5ab0e63) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-cerberus.bin", 0x0000, 0x0080, CRC(a9594f08) SHA1(02a83d60e72793667795aacefb4f04d6155428ad) )
ROM_END


ROM_START( mayhem )
	ROM_REGION( 0x28000, "master", 0 )
	ROM_LOAD( "13208.101",   0x00000, 0x04000, CRC(04306973) SHA1(83e35fa7f2b2c6c1a65ee2f76223e12234eb69ad) )
	ROM_LOAD( "13215.102",   0x10000, 0x02000, CRC(06e689ae) SHA1(1bf4ae82809eaaf06608d2015bdeceae57a345a1) )
	ROM_CONTINUE(            0x1c000, 0x02000 )
	ROM_LOAD( "13216.103",   0x12000, 0x02000, CRC(6452a82c) SHA1(8008238359fbf1c138f4fa9fce5580d63db978f2) )
	ROM_CONTINUE(            0x1e000, 0x02000 )
	ROM_LOAD( "13217.104",   0x14000, 0x02000, CRC(62f6036e) SHA1(3e88e3f4390236b0a4623678a1a6e160c30ff747) )
	ROM_CONTINUE(            0x20000, 0x02000 )
	ROM_LOAD( "13218.105",   0x16000, 0x02000, CRC(162f5eb1) SHA1(9658b8bae35ea1d55e147a5a43ec00a25e102f54) )
	ROM_CONTINUE(            0x22000, 0x02000 )
	ROM_LOAD( "13219.106",   0x18000, 0x02000, CRC(c0a74d6f) SHA1(c47ff4dc47bea79c76198a677181c92026e8c3db) )
	ROM_CONTINUE(            0x24000, 0x02000 )

	ROM_REGION( 0x28000, "slave", 0 )
	ROM_LOAD( "13207.3",  0x00000, 0x04000, CRC(be1df6aa) SHA1(72e8782a96d598580a13b2183fbdc434f68d490b) ) // DO NOT TRIM THIS ROM
	ROM_LOAD( "13209.4",  0x10000, 0x02000, CRC(39fcd7c6) SHA1(2064a7caec0753d38a39095492f705a20482eb83) )
	ROM_CONTINUE(         0x1c000, 0x02000 )
	ROM_LOAD( "13210.5",  0x12000, 0x02000, CRC(630ed136) SHA1(fc9bc18ec18a57b8d45adcab737e29512fc62d3a) )
	ROM_CONTINUE(         0x1e000, 0x02000 )
	ROM_LOAD( "13211.6",  0x14000, 0x02000, CRC(28b4aecd) SHA1(66bfcdc66efec6e8537b29382b9702f713455826) )
	ROM_CONTINUE(         0x20000, 0x02000 )
	ROM_LOAD( "13212.7",  0x16000, 0x02000, CRC(1d6b39ab) SHA1(094e3b7e2b933c5e00722f889a75e4d76569f6fb) )
	ROM_CONTINUE(         0x22000, 0x02000 )
	ROM_LOAD( "13213.8",  0x18000, 0x02000, CRC(f3b2ea05) SHA1(ee916b903ce6891e7ea98848d559362c0e0ac8d2) )
	ROM_CONTINUE(         0x24000, 0x02000 )
	ROM_LOAD( "13214.9",  0x1a000, 0x02000, CRC(96f3e8d9) SHA1(e0a663c3c9dc77f2ec10c71a9d227ec3ea765c6e) )
	ROM_CONTINUE(         0x26000, 0x02000 )

	ROM_REGION( 0x0c000, "bg_gfx", 0 )
	ROM_LOAD( "13204.93", 0x00000, 0x04000, CRC(de183518) SHA1(080cd45c2c7d81b8edd5170aa6a587ae6e7e54fb) )
	ROM_LOAD( "13205.94", 0x04000, 0x04000, CRC(c61f63ac) SHA1(c52fe331391720796556a7eab7d145fd1dacf6ed) )
	ROM_LOAD( "13206.95", 0x08000, 0x04000, CRC(8e7bd2fd) SHA1(ccd97ef604be6d4479a8a91fccecb5d71a4d82af) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	// U70 = Empty
	ROM_LOAD( "13203.92",  0x04000, 0x4000, CRC(121ed5bf) SHA1(691b09a3bad3d1fd13ec38a81a15436b8baba0a1) )
	ROM_LOAD( "13201.69",  0x08000, 0x4000, CRC(90283e29) SHA1(36b71e2df455758b139a503968b80112a65c347a) )
	// U91 = Empty
	// U68 = Empty
	// U90 = Empty
	// U67 = Empty
	ROM_LOAD( "13202.89",  0x1c000, 0x4000, CRC(c5eaa4e3) SHA1(007a526543d06b8f39e4e93da6ad19725ec6aa2d) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-mayhem.bin", 0x0000, 0x0080, CRC(52bc0453) SHA1(bfe2fdcbc7a239f7e8c77841755923c4427e7b82) )
ROM_END


ROM_START( powrplay )
	ROM_REGION( 0x28000, "master", 0 )
	ROM_LOAD( "13306.101",   0x00000, 0x02000, CRC(981fc215) SHA1(c2ae1ff12f96c713d0dc6f6503ce0ba18ac342c4) )
	ROM_LOAD( "13307.102",   0x10000, 0x02000, CRC(38a6ddfe) SHA1(a4a4372697e14584c3a6a9a8c94e5a4ee58b3ee6) )
	ROM_CONTINUE(            0x1c000, 0x02000 )
	ROM_LOAD( "13308.103",   0x12000, 0x02000, CRC(7fa2ab9e) SHA1(d774f3a32d799f845805e88e21e1687aa35a390e) )
	ROM_CONTINUE(            0x1e000, 0x02000 )
	ROM_LOAD( "13309.104",   0x14000, 0x02000, CRC(bd9e6fa8) SHA1(4530d449d9e1cee0e346f8915e3b727b396a399d) )
	ROM_CONTINUE(            0x20000, 0x02000 )
	ROM_LOAD( "13310.105",   0x16000, 0x02000, CRC(b6df3a5a) SHA1(b968c47ecceb8be7f3b21f1f35f1a13840821f32) )
	ROM_CONTINUE(            0x22000, 0x02000 )
	ROM_LOAD( "13311.106",   0x18000, 0x02000, CRC(5e17fe84) SHA1(8f53de9acc08f17dd2bc5a81489d8da86ad3c690) )
	ROM_CONTINUE(            0x24000, 0x02000 )

	ROM_REGION( 0x28000, "slave", 0 )
	ROM_LOAD( "13305.003",  0x00000, 0x02000, CRC(df8fbeed) SHA1(2b5ec692cf90fe66d06a2261d9d56cb88528750d) )
	ROM_LOAD( "13313.004",  0x10000, 0x02000, CRC(081eb88f) SHA1(97700d9dba05a459fb85911db8f4b4fe1283776b) )
	ROM_CONTINUE(           0x1c000, 0x02000 )
	ROM_LOAD( "13314.005",  0x12000, 0x02000, CRC(b8e61f8c) SHA1(0ae3439510ad8a15f9f9c1981b2278aa950cc0b4) )
	ROM_CONTINUE(           0x1e000, 0x02000 )
	ROM_LOAD( "13315.006",  0x14000, 0x02000, CRC(776d3c40) SHA1(7fc68f16dc148c860c1ae12fb8e12d3adbe3d7c1) )
	ROM_CONTINUE(           0x20000, 0x02000 )
	ROM_LOAD( "13316.007",  0x16000, 0x02000, CRC(9b3ec2a1) SHA1(a8cc461124c93019310a0cd6de5faf83f13060d6) )
	ROM_CONTINUE(           0x22000, 0x02000 )
	ROM_LOAD( "13317.008",  0x18000, 0x02000, CRC(a081a031) SHA1(c7eef2022bc623bb3399895e092d6cb56c50b5e3) )
	ROM_CONTINUE(           0x24000, 0x02000 )

	ROM_REGION( 0x06000, "bg_gfx", 0 )
	ROM_LOAD( "13302.093", 0x00000, 0x02000, CRC(9beaa403) SHA1(02af1fb98f61b3e7758524978deba094224c8a5d) )
	ROM_LOAD( "13303.094", 0x02000, 0x02000, CRC(2bf711d0) SHA1(bf20177e1b07b12b4ef833072b313a2917d1b65e) )
	ROM_LOAD( "13304.095", 0x04000, 0x02000, CRC(06b8675b) SHA1(8b25a473c03f8210f5d8542c0dc6643c499a0afa) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	ROM_LOAD( "13301.070", 0x00000, 0x2000, CRC(aa6d3b9d) SHA1(cb1f148495b029b73f5a32c5162dcc54c0387b4e) )
	// U92 = Empty
	// U69 = Empty
	// U91 = Empty
	// U68 = Empty
	// U90 = Empty
	// U67 = Empty
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-powrplay.bin", 0x0000, 0x0080, CRC(0a999b1e) SHA1(7dbfed04707edea5af68dc9c16b0bfaef570ab39) )
ROM_END


/*
For World Series: The Season, the label format is:
------------------------
|(C)1985 Cinemtaronics | -> Copyright & Manufacturer
|P/N 02-13411-00       | -> Part number with revision
|WORLD SERIES    U103  | -> Game name & ROM PCB location
------------------------
*/
ROM_START( wseries )
	ROM_REGION( 0x28000, "master", 0 )
	ROM_LOAD( "02-13409-01.101",   0x00000, 0x02000, CRC(b5eccf5c) SHA1(1ca781245292399d1b573e6be2edbb79daf9b5d6) )
	ROM_LOAD( "02-13410-01.102",   0x10000, 0x02000, CRC(dd1ec091) SHA1(ef644c49bbe1cc30ecafab928a0715ea3461a1bd) )
	ROM_CONTINUE(                  0x1c000, 0x02000 )
	ROM_LOAD( "02-13411-01.103",   0x12000, 0x02000, CRC(ec867a0e) SHA1(7b0e390e234056fcc8e6ae9605d633b6ed874e32) )
	ROM_CONTINUE(                  0x1e000, 0x02000 )
	ROM_LOAD( "02-13412-01.104",   0x14000, 0x02000, CRC(2977956d) SHA1(24c8317f10710a5ae4d4e43bc1321a815e47c78f) )
	ROM_CONTINUE(                  0x20000, 0x02000 )
	ROM_LOAD( "02-13413-01.105",   0x16000, 0x02000, CRC(569468a6) SHA1(311257c3b7575cbf442c3afbb42ae3603c03807a) )
	ROM_CONTINUE(                  0x22000, 0x02000 )
	ROM_LOAD( "02-13414-01.106",   0x18000, 0x02000, CRC(b178632d) SHA1(c764e9e69bbd9fd9eb8e950abfd869b8bef71325) )
	ROM_CONTINUE(                  0x24000, 0x02000 )
	ROM_LOAD( "02-13415-01.107",   0x1a000, 0x02000, CRC(20b92eff) SHA1(02156fb36cae6c47b6ae9afcbc27f8f5e9074bbe) )
	ROM_CONTINUE(                  0x26000, 0x02000 )

	ROM_REGION( 0x28000, "slave", 0 )
	ROM_LOAD( "02-13416-00.u3",  0x00000, 0x02000, CRC(37c960cf) SHA1(e18c72cdbd642e8dfa1184814b65770535a469cb) )
	ROM_LOAD( "02-13417-00.u4",  0x10000, 0x02000, CRC(97f044b5) SHA1(289a9e19ce46dd039c7edc4d78bd07c355da6dad) )
	ROM_CONTINUE(                0x1c000, 0x02000 )
	ROM_LOAD( "02-13418-00.u5",  0x12000, 0x02000, CRC(0931cfc0) SHA1(13adb7caf6b1dcf3918277352545fe03e27da3c1) )
	ROM_CONTINUE(                0x1e000, 0x02000 )
	ROM_LOAD( "02-13419-00.u6",  0x14000, 0x02000, CRC(a7962b5a) SHA1(857c05395b8a1d4aeb3cbac394b673d3bc551b7f) )
	ROM_CONTINUE(                0x20000, 0x02000 )
	ROM_LOAD( "02-13420-00.u7",  0x16000, 0x02000, CRC(3c275262) SHA1(3a352c184ef3ab87bc7f926eb1af2bef7befcfb6) )
	ROM_CONTINUE(                0x22000, 0x02000 )
	ROM_LOAD( "02-13421-00.u8",  0x18000, 0x02000, CRC(86f57c80) SHA1(460fb2e1d432840797edafcf4643e23072006c2e) )
	ROM_CONTINUE(                0x24000, 0x02000 )
	ROM_LOAD( "02-13422-00.u9",  0x1a000, 0x02000, CRC(222e8405) SHA1(a1cc700e06df43847b635858d21ff2e45d8e00ab) )
	ROM_CONTINUE(                0x26000, 0x02000 )

	ROM_REGION( 0x0c000, "bg_gfx", 0 )
	ROM_LOAD( "02-13401-00.u93", 0x00000, 0x04000, CRC(4ea3e641) SHA1(7628fbf25b5e36d06818d2f9cdc66e2fb15cba4f) )
	ROM_LOAD( "02-13402-00.u94", 0x04000, 0x04000, CRC(71a8a56c) SHA1(b793a9641dd5d4cd122fb8f5cf1eef5dc3fd475c) )
	ROM_LOAD( "02-13403-00.u95", 0x08000, 0x04000, CRC(8077ae25) SHA1(15bb1f99e8aea67b9057ef5ef8570f33470a24a3) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	// U70 = Empty
	ROM_LOAD( "02-13404-00.u92",  0x04000, 0x4000, CRC(22da40aa) SHA1(a0306f795f1300d9ab88356ab44117764f6f22a4) )
	ROM_LOAD( "02-13405-00.u69",  0x08000, 0x4000, CRC(6f65b313) SHA1(2ae85686f679eaa8be15f0cd7d5af61af966c4bd) )
	// U91 = Empty
	ROM_LOAD( "02-13406-00.u68",  0x12000, 0x2000, CRC(bb568693) SHA1(f7f3af505ba5caa330a36cde77b1c2c3cbf83398) )
	ROM_LOAD( "02-13407-00.u90",  0x14000, 0x4000, CRC(e46ca57f) SHA1(771b43c4a2bcedc6a5bdde14a3c04701032b5713) )
	ROM_LOAD( "02-13408-00.u67",  0x18000, 0x4000, CRC(be637305) SHA1(a13cbc1644dc06ec52faa0a18340b679c03dc902) )
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-wseries.bin", 0x0000, 0x0080, CRC(c9c45105) SHA1(8a27752663c4096174c22146092100fbae23a2f7) )
ROM_END


ROM_START( wseries0 )
	ROM_REGION( 0x28000, "master", 0 )
	ROM_LOAD( "02-13409-00.101",   0x00000, 0x02000, CRC(a0820663) SHA1(5875248f52990496abfba89b65f6f4b76e00d0ff) )
	ROM_LOAD( "02-13410-00.102",   0x10000, 0x02000, CRC(b2d71d2d) SHA1(981876058b44c11406975f04e5aab246cb9bc175) )
	ROM_CONTINUE(                  0x1c000, 0x02000 )
	ROM_LOAD( "02-13411-00.103",   0x12000, 0x02000, CRC(50c29473) SHA1(c15c863a2e8c55c2989ab740f0aba96a4ae69585) )
	ROM_CONTINUE(                  0x1e000, 0x02000 )
	ROM_LOAD( "02-13412-00.104",   0x14000, 0x02000, CRC(2e9294e9) SHA1(76c603dd827040e800f9c2bcca46cfc170e0ad3f) )
	ROM_CONTINUE(                  0x20000, 0x02000 )
	ROM_LOAD( "02-13413-00.105",   0x16000, 0x02000, CRC(2d785d3b) SHA1(687a8c79829cfb2fd48335636e0e15e97ed30cd6) )
	ROM_CONTINUE(                  0x22000, 0x02000 )
	ROM_LOAD( "02-13414-00.106",   0x18000, 0x02000, CRC(e6684f39) SHA1(676f19fa6d9bb651b75b40149a8ce56588152f09) )
	ROM_CONTINUE(                  0x24000, 0x02000 )
	ROM_LOAD( "02-13415-00.107",   0x1a000, 0x02000, CRC(4c4db073) SHA1(3492d8920998c3bad3e9769d5731fc81ca3714a7) )
	ROM_CONTINUE(                  0x26000, 0x02000 )

	ROM_REGION( 0x28000, "slave", 0 )
	ROM_LOAD( "02-13416-00.u3",  0x00000, 0x02000, CRC(37c960cf) SHA1(e18c72cdbd642e8dfa1184814b65770535a469cb) )
	ROM_LOAD( "02-13417-00.u4",  0x10000, 0x02000, CRC(97f044b5) SHA1(289a9e19ce46dd039c7edc4d78bd07c355da6dad) )
	ROM_CONTINUE(                0x1c000, 0x02000 )
	ROM_LOAD( "02-13418-00.u5",  0x12000, 0x02000, CRC(0931cfc0) SHA1(13adb7caf6b1dcf3918277352545fe03e27da3c1) )
	ROM_CONTINUE(                0x1e000, 0x02000 )
	ROM_LOAD( "02-13419-00.u6",  0x14000, 0x02000, CRC(a7962b5a) SHA1(857c05395b8a1d4aeb3cbac394b673d3bc551b7f) )
	ROM_CONTINUE(                0x20000, 0x02000 )
	ROM_LOAD( "02-13420-00.u7",  0x16000, 0x02000, CRC(3c275262) SHA1(3a352c184ef3ab87bc7f926eb1af2bef7befcfb6) )
	ROM_CONTINUE(                0x22000, 0x02000 )
	ROM_LOAD( "02-13421-00.u8",  0x18000, 0x02000, CRC(86f57c80) SHA1(460fb2e1d432840797edafcf4643e23072006c2e) )
	ROM_CONTINUE(                0x24000, 0x02000 )
	ROM_LOAD( "02-13422-00.u9",  0x1a000, 0x02000, CRC(222e8405) SHA1(a1cc700e06df43847b635858d21ff2e45d8e00ab) )
	ROM_CONTINUE(                0x26000, 0x02000 )

	ROM_REGION( 0x0c000, "bg_gfx", 0 )
	ROM_LOAD( "02-13401-00.u93", 0x00000, 0x04000, CRC(4ea3e641) SHA1(7628fbf25b5e36d06818d2f9cdc66e2fb15cba4f) )
	ROM_LOAD( "02-13402-00.u94", 0x04000, 0x04000, CRC(71a8a56c) SHA1(b793a9641dd5d4cd122fb8f5cf1eef5dc3fd475c) )
	ROM_LOAD( "02-13403-00.u95", 0x08000, 0x04000, CRC(8077ae25) SHA1(15bb1f99e8aea67b9057ef5ef8570f33470a24a3) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	// U70 = Empty
	ROM_LOAD( "02-13404-00.u92",  0x04000, 0x4000, CRC(22da40aa) SHA1(a0306f795f1300d9ab88356ab44117764f6f22a4) )
	ROM_LOAD( "02-13405-00.u69",  0x08000, 0x4000, CRC(6f65b313) SHA1(2ae85686f679eaa8be15f0cd7d5af61af966c4bd) )
	// U91 = Empty
	ROM_LOAD( "02-13406-00.u68",  0x12000, 0x2000, CRC(bb568693) SHA1(f7f3af505ba5caa330a36cde77b1c2c3cbf83398) )
	ROM_LOAD( "02-13407-00.u90",  0x14000, 0x4000, CRC(e46ca57f) SHA1(771b43c4a2bcedc6a5bdde14a3c04701032b5713) )
	ROM_LOAD( "02-13408-00.u67",  0x18000, 0x4000, CRC(be637305) SHA1(a13cbc1644dc06ec52faa0a18340b679c03dc902) )
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-wseries.bin", 0x0000, 0x0080, CRC(c9c45105) SHA1(8a27752663c4096174c22146092100fbae23a2f7) )
ROM_END


/*
For Alley Master, the label format is:
------------------------
|(C)1985 Cinematronics | -> Copyright & Manufacturer
|P/N 02-13518-00       | -> Part number with revision
|ALLEY MASTER    U005  | -> Game name & ROM PCB location
------------------------
*/
ROM_START( alleymas )
	ROM_REGION( 0x28000, "master", 0 )
	ROM_LOAD( "02-13509-00.u101",  0x00000, 0x02000, CRC(4273e260) SHA1(1b2a726e0a6fe6a60d447c987471a6e1a9e78479) )
	ROM_LOAD( "02-13510-00.u102",  0x10000, 0x02000, CRC(eb6575aa) SHA1(0876c83d13565937610b5af52aacee1ae6fd59ba) )
	ROM_CONTINUE(                  0x1c000, 0x02000 )
	ROM_LOAD( "02-13511-00.u103",  0x12000, 0x02000, CRC(cc9d778c) SHA1(293ac75d45be4531af1982c0b99597d18bab6a05) )
	ROM_CONTINUE(                  0x1e000, 0x02000 )
	ROM_LOAD( "02-13512-00.u104",  0x14000, 0x02000, CRC(8edb129b) SHA1(f1268617cf18c1c3fd5fb324e882db14cced3d8c) )
	ROM_CONTINUE(                  0x20000, 0x02000 )
	ROM_LOAD( "02-13513-00.u105",  0x16000, 0x02000, CRC(a342dc8e) SHA1(9a6657d66fba5cb1ae3d11e940467b85d47472ea) )
	ROM_CONTINUE(                  0x22000, 0x02000 )
	ROM_LOAD( "02-13514-00.u106",  0x18000, 0x02000, CRC(b396c254) SHA1(06b118ae07d3018209b7ae831f7667cc23d23abd) )
	ROM_CONTINUE(                  0x24000, 0x02000 )
	ROM_LOAD( "02-13515-00.u107",  0x1a000, 0x02000, CRC(3ca13e8c) SHA1(34e00a17ce305c8327674bd79347f01cda14bc8b) )
	ROM_CONTINUE(                  0x26000, 0x02000 )

	ROM_REGION( 0x28000, "slave", 0 )
	ROM_LOAD( "02-13516-00.u003",  0x00000, 0x02000, CRC(3fee63ae) SHA1(519fe4981dc2c6d025fc2f27af6682103c99dd5e) )
	ROM_LOAD( "02-13517-00.u004",  0x10000, 0x02000, CRC(d302b5d1) SHA1(77263944d7b4e335fbc3b91d69def6cc85648ec6) )
	ROM_CONTINUE(                  0x1c000, 0x02000 )
	ROM_LOAD( "02-13518-00.u005",  0x12000, 0x02000, CRC(79bdb24d) SHA1(f64c3c5a715d5f4a27e01aeb31e1c43f1f3d5b17) )
	ROM_CONTINUE(                  0x1e000, 0x02000 )
	ROM_LOAD( "02-13519-00.u006",  0x14000, 0x02000, CRC(f0b15d68) SHA1(8017fce4b30e2f3bee74fba82d2a0154b3a1ab6d) )
	ROM_CONTINUE(                  0x20000, 0x02000 )
	ROM_LOAD( "02-13520-00.u007",  0x16000, 0x02000, CRC(6974036c) SHA1(222dd4d8c6d69f6b44b76681a508ff2cfafe1acc) )
	ROM_CONTINUE(                  0x22000, 0x02000 )
	ROM_LOAD( "02-13521-00.u008",  0x18000, 0x02000, CRC(a4357b5a) SHA1(c58505e1ef66641f4da5f29edbb197c5a09a367b) )
	ROM_CONTINUE(                  0x24000, 0x02000 )
	ROM_LOAD( "02-13522-00.u009",  0x1a000, 0x02000, CRC(6d74274e) SHA1(10bb04243eabeb8178884b4e0691c5e1765a1dc4) )
	ROM_CONTINUE(                  0x26000, 0x02000 )

	ROM_REGION( 0x06000, "bg_gfx", 0 )
	ROM_LOAD( "02-13501-00.u093", 0x00000, 0x02000, CRC(54456e6f) SHA1(be41711f57b5b9bd6651399f0df00c538ca1a3a5) )
	ROM_LOAD( "02-13502-00.u094", 0x02000, 0x02000, CRC(edc240da) SHA1(a812ab0cccb20cd68e9dbe283d4aab92f540af24) )
	ROM_LOAD( "02-13503-00.u095", 0x04000, 0x02000, CRC(19793ed0) SHA1(2a3cb81726977b29c88d47c90d6e15a7e287c836) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	// U70 = Empty
	ROM_LOAD( "02-13504-00.u092",  0x04000, 0x2000, CRC(a020eab5) SHA1(2f4f51f0eff8a042bf23d5f3ff42166db56e7822) )
	ROM_LOAD( "02-13505-00.u069",  0x08000, 0x2000, CRC(79abb979) SHA1(dfff8ea4d13dd0db2836e75b6b57f5f3ddac0201) )
	// U91 = Empty
	ROM_LOAD( "02-13506-00.u068",  0x10000, 0x2000, CRC(0c583385) SHA1(4bf5648991441470c4427c88ce17265b447d30d0) )
	ROM_LOAD( "02-13507-00.u090",  0x14000, 0x2000, CRC(0e1769e3) SHA1(7ca5e3205e790d90e0a39dc88766c582f25147b7) )
	// U67 = Empty
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-alleymas.bin", 0x0000, 0x0080, CRC(8c622ba1) SHA1(68728af8a19a9716af9b89af3a558c62e50867f5) )
ROM_END


ROM_START( upyoural )
	ROM_REGION( 0x28000, "master", 0 )
	ROM_LOAD( "uya-u101.bin", 0x00000, 0x02000, CRC(82bf3b7a) SHA1(1a23da0535c736fa2f49a83fe5e33b8d60117bd1) )
	ROM_LOAD( "uya-u102.bin", 0x10000, 0x02000, CRC(e1681268) SHA1(205519bf59e3be1ec485de7d81e3c4751e6630f6) )
	ROM_CONTINUE(             0x1c000, 0x02000 )
	ROM_LOAD( "uya-u103.bin", 0x12000, 0x02000, CRC(0d36aa78) SHA1(77241adf02e65e5ff85dcc4a2f70411a637eed54) )
	ROM_CONTINUE(             0x1e000, 0x02000 )
	ROM_LOAD( "uya-u104.bin", 0x14000, 0x02000, CRC(a4473886) SHA1(af63f8c0e96b3f9ce58469948a0ebdb6a853f5c4) )
	ROM_CONTINUE(             0x20000, 0x02000 )
	ROM_LOAD( "uya-u105.bin", 0x16000, 0x02000, CRC(4cad86a4) SHA1(f330800ca276aec7301a76c7c974736ee25290ce) )
	ROM_CONTINUE(             0x22000, 0x02000 )
	ROM_LOAD( "uya-u106.bin", 0x18000, 0x02000, CRC(26f4848e) SHA1(b76179b89483a19ca3ca16e0b56af3667c252d8b) )
	ROM_CONTINUE(             0x24000, 0x02000 )
	ROM_LOAD( "uya-u107.bin", 0x1a000, 0x02000, CRC(fd087cc7) SHA1(96b70e94d96baf4ea66d98b225cf67bf69cdd972) )
	ROM_CONTINUE(             0x26000, 0x02000 )

	ROM_REGION( 0x28000, "slave", 0 )
	ROM_LOAD( "uya-u3.bin",  0x00000, 0x02000, CRC(3fee63ae) SHA1(519fe4981dc2c6d025fc2f27af6682103c99dd5e) )
	ROM_LOAD( "uya-u4.bin",  0x10000, 0x02000, CRC(d302b5d1) SHA1(77263944d7b4e335fbc3b91d69def6cc85648ec6) )
	ROM_CONTINUE(            0x1c000, 0x02000 )
	ROM_LOAD( "uya-u5.bin",  0x12000, 0x02000, CRC(79bdb24d) SHA1(f64c3c5a715d5f4a27e01aeb31e1c43f1f3d5b17) )
	ROM_CONTINUE(            0x1e000, 0x02000 )
	ROM_LOAD( "uya-u6.bin",  0x14000, 0x02000, CRC(f0b15d68) SHA1(8017fce4b30e2f3bee74fba82d2a0154b3a1ab6d) )
	ROM_CONTINUE(            0x20000, 0x02000 )
	ROM_LOAD( "uya-u7.bin",  0x16000, 0x02000, CRC(6974036c) SHA1(222dd4d8c6d69f6b44b76681a508ff2cfafe1acc) )
	ROM_CONTINUE(            0x22000, 0x02000 )
	ROM_LOAD( "uya-u8.bin",  0x18000, 0x02000, CRC(a4357b5a) SHA1(c58505e1ef66641f4da5f29edbb197c5a09a367b) )
	ROM_CONTINUE(            0x24000, 0x02000 )
	ROM_LOAD( "uya-u9.bin",  0x1a000, 0x02000, CRC(6d74274e) SHA1(10bb04243eabeb8178884b4e0691c5e1765a1dc4) )
	ROM_CONTINUE(            0x26000, 0x02000 )

	ROM_REGION( 0x0c000, "bg_gfx", 0 )
	ROM_LOAD( "uya-u93.bin", 0x00000, 0x04000, CRC(e8addd70) SHA1(6fd6a09fbdbe866440c3205b103e4bede7e4b2d5) )
	ROM_LOAD( "uya-u94.bin", 0x04000, 0x04000, CRC(3fd3be09) SHA1(abdafbf9472fe3320be1a3effd13407dadf66709) )
	ROM_LOAD( "uya-u95.bin", 0x08000, 0x04000, CRC(37088dd1) SHA1(35e4a3b338baceae2e4b8ac6d95691af49ebc3c1) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	// U70 = Empty
	ROM_LOAD( "uya-u92.bin",  0x04000, 0x2000, CRC(a020eab5) SHA1(2f4f51f0eff8a042bf23d5f3ff42166db56e7822) )
	ROM_LOAD( "uya-u69.bin",  0x08000, 0x2000, CRC(79abb979) SHA1(dfff8ea4d13dd0db2836e75b6b57f5f3ddac0201) )
	// U91 = Empty
	ROM_LOAD( "uya-u68.bin",  0x10000, 0x2000, CRC(0c583385) SHA1(4bf5648991441470c4427c88ce17265b447d30d0) )
	ROM_LOAD( "uya-u90.bin",  0x14000, 0x2000, CRC(0e1769e3) SHA1(7ca5e3205e790d90e0a39dc88766c582f25147b7) )
	ROM_LOAD( "uya-u67.bin",  0x18000, 0x4000, CRC(d30a385d) SHA1(a1e83d360acef6087c24235c5a56457d25ccd937) )
	ROM_LOAD( "uya-u89.bin",  0x1c000, 0x4000, CRC(5c17401e) SHA1(2759b1d336ee43116cc4e34db36bd9c56762cca9) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-upyoural.bin", 0x0000, 0x0080, CRC(8c622ba1) SHA1(68728af8a19a9716af9b89af3a558c62e50867f5) )
ROM_END


/*
For Danger Zone, the label format is:
------------------------
|(C)1986 Cinematronics | -> Copyright & Manufacturer
|P/N 03-13823-00       | -> Part number with revision
|DANGER ZONE      U012 | -> Game name & ROM PCB location
------------------------

NOTE: Only top board program ROMs were "03-" all other part numbers were "02-"

For Danger Zone revision 2, only 13823 changed: it's label format is:
------------------------
|(C)1987Leland Corp.   | -> Copyright & Manufacturer
|P/N 03-13823-02       | -> Part number with revision
|D-ZONE chk=0208  U012t| -> Game name & ROM PCB location
------------------------
*/
ROM_START( dangerz )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "03-13823-02.u012t",  0x00000, 0x10000, CRC(31604634) SHA1(0b3d4fea91faf41519888954a21a82827eae6e2a) ) // This ROM's checksum is 0208
	ROM_LOAD( "03-13824-00.u013",   0x10000, 0x10000, CRC(381026c6) SHA1(16c810d162789154e3b5ad38545855370f73b679) ) // 3 PCBs verified as U013 and not "U013T"

	ROM_REGION( 0x28000, "slave", 0 )
	ROM_LOAD( "02-13816-00.u003",   0x00000, 0x04000, CRC(71863c5b) SHA1(18fdae631d0990815b07934d9cce73a41df9152f) ) // part numbers here down start with "02", unlike the program ROMs */
	ROM_LOAD( "02-13817-00.u004",   0x10000, 0x02000, CRC(924bead3) SHA1(ba8dd39db9992b426968e5584c94a8b5ed7c0535) )
	ROM_CONTINUE(                   0x1c000, 0x02000 )
	ROM_LOAD( "02-13818-00.u005",   0x12000, 0x02000, CRC(403bdfea) SHA1(71b959c674e7583670e638ebbd44c75784f565c8) )
	ROM_CONTINUE(                   0x1e000, 0x02000 )
	ROM_LOAD( "02-13819-00.u006",   0x14000, 0x02000, CRC(1fee5f10) SHA1(0aee1e139e13528ec328a8a949f576bfca1892a1) )
	ROM_CONTINUE(                   0x20000, 0x02000 )
	ROM_LOAD( "02-13820-00.u007",   0x16000, 0x02000, CRC(42657a1e) SHA1(d5bb6b6a4bc121fea39809b3b2c891345b12f4d7) )
	ROM_CONTINUE(                   0x22000, 0x02000 )
	ROM_LOAD( "02-13821-00.u008",   0x18000, 0x02000, CRC(92f3e006) SHA1(134a2412ddc700473b70aec6331b1a65db3c7e29) )
	ROM_CONTINUE(                   0x24000, 0x02000 )

	ROM_REGION( 0x0c000, "bg_gfx", 0 )
	ROM_LOAD( "02-13801-00.u093", 0x00000, 0x04000, CRC(f9ff55ec) SHA1(2eab55b3708def97f22a1f13d1faa0bfe19c18e9) )
	ROM_LOAD( "02-13802-00.u094", 0x04000, 0x04000, CRC(d4adbcbb) SHA1(dfd427d5a0db309cc7e056857c3b63a1b6e7769b) )
	ROM_LOAD( "02-13803-00.u095", 0x08000, 0x04000, CRC(9178ed76) SHA1(f05568eea53c38f46b16217e63b73194d3a3c500) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	ROM_LOAD( "02-13809-00.u070",  0x00000, 0x4000, CRC(e44eb9f5) SHA1(f15e4262eb96989cbd13a4cbf0b4a0ab390005aa) )
	ROM_LOAD( "02-13804-00.u092",  0x04000, 0x4000, CRC(6c23f1a5) SHA1(0de32ba7b5796bfe37b142fb892beb223f27c381) )
	ROM_LOAD( "02-13805-00.u069",  0x08000, 0x4000, CRC(e9c9f38b) SHA1(6a03cf9ab4d06f05d4fb846f14eab22467c79661) )
	ROM_LOAD( "02-13808-00.u091",  0x0c000, 0x4000, CRC(035534ad) SHA1(e4759992c479d039d6810f129fa2267e0e9527a2) )
	ROM_LOAD( "02-13806-00.u068",  0x10000, 0x4000, CRC(2dbd64d2) SHA1(eaa015c92daa9562f58e5ed1d153ecd3f1403546) )
	ROM_LOAD( "02-13808-00.u090",  0x14000, 0x4000, CRC(d5b4985d) SHA1(d9a5e331f6cf9b4abf9f5d739fadf0d6216fe994) )
	ROM_LOAD( "02-13822-00.u067",  0x18000, 0x4000, CRC(00ff3033) SHA1(ca183f28cb4732ebfc41b6c1651405fee28a9ec6) )
	ROM_LOAD( "02-13810-00.u089",  0x1c000, 0x4000, CRC(4f645973) SHA1(94bf12db53dc08eb917c17f1ba0d5a40922ff22c) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-dangerz.bin", 0x0000, 0x0080, CRC(db2c632b) SHA1(0dda0895145ffab414b5ce6af6636176c19659e3) )
ROM_END

ROM_START( dangerz0 )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "03-13823-00.u012",   0x00000, 0x10000, CRC(ffc030b2) SHA1(8281d2e11cfd351a7a9a02db632f544e9d67d192) ) // 3 PCBs verified as U012 and not "U012T"
	ROM_LOAD( "03-13824-00.u013",   0x10000, 0x10000, CRC(381026c6) SHA1(16c810d162789154e3b5ad38545855370f73b679) ) // 3 PCBs verified as U013 and not "U013T"

	ROM_REGION( 0x28000, "slave", 0 )
	ROM_LOAD( "02-13816-00.u003",   0x00000, 0x04000, CRC(71863c5b) SHA1(18fdae631d0990815b07934d9cce73a41df9152f) ) // part numbers here down start with "02", unlike the program ROMs
	ROM_LOAD( "02-13817-00.u004",   0x10000, 0x02000, CRC(924bead3) SHA1(ba8dd39db9992b426968e5584c94a8b5ed7c0535) )
	ROM_CONTINUE(                   0x1c000, 0x02000 )
	ROM_LOAD( "02-13818-00.u005",   0x12000, 0x02000, CRC(403bdfea) SHA1(71b959c674e7583670e638ebbd44c75784f565c8) )
	ROM_CONTINUE(                   0x1e000, 0x02000 )
	ROM_LOAD( "02-13819-00.u006",   0x14000, 0x02000, CRC(1fee5f10) SHA1(0aee1e139e13528ec328a8a949f576bfca1892a1) )
	ROM_CONTINUE(                   0x20000, 0x02000 )
	ROM_LOAD( "02-13820-00.u007",   0x16000, 0x02000, CRC(42657a1e) SHA1(d5bb6b6a4bc121fea39809b3b2c891345b12f4d7) )
	ROM_CONTINUE(                   0x22000, 0x02000 )
	ROM_LOAD( "02-13821-00.u008",   0x18000, 0x02000, CRC(92f3e006) SHA1(134a2412ddc700473b70aec6331b1a65db3c7e29) )
	ROM_CONTINUE(                   0x24000, 0x02000 )

	ROM_REGION( 0x0c000, "bg_gfx", 0 )
	ROM_LOAD( "02-13801-00.u093", 0x00000, 0x04000, CRC(f9ff55ec) SHA1(2eab55b3708def97f22a1f13d1faa0bfe19c18e9) )
	ROM_LOAD( "02-13802-00.u094", 0x04000, 0x04000, CRC(d4adbcbb) SHA1(dfd427d5a0db309cc7e056857c3b63a1b6e7769b) )
	ROM_LOAD( "02-13803-00.u095", 0x08000, 0x04000, CRC(9178ed76) SHA1(f05568eea53c38f46b16217e63b73194d3a3c500) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	ROM_LOAD( "02-13809-00.u070",  0x00000, 0x4000, CRC(e44eb9f5) SHA1(f15e4262eb96989cbd13a4cbf0b4a0ab390005aa) )
	ROM_LOAD( "02-13804-00.u092",  0x04000, 0x4000, CRC(6c23f1a5) SHA1(0de32ba7b5796bfe37b142fb892beb223f27c381) )
	ROM_LOAD( "02-13805-00.u069",  0x08000, 0x4000, CRC(e9c9f38b) SHA1(6a03cf9ab4d06f05d4fb846f14eab22467c79661) )
	ROM_LOAD( "02-13808-00.u091",  0x0c000, 0x4000, CRC(035534ad) SHA1(e4759992c479d039d6810f129fa2267e0e9527a2) )
	ROM_LOAD( "02-13806-00.u068",  0x10000, 0x4000, CRC(2dbd64d2) SHA1(eaa015c92daa9562f58e5ed1d153ecd3f1403546) )
	ROM_LOAD( "02-13808-00.u090",  0x14000, 0x4000, CRC(d5b4985d) SHA1(d9a5e331f6cf9b4abf9f5d739fadf0d6216fe994) )
	ROM_LOAD( "02-13822-00.u067",  0x18000, 0x4000, CRC(00ff3033) SHA1(ca183f28cb4732ebfc41b6c1651405fee28a9ec6) )
	ROM_LOAD( "02-13810-00.u089",  0x1c000, 0x4000, CRC(4f645973) SHA1(94bf12db53dc08eb917c17f1ba0d5a40922ff22c) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-dangerz.bin", 0x0000, 0x0080, CRC(db2c632b) SHA1(0dda0895145ffab414b5ce6af6636176c19659e3) )
ROM_END


/*
For Baseball: The Season II, the label format is:
------------------------
|(C)1987 Cinematronics | -> Copyright & Manufacturer
|P/N 03-14115-00       | -> Part number with revision
|BASEBALL II     U101  | -> Game name & ROM PCB location
------------------------
*/
ROM_START( basebal2 )
	ROM_REGION( 0x38000, "master", 0 )
	ROM_LOAD( "03-14115-00.u101",   0x00000, 0x02000, CRC(05231fee) SHA1(d2f4f81309d344576aadb39c209240b901476ac2) )
	ROM_LOAD( "03-14116-00.u102",   0x10000, 0x02000, CRC(e1482ea3) SHA1(a55b8c99428fefd033ac481944b370a4c82ac134) )
	ROM_CONTINUE(                   0x1c000, 0x02000 )
	ROM_LOAD( "03-14117-01.u103",   0x12000, 0x02000, CRC(677181dd) SHA1(afc3f33c50551efe5087a3a90f672fe95e3b9087) )
	ROM_CONTINUE(                   0x1e000, 0x02000 )
	ROM_LOAD( "03-14118-01.u104",   0x14000, 0x02000, CRC(5f570264) SHA1(09bf8ec7e40292e3764d51988d5ed613920869ec) )
	ROM_CONTINUE(                   0x20000, 0x02000 )
	ROM_LOAD( "03-14119-01.u105",   0x16000, 0x02000, CRC(90822145) SHA1(52c872e69055589936d5804334255ffc70a5892e) )
	ROM_CONTINUE(                   0x22000, 0x02000 )
	ROM_LOAD( "03-14120-00.u106",   0x18000, 0x02000, CRC(4d2b7217) SHA1(c67cd8361077653f04fc02e8218fd933591d1e45) )
	ROM_CONTINUE(                   0x24000, 0x02000 )
	ROM_LOAD( "03-14121-01.u107",   0x1a000, 0x02000, CRC(b987b97c) SHA1(d9fb7142cbb29ce4389f38416584037a398d3fe2) )
	ROM_CONTINUE(                   0x26000, 0x02000 )
	// Extra banks ( referred to as the "top" board)
	ROM_LOAD( "03-14122-01.u2t",    0x28000, 0x02000, CRC(a89882d8) SHA1(fb17b527c65f5de271fa756d7e682449c76bd4ad) )
	ROM_RELOAD(                     0x30000, 0x02000 )
	ROM_LOAD( "03-14123-01.u3t",    0x2a000, 0x02000, CRC(f9c51e5a) SHA1(a4ed976b9490457b54f2ac6528cf9f4d04732808) )
	ROM_RELOAD(                     0x32000, 0x02000 )

	ROM_REGION( 0x28000, "slave", 0 )
	ROM_LOAD( "03-14100-01.u3",  0x00000, 0x02000, CRC(1dffbdaf) SHA1(15630a84c6034a13449cd481bcb6a93bdf009d1f) )
	ROM_LOAD( "03-14101-01.u4",  0x10000, 0x02000, CRC(c585529c) SHA1(208807c1f8761675903fcf3c590ba3920e980a8b) )
	ROM_CONTINUE(                0x1c000, 0x02000 )
	ROM_LOAD( "03-14102-01.u5",  0x12000, 0x02000, CRC(ace3f918) SHA1(d393f28d0b8c6faf4d76180208deb023f94277fc) )
	ROM_CONTINUE(                0x1e000, 0x02000 )
	ROM_LOAD( "03-14103-01.u6",  0x14000, 0x02000, CRC(cd41cf7a) SHA1(bed00824399cea55017d3cc026ae65ddf7edf5e5) )
	ROM_CONTINUE(                0x20000, 0x02000 )
	ROM_LOAD( "03-14104-01.u7",  0x16000, 0x02000, CRC(9b169e78) SHA1(16ced9610cef997d21668230a5eed6bdfc1df4bd) )
	ROM_CONTINUE(                0x22000, 0x02000 )
	ROM_LOAD( "03-14105-01.u8",  0x18000, 0x02000, CRC(ec596b43) SHA1(230cdfe0ab4dfd837b3fd66acc961a93e196ce2d) )
	ROM_CONTINUE(                0x24000, 0x02000 )
	ROM_LOAD( "03-14106-01.u9",  0x1a000, 0x02000, CRC(b9656baa) SHA1(41b25ee6127981b703859c07f730e94f5694faff) )
	ROM_CONTINUE(                0x26000, 0x02000 )

	ROM_REGION( 0x0c000, "bg_gfx", 0 )
	ROM_LOAD( "03-14112-00.u93", 0x00000, 0x04000, CRC(8ccb1404) SHA1(28ef5a7da1b9edf7ecbba0fd473599df5d181602) )
	ROM_LOAD( "03-14113-00.u94", 0x04000, 0x04000, CRC(9941a55b) SHA1(6917b70bb2a7a23c0517fde43e9375a7dbd64c18) )
	ROM_LOAD( "03-14114-00.u95", 0x08000, 0x04000, CRC(b68baf47) SHA1(ea1d5efe696af56ef5b9161c00957b2a9c7ce372) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	// U70 = Empty
	ROM_LOAD( "03-14111-01.u92",  0x04000, 0x4000, CRC(2508a9ad) SHA1(f0a56d1b8dbe57b16dc1b3d21980149bbdcd0068) )
	ROM_LOAD( "03-14109-00.u69",  0x08000, 0x4000, CRC(b123a28e) SHA1(8d244db422aee9117e901e7d150cdefcbf96dd53) )
	// U91 = Empty
	ROM_LOAD( "03-14108-01.u68",  0x10000, 0x4000, CRC(a1a51383) SHA1(6b734c5d82fb8159768f8849a26f5569cab2f074) )
	ROM_LOAD( "03-14110-01.u90",  0x14000, 0x4000, CRC(ef01d997) SHA1(693bc42b0aaa436f2734efbe2cfb8c98ad4858c6) )
	ROM_LOAD( "03-14107-00.u67",  0x18000, 0x4000, CRC(976334e6) SHA1(5b2534f5ba697bd5bad0aef9cefbb7d1c421c06b) )
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-basebal2.bin", 0x0000, 0x0080, CRC(c9c45105) SHA1(8a27752663c4096174c22146092100fbae23a2f7) )
ROM_END


ROM_START( dblplay )
	ROM_REGION( 0x38000, "master", 0 )
	ROM_LOAD( "15018-01.u101",  0x00000, 0x02000, CRC(17b6af29) SHA1(00865927d74f735ed9bbe635bb554d408bf7f856) )
	ROM_LOAD( "15019-01.u102",  0x10000, 0x02000, CRC(9fc8205e) SHA1(2b783c406539a3d06adacd6b15c8edd86b994561) )
	ROM_CONTINUE(               0x1c000, 0x02000 )
	ROM_LOAD( "15020-01.u103",  0x12000, 0x02000, CRC(4edcc091) SHA1(5db2641fb92eeba22b731074e2818484aaa247a0) )
	ROM_CONTINUE(               0x1e000, 0x02000 )
	ROM_LOAD( "15021-01.u104",  0x14000, 0x02000, CRC(a0eba1c7) SHA1(5d1afd2e6f694416ab64aec334ce6f7803dac93e) )
	ROM_CONTINUE(               0x20000, 0x02000 )
	ROM_LOAD( "15022-01.u105",  0x16000, 0x02000, CRC(7bbfe0b7) SHA1(551e4d48ffc8f3660d59bb4e59f73d438f4eb20d) )
	ROM_CONTINUE(               0x22000, 0x02000 )
	ROM_LOAD( "15023-01.u106",  0x18000, 0x02000, CRC(bbedae34) SHA1(4c15f63ea6ac822a6c9bc5c3b9f9e5a62e57b88c) )
	ROM_CONTINUE(               0x24000, 0x02000 )
	ROM_LOAD( "15024-01.u107",  0x1a000, 0x02000, CRC(02afcf52) SHA1(686332740733d92f87fb004de85be4cb9cbaabc0) )
	ROM_CONTINUE(               0x26000, 0x02000 )
	/* Extra banks ( referred to as the "top" board) */
	ROM_LOAD( "15025-01.u2t",   0x28000, 0x02000, CRC(1c959895) SHA1(efd40c1775f8283162602fdb490bfc18ee784a12) )
	ROM_RELOAD(                 0x30000, 0x02000 )
	ROM_LOAD( "15026-01.u3t",   0x2a000, 0x02000, CRC(ed5196d6) SHA1(03dbc4fa30cee9e2cc132d1fa1e45ac9f503705a) )
	ROM_RELOAD(                 0x32000, 0x02000 )
	ROM_LOAD( "15027-01.u4t",   0x2c000, 0x02000, CRC(9b1e72e9) SHA1(09609835b6951d3dc271e48c8bf91cbff99b6f50) )
	ROM_CONTINUE(               0x34000, 0x02000 )

	ROM_REGION( 0x28000, "slave", 0 )
	ROM_LOAD( "15000-01.u03",  0x00000, 0x02000, CRC(208a920a) SHA1(3544bd79e008f696a9ff400aad8bf0eb52a42451) )
	ROM_LOAD( "15001-01.u04",  0x10000, 0x02000, CRC(751c40d6) SHA1(00e0cba02916c641c85748a1b15af192aca5d60f) )
	ROM_CONTINUE(              0x1c000, 0x02000 )
	ROM_LOAD( "14402-01.u05",  0x12000, 0x02000, CRC(5ffaec36) SHA1(edb36f3f480f6a6ed3b030f7b90e6622b451d086) )
	ROM_CONTINUE(              0x1e000, 0x02000 )
	ROM_LOAD( "14403-01.u06",  0x14000, 0x02000, CRC(48d6d9d3) SHA1(6208f16883867448f478eb49155cd5dbcd25236b) )
	ROM_CONTINUE(              0x20000, 0x02000 )
	ROM_LOAD( "15004-01.u07",  0x16000, 0x02000, CRC(6a7acebc) SHA1(133258a78fdb7b8dc08312e8619767fa694f175e) )
	ROM_CONTINUE(              0x22000, 0x02000 )
	ROM_LOAD( "15005-01.u08",  0x18000, 0x02000, CRC(69d487c9) SHA1(217560f9cbb196970fb9ccbe32c640b376321b7e) )
	ROM_CONTINUE(              0x24000, 0x02000 )
	ROM_LOAD( "15006-01.u09",  0x1a000, 0x02000, CRC(ab3aac49) SHA1(699a6a66e6b35f1b287ff1ab3a12365dbdc16041) )
	ROM_CONTINUE(              0x26000, 0x02000 )

	ROM_REGION( 0x0c000, "bg_gfx", 0 )
	ROM_LOAD( "15015-01.u93", 0x00000, 0x04000, CRC(8ccb1404) SHA1(28ef5a7da1b9edf7ecbba0fd473599df5d181602) )
	ROM_LOAD( "15016-01.u94", 0x04000, 0x04000, CRC(9941a55b) SHA1(6917b70bb2a7a23c0517fde43e9375a7dbd64c18) )
	ROM_LOAD( "15017-01.u95", 0x08000, 0x04000, CRC(b68baf47) SHA1(ea1d5efe696af56ef5b9161c00957b2a9c7ce372) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	// U70 = Empty
	ROM_LOAD( "15014-01.u92",  0x04000, 0x4000, CRC(2508a9ad) SHA1(f0a56d1b8dbe57b16dc1b3d21980149bbdcd0068) )
	ROM_LOAD( "15009-01.u69",  0x08000, 0x4000, CRC(b123a28e) SHA1(8d244db422aee9117e901e7d150cdefcbf96dd53) )
	// U91 = Empty
	ROM_LOAD( "15008-01.u68",  0x10000, 0x4000, CRC(a1a51383) SHA1(6b734c5d82fb8159768f8849a26f5569cab2f074) )
	ROM_LOAD( "15012-01.u90",  0x14000, 0x4000, CRC(ef01d997) SHA1(693bc42b0aaa436f2734efbe2cfb8c98ad4858c6) )
	ROM_LOAD( "15007-01.u67",  0x18000, 0x4000, CRC(976334e6) SHA1(5b2534f5ba697bd5bad0aef9cefbb7d1c421c06b) )
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-dblplay.bin", 0x0000, 0x0080, CRC(50fe6185) SHA1(19617ed482af782dea903bd88c56b3cbb75ce1c7) )
ROM_END


/*
For Strike Zone, the label format is:
------------------------
|(C)1988 Leland Corp.  | -> Copyright & Manufacturer
|P/N 03-15007-01       | -> Part number with revision
|STRIKE ZONE     U67   | -> Game name & ROM PCB location
------------------------
*/
ROM_START( strkzone )
	ROM_REGION( 0x38000, "master", 0 )
	ROM_LOAD( "strkzone.u101",  0x00000, 0x04000, CRC(8d83a611) SHA1(d17114559c8d60e3107895bdcb1886cc843b624c) )
	ROM_LOAD( "strkzone.u102",  0x10000, 0x02000, CRC(3859e67d) SHA1(0a0d18c16fa5becae2ecc147dfafadc16dee8d2f) )
	ROM_CONTINUE(               0x1c000, 0x02000 )
	ROM_LOAD( "strkzone.u103",  0x12000, 0x02000, CRC(cdd83bfb) SHA1(6d5c1e9e951a0bfdd79fd54b06e2e4f1bf8e37b4) )
	ROM_CONTINUE(               0x1e000, 0x02000 )
	ROM_LOAD( "strkzone.u104",  0x14000, 0x02000, CRC(be280212) SHA1(f48f2edd41bd4f2729ee6c15fb228c758da40ea1) )
	ROM_CONTINUE(               0x20000, 0x02000 )
	ROM_LOAD( "strkzone.u105",  0x16000, 0x02000, CRC(afb63390) SHA1(42df802ca2a247b971ae274bd6f7d1f1e5893fe3) )
	ROM_CONTINUE(               0x22000, 0x02000 )
	ROM_LOAD( "strkzone.u106",  0x18000, 0x02000, CRC(e853b9f6) SHA1(07cc7bd0053422d68526a6e1b68165db60af6429) )
	ROM_CONTINUE(               0x24000, 0x02000 )
	ROM_LOAD( "strkzone.u107",  0x1a000, 0x02000, CRC(1b4b6c2d) SHA1(9cd5e5ce7bc3088f14b6cbbd7c2d5b5e69a7bc11) )
	ROM_CONTINUE(               0x26000, 0x02000 )
	// Extra banks ( referred to as the "top" board)
	ROM_LOAD( "03-15313-01.u2t",   0x28000, 0x02000, CRC(8e0af06f) SHA1(ad277433a2c97c388e626a0ce9119466dff85d37) )
	ROM_RELOAD(                    0x30000, 0x02000 )
	ROM_LOAD( "03-15314-01.u3t",   0x2a000, 0x02000, CRC(909d35f3) SHA1(2ec51b1591990cf13b71d6c343bfe9540d3c2b69) )
	ROM_RELOAD(                    0x32000, 0x02000 )
	ROM_LOAD( "03-15027-01.u4t",   0x2c000, 0x02000, CRC(9b1e72e9) SHA1(09609835b6951d3dc271e48c8bf91cbff99b6f50) )
	ROM_CONTINUE(                  0x34000, 0x02000 )

	ROM_REGION( 0x28000, "slave", 0 )
	ROM_LOAD( "strkzone.u3",  0x00000, 0x02000, CRC(40258fbe) SHA1(4a68dbf050455bf15fadef20f615ab1de194a1c2) )
	ROM_LOAD( "strkzone.u4",  0x10000, 0x02000, CRC(df7f2604) SHA1(4aed43905fedf84de683dea9785a73d6d9f89713) )
	ROM_CONTINUE(             0x1c000, 0x02000 )
	ROM_LOAD( "strkzone.u5",  0x12000, 0x02000, CRC(37885206) SHA1(60428a4ad16c452e7a90c6d2617c9905cef8ed0b) )
	ROM_CONTINUE(             0x1e000, 0x02000 )
	ROM_LOAD( "strkzone.u6",  0x14000, 0x02000, CRC(6892dc4f) SHA1(be0c8c0afed925e2e373e10b42c00f5ab6cfed40) )
	ROM_CONTINUE(             0x20000, 0x02000 )
	ROM_LOAD( "strkzone.u7",  0x16000, 0x02000, CRC(6ac8f87c) SHA1(cf820922f09d503bdd73e20f9e5e786910ab2ab8) )
	ROM_CONTINUE(             0x22000, 0x02000 )
	ROM_LOAD( "strkzone.u8",  0x18000, 0x02000, CRC(4b6d3725) SHA1(e7d1d31df3fd10dd51a6969a0ca688a4b7e3d3f1) )
	ROM_CONTINUE(             0x24000, 0x02000 )
	ROM_LOAD( "strkzone.u9",  0x1a000, 0x02000, CRC(ab3aac49) SHA1(699a6a66e6b35f1b287ff1ab3a12365dbdc16041) )
	ROM_CONTINUE(             0x26000, 0x02000 )

	ROM_REGION( 0x0c000, "bg_gfx", 0 )
	ROM_LOAD( "strkzone.u93", 0x00000, 0x04000, CRC(8ccb1404) SHA1(28ef5a7da1b9edf7ecbba0fd473599df5d181602) )
	ROM_LOAD( "strkzone.u94", 0x04000, 0x04000, CRC(9941a55b) SHA1(6917b70bb2a7a23c0517fde43e9375a7dbd64c18) )
	ROM_LOAD( "strkzone.u95", 0x08000, 0x04000, CRC(b68baf47) SHA1(ea1d5efe696af56ef5b9161c00957b2a9c7ce372) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	// U70 = Empty
	ROM_LOAD( "strkzone.u92",     0x04000, 0x4000, CRC(2508a9ad) SHA1(f0a56d1b8dbe57b16dc1b3d21980149bbdcd0068) )
	ROM_LOAD( "03-15009-01.u69",  0x08000, 0x4000, CRC(b123a28e) SHA1(8d244db422aee9117e901e7d150cdefcbf96dd53) )
	// U91 = Empty
	ROM_LOAD( "03-15008-01.u68",  0x10000, 0x4000, CRC(a1a51383) SHA1(6b734c5d82fb8159768f8849a26f5569cab2f074) )
	ROM_LOAD( "strkzone.u90",     0x14000, 0x4000, CRC(ef01d997) SHA1(693bc42b0aaa436f2734efbe2cfb8c98ad4858c6) )
	ROM_LOAD( "03-15007-01.u67",  0x18000, 0x4000, CRC(976334e6) SHA1(5b2534f5ba697bd5bad0aef9cefbb7d1c421c06b) )
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-strkzone.bin", 0x0000, 0x0080, CRC(0c2fe514) SHA1(79487e50cdc4e28bed2ef654eb7cc1014f4fc94d) )
ROM_END


/*
For Redline Racer, the label format is:
------------------------
|(C)1987 Cinematronics | -> Copyright & Manufacturer
|P/N 02-13913-01       | -> Part number with revision
|2P REDLINE       U009 | -> Game name & ROM PCB location
------------------------
*/
ROM_START( redlin2p )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "02-13932-01.u23t", 0x00000, 0x10000, CRC(ecdf0fbe) SHA1(186e1cecadb84b5085d9ccbf6512553a80b9ebfb) )
	ROM_LOAD( "02-13931-01.u22t", 0x10000, 0x10000, CRC(16d01978) SHA1(6882eac35a54a91f12a8d37a4f83d7ca0dc65ef5) )

	ROM_REGION( 0x80000, "slave", 0 )
	ROM_LOAD( "02-13907-01.u3",  0x00000, 0x04000, CRC(b760d63e) SHA1(b117fd1f96b861abefedd049a305b60c964aadad) )
	ROM_LOAD( "02-13908-01.u4",  0x10000, 0x02000, CRC(a30739d3) SHA1(eefce1f11ead0ca4c7fc3ed3102fbdb8bfbf3cbc) )
	ROM_CONTINUE(                0x1c000, 0x02000 )
	ROM_LOAD( "02-13909-01.u5",  0x12000, 0x02000, CRC(aaf16ad7) SHA1(d08d224ecb824204143e9fd1b1657dc2dd6035e6) )
	ROM_CONTINUE(                0x1e000, 0x02000 )
	ROM_LOAD( "02-13910-01.u6",  0x14000, 0x02000, CRC(d03469eb) SHA1(78bda66821cc458be58ae179c0d39879b9f02282) )
	ROM_CONTINUE(                0x20000, 0x02000 )
	ROM_LOAD( "02-13911-01.u7",  0x16000, 0x02000, CRC(8ee1f547) SHA1(05ef34786f0e26f5d891f5b25c007956b92bf0cb) )
	ROM_CONTINUE(                0x22000, 0x02000 )
	ROM_LOAD( "02-13912-01.u8",  0x18000, 0x02000, CRC(e5b57eac) SHA1(b31f38ddfdf896cc90703df486b840214ed16a7f) )
	ROM_CONTINUE(                0x24000, 0x02000 )
	ROM_LOAD( "02-13913-01.u9",  0x1a000, 0x02000, CRC(02886071) SHA1(699f13677a3e76e8ec2ec73e62d4da4038f9f85d) )
	ROM_CONTINUE(                0x26000, 0x02000 )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "u17t",    0x0e0001, 0x10000, CRC(8d26f221) SHA1(cd5b1d88fec0ff1ab7af554a9fcffc43d33a12e7) )
	ROM_LOAD16_BYTE( "u28t",    0x0e0000, 0x10000, CRC(7aa21b2c) SHA1(5fd9f49d4bb1dc28393b9df76dfa19e28677639b) )

	ROM_REGION( 0x0c000, "bg_gfx", 0 )
	ROM_LOAD( "02-13930-01.u93", 0x00000, 0x04000, CRC(0721f42e) SHA1(fe3d447316b1e6c7c1b6849319fad1aebe5e6aa1) )
	ROM_LOAD( "02-13929-01.u94", 0x04000, 0x04000, CRC(1522e7b2) SHA1(540fc55013a22a5afb32a89b42ef9b11dbe36d97) )
	ROM_LOAD( "02-13928-01.u95", 0x08000, 0x04000, CRC(c321b5d1) SHA1(d1524165e71fe200cab6fd6f6327da0e6efc6868) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	ROM_LOAD( "02-13920-01.u70",  0x00000, 0x4000, CRC(f343d34a) SHA1(161348e082afeb69862c3752f4dd536166edad21) )
	ROM_LOAD( "02-13921-01.u92",  0x04000, 0x4000, CRC(c9ba8d41) SHA1(777a504e3ffe6c3da94f71eb1b14e05dc861db66) )
	ROM_LOAD( "02-13922-01.u69",  0x08000, 0x4000, CRC(276cfba0) SHA1(4b252f21e2d1314801cf9329ed9383ff9158c382) )
	ROM_LOAD( "02-13923-01.u91",  0x0c000, 0x4000, CRC(4a88ea34) SHA1(e79cc404f435789ef8f62c6bef03af1b9b89caeb) )
	ROM_LOAD( "02-13924-01.u68",  0x10000, 0x4000, CRC(3995cb7e) SHA1(4a77d3c71e2a8240a21a82ac946804895f4af959) )
	// U90 = Empty
	ROM_LOAD( "02-13926-01.u67",  0x18000, 0x4000, CRC(daa30add) SHA1(e9c066406c2d50ab3fc8eea8a97a181ad8c950c7) )
	ROM_LOAD( "02-13927-01.u89",  0x1c000, 0x4000, CRC(30e60fb5) SHA1(374c84358d2b7ae7c74321996797af9adbc2a155) )

	ROM_REGION( 0x0300, "plds", 0 )
	ROM_LOAD( "pal16l8-2601.25t", 0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pl20x10-0501.u21", 0x0200, 0x00cc, CRC(09ef7a46) SHA1(c37b10d268bf730a68748eee085b9d3757ee45b3) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-redlin2p.bin", 0x0000, 0x0080, CRC(e632970c) SHA1(4357b5235e31897afc3652e3ef879e259d344a43) )
ROM_END


/*
For Quarterback, the label format is:
------------------------
|(C)1987 Leland Corp.  | -> Copyright & Manufacturer
|P/N 03-15218-05       | -> Part number with revision
|QUARTERBACK     U48T  | -> Game name & ROM PCB location
------------------------
*/
ROM_START( quarterb )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "03-15219-05.u49t", 0x00000, 0x10000, CRC(ff653e4f) SHA1(761e18ccbdc1c559648c47d06ee21a8a4710c5a0) )
	ROM_LOAD( "03-15218-05.u48t", 0x10000, 0x10000, CRC(34b83d81) SHA1(0425638063872ff562939439871631f7aa642182) )

	ROM_REGION( 0x80000, "slave", 0 )
	ROM_LOAD( "03-15200-01.u3",  0x00000, 0x04000, CRC(83297861) SHA1(9d836f647491af945de021fbf8c75159b32c22c9) )
	ROM_LOAD( "03-15201-01.u4",  0x10000, 0x02000, CRC(af8dbdab) SHA1(663a32b55ef0337074a223288e59b53c4a10b616) )
	ROM_CONTINUE(                0x1c000, 0x02000 )
	ROM_LOAD( "03-15202-01.u5",  0x12000, 0x02000, CRC(3eeecb3d) SHA1(ee2a7a2dba8137c6e414f74300b445db9141a49d) )
	ROM_CONTINUE(                0x1e000, 0x02000 )
	ROM_LOAD( "03-15203-01.u6",  0x14000, 0x02000, CRC(b9c5b663) SHA1(5948f77301446dcab64d787ae6f2c49bee666a7b) )
	ROM_CONTINUE(                0x20000, 0x02000 )
	ROM_LOAD( "03-15204-01.u7",  0x16000, 0x02000, CRC(c68821b7) SHA1(bd68282453ab2752a31681a2c5f31361a704bc07) )
	ROM_CONTINUE(                0x22000, 0x02000 )
	ROM_LOAD( "03-15205-01.u8",  0x18000, 0x02000, CRC(2be843a9) SHA1(a77c84ab95e20dfef09ff2c34b302b2c4ec87f02) )
	ROM_CONTINUE(                0x24000, 0x02000 )
	ROM_LOAD( "03-15206-01.u9",  0x1a000, 0x02000, CRC(6bf8d4ab) SHA1(cc9b3f1e651b2a667f17553aac655f0039983890) )
	ROM_CONTINUE(                0x26000, 0x02000 )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "03-15222-02.u45t", 0x040001, 0x10000, CRC(710bdc76) SHA1(610f7baa17adf2d16c9494b05556b49ae376fe81) )
	ROM_LOAD16_BYTE( "03-15225-02.u62t", 0x040000, 0x10000, CRC(041cecde) SHA1(91556a393d61979d3e92f75142832353e9081a15) )
	ROM_LOAD16_BYTE( "03-15221-02.u44t", 0x060001, 0x10000, CRC(e0459ddb) SHA1(811896fe3398ecc322ca20c2376b715b2d44992e) )
	ROM_LOAD16_BYTE( "03-15224-02.u61t", 0x060000, 0x10000, CRC(9027c579) SHA1(47177f9c42d134ec44a8b1aad17012dd971cf1fd) )
	ROM_LOAD16_BYTE( "03-15220-02.u43t", 0x0e0001, 0x10000, CRC(48a8a018) SHA1(f50d66feeab32f1edc47f4b3f33e579c06fd979e) )
	ROM_LOAD16_BYTE( "03-15223-02.u60t", 0x0e0000, 0x10000, CRC(6a299766) SHA1(4e5b1f930f668302496a314bbe8876a21012fb20) )

	ROM_REGION( 0x0c000, "bg_gfx", 0 )
	ROM_LOAD( "03-15215-01.u93", 0x00000, 0x04000, CRC(4fb678d7) SHA1(ca729ca8d2ba1e22a7b650ddfc330e85e294e48f) )
	ROM_LOAD( "03-15216-01.u94", 0x04000, 0x04000, CRC(7b57a44c) SHA1(b28ecdc8b1579e677a58a4b5257d5d754783148f) )
	ROM_LOAD( "03-15217-01.u95", 0x08000, 0x04000, CRC(29bc33fd) SHA1(e85d20b24144c5b0f6ffa6dc96f1abb35bce437a) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	ROM_LOAD( "03-15210-01.u70",  0x00000, 0x4000, CRC(a5aea20e) SHA1(c5b40bdb63cd29386f73e69b814c37eb43dadbac) )
	ROM_LOAD( "03-15214-01.u92",  0x04000, 0x4000, CRC(36f261ca) SHA1(d42868c9ace5bec75b74268393755340ccafea59) )
	ROM_LOAD( "03-15209-01.u69",  0x08000, 0x4000, CRC(0f5d74a4) SHA1(76bd78153a5f986ffdd0db606a1e2a0b895b4832) )
	// U91 = Empty
	ROM_LOAD( "03-15208-01.u68",  0x10000, 0x4000, CRC(0319aec7) SHA1(e4f14ce9b4712c1cee69141165d187e9068101fc) )
	ROM_LOAD( "03-15212-01.u90",  0x14000, 0x4000, CRC(38b298d6) SHA1(fa22d8d5fa66f1f7f052541f21408a6d755a1317) )
	ROM_LOAD( "03-15207-01.u67",  0x18000, 0x4000, CRC(5ff86aad) SHA1(6c2704dc4a934270e7080c610181018c9c5e10c5) )
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-quarterb.bin", 0x0000, 0x0080, CRC(99d8c2ef) SHA1(36b6ac0b03cac9bd0c1f9c7a30fb85e5a2790fd4) )
ROM_END

ROM_START( quarterba )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "03-15219-02.u49t",   0x00000, 0x10000, CRC(7fbe1e5a) SHA1(a4af54328935e348f2903fe7f7dea8612660b899) )
	ROM_LOAD( "03-15218-02.u48t",   0x10000, 0x10000, CRC(6fbd4b27) SHA1(8146f276af5e3ef968851fa95c8f979b8b969ef6) )

	ROM_REGION( 0x80000, "slave", 0 )
	ROM_LOAD( "03-15200-01.u3",  0x00000, 0x04000, CRC(83297861) SHA1(9d836f647491af945de021fbf8c75159b32c22c9) )
	ROM_LOAD( "03-15201-01.u4",  0x10000, 0x02000, CRC(af8dbdab) SHA1(663a32b55ef0337074a223288e59b53c4a10b616) )
	ROM_CONTINUE(                0x1c000, 0x02000 )
	ROM_LOAD( "03-15202-01.u5",  0x12000, 0x02000, CRC(3eeecb3d) SHA1(ee2a7a2dba8137c6e414f74300b445db9141a49d) )
	ROM_CONTINUE(                0x1e000, 0x02000 )
	ROM_LOAD( "03-15203-01.u6",  0x14000, 0x02000, CRC(b9c5b663) SHA1(5948f77301446dcab64d787ae6f2c49bee666a7b) )
	ROM_CONTINUE(                0x20000, 0x02000 )
	ROM_LOAD( "03-15204-01.u7",  0x16000, 0x02000, CRC(c68821b7) SHA1(bd68282453ab2752a31681a2c5f31361a704bc07) )
	ROM_CONTINUE(                0x22000, 0x02000 )
	ROM_LOAD( "03-15205-01.u8",  0x18000, 0x02000, CRC(2be843a9) SHA1(a77c84ab95e20dfef09ff2c34b302b2c4ec87f02) )
	ROM_CONTINUE(                0x24000, 0x02000 )
	ROM_LOAD( "03-15206-01.u9",  0x1a000, 0x02000, CRC(6bf8d4ab) SHA1(cc9b3f1e651b2a667f17553aac655f0039983890) )
	ROM_CONTINUE(                0x26000, 0x02000 )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "03-15222-01.u45t", 0x040001, 0x10000, CRC(722d1a19) SHA1(b7c7c606798c4357cee58b64d95f2f6a6172d72e) )
	ROM_LOAD16_BYTE( "03-15225-01.u62t", 0x040000, 0x10000, CRC(f8c20496) SHA1(5f948a56743127e19d9fbd888b546ce82c0b05f6) )
	ROM_LOAD16_BYTE( "03-15221-01.u44t", 0x060001, 0x10000, CRC(bc6abaaf) SHA1(2ca9116c1861d7089679de034c2255bc51179338) )
	ROM_LOAD16_BYTE( "03-15224-01.u61t", 0x060000, 0x10000, CRC(7ce3c3b7) SHA1(fa85a9159895e26dff03cc6955fdd880213a0dec) )
	ROM_LOAD16_BYTE( "03-15220-01.u43t", 0x0e0001, 0x10000, CRC(ccb6c8d7) SHA1(bafe1ba6259f396cfa91fc6d2ff7832199763f3e) )
	ROM_LOAD16_BYTE( "03-15223-01.u60t", 0x0e0000, 0x10000, CRC(c0ee425d) SHA1(4edbd62b8bb7f814e7ffa3111e6fb1e8b6615ae8) )

	ROM_REGION( 0x0c000, "bg_gfx", 0 )
	ROM_LOAD( "03-15215-01.u93", 0x00000, 0x04000, CRC(4fb678d7) SHA1(ca729ca8d2ba1e22a7b650ddfc330e85e294e48f) )
	ROM_LOAD( "03-15216-01.u94", 0x04000, 0x04000, CRC(7b57a44c) SHA1(b28ecdc8b1579e677a58a4b5257d5d754783148f) )
	ROM_LOAD( "03-15217-01.u95", 0x08000, 0x04000, CRC(29bc33fd) SHA1(e85d20b24144c5b0f6ffa6dc96f1abb35bce437a) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	ROM_LOAD( "03-15210-01.u70",  0x00000, 0x4000, CRC(a5aea20e) SHA1(c5b40bdb63cd29386f73e69b814c37eb43dadbac) )
	ROM_LOAD( "03-15214-01.u92",  0x04000, 0x4000, CRC(36f261ca) SHA1(d42868c9ace5bec75b74268393755340ccafea59) )
	ROM_LOAD( "03-15209-01.u69",  0x08000, 0x4000, CRC(0f5d74a4) SHA1(76bd78153a5f986ffdd0db606a1e2a0b895b4832) )
	// U91 = Empty
	ROM_LOAD( "03-15208-01.u68",  0x10000, 0x4000, CRC(0319aec7) SHA1(e4f14ce9b4712c1cee69141165d187e9068101fc) )
	ROM_LOAD( "03-15212-01.u90",  0x14000, 0x4000, CRC(38b298d6) SHA1(fa22d8d5fa66f1f7f052541f21408a6d755a1317) )
	ROM_LOAD( "03-15207-01.u67",  0x18000, 0x4000, CRC(5ff86aad) SHA1(6c2704dc4a934270e7080c610181018c9c5e10c5) )
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-quarterb.bin", 0x0000, 0x0080, CRC(99d8c2ef) SHA1(36b6ac0b03cac9bd0c1f9c7a30fb85e5a2790fd4) )
ROM_END

/*
For Quarterback (Cocktail), the label format is:
------------------------
|(C)1987 Leland Corp.  | -> Copyright & Manufacturer
|P/N 03-15505-01       | -> Part number with revision
|QB COCK.        U48T  | -> Game name & ROM PCB location
------------------------
*/
ROM_START( quarterbc )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "03-15506-01.u49t", 0x00000, 0x10000, CRC(d750c5ed) SHA1(ea3389695e96da06e7e5988c3500e953d35f2af1) )
	ROM_LOAD( "03-15505-01.u48t", 0x10000, 0x10000, CRC(e17b8a92) SHA1(a3ac8be9018b6003624d8aaebd0233059073c6b2) )

	ROM_REGION( 0x80000, "slave", 0 )
	ROM_LOAD( "03-15200-01.u3",  0x00000, 0x04000, CRC(83297861) SHA1(9d836f647491af945de021fbf8c75159b32c22c9) )
	ROM_LOAD( "03-15201-01.u4",  0x10000, 0x02000, CRC(af8dbdab) SHA1(663a32b55ef0337074a223288e59b53c4a10b616) )
	ROM_CONTINUE(                0x1c000, 0x02000 )
	ROM_LOAD( "03-15202-01.u5",  0x12000, 0x02000, CRC(3eeecb3d) SHA1(ee2a7a2dba8137c6e414f74300b445db9141a49d) )
	ROM_CONTINUE(                0x1e000, 0x02000 )
	ROM_LOAD( "03-15203-01.u6",  0x14000, 0x02000, CRC(b9c5b663) SHA1(5948f77301446dcab64d787ae6f2c49bee666a7b) )
	ROM_CONTINUE(                0x20000, 0x02000 )
	ROM_LOAD( "03-15204-01.u7",  0x16000, 0x02000, CRC(c68821b7) SHA1(bd68282453ab2752a31681a2c5f31361a704bc07) )
	ROM_CONTINUE(                0x22000, 0x02000 )
	ROM_LOAD( "03-15205-01.u8",  0x18000, 0x02000, CRC(2be843a9) SHA1(a77c84ab95e20dfef09ff2c34b302b2c4ec87f02) )
	ROM_CONTINUE(                0x24000, 0x02000 )
	ROM_LOAD( "03-15206-01.u9",  0x1a000, 0x02000, CRC(6bf8d4ab) SHA1(cc9b3f1e651b2a667f17553aac655f0039983890) )
	ROM_CONTINUE(                0x26000, 0x02000 )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "03-15222-02.u45t", 0x040001, 0x10000, CRC(710bdc76) SHA1(610f7baa17adf2d16c9494b05556b49ae376fe81) )
	ROM_LOAD16_BYTE( "03-15225-02.u62t", 0x040000, 0x10000, CRC(041cecde) SHA1(91556a393d61979d3e92f75142832353e9081a15) )
	ROM_LOAD16_BYTE( "03-15221-02.u44t", 0x060001, 0x10000, CRC(e0459ddb) SHA1(811896fe3398ecc322ca20c2376b715b2d44992e) )
	ROM_LOAD16_BYTE( "03-15224-02.u61t", 0x060000, 0x10000, CRC(9027c579) SHA1(47177f9c42d134ec44a8b1aad17012dd971cf1fd) )
	ROM_LOAD16_BYTE( "03-15220-02.u43t", 0x0e0001, 0x10000, CRC(48a8a018) SHA1(f50d66feeab32f1edc47f4b3f33e579c06fd979e) )
	ROM_LOAD16_BYTE( "03-15223-02.u60t", 0x0e0000, 0x10000, CRC(6a299766) SHA1(4e5b1f930f668302496a314bbe8876a21012fb20) )

	ROM_REGION( 0x0c000, "bg_gfx", 0 )
	ROM_LOAD( "03-15502-01.u93", 0x00000, 0x04000, CRC(5c2abd20) SHA1(a67457249d9c3bb18c4f572b5c2ebaa62b72bc0d) )
	ROM_LOAD( "03-15503-01.u94", 0x04000, 0x04000, CRC(544a192c) SHA1(cc7dc0af60a5d578bc80591c6e0e1b9fcb940fd7) )
	ROM_LOAD( "03-15504-01.u95", 0x08000, 0x04000, CRC(c8bb7ab4) SHA1(bef121653e61173896b9f5f4b80d4b5b38ec1ec6) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	ROM_LOAD( "03-15500-01.u70",  0x00000, 0x4000, CRC(1cf82a44) SHA1(e719e04ebacf5a959b577592884963da60722076) )
	ROM_LOAD( "03-15501-01.u92",  0x04000, 0x4000, CRC(281be799) SHA1(ea3097d5ad5a7b1115c97865da4e81efca60ea32) )
	ROM_LOAD( "03-15209-01.u69",  0x08000, 0x4000, CRC(0f5d74a4) SHA1(76bd78153a5f986ffdd0db606a1e2a0b895b4832) )
	// U91 = Empty
	ROM_LOAD( "03-15208-01.u68",  0x10000, 0x4000, CRC(0319aec7) SHA1(e4f14ce9b4712c1cee69141165d187e9068101fc) )
	ROM_LOAD( "03-15212-01.u90",  0x14000, 0x4000, CRC(38b298d6) SHA1(fa22d8d5fa66f1f7f052541f21408a6d755a1317) )
	ROM_LOAD( "03-15207-01.u67",  0x18000, 0x4000, CRC(5ff86aad) SHA1(6c2704dc4a934270e7080c610181018c9c5e10c5) )
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-quarterb.bin", 0x0000, 0x0080, CRC(99d8c2ef) SHA1(36b6ac0b03cac9bd0c1f9c7a30fb85e5a2790fd4) )
ROM_END


/*
Viper, the label format is:
------------------------
|(C)1988 Leland Corp.  | -> Copyright & Manufacturer
|P/N 03-15616-03       | -> Part number with revision
|VIPER           U48T  | -> Game name & ROM PCB location
------------------------
*/
ROM_START( viper )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "03-15617-03.u49t",   0x00000, 0x10000, CRC(7e4688a6) SHA1(282f98d22447b2f93d6f328a351ce1613a33069b) )
	ROM_LOAD( "03-15616-03.u48t",   0x10000, 0x10000, CRC(3fe2f0bf) SHA1(2a1a7d1654e5f45a5b30374596865006e80928f5) )

	ROM_REGION( 0x80000, "slave", 0 )
	ROM_LOAD( "03-15600-02.u3",  0x00000, 0x02000, CRC(0f57f68a) SHA1(2cb132eb41da5f8a90f83e54a6a8c00a62b66949) )
	ROM_LOAD( "03-15612-02.u2t", 0x10000, 0x10000, CRC(4043d4ee) SHA1(70ebb98444f13a25cdcd8d31ee47a20af7df5613) )
	ROM_LOAD( "03-15613-02.u3t", 0x20000, 0x10000, CRC(213bc02b) SHA1(53fadd81a0138525d3d39fd9c2ea258f90b2e6e7) )
	ROM_LOAD( "03-15614-02.u4t", 0x30000, 0x10000, CRC(ce0b95b4) SHA1(1a322714ce1e9e5589da9966f2e684e9a2c22592) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "03-15620-02.u45t", 0x040001, 0x10000, CRC(7380ece1) SHA1(c131c80c67503785ba1ec5b31366cd72f0f7e0e3) )
	ROM_LOAD16_BYTE( "03-15623-02.u62t", 0x040000, 0x10000, CRC(2921d8f9) SHA1(5ce6752ef3928b40263efdcd81fae376e2d86e36) )
	ROM_LOAD16_BYTE( "03-15619-02.u44t", 0x060001, 0x10000, CRC(c8507cc2) SHA1(aae9f19b3bc6790a137d94e3c4bb3e61e8670b42) )
	ROM_LOAD16_BYTE( "03-15622-02.u61t", 0x060000, 0x10000, CRC(32dfda37) SHA1(bbd643239add553e61735c2997bb4ffdbe67d9e1) )
	ROM_LOAD16_BYTE( "03-15618-02.u43t", 0x0e0001, 0x10000, CRC(5562e0c3) SHA1(4c7b0cedc5adc4e24a1cd6010591205ddb16d554) )
	ROM_LOAD16_BYTE( "03-15621-02.u60t", 0x0e0000, 0x10000, CRC(cb468f2b) SHA1(f37596c781b1d7c49d8f62d289c15a2ae0d752cc) )

	ROM_REGION( 0x0c000, "bg_gfx", 0 )
	ROM_LOAD( "03-15609-01.u93", 0x00000, 0x04000, CRC(08ad92e9) SHA1(6eaffd58f03db3a67871ce7390b01754842e2574) )
	ROM_LOAD( "03-15610-01.u94", 0x04000, 0x04000, CRC(d4e56dfb) SHA1(0fc83847b8629534b15f9366f197c87e3c81c61a) )
	ROM_LOAD( "03-15611-01.u95", 0x08000, 0x04000, CRC(3a2c46fb) SHA1(e96849447852a9922e72f7f1908c76fea3c603c4) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	ROM_LOAD( "03-15604-01.u70",  0x00000, 0x4000, CRC(7e3b0cce) SHA1(d9002df27e3de28d40a9cfb081512526987377b2) )
	ROM_LOAD( "03-15608-01.u92",  0x04000, 0x4000, CRC(a9bde0ef) SHA1(84f55bc62fc49ae0232ada2ac192c5c8a2519703) )
	ROM_LOAD( "03-15603-01.u69",  0x08000, 0x4000, CRC(aecc9516) SHA1(513ae810d62d5df29a96a567a7c024f12c6837d5) )
	ROM_LOAD( "03-15607-01.u91",  0x0c000, 0x4000, CRC(14f06f88) SHA1(7e76b5b7d74635dff2dd2245d345beee5c0ee46e) )
	ROM_LOAD( "03-15602-01.u68",  0x10000, 0x4000, CRC(4ef613ad) SHA1(b08445056038fdef90bd9de0a4effdfd18f81e15) )
	ROM_LOAD( "03-15606-01.u90",  0x14000, 0x4000, CRC(3c2e8e76) SHA1(f526240df82e14102854de8e391571f747dfa405) )
	ROM_LOAD( "03-15601-01.u67",  0x18000, 0x4000, CRC(dc7006cd) SHA1(d828b9c7a43c1b37aa55d1c5891fe0744ea78595) )
	ROM_LOAD( "03-15605-01.u89",  0x1c000, 0x4000, CRC(4aa9c788) SHA1(77095d7ce4949db3c39c19d131d2902e4099b6d4) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-viper.bin", 0x0000, 0x0080, CRC(1c1e7a49) SHA1(3cc0fe501d86cf63de5ac812b8c63e20d9e73372) )
ROM_END


/*
John Elway's Team Quarterback, the label format is:
------------------------
|(C)1988 Leland Corp.  | -> Copyright & Manufacturer
|P/N 03-15618-03       | -> Part number with revision
|TEAM QB         U58T  | -> Game name & ROM PCB location
------------------------
*/
ROM_START( teamqb )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "03-15618-03.u58t",   0x00000, 0x10000, CRC(b32568dc) SHA1(92fb8dd89cc7838129e7b106bc0e35107372904f) )
	ROM_LOAD( "03-15619-03.u59t",   0x10000, 0x10000, CRC(40b3319f) SHA1(26c6e26cd440fc7e1ab5ee7536e17a1c00b83f44) )

	ROM_REGION( 0x80000, "slave", 0 )
	ROM_LOAD( "03-15600-01.u3",   0x00000, 0x02000, CRC(46615844) SHA1(cb73dd73d389d1f6a5da91d0935b5461649ba706) )
	ROM_LOAD( "03-15601-01.u2t",  0x10000, 0x10000, CRC(8e523c58) SHA1(7f1133144177c39849fd6355bf9250895b2d597f) )
	ROM_LOAD( "03-15602-01.u3t",  0x20000, 0x10000, CRC(545b27a1) SHA1(1e8beebc1384cf6513bff7c2381ca214967ff135) )
	ROM_LOAD( "03-15603-01.u4t",  0x30000, 0x10000, CRC(cdc9c09d) SHA1(8641312638507d027948c17c042417b0d0362714) )
	ROM_LOAD( "03-15604-01.u5t",  0x40000, 0x10000, CRC(3c03e92e) SHA1(7cd9b02bbf1d30a8432632d902c4ea6c8108210b) )
	ROM_LOAD( "03-15605-01.u6t",  0x50000, 0x10000, CRC(cdf7d19c) SHA1(577c8bf5964d77dbfef4840c53ae40cda68bf4f3) )
	ROM_LOAD( "03-15606-01.u7t",  0x60000, 0x10000, CRC(8eeb007c) SHA1(6f9d4132c7e5e6502108cb3e8eab9114f07848b4) )
	ROM_LOAD( "03-15607-01.u8t",  0x70000, 0x10000, CRC(57cb6d2d) SHA1(56e364aedca25935a5cd7ab4460d9213fcc58b4a) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "03-15623-01.u25t", 0x040001, 0x10000, CRC(710bdc76) SHA1(610f7baa17adf2d16c9494b05556b49ae376fe81) )
	ROM_LOAD16_BYTE( "03-15620-01.u13t", 0x040000, 0x10000, CRC(7e5cb8ad) SHA1(aaff4e93053638955b95951dceea3b35e842e80f) )
	ROM_LOAD16_BYTE( "03-15624-01.u26t", 0x060001, 0x10000, CRC(dd090d33) SHA1(09a3fa4fa3a50c6692be2bc5fec2c4e9a5072d5d) )
	ROM_LOAD16_BYTE( "03-15621-01.u14t", 0x060000, 0x10000, CRC(f68c68c9) SHA1(a7d77c36831d455a8c36d2156460287cf28c9694) )
	ROM_LOAD16_BYTE( "03-15625-01.u27t", 0x0e0001, 0x10000, CRC(ac442523) SHA1(d05dcc413eb39b0938890ef80ec7b636773bb1a3) )
	ROM_LOAD16_BYTE( "03-15622-01.u15t", 0x0e0000, 0x10000, CRC(9e84509a) SHA1(4c3a3e5192ba6c38d8391eedf817350795bddb8f) )

	ROM_REGION( 0x0c000, "bg_gfx", 0 )
	ROM_LOAD( "03-15615-01.u93", 0x00000, 0x04000, CRC(a7ea6a87) SHA1(5cfd9ed6a5ffc8e86d18b7d8496761b9086b6368) )
	ROM_LOAD( "03-15616-01.u94", 0x04000, 0x04000, CRC(4a9b3900) SHA1(00398cc5056c999673604e414c9c0338d83b13d4) )
	ROM_LOAD( "03-15617-01.u95", 0x08000, 0x04000, CRC(2cd95edb) SHA1(939ff97562535b05f427186b085a74a8fe5a332a) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	ROM_LOAD( "03-15611-01.u70",  0x00000, 0x4000, CRC(bf2695fb) SHA1(58a6d1e9e83912f7567eabdf54278db85061c284) )
	ROM_LOAD( "03-15614-01.u92",  0x04000, 0x4000, CRC(c93fd870) SHA1(1086334496a4d1900a2d697cbd2575a77df89d65) )
	ROM_LOAD( "03-15610-01.u69",  0x08000, 0x4000, CRC(3e5b786f) SHA1(13d2ab7b6a1182933272b597718d3e715b547a10) )
	// U91 = Empty
	ROM_LOAD( "03-15609-01.u68",  0x10000, 0x4000, CRC(0319aec7) SHA1(e4f14ce9b4712c1cee69141165d187e9068101fc) )
	ROM_LOAD( "03-15613-01.u90",  0x14000, 0x4000, CRC(4805802e) SHA1(a121aec2b0340773288687baccf85519c0ef3160) )
	ROM_LOAD( "03-15608-01.u67",  0x18000, 0x4000, CRC(78f0fd2b) SHA1(e83b1106411bb03c64a985a08c5f20c2eb397140) )
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-teamqb.bin", 0x0000, 0x0080, CRC(119fb1e7) SHA1(1a3be0e2415c789a517b21c99e6d60d3ac18a092) )
ROM_END

ROM_START( teamqb2 )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "03-15618-03.u58t",   0x00000, 0x10000, CRC(b32568dc) SHA1(92fb8dd89cc7838129e7b106bc0e35107372904f) )
	ROM_LOAD( "03-15619-02.u59t",   0x10000, 0x10000, CRC(6d533714) SHA1(ab177aaa5b034250c85bde0c2441902f72d44f42) )

	ROM_REGION( 0x80000, "slave", 0 )
	ROM_LOAD( "03-15600-01.u3",   0x00000, 0x02000, CRC(46615844) SHA1(cb73dd73d389d1f6a5da91d0935b5461649ba706) )
	ROM_LOAD( "03-15601-01.u2t",  0x10000, 0x10000, CRC(8e523c58) SHA1(7f1133144177c39849fd6355bf9250895b2d597f) )
	ROM_LOAD( "03-15602-01.u3t",  0x20000, 0x10000, CRC(545b27a1) SHA1(1e8beebc1384cf6513bff7c2381ca214967ff135) )
	ROM_LOAD( "03-15603-01.u4t",  0x30000, 0x10000, CRC(cdc9c09d) SHA1(8641312638507d027948c17c042417b0d0362714) )
	ROM_LOAD( "03-15604-01.u5t",  0x40000, 0x10000, CRC(3c03e92e) SHA1(7cd9b02bbf1d30a8432632d902c4ea6c8108210b) )
	ROM_LOAD( "03-15605-01.u6t",  0x50000, 0x10000, CRC(cdf7d19c) SHA1(577c8bf5964d77dbfef4840c53ae40cda68bf4f3) )
	ROM_LOAD( "03-15606-01.u7t",  0x60000, 0x10000, CRC(8eeb007c) SHA1(6f9d4132c7e5e6502108cb3e8eab9114f07848b4) )
	ROM_LOAD( "03-15607-01.u8t",  0x70000, 0x10000, CRC(57cb6d2d) SHA1(56e364aedca25935a5cd7ab4460d9213fcc58b4a) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "03-15623-01.u25t", 0x040001, 0x10000, CRC(710bdc76) SHA1(610f7baa17adf2d16c9494b05556b49ae376fe81) )
	ROM_LOAD16_BYTE( "03-15620-01.u13t", 0x040000, 0x10000, CRC(7e5cb8ad) SHA1(aaff4e93053638955b95951dceea3b35e842e80f) )
	ROM_LOAD16_BYTE( "03-15624-01.u26t", 0x060001, 0x10000, CRC(dd090d33) SHA1(09a3fa4fa3a50c6692be2bc5fec2c4e9a5072d5d) )
	ROM_LOAD16_BYTE( "03-15621-01.u14t", 0x060000, 0x10000, CRC(f68c68c9) SHA1(a7d77c36831d455a8c36d2156460287cf28c9694) )
	ROM_LOAD16_BYTE( "03-15625-01.u27t", 0x0e0001, 0x10000, CRC(ac442523) SHA1(d05dcc413eb39b0938890ef80ec7b636773bb1a3) )
	ROM_LOAD16_BYTE( "03-15622-01.u15t", 0x0e0000, 0x10000, CRC(9e84509a) SHA1(4c3a3e5192ba6c38d8391eedf817350795bddb8f) )

	ROM_REGION( 0x0c000, "bg_gfx", 0 )
	ROM_LOAD( "03-15615-01.u93", 0x00000, 0x04000, CRC(a7ea6a87) SHA1(5cfd9ed6a5ffc8e86d18b7d8496761b9086b6368) )
	ROM_LOAD( "03-15616-01.u94", 0x04000, 0x04000, CRC(4a9b3900) SHA1(00398cc5056c999673604e414c9c0338d83b13d4) )
	ROM_LOAD( "03-15617-01.u95", 0x08000, 0x04000, CRC(2cd95edb) SHA1(939ff97562535b05f427186b085a74a8fe5a332a) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	ROM_LOAD( "03-15611-01.u70",  0x00000, 0x4000, CRC(bf2695fb) SHA1(58a6d1e9e83912f7567eabdf54278db85061c284) )
	ROM_LOAD( "03-15614-01.u92",  0x04000, 0x4000, CRC(c93fd870) SHA1(1086334496a4d1900a2d697cbd2575a77df89d65) )
	ROM_LOAD( "03-15610-01.u69",  0x08000, 0x4000, CRC(3e5b786f) SHA1(13d2ab7b6a1182933272b597718d3e715b547a10) )
	// U91 = Empty
	ROM_LOAD( "03-15609-01.u68",  0x10000, 0x4000, CRC(0319aec7) SHA1(e4f14ce9b4712c1cee69141165d187e9068101fc) )
	ROM_LOAD( "03-15613-01.u90",  0x14000, 0x4000, CRC(4805802e) SHA1(a121aec2b0340773288687baccf85519c0ef3160) )
	ROM_LOAD( "03-15608-01.u67",  0x18000, 0x4000, CRC(78f0fd2b) SHA1(e83b1106411bb03c64a985a08c5f20c2eb397140) )
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-teamqb.bin", 0x0000, 0x0080, CRC(119fb1e7) SHA1(1a3be0e2415c789a517b21c99e6d60d3ac18a092) )
ROM_END


ROM_START( aafb )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "03-28011.u58t",   0x00000, 0x10000, CRC(fa75a4a0) SHA1(ff37d71d8f2776a8ae9b15f95f105f282b079c5b) )
	ROM_LOAD( "03-28012.u59t",   0x10000, 0x10000, CRC(ab6a606f) SHA1(6c8872c73b26760517884b2996a0f3834b16b480) )

	ROM_REGION( 0x80000, "slave", 0 )
	ROM_LOAD( "03-28000.u3",      0x00000, 0x02000, CRC(cb531986) SHA1(e3bc3fdd1471719e0489d9990302a267a2cedc23) )
	ROM_LOAD( "02-26001-01.u2t",  0x10000, 0x10000, CRC(f118b9b4) SHA1(95d0ae9055cf60e2d0b414ab64632fed1f48bf99) )
	ROM_LOAD( "02-24002-02.u3t",  0x20000, 0x10000, CRC(bbb92184) SHA1(d9890d1c95fb19e9fff6465c977cabf71e4528b4) )
	ROM_LOAD( "02-15603-01.u4t",  0x30000, 0x10000, CRC(cdc9c09d) SHA1(8641312638507d027948c17c042417b0d0362714) )
	ROM_LOAD( "02-15604-01.u5t",  0x40000, 0x10000, CRC(3c03e92e) SHA1(7cd9b02bbf1d30a8432632d902c4ea6c8108210b) )
	ROM_LOAD( "02-15605-01.u6t",  0x50000, 0x10000, CRC(cdf7d19c) SHA1(577c8bf5964d77dbfef4840c53ae40cda68bf4f3) )
	ROM_LOAD( "02-15606-01.u7t",  0x60000, 0x10000, CRC(8eeb007c) SHA1(6f9d4132c7e5e6502108cb3e8eab9114f07848b4) )
	ROM_LOAD( "03-28002.u8",      0x70000, 0x10000, CRC(c3e09811) SHA1(9b6e036a53000c9bcb104677d9c71743f02fd841) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "02-24019-01.u25", 0x040001, 0x10000, CRC(9e344768) SHA1(7f16d29c52f3d7f0046f414185c4d889f6128597) )
	ROM_LOAD16_BYTE( "02-24016-01.u13", 0x040000, 0x10000, CRC(6997025f) SHA1(5eda3bcae896933385fe97a4e1396ae2da7576cb) )
	ROM_LOAD16_BYTE( "02-24020-01.u26", 0x060001, 0x10000, CRC(0788f2a5) SHA1(75eb1ab00185f8efa71f1d46197b5f6d20d721f2) )
	ROM_LOAD16_BYTE( "02-24017-01.u14", 0x060000, 0x10000, CRC(a48bd721) SHA1(e099074165594a7c289a25c522005db7e9554ca1) )
	ROM_LOAD16_BYTE( "02-24021-01.u27", 0x0e0001, 0x10000, CRC(94081899) SHA1(289eb2f494d1110d169552e8898296e4a47fcb1d) )
	ROM_LOAD16_BYTE( "02-24018-01.u15", 0x0e0000, 0x10000, CRC(76eb6077) SHA1(255731c63f4a846bb01d4203a786eb34a4734e66) )

	ROM_REGION( 0x0c000, "bg_gfx", 0 )
	ROM_LOAD( "03-28008.u93", 0x00000, 0x04000, CRC(68f8addc) SHA1(a52c408e2e9022f96fb766065d7266deb0df2e5f) )
	ROM_LOAD( "03-28009.u94", 0x04000, 0x04000, CRC(669791ac) SHA1(e8b7bdec313ea9d40f89f13499a31f0b125951a8) )
	ROM_LOAD( "03-28010.u95", 0x08000, 0x04000, CRC(bd62aa8a) SHA1(c8a177a11ec94671bb3bd5883b40692495c049a2) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	ROM_LOAD( "03-28005.u70",  0x00000, 0x4000, CRC(5ca6f4e2) SHA1(76c86d432fac27d0b30f38e12d340b013baf0dd4) )
	ROM_LOAD( "03-28007.u92",  0x04000, 0x4000, CRC(1d9e33c2) SHA1(0b05d1dc20eb9dd803056113265ac6a43291711b) )
	ROM_LOAD( "03-28004.u69",  0x08000, 0x4000, CRC(d4b8a471) SHA1(a9940f749a756409da303c1ebbd2382f635e9a3f) )
	// U91 = Empty
	// U68 = Empty
	ROM_LOAD( "03-28006.u90",  0x14000, 0x4000, CRC(e68c8b6e) SHA1(94f2774d1713fadf0e644641bc0226fd03040bf8) )
	ROM_LOAD( "03-28003.u67",  0x18000, 0x4000, CRC(c92f6357) SHA1(07fa8f9e07aafbe844e11cd6f9a0cbe65625dc53) )
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-aafb.bin", 0x0000, 0x0080, CRC(119fb1e7) SHA1(1a3be0e2415c789a517b21c99e6d60d3ac18a092) )
ROM_END

/*
All American Football, the label format is:
------------------------
|(C)1989 Leland Corp.  | -> Copyright & Manufacturer
|P/N 02-24014-02       | -> Part number with revision
|AA FOOTBAL      U58T  | -> Game name & ROM PCB location
------------------------
*/
ROM_START( aafbb )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "24014-01.u58t",   0x00000, 0x10000, CRC(5db4a3d0) SHA1(f759ab16de48562db1640bc5df68be188725aecf) ) // Need to verify - likely 02-24014-01
	ROM_LOAD( "24015-01.u59t",   0x10000, 0x10000, BAD_DUMP CRC(f384f716) SHA1(e689ec6b76bfdf58a059409850c397d407740dda) ) // Need to verify - likely 02-24015-01

	ROM_REGION( 0x80000, "slave", 0 )
	ROM_LOAD( "02-24000-02.u3",   0x00000, 0x02000, CRC(52df0354) SHA1(a39a2538b733e336eac5a1491c42c89fd4f4d1aa) )
	ROM_LOAD( "02-24001-02.u2t",  0x10000, 0x10000, CRC(9b20697d) SHA1(ccb9851e5db4360731f19e5446a0ef9205110860) )
	ROM_LOAD( "02-24002-02.u3t",  0x20000, 0x10000, CRC(bbb92184) SHA1(d9890d1c95fb19e9fff6465c977cabf71e4528b4) )
	ROM_LOAD( "02-15603-01.u4t",  0x30000, 0x10000, CRC(cdc9c09d) SHA1(8641312638507d027948c17c042417b0d0362714) )
	ROM_LOAD( "02-15604-01.u5t",  0x40000, 0x10000, CRC(3c03e92e) SHA1(7cd9b02bbf1d30a8432632d902c4ea6c8108210b) )
	ROM_LOAD( "02-15605-01.u6t",  0x50000, 0x10000, CRC(cdf7d19c) SHA1(577c8bf5964d77dbfef4840c53ae40cda68bf4f3) )
	ROM_LOAD( "02-15606-01.u7t",  0x60000, 0x10000, CRC(8eeb007c) SHA1(6f9d4132c7e5e6502108cb3e8eab9114f07848b4) )
	ROM_LOAD( "02-24002-02.u8t",  0x70000, 0x10000, CRC(3d9747c9) SHA1(4624ac39ff5336b0fd8c70bf35685041d5c38b1c) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "02-24019-01.u25", 0x040001, 0x10000, CRC(9e344768) SHA1(7f16d29c52f3d7f0046f414185c4d889f6128597) )
	ROM_LOAD16_BYTE( "02-24016-01.u13", 0x040000, 0x10000, CRC(6997025f) SHA1(5eda3bcae896933385fe97a4e1396ae2da7576cb) )
	ROM_LOAD16_BYTE( "02-24020-01.u26", 0x060001, 0x10000, CRC(0788f2a5) SHA1(75eb1ab00185f8efa71f1d46197b5f6d20d721f2) )
	ROM_LOAD16_BYTE( "02-24017-01.u14", 0x060000, 0x10000, CRC(a48bd721) SHA1(e099074165594a7c289a25c522005db7e9554ca1) )
	ROM_LOAD16_BYTE( "02-24021-01.u27", 0x0e0001, 0x10000, CRC(94081899) SHA1(289eb2f494d1110d169552e8898296e4a47fcb1d) )
	ROM_LOAD16_BYTE( "02-24018-01.u15", 0x0e0000, 0x10000, CRC(76eb6077) SHA1(255731c63f4a846bb01d4203a786eb34a4734e66) )

	ROM_REGION( 0x18000, "bg_gfx", 0 )
	ROM_LOAD( "02-24011-02.u93", 0x00000, 0x08000, CRC(71f4425b) SHA1(074c79d709bf9e927f538932ef05b13e5e649197) )
	ROM_LOAD( "02-24012-02.u94", 0x08000, 0x08000, CRC(b2499547) SHA1(cf5979e56cc307133cbdbfdba448cdf3087eaf8c) )
	ROM_LOAD( "02-24013-02.u95", 0x10000, 0x08000, CRC(0a604e0d) SHA1(08917c3e9fb408b8e128fe2e3617c8c17d964d66) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	ROM_LOAD( "02-24007-01.u70",  0x00000, 0x4000, CRC(40e46aa4) SHA1(e8a27c9007218906683eac29affdd748f64cc6e6) )
	ROM_LOAD( "02-24010-01.u92",  0x04000, 0x4000, CRC(78705f42) SHA1(4b941df0690a8ce4e390b0488a7ce7e083f52ff3) )
	ROM_LOAD( "02-24006-01.u69",  0x08000, 0x4000, CRC(6a576aa9) SHA1(8849929c66012de6d2d8c1b4faefe71f11133aac) )
	ROM_LOAD( "02-24009-02.u91",  0x0c000, 0x4000, CRC(b857a1ad) SHA1(40aeb6afb115af14530177f05100b7cf4baf330a) )
	ROM_LOAD( "02-24005-02.u68",  0x10000, 0x4000, CRC(8ea75319) SHA1(8651346030e51f19bd77d0ddd76a2544e951b12e) )
	ROM_LOAD( "02-24008-01.u90",  0x14000, 0x4000, CRC(4538bc58) SHA1(a568e392771398f60b2aa0f83425935fc7198f72) )
	ROM_LOAD( "02-24004-02.u67",  0x18000, 0x4000, CRC(cd7a3338) SHA1(c91d277578ad9d039f2febdd15d977d7259e5fc8) )
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-aafb.bin", 0x0000, 0x0080, CRC(119fb1e7) SHA1(1a3be0e2415c789a517b21c99e6d60d3ac18a092) )
ROM_END

ROM_START( aafbc )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "02-24014-02.u58t",   0x00000, 0x10000, CRC(25cc4ccc) SHA1(0fe02e23942a10bb9a46524e75705f10fbb0a79a) ) // 02-24014-02 verified as rev C
	ROM_LOAD( "02-24015-02.u59t",   0x10000, 0x10000, CRC(bfa1b56f) SHA1(b5dba27bfcd47cfeebdcf99e9d5f978d5d7f4fb3) ) // 02-24015-02 verified as rev C

	ROM_REGION( 0x80000, "slave", 0 )
	ROM_LOAD( "02-24000-02.u3",   0x00000, 0x02000, CRC(52df0354) SHA1(a39a2538b733e336eac5a1491c42c89fd4f4d1aa) )
	ROM_LOAD( "02-24001-02.u2t",  0x10000, 0x10000, CRC(9b20697d) SHA1(ccb9851e5db4360731f19e5446a0ef9205110860) )
	ROM_LOAD( "02-24002-02.u3t",  0x20000, 0x10000, CRC(bbb92184) SHA1(d9890d1c95fb19e9fff6465c977cabf71e4528b4) )
	ROM_LOAD( "02-15603-01.u4t",  0x30000, 0x10000, CRC(cdc9c09d) SHA1(8641312638507d027948c17c042417b0d0362714) )
	ROM_LOAD( "02-15604-01.u5t",  0x40000, 0x10000, CRC(3c03e92e) SHA1(7cd9b02bbf1d30a8432632d902c4ea6c8108210b) )
	ROM_LOAD( "02-15605-01.u6t",  0x50000, 0x10000, CRC(cdf7d19c) SHA1(577c8bf5964d77dbfef4840c53ae40cda68bf4f3) )
	ROM_LOAD( "02-15606-01.u7t",  0x60000, 0x10000, CRC(8eeb007c) SHA1(6f9d4132c7e5e6502108cb3e8eab9114f07848b4) )
	ROM_LOAD( "02-24002-02.u8t",  0x70000, 0x10000, CRC(3d9747c9) SHA1(4624ac39ff5336b0fd8c70bf35685041d5c38b1c) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "02-24019-01.u25", 0x040001, 0x10000, CRC(9e344768) SHA1(7f16d29c52f3d7f0046f414185c4d889f6128597) )
	ROM_LOAD16_BYTE( "02-24016-01.u13", 0x040000, 0x10000, CRC(6997025f) SHA1(5eda3bcae896933385fe97a4e1396ae2da7576cb) )
	ROM_LOAD16_BYTE( "02-24020-01.u26", 0x060001, 0x10000, CRC(0788f2a5) SHA1(75eb1ab00185f8efa71f1d46197b5f6d20d721f2) )
	ROM_LOAD16_BYTE( "02-24017-01.u14", 0x060000, 0x10000, CRC(a48bd721) SHA1(e099074165594a7c289a25c522005db7e9554ca1) )
	ROM_LOAD16_BYTE( "02-24021-01.u27", 0x0e0001, 0x10000, CRC(94081899) SHA1(289eb2f494d1110d169552e8898296e4a47fcb1d) )
	ROM_LOAD16_BYTE( "02-24018-01.u15", 0x0e0000, 0x10000, CRC(76eb6077) SHA1(255731c63f4a846bb01d4203a786eb34a4734e66) )

	ROM_REGION( 0x18000, "bg_gfx", 0 )
	ROM_LOAD( "02-24011-02.u93", 0x00000, 0x08000, CRC(71f4425b) SHA1(074c79d709bf9e927f538932ef05b13e5e649197) )
	ROM_LOAD( "02-24012-02.u94", 0x08000, 0x08000, CRC(b2499547) SHA1(cf5979e56cc307133cbdbfdba448cdf3087eaf8c) )
	ROM_LOAD( "02-24013-02.u95", 0x10000, 0x08000, CRC(0a604e0d) SHA1(08917c3e9fb408b8e128fe2e3617c8c17d964d66) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	ROM_LOAD( "02-24007-01.u70",  0x00000, 0x4000, CRC(40e46aa4) SHA1(e8a27c9007218906683eac29affdd748f64cc6e6) )
	ROM_LOAD( "02-24010-01.u92",  0x04000, 0x4000, CRC(78705f42) SHA1(4b941df0690a8ce4e390b0488a7ce7e083f52ff3) )
	ROM_LOAD( "02-24006-01.u69",  0x08000, 0x4000, CRC(6a576aa9) SHA1(8849929c66012de6d2d8c1b4faefe71f11133aac) )
	ROM_LOAD( "02-24009-02.u91",  0x0c000, 0x4000, CRC(b857a1ad) SHA1(40aeb6afb115af14530177f05100b7cf4baf330a) )
	ROM_LOAD( "02-24005-02.u68",  0x10000, 0x4000, CRC(8ea75319) SHA1(8651346030e51f19bd77d0ddd76a2544e951b12e) )
	ROM_LOAD( "02-24008-01.u90",  0x14000, 0x4000, CRC(4538bc58) SHA1(a568e392771398f60b2aa0f83425935fc7198f72) )
	ROM_LOAD( "02-24004-02.u67",  0x18000, 0x4000, CRC(cd7a3338) SHA1(c91d277578ad9d039f2febdd15d977d7259e5fc8) )
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-aafb.bin", 0x0000, 0x0080, CRC(119fb1e7) SHA1(1a3be0e2415c789a517b21c99e6d60d3ac18a092) )
ROM_END

ROM_START( aafbd2p )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "02-26014-01.u58t", 0x00000, 0x10000, CRC(79fd14cd) SHA1(1dd75bcecd51d414194ca19381bee0b9f70a8007) )
	ROM_LOAD( "02-26015-01.u59t", 0x10000, 0x10000, CRC(3b0382f0) SHA1(1b01af999201f202e76da8e445ff986d096103cd) )

	ROM_REGION( 0x80000, "slave", 0 )
	ROM_LOAD( "02-26000-01.u3",   0x00000, 0x02000, CRC(98c06c63) SHA1(91922f7e2d4f30018ee7302f3d9b7b6793b43dba) )
	ROM_LOAD( "02-26001-01.u2t",  0x10000, 0x10000, CRC(f118b9b4) SHA1(95d0ae9055cf60e2d0b414ab64632fed1f48bf99) )
	ROM_LOAD( "02-24002-02.u3t",  0x20000, 0x10000, CRC(bbb92184) SHA1(d9890d1c95fb19e9fff6465c977cabf71e4528b4) )
	ROM_LOAD( "02-15603-01.u4t",  0x30000, 0x10000, CRC(cdc9c09d) SHA1(8641312638507d027948c17c042417b0d0362714) )
	ROM_LOAD( "02-15604-01.u5t",  0x40000, 0x10000, CRC(3c03e92e) SHA1(7cd9b02bbf1d30a8432632d902c4ea6c8108210b) )
	ROM_LOAD( "02-15605-01.u6t",  0x50000, 0x10000, CRC(cdf7d19c) SHA1(577c8bf5964d77dbfef4840c53ae40cda68bf4f3) )
	ROM_LOAD( "02-15606-01.u7t",  0x60000, 0x10000, CRC(8eeb007c) SHA1(6f9d4132c7e5e6502108cb3e8eab9114f07848b4) )
	ROM_LOAD( "02-24002-02.u8t",  0x70000, 0x10000, CRC(3d9747c9) SHA1(4624ac39ff5336b0fd8c70bf35685041d5c38b1c) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "02-24019-01.u25", 0x040001, 0x10000, CRC(9e344768) SHA1(7f16d29c52f3d7f0046f414185c4d889f6128597) )
	ROM_LOAD16_BYTE( "02-24016-01.u13", 0x040000, 0x10000, CRC(6997025f) SHA1(5eda3bcae896933385fe97a4e1396ae2da7576cb) )
	ROM_LOAD16_BYTE( "02-24020-01.u26", 0x060001, 0x10000, CRC(0788f2a5) SHA1(75eb1ab00185f8efa71f1d46197b5f6d20d721f2) )
	ROM_LOAD16_BYTE( "02-24017-01.u14", 0x060000, 0x10000, CRC(a48bd721) SHA1(e099074165594a7c289a25c522005db7e9554ca1) )
	ROM_LOAD16_BYTE( "02-24021-01.u27", 0x0e0001, 0x10000, CRC(94081899) SHA1(289eb2f494d1110d169552e8898296e4a47fcb1d) )
	ROM_LOAD16_BYTE( "02-24018-01.u15", 0x0e0000, 0x10000, CRC(76eb6077) SHA1(255731c63f4a846bb01d4203a786eb34a4734e66) )

	ROM_REGION( 0x18000, "bg_gfx", 0 )
	ROM_LOAD( "02-24011-02.u93", 0x00000, 0x08000, CRC(71f4425b) SHA1(074c79d709bf9e927f538932ef05b13e5e649197) )
	ROM_LOAD( "02-24012-02.u94", 0x08000, 0x08000, CRC(b2499547) SHA1(cf5979e56cc307133cbdbfdba448cdf3087eaf8c) )
	ROM_LOAD( "02-24013-02.u95", 0x10000, 0x08000, CRC(0a604e0d) SHA1(08917c3e9fb408b8e128fe2e3617c8c17d964d66) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	ROM_LOAD( "02-24007-01.u70",  0x00000, 0x4000, CRC(40e46aa4) SHA1(e8a27c9007218906683eac29affdd748f64cc6e6) )
	ROM_LOAD( "02-24010-01.u92",  0x04000, 0x4000, CRC(78705f42) SHA1(4b941df0690a8ce4e390b0488a7ce7e083f52ff3) )
	ROM_LOAD( "02-24006-01.u69",  0x08000, 0x4000, CRC(6a576aa9) SHA1(8849929c66012de6d2d8c1b4faefe71f11133aac) )
	ROM_LOAD( "02-24009-02.u91",  0x0c000, 0x4000, CRC(b857a1ad) SHA1(40aeb6afb115af14530177f05100b7cf4baf330a) )
	ROM_LOAD( "02-24005-02.u68",  0x10000, 0x4000, CRC(8ea75319) SHA1(8651346030e51f19bd77d0ddd76a2544e951b12e) )
	ROM_LOAD( "02-24008-01.u90",  0x14000, 0x4000, CRC(4538bc58) SHA1(a568e392771398f60b2aa0f83425935fc7198f72) )
	ROM_LOAD( "02-24004-02.u67",  0x18000, 0x4000, CRC(cd7a3338) SHA1(c91d277578ad9d039f2febdd15d977d7259e5fc8) )
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-aafb.bin", 0x0000, 0x0080, CRC(119fb1e7) SHA1(1a3be0e2415c789a517b21c99e6d60d3ac18a092) )
ROM_END

/*
For Ironman Ivan Stewart's Super Off-Road, the label format is:
------------------------
|(C)1989 Leland Corp.  | -> Copyright & Manufacturer
|P/N 03-22121-04       | -> Part number with revision
|SUPER OFF-RD    U58T  | -> Game name & ROM PCB location
------------------------
*/
ROM_START( offroad )
	ROM_REGION( 0x40000, "master", 0 )
	ROM_LOAD( "03-22121-04.u58t",   0x00000, 0x10000, CRC(c5790988) SHA1(a6bae6b024d86b49a23805037b77d15a3c7913ef) )
	ROM_LOAD( "03-22122-03.u59t",   0x10000, 0x10000, CRC(ae862fdc) SHA1(ac31630cff5850409f87bfa5a7303eeedf8a895d) )
	ROM_LOAD( "03-22120-01.u57t",   0x20000, 0x10000, CRC(e9f0f175) SHA1(db8c55015d1e8230f1fb27dfac6b8b364b0718a2) )
	ROM_LOAD( "03-22119-02.u56t",   0x30000, 0x10000, CRC(38642f22) SHA1(9167bbc7ed8a8a0b913ead3b8b5a7749a29f15cb) )

	ROM_REGION( 0x80000, "slave", 0 )
	ROM_LOAD( "03-22100-02.u3",   0x00000, 0x02000, CRC(08c96a4b) SHA1(7d93dd918a5733c190b2668811d161f5f6339cf0) )
	ROM_LOAD( "03-22108-02.u4t",  0x30000, 0x10000, CRC(0d72780a) SHA1(634b87e7afff4ac2e8e3b98554364c5f3c4d9176) )
	ROM_LOAD( "03-22109-02.u5t",  0x40000, 0x10000, CRC(5429ce2c) SHA1(73e543796629ac719928f4fe48442f1975db5092) )
	ROM_LOAD( "03-22110-02.u6t",  0x50000, 0x10000, CRC(f97bad5c) SHA1(c68f8022c86bfc5c0480e5ce426fe2f985dc255f) )
	ROM_LOAD( "03-22111-01.u7t",  0x60000, 0x10000, CRC(f79157a1) SHA1(a5731aa92f805123cb00c6ef93a0aed3dc84dae4) )
	ROM_LOAD( "03-22112-01.u8t",  0x70000, 0x10000, CRC(3eef38d3) SHA1(9131960592a44c8567ab483f72955d2cc8898445) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "03-22116-03.u25t", 0x040001, 0x10000, CRC(95bb31d3) SHA1(e7bc43b63126fd33663865b2e41bacc58e962628) )
	ROM_LOAD16_BYTE( "03-22113-03.u13t", 0x040000, 0x10000, CRC(71b28df6) SHA1(caf8e4c98a1650dbaedf83f4d38da920d0976f78) )
	ROM_LOAD16_BYTE( "03-22117-03.u26t", 0x060001, 0x10000, CRC(703d81ce) SHA1(caf5363fb468a461a260e0ec636b0a7a8dc9cd3d) )
	ROM_LOAD16_BYTE( "03-22114-03.u14t", 0x060000, 0x10000, CRC(f8b31bf8) SHA1(cb8133effe5484c5b4c40b77769f6ec72441c333) )
	ROM_LOAD16_BYTE( "03-22118-03.u27t", 0x0e0001, 0x10000, CRC(806ccf8b) SHA1(7335a85fc84d5c2f7537548c3856c9cd2f267609) )
	ROM_LOAD16_BYTE( "03-22115-03.u15t", 0x0e0000, 0x10000, CRC(c8439a7a) SHA1(9a8bb1fca8d3414dcfd4839bc0c4289e4d810943) )

	ROM_REGION( 0x18000, "bg_gfx", 0 )
	ROM_LOAD( "03-22105-02.u93", 0x00000, 0x08000, CRC(4426e367) SHA1(298203112d724feb9a75a7bfc34b3dbb4d7fffe7) )
	ROM_LOAD( "03-22106-02.u94", 0x08000, 0x08000, CRC(687dc1fc) SHA1(876c72561d942ebc5f3a148d3d3efdceb39c9e2e) )
	ROM_LOAD( "03-22107-02.u95", 0x10000, 0x08000, CRC(cee6ee5f) SHA1(3f1c6e8d9eb9de207cabca7c9d6d8d633bd69681) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	// U70 = Empty
	ROM_LOAD( "03-22104-01.u92",  0x04000, 0x4000, CRC(03e0497d) SHA1(bffd870251d51cce262961b77f1953f360f9607b) )
	ROM_LOAD( "03-22102-01.u69",  0x08000, 0x4000, CRC(c3f2e443) SHA1(82f22dabc99b3aaa94acaa303735a155ac13e592) )
	// U91 = Empty
	// U68 = Empty
	ROM_LOAD( "03-22103-02.u90",  0x14000, 0x4000, CRC(2266757a) SHA1(22aaf4b14f11198ffd14c9830c7997fd47ee14b6) )
	ROM_LOAD( "03-22101-02.u67",  0x18000, 0x4000, CRC(ecab0527) SHA1(6bbf8243d9b2ea775897719592212b51998f1b01) )
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-offroad.bin", 0x0000, 0x0080, CRC(57955248) SHA1(d8cc09ff57c9de01c1b247a3f1d4555fcb0c2e45) )
ROM_END

ROM_START( offroad3 )
	ROM_REGION( 0x40000, "master", 0 )
	ROM_LOAD( "03-22121-03.u58t",   0x00000, 0x10000, CRC(8e3d0da1) SHA1(777f0c35f46d0c341ea535b36077e198da82d3c2) )
	ROM_LOAD( "03-22122-02.u59t",   0x10000, 0x10000, CRC(37b8a0bd) SHA1(676cee4d9feb217ca6fd9eb500478802a7ea0343) )
	ROM_LOAD( "03-22120-01.u57t",   0x20000, 0x10000, CRC(e9f0f175) SHA1(db8c55015d1e8230f1fb27dfac6b8b364b0718a2) )
	ROM_LOAD( "03-22119-01.u56t",   0x30000, 0x10000, CRC(e7d144a8) SHA1(04befee17aa7e941c68d5f42356f370879ce4e51) )

	ROM_REGION( 0x80000, "slave", 0 )
	ROM_LOAD( "03-22100-01.u3",   0x00000, 0x02000, CRC(7c959be9) SHA1(3073ba8e282f82983e95807f63f82d714a26c6f2) )
	ROM_LOAD( "03-22108-01.u4t",  0x30000, 0x10000, CRC(7219f9ed) SHA1(5b0e7e8b92d6e4d4bf1af5bf1d6e3be36e0e410e) )
	ROM_LOAD( "03-22109-01.u5t",  0x40000, 0x10000, CRC(3d670117) SHA1(c5bf531605a49b991a5533c54698d60a2503bab2) )
	ROM_LOAD( "03-22110-01.u6t",  0x50000, 0x10000, CRC(22b4dc64) SHA1(7f07c42715f1277b2468649e2c7d660237349645) )
	ROM_LOAD( "03-22111-01.u7t",  0x60000, 0x10000, CRC(f79157a1) SHA1(a5731aa92f805123cb00c6ef93a0aed3dc84dae4) )
	ROM_LOAD( "03-22112-01.u8t",  0x70000, 0x10000, CRC(3eef38d3) SHA1(9131960592a44c8567ab483f72955d2cc8898445) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "03-22116-02.u25t", 0x040001, 0x10000, CRC(1d906569) SHA1(070641606eda45b2b3b8e20db37c8a35eba3674f) )
	ROM_LOAD16_BYTE( "03-22113-02.u13t", 0x040000, 0x10000, CRC(de8a047b) SHA1(9cddcb809346737f4ce4d3c9943b4bc502321e27) )
	ROM_LOAD16_BYTE( "03-22117-02.u26t", 0x060001, 0x10000, CRC(45f18abd) SHA1(9d4ef37398d0a080f2b7daeec1be37b5564983a8) )
	ROM_LOAD16_BYTE( "03-22114-02.u14t", 0x060000, 0x10000, CRC(a0257c5a) SHA1(92f794fd4f92f1c1f61051fce437f5e715c506ff) )
	ROM_LOAD16_BYTE( "03-22118-02.u27t", 0x0e0001, 0x10000, CRC(fa62ad6f) SHA1(6361b437a3e8c6816e90d3ecb473a52aa75e638c) )
	ROM_LOAD16_BYTE( "03-22115-02.u15t", 0x0e0000, 0x10000, CRC(448648ae) SHA1(d9600fe080e10a7e6ebf7e83a1fe89c6047549a5) )

	ROM_REGION( 0x18000, "bg_gfx", 0 )
	ROM_LOAD( "03-22105-01.u93", 0x00000, 0x08000, CRC(e8af0080) SHA1(dc8b20dee75f0d35a63631dec40d1f463213004a) )
	ROM_LOAD( "03-22106-01.u94", 0x08000, 0x08000, CRC(858f27b3) SHA1(756e19892961a177549789198daaffb1d10be75c) )
	ROM_LOAD( "03-22107-01.u95", 0x10000, 0x08000, CRC(48059b20) SHA1(a76bd28ddf1a5236455f2b18da82b10f22c19827) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	// U70 = Empty
	ROM_LOAD( "03-22104-01.u92",  0x04000, 0x4000, CRC(03e0497d) SHA1(bffd870251d51cce262961b77f1953f360f9607b) )
	ROM_LOAD( "03-22102-01.u69",  0x08000, 0x4000, CRC(c3f2e443) SHA1(82f22dabc99b3aaa94acaa303735a155ac13e592) )
	// U91 = Empty
	// U68 = Empty
	ROM_LOAD( "03-22103-01.u90",  0x14000, 0x4000, CRC(0ad8c061) SHA1(b55bdb75a5eb73448ae2c730db6380ec144188e2) )
	ROM_LOAD( "03-22101-01.u67",  0x18000, 0x4000, CRC(19b46990) SHA1(fbf1323d1eab8564ab7b0b0bd369809cca26c38a) )
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-offroad.bin", 0x0000, 0x0080, CRC(57955248) SHA1(d8cc09ff57c9de01c1b247a3f1d4555fcb0c2e45) )
ROM_END

/*
For Ironman Ivan Stewart's Super Off-Road Track-Pak, the label format is:
------------------------
|(C)1989 Leland Corp.  | -> Copyright & Manufacturer
|P/N 03-25015-01       | -> Part number with revision
|TRACK PAK       U56T  | -> Game name & ROM PCB location
------------------------
*/
ROM_START( offroadt )
	ROM_REGION( 0x040000, "master", 0 )
	ROM_LOAD( "03-25016-0x.u58t",  0x00000, 0x10000, CRC(adbc6211) SHA1(cb3181a0dd64754d9a65a7a557e4a183b8d539a2) ) // likely rev 4, like the offroadt2p & offroad sets
	ROM_LOAD( "03-25017-0x.u59t",  0x10000, 0x10000, CRC(296dd3b6) SHA1(01ae1f2976e2fecc8237fc7b4cf4fb86dd170a70) ) // likely rev 3, like the offroadt2p & offroad sets
	ROM_LOAD( "03-22120-01.u57t",  0x20000, 0x10000, CRC(e9f0f175) SHA1(db8c55015d1e8230f1fb27dfac6b8b364b0718a2) )
	ROM_LOAD( "03-25015-01.u56t",  0x30000, 0x10000, CRC(2c1a22b3) SHA1(fb18af5ec873968beab47d163d9ef23532c40771) )

	ROM_REGION( 0x90000, "slave", 0 )
	ROM_LOAD( "03-25000-01.u3",   0x00000, 0x02000, CRC(95abb9f1) SHA1(98e9e8f388047d6992a664ae87c50ca65a5db0b1) )
	ROM_LOAD( "03-25001-01.u2t",  0x10000, 0x10000, CRC(c46c1627) SHA1(1e911bc774cbc0a66b2feb68b600aa5ad272daa6) )
	ROM_LOAD( "03-25002-01.u3t",  0x20000, 0x10000, CRC(2276546f) SHA1(d19335504a71ccf74864c8e9896347709bf794e4) )
	ROM_LOAD( "03-25003-01.u4t",  0x30000, 0x10000, CRC(aa4b5975) SHA1(7d695957c283aae4c7e6fb90dab117add65571b4) )
	ROM_LOAD( "03-25004-01.u5t",  0x40000, 0x10000, CRC(69100b06) SHA1(c25d1273d08fd20651d1873ce412bb1e18eff06f) )
	ROM_LOAD( "03-25005-01.u6t",  0x50000, 0x10000, CRC(b75015b8) SHA1(2bb6b4422e087502cfeb9bce0d3e3ffe18192fe0) )
	ROM_LOAD( "03-25006-01.u7t",  0x60000, 0x10000, CRC(a5af5b4f) SHA1(e4992bfbf628d034a879bf9317377348ee4c24e9) )
	ROM_LOAD( "03-25007-01.u8t",  0x70000, 0x10000, CRC(0f735078) SHA1(cb59b11fbed672cb372759384e5916418e6c3dc7) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "03-25021-01.u25t", 0x040001, 0x10000, CRC(f952f800) SHA1(0f1fc837b0b5f5495a666b0a42adb6068e58a57a) )
	ROM_LOAD16_BYTE( "03-25018-01.u13t", 0x040000, 0x10000, CRC(7beec9fc) SHA1(b03b4a28217a8c7c02dc0314db97fef1d4ab6f20) )
	ROM_LOAD16_BYTE( "03-25022-01.u26t", 0x060001, 0x10000, CRC(6227ea94) SHA1(26384af82f73452b7be8a0eeac9f8a3b464068f6) )
	ROM_LOAD16_BYTE( "03-25019-01.u14t", 0x060000, 0x10000, CRC(0a44331d) SHA1(1a52da64c44bc91c2fc9499d1c41191725f9f2be) )
	ROM_LOAD16_BYTE( "03-25023-01.u27t", 0x0e0001, 0x10000, CRC(b80c5f99) SHA1(6b0657db870fb4e14e20cbd955655d0990dd7bda) )
	ROM_LOAD16_BYTE( "03-25020-01.u15t", 0x0e0000, 0x10000, CRC(2a1a1c3c) SHA1(990328240a2dba7264bb5add5ea8cae2752327d9) )

	ROM_REGION( 0x18000, "bg_gfx", 0 )
	ROM_LOAD( "03-25012-01.u93", 0x00000, 0x08000, CRC(f0c1d8b0) SHA1(aa6e53b56474fa97b17b60ef1123a28442482b80) )
	ROM_LOAD( "03-25013-01.u94", 0x08000, 0x08000, CRC(7460d8c0) SHA1(9e3560056da89108c58b320125deeed0e009d0a8) )
	ROM_LOAD( "03-25014-01.u95", 0x10000, 0x08000, CRC(081ee7a8) SHA1(2b884a8ed4173b64f7890edf9a6954c62b5ba012) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	// U70 = Empty
	ROM_LOAD( "03-25011-01.u92",  0x04000, 0x4000, CRC(f9988e28) SHA1(250071f4a26782266303331ddbef5479cc241220) )
	ROM_LOAD( "03-25009-01.u69",  0x08000, 0x4000, CRC(fe5f8d8f) SHA1(5e520da33f30a594c8f37e8e214d0d257ba5c801) )
	// U91 = Empty
	// U68 = Empty
	ROM_LOAD( "03-25010-01.u90",  0x14000, 0x4000, CRC(bda2ecb1) SHA1(c7a70ed794cf1655aebdf4538ab25f74be38cda3) )
	ROM_LOAD( "03-25008-01.u67",  0x18000, 0x4000, CRC(38c9bf29) SHA1(aa681f0a3eb5d31f2b01116939162d296e113428) )
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-offroad.bin", 0x0000, 0x0080, CRC(57955248) SHA1(d8cc09ff57c9de01c1b247a3f1d4555fcb0c2e45) )
ROM_END

/*
For Ironman Ivan Stewart's Super Off-Road Track-Pak, the label format is:
------------------------
|(C)1989 Leland Corp.  | -> Copyright & Manufacturer
|P/N 03-25015-01       | -> Part number with revision
|TRACK PAK       U56T  | -> Game name & ROM PCB location
------------------------
Two player ROM label format:
------------------------
|(C)1990 Leland Corp.  | -> Copyright & Manufacturer
|P/N 03-27000-04       | -> Part number with revision
|TRACK PAK 2-PLAY U58T | -> Game name & ROM PCB location
------------------------

Known to exist, a version with 03-27000-03 @ U58T & 03-27001-02 @ U59T
*/
ROM_START( offroadt2p )
	ROM_REGION( 0x040000, "master", 0 )
	ROM_LOAD( "03-27000-04.u58t",  0x00000, 0x10000, CRC(8a785b66) SHA1(a9e369d446eec658294c81223399acc99e4198c2) )
	ROM_LOAD( "03-27001-03.u59t",  0x10000, 0x10000, CRC(ff553895) SHA1(8d91b46306fa680ea96ebbb76418d7a82ed19568) )
	ROM_LOAD( "03-22120-01.u57t",  0x20000, 0x10000, CRC(e9f0f175) SHA1(db8c55015d1e8230f1fb27dfac6b8b364b0718a2) )
	ROM_LOAD( "03-25015-01.u56t",  0x30000, 0x10000, CRC(2c1a22b3) SHA1(fb18af5ec873968beab47d163d9ef23532c40771) )

	ROM_REGION( 0x90000, "slave", 0 )
	ROM_LOAD( "03-25000-01.u3",   0x00000, 0x02000, CRC(95abb9f1) SHA1(98e9e8f388047d6992a664ae87c50ca65a5db0b1) )
	ROM_LOAD( "03-25001-01.u2t",  0x10000, 0x10000, CRC(c46c1627) SHA1(1e911bc774cbc0a66b2feb68b600aa5ad272daa6) )
	ROM_LOAD( "03-25002-01.u3t",  0x20000, 0x10000, CRC(2276546f) SHA1(d19335504a71ccf74864c8e9896347709bf794e4) )
	ROM_LOAD( "03-25003-01.u4t",  0x30000, 0x10000, CRC(aa4b5975) SHA1(7d695957c283aae4c7e6fb90dab117add65571b4) )
	ROM_LOAD( "03-25004-01.u5t",  0x40000, 0x10000, CRC(69100b06) SHA1(c25d1273d08fd20651d1873ce412bb1e18eff06f) )
	ROM_LOAD( "03-25005-01.u6t",  0x50000, 0x10000, CRC(b75015b8) SHA1(2bb6b4422e087502cfeb9bce0d3e3ffe18192fe0) )
	ROM_LOAD( "03-25006-01.u7t",  0x60000, 0x10000, CRC(a5af5b4f) SHA1(e4992bfbf628d034a879bf9317377348ee4c24e9) )
	ROM_LOAD( "03-25007-01.u8t",  0x70000, 0x10000, CRC(0f735078) SHA1(cb59b11fbed672cb372759384e5916418e6c3dc7) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "03-25021-01.u25t", 0x040001, 0x10000, CRC(f952f800) SHA1(0f1fc837b0b5f5495a666b0a42adb6068e58a57a) )
	ROM_LOAD16_BYTE( "03-25018-01.u13t", 0x040000, 0x10000, CRC(7beec9fc) SHA1(b03b4a28217a8c7c02dc0314db97fef1d4ab6f20) )
	ROM_LOAD16_BYTE( "03-25022-01.u26t", 0x060001, 0x10000, CRC(6227ea94) SHA1(26384af82f73452b7be8a0eeac9f8a3b464068f6) )
	ROM_LOAD16_BYTE( "03-25019-01.u14t", 0x060000, 0x10000, CRC(0a44331d) SHA1(1a52da64c44bc91c2fc9499d1c41191725f9f2be) )
	ROM_LOAD16_BYTE( "03-25023-01.u27t", 0x0e0001, 0x10000, CRC(b80c5f99) SHA1(6b0657db870fb4e14e20cbd955655d0990dd7bda) )
	ROM_LOAD16_BYTE( "03-25020-01.u15t", 0x0e0000, 0x10000, CRC(2a1a1c3c) SHA1(990328240a2dba7264bb5add5ea8cae2752327d9) )

	ROM_REGION( 0x18000, "bg_gfx", 0 )
	ROM_LOAD( "03-25012-01.u93", 0x00000, 0x08000, CRC(f0c1d8b0) SHA1(aa6e53b56474fa97b17b60ef1123a28442482b80) )
	ROM_LOAD( "03-25013-01.u94", 0x08000, 0x08000, CRC(7460d8c0) SHA1(9e3560056da89108c58b320125deeed0e009d0a8) )
	ROM_LOAD( "03-25014-01.u95", 0x10000, 0x08000, CRC(081ee7a8) SHA1(2b884a8ed4173b64f7890edf9a6954c62b5ba012) )

	ROM_REGION( 0x20000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	// U70 = Empty
	ROM_LOAD( "03-25011-01.u92",  0x04000, 0x4000, CRC(f9988e28) SHA1(250071f4a26782266303331ddbef5479cc241220) )
	ROM_LOAD( "03-25009-01.u69",  0x08000, 0x4000, CRC(fe5f8d8f) SHA1(5e520da33f30a594c8f37e8e214d0d257ba5c801) )
	// U91 = Empty
	// U68 = Empty
	ROM_LOAD( "03-25010-01.u90",  0x14000, 0x4000, CRC(bda2ecb1) SHA1(c7a70ed794cf1655aebdf4538ab25f74be38cda3) )
	ROM_LOAD( "03-25008-01.u67",  0x18000, 0x4000, CRC(38c9bf29) SHA1(aa681f0a3eb5d31f2b01116939162d296e113428) )
	// U89 = Empty

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-offroad.bin", 0x0000, 0x0080, CRC(57955248) SHA1(d8cc09ff57c9de01c1b247a3f1d4555fcb0c2e45) )
ROM_END


/*
Pig Out: Dine Like a Swine!, the label format is:
------------------------
|(C)1990 Leland Corp.  | -> Copyright & Manufacturer
|P/N 03-29020-01       | -> Part number with revision
|PIGOUT           U58T | -> Game name & ROM PCB location
------------------------
*/
ROM_START( pigout ) // To enter service mode, press Service (F2) + either P1 or P2 Start
	ROM_REGION( 0x040000, "master", 0 )
	ROM_LOAD( "03-29020-0x.u58t", 0x00000, 0x10000, CRC(8fe4b683) SHA1(6f98a4e54a558a642b7193af85823b29ade46919) ) // likely rev 02
	ROM_LOAD( "03-29021-0x.u59t", 0x10000, 0x10000, CRC(ab907762) SHA1(971c34ae42c17aa27880665966dc15a98387bebb) ) // likely rev 02
	ROM_LOAD( "03-29019-01.u57t", 0x20000, 0x10000, CRC(c22be0ff) SHA1(52b76918358046f40ea4b74e53a38d8984125dbb) )

	ROM_REGION( 0x080000, "slave", 0 )
	ROM_LOAD( "03-29000-01.u3",   0x00000, 0x02000, CRC(af213cb7) SHA1(cf31ee09ee3685274f5ce1df954e7e87199e2e80) )
	ROM_LOAD( "03-29001-01.u2t",  0x10000, 0x10000, CRC(b23164c6) SHA1(11edbea7bf54a68cb85df36345f39654d726a7f2) )
	ROM_LOAD( "03-29002-01.u3t",  0x20000, 0x10000, CRC(d93f105f) SHA1(9fe469d674e84209eb55704fd2ad317d11e4caac) )
	ROM_LOAD( "03-29003-01.u4t",  0x30000, 0x10000, CRC(b7c47bfe) SHA1(42b1ce4401e3754f6fb1453ab4a661dc4237770d) )
	ROM_LOAD( "03-29004-01.u5t",  0x40000, 0x10000, CRC(d9b9dfbf) SHA1(a6f663638d9f6e14c1a6a99ca811d1d495664412) )
	ROM_LOAD( "03-29005-01.u6t",  0x50000, 0x10000, CRC(728c7c1a) SHA1(cc3211313a6b3998a0458d3865e3d2a0f9eb8a94) )
	ROM_LOAD( "03-29006-01.u7t",  0x60000, 0x10000, CRC(393bd990) SHA1(d66d3c5c6d97bb983549d5037bd69c481751b9bf) )
	ROM_LOAD( "03-29007-01.u8t",  0x70000, 0x10000, CRC(cb9ffaad) SHA1(f39fb33e5a30619cd3017574739ccace80afbe1f) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "03-29025-01.u25t", 0x040001, 0x10000, CRC(92cd2617) SHA1(88e318f4a41c67fd9e91f013b3c29b6275b69c31) )
	ROM_LOAD16_BYTE( "03-29022-01.u13t", 0x040000, 0x10000, CRC(9448c389) SHA1(7bb0bd49044ba4b302048d2922ed300f799a2efb) )
	ROM_LOAD16_BYTE( "03-29026-01.u26t", 0x060001, 0x10000, CRC(ab57de8f) SHA1(28a366e7441bc85dfb814f7a7797aa704a0277ba) )
	ROM_LOAD16_BYTE( "03-29023-01.u14t", 0x060000, 0x10000, CRC(30678e93) SHA1(6d2c8f5c9de3d016538dc1da99ec0017fefdf35a) )
	ROM_LOAD16_BYTE( "03-29027-01.u27t", 0x0e0001, 0x10000, CRC(37a8156e) SHA1(a0b44b1ba6701daaa26576c6c892fd97ec82d5e3) )
	ROM_LOAD16_BYTE( "03-29024-01.u15t", 0x0e0000, 0x10000, CRC(1c60d58b) SHA1(93f83a231d06cd958d3539a528e6ee6c2d9904ed) )

	ROM_REGION( 0x18000, "bg_gfx", 0 )
	ROM_LOAD( "03-29016-01.u93", 0x000000, 0x08000, CRC(f102a04d) SHA1(3ecc0ab34a5d6e760679dc5fd7d32dd439f797d5) )
	ROM_LOAD( "03-29017-01.u94", 0x008000, 0x08000, CRC(ec63c015) SHA1(10010a17ffda468dbe2940fae6aae49c56e1ad78) )
	ROM_LOAD( "03-29018-01.u95", 0x010000, 0x08000, CRC(ba6e797e) SHA1(135f905b7663026a99fd9aca8e0247a72bf43cdb) )

	ROM_REGION( 0x40000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	ROM_LOAD( "03-29011-01.u70",  0x00000, 0x4000, CRC(7db4eaa1) SHA1(e1ec186a8359b3302071e87577092008065c39de) )
	ROM_LOAD( "03-29015-01.u92",  0x04000, 0x4000, CRC(20fa57bb) SHA1(7e94698a25c5459991f0e99a50e5e98f392cda41) )
	ROM_LOAD( "03-29010-01.u69",  0x08000, 0x4000, CRC(a16886f3) SHA1(48a0cbbea80cc38cd4d5594d3367282690724c59) )
	ROM_LOAD( "03-29014-01.u91",  0x0c000, 0x4000, CRC(482a3581) SHA1(bab1140a5c0a2ff4c3ef076155429e35cbfe2335) )
	ROM_LOAD( "03-29009-01.u68",  0x10000, 0x4000, CRC(7b62a3ed) SHA1(fc707626a3fa78d38f5b2cbe3b8786e8c4382563) )
	ROM_LOAD( "03-29013-01.u90",  0x14000, 0x4000, CRC(9615d710) SHA1(a9b2d2bf4d6edecdc212f5d96eec8095833bee22) )
	ROM_LOAD( "03-29008-01.u67",  0x18000, 0x4000, CRC(af85ce79) SHA1(76e421772dfdf4d27e36aa51993a987883e015b0) )
	ROM_LOAD( "03-29012-01.u89",  0x1c000, 0x4000, CRC(6c874a05) SHA1(a931ba5ac41facfaf32c5e940eb011e780ab234a) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-pigout.bin", 0x0000, 0x0080, CRC(9646fa72) SHA1(80311bd6ba8988afc4ad1aabf3f452266686917f) )

	ROM_REGION( 0x0e00, "top_board_plds", 0 ) // 80-18002-01 (Top Board), all protected
	ROM_LOAD( "35-01.u44", 0x0000, 0x0104, NO_DUMP ) // TIBPAL16L8-25CN
	ROM_LOAD( "36-01.u43", 0x0200, 0x0104, NO_DUMP ) // TIBPAL16R4-25CN
	ROM_LOAD( "37-01.u42", 0x0400, 0x0104, NO_DUMP ) // MMI PAL16R6ACN
	ROM_LOAD( "38-01.u9",  0x0600, 0x00cc, NO_DUMP ) // CPL20L10-25NC
	ROM_LOAD( "39-01.u55", 0x0700, 0x0149, NO_DUMP ) // AMPAL18P8APC
	ROM_LOAD( "40-01.u68", 0x0900, 0x0149, NO_DUMP ) // AMPAL18P8APC
	ROM_LOAD( "41-02.u54", 0x0b00, 0x0104, NO_DUMP ) // TIBPAL16L8-25CN

	ROM_REGION( 0x1400, "bottom_board_plds", 0 ) // 80-10000-01 (Bottom Board), all protected
	ROM_LOAD( "01-01.u96", 0x0000, 0x0104, NO_DUMP ) // MMI PAL16L8ACN
	ROM_LOAD( "02-29.u85", 0x0200, 0x0104, NO_DUMP ) // MMI PAL16L8ACN
	ROM_LOAD( "03-01.u16", 0x0400, 0x0104, NO_DUMP ) // TIBPAL16L8-25
	ROM_LOAD( "04-01.u17", 0x0600, 0x0104, NO_DUMP ) // MMI PAL16L8ACNS
	ROM_LOAD( "05-01.u21", 0x0800, 0x00cc, NO_DUMP ) // MMI PAL20X10ACNS
	ROM_LOAD( "06-01.u19", 0x0900, 0x00cc, NO_DUMP ) // MMI PAL20X10ACNS
	ROM_LOAD( "07-01.u43", 0x0a00, 0x0104, NO_DUMP ) // MMI PAL16R8ACN
	ROM_LOAD( "08-01.u42", 0x0c00, 0x0104, NO_DUMP ) // TIBPAL16R4-25CN
	ROM_LOAD( "09-01.u40", 0x0e00, 0x0104, NO_DUMP ) // MMI PAL16R8ACN
	ROM_LOAD( "10-01.u26", 0x1000, 0x0104, NO_DUMP ) // MMI PAL16R8ACN
	ROM_LOAD( "11-01.u27", 0x1200, 0x0104, NO_DUMP ) // MMI PAL16R8ACNS
ROM_END

ROM_START( pigouta ) // To enter service mode, press & hold Service (F2) then press P2 Start
	ROM_REGION( 0x040000, "master", 0 )
	ROM_LOAD( "03-29020-01.u58t", 0x00000, 0x10000, CRC(6c815982) SHA1(0720b22afd16e9bdc5d4a9e9a0071674ea46d038) )
	ROM_LOAD( "03-29021-01.u59t", 0x10000, 0x10000, CRC(9de7a763) SHA1(9a612730a9d80d84114c1afc4a1887277d1ad5bc) )
	ROM_LOAD( "03-29019-01.u57t", 0x20000, 0x10000, CRC(c22be0ff) SHA1(52b76918358046f40ea4b74e53a38d8984125dbb) )

	ROM_REGION( 0x80000, "slave", 0 )
	ROM_LOAD( "03-29000-01.u3",   0x00000, 0x02000, CRC(af213cb7) SHA1(cf31ee09ee3685274f5ce1df954e7e87199e2e80) )
	ROM_LOAD( "03-29001-01.u2t",  0x10000, 0x10000, CRC(b23164c6) SHA1(11edbea7bf54a68cb85df36345f39654d726a7f2) )
	ROM_LOAD( "03-29002-01.u3t",  0x20000, 0x10000, CRC(d93f105f) SHA1(9fe469d674e84209eb55704fd2ad317d11e4caac) )
	ROM_LOAD( "03-29003-01.u4t",  0x30000, 0x10000, CRC(b7c47bfe) SHA1(42b1ce4401e3754f6fb1453ab4a661dc4237770d) )
	ROM_LOAD( "03-29004-01.u5t",  0x40000, 0x10000, CRC(d9b9dfbf) SHA1(a6f663638d9f6e14c1a6a99ca811d1d495664412) )
	ROM_LOAD( "03-29005-01.u6t",  0x50000, 0x10000, CRC(728c7c1a) SHA1(cc3211313a6b3998a0458d3865e3d2a0f9eb8a94) )
	ROM_LOAD( "03-29006-01.u7t",  0x60000, 0x10000, CRC(393bd990) SHA1(d66d3c5c6d97bb983549d5037bd69c481751b9bf) )
	ROM_LOAD( "03-29007-01.u8t",  0x70000, 0x10000, CRC(cb9ffaad) SHA1(f39fb33e5a30619cd3017574739ccace80afbe1f) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "03-29025-01.u25t", 0x040001, 0x10000, CRC(92cd2617) SHA1(88e318f4a41c67fd9e91f013b3c29b6275b69c31) )
	ROM_LOAD16_BYTE( "03-29022-01.u13t", 0x040000, 0x10000, CRC(9448c389) SHA1(7bb0bd49044ba4b302048d2922ed300f799a2efb) )
	ROM_LOAD16_BYTE( "03-29026-01.u26t", 0x060001, 0x10000, CRC(ab57de8f) SHA1(28a366e7441bc85dfb814f7a7797aa704a0277ba) )
	ROM_LOAD16_BYTE( "03-29023-01.u14t", 0x060000, 0x10000, CRC(30678e93) SHA1(6d2c8f5c9de3d016538dc1da99ec0017fefdf35a) )
	ROM_LOAD16_BYTE( "03-29027-01.u27t", 0x0e0001, 0x10000, CRC(37a8156e) SHA1(a0b44b1ba6701daaa26576c6c892fd97ec82d5e3) )
	ROM_LOAD16_BYTE( "03-29024-01.u15t", 0x0e0000, 0x10000, CRC(1c60d58b) SHA1(93f83a231d06cd958d3539a528e6ee6c2d9904ed) )

	ROM_REGION( 0x18000, "bg_gfx", 0 )
	ROM_LOAD( "03-29016-01.u93", 0x000000, 0x08000, CRC(f102a04d) SHA1(3ecc0ab34a5d6e760679dc5fd7d32dd439f797d5) )
	ROM_LOAD( "03-29017-01.u94", 0x008000, 0x08000, CRC(ec63c015) SHA1(10010a17ffda468dbe2940fae6aae49c56e1ad78) )
	ROM_LOAD( "03-29018-01.u95", 0x010000, 0x08000, CRC(ba6e797e) SHA1(135f905b7663026a99fd9aca8e0247a72bf43cdb) )

	ROM_REGION( 0x40000, "bg_prom", 0 ) // Ordering: 70/92/69/91/68/90/67/89
	ROM_LOAD( "03-29011-01.u70",  0x00000, 0x4000, CRC(7db4eaa1) SHA1(e1ec186a8359b3302071e87577092008065c39de) )
	ROM_LOAD( "03-29015-01.u92",  0x04000, 0x4000, CRC(20fa57bb) SHA1(7e94698a25c5459991f0e99a50e5e98f392cda41) )
	ROM_LOAD( "03-29010-01.u69",  0x08000, 0x4000, CRC(a16886f3) SHA1(48a0cbbea80cc38cd4d5594d3367282690724c59) )
	ROM_LOAD( "03-29014-01.u91",  0x0c000, 0x4000, CRC(482a3581) SHA1(bab1140a5c0a2ff4c3ef076155429e35cbfe2335) )
	ROM_LOAD( "03-29009-01.u68",  0x10000, 0x4000, CRC(7b62a3ed) SHA1(fc707626a3fa78d38f5b2cbe3b8786e8c4382563) )
	ROM_LOAD( "03-29013-01.u90",  0x14000, 0x4000, CRC(9615d710) SHA1(a9b2d2bf4d6edecdc212f5d96eec8095833bee22) )
	ROM_LOAD( "03-29008-01.u67",  0x18000, 0x4000, CRC(af85ce79) SHA1(76e421772dfdf4d27e36aa51993a987883e015b0) )
	ROM_LOAD( "03-29012-01.u89",  0x1c000, 0x4000, CRC(6c874a05) SHA1(a931ba5ac41facfaf32c5e940eb011e780ab234a) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-pigout.bin", 0x0000, 0x0080, CRC(9646fa72) SHA1(80311bd6ba8988afc4ad1aabf3f452266686917f) )

	ROM_REGION( 0x0e00, "top_board_plds", 0 ) // 80-18002-01 (Top Board), all protected
	ROM_LOAD( "35-01.u44", 0x0000, 0x0104, NO_DUMP ) // TIBPAL16L8-25CN
	ROM_LOAD( "36-01.u43", 0x0200, 0x0104, NO_DUMP ) // TIBPAL16R4-25CN
	ROM_LOAD( "37-01.u42", 0x0400, 0x0104, NO_DUMP ) // MMI PAL16R6ACN
	ROM_LOAD( "38-01.u9",  0x0600, 0x00cc, NO_DUMP ) // CPL20L10-25NC
	ROM_LOAD( "39-01.u55", 0x0700, 0x0149, NO_DUMP ) // AMPAL18P8APC
	ROM_LOAD( "40-01.u68", 0x0900, 0x0149, NO_DUMP ) // AMPAL18P8APC
	ROM_LOAD( "41-02.u54", 0x0b00, 0x0104, NO_DUMP ) // TIBPAL16L8-25CN

	ROM_REGION( 0x1400, "bottom_board_plds", 0 ) // 80-10000-01 (Bottom Board), all protected
	ROM_LOAD( "01-01.u96", 0x0000, 0x0104, NO_DUMP ) // MMI PAL16L8ACN
	ROM_LOAD( "02-29.u85", 0x0200, 0x0104, NO_DUMP ) // MMI PAL16L8ACN
	ROM_LOAD( "03-01.u16", 0x0400, 0x0104, NO_DUMP ) // TIBPAL16L8-25
	ROM_LOAD( "04-01.u17", 0x0600, 0x0104, NO_DUMP ) // MMI PAL16L8ACNS
	ROM_LOAD( "05-01.u21", 0x0800, 0x00cc, NO_DUMP ) // MMI PAL20X10ACNS
	ROM_LOAD( "06-01.u19", 0x0900, 0x00cc, NO_DUMP ) // MMI PAL20X10ACNS
	ROM_LOAD( "07-01.u43", 0x0a00, 0x0104, NO_DUMP ) // MMI PAL16R8ACN
	ROM_LOAD( "08-01.u42", 0x0c00, 0x0104, NO_DUMP ) // TIBPAL16R4-25CN
	ROM_LOAD( "09-01.u40", 0x0e00, 0x0104, NO_DUMP ) // MMI PAL16R8ACN
	ROM_LOAD( "10-01.u26", 0x1000, 0x0104, NO_DUMP ) // MMI PAL16R8ACN
	ROM_LOAD( "11-01.u27", 0x1200, 0x0104, NO_DUMP ) // MMI PAL16R8ACNS
ROM_END


/*
For Ataxx, the label format is:
------------------------
|(C)1990 Leland Corp.  | -> Copyright & Manufacturer
|P/N E-302-31005-05    | -> Part number with revision
|ATAXX            U38  | -> Game name & ROM PCB location
------------------------
*/
ROM_START( ataxx )
	ROM_REGION( 0x30000, "master", 0 )
	ROM_LOAD( "e-302-31005-05.u38",   0x00000, 0x20000, CRC(3378937d) SHA1(3c62da7e11b2860c7fe3a35c077cadcf4d0272ca) )
	ROM_RELOAD(                       0x10000, 0x20000 )

	ROM_REGION( 0x60000, "slave", 0 )
	ROM_LOAD( "e-302-31012-01.u111",  0x00000, 0x20000, CRC(9a3297cc) SHA1(1dfa0bacd2f2b18d44bfc2d55c40291c1b142f8f) )
	ROM_LOAD( "e-302-31013-01.u112",  0x20000, 0x20000, CRC(7e7c3e2f) SHA1(a7e31e1f1b09414c40ab9ace5e9bffbdbaee8704) )
	ROM_LOAD( "e-302-31014-01.u113",  0x40000, 0x20000, CRC(8cf3e101) SHA1(672a3a0ca0f5334cf614bc49cbc1ae5ccea54cbe) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "e-302-31003-01.u15", 0x20001, 0x20000, CRC(8bb3233b) SHA1(5131ad78bdf904cde36534e99efa5576fcea25c0) )
	ROM_LOAD16_BYTE( "e-302-31001-01.u1",  0x20000, 0x20000, CRC(728d75f2) SHA1(d9e8e742cc2d536bd62370c1e474c7036e4392bb) )
	ROM_LOAD16_BYTE( "e-302-31004-01.u16", 0x60001, 0x20000, CRC(f2bdff48) SHA1(f34eb16ea180effffd81d637acc3d96bffaf81c9) )
	ROM_RELOAD(                            0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "e-302-31002-01.u2",  0x60000, 0x20000, CRC(ca06a394) SHA1(0858908bd150dd7354536e10b2a386b45f17ac9f) )
	ROM_RELOAD(                            0xc0000, 0x20000 )

	ROM_REGION( 0xc0000, "bg_gfx", 0 )
	ROM_LOAD( "e-302-31006-01.u98",   0x00000, 0x20000, CRC(59d0f2ae) SHA1(8da5dc006e192af98458227e79421b6a07ac1cdc) )
	ROM_LOAD( "e-302-31007-01.u99",   0x20000, 0x20000, CRC(6ab7db25) SHA1(25c2fa23b99ac4bab5a9b851c2087de44512a5c2) )
	ROM_LOAD( "e-302-31008-01.u100",  0x40000, 0x20000, CRC(2352849e) SHA1(f49394b6efb6a87d86516ec0a5ddd582f96f7e5d) )
	ROM_LOAD( "e-302-31009-01.u101",  0x60000, 0x20000, CRC(4c31e02b) SHA1(2d8dd97a2a737bafb44dced7ce3eef22d7d14cbe) )
	ROM_LOAD( "e-302-31010-01.u102",  0x80000, 0x20000, CRC(a951228c) SHA1(7ec5cf4d0aa3702be9236d155bea373a06c0be03) )
	ROM_LOAD( "e-302-31011-01.u103",  0xa0000, 0x20000, CRC(ed326164) SHA1(8706192f525ece200587cee7e7beb4a1975bf63e) )

	ROM_REGION( 0x40000, "xrom", ROMREGION_ERASEFF ) // X-ROM (data used by main processor)
	// Empty / not used

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-ataxx.bin", 0x0000, 0x0100, CRC(989cdb8c) SHA1(13b30a328e71a195960e98e50d1657a8b6860dcf) )
ROM_END

ROM_START( ataxxa )
	ROM_REGION( 0x30000, "master", 0 )
	ROM_LOAD( "e-302-31005-04.u38",   0x00000, 0x20000, CRC(e1cf6236) SHA1(fabf423a006b1db22273c6fffa03edc148d7d957) )
	ROM_RELOAD(                       0x10000, 0x20000 )

	ROM_REGION( 0x60000, "slave", 0 )
	ROM_LOAD( "e-302-31012-01.u111",  0x00000, 0x20000, CRC(9a3297cc) SHA1(1dfa0bacd2f2b18d44bfc2d55c40291c1b142f8f) )
	ROM_LOAD( "e-302-31013-01.u112",  0x20000, 0x20000, CRC(7e7c3e2f) SHA1(a7e31e1f1b09414c40ab9ace5e9bffbdbaee8704) )
	ROM_LOAD( "e-302-31014-01.u113",  0x40000, 0x20000, CRC(8cf3e101) SHA1(672a3a0ca0f5334cf614bc49cbc1ae5ccea54cbe) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "e-302-31003-01.u15", 0x20001, 0x20000, CRC(8bb3233b) SHA1(5131ad78bdf904cde36534e99efa5576fcea25c0) )
	ROM_LOAD16_BYTE( "e-302-31001-01.u1",  0x20000, 0x20000, CRC(728d75f2) SHA1(d9e8e742cc2d536bd62370c1e474c7036e4392bb) )
	ROM_LOAD16_BYTE( "e-302-31004-01.u16", 0x60001, 0x20000, CRC(f2bdff48) SHA1(f34eb16ea180effffd81d637acc3d96bffaf81c9) )
	ROM_RELOAD(                            0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "e-302-31002-01.u2",  0x60000, 0x20000, CRC(ca06a394) SHA1(0858908bd150dd7354536e10b2a386b45f17ac9f) )
	ROM_RELOAD(                            0xc0000, 0x20000 )

	ROM_REGION( 0xc0000, "bg_gfx", 0 )
	ROM_LOAD( "e-302-31006-01.u98",   0x00000, 0x20000, CRC(59d0f2ae) SHA1(8da5dc006e192af98458227e79421b6a07ac1cdc) )
	ROM_LOAD( "e-302-31007-01.u99",   0x20000, 0x20000, CRC(6ab7db25) SHA1(25c2fa23b99ac4bab5a9b851c2087de44512a5c2) )
	ROM_LOAD( "e-302-31008-01.u100",  0x40000, 0x20000, CRC(2352849e) SHA1(f49394b6efb6a87d86516ec0a5ddd582f96f7e5d) )
	ROM_LOAD( "e-302-31009-01.u101",  0x60000, 0x20000, CRC(4c31e02b) SHA1(2d8dd97a2a737bafb44dced7ce3eef22d7d14cbe) )
	ROM_LOAD( "e-302-31010-01.u102",  0x80000, 0x20000, CRC(a951228c) SHA1(7ec5cf4d0aa3702be9236d155bea373a06c0be03) )
	ROM_LOAD( "e-302-31011-01.u103",  0xa0000, 0x20000, CRC(ed326164) SHA1(8706192f525ece200587cee7e7beb4a1975bf63e) )

	ROM_REGION( 0x40000, "xrom", ROMREGION_ERASEFF ) // X-ROM (data used by main processor)
	// Empty / not used

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-ataxx.bin", 0x0000, 0x0100, CRC(989cdb8c) SHA1(13b30a328e71a195960e98e50d1657a8b6860dcf) )
ROM_END

ROM_START( ataxxe )
	ROM_REGION( 0x30000, "master", 0 )
	ROM_LOAD( "euro_ataxx_u38_3079.u38", 0x00000, 0x20000, CRC(16aef3b7) SHA1(b2de1e3fd032ab8cc5ed995522f528f0b3283d8a) )
	ROM_RELOAD(                          0x10000, 0x20000 )

	ROM_REGION( 0x60000, "slave", 0 )
	ROM_LOAD( "e-302-31012-01.u111",  0x00000, 0x20000, CRC(9a3297cc) SHA1(1dfa0bacd2f2b18d44bfc2d55c40291c1b142f8f) )
	ROM_LOAD( "e-302-31013-01.u112",  0x20000, 0x20000, CRC(7e7c3e2f) SHA1(a7e31e1f1b09414c40ab9ace5e9bffbdbaee8704) )
	ROM_LOAD( "e-302-31014-01.u113",  0x40000, 0x20000, CRC(8cf3e101) SHA1(672a3a0ca0f5334cf614bc49cbc1ae5ccea54cbe) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "302-38003-01.u15", 0x20001, 0x20000, CRC(db266d3f) SHA1(31c9baf4548b23e1e1939069620a937ee98a7b09) )
	ROM_LOAD16_BYTE( "302-38001-01.u1",  0x20000, 0x20000, CRC(d6db2724) SHA1(d3c7b45b165eb7c9a6369863b273ecac5c31ca65) )
	ROM_LOAD16_BYTE( "302-38004-01.u16", 0x60001, 0x20000, CRC(2b127f56) SHA1(909fed387ad6bb1d83f9cee271e6dc851ac50525) )
	ROM_RELOAD(                          0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "302-38002-01.u2",  0x60000, 0x20000, CRC(1b63b882) SHA1(cb04e641fc173f787a0f48c98f5198db265c26d8) )
	ROM_RELOAD(                          0xc0000, 0x20000 )

	ROM_REGION( 0xc0000, "bg_gfx", 0 )
	ROM_LOAD( "e-302-31006-01.u98",   0x00000, 0x20000, CRC(59d0f2ae) SHA1(8da5dc006e192af98458227e79421b6a07ac1cdc) )
	ROM_LOAD( "e-302-31007-01.u99",   0x20000, 0x20000, CRC(6ab7db25) SHA1(25c2fa23b99ac4bab5a9b851c2087de44512a5c2) )
	ROM_LOAD( "e-302-31008-01.u100",  0x40000, 0x20000, CRC(2352849e) SHA1(f49394b6efb6a87d86516ec0a5ddd582f96f7e5d) )
	ROM_LOAD( "e-302-31009-01.u101",  0x60000, 0x20000, CRC(4c31e02b) SHA1(2d8dd97a2a737bafb44dced7ce3eef22d7d14cbe) )
	ROM_LOAD( "e-302-31010-01.u102",  0x80000, 0x20000, CRC(a951228c) SHA1(7ec5cf4d0aa3702be9236d155bea373a06c0be03) )
	ROM_LOAD( "e-302-31011-01.u103",  0xa0000, 0x20000, CRC(ed326164) SHA1(8706192f525ece200587cee7e7beb4a1975bf63e) )

	ROM_REGION( 0x40000, "xrom", ROMREGION_ERASEFF ) // X-ROM (data used by main processor)
	// Empty / not used

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-ataxxe.bin", 0x0000, 0x0100, CRC(8df1dee1) SHA1(876c5d5d506c31fdf4c3e611a1869b50ceadc6fd) )
ROM_END

ROM_START( ataxxj )
	ROM_REGION( 0x30000, "master", 0 )
	ROM_LOAD( "ataxxj.u38", 0x00000, 0x20000, CRC(513fa7d4) SHA1(1aada72214c0165d76667935855bf996a5b3d55b) )
	ROM_RELOAD(             0x10000, 0x20000 )

	ROM_REGION( 0x60000, "slave", 0 )
	ROM_LOAD( "e-302-31012-01.u111",  0x00000, 0x20000, CRC(9a3297cc) SHA1(1dfa0bacd2f2b18d44bfc2d55c40291c1b142f8f) )
	ROM_LOAD( "e-302-31013-01.u112",  0x20000, 0x20000, CRC(7e7c3e2f) SHA1(a7e31e1f1b09414c40ab9ace5e9bffbdbaee8704) )
	ROM_LOAD( "e-302-31014-01.u113",  0x40000, 0x20000, CRC(8cf3e101) SHA1(672a3a0ca0f5334cf614bc49cbc1ae5ccea54cbe) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "302-38003-01.u15", 0x20001, 0x20000, CRC(db266d3f) SHA1(31c9baf4548b23e1e1939069620a937ee98a7b09) )
	ROM_LOAD16_BYTE( "302-38001-01.u1",  0x20000, 0x20000, CRC(d6db2724) SHA1(d3c7b45b165eb7c9a6369863b273ecac5c31ca65) )
	ROM_LOAD16_BYTE( "302-38004-01.u16", 0x60001, 0x20000, CRC(2b127f56) SHA1(909fed387ad6bb1d83f9cee271e6dc851ac50525) )
	ROM_RELOAD(                          0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "302-38002-01.u2",  0x60000, 0x20000, CRC(1b63b882) SHA1(cb04e641fc173f787a0f48c98f5198db265c26d8) )
	ROM_RELOAD(                          0xc0000, 0x20000 )

	ROM_REGION( 0xc0000, "bg_gfx", 0 )
	ROM_LOAD( "e-302-31006-01.u98",   0x00000, 0x20000, CRC(59d0f2ae) SHA1(8da5dc006e192af98458227e79421b6a07ac1cdc) )
	ROM_LOAD( "e-302-31007-01.u99",   0x20000, 0x20000, CRC(6ab7db25) SHA1(25c2fa23b99ac4bab5a9b851c2087de44512a5c2) )
	ROM_LOAD( "e-302-31008-01.u100",  0x40000, 0x20000, CRC(2352849e) SHA1(f49394b6efb6a87d86516ec0a5ddd582f96f7e5d) )
	ROM_LOAD( "e-302-31009-01.u101",  0x60000, 0x20000, CRC(4c31e02b) SHA1(2d8dd97a2a737bafb44dced7ce3eef22d7d14cbe) )
	ROM_LOAD( "e-302-31010-01.u102",  0x80000, 0x20000, CRC(a951228c) SHA1(7ec5cf4d0aa3702be9236d155bea373a06c0be03) )
	ROM_LOAD( "e-302-31011-01.u103",  0xa0000, 0x20000, CRC(ed326164) SHA1(8706192f525ece200587cee7e7beb4a1975bf63e) )

	ROM_REGION( 0x40000, "xrom", ROMREGION_ERASEFF ) // X-ROM (data used by main processor)
	// Empty / not used

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-ataxxj.bin", 0x0000, 0x0100, CRC(8df1dee1) SHA1(876c5d5d506c31fdf4c3e611a1869b50ceadc6fd) )
ROM_END


/*
For World Soccer Finals, the label format is:
------------------------
|(C)1990 Leland Corp.  | -> Copyright & Manufacturer
|P/N E-302-30022-04    | -> Part number with revision
|WORLD SOCCER     U64  | -> Game name & ROM PCB location
------------------------
*/
ROM_START( wsf )
	ROM_REGION( 0x50000, "master", 0 )
	ROM_LOAD( "e-302-30022-04.u64",  0x00000, 0x20000, CRC(533cc90f) SHA1(44cec1093819dc1eb5c4ed48ac8a666d9601a870) )
	ROM_RELOAD(                      0x10000, 0x20000 )
	ROM_LOAD( "e-302-30023-04.u65",  0x30000, 0x20000, CRC(763c6c1b) SHA1(4f0ef31b7ec4b060dac28ebb1dc663f9328c9f97) )

	ROM_REGION( 0x100000, "slave", 0 )
	ROM_LOAD( "e-302-30001-01.u151",  0x00000, 0x20000, CRC(31c63af5) SHA1(268093ade200241339b6f60a00123bbf73325e38) )
	ROM_LOAD( "e-302-30002-01.u152",  0x20000, 0x20000, CRC(a53e88a6) SHA1(0b7748b70d6dd9fcc1a22646e8af20f3baa4aa40) )
	ROM_LOAD( "e-302-30003-01.u153",  0x40000, 0x20000, CRC(12afad1d) SHA1(848549db714b46497176e42d6f2088ba3d6ab2f4) )
	ROM_LOAD( "e-302-30004-01.u154",  0x60000, 0x20000, CRC(b8b3d59c) SHA1(9ba6e25bb5132c556557a0395ce1d982c0853426) )
	ROM_LOAD( "e-302-30005-01.u155",  0x80000, 0x20000, CRC(505724b9) SHA1(f8a29e3e7f0a146f2daf67883de12533b2ed7341) )
	ROM_LOAD( "e-302-30006-01.u156",  0xa0000, 0x20000, CRC(c86b5c4d) SHA1(f04d8fc1e8f872f406fcad69ff71ed695f42797a) )
	ROM_LOAD( "e-302-30007-01.u157",  0xc0000, 0x20000, CRC(451321ae) SHA1(da82f0bba4341b087136afa17767b64389a0f8f4) )
	ROM_LOAD( "e-302-30008-01.u158",  0xe0000, 0x20000, CRC(4d23836f) SHA1(7b5b9419774e7537e69017c4c44a0601b6e93714) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "e-302-30017-02.u3",  0x20001, 0x20000, CRC(b1d578e1) SHA1(861d597c403b5b385395383dc70eac7a8496c11e) )
	ROM_LOAD16_BYTE( "e-302-30020-02.u6",  0x20000, 0x20000, CRC(919a62ee) SHA1(ec8110e77d5bd88c39582dc3804fa6982306ed40) )
	ROM_LOAD16_BYTE( "e-302-30018-02.u4",  0x60001, 0x20000, CRC(d24947ee) SHA1(e40913d4b16d49357e6b9a306b4cfd5091a15ded) )
	ROM_RELOAD(                            0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "e-302-30019-02.u5",  0x60000, 0x20000, CRC(d846f292) SHA1(97ede713aada8f9b7d4afd0a7f7b71a98c4fd15d) )
	ROM_RELOAD(                            0xc0000, 0x20000 )

	ROM_REGION( 0x60000, "bg_gfx", 0 )
	ROM_LOAD( "e-302-30011-03.u145",  0x00000, 0x10000, CRC(6d1c1f19) SHA1(8291a09efab00af2b24cd03f413c152e5e191a5d) )
	ROM_LOAD( "e-302-30012-03.u146",  0x10000, 0x10000, CRC(6b69bc9e) SHA1(0cc72edd40c997b583b3300136b6d5c7f774ae0a) )
	ROM_LOAD( "e-302-30013-03.u147",  0x20000, 0x10000, CRC(e1b1e36f) SHA1(4c4040b4dd36a81a6d2affb1e0e07f8d13477205) )
	ROM_LOAD( "e-302-30014-02.u148",  0x30000, 0x10000, CRC(4a5e7d2b) SHA1(c2d0f60dd1923ea0672a145f72159d71d86e1440) )
	ROM_LOAD( "e-302-30015-02.u149",  0x40000, 0x10000, CRC(b30c4ff3) SHA1(8bd709bd277584fc32983424873097300eb791c6) )
	ROM_LOAD( "e-302-30016-02.u150",  0x50000, 0x10000, CRC(3bc8efac) SHA1(78451b935fba519603db0de2e2d06e83b3d4353a) )

	ROM_REGION( 0x20000, "xrom", 0 ) // X-ROM (data used by main processor)
	ROM_LOAD( "e-302-30009-01.u68",  0x00000, 0x10000, CRC(f2fbfc15) SHA1(712cfa7b11135b1f568f38cc478ef5a3330d0608) )
	ROM_LOAD( "e-302-30010-01.u69",  0x10000, 0x10000, CRC(b4ed2d3b) SHA1(61c9d86b63cf000187a105c6eed967fecb2f3c1c) )

	ROM_REGION( 0x20000, "custom:ext", 0 ) // externally clocked DAC data
	ROM_LOAD( "e-302-30021-02.u8",   0x00000, 0x20000, CRC(a8f97be4) SHA1(738a2ec96a923ef3b3c62425365d4455ba200119) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-wsf.bin", 0x0000, 0x0100, CRC(5bd0633d) SHA1(4917a0b0be82dc1bd4cfdb5bfb509f0472f1014f) )
ROM_END

ROM_START( wsf3 )
	ROM_REGION( 0x50000, "master", 0 )
	ROM_LOAD( "e-302-30022-03.u64",  0x00000, 0x20000, CRC(2e7faa96) SHA1(d43915a433133eca650fabece61a4a65642b39f6) )
	ROM_RELOAD(                      0x10000, 0x20000 )
	ROM_LOAD( "e-302-30023-03.u65",  0x30000, 0x20000, CRC(7146328f) SHA1(390b98a2cd54a981eb4fafba700ff2fa1e379a32) )

	ROM_REGION( 0x100000, "slave", 0 )
	ROM_LOAD( "e-302-30001-01.u151",  0x00000, 0x20000, CRC(31c63af5) SHA1(268093ade200241339b6f60a00123bbf73325e38) )
	ROM_LOAD( "e-302-30002-01.u152",  0x20000, 0x20000, CRC(a53e88a6) SHA1(0b7748b70d6dd9fcc1a22646e8af20f3baa4aa40) )
	ROM_LOAD( "e-302-30003-01.u153",  0x40000, 0x20000, CRC(12afad1d) SHA1(848549db714b46497176e42d6f2088ba3d6ab2f4) )
	ROM_LOAD( "e-302-30004-01.u154",  0x60000, 0x20000, CRC(b8b3d59c) SHA1(9ba6e25bb5132c556557a0395ce1d982c0853426) )
	ROM_LOAD( "e-302-30005-01.u155",  0x80000, 0x20000, CRC(505724b9) SHA1(f8a29e3e7f0a146f2daf67883de12533b2ed7341) )
	ROM_LOAD( "e-302-30006-01.u156",  0xa0000, 0x20000, CRC(c86b5c4d) SHA1(f04d8fc1e8f872f406fcad69ff71ed695f42797a) )
	ROM_LOAD( "e-302-30007-01.u157",  0xc0000, 0x20000, CRC(451321ae) SHA1(da82f0bba4341b087136afa17767b64389a0f8f4) )
	ROM_LOAD( "e-302-30008-01.u158",  0xe0000, 0x20000, CRC(4d23836f) SHA1(7b5b9419774e7537e69017c4c44a0601b6e93714) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "e-302-30017-01.u3",  0x20001, 0x20000, CRC(39ec13c1) SHA1(4067da05cbaf205ab7cc14a3370220ad98b394cd) )
	ROM_LOAD16_BYTE( "e-302-30020-01.u6",  0x20000, 0x20000, CRC(532c02bf) SHA1(a2070d57f1ce2a68a064872ea7b77ba418187cfe) )
	ROM_LOAD16_BYTE( "e-302-30018-01.u4",  0x60001, 0x20000, CRC(1ec16735) SHA1(86766742b50edd25cfeef6f808d2733c484eca4e) )
	ROM_RELOAD(                            0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "e-302-30019-01.u5",  0x60000, 0x20000, CRC(2881f73b) SHA1(414d974018fb4518c46b913184b07add69251724) )
	ROM_RELOAD(                            0xc0000, 0x20000 )

	ROM_REGION( 0x60000, "bg_gfx", 0 )
	ROM_LOAD( "e-302-30011-02.u145",  0x00000, 0x10000, CRC(6153569b) SHA1(b6a106c8b87a9a3f01eff3854d0c1f2c4a64fd94) )
	ROM_LOAD( "e-302-30012-02.u146",  0x10000, 0x10000, CRC(52d65e21) SHA1(25f63aa29dc7e7673043e1f43e357a5232a1be9e) )
	ROM_LOAD( "e-302-30013-02.u147",  0x20000, 0x10000, CRC(b3afda12) SHA1(52bf780c642f0092114aeb994e6571c034f198a0) )
	ROM_LOAD( "e-302-30014-01.u148",  0x30000, 0x10000, CRC(624e6c64) SHA1(02240adcf4433543c8f7ad8904c34400f25409cc) )
	ROM_LOAD( "e-302-30015-01.u149",  0x40000, 0x10000, CRC(5d9064f2) SHA1(7a68a379aa6a6cd0518e8a4107b2e646f5700c2b) )
	ROM_LOAD( "e-302-30016-01.u150",  0x50000, 0x10000, CRC(d76389cd) SHA1(2b7e6cd662ffde177b110ad0ed2e42fe4ccf811f) )

	ROM_REGION( 0x20000, "xrom", 0 ) // X-ROM (data used by main processor)
	ROM_LOAD( "e-302-30009-01.u68",  0x00000, 0x10000, CRC(f2fbfc15) SHA1(712cfa7b11135b1f568f38cc478ef5a3330d0608) )
	ROM_LOAD( "e-302-30010-01.u69",  0x10000, 0x10000, CRC(b4ed2d3b) SHA1(61c9d86b63cf000187a105c6eed967fecb2f3c1c) )

	ROM_REGION( 0x20000, "custom:ext", 0 ) // externally clocked DAC data
	ROM_LOAD( "e-302-30021-01.u8",   0x00000, 0x20000, CRC(bb91dc10) SHA1(a7d8676867b5cfe1049040e593985af57ef04334) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-wsf.bin", 0x0000, 0x0100, CRC(5bd0633d) SHA1(4917a0b0be82dc1bd4cfdb5bfb509f0472f1014f) )
ROM_END


/*
For Danny Sullivan's Indy Heat, the label format is:
------------------------
|(C)1991 Leland Corp.  | -> Copyright & Manufacturer
|P/N E-302-33019-01    | -> Part number with revision
|INDY HEAT        U64  | -> Game name & ROM PCB location
------------------------
*/
ROM_START( indyheat )
	ROM_REGION( 0x90000, "master", 0 )
	ROM_LOAD( "e-302-33019-01.u64",   0x00000, 0x20000, CRC(2b97a347) SHA1(958a774e9ea3678c0fdd2466e578df8267b4413e) )
	ROM_RELOAD(                       0x10000, 0x20000 )
	ROM_LOAD( "e-302-33020-01.u65",   0x30000, 0x20000, CRC(71301d74) SHA1(bbabc71aa8d56f6984de573f0fb5d3fea35421a9) )
	ROM_LOAD( "e-302-33017-01.u66",   0x50000, 0x20000, CRC(c9612072) SHA1(d00bf703ce4ad0a344b3d8afcd1f45c3c82b54fe) )
	ROM_LOAD( "e-302-33018-01.u67",   0x70000, 0x20000, CRC(4c4b25e0) SHA1(f07d347cc844df2d824853af8dbfc557933e7765) )

	ROM_REGION( 0x160000, "slave", 0 )
	ROM_LOAD( "e-302-33007-01.u151",  0x00000, 0x20000, CRC(2622dfa4) SHA1(759e46540ad9f2ed540314b174c88f7365214051) )
	ROM_LOAD( "e-302-33008-01.u152",  0x20000, 0x20000, CRC(ad40e4e2) SHA1(58c3df82551199fb3f28c6459aedc2117caf520e) )
	ROM_CONTINUE(                     0x120000, 0x20000 )
	ROM_LOAD( "e-302-33009-01.u153",  0x40000, 0x20000, CRC(1e3803f7) SHA1(e3862ed748cdd0dffdde8e1435c20c7388e698dd) )
	ROM_CONTINUE(                     0x140000, 0x20000 )
	ROM_LOAD( "e-302-33010-01.u154",  0x60000, 0x20000, CRC(76d3c235) SHA1(48b46fe465c6db4dc46a64245a6c69b21b54ab6f) )
	ROM_LOAD( "e-302-33011-01.u155",  0x80000, 0x20000, CRC(d5d866b3) SHA1(2584e2299bdbc50c836ae86a1c4b7e68c65a49cd) )
	ROM_LOAD( "e-302-33012-01.u156",  0xa0000, 0x20000, CRC(7fe71842) SHA1(4ba09ccba29f9feef89ce61155e2508e800cdee8) )
	ROM_LOAD( "e-302-33013-01.u157",  0xc0000, 0x20000, CRC(a6462adc) SHA1(bdc744e3c836715874d40b9e32f509f288ce00fd) )
	ROM_LOAD( "e-302-33014-01.u158",  0xe0000, 0x20000, CRC(d6ef27a3) SHA1(37fcf772ce564a9300f9dd437b9015a2d25b46b5) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "e-302-33021-01.u3",  0x20001, 0x20000, CRC(97413818) SHA1(64caa14e05dd9ec43ce13f5c738df1f39f5fa75c) )
	ROM_LOAD16_BYTE( "e-302-33024-01.u6",  0x20000, 0x20000, CRC(15a89962) SHA1(52f66e1ccde0ef3fb7959a207cc967237e37833e) )
	ROM_LOAD16_BYTE( "e-302-33022-01.u4",  0x60001, 0x20000, CRC(fa7bfa04) SHA1(0174f5372117d15bf0ecd48b72c9cca4cf8bb75f) )
	ROM_RELOAD(                            0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "e-302-33023-01.u5",  0x60000, 0x20000, CRC(198285d4) SHA1(8f6b3cba2bc729f2e0623578b13720ead91333e4) )
	ROM_RELOAD(                            0xc0000, 0x20000 )

	ROM_REGION( 0xc0000, "bg_gfx", 0 )
	ROM_LOAD( "e-302-33001-01.u145",  0x00000, 0x20000, CRC(612d4bf8) SHA1(035cc8723524e2c6aa79ffa7d7c1f6fb0a25cc51) )
	ROM_LOAD( "e-302-33002-01.u146",  0x20000, 0x20000, CRC(77a725f6) SHA1(9bb521ed7202387bbf2670f9b1ae3cbe5064ae03) )
	ROM_LOAD( "e-302-33003-01.u147",  0x40000, 0x20000, CRC(d6aac372) SHA1(49f5f5d6c2a82ea15905086a2f8e3ea061d37dfc) )
	ROM_LOAD( "e-302-33004-01.u148",  0x60000, 0x20000, CRC(5d19723e) SHA1(a6f09b92c95321962f62a17fc0ccdbfbf78b8b88) )
	ROM_LOAD( "e-302-33005-01.u149",  0x80000, 0x20000, CRC(29056791) SHA1(343452b883f139eb09da6b5f384aa680d3a2218c) )
	ROM_LOAD( "e-302-33006-01.u150",  0xa0000, 0x20000, CRC(cb73dd6a) SHA1(60aabedbab409acaf8ba4f2366125290825971a4) )

	ROM_REGION( 0x40000, "xrom", 0 ) // X-ROM (data used by main processor)
	ROM_LOAD( "e-302-33015-01.u68",   0x00000, 0x10000, CRC(9e88efb3) SHA1(983bc22c9401b9d6c959dd211b6b7dfa1a6c14e2) )
	ROM_CONTINUE(                     0x20000, 0x10000 )
	ROM_LOAD( "e-302-33016-01.u69",   0x10000, 0x10000, CRC(aa39fcb3) SHA1(0cb328d784cda3e0dff3a018f52f9b06bc5d46b8) )
	ROM_CONTINUE(                     0x30000, 0x10000 )

	ROM_REGION( 0x40000, "custom:ext", 0 ) // externally clocked DAC data
	ROM_LOAD( "e-302-33025-01.u8",  0x00000, 0x20000, CRC(9f16e5b6) SHA1(0ea814db7f647f39d11dcde793a17831fca3bddd) )
	ROM_LOAD( "e-302-33026-01.u9",  0x20000, 0x20000, CRC(0dc8f488) SHA1(2ff0f45f17b8a182afdaa5603e7a1af70e6336b7) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-indyheat.bin", 0x0000, 0x0100, CRC(896f7257) SHA1(bd1f116c2650576da73f0ca647a7f872c890dfe5) )
ROM_END


ROM_START( brutforc )
	ROM_REGION( 0x90000, "master", 0 )
	ROM_LOAD( "u64",   0x00000, 0x20000, CRC(008ae3b8) SHA1(bc9fdba761501efeaf665ac33ff1ad6935d70638) )
	ROM_RELOAD(                 0x10000, 0x20000 )
	ROM_LOAD( "u65",   0x30000, 0x20000, CRC(6036e3fa) SHA1(eba79e92f3de7afdd6e404cabb4b8cfad09cf50b) )
	ROM_LOAD( "u66",   0x50000, 0x20000, CRC(7ebf0795) SHA1(6b25ccac88ff61be3c461eb49908fbecf509434f) )
	ROM_LOAD( "u67",   0x70000, 0x20000, CRC(e3cbf8b4) SHA1(ceaefc454385ee1dfbfe2d211a72af0883967bc0) )

	ROM_REGION( 0x100000, "slave", 0 )
	ROM_LOAD( "u151",  0x00000, 0x20000, CRC(bd3b677b) SHA1(8ac32b9598a97d9910ac31948f166e9474df07fa) )
	ROM_LOAD( "u152",  0x20000, 0x20000, CRC(5f4434e7) SHA1(2b8eb2f6ede328c88b7977e3bea73d00dcaa8f6f) )
	ROM_LOAD( "u153",  0x40000, 0x20000, CRC(20f7df53) SHA1(6ea4600a9cffbc414f546fcd8c036faaa6d7fffd) )
	ROM_LOAD( "u154",  0x60000, 0x20000, CRC(69ce2329) SHA1(24819883631e987a201e7dea0684410e74b9d56d) )
	ROM_LOAD( "u155",  0x80000, 0x20000, CRC(33d92e25) SHA1(fe47da054e12f7e16631cb7cb0279ace717b945b) )
	ROM_LOAD( "u156",  0xa0000, 0x20000, CRC(de7eca8b) SHA1(a5d452c0cb52be16560ccd67d423bdf33d58ec58) )
	ROM_LOAD( "u157",  0xc0000, 0x20000, CRC(e42b3dba) SHA1(ed3707932507bcddd0191e36e2f5479b2ce2e642) )
	ROM_LOAD( "u158",  0xe0000, 0x20000, CRC(a0aa3220) SHA1(bd9bffa4fcf76e34a72a497d322c0430cbc7c81e) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "u3",  0x20001, 0x20000, CRC(9984906c) SHA1(66626ea32fb510a9bb1974e41806fee6a4afa1cf) )
	ROM_LOAD16_BYTE( "u6",  0x20000, 0x20000, CRC(c9c5a413) SHA1(5d4f8bc895b89267643b41ecad52b886fd88df97) )
	ROM_LOAD16_BYTE( "u4",  0x60001, 0x20000, CRC(ca8ab3a6) SHA1(2e7c7f50fbaed7e052a97ac7954b634bbc657226) )
	ROM_RELOAD(             0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "u5",  0x60000, 0x20000, CRC(cbdb914b) SHA1(813640fa291c1245d04a628ee62afc95d5c67a03) )
	ROM_RELOAD(             0xc0000, 0x20000 )

	ROM_REGION( 0x180000, "bg_gfx", 0 )
	ROM_LOAD( "u145",  0x000000, 0x40000, CRC(c3d20d24) SHA1(a75217b0d1887c64bf5570ff7a461c8cf47c5e85) )
	ROM_LOAD( "u146",  0x040000, 0x40000, CRC(43e9dd87) SHA1(0694803a5b33c074858770c7e4cd884402c263f8) )
	ROM_LOAD( "u147",  0x080000, 0x40000, CRC(fb855ce8) SHA1(839bca2d8e344d43fad8978b812c9246a89054a8) )
	ROM_LOAD( "u148",  0x0c0000, 0x40000, CRC(e4b54eae) SHA1(591ee8e0c1b7c2eb8d7834a42548d5b25c79bb26) )
	ROM_LOAD( "u149",  0x100000, 0x40000, CRC(cf48401c) SHA1(70ba8f2d5f81795c26c2a552c29c913c5d3bd784) )
	ROM_LOAD( "u150",  0x140000, 0x40000, CRC(ca9e1e33) SHA1(f9889042b536e1fb5521702bc807d5aa0e6a25d1) )

	ROM_REGION( 0x40000, "xrom", 0 ) // X-ROM (data used by main processor)
	ROM_LOAD( "u68",   0x00000, 0x10000, CRC(77c8de62) SHA1(ae15f84b7bf3d6705edf9f41d8de7b6ecab2bcf9) )
	ROM_CONTINUE(      0x20000, 0x10000 )
	ROM_LOAD( "u69",   0x10000, 0x10000, CRC(113aa6d5) SHA1(d032a04338e12135ba410afd71cf9538e99eb109) )
	ROM_CONTINUE(      0x30000, 0x10000 )

	ROM_REGION( 0x80000, "custom:ext", 0 ) // externally clocked DAC data
	ROM_LOAD( "u8",  0x00000, 0x20000, CRC(1e0ead72) SHA1(879d5ba244238af21f6a516494c504721570ec15) )
	ROM_LOAD( "u9",  0x20000, 0x20000, CRC(3195b305) SHA1(7c795a7973e0b8dbeb882777d4bee2accc46cea0) )
	ROM_LOAD( "u10", 0x40000, 0x20000, CRC(1dc5f375) SHA1(9dd389c30d87fcb02c6a15b67b4b6ea5b555a762) )
	ROM_LOAD( "u11", 0x60000, 0x20000, CRC(5ed4877f) SHA1(eab9e949b1afd1fa21d87af5abcb1a8dc9bcf0d8) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-brutforc.bin", 0x0000, 0x0100, CRC(508809af) SHA1(17352c0922631fca2ca2bbca4c50b3e0277caaf9) )
ROM_END


ROM_START( asylum )
	ROM_REGION( 0x90000, "master", 0 )
	ROM_LOAD( "asy-m0.64",   0x00000, 0x20000, CRC(f5ca36fd) SHA1(8c36ce3ca1c30ffb0a32ff7e9df61901c1ee6151) )
	ROM_RELOAD(              0x10000, 0x20000 )
	ROM_LOAD( "asy-m1.65",   0x30000, 0x20000, CRC(14d91d09) SHA1(ad227e6f5047f43c421773385f441c634af110e6) )
	ROM_LOAD( "asy-m2.66",   0x50000, 0x20000, CRC(a34a6ef9) SHA1(c90307024039a7809b7fafb019c9ad4636708a88) )
	ROM_LOAD( "asy-m3.67",   0x70000, 0x20000, CRC(9db4c2b1) SHA1(cfe78e2fe803c816ed2f79250bbbaf293cb5bf2a) )

	ROM_REGION( 0x1e0000, "slave", 0 )
	ROM_LOAD( "asy-sp0.151",  0x00000, 0x20000, CRC(5ad5e3b0) SHA1(0162b56f63c169825677323dfbbd3ea991a9d9bb) )
	ROM_LOAD( "asy-sp2.152",  0x20000, 0x20000, CRC(6d2997ec) SHA1(bf97dba0a4a700af0eb753daf598ec8e903dbc7c) )
	ROM_CONTINUE(            0x120000, 0x20000 )
	ROM_LOAD( "asy-sp4.153",  0x40000, 0x20000, CRC(7c61973c) SHA1(560ac49f92ddb25b975cbfb3ffc1464fe0c72e90) )
	ROM_CONTINUE(            0x140000, 0x20000 )
	ROM_LOAD( "asy-sp6.154",  0x60000, 0x20000, CRC(f0a4f9d3) SHA1(af7737803c909afad0d44f328adf14a9e7b3b108) )
	ROM_CONTINUE(            0x160000, 0x20000 )
	ROM_LOAD( "asy-sp8.155",  0x80000, 0x20000, CRC(2ad0640e) SHA1(6be547c297eb09187663bf3302b01c31d2990dac) )
	ROM_CONTINUE(            0x180000, 0x20000 )
	ROM_LOAD( "asy-spa.156",  0xa0000, 0x20000, CRC(9d584fb4) SHA1(fb331c63cb3f29ed6925acc1b1e41d63a242af37) )
	ROM_CONTINUE(            0x1a0000, 0x20000 )
	ROM_LOAD( "asy-spc.157",  0xc0000, 0x20000, CRC(8485e48c) SHA1(7381b55c96b1fce58e2f8914d603b35b397c881b) )
	ROM_CONTINUE(            0x1c0000, 0x20000 )
	ROM_LOAD( "asy-spe.158",  0xe0000, 0x20000, CRC(49d19520) SHA1(6f24221c976e9dacc1ce96dfc1d1e3df4e8a8255) )

	ROM_REGION( 0x100000, "custom:audiocpu", 0 )
	ROM_LOAD16_BYTE( "asy-65.3",  0x20001, 0x20000, CRC(709bdc78) SHA1(ca235c2ab26fbb153ffe775a1a44b31695902d3f) )
	ROM_LOAD16_BYTE( "asy-65.6",  0x20000, 0x20000, CRC(d019fb2e) SHA1(9d16b0399f03067e7bf79043904a1045119937c6) )
	ROM_LOAD16_BYTE( "asy-65.4",  0x60001, 0x20000, CRC(1882c3b2) SHA1(71af49d1f59e257e5f8a0fc590d0533dda5bf82b) )
	ROM_RELOAD(                   0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "asy-65.5",  0x60000, 0x20000, CRC(5814b307) SHA1(6db97804d58941a5543424d8c4658cb3edab1e43) )
	ROM_RELOAD(                   0xc0000, 0x20000 )

	ROM_REGION( 0x180000, "bg_gfx", 0 )
	ROM_LOAD( "asy-chr0.145",  0x000000, 0x40000, CRC(4dbcae49) SHA1(0aa54daa099d6590a41df4a24a27bf6463b3e116) )
	ROM_LOAD( "asy-chr1.146",  0x040000, 0x40000, CRC(34e7762d) SHA1(2d63971effc237846481bed7d829fa924b4bea31) )
	ROM_LOAD( "asy-chr2.147",  0x080000, 0x40000, CRC(f9b0d375) SHA1(305172d8cdf390d9566c2c6f32d8da44b165022a) )
	ROM_LOAD( "asy-chr3.148",  0x0c0000, 0x40000, CRC(5efcae94) SHA1(dd7f903efd15e14c06e8d53cf7021f4323c127d1) )
	ROM_LOAD( "asy-chr4.149",  0x100000, 0x40000, CRC(dbc2b155) SHA1(ba0d90b5a6acc53ecd02317cb82b630451e9d0e9) )
	ROM_LOAD( "asy-chr5.150",  0x140000, 0x40000, CRC(9675e44f) SHA1(d2633d21fa9e798b8f96d96fdce5bb99a7dc5ba5) )

	ROM_REGION( 0x40000, "xrom", 0 ) // X-ROM (data used by main processor)
	ROM_LOAD( "asy-m4.68",   0x00000, 0x10000, CRC(77c8de62) SHA1(ae15f84b7bf3d6705edf9f41d8de7b6ecab2bcf9) )
	ROM_CONTINUE(            0x20000, 0x10000 )
	ROM_LOAD( "asy-m5.69",   0x10000, 0x10000, CRC(bfc50d6c) SHA1(3239242358e8336354a9bd35f75f9057f079b298) )
	ROM_CONTINUE(            0x30000, 0x10000 )

	ROM_REGION( 0x80000, "custom:ext", 0 ) // externally clocked DAC data
	ROM_LOAD( "asy-65.8",  0x00000, 0x20000, CRC(624ad02f) SHA1(ce2dd0d11ff39a8e04d1c27cdaca3f068e6fbcf2) )
	ROM_LOAD( "asy-65.9",  0x20000, 0x20000, CRC(c92ff376) SHA1(0189519101e3b0b464f0bd3af8352c002e45f937) )
	ROM_LOAD( "asy-65.10", 0x40000, 0x20000, CRC(744dbf25) SHA1(03ea3d6eef94005ec0fbbaf43b59e3063830452e) )
	ROM_LOAD( "asy-65.11", 0x60000, 0x20000, CRC(4b185d22) SHA1(d59a72d8c6532875f6e31939c5f846da64ba1bdd) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-asylum.bin", 0x0000, 0x0100, CRC(9a9a361b) SHA1(35daf1677ba18c09d2f9e33e75cf3f8d6a01e7c8) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void leland_state::init_master_ports(u8 mvram_base, u8 io_base)
{
	/* set up the master CPU VRAM I/O */
	m_master->space(AS_IO).install_readwrite_handler(mvram_base, mvram_base + 0x1f, read8sm_delegate(*this, FUNC(leland_state::leland_mvram_port_r)), write8sm_delegate(*this, FUNC(leland_state::leland_mvram_port_w)));

	/* set up the master CPU I/O ports */
	m_master->space(AS_IO).install_read_handler(io_base, io_base + 0x1f, read8sm_delegate(*this, FUNC(leland_state::leland_master_input_r)));
	m_master->space(AS_IO).install_write_handler(io_base, io_base + 0x0f, write8sm_delegate(*this, FUNC(leland_state::leland_master_output_w)));
}


void leland_state::init_cerberus()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::cerberus_bankswitch;
	m_master_bankslot[0]->set_base(memregion("master")->base() + 0x2000);
	m_master_bankslot[1]->set_base(memregion("master")->base() + 0xa000);
	m_slave_bankslot->set_base(memregion("slave")->base() + 0x2000);

	/* set up the master CPU I/O ports */
	init_master_ports(0x40, 0x80);

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_handler(0x80, 0x80, read8smo_delegate(*this, FUNC(leland_state::cerberus_dial_1_r)));
	m_master->space(AS_IO).install_read_handler(0x90, 0x90, read8smo_delegate(*this, FUNC(leland_state::cerberus_dial_2_r)));
}


void leland_state::init_mayhem()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::mayhem_bankswitch;

	/* set up the master CPU I/O ports */
	init_master_ports(0x00, 0xc0);
}


void leland_state::init_powrplay()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::mayhem_bankswitch;

	/* set up the master CPU I/O ports */
	init_master_ports(0x40, 0x80);
}


void leland_state::init_wseries()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::mayhem_bankswitch;

	/* set up the master CPU I/O ports */
	init_master_ports(0x40, 0x80);
}


void leland_state::init_alleymas()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::mayhem_bankswitch;

	/* set up the master CPU I/O ports */
	init_master_ports(0x00, 0xc0);

	/* kludge warning: the game uses location E0CA to determine if the joysticks are available */
	/* it gets cleared by the code, but there is no obvious way for the value to be set to a */
	/* non-zero value. If the value is zero, the joystick is never read. */
	m_master->space(AS_PROGRAM).install_write_handler(0xe0ca, 0xe0ca, write8smo_delegate(*this, FUNC(leland_state::alleymas_joystick_kludge)));
	m_alleymas_kludge_mem = m_mainram + (0xe0ca - 0xe000);
}


void leland_state::init_upyoural()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::mayhem_bankswitch;

	/* set up the master CPU I/O ports */
	init_master_ports(0x00, 0xc0);
}


void leland_state::init_dangerz()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::dangerz_bankswitch;

	/* set up the master CPU I/O ports */
	init_master_ports(0x40, 0x80);

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_handler(0xf4, 0xf4, read8smo_delegate(*this, FUNC(leland_state::dangerz_input_upper_r)));
	m_master->space(AS_IO).install_read_handler(0xf8, 0xf8, read8smo_delegate(*this, FUNC(leland_state::dangerz_input_y_r)));
	m_master->space(AS_IO).install_read_handler(0xfc, 0xfc, read8smo_delegate(*this, FUNC(leland_state::dangerz_input_x_r)));

	save_item(NAME(m_dangerz_x));
	save_item(NAME(m_dangerz_y));
}


void leland_state::init_basebal2()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::basebal2_bankswitch;

	/* set up the master CPU I/O ports */
	init_master_ports(0x00, 0xc0);
}


void leland_state::init_dblplay()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::basebal2_bankswitch;

	/* set up the master CPU I/O ports */
	init_master_ports(0x80, 0x40);
}


void leland_state::init_strkzone()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::basebal2_bankswitch;

	/* set up the master CPU I/O ports */
	init_master_ports(0x00, 0x40);
}


void redline_state::init_redlin2p()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::redline_bankswitch;

	rotate_memory("master");

	/* set up the master CPU I/O ports */
	init_master_ports(0x00, 0xc0);

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_handler(0xc0, 0xc0, read8smo_delegate(*this, FUNC(redline_state::redline_pedal_1_r)));
	m_master->space(AS_IO).install_read_handler(0xd0, 0xd0, read8smo_delegate(*this, FUNC(redline_state::redline_pedal_2_r)));
	m_master->space(AS_IO).install_read_handler(0xf8, 0xf8, read8smo_delegate(*this, FUNC(redline_state::redline_wheel_2_r)));
	m_master->space(AS_IO).install_read_handler(0xfb, 0xfb, read8smo_delegate(*this, FUNC(redline_state::redline_wheel_1_r)));
}


void redline_state::init_quarterb()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::viper_bankswitch;

	rotate_memory("master");

	/* set up the master CPU I/O ports */
	init_master_ports(0x40, 0x80);
}


void redline_state::init_viper()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::viper_bankswitch;

	rotate_memory("master");
	rotate_memory("slave");
	rotate_memory("slave");

	/* set up the master CPU I/O ports */
	init_master_ports(0x00, 0xc0);

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_handler(0xa4, 0xa4, read8smo_delegate(*this, FUNC(redline_state::dangerz_input_upper_r)));
	m_master->space(AS_IO).install_read_handler(0xb8, 0xb8, read8smo_delegate(*this, FUNC(redline_state::dangerz_input_y_r)));
	m_master->space(AS_IO).install_read_handler(0xbc, 0xbc, read8smo_delegate(*this, FUNC(redline_state::dangerz_input_x_r)));

	save_item(NAME(m_dangerz_x));
	save_item(NAME(m_dangerz_y));
}


void redline_state::init_teamqb()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::viper_bankswitch;

	rotate_memory("master");
	rotate_memory("slave");
	rotate_memory("slave");

	/* set up the master CPU I/O ports */
	init_master_ports(0x40, 0x80);

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_port(0x7c, 0x7c, "IN4");
	m_master->space(AS_IO).install_read_port(0x7f, 0x7f, "IN5");
}


void redline_state::init_aafb()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::viper_bankswitch;

	rotate_memory("master");
	rotate_memory("slave");
	rotate_memory("slave");

	/* set up the master CPU I/O ports */
	init_master_ports(0x00, 0xc0);

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_port(0x7c, 0x7c, "IN4");
	m_master->space(AS_IO).install_read_port(0x7f, 0x7f, "IN5");
}


void redline_state::init_aafbb()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::viper_bankswitch;

	rotate_memory("master");
	rotate_memory("slave");
	rotate_memory("slave");

	/* set up the master CPU I/O ports */
	init_master_ports(0x80, 0x40);

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_port(0x7c, 0x7c, "IN4");
	m_master->space(AS_IO).install_read_port(0x7f, 0x7f, "IN5");
}


void redline_state::init_aafbd2p()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::viper_bankswitch;

	rotate_memory("master");
	rotate_memory("slave");
	rotate_memory("slave");

	/* set up the master CPU I/O ports */
	init_master_ports(0x00, 0x40);

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_port(0x7c, 0x7c, "IN4");
	m_master->space(AS_IO).install_read_port(0x7f, 0x7f, "IN5");
}


void redline_state::init_offroad()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::offroad_bankswitch;

	rotate_memory("master");
	rotate_memory("slave");
	rotate_memory("slave");

	/* set up the master CPU I/O ports */
	init_master_ports(0x00, 0xc0);
	init_master_ports(0x40, 0x80);   /* yes, this is intentional */

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_handler(0xf8, 0xf8, read8smo_delegate(*this, FUNC(redline_state::offroad_wheel_3_r)));
	m_master->space(AS_IO).install_read_handler(0xf9, 0xf9, read8smo_delegate(*this, FUNC(redline_state::offroad_wheel_1_r)));
	m_master->space(AS_IO).install_read_handler(0xfb, 0xfb, read8smo_delegate(*this, FUNC(redline_state::offroad_wheel_2_r)));
}


void redline_state::init_offroadt()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::offroad_bankswitch;

	rotate_memory("master");
	rotate_memory("slave");
	rotate_memory("slave");

	/* set up the master CPU I/O ports */
	init_master_ports(0x80, 0x40);

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_handler(0xf8, 0xf8, read8smo_delegate(*this, FUNC(redline_state::offroad_wheel_3_r)));
	m_master->space(AS_IO).install_read_handler(0xf9, 0xf9, read8smo_delegate(*this, FUNC(redline_state::offroad_wheel_1_r)));
	m_master->space(AS_IO).install_read_handler(0xfb, 0xfb, read8smo_delegate(*this, FUNC(redline_state::offroad_wheel_2_r)));
}


void redline_state::init_pigout()
{
	/* master CPU bankswitching */
	m_update_master_bank = &leland_state::offroad_bankswitch;

	rotate_memory("master");
	rotate_memory("slave");
	rotate_memory("slave");

	/* set up the master CPU I/O ports */
	init_master_ports(0x00, 0x40);

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_port(0x7f, 0x7f, "IN4");
}


void ataxx_state::init_ataxx()
{
	rotate_memory("master");
	rotate_memory("slave");

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_handler(0x00, 0x03, read8sm_delegate(*this, FUNC(ataxx_state::ataxx_trackball_r)));
}


void ataxx_state::init_ataxxj()
{
	rotate_memory("master");
	rotate_memory("slave");

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_handler(0x00, 0x03, read8sm_delegate(*this, FUNC(ataxx_state::ataxx_trackball_r)));
}


void ataxx_state::init_wsf()
{
	rotate_memory("master");
	rotate_memory("slave");

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_port(0x0d, 0x0d, "P1_P2");
	m_master->space(AS_IO).install_read_port(0x0e, 0x0e, "P3_P4");
	m_master->space(AS_IO).install_read_port(0x0f, 0x0f, "BUTTONS");
}


void ataxx_state::init_indyheat()
{
	rotate_memory("master");
	rotate_memory("slave");

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_handler(0x00, 0x02, read8sm_delegate(*this, FUNC(ataxx_state::ataxx_trackball_r)));
	m_master->space(AS_IO).install_read_handler(0x08, 0x0b, read8sm_delegate(*this, FUNC(ataxx_state::indyheat_analog_r)));
	m_master->space(AS_IO).install_read_port(0x0d, 0x0d, "P1");
	m_master->space(AS_IO).install_read_port(0x0e, 0x0e, "P2");
	m_master->space(AS_IO).install_read_port(0x0f, 0x0f, "P3");

	/* set up additional output ports */
	m_master->space(AS_IO).install_write_handler(0x08, 0x0b, write8sm_delegate(*this, FUNC(ataxx_state::indyheat_analog_w)));
}


void ataxx_state::init_brutforc()
{
	rotate_memory("master");
	rotate_memory("slave");

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_port(0x0d, 0x0d, "P2");
	m_master->space(AS_IO).install_read_port(0x0e, 0x0e, "P1");
	m_master->space(AS_IO).install_read_port(0x0f, 0x0f, "P3");
}


void ataxx_state::init_asylum()
{
	rotate_memory("master");
	rotate_memory("slave");

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_port(0x0d, 0x0d, "P2");
	m_master->space(AS_IO).install_read_port(0x0e, 0x0e, "P1");
	m_master->space(AS_IO).install_read_port(0x0f, 0x0f, "P3");
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

/* small master banks, small slave banks */
GAME( 1985, cerberus,   0,        leland,   cerberus,   leland_state,  init_cerberus, ROT0,   "Cinematronics", "Cerberus", 0 )
GAME( 1985, mayhem,     0,        leland,   mayhem,     leland_state,  init_mayhem,   ROT0,   "Cinematronics", "Mayhem 2002", 0 )
GAME( 1985, powrplay,   0,        leland,   mayhem,     leland_state,  init_powrplay, ROT0,   "Cinematronics", "Power Play", 0 )
GAME( 1985, wseries,    0,        leland,   wseries,    leland_state,  init_wseries,  ROT0,   "Cinematronics", "World Series: The Season (rev 1)", 0 )
GAME( 1985, wseries0,   wseries,  leland,   wseries,    leland_state,  init_wseries,  ROT0,   "Cinematronics", "World Series: The Season (rev 0)", 0 )
GAME( 1986, alleymas,   0,        leland,   alleymas,   leland_state,  init_alleymas, ROT270, "Cinematronics", "Alley Master", 0 )
GAME( 1986, upyoural,   alleymas, leland,   upyoural,   leland_state,  init_upyoural, ROT270, "Cinematronics", "Up Your Alley", 0 ) // prototype of Alley Master?

/* odd master banks, small slave banks */
GAME( 1986, dangerz,    0,        leland,   dangerz,    leland_state,  init_dangerz,  ROT0,   "Cinematronics", "Danger Zone (rev 2)", 0 )
GAME( 1986, dangerz0,   dangerz,  leland,   dangerz,    leland_state,  init_dangerz,  ROT0,   "Cinematronics", "Danger Zone (rev 0)", 0 )

/* small master banks + extra top board, small slave banks */
GAME( 1987, basebal2,   0,        leland,   basebal2,   leland_state,  init_basebal2, ROT0,   "Cinematronics", "Baseball: The Season II", 0 )
GAME( 1987, dblplay,    0,        leland,   basebal2,   leland_state,  init_dblplay,  ROT0,   "Leland Corporation / Tradewest", "Super Baseball Double Play Home Run Derby", 0 )
GAME( 1988, strkzone,   0,        leland,   basebal2,   leland_state,  init_strkzone, ROT0,   "Leland Corporation", "Strike Zone Baseball", 0 )

/* large master banks, small slave banks, 80186 sound */
GAME( 1987, redlin2p,   0,        redline,  redline,    redline_state, init_redlin2p, ROT270, "Cinematronics (Tradewest license)", "Redline Racer (2 players)", 0 )
GAME( 1987, quarterb,   0,        quarterb, quarterb,   redline_state, init_quarterb, ROT270, "Leland Corporation", "Quarterback (rev 5)", 0 )
GAME( 1987, quarterba,  quarterb, quarterb, quarterb,   redline_state, init_quarterb, ROT270, "Leland Corporation", "Quarterback (rev 2)", 0 )
GAME( 1987, quarterbc,  quarterb, quarterb, quarterb,   redline_state, init_quarterb, ROT270, "Leland Corporation", "Quarterback (rev 1, cocktail)", 0 )

/* large master banks, large slave banks, 80186 sound */
GAME( 1988, viper,      0,        lelandi,  dangerz,    redline_state, init_viper,    ROT0,   "Leland Corporation", "Viper (rev 3)", 0 )
GAME( 1988, teamqb,     0,        lelandi,  teamqb,     redline_state, init_teamqb,   ROT270, "Leland Corporation", "John Elway's Team Quarterback (rev 3)", 0 )
GAME( 1988, teamqb2,    teamqb,   lelandi,  teamqb,     redline_state, init_teamqb,   ROT270, "Leland Corporation", "John Elway's Team Quarterback (rev 2)", 0 )
GAME( 1989, aafb,       0,        lelandi,  teamqb,     redline_state, init_aafb,     ROT270, "Leland Corporation", "All American Football (rev E)", 0 )
GAME( 1989, aafbd2p,    aafb,     lelandi,  aafb2p,     redline_state, init_aafbd2p,  ROT270, "Leland Corporation", "All American Football (rev D, 2 players)", 0 )
GAME( 1989, aafbc,      aafb,     lelandi,  teamqb,     redline_state, init_aafbb,    ROT270, "Leland Corporation", "All American Football (rev C)", 0 )
GAME( 1989, aafbb,      aafb,     lelandi,  teamqb,     redline_state, init_aafbb,    ROT270, "Leland Corporation", "All American Football (rev B)", MACHINE_NOT_WORKING )

/* huge master banks, large slave banks, 80186 sound */
GAME( 1989, offroad,    0,        lelandi,  offroad,    redline_state, init_offroad,  ROT0,   "Leland Corporation", "Ironman Ivan Stewart's Super Off-Road (rev 4)", 0 )
GAME( 1989, offroad3,   offroad,  lelandi,  offroad,    redline_state, init_offroad,  ROT0,   "Leland Corporation", "Ironman Ivan Stewart's Super Off-Road (rev 3)", 0 )
GAME( 1989, offroadt,   0,        lelandi,  offroad,    redline_state, init_offroadt, ROT0,   "Leland Corporation", "Ironman Ivan Stewart's Super Off-Road Track-Pak (rev 4?)", 0 ) // need to verify revision
GAME( 1989, offroadt2p, offroadt, lelandi,  offroadt2p, redline_state, init_offroadt, ROT0,   "Leland Corporation", "Ironman Ivan Stewart's Super Off-Road Track-Pak (rev 4, 2 players)", 0 )
GAME( 1990, pigout,     0,        lelandi,  pigout,     redline_state, init_pigout,   ROT0,   "Leland Corporation", "Pig Out: Dine Like a Swine! (rev 2?)", 0 ) // need to verify revision
GAME( 1990, pigouta,    pigout,   lelandi,  pigout,     redline_state, init_pigout,   ROT0,   "Leland Corporation", "Pig Out: Dine Like a Swine! (rev 1)", 0 )

/* Ataxx-era PCB, 80186 sound */
GAME( 1990, ataxx,      0,        ataxx,    ataxx,      ataxx_state,   init_ataxx,    ROT0,   "Leland Corporation", "Ataxx (rev 5)", 0 )
GAME( 1990, ataxxa,     ataxx,    ataxx,    ataxx,      ataxx_state,   init_ataxx,    ROT0,   "Leland Corporation", "Ataxx (rev 4)", 0 )
GAME( 1990, ataxxe,     ataxx,    ataxx,    ataxx,      ataxx_state,   init_ataxx,    ROT0,   "Leland Corporation", "Ataxx (Europe)", 0 )
GAME( 1990, ataxxj,     ataxx,    ataxx,    ataxx,      ataxx_state,   init_ataxxj,   ROT0,   "Leland Corporation (Capcom license)", "Ataxx (Japan)", 0 )
GAME( 1990, wsf,        0,        wsf,      wsf,        ataxx_state,   init_wsf,      ROT0,   "Leland Corporation", "World Soccer Finals (rev 4)", 0 )
GAME( 1990, wsf3,       wsf,      wsf,      wsf,        ataxx_state,   init_wsf,      ROT0,   "Leland Corporation", "World Soccer Finals (rev 3)", 0 )
GAME( 1991, indyheat,   0,        wsf,      indyheat,   ataxx_state,   init_indyheat, ROT0,   "Leland Corporation", "Danny Sullivan's Indy Heat (rev 1)", 0 )
GAME( 1991, brutforc,   0,        wsf,      brutforc,   ataxx_state,   init_brutforc, ROT0,   "Leland Corporation", "Brute Force", 0 )
GAME( 1991, asylum,     0,        asylum,   brutforc,   ataxx_state,   init_asylum,   ROT270, "Leland Corporation", "Asylum (prototype)", 0 )

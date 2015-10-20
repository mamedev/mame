// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi,Brad Oliver
/***************************************************************************

Playchoice 10 - (c) 1986 Nintendo of America

    Written by Ernesto Corvi.

    Portions of this code are heavily based on
    Brad Oliver's MESS implementation of the NES.

    Thanks to people that contributed to this driver, namely:
    - Brad Oliver.
    - Aaron Giles.

****************************************************************************

BIOS:
    Memory Map
    ----------
    0000 - 3fff = Program ROM (8T)
    8000 - 87ff = RAM (8V)
    8800 - 8fff = RAM (8W)
    9000 - 97ff = SRAM (8R - Videoram)
    Cxxx = /INST ROM SEL
    Exxx = /IDSEL

    Input Ports -----------
    Read:
    - Port 0
    bit0 = CHSelect(?)
    bit1 = Enter button
    bit2 = Reset button
    bit3 = INTDETECT
    bit4 = N/C
    bit5 = Coin 2
    bit6 = Service button
    bit7 = Coin 1
    - Port 1 = Dipswitch 1
    - Port 2 = Dipswitch 2
    - Port 3 = /DETECTCLR

    Write: (always bit 0)
    - Port 0 = SDCS (ShareD CS)
    - Port 1 = /CNTRLMASK
    - Port 2 = /DISPMASK
    - Port 3 = /SOUNDMASK
    - Port 4 = /GAMERES
    - Port 5 = /GAMESTOP
    - Port 6 = N/C
    - Port 7 = N/
    - Port 8 = NMI Enable
    - Port 9 = DOG DI
    - Port A = /PPURES
    - Port B = CSEL0 \
    - Port C = CSEL1  \ (Cartridge select: 0 to 9)
    - Port D = CSEL2  /
    - Port E = CSEL3 /
    - Port F = 8UP KEY

****************************************************************************

Working games:
--------------
    - 1942                              (NF) - Standard board
    - Balloon Fight                     (BF) - Standard board
    - Baseball                          (BA) - Standard board
    - Baseball Stars                    (B9) - F board
    - Captain Sky Hawk                  (YW) - i board
    - Castlevania                       (CV) - B board
    - Contra                            (CT) - B board
    - Double Dragon                     (WD) - F board
    - Double Dribble                    (DW) - B board
    - Dr. Mario                         (VU) - F board
    - Duck Hunt                         (DH) - Standard board
    - Excite Bike                       (EB) - Standard board
    - Fester's Quest                    (EQ) - F board
    - Gauntlet                          (GL) - G board
    - Golf                              (GF) - Standard board
    - Gradius                           (GR) - A board
    - Hogan's Alley                     (HA) - Standard board
    - Kung Fu                           (SX) - Standard board
    - Mario Bros.                       (MA) - Standard board
    - Mario Open Golf                   (UG) - K board
    - Mega Man 3                        (XU) - G board
    - Metroid                           (MT) - D board
    - Ninja Gaiden                      (NG) - F board
    - Ninja Gaiden 2                    (NW) - G board
    - Ninja Gaiden 3                    (3N) - G board
    - Nintendo World Cup                (XZ) - G board
    - Pinbot                            (IO) - H board
    - Power Blade                       (7T) - G board
    - Pro Wrestling                     (PW) - B board
    - Rad Racer                         (RC) - D board
    - Rad Racer II                      (QR) - G board
    - RC Pro Am                         (PM) - F board
    - Rescue Rangers                    (RU) - F board
    - Rockin' Kats                      (7A) - G board
    - Rush N' Attack                    (RA) - B board
    - Rygar                             (RY) - B board
    - Solar Jetman                      (LJ) - i board
    - Super C                           (UE) - G board
    - Super Mario Bros                  (SM) - Standard board
    - Super Mario Bros 2                (MW) - G board
    - Super Mario Bros 3                (UM) - G board
    - Tecmo Bowl                        (TW) - F board
    - Teenage Mutant Ninja Turtles      (U2) - F board
    - Teenage Mutant Ninja Turtles 2    (2N) - G board
    - Tennis                            (TE) - Standard board
    - Track & Field                     (TR) - A board
    - Trojan                            (TJ) - B board
    - The Goonies                       (GN) - C board
    - Volley Ball                       (VB) - Standard board
    - Wild Gunman                       (WG) - Standard board
    - Yo Noid                           (YC) - F board

Non working games due to mapper/nes emulation issues:
-----------------------------------------------------
    - Mike Tyson's Punchout             (PT) - E board

Non working games due to missing roms:
--------------------------------------
    - ShatterHand                       (??) - ? board

****************************************************************************

Dipswitches information:
------------------------
Steph 2000.09.07

The 6 first DSWA (A-F) are used for coinage (units of time given for coin A/coin B)
When bit 6 of DSWB (O) is ON, units of time given for coin B are divided by 2

The 6 first DSWB (I-N) are used to set timer speed :
    [0x80d5] = ( ( (IN A,02) | 0xc0 ) + 0x3c ) & 0xff

When bit 7 of DSWB (P) is ON, you're in 'Freeplay' mode with 9999 units of time ...
However, this is effective ONLY if 7 other DSWB (I-O) are OFF !

I add the 32 combinations for coinage.

As I don't know what is the default value for timer speed, and I don't want to write
the 64 combinaisons, I only put some values ... Feel free to add the other ones ...

 DSW A    DSW B
HGFEDCBA PONMLKJI    coin A  coin B

xx000000 x0xxxxxx      300       0
xx000001 x0xxxxxx      300     100
xx000010 x0xxxxxx      300     200
xx000011 x0xxxxxx      300     300
xx000100 x0xxxxxx      300     400
xx000101 x0xxxxxx      300     500
xx000110 x0xxxxxx      300     600
xx000111 x0xxxxxx      300     700
xx001000 x0xxxxxx      300     800
xx001001 x0xxxxxx      300     900
xx001010 x0xxxxxx      150       0
xx001011 x0xxxxxx      150     200
xx001100 x0xxxxxx      150     400
xx001101 x0xxxxxx      150     600
xx001110 x0xxxxxx      150     800
xx001111 x0xxxxxx      150     500
xx010000 x0xxxxxx      300    1000
xx010001 x0xxxxxx      300    1100
xx010010 x0xxxxxx      300    1200
xx010011 x0xxxxxx      300    1300
xx010100 x0xxxxxx      300    1400
xx010101 x0xxxxxx      300    1500
xx010110 x0xxxxxx      300    1600
xx010111 x0xxxxxx      300    1700
xx011000 x0xxxxxx      300    1800
xx011001 x0xxxxxx      300    1900
xx011010 x0xxxxxx      150    1000
xx011011 x0xxxxxx      150    1200
xx011100 x0xxxxxx      150    1400
xx011101 x0xxxxxx      150    1600
xx011110 x0xxxxxx      150    1800
xx011111 x0xxxxxx      150    1500
xx100000 x0xxxxxx      300    2000
xx100001 x0xxxxxx      300    2100
xx100010 x0xxxxxx      300    2200
xx100011 x0xxxxxx      300    2300
xx100100 x0xxxxxx      300    2400
xx100101 x0xxxxxx      300    2500
xx100110 x0xxxxxx      300    2600
xx100111 x0xxxxxx      300    2700
xx101000 x0xxxxxx      300    2800
xx101001 x0xxxxxx      300    2900
xx101010 x0xxxxxx      150    2000
xx101011 x0xxxxxx      150    2200
xx101100 x0xxxxxx      150    2400
xx101101 x0xxxxxx      150    2600
xx101110 x0xxxxxx      150    2800
xx101111 x0xxxxxx      150    2500
xx110000 x0xxxxxx      300    3000
xx110001 x0xxxxxx      300    3100
xx110010 x0xxxxxx      300    3200
xx110011 x0xxxxxx      300    3300
xx110100 x0xxxxxx      300    3400
xx110101 x0xxxxxx      300    3500
xx110110 x0xxxxxx      300    3600
xx110111 x0xxxxxx      300    3700
xx111000 x0xxxxxx      300    3800
xx111001 x0xxxxxx      300    3900
xx111010 x0xxxxxx      150    3000
xx111011 x0xxxxxx      150    3200
xx111100 x0xxxxxx      150    3400
xx111101 x0xxxxxx      150    3600
xx111110 x0xxxxxx      150    3800
xx111111 x0xxxxxx      150    3500

xx000000 x1xxxxxx      300       0
xx000001 x1xxxxxx      300      50
xx000010 x1xxxxxx      300     100
xx000011 x1xxxxxx      300     150
xx000100 x1xxxxxx      300     200
xx000101 x1xxxxxx      300     250
xx000110 x1xxxxxx      300     300
xx000111 x1xxxxxx      300     350
xx001000 x1xxxxxx      300     400
xx001001 x1xxxxxx      300     450
xx001010 x1xxxxxx      150       0
xx001011 x1xxxxxx      150     100
xx001100 x1xxxxxx      150     200
xx001101 x1xxxxxx      150     300
xx001110 x1xxxxxx      150     400
xx001111 x1xxxxxx      150     250
xx010000 x1xxxxxx      300     500
xx010001 x1xxxxxx      300     550
xx010010 x1xxxxxx      300     600
xx010011 x1xxxxxx      300     650
xx010100 x1xxxxxx      300     700
xx010101 x1xxxxxx      300     750
xx010110 x1xxxxxx      300     800
xx010111 x1xxxxxx      300     850
xx011000 x1xxxxxx      300     900
xx011001 x1xxxxxx      300     950
xx011010 x1xxxxxx      150     500
xx011011 x1xxxxxx      150     600
xx011100 x1xxxxxx      150     700
xx011101 x1xxxxxx      150     800
xx011110 x1xxxxxx      150     750
xx100000 x1xxxxxx      300    1000
xx100001 x1xxxxxx      300    1050
xx100010 x1xxxxxx      300    1100
xx100011 x1xxxxxx      300    1150
xx100100 x1xxxxxx      300    1200
xx100101 x1xxxxxx      300    1250
xx100110 x1xxxxxx      300    1300
xx100111 x1xxxxxx      300    1350
xx101000 x1xxxxxx      300    1400
xx101001 x1xxxxxx      300    1450
xx101010 x1xxxxxx      150    1000
xx101011 x1xxxxxx      150    1100
xx101100 x1xxxxxx      150    1200
xx101101 x1xxxxxx      150    1300
xx101110 x1xxxxxx      150    1400
xx101111 x1xxxxxx      150    1250
xx110000 x1xxxxxx      300    1500
xx110001 x1xxxxxx      300    1550
xx110010 x1xxxxxx      300    1600
xx110011 x1xxxxxx      300    1650
xx110100 x1xxxxxx      300    1700
xx110101 x1xxxxxx      300    1750
xx110110 x1xxxxxx      300    1800
xx110111 x1xxxxxx      300    1850
xx111000 x1xxxxxx      300    1900
xx111001 x1xxxxxx      300    1950
xx111010 x1xxxxxx      150    1500
xx111011 x1xxxxxx      150    1600
xx111100 x1xxxxxx      150    1700
xx111101 x1xxxxxx      150    1800
xx111110 x1xxxxxx      150    1750

I know that the way I code the DSW isn't correct, but I don't know how to link
O to A-F AND, at the same time, O to P ... Any help is appreciated ...

****************************************************************************

Notes & Todo:
-------------

- Look at Ninja Gaiden 3. It has some slight timming issues on the
  second level. Probably related to the mapper's irq timming.
- Fix some remaining bad gfx in Rad Racer II.
- Implement Dipswitches properly once Mame can support it.
- Better control layout?. This thing has odd buttons.
- Find dumps of the rest of the RP5H01's and add the remaining games.
- Any PPU optimizations that retain accuracy are certainly welcome.

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/n2a03.h"
#include "cpu/z80/z80.h"
#include "machine/rp5h01.h"
#include "machine/nvram.h"
#include "sound/dac.h"

#include "rendlay.h"
#include "includes/playch10.h"

/* clock frequency */
#define N2A03_DEFAULTCLOCK (21477272.724 / 12)

/******************************************************************************/


WRITE8_MEMBER(playch10_state::up8w_w)
{
	m_up_8w = data & 1;
}

READ8_MEMBER(playch10_state::ram_8w_r)
{
	if ( offset >= 0x400 && m_up_8w )
		return m_ram_8w[offset];

	return m_ram_8w[offset & 0x3ff];
}

WRITE8_MEMBER(playch10_state::ram_8w_w)
{
	if ( offset >= 0x400 && m_up_8w )
		m_ram_8w[offset] = data;
	else
		m_ram_8w[offset & 0x3ff] = data;
}

WRITE8_MEMBER(playch10_state::sprite_dma_w)
{
	int source = ( data & 7 );
	m_ppu->spriteram_dma(space, source);
}

/* Only used in single monitor bios */

WRITE8_MEMBER(playch10_state::time_w)
{
	if(data == 0xf)
		data = 0;

	m_timedata[offset] = data;

	popmessage("Time: %d%d%d%d",m_timedata[3],m_timedata[2],m_timedata[1],m_timedata[0]);
}

READ8_MEMBER(playch10_state::psg_4015_r)
{
	return m_nesapu->read(space, 0x15);
}

WRITE8_MEMBER(playch10_state::psg_4015_w)
{
	m_nesapu->write(space, 0x15, data);
}

WRITE8_MEMBER(playch10_state::psg_4017_w)
{
	m_nesapu->write(space, 0x17, data);
}

/******************************************************************************/

/* BIOS */
static ADDRESS_MAP_START( bios_map, AS_PROGRAM, 8, playch10_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM // 8V
	AM_RANGE(0x8800, 0x8fff) AM_READWRITE(ram_8w_r, ram_8w_w) AM_SHARE("ram_8w")    // 8W
	AM_RANGE(0x9000, 0x97ff) AM_RAM_WRITE(playch10_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xc000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_READWRITE(pc10_prot_r, pc10_prot_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bios_io_map, AS_IO, 8, playch10_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("BIOS") AM_WRITE(pc10_SDCS_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("SW1") AM_WRITE(pc10_CNTRLMASK_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("SW2") AM_WRITE(pc10_DISPMASK_w)
	AM_RANGE(0x03, 0x03) AM_READWRITE(pc10_detectclr_r, pc10_SOUNDMASK_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(pc10_GAMERES_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(pc10_GAMESTOP_w)
	AM_RANGE(0x06, 0x07) AM_WRITENOP
	AM_RANGE(0x08, 0x08) AM_WRITE(pc10_NMIENABLE_w)
	AM_RANGE(0x09, 0x09) AM_WRITE(pc10_DOGDI_w)
	AM_RANGE(0x0a, 0x0a) AM_WRITE(pc10_PPURES_w)
	AM_RANGE(0x0b, 0x0e) AM_WRITE(pc10_CARTSEL_w)
	AM_RANGE(0x0f, 0x0f) AM_WRITE(up8w_w)
	AM_RANGE(0x10, 0x13) AM_WRITE(time_w) AM_SHARE("timedata")
ADDRESS_MAP_END

static ADDRESS_MAP_START( cart_map, AS_PROGRAM, 8, playch10_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_MIRROR(0x1800) AM_SHARE("work_ram")
	AM_RANGE(0x2000, 0x3fff) AM_DEVREADWRITE("ppu", ppu2c0x_device, read, write)
	AM_RANGE(0x4011, 0x4011) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(0x4000, 0x4013) AM_DEVREADWRITE("nesapu", nesapu_device, read, write)
	AM_RANGE(0x4014, 0x4014) AM_WRITE(sprite_dma_w)
	AM_RANGE(0x4015, 0x4015) AM_READWRITE(psg_4015_r, psg_4015_w)  /* PSG status / first control register */
	AM_RANGE(0x4016, 0x4016) AM_READWRITE(pc10_in0_r, pc10_in0_w)
	AM_RANGE(0x4017, 0x4017) AM_READ(pc10_in1_r) AM_WRITE(psg_4017_w) /* IN1 - input port 2 / PSG second control register */
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( playch10 )
	PORT_START("BIOS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Channel Select") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Enter") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Reset") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, playch10_state,pc10_int_detect_r, NULL)   // INT Detect
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

/*

    The correct way to handle DIPs according to the manual.
    Doesn't work due to limitations of the conditional DIPs
    implementation in MAME.


    PORT_START("SW1")
    PORT_DIPNAME( 0x3f, 0x09, "Prime Time Bonus" )
    // STANDARD TIME (no bonus)
    PORT_DIPSETTING(    0x00, "0%" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    // PRIME TIME (bonus) for 2 COINS
    PORT_DIPSETTING(    0x07, "8%" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x08, "17%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x09, "25%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x10, "33%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x11, "42%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x12, "50%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x13, "58%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x14, "67%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x15, "75%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x16, "83%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x17, "92%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x18, "100%" )  PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    // PRIME TIME (bonus) for 4 COINS
    PORT_DIPSETTING(    0x04, "8%" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x05, "17%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x06, "25%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x07, "33%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x08, "42%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x09, "50%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x10, "58%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x11, "67%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x12, "75%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x13, "83%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x14, "92%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x15, "100%" )  PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )

    PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
    PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x40, DEF_STR( On ) )
    PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

    PORT_START("SW2")
    PORT_DIPNAME( 0x3f, 0x28, "Play Time/Coin" )
    // STANDARD TIME (no bonus)
    PORT_DIPSETTING(    0x3f, DEF_STR( Free_Play ) )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x23, "2:00 (120)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x21, "2:10 (130)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x1f, "2:20 (140)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x1d, "2:30 (150)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x1b, "2:40 (160)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x19, "2:50 (170)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x17, "3:00 (180)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x15, "3:10 (190)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x13, "3:20 (200)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x11, "3:30 (210)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x0f, "3:40 (220)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x0d, "3:50 (230)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x0b, "4:00 (240)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x09, "4:10 (250)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x07, "4:20 (260)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x05, "4:30 (270)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x03, "4:40 (280)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x01, "4:50 (290)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    // PRIME TIME (bonus) for 2 COINS
    PORT_DIPSETTING(    0x1c, "2:00 (120)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x1e, "2:10 (130)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x20, "2:20 (140)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x22, "2:30 (150)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x24, "2:40 (160)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x26, "2:50 (170)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x28, "3:00 (180)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x2a, "3:10 (190)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x2c, "3:20 (200)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x2e, "3:30 (210)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x30, "3:40 (220)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x32, "3:50 (230)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x34, "4:00 (240)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    // PRIME TIME (bonus) for 4 COINS
    PORT_DIPSETTING(    0x1c, "2:00 (120)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x1e, "2:10 (130)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x20, "2:20 (140)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x22, "2:30 (150)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x24, "2:40 (160)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x26, "2:50 (170)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x28, "3:00 (180)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x2a, "3:10 (190)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x2c, "3:20 (200)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x2e, "3:30 (210)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x30, "3:40 (220)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x32, "3:50 (230)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x34, "4:00 (240)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )

    PORT_DIPNAME( 0xc0, 0x80, "Bonus" )
    PORT_DIPSETTING(    0xc0, "Standard Time" )
    PORT_DIPSETTING(    0x80, "Prime Time for 2 Coins" )
    PORT_DIPSETTING(    0x00, "Prime Time for 4 Coins" )
*/
	PORT_START("SW1")
	PORT_DIPNAME( 0x3f, 0x00, "Units of time (coin A/coin B)" )
	PORT_DIPSETTING(    0x00, "300/0" )
	PORT_DIPSETTING(    0x01, "300/100" )
	PORT_DIPSETTING(    0x02, "300/200" )
	PORT_DIPSETTING(    0x03, "300/300" )
	PORT_DIPSETTING(    0x04, "300/400" )
	PORT_DIPSETTING(    0x05, "300/500" )
	PORT_DIPSETTING(    0x06, "300/600" )
	PORT_DIPSETTING(    0x07, "300/700" )
	PORT_DIPSETTING(    0x08, "300/800" )
	PORT_DIPSETTING(    0x09, "300/900" )
	PORT_DIPSETTING(    0x0a, "150/0" )
	PORT_DIPSETTING(    0x0b, "150/200" )
	PORT_DIPSETTING(    0x0c, "150/400" )
	PORT_DIPSETTING(    0x0f, "150/500" )
	PORT_DIPSETTING(    0x0d, "150/600" )
	PORT_DIPSETTING(    0x0e, "150/800" )
	PORT_DIPSETTING(    0x10, "300/1000" )
	PORT_DIPSETTING(    0x11, "300/1100" )
	PORT_DIPSETTING(    0x12, "300/1200" )
	PORT_DIPSETTING(    0x13, "300/1300" )
	PORT_DIPSETTING(    0x14, "300/1400" )
	PORT_DIPSETTING(    0x15, "300/1500" )
	PORT_DIPSETTING(    0x16, "300/1600" )
	PORT_DIPSETTING(    0x17, "300/1700" )
	PORT_DIPSETTING(    0x18, "300/1800" )
	PORT_DIPSETTING(    0x19, "300/1900" )
	PORT_DIPSETTING(    0x1a, "150/1000" )
	PORT_DIPSETTING(    0x1b, "150/1200" )
	PORT_DIPSETTING(    0x1c, "150/1400" )
	PORT_DIPSETTING(    0x1f, "150/1500" )
	PORT_DIPSETTING(    0x1d, "150/1600" )
	PORT_DIPSETTING(    0x1e, "150/1800" )
	PORT_DIPSETTING(    0x20, "300/2000" )
	PORT_DIPSETTING(    0x21, "300/2100" )
	PORT_DIPSETTING(    0x22, "300/2200" )
	PORT_DIPSETTING(    0x23, "300/2300" )
	PORT_DIPSETTING(    0x24, "300/2400" )
	PORT_DIPSETTING(    0x25, "300/2500" )
	PORT_DIPSETTING(    0x26, "300/2600" )
	PORT_DIPSETTING(    0x27, "300/2700" )
	PORT_DIPSETTING(    0x28, "300/2800" )
	PORT_DIPSETTING(    0x29, "300/2900" )
	PORT_DIPSETTING(    0x2a, "150/2000" )
	PORT_DIPSETTING(    0x2b, "150/2200" )
	PORT_DIPSETTING(    0x2c, "150/2400" )
	PORT_DIPSETTING(    0x2f, "150/2500" )
	PORT_DIPSETTING(    0x2d, "150/2600" )
	PORT_DIPSETTING(    0x2e, "150/2800" )
	PORT_DIPSETTING(    0x30, "300/3000" )
	PORT_DIPSETTING(    0x31, "300/3100" )
	PORT_DIPSETTING(    0x32, "300/3200" )
	PORT_DIPSETTING(    0x33, "300/3300" )
	PORT_DIPSETTING(    0x34, "300/3400" )
	PORT_DIPSETTING(    0x35, "300/3500" )
	PORT_DIPSETTING(    0x36, "300/3600" )
	PORT_DIPSETTING(    0x37, "300/3700" )
	PORT_DIPSETTING(    0x38, "300/3800" )
	PORT_DIPSETTING(    0x39, "300/3900" )
	PORT_DIPSETTING(    0x3a, "150/3000" )
	PORT_DIPSETTING(    0x3b, "150/3200" )
	PORT_DIPSETTING(    0x3c, "150/3400" )
	PORT_DIPSETTING(    0x3f, "150/3500" )
	PORT_DIPSETTING(    0x3d, "150/3600" )
	PORT_DIPSETTING(    0x3e, "150/3800" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("SW2")
	PORT_DIPNAME( 0x40, 0x00, "Coin Mode" )
	PORT_DIPSETTING(    0x00, "Mode 1" )
	PORT_DIPSETTING(    0x40, "Mode 2" )
	PORT_DIPNAME( 0xbf, 0x3f, "Timer speed" )
	PORT_DIPSETTING(    0x05, "60 units per second" )
	PORT_DIPSETTING(    0x06, "30 units per second" )
	PORT_DIPSETTING(    0x07, "20 units per second" )
	PORT_DIPSETTING(    0x08, "15 units per second" )
	PORT_DIPSETTING(    0x0a, "10 units per second" )
	PORT_DIPSETTING(    0x0e, "6 units per second" )
	PORT_DIPSETTING(    0x10, "5 units per second" )
	PORT_DIPSETTING(    0x13, "4 units per second" )
	PORT_DIPSETTING(    0x18, "3 units per second" )
	PORT_DIPSETTING(    0x22, "2 units per second" )
	PORT_DIPSETTING(    0x3f, "1 unit per second" )
	PORT_DIPSETTING(    0x00, "1 unit every 4 seconds" )
	PORT_DIPSETTING(    0x80, DEF_STR( Free_Play ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Button B")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Button A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Game Select")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Button A") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Button B") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )    // wired to 1p select button
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )    // wired to 1p start button
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
INPUT_PORTS_END

/*Input Ports for gun games*/
static INPUT_PORTS_START( playc10g )
	PORT_INCLUDE(playch10)

	PORT_START("GUNX")  /* IN2 - FAKE - Gun X pos */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(30)

	PORT_START("GUNY")  /* IN3 - FAKE - Gun Y pos */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(30)
INPUT_PORTS_END


static const gfx_layout bios_charlayout =
{
	8,8,    /* 8*8 characters */
	1024,   /* 1024 characters */
	3,      /* 3 bits per pixel */
	{ 0, 0x2000*8, 0x4000*8 },     /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( playch10 )
	GFXDECODE_ENTRY( "gfx1", 0, bios_charlayout,   0,  32 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(playch10_state::playch10_interrupt){
	/* LS161A, Sheet 1 - bottom left of Z80 */
	if ( !m_pc10_dog_di && !m_pc10_nmi_enable ) {
		device.execute().set_input_line(INPUT_LINE_RESET, PULSE_LINE );
	}

	else if ( m_pc10_nmi_enable )
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( playch10, playch10_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, 8000000/2) // 4 MHz
	MCFG_CPU_PROGRAM_MAP(bios_map)
	MCFG_CPU_IO_MAP(bios_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("top", playch10_state,  playch10_interrupt)

	MCFG_CPU_ADD("cart", N2A03, N2A03_DEFAULTCLOCK)
	MCFG_CPU_PROGRAM_MAP(cart_map)


	// video hardware
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", playch10)
	MCFG_PALETTE_ADD("palette", 256+8*4*16)
	MCFG_PALETTE_INIT_OWNER(playch10_state, playch10)
	MCFG_DEFAULT_LAYOUT(layout_dualhuov)

	MCFG_SCREEN_ADD("top", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(32*8, 262)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(playch10_state, screen_update_playch10_top)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_SCREEN_ADD("bottom", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(32*8, 262)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(playch10_state, screen_update_playch10_bottom)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PPU2C03B_ADD("ppu")
	MCFG_PPU2C0X_SET_SCREEN("bottom")
	MCFG_PPU2C0X_CPU("cart")
	MCFG_PPU2C0X_COLORBASE(256)
	MCFG_PPU2C0X_SET_NMI(playch10_state, ppu_irq)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("nesapu", NES_APU, N2A03_DEFAULTCLOCK)
	MCFG_NES_APU_CPU("cart")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_RP5H01_ADD("rp5h01")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( playchnv, playch10 )
	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( playch10_hboard, playch10 )
	MCFG_VIDEO_START_OVERRIDE(playch10_state,playch10_hboard)
	MCFG_MACHINE_START_OVERRIDE(playch10_state,playch10_hboard)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
	ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios+1)) /* Note '+1' */

#define BIOS_CPU                                            \
	ROM_REGION( 0x10000, "maincpu", 0 )                     \
	ROM_SYSTEM_BIOS( 0, "dual",   "Dual Monitor Version" ) \
	ROM_LOAD_BIOS( 0, "pch1-c__8t_e-2.8t", 0x00000, 0x4000, CRC(d52fa07a) SHA1(55cabf52ae10c050c2229081a80b9fe5454ab8c5) ) \
	ROM_SYSTEM_BIOS( 1, "single", "Single Monitor Version" ) \
	ROM_LOAD_BIOS( 1, "pck1-c.8t", 0x00000, 0x4000, CRC(503ee8b1) SHA1(3bd20bc71cac742d1b8c1430a6426d0a19db7ad0) )
#define BIOS_GFX                                            \
	ROM_REGION( 0x6000, "gfx1", 0 ) \
	ROM_LOAD( "pch1-c__8p_e-1.8p",    0x00000, 0x2000, CRC(30c15e23) SHA1(69166afdb2fe827c7f1919cdf4197caccbd961fa) )   \
	ROM_LOAD( "pch1-c__8m_e-1.8m",    0x02000, 0x2000, CRC(c1232eee) SHA1(beaf9fa2d091a3c7f70c51e966d885b1f9f0935f) )   \
	ROM_LOAD( "pch1-c__8k.8k",    0x04000, 0x2000, CRC(9acffb30) SHA1(b814f10ef23f2ca445fabafcbf7f25e2d454ba8c) )   \
	ROM_REGION( 0x0300, "proms", 0 )                        \
	ROM_LOAD( "pch1-c-6f.82s129an.6f",    0x0000, 0x0100, CRC(e5414ca3) SHA1(d2878411cda84ffe0afb2e538a67457f51bebffb) )    \
	ROM_LOAD( "pch1-c-6e.82s129an.6e",    0x0100, 0x0100, CRC(a2625c6e) SHA1(a448b47c9289902e26a3d3c4c7d5a7968c385e81) )    \
	ROM_LOAD( "pch1-c-6d.82s129an.6d",    0x0200, 0x0100, CRC(1213ebd4) SHA1(0ad386fc3eab5e53c0288ad1de33639a9e461b7c) )    \
	ROM_REGION( 0xc0, "palette", 0 )                        \
	ROM_LOAD( "rp2c0x.pal", 0x00, 0xc0, CRC(48de65dc) SHA1(d10acafc8da9ff479c270ec01180cca61efe62f5) )

/******************************************************************************/

/* Standard Games */
ROM_START( pc_smb )     /* Super Mario Bros. */
	BIOS_CPU
	ROM_LOAD( "u3sm",    0x0c000, 0x2000, CRC(4b5f717d) SHA1(c39c90f9503c4692af4a8fdb3e18ef7cf04e897f) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x10000, "cart", 0 )
	ROM_LOAD( "u1sm",    0x08000, 0x8000, CRC(5cf548d3) SHA1(fefa1097449a3a11ebf8c6199e905996c5dc8fbd) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "u2sm",    0x00000, 0x2000, CRC(867b51ad) SHA1(394badaf0b0bdd0ea279a1bca89a9d9ddc00b1b5) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(bd82d775) SHA1(e15c369d638156eeb0cd141aeeec877c62810b64) )
ROM_END

ROM_START( pc_ebike )   /* Excite Bike */
	BIOS_CPU
	ROM_LOAD( "u3eb",    0x0c000, 0x2000, CRC(8ff0e787) SHA1(35a6d7186dee4fd4ba015ec0db5181768411aa3c) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x10000, "cart", 0 )
	ROM_LOAD( "u1eb",    0x0c000, 0x4000, CRC(3a94fa0b) SHA1(6239e91ccefdc017d233cbae388c6568a17ed04b) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "u2eb",    0x00000, 0x2000, CRC(e5f72401) SHA1(a8bf028e1a62677e48e88cf421bb2a8051eb800c) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(a0263750) SHA1(2ab6e43c2bc4c07fc7887defc4fc81502167ef60) )
ROM_END

ROM_START( pc_1942 )    /* 1942 */
	BIOS_CPU
	ROM_LOAD( "u3",      0x0c000, 0x2000, CRC(415b8807) SHA1(9d6161bbc6dec5873cc6d8a570141d4af42fa232) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x10000, "cart", 0 )
	ROM_LOAD( "u1",      0x08000, 0x8000, CRC(c4e8c04a) SHA1(d608f769333b13da9c67f07599e405944893a950) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "u2",      0x00000, 0x2000, CRC(03379b76) SHA1(d2a6ca1cdd8935525f59f1d38806b2296cb12a12) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(29893c7f) SHA1(58478b7de2177c8dc1d6885bd34eeeeb5e46d7a3) )
ROM_END

ROM_START( pc_bfght )   /* Balloon Fight */
	BIOS_CPU
	ROM_LOAD( "bf-u3",   0x0c000, 0x2000, CRC(a9949544) SHA1(0bb9fab67769a4eaa1b903a3217dbb5ca6feddb8) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x10000, "cart", 0 )
	ROM_LOAD( "bf-u1",   0x0c000, 0x4000, CRC(575ed2fe) SHA1(63527ea590aa79a6b09896c35021de785fd40851) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "bf-u2",   0x00000, 0x2000, CRC(c642a1df) SHA1(e73cd3d4c0bad8e6f7a1aa6a580f3817a83756a9) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(be3c42fb) SHA1(da40c57bda36d1dbacdf246e0d2579b6be616480) )
ROM_END

ROM_START( pc_bball )   /* Baseball */
	BIOS_CPU
	ROM_LOAD( "ba-u3",   0x0c000, 0x2000, CRC(06861a0d) SHA1(b7263280a39f544ca4ab1b4d3e8c5fe17ea95e57) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x10000, "cart", 0 )
	ROM_LOAD( "ba-u1",   0x0c000, 0x4000, CRC(39d1fa03) SHA1(28d84cfefa81bbfd3d26e0f70f1b9f53383e54ad) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "ba-u2",   0x00000, 0x2000, CRC(cde71b82) SHA1(296ccef8a1fd9209f414ce0c788ab0dc95058242) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(7940cfc4) SHA1(9e36ceb5aac023472f48f2f06cf171bffa49a664) )
ROM_END

ROM_START( pc_golf )    /* Golf */
	BIOS_CPU
	ROM_LOAD( "gf-u3",   0x0c000, 0x2000, CRC(882dea87) SHA1(e3bbca36efa66231b933713dec032bbb926b36e5) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x10000, "cart", 0 )
	ROM_LOAD( "gf-u1",   0x0c000, 0x4000, CRC(f9622bfa) SHA1(b4e341a91f614bb19c67cc0205b2443591567aea) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "gf-u2",   0x00000, 0x2000, CRC(ff6fc790) SHA1(40177839b61f375f2ad03b203328683264845b5b) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(2cd98ef6) SHA1(bd5142c6a29df674ab835c8beafff7e93712d88f) )
ROM_END

ROM_START( pc_kngfu )   /* Kung Fu */
	BIOS_CPU
	ROM_LOAD( "sx-u3",   0x0c000, 0x2000, CRC(ead71b7e) SHA1(e255c08f92d6188dad6b27446b0117cd7cee4364) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x10000, "cart", 0 )
	ROM_LOAD( "sx-u1",   0x08000, 0x8000, CRC(0516375e) SHA1(55dc3550c6133f8624eb6cf3d2f145e4313c2ff6) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "sx-u2",   0x00000, 0x2000, CRC(430b49a4) SHA1(7e618dbff521c3d5ee0f3d8bb01d2e770395a6bc) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(a1687f01) SHA1(ff4c3e925ece14acfa6f51c87af310ebbe3af638) )
ROM_END

ROM_START( pc_tenis )   /* Tennis */
	BIOS_CPU
	ROM_LOAD( "te-u3",   0x0c000, 0x2000, CRC(6928e920) SHA1(0bdc64a6f37d8cf5e8efacc5004a6ae43a28cd60) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x10000, "cart", 0 )
	ROM_LOAD( "te-u1",   0x0c000, 0x4000, CRC(8b2e3e81) SHA1(e54274c0b0d651458c5459d41872b1f99904d0fb) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "te-u2",   0x00000, 0x2000, CRC(3a34c45b) SHA1(2cc26a01c38ead50503dccb3ee929ba7a2b6772c) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(bcc9a48e) SHA1(a293898f17b627cdf8e7a1074ef30ad8c2392977) )
ROM_END

ROM_START( pc_vball )   /* Volley Ball */
	BIOS_CPU
	ROM_LOAD( "vb-u3",   0x0c000, 0x2000, CRC(9104354e) SHA1(84374b1df747800f7e70b5fb6a16fd3607b724c9) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x10000, "cart", 0 )
	ROM_LOAD( "vb-u1",   0x08000, 0x8000, CRC(35226b99) SHA1(548787ba5ca00290da4efc9af40054dc1889014c) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "vb-u2",   0x00000, 0x2000, CRC(2415dce2) SHA1(fd89b4a542989a89c2d0467257dca57518bfa96b) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(f701863f) SHA1(78614e6b8a78384d9aeab439eb8d53a3691dd0a1) )
ROM_END

ROM_START( pc_mario )   /* Mario Bros. */
	BIOS_CPU
	ROM_LOAD( "ma-u3",   0x0c000, 0x2000, CRC(a426c5c0) SHA1(0cf31de3eb18f17830dd9aa3a33fe4a6947f6ceb) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x10000, "cart", 0 )
	ROM_LOAD( "ma-u1",   0x0c000, 0x4000, CRC(75f6a9f3) SHA1(b6f88f7a2f9a49cc9182a244571730198f1edc4b) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "ma-u2",   0x00000, 0x2000, CRC(10f77435) SHA1(a646c3443832ada84d31a3a8a4b34aebc17cecd5) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(79006635) SHA1(10dcb24fb7717b993110512115ab04310dc637d0) )
ROM_END

/* Gun Games */
ROM_START( pc_duckh )   /* Duck Hunt */
	BIOS_CPU
	ROM_LOAD( "u3",      0x0c000, 0x2000, CRC(2f9ec5c6) SHA1(1e1b835339b030605841a032f066ccb5ca1fef20) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x10000, "cart", 0 )
	ROM_LOAD( "u1",      0x0c000, 0x4000, CRC(90ca616d) SHA1(b742576317cd6a04caac25252d5593844c9a0bb6) )

	ROM_REGION( 0x04000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "u2",      0x00000, 0x2000, CRC(4e049e03) SHA1(ffad32a3bab2fb3826bc554b1b9838e837513576) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(8cd6aad6) SHA1(4543cdb55c3521e1b5d61f82d4800c414658fd6d) )
ROM_END

ROM_START( pc_hgaly )   /* Hogan's Alley */
	BIOS_CPU
	ROM_LOAD( "ha-u3",   0x0c000, 0x2000, CRC(a2525180) SHA1(9c981c1679c59c7b7c069f7d1cb86cb8aa280f22) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x10000, "cart", 0 )
	ROM_LOAD( "ha-u1",   0x0c000, 0x4000, CRC(8963ae6e) SHA1(bca489ed0fb58e1e99f36c427bc0d7d805b6c61a) )

	ROM_REGION( 0x04000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "ha-u2",   0x00000, 0x2000, CRC(5df42fc4) SHA1(4fcf23151d9f11c1ef1b1007dd8058f5d5fe9ab8) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(5ac61521) SHA1(75d2ad958336061e70049272ce4c88bff182f96d) )
ROM_END

ROM_START( pc_wgnmn )   /* Wild Gunman */
	BIOS_CPU
	ROM_LOAD( "wg-u3",   0x0c000, 0x2000, CRC(da08afe5) SHA1(0f505ccee372a37971bad7bbbb7341336ee70f97) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x10000, "cart", 0 )
	ROM_LOAD( "wg-u1",   0x0c000, 0x4000, CRC(389960db) SHA1(6b38f2c86ef27f653a2bdb9c682ac0bc981c7db6) )

	ROM_REGION( 0x04000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "wg-u2",   0x00000, 0x2000, CRC(a5e04856) SHA1(9194d89a34f687742216889cbb3e717a9ae81c92) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(def015a3) SHA1(b542828a74744d87331821635777d7715e22a15b) )
ROM_END

/* A-Board Games */
ROM_START( pc_tkfld )   /* Track & Field */
	BIOS_CPU
	ROM_LOAD( "u4tr",    0x0c000, 0x2000, CRC(70184fd7) SHA1(bc6f6f942948ddf5a7130d9688f12ef5511a7a30) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x10000, "cart", 0 )
	ROM_LOAD( "u2tr",    0x08000, 0x8000, CRC(d7961e01) SHA1(064cb6e3e5525682a1805b01ba64f2fd75462496) )

	ROM_REGION( 0x08000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "u3tr",    0x00000, 0x8000, CRC(03bfbc4b) SHA1(ffc4e0e1d858fb4472423ae1c1fdc1e8197c30f0) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(1e2e7f1e) SHA1(4b65f5b217586653a1d0da96539cc9bc50d989e2) )
ROM_END

ROM_START( pc_grdus )   /* Gradius */
	BIOS_CPU
	ROM_LOAD( "gr-u4",   0x0c000, 0x2000, CRC(27d76160) SHA1(605d58c57969c831778b95356fcf103a1d5f98a3) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x10000, "cart", 0 )
	ROM_LOAD( "gr-u2",   0x08000, 0x8000, CRC(aa96889c) SHA1(e4380a7c0778541af8216e3ac1e14ff23fb074a9) )

	ROM_REGION( 0x08000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "gr-u3",   0x00000, 0x8000, CRC(de963bec) SHA1(ecb76b5897658ebac31a07516bb2a5820279474f) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(b8d5bf8a) SHA1(1c208fa5409b6e21aa576e1b9e086e830dc26a1a) )
ROM_END

ROM_START( pc_grdue )   /* Gradius (Early version) */
	BIOS_CPU
	ROM_LOAD( "gr-u4",   0x0c000, 0x2000, CRC(27d76160) SHA1(605d58c57969c831778b95356fcf103a1d5f98a3) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x10000, "cart", 0 )
	ROM_LOAD( "gr-u1e",  0x08000, 0x8000, CRC(9204a65d) SHA1(500693f8f65b1e2f09b722c5fa28b32088e22a29) )

	ROM_REGION( 0x08000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "gr-u3",   0x00000, 0x8000, CRC(de963bec) SHA1(ecb76b5897658ebac31a07516bb2a5820279474f) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(b8d5bf8a) SHA1(1c208fa5409b6e21aa576e1b9e086e830dc26a1a) )
ROM_END


/* B-Board Games */
ROM_START( pc_rnatk )   /* Rush N' Attack */
	BIOS_CPU
	ROM_LOAD( "ra-u4",   0x0c000, 0x2000, CRC(ebab7f8c) SHA1(ae46e46d878cdbc28cd42b40dae1fd1a6c1b31ed) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "ra-u1",   0x10000, 0x10000, CRC(5660b3a6) SHA1(4e7ad9be59990e4a560d87a1bac9b708074e9db1) ) /* banked */
	ROM_LOAD( "ra-u2",   0x20000, 0x10000, CRC(2a1bca39) SHA1(ca1eebf85bea85ce7bcdf38933ae495856e17ae1) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(1f6596b2) SHA1(e50780241ae3a16811bd92eb366f642a4b5eadf3) )
ROM_END

ROM_START( pc_cntra )   /* Contra */
	BIOS_CPU
	ROM_LOAD( "u4ct",    0x0c000, 0x2000, CRC(431486cf) SHA1(8b8a2bcddb1dfa027c249b62659dcc7bb8ec2778) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "u1ct",    0x10000, 0x10000, CRC(9fcc91d4) SHA1(ad1742a0da87cf7f26f81a99f185f0c28b9e7e6e) ) /* banked */
	ROM_LOAD( "u2ct",    0x20000, 0x10000, CRC(612ad51d) SHA1(4428e136b55778299bb269520b459c7112c0d6b2) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(8ab3977a) SHA1(61d3a7981fbe8a76ab7eee032059d42b50892e97) )
ROM_END

ROM_START( pc_pwrst )   /* Pro Wrestling */
	BIOS_CPU
	ROM_LOAD( "pw-u4",   0x0c000, 0x2000, CRC(0f03d71b) SHA1(82b94c2e4568d6de4d8cff49f3e416005a2e22ec) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "pw-u1",   0x10000, 0x08000, CRC(6242c2ce) SHA1(ea7d1cf9dece021c9a40772af7c6dcaf58b10585) ) /* banked */
	ROM_RELOAD(          0x18000, 0x08000 )
	ROM_LOAD( "pw-u2",   0x20000, 0x10000, CRC(ef6aa17c) SHA1(52171699eaee0b811952c5706584cff4e7cfb39a) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(4c6b7983) SHA1(377bd6267ae1d3ab13389a8adf894e116b3c9daa) )
ROM_END

ROM_START( pc_cvnia )   /* Castlevania */
	BIOS_CPU
	ROM_LOAD( "u4cv",    0x0c000, 0x2000, CRC(a2d4245d) SHA1(3703171d526e6de99e475afe0d942d69b89950a9) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "u1cv",    0x10000, 0x10000, CRC(add4fc52) SHA1(bbb4638a8e7660911896393d61580610a6535c62) ) /* banked */
	ROM_LOAD( "u2cv",    0x20000, 0x10000, CRC(7885e567) SHA1(de1e5a5b4bbd0116c91564edc3d552239074e8ae) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(7da2f045) SHA1(e6048a1f94103c2896eeb33dd7f6bc639831dd7d) )
ROM_END

ROM_START( pc_dbldr )   /* Double Dribble */
	BIOS_CPU
	ROM_LOAD( "dw-u4",    0x0c000, 0x2000, CRC(5006eef8) SHA1(6051d4750d95cdc0a71ecec40b5be4477921ca54) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "dw-u1",    0x10000, 0x10000, CRC(78e08e61) SHA1(a278e012ac89b8ae56d4a186c99f5ea2591f87b5) ) /* banked */
	ROM_LOAD( "dw-u2",    0x20000, 0x10000, CRC(ab554cde) SHA1(86f5788f856dd9336eaaadf8d5295435b0421486) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(9b5f4bd2) SHA1(998d2766763eb66f4052f9f16fbfb93d5b41a582) )
ROM_END

ROM_START( pc_rygar )   /* Rygar */
	BIOS_CPU
	ROM_LOAD( "ry-u4",    0x0c000, 0x2000, CRC(7149071b) SHA1(fbc7157eb16eedfc8808ab6224406037e41c44ef) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "ry-u1",    0x10000, 0x10000, CRC(aa2e54bc) SHA1(b44cd385d4019a535a4924a093ee9b097b850db4) ) /* banked */
	ROM_LOAD( "ry-u2",    0x20000, 0x10000, CRC(80cb158b) SHA1(012f378e0b5a5bbd32ad837cdfa096df6843d274) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(b69309ab) SHA1(a11ae46ed4c6ae5c22bab36593a53535a257fd4f) )
ROM_END

ROM_START( pc_trjan )   /* Trojan */
	BIOS_CPU
	ROM_LOAD( "tj-u4",    0x0c000, 0x2000, CRC(10835e1d) SHA1(ae0f3ec8d52707088af79d00bca0871af105da36) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "tj-u1",    0x10000, 0x10000, CRC(335c0e62) SHA1(62164235dc8e2a4419cb38f4cacf7ba2f3eb536b) ) /* banked */
	ROM_LOAD( "tj-u2",    0x20000, 0x10000, CRC(c0ddc79e) SHA1(5c23bb54eda6a55357e97d7322db453170e27598) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(14df772f) SHA1(bb1c58d53ee8c059b3a06d43ee4faf849d4f005f) )
ROM_END

/* C-Board Games */
ROM_START( pc_goons )   /* The Goonies */
	BIOS_CPU
	ROM_LOAD( "gn-u3",   0x0c000, 0x2000, CRC(33adedd2) SHA1(c85151819e2550e60cbe8f7d247a8da88cb805a4) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x10000, "cart", 0 )
	ROM_LOAD( "gn-u1",   0x08000, 0x8000, CRC(efeb0c34) SHA1(8e0374858dce0a10ffcfc5109f8287ebdea388e8) )

	ROM_REGION( 0x04000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "gn-u2",   0x00000, 0x4000, CRC(0f9c7f49) SHA1(f2fcf55d22a38a01df45393c90c73ff14b3b647c) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(cdd62d08) SHA1(e2752127ac0b1217f0216854b68a5e5957a565b3) )
ROM_END

/* D-Board Games */
ROM_START( pc_radrc )   /* Rad Racer */
	BIOS_CPU
	ROM_LOAD( "rc-u5",   0x0c000, 0x2000, CRC(ae60fd08) SHA1(fa7c201499cd702d8eef545bb05b0df833d2b406) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "rc-u1",   0x10000, 0x10000, CRC(dce369a7) SHA1(d7f293956d605af7cb6b81dbb80eaa4ad482ac0e) )
	ROM_LOAD( "rc-u2",   0x20000, 0x10000, CRC(389a79b5) SHA1(58de166d757e58c515272efc9d0bc03d1eb1086d) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(17c880f9) SHA1(41be451fcc46a746d5d31dba09f524c0af0cd214) )
ROM_END

ROM_START( pc_mtoid )   /* Metroid */
	BIOS_CPU
	ROM_LOAD( "mt-u5",   0x0c000, 0x2000, CRC(3dc25049) SHA1(bf0f72db9e6904f065801e490014405a734eb04e) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "mt-u1",   0x10000, 0x10000, CRC(4006ff10) SHA1(9563a6b4ff91c78ab9cbf97ea47a3f62524844d2) )
	ROM_LOAD( "mt-u2",   0x20000, 0x10000, CRC(ace6bbd8) SHA1(ac9c22bcc33aeee18b4f42a5a628bc5e147b4c29) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(efab54c9) SHA1(1d0122b7c90a364d54bf6eaa37ce439d706a4357) )
ROM_END

/* E-Board Games */
ROM_START( pc_miket )   /* Mike Tyson's Punchout */
	BIOS_CPU
	ROM_LOAD( "u5pt",    0x0c000, 0x2000, CRC(b434e567) SHA1(8e23c580b5556aacbeeb36fe36e778137c780903) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "u1pt",    0x10000, 0x20000, CRC(dfd9a2ee) SHA1(484a6793949b8cbbc65e3bcc9188bc63bb17b575) ) /* banked */

	ROM_REGION( 0x20000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "u3pt",    0x00000, 0x20000, CRC(570b48ea) SHA1(33de517b16b61625909d2eb5307c08b337b542c4) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(60f7ea1d) SHA1(fcc04cbd8ed233bb1358fc55800f9bb6c75b195b) )
ROM_END

/* F-Board Games */
ROM_START( pc_ngaid )   /* Ninja Gaiden */
	BIOS_CPU
	ROM_LOAD( "u2ng",    0x0c000, 0x2000, CRC(7505de96) SHA1(a9cbe6d4d2d33aeecb3e041315fbb266c886ebf1) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "u4ng",    0x10000, 0x20000, CRC(5f1e7b19) SHA1(ead83487d9be2f1d16c1d0b438a361a06508cd85) )   /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "u1ng",   0x00000, 0x20000, CRC(eccd2dcb) SHA1(2a319086f7c22b8fe7ca8ab72436a7c8d07b915e) )    /* banked */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(ec5641d6) SHA1(05f546aec5a9db167688a9abbac922f5ced7f7c5) )
ROM_END

ROM_START( pc_ddrgn )   /* Double Dragon */
	BIOS_CPU
	ROM_LOAD( "wd-u2",   0x0c000, 0x2000, CRC(dfca1578) SHA1(6bc00bb2913edeaecd885fee449b8a9955c509bf) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "wd-u4",  0x10000, 0x20000, CRC(05c97f64) SHA1(36913e92943c6bb40521ab13c843691a8db4cbc9) )    /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "wd-u1",  0x00000, 0x20000, CRC(5ebe0fd0) SHA1(4a948c9784433e051f1015a6b6e985a98b81b80d) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(f9739d62) SHA1(c9574ed8f24ffb7ab5a6bb1b79805fb6dc6e991a) )
ROM_END

ROM_START( pc_drmro )   /* Dr Mario */
	BIOS_CPU
	ROM_LOAD( "vu-u2",   0x0c000, 0x2000, CRC(4b7869ac) SHA1(37afb84d963233ad92cc424fcf992aa76ea0599f) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "vu-u4",  0x10000, 0x08000, CRC(cb02a930) SHA1(6622564abc5ce28f523b0da95054d1ea825f7bd5) )    /* banked */
	ROM_RELOAD(         0x18000, 0x08000 )
	ROM_RELOAD(         0x20000, 0x08000 )
	ROM_RELOAD(         0x28000, 0x08000 )

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "vu-u1",  0x00000, 0x08000, CRC(064d4ab3) SHA1(bcdc34435bf631422ea2701f00744a3606c6dce8) )
	ROM_RELOAD(         0x08000, 0x08000 )
	ROM_RELOAD(         0x10000, 0x08000 )
	ROM_RELOAD(         0x18000, 0x08000 )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(1b26e58c) SHA1(bd2d81d3cc54966ef154b3487d43ecbc316d6d22) )
ROM_END

ROM_START( pc_virus )   /* Virus (from location test board) */
	BIOS_CPU
	ROM_LOAD( "u2",   0x0c000, 0x2000, CRC(d2764d91) SHA1(393b54148e9250f14d83318aed6686cc04b923e6) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "virus 3-12-90.u4",  0x10000, 0x08000, CRC(a5239a77) SHA1(f1e79906bcbee4e0c62036d6ba95385b95daa53f) )    /* banked */
	ROM_RELOAD(         0x18000, 0x08000 )
	ROM_RELOAD(         0x20000, 0x08000 )
	ROM_RELOAD(         0x28000, 0x08000 )

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "virus 3-12-90.u1",  0x00000, 0x08000, CRC(d233c2ae) SHA1(0de301894edfc50b26b6e4cf3697a15065035c5e) )
	ROM_RELOAD(         0x08000, 0x08000 )
	ROM_RELOAD(         0x10000, 0x08000 )
	ROM_RELOAD(         0x18000, 0x08000 )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.u6", 0x00, 0x10, CRC(5b4f6930) SHA1(bd152d6907fe55f80125b34360fdb44cfc348906) )
ROM_END

ROM_START( pc_bload )   /* Bases Loaded (from location test board) */
	BIOS_CPU
	ROM_LOAD( "new game 1.u2",   0x0c000, 0x2000, CRC(43879cc5) SHA1(dfde35e255825fffc22b5495c1e3bc1cfad7e9c0) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x50000, "cart", 0 )
	ROM_LOAD( "u3",  0x10000, 0x20000, CRC(14a77a61) SHA1(6283f0dc8e9a2bbcd7ed452aa30cf646a6526837) )    /* banked */
	ROM_LOAD( "bases loaded 9a70 prg-h.u4",  0x30000, 0x20000, CRC(f158f941) SHA1(e58bdcfb62d25348f5c81b2cf8001fc2c9e04eb2) )    /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "u1",  0x00000, 0x20000, CRC(02ff6ae9) SHA1(ba15b91f917c9e722d1d8b24b5783bd5eac6a4e7) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.u6", 0x00, 0x10, CRC(5b4f6930) SHA1(bd152d6907fe55f80125b34360fdb44cfc348906) )
ROM_END

ROM_START( pc_ftqst )   /* Fester's Quest */
	BIOS_CPU
	ROM_LOAD( "eq-u2",   0x0c000, 0x2000, CRC(85326040) SHA1(866bd15e77d911147b191c13d062cef7ae4dcf62) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "eq-u4",  0x10000, 0x20000, CRC(953a3eaf) SHA1(a22c0a64d63036b6b8d147994a3055e1040a5282) )    /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "eq-u1",  0x00000, 0x20000, CRC(0ca17ab5) SHA1(a8765d6245f64b2d94c454662a24f8d8e277aa5a) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(1c601cd7) SHA1(bc13067475aac4a7b8bf5f0df96bdd5ba33f1cd7) )
ROM_END

ROM_START( pc_rcpam )   /* RC Pro Am */
	BIOS_CPU
	ROM_LOAD( "pm-u2",   0x0c000, 0x2000, CRC(358c2de7) SHA1(0f37d7e8303a7b87ad0584c6e0a79f3029c529f8) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "pm-u4",  0x10000, 0x08000, CRC(82cfde25) SHA1(4eb9abe896e597f8ecabb4f044d8c4b545a51b11) )    /* banked */
	ROM_RELOAD(         0x18000, 0x08000 )
	ROM_RELOAD(         0x20000, 0x08000 )
	ROM_RELOAD(         0x28000, 0x08000 )

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "pm-u1",  0x00000, 0x08000, CRC(83c90d47) SHA1(26917e1e016d2be0fa48d766d332779aae12b053) )
	ROM_RELOAD(         0x08000, 0x08000 )
	ROM_RELOAD(         0x10000, 0x08000 )
	ROM_RELOAD(         0x18000, 0x08000 )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(d71d8085) SHA1(67e30ff0c31c6600890408c4dc4d0d2f19856363) )
ROM_END

ROM_START( pc_rrngr )   /* Rescue Rangers */
	BIOS_CPU
	ROM_LOAD( "ru-u2",   0x0c000, 0x2000, CRC(2a4bfc4b) SHA1(87f58659d43a236af22682df4bd01593b69c9975) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "ru-u4",  0x10000, 0x20000, CRC(02931525) SHA1(28ddca5d299e7894e3c3aa0a193684ca3e384ee9) )    /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "ru-u1",  0x00000, 0x20000, CRC(218d4224) SHA1(37a729021173bec08a8497ad03fd58379b0fce39) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(1c2e1865) SHA1(ab2aa76d74c9e76c7ee3f9a211b1aefe5708a23f) )
ROM_END

ROM_START( pc_ynoid )   /* Yo! Noid */
	BIOS_CPU
	ROM_LOAD( "yc-u2",   0x0c000, 0x2000, CRC(0449805c) SHA1(3f96687eae047d1f8095fbb55c0659c9b0e10166) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "yc-u4",  0x10000, 0x20000, CRC(4affeee7) SHA1(54da2aa7ca56d9b593c8bcabf0bb1d701439013d) )    /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "yc-u1",  0x00000, 0x20000, CRC(868f7343) SHA1(acb6f6eb9e8beb0636c59a999c8f5920ef7786a3) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(8c376465) SHA1(39b06fd2ecd5f06b90b2fe06406c9155f5601bd8) )
ROM_END

ROM_START( pc_tmnt )    /* Teenage Mutant Ninja Turtles */
	BIOS_CPU
	ROM_LOAD( "u2u2",   0x0c000, 0x2000, CRC(bdce58c0) SHA1(abaf89c0ac55cce816a7c6542a868ab47e02d550) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "u4u2",   0x10000, 0x20000, CRC(0ccd28d5) SHA1(05606cafba838eeb36198b5e5e9d11c3729971b3) )    /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "u1u2",   0x00000, 0x20000, CRC(91f01f53) SHA1(171ed0792f3ca3f195145000d96b91aa57898773) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(f5a38e98) SHA1(26ef27294fc14d84920132023fbcf406d89ce2ee) )
ROM_END

ROM_START( pc_bstar )   /* Baseball Stars */
	BIOS_CPU
	ROM_LOAD( "b9-u2",   0x0c000, 0x2000, CRC(69f3fd7c) SHA1(1cfaa40f18b1455bb41ec0e57d6a227ed3e582eb) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "b9-u4",   0x10000, 0x20000, CRC(d007231a) SHA1(60690eaeacb79dbcab7dfe1c1e40da1aac235793) )   /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "b9-u1",   0x00000, 0x20000, CRC(ce149864) SHA1(00c88525756a360f42b27f0e2afaa0a19c2645a6) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(3e871350) SHA1(b338f9ef4e18d14843c6a1e8ecb974bca1df73d4) )
ROM_END

ROM_START( pc_tbowl )   /* Tecmo Bowl */
	BIOS_CPU
	ROM_LOAD( "tw-u2",   0x0c000, 0x2000, CRC(162aa313) SHA1(d0849ce87969c077fc14790ce5658e9857035413) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x30000, "cart", 0 )
	ROM_LOAD( "tw-u4",   0x10000, 0x20000, CRC(4f0c69be) SHA1(c0b09dc81070b935b3c621b07deb62dfa521a396) )   /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "tw-u1",   0x00000, 0x20000, CRC(44b078ef) SHA1(ae0c24f4ddd822b19c60e31257279b33b5f3fcad) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(18b2d1d3) SHA1(f4d023531b3d69cad4c9c511878e5aa6afb0ac59) )
ROM_END

/* G-Board Games */
ROM_START( pc_smb3 )    /* Super Mario Bros 3 */
	BIOS_CPU
	ROM_LOAD( "u3um",    0x0c000, 0x2000, CRC(45e92f7f) SHA1(9071d5f18639ac58d6d4d72674856f9ecab911f0) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x50000, "cart", 0 )
	ROM_LOAD( "u4um",    0x10000, 0x20000, CRC(590b4d7c) SHA1(ac45940b71215a3a48983e22e1c7e71a71642b91) )   /* banked */
	ROM_LOAD( "u5um",    0x30000, 0x20000, CRC(bce25425) SHA1(69468643a3a8b9220d675e2cdc4245ada81a492c) )   /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "u1um",    0x00000, 0x20000, CRC(c2928c49) SHA1(2697d1f21b72a6d8e7d2a2d2c51c9c5550f68b56) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(e48f4945) SHA1(66fe537cfe540317d6194847321ce4a9bdf0bba4) )
ROM_END

ROM_START( pc_gntlt )   /* Gauntlet */
	BIOS_CPU
	ROM_LOAD( "u3gl",    0x0c000, 0x2000, CRC(57575b92) SHA1(7ac633f253496f353d388bef30e6ec74a3d18814) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x50000, "cart", 0 )
	ROM_LOAD( "gl-0.prg",0x10000, 0x20000, CRC(b19c48a5) SHA1(4ba8674cec6fa8b0b4d96a7b00d4883a9e58a0a9) )   /* banked */
	ROM_RELOAD(          0x30000, 0x20000 )

	ROM_REGION( 0x010000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "gl-0.chr", 0x00000, 0x10000, CRC(22af8849) SHA1(01054943c1d069f5f535e93f969a5b6bfb958e0b) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(ba7f2e13) SHA1(8b9ee3b18bcb4b258a46d1c900b18a9cb2594046) )
ROM_END

ROM_START( pc_pwbld )   /* Power Blade */
	BIOS_CPU
	ROM_LOAD( "7t-u3",    0x0c000, 0x2000, CRC(edcc21c6) SHA1(5d73c6a747cfe951dc7c6ddfbb29859e9548aded) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x50000, "cart", 0 )
	ROM_LOAD( "7t-u5",   0x10000, 0x20000, CRC(faa957b1) SHA1(612c4823ed588652a78017096a6d76dd8064807a) )   /* banked */
	ROM_RELOAD(          0x30000, 0x20000 )

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "7t-u1",    0x00000, 0x20000, CRC(344be4a6) SHA1(2894292544f4315df44cda1bdc96047453da03e8) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(31a05a48) SHA1(8b340600feae03bb5cdab852a9879ecffcc8a2b9) )
ROM_END

ROM_START( pc_ngai3 )   /* Ninja Gaiden 3 */
	BIOS_CPU
	ROM_LOAD( "u33n",    0x0c000, 0x2000, CRC(c7ba0f59) SHA1(a4822035a10a2b5de3517b461dd357b2fa5da917) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x50000, "cart", 0 )
	ROM_LOAD( "u53n",    0x10000, 0x20000, CRC(f0c77dcb) SHA1(bda1184e27f3c3e92e58519508dd281b06c70d9b) )   /* banked */
	ROM_RELOAD(          0x30000, 0x20000 )

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "u13n",    0x00000, 0x20000, CRC(584bcf5d) SHA1(f4582e2a382c8424f839e848e95e88a7f46307dc) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(13755943) SHA1(b7d809b0f60ef489777ccb35868f5c1e777356e0) )
ROM_END

ROM_START( pc_radr2 )   /* Rad Racer II */
	BIOS_CPU
	ROM_LOAD( "qr-u3",    0x0c000, 0x2000, CRC(0c8fea63) SHA1(7ac04b151df732bd16708655352b7f13926f004f) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x50000, "cart", 0 )
	ROM_LOAD( "qr-u5",    0x10000, 0x10000, CRC(ab90e397) SHA1(0956f7d9a216549dbd80b1dbf2653b36a320d0ab) )  /* banked */
	ROM_RELOAD(           0x20000, 0x10000 )
	ROM_RELOAD(           0x30000, 0x10000 )
	ROM_RELOAD(           0x40000, 0x10000 )

	ROM_REGION( 0x010000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "qr-u1",    0x00000, 0x10000, CRC(07df55d8) SHA1(dd0fa0a79d30eb04917d7309a62adfb037ef9ca5) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(40c4f294) SHA1(3fcac63fe6f147b662d59d25f905f797a1f5d0db) )
ROM_END

ROM_START( pc_rkats )   /* Rockin' Kats */
	BIOS_CPU
	ROM_LOAD( "7a-u3",    0x0c000, 0x2000, CRC(352b1e3c) SHA1(bb72b586ec4b482aef462b017de5662d83631df1) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x50000, "cart", 0 )
	ROM_LOAD( "7a-u5",    0x10000, 0x20000, CRC(319ccfcc) SHA1(06e1c34af917b84a990db895c7b44df1b3393c96) )  /* banked */
	ROM_RELOAD(           0x30000, 0x20000 )

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "7a-u1",    0x00000, 0x20000, CRC(487aa440) SHA1(ee7ebbcf89c81ba59beda1bd27289dae21bb8071) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(56ab5bf9) SHA1(9546f6e20fdb13146c5db5353a1cb2a95931d909) )
ROM_END

ROM_START( pc_suprc )   /* Super C */
	BIOS_CPU
	ROM_LOAD( "ue-u3",    0x0c000, 0x2000, CRC(a30ca248) SHA1(19feb1b4f749768773e0d24777d7e60b2b6260e2) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x50000, "cart", 0 )
	ROM_LOAD( "ue-u5",    0x10000, 0x20000, CRC(c7fbecc3) SHA1(2653456c91031dfa73a50cab3835068a7bface8d) )  /* banked */
	ROM_RELOAD(           0x30000, 0x20000 )

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "ue-u1",    0x00000, 0x20000, CRC(153295c1) SHA1(4ff1caaedca52fb9bb0ca6c8fac24edda77308d7) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(d477095e) SHA1(a179dffe529889f8e17e9f04958fea28611df0d3) )
ROM_END

ROM_START( pc_tmnt2 )   /* Teenage Mutant Ninja Turtles II */
	BIOS_CPU
	ROM_LOAD( "2n-u3",    0x0c000, 0x2000, CRC(65298370) SHA1(fd120f43e465a2622f2e2679ace2fb0fe7e709b1) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x50000, "cart", 0 )
	ROM_LOAD( "2n-u5",    0x10000, 0x40000, CRC(717e1c46) SHA1(b49cc88e026dac7f5ba96f5c16bcb897addbe259) )  /* banked */

	ROM_REGION( 0x040000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "2n-u1",    0x00000, 0x40000, CRC(0dbc575f) SHA1(8094278cf3267757953ab761dbccf38589142376) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(237e8519) SHA1(81b368d0784e4172c5cf9f4f4b92e29e05d34ae7) )
ROM_END

ROM_START( pc_wcup )    /* Nintendo World Cup */
	BIOS_CPU
	ROM_LOAD( "xz-u3",    0x0c000, 0x2000, CRC(c26cb22f) SHA1(18fea97b498812915bbd53a20b4f0a2130de6faf) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x50000, "cart", 0 )
	ROM_LOAD( "xz-u5",    0x10000, 0x20000, CRC(314ee295) SHA1(0a5963feb5a6b47f0e7bea5bdd3d5835300af7b6) )  /* banked */
	ROM_RELOAD(           0x30000, 0x20000 )

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "xz-u1",    0x00000, 0x20000, CRC(92477d53) SHA1(33225bd5ee72f92761fdce931c93dd54e6885bd4) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(e17e1d76) SHA1(3e4e1ddcc8524bf451cb568b1357ec1f0a8be44c) )
ROM_END

ROM_START( pc_mman3 )   /* Mega Man 3 */
	BIOS_CPU
	ROM_LOAD( "xu-u3",   0x0c000, 0x2000, CRC(c3984e09) SHA1(70d7e5d9cf9b1f358e1be84a0e8c5997b1aae2d9) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x50000, "cart", 0 )
	ROM_LOAD( "xu-u4",   0x10000, 0x20000, CRC(98a3263c) SHA1(02c8d8301fb220c3f4fd82bdc8cd2388b975fd05) )   /* banked */
	ROM_LOAD( "xu-u5",   0x30000, 0x20000, CRC(d365647a) SHA1(4f39de6249c5f8b7cfa34bc955fd7ea6251569b5) )   /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "xu-u1",    0x00000, 0x20000, CRC(4028916e) SHA1(f986f72ba5284129620d31c0779ac6d50638e6f1) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(0fe6e900) SHA1(544d8af1aa9186bf76d0a35e78b20e94d3afbcb5) )
ROM_END

ROM_START( pc_smb2 )    /* Super Mario Bros 2 */
	BIOS_CPU
	ROM_LOAD( "mw-u3",   0x0c000, 0x2000, CRC(beaeb43a) SHA1(c7dd186d6167e39924a000eb80bd33beedb2b8c8) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x50000, "cart", 0 )
	ROM_LOAD( "mw-u5",   0x10000, 0x20000, CRC(07854b3f) SHA1(9bea58ba97730c84232a4acbb23c3ea7bce14ec5) )   /* banked */
	ROM_RELOAD(          0x30000, 0x20000 )

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "mw-u1",    0x00000, 0x20000, CRC(f2ba1170) SHA1(d9976b677ad222b76fbdaf31713374e2f283d44e) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(372f4e84) SHA1(cdf221d49f3b454997d696f213d60b5dce0ce9fb) )
ROM_END

ROM_START( pc_ngai2 )   /* Ninja Gaiden 2 */
	BIOS_CPU
	ROM_LOAD( "nw-u3",   0x0c000, 0x2000, CRC(bc178cde) SHA1(2613f501f92d358f0085aa7002c752cb9a8521ca) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x50000, "cart", 0 )
	ROM_LOAD( "nw-u5",   0x10000, 0x20000, CRC(c43da8e2) SHA1(702a4cf2f57fff7183f2d3c18b8997a38cadc6cd) )   /* banked */
	ROM_RELOAD(          0x30000, 0x20000 )

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "nw-u1",    0x00000, 0x20000, CRC(8e0c8bb0) SHA1(6afe24b8e57f5a2174000a706b66209d7e310ed6) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(682dffd1) SHA1(87ea54b3d725a552b397ccb2af0ccf8bd6452a7c) )
ROM_END

/* H-Board Games */
ROM_START( pc_pinbt )   /* PinBot */
	BIOS_CPU
	ROM_LOAD( "io-u3",   0x0c000, 0x2000, CRC(15ba8a2e) SHA1(e64180b2f12189e3ac1e155f3544f28af8003f97) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x50000, "cart", 0 )
	ROM_LOAD( "io-u5",   0x10000, 0x20000, CRC(9f75b83b) SHA1(703e41d4c1a4716b324dece6df2ce12a847f082c) )   /* banked */
	ROM_RELOAD(           0x30000, 0x20000 )    /* banked */

	ROM_REGION( 0x010000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "io-u1",    0x00000, 0x10000, CRC(9089fc24) SHA1(0bc92a0853c5ebc47c3adbc4e919ea41a55297d0) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(ac75f323) SHA1(4bffff024132d6f71d6aa55e24af400d2915aca4) )
ROM_END

/* i-Board Games */
ROM_START( pc_cshwk )   /* Captain Sky Hawk */
	BIOS_CPU
	ROM_LOAD( "yw-u3",   0x0c000, 0x2000, CRC(9d988209) SHA1(b355911d31dfc611b9e90cca82fc10035483b89c) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x50000, "cart", 0 )
	ROM_LOAD( "yw-u1",   0x10000, 0x20000, CRC(a5e0208a) SHA1(e12086a3f1a3b5e9ec035cb778505e43f501416a) ) /* banked */
	ROM_RELOAD(          0x30000, 0x20000 )

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(eb1c794f) SHA1(d32f841fd4306389d716229da9bffea909186689) )
ROM_END

ROM_START( pc_sjetm )   /* Solar Jetman */
	BIOS_CPU
	ROM_LOAD( "lj-u3",   0x0c000, 0x2000, CRC(273d8e75) SHA1(b13b97545b39f6b0459440fb6594ebe03366dfc9) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x50000, "cart", 0 )
	ROM_LOAD( "lj-u1",   0x10000, 0x40000, CRC(8111ba08) SHA1(caa4d1ab710bd766f8505ef24f5702dac6e988af) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(f3ae712a) SHA1(51f443c65e64f1a9eb565ce017b50ec9bd4a5520) )
ROM_END


/* K-Board Games */
ROM_START( pc_moglf )   /* Mario Open Golf */
	BIOS_CPU
	ROM_LOAD( "ug-u2",   0x0c000, 0x2000, CRC(e932fe2b) SHA1(563380482525fdadd05fced2af61d5198d1654a5) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x50000, "cart", 0 )
	ROM_LOAD( "ug-u4",   0x10000, 0x40000, CRC(091a6a4c) SHA1(2d5ac7c65ce63d409b6e0b2e2185d81bc7c57c69) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(633766d5) SHA1(3a2564f3a2daf3a789e4c4056822f12243c89619) )
ROM_END

/***************************************************************************

  BIOS driver(s)

***************************************************************************/

ROM_START( playch10 )
	BIOS_CPU
	BIOS_GFX
	ROM_REGION( 0x50000, "cart", ROMREGION_ERASE00 )
ROM_END

/******************************************************************************/


/* A dummy driver, so that the bios can be debugged, and to serve as */
/* parent for the other drivers, so that we do not have to include */
/* them in every zip file */
GAME( 1986, playch10, 0, playch10, playch10, playch10_state, playch10, ROT0, "Nintendo of America", "PlayChoice-10 BIOS", MACHINE_IS_BIOS_ROOT )

/******************************************************************************/


DRIVER_INIT_MEMBER(playch10_state,virus)
{
	UINT8 *ROM = memregion("rp5h01")->base();
	UINT32 len = memregion("rp5h01")->bytes();
	for (int i = 0; i < len; i++)
	{
		ROM[i] = BITSWAP8(ROM[i],0,1,2,3,4,5,6,7);
		ROM[i] ^= 0xff;
	}

	/* common init */
	DRIVER_INIT_CALL(pcfboard);
}

/*     YEAR  NAME      PARENT    BIOS      MACHINE   INPUT     INIT      MONITOR  */

/* Standard Games */
GAME( 1983, pc_tenis, playch10, playch10, playch10, playch10_state, playch10, ROT0, "Nintendo",                                 "Tennis (PlayChoice-10)", 0 )
GAME( 1983, pc_mario, playch10, playch10, playch10, playch10_state, playch10, ROT0, "Nintendo",                                 "Mario Bros. (PlayChoice-10)", 0 )
GAME( 1984, pc_bball, playch10, playch10, playch10, playch10_state, playch10, ROT0, "Nintendo of America",                      "Baseball (PlayChoice-10)", 0 )
GAME( 1984, pc_bfght, playch10, playch10, playch10, playch10_state, playch10, ROT0, "Nintendo",                                 "Balloon Fight (PlayChoice-10)", 0 )
GAME( 1984, pc_ebike, playch10, playch10, playch10, playch10_state, playch10, ROT0, "Nintendo",                                 "Excite Bike (PlayChoice-10)", 0 )
GAME( 1984, pc_golf,  playch10, playch10, playch10, playch10_state, playch10, ROT0, "Nintendo",                                 "Golf (PlayChoice-10)", 0 )
GAME( 1985, pc_kngfu, playch10, playch10, playch10, playch10_state, playch10, ROT0, "Irem (Nintendo license)",                  "Kung Fu (PlayChoice-10)", 0 )
GAME( 1985, pc_smb,   playch10, playch10, playch10, playch10_state, playch10, ROT0, "Nintendo",                                 "Super Mario Bros. (PlayChoice-10)", 0 )
GAME( 1986, pc_vball, playch10, playch10, playch10, playch10_state, playch10, ROT0, "Nintendo",                                 "Volley Ball (PlayChoice-10)", 0 )
GAME( 1987, pc_1942,  playch10, playch10, playch10, playch10_state, pc_hrz,   ROT0, "Capcom",                                   "1942 (PlayChoice-10)", MACHINE_IMPERFECT_GRAPHICS )

/* Gun Games */
GAME( 1984, pc_duckh, playch10, playch10, playc10g, playch10_state, pc_gun,   ROT0, "Nintendo",                                 "Duck Hunt (PlayChoice-10)", 0 )
GAME( 1984, pc_hgaly, playch10, playch10, playc10g, playch10_state, pc_gun,   ROT0, "Nintendo",                                 "Hogan's Alley (PlayChoice-10)", 0 )
GAME( 1984, pc_wgnmn, playch10, playch10, playc10g, playch10_state, pc_gun,   ROT0, "Nintendo",                                 "Wild Gunman (PlayChoice-10)", 0 )

/* A-Board Games */
GAME( 1986, pc_grdus, playch10, playch10, playch10, playch10_state, pcaboard, ROT0, "Konami",                                   "Gradius (PlayChoice-10)" , 0) // date: 860917
GAME( 1986, pc_grdue, pc_grdus, playch10, playch10, playch10_state, pcaboard, ROT0, "Konami",                                   "Gradius (PlayChoice-10, older)" , 0) // date: 860219
GAME( 1987, pc_tkfld, playch10, playch10, playch10, playch10_state, pcaboard, ROT0, "Konami (Nintendo of America license)",     "Track & Field (PlayChoice-10)", 0 )

/* B-Board Games */
GAME( 1986, pc_pwrst, playch10, playch10, playch10, playch10_state, pcbboard, ROT0, "Nintendo",                                 "Pro Wrestling (PlayChoice-10)", 0 )
GAME( 1986, pc_trjan, playch10, playch10, playch10, playch10_state, pcbboard, ROT0, "Capcom USA (Nintendo of America license)", "Trojan (PlayChoice-10)", 0 )
GAME( 1987, pc_cvnia, playch10, playch10, playch10, playch10_state, pcbboard, ROT0, "Konami (Nintendo of America license)",     "Castlevania (PlayChoice-10)", 0 )
GAME( 1987, pc_dbldr, playch10, playch10, playch10, playch10_state, pcbboard, ROT0, "Konami (Nintendo of America license)",     "Double Dribble (PlayChoice-10)", 0 )
GAME( 1987, pc_rnatk, playch10, playch10, playch10, playch10_state, pcbboard, ROT0, "Konami (Nintendo of America license)",     "Rush'n Attack (PlayChoice-10)", 0 )
GAME( 1987, pc_rygar, playch10, playch10, playch10, playch10_state, pcbboard, ROT0, "Tecmo (Nintendo of America license)",      "Rygar (PlayChoice-10)", 0 )
GAME( 1988, pc_cntra, playch10, playch10, playch10, playch10_state, pcbboard, ROT0, "Konami (Nintendo of America license)",     "Contra (PlayChoice-10)", 0 )

/* C-Board Games */
GAME( 1986, pc_goons, playch10, playch10, playch10, playch10_state, pccboard, ROT0, "Konami",                                   "The Goonies (PlayChoice-10)", 0 )

/* D-Board Games */
GAME( 1986, pc_mtoid, playch10, playch10, playch10, playch10_state, pcdboard_2, ROT0, "Nintendo",                               "Metroid (PlayChoice-10)", 0 )
GAME( 1987, pc_radrc, playch10, playch10, playch10, playch10_state, pcdboard, ROT0, "Square",                                   "Rad Racer (PlayChoice-10)", 0 )

/* E-Board Games */
GAME( 1987, pc_miket, playch10, playchnv, playch10, playch10_state, pceboard, ROT0, "Nintendo",                                 "Mike Tyson's Punch-Out!! (PlayChoice-10)", MACHINE_IMPERFECT_GRAPHICS )

/* F-Board Games */
GAME( 1987, pc_rcpam, playch10, playch10, playch10, playch10_state, pcfboard, ROT0, "Rare",                                     "R.C. Pro-Am (PlayChoice-10)", 0 )
GAME( 1987, pc_rrngr, playch10, playch10, playch10, playch10_state, pcfboard, ROT0, "Capcom USA (Nintendo of America license)", "Chip'n Dale: Rescue Rangers (PlayChoice-10)", 0 )
GAME( 1988, pc_ddrgn, playch10, playch10, playch10, playch10_state, pcfboard, ROT0, "Technos Japan",                            "Double Dragon (PlayChoice-10)", 0 )
GAME( 1989, pc_ngaid, playch10, playch10, playch10, playch10_state, pcfboard, ROT0, "Tecmo (Nintendo of America license)",      "Ninja Gaiden (PlayChoice-10)", 0 )
GAME( 1989, pc_tmnt,  playch10, playch10, playch10, playch10_state, pcfboard, ROT0, "Konami (Nintendo of America license)",     "Teenage Mutant Ninja Turtles (PlayChoice-10)", 0 )
GAME( 1989, pc_ftqst, playch10, playch10, playch10, playch10_state, pcfboard, ROT0, "Sunsoft (Nintendo of America license)",    "Uncle Fester's Quest: The Addams Family (PlayChoice-10)", 0 )
GAME( 1989, pc_bstar, playch10, playch10, playch10, playch10_state, pcfboard_2, ROT0, "SNK (Nintendo of America license)",      "Baseball Stars: Be a Champ! (PlayChoice-10)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1989, pc_tbowl, playch10, playch10, playch10, playch10_state, pcfboard, ROT0, "Tecmo (Nintendo of America license)",      "Tecmo Bowl (PlayChoice-10)", 0 )
GAME( 1990, pc_virus, pc_drmro, playch10, playch10, playch10_state, virus,    ROT0, "Nintendo",                                 "Virus (Dr. Mario prototype, PlayChoice-10)", 0 )
GAME( 1990, pc_drmro, playch10, playch10, playch10, playch10_state, pcfboard, ROT0, "Nintendo",                                 "Dr. Mario (PlayChoice-10)", 0 )
GAME( 1990, pc_bload, playch10, playch10, playch10, playch10_state, virus,    ROT0, "Jaleco (Nintendo of America license)",     "Bases Loaded (Prototype, PlayChoice-10)", 0 )
GAME( 1990, pc_ynoid, playch10, playch10, playch10, playch10_state, pcfboard, ROT0, "Capcom USA (Nintendo of America license)", "Yo! Noid (PlayChoice-10)", 0 )

/* G-Board Games */
GAME( 1988, pc_smb2,  playch10, playch10, playch10, playch10_state, pcgboard, ROT0, "Nintendo",                                 "Super Mario Bros. 2 (PlayChoice-10)", 0 )
GAME( 1988, pc_smb3,  playch10, playch10, playch10, playch10_state, pcgboard, ROT0, "Nintendo",                                 "Super Mario Bros. 3 (PlayChoice-10)", 0 )
GAME( 1990, pc_mman3, playch10, playch10, playch10, playch10_state, pcgboard, ROT0, "Capcom USA (Nintendo of America license)", "Mega Man III (PlayChoice-10)", 0 )
GAME( 1990, pc_suprc, playch10, playch10, playch10, playch10_state, pcgboard, ROT0, "Konami (Nintendo of America license)",     "Super C (PlayChoice-10)", 0 )
GAME( 1990, pc_tmnt2, playch10, playch10, playch10, playch10_state, pcgboard, ROT0, "Konami (Nintendo of America license)",     "Teenage Mutant Ninja Turtles II: The Arcade Game (PlayChoice-10)", 0 )
GAME( 1990, pc_wcup,  playch10, playch10, playch10, playch10_state, pcgboard, ROT0, "Technos Japan (Nintendo license)",         "Nintendo World Cup (PlayChoice-10)", 0 )
GAME( 1990, pc_ngai2, playch10, playch10, playch10, playch10_state, pcgboard, ROT0, "Tecmo (Nintendo of America license)",      "Ninja Gaiden Episode II: The Dark Sword of Chaos (PlayChoice-10)", 0 )
GAME( 1991, pc_ngai3, playch10, playch10, playch10, playch10_state, pcgboard, ROT0, "Tecmo (Nintendo of America license)",      "Ninja Gaiden Episode III: The Ancient Ship of Doom (PlayChoice-10)", 0 )
GAME( 1991, pc_pwbld, playch10, playch10, playch10, playch10_state, pcgboard, ROT0, "Taito (Nintendo of America license)",      "Power Blade (PlayChoice-10)", 0 )
GAME( 1991, pc_rkats, playch10, playch10, playch10, playch10_state, pcgboard, ROT0, "Atlus (Nintendo of America license)",      "Rockin' Kats (PlayChoice-10)", 0 )
/* variant with 4 screen mirror */
GAME( 1990, pc_radr2, playch10, playch10, playch10, playch10_state, pcgboard_type2, ROT0, "Square (Nintendo of America license)", "Rad Racer II (PlayChoice-10)", 0 )
GAME( 1985, pc_gntlt, playch10, playch10, playch10, playch10_state, pcgboard_type2, ROT0, "Atari / Tengen (Nintendo of America license)", "Gauntlet (PlayChoice-10)", MACHINE_IMPERFECT_GRAPHICS )

/* H-Board Games */
GAME( 1988, pc_pinbt, playch10, playch10_hboard, playch10, playch10_state, pchboard, ROT0, "Rare (Nintendo of America license)", "PinBot (PlayChoice-10)", MACHINE_IMPERFECT_GRAPHICS )

/* i-Board Games */
GAME( 1989, pc_cshwk, playch10, playch10, playch10, playch10_state, pciboard, ROT0, "Rare (Nintendo of America license)",       "Captain Sky Hawk (PlayChoice-10)", 0 )
GAME( 1990, pc_sjetm, playch10, playch10, playch10, playch10_state, pciboard, ROT0, "Rare",                                     "Solar Jetman (PlayChoice-10)", 0 )

/* K-Board Games */
GAME( 1991, pc_moglf, playch10, playch10, playch10, playch10_state, pckboard, ROT0, "Nintendo",                                 "Mario's Open Golf (PlayChoice-10)", 0 )

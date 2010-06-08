/*
Real Battle Mahjong King by 'Game Men System Co. Ltd.'

PCB Layout
----------

No.6899-B
|--------------------------------------------------------|
|UPC1241H          YM3014  YM2151    14.31818MHz         |
|     VOL       358                  89C51        B1     |
|          M6295                                         |
|                  S1      PAL                           |
|                                             A1         |
|                                                        |
|J                                           6116        |
|A                 P1                        6116        |
|M   DSW3                                                |
|M   DSW2                                                |
|A   DSW1   DSW4                                         |
|                               |-------|    6116        |
|                               |LATTICE|    6116  PAL   |
|               62256    62256  |1032E  |                |
|                               |       |    T1          |
|                    68000      |-------|                |
| 3.6V_BATT     |-------------|                          |
|               |        93C46|                          |
|               |             |                          |
|               |  *          |              6116        |
|               |             |  22MHz       6116        |
|---------------|PLASTIC COVER|--------------------------|
Notes:
      68000 clock - 11.000MHz [22/2]
      VSync       - 58Hz
      Hsync       - none (dead board, no signal)
      M6295 clock - 1.100MHz [22/20], sample rate = 1100000 / 165, chip is printed 'AD-65'
      YM2151 clock- 2.750MHz [22/8], chip is printed 'K-666'. YM3014 chip is printed 'K-664'
                * - Unpopulated position for PIC16F84
        3.6V_BATT - Purpose of battery unknown, does not appear to be used for backup of suicide RAM,
                    and there's no RTC on the board.
            93C46 - 128 x8 EEPROM. This chip was covered by a plactic cover. There's nothing else under
                    the cover, but there was an unpopulated position for a PIC16F84
            89C51 - Atmel 89C51 Microcontroller (protected)

      ROMs -
            P1 - Hitachi HN27C4096  (Main PRG)
            T1 - Macronix MX27C4000 (GFX)
            A1 - Atmel AT27C080     (GFX)
            B1 - Macronix MX261000  (GFX?? or PRG/data for 89C51?)
            S1 - Macronix MX27C2000 (OKI samples)
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "machine/eeprom.h"

static UINT16 *gms_vidram;
static UINT16 *gms_vidram2;

static UINT16 tilebank=0;

static READ16_HANDLER( gms_read )
{
	return mame_rand(space->machine);
}


static WRITE16_HANDLER( gms_write1 )
{

}

static WRITE16_HANDLER( gms_write2 )
{
	tilebank=data;
}

static WRITE16_HANDLER( gms_write3 )
{

}

static WRITE16_DEVICE_HANDLER( eeprom_w )
{
	//bad ?
	if( ACCESSING_BITS_0_7 )
	{
		eeprom_write_bit(device, data & 0x04);
		eeprom_set_cs_line(device, (data & 0x01) ? CLEAR_LINE:ASSERT_LINE );

		eeprom_set_clock_line(device, (data & 0x02) ? ASSERT_LINE : CLEAR_LINE );
	}
}


static ADDRESS_MAP_START( rbmk_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM
	AM_RANGE(0x500000, 0x50ffff) AM_RAM
	AM_RANGE(0x940000, 0x940fff) AM_RAM AM_BASE(&gms_vidram2)
	AM_RANGE(0x980300, 0x983fff) AM_RAM // 0x2048  words ???, byte access
	AM_RANGE(0x900000, 0x900fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x9c0000, 0x9c0fff) AM_RAM AM_BASE(&gms_vidram)
	AM_RANGE(0xb00000, 0xb00001) AM_DEVWRITE("eeprom", eeprom_w)
	AM_RANGE(0xC00000, 0xC00001) AM_READ_PORT("IN0") AM_WRITE(gms_write1)
	AM_RANGE(0xC08000, 0xC08001) AM_READ_PORT("IN1") AM_WRITE(gms_write2)
	AM_RANGE(0xC10000, 0xC10001) AM_READ_PORT("IN3")
	AM_RANGE(0xC18080, 0xC18081) AM_READ(gms_read)
	AM_RANGE(0xC20000, 0xC20001) AM_READ_PORT("IN2")
	AM_RANGE(0xC28000, 0xC28001) AM_WRITE(gms_write3)
ADDRESS_MAP_END


static INPUT_PORTS_START( rbmk )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Controls ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Joystick ) )
	PORT_DIPSETTING(      0x0000, "Keyboard" )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Version ) )
	PORT_DIPSETTING(      0x4000, "8.8" )
	PORT_DIPSETTING(      0x0000, "8.8-" )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")	/* 16bit */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )

	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT  )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")	/* 16bit */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )


	PORT_START("IN3")	/* 16bit */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)

	PORT_START("IN4")	/* 16bit */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN5")	/* 16bit */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout rbmk32_layout =
{
	8,32,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4,0,12,8,20,16,28,24},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32,
	 16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32, 24*32, 25*32, 26*32, 27*32, 28*32, 29*32, 30*32, 31*32 },
	32*32
};

static const gfx_layout rbmk8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4,0,12,8,20,16,28,24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};


static GFXDECODE_START( rbmk )
	GFXDECODE_ENTRY( "gfx1", 0, rbmk32_layout,   0x0, 16  )
	GFXDECODE_ENTRY( "gfx2", 0, rbmk8_layout,   0x100, 16  )
GFXDECODE_END

static VIDEO_START(rbmk)
{
}

static VIDEO_UPDATE(rbmk)
{
	int x,y;
	int count = 0;

	for (y=0;y<8;y++)
	{
		for (x=0;x<64;x++)
		{
			int tile = gms_vidram2[count+0x600];
			drawgfx_opaque(bitmap,cliprect,screen->machine->gfx[0],(tile&0xfff)+((tilebank&0x10)>>4)*0x1000,tile>>12,0,0,x*8,y*32);
			count++;
		}
	}

	count=0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<64;x++)
		{
			int tile = gms_vidram[count];
			drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[1],(tile&0xfff)+((tilebank>>1)&3)*0x1000,tile>>12,0,0,x*8,y*8,0);
			count++;
		}
	}
	return 0;
}



static MACHINE_DRIVER_START( rbmk )
	MDRV_CPU_ADD("maincpu", M68000, 22000000 /2)
	MDRV_CPU_PROGRAM_MAP(rbmk_mem)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)

	MDRV_GFXDECODE(rbmk)


	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(58)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x800)

	MDRV_VIDEO_START(rbmk)
	MDRV_VIDEO_UPDATE(rbmk)

	MDRV_EEPROM_93C46_ADD("eeprom")


	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_OKIM6295_ADD("oki", 1122000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.47)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.47)
MACHINE_DRIVER_END

ROM_START( rbmk )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "p1.u64", 0x00000, 0x80000, CRC(83b3c505) SHA1(b943d7312dacdf46d4a55f9dc3cf92e291c40ce7) )

	ROM_REGION( 0x2, "cpu1", 0 ) /* protected MCU? */
	ROM_LOAD( "89c51.mcu", 0x0, 0x2, NO_DUMP ) // unless it has external rom only?

	ROM_REGION( 0x20000, "user1", 0 ) /* ??? mcu data / code */
	ROM_LOAD( "b1.u72", 0x00000, 0x20000,  CRC(1a4991ac) SHA1(523b58caa21b4a073c664c076d2d7bb07a4253cd) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s1.u83", 0x00000, 0x40000, CRC(44b20e47) SHA1(54691af73aa5d20f9a9afe145447ef1cf34c9a0c) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* 8x32 tiles, lots of girls etc. */
	ROM_LOAD( "a1.u41", 0x00000, 0x100000,  CRC(1924de6b) SHA1(1a72ee2fd0abca51893f0985a591573bfd429389) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* 8x8 tiles? cards etc */
	ROM_LOAD( "t1.u39", 0x00000, 0x80000, CRC(adf67429) SHA1(ab03c7f68403545f9e86a069581dc3fc3fa6b9c4) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) /* eeprom */
	ROM_LOAD16_WORD_SWAP( "93c46.u51", 0x00, 0x080, CRC(4ca6ff01) SHA1(66c456eac5b0d1176ef9130baf2e746efdf30152) )
ROM_END


GAME( 1998, rbmk, 0, rbmk, rbmk,0, ROT0,  "GMS", "Real Battle Mahjong King", GAME_NOT_WORKING )

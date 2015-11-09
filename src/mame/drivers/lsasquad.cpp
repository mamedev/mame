// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Land Sea Air Squad / Storming Party  (c) 1986 Taito

driver by Nicola Salmoria

TODO:
- Wrong sprite/tilemap priority. Sprites can appear above and below the middle
  layer, it's not clear how this is selected since there are no free attribute
  bits.
  The priority seems to involve split transparency on the tilemap and also
  priority on sprites (so that people pass below doors but airplanes above).
  It is confirmed that priority is controlled by PROM a64-06.9 (grounding A9
  makes sprites disappear).
- Scrollram not entirely understood - it's most likely wrong, but more than
  enough to run this particular game.
- The video driver is pretty slow and could be optimized using temporary bitmaps
  (or tilemaps), however I haven't done that because the video circuitry is not
  entirely understood and if other games are found running on this hardware, they
  might not like the optimizations.
- Unknown writes to YM2203 output ports (filters?)

TS 2006.05.28:
--------------
Added  'Daikaijuu no Gyakushuu'
- more sprite ram
- a bit different sound hardware (diff. communciation with sub cpu, no NMI)
- same video board as  LSA Squad, but different features are used


Difficulty level    Damage from..
                   Bullet   Enemy plane
A (easy)            1        2
B (medium)          2        3
C (hard)            2        5
D (hardest)         3        6

Rank A and B are described as normal. In rank C, some enemies shoot
more bullets. In rank D, all enemies shoot more bullets.

---------------------------------------------------------------------------

Daikaiju no Gyakushu
Taito, 1986

(Info from a Japanese friend...)
The Japanese letters on the title screen say "Daikaizyuu no Gyakusyuu"
Pronunciation is like "Dai-Kai-ju-no-Ghi-yaku-shu"
The correct spelling is "Daikaiju no Gyakushu"

PCB Layout
----------

J1100064A
K1100152A
CPU PCB
(sticker) M4300046A
|--------------------------------------------------------------|
|        MB3731  YM3014 PC010SA                A74_09.IC24     |
|H               4556   PC010SA                A74_08.IC23    |-|
|                 LM3900                       A74_07.IC22    | |
|              VOL                          PC040DA           | |
|-|                          YM2203         PC040DA           | |
  |                                         PC040DA           | |
|-|                          YM2149                 A74_06.IC9| |
|                                                             | |
|              Z80A                                           |-|
|                                                              |
|G             A74_04.IC44                                     |
|                                                              |
|              6116                                            |
|                                                    Z80B     |-|
|                        A74_05.IC35                          | |
|-|                                                 A74_01.IC4| |
  |     PC030CM                                               | |
|-|                                       PAL16L8   A74_02.IC3| |
|                                         (A64-18)            | |
|                                                   A74_03.IC2| |
|                                                             |-|
|              DSWB   DSWA            TL7700            6264   |
|--------------------------------------------------------------|
Notes:
      Z80A clock   - 3.000MHz [24/8]
      Z80B clock   - 6.000MHz [24/4]
      YM2203 clock - 3.000MHz [24/8]
      YM2149 clock - 3.000MHz [24/8]
      PC030CM      - Taito custom ceramic I/O module (SIL20)
      PC010SA      - Taito custom ceramic sound DAC module (SIL14)
      PC040DA      - Taito custom ceramic video DAC module (SIL19)
      TL7700       - Texas Instruments TL7700 Voltage Supervisor/Reset IC (DIP8)
      6264         - Mitsubishi M5M5165P-10 8k x8 SRAM (DIP28)
      6116         - Sony CXK5816P-12L 2k x8 SRAM (DIP24)
      MB3731       - Fujitsu MB3731 audio amp IC (SIL12)
      G            - 22-way edge connector
      H            - 12 pin Taito power connector (gnd,gnd,gnd,gnd,5,5,5,-5,12,key,12,12)
      A74_05.IC35  - Motorola 68705P5S Micro-controller, clock 3.000MHz [24/8] (protected, DIP28)
      VSync        - 60Hz

      ROMs
      ----
          A74_06/07/08/09 - MMI 63S441 1k x4 Bipolar PROM (DIP18)
          A74_01/02/03/04 - Fujitsu 27C256 EPROM (DIP28)



Bottom board
------------
J1100065A
K1100153A
VIDEO BOARD
|--------------------------------------------------------------|
|     A74_10.IC27  A74_11.IC40   6116   6116                   |
|     A74_12.IC28  A74_12.IC41   6116   6116                  |-|
|                                                             | |
|                                                             | |
|-|        2018                                               | |
  |                                                           | |
|-|                                                           | |
|                         2018                          24MHz | |
|                                                             |-|
|                                                              |
|T                                                             |
|                                                              |
|                                                2018          |
|          2018                                               |-|
|-|                                                           | |
  |                                                           | |
|-|                                                           | |
|                                                             | |
|                                                             | |
|   A74_14.IC2   A74_15.IC25                                  | |
|   A74_16.IC3   A74_17.IC26                                  |-|
|                                                 2018         |
|--------------------------------------------------------------|
Notes:
      2018       - Toshiba TMM2018 or Sony CXK5813 2k x8 SRAM (NDIP28)
      6116       - Hitachi HM6116LP-3 2k x8 SRAM (DIP24)
      T          - 18-way edge connector
      All A74_x  - Fujitsu 27C256 EPROM (DIP28)


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "sound/ay8910.h"
#include "sound/2203intf.h"
#include "includes/lsasquad.h"


#define MASTER_CLOCK    XTAL_24MHz


WRITE8_MEMBER(lsasquad_state::lsasquad_bankswitch_w)
{
	/* bits 0-2 select ROM bank */
	membank("bank1")->set_entry(data & 0x07);

	/* bit 3 is zeroed on startup, maybe reset sound CPU */

	/* bit 4 flips screen */
	flip_screen_set(data & 0x10);

	/* other bits unknown */
}

static ADDRESS_MAP_START( lsasquad_map, AS_PROGRAM, 8, lsasquad_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank1")
	AM_RANGE(0xa000, 0xbfff) AM_RAM /* SRAM */
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_SHARE("videoram")    /* SCREEN RAM */
	AM_RANGE(0xe000, 0xe3ff) AM_RAM AM_SHARE("scrollram")   /* SCROLL RAM */
	AM_RANGE(0xe400, 0xe5ff) AM_RAM AM_SHARE("spriteram")   /* OBJECT RAM */
	AM_RANGE(0xe800, 0xe800) AM_READ_PORT("DSWA")
	AM_RANGE(0xe801, 0xe801) AM_READ_PORT("DSWB")
	AM_RANGE(0xe802, 0xe802) AM_READ_PORT("DSWC")
	AM_RANGE(0xe803, 0xe803) AM_READ(lsasquad_mcu_status_r) /* COIN + 68705 status */
	AM_RANGE(0xe804, 0xe804) AM_READ_PORT("P1")
	AM_RANGE(0xe805, 0xe805) AM_READ_PORT("P2")
	AM_RANGE(0xe806, 0xe806) AM_READ_PORT("START")
	AM_RANGE(0xe807, 0xe807) AM_READ_PORT("SERVICE")
	AM_RANGE(0xea00, 0xea00) AM_WRITE(lsasquad_bankswitch_w)
	AM_RANGE(0xec00, 0xec00) AM_READWRITE(lsasquad_sound_result_r,lsasquad_sound_command_w)
	AM_RANGE(0xec01, 0xec01) AM_READ(lsasquad_sound_status_r)
	AM_RANGE(0xee00, 0xee00) AM_READWRITE(lsasquad_mcu_r,lsasquad_mcu_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lsasquad_sound_map, AS_PROGRAM, 8, lsasquad_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xa001) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0xc000, 0xc001) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0xd000, 0xd000) AM_READWRITE(lsasquad_sh_sound_command_r, lsasquad_sh_result_w)
	AM_RANGE(0xd400, 0xd400) AM_WRITE(lsasquad_sh_nmi_disable_w)
	AM_RANGE(0xd800, 0xd800) AM_WRITE(lsasquad_sh_nmi_enable_w)
	AM_RANGE(0xd800, 0xd800) AM_READ(lsasquad_sound_status_r)
	AM_RANGE(0xe000, 0xefff) AM_ROM     /* space for diagnostic ROM? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( lsasquad_m68705_map, AS_PROGRAM, 8, lsasquad_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(lsasquad_68705_port_a_r,lsasquad_68705_port_a_w)
	AM_RANGE(0x0001, 0x0001) AM_READWRITE(lsasquad_68705_port_b_r,lsasquad_68705_port_b_w)
	AM_RANGE(0x0002, 0x0002) AM_READ(lsasquad_mcu_status_r)
	AM_RANGE(0x0004, 0x0004) AM_WRITE(lsasquad_68705_ddr_a_w)
	AM_RANGE(0x0005, 0x0005) AM_WRITE(lsasquad_68705_ddr_b_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( storming_map, AS_PROGRAM, 8, lsasquad_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank1")
	AM_RANGE(0xa000, 0xbfff) AM_RAM /* SRAM */
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_SHARE("videoram")    /* SCREEN RAM */
	AM_RANGE(0xe000, 0xe3ff) AM_RAM AM_SHARE("scrollram")   /* SCROLL RAM */
	AM_RANGE(0xe400, 0xe5ff) AM_RAM AM_SHARE("spriteram")   /* OBJECT RAM */
	AM_RANGE(0xe800, 0xe800) AM_READ_PORT("DSWA")
	AM_RANGE(0xe801, 0xe801) AM_READ_PORT("DSWB")
	AM_RANGE(0xe802, 0xe802) AM_READ_PORT("DSWC")
	AM_RANGE(0xe803, 0xe803) AM_READ_PORT("COINS")
	AM_RANGE(0xe804, 0xe804) AM_READ_PORT("P1")
	AM_RANGE(0xe805, 0xe805) AM_READ_PORT("P2")
	AM_RANGE(0xe806, 0xe806) AM_READ_PORT("START")
	AM_RANGE(0xe807, 0xe807) AM_READ_PORT("SERVICE")
	AM_RANGE(0xea00, 0xea00) AM_WRITE(lsasquad_bankswitch_w)
	AM_RANGE(0xec00, 0xec00) AM_READWRITE(lsasquad_sound_result_r,lsasquad_sound_command_w)
	AM_RANGE(0xec01, 0xec01) AM_READ(lsasquad_sound_status_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( lsasquad )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "50000 100000" )
	PORT_DIPSETTING(    0x0c, "80000 150000" )
	PORT_DIPSETTING(    0x04, "100000 200000" )
	PORT_DIPSETTING(    0x00, "150000 300000" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x01, 0x01, "Freeze" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("MCU")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* 68705 ready to receive cmd */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* 0 = 68705 has sent result */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("START")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( storming )
	PORT_INCLUDE( lsasquad ) // no MCU

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END


/* DAIKAIJU */

static ADDRESS_MAP_START( daikaiju_map, AS_PROGRAM, 8, lsasquad_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank1")
	AM_RANGE(0xa000, 0xbfff) AM_RAM /* SRAM */
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_SHARE("videoram")    /* SCREEN RAM */
	AM_RANGE(0xe000, 0xe3ff) AM_RAM AM_SHARE("scrollram")   /* SCROLL RAM */
	AM_RANGE(0xe400, 0xe7ff) AM_RAM AM_SHARE("spriteram")   /* OBJECT RAM */
	AM_RANGE(0xe800, 0xe800) AM_READ_PORT("DSWA")
	AM_RANGE(0xe801, 0xe801) AM_READ_PORT("DSWB")
	AM_RANGE(0xe803, 0xe803) AM_READ(daikaiju_mcu_status_r) /* COIN + 68705 status */
	AM_RANGE(0xe804, 0xe804) AM_READ_PORT("P1")
	AM_RANGE(0xe805, 0xe805) AM_READ_PORT("P2")
	AM_RANGE(0xe806, 0xe806) AM_READ_PORT("START")
	AM_RANGE(0xe807, 0xe807) AM_READ_PORT("SERVICE")
	AM_RANGE(0xea00, 0xea00) AM_WRITE(lsasquad_bankswitch_w)
	AM_RANGE(0xec00, 0xec00) AM_WRITE(lsasquad_sound_command_w)
	AM_RANGE(0xec01, 0xec01) AM_READ(lsasquad_sound_status_r)
	AM_RANGE(0xee00, 0xee00) AM_READWRITE(lsasquad_mcu_r, lsasquad_mcu_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( daikaiju_sound_map, AS_PROGRAM, 8, lsasquad_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xa001) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0xc000, 0xc001) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0xd000, 0xd000) AM_READ(daikaiju_sh_sound_command_r)
	AM_RANGE(0xd400, 0xd400) AM_WRITENOP
	AM_RANGE(0xd800, 0xd800) AM_READ(daikaiju_sound_status_r) AM_WRITENOP
	AM_RANGE(0xdc00, 0xdc00) AM_WRITENOP
	AM_RANGE(0xe000, 0xefff) AM_ROM /* space for diagnostic ROM? */
ADDRESS_MAP_END

static INPUT_PORTS_START( daikaiju )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW ) //test mode
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) // detailed description at the top of file
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "100000" )
	PORT_DIPSETTING(    0x0c, "30000" )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" ) //stop mode
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWC")
	//unused


	PORT_START("MCU")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* 68705 ready to receive cmd */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* 0 = 68705 has sent result */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("START")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0,
			16*8+3, 16*8+2, 16*8+1, 16*8+0, 16*8+8+3, 16*8+8+2, 16*8+8+1, 16*8+8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16 },
	64*8
};

static GFXDECODE_START( lsasquad )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 256, 16 )
GFXDECODE_END



WRITE_LINE_MEMBER(lsasquad_state::irqhandler)
{
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER(lsasquad_state::unk)
{
}

MACHINE_START_MEMBER(lsasquad_state,lsasquad)
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 8, &ROM[0x10000], 0x2000);

	save_item(NAME(m_port_a_in));
	save_item(NAME(m_port_a_out));
	save_item(NAME(m_ddr_a));
	save_item(NAME(m_port_b_in));
	save_item(NAME(m_port_b_out));
	save_item(NAME(m_ddr_b));
	save_item(NAME(m_mcu_sent));
	save_item(NAME(m_main_sent));
	save_item(NAME(m_from_main));
	save_item(NAME(m_from_mcu));

	save_item(NAME(m_sound_pending));
	save_item(NAME(m_sound_nmi_enable));
	save_item(NAME(m_pending_nmi));
	save_item(NAME(m_sound_cmd));
	save_item(NAME(m_sound_result));
}

MACHINE_RESET_MEMBER(lsasquad_state,lsasquad)
{
	m_sound_pending = 0;
	m_sound_nmi_enable = 0;
	m_pending_nmi = 0;
	m_sound_cmd = 0;
	m_sound_result = 0;

	m_port_a_in = 0;
	m_port_a_out = 0;
	m_ddr_a = 0;
	m_port_b_in = 0;
	m_port_b_out = 0;
	m_ddr_b = 0;
	m_mcu_sent = 0;
	m_main_sent = 0;
	m_from_main = 0;
	m_from_mcu = 0;
}

/* Note: lsasquad clock values are not verified */
static MACHINE_CONFIG_START( lsasquad, lsasquad_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK / 4)
	MCFG_CPU_PROGRAM_MAP(lsasquad_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", lsasquad_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, MASTER_CLOCK / 8)
	MCFG_CPU_PROGRAM_MAP(lsasquad_sound_map)
								/* IRQs are triggered by the YM2203 */
	MCFG_CPU_ADD("mcu", M68705, MASTER_CLOCK / 8)
	MCFG_CPU_PROGRAM_MAP(lsasquad_m68705_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(30000)) /* 500 CPU slices per frame - an high value to ensure proper */
							/* synchronization of the CPUs */
							/* main<->sound synchronization depends on this */

	MCFG_MACHINE_START_OVERRIDE(lsasquad_state,lsasquad)
	MCFG_MACHINE_RESET_OVERRIDE(lsasquad_state,lsasquad)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(lsasquad_state, screen_update_lsasquad)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", lsasquad)
	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 512)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, MASTER_CLOCK / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.12)

	MCFG_SOUND_ADD("ymsnd", YM2203, MASTER_CLOCK / 8)
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(lsasquad_state, irqhandler))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(lsasquad_state, unk))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(lsasquad_state, unk))
	MCFG_SOUND_ROUTE(0, "mono", 0.12)
	MCFG_SOUND_ROUTE(1, "mono", 0.12)
	MCFG_SOUND_ROUTE(2, "mono", 0.12)
	MCFG_SOUND_ROUTE(3, "mono", 0.63)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( storming, lsasquad )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(storming_map)

	MCFG_DEVICE_REMOVE("mcu")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( daikaiju, lsasquad_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK / 4)
	MCFG_CPU_PROGRAM_MAP(daikaiju_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", lsasquad_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, MASTER_CLOCK / 8)
	MCFG_CPU_PROGRAM_MAP(daikaiju_sound_map)
	/* IRQs are triggered by the YM2203 */

	MCFG_CPU_ADD("mcu", M68705, MASTER_CLOCK / 8)
	MCFG_CPU_PROGRAM_MAP(lsasquad_m68705_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(30000)) /* 500 CPU slices per frame - an high value to ensure proper */
							/* synchronization of the CPUs */
							/* main<->sound synchronization depends on this */

	MCFG_MACHINE_START_OVERRIDE(lsasquad_state,lsasquad)
	MCFG_MACHINE_RESET_OVERRIDE(lsasquad_state,lsasquad)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(lsasquad_state, screen_update_daikaiju)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", lsasquad)
	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 512)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, MASTER_CLOCK / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.12)

	MCFG_SOUND_ADD("ymsnd", YM2203, MASTER_CLOCK / 8)
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(lsasquad_state, irqhandler))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(lsasquad_state, unk))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(lsasquad_state, unk))
	MCFG_SOUND_ROUTE(0, "mono", 0.12)
	MCFG_SOUND_ROUTE(1, "mono", 0.12)
	MCFG_SOUND_ROUTE(2, "mono", 0.12)
	MCFG_SOUND_ROUTE(3, "mono", 0.63)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( lsasquad )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "a64-21.4",     0x00000, 0x8000, CRC(5ff6b017) SHA1(96cc74edba1208bb8e82f93d2d3a88ea24922dc0) )
	/* ROMs banked at 8000-9fff */
	ROM_LOAD( "a64-20.3",     0x10000, 0x8000, CRC(7f8b4979) SHA1(975b1a678e1f7d7b5789565063177593639645ce) )
	ROM_LOAD( "a64-19.2",     0x18000, 0x8000, CRC(ba31d34a) SHA1(e2c515ae8146a37534b19403c03fc5a8719f115f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the second CPU */
	ROM_LOAD( "a64-04.44",    0x0000, 0x8000, CRC(c238406a) SHA1(bb8f9d952c4568edb375328a1f9f6681a1bb5907) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a64-05.35",    0x0000, 0x0800, CRC(572677b9) SHA1(e098d5d842bcc81221ba56652a7019505d8be082) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a64-10.27",    0x00000, 0x8000, CRC(bb4f1b37) SHA1(ce8dc962a3d04a624e36b57dc678e7ca7726ba1d) )
	ROM_LOAD( "a64-22.28",    0x08000, 0x8000, CRC(58e03b89) SHA1(ccec83bcd7cb2be3ba46e9fbc7952349fa8faadf) )
	ROM_LOAD( "a64-11.40",    0x10000, 0x8000, CRC(a3bbc0b3) SHA1(f565d323575af3c2e95412c50130e88954fc238c) )
	ROM_LOAD( "a64-23.41",    0x18000, 0x8000, CRC(377a538b) SHA1(1174838309a331ffec7b60d6ceaa98a02fdbe210) )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "a64-14.2",     0x00000, 0x8000, CRC(a72e2041) SHA1(c537d1620fe8562aef39a0279b35139eb0668bf9) )
	ROM_LOAD( "a64-16.3",     0x08000, 0x8000, CRC(05206333) SHA1(a7463279446de9d633ea18f1e1eb5f610d982a37) )
	ROM_LOAD( "a64-15.25",    0x10000, 0x8000, CRC(01ed5851) SHA1(6034376d30d1d17fe9aab07cb40009c4f3c03690) )
	ROM_LOAD( "a64-17.26",    0x18000, 0x8000, CRC(6eaf3735) SHA1(a91fd7c9a6f2f58d311e40edc29d1e4f97746146) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "a64-07.22",    0x0000, 0x0400, CRC(82802bbb) SHA1(4f54c9364a12809898eabd1eb13d16a6c9f0f532) )    /* red   (bottom half unused) */
	ROM_LOAD( "a64-08.23",    0x0200, 0x0400, CRC(aa9e1dbd) SHA1(be7dfabf5306747fa3d5f1f735d0064673f19c91) )    /* green (bottom half unused) */
	ROM_LOAD( "a64-09.24",    0x0400, 0x0400, CRC(dca86295) SHA1(a6f6af60caaad9f49d72a8c2ff1e6115471f8c63) )    /* blue  (bottom half unused) */
	ROM_LOAD( "a64-06.9",     0x0600, 0x0400, CRC(7ced30ba) SHA1(f22de13d4fd49b7b2ffd06032eb5e14fbdeec91c) )    /* priority */

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal16l8a.14", 0x0000, 0x0104, CRC(a7cc157d) SHA1(f06f750636d59a610e0b0eda8cb791780ebc57a5) )
ROM_END

ROM_START( storming )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "stpartyj.001", 0x00000, 0x8000, CRC(07e6bc61) SHA1(6989a1401868dd93c9466cfd1636ac48a734a5d4) )
	/* ROMs banked at 8000-9fff */
	ROM_LOAD( "stpartyj.002", 0x10000, 0x8000, CRC(1c7fe5d5) SHA1(15c09e3301d8ce55e59fe90db9f50ee19584ab7b) )
	ROM_LOAD( "stpartyj.003", 0x18000, 0x8000, CRC(159f23a6) SHA1(2cb4ed78e54dc2acbbfc2d4cfb2d29ff604aa9ae) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the second CPU */
	ROM_LOAD( "a64-04.44",    0x0000, 0x8000, CRC(c238406a) SHA1(bb8f9d952c4568edb375328a1f9f6681a1bb5907) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a64-10.27",    0x00000, 0x8000, CRC(bb4f1b37) SHA1(ce8dc962a3d04a624e36b57dc678e7ca7726ba1d) )
	ROM_LOAD( "stpartyj.009", 0x08000, 0x8000, CRC(8ee2443b) SHA1(855d8189efcfc796daa6b36f86d2872cc48adfde) )
	ROM_LOAD( "a64-11.40",    0x10000, 0x8000, CRC(a3bbc0b3) SHA1(f565d323575af3c2e95412c50130e88954fc238c) )
	ROM_LOAD( "stpartyj.011", 0x18000, 0x8000, CRC(f342d42f) SHA1(ef9367ad9763f4b38e0f12805c1eee7c430758c2) )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "a64-14.2",     0x00000, 0x8000, CRC(a72e2041) SHA1(c537d1620fe8562aef39a0279b35139eb0668bf9) )
	ROM_LOAD( "a64-16.3",     0x08000, 0x8000, CRC(05206333) SHA1(a7463279446de9d633ea18f1e1eb5f610d982a37) )
	ROM_LOAD( "a64-15.25",    0x10000, 0x8000, CRC(01ed5851) SHA1(6034376d30d1d17fe9aab07cb40009c4f3c03690) )
	ROM_LOAD( "a64-17.26",    0x18000, 0x8000, CRC(6eaf3735) SHA1(a91fd7c9a6f2f58d311e40edc29d1e4f97746146) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "a64-07.22",    0x0000, 0x0400, CRC(82802bbb) SHA1(4f54c9364a12809898eabd1eb13d16a6c9f0f532) )    /* red   (bottom half unused) */
	ROM_LOAD( "a64-08.23",    0x0200, 0x0400, CRC(aa9e1dbd) SHA1(be7dfabf5306747fa3d5f1f735d0064673f19c91) )    /* green (bottom half unused) */
	ROM_LOAD( "a64-09.24",    0x0400, 0x0400, CRC(dca86295) SHA1(a6f6af60caaad9f49d72a8c2ff1e6115471f8c63) )    /* blue  (bottom half unused) */
	ROM_LOAD( "a64-06.9",     0x0600, 0x0400, CRC(7ced30ba) SHA1(f22de13d4fd49b7b2ffd06032eb5e14fbdeec91c) )    /* priority */

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal16l8a.14", 0x0000, 0x0104, CRC(a7cc157d) SHA1(f06f750636d59a610e0b0eda8cb791780ebc57a5) )
ROM_END

ROM_START( daikaiju )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "a74_01-1.ic4",   0x00000, 0x8000, CRC(89c13d7f) SHA1(2eaec80d7aa360b700387df00b37a692acc50d74) )
	/* ROMs banked at 8000-9fff */
	ROM_LOAD( "a74_02.ic3",     0x10000, 0x8000, CRC(8ddf6131) SHA1(b5b23550e7ee52554bc1f045ed6f42e254a05bf4) )
	ROM_LOAD( "a74_03.ic2",     0x18000, 0x8000, CRC(3911ffed) SHA1(ba6dbd74d37ef26621a02baf3479e2764d10d2ba) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the second CPU */
	ROM_LOAD( "a74_04.ic44",    0x0000, 0x8000, CRC(98a6a703) SHA1(0c169a7a5f8b26606f67ee7f14bd487951536ac5) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "a74_05.ic35",    0x0000, 0x0800, CRC(d66df06f) SHA1(6a61eb15aef7f3b7a66ec9d87c0bdd731d6cb079) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a74_10.ic27",    0x00000, 0x8000, CRC(3123158e) SHA1(cdebf63c283c5c042596b0a13361fd01245e9c42) )
	ROM_LOAD( "a74_12.ic28",    0x08000, 0x8000, CRC(8a4e6c3a) SHA1(85b5c8630fe9d4faea6787f80a66ee41da64e64b) )
	ROM_LOAD( "a74_11.ic40",    0x10000, 0x8000, CRC(6432ae38) SHA1(5514c5259c5ced393b1c39436025dd13c0c61d82) )
	ROM_LOAD( "a74_13.ic41",    0x18000, 0x8000, CRC(1a1be4bb) SHA1(cbe647b2291db6432ea2cb61b8108cc089adb3c7) )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "a74_14.ic2",     0x00000, 0x8000, CRC(c28e9c35) SHA1(e9c697a91e5281ab08a43169004c235ada9391db) )
	ROM_LOAD( "a74_16.ic3",     0x08000, 0x8000, CRC(4b1c7921) SHA1(37e26a9007bfdf71af021fb218ea2b16f91d9c37) )
	ROM_LOAD( "a74_16.ic25",    0x10000, 0x8000, CRC(ef4d1945) SHA1(6b5e898e486d5786fc5d151f1fcca0015829365d) )
	ROM_LOAD( "a74_17.ic26",    0x18000, 0x8000, CRC(d1077878) SHA1(e69893db6b63d5a5192b521d61a86f60b7029b7e) )

	ROM_REGION( 0x0a00, "proms", 0 )
	ROM_LOAD( "a74_07.ic22",    0x0000, 0x0400, CRC(66132341) SHA1(8c6723dfc4f856ef27998411a98c40783d13ac41) )  /* red   (bottom half unused) */
	ROM_LOAD( "a74_08.ic23",    0x0200, 0x0400, CRC(fb3f0273) SHA1(591577c94865e2e6465e0016350450a19000e52d) )  /* green (bottom half unused) */
	ROM_LOAD( "a74_09.ic24",    0x0400, 0x0400, CRC(bed6709d) SHA1(ba5435728d6b7847bc86878f6122ce1f86982f0a) )  /* blue  (bottom half unused) */
	ROM_LOAD( "a74_06.ic9",     0x0600, 0x0400, CRC(cad554e7) SHA1(7890d948bfef198309df810f8401d224224a73a1) )  /* priority */
ROM_END


GAME( 1986, lsasquad, 0,        lsasquad, lsasquad, driver_device, 0, ROT270, "Taito", "Land Sea Air Squad / Riku Kai Kuu Saizensen", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1986, storming, lsasquad, storming, storming, driver_device, 0, ROT270, "bootleg", "Storming Party / Riku Kai Kuu Saizensen", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1986, daikaiju, 0,        daikaiju, daikaiju, driver_device, 0, ROT270, "Taito", "Daikaiju no Gyakushu", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

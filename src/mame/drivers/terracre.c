// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano
/******************************************************************
Terra Cresta (preliminary)
Nichibutsu 1985
68000 + Z80

driver by Carlos A. Lozano (calb@gsyc.inf.uc3m.es)

TODO:

  - I'm playing samples with a DAC, but they could be ADPCM
  - find correct high-scores table for 'amazon'


Stephh's notes (based on the games M68000 code and some tests) :


1) 'terracr*'

  - Each high-score name is made up of 10 chars.


2) 'amazon'

  - Each high-score name is made up of 3 chars followed by 7 0x00.


3) 'amatelas'

  - Each high-score name is made up of 10 chars.


4) 'horekid*'

  - Each high-score name is made up of 3 chars.

  - There is a "debug mode" !
    Set "Debug Mode" Dip Switch to ON and be sure that "Cabinet" Dip Switch
    is set to "Upright". Its features (see below) only affect player 1 !
    Features :
      * invulnerability and infinite time :
          . insert a coin
          . press player 2 button 1 and 2 and START1 (player 1 buttons 1 and 2 must NOT be pressed !)
      * level select (there are 32 levels) :
          . insert a coin
          . press player 2 button 1 and 2 ("00" will be displayed - this is an hex. display)
          . press player 2 button 1 and 2 and player 1 button 1 to increase level
          . press player 2 button 1 and 2 and player 1 button 2 to decrease level
          . press START1 to start a game with the selected level



Amazon
(c)1986 Nichibutsu

AT-1
                                                16MHz

                  6116    -     -    10    9
                  6116    -     -    12    11   68000-8

 6116
 6116
                                              SW
    15 14 13   clr.12f clr.11f clr.10f        SW

    16   1412M2 XBA


AT-2

 2G    6  7                              8
    4E 4  5                              6116

                                         Z80A   YM3526
      2148 2148
      2148 2148         2148 2148
                                         1 2 3  6116
 22MHz
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "sound/2203intf.h"
#include "sound/3526intf.h"
#include "includes/terracre.h"


static const UINT16 mAmazonProtData[] =
{
	/* default high scores (0x40db4) - wrong data ? */
	0x0000,0x5000,0x5341,0x4b45,0x5349,0x4755,0x5245,
	0x0000,0x4000,0x0e4b,0x4154,0x5544,0x4f4e,0x0e0e,
	0x0000,0x3000,0x414e,0x4b41,0x4b45,0x5544,0x4f4e,
	0x0000,0x2000,0x0e0e,0x4b49,0x5455,0x4e45,0x0e0e,
	0x0000,0x1000,0x0e4b,0x414b,0x4553,0x4f42,0x410e,

	/* code (0x40d92) */
	0x4ef9,0x0000,0x62fa,0x0000,0x4ef9,0x0000,0x805E,0x0000,
	0xc800 /* checksum */
};

static const UINT16 mAmatelasProtData[] =
{
	/* default high scores (0x40db4) */
	0x0000,0x5000,0x5341,0x4b45,0x5349,0x4755,0x5245,
	0x0000,0x4000,0x0e4b,0x4154,0x5544,0x4f4e,0x0e0e,
	0x0000,0x3000,0x414e,0x4b41,0x4b45,0x5544,0x4f4e,
	0x0000,0x2000,0x0e0e,0x4b49,0x5455,0x4e45,0x0e0e,
	0x0000,0x1000,0x0e4b,0x414b,0x4553,0x4f42,0x410e,

	/* code (0x40d92) */
	0x4ef9,0x0000,0x632e,0x0000,0x4ef9,0x0000,0x80C2,0x0000,
	0x6100 /* checksum */
};

static const UINT16 mHoreKidProtData[] =
{
	/* N/A */
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,

	/* code (0x40dba) */
	0x4e75,0x4e75,0x4e75,0x4e75,0x4e75,0x4e75,0x4e75,0x4e75,
	0x1800 /* checksum */
};

WRITE16_MEMBER(terracre_state::amazon_sound_w)
{
	soundlatch_byte_w(space,0,((data & 0x7f) << 1) | 1);
}

READ8_MEMBER(terracre_state::soundlatch_clear_r)
{
	soundlatch_clear_byte_w(space,0,0);
	return 0;
}

READ16_MEMBER(terracre_state::amazon_protection_r)
{
	offset = m_mAmazonProtReg[2];
	if( offset<=0x56 )
	{
		UINT16 data;
		data = m_mpProtData[offset/2];
		if( offset&1 ) return data&0xff;
		return data>>8;
	}
	return 0;
}

WRITE16_MEMBER(terracre_state::amazon_protection_w)
{
	if( ACCESSING_BITS_0_7 )
	{
		if( offset==1 )
		{
			m_mAmazonProtCmd = data;
		}
		else
		{
			if( m_mAmazonProtCmd>=32 && m_mAmazonProtCmd<=0x37 )
			{
				m_mAmazonProtReg[m_mAmazonProtCmd-0x32] = data;
			}
		}
	}
}

MACHINE_START_MEMBER(terracre_state,amazon)
{
	/* set up for save */
	save_item(NAME(m_mAmazonProtCmd));
	save_item(NAME(m_mAmazonProtReg));
}

static ADDRESS_MAP_START( terracre_map, AS_PROGRAM, 16, terracre_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x020000, 0x0201ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x020200, 0x021fff) AM_RAM
	AM_RANGE(0x022000, 0x022fff) AM_WRITE(amazon_background_w) AM_SHARE("bg_videoram")
	AM_RANGE(0x023000, 0x023fff) AM_RAM
	AM_RANGE(0x024000, 0x024001) AM_READ_PORT("P1")
	AM_RANGE(0x024002, 0x024003) AM_READ_PORT("P2")
	AM_RANGE(0x024004, 0x024005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x024006, 0x024007) AM_READ_PORT("DSW")
	AM_RANGE(0x026000, 0x026001) AM_WRITE(amazon_flipscreen_w)  /* flip screen & coin counters */
	AM_RANGE(0x026002, 0x026003) AM_WRITE(amazon_scrollx_w)
	AM_RANGE(0x026004, 0x026005) AM_WRITE(amazon_scrolly_w)
	AM_RANGE(0x02600c, 0x02600d) AM_WRITE(amazon_sound_w)
	AM_RANGE(0x028000, 0x0287ff) AM_WRITE(amazon_foreground_w) AM_SHARE("fg_videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( amazon_map, AS_PROGRAM, 16, terracre_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x040000, 0x0401ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x040200, 0x040fff) AM_RAM
	AM_RANGE(0x042000, 0x042fff) AM_WRITE(amazon_background_w) AM_SHARE("bg_videoram")
	AM_RANGE(0x044000, 0x044001) AM_READ_PORT("IN0")
	AM_RANGE(0x044002, 0x044003) AM_READ_PORT("IN1")
	AM_RANGE(0x044004, 0x044005) AM_READ_PORT("IN2")
	AM_RANGE(0x044006, 0x044007) AM_READ_PORT("IN3")
	AM_RANGE(0x046000, 0x046001) AM_WRITE(amazon_flipscreen_w)  /* flip screen & coin counters */
	AM_RANGE(0x046002, 0x046003) AM_WRITE(amazon_scrollx_w)
	AM_RANGE(0x046004, 0x046005) AM_WRITE(amazon_scrolly_w)
	AM_RANGE(0x04600c, 0x04600d) AM_WRITE(amazon_sound_w)
	AM_RANGE(0x050000, 0x050fff) AM_WRITE(amazon_foreground_w) AM_SHARE("fg_videoram")
	AM_RANGE(0x070000, 0x070003) AM_READWRITE(amazon_protection_r, amazon_protection_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, terracre_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_3526_io_map, AS_IO, 8, terracre_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("ymsnd", ym3526_device, write)
	AM_RANGE(0x02, 0x02) AM_DEVWRITE("dac1", dac_device, write_signed8)
	AM_RANGE(0x03, 0x03) AM_DEVWRITE("dac2", dac_device, write_signed8)
	AM_RANGE(0x04, 0x04) AM_READ(soundlatch_clear_r)
	AM_RANGE(0x06, 0x06) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_2203_io_map, AS_IO, 8, terracre_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("ym1", ym2203_device, write)
	AM_RANGE(0x02, 0x02) AM_DEVWRITE("dac1", dac_device, write_signed8)
	AM_RANGE(0x03, 0x03) AM_DEVWRITE("dac2", dac_device, write_signed8)
	AM_RANGE(0x04, 0x04) AM_READ(soundlatch_clear_r)
	AM_RANGE(0x06, 0x06) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( terracre )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x2000, IP_ACTIVE_LOW )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPSETTING(      0x0000, "6" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(      0x000c, "20k then every 60k" )    // "20000 60000" in the "test mode"
	PORT_DIPSETTING(      0x0008, "30k then every 70k" )    // "30000 70000" in the "test mode"
	PORT_DIPSETTING(      0x0004, "40k then every 80k" )    // "40000 80000" in the "test mode"
	PORT_DIPSETTING(      0x0000, "50k then every 90k" )    // "50000 90000" in the "test mode"
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED( 0x0040, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x0080, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Flip_Screen ) )  // not in the "test mode"
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Complete Invulnerability (Cheat)")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Base Ship Invulnerability (Cheat)")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( amazon )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x2000, IP_ACTIVE_LOW )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPSETTING(      0x0000, "6" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(      0x000c, "20k then every 40k" )    // "20000 40000" in the "test mode"
	PORT_DIPSETTING(      0x0008, "50k then every 40k" )    // "50000 40000" in the "test mode"
	PORT_DIPSETTING(      0x0004, "20k then every 70k" )    // "20000 70000" in the "test mode"
	PORT_DIPSETTING(      0x0000, "50k then every 70k" )    // "50000 70000" in the "test mode"
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED( 0x0040, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x0080, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Flip_Screen ) )  // not in the "test mode"
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Level" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( High ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Sprite Test" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( horekid )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPSETTING(      0x0000, "6" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(      0x000c, "20k then every 60k" )    // "20000 60000" in the "test mode"
	PORT_DIPSETTING(      0x0008, "50k then every 60k" )    // "50000 60000" in the "test mode"
	PORT_DIPSETTING(      0x0004, "20k then every 90k" )    // "20000 90000" in the "test mode"
	PORT_DIPSETTING(      0x0000, "50k then every 90k" )    // "50000 90000" in the "test mode"
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED( 0x1000, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, "Debug Mode" )
	PORT_DIPSETTING(      0xc000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )       // "Cabinet" Dip Switch must be set to "Upright" too !
//  PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )      // duplicated setting
//  PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )      // duplicated setting

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x2000, IP_ACTIVE_LOW)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static const gfx_layout char_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tile_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{
		4, 0, 12, 8, 20, 16, 28, 24,
		32+4, 32+0, 32+12, 32+8, 32+20, 32+16, 32+28, 32+24
	},
	{
		0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
		8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64
	},
	64*16
};

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{
			4,  0, RGN_FRAC(1,2)+4,  RGN_FRAC(1,2)+0,
		12,  8, RGN_FRAC(1,2)+12, RGN_FRAC(1,2)+8,
		20, 16, RGN_FRAC(1,2)+20, RGN_FRAC(1,2)+16,
		28, 24, RGN_FRAC(1,2)+28, RGN_FRAC(1,2)+24
	},
	{
		0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32
	},
	32*16
};

static GFXDECODE_START( terracre )
	GFXDECODE_ENTRY( "gfx1", 0, char_layout,            0,   1 )
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout,         1*16,  16 )
	GFXDECODE_ENTRY( "gfx3", 0, sprite_layout, 1*16+16*16, 256 )
GFXDECODE_END


static MACHINE_CONFIG_START( ym3526, terracre_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz/2)   // 8mhz
	MCFG_CPU_PROGRAM_MAP(terracre_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", terracre_state,  irq1_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_16MHz/4)     // 4.0mhz when compared to sound recordings, should be derived from XTAL_22MHz? how?
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_3526_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(terracre_state, irq0_line_hold,  XTAL_16MHz/4/512) // ?

	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( 60 )
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(terracre_state, screen_update_amazon)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT270)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", terracre)
	MCFG_PALETTE_ADD("palette", 1*16+16*16+16*256)
	MCFG_PALETTE_INDIRECT_ENTRIES(256)
	MCFG_PALETTE_INIT_OWNER(terracre_state, terracre)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3526, XTAL_16MHz/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ym2203, ym3526 )
	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_IO_MAP(sound_2203_io_map)

	MCFG_DEVICE_REMOVE("ymsnd")

	MCFG_SOUND_ADD("ym1", YM2203, XTAL_16MHz/4)
	MCFG_SOUND_ROUTE(0, "mono", 0.20)
	MCFG_SOUND_ROUTE(1, "mono", 0.20)
	MCFG_SOUND_ROUTE(2, "mono", 0.20)
	MCFG_SOUND_ROUTE(3, "mono", 0.40)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( amazon, ym3526 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(amazon_map)

	MCFG_MACHINE_START_OVERRIDE(terracre_state,amazon)
MACHINE_CONFIG_END


/**************************************

  ROM definitions

***************************************/

/* newer PCB, manufactured in 1987, basically the same as the Amazon layout above.
 Has 4*32K prg ROMs instead of 8*16K, contents is the same as other terracre sets though.
 top board:    BK-1 (1502), 16MHz XTAL, 68K-8, Nichibutsu 1412M2 XBA (gfx chip?)
 bottom board: BK-2 (1502), 22MHz XTAL, Z80, YM3526 */

ROM_START( terracre )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 68000 code (main CPU) */
	ROM_LOAD16_BYTE( "bk1_1.4b",    0x00001, 0x8000, CRC(60932770) SHA1(887be7a44cb7bf30d11274d34896217cc87ae158) )
	ROM_LOAD16_BYTE( "bk1_3.4d",    0x00000, 0x8000, CRC(cb36240e) SHA1(24696503d9720ced869bb96ec64f336679726668) )
	ROM_LOAD16_BYTE( "bk1_2.6b",    0x10001, 0x8000, CRC(539352f2) SHA1(b960f75d12ebdcd6781a073a66b8e503a8f55186) )
	ROM_LOAD16_BYTE( "bk1_4.6d",    0x10000, 0x8000, CRC(19387586) SHA1(76473493d173efde83ded52ad721d2c532f590e2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )/* Z80 code (sound) */
	ROM_LOAD( "bk2_11.15b",   0x0000, 0x4000, CRC(604c3b11) SHA1(c01d1ddae40fa8b65dfc72f959942cb9664a548b) )
	ROM_LOAD( "bk2_12.17b",   0x4000, 0x4000, CRC(affc898d) SHA1(a78f06fa125de16fcdb8f4dc1629eb775aad913a) )
	ROM_LOAD( "bk2_13.18b",   0x8000, 0x4000, CRC(302dc0ab) SHA1(4db8f12e70f9adf1eb993c6a8af68b5edbf79773) )

	ROM_REGION( 0x02000, "gfx1", 0 )    /* tiles */
	ROM_LOAD( "bk2_14.16g",   0x00000, 0x2000, CRC(591a3804) SHA1(e1b46f5652e7f9677d75f01c6132975ace4facdd) )

	ROM_REGION( 0x10000, "gfx2", 0 )    /* background */
	ROM_LOAD( "bk1_5.15f",   0x00000, 0x8000, CRC(984a597f) SHA1(1f33892f160691c44872b37f0f6cb1493c9f7fb1) )
	ROM_LOAD( "bk1_6.17f",   0x08000, 0x8000, CRC(30e297ff) SHA1(9843826ae63039d6693c8a0b30af721d70f40056) )

	ROM_REGION( 0x10000, "gfx3", 0 )    /* sprites */
	ROM_LOAD( "bk2_7.6e",    0x00000, 0x4000, CRC(bcf7740b) SHA1(8701862c35eb8fb1ec239253136a3858ebea4d0c) )
	ROM_LOAD( "bk2_8.7e",    0x04000, 0x4000, CRC(a70b565c) SHA1(153e5f5a9927c294660dd0d636a9f651d4984d6d) )
	ROM_LOAD( "bk2_9.6g",    0x08000, 0x4000, CRC(4a9ec3e6) SHA1(0a35b82fb49ecf7edafd02744a48490e744c0a00) )
	ROM_LOAD( "bk2_10.7g",   0x0c000, 0x4000, CRC(450749fc) SHA1(376ab98ab8db56ed45f7d97a221dfd52e389cb5a) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "bk1_3.10f", 0x0000, 0x0100, CRC(ce07c544) SHA1(c3691cb420c88f1887a55e3035b5d017decbc17a) )   /* red component */
	ROM_LOAD( "bk1_2.11f", 0x0100, 0x0100, CRC(566d323a) SHA1(fe83585a0d9c7f942a5e54620b627a5a17a0fcf4) )   /* green component */
	ROM_LOAD( "bk1_1.12f", 0x0200, 0x0100, CRC(7ea63946) SHA1(d7b89694a80736c7605b5c83d25d8b706f4504ab) )   /* blue component */
	ROM_LOAD( "bk2_4.2g",  0x0300, 0x0100, CRC(08609bad) SHA1(e5daee3c3fea6620e3c2b91becd93bc4d3cdf011) )   /* sprite lookup table */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "bk2_5.4e",  0x0000, 0x0100, CRC(2c43991f) SHA1(312112832bee511b0545524295aa9bc2e756db0f) )   /* sprite palette bank */

	/* 11e and 12a might be PALs */
ROM_END

ROM_START( terracreo ) // older pcb
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 68000 code (main CPU) */
	ROM_LOAD16_BYTE( "1a_4b.rom",    0x00001, 0x4000, CRC(76f17479) SHA1(e6be7f78fe7dc9d66feb3ada6ad08d461c66640d) )
	ROM_LOAD16_BYTE( "1a_4d.rom",    0x00000, 0x4000, CRC(8119f06e) SHA1(314e2d8e75f66862cf6567ac05f417a3a66f1254) )
	ROM_LOAD16_BYTE( "1a_6b.rom",    0x08001, 0x4000, CRC(ba4b5822) SHA1(0de3ce04e14aa5757936babdec9cd1341d4a06d6) )
	ROM_LOAD16_BYTE( "1a_6d.rom",    0x08000, 0x4000, CRC(ca4852f6) SHA1(12e968efb890ff4f982c2e04e090ac4339a97fc0) )
	ROM_LOAD16_BYTE( "1a_7b.rom",    0x10001, 0x4000, CRC(d0771bba) SHA1(ebbc24562d677488a536cb515d761f07cd50425c) )
	ROM_LOAD16_BYTE( "1a_7d.rom",    0x10000, 0x4000, CRC(029d59d9) SHA1(51053cafd5e7a4a5ba7008c6c6b28c612d935f40) )
	ROM_LOAD16_BYTE( "1a_9b.rom",    0x18001, 0x4000, CRC(69227b56) SHA1(58c8aa4baa1f5ddfc151f5ed6284a06e87866dd7) )
	ROM_LOAD16_BYTE( "1a_9d.rom",    0x18000, 0x4000, CRC(5a672942) SHA1(3890f87edb9047f3e4c6f4d4b47b7f9873962148) )

	ROM_REGION( 0x10000, "audiocpu", 0 )/* Z80 code (sound) */
	ROM_LOAD( "2a_15b.rom",   0x0000, 0x4000, CRC(604c3b11) SHA1(c01d1ddae40fa8b65dfc72f959942cb9664a548b) )
	ROM_LOAD( "2a_17b.rom",   0x4000, 0x4000, CRC(affc898d) SHA1(a78f06fa125de16fcdb8f4dc1629eb775aad913a) )
	ROM_LOAD( "2a_18b.rom",   0x8000, 0x4000, CRC(302dc0ab) SHA1(4db8f12e70f9adf1eb993c6a8af68b5edbf79773) )

	ROM_REGION( 0x02000, "gfx1", 0 )    /* tiles */
	ROM_LOAD( "2a_16b.rom",   0x00000, 0x2000, CRC(591a3804) SHA1(e1b46f5652e7f9677d75f01c6132975ace4facdd) )

	ROM_REGION( 0x10000, "gfx2", 0 )    /* background */
	ROM_LOAD( "1a_15f.rom",   0x00000, 0x8000, CRC(984a597f) SHA1(1f33892f160691c44872b37f0f6cb1493c9f7fb1) )
	ROM_LOAD( "1a_17f.rom",   0x08000, 0x8000, CRC(30e297ff) SHA1(9843826ae63039d6693c8a0b30af721d70f40056) )

	ROM_REGION( 0x10000, "gfx3", 0 )    /* sprites */
	ROM_LOAD( "2a_6e.rom",    0x00000, 0x4000, CRC(bcf7740b) SHA1(8701862c35eb8fb1ec239253136a3858ebea4d0c) )
	ROM_LOAD( "2a_7e.rom",    0x04000, 0x4000, CRC(a70b565c) SHA1(153e5f5a9927c294660dd0d636a9f651d4984d6d) )
	ROM_LOAD( "2a_6g.rom",    0x08000, 0x4000, CRC(4a9ec3e6) SHA1(0a35b82fb49ecf7edafd02744a48490e744c0a00) )
	ROM_LOAD( "2a_7g.rom",    0x0c000, 0x4000, CRC(450749fc) SHA1(376ab98ab8db56ed45f7d97a221dfd52e389cb5a) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "tc1a_10f.bin", 0x0000, 0x0100, CRC(ce07c544) SHA1(c3691cb420c88f1887a55e3035b5d017decbc17a) )    /* red component */
	ROM_LOAD( "tc1a_11f.bin", 0x0100, 0x0100, CRC(566d323a) SHA1(fe83585a0d9c7f942a5e54620b627a5a17a0fcf4) )    /* green component */
	ROM_LOAD( "tc1a_12f.bin", 0x0200, 0x0100, CRC(7ea63946) SHA1(d7b89694a80736c7605b5c83d25d8b706f4504ab) )    /* blue component */
	ROM_LOAD( "tc2a_2g.bin",  0x0300, 0x0100, CRC(08609bad) SHA1(e5daee3c3fea6620e3c2b91becd93bc4d3cdf011) )    /* sprite lookup table */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "tc2a_4e.bin",  0x0000, 0x0100, CRC(2c43991f) SHA1(312112832bee511b0545524295aa9bc2e756db0f) )    /* sprite palette bank */
ROM_END

ROM_START( terracrea ) // older pcb, the only difference is another sound rom
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 68000 code (main CPU) */
	ROM_LOAD16_BYTE( "1a_4b.rom",    0x00001, 0x4000, CRC(76f17479) SHA1(e6be7f78fe7dc9d66feb3ada6ad08d461c66640d) )
	ROM_LOAD16_BYTE( "1a_4d.rom",    0x00000, 0x4000, CRC(8119f06e) SHA1(314e2d8e75f66862cf6567ac05f417a3a66f1254) )
	ROM_LOAD16_BYTE( "1a_6b.rom",    0x08001, 0x4000, CRC(ba4b5822) SHA1(0de3ce04e14aa5757936babdec9cd1341d4a06d6) )
	ROM_LOAD16_BYTE( "1a_6d.rom",    0x08000, 0x4000, CRC(ca4852f6) SHA1(12e968efb890ff4f982c2e04e090ac4339a97fc0) )
	ROM_LOAD16_BYTE( "1a_7b.rom",    0x10001, 0x4000, CRC(d0771bba) SHA1(ebbc24562d677488a536cb515d761f07cd50425c) )
	ROM_LOAD16_BYTE( "1a_7d.rom",    0x10000, 0x4000, CRC(029d59d9) SHA1(51053cafd5e7a4a5ba7008c6c6b28c612d935f40) )
	ROM_LOAD16_BYTE( "1a_9b.rom",    0x18001, 0x4000, CRC(69227b56) SHA1(58c8aa4baa1f5ddfc151f5ed6284a06e87866dd7) )
	ROM_LOAD16_BYTE( "1a_9d.rom",    0x18000, 0x4000, CRC(5a672942) SHA1(3890f87edb9047f3e4c6f4d4b47b7f9873962148) )

	ROM_REGION( 0x10000, "audiocpu", 0 )/* Z80 code (sound) */
	ROM_LOAD( "2a_15b.rom",   0x0000, 0x4000, CRC(604c3b11) SHA1(c01d1ddae40fa8b65dfc72f959942cb9664a548b) )
	ROM_LOAD( "dg.12",        0x4000, 0x4000, CRC(9e9b3808) SHA1(7b6f8d2b75f063aa81711a7c2bf1563cc38eee8b) )
	ROM_LOAD( "2a_18b.rom",   0x8000, 0x4000, CRC(302dc0ab) SHA1(4db8f12e70f9adf1eb993c6a8af68b5edbf79773) )

	ROM_REGION( 0x02000, "gfx1", 0 )    /* tiles */
	ROM_LOAD( "2a_16b.rom",   0x00000, 0x2000, CRC(591a3804) SHA1(e1b46f5652e7f9677d75f01c6132975ace4facdd) )

	ROM_REGION( 0x10000, "gfx2", 0 )    /* background */
	ROM_LOAD( "1a_15f.rom",   0x00000, 0x8000, CRC(984a597f) SHA1(1f33892f160691c44872b37f0f6cb1493c9f7fb1) )
	ROM_LOAD( "1a_17f.rom",   0x08000, 0x8000, CRC(30e297ff) SHA1(9843826ae63039d6693c8a0b30af721d70f40056) )

	ROM_REGION( 0x10000, "gfx3", 0 )    /* sprites */
	ROM_LOAD( "2a_6e.rom",    0x00000, 0x4000, CRC(bcf7740b) SHA1(8701862c35eb8fb1ec239253136a3858ebea4d0c) )
	ROM_LOAD( "2a_7e.rom",    0x04000, 0x4000, CRC(a70b565c) SHA1(153e5f5a9927c294660dd0d636a9f651d4984d6d) )
	ROM_LOAD( "2a_6g.rom",    0x08000, 0x4000, CRC(4a9ec3e6) SHA1(0a35b82fb49ecf7edafd02744a48490e744c0a00) )
	ROM_LOAD( "2a_7g.rom",    0x0c000, 0x4000, CRC(450749fc) SHA1(376ab98ab8db56ed45f7d97a221dfd52e389cb5a) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "tc1a_10f.bin", 0x0000, 0x0100, CRC(ce07c544) SHA1(c3691cb420c88f1887a55e3035b5d017decbc17a) )    /* red component */
	ROM_LOAD( "tc1a_11f.bin", 0x0100, 0x0100, CRC(566d323a) SHA1(fe83585a0d9c7f942a5e54620b627a5a17a0fcf4) )    /* green component */
	ROM_LOAD( "tc1a_12f.bin", 0x0200, 0x0100, CRC(7ea63946) SHA1(d7b89694a80736c7605b5c83d25d8b706f4504ab) )    /* blue component */
	ROM_LOAD( "tc2a_2g.bin",  0x0300, 0x0100, CRC(08609bad) SHA1(e5daee3c3fea6620e3c2b91becd93bc4d3cdf011) )    /* sprite lookup table */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "tc2a_4e.bin",  0x0000, 0x0100, CRC(2c43991f) SHA1(312112832bee511b0545524295aa9bc2e756db0f) )    /* sprite palette bank */
ROM_END

ROM_START( terracren ) /* 'n' for OPN(YM2203), older than YM3526 sets */
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 68000 code (main CPU) */
	ROM_LOAD16_BYTE( "1a_4b.rom",    0x00001, 0x4000, CRC(76f17479) SHA1(e6be7f78fe7dc9d66feb3ada6ad08d461c66640d) )
	ROM_LOAD16_BYTE( "1a_4d.rom",    0x00000, 0x4000, CRC(8119f06e) SHA1(314e2d8e75f66862cf6567ac05f417a3a66f1254) )
	ROM_LOAD16_BYTE( "1a_6b.rom",    0x08001, 0x4000, CRC(ba4b5822) SHA1(0de3ce04e14aa5757936babdec9cd1341d4a06d6) )
	ROM_LOAD16_BYTE( "1a_6d.rom",    0x08000, 0x4000, CRC(ca4852f6) SHA1(12e968efb890ff4f982c2e04e090ac4339a97fc0) )
	ROM_LOAD16_BYTE( "1a_7b.rom",    0x10001, 0x4000, CRC(d0771bba) SHA1(ebbc24562d677488a536cb515d761f07cd50425c) )
	ROM_LOAD16_BYTE( "1a_7d.rom",    0x10000, 0x4000, CRC(029d59d9) SHA1(51053cafd5e7a4a5ba7008c6c6b28c612d935f40) )
	ROM_LOAD16_BYTE( "1a_9b.rom",    0x18001, 0x4000, CRC(69227b56) SHA1(58c8aa4baa1f5ddfc151f5ed6284a06e87866dd7) )
	ROM_LOAD16_BYTE( "1a_9d.rom",    0x18000, 0x4000, CRC(5a672942) SHA1(3890f87edb9047f3e4c6f4d4b47b7f9873962148) )

	ROM_REGION( 0x10000, "audiocpu", 0 )/* Z80 code (sound) */
	ROM_LOAD( "tc2a_15b.bin", 0x0000, 0x4000, CRC(790ddfa9) SHA1(90aa25fbfc9b5f52145ab3cf126610cf21024c20) )
	ROM_LOAD( "tc2a_17b.bin", 0x4000, 0x4000, CRC(d4531113) SHA1(efc37c33a0791cae4d4ab50bc884cd6c8a6f95f5) )

	ROM_REGION( 0x02000, "gfx1", 0 )    /* tiles */
	ROM_LOAD( "2a_16b.rom",   0x00000, 0x2000, CRC(591a3804) SHA1(e1b46f5652e7f9677d75f01c6132975ace4facdd) )

	ROM_REGION( 0x10000, "gfx2", 0 )    /* background */
	ROM_LOAD( "1a_15f.rom",   0x00000, 0x8000, CRC(984a597f) SHA1(1f33892f160691c44872b37f0f6cb1493c9f7fb1) )
	ROM_LOAD( "1a_17f.rom",   0x08000, 0x8000, CRC(30e297ff) SHA1(9843826ae63039d6693c8a0b30af721d70f40056) )

	ROM_REGION( 0x10000, "gfx3", 0 )    /* sprites */
	ROM_LOAD( "2a_6e.rom",    0x00000, 0x4000, CRC(bcf7740b) SHA1(8701862c35eb8fb1ec239253136a3858ebea4d0c) )
	ROM_LOAD( "2a_7e.rom",    0x04000, 0x4000, CRC(a70b565c) SHA1(153e5f5a9927c294660dd0d636a9f651d4984d6d) )
	ROM_LOAD( "2a_6g.rom",    0x08000, 0x4000, CRC(4a9ec3e6) SHA1(0a35b82fb49ecf7edafd02744a48490e744c0a00) )
	ROM_LOAD( "2a_7g.rom",    0x0c000, 0x4000, CRC(450749fc) SHA1(376ab98ab8db56ed45f7d97a221dfd52e389cb5a) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "tc1a_10f.bin", 0x0000, 0x0100, CRC(ce07c544) SHA1(c3691cb420c88f1887a55e3035b5d017decbc17a) )    /* red component */
	ROM_LOAD( "tc1a_11f.bin", 0x0100, 0x0100, CRC(566d323a) SHA1(fe83585a0d9c7f942a5e54620b627a5a17a0fcf4) )    /* green component */
	ROM_LOAD( "tc1a_12f.bin", 0x0200, 0x0100, CRC(7ea63946) SHA1(d7b89694a80736c7605b5c83d25d8b706f4504ab) )    /* blue component */
	ROM_LOAD( "tc2a_2g.bin",  0x0300, 0x0100, CRC(08609bad) SHA1(e5daee3c3fea6620e3c2b91becd93bc4d3cdf011) )    /* sprite lookup table */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "tc2a_4e.bin",  0x0000, 0x0100, CRC(2c43991f) SHA1(312112832bee511b0545524295aa9bc2e756db0f) )    /* sprite palette bank */
ROM_END

ROM_START( amazon )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 68000 code (main CPU) */
	ROM_LOAD16_BYTE( "11.4d",   0x00000, 0x8000,CRC(6c7f85c5) SHA1(7f78cf16a93ea1f7b8616122127327a7d337e565) )
	ROM_LOAD16_BYTE( "9.4b",    0x00001, 0x8000,CRC(e1b7a989) SHA1(ae49cbc6fc4bc151990caed1f57cc7e10aba7901) )
	ROM_LOAD16_BYTE( "12.6d",   0x10000, 0x8000,CRC(4de8a3ee) SHA1(a650d5b6ebff257f08db01e76e5c11c1ecc8cd36) )
	ROM_LOAD16_BYTE( "10.6b",   0x10001, 0x8000,CRC(d86bad81) SHA1(8f2e56422f9e604232c60f676dcd964392ec9d28) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code (sound) */
	ROM_LOAD( "1.15b",  0x00000, 0x4000, CRC(55a8b5e7) SHA1(d3f5609c1b97a54c505d12dd22f7454c88c90fb9) )
	ROM_LOAD( "2.17b",  0x04000, 0x4000, CRC(427a7cca) SHA1(b30e3dd4f685c4095478972d38bb9616369a51bf) )
	ROM_LOAD( "3.18b",  0x08000, 0x4000, CRC(b8cceaf7) SHA1(5682df3193ad1dfef366353921bfa7af08aec055) )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* alphanumerics */
	ROM_LOAD( "8.16g",  0x0000, 0x2000, CRC(0cec8644) SHA1(a8f2a67b2243771e20ba51a539a83f0017dff6bc) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "13.15f", 0x00000, 0x8000, CRC(415ff4d9) SHA1(b5d17bfbf78e620c29f8933f06bd88cc89825b4b) )
	ROM_LOAD( "14.17f", 0x08000, 0x8000, CRC(492b5c48) SHA1(822d9098427650d55cdd6e4a7e540147198d72fc) )
	ROM_LOAD( "15.18f", 0x10000, 0x8000, CRC(b1ac0b9d) SHA1(1b413823669db24a5cfb93f66f54bd08c410a0d4) )

	ROM_REGION( 0x10000, "gfx3", 0 ) /* sprites */
	ROM_LOAD( "4.6e",   0x0000, 0x4000, CRC(f77ced7a) SHA1(55bf0a0799d85877a71d1529073b0ed847c80e54) )
	ROM_LOAD( "5.7e",   0x4000, 0x4000, CRC(16ef1465) SHA1(7547f24fec79f774e1be441c3734bdcc03b5d313) )
	ROM_LOAD( "6.6g",   0x8000, 0x4000, CRC(936ec941) SHA1(b4891e207d66f8b77c237fc23ffa48f87ab6993a) )
	ROM_LOAD( "7.7g",   0xc000, 0x4000, CRC(66dd718e) SHA1(80990c6199f63b215e1dead3b09cf6160dd75333) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "clr.10f", 0x000, 0x100, CRC(6440b341) SHA1(ccf89ac889f1653100f5b0a042dcd826c4ac628b) ) /* red */
	ROM_LOAD( "clr.11f", 0x100, 0x100, CRC(271e947f) SHA1(3d1f44fe92cc5fdff001ef80e07aa46a1ca68fe5) ) /* green */
	ROM_LOAD( "clr.12f", 0x200, 0x100, CRC(7d38621b) SHA1(7904c3c2c32006d4f4608b6ee84d44ecd601de73) ) /* blue */
	ROM_LOAD( "2g",      0x300, 0x100, CRC(44ca16b9) SHA1(1893f24d1c7f4d8e24b5484b19f9284f2ec9be08) ) /* clut */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "4e",      0x000, 0x100, CRC(035f2c7b) SHA1(36e32a50146631e763711b586936b2815600f52d) ) /* ctable */

	ROM_REGION( 0x2000, "user2", 0 ) /* unknown, mostly text */
	ROM_LOAD( "16.18g", 0x0000, 0x2000, CRC(1d8d592b) SHA1(be8d6df8b5926069ae2cbc1dc26e1fa92d63f297) )
ROM_END

ROM_START( amatelas )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 68000 code (main CPU) */
	ROM_LOAD16_BYTE( "a11.4d",  0x00000, 0x8000,CRC(3d226d0b) SHA1(b3f36973117dcca7ec6f891503ada1055694059d) )
	ROM_LOAD16_BYTE( "a9.4b",   0x00001, 0x8000,CRC(e2a0d21d) SHA1(abb65ea5a10719d27f711216b7e387f2e01bdd5d) )
	ROM_LOAD16_BYTE( "a12.6d",  0x10000, 0x8000,CRC(e6607c51) SHA1(7679f84ccdf75226bb46a5357a460aa2d5e5cd32) )
	ROM_LOAD16_BYTE( "a10.6b",  0x10001, 0x8000,CRC(dbc1f1b4) SHA1(0fca999356e38d69ba5822c4ec489ea08f1d771f) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code (sound) */
	ROM_LOAD( "1.15b",  0x00000, 0x4000, CRC(55a8b5e7) SHA1(d3f5609c1b97a54c505d12dd22f7454c88c90fb9) )
	ROM_LOAD( "2.17b",  0x04000, 0x4000, CRC(427a7cca) SHA1(b30e3dd4f685c4095478972d38bb9616369a51bf) )
	ROM_LOAD( "3.18b",  0x08000, 0x4000, CRC(b8cceaf7) SHA1(5682df3193ad1dfef366353921bfa7af08aec055) )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* alphanumerics */
	ROM_LOAD( "a8.16g", 0x0000, 0x2000, CRC(aeba2102) SHA1(fb4d047a78cd47f628fedfda8349dc46cf6a6f32) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "13.15f", 0x00000, 0x8000, CRC(415ff4d9) SHA1(b5d17bfbf78e620c29f8933f06bd88cc89825b4b) )
	ROM_LOAD( "14.17f", 0x08000, 0x8000, CRC(492b5c48) SHA1(822d9098427650d55cdd6e4a7e540147198d72fc) )
	ROM_LOAD( "15.18f", 0x10000, 0x8000, CRC(b1ac0b9d) SHA1(1b413823669db24a5cfb93f66f54bd08c410a0d4) )

	ROM_REGION( 0x10000, "gfx3", 0 ) /* sprites */
	ROM_LOAD( "4.6e",   0x0000, 0x4000, CRC(f77ced7a) SHA1(55bf0a0799d85877a71d1529073b0ed847c80e54) )
	ROM_LOAD( "a5.7e",  0x4000, 0x4000, CRC(5fbf9a16) SHA1(d33a020626db8267fd1c1eacbff15c569d7fb72d) )
	ROM_LOAD( "6.6g",   0x8000, 0x4000, CRC(936ec941) SHA1(b4891e207d66f8b77c237fc23ffa48f87ab6993a) )
	ROM_LOAD( "7.7g",   0xc000, 0x4000, CRC(66dd718e) SHA1(80990c6199f63b215e1dead3b09cf6160dd75333) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "clr.10f", 0x000, 0x100, CRC(6440b341) SHA1(ccf89ac889f1653100f5b0a042dcd826c4ac628b) ) /* red */
	ROM_LOAD( "clr.11f", 0x100, 0x100, CRC(271e947f) SHA1(3d1f44fe92cc5fdff001ef80e07aa46a1ca68fe5) ) /* green */
	ROM_LOAD( "clr.12f", 0x200, 0x100, CRC(7d38621b) SHA1(7904c3c2c32006d4f4608b6ee84d44ecd601de73) ) /* blue */
	ROM_LOAD( "2g",      0x300, 0x100, CRC(44ca16b9) SHA1(1893f24d1c7f4d8e24b5484b19f9284f2ec9be08) ) /* clut */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "4e",      0x000, 0x100, CRC(035f2c7b) SHA1(36e32a50146631e763711b586936b2815600f52d) ) /* ctable */

	ROM_REGION( 0x2000, "user2", 0 ) /* unknown, mostly text */
	ROM_LOAD( "16.18g", 0x0000, 0x2000, CRC(1d8d592b) SHA1(be8d6df8b5926069ae2cbc1dc26e1fa92d63f297) )
ROM_END

ROM_START( horekid )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 68000 code (main CPU) */
	ROM_LOAD16_BYTE( "horekid.03",  0x00000, 0x8000, CRC(90ec840f) SHA1(7d04936c50c9ae02ea7dc24f8336997592def867) )
	ROM_LOAD16_BYTE( "horekid.01",  0x00001, 0x8000, CRC(a282faf8) SHA1(4c1ff36cf324dff9ddfc1035db6c52838c7be975) )
	ROM_LOAD16_BYTE( "horekid.04",  0x10000, 0x8000, CRC(375c0c50) SHA1(ee040dbdfe6673cf48f143518458609b21b4e15d) )
	ROM_LOAD16_BYTE( "horekid.02",  0x10001, 0x8000, CRC(ee7d52bb) SHA1(b9083f672a6bc37ec2bbb9af081e6f27b712b663) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code (sound) */
	ROM_LOAD( "horekid.09", 0x0000, 0x4000,CRC(49cd3b81) SHA1(284d75f6f6121d0581bb62f13ee02c85c3d972d2) )
	ROM_LOAD( "horekid.10", 0x4000, 0x4000,CRC(c1eaa938) SHA1(839f03e701f072a6441ee4980eb1961859c40d97) )
	ROM_LOAD( "horekid.11", 0x8000, 0x4000,CRC(0a2bc702) SHA1(0cef9e9022a27d30d2f83a16a55d8ede0ab686f4) )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* alphanumerics */
	ROM_LOAD( "horekid.16", 0x0000, 0x2000, CRC(104b77cc) SHA1(f875c7fe4f2b540bc44fa144a449a01268011431) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "horekid.05", 0x00000, 0x8000, CRC(da25ae10) SHA1(83d8b78cff85854b497b40525ec3c93a84ba6248) )
	ROM_LOAD( "horekid.06", 0x08000, 0x8000, CRC(616e4321) SHA1(5bf0e0a7290b6bcb5dfbb1070eeb683830e6916b) )
	ROM_LOAD( "horekid.07", 0x10000, 0x8000, CRC(8c7d2be2) SHA1(efd70997126fc7c2622546fabe69cb222dca87f9) )
	ROM_LOAD( "horekid.08", 0x18000, 0x8000, CRC(a0066b02) SHA1(d6437932028e937dab5728f40d6d09b6afe9a903) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* sprites */
	ROM_LOAD( "horekid.12", 0x00000, 0x8000, CRC(a3caa07a) SHA1(4baa7d1867dbaa8bace43416040114129f5405d6) )
	ROM_LOAD( "horekid.13", 0x08000, 0x8000, CRC(0e48ff8e) SHA1(5a3025991378ed3f9bdc2d420b1432332278178b) )
	ROM_LOAD( "horekid.14", 0x10000, 0x8000, CRC(e300747a) SHA1(5875a46c215b12f1e9a889819215bca40e4459a6) )
	ROM_LOAD( "horekid.15", 0x18000, 0x8000, CRC(51105741) SHA1(01c3bb2c03ce1ca959d62d64be3a019e74f677ba) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "kid_prom.10f", 0x000, 0x100, CRC(ca13ce23) SHA1(46f0ed22f601721fa35bab12ce8816f30b102f59) ) /* red */
	ROM_LOAD( "kid_prom.11f", 0x100, 0x100, CRC(fb44285a) SHA1(f9605e82f63188daeff044fd48d81c1dfc4d4f2a) ) /* green */
	ROM_LOAD( "kid_prom.12f", 0x200, 0x100, CRC(40d41237) SHA1(b33082540d739a3bfe096f68f3359fbf1360b5be) ) /* blue */
	ROM_LOAD( "kid_prom.2g",  0x300, 0x100, CRC(4b9be0ed) SHA1(81aa7bb24fe6ea13f5dffdb67ea699adf0b3129a) ) /* clut */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "kid_prom.4e",  0x000, 0x100, CRC(e4fb54ee) SHA1(aba89d347b24dc6680e6f25b4a6c0d6657bb6a83) ) /* ctable */

	ROM_REGION( 0x2000, "user2", 0 ) /* unknown, mostly text */
	ROM_LOAD( "horekid.17", 0x0000, 0x2000, CRC(1d8d592b) SHA1(be8d6df8b5926069ae2cbc1dc26e1fa92d63f297) )
ROM_END

ROM_START( horekidb )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 68000 code (main CPU) */
	ROM_LOAD16_BYTE( "knhhd5", 0x00000, 0x8000, CRC(786619c7) SHA1(6b4a659839a7c19370a81f9f9b26e4fe0d210d7b) )
	ROM_LOAD16_BYTE( "knhhd7", 0x00001, 0x8000, CRC(3bbb475b) SHA1(575cdc4f902f15335579c0f860fa75e33a0ea539) )
	ROM_LOAD16_BYTE( "horekid.04",  0x10000, 0x8000, CRC(375c0c50) SHA1(ee040dbdfe6673cf48f143518458609b21b4e15d) )
	ROM_LOAD16_BYTE( "horekid.02",  0x10001, 0x8000, CRC(ee7d52bb) SHA1(b9083f672a6bc37ec2bbb9af081e6f27b712b663) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code (sound) */
	ROM_LOAD( "horekid.09", 0x0000, 0x4000,CRC(49cd3b81) SHA1(284d75f6f6121d0581bb62f13ee02c85c3d972d2) )
	ROM_LOAD( "horekid.10", 0x4000, 0x4000,CRC(c1eaa938) SHA1(839f03e701f072a6441ee4980eb1961859c40d97) )
	ROM_LOAD( "horekid.11", 0x8000, 0x4000,CRC(0a2bc702) SHA1(0cef9e9022a27d30d2f83a16a55d8ede0ab686f4) )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* alphanumerics */
	ROM_LOAD( "horekid.16", 0x0000, 0x2000, CRC(104b77cc) SHA1(f875c7fe4f2b540bc44fa144a449a01268011431) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "horekid.05", 0x00000, 0x8000, CRC(da25ae10) SHA1(83d8b78cff85854b497b40525ec3c93a84ba6248) )
	ROM_LOAD( "horekid.06", 0x08000, 0x8000, CRC(616e4321) SHA1(5bf0e0a7290b6bcb5dfbb1070eeb683830e6916b) )
	ROM_LOAD( "horekid.07", 0x10000, 0x8000, CRC(8c7d2be2) SHA1(efd70997126fc7c2622546fabe69cb222dca87f9) )
	ROM_LOAD( "horekid.08", 0x18000, 0x8000, CRC(a0066b02) SHA1(d6437932028e937dab5728f40d6d09b6afe9a903) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* sprites */
	ROM_LOAD( "horekid.12", 0x00000, 0x8000, CRC(a3caa07a) SHA1(4baa7d1867dbaa8bace43416040114129f5405d6) )
	ROM_LOAD( "horekid.13", 0x08000, 0x8000, CRC(0e48ff8e) SHA1(5a3025991378ed3f9bdc2d420b1432332278178b) )
	ROM_LOAD( "horekid.14", 0x10000, 0x8000, CRC(e300747a) SHA1(5875a46c215b12f1e9a889819215bca40e4459a6) )
	ROM_LOAD( "horekid.15", 0x18000, 0x8000, CRC(51105741) SHA1(01c3bb2c03ce1ca959d62d64be3a019e74f677ba) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "kid_prom.10f", 0x000, 0x100, CRC(ca13ce23) SHA1(46f0ed22f601721fa35bab12ce8816f30b102f59) ) /* red */
	ROM_LOAD( "kid_prom.11f", 0x100, 0x100, CRC(fb44285a) SHA1(f9605e82f63188daeff044fd48d81c1dfc4d4f2a) ) /* green */
	ROM_LOAD( "kid_prom.12f", 0x200, 0x100, CRC(40d41237) SHA1(b33082540d739a3bfe096f68f3359fbf1360b5be) ) /* blue */
	ROM_LOAD( "kid_prom.2g",  0x300, 0x100, CRC(4b9be0ed) SHA1(81aa7bb24fe6ea13f5dffdb67ea699adf0b3129a) ) /* clut */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "kid_prom.4e",  0x000, 0x100, CRC(e4fb54ee) SHA1(aba89d347b24dc6680e6f25b4a6c0d6657bb6a83) ) /* ctable */

	ROM_REGION( 0x2000, "user2", 0 ) /* unknown, mostly text */
	ROM_LOAD( "horekid.17", 0x0000, 0x2000, CRC(1d8d592b) SHA1(be8d6df8b5926069ae2cbc1dc26e1fa92d63f297) )
ROM_END

/* This is not the REAL Booby Kids (early Japanese version of Kid no Hore Hore Daisakusen),
  it is a bootleg that was manufactureed in Italy which became popular in Europe.  The bootleggers
  probably called it 'Booby Kids' because this is the name under which the game was known on home systems
  at the time.  This is actually just a graphic hack of the other bootleg (horekidb) it is supported because
  it is a common PCB, and we need to clarify that this is not the real thing.
*/
ROM_START( boobhack )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 68000 code (main CPU) */
	ROM_LOAD16_BYTE( "1-c.bin", 0x00000, 0x8000, CRC(786619c7) SHA1(6b4a659839a7c19370a81f9f9b26e4fe0d210d7b) )
	ROM_LOAD16_BYTE( "1-b.bin", 0x00001, 0x8000, CRC(3bbb475b) SHA1(575cdc4f902f15335579c0f860fa75e33a0ea539) )
	ROM_LOAD16_BYTE( "1-d.bin", 0x10000, 0x8000, CRC(375c0c50) SHA1(ee040dbdfe6673cf48f143518458609b21b4e15d) )
	ROM_LOAD16_BYTE( "1-a.bin", 0x10001, 0x8000, CRC(ee7d52bb) SHA1(b9083f672a6bc37ec2bbb9af081e6f27b712b663) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code (sound) */
	ROM_LOAD( "1-i.bin",    0x0000, 0x4000,CRC(49cd3b81) SHA1(284d75f6f6121d0581bb62f13ee02c85c3d972d2) )
	ROM_LOAD( "1-j.bin",    0x4000, 0x4000,CRC(c1eaa938) SHA1(839f03e701f072a6441ee4980eb1961859c40d97) )
	ROM_LOAD( "1-k.bin",    0x8000, 0x4000,CRC(0a2bc702) SHA1(0cef9e9022a27d30d2f83a16a55d8ede0ab686f4) )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* alphanumerics */
	ROM_LOAD( "1-p.bin",    0x0000, 0x2000, CRC(104b77cc) SHA1(f875c7fe4f2b540bc44fa144a449a01268011431) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "1-e.bin",    0x00000, 0x8000, CRC(da25ae10) SHA1(83d8b78cff85854b497b40525ec3c93a84ba6248) )
	ROM_LOAD( "1-f.bin",    0x08000, 0x8000, CRC(616e4321) SHA1(5bf0e0a7290b6bcb5dfbb1070eeb683830e6916b) )
	ROM_LOAD( "1-g.bin",    0x10000, 0x8000, CRC(8c7d2be2) SHA1(efd70997126fc7c2622546fabe69cb222dca87f9) )
	ROM_LOAD( "1-h.bin",    0x18000, 0x8000, CRC(a0066b02) SHA1(d6437932028e937dab5728f40d6d09b6afe9a903) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* sprites */
	ROM_LOAD( "1-l.bin",    0x00000, 0x8000, CRC(a3caa07a) SHA1(4baa7d1867dbaa8bace43416040114129f5405d6) )
	ROM_LOAD( "1-m.bin",    0x08000, 0x8000, CRC(15b6cbdf) SHA1(b7f2a527946bcbd51aeae98b8971f4fbabcb3d14) ) // Booby Kids gfx hack changes these 2 roms ONLY
	ROM_LOAD( "1-n.bin",    0x10000, 0x8000, CRC(e300747a) SHA1(5875a46c215b12f1e9a889819215bca40e4459a6) )
	ROM_LOAD( "1-o.bin",    0x18000, 0x8000, CRC(cddc6a6c) SHA1(28d12342e0ada941f68845fa65793a3f5fa21246) ) // Booby Kids gfx hack changes these 2 roms ONLY

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "kid_prom.10f", 0x000, 0x100, CRC(ca13ce23) SHA1(46f0ed22f601721fa35bab12ce8816f30b102f59) ) /* red */
	ROM_LOAD( "kid_prom.11f", 0x100, 0x100, CRC(fb44285a) SHA1(f9605e82f63188daeff044fd48d81c1dfc4d4f2a) ) /* green */
	ROM_LOAD( "kid_prom.12f", 0x200, 0x100, CRC(40d41237) SHA1(b33082540d739a3bfe096f68f3359fbf1360b5be) ) /* blue */
	ROM_LOAD( "kid_prom.2g",  0x300, 0x100, CRC(4b9be0ed) SHA1(81aa7bb24fe6ea13f5dffdb67ea699adf0b3129a) ) /* clut */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "kid_prom.4e",  0x000, 0x100, BAD_DUMP CRC(e4fb54ee) SHA1(aba89d347b24dc6680e6f25b4a6c0d6657bb6a83) ) /* ctable */
ROM_END

DRIVER_INIT_MEMBER(terracre_state,amazon)
{
	m_mpProtData = mAmazonProtData;
}

DRIVER_INIT_MEMBER(terracre_state,amatelas)
{
	m_mpProtData = mAmatelasProtData;
}

DRIVER_INIT_MEMBER(terracre_state,horekid)
{
	m_mpProtData = mHoreKidProtData;
}

/*    YEAR, NAME,   PARENT,     MACHINE, INPUT,    INIT,     MONITOR,  COMPANY,      FULLNAME, FLAGS */
GAME( 1985, terracre, 0,        ym3526,  terracre, driver_device, 0,        ROT270,  "Nichibutsu", "Terra Cresta (YM3526 set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, terracreo,terracre, ym3526,  terracre, driver_device, 0,        ROT270,  "Nichibutsu", "Terra Cresta (YM3526 set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, terracrea,terracre, ym3526,  terracre, driver_device, 0,        ROT270,  "Nichibutsu", "Terra Cresta (YM3526 set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, terracren,terracre, ym2203,  terracre, driver_device, 0,        ROT270,  "Nichibutsu", "Terra Cresta (YM2203)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, amazon,   0,        amazon,  amazon, terracre_state,   amazon,   ROT270,  "Nichibutsu", "Soldier Girl Amazon", MACHINE_SUPPORTS_SAVE )
GAME( 1986, amatelas, amazon,   amazon,  amazon, terracre_state,   amatelas, ROT270,  "Nichibutsu", "Sei Senshi Amatelass", MACHINE_SUPPORTS_SAVE )
GAME( 1987, horekid,  0,        amazon,  horekid, terracre_state,  horekid,  ROT270,  "Nichibutsu", "Kid no Hore Hore Daisakusen", MACHINE_SUPPORTS_SAVE )
GAME( 1987, horekidb, horekid,  amazon,  horekid, terracre_state,  horekid,  ROT270,  "bootleg", "Kid no Hore Hore Daisakusen (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, boobhack, horekid,  amazon,  horekid, terracre_state,  horekid,  ROT270,  "bootleg", "Booby Kids (Italian manufactured graphic hack / bootleg of Kid no Hore Hore Daisakusen (bootleg))", MACHINE_SUPPORTS_SAVE )

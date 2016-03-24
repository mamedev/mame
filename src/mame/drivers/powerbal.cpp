// license:BSD-3-Clause
// copyright-holders:David Haywood, Pierpaolo Prazzoli
/*

Power Balls  (c) 1994 Playmark
Magic Sticks (c) 1995 Playmark

driver by David Haywood & Pierpaolo Prazzoli

TODO:

Magic Sticks:
 - ticket dispenser
 - other inputs ?

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "includes/playmark.h"

class powerbal_state : public playmark_state
{
public:
	powerbal_state(const machine_config &mconfig, device_type type, const char *tag)
		: playmark_state(mconfig, type, tag),
			m_eeprom(*this, "eeprom") { }

	/* powerbal-specific */
	int         m_tilebank;
	int         m_bg_yoffset;

	optional_device<eeprom_serial_93cxx_device> m_eeprom;

	DECLARE_DRIVER_INIT(powerbal);
	DECLARE_DRIVER_INIT(magicstk);
	TILE_GET_INFO_MEMBER(powerbal_get_bg_tile_info);
	DECLARE_MACHINE_START(powerbal);
	DECLARE_MACHINE_RESET(powerbal);
	DECLARE_VIDEO_START(powerbal);
	UINT32 screen_update_powerbal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites_powerbal( bitmap_ind16 &bitmap, const rectangle &cliprect );
	DECLARE_WRITE16_MEMBER(magicstk_coin_eeprom_w);
	DECLARE_WRITE16_MEMBER(magicstk_bgvideoram_w);
	DECLARE_WRITE16_MEMBER(tile_banking_w);
	DECLARE_WRITE16_MEMBER(oki_banking);
};


WRITE16_MEMBER(powerbal_state::magicstk_coin_eeprom_w)
{
	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_counter_w(0, data & 0x20);

		m_eeprom->cs_write((data & 8) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->di_write((data & 2) >> 1);
		m_eeprom->clk_write((data & 4) ? CLEAR_LINE : ASSERT_LINE);
	}
}

WRITE16_MEMBER(powerbal_state::magicstk_bgvideoram_w)
{
	COMBINE_DATA(&m_videoram1[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(powerbal_state::tile_banking_w)
{
	if (((data >> 12) & 0x0f) != m_tilebank)
	{
		m_tilebank = (data >> 12) & 0x0f;
		m_bg_tilemap->mark_all_dirty();
	}
}

WRITE16_MEMBER(powerbal_state::oki_banking)
{
	if (data & 3)
	{
		int addr = 0x40000 * ((data & 3) - 1);

		if (addr < memregion("oki")->bytes())
			m_oki->set_bank_base(addr);
	}
}

static ADDRESS_MAP_START( magicstk_main_map, AS_PROGRAM, 16, powerbal_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x088000, 0x0883ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x094000, 0x094001) AM_WRITENOP
	AM_RANGE(0x094002, 0x094003) AM_WRITENOP
	AM_RANGE(0x094004, 0x094005) AM_WRITE(tile_banking_w)
	AM_RANGE(0x098180, 0x09917f) AM_RAM_WRITE(magicstk_bgvideoram_w) AM_SHARE("videoram1")
	AM_RANGE(0x0c2010, 0x0c2011) AM_READ_PORT("IN0")
	AM_RANGE(0x0c2012, 0x0c2013) AM_READ_PORT("IN1")
	AM_RANGE(0x0c2014, 0x0c2015) AM_READ_PORT("IN2") AM_WRITE(magicstk_coin_eeprom_w)
	AM_RANGE(0x0c2016, 0x0c2017) AM_READ_PORT("DSW1")
	AM_RANGE(0x0c2018, 0x0c2019) AM_READ_PORT("DSW2")
	AM_RANGE(0x0c201c, 0x0c201d) AM_WRITE(oki_banking)
	AM_RANGE(0x0c201e, 0x0c201f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x0c4000, 0x0c4001) AM_WRITENOP
	AM_RANGE(0x0e0000, 0x0fffff) AM_RAM
	AM_RANGE(0x100000, 0x100fff) AM_RAM AM_SHARE("spriteram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( powerbal_main_map, AS_PROGRAM, 16, powerbal_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x088000, 0x0883ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x094000, 0x094001) AM_WRITENOP
	AM_RANGE(0x094002, 0x094003) AM_WRITENOP
	AM_RANGE(0x094004, 0x094005) AM_WRITE(tile_banking_w)
	AM_RANGE(0x098000, 0x098fff) AM_RAM_WRITE(magicstk_bgvideoram_w) AM_SHARE("videoram1")
	AM_RANGE(0x099000, 0x09bfff) AM_RAM // not used
	AM_RANGE(0x0c2010, 0x0c2011) AM_READ_PORT("IN0")
	AM_RANGE(0x0c2012, 0x0c2013) AM_READ_PORT("IN1")
	AM_RANGE(0x0c2014, 0x0c2015) AM_READ_PORT("IN2")
	AM_RANGE(0x0c2016, 0x0c2017) AM_READ_PORT("DSW1")
	AM_RANGE(0x0c2018, 0x0c2019) AM_READ_PORT("DSW2")
	AM_RANGE(0x0c201c, 0x0c201d) AM_WRITE(oki_banking)
	AM_RANGE(0x0c201e, 0x0c201f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x0c4000, 0x0c4001) AM_WRITENOP
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM
	AM_RANGE(0x101000, 0x101fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x102000, 0x10200d) AM_WRITENOP // not used scroll regs?
	AM_RANGE(0x103000, 0x103fff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( powerbal )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x08, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Italian ) )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( magicstk )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)   /* EEPROM data */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Coin Mode" )
	PORT_DIPSETTING(    0x01, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x1e, 0x1e, "Coinage Mode 1" ) PORT_CONDITION("DSW1",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x14, DEF_STR( 6C_1C ) ) PORT_CONDITION("DSW1",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x16, DEF_STR( 5C_1C ) ) PORT_CONDITION("DSW1",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x18, DEF_STR( 4C_1C ) ) PORT_CONDITION("DSW1",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x1a, DEF_STR( 3C_1C ) ) PORT_CONDITION("DSW1",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x02, DEF_STR( 8C_3C ) ) PORT_CONDITION("DSW1",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x1c, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW1",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_3C ) ) PORT_CONDITION("DSW1",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) ) PORT_CONDITION("DSW1",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x1e, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW1",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) ) PORT_CONDITION("DSW1",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSW1",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSW1",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_4C ) ) PORT_CONDITION("DSW1",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) ) PORT_CONDITION("DSW1",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) ) PORT_CONDITION("DSW1",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) ) PORT_CONDITION("DSW1",0x01,EQUALS,0x01)
	PORT_DIPNAME( 0x06, 0x06, "Coin A Mode 2" ) PORT_CONDITION("DSW1",0x01,NOTEQUALS,0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) ) PORT_CONDITION("DSW1",0x01,NOTEQUALS,0x01)
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) ) PORT_CONDITION("DSW1",0x01,NOTEQUALS,0x01)
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW1",0x01,NOTEQUALS,0x01)
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW1",0x01,NOTEQUALS,0x01)
	PORT_DIPNAME( 0x18, 0x18, "Coin B Mode 2" ) PORT_CONDITION("DSW1",0x01,NOTEQUALS,0x01)
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSW1",0x01,NOTEQUALS,0x01)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSW1",0x01,NOTEQUALS,0x01)
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) ) PORT_CONDITION("DSW1",0x01,NOTEQUALS,0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) ) PORT_CONDITION("DSW1",0x01,NOTEQUALS,0x01)
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Clear Counters" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Ticket" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Hopper" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0xe0, 0x60, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xa0, "Hard 7" )
	PORT_DIPSETTING(    0x20, "Very Hard 6" )
	PORT_DIPSETTING(    0xc0, "Very Hard 5" )
//  PORT_DIPSETTING(    0x80, "Very Hard 4" )
//  PORT_DIPSETTING(    0x40, "Very Hard 4" )
	PORT_DIPSETTING(    0x00, "Very Hard 4" )
	PORT_DIPSETTING(    0x60, "Normal 8" )
	PORT_DIPSETTING(    0xe0, "Easy 9" )
INPUT_PORTS_END

static INPUT_PORTS_START( hotminda )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)   /* EEPROM data */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Premio" )            PORT_DIPLOCATION("DSW1:7,6")
	PORT_DIPSETTING(    0x00, "1 Premio ogni 10 Vincite" )
	PORT_DIPSETTING(    0x04, "1 Premio ogni 10 Vincite" )
	PORT_DIPSETTING(    0x02, "1 Premio ogni 5 Vincite" )
	PORT_DIPSETTING(    0x06, "Paga 1 Premio ogni Vincita" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("DSW1:5,4")
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Clear Counters" )    PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Ticket" )            PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Hopper" )            PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 PORT_DIPLOCATION("DSW2:4")
	PORT_DIPNAME( 0xe0, 0x60, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("DSW2:3,2,1")
	PORT_DIPSETTING(    0xa0, "Hard 7" )
	PORT_DIPSETTING(    0x20, "Very Hard 6" )
	PORT_DIPSETTING(    0xc0, "Very Hard 5" )
	PORT_DIPSETTING(    0x40, "Very Hard 4" )
	PORT_DIPSETTING(    0x80, "Very Hard 3" )
	PORT_DIPSETTING(    0x00, "Very Hard 2" )
	PORT_DIPSETTING(    0x60, "Normal 8" )
	PORT_DIPSETTING(    0xe0, "Easy 9" )
INPUT_PORTS_END

TILE_GET_INFO_MEMBER(powerbal_state::powerbal_get_bg_tile_info)
{
	int code = (m_videoram1[tile_index] & 0x07ff) + m_tilebank * 0x800;
	int colr = m_videoram1[tile_index] & 0xf000;

	if (m_videoram1[tile_index] & 0x800)
		code |= 0x8000;

	SET_TILE_INFO_MEMBER(1, code, colr >> 12, 0);
}

void powerbal_state::draw_sprites_powerbal(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT16 *spriteram = m_spriteram;
	int offs;
	int height = m_gfxdecode->gfx(0)->height();

	for (offs = 4; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		int sx, sy, code, color, flipx;

		sy = spriteram[offs + 3 - 4];   /* typical Playmark style... */
		if (sy & 0x8000)
			return; /* end of list marker */

		flipx = sy & 0x4000;
		sx = (spriteram[offs + 1] & 0x01ff) - 16 - 7;
		sy = (256 - 8 - height - sy) & 0xff;
		code = spriteram[offs + 2];
		color = (spriteram[offs + 1] & 0xf000) >> 12;

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,0,
				sx + m_xoffset,sy + m_yoffset,0);
	}
}

VIDEO_START_MEMBER(powerbal_state,powerbal)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(powerbal_state::powerbal_get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_xoffset = -20;

	m_bg_tilemap->set_scrolly(0, m_bg_yoffset);
}

UINT32 powerbal_state::screen_update_powerbal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites_powerbal(bitmap, cliprect);
	return 0;
}

static const gfx_layout magicstk_charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};



static GFXDECODE_START( powerbal )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,          0x100, 16 )    /* colors 0x100-0x1ff */
	GFXDECODE_ENTRY( "gfx1", 0, magicstk_charlayout, 0x000, 16 )    /* colors 0x000-0x0ff */
GFXDECODE_END



MACHINE_START_MEMBER(powerbal_state,powerbal)
{
	save_item(NAME(m_tilebank));
}

MACHINE_RESET_MEMBER(powerbal_state,powerbal)
{
	m_tilebank = 0;
}

static MACHINE_CONFIG_START( powerbal, powerbal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)   /* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(powerbal_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", powerbal_state, irq2_line_hold)

	MCFG_MACHINE_START_OVERRIDE(powerbal_state,powerbal)
	MCFG_MACHINE_RESET_OVERRIDE(powerbal_state,powerbal)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(61)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(128*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(powerbal_state, screen_update_powerbal)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", powerbal)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(powerbal_state,powerbal)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( magicstk, powerbal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)   /* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(magicstk_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", powerbal_state, irq2_line_hold)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	MCFG_EEPROM_SERIAL_DEFAULT_VALUE(0)

	MCFG_MACHINE_START_OVERRIDE(powerbal_state,powerbal)
	MCFG_MACHINE_RESET_OVERRIDE(powerbal_state,powerbal)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(61)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(128*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(powerbal_state, screen_update_powerbal)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", powerbal)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(powerbal_state,powerbal)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/*
Power Balls
Playmark, 1994

PCB Layout
----------

1-6-93
LC BMJ
|------------------------------------------------------------|
|     1                        6116                          |
|      M6295  1MHz                                     4     |
| VOL                          6116                          |
|                                                      5     |
|                                                            |
|                                      |--------|      6     |
|      2018                            |TPC1020 |            |
|                                      |        |      7     |
|J     2018                            |        |            |
|A                                     |--------|      8     |
|M                                                           |
|M         GAL16V8                                     9     |
|A                                                           |
|                                                      10    |
|                                                            |
|                                                      11    |
|  DIPSW                          GAL22V10                   |
|                                                            |
|                                 2018                       |
|  DIPSW                                                     |
|                                 2018              2018     |
|        62256     62256                                     |
|12MHz     2         3          GAL16V8             2018     |
|                                                            |
|            68000          26MHz                            |
|------------------------------------------------------------|
Notes:
      HSync - 15.45kHz
      VSync - 61Hz
      68000 clock - 12MHz
      6295 clock - 1.000MHz, sample rate = 1000000 / 132
      2018 - 2k x8 SRAM
      6116 - 2k x8 SRAM
      62256 - 32k x8 SRAM

      ROMs - 2, 3 - 27C020
             1, 4-11 - 27C040
*/

ROM_START( powerbal )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "3.u67",  0x00000, 0x40000, CRC(3aecdde4) SHA1(e78373246d55f120e8d94f4606da874df439b823) )
	ROM_LOAD16_BYTE( "2.u66",  0x00001, 0x40000, CRC(a4552a19) SHA1(88b84daa1fd36d5c683cf0d6dce341aedbc360d1) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "4.u38",        0x000000, 0x80000, CRC(a60aa981) SHA1(46a5d2d2a353a45127a03a104e877ffd150daa92) )
	ROM_LOAD( "5.u42",        0x080000, 0x80000, CRC(966c71df) SHA1(daf4bcf3d2ef10ea9a5e2e7ea71b3783b9f5b1f0) )
	ROM_LOAD( "6.u39",        0x100000, 0x80000, CRC(668957b9) SHA1(31fc9328ff6044e17834b6d61a886a8ef2e6570c) )
	ROM_LOAD( "7.u45",        0x180000, 0x80000, CRC(f5721c66) SHA1(1e8b3a8e82da60378dad7727af21157c4059b071) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "8.u86",        0x000000, 0x80000, CRC(4130694c) SHA1(581d0035ce1624568f635bd79290be6c587a2533) )
	ROM_LOAD( "9.u85",        0x080000, 0x80000, CRC(e7bcd2e7) SHA1(01a5e5ac5da2fd79a0c9088f775096b9915bae92) )
	ROM_LOAD( "10.u84",       0x100000, 0x80000, CRC(90412135) SHA1(499619c72613a1dd63a6504e39b159a18a71f4fa) )
	ROM_LOAD( "11.u83",       0x180000, 0x80000, CRC(92d7d40a) SHA1(81879945790feb9aeb45750e9b5ded3356571503) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is the area that gets switched */
	ROM_REGION( 0xc0000, "oki", 0 ) /* OKI Samples */
	ROM_LOAD( "1.u16",        0x00000, 0x40000, CRC(12776dbc) SHA1(9ab9930fd581296642834d2cb4ba65264a588af3) )
	ROM_CONTINUE(             0x60000, 0x20000 )
	ROM_CONTINUE(             0xa0000, 0x20000 )
	ROM_COPY( "oki",  0x000000, 0x40000, 0x20000)
	ROM_COPY( "oki",  0x000000, 0x80000, 0x20000)

	ROM_REGION( 0x1200, "plds", 0 )
	ROM_LOAD( "palce16v8h.u102",  0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "palce22v10h.u183", 0x0200, 0x02dd, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "palce22v10h.u211", 0x0600, 0x02dd, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "palce22v10h.bin",  0x0a00, 0x02dd, NO_DUMP ) /* PAL is soldered */
	ROM_LOAD( "pal22v10a.bin",    0x0e00, 0x02dd, NO_DUMP ) /* PAL is soldered */
ROM_END

ROM_START( magicstk )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "12.u67", 0x00000, 0x20000, CRC(70a9c66f) SHA1(0cf4b2d0f796e35881d68adc69eca4360d6ad693) )
	ROM_LOAD16_BYTE( "11.u66", 0x00001, 0x20000, CRC(a9d7c90e) SHA1(e12517776dc14747b4bbe49f93c4d7e83e8eae01) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "13.u36",       0x00000, 0x20000, CRC(31e52562) SHA1(18ee5ba990d97690ece81e4066a9f0395ddc6f3e) )
	ROM_LOAD( "14.u42",       0x20000, 0x20000, CRC(b0d35eda) SHA1(a85d45d3b4fbacecf5aa2af9a18ba0ac9f1f9a26) )
	ROM_LOAD( "15.u39",       0x40000, 0x20000, CRC(af27004b) SHA1(b022020e6bd6fc9ec95f23b6a37911df0768856e) )
	ROM_LOAD( "16.u45",       0x60000, 0x20000, CRC(0c980db3) SHA1(212129bf86cdc73752be184e579299e03ba6862e) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "17.u86",       0x00000, 0x20000, CRC(ce238006) SHA1(3425a8125d56139fe5d220b0d9d5c9a4af1f4d58) )
	ROM_LOAD( "18.u85",       0x20000, 0x20000, CRC(3dc88bf6) SHA1(f9c04bca32bae4aa6df38635d73c6a4b8742fbd3) )
	ROM_LOAD( "19.u84",       0x40000, 0x20000, CRC(ee12d5b2) SHA1(872edff5a35d2725e3dd752a5f609aca995bfeff) )
	ROM_LOAD( "20.u83",       0x60000, 0x20000, CRC(a07f542b) SHA1(0c17629142a90687460b4c951f2062f5c7de8921) )

	ROM_REGION( 0x40000, "oki", 0 ) /* OKI Samples */
	ROM_LOAD( "10.u16",       0x00000, 0x20000, CRC(1e4a03ef) SHA1(6a134daa9a6d8dbda51cab348627f078c3dde8c7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "palce16v8.u33", 0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "palce16v8.u58", 0x0200, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal22v10b.bin", 0x0400, 0x02e5, NO_DUMP ) /* GAL is soldered */
ROM_END


ROM_START( hotminda )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "rom1.rom",       0x00001, 0x20000, CRC(33aaceba) SHA1(a914400b081eabd869f1ca2c843a91b03af510b1) )
	ROM_LOAD16_BYTE( "rom2.rom",       0x00000, 0x20000, CRC(f5accd9f) SHA1(12194ea7c35263be9afd91f0abe2041998528af9) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "rom13.rom",       0x00000, 0x20000, CRC(18d22109) SHA1(52bbb68f4ef5f4d41f5915bef4304784451ca6d8) )
	ROM_LOAD( "rom14.rom",       0x20000, 0x20000, CRC(f95a1ff6) SHA1(646c59199570ccd11cb53b0b59a6cd03b1b42fac) )
	ROM_LOAD( "rom15.rom",       0x40000, 0x20000, CRC(8a9ea7ed) SHA1(529c0466df3f0aa050526699099ea7a5da9dbcfe) )
	ROM_LOAD( "rom16.rom",       0x60000, 0x20000, CRC(df63b642) SHA1(d5df740717193b06267508d169bb5df6214ca13d))

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "rom17.rom",       0x00000, 0x20000, CRC(805002cf) SHA1(dc97881bc78dcb753f404b7df2cfd4a071ca8393) )
	ROM_LOAD( "rom18.rom",       0x20000, 0x20000, CRC(6a9d896b) SHA1(d617a69e6954de3bf7c322529232eadb90034fbc) )
	ROM_LOAD( "rom19.rom",       0x40000, 0x20000, CRC(223ad90f) SHA1(57b4e364f21aeea24a99deb6bab13019846e8f9b) )
	ROM_LOAD( "rom20.rom",       0x60000, 0x20000, CRC(ab37a273) SHA1(2051ee99a7ff3f4fc2b91c2c9d4e4da2f12db256) )

	ROM_REGION( 0x40000, "oki", 0 ) /* OKI Samples */
	ROM_LOAD( "rom10.rom",       0x00000, 0x40000,  CRC(0bf3a3e5) SHA1(2ae06f37a6bcd20bc5fbaa90d970aba2ebf3cf5a) )
ROM_END

DRIVER_INIT_MEMBER(powerbal_state,powerbal)
{
	m_bg_yoffset = 16;
	m_yoffset = -8;
}

DRIVER_INIT_MEMBER(powerbal_state,magicstk)
{
	m_bg_yoffset = 0;
	m_yoffset = -5;
}

/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT   MACHINE   INPUT     INIT      ROT    COMPANY     FULLNAME                      FLAGS */
GAME( 1994, powerbal, 0,       powerbal, powerbal, powerbal_state, powerbal, ROT0, "Playmark", "Power Balls",                 MACHINE_SUPPORTS_SAVE )
GAME( 1995, magicstk, 0,       magicstk, magicstk, powerbal_state, magicstk, ROT0, "Playmark", "Magic Sticks",                MACHINE_SUPPORTS_SAVE )
GAME( 1995, hotminda, hotmind, magicstk, hotminda, powerbal_state, magicstk, ROT0, "Playmark", "Hot Mind (adjustable prize)", MACHINE_SUPPORTS_SAVE )

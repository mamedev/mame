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
#include "playmark.h"

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class powerbal_state : public playmark_base_state
{
public:
	powerbal_state(const machine_config &mconfig, device_type type, const char *tag)
		: playmark_base_state(mconfig, type, tag)
	{ }

	void init_powerbal();

	void powerbal(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	u8 m_tilebank = 0;
	s8 m_bg_yoffset = 0;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void bgvideoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tile_banking_w(u16 data);
	void oki_banking(u16 data);

	void oki_map(address_map &map) ATTR_COLD;

private:
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};

class atombjt_state : public powerbal_state
{
public:
	atombjt_state(const machine_config &mconfig, device_type type, const char *tag)
		: powerbal_state(mconfig, type, tag)
	{ }

	void atombjt(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void tile_banking_w(u16 data);

	void main_map(address_map &map) ATTR_COLD;
};

class magicstk_state : public powerbal_state
{
public:
	magicstk_state(const machine_config &mconfig, device_type type, const char *tag)
		: powerbal_state(mconfig, type, tag),
		m_eeprom(*this, "eeprom"),
		m_ticket(*this, "ticket"),
		m_token(*this, "token")
	{ }

	void init_magicstk();

	void magicstk(machine_config &config);

private:
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<ticket_dispenser_device> m_ticket;
	required_device<ticket_dispenser_device> m_token;

	void coin_eeprom_w(u8 data);

	void main_map(address_map &map) ATTR_COLD;
};

void magicstk_state::coin_eeprom_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x20);

	m_eeprom->cs_write((data & 8) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write((data & 2) >> 1);
	m_eeprom->clk_write((data & 4) ? CLEAR_LINE : ASSERT_LINE);
}

void powerbal_state::bgvideoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bgvideoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

void powerbal_state::tile_banking_w(u16 data)
{
	if (((data >> 12) & 0x0f) != m_tilebank)
	{
		m_tilebank = (data >> 12) & 0x0f;
		m_bg_tilemap->mark_all_dirty();
	}
}

void atombjt_state::tile_banking_w(u16 data)
{
	if ((data & 0x0f) != m_tilebank)
	{
		m_tilebank = data & 0x0f;
		m_bg_tilemap->mark_all_dirty();
	}
}

void powerbal_state::oki_banking(u16 data)
{
	int bank = data & 3;
	m_okibank->set_entry(bank & (m_oki_numbanks - 1));
}

void magicstk_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x088000, 0x0883ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x094000, 0x094001).nopw();
	map(0x094002, 0x094003).nopw();
	map(0x094004, 0x094005).w(FUNC(magicstk_state::tile_banking_w));
	map(0x098180, 0x09917f).ram().w(FUNC(magicstk_state::bgvideoram_w)).share(m_bgvideoram);
	map(0x0c2010, 0x0c2011).portr("IN0");
	map(0x0c2012, 0x0c2013).portr("IN1");
	map(0x0c2014, 0x0c2015).portr("IN2");
	map(0x0c2015, 0x0c2015).w(FUNC(magicstk_state::coin_eeprom_w));
	map(0x0c2016, 0x0c2017).portr("DSW1");
	map(0x0c2018, 0x0c2019).portr("DSW2");
	map(0x0c201c, 0x0c201d).w(FUNC(magicstk_state::oki_banking));
	map(0x0c201f, 0x0c201f).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x0c4000, 0x0c4001).nopw();
	map(0x0e0000, 0x0fffff).ram();
	map(0x100000, 0x100fff).ram().share(m_spriteram);
}

void powerbal_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x088000, 0x0883ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x094000, 0x094001).nopw();
	map(0x094002, 0x094003).nopw();
	map(0x094004, 0x094005).w(FUNC(powerbal_state::tile_banking_w));
	map(0x098000, 0x098fff).ram().w(FUNC(powerbal_state::bgvideoram_w)).share(m_bgvideoram);
	map(0x099000, 0x09bfff).ram(); // not used
	map(0x0c2010, 0x0c2011).portr("IN0");
	map(0x0c2012, 0x0c2013).portr("IN1");
	map(0x0c2014, 0x0c2015).portr("IN2");
	map(0x0c2016, 0x0c2017).portr("DSW1");
	map(0x0c2018, 0x0c2019).portr("DSW2");
	map(0x0c201c, 0x0c201d).w(FUNC(powerbal_state::oki_banking));
	map(0x0c201f, 0x0c201f).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x0c4000, 0x0c4001).nopw();
	map(0x0f0000, 0x0fffff).ram();
	map(0x101000, 0x101fff).ram().share(m_spriteram);
	map(0x102000, 0x10200d).nopw(); // not used scroll regs?
	map(0x103000, 0x103fff).ram();
}

void atombjt_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080008, 0x080009).nopr(); // remnant of the original?
	map(0x080014, 0x080015).noprw(); // always 1 in this bootleg. Flip-screen switch not present according to dip sheet.
	map(0x088000, 0x0883ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x094000, 0x094001).w(FUNC(atombjt_state::tile_banking_w));
	map(0x094002, 0x094003).noprw();    // IRQ enable?
	map(0x09c000, 0x09cfff).mirror(0x1000).ram().w(FUNC(atombjt_state::bgvideoram_w)).share(m_bgvideoram);
	map(0x0c2010, 0x0c2011).portr("IN0");
	map(0x0c2012, 0x0c2013).portr("IN1");
	map(0x0c2014, 0x0c2015).portr("IN2");
	map(0x0c2016, 0x0c2017).portr("DSW1");
	map(0x0c2018, 0x0c2019).portr("DSW2");
	map(0x0c201c, 0x0c201d).w(FUNC(atombjt_state::oki_banking));
	map(0x0c201f, 0x0c201f).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x0c4000, 0x0c4001).nopw(); // always 0?
	map(0x0f0000, 0x0fffff).ram();
	map(0x100000, 0x100fff).ram();
	map(0x101000, 0x101fff).ram().share(m_spriteram);
}

void powerbal_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr(m_okibank);
}

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
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) // Manual shows this as "Weapon"  Off for Yes and On for No - Meaning is unknown
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)   // EEPROM data
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
	PORT_DIPNAME( 0x1e, 0x1e, "Coinage Mode 1" ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x14, DEF_STR( 6C_1C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x16, DEF_STR( 5C_1C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x18, DEF_STR( 4C_1C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x1a, DEF_STR( 3C_1C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x02, DEF_STR( 8C_3C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x1c, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_3C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x1e, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_4C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPNAME( 0x06, 0x06, "Coin A Mode 2" ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) ) PORT_CONDITION("DSW1" ,0x01, NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)
	PORT_DIPNAME( 0x18, 0x18, "Coin B Mode 2" ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)
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
	PORT_DIPSETTING(    0x80, "Very Hard 4" )
	PORT_DIPSETTING(    0x40, "Very Hard 4" )
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)   // EEPROM data
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

static INPUT_PORTS_START( atombjt ) // verified with dip sheet
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    // shown in service mode, but no effect
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    // Maybe unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    // Maybe unused

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0e, "Starting level" )        PORT_DIPLOCATION("SW1:7,6,5")
	PORT_DIPSETTING(    0x08, "Germany" )
	PORT_DIPSETTING(    0x04, "Thailand" )
	PORT_DIPSETTING(    0x0c, "Nevada" )
	PORT_DIPSETTING(    0x0e, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Korea ) )
	PORT_DIPSETTING(    0x0a, "England" )
	PORT_DIPSETTING(    0x02, DEF_STR( Hong_Kong ) )
	PORT_DIPSETTING(    0x00, DEF_STR( China ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:6,5,4")
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END


TILE_GET_INFO_MEMBER(powerbal_state::get_bg_tile_info)
{
	int code = (m_bgvideoram[tile_index] & 0x07ff) + m_tilebank * 0x800;
	int colr = m_bgvideoram[tile_index] & 0xf000;

	if (m_bgvideoram[tile_index] & 0x800)
		code |= 0x8000;

	tileinfo.set(1, code, colr >> 12, 0);
}

void powerbal_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int height = m_gfxdecode->gfx(0)->height();

	for (int offs = 4; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		int sy = m_spriteram[offs + 3 - 4];   // typical Playmark style...
		if (sy & 0x8000)
			return; // end of list marker

		int flipx = sy & 0x4000;
		int sx = (m_spriteram[offs + 1] & 0x01ff) - 16 - 7;
		sy = (256 - 8 - height - sy) & 0xff;
		int code = m_spriteram[offs + 2];
		int color = (m_spriteram[offs + 1] & 0xf000) >> 12;

		m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
				code,
				color,
				flipx,0,
				sx + m_xoffset, sy + m_yoffset, m_sprtranspen);
	}
}

void powerbal_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(powerbal_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_xoffset = -20;

	m_bg_tilemap->set_scrolly(0, m_bg_yoffset);
}

void atombjt_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(atombjt_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 64, 32);

	m_xoffset = 0x23;
	m_yoffset = 0x09;
	m_sprtranspen = 0xf;

	m_bg_tilemap->set_scrollx(0, -64);
}

u32 powerbal_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}

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



static GFXDECODE_START( gfx_powerbal )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,          0x100, 16 )    // colors 0x100-0x1ff
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_planar,    0x000, 16 )    // colors 0x000-0x0ff
GFXDECODE_END



void powerbal_state::machine_start()
{
	save_item(NAME(m_tilebank));
}

void powerbal_state::machine_reset()
{
	m_tilebank = 0;
	configure_oki_banks();
}

void powerbal_state::powerbal(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &powerbal_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(powerbal_state::irq2_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(61);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(128*8, 64*8);
	screen.set_visarea(0*8, 40*8-1, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(powerbal_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_powerbal);
	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 512);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 1_MHz_XTAL, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
	m_oki->set_addrmap(0, &powerbal_state::oki_map);
}

void magicstk_state::magicstk(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &magicstk_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(magicstk_state::irq2_line_hold));

	EEPROM_93C46_16BIT(config, "eeprom").default_value(0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(61);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(128*8, 64*8);
	screen.set_visarea(0*8, 40*8-1, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(magicstk_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_powerbal);
	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 512);

	TICKET_DISPENSER(config, m_ticket, attotime::from_msec(350));
	TICKET_DISPENSER(config, m_token,  attotime::from_msec(350));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 1_MHz_XTAL, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
	m_oki->set_addrmap(0, &magicstk_state::oki_map);
}

void atombjt_state::atombjt(machine_config &config)
{
	powerbal(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &atombjt_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(atombjt_state::irq6_line_hold));

	subdevice<screen_device>("screen")->set_size(512, 256);
	subdevice<screen_device>("screen")->set_visarea(0*8, 48*8-1, 2*8, 30*8-1);
}

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
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
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

	// $00000-$20000 stays the same in all sound banks, the second half of the bank is the area that gets switched
	ROM_REGION( 0x80000, "oki", 0 ) // OKI Samples
	ROM_LOAD( "1.u16",        0x00000, 0x80000, CRC(12776dbc) SHA1(9ab9930fd581296642834d2cb4ba65264a588af3) )

	ROM_REGION( 0x1200, "plds", 0 )
	ROM_LOAD( "palce16v8h.u102",  0x0000, 0x0117, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "palce22v10h.u183", 0x0200, 0x02dd, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "palce22v10h.u211", 0x0600, 0x02dd, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "palce22v10h.bin",  0x0a00, 0x02dd, NO_DUMP ) // PAL is soldered
	ROM_LOAD( "pal22v10a.bin",    0x0e00, 0x02dd, NO_DUMP ) // PAL is soldered
ROM_END

ROM_START( magicstk )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
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

	ROM_REGION( 0x40000, "oki", 0 ) // OKI Samples
	ROM_LOAD( "10.u16",       0x00000, 0x20000, CRC(1e4a03ef) SHA1(6a134daa9a6d8dbda51cab348627f078c3dde8c7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "palce16v8.u33", 0x0000, 0x0117, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "palce16v8.u58", 0x0200, 0x0117, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "gal22v10b.bin", 0x0400, 0x02e5, NO_DUMP ) // GAL is soldered
ROM_END


ROM_START( hotminda )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
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

	ROM_REGION( 0x40000, "oki", 0 ) // OKI Samples
	ROM_LOAD( "rom10.rom",       0x00000, 0x40000,  CRC(0bf3a3e5) SHA1(2ae06f37a6bcd20bc5fbaa90d970aba2ebf3cf5a) )
ROM_END

ROM_START( atombjt ) // based off bjtwina set
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "22.u67",  0x00000, 0x20000, CRC(bead8c70) SHA1(2694bb0639f6b94119c21faf3810f00ef20b50da) )
	ROM_LOAD16_BYTE( "21.u66",  0x00001, 0x20000, CRC(73e3d488) SHA1(7deed6e3aeda1902b75746a9b0a2737632425867) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "23.u36",  0x000000, 0x80000, CRC(a3fb6b91) SHA1(477f5722a6bb23f089f32b677efbf69e9dce4b74) )
	ROM_LOAD( "24.u42",  0x080000, 0x80000, CRC(4c30e15f) SHA1(f92185743594e4e4573ac3f6c0c091802a08d5bd) )
	ROM_LOAD( "25.u39",  0x100000, 0x80000, CRC(ff1af60f) SHA1(4fe626c9d59ab9b945535b2f796f13adc900f1ed) )
	ROM_LOAD( "26.u45",  0x180000, 0x80000, CRC(6cc4e817) SHA1(70f2ab50e228a029d3157c94fe0a79e7aad010bd) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "27.u86",  0x000000, 0x40000, CRC(5a853e5c) SHA1(dfa4e891f716bbf8a038a14a24276cb690f65230) )
	ROM_LOAD( "28.u85",  0x040000, 0x40000, CRC(41970bf6) SHA1(85b5677585dbdf96acabb59e6369d62d4c2f0e8e) )
	ROM_LOAD( "29.u84",  0x080000, 0x40000, CRC(59a7d610) SHA1(0dc39c09f7f55dbd12ddb5e2e4ba9d86a2ba24d8) )
	ROM_LOAD( "30.u83",  0x0c0000, 0x40000, CRC(9b2dfebd) SHA1(562ab22dc01a129e1b8c201665bbab0561254c2a) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "20.u16",    0x00000, 0x80000, CRC(71c74ff9) SHA1(3c22fb2976ab332e9bb1e208432ca985f274adac) )
ROM_END


void powerbal_state::init_powerbal()
{
	m_bg_yoffset = 16;
	m_yoffset = -8;
}

void magicstk_state::init_magicstk()
{
	m_bg_yoffset = 0;
	m_yoffset = -5;
}

} // Anonymous namespace


/*************************
*      Game Drivers      *
*************************/

//    YEAR  NAME      PARENT   MACHINE   INPUT     STATE           INIT           ROT      COMPANY             FULLNAME                           FLAGS
GAME( 1994, powerbal, 0,       powerbal, powerbal, powerbal_state, init_powerbal, ROT0,   "Playmark",          "Power Balls",                     MACHINE_SUPPORTS_SAVE )
GAME( 1995, magicstk, 0,       magicstk, magicstk, magicstk_state, init_magicstk, ROT0,   "Playmark",          "Magic Sticks",                    MACHINE_SUPPORTS_SAVE )
GAME( 1995, hotminda, hotmind, magicstk, hotminda, magicstk_state, init_magicstk, ROT0,   "Playmark",          "Hot Mind (adjustable prize)",     MACHINE_SUPPORTS_SAVE )
GAME( 1993, atombjt,  bjtwin,  atombjt,  atombjt,  atombjt_state,  empty_init,    ROT270, "bootleg (Kyon K.)", "Atom (bootleg of Bombjack Twin)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // some non-trivial mods to the gfx and sound hw wrt nmk16 hw original

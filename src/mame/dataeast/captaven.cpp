// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Captain America and The Avengers

    Emulation by Bryan McPhail, mish@tendril.co.uk.  Thank you to Tim,
    Avedis and Stiletto for many things including work on Fighter's
    History protection and tracking down Tattoo Assassins!

    reset with both start buttons held down for test mode with
	version info.

***************************************************************************/

#include "emu.h"

#include "deco_irq.h"
#include "deco16ic.h"
#include "deco146.h"
#include "decocrpt.h"
#include "decospr.h"

#include "cpu/arm/arm.h"
#include "cpu/h6280/h6280.h"
#include "machine/input_merger.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>


namespace {

class captaven_state : public driver_device
{
public:
	captaven_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_sprgen(*this, "spritegen")
		, m_deco_tilegen(*this, "tilegen%u", 1)
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_deco_irq(*this, "irq")
		, m_ioprot(*this, "ioprot")
		, m_ym2151(*this, "ymsnd")
		, m_oki(*this, "oki%u", 1)
		, m_pf_rowscroll32(*this, "pf%u_rowscroll32", 1)
		, m_io_dsw(*this, "DSW%u", 1U)
	{ }

	void captaven(machine_config &config) ATTR_COLD;

	void init_captaven() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void sound_bankswitch_w(u8 data);

	u16 ioprot_r(offs_t offset);
	void ioprot_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u32 _71_r();
	u8 captaven_soundcpu_status_r();

	template<int Chip> void pf_rowscroll_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void buffer_spriteram_w(u32 data);
	void pri_w(u32 data);

	void tile_callback(u32 &tile, u32 &colour, int layer, bool is_8x8);
	DECO16IC_BANK_CB_MEMBER(bank_callback);
	DECOSPR_PRIORITY_CB_MEMBER(captaven_pri_callback);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<decospr_device> m_sprgen;
	required_device_array<deco16ic_device, 2> m_deco_tilegen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<deco_irq_device> m_deco_irq;
	required_device<deco146_device> m_ioprot;
	required_device<ym2151_device> m_ym2151;
	required_device_array<okim6295_device, 2> m_oki;

	// we use the pointers below to store a 32-bit copy..
	required_shared_ptr_array<u32, 4> m_pf_rowscroll32;

	required_ioport_array<3> m_io_dsw;

	u32 m_pri = 0;
	std::unique_ptr<u16[]> m_spriteram16;
	std::unique_ptr<u16[]> m_spriteram16_buffered;
	std::unique_ptr<u16[]> m_pf_rowscroll[4]{};
};


//**************************************************************************
//  PROTECTION
//**************************************************************************

u16 captaven_state::ioprot_r(offs_t offset)
{
	const offs_t real_address = 0 + (offset * 2);
	const offs_t deco146_addr = (BIT(real_address, 14, 4) << 11) | BIT(real_address, 0, 11); // NC 31-18, 13-11
	u8 cs = 0;

	return m_ioprot->read_data(deco146_addr, cs);
}

void captaven_state::ioprot_w(offs_t offset, u16 data, u16 mem_mask)
{
	const offs_t real_address = 0 + (offset * 2);
	const offs_t deco146_addr = (BIT(real_address, 14, 4) << 11) | BIT(real_address, 0, 11); // NC 31-18, 13-11
	u8 cs = 0;

	m_ioprot->write_data(deco146_addr, data, mem_mask, cs);
}


//**************************************************************************
//  SOUND
//**************************************************************************

u8 captaven_state::captaven_soundcpu_status_r()
{
	// 7-------  sound cpu status (0 = busy)
	// -6543210  unknown

	return 0xff;
}

void captaven_state::sound_bankswitch_w(u8 data)
{
	m_oki[0]->set_rom_bank((data >> 0) & 1);
	m_oki[1]->set_rom_bank((data >> 1) & 1);
}


//**************************************************************************
//  VIDEO
//**************************************************************************

void captaven_state::video_start()
{
	m_spriteram16 = std::make_unique<u16[]>(0x2000/4);
	m_spriteram16_buffered = std::make_unique<u16[]>(0x2000/4);

	for (int i = 0; i < 4; i++)
	{
		const u32 size = m_pf_rowscroll32[i].length();

		m_pf_rowscroll[i] = make_unique_clear<u16[]>(size);

		save_pointer(NAME(m_pf_rowscroll[i]), size, i);
	}
	save_pointer(NAME(m_spriteram16), 0x2000/4);
	save_pointer(NAME(m_spriteram16_buffered), 0x2000/4);
	save_item(NAME(m_pri));
}

void captaven_state::pri_w(u32 data)
{
	m_pri = data;
}

u32 captaven_state::_71_r()
{
	// Bit 0x80 goes high when sprite DMA is complete, and low
	// while it's in progress, we don't bother to emulate it
	return 0xffffffff;
}

u32 captaven_state::spriteram_r(offs_t offset)
{
	return m_spriteram16[offset] ^ 0xffff0000;
}

void captaven_state::spriteram_w(offs_t offset, u32 data, u32 mem_mask)
{
	data &= 0x0000ffff;
	mem_mask &= 0x0000ffff;
	COMBINE_DATA(&m_spriteram16[offset]);
}

void captaven_state::buffer_spriteram_w(u32 data)
{
	std::copy(&m_spriteram16[0], &m_spriteram16[0x2000/4], &m_spriteram16_buffered[0]);
}

// tattass tests these as 32-bit ram, even if only 16-bits are hooked up to the tilemap chip - does it mirror parts of the dword?
template <int TileMap> void captaven_state::pf_rowscroll_w(offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_pf_rowscroll32[TileMap][offset]); data &= 0x0000ffff; mem_mask &= 0x0000ffff; COMBINE_DATA(&m_pf_rowscroll[TileMap][offset]); }

DECOSPR_PRIORITY_CB_MEMBER(captaven_state::captaven_pri_callback)
{
	if ((pri & 0x60) == 0x00)
	{
		return 0; // above everything
	}
	else if ((pri & 0x60) == 0x20)
	{
		return 0xfff0; // above the 2nd playfield
	}
	else if ((pri & 0x60) == 0x40)
	{
		return 0xfffc; // above the 1st playfield
	}
	else
	{
		return 0xfffe; // under everything
	}
}

void captaven_state::tile_callback(u32 &tile, u32 &colour, int layer, bool is_8x8)
{
	// Captain America operates this chip in 8bpp mode.
	// In 8bpp mode you appear to only get 1 layer, not 2, but you also
	// have an extra 2 tile bits, and 2 less colour bits.
	if (layer == 0)
	{
		tile |= ((colour & 0x3) << 12);
		colour >>= 2;
	}
}

DECO16IC_BANK_CB_MEMBER(captaven_state::bank_callback)
{
	return (bank & 0x20) << 9;
}

u32 captaven_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u16 flip = m_deco_tilegen[0]->pf_control_r(0);
	flip_screen_set(BIT(flip, 7));
	m_sprgen->set_flip_screen(BIT(flip, 7));

	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->pen(0x000), cliprect); // Palette index not confirmed

	m_deco_tilegen[0]->pf_update(m_pf_rowscroll[0].get(), m_pf_rowscroll[1].get());
	m_deco_tilegen[1]->pf_update(m_pf_rowscroll[2].get(), m_pf_rowscroll[3].get());

	// pf4 not used (because pf3 is in 8bpp mode)

	if ((m_pri & 1) == 0)
	{
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 1);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
	}
	else
	{
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 1);
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
	}

	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);

	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram16_buffered.get(), 0x400); // only low half of sprite ram is used?

	return 0;
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void captaven_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x100007).r(FUNC(captaven_state::_71_r));
	map(0x100000, 0x100003).w(FUNC(captaven_state::buffer_spriteram_w));
	map(0x108000, 0x108003).nopw(); // ?
	map(0x110000, 0x111fff).rw(FUNC(captaven_state::spriteram_r), FUNC(captaven_state::spriteram_w));
	map(0x120000, 0x127fff).ram(); // Main RAM
	map(0x128000, 0x12ffff).rw(FUNC(captaven_state::ioprot_r), FUNC(captaven_state::ioprot_w)).umask32(0x0000ffff);
	map(0x130000, 0x131fff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x148000, 0x14800f).m(m_deco_irq, FUNC(deco_irq_device::map)).umask32(0x000000ff);
	map(0x160000, 0x167fff).ram(); // Extra work RAM
	map(0x168000, 0x168000).lr8(NAME([this] () { return m_io_dsw[0]->read(); }));
	map(0x168001, 0x168001).lr8(NAME([this] () { return m_io_dsw[1]->read(); }));
	map(0x168002, 0x168002).lr8(NAME([this] () { return m_io_dsw[2]->read(); }));
	map(0x168003, 0x168003).r(FUNC(captaven_state::captaven_soundcpu_status_r));
	map(0x178000, 0x178003).w(FUNC(captaven_state::pri_w));
	map(0x180000, 0x18001f).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	// Mirror address - bug in program code
	map(0x190000, 0x191fff).mirror(0x2000).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x194000, 0x195fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w));
	map(0x1a0000, 0x1a3fff).ram().w(FUNC(captaven_state::pf_rowscroll_w<0>)).share(m_pf_rowscroll32[0]);
	map(0x1a4000, 0x1a5fff).ram().w(FUNC(captaven_state::pf_rowscroll_w<1>)).share(m_pf_rowscroll32[1]);
	map(0x1c0000, 0x1c001f).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x1d0000, 0x1d1fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x1d4000, 0x1d5fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w)); // unused
	map(0x1e0000, 0x1e3fff).ram().w(FUNC(captaven_state::pf_rowscroll_w<2>)).share(m_pf_rowscroll32[2]);
	map(0x1e4000, 0x1e5fff).ram().w(FUNC(captaven_state::pf_rowscroll_w<3>)).share(m_pf_rowscroll32[3]); // unused
}

void captaven_state::sound_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x110000, 0x110001).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x120000, 0x120001).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x130000, 0x130001).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x140000, 0x140000).r(m_ioprot, FUNC(deco146_device::soundlatch_r));
	map(0x1f0000, 0x1f1fff).ram();
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

/* Notes (2002.02.05) :

When the "Continue Coin" Dip Switch is set to "2 Start/1 Continue",
the "Coinage" Dip Switches have no effect.

START, BUTTON1 and COIN effects :

  2 players, common coin slots

STARTn starts a game for player n. It adds 100 energy points each time it is pressed
(provided there are still some credits, and energy is <= 900).

BUTTON1n selects the character for player n.

COIN1n adds credit(s)/coin(s).

  2 players, individual coin slots

NO STARTn button !

BUTTON1n starts a game for player n. It also adds 100 energy points for each credit
inserted for the player. It then selects the character for player n.

COIN1n adds 100 energy points (based on "Coinage") for player n when ingame if energy
<= 900, else adds credit(s)/coin(s) for player n.

  4 players, common coin slots

NO STARTn button !

BUTTON1n starts a game for player n. It gives 100 energy points. It then selects the
character for player n.

  4 players, individual coin slots

NO STARTn button !

BUTTON1n starts a game for player n. It also adds 100 energy points for each credit
inserted for the player. It then selects the character for player n.

COIN1n adds 100 energy points (based on "Coinage") for player n when ingame if energy
<= 900, else adds credit(s)/coin(s) for player n.

*/

static INPUT_PORTS_START( captaven )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )         PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )         PORT_DIPLOCATION("DSW1:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )    PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Continue Coin" )           PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, "1 Start/1 Continue" )
	PORT_DIPSETTING(    0x00, "2 Start/1 Continue" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )          PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )     PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, "Coin Slots" )              PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, "Common" )
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPNAME( 0x20, 0x20, "Play Mode" )               PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, "2 Player" )
	PORT_DIPSETTING(    0x00, "4 Player" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )    PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	// This one isn't documented in the manual
	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, IP_ACTIVE_LOW, "DSW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, IP_ACTIVE_LOW, "DSW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, IP_ACTIVE_LOW, "DSW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, IP_ACTIVE_LOW, "DSW3:4")
	PORT_DIPNAME(   0x10, 0x10, "Reset")                  PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(      0x10, DEF_STR( Off ))
	PORT_DIPSETTING(      0x00, DEF_STR( On ))
	PORT_DIPNAME(   0x20, 0x20, DEF_STR( Free_Play ))     PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(      0x20, DEF_STR( Off ))
	PORT_DIPSETTING(      0x00, DEF_STR( On ))
	PORT_DIPNAME(   0x40, 0x40, "Stage Select")           PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(      0x40, DEF_STR( Off ))
	PORT_DIPSETTING(      0x00, DEF_STR( On ))
	PORT_DIPNAME(   0x80, 0x80, "Debug Mode")             PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(      0x80, DEF_STR( Off ))
	PORT_DIPSETTING(      0x00, DEF_STR( On ))
INPUT_PORTS_END


//**************************************************************************
//  GFXDECODE LAYOUTS
//**************************************************************************

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	16*8    // every char takes 8 consecutive bytes
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	64*8
};

static const gfx_layout tilelayout_8bpp =
{
	16,16,
	RGN_FRAC(1,4),
	8,
	{ RGN_FRAC(3,4)+8, RGN_FRAC(3,4)+0, RGN_FRAC(2,4)+8, RGN_FRAC(2,4)+0, RGN_FRAC(1,4)+8, RGN_FRAC(1,4)+0, 8, 0 },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	64*8
};

static GFXDECODE_START( gfx_captaven )
	GFXDECODE_ENTRY( "tiles1", 0, charlayout,      0, 128 ) // Characters 8x8
	GFXDECODE_ENTRY( "tiles1", 0, tilelayout,      0, 128 ) // Tiles 16x16
	GFXDECODE_ENTRY( "tiles2", 0, tilelayout_8bpp, 0,   8 ) // Tiles 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_captaven_spr )
	GFXDECODE_ENTRY( "sprites", 0, tilelayout,      0,  32 ) // Sprites 16x16
GFXDECODE_END


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void captaven_state::captaven(machine_config &config)
{
	// basic machine hardware
	ARM(config, m_maincpu, 28_MHz_XTAL / 4); // verified on pcb (Data East 101 custom)*/
	m_maincpu->set_addrmap(AS_PROGRAM, &captaven_state::main_map);

	h6280_device &audiocpu(H6280(config, m_audiocpu, 32.22_MHz_XTAL / 4 / 3)); // pin 10 is 32mhz/4, pin 14 is High so internal divisor is 3 (verified on pcb)
	audiocpu.set_addrmap(AS_PROGRAM, &captaven_state::sound_map);
	audiocpu.add_route(ALL_OUTPUTS, "speaker", 0, 0); // internal sound unused
	audiocpu.add_route(ALL_OUTPUTS, "speaker", 0, 1);

	INPUT_MERGER_ANY_HIGH(config, "irq_merger").output_handler().set_inputline(m_maincpu, ARM_IRQ_LINE);

	DECO_IRQ(config, m_deco_irq);
	m_deco_irq->set_screen_tag(m_screen);
	m_deco_irq->raster2_irq_callback().set("irq_merger", FUNC(input_merger_any_high_device::in_w<0>));
	m_deco_irq->vblank_irq_callback().set("irq_merger", FUNC(input_merger_any_high_device::in_w<1>));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(28_MHz_XTAL / 4, 442, 0, 320, 274, 8, 248);
	m_screen->set_screen_update(FUNC(captaven_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_captaven);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_888, 2048);

	DECO16IC(config, m_deco_tilegen[0]);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_col_bank(0x20);
	m_deco_tilegen[0]->set_pf2_col_bank(0x30);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1]);    // pf3 is in 8bpp mode, pf4 is not used
	m_deco_tilegen[1]->set_pf1_size(DECO_32x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_32x32);
	m_deco_tilegen[1]->set_pf1_col_bank(0x04);
	m_deco_tilegen[1]->set_pf2_col_bank(0x00);
	m_deco_tilegen[1]->set_pf1_col_mask(0x03);
	m_deco_tilegen[1]->set_pf2_col_mask(0x00);
	m_deco_tilegen[1]->set_tile_callback(FUNC(captaven_state::tile_callback));
	m_deco_tilegen[1]->set_bank1_callback(FUNC(captaven_state::bank_callback));
	// no bank2 callback
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_SPRITE(config, m_sprgen, m_palette, gfx_captaven_spr);
	m_sprgen->set_pri_callback(FUNC(captaven_state::captaven_pri_callback));
	m_sprgen->set_alt_format(true);

	DECO146PROT(config, m_ioprot);
	m_ioprot->port_a_cb().set_ioport("IN0");
	m_ioprot->port_b_cb().set_ioport("SYSTEM");
	m_ioprot->port_c_cb().set_ioport("IN1");
	m_ioprot->soundlatch_irq_cb().set_inputline(m_audiocpu, 0);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, m_ym2151, 32.22_MHz_XTAL / 9); // verified on pcb
	m_ym2151->irq_handler().set_inputline(m_audiocpu, 1);
	m_ym2151->port_write_handler().set(FUNC(captaven_state::sound_bankswitch_w));
	m_ym2151->add_route(0, "speaker", 0.42, 0);
	m_ym2151->add_route(1, "speaker", 0.42, 1);

	OKIM6295(config, m_oki[0], 32.22_MHz_XTAL / 32, okim6295_device::PIN7_HIGH); // verified on pcb; pin 7 is floating to 2.5V (left unconnected), so I presume High
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	OKIM6295(config, m_oki[1], 32.22_MHz_XTAL / 16, okim6295_device::PIN7_HIGH); // verified on pcb; pin 7 is floating to 2.5V (left unconnected), so I presume High
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.35, 0);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.35, 1);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( captaven ) // DE-0351-x PCB (x=3 or 4)
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "hn_00-4.1e",  0x000000, 0x20000, CRC(147fb094) SHA1(6bd759c42f4b7f9e1c3f2d3ece0b3ec72de1a982) )
	ROM_LOAD32_BYTE( "hn_01-4.1h",  0x000001, 0x20000, CRC(11ecdb95) SHA1(832b56f05ae7e15e67fbdd321da8c1cc5e7629a0) )
	ROM_LOAD32_BYTE( "hn_02-4.1k",  0x000002, 0x20000, CRC(35d2681f) SHA1(3af7d959dc4842238a7f79926adf449cb7f0b2e9) )
	ROM_LOAD32_BYTE( "hn_03-4.1m",  0x000003, 0x20000, CRC(3b59ba05) SHA1(400e868e59977e56a4fa1870321c643983ba4162) )
	ROM_LOAD32_BYTE( "man-12.3e",   0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",   0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",   0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",   0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "hj_08.17k",  0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "man-00.8a",  0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) // Encrypted tiles

	ROM_REGION( 0x500000, "tiles2", 0 )
	ROM_LOAD( "man-05.16a", 0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) // Encrypted tiles
	ROM_CONTINUE(           0x140000,  0x40000 )
	ROM_CONTINUE(           0x280000,  0x40000 )
	ROM_CONTINUE(           0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a", 0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) // Encrypted tiles
	ROM_CONTINUE(           0x180000,  0x40000 )
	ROM_CONTINUE(           0x2c0000,  0x40000 )
	ROM_CONTINUE(           0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a", 0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) // Encrypted tiles
	ROM_CONTINUE(           0x1c0000,  0x40000 )
	ROM_CONTINUE(           0x300000,  0x40000 )
	ROM_CONTINUE(           0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a", 0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) // Encrypted tiles
	ROM_CONTINUE(           0x200000,  0x40000 )
	ROM_CONTINUE(           0x340000,  0x40000 )
	ROM_CONTINUE(           0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a", 0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) // Encrypted tiles
	ROM_CONTINUE(           0x240000,  0x40000 )
	ROM_CONTINUE(           0x380000,  0x40000 )
	ROM_CONTINUE(           0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "sprites", 0 ) // Sprites
	ROM_LOAD( "man-06.17a",  0x200000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD( "man-07.18a",  0x000000,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD( "man-08.17c",  0x300000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD( "man-09.21c",  0x100000,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k", 0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k", 0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "ts-00.4h",  0x0000, 0x0117, CRC(ebc2908e) SHA1(dca14a55abd1d88ee09092d4122614e55c3e7f53) )
	ROM_LOAD( "ts-01.5h",  0x0200, 0x0117, CRC(c776a980) SHA1(cd4bdcfb755f561fefa4c88fab5d6d2397332aa7) )
	ROM_LOAD( "ts-02.12l", 0x0400, 0x01bf, CRC(6f26528c) SHA1(2cf869b2a789a9b0646162a61c147bcbb13c9141) )
ROM_END

ROM_START( captavena ) // DE-0351-x PCB (x=3 or 4)
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "hn_00.e1",    0x000000, 0x20000, CRC(12dd0c71) SHA1(77bd0e5f1b105ec70de5e76cb9c8138f02a496be) )
	ROM_LOAD32_BYTE( "hn_01.h1",    0x000001, 0x20000, CRC(ac5ea492) SHA1(e08fa2b3e3a40cba6dcdf07049d67056d59ed72a) )
	ROM_LOAD32_BYTE( "hn_02.k1",    0x000002, 0x20000, CRC(0c5e13f6) SHA1(d9ebf503db7da8663f45fe307e432545651cfc13) )
	ROM_LOAD32_BYTE( "hn_03.l1",    0x000003, 0x20000, CRC(bc050740) SHA1(bee425e76734251444c9cfa9287e1eb9383625bc) )
	ROM_LOAD32_BYTE( "man-12.3e",   0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",   0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",   0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",   0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "hj_08.17k",  0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "man-00.8a",  0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) // Encrypted tiles

	ROM_REGION( 0x500000, "tiles2", 0 )
	ROM_LOAD( "man-05.16a", 0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) // Encrypted tiles
	ROM_CONTINUE(           0x140000,  0x40000 )
	ROM_CONTINUE(           0x280000,  0x40000 )
	ROM_CONTINUE(           0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a", 0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) // Encrypted tiles
	ROM_CONTINUE(           0x180000,  0x40000 )
	ROM_CONTINUE(           0x2c0000,  0x40000 )
	ROM_CONTINUE(           0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a", 0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) // Encrypted tiles
	ROM_CONTINUE(           0x1c0000,  0x40000 )
	ROM_CONTINUE(           0x300000,  0x40000 )
	ROM_CONTINUE(           0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a", 0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) // Encrypted tiles
	ROM_CONTINUE(           0x200000,  0x40000 )
	ROM_CONTINUE(           0x340000,  0x40000 )
	ROM_CONTINUE(           0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a", 0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) // Encrypted tiles
	ROM_CONTINUE(           0x240000,  0x40000 )
	ROM_CONTINUE(           0x380000,  0x40000 )
	ROM_CONTINUE(           0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "sprites", 0 ) // Sprites
	ROM_LOAD( "man-06.17a",  0x200000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD( "man-07.18a",  0x000000,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD( "man-08.17c",  0x300000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD( "man-09.21c",  0x100000,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k", 0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k", 0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "ts-00.4h",  0x0000, 0x0117, CRC(ebc2908e) SHA1(dca14a55abd1d88ee09092d4122614e55c3e7f53) )
	ROM_LOAD( "ts-01.5h",  0x0200, 0x0117, CRC(c776a980) SHA1(cd4bdcfb755f561fefa4c88fab5d6d2397332aa7) )
	ROM_LOAD( "ts-02.12l", 0x0400, 0x01bf, CRC(6f26528c) SHA1(2cf869b2a789a9b0646162a61c147bcbb13c9141) )
ROM_END

ROM_START( captavene ) // DE-0351-x PCB (x=3 or 4)
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "hg_00-4.1e",  0x000000, 0x20000, CRC(7008d43c) SHA1(a39143e13075ebc58ecc576391f04d2649675dfb) )
	ROM_LOAD32_BYTE( "hg_01-4.1h",  0x000001, 0x20000, CRC(53dc1042) SHA1(4547ad20e5bc3b9cedae53f73f1628fa3493aafa) )
	ROM_LOAD32_BYTE( "hg_02-4.1k",  0x000002, 0x20000, CRC(9e3f9ee2) SHA1(a56a68bdac58a337be48b346b6939c3f68da8e9d) )
	ROM_LOAD32_BYTE( "hg_03-4.1m",  0x000003, 0x20000, CRC(bc050740) SHA1(bee425e76734251444c9cfa9287e1eb9383625bc) )
	ROM_LOAD32_BYTE( "man-12.3e",   0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",   0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",   0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",   0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "hj_08.17k",  0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "man-00.8a",  0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) // Encrypted tiles

	ROM_REGION( 0x500000, "tiles2", 0 )
	ROM_LOAD( "man-05.16a", 0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) // Encrypted tiles
	ROM_CONTINUE(           0x140000,  0x40000 )
	ROM_CONTINUE(           0x280000,  0x40000 )
	ROM_CONTINUE(           0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a", 0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) // Encrypted tiles
	ROM_CONTINUE(           0x180000,  0x40000 )
	ROM_CONTINUE(           0x2c0000,  0x40000 )
	ROM_CONTINUE(           0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a", 0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) // Encrypted tiles
	ROM_CONTINUE(           0x1c0000,  0x40000 )
	ROM_CONTINUE(           0x300000,  0x40000 )
	ROM_CONTINUE(           0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a", 0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) // Encrypted tiles
	ROM_CONTINUE(           0x200000,  0x40000 )
	ROM_CONTINUE(           0x340000,  0x40000 )
	ROM_CONTINUE(           0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a", 0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) // Encrypted tiles
	ROM_CONTINUE(           0x240000,  0x40000 )
	ROM_CONTINUE(           0x380000,  0x40000 )
	ROM_CONTINUE(           0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "sprites", 0 ) // Sprites
	ROM_LOAD( "man-06.17a",  0x200000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD( "man-07.18a",  0x000000,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD( "man-08.17c",  0x300000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD( "man-09.21c",  0x100000,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k", 0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k", 0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "ts-00.4h",  0x0000, 0x0117, CRC(ebc2908e) SHA1(dca14a55abd1d88ee09092d4122614e55c3e7f53) )
	ROM_LOAD( "ts-01.5h",  0x0200, 0x0117, CRC(c776a980) SHA1(cd4bdcfb755f561fefa4c88fab5d6d2397332aa7) )
	ROM_LOAD( "ts-02.12l", 0x0400, 0x01bf, CRC(6f26528c) SHA1(2cf869b2a789a9b0646162a61c147bcbb13c9141) )
	ROM_LOAD( "pal16r8b.14c", 0x0600, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( captavenu ) // DE-0351-x PCB (x=3 or 4)
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "hh_00-19.1e", 0x000000, 0x20000, CRC(08b870e0) SHA1(44c837e3c5dfc9764d89b0ebb3e9b7a40fe4d76f) )
	ROM_LOAD32_BYTE( "hh_01-19.1h", 0x000001, 0x20000, CRC(0dc0feca) SHA1(cb1c97aac59dabcf6c37bc1562cf2f62bca951f1) )
	ROM_LOAD32_BYTE( "hh_02-19.1k", 0x000002, 0x20000, CRC(26ef94c0) SHA1(985fae62a6a7ca7e1e64dba2db053b08206c65e7) )
	ROM_LOAD32_BYTE( "hn_03-4.1m",  0x000003, 0x20000, CRC(3b59ba05) SHA1(400e868e59977e56a4fa1870321c643983ba4162) )
	ROM_LOAD32_BYTE( "man-12.3e",   0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",   0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",   0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",   0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "hj_08.17k",  0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "man-00.8a",  0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) // Encrypted tiles

	ROM_REGION( 0x500000, "tiles2", 0 )
	ROM_LOAD( "man-05.16a", 0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) // Encrypted tiles
	ROM_CONTINUE(           0x140000,  0x40000 )
	ROM_CONTINUE(           0x280000,  0x40000 )
	ROM_CONTINUE(           0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a", 0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) // Encrypted tiles
	ROM_CONTINUE(           0x180000,  0x40000 )
	ROM_CONTINUE(           0x2c0000,  0x40000 )
	ROM_CONTINUE(           0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a", 0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) // Encrypted tiles
	ROM_CONTINUE(           0x1c0000,  0x40000 )
	ROM_CONTINUE(           0x300000,  0x40000 )
	ROM_CONTINUE(           0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a", 0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) // Encrypted tiles
	ROM_CONTINUE(           0x200000,  0x40000 )
	ROM_CONTINUE(           0x340000,  0x40000 )
	ROM_CONTINUE(           0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a", 0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) // Encrypted tiles
	ROM_CONTINUE(           0x240000,  0x40000 )
	ROM_CONTINUE(           0x380000,  0x40000 )
	ROM_CONTINUE(           0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "sprites", 0 ) // Sprites
	ROM_LOAD( "man-06.17a",  0x200000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD( "man-07.18a",  0x000000,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD( "man-08.17c",  0x300000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD( "man-09.21c",  0x100000,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k", 0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k", 0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "ts-00.4h",  0x0000, 0x0117, CRC(ebc2908e) SHA1(dca14a55abd1d88ee09092d4122614e55c3e7f53) )
	ROM_LOAD( "ts-01.5h",  0x0200, 0x0117, CRC(c776a980) SHA1(cd4bdcfb755f561fefa4c88fab5d6d2397332aa7) )
	ROM_LOAD( "ts-02.12l", 0x0400, 0x01bf, CRC(6f26528c) SHA1(2cf869b2a789a9b0646162a61c147bcbb13c9141) )
ROM_END

ROM_START( captavenuu ) // DE-0351-x PCB (x=3 or 4)
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "hh-00.1e",    0x000000, 0x20000, CRC(c34da654) SHA1(a1988a6a45991db6dee10b484049f6703b4671c9) )
	ROM_LOAD32_BYTE( "hh-01.1h",    0x000001, 0x20000, CRC(55abe63f) SHA1(98772eff3ebb5a4f243c7a77d398eb142d1505cb) )
	ROM_LOAD32_BYTE( "hh-02.1k",    0x000002, 0x20000, CRC(6096a9fb) SHA1(aa81189b9c185dc5d59f888afcb17a1e4935c241) )
	ROM_LOAD32_BYTE( "hh-03.1m",    0x000003, 0x20000, CRC(93631ded) SHA1(b4c8a6cbf586f895e637c0ed38f0842327624423) )
	ROM_LOAD32_BYTE( "man-12.3e",   0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",   0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",   0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",   0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "hj_08.17k",  0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "man-00.8a",  0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) // Encrypted tiles

	ROM_REGION( 0x500000, "tiles2", 0 )
	ROM_LOAD( "man-05.16a", 0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) // Encrypted tiles
	ROM_CONTINUE(           0x140000,  0x40000 )
	ROM_CONTINUE(           0x280000,  0x40000 )
	ROM_CONTINUE(           0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a", 0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) // Encrypted tiles
	ROM_CONTINUE(           0x180000,  0x40000 )
	ROM_CONTINUE(           0x2c0000,  0x40000 )
	ROM_CONTINUE(           0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a", 0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) // Encrypted tiles
	ROM_CONTINUE(           0x1c0000,  0x40000 )
	ROM_CONTINUE(           0x300000,  0x40000 )
	ROM_CONTINUE(           0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a", 0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) // Encrypted tiles
	ROM_CONTINUE(           0x200000,  0x40000 )
	ROM_CONTINUE(           0x340000,  0x40000 )
	ROM_CONTINUE(           0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a", 0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) // Encrypted tiles
	ROM_CONTINUE(           0x240000,  0x40000 )
	ROM_CONTINUE(           0x380000,  0x40000 )
	ROM_CONTINUE(           0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "sprites", 0 ) // Sprites
	ROM_LOAD( "man-06.17a",  0x200000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD( "man-07.18a",  0x000000,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD( "man-08.17c",  0x300000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD( "man-09.21c",  0x100000,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k", 0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k", 0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "ts-00.4h",  0x0000, 0x0117, CRC(ebc2908e) SHA1(dca14a55abd1d88ee09092d4122614e55c3e7f53) )
	ROM_LOAD( "ts-01.5h",  0x0200, 0x0117, CRC(c776a980) SHA1(cd4bdcfb755f561fefa4c88fab5d6d2397332aa7) )
	ROM_LOAD( "ts-02.12l", 0x0400, 0x01bf, CRC(6f26528c) SHA1(2cf869b2a789a9b0646162a61c147bcbb13c9141) )
ROM_END

ROM_START( captavenua ) // DE-0351-x PCB (x=3 or 4)
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "hh_00-4.2e",   0x000000, 0x20000, CRC(0e1acc05) SHA1(7eb6206efad233f9f4ee51102f9fe6b58f0719ea) )
	ROM_LOAD32_BYTE( "hh_01-4.2h",   0x000001, 0x20000, CRC(4ff0351d) SHA1(15fc2662ff0d32986c4d4d074b985ad853da34e1) )
	ROM_LOAD32_BYTE( "hh_02-4.2k",   0x000002, 0x20000, CRC(e84c0665) SHA1(d846f04315af49abeca00314b3d23e1d8c638dcd) )
	ROM_LOAD32_BYTE( "hh_03-4.2m",   0x000003, 0x20000, CRC(bc050740) SHA1(bee425e76734251444c9cfa9287e1eb9383625bc) )
	ROM_LOAD32_BYTE( "man-12.3e",   0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",   0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",   0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",   0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "hj_08.17k",  0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "man-00.8a",  0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) // Encrypted tiles

	ROM_REGION( 0x500000, "tiles2", 0 )
	ROM_LOAD( "man-05.16a", 0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) // Encrypted tiles
	ROM_CONTINUE(           0x140000,  0x40000 )
	ROM_CONTINUE(           0x280000,  0x40000 )
	ROM_CONTINUE(           0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a", 0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) // Encrypted tiles
	ROM_CONTINUE(           0x180000,  0x40000 )
	ROM_CONTINUE(           0x2c0000,  0x40000 )
	ROM_CONTINUE(           0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a", 0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) // Encrypted tiles
	ROM_CONTINUE(           0x1c0000,  0x40000 )
	ROM_CONTINUE(           0x300000,  0x40000 )
	ROM_CONTINUE(           0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a", 0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) // Encrypted tiles
	ROM_CONTINUE(           0x200000,  0x40000 )
	ROM_CONTINUE(           0x340000,  0x40000 )
	ROM_CONTINUE(           0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a", 0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) // Encrypted tiles
	ROM_CONTINUE(           0x240000,  0x40000 )
	ROM_CONTINUE(           0x380000,  0x40000 )
	ROM_CONTINUE(           0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "sprites", 0 ) // Sprites
	ROM_LOAD( "man-06.17a",  0x200000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD( "man-07.18a",  0x000000,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD( "man-08.17c",  0x300000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD( "man-09.21c",  0x100000,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k", 0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k", 0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "ts-00.4h",  0x0000, 0x0117, CRC(ebc2908e) SHA1(dca14a55abd1d88ee09092d4122614e55c3e7f53) )
	ROM_LOAD( "ts-01.5h",  0x0200, 0x0117, CRC(c776a980) SHA1(cd4bdcfb755f561fefa4c88fab5d6d2397332aa7) )
	ROM_LOAD( "ts-02.12l", 0x0400, 0x01bf, CRC(6f26528c) SHA1(2cf869b2a789a9b0646162a61c147bcbb13c9141) )
ROM_END

ROM_START( captavenj ) // DE-0351-x PCB (x=3 or 4)
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "hj_00-2.1e",  0x000000, 0x20000, CRC(10b1faaf) SHA1(9d76885200a846b4751c8d44ff591e2aff7c4148) )
	ROM_LOAD32_BYTE( "hj_01-2.1h",  0x000001, 0x20000, CRC(62c59f27) SHA1(20bbb7f3ff63a8c795686c1d56d51e90305daa77) )
	ROM_LOAD32_BYTE( "hj_02-2.1k",  0x000002, 0x20000, CRC(ce946cad) SHA1(9f1e92f5149e8a8d0236d5a7ba854ee100fd8488) )
	ROM_LOAD32_BYTE( "hj_03-2.1m",  0x000003, 0x20000, CRC(140cf9ce) SHA1(e2260ca4cea2fd7b64b8a78fd5444a7628bdafbb) )
	ROM_LOAD32_BYTE( "man-12.3e",   0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",   0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",   0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",   0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "hj_08.17k",  0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "man-00.8a",  0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) // Encrypted tiles

	ROM_REGION( 0x500000, "tiles2", 0 )
	ROM_LOAD( "man-05.16a", 0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) // Encrypted tiles
	ROM_CONTINUE(           0x140000,  0x40000 )
	ROM_CONTINUE(           0x280000,  0x40000 )
	ROM_CONTINUE(           0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a", 0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) // Encrypted tiles
	ROM_CONTINUE(           0x180000,  0x40000 )
	ROM_CONTINUE(           0x2c0000,  0x40000 )
	ROM_CONTINUE(           0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a", 0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) // Encrypted tiles
	ROM_CONTINUE(           0x1c0000,  0x40000 )
	ROM_CONTINUE(           0x300000,  0x40000 )
	ROM_CONTINUE(           0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a", 0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) // Encrypted tiles
	ROM_CONTINUE(           0x200000,  0x40000 )
	ROM_CONTINUE(           0x340000,  0x40000 )
	ROM_CONTINUE(           0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a", 0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) // Encrypted tiles
	ROM_CONTINUE(           0x240000,  0x40000 )
	ROM_CONTINUE(           0x380000,  0x40000 )
	ROM_CONTINUE(           0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "sprites", 0 ) // Sprites
	ROM_LOAD( "man-06.17a",  0x200000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD( "man-07.18a",  0x000000,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD( "man-08.17c",  0x300000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD( "man-09.21c",  0x100000,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k", 0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k", 0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "ts-00.4h",  0x0000, 0x0117, CRC(ebc2908e) SHA1(dca14a55abd1d88ee09092d4122614e55c3e7f53) )
	ROM_LOAD( "ts-01.5h",  0x0200, 0x0117, CRC(c776a980) SHA1(cd4bdcfb755f561fefa4c88fab5d6d2397332aa7) )
	ROM_LOAD( "ts-02.12l", 0x0400, 0x01bf, CRC(6f26528c) SHA1(2cf869b2a789a9b0646162a61c147bcbb13c9141) )
ROM_END


//**************************************************************************
//  INITIALIZER
//**************************************************************************

void captaven_state::init_captaven()
{
	deco56_decrypt_gfx(machine(), "tiles1");
	deco56_decrypt_gfx(machine(), "tiles2");
}


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT    MACHINE    INPUT     CLASS           INIT            ROT   COMPANY                  FULLNAME                                              FLAGS
GAME( 1991, captaven,   0,        captaven,  captaven, captaven_state, init_captaven,  ROT0, "Data East Corporation", "Captain America and The Avengers (Asia Version 1.4)",    MACHINE_SUPPORTS_SAVE )
GAME( 1991, captavena,  captaven, captaven,  captaven, captaven_state, init_captaven,  ROT0, "Data East Corporation", "Captain America and The Avengers (Asia Version 1.0)",    MACHINE_SUPPORTS_SAVE )
GAME( 1991, captavene,  captaven, captaven,  captaven, captaven_state, init_captaven,  ROT0, "Data East Corporation", "Captain America and The Avengers (UK Version 1.4)",      MACHINE_SUPPORTS_SAVE )
GAME( 1991, captavenu,  captaven, captaven,  captaven, captaven_state, init_captaven,  ROT0, "Data East Corporation", "Captain America and The Avengers (US Version 1.9)",      MACHINE_SUPPORTS_SAVE )
GAME( 1991, captavenuu, captaven, captaven,  captaven, captaven_state, init_captaven,  ROT0, "Data East Corporation", "Captain America and The Avengers (US Version 1.6)",      MACHINE_SUPPORTS_SAVE )
GAME( 1991, captavenua, captaven, captaven,  captaven, captaven_state, init_captaven,  ROT0, "Data East Corporation", "Captain America and The Avengers (US Version 1.4)",      MACHINE_SUPPORTS_SAVE )
GAME( 1991, captavenj,  captaven, captaven,  captaven, captaven_state, init_captaven,  ROT0, "Data East Corporation", "Captain America and The Avengers (Japan Version 0.2)",   MACHINE_SUPPORTS_SAVE )

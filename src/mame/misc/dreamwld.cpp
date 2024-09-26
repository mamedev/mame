// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

    SemiCom 68020 based hardware
    Driver by David Haywood

Baryon - Future Assault          (c) 1997 SemiCom / Tirano
Cute Fighter                     (c) 1998 SemiCom
Rolling Crush                    (c) 1999 Trust / SemiCom
Gaia - The Last Choice of Earth  (c) 1999 SemiCom / XESS
Dream World                      (c) 2000 SemiCom

Note: There is a SemiCom game known as Lode Quest 1998(?). This game is very similar to Dream World.
      It's not known if Lode Quest is a alternate title or a prequel of Dream World.

Note: this hardware is a copy of Psikyo's 68020 based hardware,
      the Strikers 1945 bootleg has the same unknown rom!

      It isn't quite as flexible as the original Psikyo hardware
      by the looks of it, there are various subtle changes to how
      things work, for example the tilemap sizes and missing
      transparent pen modification.  This makes it rather hard to
      merge with psikyo.cpp and it should probably be left separate.


Stephh's notes (based on the game M68EC020 code and some tests) :

  - Don't trust the "test mode" as it displays Dip Switches infos
    that are in fact unused by the game ! Leftover from another game ?

    PORT_START("DSW")
    PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
    PORT_DIPSETTING(      0x0002, "1" )
    PORT_DIPSETTING(      0x0003, "2" )
    PORT_DIPSETTING(      0x0001, "3" )
    PORT_DIPSETTING(      0x0000, "4" )
    PORT_DIPUNUSED_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW2:3" )
    PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW2:4" )
    PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW2:5" )
    PORT_DIPNAME( 0x0060, 0x0060, "Ticket Payout" )         PORT_DIPLOCATION("SW2:6,7")
    PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
    PORT_DIPSETTING(      0x0020, "Little" )
    PORT_DIPSETTING(      0x0060, DEF_STR( Normal ) )
    PORT_DIPSETTING(      0x0040, "Much" )
    PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:8")
    PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
    PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,3,4")
    PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
    PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
    PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )
    PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(      0x0a00, DEF_STR( 2C_3C ) )
    PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_2C ) )
    PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
    PORT_DIPNAME( 0x7000, 0x7000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6,7")
    PORT_DIPSETTING(      0x2000, "Level 1" )
    PORT_DIPSETTING(      0x1000, "Level 2" )
    PORT_DIPSETTING(      0x0000, "Level 3" )
    PORT_DIPSETTING(      0x7000, "Level 4" )
    PORT_DIPSETTING(      0x6000, "Level 5" )
    PORT_DIPSETTING(      0x5000, "Level 6" )
    PORT_DIPSETTING(      0x4000, "Level 7" )
    PORT_DIPSETTING(      0x3000, "Level 8" )
    PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW1:8" )


   note:

   Baryon has some annoying sound looping clicks / cutouts, these need to
    be verified against the HW (it's a very cheap sound system, so it might
    be accurate)

   Baryon has playfield background which fade in with very rough / visible
    edges.  In this case the tilemap size registers from the original
    psikyo hardware are set to be the alternate tilemap size, however that
    doesn't make sense in the context of the data in RAM, which doesn't
    appear to wrap properly anyway, again, it's likely this is just how the
    game is.  Furthermore the BG test in the test menu indicates that it
    tests alternate tilemap sizes, but doesn't even write to the register,
    probably a leftover from hardware development as the test menu is mostly
    incomplete.

   All: sprite priority, the original psikyo.cpp HW has sprite<->tilemap
    priority but we don't support it here, does the clone HW support it?

   All: sprite zooming, again the original psikyo.cpp HW supports this, but we
    don't support it here.  The Strikers 1945 bootleg in psikyo.cpp doesn't
    appear to support it properly either, so it might be missing on these
    clone boards.

   At least based on the dumped Dream World MCU code, the internal ROM
   doesn't appear to do any kind of bounds checking when reading out the data
   table, so assuming no read address lock up the MCU you could probably just
   overflow the counter to read out internal ROM.  Also compared to earlier
   SemiCom MCUs the code is minimal, just 0x3a bytes and the payload table,
   no header with the game title, programmer name or date.

*/

#include "emu.h"
#include "cpu/m68000/m68020.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include <algorithm>


namespace {

class dreamwld_state : public driver_device
{
public:
	dreamwld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_spriteram(*this, "spriteram")
		, m_vram(*this, "vram_%u", 0U, 0x2000U, ENDIANNESS_BIG)
		, m_vregs(*this, "vregs")
		, m_workram(*this, "workram")
		, m_lineram(*this, "lineram", 0x400U, ENDIANNESS_BIG)
		, m_spritelut(*this, "spritelut")
		, m_okibank(*this, "oki%ubank", 1)
		, m_dsw(*this, "DSW")
		, m_maincpu(*this, "maincpu")
		, m_mcu(*this, "mcu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{
		std::fill(std::begin(m_old_linescroll), std::end(m_old_linescroll), 0);
	}

	void baryon(machine_config &config);
	void dreamwld(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	/* memory pointers */
	required_shared_ptr<u32> m_spriteram;
	memory_share_array_creator<u16, 2> m_vram;
	required_shared_ptr<u32> m_vregs;
	required_shared_ptr<u32> m_workram;
	memory_share_creator<u16> m_lineram;
	required_memory_region m_spritelut;
	optional_memory_bank_array<2> m_okibank;
	required_ioport m_dsw;

	u16 lineram_r(offs_t offset) { return m_lineram[offset]; }
	void lineram_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_lineram[offset]); }

	/* video-related */
	tilemap_t *m_tilemap[2][2];
	int       m_tilebank[2];
	int       m_tilebankold[2];
	int       m_old_linescroll[2];

	std::unique_ptr<u32[]> m_spritebuf[2];

	/* misc */
	uint8_t m_port1_data;
	uint8_t m_port2_data;
	uint8_t m_protlatch;

	required_device<cpu_device> m_maincpu;
	required_device<mcs51_cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void prot_p1_w(uint8_t data);
	void prot_p2_w(uint8_t data);
	uint8_t prot_p2_r();

	void to_prot_w(uint32_t data);


	template<int Layer> u16 vram_r(offs_t offset);
	template<int Layer> void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u32 protdata_r();
	template<int Chip> void okibank_w(u8 data);
	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void baryon_map(address_map &map) ATTR_COLD;
	void dreamwld_map(address_map &map) ATTR_COLD;
	void oki1_map(address_map &map) ATTR_COLD;
	void oki2_map(address_map &map) ATTR_COLD;
};


void dreamwld_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	const u32 *source = m_spritebuf[0].get();
	const u32 *finish = m_spritebuf[0].get() + 0x1000 / 4;
	const u16 *redirect = &m_spritelut->as_u16();
	const int xoffset = 4;

	while (source < finish)
	{
		int xpos = (source[0] & 0x000001ff) >> 0;
		int ypos = (source[0] & 0x01ff0000) >> 16;
		u8 xsize = (source[0] & 0x00000e00) >> 9;
		u8 ysize = (source[0] & 0x0e000000) >> 25;

		u32 tileno       = (source[1] & 0x0001ffff) >>0;
		const u32 colour = (source[1] & 0x3f000000) >>24;
		const bool xflip = (source[1] & 0x40000000);
		const bool yflip = (source[1] & 0x80000000);

		int xinc = 16;
		int yinc = 16;

		xpos += xoffset;
		xpos &= 0x1ff;

		if (xflip)
		{
			xinc = -16;
			xpos += 16 * xsize;
		}

		if (yflip)
		{
			yinc = -16;
			ypos += 16 * ysize;
		}

		ysize++; xsize++; // size 0 = 1 tile

		xpos -=16;

		for (int yct = 0; yct < ysize; yct++)
		{
			for (int xct = 0; xct < xsize; xct++)
			{
					gfx->transpen(bitmap,cliprect, redirect[tileno], colour, xflip, yflip, xpos + xct * xinc, ypos + yct * yinc, 0);
					gfx->transpen(bitmap,cliprect, redirect[tileno], colour, xflip, yflip, (xpos + xct * xinc) - 0x200, ypos + yct * yinc, 0);
					gfx->transpen(bitmap,cliprect, redirect[tileno], colour, xflip, yflip, (xpos + xct * xinc) - 0x200, (ypos + yct * yinc) - 0x200, 0);
					gfx->transpen(bitmap,cliprect, redirect[tileno], colour, xflip, yflip, xpos + xct * xinc, (ypos + yct * yinc) - 0x200 , 0);

				tileno++;
			}
		}

		source += 2;
	}
}

template<int Layer>
u16 dreamwld_state::vram_r(offs_t offset)
{
	return m_vram[Layer][offset];
}

template<int Layer>
void dreamwld_state::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram[Layer][offset]);
	for (int size = 0; size < 2; size++)
	{
		m_tilemap[Layer][size]->mark_tile_dirty(offset);
	}
}

template<int Layer>
TILE_GET_INFO_MEMBER(dreamwld_state::get_tile_info)
{
	const u16 tileno = m_vram[Layer][tile_index];
	const u16 colour = tileno >> 13;
	tileinfo.set(1, (tileno & 0x1fff) | (m_tilebank[Layer] << 13), (Layer * 0x40) + colour, 0);
}


void dreamwld_state::video_start()
{
	m_tilemap[0][0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dreamwld_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_tilemap[1][0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dreamwld_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_tilemap[0][1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dreamwld_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 128, 32);
	m_tilemap[1][1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dreamwld_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 128, 32);
	m_tilemap[1][0]->set_transparent_pen(0);
	m_tilemap[1][1]->set_transparent_pen(0);

	for (int layer = 0; layer < 2; layer++)
	{
		for (int size = 0; size < 2; size++)
		{
			m_tilemap[layer][size]->set_scroll_rows(1);
			m_tilemap[layer][size]->set_scroll_cols(1);
		}
	}

	m_spritebuf[0] = std::make_unique<u32[]>(0x2000 / 4);
	m_spritebuf[1] = std::make_unique<u32[]>(0x2000 / 4);

	save_pointer(NAME(m_spritebuf[0]), 0x2000 / 4, 0);
	save_pointer(NAME(m_spritebuf[1]), 0x2000 / 4, 1);

}

void dreamwld_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		memcpy(m_spritebuf[1].get(), m_spritebuf[0].get(), 0x2000);
		memcpy(m_spritebuf[0].get(), m_spriteram, 0x2000);
	}
}


u32 dreamwld_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	tilemap_t *tmptilemap[2];

	const u32 scrolly[2]{ m_vregs[(0x000 / 4)] + 32, m_vregs[(0x008 / 4)] + 32 };
	const u32 scrollx[2]{ m_vregs[(0x004 / 4)] + 0, m_vregs[(0x00c / 4)] + 2 };
	const u32 layer_ctrl[2]{ m_vregs[0x010 / 4], m_vregs[0x014 / 4] };

	for (int layer = 0; layer < 2; layer++)
	{
		m_tilebank[layer] = (layer_ctrl[layer] >> 6) & 1;

		if (m_tilebank[layer] != m_tilebankold[layer])
		{
			m_tilebankold[layer] = m_tilebank[layer];
			for (int size = 0; size < 2; size++)
				m_tilemap[layer][size]->mark_all_dirty();
		}

		// Test mode only, Other size is enable?
		const int size = (layer_ctrl[layer] & 0x0400) >> 10;
		tmptilemap[layer] = m_tilemap[layer][size];
		const int row_mask = 0x3ff >> size;

		tmptilemap[layer]->set_scrolly(0, scrolly[layer]);

		if (layer_ctrl[layer] & 0x0300)
		{
			const int tile_rowscroll = (layer_ctrl[layer] & 0x0200) >> 7;
			if (m_old_linescroll[layer] != (layer_ctrl[layer] & 0x0300))
			{
				for (int i = 0; i < 2; i++)
				{
					m_tilemap[layer][i]->set_scroll_rows(((64 * 16) >> i));
				}
				m_old_linescroll[layer] = (layer_ctrl[layer] & 0x0300);
			}
			const u16* linebase = &m_lineram[(layer * 0x200) / 2];
			for (int i = 0; i < 256; i++)   /* 256 screen lines */
			{
				/* per-line rowscroll */
				const int x0 = linebase[((i + 32) & 0xff) >> tile_rowscroll];

				tmptilemap[layer]->set_scrollx(
						(i + scrolly[layer]) & row_mask,
						scrollx[layer] + x0 );
			}
		}
		else
		{
			if (m_old_linescroll[layer] != (layer_ctrl[layer] & 0x0300))
			{
				for (int i = 0; i < 2; i++)
				{
					m_tilemap[layer][i]->set_scroll_rows(1);
				}
				m_old_linescroll[layer] = (layer_ctrl[layer] & 0x0300);
			}

			tmptilemap[layer]->set_scrollx(0, scrollx[layer]);
		}
	}

	tmptilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	tmptilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);

	return 0;
}




void dreamwld_state::oki1_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr("oki1bank");
}

void dreamwld_state::oki2_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr("oki2bank");
}

template<int Chip>
void dreamwld_state::okibank_w(u8 data)
{
	m_okibank[Chip]->set_entry(data & 3);
}


void dreamwld_state::baryon_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().nopw();

	map(0x400000, 0x401fff).ram().share("spriteram");
	map(0x600000, 0x601fff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x800000, 0x801fff).rw(FUNC(dreamwld_state::vram_r<0>), FUNC(dreamwld_state::vram_w<0>));
	map(0x802000, 0x803fff).rw(FUNC(dreamwld_state::vram_r<1>), FUNC(dreamwld_state::vram_w<1>));
	map(0x804000, 0x8043ff).rw(FUNC(dreamwld_state::lineram_r), FUNC(dreamwld_state::lineram_w));  // linescroll
	map(0x804400, 0x805fff).ram().share("vregs");

	map(0xc00000, 0xc00003).portr("INPUTS");
	map(0xc00004, 0xc00007).lr16(NAME([this] () { return u16(m_dsw->read()); }));

	map(0xc0000f, 0xc0000f).w(FUNC(dreamwld_state::okibank_w<0>)); // sfx
	map(0xc00018, 0xc00018).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // sfx

	// C00010 might reset the MCU?
	map(0xc00020, 0xc00023).w(FUNC(dreamwld_state::to_prot_w));
	map(0xc00030, 0xc00033).r(FUNC(dreamwld_state::protdata_r)); // it reads protection data (irq code) from here and puts it at ffd000

	map(0xfe0000, 0xffffff).ram().share("workram"); // work ram
}

void dreamwld_state::dreamwld_map(address_map &map)
{
	baryon_map(map);

	map(0xc0002f, 0xc0002f).w(FUNC(dreamwld_state::okibank_w<1>)); // sfx
	map(0xc00028, 0xc00028).rw("oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // sfx
}


static INPUT_PORTS_START( dreamwld )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0000fffc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) /* "Book" (when you get one of them) */
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) /* "Jump" */
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) /* "Dig" */
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) /* "Book" (when you get one of them) */
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) /* "Jump" */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) /* "Dig" */
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW2:6" ) /* see notes - "Ticket Payout" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW2:7" ) /* see notes - "Ticket Payout" */
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:8")     /* gives in fact 99 credits */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW1:1" ) /* see notes - "Demo Sounds" */
	PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED_DIPLOC( 0x1000, IP_ACTIVE_LOW, "SW1:5" ) /* see notes - "Difficulty" */
	PORT_DIPUNUSED_DIPLOC( 0x2000, IP_ACTIVE_LOW, "SW1:6" ) /* see notes - "Difficulty" */
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW1:7" ) /* see notes - "Difficulty" */
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( baryon )
	PORT_INCLUDE(dreamwld)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0004, 0x0004, "Bomb Stock" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0004, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( rolcrush )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0000fffc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)

	PORT_START("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW2:1" ) /* As listed in service mode, but tested */
	PORT_DIPUNUSED_DIPLOC( 0x0002, IP_ACTIVE_LOW, "SW2:2" ) /* These might have some use, requires investigation of code */
	PORT_DIPUNUSED_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x7000, 0x7000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(      0x2000, "Level 1" )
	PORT_DIPSETTING(      0x1000, "Level 2" )
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x7000, "Level 4" )
	PORT_DIPSETTING(      0x6000, "Level 5" )
	PORT_DIPSETTING(      0x5000, "Level 6" )
	PORT_DIPSETTING(      0x4000, "Level 7" )
	PORT_DIPSETTING(      0x3000, "Level 8" )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( cutefght )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0000fffc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)

	PORT_START("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW2:1" ) /* As listed in service mode, but tested */
	PORT_DIPUNUSED_DIPLOC( 0x0002, IP_ACTIVE_LOW, "SW2:2" ) /* These might have some use, requires investigation of code */
	PORT_DIPUNUSED_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPNAME( 0x0060, 0x0060, "Ticket Payout" )         PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, "Little" )
	PORT_DIPSETTING(      0x0060, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0040, "Much" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1") /* Has no effect?? */
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x7000, 0x7000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(      0x2000, "Level 1" )
	PORT_DIPSETTING(      0x1000, "Level 2" )
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x7000, "Level 4" )
	PORT_DIPSETTING(      0x6000, "Level 5" )
	PORT_DIPSETTING(      0x5000, "Level 6" )
	PORT_DIPSETTING(      0x4000, "Level 7" )
	PORT_DIPSETTING(      0x3000, "Level 8" )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( gaialast )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0000fffc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003,  0x0001, DEF_STR( Lives )  )  PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(       0x0002, "1" )
	PORT_DIPSETTING(       0x0003, "2" )
	PORT_DIPSETTING(       0x0001, "3" )
	PORT_DIPSETTING(       0x0000, "4" )
	PORT_DIPNAME( 0x0004,  0x0000, "Bomb Stock" )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(       0x0004, "2" )
	PORT_DIPSETTING(       0x0000, "3" )
	PORT_DIPNAME( 0x0008,  0x0000, "Lock Vertical Scroll" )  PORT_DIPLOCATION("SW2:4") // if Off the game pans the screen up/down when you move up/down
	PORT_DIPSETTING(       0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW2:6" ) // these are listed as ticket payout in test mode, but I believe
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW2:7" ) // that to be a leftover from some redemption game
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1") /* Title screen sounds only */
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x7000, 0x7000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(      0x2000, "Level 1" ) // listed as 'Level 3' in service mode
	PORT_DIPSETTING(      0x1000, "Level 2" ) // ^^
	PORT_DIPSETTING(      0x0000, "Level 3" )
	PORT_DIPSETTING(      0x7000, "Level 4" )
	PORT_DIPSETTING(      0x6000, "Level 5" )
	PORT_DIPSETTING(      0x5000, "Level 6" )
	PORT_DIPSETTING(      0x4000, "Level 7" )
	PORT_DIPSETTING(      0x3000, "Level 8" ) // listed as 'Little' in service mode, but still clearly the most difficult
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

static GFXDECODE_START( gfx_dreamwld )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_16x16x4_packed_msb, 0x000, 0x40 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, gfx_16x16x4_packed_msb, 0x800, 0x48 ) // [1] Layer 0 + 1
GFXDECODE_END


void dreamwld_state::machine_start()
{
	if (m_okibank[0].found())
	{
		m_okibank[0]->configure_entries(0, 4, memregion("oki1")->base() + 0x30000, 0x10000);
		m_okibank[0]->set_entry(0);
	}

	if (m_okibank[1].found())
	{
		m_okibank[1]->configure_entries(0, 4, memregion("oki2")->base() + 0x30000, 0x10000);
		m_okibank[1]->set_entry(0);
	}

	save_item(NAME(m_port1_data));
	save_item(NAME(m_port2_data));
	save_item(NAME(m_protlatch));

	save_item(NAME(m_tilebank));
	save_item(NAME(m_tilebankold));
}

void dreamwld_state::machine_reset()
{
	m_tilebankold[0] = m_tilebankold[1] = -1;
	m_tilebank[0] = m_tilebank[1] = 0;
	m_port1_data = 0;
	m_port2_data = 0;
	m_protlatch = 0;
}

void dreamwld_state::prot_p1_w(uint8_t data)
{
	logerror("%s:prot_p1_w %02x\n", machine().describe_context(), data);
	m_port1_data = data;
}

void dreamwld_state::prot_p2_w(uint8_t data)
{
	logerror("%s:prot_p2_w %02x\n", machine().describe_context(), data);

	for (int bit = 0; bit < 8; bit++)
	{
		if ((m_port2_data & (1 << bit)) != (data & (1 << bit)))
		{
			if (data & (1 << bit))
			{
				logerror("bit %d low to high\n", bit);

				// bit == 0 is toggled before reading from port 0 (data not used, maybe reads whatever was written on to_prot_w into port 0 for reading tho)

				if (bit == 1) // toggled after writing a byte on port 1
				{
					m_protlatch = m_port1_data;
				}
			}
			else
			{
				logerror("bit %d high to low\n", bit);
			}
		}
	}

	m_port2_data = data;
}

uint8_t dreamwld_state::prot_p2_r()
{
	// bit 2 is waited on before reading from port 0
	// bit 3 is waited on after writing a byte

	// will inf loop reading here once the 68k stops requesting data
	//logerror("%s:prot_p2_r\n", machine().describe_context());
	return m_port2_data;
}

void dreamwld_state::to_prot_w(uint32_t data)
{
	m_port2_data &= 0xfb; // lower bit 0x04 to indicate data sent?
	logerror("%s:to_prot_w %08x\n", machine().describe_context(), data);
	m_maincpu->spin_until_time(attotime::from_usec(50)); // give it time to respond
}

u32 dreamwld_state::protdata_r()
{
	m_port2_data &= 0xf7; // clear bit 0x08 to indicate data received?
	logerror("%s:protdata_r\n", machine().describe_context());
	m_maincpu->spin_until_time(attotime::from_usec(50)); // give it time to respond
	return m_protlatch << 24;
}


void dreamwld_state::baryon(machine_config &config)
{
	/* basic machine hardware */
	M68EC020(config, m_maincpu, XTAL(32'000'000)/2); /* 16MHz verified */
	m_maincpu->set_addrmap(AS_PROGRAM, &dreamwld_state::baryon_map);
	m_maincpu->set_vblank_int("screen", FUNC(dreamwld_state::irq4_line_hold));

	AT89C52(config, m_mcu, XTAL(32'000'000) / 2); /* AT89C52 or 87(C)52, unknown clock (value from docs) */
	m_mcu->port_out_cb<1>().set(FUNC(dreamwld_state::prot_p1_w));
	m_mcu->port_out_cb<2>().set(FUNC(dreamwld_state::prot_p2_w));
	m_mcu->port_in_cb<2>().set(FUNC(dreamwld_state::prot_p2_r));

	//config.set_perfect_quantum(m_maincpu);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57.793);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(512,256);
	screen.set_visarea(0, 308-1, 0, 224-1);
	screen.set_screen_update(FUNC(dreamwld_state::screen_update));
	screen.screen_vblank().set(FUNC(dreamwld_state::screen_vblank));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x1000);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dreamwld);

	SPEAKER(config, "mono").front_center();

	okim6295_device &oki1(OKIM6295(config, "oki1", XTAL(32'000'000)/32, okim6295_device::PIN7_LOW)); /* 1MHz verified */
	oki1.add_route(ALL_OUTPUTS, "mono", 1.00);
	oki1.set_addrmap(0, &dreamwld_state::oki1_map);
}

void dreamwld_state::dreamwld(machine_config &config)
{
	baryon(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &dreamwld_state::dreamwld_map);

	okim6295_device &oki2(OKIM6295(config, "oki2", XTAL(32'000'000)/32, okim6295_device::PIN7_LOW)); /* 1MHz verified */
	oki2.add_route(ALL_OUTPUTS, "mono", 1.00);
	oki2.set_addrmap(0, &dreamwld_state::oki2_map);
}



/*

Baryon
SemiCom, 1997

PCB Layout
----------

|-------------------------------------------------|
|        1SEMICOM  62256   ACTEL        8SEMICOM  |
|VOL        M6295  62256   A1020B                 |
|    PAL  PAL              32MHz                  |
|    62256  62256             PAL                 |
| 2SEMICOM 4SEMICOM 68EC020   PAL    PAL          |
| 3SEMICOM 5SEMICOM           PAL    PAL          |
|J   62256  62256             PAL                 |
|A                            PAL    27MHz        |
|M                                 PAL            |
|M                         ACTEL    M5M44260      |
|A             6116        A1020B   M5M44260      |
|              6116                               |
|                          PAL                    |
|              6264        PAL                    |
| DSW1         6264                               |
| DSW2  P87C52                9SEMICOM            |
|                 6SEMICOM   10SEMICOM   27C160*  |
|3* 4*            7SEMICOM   11SEMICOM   27C160*  |
|-------------------------------------------------|

The PCB used for Baryon is an earlier version with a single OKI sound chip

* denotes unpopulated components
  3 & 4 are 10 pin headers

*/

ROM_START( baryon ) // this set had original SemiCom labels
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "4_semicom", 0x000000, 0x040000, CRC(6c1cdad0) SHA1(40c437507076ce52ec2240049d6b4bef180b104a) ) //  eprom type 27C020
	ROM_LOAD32_BYTE( "5_semicom", 0x000001, 0x040000, CRC(15917c9d) SHA1(6444be93e6a997070820e3c5a2e2e703e22883d9) )
	ROM_LOAD32_BYTE( "2_semicom", 0x000002, 0x040000, CRC(42b14a6c) SHA1(37e772a673732ef16767c14ad77a4faaa06d675a) )
	ROM_LOAD32_BYTE( "3_semicom", 0x000003, 0x040000, CRC(0ae6d86e) SHA1(410ad161688ec8516fe5ac7160a4a228dbb01936) )

	ROM_REGION( 0x02000, "mcu", 0 ) /* 87C52 MCU Code */
	ROM_LOAD( "mcu.bin", 0x00000, 0x02000, BAD_DUMP CRC(1ee7896c) SHA1(f1e8500b7b6aa4e6ae939e228ffd11462f10ba33) ) // handcrafted from Dream World MCU using correct protection data for this game

	ROM_REGION( 0x80000, "oki1", 0 ) /* OKI Samples */
	ROM_LOAD( "1_semicom", 0x000000, 0x80000, CRC(e0349074) SHA1(f3d53d96dff586a0ad1632f52e5559cdce5ed0d8) ) //  eprom type 27C040

	ROM_REGION( 0x400000, "gfx1", 0 ) /* Sprite Tiles - decoded */
	ROM_LOAD( "10_semicom", 0x000000, 0x200000, CRC(28bf828f) SHA1(271390cc4f4015a3b69976f0d0527947f13c971b) ) //  eprom type 27C160
	ROM_LOAD( "11_semicom", 0x200000, 0x200000, CRC(d0ff1bc6) SHA1(4aeb795222eedeeba770cf725122e989f97119b2) ) //  eprom type 27C160

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles - decoded */
	ROM_LOAD( "8_semicom",0x000000, 0x200000, CRC(684012e6) SHA1(4cb60907184b67be130b8385e4336320c0f6e4a7) ) //  eprom type 27C160

	ROM_REGION( 0x040000, "spritelut", 0 ) /* Sprite Code Lookup ... */
	ROM_LOAD16_BYTE( "6_semicom", 0x000000, 0x020000, CRC(fdbb08b0) SHA1(4b3ac56c4c8370b1434fb6a481fce0d9c52313e0) ) //  eprom type 27C010
	ROM_LOAD16_BYTE( "7_semicom", 0x000001, 0x020000, CRC(c9d20480) SHA1(3f6170e8e08fb7508bd13c23f243ec6888a91f5e) ) //  eprom type 27C010

	ROM_REGION( 0x10000, "unknown", 0 )
	ROM_LOAD( "9_semicom", 0x000000, 0x10000, CRC(0da8db45) SHA1(7d5bd71c5b0b28ff74c732edd7c662f46f2ab25b) ) //  eprom type 27C512
ROM_END

ROM_START( baryona ) // replacment labels? no SemiCom logo
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "4.bin", 0x000000, 0x040000, CRC(59e0df20) SHA1(ff12f4adcf731f6984db7d0fbdd7fcc71ce66aa4) )
	ROM_LOAD32_BYTE( "6.bin", 0x000001, 0x040000, CRC(abccbb3d) SHA1(01524f094543d872d775306024f51258a11e9240) )
	ROM_LOAD32_BYTE( "3.bin", 0x000002, 0x040000, CRC(046d4231) SHA1(05056efe5fec7f43c400f05278de516b01be0fdf) )
	ROM_LOAD32_BYTE( "5.bin", 0x000003, 0x040000, CRC(63d5e7cb) SHA1(269bf5ffe10f2464f823c4d377921e19cfb8bc46) )

	ROM_REGION( 0x02000, "mcu", 0 ) /* 87C52 MCU Code */
	ROM_LOAD( "mcu.bin", 0x00000, 0x02000, BAD_DUMP CRC(1ee7896c) SHA1(f1e8500b7b6aa4e6ae939e228ffd11462f10ba33) ) // handcrafted from Dream World MCU using correct protection data for this game

	ROM_REGION( 0x80000, "oki1", 0 ) /* OKI Samples */
	ROM_LOAD( "1.bin", 0x000000, 0x80000, CRC(e0349074) SHA1(f3d53d96dff586a0ad1632f52e5559cdce5ed0d8) )

	ROM_REGION( 0x400000, "gfx1", 0 ) /* Sprite Tiles - decoded */
	ROM_LOAD( "9.bin",  0x000000, 0x200000, CRC(28bf828f) SHA1(271390cc4f4015a3b69976f0d0527947f13c971b) )
	ROM_LOAD( "11.bin", 0x200000, 0x200000, CRC(d0ff1bc6) SHA1(4aeb795222eedeeba770cf725122e989f97119b2) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles - decoded */
	ROM_LOAD( "2.bin",0x000000, 0x200000, CRC(684012e6) SHA1(4cb60907184b67be130b8385e4336320c0f6e4a7) )

	ROM_REGION( 0x040000, "spritelut", 0 ) /* Sprite Code Lookup ... */
	ROM_LOAD16_BYTE( "8.bin", 0x000000, 0x020000, CRC(fdbb08b0) SHA1(4b3ac56c4c8370b1434fb6a481fce0d9c52313e0) )
	ROM_LOAD16_BYTE( "10.bin",0x000001, 0x020000, CRC(c9d20480) SHA1(3f6170e8e08fb7508bd13c23f243ec6888a91f5e) )

	ROM_REGION( 0x10000, "unknown", 0 )
	ROM_LOAD( "7.bin", 0x000000, 0x10000, CRC(0da8db45) SHA1(7d5bd71c5b0b28ff74c732edd7c662f46f2ab25b) )
ROM_END

/*

Cute Fighter
SemiCom, 1998

PCB Layout
----------

|-------------------------------------------------|
|    M6295  SEMICOM1 62256    ACTEL     SEMICOM12 |
|VOL M6295  SEMICOM2 62256    A1020B              |
|    PAL  PAL        32MHz                        |
| 62256  62256              PAL                   |
| SEMICOM3 SEMICOM5 68EC020 PAL    PAL            |
| SEMICOM4 SEMICOM6         PAL    PAL            |
|J 62256 62256              PAL                   |
|A                          PAL    27MHz          |
|M                                 PAL            |
|M                         ACTEL    M5M44260      |
|A             6116        A40MX04  M5M44260      |
|              6116                               |
|                          PAL                    |
|              6264        PAL                    |
| DSW1             6264                           |
| DSW2   8752                  SEMICOM9           |
|                    SEMICOM7 SEMICOM10 SEMICOM13 |
|3* 4*               SEMICOM8 SEMICOM11 SEMICOM14 |
|-------------------------------------------------|

A later version of the SemiCom 68020 hardware added a second OKI sound chip and sample rom

Main CPU 68EC020FG16           @ 16MHz
AD-65 (OKI MSM6295 rebadged)   @ 1MHz
Atmel AT89C52 MCU (secured)    @ 16MHZ

* 3 & 4 are 10 pin headers for unknown use. One might be used to drive the ticket dispenser

*/

ROM_START( cutefght )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "5_semicom", 0x000000, 0x080000, CRC(c14fd5dc) SHA1(f332105f5f249d693e792e7115f9e6cffb6db19f) )
	ROM_LOAD32_BYTE( "6_semicom", 0x000001, 0x080000, CRC(47440088) SHA1(c45503c4b5f271b430263ca079edeaaeadf5d9f6) )
	ROM_LOAD32_BYTE( "3_semicom", 0x000002, 0x080000, CRC(e7e7a866) SHA1(a31751f4164a427de59f0c76c9a8cb34370d8183) )
	ROM_LOAD32_BYTE( "4_semicom", 0x000003, 0x080000, CRC(476a3bf5) SHA1(5be1c70bbf4fcfc534b7f20bfceaa8da2e961330) )

	ROM_REGION( 0x02000, "mcu", 0 ) /* 87C52 MCU Code */
	ROM_LOAD( "mcu.bin", 0x00000, 0x02000, BAD_DUMP CRC(7d9acd7d) SHA1(12059143ac9e10f3a21402d2879ff0b5097d6de0) ) // handcrafted from Dream World MCU using correct protection data for this game

	ROM_REGION( 0x80000, "oki1", 0 ) /* OKI Samples - 1st chip */
	ROM_LOAD( "2_semicom", 0x000000, 0x80000, CRC(694ddaf9) SHA1(f9138e7e1d8f771c4e69c17f27fb2b70fbee076a) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* OKI Samples - 2nd chip */
	ROM_LOAD( "1_semicom", 0x000000, 0x80000, CRC(fa3b6890) SHA1(7534931c96d6fa05fee840a7ea07b87e2e2acc50) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* Sprite Tiles - decoded */
	ROM_LOAD( "10_semicom",  0x000000, 0x200000, CRC(62bf1e6e) SHA1(fb4b0db313e26687f0ebc6a8505a02e5348776da) )
	ROM_LOAD( "11_semicom",  0x200000, 0x200000, CRC(796f23a7) SHA1(adaa4c8525de428599f4489ecc8e966fed0d514d) )
	ROM_LOAD( "13_semicom",  0x400000, 0x200000, CRC(24222b3c) SHA1(08163863890c01728db89b8f4447841ecb4f4f62) )
	ROM_LOAD( "14_semicom",  0x600000, 0x200000, CRC(385b69d7) SHA1(8e7cae5589e354bea0b77b061af1d0c81d796f7c) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles - decoded */
	ROM_LOAD( "12_semicom",0x000000, 0x200000, CRC(45d29c22) SHA1(df719a061dcd14fb4388fb45dfee2054e56a1299) )

	ROM_REGION( 0x040000, "spritelut", 0 ) /* Sprite Code Lookup ... */
	ROM_LOAD16_BYTE( "7_semicom", 0x000000, 0x020000, CRC(39454102) SHA1(347e9242fd7e2092cfaacdce92691cf6024471ac) )
	ROM_LOAD16_BYTE( "8_semicom", 0x000001, 0x020000, CRC(fccb1b13) SHA1(fd4aec4a660f9913651fcc084e3f13eb0adbddd6) )

	ROM_REGION( 0x10000, "unknown", 0 ) /* ???? - not decoded seems to be in blocks of 0x41 bytes.. */
	ROM_LOAD( "9_semicom", 0x000000, 0x10000, CRC(0da8db45) SHA1(7d5bd71c5b0b28ff74c732edd7c662f46f2ab25b) )
ROM_END

/*

Rolling Crush
Trust / SemiCom, 1999

PCB Layout
----------

|-------------------------------------------------|
|    M6295* 27C40*  62256   ACTEL           ROM10 |
|VOL M6295  ROM6    62256   A40MX04               |
|    PAL  PAL       32MHz                         |
| 62256  62256              PAL                   |
| ROM2 ROM4       68EC020   PAL    PAL            |
| ROM1 ROM3                 PAL    PAL            |
|J 62256 62256              PAL                   |
|A                          PAL    27MHz          |
|M                                 PAL            |
|M                         ACTEL    M5M44260      |
|A             6116        A40MX04  M5M44260      |
|              6116                               |
|                          PAL                    |
|              6264        PAL                    |
| DSW2         6264                               |
| DSW2   8752               ROM9                  |
|                    ROM7   ROM8    27C160*       |
|3* 4*               ROM6   27C160* 27C160*       |
|-------------------------------------------------|

Same PCB as Cute Fighter / Dream World PCB except one OKI M6295 and it's sample rom are unpopulated

* denotes unpopulated components
  3 & 4 are 10 pin headers

Main CPU 68EC020FG16           @ 16MHz
AD-65 (OKI MSM6295 rebadged)   @ 1MHz
Atmel AT89C52 MCU (secured)    @ 16MHZ

V-SYNC                         @57.793 Hz
H-SYNC                         @15.19 - 15.27KHz (floating)

*/

ROM_START( rolcrush )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "mx27c2000_4.bin", 0x000000, 0x040000, CRC(c47f0540) SHA1(76712f41046e5852ad6be6dbf171cf34471e2409) )
	ROM_LOAD32_BYTE( "mx27c2000_3.bin", 0x000001, 0x040000, CRC(7af59294) SHA1(f36b3d100e0d963bf51b7fbe8c4a0bdcf2180ba0) )
	ROM_LOAD32_BYTE( "mx27c2000_2.bin", 0x000002, 0x040000, CRC(5eb24adb) SHA1(0329a02e18490bfe72ff34a64722d7316814720b) )
	ROM_LOAD32_BYTE( "mx27c2000_1.bin", 0x000003, 0x040000, CRC(a37e15b2) SHA1(f0fc945a894d6ed58daf05390a17051d0f3cda20) )

	ROM_REGION( 0x02000, "mcu", 0 ) /* 87C52 MCU Code */
	ROM_LOAD( "mcu.bin", 0x00000, 0x02000, BAD_DUMP CRC(9185e237) SHA1(866255242130503fa9a164645082640f1da0a8ff) ) // handcrafted from Dream World MCU using correct protection data for this game

	ROM_REGION( 0x80000, "oki1", 0 ) /* OKI Samples - 1st chip*/
	ROM_LOAD( "mx27c4000_5.bin", 0x000000, 0x80000, CRC(7afa6adb) SHA1(d4049e1068a5f7abf0e14d0b9fbbbc6dfb5d0170) )

	ROM_REGION( 0x80000, "oki2", ROMREGION_ERASE00 ) /* OKI Samples - 2nd chip (neither OKI or rom is present, empty sockets) */
	/* not populated */

	ROM_REGION( 0x400000, "gfx1", 0 ) /* Sprite Tiles - decoded */
	ROM_LOAD( "m27c160.8.bin", 0x000000, 0x200000, CRC(a509bc36) SHA1(aaa008e07e4b24ff9dbcee5925d6516d1662931c) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles - decoded */
	ROM_LOAD( "m27c160.10.bin",0x000000, 0x200000, CRC(739b0cb0) SHA1(a7cc48502d84218586afa7276fa7ba759242f05e) )

	ROM_REGION( 0x040000, "spritelut", 0 ) /* Sprite Code Lookup ... */
	ROM_LOAD16_BYTE( "tms27c010_7.bin", 0x000000, 0x020000, CRC(4cb84384) SHA1(8dd02e2d9829c15cb19654779d2217a7d53d5971) )
	ROM_LOAD16_BYTE( "tms27c010_6.bin", 0x000001, 0x020000, CRC(0c9d197a) SHA1(da057c8d08f41c4a5b9cb4f8f00de7e1461d98f0) )

	ROM_REGION( 0x10000, "unknown", 0 ) /* ???? - not decoded seems to be in blocks of 0x41 bytes.. */
	ROM_LOAD( "mx27c512.9.bin", 0x000000, 0x10000, CRC(0da8db45) SHA1(7d5bd71c5b0b28ff74c732edd7c662f46f2ab25b) )
ROM_END


ROM_START( rolcrusha )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "4", 0x000000, 0x040000, CRC(b6afbc05) SHA1(543bd3d48d03df3500f2fa2c8ef8d9e98a8ebe1f) )
	ROM_LOAD32_BYTE( "3", 0x000001, 0x040000, CRC(ecb2f9da) SHA1(6892601bd279f7968d46798db9cbbe575f63bff2) )
	ROM_LOAD32_BYTE( "2", 0x000002, 0x040000, CRC(7b291ba9) SHA1(9629c71b00317c68394b836395c3a81bdd32273a) )
	ROM_LOAD32_BYTE( "1", 0x000003, 0x040000, CRC(ef23ccf3) SHA1(14dcf8bfca991f6aa9b20236c879ae715a009ca2) )

	ROM_REGION( 0x02000, "mcu", 0 ) /* 87C52 MCU Code */
	ROM_LOAD( "mcu.bin", 0x00000, 0x02000, BAD_DUMP CRC(9185e237) SHA1(866255242130503fa9a164645082640f1da0a8ff) ) // handcrafted from Dream World MCU using correct protection data for this game

	ROM_REGION( 0x80000, "oki1", 0 ) /* OKI Samples - 1st chip*/
	ROM_LOAD( "5", 0x000000, 0x80000, CRC(7afa6adb) SHA1(d4049e1068a5f7abf0e14d0b9fbbbc6dfb5d0170) )

	ROM_REGION( 0x80000, "oki2", ROMREGION_ERASE00 ) /* OKI Samples - 2nd chip (neither OKI or rom is present, empty sockets) */
	/* not populated */

	ROM_REGION( 0x400000, "gfx1", 0 ) /* Sprite Tiles - decoded */
	ROM_LOAD( "8", 0x000000, 0x200000, CRC(01446191) SHA1(b106ed6c085fad617552972db78866a3346e4553) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles - decoded */
	ROM_LOAD( "10",0x000000, 0x200000, CRC(8cb75392) SHA1(8b274cd13876e65fffc157d8459331032c3c16db) )

	ROM_REGION( 0x040000, "spritelut", 0 ) /* Sprite Code Lookup ... */
	ROM_LOAD16_BYTE( "7", 0x000000, 0x020000, CRC(23d641e4) SHA1(1df8afb5c0118e8588d301db64f6adeb9ae40a79) )
	ROM_LOAD16_BYTE( "6", 0x000001, 0x020000, CRC(5934dac9) SHA1(2adc63746b9a921a15b8f8461af451ad82add721) )

	ROM_REGION( 0x10000, "unknown", 0 ) /* ???? - not decoded seems to be in blocks of 0x41 bytes.. */
	ROM_LOAD( "9", 0x000000, 0x10000, CRC(0da8db45) SHA1(7d5bd71c5b0b28ff74c732edd7c662f46f2ab25b) )
ROM_END



/*

Dream World
SemiCom, 2000

PCB Layout
----------

|-------------------------------------------------|
|    M6295  ROM5    62256   ACTEL           ROM10 |
|VOL M6295  ROM6    62256   A40MX04               |
|    PAL  PAL       32MHz                         |
| 62256  62256              PAL                   |
| ROM1 ROM3       68EC020   PAL    PAL            |
| ROM2 ROM4                 PAL    PAL            |
|J 62256 62256              PAL                   |
|A                          PAL    27MHz          |
|M                                 PAL            |
|M                         ACTEL    M5M44260      |
|A             6116        A40MX04  M5M44260      |
|              6116                               |
|                          PAL                    |
|              6264        PAL                    |
| DSW1         6264                               |
| DSW2   8752               ROM11                 |
|                    ROM7   ROM9    27C160*       |
|3* 4*               ROM8   27C160* 27C160*       |
|-------------------------------------------------|

* denotes unpopulated components
  3 & 4 are 10 pin headers

Notes:
      68020 @ 16.0MHz [32/2]
      M6295 (both) @ 1.0MHz [32/32]. pin 7 LOW
      8752 @ 16.0MHz [32/2]
      HSync @ 15.2kHz
      VSync @ 58Hz
*/

ROM_START( dreamwld )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "3.bin", 0x000000, 0x040000, CRC(e8f7ae78) SHA1(cfd393cec6dec967c82e1131547b7e7fdc5d814f) )
	ROM_LOAD32_BYTE( "4.bin", 0x000001, 0x040000, CRC(3ef5d51b) SHA1(82a00b4ff7155f6d5553870dfd510fed9469d9b5) )
	ROM_LOAD32_BYTE( "1.bin", 0x000002, 0x040000, CRC(35c94ee5) SHA1(3440a65a807622b619c97bc2a88fd7d875c26f66) )
	ROM_LOAD32_BYTE( "2.bin", 0x000003, 0x040000, CRC(5409e7fc) SHA1(2f94a6a8e4c94b36b43f0b94d58525f594339a9d) )

	ROM_REGION( 0x02000, "mcu", 0 ) // chip marked P87C52EBPN, die shows 87C51RA+ after decapping
	ROM_LOAD( "87c51rap.bin", 0x00000, 0x02000 , CRC(987bbfe8) SHA1(7717ed5cf97bc11c104356f6ff1d893d1606bcf0) )

	ROM_REGION( 0x80000, "oki1", 0 ) /* OKI Samples - 1st chip */
	ROM_LOAD( "5.bin", 0x000000, 0x80000, CRC(9689570a) SHA1(4414233da8f46214ca7e9022df70953922a63aa4) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* OKI Samples - 2nd chip */
	ROM_LOAD( "6.bin", 0x000000, 0x80000, CRC(c8b91f30) SHA1(706004ca56d0a74bc7a3dfd73a21cdc09eb90f05) )

	ROM_REGION( 0x400000, "gfx1", 0 ) /* Sprite Tiles - decoded */
	ROM_LOAD( "9.bin", 0x000000, 0x200000, CRC(fa84e3af) SHA1(5978737d348fd382f4ec004d29870656c864d137) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles - decoded */
	ROM_LOAD( "10.bin",0x000000, 0x200000, CRC(3553e4f5) SHA1(c335494f4a12a01a88e7cd578cae922954303cfd) )

	ROM_REGION( 0x040000, "spritelut", 0 ) /* Sprite Code Lookup ... */
	ROM_LOAD16_BYTE( "8.bin", 0x000000, 0x020000, CRC(8d570df6) SHA1(e53e4b099c64eca11d027e0083caa101fcd99959) )
	ROM_LOAD16_BYTE( "7.bin", 0x000001, 0x020000, CRC(a68bf35f) SHA1(f48540a5415a7d9723ca6e7e03cab039751dce17) )

	ROM_REGION( 0x10000, "unknown", 0 ) /* ???? - not decoded seems to be in blocks of 0x41 bytes.. */
	ROM_LOAD( "11.bin", 0x000000, 0x10000, CRC(0da8db45) SHA1(7d5bd71c5b0b28ff74c732edd7c662f46f2ab25b) )
ROM_END

/*

Gaia - The Last Choice of Earth
SemiCom / XESS, 1999

PCB Layout
----------

|-------------------------------------------------|
|    M6295* 27C40*  62256   ACTEL           ROM8  |
|VOL M6295  ROM1    62256   A40MX04               |
|    PAL  PAL       32MHz                         |
| 62256  62256              PAL                   |
| ROM2 ROM4       68EC020   PAL    PAL            |
| ROM3 ROM5                 PAL    PAL            |
|J 62256 62256              PAL                   |
|A                          PAL    27MHz          |
|M                                 PAL            |
|M                         ACTEL    M5M44260      |
|A             6116        A40MX04  M5M44260      |
|              6116                               |
|                          PAL                    |
|              6264        PAL                    |
| DSW2         6264                               |
| DSW2   8752               ROM9                  |
|                    ROM6   ROM10   ROM12         |
|3* 4*               ROM7   ROM11   27C160*       |
|-------------------------------------------------|

Same PCB as Cute Fighter / Dream World PCB except one OKI M6295 and it's sample rom are unpopulated
IE: Same configuration as Rolling Crush

No labels on any of the roms.

Program roms 2 through 5 are MX27C2000
Rom 1 is a TMS 27C040
Rom 9 is a MX26C512
Roms 6 & 7 are MX26C1000
Roms 8 and 10 through 12 are ST M27C160

* denotes unpopulated components
  3 & 4 are 10 pin headers

Main CPU 68EC020FG16           @ 16MHz [32/2]
AD-65 (OKI MSM6295 rebadged)   @ 1MHz [32/32]. pin 7 LOW
Atmel AT89C52 MCU (secured)    @ 16MHZ [32/2]

*/

ROM_START( gaialast )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "4", 0x000000, 0x040000, CRC(10cc2dee) SHA1(0719333ff391ee7935c6e4cea7e8e75369aeb9d0) )
	ROM_LOAD32_BYTE( "5", 0x000001, 0x040000, CRC(c55f6f11) SHA1(13d543b0770bebdd4c6e064b56fd6cc2ec929566) )
	ROM_LOAD32_BYTE( "2", 0x000002, 0x040000, CRC(549e594a) SHA1(728c6b51cc478ad7251bcbe6d7f4f4e6a2ee4a4e) )
	ROM_LOAD32_BYTE( "3", 0x000003, 0x040000, CRC(a8e845d8) SHA1(f8c7e702bd747a22e76c861effec4cd3cd2f3fc9) )

	ROM_REGION( 0x02000, "mcu", 0 ) /* 87C52 MCU Code */
	ROM_LOAD( "mcu.bin", 0x00000, 0x02000, BAD_DUMP CRC(fb3db92b) SHA1(3ae3649debf79a4345b05c2f4ae9674c13a66ed1) ) // handcrafted from Dream World MCU using correct protection data for this game

	ROM_REGION( 0x80000, "oki1", 0 ) /* OKI Samples */
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(2dbad410) SHA1(bb788ea14bb605be9af9c8f8adec94ad1c17ab55) )

	ROM_REGION( 0x80000, "oki2", ROMREGION_ERASE00 ) /* OKI Samples - 2nd chip (neither OKI or rom is present, empty sockets) */
	/* not populated */

	ROM_REGION( 0x800000, "gfx1", 0 ) /* Sprite Tiles - decoded */
	ROM_LOAD( "10", 0x000000, 0x200000, CRC(5822ef93) SHA1(8ce22c30f8027f35c5f72eb6ce57a74540dd55da) )
	ROM_LOAD( "11", 0x200000, 0x200000, CRC(f4f5770d) SHA1(ac850483cae321d286a09fe93ce7e49725722de0) )
	ROM_LOAD( "12", 0x400000, 0x200000, CRC(a1f04571) SHA1(c29b3b3c209b63ad44ebfa5afb4b1832965e0936) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles - decoded */
	ROM_LOAD( "8",0x000000, 0x200000, CRC(32d16985) SHA1(2b7a20eea09e7d2debd42469e9f6ae49310f5747) )

	ROM_REGION( 0x040000, "spritelut", 0 ) /* Sprite Code Lookup ... */
	ROM_LOAD16_BYTE( "6", 0x000000, 0x020000, CRC(5c82feed) SHA1(1857afecf1081adf015ade1efb5930e3a7deef78) )
	ROM_LOAD16_BYTE( "7", 0x000001, 0x020000, CRC(9d7f04ae) SHA1(55fb82626060fe0ddc03ed3ef402ccf998063d27) )

	ROM_REGION( 0x10000, "unknown", 0 )
	ROM_LOAD( "9", 0x000000, 0x10000, CRC(0da8db45) SHA1(7d5bd71c5b0b28ff74c732edd7c662f46f2ab25b) )
ROM_END

} // anonymous namespace


GAME( 1997, baryon,   0,        baryon,   baryon,   dreamwld_state, empty_init, ROT270, "SemiCom / Tirano",               "Baryon - Future Assault (set 1)",                            MACHINE_SUPPORTS_SAVE )
GAME( 1997, baryona,  baryon,   baryon,   baryon,   dreamwld_state, empty_init, ROT270, "SemiCom / Tirano",               "Baryon - Future Assault (set 2)",                            MACHINE_SUPPORTS_SAVE )
GAME( 1998, cutefght, 0,        dreamwld, cutefght, dreamwld_state, empty_init, ROT0,   "SemiCom",                        "Cute Fighter",                                               MACHINE_SUPPORTS_SAVE )
GAME( 1999, rolcrush, 0,        baryon,   rolcrush, dreamwld_state, empty_init, ROT0,   "SemiCom / Exit (Trust license)", "Rolling Crush (version 1.07.E - 1999/02/11, Trust license)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, rolcrusha,rolcrush, baryon,   rolcrush, dreamwld_state, empty_init, ROT0,   "SemiCom / Exit",                 "Rolling Crush (version 1.03.E - 1999/01/29)",                MACHINE_SUPPORTS_SAVE )
GAME( 1999, gaialast, 0,        baryon,   gaialast, dreamwld_state, empty_init, ROT0,   "SemiCom / XESS",                 "Gaia - The Last Choice of Earth",                            MACHINE_SUPPORTS_SAVE )
GAME( 2000, dreamwld, 0,        dreamwld, dreamwld, dreamwld_state, empty_init, ROT0,   "SemiCom",                        "Dream World",                                                MACHINE_SUPPORTS_SAVE )

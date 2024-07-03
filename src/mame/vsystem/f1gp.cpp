// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    F-1 Grand Prix       (c) 1991 Video System Co.

    driver by Nicola Salmoria

    Notes:
    - The ROZ layer generator is a Konami 053936.
    - f1gp2's hardware is very similar to Lethal Crash Race, main difference
      being an extra 68000.

    TODO:
    - Hook up link for Multi Player game mode. Currently they boot with
      link set to multiple, but the ID changes every boot (and will black
      out if a multiplayer game is started).

    f1gp:
    - gfxctrl register not understood - handling of fg/sprite priority to fix
      "continue" screen is just a kludge.
     f1gpb:
    - supposedly supports only steering wheel, but the emulation doesn't seem
      to work.
    f1gp2:
    - sprite lag noticeable in the animation at the end of a race (the wheels
      of the car are sprites while the car is the fg tilemap)

***************************************************************************/

#include "emu.h"

#include "vsystem_gga.h"
#include "vsystem_spr.h"
#include "vsystem_spr2.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"
#include "video/k053936.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_PRIORITIES     (1U << 1)
#define LOG_UNKSPRITEFLAGS (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_PRIORITIES | LOG_UNKSPRITEFLAGS)

#include "logmacro.h"

#define LOGPRIORITIES(...)     LOGMASKED(LOG_PRIORITIES,     __VA_ARGS__)
#define LOGUNKSPRITEFLAGS(...) LOGMASKED(LOG_UNKSPRITEFLAGS, __VA_ARGS__)


namespace {

class f1gp_state : public driver_device
{
public:
	f1gp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_sharedram(*this, "sharedram"),
		m_sprvram(*this, "spr%uvram", 1U),
		m_sprcgram(*this, "spr%ucgram", 1U),
		m_fgvideoram(*this, "fgvideoram"),
		m_rozvideoram(*this, "rozvideoram"),
		m_spriteram(*this, "spriteram"),
		m_fgregs(*this, "fgregs"),
		m_rozregs(*this, "rozregs"),
		m_z80bank(*this, "z80bank"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_audiocpu(*this, "audiocpu"),
		m_k053936(*this, "k053936"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_acia(*this, "acia"),
		m_rozgfxram(*this, "rozgfxram"),
		m_spr_old(*this, "vsystem_spr_old%u", 1U)
	{ }

	void f1gpbl(machine_config &config);
	void f1gp(machine_config &config);

protected:
	// memory pointers
	required_shared_ptr<uint16_t> m_sharedram;
	optional_shared_ptr_array<uint16_t, 2> m_sprvram;
	optional_shared_ptr_array<uint16_t, 2> m_sprcgram;
	required_shared_ptr<uint16_t> m_fgvideoram;
	required_shared_ptr<uint16_t> m_rozvideoram;
	optional_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_fgregs;
	optional_shared_ptr<uint16_t> m_rozregs;

	optional_memory_bank m_z80bank;

	// video-related
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_roz_tilemap = nullptr;
	uint8_t m_flipscreen = 0;
	uint8_t m_gfxctrl = 0;
	uint16_t m_scroll[2]{};
	template <int Chip> uint32_t tile_callback(uint32_t code);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<cpu_device> m_audiocpu;
	optional_device<k053936_device> m_k053936;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch; // not f1gpbl
	required_device<acia6850_device> m_acia;

	void sh_bankswitch_w(uint8_t data);
	uint8_t soundlatch_pending_r();
	void soundlatch_pending_w(int state);
	void rozvideoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fgvideoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fgscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gfxctrl_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void f1gp_cpu2_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);

private:
	// memory pointers
	optional_shared_ptr<uint16_t> m_rozgfxram;

	// devices
	optional_device_array<vsystem_spr2_device, 2> m_spr_old; // f1gp

	void f1gpbl_misc_w(uint16_t data);
	void rozgfxram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_roz_tile_info);

	uint32_t screen_update_f1gp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_f1gpbl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void f1gpbl_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	void f1gp_cpu1_map(address_map &map);
	void f1gpbl_cpu1_map(address_map &map);
	void f1gpbl_cpu2_map(address_map &map);
};

class f1gp2_state : public f1gp_state
{
public:
	f1gp2_state(const machine_config &mconfig, device_type type, const char *tag) :
		f1gp_state(mconfig, type, tag),
		m_spr(*this, "vsystem_spr")
	{ }

	void f1gp2(machine_config &config);

protected:
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	// video-related
	uint8_t m_roz_bank = 0;

	// devices
	optional_device<vsystem_spr_device> m_spr; // f1gp2

	void rozbank_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_roz_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void f1gp2_cpu1_map(address_map &map);
};


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(f1gp_state::get_fg_tile_info)
{
	int const code = m_fgvideoram[tile_index];

	tileinfo.set(0, code & 0x7fff, 0, (code & 0x8000) ? TILE_FLIPY : 0);
}

TILE_GET_INFO_MEMBER(f1gp_state::get_roz_tile_info)
{
	int const code = m_rozvideoram[tile_index];

	tileinfo.set(1, code & 0x7ff, code >> 12, 0);
}

TILE_GET_INFO_MEMBER(f1gp2_state::get_roz_tile_info)
{
	int const code = m_rozvideoram[tile_index];

	tileinfo.set(1, (code & 0x7ff) + (m_roz_bank << 11), code >> 12, 0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/


void f1gp_state::video_start()
{
	m_roz_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(f1gp_state::get_roz_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(f1gp_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0xff);

	save_item(NAME(m_flipscreen));
	save_item(NAME(m_gfxctrl));
	save_item(NAME(m_scroll));
}


void f1gp2_state::video_start()
{
	m_roz_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(f1gp2_state::get_roz_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(f1gp2_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0xff);
	m_roz_tilemap->set_transparent_pen(0x0f);

	m_fg_tilemap->set_scrolldx(-80, 0);
	m_fg_tilemap->set_scrolldy(-26, 0);

	save_item(NAME(m_roz_bank));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_gfxctrl));
	save_item(NAME(m_scroll));
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void f1gp_state::rozgfxram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_rozgfxram[offset]);
	m_gfxdecode->gfx(1)->mark_dirty(offset / 64);
}

void f1gp_state::rozvideoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_rozvideoram[offset]);
	m_roz_tilemap->mark_tile_dirty(offset);
}

void f1gp_state::fgvideoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fgvideoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

void f1gp_state::fgscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scroll[offset]);

	m_fg_tilemap->set_scrollx(0, m_scroll[0]);
	m_fg_tilemap->set_scrolly(0, m_scroll[1]);
}

void f1gp_state::gfxctrl_w(uint8_t data)
{
	m_flipscreen = data & 0x20;
	m_gfxctrl = data & 0xdf;
}

void f1gp2_state::rozbank_w(uint8_t data)
{
	if (m_roz_bank != data)
	{
		m_roz_bank = data;
		m_roz_tilemap->mark_all_dirty();
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/


uint32_t f1gp_state::screen_update_f1gp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_k053936->zoom_draw(screen, bitmap, cliprect, m_roz_tilemap, 0, 0, 1);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 1);

	// quick kludge for "continue" screen priority
	if (m_gfxctrl == 0x00)
	{
		m_spr_old[0]->draw_sprites(m_sprvram[0], m_sprvram[0].bytes(), 0, bitmap, cliprect, screen.priority(), 0x02);
		m_spr_old[1]->draw_sprites(m_sprvram[1], m_sprvram[1].bytes(), 0, bitmap, cliprect, screen.priority(), 0x02);
	}
	else
	{
		m_spr_old[0]->draw_sprites(m_sprvram[0], m_sprvram[0].bytes(), 0, bitmap, cliprect, screen.priority(), 0x00);
		m_spr_old[1]->draw_sprites(m_sprvram[1], m_sprvram[1].bytes(), 0, bitmap, cliprect, screen.priority(), 0x02);
	}
	return 0;
}


uint32_t f1gp2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_gfxctrl & 4)  // blank screen
		bitmap.fill(m_palette->black_pen(), cliprect);
	else
	{
		switch (m_gfxctrl & 3)
		{
			case 0:
				m_k053936->zoom_draw(screen, bitmap, cliprect, m_roz_tilemap, TILEMAP_DRAW_OPAQUE, 0, 1);
				m_spr->draw_sprites(m_sprvram[0], 0x2000, screen, bitmap, cliprect);
				m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
				break;
			case 1:
				m_k053936->zoom_draw(screen, bitmap, cliprect, m_roz_tilemap, TILEMAP_DRAW_OPAQUE, 0, 1);
				m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
				m_spr->draw_sprites(m_sprvram[0], 0x2000, screen, bitmap, cliprect);
				break;
			case 2:
				m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
				m_k053936->zoom_draw(screen, bitmap, cliprect, m_roz_tilemap, 0, 0, 1);
				m_spr->draw_sprites(m_sprvram[0], 0x2000, screen, bitmap, cliprect);
				break;
			case 3:
				LOGPRIORITIES("unsupported priority 3\n");
		}
	}
	return 0;
}

/***************************************************************************

  BOOTLEG SUPPORT

***************************************************************************/

// BOOTLEG
void f1gp_state::f1gpbl_draw_sprites(screen_device &screen,bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	int start_offset = m_spriteram.bytes() / 2 - 4;

	// find the "end of list" to draw the sprites in reverse order
	for (int attr_start = 4; attr_start < m_spriteram.bytes() / 2; attr_start += 4)
	{
		if (m_spriteram[attr_start + 3 - 4] == 0xffff) // end of list marker
		{
			start_offset = attr_start - 4;
			break;
		}
	}

	for (int attr_start = start_offset; attr_start >= 4; attr_start -= 4)
	{
		int const x = (m_spriteram[attr_start + 2] & 0x03ff) - 48;
		int const y = (256 - (m_spriteram[attr_start + 3 - 4] & 0x03ff)) - 15;
		bool const flipx = BIT(m_spriteram[attr_start + 1], 11);
		bool const flipy = BIT(m_spriteram[attr_start + 1], 15);
		int const color = m_spriteram[attr_start + 1] & 0x000f;
		int code = m_spriteram[attr_start + 0] & 0x3fff;
		int const pri = 0; //?

		if ((m_spriteram[attr_start + 1] & 0x00f0) && (m_spriteram[attr_start + 1] & 0x00f0) != 0xc0)
		{
			LOGUNKSPRITEFLAGS("attr %X\n", m_spriteram[attr_start + 1] & 0x00f0);
			code = machine().rand();
		}

		if (m_spriteram[attr_start + 1] & ~0x88cf)
			LOGUNKSPRITEFLAGS("1 = %X\n", m_spriteram[attr_start + 1] & ~0x88cf);

		int gfx;

		if (code >= 0x2000)
		{
			gfx = 1;
			code -= 0x2000;
		}
		else
		{
			gfx = 0;
		}

		m_gfxdecode->gfx(2 + gfx)->prio_transpen(bitmap, cliprect,
			code,
			color,
			flipx, flipy,
			x, y,
			screen.priority(),
			pri ? 0 : 0x2, 15);

		// wrap around x
		m_gfxdecode->gfx(2 + gfx)->prio_transpen(bitmap, cliprect,
			code,
			color,
			flipx, flipy,
			x - 512, y,
			screen.priority(),
			pri ? 0 : 0x2, 15);
	}
}

// BOOTLEG
uint32_t f1gp_state::screen_update_f1gpbl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int const incxy = (int16_t)m_rozregs[1];
	int const incyx = -incxy;
	int const incxx = (int16_t)m_rozregs[3];
	int const incyy = incxx;
	uint32_t const startx = m_rozregs[0] + 328;
	uint32_t const starty = m_rozregs[2];

	m_fg_tilemap->set_scrolly(0, m_fgregs[0] + 8);

	screen.priority().fill(0, cliprect);

	m_roz_tilemap->draw_roz(screen, bitmap, cliprect,
		startx << 13, starty << 13,
		incxx << 5, incxy << 5, incyx << 5, incyy << 5,
		1, 0, 0);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 1);

	f1gpbl_draw_sprites(screen, bitmap, cliprect);

	return 0;
}


void f1gp_state::sh_bankswitch_w(uint8_t data)
{
	m_z80bank->set_entry(data & 0x01);
}

uint8_t f1gp_state::soundlatch_pending_r()
{
	return (m_soundlatch->pending_r() ? 0xff : 0);
}

void f1gp_state::soundlatch_pending_w(int state)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);

	// sound comms is 2-way (see soundlatch_pending_r),
	// NMI routine is very short, so briefly set perfect_quantum to make sure that the timing is right
	if (state)
		machine().scheduler().perfect_quantum(attotime::from_usec(100));
}


void f1gp_state::f1gp_cpu1_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x2fffff).rom().region("user1", 0);
	map(0xa00000, 0xbfffff).rom().region("user2", 0);
	map(0xc00000, 0xc3ffff).ram().w(FUNC(f1gp_state::rozgfxram_w)).share(m_rozgfxram);
	map(0xd00000, 0xd01fff).mirror(0x006000).ram().w(FUNC(f1gp_state::rozvideoram_w)).share(m_rozvideoram);
	map(0xe00000, 0xe03fff).ram().share(m_sprcgram[0]);                                         // SPR-1 CG RAM
	map(0xe04000, 0xe07fff).ram().share(m_sprcgram[1]);                                         // SPR-2 CG RAM
	map(0xf00000, 0xf003ff).ram().share(m_sprvram[0]);                                          // SPR-1 VRAM
	map(0xf10000, 0xf103ff).ram().share(m_sprvram[1]);                                          // SPR-2 VRAM
	map(0xff8000, 0xffbfff).ram();                                                              // WORK RAM-1
	map(0xffc000, 0xffcfff).ram().share(m_sharedram);                                           // DUAL RAM
	map(0xffd000, 0xffdfff).ram().w(FUNC(f1gp_state::fgvideoram_w)).share(m_fgvideoram);        // CHARACTER
	map(0xffe000, 0xffefff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); // PALETTE
	map(0xfff000, 0xfff001).portr("INPUTS");
	map(0xfff001, 0xfff001).w(FUNC(f1gp_state::gfxctrl_w));
	map(0xfff002, 0xfff003).portr("WHEEL");
	map(0xfff004, 0xfff005).portr("DSW1");
	map(0xfff002, 0xfff005).w(FUNC(f1gp_state::fgscroll_w));
	map(0xfff006, 0xfff007).portr("DSW2");
	map(0xfff009, 0xfff009).r(FUNC(f1gp_state::soundlatch_pending_r)).w(m_soundlatch, FUNC(generic_latch_8_device::write)).umask16(0x00ff);
	map(0xfff020, 0xfff023).w("gga", FUNC(vsystem_gga_device::write)).umask16(0x00ff);
	map(0xfff040, 0xfff05f).w(m_k053936, FUNC(k053936_device::ctrl_w));
	map(0xfff050, 0xfff051).portr("DSW3");
}

void f1gp2_state::f1gp2_cpu1_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x2fffff).rom().region("user1", 0);
	map(0xa00000, 0xa07fff).ram().share(m_sprcgram[0]);                                         // SPR-1 CG RAM + SPR-2 CG RAM
	map(0xd00000, 0xd01fff).ram().w(FUNC(f1gp2_state::rozvideoram_w)).share(m_rozvideoram);     // BACK VRAM
	map(0xe00000, 0xe00fff).ram().share(m_sprvram[0]);                                          // not checked + SPR-1 VRAM + SPR-2 VRAM
	map(0xff8000, 0xffbfff).ram();                                                              // WORK RAM-1
	map(0xffc000, 0xffcfff).ram().share(m_sharedram);                                           // DUAL RAM
	map(0xffd000, 0xffdfff).ram().w(FUNC(f1gp2_state::fgvideoram_w)).share(m_fgvideoram);       // CHARACTER
	map(0xffe000, 0xffefff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); // PALETTE
	map(0xfff000, 0xfff001).portr("INPUTS");
	map(0xfff000, 0xfff000).w(FUNC(f1gp2_state::rozbank_w));
	map(0xfff001, 0xfff001).w(FUNC(f1gp2_state::gfxctrl_w));
	map(0xfff002, 0xfff003).portr("WHEEL");
	map(0xfff004, 0xfff005).portr("DSW1");
	map(0xfff006, 0xfff007).portr("DSW2");
	map(0xfff009, 0xfff009).r(FUNC(f1gp2_state::soundlatch_pending_r)).w(m_soundlatch, FUNC(generic_latch_8_device::write)).umask16(0x00ff);
	map(0xfff00a, 0xfff00b).portr("DSW3");
	map(0xfff020, 0xfff03f).w(m_k053936, FUNC(k053936_device::ctrl_w));
	map(0xfff044, 0xfff047).w(FUNC(f1gp2_state::fgscroll_w));
}

void f1gp_state::f1gp_cpu2_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0xff8000, 0xffbfff).ram();
	map(0xffc000, 0xffcfff).ram().share(m_sharedram);
	map(0xfff030, 0xfff033).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
}

void f1gp_state::sound_map(address_map &map)
{
	map(0x0000, 0x77ff).rom();
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).bankr(m_z80bank);
}

void f1gp_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(f1gp_state::sh_bankswitch_w)); // f1gp
	map(0x0c, 0x0c).w(FUNC(f1gp_state::sh_bankswitch_w)); // f1gp2
	map(0x14, 0x14).rw(m_soundlatch, FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::acknowledge_w));
	map(0x18, 0x1b).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
}

void f1gp_state::f1gpbl_misc_w(uint16_t data)
{
	/*
	static int old=-1;
	static int old_bank = -1;
	int new_bank = (data & 0xf0) >> 4; //wrong!

	if(old_bank != new_bank && new_bank < 5)
	{
	    // oki banking
	    uint8_t *src = memregion("oki")->base() + 0x40000 + 0x10000 * new_bank;
	    uint8_t *dst = memregion("oki")->base() + 0x30000;
	    memcpy(dst, src, 0x10000);

	    old_bank = new_bank;
	}

	//data & 0x80 toggles

	if((data & 0x7f) != old)
	    printf("misc = %X\n",old=data & 0x7f);

	*/
}

void f1gp_state::f1gpbl_cpu1_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x2fffff).rom().region("user1", 0);
	map(0xa00000, 0xbfffff).rom().region("user2", 0);
	map(0x800000, 0x801fff).ram().share(m_spriteram);
	map(0xc00000, 0xc3ffff).ram().w(FUNC(f1gp_state::rozgfxram_w)).share(m_rozgfxram);
	map(0xd00000, 0xd01fff).mirror(0x006000).ram().w(FUNC(f1gp_state::rozvideoram_w)).share(m_rozvideoram);
	map(0xe00000, 0xe03fff).ram(); //unused
	map(0xe04000, 0xe07fff).ram(); //unused
	map(0xf00000, 0xf003ff).ram(); //unused
	map(0xf10000, 0xf103ff).ram(); //unused
	map(0xff8000, 0xffbfff).ram();
	map(0xffc000, 0xffcfff).ram().share(m_sharedram);
	map(0xffd000, 0xffdfff).ram().w(FUNC(f1gp_state::fgvideoram_w)).share(m_fgvideoram);
	map(0xffe000, 0xffefff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xfff000, 0xfff001).portr("INPUTS");
	map(0xfff002, 0xfff003).portr("WHEEL");
	map(0xfff004, 0xfff005).portr("DSW1");
	map(0xfff006, 0xfff007).portr("DSW2");
	map(0xfff008, 0xfff009).nopr(); //?
	map(0xfff006, 0xfff007).nopw();
	map(0xfff00a, 0xfff00b).ram().share(m_fgregs);
	map(0xfff00f, 0xfff00f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xfff00c, 0xfff00d).w(FUNC(f1gp_state::f1gpbl_misc_w));
	map(0xfff010, 0xfff011).nopw();
	map(0xfff020, 0xfff023).nopw(); // GGA access
	map(0xfff050, 0xfff051).portr("DSW3");
	map(0xfff800, 0xfff809).ram().share(m_rozregs);
}

void f1gp_state::f1gpbl_cpu2_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0xff8000, 0xffbfff).ram();
	map(0xffc000, 0xffcfff).ram().share(m_sharedram);
	map(0xfff030, 0xfff033).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
}

static INPUT_PORTS_START( f1gp )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_CONDITION("JOY_TYPE", 0x01, NOTEQUALS, 0x01)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_CONDITION("JOY_TYPE", 0x01, NOTEQUALS, 0x01)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_CONDITION("JOY_TYPE", 0x01, NOTEQUALS, 0x01)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_CONDITION("JOY_TYPE", 0x01, NOTEQUALS, 0x01)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("JOY_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("JOY_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_CONDITION("JOY_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_CONDITION("JOY_TYPE", 0x01, EQUALS, 0x01)
	// following two are logically arranged as per service mode
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Brake Button")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Accelerator Button")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("WHEEL")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(5)

	PORT_START("DSW1")
	// listed as "unused" in manual, actually enables free play
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Free_Play ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Coin_A ) )       PORT_CONDITION("DSW1", 0x8000, EQUALS, 0x8000) PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0a00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x7000, 0x7000, DEF_STR( Coin_B ) )       PORT_CONDITION("DSW1", 0x8000, EQUALS, 0x8000) PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(      0x5000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x7e00, 0x7e00, DEF_STR( Coinage ) )      PORT_CONDITION("DSW1", 0x8000, NOTEQUALS, 0x8000) PORT_DIPLOCATION("SW1:2,3,4,5,6,7")
	PORT_DIPSETTING(      0x7e00, "2 to Start, 1 to Continue" )
	PORT_DIPNAME( 0x8000, 0x8000, "Continue Coin" )         PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x8000, "Normal Coinage" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Game Mode" )         PORT_DIPLOCATION("SW2:3") // Setting to Multiple results in "ID CHECK   ERROR" then hang
	PORT_DIPSETTING(      0x0004, DEF_STR( Single ) )
	PORT_DIPSETTING(      0x0000, "Multiple" )
	PORT_DIPNAME( 0x0008, 0x0008, "Multi Player Mode" )     PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0008, "Single or Multi Player" )        PORT_CONDITION("DSW1", 0x0004, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, "Multi Player Game Only" )        PORT_CONDITION("DSW1", 0x0004, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0008, "Multi Player Off" )          PORT_CONDITION("DSW1", 0x0004, NOTEQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, "Multi Player Off" )          PORT_CONDITION("DSW1", 0x0004, NOTEQUALS, 0x0000)
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" )        // Listed as "Unused", reverses joystick left/right directions?
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" )        // Listed as "Unused", two buttons to accelerate?
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" )        // Listed as "Unused", reverses button activeness?
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW2:8" )        // Listed as "Unused"

	PORT_START("DSW2")
	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_LOW, "SW3:1" )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW3:4" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW3:5" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW3:6" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW3:7" )        // Listed as "Unused"
	// listed as "Unused" in manual, it selects between joystick or steering wheel
	PORT_DIPNAME( 0x8000, 0x8000, "Input Method" )      PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x8000, "Joystick" )
	// TODO: doesn't work in-game, reads from $fff002 ingame too but doesn't have an effect,
	//       maybe outputs threshold to $fff000 or it's not supposed to be enabled like the manual claims.
	PORT_DIPSETTING(      0x0000, "Steering Wheel" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x001f, 0x0010, DEF_STR( Region ) )           // Jumpers??
	PORT_DIPSETTING(      0x0010, DEF_STR( World ) )
	PORT_DIPSETTING(      0x0001, "USA & Canada" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japan ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Korea ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Taiwan ) )
	// all other values are invalid

	PORT_START("JOY_TYPE")
	PORT_CONFNAME( 0x01, 0x01, "Joystick Type" )
	PORT_CONFSETTING(    0x01, "2-Way" )
	// in "free run" course select lets you go up/down
	PORT_CONFSETTING(    0x00, "4-Way" )
INPUT_PORTS_END


static INPUT_PORTS_START( f1gp2 )
	PORT_INCLUDE( f1gp )

	PORT_MODIFY("INPUTS")
	// additional button, complies with 1992 F1 season
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Turbo Button")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Region ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( World ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japan ) )
	PORT_DIPUNUSED( 0x001e, 0x001e )
INPUT_PORTS_END



static GFXDECODE_START( gfx_f1gp )
	GFXDECODE_ENTRY( "fgtiles",  0, gfx_8x8x8_raw,          0x000,  1 )
	GFXDECODE_RAM( "rozgfxram",  0, gfx_16x16x4_packed_msb, 0x300, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_f1gp_spr1 )
	GFXDECODE_ENTRY( "sprites1", 0, gfx_16x16x4_packed_lsb, 0x100, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_f1gp_spr2 )
	GFXDECODE_ENTRY( "sprites2", 0, gfx_16x16x4_packed_lsb, 0x200, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_f1gpbl )
	GFXDECODE_ENTRY( "fgtiles",  0, gfx_8x8x8_raw,          0x000,  1 )
	GFXDECODE_RAM( "rozgfxram",  0, gfx_16x16x4_packed_msb, 0x300, 16 )
	GFXDECODE_ENTRY( "sprites1", 0, gfx_16x16x4_packed_lsb, 0x100, 16 )
	GFXDECODE_ENTRY( "sprites2", 0, gfx_16x16x4_packed_lsb, 0x200, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_f1gp2 )
	GFXDECODE_ENTRY( "fgtiles",  0, gfx_8x8x8_raw,          0x000,  1 )
	GFXDECODE_ENTRY( "roztiles", 0, gfx_16x16x4_packed_msb, 0x100, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_f1gp2_spr )
	GFXDECODE_ENTRY( "sprites1", 0, gfx_16x16x4_packed_lsb, 0x200, 32 )
GFXDECODE_END


void f1gp_state::machine_start()
{
	// assumes it can make an address mask with .length() - 1
	assert(!m_sprcgram[0].found() || !(m_sprcgram[0].length() & (m_sprcgram[0].length() - 1)));
	assert(!m_sprcgram[1].found() || !(m_sprcgram[1].length() & (m_sprcgram[1].length() - 1)));

	if (m_z80bank)
		m_z80bank->configure_entries(0, 2, memregion("audiocpu")->base() + 0x8000, 0x8000);

	m_acia->write_cts(0);
	m_acia->write_dcd(0);
}

void f1gp_state::machine_reset()
{
	m_flipscreen = 0;
	m_gfxctrl = 0;
	m_scroll[0] = 0;
	m_scroll[1] = 0;
}

void f1gp2_state::machine_reset()
{
	f1gp_state::machine_reset();
	m_roz_bank = 0;
}

template <int Chip>
uint32_t f1gp_state::tile_callback(uint32_t code)
{
	return m_sprcgram[Chip][code & (m_sprcgram[Chip].length() - 1)];
}

void f1gp_state::f1gp(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(20'000'000) / 2); // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &f1gp_state::f1gp_cpu1_map);
	m_maincpu->set_vblank_int("screen", FUNC(f1gp_state::irq1_line_hold));

	m68000_device &sub(M68000(config, "sub", XTAL(20'000'000) / 2));    // verified on PCB
	sub.set_addrmap(AS_PROGRAM, &f1gp_state::f1gp_cpu2_map);
	sub.set_vblank_int("screen", FUNC(f1gp_state::irq1_line_hold));

	Z80(config, m_audiocpu, XTAL(20'000'000) / 4);  // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &f1gp_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &f1gp_state::sound_io_map);

	config.set_maximum_quantum(attotime::from_hz(6'000)); // 100 CPU slices per frame

	ACIA6850(config, m_acia, 0);
	m_acia->irq_handler().set_inputline("sub", M68K_IRQ_3);
	m_acia->txd_handler().set("acia", FUNC(acia6850_device::write_rxd)); // loopback for now

	clock_device &acia_clock(CLOCK(config, "acia_clock", 1'000'000)); // guessed
	acia_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(f1gp_state::screen_update_f1gp));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_f1gp);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 2048);

	VSYSTEM_GGA(config, "gga", XTAL(14'318'181) / 2); // divider not verified

	VSYSTEM_SPR2(config, m_spr_old[0], 0, m_palette, gfx_f1gp_spr1);
	m_spr_old[0]->set_tile_indirect_cb(FUNC(f1gp2_state::tile_callback<0>));
	m_spr_old[0]->set_pritype(2);

	VSYSTEM_SPR2(config, m_spr_old[1], 0, m_palette, gfx_f1gp_spr2);
	m_spr_old[1]->set_tile_indirect_cb(FUNC(f1gp2_state::tile_callback<1>));
	m_spr_old[1]->set_pritype(2);

	K053936(config, m_k053936, 0);
	m_k053936->set_wrap(1);
	m_k053936->set_offsets(-58, -2);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set(FUNC(f1gp_state::soundlatch_pending_w));
	m_soundlatch->set_separate_acknowledge(true);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(8'000'000)));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);
}

void f1gp_state::f1gpbl(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 10'000'000); // 10 MHz ???
	m_maincpu->set_addrmap(AS_PROGRAM, &f1gp_state::f1gpbl_cpu1_map);
	m_maincpu->set_vblank_int("screen", FUNC(f1gp_state::irq1_line_hold));

	m68000_device &sub(M68000(config, "sub", 10'000'000));    // 10 MHz ???
	sub.set_addrmap(AS_PROGRAM, &f1gp_state::f1gpbl_cpu2_map);
	sub.set_vblank_int("screen", FUNC(f1gp_state::irq1_line_hold));

	// NO sound CPU
	config.set_maximum_quantum(attotime::from_hz(6'000)); // 100 CPU slices per frame

	ACIA6850(config, m_acia, 0);
	m_acia->irq_handler().set_inputline("sub", M68K_IRQ_3);
	m_acia->txd_handler().set("acia", FUNC(acia6850_device::write_rxd)); // loopback for now

	clock_device &acia_clock(CLOCK(config, "acia_clock", 1'000'000)); // guessed
	acia_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(f1gp_state::screen_update_f1gpbl));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_f1gpbl);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 2048);

	//VSYSTEM_GGA(config, "gga", 0);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", 1'000'000, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "mono", 1.00);
}

void f1gp2_state::f1gp2(machine_config &config)
{
	f1gp_state::f1gp(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &f1gp2_state::f1gp2_cpu1_map);

	// video hardware
	m_gfxdecode->set_info(gfx_f1gp2);

	subdevice<screen_device>("screen")->set_visarea(0*8, 40*8-1, 0*8, 28*8-1);
	subdevice<screen_device>("screen")->set_screen_update(FUNC(f1gp2_state::screen_update));

	config.device_remove("gga");
	config.device_remove("vsystem_spr_old1");
	config.device_remove("vsystem_spr_old2");

	VSYSTEM_SPR(config, m_spr, 0, m_palette, gfx_f1gp2_spr);
	m_spr->set_tile_indirect_cb(FUNC(f1gp2_state::tile_callback<0>));

	m_k053936->set_offsets(-48, -21);
}


ROM_START( f1gp )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "v1.rom1.rom", 0x00000, 0x40000, CRC(56c8eccd) SHA1(2b55f13df83761ea2d1f9cc64519c35a9ff64c6e) ) // 27C2048 is correct, game data extends to 0x28ADE then 0xFF filled

	ROM_REGION16_BE( 0x200000, "user1", 0 )  // extra ROMs mapped at 100000
	ROM_LOAD16_BYTE( "rom10-a.1", 0x000000, 0x40000, CRC(46a289fb) SHA1(6a8c19e08b6d836fe83378fd77fead82a0b2db7c) )
	ROM_LOAD16_BYTE( "rom11-a.2", 0x000001, 0x40000, CRC(53df8ea1) SHA1(25d50bb787f3bd35c9a8ae2b0ab9a21e000debb0) )
	ROM_LOAD16_BYTE( "rom12-a.3", 0x080000, 0x40000, CRC(d8c1bcf4) SHA1(d6d77354eb1ab413ba8cfa5d973cf5b0c851c23b) )
	ROM_LOAD16_BYTE( "rom13-a.4", 0x080001, 0x40000, CRC(7d92e1fa) SHA1(c23f5beea85b0804c61ef9e7f131b186d076221f) )
	ROM_LOAD16_BYTE( "rom7-a.5",  0x100000, 0x40000, CRC(7a014ba6) SHA1(8f0abbb68100e396e5a41337254cb6bf1a2ed00b) )
	ROM_LOAD16_BYTE( "rom6-a.6",  0x100001, 0x40000, CRC(6d947a3f) SHA1(2cd01ee2a73ab105a45a5464a29fd75aa43ba2db) )
	ROM_LOAD16_BYTE( "rom8-a.7",  0x180000, 0x40000, CRC(0ed783c7) SHA1(c0c467ede51c08d84999897c6d5cc8b584b23b67) )
	ROM_LOAD16_BYTE( "rom9-a.8",  0x180001, 0x40000, CRC(49286572) SHA1(c5e16bd1ccd43452337a4cd76db70db079ca0706) )

	ROM_REGION16_BE( 0x200000, "user2", 0 )  // extra ROMs mapped at a00000 containing gfx data for the 053936
	ROM_LOAD16_WORD_SWAP( "rom2-a.06", 0x000000, 0x100000, CRC(747dd112) SHA1(b9264bec61467ab256cf6cb698b6e0ea8f8006e0) )
	ROM_LOAD16_WORD_SWAP( "rom3-a.05", 0x100000, 0x100000, CRC(264aed13) SHA1(6f0de860d4299befffc530b7a8f19656982a51c4) )

	ROM_REGION( 0x20000, "sub", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "rom4-a.4", 0x00000, 0x20000, CRC(8e811d36) SHA1(2b806b50a3a307a21894687f16485ace287a7c4c) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "rom5-a.8", 0x00000, 0x20000, CRC(9ea36e35) SHA1(9254dea8362318d8cfbd5e36e476e0e235e6326a) )

	ROM_REGION( 0x200000, "fgtiles", 0 )
	ROM_LOAD( "rom3-b.07", 0x000000, 0x100000, CRC(ffb1d489) SHA1(9330b67e0eaaf67d6c38f40a02c72419bd38fb81) )
	ROM_LOAD( "rom2-b.04", 0x100000, 0x100000, CRC(d1b3471f) SHA1(d1a95fbaad1c3d9ec2121bf65abbcdb5441bd0ac) )

	ROM_REGION( 0x100000, "sprites1", 0 )
	ROM_LOAD32_WORD( "rom5-b.2", 0x00000, 0x80000, CRC(17572b36) SHA1(c58327c2f708783a3e8470e290cae0d71454f1da) )
	ROM_LOAD32_WORD( "rom4-b.3", 0x00002, 0x80000, CRC(72d12129) SHA1(11da6990a54ae1b6f6d0bed5d0431552f83a0dda) )

	ROM_REGION( 0x080000, "sprites2", 0 )
	ROM_LOAD32_WORD( "rom7-b.17", 0x00000, 0x40000, CRC(2aed9003) SHA1(45ff9953ad98063573e7fd7b930ae8b0183cdd04) )
	ROM_LOAD32_WORD( "rom6-b.16", 0x00002, 0x40000, CRC(6789ef12) SHA1(9b0d1cc6e9c6398ccb7f635c4c148fddd224a21f) )

	ROM_REGION( 0x100000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "rom14-a.09",   0x000000, 0x100000, CRC(b4c1ac31) SHA1(acab2e1b5ce4ca3a5c4734562481b54db4b46995) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "rom17-a.08",   0x000000, 0x100000, CRC(ea70303d) SHA1(8de1a0e6d47cd80a622663c1745a1da54cd0ea05) )
ROM_END

ROM_START( f1gpa )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "rom1-a.3", 0x00000, 0x20000, CRC(2d8f785b) SHA1(6eca42ad2d57a31e055496141c89cb537f284378) ) // 27C1024 is correct, game data extends to 0x195AC then 0xFF filled

	ROM_REGION16_BE( 0x200000, "user1", 0 )  // extra ROMs mapped at 100000
	ROM_LOAD16_BYTE( "rom10-a.1", 0x000000, 0x40000, CRC(46a289fb) SHA1(6a8c19e08b6d836fe83378fd77fead82a0b2db7c) )
	ROM_LOAD16_BYTE( "rom11-a.2", 0x000001, 0x40000, CRC(53df8ea1) SHA1(25d50bb787f3bd35c9a8ae2b0ab9a21e000debb0) )
	ROM_LOAD16_BYTE( "rom12-a.3", 0x080000, 0x40000, CRC(d8c1bcf4) SHA1(d6d77354eb1ab413ba8cfa5d973cf5b0c851c23b) )
	ROM_LOAD16_BYTE( "rom13-a.4", 0x080001, 0x40000, CRC(7d92e1fa) SHA1(c23f5beea85b0804c61ef9e7f131b186d076221f) )
	ROM_LOAD16_BYTE( "rom7-a.5",  0x100000, 0x40000, CRC(7a014ba6) SHA1(8f0abbb68100e396e5a41337254cb6bf1a2ed00b) )
	ROM_LOAD16_BYTE( "rom6-a.6",  0x100001, 0x40000, CRC(6d947a3f) SHA1(2cd01ee2a73ab105a45a5464a29fd75aa43ba2db) )
	ROM_LOAD16_BYTE( "rom8-a.7",  0x180000, 0x40000, CRC(0ed783c7) SHA1(c0c467ede51c08d84999897c6d5cc8b584b23b67) )
	ROM_LOAD16_BYTE( "rom9-a.8",  0x180001, 0x40000, CRC(49286572) SHA1(c5e16bd1ccd43452337a4cd76db70db079ca0706) )

	ROM_REGION16_BE( 0x200000, "user2", 0 )  // extra ROMs mapped at a00000 containing gfx data for the 053936
	ROM_LOAD16_WORD_SWAP( "rom2-a.06", 0x000000, 0x100000, CRC(747dd112) SHA1(b9264bec61467ab256cf6cb698b6e0ea8f8006e0) )
	ROM_LOAD16_WORD_SWAP( "rom3-a.05", 0x100000, 0x100000, CRC(264aed13) SHA1(6f0de860d4299befffc530b7a8f19656982a51c4) )

	ROM_REGION( 0x20000, "sub", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "rom4-a.4", 0x00000, 0x20000, CRC(8e811d36) SHA1(2b806b50a3a307a21894687f16485ace287a7c4c) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    // 64k for the audio CPU + banks
	ROM_LOAD( "rom5-a.8", 0x00000, 0x20000, CRC(9ea36e35) SHA1(9254dea8362318d8cfbd5e36e476e0e235e6326a) )

	ROM_REGION( 0x200000, "fgtiles", 0 )
	ROM_LOAD( "rom3-b.07", 0x000000, 0x100000, CRC(ffb1d489) SHA1(9330b67e0eaaf67d6c38f40a02c72419bd38fb81) )
	ROM_LOAD( "rom2-b.04", 0x100000, 0x100000, CRC(d1b3471f) SHA1(d1a95fbaad1c3d9ec2121bf65abbcdb5441bd0ac) )

	ROM_REGION( 0x100000, "sprites1", 0 )
	ROM_LOAD32_WORD( "rom5-b.2", 0x00000, 0x80000, CRC(17572b36) SHA1(c58327c2f708783a3e8470e290cae0d71454f1da) )
	ROM_LOAD32_WORD( "rom4-b.3", 0x00002, 0x80000, CRC(72d12129) SHA1(11da6990a54ae1b6f6d0bed5d0431552f83a0dda) )

	ROM_REGION( 0x080000, "sprites2", 0 )
	ROM_LOAD32_WORD( "rom7-b.17", 0x00000, 0x40000, CRC(2aed9003) SHA1(45ff9953ad98063573e7fd7b930ae8b0183cdd04) )
	ROM_LOAD32_WORD( "rom6-b.16", 0x00002, 0x40000, CRC(6789ef12) SHA1(9b0d1cc6e9c6398ccb7f635c4c148fddd224a21f) )

	ROM_REGION( 0x100000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "rom14-a.09", 0x000000, 0x100000, CRC(b4c1ac31) SHA1(acab2e1b5ce4ca3a5c4734562481b54db4b46995) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "rom17-a.08", 0x000000, 0x100000, CRC(ea70303d) SHA1(8de1a0e6d47cd80a622663c1745a1da54cd0ea05) )
ROM_END

ROM_START( f1gpb ) // 0F17-A-04 PCB
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "rom1.3", 0x00000, 0x40000, CRC(5d945bcd) SHA1(8dc1cce2d0eac21ec4dd716b35944ae67d35ced0) )

	ROM_REGION16_BE( 0x200000, "user1", 0 )  // extra ROMs mapped at 100000
	ROM_LOAD16_BYTE( "rom10-a.1", 0x000000, 0x40000, CRC(46a289fb) SHA1(6a8c19e08b6d836fe83378fd77fead82a0b2db7c) )
	ROM_LOAD16_BYTE( "rom11-a.2", 0x000001, 0x40000, CRC(53df8ea1) SHA1(25d50bb787f3bd35c9a8ae2b0ab9a21e000debb0) )
	ROM_LOAD16_BYTE( "rom12-a.3", 0x080000, 0x40000, CRC(d8c1bcf4) SHA1(d6d77354eb1ab413ba8cfa5d973cf5b0c851c23b) )
	ROM_LOAD16_BYTE( "rom13-a.4", 0x080001, 0x40000, CRC(7d92e1fa) SHA1(c23f5beea85b0804c61ef9e7f131b186d076221f) )
	ROM_LOAD16_BYTE( "rom7-a.5",  0x100000, 0x40000, CRC(7a014ba6) SHA1(8f0abbb68100e396e5a41337254cb6bf1a2ed00b) )
	ROM_LOAD16_BYTE( "rom6-a.6",  0x100001, 0x40000, CRC(6d947a3f) SHA1(2cd01ee2a73ab105a45a5464a29fd75aa43ba2db) )
	ROM_LOAD16_BYTE( "rom8-a.7",  0x180000, 0x40000, CRC(0ed783c7) SHA1(c0c467ede51c08d84999897c6d5cc8b584b23b67) )
	ROM_LOAD16_BYTE( "rom9-a.8",  0x180001, 0x40000, CRC(49286572) SHA1(c5e16bd1ccd43452337a4cd76db70db079ca0706) )

	ROM_REGION16_BE( 0x200000, "user2", 0 )  // extra ROMs mapped at a00000 containing gfx data for the 053936
	ROM_LOAD16_WORD_SWAP( "rom2-a.06", 0x000000, 0x100000, CRC(747dd112) SHA1(b9264bec61467ab256cf6cb698b6e0ea8f8006e0) )
	ROM_LOAD16_WORD_SWAP( "rom3-a.05", 0x100000, 0x100000, CRC(264aed13) SHA1(6f0de860d4299befffc530b7a8f19656982a51c4) )

	ROM_REGION( 0x20000, "sub", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "rom4.4", 0x00000, 0x20000, CRC(8e811d36) SHA1(2b806b50a3a307a21894687f16485ace287a7c4c) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    // 64k for the audio CPU + banks
	ROM_LOAD( "rom5.8", 0x00000, 0x20000, CRC(9ea36e35) SHA1(9254dea8362318d8cfbd5e36e476e0e235e6326a) )

	ROM_REGION( 0x200000, "fgtiles", 0 )
	ROM_LOAD( "rom3-b.07", 0x000000, 0x100000, CRC(ffb1d489) SHA1(9330b67e0eaaf67d6c38f40a02c72419bd38fb81) )
	ROM_LOAD( "rom2-b.04", 0x100000, 0x100000, CRC(d1b3471f) SHA1(d1a95fbaad1c3d9ec2121bf65abbcdb5441bd0ac) )

	ROM_REGION( 0x100000, "sprites1", 0 )
	ROM_LOAD32_WORD( "rom5-b.2", 0x00000, 0x80000, CRC(17572b36) SHA1(c58327c2f708783a3e8470e290cae0d71454f1da) )
	ROM_LOAD32_WORD( "rom4-b.3", 0x00002, 0x80000, CRC(72d12129) SHA1(11da6990a54ae1b6f6d0bed5d0431552f83a0dda) )

	ROM_REGION( 0x080000, "sprites2", 0 )
	ROM_LOAD32_WORD( "rom7-b.17", 0x00000, 0x40000, CRC(2aed9003) SHA1(45ff9953ad98063573e7fd7b930ae8b0183cdd04) )
	ROM_LOAD32_WORD( "rom6-b.16", 0x00002, 0x40000, CRC(6789ef12) SHA1(9b0d1cc6e9c6398ccb7f635c4c148fddd224a21f) )

	ROM_REGION( 0x100000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "rom14-a.09", 0x000000, 0x100000, CRC(b4c1ac31) SHA1(acab2e1b5ce4ca3a5c4734562481b54db4b46995) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "rom17-a.08", 0x000000, 0x100000, CRC(ea70303d) SHA1(8de1a0e6d47cd80a622663c1745a1da54cd0ea05) )
ROM_END

/* This is a bootleg of f1gp, produced by Playmark in Italy
   the video hardware is different, it lacks the sound z80, and has less samples
 */

ROM_START( f1gpbl )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	// these have extra data at 0x30000 which isn't preset in the f1gp set, is it related to the changed sound hardware?
	ROM_LOAD16_BYTE( "1.ic38", 0x00001, 0x20000, CRC(046dd83a) SHA1(ea65fa88f9d9a79664de666e63594a7a7de86650) )
	ROM_LOAD16_BYTE( "7.ic39", 0x00000, 0x20000, CRC(960f5db4) SHA1(addc461538e2140afae400e8d7364d0bcc42a0cb) )

	ROM_REGION16_BE( 0x200000, "user1", 0 )  // extra ROMs mapped at 100000
	ROM_LOAD16_BYTE( "8.ic41",  0x000000, 0x80000, CRC(39af8180) SHA1(aa1577195b1463069870db2d64db3b5e61d6bbe8) )
	ROM_LOAD16_BYTE( "2.ic48",  0x000001, 0x80000, CRC(b3b315c3) SHA1(568592e450401cd95206dbe439e565dd28499dd1) )
	ROM_LOAD16_BYTE( "9.ic166", 0x100000, 0x80000, CRC(bb596d5b) SHA1(f29ed135e8f09d4a15353360a811c13aba681382) )
	ROM_LOAD16_BYTE( "3.ic165", 0x100001, 0x80000, CRC(b7295a30) SHA1(4120dda38673d59343aea0f030d2f275a0ae3d95) )

	ROM_REGION16_BE( 0x200000, "user2", 0 )  // extra ROMs mapped at a00000
	ROM_LOAD16_BYTE( "10.ic43",  0x000000, 0x80000, CRC(d60e7706) SHA1(23c383e47e6600a68d6fd8bcfc9552fe0d660630) )
	ROM_LOAD16_BYTE( "4.ic42",   0x000001, 0x80000, CRC(5dbde98a) SHA1(536553eaad0ebfe219e44a4f50a4707209024469) )
	ROM_LOAD16_BYTE( "11.ic168", 0x100000, 0x80000, CRC(92a28e52) SHA1(dc203486b96fdc1930f7e63021e84f203540a64e) )
	ROM_LOAD16_BYTE( "5.ic167",  0x100001, 0x80000, CRC(48c36293) SHA1(2a5d92537ba331a99697d13b4394b8d2737eeaf2) )

	ROM_REGION( 0x20000, "sub", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "16.u7", 0x00000, 0x10000, CRC(7609d818) SHA1(eb841b8e7b34f1c677f1a79bfeda5dafc1f6849f) )
	ROM_LOAD16_BYTE( "17.u6", 0x00001, 0x10000, CRC(951befde) SHA1(28754f00ca0fe38fe1d4e68c203a7b401baa9714) )

	ROM_REGION( 0x200000, "fgtiles", 0 )
	ROM_LOAD( "13.ic151", 0x000000, 0x080000, CRC(4238074b) SHA1(a6b169165c7f7da9e746db8f1fb02e15c02c2b60) )
	ROM_LOAD( "12.ic152", 0x080000, 0x080000, CRC(e97c2b6e) SHA1(3d964999b70af2f39a734eba3feec6d4583261c7) )
	ROM_LOAD( "15.ic153", 0x100000, 0x080000, CRC(c2867d7f) SHA1(86b1be9672cf9f610e1d7efff90d6a73dc1cdb90) )
	ROM_LOAD( "14.ic154", 0x180000, 0x080000, CRC(0cd20423) SHA1(cddad02247b898c0a5a2fe061c41f68ecdf04d5c) )

	/*
	Roms 20 and 21 were missing from the PCB, however the others match perfectly (just with a different data layout)
	I've reconstructed what should be the correct data for this bootleg.

	Note, the bootleg combines 2 GFX regions into a single set of 4-way interleaved ROMs, so we load them in a user
	region and use ROM_COPY.
	*/

	ROM_REGION( 0x200000, "user3", 0 )
	ROM_LOAD32_BYTE( "rom21",    0x000003, 0x80000, CRC(7a08c3b7) SHA1(369123348a88513c066c239ed6aa4db5ae4ef0ac) )
	ROM_LOAD32_BYTE( "rom20",    0x000001, 0x80000, CRC(bd1273d0) SHA1(cc7caee231fe3bd87d8403d34059e1292c7f7a00) )
	ROM_LOAD32_BYTE( "19.ic141", 0x000002, 0x80000, CRC(aa4ebdfe) SHA1(ed117e6a84554c5ed2ad4379b834898a4c40d51e) )
	ROM_LOAD32_BYTE( "18.ic140", 0x000000, 0x80000, CRC(9b2a4325) SHA1(b2020e08251366686c4c0045f3fd523fa327badf) )

	ROM_REGION( 0x100000, "sprites1", 0 )
	ROM_COPY("user3", 0x000000, 0, 0x100000)

	ROM_REGION( 0x080000, "sprites2", 0 )
	ROM_COPY("user3", 0x100000, 0, 0x80000)

	ROM_REGION( 0x90000, "oki", 0 )
	ROM_LOAD( "6.ic13", 0x00000, 0x030000, CRC(6e83ffd8) SHA1(618fd6cd6c0844a4be96f77ff22cd41364718d16) ) // a second dump has 0x40 instead of 0x44 at 0x54cbc. Which one is bad?
	ROM_CONTINUE(       0x40000, 0x050000 )
ROM_END


ROM_START( f1gp2 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "rom12.v1", 0x00000, 0x20000, CRC(c5c5f199) SHA1(56fcbf1d9b15a37204296c578e1585599f76a107) )
	ROM_LOAD16_BYTE( "rom14.v2", 0x00001, 0x20000, CRC(dd5388e2) SHA1(66e88f86edc2407e5794519f988203a52d65636d) )

	ROM_REGION16_BE( 0x200000, "user1", 0 )  // extra ROMs mapped at 100000
	ROM_LOAD16_WORD_SWAP( "rom2", 0x100000, 0x100000, CRC(3b0cfa82) SHA1(ea6803dd8d30aa9f3bd578e113fc26f20c640751) )
	ROM_CONTINUE(                 0x000000, 0x100000 )

	ROM_REGION( 0x20000, "sub", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "rom13.v3", 0x000000, 0x20000, CRC(c37aa303) SHA1(0fe09b398191888620fb676ed0f1593be575512d) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "rom5.v4", 0x00000, 0x20000, CRC(6a9398a1) SHA1(e907fe5f9c135c5b10ec650ec0c6d08cb856230c) )

	ROM_REGION( 0x200000, "fgtiles", 0 )
	ROM_LOAD( "rom1", 0x000000, 0x200000, CRC(f2d55ad7) SHA1(2f2d9dc4fab63b06ed7cba0ef1ced286dbfaa7b4) )

	ROM_REGION( 0x200000, "sprites1", 0 )
	ROM_LOAD( "rom15", 0x000000, 0x200000, CRC(1ac03e2e) SHA1(9073d0ae24364229a993046bd71e403988692993) )

	ROM_REGION( 0x400000, "roztiles", 0 )
	ROM_LOAD16_WORD_SWAP( "rom11", 0x000000, 0x100000, CRC(b22a2c1f) SHA1(b5e67726be5a8561cd04c3c07895b8518b73b89c) )
	ROM_LOAD16_WORD_SWAP( "rom10", 0x100000, 0x100000, CRC(43fcbe23) SHA1(54ab58d904890a0b907e674f855092e974c45edc) )
	ROM_LOAD16_WORD_SWAP( "rom9",  0x200000, 0x100000, CRC(1bede8a1) SHA1(325ecc3afb30d281c2c8a56719e83e4dc20545bb) )
	ROM_LOAD16_WORD_SWAP( "rom8",  0x300000, 0x100000, CRC(98baf2a1) SHA1(df7bd1a743ad0a6e067641e2b7a352c466875ef6) )

	ROM_REGION( 0x080000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "rom4", 0x00000, 0x080000, CRC(c2d3d7ad) SHA1(3178096741583cfef1ca8f53e6efa0a59e1d5cb6) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "rom3", 0x000000, 0x100000, CRC(7f8f066f) SHA1(5e051d5feb327ac818e9c7f7ac721dada3a102b6) )
ROM_END

} // anonymous namespace


GAME( 1991, f1gp,   0,    f1gp,   f1gp,  f1gp_state,  empty_init, ROT90, "Video System Co.",   "F-1 Grand Prix (set 1)",            MACHINE_NO_COCKTAIL | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE ) // censored banners, US McO'River release?
GAME( 1991, f1gpa,  f1gp, f1gp,   f1gp,  f1gp_state,  empty_init, ROT90, "Video System Co.",   "F-1 Grand Prix (set 2)",            MACHINE_NO_COCKTAIL | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )
GAME( 1991, f1gpb,  f1gp, f1gp,   f1gp,  f1gp_state,  empty_init, ROT90, "Video System Co.",   "F-1 Grand Prix (set 3)",            MACHINE_NOT_WORKING | MACHINE_NO_COCKTAIL | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE ) // supposed to be the earliest version dumped and only work with steering wheel
GAME( 1991, f1gpbl, f1gp, f1gpbl, f1gp,  f1gp_state,  empty_init, ROT90, "bootleg (Playmark)", "F-1 Grand Prix (Playmark bootleg)", MACHINE_NOT_WORKING | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE ) // PCB marked 'Super Formula II', manufactured by Playmark.

GAME( 1992, f1gp2,  0,    f1gp2,  f1gp2, f1gp2_state, empty_init, ROT90, "Video System Co.",   "F-1 Grand Prix Part II",            MACHINE_NO_COCKTAIL | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )

// license:BSD-3-Clause
// copyright-holders: Bryan McPhail
// thanks-to: Oliver Bergmann, Randy Mongenel (for initial CPU core)

/***************************************************************************

    Seibu Raiden hardware

    Raiden                          (c) 1990 Seibu Kaihatsu

    To access test mode, reset with both start buttons held.

    There are 3 configuration bytes at 0xffffb, 0xffffd and 0xfffff.
    The byte at 0xffffb (0x1fffd in program ROM 4) controls the appearance of
    the "Winners Don't Use Drugs" screen:
        0x00  = Normal
        0x01  = "Winners Don't Use Drugs" screen appears on boot and between attract loops
        0x02  = Unknown, identical to 0x00? (seen in raident and raidenk)

    The byte at 0xffffd (0x1fffe in program ROM 4) in the main CPU region controls the
    country/game mode:
        bit 7 = if set, respawn instantly; else restart at checkpoint
        bit 5 = unknown (seen in raidenk)

    Low nibble: country/region code
        0x*0  = World/Japan version? (Seibu Kaihatsu) (distributed by Tecmo?)
        0x*1  = USA version (Fabtek license)
        0x*2  = Taiwan version (Liang HWA Electronics license)
        0x*3  = Hong Kong version (Wah Yan Electronics license)
        0x*4  = Korean version (IBL Corporation license)

        An additional 5 codes are in the newer hardware versions:
        0x*5  = Spain version (Ichi Funtel, S.A. license)
        0x*6  = Mexico version
        0x*7  = Central & South America version
        0x*8  = Greece version
        0x*9  = Union Trading Co. license

    The byte at 0xfffff (0x1ffff in program ROM 4) controls whether invincibility is enabled:
        0x00  = Disabled
        0xFF  = Enabled
    On sets with the newer hardware, any non-zero value can be used, but setting SW1:7 to On
    is additionally required.

    Common set is main PCB and an OBJ1 daughterboard.
    XTALs: 20MHz, 14.31818MHz, 12MHz
    CPUs:  2 x Sony CXQ70116P-10 (NEC V30 @ 10MHz), Z80A
    Sound: YM3812, OKI M6295
    Custom ICs:
    - SEI0160 QFP60 (2 on main PCB, 3 on OBJ1 PCB)
    - S1S6091 or SEI0181 QFP80 (4 on main PCB, 4 on OBJ1 PCB)
    - Altera EP910PC-40 EPLD, one next to each V30
    - SEI0050BU DIP40
    - SEI80BU DIP42 (next to encrypted Z80 ROM)
    - SEI0100BU "YM3931"
    - SEI0010BU TC17G008AN-0025 (2 near mask ROMs, 1 near CHR ROMs)
    - SEI0021BU TC17G008AN-0022 (4 between mask ROMs)
    - SG0140 TC110G05AN-0012 (2)

    The following alternate main PCB types have been observed:

    1. "SEI8904 MAIN"; custom chips are similar to the common dedicated
       hardware, but PCB layout is vastly different, resembling Dynamite
       Duke (Seibu's previous game); the format of CHR RAM is transposed
       here. One (undumped) set, perhaps the earliest revision of the game,
       has a half-empty SEI8904-ROM subboard which has the BG and sprite
       tiles in ROMs with even numbers starting at 20, 30, 40; another set
       instead has a SEI9008 subboard that replaces these 12 ROMs with the
       SEI420, SEI430 and SEI440 mask ROMs found on later non-bootleg sets.

    2. Newer Seibu hardware, lacking the encryption PLDs and many of the
       older-generation custom ICs listed above, though the mask ROMs and
       OBJ1 subboard remain the same. The graphics registers here are very
       different from previous licensed sets, having the CRTC-style format
       of all subsequent Seibu arcade games.

    3. Korean bootleg hardware, with no custom ICs and an enormous subboard
       with four ROMs amidst over 100 TTL chips. This hardware also uses
       different XTAL frequencies (32.000MHz, 14.000MHz, 12.000MHz).

    On some boards (both the common dedicated hardware and the older
    SEI8904), the SEI0050BU has been replaced with a small daughterboard
    with six TTL chips and two PLDs labeled S50P01 and S50P02. A bootleg
    set has been observed with a (larger) daughterboard likewise standing
    in for a Xilinx PLD.

    To be verified on the bootleg:
    - Screen timings, Z80 and YM3812 clocks
    - Are the V30 main and sub programs really encrypted here?
    - Correct numbers and locations for all ROMs

***************************************************************************/

#include "emu.h"

#include "seibu_crtc.h"
#include "seibusound.h"

#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class raiden_state : public driver_device, public seibu_sound_common
{
public:
	raiden_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_seibu_sound(*this, "seibu_sound"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_shared_ram(*this, "shared_ram"),
		m_textram(*this, "textram"),
		m_scroll_ram(*this, "scroll_ram"),
		m_bgram(*this, "bgram"),
		m_fgram(*this, "fgram")
	{ }

	void raidene(machine_config &config);
	void raiden(machine_config &config);
	void raidenkb(machine_config &config);
	void raidenu(machine_config &config);

	void init_decryption();

protected:
	required_device<cpu_device> m_maincpu;
	required_device<seibu_sound_device> m_seibu_sound;

	bool m_bg_layer_enabled = 0;
	bool m_fg_layer_enabled = 0;
	bool m_tx_layer_enabled = 0;
	bool m_sp_layer_enabled = 0;
	bool m_flipscreen = 0;

	virtual void video_start() override ATTR_COLD;

	u32 screen_update_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u16 *scrollregs);

	void textram_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void common_video_start();

	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_spriteram;

	required_shared_ptr<u16> m_shared_ram;
	required_shared_ptr<u16> m_textram;
	optional_shared_ptr<u16> m_scroll_ram;
	required_shared_ptr<u16> m_bgram;
	required_shared_ptr<u16> m_fgram;

	tilemap_t *m_bg_layer = nullptr;
	tilemap_t *m_fg_layer = nullptr;
	tilemap_t *m_tx_layer = nullptr;

	void bgram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void fgram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void control_w(u8 data);

	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void vblank_irq(int state);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &primap);

	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
	void raiden_sound_map(address_map &map) ATTR_COLD;
	void raiden_sound_decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void raidenu_main_map(address_map &map) ATTR_COLD;
	void raidenu_sub_map(address_map &map) ATTR_COLD;
	void sei80bu_encrypted_full_map(address_map &map) ATTR_COLD;
};


class raidenb_state : public raiden_state
{
public:
	using raiden_state::raiden_state;

	void raidenb(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	u16 m_scroll_ram[6];

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void control_w(u8 data);
	void layer_enable_w(u16 data);
	void layer_scroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void main_map(address_map &map) ATTR_COLD;
};


/******************************************************************************/

void raiden_state::bgram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bgram[offset]);
	m_bg_layer->mark_tile_dirty(offset);
}

void raiden_state::fgram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fgram[offset]);
	m_fg_layer->mark_tile_dirty(offset);
}

void raiden_state::textram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_textram[offset]);
	m_tx_layer->mark_tile_dirty(offset);
}


void raiden_state::control_w(u8 data)
{
	// d0: back layer disable
	// d1: fore layer disable
	// d2: text layer disable
	// d3: sprite layer disable
	// d4: unused
	// d5: unused
	// d6: flipscreen
	// d7: toggles, maybe spriteram bank? (for buffering)
	m_bg_layer_enabled = BIT(~data, 0);
	m_fg_layer_enabled = BIT(~data, 1);
	m_tx_layer_enabled = BIT(~data, 2);
	m_sp_layer_enabled = BIT(~data, 3);

	m_flipscreen = BIT(data, 6);
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
}

void raidenb_state::control_w(u8 data)
{
	// d1: flipscreen
	// d2: toggles, maybe spriteram bank? (for buffering)
	// d3: text layer disable (I guess raidenb textlayer isn't part of sei_crtc?)
	// other bits: unused
	m_tx_layer_enabled = BIT(~data, 3);

	m_flipscreen = BIT(data, 1);
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
}

void raidenb_state::layer_enable_w(u16 data)
{
	// d0: back layer disable
	// d1: fore layer disable
	// d4: sprite layer disable
	// other bits: unused? (d2-d3 always set, d5-d7 always clear)
	m_bg_layer_enabled = BIT(~data, 0);
	m_fg_layer_enabled = BIT(~data, 1);
	m_sp_layer_enabled = BIT(~data, 4);
}


/******************************************************************************/

void raiden_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &primap)
{
	if (!m_sp_layer_enabled)
		return;

	u16 *sprites = m_spriteram->buffer();
	const u32 size = m_spriteram->bytes() / 2;
	gfx_element *gfx = m_gfxdecode->gfx(3);

	for (int offs = 0; offs < size; offs += 4)
	{
		/*
		    Word #0
		    x------- --------  active
		    -x------ --------  flipy
		    --x----- --------  flipx
		    ---x---- --------  unused
		    ----xxxx --------  color
		    -------- xxxxxxxx  y

		    Word #1
		    x------- --------  ? (set when groundboss explodes)
		    -xxx---- --------  unused
		    ----xxxx xxxxxxxx  code

		    Word #2
		    xx------ --------  priority
		    --xxxxx- --------  unused
		    -------x xxxxxxxx  x (signed)

		    Word #3 unused
		*/

		if (!BIT(sprites[offs + 0], 15))
			continue;

		const u8 priority = BIT(sprites[offs + 2], 14, 2);
		if (priority == 0)
			continue;

		u32 pri_mask = GFX_PMASK_4 | GFX_PMASK_2 | GFX_PMASK_1;
		switch (priority)
		{
		case 1: // draw sprites underneath foreground
			pri_mask = GFX_PMASK_4 | GFX_PMASK_2;
			break;
		case 2:
		case 3: // rest of sprites, draw underneath text
		default:
			pri_mask = GFX_PMASK_4;
			break;
		}

		bool flipy = BIT(sprites[offs + 0], 14);
		bool flipx = BIT(sprites[offs + 0], 13);
		const u32 color = BIT(sprites[offs + 0], 8, 4);
		const u32 code = BIT(sprites[offs + 1], 0, 12);

		s32 y = BIT(sprites[offs + 0], 0, 8);
		s32 x = BIT(sprites[offs + 2], 0, 9);
		if (BIT(x, 8)) // sign bit
			x |= ~0x1ff;

		if (m_flipscreen)
		{
			x = 240 - x;
			y = 240 - y;
			flipy = !flipy;
			flipx = !flipx;
		}

		gfx->prio_transpen(bitmap, cliprect, code, color, flipx, flipy, x, y, primap, pri_mask, 15);
	}
}

u32 raiden_state::screen_update_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u16 *scrollregs)
{
	// set tilemaps scroll
	m_bg_layer->set_scrollx(0, scrollregs[0]);
	m_bg_layer->set_scrolly(0, scrollregs[1]);
	m_fg_layer->set_scrollx(0, scrollregs[2]);
	m_fg_layer->set_scrolly(0, scrollregs[3]);

	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->black_pen(), cliprect);

	// back layer
	if (m_bg_layer_enabled)
		m_bg_layer->draw(screen, bitmap, cliprect, 0, 1);

	// fore layer
	if (m_fg_layer_enabled)
		m_fg_layer->draw(screen, bitmap, cliprect, 0, 2);

	// text layer
	if (m_tx_layer_enabled)
		m_tx_layer->draw(screen, bitmap, cliprect, 0, 4);

	draw_sprites(bitmap, cliprect, screen.priority());

	return 0;
}

u32 raiden_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// set up scrollregs
	// scroll_ram is only 8 bits wide. 4 bytes per scroll, skip uneven ones
	// 00-03: 28 *0 ** ae  -  bg layer scroll y
	// 08-0b: 28 *0 ** b9  -  bg layer scroll x
	// 10-13: 28 *0 ** ae  -  fg layer scroll y
	// 18-1b: 28 *0 ** b9  -  fg layer scroll x
	u16 scrollregs[4];
	scrollregs[0] = ((m_scroll_ram[0x09] & 0xf0) << 4) | ((m_scroll_ram[0x0a] & 0x7f) << 1) | ((m_scroll_ram[0x0a] & 0x80) >> 7);
	scrollregs[1] = ((m_scroll_ram[0x01] & 0xf0) << 4) | ((m_scroll_ram[0x02] & 0x7f) << 1) | ((m_scroll_ram[0x02] & 0x80) >> 7);
	scrollregs[2] = ((m_scroll_ram[0x19] & 0xf0) << 4) | ((m_scroll_ram[0x1a] & 0x7f) << 1) | ((m_scroll_ram[0x1a] & 0x80) >> 7);
	scrollregs[3] = ((m_scroll_ram[0x11] & 0xf0) << 4) | ((m_scroll_ram[0x12] & 0x7f) << 1) | ((m_scroll_ram[0x12] & 0x80) >> 7);

	return screen_update_common(screen, bitmap, cliprect, scrollregs);
}

u32 raidenb_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return screen_update_common(screen, bitmap, cliprect, m_scroll_ram);
}


/******************************************************************************/

TILE_GET_INFO_MEMBER(raiden_state::get_back_tile_info)
{
	const u16 tiledata = m_bgram[tile_index];
	const u32 tile = tiledata & 0x0fff;
	const u32 color = tiledata >> 12;

	tileinfo.set(1, tile, color, 0);
}

TILE_GET_INFO_MEMBER(raiden_state::get_fore_tile_info)
{
	const u16 tiledata = m_fgram[tile_index];
	const u32 tile = tiledata & 0x0fff;
	const u32 color = tiledata >> 12;

	tileinfo.set(2, tile, color, 0);
}

TILE_GET_INFO_MEMBER(raiden_state::get_text_tile_info)
{
	const u16 tiledata = m_textram[tile_index];
	const u32 tile = (tiledata & 0xff) | ((tiledata >> 6) & 0x300);
	const u32 color = (tiledata >> 8) & 0x0f;

	tileinfo.set(0, tile, color, 0);
}

void raiden_state::common_video_start()
{
	m_bg_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raiden_state::get_back_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 32, 32);
	m_fg_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raiden_state::get_fore_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 32, 32);
	m_fg_layer->set_transparent_pen(15);

	save_item(NAME(m_bg_layer_enabled));
	save_item(NAME(m_fg_layer_enabled));
	save_item(NAME(m_tx_layer_enabled));
	save_item(NAME(m_sp_layer_enabled));
	save_item(NAME(m_flipscreen));
}

void raiden_state::video_start()
{
	common_video_start();

	m_tx_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raiden_state::get_text_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tx_layer->set_transparent_pen(15);
}

void raidenb_state::video_start()
{
	common_video_start();

	m_tx_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raidenb_state::get_text_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);
	m_tx_layer->set_transparent_pen(15);

	save_item(NAME(m_scroll_ram));
}


/******************************************************************************/

void raiden_state::main_map(address_map &map)
{
	map(0x00000, 0x06fff).ram();
	map(0x07000, 0x07fff).ram().share("spriteram");
	map(0x08000, 0x08fff).ram().share(m_shared_ram);
	map(0x0a000, 0x0a00d).rw(m_seibu_sound, FUNC(seibu_sound_device::main_r), FUNC(seibu_sound_device::main_w)).umask16(0x00ff);
	map(0x0c000, 0x0c7ff).w(FUNC(raiden_state::textram_w)).share(m_textram);
	map(0x0e000, 0x0e001).portr("P1_P2");
	map(0x0e002, 0x0e003).portr("DSW");
	map(0x0e004, 0x0e005).nopw(); // watchdog?
	map(0x0e006, 0x0e006).w(FUNC(raiden_state::control_w));
	map(0x0f000, 0x0f03f).writeonly().share(m_scroll_ram);
	map(0xa0000, 0xfffff).rom().region("maincpu", 0);
}

void raiden_state::sub_map(address_map &map)
{
	map(0x00000, 0x01fff).ram();
	map(0x02000, 0x027ff).ram().w(FUNC(raiden_state::bgram_w)).share(m_bgram);
	map(0x02800, 0x02fff).ram().w(FUNC(raiden_state::fgram_w)).share(m_fgram);
	map(0x03000, 0x03fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x04000, 0x04fff).ram().share(m_shared_ram);
	map(0x07ffe, 0x07fff).nopw(); // ?
	map(0x08000, 0x08001).nopw(); // watchdog?
	map(0x0a000, 0x0a001).nopw(); // ?
	map(0xc0000, 0xfffff).rom().region("sub", 0);
}


/******************************************************************************/

void raiden_state::raidenu_main_map(address_map &map)
{
	map(0x00000, 0x06fff).ram();
	map(0x07000, 0x07fff).ram().share("spriteram");
	map(0x08000, 0x0803f).writeonly().share(m_scroll_ram);
	map(0x0a000, 0x0afff).ram().share(m_shared_ram);
	map(0x0b000, 0x0b001).portr("P1_P2");
	map(0x0b002, 0x0b003).portr("DSW");
	map(0x0b004, 0x0b005).nopw(); // watchdog?
	map(0x0b006, 0x0b006).w(FUNC(raiden_state::control_w));
	map(0x0c000, 0x0c7ff).w(FUNC(raiden_state::textram_w)).share(m_textram);
	map(0x0d000, 0x0d00d).rw(m_seibu_sound, FUNC(seibu_sound_device::main_r), FUNC(seibu_sound_device::main_w)).umask16(0x00ff);
	map(0xa0000, 0xfffff).rom().region("maincpu", 0);
}

void raiden_state::raidenu_sub_map(address_map &map)
{
	map(0x00000, 0x05fff).ram();
	map(0x06000, 0x067ff).ram().w(FUNC(raiden_state::bgram_w)).share(m_bgram);
	map(0x06800, 0x06fff).ram().w(FUNC(raiden_state::fgram_w)).share(m_fgram);
	map(0x07000, 0x07fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x08000, 0x08fff).ram().share(m_shared_ram);
	map(0x0a000, 0x0a001).nopw(); // ?
	map(0x0c000, 0x0c001).nopw(); // watchdog?
	map(0xc0000, 0xfffff).rom().region("sub", 0);
}


/******************************************************************************/

void raidenb_state::main_map(address_map &map)
{
	map(0x00000, 0x06fff).ram();
	map(0x07000, 0x07fff).ram().share("spriteram");
	map(0x0a000, 0x0afff).ram().share(m_shared_ram);
	map(0x0b000, 0x0b001).portr("P1_P2");
	map(0x0b002, 0x0b003).portr("DSW");
	map(0x0b004, 0x0b005).nopw(); // watchdog?
	map(0x0b006, 0x0b006).w(FUNC(raidenb_state::control_w));
	map(0x0c000, 0x0c7ff).w(FUNC(raidenb_state::textram_w)).share(m_textram);
	map(0x0d000, 0x0d00d).rw(m_seibu_sound, FUNC(seibu_sound_device::main_r), FUNC(seibu_sound_device::main_w)).umask16(0x00ff);
	map(0x0d040, 0x0d08f).rw("crtc", FUNC(seibu_crtc_device::read), FUNC(seibu_crtc_device::write));
	map(0xa0000, 0xfffff).rom().region("maincpu", 0);
}


/*****************************************************************************/

void raiden_state::raiden_sound_map(address_map &map)
{
	map(0x0000, 0xffff).r("sei80bu", FUNC(sei80bu_device::data_r));
	map(0x2000, 0x27ff).ram();
	map(0x4000, 0x4000).w(m_seibu_sound, FUNC(seibu_sound_device::pending_w));
	map(0x4001, 0x4001).w(m_seibu_sound, FUNC(seibu_sound_device::irq_clear_w));
	map(0x4002, 0x4002).w(m_seibu_sound, FUNC(seibu_sound_device::rst10_ack_w));
	map(0x4003, 0x4003).w(m_seibu_sound, FUNC(seibu_sound_device::rst18_ack_w));
	map(0x4007, 0x4007).w(m_seibu_sound, FUNC(seibu_sound_device::bank_w));
	map(0x4008, 0x4009).rw(m_seibu_sound, FUNC(seibu_sound_device::ym_r), FUNC(seibu_sound_device::ym_w));
	map(0x4010, 0x4011).r(m_seibu_sound, FUNC(seibu_sound_device::soundlatch_r));
	map(0x4012, 0x4012).r(m_seibu_sound, FUNC(seibu_sound_device::main_data_pending_r));
	map(0x4013, 0x4013).portr("COIN");
	map(0x4018, 0x4019).w(m_seibu_sound, FUNC(seibu_sound_device::main_data_w));
	map(0x401b, 0x401b).w(m_seibu_sound, FUNC(seibu_sound_device::coin_w));
	map(0x6000, 0x6000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void raiden_state::raiden_sound_decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0xffff).r("sei80bu", FUNC(sei80bu_device::opcode_r));
}

void raiden_state::sei80bu_encrypted_full_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("audiocpu", 0);
	map(0x8000, 0xffff).bankr("seibu_bank1");
}


/*****************************************************************************/

static INPUT_PORTS_START( raiden )
	SEIBU_COIN_INPUTS // coin inputs read through sound CPU

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Coin Mode" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, "A" )
	PORT_DIPSETTING(      0x0000, "B" )
	// Coin Mode A
	PORT_DIPNAME( 0x001e, 0x001e, DEF_STR( Coinage ) ) PORT_CONDITION("DSW", 0x0001, EQUALS, 0x0001) PORT_DIPLOCATION("SW1:2,3,4,5")
	PORT_DIPSETTING(      0x0014, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0016, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x001a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x001e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0012, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	// Coin Mode B
	PORT_DIPNAME( 0x0006, 0x0006, DEF_STR( Coin_A ) ) PORT_CONDITION("DSW", 0x0001, NOTEQUALS, 0x0001) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(      0x0000, "5C/1C or Free if Coin B too" )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Coin_B ) ) PORT_CONDITION("DSW", 0x0001, NOTEQUALS, 0x0001) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, "1C/6C or Free if Coin A too" )

	PORT_DIPNAME( 0x0020, 0x0020, "Credits to Start" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7")
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, "1" )
	PORT_DIPSETTING(      0x0100, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, "80000 300000" )
	PORT_DIPSETTING(      0x0c00, "150000 400000" )
	PORT_DIPSETTING(      0x0400, "300000 1000000" )
	PORT_DIPSETTING(      0x0000, "1000000 5000000" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
INPUT_PORTS_END


/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,           // 8*8 characters
	RGN_FRAC(1,1), // 1024 characters
	4,             // 4 bits per pixel
	{ STEP4(12,-4) },
	{ STEP4(0,1), STEP4(16,1) },
	{ STEP8(0,16*2) },
	8*8*4
};

static const gfx_layout tilelayout =
{
	16,16,         // 16*16 tiles
	RGN_FRAC(1,1), // 4096 tiles
	4,             // 4 bits per pixel
	{ STEP4(12,-4) },
	{ STEP4(0,1), STEP4(16,1), STEP4(16*16*2,1), STEP4(16*16*2+16,1) },
	{ STEP16(0,16*2) },
	16*16*4
};

static GFXDECODE_START( gfx_raiden )
	GFXDECODE_ENTRY( "text",    0, charlayout, 768, 16 )
	GFXDECODE_ENTRY( "bgtiles", 0, tilelayout,   0, 16 )
	GFXDECODE_ENTRY( "fgtiles", 0, tilelayout, 256, 16 )
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 512, 16 )
GFXDECODE_END


/******************************************************************************/

void raiden_state::vblank_irq(int state)
{
	if (state)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xc8 / 4); // V30
		m_subcpu->set_input_line_and_vector(0, HOLD_LINE, 0xc8 / 4); // V30
	}
}

void raiden_state::raiden(machine_config &config)
{
	// basic machine hardware
	V30(config, m_maincpu, XTAL(20'000'000) / 2); // NEC V30 CPU, 20MHz verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &raiden_state::main_map);

	V30(config, m_subcpu, XTAL(20'000'000) / 2); // NEC V30 CPU, 20MHz verified on PCB
	m_subcpu->set_addrmap(AS_PROGRAM, &raiden_state::sub_map);

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(14'318'181) / 4)); // verified on PCB
	audiocpu.set_addrmap(AS_PROGRAM, &raiden_state::seibu_sound_map);
	audiocpu.set_irq_acknowledge_callback("seibu_sound", FUNC(seibu_sound_device::im0_vector_cb));

	config.set_maximum_quantum(attotime::from_hz(12000));

	// video hardware
	BUFFERED_SPRITERAM16(config, m_spriteram);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.60); // verified on PCB
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(raiden_state::screen_update));
	screen.screen_vblank().set(m_spriteram, FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.screen_vblank().append(FUNC(raiden_state::vblank_irq));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_raiden);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 2048);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(14'318'181) / 4));
	ymsnd.irq_handler().set("seibu_sound", FUNC(seibu_sound_device::fm_irqhandler));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(12'000'000) / 12, okim6295_device::PIN7_HIGH)); // frequency and pin 7 verified
	oki.add_route(ALL_OUTPUTS, "mono", 0.75);

	SEIBU_SOUND(config, m_seibu_sound, 0);
	m_seibu_sound->int_callback().set_inputline("audiocpu", 0);
	m_seibu_sound->set_rom_tag("audiocpu");
	m_seibu_sound->set_rombank_tag("seibu_bank1");
	m_seibu_sound->ym_read_callback().set("ymsnd", FUNC(ym3812_device::read));
	m_seibu_sound->ym_write_callback().set("ymsnd", FUNC(ym3812_device::write));
}

void raiden_state::raidene(machine_config &config)
{
	raiden(config);
	subdevice<z80_device>("audiocpu")->set_addrmap(AS_PROGRAM, &raiden_state::raiden_sound_map);
	subdevice<z80_device>("audiocpu")->set_addrmap(AS_OPCODES, &raiden_state::raiden_sound_decrypted_opcodes_map);

	sei80bu_device &sei80bu(SEI80BU(config, "sei80bu", 0));
	sei80bu.set_addrmap(AS_PROGRAM, &raiden_state::sei80bu_encrypted_full_map);
}

void raiden_state::raidenu(machine_config &config)
{
	raidene(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &raiden_state::raidenu_main_map);
	m_subcpu->set_addrmap(AS_PROGRAM, &raiden_state::raidenu_sub_map);
}

void raidenb_state::layer_scroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_scroll_ram[offset]);
}

void raiden_state::raidenkb(machine_config &config)
{
	raiden(config);
	m_maincpu->set_clock(XTAL(32'000'000) / 4); // Xtal and clock verified
	m_subcpu->set_clock(XTAL(32'000'000) / 4); // Xtal and clock verified
}

void raidenb_state::raidenb(machine_config &config)
{
	raiden(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &raidenb_state::main_map);

	// video hardware
	seibu_crtc_device &crtc(SEIBU_CRTC(config, "crtc", 0));
	crtc.layer_en_callback().set(FUNC(raidenb_state::layer_enable_w));
	crtc.layer_scroll_callback().set(FUNC(raidenb_state::layer_scroll_w));

	subdevice<screen_device>("screen")->set_screen_update(FUNC(raidenb_state::screen_update));
}


/***************************************************************************/

/*
    Note: Seibu labeled the ROMs simply as 1 through 10 and didn't generally
          change the labels at all between versions even though the data was
          different between them.
*/

// These versions use the same board and make use of the configuration bytes at 0x1fffd & 0x1fffe
ROM_START( raidenj ) // from a board with 2 daughter cards, no official board #s?
	ROM_REGION( 0x060000, "maincpu", 0 ) // v30
	ROM_LOAD16_BYTE( "1.u0253", 0x000000, 0x10000, CRC(a4b12785) SHA1(446314e82ce01315cb3e3d1f323eaa2ad6fb48dd) )
	ROM_LOAD16_BYTE( "2.u0252", 0x000001, 0x10000, CRC(17640bd5) SHA1(5bbc99900426b1a072b52537ae9a50220c378a0d) )
	ROM_LOAD16_BYTE( "3.u022",  0x020000, 0x20000, CRC(f6af09d0) SHA1(ecd49f3351359ea2d5cbd140c9962d45c5544ecd) ) // both 3 & 4 had a red "dot" on label, 4 also had printed "J"
	ROM_LOAD16_BYTE( "4j.u023", 0x020001, 0x20000, CRC(505c4c5d) SHA1(07f61fd1ff24f482a1ae2f86c4c0f32850cbd539) ) // 0x1fffd == 0x00, 0x1fffe == 0x04

	ROM_REGION( 0x040000, "sub", 0 ) // v30
	ROM_LOAD16_BYTE( "5.u042", 0x000000, 0x20000, CRC(ed03562e) SHA1(bf6b44fb53fa2321cd52c00fcb43b8ceb6ceffff) )
	ROM_LOAD16_BYTE( "6.u043", 0x000001, 0x20000, CRC(a19d5b5d) SHA1(aa5e5be60b737913e5677f88ebc218302245e5af) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "8.u212",      0x000000, 0x08000, CRC(cbe055c7) SHA1(34a06a541d059c621d87fdf41546c9d052a61963) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x010000, "text", 0 )
	ROM_LOAD16_BYTE( "9",  0x00001, 0x08000, CRC(1922b25e) SHA1(da27122dd1c43770e7385ad602ef397c64d2f754) ) // On some PCBs there is no explicit
	ROM_LOAD16_BYTE( "10", 0x00000, 0x08000, CRC(5f90786a) SHA1(4f63b07c6afbcf5196a433f3356bef984fe303ef) ) // U location for these two ROMs

	ROM_REGION( 0x080000, "bgtiles", 0 )
	ROM_LOAD( "sei420", 0x00000, 0x80000, CRC(da151f0b) SHA1(02682497caf5f058331f18c652471829fa08d54f) ) // tiles @ U105 on this PCB

	ROM_REGION( 0x080000, "fgtiles", 0 )
	ROM_LOAD( "sei430", 0x00000, 0x80000, CRC(ac1f57ac) SHA1(1de926a0db73b99904ef119ac816c53d1551156a) ) // tiles @ U115 on this PCB

	ROM_REGION( 0x090000, "sprites", 0 )
	ROM_LOAD( "sei440",  0x00000, 0x80000, CRC(946d7bde) SHA1(30e8755c2b1ca8bff6278710b8422b51f75eec10) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "7.u203", 0x00000, 0x10000, CRC(8f927822) SHA1(592f2719f2c448c3b4b239eeaec078b411e12dbb) )

	ROM_REGION( 0x2000, "plds", 0 ) // 2x Altera EP910PC-40 (read protected)
	ROM_LOAD( "rd003b.u0168", 0x0000, 0x0884, NO_DUMP )
	ROM_LOAD( "rd006b.u0365", 0x1000, 0x0884, NO_DUMP )

	ROM_REGION( 0x0200, "proms", 0 ) // N82S135N bipolar PROMs
	ROM_LOAD( "rd010.u087", 0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "rd012.u094", 0x0100, 0x0100, NO_DUMP )
ROM_END

ROM_START( raiden )
	ROM_REGION( 0x060000, "maincpu", 0 ) // v30
	ROM_LOAD16_BYTE( "1.u0253", 0x000000, 0x10000, CRC(a4b12785) SHA1(446314e82ce01315cb3e3d1f323eaa2ad6fb48dd) )
	ROM_LOAD16_BYTE( "2.u0252", 0x000001, 0x10000, CRC(17640bd5) SHA1(5bbc99900426b1a072b52537ae9a50220c378a0d) )
	ROM_LOAD16_BYTE( "3.u022",  0x020000, 0x20000, CRC(f6af09d0) SHA1(ecd49f3351359ea2d5cbd140c9962d45c5544ecd) )
	ROM_LOAD16_BYTE( "4.u023",  0x020001, 0x20000, CRC(6bdfd416) SHA1(7c3692d0c46c0fd360b9b2b5a8dc55d9217be357) ) // 0x1fffd == 0x00, 0x1fffe == 0x84

	ROM_REGION( 0x040000, "sub", 0 ) // v30
	ROM_LOAD16_BYTE( "5.u042", 0x000000, 0x20000, CRC(ed03562e) SHA1(bf6b44fb53fa2321cd52c00fcb43b8ceb6ceffff) )
	ROM_LOAD16_BYTE( "6.u043", 0x000001, 0x20000, CRC(a19d5b5d) SHA1(aa5e5be60b737913e5677f88ebc218302245e5af) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "8.u212",      0x000000, 0x08000, CRC(cbe055c7) SHA1(34a06a541d059c621d87fdf41546c9d052a61963) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x010000, "text", 0 )
	ROM_LOAD16_BYTE( "9",  0x00001, 0x08000, CRC(1922b25e) SHA1(da27122dd1c43770e7385ad602ef397c64d2f754) ) // On some PCBs there is no explicit
	ROM_LOAD16_BYTE( "10", 0x00000, 0x08000, CRC(5f90786a) SHA1(4f63b07c6afbcf5196a433f3356bef984fe303ef) ) // U location for these two ROMs

	ROM_REGION( 0x080000, "bgtiles", 0 )
	ROM_LOAD( "sei420", 0x00000, 0x80000, CRC(da151f0b) SHA1(02682497caf5f058331f18c652471829fa08d54f) ) // tiles @ U105 on this PCB

	ROM_REGION( 0x080000, "fgtiles", 0 )
	ROM_LOAD( "sei430", 0x00000, 0x80000, CRC(ac1f57ac) SHA1(1de926a0db73b99904ef119ac816c53d1551156a) ) // tiles @ U115 on this PCB

	ROM_REGION( 0x090000, "sprites", 0 )
	ROM_LOAD( "sei440",  0x00000, 0x80000, CRC(946d7bde) SHA1(30e8755c2b1ca8bff6278710b8422b51f75eec10) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "7.u203", 0x00000, 0x10000, CRC(8f927822) SHA1(592f2719f2c448c3b4b239eeaec078b411e12dbb) )

	ROM_REGION( 0x2000, "plds", 0 ) // 2x Altera EP910PC-40 (read protected)
	ROM_LOAD( "rd003b.u0168", 0x0000, 0x0884, NO_DUMP )
	ROM_LOAD( "rd006b.u0365", 0x1000, 0x0884, NO_DUMP )

	ROM_REGION( 0x0200, "proms", 0 ) // N82S135N bipolar PROMs
	ROM_LOAD( "rd010.u087", 0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "rd012.u094", 0x0100, 0x0100, NO_DUMP )
ROM_END

ROM_START( raident )
	ROM_REGION( 0x060000, "maincpu", 0 ) // v30
	ROM_LOAD16_BYTE( "1.u0253", 0x000000, 0x10000, CRC(a4b12785) SHA1(446314e82ce01315cb3e3d1f323eaa2ad6fb48dd) )
	ROM_LOAD16_BYTE( "2.u0252", 0x000001, 0x10000, CRC(17640bd5) SHA1(5bbc99900426b1a072b52537ae9a50220c378a0d) )
	ROM_LOAD16_BYTE( "3.u022",  0x020000, 0x20000, CRC(f6af09d0) SHA1(ecd49f3351359ea2d5cbd140c9962d45c5544ecd) )
	ROM_LOAD16_BYTE( "4t.u023", 0x020001, 0x20000, CRC(61eefab1) SHA1(a886ce1eb1c6451b1cf9eb8dbdc2d484d9881ced) ) // 0x1fffd == 0x02, 0x1fffe == 0x06

	ROM_REGION( 0x040000, "sub", 0 ) // v30
	ROM_LOAD16_BYTE( "5.u042", 0x000000, 0x20000, CRC(ed03562e) SHA1(bf6b44fb53fa2321cd52c00fcb43b8ceb6ceffff) )
	ROM_LOAD16_BYTE( "6.u043", 0x000001, 0x20000, CRC(a19d5b5d) SHA1(aa5e5be60b737913e5677f88ebc218302245e5af) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "8.u212",      0x000000, 0x08000, CRC(cbe055c7) SHA1(34a06a541d059c621d87fdf41546c9d052a61963) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x010000, "text", 0 )
	ROM_LOAD16_BYTE( "9",  0x00001, 0x08000, CRC(1922b25e) SHA1(da27122dd1c43770e7385ad602ef397c64d2f754) ) // On some PCBs there is no explicit
	ROM_LOAD16_BYTE( "10", 0x00000, 0x08000, CRC(5f90786a) SHA1(4f63b07c6afbcf5196a433f3356bef984fe303ef) ) // U location for these two ROMs

	ROM_REGION( 0x080000, "bgtiles", 0 )
	ROM_LOAD( "sei420", 0x00000, 0x80000, CRC(da151f0b) SHA1(02682497caf5f058331f18c652471829fa08d54f) ) // tiles @ U105 on this PCB

	ROM_REGION( 0x080000, "fgtiles", 0 )
	ROM_LOAD( "sei430", 0x00000, 0x80000, CRC(ac1f57ac) SHA1(1de926a0db73b99904ef119ac816c53d1551156a) ) // tiles @ U115 on this PCB

	ROM_REGION( 0x090000, "sprites", 0 )
	ROM_LOAD( "sei440",  0x00000, 0x80000, CRC(946d7bde) SHA1(30e8755c2b1ca8bff6278710b8422b51f75eec10) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "7.u203", 0x00000, 0x10000, CRC(8f927822) SHA1(592f2719f2c448c3b4b239eeaec078b411e12dbb) )

	ROM_REGION( 0x2000, "plds", 0 ) // 2x Altera EP910PC-40 (read protected)
	ROM_LOAD( "rd003b.u0168", 0x0000, 0x0884, NO_DUMP )
	ROM_LOAD( "rd006b.u0365", 0x1000, 0x0884, NO_DUMP )

	ROM_REGION( 0x0200, "proms", 0 ) // N82S135N bipolar PROMs
	ROM_LOAD( "rd010.u087", 0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "rd012.u094", 0x0100, 0x0100, NO_DUMP )
ROM_END

ROM_START( raidenu )
	ROM_REGION( 0x060000, "maincpu", 0 ) // v30
	ROM_LOAD16_BYTE( "1.u0253", 0x000000, 0x10000, CRC(a4b12785) SHA1(446314e82ce01315cb3e3d1f323eaa2ad6fb48dd) )
	ROM_LOAD16_BYTE( "2.u0252", 0x000001, 0x10000, CRC(17640bd5) SHA1(5bbc99900426b1a072b52537ae9a50220c378a0d) )
	ROM_LOAD16_BYTE( "3a.u022", 0x020000, 0x20000, CRC(a8fadbdd) SHA1(a23729a51c45c1dba4e625503a37d111ae72ced0) ) // Both 3A & 4A different for the US version
	ROM_LOAD16_BYTE( "4a.u023", 0x020001, 0x20000, CRC(bafb268d) SHA1(132d3ebf9d9d5fffa3040338106fad428c54dbaa) ) // 0x1fffd == 0x01, 0x1fffe == 0x85

	ROM_REGION( 0x040000, "sub", 0 ) // v30
	ROM_LOAD16_BYTE( "5.u042", 0x000000, 0x20000, CRC(ed03562e) SHA1(bf6b44fb53fa2321cd52c00fcb43b8ceb6ceffff) )
	ROM_LOAD16_BYTE( "6.u043", 0x000001, 0x20000, CRC(a19d5b5d) SHA1(aa5e5be60b737913e5677f88ebc218302245e5af) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "8.u212",      0x000000, 0x08000, CRC(cbe055c7) SHA1(34a06a541d059c621d87fdf41546c9d052a61963) )
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x010000, "text", 0 )
	ROM_LOAD16_BYTE( "9",  0x00001, 0x08000, CRC(1922b25e) SHA1(da27122dd1c43770e7385ad602ef397c64d2f754) ) // On some PCBs there is no explicit
	ROM_LOAD16_BYTE( "10", 0x00000, 0x08000, CRC(5f90786a) SHA1(4f63b07c6afbcf5196a433f3356bef984fe303ef) ) // U location for these two ROMs

	ROM_REGION( 0x080000, "bgtiles", 0 )
	ROM_LOAD( "sei420", 0x00000, 0x80000, CRC(da151f0b) SHA1(02682497caf5f058331f18c652471829fa08d54f) ) // tiles @ U105 on this PCB

	ROM_REGION( 0x080000, "fgtiles", 0 )
	ROM_LOAD( "sei430", 0x00000, 0x80000, CRC(ac1f57ac) SHA1(1de926a0db73b99904ef119ac816c53d1551156a) ) // tiles @ U115 on this PCB

	ROM_REGION( 0x090000, "sprites", 0 )
	ROM_LOAD( "sei440",  0x00000, 0x80000, CRC(946d7bde) SHA1(30e8755c2b1ca8bff6278710b8422b51f75eec10) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "7.u203", 0x00000, 0x10000, CRC(8f927822) SHA1(592f2719f2c448c3b4b239eeaec078b411e12dbb) )

	ROM_REGION( 0x2000, "plds", 0 ) // 2x Altera EP910PC-40 (read protected)
	ROM_LOAD( "rd003b.u0168", 0x0000, 0x0884, NO_DUMP )
	ROM_LOAD( "rd006b.u0365", 0x1000, 0x0884, NO_DUMP )

	ROM_REGION( 0x0200, "proms", 0 ) // N82S135N bipolar PROMs
	ROM_LOAD( "rd010.u087", 0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "rd012.u094", 0x0100, 0x0100, NO_DUMP )
ROM_END

ROM_START( raidenk ) // Same board as above. Not sure why the sound CPU would be decrypted
	ROM_REGION( 0x060000, "maincpu", 0 ) // v30
	ROM_LOAD16_BYTE( "1.u0253", 0x000000, 0x10000, CRC(a4b12785) SHA1(446314e82ce01315cb3e3d1f323eaa2ad6fb48dd) )
	ROM_LOAD16_BYTE( "2.u0252", 0x000001, 0x10000, CRC(17640bd5) SHA1(5bbc99900426b1a072b52537ae9a50220c378a0d) )
	ROM_LOAD16_BYTE( "3.u022",  0x020000, 0x20000, CRC(f6af09d0) SHA1(ecd49f3351359ea2d5cbd140c9962d45c5544ecd) )
	ROM_LOAD16_BYTE( "4k.u023", 0x020001, 0x20000, CRC(fddf24da) SHA1(ececed0b0b96d070d85bfb6174029142bc96d5f0) ) // 0x1fffd == 0x02, 0x1fffe == 0xA4

	ROM_REGION( 0x040000, "sub", 0 ) // v30
	ROM_LOAD16_BYTE( "5.u042", 0x000000, 0x20000, CRC(ed03562e) SHA1(bf6b44fb53fa2321cd52c00fcb43b8ceb6ceffff) )
	ROM_LOAD16_BYTE( "6.u043", 0x000001, 0x20000, CRC(a19d5b5d) SHA1(aa5e5be60b737913e5677f88ebc218302245e5af) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "8b.u212",     0x000000, 0x08000, CRC(99ee7505) SHA1(b97c8ee5e26e8554b5de506fba3b32cc2fde53c9) ) // Not encrypted
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x010000, "text", 0 )
	ROM_LOAD16_BYTE( "9",  0x00001, 0x08000, CRC(1922b25e) SHA1(da27122dd1c43770e7385ad602ef397c64d2f754) ) // On some PCBs there is no explicit
	ROM_LOAD16_BYTE( "10", 0x00000, 0x08000, CRC(5f90786a) SHA1(4f63b07c6afbcf5196a433f3356bef984fe303ef) ) // U location for these two ROMs

	ROM_REGION( 0x080000, "bgtiles", 0 )
	ROM_LOAD( "sei420", 0x00000, 0x80000, CRC(da151f0b) SHA1(02682497caf5f058331f18c652471829fa08d54f) ) // tiles @ U105 on this PCB

	ROM_REGION( 0x080000, "fgtiles", 0 )
	ROM_LOAD( "sei430", 0x00000, 0x80000, CRC(ac1f57ac) SHA1(1de926a0db73b99904ef119ac816c53d1551156a) ) // tiles @ U115 on this PCB

	ROM_REGION( 0x090000, "sprites", 0 )
	ROM_LOAD( "sei440",  0x00000, 0x80000, CRC(946d7bde) SHA1(30e8755c2b1ca8bff6278710b8422b51f75eec10) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "7.u203", 0x00000, 0x10000, CRC(8f927822) SHA1(592f2719f2c448c3b4b239eeaec078b411e12dbb) )

	ROM_REGION( 0x2000, "plds", 0 ) // 2x Altera EP910PC-40 (read protected)
	ROM_LOAD( "rd003b.u0168", 0x0000, 0x0884, NO_DUMP )
	ROM_LOAD( "rd006b.u0365", 0x1000, 0x0884, NO_DUMP )

	ROM_REGION( 0x0200, "proms", 0 ) // N82S135N bipolar PROMs
	ROM_LOAD( "rd010.u087", 0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "rd012.u094", 0x0100, 0x0100, NO_DUMP )
ROM_END

ROM_START( raidenkb ) // Korean bootleg board. ROMs for main, sub, audio CPUs, chars and Oki match raidenk, while object and tile ROMs are differently split
	ROM_REGION( 0x060000, "maincpu", 0 ) // v30
	ROM_LOAD16_BYTE( "1.u0253", 0x000000, 0x10000, CRC(a4b12785) SHA1(446314e82ce01315cb3e3d1f323eaa2ad6fb48dd) )
	ROM_LOAD16_BYTE( "2.u0252", 0x000001, 0x10000, CRC(17640bd5) SHA1(5bbc99900426b1a072b52537ae9a50220c378a0d) )
	ROM_LOAD16_BYTE( "3.u022",  0x020000, 0x20000, CRC(f6af09d0) SHA1(ecd49f3351359ea2d5cbd140c9962d45c5544ecd) )
	ROM_LOAD16_BYTE( "4k.u023", 0x020001, 0x20000, CRC(fddf24da) SHA1(ececed0b0b96d070d85bfb6174029142bc96d5f0) ) // 0x1fffd == 0x02, 0x1fffe == 0xA4

	ROM_REGION( 0x040000, "sub", 0 ) // v30
	ROM_LOAD16_BYTE( "5.u042", 0x000000, 0x20000, CRC(ed03562e) SHA1(bf6b44fb53fa2321cd52c00fcb43b8ceb6ceffff) )
	ROM_LOAD16_BYTE( "6.u043", 0x000001, 0x20000, CRC(a19d5b5d) SHA1(aa5e5be60b737913e5677f88ebc218302245e5af) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "8b.u212",     0x000000, 0x08000, CRC(99ee7505) SHA1(b97c8ee5e26e8554b5de506fba3b32cc2fde53c9) ) // Not encrypted
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x010000, "text", 0 )
	ROM_LOAD16_BYTE( "9",  0x00001, 0x08000, CRC(1922b25e) SHA1(da27122dd1c43770e7385ad602ef397c64d2f754) ) // On some PCBs there is no explicit
	ROM_LOAD16_BYTE( "10", 0x00000, 0x08000, CRC(5f90786a) SHA1(4f63b07c6afbcf5196a433f3356bef984fe303ef) ) // U location for these two ROMs

	ROM_REGION( 0x080000, "bgtiles", 0 )
	ROM_LOAD16_BYTE( "rkb15bg.bin", 0x00000, 0x20000, CRC(13a69064) SHA1(a9fcd785e3bac7c0d39be532b3755e6dd45fc314) )
	ROM_LOAD16_BYTE( "rkb17bg.bin", 0x00001, 0x20000, CRC(d7a6c649) SHA1(01d6f18af0385466e3956c3f3afc82393acee6bc) )
	ROM_LOAD16_BYTE( "rkb16bg.bin", 0x40000, 0x20000, CRC(66ea8484) SHA1(f4452e1b0991bf81a60b580ba822fc43b1a443e6) )
	ROM_LOAD16_BYTE( "rkb18bg.bin", 0x40001, 0x20000, CRC(42362d56) SHA1(1cad19fa3f66e34865383d9a94e9058114910365) )

	ROM_REGION( 0x080000, "fgtiles", 0 )
	ROM_LOAD16_BYTE( "rkb7bg.bin",  0x00000, 0x20000, CRC(25239711) SHA1(978cfc6487ed711cc1b513824741c347ec92889d) )
	ROM_LOAD16_BYTE( "rkb9bg.bin",  0x00001, 0x20000, CRC(6ca0d7b3) SHA1(ef63657a01b07aaa0ded7b0d405b872b4d3a56a8) )
	ROM_LOAD16_BYTE( "rkb8bg.bin",  0x40000, 0x20000, CRC(3cad38fc) SHA1(de2257f70c3e71905bc959f80be183c6d95fd06d) )
	ROM_LOAD16_BYTE( "rkb10bg.bin", 0x40001, 0x20000, CRC(6fce95a3) SHA1(1d3beda3a4dd0a2a3afbb7b5b16d87bf3257bcb4) )

	ROM_REGION( 0x090000, "sprites", 0 )
	ROM_LOAD16_BYTE( "rkb19obj.bin", 0x00000, 0x20000, CRC(34fa4485) SHA1(d9893c484ee4f80e364824500c6c048f58f49752) )
	ROM_LOAD16_BYTE( "rkb21obj.bin", 0x00001, 0x20000, CRC(d806395b) SHA1(7c6fc848aa40a49590e00d0b02ce21ad5414e387) )
	ROM_LOAD16_BYTE( "rkb20obj.bin", 0x40000, 0x20000, CRC(8b7ca3c6) SHA1(81c3e98cbd81a39e04b5e7fb3683aba50545f774) )
	ROM_LOAD16_BYTE( "rkb22obj.bin", 0x40001, 0x20000, CRC(82ee78a0) SHA1(4af0593f9c7d8db59f17d75d6f9020ecd4bdcb98) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "7.u203", 0x00000, 0x10000, CRC(8f927822) SHA1(592f2719f2c448c3b4b239eeaec078b411e12dbb) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.h7", 0x0000, 0x0200, NO_DUMP )
ROM_END

ROM_START( raidenb )// Different hardware, Main & Sub CPU code not encrypted.
	ROM_REGION( 0x060000, "maincpu", 0 ) // v30
	ROM_LOAD16_BYTE( "1.u0253", 0x000000, 0x10000, CRC(a4b12785) SHA1(446314e82ce01315cb3e3d1f323eaa2ad6fb48dd) )
	ROM_LOAD16_BYTE( "2.u0252", 0x000001, 0x10000, CRC(17640bd5) SHA1(5bbc99900426b1a072b52537ae9a50220c378a0d) )
	ROM_LOAD16_BYTE( "3__,raidenb.u022", 0x020000, 0x20000, CRC(9d735bf5) SHA1(531981eac2ef0c0635f067a649899f98738d5c67) ) // Simply labeled as 3
	ROM_LOAD16_BYTE( "4__,raidenb.u023", 0x020001, 0x20000, CRC(8d184b99) SHA1(71cd4179aa2341d2ceecbb6a9c26f5919d46ca4c) ) // Simply labeled as 4; 0x1fffd == 0x00, 0x1fffe == 0x80

	ROM_REGION( 0x040000, "sub", 0 ) // v30
	ROM_LOAD16_BYTE( "5__,raidenb.u042", 0x000000, 0x20000, CRC(7aca6d61) SHA1(4d80ec87e54d7495b9bdf819b9985b1c8183c80d) ) // Simply labeled as 5
	ROM_LOAD16_BYTE( "6__,raidenb.u043", 0x000001, 0x20000, CRC(e3d35cc2) SHA1(4329865985aaf3fb524618e2e958563c8fa6ead5) ) // Simply labeled as 6

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "rai6.u212",   0x000000, 0x08000, CRC(723a483b) SHA1(50e67945e83ea1748fb748de3287d26446d4e0a0) ) // Should be labeled "8" ???
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x010000, "text", 0 )
	ROM_LOAD16_BYTE( "9",  0x00001, 0x08000, CRC(1922b25e) SHA1(da27122dd1c43770e7385ad602ef397c64d2f754) ) // On some PCBs there is no explicit
	ROM_LOAD16_BYTE( "10", 0x00000, 0x08000, CRC(5f90786a) SHA1(4f63b07c6afbcf5196a433f3356bef984fe303ef) ) // U location for these two ROMs

	ROM_REGION( 0x080000, "bgtiles", 0 )
	ROM_LOAD( "sei420", 0x00000, 0x80000, CRC(da151f0b) SHA1(02682497caf5f058331f18c652471829fa08d54f) ) // U919 on this PCB

	ROM_REGION( 0x080000, "fgtiles", 0 )
	ROM_LOAD( "sei430", 0x00000, 0x80000, CRC(ac1f57ac) SHA1(1de926a0db73b99904ef119ac816c53d1551156a) ) // U920 on this PCB

	ROM_REGION( 0x090000, "sprites", 0 )
	ROM_LOAD( "sei440", 0x00000, 0x80000, CRC(946d7bde) SHA1(30e8755c2b1ca8bff6278710b8422b51f75eec10) ) // U165 on this PCB

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "7.u203", 0x00000, 0x10000, CRC(8f927822) SHA1(592f2719f2c448c3b4b239eeaec078b411e12dbb) )

	ROM_REGION( 0x0100, "proms", 0 ) // N82S135N bipolar PROM
	ROM_LOAD( "jj3010.u0116", 0x0000, 0x0100, NO_DUMP )
ROM_END

ROM_START( raidenub ) // only region bits differ from raidenb
	ROM_REGION( 0x060000, "maincpu", 0 ) // v30
	ROM_LOAD16_BYTE( "1.u0253", 0x000000, 0x10000, CRC(a4b12785) SHA1(446314e82ce01315cb3e3d1f323eaa2ad6fb48dd) )
	ROM_LOAD16_BYTE( "2.u0252", 0x000001, 0x10000, CRC(17640bd5) SHA1(5bbc99900426b1a072b52537ae9a50220c378a0d) )
	ROM_LOAD16_BYTE( "3u.u022", 0x020000, 0x20000, CRC(9d735bf5) SHA1(531981eac2ef0c0635f067a649899f98738d5c67) ) // Simply labeled as 3u
	ROM_LOAD16_BYTE( "4u.u023", 0x020001, 0x20000, CRC(95c110ef) SHA1(e6aea374ca63cdd851af66240e51461882d170e8) ) // Simply labeled as 4u; 0x1fffd == 0x01, 0x1fffe == 0x81

	ROM_REGION( 0x040000, "sub", 0 ) // v30
	ROM_LOAD16_BYTE( "5__,raidenb.u042", 0x000000, 0x20000, CRC(7aca6d61) SHA1(4d80ec87e54d7495b9bdf819b9985b1c8183c80d) ) // Simply labeled as 5
	ROM_LOAD16_BYTE( "6__,raidenb.u043", 0x000001, 0x20000, CRC(e3d35cc2) SHA1(4329865985aaf3fb524618e2e958563c8fa6ead5) ) // Simply labeled as 6

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "rai6.u212",   0x000000, 0x08000, CRC(723a483b) SHA1(50e67945e83ea1748fb748de3287d26446d4e0a0) ) // Should be labeled "8" ???
	ROM_CONTINUE(            0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x010000, "text", 0 )
	ROM_LOAD16_BYTE( "9",  0x00001, 0x08000, CRC(1922b25e) SHA1(da27122dd1c43770e7385ad602ef397c64d2f754) ) // On some PCBs there is no explicit
	ROM_LOAD16_BYTE( "10", 0x00000, 0x08000, CRC(5f90786a) SHA1(4f63b07c6afbcf5196a433f3356bef984fe303ef) ) // U location for these two ROMs

	ROM_REGION( 0x080000, "bgtiles", 0 )
	ROM_LOAD( "sei420", 0x00000, 0x80000, CRC(da151f0b) SHA1(02682497caf5f058331f18c652471829fa08d54f) ) // U919 on this PCB

	ROM_REGION( 0x080000, "fgtiles", 0 )
	ROM_LOAD( "sei430", 0x00000, 0x80000, CRC(ac1f57ac) SHA1(1de926a0db73b99904ef119ac816c53d1551156a) ) // U920 on this PCB

	ROM_REGION( 0x090000, "sprites", 0 )
	ROM_LOAD( "sei440", 0x00000, 0x80000, CRC(946d7bde) SHA1(30e8755c2b1ca8bff6278710b8422b51f75eec10) ) // U165 on this PCB

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "7.u203", 0x00000, 0x10000, CRC(8f927822) SHA1(592f2719f2c448c3b4b239eeaec078b411e12dbb) )

	ROM_REGION( 0x0100, "proms", 0 ) // N82S135N bipolar PROM
	ROM_LOAD( "jj3010.u0116", 0x0000, 0x0100, NO_DUMP )
ROM_END

ROM_START( raidenua )// Different hardware, Main, Sub & sound CPU code not encrypted.
	ROM_REGION( 0x060000, "maincpu", 0 ) // v30
	ROM_LOAD16_BYTE( "1.c8",   0x000000, 0x10000, CRC(a4b12785) SHA1(446314e82ce01315cb3e3d1f323eaa2ad6fb48dd) )
	ROM_LOAD16_BYTE( "2.c7",   0x000001, 0x10000, CRC(17640bd5) SHA1(5bbc99900426b1a072b52537ae9a50220c378a0d) )
	ROM_LOAD16_BYTE( "3dd.e8", 0x020000, 0x20000, CRC(b6f3bad2) SHA1(214474ab9fa65e2716155b77d7825951cc98148a) )
	ROM_LOAD16_BYTE( "4dd.e7", 0x020001, 0x20000, CRC(d294dfc1) SHA1(03606ddfa35d5cb34c447fa370495e1fbb0cad0e) ) // 0x1fffd == 0x01, 0x1fffe == 0x81

	ROM_REGION( 0x040000, "sub", 0 ) // v30
	ROM_LOAD16_BYTE( "5.p8", 0x000000, 0x20000, CRC(15c1cf45) SHA1(daac732a1d3e8f36fa665f984e05651cbca74fef) )
	ROM_LOAD16_BYTE( "6.p7", 0x000001, 0x20000, CRC(261c381b) SHA1(64a9e0ea9abcba6287829cf4abb806362b62c806) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "8.w8",        0x00000, 0x08000, CRC(105b9c11) SHA1(eb142806f8410d584d914b91207361a15ab18e6f) )
	ROM_CONTINUE(            0x10000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000, 0x18000, 0x08000 )

	ROM_REGION( 0x010000, "text", 0 )
	ROM_LOAD16_BYTE( "9",  0x00001, 0x08000, CRC(1922b25e) SHA1(da27122dd1c43770e7385ad602ef397c64d2f754) ) // U016 on this PCB
	ROM_LOAD16_BYTE( "10", 0x00000, 0x08000, CRC(5f90786a) SHA1(4f63b07c6afbcf5196a433f3356bef984fe303ef) ) // U017 on this PCB

	ROM_REGION( 0x080000, "bgtiles", 0 )
	ROM_LOAD( "sei420", 0x00000, 0x80000, CRC(da151f0b) SHA1(02682497caf5f058331f18c652471829fa08d54f) ) // U011 on this PCB

	ROM_REGION( 0x080000, "fgtiles", 0 )
	ROM_LOAD( "sei430", 0x00000, 0x80000, CRC(ac1f57ac) SHA1(1de926a0db73b99904ef119ac816c53d1551156a) ) // U013 on this PCB

	ROM_REGION( 0x090000, "sprites", 0 )
	ROM_LOAD( "sei440",  0x00000, 0x80000, CRC(946d7bde) SHA1(30e8755c2b1ca8bff6278710b8422b51f75eec10) ) // U012 on this PCB

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "7.x10", 0x00000, 0x10000, CRC(2051263e) SHA1(dff96caa11adf619360d88704e3af8427ddfe524) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "prom.n2", 0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "prom.u3", 0x0100, 0x0100, NO_DUMP )
ROM_END


/***************************************************************************/

/*
  This is based on code by Niclas Karlsson Mate, who figured out the
  encryption method! The technique is a combination of a XOR table plus
  bit-swapping
*/

void raiden_state::init_decryption()
{
	u16 *rom = (u16 *)memregion("maincpu")->base();

	for (int i = 0; i < 0x20000; i++)
	{
		static const u16 xor_table[] = { 0x200e, 0x0006, 0x000a, 0x0002, 0x240e, 0x000e, 0x04c2, 0x00c2, 0x008c, 0x0004, 0x0088, 0x0000, 0x048c, 0x000c, 0x04c0, 0x00c0 };
		u16 data = rom[0x20000 / 2 + i];
		data ^= xor_table[i & 0x0f];
		data = bitswap<16>(data, 15, 14, 10, 12, 11, 13, 9, 8, 3, 2, 5, 4, 7, 1, 6, 0);
		rom[0x20000 / 2 + i] = data;
	}

	rom = (u16 *)memregion("sub")->base();

	for (int i = 0; i < 0x20000; i++)
	{
		static const u16 xor_table[] = { 0x0080, 0x0080, 0x0244, 0x0288, 0x0288, 0x0288, 0x1041, 0x1009 };
		u16 data = rom[0x00000 / 2 + i];
		data ^= xor_table[i & 0x07];
		data = bitswap<16>(data, 15, 14, 13, 9, 11, 10, 12, 8, 2, 0, 5, 4, 7, 3, 1, 6);
		rom[0x00000 / 2 + i] = data;
	}
}

} // anonymous namespace


/***************************************************************************/

// Same PCB, differ by region byte(s)
GAME( 1990, raiden,   0,      raidene,  raiden, raiden_state,  init_decryption,  ROT270, "Seibu Kaihatsu",                                 "Raiden (World set 1)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1990, raidenj,  raiden, raidene,  raiden, raiden_state,  init_decryption,  ROT270, "Seibu Kaihatsu",                                 "Raiden (Japan)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1990, raidenu,  raiden, raidene,  raiden, raiden_state,  init_decryption,  ROT270, "Seibu Kaihatsu (Fabtek license)",                "Raiden (US set 1)",                    MACHINE_SUPPORTS_SAVE )
GAME( 1990, raident,  raiden, raidene,  raiden, raiden_state,  init_decryption,  ROT270, "Seibu Kaihatsu (Liang HWA Electronics license)", "Raiden (Taiwan)",                      MACHINE_SUPPORTS_SAVE )

// Same as above, but the sound CPU code is not encrypted
GAME( 1990, raidenk,  raiden, raiden,   raiden, raiden_state,  init_decryption,  ROT270, "Seibu Kaihatsu (IBL Corporation license)",       "Raiden (Korea)",                       MACHINE_SUPPORTS_SAVE )

// Bootleg of the Korean release
// real hw has heavy slow downs, sometimes making the game borderline unplayable (https://www.youtube.com/watch?v=_FF4N9mBxao)
GAME( 1990, raidenkb, raiden, raidenkb, raiden, raiden_state,  init_decryption,  ROT270, "bootleg",                                        "Raiden (Korea, bootleg)",              MACHINE_SUPPORTS_SAVE )

// Alternate hardware; SEI8904 + SEI9008 PCBs. Main & Sub CPU code not encrypted
GAME( 1990, raidenua, raiden, raidenu,  raiden, raiden_state,  empty_init,       ROT270, "Seibu Kaihatsu (Fabtek license)",                "Raiden (US set 2, SEI8904 hardware)",  MACHINE_SUPPORTS_SAVE )

// Alternate hardware. Main, Sub & Sound CPU code not encrypted. It also sports a Seibu custom CRTC.
GAME( 1990, raidenb,  raiden, raidenb,  raiden, raidenb_state, empty_init,       ROT270, "Seibu Kaihatsu",                                 "Raiden (World set 2, newer hardware)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, raidenub, raiden, raidenb,  raiden, raidenb_state, empty_init,       ROT270, "Seibu Kaihatsu (Fabtek license)",                "Raiden (US set 3, newer hardware)",    MACHINE_SUPPORTS_SAVE )

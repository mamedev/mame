// license:BSD-3-Clause
// copyright-holders:Yochizo

#include "machine/timer.h"
#include "jalblend.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class argus_common_state : public driver_device
{
protected:
	argus_common_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_blend(*this, "blend")
		, m_bg_scrollx(*this, "bg%u_scrollx", 0U)
		, m_bg_scrolly(*this, "bg%u_scrolly", 0U)
		, m_paletteram(*this, "paletteram")
		, m_txram(*this, "txram")
		, m_bg1ram(*this, "bg1ram")
		, m_spriteram(*this, "spriteram")
		, m_vrom(*this, "vrom%u", 1U)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<jaleco_blend_device> m_blend;

	optional_shared_ptr_array<u8, 2> m_bg_scrollx;
	optional_shared_ptr_array<u8, 2> m_bg_scrolly;
	required_shared_ptr<u8> m_paletteram;
	optional_shared_ptr<u8> m_txram;
	optional_shared_ptr<u8> m_bg1ram;
	required_shared_ptr<u8> m_spriteram;
	optional_region_ptr_array<u8, 2> m_vrom;

	// common
	u8 m_bg_status = 0U;
	u8 m_flipscreen = 0U;
	u16 m_palette_intensity = 0U;

	// argus specific
	u8 m_vrom_offset = 0U;

	tilemap_t *m_tx_tilemap = nullptr;
	tilemap_t *m_bg_tilemap[2]{};

	// common
	void bankselect_w(u8 data);
	void txram_w(offs_t offset, u8 data);
	void bg1ram_w(offs_t offset, u8 data);
	void flipscreen_w(u8 data);

	template<int Gfx> TILE_GET_INFO_MEMBER(get_tx_tile_info);

	virtual void machine_start() override ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	void reset_common();
	void change_palette(int color, int lo_offs, int hi_offs);
	void change_bg_palette(int color, int lo_offs, int hi_offs);
	void bg_setting();

	void sound_map_a(address_map &map) ATTR_COLD;
	void sound_map_b(address_map &map) ATTR_COLD;
	void sound_portmap_1(address_map &map) ATTR_COLD;
	void sound_portmap_2(address_map &map) ATTR_COLD;
};

class argus_state : public argus_common_state
{
public:
	argus_state(const machine_config &mconfig, device_type type, const char *tag)
		: argus_common_state(mconfig, type, tag)
	{ }

	void argus(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;

private:
	void bg_status_w(u8 data);
	void paletteram_w(offs_t offset, u8 data);

	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);

	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, int priority);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void argus_map(address_map &map) ATTR_COLD;
};

class valtric_state : public argus_common_state
{
public:
	valtric_state(const machine_config &mconfig, device_type type, const char *tag)
		: argus_common_state(mconfig, type, tag)
	{ }

	void valtric(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;

private:
	void mosaic_w(u8 data);
	void bg_status_w(u8 data);
	void paletteram_w(offs_t offset, u8 data);
	void unknown_w(u8 data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void draw_mosaic(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void valtric_map(address_map &map) ATTR_COLD;

	u8 m_valtric_mosaic = 0U;
	bitmap_rgb32 m_mosaicbitmap = 0;
	u8 m_valtric_unknown = 0U;
	int m_mosaic = 0;
};

class butasan_state : public argus_common_state
{
public:
	butasan_state(const machine_config &mconfig, device_type type, const char *tag)
		: argus_common_state(mconfig, type, tag)
		, m_butasan_bg1ram(*this, "butasan_bg1ram")
	{ }

	void butasan(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;

private:
	void bg0_status_w(u8 data);
	void bg1_status_w(u8 data);
	void paletteram_w(offs_t offset, u8 data);
	void bg1ram_w(offs_t offset, u8 data);
	void pageselect_w(u8 data);
	u8 pagedram_r(offs_t offset);
	void pagedram_w(offs_t offset, u8 data);
	void unknown_w(u8 data);

	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILEMAP_MAPPER_MEMBER(bg_scan);
	TILEMAP_MAPPER_MEMBER(tx_scan);

	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void log_vram();
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	void butasan_map(address_map &map) ATTR_COLD;

	required_shared_ptr<u8> m_butasan_bg1ram;

	u8 *m_butasan_txram = nullptr;
	u8 *m_butasan_bg0ram = nullptr;
	u8 *m_butasan_bg0backram = nullptr;
	u8 *m_butasan_txbackram = nullptr;
	std::unique_ptr<u8[]> m_butasan_pagedram[2]{};
	u8 m_butasan_page_latch = 0U;
	u8 m_butasan_bg1_status = 0U;
	u8 m_butasan_unknown = 0U;
};

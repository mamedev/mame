// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "video/bufsprite.h"
#include "decospr.h"
#include "deco146.h"
#include "machine/gen_latch.h"
#include "emupal.h"
#include "tilemap.h"

class lemmings_state : public driver_device
{
public:
	lemmings_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_bitmap0(2048, 256)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_deco146(*this, "ioprot")
		, m_spriteram(*this, "spriteram%u", 1)
		, m_sprgen(*this, "spritegen%u", 1)
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_control_data(*this, "control_data")
		, m_vram_data(*this, "vram_data")
		, m_pixel_data(*this, "pixel_%u_data", 0)
		, m_trackball_io(*this, "AN%u", 0)
	{
	}

	void lemmings(machine_config &config);

private:
	/* video-related */
	bitmap_ind16 m_bitmap0{};
	tilemap_t *m_vram_tilemap = nullptr;
	std::unique_ptr<uint16_t[]> m_sprite_triple_buffer[2]{};
	std::unique_ptr<uint8_t[]> m_vram_buffer{};

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<deco146_device> m_deco146;
	required_device_array<buffered_spriteram16_device, 2> m_spriteram;
	required_device_array<decospr_device, 2> m_sprgen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_control_data;
	required_shared_ptr<uint16_t> m_vram_data;
	required_shared_ptr_array<uint16_t, 2> m_pixel_data;

	required_ioport_array<4> m_trackball_io;

	void lemmings_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t lemmings_trackball_r(offs_t offset);
	void lemmings_pixel_0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void lemmings_pixel_1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void lemmings_vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void video_start() override;
	uint32_t screen_update_lemmings(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_lemmings);
	void lemmings_copy_bitmap(bitmap_rgb32& bitmap, int* xscroll, int* yscroll, const rectangle& cliprect);

	uint16_t lem_protection_region_0_146_r(offs_t offset);
	void lem_protection_region_0_146_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void lemmings_map(address_map &map);
	void sound_map(address_map &map);
};

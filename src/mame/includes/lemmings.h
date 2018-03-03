// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "video/bufsprite.h"
#include "video/decospr.h"
#include "machine/deco146.h"
#include "machine/gen_latch.h"

class lemmings_state : public driver_device
{
public:
	lemmings_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bitmap0(2048, 256),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_deco146(*this, "ioprot"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_sprgen(*this, "spritegen"),
		m_sprgen2(*this, "spritegen2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_control_data(*this, "control_data"),
		m_vram_data(*this, "vram_data"),
		m_pixel_0_data(*this, "pixel_0_data"),
		m_pixel_1_data(*this, "pixel_1_data") { }

	/* video-related */
	bitmap_ind16 m_bitmap0;
	tilemap_t *m_vram_tilemap;
	uint16_t m_sprite_triple_buffer_0[0x800];
	uint16_t m_sprite_triple_buffer_1[0x800];
	uint8_t m_vram_buffer[2048 * 64]; // 64 bytes per VRAM character

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<deco146_device> m_deco146;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<buffered_spriteram16_device> m_spriteram2;
	required_device<decospr_device> m_sprgen;
	required_device<decospr_device> m_sprgen2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_control_data;
	required_shared_ptr<uint16_t> m_vram_data;
	required_shared_ptr<uint16_t> m_pixel_0_data;
	required_shared_ptr<uint16_t> m_pixel_1_data;

	DECLARE_WRITE16_MEMBER(lemmings_control_w);
	DECLARE_READ16_MEMBER(lemmings_trackball_r);
	DECLARE_WRITE16_MEMBER(lemmings_pixel_0_w);
	DECLARE_WRITE16_MEMBER(lemmings_pixel_1_w);
	DECLARE_WRITE16_MEMBER(lemmings_vram_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void video_start() override;
	uint32_t screen_update_lemmings(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_lemmings);
	void lemmings_copy_bitmap(bitmap_rgb32& bitmap, bitmap_ind16& srcbitmap, int* xscroll, int* yscroll, const rectangle& cliprect);

	DECLARE_READ16_MEMBER( lem_protection_region_0_146_r );
	DECLARE_WRITE16_MEMBER( lem_protection_region_0_146_w );
	void lemmings(machine_config &config);
	void lemmings_map(address_map &map);
	void sound_map(address_map &map);
};

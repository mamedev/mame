// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "video/bufsprite.h"
#include "video/decospr.h"
#include "machine/deco146.h"

class lemmings_state : public driver_device
{
public:
	lemmings_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_bitmap0(2048, 256),
		m_audiocpu(*this, "audiocpu"),
		m_deco146(*this, "ioprot"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2") ,
		m_control_data(*this, "control_data"),
		m_vram_data(*this, "vram_data"),
		m_pixel_0_data(*this, "pixel_0_data"),
		m_pixel_1_data(*this, "pixel_1_data"),
		m_sprgen(*this, "spritegen"),
		m_sprgen2(*this, "spritegen2"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* video-related */
	bitmap_ind16 m_bitmap0;
	tilemap_t *m_vram_tilemap;
	UINT16 m_sprite_triple_buffer_0[0x800];
	UINT16 m_sprite_triple_buffer_1[0x800];
	UINT8 m_vram_buffer[2048 * 64]; // 64 bytes per VRAM character
	/* devices */
	required_device<cpu_device> m_audiocpu;
	optional_device<deco146_device> m_deco146;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<buffered_spriteram16_device> m_spriteram2;
	/* memory pointers */
	required_shared_ptr<UINT16> m_control_data;
	required_shared_ptr<UINT16> m_vram_data;
	required_shared_ptr<UINT16> m_pixel_0_data;
	required_shared_ptr<UINT16> m_pixel_1_data;
	optional_device<decospr_device> m_sprgen;
	optional_device<decospr_device> m_sprgen2;

	DECLARE_WRITE16_MEMBER(lemmings_control_w);
	DECLARE_READ16_MEMBER(lemmings_trackball_r);
	void lemmings_sound_cb( address_space &space, UINT16 data, UINT16 mem_mask );
	DECLARE_WRITE8_MEMBER(lemmings_sound_ack_w);
	DECLARE_WRITE16_MEMBER(lemmings_pixel_0_w);
	DECLARE_WRITE16_MEMBER(lemmings_pixel_1_w);
	DECLARE_WRITE16_MEMBER(lemmings_vram_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void video_start() override;
	UINT32 screen_update_lemmings(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_lemmings(screen_device &screen, bool state);
	void lemmings_copy_bitmap(bitmap_rgb32& bitmap, bitmap_ind16& srcbitmap, int* xscroll, int* yscroll, const rectangle& cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_READ16_MEMBER( lem_protection_region_0_146_r );
	DECLARE_WRITE16_MEMBER( lem_protection_region_0_146_w );
};

// license:BSD-3-Clause
// copyright-holders:Chris Hardy

#include "sound/sn76496.h"
#include "sound/vlm5030.h"

class hyperspt_state : public driver_device
{
public:
	hyperspt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_scroll(*this, "scroll"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_sn(*this, "snsnd"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<sn76496_device> m_sn;
	optional_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t *m_bg_tilemap;

	uint8_t m_irq_mask;
	uint8_t m_SN76496_latch;

	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);
	DECLARE_WRITE_LINE_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	DECLARE_WRITE8_MEMBER(konami_SN76496_latch_w) { m_SN76496_latch = data; };
	DECLARE_WRITE8_MEMBER(konami_SN76496_w) { m_sn->write(space, offset, m_SN76496_latch); };

	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(hyperspt);
	DECLARE_VIDEO_START(roadf);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(roadf_get_bg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void hyperspt(machine_config &config);
	void roadf(machine_config &config);
	void hypersptb(machine_config &config);
	void common_map(address_map &map);
	void common_sound_map(address_map &map);
	void hyperspt_map(address_map &map);
	void hyperspt_sound_map(address_map &map);
	void roadf_map(address_map &map);
	void roadf_sound_map(address_map &map);
	void soundb_map(address_map &map);
	void hyprolyb_adpcm_map(address_map &map);
};

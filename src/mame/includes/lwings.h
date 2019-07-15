// license:BSD-3-Clause
// copyright-holders:Paul Leaman

#include "video/bufsprite.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "emupal.h"

class lwings_state : public driver_device
{
public:
	lwings_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bg1videoram(*this, "bg1videoram"),
		m_soundlatch2(*this, "soundlatch_2"),
		m_nmi_mask(0),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_msm(*this, "5205"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	void lwings(machine_config &config);
	void trojan(machine_config &config);
	void fball(machine_config &config);
	void avengers(machine_config &config);
	void avengersb(machine_config &config);

	void init_avengersb();

private:
	/* memory pointers */
	required_device<buffered_spriteram8_device> m_spriteram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bg1videoram;
	optional_shared_ptr<uint8_t> m_soundlatch2;

	/* video-related */
	tilemap_t  *m_fg_tilemap;
	tilemap_t  *m_bg1_tilemap;
	tilemap_t  *m_bg2_tilemap;
	uint8_t    m_bg2_image;
	int      m_bg2_avenger_hw;
	int      m_spr_avenger_hw;
	uint8_t    m_scroll_x[2];
	uint8_t    m_scroll_y[2];

	/* misc */
	uint8_t    m_param[4];
	int      m_palette_pen;
	uint8_t    m_soundstate;
	uint8_t    m_adpcm;
	uint8_t    m_nmi_mask;
	int      m_sprbank;

	DECLARE_WRITE8_MEMBER(avengers_adpcm_w);
	DECLARE_READ8_MEMBER(avengers_adpcm_r);
	DECLARE_WRITE8_MEMBER(lwings_bankswitch_w);
	DECLARE_WRITE8_MEMBER(avengers_protection_w);
	DECLARE_WRITE8_MEMBER(avengers_prot_bank_w);
	DECLARE_READ8_MEMBER(avengers_protection_r);
	DECLARE_READ8_MEMBER(avengers_soundlatch2_r);
	DECLARE_WRITE8_MEMBER(lwings_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(lwings_bg1videoram_w);
	DECLARE_WRITE8_MEMBER(lwings_bg1_scrollx_w);
	DECLARE_WRITE8_MEMBER(lwings_bg1_scrolly_w);
	DECLARE_WRITE8_MEMBER(trojan_bg2_scrollx_w);
	DECLARE_WRITE8_MEMBER(trojan_bg2_image_w);
	DECLARE_WRITE8_MEMBER(msm5205_w);
	DECLARE_WRITE8_MEMBER(fball_oki_bank_w);

	TILEMAP_MAPPER_MEMBER(get_bg2_memory_offset);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(lwings_get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(trojan_get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_VIDEO_START(trojan);
	DECLARE_VIDEO_START(avengers);
	DECLARE_VIDEO_START(avengersb);
	uint32_t screen_update_lwings(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_trojan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(lwings_interrupt);
	INTERRUPT_GEN_MEMBER(avengers_interrupt);
	inline int is_sprite_on( uint8_t *buffered_spriteram, int offs );
	void lwings_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void trojan_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	int avengers_fetch_paldata(  );
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	void avengers_adpcm_io_map(address_map &map);
	void avengers_map(address_map &map);
	void fball_map(address_map &map);
	void fball_oki_map(address_map &map);
	void fball_sound_map(address_map &map);
	void lwings_map(address_map &map);
	void lwings_sound_map(address_map &map);
	void trojan_adpcm_io_map(address_map &map);
	void trojan_adpcm_map(address_map &map);
	void trojan_map(address_map &map);
};

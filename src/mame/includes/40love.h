// license:GPL-2.0+
// copyright-holders:Jarek Burczynski

#include "machine/taito68705interface.h"
#include "machine/gen_latch.h"
#include "sound/msm5232.h"
#include "sound/ay8910.h"
#include "sound/ta7630.h"

class fortyl_state : public driver_device
{
public:
	fortyl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_video_ctrl(*this, "video_ctrl"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_spriteram2(*this, "spriteram2"),
		m_mcu_ram(*this, "mcu_ram"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_bmcu(*this, "bmcu"),
		m_msm(*this, "msm"),
		m_ay(*this,"aysnd"),
		m_ta7630(*this,"ta7630"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch2(*this, "soundlatch2") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_video_ctrl;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram2;
	optional_shared_ptr<uint8_t> m_mcu_ram;

	/* video-related */
	std::unique_ptr<bitmap_ind16>    m_tmp_bitmap1;
	std::unique_ptr<bitmap_ind16>    m_tmp_bitmap2;
	tilemap_t     *m_bg_tilemap;
	uint8_t       m_flipscreen;
	uint8_t       m_pix_redraw;
	uint8_t       m_xoffset;
	std::unique_ptr<uint8_t[]>       m_pixram1;
	std::unique_ptr<uint8_t[]>       m_pixram2;
	bitmap_ind16    *m_pixel_bitmap1;
	bitmap_ind16    *m_pixel_bitmap2;
	int         m_pixram_sel;
	bool        m_color_bank;
	bool        m_screen_disable;

	/* misc */
	int         m_pix_color[4];
	uint8_t       m_pix1;
	uint8_t       m_pix2[2];
	int         m_vol_ctrl[16];
	uint8_t       m_snd_ctrl0;
	uint8_t       m_snd_ctrl1;
	uint8_t       m_snd_ctrl2;
	uint8_t       m_snd_ctrl3;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_maincpu;
	optional_device<taito68705_mcu_device> m_bmcu;
	required_device<msm5232_device> m_msm;
	required_device<ay8910_device> m_ay;
	required_device<ta7630_device> m_ta7630;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch2;

	DECLARE_WRITE8_MEMBER(bank_select_w);
	DECLARE_WRITE8_MEMBER(pix1_w);
	DECLARE_WRITE8_MEMBER(pix2_w);
	DECLARE_READ8_MEMBER(pix2_r);
	DECLARE_READ8_MEMBER(snd_flag_r);
	DECLARE_READ8_MEMBER(fortyl_mcu_status_r);
	DECLARE_WRITE8_MEMBER(fortyl_pixram_sel_w);
	DECLARE_READ8_MEMBER(fortyl_pixram_r);
	DECLARE_WRITE8_MEMBER(fortyl_pixram_w);
	DECLARE_WRITE8_MEMBER(fortyl_bg_videoram_w);
	DECLARE_READ8_MEMBER(fortyl_bg_videoram_r);
	DECLARE_WRITE8_MEMBER(fortyl_bg_colorram_w);
	DECLARE_READ8_MEMBER(fortyl_bg_colorram_r);
	DECLARE_WRITE8_MEMBER(pix1_mcu_w);
	DECLARE_WRITE8_MEMBER(sound_control_0_w);
	DECLARE_WRITE8_MEMBER(sound_control_1_w);
	DECLARE_WRITE8_MEMBER(sound_control_2_w);
	DECLARE_WRITE8_MEMBER(sound_control_3_w);
	void init_undoukai();
	void init_40love();
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	DECLARE_MACHINE_START(40love);
	DECLARE_MACHINE_RESET(40love);
	DECLARE_MACHINE_RESET(common);
	uint32_t screen_update_fortyl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void redraw_pixels();
	void fortyl_set_scroll_x( int offset );
	void fortyl_plot_pix( int offset );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_pixram( bitmap_ind16 &bitmap, const rectangle &cliprect );

	void undoukai(machine_config &config);
	void _40love(machine_config &config);
	void _40love_map(address_map &map);
	void sound_map(address_map &map);
	void undoukai_map(address_map &map);
};

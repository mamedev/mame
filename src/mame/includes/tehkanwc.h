// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Roberto Fresca
#include "sound/msm5205.h"

class tehkanwc_state : public driver_device
{
public:
	enum
	{
		TIMER_RESET
	};

	tehkanwc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_spriteram;

	int m_track0[2];
	int m_track1[2];
	int m_msm_data_offs;
	int m_toggle;
	UINT8 m_scroll_x[2];
	UINT8 m_led0;
	UINT8 m_led1;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	DECLARE_WRITE8_MEMBER(sub_cpu_halt_w);
	DECLARE_READ8_MEMBER(track_0_r);
	DECLARE_READ8_MEMBER(track_1_r);
	DECLARE_WRITE8_MEMBER(track_0_reset_w);
	DECLARE_WRITE8_MEMBER(track_1_reset_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(sound_answer_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_WRITE8_MEMBER(videoram2_w);
	DECLARE_WRITE8_MEMBER(scroll_x_w);
	DECLARE_WRITE8_MEMBER(scroll_y_w);
	DECLARE_WRITE8_MEMBER(flipscreen_x_w);
	DECLARE_WRITE8_MEMBER(flipscreen_y_w);
	DECLARE_WRITE8_MEMBER(gridiron_led0_w);
	DECLARE_WRITE8_MEMBER(gridiron_led1_w);
	DECLARE_READ8_MEMBER(portA_r);
	DECLARE_READ8_MEMBER(portB_r);
	DECLARE_WRITE8_MEMBER(portA_w);
	DECLARE_WRITE8_MEMBER(portB_w);
	DECLARE_WRITE8_MEMBER(msm_reset_w);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	DECLARE_DRIVER_INIT(teedoff);
	virtual void machine_start() override;
	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gridiron_draw_led(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 led,int player);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

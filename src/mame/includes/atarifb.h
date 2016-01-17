// license:???
// copyright-holders:Mike Balfour, Patrick Lawrence, Brad Oliver
/*************************************************************************

    Atari Football hardware

*************************************************************************/

#include "sound/discrete.h"


/* Discrete Sound Input Nodes */
#define ATARIFB_WHISTLE_EN      NODE_01
#define ATARIFB_CROWD_DATA      NODE_02
#define ATARIFB_ATTRACT_EN      NODE_03
#define ATARIFB_NOISE_EN        NODE_04
#define ATARIFB_HIT_EN          NODE_05


class atarifb_state : public driver_device
{
public:
	atarifb_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_alphap1_videoram(*this, "p1_videoram"),
		m_alphap2_videoram(*this, "p2_videoram"),
		m_field_videoram(*this, "field_videoram"),
		m_spriteram(*this, "spriteram"),
		m_scroll_register(*this, "scroll_register"),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"){ }

	/* video-related */
	required_shared_ptr<UINT8> m_alphap1_videoram;
	required_shared_ptr<UINT8> m_alphap2_videoram;
	required_shared_ptr<UINT8> m_field_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_scroll_register;

	tilemap_t  *m_alpha1_tilemap;
	tilemap_t  *m_alpha2_tilemap;
	tilemap_t  *m_field_tilemap;

	/* sound-related */
	int m_CTRLD;
	int m_sign_x_1;
	int m_sign_y_1;
	int m_sign_x_2;
	int m_sign_y_2;
	int m_sign_x_3;
	int m_sign_y_3;
	int m_sign_x_4;
	int m_sign_y_4;
	int m_counter_x_in0;
	int m_counter_y_in0;
	int m_counter_x_in0b;
	int m_counter_y_in0b;
	int m_counter_x_in2;
	int m_counter_y_in2;
	int m_counter_x_in2b;
	int m_counter_y_in2b;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(atarifb_out1_w);
	DECLARE_WRITE8_MEMBER(atarifb4_out1_w);
	DECLARE_WRITE8_MEMBER(abaseb_out1_w);
	DECLARE_WRITE8_MEMBER(soccer_out1_w);
	DECLARE_WRITE8_MEMBER(atarifb_out2_w);
	DECLARE_WRITE8_MEMBER(soccer_out2_w);
	DECLARE_WRITE8_MEMBER(atarifb_out3_w);
	DECLARE_READ8_MEMBER(atarifb_in0_r);
	DECLARE_READ8_MEMBER(atarifb_in2_r);
	DECLARE_READ8_MEMBER(atarifb4_in0_r);
	DECLARE_READ8_MEMBER(atarifb4_in2_r);
	DECLARE_WRITE8_MEMBER(atarifb_alpha1_videoram_w);
	DECLARE_WRITE8_MEMBER(atarifb_alpha2_videoram_w);
	DECLARE_WRITE8_MEMBER(atarifb_field_videoram_w);
	TILE_GET_INFO_MEMBER(alpha1_get_tile_info);
	TILE_GET_INFO_MEMBER(alpha2_get_tile_info);
	TILE_GET_INFO_MEMBER(field_get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(atarifb);
	UINT32 screen_update_atarifb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_abaseb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_soccer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void get_tile_info_common( tile_data &tileinfo, tilemap_memory_index tile_index, UINT8 *alpha_videoram );
	void draw_playfield_and_alpha( bitmap_ind16 &bitmap, const rectangle &cliprect, int playfield_x_offset, int playfield_y_offset );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int gfx, int is_soccer );
};

/*----------- defined in audio/atarifb.c -----------*/
DISCRETE_SOUND_EXTERN( atarifb );
DISCRETE_SOUND_EXTERN( abaseb );

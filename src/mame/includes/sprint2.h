// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    Atari Sprint hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define SPRINT2_SKIDSND1_EN        NODE_01
#define SPRINT2_SKIDSND2_EN        NODE_02
#define SPRINT2_MOTORSND1_DATA     NODE_03
#define SPRINT2_MOTORSND2_DATA     NODE_04
#define SPRINT2_CRASHSND_DATA      NODE_05
#define SPRINT2_ATTRACT_EN         NODE_06
#define SPRINT2_NOISE_RESET        NODE_07

#define DOMINOS_FREQ_DATA          SPRINT2_MOTORSND1_DATA
#define DOMINOS_AMP_DATA           SPRINT2_CRASHSND_DATA
#define DOMINOS_TUMBLE_EN          SPRINT2_SKIDSND1_EN
#define DOMINOS_ATTRACT_EN         SPRINT2_ATTRACT_EN


class sprint2_state : public driver_device
{
public:
	sprint2_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_video_ram(*this, "video_ram"),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	int m_attract;
	int m_steering[2];
	int m_gear[2];
	int m_game;
	UINT8 m_dial[2];
	required_shared_ptr<UINT8> m_video_ram;
	tilemap_t* m_bg_tilemap;
	bitmap_ind16 m_helper;
	int m_collision[2];
	DECLARE_READ8_MEMBER(sprint2_wram_r);
	DECLARE_READ8_MEMBER(sprint2_dip_r);
	DECLARE_READ8_MEMBER(sprint2_input_A_r);
	DECLARE_READ8_MEMBER(sprint2_input_B_r);
	DECLARE_READ8_MEMBER(sprint2_sync_r);
	DECLARE_READ8_MEMBER(sprint2_steering1_r);
	DECLARE_READ8_MEMBER(sprint2_steering2_r);
	DECLARE_WRITE8_MEMBER(sprint2_steering_reset1_w);
	DECLARE_WRITE8_MEMBER(sprint2_steering_reset2_w);
	DECLARE_WRITE8_MEMBER(sprint2_wram_w);
	DECLARE_WRITE8_MEMBER(sprint2_lamp1_w);
	DECLARE_WRITE8_MEMBER(sprint2_lamp2_w);
	DECLARE_WRITE8_MEMBER(dominos4_lamp3_w);
	DECLARE_WRITE8_MEMBER(dominos4_lamp4_w);
	DECLARE_READ8_MEMBER(sprint2_collision1_r);
	DECLARE_READ8_MEMBER(sprint2_collision2_r);
	DECLARE_WRITE8_MEMBER(sprint2_collision_reset1_w);
	DECLARE_WRITE8_MEMBER(sprint2_collision_reset2_w);
	DECLARE_WRITE8_MEMBER(sprint2_video_ram_w);
	DECLARE_WRITE8_MEMBER(sprint2_attract_w);
	DECLARE_WRITE8_MEMBER(sprint2_noise_reset_w);
	DECLARE_WRITE8_MEMBER(sprint2_skid1_w);
	DECLARE_WRITE8_MEMBER(sprint2_skid2_w);
	DECLARE_DRIVER_INIT(sprint1);
	DECLARE_DRIVER_INIT(sprint2);
	DECLARE_DRIVER_INIT(dominos);
	DECLARE_DRIVER_INIT(dominos4);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(sprint2);
	UINT32 screen_update_sprint2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_sprint2(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(sprint2);
	UINT8 collision_check(rectangle& rect);
	inline int get_sprite_code(UINT8 *video_ram, int n);
	inline int get_sprite_x(UINT8 *video_ram, int n);
	inline int get_sprite_y(UINT8 *video_ram, int n);
	int service_mode();
	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};

/*----------- defined in audio/sprint2.c -----------*/
DISCRETE_SOUND_EXTERN( sprint2 );
DISCRETE_SOUND_EXTERN( sprint1 );
DISCRETE_SOUND_EXTERN( dominos );

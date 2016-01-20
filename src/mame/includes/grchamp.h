// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Phil Stroffolino
/*************************************************************************

    Taito Grand Champ hardware

*************************************************************************/

#include "sound/discrete.h"

class grchamp_state : public driver_device
{
public:
	grchamp_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_radarram(*this, "radarram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_leftram(*this, "leftram"),
		m_rightram(*this, "rightram"),
		m_centerram(*this, "centerram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	required_shared_ptr<UINT8> m_radarram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_leftram;
	required_shared_ptr<UINT8> m_rightram;
	required_shared_ptr<UINT8> m_centerram;

	UINT8       m_cpu0_out[16];
	UINT8       m_cpu1_out[16];

	UINT8       m_comm_latch;
	UINT8       m_comm_latch2[4];

	UINT16      m_ledlatch;
	UINT8       m_ledaddr;
	UINT16      m_ledram[8];

	UINT16      m_collide;
	UINT8       m_collmode;

	bitmap_ind16 m_work_bitmap;
	tilemap_t * m_text_tilemap;
	tilemap_t * m_left_tilemap;
	tilemap_t * m_center_tilemap;
	tilemap_t * m_right_tilemap;

	DECLARE_WRITE8_MEMBER(cpu0_outputs_w);
	DECLARE_WRITE8_MEMBER(led_board_w);
	DECLARE_WRITE8_MEMBER(cpu1_outputs_w);
	DECLARE_READ8_MEMBER(pc3259_0_r);
	DECLARE_READ8_MEMBER(pc3259_1_r);
	DECLARE_READ8_MEMBER(pc3259_2_r);
	DECLARE_READ8_MEMBER(pc3259_3_r);
	DECLARE_READ8_MEMBER(sub_to_main_comm_r);
	DECLARE_WRITE8_MEMBER(main_to_sub_comm_w);
	DECLARE_READ8_MEMBER(main_to_sub_comm_r);
	UINT8 get_pc3259_bits(int offs);
	DECLARE_WRITE8_MEMBER(left_w);
	DECLARE_WRITE8_MEMBER(center_w);
	DECLARE_WRITE8_MEMBER(right_w);
	DECLARE_WRITE8_MEMBER(portA_0_w);
	DECLARE_WRITE8_MEMBER(portB_0_w);
	DECLARE_WRITE8_MEMBER(portA_2_w);
	DECLARE_WRITE8_MEMBER(portB_2_w);

	TILE_GET_INFO_MEMBER(get_text_tile_info);
	TILE_GET_INFO_MEMBER(get_left_tile_info);
	TILE_GET_INFO_MEMBER(get_right_tile_info);
	TILE_GET_INFO_MEMBER(get_center_tile_info);
	TILEMAP_MAPPER_MEMBER(get_memory_offset);

	DECLARE_PALETTE_INIT(grchamp);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	INTERRUPT_GEN_MEMBER(cpu0_interrupt);
	INTERRUPT_GEN_MEMBER(cpu1_interrupt);
	TIMER_CALLBACK_MEMBER(main_to_sub_comm_sync_w);

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_objects(int y, UINT8 *objdata);
};

/* Discrete Sound Input Nodes */
#define GRCHAMP_ENGINE_CS_EN                NODE_01
#define GRCHAMP_SIFT_DATA                   NODE_02
#define GRCHAMP_ATTACK_UP_DATA              NODE_03
#define GRCHAMP_IDLING_EN                   NODE_04
#define GRCHAMP_FOG_EN                      NODE_05
#define GRCHAMP_PLAYER_SPEED_DATA           NODE_06
#define GRCHAMP_ATTACK_SPEED_DATA           NODE_07
#define GRCHAMP_A_DATA                      NODE_08
#define GRCHAMP_B_DATA                      NODE_09

/*----------- defined in audio/grchamp.c -----------*/

DISCRETE_SOUND_EXTERN( grchamp );

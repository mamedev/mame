/***************************************************************************

    Mad Alien (c) 1980 Data East Corporation

    Original driver by Norbert Kehrer (February 2004)

***************************************************************************/

#include "sound/discrete.h"


#define MADALIEN_MAIN_CLOCK		XTAL_10_595MHz


class madalien_state : public driver_device
{
public:
	madalien_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_charram(*this, "charram"),
		m_video_control(*this, "video_control"),
		m_shift_hi(*this, "shift_hi"),
		m_shift_lo(*this, "shift_lo"),
		m_video_flags(*this, "video_flags"),
		m_headlight_pos(*this, "headlight_pos"),
		m_edge1_pos(*this, "edge1_pos"),
		m_edge2_pos(*this, "edge2_pos"),
		m_scroll(*this, "scroll"){ }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_charram;
	required_shared_ptr<UINT8> m_video_control;
	required_shared_ptr<UINT8> m_shift_hi;
	required_shared_ptr<UINT8> m_shift_lo;
	required_shared_ptr<UINT8> m_video_flags;
	required_shared_ptr<UINT8> m_headlight_pos;
	required_shared_ptr<UINT8> m_edge1_pos;
	required_shared_ptr<UINT8> m_edge2_pos;
	required_shared_ptr<UINT8> m_scroll;

	tilemap_t *m_tilemap_fg;
	tilemap_t *m_tilemap_edge1[4];
	tilemap_t *m_tilemap_edge2[4];
	bitmap_ind16 *m_headlight_bitmap;
	DECLARE_READ8_MEMBER(shift_r);
	DECLARE_READ8_MEMBER(shift_rev_r);
	DECLARE_WRITE8_MEMBER(madalien_output_w);
	DECLARE_WRITE8_MEMBER(madalien_sound_command_w);
	DECLARE_READ8_MEMBER(madalien_sound_command_r);
	DECLARE_WRITE8_MEMBER(madalien_videoram_w);
	DECLARE_WRITE8_MEMBER(madalien_charram_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_WRITE8_MEMBER(madalien_portA_w);
	DECLARE_WRITE8_MEMBER(madalien_portB_w);
	TILEMAP_MAPPER_MEMBER(scan_mode0);
	TILEMAP_MAPPER_MEMBER(scan_mode1);
	TILEMAP_MAPPER_MEMBER(scan_mode2);
	TILEMAP_MAPPER_MEMBER(scan_mode3);
	TILE_GET_INFO_MEMBER(get_tile_info_BG_1);
	TILE_GET_INFO_MEMBER(get_tile_info_BG_2);
	TILE_GET_INFO_MEMBER(get_tile_info_FG);
	DECLARE_VIDEO_START(madalien);
	DECLARE_PALETTE_INIT(madalien);
};


/*----------- defined in video/madalien.c -----------*/

MACHINE_CONFIG_EXTERN( madalien_video );



/*----------- defined in audio/madalien.c -----------*/

DISCRETE_SOUND_EXTERN( madalien );

/* Discrete Sound Input Nodes */
#define MADALIEN_8910_PORTA			NODE_01
#define MADALIEN_8910_PORTB			NODE_02

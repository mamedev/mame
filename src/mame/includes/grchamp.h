/*************************************************************************

    Taito Grand Champ hardware

*************************************************************************/

#include "sound/discrete.h"

class grchamp_state : public driver_device
{
public:
	grchamp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8		m_cpu0_out[16];
	UINT8		m_cpu1_out[16];

	UINT8		m_comm_latch;
	UINT8		m_comm_latch2[4];

	UINT16		m_ledlatch;
	UINT8		m_ledaddr;
	UINT16		m_ledram[8];

	UINT16		m_collide;
	UINT8		m_collmode;

	UINT8 *		m_radarram;
	UINT8 *		m_videoram;
	UINT8 *		m_leftram;
	UINT8 *		m_centerram;
	UINT8 *		m_rightram;
	UINT8 *		m_spriteram;

	bitmap_t *	m_work_bitmap;
	tilemap_t *	m_text_tilemap;
	tilemap_t *	m_left_tilemap;
	tilemap_t *	m_center_tilemap;
	tilemap_t *	m_right_tilemap;

	rgb_t		m_bgcolor[0x20];
};

/* Discrete Sound Input Nodes */
#define GRCHAMP_ENGINE_CS_EN				NODE_01
#define GRCHAMP_SIFT_DATA					NODE_02
#define GRCHAMP_ATTACK_UP_DATA				NODE_03
#define GRCHAMP_IDLING_EN					NODE_04
#define GRCHAMP_FOG_EN						NODE_05
#define GRCHAMP_PLAYER_SPEED_DATA			NODE_06
#define GRCHAMP_ATTACK_SPEED_DATA			NODE_07
#define GRCHAMP_A_DATA						NODE_08
#define GRCHAMP_B_DATA						NODE_09

/*----------- defined in audio/grchamp.c -----------*/

DISCRETE_SOUND_EXTERN( grchamp );

/*----------- defined in video/grchamp.c -----------*/

PALETTE_INIT( grchamp );
VIDEO_START( grchamp );
SCREEN_UPDATE( grchamp );
WRITE8_HANDLER( grchamp_left_w );
WRITE8_HANDLER( grchamp_center_w );
WRITE8_HANDLER( grchamp_right_w );

#include "devlegcy.h"

class flower_state : public driver_device
{
public:
	flower_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_sn_nmi_enable;
	UINT8 *m_spriteram;
	UINT8 *m_textram;
	UINT8 *m_bg0ram;
	UINT8 *m_bg1ram;
	UINT8 *m_bg0_scroll;
	UINT8 *m_bg1_scroll;
	tilemap_t *m_bg0_tilemap;
	tilemap_t *m_bg1_tilemap;
	tilemap_t *m_text_tilemap;
	tilemap_t *m_text_right_tilemap;
	DECLARE_WRITE8_MEMBER(flower_maincpu_irq_ack);
	DECLARE_WRITE8_MEMBER(flower_subcpu_irq_ack);
	DECLARE_WRITE8_MEMBER(flower_soundcpu_irq_ack);
	DECLARE_WRITE8_MEMBER(flower_coin_counter_w);
	DECLARE_WRITE8_MEMBER(flower_coin_lockout_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(flower_textram_w);
	DECLARE_WRITE8_MEMBER(flower_bg0ram_w);
	DECLARE_WRITE8_MEMBER(flower_bg1ram_w);
	DECLARE_WRITE8_MEMBER(flower_flipscreen_w);
};


/*----------- defined in audio/flower.c -----------*/

WRITE8_DEVICE_HANDLER( flower_sound1_w );
WRITE8_DEVICE_HANDLER( flower_sound2_w );

DECLARE_LEGACY_SOUND_DEVICE(FLOWER, flower_sound);


/*----------- defined in video/flower.c -----------*/


SCREEN_UPDATE_IND16( flower );
VIDEO_START( flower );
PALETTE_INIT( flower );

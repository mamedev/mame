/*************************************************************************

    Pole Position hardware

*************************************************************************/

#include "devlegcy.h"
#include "sound/discrete.h"


class polepos_state : public driver_device
{
public:
	polepos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_steer_last;
	UINT8 m_steer_delta;
	INT16 m_steer_accum;
	INT16 m_last_result;
	INT8 m_last_signed;
	UINT8 m_last_unsigned;
	int m_adc_input;
	int m_auto_start_mask;
	UINT16 *m_view16_memory;
	UINT16 *m_road16_memory;
	UINT16 *m_alpha16_memory;
	UINT16 *m_sprite16_memory;
	UINT16 m_vertical_position_modifier[256];
	UINT16 m_road16_vscroll;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	int m_chacl;
	UINT16 m_scroll;
	UINT8 m_main_irq_mask;
	UINT8 m_sub_irq_mask;
};


/*----------- defined in audio/polepos.c -----------*/

DECLARE_LEGACY_SOUND_DEVICE(POLEPOS, polepos_sound);

WRITE8_DEVICE_HANDLER( polepos_engine_sound_lsb_w );
WRITE8_DEVICE_HANDLER( polepos_engine_sound_msb_w );

DISCRETE_SOUND_EXTERN( polepos );


/*----------- defined in video/polepos.c -----------*/

VIDEO_START( polepos );
PALETTE_INIT( polepos );
SCREEN_UPDATE( polepos );

WRITE16_HANDLER( polepos_view16_w );
WRITE16_HANDLER( polepos_road16_w );
WRITE16_HANDLER( polepos_alpha16_w );
WRITE16_HANDLER( polepos_sprite16_w );
WRITE8_HANDLER( polepos_view_w );
WRITE8_HANDLER( polepos_road_w );
WRITE8_HANDLER( polepos_alpha_w );
WRITE8_HANDLER( polepos_sprite_w );
WRITE8_HANDLER( polepos_chacl_w );

READ16_HANDLER( polepos_view16_r );
READ16_HANDLER( polepos_road16_r );
READ16_HANDLER( polepos_alpha16_r );
READ16_HANDLER( polepos_sprite16_r );
READ8_HANDLER( polepos_view_r );
READ8_HANDLER( polepos_road_r );
READ8_HANDLER( polepos_alpha_r );
READ8_HANDLER( polepos_sprite_r );
WRITE16_HANDLER( polepos_view16_hscroll_w );
WRITE16_HANDLER( polepos_road16_vscroll_w );

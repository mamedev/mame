/*************************************************************************

    Pole Position hardware

*************************************************************************/

#include "devlegcy.h"
#include "sound/discrete.h"


class polepos_state : public driver_device
{
public:
	polepos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_sprite16_memory(*this, "sprite16_memory"),
		m_road16_memory(*this, "road16_memory"),
		m_alpha16_memory(*this, "alpha16_memory"),
		m_view16_memory(*this, "view16_memory"){ }

	UINT8 m_steer_last;
	UINT8 m_steer_delta;
	INT16 m_steer_accum;
	INT16 m_last_result;
	INT8 m_last_signed;
	UINT8 m_last_unsigned;
	int m_adc_input;
	int m_auto_start_mask;
	required_shared_ptr<UINT16> m_sprite16_memory;
	required_shared_ptr<UINT16> m_road16_memory;
	required_shared_ptr<UINT16> m_alpha16_memory;
	required_shared_ptr<UINT16> m_view16_memory;
	UINT16 m_vertical_position_modifier[256];
	UINT16 m_road16_vscroll;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	int m_chacl;
	UINT16 m_scroll;
	UINT8 m_main_irq_mask;
	UINT8 m_sub_irq_mask;
	DECLARE_READ16_MEMBER(polepos2_ic25_r);
	DECLARE_READ8_MEMBER(polepos_adc_r);
	DECLARE_READ8_MEMBER(polepos_ready_r);
	DECLARE_WRITE8_MEMBER(polepos_latch_w);
	DECLARE_WRITE16_MEMBER(polepos_z8002_nvi_enable_w);
	DECLARE_READ16_MEMBER(polepos_sprite16_r);
	DECLARE_WRITE16_MEMBER(polepos_sprite16_w);
	DECLARE_READ8_MEMBER(polepos_sprite_r);
	DECLARE_WRITE8_MEMBER(polepos_sprite_w);
	DECLARE_READ16_MEMBER(polepos_road16_r);
	DECLARE_WRITE16_MEMBER(polepos_road16_w);
	DECLARE_READ8_MEMBER(polepos_road_r);
	DECLARE_WRITE8_MEMBER(polepos_road_w);
	DECLARE_WRITE16_MEMBER(polepos_road16_vscroll_w);
	DECLARE_READ16_MEMBER(polepos_view16_r);
	DECLARE_WRITE16_MEMBER(polepos_view16_w);
	DECLARE_READ8_MEMBER(polepos_view_r);
	DECLARE_WRITE8_MEMBER(polepos_view_w);
	DECLARE_WRITE16_MEMBER(polepos_view16_hscroll_w);
	DECLARE_WRITE8_MEMBER(polepos_chacl_w);
	DECLARE_READ16_MEMBER(polepos_alpha16_r);
	DECLARE_WRITE16_MEMBER(polepos_alpha16_w);
	DECLARE_READ8_MEMBER(polepos_alpha_r);
	DECLARE_WRITE8_MEMBER(polepos_alpha_w);
	DECLARE_CUSTOM_INPUT_MEMBER(high_port_r);
	DECLARE_CUSTOM_INPUT_MEMBER(low_port_r);
	DECLARE_CUSTOM_INPUT_MEMBER(auto_start_r);
};


/*----------- defined in audio/polepos.c -----------*/

DECLARE_LEGACY_SOUND_DEVICE(POLEPOS, polepos_sound);

WRITE8_DEVICE_HANDLER( polepos_engine_sound_lsb_w );
WRITE8_DEVICE_HANDLER( polepos_engine_sound_msb_w );

DISCRETE_SOUND_EXTERN( polepos );


/*----------- defined in video/polepos.c -----------*/

VIDEO_START( polepos );
PALETTE_INIT( polepos );
SCREEN_UPDATE_IND16( polepos );



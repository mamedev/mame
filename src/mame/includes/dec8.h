#include "video/bufsprite.h"

class dec8_state : public driver_device
{
public:
	dec8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_bg_data;
	UINT8 *  m_pf1_data;
	UINT8 *  m_row;
//  UINT8 *  m_paletteram;    // currently this uses generic palette handling
//  UINT8 *  m_paletteram_2;  // currently this uses generic palette handling
	size_t   m_videoram_size;
	UINT16   m_buffered_spriteram16[0x800/2]; // for the mxc06 sprite chip emulation (oscar, cobra)

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_pf1_tilemap;
	tilemap_t  *m_fix_tilemap;
	//int      m_scroll1[4];
	int      m_scroll2[4];
	int      m_bg_control[0x20];
	int      m_pf1_control[0x20];
	int      m_game_uses_priority;

	/* misc */
	int      m_i8751_port0;
	int      m_i8751_port1;
	int      m_nmi_enable;
	int      m_i8751_return;
	int      m_i8751_value;
	int      m_coinage_id;
	int      m_coin1;
	int      m_coin2;
	int      m_need1;
	int      m_need2;
	int      m_cred1;
	int      m_cred2;
	int      m_credits;
	int      m_latch;
	int      m_snd;
	int      m_msm5205next;
	int      m_toggle;

	/* devices */
	device_t *m_maincpu;
	device_t *m_subcpu;
	device_t *m_audiocpu;
	device_t *m_mcu;
	required_device<buffered_spriteram8_device> m_spriteram;
	DECLARE_WRITE8_MEMBER(dec8_mxc06_karn_buffer_spriteram_w);
	DECLARE_READ8_MEMBER(i8751_h_r);
	DECLARE_READ8_MEMBER(i8751_l_r);
	DECLARE_WRITE8_MEMBER(i8751_reset_w);
	DECLARE_READ8_MEMBER(gondo_player_1_r);
	DECLARE_READ8_MEMBER(gondo_player_2_r);
	DECLARE_WRITE8_MEMBER(dec8_i8751_w);
	DECLARE_WRITE8_MEMBER(lastmisn_i8751_w);
	DECLARE_WRITE8_MEMBER(shackled_i8751_w);
	DECLARE_WRITE8_MEMBER(csilver_i8751_w);
	DECLARE_WRITE8_MEMBER(srdarwin_i8751_w);
	DECLARE_WRITE8_MEMBER(dec8_bank_w);
	DECLARE_WRITE8_MEMBER(ghostb_bank_w);
	DECLARE_WRITE8_MEMBER(csilver_control_w);
	DECLARE_WRITE8_MEMBER(dec8_sound_w);
	DECLARE_WRITE8_MEMBER(csilver_adpcm_data_w);
	DECLARE_WRITE8_MEMBER(csilver_sound_bank_w);
	DECLARE_WRITE8_MEMBER(oscar_int_w);
	DECLARE_WRITE8_MEMBER(shackled_int_w);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_READ8_MEMBER(dec8_mcu_from_main_r);
	DECLARE_WRITE8_MEMBER(dec8_mcu_to_main_w);
	DECLARE_WRITE8_MEMBER(dec8_bg_data_w);
	DECLARE_READ8_MEMBER(dec8_bg_data_r);
	DECLARE_WRITE8_MEMBER(dec8_videoram_w);
	DECLARE_WRITE8_MEMBER(srdarwin_videoram_w);
	DECLARE_WRITE8_MEMBER(dec8_scroll2_w);
	DECLARE_WRITE8_MEMBER(srdarwin_control_w);
	DECLARE_WRITE8_MEMBER(lastmisn_control_w);
	DECLARE_WRITE8_MEMBER(shackled_control_w);
	DECLARE_WRITE8_MEMBER(lastmisn_scrollx_w);
	DECLARE_WRITE8_MEMBER(lastmisn_scrolly_w);
	DECLARE_WRITE8_MEMBER(gondo_scroll_w);
};

/*----------- defined in video/dec8.c -----------*/


PALETTE_INIT( ghostb );
SCREEN_UPDATE_IND16( cobracom );
SCREEN_UPDATE_IND16( ghostb );
SCREEN_UPDATE_IND16( srdarwin );
SCREEN_UPDATE_IND16( gondo );
SCREEN_UPDATE_IND16( garyoret );
SCREEN_UPDATE_IND16( lastmisn );
SCREEN_UPDATE_IND16( shackled );
SCREEN_UPDATE_IND16( oscar );
VIDEO_START( cobracom );
VIDEO_START( oscar );
VIDEO_START( ghostb );
VIDEO_START( lastmisn );
VIDEO_START( shackled );
VIDEO_START( srdarwin );
VIDEO_START( gondo );
VIDEO_START( garyoret );

WRITE8_HANDLER( dec8_bac06_0_w );
WRITE8_HANDLER( dec8_bac06_1_w );
WRITE8_HANDLER( dec8_pf1_data_w );
READ8_HANDLER( dec8_pf1_data_r );

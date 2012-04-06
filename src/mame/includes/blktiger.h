/***************************************************************************

    Black Tiger

***************************************************************************/

#include "video/bufsprite.h"

class blktiger_state : public driver_device
{
public:
	blktiger_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	/* memory pointers */
	UINT8 * m_txvideoram;
//  UINT8 * m_paletteram; // currently this uses generic palette handling
//  UINT8 * m_paletteram2;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg_tilemap8x4;
	tilemap_t *m_bg_tilemap4x8;
	UINT32  m_scroll_bank;
	UINT8   m_scroll_x[2];
	UINT8   m_scroll_y[2];
	UINT8   *m_scroll_ram;
	UINT8   m_screen_layout;
	UINT8   m_chon;
	UINT8   m_objon;
	UINT8   m_bgon;

	/* mcu-related */
	UINT8   m_z80_latch;
	UINT8   m_i8751_latch;

	/* devices */
	device_t *m_mcu;
	device_t *m_audiocpu;
	required_device<buffered_spriteram8_device> m_spriteram;
	DECLARE_READ8_MEMBER(blktiger_from_mcu_r);
	DECLARE_WRITE8_MEMBER(blktiger_to_mcu_w);
	DECLARE_READ8_MEMBER(blktiger_from_main_r);
	DECLARE_WRITE8_MEMBER(blktiger_to_main_w);
	DECLARE_WRITE8_MEMBER(blktiger_bankswitch_w);
	DECLARE_WRITE8_MEMBER(blktiger_coinlockout_w);
	DECLARE_WRITE8_MEMBER(blktiger_txvideoram_w);
	DECLARE_READ8_MEMBER(blktiger_bgvideoram_r);
	DECLARE_WRITE8_MEMBER(blktiger_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(blktiger_bgvideoram_bank_w);
	DECLARE_WRITE8_MEMBER(blktiger_scrolly_w);
	DECLARE_WRITE8_MEMBER(blktiger_scrollx_w);
	DECLARE_WRITE8_MEMBER(blktiger_video_control_w);
	DECLARE_WRITE8_MEMBER(blktiger_video_enable_w);
	DECLARE_WRITE8_MEMBER(blktiger_screen_layout_w);
};


/*----------- defined in video/blktiger.c -----------*/



VIDEO_START( blktiger );
SCREEN_UPDATE_IND16( blktiger );

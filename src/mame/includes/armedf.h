#include "video/bufsprite.h"

class armedf_state : public driver_device
{
public:
	armedf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	/* memory pointers */
	UINT8  *  m_text_videoram;
	UINT16 *  m_bg_videoram;
	UINT16 *  m_fg_videoram;
	UINT16 *  m_spr_pal_clut;
	UINT16 m_legion_cmd[4];	// legiono only!
//  UINT16 *  m_paletteram;   // currently this uses generic palette handling

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	tilemap_t  *m_tx_tilemap;
	UINT16   m_scroll_msb;
	UINT16   m_vreg;
	UINT16   m_fg_scrollx;
	UINT16   m_fg_scrolly;
	UINT16   m_bg_scrollx;
	UINT16   m_bg_scrolly;
	int      m_scroll_type;
	int      m_sprite_offy;
	int      m_old_mcu_mode;
	int      m_waiting_msb;

	required_device<buffered_spriteram16_device> m_spriteram;
	DECLARE_WRITE16_MEMBER(terraf_io_w);
	DECLARE_WRITE16_MEMBER(terrafb_io_w);
	DECLARE_WRITE16_MEMBER(bootleg_io_w);
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_READ8_MEMBER(soundlatch_clear_r);
	DECLARE_WRITE16_MEMBER(irq_lv1_ack_w);
	DECLARE_WRITE16_MEMBER(irq_lv2_ack_w);
	DECLARE_WRITE8_MEMBER(legiono_fg_scroll_w);
	DECLARE_READ8_MEMBER(blitter_txram_r);
	DECLARE_WRITE8_MEMBER(blitter_txram_w);
	DECLARE_WRITE8_MEMBER(fg_scrollx_w);
	DECLARE_WRITE8_MEMBER(fg_scrolly_w);
	DECLARE_WRITE8_MEMBER(fg_scroll_msb_w);
	DECLARE_READ8_MEMBER(nb1414m4_text_videoram_r);
	DECLARE_WRITE8_MEMBER(nb1414m4_text_videoram_w);
	DECLARE_READ8_MEMBER(armedf_text_videoram_r);
	DECLARE_WRITE8_MEMBER(armedf_text_videoram_w);
	DECLARE_WRITE16_MEMBER(armedf_fg_videoram_w);
	DECLARE_WRITE16_MEMBER(armedf_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(terraf_fg_scrolly_w);
	DECLARE_WRITE16_MEMBER(terraf_fg_scrollx_w);
	DECLARE_WRITE16_MEMBER(terraf_fg_scroll_msb_arm_w);
	DECLARE_WRITE16_MEMBER(armedf_fg_scrollx_w);
	DECLARE_WRITE16_MEMBER(armedf_fg_scrolly_w);
	DECLARE_WRITE16_MEMBER(armedf_bg_scrollx_w);
	DECLARE_WRITE16_MEMBER(armedf_bg_scrolly_w);
};

class bigfghtr_state : public armedf_state
{
public:
	bigfghtr_state(const machine_config &mconfig, device_type type, const char *tag)
		: armedf_state(mconfig, type, tag) { }

	UINT16 *      m_sharedram;

	/* misc */
	int           m_read_latch;
	UINT8		  m_mcu_input_snippet;
	UINT8		  m_mcu_jsr_snippet;

	DECLARE_READ16_MEMBER(latch_r);
	DECLARE_WRITE16_MEMBER(sharedram_w);
	DECLARE_READ16_MEMBER(sharedram_r);
};

/*----------- defined in video/armedf.c -----------*/

SCREEN_UPDATE_IND16( armedf );
VIDEO_START( armedf );
VIDEO_START( terraf );


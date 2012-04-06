
#include "sound/okim6295.h"

class drgnmst_state : public driver_device
{
public:
	drgnmst_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_oki_1(*this, "oki1"),
		  m_oki_2(*this, "oki2") { }

	/* memory pointers */
	UINT16 *    m_vidregs;
	UINT16 *    m_fg_videoram;
	UINT16 *    m_bg_videoram;
	UINT16 *    m_md_videoram;
	UINT16 *    m_rowscrollram;
	UINT16 *    m_vidregs2;
	UINT16 *    m_spriteram;
//  UINT16 *    m_paletteram;     // currently this uses generic palette handling
	size_t      m_spriteram_size;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	tilemap_t     *m_md_tilemap;

	/* misc */
	UINT16      m_snd_command;
	UINT16      m_snd_flag;
	UINT8       m_oki_control;
	UINT8       m_oki_command;
	UINT8       m_pic16c5x_port0;
	UINT8       m_oki0_bank;
	UINT8       m_oki1_bank;

	/* devices */
	required_device<okim6295_device> m_oki_1;
	required_device<okim6295_device> m_oki_2;
	DECLARE_WRITE16_MEMBER(drgnmst_coin_w);
	DECLARE_WRITE16_MEMBER(drgnmst_snd_command_w);
	DECLARE_WRITE16_MEMBER(drgnmst_snd_flag_w);
	DECLARE_READ8_MEMBER(pic16c5x_port0_r);
	DECLARE_READ8_MEMBER(drgnmst_snd_command_r);
	DECLARE_READ8_MEMBER(drgnmst_snd_flag_r);
	DECLARE_WRITE8_MEMBER(drgnmst_pcm_banksel_w);
	DECLARE_WRITE8_MEMBER(drgnmst_oki_w);
	DECLARE_WRITE8_MEMBER(drgnmst_snd_control_w);
	DECLARE_READ8_MEMBER(PIC16C5X_T0_clk_r);
	DECLARE_WRITE16_MEMBER(drgnmst_fg_videoram_w);
	DECLARE_WRITE16_MEMBER(drgnmst_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(drgnmst_md_videoram_w);
};


/*----------- defined in video/drgnmst.c -----------*/


VIDEO_START(drgnmst);
SCREEN_UPDATE_IND16(drgnmst);

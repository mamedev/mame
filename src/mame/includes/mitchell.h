/*************************************************************************

    Mitchell hardware

*************************************************************************/

#include "sound/okim6295.h"

class mitchell_state : public driver_device
{
public:
	mitchell_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_audiocpu(*this, "audiocpu"),
		  m_oki(*this, "oki") { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;
	size_t     m_videoram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	UINT8      *m_objram;           /* Sprite RAM */
	int        m_flipscreen;
	int        m_video_bank;
	int        m_paletteram_bank;

	/* sound-related */
	int        m_sample_buffer;
	int        m_sample_select;

	/* misc */
	int        m_input_type;
	int        m_dial[2];
	int        m_dial_selected;
	int        m_dir[2];
	int        m_keymatrix;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	UINT8 *m_nvram;
	size_t m_nvram_size;
	UINT8 m_irq_source;
	DECLARE_READ8_MEMBER(pang_port5_r);
	DECLARE_WRITE8_MEMBER(pang_bankswitch_w);
	DECLARE_READ8_MEMBER(block_input_r);
	DECLARE_WRITE8_MEMBER(block_dial_control_w);
	DECLARE_READ8_MEMBER(mahjong_input_r);
	DECLARE_WRITE8_MEMBER(mahjong_input_select_w);
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(input_w);
	DECLARE_WRITE8_MEMBER(spangbl_msm5205_data_w);
	DECLARE_WRITE8_MEMBER(mstworld_sound_w);
	DECLARE_WRITE8_MEMBER(pang_video_bank_w);
	DECLARE_WRITE8_MEMBER(mstworld_video_bank_w);
	DECLARE_WRITE8_MEMBER(mgakuen_videoram_w);
	DECLARE_READ8_MEMBER(mgakuen_videoram_r);
	DECLARE_WRITE8_MEMBER(mgakuen_objram_w);
	DECLARE_READ8_MEMBER(mgakuen_objram_r);
	DECLARE_WRITE8_MEMBER(pang_videoram_w);
	DECLARE_READ8_MEMBER(pang_videoram_r);
	DECLARE_WRITE8_MEMBER(pang_colorram_w);
	DECLARE_READ8_MEMBER(pang_colorram_r);
	DECLARE_WRITE8_MEMBER(pang_gfxctrl_w);
	DECLARE_WRITE8_MEMBER(pangbl_gfxctrl_w);
	DECLARE_WRITE8_MEMBER(mstworld_gfxctrl_w);
	DECLARE_WRITE8_MEMBER(pang_paletteram_w);
	DECLARE_READ8_MEMBER(pang_paletteram_r);
	DECLARE_WRITE8_MEMBER(mgakuen_paletteram_w);
	DECLARE_READ8_MEMBER(mgakuen_paletteram_r);
};


/*----------- defined in video/mitchell.c -----------*/




VIDEO_START( pang );
SCREEN_UPDATE_IND16( pang );

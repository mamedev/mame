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
};


/*----------- defined in video/mitchell.c -----------*/

WRITE8_HANDLER( mgakuen_paletteram_w );
READ8_HANDLER( mgakuen_paletteram_r );
WRITE8_HANDLER( mgakuen_videoram_w );
READ8_HANDLER( mgakuen_videoram_r );
WRITE8_HANDLER( mgakuen_objram_w );
READ8_HANDLER( mgakuen_objram_r );

WRITE8_HANDLER( pang_video_bank_w );
WRITE8_HANDLER( pang_videoram_w );
READ8_HANDLER( pang_videoram_r );
WRITE8_HANDLER( pang_colorram_w );
READ8_HANDLER( pang_colorram_r );
WRITE8_HANDLER( pang_gfxctrl_w );
WRITE8_HANDLER( pangbl_gfxctrl_w );
WRITE8_HANDLER( pang_paletteram_w );
READ8_HANDLER( pang_paletteram_r );

WRITE8_HANDLER( mstworld_gfxctrl_w );
WRITE8_HANDLER( mstworld_video_bank_w );

VIDEO_START( pang );
SCREEN_UPDATE( pang );

/*************************************************************************

    Mitchell hardware

*************************************************************************/

#include "sound/okim6295.h"

class mitchell_state : public driver_device
{
public:
	mitchell_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  audiocpu(*this, "audiocpu"),
		  oki(*this, "oki") { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	size_t     videoram_size;

	/* video-related */
	tilemap_t    *bg_tilemap;
	UINT8      *objram;           /* Sprite RAM */
	int        flipscreen;
	int        video_bank;
	int        paletteram_bank;

	/* sound-related */
	int        sample_buffer;
	int        sample_select;

	/* misc */
	int        input_type;
	int        dial[2], dial_selected;
	int        dir[2];
	int        keymatrix;

	/* devices */
	optional_device<cpu_device> audiocpu;
	optional_device<okim6295_device> oki;
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
VIDEO_UPDATE( pang );

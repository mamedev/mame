/***************************************************************************

    ESD 16 Bit Games

***************************************************************************/

#include "machine/eeprom.h"

class esd16_state : public driver_device
{
public:
	esd16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *       m_vram_0;
	UINT16 *       m_vram_1;
	UINT16 *       m_scroll_0;
	UINT16 *       m_scroll_1;
	UINT16 *       m_spriteram;
	UINT16 *       m_head_layersize;
	UINT16 *       m_headpanic_platform_x;
	UINT16 *       m_headpanic_platform_y;
//  UINT16 *       m_paletteram;  // currently this uses generic palette handling
	size_t         m_spriteram_size;

	/* video-related */
	tilemap_t       *m_tilemap_0_16x16;
	tilemap_t       *m_tilemap_1_16x16;
	tilemap_t       *m_tilemap_0;
	tilemap_t       *m_tilemap_1;
	int           m_tilemap0_color;

	/* devices */
	device_t *m_audio_cpu;
	eeprom_device *m_eeprom;
};


/*----------- defined in video/esd16.c -----------*/

WRITE16_HANDLER( esd16_vram_0_w );
WRITE16_HANDLER( esd16_vram_1_w );
WRITE16_HANDLER( esd16_tilemap0_color_w );

VIDEO_START( esd16 );
SCREEN_UPDATE( esd16 );
SCREEN_UPDATE( hedpanic );
SCREEN_UPDATE( hedpanio );


class lemmings_state : public driver_device
{
public:
	lemmings_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *  m_pixel_0_data;
	UINT16 *  m_pixel_1_data;
	UINT16 *  m_vram_data;
	UINT16 *  m_control_data;
	UINT16 *  m_paletteram;
//  UINT16 *  m_spriteram;    // this currently uses generic buffered spriteram
//  UINT16 *  m_spriteram2;   // this currently uses generic buffered spriteram

	/* video-related */
	bitmap_t *m_bitmap0;
	tilemap_t *m_vram_tilemap;
	UINT16 *m_sprite_triple_buffer_0;
	UINT16 *m_sprite_triple_buffer_1;
	UINT8 *m_vram_buffer;

	/* devices */
	device_t *m_audiocpu;
};


/*----------- defined in video/lemmings.c -----------*/

WRITE16_HANDLER( lemmings_pixel_0_w );
WRITE16_HANDLER( lemmings_pixel_1_w );
WRITE16_HANDLER( lemmings_vram_w );

VIDEO_START( lemmings );
SCREEN_EOF( lemmings );
SCREEN_UPDATE( lemmings );

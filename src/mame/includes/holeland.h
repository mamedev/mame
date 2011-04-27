/*************************************************************************

    Hole Land

*************************************************************************/

class holeland_state : public driver_device
{
public:
	holeland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;
	UINT8 *    m_spriteram;
	size_t     m_videoram_size;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_palette_offset;
	int        m_po[2];
};


/*----------- defined in video/holeland.c -----------*/

VIDEO_START( holeland );
VIDEO_START( crzrally );
SCREEN_UPDATE( holeland );
SCREEN_UPDATE( crzrally );

WRITE8_HANDLER( holeland_videoram_w );
WRITE8_HANDLER( holeland_colorram_w );
WRITE8_HANDLER( holeland_flipscreen_w );
WRITE8_HANDLER( holeland_pal_offs_w );
WRITE8_HANDLER( holeland_scroll_w );

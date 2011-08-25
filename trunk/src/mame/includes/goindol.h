/*************************************************************************

    Goindol

*************************************************************************/

class goindol_state : public driver_device
{
public:
	goindol_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_bg_videoram;
	UINT8 *    m_fg_videoram;
	UINT8 *    m_fg_scrollx;
	UINT8 *    m_fg_scrolly;
	UINT8 *    m_ram;
	UINT8 *    m_spriteram;
	UINT8 *    m_spriteram2;
	size_t     m_fg_videoram_size;
	size_t     m_bg_videoram_size;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	UINT16      m_char_bank;

	/* misc */
	int         m_prot_toggle;
};



/*----------- defined in video/goindol.c -----------*/

WRITE8_HANDLER( goindol_fg_videoram_w );
WRITE8_HANDLER( goindol_bg_videoram_w );

VIDEO_START( goindol );
SCREEN_UPDATE( goindol );

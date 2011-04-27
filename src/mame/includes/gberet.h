/***************************************************************************

    Green Beret

***************************************************************************/

class gberet_state : public driver_device
{
public:
	gberet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *     m_videoram;
	UINT8 *     m_colorram;
	UINT8 *     m_spriteram;
	UINT8 *     m_spriteram2;
	UINT8 *     m_scrollram;
	size_t      m_spriteram_size;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	UINT8       m_spritebank;

	/* misc */
	UINT8       m_nmi_enable;
	UINT8       m_irq_enable;
};


/*----------- defined in video/gberet.c -----------*/

WRITE8_HANDLER( gberet_videoram_w );
WRITE8_HANDLER( gberet_colorram_w );
WRITE8_HANDLER( gberet_scroll_w );
WRITE8_HANDLER( gberetb_scroll_w );
WRITE8_HANDLER( gberet_sprite_bank_w );

PALETTE_INIT( gberet );
VIDEO_START( gberet );
SCREEN_UPDATE( gberet );
SCREEN_UPDATE( gberetb );

/***************************************************************************

    Track'n'Field

***************************************************************************/


class trackfld_state : public driver_device
{
public:
	trackfld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_colorram;
	UINT8 *  m_scroll;
	UINT8 *  m_scroll2;
	UINT8 *  m_spriteram;
	UINT8 *  m_spriteram2;
	size_t   m_spriteram_size;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int      m_bg_bank;
	int      m_sprite_bank1;
	int      m_sprite_bank2;
	int      m_old_gfx_bank;					// needed by atlantol
	int		 m_sprites_gfx_banked;

	UINT8    m_irq_mask;
	UINT8    m_yieartf_nmi_mask;
};


/*----------- defined in video/trackfld.c -----------*/

WRITE8_HANDLER( trackfld_videoram_w );
WRITE8_HANDLER( trackfld_colorram_w );
WRITE8_HANDLER( trackfld_flipscreen_w );
WRITE8_HANDLER( atlantol_gfxbank_w );

PALETTE_INIT( trackfld );
VIDEO_START( trackfld );
SCREEN_UPDATE( trackfld );
VIDEO_START( atlantol );


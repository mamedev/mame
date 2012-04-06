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
	DECLARE_WRITE8_MEMBER(coin_w);
	DECLARE_WRITE8_MEMBER(questions_bank_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(yieartf_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(trackfld_videoram_w);
	DECLARE_WRITE8_MEMBER(trackfld_colorram_w);
	DECLARE_WRITE8_MEMBER(trackfld_flipscreen_w);
	DECLARE_WRITE8_MEMBER(atlantol_gfxbank_w);
};


/*----------- defined in video/trackfld.c -----------*/


PALETTE_INIT( trackfld );
VIDEO_START( trackfld );
SCREEN_UPDATE_IND16( trackfld );
VIDEO_START( atlantol );


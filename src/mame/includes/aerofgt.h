
class aerofgt_state : public driver_device
{
public:
	aerofgt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *  m_bg1videoram;
	UINT16 *  m_bg2videoram;
	UINT16 *  m_rasterram;
	UINT16 *  m_bitmapram;
	UINT16 *  m_spriteram1;
	UINT16 *  m_spriteram2;
	UINT16 *  m_spriteram3;
	UINT16 *  m_tx_tilemap_ram;
//  UINT16 *  m_paletteram;   // currently this uses generic palette handling
	size_t    m_spriteram1_size;
	size_t    m_spriteram2_size;
	size_t    m_spriteram3_size;

	/* video-related */
	tilemap_t   *m_bg1_tilemap;
	tilemap_t   *m_bg2_tilemap;
	UINT8     m_gfxbank[8];
	UINT16    m_bank[4];
	UINT16    m_bg1scrollx;
	UINT16    m_bg1scrolly;
	UINT16    m_bg2scrollx;
	UINT16    m_bg2scrolly;
	UINT16    m_wbbc97_bitmap_enable;
	int       m_charpalettebank;
	int       m_spritepalettebank;
	int       m_sprite_gfx;
	int       m_spikes91_lookup;

	/* misc */
	int       m_pending_command;

	/* devices */
	device_t *m_audiocpu;
};


/*----------- defined in video/aerofgt.c -----------*/


WRITE16_HANDLER( aerofgt_bg1videoram_w );
WRITE16_HANDLER( aerofgt_bg2videoram_w );
WRITE16_HANDLER( pspikes_gfxbank_w );
WRITE16_HANDLER( pspikesb_gfxbank_w );
WRITE16_HANDLER( spikes91_lookup_w );
WRITE16_HANDLER( karatblz_gfxbank_w );
WRITE16_HANDLER( spinlbrk_gfxbank_w );
WRITE16_HANDLER( turbofrc_gfxbank_w );
WRITE16_HANDLER( aerofgt_gfxbank_w );
WRITE16_HANDLER( aerofgt_bg1scrollx_w );
WRITE16_HANDLER( aerofgt_bg1scrolly_w );
WRITE16_HANDLER( aerofgt_bg2scrollx_w );
WRITE16_HANDLER( aerofgt_bg2scrolly_w );
WRITE16_HANDLER( pspikes_palette_bank_w );
WRITE16_HANDLER( wbbc97_bitmap_enable_w );

VIDEO_START( pspikes );
VIDEO_START( karatblz );
VIDEO_START( spinlbrk );
VIDEO_START( turbofrc );
VIDEO_START( wbbc97 );
SCREEN_UPDATE( pspikes );
SCREEN_UPDATE( pspikesb );
SCREEN_UPDATE( spikes91 );
SCREEN_UPDATE( karatblz );
SCREEN_UPDATE( spinlbrk );
SCREEN_UPDATE( turbofrc );
SCREEN_UPDATE( aerofgt );
SCREEN_UPDATE( aerfboot );
SCREEN_UPDATE( aerfboo2 );
SCREEN_UPDATE( wbbc97 );

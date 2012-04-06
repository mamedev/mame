
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
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_WRITE16_MEMBER(turbofrc_sound_command_w);
	DECLARE_WRITE16_MEMBER(aerfboot_soundlatch_w);
	DECLARE_READ16_MEMBER(pending_command_r);
	DECLARE_WRITE8_MEMBER(pending_command_clear_w);
	DECLARE_WRITE8_MEMBER(aerofgt_sh_bankswitch_w);
	DECLARE_WRITE8_MEMBER(aerfboot_okim6295_banking_w);
	DECLARE_WRITE16_MEMBER(aerofgt_bg1videoram_w);
	DECLARE_WRITE16_MEMBER(aerofgt_bg2videoram_w);
	DECLARE_WRITE16_MEMBER(pspikes_gfxbank_w);
	DECLARE_WRITE16_MEMBER(pspikesb_gfxbank_w);
	DECLARE_WRITE16_MEMBER(spikes91_lookup_w);
	DECLARE_WRITE16_MEMBER(karatblz_gfxbank_w);
	DECLARE_WRITE16_MEMBER(spinlbrk_gfxbank_w);
	DECLARE_WRITE16_MEMBER(turbofrc_gfxbank_w);
	DECLARE_WRITE16_MEMBER(aerofgt_gfxbank_w);
	DECLARE_WRITE16_MEMBER(aerofgt_bg1scrollx_w);
	DECLARE_WRITE16_MEMBER(aerofgt_bg1scrolly_w);
	DECLARE_WRITE16_MEMBER(aerofgt_bg2scrollx_w);
	DECLARE_WRITE16_MEMBER(aerofgt_bg2scrolly_w);
	DECLARE_WRITE16_MEMBER(pspikes_palette_bank_w);
	DECLARE_WRITE16_MEMBER(wbbc97_bitmap_enable_w);
};


/*----------- defined in video/aerofgt.c -----------*/



VIDEO_START( pspikes );
VIDEO_START( karatblz );
VIDEO_START( spinlbrk );
VIDEO_START( turbofrc );
VIDEO_START( wbbc97 );
SCREEN_UPDATE_IND16( pspikes );
SCREEN_UPDATE_IND16( pspikesb );
SCREEN_UPDATE_IND16( spikes91 );
SCREEN_UPDATE_IND16( karatblz );
SCREEN_UPDATE_IND16( spinlbrk );
SCREEN_UPDATE_IND16( turbofrc );
SCREEN_UPDATE_IND16( aerofgt );
SCREEN_UPDATE_IND16( aerfboot );
SCREEN_UPDATE_IND16( aerfboo2 );
SCREEN_UPDATE_RGB32( wbbc97 );

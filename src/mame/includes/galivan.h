/***************************************************************************

    Galivan - Cosmo Police

***************************************************************************/

class galivan_state : public driver_device
{
public:
	galivan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *     m_videoram;
//  UINT8 *     m_colorram;
	UINT8 *     m_spriteram;
	size_t      m_videoram_size;
	size_t      m_spriteram_size;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_tx_tilemap;
	UINT16       m_scrollx;
	UINT16       m_scrolly;
	UINT8		m_galivan_scrollx[2],m_galivan_scrolly[2];
	UINT8       m_flipscreen;
	UINT8       m_write_layers;
	UINT8       m_layers;
	UINT8       m_ninjemak_dispdisable;

	UINT8       m_shift_scroll; //youmab
	UINT32		m_shift_val;
	DECLARE_WRITE8_MEMBER(galivan_sound_command_w);
	DECLARE_READ8_MEMBER(soundlatch_clear_r);
	DECLARE_READ8_MEMBER(IO_port_c0_r);
	DECLARE_WRITE8_MEMBER(blit_trigger_w);
	DECLARE_WRITE8_MEMBER(youmab_extra_bank_w);
	DECLARE_READ8_MEMBER(youmab_8a_r);
	DECLARE_WRITE8_MEMBER(youmab_81_w);
	DECLARE_WRITE8_MEMBER(youmab_84_w);
	DECLARE_WRITE8_MEMBER(youmab_86_w);
	DECLARE_WRITE8_MEMBER(galivan_videoram_w);
	DECLARE_WRITE8_MEMBER(galivan_gfxbank_w);
	DECLARE_WRITE8_MEMBER(ninjemak_gfxbank_w);
	DECLARE_WRITE8_MEMBER(galivan_scrollx_w);
	DECLARE_WRITE8_MEMBER(galivan_scrolly_w);
};



/*----------- defined in video/galivan.c -----------*/

WRITE8_HANDLER( ninjemak_scrollx_w );
WRITE8_HANDLER( ninjemak_scrolly_w );

PALETTE_INIT( galivan );

VIDEO_START( galivan );
VIDEO_START( ninjemak );
SCREEN_UPDATE_IND16( galivan );
SCREEN_UPDATE_IND16( ninjemak );

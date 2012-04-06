/*************************************************************************

    Deniam games

*************************************************************************/


class deniam_state : public driver_device
{
public:
	deniam_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *       m_videoram;
	UINT16 *       m_textram;
	UINT16 *       m_spriteram;
	UINT16 *       m_paletteram;
	size_t         m_spriteram_size;

	/* video-related */
	tilemap_t        *m_fg_tilemap;
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_tx_tilemap;
	int            m_display_enable;
	int            m_bg_scrollx_offs;
	int            m_bg_scrolly_offs;
	int            m_fg_scrollx_offs;
	int            m_fg_scrolly_offs;
	int            m_bg_scrollx_reg;
	int            m_bg_scrolly_reg;
	int            m_bg_page_reg;
	int            m_fg_scrollx_reg;
	int            m_fg_scrolly_reg;
	int            m_fg_page_reg;
	int            m_bg_page[4];
	int            m_fg_page[4];
	UINT16         m_coinctrl;

	/* devices */
	device_t *m_audio_cpu;	// system 16c does not have sound CPU
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_WRITE16_MEMBER(deniam_irq_ack_w);
	DECLARE_WRITE16_MEMBER(deniam_videoram_w);
	DECLARE_WRITE16_MEMBER(deniam_textram_w);
	DECLARE_WRITE16_MEMBER(deniam_palette_w);
	DECLARE_READ16_MEMBER(deniam_coinctrl_r);
	DECLARE_WRITE16_MEMBER(deniam_coinctrl_w);
};


/*----------- defined in video/deniam.c -----------*/


VIDEO_START( deniam );
SCREEN_UPDATE_IND16( deniam );

DRIVER_INIT( logicpro );
DRIVER_INIT( karianx );

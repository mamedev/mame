class sbasketb_state : public driver_device
{
public:
	sbasketb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_palettebank(*this, "palettebank"),
		m_spriteram_select(*this, "spriteramsel"),
		m_scroll(*this, "scroll"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_palettebank;
	required_shared_ptr<UINT8> m_spriteram_select;
	required_shared_ptr<UINT8> m_scroll;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	UINT8    m_irq_mask;
	DECLARE_WRITE8_MEMBER(sbasketb_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(sbasketb_coin_counter_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(sbasketb_videoram_w);
	DECLARE_WRITE8_MEMBER(sbasketb_colorram_w);
	DECLARE_WRITE8_MEMBER(sbasketb_flipscreen_w);
};

/*----------- defined in video/sbasketb.c -----------*/


PALETTE_INIT( sbasketb );
VIDEO_START( sbasketb );
SCREEN_UPDATE_IND16( sbasketb );

class dcon_state : public driver_device
{
public:
	dcon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_back_data(*this, "back_data"),
		m_fore_data(*this, "fore_data"),
		m_mid_data(*this, "mid_data"),
		m_textram(*this, "textram"),
		m_spriteram(*this, "spriteram"),
		m_scroll_ram(*this, "scroll_ram"){ }

	required_shared_ptr<UINT16> m_back_data;
	required_shared_ptr<UINT16> m_fore_data;
	required_shared_ptr<UINT16> m_mid_data;
	required_shared_ptr<UINT16> m_textram;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_scroll_ram;
	tilemap_t *m_background_layer;
	tilemap_t *m_foreground_layer;
	tilemap_t *m_midground_layer;
	tilemap_t *m_text_layer;
	UINT16 m_enable;
	int m_gfx_bank_select;
	int m_last_gfx_bank;

	DECLARE_READ16_MEMBER(dcon_control_r);
	DECLARE_WRITE16_MEMBER(dcon_control_w);
	DECLARE_WRITE16_MEMBER(dcon_gfxbank_w);
	DECLARE_WRITE16_MEMBER(dcon_background_w);
	DECLARE_WRITE16_MEMBER(dcon_foreground_w);
	DECLARE_WRITE16_MEMBER(dcon_midground_w);
	DECLARE_WRITE16_MEMBER(dcon_text_w);
};


/*----------- defined in video/dcon.c -----------*/


VIDEO_START( dcon );
SCREEN_UPDATE_IND16( dcon );
SCREEN_UPDATE_IND16( sdgndmps );

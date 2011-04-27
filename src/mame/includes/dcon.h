class dcon_state : public driver_device
{
public:
	dcon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_back_data;
	UINT16 *m_fore_data;
	UINT16 *m_mid_data;
	UINT16 *m_scroll_ram;
	UINT16 *m_textram;
	tilemap_t *m_background_layer;
	tilemap_t *m_foreground_layer;
	tilemap_t *m_midground_layer;
	tilemap_t *m_text_layer;
	UINT16 m_enable;
	int m_gfx_bank_select;
	int m_last_gfx_bank;
	UINT16 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/dcon.c -----------*/

WRITE16_HANDLER( dcon_gfxbank_w );
WRITE16_HANDLER( dcon_background_w );
WRITE16_HANDLER( dcon_foreground_w );
WRITE16_HANDLER( dcon_midground_w );
WRITE16_HANDLER( dcon_text_w );
WRITE16_HANDLER( dcon_control_w );
READ16_HANDLER( dcon_control_r );

VIDEO_START( dcon );
SCREEN_UPDATE( dcon );
SCREEN_UPDATE( sdgndmps );

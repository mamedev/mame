class tankbust_state : public driver_device
{
public:
	tankbust_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_latch;
	UINT32 m_timer1;
	int m_e0xx_data[8];
	UINT8 m_variable_data;
	UINT8 *m_txtram;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_txt_tilemap;
	UINT8 m_xscroll[2];
	UINT8 m_yscroll[2];
	UINT8 *m_spriteram;
	size_t m_spriteram_size;

	UINT8 m_irq_mask;
	DECLARE_WRITE8_MEMBER(tankbust_soundlatch_w);
	DECLARE_WRITE8_MEMBER(tankbust_e0xx_w);
	DECLARE_READ8_MEMBER(debug_output_area_r);
	DECLARE_READ8_MEMBER(read_from_unmapped_memory);
	DECLARE_READ8_MEMBER(some_changing_input);
};


/*----------- defined in video/tankbust.c -----------*/

VIDEO_START( tankbust );
SCREEN_UPDATE_IND16( tankbust );

WRITE8_HANDLER( tankbust_background_videoram_w );
READ8_HANDLER( tankbust_background_videoram_r );
WRITE8_HANDLER( tankbust_background_colorram_w );
READ8_HANDLER( tankbust_background_colorram_r );
WRITE8_HANDLER( tankbust_txtram_w );
READ8_HANDLER( tankbust_txtram_r );

WRITE8_HANDLER( tankbust_xscroll_w );
WRITE8_HANDLER( tankbust_yscroll_w );



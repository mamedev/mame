class thepit_state : public driver_device
{
public:
	thepit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_question_address;
	int m_question_rom;
	int m_remap_address[16];
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_attributesram;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	UINT8 m_graphics_bank;
	UINT8 m_flip_screen_x;
	UINT8 m_flip_screen_y;
	tilemap_t *m_solid_tilemap;
	tilemap_t *m_tilemap;
	UINT8 *m_dummy_tile;
	UINT8 m_nmi_mask;
};


/*----------- defined in video/thepit.c -----------*/

PALETTE_INIT( thepit );
PALETTE_INIT( suprmous );
VIDEO_START( thepit );
SCREEN_UPDATE( thepit );
WRITE8_HANDLER( thepit_videoram_w );
WRITE8_HANDLER( thepit_colorram_w );
WRITE8_HANDLER( thepit_flip_screen_x_w );
WRITE8_HANDLER( thepit_flip_screen_y_w );
READ8_HANDLER( thepit_input_port_0_r );
WRITE8_HANDLER( intrepid_graphics_bank_w );

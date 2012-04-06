class route16_state : public driver_device
{
public:
	route16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_sharedram;
	UINT8 m_ttmahjng_port_select;
	int m_speakres_vrx;
	UINT8 *m_videoram1;
	UINT8 *m_videoram2;
	size_t m_videoram_size;
	UINT8 m_flipscreen;
	UINT8 m_palette_1;
	UINT8 m_palette_2;
	DECLARE_READ8_MEMBER(sharedram_r);
	DECLARE_WRITE8_MEMBER(sharedram_w);
	DECLARE_WRITE8_MEMBER(route16_sharedram_w);
	DECLARE_WRITE8_MEMBER(ttmahjng_input_port_matrix_w);
	DECLARE_READ8_MEMBER(ttmahjng_input_port_matrix_r);
	DECLARE_READ8_MEMBER(speakres_in3_r);
	DECLARE_WRITE8_MEMBER(speakres_out2_w);
	DECLARE_READ8_MEMBER(routex_prot_read);
	DECLARE_WRITE8_MEMBER(route16_out0_w);
	DECLARE_WRITE8_MEMBER(route16_out1_w);
};


/*----------- defined in video/route16.c -----------*/

SCREEN_UPDATE_RGB32( route16 );
SCREEN_UPDATE_RGB32( stratvox );
SCREEN_UPDATE_RGB32( ttmahjng );

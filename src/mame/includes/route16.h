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
};


/*----------- defined in video/route16.c -----------*/

WRITE8_HANDLER( route16_out0_w );
WRITE8_HANDLER( route16_out1_w );
SCREEN_UPDATE( route16 );
SCREEN_UPDATE( stratvox );
SCREEN_UPDATE( ttmahjng );

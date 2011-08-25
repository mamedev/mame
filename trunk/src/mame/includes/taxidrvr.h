class taxidrvr_state : public driver_device
{
public:
	taxidrvr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_s1;
	int m_s2;
	int m_s3;
	int m_s4;
	int m_latchA;
	int m_latchB;
	UINT8 *m_vram0;
	UINT8 *m_vram1;
	UINT8 *m_vram2;
	UINT8 *m_vram3;
	UINT8 *m_vram4;
	UINT8 *m_vram5;
	UINT8 *m_vram6;
	UINT8 *m_vram7;
	UINT8 *m_scroll;
	int m_bghide;
	int m_spritectrl[9];
};


/*----------- defined in video/taxidrvr.c -----------*/

WRITE8_DEVICE_HANDLER( taxidrvr_spritectrl_w );

SCREEN_UPDATE( taxidrvr );

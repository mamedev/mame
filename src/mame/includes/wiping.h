class wiping_state : public driver_device
{
public:
	wiping_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_sharedram1;
	UINT8 *m_sharedram2;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	int m_flipscreen;
	UINT8 *m_soundregs;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/wiping.c -----------*/

WRITE8_HANDLER( wiping_flipscreen_w );
PALETTE_INIT( wiping );
SCREEN_UPDATE( wiping );


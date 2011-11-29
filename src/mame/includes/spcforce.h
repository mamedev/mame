class spcforce_state : public driver_device
{
public:
	spcforce_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_scrollram;
	UINT8 *m_videoram;
	UINT8 *m_colorram;

	int m_sn76496_latch;
	int m_sn76496_select;

	UINT8 m_irq_mask;
};


/*----------- defined in video/spcforce.c -----------*/

WRITE8_HANDLER( spcforce_flip_screen_w );
SCREEN_UPDATE( spcforce );

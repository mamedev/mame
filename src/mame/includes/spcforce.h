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
	DECLARE_WRITE8_MEMBER(spcforce_SN76496_latch_w);
	DECLARE_READ8_MEMBER(spcforce_SN76496_select_r);
	DECLARE_WRITE8_MEMBER(spcforce_SN76496_select_w);
	DECLARE_READ8_MEMBER(spcforce_t0_r);
	DECLARE_WRITE8_MEMBER(spcforce_soundtrigger_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(spcforce_flip_screen_w);
};


/*----------- defined in video/spcforce.c -----------*/

SCREEN_UPDATE_IND16( spcforce );

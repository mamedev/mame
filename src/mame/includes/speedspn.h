class speedspn_state : public driver_device
{
public:
	speedspn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_attram;
	tilemap_t *m_tilemap;
	UINT8 m_display_disable;
	int m_bank_vidram;
	UINT8* m_vidram;
	DECLARE_READ8_MEMBER(speedspn_irq_ack_r);
	DECLARE_WRITE8_MEMBER(speedspn_banked_rom_change);
	DECLARE_WRITE8_MEMBER(speedspn_sound_w);
};


/*----------- defined in video/speedspn.c -----------*/

VIDEO_START( speedspn );
SCREEN_UPDATE_IND16( speedspn );

WRITE8_HANDLER( speedspn_vidram_w );
WRITE8_HANDLER( speedspn_attram_w );
READ8_HANDLER( speedspn_vidram_r );
WRITE8_HANDLER( speedspn_banked_vidram_change );
WRITE8_HANDLER( speedspn_global_display_w );


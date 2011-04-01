class galpani2_state : public driver_device
{
public:
	galpani2_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *m_bg8_0;
	UINT16 *m_bg8_1;
	UINT16 *m_palette_0;
	UINT16 *m_palette_1;
	UINT16 *m_bg8_0_scrollx;
	UINT16 *m_bg8_1_scrollx;
	UINT16 *m_bg8_0_scrolly;
	UINT16 *m_bg8_1_scrolly;
	UINT16 *m_bg15;
	UINT16 m_eeprom_word;
	UINT16 *m_ram;
	UINT16 *m_ram2;
	UINT16 m_old_mcu_nmi1;
	UINT16 m_old_mcu_nmi2;
	UINT16 *m_rombank;
	bitmap_t *m_bg8_bitmap_0;
	bitmap_t *m_bg8_bitmap_1;
	bitmap_t *m_bg15_bitmap;
};


/*----------- defined in video/galpani2.c -----------*/

PALETTE_INIT( galpani2 );
VIDEO_START( galpani2 );
SCREEN_UPDATE( galpani2 );

WRITE16_HANDLER( galpani2_palette_0_w );
WRITE16_HANDLER( galpani2_palette_1_w );

WRITE16_HANDLER( galpani2_bg8_0_w );
WRITE16_HANDLER( galpani2_bg8_1_w );

WRITE16_HANDLER( galpani2_bg15_w );

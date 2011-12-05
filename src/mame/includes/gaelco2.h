class gaelco2_state : public driver_device
{
public:
	gaelco2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	UINT16 *m_snowboar_protection;
	UINT16 *m_vregs;
	int m_clr_gun_int;
	UINT8 m_analog_ports[2];
	UINT16 *m_videoram;
	tilemap_t *m_pant[2];
	int m_dual_monitor;

	required_device<cpu_device> m_maincpu;
};


/*----------- defined in machine/gaelco2.c -----------*/

DRIVER_INIT( alighunt );
DRIVER_INIT( touchgo );
DRIVER_INIT( snowboar );
DRIVER_INIT( bang );
WRITE16_HANDLER( gaelco2_coin_w );
WRITE16_HANDLER( gaelco2_coin2_w );
WRITE16_HANDLER( wrally2_coin_w );
WRITE16_HANDLER( touchgo_coin_w );
WRITE16_HANDLER( bang_clr_gun_int_w );
TIMER_DEVICE_CALLBACK( bang_irq );
CUSTOM_INPUT( wrally2_analog_bit_r );
WRITE16_HANDLER( wrally2_adc_clk );
WRITE16_HANDLER( wrally2_adc_cs );
WRITE16_DEVICE_HANDLER( gaelco2_eeprom_cs_w );
WRITE16_DEVICE_HANDLER( gaelco2_eeprom_sk_w );
WRITE16_DEVICE_HANDLER( gaelco2_eeprom_data_w );
READ16_HANDLER( snowboar_protection_r );
WRITE16_HANDLER( snowboar_protection_w );

/*----------- defined in video/gaelco2.c -----------*/

WRITE16_HANDLER( gaelco2_vram_w );
WRITE16_HANDLER( gaelco2_palette_w );
SCREEN_UPDATE( gaelco2 );
SCREEN_EOF( gaelco2 );
VIDEO_START( gaelco2 );
SCREEN_UPDATE( gaelco2_dual );
VIDEO_START( gaelco2_dual );

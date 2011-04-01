struct iox_t
{
	int reset,ff_event,ff_1,protcheck[4],protlatch[4];
	UINT8 data;
	UINT8 mux;
	UINT8 ff;
};

class srmp2_state : public driver_device
{
public:
	srmp2_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int m_color_bank;
	int m_gfx_bank;

	int m_adpcm_bank;
	int m_adpcm_data;
	UINT32 m_adpcm_sptr;
	UINT32 m_adpcm_eptr;

	int m_port_select;

	union
	{
		UINT8 *u8;
		UINT16 *u16;
	} m_spriteram1, m_spriteram2, m_spriteram3;

	iox_t m_iox;
};


/*----------- defined in video/srmp2.c -----------*/

PALETTE_INIT( srmp2 );
SCREEN_UPDATE( srmp2 );
PALETTE_INIT( srmp3 );
SCREEN_UPDATE( srmp3 );
SCREEN_UPDATE( mjyuugi );

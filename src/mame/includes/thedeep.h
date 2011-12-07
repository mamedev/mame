class thedeep_state : public driver_device
{
public:
	thedeep_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	int m_nmi_enable;
	UINT8 m_protection_command;
	UINT8 m_protection_data;
	int m_protection_index;
	int m_protection_irq;
	int m_rombank;
	UINT8 *m_vram_0;
	UINT8 *m_vram_1;
	UINT8 *m_scroll;
	UINT8 *m_scroll2;
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;

	required_device<cpu_device> m_maincpu;
};


/*----------- defined in video/thedeep.c -----------*/

WRITE8_HANDLER( thedeep_vram_0_w );
WRITE8_HANDLER( thedeep_vram_1_w );

PALETTE_INIT( thedeep );
VIDEO_START( thedeep );
SCREEN_UPDATE( thedeep );


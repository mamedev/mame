class gsword_state : public driver_device
{
public:
	gsword_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	int coins;
	int fake8910_0;
	int fake8910_1;
	int nmi_enable;
	UINT8 *cpu2_ram;
	int protect_hack;
	size_t spritexy_size;
	UINT8 *spritexy_ram;
	UINT8 *spritetile_ram;
	UINT8 *spriteattrib_ram;
	int charbank;
	int charpalbank;
	int flipscreen;
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/gsword.c -----------*/

WRITE8_HANDLER( gsword_charbank_w );
WRITE8_HANDLER( gsword_videoctrl_w );
WRITE8_HANDLER( gsword_videoram_w );
WRITE8_HANDLER( gsword_scroll_w );

PALETTE_INIT( josvolly );
PALETTE_INIT( gsword );
VIDEO_START( gsword );
SCREEN_UPDATE( gsword );

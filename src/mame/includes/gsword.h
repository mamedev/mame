class gsword_state : public driver_device
{
public:
	gsword_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in video/gsword.c -----------*/

extern size_t gsword_spritexy_size;

extern UINT8 *gsword_spritexy_ram;
extern UINT8 *gsword_spritetile_ram;
extern UINT8 *gsword_spriteattrib_ram;

WRITE8_HANDLER( gsword_charbank_w );
WRITE8_HANDLER( gsword_videoctrl_w );
WRITE8_HANDLER( gsword_videoram_w );
WRITE8_HANDLER( gsword_scroll_w );

PALETTE_INIT( josvolly );
PALETTE_INIT( gsword );
VIDEO_START( gsword );
VIDEO_UPDATE( gsword );

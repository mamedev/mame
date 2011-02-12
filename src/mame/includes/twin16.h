class twin16_state : public driver_device
{
public:
	twin16_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
	UINT16 CPUA_register;
	UINT16 CPUB_register;
	UINT16 sound_command;
	int cuebrickj_nvram_bank;
	UINT16 cuebrickj_nvram[0x400*0x20];
	UINT16 custom_video;
	UINT16 *gfx_rom;
	UINT16 *text_ram;
	UINT16 *sprite_gfx_ram;
	UINT16 *tile_gfx_ram;
	UINT16 sprite_buffer[0x800];
	emu_timer *sprite_timer;
	int sprite_busy;
	int need_process_spriteram;
	UINT16 gfx_bank;
	UINT16 scrollx[3];
	UINT16 scrolly[3];
	UINT16 video_register;
	tilemap_t *text_tilemap;
};


/*----------- defined in drivers/twin16.c -----------*/

int twin16_spriteram_process_enable( running_machine *machine );


/*----------- defined in video/twin16.c -----------*/

WRITE16_HANDLER( twin16_text_ram_w );
WRITE16_HANDLER( twin16_paletteram_word_w );
WRITE16_HANDLER( fround_gfx_bank_w );
WRITE16_HANDLER( twin16_video_register_w );
READ16_HANDLER( twin16_sprite_status_r );

VIDEO_START( twin16 );
VIDEO_UPDATE( twin16 );
VIDEO_EOF( twin16 );

void twin16_spriteram_process( running_machine *machine );

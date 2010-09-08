class twin16_state : public driver_device
{
public:
	twin16_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
};


/*----------- defined in drivers/twin16.c -----------*/

extern UINT16 twin16_custom_video;
extern UINT16 *twin16_gfx_rom;
extern UINT16 *twin16_text_ram;
extern UINT16 *twin16_sprite_gfx_ram;
extern UINT16 *twin16_tile_gfx_ram;
int twin16_spriteram_process_enable( void );


/*----------- defined in video/twin16.c -----------*/

WRITE16_HANDLER( twin16_text_ram_w );
WRITE16_HANDLER( twin16_paletteram_word_w );
WRITE16_HANDLER( fround_gfx_bank_w );
WRITE16_HANDLER( twin16_video_register_w );
READ16_HANDLER( twin16_sprite_status_r );

VIDEO_START( twin16 );
VIDEO_UPDATE( twin16 );
VIDEO_EOF( twin16 );

void twin16_spriteram_process( running_machine* );

class system1_state : public driver_device
{
public:
	system1_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	void (*videomode_custom)(running_machine *machine, UINT8 data, UINT8 prevdata);
	UINT8 mute_xor;
	UINT8 *ram;
	UINT8 dakkochn_mux_data;
	UINT8 videomode_prev;
	UINT8 mcu_control;
	UINT8 *nob_mcu_status;
	UINT8 *nob_mcu_latch;
	UINT8 nob_maincpu_latch;
	int nobb_inport23_step;
	UINT8 *mix_collide;
	UINT8 mix_collide_summary;
	UINT8 *sprite_collide;
	UINT8 sprite_collide_summary;
	bitmap_t *sprite_bitmap;
	UINT8 video_mode;
	UINT8 videoram_bank;
	tilemap_t *tilemap_page[8];
	UINT8 tilemap_pages;
};


/*----------- defined in video/system1.c -----------*/

VIDEO_START( system1 );
VIDEO_START( system2 );

WRITE8_HANDLER( system1_videomode_w );
WRITE8_HANDLER( system1_paletteram_w );

READ8_HANDLER( system1_videoram_r );
WRITE8_HANDLER( system1_videoram_w );
WRITE8_DEVICE_HANDLER( system1_videoram_bank_w );

READ8_HANDLER( system1_mixer_collision_r );
WRITE8_HANDLER( system1_mixer_collision_w );
WRITE8_HANDLER( system1_mixer_collision_reset_w );

READ8_HANDLER( system1_sprite_collision_r );
WRITE8_HANDLER( system1_sprite_collision_w );
WRITE8_HANDLER( system1_sprite_collision_reset_w );

SCREEN_UPDATE( system1 );
SCREEN_UPDATE( system2 );
SCREEN_UPDATE( system2_rowscroll );

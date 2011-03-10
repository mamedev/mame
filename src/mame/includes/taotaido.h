class taotaido_state : public driver_device
{
public:
	taotaido_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int pending_command;
	UINT16 *spriteram;
	UINT16 *spriteram2;
	UINT16 *scrollram;
	UINT16 *bgram;
	UINT16 sprite_character_bank_select[8];
	UINT16 video_bank_select[8];
	tilemap_t *bg_tilemap;
	UINT16 *spriteram_old;
	UINT16 *spriteram_older;
	UINT16 *spriteram2_old;
	UINT16 *spriteram2_older;
};


/*----------- defined in video/taotaido.c -----------*/

WRITE16_HANDLER( taotaido_sprite_character_bank_select_w );
WRITE16_HANDLER( taotaido_tileregs_w );
WRITE16_HANDLER( taotaido_bgvideoram_w );
VIDEO_START( taotaido );
SCREEN_UPDATE( taotaido );
SCREEN_EOF( taotaido );

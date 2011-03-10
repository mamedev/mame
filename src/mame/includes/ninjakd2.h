class ninjakd2_state : public driver_device
{
public:
	ninjakd2_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	const INT16* sampledata;
	UINT8 omegaf_io_protection[3];
	UINT8 omegaf_io_protection_input;
	int omegaf_io_protection_tic;
	UINT8* bg_videoram;
	UINT8* fg_videoram;
	int next_sprite_overdraw_enabled;
	int (*stencil_compare_function) (UINT16 pal);
	int sprites_updated;
	bitmap_t *sp_bitmap;
	int robokid_sprites;
	tilemap_t* fg_tilemap;
	tilemap_t* bg_tilemap;
	tilemap_t* bg0_tilemap;
	tilemap_t* bg1_tilemap;
	tilemap_t* bg2_tilemap;
	int bank_mask;
	int robokid_bg0_bank;
	int robokid_bg1_bank;
	int robokid_bg2_bank;
	UINT8* robokid_bg0_videoram;
	UINT8* robokid_bg1_videoram;
	UINT8* robokid_bg2_videoram;
};


/*----------- defined in video/ninjakd2.c -----------*/

extern WRITE8_HANDLER( ninjakd2_bgvideoram_w );
extern WRITE8_HANDLER( ninjakd2_fgvideoram_w );
extern WRITE8_HANDLER( ninjakd2_bg_ctrl_w );
extern WRITE8_HANDLER( ninjakd2_sprite_overdraw_w );

extern READ8_HANDLER( robokid_bg0_videoram_r );
extern READ8_HANDLER( robokid_bg1_videoram_r );
extern READ8_HANDLER( robokid_bg2_videoram_r );
extern WRITE8_HANDLER( robokid_bg0_videoram_w );
extern WRITE8_HANDLER( robokid_bg1_videoram_w );
extern WRITE8_HANDLER( robokid_bg2_videoram_w );
extern WRITE8_HANDLER( robokid_bg0_ctrl_w );
extern WRITE8_HANDLER( robokid_bg1_ctrl_w );
extern WRITE8_HANDLER( robokid_bg2_ctrl_w );
extern WRITE8_HANDLER( robokid_bg0_bank_w );
extern WRITE8_HANDLER( robokid_bg1_bank_w );
extern WRITE8_HANDLER( robokid_bg2_bank_w );

extern VIDEO_START( ninjakd2 );
extern VIDEO_START( mnight );
extern VIDEO_START( arkarea );
extern VIDEO_START( robokid );
extern VIDEO_START( omegaf );
extern SCREEN_UPDATE( ninjakd2 );
extern SCREEN_UPDATE( robokid );
extern SCREEN_UPDATE( omegaf );
extern SCREEN_EOF( ninjakd2 );


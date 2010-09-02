class sslam_state : public driver_device
{
public:
	sslam_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	emu_timer *music_timer;

	int sound;
	int melody;
	int bar;
	int track;
	int snd_bank;

	UINT16 *bg_tileram;
	UINT16 *tx_tileram;
	UINT16 *md_tileram;
	UINT16 *spriteram;
	UINT16 *regs;

	UINT8 oki_control;
	UINT8 oki_command;
	UINT8 oki_bank;

	tilemap_t *bg_tilemap;
	tilemap_t *tx_tilemap;
	tilemap_t *md_tilemap;

	int sprites_x_offset;
};


/*----------- defined in video/sslam.c -----------*/

WRITE16_HANDLER( sslam_tx_tileram_w );
WRITE16_HANDLER( sslam_md_tileram_w );
WRITE16_HANDLER( sslam_bg_tileram_w );
WRITE16_HANDLER( powerbls_bg_tileram_w );
VIDEO_START(sslam);
VIDEO_START(powerbls);
VIDEO_UPDATE(sslam);
VIDEO_UPDATE(powerbls);

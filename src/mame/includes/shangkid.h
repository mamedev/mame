class shangkid_state : public driver_device
{
public:
	shangkid_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in video/shangkid.c -----------*/

extern UINT8 *shangkid_videoreg;
extern int shangkid_gfx_type;

VIDEO_START( shangkid );
VIDEO_UPDATE( shangkid );
WRITE8_HANDLER( shangkid_videoram_w );

PALETTE_INIT( dynamski );
VIDEO_UPDATE( dynamski );


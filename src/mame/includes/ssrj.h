class ssrj_state : public driver_device
{
public:
	ssrj_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int oldport;
	tilemap_t *tilemap1,*tilemap2,*tilemap4;
	UINT8 *vram1;
	UINT8 *vram2;
	UINT8 *vram3;
	UINT8 *vram4;
	UINT8 *scrollram;
};


/*----------- defined in video/ssrj.c -----------*/

WRITE8_HANDLER(ssrj_vram1_w);
WRITE8_HANDLER(ssrj_vram2_w);
WRITE8_HANDLER(ssrj_vram4_w);

VIDEO_START( ssrj );
VIDEO_UPDATE( ssrj );
PALETTE_INIT( ssrj );

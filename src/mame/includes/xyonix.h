class xyonix_state : public driver_device
{
public:
	xyonix_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *vidram;
	tilemap_t *tilemap;

	int e0_data;
	int credits;
	int coins;
	int prev_coin;
};


/*----------- defined in video/xyonix.c -----------*/

PALETTE_INIT( xyonix );
WRITE8_HANDLER( xyonix_vidram_w );
VIDEO_START(xyonix);
VIDEO_UPDATE(xyonix);

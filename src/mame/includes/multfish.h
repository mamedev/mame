/*************************************************************************

    Igrosoft

*************************************************************************/

class multfish_state : public driver_device
{
public:
	multfish_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* Video related */

	UINT8* vid;

	int disp_enable;
	int xor_paltype;
	int xor_palette;

	tilemap_t *tilemap;
	tilemap_t *reel_tilemap;

	/* Misc related */

	UINT8 rambk;

	UINT8 hopper_motor;
	UINT8 hopper;

};

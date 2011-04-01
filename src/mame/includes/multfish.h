/*************************************************************************

    Igrosoft

*************************************************************************/

class multfish_state : public driver_device
{
public:
	multfish_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* Video related */

	UINT8* m_vid;

	int m_disp_enable;
	int m_xor_paltype;
	int m_xor_palette;

	tilemap_t *m_tilemap;
	tilemap_t *m_reel_tilemap;

	/* Misc related */

	UINT8 m_rambk;

	UINT8 m_hopper_motor;
	UINT8 m_hopper;

};

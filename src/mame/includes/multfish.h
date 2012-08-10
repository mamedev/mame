/*************************************************************************

    Igrosoft

*************************************************************************/

class multfish_state : public driver_device
{
public:
	multfish_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

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

	DECLARE_DRIVER_INIT(ROT0);
	DECLARE_DRIVER_INIT(customl);
};

/*************************************************************************

    Vapour Trail

*************************************************************************/

class vaportra_state : public driver_device
{
public:
	vaportra_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *  pf1_rowscroll;
	UINT16 *  pf2_rowscroll;
	UINT16 *  pf3_rowscroll;
	UINT16 *  pf4_rowscroll;

	/* misc */
	UINT16    priority[2];

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *deco16ic;
};



/*----------- defined in video/vaportra.c -----------*/

WRITE16_HANDLER( vaportra_priority_w );
WRITE16_HANDLER( vaportra_palette_24bit_rg_w );
WRITE16_HANDLER( vaportra_palette_24bit_b_w );

VIDEO_UPDATE( vaportra );

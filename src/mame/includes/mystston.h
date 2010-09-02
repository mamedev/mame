/***************************************************************************

    Technos Mysterious Stones hardware

    driver by Nicola Salmoria

***************************************************************************/


#define	MYSTSTON_MASTER_CLOCK	(XTAL_12MHz)


class mystston_state : public driver_device
{
public:
	mystston_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* machine state */
	UINT8 *ay8910_data;
	UINT8 *ay8910_select;

	/* video state */
	tilemap_t *fg_tilemap;
	tilemap_t *bg_tilemap;
	emu_timer *interrupt_timer;
	UINT8 *bg_videoram;
	UINT8 *fg_videoram;
	UINT8 *spriteram;
	UINT8 *paletteram;
	UINT8 *scroll;
	UINT8 *video_control;
};


/*----------- defined in drivers/mystston.c -----------*/

void mystston_on_scanline_interrupt(running_machine *machine);


/*----------- defined in video/mystston.c -----------*/

MACHINE_CONFIG_EXTERN( mystston_video );
WRITE8_HANDLER( mystston_video_control_w );

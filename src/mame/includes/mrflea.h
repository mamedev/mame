/*************************************************************************

    Mr. Flea

*************************************************************************/

class mrflea_state : public driver_device
{
public:
	mrflea_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    spriteram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int     gfx_bank;

	/* misc */
	int io;
	int main;
	int status;
	int select1;

	/* devices */
	device_t *maincpu;
	device_t *subcpu;
};


/*----------- defined in video/mrflea.c -----------*/

WRITE8_HANDLER( mrflea_gfx_bank_w );
WRITE8_HANDLER( mrflea_videoram_w );
WRITE8_HANDLER( mrflea_spriteram_w );

VIDEO_UPDATE( mrflea );

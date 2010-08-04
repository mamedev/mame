/*************************************************************************

    Mr. Flea

*************************************************************************/

class mrflea_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mrflea_state(machine)); }

	mrflea_state(running_machine &machine)
		: driver_data_t(machine) { }

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
	running_device *maincpu;
	running_device *subcpu;
};


/*----------- defined in video/mrflea.c -----------*/

WRITE8_HANDLER( mrflea_gfx_bank_w );
WRITE8_HANDLER( mrflea_videoram_w );
WRITE8_HANDLER( mrflea_spriteram_w );

VIDEO_UPDATE( mrflea );

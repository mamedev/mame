/*************************************************************************

    Rainbow Islands

*************************************************************************/

class rainbow_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, rainbow_state(machine)); }

	rainbow_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT16 *    spriteram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling
	size_t      spriteram_size;

	/* video-related */
	UINT16      sprite_ctrl;
	UINT16      sprites_flipscreen;

	/* misc */
	UINT8       jumping_latch;

	/* c-chip */
	UINT8       *CRAM[8];
	int         extra_version;
	UINT8       current_bank;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *pc080sn;
	running_device *pc090oj;
};


/*----------- defined in machine/rainbow.c -----------*/

void rainbow_cchip_init(running_machine *machine, int version);
READ16_HANDLER( rainbow_cchip_ctrl_r );
READ16_HANDLER( rainbow_cchip_ram_r );
WRITE16_HANDLER( rainbow_cchip_ctrl_w );
WRITE16_HANDLER( rainbow_cchip_bank_w );
WRITE16_HANDLER( rainbow_cchip_ram_w );


/*----------- defined in video/rastan.c -----------*/

VIDEO_START( jumping );

VIDEO_UPDATE( rainbow );
VIDEO_UPDATE( jumping );

WRITE16_HANDLER( jumping_spritectrl_w );
WRITE16_HANDLER( rainbow_spritectrl_w );

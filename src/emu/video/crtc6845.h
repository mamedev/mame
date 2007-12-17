/**********************************************************************

    Motorola 6845 CRT controller emulation

**********************************************************************/


typedef struct _crtc6845_interface crtc6845_interface;
struct _crtc6845_interface
{
	int scrnum;					/* screen we are acting on */
	int clock;					/* the clock (pin 21) of the chip */
	int hpixels_per_column;		/* number of pixels per video memory address */

	/* if specified, this gets called before any pixel update,
       optionally return a pointer that will be passed to the
       update and tear down callbacks */
	void * (*begin_update)(running_machine *machine, int screen,
						   mame_bitmap *bitmap, const rectangle *cliprect);

	/* this gets called for every row, the driver must output
       x_count * hpixels_per_column pixels */
	void (*update_row)(mame_bitmap *bitmap, const rectangle *cliprect,
					   UINT16 ma, UINT8 ra, UINT16 y, UINT8 x_count, void *param);

	/* if specified, this gets called after all row updating is complete */
	void (*end_update)(mame_bitmap *bitmap, const rectangle *cliprect, void *param);

	/* if specified, this gets called for every change of the disply enable (DT pin 18) */
	void (*display_enable_changed)(int display_enabled);
};


/* Deprectated - use crtc6845_init to set up for save states only, but not to configure the screen */
void crtc6845_init(void);

/* use crtc6845_init to set up for save states AND to configure the screen
   the 'which' argument is currently a dummy as only one instance is supported */
void crtc6845_config(int which, const crtc6845_interface *intf);

/* selects one of the registers for reading or writing */
WRITE8_HANDLER( crtc6845_address_w );
#define crtc6845_0_address_w  crtc6845_address_w

/* reads the currently selected register */
READ8_HANDLER( crtc6845_register_r );
#define crtc6845_0_register_r  crtc6845_register_r

/* writes the currently selected register */
WRITE8_HANDLER( crtc6845_register_w );
#define crtc6845_0_register_w  crtc6845_register_w

/* return the current value on the MA0-MA13 pins */
UINT16 crtc6845_get_ma(int which);

/* return the current value on the RA0-RA4 pins */
UINT8 crtc6845_get_ra(int which);

/* updates the screen -- this will call begin_update(),
   followed by update_row() reapeatedly and after all row
   updating is complete, end_update() */
VIDEO_UPDATE( crtc6845 );

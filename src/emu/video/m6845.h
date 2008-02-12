/**********************************************************************

    Motorola M6845 CRT controller emulation

**********************************************************************/

#ifndef M6845
#define M6845


typedef struct _m6845_t m6845_t;
typedef struct _m6845_interface m6845_interface;

struct _m6845_interface
{
	int scrnum;					/* screen we are acting on */
	int clock;					/* the clock (pin 21) of the chip */
	int hpixels_per_column;		/* number of pixels per video memory address */

	/* if specified, this gets called before any pixel update,
       optionally return a pointer that will be passed to the
       update and tear down callbacks */
	void * (*begin_update)(mame_bitmap *bitmap, const rectangle *cliprect);

	/* this gets called for every row, the driver must output
       x_count * hpixels_per_column pixels */
	void (*update_row)(mame_bitmap *bitmap, const rectangle *cliprect,
					   UINT16 ma, UINT8 ra, UINT16 y, UINT8 x_count, void *param);

	/* if specified, this gets called after all row updating is complete */
	void (*end_update)(mame_bitmap *bitmap, const rectangle *cliprect, void *param);

	/* if specified, this gets called for every change of the disply enable (DT pin 18) */
	void (*display_enable_changed)(int display_enabled);
};


/* use m6845_init to set up for save states.
   if intf is NULL, the emulator will NOT call video_configure_screen() */
m6845_t *m6845_config(const m6845_interface *intf);

/* selects one of the registers for reading or writing */
void m6845_address_w(m6845_t *m6845, UINT8 data);

/* reads the currently selected register */
UINT8 m6845_register_r(m6845_t *m6845);

/* writes the currently selected register */
void m6845_register_w(m6845_t *m6845, UINT8 data);

/* return the current value on the MA0-MA13 pins */
UINT16 m6845_get_ma(m6845_t *m6845);

/* return the current value on the RA0-RA4 pins */
UINT8 m6845_get_ra(m6845_t *m6845);

/* updates the screen -- this will call begin_update(),
   followed by update_row() reapeatedly and after all row
   updating is complete, end_update() */
void m6845_update(m6845_t *m6845, mame_bitmap *bitmap, const rectangle *cliprect);

#endif

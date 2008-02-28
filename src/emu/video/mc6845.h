/**********************************************************************

    Motorola MC6845 and compatible CRT controller emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#ifndef __MC6845__
#define __MC6845__


typedef struct _mc6845_t mc6845_t;
typedef struct _mc6845_interface mc6845_interface;

#define MC6845 mc6845_get_info
#define R6545 r6545_get_info


/* callback definitions */
typedef void * (*mc6845_begin_update_func)(running_machine *machine, mc6845_t *mc6845, mame_bitmap *bitmap, const rectangle *cliprect);
#define MC6845_BEGIN_UPDATE(name)	void *name(running_machine *machine, mc6845_t *mc6845, mame_bitmap *bitmap, const rectangle *cliprect)

typedef void (*mc6845_update_row_func)(running_machine *machine, mc6845_t *mc6845, mame_bitmap *bitmap,
					   				   const rectangle *cliprect, UINT16 ma, UINT8 ra,
					   				   UINT16 y, UINT8 x_count, INT8 cursor_x, void *param);
#define MC6845_UPDATE_ROW(name)		void name(running_machine *machine, mc6845_t *mc6845, mame_bitmap *bitmap,	\
					   						  const rectangle *cliprect, UINT16 ma, UINT8 ra,					\
					   						  UINT16 y, UINT8 x_count, INT8 cursor_x, void *param)

typedef void (*mc6845_end_update_func)(running_machine *machine, mc6845_t *mc6845,
					   				   mame_bitmap *bitmap, const rectangle *cliprect, void *param);
#define MC6845_END_UPDATE(name)		void name(running_machine *machine, mc6845_t *mc6845,						\
					   						  mame_bitmap *bitmap, const rectangle *cliprect, void *param)

typedef void (*mc6845_on_de_changed_func)(running_machine *machine, mc6845_t *mc6845, int display_enabled);
#define MC6845_ON_DE_CHANGED(name)	void name(running_machine *machine, mc6845_t *mc6845, int display_enabled)

typedef void (*mc6845_on_hsync_changed_func)(running_machine *machine, mc6845_t *mc6845, int hsync);
#define MC6845_ON_HSYNC_CHANGED(name)	void name(running_machine *machine, mc6845_t *mc6845, int hsync)

typedef void (*mc6845_on_vsync_changed_func)(running_machine *machine, mc6845_t *mc6845, int vsync);
#define MC6845_ON_VSYNC_CHANGED(name)	void name(running_machine *machine, mc6845_t *mc6845, int vsync)


/* interface */
struct _mc6845_interface
{
	int scrnum;					/* screen we are acting on */
	int clock;					/* the clock (pin 21) of the chip */
	int hpixels_per_column;		/* number of pixels per video memory address */

	/* if specified, this gets called before any pixel update,
       optionally return a pointer that will be passed to the
       update and tear down callbacks */
	mc6845_begin_update_func		begin_update;

	/* this gets called for every row, the driver must output
       x_count * hpixels_per_column pixels.
       cursor_x indicates the character position where the cursor is, or -1
       if there is no cursor on this row */
	mc6845_update_row_func			update_row;

	/* if specified, this gets called after all row updating is complete */
	mc6845_end_update_func			end_update;

	/* if specified, this gets called for every change of the disply enable pin (pin 18) */
	mc6845_on_de_changed_func		on_de_changed;

	/* if specified, this gets called for every change of the HSYNC pin (pin 39) */
	mc6845_on_hsync_changed_func	on_hsync_changed;

	/* if specified, this gets called for every change of the VSYNC pin (pin 40) */
	mc6845_on_vsync_changed_func	on_vsync_changed;
};


/* device interface */
void mc6845_get_info(running_machine *machine, void *token, UINT32 state, deviceinfo *info);
void r6545_get_info(running_machine *machine, void *token, UINT32 state, deviceinfo *info);

/* select one of the registers for reading or writing */
void mc6845_address_w(mc6845_t *mc6845, UINT8 data);

/* read from the currently selected register */
UINT8 mc6845_register_r(mc6845_t *mc6845);

/* write to the currently selected register */
void mc6845_register_w(mc6845_t *mc6845, UINT8 data);

/* return the current value on the MA0-MA13 pins */
UINT16 mc6845_get_ma(mc6845_t *mc6845);

/* return the current value on the RA0-RA4 pins */
UINT8 mc6845_get_ra(mc6845_t *mc6845);

/* updates the screen -- this will call begin_update(),
   followed by update_row() reapeatedly and after all row
   updating is complete, end_update() */
void mc6845_update(mc6845_t *mc6845, mame_bitmap *bitmap, const rectangle *cliprect);


#endif

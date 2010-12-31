/**********************************************************************

    Motorola MC6845 and compatible CRT controller emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#ifndef __MC6845__
#define __MC6845__

#include "devlegcy.h"

DECLARE_LEGACY_DEVICE(MC6845, mc6845);
DECLARE_LEGACY_DEVICE(MC6845_1, mc6845_1);
DECLARE_LEGACY_DEVICE(R6545_1, r6545_1);
DECLARE_LEGACY_DEVICE(C6545_1, c6545_1);
DECLARE_LEGACY_DEVICE(H46505, h46505);
DECLARE_LEGACY_DEVICE(HD6845, hd6845);
DECLARE_LEGACY_DEVICE(SY6545_1, sy6545_1);
DECLARE_LEGACY_DEVICE(SY6845E, sy6845e);


#define MCFG_MC6845_ADD(_tag, _variant, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, _variant, _clock) \
	MCFG_DEVICE_CONFIG(_config)


/* callback definitions */
typedef void * (*mc6845_begin_update_func)(device_t *device, bitmap_t *bitmap, const rectangle *cliprect);
#define MC6845_BEGIN_UPDATE(name)	void *name(device_t *device, bitmap_t *bitmap, const rectangle *cliprect)

typedef void (*mc6845_update_row_func)(device_t *device, bitmap_t *bitmap,
									   const rectangle *cliprect, UINT16 ma, UINT8 ra,
									   UINT16 y, UINT8 x_count, INT8 cursor_x, void *param);
#define MC6845_UPDATE_ROW(name)		void name(device_t *device, bitmap_t *bitmap,	\
											  const rectangle *cliprect, UINT16 ma, UINT8 ra,	\
											  UINT16 y, UINT8 x_count, INT8 cursor_x, void *param)

typedef void (*mc6845_end_update_func)(device_t *device, bitmap_t *bitmap, const rectangle *cliprect, void *param);
#define MC6845_END_UPDATE(name)		void name(device_t *device, bitmap_t *bitmap, const rectangle *cliprect, void *param)

typedef void (*mc6845_on_update_addr_changed_func)(device_t *device, int address, int strobe);
#define MC6845_ON_UPDATE_ADDR_CHANGED(name)	void name(device_t *device, int address, int strobe)

/* interface */
typedef struct _mc6845_interface mc6845_interface;
struct _mc6845_interface
{
	const char *screen_tag;		/* screen we are acting on */
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
	devcb_write_line				out_de_func;

	/* if specified, this gets called for every change of the cursor pin (pin 19) */
	devcb_write_line				out_cur_func;

	/* if specified, this gets called for every change of the HSYNC pin (pin 39) */
	devcb_write_line				out_hsync_func;

	/* if specified, this gets called for every change of the VSYNC pin (pin 40) */
	devcb_write_line				out_vsync_func;

	/* Called whenenever the update address changes
     * For vblank/hblank timing strobe indicates the physical update.
     * vblank/hblank timing not supported yet! */

	mc6845_on_update_addr_changed_func	on_update_addr_changed;
};

extern const mc6845_interface mc6845_null_interface;


/* select one of the registers for reading or writing */
WRITE8_DEVICE_HANDLER( mc6845_address_w );

/* read from the status register */
READ8_DEVICE_HANDLER( mc6845_status_r );

/* read from the currently selected register */
READ8_DEVICE_HANDLER( mc6845_register_r );

/* write to the currently selected register */
WRITE8_DEVICE_HANDLER( mc6845_register_w );

/* return the current value on the MA0-MA13 pins */
UINT16 mc6845_get_ma(device_t *device);

/* return the current value on the RA0-RA4 pins */
UINT8 mc6845_get_ra(device_t *device);

/* simulates the LO->HI clocking of the light pen pin (pin 3) */
void mc6845_assert_light_pen_input(device_t *device);

/* set the clock (pin 21) of the chip */
void mc6845_set_clock(device_t *device, int clock);

/* set number of pixels per video memory address */
void mc6845_set_hpixels_per_column(device_t *device, int hpixels_per_column);

/* updates the screen -- this will call begin_update(),
   followed by update_row() reapeatedly and after all row
   updating is complete, end_update() */
void mc6845_update(device_t *device, bitmap_t *bitmap, const rectangle *cliprect);

#endif

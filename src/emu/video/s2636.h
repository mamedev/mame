/**********************************************************************

    Signetics 2636 video chip

**********************************************************************/

#ifndef S2636
#define S2636


typedef struct _s2636_t s2636_t;

/* helpful macros to parse the bitmap returned by s2636_update */
#define S2636_IS_PIXEL_DRAWN(p)		(((p) & 0x08) ? TRUE : FALSE)
#define S2636_PIXEL_COLOR(p)		((p) & 0x07)

s2636_t *s2636_config(UINT8 *work_ram, int screen_height, int screen_width, int y_offset, int x_offset);

/* returns a BITMAP_FORMAT_INDEXED16 bitmap the size of the screen
   D0-D2 of each pixel is the pixel color
   D3 indicates whether the S2636 drew this pixel - 0 = not drawn, 1 = drawn */
mame_bitmap *s2636_update(s2636_t *s2636, const rectangle *cliprect);

#endif

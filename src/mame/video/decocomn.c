/* Common DECO video functions (general, not sorted by IC) */
/* I think most of this stuff is driver specific and really shouldn't be in a device at all.
   It was only put here because I wanted to split deco_tilegen1 to just be the device for the
   tilemap chips, and not contain all this extra unrelated stuff */


#include "emu.h"
#include "video/decocomn.h"
#include "ui.h"


typedef struct _decocomn_state decocomn_state;
struct _decocomn_state
{
	screen_device *screen;
	UINT16 *raster_display_list;
	UINT8 *dirty_palette;
	bitmap_t *sprite_priority_bitmap;
	UINT16 priority;
	int raster_display_position;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE decocomn_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == DECOCOMN);

	return (decocomn_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const decocomn_interface *get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == DECOCOMN));
	return (const decocomn_interface *) device->static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

/* Later games have double buffered paletteram - the real palette ram is
only updated on a DMA call */

WRITE16_DEVICE_HANDLER( decocomn_nonbuffered_palette_w )
{
	int r,g,b;

	COMBINE_DATA(&device->machine().generic.paletteram.u16[offset]);
	if (offset&1) offset--;

	b = (device->machine().generic.paletteram.u16[offset] >> 0) & 0xff;
	g = (device->machine().generic.paletteram.u16[offset + 1] >> 8) & 0xff;
	r = (device->machine().generic.paletteram.u16[offset + 1] >> 0) & 0xff;

	palette_set_color(device->machine(), offset / 2, MAKE_RGB(r,g,b));
}

WRITE16_DEVICE_HANDLER( decocomn_buffered_palette_w )
{
	decocomn_state *decocomn = get_safe_token(device);

	COMBINE_DATA(&device->machine().generic.paletteram.u16[offset]);

	decocomn->dirty_palette[offset / 2] = 1;
}

WRITE16_DEVICE_HANDLER( decocomn_palette_dma_w )
{
	decocomn_state *decocomn = get_safe_token(device);
	const int m = device->machine().total_colors();
	int r, g, b, i;

	for (i = 0; i < m; i++)
	{
		if (decocomn->dirty_palette[i])
		{
			decocomn->dirty_palette[i] = 0;

			b = (device->machine().generic.paletteram.u16[i * 2] >> 0) & 0xff;
			g = (device->machine().generic.paletteram.u16[i * 2 + 1] >> 8) & 0xff;
			r = (device->machine().generic.paletteram.u16[i * 2 + 1] >> 0) & 0xff;

			palette_set_color(device->machine(), i, MAKE_RGB(r,g,b));
		}
	}
}

/*****************************************************************************************/

/* */
READ16_DEVICE_HANDLER( decocomn_71_r )
{
	return 0xffff;
}

WRITE16_DEVICE_HANDLER( decocomn_priority_w )
{
	decocomn_state *decocomn = get_safe_token(device);
	decocomn->priority = data;
}

READ16_DEVICE_HANDLER( decocomn_priority_r )
{
	decocomn_state *decocomn = get_safe_token(device);
	return decocomn->priority;
}


/******************************************************************************/

/*****************************************************************************************/

void decocomn_clear_sprite_priority_bitmap( device_t *device )
{
	decocomn_state *decocomn = get_safe_token(device);

	if (decocomn->sprite_priority_bitmap)
		bitmap_fill(decocomn->sprite_priority_bitmap, NULL, 0);
}

/* A special pdrawgfx z-buffered sprite renderer that is needed to properly draw multiple sprite sources with alpha */
void decocomn_pdrawgfx(
		device_t *device,
		bitmap_t *dest, const rectangle *clip, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, int sx, int sy,
		int transparent_color, UINT32 pri_mask, UINT32 sprite_mask, UINT8 write_pri, UINT8 alpha)
{
	decocomn_state *decocomn = get_safe_token(device);
	int ox, oy, cx, cy;
	int x_index, y_index, x, y;
	bitmap_t *priority_bitmap = gfx->machine().priority_bitmap;
	const pen_t *pal = &gfx->machine().pens[gfx->color_base + gfx->color_granularity * (color % gfx->total_colors)];
	const UINT8 *code_base = gfx_element_get_data(gfx, code % gfx->total_elements);

	/* check bounds */
	ox = sx;
	oy = sy;

	if (sx > 319 || sy > 247 || sx < -15 || sy < -7)
		return;

	if (sy < 0) sy = 0;
	if (sx < 0) sx = 0;

	if (sx > 319) cx = 319;
	else cx = ox + 16;

	cy = (sy - oy);

	if (flipy) y_index = 15 - cy; else y_index = cy;

	for (y = 0; y < 16 - cy; y++)
	{
		const UINT8 *source = code_base + (y_index * gfx->line_modulo);
		UINT32 *destb = BITMAP_ADDR32(dest, sy, 0);
		UINT8 *pri = BITMAP_ADDR8(priority_bitmap, sy, 0);
		UINT8 *spri = BITMAP_ADDR8(decocomn->sprite_priority_bitmap, sy, 0);

		if (sy >= 0 && sy < 248)
		{
			if (flipx) { source += 15 - (sx - ox); x_index = -1; }
			else       { source += (sx - ox); x_index = 1; }

			for (x = sx; x < cx; x++)
			{
				int c = *source;
				if (c != transparent_color && x >= 0 && x < 320)
				{
					if (pri_mask>pri[x] && sprite_mask>spri[x])
					{
						if (alpha != 0xff)
							destb[x] = alpha_blend_r32(destb[x], pal[c], alpha);
						else
							destb[x] = pal[c];
						if (write_pri)
							pri[x] |= pri_mask;
					}
					spri[x] |= sprite_mask;
				}
				source += x_index;
			}
		}

		sy++;
		if (sy > 247)
			return;
		if (flipy) y_index--; else y_index++;
	}
}

/*****************************************************************************************/

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( decocomn )
{
	decocomn_state *decocomn = get_safe_token(device);
	const decocomn_interface *intf = get_interface(device);
	int width, height;

	decocomn->screen = device->machine().device<screen_device>(intf->screen);
	width = decocomn->screen->width();
	height = decocomn->screen->height();

	decocomn->sprite_priority_bitmap = auto_bitmap_alloc(device->machine(), width, height, BITMAP_FORMAT_INDEXED8);

	decocomn->dirty_palette = auto_alloc_array_clear(device->machine(), UINT8, 4096);
	decocomn->raster_display_list = auto_alloc_array_clear(device->machine(), UINT16, 20 * 256 / 2);



	device->save_item(NAME(decocomn->priority));
	device->save_item(NAME(decocomn->raster_display_position));
	device->save_pointer(NAME(decocomn->dirty_palette), 4096);
	device->save_pointer(NAME(decocomn->raster_display_list), 20 * 256 / 2);
}

static DEVICE_RESET( decocomn )
{
	decocomn_state *decocomn = get_safe_token(device);

	decocomn->raster_display_position = 0;

	decocomn->priority = 0;
}


DEVICE_GET_INFO( decocomn )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(decocomn_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(decocomn);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(decocomn);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Data East Common Video Functions");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Data East Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}


DEFINE_LEGACY_DEVICE(DECOCOMN, decocomn);

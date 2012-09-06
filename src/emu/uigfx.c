/*********************************************************************

    uigfx.c

    Internal graphics viewer.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "emu.h"
#include "ui.h"
#include "uiinput.h"
#include "render.h"
#include "rendfont.h"
#include "rendutil.h"
#include "uigfx.h"



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _ui_gfx_state ui_gfx_state;
struct _ui_gfx_state
{
	UINT8			mode;				/* which mode are we in? */

	/* intermediate bitmaps */
	UINT8			bitmap_dirty;		/* is the bitmap dirty? */
	bitmap_rgb32 *	bitmap;				/* bitmap for drawing gfx and tilemaps */
	render_texture *texture;			/* texture for rendering the above bitmap */

	/* palette-specific data */
	struct
	{
		int		which;					/* which subset (palette or colortable)? */
		int		offset;					/* current offset of top,left item */
		int		count;					/* number of items per row */
	} palette;

	/* graphics-viewer-specific data */
	struct
	{
		int		set;					/* which set is visible */
		int		offset[MAX_GFX_ELEMENTS]; /* current offset of top,left item */
		int		color[MAX_GFX_ELEMENTS]; /* current color selected */
		int		count[MAX_GFX_ELEMENTS]; /* number of items per row */
		UINT8	rotate[MAX_GFX_ELEMENTS]; /* current rotation (orientation) value */
	} gfxset;

	/* tilemap-viewer-specific data */
	struct
	{
		int		which;					/* which tilemap are we viewing? */
		int		xoffs;					/* current X offset */
		int		yoffs;					/* current Y offset */
		int		zoom;					/* zoom factor */
		UINT8	rotate;					/* current rotation (orientation) value */
	} tilemap;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static ui_gfx_state ui_gfx;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void ui_gfx_exit(running_machine &machine);

/* palette handling */
static void palette_handle_keys(running_machine &machine, ui_gfx_state *state);
static void palette_handler(running_machine &machine, render_container *container, ui_gfx_state *state);

/* graphics set handling */
static void gfxset_handle_keys(running_machine &machine, ui_gfx_state *state, int xcells, int ycells);
static void gfxset_draw_item(running_machine &machine, gfx_element *gfx, int index, bitmap_rgb32 &bitmap, int dstx, int dsty, int color, int rotate);
static void gfxset_update_bitmap(running_machine &machine, ui_gfx_state *state, int xcells, int ycells, gfx_element *gfx);
static void gfxset_handler(running_machine &machine, render_container *container, ui_gfx_state *state);

/* tilemap handling */
static void tilemap_handle_keys(running_machine &machine, ui_gfx_state *state, int viswidth, int visheight);
static void tilemap_update_bitmap(running_machine &machine, ui_gfx_state *state, int width, int height);
static void tilemap_handler(running_machine &machine, render_container *container, ui_gfx_state *state);



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    ui_gfx_init - initialize the menu system
-------------------------------------------------*/

void ui_gfx_init(running_machine &machine)
{
	ui_gfx_state *state = &ui_gfx;
	int gfx;

	/* make sure we clean up after ourselves */
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(ui_gfx_exit), &machine));

	/* initialize our global state */
	memset(state, 0, sizeof(*state));

	/* set up the palette state */
	state->palette.count = 16;

	/* set up the graphics state */
	for (gfx = 0; gfx < MAX_GFX_ELEMENTS; gfx++)
	{
		state->gfxset.rotate[gfx] = machine.system().flags & ORIENTATION_MASK;
		state->gfxset.count[gfx] = 16;
	}

	/* set up the tilemap state */
	state->tilemap.rotate = machine.system().flags & ORIENTATION_MASK;
}


/*-------------------------------------------------
    ui_gfx_exit - clean up after ourselves
-------------------------------------------------*/

static void ui_gfx_exit(running_machine &machine)
{
	/* free the texture */
	machine.render().texture_free(ui_gfx.texture);
	ui_gfx.texture = NULL;

	/* free the bitmap */
	global_free(ui_gfx.bitmap);
	ui_gfx.bitmap = NULL;
}


/*-------------------------------------------------
    ui_gfx_ui_handler - primary UI handler
-------------------------------------------------*/

UINT32 ui_gfx_ui_handler(running_machine &machine, render_container *container, UINT32 uistate)
{
	ui_gfx_state *state = &ui_gfx;

	/* if we have nothing, implicitly cancel */
	if (machine.total_colors() == 0 && machine.colortable == NULL && machine.gfx[0] == NULL && machine.tilemap().count() == 0)
		goto cancel;

	/* if we're not paused, mark the bitmap dirty */
	if (!machine.paused())
		state->bitmap_dirty = TRUE;

	/* switch off the state to display something */
again:
	switch (state->mode)
	{
		case 0:
			/* if we have a palette, display it */
			if (machine.total_colors() > 0)
			{
				palette_handler(machine, container, state);
				break;
			}

			/* fall through...*/
			state->mode++;

		case 1:
			/* if we have graphics sets, display them */
			if (machine.gfx[0] != NULL)
			{
				gfxset_handler(machine, container, state);
				break;
			}

			/* fall through...*/
			state->mode++;

		case 2:
			/* if we have tilemaps, display them */
			if (machine.tilemap().count() > 0)
			{
				tilemap_handler(machine, container, state);
				break;
			}

			state->mode = 0;
			goto again;
	}

	/* handle keys */
	if (ui_input_pressed(machine, IPT_UI_SELECT))
	{
		state->mode = (state->mode + 1) % 3;
		state->bitmap_dirty = TRUE;
	}

	if (ui_input_pressed(machine, IPT_UI_PAUSE))
	{
		if (machine.paused())
			machine.resume();
		else
			machine.pause();
	}

	if (ui_input_pressed(machine, IPT_UI_CANCEL) || ui_input_pressed(machine, IPT_UI_SHOW_GFX))
		goto cancel;

	return uistate;

cancel:
	if (!uistate)
		machine.resume();
	state->bitmap_dirty = TRUE;
	return UI_HANDLER_CANCEL;
}



/***************************************************************************
    PALETTE VIEWER
***************************************************************************/

/*-------------------------------------------------
    palette_handler - handler for the palette
    viewer
-------------------------------------------------*/

static void palette_handler(running_machine &machine, render_container *container, ui_gfx_state *state)
{
	int total = state->palette.which ? colortable_palette_get_size(machine.colortable) : machine.total_colors();
	const char *title = state->palette.which ? "COLORTABLE" : "PALETTE";
	const rgb_t *raw_color = palette_entry_list_raw(machine.palette);
	render_font *ui_font = ui_get_font(machine);
	float cellwidth, cellheight;
	float chwidth, chheight;
	float titlewidth;
	float x0, y0;
	render_bounds cellboxbounds;
	render_bounds boxbounds;
	int x, y, skip;

	/* add a half character padding for the box */
	chheight = ui_get_line_height(machine);
	chwidth = ui_font->char_width(chheight, machine.render().ui_aspect(), '0');
	boxbounds.x0 = 0.0f + 0.5f * chwidth;
	boxbounds.x1 = 1.0f - 0.5f * chwidth;
	boxbounds.y0 = 0.0f + 0.5f * chheight;
	boxbounds.y1 = 1.0f - 0.5f * chheight;

	/* the character cell box bounds starts a half character in from the box */
	cellboxbounds = boxbounds;
	cellboxbounds.x0 += 0.5f * chwidth;
	cellboxbounds.x1 -= 0.5f * chwidth;
	cellboxbounds.y0 += 0.5f * chheight;
	cellboxbounds.y1 -= 0.5f * chheight;

	/* add space on the left for 5 characters of text, plus a half character of padding */
	cellboxbounds.x0 += 5.5f * chwidth;

	/* add space on the top for a title, a half line of padding, a header, and another half line */
	cellboxbounds.y0 += 3.0f * chheight;

	/* figure out the title and expand the outer box to fit */
	titlewidth = ui_font->string_width(chheight, machine.render().ui_aspect(), title);
	x0 = 0.0f;
	if (boxbounds.x1 - boxbounds.x0 < titlewidth + chwidth)
		x0 = boxbounds.x0 - (0.5f - 0.5f * (titlewidth + chwidth));

	/* go ahead and draw the outer box now */
	ui_draw_outlined_box(container, boxbounds.x0 - x0, boxbounds.y0, boxbounds.x1 + x0, boxbounds.y1, UI_GFXVIEWER_BG_COLOR);

	/* draw the title */
	x0 = 0.5f - 0.5f * titlewidth;
	y0 = boxbounds.y0 + 0.5f * chheight;
	for (x = 0; title[x] != 0; x++)
	{
		container->add_char(x0, y0, chheight, machine.render().ui_aspect(), ARGB_WHITE, *ui_font, title[x]);
		x0 += ui_font->char_width(chheight, machine.render().ui_aspect(), title[x]);
	}

	/* compute the cell size */
	cellwidth = (cellboxbounds.x1 - cellboxbounds.x0) / (float)state->palette.count;
	cellheight = (cellboxbounds.y1 - cellboxbounds.y0) / (float)state->palette.count;

	/* draw the top column headers */
	skip = (int)(chwidth / cellwidth);
	for (x = 0; x < state->palette.count; x += 1 + skip)
	{
		x0 = boxbounds.x0 + 6.0f * chwidth + (float)x * cellwidth;
		y0 = boxbounds.y0 + 2.0f * chheight;
		container->add_char(x0 + 0.5f * (cellwidth - chwidth), y0, chheight, machine.render().ui_aspect(), ARGB_WHITE, *ui_font, "0123456789ABCDEF"[x & 0xf]);

		/* if we're skipping, draw a point between the character and the box to indicate which */
		/* one it's referring to */
		if (skip != 0)
			container->add_point(x0 + 0.5f * cellwidth, 0.5f * (y0 + chheight + cellboxbounds.y0), UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}

	/* draw the side column headers */
	skip = (int)(chheight / cellheight);
	for (y = 0; y < state->palette.count; y += 1 + skip)

		/* only display if there is data to show */
		if (state->palette.offset + y * state->palette.count < total)
		{
			char buffer[10];

			/* if we're skipping, draw a point between the character and the box to indicate which */
			/* one it's referring to */
			x0 = boxbounds.x0 + 5.5f * chwidth;
			y0 = boxbounds.y0 + 3.5f * chheight + (float)y * cellheight;
			if (skip != 0)
				container->add_point(0.5f * (x0 + cellboxbounds.x0), y0 + 0.5f * cellheight, UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

			/* draw the row header */
			sprintf(buffer, "%5X", state->palette.offset + y * state->palette.count);
			for (x = 4; x >= 0; x--)
			{
				x0 -= ui_font->char_width(chheight, machine.render().ui_aspect(), buffer[x]);
				container->add_char(x0, y0 + 0.5f * (cellheight - chheight), chheight, machine.render().ui_aspect(), ARGB_WHITE, *ui_font, buffer[x]);
			}
		}

	/* now add the rectangles for the colors */
	for (y = 0; y < state->palette.count; y++)
		for (x = 0; x < state->palette.count; x++)
		{
			int index = state->palette.offset + y * state->palette.count + x;
			if (index < total)
			{
				pen_t pen = state->palette.which ? colortable_palette_get_color(machine.colortable, index) : raw_color[index];
				container->add_rect(cellboxbounds.x0 + x * cellwidth, cellboxbounds.y0 + y * cellheight,
									cellboxbounds.x0 + (x + 1) * cellwidth, cellboxbounds.y0 + (y + 1) * cellheight,
									0xff000000 | pen, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			}
		}

	/* handle keys */
	palette_handle_keys(machine, state);
}


/*-------------------------------------------------
    palette_handler - handle key inputs for the
    palette viewer
-------------------------------------------------*/

static void palette_handle_keys(running_machine &machine, ui_gfx_state *state)
{
	int rowcount, screencount;
	int total;

	/* handle zoom (minus,plus) */
	if (ui_input_pressed(machine, IPT_UI_ZOOM_OUT))
		state->palette.count /= 2;
	if (ui_input_pressed(machine, IPT_UI_ZOOM_IN))
		state->palette.count *= 2;

	/* clamp within range */
	if (state->palette.count <= 4)
		state->palette.count = 4;
	if (state->palette.count > 64)
		state->palette.count = 64;

	/* handle colormap selection (open bracket,close bracket) */
	if (ui_input_pressed(machine, IPT_UI_PREV_GROUP))
		state->palette.which--;
	if (ui_input_pressed(machine, IPT_UI_NEXT_GROUP))
		state->palette.which++;

	/* clamp within range */
	if (state->palette.which < 0)
		state->palette.which = 1;
	if (state->palette.which > (int)(machine.colortable != NULL))
		state->palette.which = (int)(machine.colortable != NULL);

	/* cache some info in locals */
	total = state->palette.which ? colortable_palette_get_size(machine.colortable) : machine.total_colors();

	/* determine number of entries per row and total */
	rowcount = state->palette.count;
	screencount = rowcount * rowcount;

	/* handle keyboard navigation */
	if (ui_input_pressed_repeat(machine, IPT_UI_UP, 4))
		state->palette.offset -= rowcount;
	if (ui_input_pressed_repeat(machine, IPT_UI_DOWN, 4))
		state->palette.offset += rowcount;
	if (ui_input_pressed_repeat(machine, IPT_UI_PAGE_UP, 6))
		state->palette.offset -= screencount;
	if (ui_input_pressed_repeat(machine, IPT_UI_PAGE_DOWN, 6))
		state->palette.offset += screencount;
	if (ui_input_pressed_repeat(machine, IPT_UI_HOME, 4))
		state->palette.offset = 0;
	if (ui_input_pressed_repeat(machine, IPT_UI_END, 4))
		state->palette.offset = total;

	/* clamp within range */
	if (state->palette.offset + screencount > ((total + rowcount - 1) / rowcount) * rowcount)
		state->palette.offset = ((total + rowcount - 1) / rowcount) * rowcount - screencount;
	if (state->palette.offset < 0)
		state->palette.offset = 0;
}



/***************************************************************************
    GRAPHICS VIEWER
***************************************************************************/

/*-------------------------------------------------
    gfxset_handler - handler for the graphics
    viewer
-------------------------------------------------*/

static void gfxset_handler(running_machine &machine, render_container *container, ui_gfx_state *state)
{
	render_font *ui_font = ui_get_font(machine);
	int set = state->gfxset.set;
	gfx_element *gfx = machine.gfx[set];
	float fullwidth, fullheight;
	float cellwidth, cellheight;
	float chwidth, chheight;
	float titlewidth;
	//float cellaspect;
	float x0, y0;
	render_bounds cellboxbounds;
	render_bounds boxbounds;
	int cellboxwidth, cellboxheight;
	int targwidth = machine.render().ui_target().width();
	int targheight = machine.render().ui_target().height();
	int cellxpix, cellypix;
	int xcells, ycells;
	int pixelscale = 0;
	int x, y, skip;
	char title[100];

	/* add a half character padding for the box */
	chheight = ui_get_line_height(machine);
	chwidth = ui_font->char_width(chheight, machine.render().ui_aspect(), '0');
	boxbounds.x0 = 0.0f + 0.5f * chwidth;
	boxbounds.x1 = 1.0f - 0.5f * chwidth;
	boxbounds.y0 = 0.0f + 0.5f * chheight;
	boxbounds.y1 = 1.0f - 0.5f * chheight;

	/* the character cell box bounds starts a half character in from the box */
	cellboxbounds = boxbounds;
	cellboxbounds.x0 += 0.5f * chwidth;
	cellboxbounds.x1 -= 0.5f * chwidth;
	cellboxbounds.y0 += 0.5f * chheight;
	cellboxbounds.y1 -= 0.5f * chheight;

	/* add space on the left for 5 characters of text, plus a half character of padding */
	cellboxbounds.x0 += 5.5f * chwidth;

	/* add space on the top for a title, a half line of padding, a header, and another half line */
	cellboxbounds.y0 += 3.0f * chheight;

	/* convert back to pixels */
	cellboxwidth = (cellboxbounds.x1 - cellboxbounds.x0) * (float)targwidth;
	cellboxheight = (cellboxbounds.y1 - cellboxbounds.y0) * (float)targheight;

	/* compute the number of source pixels in a cell */
	cellxpix = 1 + ((state->gfxset.rotate[state->gfxset.set] & ORIENTATION_SWAP_XY) ? gfx->height() : gfx->width());
	cellypix = 1 + ((state->gfxset.rotate[state->gfxset.set] & ORIENTATION_SWAP_XY) ? gfx->width() : gfx->height());

	/* compute the largest pixel scale factor that still fits */
	xcells = state->gfxset.count[set];
	while (xcells > 1)
	{
		pixelscale = (cellboxwidth / xcells) / cellxpix;
		if (pixelscale != 0)
			break;
		xcells--;
	}

	/* worst case, we need a pixel scale of 1 */
	pixelscale = MAX(1, pixelscale);

	/* in the Y direction, we just display as many as we can */
	ycells = cellboxheight / (pixelscale * cellypix);

	/* now determine the actual cellbox size */
	cellboxwidth = MIN(cellboxwidth, xcells * pixelscale * cellxpix);
	cellboxheight = MIN(cellboxheight, ycells * pixelscale * cellypix);

	/* compute the size of a single cell at this pixel scale factor, as well as the aspect ratio */
	cellwidth = (cellboxwidth / (float)xcells) / (float)targwidth;
	cellheight = (cellboxheight / (float)ycells) / (float)targheight;
	//cellaspect = cellwidth / cellheight;

	/* working from the new width/height, recompute the boxbounds */
	fullwidth = (float)cellboxwidth / (float)targwidth + 6.5f * chwidth;
	fullheight = (float)cellboxheight / (float)targheight + 4.0f * chheight;

	/* recompute boxbounds from this */
	boxbounds.x0 = (1.0f - fullwidth) * 0.5f;
	boxbounds.x1 = boxbounds.x0 + fullwidth;
	boxbounds.y0 = (1.0f - fullheight) * 0.5f;
	boxbounds.y1 = boxbounds.y0 + fullheight;

	/* figure out the title and expand the outer box to fit */
	for (x = 0; x < MAX_GFX_ELEMENTS && machine.gfx[x] != NULL; x++) ;
	sprintf(title, "GFX %d/%d %dx%d COLOR %X", state->gfxset.set, x - 1, gfx->width(), gfx->height(), state->gfxset.color[set]);
	titlewidth = ui_font->string_width(chheight, machine.render().ui_aspect(), title);
	x0 = 0.0f;
	if (boxbounds.x1 - boxbounds.x0 < titlewidth + chwidth)
		x0 = boxbounds.x0 - (0.5f - 0.5f * (titlewidth + chwidth));

	/* go ahead and draw the outer box now */
	ui_draw_outlined_box(container, boxbounds.x0 - x0, boxbounds.y0, boxbounds.x1 + x0, boxbounds.y1, UI_GFXVIEWER_BG_COLOR);

	/* draw the title */
	x0 = 0.5f - 0.5f * titlewidth;
	y0 = boxbounds.y0 + 0.5f * chheight;
	for (x = 0; title[x] != 0; x++)
	{
		container->add_char(x0, y0, chheight, machine.render().ui_aspect(), ARGB_WHITE, *ui_font, title[x]);
		x0 += ui_font->char_width(chheight, machine.render().ui_aspect(), title[x]);
	}

	/* draw the top column headers */
	skip = (int)(chwidth / cellwidth);
	for (x = 0; x < xcells; x += 1 + skip)
	{
		x0 = boxbounds.x0 + 6.0f * chwidth + (float)x * cellwidth;
		y0 = boxbounds.y0 + 2.0f * chheight;
		container->add_char(x0 + 0.5f * (cellwidth - chwidth), y0, chheight, machine.render().ui_aspect(), ARGB_WHITE, *ui_font, "0123456789ABCDEF"[x & 0xf]);

		/* if we're skipping, draw a point between the character and the box to indicate which */
		/* one it's referring to */
		if (skip != 0)
			container->add_point(x0 + 0.5f * cellwidth, 0.5f * (y0 + chheight + boxbounds.y0 + 3.5f * chheight), UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}

	/* draw the side column headers */
	skip = (int)(chheight / cellheight);
	for (y = 0; y < ycells; y += 1 + skip)

		/* only display if there is data to show */
		if (state->gfxset.offset[set] + y * xcells < gfx->elements())
		{
			char buffer[10];

			/* if we're skipping, draw a point between the character and the box to indicate which */
			/* one it's referring to */
			x0 = boxbounds.x0 + 5.5f * chwidth;
			y0 = boxbounds.y0 + 3.5f * chheight + (float)y * cellheight;
			if (skip != 0)
				container->add_point(0.5f * (x0 + boxbounds.x0 + 6.0f * chwidth), y0 + 0.5f * cellheight, UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

			/* draw the row header */
			sprintf(buffer, "%5X", state->gfxset.offset[set] + y * xcells);
			for (x = 4; x >= 0; x--)
			{
				x0 -= ui_font->char_width(chheight, machine.render().ui_aspect(), buffer[x]);
				container->add_char(x0, y0 + 0.5f * (cellheight - chheight), chheight, machine.render().ui_aspect(), ARGB_WHITE, *ui_font, buffer[x]);
			}
		}

	/* update the bitmap */
	gfxset_update_bitmap(machine, state, xcells, ycells, gfx);

	/* add the final quad */
	container->add_quad(boxbounds.x0 + 6.0f * chwidth, boxbounds.y0 + 3.5f * chheight,
						boxbounds.x0 + 6.0f * chwidth + (float)cellboxwidth / (float)targwidth,
						boxbounds.y0 + 3.5f * chheight + (float)cellboxheight / (float)targheight,
						ARGB_WHITE, state->texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	/* handle keyboard navigation before drawing */
	gfxset_handle_keys(machine, state, xcells, ycells);
}


/*-------------------------------------------------
    gfxset_handle_keys - handle keys for the
    graphics viewer
-------------------------------------------------*/

static void gfxset_handle_keys(running_machine &machine, ui_gfx_state *state, int xcells, int ycells)
{
	ui_gfx_state oldstate = *state;
	gfx_element *gfx;
	int temp, set;

	/* handle gfxset selection (open bracket,close bracket) */
	if (ui_input_pressed(machine, IPT_UI_PREV_GROUP))
	{
		for (temp = state->gfxset.set - 1; temp >= 0; temp--)
			if (machine.gfx[temp] != NULL)
				break;
		if (temp >= 0)
			state->gfxset.set = temp;
	}
	if (ui_input_pressed(machine, IPT_UI_NEXT_GROUP))
	{
		for (temp = state->gfxset.set + 1; temp < MAX_GFX_ELEMENTS; temp++)
			if (machine.gfx[temp] != NULL)
				break;
		if (temp < MAX_GFX_ELEMENTS)
			state->gfxset.set = temp;
	}

	/* cache some info in locals */
	set = state->gfxset.set;
	gfx = machine.gfx[set];

	/* handle cells per line (minus,plus) */
	if (ui_input_pressed(machine, IPT_UI_ZOOM_OUT))
		state->gfxset.count[set] = xcells - 1;
	if (ui_input_pressed(machine, IPT_UI_ZOOM_IN))
		state->gfxset.count[set] = xcells + 1;

	/* clamp within range */
	if (state->gfxset.count[set] < 2)
		state->gfxset.count[set] = 2;
	if (state->gfxset.count[set] > 32)
		state->gfxset.count[set] = 32;

	/* handle rotation (R) */
	if (ui_input_pressed(machine, IPT_UI_ROTATE))
		state->gfxset.rotate[set] = orientation_add(ROT90, state->gfxset.rotate[set]);

	/* handle navigation within the cells (up,down,pgup,pgdown) */
	if (ui_input_pressed_repeat(machine, IPT_UI_UP, 4))
		state->gfxset.offset[set] -= xcells;
	if (ui_input_pressed_repeat(machine, IPT_UI_DOWN, 4))
		state->gfxset.offset[set] += xcells;
	if (ui_input_pressed_repeat(machine, IPT_UI_PAGE_UP, 6))
		state->gfxset.offset[set] -= xcells * ycells;
	if (ui_input_pressed_repeat(machine, IPT_UI_PAGE_DOWN, 6))
		state->gfxset.offset[set] += xcells * ycells;
	if (ui_input_pressed_repeat(machine, IPT_UI_HOME, 4))
		state->gfxset.offset[set] = 0;
	if (ui_input_pressed_repeat(machine, IPT_UI_END, 4))
		state->gfxset.offset[set] = gfx->elements();

	/* clamp within range */
	if (state->gfxset.offset[set] + xcells * ycells > ((gfx->elements() + xcells - 1) / xcells) * xcells)
		state->gfxset.offset[set] = ((gfx->elements() + xcells - 1) / xcells) * xcells - xcells * ycells;
	if (state->gfxset.offset[set] < 0)
		state->gfxset.offset[set] = 0;

	/* handle color selection (left,right) */
	if (ui_input_pressed_repeat(machine, IPT_UI_LEFT, 4))
		state->gfxset.color[set] -= 1;
	if (ui_input_pressed_repeat(machine, IPT_UI_RIGHT, 4))
		state->gfxset.color[set] += 1;

	/* clamp within range */
	if (state->gfxset.color[set] >= (int)gfx->colors())
		state->gfxset.color[set] = gfx->colors() - 1;
	if (state->gfxset.color[set] < 0)
		state->gfxset.color[set] = 0;

	/* if something changed, we need to force an update to the bitmap */
	if (state->gfxset.set != oldstate.gfxset.set ||
		state->gfxset.offset[set] != oldstate.gfxset.offset[set] ||
		state->gfxset.rotate[set] != oldstate.gfxset.rotate[set] ||
		state->gfxset.color[set] != oldstate.gfxset.color[set] ||
		state->gfxset.count[set] != oldstate.gfxset.count[set])
	{
		state->bitmap_dirty = TRUE;
	}
}


/*-------------------------------------------------
    gfxset_update_bitmap - redraw the current
    graphics view bitmap
-------------------------------------------------*/

static void gfxset_update_bitmap(running_machine &machine, ui_gfx_state *state, int xcells, int ycells, gfx_element *gfx)
{
	int set = state->gfxset.set;
	int cellxpix, cellypix;
	int x, y;

	/* compute the number of source pixels in a cell */
	cellxpix = 1 + ((state->gfxset.rotate[set] & ORIENTATION_SWAP_XY) ? gfx->height() : gfx->width());
	cellypix = 1 + ((state->gfxset.rotate[set] & ORIENTATION_SWAP_XY) ? gfx->width() : gfx->height());

	/* realloc the bitmap if it is too small */
	if (state->bitmap == NULL || state->texture == NULL || state->bitmap->bpp() != 32 || state->bitmap->width() != cellxpix * xcells || state->bitmap->height() != cellypix * ycells)
	{
		/* free the old stuff */
		machine.render().texture_free(state->texture);
		global_free(state->bitmap);

		/* allocate new stuff */
		state->bitmap = global_alloc(bitmap_rgb32(cellxpix * xcells, cellypix * ycells));
		state->texture = machine.render().texture_alloc();
		state->texture->set_bitmap(*state->bitmap, state->bitmap->cliprect(), TEXFORMAT_ARGB32);

		/* force a redraw */
		state->bitmap_dirty = TRUE;
	}

	/* handle the redraw */
	if (state->bitmap_dirty)
	{
		/* loop over rows */
		for (y = 0; y < ycells; y++)
		{
			rectangle cellbounds;

			/* make a rect that covers this row */
			cellbounds.set(0, state->bitmap->width() - 1, y * cellypix, (y + 1) * cellypix - 1);

			/* only display if there is data to show */
			if (state->gfxset.offset[set] + y * xcells < gfx->elements())
			{
				/* draw the individual cells */
				for (x = 0; x < xcells; x++)
				{
					int index = state->gfxset.offset[set] + y * xcells + x;

					/* update the bounds for this cell */
					cellbounds.min_x = x * cellxpix;
					cellbounds.max_x = (x + 1) * cellxpix - 1;

					/* only render if there is data */
					if (index < gfx->elements())
						gfxset_draw_item(machine, gfx, index, *state->bitmap, cellbounds.min_x, cellbounds.min_y, state->gfxset.color[set], state->gfxset.rotate[set]);

					/* otherwise, fill with transparency */
					else
						state->bitmap->fill(0, cellbounds);
				}
			}

			/* otherwise, fill with transparency */
			else
				state->bitmap->fill(0, cellbounds);
		}

		/* reset the texture to force an update */
		state->texture->set_bitmap(*state->bitmap, state->bitmap->cliprect(), TEXFORMAT_ARGB32);
		state->bitmap_dirty = FALSE;
	}
}


/*-------------------------------------------------
    gfxset_draw_item - draw a single item into
    the view
-------------------------------------------------*/

static void gfxset_draw_item(running_machine &machine, gfx_element *gfx, int index, bitmap_rgb32 &bitmap, int dstx, int dsty, int color, int rotate)
{
	static const pen_t default_palette[] =
	{
		MAKE_RGB(0,0,0), MAKE_RGB(0,0,255), MAKE_RGB(0,255,0), MAKE_RGB(0,255,255),
		MAKE_RGB(255,0,0), MAKE_RGB(255,0,255), MAKE_RGB(255,255,0), MAKE_RGB(255,255,255)
	};
	int width = (rotate & ORIENTATION_SWAP_XY) ? gfx->height() : gfx->width();
	int height = (rotate & ORIENTATION_SWAP_XY) ? gfx->width() : gfx->height();
	const rgb_t *palette = (machine.total_colors() != 0) ? palette_entry_list_raw(machine.palette) : NULL;
	UINT32 palette_mask = ~0;
	int x, y;

	if (palette != NULL)
		palette += gfx->colorbase() + color * gfx->granularity();
	else
	{
		palette = default_palette;
		palette_mask = 7;
	}

	/* loop over rows in the cell */
	for (y = 0; y < height; y++)
	{
		UINT32 *dest = &bitmap.pix32(dsty + y, dstx);
		const UINT8 *src = gfx->get_data(index);

		/* loop over columns in the cell */
		for (x = 0; x < width; x++)
		{
			int effx = x, effy = y;
			const UINT8 *s;

			/* compute effective x,y values after rotation */
			if (!(rotate & ORIENTATION_SWAP_XY))
			{
				if (rotate & ORIENTATION_FLIP_X)
					effx = gfx->width() - 1 - effx;
				if (rotate & ORIENTATION_FLIP_Y)
					effy = gfx->height() - 1 - effy;
			}
			else
			{
				int temp;
				if (rotate & ORIENTATION_FLIP_X)
					effx = gfx->height() - 1 - effx;
				if (rotate & ORIENTATION_FLIP_Y)
					effy = gfx->width() - 1 - effy;
				temp = effx; effx = effy; effy = temp;
			}

			/* get a pointer to the start of this source row */
			s = src + effy * gfx->rowbytes();

			/* extract the pixel */
			*dest++ = 0xff000000 | palette[s[effx] & palette_mask];
		}
	}
}



/***************************************************************************
    TILEMAP VIEWER
***************************************************************************/

/*-------------------------------------------------
    tilemap_handler - handler for the tilemap
    viewer
-------------------------------------------------*/

static void tilemap_handler(running_machine &machine, render_container *container, ui_gfx_state *state)
{
	render_font *ui_font = ui_get_font(machine);
	float chwidth, chheight;
	render_bounds mapboxbounds;
	render_bounds boxbounds;
	int targwidth = machine.render().ui_target().width();
	int targheight = machine.render().ui_target().height();
	float titlewidth;
	float x0, y0;
	int mapboxwidth, mapboxheight;
	int maxxscale, maxyscale;
	UINT32 mapwidth, mapheight;
	int x, pixelscale;
	char title[100];

	/* get the size of the tilemap itself */
	tilemap_t *tilemap = machine.tilemap().find(state->tilemap.which);
	mapwidth = tilemap->width();
	mapheight = tilemap->height();
	if (state->tilemap.rotate & ORIENTATION_SWAP_XY)
		{ UINT32 temp = mapwidth; mapwidth = mapheight; mapheight = temp; }

	/* add a half character padding for the box */
	chheight = ui_get_line_height(machine);
	chwidth = ui_font->char_width(chheight, machine.render().ui_aspect(), '0');
	boxbounds.x0 = 0.0f + 0.5f * chwidth;
	boxbounds.x1 = 1.0f - 0.5f * chwidth;
	boxbounds.y0 = 0.0f + 0.5f * chheight;
	boxbounds.y1 = 1.0f - 0.5f * chheight;

	/* the tilemap box bounds starts a half character in from the box */
	mapboxbounds = boxbounds;
	mapboxbounds.x0 += 0.5f * chwidth;
	mapboxbounds.x1 -= 0.5f * chwidth;
	mapboxbounds.y0 += 0.5f * chheight;
	mapboxbounds.y1 -= 0.5f * chheight;

	/* add space on the top for a title and a half line of padding */
	mapboxbounds.y0 += 1.5f * chheight;

	/* convert back to pixels */
	mapboxwidth = (mapboxbounds.x1 - mapboxbounds.x0) * (float)targwidth;
	mapboxheight = (mapboxbounds.y1 - mapboxbounds.y0) * (float)targheight;

	/* determine the maximum integral scaling factor */
	pixelscale = state->tilemap.zoom;
	if (pixelscale == 0)
	{
		for (maxxscale = 1; mapwidth * (maxxscale + 1) < mapboxwidth; maxxscale++) ;
		for (maxyscale = 1; mapheight * (maxyscale + 1) < mapboxheight; maxyscale++) ;
		pixelscale = MIN(maxxscale, maxyscale);
	}

	/* recompute the final box size */
	mapboxwidth = MIN(mapboxwidth, mapwidth * pixelscale);
	mapboxheight = MIN(mapboxheight, mapheight * pixelscale);

	/* recompute the bounds, centered within the existing bounds */
	mapboxbounds.x0 += 0.5f * ((mapboxbounds.x1 - mapboxbounds.x0) - (float)mapboxwidth / (float)targwidth);
	mapboxbounds.x1 = mapboxbounds.x0 + (float)mapboxwidth / (float)targwidth;
	mapboxbounds.y0 += 0.5f * ((mapboxbounds.y1 - mapboxbounds.y0) - (float)mapboxheight / (float)targheight);
	mapboxbounds.y1 = mapboxbounds.y0 + (float)mapboxheight / (float)targheight;

	/* now recompute the outer box against this new info */
	boxbounds.x0 = mapboxbounds.x0 - 0.5f * chwidth;
	boxbounds.x1 = mapboxbounds.x1 + 0.5f * chwidth;
	boxbounds.y0 = mapboxbounds.y0 - 2.0f * chheight;
	boxbounds.y1 = mapboxbounds.y1 + 0.5f * chheight;

	/* figure out the title and expand the outer box to fit */
	sprintf(title, "TMAP %d/%d %dx%d OFFS %d,%d", state->tilemap.which, machine.tilemap().count() - 1, mapwidth, mapheight, state->tilemap.xoffs, state->tilemap.yoffs);
	titlewidth = ui_font->string_width(chheight, machine.render().ui_aspect(), title);
	if (boxbounds.x1 - boxbounds.x0 < titlewidth + chwidth)
	{
		boxbounds.x0 = 0.5f - 0.5f * (titlewidth + chwidth);
		boxbounds.x1 = boxbounds.x0 + titlewidth + chwidth;
	}

	/* go ahead and draw the outer box now */
	ui_draw_outlined_box(container, boxbounds.x0, boxbounds.y0, boxbounds.x1, boxbounds.y1, UI_GFXVIEWER_BG_COLOR);

	/* draw the title */
	x0 = 0.5f - 0.5f * titlewidth;
	y0 = boxbounds.y0 + 0.5f * chheight;
	for (x = 0; title[x] != 0; x++)
	{
		container->add_char(x0, y0, chheight, machine.render().ui_aspect(), ARGB_WHITE, *ui_font, title[x]);
		x0 += ui_font->char_width(chheight, machine.render().ui_aspect(), title[x]);
	}

	/* update the bitmap */
	tilemap_update_bitmap(machine, state, mapboxwidth / pixelscale, mapboxheight / pixelscale);

	/* add the final quad */
	container->add_quad(mapboxbounds.x0, mapboxbounds.y0,
						mapboxbounds.x1, mapboxbounds.y1,
						ARGB_WHITE, state->texture,
						PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(state->tilemap.rotate));

	/* handle keyboard input */
	tilemap_handle_keys(machine, state, mapboxwidth, mapboxheight);
}


/*-------------------------------------------------
    tilemap_handle_keys - handle keys for the
    tilemap view
-------------------------------------------------*/

static void tilemap_handle_keys(running_machine &machine, ui_gfx_state *state, int viswidth, int visheight)
{
	ui_gfx_state oldstate = *state;
	UINT32 mapwidth, mapheight;
	int step;

	/* handle tilemap selection (open bracket,close bracket) */
	if (ui_input_pressed(machine, IPT_UI_PREV_GROUP))
		state->tilemap.which--;
	if (ui_input_pressed(machine, IPT_UI_NEXT_GROUP))
		state->tilemap.which++;

	/* clamp within range */
	if (state->tilemap.which < 0)
		state->tilemap.which = 0;
	if (state->tilemap.which >= machine.tilemap().count())
		state->tilemap.which = machine.tilemap().count() - 1;

	/* cache some info in locals */
	tilemap_t *tilemap = machine.tilemap().find(state->tilemap.which);
	mapwidth = tilemap->width();
	mapheight = tilemap->height();

	/* handle zoom (minus,plus) */
	if (ui_input_pressed(machine, IPT_UI_ZOOM_OUT))
		state->tilemap.zoom--;
	if (ui_input_pressed(machine, IPT_UI_ZOOM_IN))
		state->tilemap.zoom++;

	/* clamp within range */
	if (state->tilemap.zoom < 0)
		state->tilemap.zoom = 0;
	if (state->tilemap.zoom > 8)
		state->tilemap.zoom = 8;
	if (state->tilemap.zoom != oldstate.tilemap.zoom)
	{
		if (state->tilemap.zoom != 0)
			popmessage("Zoom = %d", state->tilemap.zoom);
		else
			popmessage("Zoom Auto");
	}

	/* handle rotation (R) */
	if (ui_input_pressed(machine, IPT_UI_ROTATE))
		state->tilemap.rotate = orientation_add(ROT90, state->tilemap.rotate);

	/* handle navigation (up,down,left,right) */
	step = 8;
	if (machine.input().code_pressed(KEYCODE_LSHIFT)) step = 1;
	if (machine.input().code_pressed(KEYCODE_LCONTROL)) step = 64;
	if (ui_input_pressed_repeat(machine, IPT_UI_UP, 4))
		state->tilemap.yoffs -= step;
	if (ui_input_pressed_repeat(machine, IPT_UI_DOWN, 4))
		state->tilemap.yoffs += step;
	if (ui_input_pressed_repeat(machine, IPT_UI_LEFT, 6))
		state->tilemap.xoffs -= step;
	if (ui_input_pressed_repeat(machine, IPT_UI_RIGHT, 6))
		state->tilemap.xoffs += step;

	/* clamp within range */
	while (state->tilemap.xoffs < 0)
		state->tilemap.xoffs += mapwidth;
	while (state->tilemap.xoffs >= mapwidth)
		state->tilemap.xoffs -= mapwidth;
	while (state->tilemap.yoffs < 0)
		state->tilemap.yoffs += mapheight;
	while (state->tilemap.yoffs >= mapheight)
		state->tilemap.yoffs -= mapheight;

	/* if something changed, we need to force an update to the bitmap */
	if (state->tilemap.which != oldstate.tilemap.which ||
		state->tilemap.xoffs != oldstate.tilemap.xoffs ||
		state->tilemap.yoffs != oldstate.tilemap.yoffs ||
		state->tilemap.rotate != oldstate.tilemap.rotate)
	{
		state->bitmap_dirty = TRUE;
	}
}


/*-------------------------------------------------
    tilemap_update_bitmap - update the bitmap
    for the tilemap view
-------------------------------------------------*/

static void tilemap_update_bitmap(running_machine &machine, ui_gfx_state *state, int width, int height)
{
	/* swap the coordinates back if they were talking about a rotated surface */
	if (state->tilemap.rotate & ORIENTATION_SWAP_XY)
		{ UINT32 temp = width; width = height; height = temp; }

	/* realloc the bitmap if it is too small */
	if (state->bitmap == NULL || state->texture == NULL || state->bitmap->width() != width || state->bitmap->height() != height)
	{
		/* free the old stuff */
		machine.render().texture_free(state->texture);
		global_free(state->bitmap);

		/* allocate new stuff */
		state->bitmap = global_alloc(bitmap_rgb32(width, height));
		state->bitmap->set_palette(machine.palette);
		state->texture = machine.render().texture_alloc();
		state->texture->set_bitmap(*state->bitmap, state->bitmap->cliprect(), TEXFORMAT_RGB32);

		/* force a redraw */
		state->bitmap_dirty = TRUE;
	}

	/* handle the redraw */
	if (state->bitmap_dirty)
	{
		tilemap_t *tilemap = machine.tilemap().find(state->tilemap.which);
		tilemap->draw_debug(*state->bitmap, state->tilemap.xoffs, state->tilemap.yoffs);

		/* reset the texture to force an update */
		state->texture->set_bitmap(*state->bitmap, state->bitmap->cliprect(), TEXFORMAT_RGB32);
		state->bitmap_dirty = FALSE;
	}
}

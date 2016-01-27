// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/viewgfx.c

    Internal graphics viewer.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "uiinput.h"
#include "render.h"
#include "rendfont.h"
#include "rendutil.h"
#include "ui/viewgfx.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

enum ui_gfx_modes
{
	UI_GFX_PALETTE = 0,
	UI_GFX_GFXSET,
	UI_GFX_TILEMAP
};

const int MAX_GFX_DECODERS = 8;



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// information about a single gfx device
struct ui_gfx_info
{
	device_gfx_interface *interface;    // pointer to device's gfx interface
	UINT8 setcount;                     // how many gfx sets device has
	UINT8 rotate[MAX_GFX_ELEMENTS];     // current rotation (orientation) value
	UINT8 columns[MAX_GFX_ELEMENTS];    // number of items per row
	int   offset[MAX_GFX_ELEMENTS];     // current offset of top,left item
	int   color[MAX_GFX_ELEMENTS];      // current color selected
};

struct ui_gfx_state
{
	bool            started;        // have we called ui_gfx_count_devices() yet?
	UINT8           mode;           // which mode are we in?

	// intermediate bitmaps
	bool            bitmap_dirty;   // is the bitmap dirty?
	bitmap_rgb32 *  bitmap;         // bitmap for drawing gfx and tilemaps
	render_texture *texture;        // texture for rendering the above bitmap

	// palette-specific data
	struct
	{
		palette_device *device;     // pointer to current device
		int   devcount;             // how many palette devices exist
		int   devindex;             // which palette device is visible
		UINT8 which;                // which subset (pens or indirect colors)?
		UINT8 columns;              // number of items per row
		int   offset;               // current offset of top left item
	} palette;

	// graphics-specific data
	struct
	{
		UINT8   devcount;   // how many gfx devices exist
		UINT8   devindex;   // which device is visible
		UINT8   set;        // which set is visible
	} gfxset;

	// information about each gfx device
	ui_gfx_info gfxdev[MAX_GFX_DECODERS];

	// tilemap-specific data
	struct
	{
		int   which;                // which tilemap are we viewing?
		int   xoffs;                // current X offset
		int   yoffs;                // current Y offset
		int   zoom;                 // zoom factor
		UINT8 rotate;               // current rotation (orientation) value
	} tilemap;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static ui_gfx_state ui_gfx;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void ui_gfx_count_devices(running_machine &machine, ui_gfx_state &state);
static void ui_gfx_exit(running_machine &machine);

// palette handling
static void palette_set_device(running_machine &machine, ui_gfx_state &state);
static void palette_handle_keys(running_machine &machine, ui_gfx_state &state);
static void palette_handler(running_machine &machine, render_container *container, ui_gfx_state &state);

// graphics set handling
static void gfxset_handle_keys(running_machine &machine, ui_gfx_state &state, int xcells, int ycells);
static void gfxset_draw_item(running_machine &machine, gfx_element &gfx, int index, bitmap_rgb32 &bitmap, int dstx, int dsty, int color, int rotate);
static void gfxset_update_bitmap(running_machine &machine, ui_gfx_state &state, int xcells, int ycells, gfx_element &gfx);
static void gfxset_handler(running_machine &machine, render_container *container, ui_gfx_state &state);

// tilemap handling
static void tilemap_handle_keys(running_machine &machine, ui_gfx_state &state, int viswidth, int visheight);
static void tilemap_update_bitmap(running_machine &machine, ui_gfx_state &state, int width, int height);
static void tilemap_handler(running_machine &machine, render_container *container, ui_gfx_state &state);



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

//-------------------------------------------------
//  ui_gfx_init - initialize the graphics viewer
//-------------------------------------------------

void ui_gfx_init(running_machine &machine)
{
	ui_gfx_state *state = &ui_gfx;
	int rotate = machine.system().flags & ORIENTATION_MASK;

	// make sure we clean up after ourselves
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(ui_gfx_exit), &machine));

	// initialize our global state
	memset(state, 0, sizeof(*state));

	// set up the palette state
	state->palette.columns = 16;

	// set up the graphics state
	for (int i = 0; i < MAX_GFX_DECODERS; i++)
		for (int j = 0; j < MAX_GFX_ELEMENTS; j++)
		{
			state->gfxdev[i].rotate[j] = rotate;
			state->gfxdev[i].columns[j] = 16;
		}

	// set up the tilemap state
	state->tilemap.rotate = rotate;
}


//-------------------------------------------------
//  ui_gfx_count_devices - count the palettes,
//  gfx decoders and gfx sets in the machine
//-------------------------------------------------

static void ui_gfx_count_devices(running_machine &machine, ui_gfx_state &state)
{
	palette_device_iterator pal_iter(machine.root_device());
	gfx_interface_iterator gfx_iter(machine.root_device());

	// count the palette devices
	state.palette.devcount = pal_iter.count();

	// set the pointer to the first palette
	if (state.palette.devcount > 0)
		palette_set_device(machine, state);

	// count the gfx devices
	state.gfxset.devcount = 0;
	int tempcount = gfx_iter.count();

	// count the gfx sets in each device, skipping devices with none
	if (tempcount > 0)
	{
		device_gfx_interface *interface;
		int i, count;

		for (i = 0, interface = gfx_iter.first();
				i < tempcount && state.gfxset.devcount < MAX_GFX_DECODERS;
				i++, interface = gfx_iter.next())
		{
			for (count = 0; count < MAX_GFX_ELEMENTS && interface->gfx(count) != nullptr; count++) { }

			// count = index of first NULL
			if (count > 0)
			{
				state.gfxdev[state.gfxset.devcount].interface = interface;
				state.gfxdev[state.gfxset.devcount].setcount = count;
				state.gfxset.devcount++;
			}
		}
	}

	state.started = true;
}


//-------------------------------------------------
//  ui_gfx_exit - clean up after ourselves
//-------------------------------------------------

static void ui_gfx_exit(running_machine &machine)
{
	// free the texture
	machine.render().texture_free(ui_gfx.texture);
	ui_gfx.texture = nullptr;

	// free the bitmap
	global_free(ui_gfx.bitmap);
	ui_gfx.bitmap = nullptr;
}


//-------------------------------------------------
//  ui_gfx_is_relevant - returns 'true' if the
//  internal graphics viewer has relevance
//
//  NOTE: this must not be called before machine
//  initialization is complete, as some drivers
//  create or modify gfx sets in VIDEO_START
//-------------------------------------------------

bool ui_gfx_is_relevant(running_machine &machine)
{
	ui_gfx_state &state = ui_gfx;

	if (!state.started)
		ui_gfx_count_devices(machine, state);

	return state.palette.devcount > 0
		|| state.gfxset.devcount > 0
		|| machine.tilemap().count() > 0;
}


//-------------------------------------------------
//  ui_gfx_ui_handler - primary UI handler
//-------------------------------------------------

UINT32 ui_gfx_ui_handler(running_machine &machine, render_container *container, UINT32 uistate)
{
	ui_gfx_state &state = ui_gfx;

	// if we have nothing, implicitly cancel
	if (!ui_gfx_is_relevant(machine))
		goto cancel;

	// if we're not paused, mark the bitmap dirty
	if (!machine.paused())
		state.bitmap_dirty = true;

	// switch off the state to display something
again:
	switch (state.mode)
	{
		case UI_GFX_PALETTE:
			// if we have a palette, display it
			if (state.palette.devcount > 0)
			{
				palette_handler(machine, container, state);
				break;
			}

			// fall through...
			state.mode++;

		case UI_GFX_GFXSET:
			// if we have graphics sets, display them
			if (state.gfxset.devcount > 0)
			{
				gfxset_handler(machine, container, state);
				break;
			}

			// fall through...
			state.mode++;

		case UI_GFX_TILEMAP:
			// if we have tilemaps, display them
			if (machine.tilemap().count() > 0)
			{
				tilemap_handler(machine, container, state);
				break;
			}

			state.mode = UI_GFX_PALETTE;
			goto again;
	}

	// handle keys
	if (machine.ui_input().pressed(IPT_UI_SELECT))
	{
		state.mode = (state.mode + 1) % 3;
		state.bitmap_dirty = true;
	}

	if (machine.ui_input().pressed(IPT_UI_PAUSE))
	{
		if (machine.paused())
			machine.resume();
		else
			machine.pause();
	}

	if (machine.ui_input().pressed(IPT_UI_CANCEL) || machine.ui_input().pressed(IPT_UI_SHOW_GFX))
		goto cancel;

	return uistate;

cancel:
	if (!uistate)
		machine.resume();
	state.bitmap_dirty = true;
	return UI_HANDLER_CANCEL;
}



/***************************************************************************
    PALETTE VIEWER
***************************************************************************/

//-------------------------------------------------
//  palette_set_device - set the pointer to the
//  current palette device
//-------------------------------------------------

static void palette_set_device(running_machine &machine, ui_gfx_state &state)
{
	palette_device_iterator pal_iter(machine.root_device());
	state.palette.device = pal_iter.byindex(state.palette.devindex);
}


//-------------------------------------------------
//  palette_handler - handler for the palette
//  viewer
//-------------------------------------------------

static void palette_handler(running_machine &machine, render_container *container, ui_gfx_state &state)
{
	palette_device *palette = state.palette.device;

	int total = state.palette.which ? palette->indirect_entries() : palette->entries();
	const rgb_t *raw_color = palette->palette()->entry_list_raw();
	render_font *ui_font = machine.ui().get_font();
	float cellwidth, cellheight;
	float chwidth, chheight;
	float titlewidth;
	float x0, y0;
	render_bounds cellboxbounds;
	render_bounds boxbounds;
	int x, y, skip;
	char title[100];

	// add a half character padding for the box
	chheight = machine.ui().get_line_height();
	chwidth = ui_font->char_width(chheight, machine.render().ui_aspect(), '0');
	boxbounds.x0 = 0.0f + 0.5f * chwidth;
	boxbounds.x1 = 1.0f - 0.5f * chwidth;
	boxbounds.y0 = 0.0f + 0.5f * chheight;
	boxbounds.y1 = 1.0f - 0.5f * chheight;

	// the character cell box bounds starts a half character in from the box
	cellboxbounds = boxbounds;
	cellboxbounds.x0 += 0.5f * chwidth;
	cellboxbounds.x1 -= 0.5f * chwidth;
	cellboxbounds.y0 += 0.5f * chheight;
	cellboxbounds.y1 -= 0.5f * chheight;

	// add space on the left for 5 characters of text, plus a half character of padding
	cellboxbounds.x0 += 5.5f * chwidth;

	// add space on the top for a title, a half line of padding, a header, and another half line
	cellboxbounds.y0 += 3.0f * chheight;

	// figure out the title and expand the outer box to fit
	const char *suffix = palette->indirect_entries() == 0 ? "" : state.palette.which ? " COLORS" : " PENS";
	sprintf(title, "'%s'%s", palette->tag(), suffix);
	titlewidth = ui_font->string_width(chheight, machine.render().ui_aspect(), title);
	x0 = 0.0f;
	if (boxbounds.x1 - boxbounds.x0 < titlewidth + chwidth)
		x0 = boxbounds.x0 - (0.5f - 0.5f * (titlewidth + chwidth));

	// go ahead and draw the outer box now
	machine.ui().draw_outlined_box(container, boxbounds.x0 - x0, boxbounds.y0, boxbounds.x1 + x0, boxbounds.y1, UI_GFXVIEWER_BG_COLOR);

	// draw the title
	x0 = 0.5f - 0.5f * titlewidth;
	y0 = boxbounds.y0 + 0.5f * chheight;
	for (x = 0; title[x] != 0; x++)
	{
		container->add_char(x0, y0, chheight, machine.render().ui_aspect(), ARGB_WHITE, *ui_font, title[x]);
		x0 += ui_font->char_width(chheight, machine.render().ui_aspect(), title[x]);
	}

	// compute the cell size
	cellwidth = (cellboxbounds.x1 - cellboxbounds.x0) / (float)state.palette.columns;
	cellheight = (cellboxbounds.y1 - cellboxbounds.y0) / (float)state.palette.columns;

	// draw the top column headers
	skip = (int)(chwidth / cellwidth);
	for (x = 0; x < state.palette.columns; x += 1 + skip)
	{
		x0 = boxbounds.x0 + 6.0f * chwidth + (float)x * cellwidth;
		y0 = boxbounds.y0 + 2.0f * chheight;
		container->add_char(x0 + 0.5f * (cellwidth - chwidth), y0, chheight, machine.render().ui_aspect(), ARGB_WHITE, *ui_font, "0123456789ABCDEF"[x & 0xf]);

		// if we're skipping, draw a point between the character and the box to indicate which
		// one it's referring to
		if (skip != 0)
			container->add_point(x0 + 0.5f * cellwidth, 0.5f * (y0 + chheight + cellboxbounds.y0), UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}

	// draw the side column headers
	skip = (int)(chheight / cellheight);
	for (y = 0; y < state.palette.columns; y += 1 + skip)

		// only display if there is data to show
		if (state.palette.offset + y * state.palette.columns < total)
		{
			char buffer[10];

			// if we're skipping, draw a point between the character and the box to indicate which
			// one it's referring to
			x0 = boxbounds.x0 + 5.5f * chwidth;
			y0 = boxbounds.y0 + 3.5f * chheight + (float)y * cellheight;
			if (skip != 0)
				container->add_point(0.5f * (x0 + cellboxbounds.x0), y0 + 0.5f * cellheight, UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

			// draw the row header
			sprintf(buffer, "%5X", state.palette.offset + y * state.palette.columns);
			for (x = 4; x >= 0; x--)
			{
				x0 -= ui_font->char_width(chheight, machine.render().ui_aspect(), buffer[x]);
				container->add_char(x0, y0 + 0.5f * (cellheight - chheight), chheight, machine.render().ui_aspect(), ARGB_WHITE, *ui_font, buffer[x]);
			}
		}

	// now add the rectangles for the colors
	for (y = 0; y < state.palette.columns; y++)
		for (x = 0; x < state.palette.columns; x++)
		{
			int index = state.palette.offset + y * state.palette.columns + x;
			if (index < total)
			{
				pen_t pen = state.palette.which ? palette->indirect_color(index) : raw_color[index];
				container->add_rect(cellboxbounds.x0 + x * cellwidth, cellboxbounds.y0 + y * cellheight,
									cellboxbounds.x0 + (x + 1) * cellwidth, cellboxbounds.y0 + (y + 1) * cellheight,
									0xff000000 | pen, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			}
		}

	// handle keys
	palette_handle_keys(machine, state);
}


//-------------------------------------------------
//  palette_handle_keys - handle key inputs for
//  the palette viewer
//-------------------------------------------------

static void palette_handle_keys(running_machine &machine, ui_gfx_state &state)
{
	palette_device *palette = state.palette.device;
	int rowcount, screencount;
	int total;

	// handle zoom (minus,plus)
	if (machine.ui_input().pressed(IPT_UI_ZOOM_OUT))
		state.palette.columns /= 2;
	if (machine.ui_input().pressed(IPT_UI_ZOOM_IN))
		state.palette.columns *= 2;

	// clamp within range
	if (state.palette.columns <= 4)
		state.palette.columns = 4;
	if (state.palette.columns > 64)
		state.palette.columns = 64;

	// handle colormap selection (open bracket,close bracket)
	if (machine.ui_input().pressed(IPT_UI_PREV_GROUP))
	{
		if (state.palette.which)
			state.palette.which = 0;
		else if (state.palette.devindex > 0)
		{
			state.palette.devindex--;
			palette_set_device(machine, state);
			palette = state.palette.device;
			state.palette.which = (palette->indirect_entries() > 0);
		}
	}
	if (machine.ui_input().pressed(IPT_UI_NEXT_GROUP))
	{
		if (!state.palette.which && palette->indirect_entries() > 0)
			state.palette.which = 1;
		else if (state.palette.devindex < state.palette.devcount - 1)
		{
			state.palette.devindex++;
			palette_set_device(machine, state);
			palette = state.palette.device;
			state.palette.which = 0;
		}
	}

	// cache some info in locals
	total = state.palette.which ? palette->indirect_entries() : palette->entries();

	// determine number of entries per row and total
	rowcount = state.palette.columns;
	screencount = rowcount * rowcount;

	// handle keyboard navigation
	if (machine.ui_input().pressed_repeat(IPT_UI_UP, 4))
		state.palette.offset -= rowcount;
	if (machine.ui_input().pressed_repeat(IPT_UI_DOWN, 4))
		state.palette.offset += rowcount;
	if (machine.ui_input().pressed_repeat(IPT_UI_PAGE_UP, 6))
		state.palette.offset -= screencount;
	if (machine.ui_input().pressed_repeat(IPT_UI_PAGE_DOWN, 6))
		state.palette.offset += screencount;
	if (machine.ui_input().pressed_repeat(IPT_UI_HOME, 4))
		state.palette.offset = 0;
	if (machine.ui_input().pressed_repeat(IPT_UI_END, 4))
		state.palette.offset = total;

	// clamp within range
	if (state.palette.offset + screencount > ((total + rowcount - 1) / rowcount) * rowcount)
		state.palette.offset = ((total + rowcount - 1) / rowcount) * rowcount - screencount;
	if (state.palette.offset < 0)
		state.palette.offset = 0;
}



/***************************************************************************
    GRAPHICS VIEWER
***************************************************************************/

//-------------------------------------------------
//  gfxset_handler - handler for the graphics
//  viewer
//-------------------------------------------------

static void gfxset_handler(running_machine &machine, render_container *container, ui_gfx_state &state)
{
	render_font *ui_font = machine.ui().get_font();
	int dev = state.gfxset.devindex;
	int set = state.gfxset.set;
	ui_gfx_info &info = state.gfxdev[dev];
	device_gfx_interface &interface = *info.interface;
	gfx_element &gfx = *interface.gfx(set);
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

	// add a half character padding for the box
	chheight = machine.ui().get_line_height();
	chwidth = ui_font->char_width(chheight, machine.render().ui_aspect(), '0');
	boxbounds.x0 = 0.0f + 0.5f * chwidth;
	boxbounds.x1 = 1.0f - 0.5f * chwidth;
	boxbounds.y0 = 0.0f + 0.5f * chheight;
	boxbounds.y1 = 1.0f - 0.5f * chheight;

	// the character cell box bounds starts a half character in from the box
	cellboxbounds = boxbounds;
	cellboxbounds.x0 += 0.5f * chwidth;
	cellboxbounds.x1 -= 0.5f * chwidth;
	cellboxbounds.y0 += 0.5f * chheight;
	cellboxbounds.y1 -= 0.5f * chheight;

	// add space on the left for 5 characters of text, plus a half character of padding
	cellboxbounds.x0 += 5.5f * chwidth;

	// add space on the top for a title, a half line of padding, a header, and another half line
	cellboxbounds.y0 += 3.0f * chheight;

	// convert back to pixels
	cellboxwidth = (cellboxbounds.x1 - cellboxbounds.x0) * (float)targwidth;
	cellboxheight = (cellboxbounds.y1 - cellboxbounds.y0) * (float)targheight;

	// compute the number of source pixels in a cell
	cellxpix = 1 + ((info.rotate[set] & ORIENTATION_SWAP_XY) ? gfx.height() : gfx.width());
	cellypix = 1 + ((info.rotate[set] & ORIENTATION_SWAP_XY) ? gfx.width() : gfx.height());

	// compute the largest pixel scale factor that still fits
	xcells = info.columns[set];
	while (xcells > 1)
	{
		pixelscale = (cellboxwidth / xcells) / cellxpix;
		if (pixelscale != 0)
			break;
		xcells--;
	}
	info.columns[set] = xcells;

	// worst case, we need a pixel scale of 1
	pixelscale = MAX(1, pixelscale);

	// in the Y direction, we just display as many as we can
	ycells = cellboxheight / (pixelscale * cellypix);

	// now determine the actual cellbox size
	cellboxwidth = MIN(cellboxwidth, xcells * pixelscale * cellxpix);
	cellboxheight = MIN(cellboxheight, ycells * pixelscale * cellypix);

	// compute the size of a single cell at this pixel scale factor, as well as the aspect ratio
	cellwidth = (cellboxwidth / (float)xcells) / (float)targwidth;
	cellheight = (cellboxheight / (float)ycells) / (float)targheight;
	//cellaspect = cellwidth / cellheight;

	// working from the new width/height, recompute the boxbounds
	fullwidth = (float)cellboxwidth / (float)targwidth + 6.5f * chwidth;
	fullheight = (float)cellboxheight / (float)targheight + 4.0f * chheight;

	// recompute boxbounds from this
	boxbounds.x0 = (1.0f - fullwidth) * 0.5f;
	boxbounds.x1 = boxbounds.x0 + fullwidth;
	boxbounds.y0 = (1.0f - fullheight) * 0.5f;
	boxbounds.y1 = boxbounds.y0 + fullheight;

	// figure out the title and expand the outer box to fit
	sprintf(title, "'%s' %d/%d %dx%d COLOR %X",
					interface.device().tag(),
					set, info.setcount - 1,
					gfx.width(), gfx.height(),
					info.color[set]);
	titlewidth = ui_font->string_width(chheight, machine.render().ui_aspect(), title);
	x0 = 0.0f;
	if (boxbounds.x1 - boxbounds.x0 < titlewidth + chwidth)
		x0 = boxbounds.x0 - (0.5f - 0.5f * (titlewidth + chwidth));

	// go ahead and draw the outer box now
	machine.ui().draw_outlined_box(container, boxbounds.x0 - x0, boxbounds.y0, boxbounds.x1 + x0, boxbounds.y1, UI_GFXVIEWER_BG_COLOR);

	// draw the title
	x0 = 0.5f - 0.5f * titlewidth;
	y0 = boxbounds.y0 + 0.5f * chheight;
	for (x = 0; title[x] != 0; x++)
	{
		container->add_char(x0, y0, chheight, machine.render().ui_aspect(), ARGB_WHITE, *ui_font, title[x]);
		x0 += ui_font->char_width(chheight, machine.render().ui_aspect(), title[x]);
	}

	// draw the top column headers
	skip = (int)(chwidth / cellwidth);
	for (x = 0; x < xcells; x += 1 + skip)
	{
		x0 = boxbounds.x0 + 6.0f * chwidth + (float)x * cellwidth;
		y0 = boxbounds.y0 + 2.0f * chheight;
		container->add_char(x0 + 0.5f * (cellwidth - chwidth), y0, chheight, machine.render().ui_aspect(), ARGB_WHITE, *ui_font, "0123456789ABCDEF"[x & 0xf]);

		// if we're skipping, draw a point between the character and the box to indicate which
		// one it's referring to
		if (skip != 0)
			container->add_point(x0 + 0.5f * cellwidth, 0.5f * (y0 + chheight + boxbounds.y0 + 3.5f * chheight), UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}

	// draw the side column headers
	skip = (int)(chheight / cellheight);
	for (y = 0; y < ycells; y += 1 + skip)

		// only display if there is data to show
		if (info.offset[set] + y * xcells < gfx.elements())
		{
			char buffer[10];

			// if we're skipping, draw a point between the character and the box to indicate which
			// one it's referring to
			x0 = boxbounds.x0 + 5.5f * chwidth;
			y0 = boxbounds.y0 + 3.5f * chheight + (float)y * cellheight;
			if (skip != 0)
				container->add_point(0.5f * (x0 + boxbounds.x0 + 6.0f * chwidth), y0 + 0.5f * cellheight, UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

			// draw the row header
			sprintf(buffer, "%5X", info.offset[set] + y * xcells);
			for (x = 4; x >= 0; x--)
			{
				x0 -= ui_font->char_width(chheight, machine.render().ui_aspect(), buffer[x]);
				container->add_char(x0, y0 + 0.5f * (cellheight - chheight), chheight, machine.render().ui_aspect(), ARGB_WHITE, *ui_font, buffer[x]);
			}
		}

	// update the bitmap
	gfxset_update_bitmap(machine, state, xcells, ycells, gfx);

	// add the final quad
	container->add_quad(boxbounds.x0 + 6.0f * chwidth, boxbounds.y0 + 3.5f * chheight,
						boxbounds.x0 + 6.0f * chwidth + (float)cellboxwidth / (float)targwidth,
						boxbounds.y0 + 3.5f * chheight + (float)cellboxheight / (float)targheight,
						ARGB_WHITE, state.texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	// handle keyboard navigation before drawing
	gfxset_handle_keys(machine, state, xcells, ycells);
}


//-------------------------------------------------
//  gfxset_handle_keys - handle keys for the
//  graphics viewer
//-------------------------------------------------

static void gfxset_handle_keys(running_machine &machine, ui_gfx_state &state, int xcells, int ycells)
{
	// handle gfxset selection (open bracket,close bracket)
	if (machine.ui_input().pressed(IPT_UI_PREV_GROUP))
	{
		if (state.gfxset.set > 0)
			state.gfxset.set--;
		else if (state.gfxset.devindex > 0)
		{
			state.gfxset.devindex--;
			state.gfxset.set = state.gfxdev[state.gfxset.devindex].setcount - 1;
		}
		state.bitmap_dirty = true;
	}
	if (machine.ui_input().pressed(IPT_UI_NEXT_GROUP))
	{
		if (state.gfxset.set < state.gfxdev[state.gfxset.devindex].setcount - 1)
			state.gfxset.set++;
		else if (state.gfxset.devindex < state.gfxset.devcount - 1)
		{
			state.gfxset.devindex++;
			state.gfxset.set = 0;
		}
		state.bitmap_dirty = true;
	}

	// cache some info in locals
	int dev = state.gfxset.devindex;
	int set = state.gfxset.set;
	ui_gfx_info &info = state.gfxdev[dev];
	gfx_element &gfx = *info.interface->gfx(set);

	// handle cells per line (minus,plus)
	if (machine.ui_input().pressed(IPT_UI_ZOOM_OUT))
	{ info.columns[set] = xcells - 1; state.bitmap_dirty = true; }

	if (machine.ui_input().pressed(IPT_UI_ZOOM_IN))
	{ info.columns[set] = xcells + 1; state.bitmap_dirty = true; }

	// clamp within range
	if (info.columns[set] < 2)
	{ info.columns[set] = 2; state.bitmap_dirty = true; }
	if (info.columns[set] > 128)
	{ info.columns[set] = 128; state.bitmap_dirty = true; }

	// handle rotation (R)
	if (machine.ui_input().pressed(IPT_UI_ROTATE))
	{
		info.rotate[set] = orientation_add(ROT90, info.rotate[set]);
		state.bitmap_dirty = true;
	}

	// handle navigation within the cells (up,down,pgup,pgdown)
	if (machine.ui_input().pressed_repeat(IPT_UI_UP, 4))
	{ info.offset[set] -= xcells; state.bitmap_dirty = true; }
	if (machine.ui_input().pressed_repeat(IPT_UI_DOWN, 4))
	{ info.offset[set] += xcells; state.bitmap_dirty = true; }
	if (machine.ui_input().pressed_repeat(IPT_UI_PAGE_UP, 6))
	{ info.offset[set] -= xcells * ycells; state.bitmap_dirty = true; }
	if (machine.ui_input().pressed_repeat(IPT_UI_PAGE_DOWN, 6))
	{ info.offset[set] += xcells * ycells; state.bitmap_dirty = true; }
	if (machine.ui_input().pressed_repeat(IPT_UI_HOME, 4))
	{ info.offset[set] = 0; state.bitmap_dirty = true; }
	if (machine.ui_input().pressed_repeat(IPT_UI_END, 4))
	{ info.offset[set] = gfx.elements(); state.bitmap_dirty = true; }

	// clamp within range
	if (info.offset[set] + xcells * ycells > ((gfx.elements() + xcells - 1) / xcells) * xcells)
	{
		info.offset[set] = ((gfx.elements() + xcells - 1) / xcells) * xcells - xcells * ycells;
		state.bitmap_dirty = true;
	}
	if (info.offset[set] < 0)
	{ info.offset[set] = 0; state.bitmap_dirty = true; }

	// handle color selection (left,right)
	if (machine.ui_input().pressed_repeat(IPT_UI_LEFT, 4))
	{ info.color[set] -= 1; state.bitmap_dirty = true; }
	if (machine.ui_input().pressed_repeat(IPT_UI_RIGHT, 4))
	{ info.color[set] += 1; state.bitmap_dirty = true; }

	// clamp within range
	if (info.color[set] >= (int)gfx.colors())
	{ info.color[set] = gfx.colors() - 1; state.bitmap_dirty = true; }
	if (info.color[set] < 0)
	{ info.color[set] = 0; state.bitmap_dirty = true; }
}


//-------------------------------------------------
//  gfxset_update_bitmap - redraw the current
//  graphics view bitmap
//-------------------------------------------------

static void gfxset_update_bitmap(running_machine &machine, ui_gfx_state &state, int xcells, int ycells, gfx_element &gfx)
{
	int dev = state.gfxset.devindex;
	int set = state.gfxset.set;
	ui_gfx_info &info = state.gfxdev[dev];
	int cellxpix, cellypix;
	int x, y;

	// compute the number of source pixels in a cell
	cellxpix = 1 + ((info.rotate[set] & ORIENTATION_SWAP_XY) ? gfx.height() : gfx.width());
	cellypix = 1 + ((info.rotate[set] & ORIENTATION_SWAP_XY) ? gfx.width() : gfx.height());

	// realloc the bitmap if it is too small
	if (state.bitmap == nullptr || state.texture == nullptr || state.bitmap->bpp() != 32 || state.bitmap->width() != cellxpix * xcells || state.bitmap->height() != cellypix * ycells)
	{
		// free the old stuff
		machine.render().texture_free(state.texture);
		global_free(state.bitmap);

		// allocate new stuff
		state.bitmap = global_alloc(bitmap_rgb32(cellxpix * xcells, cellypix * ycells));
		state.texture = machine.render().texture_alloc();
		state.texture->set_bitmap(*state.bitmap, state.bitmap->cliprect(), TEXFORMAT_ARGB32);

		// force a redraw
		state.bitmap_dirty = true;
	}

	// handle the redraw
	if (state.bitmap_dirty)
	{
		// loop over rows
		for (y = 0; y < ycells; y++)
		{
			rectangle cellbounds;

			// make a rect that covers this row
			cellbounds.set(0, state.bitmap->width() - 1, y * cellypix, (y + 1) * cellypix - 1);

			// only display if there is data to show
			if (info.offset[set] + y * xcells < gfx.elements())
			{
				// draw the individual cells
				for (x = 0; x < xcells; x++)
				{
					int index = info.offset[set] + y * xcells + x;

					// update the bounds for this cell
					cellbounds.min_x = x * cellxpix;
					cellbounds.max_x = (x + 1) * cellxpix - 1;

					// only render if there is data
					if (index < gfx.elements())
						gfxset_draw_item(machine, gfx, index, *state.bitmap, cellbounds.min_x, cellbounds.min_y, info.color[set], info.rotate[set]);

					// otherwise, fill with transparency
					else
						state.bitmap->fill(0, cellbounds);
				}
			}

			// otherwise, fill with transparency
			else
				state.bitmap->fill(0, cellbounds);
		}

		// reset the texture to force an update
		state.texture->set_bitmap(*state.bitmap, state.bitmap->cliprect(), TEXFORMAT_ARGB32);
		state.bitmap_dirty = false;
	}
}


//-------------------------------------------------
//  gfxset_draw_item - draw a single item into
//  the view
//-------------------------------------------------

static void gfxset_draw_item(running_machine &machine, gfx_element &gfx, int index, bitmap_rgb32 &bitmap, int dstx, int dsty, int color, int rotate)
{
	int width = (rotate & ORIENTATION_SWAP_XY) ? gfx.height() : gfx.width();
	int height = (rotate & ORIENTATION_SWAP_XY) ? gfx.width() : gfx.height();
	const rgb_t *palette = gfx.palette().palette()->entry_list_raw() + gfx.colorbase() + color * gfx.granularity();
	int x, y;

	// loop over rows in the cell
	for (y = 0; y < height; y++)
	{
		UINT32 *dest = &bitmap.pix32(dsty + y, dstx);
		const UINT8 *src = gfx.get_data(index);

		// loop over columns in the cell
		for (x = 0; x < width; x++)
		{
			int effx = x, effy = y;
			const UINT8 *s;

			// compute effective x,y values after rotation
			if (!(rotate & ORIENTATION_SWAP_XY))
			{
				if (rotate & ORIENTATION_FLIP_X)
					effx = gfx.width() - 1 - effx;
				if (rotate & ORIENTATION_FLIP_Y)
					effy = gfx.height() - 1 - effy;
			}
			else
			{
				int temp;
				if (rotate & ORIENTATION_FLIP_X)
					effx = gfx.height() - 1 - effx;
				if (rotate & ORIENTATION_FLIP_Y)
					effy = gfx.width() - 1 - effy;
				temp = effx; effx = effy; effy = temp;
			}

			// get a pointer to the start of this source row
			s = src + effy * gfx.rowbytes();

			// extract the pixel
			*dest++ = 0xff000000 | palette[s[effx]];
		}
	}
}



/***************************************************************************
    TILEMAP VIEWER
***************************************************************************/

//-------------------------------------------------
//  tilemap_handler - handler for the tilemap
//  viewer
//-------------------------------------------------

static void tilemap_handler(running_machine &machine, render_container *container, ui_gfx_state &state)
{
	render_font *ui_font = machine.ui().get_font();
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

	// get the size of the tilemap itself
	tilemap_t *tilemap = machine.tilemap().find(state.tilemap.which);
	mapwidth = tilemap->width();
	mapheight = tilemap->height();
	if (state.tilemap.rotate & ORIENTATION_SWAP_XY)
		{ UINT32 temp = mapwidth; mapwidth = mapheight; mapheight = temp; }

	// add a half character padding for the box
	chheight = machine.ui().get_line_height();
	chwidth = ui_font->char_width(chheight, machine.render().ui_aspect(), '0');
	boxbounds.x0 = 0.0f + 0.5f * chwidth;
	boxbounds.x1 = 1.0f - 0.5f * chwidth;
	boxbounds.y0 = 0.0f + 0.5f * chheight;
	boxbounds.y1 = 1.0f - 0.5f * chheight;

	// the tilemap box bounds starts a half character in from the box
	mapboxbounds = boxbounds;
	mapboxbounds.x0 += 0.5f * chwidth;
	mapboxbounds.x1 -= 0.5f * chwidth;
	mapboxbounds.y0 += 0.5f * chheight;
	mapboxbounds.y1 -= 0.5f * chheight;

	// add space on the top for a title and a half line of padding
	mapboxbounds.y0 += 1.5f * chheight;

	// convert back to pixels
	mapboxwidth = (mapboxbounds.x1 - mapboxbounds.x0) * (float)targwidth;
	mapboxheight = (mapboxbounds.y1 - mapboxbounds.y0) * (float)targheight;

	// determine the maximum integral scaling factor
	pixelscale = state.tilemap.zoom;
	if (pixelscale == 0)
	{
		for (maxxscale = 1; mapwidth * (maxxscale + 1) < mapboxwidth; maxxscale++) { }
		for (maxyscale = 1; mapheight * (maxyscale + 1) < mapboxheight; maxyscale++) { }
		pixelscale = MIN(maxxscale, maxyscale);
	}

	// recompute the final box size
	mapboxwidth = MIN(mapboxwidth, mapwidth * pixelscale);
	mapboxheight = MIN(mapboxheight, mapheight * pixelscale);

	// recompute the bounds, centered within the existing bounds
	mapboxbounds.x0 += 0.5f * ((mapboxbounds.x1 - mapboxbounds.x0) - (float)mapboxwidth / (float)targwidth);
	mapboxbounds.x1 = mapboxbounds.x0 + (float)mapboxwidth / (float)targwidth;
	mapboxbounds.y0 += 0.5f * ((mapboxbounds.y1 - mapboxbounds.y0) - (float)mapboxheight / (float)targheight);
	mapboxbounds.y1 = mapboxbounds.y0 + (float)mapboxheight / (float)targheight;

	// now recompute the outer box against this new info
	boxbounds.x0 = mapboxbounds.x0 - 0.5f * chwidth;
	boxbounds.x1 = mapboxbounds.x1 + 0.5f * chwidth;
	boxbounds.y0 = mapboxbounds.y0 - 2.0f * chheight;
	boxbounds.y1 = mapboxbounds.y1 + 0.5f * chheight;

	// figure out the title and expand the outer box to fit
	sprintf(title, "TILEMAP %d/%d %dx%d OFFS %d,%d", state.tilemap.which, machine.tilemap().count() - 1, mapwidth, mapheight, state.tilemap.xoffs, state.tilemap.yoffs);
	titlewidth = ui_font->string_width(chheight, machine.render().ui_aspect(), title);
	if (boxbounds.x1 - boxbounds.x0 < titlewidth + chwidth)
	{
		boxbounds.x0 = 0.5f - 0.5f * (titlewidth + chwidth);
		boxbounds.x1 = boxbounds.x0 + titlewidth + chwidth;
	}

	// go ahead and draw the outer box now
	machine.ui().draw_outlined_box(container, boxbounds.x0, boxbounds.y0, boxbounds.x1, boxbounds.y1, UI_GFXVIEWER_BG_COLOR);

	// draw the title
	x0 = 0.5f - 0.5f * titlewidth;
	y0 = boxbounds.y0 + 0.5f * chheight;
	for (x = 0; title[x] != 0; x++)
	{
		container->add_char(x0, y0, chheight, machine.render().ui_aspect(), ARGB_WHITE, *ui_font, title[x]);
		x0 += ui_font->char_width(chheight, machine.render().ui_aspect(), title[x]);
	}

	// update the bitmap
	tilemap_update_bitmap(machine, state, mapboxwidth / pixelscale, mapboxheight / pixelscale);

	// add the final quad
	container->add_quad(mapboxbounds.x0, mapboxbounds.y0,
						mapboxbounds.x1, mapboxbounds.y1,
						ARGB_WHITE, state.texture,
						PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(state.tilemap.rotate));

	// handle keyboard input
	tilemap_handle_keys(machine, state, mapboxwidth, mapboxheight);
}


//-------------------------------------------------
//  tilemap_handle_keys - handle keys for the
//  tilemap view
//-------------------------------------------------

static void tilemap_handle_keys(running_machine &machine, ui_gfx_state &state, int viswidth, int visheight)
{
	UINT32 mapwidth, mapheight;
	int step;

	// handle tilemap selection (open bracket,close bracket)
	if (machine.ui_input().pressed(IPT_UI_PREV_GROUP) && state.tilemap.which > 0)
	{ state.tilemap.which--; state.bitmap_dirty = true; }
	if (machine.ui_input().pressed(IPT_UI_NEXT_GROUP) && state.tilemap.which < machine.tilemap().count() - 1)
	{ state.tilemap.which++; state.bitmap_dirty = true; }

	// cache some info in locals
	tilemap_t *tilemap = machine.tilemap().find(state.tilemap.which);
	mapwidth = tilemap->width();
	mapheight = tilemap->height();

	// handle zoom (minus,plus)
	if (machine.ui_input().pressed(IPT_UI_ZOOM_OUT) && state.tilemap.zoom > 0)
	{
		state.tilemap.zoom--;
		state.bitmap_dirty = true;
		if (state.tilemap.zoom != 0)
			machine.popmessage("Zoom = %d", state.tilemap.zoom);
		else
			machine.popmessage("Zoom Auto");
	}
	if (machine.ui_input().pressed(IPT_UI_ZOOM_IN) && state.tilemap.zoom < 8)
	{
		state.tilemap.zoom++;
		state.bitmap_dirty = true;
		machine.popmessage("Zoom = %d", state.tilemap.zoom);
	}

	// handle rotation (R)
	if (machine.ui_input().pressed(IPT_UI_ROTATE))
	{
		state.tilemap.rotate = orientation_add(ROT90, state.tilemap.rotate);
		state.bitmap_dirty = true;
	}

	// handle navigation (up,down,left,right)
	step = 8;
	if (machine.input().code_pressed(KEYCODE_LSHIFT)) step = 1;
	if (machine.input().code_pressed(KEYCODE_LCONTROL)) step = 64;
	if (machine.ui_input().pressed_repeat(IPT_UI_UP, 4))
	{ state.tilemap.yoffs -= step; state.bitmap_dirty = true; }
	if (machine.ui_input().pressed_repeat(IPT_UI_DOWN, 4))
	{ state.tilemap.yoffs += step; state.bitmap_dirty = true; }
	if (machine.ui_input().pressed_repeat(IPT_UI_LEFT, 6))
	{ state.tilemap.xoffs -= step; state.bitmap_dirty = true; }
	if (machine.ui_input().pressed_repeat(IPT_UI_RIGHT, 6))
	{ state.tilemap.xoffs += step; state.bitmap_dirty = true; }

	// clamp within range
	while (state.tilemap.xoffs < 0)
		state.tilemap.xoffs += mapwidth;
	while (state.tilemap.xoffs >= mapwidth)
		state.tilemap.xoffs -= mapwidth;
	while (state.tilemap.yoffs < 0)
		state.tilemap.yoffs += mapheight;
	while (state.tilemap.yoffs >= mapheight)
		state.tilemap.yoffs -= mapheight;
}


//-------------------------------------------------
//  tilemap_update_bitmap - update the bitmap
//  for the tilemap view
//-------------------------------------------------

static void tilemap_update_bitmap(running_machine &machine, ui_gfx_state &state, int width, int height)
{
	// swap the coordinates back if they were talking about a rotated surface
	if (state.tilemap.rotate & ORIENTATION_SWAP_XY)
		{ UINT32 temp = width; width = height; height = temp; }

	// realloc the bitmap if it is too small
	if (state.bitmap == nullptr || state.texture == nullptr || state.bitmap->width() != width || state.bitmap->height() != height)
	{
		// free the old stuff
		machine.render().texture_free(state.texture);
		global_free(state.bitmap);

		// allocate new stuff
		state.bitmap = global_alloc(bitmap_rgb32(width, height));
		state.texture = machine.render().texture_alloc();
		state.texture->set_bitmap(*state.bitmap, state.bitmap->cliprect(), TEXFORMAT_RGB32);

		// force a redraw
		state.bitmap_dirty = true;
	}

	// handle the redraw
	if (state.bitmap_dirty)
	{
		tilemap_t *tilemap = machine.tilemap().find(state.tilemap.which);
		tilemap->draw_debug(*machine.first_screen(), *state.bitmap, state.tilemap.xoffs, state.tilemap.yoffs);

		// reset the texture to force an update
		state.texture->set_bitmap(*state.bitmap, state.bitmap->cliprect(), TEXFORMAT_RGB32);
		state.bitmap_dirty = false;
	}
}

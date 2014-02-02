/***************************************************************************

    stackable.c

    Stackable UI primitives

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "stackable.h"
#include "uiinput.h"


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

ui_stackable *ui_stackable::menu_stack;
ui_stackable *ui_stackable::menu_free;
bitmap_rgb32 *ui_stackable::hilight_bitmap;
render_texture *ui_stackable::hilight_texture;
render_texture *ui_stackable::arrow_texture;


/***************************************************************************
    STATIC METHODS
***************************************************************************/

//-------------------------------------------------
//  init - initialize the menu system
//-------------------------------------------------

void ui_stackable::init(running_machine &machine)
{
	int x;

	// initialize the menu stack
	stack_reset(machine);

	// create a texture for hilighting items
	hilight_bitmap = auto_bitmap_rgb32_alloc(machine, 256, 1);
	for (x = 0; x < 256; x++)
	{
		int alpha = 0xff;
		if (x < 25) alpha = 0xff * x / 25;
		if (x > 256 - 25) alpha = 0xff * (255 - x) / 25;
		hilight_bitmap->pix32(0, x) = MAKE_ARGB(alpha,0xff,0xff,0xff);
	}
	hilight_texture = machine.render().texture_alloc();
	hilight_texture->set_bitmap(*hilight_bitmap, hilight_bitmap->cliprect(), TEXFORMAT_ARGB32);

	// create a texture for arrow icons
	arrow_texture = machine.render().texture_alloc(render_triangle);

	// add an exit callback to free memory
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(ui_stackable::exit), &machine));
}


//-------------------------------------------------
//  exit - clean up after ourselves
//-------------------------------------------------

void ui_stackable::exit(running_machine &machine)
{
	// free menus
	ui_menu::stack_reset(machine);
	ui_stackable::clear_free_list(machine);

	// free textures
	machine.render().texture_free(hilight_texture);
	machine.render().texture_free(arrow_texture);
}


//-------------------------------------------------
//  stack_reset - reset the menu stack
//-------------------------------------------------

void ui_stackable::stack_reset(running_machine &machine)
{
	while (menu_stack != NULL)
		ui_menu::stack_pop(machine);
}


//-------------------------------------------------
//  stack_push - push a new menu onto the stack
//-------------------------------------------------

void ui_stackable::stack_push(ui_stackable *menu)
{
	menu->parent = menu_stack;
	menu_stack = menu;
	menu->reset();
	ui_input_reset(menu->machine());
}


//-------------------------------------------------
//  stack_pop - pop a menu from the stack
//-------------------------------------------------

void ui_stackable::stack_pop(running_machine &machine)
{
	if (menu_stack != NULL)
	{
		ui_stackable *menu = menu_stack;
		menu_stack = menu->parent;
		menu->parent = menu_free;
		menu_free = menu;
		ui_input_reset(machine);
	}
}


//-------------------------------------------------
//  stack_has_special_main_menu - check in the
//	special main menu is in the stack
//-------------------------------------------------

bool ui_stackable::stack_has_special_main_menu()
{
	ui_stackable *menu;

	for (menu = menu_stack; menu != NULL; menu = menu->parent)
		if (menu->is_special_main_menu())
			return true;

	return false;
}


//-------------------------------------------------
//  render_triangle - render a triangle that
//  is used for up/down arrows and left/right
//  indicators
//-------------------------------------------------

void ui_stackable::render_triangle(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param)
{
	int halfwidth = dest.width() / 2;
	int height = dest.height();
	int x, y;

	// start with all-transparent
	dest.fill(MAKE_ARGB(0x00,0x00,0x00,0x00));

	// render from the tip to the bottom
	for (y = 0; y < height; y++)
	{
		int linewidth = (y * (halfwidth - 1) + (height / 2)) * 255 * 2 / height;
		UINT32 *target = &dest.pix32(y, halfwidth);

		// don't antialias if height < 12
		if (dest.height() < 12)
		{
			int pixels = (linewidth + 254) / 255;
			if (pixels % 2 == 0) pixels++;
			linewidth = pixels * 255;
		}

		// loop while we still have data to generate
		for (x = 0; linewidth > 0; x++)
		{
			int dalpha;

			// first column we only consume one pixel
			if (x == 0)
			{
				dalpha = MIN(0xff, linewidth);
				target[x] = MAKE_ARGB(dalpha,0xff,0xff,0xff);
			}

			// remaining columns consume two pixels, one on each side
			else
			{
				dalpha = MIN(0x1fe, linewidth);
				target[x] = target[-x] = MAKE_ARGB(dalpha/2,0xff,0xff,0xff);
			}

			// account for the weight we consumed
			linewidth -= dalpha;
		}
	}
}


/***************************************************************************
    MENU STACK MANAGEMENT
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_stackable::ui_stackable(running_machine &machine, render_container *_container)
	: m_machine(machine)
{
	container = _container;
	special_main_menu = false;
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_stackable::~ui_stackable()
{
}


//-------------------------------------------------
//  is_special_main_menu - returns whether the
//  menu has special needs
//-------------------------------------------------

bool ui_stackable::is_special_main_menu() const
{
	return special_main_menu;
}


//-------------------------------------------------
//  set_special_main_menu - set whether the
//  menu has special needs
//-------------------------------------------------

void ui_stackable::set_special_main_menu(bool special)
{
	special_main_menu = special;
}


//-------------------------------------------------
//  clear_free_list - clear out anything
//  accumulated in the free list
//-------------------------------------------------

void ui_stackable::clear_free_list(running_machine &machine)
{
	while (menu_free != NULL)
	{
		ui_stackable *menu = menu_free;
		menu_free = menu->parent;
		auto_free(machine, menu);
	}
}


//-------------------------------------------------
//  get_line_height
//-------------------------------------------------

float ui_stackable::get_line_height()
{
	return ui_get_line_height(machine());
}


//-------------------------------------------------
//  get_char_width
//-------------------------------------------------

float ui_stackable::get_char_width(unicode_char ch)
{
	return ui_get_char_width(machine(), ch);
}


//-------------------------------------------------
//  get_string_width
//-------------------------------------------------

float ui_stackable::get_string_width(const char *s)
{
	return ui_get_string_width(machine(), s);
}


//-------------------------------------------------
//  draw_outlined_box
//-------------------------------------------------

void ui_stackable::draw_outlined_box(float x0, float y0, float x1, float y1, rgb_t backcolor)
{
	ui_draw_outlined_box(container, x0, y0, x1, y1, backcolor);
}


//-------------------------------------------------
//  draw_text
//-------------------------------------------------

void ui_stackable::draw_text(const char *origs, float x, float y, rgb_t fgcolor, rgb_t bgcolor)
{
	draw_text(origs, x, y, 1.0f - x, JUSTIFY_LEFT, WRAP_WORD, DRAW_NORMAL, fgcolor, bgcolor);
}


//-------------------------------------------------
//  draw_text
//-------------------------------------------------

void ui_stackable::draw_text(const char *origs, float x, float y, float origwrapwidth, int justify, int wrap, int draw, rgb_t fgcolor, rgb_t bgcolor, float *totalwidth, float *totalheight)
{
	ui_draw_text_full(container, origs, x, y, origwrapwidth, justify, wrap, draw, fgcolor, bgcolor, totalwidth, totalheight);
}


//-------------------------------------------------
//  draw_text_box
//-------------------------------------------------

void ui_stackable::draw_text_box(const char *text, int justify, float xpos, float ypos, rgb_t backcolor)
{
	ui_draw_text_box(container, text, justify, xpos, ypos, backcolor);
}


//-------------------------------------------------
//  highlight
//-------------------------------------------------

void ui_stackable::highlight(float x0, float y0, float x1, float y1, rgb_t bgcolor)
{
	container->add_quad(x0, y0, x1, y1, bgcolor, hilight_texture,
						PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
}


//-------------------------------------------------
//  input_pop_event
//-------------------------------------------------

bool ui_stackable::input_pop_event(ui_event &event)
{
	return ui_input_pop_event(machine(), &event);
}


//-------------------------------------------------
//  input_pressed
//-------------------------------------------------

bool ui_stackable::input_pressed(int key, int repeat)
{
	return ui_input_pressed_repeat(machine(), key, repeat);
}


//-------------------------------------------------
//  find_mouse
//-------------------------------------------------

bool ui_stackable::find_mouse(float &mouse_x, float &mouse_y)
{
	bool mouse_button;
	return find_mouse(mouse_x, mouse_y, mouse_button);
}


//-------------------------------------------------
//  find_mouse
//-------------------------------------------------

bool ui_stackable::find_mouse(float &mouse_x, float &mouse_y, bool &mouse_button)
{
	bool result = false;
	mouse_x = -1;
	mouse_y = -1;

	INT32 mouse_target_x, mouse_target_y;
	render_target *mouse_target = ui_input_find_mouse(machine(), &mouse_target_x, &mouse_target_y, &mouse_button);
	if (mouse_target != NULL)
	{
		if (mouse_target->map_point_container(mouse_target_x, mouse_target_y, *container, mouse_x, mouse_y))
			result = true;
	}

	return result;
}

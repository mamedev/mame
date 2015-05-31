/***************************************************************************

	mewui/menu.c

	Internal MEWUI menus for the user interface.

***************************************************************************/
#include "mewui/defimg.h"
#include "mewui/starimg.h"
#include "mewui/utils.h"
#include "mewui/optsmenu.h"
#include "mewui/datfile.h"
#include "rendfont.h"
#include "mewui/custmenu.h"
#include "mewui/icorender.h"

/***************************************************************************
	GLOBAL VARIABLES
***************************************************************************/

render_texture *ui_menu::snapx_texture;
render_texture *ui_menu::hilight_main_texture;
render_texture *ui_menu::bgrnd_texture;
render_texture *ui_menu::star_texture;
render_texture *ui_menu::icons_texture[MAX_ICONS_RENDER];
bitmap_argb32 *ui_menu::snapx_bitmap;
bitmap_argb32 *ui_menu::no_avail_bitmap;
bitmap_argb32 *ui_menu::star_bitmap;
bitmap_argb32 *ui_menu::bgrnd_bitmap;
bitmap_argb32 *ui_menu::icons_bitmap[MAX_ICONS_RENDER];
bitmap_rgb32 *ui_menu::hilight_main_bitmap;

/***************************************************************************
	CONSTANTS
***************************************************************************/
struct ui_arts_info
{
	const char *title, *path, *addpath;
};

static const ui_arts_info arts_info[] =
{
	{ "Snapshots",		 OPTION_SNAPSHOT_DIRECTORY,  "snap" },
	{ "Cabinets",		 OPTION_CABINETS_DIRECTORY,  "cabinets;cabdevs" },
	{ "Control Panels",	 OPTION_CPANELS_DIRECTORY,   "cpanel" },
	{ "PCBs",			 OPTION_PCBS_DIRECTORY,	     "pcb" },
	{ "Flyers",			 OPTION_FLYERS_DIRECTORY,	 "flyers" },
	{ "Titles",			 OPTION_TITLES_DIRECTORY,	 "titles" },
	{ "Artwork Preview", OPTION_ARTPREV_DIRECTORY,   "artwork preview" },
	{ "Bosses",			 OPTION_BOSSES_DIRECTORY,	 "bosses" },
	{ "Logos",			 OPTION_LOGOS_DIRECTORY,	 "logo" },
	{ "Versus",			 OPTION_VERSUS_DIRECTORY,	 "versus" },
	{ "Game Over",		 OPTION_GAMEOVER_DIRECTORY,  "gameover" },
	{ "HowTo",			 OPTION_HOWTO_DIRECTORY,	 "howto" },
	{ "Scores",			 OPTION_SCORES_DIRECTORY,	 "scores" },
	{ "Select",			 OPTION_SELECT_DIRECTORY,	 "select" },
	{ "Marquees",		 OPTION_MARQUEES_DIRECTORY,  "marquees" },
	{ NULL }
};

static const char *dats_info[] = { "General Info", "History", "Mameinfo", "Sysinfo", "Messinfo", "Command", "Mamescore" };

//-------------------------------------------------
//  init - initialize the mewui menu system
//-------------------------------------------------

void ui_menu::init_mewui(running_machine &machine)
{
	// create a texture for hilighting items in main menu
	hilight_main_bitmap = auto_bitmap_rgb32_alloc(machine, 1, 26);
	for (int y = 0; y < 26; y++)
		hilight_main_bitmap->pix32(y, 0) = rgb_t(0xff, 0, 169 - (y * 5), 255 - (y * 5));

	hilight_main_texture = machine.render().texture_alloc();
	hilight_main_texture->set_bitmap(*hilight_main_bitmap, hilight_main_bitmap->cliprect(), TEXFORMAT_ARGB32);

	// create a texture for snapshot
	snapx_bitmap = auto_alloc(machine, bitmap_argb32);
	snapx_texture = machine.render().texture_alloc(render_texture::hq_scale);

	// allocates and sets the default "no available" image
	no_avail_bitmap = auto_alloc(machine, bitmap_argb32(256, 256));
	UINT32 *dst = &no_avail_bitmap->pix32(0);
	memcpy(dst, no_avail_bmp, 256*256*sizeof(UINT32));

	// allocates and sets the favorites star image
	star_bitmap = auto_alloc(machine, bitmap_argb32(32, 32));
	dst = &star_bitmap->pix32(0);
	memcpy(dst, favorite_star_bmp, 32*32*sizeof(UINT32));
	star_texture = machine.render().texture_alloc();
	star_texture->set_bitmap(*star_bitmap, star_bitmap->cliprect(), TEXFORMAT_ARGB32);

	// allocates icons bitmap and texture
	for (int i = 0; i < MAX_ICONS_RENDER; i++)
	{
		icons_bitmap[i] = auto_alloc(machine, bitmap_argb32(32, 32));
		//icons_texture[i] = machine.render().texture_alloc(render_texture::hq_scale);
		icons_texture[i] = machine.render().texture_alloc();
	}

	// create a texture for main menu background
	bgrnd_bitmap = auto_alloc(machine, bitmap_argb32);
	bgrnd_texture = machine.render().texture_alloc(render_texture::hq_scale);

	if (machine.options().use_background_image() && (machine.options().system() == &GAME_NAME(___empty) || machine.options().system() == NULL))
	{
		emu_file backgroundfile(".", OPEN_FLAG_READ);
		render_load_jpeg(*bgrnd_bitmap, backgroundfile, NULL, "background.jpg");

		if (!bgrnd_bitmap->valid())
			render_load_png(*bgrnd_bitmap, backgroundfile, NULL, "background.png");

		if (bgrnd_bitmap->valid())
			bgrnd_texture->set_bitmap(*bgrnd_bitmap, bgrnd_bitmap->cliprect(), TEXFORMAT_ARGB32);
		else
			bgrnd_bitmap->reset();
	}
	else
		bgrnd_bitmap->reset();
}


//-------------------------------------------------
//  draw main menu
//-------------------------------------------------

void ui_menu::draw_select_game()
{
	float line_height = machine().ui().get_line_height();
	float ud_arrow_width = line_height * machine().render().ui_aspect();
	float gutter_width = 0.4f * line_height * machine().render().ui_aspect() * 1.3f;
	mouse_x = -1, mouse_y = -1;
	float right_panel_size = 0.3f;
	float visible_width = 1.0f - 4.0f * UI_BOX_LR_BORDER;
	float primary_left = (1.0f - visible_width) * 0.5f;
	float primary_width = visible_width;
	bool is_swlist = ((item[0].flags & MENU_FLAG_MEWUI_SWLIST) != 0);
	bool is_favorites = ((item[0].flags & MENU_FLAG_MEWUI_FAVORITE) != 0);

	// draw background image if available
	if (machine().options().use_background_image() && bgrnd_bitmap->valid())
		container->add_quad(0.0f, 0.0f, 1.0f, 1.0f, ARGB_WHITE, bgrnd_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	hover = item.size() + 1;
	visible_items = (is_swlist) ? item.size() - 2 : item.size() - 4;
	float extra_height = (is_swlist) ? 2.0f * line_height : 4.0f * line_height;
	float visible_extra_menu_height = customtop + custombottom + extra_height;

	// locate mouse
	mouse_hit = FALSE;
	mouse_button = FALSE;
	mouse_target = ui_input_find_mouse(machine(), &mouse_target_x, &mouse_target_y, &mouse_button);
	if (mouse_target != NULL)
		if (mouse_target->map_point_container(mouse_target_x, mouse_target_y, *container, mouse_x, mouse_y))
			mouse_hit = TRUE;

	// account for extra space at the top and bottom
	float visible_main_menu_height = 1.0f - 2.0f * UI_BOX_TB_BORDER - visible_extra_menu_height;
	visible_lines = floor(visible_main_menu_height / line_height);
	visible_main_menu_height = (float)(visible_lines * line_height);

	if (!is_swlist)
		mewui_globals::visible_main_lines = visible_lines;
	else
		mewui_globals::visible_sw_lines = visible_lines;

	// compute top/left of inner menu area by centering
	float visible_left = primary_left;
	float visible_top = (1.0f - (visible_main_menu_height + visible_extra_menu_height)) * 0.5f;

	// if the menu is at the bottom of the extra, adjust
	visible_top += customtop;

	// compute left box size
	float x1 = visible_left - UI_BOX_LR_BORDER;
	float y1 = visible_top - UI_BOX_TB_BORDER;
	float x2 = visible_left + 2 * UI_BOX_LR_BORDER;
	float y2 = visible_top + visible_main_menu_height + UI_BOX_TB_BORDER + extra_height;

	// add left box
	visible_left = draw_left_box(x1, y1, x2, y2, is_swlist);

	visible_width -= right_panel_size + visible_left - 2 * UI_BOX_LR_BORDER;

	// compute and add main a box
	x1 = visible_left - UI_BOX_LR_BORDER;
	x2 = visible_left + visible_width + UI_BOX_LR_BORDER;
	float line = visible_top + (float)(visible_lines * line_height);

	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, rgb_t(0xEF, 0x12, 0x47, 0x7B));

	if (visible_items < visible_lines)
		visible_lines = visible_items;
	if (top_line < 0 || selected == 0)
		top_line = 0;
	if (selected < visible_items && top_line + visible_lines >= visible_items)
		top_line = visible_items - visible_lines;

	// determine effective positions taking into account the hilighting arrows
	float effective_width = visible_width - 2.0f * gutter_width;
	float effective_left = visible_left + gutter_width;

	int n_loop = (visible_items >= visible_lines) ? visible_lines : visible_items;

	for (int linenum = 0; linenum < n_loop; linenum++)
	{
		float line_y = visible_top + (float)linenum * line_height;
		int itemnum = top_line + linenum;
		const ui_menu_item &pitem = item[itemnum];
		const char *itemtext = pitem.text;
		rgb_t fgcolor = UI_TEXT_COLOR;
		rgb_t bgcolor = UI_TEXT_BG_COLOR;
		rgb_t fgcolor3 = UI_CLONE_COLOR;
		float line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
		float line_y0 = line_y;
		float line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
		float line_y1 = line_y + line_height;

		// set the hover if this is our item
		if (mouse_hit && line_x0 <= mouse_x && line_x1 > mouse_x && line_y0 <= mouse_y && line_y1 > mouse_y && pitem.is_selectable())
			hover = itemnum;

		// if we're selected, draw with a different background
		if (itemnum == selected)
		{
			fgcolor = rgb_t(0xff, 0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff, 0xff);
			fgcolor3 = rgb_t(0xff, 0xcc, 0xcc, 0x00);
		}
		// else if the mouse is over this item, draw with a different background
		else if (itemnum == hover)
		{
			fgcolor = UI_MOUSEOVER_COLOR;
			bgcolor = UI_MOUSEOVER_BG_COLOR;
			fgcolor3 = UI_MOUSEOVER_COLOR;
		}

		// if we have some background hilighting to do, add a quad behind everything else
		if (bgcolor != UI_TEXT_BG_COLOR)
			machine().ui().draw_textured_box(container, line_x0 + 0.01f, line_y0, line_x1 - 0.01f, line_y1,
											 bgcolor, rgb_t(255, 43, 43, 43), hilight_main_texture,
											 PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));

		// if we're on the top line, display the up arrow
		if (linenum == 0 && top_line != 0)
		{
			draw_arrow(container, 0.5f * (x1 + x2) - 0.5f * ud_arrow_width, line_y + 0.25f * line_height,
					   0.5f * (x1 + x2) + 0.5f * ud_arrow_width, line_y + 0.75f * line_height, fgcolor, ROT0);

			if (hover == itemnum)
				hover = -2;
		}
		// if we're on the bottom line, display the down arrow
		else if (linenum == visible_lines - 1 && itemnum != visible_items - 1)
		{
			draw_arrow(container, 0.5f * (x1 + x2) - 0.5f * ud_arrow_width, line_y + 0.25f * line_height,
					   0.5f * (x1 + x2) + 0.5f * ud_arrow_width, line_y + 0.75f * line_height, fgcolor, ROT0 ^ ORIENTATION_FLIP_Y);

			if (hover == itemnum)
				hover = -1;
		}
		// if we're just a divider, draw a line
		else if (strcmp(itemtext, MENU_SEPARATOR_ITEM) == 0)
			container->add_line(visible_left, line_y + 0.5f * line_height, visible_left + visible_width, line_y + 0.5f * line_height,
								UI_LINE_WIDTH, UI_TEXT_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		// draw the item centered
		else if (pitem.subtext == NULL)
		{
			int item_invert = pitem.flags & MENU_FLAG_INVERT;
			float space = 0.0f;

			if (!is_swlist)
			{
				if (is_favorites)
				{
					ui_software_info *soft = (ui_software_info *)item[itemnum].ref;
					if (soft->startempty == 1)
						draw_icon(container, linenum, (void *)soft->driver, effective_left, line_y);
				}
				else
					draw_icon(container, linenum, item[itemnum].ref, effective_left, line_y);

				space = machine().ui().get_line_height() * container->manager().ui_aspect() * 1.5f;
			}
			machine().ui().draw_text_full(container, itemtext, effective_left + space, line_y, effective_width - space,
										  JUSTIFY_LEFT, WRAP_TRUNCATE, DRAW_NORMAL, item_invert ? fgcolor3 : fgcolor,
										  bgcolor, NULL, NULL);
		}
		else
		{
			int item_invert = pitem.flags & MENU_FLAG_INVERT;
			const char *subitem_text = pitem.subtext;
			float item_width, subitem_width;

			// compute right space for subitem
			machine().ui().draw_text_full(container, subitem_text, effective_left, line_y, machine().ui().get_string_width(pitem.subtext),
										  JUSTIFY_RIGHT, WRAP_NEVER, DRAW_NONE, item_invert ? fgcolor3 : fgcolor, bgcolor, &subitem_width, NULL);
			subitem_width += gutter_width;

			// draw the item left-justified
			machine().ui().draw_text_full(container, itemtext, effective_left, line_y, effective_width - subitem_width,
										  JUSTIFY_LEFT, WRAP_TRUNCATE, DRAW_NORMAL, item_invert ? fgcolor3 : fgcolor, bgcolor, &item_width, NULL);

			// draw the subitem right-justified
			machine().ui().draw_text_full(container, subitem_text, effective_left + item_width, line_y, effective_width - item_width,
										  JUSTIFY_RIGHT, WRAP_NEVER, DRAW_NORMAL, item_invert ? fgcolor3 : fgcolor, bgcolor, NULL, NULL);
		}
	}

	for (int count = visible_items; count < item.size(); count++)
	{
		const ui_menu_item &pitem = item[count];
		const char *itemtext = pitem.text;
		float line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
		float line_y0 = line;
		float line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
		float line_y1 = line + line_height;
		rgb_t fgcolor = UI_TEXT_COLOR;
		rgb_t bgcolor = UI_TEXT_BG_COLOR;

		if (mouse_hit && line_x0 <= mouse_x && line_x1 > mouse_x && line_y0 <= mouse_y && line_y1 > mouse_y && pitem.is_selectable())
			hover = count;

		// if we're selected, draw with a different background
		if (count == selected)
		{
			fgcolor = rgb_t(0xff, 0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff, 0xff);
		}
		// else if the mouse is over this item, draw with a different background
		else if (count == hover)
		{
			fgcolor = UI_MOUSEOVER_COLOR;
			bgcolor = UI_MOUSEOVER_BG_COLOR;
		}

		// if we have some background hilighting to do, add a quad behind everything else
		if (bgcolor != UI_TEXT_BG_COLOR)
			machine().ui().draw_textured_box(container, line_x0 + 0.01f, line_y0, line_x1 - 0.01f, line_y1, bgcolor, rgb_t(255, 43, 43, 43),
											 hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));

		if (strcmp(itemtext, MENU_SEPARATOR_ITEM) == 0)
			container->add_line(visible_left, line + 0.5f * line_height, visible_left + visible_width, line + 0.5f * line_height,
								UI_LINE_WIDTH, UI_TEXT_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		else
			machine().ui().draw_text_full(container, itemtext, effective_left, line, effective_width,
										  JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, fgcolor, bgcolor, NULL, NULL);
		line += line_height;
	}

	x1 = x2;
	x2 += right_panel_size;

	// draw right box
	float origy1 = draw_right_box_title(x1, y1, x2, y2);

	if (mewui_globals::rpanel_infos == RP_IMAGES)
		arts_render((selected >= 0 && selected < item.size()) ? item[selected].ref : NULL, x1, origy1, x2, y2, (is_swlist || is_favorites));
	else
		infos_render((selected >= 0 && selected < item.size()) ? item[selected].ref : NULL, x1, origy1, x2, y2, (is_swlist || is_favorites));

	x1 = primary_left - UI_BOX_LR_BORDER;
	x2 = primary_left + primary_width + UI_BOX_LR_BORDER;

	// if there is something special to add, do it by calling the virtual method
	custom_render((selected >= 0 && selected < item.size()) ? item[selected].ref : NULL, customtop, custombottom, x1, y1, x2, y2);

	// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	visitems = visible_lines - (top_line != 0) - (top_line + visible_lines != visible_items);

	// reset redraw icon stage
	if (!is_swlist)
		mewui_globals::redraw_icon = false;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu::arts_render(void *selectedref, float origx1, float origy1, float origx2, float origy2, bool software)
{
	static ui_software_info *oldsoft = NULL;
	static const game_driver *olddriver = NULL;
	const game_driver *driver = NULL;
	ui_software_info *soft = NULL;

	if (software)
	{
		soft = ((FPTR)selectedref > 2) ? (ui_software_info *)selectedref : NULL;
		if (soft && soft->startempty == 1)
		{
			driver = soft->driver;
			oldsoft = NULL;
		}
		else
			olddriver = NULL;
	}
	else
	{
		driver = ((FPTR)selectedref > 2) ? (const game_driver *)selectedref : NULL;
		oldsoft = NULL;
	}

	if (driver)
	{
		float line_height = machine().ui().get_line_height();
		if (mewui_globals::default_image)
			((driver->flags & GAME_TYPE_ARCADE) == 0) ? mewui_globals::curimage_view = CABINETS_VIEW : mewui_globals::curimage_view = SNAPSHOT_VIEW;

		std::string searchstr;
		searchstr = arts_render_common(origx1, origy1, origx2, origy2);

		// loads the image if necessary
		if (driver != olddriver || !snapx_bitmap->valid() || mewui_globals::switch_image)
		{
			emu_file snapfile(searchstr.c_str(), OPEN_FLAG_READ);
			bitmap_argb32 *tmp_bitmap;
			tmp_bitmap = auto_alloc(machine(), bitmap_argb32);

			// try to load snapshot first from saved "0000.png" file
			std::string fullname(driver->name);
			render_load_png(*tmp_bitmap, snapfile, fullname.c_str(), "0000.png");

			if (!tmp_bitmap->valid())
				render_load_jpeg(*tmp_bitmap, snapfile, fullname.c_str(), "0000.jpg");

			// if fail, attemp to load from standard file
			if (!tmp_bitmap->valid())
			{
				fullname.assign(driver->name).append(".png");
				render_load_png(*tmp_bitmap, snapfile, NULL, fullname.c_str());

				if (!tmp_bitmap->valid())
				{
					fullname.assign(driver->name).append(".jpg");
					render_load_jpeg(*tmp_bitmap, snapfile, NULL, fullname.c_str());
				}
			}

			olddriver = driver;
			mewui_globals::switch_image = false;
			arts_render_images(tmp_bitmap, origx1, origy1, origx2, origy2, false);
			auto_free(machine(), tmp_bitmap);
		}

		// if the image is available, loaded and valid, display it
		if (snapx_bitmap->valid())
		{
			float x1 = origx1 + 0.01f;
			float x2 = origx2 - 0.01f;
			float y1 = origy1 + UI_BOX_TB_BORDER + line_height;
			float y2 = origy2 - UI_BOX_TB_BORDER - line_height;

			// apply texture
			container->add_quad( x1, y1, x2, y2, ARGB_WHITE, snapx_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
	}
	else if (soft)
	{
		float line_height = machine().ui().get_line_height();
		std::string fullname, pathname;

		if (mewui_globals::default_image)
			(soft->startempty == 0) ? mewui_globals::curimage_view = SNAPSHOT_VIEW : mewui_globals::curimage_view = CABINETS_VIEW;

		// arts title and searchpath
		std::string searchstr;
		searchstr = arts_render_common(origx1, origy1, origx2, origy2);

		// loads the image if necessary
		if (soft != oldsoft || !snapx_bitmap->valid() || mewui_globals::switch_image)
		{
			emu_file snapfile(searchstr.c_str(), OPEN_FLAG_READ);
			bitmap_argb32 *tmp_bitmap;
			tmp_bitmap = auto_alloc(machine(), bitmap_argb32);

			if (soft->startempty == 1)
			{
				// Load driver snapshot
				fullname.assign(soft->driver->name).append(".png");
				render_load_png(*tmp_bitmap, snapfile, NULL, fullname.c_str());

				if (!tmp_bitmap->valid())
				{
					fullname.assign(soft->driver->name).append(".jpg");
					render_load_jpeg(*tmp_bitmap, snapfile, NULL, fullname.c_str());
				}
			}
			else if (mewui_globals::curimage_view == TITLES_VIEW)
			{
				// First attempt from name list
				pathname.assign(soft->listname.c_str()).append("_titles");
				fullname.assign(soft->shortname.c_str()).append(".png");
				render_load_png(*tmp_bitmap, snapfile, pathname.c_str(), fullname.c_str());

				if (!tmp_bitmap->valid())
				{
					fullname.assign(soft->shortname.c_str()).append(".jpg");
					render_load_jpeg(*tmp_bitmap, snapfile, pathname.c_str(), fullname.c_str());
				}
			}
			else
			{
				// First attempt from name list
				pathname.assign(soft->listname.c_str());
				fullname.assign(soft->shortname.c_str()).append(".png");
				render_load_png(*tmp_bitmap, snapfile, pathname.c_str(), fullname.c_str());

				if (!tmp_bitmap->valid())
				{
					fullname.assign(soft->shortname.c_str()).append(".jpg");
					render_load_jpeg(*tmp_bitmap, snapfile, pathname.c_str(), fullname.c_str());
				}

				if (!tmp_bitmap->valid())
				{
					// Second attempt from driver name + part name
					pathname.assign(soft->driver->name).append(soft->part.c_str());
					fullname.assign(soft->shortname.c_str()).append(".png");
					render_load_png(*tmp_bitmap, snapfile, pathname.c_str(), fullname.c_str());

					if (!tmp_bitmap->valid())
					{
						fullname.assign(soft->shortname.c_str()).append(".jpg");
						render_load_jpeg(*tmp_bitmap, snapfile, pathname.c_str(), fullname.c_str());
					}
				}
			}

			oldsoft = soft;
			mewui_globals::switch_image = false;
			arts_render_images(tmp_bitmap, origx1, origy1, origx2, origy2, true);
			auto_free(machine(), tmp_bitmap);
		}

		// if the image is available, loaded and valid, display it
		if (snapx_bitmap->valid())
		{
			float x1 = origx1 + 0.01f;
			float x2 = origx2 - 0.01f;
			float y1 = origy1 + UI_BOX_TB_BORDER + line_height;
			float y2 = origy2 - UI_BOX_TB_BORDER - line_height;

			// apply texture
			container->add_quad(x1, y1, x2, y2, ARGB_WHITE, snapx_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
	}
}

//-------------------------------------------------
//  get title and search path for right panel
//-------------------------------------------------

void ui_menu::get_title_search(std::string &snaptext, std::string &searchstr)
{
	// get arts title text
	snaptext.assign(arts_info[mewui_globals::curimage_view].title);

	// get search path
	path_iterator path(machine().options().value(arts_info[mewui_globals::curimage_view].path));
	std::string curpath;
	searchstr.assign(machine().options().value(arts_info[mewui_globals::curimage_view].path));

	// iterate over path and add path for zipped formats
	while (path.next(curpath))
	{
		path_iterator path_iter(arts_info[mewui_globals::curimage_view].addpath);
		std::string c_path;
		while (path_iter.next(c_path))
			searchstr.append(";").append(curpath).append(PATH_SEPARATOR).append(c_path);
	}
}

//-------------------------------------------------
//  handle keys for main menu
//-------------------------------------------------

void ui_menu::handle_main_keys(UINT32 flags)
{
	int ignorepause = ui_menu::stack_has_special_main_menu();
	int ignoreright = FALSE;
	int ignoreleft = FALSE;

	// bail if no items
	if (item.size() == 0)
		return;

	// if we hit select, return TRUE or pop the stack, depending on the item
	if (exclusive_input_pressed(IPT_UI_SELECT, 0))
	{
		if (selected == item.size() - 1)
		{
			menu_event.iptkey = IPT_UI_CANCEL;
			ui_menu::stack_pop(machine());
		}
		return;
	}

	// hitting cancel also pops the stack
	if (exclusive_input_pressed(IPT_UI_CANCEL, 0))
	{
		if (!menu_has_search_active())
			ui_menu::stack_pop(machine());
//	  else if (!ui_error)
//		  ui_menu::stack_pop(machine()); TODO
		return;
	}

	// validate the current selection
	validate_selection(1);

	// swallow left/right keys if they are not appropriate
	ignoreleft = ((item[selected].flags & MENU_FLAG_LEFT_ARROW) == 0);
	ignoreright = ((item[selected].flags & MENU_FLAG_RIGHT_ARROW) == 0);

	// accept left/right keys as-is with repeat
	if (!ignoreleft && exclusive_input_pressed(IPT_UI_LEFT, (flags & UI_MENU_PROCESS_LR_REPEAT) ? 6 : 0))
	{
		// Swap the right panel
		if (machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(JOYCODE_BUTTON1_INDEXED(0)))
			menu_event.iptkey = IPT_UI_LEFT_PANEL;
		return;
	}

	if (!ignoreright && exclusive_input_pressed(IPT_UI_RIGHT, (flags & UI_MENU_PROCESS_LR_REPEAT) ? 6 : 0))
	{
		// Swap the right panel
		if (machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(JOYCODE_BUTTON1_INDEXED(0)))
			menu_event.iptkey = IPT_UI_RIGHT_PANEL;
		return;
	}

	// up backs up by one item
	if (exclusive_input_pressed(IPT_UI_UP, 6))
	{
		// Filter
		if (machine().input().code_pressed(KEYCODE_LALT) || machine().input().code_pressed(JOYCODE_BUTTON1_INDEXED(0)))
		{
			menu_event.iptkey = IPT_UI_UP_FILTER;
			return;
		}

		// Infos
		if (machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(JOYCODE_BUTTON2_INDEXED(0)))
		{
			menu_event.iptkey = IPT_UI_UP_PANEL;
			topline_datsview--;
			return;
		}

		if (selected == visible_items + 1 || selected == 0 || ui_error)
			return;

		selected--;

		if (selected == top_line && top_line != 0)
			top_line--;
	}

	// down advances by one item
	if (exclusive_input_pressed(IPT_UI_DOWN, 6))
	{
		// Filter
		if (machine().input().code_pressed(KEYCODE_LALT) || machine().input().code_pressed(JOYCODE_BUTTON1_INDEXED(0)))
		{
			menu_event.iptkey = IPT_UI_DOWN_FILTER;
			return;
		}

		// Infos
		if (machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(JOYCODE_BUTTON2_INDEXED(0)))
		{
			menu_event.iptkey = IPT_UI_DOWN_PANEL;
			topline_datsview++;
			return;
		}

		if (selected == item.size() - 1 || selected == visible_items - 1 || ui_error)
			return;

		selected++;

		if (selected == top_line + visitems + (top_line != 0))
			top_line++;
	}

	// page up backs up by visitems
	if (exclusive_input_pressed(IPT_UI_PAGE_UP, 6))
	{
		// Infos
		if (machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(JOYCODE_BUTTON2_INDEXED(0)))
		{
			menu_event.iptkey = IPT_UI_DOWN_PANEL;
			topline_datsview -= right_visible_lines - 1;
			return;
		}

		if (selected < visible_items && !ui_error)
		{
			selected -= visitems;

			if (selected < 0)
				selected = 0;

			top_line -= visitems - (top_line + visible_lines == visible_items);
		}
	}

	// page down advances by visitems
	if (exclusive_input_pressed(IPT_UI_PAGE_DOWN, 6))
	{
		// Infos
		if (machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(JOYCODE_BUTTON2_INDEXED(0)))
		{
			menu_event.iptkey = IPT_UI_DOWN_PANEL;
			topline_datsview += right_visible_lines - 1;
			return;
		}

		if (selected < visible_items && !ui_error)
		{
			selected += visible_lines - 2 + (selected == 0);

			if (selected >= visible_items)
				selected = visible_items - 1;

			top_line += visible_lines - 2;
		}
	}

	// home goes to the start
	if (exclusive_input_pressed(IPT_UI_HOME, 0))
	{
		// Infos
		if (machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(JOYCODE_BUTTON2_INDEXED(0)))
		{
			menu_event.iptkey = IPT_UI_DOWN_PANEL;
			topline_datsview = 0;
			return;
		}

		if (selected < visible_items && !ui_error)
		{
			selected = 0;
			top_line = 0;
		}
	}

	// end goes to the last
	if (exclusive_input_pressed(IPT_UI_END, 0))
	{
		// Infos
		if (machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(JOYCODE_BUTTON2_INDEXED(0)))
		{
			menu_event.iptkey = IPT_UI_DOWN_PANEL;
			topline_datsview = totallines;
			return;
		}

		if (selected < visible_items && !ui_error)
			selected = top_line = visible_items - 1;
	}

	// pause enables/disables pause
	if (!ui_error && !ignorepause && exclusive_input_pressed(IPT_UI_PAUSE, 0))
	{
		if (machine().paused())
			machine().resume();
		else
			machine().pause();
	}

	// handle a toggle cheats request
	if (!ui_error && ui_input_pressed_repeat(machine(), IPT_UI_TOGGLE_CHEAT, 0))
		machine().cheat().set_enable(!machine().cheat().enabled());

	// see if any other UI keys are pressed
	if (menu_event.iptkey == IPT_INVALID)
		for (int code = IPT_UI_FIRST + 1; code < IPT_UI_LAST; code++)
		{
			if (ui_error || code == IPT_UI_CONFIGURE || (code == IPT_UI_LEFT && ignoreleft)
				|| (code == IPT_UI_RIGHT && ignoreright) || (code == IPT_UI_PAUSE && ignorepause))
				continue;

			if (exclusive_input_pressed(code, 0))
				break;
		}
}

//-------------------------------------------------
//  handle input events for main menu
//-------------------------------------------------

void ui_menu::handle_main_events(UINT32 flags)
{
	int stop = false;
	ui_event local_menu_event;

	// loop while we have interesting events
	while (!stop && ui_input_pop_event(machine(), &local_menu_event))
	{
		switch (local_menu_event.event_type)
		{
			// if we are hovering over a valid item, select it with a single click
			case UI_EVENT_MOUSE_DOWN:
			{
				if (ui_error)
				{
					menu_event.iptkey = IPT_OTHER;
					stop = true;
				}
   				else if ((flags & UI_MENU_PROCESS_ONLYCHAR) == 0)
   				{
					if (hover >= 0 && hover < item.size())
						selected = hover;
					else if (hover == -2)
					{
						selected -= visitems;
						if (selected < 0)
							selected = 0;
						top_line -= visitems - (top_line + visible_lines == visible_items);
					}
					else if (hover == -1)
					{
						selected += visible_lines - 2 + (selected == 0);
						if (selected >= visible_items)
							selected = visible_items - 1;
						top_line += visible_lines - 2;
					}
					else if (hover == -3)
						menu_event.iptkey = IPT_UI_RIGHT;
					else if (hover == -4)
						menu_event.iptkey = IPT_UI_LEFT;
					else if (hover == -5)
						topline_datsview += right_visible_lines - 1;
					else if (hover == -6)
						topline_datsview -= right_visible_lines - 1;
					else if (r_hover >= RP_FIRST && r_hover <= RP_LAST)
					{
						mewui_globals::rpanel_infos = r_hover;
						stop = true;
					}
					else if (l_sw_hover >= MEWUI_SW_FIRST && l_sw_hover <= MEWUI_SW_LAST)
					{
						menu_event.iptkey = IPT_OTHER;
						stop = true;
					}
					else if (ume_hover >= MEWUI_MAME_FIRST && ume_hover <= MEWUI_MAME_LAST)
					{
						mewui_globals::ume_system = ume_hover;
						menu_event.iptkey = IPT_OTHER;
						stop = true;
					}
					else if (l_hover >= FILTER_FIRST && l_hover <= FILTER_LAST)
					{
						menu_event.iptkey = IPT_OTHER;
						stop = true;
					}
				}
				break;
			}

			// if we are hovering over a valid item, fake a UI_SELECT with a double-click
			case UI_EVENT_MOUSE_DOUBLE_CLICK:
				if ((flags & UI_MENU_PROCESS_ONLYCHAR) == 0)
   				{
					if (hover >= 0 && hover < item.size())
					{
						selected = hover;
						menu_event.iptkey = IPT_UI_SELECT;
					}

					if (selected == item.size() - 1)
					{
						menu_event.iptkey = IPT_UI_CANCEL;
						ui_menu::stack_pop(machine());
					}
					stop = true;
				}

				break;

			// caught scroll event
			case UI_EVENT_MOUSE_SCROLL:
				if ((flags & UI_MENU_PROCESS_ONLYCHAR) == 0)
   				{
					if (local_menu_event.zdelta > 0)
					{
						if (selected >= visible_items || selected == 0 || ui_error)
							break;
						selected -= local_menu_event.num_lines;
						if (selected < top_line + (top_line != 0))
							top_line -= local_menu_event.num_lines;
					}
					else
					{
						if (selected >= visible_items - 1 || ui_error)
							break;
						selected += local_menu_event.num_lines;
						if (selected > visible_items - 1)
							selected = visible_items - 1;
						if (selected >= top_line + visitems + (top_line != 0))
							top_line += local_menu_event.num_lines;
					}
				}
				break;

			// translate CHAR events into specials
			case UI_EVENT_CHAR:
				menu_event.iptkey = IPT_SPECIAL;
				menu_event.unichar = local_menu_event.ch;
				stop = true;
				break;

			// ignore everything else
			default:
				break;
		}
	}
}

//-------------------------------------------------
//  draw left box
//-------------------------------------------------

float ui_menu::draw_left_box(float x1, float y1, float x2, float y2, bool software)
{
	float text_size = 0.75f;
	float line_height = machine().ui().get_line_height() * text_size;
	float left_width = 0.0f;
	int text_lenght = (software) ? mewui_globals::sw_filter_len : mewui_globals::s_filter_text;
	int afilter = (software) ? mewui_globals::actual_sw_filter : mewui_globals::actual_filter;
	int *hover = (software) ? &l_sw_hover : &l_hover;
	const char **text = (software) ? mewui_globals::sw_filter_text : mewui_globals::filter_text;

	float sc = y2 - y1 - (2.0f * UI_BOX_TB_BORDER);

	if ((text_lenght * line_height) > sc)
	{
		float lm = sc / (text_lenght);
		text_size = lm / machine().ui().get_line_height();
		line_height = machine().ui().get_line_height() * text_size;
	}

	for (int x = 0; x < text_lenght; x++)
	{
		float total_width;

		// compute width of left hand side
		total_width = machine().ui().get_string_width_ex(text[x], text_size);

		// track the maximum
		if (total_width > left_width)
			left_width = total_width;
	}

	x2 += left_width;

	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;
	y2 -= UI_BOX_TB_BORDER;

	*hover = -1;

	for (int filter = 0; filter < text_lenght; filter++)
	{
		rgb_t bgcolor = UI_TEXT_BG_COLOR;

		if (mouse_hit && x1 <= mouse_x && x2 > mouse_x && y1 <= mouse_y && y1 + line_height > mouse_y)
		{
			bgcolor = UI_MOUSEOVER_BG_COLOR;
			*hover = filter;
		}

		if (afilter == filter)
			bgcolor = UI_SELECTED_BG_COLOR;

		if (bgcolor != UI_TEXT_BG_COLOR)
			container->add_rect(x1, y1, x2, y1 + line_height, bgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));

		machine().ui().draw_text_full(container, text[filter], x1, y1, x2 - x1, JUSTIFY_LEFT, WRAP_NEVER,
									  DRAW_NORMAL, UI_TEXT_COLOR, bgcolor, NULL, NULL, text_size);

		y1 += line_height;
	}

	return x2 + 2.0f * UI_BOX_LR_BORDER;
}

//-------------------------------------------------
//  draw UME box
//-------------------------------------------------

void ui_menu::draw_ume_box(float x1, float y1, float x2, float y2)
{
	float text_size = 0.65f;
	float line_height = machine().ui().get_line_height() * text_size;
	float maxwidth = 0.0f;

	for (int x = 0; x < mewui_globals::s_ume_text; x++)
	{
		float width;
		// compute width of left hand side
		machine().ui().draw_text_full(container, mewui_globals::ume_text[x], 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
									  DRAW_NONE, UI_TEXT_COLOR, ARGB_BLACK, &width, NULL, text_size);
		width += 2 * UI_BOX_LR_BORDER;
		maxwidth = MAX(maxwidth, width);
	}

	x2 = x1 + maxwidth;

	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;
	y2 -= UI_BOX_TB_BORDER;

	ume_hover = -1;

	for (int filter = 0; filter < mewui_globals::s_ume_text; filter++)
	{
		rgb_t bgcolor = UI_TEXT_BG_COLOR;

		if (mouse_hit && x1 <= mouse_x && x2 > mouse_x && y1 <= mouse_y && y1 + line_height > mouse_y)
		{
			bgcolor = UI_MOUSEOVER_BG_COLOR;
			ume_hover = filter;
		}

		if (mewui_globals::ume_system == filter)
			bgcolor = UI_SELECTED_BG_COLOR;

		if (bgcolor != UI_TEXT_BG_COLOR)
			container->add_rect(x1, y1, x2, y1 + line_height, bgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));

		machine().ui().draw_text_full(container, mewui_globals::ume_text[filter], x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
									  DRAW_NORMAL, UI_TEXT_COLOR, bgcolor, NULL, NULL, text_size);

		y1 += line_height;
	}
}

//-------------------------------------------------
//  draw right box
//-------------------------------------------------

float ui_menu::draw_right_box_title(float x1, float y1, float x2, float y2)
{
	float line_height = machine().ui().get_line_height();
	float midl = (x2 - x1) * 0.5f;

	// add outlined box for options
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	// add separator line
	container->add_line(x1 + midl, y1, x1 + midl, y1 + line_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	std::string buffer[RP_LAST + 1];
	buffer[RP_IMAGES].assign("Images");
	buffer[RP_INFOS].assign("Infos");

	r_hover = -1;

	for (int cells = RP_IMAGES; cells <= RP_INFOS; cells++)
	{
		rgb_t bgcolor = UI_TEXT_BG_COLOR;
		rgb_t fgcolor = UI_TEXT_COLOR;

		if (mouse_hit && x1 <= mouse_x && x1 + midl > mouse_x && y1 <= mouse_y && y1 + line_height > mouse_y)
		{
			if (mewui_globals::rpanel_infos != cells)
			{
				bgcolor = UI_MOUSEOVER_BG_COLOR;
				r_hover = cells;
			}
		}

		if (mewui_globals::rpanel_infos != cells)
		{
			container->add_line(x1, y1 + line_height, x1 + midl, y1 + line_height, UI_LINE_WIDTH,
								UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			fgcolor = UI_CLONE_COLOR;
		}

		if (bgcolor != UI_TEXT_BG_COLOR)
			container->add_rect(x1 + UI_LINE_WIDTH, y1 + UI_LINE_WIDTH, x1 + midl - UI_LINE_WIDTH, y1 + line_height,
								bgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));

		machine().ui().draw_text_full(container, buffer[cells].c_str(), x1 + UI_LINE_WIDTH, y1, midl - UI_LINE_WIDTH,
									  JUSTIFY_CENTER, WRAP_NEVER, DRAW_NORMAL, fgcolor, bgcolor, NULL, NULL);
		x1 = x1 + midl;
	}

	return (y1 + line_height + UI_LINE_WIDTH);
}

//-------------------------------------------------
//  draw infos
//-------------------------------------------------

void ui_menu::infos_render(void *selectedref, float origx1, float origy1, float origx2, float origy2, bool software)
{
	static std::string buffer;
	std::vector<int> xstart;
	std::vector<int> xend;

	float text_size = machine().options().infos_size();
	const game_driver *driver = NULL;
	ui_software_info *soft = NULL;

	static ui_software_info *oldsoft = NULL;
	static const game_driver *olddriver = NULL;
	static int oldview = -1;
	static int old_sw_view = -1;

	if (software)
	{
		soft = ((FPTR)selectedref > 2) ? (ui_software_info *)selectedref : NULL;
		if (mewui_globals::actual_filter == FILTER_FAVORITE_GAME && soft->startempty == 1)
		{
			driver = soft->driver;
			oldsoft = NULL;
		}
		else
			olddriver = NULL;
	}
	else
	{
		driver = ((FPTR)selectedref > 2) ? (const game_driver *)selectedref : NULL;
		oldsoft = NULL;
	}

	if (driver)
	{
		float line_height = machine().ui().get_line_height();
		float gutter_width = 0.4f * line_height * machine().render().ui_aspect() * 1.3f;
		float ud_arrow_width = line_height * machine().render().ui_aspect();
		float oy1 = origy1 + line_height;

		// MAMESCORE? Full size text
		if (mewui_globals::curdats_view == MEWUI_STORY_LOAD)
			text_size = 1.0f;

		std::string snaptext(dats_info[mewui_globals::curdats_view]);

		// apply title to right panel
		float title_size = 0.0f;
		float txt_lenght = 0.0f;

		for (int x = MEWUI_FIRST_LOAD; x < MEWUI_LAST_LOAD; x++)
		{
			machine().ui().draw_text_full(container, dats_info[x], origx1, origy1, origx2 - origx1, JUSTIFY_CENTER,
										  WRAP_TRUNCATE, DRAW_NONE, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, &txt_lenght, NULL);
			txt_lenght += 0.01f;
			title_size = MAX(txt_lenght, title_size);
		}

		machine().ui().draw_text_full(container, snaptext.c_str(), origx1, origy1, origx2 - origx1, JUSTIFY_CENTER,
									  WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

		draw_common_arrow(origx1, origy1, origx2, origy2, mewui_globals::curdats_view, MEWUI_FIRST_LOAD, MEWUI_LAST_LOAD, title_size);

		if (driver != olddriver || mewui_globals::curdats_view != oldview)
		{
			buffer.clear();
			olddriver = driver;
			oldview = mewui_globals::curdats_view;
			topline_datsview = 0;
			int f_return = 0;
			totallines = 0;
			std::vector<std::string> m_item;

			if (mewui_globals::curdats_view == MEWUI_GENERAL_LOAD)
				general_info(machine(), driver, buffer);
			else if (mewui_globals::curdats_view != MEWUI_COMMAND_LOAD)
				machine().datfile().load_data_info(driver, buffer, mewui_globals::curdats_view);
			else
				machine().datfile().command_sub_menu(driver, m_item);

			if (!m_item.empty() && mewui_globals::curdats_view == MEWUI_COMMAND_LOAD)
			{
				for (int x = 0; x < m_item.size(); x++)
				{
					std::string t_buffer;
					machine().datfile().load_command_info(t_buffer, x);
					buffer.append(m_item[x]).append("\n");
					if (!t_buffer.empty())
						buffer.append(t_buffer).append("\n");
				}
				convert_command_glyph(buffer);
			}
		}

		if (buffer.empty())
		{
			machine().ui().draw_text_full(container, "No Infos Available", origx1, (origy2 + origy1) * 0.5f, origx2 - origx1, JUSTIFY_CENTER,
										  WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
			return;
		}
		else if (mewui_globals::curdats_view != MEWUI_STORY_LOAD && mewui_globals::curdats_view != MEWUI_COMMAND_LOAD)
			machine().ui().wrap_text(container, buffer.c_str(), origx1, origy1, origx2 - origx1 - (2.0f * gutter_width), &totallines,
									 xstart, xend, text_size);
		else
			machine().ui().wrap_text(container, buffer.c_str(), 0.0f, 0.0f, 1.0f - (2.0f * gutter_width), &totallines, xstart, xend, text_size);

		int r_visible_lines = floor((origy2 - oy1) / (line_height * text_size));
		if (totallines < r_visible_lines)
			r_visible_lines = totallines;
		if (topline_datsview < 0)
			topline_datsview = 0;
		if (topline_datsview + r_visible_lines >= totallines)
			topline_datsview = totallines - r_visible_lines;

		for (int r = 0; r < r_visible_lines; r++)
		{
			int itemline = r + topline_datsview;
			std::string tempbuf;
			tempbuf.assign(buffer.substr(xstart[itemline], xend[itemline] - xstart[itemline]));

			// up arrow
			if (r == 0 && topline_datsview != 0)
				info_arrow(0, origx1, origx2, oy1, line_height, text_size, ud_arrow_width);
			// bottom arrow
			else if (r == r_visible_lines - 1 && itemline != totallines - 1)
				info_arrow(1, origx1, origx2, oy1, line_height, text_size, ud_arrow_width);
			// special case for mamescore
			else if (mewui_globals::curdats_view == MEWUI_STORY_LOAD)
			{
				int last_underscore = tempbuf.find_last_of('_');
				if (last_underscore == -1)
					machine().ui().draw_text_full(container, tempbuf.c_str(), origx1, oy1, origx2 - origx1, JUSTIFY_CENTER,
												  WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL,
												  text_size);
				else
				{
					float effective_width = origx2 - origx1 - gutter_width;
					float effective_left = origx1 + gutter_width;
					std::string last_part(tempbuf.substr(last_underscore + 1));
					int primary = tempbuf.find("___");
					std::string first_part(tempbuf.substr(0, primary));
					float item_width;

					machine().ui().draw_text_full(container, first_part.c_str(), effective_left, oy1, effective_width,
												  JUSTIFY_LEFT, WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR,
												  &item_width, NULL, text_size);

					machine().ui().draw_text_full(container, last_part.c_str(), effective_left + item_width, oy1,
												  origx2 - origx1 - 2.0f * gutter_width - item_width, JUSTIFY_RIGHT,
												  WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR,
												  NULL, NULL, text_size);
					}
			}

			// special case for command
			else if (mewui_globals::curdats_view == MEWUI_COMMAND_LOAD || mewui_globals::curdats_view == MEWUI_GENERAL_LOAD)
			{
				int first_dspace = (mewui_globals::curdats_view == MEWUI_COMMAND_LOAD) ? tempbuf.find("  ") : tempbuf.find(":");
				if (first_dspace > 0)
				{
					float effective_width = origx2 - origx1 - gutter_width;
					float effective_left = origx1 + gutter_width;
					std::string first_part(tempbuf.substr(0, first_dspace));
					std::string last_part(tempbuf.substr(first_dspace + 1));
					strtrimspace(last_part);
					machine().ui().draw_text_full(container, first_part.c_str(), effective_left, oy1, effective_width,
													JUSTIFY_LEFT, WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR,
													NULL, NULL, text_size);

					machine().ui().draw_text_full(container, last_part.c_str(), effective_left, oy1, origx2 - origx1 - 2.0f * gutter_width,
												  JUSTIFY_RIGHT, WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR,
												  NULL, NULL, text_size);
				}
				else
					machine().ui().draw_text_full(container, tempbuf.c_str(), origx1 + gutter_width, oy1, origx2 - origx1, JUSTIFY_LEFT,
												  WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL, text_size);
			}
			else
				machine().ui().draw_text_full(container, tempbuf.c_str(), origx1 + gutter_width, oy1, origx2 - origx1, JUSTIFY_LEFT,
											  WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL, text_size);

			oy1 += (line_height * text_size);
		}

		// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
		right_visible_lines = r_visible_lines - (topline_datsview != 0) - (topline_datsview + r_visible_lines != totallines);
	}
	else if (soft)
	{
		float line_height = machine().ui().get_line_height();
		float gutter_width = 0.4f * line_height * machine().render().ui_aspect() * 1.3f;
		float ud_arrow_width = line_height * machine().render().ui_aspect();
		float oy1 = origy1 + line_height;

		// apply title to right panel
		if (soft->usage.empty())
		{
			machine().ui().draw_text_full(container, "History", origx1, origy1, origx2 - origx1, JUSTIFY_CENTER, WRAP_TRUNCATE,
										  DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
			mewui_globals::cur_sw_dats_view = 0;
		}
		else
		{
			float title_size = 0.0f;
			float txt_lenght = 0.0f;
			std::string t_text[2];
			t_text[0].assign("History");
			t_text[1].assign("Usage");

			for (int x = 0; x < 2; x++)
			{
				machine().ui().draw_text_full(container, t_text[x].c_str(), origx1, origy1, origx2 - origx1, JUSTIFY_CENTER, WRAP_TRUNCATE,
											  DRAW_NONE, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, &txt_lenght, NULL);
				txt_lenght += 0.01f;
				title_size = MAX(txt_lenght, title_size);
			}

			machine().ui().draw_text_full(container, t_text[mewui_globals::cur_sw_dats_view].c_str(), origx1, origy1, origx2 - origx1,
										  JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR,
										  NULL, NULL);

			draw_common_arrow(origx1, origy1, origx2, origy2, mewui_globals::cur_sw_dats_view, 0, 1, title_size);
		}

		if (oldsoft != soft || old_sw_view != mewui_globals::cur_sw_dats_view)
		{
			if (mewui_globals::cur_sw_dats_view == 0)
			{
				buffer.clear();
				old_sw_view = mewui_globals::cur_sw_dats_view;
				oldsoft = soft;
				if (soft->startempty == 1)
					machine().datfile().load_data_info(soft->driver, buffer, MEWUI_HISTORY_LOAD);
				else
					machine().datfile().load_software_info(soft->listname.c_str(), buffer, soft->shortname.c_str());
			}
			else
			{
				buffer.clear();
				old_sw_view = mewui_globals::cur_sw_dats_view;
				oldsoft = soft;
				buffer.assign(soft->usage);
			}
		}

		if (buffer.empty())
		{
			machine().ui().draw_text_full(container, "No Infos Available", origx1, (origy2 + origy1) * 0.5f, origx2 - origx1, JUSTIFY_CENTER,
										  WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
			return;
		}
		else
			machine().ui().wrap_text(container, buffer.c_str(), origx1, origy1, origx2 - origx1 - (2.0f * gutter_width), &totallines,
									 xstart, xend, text_size);

		int r_visible_lines = floor((origy2 - oy1) / (line_height * text_size));
		if (totallines < r_visible_lines)
			r_visible_lines = totallines;
		if (topline_datsview < 0)
				topline_datsview = 0;
		if (topline_datsview + r_visible_lines >= totallines)
				topline_datsview = totallines - r_visible_lines;

		for (int r = 0; r < r_visible_lines; r++)
		{
			int itemline = r + topline_datsview;
			std::string tempbuf;
			tempbuf.assign(buffer.substr(xstart[itemline], xend[itemline] - xstart[itemline]));

			// up arrow
			if (r == 0 && topline_datsview != 0)
				info_arrow(0, origx1, origx2, oy1, line_height, text_size, ud_arrow_width);
			// bottom arrow
			else if (r == r_visible_lines - 1 && itemline != totallines - 1)
				info_arrow(1, origx1, origx2, oy1, line_height, text_size, ud_arrow_width);
			else
				machine().ui().draw_text_full(container, tempbuf.c_str(), origx1 + gutter_width, oy1, origx2 - origx1,
											  JUSTIFY_LEFT, WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR,
											  NULL, NULL, text_size);
			oy1 += (line_height * text_size);
		}

		// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
		right_visible_lines = r_visible_lines - (topline_datsview != 0) - (topline_datsview + r_visible_lines != totallines);
	}
}

//-------------------------------------------------
//  common function for images render
//-------------------------------------------------

std::string ui_menu::arts_render_common(float origx1, float origy1, float origx2, float origy2)
{
	std::string snaptext, searchstr;
	get_title_search(snaptext, searchstr);

	// apply title to right panel
	float title_size = 0.0f;
	float txt_lenght = 0.0f;

	for (int x = FIRST_VIEW; x < LAST_VIEW; x++)
	{
		machine().ui().draw_text_full(container, arts_info[x].title, origx1, origy1, origx2 - origx1, JUSTIFY_CENTER,
									  WRAP_TRUNCATE, DRAW_NONE, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, &txt_lenght, NULL);
		txt_lenght += 0.01f;
		title_size = MAX(txt_lenght, title_size);
	}

	machine().ui().draw_text_full(container, snaptext.c_str(), origx1, origy1, origx2 - origx1, JUSTIFY_CENTER, WRAP_TRUNCATE,
								  DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

	draw_common_arrow(origx1, origy1, origx2, origy2, mewui_globals::curimage_view, FIRST_VIEW, LAST_VIEW, title_size);

	return searchstr;
}

//-------------------------------------------------
//  draw favorites star
//-------------------------------------------------

void ui_menu::draw_star(render_container *container, float x0, float y0)
{
	float y1 = y0 + machine().ui().get_line_height();
	float x1 = x0 + machine().ui().get_line_height() * container->manager().ui_aspect();
	container->add_quad(x0, y0, x1, y1, ARGB_WHITE, star_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}

//-------------------------------------------------
//  perform rendering of image
//-------------------------------------------------

void ui_menu::arts_render_images(bitmap_argb32 *tmp_bitmap, float origx1, float origy1, float origx2, float origy2, bool software)
{
	bool no_available = false;
	float line_height = machine().ui().get_line_height();

	// if it fails, use the default image
	if (!tmp_bitmap->valid())
	{
		tmp_bitmap->reset();
		tmp_bitmap->allocate(256, 256);

		for (int x = 0; x < 256; x++)
			for (int y = 0; y < 256; y++)
				tmp_bitmap->pix32(y, x) = no_avail_bitmap->pix32(y, x);

		no_available = true;
	}

	if (tmp_bitmap->valid())
	{
		float panel_width = origx2 - origx1 - 0.02f;
		float panel_height = origy2 - origy1 - 0.02f - (2.0f * UI_BOX_TB_BORDER) - (2.0f * line_height);
		int screen_width = machine().render().ui_target().width();
		int screen_height = machine().render().ui_target().height();
		int panel_width_pixel = panel_width * screen_width;
		int panel_height_pixel = panel_height * screen_height;
		float ratio = 0.0f;

		// Calculate resize ratios for resizing
		float ratioW = (float)panel_width_pixel / tmp_bitmap->width();
		float ratioH = (float)panel_height_pixel / tmp_bitmap->height();
		float ratioI = (float)tmp_bitmap->height() / tmp_bitmap->width();
		int dest_xPixel = tmp_bitmap->width();
		int dest_yPixel = tmp_bitmap->height();

		// force 4:3 ratio min
		if (machine().options().forced_4x3_snapshot() && ratioI < 0.75f && mewui_globals::curimage_view == SNAPSHOT_VIEW)
		{
			// smaller ratio will ensure that the image fits in the view
			dest_yPixel = tmp_bitmap->width() * 0.75f;
			ratioH = (float)panel_height_pixel / dest_yPixel;
			ratio = MIN(ratioW, ratioH);
			dest_xPixel = tmp_bitmap->width() * ratio;
			dest_yPixel *= ratio;
		}
		// resize the bitmap if necessary
		else if (ratioW < 1 || ratioH < 1 || (machine().options().enlarge_snaps() && !no_available))
		{
			// smaller ratio will ensure that the image fits in the view
			ratio = MIN(ratioW, ratioH);
			dest_xPixel = tmp_bitmap->width() * ratio;
			dest_yPixel = tmp_bitmap->height() * ratio;
		}

		bitmap_argb32 *dest_bitmap;
		dest_bitmap = auto_alloc(machine(), bitmap_argb32);

		// resample if necessary
		if (dest_xPixel != tmp_bitmap->width() || dest_yPixel != tmp_bitmap->height())
		{
			dest_bitmap->allocate(dest_xPixel, dest_yPixel);
			render_color color = { 1.0f, 1.0f, 1.0f, 1.0f };
			render_resample_argb_bitmap_hq(*dest_bitmap, *tmp_bitmap, color, true);
		}
		else
			dest_bitmap = tmp_bitmap;

		snapx_bitmap->reset();
		snapx_bitmap->allocate(panel_width_pixel, panel_height_pixel);
		int x1 = (0.5f * panel_width_pixel) - (0.5f * dest_xPixel);
		int y1 = (0.5f * panel_height_pixel) - (0.5f * dest_yPixel);

		for (int x = 0; x < dest_xPixel; x++)
			for (int y = 0; y < dest_yPixel; y++)
				snapx_bitmap->pix32(y + y1, x + x1) = dest_bitmap->pix32(y, x);

		auto_free(machine(), dest_bitmap);

		// apply bitmap
		snapx_texture->set_bitmap(*snapx_bitmap, snapx_bitmap->cliprect(), TEXFORMAT_ARGB32);
	}
	else
		snapx_bitmap->reset();
}

//-------------------------------------------------
//  draw common arrows
//-------------------------------------------------

void ui_menu::draw_common_arrow(float origx1, float origy1, float origx2, float origy2, int current, int dmin, int dmax, float title_size)
{
	float line_height = machine().ui().get_line_height();
	float lr_arrow_width = 0.4f * line_height * machine().render().ui_aspect();
	float gutter_width = lr_arrow_width * 1.3f;

	// set left-right arrows dimension
	float ar_x0 = 0.5f * (origx2 + origx1) + 0.5f * title_size + gutter_width - lr_arrow_width;
	float ar_y0 = origy1 + 0.1f * line_height;
	float ar_x1 = 0.5f * (origx2 + origx1) + 0.5f * title_size + gutter_width;
	float ar_y1 = origy1 + 0.9f * line_height;

	float al_x0 = 0.5f * (origx2 + origx1) - 0.5f * title_size - gutter_width;
	float al_y0 = origy1 + 0.1f * line_height;
	float al_x1 = 0.5f * (origx2 + origx1) - 0.5f * title_size - gutter_width + lr_arrow_width;
	float al_y1 = origy1 + 0.9f * line_height;

	rgb_t fgcolor_right, fgcolor_left;
	fgcolor_right = fgcolor_left = UI_TEXT_COLOR;

	// set hover
	if (mouse_hit && ar_x0 <= mouse_x && ar_x1 > mouse_x && ar_y0 <= mouse_y && ar_y1 > mouse_y && current!= dmax)
	{
		machine().ui().draw_textured_box(container, ar_x0 + 0.01f, ar_y0, ar_x1 - 0.01f, ar_y1, UI_MOUSEOVER_BG_COLOR, rgb_t(255, 43, 43, 43),
										 hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
		hover = -3;
		fgcolor_right = UI_MOUSEOVER_COLOR;
	}
	else if (mouse_hit && al_x0 <= mouse_x && al_x1 > mouse_x && al_y0 <= mouse_y && al_y1 > mouse_y && current != dmin)
	{
		machine().ui().draw_textured_box(container, al_x0 + 0.01f, al_y0, al_x1 - 0.01f, al_y1, UI_MOUSEOVER_BG_COLOR, rgb_t(255, 43, 43, 43),
										 hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
		hover = -4;
		fgcolor_left = UI_MOUSEOVER_COLOR;
	}

	// apply arrow
	if (current == dmin)
		container->add_quad(ar_x0, ar_y0, ar_x1, ar_y1, fgcolor_right, arrow_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(ROT90));
	else if (current == dmax)
		container->add_quad(al_x0, al_y0, al_x1, al_y1, fgcolor_left, arrow_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(ROT90 ^ ORIENTATION_FLIP_X));
	else
	{
		container->add_quad(ar_x0, ar_y0, ar_x1, ar_y1, fgcolor_right, arrow_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(ROT90));
		container->add_quad(al_x0, al_y0, al_x1, al_y1, fgcolor_left, arrow_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(ROT90 ^ ORIENTATION_FLIP_X));
	}
}

//-------------------------------------------------
//  draw icons
//-------------------------------------------------

void ui_menu::draw_icon(render_container *container, int linenum, void *selectedref, float x0, float y0)
{
	static const game_driver *olddriver[MAX_ICONS_RENDER] = { NULL };
	float x1 = x0 + machine().ui().get_line_height() * container->manager().ui_aspect();
	float y1 = y0 + machine().ui().get_line_height();
	const game_driver *driver = ((FPTR)selectedref > 2) ? (const game_driver *)selectedref : NULL;

	if (driver == NULL)
		return;

	if (olddriver[linenum] != driver || mewui_globals::redraw_icon)
	{
		olddriver[linenum] = driver;

		// set clone status
		bool cloneof = strcmp(driver->parent, "0");
		if (cloneof)
		{
			int cx = driver_list::find(driver->parent);
			if (cx != -1 && ((driver_list::driver(cx).flags & GAME_IS_BIOS_ROOT) != 0))
				cloneof = false;
		}

		// get search path
		path_iterator path(machine().options().icons_directory());
		std::string curpath;
		std::string searchstr(machine().options().icons_directory());

		// iterate over path and add path for zipped formats
		while (path.next(curpath))
			searchstr.append(";").append(curpath.c_str()).append(PATH_SEPARATOR).append("icons");

		emu_file snapfile(searchstr.c_str(), OPEN_FLAG_READ);
		std::string fullname = std::string(driver->name).append(".ico");
		render_load_ico(*icons_bitmap[linenum], snapfile, NULL, fullname.c_str());

		if (!icons_bitmap[linenum]->valid() && cloneof)
		{
			fullname.assign(driver->parent).append(".ico");
			render_load_ico(*icons_bitmap[linenum], snapfile, NULL, fullname.c_str());
		}
	}

	if (icons_bitmap[linenum]->valid())
	{
		icons_texture[linenum]->set_bitmap(*icons_bitmap[linenum], icons_bitmap[linenum]->cliprect(), TEXFORMAT_ARGB32);
		container->add_quad(x0, y0, x1, y1, ARGB_WHITE, icons_texture[linenum], PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}
}

//-------------------------------------------------
//  draw info arrow
//-------------------------------------------------

void ui_menu::info_arrow(int ub, float origx1, float origx2, float oy1, float line_height, float text_size, float ud_arrow_width)
{
	rgb_t fgcolor = UI_TEXT_COLOR;
	UINT32 orientation = (!ub) ? ROT0 : ROT0 ^ ORIENTATION_FLIP_Y;

	if (mouse_hit && origx1 <= mouse_x && origx2 > mouse_x && oy1 <= mouse_y && oy1 + (line_height * text_size) > mouse_y)
	{
		machine().ui().draw_textured_box(container, origx1 + 0.01f, oy1, origx2 - 0.01f, oy1 + (line_height * text_size), UI_MOUSEOVER_BG_COLOR,
										 rgb_t(255, 43, 43, 43), hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
		hover = (!ub) ? -6 : -5;
		fgcolor = UI_MOUSEOVER_COLOR;
	}

	draw_arrow(container, 0.5f * (origx1 + origx2) - 0.5f * (ud_arrow_width * text_size), oy1 + 0.25f * (line_height * text_size),
			   0.5f * (origx1 + origx2) + 0.5f * (ud_arrow_width * text_size), oy1 + 0.75f * (line_height * text_size), fgcolor, orientation);
}

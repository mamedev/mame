/***************************************************************************

    selgame.h

    Game selector

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "selgame.h"
#include "uiinput.h"
#include "uimain.h"
#include "audit.h"


/*-------------------------------------------------
    ctor
-------------------------------------------------*/

ui_menu_select_game::ui_menu_select_game(running_machine &machine, render_container *container, const char *gamename) : ui_menu(machine, container)
{
	build_driver_list();
	if(gamename)
		strcpy(search, gamename);
}


/*-------------------------------------------------
    dtor
-------------------------------------------------*/

ui_menu_select_game::~ui_menu_select_game()
{
	global_free(driver_list);
}


/*-------------------------------------------------
    build_driver_list - build a list of available
	drivers
-------------------------------------------------*/

void ui_menu_select_game::build_driver_list()
{
	// start with an empty list
	driver_enumerator drivlist(machine().options());
	drivlist.exclude_all();

	// open a path to the ROMs and find them in the array
	file_enumerator path(machine().options().media_path());
	const osd_directory_entry *dir;

	// iterate while we get new objects
	while ((dir = path.next()) != NULL)
	{
		char drivername[50];
		char *dst = drivername;
		const char *src;

		/* build a name for it */
		for (src = dir->name; *src != 0 && *src != '.' && dst < &drivername[ARRAY_LENGTH(drivername) - 1]; src++)
			*dst++ = tolower((UINT8)*src);
		*dst = 0;

		int drivnum = drivlist.find(drivername);
		if (drivnum != -1)
			drivlist.include(drivnum);
	}

	// now build the final list
	drivlist.reset();

	driver_count = 0;
	driver_list = global_alloc_array(const game_driver *, driver_list::total()+1);
	while (drivlist.next())
	{
		const game_driver *drv = &drivlist.driver();
		if (!(drv->flags & GAME_NO_STANDALONE))
			driver_list[driver_count++] = drv;
	}

	// sort
	qsort(driver_list, driver_count, sizeof(driver_list[0]), driver_list_compare);

	// NULL-terminate
	driver_list[driver_count] = NULL;
}


/*-------------------------------------------------
    driver_list_compare - qsort callback for
	alphebetizing driver lists
-------------------------------------------------*/

int ui_menu_select_game::driver_list_compare(const void *p1, const void *p2)
{
	const game_driver *d1 = *((const game_driver **) p1); 
	const game_driver *d2 = *((const game_driver **) p2);
	return core_stricmp(d1->name, d2->name);
}


/*-------------------------------------------------
    populate
-------------------------------------------------*/

void ui_menu_select_game::populate()
{
	// if nothing there, add a single multiline item and return
	if (driver_count == 0)
	{
		astring txt;
		txt.printf("No %s found. Please check the rompath specified in the %s.ini file.\n\n"
					"If this is your first time using %s, please see the config.txt file in "
					"the docs directory for information on configuring %s.",
					emulator_info::get_gamesnoun(),
					emulator_info::get_configname(),
					emulator_info::get_appname(),emulator_info::get_appname() );
		item_append(txt.cstr(), NULL, MENU_FLAG_MULTILINE | MENU_FLAG_REDTEXT, NULL);
		return;
	}

	// populate all menu items
	for (int curitem = 0; curitem < driver_count; curitem++)
	{
		item_append(
			driver_list[curitem]->name,
			driver_list[curitem]->description,
			0,
			(void *) driver_list[curitem]);
	}

	// configure the custom rendering
	customtop = ui_get_line_height(machine()) + 3.0f * UI_BOX_TB_BORDER;
	custombottom = 4.0f * ui_get_line_height(machine()) + 3.0f * UI_BOX_TB_BORDER;
}


/*-------------------------------------------------
    handle - handle the game select menu
-------------------------------------------------*/

void ui_menu_select_game::handle()
{
	// ignore pause keys by swallowing them before we process the menu
	ui_input_pressed(machine(), IPT_UI_PAUSE);

	// process the menu
	const game_driver *old_selection = (const game_driver *) get_selection();
	const ui_menu_event *menu_event = process(0);
	const game_driver *new_selection = (const game_driver *) get_selection();
	
	// has the selection changed?
	if (old_selection != new_selection)
	{
		strcpy(search, new_selection ? new_selection->name : "");
	}

	// process events
	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		/* reset the error on any future menu_event */
		if (error)
			error = false;

		/* handle selections */
		else if (menu_event->iptkey == IPT_UI_SELECT)
		{
			const game_driver *driver = (const game_driver *)menu_event->itemref;

			/* special case for configure inputs */
			if ((FPTR)driver == 1)
				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_input_groups(machine(), container)));

			/* anything else is a driver */
			else
			{
				// audit the game first to see if we're going to work
				driver_enumerator enumerator(machine().options(), *driver);
				enumerator.next();
				media_auditor auditor(enumerator);
				media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

				// if everything looks good, schedule the new driver
				if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE)
				{
					machine().schedule_new_driver(*driver);
					ui_menu::stack_reset(machine());
				}

				// otherwise, display an error
				else
				{
					reset(UI_MENU_RESET_REMEMBER_REF);
					error = true;
				}
			}
		}

		/* escape pressed with non-empty text clears the text */
		else if (menu_event->iptkey == IPT_UI_CANCEL && search[0] != 0)
		{
			/* since we have already been popped, we must recreate ourself from scratch */
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_select_game(machine(), container, NULL)));
		}

		/* typed characters append to the buffer */
		else if (menu_event->iptkey == IPT_SPECIAL)
		{
			int buflen = strlen(search);

			/* if it's a backspace and we can handle it, do so */
			if ((menu_event->unichar == 8 || menu_event->unichar == 0x7f) && buflen > 0)
			{
				*(char *)utf8_previous_char(&search[buflen]) = 0;
				select_searched_item();
			}

			/* if it's any other key and we're not maxed out, update */
			else if (menu_event->unichar >= ' ' && menu_event->unichar < 0x7f)
			{
				buflen += utf8_from_uchar(&search[buflen], ARRAY_LENGTH(search) - buflen, menu_event->unichar);
				search[buflen] = 0;
				select_searched_item();
			}
		}
	}

	/* if we're in an error state, overlay an error message */
	if (error)
		ui_draw_text_box(container,
							"The selected game is missing one or more required ROM or CHD images. "
							"Please select a different game.\n\nPress any key to continue.",
							JUSTIFY_CENTER, 0.5f, 0.5f, UI_RED_COLOR);
}


/*-------------------------------------------------
    select_searched_item
-------------------------------------------------*/

void ui_menu_select_game::select_searched_item()
{
	// find the searched item
	int first = 0;
	int last = driver_count;
	while(last > first)
	{
		int middle = (first + last) / 2;		
		int rc = core_stricmp(search, driver_list[middle]->name);
		if (rc < 0)
			last = middle;
		else if (rc > 0)
			first = middle + 1;
		else
			first = last = middle;
	}

	// and set the selection
	set_selection((void *) driver_list[first]);
}


/*-------------------------------------------------
    custom_render - perform our special rendering
-------------------------------------------------*/

void ui_menu_select_game::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	const game_driver *driver;
	float width, maxwidth;
	float x1, y1, x2, y2;
	astring tempbuf[5];
	rgb_t color;
	int line;

	/* display the current typeahead */
	if (search[0] != 0)
		tempbuf[0].printf("Type name or select: %s_", search);
	else
		tempbuf[0].printf("Type name or select: (random)");

	/* get the size of the text */
	ui_draw_text_full(container, tempbuf[0], 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
						DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(width, origx2 - origx1);

	/* compute our bounds */
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy1 - top;
	y2 = origy1 - UI_BOX_TB_BORDER;

	/* draw a box */
	ui_draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	/* take off the borders */
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;
	y2 -= UI_BOX_TB_BORDER;

	/* draw the text within it */
	ui_draw_text_full(container, tempbuf[0], x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
						DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

	/* determine the text to render below */
	driver = ((FPTR)selectedref > 1) ? (const game_driver *)selectedref : NULL;
	if ((FPTR)driver > 1)
	{
		const char *gfxstat, *soundstat;

		/* first line is game name */
		tempbuf[0].printf("%-.100s", driver->description);

		/* next line is year, manufacturer */
		tempbuf[1].printf("%s, %-.100s", driver->year, driver->manufacturer);

		/* next line source path */
		tempbuf[2].printf("Driver: %-.100s", core_filename_extract_base(tempbuf[3], driver->source_file).cstr());

		/* next line is overall driver status */
		if (driver->flags & GAME_NOT_WORKING)
			tempbuf[3].cpy("Overall: NOT WORKING");
		else if (driver->flags & GAME_UNEMULATED_PROTECTION)
			tempbuf[3].cpy("Overall: Unemulated Protection");
		else
			tempbuf[3].cpy("Overall: Working");

		/* next line is graphics, sound status */
		if (driver->flags & (GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS | GAME_IMPERFECT_COLORS))
			gfxstat = "Imperfect";
		else
			gfxstat = "OK";

		if (driver->flags & GAME_NO_SOUND)
			soundstat = "Unimplemented";
		else if (driver->flags & GAME_IMPERFECT_SOUND)
			soundstat = "Imperfect";
		else
			soundstat = "OK";

		tempbuf[4].printf("Gfx: %s, Sound: %s", gfxstat, soundstat);
	}
	else
	{
		const char *s = emulator_info::get_copyright();
		line = 0;

		/* first line is version string */
		tempbuf[line++].printf("%s %s", emulator_info::get_applongname(), build_version);

		/* output message */
		while (line < ARRAY_LENGTH(tempbuf))
		{
			if (!(*s == 0 || *s == '\n'))
				tempbuf[line].cat(*s);

			if (*s == '\n')
			{
				line++;
				s++;
			} else if (*s != 0)
				s++;
			else
				line++;
		}
	}

	/* get the size of the text */
	maxwidth = origx2 - origx1;
	for (line = 0; line < 4; line++)
	{
		ui_draw_text_full(container, tempbuf[line], 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
							DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
		width += 2 * UI_BOX_LR_BORDER;
		maxwidth = MAX(maxwidth, width);
	}

	/* compute our bounds */
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy2 + UI_BOX_TB_BORDER;
	y2 = origy2 + bottom;

	/* draw a box */
	color = UI_BACKGROUND_COLOR;
	if (driver != NULL)
		color = UI_GREEN_COLOR;
	if (driver != NULL && (driver->flags & (GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS | GAME_IMPERFECT_COLORS | GAME_NO_SOUND | GAME_IMPERFECT_SOUND)) != 0)
		color = UI_YELLOW_COLOR;
	if (driver != NULL && (driver->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION)) != 0)
		color = UI_RED_COLOR;
	ui_draw_outlined_box(container, x1, y1, x2, y2, color);

	/* take off the borders */
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;
	y2 -= UI_BOX_TB_BORDER;

	/* draw all lines */
	for (line = 0; line < 4; line++)
	{
		ui_draw_text_full(container, tempbuf[line], x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
							DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
		y1 += ui_get_line_height(machine());
	}
}

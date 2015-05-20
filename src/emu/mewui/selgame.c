/*********************************************************************

    mewui/selgame.c

    Internal MEWUI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "mewui/datfile.h"
#include "mewui/inifile.h"
#include "mewui/selgame.h"
#include "drivenum.h"
#include "rendfont.h"
#include "uiinput.h"
#include "audit.h"
#include "ui/miscmenu.h"
#include "mewui/datmenu.h"
#include "mewui/dirmenu.h"
#include "mewui/optsmenu.h"
#include "mewui/selector.h"
#include "mewui/selsoft.h"
#include "sound/samples.h"
#include "unzip.h"
#include "mewui/custmenu.h"

//-------------------------------------------------
//  sort
//-------------------------------------------------

inline int c_stricmp(const char *s1, const char *s2)
{
	for (;;)
	{
		int c1 = tolower((UINT8)*s1++);
		int c2 = tolower((UINT8)*s2++);
		if (c1 == 0 || c1 != c2)
			return c1 - c2;
	}
}

bool sort_game_list(const game_driver *x, const game_driver *y)
{
	bool clonex = strcmp(x->parent, "0");
	bool cloney = strcmp(y->parent, "0");

	if (!clonex && !cloney)
		return (c_stricmp(x->description, y->description) < 0);

	int cx = -1, cy = -1;

	if (clonex)
	{
		cx = driver_list::find(x->parent);
		if (cx != -1 && ((driver_list::driver(cx).flags & GAME_IS_BIOS_ROOT) != 0))
			clonex = false;
	}

	if (cloney)
	{
		cy = driver_list::find(y->parent);
		if (cy != -1 && ((driver_list::driver(cy).flags & GAME_IS_BIOS_ROOT) != 0))
			cloney = false;
	}

	if (!clonex && !cloney)
		return (c_stricmp(x->description, y->description) < 0);

	else if (clonex && cloney)
	{
		if (!strcmp(x->parent, y->parent))
			return (c_stricmp(x->description, y->description) < 0);
		else
			return (c_stricmp(driver_list::driver(cx).description, driver_list::driver(cy).description) < 0);
	}

	else if (!clonex && cloney)
	{
		if (!c_stricmp(x->name, y->parent))
			return true;
		else
			return (c_stricmp(x->description, driver_list::driver(cy).description) < 0);
	}

	else
	{
		if (!c_stricmp(x->parent, y->name))
			return false;
		else
			return (c_stricmp(driver_list::driver(cx).description, y->description) < 0);
	}
}

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_mewui_select_game::ui_mewui_select_game(running_machine &machine, render_container *container, const char *gamename) : ui_menu(machine, container)
{
    if (!machine.options().remember_last())
    {
        reselect_last::driver.clear();
        reselect_last::software.clear();
        reselect_last::part.clear();
    }

    std::string error_string;
    machine.options().set_value(OPTION_SNAPNAME, "%g/%i", OPTION_PRIORITY_CMDLINE, error_string);
    machine.options().set_value(OPTION_SOFTWARENAME, "", OPTION_PRIORITY_CMDLINE, error_string);

    mewui_globals::curimage_view = FIRST_VIEW;
    mewui_globals::curdats_view = MEWUI_FIRST_LOAD;
    mewui_globals::switch_image = false;
    mewui_globals::default_image = true;
    l_sw_hover = -1;
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_mewui_select_game::~ui_mewui_select_game()
{
}

//-------------------------------------------------
//  Initialize all global arrays
//-------------------------------------------------

void ui_mewui_select_game::init(running_machine &machine)
{
    driverlist.resize(driver_list::total() + 1);
    sortedlist.resize(driver_list::total() + 1);

    // load drivers cache
    load_cache_info(machine, sortedlist);

    // build drivers list
    build_driver_list(machine);
    build_available_list(machine);

    // load custom filter
    load_custom_filters();
}

//-------------------------------------------------
//  Free all global arrays
//-------------------------------------------------

void ui_mewui_select_game::exit(running_machine &machine)
{
    // free years data
    c_year::ui.clear();

    // free manufacturers data
    c_mnfct::ui.clear();
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_mewui_select_game::handle()
{
    bool check_filter = false;

    // if i have to load datfile, performe an hard reset
    if (mewui_globals::force_reset_main)
    {
        mewui_globals::force_reset_main = false;
        machine().schedule_hard_reset();
        ui_menu::stack_reset(machine());
        return;
    }

    // if i have to reselect a software, force software list submenu
    if (mewui_globals::force_reselect_software)
    {
        const game_driver *driver = (const game_driver *)item[selected].ref;
        ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_select_software(machine(), container, driver)));
        return;
    }

    // ignore pause keys by swallowing them before we process the menu
    ui_input_pressed(machine(), IPT_UI_PAUSE);

    // process the menu
    const ui_menu_event *menu_event = process(UI_MENU_PROCESS_LR_REPEAT);

    if (menu_event != NULL && menu_event->itemref != NULL)
    {
        // reset the error on any future menu_event
        if (ui_error)
            ui_error = false;

        // handle selections
        else if (menu_event->iptkey == IPT_UI_SELECT)
        {
            if (mewui_globals::actual_filter != FILTER_FAVORITE_GAME)
                inkey_select(menu_event);
            else
                inkey_select_favorite(menu_event);
        }

        // handle UI_LEFT
        else if (menu_event->iptkey == IPT_UI_LEFT)
        {
            // Images
            if (mewui_globals::rpanel_infos == RP_IMAGES && mewui_globals::curimage_view > FIRST_VIEW)
            {
                mewui_globals::curimage_view--;
                mewui_globals::switch_image = true;
                mewui_globals::default_image = false;
            }

            // Infos
            else if (mewui_globals::rpanel_infos == RP_INFOS && mewui_globals::curdats_view > MEWUI_FIRST_LOAD)
            {
                mewui_globals::curdats_view--;
                topline_datsview = 0;
            }
        }

        // handle UI_RIGHT
        else if (menu_event->iptkey == IPT_UI_RIGHT)
        {
            // Images
            if (mewui_globals::rpanel_infos == RP_IMAGES && mewui_globals::curimage_view < LAST_VIEW)
            {
                mewui_globals::curimage_view++;
                mewui_globals::switch_image = true;
                mewui_globals::default_image = false;
            }

            // Infos
            else if (mewui_globals::rpanel_infos == RP_INFOS && mewui_globals::curdats_view < MEWUI_LAST_LOAD)
            {
                mewui_globals::curdats_view++;
                topline_datsview = 0;
            }
        }

        // handle UI_UP_FILTER
        else if (menu_event->iptkey == IPT_UI_UP_FILTER && mewui_globals::actual_filter > FILTER_FIRST)
        {
            l_hover = mewui_globals::actual_filter - 1;
            check_filter = true;
        }

        // handle UI_DOWN_FILTER
        else if (menu_event->iptkey == IPT_UI_DOWN_FILTER && mewui_globals::actual_filter < FILTER_LAST)
        {
            l_hover = mewui_globals::actual_filter + 1;
            check_filter = true;
        }

        // handle UI_LEFT_PANEL
        else if (menu_event->iptkey == IPT_UI_LEFT_PANEL)
            mewui_globals::rpanel_infos = RP_IMAGES;

        // handle UI_RIGHT_PANEL
        else if (menu_event->iptkey == IPT_UI_RIGHT_PANEL)
            mewui_globals::rpanel_infos = RP_INFOS;

        // escape pressed with non-empty text clears the text
        else if (menu_event->iptkey == IPT_UI_CANCEL && search[0] != 0)
        {
            search[0] = '\0';
            reset(UI_MENU_RESET_SELECT_FIRST);
        }

        // handle UI_HISTORY
        else if (menu_event->iptkey == IPT_UI_HISTORY && machine().options().historyfile() && machine().datfile().history_file())
        {
            if (mewui_globals::actual_filter != FILTER_FAVORITE_GAME)
            {
                const game_driver *driver = (const game_driver *)menu_event->itemref;
                if ((FPTR)driver > 2)
                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_dats(machine(), container, MEWUI_HISTORY_LOAD, driver)));
            }

            else
            {
                ui_software_info *swinfo  = (ui_software_info *)menu_event->itemref;

                if ((FPTR)swinfo > 2)
                {
                    if (swinfo->startempty == 1)
                        ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_dats(machine(), container, MEWUI_HISTORY_LOAD, swinfo->driver)));
                    else
                        ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_history_sw(machine(), container, swinfo)));
                }
            }
        }

        // handle UI_MAMEINFO
        else if (menu_event->iptkey == IPT_UI_MAMEINFO && machine().options().mamefile() &&
                    (machine().datfile().mame_file() || machine().datfile().mess_file()))
        {
            if (mewui_globals::actual_filter != FILTER_FAVORITE_GAME)
            {
                const game_driver *driver = (const game_driver *)menu_event->itemref;
                if ((FPTR)driver > 2)
                {
                    if ((driver->flags & GAME_TYPE_ARCADE) != 0)
                        ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_dats(machine(), container, MEWUI_MAMEINFO_LOAD, driver)));
                    else
                        ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_dats(machine(), container, MEWUI_MESSINFO_LOAD, driver)));
                }
            }

            else
            {
                ui_software_info *swinfo  = (ui_software_info *)menu_event->itemref;

                if ((FPTR)swinfo > 2 && swinfo->startempty == 1)
                {
                    if ((swinfo->driver->flags & GAME_TYPE_ARCADE) != 0)
                        ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_dats(machine(), container, MEWUI_MAMEINFO_LOAD, swinfo->driver)));
                    else
                        ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_dats(machine(), container, MEWUI_MESSINFO_LOAD, swinfo->driver)));
                }
            }
        }

        // handle UI_STORY
        else if (menu_event->iptkey == IPT_UI_STORY && machine().options().mamefile() && machine().datfile().story_file())
        {
            if (mewui_globals::actual_filter != FILTER_FAVORITE_GAME)
            {
                const game_driver *driver = (const game_driver *)menu_event->itemref;
                if ((FPTR)driver > 2)
                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_dats(machine(), container, MEWUI_STORY_LOAD, driver)));
            }

            else
            {
                ui_software_info *swinfo  = (ui_software_info *)menu_event->itemref;

                if ((FPTR)swinfo > 2 && swinfo->startempty == 1)
                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_dats(machine(), container, MEWUI_STORY_LOAD, swinfo->driver)));
            }
        }

        // handle UI_SYSINFO
        else if (menu_event->iptkey == IPT_UI_SYSINFO && machine().options().mamefile() && machine().datfile().sysinfo_file())
        {
            if (mewui_globals::actual_filter != FILTER_FAVORITE_GAME)
            {
                const game_driver *driver = (const game_driver *)menu_event->itemref;
                if ((FPTR)driver > 2)
                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_dats(machine(), container, MEWUI_SYSINFO_LOAD, driver)));
            }

            else
            {
                ui_software_info *swinfo  = (ui_software_info *)menu_event->itemref;

                if ((FPTR)swinfo > 2 && swinfo->startempty == 1)
                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_dats(machine(), container, MEWUI_SYSINFO_LOAD, swinfo->driver)));
            }
        }

        // handle UI_COMMAND
        else if (menu_event->iptkey == IPT_UI_COMMAND && machine().options().cmndfile() && machine().datfile().command_file())
        {
            if (mewui_globals::actual_filter != FILTER_FAVORITE_GAME)
            {
                const game_driver *driver = (const game_driver *)menu_event->itemref;
                if ((FPTR)driver > 2)
                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_command(machine(), container, driver)));
            }

            else
            {
                ui_software_info *swinfo  = (ui_software_info *)menu_event->itemref;

                if ((FPTR)swinfo > 2 && swinfo->startempty == 1)
                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_command(machine(), container, swinfo->driver)));
            }
        }

        // handle UI_FAVORITES
        else if (menu_event->iptkey == IPT_UI_FAVORITES)
        {
            if (mewui_globals::actual_filter != FILTER_FAVORITE_GAME)
            {
                const game_driver *driver = (const game_driver *)menu_event->itemref;
                if ((FPTR)driver > 2)
                {
                    if (!machine().favorite().isgame_favorite(driver))
                    {
                        machine().favorite().add_favorite_game(driver);
                        popmessage("%s\n added to favorites list.", driver->description);
                    }

                    else
                    {
                        machine().favorite().remove_favorite_game();
                        popmessage("%s\n removed from favorites list.", driver->description);
                    }
                }
            }

            else
            {
                ui_software_info *swinfo = (ui_software_info *)menu_event->itemref;
                if ((FPTR)swinfo > 2)
                {
                    popmessage("%s\n removed from favorites list.", swinfo->longname.c_str());
                    machine().favorite().remove_favorite_game(*swinfo);
                    reset(UI_MENU_RESET_SELECT_FIRST);
                }
            }
        }

        // typed characters append to the buffer
        else if (menu_event->iptkey == IPT_SPECIAL)
            inkey_special(menu_event);

        else if (menu_event->iptkey == IPT_OTHER)
            check_filter = true;
    }

    if (menu_event != NULL && menu_event->itemref == NULL)
    {
        if (menu_event->iptkey == IPT_SPECIAL && menu_event->unichar == 0x09)
            selected = prev_selected;

        // handle UI_UP_FILTER
        else if (menu_event->iptkey == IPT_UI_UP_FILTER && mewui_globals::actual_filter > FILTER_FIRST)
        {
            l_hover = mewui_globals::actual_filter - 1;
            check_filter = true;
        }

        // handle UI_DOWN_FILTER
        else if (menu_event->iptkey == IPT_UI_DOWN_FILTER && mewui_globals::actual_filter < FILTER_LAST)
        {
            l_hover = mewui_globals::actual_filter + 1;
            check_filter = true;
        }
        else if (menu_event->iptkey == IPT_OTHER)
            check_filter = true;
    }

    // if we're in an error state, overlay an error message
    if (ui_error)
        machine().ui().draw_text_box(container,
                                    "The selected game is missing one or more required ROM or CHD images. "
                                    "Please select a different game.\n\nPress any key (except ESC) to continue.",
                                    JUSTIFY_CENTER, 0.5f, 0.5f, UI_RED_COLOR);

    // handle filters selection from key shortcuts
    if (check_filter)
    {
        search[0] = '\0';

        if (l_hover == FILTER_CATEGORY)
        {
            mewui_globals::actual_filter = l_hover;
            ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_game_options(machine(), container)));
        }

        else if (l_hover == FILTER_CUSTOM)
        {
            mewui_globals::actual_filter = l_hover;
            ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_custom_filter(machine(), container, true)));
        }

        else if (l_hover == FILTER_MANUFACTURER)
            ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, c_mnfct::ui,
                                                &c_mnfct::actual, SELECTOR_GAME, l_hover)));
        else if (l_hover == FILTER_YEAR)
            ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, c_year::ui,
                                                &c_year::actual, SELECTOR_GAME, l_hover)));
        else
        {
            if (l_hover >= FILTER_ALL)
                mewui_globals::actual_filter = l_hover;
            reset(UI_MENU_RESET_SELECT_FIRST);
        }
    }
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_mewui_select_game::populate()
{
    mewui_globals::redraw_icon = true;
    mewui_globals::switch_image = true;
    int old_item_selected = -1;
    UINT32 flags_mewui = MENU_FLAG_MEWUI | MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW;

    if (mewui_globals::actual_filter != FILTER_FAVORITE_GAME)
    {
        // if search is not empty, find approximate matches
        if (search[0] != 0 && !no_active_search())
            populate_search();
        else
        {
            // reset search string
            search[0] = '\0';

            // if filter is set on category, build category list
            switch (mewui_globals::actual_filter)
            {
                case FILTER_CATEGORY:
                {
                    build_category();
                    break;
                }

                case FILTER_MANUFACTURER:
                    build_list(c_mnfct::ui[c_mnfct::actual].c_str());
                    break;

                case FILTER_YEAR:
                    build_list(c_year::ui[c_year::actual].c_str());
                    break;

                case FILTER_VECTOR:
                case FILTER_STEREO:
                case FILTER_SAMPLES:
                case FILTER_RASTER:
                case FILTER_CHD:
                    build_from_cache();
                    break;

                case FILTER_CUSTOM:
                    build_custom();
                    break;

                default:
                    build_list();
                    break;
            }

            // iterate over entries
            for (int curitem = 0; displaylist[curitem]; curitem++)
            {
                if (!reselect_last::driver.empty() && !(core_stricmp(displaylist[curitem]->name, reselect_last::driver.c_str())))
                    old_item_selected = curitem;

                bool cloneof = strcmp(displaylist[curitem]->parent, "0");
                if (cloneof)
                {
                    int cx = driver_list::find(displaylist[curitem]->parent);
                    if (cx != -1 && ((driver_list::driver(cx).flags & GAME_IS_BIOS_ROOT) != 0))
                        cloneof = false;
                }

                item_append(displaylist[curitem]->description, NULL, (!cloneof) ? flags_mewui : (MENU_FLAG_INVERT | flags_mewui),
                            (void *)displaylist[curitem]);
            }
        }
    }

    // populate favorites list
    else
    {
        // reset search string
        search[0] = '\0';

        flags_mewui = MENU_FLAG_MEWUI | MENU_FLAG_MEWUI_FAVORITE | MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW;

        // iterate over entries
        for (size_t x = 0; x < favorite_manager::favorite_list.size(); x++)
        {
            if (favorite_manager::favorite_list[x].startempty == 1)
            {
                bool cloneof = strcmp(favorite_manager::favorite_list[x].driver->parent, "0");
                if (cloneof)
                {
                    int cx = driver_list::find(favorite_manager::favorite_list[x].driver->parent);
                    if (cx != -1 && ((driver_list::driver(cx).flags & GAME_IS_BIOS_ROOT) != 0))
                        cloneof = false;
                }

                item_append(favorite_manager::favorite_list[x].longname.c_str(), NULL,
							(cloneof) ? (MENU_FLAG_INVERT | flags_mewui) : flags_mewui,
							(void *)&favorite_manager::favorite_list[x]);
            }

            else
                item_append(favorite_manager::favorite_list[x].longname.c_str(),
							favorite_manager::favorite_list[x].devicetype.c_str(),
							favorite_manager::favorite_list[x].parentname.empty() ? flags_mewui : (MENU_FLAG_INVERT | flags_mewui),
							(void *)&favorite_manager::favorite_list[x]);
        }
    }

    // add special items
    item_append(MENU_SEPARATOR_ITEM, NULL, MENU_FLAG_MEWUI, NULL);
    item_append("Configure Options", NULL, MENU_FLAG_MEWUI, (void *)1);
    item_append("Configure Directories", NULL, MENU_FLAG_MEWUI, (void *)2);

    // configure the custom rendering
    customtop = 2.0f * machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
    custombottom = 5.0f * machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;

    // reselect prior game launched, if any
    if (old_item_selected != -1)
    {
        selected = old_item_selected;
        top_line = selected - (mewui_globals::visible_main_lines / 2);

        if (reselect_last::software.empty())
            mewui_globals::force_reselect_software = false;
    }

    else
    {
        reselect_last::driver.clear();
        reselect_last::software.clear();
        reselect_last::part.clear();
        mewui_globals::force_reselect_software = false;
    }
}

//-------------------------------------------------
//  build a list of all drivers
//-------------------------------------------------

void ui_mewui_select_game::build_driver_list(running_machine &machine)
{
    // build full and sorted list
    driver_enumerator drivlist(machine.options());
    int curitem = 0;

    for ( ; drivlist.next(); curitem++)
    {
        driverlist[curitem] = &drivlist.driver();
        c_mnfct::set(drivlist.driver().manufacturer);
        c_year::set(drivlist.driver().year);
    }

    // NULL-terminate
    driverlist[curitem] = NULL;

    // sort
    std::sort(c_mnfct::ui.begin(), c_mnfct::ui.end());
    std::sort(c_year::ui.begin(), c_year::ui.end());
}

//-------------------------------------------------
//  build a list of available drivers
//-------------------------------------------------

void ui_mewui_select_game::build_driver_list(running_machine &machine)
{
    driver_enumerator availlist(machine.options());
	while (availlist.next())
		m_fulllist.push_back(&drivlist.driver());

	m_sortedlist = m_fulllist;

    availlist.exclude_all();
    file_enumerator path(machine.options().media_path());
    const osd_directory_entry *dir;

    while ((dir = path.next()) != NULL)
    {
        char drivername[50];
        char *dst = drivername;
        const char *src;

        for (src = dir->name; *src != 0 && *src != '.' && dst < &drivername[ARRAY_LENGTH(drivername) - 1]; src++)
            *dst++ = tolower((UINT8) * src);

        *dst = 0;

        int drivnum = availlist.find(drivername);

        if (drivnum != -1)
            ;
    }

    availlist.reset();

    while (availlist.next_excluded())
    {
        if (!strcmp("___empty", availlist.driver().name))
            continue;

        const rom_entry *rom = availlist.driver().rom;
        if (ROMENTRY_ISREGION(rom) && ROMENTRY_ISEND(++rom))
            availlist.include();
    }

    if (machine.options().audit_mode())
    {
        availlist.reset();
        while (availlist.next_excluded())
        {
            driver_enumerator enumerator(machine.options(), availlist.driver().name);
            enumerator.next();
            media_auditor auditor(enumerator);
            media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

            if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
                availlist.include();
        }

        zip_file_cache_clear();
    }

    availlist.reset();
    int listnum = 0;

    while (availlist.next())
        avllist[listnum++] = &availlist.driver();

    // NULL-terminate
    avllist[listnum] = NULL;

    // now build sorted list
    listnum = 0;
    availlist.reset();

    for (int k = 0; sortedlist[k]; k++)
    {
        int found = availlist.find(sortedlist[k]->name);
        if (found != -1 && availlist.included(found))
            avlsortedlist[listnum++] = sortedlist[k];
    }

    // NULL-terminate
    avlsortedlist[listnum] = NULL;

    // now build the unavailable list
    unavllist = global_alloc_array(const game_driver *, (driver_list::total() + 1 - availlist.count()));
    unavlsortedlist = global_alloc_array(const game_driver *, (driver_list::total() + 1 - availlist.count()));
    availlist.reset();
    listnum = 0;

    while (availlist.next_excluded())
    {
        if (!strcmp("___empty", availlist.driver().name))
            continue;

        unavllist[listnum++] = &availlist.driver();
    }

    // NULL-terminate
    unavllist[listnum] = NULL;

    // now build unavailable sorted list
    listnum = 0;
    availlist.reset();

    for (int k = 0; sortedlist[k]; k++)
    {
        int found = availlist.find(sortedlist[k]->name);
        if (found != -1 && availlist.excluded(found))
            unavlsortedlist[listnum++] = sortedlist[k];
    }

    // NULL-terminate
    unavlsortedlist[listnum] = NULL;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_mewui_select_game::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
    const game_driver *driver = NULL;
    ui_software_info *swinfo = NULL;
    float width, maxwidth = origx2 - origx1;
    float x1, y1, x2, y2;
    std::string tempbuf[5];
    rgb_t color = UI_BACKGROUND_COLOR;
    int line;
    bool isstar = false;

    strprintf(tempbuf[0], "MEWUI %s (%d / %d machines)", mewui_version, visible_items, (driver_list::total() - 1));

    std::string filtered;

    if (mewui_globals::actual_filter == FILTER_CATEGORY && machine().inifile().has_files())
    {
        IniFileIndex *category_file = machine().inifile().getfile();
        IniCategoryIndex *category = machine().inifile().getcategory();
        filtered.assign(mewui_globals::filter_text[mewui_globals::actual_filter]).append(" (").append(category_file->file_name).append(" - ").append(category->name).append(") -");
    }

    else if (mewui_globals::actual_filter == FILTER_MANUFACTURER)
        filtered.assign(mewui_globals::filter_text[mewui_globals::actual_filter]).append(" (").append(c_mnfct::ui[c_mnfct::actual]).append(") -");

    else if (mewui_globals::actual_filter == FILTER_YEAR)
        filtered.assign(mewui_globals::filter_text[mewui_globals::actual_filter]).append(" (").append(c_year::ui[c_year::actual]).append(") -");

    // display the current typeahead
    if (no_active_search())
        tempbuf[1].clear();
    else
        tempbuf[1].assign(filtered.c_str()).append(" Search: ").append(search).append("_");

    // get the size of the text
    for (line = 0; line < 2; line++)
    {
        machine().ui().draw_text_full(container, tempbuf[line].c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
                                        DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
        width += 2 * UI_BOX_LR_BORDER;
        maxwidth = MAX(width, origx2 - origx1);
    }

    // compute our bounds
    x1 = 0.5f - 0.5f * maxwidth;
    x2 = x1 + maxwidth;
    y1 = origy1 - top;
    y2 = origy1 - UI_BOX_TB_BORDER;

    // draw a box
    machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

    // take off the borders
    x1 += UI_BOX_LR_BORDER;
    x2 -= UI_BOX_LR_BORDER;
    y1 += UI_BOX_TB_BORDER;

    // draw the text within it
    for (line = 0; line < 2; line++)
    {
        machine().ui().draw_text_full(container, tempbuf[line].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
                                        DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
        y1 += machine().ui().get_line_height();
    }

    x1 -= UI_BOX_LR_BORDER;
    y1 = origy1 - top;
    draw_ume_box(x1, y1, x2, y2);

    // determine the text to render below
    if (mewui_globals::actual_filter != FILTER_FAVORITE_GAME)
        driver = ((FPTR)selectedref > 2) ? (const game_driver *)selectedref : NULL;
    else
    {
        swinfo = ((FPTR)selectedref > 2) ? (ui_software_info *)selectedref : NULL;

        if (swinfo && swinfo->startempty == 1)
            driver = swinfo->driver;
    }

    if ((FPTR)driver > 2)
    {
        isstar = machine().favorite().isgame_favorite(driver);

        // first line is game name
        strprintf(tempbuf[0], "Romset: %-.100s", driver->name);

        // next line is year, manufacturer
        strprintf(tempbuf[1], "%s, %-.100s", driver->year, driver->manufacturer);

        // next line is clone/parent status
        int cloneof = driver_list::non_bios_clone(*driver);

        if (cloneof != -1)
            strprintf(tempbuf[2], "Driver is clone of: %-.100s", driver_list::driver(cloneof).description);
        else
            tempbuf[2].assign("Driver is parent");

        // next line is overall driver status
        if (driver->flags & GAME_NOT_WORKING)
            tempbuf[3].assign("Overall: NOT WORKING");
        else if (driver->flags & GAME_UNEMULATED_PROTECTION)
            tempbuf[3].assign("Overall: Unemulated Protection");
        else
            tempbuf[3].assign("Overall: Working");

        // next line is graphics, sound status
        if (driver->flags & (GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS | GAME_IMPERFECT_COLORS))
            tempbuf[4].assign("Graphics: Imperfect, ");
        else
            tempbuf[4].assign("Graphics: OK, ");

        if (driver->flags & GAME_NO_SOUND)
            tempbuf[4].append("Sound: Unimplemented");
        else if (driver->flags & GAME_IMPERFECT_SOUND)
            tempbuf[4].append("Sound: Imperfect");
        else
            tempbuf[4].append("Sound: OK");

        if (driver != NULL)
            color = UI_GREEN_COLOR;

        if (driver != NULL && (driver->flags & (GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS | GAME_IMPERFECT_COLORS
                                                | GAME_NO_SOUND | GAME_IMPERFECT_SOUND)) != 0)
            color = UI_YELLOW_COLOR;

        if (driver != NULL && (driver->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION)) != 0)
            color = UI_RED_COLOR;
    }

    else if ((FPTR)swinfo > 2)
    {
        isstar = machine().favorite().isgame_favorite(*swinfo);

        // first line is system
        strprintf(tempbuf[0], "System: %-.100s", swinfo->driver->name);

        // next line is year, publisher
        strprintf(tempbuf[1], "%s, %-.100s", swinfo->year.c_str(), swinfo->publisher.c_str());

        // next line is parent/clone
        if (!swinfo->parentname.empty())
            strprintf(tempbuf[2], "Software is clone of: %-.100s", !swinfo->parentlongname.empty() ? swinfo->parentlongname.c_str() : swinfo->parentname.c_str());
        else
            tempbuf[2].assign("Software is parent");

        // next line is supported status
        if (swinfo->supported == SOFTWARE_SUPPORTED_NO)
        {
            tempbuf[3].assign("Supported: No");
            color = UI_RED_COLOR;
        }

        else if (swinfo->supported == SOFTWARE_SUPPORTED_PARTIAL)
        {
            tempbuf[3].assign("Supported: Partial");
            color = UI_YELLOW_COLOR;
        }

        else
        {
            tempbuf[3].assign("Supported: Yes");
            color = UI_GREEN_COLOR;
        }

        // last line is romset name
        strprintf(tempbuf[4], "romset: %-.100s", swinfo->shortname.c_str());
    }

    else
    {
        std::string copyright(emulator_info::get_copyright());
        size_t found = copyright.find("\n");

        tempbuf[0].assign(emulator_info::get_applongname()).append(" ").append(build_version);
        tempbuf[1].assign(copyright.substr(0, found));
        tempbuf[2].assign(copyright.substr(found + 1));
        tempbuf[3].clear();
        tempbuf[4].assign("MEWUI by dankan1890 http://sourceforge.net/projects/mewui");
    }

    // get the size of the text
    maxwidth = origx2 - origx1;

    for (line = 0; line < 5; line++)
    {
        machine().ui().draw_text_full(container, tempbuf[line].c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
                                        DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
        width += 2 * UI_BOX_LR_BORDER;
        maxwidth = MAX(maxwidth, width);
    }

    // compute our bounds
    x1 = 0.5f - 0.5f * maxwidth;
    x2 = x1 + maxwidth;
    y1 = origy2 + UI_BOX_TB_BORDER;
    y2 = origy2 + bottom;

    // draw a box
    machine().ui().draw_outlined_box(container, x1, y1, x2, y2, color);

    // take off the borders
    x1 += UI_BOX_LR_BORDER;
    x2 -= UI_BOX_LR_BORDER;
    y1 += UI_BOX_TB_BORDER;

    // is favorite? draw the star
    if (isstar)
        draw_star(container, x1, y1);

    // draw all lines
    for (line = 0; line < 5; line++)
    {
        machine().ui().draw_text_full(container, tempbuf[line].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
                                        DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
        y1 += machine().ui().get_line_height();
    }
}

//-------------------------------------------------
//  force the game select menu to be visible
//  and inescapable
//-------------------------------------------------

void ui_mewui_select_game::force_game_select(running_machine &machine, render_container *container)
{
    // reset the menu stack
    ui_menu::stack_reset(machine);

    // add the quit entry followed by the game select entry
    ui_menu *quit = auto_alloc_clear(machine, ui_menu_quit_game(machine, container));
    quit->set_special_main_menu(true);
    ui_menu::stack_push(quit);
    ui_menu::stack_push(auto_alloc_clear(machine, ui_mewui_select_game(machine, container, NULL)));

    // force the menus on
    machine.ui().show_menu();

    // make sure MAME is paused
    machine.pause();
}

//-------------------------------------------------
//  handle select key event
//-------------------------------------------------

void ui_mewui_select_game::inkey_select(const ui_menu_event *menu_event)
{
    const game_driver *driver = (const game_driver *)menu_event->itemref;

    // special case for configure options
    if ((FPTR)driver == 1)
        ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_game_options(machine(), container)));

    // special case for configure directory
    else if ((FPTR)driver == 2)
        ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_directory(machine(), container)));

    // anything else is a driver
    else
    {
        // audit the game first to see if we're going to work
        driver_enumerator enumerator(machine().options(), *driver);
        enumerator.next();
        media_auditor auditor(enumerator);
        media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

        // if everything looks good, schedule the new driver
        if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
        {
            if ((driver->flags & GAME_TYPE_ARCADE) == 0)
                ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_select_software(machine(), container, driver)));
            else
            {
                reselect_last::driver.assign(driver->name);
                reselect_last::software.clear();
                reselect_last::part.clear();
                machine().manager().schedule_new_driver(*driver);
                machine().schedule_hard_reset();
                ui_menu::stack_reset(machine());
            }
        }

        // otherwise, display an error
        else
        {
            reset(UI_MENU_RESET_REMEMBER_REF);
            ui_error = true;
        }
    }
}

//-------------------------------------------------
//  handle select key event for favorites menu
//-------------------------------------------------

void ui_mewui_select_game::inkey_select_favorite(const ui_menu_event *menu_event)
{
    ui_software_info *ui_swinfo = (ui_software_info *)menu_event->itemref;

    // special case for configure options
    if ((FPTR)ui_swinfo == 1)
        ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_game_options(machine(), container)));

    // special case for configure directory
    else if ((FPTR)ui_swinfo == 2)
        ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_directory(machine(), container)));

    else if (ui_swinfo->startempty == 1)
    {
        // audit the game first to see if we're going to work
        driver_enumerator enumerator(machine().options(), *ui_swinfo->driver);
        enumerator.next();
        media_auditor auditor(enumerator);
        media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

        // if everything looks good, schedule the new driver
        if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
        {
            machine().manager().schedule_new_driver(*ui_swinfo->driver);
            machine().schedule_hard_reset();
            ui_menu::stack_reset(machine());
        }

        // otherwise, display an error
        else
        {
            reset(UI_MENU_RESET_REMEMBER_REF);
            ui_error = true;
        }
    }

    else
    {
        // first validate
        driver_enumerator drv(machine().options(), *ui_swinfo->driver);
        media_auditor auditor(drv);
        drv.next();
        software_list_device *swlist = software_list_device::find_by_name(drv.config(), ui_swinfo->listname.c_str());
        software_info *swinfo = swlist->find(ui_swinfo->shortname.c_str());
        media_auditor::summary summary = auditor.audit_software(swlist->list_name(), swinfo, AUDIT_VALIDATE_FAST);

        if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE)
        {
            std::string error_string;
            std::string string_list = std::string(ui_swinfo->listname.c_str()).append(":").append(ui_swinfo->shortname.c_str()).append(":");
            string_list.append(ui_swinfo->part.c_str()).append(":").append(ui_swinfo->instance.c_str());
            machine().options().set_value(OPTION_SOFTWARENAME, string_list.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
            std::string snap_list = std::string(ui_swinfo->listname.c_str()).append(PATH_SEPARATOR).append(ui_swinfo->shortname.c_str());
            machine().options().set_value(OPTION_SNAPNAME, snap_list.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
            reselect_last::driver.assign(drv.driver().name);
            reselect_last::software.assign(ui_swinfo->shortname.c_str());
            reselect_last::part.assign(ui_swinfo->part.c_str());
            machine().manager().schedule_new_driver(drv.driver());
            machine().schedule_hard_reset();
            ui_menu::stack_reset(machine());
        }

        // otherwise, display an error
        else
        {
            reset(UI_MENU_RESET_REMEMBER_POSITION);
            ui_error = true;
        }
    }
}

//-------------------------------------------------
//  returns if the search can be activated
//-------------------------------------------------

bool ui_mewui_select_game::no_active_search()
{
    return (mewui_globals::actual_filter == FILTER_FAVORITE_GAME);
}

//-------------------------------------------------
//  handle special key event
//-------------------------------------------------

void ui_mewui_select_game::inkey_special(const ui_menu_event *menu_event)
{
    int buflen = strlen(search);

    // if it's a backspace and we can handle it, do so
    if (((menu_event->unichar == 8 || menu_event->unichar == 0x7f) && buflen > 0) && !no_active_search())
    {
        *(char *)utf8_previous_char(&search[buflen]) = 0;
        reset(UI_MENU_RESET_SELECT_FIRST);
    }

    // if it's any other key and we're not maxed out, update
    else if ((menu_event->unichar >= ' ' && menu_event->unichar < 0x7f) && !no_active_search())
    {
        buflen += utf8_from_uchar(&search[buflen], ARRAY_LENGTH(search) - buflen, menu_event->unichar);
        search[buflen] = 0;
        reset(UI_MENU_RESET_SELECT_FIRST);
    }

    // Tab key
    else if (menu_event->unichar == 0x09)
    {
        // if the selection is in the main screen, save it and go to submenu
        if (selected <= visible_items)
        {
            prev_selected = selected;
            selected = visible_items + 1;
        }

        // otherwise, retrieve the previous position
        else
            selected = prev_selected;
    }
}

//-------------------------------------------------
//  build list
//-------------------------------------------------

void ui_mewui_select_game::build_list(const char *filter_text, const game_driver **s_drivers, int filter, bool bioscheck)
{
    int arrayindex = 0, cx = 0;
    bool cloneof = false;

    if (s_drivers == NULL)
    {
        filter = mewui_globals::actual_filter;

        if (machine().options().ui_grouped())
        {
            if (filter == FILTER_AVAILABLE)
                s_drivers = avlsortedlist;
            else if (filter == FILTER_UNAVAILABLE)
                s_drivers = unavlsortedlist;
            else
                s_drivers = sortedlist;
        }
        else if (filter == FILTER_AVAILABLE)
            s_drivers = avllist;
        else if (filter == FILTER_UNAVAILABLE)
            s_drivers = unavllist;
        else
            s_drivers = driverlist;
    }

    for (int index = 0; s_drivers[index]; index++)
    {
        if (!bioscheck && filter != FILTER_BIOS && (s_drivers[index]->flags & GAME_IS_BIOS_ROOT) != 0)
            continue;

        if ((s_drivers[index]->flags & GAME_TYPE_ARCADE) && mewui_globals::ume_system == MEWUI_SYSTEMS)
            continue;

        if (!(s_drivers[index]->flags & GAME_TYPE_ARCADE) && mewui_globals::ume_system == MEWUI_ARCADES)
            continue;

        switch (filter)
        {
            case FILTER_ALL:
            case FILTER_AVAILABLE:
            case FILTER_UNAVAILABLE:
                displaylist[arrayindex++] = s_drivers[index];
                break;

            case FILTER_WORKING:
                if (!(s_drivers[index]->flags & GAME_NOT_WORKING))
                    displaylist[arrayindex++] = s_drivers[index];
                break;

            case FILTER_NOT_MECHANICAL:
                if (!(s_drivers[index]->flags & GAME_MECHANICAL))
                    displaylist[arrayindex++] = s_drivers[index];
                break;

            case FILTER_BIOS:
                if (s_drivers[index]->flags & GAME_IS_BIOS_ROOT)
                    displaylist[arrayindex++] = s_drivers[index];
                break;

            case FILTER_PARENT:
            case FILTER_CLONES:
                cloneof = strcmp(s_drivers[index]->parent, "0");

                if (cloneof)
                {
                    cx = driver_list::find(s_drivers[index]->parent);

                    if (cx != -1 && ((driver_list::driver(cx).flags & GAME_IS_BIOS_ROOT) != 0))
                        cloneof = false;
                }

                if (filter == FILTER_CLONES && cloneof)
                    displaylist[arrayindex++] = s_drivers[index];
                else if (filter == FILTER_PARENT && !cloneof)
                    displaylist[arrayindex++] = s_drivers[index];
                break;

            case FILTER_NOT_WORKING:
                if (s_drivers[index]->flags & GAME_NOT_WORKING)
                    displaylist[arrayindex++] = s_drivers[index];
                break;

            case FILTER_MECHANICAL:
                if (s_drivers[index]->flags & GAME_MECHANICAL)
                    displaylist[arrayindex++] = s_drivers[index];
                break;

            case FILTER_SAVE:
                if (s_drivers[index]->flags & GAME_SUPPORTS_SAVE)
                    displaylist[arrayindex++] = s_drivers[index];
                break;

            case FILTER_YEAR:
                if (!core_stricmp(filter_text, s_drivers[index]->year))
                    displaylist[arrayindex++] = s_drivers[index];
                break;

            case FILTER_VERTICAL:
                if (s_drivers[index]->flags & ORIENTATION_SWAP_XY)
                    displaylist[arrayindex++] = s_drivers[index];
                break;

            case FILTER_HORIZONTAL:
                if (!(s_drivers[index]->flags & ORIENTATION_SWAP_XY))
                    displaylist[arrayindex++] = s_drivers[index];
                break;

            case FILTER_MANUFACTURER:
            {
                std::string name = c_mnfct::getname(s_drivers[index]->manufacturer);

                if (!core_stricmp(filter_text, name.c_str()))
                    displaylist[arrayindex++] = s_drivers[index];
                break;
            }
        }
    }
    displaylist[arrayindex] = NULL;
}

//-------------------------------------------------
//  build custom display list
//-------------------------------------------------

void ui_mewui_select_game::build_custom()
{
    int arrayindex = 0;
    const game_driver **s_drivers;
    bool bioscheck = false;

    if (machine().options().ui_grouped())
    {
        if (custfltr::main_filter == FILTER_AVAILABLE)
            s_drivers = avlsortedlist;
        else if (custfltr::main_filter == FILTER_UNAVAILABLE)
            s_drivers = unavlsortedlist;
        else
            s_drivers = sortedlist;
    }
    else if (custfltr::main_filter == FILTER_AVAILABLE)
        s_drivers = avllist;
    else if (custfltr::main_filter == FILTER_UNAVAILABLE)
        s_drivers = unavllist;
    else
        s_drivers = driverlist;

    for (int index = 0; s_drivers[index]; index++)
    {
        if ((s_drivers[index]->flags & GAME_TYPE_ARCADE) && mewui_globals::ume_system == MEWUI_SYSTEMS)
            continue;

        if (!(s_drivers[index]->flags & GAME_TYPE_ARCADE) && mewui_globals::ume_system == MEWUI_ARCADES)
            continue;

        displaylist[arrayindex++] = s_drivers[index];
    }

    displaylist[arrayindex] = NULL;

    for (int count = 1; count <= custfltr::numother; count++)
    {
        int filter = custfltr::other[count];
        if (filter == FILTER_BIOS)
            bioscheck = true;
    }

    for (int count = 1; count <= custfltr::numother; count++)
    {
        int filter = custfltr::other[count];
        s_drivers = &displaylist[0];

        switch (filter)
        {
            case FILTER_YEAR:
                build_list(c_year::ui[custfltr::year[count]].c_str(), s_drivers, filter, bioscheck);
                break;
            case FILTER_MANUFACTURER:
                build_list(c_mnfct::ui[custfltr::mnfct[count]].c_str(), s_drivers, filter, bioscheck);
                break;
            case FILTER_VECTOR:
            case FILTER_RASTER:
            case FILTER_CHD:
            case FILTER_SAMPLES:
            case FILTER_STEREO:
                build_from_cache(s_drivers, filter, bioscheck);
                break;
            default:
                build_list(NULL, s_drivers, filter, bioscheck);
                break;
        }
    }
}

//-------------------------------------------------
//  build category list
//-------------------------------------------------

void ui_mewui_select_game::build_category()
{
    int arrayindex = 0;
    std::vector<int> temp_filter;
    machine().inifile().load_ini_category(temp_filter);

    while (arrayindex < temp_filter.size())
    {
        int actual = temp_filter[arrayindex];
        displaylist[arrayindex++] = &driver_list::driver(actual);
    }

    displaylist[arrayindex] = NULL;
}

//-------------------------------------------------
//  populate search list
//-------------------------------------------------

void ui_mewui_select_game::populate_search()
{
    UINT32 flags_mewui = MENU_FLAG_MEWUI | MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW;
    find_matches(search, VISIBLE_GAMES_IN_SEARCH);

    for (int curitem = 0; searchlist[curitem]; curitem++)
    {
        bool cloneof = strcmp(searchlist[curitem]->parent, "0");

        if (cloneof)
        {
            int cx = driver_list::find(searchlist[curitem]->parent);

            if (cx != -1 && ((driver_list::driver(cx).flags & GAME_IS_BIOS_ROOT) != 0))
                cloneof = false;
        }

        item_append(searchlist[curitem]->description, NULL, (!cloneof) ? flags_mewui : (MENU_FLAG_INVERT | flags_mewui),
                    (void *)searchlist[curitem]);
    }
}

//-------------------------------------------------
//  build list from cache
//-------------------------------------------------

void ui_mewui_select_game::build_from_cache(const game_driver **s_drivers, int filter, bool bioscheck)
{
    int arrayindex = 0;
    int empty = driver_list::find("___empty");

    if (s_drivers == NULL)
    {
        s_drivers = (machine().options().ui_grouped()) ? sortedlist : driverlist;
        filter = mewui_globals::actual_filter;
    }

    for (int index = 0; s_drivers[index]; index++)
    {
        if (!bioscheck && filter != FILTER_BIOS && (s_drivers[index]->flags & GAME_IS_BIOS_ROOT) != 0)
            continue;

        if ((s_drivers[index]->flags & GAME_TYPE_ARCADE) && mewui_globals::ume_system == MEWUI_SYSTEMS)
            continue;

        if (!(s_drivers[index]->flags & GAME_TYPE_ARCADE) && mewui_globals::ume_system == MEWUI_ARCADES)
            continue;

        int idx = index;

        if (machine().options().ui_grouped())
        {
            idx = driver_list::find(s_drivers[index]->name);
            if (idx > empty) idx--;
        }

        switch (filter)
        {
            case FILTER_VECTOR:
                if (mewui_globals::driver_cache[idx].b_vector)
                    displaylist[arrayindex++] = s_drivers[index];
                break;

            case FILTER_RASTER:
                if (!mewui_globals::driver_cache[idx].b_vector)
                    displaylist[arrayindex++] = s_drivers[index];
                break;

            case FILTER_SAMPLES:
                if (mewui_globals::driver_cache[idx].b_samples)
                    displaylist[arrayindex++] = s_drivers[index];
                break;

            case FILTER_STEREO:
                if (mewui_globals::driver_cache[idx].b_stereo)
                    displaylist[arrayindex++] = s_drivers[index];
                break;

            case FILTER_CHD:
                if (mewui_globals::driver_cache[idx].b_chd)
                    displaylist[arrayindex++] = s_drivers[index];
                break;
        }
    }

    displaylist[arrayindex] = NULL;
}

//-------------------------------------------------
//  find approximate matches
//-------------------------------------------------

void ui_mewui_select_game::find_matches(const char *str, int count)
{
    // allocate memory to track the penalty value
    std::vector<int> penalty(count, 9999);
    std::vector<int> penalty_old(count, 9999);
    int index = 0;

    for (; displaylist[index]; index++)
    {
        // pick the best match between driver name and description
        int curpenalty_old = driver_list::penalty_compare(str, displaylist[index]->description);
        int tmp_old = driver_list::penalty_compare(str, displaylist[index]->name);
        int curpenalty = fuzzy_substring(str, displaylist[index]->description);
        int tmp = fuzzy_substring(str, displaylist[index]->name);
        curpenalty = MIN(curpenalty, tmp);
        curpenalty_old = MIN(curpenalty_old, tmp_old);

        // insert into the sorted table of matches
        for (int matchnum = count - 1; matchnum >= 0; matchnum--)
        {
            // stop if we're worse than the current entry
            if (curpenalty > penalty[matchnum])
                break;

            if (curpenalty == penalty[matchnum])
                if (curpenalty_old >= penalty_old[matchnum])
                    break;

            // as long as this isn't the last entry, bump this one down
            if (matchnum < count - 1)
            {
                penalty[matchnum + 1] = penalty[matchnum];
                penalty_old[matchnum + 1] = penalty_old[matchnum];
                searchlist[matchnum + 1] = searchlist[matchnum];
            }

            searchlist[matchnum] = displaylist[index];
            penalty[matchnum] = curpenalty;
            penalty_old[matchnum] = curpenalty_old;
        }
    }

    (index < count) ? searchlist[index] = NULL : searchlist[count] = NULL;
}

// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/dirmenu.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"

#include "ui/ui.h"
#include "ui/dirmenu.h"
#include "ui/utils.h"
#include "ui/optsmenu.h"

#include "emuopts.h"
#include "fileio.h"
#include "mame.h"

#include "util/corestr.h"
#include "util/path.h"

#include <locale>


namespace ui {

namespace {

constexpr int ADDING = 1;
constexpr int CHANGE = 2;

struct folders_entry
{
	const char *name;
	const char *option;
	const int   action;
};

const folders_entry f_folders[] =
{
	{ N_p("path-option", "ROMs"),               OPTION_MEDIAPATH,          ADDING },
	{ N_p("path-option", "Software Media"),     OPTION_SWPATH,             CHANGE },
	{ N_p("path-option", "Sound Samples"),      OPTION_SAMPLEPATH,         ADDING },
	{ N_p("path-option", "Artwork"),            OPTION_ARTPATH,            ADDING },
	{ N_p("path-option", "Crosshairs"),         OPTION_CROSSHAIRPATH,      ADDING },
	{ N_p("path-option", "Cheat Files"),        OPTION_CHEATPATH,          ADDING },
	{ N_p("path-option", "Plugins"),            OPTION_PLUGINSPATH,        ADDING },
	{ N_p("path-option", "UI Translations"),    OPTION_LANGUAGEPATH,       CHANGE },
	{ N_p("path-option", "INIs"),               OPTION_INIPATH,            ADDING },
	{ N_p("path-option", "UI Settings"),        OPTION_UI_PATH,            CHANGE },
	{ N_p("path-option", "Plugin Data"),        OPTION_PLUGINDATAPATH,     CHANGE },
	{ N_p("path-option", "DATs"),               OPTION_HISTORY_PATH,       ADDING },
	{ N_p("path-option", "Category INIs"),      OPTION_CATEGORYINI_PATH,   CHANGE },
	{ N_p("path-option", "Snapshots"),          OPTION_SNAPSHOT_DIRECTORY, ADDING },
	{ N_p("path-option", "Icons"),              OPTION_ICONS_PATH,         ADDING },
	{ N_p("path-option", "Control Panels"),     OPTION_CPANELS_PATH,       ADDING },
	{ N_p("path-option", "Cabinets"),           OPTION_CABINETS_PATH,      ADDING },
	{ N_p("path-option", "Marquees"),           OPTION_MARQUEES_PATH,      ADDING },
	{ N_p("path-option", "PCBs"),               OPTION_PCBS_PATH,          ADDING },
	{ N_p("path-option", "Flyers"),             OPTION_FLYERS_PATH,        ADDING },
	{ N_p("path-option", "Title Screens"),      OPTION_TITLES_PATH,        ADDING },
	{ N_p("path-option", "Game Endings"),       OPTION_ENDS_PATH,          ADDING },
	{ N_p("path-option", "Bosses"),             OPTION_BOSSES_PATH,        ADDING },
	{ N_p("path-option", "Artwork Previews"),   OPTION_ARTPREV_PATH,       ADDING },
	{ N_p("path-option", "Select"),             OPTION_SELECT_PATH,        ADDING },
	{ N_p("path-option", "Game Over Screens"),  OPTION_GAMEOVER_PATH,      ADDING },
	{ N_p("path-option", "HowTo"),              OPTION_HOWTO_PATH,         ADDING },
	{ N_p("path-option", "Logos"),              OPTION_LOGOS_PATH,         ADDING },
	{ N_p("path-option", "Scores"),             OPTION_SCORES_PATH,        ADDING },
	{ N_p("path-option", "Versus"),             OPTION_VERSUS_PATH,        ADDING },
	{ N_p("path-option", "Covers"),             OPTION_COVER_PATH,         ADDING }
};


/**************************************************
    MENU REMOVE FOLDER
**************************************************/

class menu_remove_folder : public menu
{
public:
	menu_remove_folder(mame_ui_manager &mui, render_container &container, int ref);

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;

	std::string  m_searchpath;
	int const    m_ref;
	std::vector<std::string> m_folders;
};

//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

menu_remove_folder::menu_remove_folder(mame_ui_manager &mui, render_container &container, int ref)
	: menu(mui, container)
	, m_ref(ref)
{
	set_heading(util::string_format(_("Remove %1$s Folder"), _("path-option", f_folders[m_ref].name)));

	if (mui.options().exists(f_folders[m_ref].option))
		m_searchpath.assign(mui.options().value(f_folders[m_ref].option));
	else
		m_searchpath.assign(mui.machine().options().value(f_folders[m_ref].option));

	path_iterator path(m_searchpath);
	std::string curpath;
	while (path.next(curpath))
		m_folders.push_back(curpath);
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_remove_folder::handle(event const *ev)
{
	// process the menu
	if (ev && ev->itemref && ev->iptkey == IPT_UI_SELECT)
	{
		std::string tmppath, error_string;
		m_folders.erase(m_folders.begin() + selected_index());
		for (int x = 0; x < m_folders.size(); ++x)
		{
			tmppath.append(m_folders[x]);
			if (x < m_folders.size() - 1)
				tmppath.append(";");
		}

		if (ui().options().exists(f_folders[m_ref].option))
			ui().options().set_value(f_folders[m_ref].option, tmppath, OPTION_PRIORITY_CMDLINE);
		else if (machine().options().value(f_folders[m_ref].option) != tmppath)
		{
			machine().options().set_value(f_folders[m_ref].option, tmppath, OPTION_PRIORITY_CMDLINE);
		}

		reset_parent(reset_options::REMEMBER_REF);
		stack_pop();
	}
}

//-------------------------------------------------
//  populate menu
//-------------------------------------------------

void menu_remove_folder::populate(float &customtop, float &custombottom)
{
	int folders_count = 0;
	for (auto & elem : m_folders)
		item_append(elem, 0, (void *)(uintptr_t)++folders_count);

	item_append(menu_item_type::SEPARATOR);
}


/**************************************************
    MENU ADD FOLDER
**************************************************/

class menu_add_change_folder : public menu
{
public:
	menu_add_change_folder(mame_ui_manager &mui, render_container &container, int ref);

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

	virtual bool custom_ui_cancel() override { return !m_search.empty(); }

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;

	int const    m_ref;
	std::string  m_current_path;
	std::string  m_search;
	bool const   m_change;
	std::vector<std::string> m_folders;
};

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_add_change_folder::menu_add_change_folder(mame_ui_manager &mui, render_container &container, int ref)
	: menu(mui, container)
	, m_ref(ref)
	, m_change(f_folders[ref].action == CHANGE)
{
	// configure the starting path
	osd_get_full_path(m_current_path, ".");

	std::string searchpath;
	if (mui.options().exists(f_folders[m_ref].option))
		searchpath = mui.options().value(f_folders[m_ref].option);
	else
		searchpath = mui.machine().options().value(f_folders[m_ref].option);

	path_iterator path(searchpath);
	std::string curpath;
	while (path.next(curpath))
		m_folders.push_back(curpath);
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_add_change_folder::handle(event const *ev)
{
	// process the menu
	if (ev && ev->itemref)
	{
		if (ev->iptkey == IPT_UI_SELECT)
		{
			assert(ev->item);
			menu_item const &pitem = *ev->item;

			// go up to the parent path
			if (pitem.text() == "..")
			{
				size_t const first_sep = m_current_path.find_first_of(PATH_SEPARATOR[0]);
				size_t const last_sep = m_current_path.find_last_of(PATH_SEPARATOR[0]);
				m_current_path.erase(last_sep + ((first_sep == last_sep) ? 1 : 0));
			}
			else
			{
				// if isn't a drive, appends the directory
				if (pitem.subtext() != "[DRIVE]")
					util::path_append(m_current_path, pitem.text());
				else
					m_current_path = pitem.text();
			}

			// reset the char buffer also in this case
			m_search.clear();
			reset(reset_options::SELECT_FIRST);
		}
		else if (ev->iptkey == IPT_SPECIAL)
		{
			bool update_selected = false;

			if (ev->unichar == 0x09)
			{
				// Tab key, save current path
				std::string error_string;
				if (m_change)
				{
					if (ui().options().exists(f_folders[m_ref].option))
						ui().options().set_value(f_folders[m_ref].option, m_current_path, OPTION_PRIORITY_CMDLINE);
					else if (machine().options().value(f_folders[m_ref].option) != m_current_path)
						machine().options().set_value(f_folders[m_ref].option, m_current_path, OPTION_PRIORITY_CMDLINE);
				}
				else
				{
					m_folders.push_back(m_current_path);
					std::string tmppath;
					for (int x = 0; x < m_folders.size(); ++x)
					{
						tmppath.append(m_folders[x]);
						if (x != m_folders.size() - 1)
							tmppath.append(";");
					}

					if (ui().options().exists(f_folders[m_ref].option))
						ui().options().set_value(f_folders[m_ref].option, tmppath, OPTION_PRIORITY_CMDLINE);
					else if (machine().options().value(f_folders[m_ref].option) != tmppath)
						machine().options().set_value(f_folders[m_ref].option, tmppath, OPTION_PRIORITY_CMDLINE);
				}

				reset_parent(reset_options::SELECT_FIRST);
				stack_pop();
			}
			else
			{
				// if it's any other key and we're not maxed out, update
				update_selected = input_character(m_search, ev->unichar, uchar_is_printable);
			}

			// check for entries which matches our search buffer
			if (update_selected)
			{
				const int cur_selected = selected_index();
				int entry, bestmatch = 0;

				// from current item to the end
				for (entry = cur_selected; entry < item_count(); entry++)
					if (item(entry).ref() && !m_search.empty())
					{
						int match = 0;
						for (int i = 0; i < m_search.size() + 1; i++)
						{
							if (core_strnicmp(item(entry).text().c_str(), m_search.data(), i) == 0)
								match = i;
						}

						if (match > bestmatch)
						{
							bestmatch = match;
							set_selected_index(entry);
						}
					}

				// and from the first item to current one
				for (entry = 0; entry < cur_selected; entry++)
				{
					if (item(entry).ref() && !m_search.empty())
					{
						int match = 0;
						for (int i = 0; i < m_search.size() + 1; i++)
						{
							if (core_strnicmp(item(entry).text().c_str(), m_search.data(), i) == 0)
								match = i;
						}

						if (match > bestmatch)
						{
							bestmatch = match;
							set_selected_index(entry);
						}
					}
				}
				centre_selection();
			}
		}
		else if (ev->iptkey == IPT_UI_CANCEL)
		{
			// reset the char buffer also in this case
			m_search.clear();
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_add_change_folder::populate(float &customtop, float &custombottom)
{
	int folders_count = 0;

	// add the drives
	for (std::string const &volume_name : osd_get_volume_names())
		item_append(volume_name, "[DRIVE]", 0, (void *)(uintptr_t)++folders_count);

	// get subdirectories
	std::vector<std::string> dirnames;
	file_enumerator path(m_current_path);
	const osd::directory::entry *dirent;
	while ((dirent = path.next()) != nullptr)
	{
		if ((osd::directory::entry::entry_type::DIR == dirent->type) && strcmp(dirent->name, "."))
			dirnames.emplace_back(dirent->name);
	}

	// sort
	std::collate<wchar_t> const &coll = std::use_facet<std::collate<wchar_t> >(std::locale());
	std::sort(
			dirnames.begin(),
			dirnames.end(),
			[&coll] (std::string const &x, std::string const &y)
			{
				std::wstring const xw = wstring_from_utf8(x);
				std::wstring const yw = wstring_from_utf8(y);
				return coll.compare(xw.data(), xw.data() + xw.size(), yw.data(), yw.data() + yw.size()) < 0;
			});

	// add to menu
	for (std::string const &name : dirnames)
		item_append(name, "[DIR]", 0, (void *)(uintptr_t)++folders_count);

	item_append(menu_item_type::SEPARATOR);

	// configure the custom rendering
	customtop = 2.0f * ui().get_line_height() + 3.0f * ui().box_tb_border();
	custombottom = 1.0f * ui().get_line_height() + 3.0f * ui().box_tb_border();
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_add_change_folder::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	std::string const toptext[] = {
			util::string_format(
					m_change ? _("Change %1$s Folder - Search: %2$s_") : _("Add %1$s Folder - Search: %2$s_"),
					_("path-option", f_folders[m_ref].name),
					m_search),
			m_current_path };
	draw_text_box(
			std::begin(toptext), std::end(toptext),
			origx1, origx2, origy1 - top, origy1 - ui().box_tb_border(),
			text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER, false,
			ui().colors().text_color(), UI_GREEN_COLOR, 1.0f);

	// bottom text
	char const *const bottomtext[] = { _("Press TAB to set") };
	draw_text_box(
			std::begin(bottomtext), std::end(bottomtext),
			origx1, origx2, origy2 + ui().box_tb_border(), origy2 + bottom,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
			ui().colors().text_color(), ui().colors().background_color(), 1.0f);
}


/**************************************************
    MENU DISPLAY PATH
**************************************************/

class menu_display_actual : public menu
{
public:
	menu_display_actual(mame_ui_manager &mui, render_container &container, int selectedref)
		: menu(mui, container)
		, m_heading{ util::string_format((f_folders[selectedref].action == ADDING) ? _("%1$s Folders") : _("%1$s Folder"), _("path-option", f_folders[selectedref].name)) }
		, m_ref(selectedref)
	{
	}

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	enum
	{
		ADD_CHANGE = 1,
		REMOVE,
	};

	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;

	std::string const        m_heading[1];
	std::string              m_searchpath;
	std::vector<std::string> m_folders;
	int const                m_ref;
};

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_display_actual::handle(event const *ev)
{
	// process the menu
	if (ev && ev->itemref && ev->iptkey == IPT_UI_SELECT)
	{
		switch ((uintptr_t)ev->itemref)
		{
		case REMOVE:
			menu::stack_push<menu_remove_folder>(ui(), container(), m_ref);
			break;

		case ADD_CHANGE:
			menu::stack_push<menu_add_change_folder>(ui(), container(), m_ref);
			break;
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_display_actual::populate(float &customtop, float &custombottom)
{
	if (ui().options().exists(f_folders[m_ref].option))
		m_searchpath.assign(ui().options().value(f_folders[m_ref].option));
	else
		m_searchpath.assign(machine().options().value(f_folders[m_ref].option));

	path_iterator path(m_searchpath);
	std::string curpath;
	m_folders.clear();
	while (path.next(curpath))
		m_folders.push_back(curpath);

	item_append((f_folders[m_ref].action == CHANGE) ? _("Change Folder") : _("Add Folder"), 0, (void *)ADD_CHANGE);

	if (m_folders.size() > 1)
		item_append(_("Remove Folder"), 0, (void *)REMOVE);

	item_append(menu_item_type::SEPARATOR);

	customtop = (m_folders.size() + 1) * ui().get_line_height() + 6.0f * ui().box_tb_border();
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_display_actual::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float const lineheight(ui().get_line_height());
	float const maxwidth(draw_text_box(
			std::begin(m_folders), std::end(m_folders),
			origx1, origx2, origy1 - (3.0f * ui().box_tb_border()) - (m_folders.size() * lineheight), origy1 - ui().box_tb_border(),
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
			ui().colors().text_color(), ui().colors().background_color(), 1.0f));
	draw_text_box(
			std::begin(m_heading), std::end(m_heading),
			0.5f * (1.0f - maxwidth), 0.5f * (1.0f + maxwidth), origy1 - top, origy1 - top + lineheight + (2.0f * ui().box_tb_border()),
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
			ui().colors().text_color(), UI_GREEN_COLOR, 1.0f);
}

} // anonymous namespace


/**************************************************
    MENU DIRECTORY
**************************************************/
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

menu_directory::menu_directory(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
	set_heading(_("Configure Folders"));
}

menu_directory::~menu_directory()
{
	ui().save_ui_options();
	ui_globals::reset = true;
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_directory::handle(event const *ev)
{
	// process the menu
	if (ev && ev->itemref && ev->iptkey == IPT_UI_SELECT)
		menu::stack_push<menu_display_actual>(ui(), container(), selected_index());
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_directory::populate(float &customtop, float &custombottom)
{
	for (auto & elem : f_folders)
		item_append(_("path-option", elem.name), 0, (void *)(uintptr_t)elem.action);

	item_append(menu_item_type::SEPARATOR);
}

} // namespace ui

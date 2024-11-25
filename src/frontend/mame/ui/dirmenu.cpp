// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/dirmenu.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/dirmenu.h"

#include "ui/ui.h"
#include "ui/utils.h"

#include "emuopts.h"
#include "fileio.h"
#include "mame.h"

#include "util/corestr.h"
#include "util/path.h"

#include <locale>


namespace ui {

namespace {

struct folders_entry
{
	const char *name;
	const char *option;
};

const folders_entry f_folders[] =
{
	{ N_p("path-option", "ROMs"),               OPTION_MEDIAPATH          },
	{ N_p("path-option", "Software Media"),     OPTION_SWPATH             },
	{ N_p("path-option", "Sound Samples"),      OPTION_SAMPLEPATH         },
	{ N_p("path-option", "Artwork"),            OPTION_ARTPATH            },
	{ N_p("path-option", "Crosshairs"),         OPTION_CROSSHAIRPATH      },
	{ N_p("path-option", "Cheat Files"),        OPTION_CHEATPATH          },
	{ N_p("path-option", "Plugins"),            OPTION_PLUGINSPATH        },
	{ N_p("path-option", "UI Translations"),    OPTION_LANGUAGEPATH       },
	{ N_p("path-option", "Software Lists"),     OPTION_HASHPATH           },
	{ N_p("path-option", "INIs"),               OPTION_INIPATH            },
	{ N_p("path-option", "UI Settings"),        OPTION_UI_PATH            },
	{ N_p("path-option", "Plugin Data"),        OPTION_PLUGINDATAPATH     },
	{ N_p("path-option", "DATs"),               OPTION_HISTORY_PATH       },
	{ N_p("path-option", "Category INIs"),      OPTION_CATEGORYINI_PATH   },
	{ N_p("path-option", "Snapshots"),          OPTION_SNAPSHOT_DIRECTORY },
	{ N_p("path-option", "Icons"),              OPTION_ICONS_PATH         },
	{ N_p("path-option", "Control Panels"),     OPTION_CPANELS_PATH       },
	{ N_p("path-option", "Cabinets"),           OPTION_CABINETS_PATH      },
	{ N_p("path-option", "Marquees"),           OPTION_MARQUEES_PATH      },
	{ N_p("path-option", "PCBs"),               OPTION_PCBS_PATH          },
	{ N_p("path-option", "Flyers"),             OPTION_FLYERS_PATH        },
	{ N_p("path-option", "Title Screens"),      OPTION_TITLES_PATH        },
	{ N_p("path-option", "Game Endings"),       OPTION_ENDS_PATH          },
	{ N_p("path-option", "Bosses"),             OPTION_BOSSES_PATH        },
	{ N_p("path-option", "Artwork Previews"),   OPTION_ARTPREV_PATH       },
	{ N_p("path-option", "Select"),             OPTION_SELECT_PATH        },
	{ N_p("path-option", "Game Over Screens"),  OPTION_GAMEOVER_PATH      },
	{ N_p("path-option", "HowTo"),              OPTION_HOWTO_PATH         },
	{ N_p("path-option", "Logos"),              OPTION_LOGOS_PATH         },
	{ N_p("path-option", "Scores"),             OPTION_SCORES_PATH        },
	{ N_p("path-option", "Versus"),             OPTION_VERSUS_PATH        },
	{ N_p("path-option", "Covers"),             OPTION_COVER_PATH         }
};


/**************************************************
    MENU REMOVE FOLDER
**************************************************/

class menu_remove_folder : public menu
{
public:
	menu_remove_folder(mame_ui_manager &mui, render_container &container, int ref);

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

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

bool menu_remove_folder::handle(event const *ev)
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
			machine().options().set_value(f_folders[m_ref].option, tmppath, OPTION_PRIORITY_CMDLINE);

		reset_parent(reset_options::REMEMBER_REF);
		stack_pop();
	}

	return false;
}

//-------------------------------------------------
//  populate menu
//-------------------------------------------------

void menu_remove_folder::populate()
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
	menu_add_change_folder(mame_ui_manager &mui, render_container &container, int ref, bool multipath);

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;

	virtual bool custom_ui_back() override { return !m_search.empty(); }

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	void update_search();

	int const    m_ref;
	bool const   m_multipath;
	std::string  m_current_path;
	std::string  m_search;
	std::vector<std::string> m_folders;
};

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_add_change_folder::menu_add_change_folder(mame_ui_manager &mui, render_container &container, int ref, bool multipath)
	: menu(mui, container)
	, m_ref(ref)
	, m_multipath(multipath)
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

bool menu_add_change_folder::handle(event const *ev)
{
	if (!ev || !ev->itemref)
		return false;

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
	else if (ev->iptkey == IPT_UI_PASTE)
	{
		if (paste_text(m_search, uchar_is_printable))
		{
			update_search();
			return true;
		}
	}
	else if (ev->iptkey == IPT_SPECIAL)
	{
		if (ev->unichar == 0x09)
		{
			// Tab key, save current path
			std::string error_string;
			if (!m_multipath)
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
		else if (input_character(m_search, ev->unichar, uchar_is_printable))
		{
			// if it's any other key and we're not maxed out, update
			update_search();
			return true;
		}
	}
	else if (ev->iptkey == IPT_UI_CANCEL)
	{
		// reset the char buffer also in this case
		m_search.clear();
		return true;
	}

	return false;
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_add_change_folder::populate()
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
	std::locale const lcl;
	std::collate<wchar_t> const &coll = std::use_facet<std::collate<wchar_t> >(lcl);
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
}

//-------------------------------------------------
//  recompute metrics
//-------------------------------------------------

void menu_add_change_folder::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);

	// configure the custom rendering
	set_custom_space(2.0f * line_height() + 3.0f * tb_border(), 1.0f * line_height() + 3.0f * tb_border());
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_add_change_folder::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	std::string const toptext[] = {
			util::string_format(
					m_multipath ? _("Add %1$s Folder - Search: %2$s_") : _("Change %1$s Folder - Search: %2$s_"),
					_("path-option", f_folders[m_ref].name),
					m_search),
			m_current_path };
	draw_text_box(
			std::begin(toptext), std::end(toptext),
			origx1, origx2, origy1 - top, origy1 - tb_border(),
			text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER, false,
			ui().colors().text_color(), UI_GREEN_COLOR);

	// bottom text
	char const *const bottomtext[] = { _("Press TAB to set") };
	draw_text_box(
			std::begin(bottomtext), std::end(bottomtext),
			origx1, origx2, origy2 + tb_border(), origy2 + bottom,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
			ui().colors().text_color(), ui().colors().background_color());
}

//-------------------------------------------------
//  update search
//-------------------------------------------------

void menu_add_change_folder::update_search()
{
	// check for entries which matches our search buffer
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


/**************************************************
    MENU DISPLAY PATH
**************************************************/

class menu_display_actual : public menu
{
public:
	menu_display_actual(mame_ui_manager &mui, render_container &container, int selectedref)
		: menu(mui, container)
		, m_ref(selectedref)
		, m_multipath(is_multipath(f_folders[selectedref].option))
		, m_heading{ util::string_format(m_multipath ? _("%1$s Folders") : _("%1$s Folder"), _("path-option", f_folders[selectedref].name)) }
	{
	}

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;

private:
	enum
	{
		ADD_CHANGE = 1,
		REMOVE,
	};

	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	bool is_multipath(std::string_view folder) const;

	int const                m_ref;
	bool const               m_multipath;
	std::string const        m_heading[1];
	std::string              m_searchpath;
	std::vector<std::string> m_folders;
};

//-------------------------------------------------
//  is_multipath
//-------------------------------------------------

bool menu_display_actual::is_multipath(std::string_view folder) const
{
	auto option = ui().options().get_entry(folder);
	if (!option)
		option = machine().options().get_entry(folder);
	assert(option);

	return option->type() == core_options::option_type::MULTIPATH;
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_display_actual::handle(event const *ev)
{
	if (ev && ev->itemref && ev->iptkey == IPT_UI_SELECT)
	{
		switch ((uintptr_t)ev->itemref)
		{
		case REMOVE:
			menu::stack_push<menu_remove_folder>(ui(), container(), m_ref);
			break;

		case ADD_CHANGE:
			menu::stack_push<menu_add_change_folder>(ui(), container(), m_ref, m_multipath);
			break;
		}
	}

	return false;
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_display_actual::populate()
{
	auto const &folder = f_folders[m_ref];
	auto option = ui().options().get_entry(folder.option);
	if (!option)
		option = machine().options().get_entry(folder.option);
	assert(option);

	m_searchpath.assign(option->value());

	m_folders.clear();
	if (m_multipath)
	{
		path_iterator path(m_searchpath);
		std::string curpath;
		while (path.next(curpath))
			m_folders.push_back(curpath);

		item_append(_("Add Folder"), 0, (void *)ADD_CHANGE);
		if (m_folders.size() > 1)
			item_append(_("Remove Folder"), 0, (void *)REMOVE);
	}
	else
	{
		m_folders.push_back(m_searchpath);
		item_append(_("Change Folder"), 0, (void *)ADD_CHANGE);
	}

	item_append(menu_item_type::SEPARATOR);

	set_custom_space((m_folders.size() + 1) * line_height() + 6.0f * tb_border(), 0.0f);
}

//-------------------------------------------------
//  recompute metrics
//-------------------------------------------------

void menu_display_actual::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);

	set_custom_space((m_folders.size() + 1) * line_height() + 6.0f * tb_border(), 0.0f);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_display_actual::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float const maxwidth(draw_text_box(
			std::begin(m_folders), std::end(m_folders),
			origx1, origx2, origy1 - (3.0f * tb_border()) - (m_folders.size() * line_height()), origy1 - tb_border(),
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
			ui().colors().text_color(), ui().colors().background_color()));
	draw_text_box(
			std::begin(m_heading), std::end(m_heading),
			0.5f * (1.0f - maxwidth), 0.5f * (1.0f + maxwidth), origy1 - top, origy1 - top + line_height() + (2.0f * tb_border()),
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
			ui().colors().text_color(), UI_GREEN_COLOR);
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

bool menu_directory::handle(event const *ev)
{
	if (ev && ev->itemref && ev->iptkey == IPT_UI_SELECT)
		menu::stack_push<menu_display_actual>(ui(), container(), selected_index());

	return false;
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_directory::populate()
{
	for (auto & elem : f_folders)
		item_append(_("path-option", elem.name), 0, this); // need a non-null reference pointer - value is immaterial

	item_append(menu_item_type::SEPARATOR);
}

} // namespace ui

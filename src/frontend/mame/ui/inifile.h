// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota, Vas Crabb
/***************************************************************************

    ui/inifile.h

    UI INIs file manager.

***************************************************************************/
#ifndef MAME_FRONTEND_UI_INIFILE_H
#define MAME_FRONTEND_UI_INIFILE_H

#pragma once

#include "ui/utils.h"

#include <functional>
#include <set>
#include <tuple>
#include <type_traits>
#include <unordered_set>


//-------------------------------------------------
//  INIFILE MANAGER
//-------------------------------------------------

class ui_options;

class inifile_manager
{
public:
	// construction/destruction
	inifile_manager(ui_options &options);

	// load systems from category
	void load_ini_category(size_t file, size_t category, std::unordered_set<game_driver const *> &result) const;

	// getters
	size_t get_file_count() const { return m_ini_index.size(); }
	std::string const &get_file_name(size_t file) const { return m_ini_index[file].first; }
	size_t get_category_count(size_t file) const { return m_ini_index[file].second.size(); }
	std::string const &get_category_name(size_t file, size_t category) const { return m_ini_index[file].second[category].first; }

private:
	// ini file structure
	using categoryindex = std::vector<std::pair<std::string, int64_t>>;

	void init_category(std::string &&filename, util::core_file &file);

	// internal state
	ui_options &m_options;
	std::vector<std::pair<std::string, categoryindex> > m_ini_index;

};

//-------------------------------------------------
//  FAVORITE MANAGER
//-------------------------------------------------

class favorite_manager
{
public:
	// construction/destruction
	favorite_manager(ui_options &options);

	// add
	void add_favorite_system(game_driver const &driver);
	void add_favorite_software(ui_software_info const &swinfo);
	void add_favorite(running_machine &machine);

	// check
	bool is_favorite_system(game_driver const &driver) const;
	bool is_favorite_software(ui_software_info const &swinfo) const;
	bool is_favorite_system_software(ui_software_info const &swinfo) const;
	bool is_favorite(running_machine &machine) const;

	// remove
	void remove_favorite_system(game_driver const &driver);
	void remove_favorite_software(ui_software_info const &swinfo);
	void remove_favorite(running_machine &machine);

	// walk
	template <typename T> void apply(T &&action)
	{
		for (auto const &item : m_favorites)
			action(item);
	}
	template <typename T> void apply_sorted(T &&action)
	{
		update_sorted();
		for (auto const &item : m_sorted)
			action(item.get());
	}

private:
	using running_software_key = std::tuple<game_driver const &, char const *, std::string const &>;

	struct favorite_compare
	{
		using is_transparent = std::true_type;

		bool operator()(ui_software_info const &lhs, ui_software_info const &rhs) const;
		bool operator()(ui_software_info const &lhs, game_driver const &rhs) const;
		bool operator()(game_driver const &lhs, ui_software_info const &rhs) const;
		bool operator()(ui_software_info const &lhs, running_software_key const &rhs) const;
		bool operator()(running_software_key const &lhs, ui_software_info const &rhs) const;
	};

	using favorites_set = std::set<ui_software_info, favorite_compare>;
	using sorted_favorites = std::vector<std::reference_wrapper<ui_software_info const> >;

	// implementation
	template <typename T> static void apply_running_machine(running_machine &machine, T &&action);
	template <typename T> void add_impl(T &&key);
	template <typename T> bool check_impl(T const &key) const;
	template <typename T> bool remove_impl(T const &key);
	void update_sorted();
	void save_favorites();

	// internal state
	ui_options &m_options;
	favorites_set m_favorites;
	sorted_favorites m_sorted;
	bool m_need_sort;
};

#endif  // MAME_FRONTEND_UI_INIFILE_H

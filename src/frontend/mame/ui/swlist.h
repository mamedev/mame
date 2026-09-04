// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/swlist.h

    Internal MAME user interface for software list.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_SWLIST_H
#define MAME_FRONTEND_UI_SWLIST_H

#include "ui/menu.h"

#include <functional>
#include <list>
#include <string>
#include <string_view>


namespace ui {

// ======================> menu_software_parts

class menu_software_parts : public menu
{
public:
	enum class result
	{
		EMPTY,
		FMGR,
		SWLIST,
		ENTRY
	};

	using handler_function = std::function<void (result, software_part const *)>;

	menu_software_parts(
			mame_ui_manager &mui,
			render_target &target,
			software_info const &info,
			char const *interface,
			bool other_opt,
			handler_function &&handler);
	virtual ~menu_software_parts() override;

private:
	struct software_part_menu_entry
	{
		result type;
		const software_part *part;
	};
	using entry_list = std::list<software_part_menu_entry>;

	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	// variables
	handler_function const  m_handler;
	entry_list              m_entries;
	software_info const &   m_info;
	char const *const       m_interface;
	bool                    m_other_opt;
};


// ======================> menu_software_list

class menu_software_list : public menu
{
public:
	using handler_function = std::function<void (std::string_view)>;

	menu_software_list(
			mame_ui_manager &mui,
			render_target &target,
			software_list_device &swlist,
			char const *interface,
			handler_function &&handler);
	virtual ~menu_software_list() override;

protected:
	virtual bool custom_ui_back() override { return !m_search.empty(); }

private:
	struct entry_info
	{
		entry_info() = default;
		entry_info(entry_info const &) = default;
		entry_info(entry_info &&) = default;
		entry_info &operator=(entry_info const &) = default;
		entry_info &operator=(entry_info &&) = default;

		std::string short_name;
		std::string long_name;
	};

	// variables
	handler_function const  m_handler;
	software_list_device &  m_swlist; // currently selected list
	char const *const       m_interface;
	std::list<entry_info>   m_entrylist;
	std::string             m_search;
	bool                    m_ordered_by_shortname;

	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	// functions
	void append_software_entry(software_info const &swinfo);
	void update_search(void *selectedref);
};


// ======================> menu_software

class menu_software : public menu
{
public:
	using handler_function = std::function<void (software_list_device *)>;

	menu_software(
			mame_ui_manager &mui,
			render_target &target,
			char const *interface,
			handler_function &&handler);
	virtual ~menu_software() override;
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

private:
	char const *const       m_interface;
	handler_function const  m_handler;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_SWLIST_H

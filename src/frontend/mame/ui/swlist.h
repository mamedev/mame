// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/swlist.h

    Internal MAME user interface for software list.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_SWLIST_H
#define MAME_FRONTEND_UI_SWLIST_H

#include "ui/menu.h"

#include <list>


namespace ui {

// ======================> menu_software_parts

class menu_software_parts : public menu
{
public:
	enum class result
	{
		INVALID = -1,
		EMPTY = 0x2000,
		FMGR,
		SWLIST,
		ENTRY
	};

	menu_software_parts(mame_ui_manager &mui, render_container &container, const software_info *info, const char *interface, const software_part **part, bool other_opt, result &result);
	virtual ~menu_software_parts() override;

private:
	struct software_part_menu_entry
	{
		result type;
		const software_part *part;
	};
	using entry_list = std::list<software_part_menu_entry>;

	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;

	// variables
	entry_list              m_entries;
	const software_info *   m_info;
	const char *            m_interface;
	const software_part **  m_selected_part;
	bool                    m_other_opt;
	result &                m_result;
};


// ======================> menu_software_list

class menu_software_list : public menu
{
public:
	menu_software_list(mame_ui_manager &mui, render_container &container, software_list_device *swlist, const char *interface, std::string &result);
	virtual ~menu_software_list() override;
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;

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
	software_list_device *          m_swlist; // currently selected list
	const char *                    m_interface;
	std::string &                   m_result;
	std::list<entry_info>           m_entrylist;
	std::string                     m_search;
	bool                            m_ordered_by_shortname;

	// functions
	void append_software_entry(const software_info &swinfo);
};


// ======================> menu_software

class menu_software : public menu
{
public:
	menu_software(mame_ui_manager &mui, render_container &container, const char *interface, software_list_device **result);
	virtual ~menu_software() override;
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;

private:
	const char *                    m_interface;
	software_list_device **         m_result;
};

} // namespace ui

#endif  /* MAME_FRONTEND_UI_SWLIST_H */

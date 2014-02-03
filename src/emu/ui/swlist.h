/***************************************************************************

    ui/swlist.h

    Internal MAME user interface for software list.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef __UI_SWLIST_H__
#define __UI_SWLIST_H__

// ======================> ui_menu_software_parts

class ui_menu_software_parts : public ui_menu {
public:
	enum { T_ENTRY, T_FMGR };
	ui_menu_software_parts(running_machine &machine, render_container *container, const software_list_device *swlist, const struct software_list *swl, const software_info *info, device_image_interface *image);	virtual ~ui_menu_software_parts();
	virtual void populate();
	virtual void handle();

private:
	struct software_part_menu_entry {
		int type;
		const software_part *part;
	};

	// variables
	const software_list_device *	m_swlist;
	const struct software_list *	m_software_list;
	const software_info	*			m_info;
	device_image_interface *		m_image;
};


// ======================> ui_menu_software_list

class ui_menu_software_list : public ui_menu {
public:
	ui_menu_software_list(running_machine &machine, render_container *container, const software_list_device *swlist, device_image_interface *image);
	virtual ~ui_menu_software_list();
	virtual void populate();
	virtual void handle();

private:
	struct entry_info {
		entry_info *next;

		const char *short_name;
		const char *long_name;
	};

	// variables
	const software_list_device *	m_swlist; // currently selected list
	device_image_interface *		m_image;
	entry_info *					m_entrylist;
	char							m_filename_buffer[1024];
	bool							m_ordered_by_shortname;

	// functions
	int compare_entries(const entry_info *e1, const entry_info *e2, bool shortname);
	entry_info *append_software_entry(const software_info *swinfo);
	void select_entry(entry_info *entry);
};


#endif  /* __UI_SWLIST_H__ */

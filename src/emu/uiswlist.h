/***************************************************************************

    uiswlist.h

    Internal MAME user interface for software list.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef __UISWLIST_H__
#define __UISWLIST_H__

class ui_menu_software_parts : public ui_menu {
public:
	enum { T_ENTRY, T_FMGR };
	ui_menu_software_parts(running_machine &machine, render_container *container, const software_info *info, const char *interface, const software_part **part, bool opt_fmgr, int *result);
	virtual ~ui_menu_software_parts();
	virtual void populate();
	virtual void handle();

private:
	struct software_part_menu_entry {
		int type;
		const software_part *part;
	};

	const software_info *info;
	const char *interface;
	const software_part **selected_part;
	bool opt_fmgr;
	int *result;
};

class ui_menu_software_list : public ui_menu {
public:
	ui_menu_software_list(running_machine &machine, render_container *container, const software_list_device *swlist, const char *interface, astring &result);
	virtual ~ui_menu_software_list();
	virtual void populate();
	virtual void handle();

private:
	struct entry_info {
		entry_info *next;

		const char *short_name;
		const char *long_name;
	};

	const software_list_device *swlist; /* currently selected list */
	const char *interface;
	astring &result;
	entry_info *entrylist;
	char filename_buffer[1024];
	bool ordered_by_shortname;

	int compare_entries(const entry_info *e1, const entry_info *e2, bool shortname);
	entry_info *append_software_entry(const software_info *swinfo);
};

class ui_menu_software : public ui_menu {
public:
	ui_menu_software(running_machine &machine, render_container *container, const char *interface, const software_list_device **result);
	virtual ~ui_menu_software();
	virtual void populate();
	virtual void handle();

private:
	const char *interface;
	const software_list_device **result;
};

#endif  /* __UISWLIST_H__ */

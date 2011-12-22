/***************************************************************************

    uiswlist.h

    Internal MAME user interface for software list.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef __UISWLIST_H__
#define __UISWLIST_H__

struct ui_menu_software_entry_info {
	ui_menu_software_entry_info *next;

	const char *short_name;
	const char *long_name;
	const char *interface;
	char *list_name;
	device_image_interface* image;
};

class ui_menu_software_parts : public ui_menu {
public:
	ui_menu_software_parts(running_machine &machine, render_container *container, ui_menu_software_entry_info *entry);
	virtual ~ui_menu_software_parts();
	virtual void populate();
	virtual void handle();

private:
	struct software_part_info {
		const char *part_name;
		const char *interface;
	};

	ui_menu_software_entry_info *entry;
};

class ui_menu_software_list : public ui_menu {
public:
	ui_menu_software_list(running_machine &machine, render_container *container, software_list_config *swlist, device_image_interface *image);
	virtual ~ui_menu_software_list();
	virtual void populate();
	virtual void handle();

private:
	software_list_config *swlist;	/* currently selected list */
	device_image_interface *image;
	ui_menu_software_entry_info *entrylist;
	char filename_buffer[1024];
	bool ordered_by_shortname;

	int compare_entries(const ui_menu_software_entry_info *e1, const ui_menu_software_entry_info *e2, bool shortname);
	ui_menu_software_entry_info *append_software_entry(software_info *swinfo, device_image_interface* image);
	bool swinfo_has_multiple_parts(software_info *swinfo, const char *interface);
	bool if_compatible(const char *compatibility, const char *filter);
};

class ui_menu_software : public ui_menu {
public:
	ui_menu_software(running_machine &machine, render_container *container, device_image_interface *device);
	virtual ~ui_menu_software();
	virtual void populate();
	virtual void handle();

private:
	device_image_interface *image;
};

#endif	/* __UISWLIST_H__ */

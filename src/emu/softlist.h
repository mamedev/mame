/*********************************************************************

    softlist.h

    Software and software list information.

*********************************************************************/

#ifndef __SOFTLIST_H_
#define __SOFTLIST_H_

#include "uimenu.h"


/*********************************************************************

    Internal structures and XML file handling

*********************************************************************/

/* Replace this with list<string>? */
struct feature_list
{
	feature_list	*next;
	char			*name;
	char			*value;
};

struct software_part
{
	const char *name;
	const char *interface_;
	feature_list *featurelist;
	struct rom_entry *romdata;
};


/* The software info struct holds basic software information. Additional,
   optional information like local software names, release dates, serial
   numbers, etc can be maintained and stored in external recources.
*/
struct software_info
{
	const char *shortname;
	const char *longname;
	const char *parentname;
	const char *year;			/* Copyright year on title screen, actual release dates can be tracked in external resources */
	const char *publisher;
	UINT32 supported;
	software_part *partdata;
	struct software_info *next;	/* Used internally */
};


typedef struct _software_list software_list;

/* Handling a software list */
software_list *software_list_open(core_options *options, const char *listname, int is_preload, void (*error_proc)(const char *message));
void software_list_close(software_list *swlist);
software_info *software_list_find(software_list *swlist, const char *look_for, software_info *prev);

software_part *software_find_part(software_info *sw, const char *partname, const char *interface_);
software_part *software_part_next(software_part *part);


bool load_software_part(device_image_interface *image, const char *path, software_info **sw_info, software_part **sw_part, char **full_sw_name);

void ui_image_menu_software(running_machine *machine, ui_menu *menu, void *parameter, void *state);


/*********************************************************************

    Driver software list configuration

*********************************************************************/
DECLARE_LEGACY_DEVICE(SOFTWARE_LIST, software_list);

#define SOFTWARE_SUPPORTED_YES		0
#define SOFTWARE_SUPPORTED_PARTIAL	1
#define SOFTWARE_SUPPORTED_NO		2


#define SOFTWARE_LIST_CONFIG_SIZE	10


typedef struct _software_list_config software_list_config;
struct _software_list_config
{
	char *list_name[SOFTWARE_LIST_CONFIG_SIZE];
};


#define DEVINFO_STR_SWLIST_0	(DEVINFO_STR_DEVICE_SPECIFIC+0)
#define DEVINFO_STR_SWLIST_MAX	(DEVINFO_STR_SWLIST_0 + SOFTWARE_LIST_CONFIG_SIZE - 1)


#define MDRV_SOFTWARE_LIST_CONFIG(_idx,_list)								\
	MDRV_DEVICE_CONFIG_DATAPTR_ARRAY(software_list_config, list_name, _idx, _list)

#define MDRV_SOFTWARE_LIST_ADD( _tag, _list )										\
	MDRV_DEVICE_ADD( _tag, SOFTWARE_LIST, 0 )				\
	MDRV_SOFTWARE_LIST_CONFIG(0,_list)


#define MDRV_SOFTWARE_LIST_MODIFY( _tag, _list )									\
	MDRV_DEVICE_MODIFY( _tag )								\
	MDRV_SOFTWARE_LIST_CONFIG(0,_list)


#endif

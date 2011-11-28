/*********************************************************************

    softlist.h

    Software and software list information.

*********************************************************************/

#ifndef __SOFTLIST_H_
#define __SOFTLIST_H_

#include "uimenu.h"
#include "expat.h"
#include "pool.h"


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


/* The software info struct holds basic software information. */
struct software_info
{
	const char *shortname;
	const char *longname;
	const char *parentname;
	const char *year;			// Copyright year on title screen, actual release dates can be tracked in external resources
	const char *publisher;
	feature_list *other_info;	// Here we store info like developer, serial #, etc. which belong to the software entry as a whole
	feature_list *shared_info;	// Here we store info like TV standard compatibility, or add-on requirements, etc. which get inherited
								// by each part of this software entry (after loading these are stored in partdata->featurelist)
	UINT32 supported;
	software_part *partdata;
	struct software_info *next;	// Used internally
};


enum softlist_parse_position
{
	POS_ROOT,
	POS_MAIN,
	POS_SOFT,
	POS_PART,
	POS_DATA
};


typedef struct _parse_state parse_state;
struct _parse_state
{
	XML_Parser	parser;
	int			done;

	void (*error_proc)(const char *message);
	void *param;

	enum softlist_parse_position pos;
	char **text_dest;
};


typedef struct _software_list software_list;
struct _software_list
{
	emu_file	*file;
	object_pool	*pool;
	parse_state	state;
	const char *description;
	struct software_info	*software_info_list;
	struct software_info	*current_software_info;
	software_info	*softinfo;
	const char *look_for;
	int part_entries;
	int current_part_entry;
	int rom_entries;
	int current_rom_entry;
	void (*error_proc)(const char *message);
	int list_entries;
};

/* Handling a software list */
software_list *software_list_open(emu_options &options, const char *listname, int is_preload, void (*error_proc)(const char *message));
void software_list_close(software_list *swlist);
software_info *software_list_find(software_list *swlist, const char *look_for, software_info *prev);
const char *software_list_get_description(software_list *swlist);
void software_list_parse(software_list *swlist, void (*error_proc)(const char *message), void *param);

software_part *software_find_part(software_info *sw, const char *partname, const char *interface_);
software_part *software_part_next(software_part *part);

/* helpers */
const char *software_get_clone(emu_options &options, char *swlist, const char *swname);
UINT32 software_get_support(emu_options &options, char *swlist, const char *swname);
const char *software_part_get_feature(software_part *part, const char *feature_name);

bool load_software_part(emu_options &options, device_image_interface *image, const char *path, software_info **sw_info, software_part **sw_part, char **full_sw_name);

void software_display_matches(const device_list &devlist, emu_options &options,const char *interface,const char *swname_bckp);

const char *software_get_default_slot(const device_list &devlist, emu_options &options, const device_image_interface *image, const char* default_card_slot);

void validate_softlists(emu_options &options);
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
	UINT32 list_type;
};


#define DEVINFO_STR_SWLIST_0	(DEVINFO_STR_DEVICE_SPECIFIC+0)
#define DEVINFO_STR_SWLIST_MAX	(DEVINFO_STR_SWLIST_0 + SOFTWARE_LIST_CONFIG_SIZE - 1)

#define SOFTWARE_LIST_ORIGINAL_SYSTEM		0
#define SOFTWARE_LIST_COMPATIBLE_SYSTEM		1

#define MCFG_SOFTWARE_LIST_CONFIG(_idx,_list,_list_type)								\
	MCFG_DEVICE_CONFIG_DATAPTR_ARRAY(software_list_config, list_name, _idx, _list)	\
	MCFG_DEVICE_CONFIG_DATA32(software_list_config, list_type, _list_type)

#define MCFG_SOFTWARE_LIST_ADD( _tag, _list )										\
	MCFG_DEVICE_ADD( _tag, SOFTWARE_LIST, 0 )				\
	MCFG_SOFTWARE_LIST_CONFIG(0,_list, SOFTWARE_LIST_ORIGINAL_SYSTEM)


#define MCFG_SOFTWARE_LIST_COMPATIBLE_ADD( _tag, _list )										\
	MCFG_DEVICE_ADD( _tag, SOFTWARE_LIST, 0 )				\
	MCFG_SOFTWARE_LIST_CONFIG(0,_list, SOFTWARE_LIST_COMPATIBLE_SYSTEM)


#define MCFG_SOFTWARE_LIST_MODIFY( _tag, _list )									\
	MCFG_DEVICE_MODIFY( _tag )								\
	MCFG_SOFTWARE_LIST_CONFIG(0,_list, SOFTWARE_LIST_ORIGINAL_SYSTEM)

#define MCFG_SOFTWARE_LIST_COMPATIBLE_MODIFY( _tag, _list )									\
	MCFG_DEVICE_MODIFY( _tag )								\
	MCFG_SOFTWARE_LIST_CONFIG(0,_list, SOFTWARE_LIST_COMPATIBLE_SYSTEM)

#endif

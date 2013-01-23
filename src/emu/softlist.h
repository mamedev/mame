/*********************************************************************

    softlist.h

    Software and software list information.

*********************************************************************/

#ifndef __SOFTLIST_H_
#define __SOFTLIST_H_

#include "uimenu.h"
#include "expat.h"
#include "pool.h"



#define SOFTWARE_SUPPORTED_YES      0
#define SOFTWARE_SUPPORTED_PARTIAL  1
#define SOFTWARE_SUPPORTED_NO       2

enum softlist_type
{
	SOFTWARE_LIST_ORIGINAL_SYSTEM,
	SOFTWARE_LIST_COMPATIBLE_SYSTEM
};

#define MCFG_SOFTWARE_LIST_CONFIG(_list,_list_type) \
	software_list_device::static_set_config(*device, _list, _list_type);

#define MCFG_SOFTWARE_LIST_ADD( _tag, _list ) \
	MCFG_DEVICE_ADD( _tag, SOFTWARE_LIST, 0 ) \
	MCFG_SOFTWARE_LIST_CONFIG(_list, SOFTWARE_LIST_ORIGINAL_SYSTEM)

#define MCFG_SOFTWARE_LIST_COMPATIBLE_ADD( _tag, _list ) \
	MCFG_DEVICE_ADD( _tag, SOFTWARE_LIST, 0 ) \
	MCFG_SOFTWARE_LIST_CONFIG(_list, SOFTWARE_LIST_COMPATIBLE_SYSTEM)

#define MCFG_SOFTWARE_LIST_MODIFY( _tag, _list ) \
	MCFG_DEVICE_MODIFY( _tag ) \
	MCFG_SOFTWARE_LIST_CONFIG(_list, SOFTWARE_LIST_ORIGINAL_SYSTEM)

#define MCFG_SOFTWARE_LIST_COMPATIBLE_MODIFY( _tag, _list ) \
	MCFG_DEVICE_MODIFY( _tag ) \
	MCFG_SOFTWARE_LIST_CONFIG(_list, SOFTWARE_LIST_COMPATIBLE_SYSTEM)

#define MCFG_SOFTWARE_LIST_FILTER( _tag, _filter ) \
	MCFG_DEVICE_MODIFY( _tag ) \
	software_list_device::static_set_filter(*device, _filter);

#define MCFG_SOFTWARE_LIST_REMOVE( _tag ) \
	MCFG_DEVICE_REMOVE( _tag )


// ======================> software_list_device

class software_list_device : public device_t
{
public:
	// construction/destruction
	software_list_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_config(device_t &device, const char *list, softlist_type list_type);
	static void static_set_filter(device_t &device, const char *filter);

	// getters
	const char *list_name() const { return m_list_name; }
	softlist_type list_type() const { return m_list_type; }
	const char *filter() const { return m_filter; }

	// validation helpers
	static void reset_checked_lists() { s_checked_lists.reset(); }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_validity_check(validity_checker &valid) const ATTR_COLD;

	// configuration state
	const char *                m_list_name;
	softlist_type               m_list_type;
	const char *                m_filter;

	// static state
	static tagmap_t<UINT8>      s_checked_lists;
};


// device type definition
extern const device_type SOFTWARE_LIST;

// device type iterator
typedef device_type_iterator<&device_creator<software_list_device>, software_list_device> software_list_device_iterator;



/*********************************************************************

    Internal structures and XML file handling

*********************************************************************/

/* Replace this with list<string>? */
struct feature_list
{
	feature_list    *next;
	char            *name;
	char            *value;
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
	const char *year;           // Copyright year on title screen, actual release dates can be tracked in external resources
	const char *publisher;
	feature_list *other_info;   // Here we store info like developer, serial #, etc. which belong to the software entry as a whole
	feature_list *shared_info;  // Here we store info like TV standard compatibility, or add-on requirements, etc. which get inherited
								// by each part of this software entry (after loading these are stored in partdata->featurelist)
	UINT32 supported;
	int part_entries;
	int current_part_entry;
	software_part *partdata;
	struct software_info *next; // Used internally
};


enum softlist_parse_position
{
	POS_ROOT,
	POS_MAIN,
	POS_SOFT,
	POS_PART,
	POS_DATA
};


struct parse_state
{
	XML_Parser  parser;
	int         done;

	void (*error_proc)(const char *message);
	void *param;

	enum softlist_parse_position pos;
	char **text_dest;
};


struct software_list
{
	emu_file    *file;
	object_pool *pool;
	parse_state state;
	const char *description;
	struct software_info    *software_info_list;
	struct software_info    *current_software_info;
	software_info   *softinfo;
	const char *look_for;
	int rom_entries;
	int current_rom_entry;
	void (*error_proc)(const char *message);
	int list_entries;
};

/* Handling a software list */
software_list *software_list_open(emu_options &options, const char *listname, int is_preload, void (*error_proc)(const char *message));
void software_list_close(const software_list *swlist);
software_info *software_list_find(software_list *swlist, const char *look_for, software_info *prev);
const char *software_list_get_description(const software_list *swlist);
void software_list_parse(software_list *swlist, void (*error_proc)(const char *message), void *param);

software_part *software_find_part(software_info *sw, const char *partname, const char *interface_);
software_part *software_part_next(software_part *part);

const software_info *software_list_find(const software_list *swlist, const char *look_for, const software_info *prev);
const software_part *software_find_part(const software_info *sw, const char *partname, const char *interface_);
const software_part *software_part_next(const software_part *part);

/* helpers */
const char *software_get_clone(emu_options &options, char *swlist, const char *swname);
UINT32 software_get_support(emu_options &options, char *swlist, const char *swname);
const char *software_part_get_feature(const software_part *part, const char *feature_name);
void software_name_split(const char *swlist_swname, char **swlist_name, char **swname, char **swpart);

bool load_software_part(emu_options &options, device_image_interface *image, const char *path, software_info **sw_info, software_part **sw_part, char **full_sw_name, char**list_name);

void software_display_matches(const machine_config &config, emu_options &options,const char *interface,const char *swname_bckp);

const char *software_get_default_slot(const machine_config &config, emu_options &options, const device_image_interface *image, const char* default_card_slot);

bool is_software_compatible(const software_part *swpart, const software_list_device *swlist);
bool swinfo_has_multiple_parts(const software_info *swinfo, const char *interface);

bool softlist_contain_interface(const char *interface, const char *part_interface);
#endif

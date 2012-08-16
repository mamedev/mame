/***************************************************************************

    softlist.c

    Software list construction helpers.


***************************************************************************/

#include "emu.h"
#include "pool.h"
#include "emuopts.h"
#include "softlist.h"
#include "clifront.h"

#include <ctype.h>

typedef tagmap_t<software_info *> softlist_map;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

tagmap_t<UINT8> software_list_device::s_checked_lists;

// device type definition
const device_type SOFTWARE_LIST = &device_creator<software_list_device>;

//-------------------------------------------------
//  software_list_device - constructor
//-------------------------------------------------

software_list_device::software_list_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SOFTWARE_LIST, "Software lists", tag, owner, clock),
	  m_list_name(NULL),
	  m_list_type(SOFTWARE_LIST_ORIGINAL_SYSTEM),
	  m_filter(NULL)
{
}


//-------------------------------------------------
//  static_set_interface - configuration helper
//  to set the interface
//-------------------------------------------------

void software_list_device::static_set_config(device_t &device, const char *list, softlist_type list_type)
{
	software_list_device &softlist = downcast<software_list_device &>(device);
	softlist.m_list_name = list;
	softlist.m_list_type = list_type;
}


//-------------------------------------------------
//  static_set_custom_handler - configuration
//  helper to set a custom callback
//-------------------------------------------------

void software_list_device::static_set_filter(device_t &device, const char *filter)
{
	downcast<software_list_device &>(device).m_filter = filter;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void software_list_device::device_start()
{
}



/***************************************************************************
    EXPAT INTERFACES
***************************************************************************/

/*-------------------------------------------------
    expat_malloc/expat_realloc/expat_free -
    wrappers for memory allocation functions so
    that they pass through out memory tracking
    systems
-------------------------------------------------*/

static void *expat_malloc(size_t size)
{
	return global_alloc_array_clear(UINT8,size);
}

static void *expat_realloc(void *ptr, size_t size)
{
	if (ptr) global_free(ptr);
	return global_alloc_array_clear(UINT8,size);
}

static void expat_free(void *ptr)
{
	global_free(ptr);
}


/*-------------------------------------------------
    parse_error
-------------------------------------------------*/

INLINE void ATTR_PRINTF(2,3) parse_error(parse_state *state, const char *fmt, ...)
{
	char buf[256];
	va_list va;

	if (state->error_proc)
	{
		va_start(va, fmt);
		vsnprintf(buf, ARRAY_LENGTH(buf), fmt, va);
		va_end(va);
		(*state->error_proc)(buf);
	}
}


/*-------------------------------------------------
    unknown_tag
-------------------------------------------------*/

INLINE void unknown_tag(software_list *swlist, const char *tagname)
{
	parse_error(&swlist->state, "%s: Unknown tag: %s (line %lu column %lu)\n",
		swlist->file->filename(),
		tagname,
		XML_GetCurrentLineNumber(swlist->state.parser),
		XML_GetCurrentColumnNumber(swlist->state.parser));
}



/*-------------------------------------------------
    unknown_attribute
-------------------------------------------------*/

INLINE void unknown_attribute(software_list *swlist, const char *attrname)
{
	parse_error(&swlist->state, "%s: Unknown attribute: %s (line %lu column %lu)\n",
		swlist->file->filename(),
		attrname,
		XML_GetCurrentLineNumber(swlist->state.parser),
		XML_GetCurrentColumnNumber(swlist->state.parser));
}



/*-------------------------------------------------
    unknown_attribute_value
-------------------------------------------------*/

INLINE void unknown_attribute_value(software_list *swlist,
	const char *attrname, const char *attrvalue)
{
	parse_error(&swlist->state, "%s: Unknown attribute value: %s (line %lu column %lu)\n",
		swlist->file->filename(),
		attrvalue,
		XML_GetCurrentLineNumber(swlist->state.parser),
		XML_GetCurrentColumnNumber(swlist->state.parser));
}


/*-------------------------------------------------
    software_name_split
    helper; splits a software_list:software:part
    string into separate software_list, software,
    and part strings.

    str1:str2:str3  => swlist_name - str1, swname - str2, swpart - str3
    str1:str2       => swlist_name - NULL, swname - str1, swpart - str2
    str1            => swlist_name - NULL, swname - str1, swpart - NULL

    swlist_namem, swnane and swpart will be global_alloc'ed
    from the global pool. So they should be global_free'ed
    when they are not used anymore.
-------------------------------------------------*/

#define global_strdup(s)				strcpy(global_alloc_array(char, strlen(s) + 1), s)

static void software_name_split(const char *swlist_swname, char **swlist_name, char **swname, char **swpart )
{
	const char *split_1st_loc = strchr( swlist_swname, ':' );
	const char *split_2nd_loc = ( split_1st_loc ) ? strchr( split_1st_loc + 1, ':' ) : NULL;

	*swlist_name = NULL;
	*swname = NULL;
	*swpart = NULL;

	if ( split_1st_loc )
	{
		if ( split_2nd_loc )
		{
			int size = split_1st_loc - swlist_swname;
			*swlist_name = global_alloc_array_clear(char,size+1);
			memcpy( *swlist_name, swlist_swname, size );

			size = split_2nd_loc - ( split_1st_loc + 1 );
			*swname = global_alloc_array_clear(char,size+1);
			memcpy( *swname, split_1st_loc + 1, size );

			size = strlen( swlist_swname ) - ( split_2nd_loc + 1 - swlist_swname );
			*swpart = global_alloc_array_clear(char,size+1);
			memcpy( *swpart, split_2nd_loc + 1, size );
		}
		else
		{
			int size = split_1st_loc - swlist_swname;
			*swname = global_alloc_array_clear(char,size+1);
			memcpy( *swname, swlist_swname, size );

			size = strlen( swlist_swname ) - ( split_1st_loc + 1 - swlist_swname );
			*swpart = global_alloc_array_clear(char,size+1);
			memcpy( *swpart, split_1st_loc + 1, size );
		}
	}
	else
	{
		*swname = global_strdup(swlist_swname);
	}
}


/*-------------------------------------------------
    add_rom_entry
-------------------------------------------------*/

static void add_rom_entry(software_list *swlist, const char *name, const char *hashdata, UINT32 offset, UINT32 length, UINT32 flags)
{
	software_part *part = &swlist->softinfo->partdata[swlist->softinfo->current_part_entry-1];
	if ((flags & ROMENTRY_TYPEMASK) == ROMENTRYTYPE_REGION && name!=NULL && part!=NULL) {
		if (swlist->current_rom_entry>0) {
			for (int i=0;i<swlist->current_rom_entry;i++) {
				if ((part->romdata[i]._name != NULL) && (strcmp(part->romdata[i]._name,name)==0)) {
					parse_error(&swlist->state, "%s: Duplicated dataarea %s in %s\n",swlist->file->filename(),name,swlist->current_software_info->shortname);
				}
			}
		}
	}

	struct rom_entry *entry = &part->romdata[swlist->current_rom_entry];

	entry->_name = name;
	entry->_hashdata = hashdata;
	entry->_offset = offset;
	entry->_length = length;
	entry->_flags = flags;

	swlist->current_rom_entry += 1;

	if ( swlist->current_rom_entry >= swlist->rom_entries )
	{
		struct rom_entry *new_entries;

		swlist->rom_entries += 10;
		new_entries = (struct rom_entry *)pool_realloc_lib(swlist->pool, part->romdata, swlist->rom_entries * sizeof(struct rom_entry) );

		if ( new_entries )
		{
			part->romdata = new_entries;
		}
		else
		{
			/* Allocation error */
			swlist->current_rom_entry -= 1;
		}
	}
}

/*-------------------------------------------------
    add_feature
-------------------------------------------------*/

static void add_feature(software_list *swlist, char *feature_name, char *feature_value)
{
	software_part *part = &swlist->softinfo->partdata[swlist->softinfo->current_part_entry-1];
	feature_list *new_entry;

	/* First allocate the new entry */
	new_entry = (feature_list *)pool_malloc_lib(swlist->pool, sizeof(feature_list) );

	if ( new_entry )
	{
		new_entry->next = NULL;
		new_entry->name = feature_name;
		new_entry->value = feature_value ? feature_value : feature_name;

		/* Add new feature to end of feature list */
		if ( part->featurelist )
		{
			feature_list *list = part->featurelist;
			while ( list->next != NULL )
			{
				list = list->next;
			}
			list->next = new_entry;
		}
		else
		{
			part->featurelist = new_entry;
		}
	}
	else
	{
		/* Unable to allocate memory */
	}
}

/*-------------------------------------------------
 add_info (same as add_feature, but its target
 is softinfo->shared_info)
 -------------------------------------------------*/

static void add_info(software_list *swlist, char *feature_name, char *feature_value)
{
	software_info *info = swlist->softinfo;
	feature_list *new_entry;

	/* First allocate the new entry */
	new_entry = (feature_list *)pool_malloc_lib(swlist->pool, sizeof(feature_list) );

	if ( new_entry )
	{
		new_entry->next = NULL;
		new_entry->name = feature_name;
		new_entry->value = feature_value ? feature_value : feature_name;

		/* Add new feature to end of feature list */
		if ( info->shared_info )
		{
			feature_list *list = info->shared_info;
			while ( list->next != NULL )
			{
				list = list->next;
			}
			list->next = new_entry;
		}
		else
		{
			info->shared_info = new_entry;
		}
	}
	else
	{
		/* Unable to allocate memory */
	}
}

/*-------------------------------------------------
    add_software_part
-------------------------------------------------*/

static void add_software_part(software_list *swlist, const char *name, const char *interface)
{
	software_part *part = &swlist->softinfo->partdata[swlist->softinfo->current_part_entry];

	part->name = name;
	part->interface_ = interface;
	part->featurelist = NULL;
	part->romdata = NULL;

	swlist->softinfo->current_part_entry += 1;

	if ( swlist->softinfo->current_part_entry >= swlist->softinfo->part_entries )
	{
		software_part *new_parts;

		swlist->softinfo->part_entries += 2;
		new_parts = (software_part *)pool_realloc_lib(swlist->pool, swlist->softinfo->partdata, swlist->softinfo->part_entries * sizeof(software_part) );

		if ( new_parts )
		{
			swlist->softinfo->partdata = new_parts;
		}
		else
		{
			/* Allocation error */
			swlist->softinfo->current_part_entry -= 1;
		}
	}
}


/*-------------------------------------------------
    start_handler
-------------------------------------------------*/

static void start_handler(void *data, const char *tagname, const char **attributes)
{
	software_list *swlist = (software_list *) data;
	char **text_dest;

	switch(swlist->state.pos)
	{
		case POS_ROOT:
			if (!strcmp(tagname, "softwarelist"))
			{
				for( ; attributes[0]; attributes += 2 )
				{
					if ( ! strcmp(attributes[0], "name" ) )
					{
					}
					else if ( ! strcmp(attributes[0], "description" ) )
					{
						swlist->description =  (const char *)pool_malloc_lib(swlist->pool, (strlen(attributes[1])  + 1) * sizeof(char));
						if (!swlist->description)
							return;

						strcpy((char *)swlist->description, attributes[1]);
					} else
						unknown_attribute(swlist, attributes[0]);
				}
			}
			else
			{
				unknown_tag(swlist, tagname);
			}
			break;

		case POS_MAIN:
			if ( !strcmp( tagname, "software" ) )
			{
				const char *name = NULL;
				const char *parent = NULL;
				const char *supported = NULL;

				for ( ; attributes[0]; attributes += 2 )
				{
					if ( !strcmp( attributes[0], "name" ) )
					{
						name = attributes[1];
					}
					else if ( !strcmp( attributes[0], "cloneof" ) )
					{
						parent = attributes[1];
					}
					else if ( !strcmp( attributes[0], "supported" ) )
					{
						supported = attributes[1];
					}
					else
						unknown_attribute(swlist, attributes[0]);
				}

				if ( name )
				{
					struct software_info *elem = (struct software_info *)pool_malloc_lib(swlist->pool,sizeof(struct software_info));

					if ( !elem )
						return;

					/* Clear element and add element to list */
					memset(elem,0,sizeof(struct software_info));

					/* Allocate space to hold the shortname and copy the short name */
					elem->shortname = (const char*)pool_malloc_lib(swlist->pool, ( strlen( name ) + 1 ) * sizeof(char) );

					if ( ! elem->shortname )
						return;

					strcpy( (char *)elem->shortname, name );

					/* Allocate space to hold the parentname and copy the parent name */
					if (parent)
					{
						elem->parentname = (const char*)pool_malloc_lib(swlist->pool, ( strlen(parent) + 1 ) * sizeof(char) );
						strcpy((char *)elem->parentname, parent);
					}

					/* Allocate initial space to hold part information */
					elem->part_entries = 2;
					elem->current_part_entry = 0;
					elem->partdata = (software_part *)pool_malloc_lib(swlist->pool, elem->part_entries * sizeof(software_part) );
					if ( !elem->partdata )
						return;
					elem->shared_info = (feature_list *)pool_malloc_lib(swlist->pool, sizeof(feature_list) );
					if ( !elem->shared_info )
						return;
					else
					{
						elem->shared_info->next = (feature_list *)pool_malloc_lib(swlist->pool, sizeof(feature_list) );
						elem->shared_info->next = NULL;
						elem->shared_info->name = NULL;
						elem->shared_info->value = NULL;
					}

					/* Handle the supported flag */
					elem->supported = SOFTWARE_SUPPORTED_YES;
					if ( supported && ! strcmp( supported, "partial" ) )
						elem->supported = SOFTWARE_SUPPORTED_PARTIAL;
					if ( supported && ! strcmp( supported, "no" ) )
						elem->supported = SOFTWARE_SUPPORTED_NO;

					/* Add the entry to the end of the list */
					if ( swlist->software_info_list == NULL )
					{
						swlist->software_info_list = elem;
						swlist->current_software_info = elem;
					}
					else
					{
						swlist->current_software_info->next = elem;
						swlist->current_software_info = elem;
					}

					/* Quick lookup for setting software information */
					swlist->softinfo = swlist->current_software_info;
				}
				else
				{
					parse_error(&swlist->state, "%s: No name defined for item (line %lu)\n",
						swlist->file->filename(),XML_GetCurrentLineNumber(swlist->state.parser));

					swlist->softinfo = NULL;
				}
			}
			else
			{
				unknown_tag(swlist, tagname);
			}
			break;

		case POS_SOFT:
			text_dest = NULL;

			if (!strcmp(tagname, "description"))
				text_dest = (char **) &swlist->softinfo->longname;
			else if (!strcmp(tagname, "year"))
				text_dest = (char **) &swlist->softinfo->year;
			else if (!strcmp(tagname, "publisher"))
				text_dest = (char **) &swlist->softinfo->publisher;
			else if (!strcmp(tagname, "info"))
			{
				// the "info" field (containing info about actual developers, etc.) is not currently stored.
				// full support will be added, but for the moment frontend have to get this info from the xml directly
			}
			else if (!strcmp(tagname, "sharedfeat"))
			{
				const char *str_feature_name = NULL;
				const char *str_feature_value = NULL;

				for ( ; attributes[0]; attributes += 2 )
				{
					if ( !strcmp( attributes[0], "name" ) )
						str_feature_name = attributes[1];

					else if ( !strcmp( attributes[0], "value" ) )
						str_feature_value = attributes[1];

					else
						unknown_attribute(swlist, attributes[0]);
				}

				/* Prepare for adding feature to feature list */
				if ( str_feature_name && swlist->softinfo )
				{
					char *name = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_feature_name ) + 1 ) * sizeof(char) );
					char *value = NULL;

					if ( !name )
						return;

					strcpy( name, str_feature_name );

					if ( str_feature_value )
					{
						value = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_feature_value ) + 1 ) * sizeof(char) );

						if ( !value )
							return;

						strcpy( value, str_feature_value );
					}

					add_info( swlist, name, value );
				} else {
					parse_error(&swlist->state, "%s: Incomplete sharedfeat definition (line %lu)\n",
						swlist->file->filename(),XML_GetCurrentLineNumber(swlist->state.parser));
				}
			}
			else if ( !strcmp(tagname, "part" ) )
			{
				const char *str_name = NULL;
				const char *str_interface = NULL;

				for ( ; attributes[0]; attributes += 2 )
				{
					if ( !strcmp( attributes[0], "name" ) )
						str_name = attributes[1];

					else if ( !strcmp( attributes[0], "interface" ) )
						str_interface = attributes[1];

					else
						unknown_attribute(swlist, attributes[0]);
				}

				if ( str_name && str_interface )
				{
					if ( swlist->softinfo )
					{
						char *name = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_name ) + 1 ) * sizeof(char) );
						char *interface = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_interface ) + 1 ) * sizeof(char) );

						if ( !name || !interface )
							return;

						strcpy( name, str_name );
						strcpy( interface, str_interface );

						add_software_part( swlist, name, interface );

						/* Allocate initial space to hold the rom information */
						swlist->rom_entries = 3;
						swlist->current_rom_entry = 0;
						swlist->softinfo->partdata[swlist->softinfo->current_part_entry-1].romdata = (struct rom_entry *)pool_malloc_lib(swlist->pool, swlist->rom_entries * sizeof(struct rom_entry));
						if ( ! swlist->softinfo->partdata[swlist->softinfo->current_part_entry-1].romdata )
							return;
					}
				}
				else
				{
					/* Incomplete/incorrect part definition */
					parse_error(&swlist->state, "%s: Incomplete part definition (line %lu)\n",
						swlist->file->filename(),XML_GetCurrentLineNumber(swlist->state.parser));
				}
			}
			else
				unknown_tag(swlist, tagname);

			if (text_dest && swlist->softinfo)
				swlist->state.text_dest = text_dest;
			break;

		case POS_PART:
			if (!strcmp(tagname, "dataarea"))
			{
				const char *str_name = NULL;
				const char *str_size = NULL;

				for ( ; attributes[0]; attributes += 2 )
				{
					if ( !strcmp( attributes[0], "name" ) )
						str_name = attributes[1];

					else if ( !strcmp( attributes[0], "size") )
						str_size = attributes[1];

					else
						unknown_attribute(swlist, attributes[0]);
				}
				if ( str_name && str_size )
				{
					if ( swlist->softinfo )
					{
						UINT32 length = strtol( str_size, NULL, 0 );
						char *s = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_name ) + 1 ) * sizeof(char) );

						if ( !s )
							return;

						strcpy( s, str_name );

						/* ROM_REGION( length, "name", flags ) */
						add_rom_entry( swlist, s, NULL, 0, length, ROMENTRYTYPE_REGION );
					}
				}
				else
				{
					/* Missing dataarea name or size */
					parse_error(&swlist->state, "%s: Incomplete dataarea definition (line %lu)\n",
						swlist->file->filename(),XML_GetCurrentLineNumber(swlist->state.parser));
				}
			}
			else if (!strcmp(tagname, "diskarea"))
			{
				const char *str_name = NULL;

				for ( ; attributes[0]; attributes += 2 )
				{
					if ( !strcmp( attributes[0], "name" ) )
						str_name = attributes[1];
					else
						unknown_attribute(swlist, attributes[0]);
				}
				if ( str_name )
				{
					if ( swlist->softinfo )
					{
						char *s = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_name ) + 1 ) * sizeof(char) );

						if ( !s )
							return;

						strcpy( s, str_name );

						/* ROM_REGION( length, "name", flags ) */
						add_rom_entry( swlist, s, NULL, 0, 1, ROMENTRYTYPE_REGION | ROMREGION_DATATYPEDISK);
					}
				}
				else
				{
					/* Missing dataarea name or size */
					parse_error(&swlist->state, "%s: Incomplete diskarea definition (line %lu)\n",
						swlist->file->filename(),XML_GetCurrentLineNumber(swlist->state.parser));
				}
			}
			else if ( !strcmp(tagname, "feature") )
			{
				const char *str_feature_name = NULL;
				const char *str_feature_value = NULL;

				for ( ; attributes[0]; attributes += 2 )
				{
					if ( !strcmp( attributes[0], "name" ) )
						str_feature_name = attributes[1];

					else if ( !strcmp( attributes[0], "value" ) )
						str_feature_value = attributes[1];

					else
						unknown_attribute(swlist, attributes[0]);
				}

				/* Prepare for adding feature to feature list */
				if ( str_feature_name && swlist->softinfo )
				{
					char *name = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_feature_name ) + 1 ) * sizeof(char) );
					char *value = NULL;

					if ( !name )
						return;

					strcpy( name, str_feature_name );

					if ( str_feature_value )
					{
						value = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_feature_value ) + 1 ) * sizeof(char) );

						if ( !value )
							return;

						strcpy( value, str_feature_value );
					}

					add_feature( swlist, name, value );
				} else {
					parse_error(&swlist->state, "%s: Incomplete feature definition (line %lu)\n",
						swlist->file->filename(),XML_GetCurrentLineNumber(swlist->state.parser));
				}
			}
			else if (!strcmp(tagname, "dipswitch"))
			{
			}
			else
				unknown_tag(swlist, tagname );
			break;

		case POS_DATA:
			if (!strcmp(tagname, "rom"))
			{
				const char *str_name = NULL;
				const char *str_size = NULL;
				const char *str_crc = NULL;
				const char *str_sha1 = NULL;
				const char *str_offset = NULL;
				const char *str_value = NULL;
				const char *str_status = NULL;
				const char *str_loadflag = NULL;

				for ( ; attributes[0]; attributes += 2 )
				{
					if ( !strcmp( attributes[0], "name" ) )
						str_name = attributes[1];
					else if ( !strcmp( attributes[0], "size" ) )
						str_size = attributes[1];
					else if ( !strcmp( attributes[0], "crc" ) )
						str_crc = attributes[1];
					else if ( !strcmp( attributes[0], "sha1" ) )
						str_sha1 = attributes[1];
					else if ( !strcmp( attributes[0], "offset" ) )
						str_offset = attributes[1];
					else if ( !strcmp( attributes[0], "value" ) )
						str_value = attributes[1];
					else if ( !strcmp( attributes[0], "status" ) )
						str_status = attributes[1];
					else if ( !strcmp( attributes[0], "loadflag" ) )
						str_loadflag = attributes[1];
					else
						unknown_attribute(swlist, attributes[0]);
				}
				if ( swlist->softinfo )
				{
					if ( str_size && str_offset )
					{
						UINT32 length = strtol( str_size, NULL, 0 );
						UINT32 offset = strtol( str_offset, NULL, 0 );

						if ( str_loadflag && !strcmp(str_loadflag, "reload") )
						{
							/* Handle 'reload' loadflag */
							add_rom_entry( swlist, NULL, NULL, offset, length, ROMENTRYTYPE_RELOAD | ROM_INHERITFLAGS );
						}
						else if ( str_loadflag && !strcmp(str_loadflag, "reload_plain") )
						{
							/* Handle 'reload_plain' loadflag */
							add_rom_entry( swlist, NULL, NULL, offset, length, ROMENTRYTYPE_RELOAD);
						}
						else if ( str_loadflag && !strcmp(str_loadflag, "continue") )
						{
							/* Handle 'continue' loadflag */
							add_rom_entry( swlist, NULL, NULL, offset, length, ROMENTRYTYPE_CONTINUE | ROM_INHERITFLAGS );
						}
						else if ( str_loadflag && !strcmp(str_loadflag, "fill") )
						{
							/* Handle 'fill' loadflag */
							add_rom_entry( swlist, NULL, (const char*)(FPTR)(strtol( str_value, NULL, 0 ) & 0xff), offset, length, ROMENTRYTYPE_FILL );
						}
						else
						{
							if ( str_name)
							{
								char *s_name = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_name ) + 1 ) * sizeof(char) );
								int hashsize = 7 + 4;
								if (str_crc) hashsize+= strlen(str_crc);
								if (str_sha1) hashsize+= strlen(str_sha1);
								char *hashdata = (char *)pool_malloc_lib( swlist->pool, sizeof(char) * (hashsize) );
								int baddump = ( str_status && !strcmp(str_status, "baddump") ) ? 1 : 0;
								int nodump = ( str_status && !strcmp(str_status, "nodump" ) ) ? 1 : 0;
								int romflags = 0;

								if ( !s_name || !hashdata )
									return;

								strcpy( s_name, str_name );
								if (nodump) {
									sprintf( hashdata, "%s", NO_DUMP);
									if (str_crc && str_sha1) {
										parse_error(&swlist->state, "%s: No need for hash definition (line %lu)\n",
											swlist->file->filename(),XML_GetCurrentLineNumber(swlist->state.parser));
									}
								} else {
									if (str_crc && str_sha1) {
										sprintf( hashdata, "%c%s%c%s%s", hash_collection::HASH_CRC, str_crc, hash_collection::HASH_SHA1, str_sha1, (baddump ? BAD_DUMP : ""));
									} else {
										parse_error(&swlist->state, "%s: Incomplete rom hash definition (line %lu)\n",
											swlist->file->filename(),XML_GetCurrentLineNumber(swlist->state.parser));
									}
								}

								/* Handle loadflag attribute */
								if ( str_loadflag && !strcmp(str_loadflag, "load16_word_swap") )
									romflags = ROM_GROUPWORD | ROM_REVERSE;
								else if ( str_loadflag && !strcmp(str_loadflag, "load16_byte") )
									romflags = ROM_SKIP(1);
								else if ( str_loadflag && !strcmp(str_loadflag, "load32_word_swap") )
									romflags = ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2);
								else if ( str_loadflag && !strcmp(str_loadflag, "load32_word") )
									romflags = ROM_GROUPWORD | ROM_SKIP(2);
								else if ( str_loadflag && !strcmp(str_loadflag, "load32_byte") )
									romflags = ROM_SKIP(3);

								/* ROM_LOAD( name, offset, length, hash ) */
								add_rom_entry( swlist, s_name, hashdata, offset, length, ROMENTRYTYPE_ROM | romflags );
							} else {
								parse_error(&swlist->state, "%s: Rom name missing (line %lu)\n",
									swlist->file->filename(),XML_GetCurrentLineNumber(swlist->state.parser));
							}
						}
					}
					else
					{
						/* Missing name, size, crc, sha1, or offset */
						parse_error(&swlist->state, "%s: Incomplete rom definition (line %lu)\n",
							swlist->file->filename(),XML_GetCurrentLineNumber(swlist->state.parser));
					}
				}
			}
			else
			if (!strcmp(tagname, "disk"))
			{
				const char *str_name = NULL;
				const char *str_sha1 = NULL;
				const char *str_status = NULL;
				const char *str_writeable = NULL;

				for ( ; attributes[0]; attributes += 2 )
				{
					if ( !strcmp( attributes[0], "name" ) )
						str_name = attributes[1];
					else if ( !strcmp( attributes[0], "sha1" ) )
						str_sha1 = attributes[1];
					else if ( !strcmp( attributes[0], "status" ) )
						str_status = attributes[1];
					else if ( !strcmp( attributes[0], "writeable" ) )
						str_writeable = attributes[1];
					else
						unknown_attribute(swlist, attributes[0]);
				}
				if ( swlist->softinfo )
				{
					if ( str_name && str_sha1 )
					{
						char *s_name = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_name ) + 1 ) * sizeof(char) );
						char *hashdata = (char *)pool_malloc_lib( swlist->pool, sizeof(char) * ( strlen(str_sha1) + 7 + 4 ) );
						int baddump = ( str_status && !strcmp(str_status, "baddump") ) ? 1 : 0;
						int nodump = ( str_status && !strcmp(str_status, "nodump" ) ) ? 1 : 0;
						int writeable = ( str_writeable && !strcmp(str_writeable, "yes" ) ) ? 1 : 0;

						if ( !s_name || !hashdata )
							return;

						strcpy( s_name, str_name );
						sprintf( hashdata, "%c%s%s", hash_collection::HASH_SHA1, str_sha1, ( nodump ? NO_DUMP : ( baddump ? BAD_DUMP : "" ) ) );

						add_rom_entry( swlist, s_name, hashdata, 0, 0, ROMENTRYTYPE_ROM | (writeable ? DISK_READWRITE : DISK_READONLY ) );
					}
					else
					{
						if (!str_status || strcmp(str_status, "nodump")) // a no_dump chd is not an incomplete entry
						{
							parse_error(&swlist->state, "%s: Incomplete disk definition (line %lu)\n",
										swlist->file->filename(),XML_GetCurrentLineNumber(swlist->state.parser));
						}
					}
				}
			}
			else if (!strcmp(tagname, "dipvalue"))
			{
			}
			else
				unknown_tag(swlist, tagname);
			break;
	}
	swlist->state.pos = (softlist_parse_position) (swlist->state.pos + 1);
}

/*-------------------------------------------------
    end_handler
-------------------------------------------------*/

static void end_handler(void *data, const char *name)
{
	software_list *swlist = (software_list *) data;
	swlist->state.text_dest = NULL;

	swlist->state.pos = (softlist_parse_position) (swlist->state.pos - 1);
	switch(swlist->state.pos)
	{
		case POS_ROOT:
			break;

		case POS_MAIN:
			if ( swlist->softinfo )
			{
				add_software_part( swlist, NULL, NULL );
			}
			break;

		case POS_SOFT:
			if ( ! strcmp( name, "part" ) && swlist->softinfo )
			{
				/* ROM_END */
				add_rom_entry( swlist, NULL, NULL, 0, 0, ROMENTRYTYPE_END );
				/* Add shared_info inherited from the software_info level, if any */
				if ( swlist->softinfo && swlist->softinfo->shared_info )
				{
					feature_list *list = swlist->softinfo->shared_info;

					while( list->next )
					{
						add_feature( swlist, list->next->name, list->next->value );
						list = list->next;
					}
				}
			}
			break;

		case POS_PART:
			break;
		case POS_DATA:
			break;
	}
}


/*-------------------------------------------------
    data_handler
-------------------------------------------------*/

static void data_handler(void *data, const XML_Char *s, int len)
{
	software_list *swlist = (software_list *) data;
	int text_len;
	char *text;

	if (swlist->state.text_dest)
	{
		text = *swlist->state.text_dest;

		text_len = text ? strlen(text) : 0;
		text = (char*)pool_realloc_lib(swlist->pool, text, text_len + len + 1);
		if (!text)
			return;

		memcpy(&text[text_len], s, len);
		text[text_len + len] = '\0';
		*swlist->state.text_dest = text;
	} else {
		if (swlist->state.error_proc)
		{
			int errcnt = 0;
			for (int i=0;i<len;i++) {
				if (!(s[i]=='\t' || s[i]=='\n' || s[i]=='\r' || s[i]==' ')) errcnt++;
			}
			if (errcnt>0) {
				parse_error(&swlist->state, "%s: Unknown content (line %lu)\n",
					swlist->file->filename(),
					XML_GetCurrentLineNumber(swlist->state.parser));
			}
		}
	}
}


/*-------------------------------------------------
 software_list_get_count
 -------------------------------------------------*/

static int software_list_get_count(const software_list *swlist)
{
	int count = 0;

	for (const software_info *swinfo = software_list_find(swlist, "*", NULL); swinfo != NULL; swinfo = software_list_find(swlist, "*", swinfo))
		count++;

	return count;
}


/*-------------------------------------------------
    software_get_clone - retrive name string of the
    parent software, if any
 -------------------------------------------------*/

const char *software_get_clone(emu_options &options, char *swlist, const char *swname)
{
	const software_list *software_list_ptr = software_list_open(options, swlist, FALSE, NULL);
	const char *retval = NULL;
	if (software_list_ptr)
	{
		const software_info *tmp = software_list_find(software_list_ptr, swname, NULL);
		retval = core_strdup(tmp->parentname);
		software_list_close(software_list_ptr);
	}

	return retval;
}


/*-------------------------------------------------
    software_get_support - retrive support state of
    the software
 -------------------------------------------------*/

UINT32 software_get_support(emu_options &options, char *swlist, const char *swname)
{
	const software_list *software_list_ptr = software_list_open(options, swlist, FALSE, NULL);
	UINT32 retval = 0;

	if (software_list_ptr)
	{
		const software_info *tmp = software_list_find(software_list_ptr, swname, NULL);
		retval = tmp->supported;
		software_list_close(software_list_ptr);
	}

	return retval;
}


/*-------------------------------------------------
    software_list_parse
-------------------------------------------------*/

void software_list_parse(software_list *swlist,
	void (*error_proc)(const char *message),
	void *param)
{
	char buf[1024];
	UINT32 len;
	XML_Memory_Handling_Suite memcallbacks;

	swlist->file->seek(0, SEEK_SET);

	memset(&swlist->state, 0, sizeof(swlist->state));
	swlist->state.error_proc = error_proc;
	swlist->state.param = param;

	/* create the XML parser */
	memcallbacks.malloc_fcn = expat_malloc;
	memcallbacks.realloc_fcn = expat_realloc;
	memcallbacks.free_fcn = expat_free;
	swlist->state.parser = XML_ParserCreate_MM(NULL, &memcallbacks, NULL);
	if (!swlist->state.parser)
		goto done;

	XML_SetUserData(swlist->state.parser, swlist);
	XML_SetElementHandler(swlist->state.parser, start_handler, end_handler);
	XML_SetCharacterDataHandler(swlist->state.parser, data_handler);

	while(!swlist->state.done)
	{
		len = swlist->file->read(buf, sizeof(buf));
		swlist->state.done = swlist->file->eof();
		if (XML_Parse(swlist->state.parser, buf, len, swlist->state.done) == XML_STATUS_ERROR)
		{
			parse_error(&swlist->state, "%s: %s (line %lu column %lu)\n",
				swlist->file->filename(),
				XML_ErrorString(XML_GetErrorCode(swlist->state.parser)),
				XML_GetCurrentLineNumber(swlist->state.parser),
				XML_GetCurrentColumnNumber(swlist->state.parser));
			goto done;
		}
	}

done:
	if (swlist->state.parser)
		XML_ParserFree(swlist->state.parser);
	swlist->state.parser = NULL;
	swlist->current_software_info = swlist->software_info_list;
	swlist->list_entries = software_list_get_count(swlist);
}


/*-------------------------------------------------
    software_list_open
-------------------------------------------------*/

software_list *software_list_open(emu_options &options, const char *listname, int is_preload,
	void (*error_proc)(const char *message))
{
	software_list *swlist = NULL;
	object_pool *pool = NULL;
	file_error filerr;

	/* create a pool for this software list file */
	pool = pool_alloc_lib(error_proc);
	if (!pool)
		goto error;

	/* allocate space for this software list file */
	swlist = (software_list *) pool_malloc_lib(pool, sizeof(*swlist));
	if (!swlist)
		goto error;

	/* set up the software_list structure */
	memset(swlist, 0, sizeof(*swlist));
	swlist->pool = pool;
	swlist->error_proc = error_proc;

	/* open a file */
	swlist->file = global_alloc(emu_file(options.hash_path(), OPEN_FLAG_READ));
	filerr = swlist->file->open(listname, ".xml");
	if (filerr != FILERR_NONE)
		goto error;

	if (is_preload)
	{
		software_list_parse(swlist, swlist->error_proc, NULL);
		swlist->current_software_info = NULL;
	}

	return swlist;

error:
	if (swlist != NULL)
		software_list_close(swlist);
	return NULL;
}


/*-------------------------------------------------
    software_list_close
-------------------------------------------------*/

void software_list_close(const software_list *swlist)
{
	if (swlist == NULL)
		return;

	if (swlist->file != NULL)
		global_free(swlist->file);
	pool_free_lib(swlist->pool);
}


/*-------------------------------------------------
 software_list_get_description
 -------------------------------------------------*/

const char *software_list_get_description(const software_list *swlist)
{
	return swlist->description;
}


/*-------------------------------------------------
 software_list_find_by_number
 -------------------------------------------------*/

INLINE software_info *software_list_find_by_number(software_list *swlist, int number)
{
	int length = swlist->list_entries;
	if (number > length)
		return NULL;

	software_info *cur_info = software_list_find(swlist, "*", NULL);

	for (int count = 0; count < number; count++)
		cur_info = software_list_find(swlist, "*", cur_info);

	return cur_info;
}


/*-------------------------------------------------
 softlist_penalty_compare (borrowed from driver.c)
 -------------------------------------------------*/

static int softlist_penalty_compare(const char *source, const char *target)
{
	int gaps = 1;
	int last = TRUE;

	/* scan the strings */
	for ( ; *source && *target; target++)
	{
		/* do a case insensitive match */
		int match = (tolower((UINT8)*source) == tolower((UINT8)*target));

		/* if we matched, advance the source */
		if (match)
			source++;

		/* if the match state changed, count gaps */
		if (match != last)
		{
			last = match;
			if (!match)
				gaps++;
		}
	}

	/* penalty if short string does not completely fit in */
	for ( ; *source; source++)
		gaps++;

	/* if we matched perfectly, gaps == 0 */
	if (gaps == 1 && *source == 0 && *target == 0)
		gaps = 0;

	return gaps;
}


/*-------------------------------------------------
 software_list_find_approx_matches
 -------------------------------------------------*/

void software_list_find_approx_matches(software_list_device *swlistdev, software_list *swlist, const char *name, int matches, software_info **list, const char* interface)
{
#undef rand

	int matchnum;
	int *penalty;

	/* if no name, return */
	if (name == NULL || name[0] == 0)
		return;

	/* allocate some temp memory */
	penalty = global_alloc_array(int, matches);

	/* initialize everyone's states */
	for (matchnum = 0; matchnum < matches; matchnum++)
	{
		penalty[matchnum] = 9999;
		list[matchnum] = NULL;
	}

	for (software_info *swinfo = software_list_find(swlist, "*", NULL); swinfo != NULL; swinfo = software_list_find(swlist, "*", swinfo))
	{
		int curpenalty, tmp;
		software_info *candidate = swinfo;

		software_part *part = software_find_part(swinfo, NULL, NULL);
		if ((interface==NULL || softlist_contain_interface(interface, part->interface_)) && (is_software_compatible(part, swlistdev)))
		{

			/* pick the best match between driver name and description */
			curpenalty = softlist_penalty_compare(name, candidate->longname);
			tmp = softlist_penalty_compare(name, candidate->shortname);
			curpenalty = MIN(curpenalty, tmp);

			/* insert into the sorted table of matches */
			for (matchnum = matches - 1; matchnum >= 0; matchnum--)
			{
				/* stop if we're worse than the current entry */
				if (curpenalty >= penalty[matchnum])
					break;

				/* as long as this isn't the last entry, bump this one down */
				if (matchnum < matches - 1)
				{
					penalty[matchnum + 1] = penalty[matchnum];
					list[matchnum + 1] = list[matchnum];
				}
				list[matchnum] = candidate;
				penalty[matchnum] = curpenalty;
			}
		}
	}

	/* free our temp memory */
	global_free(penalty);
}


/*-------------------------------------------------
    software_list_find
-------------------------------------------------*/

const software_info *software_list_find(const software_list *swlist, const char *look_for, const software_info *prev)
{
	if (swlist == NULL)
		return NULL;

	if (look_for == NULL)
		return NULL;

	/* If we haven't read in the xml file yet, then do it now */
	/* Just-in-time parsing, hence the const-cast */
	if ( ! swlist->software_info_list )
		software_list_parse( const_cast<software_list *>(swlist), swlist->error_proc, NULL );

	for ( prev = prev ? prev->next : swlist->software_info_list; prev; prev = prev->next )
	{
		if ( !mame_strwildcmp( look_for, prev->shortname ) )
			break;
	}

	return prev;
}

software_info *software_list_find(software_list *swlist, const char *look_for, software_info *prev)
{
	return const_cast<software_info *>(software_list_find(const_cast<const software_list *>(swlist),
														  look_for,
														  const_cast<const software_info *>(prev)));
}

/*-------------------------------------------------
    software_find_romdata (for validation purposes)
 -------------------------------------------------*/

static struct rom_entry *software_find_romdata(software_part *swpart, const char *dataname)
{
	struct rom_entry *data = swpart ? swpart->romdata : NULL;

	/* If no dataname supplied, then we just return the first entry */
	if (data)
	{
		while(data && data->_name)
		{
			if (dataname)
			{
				if (!strcmp(dataname, data->_name))
				{
					break;
				}
			}
			/* No specific dataname supplied, return the first rom_entry */
			else
				break;

			data++;
		}
	}

	if (!data->_name)
		data = NULL;

	return data;
}


/*-------------------------------------------------
    software_romdata_next (for validation purposes)
 -------------------------------------------------*/

static struct rom_entry *software_romdata_next(struct rom_entry *romdata)
{
	if (romdata && romdata->_name)
	{
		romdata++;
	}
	else
		romdata = NULL;

	return romdata;
}


/*-------------------------------------------------
    software_find_part
-------------------------------------------------*/

const software_part *software_find_part(const software_info *sw, const char *partname, const char *interface)
{
	const software_part *part = sw ? sw->partdata : NULL;

	 /* If neither partname nor interface supplied, then we just return the first entry */
	if ( partname || interface )
	{
		while( part && part->name )
		{
			if ( partname )
			{
				if ( !strcmp(partname, part->name ) )
				{
					if ( interface )
					{
						if ( softlist_contain_interface(interface, part->interface_) )
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
			}
			else
			{
				/* No specific partname supplied, find the first match based on interface */
				if ( interface )
				{
					if ( softlist_contain_interface(interface, part->interface_) )
					{
						break;
					}
				}
			}
			part++;
		}
	}

	if ( part && ! part->name )
		part = NULL;

	return part;
}

software_part *software_find_part(software_info *sw, const char *partname, const char *interface)
{
	return const_cast<software_part *>(software_find_part(const_cast<const software_info *>(sw), partname, interface));
}

/*-------------------------------------------------
    software_part_next
-------------------------------------------------*/

const software_part *software_part_next(const software_part *part)
{
	if ( part && part->name )
	{
		part++;
	}

	if ( ! part->name )
		part = NULL;

	return part;
}

software_part *software_part_next(software_part *part)
{
	return const_cast<software_part *>(software_part_next(const_cast<const software_part *>(part)));
}

/*-------------------------------------------------
    software_display_matches
-------------------------------------------------*/

void software_display_matches(const machine_config &config,emu_options &options, const char *interface ,const char *name)
{
	// check if there is at least a software list
	software_list_device_iterator deviter(config.root_device());
	if (deviter.first())
	{
		mame_printf_error("\n\"%s\" approximately matches the following\n"
						  "supported software items (best match first):\n\n", name);
	}

	for (software_list_device *swlist = deviter.first(); swlist != NULL; swlist = deviter.next())
	{
		software_list *list = software_list_open(options, swlist->list_name(), FALSE, NULL);

		if (list)
		{
			software_info *matches[10] = { 0 };
			int softnum;

			software_list_parse(list, list->error_proc, NULL);
			// get the top 5 approximate matches for the selected device interface (i.e. only carts for cartslot, etc.)
			software_list_find_approx_matches(swlist, list, name, ARRAY_LENGTH(matches), matches, interface);

			if (matches[0] != 0)
			{
				if (swlist->list_type() == SOFTWARE_LIST_ORIGINAL_SYSTEM)
					mame_printf_error("* Software list \"%s\" (%s) matches: \n", swlist->list_name(), software_list_get_description(list));
				else
					mame_printf_error("* Compatible software list \"%s\" (%s) matches: \n", swlist->list_name(), software_list_get_description(list));

				// print them out
				for (softnum = 0; softnum < ARRAY_LENGTH(matches); softnum++)
					if (matches[softnum] != NULL)
						mame_printf_error("%-18s%s\n", matches[softnum]->shortname, matches[softnum]->longname);

				mame_printf_error("\n");
			}
			software_list_close(list);
		}
	}
}

static void find_software_item(const machine_config &config, emu_options &options, const device_image_interface *image, const char *path, software_list **software_list_ptr, software_info **software_info_ptr,software_part **software_part_ptr, const char **sw_list_name)
{
	char *swlist_name, *swname, *swpart; //, *swname_bckp;
	*software_list_ptr = NULL;
	*software_info_ptr = NULL;
	*software_part_ptr = NULL;

	/* Split full software name into software list name and short software name */
	software_name_split(path, &swlist_name, &swname, &swpart );
//  swname_bckp = swname;

	const char *interface = NULL;
	if (image) interface = image->image_interface();

	if ( swlist_name )
	{
		/* Try to open the software list xml file explicitly named by the user */
		*software_list_ptr = software_list_open( options, swlist_name, FALSE, NULL );

		if ( *software_list_ptr )
		{
			*software_info_ptr = software_list_find( *software_list_ptr, swname, NULL );

			if ( *software_info_ptr )
			{
				*software_part_ptr = software_find_part( *software_info_ptr, swpart, interface );
			}
		}
	}
	else
	{
		/* Loop through all the software lists named in the driver */
		software_list_device_iterator deviter(config.root_device());
		for (software_list_device *swlist = deviter.first(); swlist != NULL; swlist = deviter.next())
		{
			swlist_name = (char *)swlist->list_name();

			if (swlist->list_type() == SOFTWARE_LIST_ORIGINAL_SYSTEM)
			{
				if ( *software_list_ptr )
				{
					software_list_close( *software_list_ptr );
				}

				*software_list_ptr = software_list_open( options, swlist_name, FALSE, NULL );

				if ( software_list_ptr )
				{
					*software_info_ptr = software_list_find( *software_list_ptr, swname, NULL );

					if ( *software_info_ptr )
					{
						*software_part_ptr = software_find_part( *software_info_ptr, swpart, interface );
						if (*software_part_ptr) break;
					}
				}
			}
		}

		/* If not found try to load the software list using the driver name */
		if ( ! *software_part_ptr )
		{
			swlist_name = (char *)options.system()->name;

			if ( *software_list_ptr )
			{
				software_list_close( *software_list_ptr );
			}

			*software_list_ptr = software_list_open( options, swlist_name, FALSE, NULL );

			if ( *software_list_ptr )
			{
				*software_info_ptr = software_list_find( *software_list_ptr, swname, NULL );

				if ( *software_info_ptr )
				{
					*software_part_ptr = software_find_part( *software_info_ptr, swpart, interface );
				}
			}
		}

		/* If not found try to load the software list using the software name as software */
		/* list name and software part name as software name. */
		if ( ! *software_part_ptr )
		{
			swlist_name = swname;
			swname = swpart;
			swpart = NULL;

			if ( *software_list_ptr )
			{
				software_list_close( *software_list_ptr );
			}

			*software_list_ptr = software_list_open( options, swlist_name, FALSE, NULL );

			if ( software_list_ptr )
			{
				*software_info_ptr = software_list_find( *software_list_ptr, swname, NULL );

				if ( *software_info_ptr )
				{
					*software_part_ptr = software_find_part( *software_info_ptr, swpart, interface );
				}

				if ( ! *software_part_ptr )
				{
					software_list_close( *software_list_ptr );
					*software_list_ptr = NULL;
				}
			}
		}
	}
	*sw_list_name = global_strdup(swlist_name);

	global_free( swlist_name );
	global_free( swname );
	global_free( swpart );
}

/*-------------------------------------------------
    load_software_part

    Load a software part for a device. The part to
    load is determined by the "path", software lists
    configured for a driver, and the interface
    supported by the device.

    returns true if the software could be loaded,
    false otherwise. If the software could be loaded
    sw_info and sw_part are also set.
-------------------------------------------------*/

bool load_software_part(emu_options &options, device_image_interface *image, const char *path, software_info **sw_info, software_part **sw_part, char **full_sw_name, char**list_name)
{
	software_list *software_list_ptr = NULL;
	software_info *software_info_ptr = NULL;
	software_part *software_part_ptr = NULL;
	const char *swlist_name = NULL;

	bool result = false;
	*sw_info = NULL;
	*sw_part = NULL;
	*list_name = NULL;

	find_software_item(image->device().machine().config(), options, image, path, &software_list_ptr, &software_info_ptr, &software_part_ptr, &swlist_name);

	// if no match has been found, we suggest similar shortnames
	if (software_info_ptr == NULL)
	{
		software_display_matches(image->device().machine().config(),image->device().machine().options(), image->image_interface(), path);
	}

	if ( software_part_ptr )
	{
		/* Load the software part */
		try {
			result = image->call_softlist_load((char *)swlist_name, (char *)software_info_ptr->shortname, software_part_ptr->romdata );
		}
		catch (emu_fatalerror &fatal)
		{
			software_list_close( software_list_ptr );
			global_free(swlist_name);
			throw fatal;
		}

		/* Sanity checks */
		if (software_info_ptr->shortname == NULL)
			throw emu_fatalerror("Software entry is missing the name attribute!\n");
		if (software_info_ptr->longname == NULL)
			throw emu_fatalerror("Software entry '%s' is missing the description element!\n", software_info_ptr->shortname);

		/* Create a copy of the software and part information */
		*sw_info = auto_alloc_clear( image->device().machine(), software_info );
		(*sw_info)->shortname = auto_strdup( image->device().machine(), software_info_ptr->shortname );
		(*sw_info)->longname = auto_strdup( image->device().machine(), software_info_ptr->longname );
		if ( software_info_ptr->year )
			(*sw_info)->year = auto_strdup( image->device().machine(), software_info_ptr->year );
		if ( software_info_ptr->publisher )
			(*sw_info)->publisher = auto_strdup( image->device().machine(), software_info_ptr->publisher );

		(*sw_info)->partdata = (software_part *)auto_alloc_array_clear(image->device().machine(), UINT8, software_info_ptr->part_entries * sizeof(software_part) );
		software_part *new_part = (*sw_info)->partdata;
		for (software_part *swp = software_find_part(software_info_ptr, NULL, NULL); swp != NULL; swp = software_part_next(swp))
		{
			if (strcmp(software_part_ptr->name,swp->name)==0) *sw_part = new_part;

			new_part->name = auto_strdup( image->device().machine(), swp->name );
			if ( swp->interface_ )
				new_part->interface_ = auto_strdup( image->device().machine(), swp->interface_ );

			if ( swp->featurelist )
			{
				feature_list *list = swp->featurelist;
				feature_list *new_list = auto_alloc_clear( image->device().machine(), feature_list );

				new_part->featurelist = new_list;

				new_list->name = auto_strdup( image->device().machine(), list->name );
				new_list->value = auto_strdup( image->device().machine(), list->value );

				list = list->next;

				while( list )
				{
					new_list->next = auto_alloc_clear( image->device().machine(), feature_list );
					new_list = new_list->next;
					new_list->name = auto_strdup( image->device().machine(), list->name );
					new_list->value = auto_strdup( image->device().machine(), list->value );

					list = list->next;
				}
				new_list->next = NULL;
			}
			new_part++;
		}
		*list_name = auto_strdup( image->device().machine(), swlist_name );

		/* Tell the world which part we actually loaded */
		*full_sw_name = auto_alloc_array( image->device().machine(), char, strlen(swlist_name) + strlen(software_info_ptr->shortname) + strlen(software_part_ptr->name) + 3 );
		sprintf( *full_sw_name, "%s:%s:%s", swlist_name, software_info_ptr->shortname, software_part_ptr->name );

		software_list_device_iterator iter(image->device().machine().root_device());
		for (software_list_device *swlist = iter.first(); swlist != NULL; swlist = iter.next())
		{
			if (strcmp(swlist->list_name(),swlist_name)==0) {
				if (!is_software_compatible(software_part_ptr, swlist)) {
					mame_printf_warning("WARNING! the set %s might not work on this system due to missing filter(s) '%s'\n",software_info_ptr->shortname,swlist->filter());
				}
				break;
			}
		}

		{
			const char *requirement = software_part_get_feature(software_part_ptr, "requirement");
			if (requirement!=NULL) {
				software_list *req_software_list_ptr = NULL;
				software_info *req_software_info_ptr = NULL;
				software_part *req_software_part_ptr = NULL;
				const char *req_swlist_name = NULL;

				find_software_item(image->device().machine().config(), options, NULL, requirement, &req_software_list_ptr, &req_software_info_ptr, &req_software_part_ptr, &req_swlist_name);

				if ( req_software_list_ptr )
				{
					image_interface_iterator imgiter(image->device().machine().root_device());
					for (device_image_interface *req_image = imgiter.first(); req_image != NULL; req_image = imgiter.next())
					{
						const char *interface = req_image->image_interface();
						if (interface != NULL)
						{
							if (softlist_contain_interface(interface, req_software_part_ptr->interface_))
							{
								const char *option = options.value(req_image->brief_instance_name());
								// mount only if not already mounted
								if (strlen(option)==0 && !req_image->filename()) {
									req_image->set_init_phase();
									req_image->load(requirement);
								}
								break;
							}
						}
					}
					software_list_close( req_software_list_ptr );
					req_software_info_ptr = NULL;
					req_software_list_ptr = NULL;
					global_free(req_swlist_name);
				}
			}
		}
	}

	/* Close the software list if it's still open */
	if ( software_list_ptr )
	{
		software_list_close( software_list_ptr );
		software_info_ptr = NULL;
		software_list_ptr = NULL;
	}
	global_free(swlist_name);
	return result;
}


/*-------------------------------------------------
    software_part_get_feature
 -------------------------------------------------*/

const char *software_part_get_feature(const software_part *part, const char *feature_name)
{
	const feature_list *feature;

	if (part == NULL)
		return NULL;

	for (feature = part->featurelist; feature; feature = feature->next)
	{
		if (!strcmp(feature->name, feature_name))
			return feature->value;
	}

	return NULL;

}

/*-------------------------------------------------
    software_get_default_slot
 -------------------------------------------------*/

 const char *software_get_default_slot(const machine_config &config, emu_options &options, const device_image_interface *image, const char* default_card_slot)
{
	const char* retVal = NULL;
	const char* path = options.value(image->instance_name());
	software_list *software_list_ptr = NULL;
	software_info *software_info_ptr = NULL;
	software_part *software_part_ptr = NULL;
	const char *swlist_name = NULL;

	if (strlen(path)>0) {
		retVal = default_card_slot;
		find_software_item(config, options, image, path, &software_list_ptr, &software_info_ptr, &software_part_ptr, &swlist_name);
		if (software_part_ptr!=NULL) {
			const char *slot = software_part_get_feature(software_part_ptr, "slot");
			if (slot!=NULL) {
				retVal = core_strdup(slot);
			}
		}
		software_list_close(software_list_ptr);
		global_free(swlist_name);
	}
	return retVal;
}

/*-------------------------------------------------
    is_software_compatible
 -------------------------------------------------*/

bool is_software_compatible(const software_part *swpart, const software_list_device *swlist)
{
	const char *compatibility = software_part_get_feature(swpart, "compatibility");
	const char *filter = swlist->filter();
	if ((compatibility==NULL) || (filter==NULL)) return TRUE;
	astring comp = astring(compatibility,",");
	char *filt = core_strdup(filter);
	char *token = strtok(filt,",");
	while (token!= NULL)
	{
		if (comp.find(0,astring(token,","))!=-1) return TRUE;
		token = strtok (NULL, ",");
	}
	return FALSE;
}

/*-------------------------------------------------
    swinfo_has_multiple_parts
 -------------------------------------------------*/

bool swinfo_has_multiple_parts(const software_info *swinfo, const char *interface)
{
	int count = 0;

	for (const software_part *swpart = software_find_part(swinfo, NULL, NULL); swpart != NULL; swpart = software_part_next(swpart))
	{
		if (softlist_contain_interface(interface, swpart->interface_))
			count++;
	}
	return (count > 1) ? true : false;
}

/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/


void validate_error_proc(const char *message)
{
	mame_printf_error("%s", message);
}

void software_list_device::device_validity_check(validity_checker &valid) const
{
	// add to the global map whenever we check a list so we don't re-check
	// it in the future
	if (s_checked_lists.add(m_list_name, 1, false) == TMERR_DUPLICATE)
		return;

	// do device validation only in case of validate command
	if (strcmp(mconfig().options().command(), CLICOMMAND_VALIDATE) != 0) return;

	softlist_map names;
	softlist_map descriptions;

	enum { NAME_LEN_PARENT = 8, NAME_LEN_CLONE = 16 };

	software_list *list = software_list_open(mconfig().options(), m_list_name, FALSE, NULL);
	if ( list )
	{
		software_list_parse( list, &validate_error_proc, NULL );

		for (software_info *swinfo = software_list_find(list, "*", NULL); swinfo != NULL; swinfo = software_list_find(list, "*", swinfo))
		{
			const char *s;
			int is_clone = 0;

			/* First, check if the xml got corrupted: */

			/* Did we lost any description? */
			if (swinfo->longname == NULL)
			{
				mame_printf_error("%s: %s has no description\n", list->file->filename(), swinfo->shortname);
				break;
			}

			/* Did we lost any year? */
			if (swinfo->year == NULL)
			{
				mame_printf_error("%s: %s has no year\n", list->file->filename(), swinfo->shortname);
				break;
			}

			/* Did we lost any publisher? */
			if (swinfo->publisher == NULL)
			{
				mame_printf_error("%s: %s has no publisher\n", list->file->filename(), swinfo->shortname);
				break;
			}

			/* Second, since the xml is fine, run additional checks: */

			/* check for duplicate names */
			if (names.add(swinfo->shortname, swinfo, FALSE) == TMERR_DUPLICATE)
			{
				software_info *match = names.find(swinfo->shortname);
				mame_printf_error("%s: %s is a duplicate name (%s)\n", list->file->filename(), swinfo->shortname, match->shortname);
			}

			/* check for duplicate descriptions */
			if (descriptions.add(astring(swinfo->longname).makelower().cstr(), swinfo, FALSE) == TMERR_DUPLICATE)
				mame_printf_error("%s: %s is a duplicate description (%s)\n", list->file->filename(), swinfo->longname, swinfo->shortname);

			if (swinfo->parentname != NULL)
			{
				is_clone = 1;

				if (strcmp(swinfo->parentname, swinfo->shortname) == 0)
				{
					mame_printf_error("%s: %s is set as a clone of itself\n", list->file->filename(), swinfo->shortname);
					break;
				}

				/* make sure the parent exists */
				software_info *swinfo2 = software_list_find(list, swinfo->parentname, NULL );

				if (!swinfo2)
					mame_printf_error("%s: parent '%s' software for '%s' not found\n", list->file->filename(), swinfo->parentname, swinfo->shortname);
				else if (swinfo2->parentname != NULL)
					mame_printf_error("%s: %s is a clone of a clone\n", list->file->filename(), swinfo->shortname);
			}

			/* make sure the driver name is 8 chars or less */
			if ((is_clone && strlen(swinfo->shortname) > NAME_LEN_CLONE) || ((!is_clone) && strlen(swinfo->shortname) > NAME_LEN_PARENT))
				mame_printf_error("%s: %s %s driver name must be %d characters or less\n", list->file->filename(), swinfo->shortname,
								  is_clone ? "clone" : "parent", is_clone ? NAME_LEN_CLONE : NAME_LEN_PARENT);

			/* make sure the year is only digits, '?' or '+' */
			for (s = swinfo->year; *s; s++)
				if (!isdigit((UINT8)*s) && *s != '?' && *s != '+')
				{
					mame_printf_error("%s: %s has an invalid year '%s'\n", list->file->filename(), swinfo->shortname, swinfo->year);
					break;
				}

			softlist_map part_names;

			for (software_part *swpart = software_find_part(swinfo, NULL, NULL); swpart != NULL; swpart = software_part_next(swpart))
			{
				if (swpart->interface_ == NULL)
					mame_printf_error("%s: %s has a part (%s) without interface\n", list->file->filename(), swinfo->shortname, swpart->name);

				if (software_find_romdata(swpart, NULL) == NULL)
					mame_printf_error("%s: %s has a part (%s) with no data\n", list->file->filename(), swinfo->shortname, swpart->name);

				if (part_names.add(swpart->name, swinfo, FALSE) == TMERR_DUPLICATE)
					mame_printf_error("%s: %s has a part (%s) whose name is duplicate\n", list->file->filename(), swinfo->shortname, swpart->name);

				for (struct rom_entry *swdata = software_find_romdata(swpart, NULL); swdata != NULL;  swdata = software_romdata_next(swdata))
				{
					struct rom_entry *data = swdata;

					if (data->_name && data->_hashdata)
					{
						const char *str;

						/* make sure it's all lowercase */
						for (str = data->_name; *str; str++)
							if (tolower((UINT8)*str) != *str)
							{
								mame_printf_error("%s: %s has upper case ROM name %s\n", list->file->filename(), swinfo->shortname, data->_name);
								break;
							}

						/* make sure the hash is valid */
						hash_collection hashes;
						if (!hashes.from_internal_string(data->_hashdata))
							mame_printf_error("%s: %s has rom '%s' with an invalid hash string '%s'\n", list->file->filename(), swinfo->shortname, data->_name, data->_hashdata);
					}
				}
			}
		}
		software_list_close(list);
	}
}

bool softlist_contain_interface(const char *interface, const char *part_interface)
{
    bool result = FALSE;

	astring interfaces(interface);
	char *intf = strtok((char*)interfaces.cstr(),",");
	while (intf != NULL)
	{
		if (!strcmp(intf, part_interface))
        {
            result = TRUE;
            break;
        }
		intf = strtok (NULL, ",");
	}
    return result;
}

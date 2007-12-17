/***************************************************************************

    output.c

    General purpose output routines.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "output.h"
#include "mame.h"
#include <zlib.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define HASH_SIZE		53



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _output_notify output_notify;
struct _output_notify
{
	output_notify *		next;			/* link to next item */
	output_notifier		notifier;		/* callback to call */
	void *				param;			/* parameter to pass the callback */
};


typedef struct _output_item output_item;
struct _output_item
{
	output_item *		next;			/* next item in list */
	const char *		name;			/* string name of the item */
	UINT32				hash;			/* hash for this item name */
	UINT32				id;				/* unique ID for this item */
	INT32				value;			/* current value */
	output_notify *		notifylist;		/* list of notifier callbacks */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static output_item *itemtable[HASH_SIZE];
static output_notify *global_notifylist;
static UINT32 uniqueid = 12345;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void output_pause(running_machine *machine, int pause);
static void output_exit(running_machine *machine);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    copy_string - make a copy of a string
-------------------------------------------------*/

INLINE const char *copy_string(const char *string)
{
	char *newstring = malloc_or_die(strlen(string) + 1);
	strcpy(newstring, string);
	return newstring;
}


/*-------------------------------------------------
    get_hash - return the hash of an output value
-------------------------------------------------*/

INLINE UINT32 get_hash(const char *string)
{
	return crc32(0, (UINT8 *)string, (UINT32)strlen(string));
}


/*-------------------------------------------------
    find_item - find an item based on a string
-------------------------------------------------*/

INLINE output_item *find_item(const char *string)
{
	UINT32 hash = get_hash(string);
	output_item *item;

	/* use the hash as a starting point and find an entry */
	for (item = itemtable[hash % HASH_SIZE]; item != NULL; item = item->next)
		if (item->hash == hash && strcmp(string, item->name) == 0)
			return item;

	return NULL;
}


/*-------------------------------------------------
    create_new_item - create a new item
-------------------------------------------------*/

INLINE output_item *create_new_item(const char *outname, INT32 value)
{
	output_item *item = malloc_or_die(sizeof(*item));
	UINT32 hash = get_hash(outname);

	/* fill in the data */
	item->next = itemtable[hash % HASH_SIZE];
	item->name = copy_string(outname);
	item->hash = hash;
	item->id = uniqueid++;
	item->value = value;
	item->notifylist = NULL;

	/* add us to the hash table */
	itemtable[hash % HASH_SIZE] = item;
	return item;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    output_init - initialize everything
-------------------------------------------------*/

void output_init(running_machine *machine)
{
	/* add pause callback */
	add_pause_callback(machine, output_pause);

	/* get a callback when done */
	add_exit_callback(machine, output_exit);

	/* reset the lists */
	memset(itemtable, 0, sizeof(itemtable));
	global_notifylist = NULL;
}


/*-------------------------------------------------
    output_pause - send pause message
-------------------------------------------------*/

static void output_pause(running_machine *machine, int pause)
{
	output_set_value("pause", pause & 1);
}


/*-------------------------------------------------
    output_exit - cleanup on exit
-------------------------------------------------*/

static void output_exit(running_machine *machine)
{
	output_notify *notify;
	output_item *item;
	int hash;

	/* remove all items */
	for (hash = 0; hash < HASH_SIZE; hash++)
		for (item = itemtable[hash]; item != NULL; )
		{
			output_item *next = item->next;

			/* remove all notifiers */
			for (notify = item->notifylist; notify != NULL; )
			{
				output_notify *next = notify->next;
				free(notify);
				notify = next;
			}

			/* free the name and the item */
			if (item->name != NULL)
				free((void *)item->name);
			free(item);
			item = next;
		}

	/* remove all global notifiers */
	for (notify = global_notifylist; notify != NULL; )
	{
		output_notify *next = notify->next;
		free(notify);
		notify = next;
	}
}


/*-------------------------------------------------
    output_set_value - set the value of an output
-------------------------------------------------*/

void output_set_value(const char *outname, INT32 value)
{
	output_item *item = find_item(outname);
	output_notify *notify;
	INT32 oldval;

	/* if no item of that name, create a new one and send the item's state */
	if (item == NULL)
	{
		item = create_new_item(outname, value);
		oldval = value + 1;
	}

	else
	{
		/* set the new value */
		oldval = item->value;
		item->value = value;
	}

	/* if the value is different, signal the notifier */
	if (oldval != value)
	{
		/* call the local notifiers first */
		for (notify = item->notifylist; notify != NULL; notify = notify->next)
			(*notify->notifier)(outname, value, notify->param);

		/* call the global notifiers next */
		for (notify = global_notifylist; notify != NULL; notify = notify->next)
			(*notify->notifier)(outname, value, notify->param);
	}
}


/*-------------------------------------------------
    output_set_indexed_value - set the value of an
    indexed output
-------------------------------------------------*/

void output_set_indexed_value(const char *basename, int index, int value)
{
	char buffer[100];
	char *dest = buffer;

	/* copy the string */
	while (*basename != 0)
		*dest++ = *basename++;

	/* append the index */
	if (index >= 1000) *dest++ = '0' + ((index / 1000) % 10);
	if (index >= 100) *dest++ = '0' + ((index / 100) % 10);
	if (index >= 10) *dest++ = '0' + ((index / 10) % 10);
	*dest++ = '0' + (index % 10);
	*dest++ = 0;

	/* set the value */
	output_set_value(buffer, value);
}


/*-------------------------------------------------
    output_get_value - return the value of an
    output
-------------------------------------------------*/

INT32 output_get_value(const char *outname)
{
	output_item *item = find_item(outname);

	/* if no item, value is 0 */
	if (item == NULL)
		return 0;
	return item->value;
}


/*-------------------------------------------------
    output_get_indexed_value - get the value of an
    indexed output
-------------------------------------------------*/

INT32 output_get_indexed_value(const char *basename, int index)
{
	char buffer[100];
	char *dest = buffer;

	/* copy the string */
	while (*basename != 0)
		*dest++ = *basename++;

	/* append the index */
	if (index >= 1000) *dest++ = '0' + ((index / 1000) % 10);
	if (index >= 100) *dest++ = '0' + ((index / 100) % 10);
	if (index >= 10) *dest++ = '0' + ((index / 10) % 10);
	*dest++ = '0' + (index % 10);
	*dest++ = 0;

	/* set the value */
	return output_get_value(buffer);
}


/*-------------------------------------------------
    output_set_notifier - sets a notifier callback
    for a particular output, or for all outputs
    if NULL is specified
-------------------------------------------------*/

void output_set_notifier(const char *outname, output_notifier callback, void *param)
{
	output_notify **headptr;

	/* if an item is specified, find it */
	if (outname != NULL)
	{
		output_item *item = find_item(outname);

		/* if no item of that name, create a new one */
		if (item == NULL)
			item = create_new_item(outname, 0);
		headptr = &item->notifylist;
	}

	/* if no item is specified, we add to the global list */
	else
		headptr = &global_notifylist;

	/* find the end of the list and add to it */
	while (*headptr != NULL)
		headptr = &(*headptr)->next;
	*headptr = malloc_or_die(sizeof(**headptr));

	/* fill in the new record */
	(*headptr)->next = NULL;
	(*headptr)->notifier = callback;
	(*headptr)->param = param;
}


/*-------------------------------------------------
    output_notify_all - immediately call the given
    notifier for all outputs
-------------------------------------------------*/

void output_notify_all(output_notifier callback, void *param)
{
	output_item *item;
	int hash;

	/* remove all items */
	for (hash = 0; hash < HASH_SIZE; hash++)
		for (item = itemtable[hash]; item != NULL; item = item->next)
			(*callback)(item->name, item->value, param);
}


/*-------------------------------------------------
    output_name_to_id - returns a unique ID for
    a given name
-------------------------------------------------*/

UINT32 output_name_to_id(const char *outname)
{
	output_item *item = find_item(outname);

	/* if no item, ID is 0 */
	if (item == NULL)
		return 0;
	return item->id;
}


/*-------------------------------------------------
    output_id_to_name - returns a name that maps
    to a given unique ID
-------------------------------------------------*/

const char *output_id_to_name(UINT32 id)
{
	output_item *item;
	int hash;

	/* remove all items */
	for (hash = 0; hash < HASH_SIZE; hash++)
		for (item = itemtable[hash]; item != NULL; item = item->next)
			if (item->id == id)
				return item->name;

	/* nothing found, return NULL */
	return NULL;
}

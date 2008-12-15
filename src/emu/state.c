/***************************************************************************

    state.c

    Save state management functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Save state file format:

    00..07  'MAMESAVE'
    08      Format version (this is format 2)
    09      Flags
    0A..1B  Game name padded with \0
    1C..1F  Signature
    20..end Save game data (compressed)

    Data is always written as native-endian.
    Data is converted from the endiannness it was written upon load.

***************************************************************************/

#include "driver.h"
#include "astring.h"
#include <zlib.h>



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SAVE_VERSION		2
#define HEADER_SIZE			32

/* Available flags */
enum
{
	SS_MSB_FIRST = 0x02
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _state_entry state_entry;
struct _state_entry
{
	state_entry *		next;				/* pointer to next entry */
	running_machine *	machine;			/* pointer back to the owning machine */
	void *				data;				/* pointer to the memory to save/restore */
	astring *			name;				/* full name */
	UINT8				typesize;			/* size of the raw data type */
	UINT32				typecount;			/* number of items */
	UINT32				offset;				/* offset within the final structure */
};


typedef struct _state_callback state_callback;
struct _state_callback
{
	state_callback *	next;				/* pointer to next entry */
	running_machine *	machine;			/* pointer back to the owning machine */
	void *				param;				/* function parameter */
	union
	{
		state_presave_func presave;			/* presave callback */
		state_postload_func postload;		/* postload callback */
	} func;									/* function pointers */
};


/* In mame.h: typedef struct _state_private state_private; */
struct _state_private
{
	UINT8				reg_allowed;		/* are registrations allowed? */
	int					illegal_regs;		/* number of illegal registrations */

	state_entry *		entrylist;			/* list of live entries */
	state_callback *	prefunclist;		/* presave function list */
	state_callback *	postfunclist;		/* postsave function list */

	UINT8 *				ioarray;			/* array where we accumulate all the data */
	UINT32				ioarraysize;		/* size of the array */
	mame_file *			iofile;				/* file currently in use */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

#ifdef MESS
static const char ss_magic_num[8] = { 'M', 'E', 'S', 'S', 'S', 'A', 'V', 'E' };
#else
static const char ss_magic_num[8] = { 'M', 'A', 'M', 'E', 'S', 'A', 'V', 'E' };
#endif



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    flip_data - reverse the endianness of a
    block of  data
-------------------------------------------------*/

INLINE void flip_data(state_entry *entry)
{
	UINT16 *data16;
	UINT32 *data32;
	UINT64 *data64;
	int count;

	switch (entry->typesize)
	{
		case 2:
			data16 = entry->data;
			for (count = 0; count < entry->typecount; count++)
				data16[count] = FLIPENDIAN_INT16(data16[count]);
			break;

		case 4:
			data32 = entry->data;
			for (count = 0; count < entry->typecount; count++)
				data32[count] = FLIPENDIAN_INT32(data32[count]);
			break;

		case 8:
			data64 = entry->data;
			for (count = 0; count < entry->typecount; count++)
				data64[count] = FLIPENDIAN_INT64(data64[count]);
			break;
	}
}



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    state_init - initialize the system and reset
    all registrations
-------------------------------------------------*/

void state_init(running_machine *machine)
{
	machine->state_data = auto_malloc(sizeof(*machine->state_data));
	memset(machine->state_data, 0, sizeof(*machine->state_data));
}


/*-------------------------------------------------
    state_save_get_reg_count - return the number
    of total registrations so far
-------------------------------------------------*/

int state_save_get_reg_count(running_machine *machine)
{
	state_private *global = machine->state_data;
	state_entry *entry;
	int count = 0;

	/* count entries */
	for (entry = global->entrylist; entry != NULL; entry = entry->next)
		count++;

	return count;
}



/***************************************************************************
    REGISTRATION HANDLING
***************************************************************************/

/*-------------------------------------------------
    state_save_allow_registration - allow/disallow
    registrations to happen
-------------------------------------------------*/

void state_save_allow_registration(running_machine *machine, int allowed)
{
	/* allow/deny registration */
	machine->state_data->reg_allowed = allowed;
	if (!allowed)
		state_save_dump_registry(machine);
}


/*-------------------------------------------------
    state_save_registration_allowed - query
    whether or not registrations are allowed
-------------------------------------------------*/

int state_save_registration_allowed(running_machine *machine)
{
	return machine->state_data->reg_allowed;
}


/*-------------------------------------------------
    state_save_register_memory - register an
    array of data in memory
-------------------------------------------------*/

void state_save_register_memory(running_machine *machine, const char *module, const char *tag, UINT32 index, const char *name, void *val, UINT32 valsize, UINT32 valcount)
{
	state_private *global = machine->state_data;
	state_entry **entryptr, *next;
	astring *totalname;

	assert(valsize == 1 || valsize == 2 || valsize == 4 || valsize == 8);

	/* check for invalid timing */
	if (!global->reg_allowed)
	{
		logerror("Attempt to register save state entry after state registration is closed! module %s tag %s name %s\n",module, tag, name);
		if (machine->gamedrv->flags & GAME_SUPPORTS_SAVE)
			fatalerror("Attempt to register save state entry after state registration is closed! module %s tag %s name %s\n", module, tag, name);
		global->illegal_regs++;
		return;
	}

	/* create the full name */
	totalname = astring_alloc();
	if (tag != NULL)
		astring_printf(totalname, "%s/%s/%X/%s", module, tag, index, name);
	else
		astring_printf(totalname, "%s/%X/%s", module, index, name);

	/* look for duplicates and an entry to insert in front of */
	for (entryptr = &global->entrylist; *entryptr != NULL; entryptr = &(*entryptr)->next)
	{
		/* stop if the next guy's string is greater than ours */
		int cmpval = astring_cmp((*entryptr)->name, totalname);
		if (cmpval > 0)
			break;

		/* error if we are equal */
		if (cmpval == 0)
			fatalerror("Duplicate save state registration entry (%s)", astring_c(totalname));
	}

	/* didn't find one; allocate a new one */
	next = *entryptr;
	*entryptr = malloc_or_die(sizeof(**entryptr));
	memset(*entryptr, 0, sizeof(**entryptr));

	/* fill in the rest */
	(*entryptr)->next      = next;
	(*entryptr)->machine   = machine;
	(*entryptr)->data      = val;
	(*entryptr)->name      = totalname;
	(*entryptr)->typesize  = valsize;
	(*entryptr)->typecount = valcount;
	restrack_register_object(OBJTYPE_STATEREG, *entryptr, 0, __FILE__, __LINE__);
}


/*-------------------------------------------------
    state_save_register_bitmap - register a
    bitmap to be saved
-------------------------------------------------*/

void state_save_register_bitmap(running_machine *machine, const char *module, const char *tag, UINT32 index, const char *name, bitmap_t *val)
{
	state_save_register_memory(machine, module, tag, index, name, val->base, val->bpp / 8, val->rowpixels * val->height);
}



/***************************************************************************
    CALLBACK FUNCTION REGISTRATION
***************************************************************************/

/*-------------------------------------------------
    state_save_register_presave -
    register a pre-save function callback
-------------------------------------------------*/

void state_save_register_presave(running_machine *machine, state_presave_func func, void *param)
{
	state_private *global = machine->state_data;
	state_callback **cbptr;

	/* check for invalid timing */
	if (!global->reg_allowed)
		fatalerror("Attempt to register callback function after state registration is closed!");

	/* scan for duplicates and push through to the end */
	for (cbptr = &global->prefunclist; *cbptr != NULL; cbptr = &(*cbptr)->next)
		if ((*cbptr)->func.presave == func && (*cbptr)->param == param)
			fatalerror("Duplicate save state function (%p, %p)", param, func);

	/* allocate a new entry */
	*cbptr = malloc_or_die(sizeof(state_callback));

	/* fill it in */
	(*cbptr)->next         = NULL;
	(*cbptr)->machine      = machine;
	(*cbptr)->func.presave = func;
	(*cbptr)->param        = param;
	restrack_register_object(OBJTYPE_STATEREG, *cbptr, 1, __FILE__, __LINE__);
}


/*-------------------------------------------------
    state_save_register_postload -
    register a post-load function callback
-------------------------------------------------*/

void state_save_register_postload(running_machine *machine, state_postload_func func, void *param)
{
	state_private *global = machine->state_data;
	state_callback **cbptr;

	/* check for invalid timing */
	if (!global->reg_allowed)
		fatalerror("Attempt to register callback function after state registration is closed!");

	/* scan for duplicates and push through to the end */
	for (cbptr = &global->postfunclist; *cbptr != NULL; cbptr = &(*cbptr)->next)
		if ((*cbptr)->func.postload == func && (*cbptr)->param == param)
			fatalerror("Duplicate save state function (%p, %p)", param, func);

	/* allocate a new entry */
	*cbptr = malloc_or_die(sizeof(state_callback));

	/* fill it in */
	(*cbptr)->next          = NULL;
	(*cbptr)->machine       = machine;
	(*cbptr)->func.postload = func;
	(*cbptr)->param         = param;
	restrack_register_object(OBJTYPE_STATEREG, *cbptr, 2, __FILE__, __LINE__);
}



/***************************************************************************
    REGISTRATION FREEING
***************************************************************************/

/*-------------------------------------------------
    func_free - free registered functions attached
    to the current resource tag
-------------------------------------------------*/

static void func_free(state_callback **rootptr, void *ptr)
{
	state_callback **cbptr;

	/* iterate over the function list */
	for (cbptr = rootptr; *cbptr != NULL; cbptr = &(*cbptr)->next)
		if (*cbptr == ptr)
		{
			state_callback *cb = *cbptr;

			/* de-link us from the list and free our memory */
			*cbptr = (*cbptr)->next;
			free(cb);
			break;
		}
}


/*-------------------------------------------------
    state_save_free - free all registrations that
    have been tagged with the current resource
    tag
-------------------------------------------------*/

void state_destructor(void *ptr, size_t size)
{
	state_private *global = NULL;

	/* size of 0 means an entry */
	if (size == 0)
	{
		state_entry **entryptr;

		/* iterate over entries */
		global = ((state_entry *)ptr)->machine->state_data;
		for (entryptr = &global->entrylist; *entryptr != NULL; entryptr = &(*entryptr)->next)
			if (*entryptr == ptr)
			{
				state_entry *entry = *entryptr;

				/* de-link us from the list and free our memory */
				*entryptr = (*entryptr)->next;
				astring_free(entry->name);
				free(entry);
				break;
			}
	}

	/* size of 1 means a pre function */
	else if (size == 1)
	{
		global = ((state_callback *)ptr)->machine->state_data;
		func_free(&global->prefunclist, ptr);
	}

	/* size of 2 means a post function */
	else if (size == 2)
	{
		global = ((state_callback *)ptr)->machine->state_data;
		func_free(&global->postfunclist, ptr);
	}

	/* if we're clear of all registrations, reset the invalid counter */
	if (global != NULL && global->entrylist == NULL && global->prefunclist == NULL && global->postfunclist == NULL)
		global->illegal_regs = 0;
}



/***************************************************************************
    PROCESSING HELPERS
***************************************************************************/

/*-------------------------------------------------
    get_signature - compute the signature, which
    is a CRC over the structure of the data
-------------------------------------------------*/

static UINT32 get_signature(running_machine *machine)
{
	state_private *global = machine->state_data;
	state_entry *entry;
	UINT32 crc = 0;

	/* iterate over entries */
	for (entry = global->entrylist; entry != NULL; entry = entry->next)
	{
		UINT32 temp[2];

		/* add the entry name to the CRC */
		crc = crc32(crc, (UINT8 *)astring_c(entry->name), astring_len(entry->name));

		/* add the type and size to the CRC */
		temp[0] = LITTLE_ENDIANIZE_INT32(entry->typecount);
		temp[1] = LITTLE_ENDIANIZE_INT32(entry->typesize);
		crc = crc32(crc, (UINT8 *)&temp[0], sizeof(temp));
	}

	return crc;
}



/***************************************************************************
    SAVE STATE FILE PROCESSING
***************************************************************************/

/*-------------------------------------------------
    validate_header - validate the data in the
    header
-------------------------------------------------*/

static state_save_error validate_header(const UINT8 *header, const char *gamename, UINT32 signature,
	void (CLIB_DECL *errormsg)(const char *fmt, ...), const char *error_prefix)
{
	/* check magic number */
	if (memcmp(header, ss_magic_num, 8))
	{
		if (errormsg != NULL)
			(*errormsg)("%sThis is not a " APPNAME " save file", error_prefix);
		return STATERR_INVALID_HEADER;
	}

	/* check save state version */
	if (header[8] != SAVE_VERSION)
	{
		if (errormsg != NULL)
			(*errormsg)("%sWrong version in save file (version %d, expected %d)", error_prefix, header[8], SAVE_VERSION);
		return STATERR_INVALID_HEADER;
	}

	/* check gamename, if we were asked to */
	if (gamename != NULL && strncmp(gamename, (const char *)&header[0x0a], 0x1c - 0x0a))
	{
		if (errormsg != NULL)
			(*errormsg)("%s'File is not a valid savestate file for game '%s'.", error_prefix, gamename);
		return STATERR_INVALID_HEADER;
	}

	/* check signature, if we were asked to */
	if (signature != 0)
	{
		UINT32 rawsig = *(UINT32 *)&header[0x1c];
		if (signature != LITTLE_ENDIANIZE_INT32(rawsig))
		{
			if (errormsg != NULL)
				(*errormsg)("%sIncompatible save file (signature %08x, expected %08x)", error_prefix, LITTLE_ENDIANIZE_INT32(rawsig), signature);
			return STATERR_INVALID_HEADER;
		}
	}
	return STATERR_NONE;
}


/*-------------------------------------------------
    state_save_check_file - check if a file is
    a valid save state
-------------------------------------------------*/

state_save_error state_save_check_file(running_machine *machine, mame_file *file, const char *gamename, void (CLIB_DECL *errormsg)(const char *fmt, ...))
{
	UINT8 header[HEADER_SIZE];
	UINT32 signature = 0;

	/* if we want to validate the signature, compute it */
	if (machine != NULL)
		signature = get_signature(machine);

	/* seek to the beginning and read the header */
	mame_fcompress(file, FCOMPRESS_NONE);
	mame_fseek(file, 0, SEEK_SET);
	if (mame_fread(file, header, sizeof(header)) != sizeof(header))
	{
		if (errormsg != NULL)
			(*errormsg)("Could not read " APPNAME " save file header");
		return STATERR_READ_ERROR;
	}

	/* let the generic header check work out the rest */
	return validate_header(header, gamename, signature, errormsg, "");
}


/*-------------------------------------------------
    state_save_write_file - writes the data to
    a file
-------------------------------------------------*/

state_save_error state_save_write_file(running_machine *machine, mame_file *file)
{
	state_private *global = machine->state_data;
	UINT32 signature = get_signature(machine);
	UINT8 header[HEADER_SIZE];
	state_callback *func;
	state_entry *entry;

	/* if we have illegal registrations, return an error */
	if (global->illegal_regs > 0)
		return STATERR_ILLEGAL_REGISTRATIONS;

	/* generate the header */
	memcpy(&header[0], ss_magic_num, 8);
	header[8] = SAVE_VERSION;
#ifdef LSB_FIRST
	header[9] = 0;
#else
	header[9] = SS_MSB_FIRST;
#endif
	strncpy((char *)&header[0x0a], machine->gamedrv->name, 0x1c - 0x0a);
	*(UINT32 *)&header[0x1c] = LITTLE_ENDIANIZE_INT32(signature);

	/* write the header and turn on compression for the rest of the file */
	mame_fcompress(file, FCOMPRESS_NONE);
	mame_fseek(file, 0, SEEK_SET);
	if (mame_fwrite(file, header, sizeof(header)) != sizeof(header))
		return STATERR_WRITE_ERROR;
	mame_fcompress(file, FCOMPRESS_MEDIUM);

	/* call the pre-save functions */
	for (func = global->prefunclist; func != NULL; func = func->next)
		(*func->func.presave)(machine, func->param);

	/* then write all the data */
	for (entry = global->entrylist; entry != NULL; entry = entry->next)
	{
		UINT32 totalsize = entry->typesize * entry->typecount;
		if (mame_fwrite(file, entry->data, totalsize) != totalsize)
			return STATERR_WRITE_ERROR;
	}
	return STATERR_NONE;
}


/*-------------------------------------------------
    state_save_read_file - read the data from a
    file
-------------------------------------------------*/

state_save_error state_save_read_file(running_machine *machine, mame_file *file)
{
	state_private *global = machine->state_data;
	UINT32 signature = get_signature(machine);
	UINT8 header[HEADER_SIZE];
	state_callback *func;
	state_entry *entry;
	int flip;

	/* if we have illegal registrations, return an error */
	if (global->illegal_regs > 0)
		return STATERR_ILLEGAL_REGISTRATIONS;

	/* read the header and turn on compression for the rest of the file */
	mame_fcompress(file, FCOMPRESS_NONE);
	mame_fseek(file, 0, SEEK_SET);
	if (mame_fread(file, header, sizeof(header)) != sizeof(header))
		return STATERR_READ_ERROR;
	mame_fcompress(file, FCOMPRESS_MEDIUM);

	/* verify the header and report an error if it doesn't match */
	if (validate_header(header, machine->gamedrv->name, signature, popmessage, "Error: ")  != STATERR_NONE)
		return STATERR_INVALID_HEADER;

	/* determine whether or not to flip the data when done */
#ifdef LSB_FIRST
	flip = ((header[9] & SS_MSB_FIRST) != 0);
#else
	flip = ((header[9] & SS_MSB_FIRST) == 0);
#endif

	/* read all the data, flipping if necessary */
	for (entry = global->entrylist; entry != NULL; entry = entry->next)
	{
		UINT32 totalsize = entry->typesize * entry->typecount;
		if (mame_fread(file, entry->data, totalsize) != totalsize)
			return STATERR_READ_ERROR;

		/* handle flipping */
		if (flip)
			flip_data(entry);
	}

	/* call the post-load functions */
	for (func = global->postfunclist; func != NULL; func = func->next)
		(*func->func.postload)(machine, func->param);

	return STATERR_NONE;
}



/***************************************************************************
    DEBUGGING
***************************************************************************/

/*-------------------------------------------------
    state_save_get_indexed_item - return an item
    with the given index
-------------------------------------------------*/

const char *state_save_get_indexed_item(running_machine *machine, int index, void **base, UINT32 *valsize, UINT32 *valcount)
{
	state_private *global = machine->state_data;
	state_entry *ss;

	for (ss = global->entrylist; ss != NULL; ss = ss->next)
		if (index-- == 0)
		{
			if (base != NULL)
				*base = ss->data;
			if (valsize != NULL)
				*valsize = ss->typesize;
			if (valcount != NULL)
				*valcount = ss->typecount;
			return astring_c(ss->name);
		}

	return NULL;
}


/*-------------------------------------------------
    state_save_dump_registry - dump the registry
    to the logfile
-------------------------------------------------*/

void state_save_dump_registry(running_machine *machine)
{
	state_private *global = machine->state_data;
	state_entry *entry;

	for (entry = global->entrylist; entry; entry=entry->next)
		LOG(("%s: %d x %d\n", astring_c(entry->name), entry->typesize, entry->typecount));
}


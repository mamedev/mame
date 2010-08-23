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

#include "emu.h"

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
	astring				name;				/* full name */
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
			data16 = (UINT16 *)entry->data;
			for (count = 0; count < entry->typecount; count++)
				data16[count] = FLIPENDIAN_INT16(data16[count]);
			break;

		case 4:
			data32 = (UINT32 *)entry->data;
			for (count = 0; count < entry->typecount; count++)
				data32[count] = FLIPENDIAN_INT32(data32[count]);
			break;

		case 8:
			data64 = (UINT64 *)entry->data;
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

enum test_enum_type { test_val };

class test_class_type { public: int dummy; };

void state_init(running_machine *machine)
{
	bool test_bool = false;
	INT8 test_INT8 = 0;
	UINT8 test_UINT8 = 0;
	INT16 test_INT16 = 0;
	UINT16 test_UINT16 = 0;
	INT32 test_INT32 = 0;
	UINT32 test_UINT32 = 0;
	INT64 test_INT64 = 0;
	UINT64 test_UINT64 = 0;
	float test_float = 0.0f;
	double test_double = 0.0;
	test_enum_type test_enum = test_val;
#ifdef __GNUC__
	test_class_type test_class;
#endif

	assert_always(IS_VALID_SAVE_TYPE(test_bool), "bool is not a valid type for save");
	assert_always(IS_VALID_SAVE_TYPE(test_INT8), "INT8 is not a valid type for save");
	assert_always(IS_VALID_SAVE_TYPE(test_UINT8), "UINT8 is not a valid type for save");
	assert_always(IS_VALID_SAVE_TYPE(test_INT16), "INT16 is not a valid type for save");
	assert_always(IS_VALID_SAVE_TYPE(test_UINT16), "UINT16 is not a valid type for save");
	assert_always(IS_VALID_SAVE_TYPE(test_INT32), "INT32 is not a valid type for save");
	assert_always(IS_VALID_SAVE_TYPE(test_UINT32), "UINT32 is not a valid type for save");
	assert_always(IS_VALID_SAVE_TYPE(test_INT64), "INT64 is not a valid type for save");
	assert_always(IS_VALID_SAVE_TYPE(test_UINT64), "UINT64 is not a valid type for save");
	assert_always(IS_VALID_SAVE_TYPE(test_float), "float is not a valid type for save");
	assert_always(IS_VALID_SAVE_TYPE(test_double), "double is not a valid type for save");
	assert_always(IS_VALID_SAVE_TYPE(test_enum), "enums are not a valid type for save");
#ifdef __GNUC__
	assert_always(!IS_VALID_SAVE_TYPE(test_class), "classes are a valid type for save");
#endif

	machine->state_data = auto_alloc_clear(machine, state_private);
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

void state_save_register_memory(running_machine *machine, const char *module, const char *tag, UINT32 index, const char *name, void *val, UINT32 valsize, UINT32 valcount, const char *file, int line)
{
	state_private *global = machine->state_data;
	state_entry **entryptr, *next;
	astring totalname;

	assert(valsize == 1 || valsize == 2 || valsize == 4 || valsize == 8);

	/* check for invalid timing */
	if (!global->reg_allowed)
	{
		logerror("Attempt to register save state entry after state registration is closed!\nFile: %s, line %d, module %s tag %s name %s\n", file, line, module, tag, name);
		if (machine->gamedrv->flags & GAME_SUPPORTS_SAVE)
			fatalerror("Attempt to register save state entry after state registration is closed!\nFile: %s, line %d, module %s tag %s name %s\n", file, line, module, tag, name);
		global->illegal_regs++;
		return;
	}

	/* create the full name */
	if (tag != NULL)
		totalname.printf("%s/%s/%X/%s", module, tag, index, name);
	else
		totalname.printf("%s/%X/%s", module, index, name);

	/* look for duplicates and an entry to insert in front of */
	for (entryptr = &global->entrylist; *entryptr != NULL; entryptr = &(*entryptr)->next)
	{
		/* stop if the next guy's string is greater than ours */
		int cmpval = (*entryptr)->name.cmp(totalname);
		if (cmpval > 0)
			break;

		/* error if we are equal */
		if (cmpval == 0)
			fatalerror("Duplicate save state registration entry (%s)", totalname.cstr());
	}

	/* didn't find one; allocate a new one */
	next = *entryptr;
	*entryptr = auto_alloc_clear(machine, state_entry);

	/* fill in the rest */
	(*entryptr)->next      = next;
	(*entryptr)->machine   = machine;
	(*entryptr)->data      = val;
	(*entryptr)->name      = totalname;
	(*entryptr)->typesize  = valsize;
	(*entryptr)->typecount = valcount;
}


/*-------------------------------------------------
    state_save_register_bitmap - register a
    bitmap to be saved
-------------------------------------------------*/

void state_save_register_bitmap(running_machine *machine, const char *module, const char *tag, UINT32 index, const char *name, bitmap_t *val, const char *file, int line)
{
	state_save_register_memory(machine, module, tag, index, name, val->base, val->bpp / 8, val->rowpixels * val->height, file, line);
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
	*cbptr = auto_alloc(machine, state_callback);

	/* fill it in */
	(*cbptr)->next         = NULL;
	(*cbptr)->machine      = machine;
	(*cbptr)->func.presave = func;
	(*cbptr)->param        = param;
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
	*cbptr = auto_alloc(machine, state_callback);

	/* fill it in */
	(*cbptr)->next          = NULL;
	(*cbptr)->machine       = machine;
	(*cbptr)->func.postload = func;
	(*cbptr)->param         = param;
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
		crc = crc32(crc, (UINT8 *)entry->name.cstr(), entry->name.len());

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
	header[9] = NATIVE_ENDIAN_VALUE_LE_BE(0, SS_MSB_FIRST);
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
	flip = NATIVE_ENDIAN_VALUE_LE_BE((header[9] & SS_MSB_FIRST) != 0, (header[9] & SS_MSB_FIRST) == 0);

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
			return ss->name;
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
		LOG(("%s: %d x %d\n", entry->name.cstr(), entry->typesize, entry->typecount));
}


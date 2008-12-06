/***************************************************************************

    state.c

    Save state management functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Save state file format:

     0.. 7  'MAMESAVE"
     8      Format version (this is format 1)
     9      Flags
     a..13  Game name padded with \0
    14..17  Signature
    18..end Save game data

***************************************************************************/

#include "driver.h"
#include "astring.h"
#include "deprecat.h"
#include <zlib.h>



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SAVE_VERSION		1

#define TAG_STACK_SIZE		4

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
	int					tag;				/* saving tag */
	UINT32				offset;				/* offset within the final structure */
};


typedef struct _state_callback state_callback;
struct _state_callback
{
	state_callback *	next;				/* pointer to next entry */
	running_machine *	machine;			/* pointer back to the owning machine */
	void *				param;				/* function parameter */
	int					tag;				/* saving tag */
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

/* this stuff goes away when CPU cores are fully pointer-ized */
static int ss_tag_stack[TAG_STACK_SIZE];
static int ss_tag_stack_index;
static int ss_current_tag;

#ifdef MESS
static const char ss_magic_num[8] = { 'M', 'E', 'S', 'S', 'S', 'A', 'V', 'E' };
#else
static const char ss_magic_num[8] = { 'M', 'A', 'M', 'E', 'S', 'A', 'V', 'E' };
#endif



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void ss_c2(UINT8 *, UINT32);
static void ss_c4(UINT8 *, UINT32);
static void ss_c8(UINT8 *, UINT32);

static void (*const ss_conv[])(UINT8 *, UINT32) = { 0, 0, ss_c2, 0, ss_c4, 0, 0, 0, ss_c8 };



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

	/* iterate over entries with matching tags */
	for (entry = global->entrylist; entry != NULL; entry = entry->next)
		count++;

	return count;
}



/***************************************************************************
    TAGGING
***************************************************************************/

/*-------------------------------------------------
    state_save_push_tag - push the current tag
    onto the stack and set a new tag
-------------------------------------------------*/

void state_save_push_tag(int tag)
{
	if (ss_tag_stack_index == TAG_STACK_SIZE - 1)
		fatalerror("state_save tag stack overflow");
	ss_tag_stack[ss_tag_stack_index++] = ss_current_tag;
	ss_current_tag = tag;
}


/*-------------------------------------------------
    state_save_pop_tag - pop the tag from the top
    of the stack
-------------------------------------------------*/

void state_save_pop_tag(void)
{
	if (ss_tag_stack_index == 0)
		fatalerror("state_save tag stack underflow");
	ss_current_tag = ss_tag_stack[--ss_tag_stack_index];
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
		if (Machine->gamedrv->flags & GAME_SUPPORTS_SAVE)
			fatalerror("Attempt to register save state entry after state registration is closed! module %s tag %s name %s\n", module, tag, name);
		global->illegal_regs++;
		return;
	}

	/* create the full name */
	totalname = astring_alloc();
	if (tag != NULL)
		astring_printf(totalname, "%X/%s/%s/%X/%s", ss_current_tag, module, tag, index, name);
	else
		astring_printf(totalname, "%X/%s/%X/%s", ss_current_tag, module, index, name);

	/* look for duplicates and an entry to insert in front of */
	for (entryptr = &global->entrylist; *entryptr != NULL; entryptr = &(*entryptr)->next)
	{
		/* stop if the next guy's string is greater than ours */
		int cmpval = astring_cmp((*entryptr)->name, totalname);
		if (cmpval > 0)
			break;

		/* error if we are equal */
		if ((*entryptr)->tag == ss_current_tag && cmpval == 0)
			fatalerror("Duplicate save state registration entry (%d, %s)", ss_current_tag, astring_c(totalname));
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
	(*entryptr)->tag       = ss_current_tag;
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
		if ((*cbptr)->func.presave == func && (*cbptr)->param == param && (*cbptr)->tag == ss_current_tag)
			fatalerror("Duplicate save state function (%d, %p, %p)", ss_current_tag, param, func);

	/* allocate a new entry */
	*cbptr = malloc_or_die(sizeof(state_callback));

	/* fill it in */
	(*cbptr)->next         = NULL;
	(*cbptr)->machine      = machine;
	(*cbptr)->func.presave = func;
	(*cbptr)->param        = param;
	(*cbptr)->tag          = ss_current_tag;
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
		if ((*cbptr)->func.postload == func && (*cbptr)->param == param && (*cbptr)->tag == ss_current_tag)
			fatalerror("Duplicate save state function (%d, %p, %p)", ss_current_tag, param, func);

	/* allocate a new entry */
	*cbptr = malloc_or_die(sizeof(state_callback));

	/* fill it in */
	(*cbptr)->next          = NULL;
	(*cbptr)->machine       = machine;
	(*cbptr)->func.postload = func;
	(*cbptr)->param         = param;
	(*cbptr)->tag           = ss_current_tag;
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
    ENDIAN CONVERSION
***************************************************************************/

/*-------------------------------------------------
    ss_c2 - byte swap an array of 16-bit data
-------------------------------------------------*/

static void ss_c2(UINT8 *data, UINT32 size)
{
	UINT16 *convert = (UINT16 *)data;
	unsigned i;

	for (i = 0; i < size; i++)
		convert[i] = FLIPENDIAN_INT16(convert[i]);
}


/*-------------------------------------------------
    ss_c4 - byte swap an array of 32-bit data
-------------------------------------------------*/

static void ss_c4(UINT8 *data, UINT32 size)
{
	UINT32 *convert = (UINT32 *)data;
	unsigned i;

	for (i = 0; i < size; i++)
		convert[i] = FLIPENDIAN_INT32(convert[i]);
}


/*-------------------------------------------------
    ss_c8 - byte swap an array of 64-bit data
-------------------------------------------------*/

static void ss_c8(UINT8 *data, UINT32 size)
{
	UINT64 *convert = (UINT64 *)data;
	unsigned i;

	for (i = 0; i < size; i++)
		convert[i] = FLIPENDIAN_INT64(convert[i]);
}



/***************************************************************************
    PROCESSING HELPERS
***************************************************************************/

/*-------------------------------------------------
    compute_size_and_offsets - compute the total
    size and offsets of each individual item
-------------------------------------------------*/

static int compute_size_and_offsets(state_private *global)
{
	state_entry *entry;
	int total_size;

	/* start with the header size */
	total_size = 0x18;

	/* iterate over entries */
	for (entry = global->entrylist; entry; entry = entry->next)
	{
		/* note the offset and accumulate a total size */
		entry->offset = total_size;
		total_size += entry->typesize * entry->typecount;
	}

	/* return the total size */
	return total_size;
}


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
	for (entry = global->entrylist; entry; entry = entry->next)
	{
		UINT32 temp[2];

		/* add the entry name to the CRC */
		crc = crc32(crc, (UINT8 *)astring_c(entry->name), astring_len(entry->name));

		/* add the type and size to the CRC */
		temp[0] = LITTLE_ENDIANIZE_INT32(entry->typecount);
		temp[1] = LITTLE_ENDIANIZE_INT32(entry->typesize);
		crc = crc32(crc, (UINT8 *)&temp[0], 8);
	}

	return crc;
}



/***************************************************************************
    STATE FILE VALIDATION
***************************************************************************/

/*-------------------------------------------------
    validate_header - validate the data in the
    header
-------------------------------------------------*/

static int validate_header(const UINT8 *header, const char *gamename, UINT32 signature,
	void (CLIB_DECL *errormsg)(const char *fmt, ...), const char *error_prefix)
{
	/* check magic number */
	if (memcmp(header, ss_magic_num, 8))
	{
		if (errormsg)
			errormsg("%sThis is not a " APPNAME " save file", error_prefix);
		return -1;
	}

	/* check save state version */
	if (header[8] != SAVE_VERSION)
	{
		if (errormsg)
			errormsg("%sWrong version in save file (%d, 1 expected)", error_prefix, header[8]);
		return -1;
	}

	/* check gamename, if we were asked to */
	if (gamename && strcmp(gamename, (const char *)&header[10]))
	{
		if (errormsg)
			errormsg("%s'%s' is not a valid savestate file for game '%s'.", error_prefix, gamename);
		return -1;
	}

	/* check signature, if we were asked to */
	if (signature)
	{
		UINT32 rawsig = *(UINT32 *)&header[0x14];
		UINT32 filesig = LITTLE_ENDIANIZE_INT32(rawsig);
		if (signature != filesig)
		{
			if (errormsg)
				errormsg("%sIncompatible save file (signature %08x, expected %08x)", error_prefix, filesig, signature);
			return -1;
		}
	}
	return 0;
}


/*-------------------------------------------------
    state_save_check_file - check if a file is
    a valid save state
-------------------------------------------------*/

int state_save_check_file(running_machine *machine, mame_file *file, const char *gamename, void (CLIB_DECL *errormsg)(const char *fmt, ...))
{
	UINT32 signature = 0;
	UINT8 header[0x18];

	/* if we want to validate the signature, compute it */
	if (machine != NULL)
		signature = get_signature(machine);

	/* seek to the beginning and read the header */
	mame_fseek(file, 0, SEEK_SET);
	if (mame_fread(file, header, sizeof(header)) != sizeof(header))
	{
		if (errormsg)
			errormsg("Could not read " APPNAME " save file header");
		return -1;
	}

	/* let the generic header check work out the rest */
	return validate_header(header, gamename, signature, errormsg, "");
}



/***************************************************************************
    SAVE STATE PROCESSING
***************************************************************************/

/*-------------------------------------------------
    state_save_save_begin - begin the process of
    saving
-------------------------------------------------*/

int state_save_save_begin(running_machine *machine, mame_file *file)
{
	state_private *global = machine->state_data;

	/* if we have illegal registrations, return an error */
	if (global->illegal_regs > 0)
		return 1;

	LOG(("Beginning save\n"));
	global->iofile = file;

	/* compute the total dump size and the offsets of each element */
	global->ioarraysize = compute_size_and_offsets(global);
	LOG(("   total size %u\n", global->ioarraysize));

	/* allocate memory for the array */
	global->ioarray = malloc_or_die(global->ioarraysize);
	return 0;
}


/*-------------------------------------------------
    state_save_save_continue - save within the
    current tag
-------------------------------------------------*/

void state_save_save_continue(running_machine *machine)
{
	state_private *global = machine->state_data;
	state_entry *entry;
	state_callback *func;
	int count = 0;

	LOG(("Saving tag %d\n", ss_current_tag));

	/* call the pre-save functions */
	LOG(("  calling pre-save functions\n"));

	/* iterate over the list of functions */
	for (func = global->prefunclist; func != NULL; func = func->next)
		if (func->tag == ss_current_tag)
		{
			count++;
			(*func->func.presave)(machine, func->param);
		}
	LOG(("    %d functions called\n", count));

	/* then copy in all the data */
	LOG(("  copying data\n"));

	/* iterate over entries with matching tags */
	for (entry = global->entrylist; entry != NULL; entry = entry->next)
		if (entry->tag == ss_current_tag)
		{
			memcpy(global->ioarray + entry->offset, entry->data, entry->typesize * entry->typecount);
			LOG(("    %s: %x..%x\n", astring_c(entry->name), entry->offset, entry->offset + entry->typesize * entry->typecount - 1));
		}
}


/*-------------------------------------------------
    state_save_save_finish - finish saving the
    file by writing the header and closing
-------------------------------------------------*/

void state_save_save_finish(running_machine *machine)
{
	state_private *global = machine->state_data;
	UINT32 signature;
	UINT8 flags = 0;

	LOG(("Finishing save\n"));

	/* compute the flags */
#ifndef LSB_FIRST
	flags |= SS_MSB_FIRST;
#endif

	/* build up the header */
	memcpy(global->ioarray, ss_magic_num, 8);
	global->ioarray[8] = SAVE_VERSION;
	global->ioarray[9] = flags;
	memset(global->ioarray+0xa, 0, 10);
	strcpy((char *)global->ioarray+0xa, machine->gamedrv->name);

	/* copy in the signature */
	signature = get_signature(machine);
	*(UINT32 *)&global->ioarray[0x14] = LITTLE_ENDIANIZE_INT32(signature);

	/* write the file */
	mame_fwrite(global->iofile, global->ioarray, global->ioarraysize);

	/* free memory and reset the global states */
	free(global->ioarray);
	global->ioarray = NULL;
	global->ioarraysize = 0;
	global->iofile = NULL;
}



/***************************************************************************
    LOAD STATE PROCESSING
***************************************************************************/

/*-------------------------------------------------
    state_save_load_begin - begin the process
    of loading the state
-------------------------------------------------*/

int state_save_load_begin(running_machine *machine, mame_file *file)
{
	state_private *global = machine->state_data;

	LOG(("Beginning load\n"));

	/* read the file into memory */
	global->ioarraysize = mame_fsize(file);
	global->ioarray = malloc_or_die(global->ioarraysize);
	global->iofile = file;
	mame_fread(global->iofile, global->ioarray, global->ioarraysize);

	/* verify the header and report an error if it doesn't match */
	if (validate_header(global->ioarray, NULL, get_signature(machine), popmessage, "Error: "))
	{
		free(global->ioarray);
		global->ioarray = NULL;
		global->ioarraysize = 0;
		global->iofile = NULL;
		return 1;
	}

	/* compute the total size and offset of all the entries */
	compute_size_and_offsets(global);
	return 0;
}


/*-------------------------------------------------
    state_save_load_continue - load all state in
    the current tag
-------------------------------------------------*/

void state_save_load_continue(running_machine *machine)
{
	state_private *global = machine->state_data;
	state_entry *entry;
	state_callback *func;
	int need_convert;
	int count = 0;

	/* first determine whether or not we need to convert the endianness of the data */
#ifdef LSB_FIRST
	need_convert = (global->ioarray[9] & SS_MSB_FIRST) != 0;
#else
	need_convert = (global->ioarray[9] & SS_MSB_FIRST) == 0;
#endif

	LOG(("Loading tag %d\n", ss_current_tag));
	LOG(("  copying data\n"));

	/* iterate over entries with matching tags */
	for (entry = global->entrylist; entry != NULL; entry = entry->next)
		if (entry->tag == ss_current_tag)
		{
			memcpy(entry->data, global->ioarray + entry->offset, entry->typesize * entry->typecount);
			if (need_convert && ss_conv[entry->typesize])
				(*ss_conv[entry->typesize])(entry->data, entry->typecount);
			LOG(("    %s: %x..%x\n", astring_c(entry->name), entry->offset, entry->offset + entry->typesize * entry->typecount - 1));
		}

	/* call the post-load functions */
	LOG(("  calling post-load functions\n"));
	for (func = global->postfunclist; func != NULL; func = func->next)
		if (func->tag == ss_current_tag)
		{
			count++;
			(*func->func.postload)(machine, func->param);
		}
	LOG(("    %d functions called\n", count));
}


/*-------------------------------------------------
    state_save_load_finish - complete the process
    of loading the state
-------------------------------------------------*/

void state_save_load_finish(running_machine *machine)
{
	state_private *global = machine->state_data;

	LOG(("Finishing load\n"));

	/* free memory and reset the global states */
	free(global->ioarray);
	global->ioarray = NULL;
	global->ioarraysize = 0;
	global->iofile = NULL;
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


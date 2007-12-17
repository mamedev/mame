/***************************************************************************

    state.c

    Save state management functions.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
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
#include <zlib.h>



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE

#ifdef VERBOSE
#define TRACE(x) do {x;} while (0)
#else
#define TRACE(x)
#endif



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

enum
{
	FUNC_NOPARAM,
	FUNC_INTPARAM,
	FUNC_PTRPARAM
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _ss_entry ss_entry;
struct _ss_entry
{
	ss_entry *		next;				/* pointer to next entry */
	void *			data;				/* pointer to the memory to save/restore */
	astring *		name;				/* full name */
	UINT8			typesize;			/* size of the raw data type */
	UINT32			typecount;			/* number of items */
	int				tag;				/* saving tag */
	UINT32			offset;				/* offset within the final structure */
};


typedef struct _ss_func ss_func;
struct _ss_func
{
	ss_func *		next;				/* pointer to next entry */
	int				type;				/* type of callback */
	union
	{
		void (*voidf)(void);
		void (*intf)(int param);
		void (*ptrf)(void *param);
	} func;								/* function pointers */
	union
	{
		int intp;
		void *ptrp;
	} param;							/* parameters */
	int				tag;				/* saving tag */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static int ss_illegal_regs;
static ss_entry *ss_registry;
static ss_func *ss_prefunc_reg;
static ss_func *ss_postfunc_reg;

static int ss_tag_stack[TAG_STACK_SIZE];
static int ss_tag_stack_index;
static int ss_current_tag;
static UINT8 ss_registration_allowed;

static UINT8 *ss_dump_array;
static mame_file *ss_dump_file;
static UINT32 ss_dump_size;

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

static void (*ss_conv[])(UINT8 *, UINT32) = { 0, 0, ss_c2, 0, ss_c4, 0, 0, 0, ss_c8 };



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    state_init - initialize the system and reset
    all registrations
-------------------------------------------------*/

void state_init(running_machine *machine)
{
	ss_illegal_regs = 0;
	ss_current_tag = 0;
	ss_tag_stack_index = 0;
	ss_registration_allowed = FALSE;
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


/*-------------------------------------------------
    state_save_get_reg_count - count the number of
    registered entries on the current tag
-------------------------------------------------*/

int state_save_get_reg_count(void)
{
	ss_entry *entry;
	int count = 0;

	/* iterate over entries with matching tags */
	for (entry = ss_registry; entry; entry = entry->next)
		if (entry->tag == ss_current_tag)
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

void state_save_allow_registration(int allowed)
{
	/* allow/deny registration */
	ss_registration_allowed = allowed;
}


/*-------------------------------------------------
    state_save_registration_allowed - query
    whether or not registrations are allowed
-------------------------------------------------*/

int state_save_registration_allowed(void)
{
	return ss_registration_allowed;
}


/*-------------------------------------------------
    state_save_register_memory - register an
    array of data in memory
-------------------------------------------------*/

void state_save_register_memory(const char *module, UINT32 instance, const char *name, void *val, UINT32 valsize, UINT32 valcount)
{
	ss_entry **entry, *next;
	astring *totalname;

	assert(valsize == 1 || valsize == 2 || valsize == 4 || valsize == 8);

	/* check for invalid timing */
	if (!ss_registration_allowed)
	{
		logerror("Attempt to register save state entry after state registration is closed! module %s name %s\n",module,name);
		ss_illegal_regs++;
		return;
	}

	/* create the full name */
	totalname = astring_alloc();
	astring_printf(totalname, "%X/%s/%X/%s", ss_current_tag, module, instance, name);

	/* look for duplicates and an entry to insert in front of */
	for (entry = &ss_registry; *entry; entry = &(*entry)->next)
	{
		/* stop if the next guy's string is greater than ours */
		int cmpval = astring_cmp((*entry)->name, totalname);
		if (cmpval > 0)
			break;

		/* error if we are equal */
		if ((*entry)->tag == ss_current_tag && cmpval == 0)
			fatalerror("Duplicate save state registration entry (%d, %s)", ss_current_tag, astring_c(totalname));
	}

	/* didn't find one; allocate a new one */
	next = *entry;
	*entry = malloc_or_die(sizeof(**entry));
	memset(*entry, 0, sizeof(**entry));

	/* fill in the rest */
	(*entry)->next      = next;
	(*entry)->data      = val;
	(*entry)->name      = totalname;
	(*entry)->typesize  = valsize;
	(*entry)->typecount = valcount;
	(*entry)->tag       = ss_current_tag;
	restrack_register_object(OBJTYPE_STATEREG, *entry, 0, __FILE__, __LINE__);
}


/*-------------------------------------------------
    state_save_register_bitmap - register a
    bitmap to be saved
-------------------------------------------------*/

void state_save_register_bitmap(const char *module, UINT32 instance, const char *name, mame_bitmap *val)
{
	state_save_register_memory(module, instance, name, val->base, val->bpp / 8, val->rowpixels * val->height);
}



/***************************************************************************
    CALLBACK FUNCTION REGISTRATION
***************************************************************************/

/*-------------------------------------------------
    register_func_void - register a function
    callback that takes no parameters
-------------------------------------------------*/

static void register_func_void(ss_func **root, void (*func)(void))
{
	ss_func **cur;

	/* check for invalid timing */
	if (!ss_registration_allowed)
	{
		logerror("Attempt to register callback function after state registration is closed!");
		ss_illegal_regs++;
		return;
	}

	/* scan for duplicates and push through to the end */
	for (cur = root; *cur; cur = &(*cur)->next)
		if ((*cur)->func.voidf == func && (*cur)->tag == ss_current_tag)
			fatalerror("Duplicate save state function (%d, 0x%p)", ss_current_tag, func);

	/* allocate a new entry */
	*cur = malloc_or_die(sizeof(ss_func));

	/* fill it in */
	(*cur)->next       = NULL;
	(*cur)->type       = FUNC_NOPARAM;
	(*cur)->func.voidf = func;
	(*cur)->tag        = ss_current_tag;
	restrack_register_object(OBJTYPE_STATEREG, *cur, (root == &ss_prefunc_reg) ? 1 : 2, __FILE__, __LINE__);
}

void state_save_register_func_presave(void (*func)(void))
{
	register_func_void(&ss_prefunc_reg, func);
}

void state_save_register_func_postload(void (*func)(void))
{
	register_func_void(&ss_postfunc_reg, func);
}


/*-------------------------------------------------
    register_func_int - register a function
    callback that takes an integer parameter
-------------------------------------------------*/

static void register_func_int(ss_func **root, void (*func)(int), int param)
{
	ss_func **cur;

	/* check for invalid timing */
	if (!ss_registration_allowed)
		fatalerror("Attempt to register callback function after state registration is closed!");

	/* scan for duplicates and push through to the end */
	for (cur = root; *cur; cur = &(*cur)->next)
		if ((*cur)->func.intf == func && (*cur)->param.intp == param && (*cur)->tag == ss_current_tag)
			fatalerror("Duplicate save state function (%d, %d, %p)", ss_current_tag, param, func);

	/* allocate a new entry */
	*cur = malloc_or_die(sizeof(ss_func));

	/* fill it in */
	(*cur)->next       = NULL;
	(*cur)->type       = FUNC_INTPARAM;
	(*cur)->func.intf  = func;
	(*cur)->param.intp = param;
	(*cur)->tag        = ss_current_tag;
	restrack_register_object(OBJTYPE_STATEREG, *cur, (root == &ss_prefunc_reg) ? 1 : 2, __FILE__, __LINE__);
}

void state_save_register_func_presave_int(void (*func)(int), int param)
{
	register_func_int(&ss_prefunc_reg, func, param);
}

void state_save_register_func_postload_int(void (*func)(int), int param)
{
	register_func_int(&ss_postfunc_reg, func, param);
}


/*-------------------------------------------------
    register_func_ptr - register a function
    callback that takes a void * parameter
-------------------------------------------------*/

static void register_func_ptr(ss_func **root, void (*func)(void *), void *param)
{
	ss_func **cur;

	/* check for invalid timing */
	if (!ss_registration_allowed)
		fatalerror("Attempt to register callback function after state registration is closed!");

	/* scan for duplicates and push through to the end */
	for (cur = root; *cur; cur = &(*cur)->next)
		if ((*cur)->func.ptrf == func && (*cur)->param.ptrp == param && (*cur)->tag == ss_current_tag)
			fatalerror("Duplicate save state function (%d, %p, %p)", ss_current_tag, param, func);

	/* allocate a new entry */
	*cur = malloc_or_die(sizeof(ss_func));

	/* fill it in */
	(*cur)->next       = NULL;
	(*cur)->type       = FUNC_PTRPARAM;
	(*cur)->func.ptrf  = func;
	(*cur)->param.ptrp = param;
	(*cur)->tag        = ss_current_tag;
	restrack_register_object(OBJTYPE_STATEREG, *cur, (root == &ss_prefunc_reg) ? 1 : 2, __FILE__, __LINE__);
}

void state_save_register_func_presave_ptr(void (*func)(void *), void * param)
{
	register_func_ptr(&ss_prefunc_reg, func, param);
}

void state_save_register_func_postload_ptr(void (*func)(void *), void * param)
{
	register_func_ptr(&ss_postfunc_reg, func, param);
}



/***************************************************************************
    REGISTRATION FREEING
***************************************************************************/

/*-------------------------------------------------
    func_free - free registered functions attached
    to the current resource tag
-------------------------------------------------*/

static void func_free(ss_func **root, void *ptr)
{
	ss_func **func;

	/* iterate over the function list */
	for (func = root; *func; )
	{
		/* if this entry matches, free it */
		if (*func == ptr)
		{
			ss_func *func_to_free = *func;

			/* de-link us from the list and free our memory */
			*func = (*func)->next;
			free(func_to_free);
			break;
		}

		/* otherwise, advance */
		else
			func = &(*func)->next;
	}
}


/*-------------------------------------------------
    state_save_free - free all registrations that
    have been tagged with the current resource
    tag
-------------------------------------------------*/

void state_destructor(void *ptr, size_t size)
{
	/* size of 0 means an entry */
	if (size == 0)
	{
		ss_entry **entry;

		/* iterate over entries */
		for (entry = &ss_registry; *entry; )
		{
			/* if this entry matches, free it */
			if (*entry == ptr)
			{
				ss_entry *entry_to_free = *entry;

				/* de-link us from the list and free our memory */
				*entry = (*entry)->next;
				astring_free(entry_to_free->name);
				free(entry_to_free);
				break;
			}

			/* if not a match, move on */
			else
				entry = &(*entry)->next;
		}
	}

	/* size of 1 means a pre function */
	else if (size == 1)
		func_free(&ss_prefunc_reg, ptr);

	/* size of 2 means a post function */
	else if (size == 2)
		func_free(&ss_postfunc_reg, ptr);

	/* if we're clear of all registrations, reset the invalid counter */
	if (ss_registry == NULL && ss_prefunc_reg == NULL && ss_postfunc_reg == NULL)
		ss_illegal_regs = 0;
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

static int compute_size_and_offsets(void)
{
	ss_entry *entry;
	int total_size;

	/* start with the header size */
	total_size = 0x18;

	/* iterate over entries */
	for (entry = ss_registry; entry; entry = entry->next)
	{
		/* note the offset and accumulate a total size */
		entry->offset = total_size;
		total_size += entry->typesize * entry->typecount;
	}

	/* return the total size */
	return total_size;
}


/*-------------------------------------------------
    call_hook_functions - loop through all the
    hook functions and call them
-------------------------------------------------*/

static int call_hook_functions(ss_func *funclist)
{
	ss_func *func;
	int count = 0;

	/* iterate over the list of functions */
	for (func = funclist; func; func = func->next)
		if (func->tag == ss_current_tag)
		{
			count++;

			/* call with the appropriate parameters */
			switch (func->type)
			{
				case FUNC_NOPARAM:	(func->func.voidf)();					break;
				case FUNC_INTPARAM:	(func->func.intf)(func->param.intp);	break;
				case FUNC_PTRPARAM:	(func->func.ptrf)(func->param.ptrp);	break;
			}
		}

	return count;
}


/*-------------------------------------------------
    get_signature - compute the signature, which
    is a CRC over the structure of the data
-------------------------------------------------*/

static UINT32 get_signature(void)
{
	ss_entry *entry;
	UINT32 crc = 0;

	/* iterate over entries */
	for (entry = ss_registry; entry; entry = entry->next)
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

int state_save_check_file(mame_file *file, const char *gamename, int validate_signature, void (CLIB_DECL *errormsg)(const char *fmt, ...))
{
	UINT32 signature = 0;
	UINT8 header[0x18];

	/* if we want to validate the signature, compute it */
	if (validate_signature)
		signature = get_signature();

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

int state_save_save_begin(mame_file *file)
{
	/* if we have illegal registrations, return an error */
	if (ss_illegal_regs > 0)
		return 1;

	TRACE(logerror("Beginning save\n"));
	ss_dump_file = file;

	/* compute the total dump size and the offsets of each element */
	ss_dump_size = compute_size_and_offsets();
	TRACE(logerror("   total size %u\n", ss_dump_size));

	/* allocate memory for the array */
	ss_dump_array = malloc_or_die(ss_dump_size);
	return 0;
}


/*-------------------------------------------------
    state_save_save_continue - save within the
    current tag
-------------------------------------------------*/

void state_save_save_continue(void)
{
	ss_entry *entry;
	int count;

	TRACE(logerror("Saving tag %d\n", ss_current_tag));

	/* call the pre-save functions */
	TRACE(logerror("  calling pre-save functions\n"));
	count = call_hook_functions(ss_prefunc_reg);
	TRACE(logerror("    %d functions called\n", count));

	/* then copy in all the data */
	TRACE(logerror("  copying data\n"));

	/* iterate over entries with matching tags */
	for (entry = ss_registry; entry; entry = entry->next)
		if (entry->tag == ss_current_tag)
		{
			memcpy(ss_dump_array + entry->offset, entry->data, entry->typesize * entry->typecount);
			TRACE(logerror("    %s: %x..%x\n", astring_c(entry->name), entry->offset, entry->offset + entry->typesize * entry->typecount - 1));
		}
}


/*-------------------------------------------------
    state_save_save_finish - finish saving the
    file by writing the header and closing
-------------------------------------------------*/

void state_save_save_finish(void)
{
	UINT32 signature;
	UINT8 flags = 0;

	TRACE(logerror("Finishing save\n"));

	/* compute the flags */
#ifndef LSB_FIRST
	flags |= SS_MSB_FIRST;
#endif

	/* build up the header */
	memcpy(ss_dump_array, ss_magic_num, 8);
	ss_dump_array[8] = SAVE_VERSION;
	ss_dump_array[9] = flags;
	memset(ss_dump_array+0xa, 0, 10);
	strcpy((char *)ss_dump_array+0xa, Machine->gamedrv->name);

	/* copy in the signature */
	signature = get_signature();
	*(UINT32 *)&ss_dump_array[0x14] = LITTLE_ENDIANIZE_INT32(signature);

	/* write the file */
	mame_fwrite(ss_dump_file, ss_dump_array, ss_dump_size);

	/* free memory and reset the global states */
	free(ss_dump_array);
	ss_dump_array = NULL;
	ss_dump_size = 0;
	ss_dump_file = NULL;
}



/***************************************************************************
    LOAD STATE PROCESSING
***************************************************************************/

/*-------------------------------------------------
    state_save_load_begin - begin the process
    of loading the state
-------------------------------------------------*/

int state_save_load_begin(mame_file *file)
{
	TRACE(logerror("Beginning load\n"));

	/* read the file into memory */
	ss_dump_size = mame_fsize(file);
	ss_dump_array = malloc_or_die(ss_dump_size);
	ss_dump_file = file;
	mame_fread(ss_dump_file, ss_dump_array, ss_dump_size);

	/* verify the header and report an error if it doesn't match */
	if (validate_header(ss_dump_array, NULL, get_signature(), popmessage, "Error: "))
	{
		free(ss_dump_array);
		ss_dump_array = NULL;
		ss_dump_size = 0;
		ss_dump_file = NULL;
		return 1;
	}

	/* compute the total size and offset of all the entries */
	compute_size_and_offsets();
	return 0;
}


/*-------------------------------------------------
    state_save_load_continue - load all state in
    the current tag
-------------------------------------------------*/

void state_save_load_continue(void)
{
	ss_entry *entry;
	int need_convert;
	int count;

	/* first determine whether or not we need to convert the endianness of the data */
#ifdef LSB_FIRST
	need_convert = (ss_dump_array[9] & SS_MSB_FIRST) != 0;
#else
	need_convert = (ss_dump_array[9] & SS_MSB_FIRST) == 0;
#endif

	TRACE(logerror("Loading tag %d\n", ss_current_tag));
	TRACE(logerror("  copying data\n"));

	/* iterate over entries with matching tags */
	for (entry = ss_registry; entry; entry = entry->next)
		if (entry->tag == ss_current_tag)
		{
			memcpy(entry->data, ss_dump_array + entry->offset, entry->typesize * entry->typecount);
			if (need_convert && ss_conv[entry->typesize])
				(*ss_conv[entry->typesize])(entry->data, entry->typecount);
			TRACE(logerror("    %s: %x..%x\n", astring_c(entry->name), entry->offset, entry->offset + entry->typesize * entry->typecount - 1));
		}

	/* call the post-load functions */
	TRACE(logerror("  calling post-load functions\n"));
	count = call_hook_functions(ss_postfunc_reg);
	TRACE(logerror("    %d functions called\n", count));
}


/*-------------------------------------------------
    state_save_load_finish - complete the process
    of loading the state
-------------------------------------------------*/

void state_save_load_finish(void)
{
	TRACE(logerror("Finishing load\n"));

	/* free memory and reset the global states */
	free(ss_dump_array);
	ss_dump_array = NULL;
	ss_dump_size = 0;
	ss_dump_file = NULL;
}



/***************************************************************************
    DEBUGGING
***************************************************************************/

/*-------------------------------------------------
    state_save_get_indexed_item - return an item
    with the given index
-------------------------------------------------*/

const char *state_save_get_indexed_item(int index, void **base, UINT32 *valsize, UINT32 *valcount)
{
	ss_entry *ss;

	for (ss = ss_registry; ss != NULL; ss = ss->next)
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

void state_save_dump_registry(void)
{
#ifdef VERBOSE
	ss_entry *entry;

	for (entry = ss_registry; entry; entry=entry->next)
		logerror("%s: %d x %d\n", astring_c(entry->name), entry->typesize, entry->typecount);
#endif
}


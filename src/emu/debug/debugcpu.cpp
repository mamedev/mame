// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    debugcpu.c

    Debugger CPU/memory interface engine.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "osdepend.h"
#include "debugcpu.h"
#include "debugcon.h"
#include "express.h"
#include "debugvw.h"
#include "debugger.h"
#include "uiinput.h"
#include "xmlfile.h"
#include "coreutil.h"
#include <ctype.h>


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define NUM_TEMP_VARIABLES  10

enum
{
	EXECUTION_STATE_STOPPED,
	EXECUTION_STATE_RUNNING
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct debugcpu_private
{
	device_t *livecpu;
	device_t *visiblecpu;
	device_t *breakcpu;

	FILE *          source_file;                /* script source file */

	symbol_table *  symtable;                   /* global symbol table */

	bool            within_instruction_hook;
	bool            vblank_occurred;
	bool            memory_modified;
	bool            debugger_access;

	int             execution_state;
	device_t *      m_stop_when_not_device;     // stop execution when the device ceases to be this

	UINT32          bpindex;
	UINT32          wpindex;
	UINT32          rpindex;

	UINT64          wpdata;
	UINT64          wpaddr;
	UINT64          tempvar[NUM_TEMP_VARIABLES];

	osd_ticks_t     last_periodic_update_time;

	bool            comments_loaded;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* internal helpers */
static void debug_cpu_exit(running_machine &machine);
static void on_vblank(running_machine &machine, screen_device &device, bool vblank_state);
static void reset_transient_flags(running_machine &machine);
static void process_source_file(running_machine &machine);

/* expression handlers */
static UINT64 expression_read_memory(void *param, const char *name, expression_space space, UINT32 address, int size);
static UINT64 expression_read_program_direct(address_space &space, int opcode, offs_t address, int size);
static UINT64 expression_read_memory_region(running_machine &machine, const char *rgntag, offs_t address, int size);
static void expression_write_memory(void *param, const char *name, expression_space space, UINT32 address, int size, UINT64 data);
static void expression_write_program_direct(address_space &space, int opcode, offs_t address, int size, UINT64 data);
static void expression_write_memory_region(running_machine &machine, const char *rgntag, offs_t address, int size, UINT64 data);
static expression_error::error_code expression_validate(void *param, const char *name, expression_space space);

/* variable getters/setters */
static UINT64 get_cpunum(symbol_table &table, void *ref);
static UINT64 get_beamx(symbol_table &table, void *ref);
static UINT64 get_beamy(symbol_table &table, void *ref);
static UINT64 get_frame(symbol_table &table, void *ref);



/***************************************************************************
    INITIALIZATION AND CLEANUP
***************************************************************************/

/*-------------------------------------------------
    debug_cpu_init - initialize the CPU
    information for debugging
-------------------------------------------------*/

void debug_cpu_init(running_machine &machine)
{
	screen_device *first_screen = machine.first_screen();
	debugcpu_private *global;
	int regnum;

	/* allocate and reset globals */
	machine.debugcpu_data = global = auto_alloc_clear(machine, <debugcpu_private>());
	global->execution_state = EXECUTION_STATE_STOPPED;
	global->bpindex = 1;
	global->wpindex = 1;
	global->rpindex = 1;

	/* create a global symbol table */
	global->symtable = global_alloc(symbol_table(&machine));

	// configure our base memory accessors
	debug_cpu_configure_memory(machine, *global->symtable);

	/* add "wpaddr", "wpdata", "cycles", "cpunum", "logunmap" to the global symbol table */
	global->symtable->add("wpaddr", symbol_table::READ_ONLY, &global->wpaddr);
	global->symtable->add("wpdata", symbol_table::READ_ONLY, &global->wpdata);
	global->symtable->add("cpunum", nullptr, get_cpunum);
	global->symtable->add("beamx", (void *)first_screen, get_beamx);
	global->symtable->add("beamy", (void *)first_screen, get_beamy);
	global->symtable->add("frame", (void *)first_screen, get_frame);

	/* add the temporary variables to the global symbol table */
	for (regnum = 0; regnum < NUM_TEMP_VARIABLES; regnum++)
	{
		char symname[10];
		sprintf(symname, "temp%d", regnum);
		global->symtable->add(symname, symbol_table::READ_WRITE, &global->tempvar[regnum]);
	}

	/* first CPU is visible by default */
	global->visiblecpu = machine.firstcpu;

	/* add callback for breaking on VBLANK */
	if (machine.first_screen() != nullptr)
		machine.first_screen()->register_vblank_callback(vblank_state_delegate(FUNC(on_vblank), &machine));

	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(debug_cpu_exit), &machine));
}


void debug_cpu_configure_memory(running_machine &machine, symbol_table &table)
{
	table.configure_memory(&machine, expression_validate, expression_read_memory, expression_write_memory);
}


/*-------------------------------------------------
    debug_cpu_flush_traces - flushes all traces;
    this is useful if a trace is going on when we
    fatalerror
-------------------------------------------------*/

void debug_cpu_flush_traces(running_machine &machine)
{
	/* this can be called on exit even when no debugging is enabled, so
	 make sure the devdebug is valid before proceeding */
	device_iterator iter(machine.root_device());
	for (device_t *device = iter.first(); device != nullptr; device = iter.next())
		if (device->debug() != nullptr)
			device->debug()->trace_flush();
}



/***************************************************************************
    DEBUGGING STATUS AND INFORMATION
***************************************************************************/

/*-------------------------------------------------
    cpu_get_visible_cpu - return the visible CPU
    device (the one that commands should apply to)
-------------------------------------------------*/

device_t *debug_cpu_get_visible_cpu(running_machine &machine)
{
	return machine.debugcpu_data->visiblecpu;
}


/*-------------------------------------------------
    debug_cpu_within_instruction_hook - true if
    the debugger is currently live
-------------------------------------------------*/

int debug_cpu_within_instruction_hook(running_machine &machine)
{
	return machine.debugcpu_data->within_instruction_hook;
}


/*-------------------------------------------------
    debug_cpu_is_stopped - return true if the
    current execution state is stopped
-------------------------------------------------*/

int debug_cpu_is_stopped(running_machine &machine)
{
	debugcpu_private *global = machine.debugcpu_data;
	return (global != nullptr) ? (global->execution_state == EXECUTION_STATE_STOPPED) : false;
}



/***************************************************************************
    SYMBOL TABLE INTERFACES
***************************************************************************/

/*-------------------------------------------------
    debug_cpu_get_global_symtable - return the
    global symbol table
-------------------------------------------------*/

symbol_table *debug_cpu_get_global_symtable(running_machine &machine)
{
	return machine.debugcpu_data->symtable;
}


/*-------------------------------------------------
    debug_cpu_get_visible_symtable - return the
    locally-visible symbol table
-------------------------------------------------*/

symbol_table *debug_cpu_get_visible_symtable(running_machine &machine)
{
	return &machine.debugcpu_data->visiblecpu->debug()->symtable();
}


/*-------------------------------------------------
    debug_cpu_source_script - specifies a debug
    command script to execute
-------------------------------------------------*/

void debug_cpu_source_script(running_machine &machine, const char *file)
{
	debugcpu_private *global = machine.debugcpu_data;

	/* close any existing source file */
	if (global->source_file != nullptr)
	{
		fclose(global->source_file);
		global->source_file = nullptr;
	}

	/* open a new one if requested */
	if (file != nullptr)
	{
		global->source_file = fopen(file, "r");
		if (!global->source_file)
		{
			if (machine.phase() == MACHINE_PHASE_RUNNING)
				debug_console_printf(machine, "Cannot open command file '%s'\n", file);
			else
				fatalerror("Cannot open command file '%s'\n", file);
		}
	}
}



//**************************************************************************
//  MEMORY AND DISASSEMBLY HELPERS
//**************************************************************************

//-------------------------------------------------
//  debug_comment_save - save all comments for
//  the given machine
//-------------------------------------------------

bool debug_comment_save(running_machine &machine)
{
	bool comments_saved = false;

	// if we don't have a root, bail
	xml_data_node *root = xml_file_create();
	if (root == nullptr)
		return false;

	// wrap in a try/catch to handle errors
	try
	{
		// create a comment node
		xml_data_node *commentnode = xml_add_child(root, "mamecommentfile", nullptr);
		if (commentnode == nullptr)
			throw emu_exception();
		xml_set_attribute_int(commentnode, "version", COMMENT_VERSION);

		// create a system node
		xml_data_node *systemnode = xml_add_child(commentnode, "system", nullptr);
		if (systemnode == nullptr)
			throw emu_exception();
		xml_set_attribute(systemnode, "name", machine.system().name);

		// for each device
		device_iterator iter(machine.root_device());
		bool found_comments = false;
		for (device_t *device = iter.first(); device != nullptr; device = iter.next())
			if (device->debug() && device->debug()->comment_count() > 0)
			{
				// create a node for this device
				xml_data_node *curnode = xml_add_child(systemnode, "cpu", nullptr);
				if (curnode == nullptr)
					throw emu_exception();
				xml_set_attribute(curnode, "tag", device->tag().c_str());

				// export the comments
				if (!device->debug()->comment_export(*curnode))
					throw emu_exception();
				found_comments = true;
			}

		// flush the file
		if (found_comments)
		{
			emu_file file(machine.options().comment_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
			file_error filerr = file.open(machine.basename(), ".cmt");
			if (filerr == FILERR_NONE)
			{
				xml_file_write(root, file);
				comments_saved = true;
			}
		}
	}
	catch (emu_exception &)
	{
		xml_file_free(root);
		return false;
	}

	// free and get out of here
	xml_file_free(root);
	return comments_saved;
}


//-------------------------------------------------
//  debug_comment_load - load all comments for
//  the given machine
//-------------------------------------------------

bool debug_comment_load(running_machine &machine)
{
	// open the file
	emu_file file(machine.options().comment_directory(), OPEN_FLAG_READ);
	file_error filerr = file.open(machine.basename(), ".cmt");

	// if an error, just return false
	if (filerr != FILERR_NONE)
		return false;

	// wrap in a try/catch to handle errors
	xml_data_node *root = xml_file_read(file, nullptr);
	try
	{
		// read the file
		if (root == nullptr)
			throw emu_exception();

		// find the config node
		xml_data_node *commentnode = xml_get_sibling(root->child, "mamecommentfile");
		if (commentnode == nullptr)
			throw emu_exception();

		// validate the config data version
		int version = xml_get_attribute_int(commentnode, "version", 0);
		if (version != COMMENT_VERSION)
			throw emu_exception();

		// check to make sure the file is applicable
		xml_data_node *systemnode = xml_get_sibling(commentnode->child, "system");
		const char *name = xml_get_attribute_string(systemnode, "name", "");
		if (strcmp(name, machine.system().name) != 0)
			throw emu_exception();

		// iterate over devices
		for (xml_data_node *cpunode = xml_get_sibling(systemnode->child, "cpu"); cpunode; cpunode = xml_get_sibling(cpunode->next, "cpu"))
		{
			device_t *device = machine.device(xml_get_attribute_string(cpunode, "tag", ""));
			if (device != nullptr)
				if (!device->debug()->comment_import(*cpunode))
					throw emu_exception();
		}
	}
	catch (emu_exception &)
	{
		// clean up in case of error
		if (root != nullptr)
			xml_file_free(root);
		return false;
	}

	// free the parser
	xml_file_free(root);
	return true;
}



/***************************************************************************
    MEMORY AND DISASSEMBLY HELPERS
***************************************************************************/

/*-------------------------------------------------
    debug_cpu_translate - return the physical
    address corresponding to the given logical
    address
-------------------------------------------------*/

int debug_cpu_translate(address_space &space, int intention, offs_t *address)
{
	device_memory_interface *memory;
	if (space.device().interface(memory))
		return memory->translate(space.spacenum(), intention, *address);
	return true;
}


/***************************************************************************
    DEBUGGER MEMORY ACCESSORS
***************************************************************************/

/*-------------------------------------------------
    debug_read_byte - return a byte from the
    the specified memory space
-------------------------------------------------*/

UINT8 debug_read_byte(address_space &space, offs_t address, int apply_translation)
{
	debugcpu_private *global = space.machine().debugcpu_data;
	UINT64 custom;
	UINT8 result;

	/* mask against the logical byte mask */
	address &= space.logbytemask();

	/* all accesses from this point on are for the debugger */
	space.set_debugger_access(global->debugger_access = true);

	/* translate if necessary; if not mapped, return 0xff */
	if (apply_translation && !debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &address))
		result = 0xff;

	/* if there is a custom read handler, and it returns true, use that value */
	else if (space.device().memory().read(space.spacenum(), address, 1, custom))
		result = custom;

	/* otherwise, call the byte reading function for the translated address */
	else
		result = space.read_byte(address);

	/* no longer accessing via the debugger */
	space.set_debugger_access(global->debugger_access = false);
	return result;
}


/*-------------------------------------------------
    debug_read_word - return a word from the
    specified memory space
-------------------------------------------------*/

UINT16 debug_read_word(address_space &space, offs_t address, int apply_translation)
{
	debugcpu_private *global = space.machine().debugcpu_data;
	UINT16 result;

	/* mask against the logical byte mask */
	address &= space.logbytemask();

	/* if this is misaligned read, or if there are no word readers, just read two bytes */
	if ((address & 1) != 0)
	{
		UINT8 byte0 = debug_read_byte(space, address + 0, apply_translation);
		UINT8 byte1 = debug_read_byte(space, address + 1, apply_translation);

		/* based on the endianness, the result is assembled differently */
		if (space.endianness() == ENDIANNESS_LITTLE)
			result = byte0 | (byte1 << 8);
		else
			result = byte1 | (byte0 << 8);
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		UINT64 custom;

		/* all accesses from this point on are for the debugger */
		space.set_debugger_access(global->debugger_access = true);

		/* translate if necessary; if not mapped, return 0xffff */
		if (apply_translation && !debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &address))
			result = 0xffff;

		/* if there is a custom read handler, and it returns true, use that value */
		else if (space.device().memory().read(space.spacenum(), address, 2, custom))
			result = custom;

		/* otherwise, call the byte reading function for the translated address */
		else
			result = space.read_word(address);

		/* no longer accessing via the debugger */
		space.set_debugger_access(global->debugger_access = false);
	}

	return result;
}


/*-------------------------------------------------
    debug_read_dword - return a dword from the
    specified memory space
-------------------------------------------------*/

UINT32 debug_read_dword(address_space &space, offs_t address, int apply_translation)
{
	debugcpu_private *global = space.machine().debugcpu_data;
	UINT32 result;

	/* mask against the logical byte mask */
	address &= space.logbytemask();

	/* if this is misaligned read, or if there are no dword readers, just read two words */
	if ((address & 3) != 0)
	{
		UINT16 word0 = debug_read_word(space, address + 0, apply_translation);
		UINT16 word1 = debug_read_word(space, address + 2, apply_translation);

		/* based on the endianness, the result is assembled differently */
		if (space.endianness() == ENDIANNESS_LITTLE)
			result = word0 | (word1 << 16);
		else
			result = word1 | (word0 << 16);
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		UINT64 custom;

		/* all accesses from this point on are for the debugger */
		space.set_debugger_access(global->debugger_access = true);

		/* translate if necessary; if not mapped, return 0xffffffff */
		if (apply_translation && !debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &address))
			result = 0xffffffff;

		/* if there is a custom read handler, and it returns true, use that value */
		else if (space.device().memory().read(space.spacenum(), address, 4, custom))
			result = custom;

		/* otherwise, call the byte reading function for the translated address */
		else
			result = space.read_dword(address);

		/* no longer accessing via the debugger */
		space.set_debugger_access(global->debugger_access = false);
	}

	return result;
}


/*-------------------------------------------------
    debug_read_qword - return a qword from the
    specified memory space
-------------------------------------------------*/

UINT64 debug_read_qword(address_space &space, offs_t address, int apply_translation)
{
	debugcpu_private *global = space.machine().debugcpu_data;
	UINT64 result;

	/* mask against the logical byte mask */
	address &= space.logbytemask();

	/* if this is misaligned read, or if there are no qword readers, just read two dwords */
	if ((address & 7) != 0)
	{
		UINT32 dword0 = debug_read_dword(space, address + 0, apply_translation);
		UINT32 dword1 = debug_read_dword(space, address + 4, apply_translation);

		/* based on the endianness, the result is assembled differently */
		if (space.endianness() == ENDIANNESS_LITTLE)
			result = dword0 | ((UINT64)dword1 << 32);
		else
			result = dword1 | ((UINT64)dword0 << 32);
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		UINT64 custom;

		/* all accesses from this point on are for the debugger */
		space.set_debugger_access(global->debugger_access = true);

		/* translate if necessary; if not mapped, return 0xffffffffffffffff */
		if (apply_translation && !debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &address))
			result = ~(UINT64)0;

		/* if there is a custom read handler, and it returns true, use that value */
		else if (space.device().memory().read(space.spacenum(), address, 8, custom))
			result = custom;

		/* otherwise, call the byte reading function for the translated address */
		else
			result = space.read_qword(address);

		/* no longer accessing via the debugger */
		space.set_debugger_access(global->debugger_access = false);
	}

	return result;
}


/*-------------------------------------------------
    debug_read_memory - return 1,2,4 or 8 bytes
    from the specified memory space
-------------------------------------------------*/

UINT64 debug_read_memory(address_space &space, offs_t address, int size, int apply_translation)
{
	UINT64 result = ~(UINT64)0 >> (64 - 8*size);
	switch (size)
	{
		case 1:     result = debug_read_byte(space, address, apply_translation);    break;
		case 2:     result = debug_read_word(space, address, apply_translation);    break;
		case 4:     result = debug_read_dword(space, address, apply_translation);   break;
		case 8:     result = debug_read_qword(space, address, apply_translation);   break;
	}
	return result;
}


/*-------------------------------------------------
    debug_write_byte - write a byte to the
    specified memory space
-------------------------------------------------*/

void debug_write_byte(address_space &space, offs_t address, UINT8 data, int apply_translation)
{
	debugcpu_private *global = space.machine().debugcpu_data;

	/* mask against the logical byte mask */
	address &= space.logbytemask();

	/* all accesses from this point on are for the debugger */
	space.set_debugger_access(global->debugger_access = true);

	/* translate if necessary; if not mapped, we're done */
	if (apply_translation && !debug_cpu_translate(space, TRANSLATE_WRITE_DEBUG, &address))
		;

	/* if there is a custom write handler, and it returns true, use that */
	else if (space.device().memory().write(space.spacenum(), address, 1, data))
		;

	/* otherwise, call the byte reading function for the translated address */
	else
		space.write_byte(address, data);

	/* no longer accessing via the debugger */
	space.set_debugger_access(global->debugger_access = false);
	global->memory_modified = true;
}


/*-------------------------------------------------
    debug_write_word - write a word to the
    specified memory space
-------------------------------------------------*/

void debug_write_word(address_space &space, offs_t address, UINT16 data, int apply_translation)
{
	debugcpu_private *global = space.machine().debugcpu_data;

	/* mask against the logical byte mask */
	address &= space.logbytemask();

	/* if this is a misaligned write, or if there are no word writers, just read two bytes */
	if ((address & 1) != 0)
	{
		if (space.endianness() == ENDIANNESS_LITTLE)
		{
			debug_write_byte(space, address + 0, data >> 0, apply_translation);
			debug_write_byte(space, address + 1, data >> 8, apply_translation);
		}
		else
		{
			debug_write_byte(space, address + 0, data >> 8, apply_translation);
			debug_write_byte(space, address + 1, data >> 0, apply_translation);
		}
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		/* all accesses from this point on are for the debugger */
		space.set_debugger_access(global->debugger_access = true);

		/* translate if necessary; if not mapped, we're done */
		if (apply_translation && !debug_cpu_translate(space, TRANSLATE_WRITE_DEBUG, &address))
			;

		/* if there is a custom write handler, and it returns true, use that */
		else if (space.device().memory().write(space.spacenum(), address, 2, data))
			;

		/* otherwise, call the byte reading function for the translated address */
		else
			space.write_word(address, data);

		/* no longer accessing via the debugger */
		space.set_debugger_access(global->debugger_access = false);
		global->memory_modified = true;
	}
}


/*-------------------------------------------------
    debug_write_dword - write a dword to the
    specified memory space
-------------------------------------------------*/

void debug_write_dword(address_space &space, offs_t address, UINT32 data, int apply_translation)
{
	debugcpu_private *global = space.machine().debugcpu_data;

	/* mask against the logical byte mask */
	address &= space.logbytemask();

	/* if this is a misaligned write, or if there are no dword writers, just read two words */
	if ((address & 3) != 0)
	{
		if (space.endianness() == ENDIANNESS_LITTLE)
		{
			debug_write_word(space, address + 0, data >> 0, apply_translation);
			debug_write_word(space, address + 2, data >> 16, apply_translation);
		}
		else
		{
			debug_write_word(space, address + 0, data >> 16, apply_translation);
			debug_write_word(space, address + 2, data >> 0, apply_translation);
		}
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		/* all accesses from this point on are for the debugger */
		space.set_debugger_access(global->debugger_access = true);

		/* translate if necessary; if not mapped, we're done */
		if (apply_translation && !debug_cpu_translate(space, TRANSLATE_WRITE_DEBUG, &address))
			;

		/* if there is a custom write handler, and it returns true, use that */
		else if (space.device().memory().write(space.spacenum(), address, 4, data))
			;

		/* otherwise, call the byte reading function for the translated address */
		else
			space.write_dword(address, data);

		/* no longer accessing via the debugger */
		space.set_debugger_access(global->debugger_access = false);
		global->memory_modified = true;
	}
}


/*-------------------------------------------------
    debug_write_qword - write a qword to the
    specified memory space
-------------------------------------------------*/

void debug_write_qword(address_space &space, offs_t address, UINT64 data, int apply_translation)
{
	debugcpu_private *global = space.machine().debugcpu_data;

	/* mask against the logical byte mask */
	address &= space.logbytemask();

	/* if this is a misaligned write, or if there are no qword writers, just read two dwords */
	if ((address & 7) != 0)
	{
		if (space.endianness() == ENDIANNESS_LITTLE)
		{
			debug_write_dword(space, address + 0, data >> 0, apply_translation);
			debug_write_dword(space, address + 4, data >> 32, apply_translation);
		}
		else
		{
			debug_write_dword(space, address + 0, data >> 32, apply_translation);
			debug_write_dword(space, address + 4, data >> 0, apply_translation);
		}
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		/* all accesses from this point on are for the debugger */
		space.set_debugger_access(global->debugger_access = true);

		/* translate if necessary; if not mapped, we're done */
		if (apply_translation && !debug_cpu_translate(space, TRANSLATE_WRITE_DEBUG, &address))
			;

		/* if there is a custom write handler, and it returns true, use that */
		else if (space.device().memory().write(space.spacenum(), address, 8, data))
			;

		/* otherwise, call the byte reading function for the translated address */
		else
			space.write_qword(address, data);

		/* no longer accessing via the debugger */
		space.set_debugger_access(global->debugger_access = false);
		global->memory_modified = true;
	}
}


/*-------------------------------------------------
    debug_write_memory - write 1,2,4 or 8 bytes
    to the specified memory space
-------------------------------------------------*/

void debug_write_memory(address_space &space, offs_t address, UINT64 data, int size, int apply_translation)
{
	switch (size)
	{
		case 1:     debug_write_byte(space, address, data, apply_translation);  break;
		case 2:     debug_write_word(space, address, data, apply_translation);  break;
		case 4:     debug_write_dword(space, address, data, apply_translation); break;
		case 8:     debug_write_qword(space, address, data, apply_translation); break;
	}
}


/*-------------------------------------------------
    debug_read_opcode - read 1,2,4 or 8 bytes at
    the given offset from opcode space
-------------------------------------------------*/

UINT64 debug_read_opcode(address_space &space, offs_t address, int size)
{
	UINT64 result = ~(UINT64)0 & (~(UINT64)0 >> (64 - 8*size)), result2;
	debugcpu_private *global = space.machine().debugcpu_data;

	/* keep in logical range */
	address &= space.logbytemask();

	/* return early if we got the result directly */
	space.set_debugger_access(global->debugger_access = true);
	device_memory_interface *memory;
	if (space.device().interface(memory) && memory->readop(address, size, result2))
	{
		space.set_debugger_access(global->debugger_access = false);
		return result2;
	}

	/* if we're bigger than the address bus, break into smaller pieces */
	if (size > space.data_width() / 8)
	{
		int halfsize = size / 2;
		UINT64 r0 = debug_read_opcode(space, address + 0, halfsize);
		UINT64 r1 = debug_read_opcode(space, address + halfsize, halfsize);

		if (space.endianness() == ENDIANNESS_LITTLE)
			return r0 | (r1 << (8 * halfsize));
		else
			return r1 | (r0 << (8 * halfsize));
	}

	/* translate to physical first */
	if (!debug_cpu_translate(space, TRANSLATE_FETCH_DEBUG, &address))
		return result;

	/* keep in physical range */
	address &= space.bytemask();
	offs_t addrxor = 0;
	switch (space.data_width() / 8 * 10 + size)
	{
		/* dump opcodes in bytes from a byte-sized bus */
		case 11:
			break;

		/* dump opcodes in bytes from a word-sized bus */
		case 21:
			addrxor = (space.endianness() == ENDIANNESS_LITTLE) ? BYTE_XOR_LE(0) : BYTE_XOR_BE(0);
			break;

		/* dump opcodes in words from a word-sized bus */
		case 22:
			break;

		/* dump opcodes in bytes from a dword-sized bus */
		case 41:
			addrxor = (space.endianness() == ENDIANNESS_LITTLE) ? BYTE4_XOR_LE(0) : BYTE4_XOR_BE(0);
			break;

		/* dump opcodes in words from a dword-sized bus */
		case 42:
			addrxor = (space.endianness() == ENDIANNESS_LITTLE) ? WORD_XOR_LE(0) : WORD_XOR_BE(0);
			break;

		/* dump opcodes in dwords from a dword-sized bus */
		case 44:
			break;

		/* dump opcodes in bytes from a qword-sized bus */
		case 81:
			addrxor = (space.endianness() == ENDIANNESS_LITTLE) ? BYTE8_XOR_LE(0) : BYTE8_XOR_BE(0);
			break;

		/* dump opcodes in words from a qword-sized bus */
		case 82:
			addrxor = (space.endianness() == ENDIANNESS_LITTLE) ? WORD2_XOR_LE(0) : WORD2_XOR_BE(0);
			break;

		/* dump opcodes in dwords from a qword-sized bus */
		case 84:
			addrxor = (space.endianness() == ENDIANNESS_LITTLE) ? DWORD_XOR_LE(0) : DWORD_XOR_BE(0);
			break;

		/* dump opcodes in qwords from a qword-sized bus */
		case 88:
			break;

		default:
			fatalerror("debug_read_opcode: unknown type = %d\n", space.data_width() / 8 * 10 + size);
	}

	/* turn on debugger access */
	if (!global->debugger_access)
		space.set_debugger_access(global->debugger_access = true);

	/* switch off the size and handle unaligned accesses */
	switch (size)
	{
		case 1:
			result = space.direct().read_byte(address, addrxor);
			break;

		case 2:
			result = space.direct().read_word(address & ~1, addrxor);
			if ((address & 1) != 0)
			{
				result2 = space.direct().read_word((address & ~1) + 2, addrxor);
				if (space.endianness() == ENDIANNESS_LITTLE)
					result = (result >> (8 * (address & 1))) | (result2 << (16 - 8 * (address & 1)));
				else
					result = (result << (8 * (address & 1))) | (result2 >> (16 - 8 * (address & 1)));
				result &= 0xffff;
			}
			break;

		case 4:
			result = space.direct().read_dword(address & ~3, addrxor);
			if ((address & 3) != 0)
			{
				result2 = space.direct().read_dword((address & ~3) + 4, addrxor);
				if (space.endianness() == ENDIANNESS_LITTLE)
					result = (result >> (8 * (address & 3))) | (result2 << (32 - 8 * (address & 3)));
				else
					result = (result << (8 * (address & 3))) | (result2 >> (32 - 8 * (address & 3)));
				result &= 0xffffffff;
			}
			break;

		case 8:
			result = space.direct().read_qword(address & ~7, addrxor);
			if ((address & 7) != 0)
			{
				result2 = space.direct().read_qword((address & ~7) + 8, addrxor);
				if (space.endianness() == ENDIANNESS_LITTLE)
					result = (result >> (8 * (address & 7))) | (result2 << (64 - 8 * (address & 7)));
				else
					result = (result << (8 * (address & 7))) | (result2 >> (64 - 8 * (address & 7)));
			}
			break;
	}

	/* no longer accessing via the debugger */
	space.set_debugger_access(global->debugger_access = false);
	return result;
}



/***************************************************************************
    INTERNAL HELPERS
***************************************************************************/

/*-------------------------------------------------
    debug_cpu_exit - free all memory
-------------------------------------------------*/

static void debug_cpu_exit(running_machine &machine)
{
	debugcpu_private *global = machine.debugcpu_data;

	/* free the global symbol table */
	if (global != nullptr)
		global_free(global->symtable);
}


/*-------------------------------------------------
    on_vblank - called when a VBLANK hits
-------------------------------------------------*/

static void on_vblank(running_machine &machine, screen_device &device, bool vblank_state)
{
	/* just set a global flag to be consumed later */
	if (vblank_state)
		device.machine().debugcpu_data->vblank_occurred = true;
}


/*-------------------------------------------------
    reset_transient_flags - reset the transient
    flags on all CPUs
-------------------------------------------------*/

static void reset_transient_flags(running_machine &machine)
{
	/* loop over CPUs and reset the transient flags */
	device_iterator iter(machine.root_device());
	for (device_t *device = iter.first(); device != nullptr; device = iter.next())
		device->debug()->reset_transient_flag();
	machine.debugcpu_data->m_stop_when_not_device = nullptr;
}


/*-------------------------------------------------
    process_source_file - executes commands from
    a source file
-------------------------------------------------*/

static void process_source_file(running_machine &machine)
{
	debugcpu_private *global = machine.debugcpu_data;

	/* loop until the file is exhausted or until we are executing again */
	while (global->source_file != nullptr && global->execution_state == EXECUTION_STATE_STOPPED)
	{
		char buf[512];
		int i;
		char *s;

		/* stop at the end of file */
		if (feof(global->source_file))
		{
			fclose(global->source_file);
			global->source_file = nullptr;
			return;
		}

		/* fetch the next line */
		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf), global->source_file);

		/* strip out comments (text after '//') */
		s = strstr(buf, "//");
		if (s)
			*s = '\0';

		/* strip whitespace */
		i = (int)strlen(buf);
		while((i > 0) && (isspace((UINT8)buf[i-1])))
			buf[--i] = '\0';

		/* execute the command */
		if (buf[0])
			debug_console_execute_command(machine, buf, 1);
	}
}



/***************************************************************************
    EXPRESSION HANDLERS
***************************************************************************/

/*-------------------------------------------------
    expression_get_device - return a device
    based on a case insensitive tag search
-------------------------------------------------*/

static device_t *expression_get_device(running_machine &machine, std::string tag)
{
	// convert to lowercase then lookup the name (tags are enforced to be all lower case)
	std::string fullname(tag);
	strmakelower(fullname);
	return machine.device(fullname.c_str());
}


/*-------------------------------------------------
    expression_read_memory - read 1,2,4 or 8 bytes
    at the given offset in the given address
    space
-------------------------------------------------*/

static UINT64 expression_read_memory(void *param, const char *name, expression_space spacenum, UINT32 address, int size)
{
	running_machine &machine = *(running_machine *)param;
	UINT64 result = ~(UINT64)0 >> (64 - 8*size);
	device_t *device = nullptr;

	switch (spacenum)
	{
		case EXPSPACE_PROGRAM_LOGICAL:
		case EXPSPACE_DATA_LOGICAL:
		case EXPSPACE_IO_LOGICAL:
		case EXPSPACE_SPACE3_LOGICAL:
			if (name != nullptr)
				device = expression_get_device(machine, name);
			if (device == nullptr)
				device = debug_cpu_get_visible_cpu(machine);
			if (device->memory().has_space(AS_PROGRAM + (spacenum - EXPSPACE_PROGRAM_LOGICAL)))
			{
				address_space &space = device->memory().space(AS_PROGRAM + (spacenum - EXPSPACE_PROGRAM_LOGICAL));
				result = debug_read_memory(space, space.address_to_byte(address), size, true);
			}
			break;

		case EXPSPACE_PROGRAM_PHYSICAL:
		case EXPSPACE_DATA_PHYSICAL:
		case EXPSPACE_IO_PHYSICAL:
		case EXPSPACE_SPACE3_PHYSICAL:
			if (name != nullptr)
				device = expression_get_device(machine, name);
			if (device == nullptr)
				device = debug_cpu_get_visible_cpu(machine);
			if (device->memory().has_space(AS_PROGRAM + (spacenum - EXPSPACE_PROGRAM_PHYSICAL)))
			{
				address_space &space = device->memory().space(AS_PROGRAM + (spacenum - EXPSPACE_PROGRAM_PHYSICAL));
				result = debug_read_memory(space, space.address_to_byte(address), size, false);
			}
			break;

		case EXPSPACE_OPCODE:
		case EXPSPACE_RAMWRITE:
			if (name != nullptr)
				device = expression_get_device(machine, name);
			if (device == nullptr)
				device = debug_cpu_get_visible_cpu(machine);
			result = expression_read_program_direct(device->memory().space(AS_PROGRAM), (spacenum == EXPSPACE_OPCODE), address, size);
			break;

		case EXPSPACE_REGION:
			if (name == nullptr)
				break;
			result = expression_read_memory_region(machine, name, address, size);
			break;

		default:
			break;
	}
	return result;
}


/*-------------------------------------------------
    expression_read_program_direct - read memory
    directly from an opcode or RAM pointer
-------------------------------------------------*/

static UINT64 expression_read_program_direct(address_space &space, int opcode, offs_t address, int size)
{
	UINT64 result = ~(UINT64)0 >> (64 - 8*size);
	UINT8 *base;

	/* adjust the address into a byte address, but not if being called recursively */
	if ((opcode & 2) == 0)
		address = space.address_to_byte(address);

	/* call ourself recursively until we are byte-sized */
	if (size > 1)
	{
		int halfsize = size / 2;
		UINT64 r0, r1;

		/* read each half, from lower address to upper address */
		r0 = expression_read_program_direct(space, opcode | 2, address + 0, halfsize);
		r1 = expression_read_program_direct(space, opcode | 2, address + halfsize, halfsize);

		/* assemble based on the target endianness */
		if (space.endianness() == ENDIANNESS_LITTLE)
			result = r0 | (r1 << (8 * halfsize));
		else
			result = r1 | (r0 << (8 * halfsize));
	}

	/* handle the byte-sized final requests */
	else
	{
		/* lowmask specified which address bits are within the databus width */
		offs_t lowmask = space.data_width() / 8 - 1;

		/* get the base of memory, aligned to the address minus the lowbits */
		base = (UINT8 *)space.get_read_ptr(address & ~lowmask);

		/* if we have a valid base, return the appropriate byte */
		if (base != nullptr)
		{
			if (space.endianness() == ENDIANNESS_LITTLE)
				result = base[BYTE8_XOR_LE(address) & lowmask];
			else
				result = base[BYTE8_XOR_BE(address) & lowmask];
		}
	}
	return result;
}


/*-------------------------------------------------
    expression_read_memory_region - read memory
    from a memory region
-------------------------------------------------*/

static UINT64 expression_read_memory_region(running_machine &machine, const char *rgntag, offs_t address, int size)
{
	memory_region *region = machine.root_device().memregion(rgntag);
	UINT64 result = ~(UINT64)0 >> (64 - 8*size);

	/* make sure we get a valid base before proceeding */
	if (region != nullptr)
	{
		/* call ourself recursively until we are byte-sized */
		if (size > 1)
		{
			int halfsize = size / 2;
			UINT64 r0, r1;

			/* read each half, from lower address to upper address */
			r0 = expression_read_memory_region(machine, rgntag, address + 0, halfsize);
			r1 = expression_read_memory_region(machine, rgntag, address + halfsize, halfsize);

			/* assemble based on the target endianness */
			if (region->endianness() == ENDIANNESS_LITTLE)
				result = r0 | (r1 << (8 * halfsize));
			else
				result = r1 | (r0 << (8 * halfsize));
		}

		/* only process if we're within range */
		else if (address < region->bytes())
		{
			/* lowmask specified which address bits are within the databus width */
			UINT32 lowmask = region->bytewidth() - 1;
			UINT8 *base = region->base() + (address & ~lowmask);

			/* if we have a valid base, return the appropriate byte */
			if (region->endianness() == ENDIANNESS_LITTLE)
				result = base[BYTE8_XOR_LE(address) & lowmask];
			else
				result = base[BYTE8_XOR_BE(address) & lowmask];
		}
	}
	return result;
}


/*-------------------------------------------------
    expression_write_memory - write 1,2,4 or 8
    bytes at the given offset in the given address
    space
-------------------------------------------------*/

static void expression_write_memory(void *param, const char *name, expression_space spacenum, UINT32 address, int size, UINT64 data)
{
	running_machine &machine = *(running_machine *)param;
	device_t *device = nullptr;

	switch (spacenum)
	{
		case EXPSPACE_PROGRAM_LOGICAL:
		case EXPSPACE_DATA_LOGICAL:
		case EXPSPACE_IO_LOGICAL:
		case EXPSPACE_SPACE3_LOGICAL:
			if (name != nullptr)
				device = expression_get_device(machine, name);
			if (device == nullptr)
				device = debug_cpu_get_visible_cpu(machine);
			if (device->memory().has_space(AS_PROGRAM + (spacenum - EXPSPACE_PROGRAM_LOGICAL)))
			{
				address_space &space = device->memory().space(AS_PROGRAM + (spacenum - EXPSPACE_PROGRAM_LOGICAL));
				debug_write_memory(space, space.address_to_byte(address), data, size, true);
			}
			break;

		case EXPSPACE_PROGRAM_PHYSICAL:
		case EXPSPACE_DATA_PHYSICAL:
		case EXPSPACE_IO_PHYSICAL:
		case EXPSPACE_SPACE3_PHYSICAL:
			if (name != nullptr)
				device = expression_get_device(machine, name);
			if (device == nullptr)
				device = debug_cpu_get_visible_cpu(machine);
			if (device->memory().has_space(AS_PROGRAM + (spacenum - EXPSPACE_PROGRAM_PHYSICAL)))
			{
				address_space &space = device->memory().space(AS_PROGRAM + (spacenum - EXPSPACE_PROGRAM_PHYSICAL));
				debug_write_memory(space, space.address_to_byte(address), data, size, false);
			}
			break;

		case EXPSPACE_OPCODE:
		case EXPSPACE_RAMWRITE:
			if (name != nullptr)
				device = expression_get_device(machine, name);
			if (device == nullptr)
				device = debug_cpu_get_visible_cpu(machine);
			expression_write_program_direct(device->memory().space(AS_PROGRAM), (spacenum == EXPSPACE_OPCODE), address, size, data);
			break;

		case EXPSPACE_REGION:
			if (name == nullptr)
				break;
			expression_write_memory_region(machine, name, address, size, data);
			break;

		default:
			break;
	}
}


/*-------------------------------------------------
    expression_write_program_direct - write memory
    directly to an opcode or RAM pointer
-------------------------------------------------*/

static void expression_write_program_direct(address_space &space, int opcode, offs_t address, int size, UINT64 data)
{
	debugcpu_private *global = space.machine().debugcpu_data;
	UINT8 *base;

	/* adjust the address into a byte address, but not if being called recursively */
	if ((opcode & 2) == 0)
		address = space.address_to_byte(address);

	/* call ourself recursively until we are byte-sized */
	if (size > 1)
	{
		int halfsize = size / 2;
		UINT64 r0, r1, halfmask;

		/* break apart based on the target endianness */
		halfmask = ~(UINT64)0 >> (64 - 8 * halfsize);
		if (space.endianness() == ENDIANNESS_LITTLE)
		{
			r0 = data & halfmask;
			r1 = (data >> (8 * halfsize)) & halfmask;
		}
		else
		{
			r0 = (data >> (8 * halfsize)) & halfmask;
			r1 = data & halfmask;
		}

		/* write each half, from lower address to upper address */
		expression_write_program_direct(space, opcode | 2, address + 0, halfsize, r0);
		expression_write_program_direct(space, opcode | 2, address + halfsize, halfsize, r1);
	}

	/* handle the byte-sized final case */
	else
	{
		/* lowmask specified which address bits are within the databus width */
		offs_t lowmask = space.data_width() / 8 - 1;

		/* get the base of memory, aligned to the address minus the lowbits */
		base = (UINT8 *)space.get_read_ptr(address & ~lowmask);

		/* if we have a valid base, write the appropriate byte */
		if (base != nullptr)
		{
			if (space.endianness() == ENDIANNESS_LITTLE)
				base[BYTE8_XOR_LE(address) & lowmask] = data;
			else
				base[BYTE8_XOR_BE(address) & lowmask] = data;
			global->memory_modified = true;
		}
	}
}


/*-------------------------------------------------
    expression_write_memory_region - write memory
    from a memory region
-------------------------------------------------*/

static void expression_write_memory_region(running_machine &machine, const char *rgntag, offs_t address, int size, UINT64 data)
{
	debugcpu_private *global = machine.debugcpu_data;
	memory_region *region = machine.root_device().memregion(rgntag);

	/* make sure we get a valid base before proceeding */
	if (region != nullptr)
	{
		/* call ourself recursively until we are byte-sized */
		if (size > 1)
		{
			int halfsize = size / 2;
			UINT64 r0, r1, halfmask;

			/* break apart based on the target endianness */
			halfmask = ~(UINT64)0 >> (64 - 8 * halfsize);
			if (region->endianness() == ENDIANNESS_LITTLE)
			{
				r0 = data & halfmask;
				r1 = (data >> (8 * halfsize)) & halfmask;
			}
			else
			{
				r0 = (data >> (8 * halfsize)) & halfmask;
				r1 = data & halfmask;
			}

			/* write each half, from lower address to upper address */
			expression_write_memory_region(machine, rgntag, address + 0, halfsize, r0);
			expression_write_memory_region(machine, rgntag, address + halfsize, halfsize, r1);
		}

		/* only process if we're within range */
		else if (address < region->bytes())
		{
			/* lowmask specified which address bits are within the databus width */
			UINT32 lowmask = region->bytewidth() - 1;
			UINT8 *base = region->base() + (address & ~lowmask);

			/* if we have a valid base, set the appropriate byte */
			if (region->endianness() == ENDIANNESS_LITTLE)
				base[BYTE8_XOR_LE(address) & lowmask] = data;
			else
				base[BYTE8_XOR_BE(address) & lowmask] = data;
			global->memory_modified = true;
		}
	}
}


/*-------------------------------------------------
    expression_validate - validate that the
    provided expression references an
    appropriate name
-------------------------------------------------*/

static expression_error::error_code expression_validate(void *param, const char *name, expression_space space)
{
	running_machine &machine = *(running_machine *)param;
	device_t *device = nullptr;

	switch (space)
	{
		case EXPSPACE_PROGRAM_LOGICAL:
		case EXPSPACE_DATA_LOGICAL:
		case EXPSPACE_IO_LOGICAL:
		case EXPSPACE_SPACE3_LOGICAL:
			if (name != nullptr)
			{
				device = expression_get_device(machine, name);
				if (device == nullptr)
					return expression_error::INVALID_MEMORY_NAME;
			}
			if (device == nullptr)
				device = debug_cpu_get_visible_cpu(machine);
			if (!device->memory().has_space(AS_PROGRAM + (space - EXPSPACE_PROGRAM_LOGICAL)))
				return expression_error::NO_SUCH_MEMORY_SPACE;
			break;

		case EXPSPACE_PROGRAM_PHYSICAL:
		case EXPSPACE_DATA_PHYSICAL:
		case EXPSPACE_IO_PHYSICAL:
		case EXPSPACE_SPACE3_PHYSICAL:
			if (name != nullptr)
			{
				device = expression_get_device(machine, name);
				if (device == nullptr)
					return expression_error::INVALID_MEMORY_NAME;
			}
			if (device == nullptr)
				device = debug_cpu_get_visible_cpu(machine);
			if (!device->memory().has_space(AS_PROGRAM + (space - EXPSPACE_PROGRAM_PHYSICAL)))
				return expression_error::NO_SUCH_MEMORY_SPACE;
			break;

		case EXPSPACE_OPCODE:
		case EXPSPACE_RAMWRITE:
			if (name != nullptr)
			{
				device = expression_get_device(machine, name);
				if (device == nullptr)
					return expression_error::INVALID_MEMORY_NAME;
			}
			if (device == nullptr)
				device = debug_cpu_get_visible_cpu(machine);
			if (!device->memory().has_space(AS_PROGRAM))
				return expression_error::NO_SUCH_MEMORY_SPACE;
			break;

		case EXPSPACE_REGION:
			if (name == nullptr)
				return expression_error::MISSING_MEMORY_NAME;
			if (machine.root_device().memregion(name)->base() == nullptr)
				return expression_error::INVALID_MEMORY_NAME;
			break;

		default:
			return expression_error::NO_SUCH_MEMORY_SPACE;
	}
	return expression_error::NONE;
}



/***************************************************************************
    VARIABLE GETTERS/SETTERS
***************************************************************************/

/*-------------------------------------------------
    get_beamx - get beam horizontal position
-------------------------------------------------*/

static UINT64 get_beamx(symbol_table &table, void *ref)
{
	screen_device *screen = reinterpret_cast<screen_device *>(ref);
	return (screen != nullptr) ? screen->hpos() : 0;
}


/*-------------------------------------------------
    get_beamy - get beam vertical position
-------------------------------------------------*/

static UINT64 get_beamy(symbol_table &table, void *ref)
{
	screen_device *screen = reinterpret_cast<screen_device *>(ref);
	return (screen != nullptr) ? screen->vpos() : 0;
}


/*-------------------------------------------------
    get_frame - get current frame number
-------------------------------------------------*/

static UINT64 get_frame(symbol_table &table, void *ref)
{
	screen_device *screen = reinterpret_cast<screen_device *>(ref);
	return (screen != nullptr) ? screen->frame_number() : 0;
}


/*-------------------------------------------------
    get_cpunum - getter callback for the
    'cpunum' symbol
-------------------------------------------------*/

static UINT64 get_cpunum(symbol_table &table, void *ref)
{
	running_machine &machine = *reinterpret_cast<running_machine *>(table.globalref());
	device_t *target = machine.debugcpu_data->visiblecpu;

	execute_interface_iterator iter(machine.root_device());
	return iter.indexof(target->execute());
}



//**************************************************************************
//  DEVICE DEBUG
//**************************************************************************

//-------------------------------------------------
//  device_debug - constructor
//-------------------------------------------------

device_debug::device_debug(device_t &device)
	: m_device(device),
		m_exec(nullptr),
		m_memory(nullptr),
		m_state(nullptr),
		m_disasm(nullptr),
		m_flags(0),
		m_symtable(&device, debug_cpu_get_global_symtable(device.machine())),
		m_instrhook(nullptr),
		m_dasm_override(nullptr),
		m_opwidth(0),
		m_stepaddr(0),
		m_stepsleft(0),
		m_stopaddr(0),
		m_stoptime(attotime::zero),
		m_stopirq(0),
		m_stopexception(0),
		m_endexectime(attotime::zero),
		m_total_cycles(0),
		m_last_total_cycles(0),
		m_pc_history_index(0),
		m_bplist(nullptr),
		m_rplist(nullptr),
		m_trace(nullptr),
		m_hotspot_threshhold(0),
		m_track_pc_set(),
		m_track_pc(false),
		m_comment_set(),
		m_comment_change(0),
		m_track_mem_set(),
		m_track_mem(false)
{
	memset(m_pc_history, 0, sizeof(m_pc_history));
	memset(m_wplist, 0, sizeof(m_wplist));

	// find out which interfaces we have to work with
	device.interface(m_exec);
	device.interface(m_memory);
	device.interface(m_state);
	device.interface(m_disasm);

	// set up state-related stuff
	if (m_state != nullptr)
	{
		// add global symbol for cycles and totalcycles
		if (m_exec != nullptr)
		{
			m_symtable.add("cycles", nullptr, get_cycles);
			m_symtable.add("totalcycles", nullptr, get_totalcycles);
			m_symtable.add("lastinstructioncycles", nullptr, get_lastinstructioncycles);
		}

		// add entries to enable/disable unmap reporting for each space
		if (m_memory != nullptr)
		{
			if (m_memory->has_space(AS_PROGRAM))
				m_symtable.add("logunmap", (void *)&m_memory->space(AS_PROGRAM), get_logunmap, set_logunmap);
			if (m_memory->has_space(AS_DATA))
				m_symtable.add("logunmapd", (void *)&m_memory->space(AS_DATA), get_logunmap, set_logunmap);
			if (m_memory->has_space(AS_IO))
				m_symtable.add("logunmapi", (void *)&m_memory->space(AS_IO), get_logunmap, set_logunmap);
		}

		// add all registers into it
		std::string tempstr;
		for (const device_state_entry *entry = m_state->state_first(); entry != nullptr; entry = entry->next()) {
			strmakelower(tempstr.assign(entry->symbol()));
			m_symtable.add(tempstr.c_str(), (void *)(FPTR)entry->index(), get_state, set_state);
		}
	}

	// set up execution-related stuff
	if (m_exec != nullptr)
	{
		m_flags = DEBUG_FLAG_OBSERVING | DEBUG_FLAG_HISTORY;
		m_opwidth = min_opcode_bytes();

		// if no curpc, add one
		if (m_state != nullptr && m_symtable.find("curpc") == nullptr)
			m_symtable.add("curpc", nullptr, get_current_pc);
	}
}


//-------------------------------------------------
//  ~device_debug - constructor
//-------------------------------------------------

device_debug::~device_debug()
{
	// free breakpoints and watchpoints
	breakpoint_clear_all();
	watchpoint_clear_all();
	registerpoint_clear_all();
}


//-------------------------------------------------
//  start_hook - the scheduler calls this hook
//  before beginning execution for the given device
//-------------------------------------------------

void device_debug::start_hook(const attotime &endtime)
{
	debugcpu_private *global = m_device.machine().debugcpu_data;

	assert((m_device.machine().debug_flags & DEBUG_FLAG_ENABLED) != 0);

	// stash a pointer to the current live CPU
	assert(global->livecpu == nullptr);
	global->livecpu = &m_device;

	// if we're a new device, stop now
	if (global->m_stop_when_not_device != nullptr && global->m_stop_when_not_device != &m_device)
	{
		global->m_stop_when_not_device = nullptr;
		global->execution_state = EXECUTION_STATE_STOPPED;
		reset_transient_flags(m_device.machine());
	}

	// update the target execution end time
	m_endexectime = endtime;

	// if we're running, do some periodic updating
	if (global->execution_state != EXECUTION_STATE_STOPPED)
	{
		// check for periodic updates
		if (&m_device == global->visiblecpu && osd_ticks() > global->last_periodic_update_time + osd_ticks_per_second()/4)
		{
			m_device.machine().debug_view().update_all();
			m_device.machine().debug_view().flush_osd_updates();
			global->last_periodic_update_time = osd_ticks();
		}

		// check for pending breaks
		else if (&m_device == global->breakcpu)
		{
			global->execution_state = EXECUTION_STATE_STOPPED;
			global->breakcpu = nullptr;
		}

		// if a VBLANK occurred, check on things
		if (global->vblank_occurred)
		{
			global->vblank_occurred = false;

			// if we were waiting for a VBLANK, signal it now
			if ((m_flags & DEBUG_FLAG_STOP_VBLANK) != 0)
			{
				global->execution_state = EXECUTION_STATE_STOPPED;
				debug_console_printf(m_device.machine(), "Stopped at VBLANK\n");
			}
		}
		// check for debug keypresses
		if (m_device.machine().ui_input().pressed(IPT_UI_DEBUG_BREAK))
			global->visiblecpu->debug()->halt_on_next_instruction("User-initiated break\n");
	}

	// recompute the debugging mode
	compute_debug_flags();
}


//-------------------------------------------------
//  stop_hook - the scheduler calls this hook when
//  ending execution for the given device
//-------------------------------------------------

void device_debug::stop_hook()
{
	debugcpu_private *global = m_device.machine().debugcpu_data;

	assert(global->livecpu == &m_device);

	// clear the live CPU
	global->livecpu = nullptr;
}


//-------------------------------------------------
//  interrupt_hook - called when an interrupt is
//  acknowledged
//-------------------------------------------------

void device_debug::interrupt_hook(int irqline)
{
	debugcpu_private *global = m_device.machine().debugcpu_data;

	// see if this matches a pending interrupt request
	if ((m_flags & DEBUG_FLAG_STOP_INTERRUPT) != 0 && (m_stopirq == -1 || m_stopirq == irqline))
	{
		global->execution_state = EXECUTION_STATE_STOPPED;
		debug_console_printf(m_device.machine(), "Stopped on interrupt (CPU '%s', IRQ %d)\n", m_device.tag().c_str(), irqline);
		compute_debug_flags();
	}
}


//-------------------------------------------------
//  exception_hook - called when an exception is
//  generated
//-------------------------------------------------

void device_debug::exception_hook(int exception)
{
	debugcpu_private *global = m_device.machine().debugcpu_data;

	// see if this matches a pending interrupt request
	if ((m_flags & DEBUG_FLAG_STOP_EXCEPTION) != 0 && (m_stopexception == -1 || m_stopexception == exception))
	{
		global->execution_state = EXECUTION_STATE_STOPPED;
		debug_console_printf(m_device.machine(), "Stopped on exception (CPU '%s', exception %d)\n", m_device.tag().c_str(), exception);
		compute_debug_flags();
	}
}


//-------------------------------------------------
//  instruction_hook - called by the CPU cores
//  before executing each instruction
//-------------------------------------------------

void device_debug::instruction_hook(offs_t curpc)
{
	running_machine &machine = m_device.machine();
	debugcpu_private *global = machine.debugcpu_data;

	// note that we are in the debugger code
	global->within_instruction_hook = true;

	// update the history
	m_pc_history[m_pc_history_index++ % HISTORY_SIZE] = curpc;

	// update total cycles
	m_last_total_cycles = m_total_cycles;
	m_total_cycles = m_exec->total_cycles();

	// are we tracking our recent pc visits?
	if (m_track_pc)
	{
		const UINT32 crc = compute_opcode_crc32(curpc);
		m_track_pc_set.insert(dasm_pc_tag(curpc, crc));
	}

	// are we tracing?
	if (m_trace != nullptr)
		m_trace->update(curpc);

	// per-instruction hook?
	if (global->execution_state != EXECUTION_STATE_STOPPED && (m_flags & DEBUG_FLAG_HOOKED) != 0 && (*m_instrhook)(m_device, curpc))
		global->execution_state = EXECUTION_STATE_STOPPED;

	// handle single stepping
	if (global->execution_state != EXECUTION_STATE_STOPPED && (m_flags & DEBUG_FLAG_STEPPING_ANY) != 0)
	{
		// is this an actual step?
		if (m_stepaddr == ~0 || curpc == m_stepaddr)
		{
			// decrement the count and reset the breakpoint
			m_stepsleft--;
			m_stepaddr = ~0;

			// if we hit 0, stop
			if (m_stepsleft == 0)
				global->execution_state = EXECUTION_STATE_STOPPED;

			// update every 100 steps until we are within 200 of the end
			else if ((m_flags & DEBUG_FLAG_STEPPING_OUT) == 0 && (m_stepsleft < 200 || m_stepsleft % 100 == 0))
			{
				machine.debug_view().update_all();
				machine.debug_view().flush_osd_updates();
				machine.debugger().refresh_display();
			}
		}
	}

	// handle breakpoints
	if (global->execution_state != EXECUTION_STATE_STOPPED && (m_flags & (DEBUG_FLAG_STOP_TIME | DEBUG_FLAG_STOP_PC | DEBUG_FLAG_LIVE_BP)) != 0)
	{
		// see if we hit a target time
		if ((m_flags & DEBUG_FLAG_STOP_TIME) != 0 && machine.time() >= m_stoptime)
		{
			debug_console_printf(machine, "Stopped at time interval %.1g\n", machine.time().as_double());
			global->execution_state = EXECUTION_STATE_STOPPED;
		}

		// check the temp running breakpoint and break if we hit it
		else if ((m_flags & DEBUG_FLAG_STOP_PC) != 0 && m_stopaddr == curpc)
		{
			debug_console_printf(machine, "Stopped at temporary breakpoint %X on CPU '%s'\n", m_stopaddr, m_device.tag().c_str());
			global->execution_state = EXECUTION_STATE_STOPPED;
		}

		// check for execution breakpoints
		else if ((m_flags & DEBUG_FLAG_LIVE_BP) != 0)
			breakpoint_check(curpc);
	}

	// if we are supposed to halt, do it now
	if (global->execution_state == EXECUTION_STATE_STOPPED)
	{
		bool firststop = true;

		// load comments if we haven't yet
		if (!global->comments_loaded)
		{
			debug_comment_load(m_device.machine());
			global->comments_loaded = true;
		}

		// reset any transient state
		reset_transient_flags(m_device.machine());
		global->breakcpu = nullptr;

		// remember the last visible CPU in the debugger
		global->visiblecpu = &m_device;

		// update all views
		machine.debug_view().update_all();
		machine.debugger().refresh_display();

		// wait for the debugger; during this time, disable sound output
		m_device.machine().sound().debugger_mute(true);
		while (global->execution_state == EXECUTION_STATE_STOPPED)
		{
			// flush any pending updates before waiting again
			machine.debug_view().flush_osd_updates();

			// clear the memory modified flag and wait
			global->memory_modified = false;
			if (machine.debug_flags & DEBUG_FLAG_OSD_ENABLED)
				machine.osd().wait_for_debugger(m_device, firststop);
			firststop = false;

			// if something modified memory, update the screen
			if (global->memory_modified)
			{
				machine.debug_view().update_all(DVT_DISASSEMBLY);
				machine.debugger().refresh_display();
			}

			// check for commands in the source file
			process_source_file(m_device.machine());

			// if an event got scheduled, resume
			if (machine.scheduled_event_pending())
				global->execution_state = EXECUTION_STATE_RUNNING;
		}
		m_device.machine().sound().debugger_mute(false);

		// remember the last visible CPU in the debugger
		global->visiblecpu = &m_device;
	}

	// handle step out/over on the instruction we are about to execute
	if ((m_flags & (DEBUG_FLAG_STEPPING_OVER | DEBUG_FLAG_STEPPING_OUT)) != 0 && m_stepaddr == ~0)
		prepare_for_step_overout(pc());

	// no longer in debugger code
	global->within_instruction_hook = false;
}


//-------------------------------------------------
//  memory_read_hook - the memory system calls
//  this hook when watchpoints are enabled and a
//  memory read happens
//-------------------------------------------------

void device_debug::memory_read_hook(address_space &space, offs_t address, UINT64 mem_mask)
{
	// check watchpoints
	watchpoint_check(space, WATCHPOINT_READ, address, 0, mem_mask);

	// check hotspots
	if (!m_hotspots.empty())
		hotspot_check(space, address);
}


//-------------------------------------------------
//  memory_write_hook - the memory system calls
//  this hook when watchpoints are enabled and a
//  memory write happens
//-------------------------------------------------

void device_debug::memory_write_hook(address_space &space, offs_t address, UINT64 data, UINT64 mem_mask)
{
	if (m_track_mem)
	{
		dasm_memory_access const newAccess(space.spacenum(), address, data, history_pc(0));
		std::pair<std::set<dasm_memory_access>::iterator, bool> trackedAccess = m_track_mem_set.insert(newAccess);
		if (!trackedAccess.second)
			trackedAccess.first->m_pc = newAccess.m_pc;
	}
	watchpoint_check(space, WATCHPOINT_WRITE, address, data, mem_mask);
}


//-------------------------------------------------
//  set_instruction_hook - set a hook to be
//  called on each instruction for a given device
//-------------------------------------------------

void device_debug::set_instruction_hook(debug_instruction_hook_func hook)
{
	// set the hook and also the CPU's flag for fast knowledge of the hook
	m_instrhook = hook;
	if (hook != nullptr)
		m_flags |= DEBUG_FLAG_HOOKED;
	else
		m_flags &= ~DEBUG_FLAG_HOOKED;
}


//-------------------------------------------------
//  disassemble - disassemble a line at a given
//  PC on a given device
//-------------------------------------------------

offs_t device_debug::disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram) const
{
	offs_t result = 0;

	// check for disassembler override
	if (m_dasm_override != nullptr)
		result = (*m_dasm_override)(m_device, buffer, pc, oprom, opram, 0);

	// if we have a disassembler, run it
	if (result == 0 && m_disasm != nullptr)
		result = m_disasm->disassemble(buffer, pc, oprom, opram, 0);

	// make sure we get good results
	assert((result & DASMFLAG_LENGTHMASK) != 0);
#ifdef MAME_DEBUG
	if (m_memory != nullptr && m_disasm != nullptr)
	{
		address_space &space = m_memory->space(AS_PROGRAM);
		int bytes = space.address_to_byte(result & DASMFLAG_LENGTHMASK);
		assert(bytes >= m_disasm->min_opcode_bytes());
		assert(bytes <= m_disasm->max_opcode_bytes());
		(void) bytes; // appease compiler
	}
#endif

	return result;
}


//-------------------------------------------------
//  ignore - ignore/observe a given device
//-------------------------------------------------

void device_debug::ignore(bool ignore)
{
	debugcpu_private *global = m_device.machine().debugcpu_data;

	assert(m_exec != nullptr);

	if (ignore)
		m_flags &= ~DEBUG_FLAG_OBSERVING;
	else
		m_flags |= DEBUG_FLAG_OBSERVING;

	if (&m_device == global->livecpu && ignore)
		go_next_device();
}


//-------------------------------------------------
//  single_step - single step the device past the
//  requested number of instructions
//-------------------------------------------------

void device_debug::single_step(int numsteps)
{
	debugcpu_private *global = m_device.machine().debugcpu_data;

	assert(m_exec != nullptr);

	m_stepsleft = numsteps;
	m_stepaddr = ~0;
	m_flags |= DEBUG_FLAG_STEPPING;
	global->execution_state = EXECUTION_STATE_RUNNING;
}


//-------------------------------------------------
//  single_step_over - single step the device over
//  the requested number of instructions
//-------------------------------------------------

void device_debug::single_step_over(int numsteps)
{
	debugcpu_private *global = m_device.machine().debugcpu_data;

	assert(m_exec != nullptr);

	m_stepsleft = numsteps;
	m_stepaddr = ~0;
	m_flags |= DEBUG_FLAG_STEPPING_OVER;
	global->execution_state = EXECUTION_STATE_RUNNING;
}


//-------------------------------------------------
//  single_step_out - single step the device
//  out of the current function
//-------------------------------------------------

void device_debug::single_step_out()
{
	debugcpu_private *global = m_device.machine().debugcpu_data;

	assert(m_exec != nullptr);

	m_stepsleft = 100;
	m_stepaddr = ~0;
	m_flags |= DEBUG_FLAG_STEPPING_OUT;
	global->execution_state = EXECUTION_STATE_RUNNING;
}


//-------------------------------------------------
//  go - execute the device until it hits the given
//  address
//-------------------------------------------------

void device_debug::go(offs_t targetpc)
{
	debugcpu_private *global = m_device.machine().debugcpu_data;

	assert(m_exec != nullptr);

	m_stopaddr = targetpc;
	m_flags |= DEBUG_FLAG_STOP_PC;
	global->execution_state = EXECUTION_STATE_RUNNING;
}


//-------------------------------------------------
//  go_vblank - execute until the next VBLANK
//-------------------------------------------------

void device_debug::go_vblank()
{
	debugcpu_private *global = m_device.machine().debugcpu_data;

	assert(m_exec != nullptr);

	global->vblank_occurred = false;
	m_flags |= DEBUG_FLAG_STOP_VBLANK;
	global->execution_state = EXECUTION_STATE_RUNNING;
}


//-------------------------------------------------
//  go_interrupt - execute until the specified
//  interrupt fires on the device
//-------------------------------------------------

void device_debug::go_interrupt(int irqline)
{
	debugcpu_private *global = m_device.machine().debugcpu_data;

	assert(m_exec != nullptr);

	m_stopirq = irqline;
	m_flags |= DEBUG_FLAG_STOP_INTERRUPT;
	global->execution_state = EXECUTION_STATE_RUNNING;
}


//-------------------------------------------------
//  go_exception - execute until the specified
//  exception fires on the visible CPU
//-------------------------------------------------

void device_debug::go_exception(int exception)
{
	debugcpu_private *global = m_device.machine().debugcpu_data;

	assert(m_exec != nullptr);

	m_stopexception = exception;
	m_flags |= DEBUG_FLAG_STOP_EXCEPTION;
	global->execution_state = EXECUTION_STATE_RUNNING;
}


//-------------------------------------------------
//  go_milliseconds - execute until the specified
//  delay elapses
//-------------------------------------------------

void device_debug::go_milliseconds(UINT64 milliseconds)
{
	debugcpu_private *global = m_device.machine().debugcpu_data;

	assert(m_exec != nullptr);

	m_stoptime = m_device.machine().time() + attotime::from_msec(milliseconds);
	m_flags |= DEBUG_FLAG_STOP_TIME;
	global->execution_state = EXECUTION_STATE_RUNNING;
}


//-------------------------------------------------
//  go_next_device - execute until we hit the next
//  device
//-------------------------------------------------

void device_debug::go_next_device()
{
	debugcpu_private *global = m_device.machine().debugcpu_data;

	assert(m_exec != nullptr);

	global->m_stop_when_not_device = &m_device;
	global->execution_state = EXECUTION_STATE_RUNNING;
}


//-------------------------------------------------
//  halt_on_next_instruction - halt in the
//  debugger on the next instruction
//-------------------------------------------------

void device_debug::halt_on_next_instruction(const char *fmt, ...)
{
	debugcpu_private *global = m_device.machine().debugcpu_data;
	va_list arg;

	assert(m_exec != nullptr);

	// if something is pending on this CPU already, ignore this request
	if (&m_device == global->breakcpu)
		return;

	// output the message to the console
	va_start(arg, fmt);
	debug_console_vprintf(m_device.machine(), fmt, arg);
	va_end(arg);

	// if we are live, stop now, otherwise note that we want to break there
	if (&m_device == global->livecpu)
	{
		global->execution_state = EXECUTION_STATE_STOPPED;
		if (global->livecpu != nullptr)
			global->livecpu->debug()->compute_debug_flags();
	}
	else
		global->breakcpu = &m_device;
}


//-------------------------------------------------
//  breakpoint_set - set a new breakpoint,
//  returning its index
//-------------------------------------------------

int device_debug::breakpoint_set(offs_t address, const char *condition, const char *action)
{
	// allocate a new one
	breakpoint *bp = auto_alloc(m_device.machine(), breakpoint(this, m_symtable, m_device.machine().debugcpu_data->bpindex++, address, condition, action));

	// hook it into our list
	bp->m_next = m_bplist;
	m_bplist = bp;

	// update the flags and return the index
	breakpoint_update_flags();
	return bp->m_index;
}


//-------------------------------------------------
//  breakpoint_clear - clear a breakpoint by index,
//  returning true if we found it
//-------------------------------------------------

bool device_debug::breakpoint_clear(int index)
{
	// scan the list to see if we own this breakpoint
	for (breakpoint **bp = &m_bplist; *bp != nullptr; bp = &(*bp)->m_next)
		if ((*bp)->m_index == index)
		{
			breakpoint *deleteme = *bp;
			*bp = deleteme->m_next;
			auto_free(m_device.machine(), deleteme);
			breakpoint_update_flags();
			return true;
		}

	// we don't own it, return false
	return false;
}


//-------------------------------------------------
//  breakpoint_clear_all - clear all breakpoints
//-------------------------------------------------

void device_debug::breakpoint_clear_all()
{
	// clear the head until we run out
	while (m_bplist != nullptr)
		breakpoint_clear(m_bplist->index());
}


//-------------------------------------------------
//  breakpoint_enable - enable/disable a breakpoint
//  by index, returning true if we found it
//-------------------------------------------------

bool device_debug::breakpoint_enable(int index, bool enable)
{
	// scan the list to see if we own this breakpoint
	for (breakpoint *bp = m_bplist; bp != nullptr; bp = bp->next())
		if (bp->m_index == index)
		{
			bp->m_enabled = enable;
			breakpoint_update_flags();
			return true;
		}

	// we don't own it, return false
	return false;
}


//-------------------------------------------------
//  breakpoint_enable_all - enable/disable all
//  breakpoints
//-------------------------------------------------

void device_debug::breakpoint_enable_all(bool enable)
{
	// apply the enable to all breakpoints we own
	for (breakpoint *bp = m_bplist; bp != nullptr; bp = bp->next())
		breakpoint_enable(bp->index(), enable);
}


//-------------------------------------------------
//  watchpoint_set - set a new watchpoint,
//  returning its index
//-------------------------------------------------

int device_debug::watchpoint_set(address_space &space, int type, offs_t address, offs_t length, const char *condition, const char *action)
{
	assert(space.spacenum() < ARRAY_LENGTH(m_wplist));

	// allocate a new one
	watchpoint *wp = auto_alloc(m_device.machine(), watchpoint(this, m_symtable, m_device.machine().debugcpu_data->wpindex++, space, type, address, length, condition, action));

	// hook it into our list
	wp->m_next = m_wplist[space.spacenum()];
	m_wplist[space.spacenum()] = wp;

	// update the flags and return the index
	watchpoint_update_flags(wp->m_space);
	return wp->m_index;
}


//-------------------------------------------------
//  watchpoint_clear - clear a watchpoint by index,
//  returning true if we found it
//-------------------------------------------------

bool device_debug::watchpoint_clear(int index)
{
	// scan the list to see if we own this breakpoint
	for (address_spacenum spacenum = AS_0; spacenum < ARRAY_LENGTH(m_wplist); ++spacenum)
		for (watchpoint **wp = &m_wplist[spacenum]; *wp != nullptr; wp = &(*wp)->m_next)
			if ((*wp)->m_index == index)
			{
				watchpoint *deleteme = *wp;
				address_space &space = deleteme->m_space;
				*wp = deleteme->m_next;
				auto_free(m_device.machine(), deleteme);
				watchpoint_update_flags(space);
				return true;
			}

	// we don't own it, return false
	return false;
}


//-------------------------------------------------
//  watchpoint_clear_all - clear all watchpoints
//-------------------------------------------------

void device_debug::watchpoint_clear_all()
{
	// clear the head until we run out
	for (address_spacenum spacenum = AS_0; spacenum < ARRAY_LENGTH(m_wplist); ++spacenum)
		while (m_wplist[spacenum] != nullptr)
			watchpoint_clear(m_wplist[spacenum]->index());
}


//-------------------------------------------------
//  watchpoint_enable - enable/disable a watchpoint
//  by index, returning true if we found it
//-------------------------------------------------

bool device_debug::watchpoint_enable(int index, bool enable)
{
	// scan the list to see if we own this watchpoint
	for (address_spacenum spacenum = AS_0; spacenum < ARRAY_LENGTH(m_wplist); ++spacenum)
		for (watchpoint *wp = m_wplist[spacenum]; wp != nullptr; wp = wp->next())
			if (wp->m_index == index)
			{
				wp->m_enabled = enable;
				watchpoint_update_flags(wp->m_space);
				return true;
			}

	// we don't own it, return false
	return false;
}


//-------------------------------------------------
//  watchpoint_enable_all - enable/disable all
//  watchpoints
//-------------------------------------------------

void device_debug::watchpoint_enable_all(bool enable)
{
	// apply the enable to all watchpoints we own
	for (address_spacenum spacenum = AS_0; spacenum < ARRAY_LENGTH(m_wplist); ++spacenum)
		for (watchpoint *wp = m_wplist[spacenum]; wp != nullptr; wp = wp->next())
			watchpoint_enable(wp->index(), enable);
}


//-------------------------------------------------
//  registerpoint_set - set a new registerpoint,
//  returning its index
//-------------------------------------------------

int device_debug::registerpoint_set(const char *condition, const char *action)
{
	// allocate a new one
	registerpoint *rp = auto_alloc(m_device.machine(), registerpoint(m_symtable, m_device.machine().debugcpu_data->rpindex++, condition, action));

	// hook it into our list
	rp->m_next = m_rplist;
	m_rplist = rp;

	// update the flags and return the index
	breakpoint_update_flags();
	return rp->m_index;
}


//-------------------------------------------------
//  registerpoint_clear - clear a registerpoint by index,
//  returning true if we found it
//-------------------------------------------------

bool device_debug::registerpoint_clear(int index)
{
	// scan the list to see if we own this registerpoint
	for (registerpoint **rp = &m_rplist; *rp != nullptr; rp = &(*rp)->m_next)
		if ((*rp)->m_index == index)
		{
			registerpoint *deleteme = *rp;
			*rp = deleteme->m_next;
			auto_free(m_device.machine(), deleteme);
			breakpoint_update_flags();
			return true;
		}

	// we don't own it, return false
	return false;
}


//-------------------------------------------------
//  registerpoint_clear_all - clear all registerpoints
//-------------------------------------------------

void device_debug::registerpoint_clear_all()
{
	// clear the head until we run out
	while (m_rplist != nullptr)
		registerpoint_clear(m_rplist->index());
}


//-------------------------------------------------
//  registerpoint_enable - enable/disable a registerpoint
//  by index, returning true if we found it
//-------------------------------------------------

bool device_debug::registerpoint_enable(int index, bool enable)
{
	// scan the list to see if we own this conditionpoint
	for (registerpoint *rp = m_rplist; rp != nullptr; rp = rp->next())
		if (rp->m_index == index)
		{
			rp->m_enabled = enable;
			breakpoint_update_flags();
			return true;
		}

	// we don't own it, return false
	return false;
}


//-------------------------------------------------
//  registerpoint_enable_all - enable/disable all
//  registerpoints
//-------------------------------------------------

void device_debug::registerpoint_enable_all(bool enable)
{
	// apply the enable to all registerpoints we own
	for (registerpoint *rp = m_rplist; rp != nullptr; rp = rp->next())
		registerpoint_enable(rp->index(), enable);
}


//-------------------------------------------------
//  hotspot_track - enable/disable tracking of
//  hotspots
//-------------------------------------------------

void device_debug::hotspot_track(int numspots, int threshhold)
{
	// if we already have tracking enabled, kill it
	m_hotspots.clear();

	// only start tracking if we have a non-zero count
	if (numspots > 0)
	{
		// allocate memory for hotspots
		m_hotspots.resize(numspots);
		memset(&m_hotspots[0], 0xff, numspots*sizeof(m_hotspots[0]));

		// fill in the info
		m_hotspot_threshhold = threshhold;
	}

	// update the watchpoint flags to include us
	if (m_memory != nullptr && m_memory->has_space(AS_PROGRAM))
		watchpoint_update_flags(m_memory->space(AS_PROGRAM));
}


//-------------------------------------------------
//  history_pc - return an entry from the PC
//  history
//-------------------------------------------------

offs_t device_debug::history_pc(int index) const
{
	if (index > 0)
		index = 0;
	if (index <= -HISTORY_SIZE)
		index = -HISTORY_SIZE + 1;
	return m_pc_history[(m_pc_history_index + ARRAY_LENGTH(m_pc_history) - 1 + index) % ARRAY_LENGTH(m_pc_history)];
}


//-------------------------------------------------
//  track_pc_visited - returns a boolean stating
//  if this PC has been visited or not.  CRC32 is
//  done in this function on currently active CPU.
//  TODO: Take a CPU context as input
//-------------------------------------------------

bool device_debug::track_pc_visited(const offs_t& pc) const
{
	if (m_track_pc_set.empty())
		return false;
	const UINT32 crc = compute_opcode_crc32(pc);
	return m_track_pc_set.find(dasm_pc_tag(pc, crc)) != m_track_pc_set.end();
}


//-------------------------------------------------
//  set_track_pc_visited - set this pc as visited.
//  TODO: Take a CPU context as input
//-------------------------------------------------

void device_debug::set_track_pc_visited(const offs_t& pc)
{
	const UINT32 crc = compute_opcode_crc32(pc);
	m_track_pc_set.insert(dasm_pc_tag(pc, crc));
}


//-------------------------------------------------
//  track_mem_pc_from_address_data - returns the pc that
//  wrote the data to this address or (offs_t)(-1) for
//  'not available'.
//-------------------------------------------------

offs_t device_debug::track_mem_pc_from_space_address_data(const address_spacenum& space,
															const offs_t& address,
															const UINT64& data) const
{
	const offs_t missing = (offs_t)(-1);
	if (m_track_mem_set.empty())
		return missing;
	std::set<dasm_memory_access>::iterator const mem_access = m_track_mem_set.find(dasm_memory_access(space, address, data, 0));
	if (mem_access == m_track_mem_set.end()) return missing;
	return mem_access->m_pc;
}


//-------------------------------------------------
//  comment_add - adds a comment to the list at
//  the given address
//-------------------------------------------------

void device_debug::comment_add(offs_t addr, const char *comment, rgb_t color)
{
	// create a new item for the list
	UINT32 const crc = compute_opcode_crc32(addr);
	dasm_comment const newComment = dasm_comment(addr, crc, comment, color);
	std::pair<std::set<dasm_comment>::iterator, bool> const inserted = m_comment_set.insert(newComment);
	if (!inserted.second)
	{
		// Insert returns false if comment exists
		m_comment_set.erase(inserted.first);
		m_comment_set.insert(newComment);
	}

	// force an update
	m_comment_change++;
}


//-------------------------------------------------
//  comment_remove - removes a comment at the
//  given address with a matching CRC
//-------------------------------------------------

bool device_debug::comment_remove(offs_t addr)
{
	const UINT32 crc = compute_opcode_crc32(addr);
	size_t const removed = m_comment_set.erase(dasm_comment(addr, crc, "", 0xffffffff));
	if (removed != 0U) m_comment_change++;
	return removed != 0U;
}


//-------------------------------------------------
//  comment_text - return the text of a comment
//-------------------------------------------------

const char *device_debug::comment_text(offs_t addr) const
{
	const UINT32 crc = compute_opcode_crc32(addr);
	auto comment = m_comment_set.find(dasm_comment(addr, crc, "", 0));
	if (comment == m_comment_set.end()) return nullptr;
	return comment->m_text.c_str();
}


//-------------------------------------------------
//  comment_export - export the comments to the
//  given XML data node
//-------------------------------------------------

bool device_debug::comment_export(xml_data_node &curnode)
{
	// iterate through the comments
	std::string crc_buf;
	for (const auto & elem : m_comment_set)
	{
		xml_data_node *datanode = xml_add_child(&curnode, "comment", xml_normalize_string(elem.m_text.c_str()));
		if (datanode == nullptr)
			return false;
		xml_set_attribute_int(datanode, "address", elem.m_address);
		xml_set_attribute_int(datanode, "color", elem.m_color);
		strprintf(crc_buf,"%08X", elem.m_crc);
		xml_set_attribute(datanode, "crc", crc_buf.c_str());
	}
	return true;
}


//-------------------------------------------------
//  comment_import - import the comments from the
//  given XML data node
//-------------------------------------------------

bool device_debug::comment_import(xml_data_node &cpunode)
{
	// iterate through nodes
	for (xml_data_node *datanode = xml_get_sibling(cpunode.child, "comment"); datanode; datanode = xml_get_sibling(datanode->next, "comment"))
	{
		// extract attributes
		offs_t address = xml_get_attribute_int(datanode, "address", 0);
		rgb_t color = xml_get_attribute_int(datanode, "color", 0);

		UINT32 crc;
		sscanf(xml_get_attribute_string(datanode, "crc", nullptr), "%08X", &crc);

		// add the new comment
		m_comment_set.insert(dasm_comment(address, crc, datanode->value, color));
	}
	return true;
}


//-------------------------------------------------
//  compute_opcode_crc32 - determine the CRC of
//  the opcode bytes at the given address
//-------------------------------------------------

UINT32 device_debug::compute_opcode_crc32(offs_t pc) const
{
	// Basically the same thing as dasm_wrapped, but with some tiny savings
	assert(m_memory != nullptr);

	// determine the adjusted PC
	address_space &decrypted_space = m_memory->has_space(AS_DECRYPTED_OPCODES) ? m_memory->space(AS_DECRYPTED_OPCODES) : m_memory->space(AS_PROGRAM);
	address_space &space = m_memory->space(AS_PROGRAM);
	offs_t pcbyte = space.address_to_byte(pc) & space.bytemask();

	// fetch the bytes up to the maximum
	UINT8 opbuf[64], argbuf[64];
	int maxbytes = max_opcode_bytes();
	for (int numbytes = 0; numbytes < maxbytes; numbytes++)
	{
		opbuf[numbytes] = debug_read_opcode(decrypted_space, pcbyte + numbytes, 1);
		argbuf[numbytes] = debug_read_opcode(space, pcbyte + numbytes, 1);
	}

	// disassemble to our buffer
	char diasmbuf[200];
	memset(diasmbuf, 0x00, 200);
	UINT32 numbytes = disassemble(diasmbuf, pc, opbuf, argbuf) & DASMFLAG_LENGTHMASK;

	// return a CRC of the exact count of opcode bytes
	return core_crc32(0, opbuf, numbytes);
}


//-------------------------------------------------
//  trace - trace execution of a given device
//-------------------------------------------------

void device_debug::trace(FILE *file, bool trace_over, const char *action)
{
	// delete any existing tracers
	m_trace = nullptr;

	// if we have a new file, make a new tracer
	if (file != nullptr)
		m_trace = std::make_unique<tracer>(*this, *file, trace_over, action);
}


//-------------------------------------------------
//  trace_printf - output data into the given
//  device's tracefile, if tracing
//-------------------------------------------------

void device_debug::trace_printf(const char *fmt, ...)
{
	if (m_trace != nullptr)
	{
		va_list va;
		va_start(va, fmt);
		m_trace->vprintf(fmt, va);
		va_end(va);
	}
}


//-------------------------------------------------
//  compute_debug_flags - compute the global
//  debug flags for optimal efficiency
//-------------------------------------------------

void device_debug::compute_debug_flags()
{
	running_machine &machine = m_device.machine();
	debugcpu_private *global = machine.debugcpu_data;

	// clear out global flags by default, keep DEBUG_FLAG_OSD_ENABLED
	machine.debug_flags &= DEBUG_FLAG_OSD_ENABLED;
	machine.debug_flags |= DEBUG_FLAG_ENABLED;

	// if we are ignoring this CPU, or if events are pending, we're done
	if ((m_flags & DEBUG_FLAG_OBSERVING) == 0 || machine.scheduled_event_pending() || machine.save_or_load_pending())
		return;

	// if we're stopped, keep calling the hook
	if (global->execution_state == EXECUTION_STATE_STOPPED)
		machine.debug_flags |= DEBUG_FLAG_CALL_HOOK;

	// if we're tracking history, or we're hooked, or stepping, or stopping at a breakpoint
	// make sure we call the hook
	if ((m_flags & (DEBUG_FLAG_HISTORY | DEBUG_FLAG_HOOKED | DEBUG_FLAG_STEPPING_ANY | DEBUG_FLAG_STOP_PC | DEBUG_FLAG_LIVE_BP)) != 0)
		machine.debug_flags |= DEBUG_FLAG_CALL_HOOK;

	// also call if we are tracing
	if (m_trace != nullptr)
		machine.debug_flags |= DEBUG_FLAG_CALL_HOOK;

	// if we are stopping at a particular time and that time is within the current timeslice, we need to be called
	if ((m_flags & DEBUG_FLAG_STOP_TIME) && m_endexectime <= m_stoptime)
		machine.debug_flags |= DEBUG_FLAG_CALL_HOOK;
}


//-------------------------------------------------
//  prepare_for_step_overout - prepare things for
//  stepping over an instruction
//-------------------------------------------------

void device_debug::prepare_for_step_overout(offs_t pc)
{
	// disassemble the current instruction and get the flags
	std::string dasmbuffer;
	offs_t dasmresult = dasm_wrapped(dasmbuffer, pc);

	// if flags are supported and it's a call-style opcode, set a temp breakpoint after that instruction
	if ((dasmresult & DASMFLAG_SUPPORTED) != 0 && (dasmresult & DASMFLAG_STEP_OVER) != 0)
	{
		int extraskip = (dasmresult & DASMFLAG_OVERINSTMASK) >> DASMFLAG_OVERINSTSHIFT;
		pc += dasmresult & DASMFLAG_LENGTHMASK;

		// if we need to skip additional instructions, advance as requested
		while (extraskip-- > 0)
			pc += dasm_wrapped(dasmbuffer, pc) & DASMFLAG_LENGTHMASK;
		m_stepaddr = pc;
	}

	// if we're stepping out and this isn't a step out instruction, reset the steps until stop to a high number
	if ((m_flags & DEBUG_FLAG_STEPPING_OUT) != 0)
	{
		if ((dasmresult & DASMFLAG_SUPPORTED) != 0 && (dasmresult & DASMFLAG_STEP_OUT) == 0)
			m_stepsleft = 100;
		else
			m_stepsleft = 1;
	}
}


//-------------------------------------------------
//  breakpoint_update_flags - update the device's
//  breakpoint flags
//-------------------------------------------------

void device_debug::breakpoint_update_flags()
{
	// see if there are any enabled breakpoints
	m_flags &= ~DEBUG_FLAG_LIVE_BP;
	for (breakpoint *bp = m_bplist; bp != nullptr; bp = bp->m_next)
		if (bp->m_enabled)
		{
			m_flags |= DEBUG_FLAG_LIVE_BP;
			break;
		}

	if ( ! ( m_flags & DEBUG_FLAG_LIVE_BP ) )
	{
		// see if there are any enabled registerpoints
		for (registerpoint *rp = m_rplist; rp != nullptr; rp = rp->m_next)
		{
			if (rp->m_enabled)
			{
				m_flags |= DEBUG_FLAG_LIVE_BP;
			}
		}
	}

	// push the flags out globally
	debugcpu_private *global = m_device.machine().debugcpu_data;
	if (global->livecpu != nullptr)
		global->livecpu->debug()->compute_debug_flags();
}


//-------------------------------------------------
//  breakpoint_check - check the breakpoints for
//  a given device
//-------------------------------------------------

void device_debug::breakpoint_check(offs_t pc)
{
	// see if we match
	for (breakpoint *bp = m_bplist; bp != nullptr; bp = bp->m_next)
		if (bp->hit(pc))
		{
			// halt in the debugger by default
			debugcpu_private *global = m_device.machine().debugcpu_data;
			global->execution_state = EXECUTION_STATE_STOPPED;

			// if we hit, evaluate the action
			if (!bp->m_action.empty())
				debug_console_execute_command(m_device.machine(), bp->m_action.c_str(), 0);

			// print a notification, unless the action made us go again
			if (global->execution_state == EXECUTION_STATE_STOPPED)
				debug_console_printf(m_device.machine(), "Stopped at breakpoint %X\n", bp->m_index);
			break;
		}

	// see if we have any matching registerpoints
	for (registerpoint *rp = m_rplist; rp != nullptr; rp = rp->m_next)
	{
		if (rp->hit())
		{
			// halt in the debugger by default
			debugcpu_private *global = m_device.machine().debugcpu_data;
			global->execution_state = EXECUTION_STATE_STOPPED;

			// if we hit, evaluate the action
			if (!rp->m_action.empty())
			{
				debug_console_execute_command(m_device.machine(), rp->m_action.c_str(), 0);
			}

			// print a notification, unless the action made us go again
			if (global->execution_state == EXECUTION_STATE_STOPPED)
			{
				debug_console_printf(m_device.machine(), "Stopped at registerpoint %X\n", rp->m_index);
			}
			break;
		}
	}
}


//-------------------------------------------------
//  watchpoint_update_flags - update the device's
//  watchpoint flags
//-------------------------------------------------

void device_debug::watchpoint_update_flags(address_space &space)
{
	// if hotspots are enabled, turn on all reads
	bool enableread = false;
	if (!m_hotspots.empty())
		enableread = true;

	// see if there are any enabled breakpoints
	bool enablewrite = false;
	for (watchpoint *wp = m_wplist[space.spacenum()]; wp != nullptr; wp = wp->m_next)
		if (wp->m_enabled)
		{
			if (wp->m_type & WATCHPOINT_READ)
				enableread = true;
			if (wp->m_type & WATCHPOINT_WRITE)
				enablewrite = true;
		}

	// push the flags out globally
	space.enable_read_watchpoints(enableread);
	space.enable_write_watchpoints(enablewrite);
}


//-------------------------------------------------
//  watchpoint_check - check the watchpoints
//  for a given CPU and address space
//-------------------------------------------------

void device_debug::watchpoint_check(address_space &space, int type, offs_t address, UINT64 value_to_write, UINT64 mem_mask)
{
	debugcpu_private *global = space.machine().debugcpu_data;

	// if we're within debugger code, don't stop
	if (global->within_instruction_hook || global->debugger_access)
		return;
	global->within_instruction_hook = true;

	// adjust address, size & value_to_write based on mem_mask.
	offs_t size = 0;
	if (mem_mask != 0)
	{
		int bus_size = space.data_width() / 8;
		int address_offset = 0;

		while (address_offset < bus_size && (mem_mask & 0xff) == 0)
		{
			address_offset++;
			value_to_write >>= 8;
			mem_mask >>= 8;
		}

		while (mem_mask != 0)
		{
			size++;
			mem_mask >>= 8;
		}

		// (1<<(size*8))-1 won't work when size is 8; let's just use a lut
		static const UINT64 masks[] = {0,
										0xff,
										0xffff,
										0xffffff,
										0xffffffff,
									U64(0xffffffffff),
									U64(0xffffffffffff),
									U64(0xffffffffffffff),
									U64(0xffffffffffffffff)};
		value_to_write &= masks[size];

		if (space.endianness() == ENDIANNESS_LITTLE)
			address += address_offset;
		else
			address += bus_size - size - address_offset;
	}

	// if we are a write watchpoint, stash the value that will be written
	global->wpaddr = address;
	if (type & WATCHPOINT_WRITE)
		global->wpdata = value_to_write;

	// see if we match
	for (watchpoint *wp = m_wplist[space.spacenum()]; wp != nullptr; wp = wp->m_next)
		if (wp->hit(type, address, size))
		{
			// halt in the debugger by default
			global->execution_state = EXECUTION_STATE_STOPPED;

			// if we hit, evaluate the action
			if (!wp->m_action.empty())
				debug_console_execute_command(space.machine(), wp->m_action.c_str(), 0);

			// print a notification, unless the action made us go again
			if (global->execution_state == EXECUTION_STATE_STOPPED)
			{
				static const char *const sizes[] =
				{
					"0bytes", "byte", "word", "3bytes", "dword", "5bytes", "6bytes", "7bytes", "qword"
				};
				offs_t pc = (space.device().debug()->m_state != nullptr) ? space.device().debug()->m_state->pc() : 0;
				std::string buffer;

				if (type & WATCHPOINT_WRITE)
				{
					strprintf(buffer, "Stopped at watchpoint %X writing %s to %08X (PC=%X)", wp->m_index, sizes[size], space.byte_to_address(address), pc);
					if (value_to_write >> 32)
						strcatprintf(buffer, " (data=%X%08X)", (UINT32)(value_to_write >> 32), (UINT32)value_to_write);
					else
						strcatprintf(buffer, " (data=%X)", (UINT32)value_to_write);
				}
				else
					strprintf(buffer,"Stopped at watchpoint %X reading %s from %08X (PC=%X)", wp->m_index, sizes[size], space.byte_to_address(address), pc);
				debug_console_printf(space.machine(), "%s\n", buffer.c_str());
				space.device().debug()->compute_debug_flags();
			}
			break;
		}

	global->within_instruction_hook = false;
}


//-------------------------------------------------
//  hotspot_check - check for hotspots on a
//  memory read access
//-------------------------------------------------

void device_debug::hotspot_check(address_space &space, offs_t address)
{
	offs_t curpc = pc();

	// see if we have a match in our list
	unsigned int hotindex;
	for (hotindex = 0; hotindex < m_hotspots.size(); hotindex++)
		if (m_hotspots[hotindex].m_access == address && m_hotspots[hotindex].m_pc == curpc && m_hotspots[hotindex].m_space == &space)
			break;

	// if we didn't find any, make a new entry
	if (hotindex == m_hotspots.size())
	{
		// if the bottom of the list is over the threshhold, print it
		hotspot_entry &spot = m_hotspots[m_hotspots.size() - 1];
		if (spot.m_count > m_hotspot_threshhold)
			debug_console_printf(space.machine(), "Hotspot @ %s %08X (PC=%08X) hit %d times (fell off bottom)\n", space.name(), spot.m_access, spot.m_pc, spot.m_count);

		// move everything else down and insert this one at the top
		memmove(&m_hotspots[1], &m_hotspots[0], sizeof(m_hotspots[0]) * (m_hotspots.size() - 1));
		m_hotspots[0].m_access = address;
		m_hotspots[0].m_pc = curpc;
		m_hotspots[0].m_space = &space;
		m_hotspots[0].m_count = 1;
	}

	// if we did find one, increase the count and move it to the top
	else
	{
		m_hotspots[hotindex].m_count++;
		if (hotindex != 0)
		{
			hotspot_entry temp = m_hotspots[hotindex];
			memmove(&m_hotspots[1], &m_hotspots[0], sizeof(m_hotspots[0]) * hotindex);
			m_hotspots[0] = temp;
		}
	}
}


//-------------------------------------------------
//  dasm_wrapped - wraps calls to the disassembler
//  by fetching the opcode bytes to a temporary
//  buffer and then disassembling them
//-------------------------------------------------

UINT32 device_debug::dasm_wrapped(std::string &buffer, offs_t pc)
{
	assert(m_memory != nullptr && m_disasm != nullptr);

	// determine the adjusted PC
	address_space &decrypted_space = m_memory->has_space(AS_DECRYPTED_OPCODES) ? m_memory->space(AS_DECRYPTED_OPCODES) : m_memory->space(AS_PROGRAM);
	address_space &space = m_memory->space(AS_PROGRAM);
	offs_t pcbyte = space.address_to_byte(pc) & space.bytemask();

	// fetch the bytes up to the maximum
	UINT8 opbuf[64], argbuf[64];
	int maxbytes = max_opcode_bytes();
	for (int numbytes = 0; numbytes < maxbytes; numbytes++)
	{
		opbuf[numbytes] = debug_read_opcode(decrypted_space, pcbyte + numbytes, 1);
		argbuf[numbytes] = debug_read_opcode(space, pcbyte + numbytes, 1);
	}

	// disassemble to our buffer
	char diasmbuf[200];
	memset(diasmbuf, 0x00, 200);
	UINT32 result = disassemble(diasmbuf, pc, opbuf, argbuf);
	buffer.assign(diasmbuf);
	return result;
}


//-------------------------------------------------
//  get_current_pc - getter callback for a device's
//  current instruction pointer
//-------------------------------------------------

UINT64 device_debug::get_current_pc(symbol_table &table, void *ref)
{
	device_t *device = reinterpret_cast<device_t *>(table.globalref());
	return device->debug()->pc();
}


//-------------------------------------------------
//  get_cycles - getter callback for the
//  'cycles' symbol
//-------------------------------------------------

UINT64 device_debug::get_cycles(symbol_table &table, void *ref)
{
	device_t *device = reinterpret_cast<device_t *>(table.globalref());
	return device->debug()->m_exec->cycles_remaining();
}


//-------------------------------------------------
//  get_totalcycles - getter callback for the
//  'totalcycles' symbol
//-------------------------------------------------

UINT64 device_debug::get_totalcycles(symbol_table &table, void *ref)
{
	device_t *device = reinterpret_cast<device_t *>(table.globalref());
	return device->debug()->m_total_cycles;
}


//-------------------------------------------------
//  get_lastinstructioncycles - getter callback for the
//  'lastinstructioncycles' symbol
//-------------------------------------------------

UINT64 device_debug::get_lastinstructioncycles(symbol_table &table, void *ref)
{
	device_t *device = reinterpret_cast<device_t *>(table.globalref());
	device_debug *debug = device->debug();
	return debug->m_total_cycles - debug->m_last_total_cycles;
}


//-------------------------------------------------
//  get_logunmap - getter callback for the logumap
//  symbols
//-------------------------------------------------

UINT64 device_debug::get_logunmap(symbol_table &table, void *ref)
{
	address_space &space = *reinterpret_cast<address_space *>(table.globalref());
	return space.log_unmap();
}


//-------------------------------------------------
//  set_logunmap - setter callback for the logumap
//  symbols
//-------------------------------------------------

void device_debug::set_logunmap(symbol_table &table, void *ref, UINT64 value)
{
	address_space &space = *reinterpret_cast<address_space *>(table.globalref());
	space.set_log_unmap(value ? true : false);
}


//-------------------------------------------------
//  get_state - getter callback for a device's
//  state symbols
//-------------------------------------------------

UINT64 device_debug::get_state(symbol_table &table, void *ref)
{
	device_t *device = reinterpret_cast<device_t *>(table.globalref());
	return device->debug()->m_state->state_int(reinterpret_cast<FPTR>(ref));
}


//-------------------------------------------------
//  set_state - setter callback for a device's
//  state symbols
//-------------------------------------------------

void device_debug::set_state(symbol_table &table, void *ref, UINT64 value)
{
	device_t *device = reinterpret_cast<device_t *>(table.globalref());
	device->debug()->m_state->set_state_int(reinterpret_cast<FPTR>(ref), value);
}



//**************************************************************************
//  DEBUG BREAKPOINT
//**************************************************************************

//-------------------------------------------------
//  breakpoint - constructor
//-------------------------------------------------

device_debug::breakpoint::breakpoint(device_debug* debugInterface,
										symbol_table &symbols,
										int index,
										offs_t address,
										const char *condition,
										const char *action)
	: m_debugInterface(debugInterface),
		m_next(nullptr),
		m_index(index),
		m_enabled(true),
		m_address(address),
		m_condition(&symbols, (condition != nullptr) ? condition : "1"),
		m_action((action != nullptr) ? action : "")
{
}


//-------------------------------------------------
//  hit - detect a hit
//-------------------------------------------------

bool device_debug::breakpoint::hit(offs_t pc)
{
	// don't hit if disabled
	if (!m_enabled)
		return false;

	// must match our address
	if (m_address != pc)
		return false;

	// must satisfy the condition
	if (!m_condition.is_empty())
	{
		try
		{
			return (m_condition.execute() != 0);
		}
		catch (expression_error &)
		{
			return false;
		}
	}

	return true;
}



//**************************************************************************
//  DEBUG WATCHPOINT
//**************************************************************************

//-------------------------------------------------
//  watchpoint - constructor
//-------------------------------------------------

device_debug::watchpoint::watchpoint(device_debug* debugInterface,
										symbol_table &symbols,
										int index,
										address_space &space,
										int type,
										offs_t address,
										offs_t length,
										const char *condition,
										const char *action)
	: m_debugInterface(debugInterface),
		m_next(nullptr),
		m_space(space),
		m_index(index),
		m_enabled(true),
		m_type(type),
		m_address(space.address_to_byte(address) & space.bytemask()),
		m_length(space.address_to_byte(length)),
		m_condition(&symbols, (condition != nullptr) ? condition : "1"),
		m_action((action != nullptr) ? action : "")
{
}


//-------------------------------------------------
//  hit - detect a hit
//-------------------------------------------------

bool device_debug::watchpoint::hit(int type, offs_t address, int size)
{
	// don't hit if disabled
	if (!m_enabled)
		return false;

	// must match the type
	if ((m_type & type) == 0)
		return false;

	// must match our address
	if (address + size <= m_address || address >= m_address + m_length)
		return false;

	// must satisfy the condition
	if (!m_condition.is_empty())
	{
		try
		{
			return (m_condition.execute() != 0);
		}
		catch (expression_error &)
		{
			return false;
		}
	}
	return true;
}



//**************************************************************************
//  DEBUG REGISTERPOINT
//**************************************************************************

//-------------------------------------------------
//  registerpoint - constructor
//-------------------------------------------------

device_debug::registerpoint::registerpoint(symbol_table &symbols, int index, const char *condition, const char *action)
	: m_next(nullptr),
		m_index(index),
		m_enabled(true),
		m_condition(&symbols, (condition != nullptr) ? condition : "1"),
		m_action((action != nullptr) ? action : "")
{
}


//-------------------------------------------------
//  hit - detect a hit
//-------------------------------------------------

bool device_debug::registerpoint::hit()
{
	// don't hit if disabled
	if (!m_enabled)
		return false;

	// must satisfy the condition
	if (!m_condition.is_empty())
	{
		try
		{
			return (m_condition.execute() != 0);
		}
		catch (expression_error &)
		{
			return false;
		}
	}

	return true;
}



//**************************************************************************
//  TRACER
//**************************************************************************

//-------------------------------------------------
//  tracer - constructor
//-------------------------------------------------

device_debug::tracer::tracer(device_debug &debug, FILE &file, bool trace_over, const char *action)
	: m_debug(debug),
		m_file(file),
		m_action((action != nullptr) ? action : ""),
		m_loops(0),
		m_nextdex(0),
		m_trace_over(trace_over),
		m_trace_over_target(~0)
{
	memset(m_history, 0, sizeof(m_history));
}


//-------------------------------------------------
//  ~tracer - destructor
//-------------------------------------------------

device_debug::tracer::~tracer()
{
	// make sure we close the file if we can
	fclose(&m_file);
}


//-------------------------------------------------
//  update - log to the tracefile the data for a
//  given instruction
//-------------------------------------------------

void device_debug::tracer::update(offs_t pc)
{
	// are we in trace over mode and in a subroutine?
	if (m_trace_over && m_trace_over_target != ~0)
	{
		if (m_trace_over_target != pc)
			return;
		m_trace_over_target = ~0;
	}

	// check for a loop condition
	int count = 0;
	for (auto & elem : m_history)
		if (elem == pc)
			count++;

	// if more than 1 hit, just up the loop count and get out
	if (count > 1)
	{
		m_loops++;
		return;
	}

	// if we just finished looping, indicate as much
	if (m_loops != 0)
		fprintf(&m_file, "\n   (loops for %d instructions)\n\n", m_loops);
	m_loops = 0;

	// execute any trace actions first
	if (!m_action.empty())
		debug_console_execute_command(m_debug.m_device.machine(), m_action.c_str(), 0);

	// print the address
	std::string buffer;
	int logaddrchars = m_debug.logaddrchars();
	strprintf(buffer,"%0*X: ", logaddrchars, pc);

	// print the disassembly
	std::string dasm;
	offs_t dasmresult = m_debug.dasm_wrapped(dasm, pc);
	buffer.append(dasm);

	// output the result
	fprintf(&m_file, "%s\n", buffer.c_str());

	// do we need to step the trace over this instruction?
	if (m_trace_over && (dasmresult & DASMFLAG_SUPPORTED) != 0 && (dasmresult & DASMFLAG_STEP_OVER) != 0)
	{
		int extraskip = (dasmresult & DASMFLAG_OVERINSTMASK) >> DASMFLAG_OVERINSTSHIFT;
		offs_t trace_over_target = pc + (dasmresult & DASMFLAG_LENGTHMASK);

		// if we need to skip additional instructions, advance as requested
		while (extraskip-- > 0)
			trace_over_target += m_debug.dasm_wrapped(dasm, trace_over_target) & DASMFLAG_LENGTHMASK;

		m_trace_over_target = trace_over_target;
	}

	// log this PC
	m_nextdex = (m_nextdex + 1) % TRACE_LOOPS;
	m_history[m_nextdex] = pc;
}


//-------------------------------------------------
//  vprintf - generic print to the trace file
//-------------------------------------------------

void device_debug::tracer::vprintf(const char *format, va_list va)
{
	// pass through to the file
	vfprintf(&m_file, format, va);
}


//-------------------------------------------------
//  flush - flush any pending changes to the trace
//  file
//-------------------------------------------------

void device_debug::tracer::flush()
{
	fflush(&m_file);
}


//-------------------------------------------------
//  dasm_pc_tag - constructor
//-------------------------------------------------

device_debug::dasm_pc_tag::dasm_pc_tag(const offs_t& address, const UINT32& crc)
	: m_address(address),
		m_crc(crc)
{
}

//-------------------------------------------------
//  dasm_memory_access - constructor
//-------------------------------------------------

device_debug::dasm_memory_access::dasm_memory_access(const address_spacenum& address_space,
														const offs_t& address,
														const UINT64& data,
														const offs_t& pc)
	: m_address_space(address_space),
		m_address(address),
		m_data(data),
		m_pc(pc)
{
}

//-------------------------------------------------
//  dasm_comment - constructor
//-------------------------------------------------

device_debug::dasm_comment::dasm_comment(offs_t address, UINT32 crc, const char *text, rgb_t color)
	: dasm_pc_tag(address, crc),
		m_text(text),
		m_color(std::move(color))
{
}

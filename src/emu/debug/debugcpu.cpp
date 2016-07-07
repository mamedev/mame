// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    debugcpu.c

    Debugger CPU/memory interface engine.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "debugcpu.h"
#include "debugcon.h"
#include "devdebug.h"
#include "express.h"
#include "debugvw.h"
#include "debugger.h"
#include "uiinput.h"
#include "xmlfile.h"
#include <ctype.h>

const size_t debugger_cpu::NUM_TEMP_VARIABLES = 10;

/*-------------------------------------------------
    constructor - initialize the CPU
    information for debugging
-------------------------------------------------*/

debugger_cpu::debugger_cpu(running_machine &machine)
	: m_machine(machine)
	, m_livecpu(nullptr)
	, m_visiblecpu(nullptr)
	, m_breakcpu(nullptr)
	, m_source_file(nullptr)
	, m_symtable(nullptr)
	, m_execution_state(EXECUTION_STATE_STOPPED)
	, m_bpindex(1)
	, m_wpindex(1)
	, m_rpindex(1)
	, m_last_periodic_update_time(0)
	, m_comments_loaded(false)
{
	screen_device *first_screen = m_machine.first_screen();

	m_tempvar = make_unique_clear<UINT64[]>(NUM_TEMP_VARIABLES);

	/* create a global symbol table */
	m_symtable = std::make_unique<symbol_table>(&m_machine);

	// configure our base memory accessors
	configure_memory(*m_symtable);

	// add various things to the global symbol table
	using namespace std::placeholders;
	m_symtable->add("cpunum", nullptr, std::bind(&debugger_cpu::get_cpunum, this, _1, _2));
	m_symtable->add("beamx", (void *)first_screen, std::bind(&debugger_cpu::get_beamx, this, _1, _2));
	m_symtable->add("beamy", (void *)first_screen, std::bind(&debugger_cpu::get_beamy, this, _1, _2));
	m_symtable->add("frame", (void *)first_screen, std::bind(&debugger_cpu::get_frame, this, _1, _2));

	/* add the temporary variables to the global symbol table */
	for (int regnum = 0; regnum < NUM_TEMP_VARIABLES; regnum++)
	{
		char symname[10];
		sprintf(symname, "temp%d", regnum);
		m_symtable->add(symname, symbol_table::READ_WRITE, &m_tempvar[regnum]);
	}

	/* first CPU is visible by default */
	m_visiblecpu = m_machine.firstcpu;

	/* add callback for breaking on VBLANK */
	if (m_machine.first_screen() != nullptr)
		m_machine.first_screen()->register_vblank_callback(vblank_state_delegate(FUNC(debugger_cpu::on_vblank), this));
}

void debugger_cpu::configure_memory(symbol_table &table)
{
	using namespace std::placeholders;
	table.configure_memory(
		&m_machine,
		std::bind(&debugger_cpu::expression_validate, this, _1, _2, _3),
		std::bind(&debugger_cpu::expression_read_memory, this, _1, _2, _3, _4, _5),
		std::bind(&debugger_cpu::expression_write_memory, this, _1, _2, _3, _4, _5, _6));
}

/*-------------------------------------------------
    flush_traces - flushes all traces; this is
    useful if a trace is going on when we
    fatalerror
-------------------------------------------------*/

void debugger_cpu::flush_traces()
{
	/* this can be called on exit even when no debugging is enabled, so
	 make sure the devdebug is valid before proceeding */
	for (device_t &device : device_iterator(m_machine.root_device()))
		if (device.debug() != nullptr)
			device.debug()->trace_flush();
}



/***************************************************************************
    DEBUGGING STATUS AND INFORMATION
***************************************************************************/

/*-------------------------------------------------
    get_visible_cpu - return the visible CPU
    device (the one that commands should apply to)
-------------------------------------------------*/

device_t* debugger_cpu::get_visible_cpu()
{
	return m_visiblecpu;
}


/*-------------------------------------------------
    within_instruction_hook - true if the debugger
    is currently live
-------------------------------------------------*/

bool debugger_cpu::within_instruction_hook()
{
	return m_within_instruction_hook;
}


/*-------------------------------------------------
    is_stopped - return true if the
    current execution state is stopped
-------------------------------------------------*/

bool debugger_cpu::is_stopped()
{
	return m_execution_state == EXECUTION_STATE_STOPPED;
}


/***************************************************************************
    SYMBOL TABLE INTERFACES
***************************************************************************/

/*-------------------------------------------------
    get_global_symtable - return the global
    symbol table
-------------------------------------------------*/

symbol_table* debugger_cpu::get_global_symtable()
{
	return m_symtable.get();
}


/*-------------------------------------------------
    get_visible_symtable - return the
    locally-visible symbol table
-------------------------------------------------*/

symbol_table* debugger_cpu::get_visible_symtable()
{
	return &m_visiblecpu->debug()->symtable();
}


/*-------------------------------------------------
    source_script - specifies a debug command
    script to execute
-------------------------------------------------*/

void debugger_cpu::source_script(const char *file)
{
	/* close any existing source file */
	if (m_source_file != nullptr)
	{
		fclose(m_source_file);
		m_source_file = nullptr;
	}

	/* open a new one if requested */
	if (file != nullptr)
	{
		m_source_file = fopen(file, "r");
		if (!m_source_file)
		{
			if (m_machine.phase() == MACHINE_PHASE_RUNNING)
				m_machine.debugger().console().printf("Cannot open command file '%s'\n", file);
			else
				fatalerror("Cannot open command file '%s'\n", file);
		}
	}
}



//**************************************************************************
//  MEMORY AND DISASSEMBLY HELPERS
//**************************************************************************

//-------------------------------------------------
//  omment_save - save all comments for the given
//  machine
//-------------------------------------------------

bool debugger_cpu::comment_save()
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
		xml_set_attribute(systemnode, "name", m_machine.system().name);

		// for each device
		bool found_comments = false;
		for (device_t &device : device_iterator(m_machine.root_device()))
			if (device.debug() && device.debug()->comment_count() > 0)
			{
				// create a node for this device
				xml_data_node *curnode = xml_add_child(systemnode, "cpu", nullptr);
				if (curnode == nullptr)
					throw emu_exception();
				xml_set_attribute(curnode, "tag", device.tag());

				// export the comments
				if (!device.debug()->comment_export(*curnode))
					throw emu_exception();
				found_comments = true;
			}

		// flush the file
		if (found_comments)
		{
			emu_file file(m_machine.options().comment_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
			osd_file::error filerr = file.open(m_machine.basename(), ".cmt");
			if (filerr == osd_file::error::NONE)
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
//  comment_load - load all comments for the given
//  machine
//-------------------------------------------------

bool debugger_cpu::comment_load(bool is_inline)
{
	// open the file
	emu_file file(m_machine.options().comment_directory(), OPEN_FLAG_READ);
	osd_file::error filerr = file.open(m_machine.basename(), ".cmt");

	// if an error, just return false
	if (filerr != osd_file::error::NONE)
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
		if (strcmp(name, m_machine.system().name) != 0)
			throw emu_exception();

		// iterate over devices
		for (xml_data_node *cpunode = xml_get_sibling(systemnode->child, "cpu"); cpunode; cpunode = xml_get_sibling(cpunode->next, "cpu"))
		{
			const char *cputag_name = xml_get_attribute_string(cpunode, "tag", "");
			device_t *device = m_machine.device(cputag_name);
			if (device != nullptr)
			{
				if(is_inline == false)
					m_machine.debugger().console().printf("@%s\n", cputag_name);

				if (!device->debug()->comment_import(*cpunode,is_inline))
					throw emu_exception();
			}
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
    DEBUGGER MEMORY ACCESSORS
***************************************************************************/

/*-------------------------------------------------
    read_byte - return a byte from the specified
    memory space
-------------------------------------------------*/

UINT8 debugger_cpu::read_byte(address_space &space, offs_t address, int apply_translation)
{
	device_memory_interface &memory = space.device().memory();

	/* mask against the logical byte mask */
	address &= space.logbytemask();

	/* all accesses from this point on are for the debugger */
	m_debugger_access = true;
	space.set_debugger_access(true);

	/* translate if necessary; if not mapped, return 0xff */
	UINT64 custom;
	UINT8 result;
	if (apply_translation && !memory.translate(space.spacenum(), TRANSLATE_READ_DEBUG, address))
	{
		result = 0xff;
	}
	else if (memory.read(space.spacenum(), address, 1, custom))
	{   /* if there is a custom read handler, and it returns true, use that value */
		result = custom;
	}
	else
	{   /* otherwise, call the byte reading function for the translated address */
		result = space.read_byte(address);
	}

	/* no longer accessing via the debugger */
	m_debugger_access = false;
	space.set_debugger_access(false);
	return result;
}


/*-------------------------------------------------
    read_word - return a word from the specified
    memory space
-------------------------------------------------*/

UINT16 debugger_cpu::read_word(address_space &space, offs_t address, int apply_translation)
{
	/* mask against the logical byte mask */
	address &= space.logbytemask();

	UINT16 result;
	if (!WORD_ALIGNED(address))
	{   /* if this is misaligned read, or if there are no word readers, just read two bytes */
		UINT8 byte0 = read_byte(space, address + 0, apply_translation);
		UINT8 byte1 = read_byte(space, address + 1, apply_translation);

		/* based on the endianness, the result is assembled differently */
		if (space.endianness() == ENDIANNESS_LITTLE)
			result = byte0 | (byte1 << 8);
		else
			result = byte1 | (byte0 << 8);
	}
	else
	{   /* otherwise, this proceeds like the byte case */
		device_memory_interface &memory = space.device().memory();

		/* all accesses from this point on are for the debugger */
		m_debugger_access = true;
		space.set_debugger_access(true);

		/* translate if necessary; if not mapped, return 0xffff */
		UINT64 custom;
		if (apply_translation && !memory.translate(space.spacenum(), TRANSLATE_READ_DEBUG, address))
		{
			result = 0xffff;
		}
		else if (memory.read(space.spacenum(), address, 2, custom))
		{   /* if there is a custom read handler, and it returns true, use that value */
			result = custom;
		}
		else
		{   /* otherwise, call the byte reading function for the translated address */
			result = space.read_word(address);
		}

		/* no longer accessing via the debugger */
		m_debugger_access = false;
		space.set_debugger_access(false);
	}

	return result;
}


/*-------------------------------------------------
    read_dword - return a dword from the specified
    memory space
-------------------------------------------------*/

UINT32 debugger_cpu::read_dword(address_space &space, offs_t address, int apply_translation)
{
	/* mask against the logical byte mask */
	address &= space.logbytemask();

	UINT32 result;
	if (!DWORD_ALIGNED(address))
	{   /* if this is a misaligned read, or if there are no dword readers, just read two words */
		UINT16 word0 = read_word(space, address + 0, apply_translation);
		UINT16 word1 = read_word(space, address + 2, apply_translation);

		/* based on the endianness, the result is assembled differently */
		if (space.endianness() == ENDIANNESS_LITTLE)
			result = word0 | (word1 << 16);
		else
			result = word1 | (word0 << 16);
	}
	else
	{   /* otherwise, this proceeds like the byte case */
		device_memory_interface &memory = space.device().memory();

		/* all accesses from this point on are for the debugger */
		m_debugger_access = true;
		space.set_debugger_access(true);

		UINT64 custom;
		if (apply_translation && !memory.translate(space.spacenum(), TRANSLATE_READ_DEBUG, address))
		{   /* translate if necessary; if not mapped, return 0xffffffff */
			result = 0xffffffff;
		}
		else if (memory.read(space.spacenum(), address, 4, custom))
		{   /* if there is a custom read handler, and it returns true, use that value */
			result = custom;
		}
		else
		{   /* otherwise, call the byte reading function for the translated address */
			result = space.read_dword(address);
		}

		/* no longer accessing via the debugger */
		m_debugger_access = false;
		space.set_debugger_access(false);
	}

	return result;
}


/*-------------------------------------------------
    read_qword - return a qword from the specified
    memory space
-------------------------------------------------*/

UINT64 debugger_cpu::read_qword(address_space &space, offs_t address, int apply_translation)
{
	/* mask against the logical byte mask */
	address &= space.logbytemask();

	UINT64 result;
	if (!QWORD_ALIGNED(address))
	{   /* if this is a misaligned read, or if there are no qword readers, just read two dwords */
		UINT32 dword0 = read_dword(space, address + 0, apply_translation);
		UINT32 dword1 = read_dword(space, address + 4, apply_translation);

		/* based on the endianness, the result is assembled differently */
		if (space.endianness() == ENDIANNESS_LITTLE)
			result = dword0 | ((UINT64)dword1 << 32);
		else
			result = dword1 | ((UINT64)dword0 << 32);
	}
	else
	{   /* otherwise, this proceeds like the byte case */
		device_memory_interface &memory = space.device().memory();

		/* all accesses from this point on are for the debugger */
		m_debugger_access = true;
		space.set_debugger_access(true);

		/* translate if necessary; if not mapped, return 0xffffffffffffffff */
		UINT64 custom;
		if (apply_translation && !memory.translate(space.spacenum(), TRANSLATE_READ_DEBUG, address))
		{
			result = ~(UINT64)0;
		}
		else if (memory.read(space.spacenum(), address, 8, custom))
		{   /* if there is a custom read handler, and it returns true, use that value */
			result = custom;
		}
		else
		{   /* otherwise, call the byte reading function for the translated address */
			result = space.read_qword(address);
		}

		/* no longer accessing via the debugger */
		m_debugger_access = false;
		space.set_debugger_access(false);
	}

	return result;
}


/*-------------------------------------------------
    read_memory - return 1,2,4 or 8 bytes
    from the specified memory space
-------------------------------------------------*/

UINT64 debugger_cpu::read_memory(address_space &space, offs_t address, int size, int apply_translation)
{
	UINT64 result = ~(UINT64)0 >> (64 - 8*size);
	switch (size)
	{
		case 1:     result = read_byte(space, address, apply_translation);    break;
		case 2:     result = read_word(space, address, apply_translation);    break;
		case 4:     result = read_dword(space, address, apply_translation);   break;
		case 8:     result = read_qword(space, address, apply_translation);   break;
	}
	return result;
}


/*-------------------------------------------------
    write_byte - write a byte to the specified
    memory space
-------------------------------------------------*/

void debugger_cpu::write_byte(address_space &space, offs_t address, UINT8 data, int apply_translation)
{
	device_memory_interface &memory = space.device().memory();

	/* mask against the logical byte mask */
	address &= space.logbytemask();

	/* all accesses from this point on are for the debugger */
	m_debugger_access = true;
	space.set_debugger_access(true);

	/* translate if necessary; if not mapped, we're done */
	if (apply_translation && !memory.translate(space.spacenum(), TRANSLATE_WRITE_DEBUG, address))
		;

	/* if there is a custom write handler, and it returns true, use that */
	else if (memory.write(space.spacenum(), address, 1, data))
		;

	/* otherwise, call the byte reading function for the translated address */
	else
		space.write_byte(address, data);

	/* no longer accessing via the debugger */
	m_debugger_access = false;
	space.set_debugger_access(false);

	m_memory_modified = true;
}


/*-------------------------------------------------
    write_word - write a word to the specified
    memory space
-------------------------------------------------*/

void debugger_cpu::write_word(address_space &space, offs_t address, UINT16 data, int apply_translation)
{
	/* mask against the logical byte mask */
	address &= space.logbytemask();

	/* if this is a misaligned write, or if there are no word writers, just read two bytes */
	if (!WORD_ALIGNED(address))
	{
		if (space.endianness() == ENDIANNESS_LITTLE)
		{
			write_byte(space, address + 0, data >> 0, apply_translation);
			write_byte(space, address + 1, data >> 8, apply_translation);
		}
		else
		{
			write_byte(space, address + 0, data >> 8, apply_translation);
			write_byte(space, address + 1, data >> 0, apply_translation);
		}
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		device_memory_interface &memory = space.device().memory();

		/* all accesses from this point on are for the debugger */
		m_debugger_access = true;
		space.set_debugger_access(true);

		/* translate if necessary; if not mapped, we're done */
		if (apply_translation && !memory.translate(space.spacenum(), TRANSLATE_WRITE_DEBUG, address))
			;

		/* if there is a custom write handler, and it returns true, use that */
		else if (memory.write(space.spacenum(), address, 2, data))
			;

		/* otherwise, call the byte reading function for the translated address */
		else
			space.write_word(address, data);

		/* no longer accessing via the debugger */
		m_debugger_access = false;
		space.set_debugger_access(false);

		m_memory_modified = true;
	}
}


/*-------------------------------------------------
    write_dword - write a dword to the specified
    memory space
-------------------------------------------------*/

void debugger_cpu::write_dword(address_space &space, offs_t address, UINT32 data, int apply_translation)
{
	/* mask against the logical byte mask */
	address &= space.logbytemask();

	/* if this is a misaligned write, or if there are no dword writers, just read two words */
	if (!DWORD_ALIGNED(address))
	{
		if (space.endianness() == ENDIANNESS_LITTLE)
		{
			write_word(space, address + 0, data >> 0, apply_translation);
			write_word(space, address + 2, data >> 16, apply_translation);
		}
		else
		{
			write_word(space, address + 0, data >> 16, apply_translation);
			write_word(space, address + 2, data >> 0, apply_translation);
		}
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		device_memory_interface &memory = space.device().memory();

		/* all accesses from this point on are for the debugger */
		space.set_debugger_access(m_debugger_access = true);

		/* translate if necessary; if not mapped, we're done */
		if (apply_translation && !memory.translate(space.spacenum(), TRANSLATE_WRITE_DEBUG, address))
			;

		/* if there is a custom write handler, and it returns true, use that */
		else if (memory.write(space.spacenum(), address, 4, data))
			;

		/* otherwise, call the byte reading function for the translated address */
		else
			space.write_dword(address, data);

		/* no longer accessing via the debugger */
		m_debugger_access = false;
		space.set_debugger_access(false);

		m_memory_modified = true;
	}
}


/*-------------------------------------------------
    write_qword - write a qword to the specified
    memory space
-------------------------------------------------*/

void debugger_cpu::write_qword(address_space &space, offs_t address, UINT64 data, int apply_translation)
{
	/* mask against the logical byte mask */
	address &= space.logbytemask();

	/* if this is a misaligned write, or if there are no qword writers, just read two dwords */
	if (!QWORD_ALIGNED(address))
	{
		if (space.endianness() == ENDIANNESS_LITTLE)
		{
			write_dword(space, address + 0, data >> 0, apply_translation);
			write_dword(space, address + 4, data >> 32, apply_translation);
		}
		else
		{
			write_dword(space, address + 0, data >> 32, apply_translation);
			write_dword(space, address + 4, data >> 0, apply_translation);
		}
	}

	/* otherwise, this proceeds like the byte case */
	else
	{
		device_memory_interface &memory = space.device().memory();

		/* all accesses from this point on are for the debugger */
		m_debugger_access = true;
		space.set_debugger_access(true);

		/* translate if necessary; if not mapped, we're done */
		if (apply_translation && !memory.translate(space.spacenum(), TRANSLATE_WRITE_DEBUG, address))
			;

		/* if there is a custom write handler, and it returns true, use that */
		else if (memory.write(space.spacenum(), address, 8, data))
			;

		/* otherwise, call the byte reading function for the translated address */
		else
			space.write_qword(address, data);

		/* no longer accessing via the debugger */
		m_debugger_access = false;
		space.set_debugger_access(false);

		m_memory_modified = true;
	}
}


/*-------------------------------------------------
    write_memory - write 1,2,4 or 8 bytes to the
    specified memory space
-------------------------------------------------*/

void debugger_cpu::write_memory(address_space &space, offs_t address, UINT64 data, int size, int apply_translation)
{
	switch (size)
	{
		case 1:     write_byte(space, address, data, apply_translation);  break;
		case 2:     write_word(space, address, data, apply_translation);  break;
		case 4:     write_dword(space, address, data, apply_translation); break;
		case 8:     write_qword(space, address, data, apply_translation); break;
	}
}


/*-------------------------------------------------
    read_opcode - read 1,2,4 or 8 bytes at the
    given offset from opcode space
-------------------------------------------------*/

UINT64 debugger_cpu::read_opcode(address_space &space, offs_t address, int size)
{
	device_memory_interface &memory = space.device().memory();

	UINT64 result = ~(UINT64)0 & (~(UINT64)0 >> (64 - 8*size)), result2;

	/* keep in logical range */
	address &= space.logbytemask();

	/* return early if we got the result directly */
	m_debugger_access = true;
	space.set_debugger_access(true);
	if (memory.readop(address, size, result2))
	{
		m_debugger_access = false;
		space.set_debugger_access(false);
		return result2;
	}

	/* if we're bigger than the address bus, break into smaller pieces */
	if (size > space.data_width() / 8)
	{
		int halfsize = size / 2;
		UINT64 r0 = read_opcode(space, address + 0, halfsize);
		UINT64 r1 = read_opcode(space, address + halfsize, halfsize);

		if (space.endianness() == ENDIANNESS_LITTLE)
			return r0 | (r1 << (8 * halfsize));
		else
			return r1 | (r0 << (8 * halfsize));
	}

	/* translate to physical first */
	if (!memory.translate(space.spacenum(), TRANSLATE_FETCH_DEBUG, address))
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
			fatalerror("read_opcode: unknown type = %d\n", space.data_width() / 8 * 10 + size);
	}

	/* turn on debugger access */
	if (!m_debugger_access)
	{
		m_debugger_access = true;
		space.set_debugger_access(true);
	}

	/* switch off the size and handle unaligned accesses */
	switch (size)
	{
		case 1:
			result = space.direct().read_byte(address, addrxor);
			break;

		case 2:
			result = space.direct().read_word(address & ~1, addrxor);
			if (!WORD_ALIGNED(address))
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
			if (!DWORD_ALIGNED(address))
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
			if (!QWORD_ALIGNED(address))
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
	m_debugger_access = false;
	space.set_debugger_access(false);

	return result;
}



/***************************************************************************
    INTERNAL HELPERS
***************************************************************************/

/*-------------------------------------------------
    on_vblank - called when a VBLANK hits
-------------------------------------------------*/

void debugger_cpu::on_vblank(screen_device &device, bool vblank_state)
{
	/* just set a global flag to be consumed later */
	m_vblank_occurred = true;
}


/*-------------------------------------------------
    reset_transient_flags - reset the transient
    flags on all CPUs
-------------------------------------------------*/

void debugger_cpu::reset_transient_flags()
{
	/* loop over CPUs and reset the transient flags */
	for (device_t &device : device_iterator(m_machine.root_device()))
		device.debug()->reset_transient_flag();
	m_stop_when_not_device = nullptr;
}


/*-------------------------------------------------
    process_source_file - executes commands from
    a source file
-------------------------------------------------*/

void debugger_cpu::process_source_file()
{
	/* loop until the file is exhausted or until we are executing again */
	while (m_source_file != nullptr && m_execution_state == EXECUTION_STATE_STOPPED)
	{
		/* stop at the end of file */
		if (feof(m_source_file))
		{
			fclose(m_source_file);
			m_source_file = nullptr;
			return;
		}

		/* fetch the next line */
		char buf[512];
		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf), m_source_file);

		/* strip out comments (text after '//') */
		char *s = strstr(buf, "//");
		if (s)
			*s = '\0';

		/* strip whitespace */
		int i = (int)strlen(buf);
		while((i > 0) && (isspace((UINT8)buf[i-1])))
			buf[--i] = '\0';

		/* execute the command */
		if (buf[0])
			m_machine.debugger().console().execute_command(buf, true);
	}
}



/***************************************************************************
    EXPRESSION HANDLERS
***************************************************************************/

/*-------------------------------------------------
    expression_get_device - return a device
    based on a case insensitive tag search
-------------------------------------------------*/

device_t* debugger_cpu::expression_get_device(const char *tag)
{
	// convert to lowercase then lookup the name (tags are enforced to be all lower case)
	std::string fullname(tag);
	strmakelower(fullname);
	return m_machine.device(fullname.c_str());
}


/*-------------------------------------------------
    expression_read_memory - read 1,2,4 or 8 bytes
    at the given offset in the given address
    space
-------------------------------------------------*/

UINT64 debugger_cpu::expression_read_memory(void *param, const char *name, expression_space spacenum, UINT32 address, int size)
{
	switch (spacenum)
	{
		case EXPSPACE_PROGRAM_LOGICAL:
		case EXPSPACE_DATA_LOGICAL:
		case EXPSPACE_IO_LOGICAL:
		case EXPSPACE_SPACE3_LOGICAL:
		{
			device_t *device = nullptr;
			device_memory_interface *memory;

			if (name != nullptr)
				device = expression_get_device(name);
			if (device == nullptr || !device->interface(memory))
			{
				device = m_machine.debugger().cpu().get_visible_cpu();
				memory = &device->memory();
			}
			if (memory->has_space(AS_PROGRAM + (spacenum - EXPSPACE_PROGRAM_LOGICAL)))
			{
				address_space &space = memory->space(AS_PROGRAM + (spacenum - EXPSPACE_PROGRAM_LOGICAL));
				return read_memory(space, space.address_to_byte(address), size, true);
			}
			break;
		}

		case EXPSPACE_PROGRAM_PHYSICAL:
		case EXPSPACE_DATA_PHYSICAL:
		case EXPSPACE_IO_PHYSICAL:
		case EXPSPACE_SPACE3_PHYSICAL:
		{
			device_t *device = nullptr;
			device_memory_interface *memory;

			if (name != nullptr)
				device = expression_get_device(name);
			if (device == nullptr || !device->interface(memory))
			{
				device = m_machine.debugger().cpu().get_visible_cpu();
				memory = &device->memory();
			}
			if (memory->has_space(AS_PROGRAM + (spacenum - EXPSPACE_PROGRAM_PHYSICAL)))
			{
				address_space &space = memory->space(AS_PROGRAM + (spacenum - EXPSPACE_PROGRAM_PHYSICAL));
				return read_memory(space, space.address_to_byte(address), size, false);
			}
			break;
		}

		case EXPSPACE_OPCODE:
		case EXPSPACE_RAMWRITE:
		{
			device_t *device = nullptr;
			device_memory_interface *memory;

			if (name != nullptr)
				device = expression_get_device(name);
			if (device == nullptr || !device->interface(memory))
			{
				device = m_machine.debugger().cpu().get_visible_cpu();
				memory = &device->memory();
			}
			return expression_read_program_direct(memory->space(AS_PROGRAM), (spacenum == EXPSPACE_OPCODE), address, size);
			break;
		}

		case EXPSPACE_REGION:
			if (name == nullptr)
				break;
			return expression_read_memory_region(name, address, size);
			break;

		default:
			break;
	}

	return 0;
}


/*-------------------------------------------------
    expression_read_program_direct - read memory
    directly from an opcode or RAM pointer
-------------------------------------------------*/

UINT64 debugger_cpu::expression_read_program_direct(address_space &space, int opcode, offs_t address, int size)
{
	UINT8 *base;

	/* adjust the address into a byte address, but not if being called recursively */
	if ((opcode & 2) == 0)
		address = space.address_to_byte(address);

	/* call ourself recursively until we are byte-sized */
	if (size > 1)
	{
		int halfsize = size / 2;

		/* read each half, from lower address to upper address */
		UINT64 r0 = expression_read_program_direct(space, opcode | 2, address + 0, halfsize);
		UINT64 r1 = expression_read_program_direct(space, opcode | 2, address + halfsize, halfsize);

		/* assemble based on the target endianness */
		if (space.endianness() == ENDIANNESS_LITTLE)
			return r0 | (r1 << (8 * halfsize));
		else
			return r1 | (r0 << (8 * halfsize));
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
				return base[BYTE8_XOR_LE(address) & lowmask];
			else
				return base[BYTE8_XOR_BE(address) & lowmask];
		}
	}

	return 0;
}


/*-------------------------------------------------
    expression_read_memory_region - read memory
    from a memory region
-------------------------------------------------*/

UINT64 debugger_cpu::expression_read_memory_region(const char *rgntag, offs_t address, int size)
{
	memory_region *region = m_machine.root_device().memregion(rgntag);
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
			r0 = expression_read_memory_region(rgntag, address + 0, halfsize);
			r1 = expression_read_memory_region(rgntag, address + halfsize, halfsize);

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

void debugger_cpu::expression_write_memory(void *param, const char *name, expression_space spacenum, UINT32 address, int size, UINT64 data)
{
	device_t *device = nullptr;
	device_memory_interface *memory;

	switch (spacenum)
	{
		case EXPSPACE_PROGRAM_LOGICAL:
		case EXPSPACE_DATA_LOGICAL:
		case EXPSPACE_IO_LOGICAL:
		case EXPSPACE_SPACE3_LOGICAL:
			if (name != nullptr)
				device = expression_get_device(name);
			if (device == nullptr || !device->interface(memory))
			{
				device = m_machine.debugger().cpu().get_visible_cpu();
				memory = &device->memory();
			}
			if (memory->has_space(AS_PROGRAM + (spacenum - EXPSPACE_PROGRAM_LOGICAL)))
			{
				address_space &space = memory->space(AS_PROGRAM + (spacenum - EXPSPACE_PROGRAM_LOGICAL));
				write_memory(space, space.address_to_byte(address), data, size, true);
			}
			break;

		case EXPSPACE_PROGRAM_PHYSICAL:
		case EXPSPACE_DATA_PHYSICAL:
		case EXPSPACE_IO_PHYSICAL:
		case EXPSPACE_SPACE3_PHYSICAL:
			if (name != nullptr)
				device = expression_get_device(name);
			if (device == nullptr || !device->interface(memory))
			{
				device = m_machine.debugger().cpu().get_visible_cpu();
				memory = &device->memory();
			}
			if (memory->has_space(AS_PROGRAM + (spacenum - EXPSPACE_PROGRAM_PHYSICAL)))
			{
				address_space &space = memory->space(AS_PROGRAM + (spacenum - EXPSPACE_PROGRAM_PHYSICAL));
				write_memory(space, space.address_to_byte(address), data, size, false);
			}
			break;

		case EXPSPACE_OPCODE:
		case EXPSPACE_RAMWRITE:
			if (name != nullptr)
				device = expression_get_device(name);
			if (device == nullptr || !device->interface(memory))
			{
				device = m_machine.debugger().cpu().get_visible_cpu();
				memory = &device->memory();
			}
			expression_write_program_direct(memory->space(AS_PROGRAM), (spacenum == EXPSPACE_OPCODE), address, size, data);
			break;

		case EXPSPACE_REGION:
			if (name == nullptr)
				break;
			expression_write_memory_region(name, address, size, data);
			break;

		default:
			break;
	}
}


/*-------------------------------------------------
    expression_write_program_direct - write memory
    directly to an opcode or RAM pointer
-------------------------------------------------*/

void debugger_cpu::expression_write_program_direct(address_space &space, int opcode, offs_t address, int size, UINT64 data)
{
	/* adjust the address into a byte address, but not if being called recursively */
	if ((opcode & 2) == 0)
		address = space.address_to_byte(address);

	/* call ourself recursively until we are byte-sized */
	if (size > 1)
	{
		int halfsize = size / 2;

		/* break apart based on the target endianness */
		UINT64 halfmask = ~(UINT64)0 >> (64 - 8 * halfsize);
		UINT64 r0, r1;
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
		UINT8 *base = (UINT8 *)space.get_read_ptr(address & ~lowmask);

		/* if we have a valid base, write the appropriate byte */
		if (base != nullptr)
		{
			if (space.endianness() == ENDIANNESS_LITTLE)
				base[BYTE8_XOR_LE(address) & lowmask] = data;
			else
				base[BYTE8_XOR_BE(address) & lowmask] = data;
			m_memory_modified = true;
		}
	}
}


/*-------------------------------------------------
    expression_write_memory_region - write memory
    from a memory region
-------------------------------------------------*/

void debugger_cpu::expression_write_memory_region(const char *rgntag, offs_t address, int size, UINT64 data)
{
	memory_region *region = m_machine.root_device().memregion(rgntag);

	/* make sure we get a valid base before proceeding */
	if (region != nullptr)
	{
		/* call ourself recursively until we are byte-sized */
		if (size > 1)
		{
			int halfsize = size / 2;

			/* break apart based on the target endianness */
			UINT64 halfmask = ~(UINT64)0 >> (64 - 8 * halfsize);
			UINT64 r0, r1;
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
			expression_write_memory_region(rgntag, address + 0, halfsize, r0);
			expression_write_memory_region(rgntag, address + halfsize, halfsize, r1);
		}

		/* only process if we're within range */
		else if (address < region->bytes())
		{
			/* lowmask specified which address bits are within the databus width */
			UINT32 lowmask = region->bytewidth() - 1;
			UINT8 *base = region->base() + (address & ~lowmask);

			/* if we have a valid base, set the appropriate byte */
			if (region->endianness() == ENDIANNESS_LITTLE)
			{
				base[BYTE8_XOR_LE(address) & lowmask] = data;
			}
			else
			{
				base[BYTE8_XOR_BE(address) & lowmask] = data;
			}
			m_memory_modified = true;
		}
	}
}


/*-------------------------------------------------
    expression_validate - validate that the
    provided expression references an
    appropriate name
-------------------------------------------------*/

expression_error::error_code debugger_cpu::expression_validate(void *param, const char *name, expression_space space)
{
	device_t *device = nullptr;
	device_memory_interface *memory;

	switch (space)
	{
	case EXPSPACE_PROGRAM_LOGICAL:
	case EXPSPACE_DATA_LOGICAL:
	case EXPSPACE_IO_LOGICAL:
	case EXPSPACE_SPACE3_LOGICAL:
		if (name)
		{
			device = expression_get_device(name);
			if (device == nullptr)
				return expression_error::INVALID_MEMORY_NAME;
		}
		if (!device)
			device = m_machine.debugger().cpu().get_visible_cpu();
		if (!device->interface(memory) || !memory->has_space(AS_PROGRAM + (space - EXPSPACE_PROGRAM_LOGICAL)))
			return expression_error::NO_SUCH_MEMORY_SPACE;
		break;

	case EXPSPACE_PROGRAM_PHYSICAL:
	case EXPSPACE_DATA_PHYSICAL:
	case EXPSPACE_IO_PHYSICAL:
	case EXPSPACE_SPACE3_PHYSICAL:
		if (name)
		{
			device = expression_get_device(name);
			if (device == nullptr)
				return expression_error::INVALID_MEMORY_NAME;
		}
		if (!device)
			device = m_machine.debugger().cpu().get_visible_cpu();
		if (!device->interface(memory) || !memory->has_space(AS_PROGRAM + (space - EXPSPACE_PROGRAM_PHYSICAL)))
			return expression_error::NO_SUCH_MEMORY_SPACE;
		break;

	case EXPSPACE_OPCODE:
	case EXPSPACE_RAMWRITE:
		if (name)
		{
			device = expression_get_device(name);
			if (device == nullptr || !device->interface(memory))
				return expression_error::INVALID_MEMORY_NAME;
		}
		if (!device)
			device = m_machine.debugger().cpu().get_visible_cpu();
		if (!device->interface(memory) || !memory->has_space(AS_PROGRAM))
			return expression_error::NO_SUCH_MEMORY_SPACE;
		break;

	case EXPSPACE_REGION:
		if (!name)
			return expression_error::MISSING_MEMORY_NAME;
		if (!m_machine.root_device().memregion(name) || !m_machine.root_device().memregion(name)->base())
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

UINT64 debugger_cpu::get_beamx(symbol_table &table, void *ref)
{
	screen_device *screen = reinterpret_cast<screen_device *>(ref);
	return (screen != nullptr) ? screen->hpos() : 0;
}


/*-------------------------------------------------
    get_beamy - get beam vertical position
-------------------------------------------------*/

UINT64 debugger_cpu::get_beamy(symbol_table &table, void *ref)
{
	screen_device *screen = reinterpret_cast<screen_device *>(ref);
	return (screen != nullptr) ? screen->vpos() : 0;
}


/*-------------------------------------------------
    get_frame - get current frame number
-------------------------------------------------*/

UINT64 debugger_cpu::get_frame(symbol_table &table, void *ref)
{
	screen_device *screen = reinterpret_cast<screen_device *>(ref);
	return (screen != nullptr) ? screen->frame_number() : 0;
}


/*-------------------------------------------------
    get_cpunum - getter callback for the
    'cpunum' symbol
-------------------------------------------------*/

UINT64 debugger_cpu::get_cpunum(symbol_table &table, void *ref)
{
	execute_interface_iterator iter(m_machine.root_device());
	return iter.indexof(m_visiblecpu->execute());
}


void debugger_cpu::start_hook(device_t *device, bool stop_on_vblank)
{
	// stash a pointer to the current live CPU
	assert(m_livecpu == nullptr);
	m_livecpu = device;

	// if we're a new device, stop now
	if (m_stop_when_not_device != nullptr && m_stop_when_not_device != device)
	{
		m_stop_when_not_device = nullptr;
		m_execution_state = EXECUTION_STATE_STOPPED;
		reset_transient_flags();
	}

	// if we're running, do some periodic updating
	if (m_execution_state != EXECUTION_STATE_STOPPED)
	{
		if (device == m_visiblecpu && osd_ticks() > m_last_periodic_update_time + osd_ticks_per_second() / 4)
		{   // check for periodic updates
			m_machine.debug_view().update_all();
			m_machine.debug_view().flush_osd_updates();
			m_last_periodic_update_time = osd_ticks();
		}
		else if (device == m_breakcpu)
		{   // check for pending breaks
			m_execution_state = EXECUTION_STATE_STOPPED;
			m_breakcpu = nullptr;
		}

		// if a VBLANK occurred, check on things
		if (m_vblank_occurred)
		{
			m_vblank_occurred = false;

			// if we were waiting for a VBLANK, signal it now
			if (stop_on_vblank)
			{
				m_execution_state = EXECUTION_STATE_STOPPED;
				m_machine.debugger().console().printf("Stopped at VBLANK\n");
			}
		}
		// check for debug keypresses
		if (m_machine.ui_input().pressed(IPT_UI_DEBUG_BREAK))
			m_visiblecpu->debug()->halt_on_next_instruction("User-initiated break\n");
	}
}

void debugger_cpu::stop_hook(device_t *device)
{
	assert(m_livecpu == device);

	// clear the live CPU
	m_livecpu = nullptr;
}

void debugger_cpu::ensure_comments_loaded()
{
	if (!m_comments_loaded)
	{
		comment_load(true);
		m_comments_loaded = true;
	}
}


//-------------------------------------------------
//  go_next_device - execute until we hit the next
//  device
//-------------------------------------------------

void debugger_cpu::go_next_device(device_t *device)
{
	m_stop_when_not_device = device;
	m_execution_state = EXECUTION_STATE_RUNNING;
}

void debugger_cpu::go_vblank()
{
	m_vblank_occurred = false;
	m_execution_state = EXECUTION_STATE_RUNNING;
}

void debugger_cpu::halt_on_next_instruction(device_t *device, util::format_argument_pack<std::ostream> &&args)
{
	// if something is pending on this CPU already, ignore this request
	if (device == m_breakcpu)
		return;

	// output the message to the console
	m_machine.debugger().console().vprintf(std::move(args));

	// if we are live, stop now, otherwise note that we want to break there
	if (device == m_livecpu)
	{
		m_execution_state = EXECUTION_STATE_STOPPED;
		if (m_livecpu != nullptr)
			m_livecpu->debug()->compute_debug_flags();
	}
	else
	{
		m_breakcpu = device;
	}
}

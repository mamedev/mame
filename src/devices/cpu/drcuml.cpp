// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drcuml.c

    Universal machine language for dynamic recompiling CPU cores.

****************************************************************************

    Future improvements/changes:

    * UML optimizer:
        - constant folding

    * Write a back-end validator:
        - checks all combinations of memory/register/immediate on all params
        - checks behavior of all opcodes

    * Extend registers to 16? Depends on if PPC can use them

    * Support for FPU exceptions

    * New instructions?
        - VALID opcode_desc,handle,param
            checksum/compare code referenced by opcode_desc; if not
            matching, generate exception with handle,param

        - RECALL handle
            change code at caller to call handle in the future

***************************************************************************/

#include "emu.h"
#include "drcuml.h"

#include "drcbearm64.h"
#include "drcbec.h"
#include "drcbex64.h"
#include "drcbex86.h"

#include "emuopts.h"

#include <fstream>



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_SIMPLIFICATIONS     (0)



//**************************************************************************
//  MACROS
//**************************************************************************

// determine the type of the native DRC, falling back to C
#ifndef NATIVE_DRC
#if !defined(MAME_NOASM) && (defined(__x86_64__) || defined(_M_X64))
#define NATIVE_DRC drcbe_x64
#elif !defined(MAME_NOASM) && (defined(__i386__) || defined(_M_IX86))
#define NATIVE_DRC drcbe_x86
#elif !defined(MAME_NOASM) && (defined(__aarch64__) || defined(_M_ARM64))
#define NATIVE_DRC drcbe_arm64
#else
#define NATIVE_DRC drcbe_c
#endif
#endif

#define MAKE_DRCBE_IMPL(name) make_##name
#define MAKE_DRCBE(name) MAKE_DRCBE_IMPL(name)
#define make_drcbe_native MAKE_DRCBE(NATIVE_DRC)



//**************************************************************************
//  DRC BACKEND INTERFACE
//**************************************************************************

//-------------------------------------------------
//  drcbe_interface - constructor
//-------------------------------------------------

drcbe_interface::drcbe_interface(drcuml_state &drcuml, drc_cache &cache, device_t &device)
	: m_drcuml(drcuml)
	, m_cache(cache)
	, m_device(device)
	, m_space()
	, m_state(*cache.alloc_near<drcuml_machine_state>())
{
	// reset the machine state
	memset(&m_state, 0, sizeof(m_state));

	// find the spaces and fetch memory accessors
	device_memory_interface *memory;
	if (device.interface(memory))
	{
		int const count = memory->max_space_count();
		m_space.resize(count, nullptr);

		for (int spacenum = 0; spacenum < count; ++spacenum)
		{
			if (memory->has_space(spacenum))
				m_space[spacenum] = &memory->space(spacenum);
		}
	}
}


//-------------------------------------------------
//  ~drcbe_interface - destructor
//-------------------------------------------------

drcbe_interface::~drcbe_interface()
{
}



//**************************************************************************
//  DRCUML STATE
//**************************************************************************

//-------------------------------------------------
//  drcuml_state - constructor
//-------------------------------------------------

drcuml_state::drcuml_state(device_t &device, drc_cache &cache, u32 flags, int modes, int addrbits, int ignorebits)
	: m_device(device)
	, m_cache(cache)
	, m_beintf(device.machine().options().drc_use_c()
			? drc::make_drcbe_c(*this, device, cache, flags, modes, addrbits, ignorebits)
			: drc::make_drcbe_native(*this, device, cache, flags, modes, addrbits, ignorebits))
	, m_umllog(device.machine().options().drc_log_uml()
			? new std::ofstream(util::string_format("drcuml_%s.asm", device.shortname()))
			: nullptr)
	, m_blocklist()
	, m_handlelist()
	, m_symlist()
{
}


//-------------------------------------------------
//  ~drcuml_state - destructor
//-------------------------------------------------

drcuml_state::~drcuml_state()
{
}


//-------------------------------------------------
//  reset - reset the state completely, flushing
//  the cache and all information
//-------------------------------------------------

void drcuml_state::reset()
{
	// if we error here, we are screwed
	try
	{
		// flush the cache
		m_cache.flush();

		// reset all handle code pointers
		for (uml::code_handle &handle : m_handlelist)
			*handle.codeptr_addr() = nullptr;

		// call the backend to reset
		m_beintf->reset();
	}
	catch (drcuml_block::abort_compilation &)
	{
		fatalerror("Out of cache space in drcuml_state::reset\n");
	}
}


//-------------------------------------------------
//  begin_block - begin a new code block
//-------------------------------------------------

drcuml_block &drcuml_state::begin_block(uint32_t maxinst)
{
	// find an inactive block that matches our qualifications
	drcuml_block *bestblock(nullptr);
	for (drcuml_block &block : m_blocklist)
	{
		if (!block.inuse() && (block.maxinst() >= maxinst) && (!bestblock || (block.maxinst() < bestblock->maxinst())))
			bestblock = &block;
	}

	// if we failed to find one, allocate a new one
	if (!bestblock)
		bestblock = &*m_blocklist.emplace(m_blocklist.end(), *this, maxinst * 3 / 2);

	// start the block
	bestblock->begin();
	return *bestblock;
}


//-------------------------------------------------
//  handle_alloc - allocate a new handle
//-------------------------------------------------

uml::code_handle *drcuml_state::handle_alloc(char const *name)
{
	// allocate the handle, add it to our list, and return it
	return &*m_handlelist.emplace(m_handlelist.end(), *this, name);
}


//-------------------------------------------------
//  symbol_add - add a symbol to the internal
//  symbol table
//-------------------------------------------------

void drcuml_state::symbol_add(void *base, u32 length, char const *name)
{
	m_symlist.emplace_back(base, length, name);
}


//-------------------------------------------------
//  symbol_find - look up a symbol from the
//  internal symbol table or return nullptr if not
//  found
//-------------------------------------------------

const char *drcuml_state::symbol_find(void *base, u32 *offset)
{
	drccodeptr const search(reinterpret_cast<drccodeptr>(base));

	// simple linear search
	for (symbol const &cursym : m_symlist)
	{
		// if no offset pointer, only match perfectly
		if (cursym.includes(search) && (offset || (cursym.base() == search)))
		{
			// return the offset and name
			if (offset)
				*offset = search - cursym.base();
			return cursym.name().c_str();
		}
	}

	// not found; return nullptr
	return nullptr;
}


//-------------------------------------------------
//  log_vprintf - directly printf to the UML log
//  if generated
//-------------------------------------------------

void drcuml_state::log_vprintf(util::format_argument_pack<char> const &args)
{
	// if we have a file, print to it
	if (m_umllog)
	{
		util::stream_format(*m_umllog, args);
		m_umllog->flush();
	}
}



//**************************************************************************
//  DRCUML BLOCK
//**************************************************************************

//-------------------------------------------------
//  drcuml_block - constructor
//-------------------------------------------------

drcuml_block::drcuml_block(drcuml_state &drcuml, u32 maxinst)
	: m_drcuml(drcuml)
	, m_nextinst(0)
	, m_maxinst(maxinst * 3/2)
	, m_inst(m_maxinst)
	, m_inuse(false)
{
}


//-------------------------------------------------
//  ~drcuml_block - destructor
//-------------------------------------------------

drcuml_block::~drcuml_block()
{
}


//-------------------------------------------------
//  begin - begin code generation
//-------------------------------------------------

void drcuml_block::begin()
{
	// set up the block information and return it
	m_inuse = true;
	m_nextinst = 0;
}


//-------------------------------------------------
//  end - complete a code block and commit it to
//  the cache via the back-end
//-------------------------------------------------

void drcuml_block::end()
{
	assert(m_inuse);

	// optimize the resulting code first
	optimize();

	// if we have a logfile, generate a disassembly of the block
	if (m_drcuml.logging())
		disassemble();

	// generate the code via the back-end
	m_drcuml.generate(*this, &m_inst[0], m_nextinst);

	// block is no longer in use
	m_inuse = false;
}


//-------------------------------------------------
//  abort - abort a code block in progress
//-------------------------------------------------

void drcuml_block::abort()
{
	assert(m_inuse);

	// block is no longer in use
	m_inuse = false;

	// unwind
	throw abort_compilation();
}


//-------------------------------------------------
//  append - append an opcode to the block
//-------------------------------------------------

uml::instruction &drcuml_block::append()
{
	// get a pointer to the next instruction
	uml::instruction &curinst(m_inst[m_nextinst++]);
	if (m_nextinst > m_maxinst)
		fatalerror("Overran maxinst in drcuml_block_append\n");

	return curinst;
}


//-------------------------------------------------
//  optimize - apply various optimizations to a
//  block of code
//-------------------------------------------------

void drcuml_block::optimize()
{
	u32 mapvar[uml::MAPVAR_COUNT] = { 0 };

	// iterate over instructions
	for (int instnum = 0; instnum < m_nextinst; instnum++)
	{
		uml::instruction &inst(m_inst[instnum]);

		// first compute what flags we need
		u8 accumflags(0);
		u8 remainingflags(inst.output_flags());

		// scan ahead until we run out of possible remaining flags
		for (int scannum = instnum + 1; remainingflags != 0 && scannum < m_nextinst; scannum++)
		{
			// any input flags are required
			uml::instruction const &scan(m_inst[scannum]);
			accumflags |= scan.input_flags();

			// if the scanahead instruction is unconditional, assume his flags are modified
			if (scan.condition() == uml::COND_ALWAYS)
				remainingflags &= ~scan.modified_flags();
		}
		inst.set_flags(accumflags);

		if (inst.opcode() == uml::OP_MAPVAR)
		{
			// track mapvars
			mapvar[inst.param(0).mapvar() - uml::MAPVAR_M0] = inst.param(1).immediate();
		}
		else if (inst.opcode() != uml::OP_RECOVER)
		{
			// convert all mapvar parameters to immediates
			for (int pnum = 0; pnum < inst.numparams(); pnum++)
			{
				if (inst.param(pnum).is_mapvar())
					inst.set_mapvar(pnum, mapvar[inst.param(pnum).mapvar() - uml::MAPVAR_M0]);
			}
		}

		// now that flags are correct, simplify the instruction
#if LOG_SIMPLIFICATIONS
		uml::instruction const orig = inst;
#endif
		inst.simplify();
#if LOG_SIMPLIFICATIONS
		if (orig != inst)
			osd_printf_debug("Simplified: %-50.50s -> %s\n", orig.disasm(&m_drcuml), inst.disasm(&m_drcuml));
#endif
	}
}


//-------------------------------------------------
//  disassemble - disassemble a block of
//  instructions to the log
//-------------------------------------------------

void drcuml_block::disassemble()
{
	std::string comment;

	// iterate over instructions and output
	int firstcomment(-1);
	for (int instnum = 0; instnum < m_nextinst; instnum++)
	{
		uml::instruction const &inst(m_inst[instnum]);
		bool flushcomments(false);

		// remember comments and mapvars for later
		if (inst.opcode() == uml::OP_COMMENT || inst.opcode() == uml::OP_MAPVAR)
		{
			if (firstcomment == -1)
				firstcomment = instnum;
		}

		// print labels, handles, and hashes left justified
		else if (inst.opcode() == uml::OP_LABEL)
			m_drcuml.log_printf("$%X:\n", u32(inst.param(0).label()));
		else if (inst.opcode() == uml::OP_HANDLE)
			m_drcuml.log_printf("%s:\n", inst.param(0).handle().string());
		else if (inst.opcode() == uml::OP_HASH)
			m_drcuml.log_printf("(%X,%X):\n", u32(inst.param(0).immediate()), u32(inst.param(1).immediate()));

		// indent everything else with a tab
		else
		{
			std::string const dasm(m_inst[instnum].disasm(&m_drcuml));

			// include the first accumulated comment with this line
			if (firstcomment != -1)
			{
				m_drcuml.log_printf("\t%-50.50s; %s\n", dasm, get_comment_text(m_inst[firstcomment], comment));
				firstcomment++;
				flushcomments = true;
			}
			else
			{
				m_drcuml.log_printf("\t%s\n", dasm);
			}
		}

		// flush any comments pending
		if (firstcomment != -1 && (flushcomments || instnum == m_nextinst - 1))
		{
			while (firstcomment <= instnum)
			{
				char const *const text(get_comment_text(m_inst[firstcomment++], comment));
				if (text)
					m_drcuml.log_printf("\t%50s; %s\n", "", text);
			}
			firstcomment = -1;
		}
	}
	m_drcuml.log_printf("\n\n");
	m_drcuml.log_flush();
}


//-------------------------------------------------
//  get_comment_text - determine the text
//  associated with a comment or mapvar
//-------------------------------------------------

char const *drcuml_block::get_comment_text(uml::instruction const &inst, std::string &comment)
{
	if (inst.opcode() == uml::OP_COMMENT)
	{
		// comments return their strings
		return comment.assign(inst.param(0).string()).c_str();
	}
	else if (inst.opcode() == uml::OP_MAPVAR)
	{
		// mapvars comment about their values
		comment = string_format("m%d = $%X", int(inst.param(0).mapvar() - uml::MAPVAR_M0), u32(inst.param(1).immediate()));
		return comment.c_str();
	}
	else
	{
		// everything else is nullptr
		return nullptr;
	}
}

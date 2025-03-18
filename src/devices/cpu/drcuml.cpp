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

#include "emuopts.h"
#include "drcbec.h"
#ifdef NATIVE_DRC
#ifndef ASMJIT_NO_X86
#include "drcbex86.h"
#include "drcbex64.h"
#endif
#include "drcbearm64.h"
#endif

#include <fstream>



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VALIDATE_BACKEND        (0)
#define LOG_SIMPLIFICATIONS     (0)



//**************************************************************************
//  MACROS
//**************************************************************************

// determine the type of the native DRC, falling back to C
#ifndef NATIVE_DRC
#define NATIVE_DRC drcbe_c
#endif
#define MAKE_DRCBE_IMPL(name) make_##name
#define MAKE_DRCBE(name) MAKE_DRCBE_IMPL(name)
#define make_drcbe_native MAKE_DRCBE(NATIVE_DRC)


// structure describing back-end validation test
struct bevalidate_test
{
	uml::opcode_t   opcode;
	u8              size;
	u8              iflags;
	u8              flags;
	u64             param[4];
};



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
	, m_state(*reinterpret_cast<drcuml_machine_state *>(cache.alloc_near(sizeof(m_state))))
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

		// do a one-time validation if requested
#if 0
		if (VALIDATE_BACKEND)
		{
			static bool validated = false;
			if (!validated)
			{
				validated = true;
				validate_backend(this);
			}
		}
#endif
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
	m_drcuml.cache().codegen_init();
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

		// track mapvars
		if (inst.opcode() == uml::OP_MAPVAR)
			mapvar[inst.param(0).mapvar() - uml::MAPVAR_M0] = inst.param(1).immediate();

		// convert all mapvar parameters to immediates
		else if (inst.opcode() != uml::OP_RECOVER)
			for (int pnum = 0; pnum < inst.numparams(); pnum++)
				if (inst.param(pnum).is_mapvar())
					inst.set_mapvar(pnum, mapvar[inst.param(pnum).mapvar() - uml::MAPVAR_M0]);

		// now that flags are correct, simplify the instruction
		inst.simplify();
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



#if 0

/***************************************************************************
    BACK-END VALIDATION
***************************************************************************/

//-------------------------------------------------
//  effective_test_psize - return the effective
//  parameter size based on the size and fixed
//  array of parameter values
//-------------------------------------------------

inline uint8_t effective_test_psize(const opcode_info &opinfo, int pnum, int instsize, const uint64_t *params)
{
	switch (opinfo.param[pnum].size)
	{
		case PSIZE_4:   return 4;
		case PSIZE_8:   return 8;
		case PSIZE_OP:  return instsize;
		case PSIZE_P1:  return 1 << (params[0] & 3);
		case PSIZE_P2:  return 1 << (params[1] & 3);
		case PSIZE_P3:  return 1 << (params[2] & 3);
		case PSIZE_P4:  return 1 << (params[3] & 3);
	}
	return instsize;
}

#define TEST_ENTRY_2(op, size, p1, p2, flags) { OP_##op, size, 0, flags, { u64(p1), u64(p2) } },
#define TEST_ENTRY_2F(op, size, p1, p2, iflags, flags) { OP_##op, size, iflags, flags, { u64(p1), u64(p2) } },
#define TEST_ENTRY_3(op, size, p1, p2, p3, flags) { OP_##op, size, 0, flags, { u64(p1), u64(p2), u64(p3) } },
#define TEST_ENTRY_3F(op, size, p1, p2, p3, iflags, flags) { OP_##op, size, iflags, flags, { u64(p1), u64(p2), u64(p3) } },
#define TEST_ENTRY_4(op, size, p1, p2, p3, p4, flags) { OP_##op, size, 0, flags, { u64(p1), u64(p2), u64(p3), u64(p4) } },
#define TEST_ENTRY_4F(op, size, p1, p2, p3, p4, iflags, flags) { OP_##op, size, iflags, flags, { u64(p1), u64(p2), u64(p3), u64(p4) } },

static const bevalidate_test bevalidate_test_list[] =
{
	TEST_ENTRY_3(ADD, 4, 0x7fffffff, 0x12345678, 0x6dcba987, 0)
	TEST_ENTRY_3(ADD, 4, 0x80000000, 0x12345678, 0x6dcba988, FLAG_V | FLAG_S)
	TEST_ENTRY_3(ADD, 4, 0xffffffff, 0x92345678, 0x6dcba987, FLAG_S)
	TEST_ENTRY_3(ADD, 4, 0x00000000, 0x92345678, 0x6dcba988, FLAG_C | FLAG_Z)

	TEST_ENTRY_3(ADD, 8, 0x7fffffffffffffff, 0x0123456789abcdef, 0x7edcba9876543210, 0)
	TEST_ENTRY_3(ADD, 8, 0x8000000000000000, 0x0123456789abcdef, 0x7edcba9876543211, FLAG_V | FLAG_S)
	TEST_ENTRY_3(ADD, 8, 0xffffffffffffffff, 0x8123456789abcdef, 0x7edcba9876543210, FLAG_S)
	TEST_ENTRY_3(ADD, 8, 0x0000000000000000, 0x8123456789abcdef, 0x7edcba9876543211, FLAG_C | FLAG_Z)

	TEST_ENTRY_3F(ADDC, 4, 0x7fffffff, 0x12345678, 0x6dcba987, 0,       0)
	TEST_ENTRY_3F(ADDC, 4, 0x7fffffff, 0x12345678, 0x6dcba986, FLAG_C, 0)
	TEST_ENTRY_3F(ADDC, 4, 0x80000000, 0x12345678, 0x6dcba988, 0,             FLAG_V | FLAG_S)
	TEST_ENTRY_3F(ADDC, 4, 0x80000000, 0x12345678, 0x6dcba987, FLAG_C, FLAG_V | FLAG_S)
	TEST_ENTRY_3F(ADDC, 4, 0xffffffff, 0x92345678, 0x6dcba987, 0,             FLAG_S)
	TEST_ENTRY_3F(ADDC, 4, 0xffffffff, 0x92345678, 0x6dcba986, FLAG_C, FLAG_S)
	TEST_ENTRY_3F(ADDC, 4, 0x00000000, 0x92345678, 0x6dcba988, 0,             FLAG_C | FLAG_Z)
	TEST_ENTRY_3F(ADDC, 4, 0x00000000, 0x92345678, 0x6dcba987, FLAG_C, FLAG_C | FLAG_Z)
	TEST_ENTRY_3F(ADDC, 4, 0x12345678, 0x12345678, 0xffffffff, FLAG_C, FLAG_C)

	TEST_ENTRY_3F(ADDC, 8, 0x7fffffffffffffff, 0x0123456789abcdef, 0x7edcba9876543210, 0,             0)
	TEST_ENTRY_3F(ADDC, 8, 0x7fffffffffffffff, 0x0123456789abcdef, 0x7edcba987654320f, FLAG_C, 0)
	TEST_ENTRY_3F(ADDC, 8, 0x8000000000000000, 0x0123456789abcdef, 0x7edcba9876543211, 0,             FLAG_V | FLAG_S)
	TEST_ENTRY_3F(ADDC, 8, 0x8000000000000000, 0x0123456789abcdef, 0x7edcba9876543210, FLAG_C, FLAG_V | FLAG_S)
	TEST_ENTRY_3F(ADDC, 8, 0xffffffffffffffff, 0x8123456789abcdef, 0x7edcba9876543210, 0,             FLAG_S)
	TEST_ENTRY_3F(ADDC, 8, 0xffffffffffffffff, 0x8123456789abcdef, 0x7edcba987654320f, FLAG_C, FLAG_S)
	TEST_ENTRY_3F(ADDC, 8, 0x0000000000000000, 0x8123456789abcdef, 0x7edcba9876543211, 0,             FLAG_C | FLAG_Z)
	TEST_ENTRY_3F(ADDC, 8, 0x0000000000000000, 0x8123456789abcdef, 0x7edcba9876543210, FLAG_C, FLAG_C | FLAG_Z)
	TEST_ENTRY_3F(ADDC, 8, 0x123456789abcdef0, 0x123456789abcdef0, 0xffffffffffffffff, FLAG_C, FLAG_C)

	TEST_ENTRY_3(SUB, 4, 0x12345678, 0x7fffffff, 0x6dcba987, 0)
	TEST_ENTRY_3(SUB, 4, 0x12345678, 0x80000000, 0x6dcba988, FLAG_V)
	TEST_ENTRY_3(SUB, 4, 0x92345678, 0xffffffff, 0x6dcba987, FLAG_S)
	TEST_ENTRY_3(SUB, 4, 0x92345678, 0x00000000, 0x6dcba988, FLAG_C | FLAG_S)
	TEST_ENTRY_3(SUB, 4, 0x00000000, 0x12345678, 0x12345678, FLAG_Z)

	TEST_ENTRY_3(SUB, 8, 0x0123456789abcdef, 0x7fffffffffffffff, 0x7edcba9876543210, 0)
	TEST_ENTRY_3(SUB, 8, 0x0123456789abcdef, 0x8000000000000000, 0x7edcba9876543211, FLAG_V)
	TEST_ENTRY_3(SUB, 8, 0x8123456789abcdef, 0xffffffffffffffff, 0x7edcba9876543210, FLAG_S)
	TEST_ENTRY_3(SUB, 8, 0x8123456789abcdef, 0x0000000000000000, 0x7edcba9876543211, FLAG_C | FLAG_S)
	TEST_ENTRY_3(SUB, 8, 0x0000000000000000, 0x0123456789abcdef, 0x0123456789abcdef, FLAG_Z)

	TEST_ENTRY_3F(SUBB, 4, 0x12345678, 0x7fffffff, 0x6dcba987, 0,             0)
	TEST_ENTRY_3F(SUBB, 4, 0x12345678, 0x7fffffff, 0x6dcba986, FLAG_C, 0)
	TEST_ENTRY_3F(SUBB, 4, 0x12345678, 0x80000000, 0x6dcba988, 0,             FLAG_V)
	TEST_ENTRY_3F(SUBB, 4, 0x12345678, 0x80000000, 0x6dcba987, FLAG_C, FLAG_V)
	TEST_ENTRY_3F(SUBB, 4, 0x92345678, 0xffffffff, 0x6dcba987, 0,             FLAG_S)
	TEST_ENTRY_3F(SUBB, 4, 0x92345678, 0xffffffff, 0x6dcba986, FLAG_C, FLAG_S)
	TEST_ENTRY_3F(SUBB, 4, 0x92345678, 0x00000000, 0x6dcba988, 0,             FLAG_C | FLAG_S)
	TEST_ENTRY_3F(SUBB, 4, 0x92345678, 0x00000000, 0x6dcba987, FLAG_C, FLAG_C | FLAG_S)
	TEST_ENTRY_3F(SUBB, 4, 0x12345678, 0x12345678, 0xffffffff, FLAG_C, FLAG_C)
	TEST_ENTRY_3F(SUBB, 4, 0x00000000, 0x12345678, 0x12345677, FLAG_C, FLAG_Z)

	TEST_ENTRY_3F(SUBB, 8, 0x0123456789abcdef, 0x7fffffffffffffff, 0x7edcba9876543210, 0,             0)
	TEST_ENTRY_3F(SUBB, 8, 0x0123456789abcdef, 0x7fffffffffffffff, 0x7edcba987654320f, FLAG_C, 0)
	TEST_ENTRY_3F(SUBB, 8, 0x0123456789abcdef, 0x8000000000000000, 0x7edcba9876543211, 0,             FLAG_V)
	TEST_ENTRY_3F(SUBB, 8, 0x0123456789abcdef, 0x8000000000000000, 0x7edcba9876543210, FLAG_C, FLAG_V)
	TEST_ENTRY_3F(SUBB, 8, 0x8123456789abcdef, 0xffffffffffffffff, 0x7edcba9876543210, 0,             FLAG_S)
	TEST_ENTRY_3F(SUBB, 8, 0x8123456789abcdef, 0xffffffffffffffff, 0x7edcba987654320f, FLAG_C, FLAG_S)
	TEST_ENTRY_3F(SUBB, 8, 0x8123456789abcdef, 0x0000000000000000, 0x7edcba9876543211, 0,             FLAG_C | FLAG_S)
	TEST_ENTRY_3F(SUBB, 8, 0x8123456789abcdef, 0x0000000000000000, 0x7edcba9876543210, FLAG_C, FLAG_C | FLAG_S)
	TEST_ENTRY_3F(SUBB, 8, 0x123456789abcdef0, 0x123456789abcdef0, 0xffffffffffffffff, FLAG_C, FLAG_C)
	TEST_ENTRY_3F(SUBB, 8, 0x0000000000000000, 0x123456789abcdef0, 0x123456789abcdeef, FLAG_C, FLAG_Z)

	TEST_ENTRY_2(CMP, 4, 0x7fffffff, 0x6dcba987, 0)
	TEST_ENTRY_2(CMP, 4, 0x80000000, 0x6dcba988, FLAG_V)
	TEST_ENTRY_2(CMP, 4, 0xffffffff, 0x6dcba987, FLAG_S)
	TEST_ENTRY_2(CMP, 4, 0x00000000, 0x6dcba988, FLAG_C | FLAG_S)
	TEST_ENTRY_2(CMP, 4, 0x12345678, 0x12345678, FLAG_Z)

	TEST_ENTRY_2(CMP, 8, 0x7fffffffffffffff, 0x7edcba9876543210, 0)
	TEST_ENTRY_2(CMP, 8, 0x8000000000000000, 0x7edcba9876543211, FLAG_V)
	TEST_ENTRY_2(CMP, 8, 0xffffffffffffffff, 0x7edcba9876543210, FLAG_S)
	TEST_ENTRY_2(CMP, 8, 0x0000000000000000, 0x7edcba9876543211, FLAG_C | FLAG_S)
	TEST_ENTRY_2(CMP, 8, 0x0123456789abcdef, 0x0123456789abcdef, FLAG_Z)

	TEST_ENTRY_4(MULU, 4, 0x77777777, 0x00000000, 0x11111111, 0x00000007, 0)
	TEST_ENTRY_4(MULU, 4, 0xffffffff, 0x00000000, 0x11111111, 0x0000000f, 0)
	TEST_ENTRY_4(MULU, 4, 0x00000000, 0x00000000, 0x11111111, 0x00000000, FLAG_Z)
	TEST_ENTRY_4(MULU, 4, 0xea61d951, 0x37c048d0, 0x77777777, 0x77777777, FLAG_V)
	TEST_ENTRY_4(MULU, 4, 0x32323233, 0xcdcdcdcc, 0xcdcdcdcd, 0xffffffff, FLAG_V | FLAG_S)

	TEST_ENTRY_4(MULU, 8, 0x7777777777777777, 0x0000000000000000, 0x1111111111111111, 0x0000000000000007, 0)
	TEST_ENTRY_4(MULU, 8, 0xffffffffffffffff, 0x0000000000000000, 0x1111111111111111, 0x000000000000000f, 0)
	TEST_ENTRY_4(MULU, 8, 0x0000000000000000, 0x0000000000000000, 0x1111111111111111, 0x0000000000000000, FLAG_Z)
	TEST_ENTRY_4(MULU, 8, 0x0c83fb72ea61d951, 0x37c048d159e26af3, 0x7777777777777777, 0x7777777777777777, FLAG_V)
	TEST_ENTRY_4(MULU, 8, 0x3232323232323233, 0xcdcdcdcdcdcdcdcc, 0xcdcdcdcdcdcdcdcd, 0xffffffffffffffff, FLAG_V | FLAG_S)

	TEST_ENTRY_4(MULS, 4, 0x77777777, 0x00000000, 0x11111111, 0x00000007, 0)
	TEST_ENTRY_4(MULS, 4, 0xffffffff, 0x00000000, 0x11111111, 0x0000000f, FLAG_V)
	TEST_ENTRY_4(MULS, 4, 0x00000000, 0x00000000, 0x11111111, 0x00000000, FLAG_Z)
	TEST_ENTRY_4(MULS, 4, 0x9e26af38, 0xc83fb72e, 0x77777777, 0x88888888, FLAG_V | FLAG_S)
	TEST_ENTRY_4(MULS, 4, 0x32323233, 0x00000000, 0xcdcdcdcd, 0xffffffff, 0)

	TEST_ENTRY_4(MULS, 8, 0x7777777777777777, 0x0000000000000000, 0x1111111111111111, 0x0000000000000007, 0)
	TEST_ENTRY_4(MULS, 8, 0xffffffffffffffff, 0x0000000000000000, 0x1111111111111111, 0x000000000000000f, FLAG_V)
	TEST_ENTRY_4(MULS, 8, 0x0000000000000000, 0x0000000000000000, 0x1111111111111111, 0x0000000000000000, FLAG_Z)
	TEST_ENTRY_4(MULS, 8, 0x7c048d159e26af38, 0xc83fb72ea61d950c, 0x7777777777777777, 0x8888888888888888, FLAG_V | FLAG_S)
	TEST_ENTRY_4(MULS, 8, 0x3232323232323233, 0x0000000000000000, 0xcdcdcdcdcdcdcdcd, 0xffffffffffffffff, 0)

	TEST_ENTRY_4(DIVU, 4, 0x02702702, 0x00000003, 0x11111111, 0x00000007, 0)
	TEST_ENTRY_4(DIVU, 4, 0x00000000, 0x11111111, 0x11111111, 0x11111112, FLAG_Z)
	TEST_ENTRY_4(DIVU, 4, 0x7fffffff, 0x00000000, 0xfffffffe, 0x00000002, 0)
	TEST_ENTRY_4(DIVU, 4, 0xfffffffe, 0x00000000, 0xfffffffe, 0x00000001, FLAG_S)
	TEST_ENTRY_4(DIVU, 4, UNDEFINED,  UNDEFINED,  0xffffffff, 0x00000000, FLAG_V)

	TEST_ENTRY_4(DIVU, 8, 0x0270270270270270, 0x0000000000000001, 0x1111111111111111, 0x0000000000000007, 0)
	TEST_ENTRY_4(DIVU, 8, 0x0000000000000000, 0x1111111111111111, 0x1111111111111111, 0x1111111111111112, FLAG_Z)
	TEST_ENTRY_4(DIVU, 8, 0x7fffffffffffffff, 0x0000000000000000, 0xfffffffffffffffe, 0x0000000000000002, 0)
	TEST_ENTRY_4(DIVU, 8, 0xfffffffffffffffe, 0x0000000000000000, 0xfffffffffffffffe, 0x0000000000000001, FLAG_S)
	TEST_ENTRY_4(DIVU, 8, UNDEFINED,          UNDEFINED,          0xffffffffffffffff, 0x0000000000000000, FLAG_V)

	TEST_ENTRY_4(DIVS, 4, 0x02702702, 0x00000003, 0x11111111, 0x00000007, 0)
	TEST_ENTRY_4(DIVS, 4, 0x00000000, 0x11111111, 0x11111111, 0x11111112, FLAG_Z)
	TEST_ENTRY_4(DIVS, 4, 0xffffffff, 0x00000000, 0xfffffffe, 0x00000002, FLAG_S)
	TEST_ENTRY_4(DIVS, 4, UNDEFINED,  UNDEFINED,  0xffffffff, 0x00000000, FLAG_V)

	TEST_ENTRY_4(DIVS, 8, 0x0270270270270270, 0x0000000000000001, 0x1111111111111111, 0x0000000000000007, 0)
	TEST_ENTRY_4(DIVS, 8, 0x0000000000000000, 0x1111111111111111, 0x1111111111111111, 0x1111111111111112, FLAG_Z)
	TEST_ENTRY_4(DIVS, 8, 0xffffffffffffffff, 0x0000000000000000, 0xfffffffffffffffe, 0x0000000000000002, FLAG_S)
	TEST_ENTRY_4(DIVS, 8, UNDEFINED,          UNDEFINED,          0xffffffffffffffff, 0x0000000000000000, FLAG_V)
};


/*-------------------------------------------------
    validate_backend - execute a number of
    generic tests on the backend code generator
-------------------------------------------------*/

static void validate_backend(drcuml_state *drcuml)
{
	uml::code_handle *handles[3];
	int tnum;

	// allocate handles for the code
	handles[0] = drcuml->handle_alloc("test_entry");
	handles[1] = drcuml->handle_alloc("code_start");
	handles[2] = drcuml->handle_alloc("code_end");

	// iterate over test entries
	printf("Backend validation....\n");
	for (tnum = 31; tnum < std::size(bevalidate_test_list); tnum++)
	{
		const bevalidate_test *test = &bevalidate_test_list[tnum];
		parameter param[std::size(test->param)];
		char mnemonic[20], *dst;
		const char *src;

		// progress
		dst = mnemonic;
		for (src = opcode_info_table[test->opcode()]->mnemonic; *src != 0; src++)
		{
			if (*src == '!')
			{
				if (test->size == 8)
					*dst++ = 'd';
			}
			else if (*src == '#')
				*dst++ = (test->size == 8) ? 'd' : 's';
			else
				*dst++ = *src;
		}
		*dst = 0;
		printf("Executing test %d/%d (%s)", tnum + 1, (int)std::size(bevalidate_test_list), mnemonic);

		// reset parameter list and iterate
		memset(param, 0, sizeof(param));
		bevalidate_iterate_over_params(drcuml, handles, test, param, 0);
		printf("\n");
	}
	fatalerror("All tests passed!\n");
}


/*-------------------------------------------------
    bevalidate_iterate_over_params - iterate over
    all supported types and values of a parameter
    and recursively hand off to the next parameter,
    or else move on to iterate over the flags
-------------------------------------------------*/

static void bevalidate_iterate_over_params(drcuml_state *drcuml, uml::code_handle **handles, const bevalidate_test *test, parameter *paramlist, int pnum)
{
	const opcode_info *opinfo = opcode_info_table[test->opcode()];
	drcuml_ptype ptype;

	// if no parameters, execute now
	if (pnum >= std::size(opinfo->param) || opinfo->param[pnum].typemask == PTYPES_NONE)
	{
		bevalidate_iterate_over_flags(drcuml, handles, test, paramlist);
		return;
	}

	// iterate over valid parameter types
	for (ptype = parameter::PTYPE_IMMEDIATE; ptype < parameter::PTYPE_MAX; ptype++)
		if (opinfo->param[pnum].typemask & (1 << ptype))
		{
			int pindex, pcount;

			// mapvars can only do 32-bit tests
			if (ptype == parameter::PTYPE_MAPVAR && effective_test_psize(opinfo, pnum, test->size, test->param) == 8)
				continue;

			// for some parameter types, we wish to iterate over all possibilities
			switch (ptype)
			{
				case parameter::PTYPE_INT_REGISTER:     pcount = REG_I_END - REG_I0;        break;
				case parameter::PTYPE_FLOAT_REGISTER:   pcount = REG_F_END - REG_F0;        break;
				default:                            pcount = 1;                                     break;
			}

			// iterate over possibilities
			for (pindex = 0; pindex < pcount; pindex++)
			{
				bool skip = false;
				int pscannum;

				// for param 0, print a dot
				if (pnum == 0)
					printf(".");

				// can't duplicate multiple source parameters unless they are immediates
				if (ptype != parameter::PTYPE_IMMEDIATE && (opinfo->param[pnum].output & PIO_IN))

					// loop over all parameters we've done before; if the parameter is a source and matches us, skip this case
					for (pscannum = 0; pscannum < pnum; pscannum++)
						if ((opinfo->param[pscannum].output & PIO_IN) && ptype == paramlist[pscannum].type && pindex == paramlist[pscannum].value)
							skip = true;

				// can't duplicate multiple dest parameters
				if (opinfo->param[pnum].output & PIO_OUT)

					// loop over all parameters we've done before; if the parameter is a source and matches us, skip this case
					for (pscannum = 0; pscannum < pnum; pscannum++)
						if ((opinfo->param[pscannum].output & PIO_OUT) && ptype == paramlist[pscannum].type && pindex == paramlist[pscannum].value)
							skip = true;

				// iterate over the next parameter in line
				if (!skip)
				{
					paramlist[pnum].type = ptype;
					paramlist[pnum].value = pindex;
					bevalidate_iterate_over_params(drcuml, handles, test, paramlist, pnum + 1);
				}
			}
		}
}


/*-------------------------------------------------
    bevalidate_iterate_over_flags - iterate over
    all supported flag masks
-------------------------------------------------*/

static void bevalidate_iterate_over_flags(drcuml_state *drcuml, uml::code_handle **handles, const bevalidate_test *test, parameter *paramlist)
{
	const opcode_info *opinfo = opcode_info_table[test->opcode()];
	uint8_t flagmask = opinfo->outflags;
	uint8_t curmask;

	// iterate over all possible flag combinations
	for (curmask = 0; curmask <= flagmask; curmask++)
		if ((curmask & flagmask) == curmask)
			bevalidate_execute(drcuml, handles, test, paramlist, curmask);
}


/*-------------------------------------------------
    bevalidate_execute - execute a single instance
    of a test, generating code and verifying the
    results
-------------------------------------------------*/

static void bevalidate_execute(drcuml_state *drcuml, uml::code_handle **handles, const bevalidate_test *test, const parameter *paramlist, uint8_t flagmask)
{
	parameter params[std::size(test->param)];
	drcuml_machine_state istate, fstate;
	instruction testinst;
	drcuml_block *block;
	uint64_t *parammem;
	int numparams;

	// allocate memory for parameters
	parammem = (uint64_t *)drcuml->cache->alloc_near(sizeof(uint64_t) * (std::size(test->param) + 1));

	// flush the cache
	drcuml->reset();

	// start a new block
	block = drcuml->block_begin(30);
	UML_HANDLE(block, handles[0]);

	// set up a random initial state
	bevalidate_initialize_random_state(drcuml, block, &istate);

	// then populate the state with the parameters
	numparams = bevalidate_populate_state(block, &istate, test, paramlist, params, parammem);

	// generate the code
	UML_RESTORE(block, &istate);
	UML_HANDLE(block, handles[1]);
	switch (numparams)
	{
		case 0:
			block->append(test->opcode(), test->size);
			break;

		case 1:
			block->append(test->opcode(), test->size, params[0]);
			break;

		case 2:
			block->append(test->opcode(), test->size, params[0], params[1]);
			break;

		case 3:
			block->append(test->opcode(), test->size, params[0], params[1], params[2]);
			break;

		case 4:
			block->append(test->opcode(), test->size, params[0], params[1], params[2], params[3]);
			break;
	}
	testinst = block->inst[block->nextinst - 1];
	UML_HANDLE(block, handles[2]);
	UML_GETFLGS(block, MEM(&parammem[std::size(test->param)]), flagmask);
	UML_SAVE(block, &fstate);
	UML_EXIT(block, IMM(0));

	// end the block
	block->end();

	// execute
	drcuml->execute(*handles[0]);

	// verify the results
	bevalidate_verify_state(drcuml, &istate, &fstate, test, *(uint32_t *)&parammem[std::size(test->param)], params, &testinst, handles[1]->code, handles[2]->code, flagmask);

	// free memory
	drcuml->cache->dealloc(parammem, sizeof(uint64_t) * (std::size(test->param) + 1));
}


/*-------------------------------------------------
    bevalidate_initialize_random_state -
    initialize the machine state to randomness
-------------------------------------------------*/

static void bevalidate_initialize_random_state(drcuml_state *drcuml, drcuml_block *block, drcuml_machine_state *state)
{
	running_machine &machine = drcuml->device->machine();
	int regnum;

	// initialize core state to random values
	state->fmod = machine.rand() & 0x03;
	state->flags = machine.rand() & 0x1f;
	state->exp = machine.rand();

	// initialize integer registers to random values
	for (regnum = 0; regnum < std::size(state->r); regnum++)
	{
		state->r[regnum].w.h = machine.rand();
		state->r[regnum].w.l = machine.rand();
	}

	// initialize float registers to random values
	for (regnum = 0; regnum < std::size(state->f); regnum++)
	{
		*(uint32_t *)&state->f[regnum].s.h = machine.rand();
		*(uint32_t *)&state->f[regnum].s.l = machine.rand();
	}

	// initialize map variables to random values
	for (regnum = 0; regnum < MAPVAR_COUNT; regnum++)
		UML_MAPVAR(block, MVAR(regnum), machine.rand());
}


/*-------------------------------------------------
    bevalidate_populate_state - populate the
    machine state with the proper values prior
    to executing a test
-------------------------------------------------*/

static int bevalidate_populate_state(drcuml_block *block, drcuml_machine_state *state, const bevalidate_test *test, const parameter *paramlist, parameter *params, uint64_t *parammem)
{
	const opcode_info *opinfo = opcode_info_table[test->opcode()];
	int numparams = std::size(test->param);
	int pnum;

	// copy flags as-is
	state->flags = test->iflags;

	// iterate over parameters
	for (pnum = 0; pnum < std::size(test->param); pnum++)
	{
		int psize = effective_test_psize(opinfo, pnum, test->size, test->param);
		parameter *curparam = &params[pnum];

		// start with a copy of the parameter from the list
		*curparam = paramlist[pnum];

		// switch off the type
		switch (curparam->type)
		{
			// immediate parameters: take the value from the test entry
			case parameter::PTYPE_IMMEDIATE:
				curparam->value = test->param[pnum];
				break;

			// register parameters: set the register value in the state and set the parameter value to the register index
			case parameter::PTYPE_INT_REGISTER:
				state->r[curparam->value].d = test->param[pnum];
				curparam->value += REG_I0;
				break;

			// register parameters: set the register value in the state and set the parameter value to the register index
			case parameter::PTYPE_FLOAT_REGISTER:
				state->f[curparam->value].d = test->param[pnum];
				curparam->value += REG_F0;
				break;

			// memory parameters: set the memory value in the parameter space and set the parameter value to point to it
			case parameter::PTYPE_MEMORY:
				curparam->value = (uintptr_t)&parammem[pnum];
				if (psize == 4)
					*(uint32_t *)(uintptr_t)curparam->value = test->param[pnum];
				else
					*(uint64_t *)(uintptr_t)curparam->value = test->param[pnum];
				break;

			// map variables: issue a MAPVAR instruction to set the value and set the parameter value to the mapvar index
			case parameter::PTYPE_MAPVAR:
				UML_MAPVAR(block, MVAR(curparam->value), test->param[pnum]);
				curparam->value += MAPVAR_M0;
				break;

			// use anything else to count the number of parameters
			default:
				numparams = MIN(numparams, pnum);
				break;
		}
	}

	// return the total number of parameters
	return numparams;
}


/*-------------------------------------------------
    bevalidate_verify_state - verify the final
    state after executing a test, and report any
    discrepancies
-------------------------------------------------*/

static int bevalidate_verify_state(drcuml_state *drcuml, const drcuml_machine_state *istate, drcuml_machine_state *state, const bevalidate_test *test, uint32_t flags, const parameter *params, const instruction *testinst, drccodeptr codestart, drccodeptr codeend, uint8_t flagmask)
{
	const opcode_info *opinfo = opcode_info_table[test->opcode()];
	uint8_t ireg[REG_I_END - REG_I0] = { 0 };
	uint8_t freg[REG_F_END - REG_F0] = { 0 };
	char errorbuf[1024];
	char *errend = errorbuf;
	int pnum, regnum;

	*errend = 0;

	// check flags
	if (flags != (test->flags & flagmask))
	{
		errend += sprintf(errend, "  Flags ... result:%c%c%c%c%c  expected:%c%c%c%c%c\n",
			(flagmask & FLAG_U) ? ((flags & FLAG_U) ? 'U' : '.') : '-',
			(flagmask & FLAG_S) ? ((flags & FLAG_S) ? 'S' : '.') : '-',
			(flagmask & FLAG_Z) ? ((flags & FLAG_Z) ? 'Z' : '.') : '-',
			(flagmask & FLAG_V) ? ((flags & FLAG_V) ? 'V' : '.') : '-',
			(flagmask & FLAG_C) ? ((flags & FLAG_C) ? 'C' : '.') : '-',
			(flagmask & FLAG_U) ? ((test->flags & FLAG_U) ? 'U' : '.') : '-',
			(flagmask & FLAG_S) ? ((test->flags & FLAG_S) ? 'S' : '.') : '-',
			(flagmask & FLAG_Z) ? ((test->flags & FLAG_Z) ? 'Z' : '.') : '-',
			(flagmask & FLAG_V) ? ((test->flags & FLAG_V) ? 'V' : '.') : '-',
			(flagmask & FLAG_C) ? ((test->flags & FLAG_C) ? 'C' : '.') : '-');
	}

	// check destination parameters
	for (pnum = 0; pnum < std::size(test->param); pnum++)
		if (opinfo->param[pnum].output & PIO_OUT)
		{
			int psize = effective_test_psize(opinfo, pnum, test->size, test->param);
			uint64_t mask = u64(0xffffffffffffffff) >> (64 - 8 * psize);
			uint64_t result = 0;

			// fetch the result from the parameters
			switch (params[pnum].type)
			{
				// integer registers fetch from the state
				case parameter::PTYPE_INT_REGISTER:
					ireg[params[pnum].value - REG_I0] = 1;
					result = state->r[params[pnum].value - REG_I0].d;
					break;

				// float registers fetch from the state
				case parameter::PTYPE_FLOAT_REGISTER:
					freg[params[pnum].value - REG_I0] = 1;
					result = state->f[params[pnum].value - REG_F0].d;
					break;

				// memory registers fetch from the memory address
				case parameter::PTYPE_MEMORY:
					if (psize == 4)
						result = *(uint32_t *)(uintptr_t)params[pnum].value;
					else
						result = *(uint64_t *)(uintptr_t)params[pnum].value;
					break;

				default:
					break;
			}

			// check against the mask
			if (test->param[pnum] != UNDEFINED_U64 && (result & mask) != (test->param[pnum] & mask))
			{
				if ((uint32_t)mask == mask)
					errend += sprintf(errend, "  Parameter %d ... result:%08X  expected:%08X\n", pnum,
										(uint32_t)(result & mask), (uint32_t)(test->param[pnum] & mask));
				else
					errend += sprintf(errend, "  Parameter %d ... result:%08X%08X  expected:%08X%08X\n", pnum,
										(uint32_t)((result & mask) >> 32), (uint32_t)(result & mask),
										(uint32_t)((test->param[pnum] & mask) >> 32), (uint32_t)(test->param[pnum] & mask));
			}
		}

	// check source integer parameters for unexpected alterations
	for (regnum = 0; regnum < std::size(state->r); regnum++)
		if (ireg[regnum] == 0 && istate->r[regnum].d != state->r[regnum].d)
			errend += sprintf(errend, "  Register i%d ... result:%08X%08X  originally:%08X%08X\n", regnum,
								(uint32_t)(state->r[regnum].d >> 32), (uint32_t)state->r[regnum].d,
								(uint32_t)(istate->r[regnum].d >> 32), (uint32_t)istate->r[regnum].d);

	// check source float parameters for unexpected alterations
	for (regnum = 0; regnum < std::size(state->f); regnum++)
		if (freg[regnum] == 0 && *(uint64_t *)&istate->f[regnum].d != *(uint64_t *)&state->f[regnum].d)
			errend += sprintf(errend, "  Register f%d ... result:%08X%08X  originally:%08X%08X\n", regnum,
								(uint32_t)(*(uint64_t *)&state->f[regnum].d >> 32), (uint32_t)*(uint64_t *)&state->f[regnum].d,
								(uint32_t)(*(uint64_t *)&istate->f[regnum].d >> 32), (uint32_t)*(uint64_t *)&istate->f[regnum].d);

	// output the error if we have one
	if (errend != errorbuf)
	{
		// disassemble the test instruction
		std::string disasm = testinst->disasm(drcuml);

		// output a description of what went wrong
		printf("\n");
		printf("----------------------------------------------\n");
		printf("Backend validation error:\n");
		printf("   %s\n", disasm.c_str());
		printf("\n");
		printf("Errors:\n");
		printf("%s\n", errorbuf);
		fatalerror("Error during validation\n");
	}
	return errend != errorbuf;
}

#endif

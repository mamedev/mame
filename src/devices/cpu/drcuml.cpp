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
        - FCOPYI, ICOPYF
            copy raw between float and integer registers

        - VALID opcode_desc,handle,param
            checksum/compare code referenced by opcode_desc; if not
            matching, generate exception with handle,param

        - RECALL handle
            change code at caller to call handle in the future

***************************************************************************/

#include "emu.h"
#include "drcuml.h"
#include "drcbec.h"
#include "drcbex86.h"
#include "drcbex64.h"

using namespace uml;



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VALIDATE_BACKEND        (0)
#define LOG_SIMPLIFICATIONS     (0)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// structure describing back-end validation test
struct bevalidate_test
{
	opcode_t                opcode;
	UINT8                   size;
	UINT8                   iflags;
	UINT8                   flags;
	UINT64                  param[4];
};



//**************************************************************************
//  DRC BACKEND INTERFACE
//**************************************************************************

//-------------------------------------------------
//  drcbe_interface - constructor
//-------------------------------------------------

drcbe_interface::drcbe_interface(drcuml_state &drcuml, drc_cache &cache, device_t &device)
	: m_drcuml(drcuml),
		m_cache(cache),
		m_device(device),
		m_state(*(drcuml_machine_state *)cache.alloc_near(sizeof(m_state))),
		m_accessors((data_accessors *)cache.alloc_near(sizeof(*m_accessors) * ADDRESS_SPACES))
{
	// reset the machine state
	memset(m_accessors, 0, sizeof(*m_accessors) * ADDRESS_SPACES);
	memset(&m_state, 0, sizeof(m_state));

	// find the spaces and fetch memory accessors
	device_memory_interface *memory;
	if (device.interface(memory))
		for (address_spacenum spacenum = AS_0; spacenum < ARRAY_LENGTH(m_space); ++spacenum)
			if (memory->has_space(spacenum))
			{
				m_space[spacenum] = &memory->space(spacenum);
				m_space[spacenum]->accessors(m_accessors[spacenum]);
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

drcuml_state::drcuml_state(device_t &device, drc_cache &cache, UINT32 flags, int modes, int addrbits, int ignorebits)
	: m_device(device),
		m_cache(cache),
		m_drcbe_interface(device.machine().options().drc_use_c() ?
			std::unique_ptr<drcbe_interface>{ std::make_unique<drcbe_c>(*this, device, cache, flags, modes, addrbits, ignorebits) } :
			std::unique_ptr<drcbe_interface>{ std::make_unique<drcbe_native>(*this, device, cache, flags, modes, addrbits, ignorebits) }),
		m_beintf(*m_drcbe_interface.get()),
		m_umllog(nullptr)
{
	// if we're to log, create the logfile
	if (device.machine().options().drc_log_uml())
	{
		std::string filename = std::string("drcuml_").append(m_device.shortname()).append(".asm");
		m_umllog = fopen(filename.c_str(), "w");
	}
}


//-------------------------------------------------
//  ~drcuml_state - destructor
//-------------------------------------------------

drcuml_state::~drcuml_state()
{
	// close any files
	if (m_umllog != nullptr)
		fclose(m_umllog);
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
		for (code_handle *handle = m_handlelist.first(); handle != nullptr; handle = handle->next())
			*handle->m_code = nullptr;

		// call the backend to reset
		m_beintf.reset();

		// do a one-time validation if requested
/*      if (VALIDATE_BACKEND)
        {
            static bool validated = false;
            if (!validated)
            {
                validated = true;
                validate_backend(this);
            }
        }*/
	}
	catch (drcuml_block::abort_compilation &)
	{
		fatalerror("Out of cache space in drcuml_state::reset\n");
	}
}


//-------------------------------------------------
//  begin_block - begin a new code block
//-------------------------------------------------

drcuml_block *drcuml_state::begin_block(UINT32 maxinst)
{
	// find an inactive block that matches our qualifications
	drcuml_block *bestblock = nullptr;
	for (drcuml_block *block = m_blocklist.first(); block != nullptr; block = block->next())
		if (!block->inuse() && block->maxinst() >= maxinst && (bestblock == nullptr || block->maxinst() < bestblock->maxinst()))
			bestblock = block;

	// if we failed to find one, allocate a new one
	if (bestblock == nullptr)
		bestblock = &m_blocklist.append(*global_alloc(drcuml_block(*this, maxinst * 3/2)));

	// start the block
	bestblock->begin();
	return bestblock;
}


//-------------------------------------------------
//  handle_alloc - allocate a new handle
//-------------------------------------------------

code_handle *drcuml_state::handle_alloc(const char *name)
{
	// allocate the handle, add it to our list, and return it
	return &m_handlelist.append(*global_alloc(code_handle(*this, name)));
}


//-------------------------------------------------
//  symbol_add - add a symbol to the internal
//  symbol table
//-------------------------------------------------

void drcuml_state::symbol_add(void *base, UINT32 length, const char *name)
{
	m_symlist.append(*global_alloc(symbol(base, length, name)));
}


//-------------------------------------------------
//  symbol_find - look up a symbol from the
//  internal symbol table or return NULL if not
//  found
//-------------------------------------------------

const char *drcuml_state::symbol_find(void *base, UINT32 *offset)
{
	drccodeptr search = drccodeptr(base);

	// simple linear search
	for (symbol *cursym = m_symlist.first(); cursym != nullptr; cursym = cursym->next())
		if (search >= cursym->m_base && search < cursym->m_base + cursym->m_length)
		{
			// if no offset pointer, only match perfectly
			if (offset == nullptr && search != cursym->m_base)
				continue;

			// return the offset and name
			if (offset != nullptr)
				*offset = search - cursym->m_base;
			return cursym->m_name.c_str();
		}

	// not found; return NULL
	return nullptr;
}


//-------------------------------------------------
//  log_printf - directly printf to the UML log
//  if generated
//-------------------------------------------------

void drcuml_state::log_printf(const char *format, ...)
{
	// if we have a file, print to it
	if (m_umllog != nullptr)
	{
		va_list va;

		// do the printf
		va_start(va, format);
		vfprintf(m_umllog, format, va);
		va_end(va);
		fflush(m_umllog);
	}
}



//**************************************************************************
//  DRCUML BLOCK
//**************************************************************************

//-------------------------------------------------
//  drcuml_block - constructor
//-------------------------------------------------

drcuml_block::drcuml_block(drcuml_state &drcuml, UINT32 maxinst)
	: m_drcuml(drcuml),
		m_next(nullptr),
		m_nextinst(0),
		m_maxinst(maxinst * 3/2),
		m_inst(m_maxinst),
		m_inuse(false)
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
	instruction &curinst = m_inst[m_nextinst++];
	if (m_nextinst > m_maxinst)
		fatalerror("Overran maxinst in drcuml_block_append\n");

	return curinst;
}


//-------------------------------------------------
//  comment - attach a comment to the current
//  output location in the specified block
//-------------------------------------------------

void drcuml_block::append_comment(const char *format, ...)
{
	// do the printf
	std::string temp;
	va_list va;
	va_start(va, format);
	strvprintf(temp,format, va);
	va_end(va);

	// allocate space in the cache to hold the comment
	char *comment = (char *)m_drcuml.cache().alloc_temporary(temp.length() + 1);
	if (comment == nullptr)
		return;
	strcpy(comment, temp.c_str());

	// add an instruction with a pointer
	append().comment(comment);
}


//-------------------------------------------------
//  optimize - apply various optimizations to a
//  block of code
//-------------------------------------------------

void drcuml_block::optimize()
{
	UINT32 mapvar[MAPVAR_COUNT] = { 0 };

	// iterate over instructions
	for (int instnum = 0; instnum < m_nextinst; instnum++)
	{
		instruction &inst = m_inst[instnum];

		// first compute what flags we need
		UINT8 accumflags = 0;
		UINT8 remainingflags = inst.output_flags();

		// scan ahead until we run out of possible remaining flags
		for (int scannum = instnum + 1; remainingflags != 0 && scannum < m_nextinst; scannum++)
		{
			// any input flags are required
			const instruction &scan = m_inst[scannum];
			accumflags |= scan.input_flags();

			// if the scanahead instruction is unconditional, assume his flags are modified
			if (scan.condition() == COND_ALWAYS)
				remainingflags &= ~scan.modified_flags();
		}
		inst.set_flags(accumflags);

		// track mapvars
		if (inst.opcode() == OP_MAPVAR)
			mapvar[inst.param(0).mapvar() - MAPVAR_M0] = inst.param(1).immediate();

		// convert all mapvar parameters to immediates
		else if (inst.opcode() != OP_RECOVER)
			for (int pnum = 0; pnum < inst.numparams(); pnum++)
				if (inst.param(pnum).is_mapvar())
					inst.set_mapvar(pnum, mapvar[inst.param(pnum).mapvar() - MAPVAR_M0]);

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
	int firstcomment = -1;
	for (int instnum = 0; instnum < m_nextinst; instnum++)
	{
		const instruction &inst = m_inst[instnum];
		bool flushcomments = false;

		// remember comments and mapvars for later
		if (inst.opcode() == OP_COMMENT || inst.opcode() == OP_MAPVAR)
		{
			if (firstcomment == -1)
				firstcomment = instnum;
		}

		// print labels, handles, and hashes left justified
		else if (inst.opcode() == OP_LABEL)
			m_drcuml.log_printf("$%X:\n", UINT32(inst.param(0).label()));
		else if (inst.opcode() == OP_HANDLE)
			m_drcuml.log_printf("%s:\n", inst.param(0).handle().string());
		else if (inst.opcode() == OP_HASH)
			m_drcuml.log_printf("(%X,%X):\n", UINT32(inst.param(0).immediate()), UINT32(inst.param(1).immediate()));

		// indent everything else with a tab
		else
		{
			std::string dasm = m_inst[instnum].disasm(&m_drcuml);

			// include the first accumulated comment with this line
			if (firstcomment != -1)
			{
				m_drcuml.log_printf("\t%-50.50s; %s\n", dasm.c_str(), get_comment_text(m_inst[firstcomment], comment));
				firstcomment++;
				flushcomments = TRUE;
			}
			else
				m_drcuml.log_printf("\t%s\n", dasm.c_str());
		}

		// flush any comments pending
		if (firstcomment != -1 && (flushcomments || instnum == m_nextinst - 1))
		{
			while (firstcomment <= instnum)
			{
				const char *text = get_comment_text(m_inst[firstcomment++], comment);
				if (text != nullptr)
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

const char *drcuml_block::get_comment_text(const instruction &inst, std::string &comment)
{
	// comments return their strings
	if (inst.opcode() == OP_COMMENT)
		return comment.assign(inst.param(0).string()).c_str();

	// mapvars comment about their values
	else if (inst.opcode() == OP_MAPVAR) {
		strprintf(comment,"m%d = $%X", (int)inst.param(0).mapvar() - MAPVAR_M0, (UINT32)inst.param(1).immediate());
		return comment.c_str();
	}

	// everything else is NULL
	return nullptr;
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

inline UINT8 effective_test_psize(const opcode_info &opinfo, int pnum, int instsize, const UINT64 *params)
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

#define TEST_ENTRY_2(op, size, p1, p2, flags) { OP_##op, size, 0, flags, { U64(p1), U64(p2) } },
#define TEST_ENTRY_2F(op, size, p1, p2, iflags, flags) { OP_##op, size, iflags, flags, { U64(p1), U64(p2) } },
#define TEST_ENTRY_3(op, size, p1, p2, p3, flags) { OP_##op, size, 0, flags, { U64(p1), U64(p2), U64(p3) } },
#define TEST_ENTRY_3F(op, size, p1, p2, p3, iflags, flags) { OP_##op, size, iflags, flags, { U64(p1), U64(p2), U64(p3) } },
#define TEST_ENTRY_4(op, size, p1, p2, p3, p4, flags) { OP_##op, size, 0, flags, { U64(p1), U64(p2), U64(p3), U64(p4) } },
#define TEST_ENTRY_4F(op, size, p1, p2, p3, p4, iflags, flags) { OP_##op, size, iflags, flags, { U64(p1), U64(p2), U64(p3), U64(p4) } },

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
	code_handle *handles[3];
	int tnum;

	// allocate handles for the code
	handles[0] = drcuml->handle_alloc("test_entry");
	handles[1] = drcuml->handle_alloc("code_start");
	handles[2] = drcuml->handle_alloc("code_end");

	// iterate over test entries
	printf("Backend validation....\n");
	for (tnum = 31; tnum < ARRAY_LENGTH(bevalidate_test_list); tnum++)
	{
		const bevalidate_test *test = &bevalidate_test_list[tnum];
		parameter param[ARRAY_LENGTH(test->param)];
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
		printf("Executing test %d/%d (%s)", tnum + 1, (int)ARRAY_LENGTH(bevalidate_test_list), mnemonic);

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

static void bevalidate_iterate_over_params(drcuml_state *drcuml, code_handle **handles, const bevalidate_test *test, parameter *paramlist, int pnum)
{
	const opcode_info *opinfo = opcode_info_table[test->opcode()];
	drcuml_ptype ptype;

	// if no parameters, execute now
	if (pnum >= ARRAY_LENGTH(opinfo->param) || opinfo->param[pnum].typemask == PTYPES_NONE)
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
				int skip = FALSE;
				int pscannum;

				// for param 0, print a dot
				if (pnum == 0)
					printf(".");

				// can't duplicate multiple source parameters unless they are immediates
				if (ptype != parameter::PTYPE_IMMEDIATE && (opinfo->param[pnum].output & PIO_IN))

					// loop over all parameters we've done before; if the parameter is a source and matches us, skip this case
					for (pscannum = 0; pscannum < pnum; pscannum++)
						if ((opinfo->param[pscannum].output & PIO_IN) && ptype == paramlist[pscannum].type && pindex == paramlist[pscannum].value)
							skip = TRUE;

				// can't duplicate multiple dest parameters
				if (opinfo->param[pnum].output & PIO_OUT)

					// loop over all parameters we've done before; if the parameter is a source and matches us, skip this case
					for (pscannum = 0; pscannum < pnum; pscannum++)
						if ((opinfo->param[pscannum].output & PIO_OUT) && ptype == paramlist[pscannum].type && pindex == paramlist[pscannum].value)
							skip = TRUE;

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

static void bevalidate_iterate_over_flags(drcuml_state *drcuml, code_handle **handles, const bevalidate_test *test, parameter *paramlist)
{
	const opcode_info *opinfo = opcode_info_table[test->opcode()];
	UINT8 flagmask = opinfo->outflags;
	UINT8 curmask;

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

static void bevalidate_execute(drcuml_state *drcuml, code_handle **handles, const bevalidate_test *test, const parameter *paramlist, UINT8 flagmask)
{
	parameter params[ARRAY_LENGTH(test->param)];
	drcuml_machine_state istate, fstate;
	instruction testinst;
	drcuml_block *block;
	UINT64 *parammem;
	int numparams;

	// allocate memory for parameters
	parammem = (UINT64 *)drcuml->cache->alloc_near(sizeof(UINT64) * (ARRAY_LENGTH(test->param) + 1));

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
	UML_GETFLGS(block, MEM(&parammem[ARRAY_LENGTH(test->param)]), flagmask);
	UML_SAVE(block, &fstate);
	UML_EXIT(block, IMM(0));

	// end the block
	block->end();

	// execute
	drcuml->execute(*handles[0]);

	// verify the results
	bevalidate_verify_state(drcuml, &istate, &fstate, test, *(UINT32 *)&parammem[ARRAY_LENGTH(test->param)], params, &testinst, handles[1]->code, handles[2]->code, flagmask);

	// free memory
	drcuml->cache->dealloc(parammem, sizeof(UINT64) * (ARRAY_LENGTH(test->param) + 1));
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
	for (regnum = 0; regnum < ARRAY_LENGTH(state->r); regnum++)
	{
		state->r[regnum].w.h = machine.rand();
		state->r[regnum].w.l = machine.rand();
	}

	// initialize float registers to random values
	for (regnum = 0; regnum < ARRAY_LENGTH(state->f); regnum++)
	{
		*(UINT32 *)&state->f[regnum].s.h = machine.rand();
		*(UINT32 *)&state->f[regnum].s.l = machine.rand();
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

static int bevalidate_populate_state(drcuml_block *block, drcuml_machine_state *state, const bevalidate_test *test, const parameter *paramlist, parameter *params, UINT64 *parammem)
{
	const opcode_info *opinfo = opcode_info_table[test->opcode()];
	int numparams = ARRAY_LENGTH(test->param);
	int pnum;

	// copy flags as-is
	state->flags = test->iflags;

	// iterate over parameters
	for (pnum = 0; pnum < ARRAY_LENGTH(test->param); pnum++)
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
				curparam->value = (FPTR)&parammem[pnum];
				if (psize == 4)
					*(UINT32 *)(FPTR)curparam->value = test->param[pnum];
				else
					*(UINT64 *)(FPTR)curparam->value = test->param[pnum];
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

static int bevalidate_verify_state(drcuml_state *drcuml, const drcuml_machine_state *istate, drcuml_machine_state *state, const bevalidate_test *test, UINT32 flags, const parameter *params, const instruction *testinst, drccodeptr codestart, drccodeptr codeend, UINT8 flagmask)
{
	const opcode_info *opinfo = opcode_info_table[test->opcode()];
	UINT8 ireg[REG_I_END - REG_I0] = { 0 };
	UINT8 freg[REG_F_END - REG_F0] = { 0 };
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
	for (pnum = 0; pnum < ARRAY_LENGTH(test->param); pnum++)
		if (opinfo->param[pnum].output & PIO_OUT)
		{
			int psize = effective_test_psize(opinfo, pnum, test->size, test->param);
			UINT64 mask = U64(0xffffffffffffffff) >> (64 - 8 * psize);
			UINT64 result = 0;

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
						result = *(UINT32 *)(FPTR)params[pnum].value;
					else
						result = *(UINT64 *)(FPTR)params[pnum].value;
					break;

				default:
					break;
			}

			// check against the mask
			if (test->param[pnum] != UNDEFINED_U64 && (result & mask) != (test->param[pnum] & mask))
			{
				if ((UINT32)mask == mask)
					errend += sprintf(errend, "  Parameter %d ... result:%08X  expected:%08X\n", pnum,
										(UINT32)(result & mask), (UINT32)(test->param[pnum] & mask));
				else
					errend += sprintf(errend, "  Parameter %d ... result:%08X%08X  expected:%08X%08X\n", pnum,
										(UINT32)((result & mask) >> 32), (UINT32)(result & mask),
										(UINT32)((test->param[pnum] & mask) >> 32), (UINT32)(test->param[pnum] & mask));
			}
		}

	// check source integer parameters for unexpected alterations
	for (regnum = 0; regnum < ARRAY_LENGTH(state->r); regnum++)
		if (ireg[regnum] == 0 && istate->r[regnum].d != state->r[regnum].d)
			errend += sprintf(errend, "  Register i%d ... result:%08X%08X  originally:%08X%08X\n", regnum,
								(UINT32)(state->r[regnum].d >> 32), (UINT32)state->r[regnum].d,
								(UINT32)(istate->r[regnum].d >> 32), (UINT32)istate->r[regnum].d);

	// check source float parameters for unexpected alterations
	for (regnum = 0; regnum < ARRAY_LENGTH(state->f); regnum++)
		if (freg[regnum] == 0 && *(UINT64 *)&istate->f[regnum].d != *(UINT64 *)&state->f[regnum].d)
			errend += sprintf(errend, "  Register f%d ... result:%08X%08X  originally:%08X%08X\n", regnum,
								(UINT32)(*(UINT64 *)&state->f[regnum].d >> 32), (UINT32)*(UINT64 *)&state->f[regnum].d,
								(UINT32)(*(UINT64 *)&istate->f[regnum].d >> 32), (UINT32)*(UINT64 *)&istate->f[regnum].d);

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

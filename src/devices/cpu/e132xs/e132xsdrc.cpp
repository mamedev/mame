// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "e132xs.h"
#include "e132xsfe.h"
#include "32xsdefs.h"

using namespace uml;

void hyperstone_device::execute_run_drc()
{
	drcuml_state *drcuml = m_drcuml.get();
	int execute_result;

	/* reset the cache if dirty */
	if (m_cache_dirty)
	{
		code_flush_cache();
		m_cache_dirty = false;
	}

	/* execute */
	do
	{
		/* run as much as we can */
		execute_result = drcuml->execute(*m_entry);

		/* if we need to recompile, do it */
		if (execute_result == EXECUTE_MISSING_CODE)
		{
			code_compile_block(m_global_regs[0]);
		}
		else if (execute_result == EXECUTE_UNMAPPED_CODE)
		{
			fatalerror("Attempted to execute unmapped code at PC=%08X\n", m_global_regs[0]);
		}
		else if (execute_result == EXECUTE_RESET_CACHE)
		{
			code_flush_cache();
		}
	} while (execute_result != EXECUTE_OUT_OF_CYCLES);
}

/***************************************************************************
    CACHE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    flush_drc_cache - outward-facing accessor to
    code_flush_cache
-------------------------------------------------*/

void hyperstone_device::flush_drc_cache()
{
	if (!m_enable_drc)
		return;
	m_cache_dirty = true;
}

/*-------------------------------------------------
    code_flush_cache - flush the cache and
    regenerate static code
-------------------------------------------------*/

void hyperstone_device::code_flush_cache()
{
	/* empty the transient cache contents */
	m_drcuml->reset();

	try
	{
		/* generate the entry point and out-of-cycles handlers */
		static_generate_entry_point();
		static_generate_nocode_handler();
		static_generate_out_of_cycles();

		/* add subroutines for memory accesses */
		static_generate_memory_accessor(1, false, false, "read8",     m_mem_read8);
		static_generate_memory_accessor(1, true,  false, "write8",    m_mem_write8);
		static_generate_memory_accessor(2, false, false, "read16",    m_mem_read16);
		static_generate_memory_accessor(2, true,  false, "write16",   m_mem_write16);
		static_generate_memory_accessor(4, false, false, "read32",    m_mem_read32);
		static_generate_memory_accessor(4, true,  false, "write32",   m_mem_write32);
		static_generate_memory_accessor(4, false, true,  "ioread32",  m_io_read32);
		static_generate_memory_accessor(4, true,  true,  "iowrite32", m_io_write32);
	}

	catch (drcuml_block::abort_compilation &)
	{
		fatalerror("Unable to generate static E132XS code\n");
	}
}

/*-------------------------------------------------
    code_compile_block - compile a block of the
    given mode at the specified pc
-------------------------------------------------*/

void hyperstone_device::code_compile_block(offs_t pc)
{
	drcuml_state *drcuml = m_drcuml.get();
	compiler_state compiler = { 0 };
	const opcode_desc *seqhead, *seqlast;
	const opcode_desc *desclist;
	int override = false;
	drcuml_block *block;

	g_profiler.start(PROFILER_DRC_COMPILE);

	/* get a description of this sequence */
	desclist = m_drcfe->describe_code(pc);

	bool succeeded = false;
	while (!succeeded)
	{
		try
		{
			/* start the block */
			block = drcuml->begin_block(4096);

			/* loop until we get through all instruction sequences */
			for (seqhead = desclist; seqhead != nullptr; seqhead = seqlast->next())
			{
				const opcode_desc *curdesc;
				uint32_t nextpc;

				/* add a code log entry */
				if (drcuml->logging())
					block->append_comment("-------------------------");                 // comment

				/* determine the last instruction in this sequence */
				for (seqlast = seqhead; seqlast != nullptr; seqlast = seqlast->next())
					if (seqlast->flags & OPFLAG_END_SEQUENCE)
						break;
				assert(seqlast != nullptr);

				/* if we don't have a hash for this mode/pc, or if we are overriding all, add one */
				if (override || !drcuml->hash_exists(0, seqhead->pc))
					UML_HASH(block, 0, seqhead->pc);                                        // hash    mode,pc

				/* if we already have a hash, and this is the first sequence, assume that we */
				/* are recompiling due to being out of sync and allow future overrides */
				else if (seqhead == desclist)
				{
					override = true;
					UML_HASH(block, 0, seqhead->pc);                                        // hash    mode,pc
				}

				/* otherwise, redispatch to that fixed PC and skip the rest of the processing */
				else
				{
					UML_LABEL(block, seqhead->pc | 0x80000000);                             // label   seqhead->pc
					UML_HASHJMP(block, 0, seqhead->pc, *m_nocode);
																							// hashjmp <0>,seqhead->pc,nocode
					continue;
				}

				/* validate this code block if we're not pointing into ROM */
				if (m_program->get_write_ptr(seqhead->physpc) != nullptr)
					generate_checksum_block(block, &compiler, seqhead, seqlast);

				/* label this instruction, if it may be jumped to locally */
				if (seqhead->flags & OPFLAG_IS_BRANCH_TARGET)
					UML_LABEL(block, seqhead->pc | 0x80000000);                             // label   seqhead->pc

				/* iterate over instructions in the sequence and compile them */
				for (curdesc = seqhead; curdesc != seqlast->next(); curdesc = curdesc->next())
					generate_sequence_instruction(block, &compiler, curdesc);

				/* if we need to return to the start, do it */
				if (seqlast->flags & OPFLAG_RETURN_TO_START)
					nextpc = pc;

				/* otherwise we just go to the next instruction */
				else
					nextpc = seqlast->pc + (seqlast->skipslots + 1) * 4;

				/* count off cycles and go there */
				generate_update_cycles(block, &compiler, nextpc, true);            // <subtract cycles>

				/* if the last instruction can change modes, use a variable mode; otherwise, assume the same mode */
				if (seqlast->next() == nullptr || seqlast->next()->pc != nextpc)
					UML_HASHJMP(block, 0, nextpc, *m_nocode);          // hashjmp <mode>,nextpc,nocode
			}

			/* end the sequence */
			block->end();
			g_profiler.stop();
			succeeded = true;
		}
		catch (drcuml_block::abort_compilation &)
		{
			code_flush_cache();
		}
	}
}

/***************************************************************************
    C FUNCTION CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    cfunc_unimplemented - handler for
    unimplemented opcdes
-------------------------------------------------*/

inline void hyperstone_device::ccfunc_unimplemented()
{
	fatalerror("PC=%08X: Unimplemented op %08X\n", PC, m_drc_arg0);
}

static void cfunc_unimplemented(void *param)
{
	((hyperstone_device *)param)->ccfunc_unimplemented();
}

/***************************************************************************
    STATIC CODEGEN
***************************************************************************/

/*-------------------------------------------------
    epc - compute the exception PC from a
    descriptor
-------------------------------------------------*/

static inline uint32_t epc(const opcode_desc *desc)
{
	return (desc->flags & OPFLAG_IN_DELAY_SLOT) ? (desc->pc - 3) : desc->pc;
}


/*-------------------------------------------------
    alloc_handle - allocate a handle if not
    already allocated
-------------------------------------------------*/

static inline void alloc_handle(drcuml_state *drcuml, code_handle **handleptr, const char *name)
{
	if (*handleptr == nullptr)
		*handleptr = drcuml->handle_alloc(name);
}


/*-------------------------------------------------
    generate_entry_point - generate a
    static entry point
-------------------------------------------------*/

void hyperstone_device::static_generate_entry_point()
{
	drcuml_state *drcuml = m_drcuml.get();
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(20);

	/* forward references */
	alloc_handle(drcuml, &m_nocode, "nocode");

	alloc_handle(drcuml, &m_entry, "entry");
	UML_HANDLE(block, *m_entry);

	/* load fast integer registers */
	//load_fast_iregs(block);

	/* generate a hash jump via the current mode and PC */
	UML_HASHJMP(block, 0, mem(&m_global_regs[0]), *m_nocode);
	block->end();
}


/*-------------------------------------------------
    static_generate_nocode_handler - generate an
    exception handler for "out of code"
-------------------------------------------------*/

void hyperstone_device::static_generate_nocode_handler()
{
	drcuml_state *drcuml = m_drcuml.get();
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(10);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &m_nocode, "nocode");
	UML_HANDLE(block, *m_nocode);
	UML_GETEXP(block, I0);
	UML_MOV(block, mem(&PC), I0);
	//save_fast_iregs(block);
	UML_EXIT(block, EXECUTE_MISSING_CODE);

	block->end();
}


/*-------------------------------------------------
    static_generate_out_of_cycles - generate an
    out of cycles exception handler
-------------------------------------------------*/

void hyperstone_device::static_generate_out_of_cycles()
{
	drcuml_state *drcuml = m_drcuml.get();
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(10);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &m_out_of_cycles, "out_of_cycles");
	UML_HANDLE(block, *m_out_of_cycles);
	UML_GETEXP(block, I0);
	UML_MOV(block, mem(&m_global_regs[0]), I0);
	//save_fast_iregs(block);
	UML_EXIT(block, EXECUTE_OUT_OF_CYCLES);

	block->end();
}

/*------------------------------------------------------------------
    static_generate_memory_accessor
------------------------------------------------------------------*/

void hyperstone_device::static_generate_memory_accessor(int size, int iswrite, bool isio, const char *name, code_handle *&handleptr)
{
	/* on entry, address is in I0; data for writes is in I1 */
	/* on exit, read result is in I0 */
	/* routine trashes I0-I1 */
	drcuml_state *drcuml = m_drcuml.get();
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(1024);

	/* add a global entry for this */
	alloc_handle(drcuml, &handleptr, name);
	UML_HANDLE(block, *handleptr);

	// write:
	switch (size)
	{
		case 1:
			if (iswrite)
				UML_WRITE(block, I0, I1, SIZE_BYTE, SPACE_PROGRAM);
			else
				UML_READ(block, I0, I0, SIZE_BYTE, SPACE_PROGRAM);
			break;

		case 2:
			if (iswrite)
				UML_WRITE(block, I0, I1, SIZE_WORD, SPACE_PROGRAM);
			else
				UML_READ(block, I0, I0, SIZE_WORD, SPACE_PROGRAM);
			break;

		case 4:
			if (iswrite)
				UML_WRITE(block, I0, I1, SIZE_DWORD, isio ? SPACE_IO : SPACE_PROGRAM);
			else
				UML_READ(block, I0, I0, SIZE_DWORD, isio ? SPACE_IO : SPACE_PROGRAM);
			break;
	}
	UML_RET(block);

	block->end();
}



/***************************************************************************
    CODE GENERATION
***************************************************************************/

/*-------------------------------------------------
    generate_update_cycles - generate code to
    subtract cycles from the icount and generate
    an exception if out
-------------------------------------------------*/

void hyperstone_device::generate_update_cycles(drcuml_block *block, compiler_state *compiler, uml::parameter param, bool allow_exception)
{
	/* account for cycles */
	if (compiler->m_cycles > 0)
	{
		UML_SUB(block, mem(&m_icount), mem(&m_icount), MAPVAR_CYCLES);
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);
		UML_EXHc(block, COND_S, *m_out_of_cycles, param);
	}
	compiler->m_cycles = 0;
}

/*-------------------------------------------------
    generate_checksum_block - generate code to
    validate a sequence of opcodes
-------------------------------------------------*/

void hyperstone_device::generate_checksum_block(drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast)
{
	const opcode_desc *curdesc;
	if (m_drcuml->logging())
	{
		block->append_comment("[Validation for %08X]", seqhead->pc | 0x1000);
	}
	/* loose verify or single instruction: just compare and fail */
	if (!(m_drcoptions & E132XS_STRICT_VERIFY) || seqhead->next() == nullptr)
	{
		if (!(seqhead->flags & OPFLAG_VIRTUAL_NOOP))
		{
			uint32_t sum = seqhead->opptr.l[0];
			void *base = m_direct->read_ptr(seqhead->physpc | 0x1000);
			UML_LOAD(block, I0, base, 0, SIZE_DWORD, SCALE_x4);

			if (seqhead->delay.first() != nullptr && seqhead->physpc != seqhead->delay.first()->physpc)
			{
				base = m_direct->read_ptr(seqhead->delay.first()->physpc);
				assert(base != nullptr);
				UML_LOAD(block, I1, base, 0, SIZE_DWORD, SCALE_x4);
				UML_ADD(block, I0, I0, I1);

				sum += seqhead->delay.first()->opptr.l[0];
			}

			UML_CMP(block, I0, sum);
			UML_EXHc(block, COND_NE, *m_nocode, epc(seqhead));
		}
	}
	else /* full verification; sum up everything */
	{
		void *base = m_direct->read_ptr(seqhead->physpc);
		UML_LOAD(block, I0, base, 0, SIZE_DWORD, SCALE_x4);
		uint32_t sum = seqhead->opptr.l[0];
		for (curdesc = seqhead->next(); curdesc != seqlast->next(); curdesc = curdesc->next())
			if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
			{
				base = m_direct->read_ptr(curdesc->physpc);
				assert(base != nullptr);
				UML_LOAD(block, I1, base, 0, SIZE_DWORD, SCALE_x4);
				UML_ADD(block, I0, I0, I1);
				sum += curdesc->opptr.l[0];

				if (curdesc->delay.first() != nullptr && (curdesc == seqlast || (curdesc->next() != nullptr && curdesc->next()->physpc != curdesc->delay.first()->physpc)))
				{
					base = m_direct->read_ptr(curdesc->delay.first()->physpc);
					assert(base != nullptr);
					UML_LOAD(block, I1, base, 0, SIZE_DWORD, SCALE_x4);
					UML_ADD(block, I0, I0, I1);

					sum += curdesc->delay.first()->opptr.l[0];
				}
			}
		UML_CMP(block, I0, sum);
		UML_EXHc(block, COND_NE, *m_nocode, epc(seqhead));
	}
}


/*-------------------------------------------------
    log_add_disasm_comment - add a comment
    including disassembly of a MIPS instruction
-------------------------------------------------*/

void hyperstone_device::log_add_disasm_comment(drcuml_block *block, uint32_t pc, uint32_t op)
{
	if (m_drcuml->logging())
	{
		block->append_comment("%08X: %08x", pc, op);
	}
}


/*-------------------------------------------------
    generate_sequence_instruction - generate code
    for a single instruction in a sequence
-------------------------------------------------*/

void hyperstone_device::generate_sequence_instruction(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	offs_t expc;

	/* add an entry for the log */
	if (m_drcuml->logging() && !(desc->flags & OPFLAG_VIRTUAL_NOOP))
		log_add_disasm_comment(block, desc->pc, desc->opptr.l[0]);

	/* set the PC map variable */
	expc = (desc->flags & OPFLAG_IN_DELAY_SLOT) ? desc->pc - 3 : desc->pc;
	UML_MAPVAR(block, MAPVAR_PC, expc);

	/* accumulate total cycles */
	compiler->m_cycles += desc->cycles;

	/* update the icount map variable */
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->m_cycles);

	/* if we are debugging, call the debugger */
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		UML_MOV(block, mem(&PC), desc->pc);
		//save_fast_iregs(block);
		UML_DEBUG(block, desc->pc);
	}

	if (!(desc->flags & OPFLAG_VIRTUAL_NOOP))
	{
		/* compile the instruction */
		if (!generate_opcode(block, compiler, desc))
		{
			UML_MOV(block, mem(&PC), desc->pc);
			UML_MOV(block, mem(&m_drc_arg0), desc->opptr.l[0]);
			UML_CALLC(block, cfunc_unimplemented, this);
		}
	}
}

bool hyperstone_device::generate_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	uint32_t op = desc->opptr.l[0];
	uint8_t opswitch = (op & 0x0000ff00) >> 8;
	code_label skip;

	switch (opswitch)
	{
		case 0x00: generate_op00(block, compiler, desc); break;
		case 0x01: generate_op01(block, compiler, desc); break;
		case 0x02: generate_op02(block, compiler, desc); break;
		case 0x03: generate_op03(block, compiler, desc); break;
		case 0x04: generate_op04(block, compiler, desc); break;
		case 0x05: generate_op05(block, compiler, desc); break;
		case 0x06: generate_op06(block, compiler, desc); break;
		case 0x07: generate_op07(block, compiler, desc); break;
		case 0x08: generate_op08(block, compiler, desc); break;
		case 0x09: generate_op09(block, compiler, desc); break;
		case 0x0a: generate_op0a(block, compiler, desc); break;
		case 0x0b: generate_op0b(block, compiler, desc); break;
		case 0x0c: generate_op0c(block, compiler, desc); break;
		case 0x0d: generate_op0d(block, compiler, desc); break;
		case 0x0e: generate_op0e(block, compiler, desc); break;
		case 0x0f: generate_op0f(block, compiler, desc); break;
		case 0x10: generate_op10(block, compiler, desc); break;
		case 0x11: generate_op11(block, compiler, desc); break;
		case 0x12: generate_op12(block, compiler, desc); break;
		case 0x13: generate_op13(block, compiler, desc); break;
		case 0x14: generate_op14(block, compiler, desc); break;
		case 0x15: generate_op15(block, compiler, desc); break;
		case 0x16: generate_op16(block, compiler, desc); break;
		case 0x17: generate_op17(block, compiler, desc); break;
		case 0x18: generate_op18(block, compiler, desc); break;
		case 0x19: generate_op19(block, compiler, desc); break;
		case 0x1a: generate_op1a(block, compiler, desc); break;
		case 0x1b: generate_op1b(block, compiler, desc); break;
		case 0x1c: generate_op1c(block, compiler, desc); break;
		case 0x1d: generate_op1d(block, compiler, desc); break;
		case 0x1e: generate_op1e(block, compiler, desc); break;
		case 0x1f: generate_op1f(block, compiler, desc); break;
		case 0x20: generate_op20(block, compiler, desc); break;
		case 0x21: generate_op21(block, compiler, desc); break;
		case 0x22: generate_op22(block, compiler, desc); break;
		case 0x23: generate_op23(block, compiler, desc); break;
		case 0x24: generate_op24(block, compiler, desc); break;
		case 0x25: generate_op25(block, compiler, desc); break;
		case 0x26: generate_op26(block, compiler, desc); break;
		case 0x27: generate_op27(block, compiler, desc); break;
		case 0x28: generate_op28(block, compiler, desc); break;
		case 0x29: generate_op29(block, compiler, desc); break;
		case 0x2a: generate_op2a(block, compiler, desc); break;
		case 0x2b: generate_op2b(block, compiler, desc); break;
		case 0x2c: generate_op2c(block, compiler, desc); break;
		case 0x2d: generate_op2d(block, compiler, desc); break;
		case 0x2e: generate_op2e(block, compiler, desc); break;
		case 0x2f: generate_op2f(block, compiler, desc); break;
		case 0x30: generate_op30(block, compiler, desc); break;
		case 0x31: generate_op31(block, compiler, desc); break;
		case 0x32: generate_op32(block, compiler, desc); break;
		case 0x33: generate_op33(block, compiler, desc); break;
		case 0x34: generate_op34(block, compiler, desc); break;
		case 0x35: generate_op35(block, compiler, desc); break;
		case 0x36: generate_op36(block, compiler, desc); break;
		case 0x37: generate_op37(block, compiler, desc); break;
		case 0x38: generate_op38(block, compiler, desc); break;
		case 0x39: generate_op39(block, compiler, desc); break;
		case 0x3a: generate_op3a(block, compiler, desc); break;
		case 0x3b: generate_op3b(block, compiler, desc); break;
		case 0x3c: generate_op3c(block, compiler, desc); break;
		case 0x3d: generate_op3d(block, compiler, desc); break;
		case 0x3e: generate_op3e(block, compiler, desc); break;
		case 0x3f: generate_op3f(block, compiler, desc); break;
		case 0x40: generate_op40(block, compiler, desc); break;
		case 0x41: generate_op41(block, compiler, desc); break;
		case 0x42: generate_op42(block, compiler, desc); break;
		case 0x43: generate_op43(block, compiler, desc); break;
		case 0x44: generate_op44(block, compiler, desc); break;
		case 0x45: generate_op45(block, compiler, desc); break;
		case 0x46: generate_op46(block, compiler, desc); break;
		case 0x47: generate_op47(block, compiler, desc); break;
		case 0x48: generate_op48(block, compiler, desc); break;
		case 0x49: generate_op49(block, compiler, desc); break;
		case 0x4a: generate_op4a(block, compiler, desc); break;
		case 0x4b: generate_op4b(block, compiler, desc); break;
		case 0x4c: generate_op4c(block, compiler, desc); break;
		case 0x4d: generate_op4d(block, compiler, desc); break;
		case 0x4e: generate_op4e(block, compiler, desc); break;
		case 0x4f: generate_op4f(block, compiler, desc); break;
		case 0x50: generate_op50(block, compiler, desc); break;
		case 0x51: generate_op51(block, compiler, desc); break;
		case 0x52: generate_op52(block, compiler, desc); break;
		case 0x53: generate_op53(block, compiler, desc); break;
		case 0x54: generate_op54(block, compiler, desc); break;
		case 0x55: generate_op55(block, compiler, desc); break;
		case 0x56: generate_op56(block, compiler, desc); break;
		case 0x57: generate_op57(block, compiler, desc); break;
		case 0x58: generate_op58(block, compiler, desc); break;
		case 0x59: generate_op59(block, compiler, desc); break;
		case 0x5a: generate_op5a(block, compiler, desc); break;
		case 0x5b: generate_op5b(block, compiler, desc); break;
		case 0x5c: generate_op5c(block, compiler, desc); break;
		case 0x5d: generate_op5d(block, compiler, desc); break;
		case 0x5e: generate_op5e(block, compiler, desc); break;
		case 0x5f: generate_op5f(block, compiler, desc); break;
		case 0x60: generate_op60(block, compiler, desc); break;
		case 0x61: generate_op61(block, compiler, desc); break;
		case 0x62: generate_op62(block, compiler, desc); break;
		case 0x63: generate_op63(block, compiler, desc); break;
		case 0x64: generate_op64(block, compiler, desc); break;
		case 0x65: generate_op65(block, compiler, desc); break;
		case 0x66: generate_op66(block, compiler, desc); break;
		case 0x67: generate_op67(block, compiler, desc); break;
		case 0x68: generate_op68(block, compiler, desc); break;
		case 0x69: generate_op69(block, compiler, desc); break;
		case 0x6a: generate_op6a(block, compiler, desc); break;
		case 0x6b: generate_op6b(block, compiler, desc); break;
		case 0x6c: generate_op6c(block, compiler, desc); break;
		case 0x6d: generate_op6d(block, compiler, desc); break;
		case 0x6e: generate_op6e(block, compiler, desc); break;
		case 0x6f: generate_op6f(block, compiler, desc); break;
		case 0x70: generate_op70(block, compiler, desc); break;
		case 0x71: generate_op71(block, compiler, desc); break;
		case 0x72: generate_op72(block, compiler, desc); break;
		case 0x73: generate_op73(block, compiler, desc); break;
		case 0x74: generate_op74(block, compiler, desc); break;
		case 0x75: generate_op75(block, compiler, desc); break;
		case 0x76: generate_op76(block, compiler, desc); break;
		case 0x77: generate_op77(block, compiler, desc); break;
		case 0x78: generate_op78(block, compiler, desc); break;
		case 0x79: generate_op79(block, compiler, desc); break;
		case 0x7a: generate_op7a(block, compiler, desc); break;
		case 0x7b: generate_op7b(block, compiler, desc); break;
		case 0x7c: generate_op7c(block, compiler, desc); break;
		case 0x7d: generate_op7d(block, compiler, desc); break;
		case 0x7e: generate_op7e(block, compiler, desc); break;
		case 0x7f: generate_op7f(block, compiler, desc); break;
		case 0x80: generate_op80(block, compiler, desc); break;
		case 0x81: generate_op81(block, compiler, desc); break;
		case 0x82: generate_op82(block, compiler, desc); break;
		case 0x83: generate_op83(block, compiler, desc); break;
		case 0x84: generate_op84(block, compiler, desc); break;
		case 0x85: generate_op85(block, compiler, desc); break;
		case 0x86: generate_op86(block, compiler, desc); break;
		case 0x87: generate_op87(block, compiler, desc); break;
		case 0x88: generate_op88(block, compiler, desc); break;
		case 0x89: generate_op89(block, compiler, desc); break;
		case 0x8a: generate_op8a(block, compiler, desc); break;
		case 0x8b: generate_op8b(block, compiler, desc); break;
		case 0x8c:
		case 0x8d: return false;
		case 0x8e: generate_op8e(block, compiler, desc); break;
		case 0x8f: generate_op8f(block, compiler, desc); break;
		case 0x90: generate_op90(block, compiler, desc); break;
		case 0x91: generate_op91(block, compiler, desc); break;
		case 0x92: generate_op92(block, compiler, desc); break;
		case 0x93: generate_op93(block, compiler, desc); break;
		case 0x94: generate_op94(block, compiler, desc); break;
		case 0x95: generate_op95(block, compiler, desc); break;
		case 0x96: generate_op96(block, compiler, desc); break;
		case 0x97: generate_op97(block, compiler, desc); break;
		case 0x98: generate_op98(block, compiler, desc); break;
		case 0x99: generate_op99(block, compiler, desc); break;
		case 0x9a: generate_op9a(block, compiler, desc); break;
		case 0x9b: generate_op9b(block, compiler, desc); break;
		case 0x9c: generate_op9c(block, compiler, desc); break;
		case 0x9d: generate_op9d(block, compiler, desc); break;
		case 0x9e: generate_op9e(block, compiler, desc); break;
		case 0x9f: generate_op9f(block, compiler, desc); break;
		case 0xa0: generate_opa0(block, compiler, desc); break;
		case 0xa1: generate_opa1(block, compiler, desc); break;
		case 0xa2: generate_opa2(block, compiler, desc); break;
		case 0xa3: generate_opa3(block, compiler, desc); break;
		case 0xa4: generate_opa4(block, compiler, desc); break;
		case 0xa5: generate_opa5(block, compiler, desc); break;
		case 0xa6: generate_opa6(block, compiler, desc); break;
		case 0xa7: generate_opa7(block, compiler, desc); break;
		case 0xa8: generate_opa8(block, compiler, desc); break;
		case 0xa9: generate_opa9(block, compiler, desc); break;
		case 0xaa: generate_opaa(block, compiler, desc); break;
		case 0xab: generate_opab(block, compiler, desc); break;
		case 0xac:
		case 0xad:
		case 0xae:
		case 0xaf: return false;
		case 0xb0: generate_opb0(block, compiler, desc); break;
		case 0xb1: generate_opb1(block, compiler, desc); break;
		case 0xb2: generate_opb2(block, compiler, desc); break;
		case 0xb3: generate_opb3(block, compiler, desc); break;
		case 0xb4: generate_opb4(block, compiler, desc); break;
		case 0xb5: generate_opb5(block, compiler, desc); break;
		case 0xb6: generate_opb6(block, compiler, desc); break;
		case 0xb7: generate_opb7(block, compiler, desc); break;
		case 0xb8: generate_opb8(block, compiler, desc); break;
		case 0xb9: generate_opb9(block, compiler, desc); break;
		case 0xba: generate_opba(block, compiler, desc); break;
		case 0xbb: generate_opbb(block, compiler, desc); break;
		case 0xbc: generate_opbc(block, compiler, desc); break;
		case 0xbd: generate_opbd(block, compiler, desc); break;
		case 0xbe: generate_opbe(block, compiler, desc); break;
		case 0xbf: generate_opbf(block, compiler, desc); break;
		case 0xc0: generate_opc0(block, compiler, desc); break;
		case 0xc1: generate_opc1(block, compiler, desc); break;
		case 0xc2: generate_opc2(block, compiler, desc); break;
		case 0xc3: generate_opc3(block, compiler, desc); break;
		case 0xc4: generate_opc4(block, compiler, desc); break;
		case 0xc5: generate_opc5(block, compiler, desc); break;
		case 0xc6: generate_opc6(block, compiler, desc); break;
		case 0xc7: generate_opc7(block, compiler, desc); break;
		case 0xc8: generate_opc8(block, compiler, desc); break;
		case 0xc9: generate_opc9(block, compiler, desc); break;
		case 0xca: generate_opca(block, compiler, desc); break;
		case 0xcb: generate_opcb(block, compiler, desc); break;
		case 0xcc: generate_opcc(block, compiler, desc); break;
		case 0xcd: generate_opcd(block, compiler, desc); break;
		case 0xce: generate_opce(block, compiler, desc); break;
		case 0xcf: generate_opcf(block, compiler, desc); break;
		case 0xd0: generate_opd0(block, compiler, desc); break;
		case 0xd1: generate_opd1(block, compiler, desc); break;
		case 0xd2: generate_opd2(block, compiler, desc); break;
		case 0xd3: generate_opd3(block, compiler, desc); break;
		case 0xd4: generate_opd4(block, compiler, desc); break;
		case 0xd5: generate_opd5(block, compiler, desc); break;
		case 0xd6: generate_opd6(block, compiler, desc); break;
		case 0xd7: generate_opd7(block, compiler, desc); break;
		case 0xd8: generate_opd8(block, compiler, desc); break;
		case 0xd9: generate_opd9(block, compiler, desc); break;
		case 0xda: generate_opda(block, compiler, desc); break;
		case 0xdb: generate_opdb(block, compiler, desc); break;
		case 0xdc: generate_opdc(block, compiler, desc); break;
		case 0xdd: generate_opdd(block, compiler, desc); break;
		case 0xde: generate_opde(block, compiler, desc); break;
		case 0xdf: generate_opdf(block, compiler, desc); break;
		case 0xe0: generate_ope0(block, compiler, desc); break;
		case 0xe1: generate_ope1(block, compiler, desc); break;
		case 0xe2: generate_ope2(block, compiler, desc); break;
		case 0xe3: generate_ope3(block, compiler, desc); break;
		case 0xe4: generate_ope4(block, compiler, desc); break;
		case 0xe5: generate_ope5(block, compiler, desc); break;
		case 0xe6: generate_ope6(block, compiler, desc); break;
		case 0xe7: generate_ope7(block, compiler, desc); break;
		case 0xe8: generate_ope8(block, compiler, desc); break;
		case 0xe9: generate_ope9(block, compiler, desc); break;
		case 0xea: generate_opea(block, compiler, desc); break;
		case 0xeb: generate_opeb(block, compiler, desc); break;
		case 0xec: generate_opec(block, compiler, desc); break;
		case 0xed: generate_oped(block, compiler, desc); break;
		case 0xee: generate_opee(block, compiler, desc); break;
		case 0xef: generate_opef(block, compiler, desc); break;
		case 0xf0: generate_opf0(block, compiler, desc); break;
		case 0xf1: generate_opf1(block, compiler, desc); break;
		case 0xf2: generate_opf2(block, compiler, desc); break;
		case 0xf3: generate_opf3(block, compiler, desc); break;
		case 0xf4: generate_opf4(block, compiler, desc); break;
		case 0xf5: generate_opf5(block, compiler, desc); break;
		case 0xf6: generate_opf6(block, compiler, desc); break;
		case 0xf7: generate_opf7(block, compiler, desc); break;
		case 0xf8: generate_opf8(block, compiler, desc); break;
		case 0xf9: generate_opf9(block, compiler, desc); break;
		case 0xfa: generate_opfa(block, compiler, desc); break;
		case 0xfb: generate_opfb(block, compiler, desc); break;
		case 0xfc: generate_opfc(block, compiler, desc); break;
		case 0xfd: generate_opfd(block, compiler, desc); break;
		case 0xfe: generate_opfe(block, compiler, desc); break;
		case 0xff: generate_opff(block, compiler, desc); break;
	}
	return true;
}
// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "emu.h"
#include "unsp.h"
#include "unspfe.h"
#include "unspdefs.h"

void unsp_device::invalidate_cache()
{
	m_cache_dirty = true;
}

void unsp_device::execute_run_drc()
{
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
		execute_result = m_drcuml->execute(*m_entry);

		/* if we need to recompile, do it */
		if (execute_result == EXECUTE_MISSING_CODE)
		{
			code_compile_block(UNSP_LPC);
		}
		else if (execute_result == EXECUTE_UNMAPPED_CODE)
		{
			fatalerror("Attempted to execute unmapped code at PC=%08X\n", UNSP_LPC);
		}
		else if (execute_result == EXECUTE_RESET_CACHE)
		{
			code_flush_cache();
		}
	} while (execute_result != EXECUTE_OUT_OF_CYCLES);
}


/***************************************************************************
    C FUNCTION CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    cfunc_unimplemented - handler for
    unimplemented opcdes
-------------------------------------------------*/

inline void unsp_device::ccfunc_unimplemented()
{
	fatalerror("PC=%08X: Unimplemented op %04x\n", UNSP_LPC, (uint16_t)m_core->m_arg0);
}

static void cfunc_unimplemented(void *param)
{
	((unsp_device *)param)->ccfunc_unimplemented();
}

#if UNSP_LOG_REGS
void unsp_device::cfunc_log_write()
{
	log_write(m_core->m_arg0, m_core->m_arg1);
}

static void cfunc_log_regs(void *param)
{
	((unsp_device *)param)->log_regs();
}

static void ccfunc_log_write(void *param)
{
	((unsp_device *)param)->cfunc_log_write();
}
#endif

/***************************************************************************
    CACHE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    flush_drc_cache - outward-facing accessor to
    code_flush_cache
-------------------------------------------------*/

void unsp_device::flush_drc_cache()
{
	if (!m_enable_drc)
		return;
	m_cache_dirty = true;
}

/*-------------------------------------------------
    code_flush_cache - flush the cache and
    regenerate static code
-------------------------------------------------*/

void unsp_device::code_flush_cache()
{
	/* empty the transient cache contents */
	m_drcuml->reset();

	try
	{
		/* generate the entry point and out-of-cycles handlers */
		static_generate_entry_point();
		static_generate_nocode_handler();
		static_generate_out_of_cycles();

		static_generate_memory_accessor(false, "read", m_mem_read);
		static_generate_memory_accessor(true, "write", m_mem_write);
		static_generate_trigger_fiq();
		static_generate_trigger_irq();
		static_generate_check_interrupts();
	}

	catch (drcuml_block::abort_compilation &)
	{
		fatalerror("Unable to generate static u'nSP code\n"); fflush(stdout);
	}
}

/*-------------------------------------------------
    code_compile_block - compile a block of the
    given mode at the specified pc
-------------------------------------------------*/

void unsp_device::code_compile_block(offs_t pc)
{
	compiler_state compiler = { 0 };
	const opcode_desc *seqhead, *seqlast;
	bool override = false;

	g_profiler.start(PROFILER_DRC_COMPILE);

	/* get a description of this sequence */
	const opcode_desc *desclist = m_drcfe->describe_code(pc);

	bool succeeded = false;
	while (!succeeded)
	{
		try
		{
			/* start the block */
			drcuml_block &block(m_drcuml->begin_block(1024*8));

			/* loop until we get through all instruction sequences */
			for (seqhead = desclist; seqhead != nullptr; seqhead = seqlast->next())
			{
				const opcode_desc *curdesc;
				uint32_t nextpc;

				/* add a code log entry */
				if (m_drcuml->logging())
					block.append_comment("-------------------------");

				/* determine the last instruction in this sequence */
				for (seqlast = seqhead; seqlast != nullptr; seqlast = seqlast->next())
					if (seqlast->flags & OPFLAG_END_SEQUENCE)
						break;
				assert(seqlast != nullptr);

				/* if we don't have a hash for this mode/pc, or if we are overriding all, add one */
				if (override || !m_drcuml->hash_exists(0, seqhead->pc))
					UML_HASH(block, 0, seqhead->pc);

				/* if we already have a hash, and this is the first sequence, assume that we */
				/* are recompiling due to being out of sync and allow future overrides */
				else if (seqhead == desclist)
				{
					override = true;
					UML_HASH(block, 0, seqhead->pc);
				}

				/* otherwise, redispatch to that fixed PC and skip the rest of the processing */
				else
				{
					UML_LABEL(block, seqhead->pc | 0x80000000);
					UML_HASHJMP(block, 0, seqhead->pc, *m_nocode);
					continue;
				}

				/* validate this code block if we're not pointing into ROM */
				if (m_program->get_write_ptr(seqhead->physpc) != nullptr)
					generate_checksum_block(block, compiler, seqhead, seqlast);

				/* label this instruction, if it may be jumped to locally */
				if (seqhead->flags & OPFLAG_IS_BRANCH_TARGET)
					UML_LABEL(block, seqhead->pc | 0x80000000);

				/* iterate over instructions in the sequence and compile them */
				for (curdesc = seqhead; curdesc != seqlast->next(); curdesc = curdesc->next())
				{
					generate_check_cycles(block, compiler, curdesc->pc + curdesc->length);
					generate_sequence_instruction(block, compiler, curdesc);
					UML_CALLH(block, *m_check_interrupts);
				}

				nextpc = seqlast->pc + seqlast->length;

				/* if the last instruction can change modes, use a variable mode; otherwise, assume the same mode */
				if (seqlast->next() == nullptr || seqlast->next()->pc != nextpc)
				{
					UML_HASHJMP(block, 0, nextpc, *m_nocode);          // hashjmp <mode>,nextpc,nocode
				}
			}

			/* end the sequence */
			block.end();
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
    STATIC CODEGEN
***************************************************************************/

/*-------------------------------------------------
    alloc_handle - allocate a handle if not
    already allocated
-------------------------------------------------*/

static inline void alloc_handle(drcuml_state &drcuml, uml::code_handle *&handleptr, const char *name)
{
	if (!handleptr)
		handleptr = drcuml.handle_alloc(name);
}

/*-------------------------------------------------
    generate_entry_point - generate a
    static entry point
-------------------------------------------------*/

void unsp_device::static_generate_entry_point()
{
	/* begin generating */
	drcuml_block &block(m_drcuml->begin_block(32));

	/* forward references */
	alloc_handle(*m_drcuml, m_nocode, "nocode");

	alloc_handle(*m_drcuml, m_entry, "entry");
	UML_HANDLE(block, *m_entry);

	/* load fast integer registers */
	//load_fast_iregs(block);

	/* generate a hash jump via the current mode and PC */
	UML_ROLAND(block, I1, mem(&m_core->m_r[REG_SR]), 16, 0x003f0000);
	UML_OR(block, I1, I1, mem(&m_core->m_r[REG_PC]));
	UML_HASHJMP(block, 0, I1, *m_nocode);
	block.end();
}


/*-------------------------------------------------
    static_generate_nocode_handler - generate an
    exception handler for "out of code"
-------------------------------------------------*/

void unsp_device::static_generate_nocode_handler()
{
	/* begin generating */
	drcuml_block &block(m_drcuml->begin_block(10));

	/* generate a hash jump via the current mode and PC */
	alloc_handle(*m_drcuml, m_nocode, "nocode");
	UML_HANDLE(block, *m_nocode);
	UML_GETEXP(block, I0);

	UML_ROLINS(block, mem(&m_core->m_r[REG_SR]), I0, 16, 0x3f);
	UML_AND(block, mem(&m_core->m_r[REG_PC]), I0, 0x0000ffff);
	//save_fast_iregs(block);
	UML_EXIT(block, EXECUTE_MISSING_CODE);

	block.end();
}


/*-------------------------------------------------
    static_generate_out_of_cycles - generate an
    out of cycles exception handler
-------------------------------------------------*/

void unsp_device::static_generate_out_of_cycles()
{
	/* begin generating */
	drcuml_block &block(m_drcuml->begin_block(10));

	/* generate a hash jump via the current mode and PC */
	alloc_handle(*m_drcuml, m_out_of_cycles, "out_of_cycles");
	UML_HANDLE(block, *m_out_of_cycles);
	//save_fast_iregs(block);
	UML_EXIT(block, EXECUTE_OUT_OF_CYCLES);

	block.end();
}

/*------------------------------------------------------------------
    static_generate_memory_accessor
------------------------------------------------------------------*/

void unsp_device::static_generate_memory_accessor(bool iswrite, const char *name, uml::code_handle *&handleptr)
{
	/* on entry, address is in I0; data for writes is in I1 */
	/* on exit, read result is in I1 */

	/* begin generating */
	drcuml_block &block(m_drcuml->begin_block(32));

	/* add a global entry for this */
	alloc_handle(*m_drcuml, handleptr, name);
	UML_HANDLE(block, *handleptr);

	if (iswrite)
	{
#if UNSP_LOG_REGS
		UML_MOV(block, mem(&m_core->m_arg0), I0);
		UML_MOV(block, mem(&m_core->m_arg1), I1);
		UML_CALLC(block, ccfunc_log_write, this);
#endif
		UML_WRITE(block, I0, I1, SIZE_WORD, SPACE_PROGRAM);
	}
	else
		UML_READ(block, I1, I0, SIZE_WORD, SPACE_PROGRAM);
	UML_RET(block);

	block.end();
}



/***************************************************************************
    CODE GENERATION
***************************************************************************/

void unsp_device::static_generate_check_interrupts()
{
	uml::code_label test_loop = 1;
	uml::code_label found = 2;
	uml::code_label do_irq = 3;

	/* begin generating */
	drcuml_block &block(m_drcuml->begin_block(256));

	/* generate a hash jump via the current mode and PC */
	alloc_handle(*m_drcuml, m_check_interrupts, "check_interrupts");
	UML_HANDLE(block, *m_check_interrupts);

	UML_CMP(block, mem(&m_core->m_sirq), 0);
	UML_RETc(block, uml::COND_Z);

	UML_MOV(block, I2, 0);
	UML_MOV(block, I0, 1);
	UML_MOV(block, I1, mem(&m_core->m_sirq));

	UML_LABEL(block, test_loop);
	UML_TEST(block, I1, I0);
	UML_JMPc(block, uml::COND_NZ, found);
	UML_SHL(block, I0, I0, 1);
	UML_ADD(block, I2, I2, 1);
	UML_CMP(block, I0, 1 << 9);
	UML_JMPc(block, uml::COND_NE, test_loop);
	UML_RET(block);

	UML_LABEL(block, found);
	UML_CMP(block, I0, 1);
	UML_JMPc(block, uml::COND_NE, do_irq);
	UML_CALLH(block, *m_trigger_fiq);
	UML_RET(block);

	UML_LABEL(block, do_irq);
	UML_CALLH(block, *m_trigger_irq);
	UML_RET(block);

	block.end();
}

void unsp_device::static_generate_trigger_fiq()
{
	/* begin generating */
	drcuml_block &block(m_drcuml->begin_block(256));

	/* generate a hash jump via the current mode and PC */
	alloc_handle(*m_drcuml, m_trigger_fiq, "trigger_fiq");
	UML_HANDLE(block, *m_trigger_fiq);

	UML_OR(block, I0, mem(&m_core->m_fiq), mem(&m_core->m_irq));
	UML_TEST(block, I0, 1);
	UML_RETc(block, uml::COND_NZ);

	UML_TEST(block, mem(&m_core->m_enable_fiq), 1);
	UML_RETc(block, uml::COND_Z);

	UML_MOV(block, mem(&m_core->m_fiq), 1);

	UML_MOV(block, I0, mem(&m_core->m_sb));
	UML_MOV(block, I1, mem(&m_core->m_irq));
	UML_STORE(block, (void*)m_core->m_saved_sb, I1, I0, SIZE_DWORD, SCALE_x4);
	UML_MOV(block, mem(&m_core->m_sb), mem(&m_core->m_saved_sb[2]));

	UML_MOV(block, I0, mem(&m_core->m_r[REG_SP]));

	UML_MOV(block, I1, mem(&m_core->m_r[REG_PC]));
	UML_CALLH(block, *m_mem_write);
	UML_SUB(block, I0, I0, 1);

	UML_MOV(block, I1, mem(&m_core->m_r[REG_SR]));
	UML_CALLH(block, *m_mem_write);
	UML_SUB(block, I0, I0, 1);

	UML_AND(block, mem(&m_core->m_r[REG_SP]), I0, 0x0000ffff);

	UML_MOV(block, I0, 0x0000fff6);
	UML_CALLH(block, *m_mem_read);
	UML_MOV(block, mem(&m_core->m_r[REG_PC]), I1);
	UML_MOV(block, mem(&m_core->m_r[REG_SR]), 0);
	UML_HASHJMP(block, 0, I1, *m_nocode);
	UML_RET(block);

	block.end();
}

void unsp_device::static_generate_trigger_irq()
{
	/* begin generating */
	drcuml_block &block(m_drcuml->begin_block(256));

	/* generate a hash jump via the current mode and PC */
	alloc_handle(*m_drcuml, m_trigger_irq, "trigger_irq");
	UML_HANDLE(block, *m_trigger_irq);

	UML_OR(block, I0, mem(&m_core->m_fiq), mem(&m_core->m_irq));
	UML_TEST(block, I0, 1);
	UML_RETc(block, uml::COND_NZ);

	UML_TEST(block, mem(&m_core->m_enable_irq), 1);
	UML_RETc(block, uml::COND_Z);

	UML_MOV(block, mem(&m_core->m_irq), 1);

	UML_MOV(block, mem(&m_core->m_saved_sb[0]), mem(&m_core->m_sb));
	UML_MOV(block, mem(&m_core->m_sb), mem(&m_core->m_saved_sb[1]));

	UML_MOV(block, I0, mem(&m_core->m_r[REG_SP]));
	UML_MOV(block, I1, mem(&m_core->m_r[REG_PC]));
	UML_CALLH(block, *m_mem_write);
	UML_SUB(block, I0, I0, 1);

	UML_MOV(block, I1, mem(&m_core->m_r[REG_SR]));
	UML_CALLH(block, *m_mem_write);
	UML_SUB(block, I0, I0, 1);

	UML_AND(block, mem(&m_core->m_r[REG_SP]), I0, 0x0000ffff);

	UML_ADD(block, I0, I2, 0x0000fff7);

	UML_CALLH(block, *m_mem_read);

	UML_MOV(block, mem(&m_core->m_r[REG_PC]), I1);
	UML_MOV(block, mem(&m_core->m_r[REG_SR]), 0);
	UML_HASHJMP(block, 0, I1, *m_nocode);
	UML_RET(block);

	block.end();
}

/*-------------------------------------------------
    generate_check_cycles - generate code to
    generate an exception if cycles are out
-------------------------------------------------*/

void unsp_device::generate_check_cycles(drcuml_block &block, compiler_state &compiler, uml::parameter param)
{
	UML_CMP(block, mem(&m_core->m_icount), 0);
	UML_EXHc(block, uml::COND_L, *m_out_of_cycles, param);
}

/*-------------------------------------------------
    generate_checksum_block - generate code to
    validate a sequence of opcodes
-------------------------------------------------*/

void unsp_device::generate_checksum_block(drcuml_block &block, compiler_state &compiler, const opcode_desc *seqhead, const opcode_desc *seqlast)
{
	const opcode_desc *curdesc;
	if (m_drcuml->logging())
	{
		block.append_comment("[Validation for %08X]", seqhead->pc);
	}

	/* full verification; sum up everything */
	void *memptr = m_program->get_write_ptr(seqhead->physpc);
	UML_LOAD(block, I0, memptr, 0, SIZE_WORD, SCALE_x2);
	uint32_t sum = seqhead->opptr.w[0];
	for (int i = 1; i < seqhead->length; i++)
	{
		UML_LOAD(block, I1, memptr, i, SIZE_WORD, SCALE_x2);
		UML_ADD(block, I0, I0, I1);
		sum += ((uint16_t*)memptr)[i];
	}
	for (curdesc = seqhead->next(); curdesc != seqlast->next(); curdesc = curdesc->next())
	{
		if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
		{
			memptr = m_program->get_write_ptr(curdesc->physpc);
			UML_LOAD(block, I1, memptr, 0, SIZE_WORD, SCALE_x2);
			UML_ADD(block, I0, I0, I1);
			sum += curdesc->opptr.w[0];
			for (int i = 1; i < curdesc->length; i++)
			{
				UML_LOAD(block, I1, memptr, i, SIZE_WORD, SCALE_x2);
				UML_ADD(block, I0, I0, I1);
				sum += ((uint16_t*)memptr)[i];
			}
		}
	}
	UML_CMP(block, I0, sum);
	UML_EXHc(block, COND_NE, *m_nocode, seqhead->pc);
}


/*-------------------------------------------------
    log_add_disasm_comment - add a comment
    including disassembly of a MIPS instruction
-------------------------------------------------*/

void unsp_device::log_add_disasm_comment(drcuml_block &block, uint32_t pc, uint32_t op)
{
	if (m_drcuml->logging())
	{
		block.append_comment("%08X: %08x", pc, op);
	}
}


/*------------------------------------------------------------------
    generate_branch
------------------------------------------------------------------*/

void unsp_device::generate_branch(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	if (desc->targetpc == BRANCH_TARGET_DYNAMIC)
	{
		UML_ROLAND(block, I0, mem(&m_core->m_r[REG_SR]), 16, 0x3f0000);
		UML_OR(block, mem(&m_core->m_jmpdest), I0, mem(&m_core->m_r[REG_PC]));
	}

	/* update the cycles and jump through the hash table to the target */
	if (desc->targetpc != BRANCH_TARGET_DYNAMIC)
	{
		UML_CALLH(block, *m_check_interrupts);
		UML_HASHJMP(block, 0, desc->targetpc, *m_nocode);
	}
	else
	{
		UML_CALLH(block, *m_check_interrupts);
		UML_HASHJMP(block, 0, mem(&m_core->m_jmpdest), *m_nocode);
	}
}


/*-------------------------------------------------
    generate_sequence_instruction - generate code
    for a single instruction in a sequence
-------------------------------------------------*/

void unsp_device::generate_sequence_instruction(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	/* add an entry for the log */
	if (m_drcuml->logging() && !(desc->flags & OPFLAG_VIRTUAL_NOOP))
		log_add_disasm_comment(block, desc->pc, desc->opptr.w[0]);

	/* set the PC map variable */
	UML_MAPVAR(block, MAPVAR_PC, desc->pc);

	/* if we are debugging, call the debugger */
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		//save_fast_iregs(block);
		UML_DEBUG(block, desc->pc);
	}

#if UNSP_LOG_REGS
	UML_CALLC(block, cfunc_log_regs, this);
#endif

	if (!(desc->flags & OPFLAG_VIRTUAL_NOOP))
	{
		/* compile the instruction */
		if (!generate_opcode(block, compiler, desc))
		{
			UML_ROLINS(block, mem(&m_core->m_r[REG_SR]), desc->pc, 16, 0x3f);
			UML_AND(block, mem(&m_core->m_r[REG_PC]), desc->pc, 0x0000ffff);
			UML_MOV(block, mem(&m_core->m_arg0), desc->opptr.w[0]);
			UML_CALLC(block, cfunc_unimplemented, this);
		}
	}
}


/*------------------------------------------------------------------
    generate_add_lpc - adds an offset to the long program counter
    comprised of PC and the low 6 bits of SR
------------------------------------------------------------------*/

void unsp_device::generate_add_lpc(drcuml_block &block, int32_t offset)
{
	UML_ROLAND(block, I0, mem(&m_core->m_r[REG_SR]), 16, 0x3f0000);
	UML_OR(block, I0, I0, mem(&m_core->m_r[REG_PC]));
	UML_ADD(block, I0, I0, offset);
	UML_ROLINS(block, mem(&m_core->m_r[REG_SR]), I0, 16, 0x3f);
	UML_AND(block, mem(&m_core->m_r[REG_PC]), I0, 0x0000ffff);
}


/*------------------------------------------------------------------
    generate_update_nzsc - perform a full flag update
------------------------------------------------------------------*/

void unsp_device::generate_update_nzsc(drcuml_block &block)
{
	UML_XOR(block, I1, I1, 0x0000ffff);
	UML_SEXT(block, I1, I1, SIZE_WORD);
	UML_SEXT(block, I2, I2, SIZE_WORD);
	UML_CMP(block, I2, I1);
	UML_SETc(block, uml::COND_L, I1);
	UML_ROLINS(block, mem(&m_core->m_r[REG_SR]), I1, UNSP_S_SHIFT, UNSP_S);

	UML_TEST(block, I3, 0x8000);
	UML_SETc(block, uml::COND_NZ, I1);
	UML_ROLINS(block, mem(&m_core->m_r[REG_SR]), I1, UNSP_N_SHIFT, UNSP_N);

	UML_TEST(block, I3, 0x10000);
	UML_SETc(block, uml::COND_NZ, I1);
	UML_ROLINS(block, mem(&m_core->m_r[REG_SR]), I1, UNSP_C_SHIFT, UNSP_C);

	UML_TEST(block, I3, 0x0000ffff);
	UML_SETc(block, uml::COND_Z, I1);
	UML_ROLINS(block, mem(&m_core->m_r[REG_SR]), I1, UNSP_Z_SHIFT, UNSP_Z);
}


/*------------------------------------------------------------------
    generate_update_nz - perform a partial flag update
------------------------------------------------------------------*/

void unsp_device::generate_update_nz(drcuml_block &block)
{
	UML_TEST(block, I3, 0x8000);
	UML_SETc(block, uml::COND_NZ, I1);
	UML_ROLINS(block, mem(&m_core->m_r[REG_SR]), I1, UNSP_N_SHIFT, UNSP_N);

	UML_AND(block, I2, I3, 0x0000ffff);
	UML_CMP(block, I2, 0);
	UML_SETc(block, uml::COND_E, I1);
	UML_ROLINS(block, mem(&m_core->m_r[REG_SR]), I1, UNSP_Z_SHIFT, UNSP_Z);
}


/*------------------------------------------------------------------
    generate_opcode - main handler which generates the UML for a
    single opcode
------------------------------------------------------------------*/

bool unsp_device::generate_f_group_opcode(drcuml_block& block, compiler_state& compiler, const opcode_desc* desc)
{
	return false;
}

bool unsp_newer_device::generate_f_group_opcode(drcuml_block& block, compiler_state& compiler, const opcode_desc* desc)
{
	// TODO: handle the extended opcodes
	return true;
}


bool unsp_device::generate_opcode(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	uint32_t op = (uint32_t)desc->opptr.w[0];

	generate_add_lpc(block, 1);

	const uint16_t op0 = (op >> 12) & 15;
	const uint16_t opa = (op >> 9) & 7;
	const uint16_t op1 = (op >> 6) & 7;
	const uint16_t opn = (op >> 3) & 7;
	const uint16_t opb = op & 7;

	const uint8_t lower_op = (op1 << 4) | op0;

	uml::code_label skip_branch = compiler.m_labelnum++;
	uml::code_label clear_fiq = compiler.m_labelnum++;
	uml::code_label clear_irq = compiler.m_labelnum++;
	uml::code_label reti_done = compiler.m_labelnum++;
	uml::code_label mul_opa_nohi = compiler.m_labelnum++;
	uml::code_label mul_opb_nohi = compiler.m_labelnum++;
	uml::code_label shift_no_sign = compiler.m_labelnum++;
	uml::code_label no_carry = compiler.m_labelnum++;

	if(op0 < 0xf && opa == 0x7 && op1 < 2)
	{
		const uint32_t opimm = op & 0x3f;
		switch(op0)
		{
			case 0: // JB
				UML_TEST(block, mem(&m_core->m_r[REG_SR]), UNSP_C);
				UML_JMPc(block, uml::COND_NZ, skip_branch);
				break;

			case 1: // JAE
				UML_TEST(block, mem(&m_core->m_r[REG_SR]), UNSP_C);
				UML_JMPc(block, uml::COND_Z, skip_branch);
				break;

			case 2: // JGE
				UML_TEST(block, mem(&m_core->m_r[REG_SR]), UNSP_S);
				UML_JMPc(block, uml::COND_NZ, skip_branch);
				break;

			case 3: // JL
				UML_TEST(block, mem(&m_core->m_r[REG_SR]), UNSP_S);
				UML_JMPc(block, uml::COND_Z, skip_branch);
				break;

			case 4: // JNE
				UML_TEST(block, mem(&m_core->m_r[REG_SR]), UNSP_Z);
				UML_JMPc(block, uml::COND_NZ, skip_branch);
				break;

			case 5: // JE
				UML_TEST(block, mem(&m_core->m_r[REG_SR]), UNSP_Z);
				UML_JMPc(block, uml::COND_Z, skip_branch);
				break;

			case 6: // JPL
				UML_TEST(block, mem(&m_core->m_r[REG_SR]), UNSP_N);
				UML_JMPc(block, uml::COND_NZ, skip_branch);
				break;

			case 7: // JMI
				UML_TEST(block, mem(&m_core->m_r[REG_SR]), UNSP_N);
				UML_JMPc(block, uml::COND_Z, skip_branch);
				break;

			case 8: // JBE
				UML_AND(block, I0, mem(&m_core->m_r[REG_SR]), UNSP_Z | UNSP_C);
				UML_CMP(block, I0, UNSP_C);
				UML_JMPc(block, uml::COND_E, skip_branch);
				break;

			case 9: // JA
				UML_AND(block, I0, mem(&m_core->m_r[REG_SR]), UNSP_Z | UNSP_C);
				UML_CMP(block, I0, UNSP_C);
				UML_JMPc(block, uml::COND_NE, skip_branch);
				break;

			case 10: // JLE
				UML_TEST(block, mem(&m_core->m_r[REG_SR]), UNSP_Z | UNSP_S);
				UML_JMPc(block, uml::COND_Z, skip_branch);
				break;

			case 11: // JG
				UML_TEST(block, mem(&m_core->m_r[REG_SR]), UNSP_Z);
				UML_JMPc(block, uml::COND_NZ, skip_branch);
				UML_TEST(block, mem(&m_core->m_r[REG_SR]), UNSP_S);
				UML_JMPc(block, uml::COND_NZ, skip_branch);
				break;

			case 14: // JMP
				UML_SUB(block, mem(&m_core->m_icount), mem(&m_core->m_icount), 4);
				UML_MOV(block, I0, desc->targetpc);
				UML_AND(block, mem(&m_core->m_r[REG_PC]), I0, 0x0000ffff);
				generate_branch(block, compiler, desc);
				return true;

			default:
				return false;
		}
		UML_SUB(block, mem(&m_core->m_icount), mem(&m_core->m_icount), 4);
		generate_add_lpc(block, (op1 == 0) ? opimm : (0 - opimm));
		generate_branch(block, compiler, desc);
		UML_LABEL(block, skip_branch);
		UML_SUB(block, mem(&m_core->m_icount), mem(&m_core->m_icount), 2);
		return true;
	}

	UML_SUB(block, mem(&m_core->m_icount), mem(&m_core->m_icount), desc->cycles);

	if (lower_op == 0x2d) // Push
	{
		uint32_t r0 = opn;
		uint32_t r1 = opa;
		UML_MOV(block, I0, mem(&m_core->m_r[opb]));
		while (r0)
		{
			UML_MOV(block, I1, mem(&m_core->m_r[r1]));
			UML_CALLH(block, *m_mem_write);
			UML_SUB(block, I0, I0, 1);
			UML_AND(block, mem(&m_core->m_r[opb]), I0, 0x0000ffff);
			r0--;
			r1--;
		}
		return true;
	}
	else if (lower_op == 0x29)
	{
		if (op == 0x9a98) // reti
		{
			UML_ADD(block, I0, mem(&m_core->m_r[REG_SP]), 1);
			UML_CALLH(block, *m_mem_read);
			UML_MOV(block, mem(&m_core->m_r[REG_SR]), I1);

			UML_ADD(block, I0, I0, 1);
			UML_CALLH(block, *m_mem_read);
			UML_MOV(block, mem(&m_core->m_r[REG_PC]), I1);

			UML_AND(block, mem(&m_core->m_r[REG_SP]), I0, 0x0000ffff);

			UML_TEST(block, mem(&m_core->m_fiq), 1);
			UML_JMPc(block, uml::COND_NZ, clear_fiq);
			UML_TEST(block, mem(&m_core->m_irq), 1);
			UML_JMPc(block, uml::COND_NZ, clear_irq);
			UML_JMP(block, reti_done);

			UML_LABEL(block, clear_fiq);
			UML_MOV(block, mem(&m_core->m_fiq), 0);
			UML_MOV(block, mem(&m_core->m_saved_sb[2]), mem(&m_core->m_sb));
			UML_LOAD(block, mem(&m_core->m_sb), (void*)m_core->m_saved_sb, mem(&m_core->m_irq), SIZE_DWORD, SCALE_x4);
			UML_JMP(block, reti_done);

			UML_LABEL(block, clear_irq);
			UML_MOV(block, mem(&m_core->m_irq), 0);
			UML_MOV(block, mem(&m_core->m_saved_sb[1]), mem(&m_core->m_sb));
			UML_MOV(block, mem(&m_core->m_sb), mem(&m_core->m_saved_sb[0]));

			UML_LABEL(block, reti_done);
			UML_MOV(block, mem(&m_core->m_curirq), 0);
			generate_branch(block, compiler, desc);
		}
		else // pop
		{
			uint32_t r0 = opn;
			uint32_t r1 = opa;
			bool do_branch = false;

			UML_MOV(block, I0, mem(&m_core->m_r[opb]));
			while (r0)
			{
				r1++;
				UML_ADD(block, I0, I0, 1);
				UML_AND(block, mem(&m_core->m_r[opb]), I0, 0x0000ffff);
				UML_CALLH(block, *m_mem_read);
				UML_MOV(block, mem(&m_core->m_r[r1]), I1);
				if (r1 == REG_PC)
					do_branch = true;
				r0--;
			}
			if (do_branch)
				generate_branch(block, compiler, desc);
		}
		return true;
	}
	else if (op0 == 0xf)
	{
		switch (op1)
		{
			case 0x00: // Multiply, Unsigned * Signed
				if(opn == 1 && opa != 7)
				{
					UML_MOV(block, I0, mem(&m_core->m_r[opa]));
					UML_MOV(block, I1, mem(&m_core->m_r[opb]));
					UML_MULU(block, I2, I2, I0, I1);

					UML_TEST(block, I1, 0x00008000);
					UML_JMPc(block, uml::COND_Z, mul_opb_nohi);
					UML_SHL(block, I0, I0, 16);
					UML_SUB(block, I2, I2, I0);

					UML_LABEL(block, mul_opb_nohi);
					UML_SHR(block, mem(&m_core->m_r[REG_R4]), I2, 16);
					UML_AND(block, mem(&m_core->m_r[REG_R3]), I2, 0x0000ffff);
					return true;
				}
				return false;

			case 0x01: // Call
				if(!(opa & 1))
				{
					generate_add_lpc(block, 1);

					UML_MOV(block, I0, mem(&m_core->m_r[REG_SP]));

					UML_MOV(block, I1, mem(&m_core->m_r[REG_PC]));
					UML_CALLH(block, *m_mem_write);
					UML_SUB(block, I0, I0, 1);

					UML_MOV(block, I1, mem(&m_core->m_r[REG_SR]));
					UML_CALLH(block, *m_mem_write);
					UML_SUB(block, I0, I0, 1);

					UML_AND(block, mem(&m_core->m_r[REG_SP]), I0, 0x0000ffff);

					UML_MOV(block, I0, desc->targetpc);
					UML_AND(block, mem(&m_core->m_r[REG_PC]), I0, 0x0000ffff);
					UML_ROLINS(block, mem(&m_core->m_r[REG_SR]), I0, 16, 0x3f);
					generate_branch(block, compiler, desc);
					return true;
				}
				return false;

			case 0x02: // Far Jump
				if (opa == 7)
				{
					UML_MOV(block, I0, desc->targetpc);
					UML_AND(block, mem(&m_core->m_r[REG_PC]), I0, 0x0000ffff);
					UML_ROLINS(block, mem(&m_core->m_r[REG_SR]), I0, 16, 0x3f);
					generate_branch(block, compiler, desc);
					return true;
				}
				return false;

			case 0x04: // Multiply, Signed * Signed
				if(opn == 1 && opa != 7)
				{
					UML_MOV(block, I0, mem(&m_core->m_r[opa]));
					UML_MOV(block, I1, mem(&m_core->m_r[opb]));
					UML_MULU(block, I2, I2, I0, I1);

					UML_TEST(block, I1, 0x00008000);
					UML_JMPc(block, uml::COND_Z, mul_opb_nohi);
					UML_SHL(block, I0, I0, 16);
					UML_SUB(block, I2, I2, I0);

					UML_LABEL(block, mul_opb_nohi);
					UML_TEST(block, I0, 0x00008000);
					UML_JMPc(block, uml::COND_Z, mul_opa_nohi);
					UML_SHL(block, I1, I1, 16);
					UML_SUB(block, I2, I2, I1);

					UML_LABEL(block, mul_opa_nohi);
					UML_SHR(block, mem(&m_core->m_r[REG_R4]), I2, 16);
					UML_AND(block, mem(&m_core->m_r[REG_R3]), I2, 0x0000ffff);
					return true;
				}
				return false;

			case 0x05: // Interrupt flags
				switch(op & 0x3f)
				{
					case 0:
						UML_MOV(block, mem(&m_core->m_enable_irq), 0);
						UML_MOV(block, mem(&m_core->m_enable_fiq), 0);
						break;

					case 1:
						UML_MOV(block, mem(&m_core->m_enable_irq), 1);
						UML_MOV(block, mem(&m_core->m_enable_fiq), 0);
						break;

					case 2:
						UML_MOV(block, mem(&m_core->m_enable_irq), 0);
						UML_MOV(block, mem(&m_core->m_enable_fiq), 1);
						break;

					case 3:
						UML_MOV(block, mem(&m_core->m_enable_irq), 1);
						UML_MOV(block, mem(&m_core->m_enable_fiq), 1);
						break;

					case 8:
						UML_MOV(block, mem(&m_core->m_enable_irq), 0);
						break;

					case 9:
						UML_MOV(block, mem(&m_core->m_enable_irq), 1);
						break;

					case 12:
						UML_MOV(block, mem(&m_core->m_enable_fiq), 0);
						break;

					case 14:
						UML_MOV(block, mem(&m_core->m_enable_fiq), 1);
						break;

					case 37:
						// nop
						break;

					default:
						return false;
				}
				return true;

			default:
				return false;
		}
	}

	// At this point, we should be dealing solely with ALU ops.
	UML_MOV(block, I2, mem(&m_core->m_r[opa]));

	switch (op1)
	{
		case 0x00: // r, [bp+imm6]
			UML_ADD(block, I0, mem(&m_core->m_r[REG_BP]), op & 0x3f);
			UML_AND(block, I0, I0, 0x0000ffff);
			if (op0 != 0x0d)
				UML_CALLH(block, *m_mem_read);
			break;

		case 0x01: // r, imm6
			UML_MOV(block, I1, op & 0x3f);
			break;

		case 0x03: // Indirect
		{
			const uint8_t lsbits = opn & 3;
			if (opn & 4)
			{
				switch (lsbits)
				{
					case 0: // r, [<ds:>r]
						UML_ROLAND(block, I0, mem(&m_core->m_r[REG_SR]), 6, 0x3f0000);
						UML_OR(block, I0, I0, mem(&m_core->m_r[opb]));
						if (op0 != 0x0d)
							UML_CALLH(block, *m_mem_read);
						break;

					case 1: // r, [<ds:>r--]
						UML_ROLAND(block, I0, mem(&m_core->m_r[REG_SR]), 6, 0x3f0000);
						UML_OR(block, I0, I0, mem(&m_core->m_r[opb]));
						if (op0 != 0x0d)
							UML_CALLH(block, *m_mem_read);
						UML_SUB(block, I3, I0, 1);
						UML_ROLINS(block, mem(&m_core->m_r[REG_SR]), I3, 32-6, 0x0000fc00);
						UML_AND(block, mem(&m_core->m_r[opb]), I3, 0x0000ffff);
						break;

					case 2: // r, [<ds:>r++]
						UML_ROLAND(block, I0, mem(&m_core->m_r[REG_SR]), 6, 0x3f0000);
						UML_OR(block, I0, I0, mem(&m_core->m_r[opb]));
						if (op0 != 0x0d)
							UML_CALLH(block, *m_mem_read);
						UML_ADD(block, I3, I0, 1);
						UML_ROLINS(block, mem(&m_core->m_r[REG_SR]), I3, 32-6, 0x0000fc00);
						UML_AND(block, mem(&m_core->m_r[opb]), I3, 0x0000ffff);
						break;

					case 3: // r, [<ds:>++r]
						UML_ROLAND(block, I0, mem(&m_core->m_r[REG_SR]), 6, 0x3f0000);
						UML_OR(block, I0, I0, mem(&m_core->m_r[opb]));
						UML_ADD(block, I0, I0, 1);
						UML_ROLINS(block, mem(&m_core->m_r[REG_SR]), I0, 32-6, 0x0000fc00);
						UML_AND(block, mem(&m_core->m_r[opb]), I0, 0x0000ffff);
						if (op0 != 0x0d)
							UML_CALLH(block, *m_mem_read);
						break;

					default:
						// can't happen
						break;
				}
			}
			else
			{
				switch (lsbits)
				{
					case 0: // r, [r]
						UML_MOV(block, I0, mem(&m_core->m_r[opb]));
						if (op0 != 0x0d)
							UML_CALLH(block, *m_mem_read);
						break;

					case 1: // r, [r--]
						UML_MOV(block, I0, mem(&m_core->m_r[opb]));
						if (op0 != 0x0d)
							UML_CALLH(block, *m_mem_read);
						UML_SUB(block, I3, I0, 1);
						UML_AND(block, mem(&m_core->m_r[opb]), I3, 0x0000ffff);
						break;

					case 2: // r, [r++]
						UML_MOV(block, I0, mem(&m_core->m_r[opb]));
						if (op0 != 0x0d)
							UML_CALLH(block, *m_mem_read);
						UML_ADD(block, I3, I0, 1);
						UML_AND(block, mem(&m_core->m_r[opb]), I3, 0x0000ffff);
						break;

					case 3: // r, [++r]
						UML_ADD(block, I0, mem(&m_core->m_r[opb]), 1);
						UML_AND(block, I0, I0, 0x0000ffff);
						UML_MOV(block, mem(&m_core->m_r[opb]), I0);
						if (op0 != 0x0d)
							UML_CALLH(block, *m_mem_read);
						break;
					default:
						// can't happen
						break;
				}
			}
			break;
		}

		case 0x04: // 16-bit ops
			switch (opn)
			{
				case 0x00: // r
					UML_MOV(block, I1, mem(&m_core->m_r[opb]));
					break;

				case 0x01: // imm16
				{
					UML_MOV(block, I2, mem(&m_core->m_r[opb]));
					const uint16_t r1 = m_pr16(desc->pc + 1);
					generate_add_lpc(block, 1);
					UML_MOV(block, I1, r1);
					break;
				}

				case 0x02: // [imm16]
				{
					UML_MOV(block, I2, mem(&m_core->m_r[opb]));
					const uint16_t r1 = m_pr16(desc->pc + 1);
					generate_add_lpc(block, 1);
					UML_MOV(block, I0, r1);
					if (op0 != 0x0d)
						UML_CALLH(block, *m_mem_read);
					break;
				}

				case 0x03: // store [imm16], r
				{
					UML_MOV(block, I1, I2);
					UML_MOV(block, I2, mem(&m_core->m_r[opb]));
					const uint16_t r2 = m_pr16(desc->pc + 1);
					generate_add_lpc(block, 1);
					UML_MOV(block, I0, r2);
					break;
				}

				default: // Shifted ops
				{
					UML_SHL(block, I0, mem(&m_core->m_r[opb]), 4);
					UML_OR(block, I0, I0, mem(&m_core->m_sb));
					UML_TEST(block, I0, 0x80000);
					UML_JMPc(block, uml::COND_Z, shift_no_sign);
					UML_OR(block, I0, I0, 0xf00000);

					UML_LABEL(block, shift_no_sign);
					UML_SHR(block, I0, I0, opn - 3);
					UML_AND(block, mem(&m_core->m_sb), I0, 0xf);
					UML_SHR(block, I1, I0, 4);
					UML_AND(block, I1, I1, 0x0000ffff);
					break;
				}
			}
			break;

		case 0x05: // More shifted ops
			if (opn & 4) // Shift right
			{
				UML_SHL(block, I0, mem(&m_core->m_r[opb]), 4);
				UML_OR(block, I0, I0, mem(&m_core->m_sb));
				UML_SHR(block, I0, I0, opn - 3);
				UML_AND(block, mem(&m_core->m_sb), I0, 0xf);
				UML_SHR(block, I1, I0, 4);
				UML_AND(block, I1, I1, 0x0000ffff);
			}
			else // Shift left
			{
				UML_SHL(block, I0, mem(&m_core->m_sb), 16);
				UML_OR(block, I0, I0, mem(&m_core->m_r[opb]));
				UML_SHL(block, I0, I0, opn + 1);
				UML_SHR(block, I1, I0, 16);
				UML_AND(block, mem(&m_core->m_sb), I1, 0xf);
				UML_AND(block, I1, I0, 0x0000ffff);
			}
			break;

		case 0x06: // Rotated ops
		{
			UML_SHL(block, I0, mem(&m_core->m_sb), 16);
			UML_OR(block, I0, I0, mem(&m_core->m_r[opb]));
			UML_SHL(block, I0, I0, 4);
			UML_OR(block, I0, I0, mem(&m_core->m_sb));
			if (opn & 4) // Rotate right
			{
				UML_SHR(block, I0, I0, opn - 3);
				UML_AND(block, mem(&m_core->m_sb), I0, 0xf);
			}
			else // Rotate left
			{
				UML_SHL(block, I0, I0, opn + 1);
				UML_SHR(block, I1, I0, 20);
				UML_AND(block, mem(&m_core->m_sb), I1, 0xf);
			}
			UML_SHR(block, I1, I0, 4);
			UML_AND(block, I1, I1, 0x0000ffff);
			break;
		}

		case 0x07: // Direct 8
			UML_MOV(block, I0, op & 0x3f);
			UML_CALLH(block, *m_mem_read);
			break;

		default:
			break;
	}

	switch (op0)
	{
		case 0x00: // Add
			UML_ADD(block, I3, I2, I1);
			if (opa != 7)
				generate_update_nzsc(block);
			break;

		case 0x01: // Add w/ carry
			UML_ROLAND(block, I3, mem(&m_core->m_r[REG_SR]), 32-UNSP_C_SHIFT, 1);
			UML_ADD(block, I3, I3, I2);
			UML_ADD(block, I3, I3, I1);
			if (opa != 7)
				generate_update_nzsc(block);
			break;

		case 0x02: // Subtract
			UML_XOR(block, I1, I1, 0x0000ffff);
			UML_ADD(block, I3, I1, I2);
			UML_ADD(block, I3, I3, 1);
			if (opa != 7)
				generate_update_nzsc(block);
			break;

		case 0x03: // Subtract w/ carry
			UML_XOR(block, I1, I1, 0x0000ffff);
			UML_ADD(block, I3, I1, I2);
			UML_TEST(block, mem(&m_core->m_r[REG_SR]), UNSP_C);
			UML_JMPc(block, uml::COND_Z, no_carry);
			UML_ADD(block, I3, I3, 1);

			UML_LABEL(block, no_carry);
			if (opa != 7)
				generate_update_nzsc(block);
			break;

		case 0x04: // Compare
			UML_XOR(block, I1, I1, 0x0000ffff);
			UML_ADD(block, I3, I1, I2);
			UML_ADD(block, I3, I3, 1);
			if (opa != 7)
				generate_update_nzsc(block);
			return true;

		case 0x06: // Negate
			UML_SUB(block, I3, 0, I1);
			if (opa != 7)
				generate_update_nz(block);
			break;

		case 0x08: // XOR
			UML_XOR(block, I3, I2, I1);
			if (opa != 7)
				generate_update_nz(block);
			break;

		case 0x09: // Load
			UML_MOV(block, I3, I1);
			if (opa != 7)
				generate_update_nz(block);
			break;

		case 0x0a: // OR
			UML_OR(block, I3, I2, I1);
			if (opa != 7)
				generate_update_nz(block);
			break;

		case 0x0b: // AND
			UML_AND(block, I3, I2, I1);
			if (opa != 7)
				generate_update_nz(block);
			break;

		case 0x0c: // Test
			UML_AND(block, I3, I2, I1);
			if (opa != 7)
				generate_update_nz(block);
			return true;

		case 0x0d: // Store
			UML_MOV(block, I1, I2);
			UML_CALLH(block, *m_mem_write);
			return true;

		case 0x0f: // Extended
			return generate_f_group_opcode(block, compiler, desc);

		default:
			return false;
	}

	if (op1 == 0x04 && opn == 0x03)
	{
		UML_MOV(block, I1, I3);
		UML_CALLH(block, *m_mem_write);
		return true;
	}

	UML_AND(block, mem(&m_core->m_r[opa]), I3, 0x0000ffff);
	if (opa == REG_PC)
		generate_branch(block, compiler, desc);
	return true;
}

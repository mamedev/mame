// license:BSD-3-Clause
// copyright-holders:Philip Bennett

/******************************************************************************

    DSPP UML recompiler core

******************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "dspp.h"
#include "dsppfe.h"
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"

using namespace uml;

#define USE_SWAPDQ  0


// map variables
#define MAPVAR_PC                       M0
#define MAPVAR_CYCLES                   M1
#define MAPVAR_ACC						M2

// exit codes
#define EXECUTE_OUT_OF_CYCLES           0
#define EXECUTE_MISSING_CODE            1
#define EXECUTE_UNMAPPED_CODE           2
#define EXECUTE_RESET_CACHE             3

#define FLAG_C(reg)                     mem(&m_core->m_flag_carry)
#define FLAG_Z(reg)                     mem(&m_core->m_flag_zero)
#define FLAG_N(reg)                     mem(&m_core->m_flag_neg)
#define FLAG_V(reg)                     mem(&m_core->m_flag_over)
#define FLAG_X(reg)                     mem(&m_core->m_flag_exact)

inline void dspp_device::alloc_handle(drcuml_state *drcuml, code_handle **handleptr, const char *name)
{
	if (*handleptr == nullptr)
		*handleptr = drcuml->handle_alloc(name);
}

static inline uint32_t epc(const opcode_desc *desc)
{
	return desc->pc;
}


#if 0
static void cfunc_unimplemented(void *param)
{
	dspp_device *dspp = (dspp_device *)param;
	dspp->cfunc_unimplemented();
}
#endif

void dspp_device::cfunc_unimplemented()
{
//	uint64_t op = m_core->m_arg0;
//	fatalerror("PC=%08X: Unimplemented op %04X%08X\n", m_core->m_pc, (uint32_t)(op >> 32), (uint32_t)(op));
}


/*-------------------------------------------------
load_fast_iregs - load any fast integer
registers
-------------------------------------------------*/

inline void dspp_device::load_fast_iregs(drcuml_block &block)
{
#if 0 // TODO
	for (uint32_t regnum = 0; regnum < ARRAY_LENGTH(m_regmap); regnum++)
	{
		if (m_regmap[regnum].is_int_register())
		{
			UML_MOV(block, ireg(m_regmap[regnum].ireg() - REG_I0), mem(&m_core->r[regnum]));
		}
	}
#endif
}


/*-------------------------------------------------
save_fast_iregs - save any fast integer
registers
-------------------------------------------------*/

void dspp_device::save_fast_iregs(drcuml_block &block)
{
#if 0 // TODO
	int regnum;

	for (regnum = 0; regnum < ARRAY_LENGTH(m_regmap); regnum++)
	{
		if (m_regmap[regnum].is_int_register())
		{
			UML_MOV(block, mem(&m_core->r[regnum]), ireg(m_regmap[regnum].ireg() - REG_I0));
		}
	}
#endif
}

#if 0
void dspp_device::static_generate_memory_accessor(MEM_ACCESSOR_TYPE type, const char *name, code_handle *&handleptr)
{
	// I0 = read/write data
	// I1 = address

	drcuml_block &block = m_drcuml->begin_block(1024);

	// add a global entry for this
	alloc_handle(m_drcuml.get(), &handleptr, name);
	UML_HANDLE(block, *handleptr);                                                          // handle  *handleptr

	switch (type)
	{
		case MEM_ACCESSOR_PM_READ48:
			UML_SHL(block, I1, I1, 3);
			UML_DREAD(block, I0, I1, SIZE_QWORD, SPACE_PROGRAM);
			break;

		case MEM_ACCESSOR_PM_WRITE48:
			UML_SHL(block, I1, I1, 3);
			UML_DWRITE(block, I1, I0, SIZE_QWORD, SPACE_PROGRAM);
			UML_MOV(block, mem(&m_core->force_recompile), 1);
			break;

		case MEM_ACCESSOR_PM_READ32:
			UML_SHL(block, I1, I1, 3);
			UML_READ(block, I0, I1, SIZE_DWORD, SPACE_PROGRAM);
			break;

		case MEM_ACCESSOR_PM_WRITE32:
			UML_SHL(block, I1, I1, 3);
			UML_WRITE(block, I1, I0, SIZE_DWORD, SPACE_PROGRAM);
			UML_MOV(block, mem(&m_core->force_recompile), 1);
			break;

		case MEM_ACCESSOR_DM_READ32:
			UML_SHL(block, I1, I1, 2);
			UML_READ(block, I0, I1, SIZE_DWORD, SPACE_DATA);
			break;

		case MEM_ACCESSOR_DM_WRITE32:
			UML_SHL(block, I1, I1, 2);
			UML_WRITE(block, I1, I0, SIZE_DWORD, SPACE_DATA);
			break;
	}

	UML_RET(block);

	block.end();
}
#endif


void dspp_device::execute_run_drc()
{
	drcuml_state *drcuml = m_drcuml.get();
	int execute_result;

	if (m_cache_dirty)
		flush_cache();

	m_cache_dirty = false;

	do
	{
		execute_result = drcuml->execute(*m_entry);

		/* if we need to recompile, do it */
		if (execute_result == EXECUTE_MISSING_CODE)
		{
			compile_block(m_core->m_pc);
		}
		else if (execute_result == EXECUTE_UNMAPPED_CODE)
		{
			fatalerror("Attempted to execute unmapped code at PC=%08X\n", m_core->m_pc);
		}
		else if (execute_result == EXECUTE_RESET_CACHE)
		{
			flush_cache();
		}
	} while (execute_result != EXECUTE_OUT_OF_CYCLES);
}

void dspp_device::compile_block(offs_t pc)
{
	drcuml_state *drcuml = m_drcuml.get();
	compiler_state compiler = { 0 };
	const opcode_desc *seqhead, *seqlast;
	const opcode_desc *desclist;
	int override = false;

	g_profiler.start(PROFILER_DRC_COMPILE);

	/* get a description of this sequence */
	desclist = m_drcfe->describe_code(pc);

	bool succeeded = false;
	while (!succeeded)
	{
		try
		{
			/* start the block */
			drcuml_block &block = drcuml->begin_block(4096);

			/* loop until we get through all instruction sequences */
			for (seqhead = desclist; seqhead != nullptr; seqhead = seqlast->next())
			{
				const opcode_desc *curdesc;
				uint32_t nextpc;

				/* add a code log entry */
				if (drcuml->logging())
					block.append_comment("-------------------------");                 // comment

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
				generate_update_cycles(block, &compiler, nextpc);            // <subtract cycles>

				/* if the last instruction can change modes, use a variable mode; otherwise, assume the same mode */
				if (seqlast->next() == nullptr || seqlast->next()->pc != nextpc)
					UML_HASHJMP(block, 0, nextpc, *m_nocode);          // hashjmp <mode>,nextpc,nocode
			}

			/* end the sequence */
			block.end();
			g_profiler.stop();
			succeeded = true;
		}
		catch (drcuml_block::abort_compilation &)
		{
			flush_cache();
		}
	}
}

void dspp_device::generate_checksum_block(drcuml_block &block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast)
{
	const opcode_desc *curdesc;
	if (m_drcuml->logging())
		block.append_comment("[Validation for %08X]", seqhead->pc);                // comment

	/* loose verify or single instruction: just compare and fail */
	if (!(m_drcoptions & DSPPDRC_STRICT_VERIFY) || seqhead->next() == nullptr)
	{
		if (!(seqhead->flags & OPFLAG_VIRTUAL_NOOP))
		{
			uint32_t sum = seqhead->opptr.l[0];
			uint32_t addr = seqhead->physpc;
			const void *base = m_codeptr(addr);
			UML_LOAD(block, I0, base, 0, SIZE_DWORD, SCALE_x4);         // load    i0,base,0,dword

			if (seqhead->delay.first() != nullptr && seqhead->physpc != seqhead->delay.first()->physpc)
			{
				addr = seqhead->delay.first()->physpc;
				base = m_codeptr(addr);
				assert(base != nullptr);
				UML_LOAD(block, I1, base, 0, SIZE_DWORD, SCALE_x4);                 // load    i1,base,dword
				UML_ADD(block, I0, I0, I1);                     // add     i0,i0,i1

				sum += seqhead->delay.first()->opptr.l[0];
			}

			UML_CMP(block, I0, sum);                                    // cmp     i0,opptr[0]
			UML_EXHc(block, COND_NE, *m_nocode, epc(seqhead));       // exne    nocode,seqhead->pc
		}
	}

	/* full verification; sum up everything */
	else
	{
#if 0
		for (curdesc = seqhead->next(); curdesc != seqlast->next(); curdesc = curdesc->next())
			if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
			{
				void *base = m_code_direct->read_ptr(seqhead->physpc);
				UML_LOAD(block, I0, base, 0, SIZE_DWORD, SCALE_x4);     // load    i0,base,0,dword
				UML_CMP(block, I0, curdesc->opptr.l[0]);                    // cmp     i0,opptr[0]
				UML_EXHc(block, COND_NE, *m_nocode, epc(seqhead));   // exne    nocode,seqhead->pc
			}
#else
		uint32_t sum = 0;
		uint32_t addr = seqhead->physpc;
		const void *base = m_codeptr(addr);
		UML_LOAD(block, I0, base, 0, SIZE_DWORD, SCALE_x4);             // load    i0,base,0,dword
		sum += seqhead->opptr.l[0];
		for (curdesc = seqhead->next(); curdesc != seqlast->next(); curdesc = curdesc->next())
			if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
			{
				addr = curdesc->physpc;
				base = m_codeptr(addr);
				assert(base != nullptr);
				UML_LOAD(block, I1, base, 0, SIZE_DWORD, SCALE_x4);     // load    i1,base,dword
				UML_ADD(block, I0, I0, I1);                         // add     i0,i0,i1
				sum += curdesc->opptr.l[0];

				if (curdesc->delay.first() != nullptr && (curdesc == seqlast || (curdesc->next() != nullptr && curdesc->next()->physpc != curdesc->delay.first()->physpc)))
				{
					addr = curdesc->delay.first()->physpc;
					base = m_codeptr(addr);
					assert(base != nullptr);
					UML_LOAD(block, I1, base, 0, SIZE_DWORD, SCALE_x4); // load    i1,base,dword
					UML_ADD(block, I0, I0, I1);                     // add     i0,i0,i1
					sum += curdesc->delay.first()->opptr.l[0];
				}
			}
		UML_CMP(block, I0, sum);                                            // cmp     i0,sum
		UML_EXHc(block, COND_NE, *m_nocode, epc(seqhead));           // exne    nocode,seqhead->pc
#endif
	}
}


void dspp_device::flush_cache()
{
	/* empty the transient cache contents */
	m_drcuml->reset();

	try
	{
		// generate the entry point and out-of-cycles handlers
		static_generate_entry_point();
		static_generate_nocode_handler();
		static_generate_out_of_cycles();

#if 0
		// generate utility functions
		static_generate_push_pc();
		static_generate_pop_pc();
		static_generate_push_loop();
		static_generate_pop_loop();
		static_generate_push_status();
		static_generate_pop_status();
		static_generate_mode1_ops();

		// generate exception handlers
		static_generate_exception(EXCEPTION_INTERRUPT, "exception_interrupt");

		// generate memory accessors
		static_generate_memory_accessor(MEM_ACCESSOR_PM_READ48, "pm_read48", m_pm_read48);
		static_generate_memory_accessor(MEM_ACCESSOR_PM_WRITE48, "pm_write48", m_pm_write48);
		static_generate_memory_accessor(MEM_ACCESSOR_PM_READ32, "pm_read32", m_pm_read32);
		static_generate_memory_accessor(MEM_ACCESSOR_PM_WRITE32, "pm_write32", m_pm_write32);
		static_generate_memory_accessor(MEM_ACCESSOR_DM_READ32, "dm_read32", m_dm_read32);
		static_generate_memory_accessor(MEM_ACCESSOR_DM_WRITE32, "dm_write32", m_dm_write32);
#endif
	}
	catch (drcuml_block::abort_compilation &)
	{
		fatalerror("Error generating dspp static handlers\n");
	}
}


void dspp_device::static_generate_entry_point()
{
	/* begin generating */
	drcuml_block &block = m_drcuml->begin_block(20);

	/* forward references */
	alloc_handle(m_drcuml.get(), &m_nocode, "nocode");
//	alloc_handle(m_drcuml.get(), &m_exception[EXCEPTION_INTERRUPT], "exception_interrupt");

	alloc_handle(m_drcuml.get(), &m_entry, "entry");
	UML_HANDLE(block, *m_entry);                                                            // handle  entry

	load_fast_iregs(block);                                                                 // <load fastregs>

#if 0 // TODO: No interrupts?
	/* check for interrupts */
	UML_CMP(block, mem(&m_core->irq_pending), 0);                                       // cmp     [irq_pending],0
	UML_JMPc(block, COND_E, skip);                                                      // je      skip
	UML_CMP(block, mem(&m_core->interrupt_active), 0);                                  // cmp     [interrupt_active],0
	UML_JMPc(block, COND_NE, skip);                                                     // jne     skip
	UML_TEST(block, mem(&m_core->irq_pending), IMASK);                                  // test    [irq_pending],IMASK
	UML_JMPc(block, COND_Z, skip);                                                      // jz      skip
	UML_TEST(block, mem(&m_core->mode1), MODE1_IRPTEN);                                 // test    MODE1,MODE1_IRPTEN
	UML_JMPc(block, COND_Z, skip);                                                      // jz      skip
#endif

//	UML_MOV(block, I0, mem(&m_core->m_pc));                                               // mov     i0,nextpc
//	UML_MOV(block, I1, 0);                                                              // mov     i1,0
//	UML_CALLH(block, *m_exception[EXCEPTION_INTERRUPT]);                                // callh   m_exception[EXCEPTION_INTERRUPT]

//	UML_LABEL(block, skip);

	/* generate a hash jump via the current mode and PC */
	UML_HASHJMP(block, 0, mem(&m_core->m_pc), *m_nocode);   // hashjmp <mode>,<pc>,nocode

	block.end();
}


void dspp_device::static_generate_nocode_handler()
{
	/* begin generating */
	drcuml_block &block = m_drcuml->begin_block(10);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(m_drcuml.get(), &m_nocode, "nocode");
	UML_HANDLE(block, *m_nocode);                                                           // handle  nocode
	UML_GETEXP(block, I0);                                                                  // getexp  i0
	UML_MOV(block, mem(&m_core->m_pc), I0);                                                   // mov     [pc],i0
	save_fast_iregs(block);                                                                 // <save fastregs>
	UML_EXIT(block, EXECUTE_MISSING_CODE);                                                  // exit    EXECUTE_MISSING_CODE

	block.end();
}

void dspp_device::static_generate_out_of_cycles()
{
	/* begin generating */
	drcuml_block &block = m_drcuml->begin_block(10);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(m_drcuml.get(), &m_out_of_cycles, "out_of_cycles");
	UML_HANDLE(block, *m_out_of_cycles);                                                    // handle  out_of_cycles
	UML_GETEXP(block, I0);                                                                  // getexp  i0
	UML_MOV(block, mem(&m_core->m_pc), I0);                                                   // mov     <pc>,i0
	save_fast_iregs(block);                                                                 // <save fastregs>
	UML_EXIT(block, EXECUTE_OUT_OF_CYCLES);                                                 // exit    EXECUTE_OUT_OF_CYCLES

	block.end();
}


void dspp_device::generate_sequence_instruction(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc)
{
	/* add an entry for the log */
//  if (m_drcuml->logging() && !(desc->flags & OPFLAG_VIRTUAL_NOOP))
//      log_add_disasm_comment(block, desc->pc, desc->opptr.l[0]);

	/* set the PC map variable */
	UML_MAPVAR(block, MAPVAR_PC, desc->pc);                                                 // mapvar  PC,desc->pc

																							/* accumulate total cycles */
	compiler->cycles += desc->cycles;

	/* update the icount map variable */
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);                                     // mapvar  CYCLES,compiler->cycles

	/* if we are debugging, call the debugger */
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		UML_MOV(block, mem(&m_core->m_pc), desc->pc);                                         // mov     [pc],desc->pc
		save_fast_iregs(block);                                                             // <save fastregs>
		UML_DEBUG(block, desc->pc);                                                         // debug   desc->pc
	}

	/* if we hit an unmapped address, fatal error */
	if (desc->flags & OPFLAG_COMPILER_UNMAPPED)
	{
		UML_MOV(block, mem(&m_core->m_pc), desc->pc);                                         // mov     [pc],desc->pc
		save_fast_iregs(block);                                                             // <save fastregs>
		UML_EXIT(block, EXECUTE_UNMAPPED_CODE);                                             // exit    EXECUTE_UNMAPPED_CODE
	}

	/* if this is an invalid opcode, generate the exception now */
//  if (desc->flags & OPFLAG_INVALID_OPCODE)
//      UML_EXH(block, *m_exception[EXCEPTION_PROGRAM], 0x80000);                           // exh    exception_program,0x80000

	/* unless this is a virtual no-op, it's a regular instruction */
	if (!(desc->flags & OPFLAG_VIRTUAL_NOOP))
	{
		/* compile the instruction */
		//if (!generate_opcode(block, compiler, desc))
		{
			UML_MOV(block, mem(&m_core->m_pc), desc->pc);                                     // mov     [pc],desc->pc
			UML_DMOV(block, mem(&m_core->m_arg0), desc->opptr.q[0]);                         // dmov    [m_arg0],*desc->opptr.q // FIXME
//			UML_CALLC(block, cfunc_unimplemented, this);                                    // callc   cfunc_unimplemented,ppc
		}
	}
}

void dspp_device::generate_update_cycles(drcuml_block &block, compiler_state *compiler, uml::parameter param)
{
#if 0 // No interrupts
	if (compiler->checkints)
	{
		code_label skip = compiler->labelnum++;
		compiler->checkints = false;

		UML_CMP(block, mem(&m_core->irq_pending), 0);                                       // cmp     [irq_pending],0
		UML_JMPc(block, COND_E, skip);                                                      // je      skip
		UML_CMP(block, mem(&m_core->interrupt_active), 0);                                  // cmp     [interrupt_active],0
		UML_JMPc(block, COND_NE, skip);                                                     // jne     skip
		UML_TEST(block, mem(&m_core->irq_pending), IMASK);                                  // test    [irq_pending],IMASK
		UML_JMPc(block, COND_Z, skip);                                                      // jz      skip
		UML_TEST(block, mem(&m_core->mode1), MODE1_IRPTEN);                                 // test    MODE1,MODE1_IRPTEN
		UML_JMPc(block, COND_Z, skip);                                                      // jz      skip

		UML_MOV(block, I0, param);                                                          // mov     i0,nextpc
		UML_MOV(block, I1, compiler->cycles);                                               // mov     i1,cycles
		UML_CALLH(block, *m_exception[EXCEPTION_INTERRUPT]);                                // callh   m_exception[EXCEPTION_INTERRUPT]

		UML_LABEL(block, skip);
	}
#endif

	/* account for cycles */
	if (compiler->cycles > 0)
	{
		UML_SUB(block, mem(&m_core->m_icount), mem(&m_core->m_icount), MAPVAR_CYCLES);          // sub     icount,icount,cycles
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);                                                // mapvar  cycles,0
#if 0 // FIXME
		if (allow_exception)
			UML_EXHc(block, COND_S, *m_out_of_cycles, param);                               // exh     out_of_cycles,nextpc
#endif
	}
	compiler->cycles = 0;
}

#if 0
bool dspp_device::generate_opcode(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc)
{
	uint32_t op = desc->opptr.l[0];

	if (op & 0x8000)
	{
		switch ((op >> 13) & 3)
		{
			case 0:
				return generate_special_opcode(op, desc);

			case 1:
			case 2:
				return generate_branch_opcode(op, desc);

			case 3:
				return generate_complex_branch_opcode(op, desc);
		}

		return false;
	}
	else
	{
		return generate_arithmetic_opcode(op, desc);
	}

	return false;
}

bool dspp_device::generate_special_opcode(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc)
{
	return true;
}

bool dspp_device::generate_branch_opcode(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc)
{
	uint32_t mode = (m_core->m_op >> 13) & 3;
	uint32_t select = (m_core->m_op >> 12) & 1;
	uint32_t mask = (m_core->m_op >> 10) & 3;

	bool flag0, flag1;

	if (select == 0)
	{
		flag0 = (m_core->m_flags & DSPI_FLAG_CC_NEG) != 0;
		flag1 = (m_core->m_flags & DSPI_FLAG_CC_OVER) != 0;
	}
	else
	{
		flag0 = (m_core->m_flags & DSPI_FLAG_CC_CARRY) != 0;
		flag1 = (m_core->m_flags & DSPI_FLAG_CC_ZERO) != 0;
	}

	bool mask0 = (mask & 2) != 0;
	bool mask1 = (mask & 1) != 0;

	bool branch = (flag0 || !mask0) && (flag1 || !mask1);

	if (mode == 2)
		branch = !branch;

	if (branch)
		m_core->m_pc = m_core->m_op & 0x3ff;

	return true;
}


void adsp21062_device::generate_jump(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc, bool delayslot, bool loopabort, bool clearint)
{
	compiler_state compiler_temp = *compiler;

	// save branch target
	if (desc->targetpc == BRANCH_TARGET_DYNAMIC)
	{
		UML_MOV(block, mem(&m_core->jmpdest), I0);                                     // mov     [jmpdest],i0
	}

	// update cycles and hash jump
	if (desc->targetpc != BRANCH_TARGET_DYNAMIC)
	{
		generate_update_cycles(block, &compiler_temp, desc->targetpc, true);
		if (desc->flags & OPFLAG_INTRABLOCK_BRANCH)
			UML_JMP(block, desc->targetpc | 0x80000000);                                // jmp      targetpc | 0x80000000
		else
			UML_HASHJMP(block, 0, desc->targetpc, *m_nocode);                           // hashjmp  0,targetpc,nocode
	}
	else
	{
		generate_update_cycles(block, &compiler_temp, mem(&m_core->jmpdest), true);
		UML_HASHJMP(block, 0, mem(&m_core->jmpdest), *m_nocode);                        // hashjmp  0,jmpdest,nocode
	}

	// update compiler label
	compiler->labelnum = compiler_temp.labelnum;

	/* reset the mapvar to the current cycles and account for skipped slots */
//	compiler->cycles += desc->skipslots;
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);                                 // mapvar  CYCLES,compiler->cycles
}

bool dspp_device::generate_complex_branch_opcode(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc)
{
	uint32_t op = desc->opptr.l[0];

	code_label skip_label = compiler->labelnum++;

//	generate_branch_target(block, compiler, desc, op & 0x3ff, ef2);
//	generate_condition(block, compiler, desc, ef1, true, skip_label, true);
	generate_branch(block, compiler, desc);
	UML_LABEL(block, skip_label);

	switch ((op >> 10) & 7)
	{
		case 0: // BLT
			// branch = (n && !v) || (!n && v);
			UML_XOR(I0, FLAG_N, 1);
			UML_XOR(I1, FLAG_V, 1);
			UML_AND(I0, I0, FLAG_V);
			UML_AND(I1, FLAG_N, I1);
			UML_OR(I0, I1, I2);
			UML_CMP(block, I0, 1);
			break;
		case 1: // BLE
			//branch = ((n && !v) || (!n && v)) || z;
			break;
		case 2: // BGE
			//branch = ((n && v) || (!n && !v));
			break;
		case 3: // BGT
			//branch = ((n && v) || (!n && !v)) && !z;
			UML_AND(I2, FLAG_N, FLAG_V);
			UML_XOR(I0, FLAG_N, 1);
			UML_XOR(I1, FLAG_V, 1);
			UML_AND(I0, I0, I1);
			UML_OR(I0, I2, I0);
			UML_XOR(I1, FLAG_Z, 1);
			UML_AND(I0, I0, I1);
			UML_CMP(I0, 1);
			break;
		case 4: // BHI
			//branch = c && !z;
			UML_XOR(I0, FLAG_Z, 1)
			UML_AND(I0, FLAG_C, I0);
			UML_CMP(I0, 1);
			break;
		case 5: // BLS
			//branch = !c || z;
			UML_XOR(I0, FLAG_C, 1);
			UML_OR(I0, I0, FLAG_Z);
			UML_CMP(block, I0, 1);
			break;
		case 6: // BXS
			//branch = x;
			UML_CMP(block, FLAG_X, 1);
			break;
		case 7: // BXC
			//branch = !x;
			UML_CMP(block, FLAG_X, 0);
			break;
	}

	UML_JMPc(block, COND_E, skip_label);

	return true;
}

bool dspp_device::generate_arithmetic_opcode(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc)
{
	uint32_t numops = (m_core->m_op >> 13) & 3;
	uint32_t muxa = (m_core->m_op >> 10) & 3;
	uint32_t muxb = (m_core->m_op >> 8) & 3;
	uint32_t alu_op = (m_core->m_op >> 4) & 0xf;
	uint32_t barrel_code = m_core->m_op & 0xf;

	int32_t mul_res = 0;
	uint32_t alu_res = 0;

	// Check for operand overflow
	if (numops == 0 && ((muxa == 1) || (muxa == 2) || (muxb == 1) || (muxb == 2)))
		numops = 4;

	// Implicit barrel shift
	if (barrel_code == 8)
		++numops;

	// Parse ops...
	parse_operands(numops);

	if (muxa == 3 || muxb == 3)
	{
		uint32_t mul_sel = (m_core->m_op >> 12) & 1;

		int32_t op1 = sign_extend16(read_next_operand());
		int32_t op2 = sign_extend16(mul_sel ? read_next_operand() : m_core->m_acc >> 4);

		mul_res = (op1 * op2) >> 11;

#if 0
		// SELECT
		UML_DSEXT(block, I0, OP1, SIZE_WORD);
		UML_DSEXT(block, I1, OP1, SIZE_WORD);
		UML_MULS(block, I0, I1, I0, I1);
		UML_SHR(block, I2, I0, 11);
#endif
	}

	// MULRES = I2
	// MUXA = I0
	// MUXB = I1

	// ALURES = I2
	// ACC_RESULT = I1?
	switch (alu_op)
	{
		case 0:	// _TRA
			// alu_res = alu_a;
			UML_MOV(block, I2, I0);
			break;

		case 1:	// _NEG
			// alu_res = -alu_b;
			UML_SUB(block, I2, 0, I1);
			break;

		case 2:	// _+
			// alu_res = alu_a + alu_b;
			UML_ADD(block, I2, I0, I1);

			// if ((alu_a & 0x80000) == (alu_b & 0x80000) &&
			//	  (alu_a & 0x80000) != (alu_res & 0x80000))
			//				m_core->m_flags |= DSPI_FLAG_CC_OVER;

			//			if (alu_res & 0x00100000)
			//				m_core->m_flags |= DSPI_FLAG_CC_CARRY;

			//			CC_V_MODIFIED(desc);
			//			CC_C_MODIFIED(desc);
			break;

		case 3:	// _+C
			UML_ADD(block, I2, I0, mew);
			//			alu_res = alu_a + (m_core->m_flags & DSPI_FLAG_CC_CARRY) ? (1 << 4) : 0;

			//			if (alu_res & 0x00100000)
			//				m_core->m_flags |= DSPI_FLAG_CC_CARRY;

			CC_C_USED(desc);
			CC_C_MODIFIED(desc);
			break;

		case 4:	// _-
			//			alu_res = alu_a - alu_b;
			UML_SUB(block, I2, I0, I1);

			//			if ((alu_a & 0x80000) == (~alu_b & 0x80000) &&
			//				(alu_a & 0x80000) != (alu_res & 0x80000))
			//				m_core->m_flags |= DSPI_FLAG_CC_OVER;

			//			if (alu_res & 0x00100000)
			//				m_core->m_flags |= DSPI_FLAG_CC_CARRY;

			CC_C_MODIFIED(desc);
			CC_V_MODIFIED(desc);
			break;

		case 5:	// _-B
			//			alu_res = alu_a - (m_core->m_flags & DSPI_FLAG_CC_CARRY) ? (1 << 4) : 0;

			//			if (alu_res & 0x00100000)
			//				m_core->m_flags |= DSPI_FLAG_CC_CARRY;

			CC_C_USED(desc);
			CC_C_MODIFIED(desc);
			break;

		case 6:	// _++
			UML_ADD(block, I2, I0, 1);
			//			alu_res = alu_a + 1;

			//			if (!(alu_a & 0x80000) && (alu_res & 0x80000))
			//				m_core->m_flags |= DSPI_FLAG_CC_OVER;

			CC_V_MODIFIED(desc);
			break;

		case 7:	// _--
			// alu_res = alu_a - 1;
			UML_SUB(block, I2, I0, 1);

			// if ((alu_a & 0x80000) && !(alu_res & 0x80000))
			//		m_core->m_flags |= DSPI_FLAG_CC_OVER;

			CC_V_MODIFIED(desc);
			break;

		case 8:	// _TRL
			//alu_res = alu_a;
			UML_MOV(block, I2, I0);
			break;

		case 9:	// _NOT
			//alu_res = ~alu_a;
			UML_XOR(block, I2, I0, 0xffff);
			break;

		case 10: // _AND
			//alu_res = alu_a & alu_b;
			UML_AND(block, I2, I0, I1);
			break;

		case 11: // _NAND
			//alu_res = ~(alu_a & alu_b);
			UML_AND(block, I2, I0, I1);
			UML_XOR(block, I2, 0xffff);
			break;

		case 12: // _OR
			//alu_res = alu_a | alu_b;
			UML_OR(block, I2, I0, I1);
			break;

		case 13: // _NOR
			//alu_res = ~(alu_a | alu_b);
			UML_OR(block, I2, I0, I1);
			UML_XOR(block, I2, I2, 0xffff);
			break;

		case 14: // _XOR
			//alu_res = alu_a ^ alu_b;
			UML_XOR(block, I2, I0, I1);
			break;

		case 15: // _XNOR
			//alu_res = ~(alu_a ^ alu_b);
			UML_XOR(block, I2, I0, I1);
			UML_XOR(block, I2, I0, 0xffff);
			break;
	}

	// SET FLAGS
	CC_SET_NEG(alu_res & 0x00080000);
	CC_SET_ZERO((alu_res & 0x000ffff0) == 0);
	CC_SET_EXACT((alu_res & 0x0000000f) == 0);

	// Barrel shift
	static const int32_t shifts[8] = { 0, 1, 2, 3, 4, 5, 8, 16 };

	if (barrel_code == 8)
		barrel_code = read_next_operand();

	if (barrel_code & 8)
	{
		// Right shift
		uint32_t shift = shifts[(~barrel_code + 1) & 7];

		if (alu_op < 8)
		{
			// Arithmetic
//			m_core->m_acc = sign_extend20(alu_res) >> shift;

			// TODO: Sign Extend to 20-bits
			UML_SHR(block, I2, I2, shift);
			UML_MAPVAR(block, MAPVAR_ACC, I2);
		}
		else
		{
			// Logical
//			m_core->m_acc = (alu_res & 0xfffff) >> shift;
			UML_AND(block, I2, 0xfffff);
			UML_SHR(block, I2, I2, shift);
			UML_MAPVAR(block, MAPVAR_ACC, I2);
		}

	}
	else
	{
		// Left shift
		uint32_t shift = shifts[barrel_code];

		if (shift == 16)
		{
			// Clip and saturate
			if (m_core->m_flags & DSPI_FLAG_CC_OVER)
				m_core->m_acc = (m_core->m_flags & DSPI_FLAG_CC_NEG) ? 0x7ffff : 0xfff80000;
			else
				m_core->m_acc = sign_extend20(alu_res);
		}
		else
		{
			m_core->m_acc = sign_extend20(alu_res) << shift;
		}
	}

	if (m_core->m_writeback >= 0)
	{
		write_data(m_core->m_writeback, m_core->m_acc >> 4);
		m_core->m_writeback = -1;
	}
	else if (m_core->m_opidx < numops)
	{
		write_next_operand(m_core->m_acc >> 4);
	}

	return true;
}

#endif

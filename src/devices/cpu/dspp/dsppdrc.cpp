// license:BSD-3-Clause
// copyright-holders:Philip Bennett

/******************************************************************************

    DSPP UML recompiler core

******************************************************************************/

#include "emu.h"
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

// exit codes
#define EXECUTE_OUT_OF_CYCLES           0
#define EXECUTE_MISSING_CODE            1
#define EXECUTE_UNMAPPED_CODE           2
#define EXECUTE_RESET_CACHE             3

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
//  uint64_t op = m_core->m_arg0;
//  fatalerror("PC=%08X: Unimplemented op %04X%08X\n", m_core->m_pc, (uint32_t)(op >> 32), (uint32_t)(op));
}

static void cfunc_update_fifo_dma(void *param)
{
	dspp_device *dspp = (dspp_device *)param;
	dspp->update_fifo_dma();
}

/*static void cfunc_print_sums(void *param)
{
    dspp_device *dspp = (dspp_device *)param;
    dspp->print_sums();
}

static void cfunc_print_value(void *param)
{
    dspp_device *dspp = (dspp_device *)param;
    dspp->print_value();
}

static void cfunc_print_addr(void *param)
{
    dspp_device *dspp = (dspp_device *)param;
    dspp->print_addr();
}

static void cfunc_print_branches(void *param)
{
    dspp_device *dspp = (dspp_device *)param;
    dspp->print_branches();
}*/

/*-------------------------------------------------
load_fast_iregs - load any fast integer
registers
-------------------------------------------------*/

inline void dspp_device::load_fast_iregs(drcuml_block &block)
{
#if 0 // TODO
	for (uint32_t regnum = 0; regnum < std::size(m_regmap); regnum++)
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

	for (regnum = 0; regnum < std::size(m_regmap); regnum++)
	{
		if (m_regmap[regnum].is_int_register())
		{
			UML_MOV(block, mem(&m_core->r[regnum]), ireg(m_regmap[regnum].ireg() - REG_I0));
		}
	}
#endif
}

void dspp_device::static_generate_memory_accessor(bool iswrite, const char *name, uml::code_handle *&handleptr)
{
	// I0 = read/write data
	// I1 = address

	drcuml_block &block = m_drcuml->begin_block(10);

	// add a global entry for this
	alloc_handle(m_drcuml.get(), &handleptr, name);
	UML_HANDLE(block, *handleptr);                                                          // handle  *handleptr

	if (iswrite)
	{
		UML_WRITE(block, I1, I0, SIZE_WORD, SPACE_DATA);
	}
	else
	{
		UML_READ(block, I0, I1, SIZE_WORD, SPACE_DATA);
	}

	UML_RET(block);

	block.end();
}


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
	int override = false;

	g_profiler.start(PROFILER_DRC_COMPILE);

	/* get a description of this sequence */
	const opcode_desc *desclist = m_drcfe->describe_code(pc);

	bool succeeded = false;
	while (!succeeded)
	{
		try
		{
			/* start the block */
			drcuml_block &block = drcuml->begin_block(32768);

			/* loop until we get through all instruction sequences */
			for (seqhead = desclist; seqhead != nullptr; seqhead = seqlast->next())
			{
				const opcode_desc *curdesc;
				uint32_t nextpc;

				/* add a code log entry */
				if (drcuml->logging())
					block.append_comment("-------------------------");

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

				compiler.abortlabel = compiler.labelnum++;
				/* iterate over instructions in the sequence and compile them */
				for (curdesc = seqhead; curdesc != seqlast->next(); curdesc = curdesc->next())
					generate_sequence_instruction(block, &compiler, curdesc);

				UML_LABEL(block, compiler.abortlabel);

				/* if we need to return to the start, do it */
				if (seqlast->flags & OPFLAG_RETURN_TO_START)
					nextpc = pc;

				/* otherwise we just go to the next instruction */
				else
					nextpc = seqlast->pc + seqlast->length;

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
	if (false/*!(m_drcoptions & DSPPDRC_STRICT_VERIFY)*/ || seqhead->next() == nullptr)
	{
		if (!(seqhead->flags & OPFLAG_VIRTUAL_NOOP))
		{
			uint32_t sum = seqhead->opptr.w[0];
			uint32_t addr = seqhead->physpc;
			const void *base = m_code_cache.read_ptr(addr);
			UML_MOV(block, I0, 0);
			UML_LOAD(block, I0, base, 0, SIZE_WORD, SCALE_x2);         // load    i0,base,0,word

			UML_CMP(block, I0, sum);                                    // cmp     i0,opptr[0]
			UML_EXHc(block, COND_NE, *m_nocode, seqhead->pc);           // exne    nocode,seqhead->pc
		}
	}

	/* full verification; sum up everything */
	else
	{
		uint32_t sum = 0;
		uint32_t addr = seqhead->physpc;
		const void *base = m_code_cache.read_ptr(addr);
		UML_LOAD(block, I0, base, 0, SIZE_WORD, SCALE_x2);              // load    i0,base,0,dword
		sum += seqhead->opptr.w[0];
		for (curdesc = seqhead->next(); curdesc != seqlast->next(); curdesc = curdesc->next())
			if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
			{
				addr = curdesc->physpc;
				base = m_code_cache.read_ptr(addr);
				assert(base != nullptr);
				UML_LOAD(block, I1, base, 0, SIZE_WORD, SCALE_x2);      // load    i1,base,dword
				UML_ADD(block, I0, I0, I1);                             // add     i0,i0,i1
				sum += curdesc->opptr.w[0];
			}
		UML_CMP(block, I0, sum);                                        // cmp     i0,sum
		UML_EXHc(block, COND_NE, *m_nocode, epc(seqhead));              // exne    nocode,seqhead->pc
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

		// generate memory accessors
		static_generate_memory_accessor(false, "dm_read16", m_dm_read16);
		static_generate_memory_accessor(true, "dm_write16", m_dm_write16);
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
	alloc_handle(m_drcuml.get(), &m_entry, "entry");
	UML_HANDLE(block, *m_entry);                                                            // handle  entry

	//load_fast_iregs(block);                                                                 // <load fastregs>

	/* generate a hash jump via the current mode and PC */
	UML_HASHJMP(block, 0, mem(&m_core->m_pc), *m_nocode);

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

	UML_MOV(block, mem(&m_core->m_pc), I0);                                                 // mov     [pc],i0
	//save_fast_iregs(block);                                                               // <save fastregs>
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
	UML_MOV(block, mem(&m_core->m_pc), I0);                                                 // mov     <pc>,i0
	//save_fast_iregs(block);                                                               // <save fastregs>
	UML_EXIT(block, EXECUTE_OUT_OF_CYCLES);                                                 // exit    EXECUTE_OUT_OF_CYCLES

	block.end();
}

void dspp_device::generate_sequence_instruction(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc)
{
	/* set the PC map variable */
	UML_MAPVAR(block, MAPVAR_PC, desc->pc);                                                 // mapvar  PC,desc->pc

	/* accumulate total cycles */
	compiler->cycles += desc->cycles;

	/* update the icount map variable */
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);                                     // mapvar  CYCLES,compiler->cycles

	/* if we are debugging, call the debugger */
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		UML_MOV(block, mem(&m_core->m_pc), desc->pc);                                       // mov     [pc],desc->pc
		//save_fast_iregs(block);                                                           // <save fastregs>
		UML_DEBUG(block, desc->pc);                                                         // debug   desc->pc
	}

	/* if we hit an unmapped address, fatal error */
	if (desc->flags & OPFLAG_COMPILER_UNMAPPED)
	{
		UML_MOV(block, mem(&m_core->m_pc), desc->pc);                                       // mov     [pc],desc->pc
		save_fast_iregs(block);                                                             // <save fastregs>
		UML_EXIT(block, EXECUTE_UNMAPPED_CODE);                                             // exit    EXECUTE_UNMAPPED_CODE
	}

	/* unless this is a virtual no-op, it's a regular instruction */
	if (!(desc->flags & OPFLAG_VIRTUAL_NOOP))
	{
		generate_opcode(block, compiler, desc);
	}
}

void dspp_device::generate_update_cycles(drcuml_block &block, compiler_state *compiler, uml::parameter param)
{
	/* account for cycles */
	if (compiler->cycles > 0)
	{
		UML_SUB(block, mem(&m_core->m_icount), mem(&m_core->m_icount), MAPVAR_CYCLES);      // sub     icount,icount,cycles
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);                                                // mapvar  cycles,0
		UML_EXHc(block, COND_S, *m_out_of_cycles, param);                                   // exh     out_of_cycles,nextpc
	}
	compiler->cycles = 0;
}

void dspp_device::generate_opcode(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc)
{
	uint16_t op = desc->opptr.w[0];

	UML_SUB(block, mem(&m_core->m_tclock), mem(&m_core->m_tclock), 1);
	UML_CALLC(block, cfunc_update_fifo_dma, this);

	if (m_drcuml.get()->logging())
		block.append_comment("generate_opcode: %04x", op);

	code_label skip = compiler->labelnum++;
	UML_TEST(block, mem(&m_core->m_dspx_control), DSPX_CONTROL_GWILLING);
	UML_JMPc(block, COND_Z, compiler->abortlabel);
	//UML_TEST(block, mem(&m_core->m_flag_sleep), 1);
	//UML_JMPc(block, COND_NZ, compiler->abortlabel);

	//UML_MOV(block, mem(&m_core->m_arg0), desc->physpc);
	//UML_MOV(block, mem(&m_core->m_arg1), op);
	//UML_CALLC(block, cfunc_print_sums, this);

	if (op & 0x8000)
	{
		switch ((op >> 13) & 3)
		{
			case 0:
				generate_special_opcode(block, compiler, desc);
				break;
			case 1:
			case 2:
				generate_branch_opcode(block, compiler, desc);
				break;
			case 3:
				generate_complex_branch_opcode(block, compiler, desc);
				break;
		}
	}
	else
	{
		generate_arithmetic_opcode(block, compiler, desc);
	}

	UML_LABEL(block, skip);
}

void dspp_device::generate_set_rbase(drcuml_block &block, compiler_state *compiler, uint32_t base, uint32_t addr)
{
	if (m_drcuml.get()->logging())
		block.append_comment("set_rbase");
	switch (base)
	{
		case 4:
			UML_MOV(block, mem(&m_core->m_rbase[1]), addr + 4 - base);
			break;
		case 0:
			UML_MOV(block, mem(&m_core->m_rbase[0]), addr);
			UML_MOV(block, mem(&m_core->m_rbase[1]), addr + 4 - base);
			[[fallthrough]];
		case 8:
			UML_MOV(block, mem(&m_core->m_rbase[2]), addr + 8 - base);
			[[fallthrough]];
		case 12:
			UML_MOV(block, mem(&m_core->m_rbase[3]), addr + 12 - base);
			break;
	}
}

void dspp_device::generate_super_special(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc)
{
	uint16_t op = desc->opptr.w[0];
	uint32_t sel = (op >> 7) & 7;

	switch (sel)
	{
		case 1: // BAC
		{
			if (m_drcuml.get()->logging())
				block.append_comment("BAC");
			UML_SHR(block, mem(&m_core->m_jmpdest), mem(&m_core->m_acc), 4);    // m_core->m_pc = m_core->m_acc >> 4;
			generate_branch(block, compiler, desc);
			break;
		}
		case 4: // RTS
		{
			if (m_drcuml.get()->logging())
				block.append_comment("RTS");
			// m_core->m_pc = m_core->m_stack[--m_core->m_stack_ptr];
			UML_SUB(block, mem(&m_core->m_stack_ptr), mem(&m_core->m_stack_ptr), 1);
			UML_LOAD(block, mem(&m_core->m_jmpdest), (void *)m_core->m_stack, mem(&m_core->m_stack_ptr), SIZE_DWORD, SCALE_x4);
			generate_branch(block, compiler, desc);
			break;
		}
		case 5: // OP_MASK
		{
			// TODO
			if (m_drcuml.get()->logging())
				block.append_comment("OP_MASK");
			break;
		}

		case 7: // SLEEP
		{
			// TODO: How does sleep work?
			if (m_drcuml.get()->logging())
				block.append_comment("SLEEP");
			UML_SUB(block, mem(&m_core->m_pc), mem(&m_core->m_pc), 1);  // --m_core->m_pc;
			UML_MOV(block, mem(&m_core->m_flag_sleep), 1);              // m_core->m_flag_sleep = 1;
			break;
		}

		case 0: // NOP
		case 2: // Unused
		case 3:
		case 6:
			break;
	}
}

void dspp_device::generate_special_opcode(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc)
{
	uint16_t op = desc->opptr.w[0];

	switch ((op >> 10) & 7)
	{
		case 0:
		{
			generate_super_special(block, compiler, desc);
			break;
		}
		case 1: // JUMP
		{
			if (m_drcuml.get()->logging())
				block.append_comment("JUMP");

			UML_MOV(block, mem(&m_core->m_jmpdest), op & 0x3ff);
			generate_branch(block, compiler, desc);
			break;
		}
		case 2: // JSR
		{
			if (m_drcuml.get()->logging())
				block.append_comment("JSR");
			UML_STORE(block, (void *)m_core->m_stack, mem(&m_core->m_stack_ptr), mem(&m_core->m_pc), SIZE_DWORD, SCALE_x4);
			UML_ADD(block, mem(&m_core->m_stack_ptr), mem(&m_core->m_stack_ptr), 1);
			UML_MOV(block, mem(&m_core->m_jmpdest), op & 0x3ff);
			generate_branch(block, compiler, desc);
			break;
		}
		case 3: // BFM
		{
			// TODO
			if (m_drcuml.get()->logging())
				block.append_comment("BFM");
			break;
		}
		case 4: // MOVEREG
		{
			if (m_drcuml.get()->logging())
				block.append_comment("MOVEREG");
			const uint32_t regdi = op & 0x3f;
			generate_translate_reg(block, regdi & 0xf);

			// Indirect
			if (regdi & 0x0010)
			{
				UML_CALLH(block, *m_dm_read16);                 // addr = read_data(addr);
				UML_MOV(block, I2, I0);
			}
			else
			{
				UML_MOV(block, I2, I1);
			}

			generate_parse_operands(block, compiler, desc, 1);
			generate_read_next_operand(block, compiler, desc);
			UML_MOV(block, I0, I1);
			UML_MOV(block, I1, I2);
			UML_CALLH(block, *m_dm_write16);                    // write_data(addr, read_next_operand());
			break;
		}
		case 5: // RBASE
		{
			if (m_drcuml.get()->logging())
				block.append_comment("RBASE");
			generate_set_rbase(block, compiler, (op & 3) << 2, op & 0x3fc);
			break;
		}
		case 6: // MOVED
		{
			if (m_drcuml.get()->logging())
				block.append_comment("MOVED");
			generate_parse_operands(block, compiler, desc, 1);
			generate_read_next_operand(block, compiler, desc);
			UML_MOV(block, I0, I1);
			UML_MOV(block, I1, op & 0x3ff);
			UML_CALLH(block, *m_dm_write16);                    // write_data(op & 0x3ff, read_next_operand());
			break;
		}
		case 7: // MOVEI
		{
			if (m_drcuml.get()->logging())
				block.append_comment("MOVEI");
			generate_parse_operands(block, compiler, desc, 1);
			UML_MOV(block, I1, op & 0x3ff);
			UML_CALLH(block, *m_dm_read16);                     // uint32_t addr = read_data(op & 0x3ff);
			UML_MOV(block, I2, I1);
			generate_read_next_operand(block, compiler, desc);
			UML_MOV(block, I0, I1);
			UML_MOV(block, I1, I2);
			UML_CALLH(block, *m_dm_write16);                    // write_data(addr, read_next_operand());
			break;
		}

		default:
			break;
	}
}

void dspp_device::generate_branch(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc)
{
	compiler_state compiler_temp(*compiler);

	/* update the cycles and jump through the hash table to the target */
	if (desc->targetpc != BRANCH_TARGET_DYNAMIC)
	{
		generate_update_cycles(block, &compiler_temp, desc->targetpc);  // <subtract cycles>
		if (desc->flags & OPFLAG_INTRABLOCK_BRANCH)
			UML_JMP(block, desc->targetpc | 0x80000000);                // jmp     desc->targetpc | 0x80000000
		else
			UML_HASHJMP(block, 0, desc->targetpc, *m_nocode);           // hashjmp <mode>,desc->targetpc,nocode
	}
	else
	{
		generate_update_cycles(block, &compiler_temp, uml::mem(&m_core->m_jmpdest));    // <subtract cycles>
		UML_HASHJMP(block, 0, mem(&m_core->m_jmpdest), *m_nocode);      // hashjmp <mode>,<rsreg>,nocode
	}

	/* update the label */
	compiler->labelnum = compiler_temp.labelnum;

	/* reset the mapvar to the current cycles */
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);                 // mapvar  CYCLES,compiler.cycles
}

void dspp_device::generate_branch_opcode(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc)
{
	uint16_t op = desc->opptr.w[0];

	if (m_drcuml.get()->logging())
		block.append_comment("branch_opcode");

	uint32_t mode = (op >> 13) & 3;
	uint32_t select = (op >> 12) & 1;
	uint32_t mask = (op >> 10) & 3;

	if (select == 0)
	{
		UML_MOV(block, I0, mem(&m_core->m_flag_neg));
		UML_MOV(block, I1, mem(&m_core->m_flag_over));
	}
	else
	{
		UML_MOV(block, I0, mem(&m_core->m_flag_carry));
		UML_MOV(block, I1, mem(&m_core->m_flag_zero));
	}

	//UML_MOV(block, mem(&m_core->m_arg1), I0);
	//UML_MOV(block, mem(&m_core->m_arg3), I1);

	const uint32_t mask0 = (mask & 2) ? 0 : 1;
	const uint32_t mask1 = (mask & 1) ? 0 : 1;

	UML_OR(block, I0, I0, mask0);
	UML_OR(block, I1, I1, mask1);
	UML_AND(block, I0, I0, I1);                                     // bool branch = (flag0 || mask0) && (flag1 || mask1);

	if (mode == 2)                                                  // if (mode == 2)
		UML_SUB(block, I0, 1, I0);                                  //     branch = !branch;

	//UML_MOV(block, mem(&m_core->m_arg0), I0);
	//UML_MOV(block, mem(&m_core->m_arg2), 1-mask0);
	//UML_MOV(block, mem(&m_core->m_arg4), 1-mask1);
	//UML_CALLC(block, cfunc_print_branches, this);

	code_label skip = compiler->labelnum++;
	UML_TEST(block, I0, 1);                                         // if (branch)
	UML_JMPc(block, COND_Z, skip);
	UML_MOV(block, mem(&m_core->m_jmpdest), op & 0x3ff);            //     m_core->m_pc = op & 0x3ff;
	generate_branch(block, compiler, desc);
	UML_LABEL(block, skip);
}

void dspp_device::generate_complex_branch_opcode(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc)
{
	uint16_t op = desc->opptr.w[0];

	switch ((op >> 10) & 7)
	{
		case 0: // BLT
			if (m_drcuml.get()->logging())
				block.append_comment("BLT");
			UML_XOR(block, I0, mem(&m_core->m_flag_neg), mem(&m_core->m_flag_over));    // branch = (n && !v) || (!n && v);
			break;
		case 1: // BLE
			if (m_drcuml.get()->logging())
				block.append_comment("BLE");
			UML_XOR(block, I0, mem(&m_core->m_flag_neg), mem(&m_core->m_flag_over));
			UML_OR(block, I0, I0, mem(&m_core->m_flag_zero));                           // branch = ((n && !v) || (!n && v)) || z;
			break;
		case 2: // BGE
			if (m_drcuml.get()->logging())
				block.append_comment("BGE");
			UML_XOR(block, I0, mem(&m_core->m_flag_neg), mem(&m_core->m_flag_over));
			UML_SUB(block, I0, 1, I0);                                                  // branch = ((n && v) || (!n && !v));
			break;
		case 3: // BGT
			if (m_drcuml.get()->logging())
				block.append_comment("BGT");
			UML_AND(block, I0, mem(&m_core->m_flag_neg), mem(&m_core->m_flag_over));
			UML_SUB(block, I0, 1, I0);
			UML_SUB(block, I1, 1, mem(&m_core->m_flag_zero));
			UML_AND(block, I0, I0, I1);                                                 // branch = ((n && v) || (!n && !v)) && !z;
			break;
		case 4: // BHI
			if (m_drcuml.get()->logging())
				block.append_comment("BHI");
			UML_SUB(block, I0, 1, mem(&m_core->m_flag_zero));
			UML_AND(block, I0, I0, mem(&m_core->m_flag_carry));                         // branch = c && !z;
			break;
		case 5: // BLS
			if (m_drcuml.get()->logging())
				block.append_comment("BLS");
			UML_SUB(block, I0, 1, mem(&m_core->m_flag_carry));
			UML_OR(block, I0, I0, mem(&m_core->m_flag_zero));                           // branch = !c || z;
			break;
		case 6: // BXS
			if (m_drcuml.get()->logging())
				block.append_comment("BXS");
			UML_MOV(block, I0, mem(&m_core->m_flag_exact));                             // branch = x;
			break;
		case 7: // BXC
			if (m_drcuml.get()->logging())
				block.append_comment("BXC");
			UML_SUB(block, I0, 1, mem(&m_core->m_flag_exact));                          // branch = !x;
			break;
	}

	code_label skip = compiler->labelnum++;
	UML_TEST(block, I0, 1);                                                             // if (branch)
	UML_JMPc(block, COND_Z, skip);
	UML_MOV(block, mem(&m_core->m_jmpdest), op & 0x3ff);                                //     m_core->m_pc = op & 0x3ff;
	generate_branch(block, compiler, desc);
	UML_LABEL(block, skip);
}

void dspp_device::generate_translate_reg(drcuml_block &block, uint16_t reg)
{
	const uint32_t base = (reg >> 2) & 3;
	if (m_drcuml.get()->logging())
		block.append_comment("translate_reg");
	UML_MOV(block, I1, mem(&m_core->m_rbase[base]));
	UML_ADD(block, I1, I1, reg - (reg & ~3));
}

void dspp_device::generate_parse_operands(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc, uint32_t numops)
{
	uint32_t addr, val = 0xBAD;
	uint32_t opidx = 0;
	uint32_t operand = 0;
	uint32_t numregs = 0;

	if (m_drcuml.get()->logging())
		block.append_comment("parse_operands");

	for (uint32_t i = 0; i < MAX_OPERANDS; ++i)
	{
		// Reset operands
		UML_DMOV(block, mem(&m_core->m_operands[i]), 0xffffffffffffffffULL);
	}

	// Reset global op index
	UML_MOV(block, mem(&m_core->m_opidx), 0);

	uint32_t opoffset = 1;
	while (opidx < numops)
	{
		operand = m_code_cache.read_word(desc->pc + opoffset);
		opoffset++;

		if (operand & 0x8000)
		{
			// Immediate value
			if ((operand & 0xc000) == 0xc000)
			{
				val = operand & 0x1fff;

				if (operand & 0x2000)
				{
					// Left justify
					val = val << 3;
				}
				else
				{
					// Sign extend if right justified
					if (val & 0x1000)
						val |= 0xe000;
				}
				UML_MOV(block, mem(&m_core->m_operands[opidx].value), val);
				opidx++;
			}
			else if((operand & 0xe000) == 0x8000)
			{
				// Address operand
				addr = operand & 0x03ff;

				if (operand & 0x0400)
				{
					// Indirect
					UML_MOV(block, I1, addr);
					UML_CALLH(block, *m_dm_read16);
					UML_MOV(block, mem(&m_core->m_operands[opidx].addr), I0);
					if (operand & 0x0800)
					{
						// Writeback
						UML_MOV(block, mem(&m_core->m_writeback), I0);
					}
				}
				else
				{
					UML_MOV(block, mem(&m_core->m_operands[opidx].addr), addr);
					if (operand & 0x0800)
					{
						// Writeback
						UML_MOV(block, mem(&m_core->m_writeback), addr);
					}
				}
				++opidx;
			}
			else if ((operand & 0xe000) == 0xa000)
			{
				// 1 or 2 register operand
				numregs = (operand & 0x0400) ? 2 : 1;
			}
		}
		else
		{
			numregs = 3;
		}

		if (numregs > 0)
		{
			// Shift successive register operands from a single operand word
			for (uint32_t i = 0; i < numregs; ++i)
			{
				uint32_t shifter = ((numregs - i) - 1) * 5;
				uint32_t regdi = (operand >> shifter) & 0x1f;
				generate_translate_reg(block, regdi & 0xf);

				if (regdi & 0x0010)
				{
					 // Indirect?
					UML_CALLH(block, *m_dm_read16);
				}

				if (numregs == 2)
				{
					if ((i == 0) && (operand & 0x1000))
						UML_MOV(block, mem(&m_core->m_writeback), I0);
					else if ((i == 1) && (operand & 0x0800))
						UML_MOV(block, mem(&m_core->m_writeback), I0);
				}
				else if (numregs == 1)
				{
					if (operand & 0x800)
						UML_MOV(block, mem(&m_core->m_writeback), I0);
				}

				UML_MOV(block, mem(&m_core->m_operands[opidx].addr), I0);
				opidx++;
			}
			numregs = 0;
		}
	}
	UML_ADD(block, mem(&m_core->m_pc), mem(&m_core->m_pc), opoffset-1);
	UML_SUB(block, mem(&m_core->m_tclock), mem(&m_core->m_tclock), opoffset-1);
}

void dspp_device::generate_read_next_operand(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc)
{
	//uint16_t op = (uint16_t)desc->opptr.w[0];

	code_label no_load;
	if (m_drcuml.get()->logging())
		block.append_comment("read_next_operand");
	UML_LOAD(block, I0, (void *)&m_core->m_operands[0].value, mem(&m_core->m_opidx), SIZE_DWORD, SCALE_x8);
	//if (op == 0x46a0)
	//{
	//  UML_MOV(block, mem(&m_core->m_arg0), I0);
	//  UML_CALLC(block, cfunc_print_value, this);
	//}

	UML_TEST(block, I0, 0x80000000U);
	UML_JMPc(block, COND_Z, no_load = compiler->labelnum++);
	UML_LOAD(block, I1, (void *)&m_core->m_operands[0].addr, mem(&m_core->m_opidx), SIZE_DWORD, SCALE_x8);
	//if (op == 0x46a0)
	//{
	//  UML_MOV(block, mem(&m_core->m_arg1), I1);
	//}
	UML_CALLH(block, *m_dm_read16);
	//if (op == 0x46a0)
	//{
	//  UML_MOV(block, mem(&m_core->m_arg0), I0);
	//  UML_CALLC(block, cfunc_print_addr, this);
	//}

	UML_LABEL(block, no_load);

	// Next operand
	UML_ADD(block, mem(&m_core->m_opidx), mem(&m_core->m_opidx), 1);
}

void dspp_device::generate_write_next_operand(drcuml_block &block, compiler_state *compiler)
{
	if (m_drcuml.get()->logging())
		block.append_comment("write_next_operand");
	// int32_t addr = m_core->m_operands[m_core->m_opidx].addr;
	UML_LOAD(block, I1, (void *)&m_core->m_operands[0].addr, mem(&m_core->m_opidx), SIZE_DWORD, SCALE_x8);

	// write_data(addr, m_core->m_acc >> 4);
	UML_SHR(block, I0, mem(&m_core->m_acc), 4);

	// Advance to the next operand
	UML_ADD(block, mem(&m_core->m_opidx), mem(&m_core->m_opidx), 1);
}

void dspp_device::generate_arithmetic_opcode(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc)
{
	uint16_t op = (uint16_t)desc->opptr.w[0];
	uint32_t numops = (op >> 13) & 3;
	uint32_t muxa = (op >> 10) & 3;
	uint32_t muxb = (op >> 8) & 3;
	uint32_t alu_op = (op >> 4) & 0xf;
	uint32_t barrel_code = op & 0xf;

	if (m_drcuml.get()->logging())
		block.append_comment("arithmetic_opcode");

	// Check for operand overflow
	if (numops == 0 && ((muxa == 1) || (muxa == 2) || (muxb == 1) || (muxb == 2)))
		numops = 4;

	// Implicit barrel shift
	if (barrel_code == 8)
		++numops;

	// Parse ops...
	generate_parse_operands(block, compiler, desc, numops);

	if (muxa == 3 || muxb == 3)
	{
		uint32_t mul_sel = (op >> 12) & 1;

		generate_read_next_operand(block, compiler, desc);
		UML_SEXT(block, I2, I2, SIZE_WORD);
		if (mul_sel)
		{
			generate_read_next_operand(block, compiler, desc);
		}
		else
		{
			UML_SHR(block, I0, mem(&m_core->m_acc), 4);
		}
		UML_SEXT(block, I0, I0, SIZE_WORD);
		UML_MULS(block, I3, I0, I0, I2); // mul_res = (op1 * op2);
		UML_SHR(block, I3, I3, 11); // mul_res >>= 11;
	}

	switch (muxa)
	{
		case 0:
			UML_MOV(block, I2, mem(&m_core->m_acc)); // alu_a = m_core->m_acc;
			break;
		case 1: case 2:
			generate_read_next_operand(block, compiler, desc);
			UML_SHL(block, I2, I0, 4); // alu_a = read_next_operand() << 4;
			break;
		case 3:
		{
			UML_MOV(block, I2, I3); // alu_a = mul_res;
			break;
		}
	}

	switch (muxb)
	{
		case 0:
		{
			UML_MOV(block, I3, mem(&m_core->m_acc)); // alu_b = m_core->m_acc;
			break;
		}
		case 1: case 2:
		{
			generate_read_next_operand(block, compiler, desc);
			UML_SHL(block, I3, I0, 4); // alu_b = read_next_operand() << 4;
			break;
		}
		case 3:
		{
			// alu_b = mul_res;
			break;
		}
	}

	// For carry detection apparently
	UML_AND(block, I2, I2, 0x000fffff);
	UML_AND(block, I3, I3, 0x000fffff);

	code_label skip_over = compiler->labelnum++;
	// ALU_A = I2
	// ALU_B = I3
	// ALU_RES = I0
	switch (alu_op)
	{
		case 0: // _TRA
			if (m_drcuml.get()->logging())
				block.append_comment("_TRA");
			UML_MOV(block, I0, I2); // alu_res = alu_a;
			UML_MOV(block, mem(&m_core->m_flag_over), 0);               // m_core->m_flag_over = 0;
			UML_MOV(block, mem(&m_core->m_flag_carry), 0);              // m_core->m_flag_carry = 0;
			break;

		case 1: // _NEG
			if (m_drcuml.get()->logging())
				block.append_comment("_NEG");
			UML_SUB(block, I0, 0, I3); // alu_res = -alu_b;
			UML_MOV(block, mem(&m_core->m_flag_over), 0);               // m_core->m_flag_over = 0;
			UML_MOV(block, mem(&m_core->m_flag_carry), 0);              // m_core->m_flag_carry = 0;
			break;

		case 2: // _+
			if (m_drcuml.get()->logging())
				block.append_comment("_+");
			UML_ADD(block, I0, I2, I3); // alu_res = alu_a + alu_b;

			UML_MOV(block, mem(&m_core->m_flag_over), 0);

			UML_XOR(block, I3, I2, I3);
			UML_TEST(block, I3, 0x80000);
			UML_JMPc(block, COND_NZ, skip_over);                        // if ((alu_a & 0x80000) == (alu_b & 0x80000) &&
			UML_XOR(block, I3, I2, I0);
			UML_TEST(block, I3, 0x80000);                               //     (alu_a & 0x80000) != (alu_res & 0x80000))
			UML_MOVc(block, COND_NZ, mem(&m_core->m_flag_over), 1);     //     m_core->m_flag_over = 1;

			UML_LABEL(block, skip_over);
			UML_TEST(block, I0, 0x00100000);                            // if (alu_res & 0x00100000)
			UML_MOVc(block, COND_NZ, mem(&m_core->m_flag_carry), 1);    //     m_core->m_flag_carry = 1;
																		// else
			UML_MOVc(block, COND_Z, mem(&m_core->m_flag_carry), 0);     //     m_core->m_flag_carry = 0;
			break;

		case 3: // _+C
			if (m_drcuml.get()->logging())
				block.append_comment("_+C");
			UML_SHL(block, I3, mem(&m_core->m_flag_carry), 4);
			UML_ADD(block, I0, I2, I3);                                 // alu_res = alu_a + (m_core->m_flag_carry << 4);

			UML_MOV(block, mem(&m_core->m_flag_over), 0);               // m_core->m_flag_over = 0;

			UML_TEST(block, I0, 0x00100000);                            // if (alu_res & 0x00100000)
			UML_MOVc(block, COND_NZ, mem(&m_core->m_flag_carry), 1);    //     m_core->m_flag_carry = 1;
																		// else
			UML_MOVc(block, COND_Z, mem(&m_core->m_flag_carry), 0);     //     m_core->m_flag_carry = 0;
			break;

		case 4: // _-
			if (m_drcuml.get()->logging())
				block.append_comment("_-");
			UML_SUB(block, I0, I2, I3);                                 // alu_res = alu_a - alu_b;

			UML_MOV(block, mem(&m_core->m_flag_over), 0);

			UML_XOR(block, I3, I3, 0xffffffffU);
			UML_XOR(block, I3, I2, I3);
			UML_TEST(block, I3, 0x80000);
			UML_JMPc(block, COND_NZ, skip_over);                        // if ((alu_a & 0x80000) == (~alu_b & 0x80000) &&
			UML_XOR(block, I3, I2, I0);
			UML_TEST(block, I3, 0x80000);                               //     (alu_a & 0x80000) != (alu_res & 0x80000))
			UML_MOVc(block, COND_NZ, mem(&m_core->m_flag_over), 1);     //     m_core->m_flag_over = 1;

			UML_LABEL(block, skip_over);
			UML_TEST(block, I0, 0x00100000);                            // if (alu_res & 0x00100000)
			UML_MOVc(block, COND_NZ, mem(&m_core->m_flag_carry), 1);    //     m_core->m_flag_carry = 1;
																		// else
			UML_MOVc(block, COND_Z, mem(&m_core->m_flag_carry), 0);     //     m_core->m_flag_carry = 0;
			break;

		case 5: // _-B
			if (m_drcuml.get()->logging())
				block.append_comment("_-B");
			UML_SHL(block, I3, mem(&m_core->m_flag_carry), 4);
			UML_SUB(block, I0, I2, I3);                                 // alu_res = alu_a - (m_core->m_flag_carry << 4);

			UML_MOV(block, mem(&m_core->m_flag_over), 0);               // m_core->m_flag_over = 0;

			UML_TEST(block, I0, 0x00100000);                            // if (alu_res & 0x00100000)
			UML_MOVc(block, COND_NZ, mem(&m_core->m_flag_carry), 1);    //     m_core->m_flag_carry = 1;
																		// else
			UML_MOVc(block, COND_Z, mem(&m_core->m_flag_carry), 0);     //     m_core->m_flag_carry = 0;
			break;

		case 6: // _++
			if (m_drcuml.get()->logging())
				block.append_comment("_++");
			UML_ADD(block, I0, I2, 1);                                  // alu_res = alu_a + 1;

			UML_XOR(block, I3, I2, 0x80000);
			UML_AND(block, I3, I3, I0);
			UML_TEST(block, I3, 0x80000);
			UML_MOVc(block, COND_NZ, mem(&m_core->m_flag_over), 1);
			UML_MOVc(block, COND_Z, mem(&m_core->m_flag_over), 0);      // m_core->m_flag_over = !(alu_a & 0x80000) && (alu_res & 0x80000);

			UML_MOV(block, mem(&m_core->m_flag_carry), 0);              // m_core->m_flag_carry = 0;
			break;

		case 7: // _--
			if (m_drcuml.get()->logging())
				block.append_comment("_--");
			UML_SUB(block, I0, I2, 1);                                  // alu_res = alu_a - 1;

			UML_XOR(block, I3, I0, 0x80000);
			UML_AND(block, I3, I3, I2);
			UML_TEST(block, I3, 0x80000);
			UML_MOVc(block, COND_NZ, mem(&m_core->m_flag_over), 1);
			UML_MOVc(block, COND_Z, mem(&m_core->m_flag_over), 0);      // m_core->m_flag_over = (alu_a & 0x80000) && !(alu_res & 0x80000);

			UML_MOV(block, mem(&m_core->m_flag_carry), 0);              // m_core->m_flag_carry = 0;
			break;

		case 8: // _TRL
			if (m_drcuml.get()->logging())
				block.append_comment("_TRL");
			UML_MOV(block, I0, I2);                                     // alu_res = alu_a;
			UML_MOV(block, mem(&m_core->m_flag_over), 0);               // m_core->m_flag_over = 0;
			UML_MOV(block, mem(&m_core->m_flag_carry), 0);              // m_core->m_flag_carry = 0;
			break;

		case 9: // _NOT
			if (m_drcuml.get()->logging())
				block.append_comment("_NOT");
			UML_XOR(block, I0, I2, 0xffffffff);                         // alu_res = ~alu_a;
			UML_MOV(block, mem(&m_core->m_flag_over), 0);               // m_core->m_flag_over = 0;
			UML_MOV(block, mem(&m_core->m_flag_carry), 0);              // m_core->m_flag_carry = 0;
			break;

		case 10: // _AND
			if (m_drcuml.get()->logging())
				block.append_comment("_AND");
			UML_AND(block, I0, I2, I3);                                 // alu_res = alu_a & alu_b;
			UML_MOV(block, mem(&m_core->m_flag_over), 0);               // m_core->m_flag_over = 0;
			UML_MOV(block, mem(&m_core->m_flag_carry), 0);              // m_core->m_flag_carry = 0;
			break;

		case 11: // _NAND
			if (m_drcuml.get()->logging())
				block.append_comment("_NAND");
			UML_AND(block, I0, I2, I3);
			UML_XOR(block, I0, I0, 0xffffffff);                         // alu_res = ~(alu_a & alu_b);
			UML_MOV(block, mem(&m_core->m_flag_over), 0);               // m_core->m_flag_over = 0;
			UML_MOV(block, mem(&m_core->m_flag_carry), 0);              // m_core->m_flag_carry = 0;
			break;

		case 12: // _OR
			if (m_drcuml.get()->logging())
				block.append_comment("_OR");
			UML_OR(block, I0, I2, I3);                                  // alu_res = alu_a | alu_b;
			UML_MOV(block, mem(&m_core->m_flag_over), 0);               // m_core->m_flag_over = 0;
			UML_MOV(block, mem(&m_core->m_flag_carry), 0);              // m_core->m_flag_carry = 0;
			break;

		case 13: // _NOR
			if (m_drcuml.get()->logging())
				block.append_comment("_NOR");
			UML_OR(block, I0, I2, I3);
			UML_XOR(block, I0, I0, 0xffffffff);                         // alu_res = ~(alu_a | alu_b);
			UML_MOV(block, mem(&m_core->m_flag_over), 0);               // m_core->m_flag_over = 0;
			UML_MOV(block, mem(&m_core->m_flag_carry), 0);              // m_core->m_flag_carry = 0;
			break;

		case 14: // _XOR
			if (m_drcuml.get()->logging())
				block.append_comment("_XOR");
			UML_XOR(block, I0, I2, I3);                                 // alu_res = alu_a ^ alu_b;
			UML_MOV(block, mem(&m_core->m_flag_over), 0);               // m_core->m_flag_over = 0;
			UML_MOV(block, mem(&m_core->m_flag_carry), 0);              // m_core->m_flag_carry = 0;
			break;

		case 15: // _XNOR
			if (m_drcuml.get()->logging())
				block.append_comment("_XNOR");
			UML_XOR(block, I0, I2, I3);
			UML_XOR(block, I0, I0, 0xffffffff);                         // alu_res = ~(alu_a ^ alu_b);
			UML_MOV(block, mem(&m_core->m_flag_over), 0);               // m_core->m_flag_over = 0;
			UML_MOV(block, mem(&m_core->m_flag_carry), 0);              // m_core->m_flag_carry = 0;
			break;
	}

	UML_TEST(block, I0, 0x00080000);
	UML_MOVc(block, COND_NZ, mem(&m_core->m_flag_neg), 1);
	UML_MOVc(block, COND_Z, mem(&m_core->m_flag_neg), 0);               // m_core->m_flag_neg = (alu_res & 0x00080000) != 0;

	UML_TEST(block, I0, 0x000ffff0);
	UML_MOVc(block, COND_Z, mem(&m_core->m_flag_zero), 1);
	UML_MOVc(block, COND_NZ, mem(&m_core->m_flag_zero), 0);             // m_core->m_flag_zero = (alu_res & 0x000ffff0) == 0;

	UML_TEST(block, I0, 0x0000000f);
	UML_MOVc(block, COND_Z, mem(&m_core->m_flag_exact), 1);
	UML_MOVc(block, COND_NZ, mem(&m_core->m_flag_exact), 0);            // m_core->m_flag_exact = (alu_res & 0x0000000f) == 0;

	// ALU_RES = I3
	UML_MOV(block, I3, I0);

	// Barrel shift
	static const int32_t shifts[8] = { 0, 1, 2, 3, 4, 5, 8, 16 };

	if (barrel_code == 8)
		generate_read_next_operand(block, compiler, desc);              // I0 = barrel_code;
	else
		UML_MOV(block, I0, barrel_code);                                // I0 = barrel_code;

	code_label left_shift = compiler->labelnum++;
	code_label done_shift = compiler->labelnum++;
	code_label no_shift = compiler->labelnum++;
	code_label no_clip = compiler->labelnum++;
	code_label no_writeback = compiler->labelnum++;
	code_label done = compiler->labelnum++;
	UML_TEST(block, I0, 8);                                             // if (barrel_code & 8)
	UML_JMPc(block, COND_Z, left_shift);                                // {
	UML_XOR(block, I0, I0, 0xffffffffU);
	UML_ADD(block, I0, I0, 1);
	UML_AND(block, I0, I0, 7);
	UML_LOAD(block, I0, (void *)shifts, I0, SIZE_DWORD, SCALE_x8);      //     uint32_t shift = shifts[(~barrel_code + 1) & 7];
	if (alu_op < 8)                                                     //     if (alu_op < 8)
	{                                                                   //     {
		UML_SHL(block, I3, I3, 12);
		UML_SAR(block, I3, I3, 12);                                     //         // Arithmetic
		UML_SAR(block, mem(&m_core->m_acc), I3, I0);                    //         m_core->m_acc = sign_extend20(alu_res) >> shift;
	}                                                                   //     }
	else                                                                //     else
	{                                                                   //     {
		UML_AND(block, I3, I3, 0x000fffff);                             //         // Logical
		UML_SHR(block, mem(&m_core->m_acc), I3, I0);                    //         m_core->m_acc = (alu_res & 0xfffff) >> shift;
	}                                                                   //     }
	UML_JMP(block, done_shift);                                         // }
	UML_LABEL(block, left_shift);                                       // else
																		// {
	UML_LOAD(block, I0, (void *)shifts, I0, SIZE_DWORD, SCALE_x8);      //     uint32_t shift = shifts[barrel_code];
	UML_CMP(block, I0, 16);                                             //     if (shift != 16)
	UML_JMPc(block, COND_E, no_shift);                                  //     {
	UML_SHL(block, I3, I3, 12);
	UML_SAR(block, I3, I3, 12);
	UML_SHL(block, mem(&m_core->m_acc), I3, I0);                        //         m_core->m_acc = sign_extend20(alu_res) << shift;
	UML_JMP(block, done_shift);                                         //     }
																		//     else
	UML_LABEL(block, no_shift);                                         //     {
	UML_TEST(block, mem(&m_core->m_flag_over), 1);                      //         // Clip and saturate
	UML_JMPc(block, COND_Z, no_clip);                                   //         if (m_core->m_flag_over)
	UML_TEST(block, mem(&m_core->m_flag_neg), 1);
	UML_MOVc(block, COND_NZ, mem(&m_core->m_acc), 0x7ffff);             //             m_core->m_acc = m_core->m_flag_neg ? 0x7ffff : 0xfff80000;
	UML_MOVc(block, COND_Z, mem(&m_core->m_acc), 0xfff80000);
	UML_JMP(block, done_shift);                                         //         else
	UML_LABEL(block, no_clip);
	UML_SHL(block, I3, I3, 12);                                         //             sign_extend20(alu_res);
	UML_SAR(block, mem(&m_core->m_acc), I3, 12);                        //     }
	UML_LABEL(block, done_shift);                                       // }

	UML_CMP(block, mem(&m_core->m_writeback), 0);                       // if (m_core->m_writeback >= 0)
	UML_JMPc(block, COND_L, no_writeback);                              // {
	UML_SHR(block, I0, mem(&m_core->m_acc), 4);
	UML_MOV(block, I1, mem(&m_core->m_writeback));
	UML_CALLH(block, *m_dm_write16);                                    //     write_data(m_core->m_writeback, m_core->m_acc >> 4);
	UML_MOV(block, mem(&m_core->m_writeback), 0xffffffffU);             //     m_core->m_writeback = -1;
	UML_JMP(block, done);                                               // }

	UML_LABEL(block, no_writeback);
	UML_CMP(block, mem(&m_core->m_opidx), numops);                      // else if (m_core->m_opidx < numops)
	UML_JMPc(block, COND_GE, done);                                     // {
	generate_write_next_operand(block, compiler);                       //     write_next_operand(m_core->m_acc >> 4);
	UML_LABEL(block, done);                                             // }
}

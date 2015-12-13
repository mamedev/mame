// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    rspdrc.c

    Universal machine language-based Nintendo/SGI RSP emulator.
    Written by Ryan Holtz
    SIMD versions of vector multiplication opcodes provided by Marathon Man
      of the CEN64 team.

****************************************************************************

    Future improvements/changes:

    * Confer with Aaron Giles about adding a memory hash-based caching
      system and static recompilation for maximum overhead minimization

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "rsp.h"
#include "rspfe.h"
#include "rspcp2.h"
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"

using namespace uml;

CPU_DISASSEMBLE( rsp );

extern offs_t rsp_dasm_one(char *buffer, offs_t pc, UINT32 op);

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* map variables */
#define MAPVAR_PC                       M0
#define MAPVAR_CYCLES                   M1

/* exit codes */
#define EXECUTE_OUT_OF_CYCLES           0
#define EXECUTE_MISSING_CODE            1
#define EXECUTE_UNMAPPED_CODE           2
#define EXECUTE_RESET_CACHE             3



/***************************************************************************
    Macros
***************************************************************************/

#define R32(reg)                m_regmap[reg]

/***************************************************************************
    Inline Functions
***************************************************************************/

/*-------------------------------------------------
    epc - compute the exception PC from a
    descriptor
-------------------------------------------------*/

static inline UINT32 epc(const opcode_desc *desc)
{
	return ((desc->flags & OPFLAG_IN_DELAY_SLOT) ? (desc->pc - 3) : desc->pc) | 0x1000;
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
    load_fast_iregs - load any fast integer
    registers
-------------------------------------------------*/

inline void rsp_device::load_fast_iregs(drcuml_block *block)
{
	int regnum;

	for (regnum = 0; regnum < ARRAY_LENGTH(m_regmap); regnum++)
		if (m_regmap[regnum].is_int_register())
			UML_MOV(block, ireg(m_regmap[regnum].ireg() - REG_I0), mem(&m_rsp_state->r[regnum]));
}


/*-------------------------------------------------
    save_fast_iregs - save any fast integer
    registers
-------------------------------------------------*/

inline void rsp_device::save_fast_iregs(drcuml_block *block)
{
	int regnum;

	for (regnum = 0; regnum < ARRAY_LENGTH(m_regmap); regnum++)
		if (m_regmap[regnum].is_int_register())
			UML_MOV(block, mem(&m_rsp_state->r[regnum]), ireg(m_regmap[regnum].ireg() - REG_I0));
}

/***************************************************************************
    CORE CALLBACKS
***************************************************************************/

inline void rsp_device::ccfunc_read8()
{
	m_rsp_state->arg0 = DM_READ8(m_rsp_state->arg0);
}

static void cfunc_read8(void *param)
{
	((rsp_device *)param)->ccfunc_read8();
}

inline void rsp_device::ccfunc_read16()
{
	m_rsp_state->arg0 = DM_READ16(m_rsp_state->arg0);
}

static void cfunc_read16(void *param)
{
	((rsp_device *)param)->ccfunc_read16();
}

inline void rsp_device::ccfunc_read32()
{
	m_rsp_state->arg0 = DM_READ32(m_rsp_state->arg0);
}

static void cfunc_read32(void *param)
{
	((rsp_device *)param)->ccfunc_read32();;
}

inline void rsp_device::ccfunc_write8()
{
	DM_WRITE8(m_rsp_state->arg0, m_rsp_state->arg1);
}

static void cfunc_write8(void *param)
{
	((rsp_device *)param)->ccfunc_write8();;
}

inline void rsp_device::ccfunc_write16()
{
	DM_WRITE16(m_rsp_state->arg0, m_rsp_state->arg1);
}

static void cfunc_write16(void *param)
{
	((rsp_device *)param)->ccfunc_write16();;
}

inline void rsp_device::ccfunc_write32()
{
	DM_WRITE32(m_rsp_state->arg0, m_rsp_state->arg1);
}

static void cfunc_write32(void *param)
{
	((rsp_device *)param)->ccfunc_write32();;
}

/*****************************************************************************/

/*-------------------------------------------------
    rspdrc_set_options - configure DRC options
-------------------------------------------------*/

void rsp_device::rspdrc_set_options(UINT32 options)
{
	if (!(mconfig().options().drc() && !mconfig().m_force_no_drc)) return;
	m_drcoptions = options;
}


inline void rsp_device::ccfunc_get_cop0_reg()
{
	int reg = m_rsp_state->arg0;
	int dest = m_rsp_state->arg1;

	if (reg >= 0 && reg < 8)
	{
		if(dest)
		{
			m_rsp_state->r[dest] = m_sp_reg_r_func(reg, 0xffffffff);
		}
	}
	else if (reg >= 8 && reg < 16)
	{
		if(dest)
		{
			m_rsp_state->r[dest] = m_dp_reg_r_func(reg - 8, 0xffffffff);
		}
	}
	else
	{
		fatalerror("RSP: cfunc_get_cop0_reg: %d\n", reg);
	}
}

static void cfunc_get_cop0_reg(void *param)
{
	((rsp_device *)param)->ccfunc_get_cop0_reg();
}

inline void rsp_device::ccfunc_set_cop0_reg()
{
	int reg = m_rsp_state->arg0;
	UINT32 data = m_rsp_state->arg1;

	if (reg >= 0 && reg < 8)
	{
		m_sp_reg_w_func(reg, data, 0xffffffff);
	}
	else if (reg >= 8 && reg < 16)
	{
		m_dp_reg_w_func(reg - 8, data, 0xffffffff);
	}
	else
	{
		fatalerror("RSP: set_cop0_reg: %d, %08X\n", reg, data);
	}
}

static void cfunc_set_cop0_reg(void *param)
{
	((rsp_device *)param)->ccfunc_set_cop0_reg();
}

/*****************************************************************************/

void rsp_device::rspcom_init()
{
}

inline void rsp_device::ccfunc_sp_set_status_cb()
{
	m_sp_set_status_func(0, m_rsp_state->arg0, 0xffffffff);
}

void cfunc_sp_set_status_cb(void *param)
{
	((rsp_device *)param)->ccfunc_sp_set_status_cb();
}

void rsp_device::execute_run_drc()
{
	drcuml_state *drcuml = m_drcuml;
	int execute_result;

	/* reset the cache if dirty */
	if (m_cache_dirty)
		code_flush_cache();
	m_cache_dirty = FALSE;

	/* execute */
	do
	{
		if( m_sr & ( RSP_STATUS_HALT | RSP_STATUS_BROKE ) )
		{
			m_rsp_state->icount = MIN(m_rsp_state->icount, 0);
			break;
		}

		/* run as much as we can */
		execute_result = drcuml->execute(*m_entry);

		/* if we need to recompile, do it */
		if (execute_result == EXECUTE_MISSING_CODE)
		{
			code_compile_block(m_rsp_state->pc);
		}
		else if (execute_result == EXECUTE_UNMAPPED_CODE)
		{
			fatalerror("Attempted to execute unmapped code at PC=%08X\n", m_rsp_state->pc);
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
    rspdrc_flush_drc_cache - outward-facing
    accessor to code_flush_cache
-------------------------------------------------*/

void rsp_device::rspdrc_flush_drc_cache()
{
	if (!(mconfig().options().drc() && !mconfig().m_force_no_drc)) return;
	m_cache_dirty = TRUE;
}

/*-------------------------------------------------
    code_flush_cache - flush the cache and
    regenerate static code
-------------------------------------------------*/

void rsp_device::code_flush_cache()
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
		static_generate_memory_accessor(1, FALSE, "read8",       m_read8);
		static_generate_memory_accessor(1, TRUE,  "write8",      m_write8);
		static_generate_memory_accessor(2, FALSE, "read16",      m_read16);
		static_generate_memory_accessor(2, TRUE,  "write16",     m_write16);
		static_generate_memory_accessor(4, FALSE, "read32",      m_read32);
		static_generate_memory_accessor(4, TRUE,  "write32",     m_write32);
	}
	catch (drcuml_block::abort_compilation &)
	{
		fatalerror("Unable to generate static RSP code\n");
	}
}


/*-------------------------------------------------
    code_compile_block - compile a block of the
    given mode at the specified pc
-------------------------------------------------*/

void rsp_device::code_compile_block(offs_t pc)
{
	drcuml_state *drcuml = m_drcuml;
	compiler_state compiler = { 0 };
	const opcode_desc *seqhead, *seqlast;
	const opcode_desc *desclist;
	int override = FALSE;
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
				UINT32 nextpc;

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
					override = TRUE;
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
				generate_update_cycles(block, &compiler, nextpc, TRUE);            // <subtract cycles>

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

inline void rsp_device::ccfunc_unimplemented()
{
	UINT32 opcode = m_rsp_state->arg0;
	fatalerror("PC=%08X: Unimplemented op %08X (%02X,%02X)\n", m_rsp_state->pc, opcode, opcode >> 26, opcode & 0x3f);
}

static void cfunc_unimplemented(void *param)
{
	((rsp_device *)param)->ccfunc_unimplemented();
}

/*-------------------------------------------------
    cfunc_fatalerror - a generic fatalerror call
-------------------------------------------------*/

#ifdef UNUSED_CODE
static void cfunc_fatalerror(void *param)
{
	fatalerror("fatalerror\n");
}
#endif


/***************************************************************************
    STATIC CODEGEN
***************************************************************************/

/*-------------------------------------------------
    ferate_entry_point - generate a
    static entry point
-------------------------------------------------*/

void rsp_device::static_generate_entry_point()
{
	drcuml_state *drcuml = m_drcuml;
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(20);

	/* forward references */
	alloc_handle(drcuml, &m_nocode, "nocode");

	alloc_handle(drcuml, &m_entry, "entry");
	UML_HANDLE(block, *m_entry);                                       // handle  entry

	/* load fast integer registers */
	load_fast_iregs(block);

	/* generate a hash jump via the current mode and PC */
	UML_HASHJMP(block, 0, mem(&m_rsp_state->pc), *m_nocode);                   // hashjmp <mode>,<pc>,nocode
	block->end();
}


/*-------------------------------------------------
    static_generate_nocode_handler - generate an
    exception handler for "out of code"
-------------------------------------------------*/

void rsp_device::static_generate_nocode_handler()
{
	drcuml_state *drcuml = m_drcuml;
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(10);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &m_nocode, "nocode");
	UML_HANDLE(block, *m_nocode);                                      // handle  nocode
	UML_GETEXP(block, I0);                                                      // getexp  i0
	UML_MOV(block, mem(&m_rsp_state->pc), I0);                                          // mov     [pc],i0
	save_fast_iregs(block);
	UML_EXIT(block, EXECUTE_MISSING_CODE);                                      // exit    EXECUTE_MISSING_CODE

	block->end();
}


/*-------------------------------------------------
    static_generate_out_of_cycles - generate an
    out of cycles exception handler
-------------------------------------------------*/

void rsp_device::static_generate_out_of_cycles()
{
	drcuml_state *drcuml = m_drcuml;
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(10);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &m_out_of_cycles, "out_of_cycles");
	UML_HANDLE(block, *m_out_of_cycles);                               // handle  out_of_cycles
	UML_GETEXP(block, I0);                                                      // getexp  i0
	UML_MOV(block, mem(&m_rsp_state->pc), I0);                                          // mov     <pc>,i0
	save_fast_iregs(block);
	UML_EXIT(block, EXECUTE_OUT_OF_CYCLES);                                 // exit    EXECUTE_OUT_OF_CYCLES

	block->end();
}

/*------------------------------------------------------------------
    static_generate_memory_accessor
------------------------------------------------------------------*/

void rsp_device::static_generate_memory_accessor(int size, int iswrite, const char *name, code_handle *&handleptr)
{
	/* on entry, address is in I0; data for writes is in I1 */
	/* on exit, read result is in I0 */
	/* routine trashes I0-I1 */
	drcuml_state *drcuml = m_drcuml;
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(1024);

	/* add a global entry for this */
	alloc_handle(drcuml, &handleptr, name);
	UML_HANDLE(block, *handleptr);                                                  // handle  *handleptr

	// write:
	if (iswrite)
	{
		if (size == 1)
		{
			UML_MOV(block, mem(&m_rsp_state->arg0), I0);              // mov     [arg0],i0 ; address
			UML_MOV(block, mem(&m_rsp_state->arg1), I1);              // mov     [arg1],i1 ; data
			UML_CALLC(block, cfunc_write8, this);                            // callc   cfunc_write8
		}
		else if (size == 2)
		{
			UML_MOV(block, mem(&m_rsp_state->arg0), I0);              // mov     [arg0],i0 ; address
			UML_MOV(block, mem(&m_rsp_state->arg1), I1);              // mov     [arg1],i1 ; data
			UML_CALLC(block, cfunc_write16, this);                           // callc   cfunc_write16
		}
		else if (size == 4)
		{
			UML_MOV(block, mem(&m_rsp_state->arg0), I0);              // mov     [arg0],i0 ; address
			UML_MOV(block, mem(&m_rsp_state->arg1), I1);              // mov     [arg1],i1 ; data
			UML_CALLC(block, cfunc_write32, this);                           // callc   cfunc_write32
		}
	}
	else
	{
		if (size == 1)
		{
			UML_MOV(block, mem(&m_rsp_state->arg0), I0);          // mov     [arg0],i0 ; address
			UML_CALLC(block, cfunc_read8, this);                         // callc   read8
			UML_MOV(block, I0, mem(&m_rsp_state->arg0));          // mov     i0,[arg0],i0 ; result
		}
		else if (size == 2)
		{
			UML_MOV(block, mem(&m_rsp_state->arg0), I0);          // mov     [arg0],i0 ; address
			UML_CALLC(block, cfunc_read16, this);                        // callc   cfunc_read16
			UML_MOV(block, I0, mem(&m_rsp_state->arg0));          // mov     i0,[arg0],i0 ; result
		}
		else if (size == 4)
		{
			UML_MOV(block, mem(&m_rsp_state->arg0), I0);          // mov     [arg0],i0 ; address
			UML_CALLC(block, cfunc_read32, this);                        // callc   cfunc_read32
			UML_MOV(block, I0, mem(&m_rsp_state->arg0));          // mov     i0,[arg0],i0 ; result
		}
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
void rsp_device::generate_update_cycles(drcuml_block *block, compiler_state *compiler, uml::parameter param, int allow_exception)
{
	/* account for cycles */
	if (compiler->cycles > 0)
	{
		UML_SUB(block, mem(&m_rsp_state->icount), mem(&m_rsp_state->icount), MAPVAR_CYCLES);        // sub     icount,icount,cycles
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);                                        // mapvar  cycles,0
		UML_EXHc(block, COND_S, *m_out_of_cycles, param);
	}
	compiler->cycles = 0;
}

/*-------------------------------------------------
    generate_checksum_block - generate code to
    validate a sequence of opcodes
-------------------------------------------------*/

void rsp_device::generate_checksum_block(drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast)
{
	const opcode_desc *curdesc;
	if (m_drcuml->logging())
	{
		block->append_comment("[Validation for %08X]", seqhead->pc | 0x1000);       // comment
	}
	/* loose verify or single instruction: just compare and fail */
	if (!(m_drcoptions & RSPDRC_STRICT_VERIFY) || seqhead->next() == nullptr)
	{
		if (!(seqhead->flags & OPFLAG_VIRTUAL_NOOP))
		{
			UINT32 sum = seqhead->opptr.l[0];
			void *base = m_direct->read_ptr(seqhead->physpc | 0x1000);
			UML_LOAD(block, I0, base, 0, SIZE_DWORD, SCALE_x4);                         // load    i0,base,0,dword

			if (seqhead->delay.first() != nullptr && seqhead->physpc != seqhead->delay.first()->physpc)
			{
				base = m_direct->read_ptr(seqhead->delay.first()->physpc | 0x1000);
				assert(base != nullptr);
				UML_LOAD(block, I1, base, 0, SIZE_DWORD, SCALE_x4);                 // load    i1,base,dword
				UML_ADD(block, I0, I0, I1);                     // add     i0,i0,i1

				sum += seqhead->delay.first()->opptr.l[0];
			}

			UML_CMP(block, I0, sum);                                    // cmp     i0,opptr[0]
			UML_EXHc(block, COND_NE, *m_nocode, epc(seqhead));     // exne    nocode,seqhead->pc
		}
	}

	/* full verification; sum up everything */
	else
	{
		UINT32 sum = 0;
		void *base = m_direct->read_ptr(seqhead->physpc | 0x1000);
		UML_LOAD(block, I0, base, 0, SIZE_DWORD, SCALE_x4);                             // load    i0,base,0,dword
		sum += seqhead->opptr.l[0];
		for (curdesc = seqhead->next(); curdesc != seqlast->next(); curdesc = curdesc->next())
			if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
			{
				base = m_direct->read_ptr(curdesc->physpc | 0x1000);
				assert(base != nullptr);
				UML_LOAD(block, I1, base, 0, SIZE_DWORD, SCALE_x4);                     // load    i1,base,dword
				UML_ADD(block, I0, I0, I1);                         // add     i0,i0,i1
				sum += curdesc->opptr.l[0];

				if (curdesc->delay.first() != nullptr && (curdesc == seqlast || (curdesc->next() != nullptr && curdesc->next()->physpc != curdesc->delay.first()->physpc)))
				{
					base = m_direct->read_ptr(curdesc->delay.first()->physpc | 0x1000);
					assert(base != nullptr);
					UML_LOAD(block, I1, base, 0, SIZE_DWORD, SCALE_x4);                 // load    i1,base,dword
					UML_ADD(block, I0, I0, I1);                     // add     i0,i0,i1

					sum += curdesc->delay.first()->opptr.l[0];
				}
			}
		UML_CMP(block, I0, sum);                                            // cmp     i0,sum
		UML_EXHc(block, COND_NE, *m_nocode, epc(seqhead));         // exne    nocode,seqhead->pc
	}
}


/*-------------------------------------------------
    generate_sequence_instruction - generate code
    for a single instruction in a sequence
-------------------------------------------------*/

void rsp_device::generate_sequence_instruction(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	offs_t expc;

	/* add an entry for the log */
	if (m_drcuml->logging() && !(desc->flags & OPFLAG_VIRTUAL_NOOP))
		log_add_disasm_comment(block, desc->pc, desc->opptr.l[0]);

	/* set the PC map variable */
	expc = (desc->flags & OPFLAG_IN_DELAY_SLOT) ? desc->pc - 3 : desc->pc;
	UML_MAPVAR(block, MAPVAR_PC, expc);                                             // mapvar  PC,expc

	/* accumulate total cycles */
	compiler->cycles += desc->cycles;

	/* update the icount map variable */
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);                             // mapvar  CYCLES,compiler->cycles

	/* if we are debugging, call the debugger */
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		UML_MOV(block, mem(&m_rsp_state->pc), desc->pc);                                // mov     [pc],desc->pc
		save_fast_iregs(block);
		UML_DEBUG(block, desc->pc);                                         // debug   desc->pc
	}

	/* if we hit an unmapped address, fatal error */
#if 0
	if (desc->flags & OPFLAG_COMPILER_UNMAPPED)
	{
		UML_MOV(block, mem(&m_rsp_state->pc), desc->pc);                               // mov     [pc],desc->pc
		save_fast_iregs(block);
		UML_EXIT(block, EXECUTE_UNMAPPED_CODE);                             // exit EXECUTE_UNMAPPED_CODE
	}
#endif

	/* otherwise, unless this is a virtual no-op, it's a regular instruction */
	/*else*/ if (!(desc->flags & OPFLAG_VIRTUAL_NOOP))
	{
		/* compile the instruction */
		if (!generate_opcode(block, compiler, desc))
		{
			UML_MOV(block, mem(&m_rsp_state->pc), desc->pc);                            // mov     [pc],desc->pc
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_unimplemented, this);                             // callc   cfunc_unimplemented
		}
	}
}

/*------------------------------------------------------------------
    generate_branch
------------------------------------------------------------------*/

void rsp_device::generate_branch(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	compiler_state compiler_temp = *compiler;

	/* update the cycles and jump through the hash table to the target */
	if (desc->targetpc != BRANCH_TARGET_DYNAMIC)
	{
		generate_update_cycles(block, &compiler_temp, desc->targetpc, TRUE);    // <subtract cycles>
		if (desc->flags & OPFLAG_INTRABLOCK_BRANCH)
			UML_JMP(block, desc->targetpc | 0x80000000);                        // jmp     desc->targetpc
		else
			UML_HASHJMP(block, 0, desc->targetpc, *m_nocode);                   // hashjmp <mode>,desc->targetpc,nocode
	}
	else
	{
		generate_update_cycles(block, &compiler_temp, mem(&m_rsp_state->jmpdest), TRUE);    // <subtract cycles>
		UML_HASHJMP(block, 0, mem(&m_rsp_state->jmpdest), *m_nocode);                       // hashjmp <mode>,<rsreg>,nocode
	}
}

/*------------------------------------------------------------------
    generate_delay_slot_and_branch
------------------------------------------------------------------*/

void rsp_device::generate_delay_slot_and_branch(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT8 linkreg)
{
	compiler_state compiler_temp = *compiler;
	UINT32 op = desc->opptr.l[0];

	/* fetch the target register if dynamic, in case it is modified by the delay slot */
	if (desc->targetpc == BRANCH_TARGET_DYNAMIC)
	{
		UML_AND(block, mem(&m_rsp_state->jmpdest), R32(RSREG), 0x00000fff);
		UML_OR(block, mem(&m_rsp_state->jmpdest), mem(&m_rsp_state->jmpdest), 0x1000);
	}

	/* set the link if needed -- before the delay slot */
	if (linkreg != 0)
	{
		UML_MOV(block, R32(linkreg), (INT32)(desc->pc + 8));                    // mov    <linkreg>,desc->pc + 8
	}

	/* compile the delay slot using temporary compiler state */
	assert(desc->delay.first() != nullptr);
	generate_sequence_instruction(block, &compiler_temp, desc->delay.first());     // <next instruction>

	generate_branch(block, compiler, desc);

	/* update the label */
	compiler->labelnum = compiler_temp.labelnum;

	/* reset the mapvar to the current cycles and account for skipped slots */
	compiler->cycles += desc->skipslots;
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);                             // mapvar  CYCLES,compiler->cycles
}

int rsp_device::generate_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	int in_delay_slot = ((desc->flags & OPFLAG_IN_DELAY_SLOT) != 0);
	UINT32 op = desc->opptr.l[0];
	UINT8 opswitch = op >> 26;
	code_label skip;

	switch (opswitch)
	{
		/* ----- sub-groups ----- */

		case 0x00:  /* SPECIAL - MIPS I */
			return generate_special(block, compiler, desc);

		case 0x01:  /* REGIMM - MIPS I */
			return generate_regimm(block, compiler, desc);

		/* ----- jumps and branches ----- */

		case 0x02:  /* J - MIPS I */
			generate_delay_slot_and_branch(block, compiler, desc, 0);      // <next instruction + hashjmp>
			return TRUE;

		case 0x03:  /* JAL - MIPS I */
			generate_delay_slot_and_branch(block, compiler, desc, 31);     // <next instruction + hashjmp>
			return TRUE;

		case 0x04:  /* BEQ - MIPS I */
			UML_CMP(block, R32(RSREG), R32(RTREG));                             // cmp    <rsreg>,<rtreg>
			UML_JMPc(block, COND_NE, skip = compiler->labelnum++);              // jmp    skip,NE
			generate_delay_slot_and_branch(block, compiler, desc, 0);      // <next instruction + hashjmp>
			UML_LABEL(block, skip);                                             // skip:
			return TRUE;

		case 0x05:  /* BNE - MIPS I */
			UML_CMP(block, R32(RSREG), R32(RTREG));                             // dcmp    <rsreg>,<rtreg>
			UML_JMPc(block, COND_E, skip = compiler->labelnum++);                       // jmp     skip,E
			generate_delay_slot_and_branch(block, compiler, desc, 0);      // <next instruction + hashjmp>
			UML_LABEL(block, skip);                                             // skip:
			return TRUE;

		case 0x06:  /* BLEZ - MIPS I */
			if (RSREG != 0)
			{
				UML_CMP(block, R32(RSREG), 0);                              // dcmp    <rsreg>,0
				UML_JMPc(block, COND_G, skip = compiler->labelnum++);                   // jmp     skip,G
				generate_delay_slot_and_branch(block, compiler, desc, 0);  // <next instruction + hashjmp>
				UML_LABEL(block, skip);                                         // skip:
			}
			else
				generate_delay_slot_and_branch(block, compiler, desc, 0);  // <next instruction + hashjmp>
			return TRUE;

		case 0x07:  /* BGTZ - MIPS I */
			UML_CMP(block, R32(RSREG), 0);                                  // dcmp    <rsreg>,0
			UML_JMPc(block, COND_LE, skip = compiler->labelnum++);                  // jmp     skip,LE
			generate_delay_slot_and_branch(block, compiler, desc, 0);      // <next instruction + hashjmp>
			UML_LABEL(block, skip);                                             // skip:
			return TRUE;


		/* ----- immediate arithmetic ----- */

		case 0x0f:  /* LUI - MIPS I */
			if (RTREG != 0)
				UML_MOV(block, R32(RTREG), SIMMVAL << 16);                  // dmov    <rtreg>,SIMMVAL << 16
			return TRUE;

		case 0x08:  /* ADDI - MIPS I */
		case 0x09:  /* ADDIU - MIPS I */
			if (RTREG != 0)
			{
				UML_ADD(block, R32(RTREG), R32(RSREG), SIMMVAL);                // add     i0,<rsreg>,SIMMVAL,V
			}
			return TRUE;

		case 0x0a:  /* SLTI - MIPS I */
			if (RTREG != 0)
			{
				UML_CMP(block, R32(RSREG), SIMMVAL);                            // dcmp    <rsreg>,SIMMVAL
				UML_SETc(block, COND_L, R32(RTREG));                                    // dset    <rtreg>,l
			}
			return TRUE;

		case 0x0b:  /* SLTIU - MIPS I */
			if (RTREG != 0)
			{
				UML_CMP(block, R32(RSREG), SIMMVAL);                            // dcmp    <rsreg>,SIMMVAL
				UML_SETc(block, COND_B, R32(RTREG));                                    // dset    <rtreg>,b
			}
			return TRUE;


		case 0x0c:  /* ANDI - MIPS I */
			if (RTREG != 0)
				UML_AND(block, R32(RTREG), R32(RSREG), UIMMVAL);                // dand    <rtreg>,<rsreg>,UIMMVAL
			return TRUE;

		case 0x0d:  /* ORI - MIPS I */
			if (RTREG != 0)
				UML_OR(block, R32(RTREG), R32(RSREG), UIMMVAL);             // dor     <rtreg>,<rsreg>,UIMMVAL
			return TRUE;

		case 0x0e:  /* XORI - MIPS I */
			if (RTREG != 0)
				UML_XOR(block, R32(RTREG), R32(RSREG), UIMMVAL);                // dxor    <rtreg>,<rsreg>,UIMMVAL
			return TRUE;

		/* ----- memory load operations ----- */

		case 0x20:  /* LB - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read8);                                    // callh   read8
			if (RTREG != 0)
				UML_SEXT(block, R32(RTREG), I0, SIZE_BYTE);                     // dsext   <rtreg>,i0,byte
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x21:  /* LH - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read16);                               // callh   read16
			if (RTREG != 0)
				UML_SEXT(block, R32(RTREG), I0, SIZE_WORD);                     // dsext   <rtreg>,i0,word
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x23:  /* LW - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read32);                               // callh   read32
			if (RTREG != 0)
				UML_MOV(block, R32(RTREG), I0);
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x24:  /* LBU - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read8);                                    // callh   read8
			if (RTREG != 0)
				UML_AND(block, R32(RTREG), I0, 0xff);                   // dand    <rtreg>,i0,0xff
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x25:  /* LHU - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read16);                               // callh   read16
			if (RTREG != 0)
				UML_AND(block, R32(RTREG), I0, 0xffff);                 // dand    <rtreg>,i0,0xffff
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x32:  /* LWC2 - MIPS I */
			return m_cop2->generate_lwc2(block, compiler, desc);


		/* ----- memory store operations ----- */

		case 0x28:  /* SB - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, I1, R32(RTREG));                                 // mov     i1,<rtreg>
			UML_CALLH(block, *m_write8);                               // callh   write8
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x29:  /* SH - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, I1, R32(RTREG));                                 // mov     i1,<rtreg>
			UML_CALLH(block, *m_write16);                              // callh   write16
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x2b:  /* SW - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, I1, R32(RTREG));                                 // mov     i1,<rtreg>
			UML_CALLH(block, *m_write32);                              // callh   write32
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x3a:  /* SWC2 - MIPS I */
			return m_cop2->generate_swc2(block, compiler, desc);

		/* ----- coprocessor instructions ----- */

		case 0x10:  /* COP0 - MIPS I */
			return generate_cop0(block, compiler, desc);

		case 0x12:  /* COP2 - MIPS I */
			return m_cop2->generate_cop2(block, compiler, desc);


		/* ----- unimplemented/illegal instructions ----- */

		//default:    /* ??? */       invalid_instruction(op);                                                break;
	}

	return FALSE;
}


/*-------------------------------------------------
    generate_special - compile opcodes in the
    'SPECIAL' group
-------------------------------------------------*/

int rsp_device::generate_special(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];
	UINT8 opswitch = op & 63;
	//code_label skip;

	switch (opswitch)
	{
		/* ----- shift instructions ----- */

		case 0x00:  /* SLL - MIPS I */
			if (RDREG != 0)
			{
				UML_SHL(block, R32(RDREG), R32(RTREG), SHIFT);
			}
			return TRUE;

		case 0x02:  /* SRL - MIPS I */
			if (RDREG != 0)
			{
				UML_SHR(block, R32(RDREG), R32(RTREG), SHIFT);
			}
			return TRUE;

		case 0x03:  /* SRA - MIPS I */
			if (RDREG != 0)
			{
				UML_SAR(block, R32(RDREG), R32(RTREG), SHIFT);
			}
			return TRUE;

		case 0x04:  /* SLLV - MIPS I */
			if (RDREG != 0)
			{
				UML_SHL(block, R32(RDREG), R32(RTREG), R32(RSREG));
			}
			return TRUE;

		case 0x06:  /* SRLV - MIPS I */
			if (RDREG != 0)
			{
				UML_SHR(block, R32(RDREG), R32(RTREG), R32(RSREG));
			}
			return TRUE;

		case 0x07:  /* SRAV - MIPS I */
			if (RDREG != 0)
			{
				UML_SAR(block, R32(RDREG), R32(RTREG), R32(RSREG));
			}
			return TRUE;

		/* ----- basic arithmetic ----- */

		case 0x20:  /* ADD - MIPS I */
		case 0x21:  /* ADDU - MIPS I */
			if (RDREG != 0)
			{
				UML_ADD(block, R32(RDREG), R32(RSREG), R32(RTREG));
			}
			return TRUE;

		case 0x22:  /* SUB - MIPS I */
		case 0x23:  /* SUBU - MIPS I */
			if (RDREG != 0)
			{
				UML_SUB(block, R32(RDREG), R32(RSREG), R32(RTREG));
			}
			return TRUE;

		/* ----- basic logical ops ----- */

		case 0x24:  /* AND - MIPS I */
			if (RDREG != 0)
			{
				UML_AND(block, R32(RDREG), R32(RSREG), R32(RTREG));             // dand     <rdreg>,<rsreg>,<rtreg>
			}
			return TRUE;

		case 0x25:  /* OR - MIPS I */
			if (RDREG != 0)
			{
				UML_OR(block, R32(RDREG), R32(RSREG), R32(RTREG));                  // dor      <rdreg>,<rsreg>,<rtreg>
			}
			return TRUE;

		case 0x26:  /* XOR - MIPS I */
			if (RDREG != 0)
			{
				UML_XOR(block, R32(RDREG), R32(RSREG), R32(RTREG));             // dxor     <rdreg>,<rsreg>,<rtreg>
			}
			return TRUE;

		case 0x27:  /* NOR - MIPS I */
			if (RDREG != 0)
			{
				UML_OR(block, I0, R32(RSREG), R32(RTREG));                  // dor      i0,<rsreg>,<rtreg>
				UML_XOR(block, R32(RDREG), I0, (UINT64)~0);             // dxor     <rdreg>,i0,~0
			}
			return TRUE;


		/* ----- basic comparisons ----- */

		case 0x2a:  /* SLT - MIPS I */
			if (RDREG != 0)
			{
				UML_CMP(block, R32(RSREG), R32(RTREG));                         // dcmp    <rsreg>,<rtreg>
				UML_SETc(block, COND_L, R32(RDREG));                                    // dset    <rdreg>,l
			}
			return TRUE;

		case 0x2b:  /* SLTU - MIPS I */
			if (RDREG != 0)
			{
				UML_CMP(block, R32(RSREG), R32(RTREG));                         // dcmp    <rsreg>,<rtreg>
				UML_SETc(block, COND_B, R32(RDREG));                                    // dset    <rdreg>,b
			}
			return TRUE;


		/* ----- jumps and branches ----- */

		case 0x08:  /* JR - MIPS I */
			generate_delay_slot_and_branch(block, compiler, desc, 0);      // <next instruction + hashjmp>
			return TRUE;

		case 0x09:  /* JALR - MIPS I */
			generate_delay_slot_and_branch(block, compiler, desc, RDREG);  // <next instruction + hashjmp>
			return TRUE;


		/* ----- system calls ----- */

		case 0x0d:  /* BREAK - MIPS I */
			UML_MOV(block, mem(&m_rsp_state->arg0), 3);                   // mov     [arg0],3
			UML_CALLC(block, cfunc_sp_set_status_cb, this);                      // callc   cfunc_sp_set_status_cb
			UML_MOV(block, mem(&m_rsp_state->icount), 0);                       // mov icount, #0
			UML_MOV(block, mem(&m_rsp_state->jmpdest), desc->targetpc);

			generate_branch(block, compiler, desc);

			UML_EXIT(block, EXECUTE_OUT_OF_CYCLES);
			return TRUE;
	}
	return FALSE;
}



/*-------------------------------------------------
    generate_regimm - compile opcodes in the
    'REGIMM' group
-------------------------------------------------*/

int rsp_device::generate_regimm(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];
	UINT8 opswitch = RTREG;
	code_label skip;

	switch (opswitch)
	{
		case 0x00:  /* BLTZ */
		case 0x10:  /* BLTZAL */
			if (RSREG != 0)
			{
				UML_CMP(block, R32(RSREG), 0);                              // dcmp    <rsreg>,0
				UML_JMPc(block, COND_GE, skip = compiler->labelnum++);              // jmp     skip,GE
				generate_delay_slot_and_branch(block, compiler, desc, (opswitch & 0x10) ? 31 : 0);
																					// <next instruction + hashjmp>
				UML_LABEL(block, skip);                                         // skip:
			}
			return TRUE;

		case 0x01:  /* BGEZ */
		case 0x11:  /* BGEZAL */
			if (RSREG != 0)
			{
				UML_CMP(block, R32(RSREG), 0);                              // dcmp    <rsreg>,0
				UML_JMPc(block, COND_L, skip = compiler->labelnum++);                   // jmp     skip,L
				generate_delay_slot_and_branch(block, compiler, desc, (opswitch & 0x10) ? 31 : 0);
																					// <next instruction + hashjmp>
				UML_LABEL(block, skip);                                         // skip:
			}
			else
				generate_delay_slot_and_branch(block, compiler, desc, (opswitch & 0x10) ? 31 : 0);
																					// <next instruction + hashjmp>
			return TRUE;
	}
	return FALSE;
}


/*-------------------------------------------------
    generate_cop0 - compile COP0 opcodes
-------------------------------------------------*/

int rsp_device::generate_cop0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];
	UINT8 opswitch = RSREG;

	switch (opswitch)
	{
		case 0x00:  /* MFCz */
			if (RTREG != 0)
			{
				UML_MOV(block, mem(&m_rsp_state->arg0), RDREG);               // mov     [arg0],<rdreg>
				UML_MOV(block, mem(&m_rsp_state->arg1), RTREG);               // mov     [arg1],<rtreg>
				UML_CALLC(block, cfunc_get_cop0_reg, this);                          // callc   cfunc_get_cop0_reg
				if(RDREG == 2)
				{
					generate_update_cycles(block, compiler, mem(&m_rsp_state->pc), TRUE);
					UML_HASHJMP(block, 0, mem(&m_rsp_state->pc), *m_nocode);
				}
			}
			return TRUE;

		case 0x04:  /* MTCz */
			UML_MOV(block, mem(&m_rsp_state->arg0), RDREG);                   // mov     [arg0],<rdreg>
			UML_MOV(block, mem(&m_rsp_state->arg1), R32(RTREG));                  // mov     [arg1],rtreg
			UML_CALLC(block, cfunc_set_cop0_reg, this);                              // callc   cfunc_set_cop0_reg
			return TRUE;
	}

	return FALSE;
}

/***************************************************************************
    CODE LOGGING HELPERS
***************************************************************************/

/*-------------------------------------------------
    log_add_disasm_comment - add a comment
    including disassembly of a RSP instruction
-------------------------------------------------*/

void rsp_device::log_add_disasm_comment(drcuml_block *block, UINT32 pc, UINT32 op)
{
	if (m_drcuml->logging())
	{
		char buffer[100];
		rsp_dasm_one(buffer, pc, op);
		block->append_comment("%08X: %s", pc, buffer);                                  // comment
	}
}

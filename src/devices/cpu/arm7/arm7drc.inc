// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,R. Belmont,Ryan Holtz
/*****************************************************************************
 *
 *   arm7drc.inc
 *   Portable CPU Emulator for 32-bit ARM v3/4/5/6
 *
 *   Copyright Steve Ellenoff, all rights reserved.
 *   Thumb, DSP, and MMU support and many bugfixes by R. Belmont and Ryan Holtz.
 *   Dyanmic Recompiler (DRC) / Just In Time Compiler (JIT) by Ryan Holtz.
 *
 *  This work is based on:
 *  #1) 'Atmel Corporation ARM7TDMI (Thumb) Datasheet - January 1999'
 *  #2) Arm 2/3/6 emulator By Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino (MAME CORE 0.76)
 *
 *****************************************************************************/

/******************************************************************************
 *  Notes:

    ** This is a plain vanilla implementation of an ARM7 cpu which incorporates my ARM7 core.
       It can be used as is, or used to demonstrate how to utilize the arm7 core to create a cpu
       that uses the core, since there are numerous different mcu packages that incorporate an arm7 core.

       See the notes in the arm7core.c file itself regarding issues/limitations of the arm7 core.
    **
*****************************************************************************/


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define SINGLE_INSTRUCTION_MODE         (0)

/***************************************************************************
    CONSTANTS
***************************************************************************/

#include "arm7tdrc.inc"

/* map variables */
#define MAPVAR_PC                       uml::M0
#define MAPVAR_CYCLES                   uml::M1

/* size of the execution code cache */
#define CACHE_SIZE                      (32 * 1024 * 1024)

/* compilation boundaries -- how far back/forward does the analysis extend? */
#define COMPILE_BACKWARDS_BYTES         128
#define COMPILE_FORWARDS_BYTES          512
#define COMPILE_MAX_INSTRUCTIONS        ((COMPILE_BACKWARDS_BYTES/4) + (COMPILE_FORWARDS_BYTES/4))
#define COMPILE_MAX_SEQUENCE            64

/* exit codes */
#define EXECUTE_OUT_OF_CYCLES           0
#define EXECUTE_MISSING_CODE            1
#define EXECUTE_UNMAPPED_CODE           2
#define EXECUTE_RESET_CACHE             3


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    epc - compute the exception PC from a
    descriptor
-------------------------------------------------*/

INLINE UINT32 epc(const opcode_desc *desc)
{
	return desc->pc;
}


/*-------------------------------------------------
    alloc_handle - allocate a handle if not
    already allocated
-------------------------------------------------*/

INLINE void alloc_handle(drcuml_state *drcuml, uml::code_handle **handleptr, const char *name)
{
	if (*handleptr == nullptr)
		*handleptr = drcuml->handle_alloc(name);
}


/*-------------------------------------------------
    load_fast_iregs - load any fast integer
    registers
-------------------------------------------------*/

void arm7_cpu_device::load_fast_iregs(drcuml_block *block)
{
	int regnum;

	for (regnum = 0; regnum < ARRAY_LENGTH(m_impstate.regmap); regnum++)
		if (m_impstate.regmap[regnum].is_int_register())
			UML_DMOV(block, uml::ireg(m_impstate.regmap[regnum].ireg() - uml::REG_I0), uml::mem(&m_r[regnum]));
}


/*-------------------------------------------------
    save_fast_iregs - save any fast integer
    registers
-------------------------------------------------*/

void arm7_cpu_device::save_fast_iregs(drcuml_block *block)
{
	int regnum;

	for (regnum = 0; regnum < ARRAY_LENGTH(m_impstate.regmap); regnum++)
		if (m_impstate.regmap[regnum].is_int_register())
			UML_DMOV(block, uml::mem(&m_r[regnum]), uml::ireg(m_impstate.regmap[regnum].ireg() - uml::REG_I0));
}



/***************************************************************************
    CORE CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    arm7_init - initialize the processor
-------------------------------------------------*/

void arm7_cpu_device::arm7_drc_init()
{
	drc_cache *cache;
	drcbe_info beinfo;
	UINT32 flags = 0;

	/* allocate enough space for the cache and the core */
	cache = auto_alloc(machine(), drc_cache(CACHE_SIZE));
	if (cache == nullptr)
		fatalerror("Unable to allocate cache of size %d\n", (UINT32)(CACHE_SIZE));

	/* allocate the implementation-specific state from the full cache */
	memset(&m_impstate, 0, sizeof(m_impstate));
	m_impstate.cache = cache;

	/* initialize the UML generator */
	m_impstate.drcuml = new drcuml_state(*this, *cache, flags, 1, 32, 1);

	/* add symbols for our stuff */
	m_impstate.drcuml->symbol_add(&m_icount, sizeof(m_icount), "icount");
	for (int regnum = 0; regnum < 37; regnum++)
	{
		char buf[10];
		sprintf(buf, "r%d", regnum);
		m_impstate.drcuml->symbol_add(&m_r[regnum], sizeof(m_r[regnum]), buf);
	}
	m_impstate.drcuml->symbol_add(&m_impstate.mode, sizeof(m_impstate.mode), "mode");
	m_impstate.drcuml->symbol_add(&m_impstate.arg0, sizeof(m_impstate.arg0), "arg0");
	m_impstate.drcuml->symbol_add(&m_impstate.arg1, sizeof(m_impstate.arg1), "arg1");
	m_impstate.drcuml->symbol_add(&m_impstate.numcycles, sizeof(m_impstate.numcycles), "numcycles");
	//m_impstate.drcuml->symbol_add(&m_impstate.fpmode, sizeof(m_impstate.fpmode), "fpmode"); // TODO

	/* initialize the front-end helper */
	//m_impstate.drcfe = auto_alloc(machine(), arm7_frontend(this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE));

	/* allocate memory for cache-local state and initialize it */
	//memcpy(&m_impstate.fpmode, fpmode_source, sizeof(fpmode_source)); // TODO

	/* compute the register parameters */
	for (int regnum = 0; regnum < 37; regnum++)
	{
		m_impstate.regmap[regnum] = (regnum == 0) ? uml::parameter(0) : uml::parameter::make_memory(&m_r[regnum]);
	}

	/* if we have registers to spare, assign r2, r3, r4 to leftovers */
	//if (!DISABLE_FAST_REGISTERS) // TODO
	{
		m_impstate.drcuml->get_backend_info(beinfo);
		if (beinfo.direct_iregs > 4)
		{   // PC
			m_impstate.regmap[eR15] = uml::I4;
		}
		if (beinfo.direct_iregs > 5)
		{   // Status
			m_impstate.regmap[eCPSR] = uml::I5;
		}
		if (beinfo.direct_iregs > 6)
		{   // SP
			m_impstate.regmap[eR13] = uml::I6;
		}
	}

	/* mark the cache dirty so it is updated on next execute */
	m_impstate.cache_dirty = TRUE;
}


/*-------------------------------------------------
    arm7_execute - execute the CPU for the
    specified number of cycles
-------------------------------------------------*/

void arm7_cpu_device::execute_run_drc()
{
	drcuml_state *drcuml = m_impstate.drcuml;
	int execute_result;

	/* reset the cache if dirty */
	if (m_impstate.cache_dirty)
		code_flush_cache();
	m_impstate.cache_dirty = FALSE;

	/* execute */
	do
	{
		/* run as much as we can */
		execute_result = drcuml->execute(*m_impstate.entry);

		/* if we need to recompile, do it */
		if (execute_result == EXECUTE_MISSING_CODE)
			code_compile_block(m_impstate.mode, m_r[eR15]);
		else if (execute_result == EXECUTE_UNMAPPED_CODE)
			fatalerror("Attempted to execute unmapped code at PC=%08X\n", m_r[eR15]);
		else if (execute_result == EXECUTE_RESET_CACHE)
			code_flush_cache();

	} while (execute_result != EXECUTE_OUT_OF_CYCLES);
}

/*-------------------------------------------------
    arm7_exit - cleanup from execution
-------------------------------------------------*/

void arm7_cpu_device::arm7_drc_exit()
{
	/* clean up the DRC */
	//auto_free(machine(), m_impstate.drcfe);
	delete m_impstate.drcuml;
	auto_free(machine(), m_impstate.cache);
}


/*-------------------------------------------------
    arm7drc_set_options - configure DRC options
-------------------------------------------------*/

void arm7_cpu_device::arm7drc_set_options(UINT32 options)
{
	m_impstate.drcoptions = options;
}


/*-------------------------------------------------
    arm7drc_add_fastram - add a new fastram
    region
-------------------------------------------------*/

void arm7_cpu_device::arm7drc_add_fastram(offs_t start, offs_t end, UINT8 readonly, void *base)
{
	if (m_impstate.fastram_select < ARRAY_LENGTH(m_impstate.fastram))
	{
		m_impstate.fastram[m_impstate.fastram_select].start = start;
		m_impstate.fastram[m_impstate.fastram_select].end = end;
		m_impstate.fastram[m_impstate.fastram_select].readonly = readonly;
		m_impstate.fastram[m_impstate.fastram_select].base = base;
		m_impstate.fastram_select++;
	}
}


/*-------------------------------------------------
    arm7drc_add_hotspot - add a new hotspot
-------------------------------------------------*/

void arm7_cpu_device::arm7drc_add_hotspot(offs_t pc, UINT32 opcode, UINT32 cycles)
{
	if (m_impstate.hotspot_select < ARRAY_LENGTH(m_impstate.hotspot))
	{
		m_impstate.hotspot[m_impstate.hotspot_select].pc = pc;
		m_impstate.hotspot[m_impstate.hotspot_select].opcode = opcode;
		m_impstate.hotspot[m_impstate.hotspot_select].cycles = cycles;
		m_impstate.hotspot_select++;
	}
}



/***************************************************************************
    CACHE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    code_flush_cache - flush the cache and
    regenerate static code
-------------------------------------------------*/

void arm7_cpu_device::code_flush_cache()
{
	/* empty the transient cache contents */
	m_impstate.drcuml->reset();

	try
	{
		/* generate the entry point and out-of-cycles handlers */
		static_generate_entry_point();
		static_generate_nocode_handler();
		static_generate_out_of_cycles();
		static_generate_tlb_translate(nullptr); // TODO FIXME
		static_generate_detect_fault(nullptr); // TODO FIXME
		//static_generate_tlb_mismatch();

		/* add subroutines for memory accesses */
		static_generate_memory_accessor(1, FALSE, FALSE, "read8",       &m_impstate.read8);
		static_generate_memory_accessor(1, TRUE,  FALSE, "write8",      &m_impstate.write8);
		static_generate_memory_accessor(2, FALSE, FALSE, "read16",      &m_impstate.read16);
		static_generate_memory_accessor(2, TRUE,  FALSE, "write16",     &m_impstate.write16);
		static_generate_memory_accessor(4, FALSE, FALSE, "read32",      &m_impstate.read32);
		static_generate_memory_accessor(4, TRUE,  FALSE, "write32",     &m_impstate.write32);
	}
	catch (drcuml_block::abort_compilation &)
	{
		fatalerror("Unrecoverable error generating static code\n");
	}
}


/*-------------------------------------------------
    code_compile_block - compile a block of the
    given mode at the specified pc
-------------------------------------------------*/

void arm7_cpu_device::code_compile_block(UINT8 mode, offs_t pc)
{
	drcuml_state *drcuml = m_impstate.drcuml;
	compiler_state compiler = { 0 };
	const opcode_desc *seqlast;
	int override = FALSE;

	g_profiler.start(PROFILER_DRC_COMPILE);

	/* get a description of this sequence */
	// TODO FIXME
	const opcode_desc *desclist = nullptr; //m_impstate.drcfe->describe_code(pc); // TODO
//  if (drcuml->logging() || drcuml->logging_native())
//      log_opcode_desc(drcuml, desclist, 0);

	/* if we get an error back, flush the cache and try again */
	bool succeeded = false;
	while (!succeeded)
	{
		try
		{
			/* start the block */
			drcuml_block *block = drcuml->begin_block(4096);

			/* loop until we get through all instruction sequences */
			for (const opcode_desc *seqhead = desclist; seqhead != nullptr; seqhead = seqlast->next())
			{
				const opcode_desc *curdesc;
				UINT32 nextpc;

				/* add a code log entry */
				if (drcuml->logging())
					block->append_comment("-------------------------");                     // comment

				/* determine the last instruction in this sequence */
				for (seqlast = seqhead; seqlast != nullptr; seqlast = seqlast->next())
					if (seqlast->flags & OPFLAG_END_SEQUENCE)
						break;
				assert(seqlast != nullptr);

				/* if we don't have a hash for this mode/pc, or if we are overriding all, add one */
				if (override || !drcuml->hash_exists(mode, seqhead->pc))
					UML_HASH(block, mode, seqhead->pc);                                     // hash    mode,pc

				/* if we already have a hash, and this is the first sequence, assume that we */
				/* are recompiling due to being out of sync and allow future overrides */
				else if (seqhead == desclist)
				{
					override = TRUE;
					UML_HASH(block, mode, seqhead->pc);                                     // hash    mode,pc
				}

				/* otherwise, redispatch to that fixed PC and skip the rest of the processing */
				else
				{
					UML_LABEL(block, seqhead->pc | 0x80000000);                             // label   seqhead->pc | 0x80000000
					UML_HASHJMP(block, 0, seqhead->pc, *m_impstate.nocode);
																							// hashjmp <mode>,seqhead->pc,nocode
					continue;
				}

				/* validate this code block if we're not pointing into ROM */
				if (m_program->get_write_ptr(seqhead->physpc) != nullptr)
					generate_checksum_block(block, &compiler, seqhead, seqlast);

				/* label this instruction, if it may be jumped to locally */
				if (seqhead->flags & OPFLAG_IS_BRANCH_TARGET)
					UML_LABEL(block, seqhead->pc | 0x80000000);                             // label   seqhead->pc | 0x80000000

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
				generate_update_cycles(block, &compiler, nextpc);          // <subtract cycles>

				/* if the last instruction can change modes, use a variable mode; otherwise, assume the same mode */
				/*if (seqlast->flags & OPFLAG_CAN_CHANGE_MODES)
				    UML_HASHJMP(block, uml::mem(&m_impstate.mode), nextpc, *m_impstate.nocode);
				                                                                            // hashjmp <mode>,nextpc,nocode
				else*/ if (seqlast->next() == nullptr || seqlast->next()->pc != nextpc)
					UML_HASHJMP(block, m_impstate.mode, nextpc, *m_impstate.nocode);
																							// hashjmp <mode>,nextpc,nocode
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
    cfunc_get_cycles - compute the total number
    of cycles executed so far
-------------------------------------------------*/

void arm7_cpu_device::cfunc_get_cycles()
{
	m_impstate.numcycles = total_cycles();
}


/*-------------------------------------------------
    cfunc_unimplemented - handler for
    unimplemented opcdes
-------------------------------------------------*/

void arm7_cpu_device::cfunc_unimplemented()
{
	UINT32 opcode = m_impstate.arg0;
	fatalerror("PC=%08X: Unimplemented op %08X\n", m_r[eR15], opcode);
}


/***************************************************************************
    STATIC CODEGEN
***************************************************************************/

/*-------------------------------------------------
    static_generate_entry_point - generate a
    static entry point
-------------------------------------------------*/

void arm7_cpu_device::static_generate_entry_point()
{
	drcuml_state *drcuml = m_impstate.drcuml;
	uml::code_label nodabt;
	uml::code_label nofiq;
	uml::code_label noirq;
	uml::code_label irq32;
	uml::code_label nopabd;
	uml::code_label nound;
	uml::code_label swi32;
	uml::code_label irqadjust;
	uml::code_label done;
	drcuml_block *block;

	block = drcuml->begin_block(110);

	/* forward references */
	//alloc_handle(drcuml, &m_impstate.exception_norecover[EXCEPTION_INTERRUPT], "interrupt_norecover");
	alloc_handle(drcuml, &m_impstate.nocode, "nocode");
	alloc_handle(drcuml, &m_impstate.detect_fault, "detect_fault");
	alloc_handle(drcuml, &m_impstate.tlb_translate, "tlb_translate");

	alloc_handle(drcuml, &m_impstate.entry, "entry");
	UML_HANDLE(block, *m_impstate.entry);                           // handle  entry

	/* load fast integer registers */
	load_fast_iregs(block);

	UML_CALLH(block, *m_impstate.check_irq);

	/* generate a hash jump via the current mode and PC */
	UML_HASHJMP(block, 0, uml::mem(&m_pc), *m_impstate.nocode);       // hashjmp 0,<pc>,nocode
	block->end();
}


/*-------------------------------------------------
    static_generate_check_irq - generate a handler
    to check IRQs
-------------------------------------------------*/

void arm7_cpu_device::static_generate_check_irq()
{
	drcuml_state *drcuml = m_impstate.drcuml;
	drcuml_block *block;
	uml::code_label noirq;
	int nodabt = 0;
	int nopabt = 0;
	int irqadjust = 0;
	int nofiq = 0;
	int irq32 = 0;
	int swi32 = 0;
	int done = 0;
	int label = 1;

	/* begin generating */
	block = drcuml->begin_block(120);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &m_impstate.check_irq, "check_irq");
	UML_HANDLE(block, *m_impstate.check_irq);                       // handle  check_irq
	/* Exception priorities:

	    Reset
	    Data abort
	    FIRQ
	    IRQ
	    Prefetch abort
	    Undefined instruction
	    Software Interrupt
	*/

	UML_ADD(block, uml::I0, uml::mem(&R15), 4);                                   // add      i0, PC, 4  ;insn pc

	// Data Abort
	UML_TEST(block, uml::mem(&m_pendingAbtD), 1);                          // test     pendingAbtD, 1
	UML_JMPc(block, uml::COND_Z, nodabt = label++);                          // jmpz     nodabt

	UML_ROLINS(block, uml::mem(&GET_CPSR), eARM7_MODE_ABT, 0, MODE_FLAG);     // rolins   CPSR, eARM7_MODE_ABT, 0, MODE_FLAG
	UML_MOV(block, uml::mem(&GET_REGISTER(14)), uml::I0);                    // mov      LR, i0
	UML_MOV(block, uml::mem(&GET_REGISTER(SPSR)), uml::mem(&GET_CPSR));      // mov      SPSR, CPSR
	UML_OR(block, uml::mem(&GET_CPSR), uml::mem(&GET_CPSR), I_MASK);              // or       CPSR, CPSR, I_MASK
	UML_ROLAND(block, uml::mem(&GET_CPSR), uml::mem(&GET_CPSR), 0, ~T_MASK);      // roland   CPSR, CPSR, 0, ~T_MASK
	UML_MOV(block, uml::mem(&R15), 0x00000010);                              // mov      PC, 0x10 (Data Abort vector address)
	UML_MOV(block, uml::mem(&m_pendingAbtD), 0);                           // mov      pendingAbtD, 0
	UML_JMP(block, irqadjust = label++);                                // jmp      irqadjust

	UML_LABEL(block, nodabt);                                           // nodabt:

	// FIQ
	UML_TEST(block, uml::mem(&m_pendingFiq), 1);                           // test     pendingFiq, 1
	UML_JMPc(block, uml::COND_Z, nofiq = label++);                           // jmpz     nofiq
	UML_TEST(block, uml::mem(&GET_CPSR), F_MASK);                            // test     CPSR, F_MASK
	UML_JMPc(block, uml::COND_Z, nofiq);                                     // jmpz     nofiq

	UML_MOV(block, uml::mem(&GET_REGISTER(14)), uml::I0);                    // mov      LR, i0
	UML_MOV(block, uml::mem(&GET_REGISTER(SPSR)), uml::mem(&GET_CPSR));      // mov      SPSR, CPSR
	UML_OR(block, uml::mem(&GET_CPSR), uml::mem(&GET_CPSR), I_MASK | F_MASK);     // or       CPSR, CPSR, I_MASK | F_MASK
	UML_ROLAND(block, uml::mem(&GET_CPSR), uml::mem(&GET_CPSR), 0, ~T_MASK);          // roland   CPSR, CPSR, 0, ~T_MASK
	UML_MOV(block, uml::mem(&R15), 0x0000001c);                              // mov      PC, 0x1c (FIQ vector address)
	UML_MOV(block, uml::mem(&m_pendingFiq), 0);                            // mov      pendingFiq, 0
	UML_JMP(block, irqadjust);                                          // jmp      irqadjust

	UML_LABEL(block, nofiq);                                            // nofiq:

	// IRQ
	UML_TEST(block, uml::mem(&m_pendingIrq), 1);                           // test     pendingIrq, 1
	UML_JMPc(block, uml::COND_Z, noirq = label++);                           // jmpz     noirq
	UML_TEST(block, uml::mem(&GET_CPSR), I_MASK);                            // test     CPSR, I_MASK
	UML_JMPc(block, uml::COND_Z, noirq);                                     // jmpz     noirq

	UML_MOV(block, uml::mem(&GET_REGISTER(14)), uml::I0);                    // mov      LR, i0
	UML_TEST(block, uml::mem(&GET_CPSR), SR_MODE32);                         // test     CPSR, MODE32
	UML_JMPc(block, uml::COND_NZ, irq32 = label++);                          // jmpnz    irq32
	UML_AND(block, uml::I1, uml::I0, 0xf4000000);                                 // and      i1, i0, 0xf4000000
	UML_OR(block, uml::mem(&R15), uml::I1, 0x0800001a);                           // or       PC, i1, 0x0800001a
	UML_AND(block, uml::I1, uml::mem(&GET_CPSR), 0x0fffff3f);                     // and      i1, CPSR, 0x0fffff3f
	UML_ROLAND(block, uml::I0, uml::mem(&R15), 32-20, 0x0000000c);                // roland   i0, R15, 32-20, 0x0000000c
	UML_ROLINS(block, uml::I0, uml::mem(&R15), 0, 0xf0000000);                    // rolins   i0, R15, 0, 0xf0000000
	UML_OR(block, uml::mem(&GET_CPSR), uml::I0, uml::I1);                              // or       CPSR, i0, i1
	UML_JMP(block, irqadjust);                                          // jmp      irqadjust

	UML_LABEL(block, irq32);                                            // irq32:
	UML_MOV(block, uml::mem(&GET_REGISTER(SPSR)), uml::mem(&GET_CPSR));      // mov      SPSR, CPSR
	UML_OR(block, uml::mem(&GET_CPSR), uml::mem(&GET_CPSR), I_MASK);              // or       CPSR, CPSR, I_MASK
	UML_ROLAND(block, uml::mem(&GET_CPSR), uml::mem(&GET_CPSR), 0, ~T_MASK);          // roland   CPSR, CPSR, 0, ~T_MASK
	UML_MOV(block, uml::mem(&R15), 0x00000018);                              // mov      PC, 0x18 (IRQ vector address)

	UML_JMP(block, irqadjust);                                          // jmp      irqadjust

	UML_LABEL(block, noirq);                                            // noirq:

	// Prefetch Abort
	UML_TEST(block, uml::mem(&m_pendingAbtP), 1);                          // test     pendingAbtP, 1
	UML_JMPc(block, uml::COND_Z, nopabt = label++);                          // jmpz     nopabt

	UML_ROLINS(block, uml::mem(&GET_CPSR), eARM7_MODE_ABT, 0, MODE_FLAG);     // rolins   CPSR, eARM7_MODE_ABT, 0, MODE_FLAG
	UML_MOV(block, uml::mem(&GET_REGISTER(14)), uml::I0);                    // mov      LR, i0
	UML_MOV(block, uml::mem(&GET_REGISTER(SPSR)), uml::mem(&GET_CPSR));      // mov      SPSR, CPSR
	UML_OR(block, uml::mem(&GET_CPSR), uml::mem(&GET_CPSR), I_MASK);              // or       CPSR, CPSR, I_MASK
	UML_ROLAND(block, uml::mem(&GET_CPSR), uml::mem(&GET_CPSR), 0, ~T_MASK);          // roland   CPSR, CPSR, 0, ~T_MASK
	UML_MOV(block, uml::mem(&R15), 0x0000000c);                              // mov      PC, 0x0c (Prefetch Abort vector address)
	UML_MOV(block, uml::mem(&m_pendingAbtP), 0);                           // mov      pendingAbtP, 0
	UML_JMP(block, irqadjust);                                          // jmp      irqadjust

	UML_LABEL(block, nopabt);                                           // nopabt:

	// Undefined instruction
	UML_TEST(block, uml::mem(&m_pendingUnd), 1);                           // test     pendingUnd, 1
	UML_JMPc(block, uml::COND_Z, nopabt = label++);                          // jmpz     nound

	UML_ROLINS(block, uml::mem(&GET_CPSR), eARM7_MODE_UND, 0, MODE_FLAG);     // rolins   CPSR, eARM7_MODE_UND, 0, MODE_FLAG
	UML_MOV(block, uml::I1, (UINT64)-4);                                             // mov      i1, -4
	UML_TEST(block, uml::mem(&GET_CPSR), T_MASK);                            // test     CPSR, T_MASK
	UML_MOVc(block, uml::COND_NZ, uml::I1, (UINT64)-2);                                   // movnz    i1, -2
	UML_ADD(block, uml::mem(&GET_REGISTER(14)), uml::I0, uml::I1);                // add      LR, i0, i1
	UML_MOV(block, uml::mem(&GET_REGISTER(SPSR)), uml::mem(&GET_CPSR));      // mov      SPSR, CPSR
	UML_OR(block, uml::mem(&GET_CPSR), uml::mem(&GET_CPSR), I_MASK);              // or       CPSR, CPSR, I_MASK
	UML_ROLAND(block, uml::mem(&GET_CPSR), uml::mem(&GET_CPSR), 0, ~T_MASK);          // roland   CPSR, CPSR, 0, ~T_MASK
	UML_MOV(block, uml::mem(&R15), 0x00000004);                              // mov      PC, 0x0c (Undefined Insn vector address)
	UML_MOV(block, uml::mem(&m_pendingUnd), 0);                            // mov      pendingUnd, 0
	UML_JMP(block, irqadjust);                                          // jmp      irqadjust

	UML_LABEL(block, nopabt);                                           // nopabt:

	// Software Interrupt
	UML_TEST(block, uml::mem(&m_pendingSwi), 1);                           // test     pendingSwi, 1
	UML_JMPc(block, uml::COND_Z, done = label++);                            // jmpz     done

	UML_ROLINS(block, uml::mem(&GET_CPSR), eARM7_MODE_SVC, 0, MODE_FLAG);     // rolins   CPSR, eARM7_MODE_SVC, 0, MODE_FLAG
	UML_MOV(block, uml::I1, (UINT64)-4);                                             // mov      i1, -4
	UML_TEST(block, uml::mem(&GET_CPSR), T_MASK);                            // test     CPSR, T_MASK
	UML_MOVc(block, uml::COND_NZ, uml::I1, (UINT64)-2);                                   // movnz    i1, -2
	UML_ADD(block, uml::mem(&GET_REGISTER(14)), uml::I0, uml::I1);                // add      LR, i0, i1

	UML_TEST(block, uml::mem(&GET_CPSR), SR_MODE32);                         // test     CPSR, MODE32
	UML_JMPc(block, uml::COND_NZ, swi32 = label++);                          // jmpnz    swi32
	UML_AND(block, uml::I1, uml::I0, 0xf4000000);                                 // and      i1, i0, 0xf4000000
	UML_OR(block, uml::mem(&R15), uml::I1, 0x0800001b);                           // or       PC, i1, 0x0800001b
	UML_AND(block, uml::I1, uml::mem(&GET_CPSR), 0x0fffff3f);                     // and      i1, CPSR, 0x0fffff3f
	UML_ROLAND(block, uml::I0, uml::mem(&R15), 32-20, 0x0000000c);                // roland   i0, R15, 32-20, 0x0000000c
	UML_ROLINS(block, uml::I0, uml::mem(&R15), 0, 0xf0000000);                    // rolins   i0, R15, 0, 0xf0000000
	UML_OR(block, uml::mem(&GET_CPSR), uml::I0, uml::I1);                              // or       CPSR, i0, i1
	UML_MOV(block, uml::mem(&m_pendingSwi), 0);                            // mov      pendingSwi, 0
	UML_JMP(block, irqadjust);                                          // jmp      irqadjust

	UML_LABEL(block, swi32);                                            // irq32:
	UML_MOV(block, uml::mem(&GET_REGISTER(SPSR)), uml::mem(&GET_CPSR));      // mov      SPSR, CPSR
	UML_OR(block, uml::mem(&GET_CPSR), uml::mem(&GET_CPSR), I_MASK);              // or       CPSR, CPSR, I_MASK
	UML_ROLAND(block, uml::mem(&GET_CPSR), uml::mem(&GET_CPSR), 0, ~T_MASK);          // roland   CPSR, CPSR, 0, ~T_MASK
	UML_MOV(block, uml::mem(&R15), 0x00000008);                              // mov      PC, 0x08 (SWI vector address)
	UML_MOV(block, uml::mem(&m_pendingSwi), 0);                            // mov      pendingSwi, 0
	UML_JMP(block, irqadjust);                                          // jmp      irqadjust

	UML_LABEL(block, irqadjust);                                        // irqadjust:
	UML_MOV(block, uml::I1, 0);                                              // mov      i1, 0
	UML_TEST(block, uml::mem(&COPRO_CTRL), COPRO_CTRL_MMU_EN | COPRO_CTRL_INTVEC_ADJUST);    // test COPRO_CTRL, MMU_EN | INTVEC_ADJUST
	UML_MOVc(block, uml::COND_NZ, uml::I1, 0xffff0000);                           // movnz    i1, 0xffff0000
	UML_OR(block, uml::mem(&R15), uml::mem(&R15), uml::I1);                             // or       PC, i1

	UML_LABEL(block, done);                                             // done:

	block->end();
}

/*-------------------------------------------------
    static_generate_nocode_handler - generate an
    exception handler for "out of code"
-------------------------------------------------*/

void arm7_cpu_device::static_generate_nocode_handler()
{
	drcuml_state *drcuml = m_impstate.drcuml;
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(10);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &m_impstate.nocode, "nocode");
	UML_HANDLE(block, *m_impstate.nocode);                                  // handle  nocode
	UML_GETEXP(block, uml::I0);                                                      // getexp  i0
	UML_MOV(block, uml::mem(&R15), uml::I0);                                              // mov     [pc],i0
	save_fast_iregs(block);
	UML_EXIT(block, EXECUTE_MISSING_CODE);                                      // exit    EXECUTE_MISSING_CODE

	block->end();
}


/*-------------------------------------------------
    static_generate_out_of_cycles - generate an
    out of cycles exception handler
-------------------------------------------------*/

void arm7_cpu_device::static_generate_out_of_cycles()
{
	drcuml_state *drcuml = m_impstate.drcuml;
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(10);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &m_impstate.out_of_cycles, "out_of_cycles");
	UML_HANDLE(block, *m_impstate.out_of_cycles);                       // handle  out_of_cycles
	UML_GETEXP(block, uml::I0);                                                  // getexp  i0
	UML_MOV(block, uml::mem(&R15), uml::I0);                                          // mov     <pc>,i0
	save_fast_iregs(block);
	UML_EXIT(block, EXECUTE_OUT_OF_CYCLES);                                 // exit    EXECUTE_OUT_OF_CYCLES

	block->end();
}


/*------------------------------------------------------------------
    static_generate_tlb_translate
------------------------------------------------------------------*/

void arm7_cpu_device::static_generate_detect_fault(uml::code_handle **handleptr)
{
	/* on entry, flags are in I2, vaddr is in I3, desc_lvl1 is in I4, ap is in R5 */
	/* on exit, fault result is in I6 */
	drcuml_state *drcuml = m_impstate.drcuml;
	drcuml_block *block;
	int donefault = 0;
	int checkuser = 0;
	int label = 1;

	/* begin generating */
	block = drcuml->begin_block(1024);

	/* add a global entry for this */
	alloc_handle(drcuml, &m_impstate.detect_fault, "detect_fault");
	UML_HANDLE(block, *m_impstate.detect_fault);                // handle   detect_fault

	UML_ROLAND(block, uml::I6, uml::I4, 32-4, 0x0f<<1);                       // roland   i6, i4, 32-4, 0xf<<1
	UML_ROLAND(block, uml::I6, uml::mem(&COPRO_DOMAIN_ACCESS_CONTROL), uml::I6, 3);// roland   i6, COPRO_DOMAIN_ACCESS_CONTROL, i6, 3
	// if permission == 3, FAULT_NONE
	UML_CMP(block, uml::I6, 3);                                          // cmp      i6, 3
	UML_MOVc(block, uml::COND_E, uml::I6, FAULT_NONE);                        // move     i6, FAULT_NONE
	UML_JMPc(block, uml::COND_E, donefault = label++);                   // jmpe     donefault
	// if permission == 0 || permission == 2, FAULT_DOMAIN
	UML_CMP(block, uml::I6, 1);                                          // cmp      i6, 1
	UML_MOVc(block, uml::COND_NE, uml::I6, FAULT_DOMAIN);                     // movne    i6, FAULT_DOMAIN
	UML_JMPc(block, uml::COND_NE, donefault);                            // jmpne    donefault

	// if permission == 1
	UML_CMP(block, uml::I5, 3);                                          // cmp      i5, 3
	UML_MOVc(block, uml::COND_E, uml::I6, FAULT_NONE);                        // move     i6, FAULT_NONE
	UML_JMPc(block, uml::COND_E, donefault);                             // jmpe     donefault
	UML_CMP(block, uml::I5, 0);                                          // cmp      i5, 1
	UML_JMPc(block, uml::COND_NE, checkuser = label++);                  // jmpne    checkuser
	UML_ROLAND(block, uml::I6, uml::mem(&COPRO_CTRL),                         // roland   i6, COPRO_CTRL, 32 - COPRO_CTRL_SYSTEM_SHIFT,
				32 - COPRO_CTRL_SYSTEM_SHIFT,                       //          COPRO_CTRL_SYSTEM | COPRO_CTRL_ROM
				COPRO_CTRL_SYSTEM | COPRO_CTRL_ROM);
	// if s == 0 && r == 0, FAULT_PERMISSION
	UML_CMP(block, uml::I6, 0);                                          // cmp      i6, 0
	UML_MOVc(block, uml::COND_E, uml::I6, FAULT_PERMISSION);                  // move     i6, FAULT_PERMISSION
	UML_JMPc(block, uml::COND_E, donefault);                             // jmpe     donefault
	// if s == 1 && r == 1, FAULT_PERMISSION
	UML_CMP(block, uml::I6, 3);                                          // cmp      i6, 3
	UML_MOVc(block, uml::COND_E, uml::I6, FAULT_PERMISSION);                  // move     i6, FAULT_PERMISSION
	UML_JMPc(block, uml::COND_E, donefault);                             // jmpe     donefault
	// if flags & TLB_WRITE, FAULT_PERMISSION
	UML_TEST(block, uml::I2, ARM7_TLB_WRITE);                            // test     i2, ARM7_TLB_WRITE
	UML_MOVc(block, uml::COND_NZ, uml::I6, FAULT_PERMISSION);                 // move     i6, FAULT_PERMISSION
	UML_JMPc(block, uml::COND_NZ, donefault);                            // jmpe     donefault
	// if r == 1 && s == 0, FAULT_NONE
	UML_CMP(block, uml::I6, 2);                                          // cmp      i6, 2
	UML_MOVc(block, uml::COND_E, uml::I6, FAULT_NONE);                        // move     i6, FAULT_NONE
	UML_JMPc(block, uml::COND_E, donefault);                             // jmpe     donefault
	UML_AND(block, uml::I6, uml::mem(&GET_CPSR), MODE_FLAG);                  // and      i6, GET_CPSR, MODE_FLAG
	UML_CMP(block, uml::I6, eARM7_MODE_USER);                            // cmp      i6, eARM7_MODE_USER
	// if r == 0 && s == 1 && usermode, FAULT_PERMISSION
	UML_MOVc(block, uml::COND_E, uml::I6, FAULT_PERMISSION);                  // move     i6, FAULT_PERMISSION
	UML_MOVc(block, uml::COND_NE, uml::I6, FAULT_NONE);                       // movne    i6, FAULT_NONE
	UML_JMP(block, donefault);                                      // jmp      donefault

	UML_LABEL(block, checkuser);                                    // checkuser:
	// if !write, FAULT_NONE
	UML_TEST(block, uml::I2, ARM7_TLB_WRITE);                            // test     i2, ARM7_TLB_WRITE
	UML_MOVc(block, uml::COND_Z, uml::I6, FAULT_NONE);                        // movz     i6, FAULT_NONE
	UML_JMPc(block, uml::COND_Z, donefault);                             // jmp      donefault
	UML_AND(block, uml::I6, uml::mem(&GET_CPSR), MODE_FLAG);                  // and      i6, GET_CPSR, MODE_FLAG
	UML_CMP(block, uml::I6, eARM7_MODE_USER);                            // cmp      i6, eARM7_MODE_USER
	UML_MOVc(block, uml::COND_E, uml::I6, FAULT_PERMISSION);                  // move     i6, FAULT_PERMISSION
	UML_MOVc(block, uml::COND_NE, uml::I6, FAULT_NONE);                       // move     i6, FAULT_NONE

	UML_LABEL(block, donefault);                                    // donefault:
	UML_RET(block);                                                 // ret
}

/*------------------------------------------------------------------
    static_generate_tlb_translate
------------------------------------------------------------------*/

void arm7_cpu_device::static_generate_tlb_translate(uml::code_handle **handleptr)
{
	/* on entry, address is in I0 and flags are in I2 */
	/* on exit, translated address is in I0 and success/failure is in I2 */
	/* routine trashes I4-I7 */
	drcuml_state *drcuml = m_impstate.drcuml;
	drcuml_block *block;
	uml::code_label smallfault;
	uml::code_label smallprefetch;
	int nopid = 0;
	int nounmapped = 0;
	int nounmapped2 = 0;
	int nocoarse = 0;
	int nofine = 0;
	int nosection = 0;
	int nolargepage = 0;
	int nosmallpage = 0;
	int notinypage = 0;
	int handlefault = 0;
	int level2 = 0;
	int prefetch = 0;
	int prefetch2 = 0;
	int label = 1;

	/* begin generating */
	block = drcuml->begin_block(170);

	alloc_handle(drcuml, &m_impstate.tlb_translate, "tlb_translate");
	UML_HANDLE(block, *m_impstate.tlb_translate);               // handle   tlb_translate

	// I3: vaddr
	UML_CMP(block, uml::I0, 32 * 1024 * 1024);                           // cmp      i0, 32*1024*1024
	UML_JMPc(block, uml::COND_GE, nopid = label++);                      // jmpge    nopid
	UML_AND(block, uml::I3, uml::mem(&COPRO_FCSE_PID), 0xfe000000);           // and      i3, COPRO_FCSE_PID, 0xfe000000
	UML_ADD(block, uml::I3, uml::I3, uml::I0);                                     // add      i3, i3, i0

	// I4: desc_lvl1
	UML_AND(block, uml::I4, uml::mem(&COPRO_TLB_BASE), COPRO_TLB_BASE_MASK);  // and      i4, COPRO_TLB_BASE, COPRO_TLB_BASE_MASK
	UML_ROLINS(block, uml::I4, uml::I3, 32 - COPRO_TLB_VADDR_FLTI_MASK_SHIFT, // rolins   i4, i3, 32-COPRO_TLB_VADDR_FLTI_MASK_SHIFT,
				COPRO_TLB_VADDR_FLTI_MASK);                         //          COPRO_TLB_VADDR_FLTI_MASK
	UML_READ(block, uml::I4, uml::I4, uml::SIZE_DWORD, uml::SPACE_PROGRAM);             // read32   i4, i4, PROGRAM

	// I7: desc_lvl1 & 3
	UML_AND(block, uml::I7, uml::I4, 3);                                      // and      i7, i4, 3

	UML_CMP(block, uml::I7, COPRO_TLB_UNMAPPED);                         // cmp      i7, COPRO_TLB_UNMAPPED
	UML_JMPc(block, uml::COND_NE, nounmapped = label++);                 // jmpne    nounmapped

	// TLB Unmapped
	UML_TEST(block, uml::I2, ARM7_TLB_ABORT_D);                          // test     i2, ARM7_TLB_ABORT_D
	UML_MOVc(block, uml::COND_E, uml::mem(&COPRO_FAULT_STATUS_D), (5 << 0));  // move     COPRO_FAULT_STATUS_D, (5 << 0)
	UML_MOVc(block, uml::COND_E, uml::mem(&COPRO_FAULT_ADDRESS), uml::I3);         // move     COPRO_FAULT_ADDRESS, i3
	UML_MOVc(block, uml::COND_E, uml::mem(&m_pendingAbtD), 1);             // move     pendingAbtD, 1
	UML_MOVc(block, uml::COND_E, uml::I2, 0);                                 // move     i2, 0
	UML_RETc(block, uml::COND_E);                                        // rete

	UML_TEST(block, uml::I2, ARM7_TLB_ABORT_P);                          // test     i2, ARM7_TLB_ABORT_P
	UML_MOVc(block, uml::COND_E, uml::mem(&m_pendingAbtP), 1);             // move     pendingAbtP, 1
	UML_MOV(block, uml::I2, 0);                                          // mov      i2, 0
	UML_RET(block);                                                 // ret

	UML_LABEL(block, nounmapped);                                   // nounmapped:
	UML_CMP(block, uml::I7, COPRO_TLB_COARSE_TABLE);                     // cmp      i7, COPRO_TLB_COARSE_TABLE
	UML_JMPc(block, uml::COND_NE, nocoarse = label++);                   // jmpne    nocoarse

	UML_ROLAND(block, uml::I5, uml::I4, 32-4, 0x0f<<1);                       // roland   i5, i4, 32-4, 0xf<<1
	UML_ROLAND(block, uml::I5, uml::mem(&COPRO_DOMAIN_ACCESS_CONTROL), uml::I5, 3);// roland   i5, COPRO_DOMAIN_ACCESS_CONTROL, i5, 3
	UML_CMP(block, uml::I5, 1);                                          // cmp      i5, 1
	UML_JMPc(block, uml::COND_E, level2 = label++);                      // jmpe     level2
	UML_CMP(block, uml::I5, 3);                                          // cmp      i5, 3
	UML_JMPc(block, uml::COND_NE, nofine = label++);                     // jmpne    nofine
	UML_LABEL(block, level2);                                       // level2:

	// I7: desc_level2
	UML_AND(block, uml::I7, uml::I4, COPRO_TLB_CFLD_ADDR_MASK);               // and      i7, i4, COPRO_TLB_CFLD_ADDR_MASK
	UML_ROLINS(block, uml::I7, uml::I3, 32 - COPRO_TLB_VADDR_CSLTI_MASK_SHIFT,// rolins   i7, i3, 32 - COPRO_TLB_VADDR_CSLTI_MASK_SHIFT
				COPRO_TLB_VADDR_CSLTI_MASK);                        //          COPRO_TLB_VADDR_CSLTI_MASK
	UML_READ(block, uml::I7, uml::I7, uml::SIZE_DWORD, uml::SPACE_PROGRAM);             // read32   i7, i7, PROGRAM
	UML_JMP(block, nofine);                                         // jmp      nofine

	UML_LABEL(block, nocoarse);                                     // nocoarse:
	UML_CMP(block, uml::I7, COPRO_TLB_SECTION_TABLE);                    // cmp      i7, COPRO_TLB_SECTION_TABLE
	UML_JMPc(block, uml::COND_NE, nosection = label++);                  // jmpne    nosection

	UML_ROLAND(block, uml::I5, uml::I4, 32-10, 3);                            // roland   i7, i4, 32-10, 3
	// result in I6
	UML_CALLH(block, *m_impstate.detect_fault);                 // callh    detect_fault
	UML_CMP(block, uml::I6, FAULT_NONE);                                 // cmp      i6, FAULT_NONE
	UML_JMPc(block, uml::COND_NE, handlefault = label++);                // jmpne    handlefault

	// no fault, return translated address
	UML_AND(block, uml::I0, uml::I3, ~COPRO_TLB_SECTION_PAGE_MASK);           // and      i0, i3, ~COPRO_TLB_SECTION_PAGE_MASK
	UML_ROLINS(block, uml::I0, uml::I4, 0, COPRO_TLB_SECTION_PAGE_MASK);      // rolins   i0, i4, COPRO_TLB_SECTION_PAGE_MASK
	UML_MOV(block, uml::I2, 1);                                          // mov      i2, 1
	UML_RET(block);                                                 // ret

	UML_LABEL(block, handlefault);                                  // handlefault:
	UML_TEST(block, uml::I2, ARM7_TLB_ABORT_D);                          // test     i2, ARM7_TLB_ABORT_D
	UML_JMPc(block, uml::COND_Z, prefetch = label++);                    // jmpz     prefetch
	UML_MOV(block, uml::mem(&COPRO_FAULT_ADDRESS), uml::I3);                  // mov      COPRO_FAULT_ADDRESS, i3
	UML_MOV(block, uml::mem(&m_pendingAbtD), 1);                      // mov      m_pendingAbtD, 1
	UML_ROLAND(block, uml::I5, uml::I4, 31, 0xf0);                            // roland   i5, i4, 31, 0xf0
	UML_CMP(block, uml::I6, FAULT_DOMAIN);                               // cmp      i6, FAULT_DOMAIN
	UML_MOVc(block, uml::COND_E, uml::I6, 9 << 0);                            // move     i6, 9 << 0
	UML_MOVc(block, uml::COND_NE, uml::I6, 13 << 0);                          // movne    i6, 13 << 0
	UML_OR(block, uml::mem(&COPRO_FAULT_STATUS_D), uml::I5, uml::I6);              // or       COPRO_FAULT_STATUS_D, i5, i6
	UML_MOV(block, uml::I2, 0);                                          // mov      i2, 0
	UML_RET(block);                                                 // ret

	UML_LABEL(block, prefetch);                                     // prefetch:
	UML_MOV(block, uml::mem(&m_pendingAbtP), 1);                      // mov      m_pendingAbtP, 1
	UML_MOV(block, uml::I2, 0);                                          // mov      i2, 0
	UML_RET(block);                                                 // ret

	UML_LABEL(block, nosection);                                    // nosection:
	UML_CMP(block, uml::I7, COPRO_TLB_FINE_TABLE);                       // cmp      i7, COPRO_TLB_FINE_TABLE
	UML_JMPc(block, uml::COND_NE, nofine);                               // jmpne    nofine

	// Not yet implemented
	UML_MOV(block, uml::I2, 1);                                          // mov      i2, 1
	UML_RET(block);                                                 // ret

	UML_LABEL(block, nofine);                                       // nofine:

	// I7: desc_lvl2
	UML_AND(block, uml::I6, uml::I7, 3);                                      // and      i6, i7, 3
	UML_CMP(block, uml::I6, COPRO_TLB_UNMAPPED);                         // cmp      i6, COPRO_TLB_UNMAPPED
	UML_JMPc(block, uml::COND_NE, nounmapped2 = label++);                // jmpne    nounmapped2

	UML_TEST(block, uml::I2, ARM7_TLB_ABORT_D);                          // test     i2, ARM7_TLB_ABORT_D
	UML_JMPc(block, uml::COND_Z, prefetch2 = label++);                   // jmpz     prefetch2
	UML_MOV(block, uml::mem(&COPRO_FAULT_ADDRESS), uml::I3);                  // mov      COPRO_FAULT_ADDRESS, i3
	UML_MOV(block, uml::mem(&m_pendingAbtD), 1);                      // mov      m_pendingAbtD, 1
	UML_ROLAND(block, uml::I5, uml::I4, 31, 0xf0);                            // roland   i5, i4, 31, 0xf0
	UML_OR(block, uml::I5, uml::I5, 7 << 0);                                  // or       i5, i5, 7 << 0
	UML_OR(block, uml::mem(&COPRO_FAULT_STATUS_D), uml::I5, uml::I6);              // or       COPRO_FAULT_STATUS_D, i5, i6
	UML_MOV(block, uml::I2, 0);                                          // mov      i2, 0
	UML_RET(block);                                                 // ret

	UML_LABEL(block, prefetch2);                                    // prefetch2:
	UML_MOV(block, uml::mem(&m_pendingAbtP), 1);                      // mov      m_pendingAbtP, 1
	UML_MOV(block, uml::I2, 0);                                          // mov      i2, 0
	UML_RET(block);                                                 // ret

	UML_LABEL(block, nounmapped2);                                  // nounmapped2:
	UML_CMP(block, uml::I6, COPRO_TLB_LARGE_PAGE);                       // cmp      i6, COPRO_TLB_LARGE_PAGE
	UML_JMPc(block, uml::COND_NE, nolargepage = label++);                // jmpne    nolargepage

	UML_AND(block, uml::I0, uml::I3, ~COPRO_TLB_LARGE_PAGE_MASK);             // and      i0, i3, ~COPRO_TLB_LARGE_PAGE_MASK
	UML_ROLINS(block, uml::I0, uml::I7, 0, COPRO_TLB_LARGE_PAGE_MASK);        // rolins   i0, i7, 0, COPRO_TLB_LARGE_PAGE_MASK
	UML_MOV(block, uml::I2, 1);                                          // mov      i2, 1
	UML_RET(block);                                                 // ret

	UML_LABEL(block, nolargepage);                                  // nolargepage:
	UML_CMP(block, uml::I6, COPRO_TLB_SMALL_PAGE);                       // cmp      i6, COPRO_TLB_SMALL_PAGE
	UML_JMPc(block, uml::COND_NE, nosmallpage = label++);                // jmpne    nosmallpage

	UML_ROLAND(block, uml::I5, uml::I3, 32-9, 3<<1);                          // roland   i5, i3, 32-9, 3<<1
	UML_ROLAND(block, uml::I6, uml::I7, 32-4, 0xff);                          // roland   i6, i7, 32-4, 0xff
	UML_SHR(block, uml::I5, uml::I7, uml::I5);                                     // shr      i5, i7, i5
	UML_AND(block, uml::I5, uml::I5, 3);                                      // and      i5, i5, 3
	// result in I6
	UML_CALLH(block, *m_impstate.detect_fault);                 // callh    detect_fault

	UML_CMP(block, uml::I6, FAULT_NONE);                                 // cmp      i6, FAULT_NONE
	UML_JMPc(block, uml::COND_NE, smallfault = label++);                 // jmpne    smallfault
	UML_AND(block, uml::I0, uml::I7, COPRO_TLB_SMALL_PAGE_MASK);              // and      i0, i7, COPRO_TLB_SMALL_PAGE_MASK
	UML_ROLINS(block, uml::I0, uml::I3, 0, ~COPRO_TLB_SMALL_PAGE_MASK);       // rolins   i0, i3, 0, ~COPRO_TLB_SMALL_PAGE_MASK
	UML_MOV(block, uml::I2, 1);                                          // mov      i2, 1
	UML_RET(block);                                                 // ret

	UML_LABEL(block, smallfault);                                   // smallfault:
	UML_TEST(block, uml::I2, ARM7_TLB_ABORT_D);                          // test     i2, ARM7_TLB_ABORT_D
	UML_JMPc(block, uml::COND_NZ, smallprefetch = label++);              // jmpnz    smallprefetch
	UML_MOV(block, uml::mem(&COPRO_FAULT_ADDRESS), uml::I3);                  // mov      COPRO_FAULT_ADDRESS, i3
	UML_MOV(block, uml::mem(&m_pendingAbtD), 1);                      // mov      pendingAbtD, 1
	UML_CMP(block, uml::I6, FAULT_DOMAIN);                               // cmp      i6, FAULT_DOMAIN
	UML_MOVc(block, uml::COND_E, uml::I5, 11 << 0);                           // move     i5, 11 << 0
	UML_MOVc(block, uml::COND_NE, uml::I5, 15 << 0);                          // movne    i5, 15 << 0
	UML_ROLINS(block, uml::I5, uml::I4, 31, 0xf0);                            // rolins   i5, i4, 31, 0xf0
	UML_MOV(block, uml::mem(&COPRO_FAULT_STATUS_D), uml::I5);                 // mov      COPRO_FAULT_STATUS_D, i5
	UML_MOV(block, uml::I2, 0);                                          // mov      i2, 0
	UML_RET(block);                                                 // ret

	UML_LABEL(block, smallprefetch);                                // smallprefetch:
	UML_MOV(block, uml::mem(&m_pendingAbtP), 1);                      // mov      pendingAbtP, 1
	UML_MOV(block, uml::I2, 0);                                          // mov      i2, 0
	UML_RET(block);                                                 // ret

	UML_LABEL(block, nosmallpage);                                  // nosmallpage:
	UML_CMP(block, uml::I6, COPRO_TLB_TINY_PAGE);                        // cmp      i6, COPRO_TLB_TINY_PAGE
	UML_JMPc(block, uml::COND_NE, notinypage = label++);                 // jmpne    notinypage

	UML_AND(block, uml::I0, uml::I3, ~COPRO_TLB_TINY_PAGE_MASK);              // and      i0, i3, ~COPRO_TLB_TINY_PAGE_MASK
	UML_ROLINS(block, uml::I0, uml::I7, 0, COPRO_TLB_TINY_PAGE_MASK);         // rolins   i0, i7, 0, COPRO_TLB_TINY_PAGE_MASK
	UML_MOV(block, uml::I2, 1);                                          // mov      i2, 1
	UML_RET(block);                                                 // ret

	UML_LABEL(block, notinypage);                                   // notinypage:
	UML_MOV(block, uml::I0, uml::I3);                                         // mov      i0, i3
	UML_RET(block);                                                 // ret

	block->end();
}

/*------------------------------------------------------------------
    static_generate_memory_accessor
------------------------------------------------------------------*/

void arm7_cpu_device::static_generate_memory_accessor(int size, bool istlb, bool iswrite, const char *name, uml::code_handle **handleptr)
{
	/* on entry, address is in I0; data for writes is in I1, fetch type in I2 */
	/* on exit, read result is in I0 */
	/* routine trashes I0-I3 */
	drcuml_state *drcuml = m_impstate.drcuml;
	drcuml_block *block;
	//int tlbmiss = 0;
	int label = 1;

	/* begin generating */
	block = drcuml->begin_block(1024);

	/* add a global entry for this */
	alloc_handle(drcuml, handleptr, name);
	UML_HANDLE(block, **handleptr);                                         // handle  *handleptr

	if (istlb)
	{
		UML_TEST(block, uml::mem(&COPRO_CTRL), COPRO_CTRL_MMU_EN);               // test     COPRO_CTRL, COPRO_CTRL_MMU_EN
		if (iswrite)
		{
			UML_MOVc(block, uml::COND_NZ, uml::I3, ARM7_TLB_WRITE);                   // movnz    i3, ARM7_TLB_WRITE
		}
		else
		{
			UML_MOVc(block, uml::COND_NZ, uml::I3, ARM7_TLB_READ);                    // movnz    i3, ARM7_TLB_READ
		}
		UML_OR(block, uml::I2, uml::I2, uml::I3);                                          // or       i2, i2, i3
		UML_CALLHc(block, uml::COND_NZ, *m_impstate.tlb_translate);          // callhnz  tlb_translate
	}

	/* general case: assume paging and perform a translation */
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) == 0)
	{
		for (int ramnum = 0; ramnum < ARM7_MAX_FASTRAM; ramnum++)
		{
			if (m_impstate.fastram[ramnum].base != nullptr && (!iswrite || !m_impstate.fastram[ramnum].readonly))
			{
				void *fastbase = (UINT8 *)m_impstate.fastram[ramnum].base - m_impstate.fastram[ramnum].start;
				UINT32 skip = label++;
				if (m_impstate.fastram[ramnum].end != 0xffffffff)
				{
					UML_CMP(block, uml::I0, m_impstate.fastram[ramnum].end);     // cmp     i0, end
					UML_JMPc(block, uml::COND_A, skip);                              // ja      skip
				}
				if (m_impstate.fastram[ramnum].start != 0x00000000)
				{
					UML_CMP(block, uml::I0, m_impstate.fastram[ramnum].start);   // cmp     i0, fastram_start
					UML_JMPc(block, uml::COND_B, skip);                              // jb      skip
				}

				if (!iswrite)
				{
					if (size == 1)
					{
						UML_XOR(block, uml::I0, uml::I0, (m_endian == ENDIANNESS_BIG) ? BYTE4_XOR_BE(0) : BYTE4_XOR_LE(0));
																						// xor     i0, i0, bytexor
						UML_LOAD(block, uml::I0, fastbase, uml::I0, uml::SIZE_BYTE, uml::SCALE_x1);         // load    i0, fastbase, i0, byte
					}
					else if (size == 2)
					{
						UML_XOR(block, uml::I0, uml::I0, (m_endian == ENDIANNESS_BIG) ? WORD_XOR_BE(0) : WORD_XOR_LE(0));
																						// xor     i0, i0, wordxor
						UML_LOAD(block, uml::I0, fastbase, uml::I0, uml::SIZE_WORD, uml::SCALE_x1);         // load    i0, fastbase, i0, word_x1
					}
					else if (size == 4)
					{
						UML_LOAD(block, uml::I0, fastbase, uml::I0, uml::SIZE_DWORD, uml::SCALE_x1);        // load    i0, fastbase, i0, dword_x1
					}
					UML_RET(block);                                                     // ret
				}
				else
				{
					if (size == 1)
					{
						UML_XOR(block, uml::I0, uml::I0, (m_endian == ENDIANNESS_BIG) ? BYTE4_XOR_BE(0) : BYTE4_XOR_LE(0));
																						// xor     i0, i0, bytexor
						UML_STORE(block, fastbase, uml::I0, uml::I1, uml::SIZE_BYTE, uml::SCALE_x1);        // store   fastbase, i0, i1, byte
					}
					else if (size == 2)
					{
						UML_XOR(block, uml::I0, uml::I0, (m_endian == ENDIANNESS_BIG) ? WORD_XOR_BE(0) : WORD_XOR_LE(0));
																						// xor     i0, i0, wordxor
						UML_STORE(block, fastbase, uml::I0, uml::I1, uml::SIZE_WORD, uml::SCALE_x1);        // store   fastbase, i0, i1, word_x1
					}
					else if (size == 4)
					{
						UML_STORE(block, fastbase, uml::I0, uml::I1, uml::SIZE_DWORD, uml::SCALE_x1);       // store   fastbase,i0,i1,dword_x1
					}
					UML_RET(block);                                                     // ret
				}

				UML_LABEL(block, skip);                                                 // skip:
			}
		}
	}

	switch (size)
	{
		case 1:
			if (iswrite)
			{
				UML_WRITE(block, uml::I0, uml::I1, uml::SIZE_BYTE, uml::SPACE_PROGRAM);                 // write   i0, i1, program_byte
			}
			else
			{
				UML_READ(block, uml::I0, uml::I0, uml::SIZE_BYTE, uml::SPACE_PROGRAM);                  // read    i0, i0, program_byte
			}
			break;

		case 2:
			if (iswrite)
			{
				UML_WRITE(block, uml::I0, uml::I1, uml::SIZE_WORD, uml::SPACE_PROGRAM);                 // write   i0,i1,program_word
			}
			else
			{
				UML_READ(block, uml::I0, uml::I0, uml::SIZE_WORD, uml::SPACE_PROGRAM);                  // read    i0,i0,program_word
			}
			break;

		case 4:
			if (iswrite)
			{
				UML_WRITE(block, uml::I0, uml::I1, uml::SIZE_DWORD, uml::SPACE_PROGRAM);                // write   i0,i1,program_dword
			}
			else
			{
				UML_READ(block, uml::I0, uml::I0, uml::SIZE_DWORD, uml::SPACE_PROGRAM);                 // read    i0,i0,program_dword
			}
			break;
	}
	UML_RET(block);                                                                 // ret

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

void arm7_cpu_device::generate_update_cycles(drcuml_block *block, compiler_state *compiler, uml::parameter param)
{
	/* check full interrupts if pending */
	if (compiler->checkints)
	{
		uml::code_label skip;

		compiler->checkints = FALSE;
		UML_CALLH(block, *m_impstate.check_irq);
	}

	/* account for cycles */
	if (compiler->cycles > 0)
	{
		UML_SUB(block, uml::mem(&m_icount), uml::mem(&m_icount), MAPVAR_CYCLES);    // sub     icount,icount,cycles
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);                                    // mapvar  cycles,0
		UML_EXHc(block, uml::COND_S, *m_impstate.out_of_cycles, param);          // exh     out_of_cycles,nextpc
	}
	compiler->cycles = 0;
}


/*-------------------------------------------------
    generate_checksum_block - generate code to
    validate a sequence of opcodes
-------------------------------------------------*/

void arm7_cpu_device::generate_checksum_block(drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast)
{
	const opcode_desc *curdesc;
	if (m_impstate.drcuml->logging())
	{
		block->append_comment("[Validation for %08X]", seqhead->pc);                // comment
	}

	/* loose verify or single instruction: just compare and fail */
	if (!(m_impstate.drcoptions & ARM7DRC_STRICT_VERIFY) || seqhead->next() == nullptr)
	{
		if (!(seqhead->flags & OPFLAG_VIRTUAL_NOOP))
		{
			UINT32 sum = seqhead->opptr.l[0];
			void *base = m_direct->read_ptr(seqhead->physpc);
			UML_LOAD(block, uml::I0, base, 0, uml::SIZE_DWORD, uml::SCALE_x4);             // load    i0,base,0,dword

			if (seqhead->delay.first() != nullptr && seqhead->physpc != seqhead->delay.first()->physpc)
			{
				base = m_direct->read_ptr(seqhead->delay.first()->physpc);
				UML_LOAD(block, uml::I1, base, 0, uml::SIZE_DWORD, uml::SCALE_x4);         // load    i1,base,dword
				UML_ADD(block, uml::I0, uml::I0, uml::I1);                                 // add     i0,i0,i1

				sum += seqhead->delay.first()->opptr.l[0];
			}

			UML_CMP(block, uml::I0, sum);                                        // cmp     i0,opptr[0]
			UML_EXHc(block, uml::COND_NE, *m_impstate.nocode, epc(seqhead)); // exne    nocode,seqhead->pc
		}
	}

	/* full verification; sum up everything */
	else
	{
		UINT32 sum = 0;
		void *base = m_direct->read_ptr(seqhead->physpc);
		UML_LOAD(block, uml::I0, base, 0, uml::SIZE_DWORD, uml::SCALE_x4);                 // load    i0,base,0,dword
		sum += seqhead->opptr.l[0];
		for (curdesc = seqhead->next(); curdesc != seqlast->next(); curdesc = curdesc->next())
			if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
			{
				base = m_direct->read_ptr(curdesc->physpc);
				UML_LOAD(block, uml::I1, base, 0, uml::SIZE_DWORD, uml::SCALE_x4);         // load    i1,base,dword
				UML_ADD(block, uml::I0, uml::I0, uml::I1);                                 // add     i0,i0,i1
				sum += curdesc->opptr.l[0];

				if (curdesc->delay.first() != nullptr && (curdesc == seqlast || (curdesc->next() != nullptr && curdesc->next()->physpc != curdesc->delay.first()->physpc)))
				{
					base = m_direct->read_ptr(curdesc->delay.first()->physpc);
					UML_LOAD(block, uml::I1, base, 0, uml::SIZE_DWORD, uml::SCALE_x4);     // load    i1,base,dword
					UML_ADD(block, uml::I0, uml::I0, uml::I1);                             // add     i0,i0,i1
					sum += curdesc->delay.first()->opptr.l[0];
				}
			}
		UML_CMP(block, uml::I0, sum);                                            // cmp     i0,sum
		UML_EXHc(block, uml::COND_NE, *m_impstate.nocode, epc(seqhead));     // exne    nocode,seqhead->pc
	}
}


/*-------------------------------------------------
    generate_sequence_instruction - generate code
    for a single instruction in a sequence
-------------------------------------------------*/

void arm7_cpu_device::generate_sequence_instruction(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	//offs_t expc;
	int hotnum;

	/* add an entry for the log */
	// TODO FIXME
//  if (m_impstate.drcuml->logging() && !(desc->flags & OPFLAG_VIRTUAL_NOOP))
//      log_add_disasm_comment(block, desc->pc, desc->opptr.l[0]);

	/* set the PC map variable */
	//expc = (desc->flags & OPFLAG_IN_DELAY_SLOT) ? desc->pc - 3 : desc->pc;
	UML_MAPVAR(block, MAPVAR_PC, desc->pc);                                 // mapvar  PC,pc

	/* accumulate total cycles */
	compiler->cycles += desc->cycles;

	/* update the icount map variable */
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);                     // mapvar  CYCLES,compiler->cycles

	/* is this a hotspot? */
	for (hotnum = 0; hotnum < ARM7_MAX_HOTSPOTS; hotnum++)
	{
		if (m_impstate.hotspot[hotnum].pc != 0 && desc->pc == m_impstate.hotspot[hotnum].pc && desc->opptr.l[0] == m_impstate.hotspot[hotnum].opcode)
		{
			compiler->cycles += m_impstate.hotspot[hotnum].cycles;
			break;
		}
	}

	/* update the icount map variable */
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);                     // mapvar  CYCLES,compiler->cycles

	/* if we are debugging, call the debugger */
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		UML_MOV(block, uml::mem(&R15), desc->pc);                                // mov     [pc],desc->pc
		save_fast_iregs(block);
		UML_DEBUG(block, desc->pc);                                         // debug   desc->pc
	}

	/* if we hit an unmapped address, fatal error */
	if (desc->flags & OPFLAG_COMPILER_UNMAPPED)
	{
		UML_MOV(block, uml::mem(&R15), desc->pc);                                // mov     R15,desc->pc
		save_fast_iregs(block);
		UML_EXIT(block, EXECUTE_UNMAPPED_CODE);                             // exit    EXECUTE_UNMAPPED_CODE
	}

	/* otherwise, unless this is a virtual no-op, it's a regular instruction */
	else if (!(desc->flags & OPFLAG_VIRTUAL_NOOP))
	{
		/* compile the instruction */
		if (!generate_opcode(block, compiler, desc))
		{
			UML_MOV(block, uml::mem(&R15), desc->pc);                            // mov     R15,desc->pc
			UML_MOV(block, uml::mem(&m_impstate.arg0), desc->opptr.l[0]);    // mov     [arg0],desc->opptr.l
			//UML_CALLC(block, cfunc_unimplemented, arm);                     // callc   cfunc_unimplemented // TODO FIXME
		}
	}
}


/*------------------------------------------------------------------
    generate_delay_slot_and_branch
------------------------------------------------------------------*/

void arm7_cpu_device::generate_delay_slot_and_branch(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT8 linkreg)
{
	compiler_state compiler_temp = *compiler;

	/* update the cycles and jump through the hash table to the target */
	if (desc->targetpc != BRANCH_TARGET_DYNAMIC)
	{
		generate_update_cycles(block, &compiler_temp, desc->targetpc);   // <subtract cycles>
		UML_HASHJMP(block, 0, desc->targetpc, *m_impstate.nocode);
																					// hashjmp 0,desc->targetpc,nocode
	}
	else
	{
		generate_update_cycles(block, &compiler_temp, uml::mem(&m_impstate.jmpdest));
																					// <subtract cycles>
		UML_HASHJMP(block, 0, uml::mem(&m_impstate.jmpdest), *m_impstate.nocode);// hashjmp 0,<rsreg>,nocode
	}

	/* update the label */
	compiler->labelnum = compiler_temp.labelnum;

	/* reset the mapvar to the current cycles and account for skipped slots */
	compiler->cycles += desc->skipslots;
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);                             // mapvar  CYCLES,compiler->cycles
}


const arm7_cpu_device::drcarm7ops_ophandler arm7_cpu_device::drcops_handler[0x10] =
{
	&arm7_cpu_device::drcarm7ops_0123, &arm7_cpu_device::drcarm7ops_0123, &arm7_cpu_device::drcarm7ops_0123, &arm7_cpu_device::drcarm7ops_0123,
	&arm7_cpu_device::drcarm7ops_4567, &arm7_cpu_device::drcarm7ops_4567, &arm7_cpu_device::drcarm7ops_4567, &arm7_cpu_device::drcarm7ops_4567,
	&arm7_cpu_device::drcarm7ops_89,   &arm7_cpu_device::drcarm7ops_89,   &arm7_cpu_device::drcarm7ops_ab,   &arm7_cpu_device::drcarm7ops_ab,
	&arm7_cpu_device::drcarm7ops_cd,   &arm7_cpu_device::drcarm7ops_cd,   &arm7_cpu_device::drcarm7ops_e,    &arm7_cpu_device::drcarm7ops_f,
};

void arm7_cpu_device::saturate_qbit_overflow(drcuml_block *block)
{
	UML_MOV(block, uml::I1, 0);
	UML_DCMP(block, uml::I0, 0x000000007fffffffL);
	UML_MOVc(block, uml::COND_G, uml::I1, Q_MASK);
	UML_MOVc(block, uml::COND_G, uml::I0, 0x7fffffff);
	UML_DCMP(block, uml::I0, U64(0xffffffff80000000));
	UML_MOVc(block, uml::COND_L, uml::I1, Q_MASK);
	UML_MOVc(block, uml::COND_L, uml::I0, 0x80000000);
	UML_OR(block, DRC_CPSR, DRC_CPSR, uml::I1);
}

bool arm7_cpu_device::drcarm7ops_0123(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 insn)
{
	uml::code_label done;
	/* Branch and Exchange (BX) */
	if ((insn & 0x0ffffff0) == 0x012fff10)     // bits 27-4 == 000100101111111111110001
	{
		UML_MOV(block, DRC_PC, DRC_REG(insn & 0x0f));
		UML_TEST(block, DRC_PC, 1);
		UML_JMPc(block, uml::COND_Z, done = compiler->labelnum++);
		UML_OR(block, DRC_CPSR, DRC_CPSR, T_MASK);
		UML_AND(block, DRC_PC, DRC_PC, ~1);
	}
	else if ((insn & 0x0ff000f0) == 0x01600010) // CLZ - v5
	{
		UINT32 rm = insn&0xf;
		UINT32 rd = (insn>>12)&0xf;

		UML_LZCNT(block, DRC_REG(rd), DRC_REG(rm));
		UML_ADD(block, DRC_PC, DRC_PC, 4);
	}
	else if ((insn & 0x0ff000f0) == 0x01000050) // QADD - v5
	{
		UINT32 rm = insn&0xf;
		UINT32 rn = (insn>>16)&0xf;
		UINT32 rd = (insn>>12)&0xf;
		UML_DSEXT(block, uml::I0, DRC_REG(rm), uml::SIZE_DWORD);
		UML_DSEXT(block, uml::I1, DRC_REG(rn), uml::SIZE_DWORD);
		UML_DADD(block, uml::I0, uml::I0, uml::I1);
		saturate_qbit_overflow(block);
		UML_MOV(block, DRC_REG(rd), uml::I0);
		UML_ADD(block, DRC_PC, DRC_PC, 4);
	}
	else if ((insn & 0x0ff000f0) == 0x01400050) // QDADD - v5
	{
		UINT32 rm = insn&0xf;
		UINT32 rn = (insn>>16)&0xf;
		UINT32 rd = (insn>>12)&0xf;

		UML_DSEXT(block, uml::I1, DRC_REG(rn), uml::SIZE_DWORD);
		UML_DADD(block, uml::I0, uml::I1, uml::I1);
		saturate_qbit_overflow(block);

		UML_DSEXT(block, uml::I0, DRC_REG(rm), uml::SIZE_DWORD);
		UML_DSEXT(block, uml::I1, DRC_REG(rn), uml::SIZE_DWORD);
		UML_DADD(block, uml::I1, uml::I1, uml::I1);
		UML_DADD(block, uml::I0, uml::I0, uml::I1);
		saturate_qbit_overflow(block);
		UML_MOV(block, DRC_REG(rd), uml::I0);

		UML_ADD(block, DRC_PC, DRC_PC, 4);
	}
	else if ((insn & 0x0ff000f0) == 0x01200050) // QSUB - v5
	{
		UINT32 rm = insn&0xf;
		UINT32 rn = (insn>>16)&0xf;
		UINT32 rd = (insn>>12)&0xf;

		UML_DSEXT(block, uml::I0, DRC_REG(rm), uml::SIZE_DWORD);
		UML_DSEXT(block, uml::I1, DRC_REG(rn), uml::SIZE_DWORD);
		UML_DSUB(block, uml::I0, uml::I0, uml::I1);
		saturate_qbit_overflow(block);
		UML_MOV(block, DRC_REG(rd), uml::I0);
		UML_ADD(block, DRC_PC, DRC_PC, 4);
	}
	else if ((insn & 0x0ff000f0) == 0x01600050) // QDSUB - v5
	{
		UINT32 rm = insn&0xf;
		UINT32 rn = (insn>>16)&0xf;
		UINT32 rd = (insn>>12)&0xf;

		UML_DSEXT(block, uml::I1, DRC_REG(rn), uml::SIZE_DWORD);
		UML_DADD(block, uml::I0, uml::I1, uml::I1);
		saturate_qbit_overflow(block);

		UML_DSEXT(block, uml::I0, DRC_REG(rm), uml::SIZE_DWORD);
		UML_DSEXT(block, uml::I1, DRC_REG(rn), uml::SIZE_DWORD);
		UML_DADD(block, uml::I1, uml::I1, uml::I1);
		UML_DSUB(block, uml::I0, uml::I0, uml::I1);
		saturate_qbit_overflow(block);
		UML_MOV(block, DRC_REG(rd), uml::I0);

		UML_ADD(block, DRC_PC, DRC_PC, 4);
	}
	else if ((insn & 0x0ff00090) == 0x01000080) // SMLAxy - v5
	{
		UINT32 rm = insn&0xf;
		UINT32 rn = (insn>>8)&0xf;
		UINT32 rd = (insn>>16)&0xf;
		UINT32 ra = (insn>>12)&0xf;

		UML_MOV(block, uml::I0, DRC_REG(rm));
		UML_MOV(block, uml::I1, DRC_REG(rn));

		// select top and bottom halves of src1/src2 and sign extend if necessary
		if (insn & 0x20)
		{
			UML_SHR(block, uml::I0, uml::I0, 16);
		}
		UML_SEXT(block, uml::I0, uml::I0, uml::SIZE_WORD);

		if (insn & 0x40)
		{
			UML_SHR(block, uml::I1, uml::I1, 16);
		}
		UML_SEXT(block, uml::I0, uml::I0, uml::SIZE_WORD);

		// do the signed multiply
		UML_MULS(block, uml::I0, uml::I1, uml::I0, uml::I1);
		UML_DSHL(block, uml::I0, uml::I0, 32);
		UML_DOR(block, uml::I0, uml::I0, uml::I1);
		UML_MOV(block, uml::I1, DRC_REG(ra));
		UML_DADD(block, uml::I0, uml::I0, uml::I1);
		// and the accumulate.  NOTE: only the accumulate can cause an overflow, which is why we do it this way.
		saturate_qbit_overflow(block);
		UML_MOV(block, DRC_REG(rd), uml::I0);
		UML_ADD(block, DRC_PC, DRC_PC, 4);
	}
	else if ((insn & 0x0ff00090) == 0x01400080) // SMLALxy - v5
	{
		UINT32 rm = insn&0xf;
		UINT32 rn = (insn>>8)&0xf;
		UINT32 rdh = (insn>>16)&0xf;
		UINT32 rdl = (insn>>12)&0xf;

		UML_DSEXT(block, uml::I0, DRC_REG(rm), uml::SIZE_DWORD);
		UML_DSEXT(block, uml::I1, DRC_REG(rn), uml::SIZE_DWORD);
		// do the signed multiply
		UML_DMULS(block, uml::I2, uml::I3, uml::I0, uml::I1);

		UML_MOV(block, uml::I0, DRC_REG(rdh));
		UML_MOV(block, uml::I1, DRC_REG(rdl));
		UML_DSHL(block, uml::I0, uml::I0, 32);
		UML_DOR(block, uml::I0, uml::I0, uml::I1);
		UML_DADD(block, uml::I0, uml::I0, uml::I2);
		UML_MOV(block, DRC_REG(rdl), uml::I0);
		UML_DSHR(block, uml::I0, uml::I0, 32);
		UML_MOV(block, DRC_REG(rdh), uml::I0);
	}
	else if ((insn & 0x0ff00090) == 0x01600080) // SMULxy - v5
	{
		INT32 src1 = GET_REGISTER(insn&0xf);
		INT32 src2 = GET_REGISTER((insn>>8)&0xf);
		INT32 res;

		// select top and bottom halves of src1/src2 and sign extend if necessary
		if (insn & 0x20)
		{
			src1 >>= 16;
		}

		src1 &= 0xffff;
		if (src1 & 0x8000)
		{
			src1 |= 0xffff0000;
		}

		if (insn & 0x40)
		{
			src2 >>= 16;
		}

		src2 &= 0xffff;
		if (src2 & 0x8000)
		{
			src2 |= 0xffff0000;
		}

		res = src1 * src2;
		SET_REGISTER((insn>>16)&0xf, res);
		R15 += 4;
	}
	else if ((insn & 0x0ff000b0) == 0x012000a0) // SMULWy - v5
	{
		INT32 src1 = GET_REGISTER(insn&0xf);
		INT32 src2 = GET_REGISTER((insn>>8)&0xf);
		INT64 res;

		if (insn & 0x40)
		{
			src2 >>= 16;
		}
		else
		{
			src2 &= 0xffff;
			if (src2 & 0x8000)
			{
				src2 |= 0xffff;
			}
		}

		res = (INT64)src1 * (INT64)src2;
		res >>= 16;
		SET_REGISTER((insn>>16)&0xf, (UINT32)res);
	}
	else if ((insn & 0x0ff000b0) == 0x01200080) // SMLAWy - v5
	{
		INT32 src1 = GET_REGISTER(insn&0xf);
		INT32 src2 = GET_REGISTER((insn>>8)&0xf);
		INT32 src3 = GET_REGISTER((insn>>12)&0xf);
		INT64 res;

		if (insn & 0x40)
		{
			src2 >>= 16;
		}
		else
		{
			src2 &= 0xffff;
			if (src2 & 0x8000)
			{
				src2 |= 0xffff;
			}
		}

		res = (INT64)src1 * (INT64)src2;
		res >>= 16;

		// check for overflow and set the Q bit
		saturate_qbit_overflow((INT64)src3 + res);

		// do the real accumulate
		src3 += (INT32)res;

		// write the result back
		SET_REGISTER((insn>>16)&0xf, (UINT32)res);
	}
	else
	/* Multiply OR Swap OR Half Word Data Transfer */
	if ((insn & 0x0e000000) == 0 && (insn & 0x80) && (insn & 0x10))  // bits 27-25=000 bit 7=1 bit 4=1
	{
		/* Half Word Data Transfer */
		if (insn & 0x60)         // bits = 6-5 != 00
		{
			HandleHalfWordDT(insn);
		}
		else
		/* Swap */
		if (insn & 0x01000000)   // bit 24 = 1
		{
			HandleSwap(insn);
		}
		/* Multiply Or Multiply Long */
		else
		{
			/* multiply long */
			if (insn & 0x800000) // Bit 23 = 1 for Multiply Long
			{
				/* Signed? */
				if (insn & 0x00400000)
					HandleSMulLong(insn);
				else
					HandleUMulLong(insn);
			}
			/* multiply */
			else
			{
				HandleMul(insn);
			}
			R15 += 4;
		}
	}
	/* Data Processing OR PSR Transfer */
	else if ((insn & 0x0c000000) == 0)   // bits 27-26 == 00 - This check can only exist properly after Multiplication check above
	{
		/* PSR Transfer (MRS & MSR) */
		if (((insn & 0x00100000) == 0) && ((insn & 0x01800000) == 0x01000000)) // S bit must be clear, and bit 24,23 = 10
		{
			HandlePSRTransfer(insn);
			ARM7_ICOUNT += 2;       // PSR only takes 1 - S Cycle, so we add + 2, since at end, we -3..
			R15 += 4;
		}
		/* Data Processing */
		else
		{
			HandleALU(insn);
		}
	}

	UML_LABEL(block, done);
	return true;
}

bool arm7_cpu_device::drcarm7ops_4567(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op)
{
	return false;
}

bool arm7_cpu_device::drcarm7ops_89(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op)
{
	return false;
}

bool arm7_cpu_device::drcarm7ops_ab(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op)
{
	return false;
}

bool arm7_cpu_device::drcarm7ops_cd(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op)
{
	return false;
}

bool arm7_cpu_device::drcarm7ops_e(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op)
{
	return false;
}

bool arm7_cpu_device::drcarm7ops_f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op)
{
	return false;
}

/*-------------------------------------------------
    generate_opcode - generate code for a specific
    opcode
-------------------------------------------------*/

int arm7_cpu_device::generate_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	//int in_delay_slot = ((desc->flags & OPFLAG_IN_DELAY_SLOT) != 0);
	UINT32 op = desc->opptr.l[0];
	UINT8 opswitch = op >> 26;
	uml::code_label skip;
	uml::code_label contdecode;
	uml::code_label unexecuted;

	if (T_IS_SET(GET_CPSR))
	{
		// "In Thumb state, bit [0] is undefined and must be ignored. Bits [31:1] contain the PC."
		UML_AND(block, uml::I0, DRC_PC, ~1);
	}
	else
	{
		UML_AND(block, uml::I0, DRC_PC, ~3);
	}

	UML_TEST(block, uml::mem(&COPRO_CTRL), COPRO_CTRL_MMU_EN);                       // test     COPRO_CTRL, COPRO_CTRL_MMU_EN
	UML_MOVc(block, uml::COND_NZ, uml::I2, ARM7_TLB_ABORT_P | ARM7_TLB_READ);             // movnz    i0, ARM7_TLB_ABORT_P | ARM7_TLB_READ
	UML_CALLHc(block, uml::COND_NZ, *m_impstate.tlb_translate);                  // callhnz  tlb_translate);

	if (T_IS_SET(GET_CPSR))
	{
		//UML_CALLH(block, *m_impstate.drcthumb[(op & 0xffc0) >> 6]);          // callh    drcthumb[op] // TODO FIXME
		return TRUE;
	}

	switch (op >> INSN_COND_SHIFT)
	{
		case COND_EQ:
			UML_TEST(block, DRC_CPSR, Z_MASK);
			UML_JMPc(block, uml::COND_Z, unexecuted = compiler->labelnum++);
			break;
		case COND_NE:
			UML_TEST(block, DRC_CPSR, Z_MASK);
			UML_JMPc(block, uml::COND_NZ, unexecuted = compiler->labelnum++);
			break;
		case COND_CS:
			UML_TEST(block, DRC_CPSR, C_MASK);
			UML_JMPc(block, uml::COND_Z, unexecuted = compiler->labelnum++);
			break;
		case COND_CC:
			UML_TEST(block, DRC_CPSR, C_MASK);
			UML_JMPc(block, uml::COND_NZ, unexecuted = compiler->labelnum++);
			break;
		case COND_MI:
			UML_TEST(block, DRC_CPSR, N_MASK);
			UML_JMPc(block, uml::COND_Z, unexecuted = compiler->labelnum++);
			break;
		case COND_PL:
			UML_TEST(block, DRC_CPSR, N_MASK);
			UML_JMPc(block, uml::COND_NZ, unexecuted = compiler->labelnum++);
			break;
		case COND_VS:
			UML_TEST(block, DRC_CPSR, V_MASK);
			UML_JMPc(block, uml::COND_Z, unexecuted = compiler->labelnum++);
			break;
		case COND_VC:
			UML_TEST(block, DRC_CPSR, V_MASK);
			UML_JMPc(block, uml::COND_NZ, unexecuted = compiler->labelnum++);
			break;
		case COND_HI:
			UML_TEST(block, DRC_CPSR, Z_MASK);
			UML_JMPc(block, uml::COND_NZ, unexecuted = compiler->labelnum++);
			UML_TEST(block, DRC_CPSR, C_MASK);
			UML_JMPc(block, uml::COND_Z, unexecuted = compiler->labelnum++);
			break;
		case COND_LS:
			UML_TEST(block, DRC_CPSR, Z_MASK);
			UML_JMPc(block, uml::COND_NZ, contdecode = compiler->labelnum++);
			UML_TEST(block, DRC_CPSR, C_MASK);
			UML_JMPc(block, uml::COND_Z, contdecode);
			UML_JMP(block, unexecuted);
			break;
		case COND_GE:
			UML_TEST(block, DRC_CPSR, N_MASK);
			UML_MOVc(block, uml::COND_Z, uml::I0, 0);
			UML_MOVc(block, uml::COND_NZ, uml::I0, 1);
			UML_TEST(block, DRC_CPSR, V_MASK);
			UML_MOVc(block, uml::COND_Z, uml::I1, 0);
			UML_MOVc(block, uml::COND_NZ, uml::I1, 1);
			UML_CMP(block, uml::I0, uml::I1);
			UML_JMPc(block, uml::COND_NE, unexecuted);
			break;
		case COND_LT:
			UML_TEST(block, DRC_CPSR, N_MASK);
			UML_MOVc(block, uml::COND_Z, uml::I0, 0);
			UML_MOVc(block, uml::COND_NZ, uml::I0, 1);
			UML_TEST(block, DRC_CPSR, V_MASK);
			UML_MOVc(block, uml::COND_Z, uml::I1, 0);
			UML_MOVc(block, uml::COND_NZ, uml::I1, 1);
			UML_CMP(block, uml::I0, uml::I1);
			UML_JMPc(block, uml::COND_E, unexecuted);
			break;
		case COND_GT:
			UML_TEST(block, DRC_CPSR, Z_MASK);
			UML_JMPc(block, uml::COND_NZ, unexecuted);
			UML_TEST(block, DRC_CPSR, N_MASK);
			UML_MOVc(block, uml::COND_Z, uml::I0, 0);
			UML_MOVc(block, uml::COND_NZ, uml::I0, 1);
			UML_TEST(block, DRC_CPSR, V_MASK);
			UML_MOVc(block, uml::COND_Z, uml::I1, 0);
			UML_MOVc(block, uml::COND_NZ, uml::I1, 1);
			UML_CMP(block, uml::I0, uml::I1);
			UML_JMPc(block, uml::COND_NE, unexecuted);
			break;
		case COND_LE:
			UML_TEST(block, DRC_CPSR, N_MASK);
			UML_MOVc(block, uml::COND_Z, uml::I0, 0);
			UML_MOVc(block, uml::COND_NZ, uml::I0, 1);
			UML_TEST(block, DRC_CPSR, V_MASK);
			UML_MOVc(block, uml::COND_Z, uml::I1, 0);
			UML_MOVc(block, uml::COND_NZ, uml::I1, 1);
			UML_CMP(block, uml::I0, uml::I1);
			UML_JMPc(block, uml::COND_NE, contdecode);
			UML_TEST(block, DRC_CPSR, Z_MASK);
			UML_JMPc(block, uml::COND_Z, unexecuted);
			break;
		case COND_NV:
			UML_JMP(block, unexecuted);
			break;
	}

	UML_LABEL(block, contdecode);

	(this->*drcops_handler[(op & 0xF000000) >> 24])(block, compiler, desc, op);

	UML_LABEL(block, unexecuted);
	UML_ADD(block, DRC_PC, DRC_PC, 4);
	UML_ADD(block, MAPVAR_CYCLES, MAPVAR_CYCLES, 2);                                // add      cycles, cycles, 2

	UML_LABEL(block, skip);

	switch (opswitch)
	{
		/* ----- sub-groups ----- */

		case 0x00:  /* SPECIAL - MIPS I */
			return TRUE;

		// TODO: FINISH ME
	}

	return FALSE;
}

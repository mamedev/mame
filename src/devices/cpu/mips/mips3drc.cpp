// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mips3drc.c

    Universal machine language-based MIPS III/IV emulator.

****************************************************************************

    Future improvements/changes:

    * Add DRC option to flush PC before calling memory handlers

    * Constant tracking? (hasn't bought us much in the past)

    * Customized mapped/unmapped memory handlers
        - create 3 sets of handlers: cached, uncached, general
        - default to general
        - in general case, if cached use RECALL to point to cached code
        - (same for uncached)
        - in cached/uncached case, fall back to general case

***************************************************************************/

#include "emu.h"
#include "mips3com.h"
#include "mips3fe.h"
#include "mips3dsm.h"
#include "ps2vu.h"
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"


/* Use with STRICT_VERIFY to print debug info to console for extra validation checks */
/* Set to 1 to activate and use MIPS3DRC_STRICT_VERIFY in the drc options */
#define DEBUG_STRICT_VERIFY 0


/***************************************************************************
    MACROS
***************************************************************************/

#define IS_FR0      ((m_core->mode & 1) == 0)
#define IS_FR1      ((m_core->mode & 1) != 0)

#define R32(reg)                m_regmaplo[reg]
#define LO32                    R32(REG_LO)
#define HI32                    R32(REG_HI)
#define CPR032(reg)             uml::mem(LOPTR(&m_core->cpr[0][reg]))
#define CCR032(reg)             uml::mem(LOPTR(&m_core->ccr[0][reg]))
#define FPR32(reg)              uml::mem((IS_FR0) ? &((float *)&m_core->cpr[1][reg & 0x1E])[BYTE_XOR_LE(reg & 1)] : (float *)&m_core->cpr[1][reg])
#define CCR132(reg)             uml::mem(LOPTR(&m_core->ccr[1][reg]))
#define CPR232(reg)             uml::mem(LOPTR(&m_core->cpr[2][reg]))
#define CCR232(reg)             uml::mem(LOPTR(&m_core->ccr[2][reg]))

#define R64(reg)                m_regmap[reg]
#define LO64                    R64(REG_LO)
#define HI64                    R64(REG_HI)
#define CPR064(reg)             uml::mem(&m_core->cpr[0][reg])
#define CCR064(reg)             uml::mem(&m_core->ccr[0][reg])
#define FPR64(reg)              uml::mem((IS_FR0) ? (double *)&m_core->cpr[1][reg & 0x1E] : (double *)&m_core->cpr[1][reg])
#define CCR164(reg)             uml::mem(&m_core->ccr[1][reg])
#define CPR264(reg)             uml::mem(&m_core->cpr[2][reg])
#define CCR264(reg)             uml::mem(&m_core->ccr[2][reg])

#define FCCSHIFT(which)         fcc_shift[(m_flavor < MIPS3_TYPE_MIPS_IV) ? 0 : ((which) & 7)]
#define FCCMASK(which)          (uint32_t(1 << FCCSHIFT(which)))



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void cfunc_printf_exception(void *param);
static void cfunc_get_cycles(void *param);
static void cfunc_printf_probe(void *param);
static void cfunc_debug_break(void *param);


/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

/* bit indexes for various FCCs */
static const uint8_t fcc_shift[8] = { 23, 25, 26, 27, 28, 29, 30, 31 };


/***************************************************************************
    INLINE FUNCTIONS
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

static inline void alloc_handle(drcuml_state &drcuml, uml::code_handle *&handleptr, const char *name)
{
	if (!handleptr)
		handleptr = drcuml.handle_alloc(name);
}


/*-------------------------------------------------
    load_fast_iregs - load any fast integer
    registers
-------------------------------------------------*/

inline void mips3_device::load_fast_iregs(drcuml_block &block)
{
	int regnum;

	for (regnum = 0; regnum < std::size(m_regmap); regnum++)
		if (m_regmap[regnum].is_int_register())
			UML_DMOV(block, ireg(m_regmap[regnum].ireg() - REG_I0), mem(&m_core->r[regnum]));
}


/*-------------------------------------------------
    save_fast_iregs - save any fast integer
    registers
-------------------------------------------------*/

inline void mips3_device::save_fast_iregs(drcuml_block &block)
{
	int regnum;

	for (regnum = 0; regnum < std::size(m_regmap); regnum++)
		if (m_regmap[regnum].is_int_register())
			UML_DMOV(block, mem(&m_core->r[regnum]), ireg(m_regmap[regnum].ireg() - REG_I0));
}



/***************************************************************************
    CORE CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    mips3drc_set_options - configure DRC options
-------------------------------------------------*/

void mips3_device::mips3drc_set_options(uint32_t options)
{
	if (!allow_drc()) return;
	m_drcoptions = options;
}

/*-------------------------------------------------
    mips3drc_get_options - return current DRC options
-------------------------------------------------*/

uint32_t mips3_device::mips3drc_get_options()
{
	return m_drcoptions;
}

/*-------------------------------------------------
    mips3drc_clears_fastram - clears fastram
    region starting at index select_start
-------------------------------------------------*/
void mips3_device::clear_fastram(uint32_t select_start)
{
	m_fastram_select=select_start;
	// Set cache to dirty so that re-mapping occurs
	m_drc_cache_dirty = true;
}

/*-------------------------------------------------
    mips3drc_add_fastram - add a new fastram
    region
-------------------------------------------------*/

void mips3_device::add_fastram(offs_t start, offs_t end, uint8_t readonly, void *base)
{
	if (m_fastram_select < std::size(m_fastram))
	{
		m_fastram[m_fastram_select].start = start;
		m_fastram[m_fastram_select].end = end;
		m_fastram[m_fastram_select].readonly = readonly;
		m_fastram[m_fastram_select].base = base;
		m_fastram[m_fastram_select].offset_base8 = (uint8_t*)base - start;
		m_fastram[m_fastram_select].offset_base16 = (uint16_t*)((uint8_t*)base - start);
		m_fastram[m_fastram_select].offset_base32 = (uint32_t*)((uint8_t*)base - start);
		m_fastram_select++;
		// Set cache to dirty so that re-mapping occurs
		m_drc_cache_dirty = true;
	}
}


/*-------------------------------------------------
    mips3drc_add_hotspot - add a new hotspot
-------------------------------------------------*/

void mips3_device::mips3drc_add_hotspot(offs_t pc, uint32_t opcode, uint32_t cycles)
{
	if (!allow_drc()) return;
	if (m_hotspot_select < std::size(m_hotspot))
	{
		m_hotspot[m_hotspot_select].pc = pc;
		m_hotspot[m_hotspot_select].opcode = opcode;
		m_hotspot[m_hotspot_select].cycles = cycles;
		m_hotspot_select++;
	}
}



/***************************************************************************
    CACHE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    code_flush_cache - flush the cache and
    regenerate static code
-------------------------------------------------*/

void mips3_device::code_flush_cache()
{
	int mode;

	/* empty the transient cache contents */
	m_drcuml->reset();

	try
	{
		{
			drcuml_block &block(m_drcuml->begin_block(1024));
			int label = 1;

			/* generate the entry point and out-of-cycles handlers */
			static_generate_entry_point(block, label);
			static_generate_nocode_handler(block, label);
			static_generate_out_of_cycles(block, label);
			static_generate_tlb_mismatch(block, label);

			/* append exception handlers for various types */
			static_generate_exception(block, label, EXCEPTION_INTERRUPT,     true,  "exception_interrupt");
			static_generate_exception(block, label, EXCEPTION_INTERRUPT,     false, "exception_interrupt_norecover");
			static_generate_exception(block, label, EXCEPTION_TLBMOD,        true,  "exception_tlbmod");
			static_generate_exception(block, label, EXCEPTION_TLBLOAD,       true,  "exception_tlbload");
			static_generate_exception(block, label, EXCEPTION_TLBSTORE,      true,  "exception_tlbstore");
			static_generate_exception(block, label, EXCEPTION_TLBLOAD_FILL,  true,  "exception_tlbload_fill");
			static_generate_exception(block, label, EXCEPTION_TLBSTORE_FILL, true,  "exception_tlbstore_fill");
			static_generate_exception(block, label, EXCEPTION_ADDRLOAD,      true,  "exception_addrload");
			static_generate_exception(block, label, EXCEPTION_ADDRSTORE,     true,  "exception_addrstore");
			static_generate_exception(block, label, EXCEPTION_SYSCALL,       true,  "exception_syscall");
			static_generate_exception(block, label, EXCEPTION_BREAK,         true,  "exception_break");
			static_generate_exception(block, label, EXCEPTION_INVALIDOP,     true,  "exception_invalidop");
			static_generate_exception(block, label, EXCEPTION_BADCOP,        true,  "exception_badcop");
			static_generate_exception(block, label, EXCEPTION_OVERFLOW,      true,  "exception_overflow");
			static_generate_exception(block, label, EXCEPTION_TRAP,          true,  "exception_trap");
			static_generate_exception(block, label, EXCEPTION_FPE,           true,  "exception_fpe");

			block.end();
		}

		/* add subroutines for memory accesses */
		for (mode = 0; mode < 3; mode++)
		{
			drcuml_block &block(m_drcuml->begin_block(1024));
			int label = 1;
			static_generate_memory_accessor(block, label, mode, 1, false, false, "read8",       m_read8[mode]);
			static_generate_memory_accessor(block, label, mode, 1, true,  false, "write8",      m_write8[mode]);
			static_generate_memory_accessor(block, label, mode, 2, false, false, "read16",      m_read16[mode]);
			static_generate_memory_accessor(block, label, mode, 2, true,  false, "write16",     m_write16[mode]);
			static_generate_memory_accessor(block, label, mode, 4, false, false, "read32",      m_read32[mode]);
			static_generate_memory_accessor(block, label, mode, 4, false, true,  "read32mask",  m_read32mask[mode]);
			static_generate_memory_accessor(block, label, mode, 4, true,  false, "write32",     m_write32[mode]);
			static_generate_memory_accessor(block, label, mode, 4, true,  true,  "write32mask", m_write32mask[mode]);
			static_generate_memory_accessor(block, label, mode, 8, false, false, "read64",      m_read64[mode]);
			static_generate_memory_accessor(block, label, mode, 8, false, true,  "read64mask",  m_read64mask[mode]);
			static_generate_memory_accessor(block, label, mode, 8, true,  false, "write64",     m_write64[mode]);
			static_generate_memory_accessor(block, label, mode, 8, true,  true,  "write64mask", m_write64mask[mode]);
			block.end();
		}
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

void mips3_device::code_compile_block(uint8_t mode, offs_t pc)
{
	compiler_state compiler = { 0 };
	const opcode_desc *seqhead, *seqlast;
	const opcode_desc *codelast;
	const opcode_desc *desclist;
	bool override = false;

	auto profile = g_profiler.start(PROFILER_DRC_COMPILE);

	/* get a description of this sequence */
	desclist = m_drcfe->describe_code(pc);
	/* get last instruction of the code (potentially used in generate_checksum) */
	codelast = m_drcfe->get_last();
	if (m_drcuml->logging() || m_drcuml->logging_native())
		log_opcode_desc(desclist, 0);

	/* if we get an error back, flush the cache and try again */
	bool succeeded = false;
	while (!succeeded)
	{
		try
		{
			/* start the block */
			drcuml_block &block(m_drcuml->begin_block(4096));

			/* loop until we get through all instruction sequences */
			for (seqhead = desclist; seqhead != nullptr; seqhead = seqlast->next())
			{
				const opcode_desc *curdesc;
				uint32_t nextpc;

				/* add a code log entry */
				if (m_drcuml->logging())
					block.append_comment("-------------------------");                     // comment

				/* determine the last instruction in this sequence */
				for (seqlast = seqhead; seqlast != nullptr; seqlast = seqlast->next())
					if (seqlast->flags & OPFLAG_END_SEQUENCE)
						break;
				assert(seqlast != nullptr);

				/* if we don't have a hash for this mode/pc, or if we are overriding all, add one */
				if (override || !m_drcuml->hash_exists(mode, seqhead->pc))
				{
					UML_HASH(block, mode, seqhead->pc);                                     // hash    mode,pc
				}
				/* if we already have a hash, and this is the first sequence, assume that we */
				/* are recompiling due to being out of sync and allow future overrides */
				else if (seqhead == desclist)
				{
					override = true;
					UML_HASH(block, mode, seqhead->pc);                                     // hash    mode,pc
				}

				/* otherwise, redispatch to that fixed PC and skip the rest of the processing */
				else
				{
					UML_LABEL(block, seqhead->pc | 0x80000000);                             // label   seqhead->pc | 0x80000000
					UML_HASHJMP(block, m_core->mode, seqhead->pc, *m_nocode);               // hashjmp <mode>,seqhead->pc,nocode
					continue;
				}

				/* validate this code block if we're not pointing into ROM */
				if (m_program->get_write_ptr(seqhead->physpc) != nullptr)
					generate_checksum_block(block, compiler, seqhead, seqlast, codelast);

				/* label this instruction, if it may be jumped to locally */
				if (seqhead->flags & OPFLAG_IS_BRANCH_TARGET)
				{
					UML_LABEL(block, seqhead->pc | 0x80000000);                             // label   seqhead->pc | 0x80000000
				}

				/* iterate over instructions in the sequence and compile them */
				for (curdesc = seqhead; curdesc != seqlast->next(); curdesc = curdesc->next())
				{
					generate_sequence_instruction(block, compiler, curdesc);
				}

				/* if we need to return to the start, do it */
				if (seqlast->flags & OPFLAG_RETURN_TO_START)
					nextpc = pc;

				/* otherwise we just go to the next instruction */
				else
					nextpc = seqlast->pc + (seqlast->skipslots + 1) * 4;

				/* count off cycles and go there */
				generate_update_cycles(block, compiler, nextpc, true);          // <subtract cycles>

				/* if the last instruction can change modes, use a variable mode; otherwise, assume the same mode */
				if (seqlast->flags & OPFLAG_CAN_CHANGE_MODES)
				{
					UML_HASHJMP(block, mem(&m_core->mode), nextpc, *m_nocode);              // hashjmp <mode>,nextpc,nocode
				}
				else if (seqlast->next() == nullptr || seqlast->next()->pc != nextpc)
				{
					UML_HASHJMP(block, m_core->mode, nextpc, *m_nocode);                    // hashjmp <mode>,nextpc,nocode
				}
			}

			/* end the sequence */
			block.end();
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

static void cfunc_mips3com_update_cycle_counting(void *param)
{
	((mips3_device *)param)->mips3com_update_cycle_counting();
}

static void cfunc_mips3com_asid_changed(void *param)
{
	((mips3_device *)param)->mips3com_asid_changed();
}

static void cfunc_mips3com_tlbr(void *param)
{
	((mips3_device *)param)->mips3com_tlbr();
}

static void cfunc_mips3com_tlbwi(void *param)
{
	((mips3_device *)param)->mips3com_tlbwi();
}

static void cfunc_mips3com_tlbwr(void *param)
{
	((mips3_device *)param)->mips3com_tlbwr();
}

static void cfunc_mips3com_tlbp(void *param)
{
	((mips3_device *)param)->mips3com_tlbp();
}

/*-------------------------------------------------
    cfunc_get_cycles - compute the total number
    of cycles executed so far
-------------------------------------------------*/

void mips3_device::func_get_cycles()
{
	m_core->numcycles = total_cycles();
}

static void cfunc_get_cycles(void *param)
{
	((mips3_device *)param)->func_get_cycles();
}


/*-------------------------------------------------
    cfunc_printf_exception - log any exceptions that
    aren't interrupts
-------------------------------------------------*/

void mips3_device::func_printf_exception()
{
	printf("Exception: EPC=%08X Cause=%08X BadVAddr=%08X Jmp=%08X\n", (uint32_t)m_core->cpr[0][COP0_EPC], (uint32_t)m_core->cpr[0][COP0_Cause], (uint32_t)m_core->cpr[0][COP0_BadVAddr], m_core->pc);
	func_printf_probe();
}

static void cfunc_printf_exception(void *param)
{
	((mips3_device *)param)->func_printf_exception();
}

/*-------------------------------------------------
    cfunc_printf_debug - generic printf for
    debugging
-------------------------------------------------*/

void mips3_device::func_printf_debug()
{
	printf(m_core->format, m_core->arg0, m_core->arg1);
}

static void cfunc_printf_debug(void *param)
{
	((mips3_device *)param)->func_printf_debug();
}


/*-------------------------------------------------
    cfunc_printf_probe - print the current CPU
    state and return
-------------------------------------------------*/

void mips3_device::func_printf_probe()
{
	printf(" PC=%08X          r1=%08X%08X  r2=%08X%08X  r3=%08X%08X\n",
		m_core->pc,
		(uint32_t)(m_core->r[1] >> 32), (uint32_t)m_core->r[1],
		(uint32_t)(m_core->r[2] >> 32), (uint32_t)m_core->r[2],
		(uint32_t)(m_core->r[3] >> 32), (uint32_t)m_core->r[3]);
	printf(" r4=%08X%08X  r5=%08X%08X  r6=%08X%08X  r7=%08X%08X\n",
		(uint32_t)(m_core->r[4] >> 32), (uint32_t)m_core->r[4],
		(uint32_t)(m_core->r[5] >> 32), (uint32_t)m_core->r[5],
		(uint32_t)(m_core->r[6] >> 32), (uint32_t)m_core->r[6],
		(uint32_t)(m_core->r[7] >> 32), (uint32_t)m_core->r[7]);
	printf(" r8=%08X%08X  r9=%08X%08X r10=%08X%08X r11=%08X%08X\n",
		(uint32_t)(m_core->r[8] >> 32), (uint32_t)m_core->r[8],
		(uint32_t)(m_core->r[9] >> 32), (uint32_t)m_core->r[9],
		(uint32_t)(m_core->r[10] >> 32), (uint32_t)m_core->r[10],
		(uint32_t)(m_core->r[11] >> 32), (uint32_t)m_core->r[11]);
	printf("r12=%08X%08X r13=%08X%08X r14=%08X%08X r15=%08X%08X\n",
		(uint32_t)(m_core->r[12] >> 32), (uint32_t)m_core->r[12],
		(uint32_t)(m_core->r[13] >> 32), (uint32_t)m_core->r[13],
		(uint32_t)(m_core->r[14] >> 32), (uint32_t)m_core->r[14],
		(uint32_t)(m_core->r[15] >> 32), (uint32_t)m_core->r[15]);
	printf("r16=%08X%08X r17=%08X%08X r18=%08X%08X r19=%08X%08X\n",
		(uint32_t)(m_core->r[16] >> 32), (uint32_t)m_core->r[16],
		(uint32_t)(m_core->r[17] >> 32), (uint32_t)m_core->r[17],
		(uint32_t)(m_core->r[18] >> 32), (uint32_t)m_core->r[18],
		(uint32_t)(m_core->r[19] >> 32), (uint32_t)m_core->r[19]);
	printf("r20=%08X%08X r21=%08X%08X r22=%08X%08X r23=%08X%08X\n",
		(uint32_t)(m_core->r[20] >> 32), (uint32_t)m_core->r[20],
		(uint32_t)(m_core->r[21] >> 32), (uint32_t)m_core->r[21],
		(uint32_t)(m_core->r[22] >> 32), (uint32_t)m_core->r[22],
		(uint32_t)(m_core->r[23] >> 32), (uint32_t)m_core->r[23]);
	printf("r24=%08X%08X r25=%08X%08X r26=%08X%08X r27=%08X%08X\n",
		(uint32_t)(m_core->r[24] >> 32), (uint32_t)m_core->r[24],
		(uint32_t)(m_core->r[25] >> 32), (uint32_t)m_core->r[25],
		(uint32_t)(m_core->r[26] >> 32), (uint32_t)m_core->r[26],
		(uint32_t)(m_core->r[27] >> 32), (uint32_t)m_core->r[27]);
	printf("r28=%08X%08X r29=%08X%08X r30=%08X%08X r31=%08X%08X\n",
		(uint32_t)(m_core->r[28] >> 32), (uint32_t)m_core->r[28],
		(uint32_t)(m_core->r[29] >> 32), (uint32_t)m_core->r[29],
		(uint32_t)(m_core->r[30] >> 32), (uint32_t)m_core->r[30],
		(uint32_t)(m_core->r[31] >> 32), (uint32_t)m_core->r[31]);
	printf(" hi=%08X%08X  lo=%08X%08X\n",
		(uint32_t)(m_core->r[REG_HI] >> 32), (uint32_t)m_core->r[REG_HI],
		(uint32_t)(m_core->r[REG_LO] >> 32), (uint32_t)m_core->r[REG_LO]);
}

static void cfunc_printf_probe(void *param)
{
	((mips3_device *)param)->func_printf_probe();
}

/*-------------------------------------------------
    func_debug_break - debugger break
-------------------------------------------------*/

void mips3_device::func_debug_break()
{
	machine().debug_break();
}

static void cfunc_debug_break(void *param)
{
	((mips3_device *)param)->func_debug_break();
}

/*-------------------------------------------------
    cfunc_unimplemented - handler for
    unimplemented opcdes
-------------------------------------------------*/

void mips3_device::func_unimplemented()
{
	uint32_t opcode = m_core->arg0;
	fatalerror("PC=%08X: Unimplemented op %08X (%02X,%02X)\n", m_core->pc, opcode, opcode >> 26, opcode & 0x3f);
}

static void cfunc_unimplemented(void *param)
{
	((mips3_device *)param)->func_unimplemented();
}


/***************************************************************************
    STATIC CODEGEN
***************************************************************************/

/*-------------------------------------------------
    static_generate_entry_point - generate a
    static entry point
-------------------------------------------------*/

void mips3_device::static_generate_entry_point(drcuml_block &block, int &label)
{
	uml::code_label const skip = label++;

	/* forward references */
	alloc_handle(*m_drcuml, m_exception_norecover[EXCEPTION_INTERRUPT], "interrupt_norecover");
	alloc_handle(*m_drcuml, m_nocode, "nocode");

	alloc_handle(*m_drcuml, m_entry, "entry");
	UML_HANDLE(block, *m_entry);                                                // handle  entry

	/* reset the FPU mode */
	UML_AND(block, I0, CCR132(31), 3);                                          // and     i0,ccr1[31],3
	UML_LOAD(block, I0, &m_fpmode[0], I0, SIZE_BYTE, SCALE_x1);                 // load    i0,fpmode,i0,byte
	UML_SETFMOD(block, I0);                                                     // setfmod i0

	/* load fast integer registers */
	load_fast_iregs(block);

	/* check for interrupts */
	UML_AND(block, I0, CPR032(COP0_Cause), CPR032(COP0_Status));                // and     i0,[Cause],[Status]
	UML_AND(block, I0, I0, 0xfc00);                                             // and     i0,i0,0xfc00,Z
	UML_JMPc(block, COND_Z, skip);                                              // jmp     skip,Z
	UML_TEST(block, CPR032(COP0_Status), SR_IE);                                // test    [Status],SR_IE
	UML_JMPc(block, COND_Z, skip);                                              // jmp     skip,Z
	UML_TEST(block, CPR032(COP0_Status), SR_EXL | SR_ERL);                      // test    [Status],SR_EXL | SR_ERL
	UML_JMPc(block, COND_NZ, skip);                                             // jmp     skip,NZ
	UML_MOV(block, I0, mem(&m_core->pc));                                       // mov     i0,pc
	UML_MOV(block, I1, 0);                                                      // mov     i1,0
	UML_CALLH(block, *m_exception_norecover[EXCEPTION_INTERRUPT]);              // callh   exception_norecover
	UML_LABEL(block, skip);                                                     // skip:

	/* generate a hash jump via the current mode and PC */
	UML_HASHJMP(block, mem(&m_core->mode), mem(&m_core->pc), *m_nocode);        // hashjmp <mode>,<pc>,nocode
}


/*-------------------------------------------------
    static_generate_nocode_handler - generate an
    exception handler for "out of code"
-------------------------------------------------*/

void mips3_device::static_generate_nocode_handler(drcuml_block &block, int &label)
{
	/* generate a hash jump via the current mode and PC */
	alloc_handle(*m_drcuml, m_nocode, "nocode");
	UML_HANDLE(block, *m_nocode);                                               // handle  nocode
	UML_GETEXP(block, I0);                                                      // getexp  i0
	UML_MOV(block, mem(&m_core->pc), I0);                                       // mov     [pc],i0
	save_fast_iregs(block);
	UML_EXIT(block, EXECUTE_MISSING_CODE);                                      // exit    EXECUTE_MISSING_CODE
}


/*-------------------------------------------------
    static_generate_out_of_cycles - generate an
    out of cycles exception handler
-------------------------------------------------*/

void mips3_device::static_generate_out_of_cycles(drcuml_block &block, int &label)
{
	/* generate a hash jump via the current mode and PC */
	alloc_handle(*m_drcuml, m_out_of_cycles, "out_of_cycles");
	UML_HANDLE(block, *m_out_of_cycles);                                        // handle  out_of_cycles
	UML_GETEXP(block, I0);                                                      // getexp  i0
	UML_MOV(block, mem(&m_core->pc), I0);                                       // mov     <pc>,i0
	save_fast_iregs(block);
	UML_EXIT(block, EXECUTE_OUT_OF_CYCLES);                                     // exit    EXECUTE_OUT_OF_CYCLES
}


/*-------------------------------------------------
    static_generate_tlb_mismatch - generate a
    TLB mismatch handler
-------------------------------------------------*/

void mips3_device::static_generate_tlb_mismatch(drcuml_block &block, int &label)
{
	uml::code_label const skip = label++;

	/* forward references */
	alloc_handle(*m_drcuml, m_exception[EXCEPTION_TLBLOAD], "exception_tlbload");
	alloc_handle(*m_drcuml, m_exception[EXCEPTION_TLBLOAD_FILL], "exception_tlbload_fill");

	/* generate a hash jump via the current mode and PC */
	alloc_handle(*m_drcuml, m_tlb_mismatch, "tlb_mismatch");
	UML_HANDLE(block, *m_tlb_mismatch);                                         // handle  tlb_mismatch
	UML_RECOVER(block, I0, MAPVAR_PC);                                          // recover i0,PC
	// Need the current PC address for the instruction to do the TLB lookup
	// so need to adjust I0 so it points to the true PC address in case this is a delay slot
	UML_ADD(block, I0, I0, 3);                                                  // add     i1,i0,3
	UML_AND(block, I0, I0, ~3);                                                 // and     i0,i0,~3
	UML_SHR(block, I1, I0, 12);                                                 // shr     i1,i0,12
	UML_LOAD(block, I1, (void *)vtlb_table(), I1, SIZE_DWORD, SCALE_x4);        // load    i1,[vtlb_table],i1,dword
	if (PRINTF_MMU)
	{
		static const char text[] = "TLB mismatch @ %08X (ent=%08X)\n";
		UML_MOV(block, mem(&m_core->format), (uintptr_t)text);                  // mov     [format],text
		UML_MOV(block, mem(&m_core->arg0), I0);                                 // mov     [arg0],i0
		UML_MOV(block, mem(&m_core->arg1), I1);                                 // mov     [arg1],i1
		UML_CALLC(block, cfunc_printf_debug, this);                             // callc   printf_debug
	}
	UML_TEST(block, I1, FETCH_ALLOWED);                                         // test    i1,FETCH_ALLOWED
	UML_JMPc(block, COND_NZ, skip);                                             // jmp     skip,nz
	UML_TEST(block, I1, FLAG_FIXED);                                            // test    i1,FLAG_FIXED
	UML_EXHc(block, COND_NZ, *m_exception[EXCEPTION_TLBLOAD], I0);              // exh     exception[TLBLOAD],i0,nz
	UML_EXH(block, *m_exception[EXCEPTION_TLBLOAD_FILL], I0);                   // exh     exception[TLBLOAD_FILL],i0
	UML_LABEL(block, skip);                                                     // skip:
	save_fast_iregs(block);

	// the saved PC may be set 1 instruction back with the low bit set to indicate
	// a delay slot; in this path we want the previous instruction address, so recover it
	UML_RECOVER(block, I0, MAPVAR_PC);                                          // recover i0,PC
	UML_AND(block, I0, I0, ~3);                                                 // and     i0,i0,~3
	UML_MOV(block, mem(&m_core->pc), I0);                                       // mov     i0,[pc]

	UML_EXIT(block, EXECUTE_MISSING_CODE);                                      // exit    EXECUTE_MISSING_CODE
}


/*-------------------------------------------------
    static_generate_exception - generate a static
    exception handler
-------------------------------------------------*/

void mips3_device::static_generate_exception(drcuml_block &block, int &label, uint8_t exception, int recover, const char *name)
{
	uml::code_handle *&exception_handle = recover ? m_exception[exception] : m_exception_norecover[exception];
	uint32_t offset = 0x180;
	uml::code_label const next = label++;
	uml::code_label const skip = label++;

	/* translate our fake fill exceptions into real exceptions */
	if (exception == EXCEPTION_TLBLOAD_FILL || exception == EXCEPTION_TLBSTORE_FILL)
	{
		offset = 0x000;
		exception = (exception - EXCEPTION_TLBLOAD_FILL) + EXCEPTION_TLBLOAD;
	}

	/* add a global entry for this */
	alloc_handle(*m_drcuml, exception_handle, name);
	UML_HANDLE(block, *exception_handle);                                           // handle  name

	/* exception parameter is expected to be the fault address in this case */
	if (exception == EXCEPTION_TLBLOAD || exception == EXCEPTION_TLBSTORE || exception == EXCEPTION_TLBMOD || exception == EXCEPTION_ADDRLOAD || exception == EXCEPTION_ADDRSTORE)
	{
		/* set BadVAddr to the fault address */
		UML_GETEXP(block, I0);                                                  // getexp  i0
		UML_MOV(block, CPR032(COP0_BadVAddr), I0);                              // mov     [BadVAddr],i0
	}

	if (exception == EXCEPTION_TLBMOD || exception == EXCEPTION_TLBLOAD || exception == EXCEPTION_TLBSTORE)
	{
		/* set the upper bits of EntryHi and the lower bits of Context to the fault page */
		UML_ROLINS(block, CPR032(COP0_EntryHi), I0, 0, 0xffffe000); // rolins  [EntryHi],i0,0,0xffffe000
		UML_ROLINS(block, CPR032(COP0_Context), I0, 32-9, 0x7ffff0);    // rolins  [Context],i0,32-9,0x7ffff0
	}

	if (exception == EXCEPTION_FPE)
	{
		/* set the flag and cause */
		UML_GETEXP(block, I0);                                      // getexp  i0
		UML_ROLINS(block, CCR132(31), I0, FCR31_FLAGS, 0x0000007c); // rolins  [CCR31],i0,FCR31_FLAGS,0x0000007c
		UML_ROLINS(block, CCR132(31), I0, FCR31_CAUSE, 0x0003f000); // rolins  [CCR31],i0,FCR31_CAUSE,0x0003f000
	}

	/* set the EPC and Cause registers */
	if (recover)
	{
		UML_RECOVER(block, I0, MAPVAR_PC);                                      // recover i0,PC
		UML_RECOVER(block, I1, MAPVAR_CYCLES);                                  // recover i1,CYCLES
	}

	UML_AND(block, I2, CPR032(COP0_Cause), ~0x800000ff);                        // and     i2,[Cause],~0x800000ff
	UML_TEST(block, I0, 1);                                                     // test    i0,1
	UML_JMPc(block, COND_Z, next);                                              // jz      <next>
	UML_OR(block, I2, I2, 0x80000000);                                          // or      i2,i2,0x80000000
	UML_SUB(block, I0, I0, 1);                                                  // sub     i0,i0,1
	UML_LABEL(block, next);                                                     // <next>:
	UML_MOV(block, I3, offset);                                                 // mov     i3,offset
	UML_TEST(block, CPR032(COP0_Status), SR_EXL);                               // test    [Status],SR_EXL
	UML_MOVc(block, COND_Z, CPR032(COP0_EPC), I0);                              // mov     [EPC],i0,Z
	UML_MOVc(block, COND_NZ, I3, 0x180);                                        // mov     i3,0x180,NZ
	UML_OR(block, CPR032(COP0_Cause), I2, exception << 2);                      // or      [Cause],i2,exception << 2

	/* for BADCOP exceptions, we use the exception parameter to know which COP */
	if (exception == EXCEPTION_BADCOP)
	{
		UML_GETEXP(block, I0);                                                  // getexp  i0
		UML_ROLINS(block, CPR032(COP0_Cause), I0, 28, 0x30000000);              // rolins  [Cause],i0,28,0x30000000
	}

	/* set EXL in the SR */
	UML_OR(block, I0, CPR032(COP0_Status), SR_EXL);                             // or      i0,[Status],SR_EXL
	UML_MOV(block, CPR032(COP0_Status), I0);                                    // mov     [Status],i0
	generate_update_mode(block);

	/* optionally print exceptions */
	if ((PRINTF_EXCEPTIONS && exception != EXCEPTION_INTERRUPT && exception != EXCEPTION_SYSCALL) ||
		(PRINTF_MMU && (exception == EXCEPTION_TLBLOAD || exception == EXCEPTION_TLBSTORE)))
	{
		UML_CALLC(block, cfunc_printf_exception, this);                         // callc   cfunc_printf_exception,nullptr
		//UML_CALLC(block, cfunc_debug_break, this);                            // callc   cfunc_debug_break,nullptr
	}

	/* choose our target PC */
	UML_ADD(block, I0, I3, 0xbfc00200);                                         // add     i0,i3,0xbfc00200
	UML_TEST(block, CPR032(COP0_Status), SR_BEV);                               // test    CPR032(COP0_Status),SR_BEV
	UML_JMPc(block, COND_NZ, skip);                                             // jnz     <skip>
	UML_ADD(block, I0, I3, 0x80000000);                                         // add     i0,i3,0x80000000,z
	UML_LABEL(block, skip);                                                     // <skip>:

	/* adjust cycles */
	UML_SUB(block, mem(&m_core->icount), mem(&m_core->icount), I1);             // sub icount,icount,cycles,S
	UML_EXHc(block, COND_S, *m_out_of_cycles, I0);                              // exh     out_of_cycles,i0

	UML_HASHJMP(block, mem(&m_core->mode), I0, *m_nocode);                      // hashjmp <mode>,i0,nocode
}


/*------------------------------------------------------------------
    static_generate_memory_mode_checks
------------------------------------------------------------------*/

void mips3_device::static_generate_memory_mode_checks(drcuml_block &block, uml::code_handle &exception_addrerr, int &label, int mode)
{
	/* user mode? generate address exception if top bit is set */
	if (mode == MODE_USER)
	{
		UML_TEST(block, I0, 0x80000000);                                            // test    i0,0x80000000
		UML_EXHc(block, COND_NZ, exception_addrerr, I0);                            // exh     addrerr,i0,nz
	}

	/* supervisor mode? generate address exception if not in user space or in $C0000000-DFFFFFFF */
	if (mode == MODE_SUPER)
	{
		int addrok;
		UML_TEST(block, I0, 0x80000000);                                            // test    i0,0x80000000
		UML_JMPc(block, COND_Z, addrok = label++);                                  // jz      addrok
		UML_SHR(block, I3, I0, 29);                                                 // shr     i3,i0,29
		UML_CMP(block, I3, 6);                                                      // cmp     i3,6
		UML_EXHc(block, COND_NE, exception_addrerr, I0);                            // exh     addrerr,i0,ne
		UML_LABEL(block, addrok);                                                   // addrok:
	}
}


/*------------------------------------------------------------------
    static_generate_fastram_accessor
------------------------------------------------------------------*/

void mips3_device::static_generate_fastram_accessor(drcuml_block &block, int &label, int size, bool iswrite, bool ismasked)
{
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) == 0)
	{
		for (int ramnum = 0; ramnum < m_fastram_select; ramnum++)
		{
			if (!(iswrite && m_fastram[ramnum].readonly))
			{
				void *fastbase = (uint8_t *)m_fastram[ramnum].base - m_fastram[ramnum].start;
				uint32_t skip = label++;
				if (m_fastram[ramnum].end != 0xffffffff)
				{
					UML_CMP(block, I0, m_fastram[ramnum].end);                      // cmp     i0,end
					UML_JMPc(block, COND_A, skip);                                  // ja      skip
				}
				if (m_fastram[ramnum].start != 0x00000000)
				{
					UML_CMP(block, I0, m_fastram[ramnum].start);                    // cmp     i0,fastram_start
					UML_JMPc(block, COND_B, skip);                                  // jb      skip
				}
				if (!iswrite)
				{
					if (size == 1)
					{
						UML_XOR(block, I0, I0, m_bigendian ? BYTE4_XOR_BE(0) : BYTE4_XOR_LE(0));
																					// xor     i0,i0,bytexor
						UML_LOAD(block, I0, fastbase, I0, SIZE_BYTE, SCALE_x1);     // load    i0,fastbase,i0,byte
					}
					else if (size == 2)
					{
						UML_XOR(block, I0, I0, m_bigendian ? WORD_XOR_BE(0) : WORD_XOR_LE(0));
																					// xor     i0,i0,wordxor
						UML_LOAD(block, I0, fastbase, I0, SIZE_WORD, SCALE_x1);     // load    i0,fastbase,i0,word_x1
					}
					else if (size == 4)
					{
						UML_LOAD(block, I0, fastbase, I0, SIZE_DWORD, SCALE_x1);    // load    i0,fastbase,i0,dword_x1
					}
					else if (size == 8)
					{
						UML_DLOAD(block, I0, fastbase, I0, SIZE_QWORD, SCALE_x1);   // dload   i0,fastbase,i0,qword_x1
						UML_DROR(block, I0, I0, 32 * (m_bigendian ? BYTE_XOR_BE(0) : BYTE_XOR_LE(0)));
																					// dror    i0,i0,32*bytexor
					}
					UML_RET(block);                                                 // ret
				}
				else
				{
					if (size == 1)
					{
						UML_XOR(block, I0, I0, m_bigendian ? BYTE4_XOR_BE(0) : BYTE4_XOR_LE(0));
																					// xor     i0,i0,bytexor
						UML_STORE(block, fastbase, I0, I1, SIZE_BYTE, SCALE_x1);    // store   fastbase,i0,i1,byte
					}
					else if (size == 2)
					{
						UML_XOR(block, I0, I0, m_bigendian ? WORD_XOR_BE(0) : WORD_XOR_LE(0));
																					// xor     i0,i0,wordxor
						UML_STORE(block, fastbase, I0, I1, SIZE_WORD, SCALE_x1);    // store   fastbase,i0,i1,word_x1
					}
					else if (size == 4)
					{
						if (ismasked)
						{
							UML_LOAD(block, I3, fastbase, I0, SIZE_DWORD, SCALE_x1);    // load    i3,fastbase,i0,dword_x1
							UML_ROLINS(block, I3, I1, 0, I2);                           // rolins  i3,i1,0,i2
							UML_STORE(block, fastbase, I0, I3, SIZE_DWORD, SCALE_x1);   // store   fastbase,i0,i3,dword_x1
						}
						else
							UML_STORE(block, fastbase, I0, I1, SIZE_DWORD, SCALE_x1);   // store   fastbase,i0,i1,dword_x1
					}
					else if (size == 8)
					{
						UML_DROR(block, I1, I1, 32 * (m_bigendian ? BYTE_XOR_BE(0) : BYTE_XOR_LE(0)));
																						// dror    i1,i1,32*bytexor
						if (ismasked)
						{
							UML_DROR(block, I2, I2, 32 * (m_bigendian ? BYTE_XOR_BE(0) : BYTE_XOR_LE(0)));
																						// dror    i2,i2,32*bytexor
							UML_DLOAD(block, I3, fastbase, I0, SIZE_QWORD, SCALE_x1);   // dload   i3,fastbase,i0,qword_x1
							UML_DROLINS(block, I3, I1, 0, I2);                          // drolins i3,i1,0,i2
							UML_DSTORE(block, fastbase, I0, I3, SIZE_QWORD, SCALE_x1);  // dstore  fastbase,i0,i3,qword_x1
						}
						else
							UML_DSTORE(block, fastbase, I0, I1, SIZE_QWORD, SCALE_x1);  // dstore  fastbase,i0,i1,qword_x1
					}
					UML_RET(block);                                                     // ret
				}

				UML_LABEL(block, skip);                                                 // skip:
			}
		}
	}
}


/*------------------------------------------------------------------
    static_generate_memory_rw
------------------------------------------------------------------*/

void mips3_device::static_generate_memory_rw(drcuml_block &block, int size, bool iswrite, bool ismasked)
{
	switch (size)
	{
		case 1:
			if (iswrite)
				UML_WRITE(block, I0, I1, SIZE_BYTE, SPACE_PROGRAM);                 // write   i0,i1,program_byte
			else
				UML_READ(block, I0, I0, SIZE_BYTE, SPACE_PROGRAM);                  // read    i0,i0,program_byte
			break;

		case 2:
			if (iswrite)
				UML_WRITE(block, I0, I1, SIZE_WORD, SPACE_PROGRAM);                 // write   i0,i1,program_word
			else
				UML_READ(block, I0, I0, SIZE_WORD, SPACE_PROGRAM);                  // read    i0,i0,program_word
			break;

		case 4:
			if (iswrite)
			{
				if (!ismasked)
					UML_WRITE(block, I0, I1, SIZE_DWORD, SPACE_PROGRAM);            // write   i0,i1,program_dword
				else
					UML_WRITEM(block, I0, I1, I2, SIZE_DWORD, SPACE_PROGRAM);       // writem  i0,i1,i2,program_dword
			}
			else
			{
				if (!ismasked)
					UML_READ(block, I0, I0, SIZE_DWORD, SPACE_PROGRAM);             // read    i0,i0,program_dword
				else
					UML_READM(block, I0, I0, I2, SIZE_DWORD, SPACE_PROGRAM);        // readm   i0,i0,i2,program_dword
			}
			break;

		case 8:
			if (iswrite)
			{
				if (!ismasked)
					UML_DWRITE(block, I0, I1, SIZE_QWORD, SPACE_PROGRAM);           // dwrite  i0,i1,program_qword
				else
					UML_DWRITEM(block, I0, I1, I2, SIZE_QWORD, SPACE_PROGRAM);      // dwritem i0,i1,i2,program_qword
			}
			else
			{
				if (!ismasked)
					UML_DREAD(block, I0, I0, SIZE_QWORD, SPACE_PROGRAM);            // dread   i0,i0,program_qword
				else
					UML_DREADM(block, I0, I0, I2, SIZE_QWORD, SPACE_PROGRAM);       // dreadm  i0,i0,i2,program_qword
			}
			break;
	}
	UML_RET(block);                                                                 // ret
}


/*------------------------------------------------------------------
    static_generate_memory_accessor
------------------------------------------------------------------*/

void mips3_device::static_generate_memory_accessor(drcuml_block &block, int &label, int mode, int size, bool iswrite, bool ismasked, const char *name, uml::code_handle *&handleptr)
{
	/* on entry, address is in I0; data for writes is in I1; mask for accesses is in I2 */
	/* on exit, read result is in I0 */
	/* routine trashes I0-I3 */
	uml::code_handle &exception_tlb = *m_exception[iswrite ? EXCEPTION_TLBSTORE : EXCEPTION_TLBLOAD];
	uml::code_handle &exception_tlbfill = *m_exception[iswrite ? EXCEPTION_TLBSTORE_FILL : EXCEPTION_TLBLOAD_FILL];
	uml::code_handle &exception_addrerr = *m_exception[iswrite ? EXCEPTION_ADDRSTORE : EXCEPTION_ADDRLOAD];
	int tlbmiss = 0;

	/* add a global entry for this */
	alloc_handle(*m_drcuml, handleptr, name);
	UML_HANDLE(block, *handleptr);                                                  // handle  handleptr

	static_generate_memory_mode_checks(block, exception_addrerr, label, mode);

	/* general case: assume paging and perform a translation */
	UML_SHR(block, I3, I0, 12);                                                     // shr     i3,i0,12
	UML_LOAD(block, I3, (void *)vtlb_table(), I3, SIZE_DWORD, SCALE_x4);            // load    i3,[vtlb_table],i3,dword
	UML_TEST(block, I3, iswrite ? WRITE_ALLOWED : READ_ALLOWED);                    // test    i3,iswrite ? WRITE_ALLOWED : READ_ALLOWED
	UML_JMPc(block, COND_Z, tlbmiss = label++);                                     // jmp     tlbmiss,z
	UML_ROLINS(block, I0, I3, 0, 0xfffff000);                                       // rolins  i0,i3,0,0xfffff000

	static_generate_fastram_accessor(block, label, size, iswrite, ismasked);
	static_generate_memory_rw(block, size, iswrite, ismasked);

	UML_LABEL(block, tlbmiss);                                                      // tlbmiss:
	if (iswrite)
	{
		UML_TEST(block, I3, READ_ALLOWED);                                          // test    i3,READ_ALLOWED
		UML_EXHc(block, COND_NZ, *m_exception[EXCEPTION_TLBMOD], I0);               // exh     tlbmod,i0,nz
	}
	UML_TEST(block, I3, FLAG_FIXED);                                                // test    i3,FLAG_FIXED
	UML_EXHc(block, COND_NZ, exception_tlb, I0);                                    // exh     tlb,i0,nz
	UML_EXH(block, exception_tlbfill, I0);                                          // exh     tlbfill,i0
}

void r4650_device::static_generate_memory_accessor(drcuml_block &block, int &label, int mode, int size, bool iswrite, bool ismasked, const char *name, uml::code_handle *&handleptr)
{
	/* on entry, address is in I0; data for writes is in I1; mask for accesses is in I2 */
	/* on exit, read result is in I0 */
	/* routine trashes I0-I3 */
	uml::code_handle &exception_addrerr = *m_exception[iswrite ? EXCEPTION_ADDRSTORE : EXCEPTION_ADDRLOAD];

	/* add a global entry for this */
	alloc_handle(*m_drcuml, handleptr, name);
	UML_HANDLE(block, *handleptr);                                                  // handle  handleptr

	static_generate_memory_mode_checks(block, exception_addrerr, label, mode);

	if (mode == MODE_USER)
	{
		int addrok;
		UML_CMP(block, I0, CPR032(COP0_R4650_DBound));                              // cmp     i0,CPR0[DBound]
		UML_JMPc(block, COND_LE, addrok = label++);                                 // jle     addrok
		UML_EXHc(block, COND_G, exception_addrerr, I0);                             // exh     addrerr,i0,ne
		UML_LABEL(block, addrok);                                                   // addrok:
		UML_ADD(block, I0, I0, CPR032(COP0_R4650_DBase));                           // add     i0,i0,CPR0[DBase]
	}

	static_generate_fastram_accessor(block, label, size, iswrite, ismasked);
	static_generate_memory_rw(block, size, iswrite, ismasked);
}


/***************************************************************************
    CODE GENERATION
***************************************************************************/

/*-------------------------------------------------
    generate_update_mode - update the mode based
    on a new SR (in i0); trashes i2
-------------------------------------------------*/

void mips3_device::generate_update_mode(drcuml_block &block)
{
	UML_ROLAND(block, I2, I0, 32-2, 0x06);                      // roland  i2,i0,32-2,0x06
	UML_TEST(block, I0, SR_EXL | SR_ERL);                                   // test    i0,SR_EXL | SR_ERL
	UML_MOVc(block, COND_NZ, I2, 0);                                        // mov     i2,0,nz
	UML_ROLINS(block, I2, I0, 32-26, 0x01);                     // rolins  i2,i0,32-26,0x01
	UML_MOV(block, mem(&m_core->mode), I2);                            // mov     [mode],i2
}


/*-------------------------------------------------
    generate_update_cycles - generate code to
    subtract cycles from the icount and generate
    an exception if out
-------------------------------------------------*/

void mips3_device::generate_update_cycles(drcuml_block &block, compiler_state &compiler, uml::parameter param, bool allow_exception)
{
	/* check software interrupts if pending */
	if (compiler.checksoftints)
	{
		uml::code_label skip;

		compiler.checksoftints = false;
		UML_AND(block, I0, CPR032(COP0_Cause), CPR032(COP0_Status));            // and     i0,[Cause],[Status]
		UML_AND(block, I0, I0, 0x0300);                                         // and     i0,i0,0x0300
		UML_JMPc(block, COND_Z, skip = compiler.labelnum++);                    // jmp     skip,Z
		UML_MOV(block, I0, param);                                              // mov     i0,nextpc
		UML_MOV(block, I1, compiler.cycles);                                    // mov     i1,cycles
		UML_CALLH(block, *m_exception_norecover[EXCEPTION_INTERRUPT]);          // callh   interrupt_norecover
		UML_LABEL(block, skip);                                                 // skip:
	}

	/* check full interrupts if pending */
	if (compiler.checkints)
	{
		uml::code_label skip;

		compiler.checkints = false;
		UML_AND(block, I0, CPR032(COP0_Cause), CPR032(COP0_Status));            // and     i0,[Cause],[Status]
		UML_AND(block, I0, I0, 0xfc00);                                         // and     i0,i0,0xfc00
		UML_JMPc(block, COND_Z, skip = compiler.labelnum++);                    // jmp     skip,Z
		UML_TEST(block, CPR032(COP0_Status), SR_IE);                            // test    [Status],SR_IE
		UML_JMPc(block, COND_Z, skip);                                          // jmp     skip,Z
		UML_TEST(block, CPR032(COP0_Status), SR_EXL | SR_ERL);                  // test    [Status],SR_EXL | SR_ERL
		UML_JMPc(block, COND_NZ, skip);                                         // jmp     skip,NZ
		UML_MOV(block, I0, param);                                              // mov     i0,nextpc
		UML_MOV(block, I1, compiler.cycles);                                    // mov     i1,cycles
		UML_CALLH(block, *m_exception_norecover[EXCEPTION_INTERRUPT]);          // callh   interrupt_norecover
		UML_LABEL(block, skip);                                                 // skip:
	}

	/* account for cycles */
	if (compiler.cycles > 0)
	{
		UML_SUB(block, mem(&m_core->icount), mem(&m_core->icount), MAPVAR_CYCLES);    // sub     icount,icount,cycles
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);                                        // mapvar  cycles,0
		if (allow_exception)
			UML_EXHc(block, COND_S, *m_out_of_cycles, param);
																					// exh     out_of_cycles,nextpc
	}
	compiler.cycles = 0;
}

/*-------------------------------------------------
    generate_checksum_block - generate code to
    validate a sequence of opcodes
-------------------------------------------------*/

void mips3_device::generate_checksum_block(drcuml_block &block, compiler_state &compiler, const opcode_desc *seqhead, const opcode_desc *seqlast, const opcode_desc *codelast)
{
	const opcode_desc *curdesc;
	if (m_drcuml->logging())
		block.append_comment("[Validation for %08X]", seqhead->pc);                // comment

	/* loose verify or single instruction: just compare and fail */
	if (!(m_drcoptions & MIPS3DRC_STRICT_VERIFY) || seqhead->next() == nullptr)
	{
		if (!(seqhead->flags & OPFLAG_VIRTUAL_NOOP))
		{
			uint32_t sum = seqhead->opptr.l[0];
			const void *base = m_prptr(seqhead->physpc);
			uint32_t low_bits = (seqhead->physpc & (m_data_bits == 64 ? 4 : 0)) ^ m_dword_xor;
			UML_LOAD(block, I0, base, low_bits, SIZE_DWORD, SCALE_x1);         // load    i0,base,0,dword

			if (seqhead->delay.first() != nullptr
				&& !(seqhead->delay.first()->flags & OPFLAG_VIRTUAL_NOOP)
				&& seqhead->physpc != seqhead->delay.first()->physpc)
			{
				uint32_t low_bits = (seqhead->delay.first()->physpc & (m_data_bits == 64 ? 4 : 0)) ^ m_dword_xor;
				base = m_prptr(seqhead->delay.first()->physpc);
				assert(base != nullptr);
				UML_LOAD(block, I1, base, low_bits, SIZE_DWORD, SCALE_x1);                 // load    i1,base,dword
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
				const void *base = m_prptr(seqhead->physpc);
				UML_LOAD(block, I0, base, m_dword_xor, SIZE_DWORD, SCALE_x1);     // load    i0,base,0,dword
				UML_CMP(block, I0, curdesc->opptr.l[0]);                    // cmp     i0,opptr[0]
				UML_EXHc(block, COND_NE, *m_nocode, epc(seqhead));   // exne    nocode,seqhead->pc
			}
#else
		uint32_t sum = 0;
		const void *base = m_prptr(seqhead->physpc);
		const uint32_t data_bits_mask = (m_data_bits == 64 ? 4 : 0);
		const uint32_t last_physpc = codelast->physpc;
		uint32_t low_bits = (seqhead->physpc & data_bits_mask) ^ m_dword_xor;
		UML_LOAD(block, I0, base, low_bits, SIZE_DWORD, SCALE_x1);             // load    i0,base,0,dword
		sum += seqhead->opptr.l[0];
		if ((m_drcoptions & MIPS3DRC_EXTRA_INSTR_CHECK) && !(codelast->flags & OPFLAG_VIRTUAL_NOOP) && last_physpc != seqhead->physpc)
		{
			base = m_prptr(last_physpc);
			assert(base != nullptr);
			low_bits = (last_physpc & data_bits_mask) ^ m_dword_xor;
			UML_LOAD(block, I1, base, low_bits, SIZE_DWORD, SCALE_x1);     // load    i1,base,dword
			UML_ADD(block, I0, I0, I1);                         // add     i0,i0,i1
			sum += codelast->opptr.l[0];
		}
		if (!(m_drcoptions & MIPS3DRC_EXTRA_INSTR_CHECK))
		{
			for (curdesc = seqhead->next(); curdesc != seqlast->next(); curdesc = curdesc->next())
			{
				if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
				{
					base = m_prptr(curdesc->physpc);
					assert(base != nullptr);
					low_bits = (curdesc->physpc & data_bits_mask) ^ m_dword_xor;
					UML_LOAD(block, I1, base, low_bits, SIZE_DWORD, SCALE_x1);     // load    i1,base,dword
					UML_ADD(block, I0, I0, I1);                         // add     i0,i0,i1
					sum += curdesc->opptr.l[0];

					if (curdesc->delay.first() != nullptr
						&& !(curdesc->delay.first()->flags & OPFLAG_VIRTUAL_NOOP)
						&& (curdesc == seqlast || (curdesc->next() != nullptr && curdesc->next()->physpc != curdesc->delay.first()->physpc)))
					{
						base = m_prptr(curdesc->delay.first()->physpc);
						assert(base != nullptr);
						low_bits = (curdesc->delay.first()->physpc & data_bits_mask) ^ m_dword_xor;
						UML_LOAD(block, I1, base, low_bits, SIZE_DWORD, SCALE_x1); // load    i1,base,dword
						UML_ADD(block, I0, I0, I1);                     // add     i0,i0,i1
						sum += curdesc->delay.first()->opptr.l[0];
					}
				}
			}
		}
		else
		{
			for (curdesc = seqhead->next(); curdesc != seqlast->next(); curdesc = curdesc->next())
			{
				if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
				{
					// Skip the last if it was already included above
					if (curdesc->physpc != last_physpc)
					{
						base = m_prptr(curdesc->physpc);
						assert(base != nullptr);
						low_bits = (curdesc->physpc & data_bits_mask) ^ m_dword_xor;
						UML_LOAD(block, I1, base, low_bits, SIZE_DWORD, SCALE_x1);     // load    i1,base,dword
						UML_ADD(block, I0, I0, I1);                         // add     i0,i0,i1
						sum += curdesc->opptr.l[0];
					}

					if (curdesc->delay.first() != nullptr
						&& !(curdesc->delay.first()->flags & OPFLAG_VIRTUAL_NOOP)
						&& (curdesc == seqlast || (curdesc->next() != nullptr && curdesc->next()->physpc != curdesc->delay.first()->physpc)))
					{
						base = m_prptr(curdesc->delay.first()->physpc);
						assert(base != nullptr);
						low_bits = (curdesc->delay.first()->physpc & data_bits_mask) ^ m_dword_xor;
						UML_LOAD(block, I1, base, low_bits, SIZE_DWORD, SCALE_x1); // load    i1,base,dword
						UML_ADD(block, I0, I0, I1);                     // add     i0,i0,i1
						sum += curdesc->delay.first()->opptr.l[0];
					}
				}
			}
		}
		UML_CMP(block, I0, sum);                                            // cmp     i0,sum
		UML_EXHc(block, COND_NE, *m_nocode, epc(seqhead));           // exne    nocode,seqhead->pc
		if (DEBUG_STRICT_VERIFY)
		{
			// This code will do additional checks on the last instruction and last delay slot and indicate if the check failed
			uml::code_label check_second = compiler.labelnum++;
			uml::code_label check_failed = compiler.labelnum++;
			uml::code_label check_passed = compiler.labelnum++;
			// Check the last instruction
			if (!(codelast->flags & OPFLAG_VIRTUAL_NOOP) && last_physpc != seqhead->physpc)
			{
				base = m_prptr(last_physpc);
				assert(base != nullptr);
				low_bits = (last_physpc & (m_data_bits == 64 ? 4 : 0)) ^ m_dword_xor;
				UML_LOAD(block, I0, base, low_bits, SIZE_DWORD, SCALE_x1);     // load    i1,base,dword
				sum = codelast->opptr.l[0];
				UML_CMP(block, I0, sum);                                            // cmp     i0,sum
				UML_JMPc(block, COND_E, check_second);
				static const char text[] = "Last instr validation fail seq: %08X end: %08x\n";
				UML_DMOV(block, mem(&m_core->format), (uintptr_t)text);          // mov     [format],text
				UML_MOV(block, mem(&m_core->arg0), seqhead->pc);
				UML_MOV(block, mem(&m_core->arg1), last_physpc);              // mov     [arg0],desc->pc
				UML_CALLC(block, cfunc_printf_debug, this);                            // callc   printf_debug
				//UML_CALLC(block, cfunc_debug_break, this);
				// Skip delay slot check
				UML_JMP(block, check_failed);
				// Check the last instruction delay slot
				UML_LABEL(block, check_second);
				if (codelast->delay.first() != nullptr && !(codelast->delay.first()->flags & OPFLAG_VIRTUAL_NOOP)
					&& last_physpc != seqhead->physpc)
				{
					base = m_prptr(codelast->delay.first()->physpc);
					assert(base != nullptr);
					low_bits = (codelast->delay.first()->physpc & (m_data_bits == 64 ? 4 : 0)) ^ m_dword_xor;
					UML_LOAD(block, I0, base, low_bits, SIZE_DWORD, SCALE_x1); // load    i1,base,dword
					sum = codelast->delay.first()->opptr.l[0];
					UML_CMP(block, I0, sum);                                            // cmp     i0,sum
					UML_JMPc(block, COND_E, check_passed);
					static const char text[] = "Last delay slot validation fail seq: %08X end: %08x\n";
					UML_DMOV(block, mem(&m_core->format), (uintptr_t)text);          // mov     [format],text
					UML_MOV(block, mem(&m_core->arg0), seqhead->pc);
					UML_MOV(block, mem(&m_core->arg1), last_physpc);              // mov     [arg0],desc->pc
					UML_CALLC(block, cfunc_printf_debug, this);                            // callc   printf_debug
					//UML_CALLC(block, cfunc_debug_break, this);
					UML_JMP(block, check_failed);
				}
			}
			UML_JMP(block, check_passed);
			UML_LABEL(block, check_failed);
			UML_EXH(block, *m_nocode, epc(seqhead));           // exne    nocode,seqhead->pc
			UML_LABEL(block, check_passed);
		}
#endif
	}
}


/*-------------------------------------------------
    generate_sequence_instruction - generate code
    for a single instruction in a sequence
-------------------------------------------------*/

void mips3_device::generate_sequence_instruction(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	/* add an entry for the log */
	if (m_drcuml->logging() && !(desc->flags & OPFLAG_VIRTUAL_NOOP))
		log_add_disasm_comment(block, desc->pc, desc->opptr.l[0]);

	/* set the PC map variable */
	offs_t expc = (desc->flags & OPFLAG_IN_DELAY_SLOT) ? desc->pc - 3 : desc->pc;
	UML_MAPVAR(block, MAPVAR_PC, expc);                                             // mapvar  PC,expc

	/* accumulate total cycles */
	compiler.cycles += desc->cycles;

	/* is this a hotspot? */
	for (int hotnum = 0; hotnum < MIPS3_MAX_HOTSPOTS; hotnum++)
		if (m_hotspot[hotnum].pc != 0 && desc->pc == m_hotspot[hotnum].pc && desc->opptr.l[0] == m_hotspot[hotnum].opcode)
		{
			compiler.cycles += m_hotspot[hotnum].cycles;
			break;
		}

	/* update the icount map variable */
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler.cycles);                             // mapvar  CYCLES,compiler.cycles

	/* if we want a probe, add it here */
	if (desc->pc == PROBE_ADDRESS)
	{
		UML_MOV(block, mem(&m_core->pc), desc->pc);                              // mov     [pc],desc->pc
		UML_CALLC(block, cfunc_printf_probe, this);                                // callc   cfunc_printf_probe,mips3
	}

	/* if we are debugging, call the debugger */
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		UML_MOV(block, mem(&m_core->pc), desc->pc);                              // mov     [pc],desc->pc
		save_fast_iregs(block);
		UML_DEBUG(block, desc->pc);                                         // debug   desc->pc
	}

	/* if we hit an unmapped address, fatal error */
	if (desc->flags & OPFLAG_COMPILER_UNMAPPED)
	{
		UML_MOV(block, mem(&m_core->pc), desc->pc);                              // mov     [pc],desc->pc
		save_fast_iregs(block);
		UML_EXIT(block, EXECUTE_UNMAPPED_CODE);                             // exit    EXECUTE_UNMAPPED_CODE
	}

	/* if we hit a compiler page fault, it's just like a TLB mismatch */
	if (desc->flags & OPFLAG_COMPILER_PAGE_FAULT)
	{
		if (PRINTF_MMU)
		{
			static const char text[] = "Compiler page fault @ %08X\n";
			UML_MOV(block, mem(&m_core->format), (uintptr_t)text);          // mov     [format],text
			UML_MOV(block, mem(&m_core->arg0), desc->pc);              // mov     [arg0],desc->pc
			UML_CALLC(block, cfunc_printf_debug, this);                            // callc   printf_debug
		}
		UML_EXH(block, *m_tlb_mismatch, 0);                      // exh     tlb_mismatch,0
		// Unconditional tlb exception, no point going further
		return;
	}

	/* validate our TLB entry at this PC; if we fail, we need to handle it */
	if ((desc->flags & OPFLAG_VALIDATE_TLB) && (desc->pc < 0x80000000 || desc->pc >= 0xc0000000))
	{
		const vtlb_entry *tlbtable = vtlb_table();

		/* if we currently have a valid TLB read entry, we just verify */
		if (tlbtable[desc->pc >> 12] & FETCH_ALLOWED)
		{
			if (PRINTF_MMU)
			{
				static const char text[] = "Checking TLB at @ %08X\n";
				UML_MOV(block, mem(&m_core->format), (uintptr_t)text);      // mov     [format],text
				UML_MOV(block, mem(&m_core->arg0), desc->pc);          // mov     [arg0],desc->pc
				UML_CALLC(block, cfunc_printf_debug, this);                        // callc   printf_debug
			}
			UML_LOAD(block, I0, &tlbtable[desc->pc >> 12], 0, SIZE_DWORD, SCALE_x4);        // load i0,tlbtable[desc->pc >> 12],0,dword
			UML_CMP(block, I0, tlbtable[desc->pc >> 12]);                   // cmp     i0,*tlbentry
			UML_EXHc(block, COND_NE, *m_tlb_mismatch, 0);            // exh     tlb_mismatch,0,NE
		}

		/* otherwise, we generate an unconditional exception */
		else
		{
			if (PRINTF_MMU)
			{
				static const char text[] = "No valid TLB @ %08X\n";
				UML_MOV(block, mem(&m_core->format), (uintptr_t)text);      // mov     [format],text
				UML_MOV(block, mem(&m_core->arg0), desc->pc);          // mov     [arg0],desc->pc
				UML_CALLC(block, cfunc_printf_debug, this);                        // callc   printf_debug
			}
			UML_EXH(block, *m_tlb_mismatch, 0);                  // exh     tlb_mismatch,0
			// Unconditional tlb exception, no point going further
			return;
		}
	}

	/* if this is an invalid opcode, generate the exception now */
	if (desc->flags & OPFLAG_INVALID_OPCODE)
		UML_EXH(block, *m_exception[EXCEPTION_INVALIDOP], 0);    // exh     invalidop,0

	/* otherwise, unless this is a virtual no-op, it's a regular instruction */
	else if (!(desc->flags & OPFLAG_VIRTUAL_NOOP))
	{
		/* compile the instruction */
		if (!generate_opcode(block, compiler, desc))
		{
			UML_MOV(block, mem(&m_core->pc), desc->pc);                          // mov     [pc],desc->pc
			UML_MOV(block, mem(&m_core->arg0), desc->opptr.l[0]);      // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_unimplemented, this);                           // callc   cfunc_unimplemented
		}
	}
}


/*------------------------------------------------------------------
    generate_delay_slot_and_branch
------------------------------------------------------------------*/

void mips3_device::generate_delay_slot_and_branch(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint8_t linkreg)
{
	compiler_state compiler_temp(compiler);
	uint32_t op = desc->opptr.l[0];

	/* fetch the target register if dynamic, in case it is modified by the delay slot */
	if (desc->targetpc == BRANCH_TARGET_DYNAMIC)
	{
		UML_MOV(block, mem(&m_core->jmpdest), R32(RSREG));                 // mov     [jmpdest],<rsreg>

	}

	/* set the link if needed -- before the delay slot */
	if (linkreg != 0)
	{
		UML_DMOV(block, R64(linkreg), (int32_t)(desc->pc + 8));                   // dmov    <linkreg>,desc->pc + 8
	}

	/* compile the delay slot using temporary compiler state */
	assert(desc->delay.first() != nullptr);
	generate_sequence_instruction(block, compiler_temp, desc->delay.first());       // <next instruction>

	/* update the cycles and jump through the hash table to the target */
	if (desc->targetpc != BRANCH_TARGET_DYNAMIC)
	{
		generate_update_cycles(block, compiler_temp, desc->targetpc, true); // <subtract cycles>
		if (!(m_drcoptions & MIPS3DRC_DISABLE_INTRABLOCK) && (desc->flags & OPFLAG_INTRABLOCK_BRANCH))
		{
			UML_JMP(block, desc->targetpc | 0x80000000);                            // jmp     desc->targetpc | 0x80000000
		}
		else
		{
			UML_HASHJMP(block, m_core->mode, desc->targetpc, *m_nocode);            // hashjmp <mode>,desc->targetpc,nocode
		}
	}
	else
	{
		generate_update_cycles(block, compiler_temp, uml::mem(&m_core->jmpdest), true); // <subtract cycles>
		UML_HASHJMP(block, m_core->mode, mem(&m_core->jmpdest), *m_nocode);         // hashjmp <mode>,<rsreg>,nocode
	}

	/* update the label */
	compiler.labelnum = compiler_temp.labelnum;

	/* reset the mapvar to the current cycles and account for skipped slots */
	compiler.cycles += desc->skipslots;
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler.cycles);                             // mapvar  CYCLES,compiler.cycles
}


/*-------------------------------------------------
    generate_opcode - generate code for a specific
    opcode
-------------------------------------------------*/

bool mips3_device::generate_opcode(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	int in_delay_slot = ((desc->flags & OPFLAG_IN_DELAY_SLOT) != 0);
	uint32_t op = desc->opptr.l[0];
	uint8_t opswitch = op >> 26;
	uml::code_label skip;

	switch (opswitch)
	{
		/* ----- sub-groups ----- */

		case 0x00:  /* SPECIAL - MIPS I */
			return generate_special(block, compiler, desc);

		case 0x01:  /* REGIMM - MIPS I */
			return generate_regimm(block, compiler, desc);

		case 0x1c:  /* IDT-specific */
			return generate_idt(block, compiler, desc);


		/* ----- jumps and branches ----- */

		case 0x02:  /* J - MIPS I */
			generate_delay_slot_and_branch(block, compiler, desc, 0);        // <next instruction + hashjmp>
			return true;

		case 0x03:  /* JAL - MIPS I */
			generate_delay_slot_and_branch(block, compiler, desc, 31);       // <next instruction + hashjmp>
			return true;

		case 0x04:  /* BEQ - MIPS I */
		case 0x14:  /* BEQL - MIPS II */
			UML_DCMP(block, R64(RSREG), R64(RTREG));                                // dcmp    <rsreg>,<rtreg>
			UML_JMPc(block, COND_NE, skip = compiler.labelnum++);                  // jmp     skip,NE
			generate_delay_slot_and_branch(block, compiler, desc, 0);        // <next instruction + hashjmp>
			UML_LABEL(block, skip);                                             // skip:
			return true;

		case 0x05:  /* BNE - MIPS I */
		case 0x15:  /* BNEL - MIPS II */
			UML_DCMP(block, R64(RSREG), R64(RTREG));                                // dcmp    <rsreg>,<rtreg>
			UML_JMPc(block, COND_E, skip = compiler.labelnum++);                       // jmp     skip,E
			generate_delay_slot_and_branch(block, compiler, desc, 0);        // <next instruction + hashjmp>
			UML_LABEL(block, skip);                                             // skip:
			return true;

		case 0x06:  /* BLEZ - MIPS I */
		case 0x16:  /* BLEZL - MIPS II */
			if (RSREG != 0)
			{
				UML_DCMP(block, R64(RSREG), 0);                             // dcmp    <rsreg>,0
				UML_JMPc(block, COND_G, skip = compiler.labelnum++);                   // jmp     skip,G
				generate_delay_slot_and_branch(block, compiler, desc, 0);    // <next instruction + hashjmp>
				UML_LABEL(block, skip);                                         // skip:
			}
			else
				generate_delay_slot_and_branch(block, compiler, desc, 0);    // <next instruction + hashjmp>
			return true;

		case 0x07:  /* BGTZ - MIPS I */
		case 0x17:  /* BGTZL - MIPS II */
			UML_DCMP(block, R64(RSREG), 0);                                 // dcmp    <rsreg>,0
			UML_JMPc(block, COND_LE, skip = compiler.labelnum++);                  // jmp     skip,LE
			generate_delay_slot_and_branch(block, compiler, desc, 0);        // <next instruction + hashjmp>
			UML_LABEL(block, skip);                                             // skip:
			return true;


		/* ----- immediate arithmetic ----- */

		case 0x0f:  /* LUI - MIPS I */
			if (RTREG != 0)
				UML_DMOV(block, R64(RTREG), UIMMVAL << 16);                 // dmov    <rtreg>,UIMMVAL << 16
			return true;

		case 0x08:  /* ADDI - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			if (m_drcoptions & MIPS3DRC_CHECK_OVERFLOWS)
				UML_EXHc(block, COND_V, *m_exception[EXCEPTION_OVERFLOW], 0);
																					// exh    overflow,0
			if (RTREG != 0)
				UML_DSEXT(block, R64(RTREG), I0, SIZE_DWORD);                       // dsext   <rtreg>,i0,dword
			return true;

		case 0x09:  /* ADDIU - MIPS I */
			if (RTREG != 0)
			{
				UML_ADD(block, I0, R32(RSREG), SIMMVAL);                    // add     i0,<rsreg>,SIMMVAL,V
				UML_DSEXT(block, R64(RTREG), I0, SIZE_DWORD);                       // dsext   <rtreg>,i0,dword
			}
			return true;

		case 0x18:  /* DADDI - MIPS III */
			UML_DADD(block, I0, R64(RSREG), SIMMVAL);                       // dadd    i0,<rsreg>,SIMMVAL
			if (m_drcoptions & MIPS3DRC_CHECK_OVERFLOWS)
				UML_EXHc(block, COND_V, *m_exception[EXCEPTION_OVERFLOW], 0);
																					// exh    overflow,0
			if (RTREG != 0)
				UML_DMOV(block, R64(RTREG), I0);                                // dmov    <rtreg>,i0
			return true;

		case 0x19:  /* DADDIU - MIPS III */
			if (RTREG != 0)
				UML_DADD(block, R64(RTREG), R64(RSREG), SIMMVAL);               // dadd    <rtreg>,<rsreg>,SIMMVAL
			return true;

		case 0x0c:  /* ANDI - MIPS I */
			if (RTREG != 0)
				UML_DAND(block, R64(RTREG), R64(RSREG), UIMMVAL);               // dand    <rtreg>,<rsreg>,UIMMVAL
			return true;

		case 0x0d:  /* ORI - MIPS I */
			if (RTREG != 0)
				UML_DOR(block, R64(RTREG), R64(RSREG), UIMMVAL);                // dor     <rtreg>,<rsreg>,UIMMVAL
			return true;

		case 0x0e:  /* XORI - MIPS I */
			if (RTREG != 0)
				UML_DXOR(block, R64(RTREG), R64(RSREG), UIMMVAL);               // dxor    <rtreg>,<rsreg>,UIMMVAL
			return true;

		case 0x0a:  /* SLTI - MIPS I */
			if (RTREG != 0)
			{
				UML_DCMP(block, R64(RSREG), SIMMVAL);                           // dcmp    <rsreg>,SIMMVAL
				UML_DSETc(block, COND_L, R64(RTREG));                                   // dset    <rtreg>,l
			}
			return true;

		case 0x0b:  /* SLTIU - MIPS I */
			if (RTREG != 0)
			{
				UML_DCMP(block, R64(RSREG), SIMMVAL);                           // dcmp    <rsreg>,SIMMVAL
				UML_DSETc(block, COND_B, R64(RTREG));                                   // dset    <rtreg>,b
			}
			return true;


		/* ----- memory load operations ----- */

		case 0x20:  /* LB - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read8[m_core->mode >> 1]);  // callh   read8
			if (RTREG != 0)
				UML_DSEXT(block, R64(RTREG), I0, SIZE_BYTE);                        // dsext   <rtreg>,i0,byte
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x21:  /* LH - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read16[m_core->mode >> 1]); // callh   read16
			if (RTREG != 0)
				UML_DSEXT(block, R64(RTREG), I0, SIZE_WORD);                        // dsext   <rtreg>,i0,word
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x23:  /* LW - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read32[m_core->mode >> 1]); // callh   read32
			if (RTREG != 0)
				UML_DSEXT(block, R64(RTREG), I0, SIZE_DWORD);                       // dsext   <rtreg>,i0
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x30:  /* LL - MIPS II */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, mem(&m_core->cpr[0][COP0_LLAddr]), I0);          // mov     [LLAddr],i0
			UML_CALLH(block, *m_read32[m_core->mode >> 1]);                 // callh   read32
			if (RTREG != 0)
				UML_DSEXT(block, R64(RTREG), I0, SIZE_DWORD);               // dsext   <rtreg>,i0
			UML_MOV(block, mem(&m_core->llbit), 1);                         // mov     [llbit],1
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			if LL_BREAK
				UML_CALLC(block, cfunc_debug_break, this);                  // callc   cfunc_debug_break,nullptr
			return true;

		case 0x24:  /* LBU - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read8[m_core->mode >> 1]);  // callh   read8
			if (RTREG != 0)
				UML_DAND(block, R64(RTREG), I0, 0xff);                  // dand    <rtreg>,i0,0xff
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x25:  /* LHU - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read16[m_core->mode >> 1]); // callh   read16
			if (RTREG != 0)
				UML_DAND(block, R64(RTREG), I0, 0xffff);                    // dand    <rtreg>,i0,0xffff
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x27:  /* LWU - MIPS III */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read32[m_core->mode >> 1]); // callh   read32
			if (RTREG != 0)
				UML_DAND(block, R64(RTREG), I0, 0xffffffff);                // dand    <rtreg>,i0,0xffffffff
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x37:  /* LD - MIPS III */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read64[m_core->mode >> 1]); // callh   read64
			if (RTREG != 0)
				UML_DMOV(block, R64(RTREG), I0);                                // dmov    <rtreg>,i0
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x34:  /* LLD - MIPS III */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, mem(&m_core->cpr[0][COP0_LLAddr]), I0);          // mov     [LLAddr],i0
			UML_CALLH(block, *m_read64[m_core->mode >> 1]);                 // callh   read64
			if (RTREG != 0)
				UML_DMOV(block, R64(RTREG), I0);                            // dmov    <rtreg>,i0
			UML_MOV(block, mem(&m_core->llbit), 1);                         // mov     [llbit],1
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			if LL_BREAK
				UML_CALLC(block, cfunc_debug_break, this);                  // callc   cfunc_debug_break,nullptr
			return true;

		case 0x22:  /* LWL - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_SHL(block, I1, I0, 3);                              // shl     i1,i0,3
			UML_AND(block, I0, I0, ~3);                             // and     i0,i0,~3
			if (!m_bigendian)
				UML_XOR(block, I1, I1, 0x18);                       // xor     i1,i1,0x18
			UML_SHR(block, I2, ~0, I1);                             // shr     i2,~0,i1
			UML_CALLH(block, *m_read32mask[m_core->mode >> 1]);
																					// callh   read32mask
			if (RTREG != 0)
			{
				UML_SHL(block, I2, ~0, I1);                         // shl     i2,~0,i1
				UML_MOV(block, I3, R32(RTREG));                             // mov     i3,<rtreg>
				UML_ROLINS(block, I3, I0, I1, I2);              // rolins  i3,i0,i1,i2
				UML_DSEXT(block, R64(RTREG), I3, SIZE_DWORD);                       // dsext   <rtreg>,i3,dword
			}
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x26:  /* LWR - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_SHL(block, I1, I0, 3);                              // shl     i1,i0,3
			UML_AND(block, I0, I0, ~3);                             // and     i0,i0,~3
			if (m_bigendian)
				UML_XOR(block, I1, I1, 0x18);                       // xor     i1,i1,0x18
			UML_SHL(block, I2, ~0, I1);                             // shl     i2,~0,i1
			UML_CALLH(block, *m_read32mask[m_core->mode >> 1]);
																					// callh   read32mask
			if (RTREG != 0)
			{
				UML_SHR(block, I2, ~0, I1);                         // shr     i2,~0,i1
				UML_SUB(block, I1, 32, I1);                         // sub     i1,32,i1
				UML_MOV(block, I3, R32(RTREG));                             // mov     i3,<rtreg>
				UML_ROLINS(block, I3, I0, I1, I2);              // rolins  i3,i0,i1,i2
				UML_DSEXT(block, R64(RTREG), I3, SIZE_DWORD);                       // dsext   <rtreg>,i3,dword
			}
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x1a:  /* LDL - MIPS III */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_SHL(block, I1, I0, 3);                              // shl     i1,i0,3
			UML_AND(block, I0, I0, ~7);                             // and     i0,i0,~7
			if (!m_bigendian)
				UML_XOR(block, I1, I1, 0x38);                       // xor     i1,i1,0x38
			UML_DSHR(block, I2, (uint64_t)~0, I1);                        // dshr    i2,~0,i1
			UML_CALLH(block, *m_read64mask[m_core->mode >> 1]);
																					// callh   read64mask
			if (RTREG != 0)
			{
				UML_DSHL(block, I2, (uint64_t)~0, I1);                    // dshl    i2,~0,i1
				UML_DROLINS(block, R64(RTREG), I0, I1, I2);         // drolins <rtreg>,i0,i1,i2
			}
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x1b:  /* LDR - MIPS III */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);            // add     i0,<rsreg>,SIMMVAL
			UML_SHL(block, I1, I0, 3);                          // shl     i1,i0,3
			UML_AND(block, I0, I0, ~7);                         // and     i0,i0,~7
			if (m_bigendian)
				UML_XOR(block, I1, I1, 0x38);                   // xor     i1,i1,0x38
			UML_DSHL(block, I2, (uint64_t)~0, I1);                // dshl    i2,~0,i1
			UML_CALLH(block, *m_read64mask[m_core->mode >> 1]); // callh   read64mask
			if (RTREG != 0)
			{
				UML_DSHR(block, I2, (uint64_t)~0, I1);            // dshr    i2,~0,i1
				UML_SUB(block, I1, 64, I1);                     // sub     i1,64,i1
				UML_DROLINS(block, R64(RTREG), I0, I1, I2);     // drolins <rtreg>,i0,i1,i2
			}
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x31:  /* LWC1 - MIPS I */
			check_cop1_access(block);
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read32[m_core->mode >> 1]); // callh   read32
			UML_MOV(block, FPR32(RTREG), I0);           // mov     <cpr1_rt>,i0
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x35:  /* LDC1 - MIPS III */
			check_cop1_access(block);
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read64[m_core->mode >> 1]); // callh   read64
			UML_DMOV(block, FPR64(RTREG), I0);              // dmov    <cpr1_rt>,i0
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x32:  /* LWC2 - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read32[m_core->mode >> 1]); // callh   read32
			UML_DAND(block, CPR264(RTREG), I0, 0xffffffff);             // dand    <cpr2_rt>,i0,0xffffffff
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x36:  /* LDC2 - MIPS II */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read64[m_core->mode >> 1]); // callh   read64
			UML_DMOV(block, CPR264(RTREG), I0);                             // dmov    <cpr2_rt>,i0
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;


		/* ----- memory store operations ----- */

		case 0x28:  /* SB - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, I1, R32(RTREG));                                 // mov     i1,<rtreg>
			UML_CALLH(block, *m_write8[m_core->mode >> 1]); // callh   write8
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x29:  /* SH - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, I1, R32(RTREG));                                 // mov     i1,<rtreg>
			UML_CALLH(block, *m_write16[m_core->mode >> 1]);    // callh   write16
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x2b:  /* SW - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, I1, R32(RTREG));                                 // mov     i1,<rtreg>
			UML_CALLH(block, *m_write32[m_core->mode >> 1]);    // callh   write32
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x38:  /* SC - MIPS II */
			UML_MOV(block, I1, R32(RTREG));                                 // mov     i1,<rtreg>
			UML_MOV(block, R32(RTREG), 0);                                  // mov     <rtreg>, 0
			UML_CMP(block, mem(&m_core->llbit), 0);                         // cmp     [llbit],0
			UML_JMPc(block, COND_E, skip = compiler.labelnum++);            // je      skip
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CMP(block, mem(&m_core->cpr[0][COP0_LLAddr]), I0);          // cmp [LLADDR], RSREG + SIMMVAL
			UML_JMPc(block, COND_NE, skip);                                 // jne      skip
			UML_CALLH(block, *m_write32[m_core->mode >> 1]);                // callh   write32
			UML_MOV(block, R32(RTREG), 1);                                  // mov     <rtreg>, 0
			UML_LABEL(block, skip);                                         // skip:
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x3f:  /* SD - MIPS III */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_DMOV(block, I1, R64(RTREG));                                // dmov    i1,<rtreg>
			UML_CALLH(block, *m_write64[m_core->mode >> 1]);    // callh   write64
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x3c:  /* SCD - MIPS III */
			UML_DMOV(block, I1, R64(RTREG));                                    // dmov    i1,<rtreg>
			UML_DMOV(block, R64(RTREG), 0);                                     // dmov   <rtreg>,0
			UML_CMP(block, mem(&m_core->llbit), 0);                             // cmp     [llbit],0
			UML_JMPc(block, COND_E, skip = compiler.labelnum++);                // je      skip
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                            // add     i0,<rsreg>,SIMMVAL
			UML_CMP(block, mem(&m_core->cpr[0][COP0_LLAddr]), I0);              // cmp [LLADDR], RSREG + SIMMVAL
			UML_JMPc(block, COND_NE, skip);                                     // jne      skip
			UML_CALLH(block, *m_write64[m_core->mode >> 1]);                    // callh   write64
			UML_DMOV(block, R64(RTREG), 1);                                     // dmov   <rtreg>,1
			UML_LABEL(block, skip);                                             // skip:
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x2a:  /* SWL - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_SHL(block, I3, I0, 3);                              // shl     i3,i0,3
			UML_AND(block, I0, I0, ~3);                             // and     i0,i0,~3
			UML_MOV(block, I1, R32(RTREG));                                 // mov     i1,<rtreg>
			if (!m_bigendian)
				UML_XOR(block, I3, I3, 0x18);                       // xor     i3,i3,0x18
			UML_SHR(block, I2, ~0, I3);                             // shr     i2,~0,i3
			UML_SHR(block, I1, I1, I3);                             // shr     i1,i1,i3
			UML_CALLH(block, *m_write32mask[m_core->mode >> 1]);
																					// callh   write32mask
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x2e:  /* SWR - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_SHL(block, I3, I0, 3);                              // shl     i3,i0,3
			UML_AND(block, I0, I0, ~3);                             // and     i0,i0,~3
			UML_MOV(block, I1, R32(RTREG));                                 // mov     i1,<rtreg>
			if (m_bigendian)
				UML_XOR(block, I3, I3, 0x18);                       // xor     i3,i3,0x18
			UML_SHL(block, I2, ~0, I3);                             // shl     i2,~0,i3
			UML_SHL(block, I1, I1, I3);                             // shl     i1,i1,i3
			UML_CALLH(block, *m_write32mask[m_core->mode >> 1]);
																					// callh   write32mask
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x2c:  /* SDL - MIPS III */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);            // add     i0,<rsreg>,SIMMVAL
			UML_SHL(block, I3, I0, 3);                          // shl     i3,i0,3
			UML_AND(block, I0, I0, ~7);                         // and     i0,i0,~7
			UML_DMOV(block, I1, R64(RTREG));                    // dmov    i1,<rtreg>
			if (!m_bigendian)
				UML_XOR(block, I3, I3, 0x38);                   // xor     i3,i3,0x38
			UML_DSHR(block, I2, (uint64_t)~0, I3);                // dshr    i2,~0,i3
			UML_DSHR(block, I1, I1, I3);                        // dshr    i1,i1,i3
			UML_CALLH(block, *m_write64mask[m_core->mode >> 1]);// callh   write64mask

			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x2d:  /* SDR - MIPS III */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);            // add     i0,<rsreg>,SIMMVAL
			UML_SHL(block, I3, I0, 3);                          // shl     i3,i0,3
			UML_AND(block, I0, I0, ~7);                         // and     i0,i0,~7
			UML_DMOV(block, I1, R64(RTREG));                    // dmov    i1,<rtreg>
			if (m_bigendian)
				UML_XOR(block, I3, I3, 0x38);                   // xor     i3,i3,0x38
			UML_DSHL(block, I2, (uint64_t)~0, I3);                // dshl    i2,~0,i3
			UML_DSHL(block, I1, I1, I3);                        // dshl    i1,i1,i3
			UML_CALLH(block, *m_write64mask[m_core->mode >> 1]);// callh   write64mask

			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x39:  /* SWC1 - MIPS I */
			check_cop1_access(block);
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);            // add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, I1, FPR32(RTREG));                   // mov     i1,<cpr1_rt>
			UML_CALLH(block, *m_write32[m_core->mode >> 1]);    // callh   write32
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x3d:  /* SDC1 - MIPS III */
			check_cop1_access(block);
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);            // add     i0,<rsreg>,SIMMVAL
			UML_DMOV(block, I1, FPR64(RTREG));                  // dmov    i1,<cpr1_rt>
			UML_CALLH(block, *m_write64[m_core->mode >> 1]);    // callh   write64
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x3a:  /* SWC2 - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, I1, CPR232(RTREG));                                  // mov     i1,<cpr2_rt>
			UML_CALLH(block, *m_write32[m_core->mode >> 1]);    // callh   write32
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x3e:  /* SDC2 - MIPS II */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_DMOV(block, I1, CPR264(RTREG));                             // dmov    i1,<cpr2_rt>
			UML_CALLH(block, *m_write64[m_core->mode >> 1]);    // callh   write64
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;


		/* ----- effective no-ops ----- */

		case 0x2f:  /* CACHE - MIPS II */
		case 0x33:  /* PREF - MIPS IV */
			return true;


		/* ----- coprocessor instructions ----- */

		case 0x10:  /* COP0 - MIPS I */
			return generate_cop0(block, compiler, desc);

		case 0x11:  /* COP1 - MIPS I */
			return generate_cop1(block, compiler, desc);

		case 0x13:  /* COP1X - MIPS IV */
			return generate_cop1x(block, compiler, desc);

		case 0x12:  /* COP2 - MIPS I */
			UML_EXH(block, *m_exception[EXCEPTION_INVALIDOP], 0);// exh     invalidop,0
			return true;


		/* ----- unimplemented/illegal instructions ----- */

//      default:    /* ??? */       invalid_instruction(op);                                                break;
	}

	return false;
}


/*-------------------------------------------------
    generate_special - compile opcodes in the
    'SPECIAL' group
-------------------------------------------------*/

bool mips3_device::generate_special(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	uint32_t op = desc->opptr.l[0];
	uint8_t opswitch = op & 63;

	switch (opswitch)
	{
		/* ----- shift instructions ----- */

		case 0x00:  /* SLL - MIPS I */
			if (RDREG != 0)
			{
				UML_SHL(block, I0, R32(RTREG), SHIFT);                  // shl     i0,<rtreg>,<shift>
				UML_DSEXT(block, R64(RDREG), I0, SIZE_DWORD);                       // dsext   <rdreg>,i0,dword
			}
			return true;

		case 0x02:  /* SRL - MIPS I */
			if (RDREG != 0)
			{
				UML_SHR(block, I0, R32(RTREG), SHIFT);                  // shr     i0,<rtreg>,<shift>
				UML_DSEXT(block, R64(RDREG), I0, SIZE_DWORD);                       // dsext   <rdreg>,i0,dword
			}
			return true;

		case 0x03:  /* SRA - MIPS I */
			if (RDREG != 0)
			{
				UML_SAR(block, I0, R32(RTREG), SHIFT);                  // sar     i0,<rtreg>,<shift>
				UML_DSEXT(block, R64(RDREG), I0, SIZE_DWORD);                       // dsext   <rdreg>,i0,dword
			}
			return true;

		case 0x04:  /* SLLV - MIPS I */
			if (RDREG != 0)
			{
				UML_SHL(block, I0, R32(RTREG), R32(RSREG));                 // shl     i0,<rtreg>,<rsreg>
				UML_DSEXT(block, R64(RDREG), I0, SIZE_DWORD);                       // dsext   <rdreg>,i0,dword
			}
			return true;

		case 0x06:  /* SRLV - MIPS I */
			if (RDREG != 0)
			{
				UML_SHR(block, I0, R32(RTREG), R32(RSREG));                 // shr     i0,<rtreg>,<rsreg>
				UML_DSEXT(block, R64(RDREG), I0, SIZE_DWORD);                       // dsext   <rdreg>,i0,dword
			}
			return true;

		case 0x07:  /* SRAV - MIPS I */
			if (RDREG != 0)
			{
				UML_SAR(block, I0, R32(RTREG), R32(RSREG));                 // sar     i0,<rtreg>,<rsreg>
				UML_DSEXT(block, R64(RDREG), I0, SIZE_DWORD);                       // dsext   <rdreg>,i0,dword
			}
			return true;

		case 0x38:  /* DSLL - MIPS III */
			if (RDREG != 0)
				UML_DSHL(block, R64(RDREG), R64(RTREG), SHIFT);             // dshl    <rdreg>,<rtreg>,<shift>
			return true;

		case 0x3a:  /* DSRL - MIPS III */
			if (RDREG != 0)
				UML_DSHR(block, R64(RDREG), R64(RTREG), SHIFT);             // dshr    <rdreg>,<rtreg>,<shift>
			return true;

		case 0x3b:  /* DSRA - MIPS III */
			if (RDREG != 0)
				UML_DSAR(block, R64(RDREG), R64(RTREG), SHIFT);             // dsar    <rdreg>,<rtreg>,<shift>
			return true;

		case 0x3c:  /* DSLL32 - MIPS III */
			if (RDREG != 0)
				UML_DSHL(block, R64(RDREG), R64(RTREG), SHIFT + 32);            // dshl    <rdreg>,<rtreg>,<shift>+32
			return true;

		case 0x3e:  /* DSRL32 - MIPS III */
			if (RDREG != 0)
				UML_DSHR(block, R64(RDREG), R64(RTREG), SHIFT + 32);            // dshr    <rdreg>,<rtreg>,<shift>+32
			return true;

		case 0x3f:  /* DSRA32 - MIPS III */
			if (RDREG != 0)
				UML_DSAR(block, R64(RDREG), R64(RTREG), SHIFT + 32);            // dsar    <rdreg>,<rtreg>,<shift>+32
			return true;

		case 0x14:  /* DSLLV - MIPS III */
			if (RDREG != 0)
				UML_DSHL(block, R64(RDREG), R64(RTREG), R64(RSREG));                // dshl    <rdreg>,<rtreg>,<rsreg>
			return true;

		case 0x16:  /* DSRLV - MIPS III */
			if (RDREG != 0)
				UML_DSHR(block, R64(RDREG), R64(RTREG), R64(RSREG));                // dshr    <rdreg>,<rtreg>,<rsreg>
			return true;

		case 0x17:  /* DSRAV - MIPS III */
			if (RDREG != 0)
				UML_DSAR(block, R64(RDREG), R64(RTREG), R64(RSREG));                // dsar    <rdreg>,<rtreg>,<rsreg>
			return true;


		/* ----- basic arithmetic ----- */

		case 0x20:  /* ADD - MIPS I */
			if (m_drcoptions & MIPS3DRC_CHECK_OVERFLOWS)
			{
				UML_ADD(block, I0, R32(RSREG), R32(RTREG));                 // add     i0,<rsreg>,<rtreg>
				UML_EXHc(block, COND_V, *m_exception[EXCEPTION_OVERFLOW], 0);
																					// exh     overflow,0,V
				if (RDREG != 0)
					UML_DSEXT(block, R64(RDREG), I0, SIZE_DWORD);                   // dsext   <rdreg>,i0,dword
			}
			else if (RDREG != 0)
			{
				UML_ADD(block, I0, R32(RSREG), R32(RTREG));                 // add     i0,<rsreg>,<rtreg>
				UML_DSEXT(block, R64(RDREG), I0, SIZE_DWORD);                       // dsext   <rdreg>,i0,dword
			}
			return true;

		case 0x21:  /* ADDU - MIPS I */
			if (RDREG != 0)
			{
				UML_ADD(block, I0, R32(RSREG), R32(RTREG));                 // add     i0,<rsreg>,<rtreg>
				UML_DSEXT(block, R64(RDREG), I0, SIZE_DWORD);                       // dsext   <rdreg>,i0,dword
			}
			return true;

		case 0x2c:  /* DADD - MIPS III */
			if (m_drcoptions & MIPS3DRC_CHECK_OVERFLOWS)
			{
				UML_DADD(block, I0, R64(RSREG), R64(RTREG));                    // dadd    i0,<rsreg>,<rtreg>
				UML_EXHc(block, COND_V, *m_exception[EXCEPTION_OVERFLOW], 0);
																					// exh     overflow,0,V
				if (RDREG != 0)
					UML_DMOV(block, R64(RDREG), I0);                            // dmov    <rdreg>,i0
			}
			else if (RDREG != 0)
				UML_DADD(block, R64(RDREG), R64(RSREG), R64(RTREG));                // dadd    <rdreg>,<rsreg>,<rtreg>
			return true;

		case 0x2d:  /* DADDU - MIPS III */
			if (RDREG != 0)
				UML_DADD(block, R64(RDREG), R64(RSREG), R64(RTREG));                // dadd    <rdreg>,<rsreg>,<rtreg>
			return true;

		case 0x22:  /* SUB - MIPS I */
			if (m_drcoptions & MIPS3DRC_CHECK_OVERFLOWS)
			{
				UML_SUB(block, I0, R32(RSREG), R32(RTREG));                 // sub     i0,<rsreg>,<rtreg>
				UML_EXHc(block, COND_V, *m_exception[EXCEPTION_OVERFLOW], 0);
																					// exh     overflow,0,V
				if (RDREG != 0)
					UML_DSEXT(block, R64(RDREG), I0, SIZE_DWORD);                   // dsext   <rdreg>,i0,dword
			}
			else if (RDREG != 0)
			{
				UML_SUB(block, I0, R32(RSREG), R32(RTREG));                 // sub     i0,<rsreg>,<rtreg>
				UML_DSEXT(block, R64(RDREG), I0, SIZE_DWORD);                       // dsext   <rdreg>,i0,dword
			}
			return true;

		case 0x23:  /* SUBU - MIPS I */
			if (RDREG != 0)
			{
				UML_SUB(block, I0, R32(RSREG), R32(RTREG));                 // sub     i0,<rsreg>,<rtreg>
				UML_DSEXT(block, R64(RDREG), I0, SIZE_DWORD);                       // dsext   <rdreg>,i0,dword
			}
			return true;

		case 0x2e:  /* DSUB - MIPS III */
			if (m_drcoptions & MIPS3DRC_CHECK_OVERFLOWS)
			{
				UML_DSUB(block, I0, R64(RSREG), R64(RTREG));                    // dsub    i0,<rsreg>,<rtreg>
				UML_EXHc(block, COND_V, *m_exception[EXCEPTION_OVERFLOW], 0);
																					// exh     overflow,0,V
				if (RDREG != 0)
					UML_DMOV(block, R64(RDREG), I0);                            // dmov    <rdreg>,i0
			}
			else if (RDREG != 0)
				UML_DSUB(block, R64(RDREG), R64(RSREG), R64(RTREG));                // dsub    <rdreg>,<rsreg>,<rtreg>
			return true;

		case 0x2f:  /* DSUBU - MIPS III */
			if (RDREG != 0)
				UML_DSUB(block, R64(RDREG), R64(RSREG), R64(RTREG));                // dsub    <rdreg>,<rsreg>,<rtreg>
			return true;

		case 0x18:  /* MULT - MIPS I */
			UML_MULS(block, I0, I1, R32(RSREG), R32(RTREG));                // muls    i0,i1,<rsreg>,<rtreg>
			UML_DSEXT(block, LO64, I0, SIZE_DWORD);                                 // dsext   lo,i0,dword
			UML_DSEXT(block, HI64, I1, SIZE_DWORD);                                 // dsext   hi,i1,dword
			return true;

		case 0x19:  /* MULTU - MIPS I */
			UML_MULU(block, I0, I1, R32(RSREG), R32(RTREG));                // mulu    i0,i1,<rsreg>,<rtreg>
			UML_DSEXT(block, LO64, I0, SIZE_DWORD);                                 // dsext   lo,i0,dword
			UML_DSEXT(block, HI64, I1, SIZE_DWORD);                                 // dsext   hi,i1,dword
			return true;

		case 0x1c:  /* DMULT - MIPS III */
			UML_DMULS(block, LO64, HI64, R64(RSREG), R64(RTREG));
			return true;

		case 0x1d:  /* DMULTU - MIPS III */
			UML_DMULU(block, LO64, HI64, R64(RSREG), R64(RTREG));
			return true;

		case 0x1a:  /* DIV - MIPS I */
			UML_DIVS(block, I0, I1, R32(RSREG), R32(RTREG));                    // divs    i0,i1,<rsreg>,<rtreg>
			UML_DSEXT(block, LO64, I0, SIZE_DWORD);                             // dsext   lo,i0,dword
			UML_DSEXT(block, HI64, I1, SIZE_DWORD);                             // dsext   hi,i1,dword
			return true;

		case 0x1b:  /* DIVU - MIPS I */
			UML_DIVU(block, I0, I1, R32(RSREG), R32(RTREG));                    // divu    i0,i1,<rsreg>,<rtreg>
			UML_DSEXT(block, LO64, I0, SIZE_DWORD);                             // dsext   lo,i0,dword
			UML_DSEXT(block, HI64, I1, SIZE_DWORD);                             // dsext   hi,i1,dword
			return true;

		case 0x1e:  /* DDIV - MIPS III */
			UML_DDIVS(block, LO64, HI64, R64(RSREG), R64(RTREG));                   // ddivs    lo,hi,<rsreg>,<rtreg>
			return true;

		case 0x1f:  /* DDIVU - MIPS III */
			UML_DDIVU(block, LO64, HI64, R64(RSREG), R64(RTREG));                   // ddivu    lo,hi,<rsreg>,<rtreg>
			return true;


		/* ----- basic logical ops ----- */

		case 0x24:  /* AND - MIPS I */
			if (RDREG != 0)
				UML_DAND(block, R64(RDREG), R64(RSREG), R64(RTREG));                // dand     <rdreg>,<rsreg>,<rtreg>
			return true;

		case 0x25:  /* OR - MIPS I */
			if (RDREG != 0)
				UML_DOR(block, R64(RDREG), R64(RSREG), R64(RTREG));                 // dor      <rdreg>,<rsreg>,<rtreg>
			return true;

		case 0x26:  /* XOR - MIPS I */
			if (RDREG != 0)
				UML_DXOR(block, R64(RDREG), R64(RSREG), R64(RTREG));                // dxor     <rdreg>,<rsreg>,<rtreg>
			return true;

		case 0x27:  /* NOR - MIPS I */
			if (RDREG != 0)
			{
				UML_DOR(block, I0, R64(RSREG), R64(RTREG));                 // dor      i0,<rsreg>,<rtreg>
				UML_DXOR(block, R64(RDREG), I0, (uint64_t)~0);                // dxor     <rdreg>,i0,~0
			}
			return true;


		/* ----- basic comparisons ----- */

		case 0x2a:  /* SLT - MIPS I */
			if (RDREG != 0)
			{
				UML_DCMP(block, R64(RSREG), R64(RTREG));                            // dcmp    <rsreg>,<rtreg>
				UML_DSETc(block, COND_L, R64(RDREG));                                   // dset    <rdreg>,l
			}
			return true;

		case 0x2b:  /* SLTU - MIPS I */
			if (RDREG != 0)
			{
				UML_DCMP(block, R64(RSREG), R64(RTREG));                            // dcmp    <rsreg>,<rtreg>
				UML_DSETc(block, COND_B, R64(RDREG));                                   // dset    <rdreg>,b
			}
			return true;


		/* ----- conditional traps ----- */

		case 0x30:  /* TGE - MIPS II */
			UML_DCMP(block, R64(RSREG), R64(RTREG));                                // dcmp    <rsreg>,<rtreg>
			UML_EXHc(block, COND_GE, *m_exception[EXCEPTION_TRAP], 0);// exh     trap,0,GE
			return true;

		case 0x31:  /* TGEU - MIPS II */
			UML_DCMP(block, R64(RSREG), R64(RTREG));                                // dcmp    <rsreg>,<rtreg>
			UML_EXHc(block, COND_AE, *m_exception[EXCEPTION_TRAP], 0);// exh     trap,0,AE
			return true;

		case 0x32:  /* TLT - MIPS II */
			UML_DCMP(block, R64(RSREG), R64(RTREG));                                // dcmp    <rsreg>,<rtreg>
			UML_EXHc(block, COND_L, *m_exception[EXCEPTION_TRAP], 0);// exh     trap,0,LT
			return true;

		case 0x33:  /* TLTU - MIPS II */
			UML_DCMP(block, R64(RSREG), R64(RTREG));                                // dcmp    <rsreg>,<rtreg>
			UML_EXHc(block, COND_B, *m_exception[EXCEPTION_TRAP], 0);// exh     trap,0,B
			return true;

		case 0x34:  /* TEQ - MIPS II */
			UML_DCMP(block, R64(RSREG), R64(RTREG));                                // dcmp    <rsreg>,<rtreg>
			UML_EXHc(block, COND_E, *m_exception[EXCEPTION_TRAP], 0);// exh     trap,0,E
			return true;

		case 0x36:  /* TNE - MIPS II */
			UML_DCMP(block, R64(RSREG), R64(RTREG));                                // dcmp    <rsreg>,<rtreg>
			UML_EXHc(block, COND_NE, *m_exception[EXCEPTION_TRAP], 0);// exh     trap,0,NE
			return true;


		/* ----- conditional moves ----- */

		case 0x0a:  /* MOVZ - MIPS IV */
			if (RDREG != 0)
			{
				UML_DCMP(block, R64(RTREG), 0);                             // dcmp    <rtreg>,0
				UML_DMOVc(block, COND_Z, R64(RDREG), R64(RSREG));                       // dmov    <rdreg>,<rsreg>,Z
			}
			return true;

		case 0x0b:  /* MOVN - MIPS IV */
			if (RDREG != 0)
			{
				UML_DCMP(block, R64(RTREG), 0);                             // dcmp    <rtreg>,0
				UML_DMOVc(block, COND_NZ, R64(RDREG), R64(RSREG));                  // dmov    <rdreg>,<rsreg>,NZ
			}
			return true;

		case 0x01:  /* MOVF/MOVT - MIPS IV */
			if (RDREG != 0)
			{
				UML_TEST(block, CCR132(31), FCCMASK(op >> 18));             // test    ccr31,fcc_mask[x]
				UML_DMOVc(block, ((op >> 16) & 1) ? COND_NZ : COND_Z, R64(RDREG), R64(RSREG));
																					// dmov    <rdreg>,<rsreg>,NZ/Z
			}
			return true;


		/* ----- jumps and branches ----- */

		case 0x08:  /* JR - MIPS I */
			generate_delay_slot_and_branch(block, compiler, desc, 0);        // <next instruction + hashjmp>
			return true;

		case 0x09:  /* JALR - MIPS I */
			generate_delay_slot_and_branch(block, compiler, desc, RDREG);    // <next instruction + hashjmp>
			return true;


		/* ----- system calls ----- */

		case 0x0c:  /* SYSCALL - MIPS I */
			UML_EXH(block, *m_exception[EXCEPTION_SYSCALL], 0);  // exh     syscall,0
			return true;

		case 0x0d:  /* BREAK - MIPS I */
			UML_EXH(block, *m_exception[EXCEPTION_BREAK], 0);    // exh     break,0
			return true;


		/* ----- effective no-ops ----- */

		case 0x0f:  /* SYNC - MIPS II */
			return true;


		/* ----- hi/lo register access ----- */

		case 0x10:  /* MFHI - MIPS I */
			if (RDREG != 0)
				UML_DMOV(block, R64(RDREG), HI64);                                  // dmov    <rdreg>,hi
			return true;

		case 0x11:  /* MTHI - MIPS I */
			UML_DMOV(block, HI64, R64(RSREG));                                      // dmov    hi,<rsreg>
			return true;

		case 0x12:  /* MFLO - MIPS I */
			if (RDREG != 0)
				UML_DMOV(block, R64(RDREG), LO64);                                  // dmov    <rdreg>,lo
			return true;

		case 0x13:  /* MTLO - MIPS I */
			UML_DMOV(block, LO64, R64(RSREG));                                      // dmov    lo,<rsreg>
			return true;
	}
	return false;
}


/*-------------------------------------------------
    generate_regimm - compile opcodes in the
    'REGIMM' group
-------------------------------------------------*/

bool mips3_device::generate_regimm(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	uint32_t op = desc->opptr.l[0];
	uint8_t opswitch = RTREG;
	uml::code_label skip;

	switch (opswitch)
	{
		case 0x00:  /* BLTZ */
		case 0x02:  /* BLTZL */
		case 0x10:  /* BLTZAL */
		case 0x12:  /* BLTZALL */
			if (opswitch & 0x10)
			{
				UML_DMOV(block, R64(31), (int32_t)(desc->pc + 8));
			}
			if (RSREG != 0)
			{
				UML_DCMP(block, R64(RSREG), 0);                             // dcmp    <rsreg>,0
				UML_JMPc(block, COND_GE, skip = compiler.labelnum++);              // jmp     skip,GE
				generate_delay_slot_and_branch(block, compiler, desc, 0);
																					// <next instruction + hashjmp>
				UML_LABEL(block, skip);                                         // skip:
			}
			return true;

		case 0x01:  /* BGEZ */
		case 0x03:  /* BGEZL */
		case 0x11:  /* BGEZAL */
		case 0x13:  /* BGEZALL */
			if (opswitch & 0x10)
			{
				UML_DMOV(block, R64(31), (int32_t)(desc->pc + 8));
			}

			if (RSREG != 0)
			{
				UML_DCMP(block, R64(RSREG), 0);                             // dcmp    <rsreg>,0
				UML_JMPc(block, COND_L, skip = compiler.labelnum++);       // jmp     skip,L
				generate_delay_slot_and_branch(block, compiler, desc, 0);   // <next instruction + hashjmp>
				UML_LABEL(block, skip);                                     // skip:
			}
			else
			{
				generate_delay_slot_and_branch(block, compiler, desc, 0);   // <next instruction + hashjmp>
			}
			return true;

		case 0x08:  /* TGEI */
			UML_DCMP(block, R64(RSREG), SIMMVAL);                               // dcmp    <rsreg>,SIMMVAL
			UML_EXHc(block, COND_GE, *m_exception[EXCEPTION_TRAP], 0);// exh     trap,0,GE
			return true;

		case 0x09:  /* TGEIU */
			UML_DCMP(block, R64(RSREG), SIMMVAL);                               // dcmp    <rsreg>,SIMMVAL
			UML_EXHc(block, COND_AE, *m_exception[EXCEPTION_TRAP], 0);// exh     trap,0,AE
			return true;

		case 0x0a:  /* TLTI */
			UML_DCMP(block, R64(RSREG), SIMMVAL);                               // dcmp    <rsreg>,SIMMVAL
			UML_EXHc(block, COND_L, *m_exception[EXCEPTION_TRAP], 0);// exh     trap,0,L
			return true;

		case 0x0b:  /* TLTIU */
			UML_DCMP(block, R64(RSREG), SIMMVAL);                               // dcmp    <rsreg>,SIMMVAL
			UML_EXHc(block, COND_B, *m_exception[EXCEPTION_TRAP], 0);// exh     trap,0,B
			return true;

		case 0x0c:  /* TEQI */
			UML_DCMP(block, R64(RSREG), SIMMVAL);                               // dcmp    <rsreg>,SIMMVAL
			UML_EXHc(block, COND_E, *m_exception[EXCEPTION_TRAP], 0);// exh     trap,0,E
			return true;

		case 0x0e:  /* TNEI */
			UML_DCMP(block, R64(RSREG), SIMMVAL);                               // dcmp    <rsreg>,SIMMVAL
			UML_EXHc(block, COND_NE, *m_exception[EXCEPTION_TRAP], 0);// exh     trap,0,NE
			return true;
	}
	return false;
}


/*-------------------------------------------------
    generate_idt - compile opcodes in the IDT-
    specific group
-------------------------------------------------*/

bool mips3_device::generate_idt(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	uint32_t op = desc->opptr.l[0];
	uint8_t opswitch = op & 0x1f;

	/* only enabled on IDT processors */
	if (m_flavor != MIPS3_TYPE_R4650)
		return false;

	switch (opswitch)
	{
		case 0: /* MAD */
			if (RSREG != 0 && RTREG != 0)
			{
				UML_MULS(block, I0, I1, R32(RSREG), R32(RTREG));                // muls   i0,i1,rsreg,rtreg
				UML_ADD(block, I0, I0, LO32);                                   // add    i0,i0,lo
				UML_ADDC(block, I1, I1, HI32);                                  // addc   i1,i1,hi
				UML_DSEXT(block, LO64, I0, SIZE_DWORD);                         // dsext   lo,i0,dword
				UML_DSEXT(block, HI64, I1, SIZE_DWORD);                         // dsext   hi,i1,dword
			}
			return true;

		case 1: /* MADU */
			if (RSREG != 0 && RTREG != 0)
			{
				UML_MULU(block, I0, I1, R32(RSREG), R32(RTREG));                // mulu   i0,i1,rsreg,rtreg
				UML_ADD(block, I0, I0, LO32);                                   // add    i0,i0,lo
				UML_ADDC(block, I1, I1, HI32);                                  // addc   i1,i1,hi
				UML_DSEXT(block, LO64, I0, SIZE_DWORD);                         // dsext   lo,i0,dword
				UML_DSEXT(block, HI64, I1, SIZE_DWORD);                         // dsext   hi,i1,dword
			}
			return true;

		case 2: /* MUL */
			if (RDREG != 0)
			{
				UML_MULS(block, I0, I0, R32(RSREG), R32(RTREG));                // muls   i0,i0,rsreg,rtreg
				UML_DSEXT(block, R64(RDREG), I0, SIZE_DWORD);                   // dsext  rdreg,i0,dword
			}
			return true;
	}
	return false;
}


/*-------------------------------------------------
    generate_set_cop0_reg - generate code to
    handle special COP0 registers
-------------------------------------------------*/

bool mips3_device::generate_set_cop0_reg(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint8_t reg)
{
	int in_delay_slot = ((desc->flags & OPFLAG_IN_DELAY_SLOT) != 0);
	uml::code_label link;

	switch (reg)
	{
		case COP0_Cause:
			UML_ROLINS(block, CPR032(COP0_Cause), I0, 0, ~0xfc00);              // rolins  [Cause],i0,0,~0xfc00
			compiler.checksoftints = true;
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case COP0_Status:
			generate_update_cycles(block, compiler, desc->pc, !in_delay_slot);  // <subtract cycles>
			UML_MOV(block, I1, CPR032(COP0_Status));                            // mov     i1,[Status]
			UML_MOV(block, CPR032(COP0_Status), I0);                            // mov     [Status],i0
			generate_update_mode(block);                                        // <update mode>
			UML_XOR(block, I0, I0, I1);                                         // xor     i0,i0,i1
			UML_TEST(block, I0, 0x8000);                                        // test    i0,0x8000
			UML_CALLCc(block, COND_NZ, cfunc_mips3com_update_cycle_counting, this);      // callc   mips3com_update_cycle_counting,mips.core,NZ
			compiler.checkints = true;
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case COP0_Count:
			// don't count the cycle for this instruction yet; we need a non-zero cycle
			// count in case we are in a delay slot, otherwise the test for negative cycles
			// won't be generated (due to compiler.cycles == 0); see the loop during early
			// boot of gauntdl, @BFC01A24
			compiler.cycles--;
			generate_update_cycles(block, compiler, desc->pc, !in_delay_slot);  // <subtract cycles>
			UML_MOV(block, CPR032(COP0_Count), I0);                             // mov     [Count],i0
			UML_CALLC(block, cfunc_get_cycles, this);                           // callc   cfunc_get_cycles,mips3
			UML_DAND(block, I0, I0, 0xffffffff);                                // and     i0,i0,0xffffffff
			UML_DADD(block, I0, I0, I0);                                        // dadd    i0,i0,i0
			UML_DSUB(block, mem(&m_core->count_zero_time), mem(&m_core->numcycles), I0);
																				// dsub    [count_zero_time],[m_numcycles],i0
			UML_CALLC(block, cfunc_mips3com_update_cycle_counting, this);       // callc   mips3com_update_cycle_counting,mips.core
			compiler.cycles++;
			return true;

		case COP0_Compare:
			UML_MOV(block, mem(&m_core->compare_armed), 1);                     // mov     [compare_armed],1
			generate_update_cycles(block, compiler, desc->pc, !in_delay_slot);  // <subtract cycles>
			UML_MOV(block, CPR032(COP0_Compare), I0);                           // mov     [Compare],i0
			UML_AND(block, CPR032(COP0_Cause), CPR032(COP0_Cause), ~0x8000);    // and     [Cause],[Cause],~0x8000
			UML_CALLC(block, cfunc_mips3com_update_cycle_counting, this);       // callc   mips3com_update_cycle_counting,mips.core
			return true;

		case COP0_PRId:
			return true;

		case COP0_Config:
			UML_ROLINS(block, CPR032(COP0_Config), I0, 0, 0x0007);              // rolins  [Config],i0,0,0x0007
			return true;

		case COP0_EntryHi:
			UML_XOR(block, I1, I0, CPR032(reg));                                // xor     i1,i0,cpr0[reg]
			UML_MOV(block, CPR032(reg), I0);                                    // mov     cpr0[reg],i0
			UML_TEST(block, I1, 0xff);                                          // test    i1,0xff
			UML_JMPc(block, COND_Z, link = compiler.labelnum++);                // jmp     link,z
			UML_CALLC(block, cfunc_mips3com_asid_changed, this);                // callc   mips3com_asid_changed
			UML_LABEL(block, link);                                             // link:
			return true;

		default:
			UML_MOV(block, CPR032(reg), I0);                                    // mov     cpr0[reg],i0
			return true;
	}
}


/*-------------------------------------------------
    generate_get_cop0_reg - generate code to
    read special COP0 registers
-------------------------------------------------*/

bool mips3_device::generate_get_cop0_reg(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint8_t reg)
{
	uml::code_label link1, link2;

	switch (reg)
	{
		case COP0_Count:
			// don't count the cycle for this instruction yet; we need a non-zero cycle
			// count in case we are in a delay slot, otherwise the test for negative cycles
			// won't be generated (due to compiler.cycles == 0); see the loop during early
			// boot of gauntdl, @BFC01A24
			compiler.cycles--;
			generate_update_cycles(block, compiler, desc->pc, false);           // <subtract cycles>
			UML_CALLC(block, cfunc_get_cycles, this);                           // callc   cfunc_get_cycles,mips3
			UML_DSUB(block, I0, mem(&m_core->numcycles), mem(&m_core->count_zero_time));
																				// dsub    i0,[numcycles],[count_zero_time]
			UML_DSHR(block, I0, I0, 1);                                         // dshr    i0,i0,1
			UML_DSEXT(block, I0, I0, SIZE_DWORD);                               // dsext   i0,i0,dword
			compiler.cycles++;
			return true;

		case COP0_Random:
			generate_update_cycles(block, compiler, desc->pc, false);           // <subtract cycles>
			UML_CALLC(block, cfunc_get_cycles, this);                           // callc   cfunc_get_cycles,mips3
			UML_DSUB(block, I0, mem(&m_core->numcycles), mem(&m_core->count_zero_time));
																				// dsub    i0,[numcycles],[count_zero_time]
			UML_AND(block, I1, CPR032(COP0_Wired), 0x3f);                       // and     i1,[Wired],0x3f
			UML_SUB(block, I2, 48, I1);                                         // sub     i2,48,i1
			UML_JMPc(block, COND_BE, link1 = compiler.labelnum++);              // jmp     link1,BE
			UML_DAND(block, I2, I2, 0xffffffff);                                // dand    i2,i2,0xffffffff
			UML_DDIVU(block, I0, I2, I0, I2);                                   // ddivu   i0,i2,i0,i2
			UML_ADD(block, I0, I2, I1);                                         // add     i0,i2,i1
			UML_DAND(block, I0, I0, 0x3f);                                      // dand    i0,i0,0x3f
			UML_JMP(block, link2 = compiler.labelnum++);                        // jmp     link2
			UML_LABEL(block, link1);                                            // link1:
			UML_DMOV(block, I0, 47);                                            // dmov    i0,47
			UML_LABEL(block, link2);                                            // link2:
			return true;

		default:
			UML_DSEXT(block, I0, CPR032(reg), SIZE_DWORD);                      // dsext   i0,cpr0[reg],dword
			return true;
	}
}


/*-------------------------------------------------------------------------
    generate_badcop - raise a BADCOP exception
-------------------------------------------------------------------------*/

void mips3_device::generate_badcop(drcuml_block &block, const int cop)
{
	UML_TEST(block, CPR032(COP0_Status), SR_COP0 << cop);               // test    [Status], SR_COP0 << cop
	UML_EXHc(block, COND_Z, *m_exception[EXCEPTION_BADCOP], cop);       // exh     badcop,cop,Z
}

/*-------------------------------------------------------------------------
    check_cop0_access - raise a BADCOP exception if we're not in kernel mode
-------------------------------------------------------------------------*/

void mips3_device::check_cop0_access(drcuml_block &block)
{
	if ((m_core->mode >> 1) != MODE_KERNEL)
	{
		generate_badcop(block, 0);
	}
}

/*-------------------------------------------------
    generate_cop0 - compile COP0 opcodes
-------------------------------------------------*/

bool mips3_device::generate_cop0(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	uint32_t op = desc->opptr.l[0];
	uint8_t opswitch = RSREG;
	int skip;

	/* generate an exception if COP0 is disabled unless we are in kernel mode */
	if ((m_core->mode >> 1) != MODE_KERNEL)
	{
		UML_TEST(block, CPR032(COP0_Status), SR_COP0);                          // test    [Status],SR_COP0
		UML_EXHc(block, COND_Z, *m_exception[EXCEPTION_BADCOP], 0);             // exh     cop,0,Z
	}

	switch (opswitch)
	{
		case 0x00:  /* MFCz */
			if (RTREG != 0)
			{
				generate_get_cop0_reg(block, compiler, desc, RDREG);            // <get cop0 reg>
				UML_DSEXT(block, R64(RTREG), I0, SIZE_DWORD);                   // dsext   <rtreg>,i0,dword
			}
			return true;

		case 0x01:  /* DMFCz */
			if (RTREG != 0)
			{
				generate_get_cop0_reg(block, compiler, desc, RDREG);            // <get cop0 reg>
				UML_DMOV(block, R64(RTREG), I0);                                // dmov    <rtreg>,i0
			}
			return true;

		case 0x02:  /* CFCz */
			if (RTREG != 0)
				UML_DSEXT(block, R64(RTREG), CCR032(RDREG), SIZE_DWORD);        // dsext   <rtreg>,ccr0[rdreg],dword
			return true;

		case 0x04:  /* MTCz */
			UML_DSEXT(block, I0, R32(RTREG), SIZE_DWORD);                       // dsext   i0,<rtreg>,dword
			generate_set_cop0_reg(block, compiler, desc, RDREG);                // <set cop0 reg>
			return true;

		case 0x05:  /* DMTCz */
			UML_DMOV(block, I0, R64(RTREG));                                    // dmov    i0,<rtreg>
			generate_set_cop0_reg(block, compiler, desc, RDREG);                // <set cop0 reg>
			return true;

		case 0x06:  /* CTCz */
			UML_DSEXT(block, CCR064(RDREG), R32(RTREG), SIZE_DWORD);            // dsext   ccr0[rdreg],<rtreg>,dword
			return true;

		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:  /* COP */
			switch (op & 0x01ffffff)
			{
				case 0x01:  /* TLBR */
					UML_CALLC(block, cfunc_mips3com_tlbr, this);                // callc   mips3com_tlbr,mips3
					return true;

				case 0x02:  /* TLBWI */
					UML_CALLC(block, cfunc_mips3com_tlbwi, this);               // callc   mips3com_tlbwi,mips3
					return true;

				case 0x06:  /* TLBWR */
					UML_CALLC(block, cfunc_mips3com_tlbwr, this);               // callc   mips3com_tlbwr,mips3
					return true;

				case 0x08:  /* TLBP */
					UML_CALLC(block, cfunc_mips3com_tlbp, this);                // callc   mips3com_tlbp,mips3
					return true;

				case 0x18:  /* ERET */
					UML_MOV(block, mem(&m_core->llbit), 0);                     // mov     [llbit],0
					UML_MOV(block, I0, CPR032(COP0_Status));                    // mov     i0,[Status]
					UML_TEST(block, I0, SR_ERL);                                // test    i0,SR_ERL
					UML_JMPc(block, COND_NZ, skip = compiler.labelnum++);       // jmp     skip,nz
					UML_AND(block, I0, I0, ~SR_EXL);                            // and     i0,i0,~SR_EXL
					UML_MOV(block, CPR032(COP0_Status), I0);                    // mov     [Status],i0
					generate_update_mode(block);
					compiler.checkints = true;
					generate_update_cycles(block, compiler, CPR032(COP0_EPC), true);// <subtract cycles>
					UML_HASHJMP(block, mem(&m_core->mode), CPR032(COP0_EPC), *m_nocode);
																				// hashjmp <mode>,[EPC],nocode
					UML_LABEL(block, skip);                                     // skip:
					UML_AND(block, I0, I0, ~SR_ERL);                            // and     i0,i0,~SR_ERL
					UML_MOV(block, CPR032(COP0_Status), I0);                    // mov     [Status],i0
					generate_update_mode(block);
					compiler.checkints = true;
					generate_update_cycles(block, compiler, CPR032(COP0_ErrorPC), true); // <subtract cycles>
					UML_HASHJMP(block, mem(&m_core->mode), CPR032(COP0_ErrorPC), *m_nocode); // hashjmp <mode>,[EPC],nocode
					return true;

				case 0x20:  /* WAIT */
					return true;
			}
			break;
	}

	return false;
}



/***************************************************************************
    COP1 RECOMPILATION
***************************************************************************/

/*-------------------------------------------------------------------------
    check_cop1_access - raise a BADCOP exception if COP1 is not enabled
-------------------------------------------------------------------------*/

void mips3_device::check_cop1_access(drcuml_block &block)
{
	if (m_drcoptions & MIPS3DRC_STRICT_COP1)
	{
		generate_badcop(block, 1);
	}
}

/*-------------------------------------------------------
    generate_cop1 - compile COP1 opcodes
---------------------------------------------------------*/

bool mips3_device::generate_cop1(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	uint32_t op = desc->opptr.l[0];
	uml::code_label skip;
	uml::condition_t condition;

	check_cop1_access(block);

	switch (RSREG)
	{
		case 0x00:  /* MFC1 - MIPS I */
			if (RTREG != 0)
			{
				UML_DSEXT(block, R64(RTREG), FPR32(RDREG), SIZE_DWORD);     // dsext   <rtreg>,fpr[rdreg],dword
			}
			return true;

		case 0x01:  /* DMFC1 - MIPS III */
			if (RTREG != 0)
			{
				UML_DMOV(block, R64(RTREG), FPR64(RDREG));                  // dmov    <rtreg>,fpr[rdreg]
			}
			return true;

		case 0x02:  /* CFC1 - MIPS I */
			if (RTREG != 0)
				UML_DSEXT(block, R64(RTREG), CCR132(RDREG), SIZE_DWORD);        // dsext   <rtreg>,ccr132[rdreg],dword
			return true;

		case 0x04:  /* MTC1 - MIPS I */
			UML_MOV(block, FPR32(RDREG), R32(RTREG));                       // mov     fpr[rdreg],<rtreg>
			return true;

		case 0x05:  /* DMTC1 - MIPS III */
			UML_DMOV(block, FPR64(RDREG), R64(RTREG));                      // dmov    fpr[rdreg],<rtreg>
			return true;

		case 0x06:  /* CTC1 - MIPS I */
			if (RDREG != 31)
			{
				UML_DSEXT(block, CCR164(RDREG), R32(RTREG), SIZE_DWORD);        // dsext   ccr1[rdreg],<rtreg>,dword
			}
			else
			{
				UML_XOR(block, I0, CCR132(31), R32(RTREG));                     // xor     i0,ccr1[31],<rtreg>
				UML_DSEXT(block, CCR164(31), R32(RTREG), SIZE_DWORD);           // dsext   ccr1[31],<rtreg>,dword
				UML_TEST(block, I0, 3);                                         // test    i0,3
				UML_JMPc(block, COND_Z, skip = compiler.labelnum++);            // jmp     skip,Z
				UML_AND(block, I0, CCR132(31), 3);                              // and     i0,ccr1[31],3
				UML_LOAD(block, I0, &m_fpmode[0], I0, SIZE_BYTE, SCALE_x1);     // load   i0,fpmode,i0,byte
				UML_SETFMOD(block, I0);                                         // setfmod i0
				UML_LABEL(block, skip);                                         // skip:
			}
			return true;

		case 0x08:  /* BC */
			switch ((op >> 16) & 3)
			{
				case 0x00:  /* BCzF - MIPS I */
				case 0x02:  /* BCzFL - MIPS II */
					UML_TEST(block, CCR132(31), FCCMASK(op >> 18));             // test    ccr1[31],fccmask[which]
					UML_JMPc(block, COND_NZ, skip = compiler.labelnum++);       // jmp     skip,NZ
					generate_delay_slot_and_branch(block, compiler, desc, 0);   // <next instruction + hashjmp>
					UML_LABEL(block, skip);                                     // skip:
					return true;

				case 0x01:  /* BCzT - MIPS I */
				case 0x03:  /* BCzTL - MIPS II */
					UML_TEST(block, CCR132(31), FCCMASK(op >> 18));             // test    ccr1[31],fccmask[which]
					UML_JMPc(block, COND_Z, skip = compiler.labelnum++);        // jmp     skip,Z
					generate_delay_slot_and_branch(block, compiler, desc, 0);   // <next instruction + hashjmp>
					UML_LABEL(block, skip);                                     // skip:
					return true;
			}
			break;

		default:
			switch (op & 0x3f)
			{
				case 0x00:
					if (IS_SINGLE(op))  /* ADD.S - MIPS I */
					{
						UML_FSADD(block, FPR32(FDREG), FPR32(FSREG), FPR32(FTREG));       // fsadd   <fdreg>,<fsreg>,<ftreg>
					}
					else                /* ADD.D - MIPS I */
					{
						UML_FDADD(block, FPR64(FDREG), FPR64(FSREG), FPR64(FTREG));       // fdadd   <fdreg>,<fsreg>,<ftreg>
					}
					return true;

				case 0x01:
					if (IS_SINGLE(op))  /* SUB.S - MIPS I */
					{
						UML_FSSUB(block, FPR32(FDREG), FPR32(FSREG), FPR32(FTREG));       // fssub   <fdreg>,<fsreg>,<ftreg>
					}
					else                /* SUB.D - MIPS I */
					{
						UML_FDSUB(block, FPR64(FDREG), FPR64(FSREG), FPR64(FTREG));       // fdsub   <fdreg>,<fsreg>,<ftreg>
					}
					return true;

				case 0x02:
					if (IS_SINGLE(op))  /* MUL.S - MIPS I */
					{
						UML_FSMUL(block, FPR32(FDREG), FPR32(FSREG), FPR32(FTREG));       // fsmul   <fdreg>,<fsreg>,<ftreg>
					}
					else                /* MUL.D - MIPS I */
					{
						UML_FDMUL(block, FPR64(FDREG), FPR64(FSREG), FPR64(FTREG));       // fdmul    <fdreg>,<fsreg>,<ftreg>
					}
					return true;

				case 0x03:
					if (IS_SINGLE(op))  /* DIV.S - MIPS I */
					{
						UML_TEST(block, FPR32(FTREG), 0xffffffff);                              // test    <ftreg>,-1
						UML_JMPc(block, COND_NZ, skip = compiler.labelnum++);                   // jmp     skip,NZ
						UML_TEST(block, CCR132(31), 1 << (FCR31_ENABLE + FPE_DIV0));            // test    [FCR31], 1 << DIV0_EN
						UML_EXHc(block, COND_NZ, *m_exception[EXCEPTION_FPE], 1 << FPE_DIV0);   // exh     FPE,cop,FPE_DIV0
						UML_LABEL(block, skip);                                                 // skip:
						UML_FSDIV(block, FPR32(FDREG), FPR32(FSREG), FPR32(FTREG));             // fsdiv   <fdreg>,<fsreg>,<ftreg>
					}
					else                /* DIV.D - MIPS I */
					{
						UML_DTEST(block, FPR64(FTREG), 0xffffffff'ffffffffull);                 // dtest    <ftreg>,-1
						UML_JMPc(block, COND_NZ, skip = compiler.labelnum++);                   // jmp     skip,NZ
						UML_TEST(block, CCR132(31), 1 << (FCR31_ENABLE + FPE_DIV0));            // test    [FCR31], 1 << DIV0_EN
						UML_EXHc(block, COND_NZ, *m_exception[EXCEPTION_FPE], 1 << FPE_DIV0);   // exh     FPE,cop,FPE_DIV0
						UML_LABEL(block, skip);                                                 // skip:
						UML_FDDIV(block, FPR64(FDREG), FPR64(FSREG), FPR64(FTREG));             // fddiv   <fdreg>,<fsreg>,<ftreg>
					}
					return true;

				case 0x04:
					if (IS_SINGLE(op))  /* SQRT.S - MIPS II */
					{
						UML_FSSQRT(block, FPR32(FDREG), FPR32(FSREG));                  // fssqrt  <fdreg>,<fsreg>
					}
					else                /* SQRT.D - MIPS II */
					{
						UML_FDSQRT(block, FPR64(FDREG), FPR64(FSREG));                  // fdsqrt  <fdreg>,<fsreg>
					}
					return true;

				case 0x05:
					if (IS_SINGLE(op))  /* ABS.S - MIPS I */
					{
						UML_FSABS(block, FPR32(FDREG), FPR32(FSREG));                   // fsabs   <fdreg>,<fsreg>
					}
					else                /* ABS.D - MIPS I */
					{
						UML_FDABS(block, FPR64(FDREG), FPR64(FSREG));                   // fdabs   <fdreg>,<fsreg>
					}
					return true;

				case 0x06:
					if (IS_SINGLE(op))  /* MOV.S - MIPS I */
					{
						UML_FSMOV(block, FPR32(FDREG), FPR32(FSREG));                   // fsmov   <fdreg>,<fsreg>
					}
					else                /* MOV.D - MIPS I */
					{
						UML_FDMOV(block, FPR64(FDREG), FPR64(FSREG));                   // fdmov   <fdreg>,<fsreg>
					}
					return true;

				case 0x07:
					if (IS_SINGLE(op))  /* NEG.S - MIPS I */
					{
						UML_FSNEG(block, FPR32(FDREG), FPR32(FSREG));         // fsneg   <fdreg>,<fsreg>
						UML_CMP(block, FPR32(FSREG), 0);                      // cmp     <fsreg>,0.0
						UML_MOVc(block, COND_E, FPR32(FDREG), 0x80000000);    // mov     <fdreg>,-0.0,e
					}
					else                /* NEG.D - MIPS I */
					{
						UML_FDNEG(block, FPR64(FDREG), FPR64(FSREG));                         // fdneg   <fdreg>,<fsreg>
						UML_DCMP(block, FPR64(FSREG), 0);                                     // cmp     <fsreg>,0.0
						UML_DMOVc(block, COND_E, FPR64(FDREG), 0x8000000000000000U);          // dmov    <fdreg>,-0.0,e
					}
					return true;

				case 0x08:
					if (IS_SINGLE(op))  /* ROUND.L.S - MIPS III */
					{
						UML_FSTOINT(block, FPR64(FDREG), FPR32(FSREG), SIZE_QWORD, ROUND_ROUND);// fstoint <fdreg>,<fsreg>,qword,round
					}
					else                /* ROUND.L.D - MIPS III */
					{
						UML_FDTOINT(block, FPR64(FDREG), FPR64(FSREG), SIZE_QWORD, ROUND_ROUND);// fdtoint <fdreg>,<fsreg>,qword,round
					}
					return true;

				case 0x09:
					if (IS_SINGLE(op))  /* TRUNC.L.S - MIPS III */
					{
						UML_FSTOINT(block, FPR64(FDREG), FPR32(FSREG), SIZE_QWORD, ROUND_TRUNC);// fstoint <fdreg>,<fsreg>,qword,trunc
					}
					else                /* TRUNC.L.D - MIPS III */
					{
						UML_FDTOINT(block, FPR64(FDREG), FPR64(FSREG), SIZE_QWORD, ROUND_TRUNC);// fdtoint <fdreg>,<fsreg>,qword,trunc
					}
					return true;

				case 0x0a:
					if (IS_SINGLE(op))  /* CEIL.L.S - MIPS III */
					{
						UML_FSTOINT(block, FPR64(FDREG), FPR32(FSREG), SIZE_QWORD, ROUND_CEIL);// fstoint <fdreg>,<fsreg>,qword,ceil
					}
					else                /* CEIL.L.D - MIPS III */
					{
						UML_FDTOINT(block, FPR64(FDREG), FPR64(FSREG), SIZE_QWORD, ROUND_CEIL);// fdtoint <fdreg>,<fsreg>,qword,ceil
					}
					return true;

				case 0x0b:
					if (IS_SINGLE(op))  /* FLOOR.L.S - MIPS III */
					{
						UML_FSTOINT(block, FPR64(FDREG), FPR32(FSREG), SIZE_QWORD, ROUND_FLOOR);// fstoint <fdreg>,<fsreg>,qword,floor
					}
					else                /* FLOOR.L.D - MIPS III */
					{
						UML_FDTOINT(block, FPR64(FDREG), FPR64(FSREG), SIZE_QWORD, ROUND_FLOOR);// fdtoint <fdreg>,<fsreg>,qword,floor
					}
					return true;

				case 0x0c:
					if (IS_SINGLE(op))  /* ROUND.W.S - MIPS II */
					{
						UML_FSTOINT(block, FPR32(FDREG), FPR32(FSREG), SIZE_DWORD, ROUND_ROUND);// fstoint <fdreg>,<fsreg>,dword,round
					}
					else                /* ROUND.W.D - MIPS II */
					{
						UML_FDTOINT(block, FPR32(FDREG), FPR64(FSREG), SIZE_DWORD, ROUND_ROUND);// fdtoint <fdreg>,<fsreg>,dword,round
					}
					return true;

				case 0x0d:
					if (IS_SINGLE(op))  /* TRUNC.W.S - MIPS II */
					{
						UML_FSTOINT(block, FPR32(FDREG), FPR32(FSREG), SIZE_DWORD, ROUND_TRUNC);// fstoint <fdreg>,<fsreg>,dword,trunc
					}
					else                /* TRUNC.W.D - MIPS II */
					{
						UML_FDTOINT(block, FPR32(FDREG), FPR64(FSREG), SIZE_DWORD, ROUND_TRUNC);// fdtoint <fdreg>,<fsreg>,dword,trunc
					}
					return true;

				case 0x0e:
					if (IS_SINGLE(op))  /* CEIL.W.S - MIPS II */
					{
						UML_FSTOINT(block, FPR32(FDREG), FPR32(FSREG), SIZE_DWORD, ROUND_CEIL);// fstoint <fdreg>,<fsreg>,dword,ceil
					}
					else                /* CEIL.W.D - MIPS II */
					{
						UML_FDTOINT(block, FPR32(FDREG), FPR64(FSREG), SIZE_DWORD, ROUND_CEIL);// fdtoint <fdreg>,<fsreg>,dword,ceil
					}
					return true;

				case 0x0f:
					if (IS_SINGLE(op))  /* FLOOR.W.S - MIPS II */
					{
						UML_FSTOINT(block, FPR32(FDREG), FPR32(FSREG), SIZE_DWORD, ROUND_FLOOR);// fstoint <fdreg>,<fsreg>,dword,floor
					}
					else                /* FLOOR.W.D - MIPS II */
					{
						UML_FDTOINT(block, FPR32(FDREG), FPR64(FSREG), SIZE_DWORD, ROUND_FLOOR);// fdtoint <fdreg>,<fsreg>,dword,floor
					}
					return true;

				case 0x11:
					condition = ((op >> 16) & 1) ? uml::COND_NZ : uml::COND_Z;
					UML_TEST(block, CCR132(31), FCCMASK(op >> 18));         // test    ccr31,fccmask[op]
					if (IS_SINGLE(op))  /* MOVT/F.S - MIPS IV */
					{
						UML_FSMOVc(block, condition, FPR32(FDREG), FPR32(FSREG));   // fsmov   <fdreg>,<fsreg>,condition
					}
					else                /* MOVT/F.D - MIPS IV */
					{
						UML_FDMOVc(block, condition, FPR64(FDREG), FPR64(FSREG));   // fdmov   <fdreg>,<fsreg>,condition
					}
					return true;

				case 0x12:
					UML_DCMP(block, R64(RTREG), 0);                             // dcmp    <rtreg>,0
					if (IS_SINGLE(op))  /* MOVZ.S - MIPS IV */
					{
						UML_FSMOVc(block, COND_Z, FPR32(FDREG), FPR32(FSREG));      // fsmov   <fdreg>,<fsreg>,Z
					}
					else                /* MOVZ.D - MIPS IV */
					{
						UML_FDMOVc(block, COND_Z, FPR64(FDREG), FPR64(FSREG));      // fdmov   <fdreg>,<fsreg>,Z
					}
					return true;

				case 0x13:
					UML_DCMP(block, R64(RTREG), 0);                                 // dcmp    <rtreg>,0
					if (IS_SINGLE(op))  /* MOVN.S - MIPS IV */
					{
						UML_FSMOVc(block, COND_NZ, FPR32(FDREG), FPR32(FSREG));         // fsmov   <fdreg>,<fsreg>,NZ
					}
					else                /* MOVN.D - MIPS IV */
					{
						UML_FDMOVc(block, COND_NZ, FPR64(FDREG), FPR64(FSREG));         // fdmov   <fdreg>,<fsreg>,NZ
					}
					return true;

				case 0x15:
					if (IS_SINGLE(op))  /* RECIP.S - MIPS IV */
					{
						UML_FSRECIP(block, FPR32(FDREG), FPR32(FSREG));                 // fsrecip <fdreg>,<fsreg>
					}
					else                /* RECIP.D - MIPS IV */
					{
						UML_FDRECIP(block, FPR64(FDREG), FPR64(FSREG));                 // fdrecip <fdreg>,<fsreg>
					}
					return true;

				case 0x16:
					if (IS_SINGLE(op))  /* RSQRT.S - MIPS IV */
					{
						UML_FSRSQRT(block, FPR32(FDREG), FPR32(FSREG));                 // fsrsqrt <fdreg>,<fsreg>
					}
					else                /* RSQRT.D - MIPS IV */
					{
						UML_FDRSQRT(block, FPR64(FDREG), FPR64(FSREG));                 // fdrsqrt <fdreg>,<fsreg>
					}
					return true;

				case 0x20:
					if (IS_INTEGRAL(op))
					{
						if (IS_SINGLE(op))  /* CVT.S.W - MIPS I */
						{
							UML_FSFRINT(block, FPR32(FDREG), FPR32(FSREG), SIZE_DWORD); // fsfrint <fdreg>,<fsreg>,dword
						}
						else                /* CVT.S.L - MIPS I */
						{
							UML_FSFRINT(block, FPR32(FDREG), FPR64(FSREG), SIZE_QWORD); // fsfrint <fdreg>,<fsreg>,qword
						}
					}
					else                    /* CVT.S.D - MIPS I */
					{
						UML_FSFRFLT(block, FPR32(FDREG), FPR64(FSREG), SIZE_QWORD);         // fsfrflt <fdreg>,<fsreg>,qword
					}
					return true;

				case 0x21:
					if (IS_INTEGRAL(op))
					{
						if (IS_SINGLE(op))  /* CVT.D.W - MIPS I */
						{
							UML_FDFRINT(block, FPR64(FDREG), FPR32(FSREG), SIZE_DWORD); // fdfrint <fdreg>,<fsreg>,dword
						}
						else                /* CVT.D.L - MIPS I */
						{
							UML_FDFRINT(block, FPR64(FDREG), FPR64(FSREG), SIZE_QWORD); // fdfrint <fdreg>,<fsreg>,qword
						}
					}
					else                    /* CVT.D.S - MIPS I */
					{
						UML_FDFRFLT(block, FPR64(FDREG), FPR32(FSREG), SIZE_DWORD);         // fdfrflt <fdreg>,<fsreg>,dword
					}
					return true;

				case 0x24:
					if (IS_SINGLE(op))  /* CVT.W.S - MIPS I */
					{
						UML_FSTOINT(block, FPR32(FDREG), FPR32(FSREG), SIZE_DWORD, ROUND_DEFAULT);// fstoint <fdreg>,<fsreg>,dword,default
					}
					else                /* CVT.W.D - MIPS I */
					{
						UML_FDTOINT(block, FPR32(FDREG), FPR64(FSREG), SIZE_DWORD, ROUND_DEFAULT);// fdtoint <fdreg>,<fsreg>,dword,default
					}
					return true;

				case 0x25:
					if (IS_SINGLE(op))  /* CVT.L.S - MIPS I */
					{
						UML_FSTOINT(block, FPR64(FDREG), FPR32(FSREG), SIZE_QWORD, ROUND_DEFAULT);// fstoint <fdreg>,<fsreg>,qword,default
					}
					else                /* CVT.L.D - MIPS I */
					{
						UML_FDTOINT(block, FPR64(FDREG), FPR64(FSREG), SIZE_QWORD, ROUND_DEFAULT);// fdtoint <fdreg>,<fsreg>,qword,default
					}
					return true;

				case 0x30:
				case 0x38:              /* C.F.S/D - MIPS I */
					UML_AND(block, CCR132(31), CCR132(31), ~FCCMASK(op >> 8));  // and     ccr31,ccr31,~fccmask[op]
					return true;

				case 0x31:
				case 0x39:
					if (IS_SINGLE(op))  /* C.UN.S - MIPS I */
					{
						UML_FSCMP(block, FPR32(FSREG), FPR32(FTREG));               // fscmp   <fsreg>,<ftreg>
					}
					else                /* C.UN.D - MIPS I */
					{
						UML_FDCMP(block, FPR64(FSREG), FPR64(FTREG));               // fdcmp   <fsreg>,<ftreg>
					}
					UML_SETc(block, COND_U, I0);                                    // set     i0,u
					UML_ROLINS(block, CCR132(31), I0, FCCSHIFT(op >> 8), FCCMASK(op >> 8));
																					// rolins  ccr31,i0,fccshift,fcc
					return true;

				case 0x32:
				case 0x3a:
					if (IS_SINGLE(op))  /* C.EQ.S - MIPS I */
					{
						UML_FSCMP(block, FPR32(FSREG), FPR32(FTREG));               // fscmp   <fsreg>,<ftreg>
					}
					else                /* C.EQ.D - MIPS I */
					{
						UML_FDCMP(block, FPR64(FSREG), FPR64(FTREG));               // fdcmp   <fsreg>,<ftreg>
					}
					UML_SETc(block, COND_E, I0);                                    // set     i0,e
					UML_SETc(block, COND_NU, I1);                               // set     i1,nu
					UML_AND(block, I0, I0, I1);                     // and     i0,i0,i1
					UML_ROLINS(block, CCR132(31), I0, FCCSHIFT(op >> 8), FCCMASK(op >> 8));
																					// rolins  ccr31,i0,fccshift,fcc
					return true;

				case 0x33:
				case 0x3b:
					if (IS_SINGLE(op))  /* C.UEQ.S - MIPS I */
					{
						UML_FSCMP(block, FPR32(FSREG), FPR32(FTREG));               // fscmp   <fsreg>,<ftreg>
					}
					else                /* C.UEQ.D - MIPS I */
					{
						UML_FDCMP(block, FPR64(FSREG), FPR64(FTREG));               // fdcmp   <fsreg>,<ftreg>
					}
					UML_SETc(block, COND_U, I0);                                    // set     i0,u
					UML_SETc(block, COND_E, I1);                                    // set     i1,e
					UML_OR(block, I0, I0, I1);                      // or      i0,i0,i1
					UML_ROLINS(block, CCR132(31), I0, FCCSHIFT(op >> 8), FCCMASK(op >> 8));
																					// rolins  ccr31,i0,fccshift,fcc
					return true;

				case 0x34:
				case 0x3c:
					if (IS_SINGLE(op))  /* C.OLT.S - MIPS I */
					{
						UML_FSCMP(block, FPR32(FSREG), FPR32(FTREG));               // fscmp   <fsreg>,<ftreg>
					}
					else                /* C.OLT.D - MIPS I */
					{
						UML_FDCMP(block, FPR64(FSREG), FPR64(FTREG));               // fdcmp   <fsreg>,<ftreg>
					}
					UML_SETc(block, COND_B, I0);                                    // set     i0,b
					UML_SETc(block, COND_NU, I1);                               // set     i1,nu
					UML_AND(block, I0, I0, I1);                     // and     i0,i0,i1
					UML_ROLINS(block, CCR132(31), I0, FCCSHIFT(op >> 8), FCCMASK(op >> 8));
																					// rolins  ccr31,i0,fccshift,fcc
					return true;

				case 0x35:
				case 0x3d:
					if (IS_SINGLE(op))  /* C.ULT.S - MIPS I */
					{
						UML_FSCMP(block, FPR32(FSREG), FPR32(FTREG));               // fscmp   <fsreg>,<ftreg>
					}
					else                /* C.ULT.D - MIPS I */
					{
						UML_FDCMP(block, FPR64(FSREG), FPR64(FTREG));               // fdcmp   <fsreg>,<ftreg>
					}
					UML_SETc(block, COND_U, I0);                                    // set     i0,u
					UML_SETc(block, COND_B, I1);                                    // set     i1,b
					UML_OR(block, I0, I0, I1);                      // or      i0,i0,i1
					UML_ROLINS(block, CCR132(31), I0, FCCSHIFT(op >> 8), FCCMASK(op >> 8));
																					// rolins  ccr31,i0,fccshift,fcc
					return true;

				case 0x36:
				case 0x3e:
					if (IS_SINGLE(op))  /* C.OLE.S - MIPS I */
					{
						UML_FSCMP(block, FPR32(FSREG), FPR32(FTREG));               // fscmp   <fsreg>,<ftreg>
					}
					else                /* C.OLE.D - MIPS I */
					{
						UML_FDCMP(block, FPR64(FSREG), FPR64(FTREG));               // fdcmp   <fsreg>,<ftreg>
					}
					UML_SETc(block, COND_BE, I0);                               // set     i0,be
					UML_SETc(block, COND_NU, I1);                               // set     i1,nu
					UML_AND(block, I0, I0, I1);                     // and     i0,i0,i1
					UML_ROLINS(block, CCR132(31), I0, FCCSHIFT(op >> 8), FCCMASK(op >> 8));
																					// rolins  ccr31,i0,fccshift,fcc
					return true;

				case 0x37:
				case 0x3f:
					if (IS_SINGLE(op))  /* C.ULE.S - MIPS I */
					{
						UML_FSCMP(block, FPR32(FSREG), FPR32(FTREG));               // fscmp   <fsreg>,<ftreg>
					}
					else                /* C.ULE.D - MIPS I */
					{
						UML_FDCMP(block, FPR64(FSREG), FPR64(FTREG));               // fdcmp   <fsreg>,<ftreg>
					}
					UML_SETc(block, COND_U, I0);                                    // set     i0,u
					UML_SETc(block, COND_BE, I1);                               // set     i1,be
					UML_OR(block, I0, I0, I1);                      // or      i0,i0,i1
					UML_ROLINS(block, CCR132(31), I0, FCCSHIFT(op >> 8), FCCMASK(op >> 8));
																					// rolins  ccr31,i0,fccshift,fcc
					return true;
			}
			break;
	}
	return false;
}

/***************************************************************************
    COP1X RECOMPILATION
***************************************************************************/

/*----------------------------------------------------------
    generate_cop1x - compile COP1X opcodes
----------------------------------------------------------*/

bool mips3_device::generate_cop1x(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	int in_delay_slot = ((desc->flags & OPFLAG_IN_DELAY_SLOT) != 0);
	uint32_t op = desc->opptr.l[0];

	check_cop1_access(block);

	switch (op & 0x3f)
	{
		case 0x00:      /* LWXC1 - MIPS IV */
			UML_ADD(block, I0, R32(RSREG), R32(RTREG));         // add     i0,<rsreg>,<rtreg>
			UML_CALLH(block, *m_read32[m_core->mode >> 1]);     // callh   read32
			UML_MOV(block, FPR32(FDREG), I0);                   // mov     <cpr1_fd>,i0
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x01:      /* LDXC1 - MIPS IV */
			UML_ADD(block, I0, R32(RSREG), R32(RTREG));         // add     i0,<rsreg>,<rtreg>
			UML_CALLH(block, *m_read64[m_core->mode >> 1]);     // callh   read64
			UML_DMOV(block, FPR64(FDREG), I0);                  // dmov    <cpr1_fd>,i0
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x08:      /* SWXC1 - MIPS IV */
			UML_ADD(block, I0, R32(RSREG), R32(RTREG));         // add     i0,<rsreg>,<rtreg>
			UML_MOV(block, I1, FPR32(FSREG));                   // mov     i1,<cpr1_fs>
			UML_CALLH(block, *m_write32[m_core->mode >> 1]);    // callh   write32
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x09:      /* SDXC1 - MIPS IV */
			UML_ADD(block, I0, R32(RSREG), R32(RTREG));         // add     i0,<rsreg>,<rtreg>
			UML_DMOV(block, I1, FPR64(FSREG));                  // dmov    i1,<cpr1_fs>
			UML_CALLH(block, *m_write64[m_core->mode >> 1]);    // callh   write64
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, true);
			return true;

		case 0x0f:      /* PREFX */
			return true;

		case 0x20:      /* MADD.S - MIPS IV */
			UML_FSMUL(block, F0, FPR32(FSREG), FPR32(FTREG));           // fsmul   f0,<fsreg>,<ftreg>
			UML_FSADD(block, FPR32(FDREG), F0, FPR32(FRREG));           // fsadd   <fdreg>,f0,<frreg>
			return true;

		case 0x21:      /* MADD.D - MIPS IV */
			UML_FDMUL(block, F0, FPR64(FSREG), FPR64(FTREG));           // fdmul   f0,<fsreg>,<ftreg>
			UML_FDADD(block, FPR64(FDREG), F0, FPR64(FRREG));           // fdadd   <fdreg>,f0,<frreg>
			return true;

		case 0x28:      /* MSUB.S - MIPS IV */
			UML_FSMUL(block, F0, FPR32(FSREG), FPR32(FTREG));           // fsmul   f0,<fsreg>,<ftreg>
			UML_FSSUB(block, FPR32(FDREG), F0, FPR32(FRREG));           // fssub   <fdreg>,f0,<frreg>
			return true;

		case 0x29:      /* MSUB.D - MIPS IV */
			UML_FDMUL(block, F0, FPR64(FSREG), FPR64(FTREG));           // fdmul   f0,<fsreg>,<ftreg>
			UML_FDSUB(block, FPR64(FDREG), F0, FPR64(FRREG));           // fdadd   <fdreg>,f0,<frreg>
			return true;

		case 0x30:      /* NMADD.S - MIPS IV */
			UML_FSMUL(block, F0, FPR32(FSREG), FPR32(FTREG));           // fsmul   f0,<fsreg>,<ftreg>
			UML_FSADD(block, F0, F0, FPR32(FRREG));                     // fsadd   f0,f0,<frreg>
			UML_FSNEG(block, FPR32(FDREG), F0);                         // fsneg   <fdreg>,f0
			return true;

		case 0x31:      /* NMADD.D - MIPS IV */
			UML_FDMUL(block, F0, FPR64(FSREG), FPR64(FTREG));           // fdmul   f0,<fsreg>,<ftreg>
			UML_FDADD(block, F0, F0, FPR64(FRREG));                     // fdadd   f0,f0,<frreg>
			UML_FDNEG(block, FPR64(FDREG), F0);                         // fdneg   <fdreg>,f0
			return true;

		case 0x38:      /* NMSUB.S - MIPS IV */
			UML_FSMUL(block, F0, FPR32(FSREG), FPR32(FTREG));           // fsmul   f0,<fsreg>,<ftreg>
			UML_FSSUB(block, FPR32(FDREG), FPR32(FRREG), F0);           // fssub   <fdreg>,<frreg>,f0
			return true;

		case 0x39:      /* NMSUB.D - MIPS IV */
			UML_FDMUL(block, F0, FPR64(FSREG), FPR64(FTREG));           // fdmul   f0,<fsreg>,<ftreg>
			UML_FDSUB(block, FPR64(FDREG), FPR64(FRREG), F0);           // fdsub   <fdreg>,<frreg>,f0
			return true;

		default:
			fprintf(stderr, "cop1x %X\n", op);
			break;
	}
	return false;
}

/***************************************************************************
    CODE LOGGING HELPERS
***************************************************************************/

/*-------------------------------------------------
    log_add_disasm_comment - add a comment
    including disassembly of a MIPS instruction
-------------------------------------------------*/

void mips3_device::log_add_disasm_comment(drcuml_block &block, uint32_t pc, uint32_t op)
{
	if (m_drcuml->logging())
	{
		mips3_disassembler mips3d;
		std::ostringstream stream;
		mips3d.dasm_one(stream, pc, op);
		const std::string stream_string = stream.str();
		block.append_comment("%08X: %s", pc, stream_string.c_str());                                  // comment
	}
}


/*-------------------------------------------------
    log_desc_flags_to_string - generate a string
    representing the instruction description
    flags
-------------------------------------------------*/

const char *mips3_device::log_desc_flags_to_string(uint32_t flags)
{
	static char tempbuf[30];
	char *dest = tempbuf;

	/* branches */
	if (flags & OPFLAG_IS_UNCONDITIONAL_BRANCH)
		*dest++ = 'U';
	else if (flags & OPFLAG_IS_CONDITIONAL_BRANCH)
		*dest++ = 'C';
	else
		*dest++ = '.';

	/* intrablock branches */
	*dest++ = (flags & OPFLAG_INTRABLOCK_BRANCH) ? 'i' : '.';

	/* branch targets */
	*dest++ = (flags & OPFLAG_IS_BRANCH_TARGET) ? 'B' : '.';

	/* delay slots */
	*dest++ = (flags & OPFLAG_IN_DELAY_SLOT) ? 'D' : '.';

	/* exceptions */
	if (flags & OPFLAG_WILL_CAUSE_EXCEPTION)
		*dest++ = 'E';
	else if (flags & OPFLAG_CAN_CAUSE_EXCEPTION)
		*dest++ = 'e';
	else
		*dest++ = '.';

	/* read/write */
	if (flags & OPFLAG_READS_MEMORY)
		*dest++ = 'R';
	else if (flags & OPFLAG_WRITES_MEMORY)
		*dest++ = 'W';
	else
		*dest++ = '.';

	/* TLB validation */
	*dest++ = (flags & OPFLAG_VALIDATE_TLB) ? 'V' : '.';

	/* TLB modification */
	*dest++ = (flags & OPFLAG_MODIFIES_TRANSLATION) ? 'T' : '.';

	/* redispatch */
	*dest++ = (flags & OPFLAG_REDISPATCH) ? 'R' : '.';
	return tempbuf;
}


/*-------------------------------------------------
    log_register_list - log a list of GPR registers
-------------------------------------------------*/

void mips3_device::log_register_list(const char *string, const uint32_t *reglist, const uint32_t *regnostarlist)
{
	int count = 0;
	int regnum;

	/* skip if nothing */
	if (reglist[0] == 0 && reglist[1] == 0 && reglist[2] == 0)
		return;

	m_drcuml->log_printf("[%s:", string);

	for (regnum = 1; regnum < 32; regnum++)
		if (reglist[0] & REGFLAG_R(regnum))
		{
			m_drcuml->log_printf("%sr%d", (count++ == 0) ? "" : ",", regnum);
			if (regnostarlist != nullptr && !(regnostarlist[0] & REGFLAG_R(regnum)))
				m_drcuml->log_printf("*");
		}

	for (regnum = 0; regnum < 32; regnum++)
		if (reglist[1] & REGFLAG_CPR1(regnum))
		{
			m_drcuml->log_printf("%sfr%d", (count++ == 0) ? "" : ",", regnum);
			if (regnostarlist != nullptr && !(regnostarlist[1] & REGFLAG_CPR1(regnum)))
				m_drcuml->log_printf("*");
		}

	if (reglist[2] & REGFLAG_LO)
	{
		m_drcuml->log_printf("%slo", (count++ == 0) ? "" : ",");
		if (regnostarlist != nullptr && !(regnostarlist[2] & REGFLAG_LO))
			m_drcuml->log_printf("*");
	}
	if (reglist[2] & REGFLAG_HI)
	{
		m_drcuml->log_printf("%shi", (count++ == 0) ? "" : ",");
		if (regnostarlist != nullptr && !(regnostarlist[2] & REGFLAG_HI))
			m_drcuml->log_printf("*");
	}
	if (reglist[2] & REGFLAG_FCC)
	{
		m_drcuml->log_printf("%sfcc", (count++ == 0) ? "" : ",");
		if (regnostarlist != nullptr && !(regnostarlist[2] & REGFLAG_FCC))
			m_drcuml->log_printf("*");
	}

	m_drcuml->log_printf("] ");
}


/*-------------------------------------------------
    log_opcode_desc - log a list of descriptions
-------------------------------------------------*/

void mips3_device::log_opcode_desc(const opcode_desc *desclist, int indent)
{
	/* open the file, creating it if necessary */
	if (indent == 0)
		m_drcuml->log_printf("\nDescriptor list @ %08X\n", desclist->pc);

	/* output each descriptor */
	for ( ; desclist != nullptr; desclist = desclist->next())
	{
		std::ostringstream buffer;

		/* disassemle the current instruction and output it to the log */
		if (m_drcuml->logging() || m_drcuml->logging_native())
		{
			if (desclist->flags & OPFLAG_VIRTUAL_NOOP)
				buffer << "<virtual nop>";
			else
			{
				mips3_disassembler mips3d;
				mips3d.dasm_one(buffer, desclist->pc, desclist->opptr.l[0]);
			}
		}
		else
			buffer << "???";

		const std::string buffer_string = buffer.str();
		m_drcuml->log_printf("%08X [%08X] t:%08X f:%s: %-30s", desclist->pc, desclist->physpc, desclist->targetpc, log_desc_flags_to_string(desclist->flags), buffer_string.c_str());

		/* output register states */
		log_register_list("use", desclist->regin, nullptr);
		log_register_list("mod", desclist->regout, desclist->regreq);
		m_drcuml->log_printf("\n");

		/* if we have a delay slot, output it recursively */
		if (desclist->delay.first() != nullptr)
			log_opcode_desc(desclist->delay.first(), indent + 1);

		/* at the end of a sequence add a dividing line */
		if (desclist->flags & OPFLAG_END_SEQUENCE)
			m_drcuml->log_printf("-----\n");
	}
}

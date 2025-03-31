// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "emu.h"
#include "e132xs.h"
#include "e132xsfe.h"
#include "32xsdefs.h"


/* map variables */
#define MAPVAR_PC       M0
#define MAPVAR_CYCLES   M1


struct hyperstone_device::compiler_state
{
private:
	const uint8_t   m_mode;
	uint32_t        m_pc = 0;
public:
	uml::code_label m_labelnum  = 1;
	uint8_t         m_check_delay = 1;

	compiler_state(uint8_t mode) : m_mode(mode) { }
	compiler_state(compiler_state const &) = delete;
	compiler_state &operator=(compiler_state const &) = delete;

	uint8_t mode() const { return m_mode; }
	bool user_mode() const { return !BIT(m_mode, 0); }
	bool supervisor_mode() const { return BIT(m_mode, 0); }
	bool trace_mode() const { return BIT(m_mode, 1); }

	auto next_label() { return m_labelnum++; }

	void set_delayed_branch() { m_check_delay = 2; }
	bool check_delay() const { return m_check_delay == 1; }

	uint32_t set_pc(uint32_t val) { return m_pc = val; }
	uint32_t pc() const { return m_pc; }
};


/***************************************************************************
    C FUNCTION CALLBACKS
***************************************************************************/

struct hyperstone_device::c_funcs
{

	static void unimplemented(void *param)
	{
		auto &that = *reinterpret_cast<hyperstone_device *>(param);
		fatalerror("PC=%08X: Unimplemented op %08X\n", that.PC, that.m_core->arg0);
	}

	static void print(void *param)
	{
		auto &that = *reinterpret_cast<hyperstone_device *>(param);
		printf("%c: %08x\n", (char)that.m_core->arg0, that.m_core->arg1);
	}

	static void standard_irq_callback(void *param)
	{
		auto &that = *reinterpret_cast<hyperstone_device *>(param);
		that.standard_irq_callback(that.m_core->arg0, that.m_core->global_regs[0]);
	}

	static void debugger_exception_hook(void *param)
	{
		auto &that = *reinterpret_cast<hyperstone_device *>(param);
		that.debugger_exception_hook(int32_t(that.m_core->arg0));
	}

	static void adjust_timer_interrupt(void *param)
	{
		reinterpret_cast<hyperstone_device *>(param)->adjust_timer_interrupt();
	}

	static void compute_tr(void *param)
	{
		reinterpret_cast<hyperstone_device *>(param)->compute_tr();
	}

	static void update_timer_prescale(void *param)
	{
		reinterpret_cast<hyperstone_device *>(param)->update_timer_prescale();
	}

#if E132XS_LOG_DRC_REGS || E132XS_LOG_INTERPRETER_REGS
	static void dump_registers(void *param)
	{
		reinterpret_cast<hyperstone_device *>(param)->dump_registers();
	}
#endif

	static void total_cycles(void *param)
	{
		auto &that = *reinterpret_cast<hyperstone_device *>(param);
		that.m_core->numcycles = that.total_cycles();
	}
};


#define DRC_PC uml::mem(&m_core->global_regs[0])
#define DRC_SR uml::mem(&m_core->global_regs[1])

void hyperstone_device::execute_run_drc()
{
	// reset the cache if dirty
	if (m_cache_dirty)
	{
		code_flush_cache();
		m_cache_dirty = false;
	}

	// execute
	while (true)
	{
		// run as much as we can
		const int execute_result = m_drcuml->execute(*m_entry);

		// if we need to recompile, do it
		if (execute_result == EXECUTE_MISSING_CODE)
			code_compile_block(bitswap<2>(m_core->global_regs[1], T_SHIFT, S_SHIFT), m_core->global_regs[0]);
		else if (execute_result == EXECUTE_UNMAPPED_CODE)
			fatalerror("Attempted to execute unmapped code at PC=%08X\n", m_core->global_regs[0]);
		else if (execute_result == EXECUTE_RESET_CACHE)
			code_flush_cache();
		else if (execute_result == EXECUTE_OUT_OF_CYCLES)
			break;
	}
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
		{
			// generate the entry point and out-of-cycles handlers
			drcuml_block &block(m_drcuml->begin_block(512));
			uml::code_label label = 1;
			static_generate_helpers(block, label);
			static_generate_exception(block, label);
			block.end();
		}

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
		fatalerror("Unable to generate static E132XS code\n"); fflush(stdout);
	}
}

/* Return the entry point for a determinated trap */
void hyperstone_device::generate_get_trap_addr(drcuml_block &block, uml::code_label &label, uml::parameter trapno)
{
	// sets I0 to target address
	// clobbers I1
	// updates label

	UML_MOV(block, I1, mem(&m_core->trap_entry));
	UML_CMP(block, I1, 0xffffff00);

	if (trapno.is_immediate())
	{
		UML_MOV(block, I0, trapno.immediate() * 4);
		UML_MOVc(block, uml::COND_NE, I0, (63 - trapno.immediate()) * 4);
	}
	else
	{
		const int no_subtract = label++;
		if (trapno != uml::I0)
			UML_MOV(block, I0, trapno);
		UML_JMPc(block, uml::COND_E, no_subtract);
		UML_SUB(block, I0, 63, I0);
		UML_LABEL(block, no_subtract);
		UML_SHL(block, I0, I0, 2);
	}

	UML_OR(block, I0, I0, I1);
}

/*-------------------------------------------------
    code_compile_block - compile a block of the
    given mode at the specified pc
-------------------------------------------------*/

void hyperstone_device::code_compile_block(uint8_t mode, offs_t pc)
{
	compiler_state compiler(mode);
	const opcode_desc *seqhead, *seqlast;
	bool override = false;

	auto profile = g_profiler.start(PROFILER_DRC_COMPILE);

	/* get a description of this sequence */
	const opcode_desc *desclist = m_drcfe->describe_code(pc);

	bool succeeded = false;
	while (!succeeded)
	{
		try
		{
			/* start the block */
			drcuml_block &block(m_drcuml->begin_block(8192));

			/* loop until we get through all instruction sequences */
			for (seqhead = desclist; seqhead != nullptr; seqhead = seqlast->next())
			{
				/* add a code log entry */
				if (m_drcuml->logging())
					block.append_comment("-------------------------");

				/* determine the last instruction in this sequence */
				for (seqlast = seqhead; seqlast != nullptr; seqlast = seqlast->next())
					if (seqlast->flags & OPFLAG_END_SEQUENCE)
						break;
				assert(seqlast != nullptr);

				if (override || !m_drcuml->hash_exists(mode, seqhead->pc))
				{
					// if we don't have a hash for this mode/pc, or if we are overriding all, add one
					UML_HASH(block, mode, seqhead->pc);
				}
				else if (seqhead == desclist)
				{
					// if we already have a hash, and this is the first sequence, assume that we
					// are recompiling due to being out of sync and allow future overrides
					override = true;
					UML_HASH(block, mode, seqhead->pc);
				}
				else
				{
					// otherwise, redispatch to that fixed PC and skip the rest of the processing
					UML_LABEL(block, seqhead->pc | 0x80000000);
					UML_HASHJMP(block, mode, seqhead->pc, *m_nocode);
					continue;
				}

				// validate this code block if we're not pointing into ROM
				if (m_program->get_write_ptr(seqhead->physpc) != nullptr)
					generate_checksum_block(block, compiler, seqhead, seqlast);

				// label this instruction, if it may be jumped to locally
				if (seqhead->flags & OPFLAG_IS_BRANCH_TARGET)
					UML_LABEL(block, seqhead->pc | 0x80000000);

				UML_MOV(block, I7, 0);

				/* iterate over instructions in the sequence and compile them */
				compiler.m_check_delay = 1;
				for (const opcode_desc *curdesc = seqhead; curdesc != seqlast->next(); curdesc = curdesc->next())
				{
					generate_sequence_instruction(block, compiler, curdesc);
					generate_update_cycles(block);
				}

				uint32_t nextpc;
				if (seqlast->flags & OPFLAG_RETURN_TO_START) /* if we need to return to the start, do it */
					nextpc = pc;
				else /* otherwise we just go to the next instruction */
					nextpc = seqlast->pc + seqlast->length;

				/* if the last instruction can change modes, use a variable mode; otherwise, assume the same mode */
				if (seqlast->next() == nullptr || seqlast->next()->pc != nextpc)
				{
					UML_HASHJMP(block, mode, nextpc, *m_nocode);
				}
			}

			/* end the sequence */
			block.end();
			succeeded = true;
		}
		catch (const drcuml_block::abort_compilation &)
		{
			code_flush_cache();
		}
	}
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

static inline void alloc_handle(drcuml_state &drcuml, uml::code_handle *&handleptr, const char *name)
{
	if (!handleptr)
		handleptr = drcuml.handle_alloc(name);
}



/*-------------------------------------------------
    static_generate_exception - generate an
    exception handler
-------------------------------------------------*/

void hyperstone_device::static_generate_exception(drcuml_block &block, uml::code_label &label)
{
	// add a global entry for this
	alloc_handle(*m_drcuml, m_exception, "exception");
	UML_HANDLE(block, *m_exception);

	UML_GETEXP(block, I0);
	generate_trap_exception_or_int<IS_EXCEPTION>(block, label, uml::I0);
}



void hyperstone_device::static_generate_interrupt_checks(drcuml_block &block, uml::code_label &label)
{
	UML_HANDLE(block, *m_interrupt_checks);

	const int timer_int_pending = label++;
	const int take_int = label++;
	const int take_timer = label++;
	const int dispatch_int = label++;
	const int done_int = label++;

	UML_TEST(block, DRC_SR, L_MASK);
	UML_JMPc(block, uml::COND_NZ, done_int);

	UML_MOV(block, I0, mem(&ISR));

	UML_TEST(block, mem(&m_core->timer_int_pending), 1);
	UML_JMPc(block, uml::COND_NZ, timer_int_pending);

	UML_TEST(block, I0, 0x7f);
	UML_JMPc(block, uml::COND_Z, done_int);

	generate_interrupt_checks(block, label, false, take_int, take_timer);
	UML_JMP(block, done_int);

	UML_LABEL(block, timer_int_pending);
	generate_interrupt_checks(block, label, true, take_int, take_timer);
	UML_JMP(block, done_int);

	UML_LABEL(block, take_int);
	UML_CALLC(block, &c_funcs::standard_irq_callback, this);
	UML_JMP(block, dispatch_int);

	UML_LABEL(block, take_timer);
	UML_MOV(block, mem(&m_core->timer_int_pending), 0);
	UML_MOV(block, I0, TRAPNO_TIMER);

	UML_LABEL(block, dispatch_int);
	generate_trap_exception_or_int<IS_INT>(block, label, uml::I0);

	UML_LABEL(block, done_int);
	UML_RET(block);
}

/*-------------------------------------------------
    generate_entry_helpers - generate helper
    stubs and functions
-------------------------------------------------*/

void hyperstone_device::static_generate_helpers(drcuml_block &block, uml::code_label &label)
{
	// forward references
	alloc_handle(*m_drcuml, m_entry, "entry");
	alloc_handle(*m_drcuml, m_nocode, "nocode");
	alloc_handle(*m_drcuml, m_out_of_cycles, "out_of_cycles");
	alloc_handle(*m_drcuml, m_delay_taken[0], "delay_taken");
	alloc_handle(*m_drcuml, m_delay_taken[1], "delay_taken_s");
	alloc_handle(*m_drcuml, m_delay_taken[2], "delay_taken_t");
	alloc_handle(*m_drcuml, m_delay_taken[3], "delay_taken_st");
	alloc_handle(*m_drcuml, m_interrupt_checks, "int_checks");

	// static entry point
	UML_HANDLE(block, *m_entry);
	//load_fast_iregs(block);
	UML_MOV(block, I0, DRC_SR);
	UML_ROLAND(block, I1, I0, 32 - T_SHIFT + 1, 0x2);
	UML_ROLAND(block, I0, I0, 32 - S_SHIFT, 0x1);
	UML_OR(block, I0, I0, I1);
	UML_HASHJMP(block, I0, DRC_PC, *m_nocode);

	// exception handler for "out of code"
	UML_HANDLE(block, *m_nocode);
	UML_GETEXP(block, I0);
	UML_MOV(block, mem(&PC), I0);
	//save_fast_iregs(block);
	UML_EXIT(block, EXECUTE_MISSING_CODE);

	// out of cycles exception handler
	UML_HANDLE(block, *m_out_of_cycles);
	//save_fast_iregs(block);
	UML_EXIT(block, EXECUTE_OUT_OF_CYCLES);

	// delayed branch taken
	for (int mode = 0; 4 > mode; ++mode)
	{
		UML_HANDLE(block, *m_delay_taken[mode]);
		UML_MOV(block, mem(&m_core->delay_slot_taken), 0);
		generate_update_cycles(block);
		UML_HASHJMP(block, mode, DRC_PC, *m_nocode);
	}

	static_generate_interrupt_checks(block, label);
}


/*------------------------------------------------------------------
    static_generate_memory_accessor
------------------------------------------------------------------*/

void hyperstone_device::static_generate_memory_accessor(int size, int iswrite, bool isio, const char *name, uml::code_handle *&handleptr)
{
	// on entry, address is in I0; data for writes is in I1
	// on exit, read result is in I1

	/* begin generating */
	drcuml_block &block(m_drcuml->begin_block(1024));

	/* add a global entry for this */
	alloc_handle(*m_drcuml, handleptr, name);
	UML_HANDLE(block, *handleptr);

	// write:
	switch (size)
	{
		case 1:
			if (iswrite)
				UML_WRITE(block, I0, I1, SIZE_BYTE, SPACE_PROGRAM);
			else
				UML_READ(block, I1, I0, SIZE_BYTE, SPACE_PROGRAM);
			break;

		case 2:
			if (iswrite)
				UML_WRITE(block, I0, I1, SIZE_WORD, SPACE_PROGRAM);
			else
				UML_READ(block, I1, I0, SIZE_WORD, SPACE_PROGRAM);
			break;

		case 4:
			if (iswrite)
				UML_WRITE(block, I0, I1, SIZE_DWORD, isio ? SPACE_IO : SPACE_PROGRAM);
			else
				UML_READ(block, I1, I0, SIZE_DWORD, isio ? SPACE_IO : SPACE_PROGRAM);
			break;
	}
	UML_RET(block);

	block.end();
}



/***************************************************************************
    CODE GENERATION
***************************************************************************/

void hyperstone_device::generate_interrupt_checks(drcuml_block &block, uml::code_label &labelnum, bool with_timer, int take_int, int take_timer)
{
	UML_MOV(block, I1, mem(&FCR));

	// combine I/O mask and direction in I4 bits 0/4/8
	UML_SHR(block, I2, I1, 2);
	UML_XOR(block, I4, I1, 0xffffffff);
	UML_AND(block, I4, I4, I2);

	const int skip_io3 = labelnum++;
	UML_TEST(block, I0, 0x40);
	UML_JMPc(block, uml::COND_Z, skip_io3);
	UML_TEST(block, I4, 0x100);
	UML_JMPc(block, uml::COND_Z, skip_io3);
	UML_MOV(block, mem(&m_core->arg0), IRQ_IO3);
	UML_MOV(block, I0, TRAPNO_IO3);
	UML_JMP(block, take_int);
	UML_LABEL(block, skip_io3);

	if (with_timer)
	{
		UML_ROLAND(block, I2, I1, 12, 0xb);

		UML_CMP(block, I2, 0x3);
		UML_JMPc(block, uml::COND_E, take_timer);
	}

	// get masked INT state in I3 bits 0-3
	UML_SHR(block, I3, I1, 28);
	UML_XOR(block, I3, I3, 0xffffffff);
	UML_AND(block, I3, I3, I0);

	const int skip_int1 = labelnum++;
	UML_TEST(block, I3, 0x01);
	UML_JMPc(block, uml::COND_Z, skip_int1);
	UML_MOV(block, mem(&m_core->arg0), IRQ_INT1);
	UML_MOV(block, I0, TRAPNO_INT1);
	UML_JMP(block, take_int);
	UML_LABEL(block, skip_int1);

	if (with_timer)
	{
		UML_CMP(block, I2, 0x2);
		UML_JMPc(block, uml::COND_E, take_timer);
	}

	const int skip_int2 = labelnum++;
	UML_TEST(block, I3, 0x02);
	UML_JMPc(block, uml::COND_Z, skip_int2);
	UML_MOV(block, mem(&m_core->arg0), IRQ_INT2);
	UML_MOV(block, I0, TRAPNO_INT2);
	UML_JMP(block, take_int);
	UML_LABEL(block, skip_int2);

	if (with_timer)
	{
		UML_CMP(block, I2, 0x1);
		UML_JMPc(block, uml::COND_E, take_timer);
	}

	const int skip_int3 = labelnum++;
	UML_TEST(block, I3, 0x04);
	UML_JMPc(block, uml::COND_Z, skip_int3);
	UML_MOV(block, mem(&m_core->arg0), IRQ_INT3);
	UML_MOV(block, I0, TRAPNO_INT3);
	UML_JMP(block, take_int);
	UML_LABEL(block, skip_int3);

	if (with_timer)
	{
		UML_CMP(block, I2, 0x0);
		UML_JMPc(block, uml::COND_E, take_timer);
	}

	const int skip_int4 = labelnum++;
	UML_TEST(block, I3, 0x08);
	UML_JMPc(block, uml::COND_Z, skip_int4);
	UML_MOV(block, mem(&m_core->arg0), IRQ_INT4);
	UML_MOV(block, I0, TRAPNO_INT4);
	UML_JMP(block, take_int);
	UML_LABEL(block, skip_int4);

	const int skip_io1 = labelnum++;
	UML_TEST(block, I0, 0x10);
	UML_JMPc(block, uml::COND_Z, skip_io1);
	UML_TEST(block, I4, 0x1);
	UML_JMPc(block, uml::COND_Z, skip_io1);
	UML_MOV(block, mem(&m_core->arg0), IRQ_IO1);
	UML_MOV(block, I0, TRAPNO_IO1);
	UML_JMP(block, take_int);
	UML_LABEL(block, skip_io1);

	const int skip_io2 = labelnum++;
	UML_TEST(block, I0, 0x20);
	UML_JMPc(block, uml::COND_Z, skip_io2);
	UML_TEST(block, I4, 0x10);
	UML_JMPc(block, uml::COND_Z, skip_io1);
	UML_MOV(block, mem(&m_core->arg0), IRQ_IO2);
	UML_MOV(block, I0, TRAPNO_IO2);
	UML_JMP(block, take_int);
	UML_LABEL(block, skip_io2);
}

/*-------------------------------------------------
    generate_update_cycles - generate code to
    subtract cycles from the icount and generate
    an exception if out
-------------------------------------------------*/

void hyperstone_device::generate_update_cycles(drcuml_block &block)
{
	UML_SUB(block, mem(&m_core->icount), mem(&m_core->icount), I7);
	UML_MOV(block, I7, 0);
	UML_CALLHc(block, uml::COND_LE, *m_out_of_cycles);
}

/*-------------------------------------------------
    generate_checksum_block - generate code to
    validate a sequence of opcodes
-------------------------------------------------*/

void hyperstone_device::generate_checksum_block(drcuml_block &block, compiler_state &compiler, const opcode_desc *seqhead, const opcode_desc *seqlast)
{
	const opcode_desc *curdesc;
	if (m_drcuml->logging())
	{
		block.append_comment("[Validation for %08X]", seqhead->pc);
	}

	/* loose verify or single instruction: just compare and fail */
	if (!(m_drcoptions & E132XS_STRICT_VERIFY) || !seqhead->next())
	{
		if (!(seqhead->flags & OPFLAG_VIRTUAL_NOOP))
		{
			uint32_t sum = seqhead->opptr.w[0];
			uint32_t addr = seqhead->physpc;
			const void *base = m_prptr(addr);
			if (!base)
			{
				osd_printf_info("%s: cache read_ptr returned nullptr for address %08x\n", tag(), addr);
				return;
			}

			auto const *delayslot = seqhead->delay.first();
			const void *delaybase = nullptr;
			if (delayslot && (seqhead->physpc != delayslot->physpc))
			{
				delaybase = m_prptr(delayslot->physpc);
				if (!delaybase)
				{
					osd_printf_info("%s: cache read_ptr returned nullptr for address %08x\n", tag(), delayslot->physpc);
				}
			}

			UML_LOAD(block, I0, base, 0, SIZE_WORD, SCALE_x1);

			if (delayslot && (seqhead->physpc != delayslot->physpc))
			{
				UML_LOAD(block, I1, delaybase, 0, SIZE_WORD, SCALE_x1);
				UML_ADD(block, I0, I0, I1);

				sum += delayslot->opptr.w[0];
			}

			UML_CMP(block, I0, sum);
			UML_EXHc(block, COND_NE, *m_nocode, epc(seqhead));
		}
	}
	else /* full verification; sum up everything */
	{
		uint32_t addr = seqhead->physpc;
		const void *base = m_prptr(addr);
		if (!base)
		{
			osd_printf_info("%s: cache read_ptr returned nullptr for address %08x\n", tag(), addr);
			return;
		}

		UML_LOAD(block, I0, base, 0, SIZE_WORD, SCALE_x1);

		uint32_t sum = seqhead->opptr.w[0];
		for (curdesc = seqhead->next(); curdesc != seqlast->next(); curdesc = curdesc->next())
		{
			if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
			{
				addr = curdesc->physpc;
				base = m_prptr(addr);
				assert(base != nullptr);
				UML_LOAD(block, I1, base, 0, SIZE_WORD, SCALE_x1);
				UML_ADD(block, I0, I0, I1);
				sum += curdesc->opptr.w[0];

				if (curdesc->delay.first() != nullptr && (curdesc == seqlast || (curdesc->next() != nullptr && curdesc->next()->physpc != curdesc->delay.first()->physpc)))
				{
					addr = curdesc->delay.first()->physpc;
					base = m_prptr(addr);
					assert(base != nullptr);
					UML_LOAD(block, I1, base, 0, SIZE_WORD, SCALE_x1);
					UML_ADD(block, I0, I0, I1);

					sum += curdesc->delay.first()->opptr.w[0];
				}
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

void hyperstone_device::log_add_disasm_comment(drcuml_block &block, uint32_t pc, uint32_t op)
{
	if (m_drcuml->logging())
	{
		block.append_comment("%08X: %08x", pc, op);
	}
}


/*------------------------------------------------------------------
    generate_branch
------------------------------------------------------------------*/

void hyperstone_device::generate_branch(drcuml_block &block, compiler_state &compiler, uml::parameter mode, uml::parameter targetpc, const opcode_desc *desc)
{
	// clobbers I0 and I1 if mode is BRANCH_TARGET_DYNAMIC

	if (desc)
		UML_ROLINS(block, DRC_SR, ((desc->length >> 1) << ILC_SHIFT) | P_MASK, 0, ILC_MASK | P_MASK);

	generate_update_cycles(block);

	if (desc && (mode == compiler.mode()) && (desc->flags & OPFLAG_INTRABLOCK_BRANCH))
	{
		assert(desc->targetpc != BRANCH_TARGET_DYNAMIC);

		UML_JMP(block, desc->targetpc | 0x80000000);
	}
	else
	{
		// jump through the hash table to the target
		const uml::parameter pc = (targetpc != BRANCH_TARGET_DYNAMIC) ? targetpc : DRC_PC;
		const uml::parameter m = (mode != BRANCH_TARGET_DYNAMIC) ? mode : uml::I0;
		if (mode == BRANCH_TARGET_DYNAMIC)
		{
			UML_MOV(block, I0, DRC_SR);
			UML_ROLAND(block, I1, I0, 32 - T_SHIFT + 1, 0x2);
			UML_ROLAND(block, I0, I0, 32 - S_SHIFT, 0x1);
			UML_OR(block, I0, I0, I1);
		}
		UML_HASHJMP(block, m, pc, *m_nocode);
	}
}


/*-------------------------------------------------
    generate_sequence_instruction - generate code
    for a single instruction in a sequence
-------------------------------------------------*/

void hyperstone_device::generate_sequence_instruction(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	// add an entry for the log
	if (m_drcuml->logging() && !(desc->flags & OPFLAG_VIRTUAL_NOOP))
		log_add_disasm_comment(block, desc->pc, desc->opptr.w[0]);

	// check for pending interrupts
	UML_SUB(block, I0, mem(&m_core->intblock), 1);
	UML_MOVc(block, uml::COND_S, I0, 0);
	UML_MOV(block, mem(&m_core->intblock), I0);
	UML_CALLHc(block, uml::COND_LE, *m_interrupt_checks);

#if E132XS_LOG_DRC_REGS
	UML_CALLC(block, &c_funcs::dump_registers, this);
#endif

	// if we are debugging, call the debugger
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		//save_fast_iregs(block);
		UML_DEBUG(block, desc->pc);
	}

	if (!(desc->flags & OPFLAG_VIRTUAL_NOOP))
	{
		// set the PC map variable
		UML_MAPVAR(block, MAPVAR_PC, compiler.set_pc(desc->pc + desc->length));

		if (compiler.check_delay())
		{
			// if PC is used in a delay instruction, the delayed PC should be used
			const int nodelay = compiler.next_label();
			const int done = compiler.next_label();
			UML_TEST(block, mem(&m_core->delay_slot), 1);
			UML_JMPc(block, uml::COND_Z, nodelay);
			UML_MOV(block, DRC_PC, mem(&m_core->delay_pc));
			UML_MOV(block, mem(&m_core->delay_slot), 0);
			UML_MOV(block, mem(&m_core->delay_slot_taken), 1);
			UML_JMP(block, done);
			UML_LABEL(block, nodelay);
			UML_ADD(block, DRC_PC, DRC_PC, desc->length);
			UML_MOV(block, mem(&m_core->delay_slot_taken), 0);
			UML_LABEL(block, done);
		}
		else
		{
			UML_ADD(block, DRC_PC, DRC_PC, desc->length);
			UML_MOV(block, mem(&m_core->delay_slot_taken), 0);
		}

		// compile the instruction
		if (generate_opcode(block, compiler, desc))
		{
			if (compiler.m_check_delay)
			{
				if (compiler.check_delay())
				{
					UML_TEST(block, mem(&m_core->delay_slot_taken), ~uint32_t(0));
					UML_CALLHc(block, uml::COND_NZ, *m_delay_taken[compiler.mode()]);
				}
				--compiler.m_check_delay;
			}

			if (compiler.trace_mode() && !desc->delayslots)
				UML_EXHc(block, uml::COND_Z, *m_exception, TRAPNO_TRACE_EXCEPTION);
		}
		else
		{
			UML_MOV(block, mem(&m_core->arg0), desc->opptr.w[0]);
			UML_CALLC(block, &c_funcs::unimplemented, this);
		}
	}
}

#include "e132xsdrc_ops.hxx"

bool hyperstone_device::generate_opcode(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	uint32_t op = (uint32_t)desc->opptr.w[0];

	switch (op >> 8)
	{
		case 0x00: generate_chk<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x01: generate_chk<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x02: generate_chk<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x03: generate_chk<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x04: generate_movd<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x05: generate_movd<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x06: generate_movd<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x07: generate_movd<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x08: generate_divsu<GLOBAL, GLOBAL, IS_UNSIGNED>(block, compiler, desc); break;
		case 0x09: generate_divsu<GLOBAL, LOCAL, IS_UNSIGNED>(block, compiler, desc); break;
		case 0x0a: generate_divsu<LOCAL, GLOBAL, IS_UNSIGNED>(block, compiler, desc); break;
		case 0x0b: generate_divsu<LOCAL, LOCAL, IS_UNSIGNED>(block, compiler, desc); break;
		case 0x0c: generate_divsu<GLOBAL, GLOBAL, IS_SIGNED>(block, compiler, desc); break;
		case 0x0d: generate_divsu<GLOBAL, LOCAL, IS_SIGNED>(block, compiler, desc); break;
		case 0x0e: generate_divsu<LOCAL, GLOBAL, IS_SIGNED>(block, compiler, desc); break;
		case 0x0f: generate_divsu<LOCAL, LOCAL, IS_SIGNED>(block, compiler, desc); break;
		case 0x10: generate_xm<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x11: generate_xm<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x12: generate_xm<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x13: generate_xm<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x14: generate_mask<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x15: generate_mask<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x16: generate_mask<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x17: generate_mask<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x18: generate_sum<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x19: generate_sum<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x1a: generate_sum<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x1b: generate_sum<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x1c: generate_sums<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x1d: generate_sums<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x1e: generate_sums<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x1f: generate_sums<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x20: generate_cmp<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x21: generate_cmp<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x22: generate_cmp<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x23: generate_cmp<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x24: generate_mov<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x25: generate_mov<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x26: generate_mov<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x27: generate_mov<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x28: generate_add<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x29: generate_add<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x2a: generate_add<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x2b: generate_add<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x2c: generate_adds<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x2d: generate_adds<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x2e: generate_adds<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x2f: generate_adds<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x30: generate_cmpb<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x31: generate_cmpb<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x32: generate_cmpb<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x33: generate_cmpb<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x34: generate_andn<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x35: generate_andn<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x36: generate_andn<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x37: generate_andn<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x38: generate_or<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x39: generate_or<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x3a: generate_or<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x3b: generate_or<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x3c: generate_xor<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x3d: generate_xor<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x3e: generate_xor<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x3f: generate_xor<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x40: generate_subc<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x41: generate_subc<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x42: generate_subc<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x43: generate_subc<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x44: generate_not<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x45: generate_not<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x46: generate_not<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x47: generate_not<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x48: generate_sub<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x49: generate_sub<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x4a: generate_sub<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x4b: generate_sub<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x4c: generate_subs<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x4d: generate_subs<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x4e: generate_subs<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x4f: generate_subs<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x50: generate_addc<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x51: generate_addc<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x52: generate_addc<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x53: generate_addc<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x54: generate_and<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x55: generate_and<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x56: generate_and<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x57: generate_and<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x58: generate_neg<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x59: generate_neg<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x5a: generate_neg<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x5b: generate_neg<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x5c: generate_negs<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x5d: generate_negs<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x5e: generate_negs<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x5f: generate_negs<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x60: generate_cmpi<GLOBAL, SIMM>(block, compiler, desc); break;
		case 0x61: generate_cmpi<GLOBAL, LIMM>(block, compiler, desc); break;
		case 0x62: generate_cmpi<LOCAL, SIMM>(block, compiler, desc); break;
		case 0x63: generate_cmpi<LOCAL, LIMM>(block, compiler, desc); break;
		case 0x64: generate_movi<GLOBAL, SIMM>(block, compiler, desc); break;
		case 0x65: generate_movi<GLOBAL, LIMM>(block, compiler, desc); break;
		case 0x66: generate_movi<LOCAL, SIMM>(block, compiler, desc); break;
		case 0x67: generate_movi<LOCAL, LIMM>(block, compiler, desc); break;
		case 0x68: generate_addi<GLOBAL, SIMM>(block, compiler, desc); break;
		case 0x69: generate_addi<GLOBAL, LIMM>(block, compiler, desc); break;
		case 0x6a: generate_addi<LOCAL, SIMM>(block, compiler, desc); break;
		case 0x6b: generate_addi<LOCAL, LIMM>(block, compiler, desc); break;
		case 0x6c: generate_addsi<GLOBAL, SIMM>(block, compiler, desc); break;
		case 0x6d: generate_addsi<GLOBAL, LIMM>(block, compiler, desc); break;
		case 0x6e: generate_addsi<LOCAL, SIMM>(block, compiler, desc); break;
		case 0x6f: generate_addsi<LOCAL, LIMM>(block, compiler, desc); break;
		case 0x70: generate_cmpbi<GLOBAL, SIMM>(block, compiler, desc); break;
		case 0x71: generate_cmpbi<GLOBAL, LIMM>(block, compiler, desc); break;
		case 0x72: generate_cmpbi<LOCAL, SIMM>(block, compiler, desc); break;
		case 0x73: generate_cmpbi<LOCAL, LIMM>(block, compiler, desc); break;
		case 0x74: generate_andni<GLOBAL, SIMM>(block, compiler, desc); break;
		case 0x75: generate_andni<GLOBAL, LIMM>(block, compiler, desc); break;
		case 0x76: generate_andni<LOCAL, SIMM>(block, compiler, desc); break;
		case 0x77: generate_andni<LOCAL, LIMM>(block, compiler, desc); break;
		case 0x78: generate_ori<GLOBAL, SIMM>(block, compiler, desc); break;
		case 0x79: generate_ori<GLOBAL, LIMM>(block, compiler, desc); break;
		case 0x7a: generate_ori<LOCAL, SIMM>(block, compiler, desc); break;
		case 0x7b: generate_ori<LOCAL, LIMM>(block, compiler, desc); break;
		case 0x7c: generate_xori<GLOBAL, SIMM>(block, compiler, desc); break;
		case 0x7d: generate_xori<GLOBAL, LIMM>(block, compiler, desc); break;
		case 0x7e: generate_xori<LOCAL, SIMM>(block, compiler, desc); break;
		case 0x7f: generate_xori<LOCAL, LIMM>(block, compiler, desc); break;
		case 0x80: generate_shrdi<N_LO>(block, compiler, desc); break;
		case 0x81: generate_shrdi<N_HI>(block, compiler, desc); break;
		case 0x82: generate_shrd(block, compiler, desc); break;
		case 0x83: generate_shr(block, compiler, desc); break;
		case 0x84: generate_sardi<N_LO>(block, compiler, desc); break;
		case 0x85: generate_sardi<N_HI>(block, compiler, desc); break;
		case 0x86: generate_sard(block, compiler, desc); break;
		case 0x87: generate_sar(block, compiler, desc); break;
		case 0x88: generate_shldi<N_LO>(block, compiler, desc); break;
		case 0x89: generate_shldi<N_HI>(block, compiler, desc); break;
		case 0x8a: generate_shld(block, compiler, desc); break;
		case 0x8b: generate_shl(block, compiler, desc); break;
		case 0x8c: generate_reserved(block, compiler, desc); break;
		case 0x8d: generate_reserved(block, compiler, desc); break;
		case 0x8e: generate_testlz(block, compiler, desc); break;
		case 0x8f: generate_rol(block, compiler, desc); break;
		case 0x90: generate_ldxx1<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x91: generate_ldxx1<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x92: generate_ldxx1<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x93: generate_ldxx1<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x94: generate_ldxx2<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x95: generate_ldxx2<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x96: generate_ldxx2<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x97: generate_ldxx2<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x98: generate_stxx1<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x99: generate_stxx1<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x9a: generate_stxx1<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x9b: generate_stxx1<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0x9c: generate_stxx2<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0x9d: generate_stxx2<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0x9e: generate_stxx2<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0x9f: generate_stxx2<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0xa0: generate_shri<N_LO, GLOBAL>(block, compiler, desc); break;
		case 0xa1: generate_shri<N_HI, GLOBAL>(block, compiler, desc); break;
		case 0xa2: generate_shri<N_LO, LOCAL>(block, compiler, desc); break;
		case 0xa3: generate_shri<N_HI, LOCAL>(block, compiler, desc); break;
		case 0xa4: generate_sari<N_LO, GLOBAL>(block, compiler, desc); break;
		case 0xa5: generate_sari<N_HI, GLOBAL>(block, compiler, desc); break;
		case 0xa6: generate_sari<N_LO, LOCAL>(block, compiler, desc); break;
		case 0xa7: generate_sari<N_HI, LOCAL>(block, compiler, desc); break;
		case 0xa8: generate_shli<N_LO, GLOBAL>(block, compiler, desc); break;
		case 0xa9: generate_shli<N_HI, GLOBAL>(block, compiler, desc); break;
		case 0xaa: generate_shli<N_LO, LOCAL>(block, compiler, desc); break;
		case 0xab: generate_shli<N_HI, LOCAL>(block, compiler, desc); break;
		case 0xac: generate_reserved(block, compiler, desc); break;
		case 0xad: generate_reserved(block, compiler, desc); break;
		case 0xae: generate_reserved(block, compiler, desc); break;
		case 0xaf: generate_reserved(block, compiler, desc); break;
		case 0xb0: generate_mulsu<GLOBAL, GLOBAL, IS_UNSIGNED>(block, compiler, desc); break;
		case 0xb1: generate_mulsu<GLOBAL, LOCAL, IS_UNSIGNED>(block, compiler, desc); break;
		case 0xb2: generate_mulsu<LOCAL, GLOBAL, IS_UNSIGNED>(block, compiler, desc); break;
		case 0xb3: generate_mulsu<LOCAL, LOCAL, IS_UNSIGNED>(block, compiler, desc); break;
		case 0xb4: generate_mulsu<GLOBAL, GLOBAL, IS_SIGNED>(block, compiler, desc); break;
		case 0xb5: generate_mulsu<GLOBAL, LOCAL, IS_SIGNED>(block, compiler, desc); break;
		case 0xb6: generate_mulsu<LOCAL, GLOBAL, IS_SIGNED>(block, compiler, desc); break;
		case 0xb7: generate_mulsu<LOCAL, LOCAL, IS_SIGNED>(block, compiler, desc); break;
		case 0xb8: generate_set<N_LO, GLOBAL>(block, compiler, desc); break;
		case 0xb9: generate_set<N_HI, GLOBAL>(block, compiler, desc); break;
		case 0xba: generate_set<N_LO, LOCAL>(block, compiler, desc); break;
		case 0xbb: generate_set<N_HI, LOCAL>(block, compiler, desc); break;
		case 0xbc: generate_mul<GLOBAL, GLOBAL>(block, compiler, desc); break;
		case 0xbd: generate_mul<GLOBAL, LOCAL>(block, compiler, desc); break;
		case 0xbe: generate_mul<LOCAL, GLOBAL>(block, compiler, desc); break;
		case 0xbf: generate_mul<LOCAL, LOCAL>(block, compiler, desc); break;
		case 0xc0: generate_software(block, compiler, desc); break; // fadd
		case 0xc1: generate_software(block, compiler, desc); break; // faddd
		case 0xc2: generate_software(block, compiler, desc); break; // fsub
		case 0xc3: generate_software(block, compiler, desc); break; // fsubd
		case 0xc4: generate_software(block, compiler, desc); break; // fmul
		case 0xc5: generate_software(block, compiler, desc); break; // fmuld
		case 0xc6: generate_software(block, compiler, desc); break; // fdiv
		case 0xc7: generate_software(block, compiler, desc); break; // fdivd
		case 0xc8: generate_software(block, compiler, desc); break; // fcmp
		case 0xc9: generate_software(block, compiler, desc); break; // fcmpd
		case 0xca: generate_software(block, compiler, desc); break; // fcmpu
		case 0xcb: generate_software(block, compiler, desc); break; // fcmpud
		case 0xcc: generate_software(block, compiler, desc); break; // fcvt
		case 0xcd: generate_software(block, compiler, desc); break; // fcvtd
		case 0xce: generate_extend(block, compiler, desc); break;
		case 0xcf: generate_do(block, compiler, desc); break;
		case 0xd0: generate_ldwr<GLOBAL>(block, compiler, desc); break;
		case 0xd1: generate_ldwr<LOCAL>(block, compiler, desc); break;
		case 0xd2: generate_lddr<GLOBAL>(block, compiler, desc); break;
		case 0xd3: generate_lddr<LOCAL>(block, compiler, desc); break;
		case 0xd4: generate_ldwp<GLOBAL>(block, compiler, desc); break;
		case 0xd5: generate_ldwp<LOCAL>(block, compiler, desc); break;
		case 0xd6: generate_lddp<GLOBAL>(block, compiler, desc); break;
		case 0xd7: generate_lddp<LOCAL>(block, compiler, desc); break;
		case 0xd8: generate_stwr<GLOBAL>(block, compiler, desc); break;
		case 0xd9: generate_stwr<LOCAL>(block, compiler, desc); break;
		case 0xda: generate_stdr<GLOBAL>(block, compiler, desc); break;
		case 0xdb: generate_stdr<LOCAL>(block, compiler, desc); break;
		case 0xdc: generate_stwp<GLOBAL>(block, compiler, desc); break;
		case 0xdd: generate_stwp<LOCAL>(block, compiler, desc); break;
		case 0xde: generate_stdp<GLOBAL>(block, compiler, desc); break;
		case 0xdf: generate_stdp<LOCAL>(block, compiler, desc); break;
		case 0xe0: generate_db<COND_V,  IS_SET>(block, compiler, desc); break;
		case 0xe1: generate_db<COND_V,  IS_CLEAR>(block, compiler, desc); break;
		case 0xe2: generate_db<COND_Z,  IS_SET>(block, compiler, desc); break;
		case 0xe3: generate_db<COND_Z,  IS_CLEAR>(block, compiler, desc); break;
		case 0xe4: generate_db<COND_C,  IS_SET>(block, compiler, desc); break;
		case 0xe5: generate_db<COND_C,  IS_CLEAR>(block, compiler, desc); break;
		case 0xe6: generate_db<COND_CZ, IS_SET>(block, compiler, desc); break;
		case 0xe7: generate_db<COND_CZ, IS_CLEAR>(block, compiler, desc); break;
		case 0xe8: generate_db<COND_N,  IS_SET>(block, compiler, desc); break;
		case 0xe9: generate_db<COND_N,  IS_CLEAR>(block, compiler, desc); break;
		case 0xea: generate_db<COND_NZ, IS_SET>(block, compiler, desc); break;
		case 0xeb: generate_db<COND_NZ, IS_CLEAR>(block, compiler, desc); break;
		case 0xec: generate_dbr(block, compiler, desc); break;
		case 0xed: generate_frame(block, compiler, desc); break;
		case 0xee: generate_call<GLOBAL>(block, compiler, desc); break;
		case 0xef: generate_call<LOCAL>(block, compiler, desc); break;
		case 0xf0: generate_b<COND_V,  IS_SET>(block, compiler, desc); break;
		case 0xf1: generate_b<COND_V,  IS_CLEAR>(block, compiler, desc); break;
		case 0xf2: generate_b<COND_Z,  IS_SET>(block, compiler, desc); break;
		case 0xf3: generate_b<COND_Z,  IS_CLEAR>(block, compiler, desc); break;
		case 0xf4: generate_b<COND_C,  IS_SET>(block, compiler, desc); break;
		case 0xf5: generate_b<COND_C,  IS_CLEAR>(block, compiler, desc); break;
		case 0xf6: generate_b<COND_CZ, IS_SET>(block, compiler, desc); break;
		case 0xf7: generate_b<COND_CZ, IS_CLEAR>(block, compiler, desc); break;
		case 0xf8: generate_b<COND_N,  IS_SET>(block, compiler, desc); break;
		case 0xf9: generate_b<COND_N,  IS_CLEAR>(block, compiler, desc); break;
		case 0xfa: generate_b<COND_NZ, IS_SET>(block, compiler, desc); break;
		case 0xfb: generate_b<COND_NZ, IS_CLEAR>(block, compiler, desc); break;
		case 0xfc: generate_br(block, compiler, desc); break;
		case 0xfd: generate_trap_op(block, compiler, desc); break;
		case 0xfe: generate_trap_op(block, compiler, desc); break;
		case 0xff: generate_trap_op(block, compiler, desc); break;
	}

	UML_ROLINS(block, DRC_SR, ((desc->length >> 1) << ILC_SHIFT) | P_MASK, 0, ILC_MASK | P_MASK);

	return true;
}

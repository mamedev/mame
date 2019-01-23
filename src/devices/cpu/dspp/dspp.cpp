// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    dspp.c

    Core implementation for the portable 3DO (M2) DSPP emulator.
    DSPP = Don's Super Performing Processor

***************************************************************************/

#include "emu.h"
#include "dspp.h"
#include "dsppfe.h"
#include "dsppdasm.h"

#include "debugger.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum
{
	DSPP_PC = 1,
	DSPP_ACC,
	DSPP_FLAGS,
	DSPP_CLOCK,
};


// DRC
#define SINGLE_INSTRUCTION_MODE         (0)


#define CACHE_SIZE  (4 * 1024 * 1024)

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

//**************************************************************************
//  INTERNAL MEMORY MAPS
//**************************************************************************

void dspp_device::code_map(address_map &map)
{
	map(0x000, 0x3ff).ram();
}

void dspp_device::data_map(address_map &map)
{
	map(0x000, 0x2df).ram();
	map(0x2f0, 0x2f1).r(FUNC(dspp_device::input_r));
	map(0x2e0, 0x2e7).w(FUNC(dspp_device::output_w));
	map(0x300, 0x37f).rw(FUNC(dspp_device::fifo_osc_r), FUNC(dspp_device::fifo_osc_w));
	map(0x3bc, 0x3bc).nopr(); // ?
	map(0x3d6, 0x3d6).w(FUNC(dspp_device::input_control_w));
	map(0x3d7, 0x3d7).w(FUNC(dspp_device::output_control_w));
	map(0x3de, 0x3de).r(FUNC(dspp_device::input_status_r));
	map(0x3df, 0x3df).r(FUNC(dspp_device::output_status_r));
	map(0x3e6, 0x3e6).w(FUNC(dspp_device::cpu_int_w));
	map(0x3ee, 0x3ee).rw(FUNC(dspp_device::pc_r), FUNC(dspp_device::pc_w));
	map(0x3f6, 0x3f6).rw(FUNC(dspp_device::audlock_r), FUNC(dspp_device::audlock_w));
	map(0x3f7, 0x3f7).rw(FUNC(dspp_device::clock_r), FUNC(dspp_device::clock_w));
	map(0x3ff, 0x3ff).r(FUNC(dspp_device::noise_r));
}



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

DEFINE_DEVICE_TYPE(DSPP, dspp_device, "dspp", "3DO DSPP")

//-------------------------------------------------
//  dspp_device - constructor
//-------------------------------------------------

dspp_device::dspp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dspp_device(mconfig, DSPP, tag, owner, clock, address_map_constructor(FUNC(dspp_device::code_map), this),
		address_map_constructor(FUNC(dspp_device::data_map), this))
{
}

dspp_device::dspp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor code_map_ctor, address_map_constructor data_map_ctor)
	: cpu_device(mconfig, type, tag, owner, clock),
		m_int_handler(*this),
		m_dma_read_handler(*this),
		m_dma_write_handler(*this),
		m_code_config("code", ENDIANNESS_BIG, 16, 10, -1, code_map_ctor),
		m_data_config("data", ENDIANNESS_BIG, 16, 10, -1, data_map_ctor),
		m_code(nullptr),
		m_data(nullptr),
		m_output_fifo_start(0),
		m_output_fifo_count(0),
		m_dspx_reset(0),
		m_dspx_int_enable(0),
		m_dspx_channel_enable(0),
		m_dspx_channel_complete(0),
		m_dspx_channel_direction(0),
		m_dspx_channel_8bit(0),
		m_dspx_channel_sqxd(0),
		m_dspx_shadow_current_addr(0),
		m_dspx_shadow_current_count(0),
		m_dspx_shadow_next_addr(0),
		m_dspx_shadow_next_count(0),
		m_dspx_dmanext_int(0),
		m_dspx_dmanext_enable(0),
		m_dspx_consumed_int(0),
		m_dspx_consumed_enable(0),
		m_dspx_underover_int(0),
		m_dspx_underover_enable(0),
		m_dspx_audio_time(0),
		m_dspx_audio_duration(0),
		m_cache(CACHE_SIZE),
		m_drcuml(nullptr),
		m_drcfe(nullptr),
		m_drcoptions(0)
{
#if 0
	memset(m_core->m_stack, 0, sizeof(m_core->m_stack));
	memset(m_core->m_rbase, 0, sizeof(m_core->m_rbase));
	memset(m_outputs, 0, sizeof(m_outputs));
	memset(m_output_fifo, 0, sizeof(m_output_fifo));
	memset(m_fifo_dma, 0, sizeof(m_fifo_dma));
#endif
}


//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void dspp_device::device_start()
{
	m_isdrc = false;//allow_drc();

	m_core = (dspp_internal_state *)m_cache.alloc_near(sizeof(dspp_internal_state));
	memset(m_core, 0, sizeof(dspp_internal_state));

	uint32_t flags = 0;
	m_drcuml = std::make_unique<drcuml_state>(*this, m_cache, flags, 1, 16, 0);

	m_drcfe = std::make_unique<dspp_frontend>(this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE);

	// Resolve our callbacks
	m_int_handler.resolve_safe();
	m_dma_read_handler.resolve_safe(0);
	m_dma_write_handler.resolve_safe();

	// Get our address spaces
	m_code = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	auto code_cache = m_code->cache<1, -1, ENDIANNESS_BIG>();
	m_code_cache = code_cache;
	m_code16 = [code_cache](offs_t address) -> uint16_t { return code_cache->read_word(address); };
	m_codeptr = [code_cache](offs_t address) -> const void * { return code_cache->read_ptr(address); };

	// Register our state for the debugger
	state_add(DSPP_PC,         "PC",        m_core->m_pc);
	state_add(DSPP_ACC,        "ACC",       m_core->m_acc);
	state_add(STATE_GENPC,     "GENPC",     m_core->m_pc).noshow();
#if 0
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_core->m_flags).callimport().callexport().formatstr("%6s").noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_ppc).noshow();
	state_add(STATE_GENSP,     "GENSP",     m_src2val[REGBASE + 31]).noshow();

	state_add(DSPP_PS,         "PS",        m_core->m_flagsio).callimport().callexport();
	for (int regnum = 0; regnum < 32; regnum++)
		state_add(DSPP_R0 + regnum, tempstr.format("R%d", regnum), m_src2val[REGBASE + regnum]);
#endif

	// Register our state for saving
	save_item(NAME(m_core->m_pc));
	save_item(NAME(m_core->m_stack));
	save_item(NAME(m_core->m_stack_ptr));
	save_item(NAME(m_core->m_rbase));
	save_item(NAME(m_core->m_acc));
	save_item(NAME(m_core->m_tclock));

	save_item(NAME(m_core->m_flag_carry));
	save_item(NAME(m_core->m_flag_zero));
	save_item(NAME(m_core->m_flag_neg));
	save_item(NAME(m_core->m_flag_over));
	save_item(NAME(m_core->m_flag_exact));
	save_item(NAME(m_core->m_flag_audlock));
	save_item(NAME(m_core->m_flag_sleep));

	save_item(NAME(m_outputs));
	save_item(NAME(m_output_fifo_start));
	save_item(NAME(m_output_fifo_count));

	for (uint32_t i = 0; i < NUM_DMA_CHANNELS; ++i)
	{
		save_item(NAME(m_fifo_dma[i].m_current_addr), i);
		save_item(NAME(m_fifo_dma[i].m_current_count), i);
		save_item(NAME(m_fifo_dma[i].m_next_addr), i);
		save_item(NAME(m_fifo_dma[i].m_next_count), i);
		save_item(NAME(m_fifo_dma[i].m_prev_value), i);
		save_item(NAME(m_fifo_dma[i].m_prev_current), i);
		save_item(NAME(m_fifo_dma[i].m_go_forever), i);
		save_item(NAME(m_fifo_dma[i].m_next_valid), i);
		save_item(NAME(m_fifo_dma[i].m_reserved), i);
		save_item(NAME(m_fifo_dma[i].m_fifo), i);
		save_item(NAME(m_fifo_dma[i].m_dma_ptr), i);
		save_item(NAME(m_fifo_dma[i].m_dspi_ptr), i);
		save_item(NAME(m_fifo_dma[i].m_depth), i);
	}

	save_item(NAME(m_last_frame_clock));
	save_item(NAME(m_last_osc_count));
	save_item(NAME(m_osc_phase));
	save_item(NAME(m_osc_freq));

	save_item(NAME(m_core->m_partial_int));

	save_item(NAME(m_core->m_dspx_control));
	save_item(NAME(m_dspx_reset));
	save_item(NAME(m_dspx_int_enable));
	save_item(NAME(m_dspx_channel_enable));
	save_item(NAME(m_dspx_channel_complete));
	save_item(NAME(m_dspx_channel_direction));
	save_item(NAME(m_dspx_channel_8bit));
	save_item(NAME(m_dspx_channel_sqxd));
	save_item(NAME(m_dspx_shadow_current_addr));
	save_item(NAME(m_dspx_shadow_current_count));
	save_item(NAME(m_dspx_shadow_next_addr));
	save_item(NAME(m_dspx_shadow_next_count));
	save_item(NAME(m_dspx_dmanext_int));
	save_item(NAME(m_dspx_dmanext_enable));
	save_item(NAME(m_dspx_consumed_int));
	save_item(NAME(m_dspx_consumed_enable));
	save_item(NAME(m_dspx_underover_int));
	save_item(NAME(m_dspx_underover_enable));
	save_item(NAME(m_dspx_audio_time));
	save_item(NAME(m_dspx_audio_duration));

	// Set our instruction counter
	set_icountptr(m_core->m_icount);

	m_cache_dirty = true;
}


//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void dspp_device::device_reset()
{
	// initialize the state
	m_core->m_pc = 0;
	m_core->m_stack_ptr = 0;
	m_output_fifo_start = 0;
	m_output_fifo_count = 0;

	m_core->m_flag_audlock = 0;
	m_core->m_flag_sleep = 0;
	m_core->m_stack_ptr = 0;
	m_core->m_writeback = ~1; // TODO
	set_rbase(0, 0);

	// TODO: CLEAR DMA CHANNELS

	// Clear interrupts
	m_core->m_partial_int = 0;
	update_host_interrupt();

	m_cache_dirty = true;
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the CPU's address spaces
//-------------------------------------------------

device_memory_interface::space_config_vector dspp_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_code_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}


//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void dspp_device::state_import(const device_state_entry &entry)
{

}


//-------------------------------------------------
//  state_export - export state from the device,
//  to a known location where it can be read
//-------------------------------------------------

void dspp_device::state_export(const device_state_entry &entry)
{

}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void dspp_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c.%c%c%c%c%c",
							m_core->m_flag_audlock  ? 'A' : '.',
							m_core->m_flag_sleep    ? 'S' : '.',
							m_core->m_flag_carry    ? 'C' : '.',
							m_core->m_flag_zero     ? 'Z' : '.',
							m_core->m_flag_neg      ? 'N' : '.',
							m_core->m_flag_over     ? 'V' : '.',
							m_core->m_flag_exact    ? 'E' : '.');
			break;
	}
}

std::unique_ptr<util::disasm_interface> dspp_device::create_disassembler()
{
	return std::make_unique<dspp_disassembler>();
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  update_pc -
//-------------------------------------------------

inline void dspp_device::update_pc()
{
	++m_core->m_pc;
}

inline void dspp_device::update_ticks()
{
	--m_core->m_tclock;
	--m_core->m_icount;
}


//-------------------------------------------------
//  readop - Read an opcode at the given address
//-------------------------------------------------

uint16_t dspp_device::read_op(offs_t pc)
{
	return m_code_cache->read_word(pc);
}


//-------------------------------------------------
//  read_data - Read a word from the data space
//-------------------------------------------------

inline uint16_t dspp_device::read_data(offs_t addr)
{
	return m_data->read_word(addr);
}


//-------------------------------------------------
//  write_data - Write a word to the data space
//-------------------------------------------------

inline void dspp_device::write_data(offs_t addr, uint16_t data)
{
	m_data->write_word(addr, data);
}


//-------------------------------------------------
//  get_interrupt_state -
//-------------------------------------------------

uint32_t dspp_device::get_interrupt_state()
{
	uint32_t host_int = m_core->m_partial_int;

	if (m_dspx_dmanext_int & m_dspx_dmanext_enable)
		host_int |= DSPX_F_INT_DMANEXT;

	if (m_dspx_consumed_int & m_dspx_consumed_enable)
		host_int |= DSPX_F_INT_CONSUMED;

	return host_int;
}


//-------------------------------------------------
//  update_host_interrupt -
//-------------------------------------------------

void dspp_device::update_host_interrupt()
{
	// TODO: Underflow/overflow interrupts
	m_int_handler(get_interrupt_state() & m_dspx_int_enable ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  parse_operand - Parse instruction operands
//-------------------------------------------------

void dspp_device::parse_operands(uint32_t numops)
{
	uint32_t addr, val = 0xBAD;
	uint32_t opidx = 0;
	uint32_t operand = 0;
	uint32_t numregs = 0;

	for (uint32_t i = 0; i < MAX_OPERANDS; ++i)
	{
		// Reset operands
		m_core->m_operands[i].value = -1;
		m_core->m_operands[i].addr = -1;
	}

	// Reset global op index
	m_core->m_opidx = 0;

	while (opidx < numops)
	{
		operand = read_op(m_core->m_pc);
		update_pc();
		update_ticks();

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
				m_core->m_operands[opidx++].value = val;
			}
			else if((operand & 0xe000) == 0x8000)
			{
				// Address operand
				addr = operand & 0x03ff;

				if (operand & 0x0400)
				{
					 // Indirect
					addr = read_data(addr);
				}
				m_core->m_operands[opidx].addr = addr;

				if (operand & 0x0800)
				{
					// Write Back
					m_core->m_writeback = addr;
				}
				opidx++;
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
			uint32_t shifter, regdi;

			// Shift successive register operands from a single operand word
			for (uint32_t i = 0; i < numregs; ++i)
			{
				shifter = ((numregs - i) - 1) * 5;
				regdi = (operand >> shifter) & 0x1f;
				addr = translate_reg(regdi & 0xf);

				if (regdi & 0x0010)
				{
					 // Indirect?
					addr = read_data(addr);
				}

				if (numregs == 2)
				{
					if ((i == 0) && (operand & 0x1000))
						m_core->m_writeback = addr;
					else if ((i == 1) && (operand & 0x0800))
						m_core->m_writeback = addr;
				}
				else if (numregs == 1)
				{
					if (operand & 0x800)
						m_core->m_writeback = addr;
				}

				m_core->m_operands[opidx++].addr = addr;
			}
			numregs = 0;
		}
	}
}


//-------------------------------------------------
//  read_next_operand - Return the value encoded by
//  the next operand
//-------------------------------------------------

uint16_t dspp_device::read_next_operand()
{
	int32_t value = m_core->m_operands[m_core->m_opidx].value;
	//if (m_core->m_op == 0x46a0) printf("Value is %08x\n", value);

	if (value < 0)
	{
		value = read_data(m_core->m_operands[m_core->m_opidx].addr);
		//if (m_core->m_op == 0x46a0) printf("New value is %08x from %08x\n", value, m_core->m_operands[m_core->m_opidx].addr);
	}

	// Next operand
	++m_core->m_opidx;

	return value;
}


//-------------------------------------------------
//  write_next_operand - Write to the address
//  encoded by the next operand
//-------------------------------------------------

void dspp_device::write_next_operand(uint16_t value)
{
	int32_t addr = m_core->m_operands[m_core->m_opidx].addr;

	assert(addr != -1);

	write_data(addr, value);

	// Advance to the next operand
	++m_core->m_opidx;
}


//-------------------------------------------------
//  push_pc - Push program counter onto the stack
//-------------------------------------------------

inline void dspp_device::push_pc()
{
	if (m_core->m_stack_ptr < PC_STACK_DEPTH)
		m_core->m_stack[m_core->m_stack_ptr++] = m_core->m_pc;
	else
		fatalerror("DSPP stack overflow!");
}


//-------------------------------------------------
//  pop_pc - Pop program counter from the stack
//-------------------------------------------------

inline uint16_t dspp_device::pop_pc()
{
	if (m_core->m_stack_ptr == 0)
		fatalerror("DSPP stack underflow!");

	return m_core->m_stack[--m_core->m_stack_ptr];
}


//-------------------------------------------------
//  set_rbase - Set register address base
//-------------------------------------------------

inline void dspp_device::set_rbase(uint32_t base, uint32_t addr)
{
	switch (base)
	{
		case 4:
			m_core->m_rbase[1] = addr + 4 - base;
			break;
		case 0:
			m_core->m_rbase[0] = addr;
			m_core->m_rbase[1] = addr + 4 - base;
		// Intentional fall-through
		case 8:
			m_core->m_rbase[2] = addr + 8 - base;

		case 12:
			m_core->m_rbase[3] = addr + 12 - base;
			break;
	}
}


//-------------------------------------------------
//  translate_reg - Translate register address
//-------------------------------------------------

inline uint16_t dspp_device::translate_reg(uint16_t reg)
{
	uint32_t base = (reg >> 2) & 3;
	return m_core->m_rbase[base] + reg - (reg & ~3);
}



//**************************************************************************
//  CORE EXECUTION
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t dspp_device::execute_min_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t dspp_device::execute_max_cycles() const
{
	return 5; // TODO ?
}


//-------------------------------------------------
//  execute_run - core execution loop
//-------------------------------------------------

void dspp_device::execute_run()
{
	if (m_isdrc)
	{
		// Only run if enabled
		do
		{
			if (m_core->m_dspx_control & DSPX_CONTROL_GWILLING)
			{
				execute_run_drc();
			}
			else
			{
				update_ticks();
				update_fifo_dma();
			}
		} while (m_core->m_icount > 0);
		return;
	}

	bool check_debugger = ((device_t::machine().debug_flags & DEBUG_FLAG_ENABLED) != 0);

	do
	{
		update_ticks();
		update_fifo_dma();

		// Only run if enabled
		if (m_core->m_dspx_control & DSPX_CONTROL_GWILLING)
		{
			if (check_debugger)
				debugger_instruction_hook(m_core->m_pc);

			m_core->m_op = read_op(m_core->m_pc);
			//printf("%04x: %04x\n", (uint16_t)m_core->m_pc, (uint16_t)m_core->m_op);
			update_pc();

			// Decode and execute
			if (m_core->m_op & 0x8000)
				exec_control();
			else
				exec_arithmetic();
		}

	} while (m_core->m_icount > 0);
}



//**************************************************************************
//  OPCODE IMPLEMENTATIONS
//**************************************************************************

//-------------------------------------------------
//  exec_super_special - Execute a super special
//  control op
//-------------------------------------------------

inline void dspp_device::exec_super_special()
{
	uint32_t sel = (m_core->m_op >> 7) & 7;

	switch (sel)
	{
		case 1: // BAC
		{
			m_core->m_pc = m_core->m_acc >> 4;
			break;
		}
		case 4: // RTS
		{
			m_core->m_pc = pop_pc();
			break;
		}
		case 5: // OP_MASK
		{
			// TODO
			break;
		}

		case 7: // SLEEP
		{
			// TODO: How does sleep work?
			--m_core->m_pc;
			m_core->m_flag_sleep = 1;
			break;
		}

		case 0: // NOP
		case 2: // Unused
		case 3:
		case 6:
			break;
	}
}


//-------------------------------------------------
//  exec_special - Execute a special control op
//-------------------------------------------------

inline void dspp_device::exec_special()
{
	switch ((m_core->m_op >> 10) & 7)
	{
		case 0:
		{
			exec_super_special();
			break;
		}
		case 1: // JUMP
		{
			m_core->m_pc = m_core->m_op & 0x3ff;
			break;
		}
		case 2: // JSR
		{
			push_pc();
			m_core->m_pc = m_core->m_op & 0x3ff;
			break;
		}
		case 3: // BFM
		{
			break;
		}
		case 4: // MOVEREG
		{
			uint32_t regdi = m_core->m_op & 0x3f;
			uint32_t addr = translate_reg(regdi & 0xf);

			// Indirect
			if (regdi & 0x0010)
			{
				addr = read_data(addr);
			}

			parse_operands(1);
			write_data(addr, read_next_operand());
			break;
		}
		case 5: // RBASE
		{
			set_rbase((m_core->m_op & 3) << 2, m_core->m_op & 0x3fc);
			break;
		}
		case 6: // MOVED
		{
			parse_operands(1);
			write_data(m_core->m_op & 0x3ff, read_next_operand());
			break;
		}
		case 7: // MOVEI
		{
			parse_operands(1);
			uint32_t addr = read_data(m_core->m_op & 0x3ff);
			write_data(addr, read_next_operand());
			break;
		}

		default:
			break;
	}
}


//-------------------------------------------------
//  exec_branch - Execute a branch control op
//-------------------------------------------------

void dspp_device::exec_branch()
{
	uint32_t mode = (m_core->m_op >> 13) & 3;
	uint32_t select = (m_core->m_op >> 12) & 1;
	uint32_t mask = (m_core->m_op >> 10) & 3;

	bool flag0, flag1;

	if (select == 0)
	{
		flag0 = m_core->m_flag_neg;
		flag1 = m_core->m_flag_over;
	}
	else
	{
		flag0 = m_core->m_flag_carry;
		flag1 = m_core->m_flag_zero;
	}

	bool mask0 = (mask & 2) != 0;
	bool mask1 = (mask & 1) != 0;

	bool branch = (flag0 || !mask0) && (flag1 || !mask1);

	if (mode == 2)
		branch = !branch;

	//printf("Branch: %d %d %d %d %d\n", branch ? 1 : 0, flag0 ? 1 : 0, mask0 ? 1 : 0, flag1 ? 1 : 0, mask1 ? 1 : 0);

	if (branch)
		m_core->m_pc = m_core->m_op & 0x3ff;
}


//-------------------------------------------------
//  exec_complex_branch - Execute a complex branch
//  control op
//-------------------------------------------------

inline void dspp_device::exec_complex_branch()
{
	uint32_t type = (m_core->m_op >> 10) & 7;

	const bool c = m_core->m_flag_carry;
	const bool z = m_core->m_flag_zero;
	const bool n = m_core->m_flag_neg;
	const bool v = m_core->m_flag_over;
	const bool x = m_core->m_flag_exact;

	bool branch = false;

	switch (type)
	{
		case 0: // BLT
			branch = (n && !v) || (!n && v);
			break;
		case 1: // BLE
			branch = ((n && !v) || (!n && v)) || z;
			break;
		case 2: // BGE
			branch = ((n && v) || (!n && !v));
			break;
		case 3: // BGT
			branch = ((n && v) || (!n && !v)) && !z;
			break;
		case 4: // BHI
			branch = c && !z;
			break;
		case 5: // BLS
			branch = !c || z;
			break;
		case 6: // BXS
			branch = x;
			break;
		case 7: // BXC
			branch = !x;
			break;
	}

	if (branch)
		m_core->m_pc = m_core->m_op & 0x3ff;
}


//-------------------------------------------------
//  exec_control - Execute a control op
//-------------------------------------------------

inline void dspp_device::exec_control()
{
	uint32_t mode = (m_core->m_op >> 13) & 3;

	switch (mode)
	{
		// Special
		case 0:
		{
			exec_special();
			break;
		}

		// Branches
		case 1: case 2:
		{
			exec_branch();
			break;
		}

		// Complex branches
		case 3:
		{
			exec_complex_branch();
			break;
		}

		default:
			fatalerror("Invalid DSPP control instruction mode");
			break;
	}
}

//-------------------------------------------------
//  sign_extend8 - Sign extend 8-bits to 32-bits
//-------------------------------------------------

static inline int32_t sign_extend8(uint8_t val)
{
	return (int32_t)(int8_t)val;
}


//-------------------------------------------------
//  sign_extend16 - Sign extend 16-bits to 32-bits
//-------------------------------------------------

static inline int32_t sign_extend16(uint16_t val)
{
	return (int32_t)(int16_t)val;
}


//-------------------------------------------------
//  sign_extend20 - Sign extend 20-bits to 32-bits
//-------------------------------------------------

static inline int32_t sign_extend20(uint32_t val)
{
	if (val & 0x00080000)
		return (int32_t)(0xfff00000 | val);
	else
		return (int32_t)val;
}


//-------------------------------------------------
//  exec_arithmetic - Execute an arithmetic op
//-------------------------------------------------

inline void dspp_device::exec_arithmetic()
{
	// Decode the various fields
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
	}

	int32_t alu_a, alu_b;

	//if (m_core->m_op == 0x46a0)
		//printf("Arithmetic: numops:%d, muxa:%d, muxb:%d, alu_op:%d, barrel_code:%d\n", numops, muxa, muxb, alu_op, barrel_code);

	switch (muxa)
	{
		case 0:
		{
			alu_a = m_core->m_acc;
			break;
		}
		case 1: case 2:
		{
			alu_a = read_next_operand() << 4;
			//if (m_core->m_op == 0x46a0)
				//printf("Arithmetic: Next operand: %04x\n", alu_a >> 4);
			break;
		}
		case 3:
		{
			alu_a = mul_res;
			break;
		}
	}

	switch (muxb)
	{
		case 0:
		{
			alu_b = m_core->m_acc;
			break;
		}
		case 1: case 2:
		{
			alu_b = read_next_operand() << 4;
			break;
		}
		case 3:
		{
			alu_b = mul_res;
			break;
		}
	}

	// For carry detection apparently
	alu_a &= 0x00fffff;
	alu_b &= 0x00fffff;

	switch (alu_op)
	{
		case 0: // _TRA
		{
			alu_res = alu_a;
			m_core->m_flag_over = 0;
			m_core->m_flag_carry = 0;
			break;
		}
		case 1: // _NEG
		{
			alu_res = -alu_b;
			m_core->m_flag_over = 0;
			m_core->m_flag_carry = 0;
			break;
		}
		case 2: // _+
		{
			alu_res = alu_a + alu_b;
			m_core->m_flag_over = (((alu_a & 0x80000) == (alu_b & 0x80000) && (alu_a & 0x80000) != (alu_res & 0x80000)));
			m_core->m_flag_carry = (alu_res & 0x00100000) != 0;
			break;
		}
		case 3: // _+C
		{
			alu_res = alu_a + (m_core->m_flag_carry << 4);
			m_core->m_flag_over = 0;
			m_core->m_flag_carry = (alu_res & 0x00100000) != 0;
			break;
		}
		case 4: // _-
		{
			alu_res = alu_a - alu_b;
			m_core->m_flag_over = ((alu_a & 0x80000) == (~alu_b & 0x80000) && (alu_a & 0x80000) != (alu_res & 0x80000));
			m_core->m_flag_carry = (alu_res & 0x00100000) != 0;
			break;
		}
		case 5: // _-B
		{
			alu_res = alu_a - (m_core->m_flag_carry << 4);
			m_core->m_flag_over = 0;
			m_core->m_flag_carry = (alu_res & 0x00100000) != 0;
			break;
		}
		case 6: // _++
		{
			alu_res = alu_a + 1;
			m_core->m_flag_over = !(alu_a & 0x80000) && (alu_res & 0x80000);
			m_core->m_flag_carry = 0;
			break;
		}
		case 7: // _--
		{
			alu_res = alu_a - 1;
			m_core->m_flag_over = (alu_a & 0x80000) && !(alu_res & 0x80000);
			m_core->m_flag_carry = 0;
			break;
		}
		case 8: // _TRL
		{
			alu_res = alu_a;
			m_core->m_flag_over = 0;
			m_core->m_flag_carry = 0;
			break;
		}
		case 9: // _NOT
		{
			alu_res = ~alu_a;
			m_core->m_flag_over = 0;
			m_core->m_flag_carry = 0;
			break;
		}
		case 10: // _AND
		{
			alu_res = alu_a & alu_b;
			m_core->m_flag_over = 0;
			m_core->m_flag_carry = 0;
			break;
		}
		case 11: // _NAND
		{
			alu_res = ~(alu_a & alu_b);
			m_core->m_flag_over = 0;
			m_core->m_flag_carry = 0;
			break;
		}
		case 12: // _OR
		{
			alu_res = alu_a | alu_b;
			m_core->m_flag_over = 0;
			m_core->m_flag_carry = 0;
			break;
		}
		case 13: // _NOR
		{
			alu_res = ~(alu_a | alu_b);
			m_core->m_flag_over = 0;
			m_core->m_flag_carry = 0;
			break;
		}
		case 14: // _XOR
		{
			alu_res = alu_a ^ alu_b;
			m_core->m_flag_over = 0;
			m_core->m_flag_carry = 0;
			break;
		}
		case 15: // _XNOR
		{
			alu_res = ~(alu_a ^ alu_b);
			m_core->m_flag_over = 0;
			m_core->m_flag_carry = 0;
			break;
		}
	}

	m_core->m_flag_neg = (alu_res & 0x00080000) != 0;
	m_core->m_flag_zero = (alu_res & 0x000ffff0) == 0;
	m_core->m_flag_exact = (alu_res & 0x0000000f) == 0;

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
			m_core->m_acc = sign_extend20(alu_res) >> shift;
		}
		else
		{
			// Logical
			m_core->m_acc = (alu_res & 0xfffff) >> shift;
		}

	}
	else
	{
		// Left shift
		uint32_t shift = shifts[barrel_code];

		if (shift == 16)
		{
			// Clip and saturate
			if (m_core->m_flag_over)
				m_core->m_acc = m_core->m_flag_neg ? 0x7ffff : 0xfff80000;
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
}



//**************************************************************************
//  FIFO DMA
//**************************************************************************

//-------------------------------------------------
//  write_dma_to_fifo -
//-------------------------------------------------

void dspp_device::write_dma_to_fifo(int32_t channel, int16_t value)
{
	fifo_dma &dma = m_fifo_dma[channel];

	dma.m_fifo[dma.m_dma_ptr] = value;

	if (dma.m_depth < DMA_FIFO_DEPTH)
	{
		dma.m_dma_ptr = (dma.m_dma_ptr + 1) & DMA_FIFO_MASK;
		dma.m_depth += 1;
	}
	else
	{
		fatalerror("DMA TO FIFO OVERFLOW");
	}
}


//-------------------------------------------------
//  write_dspp_to_fifo -
//-------------------------------------------------

void dspp_device::write_dspp_to_fifo(int32_t channel, int16_t value)
{
	fifo_dma &dma = m_fifo_dma[channel];

	dma.m_fifo[dma.m_dspi_ptr] = value;

	if (dma.m_depth < DMA_FIFO_DEPTH)
	{
		dma.m_dspi_ptr = (dma.m_dspi_ptr + 1) & DMA_FIFO_MASK;
		dma.m_depth += 1;
	}
	else
	{
		fatalerror("DSPP TO FIFO OVERFLOW");
	}
}


//-------------------------------------------------
//  read_fifo_to_dspp -
//-------------------------------------------------

int16_t dspp_device::read_fifo_to_dspp(int32_t channel)
{
	int16_t data = 0;

	fifo_dma &dma = m_fifo_dma[channel];

	if (dma.m_depth > 0)
	{
		data = dma.m_fifo[dma.m_dspi_ptr];

		dma.m_dspi_ptr = (dma.m_dspi_ptr + 1) & DMA_FIFO_MASK;
		--dma.m_depth;

		if (dma.m_depth == 0)
		{
			// Set consumed interrupt
			if (m_dspx_channel_complete & (1 << channel))
			{
				m_dspx_consumed_int |= 1 << channel;
				update_host_interrupt();
			}
		}

		dma.m_prev_current = data;
	}
	else
	{
		// TODO: Is this right?
		m_dspx_underover_int |= 1 << channel;
		update_host_interrupt();
		data = dma.m_prev_current;
	}

	return data;
}


//-------------------------------------------------
//  read_fifo_to_dma -
//-------------------------------------------------

int16_t dspp_device::read_fifo_to_dma(int32_t channel)
{
	fifo_dma &dma = m_fifo_dma[channel];

	uint32_t data = dma.m_fifo[dma.m_dma_ptr];

	if (dma.m_depth > 0)
	{
		dma.m_dma_ptr = (dma.m_dma_ptr + 1) & DMA_FIFO_MASK;
		dma.m_depth -= 1;
	}
	else
	{
		//fatalerror("FIFO TO DMA UNDERFLOW");
	}
	return data;
}


//-------------------------------------------------
//  run_oscillator -
//-------------------------------------------------

void dspp_device::run_oscillator(int32_t channel)
{
	fifo_dma &dma = m_fifo_dma[channel];

	// Add phase increment
	m_osc_phase += m_osc_freq;

	// Extract two high bits to advance FIFO
	uint32_t count = (m_osc_phase >> 15) & 3;

	// Clip to positive phase
	m_osc_phase &= 0x7fff;

	// Advance FIFO if data present
	if (count > dma.m_depth)
		count = dma.m_depth;

	for (uint32_t i = 0; i < count; ++i)
		read_fifo_to_dspp(channel);

	// Return count to program for counting samples
	m_last_osc_count = count;
}


//-------------------------------------------------
//  advance_audio_timer -
//-------------------------------------------------

void dspp_device::advance_audio_timer()
{
	// Advance time on each frame count
	++m_dspx_audio_time;

	// Interrupt on transition from 0 to 0xFFFF
	if (--m_dspx_audio_duration == 0xffff)
	{
		m_core->m_partial_int |= DSPX_F_INT_TIMER;
		update_host_interrupt();
	}
}


//-------------------------------------------------
//  advance_audio_frame -
//-------------------------------------------------

void dspp_device::advance_audio_frame()
{
	m_last_frame_clock = m_clock;
	advance_audio_timer();

	if (m_core->m_flag_audlock)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  process_next_dma -
//-------------------------------------------------

void dspp_device::process_next_dma(int32_t channel)
{
	fifo_dma &dma = m_fifo_dma[channel];

	if (dma.m_current_count <= 0)
	{
		uint32_t chmask = 1 << channel;

		if (dma.m_next_valid)
		{
			dma.m_current_addr = dma.m_next_addr;
			dma.m_current_count = dma.m_next_count;

			if (!dma.m_go_forever)
				dma.m_next_valid = 0;

			// Set completion bit
			m_dspx_channel_complete &= ~chmask;
			m_dspx_dmanext_int |= chmask;
			update_host_interrupt();
		}
		else
		{
			// Disable the channel so we don't keep servicing it
			m_dspx_channel_enable &= ~chmask;
		}
	}
}


//-------------------------------------------------
//  decode_sqxd - Decompress an SQXD coded sample
//-------------------------------------------------

int16_t dspp_device::decode_sqxd(int8_t data, int16_t prev)
{
	int16_t temp = sign_extend8(data & 0xfe);
	int32_t expanded = (temp * iabs(temp)) << 1;
	int16_t output;

	if (data & 1)
	{
		expanded = expanded >> 2;
		output = expanded + prev;
	}
	else
	{
		output = expanded;
	}

	return output;
}


//-------------------------------------------------
//  service_output_dma -
//-------------------------------------------------

void dspp_device::service_output_dma(int32_t channel)
{
	fifo_dma & dma = m_fifo_dma[channel];

	if (dma.m_current_count == 0)
		return;

	// Transfer a maximum of 4 samples per tick
	uint32_t count = dma.m_current_count;

	if (count > 4)
		count = 4;

	for (uint32_t i = 0; i < count; ++i)
	{
		uint16_t sample = read_fifo_to_dma(channel);

		m_dma_write_handler(dma.m_current_addr++, sample >> 8);
		m_dma_write_handler(dma.m_current_addr++, sample & 0xff);
	}

	dma.m_current_count -= count;

	process_next_dma(channel);
}


//-------------------------------------------------
//  service_input_dma -
//-------------------------------------------------

void dspp_device::service_input_dma(int32_t channel)
{
	fifo_dma &dma = m_fifo_dma[channel];

	if (dma.m_current_count == 0)
		return;

	// Transfer a maximum of 4 samples per tick
	uint32_t count = dma.m_current_count;

	if (count > 4)
		count = 4;

	// Determine sample format
	bool is8bit = (m_dspx_channel_8bit & (1 << channel)) != 0;
	bool isSQXD = (m_dspx_channel_sqxd & (1 << channel)) != 0;

	if (is8bit)
	{
		// Fetch data from memory, {decompress}, and write to FIFO
		for (uint32_t i = 0; i < count; ++i)
		{
			int16_t sample;
			int8_t curbyte = m_dma_read_handler(dma.m_current_addr++);

			if (isSQXD)
			{
				printf("SQXD NOT TESTED!");

				sample = decode_sqxd(curbyte, dma.m_prev_value);
				dma.m_prev_value = sample;
			}
			else
			{
				sample = curbyte << 8;
			}

			write_dma_to_fifo(channel, sample);
		}
	}
	else
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			int16_t sample;

			sample = m_dma_read_handler(dma.m_current_addr++) << 8;
			sample |= m_dma_read_handler(dma.m_current_addr++);

			write_dma_to_fifo(channel, sample);
		}
	}

	dma.m_current_count -= count;

	process_next_dma(channel);
}


//-------------------------------------------------
//  update_fifo_dma -
//-------------------------------------------------

void dspp_device::update_fifo_dma()
{
	uint32_t mask = m_dspx_channel_enable & ~m_dspx_channel_complete;

	while (mask != 0)
	{
		uint32_t channel = 31 - count_leading_zeros(mask);

		const fifo_dma & dma = m_fifo_dma[channel];

		if (m_dspx_channel_direction & (1 << channel))
		{
			if (dma.m_depth >= 4)
			{
				service_output_dma(channel);
				break;
			}
		}
		else
		{
			if (dma.m_depth <= 4)
			{
				service_input_dma(channel);
				break;
			}
		}

		mask &= (1 << channel) - 1;
	}
}


//-------------------------------------------------
//  reset_channel
//-------------------------------------------------

void dspp_device::reset_channel(int32_t channel)
{
	fifo_dma &dma = m_fifo_dma[channel];

	m_dspx_channel_complete &= ~(1 << channel);

	dma.m_dma_ptr = 0;
	dma.m_dspi_ptr = 0;
	dma.m_depth = 0;
}



//**************************************************************************
//  INTERNAL REGISTERS
//**************************************************************************

//-------------------------------------------------
//  input_r - Read digital input
//-------------------------------------------------

READ16_MEMBER( dspp_device::input_r )
{
	// TODO
	return 0;
}


//-------------------------------------------------
//  output_w - Write to the 8 output registers
//-------------------------------------------------

WRITE16_MEMBER( dspp_device::output_w )
{
	m_outputs[offset] = data;
}


//-------------------------------------------------
//  fifo_osc_r -
//-------------------------------------------------

READ16_MEMBER( dspp_device::fifo_osc_r )
{
	uint32_t data = 0;
	uint32_t channel = offset / 8;

	fifo_dma &dma = m_fifo_dma[channel];

	switch (offset & 7)
	{
		// DSPI_FIFO_OSC_OFFSET_CURRENT
		case 0:
		{
			if (dma.m_depth == 0)
				data = dma.m_prev_current;
			else
				data = dma.m_fifo[dma.m_dspi_ptr];

			break;
		}

		// DSPI_FIFO_OSC_OFFSET_NEXT
		case 1:
		{
			if (dma.m_depth == 0)
				data = dma.m_prev_current;
			else if (dma.m_depth == 1)
				data = dma.m_fifo[dma.m_dspi_ptr];
			else
				data = dma.m_fifo[(dma.m_dspi_ptr + 1) & DMA_FIFO_MASK];

			break;
		}

		// DSPI_FIFO_OSC_OFFSET_FREQUENCY
		case 2:
		{
			data = m_last_osc_count;
			break;
		}

		// DSPI_FIFO_OSC_OFFSET_PHASE
		case 3:
		{
			data = m_osc_phase;
			break;
		}

		// DSPI_FIFO_OFFSET_DATA
		case 4:
		{
			data = read_fifo_to_dspp(channel);
			break;
		}

		// DSPI_FIFO_OFFSET_CONTROL
		case 5:
		{
			data = dma.m_depth;
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  fifo_osc_w -
//-------------------------------------------------

WRITE16_MEMBER( dspp_device::fifo_osc_w )
{
	uint32_t channel = offset / 8;

	switch (offset & 7)
	{
		// DSPI_FIFO_OSC_OFFSET_CURRENT
		case 0:
		{
			// Read only
			break;
		}

		// DSPI_FIFO_OSC_OFFSET_NEXT
		case 1:
		{
			// Read only
			break;
		}

		// DSPI_FIFO_OSC_OFFSET_FREQUENCY
		case 2:
		{
			m_osc_freq = data;
			run_oscillator(channel);
			break;
		}

		// DSPI_FIFO_OSC_OFFSET_PHASE
		case 3:
		{
			m_osc_phase = data;
			break;
		}

		// DSPI_FIFO_OFFSET_DATA
		case 4:
		{
			write_dspp_to_fifo(channel, data);
			break;
		}

		// DSPI_FIFO_OFFSET_CONTROL
		case 5:
		{
			// Read only
			break;
		}
	}
}


//-------------------------------------------------
//  input_control_w -
//-------------------------------------------------

WRITE16_MEMBER( dspp_device::input_control_w )
{
	// TODO
}


//-------------------------------------------------
//  output_control_w -
//-------------------------------------------------

WRITE16_MEMBER( dspp_device::output_control_w )
{
	// TODO
	if (data & 1)
	{
		uint32_t end;

		if (m_output_fifo_count == OUTPUT_FIFO_DEPTH)
		{
			// Overflow
			end = (m_output_fifo_start + m_output_fifo_count) & OUTPUT_FIFO_MASK;

			m_output_fifo_start = (m_output_fifo_start + 2) & OUTPUT_FIFO_MASK;

			m_output_fifo[(end + 0) & OUTPUT_FIFO_MASK] = m_outputs[0];
			m_output_fifo[(end + 1) & OUTPUT_FIFO_MASK] = m_outputs[1];
		}
		else
		{
			end = (m_output_fifo_start + m_output_fifo_count) & OUTPUT_FIFO_MASK;

			m_output_fifo[(end + 0) & OUTPUT_FIFO_MASK] = m_outputs[0];
			m_output_fifo[(end + 1) & OUTPUT_FIFO_MASK] = m_outputs[1];

			// Advance and update FIFO status
			m_output_fifo_count += 2;
		}

		advance_audio_frame();
	}
}


//-------------------------------------------------
//  input_status_r - Read input state
//-------------------------------------------------

READ16_MEMBER( dspp_device::input_status_r )
{
	// TODO: How should this work?
	return 1;
}


//-------------------------------------------------
//  output_status_r - Return number of unread
//  entries in the output FIFO
//-------------------------------------------------

READ16_MEMBER( dspp_device::output_status_r )
{
	return m_output_fifo_count;
}


//-------------------------------------------------
//  cpu_int_w - Host CPU soft interrupt
//-------------------------------------------------

WRITE16_MEMBER( dspp_device::cpu_int_w )
{
	m_core->m_partial_int |= (data << DSPX_FLD_INT_SOFT_SHIFT) & DSPX_FLD_INT_SOFT_MASK;
	update_host_interrupt();
}


//-------------------------------------------------
//  pc_r - Read program counter
//-------------------------------------------------

READ16_MEMBER( dspp_device::pc_r )
{
	return m_core->m_pc;
}


//-------------------------------------------------
//  pc_w - Write program counter
//-------------------------------------------------

WRITE16_MEMBER(dspp_device:: pc_w )
{
	m_core->m_pc = data;
}


//-------------------------------------------------
//  audlock_r - Read Audio Lock status
//-------------------------------------------------

READ16_MEMBER( dspp_device::audlock_r )
{
	return m_core->m_flag_audlock;
}


//-------------------------------------------------
//  audlock_w - Write Audio Lock status
//-------------------------------------------------

WRITE16_MEMBER( dspp_device::audlock_w )
{
	m_core->m_flag_audlock = data & 1;
}


//-------------------------------------------------
//  clock_r - Read CPU tick counter
//-------------------------------------------------

READ16_MEMBER( dspp_device::clock_r )
{
	return m_core->m_tclock;
}


//-------------------------------------------------
//  clock_w - Write CPU tick counter
//-------------------------------------------------

WRITE16_MEMBER( dspp_device::clock_w )
{
	m_core->m_tclock = data;
}


//-------------------------------------------------
//  noise_r - PRNG noise
//-------------------------------------------------

READ16_MEMBER( dspp_device::noise_r )
{
	// TODO: Obviously this isn't accurate
	return machine().rand();
}



//**************************************************************************
//  EXTERNAL INTERFACE AND CONTROL REGISTERS
//**************************************************************************

//-------------------------------------------------
//  read_ext_control -
//-------------------------------------------------

uint32_t dspp_device::read_ext_control(offs_t offset)
{
	uint32_t data = 0;

	switch (offset)
	{
		// DSPX_INTERRUPT_SET
		case 0x4000/4:
		// DSPX_INTERRUPT_CLR
		case 0x4004/4:
		{
			data = get_interrupt_state();
			break;
		}

		// DSPX_INTERRUPT_ENABLE
		case 0x4008/4:
		// DSPX_INTERRUPT_DISABLE
		case 0x400C/4:
		{
			data = m_dspx_int_enable;
			break;
		}
		// DSPX_INT_DMANEXT_SET
		case 0x4010/4:
		// DSPX_INT_DMANEXT_CLR
		case 0x4014/4:
		{
			data = m_dspx_dmanext_int;
			break;
		}
		// DSPX_INT_DMANEXT_ENABLE
		case 0x4018/4:
		{
			data = m_dspx_dmanext_enable;
			break;
		}
		// DSPX_INT_CONSUMED_SET
		case 0x4020/4:
		// DSPX_INT_CONSUMED_CLR
		case 0x4024/4:
		{
			data = m_dspx_consumed_int;
			break;
		}
		// DSPX_INT_CONSUMED_ENABLE
		case 0x4028/4:
		// DSPX_INT_CONSUMED_DISABLE
		case 0x402c/4:
		{
			data = m_dspx_consumed_enable;
			break;
		}

		// DSPX_INT_UNDEROVER_SET
		case 0x4030/4:
		// DSPX_INT_UNDEROVER_CLR
		case 0x4034/4:
		{
			data = m_dspx_underover_int;
			break;
		}

		// DSPX_INT_UNDEROVER_ENABLE
		case 0x4038/4:
		// DSPX_INT_UNDEROVER_DISABLE
		case 0x403c/4:
		{
			data = m_dspx_underover_enable;
			break;
		}

		// DSPX_CHANNEL_ENABLE
		case 0x6000/4:
		// DSPX_CHANNEL_DISABLE
		case 0x6004/4:
		{
			data = m_dspx_channel_enable;
			break;
		}
		// DSPX_CHANNEL_DIRECTION_SET
		case 0x6008/4:
		// DSPX_CHANNEL_DIRECTION_CLR
		case 0x600c/4:
		{
			data = m_dspx_channel_direction;
			break;
		}
		// DSPX_CHANNEL_8BIT_SET
		case 0x6010/4:
		// DSPX_CHANNEL_8BIT_CLR
		case 0x6014/4:
		{
			data = m_dspx_channel_8bit;
			break;
		}
		// DSPX_CHANNEL_SQXD_SET
		case 0x6018/4:
		// DSPX_CHANNEL_SQXD_CLR
		case 0x601c/4:
		{
			data = m_dspx_channel_sqxd;
			break;
		}
		// DSPX_CHANNEL_STATUS
		case 0x603c/4:
		{
			data = m_dspx_channel_complete;
			break;
		}

		// DSPX_FRAME_DOWN_COUNTER:
		case 0x6040/4:
		{
			data = m_dspx_audio_duration;
			break;
		}
		// DSPX_FRAME_UP_COUNTER:
		case 0x6044/4:
		{
			data = m_dspx_audio_time;
			break;
		}

		// AUDIO_CONFIG
		case 0x6050/4:
		{
			break;
		}
		// AUDIN_CONFIG
		case 0x6060/4:
		{
			break;
		}
		// AUDOUT_CONFIG
		case 0x6068/4:
		{
			break;
		}
		// DSPX_CONTROL
		case 0x6070/4:
		{
			data = m_core->m_dspx_control;
			break;
		}
		default:
		{
			printf("DSPP: Unhandled external control read (%.4x)\n", offset << 2);
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  write_ext_control -
//-------------------------------------------------

void dspp_device::write_ext_control(offs_t offset, uint32_t data)
{
	switch (offset)
	{
		// DSPX_INTERRUPT_SET
		case 0x4000/4:
		{
			m_core->m_partial_int |= data & ~DSPX_F_INT_ALL_DMA;
			update_host_interrupt();
			break;
		}
		// DSPX_INTERRUPT_CLR
		case 0x4004/4:
		{
			m_core->m_partial_int &= ~(data & ~DSPX_F_INT_ALL_DMA);
			update_host_interrupt();
			break;
		}
		// DSPX_INTERRUPT_ENABLE
		case 0x4008/4:
		{
			m_dspx_int_enable |= data;
			update_host_interrupt();
			break;
		}
		// DSPX_INTERRUPT_DISABLE
		case 0x400C/4:
		{
			m_dspx_int_enable &= ~data;
			update_host_interrupt();
			break;
		}
		// DSPX_INT_DMANEXT_SET
		case 0x4010/4:
		{
			m_dspx_dmanext_int |= data;
			update_host_interrupt();
			break;
		}
		// DSPX_INT_DMANEXT_CLR
		case 0x4014/4:
		{
			m_dspx_dmanext_int &= ~data;
			update_host_interrupt();
			break;
		}
		// DSPX_INT_DMANEXT_ENABLE
		case 0x4018/4:
		{
			m_dspx_dmanext_enable |= data;
			update_host_interrupt();
			break;
		}
		// DSPX_INT_CONSUMED_SET
		case 0x4020/4:
		{
			m_dspx_consumed_int |= data;
			update_host_interrupt();
			break;
		}
		// DSPX_INT_CONSUMED_CLR
		case 0x4024/4:
		{
			m_dspx_consumed_int &= ~data;
			update_host_interrupt();
			break;
		}
		// DSPX_INT_CONSUMED_ENABLE
		case 0x4028/4:
		{
			m_dspx_consumed_enable |= data;
			update_host_interrupt();
			break;
		}
		// DSPX_INT_CONSUMED_DISABLE
		case 0x402c/4:
		{
			m_dspx_consumed_enable &= ~data;
			update_host_interrupt();
			break;
		}

		// DSPX_INT_UNDEROVER_SET
		case 0x4030/4:
		{
			m_dspx_underover_int |= data;
			update_host_interrupt();
			break;
		}
		// DSPX_INT_UNDEROVER_CLR
		case 0x4034/4:
		{
			m_dspx_underover_int &= ~data;
			update_host_interrupt();
			break;
		}
		// DSPX_INT_UNDEROVER_ENABLE
		case 0x4038/4:
		{
			m_dspx_underover_enable |= data;
			update_host_interrupt();
			break;
		}
		// DSPX_INT_UNDEROVER_DISABLE
		case 0x403c/4:
		{
			m_dspx_underover_enable &= ~data;
			update_host_interrupt();
			break;
		}

		// DSPX_CHANNEL_ENABLE
		case 0x6000/4:
		{
			m_dspx_channel_enable |= data;
			update_host_interrupt();
			break;
		}
		// DSPX_CHANNEL_DISABLE
		case 0x6004/4:
		{
			m_dspx_channel_enable &= ~data;
			update_host_interrupt();
			break;
		}
		// DSPX_CHANNEL_DIRECTION_SET
		case 0x6008/4:
		{
			m_dspx_channel_direction |= data;
			break;
		}
		// DSPX_CHANNEL_DIRECTION_CLR
		case 0x600c/4:
		{
			m_dspx_channel_direction &= ~data;
			break;
		}
		// DSPX_CHANNEL_8BIT_SET
		case 0x6010/4:
		{
			m_dspx_channel_8bit |= data;
			break;
		}
		// DSPX_CHANNEL_8BIT_CLR
		case 0x6014/4:
		{
			m_dspx_channel_8bit &= ~data;
			break;
		}
		// DSPX_CHANNEL_SQXD_SET
		case 0x6018/4:
		{
			m_dspx_channel_sqxd |= data;
			break;
		}
		// DSPX_CHANNEL_SQXD_CLR
		case 0x601c/4:
		{
			m_dspx_channel_sqxd &= ~data;
			break;
		}
		// DSPX_CHANNEL_RESET
		case 0x6030/4:
		{
			for (uint32_t i = 0; i < NUM_DMA_CHANNELS; ++i)
			{
				if (data & (1 << i))
					reset_channel(i);
			}
			break;
		}

		// DSPX_FRAME_DOWN_COUNTER:
		case 0x6040/4:
		{
			m_dspx_audio_duration = data;
			break;
		}
		// DSPX_FRAME_UP_COUNTER:
		case 0x6044/4:
		{
			m_dspx_audio_time = data;
			break;
		}

		// AUDIO_CONFIG
		case 0x6050/4:
		{
			break;
		}

		// AUDIN_CONFIG
		case 0x6060/4:
		{
			break;
		}

		// AUDOUT_CONFIG
		case 0x6068/4:
		{
			break;
		}

		// DSPX_CONTROL
		case 0x6070/4:
		{
			m_core->m_dspx_control = data;
			break;
		}

		// DSPX_RESET
		case 0x6074/4:
		{
			if (data & 1)
				device_reset();

			// TODO: DSPX_F_RESET_INPUT and DSPX_F_RESET_OUTPUT

			break;
		}
		default:
		{
			printf("DSPP: Unhandled external control write (%.4x with %.8x)\n", offset << 2, data);
			break;
		}
	}
}


//-------------------------------------------------
//  read - host CPU read from DSPP internals
//-------------------------------------------------

READ32_MEMBER( dspp_device::read )
{
	if (offset < 0x1000/4)
	{
		// 16-bit code memory
		return m_code->read_word(offset);
	}
	else if (offset >= 0x1000/4 && offset < 0x2000/4)
	{
		// 16-bit data memory and registers
		return m_data->read_word((offset - 0x1000/4));
	}
	else if(offset >= 0x5000/4 && offset < 0x6000/4)
	{
		// DMA registers
		return read_dma_stack(offset - 0x5000/4);
	}
	else
	{
		// 32-bit control registers
		return read_ext_control(offset);
	}
}


//-------------------------------------------------
//  read_dma_stack -
//-------------------------------------------------

uint32_t dspp_device::read_dma_stack(offs_t offset)
{
	uint32_t data = 0;

	if (offset < 0x200 / 4)
	{
		uint32_t channel = offset / (16/4);
		uint32_t reg = offset & 3;
		fifo_dma &dma = m_fifo_dma[channel];

		switch (reg)
		{
			case 0x00/4://DSPX_DMA_ADDR_OFFSET:
			{
				data = dma.m_current_addr;
				break;
			}
			case 0x04/4://DSPX_DMA_COUNT_OFFSET:
			{
				data = dma.m_current_count;
				break;
			}
			case 0x08/4://DSPX_DMA_NEXT_ADDR_OFFSET:
			{
				data = dma.m_next_addr;
				break;
			}
			case 0x0c/4://DSPX_DMA_NEXT_COUNT_OFFSET:
			{
				data = dma.m_next_count;
				break;
			}
		}
	}
	else if (offset >= 0x200/4 && offset < (0x200/4 + (NUM_DMA_CHANNELS * (16/4))))
	{
		uint32_t channel = (offset - 0x200/4) / (16/4);
		fifo_dma &dma = m_fifo_dma[channel];

		data = dma.m_go_forever ? DSPX_F_DMA_GO_FOREVER : 0;
		data |= dma.m_next_valid ? DSPX_F_DMA_NEXTVALID : 0;
	}
	else
	{
		fatalerror("Unhandled DMA stack read");
	}

	return data;
}


//-------------------------------------------------
//  write_dma_stack -
//-------------------------------------------------

void dspp_device::write_dma_stack(offs_t offset, uint32_t data)
{
	if (offset < 0x200 / 4)
	{
		switch (offset & 3)
		{
			case 0x00/4://DSPX_DMA_ADDR_OFFSET:
			{
				m_dspx_shadow_current_addr = data;
				break;
			}
			case 0x04/4://DSPX_DMA_COUNT_OFFSET:
			{
				m_dspx_shadow_current_count = data;
				break;
			}
			case 0x08/4://DSPX_DMA_NEXT_ADDR_OFFSET:
			{
				m_dspx_shadow_next_addr = data;
				break;
			}
			case 0x0c/4://DSPX_DMA_NEXT_COUNT_OFFSET:
			{
				m_dspx_shadow_next_count = data;
				break;
			}
		}
	}
	else if (offset >= 0x200/4 && offset < (0x200/4 + (NUM_DMA_CHANNELS * (16/4))))
	{
		uint32_t channel = (offset - 0x200/4) / (16/4);
		fifo_dma &dma = m_fifo_dma[channel];

		if (data & DSPX_F_SHADOW_SET_NEXTVALID)
		{
			dma.m_next_valid = (data & DSPX_F_DMA_NEXTVALID) != 0;
		}
		if (data & DSPX_F_SHADOW_SET_FOREVER)
		{
			dma.m_go_forever = (data & DSPX_F_DMA_GO_FOREVER) != 0;
		}
		if (data & DSPX_F_SHADOW_SET_DMANEXT)
		{
			if (data & DSPX_F_INT_DMANEXT_EN)
			{
				m_dspx_dmanext_enable |= (1 << channel);
			}
			else
			{
				m_dspx_dmanext_enable &= ~(1 << channel);
			}
		}
		if (data & DSPX_F_SHADOW_SET_ADDRESS_COUNT)
		{
			if (offset & (8/4))
			{
				dma.m_next_addr = m_dspx_shadow_next_addr;
				dma.m_next_count = m_dspx_shadow_next_count;
			}
			else
			{
				dma.m_current_addr = m_dspx_shadow_current_addr;
				dma.m_current_count = m_dspx_shadow_current_count;
			}
		}
	}
	else
	{
		fatalerror("Unhandled DMA stack write");
	}
}


//-------------------------------------------------
//  write - host CPU write to DSPP internals
//-------------------------------------------------

WRITE32_MEMBER( dspp_device::write )
{
	if (offset < 0x1000/4)
	{
		// 16-bit code memory
		m_code->write_word(offset, data);
	}
	else if (offset >= 0x1000/4 && offset < 0x2000/4)
	{
		// 16-bit data memory and registers
		m_data->write_word((offset - 0x1000/4), data);
	}
	else if(offset >= 0x5000/4 && offset < 0x6000/4)
	{
		// DMA registers
		write_dma_stack(offset - 0x5000/4, data);

		// Better safe than sorry...
		machine().scheduler().synchronize();
	}
	else
	{
		// 32-bit control registers
		write_ext_control(offset, data);

		// Better safe than sorry...
		machine().scheduler().synchronize();
	}
}


//-------------------------------------------------
//  read_output_fifo - Get data for the DACs
//-------------------------------------------------

uint16_t dspp_device::read_output_fifo()
{
	uint16_t data = 0;

	if (m_output_fifo_count == 0)
	{
		// Underflow
		return m_output_fifo[m_output_fifo_start];
	}

	data = m_output_fifo[m_output_fifo_start];

	m_output_fifo_start = (m_output_fifo_start + 1) & OUTPUT_FIFO_MASK;
	--m_output_fifo_count;

	return data;
}

// DEBUG!

char * GetBinary(char * buffer, uint32_t val, uint32_t bits)
{
	uint32_t i;

	for (i = 0; i < bits; ++i)
		buffer[i] = (val >> (bits - 1 - i)) & 1 ? '1' : '0';

	buffer[i] = '\0';

	return buffer;
}

void dspp_device::dump_state()
{
	// DMA
	for (uint32_t i = 0; i < NUM_DMA_CHANNELS; ++i)
	{
		printf("\n=== CHANNEL %02X ===\n", i);
		printf("CURR_ADDRESS: %08X\n", m_fifo_dma[i].m_current_addr);
		printf("CURR_COUNT:   %08X\n", m_fifo_dma[i].m_current_count);
		printf("NEXT_ADDR:    %08X\n", m_fifo_dma[i].m_next_addr);
		printf("NEXT_COUNT:   %08X\n", m_fifo_dma[i].m_next_count);
		printf("PREV_VALUE:   %08X\n", m_fifo_dma[i].m_prev_value);
		printf("PREV_CURRENT: %08X\n", m_fifo_dma[i].m_prev_current);
		printf("GO_FOREVER:   %X\n", m_fifo_dma[i].m_go_forever);
		printf("NEXT_VALID:   %X\n", m_fifo_dma[i].m_next_valid);
	}

	char buffer[64];

	printf("\n=== GLOBAL REGISTER===\n");
	printf("DSPX_CONTROL:           %08X\n", m_core->m_dspx_control);
	printf("DSPX_RESET:             %08X\n", m_dspx_reset);
	printf("DSPX_INT_ENABLE:        %08X\n", m_dspx_int_enable);
	printf("DSPX_CHANNEL_ENABLE:    %08X %s\n", m_dspx_channel_enable, GetBinary(buffer, m_dspx_channel_enable, 32));
	printf("DSPX_CHANNEL_COMPLETE:  %08X %s\n", m_dspx_channel_complete, GetBinary(buffer, m_dspx_channel_complete, 32));
	printf("DSPX_CHANNEL_DIRECTION: %08X %s\n", m_dspx_channel_direction, GetBinary(buffer, m_dspx_channel_direction, 32));
	printf("DSPX_CHANNEL_8BIT:      %08X %s\n", m_dspx_channel_8bit, GetBinary(buffer, m_dspx_channel_8bit, 32));
	printf("DSPX_CHANNEL_SQXD:      %08X %s\n", m_dspx_channel_sqxd, GetBinary(buffer, m_dspx_channel_sqxd, 32));

#if 0
	uint32_t    m_dspx_shadow_current_addr;
	uint32_t    m_dspx_shadow_current_count;
	uint32_t    m_dspx_shadow_next_addr;
	uint32_t    m_dspx_shadow_next_count;
	uint32_t    m_dspx_dmanext_int;
	uint32_t    m_dspx_dmanext_enable;
	uint32_t    m_dspx_consumed_int;
	uint32_t    m_dspx_consumed_enable;
	uint32_t    m_dspx_underover_int;
	uint32_t    m_dspx_underover_enable;
	uint32_t    m_dspx_audio_time;
	uint16_t    m_dspx_audio_duration;
#endif
}

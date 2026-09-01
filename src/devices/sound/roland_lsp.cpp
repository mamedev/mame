// license:BSD-3-Clause
// copyright-holders:superctr
// thanks-to: giulioz
/*
 *  Roland LSP (Fujitsu MB87837)
 *
 *  Effect processor of the Boss ME-6 and, as the insertion-effect DSP, of the Roland
 *  SC-88Pro and successors. A 384-instruction program runs once per sample -
 *  the clock is 2 x 384 x Fs and the program is the sample period, so the chip
 *  is a CPU with a fixed schedule rather than a streaming sound device: the host PCM
 *  chip writes the serial inputs, runs one pass and reads the outputs back.
 *
 *  Host window: sixteen byte registers, a 24-bit data latch committed by the write of
 *  the address low byte, a read latch loaded by the read address, and a configuration
 *  word that starts and halts the program. The address space is 128 words of internal
 *  RAM below the 384 program words, and the firmware patches individual program words
 *  while the program runs - the coefficient bytes are the effect's parameters and there
 *  are no parameter registers.
 *
 *  Datapath: two 24-bit accumulators over the internal RAM, which is a delay line whose
 *  pointer steps back once per sample, so an instruction's 7-bit offset is a fixed
 *  delay. A store writes the accumulator as it stood three slots earlier, saturating or
 *  merely narrowed depending on the store field, and it lands before the same slot reads
 *  its operand. Long delays live in an external DRAM that steps back the same way; an
 *  access carries no address of its own, but assembles one from the three-bit external
 *  RAM field of the six slots following an opener eight slots before a write and twelve
 *  before a read, and it keeps twenty of a word's twenty-four bits. Stereo is by program
 *  position: the first half of the program is one channel and the second half the other.
 *
 *  In the SC-88Pro, the XP is the pump, so the device is halted as far as the scheduler
 *  is concerned and execute_run() serves the debugger; run_once() is one sample.
 *
 *  TODO:
 *  - the DRAM row/column walk, modelled here as a flat ring
 *  - what raises INT, and the busy bit's timing
 *  - the serial word width the configuration word selects
 *  - whether the program can write the program area
 */
#include "emu.h"
#include "roland_lsp.h"

#include "roland_lspd.h"

#include <algorithm>

#define LOG_HOST    (1U << 1)
#define LOG_CONFIG  (1U << 2)

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(ROLAND_LSP, roland_lsp_device, "roland_lsp", "Roland LSP")

namespace {

// offsets 1 to 4 are not memory reads but a power-of-two operand, so the coefficient is an immediate
constexpr s32 IMMEDIATE[5] = { 0, 1 << 7, 1 << 12, 1 << 17, 1 << 22 };

constexpr bool is_immediate(int offset) { return offset >= 1 && offset <= 4; }

} // anonymous namespace

roland_lsp_device::roland_lsp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, ROLAND_LSP, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 32, 9, -2, address_map_constructor(FUNC(roland_lsp_device::program_map), this))
{
}

void roland_lsp_device::program_map(address_map &map)
{
	map(0x000, 0x1ff).rw(FUNC(roland_lsp_device::internal_r), FUNC(roland_lsp_device::internal_w));
}

device_memory_interface::space_config_vector roland_lsp_device::memory_space_config() const
{
	return space_config_vector { std::make_pair(AS_PROGRAM, &m_program_config) };
}

std::unique_ptr<util::disasm_interface> roland_lsp_device::create_disassembler()
{
	return std::make_unique<roland_lsp_disassembler>();
}

void roland_lsp_device::device_start()
{
	m_eram = make_unique_clear<s32[]>(ERAM_SIZE);
	set_icountptr(m_icount);

	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(0, "PC", m_pc);
	state_add(1, "ACCA", m_acc[0]);
	state_add(2, "ACCB", m_acc[1]);
	state_add(3, "BUF", m_buffer_pos);
	state_add(4, "ERAM", m_eram_pos);
	state_add(5, "MUL0", m_multiplier[0]);
	state_add(6, "MUL1", m_multiplier[1]);
	state_add(7, "TAP", m_tap);

	save_pointer(NAME(m_eram), ERAM_SIZE);
	save_item(NAME(m_program));
	save_item(NAME(m_iram));
	save_item(NAME(m_pc));
	save_item(NAME(m_slot));
	save_item(NAME(m_configuration));
	save_item(NAME(m_running));
	save_item(NAME(m_halted));
	save_item(NAME(m_acc));
	save_item(NAME(m_history));
	save_item(NAME(m_prev_offset));
	save_item(NAME(m_eram_read));
	save_item(NAME(m_multiplier));
	save_item(NAME(m_eram_latch));
	save_item(NAME(m_tap));
	save_item(NAME(m_buffer_pos));
	save_item(NAME(m_eram_pos));
	save_item(NAME(m_jump_target));
	save_item(NAME(m_jump_delay));
	save_item(NAME(m_serial_in));
	save_item(NAME(m_serial_out));
	save_item(NAME(m_audio_out));
	save_item(NAME(m_host_data));
	save_item(NAME(m_host_read));
	save_item(NAME(m_host_address));
}

void roland_lsp_device::device_reset()
{
	std::fill(std::begin(m_program), std::end(m_program), 0);
	std::fill(std::begin(m_iram), std::end(m_iram), 0);
	std::fill_n(m_eram.get(), ERAM_SIZE, 0);

	m_icount = 0;
	m_pc = PROGRAM_BASE;
	m_slot = 0;
	m_configuration = 0;
	m_running = false;
	m_halted = true;

	m_acc[0] = m_acc[1] = 0;
	std::fill(&m_history[0][0], &m_history[0][0] + 6, 0);
	m_prev_offset = 0;
	m_eram_read = 0;
	m_multiplier[0] = m_multiplier[1] = 0;
	m_eram_latch = 0;
	m_tap = 0;
	m_buffer_pos = 0;
	m_eram_pos = 0;
	m_jump_target = PROGRAM_BASE;
	m_jump_delay = 0;

	m_serial_in[0] = m_serial_in[1] = 0;
	m_serial_out[0] = m_serial_out[1] = 0;
	m_audio_out = 0;

	m_host_data = 0;
	m_host_read = 0;
	m_host_address = 0;
}

roland_lsp_device::instruction roland_lsp_device::decode(u32 word)
{
	instruction s;
	s.opcode = (word >> 21) & 7;
	s.store = (word >> 19) & 3;
	s.eram_cmd = (word >> 16) & 7;
	s.offset = (word >> 8) & 0x7f;
	s.coefficient = word & 0xff;
	s.shift = BIT(word, 15) ? 5 : 7;
	return s;
}

u32 roland_lsp_device::internal_r(offs_t address)
{
	return (address < PROGRAM_BASE) ? (m_iram[address] & 0xffffff) : m_program[address - PROGRAM_BASE];
}

void roland_lsp_device::internal_w(offs_t address, u32 data)
{
	if (address < PROGRAM_BASE)
		m_iram[address] = narrow(data);
	else
		m_program[address - PROGRAM_BASE] = data & 0xffffff;
}

void roland_lsp_device::configure(u16 word)
{
	LOGMASKED(LOG_CONFIG, "configure %04x\n", word);

	m_configuration = word;
	if (BIT(word, 12))
		m_running = false;
	else if (!m_running)
	{
		m_running = true;
		m_pc = PROGRAM_BASE;
		m_slot = 0;
		m_jump_delay = 0;
	}
}

u8 roland_lsp_device::host_r(offs_t offset)
{
	switch (offset & 0x0f)
	{
	case HOST_ADDRESS_LOW:  return m_host_read & 0xff;
	case HOST_ADDRESS_HIGH: return (m_host_read >> 8) & 0xff;
	case HOST_DATA_LOW:     return (m_host_read >> 16) & 0xff;
	case HOST_DATA_MID:     return 0;
	}
	return 0;
}

void roland_lsp_device::host_w(offs_t offset, u8 data)
{
	switch (offset & 0x0f)
	{
	case HOST_ADDRESS_LOW:
		m_host_address = (m_host_address & 0xff00) | data;
		LOGMASKED(LOG_HOST, "write %03x = %06x\n", m_host_address & 0x1ff, m_host_data);
		internal_w(m_host_address & 0x1ff, m_host_data);
		break;

	case HOST_ADDRESS_HIGH:
		m_host_address = (m_host_address & 0x00ff) | (data << 8);
		break;

	case HOST_DATA_LOW:
		m_host_data = (m_host_data & 0xffff00) | data;
		break;

	case HOST_DATA_MID:
		m_host_data = (m_host_data & 0xff00ff) | (data << 8);
		break;

	case HOST_DATA_HIGH:
		m_host_data = (m_host_data & 0x00ffff) | (data << 16);
		break;

	case HOST_CONFIGURE:
		configure(m_host_data & 0xffff);
		break;

	case HOST_READ_LOW:
		m_host_address = (m_host_address & 0xff00) | data;
		m_host_read = internal_r(m_host_address & 0x1ff);
		break;

	case HOST_READ_HIGH:
		m_host_address = (m_host_address & 0x00ff) | (data << 8);
		break;
	}
}

u16 roland_lsp_device::eram_address(int opener, bool read) const
{
	u16 base = 0;

	if (opener >= 0)
	{
		if ((eram_cmd_at(opener) & 6) == 4)
		{
			if (eram_cmd_at(opener + 1) == 2)
				base = 1;
			if (read)
				base += m_tap;
		}
		else
		{
			for (int n = 0; n < 5; n++)
				base += eram_cmd_at(opener + 1 + n) << (n * 3);
			if (BIT(eram_cmd_at(opener + 6), 0))
				base += eram_cmd_at(opener + 6) << 15;
		}
	}

	return m_eram_pos + base;
}

s32 roland_lsp_device::source(const instruction &s) const
{
	const s32 value = m_history[s.store == 2][2];
	return (s.store == 3) ? narrow(value) : saturate(value);
}

void roland_lsp_device::multiply(const instruction &s, s32 operand)
{
	const u8 code = s.coefficient;
	const s32 reg = m_multiplier[BIT(code, 1)];
	s32 &acc = m_acc[BIT(code, 4)];
	s32 product;

	if (BIT(code, 6))
		product = s32(((s64(operand) * ((reg & 0xffff) >> 9)) >> s.shift) >> 7);
	else
		product = s32((s64(operand) * (reg >> 16)) >> s.shift);

	if (BIT(code, 2))
		product = -product;
	if (!code)
		product = 0;

	acc = (BIT(code, 3) && !BIT(code, 6)) ? product : add(saturate(acc), product);
}

void roland_lsp_device::special(const instruction &s)
{
	const int slot = s.slot();
	s32 value = s.store ? source(s) : 0;
	s32 &acc = m_acc[s.opcode & 1];

	switch (slot)
	{
	case SLOT_JUMP_NEGATIVE:
	case SLOT_JUMP_POSITIVE:
	case SLOT_JUMP:
		if (slot == SLOT_JUMP || (value < 0) == (slot == SLOT_JUMP_NEGATIVE))
		{
			m_jump_target = (s.coefficient << 1) & 0x1ff;
			m_jump_delay = 2;
		}
		return;

	case SLOT_ERAM_WRITE:
		if (s.store)
			m_eram_latch = value;
		else
		{
			const s32 previous = is_immediate(m_prev_offset) ? IMMEDIATE[m_prev_offset] : m_iram[iram_address(m_prev_offset)];
			acc = add(s.replace() ? 0 : acc, s32(((s64(previous) * s.coefficient) >> 8) >> s.shift));
		}
		return;

	case SLOT_TAP:
		if (s.store == 3)
		{
			const s32 raw = m_history[0][2];
			m_tap = u16(raw >> 10);
			m_multiplier[0] = (raw & 0x3ff) << 13;
		}
		return;

	case SLOT_MULTIPLIER:
	case SLOT_MULTIPLIER + 1:
		m_multiplier[slot - SLOT_MULTIPLIER] = value;
		return;

	case SLOT_AUDIO_OUT:
		m_audio_out = value;
		if (first_half())
			m_serial_out[1] = value;
		break;

	case SLOT_ERAM_READ:
	case SLOT_ERAM_READ + 1:
	case SLOT_ERAM_READ + 2:
	case SLOT_ERAM_READ + 3:
		if (s.store)
			m_eram_read = m_eram[eram_address(m_pc - 12 - PROGRAM_BASE, true)] << 4;
		value = m_eram_read;
		break;

	case SLOT_AUDIO_IN:
		value = m_serial_in[first_half()];
		break;

	default:
		return;
	}

	m_iram[iram_address(0x60 + slot)] = value;
	acc = add(s.replace() ? 0 : acc, s32((s64(value) * s8(s.coefficient)) >> s.shift));
}

void roland_lsp_device::step()
{
	const u32 raw = m_program[m_pc - PROGRAM_BASE];
	const instruction s = decode(raw);

	if (raw && s.opcode >= OP_SPECIAL_A)
		special(s);
	else if (raw)
	{
		const int address = iram_address(s.offset);
		if (s.store)
			m_iram[address] = source(s);

		const s32 operand = s.immediate() ? IMMEDIATE[s.offset] : m_iram[address];
		const s32 product = s32((s64(operand) * s8(s.coefficient)) >> s.shift);

		switch (s.opcode)
		{
		case OP_MAC_A: m_acc[0] = add(m_acc[0], product); break;
		case OP_SET_A: m_acc[0] = product; break;
		case OP_MAC_B: m_acc[1] = add(m_acc[1], product); break;
		case OP_SET_B: m_acc[1] = product; break;
		case OP_MUL:   multiply(s, s.immediate() ? operand * s8(s.coefficient) : operand); break;
		case OP_ABS:   m_acc[0] = std::abs(product); break;
		}
	}

	for (int n = 0; n < 2; n++)
	{
		m_history[n][2] = m_history[n][1];
		m_history[n][1] = m_history[n][0];
		m_history[n][0] = m_acc[n];
	}
	m_prev_offset = s.offset;

	if (raw && s.opcode >= OP_SPECIAL_A && s.slot() == SLOT_ERAM_WRITE && s.store)
		m_eram[eram_address(m_pc - 8 - PROGRAM_BASE, false)] = m_eram_latch >> 4;

	if (m_jump_delay && !--m_jump_delay)
		m_pc = m_jump_target;
	else if (++m_pc >= PROGRAM_BASE + PROGRAM_SIZE)
		m_pc = PROGRAM_BASE;

	if (++m_slot >= PROGRAM_SIZE || m_pc == PROGRAM_BASE)
	{
		finish_sample();
		m_pc = PROGRAM_BASE;
		m_slot = 0;
	}
}

void roland_lsp_device::finish_sample()
{
	m_serial_out[0] = m_audio_out;
	m_jump_delay = 0;
	m_buffer_pos = (m_buffer_pos - 1) & 0x7f;
	m_eram_pos--;
}

void roland_lsp_device::execute_run()
{
	while (m_icount > 0)
	{
		if (m_halted || !m_running)
		{
			m_icount = 0;
			return;
		}

		debugger_instruction_hook(m_pc);
		step();
		m_icount--;
	}
}

void roland_lsp_device::run_once()
{
	m_halted = false;
	for (int n = 0; m_running && n < PROGRAM_SIZE; n++)
	{
		m_icount = 1;
		execute_run();
		if (!m_slot)
			break;
	}
	m_halted = true;
}

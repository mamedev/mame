// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/*

  TMS1000 family - base/shared

  TODO:
  - accurate INIT pin (currently, just use INPUT_LINE_RESET)
  - emulate newer die revisions? TMS1xxx rev. E and up have 4 cycles
    per opcode instead of 6. But which steps go where, is unknown.
    For now, just overclock the MCU instead.


The TMS0980 and TMS1000-family MCU cores are very similar. The TMS0980 has a
slightly bigger addressable area and uses 9bit instructions where the TMS1000
family uses 8bit instruction. The instruction set themselves are very similar
though.

Each instruction takes 12 cycles to execute in 2 phases: a fetch phase and an
execution phase. The execution phase takes place at the same time as the fetch
phase of the next instruction. So, during execution there are both fetch and
execution operations taking place. The operation can be split up as follows:
cycle #0
    - Fetch:
        1. ROM address 0
    - Execute:
        1. Read RAM
        2. Clear ALU inputs
        3. Execute BRANCH/CALL/RETN part #2
        4. K input valid
cycle #1
    - Fetch:
        1. ROM address 1
    - Execute:
        1. Update ALU inputs
cycle #2
    - Fetch:
        1. nothing/wait(?)
    - Execute:
        1. Perform ALU operation
        2. Write RAM
cycle #3
    - Fetch:
        1. Fetch/Update PC/RAM address #1
    - Execute:
        1. Register store part #1
cycle #4
    - Fetch:
        1. Fetch/Update PC/RAM address #2
    - Execute:
        1. Register store part #2
cycle #5
    - Fetch:
        1. Instruction decode
    - Execute:
        1. Execute BRANCH/CALL/RETN part #1

The MCU cores contains a set of fixed instructions and a set of
instructions created using microinstructions. A subset of the
instruction set could be defined from the microinstructions by
TI customers.

cycle #0: 15TN, ATN, CIN, CKN, CKP, DMTP, MTN, MTP, NATN, NDMTP, YTP
cycle #2: C8(?), CKM, NE(?), STO
cycle #3,#4: AUTA, AUTY

unknown cycle: CME, SSE, SSS

*/

#include "emu.h"
#include "tms1k_base.h"

tms1k_base_device::tms1k_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map) :
	cpu_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_BIG, byte_bits > 8 ? 16 : 8, rom_width, byte_bits > 8 ? -1 : 0, rom_map),
	m_data_config("data", ENDIANNESS_BIG, 8, ram_width, 0, ram_map),
	m_mpla(*this, "mpla"),
	m_ipla(*this, "ipla"),
	m_opla(*this, "opla"),
	m_opla_b(*this, "opla_b"),
	m_spla(*this, "spla"),
	m_o_pins(o_pins),
	m_r_pins(r_pins),
	m_pc_bits(pc_bits),
	m_byte_bits(byte_bits),
	m_x_bits(x_bits),
	m_stack_levels(stack_levels),
	m_output_pla_table(nullptr),
	m_read_k(*this),
	m_write_o(*this),
	m_write_r(*this),
	m_power_off(*this),
	m_read_ctl(*this),
	m_write_ctl(*this),
	m_write_pdc(*this),
	m_decode_micro(*this)
{ }

// disasm
void tms1k_base_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENPC:
		case STATE_GENPCBASE:
			str = string_format("%03X", m_rom_address << ((m_byte_bits > 8) ? 1 : 0));
			break;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms1k_base_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	m_o_mask = (1 << m_o_pins) - 1;
	m_r_mask = (1 << m_r_pins) - 1;
	m_pc_mask = (1 << m_pc_bits) - 1;
	m_x_mask = (1 << m_x_bits) - 1;

	// resolve callbacks
	m_read_k.resolve_safe(0);
	m_write_o.resolve_safe();
	m_write_r.resolve_safe();
	m_power_off.resolve_safe();
	m_read_ctl.resolve_safe(0);
	m_write_ctl.resolve_safe();
	m_write_pdc.resolve_safe();
	m_decode_micro.resolve();

	if (m_opla_b != nullptr && m_output_pla_table == nullptr)
		set_output_pla(&m_opla_b->as_u16());

	// zerofill
	m_pc = 0;
	m_sr = 0;
	m_pa = 0;
	m_pb = 0;
	m_ps = 0;
	m_a = 0;
	m_x = 0;
	m_y = 0;
	m_ca = 0;
	m_cb = 0;
	m_cs = 0;
	m_r = 0;
	m_o = 0;
	m_o_index = 0;
	m_cki_bus = 0;
	m_c4 = 0;
	m_p = 0;
	m_n = 0;
	m_adder_out = 0;
	m_carry_in = 0;
	m_carry_out = 0;
	m_status = 0;
	m_status_latch = 0;
	m_eac = 0;
	m_clatch = 0;
	m_add = 0;
	m_bl = 0;

	m_ram_in = 0;
	m_dam_in = 0;
	m_ram_out = 0;
	m_ram_address = 0;
	m_rom_address = 0;
	m_opcode = 0;
	m_fixed = 0;
	m_micro = 0;
	m_subcycle = 0;

	// register for savestates
	save_item(NAME(m_pc));
	save_item(NAME(m_sr));
	save_item(NAME(m_pa));
	save_item(NAME(m_pb));
	save_item(NAME(m_ps));
	save_item(NAME(m_a));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_ca));
	save_item(NAME(m_cb));
	save_item(NAME(m_cs));
	save_item(NAME(m_r));
	save_item(NAME(m_o));
	save_item(NAME(m_o_index));
	save_item(NAME(m_cki_bus));
	save_item(NAME(m_c4));
	save_item(NAME(m_p));
	save_item(NAME(m_n));
	save_item(NAME(m_adder_out));
	save_item(NAME(m_carry_in));
	save_item(NAME(m_carry_out));
	save_item(NAME(m_status));
	save_item(NAME(m_status_latch));
	save_item(NAME(m_eac));
	save_item(NAME(m_clatch));
	save_item(NAME(m_add));
	save_item(NAME(m_bl));

	save_item(NAME(m_ram_in));
	save_item(NAME(m_dam_in));
	save_item(NAME(m_ram_out));
	save_item(NAME(m_ram_address));
	save_item(NAME(m_rom_address));
	save_item(NAME(m_opcode));
	save_item(NAME(m_fixed));
	save_item(NAME(m_micro));
	save_item(NAME(m_subcycle));

	// register state for debugger
	state_add(STATE_GENPC, "GENPC", m_rom_address).formatstr("%03X").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_rom_address).formatstr("%03X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_sr).formatstr("%8s").noshow();

	m_state_count = 0;
	state_add(++m_state_count, "PC", m_pc).formatstr("%02X"); // 1
	state_add(++m_state_count, "SR", m_sr).formatstr("%01X"); // 2
	state_add(++m_state_count, "PA", m_pa).formatstr("%01X"); // 3
	state_add(++m_state_count, "PB", m_pb).formatstr("%01X"); // 4
	state_add(++m_state_count, "A", m_a).formatstr("%01X"); // 5
	state_add(++m_state_count, "X", m_x).formatstr("%01X"); // 6
	state_add(++m_state_count, "Y", m_y).formatstr("%01X"); // 7
	state_add(++m_state_count, "STATUS", m_status).formatstr("%01X"); // 8

	set_icountptr(m_icount);
}

device_memory_interface::space_config_vector tms1k_base_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tms1k_base_device::device_reset()
{
	m_pa = 0xf;
	m_pb = 0xf;
	m_pc = 0;
	m_ca = 0;
	m_cb = 0;
	m_cs = 0;

	m_eac = 0;
	m_bl = 0;
	m_add = 0;
	m_status = 0;
	m_clatch = 0;

	m_opcode = 0;
	m_micro = 0;
	m_fixed = 0;

	m_subcycle = 0;

	// clear outputs
	m_r = 0;
	m_write_r(m_r & m_r_mask);
	write_o_output(0);
	m_write_r(m_r & m_r_mask);
	m_power_off(0);
}



//-------------------------------------------------
//  common internal memory maps
//-------------------------------------------------

void tms1k_base_device::rom_10bit(address_map &map)
{
	map(0x000, 0x3ff).rom();
}

void tms1k_base_device::rom_11bit(address_map &map)
{
	map(0x000, 0x7ff).rom();
}

void tms1k_base_device::rom_12bit(address_map &map)
{
	map(0x000, 0xfff).rom();
}

void tms1k_base_device::ram_6bit(address_map &map)
{
	map(0x00, 0x3f).ram();
}

void tms1k_base_device::ram_7bit(address_map &map)
{
	map(0x00, 0x7f).ram();
}

void tms1k_base_device::ram_8bit(address_map &map)
{
	map(0x00, 0xff).ram();
}



//-------------------------------------------------
//  program counter/opcode decode
//-------------------------------------------------

void tms1k_base_device::next_pc()
{
	// The program counter is a LFSR. To put it simply, the feedback bit is a XOR of the two highest bits,
	// but it makes an exception when all low bits are set (eg. in TMS1000 case, when PC is 0x1f or 0x3f).
	int high = 1 << (m_pc_bits - 1);
	int fb = (m_pc << 1 & high) == (m_pc & high);

	if (m_pc == (m_pc_mask >> 1))
		fb = 1;
	else if (m_pc == m_pc_mask)
		fb = 0;

	m_pc = (m_pc << 1 | fb) & m_pc_mask;
}

void tms1k_base_device::read_opcode()
{
	debugger_instruction_hook(m_rom_address);
	m_opcode = m_program->read_byte(m_rom_address);
	m_c4 = bitswap<4>(m_opcode,0,1,2,3); // opcode operand is bitswapped for most opcodes

	m_fixed = m_fixed_decode[m_opcode];
	m_micro = m_micro_decode[m_opcode];

	next_pc();
}



//-------------------------------------------------
//  i/o handling
//-------------------------------------------------

void tms1k_base_device::write_o_output(u8 index)
{
	// a hardcoded table is supported if the output pla is unknown
	m_o_index = index;
	m_o = (m_output_pla_table == nullptr) ? m_opla->read(index) : m_output_pla_table[index];
	m_write_o(m_o & m_o_mask);
}

u8 tms1k_base_device::read_k_input()
{
	// K1,2,4,8 (KC test pin is not emulated)
	return m_read_k() & 0xf;
}

void tms1k_base_device::set_cki_bus()
{
	switch (m_opcode & 0xf8)
	{
		// 00001XXX: K-inputs
		case 0x08:
			m_cki_bus = read_k_input();
			break;

		// 0011XXXX: select bit
		case 0x30: case 0x38:
			m_cki_bus = 1 << (m_c4 >> 2) ^ 0xf;
			break;

		// 01XXXXXX: constant
		case 0x00: // R2,3,4 are NANDed with eachother, and then ORed with R1, making 00000XXX valid too
		case 0x40: case 0x48: case 0x50: case 0x58: case 0x60: case 0x68: case 0x70: case 0x78:
			m_cki_bus = m_c4;
			break;

		default:
			m_cki_bus = 0;
			break;
	}
}



//-------------------------------------------------
//  fixed opcode set
//-------------------------------------------------

// handle branches:

// add(latch) and bl(branch latch) are specific to 0980 series, c(chapter) bits are specific to 1100(and 1400) series
// TMS1400 and up and the CMOS chips have multiple stack levels, branches work a bit differently

void tms1k_base_device::op_br()
{
	// BR/BL: conditional branch
	if (m_stack_levels == 1)
	{
		if (m_status)
		{
			if (m_clatch == 0)
				m_pa = m_pb;

			m_ca = m_cb;
			m_pc = m_opcode & m_pc_mask;
		}
	}
	else
	{
		if (m_status)
		{
			m_pa = m_pb; // don't care about clatch
			m_ca = m_cb;
			m_pc = m_opcode & m_pc_mask;
		}
	}
}

void tms1k_base_device::op_call()
{
	// CALL/CALLL: conditional call
	if (m_stack_levels == 1)
	{
		if (m_status)
		{
			u8 prev_pa = m_pa;

			if (!m_clatch)
			{
				m_clatch = 1;
				m_sr = m_pc;
				m_pa = m_pb;
				m_cs = m_ca;
			}

			m_ca = m_cb;
			m_pb = prev_pa;
			m_pc = m_opcode & m_pc_mask;
		}
	}
	else
	{
		if (m_status)
		{
			// mask clatch bits (no need to mask others)
			u8 smask = (1 << m_stack_levels) - 1;
			m_clatch = (m_clatch << 1 | 1) & smask;

			m_sr = m_sr << m_pc_bits | m_pc;
			m_pc = m_opcode & m_pc_mask;

			m_ps = m_ps << 4 | m_pa;
			m_pa = m_pb;

			m_cs = m_cs << 2 | m_ca;
			m_ca = m_cb;
		}
		else
		{
			m_pb = m_pa;
			m_cb = m_ca;
		}
	}
}

void tms1k_base_device::op_retn()
{
	// RETN: return from subroutine
	if (m_stack_levels == 1)
	{
		if (m_clatch)
		{
			m_clatch = 0;
			m_pc = m_sr;
			m_ca = m_cs;
		}

		m_add = 0;
		m_bl = 0;
		m_pa = m_pb;
	}
	else
	{
		if (m_clatch & 1)
		{
			m_clatch >>= 1;

			m_pc = m_sr & m_pc_mask;
			m_sr >>= m_pc_bits;

			m_pa = m_pb = m_ps & 0xf;
			m_ps >>= 4;

			m_ca = m_cb = m_cs & 3;
			m_cs >>= 2;
		}
	}
}


// handle other:

// TMS1000/common

void tms1k_base_device::op_sbit()
{
	// SBIT: set memory bit
	if (m_ram_out == -1)
		m_ram_out = m_ram_in;
	m_ram_out |= (m_cki_bus ^ 0xf);
}

void tms1k_base_device::op_rbit()
{
	// RBIT: reset memory bit
	if (m_ram_out == -1)
		m_ram_out = m_ram_in;
	m_ram_out &= m_cki_bus;
}

void tms1k_base_device::op_setr()
{
	// SETR: set one R-output line
	m_r = m_r | (1 << m_y);
	m_write_r(m_r & m_r_mask);
}

void tms1k_base_device::op_rstr()
{
	// RSTR: reset one R-output line
	m_r = m_r & ~(1 << m_y);
	m_write_r(m_r & m_r_mask);
}

void tms1k_base_device::op_tdo()
{
	// TDO: transfer accumulator and status latch to O-output
	write_o_output(m_status_latch << 4 | m_a);
}

void tms1k_base_device::op_clo()
{
	// CLO: clear O-output
	write_o_output(0);
}

void tms1k_base_device::op_ldx()
{
	// LDX: load X register with (x_bits) constant
	m_x = m_c4 >> (4 - m_x_bits);
}

void tms1k_base_device::op_comx()
{
	// COMX: complement X register
	m_x ^= m_x_mask;
}

void tms1k_base_device::op_comx8()
{
	// COMX8: complement MSB of X register
	// note: on TMS1100, the mnemonic is simply called "COMX"
	m_x ^= 1 << (m_x_bits - 1);
}

void tms1k_base_device::op_ldp()
{
	// LDP: load page buffer with constant
	m_pb = m_c4;
}


// TMS1100-specific

void tms1k_base_device::op_comc()
{
	// COMC: complement chapter buffer
	m_cb ^= 1;
}


// TMS1400-specific

void tms1k_base_device::op_tpc()
{
	// TPC: transfer page buffer to chapter buffer
	m_cb = m_pb & 3;
}


// TMS0980-specific (and possibly child classes)

void tms1k_base_device::op_xda()
{
	// XDA: exchange DAM and A
	// note: setting A to DAM is done with DMTP and AUTA during this instruction
	m_ram_address |= (0x10 << (m_x_bits - 1));
}

void tms1k_base_device::op_off()
{
	// OFF: request auto power-off
	m_power_off(1);
}

void tms1k_base_device::op_seac()
{
	// SEAC: set end around carry
	m_eac = 1;
}

void tms1k_base_device::op_reac()
{
	// REAC: reset end around carry
	m_eac = 0;
}

void tms1k_base_device::op_sal()
{
	// SAL: set add latch (reset is done with RETN)
	m_add = 1;
}

void tms1k_base_device::op_sbl()
{
	// SBL: set branch latch (reset is done with RETN)
	m_bl = 1;
}



//-------------------------------------------------
//  execute
//-------------------------------------------------

void tms1k_base_device::execute_one()
{
	switch (m_subcycle)
	{
	case 0:
		// fetch: rom address 1/2

		// execute: br/call 2/2
		if (m_fixed & F_BR)    op_br();
		if (m_fixed & F_CALL)  op_call();
		if (m_fixed & F_RETN)  op_retn();

		// execute: k input valid, read ram, clear alu inputs
		dynamic_output();
		set_cki_bus();
		m_ram_in = m_data->read_byte(m_ram_address) & 0xf;
		m_dam_in = m_data->read_byte(m_ram_address | (0x10 << (m_x_bits - 1))) & 0xf;
		m_p = 0;
		m_n = 0;
		m_carry_in = 0;

		break;

	case 1:
		// fetch: rom address 2/2
		m_rom_address = (m_ca << (m_pc_bits+4)) | (m_pa << m_pc_bits) | m_pc;

		// execute: update alu inputs
		// N inputs
		if (m_micro & M_15TN)  m_n |= 0xf;
		if (m_micro & M_ATN)   m_n |= m_a;
		if (m_micro & M_NATN)  m_n |= (~m_a & 0xf);
		if (m_micro & M_CKN)   m_n |= m_cki_bus;
		if (m_micro & M_MTN)   m_n |= m_ram_in;

		// P inputs
		if (m_micro & M_CKP)   m_p |= m_cki_bus;
		if (m_micro & M_MTP)   m_p |= m_ram_in;
		if (m_micro & M_YTP)   m_p |= m_y;
		if (m_micro & M_DMTP)  m_p |= m_dam_in;
		if (m_micro & M_NDMTP) m_p |= (~m_dam_in & 0xf);

		// carry input
		if (m_micro & M_CIN)   m_carry_in |= 1;
		if (m_micro & M_SSS)   m_carry_in |= m_eac;

		break;

	case 2:
	{
		// fetch: nothing

		// execute: perform alu logic
		// note: officially, only 1 alu operation is allowed per opcode
		m_adder_out = m_p + m_n + m_carry_in;
		int carry_out = m_adder_out >> 4 & 1;
		int status = 1;
		m_ram_out = -1;

		if (m_micro & M_C8)    status &= carry_out;
		if (m_micro & M_NE)    status &= (m_n != m_p); // COMP
		if (m_micro & M_CKM)   m_ram_out = m_cki_bus;

		// special status circuit
		if (m_micro & M_SSE)
		{
			m_eac = m_carry_out;
			if (m_add)
				m_eac |= carry_out;
		}
		m_carry_out = carry_out;

		if (m_micro & M_STO || (m_micro & M_CME && m_eac == m_add))
			m_ram_out = m_a;

		// handle the other fixed opcodes here
		if (m_fixed & F_SBIT)  op_sbit();
		if (m_fixed & F_RBIT)  op_rbit();
		if (m_fixed & F_SETR)  op_setr();
		if (m_fixed & F_RSTR)  op_rstr();
		if (m_fixed & F_TDO)   op_tdo();
		if (m_fixed & F_CLO)   op_clo();
		if (m_fixed & F_LDX)   op_ldx();
		if (m_fixed & F_COMX)  op_comx();
		if (m_fixed & F_COMX8) op_comx8();
		if (m_fixed & F_LDP)   op_ldp();
		if (m_fixed & F_COMC)  op_comc();
		if (m_fixed & F_TPC)   op_tpc();
		if (m_fixed & F_OFF)   op_off();
		if (m_fixed & F_SEAC)  op_seac();
		if (m_fixed & F_REAC)  op_reac();
		if (m_fixed & F_SAL)   op_sal();
		if (m_fixed & F_SBL)   op_sbl();
		if (m_fixed & F_XDA)   op_xda();

		// after fixed opcode handling: store status, write ram
		m_status = status;
		if (m_ram_out != -1)
			m_data->write_byte(m_ram_address, m_ram_out);

		break;
	}

	case 3:
		// fetch: update pc, ram address 1/2
		// execute: register store 1/2
		break;

	case 4:
		// execute: register store 2/2
		if (m_micro & M_AUTA)  m_a = m_adder_out & 0xf;
		if (m_micro & M_AUTY)  m_y = m_adder_out & 0xf;
		if (m_micro & M_STSL)  m_status_latch = m_status;

		// fetch: update pc, ram address 2/2
		read_opcode();
		m_ram_address = m_x << 4 | m_y;
		break;

	case 5:
		// fetch: instruction decode (handled above, before next_pc)
		// execute: br/call 1/2
		break;
	}

	m_subcycle = (m_subcycle + 1) % 6;
}

void tms1k_base_device::execute_run()
{
	while (m_icount > 0)
	{
		m_icount--;
		execute_one();
	}
}

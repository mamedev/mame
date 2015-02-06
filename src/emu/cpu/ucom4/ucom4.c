// license:BSD-3-Clause
// copyright-holders:hap
/*

  NEC uCOM-4 MCU family cores

*/

#include "ucom4.h"
#include "debugger.h"

#include "ucom4op.inc"


// uCOM-43 products
const device_type NEC_D553 = &device_creator<upd553_cpu_device>; // 42-pin PMOS, 35 pins for I/O, Open Drain output, 2000x8 ROM, 96x4 RAM
const device_type NEC_D650 = &device_creator<upd650_cpu_device>; // 42-pin CMOS, 35 pins for I/O, push-pull output, 2000x8 ROM, 96x4 RAM

// uCOM-44 products
const device_type NEC_D552 = &device_creator<upd552_cpu_device>; // 42-pin PMOS, 35 pins for I/O, Open Drain output, 1000x8 ROM, 64x4 RAM

// internal memory maps
static ADDRESS_MAP_START(program_1k, AS_PROGRAM, 8, ucom4_cpu_device)
	AM_RANGE(0x0000, 0x03ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(program_2k, AS_PROGRAM, 8, ucom4_cpu_device)
	AM_RANGE(0x0000, 0x07ff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(data_64x4, AS_DATA, 8, ucom4_cpu_device)
	AM_RANGE(0x00, 0x3f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_96x4, AS_DATA, 8, ucom4_cpu_device)
	AM_RANGE(0x00, 0x3f) AM_RAM
	AM_RANGE(0x40, 0x5f) AM_RAM AM_MIRROR(0x20)
ADDRESS_MAP_END


// device definitions
upd553_cpu_device::upd553_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ucom4_cpu_device(mconfig, NEC_D553, "uPD553", tag, owner, clock, 3, 11, ADDRESS_MAP_NAME(program_2k), 7, ADDRESS_MAP_NAME(data_96x4), "upd553", __FILE__)
{ }

upd650_cpu_device::upd650_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ucom4_cpu_device(mconfig, NEC_D650, "uPD650", tag, owner, clock, 3, 11, ADDRESS_MAP_NAME(program_2k), 7, ADDRESS_MAP_NAME(data_96x4), "upd650", __FILE__)
{ }

upd552_cpu_device::upd552_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ucom4_cpu_device(mconfig, NEC_D552, "uPD552", tag, owner, clock, 1, 10, ADDRESS_MAP_NAME(program_1k), 6, ADDRESS_MAP_NAME(data_64x4), "upd552", __FILE__)
{ }


// disasm
void ucom4_cpu_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%c%c%c%c",
				m_inte_f  ? 'E':'e',
				m_int_f   ? 'I':'i',
				m_timer_f ? 'T':'t',
				m_carry_f ? 'C':'c'
			);
			break;

		default: break;
	}
}

offs_t ucom4_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(ucom4);
	return CPU_DISASSEMBLE_NAME(ucom4)(this, buffer, pc, oprom, opram, options);
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

enum
{
	UCOM4_PC=1, UCOM4_DPL, UCOM4_DPH,
	UCOM4_ACC
};

void ucom4_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_prgmask = (1 << m_prgwidth) - 1;
	m_datamask = (1 << m_datawidth) - 1;

	m_read_a.resolve_safe(0);
	m_read_b.resolve_safe(0);
	m_read_c.resolve_safe(0);
	m_read_d.resolve_safe(0);

	m_write_c.resolve_safe();
	m_write_d.resolve_safe();
	m_write_e.resolve_safe();
	m_write_f.resolve_safe();
	m_write_g.resolve_safe();
	m_write_h.resolve_safe();
	m_write_i.resolve_safe();

	// zerofill
	memset(m_stack, 0, sizeof(m_stack));
	m_op = 0;
	m_prev_op = 0;
	m_pc = 0;
	m_acc = 0;
	m_dpl = 0;
	m_dph = 0;
	m_carry_f = 0;
	m_timer_f = 0;
	m_int_f = 0;
	m_inte_f = 0;

	// register for savestates
	save_item(NAME(m_stack));
	save_item(NAME(m_op));
	save_item(NAME(m_prev_op));
	save_item(NAME(m_pc));
	save_item(NAME(m_acc));
	save_item(NAME(m_dpl));
	save_item(NAME(m_dph));
	save_item(NAME(m_carry_f));
	save_item(NAME(m_timer_f));
	save_item(NAME(m_int_f));
	save_item(NAME(m_inte_f));

	// register state for debugger
	state_add(UCOM4_PC, "PC",  m_pc).formatstr("%04X");
	state_add(UCOM4_DPL, "DPL", m_dpl).formatstr("%01X");
	state_add(UCOM4_DPH, "DPH", m_dph).formatstr("%01X");
	state_add(UCOM4_ACC, "ACC", m_acc).formatstr("%01X");

	state_add(STATE_GENPC, "curpc", m_pc).formatstr("%04X").noshow();
//	state_add(STATE_GENFLAGS, "GENFLAGS", m_flags).formatstr("%4s").noshow();

	m_icountptr = &m_icount;
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ucom4_cpu_device::device_reset()
{
	m_pc = 0;
	m_op = 0;
}



//-------------------------------------------------
//  execute
//-------------------------------------------------

void ucom4_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		m_icount--;
		
		// remember previous opcode
		m_prev_op = m_op;

		debugger_instruction_hook(this, m_pc);
		m_op = m_program->read_byte(m_pc);
		m_pc = (m_pc + 1) & m_prgmask;
	}
}

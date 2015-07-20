// license:BSD-3-Clause
// copyright-holders:hap
/*

  Mitsubishi M58846 MCU

*/

#include "m58846.h"


const device_type M58846 = &device_creator<m58846_device>;


// internal memory maps
static ADDRESS_MAP_START(program_2kx9, AS_PROGRAM, 16, m58846_device)
	AM_RANGE(0x0000, 0x07ff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(data_128x4, AS_DATA, 8, m58846_device)
	AM_RANGE(0x00, 0x7f) AM_RAM
ADDRESS_MAP_END


// device definitions
m58846_device::m58846_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: melps4_cpu_device(mconfig, M58846, "M58846", tag, owner, clock, 11, ADDRESS_MAP_NAME(program_2kx9), 7, ADDRESS_MAP_NAME(data_128x4), 12 /* number of D pins */, 2 /* subroutine page */, 1 /* interrupt page */, "m58846", __FILE__)
{ }


// disasm
offs_t m58846_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(m58846);
	return CPU_DISASSEMBLE_NAME(m58846)(this, buffer, pc, oprom, opram, options);
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m58846_device::device_start()
{
	melps4_cpu_device::device_start();
	m_timer = timer_alloc(0);
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m58846_device::device_reset()
{
	melps4_cpu_device::device_reset();
	reset_timer();
}



//-------------------------------------------------
//  timers
//-------------------------------------------------

void m58846_device::reset_timer()
{
	attotime base = attotime::from_ticks(6, unscaled_clock());
	m_timer->adjust(base);
}

void m58846_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id != 0)
		return;

	// timer 1: 7-bit fixed counter (manual specifically says 127)
	if (++m_tmr_count[0] == 127)
	{
		m_tmr_count[0] = 0;
		m_irqflag[1] = true;
		m_possible_irq = true;
	}

	// timer 2: 8-bit user defined counter with auto-reload
	if (m_v & 8 && ++m_tmr_count[1] == 0)
	{
		m_tmr_count[1] = m_tmr_reload;
		m_irqflag[2] = true;
		m_possible_irq = true;
		m_port_t ^= 1;
		m_write_t(m_port_t);
	}

	// schedule next timeout
	reset_timer();
}

void m58846_device::write_v(UINT8 data)
{
	// d0: enable timer 1 irq
	// d1: enable timer 2 irq? (TODO)
	// d2: ?
	// d3: timer 2 enable
	m_tmr_irq_enabled[0] = (data & 1) ? true : false;
	m_possible_irq = true;

	m_v = data;
}



//-------------------------------------------------
//  execute
//-------------------------------------------------

void m58846_device::execute_one()
{
	// handle one opcode
	switch (m_op & 0x1f0)
	{
		case 0x30: op_sey(); break;
		case 0x70: op_sp(); break;
		case 0xa0: op_a(); break;
		case 0xb0: op_la(); break;

		case 0xc0: case 0xd0: case 0xe0: case 0xf0: op_lxy(); break;

		default:
			switch (m_op & 0x1fc)
			{
		case 0x20: op_szb(); break;
		case 0x4c: op_sb(); break;
		case 0x58: op_szk(); break;
		case 0x5c: op_rb(); break;
		case 0x60: op_xam(); break;
		case 0x64: op_tam(); break;
		case 0x68: op_xamd(); break;
		case 0x6c: op_xami(); break;

		default:
			switch (m_op)
			{
		case 0x06: case 0x07: op_su(); break;
		case 0x40: case 0x41: op_lcps(); break;
		case 0x4a: case 0x4b: op_lz(); break;
		case 0x54: case 0x55: op_ias(); break;

		case 0x00: op_nop(); break;
		case 0x01: op_ba(); break;
		case 0x02: op_iny(); break;
		case 0x03: op_dey(); break;
		case 0x04: op_di(); break;
		case 0x05: op_ei(); break;
		case 0x09: op_tabe(); break; // undocumented
		case 0x0a: op_am(); break;
		case 0x0b: op_ose(); break;
		case 0x0c: op_tya(); break;
		case 0x0f: op_cma(); break;

		case 0x10: op_cls(); break;
		case 0x11: op_clds(); break;
		case 0x13: op_cld(); break;
		case 0x14: op_rd(); break;
		case 0x15: op_sd(); break;
		case 0x16: op_tepa(); break;
		case 0x17: op_ospa(); break;
		case 0x18: op_rl(); break; // undocumented
		case 0x19: op_rr(); break; // undocumented
		case 0x1a: op_teab(); break;
		case 0x1b: op_osab(); break;
		case 0x1c: op_tba(); break;
		case 0x1d: op_tay(); break;
		case 0x1e: op_tab(); break;

		case 0x26: op_seam(); break;
		case 0x2b: op_szd(); break;
		case 0x2f: op_szc(); break;

		case 0x43: op_amc(); break;
		case 0x44: op_rt(); break;
		case 0x45: op_rts(); break;
		case 0x46: op_rti(); break;
		case 0x48: op_rc(); break;
		case 0x49: op_sc(); break;

		case 0x53: op_amcs(); break;
		case 0x57: op_iak(); break;

		case 0x81: op_ofa(); break;
		case 0x82: op_snz1(); break;
		case 0x83: op_snz2(); break;
		case 0x84: op_oga(); break;
		case 0x85: op_t2ab(); break;
		case 0x86: op_tva(); break;
		case 0x8a: op_tab2(); break;
		case 0x8c: op_iaf(); break;

		default:
			melps4_cpu_device::execute_one();
			break;

			}
			break; // 0x1ff

			}
			break; // 0x1fc

	} // big switch
}

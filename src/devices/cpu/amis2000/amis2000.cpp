// license:BSD-3-Clause
// copyright-holders:hap
/*

  American Microsystems, Inc.(AMI) S2000-family 4-bit MCU cores, introduced late 1970s
  Overall functionality is similar to (and probably derived from) NEC uCOM-4.

  References:
  - AMI MOS Products Catalog 1979/1980
  - AMI S2000 Programming Manual (rev. 2)

  TODO:
  - unemulated opcodes (need more testing material)
  - is K/I input handling correct?
  - support external program map
  - add 50/60hz timer
  - add S2200/S2400

*/

#include "amis2000.h"
#include "debugger.h"


// S2000 is the most basic one, 64 nibbles internal RAM and 1KB internal ROM
// S2150 increased RAM to 80 nibbles and ROM to 1.5KB
// high-voltage output versions of these chips (S2000A and S2150A) are identical overall
const device_type AMI_S2000 = &device_creator<amis2000_cpu_device>;
const device_type AMI_S2150 = &device_creator<amis2150_cpu_device>;

// S2152 is an extension to S2150, removing the K pins and adding a better timer
const device_type AMI_S2152 = &device_creator<amis2152_cpu_device>;


// internal memory maps
static ADDRESS_MAP_START(program_1k, AS_PROGRAM, 8, amis2000_base_device)
	AM_RANGE(0x0000, 0x03ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(program_1_5k, AS_PROGRAM, 8, amis2000_base_device)
	AM_RANGE(0x0000, 0x03ff) AM_ROM
	AM_RANGE(0x0400, 0x05ff) AM_NOP // 0x00
	AM_RANGE(0x0600, 0x07ff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(data_64x4, AS_DATA, 8, amis2000_base_device)
	AM_RANGE(0x00, 0x3f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_80x4, AS_DATA, 8, amis2000_base_device)
	AM_RANGE(0x00, 0x3f) AM_RAM
	AM_RANGE(0x40, 0x4f) AM_RAM
ADDRESS_MAP_END


// device definitions
amis2000_cpu_device::amis2000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: amis2000_base_device(mconfig, AMI_S2000, "AMI S2000", tag, owner, clock, 2, 10, 3, 13, ADDRESS_MAP_NAME(program_1k), 6, ADDRESS_MAP_NAME(data_64x4), "amis2000", __FILE__)
{ }

amis2150_cpu_device::amis2150_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: amis2000_base_device(mconfig, AMI_S2150, "AMI S2150", tag, owner, clock, 3, 11, 3, 13, ADDRESS_MAP_NAME(program_1_5k), 7, ADDRESS_MAP_NAME(data_80x4), "amis2150", __FILE__)
{ }

amis2152_cpu_device::amis2152_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: amis2000_base_device(mconfig, AMI_S2152, "AMI S2152", tag, owner, clock, 3, 11, 3, 13, ADDRESS_MAP_NAME(program_1_5k), 7, ADDRESS_MAP_NAME(data_80x4), "amis2152", __FILE__)
{ }



// disasm
void amis2000_base_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c",
				m_f & 0x20 ? '6':'.',
				m_f & 0x10 ? '5':'.',
				m_f & 0x08 ? '4':'.',
				m_f & 0x04 ? '3':'.',
				m_f & 0x02 ? '2':'.',
				m_f & 0x01 ? '1':'.'
			);
			break;

		default: break;
	}
}

offs_t amis2000_base_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(amis2000);
	return CPU_DISASSEMBLE_NAME(amis2000)(this, buffer, pc, oprom, opram, options);
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

enum
{
	S2000_PC=1, S2000_BL, S2000_BU,
	S2000_ACC, S2000_E, S2000_CY
};

void amis2000_base_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	m_read_k.resolve_safe(0xf);
	m_read_i.resolve_safe(0xf);
	m_read_d.resolve_safe(0);
	m_write_d.resolve_safe();
	m_write_a.resolve_safe();
	m_write_f.resolve_safe();

	m_bu_mask = (1 << m_bu_bits) - 1;
	m_callstack_mask = (1 << m_callstack_bits) - 1;

	// zerofill
	memset(m_callstack, 0, sizeof(m_callstack));
	m_pc = 0;
	m_ppr = 0;
	m_pbr = 0;
	m_skip = false;
	m_op = 0;
	m_prev_op = 0;
	m_f = 0;
	m_carry = 0;
	m_bl = 0;
	m_bu = 0;
	m_acc = 0;
	m_e = 0;
	m_ki_mask = 0;
	m_d = 0;
	m_d_active = false;
	m_d_polarity = 0;
	m_a = 0;

	// register for savestates
	save_item(NAME(m_callstack));
	save_item(NAME(m_pc));
	save_item(NAME(m_ppr));
	save_item(NAME(m_pbr));
	save_item(NAME(m_skip));
	save_item(NAME(m_op));
	save_item(NAME(m_prev_op));
	save_item(NAME(m_f));
	save_item(NAME(m_carry));
	save_item(NAME(m_bl));
	save_item(NAME(m_bu));
	save_item(NAME(m_acc));
	save_item(NAME(m_e));
	save_item(NAME(m_ki_mask));
	save_item(NAME(m_d));
	save_item(NAME(m_d_active));
	save_item(NAME(m_d_polarity));
	save_item(NAME(m_a));

	// register state for debugger
	state_add(S2000_PC,     "PC",     m_pc    ).formatstr("%04X");
	state_add(S2000_BL,     "BL",     m_bl    ).formatstr("%01X");
	state_add(S2000_BU,     "BU",     m_bu    ).formatstr("%01X");
	state_add(S2000_ACC,    "ACC",    m_acc   ).formatstr("%01X");
	state_add(S2000_E,      "E",      m_e     ).formatstr("%01X");
	state_add(S2000_CY,     "CY",     m_carry ).formatstr("%01X");

	state_add(STATE_GENPC, "curpc", m_pc).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_f).formatstr("%6s").noshow();

	m_icountptr = &m_icount;
}


void amis2152_cpu_device::device_start()
{
	amis2000_base_device::device_start();

	m_d2f_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(amis2152_cpu_device::d2f_timer_cb), this));

	// zerofill
	m_d2f_latch = 0;
	m_fout_state = 0;

	// register for savestates
	save_item(NAME(m_d2f_latch));
	save_item(NAME(m_fout_state));
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void amis2000_base_device::device_reset()
{
	m_pc = 0;
	m_op = 0;
	m_skip = false;

	// clear i/o
	m_a = 0x1fff;
	m_write_a(0, m_a, 0xffff);
	m_d_polarity = 0;
	m_d = 0;
	d_latch_out(false);
}


void amis2152_cpu_device::device_reset()
{
	amis2000_base_device::device_reset();

	// start d2f timer
	m_write_f(0);
	d2f_timer_clock();
}



//-------------------------------------------------
//  execute
//-------------------------------------------------

void amis2000_base_device::execute_run()
{
	while (m_icount > 0)
	{
		m_icount--;

		// remember previous opcode
		m_prev_op = m_op;

		debugger_instruction_hook(this, m_pc);
		m_op = m_program->read_byte(m_pc);
		m_pc = (m_pc + 1) & 0x1fff;

		if (m_skip)
		{
			// always skip over PP prefix
			m_skip = ((m_op & 0xf0) == 0x60);
			m_op = 0; // nop
		}

		switch (m_op & 0xf0)
		{
			case 0x50: op_adis(); break;
			case 0x60: op_pp(); break;
			case 0x70: op_lai(); break;

			case 0x80: case 0x90: case 0xa0: case 0xb0: op_jms(); break;
			case 0xc0: case 0xd0: case 0xe0: case 0xf0: op_jmp(); break;

			default:
				switch (m_op)
				{
			case 0x00: op_nop(); break;
			case 0x01: op_halt(); break;
			case 0x02: op_rt(); break;
			case 0x03: op_rts(); break;
			case 0x04: op_psh(); break;
			case 0x05: op_psl(); break;
			case 0x06: op_and(); break;
			case 0x07: op_sos(); break;
			case 0x08: op_sbe(); break;
			case 0x09: op_szc(); break;
			case 0x0a: op_stc(); break;
			case 0x0b: op_rsc(); break;
			case 0x0c: op_lae(); break;
			case 0x0d: op_xae(); break;
			case 0x0e: op_inp(); break;
			case 0x0f: op_eur(); break;
			case 0x10: op_cma(); break;
			case 0x11: op_xabu(); break;
			case 0x12: op_lab(); break;
			case 0x13: op_xab(); break;
			case 0x14: op_adcs(); break;
			case 0x15: op_xor(); break;
			case 0x16: op_add(); break;
			case 0x17: op_sam(); break;
			case 0x18: op_disb(); break;
			case 0x19: op_mvs(); break;
			case 0x1a: op_out(); break;
			case 0x1b: op_disn(); break;

			case 0x28: op_szk(); break;
			case 0x29: op_szi(); break;
			case 0x2a: op_rf1(); break;
			case 0x2b: op_sf1(); break;
			case 0x2c: op_rf2(); break;
			case 0x2d: op_sf2(); break;
			case 0x2e: op_tf1(); break;
			case 0x2f: op_tf2(); break;

			default:
				switch (m_op & 0xfc)
				{
			case 0x1c: op_szm(); break;
			case 0x20: op_stm(); break;
			case 0x24: op_rsm(); break;

			case 0x30: op_xci(); break;
			case 0x34: op_xcd(); break;
			case 0x38: op_xc(); break;
			case 0x3c: op_lam(); break;
			case 0x40: op_lbz(); break;
			case 0x44: op_lbf(); break;
			case 0x48: op_lbe(); break;
			case 0x4c: op_lbep(); break;
				}
				break; // 0xfc

				}
				break; // 0xff

		} // big switch
	}
}

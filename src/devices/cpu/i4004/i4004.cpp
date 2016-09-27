// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 *   i4004.c
 *
 *   Intel 4004 CPU
 *
 *****************************************************************************/
#include "emu.h"
#include "debugger.h"
#include "i4004.h"


static const UINT8 kbp_table[] = { 0x00,0x01,0x02,0x0f,0x03,0x0f,0x0f,0x0f,0x04,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f };


/***************************************************************************
    MACROS
***************************************************************************/
#define GET_PC                  (m_ADDR[m_pc_pos])


const device_type I4004 = &device_creator<i4004_cpu_device>;


i4004_cpu_device::i4004_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, I4004, "Intel I4004", tag, owner, clock, "i4004", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 12, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 6, 0)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 12, 0), m_A(0), m_C(0), m_TEST(0), m_flags(0), m_program(nullptr), m_direct(nullptr), m_data(nullptr), m_io(nullptr), m_icount(0), m_pc_pos(0), m_addr_mask(0)
{
	m_program_config.m_is_octal = true;
	m_io_config.m_is_octal = true;
	m_data_config.m_is_octal = true;
}


UINT8 i4004_cpu_device::ROP()
{
	UINT8 retVal = m_direct->read_byte(GET_PC.w.l);
	GET_PC.w.l = (GET_PC.w.l + 1) & 0x0fff;
	m_PC = GET_PC;
	return retVal;
}

UINT8 i4004_cpu_device::READ_ROM()
{
	return m_direct->read_byte((GET_PC.w.l & 0x0f00) | m_R[0]);
}

void i4004_cpu_device::WPM()
{
	UINT8 t =  (m_program->read_byte(m_RAM.d) << 4) | m_A;
	m_program->write_byte((GET_PC.w.l & 0x0f00) | m_RAM.d, t);
}


UINT8 i4004_cpu_device::ARG()
{
	UINT8 retVal = m_direct->read_byte(GET_PC.w.l);
	GET_PC.w.l = (GET_PC.w.l + 1) & 0x0fff;
	m_PC = GET_PC;
	return retVal;
}

UINT8 i4004_cpu_device::RM()
{
	return m_data->read_byte(m_RAM.d) & 0x0f;
}

UINT8 i4004_cpu_device::RMS(UINT32 a)
{
	return m_data->read_byte((m_RAM.d & 0xff0) + a) >> 4;
}

void i4004_cpu_device::WM(UINT8 v)
{
	UINT8 t =  m_data->read_byte(m_RAM.d);
	m_data->write_byte(m_RAM.d, (t & 0xf0) | v);
}


void i4004_cpu_device::WMP(UINT8 v)
{
	m_io->write_byte((m_RAM.d >> 6) | 0x10, v & 0x0f);
}

void i4004_cpu_device::WMS(UINT32 a, UINT8 v)
{
	UINT8 t =  m_data->read_byte((m_RAM.d & 0xff0) + a);
	m_data->write_byte((m_RAM.d & 0xff0) + a, (t & 0x0f) | (v<<4));
}

UINT8 i4004_cpu_device::RIO()
{
	return m_io->read_byte(m_RAM.b.l >> 4) & 0x0f;
}

void i4004_cpu_device::WIO(UINT8 v)
{
	m_io->write_byte(m_RAM.b.l >> 4, v & 0x0f);
}

UINT8 i4004_cpu_device::GET_REG(UINT8 num)
{
	UINT8 r = m_R[num>>1];
	if (num & 1) {
		return r & 0x0f;
	} else {
		return (r >> 4) & 0x0f;
	}
}

void i4004_cpu_device::SET_REG(UINT8 num, UINT8 val)
{
	if (num & 1) {
		m_R[num>>1] = (m_R[num>>1] & 0xf0) + (val & 0x0f);
	} else {
		m_R[num>>1] = (m_R[num>>1] & 0x0f) + ((val & 0x0f) << 4);
	}
}

void i4004_cpu_device::PUSH_STACK()
{
	m_pc_pos = (m_pc_pos + 1) & m_addr_mask;
}

void i4004_cpu_device::POP_STACK()
{
	m_ADDR[m_pc_pos].d = 0;
	m_pc_pos = (m_pc_pos - 1) & m_addr_mask;
}

void i4004_cpu_device::set_test(UINT8 val)
{
	m_TEST = val;
}

void i4004_cpu_device::execute_one(int opcode)
{
	m_icount -= 8;
	switch (opcode)
	{
		case 0x00:  /* NOP  */
			/* no op */
			break;
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		case 0x1c: case 0x1d: case 0x1e: case 0x1f: /* JCN */
			{
				UINT8 arg =  ARG();

				UINT8 C1 = BIT(opcode,3);
				UINT8 C2 = BIT(opcode,2);
				UINT8 C3 = BIT(opcode,1);
				UINT8 C4 = BIT(opcode,0);
				UINT8 JUMP = (((m_A == 0) ? 1 : 0) & C2) | ((m_C) & C3) | ((m_TEST ^ 1) & C4);
				m_icount -= 8;

				if(((C1 ^ 1) &  JUMP) | (C1 & (JUMP ^ 1))) {
					GET_PC.w.l = (GET_PC.w.l & 0x0f00) | arg;
					m_PC = GET_PC;
				}
			}
			break;
		case 0x20: case 0x22: case 0x24: case 0x26:
		case 0x28: case 0x2a: case 0x2c: case 0x2e: /* FIM */
			m_icount -= 8;
			m_R[(opcode & 0x0f)>>1] = ROP();
			break;
		case 0x21: case 0x23: case 0x25: case 0x27:
		case 0x29: case 0x2b: case 0x2d: case 0x2f: /* SRC */
			m_RAM.b.l = m_R[(opcode & 0x0f)>>1];
			break;
		case 0x30: case 0x32: case 0x34: case 0x36:
		case 0x38: case 0x3a: case 0x3c: case 0x3e: /* FIN */
			m_icount -= 8;
			m_R[(opcode & 0x0f)>>1] = READ_ROM();
			break;
		case 0x31: case 0x33: case 0x35: case 0x37:
		case 0x39: case 0x3b: case 0x3d: case 0x3f: /* JIN */
			GET_PC.w.l = (GET_PC.w.l & 0x0f00) | m_R[(opcode & 0x0f)>>1];
			m_PC = GET_PC;
			break;
		case 0x40: case 0x41: case 0x42: case 0x43:
		case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b:
		case 0x4c: case 0x4d: case 0x4e: case 0x4f: /* JUN */
			m_icount -= 8;
			GET_PC.w.l = ((opcode & 0x0f) << 8) | ARG();
			m_PC = GET_PC;
			break;
		case 0x50: case 0x51: case 0x52: case 0x53:
		case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b:
		case 0x5c: case 0x5d: case 0x5e: case 0x5f: /* JMS */
			{
				UINT16 newPC = ((opcode & 0x0f) << 8) | ARG();
				m_icount -= 8;
				PUSH_STACK();
				GET_PC.w.l = newPC;
				m_PC = GET_PC;
			}
			break;
		case 0x60: case 0x61: case 0x62: case 0x63:
		case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b:
		case 0x6c: case 0x6d: case 0x6e: case 0x6f: /* INC */
			SET_REG(opcode & 0x0f, GET_REG(opcode & 0x0f) + 1);
			break;
		case 0x70: case 0x71: case 0x72: case 0x73:
		case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b:
		case 0x7c: case 0x7d: case 0x7e: case 0x7f: /* ISZ */
			{
				UINT8 val = (GET_REG(opcode & 0x0f) + 1) & 0xf;
				UINT16 addr = ARG();
				m_icount -= 8;
				SET_REG(opcode & 0x0f, val);
				if (val!=0) {
					GET_PC.w.l = (GET_PC.w.l & 0x0f00) | addr;
				}
				m_PC = GET_PC;
			}
			break;
		case 0x80: case 0x81: case 0x82: case 0x83:
		case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8a: case 0x8b:
		case 0x8c: case 0x8d: case 0x8e: case 0x8f: /* ADD */
			{
				UINT8 acc = m_A + GET_REG(opcode & 0x0f) + m_C;
				m_A = acc & 0x0f;
				m_C = (acc >> 4) & 1;
			}
			break;
		case 0x90: case 0x91: case 0x92: case 0x93:
		case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b:
		case 0x9c: case 0x9d: case 0x9e: case 0x9f: /* SUB */
			{
				UINT8 acc = m_A + (GET_REG(opcode & 0x0f) ^ 0x0f) + (m_C ^ 1);
				m_A = acc & 0x0f;
				m_C = (acc >> 4) & 1;
			}
			break;
		case 0xa0: case 0xa1: case 0xa2: case 0xa3:
		case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab:
		case 0xac: case 0xad: case 0xae: case 0xaf: /* LD */
			m_A = GET_REG(opcode & 0x0f);
			break;
		case 0xb0: case 0xb1: case 0xb2: case 0xb3:
		case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb:
		case 0xbc: case 0xbd: case 0xbe: case 0xbf: /* XCH */
			{
				UINT8 temp = m_A;
				m_A = GET_REG(opcode & 0x0f);
				SET_REG(opcode & 0x0f, temp);
			}
			break;
		case 0xc0: case 0xc1: case 0xc2: case 0xc3:
		case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		case 0xc8: case 0xc9: case 0xca: case 0xcb:
		case 0xcc: case 0xcd: case 0xce: case 0xcf: /*  BBL */
			POP_STACK();
			m_A = opcode & 0x0f;
			m_PC = GET_PC;
			break;
		case 0xd0: case 0xd1: case 0xd2: case 0xd3:
		case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		case 0xd8: case 0xd9: case 0xda: case 0xdb:
		case 0xdc: case 0xdd: case 0xde: case 0xdf: /* LDM */
			m_A = opcode & 0x0f;
			break;
		case 0xe0: /* WRM */
			WM(m_A);
			break;
		case 0xe1: /* WMP */
			WMP(m_A);
			break;
		case 0xe2: /* WRR */
			WIO(m_A);
			break;
		case 0xe3: /* WPM */
			WPM();
			break;
		case 0xe4: /* WR0 */
			WMS(0,m_A);
			break;
		case 0xe5: /* WR1 */
			WMS(1,m_A);
			break;
		case 0xe6: /* WR2 */
			WMS(2,m_A);
			break;
		case 0xe7: /* WR3 */
			WMS(3,m_A);
			break;
		case 0xe8: /* SBM */
			m_A = m_A + (RM() ^ 0x0f) + (m_C ^ 1);
			m_C = m_A >> 4;
			m_A &= 0x0f;
			break;
		case 0xe9: /* RDM */
			m_A = RM();
			break;
		case 0xea: /* RDR */
			m_A = RIO();
			break;
		case 0xeb: /* ADM */
			m_A += RM() + m_C;
			m_C = m_A >> 4;
			m_A &= 0x0f;
			break;
		case 0xec: /* RD0 */
			m_A = RMS(0);
			break;
		case 0xed: /* RD1 */
			m_A = RMS(1);
			break;
		case 0xee: /* RD2 */
			m_A = RMS(2);
			break;
		case 0xef: /* RD3 */
			m_A = RMS(3);
			break;

		case 0xf0: /* CLB */
			m_A = 0;
			m_C = 0;
			break;
		case 0xf1: /* CLC */
			m_C = 0;
			break;
		case 0xf2: /* IAC */
			m_A += 1;
			m_C = m_A >> 4;
			m_A &= 0x0f;
			break;
		case 0xf3: /* CMC */
			m_C ^= 1;
			break;
		case 0xf4: /* CMA */
			m_A ^= 0x0f;
			break;
		case 0xf5: /* RAL */
			m_A = (m_A << 1) | m_C;
			m_C = m_A >> 4;
			m_A &= 0x0f;
			break;
		case 0xf6: /* RAR */
			{
				UINT8 c = m_A & 1;
				m_A = (m_A >> 1) | (m_C  << 3);
				m_C = c;
			}
			break;
		case 0xf7: /* TCC */
			m_A = m_C;
			m_C = 0;
			break;
		case 0xf8: /* DAC */
			m_A = m_A + 0x0f;
			m_C = m_A >> 4;
			m_A &= 0x0f;
			break;
		case 0xf9: /* TCS */
			m_A = m_C ? 10 : 9;
			m_C = 0;
			break;
		case 0xfa: /* STC */
			m_C = 1;
			break;
		case 0xfb: /* DAA */
			if (m_C || (m_A > 9)) {
				m_A += 6;
			}
			if (m_A > 0x0f) {
				// it is unaffected if it is in range
				m_C = 1;
			}
			m_A &= 0x0f;
			break;
		case 0xfc: /* KBP */
			m_A = kbp_table[m_A];
			break;
		case 0xfd: /* DCL */
			m_RAM.b.h = m_A;
			break;
	}
}


/***************************************************************************
    COMMON EXECUTION
***************************************************************************/

void i4004_cpu_device::execute_run()
{
	do
	{
		debugger_instruction_hook(this, GET_PC.d);
		execute_one(ROP());

	} while (m_icount > 0);
}

/***************************************************************************
    CORE INITIALIZATION
***************************************************************************/

void i4004_cpu_device::device_start()
{
	/* set up the state table */
	{
		state_add(I4004_PC,       "PC",       m_PC.w.l).mask(0x0fff);
		state_add(STATE_GENPC,    "GENPC",    m_PC.w.l).mask(0x0fff).noshow();
		state_add(STATE_GENFLAGS, "GENFLAGS", m_flags).mask(0x0f).callimport().callexport().noshow().formatstr("%4s");
		state_add(I4004_A,        "A",        m_A).mask(0x0f);

		for (int regnum = 0; regnum < 8; regnum++)
		{
			state_add(I4004_R01 + regnum, string_format("R%X%X", regnum * 2, regnum * 2 + 1).c_str(), m_R[regnum]);
		}

		for (int addrnum = 0; addrnum < 4; addrnum++)
		{
			state_add(I4004_ADDR1 + addrnum, string_format("ADDR%d", addrnum + 1).c_str(), m_ADDR[addrnum].w.l).mask(0xfff);
		}

		state_add(I4004_RAM,   "RAM",   m_RAM.w.l).mask(0x0fff);
	}

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_data = &space(AS_DATA);
	m_io = &space(AS_IO);

	save_item(NAME(m_PC));
	save_item(NAME(m_A));
	save_item(NAME(m_C));
	save_item(NAME(m_TEST));
	save_item(NAME(m_pc_pos));
	save_item(NAME(m_ADDR[0]));
	save_item(NAME(m_ADDR[1]));
	save_item(NAME(m_ADDR[2]));
	save_item(NAME(m_ADDR[3]));
	save_item(NAME(m_R[0]));
	save_item(NAME(m_R[1]));
	save_item(NAME(m_R[2]));
	save_item(NAME(m_R[3]));
	save_item(NAME(m_R[4]));
	save_item(NAME(m_R[5]));
	save_item(NAME(m_R[6]));
	save_item(NAME(m_R[7]));
	save_item(NAME(m_RAM));

	m_icountptr = &m_icount;
}


/***************************************************************************
    COMMON RESET
***************************************************************************/

void i4004_cpu_device::device_reset()
{
	m_addr_mask = 3;
	m_C = 0;
	m_pc_pos = 0;
	m_A = 0;
	memset(m_R,0,8);
	memset(m_ADDR,0,sizeof(m_ADDR));
	m_RAM.d = 0;
	m_PC = GET_PC;

}



/***************************************************************************
    COMMON STATE IMPORT/EXPORT
***************************************************************************/

void i4004_cpu_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			m_C = (m_flags >> 1) & 1;
			m_TEST = (m_flags >> 0) & 1;
			break;
	}
}

void i4004_cpu_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			m_flags = ((m_A == 0) ? 0x04 : 0x00) |
						(m_C ? 0x02 : 0x00) |
						(m_TEST ? 0x01 : 0x00);
			break;
	}
}

void i4004_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format(".%c%c%c",
				(m_A==0) ? 'Z':'.',
				m_C      ? 'C':'.',
				m_TEST   ? 'T':'.');
			break;
	}
}

offs_t i4004_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( i4004 );
	return CPU_DISASSEMBLE_NAME(i4004)(this, buffer, pc, oprom, opram, options);
}

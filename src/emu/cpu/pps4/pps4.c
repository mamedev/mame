// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 *   pps4.c
 *
 *   Rockwell PPS-4 CPU
 *
 *****************************************************************************/
#include "emu.h"
#include "debugger.h"
#include "pps4.h"

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


const device_type PPS4 = &device_creator<pps4_device>;


pps4_device::pps4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, PPS4, "PPS4", tag, owner, clock, "pps4", __FILE__ )
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 12)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 12)  // 4bit RAM
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 8)  // 4bit IO
{
}


offs_t pps4_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( pps4 );
	return CPU_DISASSEMBLE_NAME(pps4)(this, buffer, pc, oprom, opram, options);
}


inline UINT8 pps4_device::ROP()
{
	UINT8 retVal = m_direct->read_decrypted_byte(m_P.w.l);
	m_P.w.l = (m_P.w.l + 1) & 0x0fff;
	return retVal;
}

inline UINT8 pps4_device::ARG()
{
	UINT8 retVal = m_direct->read_raw_byte(m_P.w.l);
	m_P.w.l = (m_P.w.l + 1) & 0x0fff;
	return retVal;
}

inline void pps4_device::DO_SKIP()
{
	m_P.w.l = (m_P.w.l + 1) & 0x0fff;
}

void pps4_device::execute_one(int opcode)
{
	m_icount -= 1;
	switch (opcode)
	{
		// Arithmetic instructions
		case 0x0b:  // AD
					break;
		case 0x0a:  // ADC
					break;
		case 0x09:  // ADSK
					break;
		case 0x08:  // ADCSK
					break;
		case 0x60:  case 0x61:  case 0x62:  case 0x63:
		case 0x64:  case 0x66:  case 0x67:  case 0x68:
		case 0x69:  case 0x6a:  case 0x6b:  case 0x6c:
		case 0x6d:  case 0x6e:
					// ADI
					break;
		case 0x65:  //DC
					m_A = (m_A + 10) & 0x0f;
					break;
		// Logical instructions
		case 0x0d:  // AND
					break;
		case 0x0f:  // OR
					break;
		case 0x0c:  // EOR
					break;
		case 0x0e:  // COMP
					m_A ^= 0x0f;
					break;
		// Data transfer instructions
		case 0x20:  // SC
					m_C = 1;
					break;
		case 0x24:  //RC
					m_C = 0;
					break;
		case 0x22:  // SF1
					m_FF1 = 1;
					break;
		case 0x26:  // RF1
					m_FF1 = 0;
					break;
		case 0x21:  // SF2
					m_FF2 = 1;
					break;
		case 0x25:  // RF2
					m_FF2 = 0;
					break;
		case 0x30:  case 0x31:  case 0x32:  case 0x33:
		case 0x34:  case 0x35:  case 0x36:  case 0x37:
					// LD
					break;
		case 0x38:  case 0x39:  case 0x3a:  case 0x3b:
		case 0x3c:  case 0x3d:  case 0x3e:  case 0x3f:
					// EX
					break;
		case 0x28:  case 0x29:  case 0x2a:  case 0x2b:
		case 0x2c:  case 0x2d:  case 0x2e:  case 0x2f:
					// EXD
					break;
		case 0x70:  case 0x71:  case 0x72:  case 0x73:
		case 0x74:  case 0x75:  case 0x76:  case 0x77:
		case 0x78:  case 0x79:  case 0x7a:  case 0x7b:
		case 0x7c:  case 0x7d:  case 0x7e:  case 0x7f:
					// LDI
					m_A = opcode & 0x0f;
					break;
		case 0x12:  // LAX
					m_A = m_X;
					break;
		case 0x1b:  // LXA
					m_X = m_A;
					break;
		case 0x11:  // LABL
					m_A = m_B.w.l & 0x00f;
					break;
		case 0x10:  // LBMX
					m_B.w.l &= 0xf0f;
					m_B.w.l |= (m_X << 4);
					break;
		case 0x04:  // LBUA
					break;
		case 0x19:  // XABL
					{
						UINT8 tmp = m_B.w.l & 0x00f;
						m_B.w.l &= 0xff0;
						m_B.w.l |= m_A;
						m_A = tmp;
					}
					break;
		case 0x18:  // XBMX
					{
						UINT8 tmp = (m_B.w.l & 0x0f0) >> 4;
						m_B.w.l &= 0xf0f;
						m_B.w.l |= (m_X << 4);
						m_X = tmp;
					}
					break;
		case 0x1a:  // XAX
					{
						UINT8 tmp = m_A;
						m_A = m_X;
						m_X = tmp;
					}
					break;
		case 0x06:  // XS
					{
						PAIR tmp = m_SA;
						m_SA = m_SB;
						m_SB = tmp;
					}
					break;
		case 0x6f:  // CYS
					break;
		case 0xc0:  case 0xc1:  case 0xc2:  case 0xc3:
		case 0xc4:  case 0xc5:  case 0xc6:  case 0xc7:
		case 0xc8:  case 0xc9:  case 0xca:  case 0xcb:
		case 0xcc:  case 0xcd:  case 0xce:  case 0xcf:
					// LB
					{
						//UINT8 tmp = ARG();
						m_icount -= 1;
					}
					break;
		case 0x00:  // LBL
					{
						UINT8 tmp = ARG();
						m_icount -= 1;
						m_B.w.l = tmp;
					}
					break;
		case 0x17:  // INCB
					if ((m_B.w.l & 0x0f) == 0x0f) {
						m_B.w.l &= 0xff0;
						DO_SKIP();
					} else {
						m_B.w.l += 1;
					}
					break;
		case 0x1f:  // DECB
					if ((m_B.w.l & 0x0f) == 0x00) {
						m_B.w.l |= 0x00f;
						DO_SKIP();
					} else {
						m_B.w.l -= 1;
					}
					break;
		// Control transfer instructions
		case 0x80:  case 0x81:  case 0x82:  case 0x83:
		case 0x84:  case 0x85:  case 0x86:  case 0x87:
		case 0x88:  case 0x89:  case 0x8a:  case 0x8b:
		case 0x8c:  case 0x8d:  case 0x8e:  case 0x8f:
		case 0x90:  case 0x91:  case 0x92:  case 0x93:
		case 0x94:  case 0x95:  case 0x96:  case 0x97:
		case 0x98:  case 0x99:  case 0x9a:  case 0x9b:
		case 0x9c:  case 0x9d:  case 0x9e:  case 0x9f:
		case 0xa0:  case 0xa1:  case 0xa2:  case 0xa3:
		case 0xa4:  case 0xa5:  case 0xa6:  case 0xa7:
		case 0xa8:  case 0xa9:  case 0xaa:  case 0xab:
		case 0xac:  case 0xad:  case 0xae:  case 0xaf:
		case 0xb0:  case 0xb1:  case 0xb2:  case 0xb3:
		case 0xb4:  case 0xb5:  case 0xb6:  case 0xb7:
		case 0xb8:  case 0xb9:  case 0xba:  case 0xbb:
		case 0xbc:  case 0xbd:  case 0xbe:  case 0xbf:
					// T
					m_P.w.l = (m_P.w.l & 0xfc0) | (opcode & 0x3f);
					break;
		case 0xd0:  case 0xd1:  case 0xd2:  case 0xd3:
		case 0xd4:  case 0xd5:  case 0xd6:  case 0xd7:
		case 0xd8:  case 0xd9:  case 0xda:  case 0xdb:
		case 0xdc:  case 0xdd:  case 0xde:  case 0xdf:
		case 0xe0:  case 0xe1:  case 0xe2:  case 0xe3:
		case 0xe4:  case 0xe5:  case 0xe6:  case 0xe7:
		case 0xe8:  case 0xe9:  case 0xea:  case 0xeb:
		case 0xec:  case 0xed:  case 0xee:  case 0xef:
		case 0xf0:  case 0xf1:  case 0xf2:  case 0xf3:
		case 0xf4:  case 0xf5:  case 0xf6:  case 0xf7:
		case 0xf8:  case 0xf9:  case 0xfa:  case 0xfb:
		case 0xfc:  case 0xfd:  case 0xfe:  case 0xff:
					// TM
					break;
		case 0x50:  case 0x51:  case 0x52:  case 0x53:
		case 0x54:  case 0x55:  case 0x56:  case 0x57:
		case 0x58:  case 0x59:  case 0x5a:  case 0x5b:
		case 0x5c:  case 0x5d:  case 0x5e:  case 0x5f:
					// TL
					{
						//UINT8 tmp = ARG();
						m_icount -= 1;
					}
					break;
		case 0x01:  case 0x02:  case 0x03:
					// TML
					{
						//UINT8 tmp = ARG();
						m_icount -= 1;
					}
					break;
		case 0x15:  // SKC
					break;
		case 0x1e:  // SKZ
					break;
		case 0x40:  case 0x41:  case 0x42:  case 0x43:
		case 0x44:  case 0x45:  case 0x46:  case 0x47:
		case 0x48:  case 0x49:  case 0x4a:  case 0x4b:
		case 0x4c:  case 0x4d:  case 0x4e:  case 0x4f:
					// SKBI
					break;
		case 0x16:  // SKF1
					break;
		case 0x14:  // SKF2
					break;
		case 0x05:  // RTN
					break;
		case 0x07:  // RTNSK
					break;
		// Input/Output instructions
		case 0x1c:  // IOL
					{
						//UINT8 tmp = ARG();
						m_icount -= 1;
					}
					break;
		case 0x27:  // DIA
					break;
		case 0x23:  // DIB
					break;
		case 0x1d:  // DOA
					break;
		// Special instructions
		case 0x13:  // SAG
					break;
	}
}


/***************************************************************************
    COMMON EXECUTION
***************************************************************************/
void pps4_device::execute_run()
{
	do
	{
		debugger_instruction_hook(this, m_P.d);
		execute_one(ROP());

	} while (m_icount > 0);
}

/***************************************************************************
    CORE INITIALIZATION
***************************************************************************/

void pps4_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_data = &space(AS_DATA);
	m_io = &space(AS_IO);

	save_item(NAME(m_A));
	save_item(NAME(m_X));
	save_item(NAME(m_P));
	save_item(NAME(m_SA));
	save_item(NAME(m_SB));
	save_item(NAME(m_B));
	save_item(NAME(m_C));
	save_item(NAME(m_FF1));
	save_item(NAME(m_FF2));

	state_add( PPS4_PC, "PC", m_P.d ).mask(0xfff).formatstr("%03X");
	state_add( PPS4_A,  "A",  m_A ).formatstr("%02X");  // TODO: size?
	state_add( PPS4_X,  "X",  m_X ).formatstr("%02X");  // TODO: size?
	state_add( PPS4_SA, "SA", m_SA.d ).formatstr("%04X");  // TODO: size?
	state_add( PPS4_SB, "SB", m_SB.d ).formatstr("%04X");  // TODO: size?
	state_add( PPS4_B,  "B",  m_B.d ).formatstr("%04X"); // TODO: size?
	state_add( STATE_GENPC, "GENPC", m_P.d ).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_C ).formatstr("%3s").noshow();

	m_icountptr = &m_icount;
}

void pps4_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%c%c%c",
				m_C ? 'C':'.',
				m_FF1 ? '1':'.',
				m_FF2 ? '2':'.');
			break;
	}
}

/***************************************************************************
    COMMON RESET
***************************************************************************/

void pps4_device::device_reset()
{
	m_A = m_X = 0;
	m_C = m_FF1 = m_FF2 = 0;

	m_P.d = 0;
	m_SA.d = 0;
	m_SB.d = 0;
	m_B.d = 0;
}

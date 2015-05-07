// license:BSD-3-Clause
// copyright-holders:hap
/*

  Seiko Epson E0C6200 CPU core and E0C62 MCU family

  References:
  - 1998 MF297-06a E0C6200/E0C6200A Core CPU Manual
  - 1998 MF1049-01a E0C6S46 Technical Manual

  TODO:
  - niks

*/

#include "e0c6200.h"
#include "debugger.h"


const device_type EPSON_E0C6S46 = &device_creator<e0c6s46_device>;


// internal memory maps
static ADDRESS_MAP_START(program_1k, AS_PROGRAM, 16, e0c6200_cpu_device)
	AM_RANGE(0x0000, 0x03ff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(data_64x4, AS_DATA, 8, e0c6200_cpu_device)
	AM_RANGE(0x00, 0x3f) AM_RAM
ADDRESS_MAP_END


// device definitions
e0c6s46_device::e0c6s46_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: e0c6200_cpu_device(mconfig, EPSON_E0C6S46, "E0C6S46", tag, owner, clock, 10, ADDRESS_MAP_NAME(program_1k), 6, ADDRESS_MAP_NAME(data_64x4), "e0c6s46", __FILE__)
{ }


// disasm
void e0c6200_cpu_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			break;

		default: break;
	}
}

offs_t e0c6200_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(e0c6200);
	return CPU_DISASSEMBLE_NAME(e0c6200)(this, buffer, pc, oprom, opram, options);
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void e0c6200_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_prgmask = (1 << m_prgwidth) - 1;
	m_datamask = (1 << m_datawidth) - 1;

	// zerofill
	m_op = 0;
	m_prev_op = 0;
	m_pc = 0;
	m_prev_pc = 0;
	m_npc = 0;
	m_jpc = 0;
	
	m_a = 0;
	m_b = 0;
	m_xp = m_xh = m_xl = 0;
	m_yp = m_yh = m_yl = 0;
	m_sp = 0;
	m_f = 0;

	// register for savestates
	save_item(NAME(m_op));
	save_item(NAME(m_prev_op));
	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_npc));
	save_item(NAME(m_jpc));
	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_xp));
	save_item(NAME(m_xh));
	save_item(NAME(m_xl));
	save_item(NAME(m_yp));
	save_item(NAME(m_yh));
	save_item(NAME(m_yl));
	save_item(NAME(m_sp));
	save_item(NAME(m_f));

	// register state for debugger

	m_icountptr = &m_icount;
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void e0c6200_cpu_device::device_reset()
{
}



//-------------------------------------------------
//  execute
//-------------------------------------------------

void e0c6200_cpu_device::execute_one()
{
	// legend:
	// X   = --.XH.XL 8-bit
	// Y   = --.YH.YL 8-bit
	// IX  = XP.XH.XL 12-bit index register
	// IY  = YP.YH.YL 12-bit index register
	// MX  = data memory at IX
	// MY  = data memory at IY
	// Mn  = data memory at 0-F, via 4-bit immediate param
	// r/q = 2-bit param directing to A/B/MX/MY
	// i   = 4-bit immediate param
	// e   = 8-bit immediate param
	// s   = 8-bit immediate branch destination
	
	switch (m_op & 0xf00)
	{
		// JP s: jump unconditional
		case 0x000:
			m_pc = m_jpc | (m_op & 0xff);
			break;

		// JP C,s: jump if carry
		case 0x200:
			if (m_f & 1) m_pc = m_jpc | (m_op & 0xff);
			break;

		// JP NC,s: jump if no carry
		case 0x300:
			if (~m_f & 1) m_pc = m_jpc | (m_op & 0xff);
			break;

		// JP Z,s: jump if zero
		case 0x600:
			if (m_f & 2) m_pc = m_jpc | (m_op & 0xff);
			break;

		// JP NZ,s: jump if not zero
		case 0x700:
			if (~m_f & 2) m_pc = m_jpc | (m_op & 0xff);
			break;

		// CALL s: call unconditional (on current bank)
		case 0x400:
			m_icount -= 2;
			push_pc();
			m_pc = (m_pc & 0x1000) | (m_jpc & 0x0f00) | (m_op & 0xff);
			break;

		// CALZ s: call zero page (on current bank)
		case 0x500:
			m_icount -= 2;
			push_pc();
			m_pc = (m_pc & 0x1000) | (m_op & 0xff);
			break;

		// RETD e: return from subroutine, then LBPX MX,e
		case 0x100:
			m_icount -= 7;
			pop_pc();
			// fall through!

		// LBPX MX,e: load memory with 8-bit immediate data, increment X by 2
		case 0x900:
			write_mx(m_op & 0xf); inc_x();
			write_mx(m_op >> 4 & 0xf); inc_x();
			break;

		// LD X,e: load X with 8-bit immediate data
		case 0xb00:
			m_xh = m_op >> 4 & 0xf;
			m_xl = m_op & 0xf;
			break;

		// LD Y,e: load Y with 8-bit immediate data
		case 0x800:
			m_yh = m_op >> 4 & 0xf;
			m_yl = m_op & 0xf;
			break;


		default:
			switch (m_op)
			{

		// LD r,q: load register with register
		case 0xec0: /* m_a = m_a; */ break;
		case 0xec1: m_a = m_b; break;
		case 0xec2: m_a = read_mx(); break;
		case 0xec3: m_a = read_my(); break;
		case 0xec4: m_b = m_a; break;
		case 0xec5: /* m_b = m_b; */ break;
		case 0xec6: m_b = read_mx(); break;
		case 0xec7: m_b = read_my(); break;
		case 0xec8: write_mx(m_a); break;
		case 0xec9: write_mx(m_b); break;
		case 0xeca: write_mx(read_mx()); break;
		case 0xecb: write_mx(read_my()); break;
		case 0xecc: write_my(m_a); break;
		case 0xecd: write_my(m_b); break;
		case 0xece: write_my(read_mx()); break;
		case 0xecf: write_my(read_my()); break;

		// LDPX r,q: LD r,q, then increment X
		case 0xee0: /* m_a = m_a; */ inc_x(); break;
		case 0xee1: m_a = m_b; inc_x(); break;
		case 0xee2: m_a = read_mx(); inc_x(); break;
		case 0xee3: m_a = read_my(); inc_x(); break;
		case 0xee4: m_b = m_a; inc_x(); break;
		case 0xee5: /* m_b = m_b; */ inc_x(); break;
		case 0xee6: m_b = read_mx(); inc_x(); break;
		case 0xee7: m_b = read_my(); inc_x(); break;
		case 0xee8: write_mx(m_a); inc_x(); break;
		case 0xee9: write_mx(m_b); inc_x(); break;
		case 0xeea: write_mx(read_mx()); inc_x(); break;
		case 0xeeb: write_mx(read_my()); inc_x(); break;
		case 0xeec: write_my(m_a); inc_x(); break;
		case 0xeed: write_my(m_b); inc_x(); break;
		case 0xeee: write_my(read_mx()); inc_x(); break;
		case 0xeef: write_my(read_my()); inc_x(); break;

		// LDPY r,q: LD r,q, then increment Y
		case 0xef0: /* m_a = m_a; */ inc_y(); break;
		case 0xef1: m_a = m_b; inc_y(); break;
		case 0xef2: m_a = read_mx(); inc_y(); break;
		case 0xef3: m_a = read_my(); inc_y(); break;
		case 0xef4: m_b = m_a; inc_y(); break;
		case 0xef5: /* m_b = m_b; */ inc_y(); break;
		case 0xef6: m_b = read_mx(); inc_y(); break;
		case 0xef7: m_b = read_my(); inc_y(); break;
		case 0xef8: write_mx(m_a); inc_y(); break;
		case 0xef9: write_mx(m_b); inc_y(); break;
		case 0xefa: write_mx(read_mx()); inc_y(); break;
		case 0xefb: write_mx(read_my()); inc_y(); break;
		case 0xefc: write_my(m_a); inc_y(); break;
		case 0xefd: write_my(m_b); inc_y(); break;
		case 0xefe: write_my(read_mx()); inc_y(); break;
		case 0xeff: write_my(read_my()); inc_y(); break;

		// LD Xphl/Yphl,r: load IX/IY with register
		case 0xe80: m_xp = m_a; break;
		case 0xe81: m_xp = m_b; break;
		case 0xe82: m_xp = read_mx(); break;
		case 0xe83: m_xp = read_my(); break;
		case 0xe84: m_xh = m_a; break;
		case 0xe85: m_xh = m_b; break;
		case 0xe86: m_xh = read_mx(); break;
		case 0xe87: m_xh = read_my(); break;
		case 0xe88: m_xl = m_a; break;
		case 0xe89: m_xl = m_b; break;
		case 0xe8a: m_xl = read_mx(); break;
		case 0xe8b: m_xl = read_my(); break;
		case 0xe90: m_yp = m_a; break;
		case 0xe91: m_yp = m_b; break;
		case 0xe92: m_yp = read_mx(); break;
		case 0xe93: m_yp = read_my(); break;
		case 0xe94: m_yh = m_a; break;
		case 0xe95: m_yh = m_b; break;
		case 0xe96: m_yh = read_mx(); break;
		case 0xe97: m_yh = read_my(); break;
		case 0xe98: m_yl = m_a; break;
		case 0xe99: m_yl = m_b; break;
		case 0xe9a: m_yl = read_mx(); break;
		case 0xe9b: m_yl = read_my(); break;

		// LD r,Xphl/Yphl: load register with IX/IY
		case 0xea0: m_a = m_xp; break;
		case 0xea1: m_b = m_xp; break;
		case 0xea2: write_mx(m_xp); break;
		case 0xea3: write_my(m_xp); break;
		case 0xea4: m_a = m_xh; break;
		case 0xea5: m_b = m_xh; break;
		case 0xea6: write_mx(m_xh); break;
		case 0xea7: write_my(m_xh); break;
		case 0xea8: m_a = m_xl; break;
		case 0xea9: m_b = m_xl; break;
		case 0xeaa: write_mx(m_xl); break;
		case 0xeab: write_my(m_xl); break;
		case 0xeb0: m_a = m_yp; break;
		case 0xeb1: m_b = m_yp; break;
		case 0xeb2: write_mx(m_yp); break;
		case 0xeb3: write_my(m_yp); break;
		case 0xeb4: m_a = m_yh; break;
		case 0xeb5: m_b = m_yh; break;
		case 0xeb6: write_mx(m_yh); break;
		case 0xeb7: write_my(m_yh); break;
		case 0xeb8: m_a = m_yl; break;
		case 0xeb9: m_b = m_yl; break;
		case 0xeba: write_mx(m_yl); break;
		case 0xebb: write_my(m_yl); break;

		// LD SPhl,r: load stackpointer with register
		case 0xfe0: m_sp = (m_sp & 0xf0) | m_a; break;
		case 0xfe1: m_sp = (m_sp & 0xf0) | m_b; break;
		case 0xfe2: m_sp = (m_sp & 0xf0) | read_mx(); break;
		case 0xfe3: m_sp = (m_sp & 0xf0) | read_my(); break;
		case 0xff0: m_sp = (m_sp & 0x0f) | m_a << 4; break;
		case 0xff1: m_sp = (m_sp & 0x0f) | m_b << 4; break;
		case 0xff2: m_sp = (m_sp & 0x0f) | read_mx() << 4; break;
		case 0xff3: m_sp = (m_sp & 0x0f) | read_my() << 4; break;

		// LD r,SPhl: load register with stackpointer
		case 0xfe4: m_a = m_sp >> 4 & 0xf; break;
		case 0xfe5: m_b = m_sp >> 4 & 0xf; break;
		case 0xfe6: write_mx(m_sp >> 4 & 0xf); break;
		case 0xfe7: write_my(m_sp >> 4 & 0xf); break;
		case 0xff4: m_a = m_sp & 0xf; break;
		case 0xff5: m_b = m_sp & 0xf; break;
		case 0xff6: write_mx(m_sp & 0xf); break;
		case 0xff7: write_my(m_sp & 0xf); break;

		// ADD r,q
		case 0xa80:
		case 0xa81:
		case 0xa82:
		case 0xa83:
		case 0xa84:
		case 0xa85:
		case 0xa86:
		case 0xa87:
		case 0xa88:
		case 0xa89:
		case 0xa8a:
		case 0xa8b:
		case 0xa8c:
		case 0xa8d:
		case 0xa8e:
		case 0xa8f:

		// ADC r,q
		case 0xa90:
		case 0xa91:
		case 0xa92:
		case 0xa93:
		case 0xa94:
		case 0xa95:
		case 0xa96:
		case 0xa97:
		case 0xa98:
		case 0xa99:
		case 0xa9a:
		case 0xa9b:
		case 0xa9c:
		case 0xa9d:
		case 0xa9e:
		case 0xa9f:

		// ACPX MX,r
		case 0xf28:
		case 0xf29:
		case 0xf2a:
		case 0xf2b:

		// ACPY MY,
		case 0xf2c:
		case 0xf2d:
		case 0xf2e:
		case 0xf2f:

		// SUB r,q
		case 0xaa0:
		case 0xaa1:
		case 0xaa2:
		case 0xaa3:
		case 0xaa4:
		case 0xaa5:
		case 0xaa6:
		case 0xaa7:
		case 0xaa8:
		case 0xaa9:
		case 0xaaa:
		case 0xaab:
		case 0xaac:
		case 0xaad:
		case 0xaae:
		case 0xaaf:

		// SBC r,q
		case 0xab0:
		case 0xab1:
		case 0xab2:
		case 0xab3:
		case 0xab4:
		case 0xab5:
		case 0xab6:
		case 0xab7:
		case 0xab8:
		case 0xab9:
		case 0xaba:
		case 0xabb:
		case 0xabc:
		case 0xabd:
		case 0xabe:
		case 0xabf:

		// SCPX MX,
		case 0xf38:
		case 0xf39:
		case 0xf3a:
		case 0xf3b:

		// SCPY MY,
		case 0xf3c:
		case 0xf3d:
		case 0xf3e:
		case 0xf3f:

		// CP r,q: SUB r,q, but discard result (D flag has no effect)
		case 0xf00:
		case 0xf01:
		case 0xf02:
		case 0xf03:
		case 0xf04:
		case 0xf05:
		case 0xf06:
		case 0xf07:
		case 0xf08:
		case 0xf09:
		case 0xf0a:
		case 0xf0b:
		case 0xf0c:
		case 0xf0d:
		case 0xf0e:
		case 0xf0f:

		// AND r,q: logical AND register with register (affect flags: Z)
		case 0xac0: m_icount -= 2; /* m_a &= m_a; */ set_zf(m_a); break;
		case 0xac1: m_icount -= 2; m_a &= m_b; set_zf(m_a); break;
		case 0xac2: m_icount -= 2; m_a &= read_mx(); set_zf(m_a); break;
		case 0xac3: m_icount -= 2; m_a &= read_my(); set_zf(m_a); break;
		case 0xac4: m_icount -= 2; m_b &= m_a; set_zf(m_b); break;
		case 0xac5: m_icount -= 2; /* m_b &= m_b; */ set_zf(m_b); break;
		case 0xac6: m_icount -= 2; m_b &= read_mx(); set_zf(m_b); break;
		case 0xac7: m_icount -= 2; m_b &= read_my(); set_zf(m_b); break;
		case 0xac8: m_icount -= 2; { UINT8 t = read_mx() & m_a; write_mx(t); set_zf(t); break; }
		case 0xac9: m_icount -= 2; { UINT8 t = read_mx() & m_b; write_mx(t); set_zf(t); break; }
		case 0xaca: m_icount -= 2; { UINT8 t = read_mx() & read_mx(); write_mx(t); set_zf(t); break; }
		case 0xacb: m_icount -= 2; { UINT8 t = read_mx() & read_my(); write_mx(t); set_zf(t); break; }
		case 0xacc: m_icount -= 2; { UINT8 t = read_my() & m_a; write_my(t); set_zf(t); break; }
		case 0xacd: m_icount -= 2; { UINT8 t = read_my() & m_b; write_my(t); set_zf(t); break; }
		case 0xace: m_icount -= 2; { UINT8 t = read_my() & read_mx(); write_my(t); set_zf(t); break; }
		case 0xacf: m_icount -= 2; { UINT8 t = read_my() & read_my(); write_my(t); set_zf(t); break; }

		// FAN r,q: AND r,q, but discard result
		case 0xf10: m_icount -= 2; set_zf(m_a /* & m_a */); break;
		case 0xf11: m_icount -= 2; set_zf(m_a & m_b); break;
		case 0xf12: m_icount -= 2; set_zf(m_a & read_mx()); break;
		case 0xf13: m_icount -= 2; set_zf(m_a & read_my()); break;
		case 0xf14: m_icount -= 2; set_zf(m_b & m_a); break;
		case 0xf15: m_icount -= 2; set_zf(m_b /* & m_b */); break;
		case 0xf16: m_icount -= 2; set_zf(m_b & read_mx()); break;
		case 0xf17: m_icount -= 2; set_zf(m_b & read_my()); break;
		case 0xf18: m_icount -= 2; set_zf(read_mx() & m_a); break;
		case 0xf19: m_icount -= 2; set_zf(read_mx() & m_b); break;
		case 0xf1a: m_icount -= 2; set_zf(read_mx() & read_mx()); break;
		case 0xf1b: m_icount -= 2; set_zf(read_mx() & read_my()); break;
		case 0xf1c: m_icount -= 2; set_zf(read_my() & m_a); break;
		case 0xf1d: m_icount -= 2; set_zf(read_my() & m_b); break;
		case 0xf1e: m_icount -= 2; set_zf(read_my() & read_mx()); break;
		case 0xf1f: m_icount -= 2; set_zf(read_my() & read_my()); break;

		// OR r,q: logical OR register with register (affect flags: Z)
		case 0xad0: m_icount -= 2; /* m_a |= m_a; */ set_zf(m_a); break;
		case 0xad1: m_icount -= 2; m_a |= m_b; set_zf(m_a); break;
		case 0xad2: m_icount -= 2; m_a |= read_mx(); set_zf(m_a); break;
		case 0xad3: m_icount -= 2; m_a |= read_my(); set_zf(m_a); break;
		case 0xad4: m_icount -= 2; m_b |= m_a; set_zf(m_b); break;
		case 0xad5: m_icount -= 2; /* m_b |= m_b; */ set_zf(m_b); break;
		case 0xad6: m_icount -= 2; m_b |= read_mx(); set_zf(m_b); break;
		case 0xad7: m_icount -= 2; m_b |= read_my(); set_zf(m_b); break;
		case 0xad8: m_icount -= 2; { UINT8 t = read_mx() | m_a; write_mx(t); set_zf(t); break; }
		case 0xad9: m_icount -= 2; { UINT8 t = read_mx() | m_b; write_mx(t); set_zf(t); break; }
		case 0xada: m_icount -= 2; { UINT8 t = read_mx() | read_mx(); write_mx(t); set_zf(t); break; }
		case 0xadb: m_icount -= 2; { UINT8 t = read_mx() | read_my(); write_mx(t); set_zf(t); break; }
		case 0xadc: m_icount -= 2; { UINT8 t = read_my() | m_a; write_my(t); set_zf(t); break; }
		case 0xadd: m_icount -= 2; { UINT8 t = read_my() | m_b; write_my(t); set_zf(t); break; }
		case 0xade: m_icount -= 2; { UINT8 t = read_my() | read_mx(); write_my(t); set_zf(t); break; }
		case 0xadf: m_icount -= 2; { UINT8 t = read_my() | read_my(); write_my(t); set_zf(t); break; }

		// XOR r,q: exclusive-OR register with register (affect flags: Z)
		case 0xae0: m_icount -= 2; m_a ^= m_a; set_zf(m_a); break;
		case 0xae1: m_icount -= 2; m_a ^= m_b; set_zf(m_a); break;
		case 0xae2: m_icount -= 2; m_a ^= read_mx(); set_zf(m_a); break;
		case 0xae3: m_icount -= 2; m_a ^= read_my(); set_zf(m_a); break;
		case 0xae4: m_icount -= 2; m_b ^= m_a; set_zf(m_b); break;
		case 0xae5: m_icount -= 2; m_b ^= m_b; set_zf(m_b); break;
		case 0xae6: m_icount -= 2; m_b ^= read_mx(); set_zf(m_b); break;
		case 0xae7: m_icount -= 2; m_b ^= read_my(); set_zf(m_b); break;
		case 0xae8: m_icount -= 2; { UINT8 t = read_mx() ^ m_a; write_mx(t); set_zf(t); break; }
		case 0xae9: m_icount -= 2; { UINT8 t = read_mx() ^ m_b; write_mx(t); set_zf(t); break; }
		case 0xaea: m_icount -= 2; { UINT8 t = read_mx() ^ read_mx(); write_mx(t); set_zf(t); break; }
		case 0xaeb: m_icount -= 2; { UINT8 t = read_mx() ^ read_my(); write_mx(t); set_zf(t); break; }
		case 0xaec: m_icount -= 2; { UINT8 t = read_my() ^ m_a; write_my(t); set_zf(t); break; }
		case 0xaed: m_icount -= 2; { UINT8 t = read_my() ^ m_b; write_my(t); set_zf(t); break; }
		case 0xaee: m_icount -= 2; { UINT8 t = read_my() ^ read_mx(); write_my(t); set_zf(t); break; }
		case 0xaef: m_icount -= 2; { UINT8 t = read_my() ^ read_my(); write_my(t); set_zf(t); break; }

		// RLC r: rotate register left through carry (affect flags: C, Z)
		case 0xaf0: m_icount -= 2;
		case 0xaf5: m_icount -= 2;
		case 0xafa: m_icount -= 2; read_mx();
		case 0xaff: m_icount -= 2; read_my();

		// RRC r: rotate register right through carry (affect flags: C, Z)
		case 0xe8c:
		case 0xe8d:
		case 0xe8e:
		case 0xe8f:

		// INC SP: increment stackpointer
		case 0xfdb:
			m_sp++;
			break;

		// DEC SP: decrement stackpointer
		case 0xfcb:
			m_sp--;
			break;

		// PUSH r/Xphl/Yphl/F: push register to stack
		case 0xfc0: push(m_a); break;
		case 0xfc1: push(m_b); break;
		case 0xfc2: push(read_mx()); break;
		case 0xfc3: push(read_my()); break;
		case 0xfc4: push(m_xp); break;
		case 0xfc5: push(m_xh); break;
		case 0xfc6: push(m_xl); break;
		case 0xfc7: push(m_yp); break;
		case 0xfc8: push(m_yh); break;
		case 0xfc9: push(m_yl); break;
		case 0xfca: push(m_f); break;

		// POP r/Xphl/Yphl/F: pop value from stack
		case 0xfd0: m_a = pop(); break;
		case 0xfd1: m_b = pop(); break;
		case 0xfd2: write_mx(pop()); break;
		case 0xfd3: write_my(pop()); break;
		case 0xfd4: m_xp = pop(); break;
		case 0xfd5: m_xh = pop(); break;
		case 0xfd6: m_xl = pop(); break;
		case 0xfd7: m_yp = pop(); break;
		case 0xfd8: m_yh = pop(); break;
		case 0xfd9: m_yl = pop(); break;
		case 0xfda: m_f = pop(); break;

		// RETS: return from subroutine, then skip next instruction
		case 0xfde:
			m_icount -= 7;
			pop_pc();
			m_pc = (m_pc & 0x1000) | ((m_pc + 1) & 0x0fff);
			break;

		// RET: return from subroutine
		case 0xfdf:
			m_icount -= 2;
			pop_pc();
			break;

		// JPBA: jump indirect using registers A and B
		case 0xfe8:
			m_pc = m_jpc | m_b << 4 | m_a;
			break;

		// HALT: halt (stop clock)
		case 0xff8:
			break;

		// SLP: sleep (stop oscillation)
		case 0xff9:
			break;

		// NOP5: no operation (5 clock cycles)
		case 0xffb:
			break;

		// NOP7: no operation (7 clock cycles)
		case 0xfff:
			m_icount -= 2;
			break;


		default:
			switch (m_op & 0xff0)
			{

		// LD r,i: load register with 4-bit immediate data
		case 0xe00: m_a = m_op & 0xf; break;
		case 0xe10: m_b = m_op & 0xf; break;
		case 0xe20: write_mx(m_op & 0xf); break;
		case 0xe30: write_my(m_op & 0xf); break;

		// LDPX MX,i: LD MX,i, then increment X
		case 0xe60:
			write_mx(m_op & 0xf); inc_x();
			break;

		// LDPY MY,i: LD MY,i, then increment Y
		case 0xe70:
			write_my(m_op & 0xf); inc_y();
			break;

		// LD A,Mn: load A with memory
		case 0xfa0:
			m_a = read_mn();
			break;

		// LD B,Mn: load B with memory
		case 0xfb0:
			m_b = read_mn();
			break;

		// LD Mn,A: load memory with A
		case 0xf80: 
			write_mn(m_a);
			break;

		// LD Mn,B: load memory with B
		case 0xf90:
			write_mn(m_b);
			break;

		// INC Mn: increment memory (affect flags: C, Z)
		case 0xf60:
		{
			m_icount -= 2;
			UINT8 t = read_mn();
			t = (t + 1) & 0xf;
			write_mn(t);
			m_f = (m_f & ~1) | ((t == 0) ? 1 : 0);
			set_zf(t);
			break;
		}

		// DEC Mn: decrement memory (affect flags: C, Z)
		case 0xf70:
		{
			m_icount -= 2;
			UINT8 t = read_mn();
			t = (t - 1) & 0xf;
			write_mn(t);
			m_f = (m_f & ~1) | ((t == 0xf) ? 1 : 0);
			set_zf(t);
			break;
		}

		// ADD r,i
		case 0xc00:
		case 0xc10:
		case 0xc20:
		case 0xc30:

		// ADC r,i
		case 0xc40:
		case 0xc50:
		case 0xc60:
		case 0xc70:


		// ADC XH,i
		case 0xa00:
			break;

		// ADC XL,i
		case 0xa10:
			break;

		// ADC YH,i
		case 0xa20:
			break;

		// ADC YL,i
		case 0xa30:
			break;



		// SBC r,i
		case 0xd40:
		case 0xd50:
		case 0xd60:
		case 0xd70:


		// CP r,i: SUB r,i, but discard result (D flag has no effect)
		case 0xdc0:
		case 0xdd0:
		case 0xde0:
		case 0xdf0:


		// CP XH,i
		case 0xa40:
			break;

		// CP XL,i
		case 0xa50:
			break;

		// CP YH,i
		case 0xa60:
			break;

		// CP YL,i
		case 0xa70:
			break;





		// AND r,i: logical AND register with 4-bit immediate data (affect flags: Z)
		case 0xc80: m_icount -= 2; m_a &= m_op & 0xf; set_zf(m_a); break;
		case 0xc90: m_icount -= 2; m_b &= m_op & 0xf; set_zf(m_b); break;
		case 0xca0: m_icount -= 2; { UINT8 t = read_mx() & (m_op & 0xf); write_mx(t); set_zf(t); break; }
		case 0xcb0: m_icount -= 2; { UINT8 t = read_my() & (m_op & 0xf); write_my(t); set_zf(t); break; }

		// FAN r,i: AND r,i, but discard result
		case 0xd80: m_icount -= 2; set_zf(m_a & (m_op & 0xf)); break;
		case 0xd90: m_icount -= 2; set_zf(m_b & (m_op & 0xf)); break;
		case 0xda0: m_icount -= 2; set_zf(read_mx() & (m_op & 0xf)); break;
		case 0xdb0: m_icount -= 2; set_zf(read_my() & (m_op & 0xf)); break;

		// OR r,i: logical OR register with 4-bit immediate data (affect flags: Z)
		case 0xcc0: m_icount -= 2; m_a |= m_op & 0xf; set_zf(m_a); break;
		case 0xcd0: m_icount -= 2; m_b |= m_op & 0xf; set_zf(m_b); break;
		case 0xce0: m_icount -= 2; { UINT8 t = read_mx() | (m_op & 0xf); write_mx(t); set_zf(t); break; }
		case 0xcf0: m_icount -= 2; { UINT8 t = read_my() | (m_op & 0xf); write_my(t); set_zf(t); break; }

		// XOR r,i: exclusive-OR register with 4-bit immediate data (affect flags: Z)
		case 0xd00: m_icount -= 2; m_a ^= m_op & 0xf; set_zf(m_a); break;
		case 0xd10: m_icount -= 2; m_b ^= m_op & 0xf; set_zf(m_b); break;
		case 0xd20: m_icount -= 2; { UINT8 t = read_mx() ^ (m_op & 0xf); write_mx(t); set_zf(t); break; }
		case 0xd30: m_icount -= 2; { UINT8 t = read_my() ^ (m_op & 0xf); write_my(t); set_zf(t); break; }

		// SET F,i: set flag(s), this includes opcodes SCF, SZF, SDF, EI
		case 0xf40:
			m_icount -= 2;
			m_f |= (m_op & 0xf);
			break;

		// RST F,i: reset flag(s), this includes opcodes RCF, RZF, RDF, DI
		case 0xf50:
			m_icount -= 2;
			m_f &= (m_op & 0xf);
			break;

		// PSET p: page set, used to set page/bank before a jump instruction
		case 0xe40: case 0xe50:
			m_npc = m_op << 8 & 0x1f00;
			break;


		// illegal opcode
		default:
			logerror("%s unknown opcode $%03X at $%04X\n", tag(), m_op, m_prev_pc);
			break;

			} // 0xff0
			break;

			} // 0xfff
			break;

	} // 0xf00 (big switch)
}

void e0c6200_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		// remember previous state, prepare pset-longjump
		m_prev_op = m_op;
		m_prev_pc = m_pc;
		m_jpc = ((m_prev_op & 0xfe0) == 0xe40) ? m_npc : (m_prev_pc & 0x1f00);
		
		// fetch next opcode
		debugger_instruction_hook(this, m_pc);
		m_op = m_program->read_word(m_pc << 1) & 0xfff;
		m_pc = (m_pc & 0x1000) | ((m_pc + 1) & 0x0fff);
		
		// minimal opcode time is 5 clock cycles, opcodes take 5, 7, or 12 clock cycles
		m_icount -= 5;
		
		// handle opcode
		execute_one();
	}
}

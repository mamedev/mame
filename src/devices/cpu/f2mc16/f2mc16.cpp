// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Fujitsu Micro F2MC-16 series
    Emulation by R. Belmont

    From 50,000 feet these chips look a lot like a 65C816 with no index
    registers.  As you get closer, you can see the banking includes some
    concepts from 8086 segmentation, and the interrupt handling is 68000-like.

    There are two main branches: F and L.  They appear to be compatible with
    each other as far as their extentions to the base ISA not conflicting.

***************************************************************************/

#include "emu.h"
#include "f2mc16.h"
#include "f2mc16dasm.h"

// device type definitions
DEFINE_DEVICE_TYPE(F2MC16, f2mc16_device, "f2mc16", "Fujitsu Micro F2MC-16")

// memory accessors (separated out for future expansion because vector space can be externally recognized)
#define read_8_vector(addr)     m_program->read_byte(addr)
#define read_16_vector(addr)    m_program->read_word(addr)
#define read_32_vector(addr)    m_program->read_dword(addr)

std::unique_ptr<util::disasm_interface> f2mc16_device::create_disassembler()
{
	return std::make_unique<f2mc16_disassembler>();
}

f2mc16_device::f2mc16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 24, 0)
	, m_program(nullptr)
{
	m_tmp8 = 0;
	m_tmp16 = 0;
	m_tmp32 = 0;
}

f2mc16_device::f2mc16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: f2mc16_device(mconfig, F2MC16, tag, owner, clock)
{
}

device_memory_interface::space_config_vector f2mc16_device::memory_space_config() const
{
	return space_config_vector { std::make_pair(AS_PROGRAM, &m_program_config) };
}

void f2mc16_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	set_icountptr(m_icount);

	state_add(F2MC16_PCB, "PCB", m_pcb);
	state_add(F2MC16_PC, "PC", m_pc).formatstr("%04X");
	state_add(STATE_GENPC, "GENPC", m_temp).callimport().callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_temp).callimport().callexport().noshow();
	state_add(F2MC16_PS, "PS", m_ps).formatstr("%04X");
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_ps).callimport().formatstr("%7s").noshow();
	state_add(F2MC16_DTB, "DTB", m_dtb).formatstr("%02X");
	state_add(F2MC16_ADB, "ADB", m_adb).formatstr("%02X");
	state_add(F2MC16_ACC, "AL", m_acc).formatstr("%08X");
	state_add(F2MC16_USB, "USB", m_usb).formatstr("%02X");
	state_add(F2MC16_USP, "USP", m_usp).formatstr("%04X");
	state_add(F2MC16_SSB, "SSB", m_ssb).formatstr("%02X");
	state_add(F2MC16_SSP, "SSP", m_ssp).formatstr("%04X");
	state_add(F2MC16_DPR, "DPR", m_dpr).formatstr("%02X");
	state_add(F2MC16_RW0, "RW0", m_temp).callimport().callexport().formatstr("%04X");
	state_add(F2MC16_RW1, "RW1", m_temp).callimport().callexport().formatstr("%04X");
	state_add(F2MC16_RW2, "RW2", m_temp).callimport().callexport().formatstr("%04X");
	state_add(F2MC16_RW3, "RW3", m_temp).callimport().callexport().formatstr("%04X");
	state_add(F2MC16_RW4, "RW4", m_temp).callimport().callexport().formatstr("%04X");
	state_add(F2MC16_RW5, "RW5", m_temp).callimport().callexport().formatstr("%04X");
	state_add(F2MC16_RW6, "RW6", m_temp).callimport().callexport().formatstr("%04X");
	state_add(F2MC16_RW7, "RW7", m_temp).callimport().callexport().formatstr("%04X");
	state_add(F2MC16_RL0, "RL0", m_temp).callimport().callexport().formatstr("%08X");
	state_add(F2MC16_RL1, "RL1", m_temp).callimport().callexport().formatstr("%08X");
	state_add(F2MC16_RL2, "RL2", m_temp).callimport().callexport().formatstr("%08X");
	state_add(F2MC16_RL3, "RL3", m_temp).callimport().callexport().formatstr("%08X");
	state_add(F2MC16_R0, "R0", m_temp).callimport().callexport().formatstr("%02X");
	state_add(F2MC16_R1, "R1", m_temp).callimport().callexport().formatstr("%02X");
	state_add(F2MC16_R2, "R2", m_temp).callimport().callexport().formatstr("%02X");
	state_add(F2MC16_R3, "R3", m_temp).callimport().callexport().formatstr("%02X");
	state_add(F2MC16_R4, "R4", m_temp).callimport().callexport().formatstr("%02X");
	state_add(F2MC16_R5, "R5", m_temp).callimport().callexport().formatstr("%02X");
	state_add(F2MC16_R6, "R6", m_temp).callimport().callexport().formatstr("%02X");
	state_add(F2MC16_R7, "R7", m_temp).callimport().callexport().formatstr("%02X");
}

void f2mc16_device::device_reset()
{
	m_usb = m_ssb = 0;
	m_usp = m_ssp = 0;
	m_ps &= 0x009f; // clear I and S flags
	m_ps |= F_S;    // set system stack, interrupts disabled, registers at 0x180
	m_acc = 0;
	m_dpr = 0x01;
	m_dtb = 0;

	m_pc = read_16_vector(0xffffdc);
	m_pcb = read_8_vector(0xffffde);
}

void f2mc16_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
		case STATE_GENPCBASE:
			m_pc = (m_temp & 0xffff);
			m_pcb = (m_temp >> 16) & 0xff;
			break;

		case F2MC16_RW0:  write_rwX(0, m_temp); break;
		case F2MC16_RW1:  write_rwX(1, m_temp); break;
		case F2MC16_RW2:  write_rwX(2, m_temp); break;
		case F2MC16_RW3:  write_rwX(3, m_temp); break;
		case F2MC16_RW4:  write_rwX(4, m_temp); break;
		case F2MC16_RW5:  write_rwX(5, m_temp); break;
		case F2MC16_RW6:  write_rwX(6, m_temp); break;
		case F2MC16_RW7:  write_rwX(7, m_temp); break;

		case F2MC16_RL0:  write_rlX(0, m_temp); break;
		case F2MC16_RL1:  write_rlX(1, m_temp); break;
		case F2MC16_RL2:  write_rlX(2, m_temp); break;
		case F2MC16_RL3:  write_rlX(3, m_temp); break;

		case F2MC16_R0:  write_rX(0, m_temp); break;
		case F2MC16_R1:  write_rX(1, m_temp); break;
		case F2MC16_R2:  write_rX(2, m_temp); break;
		case F2MC16_R3:  write_rX(3, m_temp); break;
		case F2MC16_R4:  write_rX(4, m_temp); break;
		case F2MC16_R5:  write_rX(5, m_temp); break;
		case F2MC16_R6:  write_rX(6, m_temp); break;
		case F2MC16_R7:  write_rX(7, m_temp); break;

		case STATE_GENFLAGS:
			break;
	}
}

void f2mc16_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case F2MC16_RW0:  m_temp = read_rwX(0); break;
		case F2MC16_RW1:  m_temp = read_rwX(1); break;
		case F2MC16_RW2:  m_temp = read_rwX(2); break;
		case F2MC16_RW3:  m_temp = read_rwX(3); break;
		case F2MC16_RW4:  m_temp = read_rwX(4); break;
		case F2MC16_RW5:  m_temp = read_rwX(5); break;
		case F2MC16_RW6:  m_temp = read_rwX(6); break;
		case F2MC16_RW7:  m_temp = read_rwX(7); break;

		case F2MC16_RL0:  m_temp = read_rlX(0); break;
		case F2MC16_RL1:  m_temp = read_rlX(1); break;
		case F2MC16_RL2:  m_temp = read_rlX(2); break;
		case F2MC16_RL3:  m_temp = read_rlX(3); break;

		case F2MC16_R0:  m_temp = read_rX(0); break;
		case F2MC16_R1:  m_temp = read_rX(1); break;
		case F2MC16_R2:  m_temp = read_rX(2); break;
		case F2MC16_R3:  m_temp = read_rX(3); break;
		case F2MC16_R4:  m_temp = read_rX(4); break;
		case F2MC16_R5:  m_temp = read_rX(5); break;
		case F2MC16_R6:  m_temp = read_rX(6); break;
		case F2MC16_R7:  m_temp = read_rX(7); break;

		case STATE_GENPC:
		case STATE_GENPCBASE:
			m_temp = m_pc;
			m_temp |= (m_pcb << 16);
			break;
	}
}

void f2mc16_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
		switch(entry.index()) {
		case STATE_GENFLAGS:
		case F2MC16_PS:
				str = string_format("%c%c%c%c%c%c%c",
												m_ps & F_I ? 'I' : '.',
												m_ps & F_S ? 'S' : '.',
												m_ps & F_T ? 'T' : '.',
												m_ps & F_N ? 'N' : '.',
												m_ps & F_Z ? 'Z' : '.',
												m_ps & F_V ? 'V' : '.',
												m_ps & F_C ? 'C' : '.');
				break;
		}
}

void f2mc16_device::execute_run()
{
	while (m_icount > 0)
	{
		u8 opcode = read_8((m_pcb<<16) | m_pc);

		debugger_instruction_hook((m_pcb<<16) | m_pc);

		switch (opcode)
		{
		case 0x00:  // NOP
			m_icount--;
			break;

		case 0x03:
	//      util::stream_format(stream, "NEG    A");
			break;

		case 0x04:
	//      stream << "PCB ";
			break;

		case 0x05:
	//      stream << "DTB ";
			break;

		case 0x06:
	//      stream << "ADB ";
			break;

		case 0x07:
	//      stream << "SPB ";
			break;

		case 0x08:
	//      util::stream_format(stream, "LINK   #$%02x", operand);
			break;

		case 0x09:
	//      stream << "UNLINK";
			break;

		// MOV RP, #imm8
		case 0x0a:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1)) & 0x1f;
			m_ps &= 0xe0ff;
			m_ps |= (m_tmp8<<8);
			m_pc += 2;
			m_icount -= 2;
			break;

		case 0x0b:
	//      stream << "NEGW   A";
			break;

		case 0x0c:
	//      stream << "LSLW   A";
			break;

		case 0x0e:
	//      stream << "ASRW   A";
			break;

		case 0x0f:
	//      stream << "LSRW   A";
			break;

		case 0x10:
	//      stream << "CMR ";
			break;

		case 0x11:
	//      stream << "NCC ";
			break;

		case 0x12:
	//      stream << "SUBDC  A";
			break;

		case 0x14:
	//      stream << "EXT";
			break;

		case 0x15:
	//      stream << "ZEXT";
			break;

		case 0x16:
	//      stream << "SWAP";
			break;

		case 0x17:
	//      util::stream_format(stream, "ADDSP  #$%02x", operand);
			break;

		case 0x18:
	//      util::stream_format(stream, "ADDL   A, #$%08x", opcodes.r16(pc+1) | (opcodes.r16(pc+3)<<16));
			break;

		case 0x19:
	//      util::stream_format(stream, "SUBL   A, #$%08x", opcodes.r16(pc+1) | (opcodes.r16(pc+3)<<16));
			break;

		// MOV ILM, #imm8
		case 0x1a:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1)) & 7;
			m_ps &= 0x1fff;
			m_ps |= (m_tmp8<<13);
			m_pc += 2;
			m_icount -= 2;
			break;

		case 0x1c:
	//      stream << "EXTW";
			break;

		case 0x1d:
	//      stream << "ZEXTW";
			break;

		case 0x1e:
	//      stream << "SWAPW";
			break;

		case 0x20:
	//      util::stream_format(stream, "ADD    A, $%02x", operand);
			break;

		case 0x21:
	//      util::stream_format(stream, "SUB    A, $%02x", operand);
			break;

		case 0x22:
	//      stream << "ADDC   A";
			break;

		case 0x23:
	//      stream << "CMP    A";
			break;

		// AND CCR, #imm8
		case 0x24:
			m_tmp16 = read_8((m_pcb<<16) | (m_pc+1)) | 0xff80;
			m_ps &= m_tmp16;
			m_pc += 2;
			m_icount -= 3;
			break;

		// OR CCR, #imm8
		case 0x25:
			m_tmp16 = read_8((m_pcb<<16) | (m_pc+1)) & 0x7f;
			m_ps |= m_tmp16;
			m_pc += 2;
			m_icount -= 3;
			break;

		case 0x26:
	//      stream << "DIVU   A";
			break;

		case 0x27:
	//      stream << "MULU   A";
			break;

		case 0x28:
	//      stream << "ADDW   A";
			break;

		case 0x29:
	//      stream << "SUBW   A";
			break;

		case 0x2a:
	//      util::stream_format(stream, "CBNE   A, #$%02X, $%04X", operand, (s8)opcodes.r8(pc+2) + (pc & 0xffff) + 3);
			break;

		case 0x2b:
	//      stream << "CMPW   A";
			break;

		case 0x2c:
	//      stream << "ANDW   A";
			break;

		case 0x2d:
	//      stream << "ORW    A";
			break;

		case 0x2e:
	//      stream << "XORW   A";
			break;

		case 0x2f:
	//      stream << "MULUW  A";
			break;

		case 0x30:
	//      util::stream_format(stream, "ADD    A, #$%02x", operand);
			break;

		case 0x31:
	//      util::stream_format(stream, "SUB    A, #$%02x", operand);
			break;

		case 0x32:
	//      stream << "SUBC   A";
			break;

		case 0x33:
	//      util::stream_format(stream, "CMP    A, #$%02x", operand);
			break;

		case 0x34:
	//      util::stream_format(stream, "AND    A, #$%02x", operand);
			break;

		case 0x35:
	//      util::stream_format(stream, "OR     A, #$%02x", operand);
			break;

		case 0x36:
	//      util::stream_format(stream, "XOR    A, #$%02x", operand);
			break;

		case 0x37:
	//      stream << "NOT    A";
			break;

		case 0x38:
	//      util::stream_format(stream, "ADDW   A, #$%04x", opcodes.r16(pc+1));
			break;

		case 0x39:
	//      util::stream_format(stream, "SUBW   A, #$%04x", opcodes.r16(pc+1));
			break;

		case 0x3a:
	//      util::stream_format(stream, "CWBNE  A, #$%04X, $%04X", opcodes.r16(pc+1), (s8)opcodes.r8(pc+3) + (pc & 0xffff) + 4);
			break;

		case 0x3b:
	//      util::stream_format(stream, "CMPW   A, #$%04x", opcodes.r16(pc+1));
			break;

		case 0x3f:
	//      stream << "NOTW   A";
			break;

		// MOV A, #imm8
		case 0x42:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1));
			m_acc <<= 16;
			m_acc |= m_tmp8;
			setNZ_8(m_tmp8);
			m_pc += 2;
			m_icount -= 2;
			break;

		case 0x46:
	//      util::stream_format(stream, "MOVW   A, SP");
			break;

		// MOVW SP, A
		case 0x47:
			if (m_ps & F_S)
			{
				m_ssp = m_acc & 0xffff;
			}
			else
			{
				m_usp = m_acc & 0xffff;
			}
			m_icount -= 1;
			m_pc += 1;
			break;

		// MOVW A, #imm16
		case 0x4a:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+1));
			m_acc <<= 16;
			m_acc |= m_tmp16;
			m_icount -= 2;
			m_pc += 3;
			break;

		case 0x4b:
	//      util::stream_format(stream, "MOVL   A, #$%04x", opcodes.r32(pc+1));
			break;

		case 0x4c:
	//      stream << "PUSHW  A";
			break;

		case 0x4d:
	//      stream << "PUSHW  AH";
			break;

		case 0x4e:
	//      stream << "PUSHW  PS";
			break;

		case 0x4f:
	//      util::stream_format(stream, "PUSHW   ");
	//      for (int i = 0; i < 8; i++)
	//      {
	//          if (operand & (1<<i))
	//          {
	//              util::stream_format(stream, "RW%d, ", i);
	//          }
	//      }
			break;

		case 0x52:
	//      util::stream_format(stream, "MOV    A, $%04x", opcodes.r16(pc+1));
			break;

		case 0x53:
	//      util::stream_format(stream, "MOV    $%04x, A", opcodes.r16(pc+1));
			break;

		case 0x57:
	//      util::stream_format(stream, "MOVX   A, $%04x", opcodes.r16(pc+1));
			break;

		case 0x5a:
	//      util::stream_format(stream, "MOVW   A, $%04x", opcodes.r16(pc+1));
			break;

		case 0x5b:
	//      util::stream_format(stream, "MOVW   $%04x, A", opcodes.r16(pc+1));
			break;

		case 0x5c:
	//      stream << "POPW   A";
			break;

		case 0x5d:
	//      stream << "POPW   AH";
			break;

		case 0x5e:
	//      stream << "POPW   PS";
			break;

		case 0x5f:
	//      util::stream_format(stream, "POPW    ");
	//      for (int i = 0; i < 8; i++)
	//      {
	//          if (operand & (1<<i))
	//          {
	//              util::stream_format(stream, "RW%d, ", i);
	//          }
	//      }
			break;

		case 0x60: //branch_helper(stream, "BRA   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
			break;

		case 0x61:
//          stream << "JMP    @A";
			break;

		case 0x62:
//          util::stream_format(stream, "JMP    #$%04x", opcodes.r16(pc+1));
			break;

		case 0x63:
	//      util::stream_format(stream, "JMPP   #$%06x", opcodes.r8(pc+3)<<16|opcodes.r8(pc+2)<<8|opcodes.r8(pc+1));
			break;

		case 0x64:
	//      util::stream_format(stream, "CALL   #$%04x", opcodes.r16(pc+1));
			break;

		case 0x65:
	//      util::stream_format(stream, "CALLP  #$%06x", opcodes.r8(pc+3)<<16|opcodes.r8(pc+2)<<8|opcodes.r8(pc+1));
			break;

		case 0x66:
	//      util::stream_format(stream, "RETP");
			break;

		case 0x67:
	//      stream << "RET";
			break;

		case 0x6b:
	//      stream << "RETI";
			break;

		case 0x6e:  // string instructions
			opcodes_str6e(read_8((m_pcb<<16) | (m_pc+1)));
			break;

		case 0x6f:  // 2-byte instructions
			opcodes_2b6f(read_8((m_pcb<<16) | (m_pc+1)));
			break;

		case 0x70:  // ea-type instructions
			fatalerror("Unknown F2MC EA70 opcode %02x\n", opcode);
			break;

		case 0x71:  // ea-type instructions
			opcodes_ea71(read_8((m_pcb<<16) | (m_pc+1)));
			break;

		case 0x72:  // ea-type instructions
			fatalerror("Unknown F2MC EA72 opcode %02x\n", opcode);
			break;

		case 0x73:  // ea-type instructions
			fatalerror("Unknown F2MC EA73 opcode %02x\n", opcode);
			break;

		case 0x74:  // ea-type instructions
			fatalerror("Unknown F2MC EA74 opcode %02x\n", opcode);
			break;

		case 0x75:  // ea-type instructions
			fatalerror("Unknown F2MC EA75 opcode %02x\n", opcode);
			break;

		case 0x76:  // ea-type instructions
			fatalerror("Unknown F2MC EA76 opcode %02x\n", opcode);
			break;

		case 0x77:  // ea-type instructions
			fatalerror("Unknown F2MC EA77 opcode %02x\n", opcode);
			break;

		case 0x78:  // ea-type instructions
			fatalerror("Unknown F2MC EA78 opcode %02x\n", opcode);
			break;

		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
	//      util::stream_format(stream, "MOV    A, R%d", (opcode & 0x7));
			break;

		case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
	//      util::stream_format(stream, "MOVW   A, RW%d", (opcode & 0x7));
			break;

		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
	//      util::stream_format(stream, "MOV    R%d, A", (opcode & 0x7));
			break;

		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
	//      util::stream_format(stream, "MOVW   RW%d, A", (opcode & 0x7));
			break;

		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
	//      util::stream_format(stream, "MOVW   RW%d, #$%02x", (opcode & 0x7), operand);
			break;

		// MOVW RWx, #imm16
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+1));
			write_rwX(opcode & 7, m_tmp16);
			setNZ_16(m_tmp16);
			m_icount -= 2;
			m_pc += 3;
			break;

		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
	//      util::stream_format(stream, "MOVX   A, R%d", (opcode & 0x7));
			break;

		case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
	//      util::stream_format(stream, "MOVW   A, @R%d+%02x", (opcode & 0x7), operand);
			break;

		case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
	//      util::stream_format(stream, "MOVX   A, @R%d+%02x", (opcode & 0x7), operand);
			break;

		case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
	//      util::stream_format(stream, "MOVW   @R%d+%02x, A", (opcode & 0x7), operand);
			break;

		case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
	//      util::stream_format(stream, "MOVN   A, #$%01x", (opcode & 0xf));
			break;

		case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
		case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
	  //    util::stream_format(stream, "CALL   #$%01x", (opcode & 0xf));
			break;

		case 0xf0: //branch_helper(stream, "BEQ   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
		case 0xf1: //branch_helper(stream, "BNE   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
		case 0xf2: //branch_helper(stream, "BC    ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
		case 0xf3: //branch_helper(stream, "BNC   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
		case 0xf4: //branch_helper(stream, "BN    ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
		case 0xf5: //branch_helper(stream, "BP    ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
		case 0xf6: //branch_helper(stream, "BV    ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
		case 0xf7: //branch_helper(stream, "BNV   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
		case 0xf8: //branch_helper(stream, "BT    ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
		case 0xf9: //branch_helper(stream, "BNT   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
		case 0xfa: //branch_helper(stream, "BLT   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
		case 0xfb: //branch_helper(stream, "BGE   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
		case 0xfc: //branch_helper(stream, "BLE   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
		case 0xfd: //branch_helper(stream, "BGT   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
		case 0xfe: //branch_helper(stream, "BLS   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
		case 0xff: //branch_helper(stream, "BHI   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
			break;

		default:
			fatalerror("Unknown F2MC opcode %02x\n", opcode);
			break;

		}
	}   // while icount > 0
}

void f2mc16_device::opcodes_str6e(u8 operand)
{
	switch (operand)
	{
		// MOVSI ADB, DTB
		case 0x09:
			if (read_rwX(0) > 0)
			{
				m_tmp8 = read_8((m_dtb<<16) | (m_acc & 0xffff));
				write_8((m_adb<<16) | ((m_acc >> 16) & 0xffff), m_tmp8);
				write_rwX(0, read_rwX(0) - 1);
				m_icount -= 8;
			}
			else
			{
				m_pc += 2;
				m_icount -= 5;
			}
			break;

		default:
			fatalerror("Unknown F2MC STR6E opcode %02x (PC=%x)\n", operand, (m_pcb<<16) | m_pc);
			break;
	}
}

void f2mc16_device::opcodes_2b6f(u8 operand)
{
	switch (operand)
	{
		// MOV DTB, A
		case 0x10:
			m_dtb = m_acc & 0xff;
			m_pc += 2;
			m_icount -= 1;
			break;

		// MOV ADB, A
		case 0x11:
			m_adb = m_acc & 0xff;
			m_pc += 2;
			m_icount -= 1;
			break;

		// MOV SSB, A
		case 0x12:
			m_ssb = m_acc & 0xff;
			m_pc += 2;
			m_icount -= 1;
			break;

		// MOV USB, A
		case 0x13:
			m_usb = m_acc & 0xff;
			m_pc += 2;
			m_icount -= 1;
			break;

		// MOV DPR, A
		case 0x14:
			m_dpr = m_acc & 0xff;
			m_pc += 2;
			m_icount -= 1;
			break;

		default:
			fatalerror("Unknown F2MC 2B6F opcode %02x (PC=%x)\n", operand, (m_pcb<<16) | m_pc);
			break;
	}
}

void f2mc16_device::opcodes_ea71(u8 operand)
{
	switch (operand)
	{
		// MOV @RWx, #imm8
		case 0xc8: case 0xc9: case 0xca: case 0xcb:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp16 = read_rwX(operand & 7);
			write_8(getRWbank(operand & 7, m_tmp16), m_tmp8);
			m_pc += 3;
			m_icount -= 2;
			break;

		default:
			fatalerror("Unknown F2MC EA71 opcode %02x (PC=%x)\n", operand, (m_pcb<<16) | m_pc);
			break;
	}
}

void f2mc16_device::execute_set_input(int inputnum, int state)
{
}

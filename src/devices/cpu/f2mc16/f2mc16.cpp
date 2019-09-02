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
	m_tmp64 = 0;
	m_prefix_valid = false;
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
	state_add(F2MC16_ACC, "A", m_acc).formatstr("%08X");
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

	save_item(NAME(m_pc));
	save_item(NAME(m_usp));
	save_item(NAME(m_ssp));
	save_item(NAME(m_ps));
	save_item(NAME(m_tmp16));
	save_item(NAME(m_tmpea));
	save_item(NAME(m_tmp16aux));
	save_item(NAME(m_pcb));
	save_item(NAME(m_dtb));
	save_item(NAME(m_usb));
	save_item(NAME(m_ssb));
	save_item(NAME(m_adb));
	save_item(NAME(m_dpr));
	save_item(NAME(m_tmp8));
	save_item(NAME(m_prefix));
	save_item(NAME(m_acc));
	save_item(NAME(m_temp));
	save_item(NAME(m_tmp32));
	save_item(NAME(m_tmp32aux));
	save_item(NAME(m_tmp64));
	save_item(NAME(m_prefix_valid));
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

	for (int &entry : m_vector_level)
	{
		entry = 7;
	}
	m_outstanding_irqs = 0;

	m_pc = read_16_vector(0xffffdc);
	m_pcb = read_8_vector(0xffffde);

	m_prefix_valid = false;
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
		if (m_outstanding_irqs)
		{
			int cpulevel = m_ps >> 13;

			for (int irq = 0; irq < 256; irq++)
			{
				if (m_vector_level[irq] < cpulevel)
				{
					take_irq(irq, m_vector_level[irq]);
					break;
				}
			}
		}

		//m_icount--;

		u8 opcode = read_8((m_pcb<<16) | m_pc);

		debugger_instruction_hook((m_pcb<<16) | m_pc);
		switch (opcode)
		{
		case 0x00:  // NOP
			m_icount--;
			m_pc++;
			break;

		case 0x03:
	//      util::stream_format(stream, "NEG    A");
			break;

		// PCB prefix
		case 0x04:
			m_prefix = m_pcb;
			m_prefix_valid = true;
			m_pc++;
			break;

		// DTB prefix
		case 0x05:
			m_prefix = m_dtb;
			m_prefix_valid = true;
			m_pc++;
			break;

		// ADB prefix
		case 0x06:
			m_prefix = m_adb;
			m_prefix_valid = true;
			m_pc++;
			break;

		// SPB prefix
		case 0x07:
			if (m_ps & F_S)
			{
				m_prefix = m_ssb;
			}
			else
			{
				m_prefix = m_usb;
			}
			m_prefix_valid = true;
			m_pc++;
			break;

		// LINK #imm8
		case 0x08:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1));
			if (m_ps & F_S)
			{
				m_ssp-=2;
				write_16((m_ssb << 16) | m_ssp, read_rwX(3));
				write_rwX(3, m_ssp);
				m_ssp -= m_tmp8;
			}
			else
			{
				m_usp-=2;
				write_16((m_usb << 16) | m_usp, read_rwX(3));
				write_rwX(3, m_usp);
				m_ssp -= m_tmp8;
			}
			m_pc += 2;
			m_icount -= 6;
			break;

		// UNLINK
		case 0x09:
			if (m_ps & F_S)
			{
				m_ssp = read_rwX(3);
			}
			else
			{
				m_usp = read_rwX(3);
			}
			write_rwX(3, pull_16());
			m_pc++;
			m_icount -= 5;
			break;

		// MOV RP, #imm8
		case 0x0a:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1)) & 0x1f;
			m_ps &= 0xe0ff;
			m_ps |= (m_tmp8<<8);
			m_pc += 2;
			m_icount -= 2;
			break;

		// NEGW A
		case 0x0b:
			m_tmp32 = doSUB_32(0, m_acc&0xffff);
			m_acc &= 0xffff0000;
			m_acc |= m_tmp32 & 0xffff;
			setNZ_16(m_acc & 0xffff);
			m_pc++;
			m_icount -= 2;
			break;

		// LSLW A
		case 0x0c:
			m_tmp16 = m_acc & 0xffff;
			m_ps &= ~F_C;
			if (m_tmp16 & 0x8000)
			{
				m_ps |= F_C;
			}
			m_tmp16 <<= 1;
			setNZ_16(m_tmp16);
			m_acc &= 0xffff0000;
			m_acc |= m_tmp16;
			m_pc++;
			m_icount -= 2;
			break;

		// ASRW A
		case 0x0e:
			m_tmp16 = m_acc & 0xffff;

			// T is set if either carry or T are set beforehand
			if ((m_ps & F_C) || (m_ps & F_T))
			{
				m_ps |= F_T;
			}
			// C becomes the previous LSB
			m_ps &= ~F_C;
			if (m_tmp16 & 1)
			{
				m_ps |= F_C;
			}

			if (m_tmp16 & 0x8000)
			{
				m_tmp16 >>= 1;
				m_tmp16 |= 0x8000;
			}
			else
			{
				m_tmp16 >>= 1;
				m_tmp16 &= ~0x8000;
			}
			setNZ_16(m_tmp16);

			m_acc &= 0xffff0000;
			m_acc |= m_tmp16;

			m_pc++;
			m_icount -= 2;
			break;

		// LSRW A
		case 0x0f:
			m_tmp16 = m_acc & 0xffff;
			// T is set if either carry or T are set beforehand
			if ((m_ps & F_C) || (m_ps & F_T))
			{
				m_ps |= F_T;
			}
			// C becomes the previous LSB
			m_ps &= ~F_C;
			if (m_tmp16 & 1)
			{
				m_ps |= F_C;
			}

			m_tmp16 >>= 1;
			setNZ_16(m_tmp16);

			m_acc &= 0xffff0000;
			m_acc |= m_tmp16;
			m_pc++;
			m_icount-=2;
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

		// ZEXT
		case 0x15:
			m_acc &= 0xffff00ff;
			m_ps &= ~(F_N|F_Z);
			if ((m_acc & 0xffff) == 0)
			{
				m_ps |= F_Z;
			}
			m_pc++;
			m_icount--;
			break;

		case 0x16:
	//      stream << "SWAP";
			break;

		// ADDSP #imm8
		case 0x17:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1));
			if (m_ps & F_S)
			{
				m_ssp += m_tmp8;
			}
			else
			{
				m_usp += m_tmp8;
			}
			m_pc += 2;
			m_icount -= 3;
			break;

		// ADDL A, #imm32
		case 0x18:
			m_tmp32 = read_32((m_pcb << 16) | (m_pc+1));
			m_acc = doADD_32(m_acc, m_tmp32);
			m_pc += 5;
			m_icount -= 4;
			break;

		// SUBL A, #imm32
		case 0x19:
			m_tmp32 = read_32((m_pcb << 16) | (m_pc+1));
			m_acc = doSUB_32(m_acc, m_tmp32);
			m_pc += 5;
			m_icount -= 4;
			break;

		// MOV ILM, #imm8
		case 0x1a:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1)) & 7;
			m_ps &= 0x1fff;
			m_ps |= (m_tmp8<<13);
			m_pc += 2;
			m_icount -= 2;
			break;

		// EXTW
		case 0x1c:
			m_acc &= 0xffff;
			m_ps &= ~(F_N|F_Z);
			if (m_acc & 0x8000)
			{
				m_acc |= 0xffff0000;
				m_ps |= F_N;
			}
			else
			{
				m_ps |= F_Z;
			}
			m_pc++;
			m_icount-=2;
			break;

		// ZEXTW
		case 0x1d:
			m_acc &= 0xffff;
			setNZ_32(m_acc);
			m_pc++;
			m_icount--;
			break;

		// SWAPW
		case 0x1e:
			m_tmp16 = (m_acc & 0xffff);
			m_acc >>= 16;
			m_acc |= (m_tmp16 << 16);
			m_pc++;
			m_icount -= 2;
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

		// CMP A
		case 0x23:
			doCMP_16(m_acc>>16, m_acc&0xffff);
			m_pc++;
			m_icount--;
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

		// ADDW A
		case 0x28:
			m_tmp16 = doADD_16((m_acc>>16) & 0xffff, m_acc & 0xffff);
			m_acc &= 0xffff0000;
			m_acc |= m_tmp16;
			m_pc++;
			m_icount -= 2;
			break;

		// SUBW A
		case 0x29:
			m_tmp16 = doSUB_16((m_acc>>16) & 0xffff, m_acc & 0xffff);
			m_acc &= 0xffff0000;
			m_acc |= m_tmp16;
			m_pc++;
			m_icount -= 2;
			break;

		// CBNE A, #imm8
		case 0x2a:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1));
			doCMP_8(m_acc & 0xff, m_tmp8);
			if (m_ps & F_Z) // they're equal
			{
				m_pc += 3;
				m_icount -= 4;
			}
			else
			{
				m_pc++;
				take_branch();
			}
			break;

		// CMPW A
		case 0x2b:
			doCMP_16((m_acc>>16) & 0xffff, m_acc & 0xffff);
			m_pc++;
			m_icount--;
			break;

		// ANDW A
		case 0x2c:
			m_tmp32 = m_acc;
			m_tmp32 >>= 16;
			m_tmp32 |= 0xffff0000;
			m_acc &= m_tmp32;
			setNZ_16(m_acc & 0xffff);
			m_ps &= ~F_V;
			m_pc++;
			m_icount -= 2;
			break;

		// ORW A
		case 0x2d:
			m_tmp16 = (m_acc>>16) & 0xffff;
			m_acc |= m_tmp16;
			setNZ_16(m_acc & 0xffff);
			m_ps &= ~F_V;
			m_pc++;
			m_icount -= 2;
			break;

		// XORW A
		case 0x2e:
			m_tmp16 = (m_acc>>16) & 0xffff;
			m_acc ^= m_tmp16;
			setNZ_16(m_acc & 0xffff);
			m_ps &= ~F_V;
			m_pc++;
			m_icount -= 2;
			break;

		// MULUW A
		case 0x2f:
			m_tmp16 = (m_acc >> 16) & 0xffff;
			if (m_tmp16 == 0)
			{
				m_acc = 0;
				m_icount -= 3;
			}
			else
			{
				m_acc = m_tmp16 * (m_acc & 0xffff);
				m_icount -= 11;
			}
			m_pc++;
			break;

		// ADD A, #imm8
		case 0x30:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1));
			m_tmp8 = doADD_8(m_acc & 0xff, m_tmp8);
			m_acc &= ~0xff;
			m_acc |= m_tmp8;
			m_pc += 2;
			m_icount -= 2;
			break;

		// SUB A, #imm8
		case 0x31:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1));
			m_tmp8 = doSUB_8(m_acc & 0xff, m_tmp8);
			m_acc &= ~0xff;
			m_acc |= m_tmp8;
			m_pc += 2;
			m_icount -= 2;
			break;

		case 0x32:
	//      stream << "SUBC   A";
			break;

		// CMP A, #imm8
		case 0x33:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1));
			doCMP_8(m_acc & 0xff, m_tmp8);
			m_pc += 2;
			m_icount -= 2;
			break;

		// AND A, #imm8
		case 0x34:
			m_tmp32 = read_8((m_pcb<<16) | (m_pc+1));
			m_tmp32 |= 0xffffff00;
			m_acc &= m_tmp32;
			setNZ_8(m_acc & 0xff);
			m_ps &= ~F_V;
			m_pc += 2;
			m_icount -= 2;
			break;

		case 0x35:
	//      util::stream_format(stream, "OR     A, #$%02x", operand);
			break;

		// XOR A, #imm8
		case 0x36:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1));
			m_tmp16 = m_acc & 0xffff;
			m_acc <<= 16;
			m_acc |= m_tmp16;
			m_acc ^= m_tmp8;
			setNZ_8(m_acc & 0xff);
			m_ps &= ~F_V;
			m_pc += 2;
			m_icount -= 2;
			break;

		// NOT A
		case 0x37:
			m_acc ^= 0xff;
			setNZ_8(m_acc & 0xff);
			m_ps &= ~F_V;
			m_pc++;
			m_icount -= 2;
			break;

		// ADDW A, #imm16
		case 0x38:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+1));
			m_tmp16 = doADD_16(m_acc & 0xffff, m_tmp16);
			m_acc &= 0xffff0000;
			m_acc |= m_tmp16;
			m_pc += 3;
			m_icount -= 2;
			break;

		// SUBW A, #imm16
		case 0x39:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+1));
			m_tmp16 = doSUB_16(m_acc & 0xffff, m_tmp16);
			m_acc &= 0xffff0000;
			m_acc |= m_tmp16;
			m_pc += 3;
			m_icount -= 2;
			break;

		// CWBNE A, #imm16, disp8
		case 0x3a:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+1));
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+3));
			doCMP_16(m_acc & 0xffff, m_tmp16);
			if (m_ps & F_Z)
			{
				m_pc += 4;
				m_icount -= 4;
			}
			else
			{
				m_pc = m_pc + 4 + (s8)m_tmp8;
				m_icount -= 5;
			}
			break;

		// CMPW A, #imm16
		case 0x3b:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+1));
			doCMP_16(m_acc & 0xffff, m_tmp16);
			m_pc += 3;
			m_icount -= 2;
			break;

		// ANDW A, #imm16
		case 0x3c:
			m_tmp32 = read_16((m_pcb<<16) | (m_pc+1));
			m_tmp32 |= 0xffff0000;
			m_acc &= m_tmp32;
			setNZ_16(m_acc & 0xffff);
			m_ps &= ~F_V;
			m_pc += 3;
			m_icount -= 2;
			break;

		// ORW A, #imm16
		case 0x3d:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+1));
			m_acc |= m_tmp16;
			setNZ_16(m_acc & 0xffff);
			m_ps &= ~F_V;
			m_pc += 3;
			m_icount -= 2;
			break;

		// XORW A, #imm16
		case 0x3e:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+1));
			m_acc ^= m_tmp16;
			setNZ_16(m_acc & 0xffff);
			m_ps &= ~F_V;
			m_pc += 3;
			m_icount -= 2;
			break;

		// NOTW A
		case 0x3f:
			m_acc ^= 0xffff;
			setNZ_16(m_acc & 0xffff);
			m_ps &= ~F_V;
			m_pc++;
			m_icount -= 2;
			break;

		// MOV A, dir
		case 0x40:
			m_tmp32 = read_8((m_pcb<<16) | (m_pc+1));
			m_tmp32 |= (m_dpr<<8) | (m_dtb<<8);
			m_tmp8 = read_8(m_tmp32);
			m_acc <<= 16;
			m_acc |= m_tmp8;
			setNZ_16(m_acc & 0xff);
			m_pc += 2;
			m_icount -= 3;
			break;

		// MOV dir, A
		case 0x41:
			m_tmp32 = read_8((m_pcb<<16) | (m_pc+1));
			m_tmp32 |= (m_dpr<<8) | (m_dtb<<8);
			write_8(m_tmp32, m_acc & 0xff);
			setNZ_8(m_acc & 0xff);
			m_pc += 2;
			m_icount -= 3;
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

		// MOVX A, #imm8
		case 0x43:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1));
			m_acc <<= 16;
			m_acc |= m_tmp8;
			if (m_acc & 0x80)
			{
				m_acc |= 0xff00;
			}
			setNZ_8(m_tmp8);
			m_pc += 2;
			m_icount -= 2;
			break;

		// MOV dir, #imm8
		case 0x44:
			m_tmp32 = read_8((m_pcb<<16) | (m_pc+1));
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp32 |= (m_dpr<<8) | (m_dtb<<8);
			write_8(m_tmp32, m_tmp8);
			m_pc += 3;
			m_icount -= 5;
			break;

		// MOVW A, SP
		case 0x46:
			m_acc <<= 16;
			if (m_ps & F_S)
			{
				m_acc |= m_ssp;
			}
			else
			{
				m_acc |= m_usp;
			}
			m_pc++;
			m_icount--;
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
			m_pc++;
			m_icount--;
			break;

		// MOVW A, dir
		case 0x48:
			m_tmp32 = read_8((m_pcb<<16) | (m_pc+1));
			m_tmp32 |= (m_dpr<<8) | (m_dtb<<8);
			m_tmp16 = read_16(m_tmp32);
			m_acc <<= 16;
			m_acc |= m_tmp16;
			setNZ_16(m_acc & 0xffff);
			m_pc += 2;
			m_icount -= 3;
			break;

		// MOVW dir, A
		case 0x49:
			m_tmp32 = read_8((m_pcb<<16) | (m_pc+1));
			m_tmp32 |= (m_dpr<<8) | (m_dtb<<8);
			write_16(m_tmp32, m_acc & 0xffff);
			setNZ_16(m_acc & 0xffff);
			m_pc += 2;
			m_icount -= 3;
			break;

		// MOVW A, #imm16
		case 0x4a:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+1));
			m_acc <<= 16;
			m_acc |= m_tmp16;
			setNZ_16(m_tmp16);
			m_icount -= 2;
			m_pc += 3;
			break;

		// MOVL A, #imm16
		case 0x4b:
			m_acc = read_32((m_pcb<<16) | (m_pc + 1));
			setNZ_32(m_acc);
			m_icount -= 3;
			m_pc += 5;
			break;

		// PUSHW A
		case 0x4c:
			push_16(m_acc & 0xffff);
			m_icount -= 4;
			m_pc++;
			break;

		// PUSHW AH
		case 0x4d:
			push_16((m_acc >> 16) & 0xffff);
			m_icount -= 4;
			m_pc++;
			break;

		// PUSHW PS
		case 0x4e:
			push_16(m_ps);
			m_icount -= 4;
			m_pc++;
			break;

		// PUSHW (register bitmap)
		case 0x4f:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1));
			for (int i = 0; i < 8; i++)
			{
				if (m_tmp8 & (1<<i))
				{
					push_16(read_rwX(i));
					m_icount -= 3;
				}
			}
			m_pc += 2;
			if (m_tmp8 == 0)
			{
				m_icount -= 8;
			}
			else
			{
				m_icount -= 29;
			}
			break;

		// MOV A, adr16
		case 0x52:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+1));
			m_acc <<= 16;
			if (m_prefix_valid)
			{
				m_prefix_valid = false;
				m_acc |= read_8((m_prefix<<16) | m_tmp16);
			}
			else
			{
				m_acc |= read_8((m_dtb<<16) | m_tmp16);
			}
			setNZ_8(m_acc & 0xff);
			m_pc += 3;
			m_icount -= 4;
			break;

		// MOV adr16, A
		case 0x53:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+1));
			if (m_prefix_valid)
			{
				m_prefix_valid = false;
				write_8((m_prefix<<16) | m_tmp16, m_acc & 0xff);
			}
			else
			{
				write_8((m_dtb<<16) | m_tmp16, m_acc & 0xff);
			}
			setNZ_8(m_acc & 0xff);
			m_pc += 3;
			m_icount -= 4;
			break;

		case 0x57:
	//      util::stream_format(stream, "MOVX   A, $%04x", opcodes.r16(pc+1));
			break;

		// MOVW A, adr16
		case 0x5a:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+1));
			m_acc <<= 16;
			if (m_prefix_valid)
			{
				m_prefix_valid = false;
				m_acc |= read_16((m_prefix<<16) | m_tmp16);
			}
			else
			{
				m_acc |= read_16((m_dtb<<16) | m_tmp16);
			}
			setNZ_16(m_acc & 0xffff);
			m_pc += 3;
			m_icount -= 4;
			break;

		// MOVW adr16, A
		case 0x5b:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+1));
			if (m_prefix_valid)
			{
				m_prefix_valid = false;
				write_16((m_prefix<<16) | m_tmp16, m_acc & 0xffff);
			}
			else
			{
				write_16((m_dtb<<16) | m_tmp16, m_acc & 0xffff);
			}
			setNZ_16(m_acc & 0xfff);
			m_pc += 3;
			m_icount -= 4;
			break;

		// POPW A
		case 0x5c:
			m_acc <<= 16;
			m_acc |= pull_16();
			m_pc++;
			m_icount -= 3;
			break;

		// POPW AH
		case 0x5d:
			m_acc &= 0xffff;
			m_acc |= (pull_16() << 16);
			m_pc++;
			m_icount -= 3;
			break;

		// POPW PS
		case 0x5e:
			m_ps = pull_16();
			m_pc++;
			m_icount -= 4;
			break;

		// POPW register list
		case 0x5f:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1));
			for (int i = 7; i >= 0; i--)
			{
				if (m_tmp8 & (1<<i))
				{
					write_rwX(i, pull_16());
					m_icount -= 3;
				}
			}
			m_pc += 2;
			if (m_tmp8 == 0)
			{
				m_icount -= 7;
			}
			break;

		// BRA
		case 0x60:
			take_branch();
			break;

		case 0x61:
//          stream << "JMP    @A";
			break;

		// JMP #imm16
		case 0x62:
			m_pc = read_16((m_pcb<<16) | (m_pc+1));
			m_icount -= 3;
			break;

		case 0x63:
	//      util::stream_format(stream, "JMPP   #$%06x", opcodes.r8(pc+3)<<16|opcodes.r8(pc+2)<<8|opcodes.r8(pc+1));
			break;

		case 0x64:
	//      util::stream_format(stream, "CALL   #$%04x", opcodes.r16(pc+1));
			break;

		// CALLP #imm24
		case 0x65:
			push_16(m_pcb);
			push_16(m_pc+4);
			m_tmp8 = read_8((m_pcb << 16) | (m_pc + 3));
			m_tmp16 = read_16((m_pcb << 16) | (m_pc + 1));
			m_pcb = m_tmp8;
			m_pc = m_tmp16;
			m_icount -= 10;
			break;

		// RETP
		case 0x66:
			m_pc = pull_16();
			m_pcb = pull_16() & 0xff;
			m_icount -= 5;
			break;

		case 0x67:
	//      stream << "RET";
			break;

		// RETI
		case 0x6b:
			{
				bool bFoundVec = false;

				// there's an IRQ chaining facility, let's do it
				if (m_outstanding_irqs)
				{
					int cpulevel = peek_stack_16() >> 13;
					for (int irq = 0; irq < 256; irq++)
					{
						if (m_vector_level[irq] < cpulevel)
						{
							m_ps = read_16((m_ssb << 16) | m_ssp);
							m_ps |= F_S;
							m_ps &= ~0x7000;
							m_ps |= (m_vector_level[irq] & 7) << 13;

							u32 uVecAddr = 0xfffffc - (irq * 4);
							m_pc = read_16(uVecAddr);
							m_pcb = read_8(uVecAddr + 2);
							bFoundVec = true;
							printf("RETI vector chain to %02x%04x\n", m_pcb, m_pc);
							break;
						}
					}
				}

				// if no new IRQ was found or could be dispatched by the level
				if (!bFoundVec)
				{
					m_ps = pull_16_ssp();
					m_pc = pull_16_ssp();
					m_tmp16 = pull_16_ssp();
					m_pcb = m_tmp16 & 0xff;
					m_dtb = m_tmp16 >> 8;
					m_tmp16 = pull_16_ssp();
					m_adb = m_tmp16 & 0xff;
					m_dpr = m_tmp16 >> 8;
					m_acc = 0;
					m_acc = pull_16_ssp();
					m_acc |= (pull_16_ssp() << 16);
					m_icount -= 17;
				}
			}
			break;

		case 0x6c:  // bit operation instructions
			opcodes_bo6c(read_8((m_pcb<<16) | (m_pc+1)));
			break;

		case 0x6e:  // string instructions
			opcodes_str6e(read_8((m_pcb<<16) | (m_pc+1)));
			break;

		case 0x6f:  // 2-byte instructions
			opcodes_2b6f(read_8((m_pcb<<16) | (m_pc+1)));
			break;

		case 0x70:  // ea-type instructions
			opcodes_ea70(read_8((m_pcb<<16) | (m_pc+1)));
			break;

		case 0x71:  // ea-type instructions
			opcodes_ea71(read_8((m_pcb<<16) | (m_pc+1)));
			break;

		case 0x72:  // ea-type instructions
			opcodes_ea72(read_8((m_pcb<<16) | (m_pc+1)));
			break;

		case 0x73:  // ea-type instructions
			opcodes_ea73(read_8((m_pcb<<16) | (m_pc+1)));
			break;

		case 0x74:  // ea-type instructions
			opcodes_ea74(read_8((m_pcb<<16) | (m_pc+1)));
			break;

		case 0x75:  // ea-type instructions
			opcodes_ea75(read_8((m_pcb<<16) | (m_pc+1)));
			break;

		case 0x76:  // ea-type instructions
			opcodes_ea76(read_8((m_pcb<<16) | (m_pc+1)));
			break;

		case 0x77:  // ea-type instructions
			opcodes_ea77(read_8((m_pcb<<16) | (m_pc+1)));
			break;

		case 0x78:  // ea-type instructions
			opcodes_ea78(read_8((m_pcb<<16) | (m_pc+1)));
			break;

		case 0x7a:  // ea RWi-type instructions
			opcodes_rwi7a(read_8((m_pcb<<16) | (m_pc+1)));
			break;

		case 0x7b:  // ea RWi-type instructions
			opcodes_rwi7b(read_8((m_pcb<<16) | (m_pc+1)));
			break;

		// MOV A, Rx
		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
			m_acc <<= 16;
			m_acc |= read_rX(opcode & 7);
			setNZ_8(m_acc & 0xff);
			m_pc++;
			m_icount -= 2;
			break;

		// MOVW A, RWx
		case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
			m_acc <<= 16;
			m_acc |= read_rwX(opcode & 7);
			setNZ_16(m_acc & 0xff);
			m_pc++;
			m_icount -= 2;
			break;

		// MOV Rx, A
		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
			write_rX(opcode & 0x7, m_acc & 0xff);
			setNZ_8(m_acc & 0xff);
			m_pc++;
			m_icount -= 2;
			break;

		// MOVW RWx, A
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			write_rwX(opcode & 0x7, m_acc & 0xffff);
			setNZ_16(m_acc & 0xffff);
			m_pc++;
			m_icount -= 2;
			break;

		// MOV Rx, #imm8
		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1));
			write_rX(opcode & 7, m_tmp8);
			setNZ_8(m_tmp8);
			m_icount -= 2;
			m_pc += 2;
			break;

		// MOVW RWx, #imm16
		case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+1));
			write_rwX(opcode & 7, m_tmp16);
			setNZ_16(m_tmp16);
			m_icount -= 2;
			m_pc += 3;
			break;

		// MOVX A, Rx
		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
	//      util::stream_format(stream, "MOVX   A, R%d", (opcode & 0x7));
			break;

		// MOVW A, @RWx + disp8
		case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1));
			m_tmp16 = read_rwX(opcode & 0x7) + (s8)m_tmp8;
			m_acc <<= 16;
			m_acc |= read_16(getRWbank(opcode & 0x7, m_tmp16));
			setNZ_16(m_acc & 0xffff);
			m_pc += 2;
			m_icount -= 10;
			break;

		// MOVX A, @Rx + disp8
		case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1));
			m_tmpea = read_rwX(opcode & 0x7) + (s8)m_tmp8;
			m_tmp8 = read_8(getRWbank(opcode & 7, m_tmpea));
			m_tmp16 = (m_tmp8 & 0x80) ? (m_tmp8 | 0xff00) : m_tmp8;
			m_acc <<= 16;
			m_acc |= m_tmp16;
			m_pc += 2;
			m_icount -= 3;
			break;

		// MOVW @RWx + disp8, A
		case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+1));
			m_tmp16 = read_rwX(opcode & 0x7) + (s8)m_tmp8;
			write_16(getRWbank(opcode & 0x7, m_tmp16), m_acc & 0xffff);
			setNZ_16(m_acc & 0xffff);
			m_pc += 2;
			m_icount -= 10;
			break;

		// MOVN A, #imm4
		case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
			m_acc <<= 16;
			m_acc |= (opcode & 0xf);
			m_ps &= ~(F_N|F_Z);
			if (!(opcode & 0xf))
			{
				m_ps |= F_Z;
			}
			m_pc++;
			m_icount--;
			break;

		case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
		case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
	  //    util::stream_format(stream, "CALLV  #$%01x", (opcode & 0xf));
			break;

		// BEQ
		case 0xf0:
			if (m_ps & F_Z)
			{
				take_branch();
			}
			else
			{
				m_pc += 2;
				m_icount -= 3;
			}
			break;

		// BNE
		case 0xf1:
			if (!(m_ps & F_Z))
			{
				take_branch();
			}
			else
			{
				m_pc += 2;
				m_icount -= 3;
			}
			break;

		// BC
		case 0xf2:
			if (m_ps & F_C)
			{
				take_branch();
			}
			else
			{
				m_pc += 2;
				m_icount -= 3;
			}
			break;

		// BNC
		case 0xf3:
			if (!(m_ps & F_C))
			{
				take_branch();
			}
			else
			{
				m_pc += 2;
				m_icount -= 3;
			}
			break;

		//
		case 0xf4:
			if (m_ps & F_N)
			{
				take_branch();
			}
			else
			{
				m_pc += 2;
				m_icount -= 3;
			}
			break;

		// BP
		case 0xf5:
			if (!(m_ps & F_N))
			{
				take_branch();
			}
			else
			{
				m_pc += 2;
				m_icount -= 3;
			}
			break;

		// BV
		case 0xf6:
			if (m_ps & F_V)
			{
				take_branch();
			}
			else
			{
				m_pc += 2;
				m_icount -= 3;
			}
			break;

		// BNV
		case 0xf7:
			if (!(m_ps & F_V))
			{
				take_branch();
			}
			else
			{
				m_pc += 2;
				m_icount -= 3;
			}
			break;

		// BT
		case 0xf8:;
			if (m_ps & F_T)
			{
				take_branch();
			}
			else
			{
				m_pc += 2;
				m_icount -= 3;
			}
			break;

		// BNT
		case 0xf9:
			if (!(m_ps & F_T))
			{
				take_branch();
			}
			else
			{
				m_pc += 2;
				m_icount -= 3;
			}
			break;

		// BLT
		case 0xfa:
			{
				u8 n = (m_ps & F_N) ? 1 : 0;
				u8 v = (m_ps & F_V) ? 1 : 0;
				if ((v ^ n) == 1)
				{
					take_branch();
				}
				else
				{
					m_pc += 2;
					m_icount -= 3;
				}
			}
			break;

		// BGE
		case 0xfb:
			{
				u8 n = (m_ps & F_N) ? 1 : 0;
				u8 v = (m_ps & F_V) ? 1 : 0;
				if ((v ^ n) == 0)
				{
					take_branch();
				}
				else
				{
					m_pc += 2;
					m_icount -= 3;
				}
			}
			break;

		// BLE
		case 0xfc:
			{
				u8 n = (m_ps & F_N) ? 1 : 0;
				u8 v = (m_ps & F_V) ? 1 : 0;
				if (((v ^ n) == 1) || (m_ps & F_Z))
				{
					take_branch();
				}
				else
				{
					m_pc += 2;
					m_icount -= 3;
				}
			}
			break;

		// BGT
		case 0xfd:
			{
				u8 n = (m_ps & F_N) ? 1 : 0;
				u8 v = (m_ps & F_V) ? 1 : 0;
				if (((v ^ n) == 1) || !(m_ps & F_Z))
				{
					take_branch();
				}
				else
				{
					m_pc += 2;
					m_icount -= 3;
				}
			}
			break;

		//BLS
		case 0xfe:
			if ((m_ps & F_C) || (m_ps & F_Z))
			{
				take_branch();
			}
			else
			{
				m_pc += 2;
				m_icount -= 3;
			}
			break;

		// BHI
		case 0xff:
			if (!(m_ps & F_C) || !(m_ps & F_Z))
			{
				take_branch();
			}
			else
			{
				m_pc += 2;
				m_icount -= 3;
			}
			break;

		default:
			fatalerror("Unknown F2MC opcode %02x, PC=%02x%04x\n", opcode, m_pcb, m_pc);
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
				u16 al = (m_acc & 0xffff);
				u16 ah = (m_acc >> 16) & 0xffff;
				m_tmp8 = read_8((m_dtb<<16) | al);
				write_8((m_adb<<16) | ah, m_tmp8);
				al++;
				ah++;
				m_acc = (ah<<16) | al;
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

void f2mc16_device::opcodes_bo6c(u8 operand)
{
	switch (operand)
	{
		// CLRB adr16, bit
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			m_tmp16 = read_16((m_pcb << 16) | (m_pc + 2));
			if (m_prefix_valid)
			{
				m_prefix_valid = false;
				m_tmp8 = read_8((m_prefix<<16) | m_tmp16);
				m_tmp8 |= (1 << (operand & 7));
				write_8((m_prefix<<16) | m_tmp16, m_tmp8);
			}
			else
			{
				m_tmp8= read_8((m_dtb<<16) | m_tmp16);
				m_tmp8 |= (1 << (operand & 7));
				write_8((m_dtb<<16) | m_tmp16, m_tmp8);
			}
			m_pc += 2;
			m_icount -= 7;
			break;

		// SETB adr16, bit
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			m_tmp16 = read_16((m_pcb << 16) | (m_pc + 2));
			if (m_prefix_valid)
			{
				m_prefix_valid = false;
				m_tmp8 = read_8((m_prefix<<16) | m_tmp16);
				m_tmp8 |= (1 << (operand & 7));
				write_8((m_prefix<<16) | m_tmp16, m_tmp8);
			}
			else
			{
				m_tmp8= read_8((m_dtb<<16) | m_tmp16);
				m_tmp8 |= (1 << (operand & 7));
				write_8((m_dtb<<16) | m_tmp16, m_tmp8);
			}
			m_pc += 2;
			m_icount -= 7;
			break;

		// BBC dir8, bit, disp8
		case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
			m_tmp32 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp32 |= (m_dpr<<8) | (m_dtb<<8);
			m_tmp8 = read_8(m_tmp32);
			m_tmp8 = read_8((m_pcb << 16) | (m_pc + 3));
			m_ps &= ~F_Z;
			m_pc += 4;
			if (!(read_8(m_tmp32) & (1 << (operand & 7))))
			{
				m_ps |= F_Z;
				m_pc += (s8)m_tmp8;
				m_icount--;
			}
			m_icount -= 7;
			break;

		// BBC adr16, bit, disp8
		case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			m_tmp16 = read_16((m_pcb << 16) | (m_pc + 2));
			m_tmp8 = read_8((m_pcb << 16) | (m_pc + 4));
			m_ps &= ~F_Z;
			m_pc += 5;
			if (m_prefix_valid)
			{
				m_prefix_valid = false;
				m_tmp32 = (m_prefix<<16) | m_tmp16;
			}
			else
			{
				m_tmp32 = (m_dtb<<16) | m_tmp16;
			}

			if (!(read_8(m_tmp32) & (1 << (operand & 7))))
			{
				m_ps |= F_Z;
				m_pc += (s8)m_tmp8;
				m_icount--;
			}
			m_icount -= 7;
			break;

		// BBS adr16, bit, disp8
		case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			m_tmp16 = read_16((m_pcb << 16) | (m_pc + 2));
			m_tmp8 = read_8((m_pcb << 16) | (m_pc + 4));
			m_ps &= ~F_Z;
			m_pc += 5;
			if (m_prefix_valid)
			{
				m_prefix_valid = false;
				m_tmp32 = (m_prefix<<16) | m_tmp16;
			}
			else
			{
				m_tmp32 = (m_dtb<<16) | m_tmp16;
			}

			if (read_8(m_tmp32) & (1 << (operand & 7)))
			{
				m_pc += (s8)m_tmp8;
				m_icount--;
			}
			else
			{
				m_ps |= F_Z;
			}
			m_icount -= 7;
			break;

		default:
			fatalerror("Unknown F2MC 6C bit-op opcode %02x (PC=%x)\n", operand, (m_pcb<<16) | m_pc);
			break;
	}
}

void f2mc16_device::opcodes_2b6f(u8 operand)
{
	switch (operand)
	{
		// MOV A, DTB
		case 0x00:
			m_acc <<= 16;
			m_acc |= m_dtb;
			m_pc += 2;
			m_icount -= 2;
			break;

		// MOV A, ADB
		case 0x01:
			m_acc <<= 16;
			m_acc |= m_adb;
			m_pc += 2;
			m_icount -= 1;
			break;

		// MOV A, SSB
		case 0x02:
			m_acc <<= 16;
			m_acc |= m_ssb;
			m_pc += 2;
			m_icount -= 1;
			break;

		// MOV A, USB
		case 0x03:
			m_acc <<= 16;
			m_acc |= m_usb;
			m_pc += 2;
			m_icount -= 1;
			break;

		// MOV A, DPR
		case 0x04:
			m_acc <<= 16;
			m_acc |= m_dpr;
			m_pc += 2;
			m_icount -= 2;
			break;

		// MOV A, PCB
		case 0x06:
			m_acc <<= 16;
			m_acc |= m_pcb;
			m_pc += 2;
			m_icount -= 1;
			break;

		// LSLW A, R0
		case 0x0c:
			m_tmp8 = read_rX(0);
			if (m_tmp8 == 0)
			{
				m_icount -= 6;
			}
			else
			{
				m_tmp16 = m_acc & 0xffff;
				for (u8 count = 0; count < m_tmp8; count++)
				{
					m_ps &= ~F_C;
					m_ps |= (m_tmp16 & 0x8000) ? F_C : 0;
					m_tmp16 <<= 1;
					m_icount -= 5;
				}
				setNZ_16(m_tmp16);
				m_acc &= 0xffff0000;
				m_acc |= m_tmp16;
			}
			m_pc += 2;
			break;

		// ASRW A, R0
		case 0x0e:
			m_tmp8 = read_rX(0);
			if (m_tmp8 == 0)
			{
				// docs don't say if N is cleared in this case or not
				m_ps &= ~(F_C|F_T);
				m_ps |= F_Z;
				m_pc += 2;
				m_icount -= 6;
			}
			else
			{
				m_tmp16 = m_acc & 0xffff;
				for (u8 count = 0; count < m_tmp8; count++)
				{
					// T is set if either carry or T are set beforehand
					if ((m_ps & F_C) || (m_ps & F_T))
					{
						m_ps |= F_T;
					}
					// C becomes the previous LSB
					m_ps &= ~F_C;
					if (m_tmp16 & 1)
					{
						m_ps |= F_C;
					}

					if (m_tmp16 & 0x8000)
					{
						m_tmp16 >>= 1;
						m_tmp16 |= 0x8000;
					}
					else
					{
						m_tmp16 >>= 1;
						m_tmp16 &= ~0x8000;
					}
					setNZ_16(m_tmp16);
					m_icount -= 5;
				}

				m_acc &= 0xffff0000;
				m_acc |= m_tmp16;
				m_pc += 2;
			}
			break;

		// LSRW A, R0
		case 0xf:
			m_tmp8 = read_rX(0);
			if (m_tmp8 == 0)
			{
				// docs don't say if N is cleared in this case or not
				m_ps &= ~(F_C|F_T);
				m_ps |= F_Z;
				m_pc += 2;
				m_icount -= 6;
			}
			else
			{
				m_tmp16 = m_acc & 0xffff;
				for (u8 count = 0; count < m_tmp8; count++)
				{
					// T is set if either carry or T are set beforehand
					if ((m_ps & F_C) || (m_ps & F_T))
					{
						m_ps |= F_T;
					}
					// C becomes the previous LSB
					m_ps &= ~F_C;
					if (m_tmp16 & 1)
					{
						m_ps |= F_C;
					}

					m_tmp16 >>= 1;
					setNZ_16(m_tmp16);
					m_icount -= 5;
				}

				m_acc &= 0xffff0000;
				m_acc |= m_tmp16;
				m_pc += 2;
			}
			break;

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

		// MOV @AL, AH
		case 0x15:
			if (m_prefix_valid)
			{
				m_prefix_valid = false;
				write_8((m_prefix<<16) | (m_acc & 0xffff), (m_acc>>16) & 0xff);
			}
			else
			{
				write_8((m_dtb<<16) | (m_acc & 0xffff), (m_acc>>16) & 0xff);
			}
			m_pc += 2;
			m_icount -= 3;
			break;

		// LSLL A, R0
		case 0x1c:
			m_tmp8 = read_rX(0);
			m_icount -= 6;  // 6 cycles base
			if (m_tmp8 == 0)
			{
				m_ps &= ~F_C;
			}
			else
			{
				m_icount -= 6;
				for (u8 count = 0; count < m_tmp8; count++)
				{
					m_ps &= ~F_C;
					m_ps |= (m_acc & 0x80000000) ? F_C : 0;
					m_acc <<= 1;
					m_icount --;    // 1 additional cycle per iteration
				}
				setNZ_32(m_acc);
			}
			m_pc += 2;
			break;

		// MOV @RLx + #disp8, A
		case 0x30: case 0x32: case 0x34: case 0x36:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp32 = read_rlX((operand>>1) & 0x3) + (s8)m_tmp8;
			write_8(m_tmp32, m_acc & 0xff);
			setNZ_8(m_acc & 0xff);
			m_pc += 3;
			m_icount -= 10;
			break;

		// MOVW @RLx + #disp8, A
		case 0x38: case 0x3a: case 0x3c: case 0x3e:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmpea = read_rlX((operand>>1) & 0x3) + (s8)m_tmp8;
			write_8(m_tmpea, m_acc & 0xffff);
			setNZ_8(m_acc & 0xffff);
			m_pc += 3;
			m_icount -= 10;
			break;

		// MOV A, @RLx + disp8
		case 0x40: case 0x42: case 0x44: case 0x46:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp32 = read_rlX((operand>>1) & 0x3) + (s8)m_tmp8;
			m_acc <<= 16;
			m_acc |= read_8(m_tmp32);
			setNZ_8(m_acc & 0xff);
			m_pc += 3;
			m_icount -= 10;
			break;

		// MOVW A, @RLx + disp8
		case 0x48: case 0x4a: case 0x4c: case 0x4e:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp32 = read_rlX((operand>>1) & 0x3) + (s8)m_tmp8;
			m_acc <<= 16;
			m_acc |= read_16(m_tmp32);
			setNZ_8(m_acc & 0xffff);
			m_pc += 3;
			m_icount -= 10;
			break;

		default:
			fatalerror("Unknown F2MC 2B6F opcode %02x (PC=%x)\n", operand, (m_pcb<<16) | m_pc);
			break;
	}
}

void f2mc16_device::opcodes_ea70(u8 operand)
{
	switch (operand)
	{
		// ADDL A, RLx
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
			m_acc = doADD_32(m_acc, read_rlX((operand>>1) & 3));
			m_pc += 3;
			m_icount -= 7;
			break;

		// ADDL A, @RWx + disp8
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
			m_tmp8 = read_32((m_pcb<<16) | (m_pc+2));
			m_tmpea = read_rwX(operand & 7) + (s8)m_tmp8;
			m_tmp32 = read_32(getRWbank(operand & 7, m_tmpea));
			m_acc = doADD_32(m_acc, m_tmp32);
			m_pc += 3;
			m_icount -= 7;
			break;

		// SUBL A, RLx
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
			m_acc = doSUB_32(m_acc, read_rlX((operand>>1) & 3));
			m_pc += 2;
			m_icount -= 7;
			break;

		// SUBL A, @RWx + disp8
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
			m_tmp8 = read_32((m_pcb<<16) | (m_pc+2));
			m_tmpea = read_rwX(operand & 7) + (s8)m_tmp8;
			m_tmp32 = read_32(getRWbank(operand & 7, m_tmpea));
			m_acc = doSUB_32(m_acc, m_tmp32);
			m_pc += 3;
			m_icount -= 7;
			break;

		// CMPL A, RLx
		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
			doCMP_32(m_acc, read_rlX((operand>>1) & 3));
			m_pc += 2;
			m_icount -= 7;
			break;

		// CMPL A, @RWx + disp8
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
			m_tmp8 = read_32((m_pcb<<16) | (m_pc+2));
			m_tmpea = read_rwX(operand & 7) + (s8)m_tmp8;
			m_tmp32 = read_32(getRWbank(operand & 7, m_tmpea));
			doCMP_32(m_acc, m_tmp32);
			m_pc += 3;
			m_icount -= 7;
			break;

		// ANDL A, RLx
		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
			m_tmp32 = read_rlX((operand>>1) & 3);
			m_acc &= m_tmp32;
			setNZ_32(m_acc);
			m_ps &= ~F_V;
			m_pc += 2;
			m_icount -= 7;
			break;

		// ANDL A, @RWx + disp8
		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
			m_tmp8 = read_32((m_pcb<<16) | (m_pc+2));
			m_tmpea = read_rwX(operand & 7) + (s8)m_tmp8;
			m_tmp32 = read_32(getRWbank(operand & 7, m_tmpea));
			m_acc &= m_tmp32;
			setNZ_32(m_acc);
			m_ps &= ~F_V;
			m_pc += 3;
			m_icount -= 7;
			break;

		default:
			fatalerror("Unknown F2MC EA70 opcode %02x (PC=%x)\n", operand, (m_pcb<<16) | m_pc);
			break;
	}
}

void f2mc16_device::opcodes_ea71(u8 operand)
{
	switch (operand)
	{
		// CALLP @RWx + disp8
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
			push_16(m_pcb);
			push_16(m_pc+4);
			m_tmp8 = read_8((m_pcb << 16) | (m_pc + 2));
			m_tmp16 = read_rwX(operand & 7) + (s8)m_tmp8;
			m_tmp32 = read_32(getRWbank(operand & 7, m_tmp16));
			m_pc = m_tmp32 & 0xffff;
			m_pcb = (m_tmp32 >> 16) & 0xff;
			m_icount -= 11;
			break;

		// INCL RLx
		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
			m_tmp64 = read_rlX((operand>>1) & 3);
			m_tmp64++;
			write_rlX((operand>>1) & 3, m_tmp64&0xffffffff);
			setNZ_32(m_tmp64 & 0xffffffff);
			m_ps &= ~F_V;
			if (m_tmp64 & 0x100000000)
			{
				m_ps |= F_V;
			}
			m_pc += 2;
			m_icount -= 7;
			break;

		// INCL @RWx + disp8
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp16 = read_rwX(operand & 7) + (s8)m_tmp8;
			m_tmp64 = read_32(getRWbank(operand & 7, m_tmp16));
			m_tmp64++;
			write_32(getRWbank(operand & 7, m_tmp16), m_tmp64 & 0xffffffff);
			setNZ_32(m_tmp64 & 0xffffffff);
			m_ps &= ~F_V;
			if (m_tmp64 & 0x100000000)
			{
				m_ps |= F_V;
			}
			m_pc += 3;
			m_icount -= 7;
			break;

		// MOVL A, RLx
		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
			m_acc = read_rlX((operand>>1) & 3);
			setNZ_32(m_acc);
			m_pc += 2;
			m_icount -= 4;
			break;

		// MOVL A, @RWx + disp8
		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmpea = read_rwX(operand & 7) + (s8)m_tmp8;
			m_acc = read_32(getRWbank(operand & 7, m_tmpea));
			setNZ_32(m_acc);
			m_pc += 3;
			m_icount -= 4;
			break;

		// MOVL A, @RWx + disp16
		case 0x98: case 0x99: case 0x9a: case 0x9b:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+2));
			m_tmpea = read_rwX(operand & 3) + (s16)m_tmp16;
			m_acc = read_32(getRWbank(operand & 3, m_tmpea));
			setNZ_32(m_acc);
			m_pc += 4;
			m_icount -= 4;
			break;

		// MOVL A, adr16
		case 0x9f:
			m_tmp32 = read_16((m_pcb<<16) | (m_pc+2));
			if (m_prefix_valid)
			{
				m_prefix_valid = false;
				m_tmp32 |= (m_prefix << 16);
			}
			else
			{
				m_tmp32 |= (m_dtb << 16);
			}
			m_acc = read_32(m_tmp32);
			setNZ_32(m_acc);
			m_pc += 4;
			m_icount -= 4;
			break;

		// MOVL RLx, A
		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
			write_rlX((operand & 7) >> 1, m_acc);
			setNZ_32(m_acc);
			m_pc += 2;
			m_icount -= 4;
			break;

		// MOVL @RWx + disp8, A
		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp16 = read_rwX(operand & 7) + (s8)m_tmp8;
			write_32(getRWbank(operand & 7, m_tmp16), m_acc);
			setNZ_32(m_acc);
			m_pc += 3;
			m_icount -= 4;
			break;

		// MOVL addr16, A
		case 0xbf:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+2));
			if (m_prefix_valid)
			{
				m_prefix_valid = false;
				write_32((m_prefix<<16) | m_tmp16, m_acc);
			}
			else
			{
				write_32((m_dtb<<16) | m_tmp16, m_acc);
			}
			setNZ_32(m_acc);
			m_pc += 4;
			m_icount -= 5;
			break;

		// MOV @RWx, #imm8
		case 0xc8: case 0xc9: case 0xca: case 0xcb:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp16 = read_rwX(operand & 7);
			write_8(getRWbank(operand & 7, m_tmp16), m_tmp8);
			setNZ_8(m_tmp8);
			m_pc += 3;
			m_icount -= 2;
			break;

		// MOVEA A, @RWx + disp8
		case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
			m_acc <<= 16;
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp16 = read_rwX(operand & 7);
			m_tmp16 += (s8)m_tmp8;
			m_acc |= m_tmp16;
			m_pc += 3;
			m_icount -= 1;
			break;

		default:
			fatalerror("Unknown F2MC EA71 opcode %02x (PC=%x)\n", operand, (m_pcb<<16) | m_pc);
			break;
	}
}

void f2mc16_device::opcodes_ea72(u8 operand)
{
	switch (operand)
	{
		// INC @RWx + disp8
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp16 = read_rwX(operand & 7) + (s8)m_tmp8;
			m_tmp32 = read_16(getRWbank(operand & 7, m_tmp16));
			m_tmp32++;
			write_8(getRWbank(operand & 7, m_tmp16), m_tmp32 & 0xff);
			setNZ_8(m_tmp32 & 0xff);
			m_ps &= ~F_V;
			if (m_tmp32 & 0x100)
			{
				m_ps |= F_V;
			}
			m_pc += 3;
			m_icount -= 5;
			break;

		// INC addr16
		case 0x5f:
			m_tmp32 = read_16((m_pcb<<16) | (m_pc+2));
			if (m_prefix_valid)
			{
				m_prefix_valid = false;
				m_tmp32 |= (m_prefix << 16);
			}
			else
			{
				m_tmp32 |= (m_dtb << 16);
			}
			m_tmp16 = read_8(m_tmp32);
			m_tmp16++;
			write_8(m_tmp32, m_tmp16 & 0xff);
			setNZ_8(m_tmp16 & 0xff);
			m_ps &= ~F_V;
			if (m_tmp16 & 0x100)
			{
				m_ps |= F_V;
			}
			m_pc += 4;
			m_icount -= 5;
			break;

		// MOV  A, @RWx + disp8
		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp16 = read_rwX(operand & 7) + (s8)m_tmp8;
			m_acc <<= 16;
			m_acc |= read_8(getRWbank(operand & 7, m_tmp16));
			setNZ_8(m_acc & 0xff);
			m_pc += 3;
			m_icount -= 5;
			break;

		// MOV @RWx + disp8, A
		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp16 = read_rwX(operand & 7) + (s8)m_tmp8;
			write_8(getRWbank(operand & 7, m_tmp16), m_acc & 0xff);
			setNZ_8(m_acc & 0xff);
			m_pc += 3;
			m_icount -= 10;
			break;

		default:
			fatalerror("Unknown F2MC EA72 opcode %02x (PC=%x)\n", operand, (m_pcb<<16) | m_pc);
			break;
	}
}

void f2mc16_device::opcodes_ea73(u8 operand)
{
	switch (operand)
	{
		// INCW @RWx + disp8
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp16 = read_rwX(operand & 7) + (s8)m_tmp8;
			m_tmp32 = read_16(getRWbank(operand & 7, m_tmp16));
			m_tmp32++;
			write_16(getRWbank(operand & 7, m_tmp16), m_tmp32 & 0xffff);
			setNZ_16(m_tmp32 & 0xffff);
			m_ps &= ~F_V;
			if (m_tmp32 & 0x10000)
			{
				m_ps |= F_V;
			}
			m_pc += 3;
			m_icount -= 5;
			break;

		// INCW addr16
		case 0x5f:
			m_tmp32 = read_16((m_pcb<<16) | (m_pc+2));
			if (m_prefix_valid)
			{
				m_prefix_valid = false;
				m_tmp32 |= (m_prefix << 16);
			}
			else
			{
				m_tmp32 |= (m_dtb << 16);
			}
			m_tmp32aux = read_16(m_tmp32);
			m_tmp32aux++;
			write_16(m_tmp32, m_tmp32aux & 0xffff);
			setNZ_16(m_tmp32aux & 0xffff);
			m_ps &= ~F_V;
			if (m_tmp32aux & 0x10000)
			{
				m_ps |= F_V;
			}
			m_pc += 4;
			m_icount -= 5;
			break;

		// DECW @RWx + disp8
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp16 = read_rwX(operand & 7) + (s8)m_tmp8;
			m_tmp32 = read_16(getRWbank(operand & 7, m_tmp16));
			m_tmp32aux = m_tmp32;
			m_tmp32--;
			write_16(getRWbank(operand & 7, m_tmp16), m_tmp32 & 0xffff);
			setNZ_16(m_tmp32 & 0xffff);
			m_ps &= ~F_V;
			if ((m_tmp32aux ^ 0x0001) & (m_tmp32aux ^ m_tmp32) & 0x8000)
			{
				m_ps |= F_V;
			}
			m_pc += 3;
			m_icount -= 5;
			break;

		// DECW addr16
		case 0x7f:
			m_tmp32 = read_16((m_pcb<<16) | (m_pc+2));
			if (m_prefix_valid)
			{
				m_prefix_valid = false;
				m_tmp32 |= (m_prefix << 16);
			}
			else
			{
				m_tmp32 |= (m_dtb << 16);
			}
			m_tmp16 = read_16(m_tmp32);
			m_tmp16aux = m_tmp16;
			m_tmp16--;
			write_16(m_tmp32, m_tmp16);
			setNZ_16(m_tmp16);
			m_ps &= ~F_V;
			if ((m_tmp16aux ^ 0x0001) & (m_tmp16aux ^ m_tmp16) & 0x8000)
			{
				m_ps |= F_V;
			}
			m_pc += 4;
			m_icount -= 5;
			break;

		// MOVW A, @RWx
		case 0x88: case 0x89: case 0x8a: case 0x8b:
			m_tmpea = read_rwX(operand & 3);
			m_acc <<= 16;
			m_acc |= read_16(getRWbank(operand & 3, m_tmpea));
			setNZ_16(m_acc & 0xffff);
			m_pc += 2;
			m_icount -= 2;
			break;

		// MOVW @RWx + disp8, #imm16
		case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
			m_tmp8 = read_16((m_pcb<<16) | (m_pc+2));
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+3));
			m_tmpea = read_rwX(operand & 7) + (s8)m_tmp8;
			write_16(getRWbank(operand & 7, m_tmpea), m_tmp16);
			m_pc += 5;
			m_icount -= 4;
			break;

		// MOVW @RWx + disp16, #imm16
		case 0xd8: case 0xd9: case 0xda: case 0xdb:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+2));
			m_tmp32 = read_16((m_pcb<<16) | (m_pc+4));
			m_tmpea = read_rwX(operand & 7) + (s16)m_tmp16;
			write_16(getRWbank(operand & 7, m_tmpea), m_tmp32 & 0xffff);
			m_pc += 6;
			m_icount -= 4;
			break;

		// MOVW addr16, #imm16
		case 0xdf:
			m_tmp32 = read_16((m_pcb<<16) | (m_pc+2));
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+4));
			if (m_prefix_valid)
			{
				m_prefix_valid = false;
				write_16((m_prefix<<16) | m_tmp32, m_tmp16);
			}
			else
			{
				write_16((m_dtb<<16) | m_tmp32, m_tmp16);
			}
			m_pc += 6;
			m_icount -= 4;
			break;

		default:
			fatalerror("Unknown F2MC EA73 opcode %02x (PC=%x)\n", operand, (m_pcb<<16) | m_pc);
			break;
	}
}

void f2mc16_device::opcodes_ea74(u8 operand)
{
	switch (operand)
	{
		// CMP A, @RWx + disp8
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp16 = read_rwX(operand & 7) + (s8)m_tmp8;
			m_tmp8 = read_8(getRWbank(operand & 7, m_tmp16));
			doCMP_8(m_acc & 0xff, m_tmp8);
			m_pc += 3;
			m_icount -= 10;
			break;

		// DBNZ Rx, disp8
		case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
			m_tmp16 = read_rX(operand & 7);
			m_tmp16aux = m_tmp16;
			m_tmp16--;
			write_rX(operand & 7, m_tmp16 & 0xff);
			setNZ_8(m_tmp16 & 0xff);
			m_ps &= ~F_V;
			if ((m_tmp16aux ^ 0x01) & (m_tmp16aux ^ m_tmp16) & 0x80)
			{
				m_ps |= F_V;
			}
			if (m_tmp16 & 0xff)
			{
				m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
				m_pc = (m_pc + 3) + (s8)m_tmp8;
				m_icount -= 7;
			}
			else
			{
				m_pc += 3;
				m_icount -= 6;
			}
			break;

		default:
			fatalerror("Unknown F2MC EA74 opcode %02x (PC=%x)\n", operand, (m_pcb<<16) | m_pc);
			break;
	}
}

void f2mc16_device::opcodes_ea75(u8 operand)
{
	switch (operand)
	{
		default:
			fatalerror("Unknown F2MC EA75 opcode %02x (PC=%x)\n", operand, (m_pcb<<16) | m_pc);
			break;
	}
}

void f2mc16_device::opcodes_ea76(u8 operand)
{
	switch (operand)
	{
		// ADDW A, RWx
		case 0x0: case 0x1: case 0x2: case 0x3: case 0x4: case 0x5: case 0x6: case 0x7:
			m_tmp16 = doADD_16(m_acc & 0xffff, read_rwX(operand & 7));
			m_acc &= 0xffff0000;
			m_acc |= m_tmp16;
			m_pc += 2;
			m_icount -= 3;
			break;

		// ADDW A, @RWx + disp8
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp16 = read_rwX(operand & 7) + (s8)m_tmp8;
			m_tmp16 = read_16(getRWbank(operand & 7, m_tmp16));
			m_tmp16aux = doADD_16(m_acc & 0xffff, m_tmp16);
			m_acc &= 0xffff0000;
			m_acc |= m_tmp16aux;
			m_pc += 3;
			m_icount -= 3;
			break;

		// SUBW A, RWx
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
			m_tmp16 = doSUB_16(m_acc & 0xffff, read_rwX(operand & 7));
			m_acc &= 0xffff0000;
			m_acc |= m_tmp16;
			m_pc += 2;
			m_icount -= 3;
			break;

		// SUBW A, @RWx + disp8
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp16 = read_rwX(operand & 7) + (s8)m_tmp8;
			m_tmp16 = read_16(getRWbank(operand & 7, m_tmp16));
			m_tmp16aux = doSUB_16(m_acc & 0xffff, m_tmp16);
			m_acc &= 0xffff0000;
			m_acc |= m_tmp16aux;
			m_pc += 3;
			m_icount -= 3;
			break;

		// SUBW A, addr16
		case 0x3f:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+2));
			if (m_prefix_valid)
			{
				m_prefix_valid = false;
				m_tmp16aux = read_16((m_prefix<<16) | m_tmp16);
			}
			else
			{
				m_tmp16aux = read_16((m_dtb<<16) | m_tmp16);
			}
			m_tmp16aux = doSUB_16(m_acc & 0xffff, m_tmp16aux);
			m_acc &= 0xffff0000;
			m_acc |= m_tmp16aux;
			m_pc += 4;
			m_icount -= 3;
			break;

		// CMPW A, RWx
		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
			doCMP_16(m_acc & 0xffff, read_rwX(operand & 7));
			m_pc += 2;
			m_icount -= 2;
			break;

		// CMPW A, @RWx + disp8
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp16 = read_rwX(operand & 7) + (s8)m_tmp8;
			m_tmp16 = read_16(getRWbank(operand & 7, m_tmp16));
			doCMP_16(m_acc & 0xffff, m_tmp16);
			m_pc += 3;
			m_icount -= 10;
			break;

		// CMPW A, addr16
		case 0x7f:
			m_tmp16 = read_16((m_pcb<<16) | (m_pc+2));
			if (m_prefix_valid)
			{
				m_prefix_valid = false;
				m_tmp16aux = read_16((m_prefix<<16) | m_tmp16);
			}
			else
			{
				m_tmp16aux = read_16((m_dtb<<16) | m_tmp16);
			}
			doCMP_16(m_acc & 0xffff, m_tmp16aux);
			m_pc += 4;
			m_icount -= 10;
			break;

		// ORW A, RWx
		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
			m_tmp16 = read_rwX(operand & 7);
			m_acc |= m_tmp16;
			setNZ_16(m_acc & 0xffff);
			m_ps &= ~F_V;
			m_pc += 2;
			m_icount -= 4;
			break;

		// ORW A, @RWx + disp8
		case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp16 = read_rwX(operand & 7) + (s8)m_tmp8;
			m_tmp16 = read_16(getRWbank(operand & 7, m_tmp16));
			m_acc |= m_tmp16;
			setNZ_16(m_acc & 0xffff);
			m_ps &= ~F_V;
			m_pc += 3;
			m_icount -= 3;
			break;

		default:
			fatalerror("Unknown F2MC EA76 opcode %02x (PC=%x)\n", operand, (m_pcb<<16) | m_pc);
			break;
	}
}

void f2mc16_device::opcodes_ea77(u8 operand)
{
	switch (operand)
	{
		default:
			fatalerror("Unknown F2MC EA77 opcode %02x (PC=%x)\n", operand, (m_pcb<<16) | m_pc);
			break;
	}
}

void f2mc16_device::opcodes_ea78(u8 operand)
{
	switch (operand)
	{
		// MULUW A, RWx
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
			m_tmp16 = read_rwX(operand & 7);
			if (m_tmp16 == 0)
			{
				m_icount -= 4;
				m_acc = 0;
			}
			else
			{
				m_icount -= 12;
				m_acc = m_tmp16 * (m_acc & 0xffff);
			}
			m_pc += 2;
			break;

		// MULUW A, @RWx + disp8
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
			m_tmp8 = read_8((m_pcb<<16) | (m_pc+2));
			m_tmp16 = read_rwX(operand & 7) + (s8)m_tmp8;
			m_tmp16 = read_16(getRWbank(operand & 7, m_tmp16));
			if (m_tmp16 == 0)
			{
				m_icount -= 4;
				m_acc = 0;
			}
			else
			{
				m_icount -= 12;
				m_acc = m_tmp16 * (m_acc & 0xffff);
			}
			m_pc += 3;
			break;

		// DIVUW A, RWx
		case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
			m_ps &= ~(F_V|F_C);
			m_tmp16 = read_rwX(operand & 7);
			if (m_tmp16)
			{
				m_tmp32aux = m_acc / m_tmp16;
				m_tmp16 = (m_acc % m_tmp16) & 0xffff;

				// overflow?
				if (m_tmp32aux >= 0x10000)
				{
					m_ps |= F_V;
					m_icount -= 7;
					m_ps |= F_V;
				}
				else    // normal operation
				{
					m_acc &= ~0xffff;
					m_acc |= (m_tmp32aux & 0xffff);
					write_rwX(operand & 7, m_tmp16);
					m_icount -= 22;
				}
			}
			else    // divide by zero
			{
				m_icount -= 4;
				m_ps |= F_V;
			}
			m_pc += 2;
			break;

		default:
			fatalerror("Unknown F2MC EA78 opcode %02x (PC=%x)\n", operand, (m_pcb<<16) | m_pc);
			break;
	}
}

void f2mc16_device::opcodes_rwi7a(u8 operand)
{
	if (operand & 0x10)
	{   // MOV Rx, @RWy + disp8
		m_tmpea = read_rwX(operand & 7) + (s8)read_8((m_pcb<<16) | (m_pc+2));
		m_tmp32 = getRWbank(operand & 0x7, m_tmpea);
		m_tmp8 = read_8(m_tmp32);
		write_rX((operand>>5) & 0x7, m_tmp8);
		setNZ_8(m_tmp8);
		m_pc += 3;
		m_icount -= 5;
	}
	else
	{ // MOV Rx, Ry
		m_tmp8 = read_rX(operand & 0x7);
		write_rX((operand>>5) & 0x7, m_tmp8);
		setNZ_8(m_tmp8);
		m_pc += 2;
		m_icount -= 4;
	}
}

void f2mc16_device::opcodes_rwi7b(u8 operand)
{
	if (operand & 0x10)
	{   // MOVW RWx, @RWy + disp8
		m_tmpea = read_rwX(operand & 7) + (s8)read_8((m_pcb<<16) | (m_pc+2));
		m_tmp32 = getRWbank(operand & 0x7, m_tmpea);
		m_tmp16 = read_16(m_tmp32);
		write_rwX((operand>>5) & 0x7, m_tmp16);
		setNZ_16(m_tmp16);
		m_pc += 3;
		m_icount -= 5;
	}
	else
	{ // MOVW RWx, RWy
		m_tmp16 = read_rwX(operand & 0x7);
		write_rwX((operand>>5) & 0x7, m_tmp16);
		setNZ_16(m_tmp16);
		m_pc += 2;
		m_icount -= 4;
	}
}

void f2mc16_device::execute_set_input(int inputnum, int state)
{
}

void f2mc16_device::set_irq(int vector, int level)
{
	if (m_vector_level[vector] != level)
	{
		m_outstanding_irqs++;
		m_vector_level[vector] = level;
//      printf("set_irq: vec %d, level %d, %d outstanding\n", vector, level, m_outstanding_irqs);
	}
}

void f2mc16_device::clear_irq(int vector)
{
	if (m_vector_level[vector] < 7)
	{
		m_outstanding_irqs--;
		m_vector_level[vector] = 7;
		//printf("clear_irq: vec %d, %d outstanding\n", vector, m_outstanding_irqs);
	}
}

// note: this function must not use m_tmp16 unless you change RETI
void f2mc16_device::take_irq(int vector, int level)
{
//  printf("take_irq: vector %d, level %d, old PC = %02x%04x\n", vector, level, m_pcb, m_pc);
	push_16_ssp(m_acc>>16);
	push_16_ssp(m_acc & 0xffff);
	push_16_ssp((m_dpr<<8) | m_adb);
	push_16_ssp((m_dtb<<8) | m_pcb);
	push_16_ssp(m_pc);
	push_16_ssp(m_ps);

	m_ps |= F_S;
	m_ps &= ~0xe000;
	m_ps |= (level & 7) << 13;

	u32 uVecAddr = 0xfffffc - (vector * 4);
	m_pc = read_16(uVecAddr);
	m_pcb = read_8(uVecAddr + 2);
	//printf("New PC = %02x%04x, new level=%d\n", m_pcb, m_pc, m_ps>>13);
}

// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * 8x300.c
 *
 *  Implementation of the Scientific Micro Systems SMS300 / Signetics 8X300 CPU
 *  Created on: 18/12/2013
 *
 *  Written by Barry Rodewald
 */

#include "emu.h"
#include "8x300.h"
#include "8x300dasm.h"
#include "debugger.h"

#define FETCHOP(a)         (m_cache->read_word(a))
#define CYCLES(x)          do { m_icount -= (x); } while (0)
#define READPORT(a)        (m_io->read_byte(a))
#define WRITEPORT(a,v)     (m_io->write_byte((a), (v)))

#define SRC    ((opcode & 0x1f00) >> 8)
#define DST    (opcode & 0x001f)
#define ROTLEN ((opcode & 0x00e0) >> 5)
#define IMM8   (opcode & 0x00ff)
#define IMM5   (opcode & 0x001f)
#define ADDR   (opcode & 0x1fff)
#define OP     ((opcode & 0xe000) >> 13)
#define SRC_IS_RIGHT_BANK  (opcode & 0x0800)
#define DST_IS_RIGHT_BANK  (opcode & 0x0008)
#define SRC_LSB ((opcode & 0x0700) >> 8)
#define DST_LSB (opcode & 0x0007)
#define SET_PC(x)  do { m_PC = (x); m_AR = m_PC; } while (0)
// for XEC intruction, which sets the AR, but not PC, so that after the instruction at the relative address is done, execution
// returns back to next instruction after XEC, unless a JMP or successful NZT is there.
#define SET_AR(x)  do { m_AR = (x); m_increment_pc = false; } while (0)
#define SRC_LATCH  do { if(SRC_IS_RIGHT_BANK) m_right_IV = READPORT(m_IVR+0x100); else m_left_IV = READPORT(m_IVL); } while (0)
#define DST_LATCH  do { if(DST_IS_RIGHT_BANK) m_right_IV = READPORT(m_IVR+0x100); else m_left_IV = READPORT(m_IVL); } while (0)
#define SET_OVF    do { if(result & 0xff00) m_OVF = 1; else m_OVF = 0; } while (0)

DEFINE_DEVICE_TYPE(N8X300, n8x300_cpu_device, "8x300", "Signetics 8X300")


n8x300_cpu_device::n8x300_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, N8X300, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 16, 14, 0)
	, m_io_config("io", ENDIANNESS_BIG, 8, 9, 0)
{
}

device_memory_interface::space_config_vector n8x300_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}

void n8x300_cpu_device::set_reg(uint8_t reg, uint8_t val)
{
	switch(reg)
	{
	case 0x00: m_AUX = val; break;
	case 0x01: m_R1 = val; break;
	case 0x02: m_R2 = val; break;
	case 0x03: m_R3 = val; break;
	case 0x04: m_R4 = val; break;
	case 0x05: m_R5 = val; break;
	case 0x06: m_R6 = val; break;
	case 0x07: m_IVL = val; break;
//  OVF is read-only
	case 0x09: m_R11 = val; break;
	case 0x0f: m_IVR = val; break;
	default: logerror("8X300: Invalid register %02x written to.\n",reg); break;
	}
}

uint8_t n8x300_cpu_device::get_reg(uint8_t reg)
{
	switch(reg)
	{
	case 0x00: return m_AUX;
	case 0x01: return m_R1;
	case 0x02: return m_R2;
	case 0x03: return m_R3;
	case 0x04: return m_R4;
	case 0x05: return m_R5;
	case 0x06: return m_R6;
//  IVL is write-only
	case 0x08: return m_OVF;
	case 0x09: return m_R11;
//  IVR is write-only
	default: logerror("8X300: Invalid register %02x read.\n",reg); return 0;
	}
}

void n8x300_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_cache = m_program->cache<1, 0, ENDIANNESS_BIG>();
	m_io = &space(AS_IO);

	save_item(NAME(m_PC));
	save_item(NAME(m_AR));
	save_item(NAME(m_IR));
	save_item(NAME(m_R1));
	save_item(NAME(m_R2));
	save_item(NAME(m_R3));
	save_item(NAME(m_R4));
	save_item(NAME(m_R5));
	save_item(NAME(m_R6));
	save_item(NAME(m_R11));
	save_item(NAME(m_AUX));
	save_item(NAME(m_IVL));
	save_item(NAME(m_IVR));
	save_item(NAME(m_OVF));
	save_item(NAME(m_left_IV));
	save_item(NAME(m_right_IV));
	save_item(NAME(m_genPC));
	save_item(NAME(m_increment_pc));

	// reset registers here, since they are unchanged when /RESET goes low.
	m_R1 = 0;
	m_R2 = 0;
	m_R3 = 0;
	m_R4 = 0;
	m_R5 = 0;
	m_R6 = 0;
	m_R11 = 0;
	m_IVL = 0;
	m_IVR = 0;
	m_AUX = 0;

	m_IR = 0;
	m_OVF = 0;

	// Register state for debugger
	state_add( _8X300_PC, "PC", m_PC).mask(0x1fff).callimport().formatstr("%04X");
	state_add( _8X300_AR,  "AR",  m_AR).mask(0x1fff).callimport().formatstr("%04X");
	state_add( _8X300_IR,  "IR",  m_IR).mask(0xffff).formatstr("%04X");
	state_add( _8X300_AUX,  "AUX",  m_AUX).mask(0xff).formatstr("%02X");
	state_add( _8X300_R1,  "R1",  m_R1).mask(0xff).formatstr("%02X");
	state_add( _8X300_R2,  "R2",  m_R2).mask(0xff).formatstr("%02X");
	state_add( _8X300_R3,  "R3",  m_R3).mask(0xff).formatstr("%02X");
	state_add( _8X300_R4,  "R4",  m_R4).mask(0xff).formatstr("%02X");
	state_add( _8X300_R5,  "R5",  m_R5).mask(0xff).formatstr("%02X");
	state_add( _8X300_R6,  "R6",  m_R6).mask(0xff).formatstr("%02X");
	state_add( _8X300_R11,  "R11",  m_R11).mask(0xff).formatstr("%02X");
	state_add( _8X300_OVF,  "OVF",  m_OVF).mask(0x01).formatstr("%01X");
	state_add( _8X300_IVL,  "IVL",  m_IVL).mask(0xff).formatstr("%02X");
	state_add( _8X300_IVR,  "IVR",  m_IVR).mask(0xff).formatstr("%02X");
	state_add(STATE_GENPC, "GENPC", m_genPC).mask(0x3ffe).callimport().noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_genPC).mask(0x3ffe).callimport().noshow();

	set_icountptr(m_icount);
}

//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void n8x300_cpu_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
	case _8X300_PC:
		m_AR = m_PC;
		m_genPC = m_AR << 1;
		m_increment_pc = true;
		break;

	case _8X300_AR:
		m_genPC = m_AR << 1;
		m_increment_pc = false;
		break;

	case STATE_GENPC:
	case STATE_GENPCBASE:
		m_AR = m_genPC >> 1;
		m_PC = m_AR;
		m_increment_pc = true;
		break;
	}
}

void n8x300_cpu_device::device_reset()
{
	/* zero registers */
	m_PC = 0;
	m_AR = 0;
	m_genPC = 0;
	m_increment_pc = true;
}

void n8x300_cpu_device::execute_run()
{
	do
	{
		uint16_t opcode;
		uint8_t src;
		uint8_t dst;
		uint8_t rotlen;  // rotate amount or I/O field length
		uint8_t mask;
		uint16_t result;

		/* fetch the opcode */
		m_genPC = m_AR << 1;
		debugger_instruction_hook(m_genPC);
		opcode = FETCHOP(m_genPC);

		if (m_increment_pc)
		{
			m_PC++;
			m_PC &= 0x1fff;
		}
		else
		{
			m_increment_pc = true;
		}

		m_AR = m_PC;
		m_IR = opcode;

		switch (OP)
		{
		case 0x00:  // MOVE
			rotlen = ROTLEN;
			if(is_rot(opcode))  // MOVE reg,reg
			{
				src = get_reg(SRC);
				dst = rotate(src,rotlen);
				set_reg(DST,dst);
			}
			else
			{
				if(rotlen == 0)
					rotlen = 8;  // 0 = 8-bit I/O field length
				if(is_src_reg(opcode) && !(is_dst_reg(opcode)))
				{  // MOVE reg,IV
					DST_LATCH;
					mask = ((1 << rotlen)-1);
					src = (get_reg(SRC)) << (7-DST_LSB);
					mask <<= (7-DST_LSB);
					if(DST_IS_RIGHT_BANK)
					{
						dst = (m_right_IV & ~mask) | (src & mask);
						m_right_IV = dst;
						WRITEPORT(m_IVR+0x100,m_right_IV);
					}
					else
					{
						dst = (m_left_IV & ~mask) | (src & mask);
						m_left_IV = dst;
						WRITEPORT(m_IVL,m_left_IV);
					}
				}
				else if(!(is_src_reg(opcode)) && is_dst_reg(opcode))
				{  // MOVE IV,reg
					SRC_LATCH;
					if(SRC_IS_RIGHT_BANK)
						src = rotate(m_right_IV,7-SRC_LSB);
					else
						src = rotate(m_left_IV,7-SRC_LSB);
					mask = ((1 << rotlen)-1);
					dst = src & mask;
					set_reg(DST,dst);
				}
				else if(!(is_src_reg(opcode)) && !(is_dst_reg(opcode)))
				{  // MOVE IV,IV
					SRC_LATCH;
					if(SRC_IS_RIGHT_BANK)
						src = rotate(m_right_IV,7-SRC_LSB);
					else
						src = rotate(m_left_IV,7-SRC_LSB);
					mask = ((1 << rotlen)-1);
					dst = src & mask;
					dst <<= (7-DST_LSB);
					mask <<= (7-DST_LSB);
					if(SRC_IS_RIGHT_BANK)  // untouched source IV bits are preserved and sent to destination IV
					{
						dst = (m_right_IV & ~mask) | (dst & mask);
						m_right_IV = dst;
						WRITEPORT(m_IVR+0x100,m_right_IV);
					}
					else
					{
						dst = (m_left_IV & ~mask) | (dst & mask);
						m_left_IV = dst;
						WRITEPORT(m_IVL,m_left_IV);
					}
				}
			}
			break;
		case 0x01:  // ADD
			rotlen = ROTLEN;
			if(is_rot(opcode))
			{  // ADD reg,reg
				src = rotate(get_reg(SRC),rotlen);
				result = src + m_AUX;
				set_reg(DST,result & 0xff);
				SET_OVF;
			}
			else
			{
				if(rotlen == 0)
					rotlen = 8;  // 0 = 8-bit I/O field length
				if(is_src_reg(opcode) && !(is_dst_reg(opcode)))
				{  // ADD reg,IV
					DST_LATCH;
					result = get_reg(SRC) + m_AUX;
					mask = ((1 << rotlen)-1);
					dst = (result & 0xff) << DST_LSB;
					mask <<= DST_LSB;
					SET_OVF;
					if(DST_IS_RIGHT_BANK)
					{
						dst = (m_right_IV & ~mask) | (dst & mask);
						m_right_IV = dst;
						WRITEPORT(m_IVR+0x100,m_right_IV);
					}
					else
					{
						dst = (m_left_IV & ~mask) | (dst & mask);
						m_left_IV = dst;
						WRITEPORT(m_IVL,m_left_IV);
					}
				}
				else if(!(is_src_reg(opcode)) && is_dst_reg(opcode))
				{  // ADD IV,reg
					SRC_LATCH;
					mask = ((1 << rotlen)-1);
					if(SRC_IS_RIGHT_BANK)
						src = rotate(m_right_IV,7-SRC_LSB) & mask;
					else
						src = rotate(m_left_IV,7-SRC_LSB) & mask;
					result = src + m_AUX;
					SET_OVF;
					set_reg(DST,result & 0xff);
				}
				else if(!(is_src_reg(opcode)) && !(is_dst_reg(opcode)))
				{  // ADD IV,IV
					SRC_LATCH;
					DST_LATCH;
					mask = ((1 << rotlen)-1);
					if(SRC_IS_RIGHT_BANK)
						src = rotate(m_right_IV,7-SRC_LSB) & mask;
					else
						src = rotate(m_left_IV,7-SRC_LSB) & mask;
					result = src + m_AUX;
					SET_OVF;
					dst = (result << (7-DST_LSB)) & 0xff;
					mask <<= (7-DST_LSB);
					if(SRC_IS_RIGHT_BANK)  // unused destination IV data is not preserved, is merged with input IV data
					{
						dst = (m_right_IV & ~mask) | (dst & mask);
						m_right_IV = dst;
						WRITEPORT(m_IVR+0x100,m_right_IV);
					}
					else
					{
						dst = (m_left_IV & ~mask) | (dst & mask);
						m_left_IV = dst;
						WRITEPORT(m_IVL,m_left_IV);
					}
				}
			}
			break;
		case 0x02:  // AND
			rotlen = ROTLEN;
			if(is_rot(opcode))
			{  // AND reg,reg
				src = rotate(get_reg(SRC),rotlen);
				dst = src & m_AUX;
				set_reg(DST,dst);
			}
			else
			{
				if(rotlen == 0)
					rotlen = 8;  // 0 = 8-bit I/O field length
				if(is_src_reg(opcode) && !(is_dst_reg(opcode)))
				{  // AND reg,IV
					DST_LATCH;
					src = get_reg(SRC) & m_AUX;
					mask = ((1 << rotlen)-1);
					src <<= (7-DST_LSB);
					mask <<= (7-DST_LSB);
					if(DST_IS_RIGHT_BANK)
					{
						dst = (m_right_IV & ~mask) | (src & mask);
						m_right_IV = dst;
						WRITEPORT(m_IVR+0x100,m_right_IV);
					}
					else
					{
						dst = (m_left_IV & ~mask) | (src & mask);
						m_left_IV = dst;
						WRITEPORT(m_IVL,m_left_IV);
					}
				}
				else if(!(is_src_reg(opcode)) && is_dst_reg(opcode))
				{  // AND IV,reg
					SRC_LATCH;
					mask = ((1 << rotlen)-1);
					if(SRC_IS_RIGHT_BANK)
						src = rotate(m_right_IV,7-SRC_LSB) & mask;
					else
						src = rotate(m_left_IV,7-SRC_LSB) & mask;
					src &= mask;
					dst = src & m_AUX;
					set_reg(DST,dst);
				}
				else if(!(is_src_reg(opcode)) && !(is_dst_reg(opcode)))
				{  // AND IV,IV
					SRC_LATCH;
					DST_LATCH;
					mask = ((1 << rotlen)-1);
					if(SRC_IS_RIGHT_BANK)
						src = rotate(m_right_IV,7-SRC_LSB) & mask;
					else
						src = rotate(m_left_IV,7-SRC_LSB) & mask;
					src &= mask;
					dst = src & m_AUX;
					dst <<= (7-DST_LSB);
					mask <<= (7-DST_LSB);
					if(SRC_IS_RIGHT_BANK)
					{
						dst = (m_right_IV & ~mask) | (src & mask);
						m_right_IV = dst;
						WRITEPORT(m_IVR+0x100,m_right_IV);
					}
					else
					{
						dst = (m_left_IV & ~mask) | (src & mask);
						m_left_IV = dst;
						WRITEPORT(m_IVL,m_left_IV);
					}
				}
			}
			break;
		case 0x03:  // XOR
			rotlen = ROTLEN;
			if(is_rot(opcode))
			{  // AND reg,reg
				src = rotate(get_reg(SRC),rotlen);
				dst = src ^ m_AUX;
				set_reg(DST,dst);
			}
			else
			{
				if(rotlen == 0)
					rotlen = 8;  // 0 = 8-bit I/O field length
				if(is_src_reg(opcode) && !(is_dst_reg(opcode)))
				{  // AND reg,IV
					DST_LATCH;
					src = get_reg(SRC) ^ m_AUX;
					mask = ((1 << rotlen)-1);
					src <<= (7-DST_LSB);
					mask <<= (7-DST_LSB);
					if(DST_IS_RIGHT_BANK)
					{
						dst = (m_right_IV & ~mask) | (src & mask);
						m_right_IV = dst;
						WRITEPORT(m_IVR+0x100,m_right_IV);
					}
					else
					{
						dst = (m_left_IV & ~mask) | (src & mask);
						m_left_IV = dst;
						WRITEPORT(m_IVL,m_left_IV);
					}
				}
				else if(!(is_src_reg(opcode)) && is_dst_reg(opcode))
				{  // AND IV,reg
					SRC_LATCH;
					mask = ((1 << rotlen)-1);
					if(SRC_IS_RIGHT_BANK)
						src = rotate(m_right_IV,7-SRC_LSB) & mask;
					else
						src = rotate(m_left_IV,7-SRC_LSB) & mask;
					src &= mask;
					dst = src ^ m_AUX;
					set_reg(DST,dst);
				}
				else if(!(is_src_reg(opcode)) && !(is_dst_reg(opcode)))
				{  // AND IV,IV
					SRC_LATCH;
					DST_LATCH;
					mask = ((1 << rotlen)-1);
					if(SRC_IS_RIGHT_BANK)
						src = rotate(m_right_IV,7-SRC_LSB) & mask;
					else
						src = rotate(m_left_IV,7-SRC_LSB) & mask;
					src &= mask;
					dst = src ^ m_AUX;
					dst <<= (7-DST_LSB);
					mask <<= (7-DST_LSB);
					if(SRC_IS_RIGHT_BANK)
					{
						dst = (m_right_IV & ~mask) | (src & mask);
						m_right_IV = dst;
						WRITEPORT(m_IVR+0x100,m_right_IV);
					}
					else
					{
						dst = (m_left_IV & ~mask) | (src & mask);
						m_left_IV = dst;
						WRITEPORT(m_IVL,m_left_IV);
					}
				}
			}
			break;
		case 0x04:  // XEC  (Execute)
			if(is_src_reg(opcode))
			{
				src = get_reg(SRC);
				src += IMM8;
				SET_AR((m_AR & 0x1f00) | src);
			}
			else
			{
				SRC_LATCH;
				rotlen = ROTLEN;
				if(rotlen == 0)
					rotlen = 8;  // 0 = 8-bit I/O field length
				mask = ((1 << rotlen)-1);
				if(SRC_IS_RIGHT_BANK)
					src = rotate(m_right_IV,7-SRC_LSB);
				else
					src = rotate(m_left_IV,7-SRC_LSB);
				src &= mask;
				src += IMM5;
				SET_AR((m_AR & 0x1fe0) | (src & 0x1f));
			}
			break;
		case 0x05:  // NZT  (Non-zero transfer)
			if(is_src_reg(opcode))
			{
				src = get_reg(SRC);
				if(src != 0)
					SET_PC((m_PC & 0x1f00) | IMM8);
			}
			else
			{
				SRC_LATCH;
				rotlen = ROTLEN;
				if(rotlen == 0)
					rotlen = 8;  // 0 = 8-bit I/O field length
				mask = ((1 << rotlen)-1);
				if(SRC_IS_RIGHT_BANK)
					src = rotate(m_right_IV,7-SRC_LSB);
				else
					src = rotate(m_left_IV,7-SRC_LSB);
				rotate(src,SRC_LSB);
				src &= mask;
				if(src != 0)
					SET_PC((m_PC & 0x1fe0) | IMM5);
			}
			break;
		case 0x06:  // XMIT (Transmit)
			// the source is actually the destination for this instruction
			if(is_src_reg(opcode))
				set_reg(SRC,IMM8);
			else
			{
				SRC_LATCH;
				rotlen = ROTLEN;
				if(rotlen == 0)
					rotlen = 8;  // 0 = 8-bit I/O field length
				mask = ((1 << rotlen)-1);
				dst = IMM5;
				mask <<= (7-SRC_LSB);
				dst <<= (7-SRC_LSB);
				if(SRC_IS_RIGHT_BANK)
				{
					m_right_IV = (m_right_IV & ~mask) | (dst & mask);
					WRITEPORT(m_IVR+0x100,m_right_IV);
				}
				else
				{
					m_left_IV = (m_left_IV & ~mask) | (dst & mask);
					WRITEPORT(m_IVL,m_left_IV);
				}
			}
			break;
		case 0x07:  // JMP
			SET_PC(ADDR);
			break;
		}
		CYCLES(1);  // all instructions take 1 cycle (250ns)
	} while (m_icount > 0);
}

std::unique_ptr<util::disasm_interface> n8x300_cpu_device::create_disassembler()
{
	return std::make_unique<n8x300_disassembler>();
}

// license:BSD-3-Clause
// copyright-holders:ElSemi
#include "emu.h"
#include "se3208.h"
#include "se3208dis.h"

#define LOG_ALIGN (1 << 1)

#define VERBOSE (0)

#include "logmacro.h"


/*
    SE3208 CPU Emulator by ElSemi

    For information about this CPU:
    www.adc.co.kr

*/


enum : u32
{
	FLAG_C      = 0x0080,
	FLAG_V      = 0x0010,
	FLAG_S      = 0x0020,
	FLAG_Z      = 0x0040,

	FLAG_M      = 0x0200,
	FLAG_E      = 0x0800,
	FLAG_AUT    = 0x1000,
	FLAG_ENI    = 0x2000,
	FLAG_NMI    = 0x4000
};

//Precompute the instruction decoding in a big table
#define INST(a) void se3208_device::a(u16 opcode)

DEFINE_DEVICE_TYPE(SE3208, se3208_device, "se3208", "ADChips SE3208")


se3208_device::se3208_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, SE3208, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_machinex_cb(*this)
	, m_iackx_cb(*this, 0)
	, m_PC(0), m_SR(0), m_SP(0), m_ER(0), m_PPC(0), m_IRQ(0), m_NMI(0), m_icount(0)
{
}

device_memory_interface::space_config_vector se3208_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}


u8 se3208_device::read8(u32 address)
{
	return m_program.read_byte(address);
}

u16 se3208_device::read16(u32 address)
{
	if (!WORD_ALIGNED(address))
		return m_program.read_byte(address) | m_program.read_byte(address + 1) << 8;
	else
		return m_program.read_word(address);
}

u32 se3208_device::read32(u32 address)
{
	if (DWORD_ALIGNED(address))
		return m_program.read_dword(address);
	else
	{
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_ALIGN, "%s: dword READ unaligned %08x\n", machine().describe_context(), address);
		return m_program.read_byte(address) | m_program.read_byte(address + 1) << 8 | m_program.read_byte(address + 2) << 16 | m_program.read_byte(address + 3) << 24;
	}
}

void se3208_device::write8(u32 address, u8 data)
{
	m_program.write_byte(address,data);
}

void se3208_device::write16(u32 address, u16 data)
{
	if (!WORD_ALIGNED(address))
	{
		m_program.write_byte(address, data & 0xff);
		m_program.write_byte(address + 1, (data >> 8) & 0xff);
	}
	else
	{
		m_program.write_word(address, data);
	}
}

void se3208_device::write32(u32 address, u32 data)
{
	if (DWORD_ALIGNED(address))
		m_program.write_dword(address, data);
	else
	{
		m_program.write_byte(address, data & 0xff);
		m_program.write_byte(address + 1, (data >> 8) & 0xff);
		m_program.write_byte(address + 2, (data >> 16) & 0xff);
		m_program.write_byte(address + 3, (data >> 24) & 0xff);
		LOGMASKED(LOG_ALIGN, "%s: dword WRITE unaligned %08x\n", machine().describe_context(), address);
	}
}



u32 se3208_device::add_with_lfags(u32 a, u32 b)
{
	const u32 r = a + b;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if (!r)
		SETFLAG(FLAG_Z);
	if (r & 0x80000000)
		SETFLAG(FLAG_S);
	if (((((a & b) | (~r & (a | b))) >> 31)) & 1)
		SETFLAG(FLAG_C);
	if (((((a ^ r) & (b ^ r)) >> 31)) & 1)
		SETFLAG(FLAG_V);
	return r;
}

u32 se3208_device::sub_with_lfags(u32 a, u32 b) //a-b
{
	const u32 r = a - b;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if (!r)
		SETFLAG(FLAG_Z);
	if (r & 0x80000000)
		SETFLAG(FLAG_S);
	if ((((b & r) | (~a & (b | r))) >> 31) & 1)
		SETFLAG(FLAG_C);
	if ((((b ^ a) & (r ^ a)) >> 31) & 1)
		SETFLAG(FLAG_V);
	return r;
}

u32 se3208_device::adc_with_lfags(u32 a, u32 b)
{
	const u32 carry = (m_SR & FLAG_C) ? 1 : 0;
	const u32 r = a + b + carry;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if (!r)
		SETFLAG(FLAG_Z);
	if (r & 0x80000000)
		SETFLAG(FLAG_S);
	if (((((a & b) | (~r & (a | b))) >> 31)) & 1)
		SETFLAG(FLAG_C);
	if (((((a ^ r) & (b ^ r)) >> 31)) & 1)
		SETFLAG(FLAG_V);
	return r;

}

u32 se3208_device::sbc_with_lfags(u32 a, u32 b)
{
	const u32 carry = (m_SR & FLAG_C) ? 1 : 0;
	const u32 r = a - b - carry;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if (!r)
		SETFLAG(FLAG_Z);
	if (r & 0x80000000)
		SETFLAG(FLAG_S);
	if ((((b & r) | (~a & (b | r))) >> 31) & 1)
		SETFLAG(FLAG_C);
	if ((((b ^ a) & (r ^ a)) >> 31) & 1)
		SETFLAG(FLAG_V);
	return r;
}

u32 se3208_device::mul_with_lfags(u32 a, u32 b)
{
	const int64_t r = int64_t(a) * int64_t(b);
	CLRFLAG(FLAG_V);
	if (r >> 32)
		SETFLAG(FLAG_V);
	return u32(r & 0xffffffff);
}

u32 se3208_device::neg_with_lfags(u32 a)
{
	return sub_with_lfags(0, a);
}

u32 se3208_device::asr_with_lfags(u32 val, u8 by)
{
	s32 v = s32(val);
	v >>= by;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if (!v)
		SETFLAG(FLAG_Z);
	if (v & 0x80000000)
		SETFLAG(FLAG_S);
	if (BIT(val, by - 1))
		SETFLAG(FLAG_C);
	return u32(v);
}

u32 se3208_device::lsr_with_lfags(u32 val, u8 by)
{
	u32 v = val;
	v >>= by;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if (!v)
		SETFLAG(FLAG_Z);
	if (v & 0x80000000)
		SETFLAG(FLAG_S);
	if (BIT(val, by - 1))
		SETFLAG(FLAG_C);
	return v;
}

u32 se3208_device::asl_with_lfags(u32 val, u8 by)
{
	u32 v = val;
	v <<= by;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if (!v)
		SETFLAG(FLAG_Z);
	if (v & 0x80000000)
		SETFLAG(FLAG_S);
	if (BIT(val, 32 - by))
		SETFLAG(FLAG_C);
	return v;
}

u32 se3208_device::get_index(u32 index)
{
	if (index)
		return m_R[index];
	else
		return 0;
}

u32 se3208_device::get_extended_operand(u32 imm, u8 shift)
{
	return (m_ER << shift) | (imm & ((1 << shift) - 1));
}

INST(INVALIDOP)
{
	//assert(false);
}

INST(LDB)
{
	u32 offset = BIT(opcode, 0, 5);
	u32 index = BIT(opcode, 5, 3);
	const u32 src_dst = BIT(opcode, 8, 3);

	index = get_index(index);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 4);

	const u32 val = read8(index + offset);
	m_R[src_dst] = s8(val);

	CLRFLAG(FLAG_E);
}

INST(STB)
{
	u32 offset = BIT(opcode, 0, 5);
	u32 index = BIT(opcode, 5, 3);
	const u32 src_dst = BIT(opcode, 8, 3);

	index = get_index(index);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 4);

	write8(index + offset, m_R[src_dst] & 0xff);

	CLRFLAG(FLAG_E);
}

INST(LDS)
{
	u32 offset = BIT(opcode, 0, 5);
	u32 index = BIT(opcode, 5, 3);
	const u32 src_dst = BIT(opcode, 8, 3);

	offset <<= 1;

	index = get_index(index);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 4);

	const u32 val = read16(index + offset);
	m_R[src_dst] = s16(val);

	CLRFLAG(FLAG_E);
}

INST(STS)
{
	u32 offset = BIT(opcode, 0, 5);
	u32 index = BIT(opcode, 5, 3);
	const u32 src_dst = BIT(opcode, 8, 3);

	offset <<= 1;

	index = get_index(index);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 4);

	write16(index + offset, m_R[src_dst] & 0xffff);

	CLRFLAG(FLAG_E);
}

INST(LD)
{
	u32 offset = BIT(opcode, 0, 5);
	u32 index = BIT(opcode, 5, 3);
	const u32 src_dst = BIT(opcode, 8, 3);

	offset <<= 2;

	index = get_index(index);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 4);

	m_R[src_dst] = read32(index + offset);

	CLRFLAG(FLAG_E);
}

INST(ST)
{
	u32 offset = BIT(opcode, 0, 5);
	u32 index = BIT(opcode, 5, 3);
	const u32 src_dst = BIT(opcode, 8, 3);

	offset <<= 2;

	index = get_index(index);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 4);

	write32(index + offset, m_R[src_dst]);

	CLRFLAG(FLAG_E);
}

INST(LDBU)
{
	u32 offset = BIT(opcode, 0, 5);
	u32 index = BIT(opcode, 5, 3);
	const u32 src_dst = BIT(opcode, 8, 3);

	index = get_index(index);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 4);

	const u32 val = read8(index + offset);
	m_R[src_dst] = val;

	CLRFLAG(FLAG_E);
}

INST(LDSU)
{
	u32 offset = BIT(opcode, 0, 5);
	u32 index = BIT(opcode, 5, 3);
	const u32 src_dst = BIT(opcode, 8, 3);

	offset <<= 1;

	index = get_index(index);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 4);

	const u32 val = read16(index + offset);
	m_R[src_dst] = val & 0xffff;

	CLRFLAG(FLAG_E);
}


INST(LERI)
{
	const u32 imm = BIT(opcode, 0, 14);
	if (TESTFLAG(FLAG_E))
		m_ER = (m_ER << 14) | imm;
	else
		m_ER = util::sext(imm, 14);

	SETFLAG(FLAG_E);
}

INST(LDSP)
{
	u32 offset = BIT(opcode, 0, 8);
	const u32 index = m_SP;
	const u32 src_dst = BIT(opcode, 8, 3);

	offset <<= 2;

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 4);

	m_R[src_dst] = read32(index + offset);

	CLRFLAG(FLAG_E);
}

INST(STSP)
{
	u32 offset = BIT(opcode, 0, 8);
	const u32 index = m_SP;
	const u32 src_dst = BIT(opcode, 8, 3);

	offset <<= 2;

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 4);

	write32(index + offset, m_R[src_dst]);

	CLRFLAG(FLAG_E);
}

void se3208_device::push_val(u32 val)
{
	m_SP -= 4;
	write32(m_SP, val);
}

u32 se3208_device::pop_val()
{
	const u32 val = read32(m_SP);
	m_SP += 4;
	return val;
}

INST(PUSH)
{
	const u32 set = BIT(opcode, 0, 11);
	if (BIT(set, 10))
		push_val(m_PC);
	if (BIT(set, 9))
		push_val(m_SR);
	if (BIT(set, 8))
		push_val(m_ER);
	if (BIT(set, 7))
		push_val(m_R[7]);
	if (BIT(set, 6))
		push_val(m_R[6]);
	if (BIT(set, 5))
		push_val(m_R[5]);
	if (BIT(set, 4))
		push_val(m_R[4]);
	if (BIT(set, 3))
		push_val(m_R[3]);
	if (BIT(set, 2))
		push_val(m_R[2]);
	if (BIT(set, 1))
		push_val(m_R[1]);
	if (BIT(set, 0))
		push_val(m_R[0]);
}

INST(POP)
{
	const u32 set = BIT(opcode, 0, 11);
	if (BIT(set, 0))
		m_R[0] = pop_val();
	if (BIT(set, 1))
		m_R[1] = pop_val();
	if (BIT(set, 2))
		m_R[2] = pop_val();
	if (BIT(set, 3))
		m_R[3] = pop_val();
	if (BIT(set, 4))
		m_R[4] = pop_val();
	if (BIT(set, 5))
		m_R[5] = pop_val();
	if (BIT(set, 6))
		m_R[6] = pop_val();
	if (BIT(set, 7))
		m_R[7] = pop_val();
	if (BIT(set, 8))
		m_ER = pop_val();
	if (BIT(set, 9))
		m_SR = pop_val();
	if (BIT(set, 10))
	{
		m_PC = pop_val() - 2;        //PC automatically incresases by 2
	}
}

INST(LEATOSP)
{
	u32 offset = BIT(opcode, 9, 4);
	u32 index = BIT(opcode, 3, 3);

	index = get_index(index);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 4);
	else
		offset = util::sext(offset, 4);

	m_SP = (index + offset) & ~3;

	CLRFLAG(FLAG_E);
}

INST(LEAFROMSP)
{
	u32 offset = BIT(opcode, 9, 4);
	const u32 index = BIT(opcode, 3, 3);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 4);
	else
		offset = util::sext(offset, 4);

	m_R[index] = m_SP + offset;

	CLRFLAG(FLAG_E);
}

INST(LEASPTOSP)
{
	u32 offset = BIT(opcode, 0, 8);

	offset <<= 2;

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 8);
	else
		offset = util::sext(offset, 10);

	m_SP = (m_SP + offset) & ~3;

	CLRFLAG(FLAG_E);
}

INST(MOV)
{
	const u32 src = BIT(opcode, 3, 3);
	const u32 dst = BIT(opcode, 9, 3);

	m_R[dst] = m_R[src];
}

INST(LDI)
{
	const u32 dst = BIT(opcode, 8, 3);
	u32 imm = BIT(opcode, 0, 8);

	if (TESTFLAG(FLAG_E))
		imm = get_extended_operand(imm, 4);
	else
		imm = s8(imm);

	m_R[dst] = imm;

	CLRFLAG(FLAG_E);
}

INST(LDBSP)
{
	u32 offset = BIT(opcode, 0, 4);
	const u32 index = m_SP;
	const u32 src_dst = BIT(opcode, 4, 3);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 4);

	const u32 val = read8(index + offset);
	m_R[src_dst] = s8(val);

	CLRFLAG(FLAG_E);
}

INST(STBSP)
{
	u32 offset = BIT(opcode, 0, 4);
	const u32 index = m_SP;
	const u32 src_dst = BIT(opcode, 4, 3);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 4);

	write8(index + offset, m_R[src_dst] & 0xff);

	CLRFLAG(FLAG_E);
}

INST(LDSSP)
{
	u32 offset = BIT(opcode, 0, 4);
	const u32 index = m_SP;
	const u32 src_dst = BIT(opcode, 4, 3);

	offset <<= 1;

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 4);

	const u32 val = read16(index + offset);
	m_R[src_dst] = s16(val);

	CLRFLAG(FLAG_E);
}

INST(STSSP)
{
	u32 offset = BIT(opcode, 0, 4);
	const u32 index = m_SP;
	const u32 src_dst = BIT(opcode, 4, 3);

	offset <<= 1;

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 4);

	write16(index + offset, m_R[src_dst] & 0xffff);

	CLRFLAG(FLAG_E);
}

INST(LDBUSP)
{
	u32 offset = BIT(opcode, 0, 4);
	const u32 index = m_SP;
	const u32 src_dst = BIT(opcode, 4, 3);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 4);

	const u32 val = read8(index + offset);
	m_R[src_dst] = val;

	CLRFLAG(FLAG_E);
}

INST(LDSUSP)
{
	u32 offset = BIT(opcode, 0, 4);
	const u32 index = m_SP;
	const u32 src_dst = BIT(opcode, 4, 3);

	offset <<= 1;

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 4);

	const u32 val = read16(index + offset);
	m_R[src_dst] = val;

	CLRFLAG(FLAG_E);
}

INST(ADDI)
{
	u32 imm = BIT(opcode, 9, 4);
	const u32 src = BIT(opcode, 3, 3);
	const u32 dst = BIT(opcode, 0, 3);

	if (TESTFLAG(FLAG_E))
		imm = get_extended_operand(imm, 4);
	else
		imm = util::sext(imm, 4);

	m_R[dst] = add_with_lfags(m_R[src], imm);

	CLRFLAG(FLAG_E);
}

INST(SUBI)
{
	u32 imm = BIT(opcode, 9, 4);
	const u32 src = BIT(opcode, 3, 3);
	const u32 dst = BIT(opcode, 0, 3);

	if (TESTFLAG(FLAG_E))
		imm = get_extended_operand(imm, 4);
	else
		imm = util::sext(imm, 4);

	m_R[dst] = sub_with_lfags(m_R[src], imm);

	CLRFLAG(FLAG_E);
}

INST(ADCI)
{
	u32 imm = BIT(opcode, 9, 4);
	const u32 src = BIT(opcode, 3, 3);
	const u32 dst = BIT(opcode, 0, 3);

	if (TESTFLAG(FLAG_E))
		imm = get_extended_operand(imm, 4);
	else
		imm = util::sext(imm, 4);

	m_R[dst] = adc_with_lfags(m_R[src], imm);

	CLRFLAG(FLAG_E);
}

INST(SBCI)
{
	u32 imm = BIT(opcode, 9, 4);
	const u32 src = BIT(opcode, 3, 3);
	const u32 dst = BIT(opcode, 0, 3);

	if (TESTFLAG(FLAG_E))
		imm = get_extended_operand(imm, 4);
	else
		imm = util::sext(imm, 4);

	m_R[dst] = sbc_with_lfags(m_R[src], imm);

	CLRFLAG(FLAG_E);
}

INST(ANDI)
{
	u32 imm = BIT(opcode, 9, 4);
	const u32 src = BIT(opcode, 3, 3);
	const u32 dst = BIT(opcode, 0, 3);

	if (TESTFLAG(FLAG_E))
		imm = get_extended_operand(imm, 4);
	else
		imm = util::sext(imm, 4);

	m_R[dst] = m_R[src] & imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if (!m_R[dst])
		SETFLAG(FLAG_Z);
	if (m_R[dst] & 0x80000000)
		SETFLAG(FLAG_S);
}

INST(ORI)
{
	u32 imm = BIT(opcode, 9, 4);
	const u32 src = BIT(opcode, 3, 3);
	const u32 dst = BIT(opcode, 0, 3);

	if (TESTFLAG(FLAG_E))
		imm = get_extended_operand(imm, 4);
	else
		imm = util::sext(imm, 4);

	m_R[dst] = m_R[src] | imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if (!m_R[dst])
		SETFLAG(FLAG_Z);
	if (m_R[dst] & 0x80000000)
		SETFLAG(FLAG_S);
}

INST(XORI)
{
	u32 imm = BIT(opcode, 9, 4);
	const u32 src = BIT(opcode, 3, 3);
	const u32 dst = BIT(opcode, 0, 3);

	if (TESTFLAG(FLAG_E))
		imm = get_extended_operand(imm, 4);
	else
		imm = util::sext(imm, 4);

	m_R[dst] = m_R[src] ^ imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if (!m_R[dst])
		SETFLAG(FLAG_Z);
	if (m_R[dst] & 0x80000000)
		SETFLAG(FLAG_S);
}

INST(CMPI)
{
	u32 imm = BIT(opcode, 9, 4);
	const u32 src = BIT(opcode, 3, 3);

	if (TESTFLAG(FLAG_E))
		imm = get_extended_operand(imm, 4);
	else
		imm = util::sext(imm, 4);

	sub_with_lfags(m_R[src], imm);

	CLRFLAG(FLAG_E);
}

INST(TSTI)
{
	u32 imm = BIT(opcode, 9, 4);
	const u32 src = BIT(opcode, 3, 3);

	if (TESTFLAG(FLAG_E))
		imm = get_extended_operand(imm, 4);
	else
		imm = util::sext(imm, 4);

	const u32 dst = m_R[src] & imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if (!dst)
		SETFLAG(FLAG_Z);
	if (dst & 0x80000000)
		SETFLAG(FLAG_S);
}

INST(ADD)
{
	const u32 src2 = BIT(opcode, 9, 3);
	const u32 src1 = BIT(opcode, 3, 3);
	const u32 dst = BIT(opcode, 0, 3);

	m_R[dst] = add_with_lfags(m_R[src1], m_R[src2]);
}

INST(SUB)
{
	const u32 src2 = BIT(opcode, 9, 3);
	const u32 src1 = BIT(opcode, 3, 3);
	const u32 dst = BIT(opcode, 0, 3);

	m_R[dst] = sub_with_lfags(m_R[src1], m_R[src2]);
}

INST(ADC)
{
	const u32 src2 = BIT(opcode, 9, 3);
	const u32 src1 = BIT(opcode, 3, 3);
	const u32 dst = BIT(opcode, 0, 3);

	m_R[dst] = adc_with_lfags(m_R[src1], m_R[src2]);
}

INST(SBC)
{
	const u32 src2 = BIT(opcode, 9, 3);
	const u32 src1 = BIT(opcode, 3, 3);
	const u32 dst = BIT(opcode, 0, 3);

	m_R[dst] = sbc_with_lfags(m_R[src1], m_R[src2]);
}

INST(AND)
{
	const u32 src2 = BIT(opcode, 9, 3);
	const u32 src1 = BIT(opcode, 3, 3);
	const u32 dst = BIT(opcode, 0, 3);

	m_R[dst] = m_R[src1] & m_R[src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if (!m_R[dst])
		SETFLAG(FLAG_Z);
	if (m_R[dst] & 0x80000000)
		SETFLAG(FLAG_S);
}

INST(OR)
{
	const u32 src2 = BIT(opcode, 9, 3);
	const u32 src1 = BIT(opcode, 3, 3);
	const u32 dst = BIT(opcode, 0, 3);

	m_R[dst] = m_R[src1] | m_R[src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if (!m_R[dst])
		SETFLAG(FLAG_Z);
	if (m_R[dst] & 0x80000000)
		SETFLAG(FLAG_S);

}

INST(XOR)
{
	const u32 src2 = BIT(opcode, 9, 3);
	const u32 src1 = BIT(opcode, 3, 3);
	const u32 dst = BIT(opcode, 0, 3);

	m_R[dst] = m_R[src1] ^ m_R[src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if (!m_R[dst])
		SETFLAG(FLAG_Z);
	if (m_R[dst] & 0x80000000)
		SETFLAG(FLAG_S);

}

INST(CMP)
{
	const u32 src2 = BIT(opcode, 9, 3);
	const u32 src1 = BIT(opcode, 3, 3);

	sub_with_lfags(m_R[src1], m_R[src2]);
}

INST(TST)
{
	const u32 src2 = BIT(opcode, 9, 3);
	const u32 src1 = BIT(opcode, 3, 3);

	const u32 dst = m_R[src1] & m_R[src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if (!dst)
		SETFLAG(FLAG_Z);
	if (dst & 0x80000000)
		SETFLAG(FLAG_S);
}

INST(MULS)
{
	const u32 src2 = BIT(opcode, 6, 3);
	const u32 src1 = BIT(opcode, 3, 3);
	const u32 dst = BIT(opcode, 0, 3);

	m_R[dst] = mul_with_lfags(m_R[src1], m_R[src2]);

	CLRFLAG(FLAG_E);
}

INST(NEG)
{
	const u32 dst = BIT(opcode, 9, 3);
	const u32 src = BIT(opcode, 3, 3);

	m_R[dst] = neg_with_lfags(m_R[src]);
}

INST(CALL)
{
	u32 offset = BIT(opcode, 0, 8);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 8);
	else
		offset = s8(offset);
	offset <<= 1;
	push_val(m_PC + 2);
	m_PC = m_PC + offset;

	CLRFLAG(FLAG_E);
}

INST(JV)
{
	u32 offset = BIT(opcode, 0, 8);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 8);
	else
		offset = s8(offset);
	offset <<= 1;

	if (TESTFLAG(FLAG_V))
	{
		m_PC = m_PC + offset;
	}

	CLRFLAG(FLAG_E);

}

INST(JNV)
{
	u32 offset = BIT(opcode, 0, 8);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 8);
	else
		offset = s8(offset);
	offset <<= 1;

	if (!TESTFLAG(FLAG_V))
	{
		m_PC = m_PC + offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JC)
{
	u32 offset = BIT(opcode, 0, 8);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 8);
	else
		offset = s8(offset);
	offset <<= 1;

	if (TESTFLAG(FLAG_C))
	{
		m_PC = m_PC + offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JNC)
{
	u32 offset = BIT(opcode, 0, 8);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 8);
	else
		offset = s8(offset);
	offset <<= 1;

	if (!TESTFLAG(FLAG_C))
	{
		m_PC = m_PC + offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JP)
{
	u32 offset = BIT(opcode, 0, 8);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 8);
	else
		offset = s8(offset);
	offset <<= 1;

	if (!TESTFLAG(FLAG_S))
	{
		m_PC = m_PC + offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JM)
{
	u32 offset = BIT(opcode, 0, 8);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 8);
	else
		offset = s8(offset);
	offset <<= 1;

	if (TESTFLAG(FLAG_S))
	{
		m_PC = m_PC + offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JNZ)
{
	u32 offset = BIT(opcode, 0, 8);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 8);
	else
		offset = s8(offset);
	offset <<= 1;

	if (!TESTFLAG(FLAG_Z))
	{
		m_PC = m_PC + offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JZ)
{
	u32 offset = BIT(opcode, 0, 8);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 8);
	else
		offset = s8(offset);
	offset <<= 1;

	if (TESTFLAG(FLAG_Z))
	{
		m_PC = m_PC + offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JGE)
{
	u32 offset = BIT(opcode, 0, 8);
	const u32 s = TESTFLAG(FLAG_S) ? 1 : 0;
	const u32 v = TESTFLAG(FLAG_V) ? 1 : 0;

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 8);
	else
		offset = s8(offset);
	offset <<= 1;

	if (!(s ^ v))
	{
		m_PC = m_PC + offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JLE)
{
	u32 offset = BIT(opcode, 0, 8);
	const u32 s = TESTFLAG(FLAG_S) ? 1 : 0;
	const u32 v = TESTFLAG(FLAG_V) ? 1 : 0;

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 8);
	else
		offset = s8(offset);
	offset <<= 1;

	if (TESTFLAG(FLAG_Z) || (s ^ v))
	{
		m_PC = m_PC + offset;
	}
	CLRFLAG(FLAG_E);
}

INST(JHI)
{
	u32 offset = BIT(opcode, 0, 8);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 8);
	else
		offset = s8(offset);
	offset <<= 1;

	if (!(TESTFLAG(FLAG_Z) || TESTFLAG(FLAG_C)))
	{
		m_PC = m_PC + offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JLS)
{
	u32 offset = BIT(opcode, 0, 8);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 8);
	else
		offset = s8(offset);
	offset <<= 1;

	if (TESTFLAG(FLAG_Z) || TESTFLAG(FLAG_C))
	{
		m_PC = m_PC + offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JGT)
{
	u32 offset = BIT(opcode, 0, 8);
	const u32 s = TESTFLAG(FLAG_S) ? 1 : 0;
	const u32 v = TESTFLAG(FLAG_V) ? 1 : 0;

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 8);
	else
		offset = s8(offset);
	offset <<= 1;

	if (!(TESTFLAG(FLAG_Z) || (s ^ v)))
	{
		m_PC = m_PC + offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JLT)
{
	u32 offset = BIT(opcode, 0, 8);
	const u32 s = TESTFLAG(FLAG_S) ? 1 : 0;
	const u32 v = TESTFLAG(FLAG_V) ? 1 : 0;

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 8);
	else
		offset = s8(offset);
	offset <<= 1;

	if (s ^ v)
	{
		m_PC = m_PC + offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JMP)
{
	u32 offset = BIT(opcode, 0, 8);

	if (TESTFLAG(FLAG_E))
		offset = get_extended_operand(offset, 8);
	else
		offset = s8(offset);

	offset <<= 1;

	m_PC = m_PC + offset;

	CLRFLAG(FLAG_E);
}

INST(JR)
{
	const u32 src = BIT(opcode, 0, 4);

	m_PC = m_R[src] - 2;

	CLRFLAG(FLAG_E);
}

INST(CALLR)
{
	const u32 src = BIT(opcode, 0, 4);
	push_val(m_PC + 2);
	m_PC = m_R[src] - 2;

	CLRFLAG(FLAG_E);
}

INST(ASR)
{
	const u32 dst = BIT(opcode, 0, 3);

	if (BIT(opcode, 10))
	{
		const u32 cnt = BIT(opcode, 5, 3);
		m_R[dst] = asr_with_lfags(m_R[dst], m_R[cnt] & 0x1f);
	}
	else
	{
		const u32 imm = BIT(opcode, 5, 5);
		m_R[dst] = asr_with_lfags(m_R[dst], imm);
	}

	CLRFLAG(FLAG_E);
}

INST(LSR)
{
	const u32 dst = BIT(opcode, 0, 3);

	if (BIT(opcode, 10))
	{
		const u32 cnt = BIT(opcode, 5, 3);
		m_R[dst] = lsr_with_lfags(m_R[dst], m_R[cnt] & 0x1f);
	}
	else
	{
		const u32 imm = BIT(opcode, 5, 5);
		m_R[dst] = lsr_with_lfags(m_R[dst], imm);
	}

	CLRFLAG(FLAG_E);
}

INST(ASL)
{
	const u32 dst = BIT(opcode, 0, 3);

	if (BIT(opcode, 10))
	{
		const u32 cnt = BIT(opcode, 5, 3);
		m_R[dst] = asl_with_lfags(m_R[dst], m_R[cnt] & 0x1f);
	}
	else
	{
		const u32 imm = BIT(opcode, 5, 5);
		m_R[dst] = asl_with_lfags(m_R[dst], imm);
	}

	CLRFLAG(FLAG_E);
}

INST(EXTB)
{
	const u32 dst = BIT(opcode,0,4);
	const u32 val = m_R[dst];

	m_R[dst] = s8(val & 0xff);

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if (!m_R[dst])
		SETFLAG(FLAG_Z);
	if (m_R[dst] & 0x80000000)
		SETFLAG(FLAG_S);
}

INST(EXTS)
{
	const u32 dst = BIT(opcode,0,4);
	const u32 val = m_R[dst];

	m_R[dst] = s16(val & 0xffff);

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if (!m_R[dst])
		SETFLAG(FLAG_Z);
	if (m_R[dst] & 0x80000000)
		SETFLAG(FLAG_S);
}

INST(SET)
{
	const u32 imm = BIT(opcode, 0, 4);

	m_SR |= (1 << imm);
}

INST(CLR)
{
	const u32 imm = BIT(opcode, 0, 4);

	m_SR &= ~(1 << imm);
}

void se3208_device::take_exception_vector(u8 vector)
{
	debugger_exception_hook(vector);
	m_PC = read32(4 * vector);
}

INST(SWI)
{
	const u32 imm = BIT(opcode, 0, 4);

	if (!TESTFLAG(FLAG_ENI))
		return;
	push_val(m_PC);
	push_val(m_SR);

	CLRFLAG(FLAG_ENI|FLAG_E|FLAG_M);

	take_exception_vector(imm + 0x10);
	m_PC -= 2;
}

INST(HALT)
{
	const u32 imm = BIT(opcode, 0, 4);

	m_machinex_cb(0x10 | imm);

//  DEBUGMESSAGE("HALT\t0x%x",imm);
}

INST(MVTC)
{
//  const u32 imm = BIT(opcode, 0, 4);

//  DEBUGMESSAGE("MVTC\t%%R0,%%CR%d",imm);
}

INST(MVFC)
{
//  const u32 imm = BIT(opcode, 0, 4);

//  DEBUGMESSAGE("MVFC\t%%CR0%d,%%R0",imm);
}


se3208_device::OP se3208_device::decode_op(u16 opcode)
{
	switch (BIT(opcode, 14, 2))
	{
		case 0x0:
			{
				const u8 op = BIT(opcode, 11, 3);
				switch (op)
				{
					case 0x0:
						return &se3208_device::LDB;
					case 0x1:
						return &se3208_device::LDS;
					case 0x2:
						return &se3208_device::LD;
					case 0x3:
						return &se3208_device::LDBU;
					case 0x4:
						return &se3208_device::STB;
					case 0x5:
						return &se3208_device::STS;
					case 0x6:
						return &se3208_device::ST;
					case 0x7:
						return &se3208_device::LDSU;
				}
			}
			break;
		case 0x1:
			return &se3208_device::LERI;
		case 0x2:
			{
				switch (BIT(opcode, 11, 3))
				{
					case 0:
						return &se3208_device::LDSP;
					case 1:
						return &se3208_device::STSP;
					case 2:
						return &se3208_device::PUSH;
					case 3:
						return &se3208_device::POP;
					case 4:
					case 5:
					case 6:
					case 7:
					case 8: //arith
					case 9:
					case 10:
					case 11:
					case 12:
					case 13:
					case 14:
					case 15:
						switch (BIT(opcode, 6, 3))
						{
							case 0:
								return &se3208_device::ADDI;
							case 1:
								return &se3208_device::ADCI;
							case 2:
								return &se3208_device::SUBI;
							case 3:
								return &se3208_device::SBCI;
							case 4:
								return &se3208_device::ANDI;
							case 5:
								return &se3208_device::ORI;
							case 6:
								return &se3208_device::XORI;
							case 7:
								switch (BIT(opcode, 0, 3))
								{
									case 0:
										return &se3208_device::CMPI;
									case 1:
										return &se3208_device::TSTI;
									case 2:
										return &se3208_device::LEATOSP;
									case 3:
										return &se3208_device::LEAFROMSP;
								}
								break;
						}
						break;
				}
			}
			break;
		case 3:
			switch (BIT(opcode, 12, 2))
			{
				case 0:
					switch (BIT(opcode, 6, 3))
					{
						case 0:
							return &se3208_device::ADD;
						case 1:
							return &se3208_device::ADC;
						case 2:
							return &se3208_device::SUB;
						case 3:
							return &se3208_device::SBC;
						case 4:
							return &se3208_device::AND;
						case 5:
							return &se3208_device::OR;
						case 6:
							return &se3208_device::XOR;
						case 7:
							switch (BIT(opcode, 0, 3))
							{
								case 0:
									return &se3208_device::CMP;
								case 1:
									return &se3208_device::TST;
								case 2:
									return &se3208_device::MOV;
								case 3:
									return &se3208_device::NEG;
							}
							break;
					}
					break;
				case 1:     //Jumps
					switch (BIT(opcode, 8, 4))
					{
						case 0x0:
							return &se3208_device::JNV;
						case 0x1:
							return &se3208_device::JV;
						case 0x2:
							return &se3208_device::JP;
						case 0x3:
							return &se3208_device::JM;
						case 0x4:
							return &se3208_device::JNZ;
						case 0x5:
							return &se3208_device::JZ;
						case 0x6:
							return &se3208_device::JNC;
						case 0x7:
							return &se3208_device::JC;
						case 0x8:
							return &se3208_device::JGT;
						case 0x9:
							return &se3208_device::JLT;
						case 0xa:
							return &se3208_device::JGE;
						case 0xb:
							return &se3208_device::JLE;
						case 0xc:
							return &se3208_device::JHI;
						case 0xd:
							return &se3208_device::JLS;
						case 0xe:
							return &se3208_device::JMP;
						case 0xf:
							return &se3208_device::CALL;
					}
					break;
				case 2:
					if (BIT(opcode, 11))
						return &se3208_device::LDI;
					else    //SP Ops
					{
						if (BIT(opcode, 10))
						{
							switch (BIT(opcode, 7, 3))
							{
								case 0:
									return &se3208_device::LDBSP;
								case 1:
									return &se3208_device::LDSSP;
								case 3:
									return &se3208_device::LDBUSP;
								case 4:
									return &se3208_device::STBSP;
								case 5:
									return &se3208_device::STSSP;
								case 7:
									return &se3208_device::LDSUSP;
							}
						}
						else
						{
							if (BIT(opcode, 9))
							{
								return &se3208_device::LEASPTOSP;
							}
							else
							{
								if (BIT(opcode, 8))
								{
								}
								else
								{
									switch (BIT(opcode, 4, 4))
									{
										case 0:
											return &se3208_device::EXTB;
										case 1:
											return &se3208_device::EXTS;
										case 8:
											return &se3208_device::JR;
										case 9:
											return &se3208_device::CALLR;
										case 10:
											return &se3208_device::SET;
										case 11:
											return &se3208_device::CLR;
										case 12:
											return &se3208_device::SWI;
										case 13:
											return &se3208_device::HALT;
									}
								}
							}
						}
					}
					break;
				case 3:
					switch (BIT(opcode, 9, 3))
					{
						case 0:
						case 1:
						case 2:
						case 3:
							switch (BIT(opcode, 3, 2))
							{
								case 0:
									return &se3208_device::ASR;
								case 1:
									return &se3208_device::LSR;
								case 2:
									return &se3208_device::ASL;
								//case 3:
								//  return &se3208_device::LSL;
							}
							break;
						case 4:
							return &se3208_device::MULS;
						case 6:
							if (BIT(opcode, 3))
								return &se3208_device::MVFC;
							else
								return &se3208_device::MVTC;
					}
					break;
			}
			break;

	}
	return &se3208_device::INVALIDOP;
}


void se3208_device::build_table()
{
	for (int i = 0; i < 0x10000; ++i)
		m_optable[i] = decode_op(i);
}

void se3208_device::device_reset()
{
	for (auto & elem : m_R)
	{
		elem = 0;
	}
	m_SP = 0;
	m_ER = 0;
	m_PPC = 0;
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);
	m_PC = read32(0);
	m_SR = 0;
	m_IRQ = CLEAR_LINE;
	m_NMI = CLEAR_LINE;
}

void se3208_device::nmi_execute()
{
	standard_irq_callback(INPUT_LINE_NMI, m_PC);
	m_machinex_cb(0x00);

	push_val(m_PC);
	push_val(m_SR);

	CLRFLAG(FLAG_NMI|FLAG_ENI|FLAG_E|FLAG_M);

	take_exception_vector(0x01);
}

void se3208_device::interrupt_execute()
{
	if (!TESTFLAG(FLAG_ENI))
		return;

	standard_irq_callback(0, m_PC);
	m_machinex_cb(0x01);

	push_val(m_PC);
	push_val(m_SR);

	CLRFLAG(FLAG_ENI|FLAG_E|FLAG_M);

	if (!(TESTFLAG(FLAG_AUT)))
		take_exception_vector(0x02);
	else
		take_exception_vector(m_iackx_cb());
}


void se3208_device::execute_run()
{
	do
	{
		const u16 opcode = m_cache.read_word(m_PC, WORD_XOR_LE(0));

		m_PPC = m_PC;
		debugger_instruction_hook(m_PC);

		(this->*m_optable[opcode])(opcode);
		m_PC += 2;
		//Check interrupts
		if (m_NMI == ASSERT_LINE)
		{
			nmi_execute();
			m_NMI = CLEAR_LINE;
		}
		else if (m_IRQ == ASSERT_LINE && TESTFLAG(FLAG_ENI))
		{
			interrupt_execute();
		}
		--(m_icount);
	} while(m_icount > 0);
}

void se3208_device::device_start()
{
	build_table();

	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);

	save_item(NAME(m_R));
	save_item(NAME(m_PC));
	save_item(NAME(m_SR));
	save_item(NAME(m_SP));
	save_item(NAME(m_ER));
	save_item(NAME(m_IRQ));
	save_item(NAME(m_NMI));

	state_add(SE3208_PC,  "PC", m_PC).formatstr("%08X");
	state_add(SE3208_SR,  "SR", m_SR).formatstr("%08X");
	state_add(SE3208_ER,  "ER", m_ER).formatstr("%08X");
	state_add(SE3208_SP,  "SP", m_SP).formatstr("%08X");
	state_add(SE3208_R0,  "R0", m_R[0]).formatstr("%08X");
	state_add(SE3208_R1,  "R1", m_R[1]).formatstr("%08X");
	state_add(SE3208_R2,  "R2", m_R[2]).formatstr("%08X");
	state_add(SE3208_R3,  "R3", m_R[3]).formatstr("%08X");
	state_add(SE3208_R4,  "R4", m_R[4]).formatstr("%08X");
	state_add(SE3208_R5,  "R5", m_R[5]).formatstr("%08X");
	state_add(SE3208_R6,  "R6", m_R[6]).formatstr("%08X");
	state_add(SE3208_R7,  "R7", m_R[7]).formatstr("%08X");
	state_add(SE3208_PPC, "PPC", m_PPC).formatstr("%08X");

	state_add(STATE_GENPC, "GENPC", m_PC).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_PPC).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_SR).formatstr("%10s").noshow();

	set_icountptr(m_icount);
}


void se3208_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c %c%c%c%c%c",
					m_SR & FLAG_C ? 'C' : '.',
					m_SR & FLAG_V ? 'V' : '.',
					m_SR & FLAG_S ? 'S' : '.',
					m_SR & FLAG_Z ? 'Z' : '.',

					m_SR & FLAG_M ? 'M' : '.',
					m_SR & FLAG_E ? 'E' : '.',
					m_SR & FLAG_AUT ? 'A' : '.',
					m_SR & FLAG_ENI ? 'I' : '.',
					m_SR & FLAG_NMI ? 'N' : '.'
			);
			break;
	}
}

void se3208_device::execute_set_input(int line, int state)
{
	if (line == INPUT_LINE_NMI)    //NMI
		m_NMI = state;
	else
		m_IRQ = state;
}

std::unique_ptr<util::disasm_interface> se3208_device::create_disassembler()
{
	return std::make_unique<se3208_disassembler>();
}

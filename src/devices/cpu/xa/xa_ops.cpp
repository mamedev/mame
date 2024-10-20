// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "xa.h"
#include "xadasm.h"

#define get_addr gr16
#define set_addr sr16

u32 xa_cpu::gr32(int reg)
{
	if (reg & 1)
		fatalerror("gr32 with low bit set\n");

	u16 reg1 = gr16(reg);
	u16 reg2 = gr16(reg + 1);

	return (reg2 << 16) | reg1;
}

void xa_cpu::sr32(int reg, u32 data)
{
	if (reg & 1)
		fatalerror("sr32 with low bit set\n");

	sr16(reg, data & 0xffff);
	sr16(reg + 1, (data >> 16) & 0xffff);
}

u16 xa_cpu::do_subb_16(u16 val1, u16 val2)
{
	return do_sub_16_helper(val1, val2, get_c_flag());
}

u16 xa_cpu::do_sub_16(u16 val1, u16 val2)
{
	return do_sub_16_helper(val1, val2, 0);
}

u16 xa_cpu::do_sub_16_helper(u16 val1, u16 val2, u8 c)
{
	u32 result = val1 - (val2 + c);
	s32 result1 = (s16)val1 - (s16)(val2 + c);
	u32 result3 = (val1 & 0x0fff) - ((val2 & 0x0fff) + c);

	if (result & 0x10000) set_c_flag(); else clear_c_flag();
	if (result3 & 0x1000) set_ac_flag(); else clear_ac_flag();
	if ((result1 < -32768 || result1 > 32767)) set_v_flag(); else clear_v_flag();

	do_nz_flags_16((u16)result);
	return (u16)result;
}


u16 xa_cpu::do_addc_16(u16 val1, u16 val2)
{
	return do_add_16_helper(val1, val2, get_c_flag());
}

u16 xa_cpu::do_add_16(u16 val1, u16 val2)
{
	return do_add_16_helper(val1, val2, 0);
}


u16 xa_cpu::do_add_16_helper(u16 val1, u16 val2, u8 c)
{
	u32 result = val1 + val2 + c;
	s32 result1 = (s16)val1 + (s16)val2 + c;
	u32 result3 = (val1 & 0x0fff) + (val2 & 0x0fff) + c;
	if (result & 0x10000) set_c_flag(); else clear_c_flag();
	if (result3 & 0x1000) set_ac_flag(); else clear_ac_flag();
	if ((result1 < -32768 || result1 > 32767)) set_v_flag(); else clear_v_flag();

	do_nz_flags_16((u16)result);
	return (u16)result;
}


u16 xa_cpu::do_xor_16(u16 val1, u16 val2)
{
	u16 result = val1 ^ val2;
	do_nz_flags_16(result);
	return result;
}

u16 xa_cpu::do_or_16(u16 val1, u16 val2)
{
	u16 result = val1 | val2;
	do_nz_flags_16(result);
	return result;
}

u16 xa_cpu::do_and_16(u16 val1, u16 val2)
{
	u16 result = val1 & val2;
	do_nz_flags_16(result);
	return result;
}

u32 xa_cpu::asl32_helper(u32 fullreg, u8 amount)
{
	cy(6 + (amount >> 1));

	int lastbit = 0;

	u32 topbit = fullreg & 0x8000000;

	while (amount)
	{
		lastbit = fullreg & 0x8000000;
		fullreg = fullreg << 1;
		amount--;
	}

	if (lastbit)
		set_c_flag();
	else
		clear_c_flag();

	if (fullreg == 0)
		set_z_flag();
	else
		clear_z_flag();

	if (fullreg & 0x8000000)
		set_n_flag();
	else
		clear_n_flag();

	if (topbit != (fullreg & 0x8000000))
		set_v_flag();
	else
		clear_v_flag();

	return fullreg;
}

u32 xa_cpu::lsr32_helper(u32 fullreg, u8 amount)
{
	cy(6 + (amount >> 1));

	int lastbit = 0;

	while (amount)
	{
		lastbit = fullreg & 0x0000001;
		fullreg = fullreg >> 1;
		amount--;
	}

	if (lastbit)
		set_c_flag();
	else
		clear_c_flag();

	if (fullreg == 0)
		set_z_flag();
	else
		clear_z_flag();

	if (fullreg & 0x8000000)
		set_n_flag();
	else
		clear_n_flag();

	return fullreg;
}

u16 xa_cpu::lsr16_helper(u16 fullreg, u8 amount)
{
	cy(6 + (amount >> 1));

	int lastbit = 0;

	while (amount)
	{
		lastbit = fullreg & 0x0001;
		fullreg = fullreg >> 1;
		amount--;
	}

	if (lastbit)
		set_c_flag();
	else
		clear_c_flag();

	if (fullreg == 0)
		set_z_flag();
	else
		clear_z_flag();

	if (fullreg & 0x8000)
		set_n_flag();
	else
		clear_n_flag();

	return fullreg;
}


u8 xa_cpu::do_subb_8(u8 val1, u8 val2)
{
	return do_sub_8_helper(val1, val2, get_c_flag());
}

u8 xa_cpu::do_sub_8(u8 val1, u8 val2)
{
	return do_sub_8_helper(val1, val2, 0);
}

u8 xa_cpu::do_sub_8_helper(u8 val1, u8 val2, u8 c)
{
	u16 result = val1 - (val2 + c);
	s16 result1 = (s8)val1 - (s8)(val2 + c);
	u16 result3 = (val1 & 0x0f) - ((val2 & 0x0f) + c);
	if (result & 0x100) set_c_flag(); else clear_c_flag();
	if (result3 & 0x10) set_ac_flag(); else clear_ac_flag();
	if ((result1 < -128 || result1 > 127)) set_v_flag(); else clear_v_flag();

	do_nz_flags_8((u8)result);
	return (u8)result;
}

u8 xa_cpu::do_addc_8(u8 val1, u8 val2)
{
	return do_add_8_helper(val1, val2, get_c_flag());
}

u8 xa_cpu::do_add_8(u8 val1, u8 val2)
{
	return do_add_8_helper(val1, val2, 0);
}


u8 xa_cpu::do_add_8_helper(u8 val1, u8 val2, u8 c)
{
	u16 result = val1 + val2 + c;
	s16 result1 = (s8)val1 + (s8)val2 + c;
	u16 result3 = (val1 & 0x0f) + (val2 & 0x0f) + c;
	if (result & 0x100) set_c_flag(); else clear_c_flag();
	if (result3 & 0x10) set_ac_flag(); else clear_ac_flag();
	if ((result1 < -128 || result1 > 127)) set_v_flag(); else clear_v_flag();

	do_nz_flags_8((u8)result);
	return (u8)result;
}

u8 xa_cpu::do_xor_8(u8 val1, u8 val2)
{
	u8 result = val1 ^ val2;
	do_nz_flags_8(result);
	return result;
}

u8 xa_cpu::do_or_8(u8 val1, u8 val2)
{
	u8 result = val1 | val2;
	do_nz_flags_8(result);
	return result;
}

u8 xa_cpu::do_and_8(u8 val1, u8 val2)
{
	u8 result = val1 & val2;
	do_nz_flags_8(result);
	return result;
}



u8 xa_cpu::do_cjne_8_helper(u8 val1, u8 val2)
{
	u16 result = val1 - val2;
	do_nz_flags_8((u8)result);
	if (result & 0x100) set_c_flag(); else clear_c_flag();
	return (u8)result;
}


void xa_cpu::set_bit_8_helper(u16 bit, u8 val)
{
	int position = bit & 7;

	if (bit < 0x100)
	{
		int reg = ((bit & 0x1ff) >> 3);

		if (reg < 16)
			fatalerror("set_bit_helper %s.%d", m_regnames8[reg], position);
		else
			fatalerror("set_bit_helper ill_REG_%02x.%d", reg, position);
	}
	else if (bit < 0x200)
	{
		int addr = ((bit & 0x1ff) >> 3) + 0x20;
		fatalerror("set_bit_helper $%02x.%d", addr, position);
	}

	int sfr = ((bit & 0x1ff) >> 3);
	u8 mask = (1 << position) ^ 0xff;

	u8 sfr_val = m_sfr->read_byte(sfr) & mask;
	sfr_val |= (val << position);
	m_sfr->write_byte(sfr, sfr_val);
}

// NOP                         No operation                                                            1 3         0000 0000
void xa_cpu::do_nop() { cy(3); }

// ALUOP.b Rd, data8
// ADD Rd, #data8              Add 8-bit imm data to reg                                               3 3         1001 0001  dddd 0000  iiii iiii
// ADDC Rd, #data8             Add 8-bit imm data to reg w/ carry                                      3 3         1001 0001  dddd 0001  iiii iiii
// SUB Rd, #data8              Subtract 8-bit imm data to reg                                          3 3         1001 0001  dddd 0010  iiii iiii
// SUBB Rd, #data8             Subtract w/ borrow 8-bit imm data to reg                                3 3         1001 0001  dddd 0011  iiii iiii
// CMP Rd, #data8              Compare 8-bit imm data to reg                                           3 3         1001 0001  dddd 0100  iiii iiii
// AND Rd, #data8              Logical AND 8-bit imm data to reg                                       3 3         1001 0001  dddd 0101  iiii iiii
// OR Rd, #data8               Logical OR 8-bit imm data to reg                                        3 3         1001 0001  dddd 0110  iiii iiii
// XOR Rd, #data8              Logical XOR 8-bit imm data to reg                                       3 3         1001 0001  dddd 0111  iiii iiii
// MOV Rd, #data8              Move 8-bit imm data to reg                                              3 3         1001 0001  dddd 1000  iiii iiii
void xa_cpu::aluop_byte_rd_data8(int alu_op, u8 rd, u8 data8)
{
	switch (alu_op)
	{
	case 0x0: add_byte_rd_data8(rd, data8); break;
	case 0x1: addc_byte_rd_data8(rd, data8); break;
	case 0x2: sub_byte_rd_data8(rd, data8); break;
	case 0x3: subb_byte_rd_data8(rd, data8); break;
	case 0x4: cmp_byte_rd_data8(rd, data8); break;
	case 0x5: and_byte_rd_data8(rd, data8); break;
	case 0x6: or_byte_rd_data8(rd, data8); break;
	case 0x7: xor_byte_rd_data8(rd, data8); break;
	case 0x8: mov_byte_rd_data8(rd, data8); break;
	default: logerror("UNK_ALUOP.b %s, #$%02x", m_regnames8[rd], data8); do_nop(); break;
	}
}

void xa_cpu::add_byte_rd_data8(u8 rd, u8 data8) { u8 rdval = gr8(rd); u8 result = do_add_8(rdval, data8); sr8(rd, result); cy(3); }
void xa_cpu::addc_byte_rd_data8(u8 rd, u8 data8){ u8 rdval = gr8(rd); u8 result = do_addc_8(rdval, data8); sr8(rd, result); cy(3); }
void xa_cpu::sub_byte_rd_data8(u8 rd, u8 data8) { u8 rdval = gr8(rd); u8 result = do_sub_8(rdval, data8); sr8(rd, result); cy(3); }
void xa_cpu::subb_byte_rd_data8(u8 rd, u8 data8){ u8 rdval = gr8(rd); u8 result = do_subb_8(rdval, data8); sr8(rd, result); cy(3); }
void xa_cpu::cmp_byte_rd_data8(u8 rd, u8 data8) { u8 rdval = gr8(rd); do_sub_8(rdval, data8); cy(3); }
void xa_cpu::and_byte_rd_data8(u8 rd, u8 data8) { u8 rdval = gr8(rd); u8 result = do_and_8(rdval, data8); sr8(rd, result); cy(3); }
void xa_cpu::or_byte_rd_data8(u8 rd, u8 data8)  { u8 rdval = gr8(rd); u8 result = do_or_8(rdval, data8); sr8(rd, result); cy(3); }
void xa_cpu::xor_byte_rd_data8(u8 rd, u8 data8) { u8 rdval = gr8(rd); u8 result = do_xor_8(rdval, data8); sr8(rd, result); cy(3); }
void xa_cpu::mov_byte_rd_data8(u8 rd, u8 data8) { u8 result = data8; do_nz_flags_8(result); sr8(rd, result); cy(3); }

// ------------------------------------------

// ALUOP.b [Rd], data8
// ADD [Rd], #data8            Add 8-bit imm data to reg-ind                                           3 4         1001 0010  0ddd 0000  iiii iiii
// ADDC [Rd], #data8           Add 16-bit imm data to reg-ind w/ carry                                 3 4         1001 0010  0ddd 0001  iiii iiii
// SUB [Rd], #data8            Subtract 8-bit imm data to reg-ind                                      3 4         1001 0010  0ddd 0010  iiii iiii
// SUBB [Rd], #data8           Subtract w/ borrow 8-bit imm data to reg-ind                            3 4         1001 0010  0ddd 0011  iiii iiii
// CMP [Rd], #data8            Compare 8-bit imm data to reg-ind                                       3 4         1001 0010  0ddd 0100  iiii iiii
// AND [Rd], #data8            Logical AND 8-bit imm data to reg-ind                                   3 4         1001 0010  0ddd 0101  iiii iiii
// OR [Rd], #data8             Logical OR 8-bit imm data to reg-ind                                    3 4         1001 0010  0ddd 0110  iiii iiii
// XOR [Rd], #data8            Logical XOR 8-bit imm data to reg-ind                                   3 4         1001 0010  0ddd 0111  iiii iiii
// MOV [Rd], #data8            Move 16-bit imm data to reg-ind                                         3 3         1001 0010  0ddd 1000  iiii iiii
void xa_cpu::aluop_byte_indrd_data8(int alu_op, u8 rd, u8 data8)
{
	switch (alu_op)
	{
	case 0x0: add_byte_indrd_data8(rd, data8); break;
	case 0x1: addc_byte_indrd_data8(rd, data8); break;
	case 0x2: sub_byte_indrd_data8(rd, data8); break;
	case 0x3: subb_byte_indrd_data8(rd, data8); break;
	case 0x4: cmp_byte_indrd_data8(rd, data8); break;
	case 0x5: and_byte_indrd_data8(rd, data8); break;
	case 0x6: or_byte_indrd_data8(rd, data8); break;
	case 0x7: xor_byte_indrd_data8(rd, data8); break;
	case 0x8: mov_byte_indrd_data8(rd, data8); break;
	default: logerror("UNK_ALUOP.b [%s], #$%02x", m_regnames16[rd], data8); do_nop(); break;
	}
}
void xa_cpu::add_byte_indrd_data8(u8 rd, u8 data8) { fatalerror( "ADD.b [%s], #$%02x ([RD], DATA8)", m_regnames16[rd], data8 ); }
void xa_cpu::addc_byte_indrd_data8(u8 rd, u8 data8){ fatalerror( "ADDC.b [%s], #$%02x ([RD], DATA8)", m_regnames16[rd], data8 ); }
void xa_cpu::sub_byte_indrd_data8(u8 rd, u8 data8) { fatalerror( "SUB.b [%s], #$%02x ([RD], DATA8)", m_regnames16[rd], data8 ); }
void xa_cpu::subb_byte_indrd_data8(u8 rd, u8 data8){ fatalerror( "SUBB.b [%s], #$%02x ([RD], DATA8)", m_regnames16[rd], data8 ); }
void xa_cpu::cmp_byte_indrd_data8(u8 rd, u8 data8) { u16 address = get_addr(rd); u8 rdval = rdat8(address); do_sub_8(rdval, data8); cy(3); }
void xa_cpu::and_byte_indrd_data8(u8 rd, u8 data8) { u16 address = get_addr(rd); u8 rdval = rdat8(address); u8 result = do_and_8(rdval, data8); wdat8(address, result); cy(3); }
void xa_cpu::or_byte_indrd_data8(u8 rd, u8 data8)  { fatalerror( "OR.b [%s], #$%02x ([RD], DATA8)", m_regnames16[rd], data8 ); }
void xa_cpu::xor_byte_indrd_data8(u8 rd, u8 data8) { fatalerror( "XOR.b [%s], #$%02x ([RD], DATA8)", m_regnames16[rd], data8 ); }
void xa_cpu::mov_byte_indrd_data8(u8 rd, u8 data8) { fatalerror( "MOV.b [%s], #$%02x ([RD], DATA8)", m_regnames16[rd], data8 ); }

// ------------------------------------------
// ALUOP.b [Rd+], data8
// ADD [Rd+], #data8           Add 8-bit imm data to reg-ind w/ autoinc                                3 5         1001 0011  0ddd 0000  iiii iiii
// ADDC [Rd+], #data8          Add 8-bit imm data to reg-ind and autoinc w/ carry                      3 5         1001 0011  0ddd 0001  iiii iiii
// SUB [Rd+], #data8           Subtract 8-bit imm data to reg-ind w/ autoinc                           3 5         1001 0011  0ddd 0010  iiii iiii
// SUBB [Rd+], #data8          Subtract w/ borrow 8-bit imm data to reg-ind w/ autoinc                 3 5         1001 0011  0ddd 0011  iiii iiii
// CMP [Rd+], #data8           Compare 8-bit imm data to reg-ind w/ autoinc                            3 5         1001 0011  0ddd 0100  iiii iiii
// AND [Rd+], #data8           Logical AND 8-bit imm data to reg-ind and autoinc                       3 5         1001 0011  0ddd 0101  iiii iiii
// OR [Rd+], #data8            Logical OR 8-bit imm data to reg-ind w/ autoinc                         3 5         1001 0011  0ddd 0110  iiii iiii
// XOR [Rd+], #data8           Logical XOR 8-bit imm data to reg-ind w/ autoinc                        3 5         1001 0011  0ddd 0111  iiii iiii
// MOV [Rd+], #data8           Move 8-bit imm data to reg-ind w/ autoinc                               3 4         1001 0011  0ddd 1000  iiii iiii
void xa_cpu::aluop_byte_indrdinc_data8(int alu_op, u8 rd, u8 data8)
{
	switch (alu_op)
	{
	case 0x0: add_byte_indrdinc_data8(rd, data8); break;
	case 0x1: addc_byte_indrdinc_data8(rd, data8); break;
	case 0x2: sub_byte_indrdinc_data8(rd, data8); break;
	case 0x3: subb_byte_indrdinc_data8(rd, data8); break;
	case 0x4: cmp_byte_indrdinc_data8(rd, data8); break;
	case 0x5: and_byte_indrdinc_data8(rd, data8); break;
	case 0x6: or_byte_indrdinc_data8(rd, data8); break;
	case 0x7: xor_byte_indrdinc_data8(rd, data8); break;
	case 0x8: mov_byte_indrdinc_data8(rd, data8); break;
	default: logerror("UNK_ALUOP.b [%s+], #$%02x", m_regnames16[rd], data8); do_nop(); break;
	}
}
void xa_cpu::add_byte_indrdinc_data8(u8 rd, u8 data8) { fatalerror( "ADD.b [%s+], #$%02x ([RD+], DATA8)", m_regnames16[rd], data8 ); }
void xa_cpu::addc_byte_indrdinc_data8(u8 rd, u8 data8){ fatalerror( "ADDC.b [%s+], #$%02x ([RD+], DATA8)", m_regnames16[rd], data8 ); }
void xa_cpu::sub_byte_indrdinc_data8(u8 rd, u8 data8) { fatalerror( "SUB.b [%s+], #$%02x ([RD+], DATA8)", m_regnames16[rd], data8 ); }
void xa_cpu::subb_byte_indrdinc_data8(u8 rd, u8 data8){ fatalerror( "SUBB.b [%s+], #$%02x ([RD+], DATA8)", m_regnames16[rd], data8 ); }
void xa_cpu::cmp_byte_indrdinc_data8(u8 rd, u8 data8) { fatalerror( "CMP.b [%s+], #$%02x ([RD+], DATA8)", m_regnames16[rd], data8 ); }
void xa_cpu::and_byte_indrdinc_data8(u8 rd, u8 data8) { fatalerror( "AND.b [%s+], #$%02x ([RD+], DATA8)", m_regnames16[rd], data8 ); }
void xa_cpu::or_byte_indrdinc_data8(u8 rd, u8 data8)  { fatalerror( "OR.b [%s+], #$%02x ([RD+], DATA8)", m_regnames16[rd], data8 ); }
void xa_cpu::xor_byte_indrdinc_data8(u8 rd, u8 data8) { fatalerror( "XOR.b [%s+], #$%02x ([RD+], DATA8)", m_regnames16[rd], data8 ); }
void xa_cpu::mov_byte_indrdinc_data8(u8 rd, u8 data8) { fatalerror( "MOV.b [%s+], #$%02x ([RD+], DATA8)", m_regnames16[rd], data8 ); }

// ------------------------------------------
// ALUOP.b [Rd+offs8], data8
// ADD [Rd+offset8], #data8    Add 8-bit imm data to reg-ind w/ 8-bit offs                             4 6         1001 0100  0ddd 0000  oooo oooo  iiii iiii
// ADDC [Rd+offset8], #data8   Add 8-bit imm data to reg-ind w/ 8-bit offs and carry                   4 6         1001 0100  0ddd 0001  oooo oooo  iiii iiii
// SUB [Rd+offset8], #data8    Subtract 8-bit imm data to reg-ind w/ 8-bit offs                        4 6         1001 0100  0ddd 0010  oooo oooo  iiii iiii
// SUBB [Rd+offset8], #data8   Subtract w/ borrow 8-bit imm data to reg-ind w/ 8-bit offs              4 6         1001 0100  0ddd 0011  oooo oooo  iiii iiii
// CMP [Rd+offset8], #data8    Compare 8-bit imm data to reg-ind w/ 8-bit offs                         4 6         1001 0100  0ddd 0100  oooo oooo  iiii iiii
// AND [Rd+offset8], #data8    Logical AND 8-bit imm data to reg-ind w/ 8-bit offs                     4 6         1001 0100  0ddd 0101  oooo oooo  iiii iiii
// OR [Rd+offset8], #data8     Logical OR 8-bit imm data to reg-ind w/ 8-bit offs                      4 6         1001 0100  0ddd 0110  oooo oooo  iiii iiii
// XOR [Rd+offset8], #data8    Logical XOR 8-bit imm data to reg-ind w/ 8-bit offs                     4 6         1001 0100  0ddd 0111  oooo oooo  iiii iiii
// MOV [Rd+offset8], #data8    Move 8-bit imm data to reg-ind w/ 8-bit offs                            4 5         1001 0100  0ddd 1000  oooo oooo  iiii iiii
void xa_cpu::aluop_byte_rdoff8_data8(int alu_op, u8 rd, u8 offset8, u8 data8)
{
	switch (alu_op)
	{
	case 0x0: add_byte_indrdoff8_data8(rd, offset8, data8); break;
	case 0x1: addc_byte_indrdoff8_data8(rd, offset8, data8); break;
	case 0x2: sub_byte_indrdoff8_data8(rd, offset8, data8); break;
	case 0x3: subb_byte_indrdoff8_data8(rd, offset8, data8); break;
	case 0x4: cmp_byte_indrdoff8_data8(rd, offset8, data8); break;
	case 0x5: and_byte_indrdoff8_data8(rd, offset8, data8); break;
	case 0x6: or_byte_indrdoff8_data8(rd, offset8, data8); break;
	case 0x7: xor_byte_indrdoff8_data8(rd, offset8, data8); break;
	case 0x8: mov_byte_indrdoff8_data8(rd, offset8, data8); break;
	default: logerror("UNK_ALUOP.b [%s+#$%02x], #$%02x", m_regnames16[rd], offset8, data8); do_nop(); break;
	}
}
void xa_cpu::add_byte_indrdoff8_data8(u8 rd, u8 offset8, u8 data8) { u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; u8 val = rdat8(address); u8 result = do_add_8(val, data8); wdat8(address, result); cy(6); }
void xa_cpu::addc_byte_indrdoff8_data8(u8 rd, u8 offset8, u8 data8){ u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; u8 val = rdat8(address); u8 result = do_addc_8(val, data8); wdat8(address, result); cy(6); }
void xa_cpu::sub_byte_indrdoff8_data8(u8 rd, u8 offset8, u8 data8) { u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; u8 val = rdat8(address); u8 result = do_sub_8(val, data8); wdat8(address, result); cy(6); }
void xa_cpu::subb_byte_indrdoff8_data8(u8 rd, u8 offset8, u8 data8){ u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; u8 val = rdat8(address); u8 result = do_subb_8(val, data8); wdat8(address, result); cy(6); }
void xa_cpu::cmp_byte_indrdoff8_data8(u8 rd, u8 offset8, u8 data8) { u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; u8 val = rdat8(address); do_sub_8(val, data8); cy(6); }
void xa_cpu::and_byte_indrdoff8_data8(u8 rd, u8 offset8, u8 data8) { u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; u8 val = rdat8(address); u8 result = do_and_8(val, data8); wdat8(address, result); cy(6); }
void xa_cpu::or_byte_indrdoff8_data8(u8 rd, u8 offset8, u8 data8)  { u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; u8 val = rdat8(address); u8 result = do_or_8(val, data8); wdat8(address, result); cy(6);  }
void xa_cpu::xor_byte_indrdoff8_data8(u8 rd, u8 offset8, u8 data8) { u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; u8 val = rdat8(address); u8 result = do_xor_8(val, data8); wdat8(address, result); cy(6); }
void xa_cpu::mov_byte_indrdoff8_data8(u8 rd, u8 offset8, u8 data8) { u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; do_nz_flags_8(data8); wdat8(address, data8); cy(5); }

// ------------------------------------------
// ALUOP.b [Rd+offs16], data8
// ADD [Rd+offset16], #data8   Add 8-bit imm data to reg-ind w/ 16-bit offs                            5 6         1001 0101  0ddd 0000  oooo oooo  oooo oooo  iiii iiii
// ADDC [Rd+offset16], #data8  Add 8-bit imm data to reg-ind w/ 16-bit offs and carry                  5 6         1001 0101  0ddd 0001  oooo oooo  oooo oooo  iiii iiii
// SUB [Rd+offset16], #data8   Subtract 8-bit imm data to reg-ind w/ 16-bit offs                       5 6         1001 0101  0ddd 0010  oooo oooo  oooo oooo  iiii iiii
// SUBB [Rd+offset16], #data8  Subtract w/ borrow 8-bit imm data to reg-ind w/ 16-bit offs             5 6         1001 0101  0ddd 0011  oooo oooo  oooo oooo  iiii iiii
// CMP [Rd+offset16], #data8   Compare 8-bit imm data to reg-ind w/ 16-bit offs                        5 6         1001 0101  0ddd 0100  oooo oooo  oooo oooo  iiii iiii
// AND [Rd+offset16], #data8   Logical AND 8-bit imm data to reg-ind w/ 16-bit offs                    5 6         1001 0101  0ddd 0101  oooo oooo  oooo oooo  iiii iiii
// OR [Rd+offset16], #data8    Logical OR 8-bit imm data to reg-ind w/ 16-bit offs                     5 6         1001 0101  0ddd 0110  oooo oooo  oooo oooo  iiii iiii
// XOR [Rd+offset16], #data8   Logical XOR 8-bit imm data to reg-ind w/ 16-bit offs                    5 6         1001 0101  0ddd 0111  oooo oooo  oooo oooo  iiii iiii
// MOV [Rd+offset16], #data8   Move 8-bit imm data to reg-ind w/ 16-bit offs                           5 5         1001 0101  0ddd 1000  oooo oooo  oooo oooo  iiii iiii
void xa_cpu::aluop_byte_rdoff16_data8(int alu_op, u8 rd, u16 offset16, u8 data8)
{
	switch (alu_op)
	{
	case 0x0: add_byte_indrdoff16_data8(rd, offset16, data8); break;
	case 0x1: addc_byte_indrdoff16_data8(rd, offset16, data8); break;
	case 0x2: sub_byte_indrdoff16_data8(rd, offset16, data8); break;
	case 0x3: subb_byte_indrdoff16_data8(rd, offset16, data8); break;
	case 0x4: cmp_byte_indrdoff16_data8(rd, offset16, data8); break;
	case 0x5: and_byte_indrdoff16_data8(rd, offset16, data8); break;
	case 0x6: or_byte_indrdoff16_data8(rd, offset16, data8); break;
	case 0x7: xor_byte_indrdoff16_data8(rd, offset16, data8); break;
	case 0x8: mov_byte_indrdoff16_data8(rd, offset16, data8); break;
	default: logerror("UNK_ALUOP.b [%s+#$%04x], #$%02d", m_regnames16[rd], offset16, data8); do_nop(); break;
	}
}
void xa_cpu::add_byte_indrdoff16_data8(u8 rd, u16 offset16, u8 data8) { u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; u8 val = rdat8(address); u8 result = do_add_8(val, data8); wdat8(address, result); cy(6); }
void xa_cpu::addc_byte_indrdoff16_data8(u8 rd, u16 offset16, u8 data8){ u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; u8 val = rdat8(address); u8 result = do_addc_8(val, data8); wdat8(address, result); cy(6); }
void xa_cpu::sub_byte_indrdoff16_data8(u8 rd, u16 offset16, u8 data8) { u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; u8 val = rdat8(address); u8 result = do_sub_8(val, data8); wdat8(address, result); cy(6); }
void xa_cpu::subb_byte_indrdoff16_data8(u8 rd, u16 offset16, u8 data8){ u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; u8 val = rdat8(address); u8 result = do_subb_8(val, data8); wdat8(address, result); cy(6); }
void xa_cpu::cmp_byte_indrdoff16_data8(u8 rd, u16 offset16, u8 data8) { u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; u8 val = rdat8(address); do_sub_8(val, data8); cy(6); }
void xa_cpu::and_byte_indrdoff16_data8(u8 rd, u16 offset16, u8 data8) { u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; u8 val = rdat8(address); u8 result = do_and_8(val, data8); wdat8(address, result); cy(6); }
void xa_cpu::or_byte_indrdoff16_data8(u8 rd, u16 offset16, u8 data8)  { u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; u8 val = rdat8(address); u8 result = do_or_8(val, data8); wdat8(address, result); cy(6);  }
void xa_cpu::xor_byte_indrdoff16_data8(u8 rd, u16 offset16, u8 data8) { u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; u8 val = rdat8(address); u8 result = do_xor_8(val, data8); wdat8(address, result); cy(6); }
void xa_cpu::mov_byte_indrdoff16_data8(u8 rd, u16 offset16, u8 data8) { u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; do_nz_flags_8(data8); wdat8(address, data8); cy(5); }

// ------------------------------------------
// ADD direct, #data8          Add 8-bit imm data to mem                                               4 4         1001 0110  0DDD 0000  DDDD DDDD  iiii iiii
// ADDC direct, #data8         Add 8-bit imm data to mem w/ carry                                      4 4         1001 0110  0DDD 0001  DDDD DDDD  iiii iiii
// SUB direct, #data8          Subtract 8-bit imm data to mem                                          4 4         1001 0110  0DDD 0010  DDDD DDDD  iiii iiii
// SUBB direct, #data8         Subtract w/ borrow 8-bit imm data to mem                                4 4         1001 0110  0DDD 0011  DDDD DDDD  iiii iiii
// CMP direct, #data8          Compare 8-bit imm data to mem                                           4 4         1001 0110  0DDD 0100  DDDD DDDD  iiii iiii
// AND direct, #data8          Logical AND 8-bit imm data to mem                                       4 4         1001 0110  0DDD 0101  DDDD DDDD  iiii iiii
// OR direct, #data8           Logical OR 8-bit imm data to mem                                        4 4         1001 0110  0DDD 0110  DDDD DDDD  iiii iiii
// XOR direct, #data8          Logical XOR 8-bit imm data to mem                                       4 4         1001 0110  0DDD 0111  DDDD DDDD  iiii iiii
// MOV direct, #data8          Move 8-bit imm data to mem                                              4 3         1001 0110  0DDD 1000  DDDD DDDD  iiii iiii
void xa_cpu::aluop_byte_direct_data8(int alu_op, u16 direct, u8 data8)
{
	switch (alu_op)
	{
	case 0x0: add_byte_direct_data8(direct, data8); break;
	case 0x1: addc_byte_direct_data8(direct, data8); break;
	case 0x2: sub_byte_direct_data8(direct, data8); break;
	case 0x3: subb_byte_direct_data8(direct, data8); break;
	case 0x4: cmp_byte_direct_data8(direct, data8); break;
	case 0x5: and_byte_direct_data8(direct, data8); break;
	case 0x6: or_byte_direct_data8(direct, data8); break;
	case 0x7: xor_byte_direct_data8(direct, data8); break;
	case 0x8: mov_byte_direct_data8(direct, data8); break;
	default: logerror("UNK_ALUOP.b %s, #$%02x (DIRECT, DATA8)", get_directtext(direct), data8); do_nop(); break;
	}
}
void xa_cpu::add_byte_direct_data8(u16 direct, u8 data8) { u8 val = read_direct8(direct); u8 result = do_add_8(val, data8); write_direct8(direct, result); cy(4); }
void xa_cpu::addc_byte_direct_data8(u16 direct, u8 data8){ u8 val = read_direct8(direct); u8 result = do_addc_8(val, data8); write_direct8(direct, result); cy(4);}
void xa_cpu::sub_byte_direct_data8(u16 direct, u8 data8) { u8 val = read_direct8(direct); u8 result = do_sub_8(val, data8); write_direct8(direct, result); cy(4); }
void xa_cpu::subb_byte_direct_data8(u16 direct, u8 data8){ u8 val = read_direct8(direct); u8 result = do_subb_8(val, data8); write_direct8(direct, result); cy(4);}
void xa_cpu::cmp_byte_direct_data8(u16 direct, u8 data8) { u8 val = read_direct8(direct); do_sub_8(val, data8); cy(4); }
void xa_cpu::and_byte_direct_data8(u16 direct, u8 data8) { u8 val = read_direct8(direct); u8 result = do_and_8(val, data8); write_direct8(direct, result); cy(4); }
void xa_cpu::or_byte_direct_data8(u16 direct, u8 data8)  { u8 val = read_direct8(direct); u8 result = do_or_8(val, data8); write_direct8(direct, result); cy(4); }
void xa_cpu::xor_byte_direct_data8(u16 direct, u8 data8) { u8 val = read_direct8(direct); u8 result = do_xor_8(val, data8); write_direct8(direct, result); cy(4); }
void xa_cpu::mov_byte_direct_data8(u16 direct, u8 data8) { u8 result = data8; do_nz_flags_8(result); write_direct8(direct, result); cy(3); }

// -----------------------------------------------
// ALUOP.w Rd, data16
// ADD Rd, #data16             Add 16-bit imm data to reg                                              4 3         1001 1001  dddd 0000  iiii iiii  iiii iiii
// ADDC Rd, #data16            Add 16-bit imm data to reg w/ carry                                     4 3         1001 1001  dddd 0001  iiii iiii  iiii iiii
// SUB Rd, #data16             Subtract 16-bit imm data to reg                                         4 3         1001 1001  dddd 0010  iiii iiii  iiii iiii
// SUBB Rd, #data16            Subtract w/ borrow 16-bit imm data to reg                               4 3         1001 1001  dddd 0011  iiii iiii  iiii iiii
// CMP Rd, #data16             Compare 16-bit imm data to reg                                          4 3         1001 1001  dddd 0100  iiii iiii  iiii iiii
// AND Rd, #data16             Logical AND 16-bit imm data to reg                                      4 3         1001 1001  dddd 0101  iiii iiii  iiii iiii
// OR Rd, #data16              Logical OR 16-bit imm data to reg                                       4 3         1001 1001  dddd 0110  iiii iiii  iiii iiii
// XOR Rd, #data16             Logical XOR 16-bit imm data to reg                                      4 3         1001 1001  dddd 0111  iiii iiii  iiii iiii
// MOV Rd, #data16             Move 16-bit imm data to reg                                             4 3         1001 1001  dddd 1000  iiii iiii  iiii iiii
void xa_cpu::aluop_byte_rd_data16(int alu_op, u8 rd, u16 data16)
{
	switch (alu_op)
	{
	case 0x0: add_word_rd_data16(rd, data16); break;
	case 0x1: addc_word_rd_data16(rd, data16); break;
	case 0x2: sub_word_rd_data16(rd, data16); break;
	case 0x3: subb_word_rd_data16(rd, data16); break;
	case 0x4: cmp_word_rd_data16(rd, data16); break;
	case 0x5: and_word_rd_data16(rd, data16); break;
	case 0x6: or_word_rd_data16(rd, data16); break;
	case 0x7: xor_word_rd_data16(rd, data16); break;
	case 0x8: mov_word_rd_data16(rd, data16); break;
	default: logerror("UNK_ALUOP.w %s, #$%04x (RD, DATA16)", m_regnames16[rd], data16); do_nop(); break;
	}
}
void xa_cpu::add_word_rd_data16(u8 rd, u16 data16) { u16 rdval = gr16(rd); u16 result = do_add_16(rdval, data16); sr16(rd, result); cy(3); }
void xa_cpu::addc_word_rd_data16(u8 rd, u16 data16){ u16 rdval = gr16(rd); u16 result = do_addc_16(rdval, data16); sr16(rd, result); cy(3); }
void xa_cpu::sub_word_rd_data16(u8 rd, u16 data16) { u16 rdval = gr16(rd); u16 result = do_sub_16(rdval, data16); sr16(rd, result); cy(3); }
void xa_cpu::subb_word_rd_data16(u8 rd, u16 data16){ u16 rdval = gr16(rd); u16 result = do_subb_16(rdval, data16); sr16(rd, result); cy(3); }
void xa_cpu::cmp_word_rd_data16(u8 rd, u16 data16) { u16 rdval = gr16(rd); do_sub_16(rdval, data16); cy(3); }
void xa_cpu::and_word_rd_data16(u8 rd, u16 data16) { u16 rdval = gr16(rd); u16 result = do_and_16(rdval, data16); sr16(rd, result); cy(3); }
void xa_cpu::or_word_rd_data16(u8 rd, u16 data16)  { u16 rdval = gr16(rd); u16 result = do_or_16(rdval, data16); sr16(rd, result); cy(3); }
void xa_cpu::xor_word_rd_data16(u8 rd, u16 data16) { u16 rdval = gr16(rd); u16 result = do_xor_16(rdval, data16); sr16(rd, result); cy(3); }
void xa_cpu::mov_word_rd_data16(u8 rd, u16 data16) { u16 result = data16; do_nz_flags_16(result); sr16(rd, result); cy(3); }

// -----------------------------------------------
// ALUOP.w [Rd], data16
// ADD [Rd], #data16           Add 16-bit imm data to reg-ind                                          4 4         1001 1010  0ddd 0000  iiii iiii  iiii iiii
// ADDC [Rd], #data16          Add 16-bit imm data to reg-ind w/ carry                                 4 4         1001 1010  0ddd 0001  iiii iiii  iiii iiii
// SUB [Rd], #data16           Subtract 16-bit imm data to reg-ind                                     4 4         1001 1010  0ddd 0010  iiii iiii  iiii iiii
// SUBB [Rd], #data16          Subtract w/ borrow 16-bit imm data to reg-ind                           4 4         1001 1010  0ddd 0011  iiii iiii  iiii iiii
// CMP [Rd], #data16           Compare 16-bit imm data to reg-ind                                      4 4         1001 1010  0ddd 0100  iiii iiii  iiii iiii
// AND [Rd], #data16           Logical AND 16-bit imm data to reg-ind                                  4 4         1001 1010  0ddd 0101  iiii iiii  iiii iiii
// OR [Rd], #data16            Logical OR 16-bit imm data to reg-ind                                   4 4         1001 1010  0ddd 0110  iiii iiii  iiii iiii
// XOR [Rd], #data16           Logical XOR 16-bit imm data to reg-ind                                  4 4         1001 1010  0ddd 0111  iiii iiii  iiii iiii
// MOV [Rd], #data16           Move 16-bit imm data to reg-ind                                         4 3         1001 1010  0ddd 1000  iiii iiii  iiii iiii
void xa_cpu::aluop_byte_indrd_data16(int alu_op, u8 rd, u16 data16)
{
	switch (alu_op)
	{
	case 0x0: add_word_indrd_data16(rd, data16); break;
	case 0x1: addc_word_indrd_data16(rd, data16); break;
	case 0x2: sub_word_indrd_data16(rd, data16); break;
	case 0x3: subb_word_indrd_data16(rd, data16); break;
	case 0x4: cmp_word_indrd_data16(rd, data16); break;
	case 0x5: and_word_indrd_data16(rd, data16); break;
	case 0x6: or_word_indrd_data16(rd, data16); break;
	case 0x7: xor_word_indrd_data16(rd, data16); break;
	case 0x8: mov_word_indrd_data16(rd, data16); break;
	default: logerror("UNK_ALUOP.w [%s], #$%04x", m_regnames16[rd], data16); do_nop(); break;
	}
}
void xa_cpu::add_word_indrd_data16(u8 rd, u16 data16) { u16 address = get_addr(rd); u16 val = rdat16(address); u16 result = do_add_16(val, data16); wdat16(address, result); cy(4); }
void xa_cpu::addc_word_indrd_data16(u8 rd, u16 data16){ u16 address = get_addr(rd); u16 val = rdat16(address); u16 result = do_addc_16(val, data16); wdat16(address, result); cy(4); }
void xa_cpu::sub_word_indrd_data16(u8 rd, u16 data16) { u16 address = get_addr(rd); u16 val = rdat16(address); u16 result = do_sub_16(val, data16); wdat16(address, result); cy(4); }
void xa_cpu::subb_word_indrd_data16(u8 rd, u16 data16){ u16 address = get_addr(rd); u16 val = rdat16(address); u16 result = do_subb_16(val, data16); wdat16(address, result); cy(4); }
void xa_cpu::cmp_word_indrd_data16(u8 rd, u16 data16) { u16 address = get_addr(rd); u16 val = rdat16(address); do_sub_16(val, data16); cy(4); }
void xa_cpu::and_word_indrd_data16(u8 rd, u16 data16) { u16 address = get_addr(rd); u16 val = rdat16(address); u16 result = do_and_16(val, data16); wdat16(address, result); cy(4); }
void xa_cpu::or_word_indrd_data16(u8 rd, u16 data16)  { u16 address = get_addr(rd); u16 val = rdat16(address); u16 result = do_or_16(val, data16); wdat16(address, result); cy(4); }
void xa_cpu::xor_word_indrd_data16(u8 rd, u16 data16) { u16 address = get_addr(rd); u16 val = rdat16(address); u16 result = do_xor_16(val, data16); wdat16(address, result); cy(4); }
void xa_cpu::mov_word_indrd_data16(u8 rd, u16 data16) { u16 address = get_addr(rd); do_nz_flags_16(data16); wdat16(address, data16); cy(3); }

// ALUOP.w [Rd+], data16
// ADD [Rd+], #data16          Add 16-bit imm data to reg-ind w/ autoinc                               4 5         1001 1011  0ddd 0000  iiii iiii  iiii iiii
// ADDC [Rd+], #data16         Add 16-bit imm data to reg-ind and autoinc w/ carry                     4 5         1001 1011  0ddd 0001  iiii iiii  iiii iiii
// SUB [Rd+], #data16          Subtract 16-bit imm data to reg-ind w/ autoinc                          4 5         1001 1011  0ddd 0010  iiii iiii  iiii iiii
// SUBB [Rd+], #data16         Subtract w/ borrow 16-bit imm data to reg-ind w/ autoinc                4 5         1001 1011  0ddd 0011  iiii iiii  iiii iiii
// CMP [Rd+], #data16          Compare 16-bit imm data to reg-ind w/ autoinc                           4 5         1001 1011  0ddd 0100  iiii iiii  iiii iiii
// AND [Rd+], #data16          Logical AND 16-bit imm data to reg-ind and autoinc                      4 5         1001 1011  0ddd 0101  iiii iiii  iiii iiii
// OR [Rd+], #data16           Logical OR 16-bit imm data to reg-ind w/ autoinc                        4 5         1001 1011  0ddd 0110  iiii iiii  iiii iiii
// XOR [Rd+], #data16          Logical XOR 16-bit imm data to reg-ind w/ autoinc                       4 5         1001 1011  0ddd 0111  iiii iiii  iiii iiii
// MOV [Rd+], #data16          Move 16-bit imm data to reg-ind w/ autoinc                              4 4         1001 1011  0ddd 1000  iiii iiii  iiii iiii
void xa_cpu::aluop_byte_indrdinc_data16(int alu_op, u8 rd, u16 data16)
{
	switch (alu_op)
	{
	case 0x0: add_word_indrdinc_data16(rd, data16); break;
	case 0x1: addc_word_indrdinc_data16(rd, data16); break;
	case 0x2: sub_word_indrdinc_data16(rd, data16); break;
	case 0x3: subb_word_indrdinc_data16(rd, data16); break;
	case 0x4: cmp_word_indrdinc_data16(rd, data16); break;
	case 0x5: and_word_indrdinc_data16(rd, data16); break;
	case 0x6: or_word_indrdinc_data16(rd, data16); break;
	case 0x7: xor_word_indrdinc_data16(rd, data16); break;
	case 0x8: mov_word_indrdinc_data16(rd, data16); break;
	default: logerror("UNK_ALUOP.w [%s+], #$%04x", m_regnames16[rd], data16); do_nop(); break;
	}
}
void xa_cpu::add_word_indrdinc_data16(u8 rd, u16 data16) { fatalerror( "ADD.w [%s+], #$%04x ([RD+], DATA16)", m_regnames16[rd], data16); }
void xa_cpu::addc_word_indrdinc_data16(u8 rd, u16 data16){ fatalerror( "ADDC.w [%s+], #$%04x ([RD+], DATA16)", m_regnames16[rd], data16); }
void xa_cpu::sub_word_indrdinc_data16(u8 rd, u16 data16) { fatalerror( "SUB.w [%s+], #$%04x ([RD+], DATA16)", m_regnames16[rd], data16); }
void xa_cpu::subb_word_indrdinc_data16(u8 rd, u16 data16){ fatalerror( "SUBB.w [%s+], #$%04x ([RD+], DATA16)", m_regnames16[rd], data16); }
void xa_cpu::cmp_word_indrdinc_data16(u8 rd, u16 data16) { fatalerror( "CMP.w [%s+], #$%04x ([RD+], DATA16)", m_regnames16[rd], data16); }
void xa_cpu::and_word_indrdinc_data16(u8 rd, u16 data16) { fatalerror( "AND.w [%s+], #$%04x ([RD+], DATA16)", m_regnames16[rd], data16); }
void xa_cpu::or_word_indrdinc_data16(u8 rd, u16 data16)  { fatalerror( "OR.w [%s+], #$%04x ([RD+], DATA16)", m_regnames16[rd], data16); }
void xa_cpu::xor_word_indrdinc_data16(u8 rd, u16 data16) { fatalerror( "XOR.w [%s+], #$%04x ([RD+], DATA16)", m_regnames16[rd], data16); }
void xa_cpu::mov_word_indrdinc_data16(u8 rd, u16 data16) { fatalerror( "MOV.w [%s+], #$%04x ([RD+], DATA16)", m_regnames16[rd], data16); }

// ALPOP.w [Rd+offs8], data16
// ADD [Rd+offset8], #data16   Add 16-bit imm data to reg-ind w/ 8-bit offs                            5 6         1001 1100  0ddd 0000  oooo oooo  iiii iiii  iiii iiii
// ADDC [Rd+offset8], #data16  Add 16-bit imm data to reg-ind w/ 8-bit offs and carry                  5 6         1001 1100  0ddd 0001  oooo oooo  iiii iiii  iiii iiii
// SUB [Rd+offset8], #data16   Subtract 16-bit imm data to reg-ind w/ 8-bit offs                       5 6         1001 1100  0ddd 0010  oooo oooo  iiii iiii  iiii iiii
// SUBB [Rd+offset8], #data16  Subtract w/ borrow 16-bit imm data to reg-ind w/ 8-bit offs             5 6         1001 1100  0ddd 0011  oooo oooo  iiii iiii  iiii iiii
// CMP [Rd+offset8], #data16   Compare 16-bit imm data to reg-ind w/ 8-bit offs                        5 6         1001 1100  0ddd 0100  oooo oooo  iiii iiii  iiii iiii
// AND [Rd+offset8], #data16   Logical AND 16-bit imm data to reg-ind w/ 8-bit offs                    5 6         1001 1100  0ddd 0101  oooo oooo  iiii iiii  iiii iiii
// OR [Rd+offset8], #data16    Logical OR 16-bit imm data to reg-ind w/ 8-bit offs                     5 6         1001 1100  0ddd 0110  oooo oooo  iiii iiii  iiii iiii
// XOR [Rd+offset8], #data16   Logical XOR 16-bit imm data to reg-ind w/ 8-bit offs                    5 6         1001 1100  0ddd 0111  oooo oooo  iiii iiii  iiii iiii
// MOV [Rd+offset8], #data16   Move 16-bit imm data to reg-ind w/ 8-bit offs                           5 5         1001 1100  0ddd 1000  oooo oooo  iiii iiii  iiii iiii
void xa_cpu::aluop_byte_rdoff8_data16(int alu_op, u8 rd, u8 offset8, u16 data16)
{
	switch (alu_op)
	{
	case 0x0: add_word_indrdoff8_data16(rd, offset8, data16); break;
	case 0x1: addc_word_indrdoff8_data16(rd, offset8, data16); break;
	case 0x2: sub_word_indrdoff8_data16(rd, offset8, data16); break;
	case 0x3: subb_word_indrdoff8_data16(rd, offset8, data16); break;
	case 0x4: cmp_word_indrdoff8_data16(rd, offset8, data16); break;
	case 0x5: and_word_indrdoff8_data16(rd, offset8, data16); break;
	case 0x6: or_word_indrdoff8_data16(rd, offset8, data16); break;
	case 0x7: xor_word_indrdoff8_data16(rd, offset8, data16); break;
	case 0x8: mov_word_indrdoff8_data16(rd, offset8, data16); break;
	default: logerror("UNK_ALUOP.w [%s+#$%02x], #$%04x", m_regnames16[rd], offset8, data16); do_nop(); break;
	}
}


void xa_cpu::add_word_indrdoff8_data16(u8 rd, u8 offset8, u16 data16) { u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; u16 val = rdat16(address); u16 result = do_add_16(val, data16); wdat16(address, result); cy(6); }
void xa_cpu::addc_word_indrdoff8_data16(u8 rd, u8 offset8, u16 data16){ u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; u16 val = rdat16(address); u16 result = do_addc_16(val, data16); wdat16(address, result); cy(6); }
void xa_cpu::sub_word_indrdoff8_data16(u8 rd, u8 offset8, u16 data16) { u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; u16 val = rdat16(address); u16 result = do_sub_16(val, data16); wdat16(address, result); cy(6); }
void xa_cpu::subb_word_indrdoff8_data16(u8 rd, u8 offset8, u16 data16){ u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; u16 val = rdat16(address); u16 result = do_subb_16(val, data16); wdat16(address, result); cy(6); }
void xa_cpu::cmp_word_indrdoff8_data16(u8 rd, u8 offset8, u16 data16) { u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; u16 val = rdat16(address); do_sub_16(val, data16); cy(6); }
void xa_cpu::and_word_indrdoff8_data16(u8 rd, u8 offset8, u16 data16) { u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; u16 val = rdat16(address); u16 result = do_and_16(val, data16); wdat16(address, result); cy(6); }
void xa_cpu::or_word_indrdoff8_data16(u8 rd, u8 offset8, u16 data16)  { u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; u16 val = rdat16(address); u16 result = do_or_16(val, data16); wdat16(address, result); cy(6); }
void xa_cpu::xor_word_indrdoff8_data16(u8 rd, u8 offset8, u16 data16) { u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; u16 val = rdat16(address); u16 result = do_xor_16(val, data16); wdat16(address, result); cy(6); }
void xa_cpu::mov_word_indrdoff8_data16(u8 rd, u8 offset8, u16 data16) { u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; do_nz_flags_16(data16); wdat16(address, data16); cy(5);  }

// ALUOP.w [Rd+offs16], data16
// ADD [Rd+offset16], #data16  Add 16-bit imm data to reg-ind w/ 16-bit offs                           6 6         1001 1101  0ddd 0000  oooo oooo  oooo oooo  iiii iiii  iiii iiii
// ADDC [Rd+offset16], #data16 Add 16-bit imm data to reg-ind w/ 16-bit offs and carry                 6 6         1001 1101  0ddd 0001  oooo oooo  oooo oooo  iiii iiii  iiii iiii
// SUB [Rd+offset16], #data16  Subtract 16-bit imm data to reg-ind w/ 16-bit offs                      6 6         1001 1101  0ddd 0010  oooo oooo  oooo oooo  iiii iiii  iiii iiii
// SUBB [Rd+offset16], #data16 Subtract w/ borrow 16-bit imm data to reg-ind w/ 16-bit offs            6 6         1001 1101  0ddd 0011  oooo oooo  oooo oooo  iiii iiii  iiii iiii
// CMP [Rd+offset16], #data16  Compare 16-bit imm data to reg-ind w/ 16-bit offs                       6 6         1001 1101  0ddd 0100  oooo oooo  oooo oooo  iiii iiii  iiii iiii
// AND [Rd+offset16], #data16  Logical AND 16-bit imm data to reg-ind w/ 16-bit offs                   6 6         1001 1101  0ddd 0101  oooo oooo  oooo oooo  iiii iiii  iiii iiii
// OR [Rd+offset16], #data16   Logical OR 16-bit imm data to reg-ind w/ 16-bit offs                    6 6         1001 1101  0ddd 0110  oooo oooo  oooo oooo  iiii iiii  iiii iiii
// XOR [Rd+offset16], #data16  Logical XOR 16-bit imm data to reg-ind w/ 16-bit offs                   6 6         1001 1101  0ddd 0111  oooo oooo  oooo oooo  iiii iiii  iiii iiii
// MOV [Rd+offset16], #data16  Move 16-bit imm data to reg-ind w/ 16-bit offs                          6 5         1001 1101  0ddd 1000  oooo oooo  oooo oooo  iiii iiii  iiii iiii
void xa_cpu::aluop_byte_rdoff16_data16(int alu_op, u8 rd, u16 offset16, u16 data16)
{
	switch (alu_op)
	{
	case 0x0: add_word_indrdoff16_data16(rd, offset16, data16); break;
	case 0x1: addc_word_indrdoff16_data16(rd, offset16, data16); break;
	case 0x2: sub_word_indrdoff16_data16(rd, offset16, data16); break;
	case 0x3: subb_word_indrdoff16_data16(rd, offset16, data16); break;
	case 0x4: cmp_word_indrdoff16_data16(rd, offset16, data16); break;
	case 0x5: and_word_indrdoff16_data16(rd, offset16, data16); break;
	case 0x6: or_word_indrdoff16_data16(rd, offset16, data16); break;
	case 0x7: xor_word_indrdoff16_data16(rd, offset16, data16); break;
	case 0x8: mov_word_indrdoff16_data16(rd, offset16, data16); break;
	default: logerror("UNK_ALUOP.w [%s+#$%04x], #$%04x", m_regnames16[rd], offset16, data16); do_nop(); break;
	}
}

void xa_cpu::add_word_indrdoff16_data16(u8 rd, u16 offset16, u16 data16) { u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; u16 val = rdat16(address); u16 result = do_add_16(val, data16); wdat16(address, result); cy(6); }
void xa_cpu::addc_word_indrdoff16_data16(u8 rd, u16 offset16, u16 data16){ u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; u16 val = rdat16(address); u16 result = do_addc_16(val, data16); wdat16(address, result); cy(6); }
void xa_cpu::sub_word_indrdoff16_data16(u8 rd, u16 offset16, u16 data16) { u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; u16 val = rdat16(address); u16 result = do_sub_16(val, data16); wdat16(address, result); cy(6); }
void xa_cpu::subb_word_indrdoff16_data16(u8 rd, u16 offset16, u16 data16){ u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; u16 val = rdat16(address); u16 result = do_subb_16(val, data16); wdat16(address, result); cy(6); }
void xa_cpu::cmp_word_indrdoff16_data16(u8 rd, u16 offset16, u16 data16) { u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; u16 val = rdat16(address); do_sub_16(val, data16); cy(6); }
void xa_cpu::and_word_indrdoff16_data16(u8 rd, u16 offset16, u16 data16) { u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; u16 val = rdat16(address); u16 result = do_and_16(val, data16); wdat16(address, result); cy(6); }
void xa_cpu::or_word_indrdoff16_data16(u8 rd, u16 offset16, u16 data16)  { u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; u16 val = rdat16(address); u16 result = do_or_16(val, data16); wdat16(address, result); cy(6); }
void xa_cpu::xor_word_indrdoff16_data16(u8 rd, u16 offset16, u16 data16) { u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; u16 val = rdat16(address); u16 result = do_xor_16(val, data16); wdat16(address, result); cy(6); }
void xa_cpu::mov_word_indrdoff16_data16(u8 rd, u16 offset16, u16 data16) { u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; do_nz_flags_16(data16); wdat16(address, data16); cy(5);  }

// ALUOP.w DIRECT, data16
// ADD direct, #data16         Add 16-bit imm data to mem                                              5 4         1001 1110  0DDD 0000  DDDD DDDD  iiii iiii  iiii iiii
// ADDC direct, #data16        Add 16-bit imm data to mem w/ carry                                     5 4         1001 1110  0DDD 0001  DDDD DDDD  iiii iiii  iiii iiii
// SUB direct, #data16         Subtract 16-bit imm data to mem                                         5 4         1001 1110  0DDD 0010  DDDD DDDD  iiii iiii  iiii iiii
// SUBB direct, #data16        Subtract w/ borrow 16-bit imm data to mem                               5 4         1001 1110  0DDD 0011  DDDD DDDD  iiii iiii  iiii iiii
// CMP direct, #data16         Compare 16-bit imm data to mem                                          5 4         1001 1110  0DDD 0100  DDDD DDDD  iiii iiii  iiii iiii
// AND direct, #data16         Logical AND 16-bit imm data to mem                                      5 4         1001 1110  0DDD 0101  DDDD DDDD  iiii iiii  iiii iiii
// OR direct, #data16          Logical OR 16-bit imm data to mem                                       5 4         1001 1110  0DDD 0110  DDDD DDDD  iiii iiii  iiii iiii
// XOR direct, #data16         Logical XOR 16-bit imm data to mem                                      5 4         1001 1110  0DDD 0111  DDDD DDDD  iiii iiii  iiii iiii
// MOV direct, #data16         Move 16-bit imm data to mem                                             5 3         1001 1110  0DDD 1000  DDDD DDDD  iiii iiii  iiii iiii
void xa_cpu::aluop_byte_direct_data16(int alu_op, u16 direct, u16 data16)
{
	switch (alu_op)
	{
	case 0x0: add_word_direct_data16(direct, data16); break;
	case 0x1: addc_word_direct_data16(direct, data16); break;
	case 0x2: sub_word_direct_data16(direct, data16); break;
	case 0x3: subb_word_direct_data16(direct, data16); break;
	case 0x4: cmp_word_direct_data16(direct, data16); break;
	case 0x5: and_word_direct_data16(direct, data16); break;
	case 0x6: or_word_direct_data16(direct, data16); break;
	case 0x7: xor_word_direct_data16(direct, data16); break;
	case 0x8: mov_word_direct_data16(direct, data16); break;
	default: logerror("UNK_ALUOP.w %s, #$%04x", get_directtext(direct), data16); do_nop(); break;
	}
}
void xa_cpu::add_word_direct_data16(u16 direct, u16 data16) { u16 val = read_direct16(direct); u16 result = do_add_16(val, data16); write_direct16(direct, result); cy(4); }
void xa_cpu::addc_word_direct_data16(u16 direct, u16 data16){ u16 val = read_direct16(direct); u16 result = do_addc_16(val, data16); write_direct16(direct, result); cy(4);}
void xa_cpu::sub_word_direct_data16(u16 direct, u16 data16) { u16 val = read_direct16(direct); u16 result = do_sub_16(val, data16); write_direct16(direct, result); cy(4); }
void xa_cpu::subb_word_direct_data16(u16 direct, u16 data16){ u16 val = read_direct16(direct); u16 result = do_subb_16(val, data16); write_direct16(direct, result); cy(4);}
void xa_cpu::cmp_word_direct_data16(u16 direct, u16 data16) { u16 val = read_direct16(direct); do_sub_16(val, data16); cy(4); }
void xa_cpu::and_word_direct_data16(u16 direct, u16 data16) { u16 val = read_direct16(direct); u16 result = do_and_16(val, data16); write_direct16(direct, result); cy(4); }
void xa_cpu::or_word_direct_data16(u16 direct, u16 data16)  { u16 val = read_direct16(direct); u16 result = do_or_16(val, data16); write_direct16(direct, result); cy(4); }
void xa_cpu::xor_word_direct_data16(u16 direct, u16 data16) { u16 val = read_direct16(direct); u16 result = do_xor_16(val, data16); write_direct16(direct, result); cy(4); }
void xa_cpu::mov_word_direct_data16(u16 direct, u16 data16) { u16 result = data16; do_nz_flags_16(result); write_direct16(direct, result); cy(3); }

// ------------------------------------------
// ------------------------------------------
// ------------------------------------------


// ALUOP.w Rd, Rs
// ALUOP.b Rd, Rs
// ADD Rd, Rs                  Add regs direct                                                         2 3         0000 S001  dddd ssss
// ADDC Rd, Rs                 Add regs direct w/ carry                                                2 3         0001 S001  dddd ssss
// SUB Rd, Rs                  Subtract regs direct                                                    2 3         0010 S001  dddd ssss
// SUBB Rd, Rs                 Subtract w/ borrow regs direct                                          2 3         0011 S001  dddd ssss
// CMP Rd, Rs                  Compare dest and src regs                                               2 3         0100 S001  dddd ssss
// AND Rd, Rs                  Logical AND regs direct                                                 2 3         0101 S001  dddd ssss
// OR Rd, Rs                   Logical OR regs                                                         2 3         0110 S001  dddd ssss
// XOR Rd, Rs                  Logical XOR regs                                                        2 3         0111 S001  dddd ssss
// MOV Rd, Rs                  Move reg to reg                                                         2 3         1000 S001  dddd ssss
void xa_cpu::aluop_word_rd_rs(int alu_op, u8 rd, u8 rs)
{
	switch (alu_op)
	{
	case 0x0: add_word_rd_rs(rd, rs); break;
	case 0x1: addc_word_rd_rs(rd, rs); break;
	case 0x2: sub_word_rd_rs(rd, rs); break;
	case 0x3: subb_word_rd_rs(rd, rs); break;
	case 0x4: cmp_word_rd_rs(rd, rs); break;
	case 0x5: and_word_rd_rs(rd, rs); break;
	case 0x6: or_word_rd_rs(rd, rs); break;
	case 0x7: xor_word_rd_rs(rd, rs); break;
	case 0x8: mov_word_rd_rs(rd, rs); break;
	default: fatalerror("UNK_ALUOP.w %s, %s", m_regnames16[rd], m_regnames16[rs]); // ALUOP.w Rd, Rs
	}
}
void xa_cpu::aluop_byte_rd_rs(int alu_op, u8 rd, u8 rs)
{
	switch (alu_op)
	{
	case 0x0: add_byte_rd_rs(rd, rs); break;
	case 0x1: addc_byte_rd_rs(rd, rs); break;
	case 0x2: sub_byte_rd_rs(rd, rs); break;
	case 0x3: subb_byte_rd_rs(rd, rs); break;
	case 0x4: cmp_byte_rd_rs(rd, rs); break;
	case 0x5: and_byte_rd_rs(rd, rs); break;
	case 0x6: or_byte_rd_rs(rd, rs); break;
	case 0x7: xor_byte_rd_rs(rd, rs); break;
	case 0x8: mov_byte_rd_rs(rd, rs); break;
	default: fatalerror("UNK_ALUOP.b %s, %s", m_regnames8[rd], m_regnames8[rs]); // ALUOP.b Rd, Rs
	}
}

void xa_cpu::add_word_rd_rs(u8 rd, u8 rs) { u16 rdval = gr16(rd); u16 rsval = gr16(rs); u16 result = do_add_16(rdval, rsval); sr16(rd, result); cy(3);}
void xa_cpu::addc_word_rd_rs(u8 rd, u8 rs){ u16 rdval = gr16(rd); u16 rsval = gr16(rs); u16 result = do_addc_16(rdval, rsval); sr16(rd, result); cy(3);}
void xa_cpu::sub_word_rd_rs(u8 rd, u8 rs) { u16 rdval = gr16(rd); u16 rsval = gr16(rs); u16 result = do_sub_16(rdval, rsval); sr16(rd, result); cy(3);}
void xa_cpu::subb_word_rd_rs(u8 rd, u8 rs){ u16 rdval = gr16(rd); u16 rsval = gr16(rs); u16 result = do_subb_16(rdval, rsval); sr16(rd, result); cy(3);}
void xa_cpu::cmp_word_rd_rs(u8 rd, u8 rs) { u16 rdval = gr16(rd); u16 rsval = gr16(rs); do_sub_16(rdval, rsval); cy(3); }
void xa_cpu::and_word_rd_rs(u8 rd, u8 rs) { u16 rdval = gr16(rd); u16 rsval = gr16(rs); u16 result = do_and_16(rdval, rsval); sr16(rd, result); cy(3); }
void xa_cpu::or_word_rd_rs(u8 rd, u8 rs) { u16 rdval = gr16(rd); u16 rsval = gr16(rs); u16 result = do_or_16(rdval, rsval); sr16(rd, result); cy(3); }
void xa_cpu::xor_word_rd_rs(u8 rd, u8 rs) { u16 rdval = gr16(rd); u16 rsval = gr16(rs); u16 result = do_xor_16(rdval, rsval); sr16(rd, result); cy(3);}
void xa_cpu::mov_word_rd_rs(u8 rd, u8 rs) { u16 val = gr16(rs);  do_nz_flags_16(val); sr16(rd, val); cy(3); }

void xa_cpu::add_byte_rd_rs(u8 rd, u8 rs) { u8 rdval = gr8(rd); u8 rsval = gr8(rs); u8 result = do_add_8(rdval, rsval); sr8(rd, result); cy(3); }
void xa_cpu::addc_byte_rd_rs(u8 rd, u8 rs){ u8 rdval = gr8(rd); u8 rsval = gr8(rs); u8 result = do_addc_8(rdval, rsval); sr8(rd, result); cy(3); }
void xa_cpu::sub_byte_rd_rs(u8 rd, u8 rs) { u8 rdval = gr8(rd); u8 rsval = gr8(rs); u8 result = do_sub_8(rdval, rsval); sr8(rd, result); cy(3); }
void xa_cpu::subb_byte_rd_rs(u8 rd, u8 rs){ u8 rdval = gr8(rd); u8 rsval = gr8(rs); u8 result = do_subb_8(rdval, rsval); sr8(rd, result); cy(3); }
void xa_cpu::cmp_byte_rd_rs(u8 rd, u8 rs) { u8 rdval = gr8(rd); u8 rsval = gr8(rs); do_sub_8(rdval, rsval); cy(3);}
void xa_cpu::and_byte_rd_rs(u8 rd, u8 rs) { u8 rdval = gr8(rd); u8 rsval = gr8(rs); u8 result = do_and_8(rdval, rsval); sr8(rd, result); cy(3); }
void xa_cpu::or_byte_rd_rs(u8 rd, u8 rs)  { u8 rdval = gr8(rd); u8 rsval = gr8(rs); u8 result = do_or_8(rdval, rsval); sr8(rd, result); cy(3); }
void xa_cpu::xor_byte_rd_rs(u8 rd, u8 rs) { u8 rdval = gr8(rd); u8 rsval = gr8(rs); u8 result = do_xor_8(rdval, rsval); sr8(rd, result); cy(3); }
void xa_cpu::mov_byte_rd_rs(u8 rd, u8 rs) { u8 val = gr8(rs);  do_nz_flags_8(val); sr8(rd, val); cy(3);}

// ALUOP.w Rd, [Rs]
// ALUOP.b Rd, [Rs]
// ADD Rd, [Rs]                Add reg-ind to reg                                                      2 4         0000 S010  dddd 0sss
// ADDC Rd, [Rs]               Add reg-ind to reg w/ carry                                             2 4         0001 S010  dddd 0sss
// SUB Rd, [Rs]                Subtract reg-ind to reg                                                 2 4         0010 S010  dddd 0sss
// SUBB Rd, [Rs]               Subtract w/ borrow reg-ind to reg                                       2 4         0011 S010  dddd 0sss
// CMP Rd, [Rs]                Compare reg-ind w/ reg                                                  2 4         0100 S010  dddd 0sss
// AND Rd, [Rs]                Logical AND reg-ind to reg                                              2 4         0101 S010  dddd 0sss
// OR Rd, [Rs]                 Logical OR reg-ind to reg                                               2 4         0110 S010  dddd 0sss
// XOR Rd, [Rs]                Logical XOR reg-ind to reg                                              2 4         0111 S010  dddd 0sss
// MOV Rd, [Rs]                Move reg-ind to reg                                                     2 3         1000 S010  dddd 0sss
void xa_cpu::aluop_word_rd_indrs(int alu_op, u8 rd, u8 rs)
{
	switch (alu_op)
	{
	case 0x0: add_word_rd_indrs(rd, rs); break;
	case 0x1: addc_word_rd_indrs(rd, rs); break;
	case 0x2: sub_word_rd_indrs(rd, rs); break;
	case 0x3: subb_word_rd_indrs(rd, rs); break;
	case 0x4: cmp_word_rd_indrs(rd, rs); break;
	case 0x5: and_word_rd_indrs(rd, rs); break;
	case 0x6: or_word_rd_indrs(rd, rs); break;
	case 0x7: xor_word_rd_indrs(rd, rs); break;
	case 0x8: mov_word_rd_indrs(rd, rs); break;
	default: fatalerror("UNK_ALUOP.w %s, [%s]", m_regnames16[rd], m_regnames16[rs]); // ALUOP.w Rd, [Rs]
	}
}
void xa_cpu::aluop_byte_rd_indrs(int alu_op, u8 rd, u8 rs)
{
	switch (alu_op)
	{
	case 0x0: add_byte_rd_indrs(rd, rs); break;
	case 0x1: addc_byte_rd_indrs(rd, rs); break;
	case 0x2: sub_byte_rd_indrs(rd, rs); break;
	case 0x3: subb_byte_rd_indrs(rd, rs); break;
	case 0x4: cmp_byte_rd_indrs(rd, rs); break;
	case 0x5: and_byte_rd_indrs(rd, rs); break;
	case 0x6: or_byte_rd_indrs(rd, rs); break;
	case 0x7: xor_byte_rd_indrs(rd, rs); break;
	case 0x8: mov_byte_rd_indrs(rd, rs); break;
	default: fatalerror("UNK_ALUOP.b %s, [%s]", m_regnames8[rd], m_regnames16[rs]); // ALUOP.b Rd, [Rs]
	}
}
void xa_cpu::add_word_rd_indrs(u8 rd, u8 rs) { fatalerror("ADD.w %s, [%s]", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::addc_word_rd_indrs(u8 rd, u8 rs){ fatalerror("ADDC.w %s, [%s]", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::sub_word_rd_indrs(u8 rd, u8 rs) { fatalerror("SUB.w %s, [%s]", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::subb_word_rd_indrs(u8 rd, u8 rs){ fatalerror("SUBB.w %s, [%s]", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::cmp_word_rd_indrs(u8 rd, u8 rs) { fatalerror("CMP.w %s, [%s]", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::and_word_rd_indrs(u8 rd, u8 rs) { fatalerror("AND.w %s, [%s]", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::or_word_rd_indrs(u8 rd, u8 rs)  { fatalerror("OR.w %s, [%s]", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::xor_word_rd_indrs(u8 rd, u8 rs) { fatalerror("XOR.w %s, [%s]", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::mov_word_rd_indrs(u8 rd, u8 rs) { u16 address = get_addr(rs); u16 val = rdat16(address); do_nz_flags_16(val); sr16(rd, val); cy(3); }

void xa_cpu::add_byte_rd_indrs(u8 rd, u8 rs) { fatalerror("ADD.b %s, [%s]", m_regnames8[rd], m_regnames16[rs]);}
void xa_cpu::addc_byte_rd_indrs(u8 rd, u8 rs){ fatalerror("ADDC.b %s, [%s]", m_regnames8[rd], m_regnames16[rs]);}
void xa_cpu::sub_byte_rd_indrs(u8 rd, u8 rs) { fatalerror("SUB.b %s, [%s]", m_regnames8[rd], m_regnames16[rs]);}
void xa_cpu::subb_byte_rd_indrs(u8 rd, u8 rs){ fatalerror("SUBB.b %s, [%s]", m_regnames8[rd], m_regnames16[rs]);}
void xa_cpu::cmp_byte_rd_indrs(u8 rd, u8 rs) { fatalerror("CMP.b %s, [%s]", m_regnames8[rd], m_regnames16[rs]);}
void xa_cpu::and_byte_rd_indrs(u8 rd, u8 rs) { fatalerror("AND.b %s, [%s]", m_regnames8[rd], m_regnames16[rs]);}
void xa_cpu::or_byte_rd_indrs(u8 rd, u8 rs)  { fatalerror("OR.b %s, [%s]", m_regnames8[rd], m_regnames16[rs]);}
void xa_cpu::xor_byte_rd_indrs(u8 rd, u8 rs) { fatalerror("XOR.b %s, [%s]", m_regnames8[rd], m_regnames16[rs]);}
void xa_cpu::mov_byte_rd_indrs(u8 rd, u8 rs) { u16 address = get_addr(rs); u8 val = rdat8(address); do_nz_flags_8(val); sr8(rd, val); cy(3); }


// ALUOP.w [Rd], Rs
// ALUOP.b [Rd], Rs
// ADD [Rd], Rs                Add reg to reg-ind                                                      2 4         0000 S010  ssss 1ddd
// ADDC [Rd], Rs               Add reg to reg-ind w/ carry                                             2 4         0001 S010  ssss 1ddd
// SUB [Rd], Rs                Subtract reg to reg-ind                                                 2 4         0010 S010  ssss 1ddd
// SUBB [Rd], Rs               Subtract w/ borrow reg to reg-ind                                       2 4         0011 S010  ssss 1ddd
// CMP [Rd], Rs                Compare reg w/ reg-ind                                                  2 4         0100 S010  ssss 1ddd
// AND [Rd], Rs                Logical AND reg to reg-ind                                              2 4         0101 S010  ssss 1ddd
// OR [Rd], Rs                 Logical OR reg to reg-ind                                               2 4         0110 S010  ssss 1ddd
// XOR [Rd], Rs                Logical XOR reg to reg-ind                                              2 4         0111 S010  ssss 1ddd
// MOV [Rd], Rs                Move reg to reg-ind                                                     2 3         1000 S010  ssss 1ddd
void xa_cpu::aluop_word_indrd_rs(int alu_op, u8 rd, u8 rs)
{
	switch (alu_op)
	{
	case 0x0: add_word_indrd_rs(rd, rs); break;
	case 0x1: addc_word_indrd_rs(rd, rs); break;
	case 0x2: sub_word_indrd_rs(rd, rs); break;
	case 0x3: subb_word_indrd_rs(rd, rs); break;
	case 0x4: cmp_word_indrd_rs(rd, rs); break;
	case 0x5: and_word_indrd_rs(rd, rs); break;
	case 0x6: or_word_indrd_rs(rd, rs); break;
	case 0x7: xor_word_indrd_rs(rd, rs); break;
	case 0x8: mov_word_indrd_rs(rd, rs); break;
	default: logerror("UNK_ALUOP.w [%s], %s", m_regnames16[rd], m_regnames16[rs]); do_nop(); break;
	}
}

void xa_cpu::aluop_byte_indrd_rs(int alu_op, u8 rd, u8 rs)
{
	switch (alu_op)
	{
	case 0x0: add_byte_indrd_rs(rd, rs); break;
	case 0x1: addc_byte_indrd_rs(rd, rs); break;
	case 0x2: sub_byte_indrd_rs(rd, rs); break;
	case 0x3: subb_byte_indrd_rs(rd, rs); break;
	case 0x4: cmp_byte_indrd_rs(rd, rs); break;
	case 0x5: and_byte_indrd_rs(rd, rs); break;
	case 0x6: or_byte_indrd_rs(rd, rs); break;
	case 0x7: xor_byte_indrd_rs(rd, rs); break;
	case 0x8: mov_byte_indrd_rs(rd, rs); break;
	default: logerror("UNK_ALUOP.b [%s], %s", m_regnames16[rd], m_regnames8[rs]); do_nop(); break;
	}
}

void xa_cpu::add_word_indrd_rs(u8 rd, u8 rs) { fatalerror("ADD.w [%s], %s", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::addc_word_indrd_rs(u8 rd, u8 rs){ fatalerror("ADDC.w [%s], %s", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::sub_word_indrd_rs(u8 rd, u8 rs) { fatalerror("SUB.w [%s], %s", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::subb_word_indrd_rs(u8 rd, u8 rs){ fatalerror("SUBB.w [%s], %s", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::cmp_word_indrd_rs(u8 rd, u8 rs) { fatalerror("CMP.w [%s], %s", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::and_word_indrd_rs(u8 rd, u8 rs) { fatalerror("AND.w [%s], %s", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::or_word_indrd_rs(u8 rd, u8 rs)  { fatalerror("OR.w [%s], %s", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::xor_word_indrd_rs(u8 rd, u8 rs) { fatalerror("XOR.w [%s], %s", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::mov_word_indrd_rs(u8 rd, u8 rs) { u16 val = gr16(rs); do_nz_flags_16(val); u16 addr = get_addr(rd); wdat16(addr, val); cy(3); }

void xa_cpu::add_byte_indrd_rs(u8 rd, u8 rs) { fatalerror("ADD.b [%s], %s", m_regnames16[rd], m_regnames8[rs]);}
void xa_cpu::addc_byte_indrd_rs(u8 rd, u8 rs){ fatalerror("ADDC.b [%s], %s", m_regnames16[rd], m_regnames8[rs]);}
void xa_cpu::sub_byte_indrd_rs(u8 rd, u8 rs) { fatalerror("SUB.b [%s], %s", m_regnames16[rd], m_regnames8[rs]);}
void xa_cpu::subb_byte_indrd_rs(u8 rd, u8 rs){ fatalerror("SUBB.b [%s], %s", m_regnames16[rd], m_regnames8[rs]);}
void xa_cpu::cmp_byte_indrd_rs(u8 rd, u8 rs) { fatalerror("CMP.b [%s], %s", m_regnames16[rd], m_regnames8[rs]);}
void xa_cpu::and_byte_indrd_rs(u8 rd, u8 rs) { fatalerror("AND.b [%s], %s", m_regnames16[rd], m_regnames8[rs]);}
void xa_cpu::or_byte_indrd_rs(u8 rd, u8 rs)  { fatalerror("OR.b [%s], %s", m_regnames16[rd], m_regnames8[rs]);}
void xa_cpu::xor_byte_indrd_rs(u8 rd, u8 rs) { fatalerror("XOR.b [%s], %s", m_regnames16[rd], m_regnames8[rs]);}
void xa_cpu::mov_byte_indrd_rs(u8 rd, u8 rs) { u8 val = gr8(rs); do_nz_flags_8(val); u16 addr = get_addr(rd); wdat8(addr, val); cy(3); }

// ALUOP.w Rd, [Rs+]
// ALUOP.b Rd, [Rs+]
// ADD Rd, [Rs+]               Add reg-ind w/ autoinc to reg                                           2 5         0000 S011  dddd 0sss
// ADDC Rd, [Rs+]              Add reg-ind w/ autoinc to reg w/ carry                                  2 5         0001 S011  dddd 0sss
// SUB Rd, [Rs+]               Subtract reg-ind w/ autoinc to reg                                      2 5         0010 S011  dddd 0sss
// SUBB Rd, [Rs+]              Subtract w/ borrow reg-ind w/ autoinc to reg                            2 5         0011 S011  dddd 0sss
// CMP Rd, [Rs+]               Compare autoinc reg-ind w/ reg                                          2 5         0100 S011  dddd 0sss
// AND Rd, [Rs+]               Logical AND reg-ind w/ autoinc to reg                                   2 5         0101 S011  dddd 0sss
// OR Rd, [Rs+]                Logical OR reg-ind w/ autoinc to reg                                    2 5         0110 S011  dddd 0sss
// XOR Rd, [Rs+]               Logical XOR reg-ind w/ autoinc to reg                                   2 5         0111 S011  dddd 0sss
// MOV Rd, [Rs+]               Move reg-ind w/ autoinc to reg                                          2 4         1000 S011  dddd 0sss
void xa_cpu::aluop_word_rd_indrsinc(int alu_op, u8 rd, u8 rs)
{
	switch (alu_op)
	{
	case 0x0: add_word_rd_indrsinc(rd, rs); break;
	case 0x1: addc_word_rd_indrsinc(rd, rs); break;
	case 0x2: sub_word_rd_indrsinc(rd, rs); break;
	case 0x3: subb_word_rd_indrsinc(rd, rs); break;
	case 0x4: cmp_word_rd_indrsinc(rd, rs); break;
	case 0x5: and_word_rd_indrsinc(rd, rs); break;
	case 0x6: or_word_rd_indrsinc(rd, rs); break;
	case 0x7: xor_word_rd_indrsinc(rd, rs); break;
	case 0x8: mov_word_rd_indrsinc(rd, rs); break;
	default: logerror("UNK_ALUOP.w %s, [%s+]", m_regnames16[rd], m_regnames16[rs]); do_nop(); break;
	}
}
void xa_cpu::aluop_byte_rd_indrsinc(int alu_op, u8 rd, u8 rs)
{
	switch (alu_op)
	{
	case 0x0: add_byte_rd_indrsinc(rd, rs); break;
	case 0x1: addc_byte_rd_indrsinc(rd, rs); break;
	case 0x2: sub_byte_rd_indrsinc(rd, rs); break;
	case 0x3: subb_byte_rd_indrsinc(rd, rs); break;
	case 0x4: cmp_byte_rd_indrsinc(rd, rs); break;
	case 0x5: and_byte_rd_indrsinc(rd, rs); break;
	case 0x6: or_byte_rd_indrsinc(rd, rs); break;
	case 0x7: xor_byte_rd_indrsinc(rd, rs); break;
	case 0x8: mov_byte_rd_indrsinc(rd, rs); break;
	default: logerror("UNK_ALUOP.b %s, [%s+]", m_regnames8[rd], m_regnames16[rs]); do_nop(); break;
	}
}
void xa_cpu::add_word_rd_indrsinc(u8 rd, u8 rs) { fatalerror("ADD.w %s, [%s+]", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::addc_word_rd_indrsinc(u8 rd, u8 rs){ fatalerror("ADDC.w %s, [%s+]", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::sub_word_rd_indrsinc(u8 rd, u8 rs) { fatalerror("SUB.w %s, [%s+]", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::subb_word_rd_indrsinc(u8 rd, u8 rs){ fatalerror("SUBB.w %s, [%s+]", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::cmp_word_rd_indrsinc(u8 rd, u8 rs) { fatalerror("CMP.w %s, [%s+]", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::and_word_rd_indrsinc(u8 rd, u8 rs) { fatalerror("AND.w %s, [%s+]", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::or_word_rd_indrsinc(u8 rd, u8 rs)  { fatalerror("OR.w %s, [%s+]", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::xor_word_rd_indrsinc(u8 rd, u8 rs) { fatalerror("XOR.w %s, [%s+]", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::mov_word_rd_indrsinc(u8 rd, u8 rs) { fatalerror("MOV.w %s, [%s+]", m_regnames16[rd], m_regnames16[rs]);}

void xa_cpu::add_byte_rd_indrsinc(u8 rd, u8 rs) { fatalerror("ADD.b %s, [%s+]", m_regnames8[rd], m_regnames16[rs]);}
void xa_cpu::addc_byte_rd_indrsinc(u8 rd, u8 rs){ fatalerror("ADDC.b %s, [%s+]", m_regnames8[rd], m_regnames16[rs]);}
void xa_cpu::sub_byte_rd_indrsinc(u8 rd, u8 rs) { fatalerror("SUB.b %s, [%s+]", m_regnames8[rd], m_regnames16[rs]);}
void xa_cpu::subb_byte_rd_indrsinc(u8 rd, u8 rs){ fatalerror("SUBB.b %s, [%s+]", m_regnames8[rd], m_regnames16[rs]);}
void xa_cpu::cmp_byte_rd_indrsinc(u8 rd, u8 rs) { fatalerror("CMP.b %s, [%s+]", m_regnames8[rd], m_regnames16[rs]);}
void xa_cpu::and_byte_rd_indrsinc(u8 rd, u8 rs) { fatalerror("AND.b %s, [%s+]", m_regnames8[rd], m_regnames16[rs]);}
void xa_cpu::or_byte_rd_indrsinc(u8 rd, u8 rs)  { fatalerror("OR.b %s, [%s+]", m_regnames8[rd], m_regnames16[rs]);}
void xa_cpu::xor_byte_rd_indrsinc(u8 rd, u8 rs) { fatalerror("XOR.b %s, [%s+]", m_regnames8[rd], m_regnames16[rs]);}
void xa_cpu::mov_byte_rd_indrsinc(u8 rd, u8 rs) { fatalerror("MOV.b %s, [%s+]", m_regnames8[rd], m_regnames16[rs]);}

// ALUOP.w [Rd+], Rs
// ALUOP.b [Rd+], Rs
// ADD [Rd+], Rs               Add reg-ind w/ autoinc to reg                                           2 5         0000 S011  ssss 1ddd
// ADDC [Rd+], Rs              Add reg-ind w/ autoinc to reg w/ carry                                  2 5         0001 S011  ssss 1ddd
// SUB [Rd+], Rs               Subtract reg-ind w/ autoinc to reg                                      2 5         0010 S011  ssss 1ddd
// SUBB [Rd+], Rs              Subtract w/ borrow reg-ind w/ autoinc to reg                            2 5         0011 S011  ssss 1ddd
// CMP [Rd+], Rs               Compare reg w/ autoinc reg-ind                                          2 5         0100 S011  ssss 1ddd
// AND [Rd+], Rs               Logical AND reg-ind w/ autoinc to reg                                   2 5         0101 S011  ssss 1ddd
// OR [Rd+], Rs                Logical OR reg-ind w/ autoinc to reg                                    2 5         0110 S011  ssss 1ddd
// XOR [Rd+], Rs               Logical XOR reg-ind w/ autoinc to reg                                   2 5         0111 S011  ssss 1ddd
// MOV [Rd+], Rs               Move reg-ind w/ autoinc to reg                                          2 4         1000 S011  ssss 1ddd
void xa_cpu::aluop_word_indrdinc_rs(int alu_op, u8 rd, u8 rs)
{
	switch (alu_op)
	{
	case 0x0: add_word_indrdinc_rs(rd, rs); break;
	case 0x1: addc_word_indrdinc_rs(rd, rs); break;
	case 0x2: sub_word_indrdinc_rs(rd, rs); break;
	case 0x3: subb_word_indrdinc_rs(rd, rs); break;
	case 0x4: cmp_word_indrdinc_rs(rd, rs); break;
	case 0x5: and_word_indrdinc_rs(rd, rs); break;
	case 0x6: or_word_indrdinc_rs(rd, rs); break;
	case 0x7: xor_word_indrdinc_rs(rd, rs); break;
	case 0x8: mov_word_indrdinc_rs(rd, rs); break;
	default: logerror("UNK_ALUOP.w [%s+], %s", m_regnames16[rd], m_regnames16[rs]); do_nop(); break;
	}
}

void xa_cpu::aluop_byte_indrdinc_rs(int alu_op, u8 rd, u8 rs)
{
	switch (alu_op)
	{
	case 0x0: add_byte_indrdinc_rs(rd, rs); break;
	case 0x1: addc_byte_indrdinc_rs(rd, rs); break;
	case 0x2: sub_byte_indrdinc_rs(rd, rs); break;
	case 0x3: subb_byte_indrdinc_rs(rd, rs); break;
	case 0x4: cmp_byte_indrdinc_rs(rd, rs); break;
	case 0x5: and_byte_indrdinc_rs(rd, rs); break;
	case 0x6: or_byte_indrdinc_rs(rd, rs); break;
	case 0x7: xor_byte_indrdinc_rs(rd, rs); break;
	case 0x8: mov_byte_indrdinc_rs(rd, rs); break;
	default: logerror("UNK_ALUOP.b [%s+], %s", m_regnames16[rd], m_regnames8[rs]); do_nop(); break;
	}
}

void xa_cpu::add_word_indrdinc_rs(u8 rd, u8 rs) { fatalerror("ADD.w [%s+], %s", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::addc_word_indrdinc_rs(u8 rd, u8 rs){ fatalerror("ADDC.w [%s+], %s", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::sub_word_indrdinc_rs(u8 rd, u8 rs) { fatalerror("SUB.w [%s+], %s", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::subb_word_indrdinc_rs(u8 rd, u8 rs){ fatalerror("SUBB.w [%s+], %s", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::cmp_word_indrdinc_rs(u8 rd, u8 rs) { fatalerror("CMP.w [%s+], %s", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::and_word_indrdinc_rs(u8 rd, u8 rs) { fatalerror("AND.w [%s+], %s", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::or_word_indrdinc_rs(u8 rd, u8 rs)  { fatalerror("OR.w [%s+], %s", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::xor_word_indrdinc_rs(u8 rd, u8 rs) { fatalerror("XOR.w [%s+], %s", m_regnames16[rd], m_regnames16[rs]);}
void xa_cpu::mov_word_indrdinc_rs(u8 rd, u8 rs) { fatalerror("MOV.w [%s+], %s", m_regnames16[rd], m_regnames16[rs]);}

void xa_cpu::add_byte_indrdinc_rs(u8 rd, u8 rs) { fatalerror("ADD.b [%s+], %s", m_regnames16[rd], m_regnames8[rs]);}
void xa_cpu::addc_byte_indrdinc_rs(u8 rd, u8 rs){ fatalerror("ADDC.b [%s+], %s", m_regnames16[rd], m_regnames8[rs]);}
void xa_cpu::sub_byte_indrdinc_rs(u8 rd, u8 rs) { fatalerror("SUB.b [%s+], %s", m_regnames16[rd], m_regnames8[rs]);}
void xa_cpu::subb_byte_indrdinc_rs(u8 rd, u8 rs){ fatalerror("SUBB.b [%s+], %s", m_regnames16[rd], m_regnames8[rs]);}
void xa_cpu::cmp_byte_indrdinc_rs(u8 rd, u8 rs) { fatalerror("CMP.b [%s+], %s", m_regnames16[rd], m_regnames8[rs]);}
void xa_cpu::and_byte_indrdinc_rs(u8 rd, u8 rs) { fatalerror("AND.b [%s+], %s", m_regnames16[rd], m_regnames8[rs]);}
void xa_cpu::or_byte_indrdinc_rs(u8 rd, u8 rs)  { fatalerror("OR.b [%s+], %s", m_regnames16[rd], m_regnames8[rs]);}
void xa_cpu::xor_byte_indrdinc_rs(u8 rd, u8 rs) { fatalerror("XOR.b [%s+], %s", m_regnames16[rd], m_regnames8[rs]);}
void xa_cpu::mov_byte_indrdinc_rs(u8 rd, u8 rs) { u16 address = get_addr(rd); u8 result = gr8(rs); wdat8(address, result); address++; do_nz_flags_8(result); set_addr(rd, address); cy(4); }


// ALUOP.w Rd, [Rs+off8]
// ALUOP.b Rd, [Rs+off8]
// ADD Rd, [Rs+offset8]        Add reg-ind w/ 8-bit offs to reg                                        3 6         0000 S100  dddd 0sss  oooo oooo
// ADDC Rd, [Rs+offset8]       Add reg-ind w/ 8-bit offs to reg w/ carry                               3 6         0001 S100  dddd 0sss  oooo oooo
// SUB Rd, [Rs+offset8]        Subtract reg-ind w/ 8-bit offs to reg                                   3 6         0010 S100  dddd 0sss  oooo oooo
// SUBB Rd, [Rs+offset8]       Subtract w/ borrow reg-ind w/ 8-bit offs to reg                         3 6         0011 S100  dddd 0sss  oooo oooo
// CMP Rd, [Rs+offset8]        Compare reg-ind w/ 8-bit offs w/ reg                                    3 6         0100 S100  dddd 0sss  oooo oooo
// AND Rd, [Rs+offset8]        Logical AND reg-ind w/ 8-bit offs to reg                                3 6         0101 S100  dddd 0sss  oooo oooo
// OR Rd, [Rs+offset8]         Logical OR reg-ind w/ 8-bit offs to reg                                 3 6         0110 S100  dddd 0sss  oooo oooo
// XOR Rd, [Rs+offset8]        Logical XOR reg-ind w/ 8-bit offs to reg                                3 6         0111 S100  dddd 0sss  oooo oooo
// MOV Rd, [Rs+offset8]        Move reg-ind w/ 8-bit offs to reg                                       3 5         1000 S100  dddd 0sss  oooo oooo
void xa_cpu::aluop_word_rd_rsoff8(int alu_op, u8 rd, u8 rs, u8 offset8)
{
	switch (alu_op)
	{
	case 0x0: add_word_rd_rsoff8(rd, rs, offset8); break;
	case 0x1: addc_word_rd_rsoff8(rd, rs, offset8); break;
	case 0x2: sub_word_rd_rsoff8(rd, rs, offset8); break;
	case 0x3: subb_word_rd_rsoff8(rd, rs, offset8); break;
	case 0x4: cmp_word_rd_rsoff8(rd, rs, offset8); break;
	case 0x5: and_word_rd_rsoff8(rd, rs, offset8); break;
	case 0x6: or_word_rd_rsoff8(rd, rs, offset8); break;
	case 0x7: xor_word_rd_rsoff8(rd, rs, offset8); break;
	case 0x8: mov_word_rd_rsoff8(rd, rs, offset8); break;
	default: logerror("UNK_ALUOP.w %s, [%s+#$%02x]", m_regnames16[rd], m_regnames16[rs], offset8); do_nop(); break;
	}
}
void xa_cpu::aluop_byte_rd_rsoff8(int alu_op, u8 rd, u8 rs, u8 offset8)
{
	switch (alu_op)
	{
	case 0x0: add_byte_rd_rsoff8(rd, rs, offset8); break;
	case 0x1: addc_byte_rd_rsoff8(rd, rs, offset8); break;
	case 0x2: sub_byte_rd_rsoff8(rd, rs, offset8); break;
	case 0x3: subb_byte_rd_rsoff8(rd, rs, offset8); break;
	case 0x4: cmp_byte_rd_rsoff8(rd, rs, offset8); break;
	case 0x5: and_byte_rd_rsoff8(rd, rs, offset8); break;
	case 0x6: or_byte_rd_rsoff8(rd, rs, offset8); break;
	case 0x7: xor_byte_rd_rsoff8(rd, rs, offset8); break;
	case 0x8: mov_byte_rd_rsoff8(rd, rs, offset8); break;
	default: logerror("UNK_ALUOP.b %s, [%s+#$%02x]", m_regnames8[rd], m_regnames16[rs], offset8); do_nop(); break;
	}
}

void xa_cpu::add_word_rd_rsoff8(u8 rd, u8 rs, u8 offset8) { u16 rdval = gr16(rd); u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rs) + fulloffset; u16 val = rdat16(address); u16 result = do_add_16(rdval, val); sr16(rd, result); cy(6); }
void xa_cpu::addc_word_rd_rsoff8(u8 rd, u8 rs, u8 offset8){ fatalerror("ADDC.w %s, [%s+#$%02x]", m_regnames16[rd], m_regnames16[rs], offset8);}
void xa_cpu::sub_word_rd_rsoff8(u8 rd, u8 rs, u8 offset8) { fatalerror("SUB.w %s, [%s+#$%02x]", m_regnames16[rd], m_regnames16[rs], offset8);}
void xa_cpu::subb_word_rd_rsoff8(u8 rd, u8 rs, u8 offset8){ fatalerror("SUBB.w %s, [%s+#$%02x]", m_regnames16[rd], m_regnames16[rs], offset8);}
void xa_cpu::cmp_word_rd_rsoff8(u8 rd, u8 rs, u8 offset8) { fatalerror("CMP.w %s, [%s+#$%02x]", m_regnames16[rd], m_regnames16[rs], offset8);}
void xa_cpu::and_word_rd_rsoff8(u8 rd, u8 rs, u8 offset8) { fatalerror("AND.w %s, [%s+#$%02x]", m_regnames16[rd], m_regnames16[rs], offset8);}
void xa_cpu::or_word_rd_rsoff8(u8 rd, u8 rs, u8 offset8)  { fatalerror("OR.w %s, [%s+#$%02x]", m_regnames16[rd], m_regnames16[rs], offset8);}
void xa_cpu::xor_word_rd_rsoff8(u8 rd, u8 rs, u8 offset8) { fatalerror("XOR.w %s, [%s+#$%02x]", m_regnames16[rd], m_regnames16[rs], offset8);}
void xa_cpu::mov_word_rd_rsoff8(u8 rd, u8 rs, u8 offset8) { u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rs) + fulloffset; u16 val = rdat16(address); do_nz_flags_16(val); sr16(rd, val); cy(5); }

void xa_cpu::add_byte_rd_rsoff8(u8 rd, u8 rs, u8 offset8) { fatalerror("ADD.b %s, [%s+#$%02x]", m_regnames8[rd], m_regnames16[rs], offset8);}
void xa_cpu::addc_byte_rd_rsoff8(u8 rd, u8 rs, u8 offset8){ fatalerror("ADDC.b %s, [%s+#$%02x]", m_regnames8[rd], m_regnames16[rs], offset8);}
void xa_cpu::sub_byte_rd_rsoff8(u8 rd, u8 rs, u8 offset8) { fatalerror("SUB.b %s, [%s+#$%02x]", m_regnames8[rd], m_regnames16[rs], offset8);}
void xa_cpu::subb_byte_rd_rsoff8(u8 rd, u8 rs, u8 offset8){ fatalerror("SUBB.b %s, [%s+#$%02x]", m_regnames8[rd], m_regnames16[rs], offset8);}
void xa_cpu::cmp_byte_rd_rsoff8(u8 rd, u8 rs, u8 offset8) { fatalerror("CMP.b %s, [%s+#$%02x]", m_regnames8[rd], m_regnames16[rs], offset8);}
void xa_cpu::and_byte_rd_rsoff8(u8 rd, u8 rs, u8 offset8) { fatalerror("AND.b %s, [%s+#$%02x]", m_regnames8[rd], m_regnames16[rs], offset8);}
void xa_cpu::or_byte_rd_rsoff8(u8 rd, u8 rs, u8 offset8)  { fatalerror("OR.b %s, [%s+#$%02x]", m_regnames8[rd], m_regnames16[rs], offset8);}
void xa_cpu::xor_byte_rd_rsoff8(u8 rd, u8 rs, u8 offset8) { fatalerror("XOR.b %s, [%s+#$%02x]", m_regnames8[rd], m_regnames16[rs], offset8);}
void xa_cpu::mov_byte_rd_rsoff8(u8 rd, u8 rs, u8 offset8) { u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rs) + fulloffset; u8 val = rdat8(address); do_nz_flags_8(val); sr8(rd, val); cy(5); }

// ALUOP.w [Rd+off8], Rs
// ALUOP.b [Rd+off8], Rs
// ADD [Rd+offset8], Rs        Add reg to reg-ind w/ 8-bit offs                                        3 6         0000 S100  ssss 1ddd  oooo oooo
// ADDC [Rd+offset8], Rs       Add reg to reg-ind w/ 8-bit offs w/ carry                               3 6         0001 S100  ssss 1ddd  oooo oooo
// SUB [Rd+offset8], Rs        Subtract reg to reg-ind w/ 8-bit offs                                   3 6         0010 S100  ssss 1ddd  oooo oooo
// SUBB [Rd+offset8], Rs       Subtract w/ borrow reg to reg-ind w/ 8-bit offs                         3 6         0011 S100  ssss 1ddd  oooo oooo
// CMP [Rd+offset8], Rs        Compare reg w/ reg-ind w/ 8-bit offs                                    3 6         0100 S100  ssss 1ddd  oooo oooo
// AND [Rd+offset8], Rs        Logical AND reg to reg-ind w/ 8-bit offs                                3 6         0101 S100  ssss 1ddd  oooo oooo
// OR [Rd+offset8], Rs         Logical OR reg to reg-ind w/ 8-bit offs                                 3 6         0110 S100  ssss 1ddd  oooo oooo
// XOR [Rd+offset8], Rs        Logical XOR reg to reg-ind w/ 8-bit offs                                3 6         0111 S100  ssss 1ddd  oooo oooo
// MOV [Rd+offset8], Rs        Move reg to reg-ind w/ 8-bit offs                                       3 5         1000 S100  ssss 1ddd  oooo oooo
void xa_cpu::aluop_word_rdoff8_rs(int alu_op, u8 rd, u8 offset8, u8 rs)
{
	switch (alu_op)
	{
	case 0x0: add_word_rdoff8_rs(rd, offset8, rs); break;
	case 0x1: addc_word_rdoff8_rs(rd, offset8, rs); break;
	case 0x2: sub_word_rdoff8_rs(rd, offset8, rs); break;
	case 0x3: subb_word_rdoff8_rs(rd, offset8, rs); break;
	case 0x4: cmp_word_rdoff8_rs(rd, offset8, rs); break;
	case 0x5: and_word_rdoff8_rs(rd, offset8, rs); break;
	case 0x6: or_word_rdoff8_rs(rd, offset8, rs); break;
	case 0x7: xor_word_rdoff8_rs(rd, offset8, rs); break;
	case 0x8: mov_word_rdoff8_rs(rd, offset8, rs); break;
	default: logerror("UNK_ALUOP.w [%s+#$%02x], %s", m_regnames16[rd], offset8, m_regnames16[rs]); do_nop(); break;
	}
}
void xa_cpu::aluop_byte_rdoff8_rs(int alu_op, u8 rd, u8 offset8, u8 rs)
{
	switch (alu_op)
	{
	case 0x0: add_byte_rdoff8_rs(rd, offset8, rs); break;
	case 0x1: addc_byte_rdoff8_rs(rd, offset8, rs); break;
	case 0x2: sub_byte_rdoff8_rs(rd, offset8, rs); break;
	case 0x3: subb_byte_rdoff8_rs(rd, offset8, rs); break;
	case 0x4: cmp_byte_rdoff8_rs(rd, offset8, rs); break;
	case 0x5: and_byte_rdoff8_rs(rd, offset8, rs); break;
	case 0x6: or_byte_rdoff8_rs(rd, offset8, rs); break;
	case 0x7: xor_byte_rdoff8_rs(rd, offset8, rs); break;
	case 0x8: mov_byte_rdoff8_rs(rd, offset8, rs); break;
	default: logerror("UNK_ALUOP.b [%s+#$%02x], %s", m_regnames16[rd], offset8, m_regnames8[rs]); do_nop(); break;
	}
}

void xa_cpu::add_word_rdoff8_rs(u8 rd, u8 offset8, u8 rs) { fatalerror("ADD.w [%s+#$%02x], %s", m_regnames16[rd], offset8, m_regnames16[rs]);}
void xa_cpu::addc_word_rdoff8_rs(u8 rd, u8 offset8, u8 rs){ fatalerror("ADDC.w [%s+#$%02x], %s", m_regnames16[rd], offset8, m_regnames16[rs]);}
void xa_cpu::sub_word_rdoff8_rs(u8 rd, u8 offset8, u8 rs) { fatalerror("SUB.w [%s+#$%02x], %s", m_regnames16[rd], offset8, m_regnames16[rs]);}
void xa_cpu::subb_word_rdoff8_rs(u8 rd, u8 offset8, u8 rs){ fatalerror("SUBB.w [%s+#$%02x], %s", m_regnames16[rd], offset8, m_regnames16[rs]);}
void xa_cpu::cmp_word_rdoff8_rs(u8 rd, u8 offset8, u8 rs) { fatalerror("CMP.w [%s+#$%02x], %s", m_regnames16[rd], offset8, m_regnames16[rs]);}
void xa_cpu::and_word_rdoff8_rs(u8 rd, u8 offset8, u8 rs) { fatalerror("AND.w [%s+#$%02x], %s", m_regnames16[rd], offset8, m_regnames16[rs]);}
void xa_cpu::or_word_rdoff8_rs(u8 rd, u8 offset8, u8 rs)  { fatalerror("OR.w [%s+#$%02x], %s", m_regnames16[rd], offset8, m_regnames16[rs]);}
void xa_cpu::xor_word_rdoff8_rs(u8 rd, u8 offset8, u8 rs) { fatalerror("XOR.w [%s+#$%02x], %s", m_regnames16[rd], offset8, m_regnames16[rs]);}
void xa_cpu::mov_word_rdoff8_rs(u8 rd, u8 offset8, u8 rs) { u16 val = gr16(rs); u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; do_nz_flags_16(val); wdat16(address, val); cy(5); }

void xa_cpu::add_byte_rdoff8_rs(u8 rd, u8 offset8, u8 rs) { fatalerror("ADD.b [%s+#$%02x], %s", m_regnames16[rd], offset8, m_regnames8[rs]);}
void xa_cpu::addc_byte_rdoff8_rs(u8 rd, u8 offset8, u8 rs){ fatalerror("ADDC.b [%s+#$%02x], %s", m_regnames16[rd], offset8, m_regnames8[rs]);}
void xa_cpu::sub_byte_rdoff8_rs(u8 rd, u8 offset8, u8 rs) { fatalerror("SUB.b [%s+#$%02x], %s", m_regnames16[rd], offset8, m_regnames8[rs]);}
void xa_cpu::subb_byte_rdoff8_rs(u8 rd, u8 offset8, u8 rs){ fatalerror("SUBB.b [%s+#$%02x], %s", m_regnames16[rd], offset8, m_regnames8[rs]);}
void xa_cpu::cmp_byte_rdoff8_rs(u8 rd, u8 offset8, u8 rs) { fatalerror("CMP.b [%s+#$%02x], %s", m_regnames16[rd], offset8, m_regnames8[rs]);}
void xa_cpu::and_byte_rdoff8_rs(u8 rd, u8 offset8, u8 rs) { fatalerror("AND.b [%s+#$%02x], %s", m_regnames16[rd], offset8, m_regnames8[rs]);}
void xa_cpu::or_byte_rdoff8_rs(u8 rd, u8 offset8, u8 rs)  { fatalerror("OR.b [%s+#$%02x], %s", m_regnames16[rd], offset8, m_regnames8[rs]);}
void xa_cpu::xor_byte_rdoff8_rs(u8 rd, u8 offset8, u8 rs) { fatalerror("XOR.b [%s+#$%02x], %s", m_regnames16[rd], offset8, m_regnames8[rs]);}
void xa_cpu::mov_byte_rdoff8_rs(u8 rd, u8 offset8, u8 rs) { u8 val = gr8(rs); u16 fulloffset = util::sext(offset8, 8); u16 address = get_addr(rd) + fulloffset; do_nz_flags_8(val); wdat8(address, val); cy(5); }


// ALUOP.w Rd, [Rs+off16]
// ALUOP.b Rd, [Rs+off16]
// ADD Rd, [Rs+offset16]       Add reg-ind w/ 16-bit offs to reg                                       4 6         0000 S101  dddd 0sss  oooo oooo  oooo oooo
// ADDC Rd, [Rs+offset16]      Add reg-ind w/ 16-bit offs to reg w/ carry                              4 6         0001 S101  dddd 0sss  oooo oooo  oooo oooo
// SUB Rd, [Rs+offset16]       Subtract reg-ind w/ 16-bit offs to reg                                  4 6         0010 S101  dddd 0sss  oooo oooo  oooo oooo
// SUBB Rd, [Rs+offset16]      Subtract w/ borrow reg-ind w/ 16-bit offs to reg                        4 6         0011 S101  dddd 0sss  oooo oooo  oooo oooo
// CMP Rd, [Rs+offset16]       Compare reg-ind w/ 16-bit offs w/ reg                                   4 6         0100 S101  dddd 0sss  oooo oooo  oooo oooo
// AND Rd, [Rs+offset16]       Logical AND reg-ind w/ 16-bit offs to reg                               4 6         0101 S101  dddd 0sss  oooo oooo  oooo oooo
// OR Rd, [Rs+offset16]        Logical OR reg-ind w/ 16-bit offs to reg                                4 6         0110 S101  dddd 0sss  oooo oooo  oooo oooo
// XOR Rd, [Rs+offset16]       Logical XOR reg-ind w/ 16-bit offs to reg                               4 6         0111 S101  dddd 0sss  oooo oooo  oooo oooo
// MOV Rd, [Rs+offset16]       Move reg-ind w/ 16-bit offs to reg                                      4 5         1000 S101  dddd 0sss  oooo oooo  oooo oooo
void xa_cpu::aluop_word_rsoff16(int alu_op, u8 rd, u8 rs, u16 offset16)
{
	switch (alu_op)
	{
	case 0x0: add_word_rd_rsoff16(rd, rs, offset16); break;
	case 0x1: addc_word_rd_rsoff16(rd, rs, offset16); break;
	case 0x2: sub_word_rd_rsoff16(rd, rs, offset16); break;
	case 0x3: subb_word_rd_rsoff16(rd, rs, offset16); break;
	case 0x4: cmp_word_rd_rsoff16(rd, rs, offset16); break;
	case 0x5: and_word_rd_rsoff16(rd, rs, offset16); break;
	case 0x6: or_word_rd_rsoff16(rd, rs, offset16); break;
	case 0x7: xor_word_rd_rsoff16(rd, rs, offset16); break;
	case 0x8: mov_word_rd_rsoff16(rd, rs, offset16); break;
	default: logerror("UNK_ALUOP.w %s, [%s+#$%04x]", m_regnames16[rd], m_regnames16[rs], offset16); do_nop(); break;
	}
}
void xa_cpu::aluop_byte_rsoff16(int alu_op, u8 rd, u8 rs, u16 offset16)
{
	switch (alu_op)
	{
	case 0x0: add_byte_rd_rsoff16(rd, rs, offset16); break;
	case 0x1: addc_byte_rd_rsoff16(rd, rs, offset16); break;
	case 0x2: sub_byte_rd_rsoff16(rd, rs, offset16); break;
	case 0x3: subb_byte_rd_rsoff16(rd, rs, offset16); break;
	case 0x4: cmp_byte_rd_rsoff16(rd, rs, offset16); break;
	case 0x5: and_byte_rd_rsoff16(rd, rs, offset16); break;
	case 0x6: or_byte_rd_rsoff16(rd, rs, offset16); break;
	case 0x7: xor_byte_rd_rsoff16(rd, rs, offset16); break;
	case 0x8: mov_byte_rd_rsoff16(rd, rs, offset16); break;
	default: logerror("UNK_ALUOP.b %s, [%s+#$%04x]", m_regnames8[rd], m_regnames16[rs], offset16); do_nop(); break;
	}
}

void xa_cpu::add_word_rd_rsoff16(u8 rd, u8 rs, u16 offset16) { fatalerror("ADD.w %s, [%s+#$%04x]", m_regnames16[rd], m_regnames16[rs], offset16);}
void xa_cpu::addc_word_rd_rsoff16(u8 rd, u8 rs, u16 offset16){ fatalerror("ADDC.w %s, [%s+#$%04x]", m_regnames16[rd], m_regnames16[rs], offset16);}
void xa_cpu::sub_word_rd_rsoff16(u8 rd, u8 rs, u16 offset16) { fatalerror("SUB.w %s, [%s+#$%04x]", m_regnames16[rd], m_regnames16[rs], offset16);}
void xa_cpu::subb_word_rd_rsoff16(u8 rd, u8 rs, u16 offset16){ fatalerror("SUBB.w %s, [%s+#$%04x]", m_regnames16[rd], m_regnames16[rs], offset16);}
void xa_cpu::cmp_word_rd_rsoff16(u8 rd, u8 rs, u16 offset16) { fatalerror("CMP.w %s, [%s+#$%04x]", m_regnames16[rd], m_regnames16[rs], offset16);}
void xa_cpu::and_word_rd_rsoff16(u8 rd, u8 rs, u16 offset16) { fatalerror("AND.w %s, [%s+#$%04x]", m_regnames16[rd], m_regnames16[rs], offset16);}
void xa_cpu::or_word_rd_rsoff16(u8 rd, u8 rs, u16 offset16)  { fatalerror("OR.w %s, [%s+#$%04x]", m_regnames16[rd], m_regnames16[rs], offset16);}
void xa_cpu::xor_word_rd_rsoff16(u8 rd, u8 rs, u16 offset16) { fatalerror("XOR.w %s, [%s+#$%04x]", m_regnames16[rd], m_regnames16[rs], offset16);}
void xa_cpu::mov_word_rd_rsoff16(u8 rd, u8 rs, u16 offset16) { fatalerror("MOV.w %s, [%s+#$%04x]", m_regnames16[rd], m_regnames16[rs], offset16);}

void xa_cpu::add_byte_rd_rsoff16(u8 rd, u8 rs, u16 offset16) { fatalerror("ADD.b %s, [%s+#$%04x]", m_regnames8[rd], m_regnames16[rs], offset16);}
void xa_cpu::addc_byte_rd_rsoff16(u8 rd, u8 rs, u16 offset16){ fatalerror("ADDC.b %s, [%s+#$%04x]", m_regnames8[rd], m_regnames16[rs], offset16);}
void xa_cpu::sub_byte_rd_rsoff16(u8 rd, u8 rs, u16 offset16) { fatalerror("SUB.b %s, [%s+#$%04x]", m_regnames8[rd], m_regnames16[rs], offset16);}
void xa_cpu::subb_byte_rd_rsoff16(u8 rd, u8 rs, u16 offset16){ fatalerror("SUBB.b %s, [%s+#$%04x]", m_regnames8[rd], m_regnames16[rs], offset16);}
void xa_cpu::cmp_byte_rd_rsoff16(u8 rd, u8 rs, u16 offset16) { fatalerror("CMP.b %s, [%s+#$%04x]", m_regnames8[rd], m_regnames16[rs], offset16);}
void xa_cpu::and_byte_rd_rsoff16(u8 rd, u8 rs, u16 offset16) { fatalerror("AND.b %s, [%s+#$%04x]", m_regnames8[rd], m_regnames16[rs], offset16);}
void xa_cpu::or_byte_rd_rsoff16(u8 rd, u8 rs, u16 offset16)  { fatalerror("OR.b %s, [%s+#$%04x]", m_regnames8[rd], m_regnames16[rs], offset16);}
void xa_cpu::xor_byte_rd_rsoff16(u8 rd, u8 rs, u16 offset16) { fatalerror("XOR.b %s, [%s+#$%04x]", m_regnames8[rd], m_regnames16[rs], offset16);}
void xa_cpu::mov_byte_rd_rsoff16(u8 rd, u8 rs, u16 offset16) { fatalerror("MOV.b %s, [%s+#$%04x]", m_regnames8[rd], m_regnames16[rs], offset16);}

// ALUOP.w [Rd+off16], Rs
// ALUOP.b [Rd+off16], Rs
// ADD [Rd+offset16], Rs       Add reg to reg-ind w/ 16-bit offs                                       4 6         0000 S101  ssss 1ddd  oooo oooo  oooo oooo
// ADDC [Rd+offset16], Rs      Add reg to reg-ind w/ 16-bit offs w/ carry                              4 6         0001 S101  ssss 1ddd  oooo oooo  oooo oooo
// SUB [Rd+offset16], Rs       Subtract reg to reg-ind w/ 16-bit offs                                  4 6         0010 S101  ssss 1ddd  oooo oooo  oooo oooo
// SUBB [Rd+offset16], Rs      Subtract w/ borrow reg to reg-ind w/ 16-bit offs                        4 6         0011 S101  ssss 1ddd  oooo oooo  oooo oooo
// CMP [Rd+offset16], Rs       Compare reg w/ reg-ind w/ 16-bit offs                                   4 6         0100 S101  ssss 1ddd  oooo oooo  oooo oooo
// AND [Rd+offset16], Rs       Logical AND reg to reg-ind w/ 16-bit offs                               4 6         0101 S101  ssss 1ddd  oooo oooo  oooo oooo
// OR [Rd+offset16], Rs        Logical OR reg to reg-ind w/ 16-bit offs                                4 6         0110 S101  ssss 1ddd  oooo oooo  oooo oooo
// XOR [Rd+offset16], Rs       Logical XOR reg to reg-ind w/ 16-bit offs                               4 6         0111 S101  ssss 1ddd  oooo oooo  oooo oooo
// MOV [Rd+offset16], Rs       Move reg to reg-ind w/ 16-bit offs                                      4 5         1000 S101  ssss 1ddd  oooo oooo  oooo oooo
void xa_cpu::aluop_word_rdoff16_rs(int alu_op, u8 rd, u16 offset16, u8 rs)
{
	switch (alu_op)
	{
	case 0x0: add_word_rdoff16_rs(rd, offset16, rs); break;
	case 0x1: addc_word_rdoff16_rs(rd, offset16, rs); break;
	case 0x2: sub_word_rdoff16_rs(rd, offset16, rs); break;
	case 0x3: subb_word_rdoff16_rs(rd, offset16, rs); break;
	case 0x4: cmp_word_rdoff16_rs(rd, offset16, rs); break;
	case 0x5: and_word_rdoff16_rs(rd, offset16, rs); break;
	case 0x6: or_word_rdoff16_rs(rd, offset16, rs); break;
	case 0x7: xor_word_rdoff16_rs(rd, offset16, rs); break;
	case 0x8: mov_word_rdoff16_rs(rd, offset16, rs); break;
	default: logerror("UNK_ALUOP.w [%s+#$%04x], %s", m_regnames16[rd], offset16, m_regnames16[rs]); do_nop(); break;
	}
}
void xa_cpu::aluop_byte_rdoff16_rs(int alu_op, u8 rd, u16 offset16, u8 rs)
{
	switch (alu_op)
	{
	case 0x0: add_byte_rdoff16_rs(rd, offset16, rs); break;
	case 0x1: addc_byte_rdoff16_rs(rd, offset16, rs); break;
	case 0x2: sub_byte_rdoff16_rs(rd, offset16, rs); break;
	case 0x3: subb_byte_rdoff16_rs(rd, offset16, rs); break;
	case 0x4: cmp_byte_rdoff16_rs(rd, offset16, rs); break;
	case 0x5: and_byte_rdoff16_rs(rd, offset16, rs); break;
	case 0x6: or_byte_rdoff16_rs(rd, offset16, rs); break;
	case 0x7: xor_byte_rdoff16_rs(rd, offset16, rs); break;
	case 0x8: mov_byte_rdoff16_rs(rd, offset16, rs); break;
	default: logerror("UNK_ALUOP.b [%s+#$%04x], %s", m_regnames16[rd], offset16, m_regnames8[rs]); do_nop(); break;
	}
}

void xa_cpu::add_word_rdoff16_rs(u8 rd, u16 offset16, u8 rs) { fatalerror("ADD.w [%s+#$%04x], %s", m_regnames16[rd], offset16, m_regnames16[rs]);}
void xa_cpu::addc_word_rdoff16_rs(u8 rd, u16 offset16, u8 rs){ fatalerror("ADDC.w [%s+#$%04x], %s", m_regnames16[rd], offset16, m_regnames16[rs]);}
void xa_cpu::sub_word_rdoff16_rs(u8 rd, u16 offset16, u8 rs) { fatalerror("SUB.w [%s+#$%04x], %s", m_regnames16[rd], offset16, m_regnames16[rs]);}
void xa_cpu::subb_word_rdoff16_rs(u8 rd, u16 offset16, u8 rs){ fatalerror("SUBB.w [%s+#$%04x], %s", m_regnames16[rd], offset16, m_regnames16[rs]);}
void xa_cpu::cmp_word_rdoff16_rs(u8 rd, u16 offset16, u8 rs) { fatalerror("CMP.w [%s+#$%04x], %s", m_regnames16[rd], offset16, m_regnames16[rs]);}
void xa_cpu::and_word_rdoff16_rs(u8 rd, u16 offset16, u8 rs) { fatalerror("AND.w [%s+#$%04x], %s", m_regnames16[rd], offset16, m_regnames16[rs]);}
void xa_cpu::or_word_rdoff16_rs(u8 rd, u16 offset16, u8 rs)  { fatalerror("OR.w [%s+#$%04x], %s", m_regnames16[rd], offset16, m_regnames16[rs]);}
void xa_cpu::xor_word_rdoff16_rs(u8 rd, u16 offset16, u8 rs) { fatalerror("XOR.w [%s+#$%04x], %s", m_regnames16[rd], offset16, m_regnames16[rs]);}
void xa_cpu::mov_word_rdoff16_rs(u8 rd, u16 offset16, u8 rs) { u16 val = gr16(rs); u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; do_nz_flags_16(val); wdat16(address, val); cy(5); }

void xa_cpu::add_byte_rdoff16_rs(u8 rd, u16 offset16, u8 rs) { fatalerror("ADD.b [%s+#$%04x], %s", m_regnames16[rd], offset16, m_regnames8[rs]);}
void xa_cpu::addc_byte_rdoff16_rs(u8 rd, u16 offset16, u8 rs){ fatalerror("ADDC.b [%s+#$%04x], %s", m_regnames16[rd], offset16, m_regnames8[rs]);}
void xa_cpu::sub_byte_rdoff16_rs(u8 rd, u16 offset16, u8 rs) { fatalerror("SUB.b [%s+#$%04x], %s", m_regnames16[rd], offset16, m_regnames8[rs]);}
void xa_cpu::subb_byte_rdoff16_rs(u8 rd, u16 offset16, u8 rs){ fatalerror("SUBB.b [%s+#$%04x], %s", m_regnames16[rd], offset16, m_regnames8[rs]);}
void xa_cpu::cmp_byte_rdoff16_rs(u8 rd, u16 offset16, u8 rs) { fatalerror("CMP.b [%s+#$%04x], %s", m_regnames16[rd], offset16, m_regnames8[rs]);}
void xa_cpu::and_byte_rdoff16_rs(u8 rd, u16 offset16, u8 rs) { fatalerror("AND.b [%s+#$%04x], %s", m_regnames16[rd], offset16, m_regnames8[rs]);}
void xa_cpu::or_byte_rdoff16_rs(u8 rd, u16 offset16, u8 rs)  { fatalerror("OR.b [%s+#$%04x], %s", m_regnames16[rd], offset16, m_regnames8[rs]);}
void xa_cpu::xor_byte_rdoff16_rs(u8 rd, u16 offset16, u8 rs) { fatalerror("XOR.b [%s+#$%04x], %s", m_regnames16[rd], offset16, m_regnames8[rs]);}
void xa_cpu::mov_byte_rdoff16_rs(u8 rd, u16 offset16, u8 rs) { u8 val = gr16(rs); u16 fulloffset = offset16; u16 address = get_addr(rd) + fulloffset; do_nz_flags_8(val); wdat8(address, val); cy(5); }

// ALUOP.w Rd, Direct
// ALUOP.b Rd, Direct
// ADD Rd, direct              Add mem to reg                                                          3 4         0000 S110  dddd 0DDD  DDDD DDDD
// ADDC Rd, direct             Add mem to reg w/ carry                                                 3 4         0001 S110  dddd 0DDD  DDDD DDDD
// SUB Rd, direct              Subtract mem to reg                                                     3 4         0010 S110  dddd 0DDD  DDDD DDDD
// SUBB Rd, direct             Subtract w/ borrow mem to reg                                           3 4         0011 S110  dddd 0DDD  DDDD DDDD
// CMP Rd, direct              Compare mem w/ reg                                                      3 4         0100 S110  dddd 0DDD  DDDD DDDD
// AND Rd, direct              Logical AND mem to reg                                                  3 4         0101 S110  dddd 0DDD  DDDD DDDD
// OR Rd, direct               Logical OR mem to reg                                                   3 4         0110 S110  dddd 0DDD  DDDD DDDD
// XOR Rd, direct              Logical XOR mem to reg                                                  3 4         0111 S110  dddd 0DDD  DDDD DDDD
// MOV Rd, direct              Move mem to reg                                                         3 4         1000 S110  dddd 0DDD  DDDD DDDD
void xa_cpu::aluop_word_rd_direct(int alu_op, u8 rd, u16 direct)
{
	switch (alu_op)
	{
	case 0x0: add_word_rd_direct(rd, direct); break;
	case 0x1: addc_word_rd_direct(rd, direct); break;
	case 0x2: sub_word_rd_direct(rd, direct); break;
	case 0x3: subb_word_rd_direct(rd, direct); break;
	case 0x4: cmp_word_rd_direct(rd, direct); break;
	case 0x5: and_word_rd_direct(rd, direct); break;
	case 0x6: or_word_rd_direct(rd, direct); break;
	case 0x7: xor_word_rd_direct(rd, direct); break;
	case 0x8: mov_word_rd_direct(rd, direct); break;
	default: logerror("UNK_ALUOP.w %s, %s", m_regnames16[rd], get_directtext(direct)); do_nop(); break;
	}
}

void xa_cpu::aluop_byte_rd_direct(int alu_op, u8 rd, u16 direct)
{
	switch (alu_op)
	{
	case 0x0: add_byte_rd_direct(rd, direct); break;
	case 0x1: addc_byte_rd_direct(rd, direct); break;
	case 0x2: sub_byte_rd_direct(rd, direct); break;
	case 0x3: subb_byte_rd_direct(rd, direct); break;
	case 0x4: cmp_byte_rd_direct(rd, direct); break;
	case 0x5: and_byte_rd_direct(rd, direct); break;
	case 0x6: or_byte_rd_direct(rd, direct); break;
	case 0x7: xor_byte_rd_direct(rd, direct); break;
	case 0x8: mov_byte_rd_direct(rd, direct); break;
	default: logerror("UNK_ALUOP.b %s, %s", m_regnames8[rd], get_directtext(direct)); do_nop(); break;
	}
}

void xa_cpu::add_word_rd_direct(u8 rd, u16 direct) { u16 rdval = gr16(rd); u16 val = read_direct16(direct); u16 result = do_add_16(rdval, val); sr16(rd, result); cy(4); }
void xa_cpu::addc_word_rd_direct(u8 rd, u16 direct){ u16 rdval = gr16(rd); u16 val = read_direct16(direct); u16 result = do_addc_16(rdval, val); sr16(rd, result); cy(4);}
void xa_cpu::sub_word_rd_direct(u8 rd, u16 direct) { u16 rdval = gr16(rd); u16 val = read_direct16(direct); u16 result = do_sub_16(rdval, val); sr16(rd, result); cy(4); }
void xa_cpu::subb_word_rd_direct(u8 rd, u16 direct){ u16 rdval = gr16(rd); u16 val = read_direct16(direct); u16 result = do_subb_16(rdval, val); sr16(rd, result); cy(4); }
void xa_cpu::cmp_word_rd_direct(u8 rd, u16 direct) { u16 rdval = gr16(rd); u16 val = read_direct16(direct); do_sub_16(rdval, val); cy(4); }
void xa_cpu::and_word_rd_direct(u8 rd, u16 direct) { u16 rdval = gr16(rd); u16 val = read_direct16(direct); u16 result = do_and_16(rdval, val); sr16(rd, result); cy(4); }
void xa_cpu::or_word_rd_direct(u8 rd, u16 direct)  { u16 rdval = gr16(rd); u16 val = read_direct16(direct); u16 result = do_or_16(rdval, val); sr16(rd, result); cy(4); }
void xa_cpu::xor_word_rd_direct(u8 rd, u16 direct) { u16 rdval = gr16(rd); u16 val = read_direct16(direct); u16 result = do_xor_16(rdval, val); sr16(rd, result); cy(4); }
void xa_cpu::mov_word_rd_direct(u8 rd, u16 direct) { u16 val = read_direct16(direct); do_nz_flags_16(val); sr16(rd, val); cy(4);}

void xa_cpu::add_byte_rd_direct(u8 rd, u16 direct) { fatalerror("ADD.b %s, %s", m_regnames8[rd], get_directtext(direct));}
void xa_cpu::addc_byte_rd_direct(u8 rd, u16 direct){ fatalerror("ADDC.b %s, %s", m_regnames8[rd], get_directtext(direct));}
void xa_cpu::sub_byte_rd_direct(u8 rd, u16 direct) { fatalerror("SUB.b %s, %s", m_regnames8[rd], get_directtext(direct));}
void xa_cpu::subb_byte_rd_direct(u8 rd, u16 direct){ fatalerror("SUBB.b %s, %s", m_regnames8[rd], get_directtext(direct));}
void xa_cpu::cmp_byte_rd_direct(u8 rd, u16 direct) { u8 rdval = gr8(rd); u8 val = read_direct8(direct); do_sub_8(rdval, val); cy(4); }
void xa_cpu::and_byte_rd_direct(u8 rd, u16 direct) { fatalerror("AND.b %s, %s", m_regnames8[rd], get_directtext(direct));}
void xa_cpu::or_byte_rd_direct(u8 rd, u16 direct)  { fatalerror("OR.b %s, %s", m_regnames8[rd], get_directtext(direct));}
void xa_cpu::xor_byte_rd_direct(u8 rd, u16 direct) { fatalerror("XOR.b %s, %s", m_regnames8[rd], get_directtext(direct));}
void xa_cpu::mov_byte_rd_direct(u8 rd, u16 direct) { u8 val = read_direct8(direct); do_nz_flags_8(val); sr8(rd, val); cy(4); }

// ALUOP.w Direct, Rs
// ALUOP.b Direct, Rs
// ADD direct, Rs              Add reg to mem                                                          3 4         0000 S110  ssss 1DDD  DDDD DDDD
// ADDC direct, Rs             Add reg to mem w/ carry                                                 3 4         0001 S110  ssss 1DDD  DDDD DDDD
// SUB direct, Rs              Subtract reg to mem                                                     3 4         0010 S110  ssss 1DDD  DDDD DDDD
// SUBB direct, Rs             Subtract w/ borrow reg to mem                                           3 4         0011 S110  ssss 1DDD  DDDD DDDD
// CMP direct, Rs              Compare reg w/ mem                                                      3 4         0100 S110  ssss 1DDD  DDDD DDDD
// AND direct, Rs              Logical AND reg to mem                                                  3 4         0101 S110  ssss 1DDD  DDDD DDDD
// OR direct, Rs               Logical OR reg to mem                                                   3 4         0110 S110  ssss 1DDD  DDDD DDDD
// XOR direct, Rs              Logical XOR reg to mem                                                  3 4         0111 S110  ssss 1DDD  DDDD DDDD
// MOV direct, Rs              Move reg to mem                                                         3 4         1000 S110  ssss 1DDD  DDDD DDDD

void xa_cpu::aluop_word_direct_rs(int alu_op, u16 direct, u8 rs)
{
	switch (alu_op)
	{
	case 0x0: add_word_direct_rs(direct, rs); break;
	case 0x1: addc_word_direct_rs(direct, rs); break;
	case 0x2: sub_word_direct_rs(direct, rs); break;
	case 0x3: subb_word_direct_rs(direct, rs); break;
	case 0x4: cmp_word_direct_rs(direct, rs); break;
	case 0x5: and_word_direct_rs(direct, rs); break;
	case 0x6: or_word_direct_rs(direct, rs); break;
	case 0x7: xor_word_direct_rs(direct, rs); break;
	case 0x8: mov_word_direct_rs(direct, rs); break;
	default: logerror("UNK_ALUOP.w %s, %s", get_directtext(direct), m_regnames16[rs]); do_nop(); break;
	}
}

void xa_cpu::aluop_byte_direct_rs(int alu_op, u16 direct, u8 rs)
{
	switch (alu_op)
	{
	case 0x0: add_byte_direct_rs(direct, rs); break;
	case 0x1: addc_byte_direct_rs(direct, rs); break;
	case 0x2: sub_byte_direct_rs(direct, rs); break;
	case 0x3: subb_byte_direct_rs(direct, rs); break;
	case 0x4: cmp_byte_direct_rs(direct, rs); break;
	case 0x5: and_byte_direct_rs(direct, rs); break;
	case 0x6: or_byte_direct_rs(direct, rs); break;
	case 0x7: xor_byte_direct_rs(direct, rs); break;
	case 0x8: mov_byte_direct_rs(direct, rs); break;
	default: logerror("UNK_ALUOP.b %s, %s", get_directtext(direct), m_regnames8[rs]); do_nop(); break;
	}
}

void xa_cpu::add_word_direct_rs(u16 direct, u8 rs) { u16 directval = read_direct16(direct); u16 val = gr16(rs); u16 result = do_add_16(directval, val); write_direct16(direct, result); cy(4); }
void xa_cpu::addc_word_direct_rs(u16 direct, u8 rs){ u16 directval = read_direct16(direct); u16 val = gr16(rs); u16 result = do_addc_16(directval, val); write_direct16(direct, result); cy(4); }
void xa_cpu::sub_word_direct_rs(u16 direct, u8 rs) { u16 directval = read_direct16(direct); u16 val = gr16(rs); u16 result = do_sub_16(directval, val); write_direct16(direct, result); cy(4); }
void xa_cpu::subb_word_direct_rs(u16 direct, u8 rs){ u16 directval = read_direct16(direct); u16 val = gr16(rs); u16 result = do_subb_16(directval, val); write_direct16(direct, result); cy(4) ; }
void xa_cpu::cmp_word_direct_rs(u16 direct, u8 rs) { u16 directval = read_direct16(direct); u16 val = gr16(rs); do_sub_16(directval, val); cy(4); }
void xa_cpu::and_word_direct_rs(u16 direct, u8 rs) { u16 directval = read_direct16(direct); u16 val = gr16(rs); u16 result = do_and_16(directval, val); write_direct16(direct, result); cy(4); }
void xa_cpu::or_word_direct_rs(u16 direct, u8 rs)  { u16 directval = read_direct16(direct); u16 val = gr16(rs); u16 result = do_or_16(directval, val); write_direct16(direct, result); cy(4); }
void xa_cpu::xor_word_direct_rs(u16 direct, u8 rs) { u16 directval = read_direct16(direct); u16 val = gr16(rs); u16 result = do_xor_16(directval, val); write_direct16(direct, result); cy(4); }
void xa_cpu::mov_word_direct_rs(u16 direct, u8 rs) { u16 val = gr16(rs); do_nz_flags_16(val); write_direct16(direct, val); cy(4); }

void xa_cpu::add_byte_direct_rs(u16 direct, u8 rs) { fatalerror("ADD.b %s, %s", get_directtext(direct), m_regnames8[rs]);}
void xa_cpu::addc_byte_direct_rs(u16 direct, u8 rs){ fatalerror("ADDC.b %s, %s", get_directtext(direct), m_regnames8[rs]);}
void xa_cpu::sub_byte_direct_rs(u16 direct, u8 rs) { fatalerror("SUB.b %s, %s", get_directtext(direct), m_regnames8[rs]);}
void xa_cpu::subb_byte_direct_rs(u16 direct, u8 rs){ fatalerror("SUBB.b %s, %s", get_directtext(direct), m_regnames8[rs]);}
void xa_cpu::cmp_byte_direct_rs(u16 direct, u8 rs) { u8 directval = read_direct8(direct); u8 val = gr8(rs); do_sub_8(directval, val); cy(4); }
void xa_cpu::and_byte_direct_rs(u16 direct, u8 rs) { fatalerror("AND.b %s, %s", get_directtext(direct), m_regnames8[rs]);}
void xa_cpu::or_byte_direct_rs(u16 direct, u8 rs)  { fatalerror("OR.b %s, %s", get_directtext(direct), m_regnames8[rs]);}
void xa_cpu::xor_byte_direct_rs(u16 direct, u8 rs) { fatalerror("XOR.b %s, %s", get_directtext(direct), m_regnames8[rs]);}
void xa_cpu::mov_byte_direct_rs(u16 direct, u8 rs) { u8 val = gr8(rs); do_nz_flags_8(val); write_direct8(direct, val); cy(4); }

///////////////////////////////////////////////////////

// MOVS Rd, #data4             Move 4-bit sign-extended imm data to reg                                2 3         1011 S001  dddd iiii
void xa_cpu::movs_word_rd_data4(u8 rd, u8 data4) { u16 data = util::sext(data4, 4); sr16(rd, data); do_nz_flags_16(data); cy(3); }
void xa_cpu::movs_byte_rd_data4(u8 rd, u8 data4) { u8 data  = util::sext(data4, 4); sr8(rd, data);  do_nz_flags_8(data);  cy(3); }
// ADDS Rd, #data4             Add 4-bit signed imm data to reg                                        2 3         1010 S001  dddd iiii
void xa_cpu::adds_word_rd_data4(u8 rd, u8 data4) { u16 data = util::sext(data4, 4); u16 regval = gr16(rd); regval += data; do_nz_flags_16(regval); sr16(rd, regval); cy(3); }
void xa_cpu::adds_byte_rd_data4(u8 rd, u8 data4) { u8 data  = util::sext(data4, 4); u8 regval  = gr8(rd);  regval += data; do_nz_flags_8(regval);  sr8(rd, regval); cy(3); }

// MOVS [Rd], #data4           Move 4-bit sign-extended imm data to reg-ind                            2 3         1011 S010  0ddd iiii
void xa_cpu::movs_word_indrd_data4(u8 rd, u8 data4){ u16 data = util::sext(data4, 4); u16 address = get_addr(rd); wdat16(address, data); do_nz_flags_16(data); cy(3); }
void xa_cpu::movs_byte_indrd_data4(u8 rd, u8 data4){ u8 data = util::sext(data4, 4); u16 address = get_addr(rd); wdat8(address, data); do_nz_flags_8(data); cy(3); }
// ADDS [Rd], #data4           Add 4-bit signed imm data to reg-ind                                    2 4         1010 S010  0ddd iiii
void xa_cpu::adds_word_indrd_data4(u8 rd, u8 data4){ u16 data = util::sext(data4, 4); u16 address = get_addr(rd); u16 rdval = rdat16(address); u16 result = do_add_16(rdval, data); wdat16(address, result); cy(4); }
void xa_cpu::adds_byte_indrd_data4(u8 rd, u8 data4){ u8 data = util::sext(data4, 4); u16 address = get_addr(rd); u8 rdval = rdat8(address); u8 result = do_add_8(rdval, data); wdat8(address, result); cy(4); }

// MOVS [Rd+], #data4          Move 4-bit sign-extended imm data to reg-ind w/ autoinc                 2 4         1011 S011  0ddd iiii
void xa_cpu::movs_word_indrdinc_data4(u8 rd, u8 data4) { u16 data = util::sext(data4, 4); u16 address = get_addr(rd); wdat16(address, data); address += 2; do_nz_flags_16(data); set_addr(rd, address); cy(4); }
void xa_cpu::movs_byte_indrdinc_data4(u8 rd, u8 data4) { u8 data  = util::sext(data4, 4); u16 address = get_addr(rd); wdat8(address, data);  address++;    do_nz_flags_8(data);  set_addr(rd, address); cy(4); }
// ADDS [Rd+], #data4          Add 4-bit signed imm data to reg-ind w/ autoinc                         2 5         1010 S011  0ddd iiii
void xa_cpu::adds_word_indrdinc_data4(u8 rd, u8 data4){ fatalerror("ADDS.w [%s+], %s", m_regnames16[rd], show_expanded_data4(data4, 1)); }
void xa_cpu::adds_byte_indrdinc_data4(u8 rd, u8 data4){ fatalerror("ADDS.b [%s+], %s", m_regnames16[rd], show_expanded_data4(data4, 0)); }

// MOVS [Rd+offset8], #data4   Move reg-ind w/ 8-bit offs to 4-bit sign-extended imm data              3 5         1011 S100  0ddd iiii  oooo oooo
void xa_cpu::movs_word_indrdoff8_data4(u8 rd, u8 off8, u8 data4) { u16 data = util::sext(data4, 4); u16 fulloffset = util::sext(off8, 8); u16 address = get_addr(rd) + fulloffset; wdat16(address, data); do_nz_flags_16(data); cy(5); }
void xa_cpu::movs_byte_indrdoff8_data4(u8 rd, u8 off8, u8 data4){ u8 data = util::sext(data4, 4); u16 fulloffset = util::sext(off8, 8); u16 address = get_addr(rd) + fulloffset; wdat8(address, data); do_nz_flags_8(data); cy(5);  }
// ADDS [Rd+offset8], #data4   Add reg-ind w/ 8-bit offs to 4-bit signed imm data                      3 6         1010 S100  0ddd iiii  oooo oooo
void xa_cpu::adds_word_indrdoff8_data4(u8 rd, u8 off8, u8 data4){ u16 data = util::sext(data4, 4); u16 fulloffset = util::sext(off8, 8); u16 address = get_addr(rd) + fulloffset; u16 rdval = rdat16(address); u16 result = do_add_16(rdval, data); wdat16(address, result); cy(4); }
void xa_cpu::adds_byte_indrdoff8_data4(u8 rd, u8 off8, u8 data4){ u8 data = util::sext(data4, 4); u16 fulloffset = util::sext(off8, 8); u16 address = get_addr(rd) + fulloffset; u8 rdval = rdat8(address); u8 result = do_add_8(rdval, data); wdat8(address, result); cy(4); }

// MOVS [Rd+offset16], #data4  Move reg-ind w/ 16-bit offs to 4-bit sign-extended imm data             4 5         1011 S101  0ddd iiii  oooo oooo  oooo oooo
void xa_cpu::movs_word_indrdoff16_data4(u8 rd, u16 off16, u8 data4){ fatalerror("MOVS.w [%s+$%04x], %s", m_regnames16[rd], off16, show_expanded_data4(data4, 1)); }
void xa_cpu::movs_byte_indrdoff16_data4(u8 rd, u16 off16, u8 data4){ fatalerror("MOVS.b [%s+$%04x], %s", m_regnames16[rd], off16, show_expanded_data4(data4, 0)); }
// ADDS [Rd+offset16], #data4  Add reg-ind w/ 16-bit offs to 4-bit signed imm data                     4 6         1010 S101  0ddd iiii  oooo oooo  oooo oooo
void xa_cpu::adds_word_indrdoff16_data4(u8 rd, u16 off16, u8 data4){ fatalerror("ADDS.w [%s+$%04x], %s", m_regnames16[rd], off16, show_expanded_data4(data4, 1)); }
void xa_cpu::adds_byte_indrdoff16_data4(u8 rd, u16 off16, u8 data4){ fatalerror("ADDS.b [%s+$%04x], %s", m_regnames16[rd], off16, show_expanded_data4(data4, 0)); }

// MOVS direct, #data4         Move 4-bit sign-extended imm data to mem                                3 3         1011 S110  0DDD iiii  DDDD DDDD
void xa_cpu::movs_word_direct_data4(u16 direct, u8 data4) { u16 data = util::sext(data4, 4); do_nz_flags_16(data); write_direct16(direct, data); cy(3); }
void xa_cpu::movs_byte_direct_data4(u16 direct, u8 data4) { u8  data = util::sext(data4, 4); do_nz_flags_8(data);  write_direct8(direct, data); cy(3); }
// ADDS direct, #data4         Add 4-bit signed imm data to mem                                        3 4         1010 S110  0DDD iiii  DDDD DDDD
void xa_cpu::adds_word_direct_data4(u16 direct, u8 data4){ fatalerror("ADDS.w %s, %s\n", get_directtext(direct), show_expanded_data4(data4, 0)); }
void xa_cpu::adds_byte_direct_data4(u16 direct, u8 data4){ u8 directval = read_direct8(direct); u8 data = util::sext(data4, 4); directval += data; do_nz_flags_8(directval); write_direct8(direct, directval); cy(4); }

// CALL rel16                  Relative call (range +/- 64K)                                           3 7/4(PZ)   1100 0101  rrrr rrrr  rrrr rrrr
void xa_cpu::call_rel16(u16 rel16) { if (m_pagezeromode) { push_word_to_stack(m_pc); set_pc_in_current_page(expand_rel16(rel16)); cy(4); } else { cy(7); fatalerror("CALL rel16 not in pagezero mode"); } }

// BCC rel8                    Branch if the carry flag is clear                                       2 6t/3nt    1111 0000  rrrr rrrr
void xa_cpu::bcc_rel8(u8 rel8) { if (get_c_flag() == 0) { set_pc_in_current_page(expand_rel8(rel8)); cy(6); } else { cy(3); } }
// BCS rel8                    Branch if the carry flag is set                                         2 6t/3nt    1111 0001  rrrr rrrr
void xa_cpu::bcs_rel8(u8 rel8) { if (get_c_flag()) { set_pc_in_current_page(expand_rel8(rel8)); cy(6); } else { cy(3); } }
// BNE rel8                    Branch if the zero flag is not set                                      2 6t/3nt    1111 0010  rrrr rrrr
void xa_cpu::bne_rel8(u8 rel8) { if (get_z_flag() == 0) { set_pc_in_current_page(expand_rel8(rel8)); cy(6); } else { cy(3); } }
// BEQ rel8                    Branch if the zero flag is set                                          2 6t/3nt    1111 0011  rrrr rrrr
void xa_cpu::beq_rel8(u8 rel8) { if (get_z_flag()) { set_pc_in_current_page(expand_rel8(rel8)); cy(6); } else { cy(3); } }
// BNV rel8                    Branch if overflow flag is clear                                        2 6t/3nt    1111 0100  rrrr rrrr
void xa_cpu::bnv_rel8(u8 rel8) { fatalerror("BNV %04x\n", expand_rel8(rel8)); }
// BOV rel8                    Branch if overflow flag is set                                          2 6t/3nt    1111 0101  rrrr rrrr
void xa_cpu::bov_rel8(u8 rel8) { fatalerror("BOV %04x\n", expand_rel8(rel8)); }
// BPL rel8                    Branch if the negative flag is clear                                    2 6t/3nt    1111 0110  rrrr rrrr
void xa_cpu::bpl_rel8(u8 rel8) { fatalerror("BPL %04x\n", expand_rel8(rel8)); }
// BMI rel8                    Branch if the negative flag is set                                      2 6t/3nt    1111 0111  rrrr rrrr
void xa_cpu::bmi_rel8(u8 rel8) { fatalerror("BMI %04x\n", expand_rel8(rel8)); }
// BG rel8                     Branch if greater than (unsigned)                                       2 6t/3nt    1111 1000  rrrr rrrr
void xa_cpu::bg_rel8(u8 rel8) { fatalerror("BG %04x\n", expand_rel8(rel8)); }
// BL rel8                     Branch if less than or equal to (unsigned)                              2 6t/3nt    1111 1001  rrrr rrrr
void xa_cpu::bl_rel8(u8 rel8) { fatalerror("BL %04x\n", expand_rel8(rel8)); }
// BGE rel8                    Branch if greater than or equal to (signed)                             2 6t/3nt    1111 1010  rrrr rrrr
void xa_cpu::bge_rel8(u8 rel8) { fatalerror("BGE %04x\n", expand_rel8(rel8)); }
// BLT rel8                    Branch if less than (signed)                                            2 6t/3nt    1111 1011  rrrr rrrr
void xa_cpu::blt_rel8(u8 rel8) { if (get_n_flag() ^ get_v_flag()) { set_pc_in_current_page(expand_rel8(rel8)); cy(6); } else { cy(3); } }
// BGT rel8                    Branch if greater than (signed)                                         2 6t/3nt    1111 1100  rrrr rrrr
void xa_cpu::bgt_rel8(u8 rel8) { if ((get_z_flag() | get_n_flag()) ^ get_v_flag()) { set_pc_in_current_page(expand_rel8(rel8)); cy(6); } else { cy(3); } }
// BLE rel8                    Branch if less than or equal to (signed)                                2 6t/3nt    1111 1101  rrrr rrrr
void xa_cpu::ble_rel8(u8 rel8) { fatalerror("BLE %04x\n", expand_rel8(rel8)); }
// BR rel8                     Short unconditional branch                                              2 6         1111 1110  rrrr rrrr
void xa_cpu::br_rel8(u8 rel8) { set_pc_in_current_page(expand_rel8(rel8)); cy(6); }

// immediate shifts

// ASL Rd, #data4              Logical left shift reg by the 4-bit imm value                           2 a*        1101 SS01  dddd iiii
void xa_cpu::asl_byte_rd_imm4(u8 rd, u8 amount) { fatalerror("ASL.b %s, %d", m_regnames8[rd], amount); }
void xa_cpu::asl_word_rd_imm4(u8 rd, u8 amount) { u16 fullreg = gr16(rd); fullreg = asl32_helper(fullreg, amount); sr16(rd, fullreg); }
// ASL Rd, #data5              Logical left shift reg by the 5-bit imm value                           2 a*        1101 1101  dddi iiii
void xa_cpu::asl_dword_rd_imm5(u8 rd, u8 amount) { u32 fullreg = gr32(rd); fullreg = asl32_helper(fullreg, amount); sr32(rd, fullreg); }

// ASR Rd, #data4              Arithmetic shift right reg by the 4-bit imm count                       2 a*        1101 SS10  dddd iiii
void xa_cpu::asr_byte_rd_imm4(u8 rd, u8 amount) { fatalerror("ASR.b %s, %d", m_regnames8[rd], amount); }
void xa_cpu::asr_word_rd_imm4(u8 rd, u8 amount) { fatalerror("ASR.w %s, %d", m_regnames16[rd], amount); }
// ASR Rd, #data5              Arithmetic shift right reg by the 5-bit imm count                       2 a*        1101 1110  dddi iiii
void xa_cpu::asr_dword_rd_imm5(u8 rd, u8 amount) { fatalerror("ASR.dw %s, %d", m_regnames16[rd], amount); }

// LSR Rd, #data4              Logical right shift reg by the 4-bit imm value                          2 a*        1101 SS00  dddd iiii
void xa_cpu::lsr_byte_rd_imm4(u8 rd, u8 amount) { fatalerror("LSR.b %s, %d", m_regnames8[rd], amount); }
void xa_cpu::lsr_word_rd_imm4(u8 rd, u8 amount) { u16 fullreg = gr16(rd); fullreg = lsr32_helper(fullreg, amount); sr16(rd, fullreg); }
// LSR Rd, #data5              Logical right shift reg by the 4-bit imm value                          2 a*        1101 1100  dddi iiii
void xa_cpu::lsr_dword_rd_imm5(u8 rd, u8 amount) { u32 fullreg = gr32(rd); fullreg = lsr32_helper(fullreg, amount); sr32(rd, fullreg); }

// register form shifts

//ASL Rd, Rs                  Logical left shift dest reg by the value in the src reg                 2 a*        1100 SS01  dddd ssss
void xa_cpu::asl_byte_rd_rs(u8 rd, u8 rs) { fatalerror("ASL.b %s, %d", m_regnames8[rd], m_regnames8[rs]); }
void xa_cpu::asl_word_rd_rs(u8 rd, u8 rs) { fatalerror("ASL.w %s, %d", m_regnames16[rd], m_regnames8[rs]); }
void xa_cpu::asl_dword_rd_rs(u8 rd, u8 rs) { u32 fullreg = gr32(rd); u8 amount = gr8(rs); fullreg = lsr32_helper(fullreg, amount); sr32(rd, fullreg); cy(7); }

// ASR Rd, Rs                  Arithmetic shift right dest reg by the count in the src                 2 a*        1100 SS10  dddd ssss
void xa_cpu::asr_byte_rd_rs(u8 rd, u8 rs) { fatalerror("ASR.b %s, %d", m_regnames8[rd], m_regnames8[rs]); }
void xa_cpu::asr_word_rd_rs(u8 rd, u8 rs) { fatalerror("ASR.w %s, %d", m_regnames16[rd], m_regnames8[rs]); }
void xa_cpu::asr_dword_rd_rs(u8 rd, u8 rs) { fatalerror("ASR.dw %s, %d", m_regnames16[rd], m_regnames8[rs]); }

// LSR Rd, Rs                  Logical right shift dest reg by the value in the src reg                2 a*        1100 SS00  dddd ssss
void xa_cpu::lsr_byte_rd_rs(u8 rd, u8 rs) { fatalerror("LSR.b %s, %d", m_regnames8[rd], m_regnames8[rs]); }
void xa_cpu::lsr_word_rd_rs(u8 rd, u8 rs) { fatalerror("LSR.w %s, %d", m_regnames16[rd], m_regnames8[rs]); }
void xa_cpu::lsr_dword_rd_rs(u8 rd, u8 rs) { fatalerror("LSR.dw %s, %d", m_regnames16[rd], m_regnames8[rs]); }

// NORM Rd, Rs                 Logical shift left dest reg by the value in the src reg until MSB set   2 a*        1100 SS11  dddd ssss
void xa_cpu::norm_byte_rd_rs(u8 rd, u8 rs) { fatalerror("NORM.b %s, %d", m_regnames8[rd], m_regnames8[rs]); }
void xa_cpu::norm_word_rd_rs(u8 rd, u8 rs) { fatalerror("NORM.w %s, %d", m_regnames16[rd], m_regnames8[rs]); }
void xa_cpu::norm_dword_rd_rs(u8 rd, u8 rs) { fatalerror("NORM.dw %s, %d", m_regnames16[rd], m_regnames8[rs]); }


//MULU.b Rd, Rs
void xa_cpu::mulu_byte_rd_rs(u8 rd, u8 rs) { fatalerror( "MULU.b %s, %s", m_regnames8[rd], m_regnames8[rs]); }
//DIVU.b Rd, Rs
void xa_cpu::divu_byte_rd_rs(u8 rd, u8 rs) { fatalerror( "DIVU.b %s, %s", m_regnames8[rd], m_regnames8[rs]); }
//MULU.w Rd, Rs
void xa_cpu::mulu_word_rd_rs(u8 rd, u8 rs) { fatalerror( "MULU.w %s, %s", m_regnames16[rd], m_regnames16[rs]); }
//DIVU.w Rd, Rs
void xa_cpu::divu_word_rd_rs(u8 rd, u8 rs) { fatalerror( "DIVU.w %s, %s", m_regnames16[rd], m_regnames16[rs]); }
// MUL.w Rd, Rs
void xa_cpu::mul_word_rd_rs(u8 rd, u8 rs) { fatalerror( "MUL.w %s, %s", m_regnames16[rd], m_regnames16[rs]); }
// DIV.w Rd, Rs
void xa_cpu::div_word_rd_rs(u8 rd, u8 rs) { fatalerror( "DIV.w %s, %s", m_regnames16[rd], m_regnames16[rs]); }

//DIV.w Rd, #data8
void xa_cpu::div_word_rd_data8(u8 rd, u8 data8) { fatalerror( "DIV.w %s, #$%02x", m_regnames8[rd], data8); }
//DIVU.b Rd, #data8
void xa_cpu::divu_byte_rd_data8(u8 rd, u8 data8) { fatalerror( "DIVU.b %s, #$%02x", m_regnames8[rd], data8); }
//DIVU.w Rd, #data8
void xa_cpu::divu_word_rd_data8(u8 rd, u8 data8) { fatalerror( "DIVU.w %s, #$%02x", m_regnames8[rd], data8); }
//MULU.b Rd, #data8
void xa_cpu::mulu_byte_rd_data8(u8 rd, u8 data8) { fatalerror( "MULU.b %s, #$%02x", m_regnames8[rd], data8); }

//MULU.w Rd, #data16
void xa_cpu::mulu_word_rd_data16(u8 rd, u16 data16) { fatalerror( "MULU.w %s, #$%04x", m_regnames16[rd], data16); }
// DIVU.d Rd, #data16          32X16 unsigned double reg divide w/ imm word                            4 22        1110 1001  ddd0 0001  iiii iiii  iiii iiii
void xa_cpu::divu_dword_rd_data16(u8 rd, u16 data16) { fatalerror( "DIVU.d %s, #$%04x", m_regnames16[rd], data16); }
//MUL.w Rd, #data16
void xa_cpu::mul_word_rd_data16(u8 rd, u16 data16) { fatalerror( "MUL.w %s, #$%04x", m_regnames16[rd], data16); }
//DIV.d Rd, #data16
void xa_cpu::div_dword_rd_data16(u8 rd, u16 data16) { fatalerror( "DIV.d %s, #$%04x", m_regnames16[rd], data16); }

// DIVU.d Rd, Rs               32X16 unsigned double reg divide                                        2 22        1110 1101  ddd0 ssss
void xa_cpu::divu_dword_rd_rs(u8 rd, u8 rs)
{
	u32 fullval = gr32(rd);
	u16 rsval = gr16(rs);

	if (rsval)
	{
		u32 result = fullval / rsval;
		u32 remainder = fullval % rsval;

		if (result & 0xffff0000)
			set_v_flag();
		else
			clear_v_flag();

		// are z and n based on the 32-bit result, or 16 bit result?
		result &= 0xffff;

		if (result == 0x0000)
			set_z_flag();
		else
			clear_n_flag();

		if (result & 0x8000)
			set_n_flag();
		else
			clear_n_flag();

		sr16(rd, result);
		sr16(rd + 1, remainder);
	}
	else
	{
		set_v_flag();
		logerror("%s divide by zero\n", machine().describe_context());
	}

	clear_c_flag();

	cy(22);
}
//DIV.d Rd, Rs
void xa_cpu::div_dword_rd_rs(u8 rd, u8 rs) { fatalerror( "DIV.d %s, %s", m_regnames16[rd], m_regnames16[rs]); }

// CLR bit                     Clear bit                                                               3 4         0000 1000  0000 00bb  bbbb bbbb
void xa_cpu::clr_bit(u16 bit) { set_bit_8_helper(bit, 0); cy(4); }
// SETB bit                    Sets the bit specified                                                  3 4         0000 1000  0001 00bb  bbbb bbbb
void xa_cpu::setb_bit(u16 bit) { set_bit_8_helper(bit, 1); cy(4); }
// MOV C, bit                  Move bit to the carry flag                                              3 4         0000 1000  0010 00bb  bbbb bbbb
void xa_cpu::mov_c_bit(u16 bit) { fatalerror( "MOV C, %s", get_bittext(bit) ); }
// MOV bit, C                  Move carry to bit                                                       3 4         0000 1000  0011 00bb  bbbb bbbb
void xa_cpu::mov_bit_c(u16 bit) { fatalerror( "MOV %s, C", get_bittext(bit) ); }
// ANL C, bit                  Logical AND bit to carry                                                3 4         0000 1000  0100 00bb  bbbb bbbb
void xa_cpu::anl_c_bit(u16 bit) { fatalerror( "ANL C, %s", get_bittext(bit) ); }
// ANL C, /bit                 Logical AND complement of a bit to carry                                3 4         0000 1000  0101 00bb  bbbb bbbb
void xa_cpu::anl_c_notbit(u16 bit) { fatalerror( "ANL C, /%s", get_bittext(bit) ); }
// ORL C, bit                  Logical OR a bit to carry                                               3 4         0000 1000  0110 00bb  bbbb bbbb
void xa_cpu::orl_c_bit(u16 bit) { fatalerror( "ORL C, %s", get_bittext(bit) ); }
// ORL C, /bit                 Logical OR complement of a bit to carry                                 3 4         0000 1000  0111 00bb  bbbb bbbb
void xa_cpu::orl_c_notbit(u16 bit) { fatalerror( "ORL C, /%s", get_bittext(bit) ); }

// LEA Rd, Rs+offset8          Load 16-bit effective address w/ 8-bit offs to reg                      3 3         0100 0000  0ddd 0sss  oooo oooo
void xa_cpu::lea_word_rd_rs_off8(u8 rd, u8 rs, u8 offs8) { fatalerror("LEA %s, %s+#$%02x", m_regnames16[rd], m_regnames16[rs], offs8); }

// LEA Rd, Rs+offset16         Load 16-bit effective address w/ 16-bit offs to reg                     4 3         0100 1000  0ddd 0sss  oooo oooo  oooo oooo
void xa_cpu::lea_word_rd_rs_off16(u8 rd, u8 rs, u16 offs16) { fatalerror( "LEA %s, %s+#$%04x", m_regnames16[rd], m_regnames16[rs], offs16); }

// XCH Rd, [Rs]                Exchange contents of a reg-ind address w/ a reg                         2 6         0101 S000  dddd 0sss
void xa_cpu::xch_word_rd_indrs(u8 rd, u8 rs) { fatalerror("XCH.w %s, [%s]", m_regnames16[rd], m_regnames16[rs]); }
void xa_cpu::xch_byte_rd_indrs(u8 rd, u8 rs) { fatalerror("XCH.b %s, [%s]", m_regnames8[rd], m_regnames16[rs]); }

// XCH Rd, Rs                  Exchange contents of two regs                                           2 5         0110 S000  dddd ssss
void xa_cpu::xch_word_rd_rs(u8 rd, u8 rs) { u16 rdval = gr16(rd); u16 rsval = gr16(rs); sr16(rd, rsval); sr16(rs, rdval); cy(5); }
void xa_cpu::xch_byte_rd_rs(u8 rd, u8 rs) { fatalerror("XCH.b %s, %s", m_regnames8[rd], m_regnames8[rs]); }

// MOVC Rd, [Rs+]              Move data from WS:Rs address of code mem to reg w/ autoinc              2 4         1000 S000  dddd 0sss
void xa_cpu::movc_word_rd_indrsinc(u8 rd, u8 rs) { u16 address = get_addr(rs); u16 data = m_program->read_word(address); address+=2; set_addr(rs, address); do_nz_flags_16(data); sr16(rd, data); cy(4); }
void xa_cpu::movc_byte_rd_indrsinc(u8 rd, u8 rs) { u16 address = get_addr(rs); u8 data = m_program->read_byte(address); address++; set_addr(rs, address); do_nz_flags_8(data); sr8(rd, data); cy(4); }

// DJNZ Rd,rel8                Decrement reg and jump if not zero                                      3 8t/5nt    1000 S111  dddd 1000  rrrr rrrr
void xa_cpu::djnz_word_rd_rel8(u8 rd, u8 rel8) { u16 regval = gr16(rd); regval--; do_nz_flags_16(regval); sr16(rd, regval); if (get_z_flag() == 0) { set_pc_in_current_page(expand_rel8(rel8)); cy(8); } else { cy(5); } }
void xa_cpu::djnz_byte_rd_rel8(u8 rd, u8 rel8) { fatalerror("DJNZ.b %s, $%04x", m_regnames8[rd], expand_rel8(rel8)); }

// POPU direct                 Pop the mem content (b/w) from the user stack                           3 5         1000 S111  0000 0DDD  DDDD DDDD
void xa_cpu::popu_word_direct(u16 direct) { fatalerror("POPU.w %s", get_directtext(direct)); }
void xa_cpu::popu_byte_direct(u16 direct) { fatalerror("POPU.b %s", get_directtext(direct)); }
// POP direct                  Pop the mem content (b/w) from the current stack                        3 5         1000 S111  0001 0DDD  DDDD DDDD
void xa_cpu::pop_word_direct(u16 direct) { fatalerror("POP.w %s", get_directtext(direct)); }
void xa_cpu::pop_byte_direct(u16 direct) { fatalerror("POP.b %s", get_directtext(direct)); }
// PUSHU direct                Push the mem content (b/w) onto the user stack                          3 5         1000 S111  0010 0DDD  DDDD DDDD
void xa_cpu::pushu_word_direct(u16 direct) { fatalerror("PUSHU.w %s", get_directtext(direct)); }
void xa_cpu::pushu_byte_direct(u16 direct) { fatalerror("PUSHU.b %s", get_directtext(direct)); }
// PUSH direct                 Push the mem content (b/w) onto the current stack                       3 5         1000 S111  0011 0DDD  DDDD DDDD
void xa_cpu::push_word_direct(u16 direct) { fatalerror("PUSH.w %s", get_directtext(direct)); }
void xa_cpu::push_byte_direct(u16 direct) { fatalerror("PUSH.b %s", get_directtext(direct)); }

// MOV [Rd+], [Rs+]            Move reg-ind to reg-ind, both pointers autoinc                          2 6         1001 S000  0ddd 0sss
void xa_cpu::mov_word_indrdinc_indrsinc(u8 rd, u8 rs) { fatalerror("MOV.w [%s+], [%s+]", m_regnames16[rd], m_regnames16[rs]); }
void xa_cpu::mov_byte_indrdinc_indrsinc(u8 rd, u8 rs) { fatalerror("MOV.b [%s+], [%s+]", m_regnames16[rd], m_regnames16[rs]); }

// DA Rd                       Decimal Adjust byte reg                                                 2 4         1001 0000  dddd 1000
void xa_cpu::da_rd(u8 rd) { fatalerror( "DA %s", m_regnames8[rd]); }
// SEXT Rd                     Sign extend last operation to reg                                       2 3         1001 S000  dddd 1001
void xa_cpu::sext_word_rd(u8 rd) { if(m_nflag) sr16(rd, 0xffff); else sr16(rd, 0); cy(2); }
void xa_cpu::sext_byte_rd(u8 rd) { if(m_nflag) sr8(rd, 0xff); else sr8(rd, 0); cy(2);  }
// CPL Rd                      Complement (ones complement) reg                                        2 3         1001 S000  dddd 1010
void xa_cpu::cpl_word_rd(u8 rd) { u16 rdval = gr16(rd); u16 result = rdval ^ 0xffff; do_nz_flags_16(result); sr16(rd, result); cy(3); }
void xa_cpu::cpl_byte_rd(u8 rd) { fatalerror("CPL.b %s", m_regnames8[rd]); }
// NEG Rd                      Negate (twos complement) reg                                            2 3         1001 S000  dddd 1011
void xa_cpu::neg_word_rd(u8 rd) { fatalerror("NEG.w %sx", m_regnames16[rd]); }
void xa_cpu::neg_byte_rd(u8 rd) { fatalerror("NEG.b %sx", m_regnames8[rd]); }
// MOVC A, [A+PC]              Move data from code mem to the accumulator ind w/ PC                    2 6         1001 0000  0100 1100
void xa_cpu::movc_a_apc() { fatalerror( "MOVC A, [A+PC]"); }
// MOVC A, [A+DPTR]            Move data from code mem to the accumulator ind w/ DPTR                  2 6         1001 0000  0100 1110
void xa_cpu::movc_a_adptr() { fatalerror( "MOVC A, [A+DPTR]"); }
// MOV Rd, USP                 Move User Stack Pointer to reg (system mode only)                       2 3         1001 0000  dddd 1111
void xa_cpu::mov_rd_usp(u8 rd) { fatalerror( "MOV %s, USP", m_regnames16[rd]); }
// MOV USP, Rs                 Move reg to User Stack Pointer (system mode only)                       2 3         1001 1000  ssss 1111
void xa_cpu::mov_usp_rs(u8 rs) { fatalerror( "MOV USP, %s", m_regnames16[rs]); }

//JB bit,rel8                 Jump if bit set                                                         4 10t/6nt   1001 0111  1000 00bb  bbbb bbbb  rrrr rrrr
void xa_cpu::jb_bit_rel8(u16 bit, u8 rel8) { fatalerror( "JB %s, $%02x", get_bittext(bit), expand_rel8(rel8) ); }
//JNB bit,rel8                Jump if bit not set                                                     4 10t/6nt   1001 0111  1010 00bb  bbbb bbbb  rrrr rrrr
void xa_cpu::jnb_bit_rel8(u16 bit, u8 rel8) { fatalerror( "JNB %s, $%02x", get_bittext(bit), expand_rel8(rel8) ); }
//JBC bit,rel8                Jump if bit set and then clear the bit                                  4 11t/7nt   1001 0111  1100 00bb  bbbb bbbb  rrrr rrrr
void xa_cpu::jbc_bit_rel8(u16 bit, u8 rel8) { fatalerror( "JBC %s, $%02x", get_bittext(bit), expand_rel8(rel8) ); }

// MOV direct, direct          Move mem to mem                                                         4 4         1001 S111  0DDD 0ddd  DDDD DDDD  dddd dddd
void xa_cpu::mov_word_direct_direct(u16 direct_d, u16 direct_s) { fatalerror("MOV.w %s, %s", get_directtext(direct_d), get_directtext(direct_s)); }
void xa_cpu::mov_byte_direct_direct(u16 direct_d, u16 direct_s) { u8 val = read_direct8(direct_s); do_nz_flags_8(val); write_direct8(direct_d, val); cy(4); }

// XCH Rd, direct              Exchange contents of mem w/ a reg                                       3 6         1010 S000  dddd 1DDD  DDDD DDDD
void xa_cpu::xch_word_rd_direct(u8 rd, u16 direct) { fatalerror("XCH.w %s, %s", m_regnames16[rd], get_directtext(direct)); }
void xa_cpu::xch_byte_rd_direct(u8 rd, u16 direct) { fatalerror("XCH.b %s, %s", m_regnames8[rd], get_directtext(direct)); }

// MOV direct, [Rs]            Move reg-ind to mem                                                     3 4         1010 S000  1sss 0DDD  DDDD DDDD
void xa_cpu::mov_word_direct_indrs(u16 direct, u8 rs) { fatalerror("MOV.w %s, [%s]", get_directtext(direct), m_regnames16[rs]); }
void xa_cpu::mov_byte_direct_indrs(u16 direct, u8 rs) { fatalerror("MOV.b %s, [%s]", get_directtext(direct), m_regnames16[rs]); }

// MOV [Rd], direct            Move mem to reg-ind                                                     3 4         1010 S000  0ddd 0DDD  DDDD DDDD
void xa_cpu::mov_word_indrd_direct(u8 rd, u16 direct) { fatalerror("MOV.w [%s], %s", m_regnames16[rd], get_directtext(direct)); }
void xa_cpu::mov_byte_indrd_direct(u8 rd, u16 direct) { fatalerror("MOV.b [%s], %s", m_regnames16[rd], get_directtext(direct)); }

// MOVX [Rd], Rs               Move external data from reg to mem                                      2 6         1010 S111  ssss 1ddd
void xa_cpu::movx_word_indrd_rs(u8 rd, u8 rs) { fatalerror("MOVX.w [%s], %s", m_regnames16[rd], m_regnames16[rs]); }
void xa_cpu::movx_byte_indrd_rs(u8 rd, u8 rs) { fatalerror("MOVX.b [%s], %s", m_regnames16[rd], m_regnames8[rs]); }

// MOVX Rd, [Rs]               Move external data from mem to reg                                      2 6         1010 S111  dddd 0sss
void xa_cpu::movx_word_rd_indrs(u8 rd, u8 rs) { fatalerror("MOVX.w %s, [%s]", m_regnames16[rd], m_regnames16[rs]); }
void xa_cpu::movx_byte_rd_indrs(u8 rd, u8 rs) { fatalerror("MOVX.b %s, [%s]", m_regnames8[rd], m_regnames16[rs]); }

// RR Rd, #data4               Rotate right reg by the 4-bit imm value                                 2 a*        1011 S000  dddd iiii
void xa_cpu::rr_word_rd_data4(u8 rd, u8 data4) { fatalerror("RR.w %s, %d", m_regnames16[rd], data4); }
void xa_cpu::rr_byte_rd_data4(u8 rd, u8 data4) { fatalerror("RR.b %s, %d", m_regnames8[rd], data4); }

// RRC Rd, #data4              Rotate right reg though carry by the 4-bit imm value                    2 a*        1011 S111  dddd iiii
void xa_cpu::rrc_word_rd_data4(u8 rd, u8 data4) { fatalerror("RRC.w %s, %d", m_regnames16[rd], data4); }
void xa_cpu::rrc_byte_rd_data4(u8 rd, u8 data4) { fatalerror("RRC.b %s, %d", m_regnames8[rd], data4); }

// RL Rd, #data4               Rotate left reg by the 4-bit imm value                                  2 a*        1101 S011  dddd iiii
void xa_cpu::rl_word_rd_data4(u8 rd, u8 data4) { fatalerror("RL.w %d, %d", m_regnames16[rd], data4); }
void xa_cpu::rl_byte_rd_data4(u8 rd, u8 data4) { fatalerror("RL.b %d, %d", m_regnames8[rd], data4); }

// RLC Rd, #data4              Rotate left reg though carry by the 4-bit imm value                     2 a*        1101 S111  dddd iiii
void xa_cpu::rlc_word_rd_data4(u8 rd, u8 data4) { fatalerror( "RLC.w Rd, %d", m_regnames16[rd], data4); }
void xa_cpu::rlc_byte_rd_data4(u8 rd, u8 data4) { fatalerror( "RLC.b Rd, %d", m_regnames8[rd], data4); }

// FCALL addr24                Far call (full 24-bit address space)                                    4 12/8(PZ)  1100 0100  aaaa aaaa  AAAA AAAA  AAAA AAAA
void xa_cpu::fcall_addr24(u32 addr24) { fatalerror("FCALL $%06x", addr24); }

// CALL [Rs]                   Subroutine call ind w/ a reg                                            2 8/5(PZ)   1100 0110  0000 0sss
void xa_cpu::call_indrs(u8 rs) { fatalerror("CALL [%s]", m_regnames16[rs]); }

// FJMP addr24                 Far jump (full 24-bit address space)                                    4 6         1101 0100  aaaa aaaa  AAAA AAAA  AAAA AAAA
void xa_cpu::fjmp_addr24(u32 addr24) { fatalerror( "FJMP $%06x", addr24); }

// JMP rel16                   Long unconditional branch                                               3 6         1101 0101  rrrr rrrr  rrrr rrrr
void xa_cpu::jmp_rel16(u16 rel16) { m_pc = expand_rel16(rel16); cy(6); }

// DJNZ direct,rel8            Decrement mem and jump if not zero                                      4 9t/5nt    1110 S010  0000 1DDD  DDDD DDDD  rrrr rrrr
void xa_cpu::djnz_word_direct_rel8(u16 direct, u8 rel8) { fatalerror("DJNZ.w %s, $%04x", get_directtext(direct), expand_rel8(rel8)); }
void xa_cpu::djnz_byte_direct_rel8(u16 direct, u8 rel8) { fatalerror("DJNZ.b %s, $%04x", get_directtext(direct), expand_rel8(rel8)); }

// CJNE Rd,direct,rel8         Compare dir byte to reg and jump if not equal                           4 10t/7nt   1110 S010  dddd 0DDD  DDDD DDDD  rrrr rrrr
void xa_cpu::cjne_word_rd_direct_rel8(u8 rd, u16 direct, u8 rel8) { fatalerror("CJNE.w %s, %s, $%04x", m_regnames16[rd], get_directtext(direct), expand_rel8(rel8)); }
void xa_cpu::cjne_byte_rd_direct_rel8(u8 rd, u16 direct, u8 rel8) { fatalerror("CJNE.b %s, %s, $%04x", m_regnames8[rd], get_directtext(direct), expand_rel8(rel8)); }

// CJNE [Rd],#data8,rel8       Compare imm word to reg-ind and jump if not equal                       4 10t/7nt   1110 0011  0ddd 1000  rrrr rrrr  iiii iiii
void xa_cpu::cjne_indrd_data8_rel8(u8 rd, u8 data8, u8 rel8) { fatalerror( "CJNE [%s], #$%02x, $%04x", m_regnames16[rd], data8, expand_rel8(rel8));}

// CJNE Rd,#data8,rel8         Compare imm byte to reg and jump if not equal                           4 9t/6nt    1110 0011  dddd 0000  rrrr rrrr  iiii iiii
void xa_cpu::cjne_rd_data8_rel8(u8 rd, u8 data8, u8 rel8) { u8 regval = gr8(rd); do_cjne_8_helper(regval, data8); if (!get_z_flag()) { set_pc_in_current_page(expand_rel8(rel8)); cy(9); } else { cy(6); } }

// CJNE [Rd],#data16,rel8      Compare imm word to reg-ind and jump if not equal                       5 10t/7nt   1110 1011  0ddd 1000  rrrr rrrr  iiii iiii  iiii iiii
void xa_cpu::cjne_indrd_data16_rel8(u8 rd, u16 data16, u8 rel8) { fatalerror( "CJNE [%s], #$%04x, $%04x", m_regnames16[rd], data16, expand_rel8(rel8));}

// CJNE Rd,#data16,rel8        Compare imm word to reg and jump if not equal                           5 9t/6nt    1110 1011  dddd 0000  rrrr rrrr  iiii iiii  iiii iiii
void xa_cpu::cjne_rd_data16_rel8(u8 rd, u16 data16, u8 rel8) { fatalerror( "CJNE %s, #$%04x, $%04x", m_regnames8[rd], data16, expand_rel8(rel8)); }

// RESET                       Causes a hardware Reset (same as external Reset)                        2 18        1101 0110  0001 0000
void xa_cpu::reset() { fatalerror( "RESET"); }

// TRAP #data4                 Causes 1 of 16 hardware traps to be executed                            2 23/19(PZ) 1101 0110  0011 tttt
void xa_cpu::trap_data4(u8 data4) { fatalerror( "TRAP %d", data4); }

// JMP [A+DPTR]                Jump ind relative to the DPTR                                           2 5         1101 0110  0100 0110
void xa_cpu::jmp_ind_adptr() { fatalerror( "JMP [A+DPTR]"); }

// JMP [[Rs+]]                 Jump double-ind to the address (pointer to a pointer)                   2 8         1101 0110  0110 0sss
void xa_cpu::jmp_dblindrs(u8 rs) { fatalerror( "JMP [[%s+]]", m_regnames16[rs]); }

// JMP [Rs]                    Jump ind to the address in the reg (64K)                                2 7         1101 0110  0111 0sss
void xa_cpu::jmp_indrs(u8 rs) { fatalerror( "JMP [%s]", m_regnames16[rs]); }

// RET                         Return from subroutine                                                  2 8/6(PZ)   1101 0110  1000 0000
void xa_cpu::ret() {
	if (m_pagezeromode)
	{
		u16 addr = pull_word_from_stack();
		set_pc_in_current_page(addr);
		cy(6);
	}
	else
	{
		cy(8);
		fatalerror("RET not in pagezero mode");
	}
}

// RETI                        Return from interrupt                                                   2 10/8(PZ)  1101 0110  1001 0000
void xa_cpu::reti()
{
	if (m_pagezeromode)
	{
		// probably wrong order
		m_pc = pull_word_from_system_stack();
		m_PSWL = pull_word_from_system_stack();
		m_PSWH = pull_word_from_system_stack();
		sfr_PSWL_w(m_PSWL);
		sfr_PSWH_w(m_PSWH);
		cy(8);
		m_in_interrupt = 0;

	}
	else
	{
		fatalerror("RETI when not in page zero mode\n");
	}
}

// JZ rel8                     Jump if accumulator equals zero                                         2 6t/3nt    1110 1100  rrrr rrrr
void xa_cpu::jz_rel8(u8 rel8) { fatalerror( "JZ $%04x", expand_rel8(rel8)); }

// JNZ rel8                    Jump if accumulator not equal zero                                      2 6t/3nt    1110 1110  rrrr rrrr
void xa_cpu::jnz_rel8(u8 rel8) { fatalerror( "JNZ $%04x", expand_rel8(rel8)); }

// PUSH Rlist                  Push regs (b/w) onto the current stack                                  2 b*        0H00 S111  LLLL LLLL
void xa_cpu::push_word_rlist(u8 bitfield, int h) { cy(2); push_word_reglist(bitfield, h, false); }
void xa_cpu::push_byte_rlist(u8 bitfield, int h) { cy(2); push_byte_reglist(bitfield, h, false); }

// PUSHU Rlist                 Push regs (b/w) from the user stack                                     2 b*        0H01 S111  LLLL LLLL
void xa_cpu::pushu_word_rlist(u8 bitfield, int h) { fatalerror("PUSHU.w %s", get_word_reglist(bitfield)); }
void xa_cpu::pushu_byte_rlist(u8 bitfield, int h) { push_byte_reglist(bitfield, h, true); }

// POP Rlist                   Pop regs (b/w) from the current stack                                   2 c*        0H10 S111  LLLL LLLL
void xa_cpu::pop_word_rlist(u8 bitfield, int h) { cy(2); pull_word_reglist(bitfield, h, false); }
void xa_cpu::pop_byte_rlist(u8 bitfield, int h) { cy(2); pull_byte_reglist(bitfield, h, false); }

// POPU Rlist                  Pop regs (b/w) from the user stack                                      2 c*        0H11 S111  LLLL LLLL
void xa_cpu::popu_word_rlist(u8 bitfield, int h) { fatalerror("POPU.w %s", get_word_reglist(bitfield)); }
void xa_cpu::popu_byte_rlist(u8 bitfield, int h) { cy(2); pull_byte_reglist(bitfield, h, true); }

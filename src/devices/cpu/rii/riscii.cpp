// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    ELAN Microelectronics RISC II (RII) Series

    Architecture is very similar to the GI/Microchip PIC series, with
    16-bit opcodes and a banked 8-bit register file with special registers
    for indirect access. (It has no relation to Berkeley RISC II. Elan's
    first generation of PIC-like microcontrollers, the EM78 series, has
    13-bit opcodes.)

    Currently the execution core is mostly complete, though interrupts and
    on-chip peripherals are completely unemulated.

***************************************************************************/

#include "emu.h"
#include "riscii.h"
#include "riidasm.h"

// device type definitions
DEFINE_DEVICE_TYPE(RISCII, riscii_series_device, "riscii", "Elan RISC II")

ALLOW_SAVE_TYPE(riscii_series_device::exec_state);


//**************************************************************************
//  DEVICE CONSTRUCTION AND INITIALIZATION
//**************************************************************************

void riscii_series_device::regs_map(address_map &map)
{
	// 0x00 (INDF0) is not physically implemented
	map(0x0001, 0x0001).rw(FUNC(riscii_series_device::fsr0_r), FUNC(riscii_series_device::fsr0_w));
	map(0x0002, 0x0002).rw(FUNC(riscii_series_device::pcl_r), FUNC(riscii_series_device::pcl_w));
	map(0x0003, 0x0003).rw(FUNC(riscii_series_device::pcm_r), FUNC(riscii_series_device::pcm_w));
	map(0x0004, 0x0004).rw(FUNC(riscii_series_device::pch_r), FUNC(riscii_series_device::pch_w));
	map(0x0005, 0x0005).rw(FUNC(riscii_series_device::bsr_r), FUNC(riscii_series_device::bsr_w));
	map(0x0006, 0x0006).rw(FUNC(riscii_series_device::stkptr_r), FUNC(riscii_series_device::stkptr_w));
	map(0x0007, 0x0007).rw(FUNC(riscii_series_device::bsr1_r), FUNC(riscii_series_device::bsr1_w));
	// 0x08 (INDF1) is not physically implemented
	map(0x0009, 0x0009).rw(FUNC(riscii_series_device::fsr1_r), FUNC(riscii_series_device::fsr1_w));
	map(0x000a, 0x000a).rw(FUNC(riscii_series_device::acc_r), FUNC(riscii_series_device::acc_w));
	map(0x000b, 0x000b).rw(FUNC(riscii_series_device::tabptrl_r), FUNC(riscii_series_device::tabptrl_w));
	map(0x000c, 0x000c).rw(FUNC(riscii_series_device::tabptrm_r), FUNC(riscii_series_device::tabptrm_w));
	map(0x000d, 0x000d).rw(FUNC(riscii_series_device::tabptrh_r), FUNC(riscii_series_device::tabptrh_w));
	map(0x000e, 0x000e).rw(FUNC(riscii_series_device::cpucon_r), FUNC(riscii_series_device::cpucon_w));
	map(0x000f, 0x000f).rw(FUNC(riscii_series_device::status_r), FUNC(riscii_series_device::status_w));
	map(0x0010, 0x0010).ram(); // TODO: TRL2
	map(0x0011, 0x0011).rw(FUNC(riscii_series_device::prodl_r), FUNC(riscii_series_device::prodl_w));
	map(0x0012, 0x0012).rw(FUNC(riscii_series_device::prodh_r), FUNC(riscii_series_device::prodh_w));
	map(0x0013, 0x0029).ram(); // TODO: other special function registers
	map(0x002b, 0x002b).rw(FUNC(riscii_series_device::post_id_r), FUNC(riscii_series_device::post_id_w));
	map(0x002c, 0x007f).ram(); // TODO: other special function registers
	for (unsigned b = 0; b <= m_maxbank; b++)
		map(0x0080 | (b << 8), 0x00ff | (b << 8)).ram();
}

std::unique_ptr<util::disasm_interface> riscii_series_device::create_disassembler()
{
	return std::make_unique<riscii_disassembler>();
}

riscii_series_device::riscii_series_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, unsigned prgbits, unsigned bankbits, uint8_t maxbank)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, prgbits, -1)
	, m_regs_config("register", ENDIANNESS_LITTLE, 8, 8 + bankbits, 0, address_map_constructor(FUNC(riscii_series_device::regs_map), this))
	, m_program(nullptr)
	, m_regs(nullptr)
	, m_cache(nullptr)
	, m_prgbits(prgbits)
	, m_bankmask((1 << bankbits) - 1)
	, m_maxbank(maxbank)
	, m_pc(0)
	, m_acc(0)
	, m_fsr{0, 0}
	, m_bsr{0, 0}
	, m_tabptr(0)
	, m_stkptr(0)
	, m_cpucon(0)
	, m_status(0)
	, m_prod(0)
	, m_post_id(0)
	, m_icount(0)
	, m_exec_state(EXEC_CYCLE1)
	, m_repeat(0)
	, m_curreg(0)
{
}

riscii_series_device::riscii_series_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: riscii_series_device(mconfig, RISCII, tag, owner, clock, 18, 5, 0x1f)
{
}

device_memory_interface::space_config_vector riscii_series_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_regs_config)
	};
}

void riscii_series_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_regs = &space(AS_DATA);
	m_cache = m_program->cache<1, -1, ENDIANNESS_LITTLE>();

	set_icountptr(m_icount);

	state_add(RII_PC, "PC", m_pc).mask((1 << m_prgbits) - 1);
	state_add(STATE_GENPC, "GENPC", m_pc).mask((1 << m_prgbits) - 1).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_ppc).mask((1 << m_prgbits) - 1).noshow();
	state_add(RII_REPEAT, "REPEAT", m_repeat);
	state_add(RII_ACC, "ACC", m_acc);
	state_add(RII_BSR, "BSR", m_bsr[0]).mask(m_bankmask);
	state_add(RII_FSR0, "FSR0", m_fsr[0]);
	state_add(RII_BSR1, "BSR1", m_bsr[1]).mask(m_bankmask);
	state_add(RII_FSR1, "FSR1", m_fsr[1]); // TODO: high bit forced to 1
	state_add(RII_TABPTR, "TABPTR", m_tabptr).mask(0x800000 + (1 << (m_prgbits + 1)) - 1);
	state_add(RII_STKPTR, "STKPTR", m_stkptr);
	state_add(RII_CPUCON, "CPUCON", m_cpucon).mask(0x9f);
	state_add(RII_STATUS, "STATUS", m_status);
	state_add(STATE_GENFLAGS, "CURFLAGS", m_status).noshow().formatstr("%8s");
	state_add(RII_PROD, "PROD", m_prod);
	state_add(RII_POST_ID, "POST_ID", m_post_id);

	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_acc));
	save_item(NAME(m_bsr));
	save_item(NAME(m_fsr));
	save_item(NAME(m_tabptr));
	save_item(NAME(m_stkptr));
	save_item(NAME(m_cpucon));
	save_item(NAME(m_status));
	save_item(NAME(m_prod));
	save_item(NAME(m_post_id));
	save_item(NAME(m_exec_state));
	save_item(NAME(m_repeat));
	save_item(NAME(m_curreg));
}

void riscii_series_device::device_reset()
{
	m_pc = m_ppc = 0x00000;
	m_fsr[0] = 0x00;
	m_bsr[0] = 0x00;
	m_fsr[1] = 0x80;
	m_bsr[1] = 0x00;
	m_tabptr = 0x000000;
	m_stkptr = 0x00;
	m_cpucon &= 0x01;
	m_status |= 0xc0;
	m_post_id = 0xf0;
	m_exec_state = EXEC_CYCLE1;
	m_repeat = 0x00;
}


//**************************************************************************
//  REGISTER HANDLERS
//**************************************************************************

u8 riscii_series_device::fsr0_r()
{
	return m_fsr[0];
}

void riscii_series_device::fsr0_w(u8 data)
{
	m_fsr[0] = data;
}

u8 riscii_series_device::bsr_r()
{
	return m_bsr[0];
}

void riscii_series_device::bsr_w(u8 data)
{
	m_bsr[0] = data & m_bankmask;
}

u8 riscii_series_device::fsr1_r()
{
	return m_fsr[1];
}

void riscii_series_device::fsr1_w(u8 data)
{
	m_fsr[1] = data | 0x80;
}

u8 riscii_series_device::bsr1_r()
{
	return m_bsr[1];
}

void riscii_series_device::bsr1_w(u8 data)
{
	m_bsr[1] = data & m_bankmask;
}

u8 riscii_series_device::pcl_r()
{
	return m_pc & 0x000ff;
}

void riscii_series_device::pcl_w(u8 data)
{
	m_pc = (m_pc & 0xfff00) | data;
}

u8 riscii_series_device::pcm_r()
{
	return (m_pc & 0x0ff00) >> 8;
}

void riscii_series_device::pcm_w(u8 data)
{
	m_pc = (m_pc & 0xf00ff) | u32(data) << 8;
}

u8 riscii_series_device::pch_r()
{
	return (m_pc & 0xf0000) >> 16;
}

void riscii_series_device::pch_w(u8 data)
{
	if (m_prgbits > 16)
		m_pc = (m_pc & 0x0ffff) | u32(data & ((1 << (m_prgbits - 16)) - 1)) << 16;
}

u8 riscii_series_device::tabptrl_r()
{
	return m_tabptr & 0x0000ff;
}

void riscii_series_device::tabptrl_w(u8 data)
{
	m_tabptr = (m_tabptr & 0xffff00) | data;
}

u8 riscii_series_device::tabptrm_r()
{
	return (m_tabptr & 0x00ff00) >> 8;
}

void riscii_series_device::tabptrm_w(u8 data)
{
	m_tabptr = (m_tabptr & 0xff00ff) | u32(data) << 8;
}

u8 riscii_series_device::tabptrh_r()
{
	return (m_tabptr & 0xff0000) >> 16;
}

void riscii_series_device::tabptrh_w(u8 data)
{
	m_tabptr = (m_tabptr & 0x00ffff) | u32(data & (0x80 | ((1 << (m_prgbits - 15)) - 1))) << 16;
}

u8 riscii_series_device::acc_r()
{
	return m_acc;
}

void riscii_series_device::acc_w(u8 data)
{
	m_acc = data;
}

u8 riscii_series_device::stkptr_r()
{
	return m_stkptr;
}

void riscii_series_device::stkptr_w(u8 data)
{
	m_stkptr = data;
}

u8 riscii_series_device::cpucon_r()
{
	return m_cpucon;
}

void riscii_series_device::cpucon_w(u8 data)
{
	m_cpucon = data & 0x9f;
}

u8 riscii_series_device::status_r()
{
	return m_status;
}

void riscii_series_device::status_w(u8 data)
{
	m_status = (m_status & 0xc0) | (data & 0x3f);
}

u8 riscii_series_device::prodl_r()
{
	return m_prod & 0x00ff;
}

void riscii_series_device::prodl_w(u8 data)
{
	m_prod = (m_prod & 0xff00) | data;
}

u8 riscii_series_device::prodh_r()
{
	return m_prod >> 8;
}

void riscii_series_device::prodh_w(u8 data)
{
	m_prod = (m_prod & 0x00ff) | u16(data) << 8;
}

u8 riscii_series_device::post_id_r()
{
	return m_post_id;
}

void riscii_series_device::post_id_w(u8 data)
{
	m_post_id = data;
}


//**************************************************************************
//  MEMORY HELPERS
//**************************************************************************

u16 riscii_series_device::get_banked_address(u8 reg)
{
	if (reg == 0x00)
	{
		// INDF0 address comes from BSR and FSR0
		u16 bfsr0 = u16(m_bsr[0]) << 8 | m_fsr[0];
		if (BIT(m_post_id, 0))
		{
			// Auto increment/decrement (no carry into BSR)
			if (BIT(m_post_id, 4))
				++m_fsr[0];
			else
				--m_fsr[0];
		}
		return bfsr0;
	}
	else if (reg == 0x08)
	{
		// INDF1 address comes from BSR1 and FSR1
		u16 bfsr1 = u16(m_bsr[1]) << 8 | m_fsr[1];
		if (BIT(m_post_id, 1))
		{
			// Auto increment/decrement (carry into BSR1)
			if (BIT(m_post_id, 5))
			{
				m_fsr[1] = (m_fsr[1] + 1) | 0x80;
				if (m_fsr[1] == 0x80)
					++m_bsr[1];
			}
			else
			{
				m_fsr[1] = (m_fsr[1] - 1) | 0x80;
				if (m_fsr[1] == 0xff)
					--m_bsr[1];
			}
		}
		return bfsr1;
	}
	else if (reg >= 0x80)
		return u16(m_bsr[0]) << 8 | reg;
	else
		return reg;
}

u32 riscii_series_device::tabptr_offset(int offset) const
{
	return (m_tabptr & 0x800000) | ((m_tabptr + offset) & ((1 << (m_prgbits + 1)) - 1));
}


//**************************************************************************
//  EXECUTION CORE
//**************************************************************************

void riscii_series_device::execute_move(u8 dstreg, u8 srcreg)
{
	u8 tmp = m_regs->read_byte(get_banked_address(srcreg));
	m_regs->write_byte(get_banked_address(dstreg), tmp);
}

void riscii_series_device::execute_add(u8 reg, bool a, bool c)
{
	u16 addr = get_banked_address(reg);
	s8 data = m_regs->read_byte(addr);
	s16 tmp = s16(data) + s8(m_acc) + (c ? m_status & 0x01 : 0);
	bool dc = (data & 0x0f) + (m_acc & 0x0f) + (c ? m_status & 0x01 : 0) >= 0x10;
	if (a)
		acc_w(tmp & 0xff);
	else
		m_regs->write_byte(addr, tmp & 0xff);
	m_status = (m_status & 0xc0)
		| (BIT(tmp, 8) ? 0x01 : 0x00)
		| (dc ? 0x02 : 0x00)
		| ((tmp & 0xff) == 0 ? 0x04 : 0x00)
		| ((tmp < 0) != BIT(tmp, 7) ? 0x08 : 0x00)
		| (tmp <= 0 ? 0x10 : 0x00)
		| (tmp >= 0 ? 0x20 : 0x00);
}

void riscii_series_device::execute_sub(u8 reg, bool a, bool c)
{
	u16 addr = get_banked_address(reg);
	s8 data = m_regs->read_byte(addr);
	s16 tmp = s16(data) - s8(m_acc) - (c ? ~m_status & 0x01 : 0);
	bool dc = (data & 0x0f) >= (m_acc & 0x0f) + (c ? ~m_status & 0x01 : 0);
	if (a)
		acc_w(tmp & 0xff);
	else
		m_regs->write_byte(addr, tmp & 0xff);
	m_status = (m_status & 0xc0)
		| (BIT(tmp, 8) ? 0x00 : 0x01) // borrow is inverted
		| (dc ? 0x02 : 0x00)
		| ((tmp & 0xff) == 0 ? 0x04 : 0x00)
		| ((tmp < 0) != BIT(tmp, 7) ? 0x08 : 0x00)
		| (tmp <= 0 ? 0x10 : 0x00)
		| (tmp >= 0 ? 0x20 : 0x00);
}

void riscii_series_device::execute_add_imm(u8 data, bool c)
{
	s16 tmp = s16(s8(data)) + s8(m_acc) + (c ? m_status & 0x01 : 0);
	bool dc = (data & 0x0f) + (m_acc & 0x0f) + (c ? m_status & 0x01 : 0) >= 0x10;
	acc_w(tmp & 0xff);
	m_status = (m_status & 0xc0)
		| (BIT(tmp, 8) ? 0x01 : 0x00)
		| (dc ? 0x02 : 0x00)
		| ((tmp & 0xff) == 0 ? 0x04 : 0x00)
		| ((tmp < 0) != BIT(tmp, 7) ? 0x08 : 0x00)
		| (tmp <= 0 ? 0x10 : 0x00)
		| (tmp >= 0 ? 0x20 : 0x00);
}

void riscii_series_device::execute_sub_imm(u8 data, bool c)
{
	s16 tmp = s16(s8(data)) - s8(m_acc) - (c ? ~m_status & 0x01 : 0);
	bool dc = (data & 0x0f) + (m_acc & 0x0f) + (c ? ~m_status & 0x01 : 0) >= 0x10;
	acc_w(tmp & 0xff);
	m_status = (m_status & 0xc0)
		| (BIT(tmp, 8) ? 0x00 : 0x01) // borrow is inverted
		| (dc ? 0x02 : 0x00)
		| ((tmp & 0xff) == 0 ? 0x04 : 0x00)
		| ((tmp < 0) != BIT(tmp, 7) ? 0x08 : 0x00)
		| (tmp <= 0 ? 0x10 : 0x00)
		| (tmp >= 0 ? 0x20 : 0x00);
}

void riscii_series_device::execute_adddc(u8 reg, bool a)
{
	u16 addr = get_banked_address(reg);
	u8 data = m_regs->read_byte(addr);
	u16 tmp = u16(data) + m_acc + (m_status & 0x01);
	bool dc = (data & 0x0f) + (m_acc & 0x0f) + (m_status & 0x01) >= 0x0a;
	if (dc)
		tmp += 0x06;
	if ((tmp & 0x1ff) >= 0xa0)
		tmp += 0x60;
	if (a)
		acc_w(tmp & 0xff);
	else
		m_regs->write_byte(addr, tmp & 0xff);
	m_status = (m_status & 0xf8)
		| (BIT(tmp, 8) ? 0x01 : 0x00)
		| (dc ? 0x02 : 0x00)
		| ((tmp & 0xff) == 0 ? 0x04 : 0x00);
}

void riscii_series_device::execute_subdb(u8 reg, bool a)
{
	u16 addr = get_banked_address(reg);
	u8 data = m_regs->read_byte(addr);
	u16 tmp = u16(data) - m_acc - (~m_status & 0x01);
	bool dc = (data & 0x0f) + (~m_acc & 0x0f) + (m_status & 0x01) >= 0x0a;
	if (dc)
		tmp -= 0x06;
	if ((tmp & 0x1ff) >= 0xa0)
		tmp -= 0x60;
	if (a)
		acc_w(tmp & 0xff);
	else
		m_regs->write_byte(addr, tmp & 0xff);
	m_status = (m_status & 0xf8)
		| (BIT(tmp, 8) ? 0x00 : 0x01) // borrow is inverted
		| (dc ? 0x02 : 0x00)
		| ((tmp & 0xff) == 0 ? 0x04 : 0x00);
}

void riscii_series_device::execute_mul(u8 reg)
{
	m_prod = u16(m_acc) * m_regs->read_byte(get_banked_address(reg));
}

void riscii_series_device::execute_mul_imm(u8 data)
{
	m_prod = u16(m_acc) * data;
}

void riscii_series_device::execute_or(u8 reg, bool a)
{
	u16 addr = get_banked_address(reg);
	u8 tmp = m_acc | m_regs->read_byte(addr);
	if (a)
		acc_w(tmp);
	else
		m_regs->write_byte(addr, tmp);
	if (tmp == 0)
		m_status |= 0x04;
	else
		m_status &= 0xfb;
}

void riscii_series_device::execute_and(u8 reg, bool a)
{
	u16 addr = get_banked_address(reg);
	u8 tmp = m_acc & m_regs->read_byte(addr);
	if (a)
		acc_w(tmp);
	else
		m_regs->write_byte(addr, tmp);
	if (tmp == 0)
		m_status |= 0x04;
	else
		m_status &= 0xfb;
}

void riscii_series_device::execute_xor(u8 reg, bool a)
{
	u16 addr = get_banked_address(reg);
	u8 tmp = m_acc ^ m_regs->read_byte(addr);
	if (a)
		acc_w(tmp);
	else
		m_regs->write_byte(addr, tmp);
	if (tmp == 0)
		m_status |= 0x04;
	else
		m_status &= 0xfb;
}

void riscii_series_device::execute_com(u8 reg, bool a)
{
	u16 addr = get_banked_address(reg);
	u8 tmp = ~m_regs->read_byte(addr);
	if (a)
		acc_w(tmp);
	else
		m_regs->write_byte(addr, tmp);
	if (tmp == 0)
		m_status |= 0x04;
	else
		m_status &= 0xfb;
}

void riscii_series_device::execute_clr(u8 reg)
{
	m_regs->write_byte(get_banked_address(reg), 0);
	m_status |= 0x04;
}

void riscii_series_device::execute_rrc(u8 reg, bool a)
{
	u16 addr = get_banked_address(reg);
	u16 tmp = m_regs->read_byte(addr) | u16(m_status & 0x01) << 8;
	if (a)
		acc_w(tmp >> 1);
	else
		m_regs->write_byte(addr, tmp >> 1);
	m_status = (m_status & 0xfe) | (tmp & 0x01);
}

void riscii_series_device::execute_rlc(u8 reg, bool a)
{
	u16 addr = get_banked_address(reg);
	u16 tmp = u16(m_regs->read_byte(addr)) << 1 | (m_status & 0x01);
	if (a)
		acc_w(tmp & 0xff);
	else
		m_regs->write_byte(addr, tmp & 0xff);
	m_status = (m_status & 0xfe) | (tmp >> 8);
}

void riscii_series_device::execute_shra(u8 reg)
{
	u8 tmp = m_regs->read_byte(get_banked_address(reg));
	acc_w((tmp >> 1) | (m_status & 0x01) << 7);
}

void riscii_series_device::execute_shla(u8 reg)
{
	u8 tmp = m_regs->read_byte(get_banked_address(reg));
	acc_w((tmp << 1) | (m_status & 0x01));
}

void riscii_series_device::execute_jump(u32 addr)
{
	m_pc = addr;
}

void riscii_series_device::execute_call(u32 addr)
{
	m_stkptr -= 2;
	u16 stkaddr = u16(m_maxbank - (BIT(m_stkptr, 7) ? 0 : 1)) << 8 | 0x80 | (m_stkptr & 0x7e);
	m_regs->write_word(stkaddr, swapendian_int16(m_pc & 0xffff));
	execute_jump(addr);
}

void riscii_series_device::execute_jcc(bool condition)
{
	if (condition)
		m_exec_state = static_cast<exec_state>(EXEC_L0JMP + (m_pc >> 16));
	else
		m_exec_state = EXEC_NOJMP;
}

void riscii_series_device::execute_jdnz(u8 reg, bool a)
{
	u16 addr = get_banked_address(reg);
	u8 tmp = m_regs->read_byte(addr) - 1;
	if (a)
		acc_w(tmp);
	else
		m_regs->write_byte(addr, tmp);
	execute_jcc(tmp != 0);
}

void riscii_series_device::execute_jinz(u8 reg, bool a)
{
	u16 addr = get_banked_address(reg);
	u8 tmp = m_regs->read_byte(addr) + 1;
	if (a)
		acc_w(tmp);
	else
		m_regs->write_byte(addr, tmp);
	execute_jcc(tmp != 0);
}

void riscii_series_device::set_z_acc(u8 tmp)
{
	acc_w(tmp);
	if (tmp == 0)
		m_status |= 0x04;
	else
		m_status &= 0xfb;
}

void riscii_series_device::execute_load(u8 reg)
{
	set_z_acc(m_regs->read_byte(get_banked_address(reg)));
}

void riscii_series_device::execute_store(u8 reg)
{
	m_regs->write_byte(get_banked_address(reg), m_acc);
}

void riscii_series_device::execute_test(u8 reg)
{
	u8 tmp = m_regs->read_byte(get_banked_address(reg));
	if (tmp == 0)
		m_status |= 0x04;
	else
		m_status &= 0xfb;
}

void riscii_series_device::execute_swap(u8 reg, bool a)
{
	u16 addr = get_banked_address(reg);
	u8 tmp = m_regs->read_byte(addr);
	if (a)
		acc_w((tmp >> 4) | (tmp << 4));
	else
		m_regs->write_byte(addr, (tmp >> 4) | (tmp << 4));
}

void riscii_series_device::execute_jbc(u8 reg, int b)
{
	execute_jcc(!BIT(m_regs->read_byte(get_banked_address(reg)), b));
}

void riscii_series_device::execute_jbs(u8 reg, int b)
{
	execute_jcc(BIT(m_regs->read_byte(get_banked_address(reg)), b));
}

void riscii_series_device::execute_bc(u8 reg, int b)
{
	u16 addr = get_banked_address(reg);
	u8 tmp = m_regs->read_byte(addr) & ~(1 << b);
	m_regs->write_byte(addr, tmp);
}

void riscii_series_device::execute_bs(u8 reg, int b)
{
	u16 addr = get_banked_address(reg);
	u8 tmp = m_regs->read_byte(addr) | (1 << b);
	m_regs->write_byte(addr, tmp);
}

void riscii_series_device::execute_btg(u8 reg, int b)
{
	u16 addr = get_banked_address(reg);
	u8 tmp = m_regs->read_byte(addr) ^ (1 << b);
	m_regs->write_byte(addr, tmp);
}

void riscii_series_device::execute_inc(u8 reg, bool a)
{
	u16 addr = get_banked_address(reg);
	u16 tmp = u16(m_regs->read_byte(addr)) + 1;
	if (a)
		acc_w(tmp & 0xff);
	else
		m_regs->write_byte(addr, tmp & 0xff);
	m_status = (m_status & 0xfa) | ((tmp & 0xff) == 0 ? 0x04 : 0x00) | (tmp >> 8);
}

void riscii_series_device::execute_dec(u8 reg, bool a)
{
	u16 addr = get_banked_address(reg);
	u16 tmp = (u16(m_regs->read_byte(addr)) - 1) & 0x1ff;
	if (a)
		acc_w(tmp & 0xff);
	else
		m_regs->write_byte(addr, tmp & 0xff);
	m_status = (m_status & 0xfa) | ((tmp & 0xff) == 0 ? 0x04 : 0x00) | (tmp >> 8);
}

void riscii_series_device::execute_rpt(u8 reg)
{
	m_repeat = m_regs->read_byte(get_banked_address(reg)) - 1;
}

void riscii_series_device::execute_ret(bool inte)
{
	u16 stkaddr = u16(m_maxbank - (BIT(m_stkptr, 7) ? 0 : 1)) << 8 | 0x80 | (m_stkptr & 0x7e);
	execute_jump((m_pc & 0xf0000) | swapendian_int16(m_regs->read_word(stkaddr)));
	m_stkptr += 2;
	if (inte)
		m_cpucon |= 0x04;
}

void riscii_series_device::execute_wdtc()
{
	logerror("WDTC (PC = %05X)\n", m_ppc);
}

void riscii_series_device::execute_slep()
{
	logerror("SLEP (PC = %05X)\n", m_ppc);
}

void riscii_series_device::execute_undef(u16 opcode)
{
	logerror("Undefined opcode %04Xh encountered (PC = %05X)\n", opcode, m_ppc);
}

void riscii_series_device::execute_cycle1(u16 opcode)
{
	if (BIT(opcode, 15))
	{
		if (BIT(opcode, 14))
		{
			if (BIT(opcode, 13))
				execute_call((m_pc & 0x3e000) | (opcode & 0x1fff));
			else
				execute_jump((m_pc & 0x3e000) | (opcode & 0x1fff));
		}
		else
		{
			if (BIT(opcode, 13))
				execute_move(opcode & 0x00ff, (opcode & 0x1f00) >> 8);
			else
				execute_move((opcode & 0x1f00) >> 8, opcode & 0x00ff);
		}
	}
	else switch (opcode & 0xff00)
	{
	case 0x0000:
		if (opcode == 0x0001)
			execute_wdtc();
		else if (opcode == 0x0002)
			execute_slep();
		else if ((opcode & 0x00e0) == 0x0020)
			m_exec_state = static_cast<exec_state>((BIT(opcode, 4) ? EXEC_L0CALL : EXEC_L0JMP) + (opcode & 0x000f));
		else if (opcode != 0x0000) // NOP
			execute_undef(opcode);
		break;

	case 0x0200: case 0x0300:
		execute_or(opcode & 0x00ff, !BIT(opcode, 8));
		break;

	case 0x0400: case 0x0500:
		execute_and(opcode & 0x00ff, !BIT(opcode, 8));
		break;

	case 0x0600: case 0x0700:
		execute_xor(opcode & 0x00ff, !BIT(opcode, 8));
		break;

	case 0x0800: case 0x0900:
		execute_com(opcode & 0x00ff, !BIT(opcode, 8));
		break;

	case 0x0a00: case 0x0b00:
		execute_rrc(opcode & 0x00ff, !BIT(opcode, 8));
		break;

	case 0x0c00: case 0x0d00:
		execute_rlc(opcode & 0x00ff, !BIT(opcode, 8));
		break;

	case 0x0e00: case 0x0f00:
		execute_swap(opcode & 0x00ff, !BIT(opcode, 8));
		break;

	case 0x1000: case 0x1100:
		execute_add(opcode & 0x00ff, !BIT(opcode, 8), false);
		break;

	case 0x1200: case 0x1300:
		execute_add(opcode & 0x00ff, !BIT(opcode, 8), true);
		break;

	case 0x1400: case 0x1500:
		execute_adddc(opcode & 0x00ff, !BIT(opcode, 8));
		break;

	case 0x1600: case 0x1700:
		execute_sub(opcode & 0x00ff, !BIT(opcode, 8), false);
		break;

	case 0x1800: case 0x1900:
		execute_sub(opcode & 0x00ff, !BIT(opcode, 8), true);
		break;

	case 0x1a00: case 0x1b00:
		execute_subdb(opcode & 0x00ff, !BIT(opcode, 8));
		break;

	case 0x1c00: case 0x1d00:
		execute_inc(opcode & 0x00ff, !BIT(opcode, 8));
		break;

	case 0x1e00: case 0x1f00:
		execute_dec(opcode & 0x00ff, !BIT(opcode, 8));
		break;

	case 0x2000:
		execute_load(opcode & 0x00ff);
		break;

	case 0x2100:
		execute_store(opcode & 0x00ff);
		break;

	case 0x2200:
		execute_shra(opcode & 0x00ff);
		break;

	case 0x2300:
		execute_shla(opcode & 0x00ff);
		break;

	case 0x2400:
		execute_clr(opcode & 0x00ff);
		break;

	case 0x2500:
		execute_test(opcode & 0x00ff);
		break;

	case 0x2600:
		execute_mul(opcode & 0x00ff);
		break;

	case 0x2700:
		execute_rpt(opcode & 0x00ff);
		break;

	case 0x2b00:
		if ((opcode & 0x00fe) == 0x00fe)
			execute_ret(BIT(opcode, 0));
		else
			execute_undef(opcode);
		break;

	case 0x2c00:
		m_curreg = opcode & 0x00ff;
		m_exec_state = EXEC_TBRD0;
		break;

	case 0x2d00:
		m_curreg = opcode & 0x00ff;
		m_exec_state = EXEC_TBRD1;
		break;

	case 0x2e00:
		m_curreg = opcode & 0x00ff;
		m_exec_state = EXEC_TBRD2;
		break;

	case 0x2f00:
		m_curreg = opcode & 0x00ff;
		m_exec_state = EXEC_TBRDA;
		break;

	case 0x3000: case 0x3100: case 0x3200: case 0x3300:
	case 0x3400: case 0x3500: case 0x3600: case 0x3700:
	case 0x3800: case 0x3900: case 0x3a00: case 0x3b00:
	case 0x3c00: case 0x3d00: case 0x3e00: case 0x3f00:
		execute_call(opcode & 0x0fff);
		break;

	case 0x4000:
		tabptrl_w(opcode & 0x00ff);
		break;

	case 0x4100:
		tabptrm_w(opcode & 0x00ff);
		break;

	case 0x4200:
		tabptrh_w(opcode & 0x00ff);
		break;

	case 0x4300:
		bsr_w(opcode & 0x00ff);
		break;

	case 0x4400:
		set_z_acc(m_acc | (opcode & 0x00ff));
		break;

	case 0x4500:
		set_z_acc(m_acc & opcode & 0x00ff);
		break;

	case 0x4600:
		set_z_acc(m_acc ^ (opcode & 0x00ff));
		break;

	case 0x4700:
		execute_jcc(m_acc >= (opcode & 0x00ff));
		break;

	case 0x4800:
		execute_jcc(m_acc <= (opcode & 0x00ff));
		break;

	case 0x4900:
		execute_jcc(m_acc == (opcode & 0x00ff));
		break;

	case 0x4a00:
		execute_add_imm(opcode & 0x00ff, false);
		break;

	case 0x4b00:
		execute_add_imm(opcode & 0x00ff, true);
		break;

	case 0x4c00:
		execute_sub_imm(opcode & 0x00ff, false);
		break;

	case 0x4d00:
		execute_sub_imm(opcode & 0x00ff, true);
		break;

	case 0x4e00:
		acc_w(opcode & 0x00ff);
		break;

	case 0x4f00:
		execute_mul_imm(opcode & 0x00ff);
		break;

	case 0x5000:
		execute_jdnz(opcode & 0x00ff, true);
		break;

	case 0x5100:
		execute_jdnz(opcode & 0x00ff, false);
		break;

	case 0x5200:
		execute_jinz(opcode & 0x00ff, true);
		break;

	case 0x5300:
		execute_jinz(opcode & 0x00ff, false);
		break;

	case 0x5500:
		execute_jcc(m_acc >= m_regs->read_byte(get_banked_address(opcode & 0x00ff)));
		break;

	case 0x5600:
		execute_jcc(m_acc <= m_regs->read_byte(get_banked_address(opcode & 0x00ff)));
		break;

	case 0x5700:
		execute_jcc(m_acc == m_regs->read_byte(get_banked_address(opcode & 0x00ff)));
		break;

	case 0x5800: case 0x5900: case 0x5a00: case 0x5b00:
	case 0x5c00: case 0x5d00: case 0x5e00: case 0x5f00:
		execute_jbc(opcode & 0x00ff, (opcode & 0x0700) >> 8);
		break;

	case 0x6000: case 0x6100: case 0x6200: case 0x6300:
	case 0x6400: case 0x6500: case 0x6600: case 0x6700:
		execute_jbs(opcode & 0x00ff, (opcode & 0x0700) >> 8);
		break;

	case 0x6800: case 0x6900: case 0x6a00: case 0x6b00:
	case 0x6c00: case 0x6d00: case 0x6e00: case 0x6f00:
		execute_bc(opcode & 0x00ff, (opcode & 0x0700) >> 8);
		break;

	case 0x7000: case 0x7100: case 0x7200: case 0x7300:
	case 0x7400: case 0x7500: case 0x7600: case 0x7700:
		execute_bs(opcode & 0x00ff, (opcode & 0x0700) >> 8);
		break;

	case 0x7800: case 0x7900: case 0x7a00: case 0x7b00:
	case 0x7c00: case 0x7d00: case 0x7e00: case 0x7f00:
		execute_btg(opcode & 0x00ff, (opcode & 0x0700) >> 8);
		break;

	default:
		execute_undef(opcode);
		break;
	}
}

void riscii_series_device::execute_tbrd(u32 ptr)
{
	// TODO: "Bit 23 is used to select the internal/external memory"
	u16 addr = get_banked_address(m_curreg);
	u16 data = m_program->read_word(ptr >> 1);
	if (BIT(ptr, 0))
		m_regs->write_byte(addr, data >> 8);
	else
		m_regs->write_byte(addr, data);
	if (m_repeat != 0)
		--m_repeat;
	else
		m_exec_state = EXEC_CYCLE1;
}

void riscii_series_device::execute_run()
{
	while (m_icount > 0)
	{
		switch (m_exec_state)
		{
		case EXEC_CYCLE1:
			m_ppc = m_pc;
			debugger_instruction_hook(m_pc);
			if (m_repeat != 0)
			{
				execute_cycle1(m_cache->read_word(m_pc++));
				if (m_exec_state == EXEC_CYCLE1)
				{
					--m_repeat;
					m_pc = m_ppc;
				}
			}
			else
				execute_cycle1(m_cache->read_word(m_pc++));
			break;

		case EXEC_TBRD0:
			execute_tbrd(m_tabptr);
			break;

		case EXEC_TBRD1:
			execute_tbrd(std::exchange(m_tabptr, tabptr_offset(1)));
			break;

		case EXEC_TBRD2:
			execute_tbrd(std::exchange(m_tabptr, tabptr_offset(-1)));
			break;

		case EXEC_TBRDA:
			execute_tbrd(tabptr_offset(m_acc));
			break;

		case EXEC_L0JMP: case EXEC_L1JMP: case EXEC_L2JMP: case EXEC_L3JMP:
		case EXEC_L4JMP: case EXEC_L5JMP: case EXEC_L6JMP: case EXEC_L7JMP:
		case EXEC_L8JMP: case EXEC_L9JMP: case EXEC_LAJMP: case EXEC_LBJMP:
		case EXEC_LCJMP: case EXEC_LDJMP: case EXEC_LEJMP: case EXEC_LFJMP:
			execute_jump(u32(m_exec_state - EXEC_L0JMP) << 16 | m_cache->read_word(m_pc++));
			m_exec_state = EXEC_CYCLE1;
			break;

		case EXEC_L0CALL: case EXEC_L1CALL: case EXEC_L2CALL: case EXEC_L3CALL:
		case EXEC_L4CALL: case EXEC_L5CALL: case EXEC_L6CALL: case EXEC_L7CALL:
		case EXEC_L8CALL: case EXEC_L9CALL: case EXEC_LACALL: case EXEC_LBCALL:
		case EXEC_LCCALL: case EXEC_LDCALL: case EXEC_LECALL: case EXEC_LFCALL:
			execute_call(u32(m_exec_state - EXEC_L0CALL) << 16 | m_cache->read_word(m_pc++));
			m_exec_state = EXEC_CYCLE1;
			break;

		case EXEC_NOJMP:
			(void)m_cache->read_word(m_pc++);
			m_exec_state = EXEC_CYCLE1;
			break;
		}

		m_icount--;
	}
}

void riscii_series_device::execute_set_input(int inputnum, int state)
{
	// TODO
}

void riscii_series_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c%c%c%c%c",
			BIT(m_status, 7) ? '.' : 'T', // /TO
			BIT(m_status, 6) ? '.' : 'P', // /PD
			BIT(m_status, 5) ? 'G' : '.', // SGE
			BIT(m_status, 4) ? 'L' : '.', // SLE
			BIT(m_status, 3) ? 'V' : '.', // OV
			BIT(m_status, 2) ? 'Z' : '.',
			BIT(m_status, 1) ? 'D' : '.', // auxiliary carry
			BIT(m_status, 0) ? 'C' : '.');
		break;
	}
}

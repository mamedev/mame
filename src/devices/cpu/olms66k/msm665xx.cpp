// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Oki MSM665xx 16-bit microcontroller family (nX-8/500S core)

****************************************************************************/

#include "emu.h"
#include "msm665xx.h"
#include "nx8dasm.h"

// device type definition
DEFINE_DEVICE_TYPE(MSM66573, msm66573_device, "msm66573", "Oki MSM66573")

ALLOW_SAVE_TYPE(msm665xx_device::inst_state)


msm665xx_device::msm665xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor mem_map, address_map_constructor data_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 20, 0, mem_map)
	, m_data_config("data", ENDIANNESS_LITTLE, 16, 20, 0, data_map)
	, m_acc(0)
	, m_pc(0)
	, m_ppc(0)
	, m_psw(0)
	, m_lrb(0)
	, m_ssp(0)
	, m_csr(0)
	, m_dsr(0)
	, m_tsr(0)
	, m_romwin(0x30)
	, m_memscon(0)
	, m_inst(0)
	, m_fetch_byte(0)
	, m_adr(0)
	, m_seg(0)
	, m_tmp{0, 0}
	, m_icount(0)
{
}

device_memory_interface::space_config_vector msm665xx_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}

std::unique_ptr<util::disasm_interface> msm665xx_device::create_disassembler()
{
	return std::make_unique<nx8_500s_disassembler>(m_psw);
}


msm66573_device::msm66573_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msm665xx_device(mconfig, MSM66573, tag, owner, clock,
						address_map_constructor(), // TODO: 64KB internal ROM (optional)
						address_map_constructor(FUNC(msm66573_device::data_map), this))
{
}

void msm66573_device::data_map(address_map &map)
{
	map(0x00000, 0x00001).rw(FUNC(msm66573_device::ssp_r), FUNC(msm66573_device::ssp_w));
	map(0x00002, 0x00003).rw(FUNC(msm66573_device::lrb_r), FUNC(msm66573_device::lrb_w));
	map(0x00004, 0x00005).rw(FUNC(msm66573_device::psw_r), FUNC(msm66573_device::psw_w));
	map(0x00006, 0x00007).rw(FUNC(msm66573_device::acc_r), FUNC(msm66573_device::acc_w));
	map(0x00008, 0x00008).rw(FUNC(msm66573_device::tsr_r), FUNC(msm66573_device::tsr_w));
	map(0x00009, 0x00009).rw(FUNC(msm66573_device::dsr_r), FUNC(msm66573_device::dsr_w));
	map(0x0000b, 0x0000b).rw(FUNC(msm66573_device::romwin_r), FUNC(msm66573_device::romwin_w));
	map(0x00010, 0x00010).w(FUNC(msm66573_device::memsacp_w));
	map(0x00011, 0x00011).rw(FUNC(msm66573_device::memscon_r), FUNC(msm66573_device::memscon_w));
	// TODO: many, many other SFRs
	map(0x00200, 0x011ff).ram().share("internal");
}


void msm665xx_device::device_start()
{
	// get address spaces and access caches
	space(AS_PROGRAM).specific(m_program_space);
	space(AS_PROGRAM).cache(m_program_cache);
	space(AS_DATA).specific(m_data_space);
	space(AS_DATA).cache(m_data_cache);

	set_icountptr(m_icount);

	// debug state
	state_add(MSM665XX_ACC, "ACC", m_acc);
	state_add(MSM665XX_CSR, "CSR", m_csr).mask(0x0f);
	state_add(MSM665XX_PC, "PC", m_pc, [this](u16 data) { m_pc = data; prefetch(); next_inst(); });
	state_add<u32>(STATE_GENPC, "GENPC",
		[this]() { return u32(m_csr) << 16 | m_pc; },
		[this](u32 data) { m_csr = (data >> 16) & 0x0f; m_pc = data & 0xffff; m_ppc = data & 0xfffff; }
	).mask(0xfffff).noshow();
	state_add<u32>(STATE_GENPCBASE, "GENPCBASE", m_ppc,
		[this](u32 data) { m_csr = (data >> 16) & 0x0f; m_pc = data & 0xffff; m_ppc = data & 0xfffff; }
	).mask(0xfffff).noshow();
	state_add(MSM665XX_PSW, "PSW", m_psw);
	state_add(STATE_GENFLAGS, "FLAGS", m_psw).formatstr("%8s").noshow();
	state_add(MSM665XX_LRB, "LRB", m_lrb);
	state_add(MSM665XX_SSP, "SSP", m_ssp);
	u16 *fixed = static_cast<u16 *>(memshare("internal")->ptr());
	for (int n = 0; n < 4; n++)
		state_add<u16>(MSM665XX_PR0 + n, std::array<const char *, 4>{{"X1", "X2", "DP", "USP"}}[n],
			[this, fixed, n]() { return fixed[(m_psw & 0x07) << 2 | n]; },
			[this, fixed, n](u16 data) { fixed[(m_psw & 0x07) << 2 | n] = data; }
		);
	// NOTE: This assumes internal RAM is large enough (≥2KB) to provide all 256 register banks.
	// While most nX-8/500S MCUs have that much internal RAM, ML66514 has only 1KB.
	for (int n = 0; n < 4; n++)
		state_add<u16>(MSM665XX_ER0 + n, util::string_format("ER%d", n).c_str(),
			[this, fixed, n]() { return fixed[(m_lrb & 0x00ff) << 2 | n]; },
			[this, fixed, n](u16 data) { fixed[(m_lrb & 0x00ff) << 2 | n] = data; }
		);
	for (int n = 0; n < 8; n++)
		state_add<u8>(MSM665XX_R0 + n, util::string_format("R%d", n).c_str(),
			[this, fixed, n]() { return util::little_endian_cast<const u8>(fixed)[(m_lrb & 0x00ff) << 3 | n]; },
			[this, fixed, n](u8 data) { util::little_endian_cast<u8>(fixed)[(m_lrb & 0x00ff) << 3 | n] = data; }
		).noshow();
	state_add(MSM665XX_DSR, "DSR", m_dsr).mask(0x0f);
	state_add(MSM665XX_TSR, "TSR", m_tsr).mask(0x0f);
	state_add(MSM665XX_ROMWIN, "ROMWIN", m_romwin);
	state_add(MSM665XX_MEMSCON, "MEMSCON", m_memscon).mask(0x03);

	// save state
	save_item(NAME(m_acc));
	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_psw));
	save_item(NAME(m_lrb));
	save_item(NAME(m_ssp));
	save_item(NAME(m_csr));
	save_item(NAME(m_dsr));
	save_item(NAME(m_tsr));
	save_item(NAME(m_romwin));
	save_item(NAME(m_state));
	save_item(NAME(m_inst));
	save_item(NAME(m_fetch_byte));
	save_item(NAME(m_adr));
	save_item(NAME(m_seg));
	save_item(NAME(m_tmp));
}

u16 msm665xx_device::do_add(u16 n1, u16 n2, bool c) noexcept
{
	s32 sum = s16(n1) + (s16(n2) + s32(c));
	if (u32(n1) + c > u16(~n2))
		m_psw |= 0x8000;
	else
		m_psw &= 0x7fff;
	if ((sum & 0xffff) == 0)
		m_psw |= 0x4000;
	else
		m_psw &= 0xbfff;
	if (BIT(sum, 15))
		m_psw |= 0x0800;
	else
		m_psw &= 0xf7ff;
	if (sum >= 0x8000 || sum < s32(-0x8000))
		m_psw |= 0x0200;
	else
		m_psw &= 0xfdff;
	if ((n1 & 0x000f) + c > (~n2 & 0x000f))
		m_psw |= 0x2000;
	else
		m_psw &= 0xdfff;
	return sum & 0xffff;
}

u8 msm665xx_device::do_addb(u8 n1, u8 n2, bool c) noexcept
{
	s16 sum = s8(n1) + s16(c) + s8(n2);
	if (u16(n1) + c > u8(~n2))
		m_psw |= 0x8000;
	else
		m_psw &= 0x7fff;
	if ((sum & 0xff) == 0)
		m_psw |= 0x4000;
	else
		m_psw &= 0xbfff;
	if (BIT(sum, 7))
		m_psw |= 0x0800;
	else
		m_psw &= 0xf7ff;
	if (sum >= 0x80 || sum < s16(-0x80))
		m_psw |= 0x0200;
	else
		m_psw &= 0xfdff;
	if ((n1 & 0x0f) + c > (~n2 & 0x0f))
		m_psw |= 0x2000;
	else
		m_psw &= 0xdfff;
	return sum & 0xff;
}

u16 msm665xx_device::do_sub(u16 n1, u16 n2, bool c) noexcept
{
	s32 diff = s16(n1) - s32(c) - s16(n2);
	if (n1 < n2 + u32(c))
		m_psw |= 0x8000;
	else
		m_psw &= 0x7fff;
	if ((diff & 0xffff) == 0)
		m_psw |= 0x4000;
	else
		m_psw &= 0xbfff;
	if (BIT(diff, 15))
		m_psw |= 0x0800;
	else
		m_psw &= 0xf7ff;
	if (diff >= 0x8000 || diff < s32(-0x8000))
		m_psw |= 0x0200;
	else
		m_psw &= 0xfdff;
	if ((n1 & 0x000f) < (n2 & 0x000f) + c)
		m_psw |= 0x2000;
	else
		m_psw &= 0xdfff;
	return diff & 0xffff;
}

u8 msm665xx_device::do_subb(u8 n1, u8 n2, bool c) noexcept
{
	s16 diff = s8(n1) - s16(c) - s8(n2);
	if (n1 < n2 + u16(c))
		m_psw |= 0x8000;
	else
		m_psw &= 0x7fff;
	if ((diff & 0xff) == 0)
		m_psw |= 0x4000;
	else
		m_psw &= 0xbfff;
	if (BIT(diff, 7))
		m_psw |= 0x0800;
	else
		m_psw &= 0xf7ff;
	if (diff >= 0x80 || diff < s16(-0x80))
		m_psw |= 0x0200;
	else
		m_psw &= 0xfdff;
	if ((n1 & 0x0f) < (n2 & 0x0f) + c)
		m_psw |= 0x2000;
	else
		m_psw &= 0xdfff;
	return diff & 0xff;
}

u16 msm665xx_device::do_aluop_word(u16 n1, u16 n2) noexcept
{
	u16 result;
	switch (BIT(m_inst, 4, 3))
	{
	default: // (what even are bitfields?)
	case 0: // SUB
		return do_sub(n1, n2);

	case 1: // CMP
		(void)do_sub(n1, n2);
		return n1;

	case 2: // ADD
		return do_add(n1, n2);

	case 3: // AND
		result = n1 & n2;
		setzs_word(result);
		return result;

	case 4: // OR
		result = n1 | n2;
		setzs_word(result);
		return result;

	case 5: // XOR
		result = n1 ^ n2;
		setzs_word(result);
		return result;

	case 6: // ADC
		return do_add(n1, n2, BIT(m_psw, 15));

	case 7: // SBC
		return do_sub(n1, n2, BIT(m_psw, 15));
	}
}

u8 msm665xx_device::do_aluop_byte(u8 n1, u8 n2) noexcept
{
	u8 result;
	switch (BIT(m_inst, 4, 3))
	{
	default: // (what even are bitfields?)
	case 0: // SUB
		return do_subb(n1, n2);

	case 1: // CMP
		(void)do_subb(n1, n2);
		return n1;

	case 2: // ADD
		return do_addb(n1, n2);

	case 3: // AND
		result = n1 & n2;
		setzs_byte(result);
		return result;

	case 4: // OR
		result = n1 | n2;
		setzs_byte(result);
		return result;

	case 5: // XOR
		result = n1 ^ n2;
		setzs_byte(result);
		return result;

	case 6: // ADC
		return do_addb(n1, n2, BIT(m_psw, 15));

	case 7: // SBC
		return do_subb(n1, n2, BIT(m_psw, 15));
	}
}

u16 msm665xx_device::do_inc(u16 n) noexcept
{
	u16 result = n + 1;
	if (result == 0)
		m_psw |= 0x4000;
	else
		m_psw &= 0xbfff;
	if (BIT(result, 15))
		m_psw |= 0x0800;
	else
		m_psw &= 0xf7ff;
	if (result == 0x8000)
		m_psw |= 0x0200;
	else
		m_psw &= 0xfdff;
	if ((result & 0x000f) == 0)
		m_psw |= 0x2000;
	else
		m_psw &= 0xdfff;
	return result;
}

u8 msm665xx_device::do_incb(u8 n) noexcept
{
	u8 result = n + 1;
	if (result == 0)
		m_psw |= 0x4000;
	else
		m_psw &= 0xbfff;
	if (BIT(result, 7))
		m_psw |= 0x0800;
	else
		m_psw &= 0xf7ff;
	if (result == 0x80)
		m_psw |= 0x0200;
	else
		m_psw &= 0xfdff;
	if ((result & 0x0f) == 0)
		m_psw |= 0x2000;
	else
		m_psw &= 0xdfff;
	return result;
}

u16 msm665xx_device::do_dec(u16 n) noexcept
{
	u16 result = n - 1;
	if (result == 0)
		m_psw |= 0x4000;
	else
		m_psw &= 0xbfff;
	if (BIT(result, 15))
		m_psw |= 0x0800;
	else
		m_psw &= 0xf7ff;
	if (n == 0x8000)
		m_psw |= 0x0200;
	else
		m_psw &= 0xfdff;
	if ((n & 0x000f) == 0)
		m_psw |= 0x2000;
	else
		m_psw &= 0xdfff;
	return result;
}

u8 msm665xx_device::do_decb(u8 n) noexcept
{
	u8 result = n - 1;
	if (result == 0)
		m_psw |= 0x4000;
	else
		m_psw &= 0xbfff;
	if (BIT(result, 7))
		m_psw |= 0x0800;
	else
		m_psw &= 0xf7ff;
	if (n == 0x80)
		m_psw |= 0x0200;
	else
		m_psw &= 0xfdff;
	if ((n & 0x0f) == 0)
		m_psw |= 0x2000;
	else
		m_psw &= 0xdfff;
	return result;
}

u16 msm665xx_device::do_rol(u16 n) noexcept
{
	bool c = BIT(m_psw, 15);
	if (BIT(n, 15))
		m_psw |= 0x8000;
	else
		m_psw &= 0x7fff;
	return n << 1 | (c ? 1 : 0);
}

u8 msm665xx_device::do_rolb(u8 n) noexcept
{
	bool c = BIT(m_psw, 15);
	if (BIT(n, 7))
		m_psw |= 0x8000;
	else
		m_psw &= 0x7fff;
	return n << 1 | (c ? 1 : 0);
}

u16 msm665xx_device::do_ror(u16 n) noexcept
{
	bool c = BIT(m_psw, 15);
	if (BIT(n, 0))
		m_psw |= 0x8000;
	else
		m_psw &= 0x7fff;
	return n >> 1 | (c ? 0x8000 : 0);
}

u8 msm665xx_device::do_rorb(u8 n) noexcept
{
	bool c = BIT(m_psw, 15);
	if (BIT(n, 0))
		m_psw |= 0x8000;
	else
		m_psw &= 0x7fff;
	return n >> 1 | (c ? 0x80 : 0);
}

void msm665xx_device::do_mul(u16 data) noexcept
{
	u32 prod = m_acc * u32(data);
	if (prod == 0)
		m_psw |= 0x4000;
	else
		m_psw &= 0xbfff;

	// Store 32-bit product
	m_acc = prod >> 16;
	m_data_cache.write_word(addr_er(0), prod & 0xffff);
}

void msm665xx_device::do_div(u16 data) noexcept
{
	if (data == 0)
	{
		m_psw |= 0x8000;
		return;
	}
	m_psw &= 0x7fff;

	u32 div = u32(m_acc) << 16 | m_data_cache.read_word(addr_er(0));
	u32 quot = div / data;
	if (quot == 0)
		m_psw |= 0x4000;
	else
		m_psw &= 0xbfff;

	// Store 32-bit quotient and 16-bit remainder
	m_acc = quot >> 16;
	m_data_cache.write_word(addr_er(0), quot & 0xffff);
	m_data_cache.write_word(addr_er(1), div % data);
}

void msm665xx_device::do_divb(u8 data) noexcept
{
	if (data == 0)
	{
		m_psw |= 0x8000;
		return;
	}
	m_psw &= 0x7fff;

	u16 quot = m_acc / data;
	if (quot == 0)
		m_psw |= 0x4000;
	else
		m_psw &= 0xbfff;

	// Store 16-bit quotient and 8-bit remainder
	m_data_cache.write_byte(addr_lr(1), m_acc % data);
	m_acc = quot;
}

void msm665xx_device::do_divq(u16 data) noexcept
{
	if (data == 0)
	{
		m_psw |= 0x8000;
		return;
	}
	m_psw &= 0x7fff;

	u32 div = u32(m_acc) << 16 | m_data_cache.read_word(addr_er(0));
	u32 quot = div / data;
	if (quot >= 0x10000)
		m_psw |= 0x0200;
	else
		m_psw &= 0xfdff;
	if ((quot & 0xffff) == 0)
		m_psw |= 0x4000;
	else
		m_psw &= 0xbfff;

	// Store 16-bit quotient and 16-bit remainder
	m_acc = quot & 0xffff;
	m_data_cache.write_word(addr_er(1), div % data);
}


u16 msm665xx_device::ssp_r()
{
	return m_ssp;
}

void msm665xx_device::ssp_w(u16 data)
{
	m_ssp = data;
}

u16 msm665xx_device::lrb_r()
{
	return m_lrb;
}

void msm665xx_device::lrb_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_lrb = (m_lrb & ~mem_mask) | (data & mem_mask);
}

u16 msm665xx_device::psw_r()
{
	return m_psw;
}

void msm665xx_device::psw_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_psw = (m_psw & ~mem_mask) | (data & mem_mask);
}

u16 msm665xx_device::acc_r()
{
	return m_acc;
}

void msm665xx_device::acc_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_acc = (m_acc & ~mem_mask) | (data & mem_mask);
}

u8 msm665xx_device::dsr_r()
{
	return m_dsr;
}

void msm665xx_device::dsr_w(u8 data)
{
	if (!BIT(m_memscon, 0))
		logerror("%02X:%04X: Writing %02X to DSR without data memory space expansion\n", m_csr, m_pc, data);
	m_dsr = data & 0x0f;
}

u8 msm665xx_device::tsr_r()
{
	return m_tsr;
}

void msm665xx_device::tsr_w(u8 data)
{
	if (!BIT(m_memscon, 1))
		logerror("%02X:%04X: Writing %02X to TSR without program memory space expansion\n", m_csr, m_pc, data);
	m_tsr = data & 0x0f;
}

u8 msm665xx_device::romwin_r()
{
	return m_romwin;
}

void msm665xx_device::romwin_w(u8 data)
{
	// ROMWIN is only supposed to be written once after reset
	m_romwin = data | 0x30;
}

void msm665xx_device::memsacp_w(u8 data)
{
	logerror("%02X:%04X: Writing %02X to MEMSCAP\n", m_csr, m_pc, data);
}

u8 msm665xx_device::memscon_r()
{
	return m_memscon | 0xfc;
}

void msm665xx_device::memscon_w(u8 data)
{
	// FIXME: may be written only once after reset after double write to MEMSACP
	m_memscon = data & 0x03;
}

u16 msm665xx_device::get_seg(u16 addr) const noexcept
{
	// Common area from 0 to 03FFH, 1FFFH, 3FFFH, 7FFFH is always segment 0
	if (m_dsr == 0 || addr < (BIT(m_psw, 4, 2) == 0 ? 0x0400 : 0x1000 << BIT(m_psw, 4, 2)))
	{
		if (addr >= 0x1200 && (m_romwin & 0x0f) != 0 && (addr >> 12) >= (m_romwin & 0x0f) && (addr >> 12) <= (m_romwin >> 4))
			return 0x8000 | m_tsr;
		else
			return 0;
	}
	else
	{
		if ((m_romwin & 0x0f) != 0 && (addr >> 12) >= (m_romwin & 0x0f) && (addr >> 12) <= (m_romwin >> 4))
			return 0x8000 | m_tsr;
		else
			return m_dsr;
	}
}


enum class msm665xx_device::inst_state : u8
{
	NOP, no_jump, store_byte, store_word,
	RT, RTI, J_Cadr, J_Cadr_2,
	SJ, DJNZ_Rn, prefix_stackop, FCAL, FCAL_2, FCAL_3, FRT, PUSHS_A,
	op16_A_ERn_PRn, op8_A_Rn,
	MOVB_Rn_N8,
	MOV_PRn_ERn_N16, MOV_PRn_ERn_N16_2,
	ST_A_ind_PRn, ST_A_ind_DP_post, STB_A_ind_PRn, STB_A_ind_DP_post,
	ST_A_fix8, ST_A_off8, ST_A_sfr8, ST_A_dir, ST_A_dir_2,
	STB_A_fix8, STB_A_off8, STB_A_sfr8, STB_A_dir, STB_A_dir_2,
	ST_A_ERn_PRn, STB_A_Rn,
	INC_DEC_PRn, ACAL, prefix_sbaoff6, prefix_sbafix6,
	prefix_PRn_ERn, prefix_Rn,
	L_A_PRn_ERn, LB_A_Rn,
	L_A_ind_PRn, L_A_ind_DP_post,
	L_A_fix8, L_A_off8, L_A_sfr8, L_A_dir, L_A_dir_2, L_A_obj,
	L_A_D16_X1, L_A_D16_X1_2, L_A_n7_ind_PR, prefix_PSW, prefix_n7_ind_PR,
	op16_A_fix8, op16_A_off8, op16_A_N16, op16_A_N16_2, op16_A_N16_3, SLL_A,
	op8_A_fix8, op8_A_off8, op8_A_N8, op8_A_N8_2, SLLB_A,
	LB_A_ind_PRn, LB_A_ind_DP_post,
	LB_A_obj,
	SRL_A,
	SRLB_A,
	prefix_ind8_PRn, prefix_ind16_PRn, prefix_ind_DP_post,
	prefix_fix8, prefix_off8, prefix_sfr8, prefix_dir, prefix_dir_2,
	prefix_D16_PRn, prefix_D16_PRn_2, prefix_ind8_X1_A, prefix_ind16_X1_A, prefix_ind8_X1_R0, prefix_ind16_X1_R0,
	prefix_obj8, prefix_obj16,
	ROL_A,
	ROLB_A,
	prefix_A16, prefix_A8,
	ROR_A,
	RORB_A,
	INCB_DECB_Rn,
	CMP_fix8_N16, CMP_off8_N16,
	MOV_sfr8_N16, MOV_off8_N16,
	ST_A_D16_X1, ST_A_D16_X1_2, ST_A_n7_ind_PR,
	STB_A_D16_X1, STB_A_D16_X1_2, STB_A_n7_ind_PR,
	RC_SC,
	INC_A, NEG_A,
	INCB_A, NEGB_A,
	CMPB_fix8_N8, CMPB_off8_N8,
	MOVB_sfr8_N8, MOVB_off8_N8,
	RDD_SDD,
	DI_EI,
	DEC_A, SWAP,
	DECB_A,
	VCAL,
	JGT_JLE, JZ_JNZ, JCY_JNC, JNS_JPS,
	L_A_N16, L_A_N16_2, L_A_N16_3, LB_A_N8, LB_A_N8_2, CLR_A, CLRB_A,
	EXTND, CPL_C, CAL_Cadr, CAL_Cadr_2, BRK,

	RB_SB_obj_bit,
	MB_C_obj_bit, MB_obj_bit_C,
	JBR_JBS_obj_bit,
	JBRS_JBSR_obj_bit,
	BAND_C_obj_bit,
	BOR_C_obj_bit,
	BXOR_C_obj_bit,
	MOV_ERn_PRn_obj, MOVB_Rn_obj,
	op16_obj_fix8, op8_obj_fix8,
	op16_obj_off8, op8_obj_off8,
	op16_obj_sfr8, op8_obj_sfr8,
	op16_obj1_obj2, op8_obj1_obj2,
	op16_obj_N16, op16_obj_N16_2, op16_obj_N16_3,
	op8_obj_N8, op8_obj_N8_2,
	op16_obj_A, op8_obj_A,
	op16_A_obj, op8_A_obj,
	MOV_fix8_obj, MOV_off8_obj,
	MOVB_fix8_obj, MOVB_off8_obj,
	MOV_ind_PRn_obj, MOV_ind_DP_post_obj,
	MOVB_ind_PRn_obj, MOVB_ind_DP_post_obj,
	SLL_obj, SLLB_obj,
	CMP_obj1_obj2, CMPB_obj1_obj2,
	CMP_obj_N16, CMP_obj_N16_2, CMP_obj_N16_3,
	CMPB_obj_N8, CMPB_obj_N8_2,
	CMP_obj_A, CMPB_obj_A,
	MOV_sfr8_obj, MOV_A_obj,
	MOVB_sfr8_obj, MOVB_A_obj,
	MOV_D16_PRn_obj, MOV_D16_PRn_obj_2, MOVB_D16_PRn_obj, MOVB_D16_PRn_obj_2,
	MOV_n7_ind_PR_obj, MOVB_n7_ind_PR_obj,
	MOV_dir_obj, MOV_dir_obj_2, MOVB_dir_obj, MOVB_dir_obj_2,
	SRL_obj, SRLB_obj,
	TJNZ_TJZ, TJNZB_TJZB,
	DIV_obj, DIVB_obj,
	MUL_obj, MULB_obj,
	MOV_obj_A, MOV_obj_N16, MOV_obj_N16_2,
	MOVB_obj_A, MOVB_obj_N8,
	ROL_ROR_obj, ROLB_RORB_obj,
	CMPC_LC_A_Tadr, CMPC_LC_A_Tadr_2, CMPCB_LCB_A_Tadr, CMPCB_LCB_A_Tadr_2,
	SBR_RBR_obj, MBR_C_obj, MBR_obj_C,
	INC_DEC_obj, CLR_FILL_obj, XCHG_A_obj,
	INCB_DECB_obj, CLRB_FILLB_obj, XCHGB_A_obj,
	TBR_obj,
	J_ind_obj,
	CMPC_LC_A_ind_obj, CMPCB_LCB_A_ind_obj,
	CMPC_LC_A_T16_obj, CMPC_LC_A_T16_obj_2, CMPC_A_obj_3, CMPCB_A_obj_3, LC_A_obj_3, LCB_A_obj_3,
	DJNZ_obj,
	CAL_ind_obj,
	SRA_obj, SRAB_obj,
	MOV_obj_ind_X1_A, MOV_obj_ind_X1_R0, MOVB_obj_ind_X1_A, MOVB_obj_ind_X1_R0,
	FJ, FJ_2, FJ_3,
	DIVQ_obj,
	JLTS_JGES, JLES_JGTS,
	illegal
};

const msm665xx_device::inst_state msm665xx_device::s_inst_decode[2][0x100] =
{
	{
		// 00-0F (DD = 0)
		inst_state::NOP, inst_state::RT, inst_state::RTI, inst_state::J_Cadr,
		inst_state::SJ, inst_state::DJNZ_Rn, inst_state::prefix_stackop, inst_state::PUSHS_A,
		inst_state::op8_A_Rn, inst_state::op8_A_Rn, inst_state::op8_A_Rn, inst_state::op8_A_Rn,
		inst_state::op8_A_Rn, inst_state::op8_A_Rn, inst_state::op8_A_Rn, inst_state::op8_A_Rn,

		// 10-1F (DD = 0)
		inst_state::MOVB_Rn_N8, inst_state::MOVB_Rn_N8, inst_state::MOVB_Rn_N8, inst_state::MOVB_Rn_N8,
		inst_state::MOVB_Rn_N8, inst_state::MOVB_Rn_N8, inst_state::MOVB_Rn_N8, inst_state::MOVB_Rn_N8,
		inst_state::op8_A_Rn, inst_state::op8_A_Rn, inst_state::op8_A_Rn, inst_state::op8_A_Rn,
		inst_state::op8_A_Rn, inst_state::op8_A_Rn, inst_state::op8_A_Rn, inst_state::op8_A_Rn,

		// 20-2F (DD = 0)
		inst_state::MOV_PRn_ERn_N16, inst_state::MOV_PRn_ERn_N16, inst_state::MOV_PRn_ERn_N16, inst_state::MOV_PRn_ERn_N16,
		inst_state::MOV_PRn_ERn_N16, inst_state::MOV_PRn_ERn_N16, inst_state::MOV_PRn_ERn_N16, inst_state::MOV_PRn_ERn_N16,
		inst_state::op8_A_Rn, inst_state::op8_A_Rn, inst_state::op8_A_Rn, inst_state::op8_A_Rn,
		inst_state::op8_A_Rn, inst_state::op8_A_Rn, inst_state::op8_A_Rn, inst_state::op8_A_Rn,

		// 30-3F (DD = 0)
		inst_state::STB_A_ind_PRn, inst_state::STB_A_ind_DP_post, inst_state::STB_A_ind_PRn, inst_state::STB_A_ind_DP_post,
		inst_state::STB_A_fix8, inst_state::STB_A_off8, inst_state::STB_A_sfr8, inst_state::STB_A_dir,
		inst_state::STB_A_Rn, inst_state::STB_A_Rn, inst_state::STB_A_Rn, inst_state::STB_A_Rn,
		inst_state::STB_A_Rn, inst_state::STB_A_Rn, inst_state::STB_A_Rn, inst_state::STB_A_Rn,

		// 40-4F
		inst_state::INC_DEC_PRn, inst_state::INC_DEC_PRn, inst_state::INC_DEC_PRn, inst_state::INC_DEC_PRn,
		inst_state::ACAL, inst_state::ACAL, inst_state::ACAL, inst_state::ACAL,
		inst_state::prefix_sbaoff6, inst_state::prefix_sbaoff6, inst_state::prefix_sbaoff6, inst_state::prefix_sbaoff6,
		inst_state::prefix_sbaoff6, inst_state::prefix_sbaoff6, inst_state::prefix_sbaoff6, inst_state::prefix_sbaoff6,

		// 50-5F
		inst_state::INC_DEC_PRn, inst_state::INC_DEC_PRn, inst_state::INC_DEC_PRn, inst_state::INC_DEC_PRn,
		inst_state::ACAL, inst_state::ACAL, inst_state::ACAL, inst_state::ACAL,
		inst_state::prefix_sbafix6, inst_state::prefix_sbafix6, inst_state::prefix_sbafix6, inst_state::prefix_sbafix6,
		inst_state::prefix_sbafix6, inst_state::prefix_sbafix6, inst_state::prefix_sbafix6, inst_state::prefix_sbafix6,

		// 60-6F
		inst_state::prefix_PRn_ERn, inst_state::prefix_PRn_ERn, inst_state::prefix_PRn_ERn, inst_state::prefix_PRn_ERn,
		inst_state::prefix_PRn_ERn, inst_state::prefix_PRn_ERn, inst_state::prefix_PRn_ERn, inst_state::prefix_PRn_ERn,
		inst_state::prefix_Rn, inst_state::prefix_Rn, inst_state::prefix_Rn, inst_state::prefix_Rn,
		inst_state::prefix_Rn, inst_state::prefix_Rn, inst_state::prefix_Rn, inst_state::prefix_Rn,

		// 70-7F
		inst_state::L_A_PRn_ERn, inst_state::L_A_PRn_ERn, inst_state::L_A_PRn_ERn, inst_state::L_A_PRn_ERn,
		inst_state::L_A_PRn_ERn, inst_state::L_A_PRn_ERn, inst_state::L_A_PRn_ERn, inst_state::L_A_PRn_ERn,
		inst_state::LB_A_Rn, inst_state::LB_A_Rn, inst_state::LB_A_Rn, inst_state::LB_A_Rn,
		inst_state::LB_A_Rn, inst_state::LB_A_Rn, inst_state::LB_A_Rn, inst_state::LB_A_Rn,

		// 80-8F (DD = 0)
		inst_state::L_A_ind_PRn, inst_state::L_A_ind_DP_post, inst_state::L_A_ind_PRn, inst_state::L_A_ind_DP_post,
		inst_state::L_A_fix8, inst_state::L_A_off8, inst_state::L_A_sfr8, inst_state::L_A_dir,
		inst_state::L_A_D16_X1, inst_state::L_A_n7_ind_PR, inst_state::prefix_PSW, inst_state::prefix_n7_ind_PR,
		inst_state::op8_A_fix8, inst_state::op8_A_off8, inst_state::op8_A_N8, inst_state::SLLB_A,

		// 90-9F (DD = 0)
		inst_state::LB_A_ind_PRn, inst_state::LB_A_ind_DP_post, inst_state::LB_A_ind_PRn, inst_state::LB_A_ind_DP_post,
		inst_state::L_A_fix8, inst_state::L_A_off8, inst_state::L_A_sfr8, inst_state::L_A_dir,
		inst_state::L_A_D16_X1, inst_state::L_A_n7_ind_PR, inst_state::prefix_PSW, inst_state::prefix_n7_ind_PR,
		inst_state::op8_A_fix8, inst_state::op8_A_off8, inst_state::op8_A_N8, inst_state::SRLB_A,

		// A0-AF (DD = 0)
		inst_state::prefix_ind8_PRn, inst_state::prefix_ind_DP_post, inst_state::prefix_ind8_PRn, inst_state::prefix_ind_DP_post,
		inst_state::prefix_fix8, inst_state::prefix_off8, inst_state::prefix_sfr8, inst_state::prefix_dir,
		inst_state::prefix_D16_PRn, inst_state::prefix_D16_PRn, inst_state::prefix_ind8_X1_A, inst_state::prefix_ind8_X1_R0,
		inst_state::op8_A_fix8, inst_state::op8_A_off8, inst_state::op8_A_N8, inst_state::ROLB_A,

		// B0-BF (DD = 0)
		inst_state::prefix_ind8_PRn, inst_state::prefix_ind_DP_post, inst_state::prefix_ind8_PRn, inst_state::prefix_ind_DP_post,
		inst_state::prefix_fix8, inst_state::prefix_off8, inst_state::prefix_sfr8, inst_state::prefix_dir,
		inst_state::prefix_D16_PRn, inst_state::prefix_D16_PRn, inst_state::prefix_ind8_X1_A, inst_state::prefix_ind8_X1_R0,
		inst_state::prefix_A8, inst_state::op8_A_off8, inst_state::op8_A_N8, inst_state::RORB_A,

		// C0-CF (DD = 0)
		inst_state::INCB_DECB_Rn, inst_state::INCB_DECB_Rn, inst_state::INCB_DECB_Rn, inst_state::INCB_DECB_Rn,
		inst_state::CMP_fix8_N16, inst_state::CMP_off8_N16, inst_state::MOV_sfr8_N16, inst_state::MOV_off8_N16,
		inst_state::STB_A_D16_X1, inst_state::STB_A_n7_ind_PR, inst_state::RC_SC, inst_state::RC_SC,
		inst_state::INCB_A, inst_state::op8_A_off8, inst_state::op8_A_N8, inst_state::NEGB_A,

		// D0-DF (DD = 0)
		inst_state::INCB_DECB_Rn, inst_state::INCB_DECB_Rn, inst_state::INCB_DECB_Rn, inst_state::INCB_DECB_Rn,
		inst_state::CMPB_fix8_N8, inst_state::CMPB_off8_N8, inst_state::MOVB_sfr8_N8, inst_state::MOVB_off8_N8,
		inst_state::RDD_SDD, inst_state::RDD_SDD, inst_state::DI_EI, inst_state::DI_EI,
		inst_state::DECB_A, inst_state::op8_A_off8, inst_state::op8_A_N8, inst_state::SWAP,

		// E0-EF
		inst_state::VCAL, inst_state::VCAL, inst_state::VCAL, inst_state::VCAL,
		inst_state::VCAL, inst_state::VCAL, inst_state::VCAL, inst_state::VCAL,
		inst_state::VCAL, inst_state::VCAL, inst_state::VCAL, inst_state::VCAL,
		inst_state::VCAL, inst_state::VCAL, inst_state::VCAL, inst_state::VCAL,

		// F0-FF
		inst_state::JGT_JLE, inst_state::JZ_JNZ, inst_state::JCY_JNC, inst_state::JNS_JPS,
		inst_state::JNS_JPS, inst_state::JCY_JNC, inst_state::JZ_JNZ, inst_state::JGT_JLE,
		inst_state::L_A_N16, inst_state::LB_A_N8, inst_state::CLR_A, inst_state::CLRB_A,
		inst_state::EXTND, inst_state::CPL_C, inst_state::CAL_Cadr, inst_state::BRK
	},

	{
		// 00-0F (DD = 1)
		inst_state::NOP, inst_state::RT, inst_state::RTI, inst_state::J_Cadr,
		inst_state::SJ, inst_state::DJNZ_Rn, inst_state::prefix_stackop, inst_state::PUSHS_A,
		inst_state::op16_A_ERn_PRn, inst_state::op16_A_ERn_PRn, inst_state::op16_A_ERn_PRn, inst_state::op16_A_ERn_PRn,
		inst_state::op16_A_ERn_PRn, inst_state::op16_A_ERn_PRn, inst_state::op16_A_ERn_PRn, inst_state::op16_A_ERn_PRn,

		// 10-1F (DD = 1)
		inst_state::MOVB_Rn_N8, inst_state::MOVB_Rn_N8, inst_state::MOVB_Rn_N8, inst_state::MOVB_Rn_N8,
		inst_state::MOVB_Rn_N8, inst_state::MOVB_Rn_N8, inst_state::MOVB_Rn_N8, inst_state::MOVB_Rn_N8,
		inst_state::op16_A_ERn_PRn, inst_state::op16_A_ERn_PRn, inst_state::op16_A_ERn_PRn, inst_state::op16_A_ERn_PRn,
		inst_state::op16_A_ERn_PRn, inst_state::op16_A_ERn_PRn, inst_state::op16_A_ERn_PRn, inst_state::op16_A_ERn_PRn,

		// 20-2F (DD = 1)
		inst_state::MOV_PRn_ERn_N16, inst_state::MOV_PRn_ERn_N16, inst_state::MOV_PRn_ERn_N16, inst_state::MOV_PRn_ERn_N16,
		inst_state::MOV_PRn_ERn_N16, inst_state::MOV_PRn_ERn_N16, inst_state::MOV_PRn_ERn_N16, inst_state::MOV_PRn_ERn_N16,
		inst_state::op16_A_ERn_PRn, inst_state::op16_A_ERn_PRn, inst_state::op16_A_ERn_PRn, inst_state::op16_A_ERn_PRn,
		inst_state::op16_A_ERn_PRn, inst_state::op16_A_ERn_PRn, inst_state::op16_A_ERn_PRn, inst_state::op16_A_ERn_PRn,

		// 30-3F (DD = 1)
		inst_state::ST_A_ind_PRn, inst_state::ST_A_ind_DP_post, inst_state::ST_A_ind_PRn, inst_state::ST_A_ind_DP_post,
		inst_state::ST_A_fix8, inst_state::ST_A_off8, inst_state::ST_A_sfr8, inst_state::ST_A_dir,
		inst_state::ST_A_ERn_PRn, inst_state::ST_A_ERn_PRn, inst_state::ST_A_ERn_PRn, inst_state::ST_A_ERn_PRn,
		inst_state::ST_A_ERn_PRn, inst_state::ST_A_ERn_PRn, inst_state::ST_A_ERn_PRn, inst_state::ST_A_ERn_PRn,

		// 40-4F
		inst_state::INC_DEC_PRn, inst_state::INC_DEC_PRn, inst_state::INC_DEC_PRn, inst_state::INC_DEC_PRn,
		inst_state::ACAL, inst_state::ACAL, inst_state::ACAL, inst_state::ACAL,
		inst_state::prefix_sbaoff6, inst_state::prefix_sbaoff6, inst_state::prefix_sbaoff6, inst_state::prefix_sbaoff6,
		inst_state::prefix_sbaoff6, inst_state::prefix_sbaoff6, inst_state::prefix_sbaoff6, inst_state::prefix_sbaoff6,

		// 50-5F
		inst_state::INC_DEC_PRn, inst_state::INC_DEC_PRn, inst_state::INC_DEC_PRn, inst_state::INC_DEC_PRn,
		inst_state::ACAL, inst_state::ACAL, inst_state::ACAL, inst_state::ACAL,
		inst_state::prefix_sbafix6, inst_state::prefix_sbafix6, inst_state::prefix_sbafix6, inst_state::prefix_sbafix6,
		inst_state::prefix_sbafix6, inst_state::prefix_sbafix6, inst_state::prefix_sbafix6, inst_state::prefix_sbafix6,

		// 60-6F
		inst_state::prefix_PRn_ERn, inst_state::prefix_PRn_ERn, inst_state::prefix_PRn_ERn, inst_state::prefix_PRn_ERn,
		inst_state::prefix_PRn_ERn, inst_state::prefix_PRn_ERn, inst_state::prefix_PRn_ERn, inst_state::prefix_PRn_ERn,
		inst_state::prefix_Rn, inst_state::prefix_Rn, inst_state::prefix_Rn, inst_state::prefix_Rn,
		inst_state::prefix_Rn, inst_state::prefix_Rn, inst_state::prefix_Rn, inst_state::prefix_Rn,

		// 70-7F
		inst_state::L_A_PRn_ERn, inst_state::L_A_PRn_ERn, inst_state::L_A_PRn_ERn, inst_state::L_A_PRn_ERn,
		inst_state::L_A_PRn_ERn, inst_state::L_A_PRn_ERn, inst_state::L_A_PRn_ERn, inst_state::L_A_PRn_ERn,
		inst_state::LB_A_Rn, inst_state::LB_A_Rn, inst_state::LB_A_Rn, inst_state::LB_A_Rn,
		inst_state::LB_A_Rn, inst_state::LB_A_Rn, inst_state::LB_A_Rn, inst_state::LB_A_Rn,

		// 80-8F (DD = 1)
		inst_state::L_A_ind_PRn, inst_state::L_A_ind_DP_post, inst_state::L_A_ind_PRn, inst_state::L_A_ind_DP_post,
		inst_state::L_A_fix8, inst_state::L_A_off8, inst_state::L_A_sfr8, inst_state::L_A_dir,
		inst_state::L_A_D16_X1, inst_state::L_A_n7_ind_PR, inst_state::prefix_PSW, inst_state::prefix_n7_ind_PR,
		inst_state::op16_A_fix8, inst_state::op16_A_off8, inst_state::op16_A_N16, inst_state::SLL_A,

		// 90-9F (DD = 1)
		inst_state::LB_A_ind_PRn, inst_state::LB_A_ind_DP_post, inst_state::LB_A_ind_PRn, inst_state::LB_A_ind_DP_post,
		inst_state::L_A_fix8, inst_state::L_A_off8, inst_state::L_A_sfr8, inst_state::L_A_dir,
		inst_state::L_A_D16_X1, inst_state::L_A_n7_ind_PR, inst_state::prefix_PSW, inst_state::prefix_n7_ind_PR,
		inst_state::op16_A_fix8, inst_state::op16_A_off8, inst_state::op16_A_N16, inst_state::SRL_A,

		// A0-AF (DD = 1)
		inst_state::prefix_ind16_PRn, inst_state::prefix_ind_DP_post, inst_state::prefix_ind16_PRn, inst_state::prefix_ind_DP_post,
		inst_state::prefix_fix8, inst_state::prefix_off8, inst_state::prefix_sfr8, inst_state::prefix_dir,
		inst_state::prefix_D16_PRn, inst_state::prefix_D16_PRn, inst_state::prefix_ind16_X1_A, inst_state::prefix_ind16_X1_R0,
		inst_state::op16_A_fix8, inst_state::op16_A_off8, inst_state::op16_A_N16, inst_state::ROL_A,

		// B0-BF (DD = 1)
		inst_state::prefix_ind16_PRn, inst_state::prefix_ind_DP_post, inst_state::prefix_ind16_PRn, inst_state::prefix_ind_DP_post,
		inst_state::prefix_fix8, inst_state::prefix_off8, inst_state::prefix_sfr8, inst_state::prefix_dir,
		inst_state::prefix_D16_PRn, inst_state::prefix_D16_PRn, inst_state::prefix_ind16_X1_A, inst_state::prefix_ind16_X1_R0,
		inst_state::prefix_A16, inst_state::op16_A_off8, inst_state::op16_A_N16, inst_state::ROR_A,

		// C0-CF (DD = 1)
		inst_state::INCB_DECB_Rn, inst_state::INCB_DECB_Rn, inst_state::INCB_DECB_Rn, inst_state::INCB_DECB_Rn,
		inst_state::CMP_fix8_N16, inst_state::CMP_off8_N16, inst_state::MOV_sfr8_N16, inst_state::MOV_off8_N16,
		inst_state::ST_A_D16_X1, inst_state::ST_A_n7_ind_PR, inst_state::RC_SC, inst_state::RC_SC,
		inst_state::INC_A, inst_state::op16_A_off8, inst_state::op16_A_N16, inst_state::NEG_A,

		// D0-DF (DD = 1)
		inst_state::INCB_DECB_Rn, inst_state::INCB_DECB_Rn, inst_state::INCB_DECB_Rn, inst_state::INCB_DECB_Rn,
		inst_state::CMPB_fix8_N8, inst_state::CMPB_off8_N8, inst_state::MOVB_sfr8_N8, inst_state::MOVB_off8_N8,
		inst_state::RDD_SDD, inst_state::RDD_SDD, inst_state::DI_EI, inst_state::DI_EI,
		inst_state::DEC_A, inst_state::op16_A_off8, inst_state::op16_A_N16, inst_state::SWAP,

		// E0-EF
		inst_state::VCAL, inst_state::VCAL, inst_state::VCAL, inst_state::VCAL,
		inst_state::VCAL, inst_state::VCAL, inst_state::VCAL, inst_state::VCAL,
		inst_state::VCAL, inst_state::VCAL, inst_state::VCAL, inst_state::VCAL,
		inst_state::VCAL, inst_state::VCAL, inst_state::VCAL, inst_state::VCAL,

		// F0-FF
		inst_state::JGT_JLE, inst_state::JZ_JNZ, inst_state::JCY_JNC, inst_state::JNS_JPS,
		inst_state::JNS_JPS, inst_state::JCY_JNC, inst_state::JZ_JNZ, inst_state::JGT_JLE,
		inst_state::L_A_N16, inst_state::LB_A_N8, inst_state::CLR_A, inst_state::CLRB_A,
		inst_state::EXTND, inst_state::CPL_C, inst_state::CAL_Cadr, inst_state::BRK
	}
};

const msm665xx_device::inst_state msm665xx_device::s_prefixed_inst_decode[2][0x100] =
{
	{
		// 00-0F (byte mode)
		inst_state::RB_SB_obj_bit, inst_state::RB_SB_obj_bit, inst_state::RB_SB_obj_bit, inst_state::RB_SB_obj_bit,
		inst_state::RB_SB_obj_bit, inst_state::RB_SB_obj_bit, inst_state::RB_SB_obj_bit, inst_state::RB_SB_obj_bit,
		inst_state::RB_SB_obj_bit, inst_state::RB_SB_obj_bit, inst_state::RB_SB_obj_bit, inst_state::RB_SB_obj_bit,
		inst_state::RB_SB_obj_bit, inst_state::RB_SB_obj_bit, inst_state::RB_SB_obj_bit, inst_state::RB_SB_obj_bit,

		// 10-1F (byte mode)
		inst_state::MB_C_obj_bit, inst_state::MB_C_obj_bit, inst_state::MB_C_obj_bit, inst_state::MB_C_obj_bit,
		inst_state::MB_C_obj_bit, inst_state::MB_C_obj_bit, inst_state::MB_C_obj_bit, inst_state::MB_C_obj_bit,
		inst_state::MB_obj_bit_C, inst_state::MB_obj_bit_C, inst_state::MB_obj_bit_C, inst_state::MB_obj_bit_C,
		inst_state::MB_obj_bit_C, inst_state::MB_obj_bit_C, inst_state::MB_obj_bit_C, inst_state::MB_obj_bit_C,

		// 20-2F (byte mode)
		inst_state::JBR_JBS_obj_bit, inst_state::JBR_JBS_obj_bit, inst_state::JBR_JBS_obj_bit, inst_state::JBR_JBS_obj_bit,
		inst_state::JBR_JBS_obj_bit, inst_state::JBR_JBS_obj_bit, inst_state::JBR_JBS_obj_bit, inst_state::JBR_JBS_obj_bit,
		inst_state::JBR_JBS_obj_bit, inst_state::JBR_JBS_obj_bit, inst_state::JBR_JBS_obj_bit, inst_state::JBR_JBS_obj_bit,
		inst_state::JBR_JBS_obj_bit, inst_state::JBR_JBS_obj_bit, inst_state::JBR_JBS_obj_bit, inst_state::JBR_JBS_obj_bit,

		// 30-3F (byte mode)
		inst_state::JBRS_JBSR_obj_bit, inst_state::JBRS_JBSR_obj_bit, inst_state::JBRS_JBSR_obj_bit, inst_state::JBRS_JBSR_obj_bit,
		inst_state::JBRS_JBSR_obj_bit, inst_state::JBRS_JBSR_obj_bit, inst_state::JBRS_JBSR_obj_bit, inst_state::JBRS_JBSR_obj_bit,
		inst_state::JBRS_JBSR_obj_bit, inst_state::JBRS_JBSR_obj_bit, inst_state::JBRS_JBSR_obj_bit, inst_state::JBRS_JBSR_obj_bit,
		inst_state::JBRS_JBSR_obj_bit, inst_state::JBRS_JBSR_obj_bit, inst_state::JBRS_JBSR_obj_bit, inst_state::JBRS_JBSR_obj_bit,

		// 40-4F (byte mode)
		inst_state::BAND_C_obj_bit, inst_state::BAND_C_obj_bit, inst_state::BAND_C_obj_bit, inst_state::BAND_C_obj_bit,
		inst_state::BAND_C_obj_bit, inst_state::BAND_C_obj_bit, inst_state::BAND_C_obj_bit, inst_state::BAND_C_obj_bit,
		inst_state::BAND_C_obj_bit, inst_state::BAND_C_obj_bit, inst_state::BAND_C_obj_bit, inst_state::BAND_C_obj_bit,
		inst_state::BAND_C_obj_bit, inst_state::BAND_C_obj_bit, inst_state::BAND_C_obj_bit, inst_state::BAND_C_obj_bit,

		// 50-5F (byte mode)
		inst_state::BOR_C_obj_bit, inst_state::BOR_C_obj_bit, inst_state::BOR_C_obj_bit, inst_state::BOR_C_obj_bit,
		inst_state::BOR_C_obj_bit, inst_state::BOR_C_obj_bit, inst_state::BOR_C_obj_bit, inst_state::BOR_C_obj_bit,
		inst_state::BOR_C_obj_bit, inst_state::BOR_C_obj_bit, inst_state::BOR_C_obj_bit, inst_state::BOR_C_obj_bit,
		inst_state::BOR_C_obj_bit, inst_state::BOR_C_obj_bit, inst_state::BOR_C_obj_bit, inst_state::BOR_C_obj_bit,

		// 60-6F (byte mode)
		inst_state::BXOR_C_obj_bit, inst_state::BXOR_C_obj_bit, inst_state::BXOR_C_obj_bit, inst_state::BXOR_C_obj_bit,
		inst_state::BXOR_C_obj_bit, inst_state::BXOR_C_obj_bit, inst_state::BXOR_C_obj_bit, inst_state::BXOR_C_obj_bit,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,

		// 70-7F (byte mode)
		inst_state::MOVB_Rn_obj, inst_state::MOVB_Rn_obj, inst_state::MOVB_Rn_obj, inst_state::MOVB_Rn_obj,
		inst_state::MOVB_Rn_obj, inst_state::MOVB_Rn_obj, inst_state::MOVB_Rn_obj, inst_state::MOVB_Rn_obj,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,

		// 80-8F (byte mode)
		inst_state::op8_obj_fix8, inst_state::op8_obj_off8, inst_state::op8_obj_sfr8, inst_state::op8_obj_N8,
		inst_state::op8_obj_A, inst_state::op8_A_obj, inst_state::MOVB_fix8_obj, inst_state::MOVB_off8_obj,
		inst_state::MOVB_ind_PRn_obj, inst_state::MOVB_ind_DP_post_obj, inst_state::MOVB_ind_PRn_obj, inst_state::MOVB_ind_DP_post_obj,
		inst_state::SLLB_obj, inst_state::SLLB_obj, inst_state::SLLB_obj, inst_state::SLLB_obj,

		// 90-9F (byte mode)
		inst_state::op8_obj_fix8, inst_state::op8_obj_off8, inst_state::op8_obj_sfr8, inst_state::CMPB_obj_N8,
		inst_state::CMPB_obj_A, inst_state::op8_A_obj, inst_state::MOVB_sfr8_obj, inst_state::MOVB_A_obj,
		inst_state::MOVB_D16_PRn_obj, inst_state::MOVB_D16_PRn_obj, inst_state::MOVB_n7_ind_PR_obj, inst_state::MOVB_dir_obj,
		inst_state::SRLB_obj, inst_state::SRLB_obj, inst_state::SRLB_obj, inst_state::SRLB_obj,

		// A0-AF (byte mode)
		inst_state::op8_obj_fix8, inst_state::op8_obj_off8, inst_state::op8_obj_sfr8, inst_state::op8_obj_N8,
		inst_state::op8_obj_A, inst_state::op8_A_obj, inst_state::TJNZB_TJZB, inst_state::TJNZB_TJZB,
		inst_state::DIVB_obj, inst_state::MULB_obj, inst_state::MOVB_obj_A, inst_state::MOVB_obj_N8,
		inst_state::ROLB_RORB_obj, inst_state::illegal, inst_state::illegal, inst_state::illegal,

		// B0-BF (byte mode)
		inst_state::op8_obj_fix8, inst_state::op8_obj_off8, inst_state::op8_obj_sfr8, inst_state::op8_obj_N8,
		inst_state::op8_obj_A, inst_state::op8_A_obj, inst_state::CMPCB_LCB_A_Tadr, inst_state::CMPCB_LCB_A_Tadr,
		inst_state::SBR_RBR_obj, inst_state::SBR_RBR_obj, inst_state::MBR_C_obj, inst_state::MBR_obj_C,
		inst_state::ROLB_RORB_obj, inst_state::illegal, inst_state::illegal, inst_state::illegal,

		// C0-CF (byte mode)
		inst_state::op8_obj_fix8, inst_state::op8_obj_off8, inst_state::op8_obj_sfr8, inst_state::op8_obj_N8,
		inst_state::op8_obj_A, inst_state::op8_A_obj, inst_state::INCB_DECB_obj, inst_state::CLRB_FILLB_obj,
		inst_state::XCHGB_A_obj, inst_state::illegal, inst_state::TBR_obj, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,

		// D0-DF (byte mode)
		inst_state::op8_obj_fix8, inst_state::op8_obj_off8, inst_state::op8_obj_sfr8, inst_state::op8_obj_N8,
		inst_state::op8_obj_A, inst_state::op8_A_obj, inst_state::INCB_DECB_obj, inst_state::CLRB_FILLB_obj,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,

		// E0-EF (byte mode)
		inst_state::op8_obj_fix8, inst_state::op8_obj_off8, inst_state::op8_obj_sfr8, inst_state::op8_obj_N8,
		inst_state::op8_obj_A, inst_state::op8_A_obj, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::DJNZ_obj, inst_state::illegal,
		inst_state::SRAB_obj, inst_state::SRAB_obj, inst_state::SRAB_obj, inst_state::SRAB_obj,

		// F0-FF (byte mode)
		inst_state::op8_obj_fix8, inst_state::op8_obj_off8, inst_state::op8_obj_sfr8, inst_state::op8_obj_N8,
		inst_state::op8_obj_A, inst_state::op8_A_obj, inst_state::illegal, inst_state::illegal,
		inst_state::MOVB_obj_ind_X1_A, inst_state::MOVB_obj_ind_X1_R0, inst_state::illegal, inst_state::illegal,
		inst_state::JLTS_JGES, inst_state::JLES_JGTS, inst_state::JLES_JGTS, inst_state::JLTS_JGES
	},

	{
		// 00-0F (word mode)
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,

		// 10-1F (word mode)
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,

		// 20-2F (word mode)
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,

		// 30-3F (word mode)
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,

		// 40-4F (word mode)
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,

		// 50-5F (word mode)
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,

		// 60-6F (word mode)
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,

		// 70-7F (word mode)
		inst_state::MOV_ERn_PRn_obj, inst_state::MOV_ERn_PRn_obj, inst_state::MOV_ERn_PRn_obj, inst_state::MOV_ERn_PRn_obj,
		inst_state::MOV_ERn_PRn_obj, inst_state::MOV_ERn_PRn_obj, inst_state::MOV_ERn_PRn_obj, inst_state::MOV_ERn_PRn_obj,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,

		// 80-8F (word mode)
		inst_state::op16_obj_fix8, inst_state::op16_obj_off8, inst_state::op16_obj_sfr8, inst_state::op16_obj_N16,
		inst_state::op16_obj_A, inst_state::op16_A_obj, inst_state::MOV_fix8_obj, inst_state::MOV_off8_obj,
		inst_state::MOV_ind_PRn_obj, inst_state::MOV_ind_DP_post_obj, inst_state::MOV_ind_PRn_obj, inst_state::MOV_ind_DP_post_obj,
		inst_state::SLL_obj, inst_state::SLL_obj, inst_state::SLL_obj, inst_state::SLL_obj,

		// 90-9F (word mode)
		inst_state::op16_obj_fix8, inst_state::op16_obj_off8, inst_state::op16_obj_sfr8, inst_state::CMP_obj_N16,
		inst_state::CMP_obj_A, inst_state::op16_A_obj, inst_state::MOV_sfr8_obj, inst_state::MOV_A_obj,
		inst_state::MOV_D16_PRn_obj, inst_state::MOV_D16_PRn_obj, inst_state::MOV_n7_ind_PR_obj, inst_state::MOV_dir_obj,
		inst_state::SRL_obj, inst_state::SRL_obj, inst_state::SRL_obj, inst_state::SRL_obj,

		// A0-AF (word mode)
		inst_state::op16_obj_fix8, inst_state::op16_obj_off8, inst_state::op16_obj_sfr8, inst_state::op16_obj_N16,
		inst_state::op16_obj_A, inst_state::op16_A_obj, inst_state::TJNZ_TJZ, inst_state::TJNZ_TJZ,
		inst_state::DIV_obj, inst_state::MUL_obj, inst_state::MOV_obj_A, inst_state::MOV_obj_N16,
		inst_state::ROL_ROR_obj, inst_state::illegal, inst_state::illegal, inst_state::illegal,

		// B0-BF (word mode)
		inst_state::op16_obj_fix8, inst_state::op16_obj_off8, inst_state::op16_obj_sfr8, inst_state::op16_obj_N16,
		inst_state::op16_obj_A, inst_state::op16_A_obj, inst_state::CMPC_LC_A_Tadr, inst_state::CMPC_LC_A_Tadr,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,
		inst_state::ROL_ROR_obj, inst_state::illegal, inst_state::illegal, inst_state::illegal,

		// C0-CF (word mode)
		inst_state::op16_obj_fix8, inst_state::op16_obj_off8, inst_state::op16_obj_sfr8, inst_state::op16_obj_N16,
		inst_state::op16_obj_A, inst_state::op16_A_obj, inst_state::INC_DEC_obj, inst_state::CLR_FILL_obj,
		inst_state::XCHG_A_obj, inst_state::J_ind_obj, inst_state::illegal, inst_state::illegal,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,

		// D0-DF (word mode)
		inst_state::op16_obj_fix8, inst_state::op16_obj_off8, inst_state::op16_obj_sfr8, inst_state::op16_obj_N16,
		inst_state::op16_obj_A, inst_state::op16_A_obj, inst_state::INC_DEC_obj, inst_state::CLR_FILL_obj,
		inst_state::CMPC_LC_A_ind_obj, inst_state::CMPCB_LCB_A_ind_obj, inst_state::CMPC_LC_A_ind_obj, inst_state::CMPCB_LCB_A_ind_obj,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal,

		// E0-EF (word mode)
		inst_state::op16_obj_fix8, inst_state::op16_obj_off8, inst_state::op16_obj_sfr8, inst_state::op16_obj_N16,
		inst_state::op16_obj_A, inst_state::op16_A_obj, inst_state::CMPC_LC_A_T16_obj, inst_state::CMPC_LC_A_T16_obj,
		inst_state::illegal, inst_state::illegal, inst_state::DJNZ_obj, inst_state::CAL_ind_obj,
		inst_state::SRA_obj, inst_state::SRA_obj, inst_state::SRA_obj, inst_state::SRA_obj,

		// F0-FF (word mode)
		inst_state::op16_obj_fix8, inst_state::op16_obj_off8, inst_state::op16_obj_sfr8, inst_state::op16_obj_N16,
		inst_state::op16_obj_A, inst_state::op16_A_obj, inst_state::CMPC_LC_A_T16_obj, inst_state::CMPC_LC_A_T16_obj,
		inst_state::MOV_obj_ind_X1_A, inst_state::MOV_obj_ind_X1_R0, inst_state::FJ, inst_state::DIVQ_obj,
		inst_state::illegal, inst_state::illegal, inst_state::illegal, inst_state::illegal
	}
};


void msm665xx_device::device_reset()
{
	// PC will be initialized from vector following reset; LRB and ROMWIN are undefined at reset
	m_acc = 0;
	m_psw = 0;
	m_ssp = 0xffff;
	m_csr = 0;
	m_dsr = 0;
	m_tsr = 0;
	m_memscon = 0;
	m_state = inst_state::J_Cadr;
}

void msm665xx_device::execute_run()
{
	do
	{
		prefetch();
		switch (m_state)
		{
		case inst_state::NOP:
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::no_jump:
			m_icount -= 2;
			m_state = inst_state::NOP;
			break;

		case inst_state::store_byte:
			m_icount -= 2;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::store_word:
			m_icount -= 2;
			next_inst_and_store_word(m_tmp[0]);
			break;

		case inst_state::RT:
			m_ssp += 2;
			m_pc = data_read_word(0, m_ssp);
			m_icount -= 4;
			m_state = inst_state::NOP;
			break;

		case inst_state::RTI:
			m_adr = m_ssp + 2;
			m_psw = data_read_word(0, m_adr);
			m_adr += 2;
			m_lrb = data_read_word(0, m_adr);
			m_adr += 2;
			m_acc = data_read_word(0, m_adr);
			if (BIT(m_memscon, 1))
			{
				m_adr += 2;
				m_csr = data_read_word(0, m_adr) & 0x000f;
			}
			m_adr += 2;
			m_pc = data_read_word(0, m_adr);
			m_ssp = m_adr;
			m_icount -= BIT(m_memscon, 1) ? 12 : 10;
			m_state = inst_state::NOP;
			break;

		case inst_state::J_Cadr:
			m_tmp[0] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::J_Cadr_2;
			break;

		case inst_state::J_Cadr_2:
			m_pc = m_fetch_byte << 8 | m_tmp[0];
			m_icount -= 3;
			m_state = inst_state::NOP;
			break;

		case inst_state::SJ:
			m_pc += s8(m_fetch_byte);
			m_icount -= 4;
			m_state = inst_state::NOP;
			break;

		case inst_state::DJNZ_Rn:
			m_adr = addr_lr(BIT(m_fetch_byte, 7) ? 5 : 4);
			m_tmp[0] = m_data_cache.read_byte(m_adr) - 1;
			m_data_cache.write_byte(m_adr, m_tmp[0]);
			if (m_tmp[0] != 0)
			{
				m_pc += s8(m_fetch_byte | 0x80);
				m_icount -= 10 - 2;
				m_state = inst_state::NOP;
			}
			else
			{
				m_icount -= 7;
				next_inst();
			}
			break;

		case inst_state::prefix_stackop:
			m_inst = m_fetch_byte;
			if ((m_inst & 0x30) != 0x30)
			{
				if (BIT(m_inst, 6))
				{
					m_adr = m_data_cache.read_word(addr_pr(3));
					m_seg = get_seg(m_adr);
				}
				else
				{
					m_adr = m_ssp;
					m_seg = 0;
				}
				m_tmp[1] = BIT(m_inst, 5) ? 0 : BIT(m_inst, 4) ? addr_pr(0) : addr_er(0);
				if (BIT(m_inst, 7))
				{
					// POPS/POPU
					for (int n = 0; n < 4; n++)
					{
						if (BIT(m_inst, n))
						{
							m_adr += 2;
							if (BIT(m_inst, 6))
								m_seg = get_seg(m_adr);
							m_tmp[0] = data_read_word(m_seg, m_adr);
							data_write_word(0, m_tmp[1] + n * 2, m_tmp[0]);
							m_icount -= 4;
						}
					}
				}
				else
				{
					// PUSHS/PUSHU
					for (int n = 3; n >= 0; n--)
					{
						if (BIT(m_inst, n))
						{
							m_tmp[0] = data_read_word(0, m_tmp[1] + n * 2);
							data_write_word(m_seg, m_adr, m_tmp[0]);
							m_adr -= 2;
							if (BIT(m_inst, 6))
								m_seg = get_seg(m_adr);
							m_icount -= 4;
						}
					}
				}
				if (BIT(m_inst, 6))
					m_data_cache.write_word(addr_pr(3), m_adr);
				else
					m_ssp = m_adr;
				m_icount -= 3;
				m_state = inst_state::NOP;
				break;
			}
			else if (m_inst == 0x38 && BIT(m_memscon, 1))
			{
				m_state = inst_state::FCAL;
				m_icount -= 2;
			}
			else if (m_inst == 0x39 && BIT(m_memscon, 1))
			{
				m_state = inst_state::FRT;
				m_icount -= 2;
			}
			else
				m_state = inst_state::illegal;
			break;

		case inst_state::FCAL:
			m_tmp[0] = m_fetch_byte;
			m_state = inst_state::FCAL_2;
			m_icount -= 2;
			break;

		case inst_state::FCAL_2:
			m_tmp[0] |= m_fetch_byte << 8;
			m_adr = m_ssp;
			m_seg = 0;
			m_state = inst_state::FCAL_3;
			m_icount -= 2;
			break;

		case inst_state::FCAL_3:
			data_write_word(0, m_adr, m_pc);
			m_adr -= 2;
			data_write_word(0, m_adr, m_csr);
			m_ssp = m_adr - 2;
			m_pc = m_tmp[0];
			m_csr = m_fetch_byte;
			m_state = inst_state::NOP;
			m_icount -= 5;
			break;

		case inst_state::FRT:
			m_adr = m_ssp + 2;
			m_csr = data_read_word(0, m_adr) & 0x000f;
			m_adr += 2;
			m_pc = data_read_word(0, m_adr);
			m_ssp = m_adr;
			m_state = inst_state::NOP;
			m_icount -= 9 - (2 * 2);
			break;

		case inst_state::PUSHS_A:
			data_write_word(0, m_ssp, m_acc);
			m_ssp -= 2;
			m_icount -= 3;
			next_inst();
			break;

		case inst_state::op16_A_ERn_PRn:
			m_adr = BIT(m_inst, 2) ? addr_pr(m_inst) : addr_er(m_inst);
			m_acc = do_aluop_word(m_acc, m_data_cache.read_word(m_adr));
			m_icount -= 3;
			next_inst();
			break;

		case inst_state::op8_A_Rn:
			m_adr = addr_lr(m_inst);
			m_acc = (m_acc & 0xff00) | do_aluop_byte(m_acc & 0x00ff, m_data_cache.read_byte(m_adr));
			m_icount -= 3;
			next_inst();
			break;

		case inst_state::MOVB_Rn_N8:
			m_adr = addr_lr(m_inst);
			m_seg = 0;
			m_tmp[0] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::store_byte;
			break;

		case inst_state::MOV_PRn_ERn_N16:
			m_adr = BIT(m_inst, 2) ? addr_er(m_inst) : addr_pr(m_inst);
			m_seg = 0;
			m_tmp[0] = m_fetch_byte;
			m_state = inst_state::MOV_PRn_ERn_N16_2;
			m_icount -= 2;
			break;

		case inst_state::MOV_PRn_ERn_N16_2:
			m_tmp[0] |= m_fetch_byte << 8;
			m_state = inst_state::store_word;
			m_icount -= 2;
			break;

		case inst_state::ST_A_ind_PRn:
			m_adr = m_data_cache.read_word(addr_pr(m_inst));
			m_seg = get_seg(m_adr);
			m_icount -= 4;
			next_inst_and_store_word(m_acc);
			break;

		case inst_state::ST_A_ind_DP_post:
			m_tmp[1] = addr_pr(2);
			m_adr = m_data_cache.read_word(m_tmp[1]);
			m_seg = get_seg(m_adr);
			m_data_cache.write_word(m_tmp[1], m_adr + (BIT(m_inst, 1) ? 2 : -2));
			m_icount -= 5;
			next_inst_and_store_word(m_acc);
			break;

		case inst_state::STB_A_ind_PRn:
			m_adr = m_data_cache.read_word(addr_pr(m_inst));
			m_seg = get_seg(m_adr);
			m_icount -= 4;
			next_inst_and_store_byte(m_acc & 0x00ff);
			break;

		case inst_state::STB_A_ind_DP_post:
			m_tmp[1] = addr_pr(2);
			m_adr = m_data_cache.read_word(m_tmp[1]);
			m_seg = get_seg(m_adr);
			m_data_cache.write_word(m_tmp[1], m_adr + (BIT(m_inst, 1) ? 1 : -1));
			m_icount -= 5;
			next_inst_and_store_byte(m_acc & 0x00ff);
			break;

		case inst_state::ST_A_fix8:
			set_adr_fix8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::MOV_obj_A;
			break;

		case inst_state::ST_A_off8:
			set_adr_off8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::MOV_obj_A;
			break;

		case inst_state::ST_A_sfr8:
			set_adr_sfr8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::MOV_obj_A;
			break;

		case inst_state::ST_A_dir:
			m_tmp[0] = m_fetch_byte;
			m_state = inst_state::ST_A_dir_2;
			m_icount -= 2;
			break;

		case inst_state::ST_A_dir_2:
			m_adr = m_fetch_byte << 8 | m_tmp[0];
			m_seg = get_seg(m_adr);
			m_tmp[0] = m_acc;
			m_state = inst_state::store_word;
			m_icount -= 2;
			break;

		case inst_state::STB_A_fix8:
			set_adr_fix8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::MOVB_obj_A;
			break;

		case inst_state::STB_A_off8:
			set_adr_off8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::MOVB_obj_A;
			break;

		case inst_state::STB_A_sfr8:
			set_adr_sfr8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::MOVB_obj_A;
			break;

		case inst_state::STB_A_dir:
			m_tmp[0] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::STB_A_dir_2;
			break;

		case inst_state::STB_A_dir_2:
			m_adr = m_fetch_byte << 8 | m_tmp[0];
			m_seg = get_seg(m_adr);
			m_icount -= 2;
			m_state = inst_state::MOVB_obj_A;
			break;

		case inst_state::ST_A_ERn_PRn:
			m_adr = BIT(m_inst, 2) ? addr_pr(m_inst) : addr_er(m_inst);
			m_data_cache.write_word(m_adr, m_acc);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::STB_A_Rn:
			m_adr = addr_lr(m_inst);
			m_data_cache.write_byte(m_adr, m_acc & 0x00ff);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::INC_DEC_PRn:
			m_adr = addr_pr(m_inst);
			m_tmp[0] = m_data_cache.read_word(m_adr);
			if (BIT(m_inst, 4))
				m_tmp[0] = do_dec(m_tmp[0]);
			else
				m_tmp[0] = do_inc(m_tmp[0]);
			m_data_cache.write_word(m_adr, m_tmp[0]);
			m_icount -= 3;
			next_inst();
			break;

		case inst_state::ACAL:
			m_tmp[0] = (BIT(m_inst, 4) ? 0x1400 : 0x1000) | u16(m_inst & 0x03) << 8 | m_fetch_byte;
			data_write_word(0, m_ssp, m_pc);
			m_ssp -= 2;
			m_pc = m_tmp[0];
			m_icount -= 7;
			m_state = inst_state::NOP;
			break;

		case inst_state::prefix_sbaoff6:
			set_adr_off8(0xc0 | m_fetch_byte);
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			if (BIT(m_fetch_byte, 7))
			{
				if (BIT(m_fetch_byte, 6) != BIT(m_tmp[0], m_inst & 0x07))
				{
					m_state = inst_state::SJ;
					m_icount -= 9 - 6;
				}
				else
				{
					m_state = inst_state::no_jump;
					m_icount -= 6 - 4;
				}
			}
			else
			{
				if (BIT(m_fetch_byte, 6))
					m_tmp[0] &= ~(1 << (m_inst & 0x07));
				else
					m_tmp[0] |= 1 << (m_inst & 0x07);
				m_state = inst_state::store_byte;
				m_icount -= 4 - 2;
			}
			break;

		case inst_state::prefix_sbafix6:
			set_adr_fix8(0xc0 | m_fetch_byte);
			m_tmp[0] = m_data_cache.read_byte(m_adr);
			if (BIT(m_fetch_byte, 7))
			{
				if (BIT(m_fetch_byte, 6) != BIT(m_tmp[0], m_inst & 0x07))
				{
					m_state = inst_state::SJ;
					m_icount -= 9 - 6;
				}
				else
				{
					m_state = inst_state::no_jump;
					m_icount -= 6 - 4;
				}
			}
			else
			{
				if (BIT(m_fetch_byte, 6))
					m_tmp[0] &= ~(1 << (m_inst & 0x07));
				else
					m_tmp[0] |= 1 << (m_inst & 0x07);
				m_state = inst_state::store_byte;
				m_icount -= 4 - 2;
			}
			break;

		case inst_state::prefix_PRn_ERn:
			m_adr = BIT(m_inst, 2) ? addr_er(m_inst) : addr_pr(m_inst);
			m_seg = 0;
			m_inst = m_fetch_byte;
			m_icount -= 2;
			m_state = s_prefixed_inst_decode[1][m_inst];
			break;

		case inst_state::prefix_Rn:
			m_adr = addr_lr(m_inst);
			m_seg = 0;
			m_inst = m_fetch_byte;
			m_icount -= 2;
			m_state = s_prefixed_inst_decode[0][m_inst];
			break;

		case inst_state::L_A_PRn_ERn:
			m_adr = BIT(m_inst, 2) ? addr_er(m_inst) : addr_pr(m_inst);
			acc_load_word(m_data_cache.read_word(m_adr));
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::LB_A_Rn:
			m_adr = addr_lr(m_inst);
			acc_load_byte(m_data_cache.read_byte(m_adr));
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::L_A_ind_PRn:
			m_adr = m_data_cache.read_word(addr_pr(m_inst));
			m_seg = get_seg(m_adr);
			acc_load_word(data_read_word(m_seg, m_adr));
			m_icount -= 4;
			next_inst();
			break;

		case inst_state::L_A_ind_DP_post:
			m_tmp[1] = addr_pr(2);
			m_adr = m_data_cache.read_word(m_tmp[1]);
			m_seg = get_seg(m_adr);
			m_data_cache.write_word(m_tmp[1], m_adr + (BIT(m_inst, 1) ? 2 : -2));
			acc_load_word(data_read_word(m_seg, m_adr));
			m_icount -= 5;
			next_inst();
			break;

		case inst_state::L_A_fix8:
			set_adr_fix8(m_fetch_byte);
			m_state = BIT(m_inst, 4) ? inst_state::LB_A_obj : inst_state::L_A_obj;
			m_icount -= 2;
			break;

		case inst_state::L_A_off8:
			set_adr_off8(m_fetch_byte);
			m_icount -= 2;
			m_state = BIT(m_inst, 4) ? inst_state::LB_A_obj : inst_state::L_A_obj;
			break;

		case inst_state::L_A_sfr8:
			set_adr_sfr8(m_fetch_byte);
			m_icount -= 2;
			m_state = BIT(m_inst, 4) ? inst_state::LB_A_obj : inst_state::L_A_obj;
			break;

		case inst_state::L_A_dir:
			m_tmp[0] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::L_A_dir_2;
			break;

		case inst_state::L_A_dir_2:
			m_adr = m_fetch_byte << 8 | m_tmp[0];
			m_seg = get_seg(m_adr);
			m_icount -= 2;
			m_state = BIT(m_inst, 4) ? inst_state::LB_A_obj : inst_state::L_A_obj;
			break;

		case inst_state::L_A_obj:
			acc_load_word(data_read_word(m_seg, m_adr));
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::L_A_D16_X1:
			m_tmp[0] = m_fetch_byte;
			m_tmp[1] = m_data_cache.read_word(addr_pr(0));
			m_icount -= 2;
			m_state = inst_state::L_A_D16_X1_2;
			break;

		case inst_state::L_A_D16_X1_2:
			m_adr = (m_fetch_byte << 8 | m_tmp[0]) + m_tmp[1];
			m_seg = get_seg(m_adr);
			m_icount -= 2;
			m_state = BIT(m_inst, 4) ? inst_state::LB_A_obj : inst_state::L_A_obj;
			break;

		case inst_state::L_A_n7_ind_PR:
			m_adr = m_data_cache.read_word(addr_pr(2 + BIT(m_fetch_byte, 7))) + util::sext(m_fetch_byte, 7);
			m_seg = get_seg(m_adr);
			m_icount -= 4;
			m_state = BIT(m_inst, 4) ? inst_state::LB_A_obj : inst_state::L_A_obj;
			break;

		case inst_state::prefix_PSW:
			set_adr_sfr8(0x04 + BIT(m_inst, 4));
			m_inst = m_fetch_byte;
			m_icount -= 2;
			m_state = s_prefixed_inst_decode[0][m_inst];
			break;

		case inst_state::prefix_n7_ind_PR:
			m_adr = m_data_cache.read_word(addr_pr(2 + BIT(m_fetch_byte, 7))) + util::sext(m_fetch_byte, 7);
			m_seg = get_seg(m_adr);
			m_state = BIT(m_inst, 4) ? inst_state::prefix_obj8 : inst_state::prefix_obj16;
			m_icount -= 6 - 2;
			break;

		case inst_state::op16_A_fix8:
			set_adr_fix8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::op16_A_obj;
			break;

		case inst_state::op16_A_off8:
			set_adr_off8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::op16_A_obj;
			break;

		case inst_state::op16_A_N16:
			m_tmp[0] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::op16_A_N16_2;
			break;

		case inst_state::op16_A_N16_2:
			m_tmp[0] |= m_fetch_byte << 8;
			m_icount -= 2;
			m_state = inst_state::op16_A_N16_3;
			break;

		case inst_state::op16_A_N16_3:
			m_acc = do_aluop_word(m_acc, m_tmp[0]);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::op8_A_fix8:
			set_adr_fix8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::op8_A_obj;
			break;

		case inst_state::op8_A_off8:
			set_adr_off8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::op8_A_obj;
			break;

		case inst_state::op8_A_N8:
			m_tmp[0] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::op8_A_N8_2;
			break;

		case inst_state::op8_A_N8_2:
			m_acc = (m_acc & 0xff00) | do_aluop_byte(m_acc & 0x00ff, m_tmp[0]);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::LB_A_ind_PRn:
			m_adr = m_data_cache.read_word(addr_pr(m_inst));
			m_seg = get_seg(m_adr);
			acc_load_byte(data_read_byte(m_seg, m_adr));
			m_icount -= 4;
			next_inst();
			break;

		case inst_state::LB_A_ind_DP_post:
			m_tmp[1] = addr_pr(2);
			m_adr = m_data_cache.read_word(m_tmp[1]);
			m_seg = get_seg(m_adr);
			m_data_cache.write_word(m_tmp[1], m_adr + (BIT(m_inst, 1) ? 1 : -1));
			acc_load_byte(data_read_byte(m_seg, m_adr));
			m_icount -= 5;
			next_inst();
			break;

		case inst_state::LB_A_obj:
			acc_load_byte(data_read_byte(m_seg, m_adr));
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::SLL_A:
			if (BIT(m_acc, 15))
				m_psw |= 0x8000;
			else
				m_psw &= 0x7fff;
			m_acc <<= 1;
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::SLLB_A:
			if (BIT(m_acc, 7))
				m_psw |= 0x8000;
			else
				m_psw &= 0x7fff;
			m_acc = (m_acc & 0xff00) | ((m_acc << 1) & 0x00ff);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::SRL_A:
			if (BIT(m_acc, 0))
				m_psw |= 0x8000;
			else
				m_psw &= 0x7fff;
			m_acc >>= 1;
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::SRLB_A:
			if (BIT(m_acc, 0))
				m_psw |= 0x8000;
			else
				m_psw &= 0x7fff;
			m_acc = (m_acc & 0xff00) | (m_acc & 0x00ff) >> 1;
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::prefix_ind8_PRn:
			m_adr = m_data_cache.read_word(addr_pr(m_inst));
			m_seg = get_seg(m_adr);
			m_inst = m_fetch_byte;
			m_icount -= 4;
			m_state = s_prefixed_inst_decode[0][m_inst];
			break;

		case inst_state::prefix_ind16_PRn:
			m_adr = m_data_cache.read_word(addr_pr(m_inst));
			m_seg = get_seg(m_adr);
			m_inst = m_fetch_byte;
			m_icount -= 4;
			m_state = s_prefixed_inst_decode[1][m_inst];
			break;

		case inst_state::prefix_ind_DP_post:
			m_tmp[1] = addr_pr(2);
			m_adr = m_data_cache.read_word(m_tmp[1]);
			m_seg = get_seg(m_adr);
			if (BIT(m_inst, 4))
				m_data_cache.write_word(m_tmp[1], m_adr + (BIT(m_inst, 1) ? 1 : -1));
			else
				m_data_cache.write_word(m_tmp[1], m_adr + (BIT(m_inst, 1) ? 2 : -2));
			m_icount -= 3;
			m_state = BIT(m_inst, 4) ? inst_state::prefix_obj8 : inst_state::prefix_obj16;
			break;

		case inst_state::prefix_fix8:
			set_adr_fix8(m_fetch_byte);
			m_icount -= 2;
			m_state = BIT(m_inst, 4) ? inst_state::prefix_obj8 : inst_state::prefix_obj16;
			break;

		case inst_state::prefix_off8:
			set_adr_off8(m_fetch_byte);
			m_icount -= 2;
			m_state = BIT(m_inst, 4) ? inst_state::prefix_obj8 : inst_state::prefix_obj16;
			break;

		case inst_state::prefix_sfr8:
			set_adr_sfr8(m_fetch_byte);
			m_icount -= 2;
			m_state = BIT(m_inst, 4) ? inst_state::prefix_obj8 : inst_state::prefix_obj16;
			break;

		case inst_state::prefix_dir:
			m_tmp[0] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::prefix_dir_2;
			break;

		case inst_state::prefix_dir_2:
			m_adr = m_fetch_byte << 8 | m_tmp[0];
			m_seg = get_seg(m_adr);
			m_icount -= 2;
			m_state = BIT(m_inst, 4) ? inst_state::prefix_obj8 : inst_state::prefix_obj16;
			break;

		case inst_state::prefix_D16_PRn:
			m_tmp[0] = m_fetch_byte;
			m_tmp[1] = m_data_cache.read_word(addr_pr(m_inst));
			m_icount -= 2;
			m_state = inst_state::prefix_D16_PRn_2;
			break;

		case inst_state::prefix_D16_PRn_2:
			m_adr = (m_fetch_byte << 8 | m_tmp[0]) + m_tmp[1];
			m_seg = get_seg(m_adr);
			m_icount -= 2;
			m_state = BIT(m_inst, 4) ? inst_state::prefix_obj8 : inst_state::prefix_obj16;
			break;

		case inst_state::prefix_obj8:
			m_inst = m_fetch_byte;
			m_icount -= 2;
			m_state = s_prefixed_inst_decode[0][m_inst];
			break;

		case inst_state::prefix_obj16:
			m_inst = m_fetch_byte;
			m_icount -= 2;
			m_state = s_prefixed_inst_decode[1][m_inst];
			break;

		case inst_state::prefix_ind8_X1_A:
			m_tmp[0] = m_data_cache.read_word(addr_pr(0));
			m_adr = m_tmp[0] + (m_acc & 0x00ff);
			m_seg = get_seg(m_adr);
			m_inst = m_fetch_byte;
			m_icount -= 6;
			m_state = s_prefixed_inst_decode[0][m_inst];
			break;

		case inst_state::prefix_ind16_X1_A:
			m_tmp[0] = m_data_cache.read_word(addr_pr(0));
			m_adr = m_tmp[0] + (m_acc & 0x00ff);
			m_seg = get_seg(m_adr);
			m_inst = m_fetch_byte;
			m_icount -= 6;
			m_state = s_prefixed_inst_decode[1][m_inst];
			break;

		case inst_state::prefix_ind8_X1_R0:
			m_tmp[0] = m_data_cache.read_word(addr_pr(0));
			m_adr = m_tmp[0] + m_data_cache.read_byte(addr_lr(0));
			m_seg = get_seg(m_adr);
			m_inst = m_fetch_byte;
			m_icount -= 6;
			m_state = s_prefixed_inst_decode[0][m_inst];
			break;

		case inst_state::prefix_ind16_X1_R0:
			m_tmp[0] = m_data_cache.read_word(addr_pr(0));
			m_adr = m_tmp[0] + m_data_cache.read_byte(addr_lr(0));
			m_seg = get_seg(m_adr);
			m_inst = m_fetch_byte;
			m_icount -= 6;
			m_state = s_prefixed_inst_decode[1][m_inst];
			break;

		case inst_state::ROL_A:
			m_acc = do_rol(m_acc);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::ROLB_A:
			m_acc = (m_acc & 0xff00) | do_rolb(m_acc & 0x00ff);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::prefix_A16:
			set_adr_sfr8(0x06);
			m_inst = m_fetch_byte;
			m_icount -= 2;
			m_state = s_prefixed_inst_decode[m_inst >= 0x70][m_inst];
			break;

		case inst_state::prefix_A8:
			set_adr_sfr8(0x06);
			m_inst = m_fetch_byte;
			m_icount -= 2;
			m_state = s_prefixed_inst_decode[0][m_inst];
			break;

		case inst_state::ROR_A:
			m_acc = do_ror(m_acc);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::RORB_A:
			m_acc = (m_acc & 0xff00) | do_rorb(m_acc & 0x00ff);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::INCB_DECB_Rn:
			m_adr = addr_lr(m_inst);
			m_tmp[0] = m_data_cache.read_byte(m_adr);
			if (BIT(m_inst, 4))
				m_tmp[0] = do_decb(m_tmp[0]);
			else
				m_tmp[0] = do_incb(m_tmp[0]);
			m_data_cache.write_byte(m_adr, m_tmp[0]);
			m_icount -= 3;
			next_inst();
			break;

		case inst_state::CMP_fix8_N16:
			set_adr_fix8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::CMP_obj_N16;
			break;

		case inst_state::CMP_off8_N16:
			set_adr_off8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::CMP_obj_N16;
			break;

		case inst_state::MOV_sfr8_N16:
			set_adr_sfr8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::MOV_obj_N16;
			break;

		case inst_state::MOV_off8_N16:
			set_adr_off8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::MOV_obj_N16;
			break;

		case inst_state::ST_A_D16_X1:
			m_tmp[0] = m_data_cache.read_word(addr_pr(0));
			m_tmp[1] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::ST_A_D16_X1_2;
			break;

		case inst_state::ST_A_D16_X1_2:
			m_adr = m_tmp[0] + (m_fetch_byte << 8 | m_tmp[1]);
			m_seg = get_seg(m_adr);
			m_tmp[0] = m_acc;
			m_icount -= 2;
			m_state = inst_state::store_word;
			break;

		case inst_state::ST_A_n7_ind_PR:
			m_adr = m_data_cache.read_word(addr_pr(2 + BIT(m_fetch_byte, 7))) + util::sext(m_fetch_byte, 7);
			m_seg = get_seg(m_adr);
			m_tmp[0] = m_acc;
			m_icount -= 4;
			m_state = inst_state::store_word;
			break;

		case inst_state::STB_A_D16_X1:
			m_tmp[0] = m_data_cache.read_word(addr_pr(0));
			m_tmp[1] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::STB_A_D16_X1_2;
			break;

		case inst_state::STB_A_D16_X1_2:
			m_adr = m_tmp[0] + (m_fetch_byte << 8 | m_tmp[1]);
			m_seg = get_seg(m_adr);
			m_tmp[0] = m_acc & 0x00ff;
			m_icount -= 2;
			m_state = inst_state::store_byte;
			break;

		case inst_state::STB_A_n7_ind_PR:
			m_adr = m_data_cache.read_word(addr_pr(2 + BIT(m_fetch_byte, 7))) + util::sext(m_fetch_byte, 7);
			m_seg = get_seg(m_adr);
			m_tmp[0] = m_acc;
			m_icount -= 4;
			m_state = inst_state::store_byte;
			break;

		case inst_state::RC_SC:
			if (BIT(m_inst, 0))
				m_psw |= 0x8000;
			else
				m_psw &= 0x7fff;
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::INC_A:
			m_acc = do_inc(m_acc);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::NEG_A:
			m_acc = do_sub(0, m_acc);
			m_icount -= 3;
			next_inst();
			break;

		case inst_state::INCB_A:
			m_acc = (m_acc & 0xff00) | do_incb(m_acc & 0x00ff);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::NEGB_A:
			m_acc = (m_acc & 0xff00) | do_subb(0, m_acc & 0x00ff);
			m_icount -= 3;
			next_inst();
			break;

		case inst_state::CMPB_fix8_N8:
			set_adr_fix8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::CMPB_obj_N8;
			break;

		case inst_state::CMPB_off8_N8:
			set_adr_off8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::CMPB_obj_N8;
			break;

		case inst_state::MOVB_sfr8_N8:
			set_adr_sfr8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::MOVB_obj_N8;
			break;

		case inst_state::MOVB_off8_N8:
			set_adr_off8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::MOVB_obj_N8;
			break;

		case inst_state::RDD_SDD:
			if (BIT(m_inst, 0))
				m_psw |= 0x1000;
			else
				m_psw &= 0xefff;
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::DI_EI:
			if (BIT(m_inst, 0))
				m_psw |= 0x0100;
			else
				m_psw &= 0xfeff;
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::DEC_A:
			m_acc = do_dec(m_acc);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::SWAP:
			m_acc = swapendian_int16(m_acc);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::DECB_A:
			m_acc = (m_acc & 0xff00) | do_decb(m_acc & 0x00ff);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::VCAL:
			m_adr = m_ssp;
			data_write_word(0, m_adr, m_pc - 1);
			if (BIT(m_memscon, 1))
			{
				m_adr -= 2;
				data_write_word(0, m_adr, m_csr);
			}
			m_ssp = m_adr - 2;
			m_csr = 0;
			m_pc = 0x004a + (m_inst & 0x0f) * 2;
			m_state = inst_state::J_Cadr; // hack
			m_icount -= 3;
			break;

		case inst_state::JGT_JLE:
			if ((BIT(m_psw, 14) || BIT(m_psw, 15)) == BIT(m_inst, 2))
			{
				m_pc += s8(m_fetch_byte);
				m_icount -= 4;
			}
			else
				m_icount -= 2;
			m_state = inst_state::NOP;
			break;

		case inst_state::JZ_JNZ:
			if (BIT(m_psw, 14) != BIT(m_inst, 2))
			{
				m_pc += s8(m_fetch_byte);
				m_icount -= 4;
			}
			else
				m_icount -= 2;
			m_state = inst_state::NOP;
			break;

		case inst_state::JCY_JNC:
			if (BIT(m_psw, 15) != BIT(m_inst, 2))
			{
				m_pc += s8(m_fetch_byte);
				m_icount -= 4;
			}
			else
				m_icount -= 2;
			m_state = inst_state::NOP;
			break;

		case inst_state::JNS_JPS:
			if (BIT(m_psw, 11) != BIT(m_inst, 2))
			{
				m_pc += s8(m_fetch_byte);
				m_icount -= 4;
			}
			else
				m_icount -= 2;
			m_state = inst_state::NOP;
			break;

		case inst_state::L_A_N16:
			m_tmp[0] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::L_A_N16_2;
			break;

		case inst_state::L_A_N16_2:
			m_tmp[0] |= m_fetch_byte << 8;
			m_icount -= 2;
			m_state = inst_state::L_A_N16_3;
			break;

		case inst_state::L_A_N16_3:
			acc_load_word(m_tmp[0]);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::LB_A_N8:
			m_tmp[0] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::LB_A_N8_2;
			break;

		case inst_state::LB_A_N8_2:
			acc_load_byte(m_tmp[0]);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::CLR_A:
			acc_load_word(0);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::CLRB_A:
			acc_load_byte(0);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::EXTND:
			if (BIT(m_acc, 7))
			{
				m_acc |= 0xff00;
				m_psw |= 0x2000;
			}
			else
			{
				m_acc &= 0x00ff;
				m_psw &= 0xdfff;
			}
			m_psw |= 0x1000;
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::CPL_C:
			m_psw ^= 0x8000;
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::CAL_Cadr:
			m_tmp[0] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::CAL_Cadr_2;
			break;

		case inst_state::CAL_Cadr_2:
			m_tmp[0] |= m_fetch_byte << 8;
			m_adr = m_ssp;
			m_seg = 0;
			m_ssp -= 2;
			std::swap(m_pc, m_tmp[0]);
			m_icount -= 5;
			m_state = inst_state::store_word;
			break;

		case inst_state::BRK:
			reset(); // hacky method of performing a "software system reset"
			m_pc = 0x0002;
			m_icount -= 2;
			m_state = inst_state::J_Cadr;
			break;

		case inst_state::RB_SB_obj_bit:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			if (BIT(m_tmp[0], m_inst & 0x07))
				m_psw &= 0xbfff;
			else
				m_psw |= 0x4000;
			if (BIT(m_inst, 3))
				m_tmp[0] |= 1 << (m_inst & 0x07);
			else
				m_tmp[0] &= ~(1 << (m_inst & 0x07));
			m_icount -= 3;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::MB_C_obj_bit:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			if (BIT(m_tmp[0], m_inst & 0x07))
				m_psw |= 0x8000;
			else
				m_psw &= 0x7fff;
			m_icount -= 3;
			next_inst();
			break;

		case inst_state::MB_obj_bit_C:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			if (BIT(m_psw, 15))
				m_tmp[0] |= 1 << (m_inst & 0x07);
			else
				m_tmp[0] &= ~(1 << (m_inst & 0x07));
			m_icount -= 3;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::JBR_JBS_obj_bit:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			if (BIT(m_tmp[0], m_inst & 0x07) == BIT(m_inst, 3))
			{
				m_pc += s8(m_fetch_byte);
				m_icount -= 6;
			}
			else
				m_icount -= 2;
			m_state = inst_state::NOP;
			break;

		case inst_state::JBRS_JBSR_obj_bit:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			if (BIT(m_tmp[0], m_inst & 0x07) == BIT(m_inst, 3))
			{
				m_tmp[0] ^= 1 << (m_inst & 0x07);
				m_pc += s8(m_fetch_byte);
				m_icount -= 8;
				m_state = inst_state::store_byte;
			}
			else
			{
				m_icount -= 2;
				m_state = inst_state::NOP;
			}
			break;

		case inst_state::BAND_C_obj_bit:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			if (BIT(m_tmp[0], m_inst & 0x07) == BIT(m_inst, 3))
				m_psw &= 0x7fff;
			m_icount -= 3;
			next_inst();
			break;

		case inst_state::BOR_C_obj_bit:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			if (BIT(m_tmp[0], m_inst & 0x07) != BIT(m_inst, 3))
				m_psw |= 0x8000;
			m_icount -= 3;
			next_inst();
			break;

		case inst_state::BXOR_C_obj_bit:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			if (BIT(m_tmp[0], m_inst & 0x07))
				m_psw ^= 0x8000;
			m_icount -= 3;
			next_inst();
			break;

		case inst_state::MOV_ERn_PRn_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			m_data_cache.write_word(BIT(m_inst, 2) ? addr_pr(m_inst) : addr_er(m_inst), m_tmp[0]);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::MOVB_Rn_obj:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			m_data_cache.write_byte(addr_lr(m_inst), m_tmp[0]);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::op16_obj_fix8:
			m_tmp[1] = m_data_cache.read_word(0x0200 | m_fetch_byte);
			m_icount -= 2;
			m_state = (m_inst & 0x90) == 0x90 ? inst_state::CMP_obj1_obj2 : inst_state::op16_obj1_obj2;
			break;

		case inst_state::op8_obj_fix8:
			m_tmp[1] = m_data_cache.read_byte(0x0200 | m_fetch_byte);
			m_icount -= 2;
			m_state = (m_inst & 0x90) == 0x90 ? inst_state::CMPB_obj1_obj2 : inst_state::op8_obj1_obj2;
			break;

		case inst_state::op16_obj_off8:
			m_tmp[1] = data_read_word(get_seg((m_lrb & 0xff00) | m_fetch_byte), (m_lrb & 0xff00) | m_fetch_byte);
			m_icount -= 2;
			m_state = (m_inst & 0x90) == 0x90 ? inst_state::CMP_obj1_obj2 : inst_state::op16_obj1_obj2;
			break;

		case inst_state::op8_obj_off8:
			m_tmp[1] = data_read_byte(get_seg((m_lrb & 0xff00) | m_fetch_byte), (m_lrb & 0xff00) | m_fetch_byte);
			m_icount -= 2;
			m_state = (m_inst & 0x90) == 0x90 ? inst_state::CMPB_obj1_obj2 : inst_state::op8_obj1_obj2;
			break;

		case inst_state::op16_obj_sfr8:
			m_tmp[1] = data_read_word(0, m_fetch_byte);
			m_icount -= 2;
			m_state = (m_inst & 0x90) == 0x90 ? inst_state::CMP_obj1_obj2 : inst_state::op16_obj1_obj2;
			break;

		case inst_state::op8_obj_sfr8:
			m_tmp[1] = data_read_byte(0, m_fetch_byte);
			m_icount -= 2;
			m_state = (m_inst & 0x90) == 0x90 ? inst_state::CMPB_obj1_obj2 : inst_state::op8_obj1_obj2;
			break;

		case inst_state::op16_obj1_obj2:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			m_tmp[0] = do_aluop_word(m_tmp[0], m_tmp[1]);
			m_icount -= 3;
			next_inst_and_store_word(m_tmp[0]);
			break;

		case inst_state::op8_obj1_obj2:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			m_tmp[0] = do_aluop_byte(m_tmp[0], m_tmp[1]);
			m_icount -= 3;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::op16_obj_N16:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			m_tmp[1] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::op16_obj_N16_2;
			break;

		case inst_state::op16_obj_N16_2:
			m_tmp[1] |= m_fetch_byte << 8;
			m_icount -= 2;
			m_state = inst_state::op16_obj_N16_3;
			break;

		case inst_state::op16_obj_N16_3:
			m_tmp[0] = do_aluop_word(m_tmp[0], m_tmp[1]);
			m_icount -= 2;
			next_inst_and_store_word(m_tmp[0]);
			break;

		case inst_state::op8_obj_N8:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			m_tmp[1] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::op8_obj_N8_2;
			break;

		case inst_state::op8_obj_N8_2:
			m_tmp[0] = do_aluop_byte(m_tmp[0], m_tmp[1]);
			m_icount -= 2;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::op16_obj_A:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			m_tmp[0] = do_aluop_word(m_tmp[0], m_acc);
			m_icount -= 2;
			next_inst_and_store_word(m_tmp[0]);
			break;

		case inst_state::op8_obj_A:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			m_tmp[0] = do_aluop_byte(m_tmp[0], m_acc & 0x00ff);
			m_icount -= 2;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::op16_A_obj:
			m_acc = do_aluop_word(m_acc, data_read_word(m_seg, m_adr));
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::op8_A_obj:
			m_acc = (m_acc & 0xff00) | do_aluop_byte(m_acc & 0x00ff, data_read_byte(m_seg, m_adr));
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::MOV_fix8_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			set_adr_fix8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::store_word;
			break;

		case inst_state::MOV_off8_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			set_adr_off8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::store_word;
			break;

		case inst_state::MOVB_fix8_obj:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			set_adr_fix8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::store_byte;
			break;

		case inst_state::MOVB_off8_obj:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			set_adr_off8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::store_byte;
			break;

		case inst_state::MOV_ind_PRn_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			m_adr = m_data_cache.read_word(addr_pr(m_inst));
			m_seg = get_seg(m_adr);
			m_icount -= 4;
			next_inst_and_store_word(m_tmp[0]);
			break;

		case inst_state::MOV_ind_DP_post_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			m_tmp[1] = addr_pr(2);
			m_adr = m_data_cache.read_word(m_tmp[1]);
			m_seg = get_seg(m_adr);
			m_data_cache.write_word(m_tmp[1], m_tmp[0] + (BIT(m_inst, 1) ? 2 : -2));
			m_icount -= 5;
			next_inst_and_store_word(m_tmp[0]);
			break;

		case inst_state::MOVB_ind_PRn_obj:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			m_adr = m_data_cache.read_word(addr_pr(m_inst));
			m_seg = get_seg(m_adr);
			m_icount -= 4;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::MOVB_ind_DP_post_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			m_tmp[1] = addr_pr(2);
			m_adr = m_data_cache.read_word(m_tmp[1]);
			m_seg = get_seg(m_adr);
			m_data_cache.write_word(m_tmp[1], m_adr + (BIT(m_inst, 1) ? 1 : -1));
			m_icount -= 5;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::SLL_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			m_psw &= 0x7fff;
			for (int n = m_inst & 0x03; n >= 0; n--)
			{
				if (BIT(m_tmp[0], 15))
					m_psw |= 0x8000;
				m_tmp[0] <<= 1;
				--m_icount;
			}
			m_icount -= 2;
			next_inst_and_store_word(m_tmp[0]);
			break;

		case inst_state::SLLB_obj:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			m_psw &= 0x7fff;
			for (int n = m_inst & 0x03; n >= 0; n--)
			{
				if (BIT(m_tmp[0], 7))
					m_psw |= 0x8000;
				m_tmp[0] <<= 1;
				--m_icount;
			}
			m_icount -= 2;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::CMP_obj1_obj2:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			(void)do_sub(m_tmp[0], m_tmp[1]);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::CMPB_obj1_obj2:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			(void)do_subb(m_tmp[0], m_tmp[1]);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::CMP_obj_N16:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			m_tmp[1] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::CMP_obj_N16_2;
			break;

		case inst_state::CMP_obj_N16_2:
			m_tmp[1] |= m_fetch_byte << 8;
			m_icount -= 2;
			m_state = inst_state::CMP_obj_N16_3;
			break;

		case inst_state::CMP_obj_N16_3:
			(void)do_sub(m_tmp[0], m_tmp[1]);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::CMPB_obj_N8:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			m_tmp[1] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::CMPB_obj_N8_2;
			break;

		case inst_state::CMPB_obj_N8_2:
			(void)do_subb(m_tmp[0], m_tmp[1]);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::CMP_obj_A:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			(void)do_sub(m_tmp[0], m_acc);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::CMPB_obj_A:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			(void)do_subb(m_tmp[0], m_acc & 0x00ff);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::MOV_sfr8_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			set_adr_sfr8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::store_word;
			break;

		case inst_state::MOV_A_obj:
			m_acc = data_read_word(m_seg, m_adr);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::MOVB_sfr8_obj:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			set_adr_sfr8(m_fetch_byte);
			m_icount -= 2;
			m_state = inst_state::store_byte;
			break;

		case inst_state::MOVB_A_obj:
			m_acc = (m_acc & 0xff00) | data_read_byte(m_seg, m_adr);
			m_icount -= 2;
			next_inst();
			break;

		case inst_state::MOV_D16_PRn_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			m_tmp[1] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::MOV_dir_obj_2;
			break;

		case inst_state::MOV_D16_PRn_obj_2:
			m_adr = (m_fetch_byte << 8 | m_tmp[1]) + m_data_cache.read_word(addr_pr(m_inst));
			m_seg = get_seg(m_adr);
			m_icount -= 2;
			m_state = inst_state::store_word;
			break;

		case inst_state::MOVB_D16_PRn_obj:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			m_tmp[1] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::MOV_dir_obj_2;
			break;

		case inst_state::MOVB_D16_PRn_obj_2:
			m_adr = (m_fetch_byte << 8 | m_tmp[1]) + m_data_cache.read_word(addr_pr(m_inst));
			m_seg = get_seg(m_adr);
			m_icount -= 2;
			m_state = inst_state::store_byte;
			break;

		case inst_state::MOV_n7_ind_PR_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			m_adr = m_data_cache.read_word(addr_pr(2 + BIT(m_fetch_byte, 7))) + util::sext(m_fetch_byte, 7);
			m_seg = get_seg(m_adr);
			m_icount -= 4;
			m_state = inst_state::store_word;
			break;

		case inst_state::MOVB_n7_ind_PR_obj:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			m_adr = m_data_cache.read_word(addr_pr(2 + BIT(m_fetch_byte, 7))) + util::sext(m_fetch_byte, 7);
			m_seg = get_seg(m_adr);
			m_icount -= 4;
			m_state = inst_state::store_byte;
			break;

		case inst_state::MOV_dir_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			m_tmp[1] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::MOV_dir_obj_2;
			break;

		case inst_state::MOV_dir_obj_2:
			m_adr = m_fetch_byte << 8 | m_tmp[1];
			m_seg = get_seg(m_adr);
			m_icount -= 2;
			m_state = inst_state::store_word;
			break;

		case inst_state::MOVB_dir_obj:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			m_tmp[1] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::MOVB_dir_obj_2;
			break;

		case inst_state::MOVB_dir_obj_2:
			m_adr = m_fetch_byte << 8 | m_tmp[1];
			m_seg = get_seg(m_adr);
			m_icount -= 2;
			m_state = inst_state::store_byte;
			break;

		case inst_state::SRL_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			for (int n = m_inst & 0x03; n >= 0; n--)
			{
				if (BIT(m_tmp[0], 0))
					m_psw |= 0x8000;
				else
					m_psw &= 0x7fff;
				m_tmp[0] >>= 1;
				--m_icount;
			}
			m_icount -= 2;
			next_inst_and_store_word(m_tmp[0]);
			break;

		case inst_state::SRLB_obj:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			for (int n = m_inst & 0x03; n >= 0; n--)
			{
				if (BIT(m_tmp[0], 0))
					m_psw |= 0x8000;
				else
					m_psw &= 0x7fff;
				m_tmp[0] >>= 1;
				--m_icount;
			}
			m_icount -= 2;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::TJNZ_TJZ:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			if (BIT(m_inst, 0) ? m_tmp[0] == 0 : m_tmp[0] != 0)
			{
				m_pc += s8(m_fetch_byte);
				m_icount -= 4;
			}
			else
				m_icount -= 2;
			m_state = inst_state::NOP;
			break;

		case inst_state::TJNZB_TJZB:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			if (BIT(m_inst, 0) ? m_tmp[0] == 0 : m_tmp[0] != 0)
			{
				m_pc += s8(m_fetch_byte);
				m_icount -= 4;
			}
			else
				m_icount -= 2;
			m_state = inst_state::NOP;
			break;

		case inst_state::DIV_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			do_div(m_tmp[0]);
			m_icount -= 42;
			next_inst();
			break;

		case inst_state::DIVB_obj:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			do_divb(m_tmp[0]);
			m_icount -= 22;
			next_inst();
			break;

		case inst_state::MUL_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			do_mul(m_tmp[0]);
			m_icount -= 21; // TODO: 3 if high-speed multiplier provided
			next_inst();
			break;

		case inst_state::MULB_obj:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			m_acc = (m_acc & 0x00ff) * m_tmp[0];
			if (m_acc == 0)
				m_psw |= 0x4000;
			else
				m_psw &= 0xbfff;
			m_icount -= 12; // TODO: 2 if high-speed multiplier provided
			next_inst();
			break;

		case inst_state::MOV_obj_A:
			m_icount -= 2;
			next_inst_and_store_word(m_acc);
			break;

		case inst_state::MOV_obj_N16:
			m_tmp[0] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::MOV_obj_N16_2;
			break;

		case inst_state::MOV_obj_N16_2:
			m_tmp[0] |= m_fetch_byte << 8;
			m_icount -= 2;
			m_state = inst_state::store_word;
			break;

		case inst_state::MOVB_obj_A:
			m_icount -= 2;
			next_inst_and_store_byte(m_acc & 0x00ff);
			break;

		case inst_state::MOVB_obj_N8:
			m_tmp[0] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::store_byte;
			break;

		case inst_state::ROL_ROR_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			for (int n = m_inst & 0x03; n >= 0; n--)
			{
				m_tmp[0] = BIT(m_inst, 4) ? do_ror(m_tmp[0]) : do_rol(m_tmp[0]);
				--m_icount;
			}
			m_icount -= 2;
			next_inst_and_store_word(m_tmp[0]);
			break;

		case inst_state::ROLB_RORB_obj:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			for (int n = m_inst & 0x03; n >= 0; n--)
			{
				m_tmp[0] = BIT(m_inst, 4) ? do_rorb(m_tmp[0]) : do_rolb(m_tmp[0]);
				--m_icount;
			}
			m_icount -= 2;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::CMPC_LC_A_Tadr:
			m_tmp[0] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::CMPC_LC_A_Tadr_2;
			break;

		case inst_state::CMPC_LC_A_Tadr_2:
			m_tmp[0] |= m_fetch_byte << 8;
			m_icount -= 2;
			m_state = BIT(m_inst, 0) ? inst_state::LC_A_obj_3 : inst_state::CMPC_A_obj_3;
			break;

		case inst_state::CMPCB_LCB_A_Tadr:
			m_tmp[0] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::CMPCB_LCB_A_Tadr_2;
			break;

		case inst_state::CMPCB_LCB_A_Tadr_2:
			m_tmp[0] |= m_fetch_byte << 8;
			m_icount -= 2;
			m_state = BIT(m_inst, 0) ? inst_state::LCB_A_obj_3 : inst_state::CMPCB_A_obj_3;
			break;

		case inst_state::SBR_RBR_obj:
			m_adr += (m_acc & 0x00f8) >> 3;
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			if (BIT(m_tmp[0], m_acc & 0x0007))
				m_psw &= 0xbfff;
			else
				m_psw |= 0x4000;
			if (BIT(m_inst, 0))
				m_tmp[0] &= ~(1 << (m_acc & 0x0007));
			else
				m_tmp[0] |= 1 << (m_acc & 0x0007);
			m_icount -= 5;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::MBR_C_obj:
			m_adr += (m_acc & 0x00f8) >> 3;
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			if (BIT(m_tmp[0], m_acc & 0x0007))
				m_psw |= 0x8000;
			else
				m_psw &= 0x7fff;
			m_icount -= 6;
			next_inst();
			break;

		case inst_state::MBR_obj_C:
			m_adr += (m_acc & 0x00f8) >> 3;
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			if (BIT(m_psw, 15))
				m_tmp[0] |= 1 << (m_acc & 0x0007);
			else
				m_tmp[0] &= ~(1 << (m_acc & 0x0007));
			m_icount -= 5;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::INC_DEC_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			if (BIT(m_inst, 4))
				m_tmp[0] = do_dec(m_tmp[0]);
			else
				m_tmp[0] = do_inc(m_tmp[0]);
			m_icount -= 2;
			next_inst_and_store_word(m_tmp[0]);
			break;

		case inst_state::CLR_FILL_obj:
			m_tmp[0] = BIT(m_inst, 4) ? 0xffff : 0;
			m_icount -= 2;
			next_inst_and_store_word(m_tmp[0]);
			break;

		case inst_state::XCHG_A_obj:
			m_tmp[1] = data_read_word(m_seg, m_adr);
			m_tmp[0] = m_acc;
			m_acc = m_tmp[1];
			m_icount -= 3;
			next_inst_and_store_word(m_tmp[0]);
			break;

		case inst_state::TBR_obj:
			m_adr += (m_acc & 0x00f8) >> 3;
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			if (BIT(m_tmp[0], m_acc & 0x0007))
				m_psw &= 0xbfff;
			else
				m_psw |= 0x4000;
			m_icount -= 5;
			next_inst();
			break;

		case inst_state::J_ind_obj:
			m_pc = data_read_word(m_seg, m_adr);
			m_icount -= 2;
			m_state = inst_state::NOP;
			break;

		case inst_state::CMPC_LC_A_ind_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			m_tmp[1] = data_read_word(0x8000 | m_tsr, m_tmp[0]);
			if (BIT(m_inst, 1))
			{
				m_acc = m_tmp[1];
				if (m_tmp[1] == 0)
					m_psw |= 0x4000;
				else
					m_psw &= 0xbfff;
			}
			else
				(void)do_sub(m_acc, m_tmp[1]);
			m_icount -= 9;
			next_inst();
			break;

		case inst_state::CMPCB_LCB_A_ind_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			m_tmp[1] = data_read_byte(0x8000 | m_tsr, m_tmp[0]);
			m_acc = (m_acc & 0xff00) | m_tmp[1];
			if (BIT(m_inst, 1))
			{
				if (m_tmp[1] == 0)
					m_psw |= 0x4000;
				else
					m_psw &= 0xbfff;
			}
			else
				(void)do_subb(m_acc & 0x00ff, m_tmp[1]);
			m_icount -= 6;
			next_inst();
			break;

		case inst_state::CMPC_LC_A_T16_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			m_tmp[1] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::CMPC_LC_A_T16_obj_2;
			break;

		case inst_state::CMPC_LC_A_T16_obj_2:
			m_tmp[0] += m_fetch_byte << 8 | m_tmp[1];
			m_icount -= 2;
			if (BIT(m_inst, 4))
				m_state = BIT(m_inst, 1) ? inst_state::LCB_A_obj_3 : inst_state::CMPCB_A_obj_3;
			else
				m_state = BIT(m_inst, 1) ? inst_state::LC_A_obj_3 : inst_state::CMPC_A_obj_3;
			break;

		case inst_state::CMPC_A_obj_3:
			m_tmp[1] = data_read_word(0x8000 | m_tsr, m_tmp[0]);
			(void)do_sub(m_acc, m_tmp[1]);
			m_icount -= 9;
			next_inst();
			break;

		case inst_state::CMPCB_A_obj_3:
			m_tmp[1] = data_read_byte(0x8000 | m_tsr, m_tmp[0]);
			(void)do_subb(m_acc & 0x00ff, m_tmp[1]);
			m_icount -= 6;
			next_inst();
			break;

		case inst_state::LC_A_obj_3:
			m_tmp[1] = data_read_word(0x8000 | m_tsr, m_tmp[0]);
			m_acc = m_tmp[1];
			if (m_tmp[1] == 0)
				m_psw |= 0x4000;
			else
				m_psw &= 0xbfff;
			m_icount -= 9;
			next_inst();
			break;

		case inst_state::LCB_A_obj_3:
			m_tmp[1] = data_read_byte(0x8000 | m_tsr, m_tmp[0]);
			m_acc = (m_acc & 0xff00) | m_tmp[1];
			if (m_tmp[1] == 0)
				m_psw |= 0x4000;
			else
				m_psw &= 0xbfff;
			m_icount -= 6;
			next_inst();
			break;

		case inst_state::INCB_DECB_obj:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			if (BIT(m_inst, 4))
				m_tmp[0] = do_decb(m_tmp[0]);
			else
				m_tmp[0] = do_incb(m_tmp[0]);
			m_icount -= 2;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::CLRB_FILLB_obj:
			m_tmp[0] = BIT(m_inst, 4) ? 0xff : 0;
			m_icount -= 2;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::XCHGB_A_obj:
			m_tmp[1] = data_read_byte(m_seg, m_adr);
			m_tmp[0] = m_acc & 0x00ff;
			m_acc = (m_acc & 0xff00) | m_tmp[1];
			m_icount -= 3;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::DJNZ_obj:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			--m_tmp[0];
			if (m_tmp[0] != 0)
			{
				m_pc += s8(m_fetch_byte);
				m_icount -= 6;
			}
			else
				m_icount -= 3;
			m_state = inst_state::store_byte;
			break;

		case inst_state::CAL_ind_obj:
			m_tmp[0] = m_pc - 1;
			m_pc = data_read_word(m_seg, m_adr);
			m_adr = m_ssp;
			m_seg = 0;
			m_ssp -= 2;
			m_icount -= 3;
			m_state = inst_state::store_word;
			break;

		case inst_state::SRA_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			for (int n = m_inst & 0x03; n >= 0; n--)
			{
				if (BIT(m_tmp[0], 0))
					m_psw |= 0x8000;
				else
					m_psw &= 0x7fff;
				m_tmp[0] = (m_tmp[0] & 0x8000) | m_tmp[0] >> 1;
				--m_icount;
			}
			m_icount -= 2;
			next_inst_and_store_word(m_tmp[0]);
			break;

		case inst_state::SRAB_obj:
			m_tmp[0] = data_read_byte(m_seg, m_adr);
			for (int n = m_inst & 0x03; n >= 0; n--)
			{
				if (BIT(m_tmp[0], 0))
					m_psw |= 0x8000;
				else
					m_psw &= 0x7fff;
				m_tmp[0] = (m_tmp[0] & 0x80) | m_tmp[0] >> 1;
				--m_icount;
			}
			m_icount -= 2;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::MOV_obj_ind_X1_A:
			m_tmp[1] = m_data_cache.read_word(addr_pr(0));
			m_tmp[1] += m_acc & 0x00ff;
			m_tmp[0] = data_read_word(get_seg(m_tmp[1]), m_tmp[1]);
			m_icount -= 6;
			next_inst_and_store_word(m_tmp[0]);
			break;

		case inst_state::MOV_obj_ind_X1_R0:
			m_tmp[1] = m_data_cache.read_word(addr_pr(0));
			m_tmp[1] += m_data_cache.read_byte(addr_lr(0));
			m_tmp[0] = data_read_word(get_seg(m_tmp[1]), m_tmp[1]);
			m_icount -= 6;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::MOVB_obj_ind_X1_A:
			m_tmp[1] = m_data_cache.read_word(addr_pr(0));
			m_tmp[1] += m_acc & 0x00ff;
			m_tmp[0] = data_read_word(get_seg(m_tmp[1]), m_tmp[1]);
			m_icount -= 6;
			next_inst_and_store_word(m_tmp[0]);
			break;

		case inst_state::MOVB_obj_ind_X1_R0:
			m_tmp[1] = m_data_cache.read_word(addr_pr(0));
			m_tmp[1] += m_data_cache.read_byte(addr_lr(0));
			m_tmp[0] = data_read_byte(get_seg(m_tmp[1]), m_tmp[1]);
			m_icount -= 6;
			next_inst_and_store_byte(m_tmp[0]);
			break;

		case inst_state::FJ:
			if (!BIT(m_memscon, 1))
			{
				m_state = inst_state::illegal;
				break;
			}
			m_tmp[0] = m_fetch_byte;
			m_icount -= 2;
			m_state = inst_state::FJ_2;
			break;

		case inst_state::FJ_2:
			m_tmp[0] |= m_fetch_byte << 8;
			m_icount -= 2;
			m_state = inst_state::FJ_3;
			break;

		case inst_state::FJ_3:
			m_csr = m_fetch_byte & 0x0f;
			m_pc = m_tmp[0];
			m_icount -= 3;
			m_state = inst_state::NOP;
			break;

		case inst_state::DIVQ_obj:
			m_tmp[0] = data_read_word(m_seg, m_adr);
			do_divq(m_tmp[0]);
			m_icount -= 42;
			next_inst();
			break;

		case inst_state::JLTS_JGES:
			if ((BIT(m_psw, 9) != BIT(m_psw, 11)) != BIT(m_inst, 1))
			{
				m_pc += s8(m_fetch_byte);
				m_icount -= 6;
			}
			else
				m_icount -= 2;
			m_state = inst_state::NOP;
			break;

		case inst_state::JLES_JGTS:
			if (((BIT(m_psw, 9) != BIT(m_psw, 11)) || BIT(m_psw, 14)) != BIT(m_inst, 1))
			{
				m_pc += s8(m_fetch_byte);
				m_icount -= 6;
			}
			else
				m_icount -= 2;
			m_state = inst_state::NOP;
			break;

		case inst_state::illegal:
			logerror("%06X: Illegal prefixed opcode %02X encountered\n", m_ppc, m_inst);
			reset();
			m_pc = 0x0006;
			m_icount -= 2;
			m_state = inst_state::J_Cadr;
			break;
		}
	}
	while (m_icount > 0);
}


void msm665xx_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = util::string_format("%c%c%c%c%c%c%c%c",
						BIT(m_psw, 15) ? 'C' : '.',
						BIT(m_psw, 14) ? 'Z' : '.',
						BIT(m_psw, 13) ? 'H' : '.',
						BIT(m_psw, 12) ? 'D' : 'd',
						BIT(m_psw, 11) ? 'S' : '.',
						BIT(m_psw, 10) ? 'P' : '.',
						BIT(m_psw, 9)  ? 'V' : '.',
						BIT(m_psw, 8)  ? 'I' : '.');
		break;
	}
}

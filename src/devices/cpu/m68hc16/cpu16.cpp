// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Motorola MC68HC16/CPU16 emulation

    Motorola's description of the internal organization of this CPU defines
    PC, IX, IY, IZ and SP as 16-bit registers, though a 4-bit extension for
    each is also provided, usually in either CCR or K. However, most
    address arithmetic does involve full 20-bit calculations. Accordingly,
    PC, SP and the index registers have been made 20-bit variables here to
    simplify emulation and make debugging more convenient, and the 16 bits
    of K are not stored together but packed and unpacked for PSHM, PULM
    and debugging. (Adding to the confusion is that the typical CPU16 bus
    interface uses 24-bit physical addresses, with or without a MMU.)

    The execution core is currently somewhat incomplete. Indexed jump
    instructions, extended divisions and the MAC unit are not yet
    supported. Interrupts have also not been implemented yet.

***************************************************************************/

#include "emu.h"
#include "cpu16.h"
#include "cpu16dasm.h"


enum class cpu16_device::seq : u16
{
	BOOT_0, BOOT_1, BOOT_2, BOOT_3,
	INVALID,
	PREFETCH_1, PREFETCH_2,
	LOAD8_EA, LOAD8_EXT,
	LOAD16_EA, LOAD16_EA_LSB, LOAD16_EXT, LOAD16_EXT_LSB,
	STORE8_EA, STORE8_EXT,
	STORE16_EA, STORE16_EXT,
	ABA,
	ABX,
	ADDA_IND8, ADDA_IND8_3, ADDA_IMM8, ADDA_IND16, ADDA_IND16_3, ADDA_EXT, ADDA_INDE, ADDA_INDE_3,
	ADDB_IND8, ADDB_IND8_3, ADDB_IMM8, ADDB_IND16, ADDB_IND16_3, ADDB_EXT, ADDB_INDE, ADDB_INDE_3,
	ADDD_IND8, ADDD_IND8_3, ADDD_IMM8, ADDD_IMM16, ADDD_IND16, ADDD_IND16_3, ADDD_EXT, ADDD_INDE, ADDD_INDE_3,
	ADDE_IMM8, ADDE_IMM16, ADDE_IND16, ADDE_IND16_3, ADDE_EXT,
	ADE,
	ADX,
	AEX,
	AIX_IMM8, AIX_IMM16,
	ANDA_IND8, ANDA_IND8_3, ANDA_IMM8, ANDA_IND16, ANDA_IND16_3, ANDA_EXT, ANDA_INDE,
	ANDB_IND8, ANDB_IND8_3, ANDB_IMM8, ANDB_IND16, ANDB_IND16_3, ANDB_EXT, ANDB_INDE,
	ANDD_IND8, ANDD_IND8_3, ANDD_IMM16, ANDD_IND16, ANDD_IND16_3, ANDD_EXT, ANDD_INDE,
	ANDE_IMM16, ANDE_IND16, ANDE_IND16_3, ANDE_EXT,
	ANDP,
	ASR_IND8, ASR_IND8_3, ASR_IND16, ASR_IND16_3, ASR_EXT,
	ASRA,
	ASRB,
	ASRD,
	ASRE,
	ASRW_IND16, ASRW_IND16_3, ASRW_EXT,
	BCLR_IND16, BCLR_IND16_3, BCLR_EXT, BCLR_IND8, BCLR_IND8_2, BCLR_IND8_3,
	BCLRW_IND16, BCLRW_IND16_3, BCLRW_EXT,
	BITA_IND8, BITA_IND8_3, BITA_IMM8, BITA_IND16, BITA_IND16_3, BITA_EXT, BITA_INDE,
	BITB_IND8, BITB_IND8_3, BITB_IMM8, BITB_IND16, BITB_IND16_3, BITB_EXT, BITB_INDE,
	BCC,
	BRSET_IND8, BRSET_IND8_3, BRSET_IND8_4, BRSET_IND16, BRSET_IND16_3, BRSET_EXT,
	BSET_IND16, BSET_IND16_3, BSET_EXT, BSET_IND8, BSET_IND8_2, BSET_IND8_3,
	BSETW_IND16, BSETW_IND16_3, BSETW_EXT,
	BSR, BSR_2, BSR_2B, BSR_3, BSR_3B,
	CBA,
	CLR_IND8, CLR_IND16, CLR_EXT,
	CLRA,
	CLRB,
	CLRD,
	CLRE,
	CLRM,
	CLRW_IND16, CLRW_EXT,
	CMPA_IND8, CMPA_IND8_3, CMPA_IMM8, CMPA_IND16, CMPA_IND16_3, CMPA_EXT, CMPA_INDE,
	CMPB_IND8, CMPB_IND8_3, CMPB_IMM8, CMPB_IND16, CMPB_IND16_3, CMPB_EXT, CMPB_INDE,
	CPD_IND8, CPD_IND8_3, CPD_IMM16, CPD_IND16, CPD_IND16_3, CPD_EXT, CPD_INDE,
	CPE_IMM16, CPE_IND16, CPE_IND16_3, CPE_EXT,
	CPX_IND8, CPX_IND8_3, CPX_IMM16, CPX_IND16, CPX_IND16_3, CPX_EXT,
	DAA,
	DEC_IND8, DEC_IND8_3, DEC_IND16, DEC_IND16_3, DEC_EXT,
	DECA,
	DECB,
	DECW_IND16, DECW_IND16_3, DECW_EXT,
	DIV, DIV_2, DIV_3, DIV_4, DIV_5, DIV_6, DIV_7, DIV_8, DIV_9, DIV_10, DIV_11,
	EMUL, EMUL_2, EMUL_3, EMUL_4, EMUL_5,
	EMULS, EMULS_2, EMULS_3, EMULS_4,
	EORA_IND8, EORA_IND8_3, EORA_IMM8, EORA_IND16, EORA_IND16_3, EORA_EXT, EORA_INDE,
	EORB_IND8, EORB_IND8_3, EORB_IMM8, EORB_IND16, EORB_IND16_3, EORB_EXT, EORB_INDE,
	EORD_IND8, EORD_IND8_3, EORD_IMM16, EORD_IND16, EORD_IND16_3, EORD_EXT, EORD_INDE,
	EORE_IMM16, EORE_IND16, EORE_IND16_3, EORE_EXT,
	INC_IND8, INC_IND8_3, INC_IND16, INC_IND16_3, INC_EXT,
	INCA,
	INCB,
	INCW_IND16, INCW_IND16_3, INCW_EXT,
	JMP_EXT20,
	JSR_EXT20, JSR_EXT20_2, JSR_EXT20_2B, JSR_EXT20_3, JSR_EXT20_3B,
	LBCC,
	LBSR, LBSR_2, LBSR_2B, LBSR_3, LBSR_3B,
	LDAA_IND8, LDAA_IND8_3, LDAA_IMM8, LDAA_IND16, LDAA_IND16_3, LDAA_EXT, LDAA_INDE,
	LDAB_IND8, LDAB_IND8_3, LDAB_IMM8, LDAB_IND16, LDAB_IND16_3, LDAB_EXT, LDAB_INDE,
	LDD_IND8, LDD_IND8_3, LDD_IMM16, LDD_IND16, LDD_IND16_3, LDD_EXT, LDD_INDE,
	LDE_IMM16, LDE_IND16, LDE_IND16_3, LDE_EXT,
	LDED, LDED_2, LDED_2B, LDED_3, LDED_3B, LDED_4,
	LDHI, LDHI_2, LDHI_2B, LDHI_3, LDHI_3B, LDHI_4,
	LDX_IND8, LDX_IND8_3, LDX_IMM16, LDX_IND16, LDX_IND16_3, LDX_EXT,
	MOVB_IXP_EXT, MOVB_IXP_EXT_3, MOVB_EXT_IXP, MOVB_EXT_IXP_3, MOVB_EXT_EXT, MOVB_EXT_EXT_3,
	MOVW_IXP_EXT, MOVW_IXP_EXT_3, MOVW_EXT_IXP, MOVW_EXT_IXP_3, MOVW_EXT_EXT, MOVW_EXT_EXT_3,
	MUL, MUL_2, MUL_3, MUL_4, MUL_5,
	NEG_IND8, NEG_IND8_3, NEG_IND16, NEG_IND16_3, NEG_EXT,
	NEGA,
	NEGB,
	NEGD,
	NEGE,
	NEGW_IND16, NEGW_IND16_3, NEGW_EXT,
	ORAA_IND8, ORAA_IND8_3, ORAA_IMM8, ORAA_IND16, ORAA_IND16_3, ORAA_EXT, ORAA_INDE,
	ORAB_IND8, ORAB_IND8_3, ORAB_IMM8, ORAB_IND16, ORAB_IND16_3, ORAB_EXT, ORAB_INDE,
	ORD_IND8, ORD_IND8_3, ORD_IMM16, ORD_IND16, ORD_IND16_3, ORD_EXT, ORD_INDE,
	ORE_IMM16, ORE_IND16, ORE_IND16_3, ORE_EXT,
	ORP,
	PSHA, PSHA_2,
	PSHB,
	PSHM, PSHM_0, PSHM_1, PSHM_2, PSHM_3, PSHM_4, PSHM_5, PSHM_6, PSHM_8, PSHM_0B, PSHM_1B, PSHM_2B, PSHM_3B, PSHM_4B, PSHM_5B, PSHM_6B,
	PULA, PULA_2, PULA_3,
	PULB, PULB_2, PULB_3,
	PULM, PULM_0, PULM_1, PULM_2, PULM_3, PULM_4, PULM_5, PULM_6, PULM_7, PULM_8, PULM_0B, PULM_1B, PULM_2B, PULM_3B, PULM_4B, PULM_5B, PULM_6B, PULM_7B,
	ROL_IND8, ROL_IND8_3, ROL_IND16, ROL_IND16_3, ROL_EXT,
	ROLA,
	ROLB,
	ROLD,
	ROLE,
	ROLW_IND16, ROLW_IND16_3, ROLW_EXT,
	ROR_IND8, ROR_IND8_3, ROR_IND16, ROR_IND16_3, ROR_EXT,
	RORA,
	RORB,
	RORD,
	RORE,
	RORW_IND16, RORW_IND16_3, RORW_EXT,
	RTS, RTS_2, RTS_2B, RTS_3, RTS_3B, RTS_4,
	SBA,
	SDE,
	STAA_IND8, STAA_IND16, STAA_EXT, STAA_INDE,
	STAB_IND8, STAB_IND16, STAB_EXT, STAB_INDE,
	STD_IND8, STD_IND16, STD_EXT, STD_INDE,
	STE_IND16, STE_EXT,
	STED, STED_2, STED_2B,
	STX_IND8, STX_IND16, STX_EXT,
	SUBA_IND8, SUBA_IND8_3, SUBA_IMM8, SUBA_IND16, SUBA_IND16_3, SUBA_EXT, SUBA_INDE, SUBA_INDE_3,
	SUBB_IND8, SUBB_IND8_3, SUBB_IMM8, SUBB_IND16, SUBB_IND16_3, SUBB_EXT, SUBB_INDE, SUBB_INDE_3,
	SUBD_IND8, SUBD_IND8_3, SUBD_IMM16, SUBD_IND16, SUBD_IND16_3, SUBD_EXT, SUBD_INDE, SUBD_INDE_3,
	SUBE_IMM16, SUBE_IND16, SUBE_IND16_3, SUBE_EXT,
	SXT,
	TAB,
	TAP,
	TBA,
	TBEK,
	TBXK,
	TDE,
	TDMSK,
	TDP,
	TED,
	TEKB,
	TPA,
	TPD,
	TST_IND8, TST_IND8_3, TST_IND16, TST_IND16_3, TST_EXT,
	TSTA,
	TSTB,
	TSTD,
	TSTE,
	TSTW_IND16, TSTW_IND16_3, TSTW_EXT,
	TSX,
	TXKB,
	TXS,
	TXX,
	XGAB,
	XGDE,
	XGDX,
	XGEX
};

ALLOW_SAVE_TYPE(cpu16_device::seq)


cpu16_device::cpu16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 16, 20, 0, map)
	, m_data_config("data", ENDIANNESS_BIG, 16, 20, 0, map)
	, m_pc(0)
	, m_fwa(0)
	, m_fetch_pipe{0, 0, 0}
	, m_ccr(0)
	, m_d(0)
	, m_e(0)
	, m_ek(0)
	, m_index_regs{0, 0, 0, 0}
	, m_hr(0)
	, m_ir(0)
	, m_am(0)
	, m_sl(false)
	, m_index_mask{0, 0}
	, m_sequence(seq::BOOT_0)
	, m_return_sequence(seq::BOOT_0)
	, m_ea(0)
	, m_tmp(0)
	, m_start(false)
	, m_icount(0)
{
}

std::unique_ptr<util::disasm_interface> cpu16_device::create_disassembler()
{
	return std::make_unique<cpu16_disassembler>();
}

device_memory_interface::space_config_vector cpu16_device::memory_space_config() const
{
	if (has_configured_map(AS_DATA))
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_DATA, &m_data_config),
		};
	else
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
		};
}

void cpu16_device::debug_set_pcbase(u32 value)
{
	m_fwa = value;
	set_pc(value + 4);

	// Fake prefetch operations
	m_fetch_pipe[2] = m_cache.read_word(m_fwa);
	m_fetch_pipe[1] = m_cache.read_word(m_fwa + 2);
}

void cpu16_device::set_pc(u32 value) noexcept
{
	m_pc = value & 0xffffe;
	m_ccr = (m_ccr & 0xfff0) | (value & 0xf0000) >> 16;
}

void cpu16_device::debug_set_ccr(u16 value)
{
	m_ccr = value;
	m_pc = (value & 0x000f) << 16 | (m_pc & 0x0ffff);
}

u16 cpu16_device::get_k() noexcept
{
	return u16(m_ek) << 12
			| (m_index_regs[0] & 0xf0000) >> 8
			| (m_index_regs[1] & 0xf0000) >> 12
			| (m_index_regs[2] & 0xf0000) >> 16;
}

void cpu16_device::set_k(u16 value) noexcept
{
	m_ek = (value & 0xf000) >> 12;
	m_index_regs[0] = u32(value & 0x0f00) << 8 | (m_index_regs[0] & 0x0ffff);
	m_index_regs[1] = u32(value & 0x00f0) << 12 | (m_index_regs[1] & 0x0ffff);
	m_index_regs[2] = u32(value & 0x000f) << 16 | (m_index_regs[2] & 0x0ffff);
}

u16 cpu16_device::get_ix(int which) const noexcept
{
	return m_index_regs[which] & 0x0ffff;
}

void cpu16_device::set_ix(int which, u16 value) noexcept
{
	m_index_regs[which] = (m_index_regs[which] & 0xf0000) | value;
}

u8 cpu16_device::get_xk(int which) const noexcept
{
	return (m_index_regs[which] & 0xf0000) >> 16;
}

void cpu16_device::set_xk(int which, u8 value) noexcept
{
	m_index_regs[which] = u32(value & 0xf) << 16 | (m_index_regs[which] & 0x0ffff);
}

void cpu16_device::set_a(u8 value) noexcept
{
	m_d = u16(value) << 8 | (m_d & 0x00ff);
}

void cpu16_device::set_b(u8 value) noexcept
{
	m_d = (m_d & 0xff00) | value;
}

void cpu16_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(has_space(AS_DATA) ? AS_DATA : AS_PROGRAM).specific(m_data);

	set_icountptr(m_icount);

	using namespace std::placeholders;
	state_add(CPU16_FWA, "FWA", m_fwa, std::bind(&cpu16_device::debug_set_pcbase, this, _1)).mask(0xffffe);
	state_add(CPU16_IPIPEC, "IPIPEC", m_fetch_pipe[2]);
	state_add(CPU16_IPIPEB, "IPIPEB", m_fetch_pipe[1]);
	//state_add(CPU16_IPIPEA, "IPIPEA", m_fetch_pipe[0]);
	state_add(CPU16_D, "D", m_d);
	state_add<u8>(CPU16_A, "A",
		[this]() { return (m_d & 0xff00) >> 8; },
		std::bind(&cpu16_device::set_a, this, _1)).noshow();
	state_add<u8>(CPU16_B, "B",
		[this]() { return m_d & 0x00ff; },
		std::bind(&cpu16_device::set_b, this, _1)).noshow();
	state_add(CPU16_E, "E", m_e);
	state_add(CPU16_EK, "EK", m_ek).mask(0xf);
	for (int i = 0; i < 3; i++)
	{
		state_add(CPU16_X + i, util::string_format("%c", 'X' + i).c_str(), m_index_regs[i]).mask(0xfffff);
		state_add<u16>(CPU16_IX + i, util::string_format("I%c", 'X' + i).c_str(),
			std::bind(&cpu16_device::get_ix, this, i),
			std::bind(&cpu16_device::set_ix, this, i, _1)).noshow();
		state_add<u8>(CPU16_XK + i, util::string_format("%cK", 'X' + i).c_str(),
			std::bind(&cpu16_device::get_xk, this, i),
			std::bind(&cpu16_device::set_xk, this, i, _1)).mask(0xf).noshow();
	}
	state_add(CPU16_SP, "SP", m_index_regs[3]).mask(0xfffff);
	state_add<u8>(CPU16_SK, "SK",
		[this]() { return (m_index_regs[3] & 0xf0000) >> 16; },
		[this](u8 value) { m_index_regs[3] = (m_index_regs[3] & 0x0ffff) | u32(value) << 16; }).mask(0xf).noshow();
	state_add(CPU16_PC, "PC", m_pc, std::bind(&cpu16_device::set_pc, this, _1)).mask(0xffffe);
	state_add(STATE_GENPC, "GENPC", m_pc, std::bind(&cpu16_device::set_pc, this, _1)).mask(0xffffe).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_fwa, std::bind(&cpu16_device::debug_set_pcbase, this, _1)).mask(0xffffe).noshow();
	state_add(CPU16_CCR, "CCR", m_ccr, std::bind(&cpu16_device::debug_set_ccr, this, _1));
	state_add(STATE_GENFLAGS, "CURFLAGS", m_ccr, std::bind(&cpu16_device::debug_set_ccr, this, _1)).noshow().formatstr("%13s");
	state_add<u16>(CPU16_K, "K", std::bind(&cpu16_device::get_k, this), std::bind(&cpu16_device::set_k, this, _1)).noshow();
	state_add(CPU16_HR, "HR", m_hr);
	state_add(CPU16_IR, "IR", m_ir);
	state_add(CPU16_AM, "AM", m_am).mask(0xfffffffff);
	state_add(CPU16_SL, "SL", m_sl);
	state_add(CPU16_XMSK, "XMSK", m_index_mask[0]);
	state_add(CPU16_YMSK, "YMSK", m_index_mask[1]);

	save_item(NAME(m_fwa));
	save_item(NAME(m_fetch_pipe));
	save_item(NAME(m_ccr));
	save_item(NAME(m_d));
	save_item(NAME(m_e));
	save_item(NAME(m_ek));
	save_item(NAME(m_index_regs));
	save_item(NAME(m_pc));
	save_item(NAME(m_hr));
	save_item(NAME(m_ir));
	save_item(NAME(m_am));
	save_item(NAME(m_sl));
	save_item(NAME(m_index_mask));
	save_item(NAME(m_sequence));
	save_item(NAME(m_return_sequence));
	save_item(NAME(m_ea));
	save_item(NAME(m_tmp));
	save_item(NAME(m_start));
}

void cpu16_device::device_reset()
{
	m_ccr = 0x80e0 | (m_ccr & 0x7f00); // IP, S, SM initialized
	set_k(0);

	m_sequence = seq::BOOT_0;
	m_start = false;
}

const cpu16_device::seq cpu16_device::s_inst_decode[4][256] =
{
	// Page 0
	{
		// 0X-3X
		seq::NEG_IND8, seq::DEC_IND8, seq::NEG_IND8, seq::INC_IND8, seq::ROL_IND8, seq::CLR_IND8, seq::TST_IND8, seq::INVALID,
		seq::BCLR_IND16, seq::BSET_IND16, seq::BRSET_IND16, seq::BRSET_IND16, seq::ROL_IND8, seq::ASR_IND8, seq::ROR_IND8, seq::ROR_IND8,
		seq::NEG_IND8, seq::DEC_IND8, seq::NEG_IND8, seq::INC_IND8, seq::ROL_IND8, seq::CLR_IND8, seq::TST_IND8, seq::INVALID,
		seq::BCLR_IND16, seq::BSET_IND16, seq::BRSET_IND16, seq::BRSET_IND16, seq::ROL_IND8, seq::ASR_IND8, seq::ROR_IND8, seq::ROR_IND8,
		seq::NEG_IND8, seq::DEC_IND8, seq::NEG_IND8, seq::INC_IND8, seq::ROL_IND8, seq::CLR_IND8, seq::TST_IND8, seq::INVALID,
		seq::BCLR_IND16, seq::BSET_IND16, seq::BRSET_IND16, seq::BRSET_IND16, seq::ROL_IND8, seq::ASR_IND8, seq::ROR_IND8, seq::ROR_IND8,
		seq::MOVB_IXP_EXT, seq::MOVW_IXP_EXT, seq::MOVB_EXT_IXP, seq::MOVW_EXT_IXP, seq::PSHM, seq::PULM, seq::BSR, seq::INVALID,
		seq::BCLR_EXT, seq::BSET_EXT, seq::BRSET_EXT, seq::BRSET_EXT, seq::AIX_IMM8, seq::AIX_IMM8, seq::AIX_IMM8, seq::AIX_IMM8,

		// 4X-7X
		seq::SUBA_IND8, seq::ADDA_IND8, seq::SUBA_IND8, seq::ADDA_IND8, seq::EORA_IND8, seq::LDAA_IND8, seq::ANDA_IND8, seq::ORAA_IND8,
		seq::CMPA_IND8, seq::BITA_IND8, seq::STAA_IND8, seq::INVALID, seq::CPX_IND8, seq::CPX_IND8, seq::CPX_IND8, seq::CPX_IND8,
		seq::SUBA_IND8, seq::ADDA_IND8, seq::SUBA_IND8, seq::ADDA_IND8, seq::EORA_IND8, seq::LDAA_IND8, seq::ANDA_IND8, seq::ORAA_IND8,
		seq::CMPA_IND8, seq::BITA_IND8, seq::STAA_IND8, seq::INVALID, seq::CPX_IND8, seq::CPX_IND8, seq::CPX_IND8, seq::CPX_IND8,
		seq::SUBA_IND8, seq::ADDA_IND8, seq::SUBA_IND8, seq::ADDA_IND8, seq::EORA_IND8, seq::LDAA_IND8, seq::ANDA_IND8, seq::ORAA_IND8,
		seq::CMPA_IND8, seq::BITA_IND8, seq::STAA_IND8, seq::INVALID, seq::CPX_IND8, seq::CPX_IND8, seq::CPX_IND8, seq::CPX_IND8,
		seq::SUBA_IMM8, seq::ADDA_IMM8, seq::SUBA_IMM8, seq::ADDA_IMM8, seq::EORA_IMM8, seq::LDAA_IMM8, seq::ANDA_IMM8, seq::ORAA_IMM8,
		seq::CMPA_IMM8, seq::BITA_IMM8, seq::JMP_EXT20, seq::INVALID, seq::ADDE_IMM8, seq::INVALID, seq::INVALID, seq::INVALID,

		// AX-BX
		seq::SUBD_IND8, seq::ADDD_IND8, seq::SUBD_IND8, seq::ADDD_IND8, seq::EORD_IND8, seq::LDD_IND8, seq::ANDD_IND8, seq::ORD_IND8,
		seq::CPD_IND8, seq::INVALID, seq::STD_IND8, seq::BRSET_IND8, seq::STX_IND8, seq::STX_IND8, seq::STX_IND8, seq::STX_IND8,
		seq::SUBD_IND8, seq::ADDD_IND8, seq::SUBD_IND8, seq::ADDD_IND8, seq::EORD_IND8, seq::LDD_IND8, seq::ANDD_IND8, seq::ORD_IND8,
		seq::CPD_IND8, seq::INVALID, seq::STD_IND8, seq::BRSET_IND8, seq::STX_IND8, seq::STX_IND8, seq::STX_IND8, seq::STX_IND8,
		seq::SUBD_IND8, seq::ADDD_IND8, seq::SUBD_IND8, seq::ADDD_IND8, seq::EORD_IND8, seq::LDD_IND8, seq::ANDD_IND8, seq::ORD_IND8,
		seq::CPD_IND8, seq::INVALID, seq::STD_IND8, seq::BRSET_IND8, seq::STX_IND8, seq::STX_IND8, seq::STX_IND8, seq::STX_IND8,
		seq::BCC, seq::BCC, seq::BCC, seq::BCC, seq::BCC, seq::BCC, seq::BCC, seq::BCC,
		seq::BCC, seq::BCC, seq::BCC, seq::BCC, seq::BCC, seq::BCC, seq::BCC, seq::BCC,

		// CX-FX
		seq::SUBB_IND8, seq::ADDB_IND8, seq::SUBB_IND8, seq::ADDB_IND8, seq::EORB_IND8, seq::LDAB_IND8, seq::ANDB_IND8, seq::ORAB_IND8,
		seq::CMPB_IND8, seq::BITB_IND8, seq::STAB_IND8, seq::BRSET_IND8, seq::LDX_IND8, seq::LDX_IND8, seq::LDX_IND8, seq::LDX_IND8,
		seq::SUBB_IND8, seq::ADDB_IND8, seq::SUBB_IND8, seq::ADDB_IND8, seq::EORB_IND8, seq::LDAB_IND8, seq::ANDB_IND8, seq::ORAB_IND8,
		seq::CMPB_IND8, seq::BITB_IND8, seq::STAB_IND8, seq::BRSET_IND8, seq::LDX_IND8, seq::LDX_IND8, seq::LDX_IND8, seq::LDX_IND8,
		seq::SUBB_IND8, seq::ADDB_IND8, seq::SUBB_IND8, seq::ADDB_IND8, seq::EORB_IND8, seq::LDAB_IND8, seq::ANDB_IND8, seq::ORAB_IND8,
		seq::CMPB_IND8, seq::BITB_IND8, seq::STAB_IND8, seq::BRSET_IND8, seq::LDX_IND8, seq::LDX_IND8, seq::LDX_IND8, seq::LDX_IND8,
		seq::SUBB_IMM8, seq::ADDB_IMM8, seq::SUBB_IND8, seq::ADDB_IMM8, seq::EORB_IMM8, seq::LDAB_IMM8, seq::ANDB_IMM8, seq::ORAB_IMM8,
		seq::CMPB_IMM8, seq::BITB_IMM8, seq::JSR_EXT20, seq::INVALID, seq::ADDD_IMM8, seq::INVALID, seq::INVALID, seq::INVALID
	},

	// Page 1
	{
		// 170X-173X
		seq::NEG_IND16, seq::DEC_IND16, seq::NEG_IND16, seq::INC_IND16, seq::ROL_IND16, seq::CLR_IND16, seq::TST_IND16, seq::INVALID,
		seq::BCLR_IND8, seq::BSET_IND8, seq::INVALID, seq::INVALID, seq::ROL_IND16, seq::ASR_IND16, seq::ROR_IND16, seq::ROR_IND16,
		seq::NEG_IND16, seq::DEC_IND16, seq::NEG_IND16, seq::INC_IND16, seq::ROL_IND16, seq::CLR_IND16, seq::TST_IND16, seq::INVALID,
		seq::BCLR_IND8, seq::BSET_IND8, seq::INVALID, seq::INVALID, seq::ROL_IND16, seq::ASR_IND16, seq::ROR_IND16, seq::ROR_IND16,
		seq::NEG_IND16, seq::DEC_IND16, seq::NEG_IND16, seq::INC_IND16, seq::ROL_IND16, seq::CLR_IND16, seq::TST_IND16, seq::INVALID,
		seq::BCLR_IND8, seq::BSET_IND8, seq::INVALID, seq::INVALID, seq::ROL_IND16, seq::ASR_IND16, seq::ROR_IND16, seq::ROR_IND16,
		seq::NEG_EXT, seq::DEC_EXT, seq::NEG_EXT, seq::INC_EXT, seq::ROL_EXT, seq::CLR_EXT, seq::TST_EXT, seq::INVALID,
		seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::ROL_EXT, seq::ASR_EXT, seq::ROR_EXT, seq::ROR_EXT,

		// 174X-177X
		seq::SUBA_IND16, seq::ADDA_IND16, seq::SUBA_IND16, seq::ADDA_IND16, seq::EORA_IND16, seq::LDAA_IND16, seq::ANDA_IND16, seq::ORAA_IND16,
		seq::CMPA_IND16, seq::BITA_IND16, seq::STAA_IND16, seq::INVALID, seq::CPX_IND16, seq::CPX_IND16, seq::CPX_IND16, seq::CPX_IND16,
		seq::SUBA_IND16, seq::ADDA_IND16, seq::SUBA_IND16, seq::ADDA_IND16, seq::EORA_IND16, seq::LDAA_IND16, seq::ANDA_IND16, seq::ORAA_IND16,
		seq::CMPA_IND16, seq::BITA_IND16, seq::STAA_IND16, seq::INVALID, seq::CPX_IND16, seq::CPX_IND16, seq::CPX_IND16, seq::CPX_IND16,
		seq::SUBA_IND16, seq::ADDA_IND16, seq::SUBA_IND16, seq::ADDA_IND16, seq::EORA_IND16, seq::LDAA_IND16, seq::ANDA_IND16, seq::ORAA_IND16,
		seq::CMPA_IND16, seq::BITA_IND16, seq::STAA_IND16, seq::INVALID, seq::CPX_IND16, seq::CPX_IND16, seq::CPX_IND16, seq::CPX_IND16,
		seq::SUBA_EXT, seq::ADDA_EXT, seq::SUBA_EXT, seq::ADDA_EXT, seq::EORA_EXT, seq::LDAA_EXT, seq::ANDA_EXT, seq::ORAA_EXT,
		seq::CMPA_EXT, seq::BITA_EXT, seq::STAA_EXT, seq::INVALID, seq::CPX_EXT, seq::CPX_EXT, seq::CPX_EXT, seq::CPX_EXT,

		// 178X-17BX
		seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID,
		seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::STX_IND16, seq::STX_IND16, seq::STX_IND16, seq::STX_IND16,
		seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID,
		seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::STX_IND16, seq::STX_IND16, seq::STX_IND16, seq::STX_IND16,
		seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID,
		seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::STX_IND16, seq::STX_IND16, seq::STX_IND16, seq::STX_IND16,
		seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID,
		seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::STX_EXT, seq::STX_EXT, seq::STX_EXT, seq::STX_EXT,

		// 17CX-17FX
		seq::SUBB_IND16, seq::ADDB_IND16, seq::SUBB_IND16, seq::ADDB_IND16, seq::EORB_IND16, seq::LDAB_IND16, seq::ANDB_IND16, seq::ORAB_IND16,
		seq::CMPB_IND16, seq::BITB_IND16, seq::STAB_IND16, seq::INVALID, seq::LDX_IND16, seq::LDX_IND16, seq::LDX_IND16, seq::LDX_IND16,
		seq::SUBB_IND16, seq::ADDB_IND16, seq::SUBB_IND16, seq::ADDB_IND16, seq::EORB_IND16, seq::LDAB_IND16, seq::ANDB_IND16, seq::ORAB_IND16,
		seq::CMPB_IND16, seq::BITB_IND16, seq::STAB_IND16, seq::INVALID, seq::LDX_IND16, seq::LDX_IND16, seq::LDX_IND16, seq::LDX_IND16,
		seq::SUBB_IND16, seq::ADDB_IND16, seq::SUBB_IND16, seq::ADDB_IND16, seq::EORB_IND16, seq::LDAB_IND16, seq::ANDB_IND16, seq::ORAB_IND16,
		seq::CMPB_IND16, seq::BITB_IND16, seq::STAB_IND16, seq::INVALID, seq::LDX_IND16, seq::LDX_IND16, seq::LDX_IND16, seq::LDX_IND16,
		seq::SUBB_EXT, seq::ADDB_EXT, seq::SUBB_EXT, seq::ADDB_EXT, seq::EORB_EXT, seq::LDAB_EXT, seq::ANDB_EXT, seq::ORAB_EXT,
		seq::CMPB_EXT, seq::BITB_EXT, seq::STAB_EXT, seq::INVALID, seq::LDX_EXT, seq::LDX_EXT, seq::LDX_EXT, seq::LDX_EXT
	},

	// Page 2
	{
		// 270X-273X
		seq::NEGW_IND16, seq::DECW_IND16, seq::NEGW_IND16, seq::INCW_IND16, seq::ROLW_IND16, seq::CLRW_IND16, seq::TSTW_IND16, seq::INVALID,
		seq::BCLRW_IND16, seq::BSETW_IND16, seq::INVALID, seq::INVALID, seq::ROLW_IND16, seq::ASRW_IND16, seq::RORW_IND16, seq::RORW_IND16,
		seq::NEGW_IND16, seq::DECW_IND16, seq::NEGW_IND16, seq::INCW_IND16, seq::ROLW_IND16, seq::CLRW_IND16, seq::TSTW_IND16, seq::INVALID,
		seq::BCLRW_IND16, seq::BSETW_IND16, seq::INVALID, seq::INVALID, seq::ROLW_IND16, seq::ASRW_IND16, seq::RORW_IND16, seq::RORW_IND16,
		seq::NEGW_IND16, seq::DECW_IND16, seq::NEGW_IND16, seq::INCW_IND16, seq::ROLW_IND16, seq::CLRW_IND16, seq::TSTW_IND16, seq::INVALID,
		seq::BCLRW_IND16, seq::BSETW_IND16, seq::INVALID, seq::INVALID, seq::ROLW_IND16, seq::ASRW_IND16, seq::RORW_IND16, seq::RORW_IND16,
		seq::NEGW_EXT, seq::DECW_EXT, seq::NEGW_EXT, seq::INCW_EXT, seq::ROLW_EXT, seq::CLRW_EXT, seq::TSTW_EXT, seq::INVALID,
		seq::BCLRW_EXT, seq::BSETW_EXT, seq::INVALID, seq::INVALID, seq::ROLW_EXT, seq::ASRW_EXT, seq::RORW_EXT, seq::RORW_EXT,

		// 274X-277X
		seq::SUBA_INDE, seq::ADDA_INDE, seq::SUBA_INDE, seq::ADDA_INDE, seq::EORA_INDE, seq::LDAA_INDE, seq::ANDA_INDE, seq::ORAA_INDE,
		seq::CMPA_INDE, seq::BITA_INDE, seq::STAA_INDE, seq::INVALID, seq::TXX, seq::TXX, seq::TXX, seq::TSX,
		seq::SUBA_INDE, seq::ADDA_INDE, seq::SUBA_INDE, seq::ADDA_INDE, seq::EORA_INDE, seq::LDAA_INDE, seq::ANDA_INDE, seq::ORAA_INDE,
		seq::CMPA_INDE, seq::BITA_INDE, seq::STAA_INDE, seq::INVALID, seq::TXX, seq::INVALID, seq::TXX, seq::TSX,
		seq::SUBA_INDE, seq::ADDA_INDE, seq::SUBA_INDE, seq::ADDA_INDE, seq::EORA_INDE, seq::LDAA_INDE, seq::ANDA_INDE, seq::ORAA_INDE,
		seq::CMPA_INDE, seq::BITA_INDE, seq::STAA_INDE, seq::INVALID, seq::TXX, seq::TXX, seq::INVALID, seq::TSX,
		seq::NEGE, seq::LDED, seq::NEGE, seq::STED, seq::ROLE, seq::CLRE, seq::TSTE, seq::INVALID,
		seq::ADE, seq::SDE, seq::XGDE, seq::TDE, seq::ROLE, seq::ASRE, seq::RORE, seq::RORE,

		// 278X-27BX
		seq::SUBD_INDE, seq::ADDD_INDE, seq::SUBD_INDE, seq::ADDD_INDE, seq::EORD_INDE, seq::LDD_INDE, seq::ANDD_INDE, seq::ORD_INDE,
		seq::CPD_INDE, seq::INVALID, seq::STD_INDE, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID,
		seq::SUBD_INDE, seq::ADDD_INDE, seq::SUBD_INDE, seq::ADDD_INDE, seq::EORD_INDE, seq::LDD_INDE, seq::ANDD_INDE, seq::ORD_INDE,
		seq::CPD_INDE, seq::INVALID, seq::STD_INDE, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID,
		seq::SUBD_INDE, seq::ADDD_INDE, seq::SUBD_INDE, seq::ADDD_INDE, seq::EORD_INDE, seq::LDD_INDE, seq::ANDD_INDE, seq::ORD_INDE,
		seq::CPD_INDE, seq::INVALID, seq::STD_INDE, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID,
		seq::LDHI, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::CLRM,
		seq::INVALID, seq::INVALID, seq::INVALID, seq::TEKB, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID,

		// 27CX-27FX
		seq::SUBB_INDE, seq::ADDB_INDE, seq::SUBB_INDE, seq::ADDB_INDE, seq::EORB_INDE, seq::LDAB_INDE, seq::ANDB_INDE, seq::ORAB_INDE,
		seq::CMPB_INDE, seq::BITB_INDE, seq::STAB_INDE, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID,
		seq::SUBB_INDE, seq::ADDB_INDE, seq::SUBB_INDE, seq::ADDB_INDE, seq::EORB_INDE, seq::LDAB_INDE, seq::ANDB_INDE, seq::ORAB_INDE,
		seq::CMPB_INDE, seq::BITB_INDE, seq::STAB_INDE, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID,
		seq::SUBB_INDE, seq::ADDB_INDE, seq::SUBB_INDE, seq::ADDB_INDE, seq::EORB_INDE, seq::LDAB_INDE, seq::ANDB_INDE, seq::ORAB_INDE,
		seq::CMPB_INDE, seq::BITB_INDE, seq::STAB_INDE, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID,
		seq::NEGD, seq::INVALID, seq::NEGD, seq::INVALID, seq::ROLD, seq::CLRD, seq::TSTD, seq::RTS,
		seq::SXT, seq::LBSR, seq::TBEK, seq::TDE, seq::ROLD, seq::ASRD, seq::RORD, seq::RORD
	},

	// Page 3
	{
		// 370X-373X
		seq::NEGA, seq::DECA, seq::NEGA, seq::INCA, seq::ROLA, seq::CLRA, seq::TSTA, seq::TBA,
		seq::PSHA, seq::PULA, seq::SBA, seq::ABA, seq::ROLA, seq::ASRA, seq::RORA, seq::RORA,
		seq::NEGB, seq::DECB, seq::NEGB, seq::INCB, seq::ROLB, seq::CLRB, seq::TSTB, seq::TAB,
		seq::PSHB, seq::PULB, seq::XGAB, seq::CBA, seq::ROLB, seq::ASRB, seq::RORB, seq::RORB,
		seq::INVALID, seq::DAA, seq::INVALID, seq::INVALID, seq::MUL, seq::EMUL, seq::EMULS, seq::EMULS,
		seq::INVALID, seq::INVALID, seq::DIV, seq::DIV, seq::TPD, seq::TDP, seq::INVALID, seq::TDMSK,
		seq::SUBE_IMM16, seq::ADDE_IMM16, seq::SUBE_IMM16, seq::ADDE_IMM16, seq::EORE_IMM16, seq::LDE_IMM16, seq::ANDE_IMM16, seq::ORE_IMM16,
		seq::CPE_IMM16, seq::INVALID, seq::ANDP, seq::ORP, seq::AIX_IMM16, seq::AIX_IMM16, seq::AIX_IMM16, seq::AIX_IMM16,

		// 374X-377X
		seq::SUBE_IND16, seq::ADDE_IND16, seq::SUBE_IND16, seq::ADDE_IND16, seq::EORE_IND16, seq::LDE_IND16, seq::ANDE_IND16, seq::ORE_IND16,
		seq::CPE_IND16, seq::INVALID, seq::STE_IND16, seq::INVALID, seq::XGEX, seq::AEX, seq::TXS, seq::ABX,
		seq::SUBE_IND16, seq::ADDE_IND16, seq::SUBE_IND16, seq::ADDE_IND16, seq::EORE_IND16, seq::LDE_IND16, seq::ANDE_IND16, seq::ORE_IND16,
		seq::CPE_IND16, seq::INVALID, seq::STE_IND16, seq::INVALID, seq::XGEX, seq::AEX, seq::TXS, seq::ABX,
		seq::SUBE_IND16, seq::ADDE_IND16, seq::SUBE_IND16, seq::ADDE_IND16, seq::EORE_IND16, seq::LDE_IND16, seq::ANDE_IND16, seq::ORE_IND16,
		seq::CPE_IND16, seq::INVALID, seq::STE_IND16, seq::INVALID, seq::XGEX, seq::AEX, seq::TXS, seq::ABX,
		seq::SUBE_EXT, seq::ADDE_EXT, seq::SUBE_EXT, seq::ADDE_EXT, seq::EORE_EXT, seq::LDE_EXT, seq::ANDE_EXT, seq::ORE_EXT,
		seq::CPE_EXT, seq::INVALID, seq::STE_EXT, seq::INVALID, seq::CPX_IMM16, seq::CPX_IMM16, seq::CPX_IMM16, seq::CPX_IMM16,

		// 378X-37BX
		seq::LBCC, seq::LBCC, seq::LBCC, seq::LBCC, seq::LBCC, seq::LBCC, seq::LBCC, seq::LBCC,
		seq::LBCC, seq::LBCC, seq::LBCC, seq::LBCC, seq::LBCC, seq::LBCC, seq::LBCC, seq::LBCC,
		seq::LBCC, seq::LBCC, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID,
		seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::TBXK, seq::TBXK, seq::TBXK, seq::TBXK,
		seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID,
		seq::INVALID, seq::INVALID, seq::INVALID, seq::INVALID, seq::TXKB, seq::TXKB, seq::TXKB, seq::TXKB,
		seq::SUBD_IMM16, seq::ADDD_IMM16, seq::SUBD_IMM16, seq::ADDD_IMM16, seq::EORD_IMM16, seq::LDD_IMM16, seq::ANDD_IMM16, seq::ORD_IMM16,
		seq::CPD_IMM16, seq::INVALID, seq::INVALID, seq::INVALID, seq::LDX_IMM16, seq::LDX_IMM16, seq::LDX_IMM16, seq::LDX_IMM16,

		// 37CX-37FX
		seq::SUBD_IND16, seq::ADDD_IND16, seq::SUBD_IND16, seq::ADDD_IND16, seq::EORD_IND16, seq::LDD_IND16, seq::ANDD_IND16, seq::ORD_IND16,
		seq::CPD_IND16, seq::INVALID, seq::STD_IND16, seq::INVALID, seq::XGDX, seq::ADX, seq::INVALID, seq::INVALID,
		seq::SUBD_IND16, seq::ADDD_IND16, seq::SUBD_IND16, seq::ADDD_IND16, seq::EORD_IND16, seq::LDD_IND16, seq::ANDD_IND16, seq::ORD_IND16,
		seq::CPD_IND16, seq::INVALID, seq::STD_IND16, seq::INVALID, seq::XGDX, seq::ADX, seq::INVALID, seq::INVALID,
		seq::SUBD_IND16, seq::ADDD_IND16, seq::SUBD_IND16, seq::ADDD_IND16, seq::EORD_IND16, seq::LDD_IND16, seq::ANDD_IND16, seq::ORD_IND16,
		seq::CPD_IND16, seq::INVALID, seq::STD_IND16, seq::INVALID, seq::XGDX, seq::ADX, seq::INVALID, seq::INVALID,
		seq::SUBD_EXT, seq::ADDD_EXT, seq::SUBD_EXT, seq::ADDD_EXT, seq::EORD_EXT, seq::LDD_EXT, seq::ANDD_EXT, seq::ORD_EXT,
		seq::CPD_EXT, seq::INVALID, seq::STD_EXT, seq::INVALID, seq::TPA, seq::TAP, seq::MOVB_EXT_EXT, seq::MOVW_EXT_EXT
	}
};

void cpu16_device::advance() noexcept
{
	if (m_start)
	{
		m_fetch_pipe[2] = m_fetch_pipe[1];
		m_fwa = m_pc - 2;
	}
	m_fetch_pipe[1] = m_fetch_pipe[0];
	m_pc = (m_pc + 2) & 0xfffff;
	if ((m_pc & 0x0ffff) == 0)
		m_ccr = (m_ccr & 0xfff0) | (m_pc >> 16);
}

void cpu16_device::set_nzv8(u8 data, bool v) noexcept
{
	m_ccr = (m_ccr & 0xf1ff) | (s8(data) < 0 ? 0x0800 : 0) | (data == 0 ? 0x0400 : 0) | (v ? 0x0200 : 0);
}

void cpu16_device::set_nzv16(u16 data, bool v) noexcept
{
	m_ccr = (m_ccr & 0xf1ff) | (s16(data) < 0 ? 0x0800 : 0) | (data == 0 ? 0x0400 : 0) | (v ? 0x0200 : 0);
}

void cpu16_device::set_z16(u16 data) noexcept
{
	m_ccr = (m_ccr & 0xfbff) | (data == 0 ? 0x0400 : 0);
}

u8 cpu16_device::adc8(u8 data1, u8 data2, bool cin) noexcept
{
	// 8-bit additions set the half carry flag as well as NZVC
	s8 result = data1 + data2 + (cin ? 1 : 0);
	m_ccr = (m_ccr & 0xd0ff)
			| ((data1 & 0x0f) + (data2 & 0x0f) + (cin ? 1 : 0) > 0x0f ? 0x2000 : 0)
			| (result < 0 ? 0x0800 : 0)
			| (result == 0 ? 0x0400 : 0)
			| ((s8(data2) < 0 ? s8(data1) < 0 && result >= 0 : s8(data1) >= 0 && result < 0) ? 0x0200 : 0)
			| (data1 >= (cin ? 0x100 : 0xff) - data2 ? 0x0100 : 0);
	return result;
}

u8 cpu16_device::sbc8(u8 data1, u8 data2, bool cin) noexcept
{
	s8 result = data1 - data2 - (cin ? 1 : 0);
	m_ccr = (m_ccr & 0xf0ff)
			| (result < 0 ? 0x0800 : 0)
			| (result == 0 ? 0x0400 : 0)
			| ((s8(data2) >= 0 ? s8(data1) < 0 && result >= 0 : s8(data1) >= 0 && result < 0) ? 0x0200 : 0)
			| (data1 < data2 + (cin ? 1 : 0) ? 0x0100 : 0);
	return result;
}

u16 cpu16_device::adc16(u16 data1, u16 data2, bool cin) noexcept
{
	s16 result = data1 + data2 + (cin ? 1 : 0);
	m_ccr = (m_ccr & 0xf0ff)
			| (result < 0 ? 0x0800 : 0)
			| (result == 0 ? 0x0400 : 0)
			| ((s16(data2) < 0 ? s16(data1) < 0 && result >= 0 : s16(data1) >= 0 && result < 0) ? 0x0200 : 0)
			| (data1 >= (cin ? 0x10000 : 0xffff) - data2 ? 0x0100 : 0);
	return result;
}

u16 cpu16_device::sbc16(u16 data1, u16 data2, bool cin) noexcept
{
	s16 result = data1 - data2 - (cin ? 1 : 0);
	m_ccr = (m_ccr & 0xf0ff)
			| (result < 0 ? 0x0800 : 0)
			| (result == 0 ? 0x0400 : 0)
			| ((s16(data2) >= 0 ? s16(data1) < 0 && result >= 0 : s16(data1) >= 0 && result < 0) ? 0x0200 : 0)
			| (data1 < data2 + (cin ? 1 : 0) ? 0x0100 : 0);
	return result;
}

u8 cpu16_device::rol8(u8 data, bool cin) noexcept
{
	u8 result = (data << 1) | (cin ? 1 : 0);
	m_ccr = (m_ccr & 0xf0ff)
			| (BIT(result, 7) ? 0x0800 : 0)
			| (result == 0 ? 0x0400 : 0)
			| (BIT(data ^ result, 7) ? 0x0200 : 0)
			| (BIT(data, 7) ? 0x0100 : 0);
	return result;
}

u16 cpu16_device::rol16(u16 data, bool cin) noexcept
{
	u16 result = (data << 1) | (cin ? 1 : 0);
	m_ccr = (m_ccr & 0xf0ff)
			| (BIT(result, 15) ? 0x0800 : 0)
			| (result == 0 ? 0x0400 : 0)
			| (BIT(data ^ result, 15) ? 0x0200 : 0)
			| (BIT(data, 15) ? 0x0100 : 0);
	return result;
}

u8 cpu16_device::ror8(u8 data, bool cin) noexcept
{
	u8 result = (data >> 1) | (cin ? 0x80 : 0);
	m_ccr = (m_ccr & 0xf0ff)
			| (BIT(result, 7) ? 0x0800 : 0)
			| (result == 0 ? 0x0400 : 0)
			| (BIT(data, 0) != BIT(result, 7) ? 0x0200 : 0)
			| (BIT(data, 0) ? 0x0100 : 0);
	return result;
}

u16 cpu16_device::ror16(u16 data, bool cin) noexcept
{
	u16 result = (data >> 1) | (cin ? 0x8000 : 0);
	m_ccr = (m_ccr & 0xf0ff)
			| (BIT(result, 15) ? 0x0800 : 0)
			| (result == 0 ? 0x0400 : 0)
			| (BIT(data, 0) != BIT(result, 15) ? 0x0200 : 0)
			| (BIT(data, 0) ? 0x0100 : 0);
	return result;
}

u8 cpu16_device::asr8(u8 data) noexcept
{
	u8 result = (data >> 1) | (data & 0x80);
	m_ccr = (m_ccr & 0xf0ff)
			| (BIT(result, 7) ? 0x0800 : 0)
			| (result == 0 ? 0x0400 : 0)
			| (BIT(data, 0) != BIT(result, 7) ? 0x0200 : 0)
			| (BIT(data, 0) ? 0x0100 : 0);
	return result;
}

u16 cpu16_device::asr16(u16 data) noexcept
{
	u16 result = (data >> 1) | (data & 0x8000);
	m_ccr = (m_ccr & 0xf0ff)
			| (BIT(result, 15) ? 0x0800 : 0)
			| (result == 0 ? 0x0400 : 0)
			| (BIT(data, 0) != BIT(result, 15) ? 0x0200 : 0)
			| (BIT(data, 0) ? 0x0100 : 0);
	return result;
}

void cpu16_device::mulu8() noexcept
{
	// 8×8 → 16-bit unsigned multiply, setting C to permit rounding into A
	m_d = (m_d & 0x00ff) * ((m_d >> 8) & 0x00ff);
	if (BIT(m_d, 7))
		m_ccr |= 0x0100;
	else
		m_ccr &= 0xfeff;
}

void cpu16_device::mulu16() noexcept
{
	// 16×16 → 32-bit unsigned multiply, setting N, Z and C according to result
	u32 prod = u32(m_d) * u32(m_e);
	m_ccr = (m_ccr & 0xf2ff)
			| (BIT(prod, 31) ? 0x0800 : 0)
			| (prod == 0 ? 0x0400 : 0)
			| (BIT(prod, 15) ? 0x0100 : 0);

	// Product is stored in E:D
	m_e = prod >> 16;
	m_d = prod & 0xffff;
}

void cpu16_device::muls16(bool frac) noexcept
{
	// 16×16 → 32-bit signed multiply, setting N, Z and C according to result
	s32 prod = s32(s16(m_d)) * s32(s16(m_e));
	m_ccr = (m_ccr & 0xf2ff)
			| (prod < 0 ? 0x0800 : 0)
			| (prod == 0 ? 0x0400 : 0)
			| (BIT(prod, 15) ? 0x0100 : 0);

	// Optional adjustment for fractional mode
	if (frac)
	{
		// Set V flag if -1.0 × -1.0 = -1.0
		if (prod == 0x40000000)
			m_ccr |= 0x0200;
		else
			m_ccr &= 0xfdff;
		prod <<= 1;
	}

	// Product is stored in E:D
	m_e = prod >> 16;
	m_d = prod & 0xffff;
}

void cpu16_device::divu16(bool frac) noexcept
{
	u16 divisor = get_ix(0);
	if (divisor <= (frac ? m_d : 0))
	{
		// Division by zero or fractional quotient ≥ 1.0
		m_ccr = (m_ccr & 0xf8ff)
				| (frac ? 0x0200 : 0)
				| (divisor == 0 ? 0x0100 : 0);
		m_index_regs[0] = 0xffff;
		// D is officially indeterminate, so leave it alone
	}
	else
	{
		if (frac)
		{
			set_ix(0, (u32(m_d) << 16) / divisor);
			m_d = (u32(m_d) << 16) % divisor;
		}
		else
		{
			set_ix(0, m_d / divisor);
			m_d %= divisor;
		}

		// Set Z flag if quotient is zero; clear V and C
		m_ccr = (m_ccr & 0xf8ff) | (get_ix(0) == 0 ? 0x0400 : 0);
	}
}

void cpu16_device::pshm_step(int n)
{
	switch (n)
	{
	case 0:
		if (BIT(m_fetch_pipe[2], 0))
		{
			m_sequence = seq::PSHM_0;
			m_tmp = m_d;
			break;
		}
		[[fallthrough]];

	case 1:
		if (BIT(m_fetch_pipe[2], 1))
		{
			m_sequence = seq::PSHM_1;
			m_tmp = m_e;
			break;
		}
		[[fallthrough]];

	case 2:
		if (BIT(m_fetch_pipe[2], 2))
		{
			m_sequence = seq::PSHM_2;
			m_tmp = get_ix(0);
			break;
		}
		[[fallthrough]];

	case 3:
		if (BIT(m_fetch_pipe[2], 3))
		{
			m_sequence = seq::PSHM_3;
			m_tmp = get_ix(1);
			break;
		}
		[[fallthrough]];

	case 4:
		if (BIT(m_fetch_pipe[2], 4))
		{
			m_sequence = seq::PSHM_4;
			m_tmp = get_ix(2);
			break;
		}
		[[fallthrough]];

	case 5:
		if (BIT(m_fetch_pipe[2], 5))
		{
			m_sequence = seq::PSHM_5;
			m_tmp = get_k();
			break;
		}
		[[fallthrough]];

	case 6:
		if (BIT(m_fetch_pipe[2], 6))
		{
			m_sequence = seq::PSHM_6;
			m_tmp = m_ccr;
			break;
		}
		[[fallthrough]];

	default:
		m_sequence = seq::PSHM_8;
		break;
	}
}

void cpu16_device::pulm_step(int n)
{
	while (n < 8 && !BIT(m_fetch_pipe[2], n))
		++n;

	switch (n)
	{
	case 0:
		m_ccr = (m_tmp & 0xfff0) | (m_ccr & 0x000f);
		m_sequence = seq::PULM_1;
		break;

	case 1:
		set_k(m_tmp);
		m_sequence = seq::PULM_2;
		break;

	case 2:
		set_ix(2, m_tmp);
		m_sequence = seq::PULM_3;
		break;

	case 3:
		set_ix(1, m_tmp);
		m_sequence = seq::PULM_4;
		break;

	case 4:
		set_ix(0, m_tmp);
		m_sequence = seq::PULM_5;
		break;

	case 5:
		m_e = m_tmp;
		m_sequence = seq::PULM_6;
		break;

	case 6:
		m_d = m_tmp;
		m_sequence = seq::PULM_7;
		break;

	default:
		m_sequence = seq::PULM_8;
		break;
	}
}

bool cpu16_device::cc_test(u8 cc) const noexcept
{
	switch (cc)
	{
	case 0x00:
		// Unary: always (1)
		return true;

	case 0x01: default:
		// Unary: never (0)
		return false;

	case 0x02:
		// Unsigned: higher (C + Z = 0)
		return !BIT(m_ccr, 8) && !BIT(m_ccr, 10);

	case 0x03:
		// Unsigned: lower or same (C + Z = 1)
		return BIT(m_ccr, 8) || BIT(m_ccr, 10);

	case 0x04:
		// Simple: carry clear (C = 0)
		return !BIT(m_ccr, 8);

	case 0x05:
		// Simple: carry set (C = 1)
		return BIT(m_ccr, 8);

	case 0x06:
		// Simple: not equal to zero (Z = 0)
		return !BIT(m_ccr, 10);

	case 0x07:
		// Simple: equal to zero (Z = 1)
		return BIT(m_ccr, 10);

	case 0x08:
		// Simple: overflow clear (V = 0)
		return !BIT(m_ccr, 9);

	case 0x09:
		// Simple: overflow set (V = 1)
		return BIT(m_ccr, 9);

	case 0x0a:
		// Simple: plus sign (N = 0)
		return !BIT(m_ccr, 11);

	case 0x0b:
		// Simple: minus sign (N = 1)
		return BIT(m_ccr, 11);

	case 0x0c:
		// Signed: greater than or equal (N ⊕ V = 0)
		return BIT(m_ccr, 11) == BIT(m_ccr, 9);

	case 0x0d:
		// Signed: less than (N ⊕ V = 1)
		return BIT(m_ccr, 11) != BIT(m_ccr, 9);

	case 0x0e:
		// Signed: greater than (Z + (N ⊕ V) = 0)
		return !BIT(m_ccr, 10) && BIT(m_ccr, 11) == BIT(m_ccr, 9);

	case 0x0f:
		// Signed: less than or equal (Z + (N ⊕ V) = 1)
		return BIT(m_ccr, 10) || BIT(m_ccr, 11) != BIT(m_ccr, 9);

	case 0x10:
		// MAC unit: AM overflow set (MV = 1)
		return BIT(m_ccr, 14);

	case 0x11:
		// MAC unit: extension bit overflow set (EV = 1)
		return BIT(m_ccr, 12);
	}
}

void cpu16_device::execute_run()
{
	while (m_icount > 0)
	{
		if (m_start)
		{
			debugger_instruction_hook(m_fwa);
			u8 page = (m_fetch_pipe[2] & 0xcf00) == 0x0700 ? BIT(m_fetch_pipe[2], 12, 2) : 0;
			m_sequence = s_inst_decode[page][page != 0 ? BIT(m_fetch_pipe[2], 0, 8) : BIT(m_fetch_pipe[2], 8, 8)];
		}

		switch (m_sequence)
		{
		case seq::BOOT_0:
			m_tmp = m_cache.read_word(0x00000); // reserved:ZK:SK:PK
			m_sequence = seq::BOOT_1;
			m_icount -= 2;
			break;

		case seq::BOOT_1:
			m_pc = m_cache.read_word(0x00002) | u32(m_tmp & 0x000f) << 16;
			m_ccr = (m_ccr & 0xfff0) | (m_tmp & 0x000f);
			m_sequence = seq::BOOT_2;
			m_icount -= 2;
			break;

		case seq::BOOT_2:
			m_index_regs[3] = m_cache.read_word(0x00004) | u32(m_tmp & 0x00f0) << 12;
			m_sequence = seq::BOOT_3;
			m_icount -= 2;
			break;

		case seq::BOOT_3:
			m_index_regs[2] = m_cache.read_word(0x00006) | u32(m_tmp & 0x0f00) << 8;
			m_sequence = seq::PREFETCH_1;
			m_icount -= 2;
			break;

		case seq::INVALID: // TODO: these are trapped
			logerror("%05X: Invalid/unimplemented instruction word $%04X encountered\n", m_fwa, m_fetch_pipe[2]);
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::PREFETCH_1:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_sequence = seq::PREFETCH_2;
			advance();
			m_icount -= 2;
			break;

		case seq::PREFETCH_2:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::LOAD8_EA:
			m_tmp = m_data.read_byte(m_ea);
			m_sequence = m_return_sequence;
			m_icount -= 2;
			break;

		case seq::LOAD8_EXT:
			m_tmp = m_data.read_byte(m_ea);
			m_sequence = m_return_sequence;
			m_icount -= 2;
			advance();
			break;

		case seq::LOAD16_EA:
			if (BIT(m_ea, 0))
			{
				m_tmp = m_data.read_byte(m_ea);
				m_ea = (m_ea + 1) & 0xfffff;
				m_sequence = seq::LOAD16_EA_LSB;
			}
			else
			{
				m_tmp = m_data.read_word(m_ea);
				m_sequence = m_return_sequence;
			}
			m_icount -= 2;
			break;

		case seq::LOAD16_EA_LSB:
			m_tmp = m_tmp << 8 | m_data.read_byte(m_ea);
			m_ea = (m_ea - 1) & 0xfffff;
			m_sequence = m_return_sequence;
			m_icount -= 2;
			break;

		case seq::LOAD16_EXT:
			if (BIT(m_ea, 0))
			{
				m_tmp = m_data.read_byte(m_ea);
				m_ea = (m_ea + 1) & 0xfffff;
				m_sequence = seq::LOAD16_EXT_LSB;
			}
			else
			{
				m_tmp = m_data.read_word(m_ea);
				m_sequence = m_return_sequence;
				advance();
			}
			m_icount -= 2;
			break;

		case seq::LOAD16_EXT_LSB:
			m_tmp = m_tmp << 8 | m_data.read_byte(m_ea);
			m_ea = (m_ea - 1) & 0xfffff;
			m_sequence = m_return_sequence;
			advance();
			m_icount -= 2;
			break;

		case seq::STORE8_EA:
			m_data.write_word(m_ea, (m_tmp & 0x00ff) | (m_tmp << 8), BIT(m_ea, 0) ? 0x00ff : 0xff00);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::STORE8_EXT:
			m_data.write_word(m_ea, (m_tmp & 0x00ff) | (m_tmp << 8), BIT(m_ea, 0) ? 0x00ff : 0xff00);
			m_sequence = seq::PREFETCH_2;
			advance();
			m_icount -= 2;
			break;

		case seq::STORE16_EA:
			if (BIT(m_ea, 0))
			{
				m_data.write_word(m_ea, (m_tmp & 0xff00) | (m_tmp >> 8), 0x00ff);
				m_ea = (m_ea + 1) & 0xfffff;
				m_sequence = seq::STORE8_EA;
			}
			else
			{
				m_data.write_word(m_ea, m_tmp);
				m_start = true;
				advance();
			}
			m_icount -= 2;
			break;

		case seq::STORE16_EXT:
			if (BIT(m_ea, 0))
			{
				m_data.write_word(m_ea, (m_tmp & 0xff00) | (m_tmp >> 8), 0x00ff);
				m_ea = (m_ea + 1) & 0xfffff;
				m_sequence = seq::STORE8_EXT;
			}
			else
			{
				m_data.write_word(m_ea, m_tmp);
				m_sequence = seq::PREFETCH_2;
				advance();
			}
			m_icount -= 2;
			break;

		case seq::ABA:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_a(adc8(m_d >> 8, m_d & 0x00ff, false));
			advance();
			m_icount -= 2;
			break;

		case seq::ABX:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + (m_d & 0x00ff)) & 0xfffff; // B is zero-extended to 20 bits
			advance();
			m_icount -= 2;
			break;

		case seq::ADDA_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::ADDA_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ADDA_IND8_3:
			set_a(adc8(m_d >> 8, m_tmp, BIT(m_fetch_pipe[2], 9) && BIT(m_ccr, 8)));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ADDA_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_a(adc8(m_d >> 8, m_fetch_pipe[2] & 0x00ff, BIT(m_fetch_pipe[2], 9) && BIT(m_ccr, 8)));
			advance();
			m_icount -= 2;
			break;

		case seq::ADDA_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::ADDA_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ADDA_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_a(adc8(m_d >> 8, m_tmp, BIT(m_fetch_pipe[2], 1) && BIT(m_ccr, 8)));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ADDA_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::ADDA_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ADDA_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::ADDA_INDE_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ADDA_INDE_3:
			set_a(adc8(m_d >> 8, m_tmp, BIT(m_fetch_pipe[2], 1) && BIT(m_ccr, 8)));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ADDB_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::ADDB_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ADDB_IND8_3:
			set_b(adc8(m_d & 0x00ff, m_tmp, BIT(m_fetch_pipe[2], 9) && BIT(m_ccr, 8)));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ADDB_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_b(adc8(m_d & 0x00ff, m_fetch_pipe[2] & 0x00ff, BIT(m_fetch_pipe[2], 9) && BIT(m_ccr, 8)));
			advance();
			m_icount -= 2;
			break;

		case seq::ADDB_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::ADDB_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ADDB_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_b(adc8(m_d & 0x00ff, m_tmp, BIT(m_fetch_pipe[2], 1) && BIT(m_ccr, 8)));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ADDB_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::ADDB_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ADDB_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::ADDB_INDE_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ADDB_INDE_3:
			set_b(adc8(m_d & 0x00ff, m_tmp, BIT(m_fetch_pipe[2], 1) && BIT(m_ccr, 8)));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ADDD_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD16_EA;
			m_return_sequence = seq::ADDD_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ADDD_IND8_3:
			m_d = adc16(m_d, m_tmp, BIT(m_fetch_pipe[2], 9) && BIT(m_ccr, 8));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ADDD_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d = adc16(m_d, s16(s8(m_fetch_pipe[2] & 0x00ff)), false);
			advance();
			m_icount -= 2;
			break;

		case seq::ADDD_IMM16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_fetch_pipe[1];
			m_sequence = seq::ADDD_IND16_3;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::ADDD_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::ADDD_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ADDD_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d = adc16(m_d, m_tmp, BIT(m_fetch_pipe[2], 1) && BIT(m_ccr, 8));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ADDD_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::ADDD_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ADDD_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD16_EA;
			m_return_sequence = seq::ADDD_INDE_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ADDD_INDE_3:
			m_d = adc16(m_d, m_tmp, BIT(m_fetch_pipe[2], 1) && BIT(m_ccr, 8));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ADDE_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_e = adc16(m_e, s16(s8(m_fetch_pipe[2] & 0x00ff)), false);
			advance();
			m_icount -= 2;
			break;

		case seq::ADDE_IMM16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_fetch_pipe[1];
			m_sequence = seq::ADDE_IND16_3;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::ADDE_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::ADDE_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ADDE_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_e = adc16(m_e, m_tmp, BIT(m_fetch_pipe[2], 1) && BIT(m_ccr, 8));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ADDE_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::ADDE_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ADE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_e = adc16(m_e, m_d, false);
			advance();
			m_icount -= 2;
			break;

		case seq::ADX:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_d)) & 0xfffff;
			advance();
			m_icount -= 2;
			break;

		case seq::AEX:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			advance();
			m_icount -= 2;
			break;

		case seq::AIX_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_index_regs[BIT(m_fetch_pipe[2], 8, 2)] = (m_index_regs[BIT(m_fetch_pipe[2], 8, 2)] + s8(m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			if (BIT(m_fetch_pipe[2], 8, 2) != 3)
				set_z16(get_ix(BIT(m_fetch_pipe[2], 8, 2)));
			advance();
			m_icount -= 2;
			break;

		case seq::AIX_IMM16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_index_regs[BIT(m_fetch_pipe[2], 0, 2)] = (m_index_regs[BIT(m_fetch_pipe[2], 0, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			if (BIT(m_fetch_pipe[2], 0, 2) != 3)
				set_z16(get_ix(BIT(m_fetch_pipe[2], 0, 2)));
			m_sequence = seq::PREFETCH_2;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::ANDA_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::ANDA_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ANDA_IND8_3:
			m_d &= m_tmp << 8 | 0x00ff;
			set_nzv8(m_d >> 8, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ANDA_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d &= m_fetch_pipe[2] << 8 | 0x00ff;
			set_nzv8(m_d >> 8, false);
			advance();
			m_icount -= 2;
			break;

		case seq::ANDA_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::ANDA_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ANDA_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d &= m_tmp << 8 | 0x00ff;
			set_nzv8(m_d >> 8, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ANDA_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::ANDA_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ANDA_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::ANDA_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ANDB_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::ANDB_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ANDB_IND8_3:
			m_d &= m_tmp | 0xff00;
			set_nzv8(m_d & 0x00ff, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ANDB_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d &= m_fetch_pipe[2] | 0xff00;
			set_nzv8(m_d & 0x00ff, false);
			advance();
			m_icount -= 2;
			break;

		case seq::ANDB_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::ANDB_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ANDB_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d &= m_tmp | 0xff00;
			set_nzv8(m_d & 0x00ff, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ANDB_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::ANDB_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ANDB_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::ANDB_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ANDD_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD16_EA;
			m_return_sequence = seq::ANDD_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ANDD_IND8_3:
			m_d &= m_tmp;
			set_nzv16(m_d, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ANDD_IMM16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_fetch_pipe[1];
			m_sequence = seq::ANDD_IND16_3;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::ANDD_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::ANDD_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ANDD_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d &= m_tmp;
			set_nzv16(m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ANDD_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::ANDD_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ANDD_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD16_EA;
			m_return_sequence = seq::ANDD_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ANDE_IMM16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_fetch_pipe[1];
			m_sequence = seq::ANDE_IND16_3;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::ANDE_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::ANDE_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ANDE_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_e &= m_tmp;
			set_nzv16(m_e, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ANDE_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::ANDE_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ANDP:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ccr &= m_fetch_pipe[1] | 0x000f; // PK is unaffected
			m_sequence = seq::PREFETCH_2; // TODO: prohibit interrupts
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::ASR_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::ASR_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ASR_IND8_3:
			m_tmp = asr8(m_tmp);
			m_sequence = seq::STORE8_EA;
			m_icount -= 2;
			break;

		case seq::ASR_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::ASR_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ASR_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = asr8(m_tmp);
			m_sequence = seq::STORE8_EA;
			m_icount -= 2;
			break;

		case seq::ASR_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::ASR_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ASRA:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_a(asr8(m_d >> 8));
			advance();
			m_icount -= 2;
			break;

		case seq::ASRB:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_b(asr8(m_d & 0x00ff));
			advance();
			m_icount -= 2;
			break;

		case seq::ASRD:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d = asr16(m_d);
			advance();
			m_icount -= 2;
			break;

		case seq::ASRE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_e = asr16(m_e);
			advance();
			m_icount -= 2;
			break;

		case seq::ASRW_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::ASRW_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ASRW_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = asr16(m_tmp);
			m_sequence = seq::STORE16_EA;
			m_icount -= 2;
			break;

		case seq::ASRW_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::ASRW_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BCLR_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::BCLR_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BCLR_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp &= ~m_fetch_pipe[2] & 0x00ff;
			set_nzv8(m_tmp, false);
			m_sequence = seq::STORE8_EA;
			m_icount -= 2;
			break;

		case seq::BCLR_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::BCLR_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BCLR_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + (m_fetch_pipe[1] & 0x00ff)) & 0xfffff;
			m_sequence = seq::BCLR_IND8_2;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BCLR_IND8_2:
			m_tmp = (m_fetch_pipe[1] & 0xff00) | m_data.read_byte(m_ea);
			m_sequence = seq::BCLR_IND8_3;
			advance();
			m_icount -= 2;
			break;

		case seq::BCLR_IND8_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp &= m_tmp >> 8;
			set_nzv8(m_tmp, false);
			m_sequence = seq::STORE8_EA;
			m_icount -= 2;
			break;

		case seq::BCLRW_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::BCLRW_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BCLRW_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp &= ~m_fetch_pipe[1];
			set_nzv16(m_tmp, false);
			m_sequence = seq::STORE16_EXT;
			m_icount -= 2;
			break;

		case seq::BCLRW_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::BCLRW_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BITA_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::BITA_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BITA_IND8_3:
			set_nzv8((m_d >> 8) & m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::BITA_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_nzv8((m_d >> 8) & m_fetch_pipe[2] & 0x00ff, false);
			advance();
			m_icount -= 2;
			break;

		case seq::BITA_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::BITA_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BITA_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_nzv8((m_d >> 8) & m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::BITA_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::BITA_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BITA_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::BITA_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BITB_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::BITB_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BITB_IND8_3:
			set_nzv8(m_d & m_tmp & 0x00ff, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::BITB_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_nzv8(m_d & m_fetch_pipe[2] & 0x00ff, false);
			advance();
			m_icount -= 2;
			break;

		case seq::BITB_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::BITB_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BITB_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_nzv8(m_d & m_tmp & 0x00ff, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::BITB_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::BITB_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BITB_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::BITB_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BCC:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			if (cc_test(BIT(m_fetch_pipe[2], 8, 4)))
			{
				m_sequence = seq::PREFETCH_1;
				m_start = false;
				set_pc(m_pc + s8(m_fetch_pipe[2] & 0x00ff));
			}
			advance();
			m_icount -= 2;
			break;

		case seq::BRSET_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[1] >> 8)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::BRSET_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BRSET_IND8_3:
			m_tmp = (BIT(m_fetch_pipe[2], 14) ? m_tmp : ~m_tmp) & m_fetch_pipe[2] & 0x00ff;
			m_ea = (m_pc + 2 + s8(m_fetch_pipe[1] & 0x00fe)) & 0xfffff;
			m_sequence = seq::BRSET_IND8_4;
			advance();
			m_icount -= 2;
			break;

		case seq::BRSET_IND8_4:
			if (m_tmp == 0)
			{
				m_sequence = seq::PREFETCH_1;
				m_pc = m_ea;
			}
			else
				m_sequence = seq::PREFETCH_2;
			m_icount -= 2;
			break;

		case seq::BRSET_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::BRSET_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BRSET_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = (BIT(m_fetch_pipe[2], 8) ? ~m_tmp : m_tmp) & m_fetch_pipe[2] & 0x00ff;
			m_ea = (m_pc + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::BRSET_IND8_4;
			advance();
			m_icount -= 2;
			break;

		case seq::BRSET_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::BRSET_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BSET_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::BSET_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BSET_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp |= m_fetch_pipe[2] & 0x00ff;
			set_nzv8(m_tmp, false);
			m_sequence = seq::STORE8_EA;
			m_icount -= 2;
			break;

		case seq::BSET_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::BSET_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BSET_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + (m_fetch_pipe[1] & 0x00ff)) & 0xfffff;
			m_sequence = seq::BSET_IND8_2;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BSET_IND8_2:
			m_tmp = (m_fetch_pipe[1] & 0xff00) | m_data.read_byte(m_ea);
			m_sequence = seq::BSET_IND8_3;
			advance();
			m_icount -= 2;
			break;

		case seq::BSET_IND8_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = (m_tmp >> 8) | (m_tmp & 0x00ff);
			set_nzv8(m_tmp, false);
			m_sequence = seq::STORE8_EA;
			m_icount -= 2;
			break;

		case seq::BSETW_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::BSETW_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BSETW_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp |= m_fetch_pipe[1];
			set_nzv16(m_tmp, false);
			m_sequence = seq::STORE16_EXT;
			m_icount -= 2;
			break;

		case seq::BSETW_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::BSETW_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BSR:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_pc & 0x0ffff;
			m_sequence = seq::BSR_2;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::BSR_2:
			if (BIT(m_index_regs[3], 0))
			{
				m_data.write_word(m_index_regs[3], (m_tmp & 0xff00) | (m_tmp >> 8), 0x00ff);
				m_index_regs[3] = (m_index_regs[3] + 1) & 0xfffff;
				m_sequence = seq::BSR_2B;
			}
			else
			{
				m_data.write_word(m_index_regs[3], m_tmp);
				m_tmp = m_ccr;
				m_index_regs[3] = (m_index_regs[3] - 2) & 0xfffff;
				m_sequence = seq::BSR_3;
			}
			m_icount -= 2;
			break;

		case seq::BSR_2B:
			m_data.write_word(m_index_regs[3], (m_tmp & 0x00ff) | (m_tmp << 8), 0xff00);
			m_index_regs[3] = (m_index_regs[3] - 3) & 0xfffff;
			m_tmp = m_ccr;
			m_sequence = seq::BSR_3;
			m_icount -= 2;
			break;

		case seq::BSR_3:
			if (BIT(m_index_regs[3], 0))
			{
				m_data.write_word(m_index_regs[3], (m_tmp & 0xff00) | (m_tmp >> 8), 0x00ff);
				m_index_regs[3] = (m_index_regs[3] + 1) & 0xfffff;
				m_sequence = seq::BSR_3B;
			}
			else
			{
				m_data.write_word(m_index_regs[3], m_tmp);
				set_pc(m_pc + 2 + s8(m_fetch_pipe[2] & 0x00ff));
				m_index_regs[3] = (m_index_regs[3] - 2) & 0xfffff;
				m_sequence = seq::PREFETCH_1;
			}
			m_icount -= 2;
			break;

		case seq::BSR_3B:
			m_data.write_word(m_index_regs[3], (m_tmp & 0x00ff) | (m_tmp << 8), 0xff00);
			set_pc(u32(m_fetch_pipe[2] & 0x000f) << 16 | m_fetch_pipe[1]);
			m_index_regs[3] = (m_index_regs[3] - 3) & 0xfffff;
			m_sequence = seq::PREFETCH_1;
			m_icount -= 2;
			break;

		case seq::CBA:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			(void)sbc8(m_d >> 8, m_d & 0x00ff, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::CLR_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = 0;
			set_nzv8(0, false);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::STORE8_EA;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CLR_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = 0;
			set_nzv8(0, false);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::STORE8_EXT;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CLR_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = 0;
			set_nzv8(0, false);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::STORE8_EXT;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CLRA:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d &= 0x00ff;
			set_nzv8(0, false);
			advance();
			m_icount -= 2;
			break;

		case seq::CLRB:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d &= 0xff00;
			set_nzv8(0, false);
			advance();
			m_icount -= 2;
			break;

		case seq::CLRD:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d = 0;
			set_nzv16(0, false);
			advance();
			m_icount -= 2;
			break;

		case seq::CLRE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_e = 0;
			set_nzv16(0, false);
			advance();
			m_icount -= 2;
			break;

		case seq::CLRM:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_am = 0;
			m_ccr &= 0xafff; // MV and EV flags cleared
			advance();
			m_icount -= 2;
			break;

		case seq::CLRW_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = 0;
			set_nzv16(0, false);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::STORE16_EXT;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CLRW_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = 0;
			set_nzv16(0, false);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::STORE16_EXT;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CMPA_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::CMPA_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CMPA_IND8_3:
			(void)sbc8(m_d >> 8, m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::CMPA_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			(void)sbc8(m_d >> 8, m_fetch_pipe[2] & 0x00ff, false);
			advance();
			m_icount -= 2;
			break;

		case seq::CMPA_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::CMPA_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CMPA_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			(void)sbc8(m_d >> 8, m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::CMPA_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::CMPA_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CMPA_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::CMPA_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CMPB_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::CMPB_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CMPB_IND8_3:
			(void)sbc8(m_d & 0x00ff, m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::CMPB_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			(void)sbc8(m_d & 0x00ff, m_fetch_pipe[2] & 0x00ff, false);
			advance();
			m_icount -= 2;
			break;

		case seq::CMPB_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::CMPB_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CMPB_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			(void)sbc8(m_d & 0x00ff, m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::CMPB_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::CMPB_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CMPB_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::CMPB_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CPD_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD16_EA;
			m_return_sequence = seq::CPD_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CPD_IND8_3:
			(void)sbc16(m_d, m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::CPD_IMM16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_fetch_pipe[1];
			m_sequence = seq::CPD_IND16_3;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::CPD_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::CPD_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CPD_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			(void)sbc16(m_d, m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::CPD_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::CPD_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CPD_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD16_EA;
			m_return_sequence = seq::CPD_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CPE_IMM16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_fetch_pipe[1];
			m_sequence = seq::CPE_IND16_3;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::CPE_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::CPE_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CPE_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			(void)sbc16(m_e, m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::CPE_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::CPE_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CPX_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD16_EA;
			m_return_sequence = seq::CPX_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CPX_IND8_3:
			(void)sbc16(get_ix(BIT(m_fetch_pipe[2], 8, 2)), m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::CPX_IMM16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_fetch_pipe[1];
			m_sequence = seq::CPX_IND16_3;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::CPX_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::CPX_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::CPX_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			(void)sbc16(get_ix(BIT(m_fetch_pipe[2], 0, 2)), m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::CPX_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::CPX_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::DAA:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			if ((m_d & 0x0f00) > 0x0900 || BIT(m_ccr, 13))
			{
				// Decimal adjust A[3:0]
				if ((m_d & 0xff00) >= 0xfa00)
					m_ccr |= 0x0100;
				m_d += 0x0600;
			}
			if ((m_d & 0xf000) > 0x9000 || BIT(m_ccr, 8))
			{
				// Decimal adjust A[7:4]
				m_ccr |= 0x0100;
				m_d += 0x6000;
			}
			// V is officially undefined (as with decimal arithmetic on most other CPUs), so just leave it alone here
			set_nzv8(m_d >> 8, BIT(m_ccr, 9));
			advance();
			m_icount -= 2;
			break;

		case seq::DEC_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::DEC_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::DEC_IND8_3:
			--m_tmp;
			set_nzv8(m_tmp, m_tmp == 0x007f);
			m_sequence = seq::STORE8_EA;
			m_icount -= 2;
			break;

		case seq::DEC_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::DEC_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::DEC_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			--m_tmp;
			set_nzv8(m_tmp, m_tmp == 0x007f);
			m_sequence = seq::STORE8_EA;
			m_icount -= 2;
			break;

		case seq::DEC_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::DEC_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::DECA:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d -= 0x0100;
			set_nzv8(m_d >> 8, (m_d & 0xff00) == 0x7f00);
			advance();
			m_icount -= 2;
			break;

		case seq::DECB:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_b((m_d - 1) & 0x00ff);
			set_nzv8(m_d & 0x00ff, (m_d & 0x00ff) == 0x007f);
			advance();
			m_icount -= 2;
			break;

		case seq::DECW_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::DECW_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::DECW_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			--m_tmp;
			set_nzv16(m_tmp, m_tmp == 0x7fff);
			m_sequence = seq::STORE16_EA;
			m_icount -= 2;
			break;

		case seq::DECW_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::DECW_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::DIV:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_sequence = seq::DIV_2;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::DIV_2:
			m_sequence = seq::DIV_3;
			m_icount -= 2;
			break;

		case seq::DIV_3:
			m_sequence = seq::DIV_4;
			m_icount -= 2;
			break;

		case seq::DIV_4:
			m_sequence = seq::DIV_5;
			m_icount -= 2;
			break;

		case seq::DIV_5:
			m_sequence = seq::DIV_6;
			m_icount -= 2;
			break;

		case seq::DIV_6:
			m_sequence = seq::DIV_7;
			m_icount -= 2;
			break;

		case seq::DIV_7:
			m_sequence = seq::DIV_8;
			m_icount -= 2;
			break;

		case seq::DIV_8:
			m_sequence = seq::DIV_9;
			m_icount -= 2;
			break;

		case seq::DIV_9:
			m_sequence = seq::DIV_10;
			m_icount -= 2;
			break;

		case seq::DIV_10:
			m_sequence = seq::DIV_11;
			m_icount -= 2;
			break;

		case seq::DIV_11:
			divu16(BIT(m_fetch_pipe[2], 0));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::EMUL:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_sequence = seq::EMUL_2;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::EMUL_2:
			m_sequence = seq::EMUL_3;
			m_icount -= 2;
			break;

		case seq::EMUL_3:
			m_sequence = seq::EMUL_4;
			m_icount -= 2;
			break;

		case seq::EMUL_4:
			m_sequence = seq::EMUL_5;
			m_icount -= 2;
			break;

		case seq::EMUL_5:
			mulu16();
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::EMULS:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_sequence = seq::EMULS_2;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::EMULS_2:
			m_sequence = seq::EMULS_3;
			m_icount -= 2;
			break;

		case seq::EMULS_3:
			m_sequence = seq::EMULS_4;
			m_icount -= 2;
			break;

		case seq::EMULS_4:
			muls16(BIT(m_fetch_pipe[2], 0));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::EORA_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::EORA_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::EORA_IND8_3:
			m_d ^= m_tmp << 8;
			set_nzv8(m_d >> 8, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::EORA_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d ^= m_fetch_pipe[2] << 8;
			set_nzv8(m_d >> 8, false);
			advance();
			m_icount -= 2;
			break;

		case seq::EORA_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::EORA_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::EORA_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d ^= m_tmp << 8;
			set_nzv8(m_d >> 8, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::EORA_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::EORA_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::EORA_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::EORA_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::EORB_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::EORB_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::EORB_IND8_3:
			m_d ^= m_tmp & 0x00ff;
			set_nzv8(m_d & 0x00ff, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::EORB_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d ^= m_fetch_pipe[2] & 0x00ff;
			set_nzv8(m_d & 0x00ff, false);
			advance();
			m_icount -= 2;
			break;

		case seq::EORB_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::EORB_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::EORB_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d ^= m_tmp & 0x00ff;
			set_nzv8(m_d & 0x00ff, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::EORB_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::EORB_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::EORB_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::EORB_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::EORD_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD16_EA;
			m_return_sequence = seq::EORD_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::EORD_IND8_3:
			m_d ^= m_tmp;
			set_nzv16(m_d, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::EORD_IMM16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_fetch_pipe[1];
			m_sequence = seq::EORD_IND16_3;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::EORD_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::EORD_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::EORD_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d ^= m_tmp;
			set_nzv16(m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::EORD_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::EORD_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::EORD_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD16_EA;
			m_return_sequence = seq::EORD_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::EORE_IMM16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_fetch_pipe[1];
			m_sequence = seq::EORE_IND16_3;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::EORE_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::EORE_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::EORE_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_e &= m_tmp;
			set_nzv16(m_e, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::EORE_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::EORE_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::INC_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::INC_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::INC_IND8_3:
			++m_tmp;
			set_nzv8(m_tmp, m_tmp == 0x0080);
			m_sequence = seq::STORE8_EA;
			m_icount -= 2;
			break;

		case seq::INC_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::INC_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::INC_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			++m_tmp;
			set_nzv8(m_tmp, m_tmp == 0x0080);
			m_sequence = seq::STORE8_EA;
			m_icount -= 2;
			break;

		case seq::INC_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::INC_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::INCA:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d += 0x0100;
			set_nzv8(m_d >> 8, (m_d & 0xff00) == 0x8000);
			advance();
			m_icount -= 2;
			break;

		case seq::INCB:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_b((m_d + 1) & 0x00ff);
			set_nzv8(m_d & 0x00ff, (m_d & 0x00ff) == 0x0080);
			advance();
			m_icount -= 2;
			break;

		case seq::INCW_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::INCW_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::INCW_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			++m_tmp;
			set_nzv16(m_tmp, m_tmp == 0x8000);
			m_sequence = seq::STORE16_EA;
			m_icount -= 2;
			break;

		case seq::INCW_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::INCW_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::JMP_EXT20:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_pc(u32(m_fetch_pipe[2] & 0x000f) << 16 | m_fetch_pipe[1]);
			m_sequence = seq::PREFETCH_1;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::JSR_EXT20:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = (m_pc + 2) & 0x0ffff;
			m_sequence = seq::JSR_EXT20_2;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::JSR_EXT20_2:
			if (BIT(m_index_regs[3], 0))
			{
				m_data.write_word(m_index_regs[3], (m_tmp & 0xff00) | (m_tmp >> 8), 0x00ff);
				m_index_regs[3] = (m_index_regs[3] + 1) & 0xfffff;
				m_sequence = seq::JSR_EXT20_2B;
			}
			else
			{
				m_data.write_word(m_index_regs[3], m_tmp);
				m_tmp = m_ccr;
				m_index_regs[3] = (m_index_regs[3] - 2) & 0xfffff;
				m_sequence = seq::JSR_EXT20_3;
			}
			m_icount -= 2;
			break;

		case seq::JSR_EXT20_2B:
			m_data.write_word(m_index_regs[3], (m_tmp & 0x00ff) | (m_tmp << 8), 0xff00);
			m_index_regs[3] = (m_index_regs[3] - 3) & 0xfffff;
			m_tmp = m_ccr;
			m_sequence = seq::JSR_EXT20_3;
			m_icount -= 2;
			break;

		case seq::JSR_EXT20_3:
			if (BIT(m_index_regs[3], 0))
			{
				m_data.write_word(m_index_regs[3], (m_tmp & 0xff00) | (m_tmp >> 8), 0x00ff);
				m_index_regs[3] = (m_index_regs[3] + 1) & 0xfffff;
				m_sequence = seq::JSR_EXT20_3B;
			}
			else
			{
				m_data.write_word(m_index_regs[3], m_tmp);
				set_pc(u32(m_fetch_pipe[2] & 0x000f) << 16 | m_fetch_pipe[1]);
				m_index_regs[3] = (m_index_regs[3] - 2) & 0xfffff;
				m_sequence = seq::PREFETCH_1;
			}
			m_icount -= 2;
			break;

		case seq::JSR_EXT20_3B:
			m_data.write_word(m_index_regs[3], (m_tmp & 0x00ff) | (m_tmp << 8), 0xff00);
			set_pc(u32(m_fetch_pipe[2] & 0x000f) << 16 | m_fetch_pipe[1]);
			m_index_regs[3] = (m_index_regs[3] - 3) & 0xfffff;
			m_sequence = seq::PREFETCH_1;
			m_icount -= 2;
			break;

		case seq::LBCC:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			if (cc_test(BIT(m_fetch_pipe[2], 0, 5)))
			{
				set_pc(m_pc + s16(m_fetch_pipe[1]));
				m_sequence = seq::PREFETCH_1;
			}
			else
				m_sequence = seq::PREFETCH_2;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::LBSR:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = (m_pc + 2) & 0x0ffff;
			m_sequence = seq::LBSR_2;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LBSR_2:
			if (BIT(m_index_regs[3], 0))
			{
				m_data.write_word(m_index_regs[3], (m_tmp & 0xff00) | (m_tmp >> 8), 0x00ff);
				m_index_regs[3] = (m_index_regs[3] + 1) & 0xfffff;
				m_sequence = seq::LBSR_2B;
			}
			else
			{
				m_data.write_word(m_index_regs[3], m_tmp);
				m_tmp = m_ccr;
				m_index_regs[3] = (m_index_regs[3] - 2) & 0xfffff;
				m_sequence = seq::LBSR_3;
			}
			m_icount -= 2;
			break;

		case seq::LBSR_2B:
			m_data.write_word(m_index_regs[3], (m_tmp & 0x00ff) | (m_tmp << 8), 0xff00);
			m_index_regs[3] = (m_index_regs[3] - 3) & 0xfffff;
			m_tmp = m_ccr;
			m_sequence = seq::LBSR_3;
			m_icount -= 2;
			break;

		case seq::LBSR_3:
			if (BIT(m_index_regs[3], 0))
			{
				m_data.write_word(m_index_regs[3], (m_tmp & 0xff00) | (m_tmp >> 8), 0x00ff);
				m_index_regs[3] = (m_index_regs[3] + 1) & 0xfffff;
				m_sequence = seq::LBSR_3B;
			}
			else
			{
				m_data.write_word(m_index_regs[3], m_tmp);
				set_pc(m_pc + 2 + s16(m_fetch_pipe[1]));
				m_index_regs[3] = (m_index_regs[3] - 2) & 0xfffff;
				m_sequence = seq::PREFETCH_1;
			}
			m_icount -= 2;
			break;

		case seq::LBSR_3B:
			m_data.write_word(m_index_regs[3], (m_tmp & 0x00ff) | (m_tmp << 8), 0xff00);
			set_pc(m_pc + 2 + s16(m_fetch_pipe[1]));
			m_index_regs[3] = (m_index_regs[3] - 3) & 0xfffff;
			m_sequence = seq::PREFETCH_1;
			m_icount -= 2;
			break;

		case seq::LDAA_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::LDAA_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LDAA_IND8_3:
			set_a(m_tmp);
			set_nzv8(m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::LDAA_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_a(m_fetch_pipe[2] & 0x00ff);
			set_nzv8(m_fetch_pipe[2] & 0x00ff, false);
			advance();
			m_icount -= 2;
			break;

		case seq::LDAA_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::LDAA_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LDAA_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_a(m_tmp);
			set_nzv8(m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::LDAA_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::LDAA_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LDAA_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::LDAA_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LDAB_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::LDAB_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LDAB_IND8_3:
			set_b(m_tmp);
			set_nzv8(m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::LDAB_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_b(m_fetch_pipe[2] & 0x00ff);
			set_nzv8(m_fetch_pipe[2] & 0x00ff, false);
			advance();
			m_icount -= 2;
			break;

		case seq::LDAB_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::LDAB_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LDAB_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_b(m_tmp);
			set_nzv8(m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::LDAB_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::LDAB_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LDAB_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::LDAB_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LDD_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD16_EA;
			m_return_sequence = seq::LDD_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LDD_IND8_3:
			m_d = m_tmp;
			set_nzv16(m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::LDD_IMM16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_fetch_pipe[1];
			m_sequence = seq::LDD_IND16_3;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::LDD_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::LDD_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LDD_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d = m_tmp;
			set_nzv16(m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::LDD_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::LDD_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LDD_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD16_EA;
			m_return_sequence = seq::LDD_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LDE_IMM16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_fetch_pipe[1];
			m_sequence = seq::LDE_IND16_3;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::LDE_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::LDE_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LDE_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_e = m_tmp;
			set_nzv16(m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::LDE_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::LDE_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LDED:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LDED_2;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LDED_2:
			if (BIT(m_ea, 0))
			{
				m_tmp = m_data.read_byte(m_ea);
				m_ea = (m_ea + 1) & 0xfffff;
				m_sequence = seq::LDED_2B;
			}
			else
			{
				m_tmp = m_data.read_word(m_ea);
				m_ea = (m_ea + 2) & 0xfffff;
				m_sequence = seq::LDED_3;
			}
			m_icount -= 2;
			break;

		case seq::LDED_2B:
			m_tmp = m_tmp << 8 | m_data.read_byte(m_ea);
			m_ea = (m_ea + 1) & 0xfffff;
			m_sequence = seq::LDED_3;
			m_icount -= 2;
			break;

		case seq::LDED_3:
			m_e = m_tmp;
			if (BIT(m_ea, 0))
			{
				m_tmp = m_data.read_byte(m_ea);
				m_ea = (m_ea + 1) & 0xfffff;
				m_sequence = seq::LDED_3B;
			}
			else
			{
				m_tmp = m_data.read_word(m_ea);
				m_sequence = seq::LDED_4;
				advance();
			}
			m_icount -= 2;
			break;

		case seq::LDED_3B:
			m_tmp = m_tmp << 8 | m_data.read_byte(m_ea);
			m_sequence = seq::LDED_4;
			advance();
			m_icount -= 2;
			break;

		case seq::LDED_4:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d = m_tmp;
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::LDHI:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = m_index_regs[0];
			m_sequence = seq::LDHI_2;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LDHI_2:
			if (BIT(m_ea, 0))
			{
				m_tmp = m_data.read_byte(m_ea);
				m_ea = (m_ea + 1) & 0xfffff;
				m_sequence = seq::LDHI_2B;
			}
			else
			{
				m_tmp = m_data.read_word(m_ea);
				m_ea = m_index_regs[1];
				m_sequence = seq::LDHI_3;
			}
			m_icount -= 2;
			break;

		case seq::LDHI_2B:
			m_tmp = m_tmp << 8 | m_data.read_byte(m_ea);
			m_ea = m_index_regs[1];
			m_sequence = seq::LDHI_3;
			m_icount -= 2;
			break;

		case seq::LDHI_3:
			m_hr = m_tmp;
			if (BIT(m_ea, 0))
			{
				m_tmp = m_data.read_byte(m_ea);
				m_ea = (m_ea + 1) & 0xfffff;
				m_sequence = seq::LDHI_3B;
			}
			else
			{
				m_tmp = m_data.read_word(m_ea);
				m_sequence = seq::LDHI_4;
			}
			m_icount -= 2;
			break;

		case seq::LDHI_3B:
			m_tmp = m_tmp << 8 | m_data.read_byte(m_ea);
			m_sequence = seq::LDHI_4;
			m_icount -= 2;
			break;

		case seq::LDHI_4:
			m_ir = m_tmp;
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::LDX_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD16_EA;
			m_return_sequence = seq::LDX_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LDX_IND8_3:
			set_ix(BIT(m_fetch_pipe[2], 8, 2), m_tmp);
			set_nzv16(m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::LDX_IMM16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_fetch_pipe[1];
			m_sequence = seq::LDX_IND16_3;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::LDX_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::LDX_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::LDX_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_ix(BIT(m_fetch_pipe[2], 0, 2), m_tmp);
			set_nzv16(m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::LDX_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::LDX_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::MOVB_IXP_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = m_index_regs[0];
			m_index_regs[0] = u32(m_ek) << 16 | m_fetch_pipe[1]; // HACK: save destination in XK:IX to prevent it from being overwritten
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::MOVB_IXP_EXT_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::MOVB_IXP_EXT_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_nzv8(m_tmp, false);
			m_index_regs[0] = (std::exchange(m_ea, m_index_regs[0]) + s8(m_fetch_pipe[2] & 0x00ff)) & 0xfffff; // post-modify XK:IX by signed offset
			m_sequence = seq::STORE8_EA;
			m_icount -= 2;
			break;

		case seq::MOVB_EXT_IXP:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = m_index_regs[0];
			m_index_regs[0] = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::MOVB_EXT_IXP_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::MOVB_EXT_IXP_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_nzv8(m_tmp, false);
			m_ea = m_index_regs[0];
			m_index_regs[0] = (m_index_regs[0] + s8(m_fetch_pipe[2] & 0x00ff)) & 0xfffff; // post-modify XK:IX by signed offset
			m_sequence = seq::STORE8_EA;
			m_icount -= 2;
			break;

		case seq::MOVB_EXT_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::MOVB_EXT_EXT_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::MOVB_EXT_EXT_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_nzv8(m_tmp, false);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::STORE8_EXT;
			m_icount -= 2;
			break;

		case seq::MOVW_IXP_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = m_index_regs[0];
			m_index_regs[0] = u32(m_ek) << 16 | m_fetch_pipe[1]; // HACK: save destination in XK:IX to prevent it from being overwritten
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::MOVW_IXP_EXT_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::MOVW_IXP_EXT_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_nzv16(m_tmp, false);
			m_index_regs[0] = (std::exchange(m_ea, m_index_regs[0]) + s8(m_fetch_pipe[2] & 0x00ff)) & 0xfffff; // post-modify XK:IX by signed offset
			m_sequence = seq::STORE16_EA;
			m_icount -= 2;
			break;

		case seq::MOVW_EXT_IXP:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = m_index_regs[0];
			m_index_regs[0] = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::MOVW_EXT_IXP_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::MOVW_EXT_IXP_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_nzv8(m_tmp, false);
			m_ea = m_index_regs[0];
			m_index_regs[0] = (m_index_regs[0] + s8(m_fetch_pipe[2] & 0x00ff)) & 0xfffff; // post-modify XK:IX by signed offset
			m_sequence = seq::STORE16_EA;
			m_icount -= 2;
			break;

		case seq::MOVW_EXT_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::MOVW_EXT_EXT_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::MOVW_EXT_EXT_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_nzv16(m_tmp, false);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::STORE16_EXT;
			m_icount -= 2;
			break;

		case seq::MUL:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_sequence = seq::MUL_2;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::MUL_2:
			m_sequence = seq::MUL_3;
			m_icount -= 2;
			break;

		case seq::MUL_3:
			m_sequence = seq::MUL_4;
			m_icount -= 2;
			break;

		case seq::MUL_4:
			m_sequence = seq::MUL_5;
			m_icount -= 2;
			break;

		case seq::MUL_5:
			mulu8();
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::NEG_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::NEG_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::NEG_IND8_3:
			m_tmp = sbc8(0, m_tmp, !BIT(m_fetch_pipe[2], 1));
			m_sequence = seq::STORE8_EA;
			m_icount -= 2;
			break;

		case seq::NEG_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::NEG_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::NEG_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = sbc8(0, m_tmp, !BIT(m_fetch_pipe[2], 1));
			m_sequence = seq::STORE8_EA;
			m_icount -= 2;
			break;

		case seq::NEG_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::NEG_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::NEGA:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_a(sbc8(0, m_d >> 8, !BIT(m_fetch_pipe[2], 1)));
			advance();
			m_icount -= 2;
			break;

		case seq::NEGB:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_b(sbc8(0, m_d & 0x00ff, !BIT(m_fetch_pipe[2], 1)));
			advance();
			m_icount -= 2;
			break;

		case seq::NEGD:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d = sbc16(0, m_d, !BIT(m_fetch_pipe[2], 1));
			advance();
			m_icount -= 2;
			break;

		case seq::NEGE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_e = sbc16(0, m_e, !BIT(m_fetch_pipe[2], 1));
			advance();
			m_icount -= 2;
			break;

		case seq::NEGW_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::NEGW_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::NEGW_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = sbc16(0, m_tmp, !BIT(m_fetch_pipe[2], 1));
			set_nzv16(m_tmp, false);
			m_sequence = seq::STORE16_EA;
			m_icount -= 2;
			break;

		case seq::NEGW_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::NEGW_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ORAA_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::ORAA_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ORAA_IND8_3:
			m_d |= m_tmp << 8;
			set_nzv8(m_d >> 8, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ORAA_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d |= m_fetch_pipe[2] << 8;
			set_nzv8(m_d >> 8, false);
			advance();
			m_icount -= 2;
			break;

		case seq::ORAA_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::ORAA_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ORAA_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d |= m_tmp << 8;
			set_nzv8(m_d >> 8, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ORAA_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::ORAA_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ORAA_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::ORAA_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ORAB_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::ORAB_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ORAB_IND8_3:
			m_d |= m_tmp & 0x00ff;
			set_nzv8(m_d & 0x00ff, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ORAB_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d |= m_fetch_pipe[2] & 0x00ff;
			set_nzv8(m_d & 0x00ff, false);
			advance();
			m_icount -= 2;
			break;

		case seq::ORAB_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::ORAB_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ORAB_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d |= m_tmp & 0x00ff;
			set_nzv8(m_d & 0x00ff, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ORAB_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::ORAB_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ORAB_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::ORAB_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ORD_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD16_EA;
			m_return_sequence = seq::ORD_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ORD_IND8_3:
			m_d |= m_tmp;
			set_nzv16(m_d, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ORD_IMM16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_fetch_pipe[1];
			m_sequence = seq::ORD_IND16_3;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::ORD_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::ORD_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ORD_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d |= m_tmp;
			set_nzv16(m_tmp, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ORD_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::ORD_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ORD_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD16_EA;
			m_return_sequence = seq::ORD_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ORE_IMM16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_fetch_pipe[1];
			m_sequence = seq::ORE_IND16_3;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::ORE_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::ORE_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ORE_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_e &= m_tmp;
			set_nzv16(m_e, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ORE_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::ORE_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ORP:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ccr |= m_fetch_pipe[1] & 0xfff0; // PK is unaffected
			m_sequence = seq::PREFETCH_2; // TODO: prohibit interrupts
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::PSHA:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[3] + 1) & 0xfffff;
			m_tmp = m_d >> 8;
			m_sequence = seq::PSHA_2;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::PSHA_2:
			m_data.write_word(m_ea, m_tmp | (m_tmp << 8), BIT(m_ea, 0) ? 0x00ff : 0xff00);
			m_index_regs[3] = (m_ea - 2) & 0xfffff; // these instructions may leave the stack pointer misaligned
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::PSHB:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[3] + 1) & 0xfffff;
			m_tmp = m_d & 0x00ff;
			m_sequence = seq::PSHA_2;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::PSHM:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_start = false;
			pshm_step(0);
			m_icount -= 2;
			break;

		case seq::PSHM_0:
		case seq::PSHM_1:
		case seq::PSHM_2:
		case seq::PSHM_3:
		case seq::PSHM_4:
		case seq::PSHM_5:
		case seq::PSHM_6:
			if (BIT(m_index_regs[3], 0))
			{
				m_data.write_word(m_index_regs[3], (m_tmp & 0xff00) | (m_tmp >> 8), 0x00ff);
				m_index_regs[3] = (m_index_regs[3] + 1) & 0xfffff;
				m_sequence = static_cast<seq>(static_cast<int>(m_sequence) - static_cast<int>(seq::PSHM_0) + static_cast<int>(seq::PSHM_0B));
			}
			else
			{
				m_data.write_word(m_index_regs[3], m_tmp);
				m_index_regs[3] = (m_index_regs[3] - 2) & 0xfffff;
				pshm_step(static_cast<int>(m_sequence) - static_cast<int>(seq::PSHM_0) + 1);
			}
			m_icount -= 2;
			break;

		case seq::PSHM_0B:
		case seq::PSHM_1B:
		case seq::PSHM_2B:
		case seq::PSHM_3B:
		case seq::PSHM_4B:
		case seq::PSHM_5B:
		case seq::PSHM_6B:
			m_data.write_word(m_index_regs[3], (m_tmp & 0x00ff) | (m_tmp << 8), BIT(m_ea, 0) ? 0x00ff : 0xff00);
			pshm_step(static_cast<int>(m_sequence) - static_cast<int>(seq::PSHM_0B) + 1);
			m_icount -= 2;
			break;

		case seq::PSHM_8:
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::PULA:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[3] + 2) & 0xfffff;
			m_sequence = seq::PULA_2;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::PULA_2:
			m_tmp = m_data.read_byte(m_ea);
			m_index_regs[3] = (m_ea - 1) & 0xfffff;
			m_sequence = seq::PULA_3;
			m_icount -= 2;
			break;

		case seq::PULA_3:
			set_a(m_tmp);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::PULB:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[3] + 2) & 0xfffff;
			m_sequence = seq::PULB_2;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::PULB_2:
			m_tmp = m_data.read_byte(m_ea);
			m_index_regs[3] = (m_ea - 1) & 0xfffff;
			m_sequence = seq::PULB_3;
			m_icount -= 2;
			break;

		case seq::PULB_3:
			set_b(m_tmp);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::PULM:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[3] + 2) & 0xfffff;
			m_sequence = seq::PULM_0;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::PULM_0:
		case seq::PULM_1:
		case seq::PULM_2:
		case seq::PULM_3:
		case seq::PULM_4:
		case seq::PULM_5:
		case seq::PULM_6:
		case seq::PULM_7:
			if (BIT(m_ea, 0))
			{
				m_tmp = m_data.read_byte(m_ea);
				m_ea = (m_ea + 1) & 0xfffff;
				m_sequence = static_cast<seq>(static_cast<int>(m_sequence) - static_cast<int>(seq::PULM_0) + static_cast<int>(seq::PULM_0B));
			}
			else
			{
				m_tmp = m_data.read_word(m_ea);
				m_ea = (m_ea + 2) & 0xfffff;
				pulm_step(static_cast<int>(m_sequence) - static_cast<int>(seq::PULM_0));
			}
			m_icount -= 2;
			break;

		case seq::PULM_0B:
		case seq::PULM_1B:
		case seq::PULM_2B:
		case seq::PULM_3B:
		case seq::PULM_4B:
		case seq::PULM_5B:
		case seq::PULM_6B:
		case seq::PULM_7B:
			m_tmp = u16(m_tmp) << 8 | m_data.read_byte(m_ea);
			m_ea = (m_ea + 1) & 0xfffff;
			pulm_step(static_cast<int>(m_sequence) - static_cast<int>(seq::PULM_0B));
			m_icount -= 2;
			break;

		case seq::PULM_8:
			// The algorithm overshoots and reads one word too many... as documented
			m_index_regs[3] = (m_ea - 4) & 0xfffff;
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::ROL_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::ROL_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ROL_IND8_3:
			m_tmp = rol8(m_tmp, BIT(m_fetch_pipe[2], 11) && BIT(m_ccr, 8));
			m_sequence = seq::STORE8_EA;
			m_icount -= 2;
			break;

		case seq::ROL_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::ROL_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ROL_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = rol8(m_tmp, BIT(m_fetch_pipe[2], 3) && BIT(m_ccr, 8));
			m_sequence = seq::STORE8_EA;
			m_icount -= 2;
			break;

		case seq::ROL_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::ROL_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ROLA:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_a(rol8(m_d >> 8, BIT(m_fetch_pipe[2], 3) && BIT(m_ccr, 8)));
			advance();
			m_icount -= 2;
			break;

		case seq::ROLB:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_b(rol8(m_d & 0x00ff, BIT(m_fetch_pipe[2], 3) && BIT(m_ccr, 8)));
			advance();
			m_icount -= 2;
			break;

		case seq::ROLD:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d = rol16(m_d, BIT(m_fetch_pipe[2], 3) && BIT(m_ccr, 8));
			advance();
			m_icount -= 2;
			break;

		case seq::ROLE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_e = rol16(m_e, BIT(m_fetch_pipe[2], 3) && BIT(m_ccr, 8));
			advance();
			m_icount -= 2;
			break;

		case seq::ROLW_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::ROLW_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ROLW_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = rol16(m_tmp, BIT(m_fetch_pipe[2], 3) && BIT(m_ccr, 8));
			m_sequence = seq::STORE16_EA;
			m_icount -= 2;
			break;

		case seq::ROLW_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::ROLW_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ROR_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::ROR_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ROR_IND8_3:
			m_tmp = ror8(m_tmp, !BIT(m_fetch_pipe[2], 8) && BIT(m_ccr, 8));
			m_sequence = seq::STORE8_EA;
			m_icount -= 2;
			break;

		case seq::ROR_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::ROR_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::ROR_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = ror8(m_tmp, !BIT(m_fetch_pipe[2], 0) && BIT(m_ccr, 8));
			m_sequence = seq::STORE8_EA;
			m_icount -= 2;
			break;

		case seq::ROR_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::ROR_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::RORA:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_a(ror8(m_d >> 8, !BIT(m_fetch_pipe[2], 0) && BIT(m_ccr, 8)));
			advance();
			m_icount -= 2;
			break;

		case seq::RORB:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_b(ror8(m_d & 0x00ff, !BIT(m_fetch_pipe[2], 0) && BIT(m_ccr, 8)));
			advance();
			m_icount -= 2;
			break;

		case seq::RORD:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d = ror16(m_d, !BIT(m_fetch_pipe[2], 0) && BIT(m_ccr, 8));
			advance();
			m_icount -= 2;
			break;

		case seq::RORE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_e = ror16(m_e, !BIT(m_fetch_pipe[2], 0) && BIT(m_ccr, 8));
			advance();
			m_icount -= 2;
			break;

		case seq::RORW_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::RORW_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::RORW_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = ror16(m_tmp, !BIT(m_fetch_pipe[2], 0) && BIT(m_ccr, 8));
			m_sequence = seq::STORE16_EA;
			m_icount -= 2;
			break;

		case seq::RORW_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::RORW_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::RTS:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_index_regs[3] = (m_index_regs[3] + 2) & 0xfffff;
			m_sequence = seq::RTS_2;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::RTS_2:
			if (BIT(m_index_regs[3], 0))
			{
				m_tmp = m_data.read_byte(m_index_regs[3]);
				m_index_regs[3] = (m_index_regs[3] + 1) & 0xfffff;
				m_sequence = seq::RTS_2B;
			}
			else
			{
				m_ea = u32(m_data.read_word(m_index_regs[3])) << 16;
				m_index_regs[3] = (m_index_regs[3] + 2) & 0xfffff;
				m_sequence = seq::RTS_3;
			}
			m_icount -= 2;
			break;

		case seq::RTS_2B:
			m_ea = (m_tmp << 8 | m_data.read_byte(m_index_regs[3])) << 16;
			m_index_regs[3] = (m_index_regs[3] + 1) & 0xfffff;
			m_sequence = seq::RTS_3;
			m_icount -= 2;
			break;

		case seq::RTS_3:
			if (BIT(m_index_regs[3], 0))
			{
				m_tmp = m_data.read_byte(m_index_regs[3]);
				m_index_regs[3] = (m_index_regs[3] + 1) & 0xfffff;
				m_sequence = seq::RTS_3B;
			}
			else
			{
				m_ea |= m_data.read_word(m_index_regs[3]);
				m_sequence = seq::RTS_4;
			}
			m_icount -= 2;
			break;

		case seq::RTS_3B:
			m_ea |= m_tmp << 8 | m_data.read_byte(m_index_regs[3]);
			m_index_regs[3] = (m_index_regs[3] - 1) & 0xfffff;
			m_sequence = seq::RTS_4;
			m_icount -= 2;
			break;

		case seq::RTS_4:
			set_pc(m_ea - 2);
			m_sequence = seq::PREFETCH_1;
			m_icount -= 2;
			break;

		case seq::SBA:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_a(sbc8(m_d >> 8, m_d & 0x00ff, false));
			advance();
			m_icount -= 2;
			break;

		case seq::SDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_e = sbc16(m_e, m_d, false);
			advance();
			m_icount -= 2;
			break;

		case seq::STAA_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = (m_d & 0xff00) >> 8;
			set_nzv8(m_tmp, false);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::STORE8_EA;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::STAA_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = (m_d & 0xff00) >> 8;
			set_nzv8(m_tmp, false);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::STORE8_EXT;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::STAA_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = (m_d & 0xff00) >> 8;
			set_nzv8(m_tmp, false);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::STORE8_EXT;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::STAA_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = (m_d & 0xff00) >> 8;
			set_nzv8(m_tmp, false);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::STORE8_EA;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::STAB_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_d & 0x00ff;
			set_nzv8(m_tmp, false);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::STORE8_EA;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::STAB_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_d & 0x00ff;
			set_nzv8(m_tmp, false);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::STORE8_EXT;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::STAB_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_d & 0x00ff;
			set_nzv8(m_tmp, false);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::STORE8_EXT;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::STAB_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_d & 0x00ff;
			set_nzv8(m_tmp, false);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::STORE8_EA;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::STD_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_d;
			set_nzv16(m_d, false);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::STORE16_EA;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::STD_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_d;
			set_nzv16(m_d, false);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::STORE16_EXT;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::STD_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_d;
			set_nzv16(m_d, false);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::STORE16_EXT;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::STD_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_d;
			set_nzv16(m_d, false);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::STORE16_EA;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::STE_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_e;
			set_nzv16(m_e, false);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::STORE16_EXT;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::STE_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_e;
			set_nzv16(m_e, false);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::STORE16_EXT;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::STED:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_e;
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::STED_2;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::STED_2:
			if (BIT(m_ea, 0))
			{
				m_data.write_word(m_ea, (m_tmp & 0xff00) | (m_tmp >> 8), 0x00ff);
				m_ea = (m_ea + 1) & 0xfffff;
				m_sequence = seq::STED_2B;
			}
			else
			{
				m_data.write_word(m_ea, m_tmp);
				m_tmp = m_d;
				m_ea = (m_ea + 2) & 0xfffff;
				m_sequence = seq::STORE16_EXT;
			}
			m_icount -= 2;
			break;

		case seq::STED_2B:
			m_data.write_word(m_ea, (m_tmp & 0x00ff) | (m_tmp << 8), 0xff00);
			m_ea = (m_ea + 1) & 0xfffff;
			m_tmp = m_d;
			m_sequence = seq::STORE16_EXT;
			m_icount -= 2;
			break;

		case seq::STX_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = get_ix(BIT(m_fetch_pipe[2], 8, 2));
			set_nzv16(m_tmp, false);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::STORE16_EA;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::STX_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = get_ix(BIT(m_fetch_pipe[2], 0, 2));
			set_nzv16(m_tmp, false);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::STORE16_EXT;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::STX_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = get_ix(BIT(m_fetch_pipe[2], 0, 2));
			set_nzv16(m_tmp, false);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::STORE16_EXT;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::SUBA_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::SUBA_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::SUBA_IND8_3:
			set_a(sbc8(m_d >> 8, m_tmp, BIT(m_fetch_pipe[2], 9) && BIT(m_ccr, 8)));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::SUBA_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_a(sbc8(m_d >> 8, m_fetch_pipe[2] & 0x00ff, BIT(m_fetch_pipe[2], 9) && BIT(m_ccr, 8)));
			advance();
			m_icount -= 2;
			break;

		case seq::SUBA_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::SUBA_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::SUBA_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_a(sbc8(m_d >> 8, m_tmp, BIT(m_fetch_pipe[2], 1) && BIT(m_ccr, 8)));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::SUBA_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::SUBA_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::SUBA_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::SUBA_INDE_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::SUBA_INDE_3:
			set_a(sbc8(m_d >> 8, m_tmp, BIT(m_fetch_pipe[2], 1) && BIT(m_ccr, 8)));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::SUBB_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::SUBB_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::SUBB_IND8_3:
			set_b(sbc8(m_d & 0x00ff, m_tmp, BIT(m_fetch_pipe[2], 9) && BIT(m_ccr, 8)));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::SUBB_IMM8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_b(sbc8(m_d & 0x00ff, m_fetch_pipe[2] & 0x00ff, BIT(m_fetch_pipe[2], 9) && BIT(m_ccr, 8)));
			advance();
			m_icount -= 2;
			break;

		case seq::SUBB_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::SUBB_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::SUBB_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_b(sbc8(m_d & 0x00ff, m_tmp, BIT(m_fetch_pipe[2], 1) && BIT(m_ccr, 8)));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::SUBB_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::SUBB_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::SUBB_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::SUBB_INDE_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::SUBB_INDE_3:
			set_b(sbc8(m_d & 0x00ff, m_tmp, BIT(m_fetch_pipe[2], 1) && BIT(m_ccr, 8)));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::SUBD_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD16_EA;
			m_return_sequence = seq::SUBD_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::SUBD_IND8_3:
			m_d = sbc16(m_d, m_tmp, BIT(m_fetch_pipe[2], 9) && BIT(m_ccr, 8));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::SUBD_IMM16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_fetch_pipe[1];
			m_sequence = seq::SUBD_IND16_3;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::SUBD_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::SUBD_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::SUBD_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d = sbc16(m_d, m_tmp, BIT(m_fetch_pipe[2], 1) && BIT(m_ccr, 8));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::SUBD_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::SUBD_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::SUBD_INDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_e)) & 0xfffff;
			m_sequence = seq::LOAD16_EA;
			m_return_sequence = seq::SUBD_INDE_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::SUBD_INDE_3:
			m_d = sbc16(m_d, m_tmp, BIT(m_fetch_pipe[2], 1) && BIT(m_ccr, 8));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::SUBE_IMM16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_tmp = m_fetch_pipe[1];
			m_sequence = seq::SUBE_IND16_3;
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::SUBE_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::SUBE_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::SUBE_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_e = sbc16(m_e, m_tmp, BIT(m_fetch_pipe[2], 1) && BIT(m_ccr, 8));
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::SUBE_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u32(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::SUBE_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::SXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d = s16(s8(m_d & 0x00ff));
			set_nzv16(m_d, BIT(m_ccr, 9)); // V is unaffected
			advance();
			m_icount -= 2;
			break;

		case seq::TAB:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_b(m_d >> 8);
			set_nzv8(m_d & 0x00ff, false);
			advance();
			m_icount -= 2;
			break;

		case seq::TAP:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ccr = (m_d & 0xff00) | (m_ccr & 0x00ff);
			m_sequence = seq::PREFETCH_2; // TODO: prohibit interrupts
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::TBA:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_a(m_d & 0x00ff);
			set_nzv8(m_d & 0x00ff, false);
			advance();
			m_icount -= 2;
			break;

		case seq::TBEK:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ek = m_d & 0x000f;
			advance();
			m_icount -= 2;
			break;

		case seq::TBXK:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_xk(BIT(m_fetch_pipe[2], 0, 2), m_d & 0x000f);
			advance();
			m_icount -= 2;
			break;

		case seq::TDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_e = m_d;
			set_nzv16(m_d, false);
			advance();
			m_icount -= 2;
			break;

		case seq::TDMSK:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_index_mask[0] = m_d >> 8;
			m_index_mask[1] = m_d & 0x00ff;
			advance();
			m_icount -= 2;
			break;

		case seq::TDP:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ccr = (m_d & 0xfff0) | (m_ccr & 0x000f); // PK is unaffected
			m_sequence = seq::PREFETCH_2; // TODO: prohibit interrupts
			m_start = false;
			advance();
			m_icount -= 2;
			break;

		case seq::TED:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d = m_e;
			set_nzv16(m_e, false);
			advance();
			m_icount -= 2;
			break;

		case seq::TEKB:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_b(m_ek);
			advance();
			m_icount -= 2;
			break;

		case seq::TPA:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_a(m_ccr >> 8);
			advance();
			m_icount -= 2;
			break;

		case seq::TPD:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d = m_ccr;
			advance();
			m_icount -= 2;
			break;

		case seq::TST_IND8:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 12, 2)] + (m_fetch_pipe[2] & 0x00ff)) & 0xfffff;
			m_sequence = seq::LOAD8_EA;
			m_return_sequence = seq::TST_IND8_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::TST_IND8_3:
			(void)sbc8(m_tmp, 0, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::TST_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::TST_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::TST_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			(void)sbc8(m_tmp, 0, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::TST_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD8_EXT;
			m_return_sequence = seq::TST_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::TSTA:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			(void)sbc8((m_d & 0xff00) >> 8, 0, false);
			advance();
			m_icount -= 2;
			break;

		case seq::TSTB:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			(void)sbc8(m_d & 0x00ff, 0, false);
			advance();
			m_icount -= 2;
			break;

		case seq::TSTD:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			(void)sbc16(m_d, 0, false);
			advance();
			m_icount -= 2;
			break;

		case seq::TSTE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			(void)sbc16(m_e, 0, false);
			advance();
			m_icount -= 2;
			break;

		case seq::TSTW_IND16:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] + s16(m_fetch_pipe[1])) & 0xfffff;
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::TSTW_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::TSTW_IND16_3:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			(void)sbc16(m_tmp, 0, false);
			m_start = true;
			advance();
			m_icount -= 2;
			break;

		case seq::TSTW_EXT:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_ea = u16(m_ek) << 16 | m_fetch_pipe[1];
			m_sequence = seq::LOAD16_EXT;
			m_return_sequence = seq::TSTW_IND16_3;
			m_start = false;
			m_icount -= 2;
			break;

		case seq::TSX:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] = (m_index_regs[3] + 2) & 0xfffff;
			advance();
			m_icount -= 2;
			break;

		case seq::TXKB:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_b(get_xk(BIT(m_fetch_pipe[2], 0, 2)));
			advance();
			m_icount -= 2;
			break;

		case seq::TXS:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_index_regs[3] = (m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] - 2) & 0xfffff;
			advance();
			m_icount -= 2;
			break;

		case seq::TXX:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_index_regs[BIT(m_fetch_pipe[2], 4, 2)] = m_index_regs[BIT(m_fetch_pipe[2], 0, 2)];
			advance();
			m_icount -= 2;
			break;

		case seq::XGAB:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			m_d = swapendian_int16(m_d);
			advance();
			m_icount -= 2;
			break;

		case seq::XGDE:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			std::swap(m_d, m_e);
			advance();
			m_icount -= 2;
			break;

		case seq::XGDX:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_ix(BIT(m_fetch_pipe[2], 4, 2), std::exchange(m_d, get_ix(BIT(m_fetch_pipe[2], 4, 2))));
			advance();
			m_icount -= 2;
			break;

		case seq::XGEX:
			m_fetch_pipe[0] = m_cache.read_word(m_pc);
			set_ix(BIT(m_fetch_pipe[2], 4, 2), std::exchange(m_e, get_ix(BIT(m_fetch_pipe[2], 4, 2))));
			advance();
			m_icount -= 2;
			break;
		}
	}
}

void cpu16_device::execute_set_input(int inputnum, int state)
{
	// TODO
}

void cpu16_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c%c%c%c%c$%d %s",
			BIT(m_ccr, 15) ? '.' : 'S',
			BIT(m_ccr, 14) ? 'M' : '.',
			BIT(m_ccr, 13) ? 'H' : '.',
			BIT(m_ccr, 12) ? 'E' : '.',
			BIT(m_ccr, 11) ? 'N' : '.',
			BIT(m_ccr, 10) ? 'Z' : '.',
			BIT(m_ccr, 9) ? 'V' : '.',
			BIT(m_ccr, 8) ? 'C' : '.',
			BIT(m_ccr, 5, 3),
			BIT(m_ccr, 4) ? "SM" : "  ");
		break;
	}
}

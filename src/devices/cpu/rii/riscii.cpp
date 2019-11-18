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
    on-chip peripherals are mostly unemulated.

***************************************************************************/

#include "emu.h"
#include "riscii.h"
#include "riidasm.h"

// device type definitions
DEFINE_DEVICE_TYPE(EPG3231, epg3231_device, "epg3231", "Elan ePG3231")

ALLOW_SAVE_TYPE(riscii_series_device::exec_state);


//**************************************************************************
//  DEVICE CONSTRUCTION AND INITIALIZATION
//**************************************************************************

void riscii_series_device::core_regs_map(address_map &map)
{
	// INDF0 (R00h) is not physically implemented
	map(0x0001, 0x0001).rw(FUNC(riscii_series_device::fsr0_r), FUNC(riscii_series_device::fsr0_w));
	map(0x0002, 0x0002).rw(FUNC(riscii_series_device::pcl_r), FUNC(riscii_series_device::pcl_w));
	map(0x0003, 0x0003).rw(FUNC(riscii_series_device::pcm_r), FUNC(riscii_series_device::pcm_w));
	if (m_pcmask > 0xffff)
		map(0x0004, 0x0004).rw(FUNC(riscii_series_device::pch_r), FUNC(riscii_series_device::pch_w));
	map(0x0005, 0x0005).rw(FUNC(riscii_series_device::bsr_r), FUNC(riscii_series_device::bsr_w));
	map(0x0006, 0x0006).rw(FUNC(riscii_series_device::stkptr_r), FUNC(riscii_series_device::stkptr_w));
	map(0x0007, 0x0007).rw(FUNC(riscii_series_device::bsr1_r), FUNC(riscii_series_device::bsr1_w));
	// INDF1 (R08h) is not physically implemented
	map(0x0009, 0x0009).rw(FUNC(riscii_series_device::fsr1_r), FUNC(riscii_series_device::fsr1_w));
	map(0x000a, 0x000a).rw(FUNC(riscii_series_device::acc_r), FUNC(riscii_series_device::acc_w));
	map(0x000b, 0x000b).rw(FUNC(riscii_series_device::tabptrl_r), FUNC(riscii_series_device::tabptrl_w));
	map(0x000c, 0x000c).rw(FUNC(riscii_series_device::tabptrm_r), FUNC(riscii_series_device::tabptrm_w));
	map(0x000d, 0x000d).rw(FUNC(riscii_series_device::tabptrh_r), FUNC(riscii_series_device::tabptrh_w));
	map(0x000e, 0x000e).rw(FUNC(riscii_series_device::cpucon_r), FUNC(riscii_series_device::cpucon_w));
	map(0x000f, 0x000f).rw(FUNC(riscii_series_device::status_r), FUNC(riscii_series_device::status_w));
	for (unsigned b = 0; b <= m_maxbank; b++)
		map(0x0080 | (b << 8), 0x00ff | (b << 8)).ram();
}

std::unique_ptr<util::disasm_interface> epg3231_device::create_disassembler()
{
	return std::make_unique<epg3231_disassembler>();
}

riscii_series_device::riscii_series_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, unsigned addrbits, unsigned pcbits, u32 datastart, unsigned bankbits, u8 maxbank, u8 post_id_mask, address_map_constructor regs)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, addrbits, -1)
	, m_regs_config("register", ENDIANNESS_LITTLE, 8, 8 + bankbits, 0, regs)
	, m_program(nullptr)
	, m_regs(nullptr)
	, m_cache(nullptr)
	, m_porta_in_cb(*this)
	, m_port_in_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
	, m_port_out_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
	, m_pcmask((1 << pcbits) - 1)
	, m_datastart(datastart)
	, m_tbptmask(((1 << (addrbits + 1)) - 1) | (datastart != 0 ? 0x800000 : 0))
	, m_bankmask((1 << bankbits) - 1)
	, m_maxbank(maxbank)
	, m_post_id_mask(post_id_mask)
	, m_pc(0)
	, m_acc(0)
	, m_fsr{0, 0}
	, m_bsr{0, 0}
	, m_tabptr(0)
	, m_stkptr(0)
	, m_cpucon(0)
	, m_status(0)
	, m_prod(0)
	, m_pfs(0)
	, m_intcon(0)
	, m_intsta(0)
	, m_post_id(0)
	, m_port_data{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	, m_port_dcr{0, 0, 0, 0, 0, 0}
	, m_port_control{0, 0}
	, m_stbcon(0)
	, m_painten(0)
	, m_paintsta(0)
	, m_pawake(0)
	, m_uartcon(0)
	, m_uartsta(0)
	, m_spicon(0)
	, m_spista(0)
	, m_trl0(0)
	, m_trl1(0)
	, m_trl2(0)
	, m_tr01con(0)
	, m_tr2con(0)
	, m_trlir(0)
	, m_sfcr(0)
	, m_add{0, 0, 0, 0}
	, m_env{0, 0, 0, 0}
	, m_mtcon{0, 0, 0, 0}
	, m_mtrl{0, 0, 0, 0}
	, m_sphdr(0)
	, m_sphtcon(0)
	, m_sphtrl(0)
	, m_vocon(0)
	, m_icount(0)
	, m_exec_state(EXEC_CYCLE1)
	, m_repeat(0)
	, m_curreg(0)
{
}

void epg3231_device::regs_map(address_map &map)
{
	core_regs_map(map);
	map(0x0010, 0x0010).rw(FUNC(epg3231_device::trl2_r), FUNC(epg3231_device::trl2_w));
	map(0x0011, 0x0011).rw(FUNC(epg3231_device::prodl_r), FUNC(epg3231_device::prodl_w));
	map(0x0012, 0x0012).rw(FUNC(epg3231_device::prodh_r), FUNC(epg3231_device::prodh_w));
	// TODO: ADOTL (R13h), ADOTH (R14h)
	map(0x0015, 0x0015).w(FUNC(epg3231_device::uarttx_w));
	map(0x0016, 0x0016).r(FUNC(epg3231_device::uartrx_r));
	map(0x0017, 0x0017).r(FUNC(epg3231_device::porta_r));
	map(0x0018, 0x001f).rw(FUNC(epg3231_device::port_r), FUNC(epg3231_device::port_w));
	map(0x0020, 0x0020).rw(FUNC(epg3231_device::pfs_r), FUNC(epg3231_device::pfs_w));
	map(0x0021, 0x0021).rw(FUNC(epg3231_device::stbcon_r), FUNC(epg3231_device::stbcon_w));
	map(0x0022, 0x0022).rw(FUNC(epg3231_device::intcon_r), FUNC(epg3231_device::intcon_w));
	map(0x0023, 0x0023).rw(FUNC(epg3231_device::intsta_r), FUNC(epg3231_device::intsta_w));
	map(0x0024, 0x0024).rw(FUNC(epg3231_device::trl0l_r), FUNC(epg3231_device::trl0l_w));
	map(0x0025, 0x0025).rw(FUNC(epg3231_device::trl0h_r), FUNC(epg3231_device::trl0h_w));
	map(0x0026, 0x0026).rw(FUNC(epg3231_device::trl1_r), FUNC(epg3231_device::trl1_w));
	map(0x0027, 0x0027).rw(FUNC(epg3231_device::tr01con_r), FUNC(epg3231_device::tr01con_w));
	map(0x0028, 0x0028).rw(FUNC(epg3231_device::tr2con_r), FUNC(epg3231_device::tr2con_w));
	map(0x0028, 0x0028).rw(FUNC(epg3231_device::trlir_r), FUNC(epg3231_device::trlir_w));
	// R2Ah is reserved
	map(0x002b, 0x002b).rw(FUNC(epg3231_device::post_id_r), FUNC(epg3231_device::post_id_w));
	// TODO: ADCON (R2Ch)
	map(0x002d, 0x002d).rw(FUNC(epg3231_device::painten_r), FUNC(epg3231_device::painten_w));
	map(0x002e, 0x002e).rw(FUNC(epg3231_device::paintsta_r), FUNC(epg3231_device::paintsta_w));
	map(0x002f, 0x002f).rw(FUNC(epg3231_device::pawake_r), FUNC(epg3231_device::pawake_w));
	map(0x0030, 0x0030).rw(FUNC(epg3231_device::uartcon_r), FUNC(epg3231_device::uartcon_w));
	map(0x0031, 0x0031).rw(FUNC(epg3231_device::uartsta_r), FUNC(epg3231_device::uartsta_w));
	map(0x0032, 0x0033).rw(FUNC(epg3231_device::portjk_r), FUNC(epg3231_device::portjk_w));
	map(0x0034, 0x0039).rw(FUNC(epg3231_device::dcr_r), FUNC(epg3231_device::dcr_w));
	map(0x003a, 0x003b).rw(FUNC(epg3231_device::pcon_r), FUNC(epg3231_device::pcon_w));
	// TODO: PLLF (R3Ch)
	map(0x003d, 0x003d).r(FUNC(epg3231_device::t0cl_r));
	map(0x003e, 0x003e).r(FUNC(epg3231_device::t0ch_r));
	map(0x003f, 0x003f).rw(FUNC(epg3231_device::spicon_r), FUNC(epg3231_device::spicon_w));
	map(0x0040, 0x0040).rw(FUNC(epg3231_device::spista_r), FUNC(epg3231_device::spista_w));
	map(0x0041, 0x0041).rw(FUNC(epg3231_device::sprl_r), FUNC(epg3231_device::sprl_w));
	map(0x0042, 0x0042).rw(FUNC(epg3231_device::sprm_r), FUNC(epg3231_device::sprm_w));
	map(0x0043, 0x0043).rw(FUNC(epg3231_device::sprh_r), FUNC(epg3231_device::sprh_w));
	map(0x0044, 0x0044).rw(FUNC(epg3231_device::sfcr_r), FUNC(epg3231_device::sfcr_w));
	map(0x0045, 0x0045).rw(FUNC(epg3231_device::addl_r), FUNC(epg3231_device::addl_w));
	map(0x0046, 0x0046).rw(FUNC(epg3231_device::addm_r), FUNC(epg3231_device::addm_w));
	map(0x0047, 0x0047).rw(FUNC(epg3231_device::addh_r), FUNC(epg3231_device::addh_w));
	map(0x0048, 0x0048).rw(FUNC(epg3231_device::env_sphdr_r), FUNC(epg3231_device::env_sphdr_w));
	map(0x0049, 0x0049).rw(FUNC(epg3231_device::mtcon_sphtcon_r), FUNC(epg3231_device::mtcon_sphtcon_w));
	map(0x004a, 0x004a).rw(FUNC(epg3231_device::mtrl_sphtrl_r), FUNC(epg3231_device::mtrl_sphtrl_w));
	map(0x004b, 0x004b).rw(FUNC(epg3231_device::vocon_r), FUNC(epg3231_device::vocon_w));
	map(0x004c, 0x004c).r(FUNC(epg3231_device::tr1c_r));
	map(0x004d, 0x004d).r(FUNC(epg3231_device::tr2c_r));
	// TODO: ADCF (R4Eh)
	// R4Fh is reserved
	map(0x0050, 0x007f).ram();
}

epg3231_device::epg3231_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: riscii_series_device(mconfig, EPG3231, tag, owner, clock, 22, 18, 0x40000, 5, 0x1f, 0xbb,
		address_map_constructor(FUNC(epg3231_device::regs_map), this))
{
}

device_memory_interface::space_config_vector riscii_series_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_regs_config)
	};
}

void riscii_series_device::device_resolve_objects()
{
	m_porta_in_cb.resolve_safe(0xff);
	for (auto &cb : m_port_in_cb)
		cb.resolve_safe(0xff);
	for (auto &cb : m_port_out_cb)
		cb.resolve_safe();
}

void riscii_series_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_regs = &space(AS_DATA);
	m_cache = m_program->cache<1, -1, ENDIANNESS_LITTLE>();

	if (m_pcmask > 0xffff)
		m_pchstack = make_unique_clear<u8[]>(128);

	set_icountptr(m_icount);

	state_add<u32>(RII_PC, "PC", [this]() { return m_pc; }, [this](u32 pc) { debug_set_pc(pc); }).mask(m_pcmask);
	state_add<u32>(STATE_GENPC, "GENPC", [this]() { return m_pc; }, [this](u32 pc) { debug_set_pc(pc); }).mask(m_pcmask).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_ppc).mask(m_pcmask).noshow();
	state_add(RII_REPEAT, "REPEAT", m_repeat);
	state_add(RII_ACC, "ACC", m_acc);
	state_add(RII_BSR, "BSR", m_bsr[0]).mask(m_bankmask);
	state_add(RII_FSR0, "FSR0", m_fsr[0]);
	state_add(RII_BSR1, "BSR1", m_bsr[1]).mask(m_bankmask);
	state_add<u8>(RII_FSR1, "FSR1", [this]() { return m_fsr[1]; }, [this](u8 data) { m_fsr[1] = data | 0x80; });
	state_add(RII_TABPTR, "TABPTR", m_tabptr).mask(m_tbptmask);
	state_add(RII_STKPTR, "STKPTR", m_stkptr);
	state_add(RII_CPUCON, "CPUCON", m_cpucon).mask(0x9f);
	state_add(RII_STATUS, "STATUS", m_status);
	state_add(STATE_GENFLAGS, "CURFLAGS", m_status).noshow().formatstr("%8s");
	state_add(RII_PROD, "PROD", m_prod);
	state_add(RII_PFS, "PFS", m_pfs);
	state_add(RII_INTCON, "INTCON", m_intcon);
	state_add(RII_INTSTA, "INTSTA", m_intsta);
	state_add(RII_POST_ID, "POST_ID", m_post_id).mask(m_post_id_mask);
	state_add(RII_STBCON, "STBCON", m_stbcon);
	state_add(RII_PAINTEN, "PAINTEN", m_painten);
	state_add(RII_PAINTSTA, "PAINTSTA", m_paintsta);
	state_add(RII_PAWAKE, "PAWAKE", m_pawake);
	for (int i = 0; i < 2; i++)
	{
		state_add(RII_PORTB + i, string_format("PORT%c", i + 'B').c_str(), m_port_data[i]);
		state_add(RII_DCRB + i, string_format("DCR%c", i + 'B').c_str(), m_port_dcr[i]);
		state_add(RII_PBCON + i, string_format("P%cCON", i + 'B').c_str(), m_port_control[i]);
	}
	for (int i = 2; i < 6; i++)
	{
		int p = i * 2 - 2;
		state_add(RII_PORTB + p, string_format("PORT%c", p + 'B').c_str(), m_port_data[p]);
		state_add(RII_PORTB + p + 1, string_format("PORT%c", p + 'C').c_str(), m_port_data[p + 1]);
		state_add(RII_DCRB + i, string_format("DCR%c%c", p + 'B', p + 'C').c_str(), m_port_dcr[i]);
	}
	state_add(RII_UARTCON, "UARTCON", m_uartcon);
	state_add(RII_UARTSTA, "UARTSTA", m_uartsta);
	state_add(RII_SPICON, "SPICON", m_spicon);
	state_add(RII_SPISTA, "SPISTA", m_spista).mask(0xbf);
	state_add(RII_TRL0, "TRL0", m_trl0);
	state_add(RII_TRL1, "TRL1", m_trl1);
	state_add(RII_TR01CON, "TR01CON", m_tr01con);
	state_add(RII_TRL2, "TRL2", m_trl2);
	state_add(RII_TR2CON, "TR2CON", m_tr2con);
	state_add(RII_TRLIR, "TRLIR", m_trlir);
	state_add(RII_SFCR, "SFCR", m_sfcr);
	for (int i = 0; i < 4; i++)
	{
		state_add(RII_ADD1 + i, string_format("ADD%d", i + 1).c_str(), m_add[i]).mask(0xffffff);
		state_add(RII_ENV1 + i, string_format("ENV%d", i + 1).c_str(), m_env[i]);
		state_add(RII_MTCON1 + i, string_format("MTCON%d", i + 1).c_str(), m_mtcon[i]);
		state_add(RII_MTRL1 + i, string_format("MTRL%d", i + 1).c_str(), m_mtrl[i]);
	}
	state_add(RII_SPHDR, "SPHDR", m_sphdr);
	state_add(RII_SPHTCON, "SPHTCON", m_sphtcon);
	state_add(RII_SPHTRL, "SPHTRL", m_sphtrl);
	state_add(RII_VOCON, "VOCON", m_vocon);

	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_acc));
	save_item(NAME(m_bsr));
	save_item(NAME(m_fsr));
	save_item(NAME(m_tabptr));
	save_item(NAME(m_stkptr));
	save_item(NAME(m_cpucon));
	save_item(NAME(m_status));
	if (m_pcmask > 0xffff)
		save_pointer(NAME(m_pchstack), 128);
	save_item(NAME(m_prod));
	save_item(NAME(m_pfs));
	save_item(NAME(m_intcon));
	save_item(NAME(m_intsta));
	save_item(NAME(m_post_id));
	save_item(NAME(m_port_data));
	save_item(NAME(m_port_dcr));
	save_item(NAME(m_port_control));
	save_item(NAME(m_stbcon));
	save_item(NAME(m_painten));
	save_item(NAME(m_paintsta));
	save_item(NAME(m_pawake));
	save_item(NAME(m_uartcon));
	save_item(NAME(m_uartsta));
	save_item(NAME(m_spicon));
	save_item(NAME(m_spista));
	save_item(NAME(m_trl0));
	save_item(NAME(m_trl1));
	save_item(NAME(m_trl2));
	save_item(NAME(m_tr01con));
	save_item(NAME(m_tr2con));
	save_item(NAME(m_trlir));
	save_item(NAME(m_sfcr));
	save_item(NAME(m_add));
	save_item(NAME(m_env));
	save_item(NAME(m_mtcon));
	save_item(NAME(m_mtrl));
	save_item(NAME(m_vocon));
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
	m_pfs = 0x20;
	m_intcon = 0x00;
	m_intsta = 0x00;
	m_post_id = 0xf0 & m_post_id_mask;
	m_exec_state = EXEC_CYCLE1;
	m_repeat = 0x00;

	// reset ports (input mode, no pullups)
	std::fill_n(&m_port_dcr[0], 2, 0xff);
	std::fill_n(&m_port_dcr[2], 4, 0x33);
	std::fill_n(&m_port_control[0], 2, 0x00);
	m_stbcon = 0x40;
	m_painten = 0x00;
	m_paintsta = 0x00;
	m_pawake = 0x00;

	// reset UART
	m_uartcon = 0x02;
	m_uartsta = 0x00;

	// reset SPI
	m_spicon = 0x00;
	m_spista = 0x00;

	// reset timers
	m_tr01con = 0x00;
	m_tr2con = 0x00;
	m_sfcr = 0x00;

	// reset synthesizer
	std::fill_n(&m_env[0], 4, 0x00);
	std::fill_n(&m_mtcon[0], 4, 0x00);
	std::fill_n(&m_mtrl[0], 4, 0x00);
	m_sphdr = 0x00;
	m_sphtcon = 0x00;
	m_sphtrl = 0x00;
	m_vocon = 0x07;
}

void riscii_series_device::debug_set_pc(u32 pc)
{
	m_ppc = m_pc = pc;
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
	m_pc = (m_pc & 0xf00ff) | ((u32(data) << 8) & m_pcmask);
}

u8 riscii_series_device::pch_r()
{
	return (m_pc & 0xf0000) >> 16;
}

void riscii_series_device::pch_w(u8 data)
{
	m_pc = (m_pc & 0x0ffff) | ((u32(data) << 16) & m_pcmask);
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
	m_tabptr = (m_tabptr & 0x00ffff) | ((u32(data) << 16) & m_tbptmask);
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
	// TO and PD are read-only
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

u8 riscii_series_device::pfs_r()
{
	return m_pfs;
}

void riscii_series_device::pfs_w(u8 data)
{
	m_pfs = data;
}

u8 riscii_series_device::intcon_r()
{
	return m_intcon;
}

void riscii_series_device::intcon_w(u8 data)
{
	m_intcon = data;
}

u8 riscii_series_device::intsta_r()
{
	return m_intsta;
}

void riscii_series_device::intsta_w(u8 data)
{
	m_intsta = data;
}

u8 riscii_series_device::post_id_r()
{
	return m_post_id;
}

void riscii_series_device::post_id_w(u8 data)
{
	m_post_id = data & m_post_id_mask;
}


//**************************************************************************
//  PORT REGISTER HANDLERS
//**************************************************************************

u8 riscii_series_device::porta_r()
{
	// Port A is read-only
	return m_porta_in_cb();
}

u8 riscii_series_device::port_r(offs_t offset)
{
	if (offset < 2)
	{
		u8 dc = m_port_dcr[offset];
		u8 pu = ~dc & m_port_control[offset];
		if (pu == 0xff)
			return m_port_data[offset];
		else
			return (m_port_data[offset] | dc) & (m_port_in_cb[offset](0, ~pu) | pu);
	}
	else
	{
		u8 control = (m_port_dcr[(offset >> 1) + 1] >> (BIT(offset, 0) ? 4 : 0)) & 0xf;
		if (control == 0xc)
			return m_port_data[offset];
		else
		{
			u8 dc = (BIT(control, 0) ? 0x0f : 0x00) | (BIT(control, 1) ? 0xf0 : 0x00);
			u8 pu = ~dc & ((BIT(control, 2) ? 0x0f : 0x00) | (BIT(control, 3) ? 0xf0 : 0x00));
			return (m_port_data[offset] | dc) & (m_port_in_cb[offset](0, ~pu) | pu);
		}
	}
}

void riscii_series_device::port_w(offs_t offset, u8 data)
{
	m_port_data[offset] = data;
	if (offset < 2)
	{
		u8 dc = m_port_dcr[offset];
		if (dc != 0xff)
			m_port_out_cb[offset](0, data, ~dc);
	}
	else
	{
		u8 control = (m_port_dcr[(offset >> 1) + 1] >> (BIT(offset, 0) ? 4 : 0)) & 0xf;
		u8 dc = (BIT(control, 0) ? 0x0f : 0x00) | (BIT(control, 1) ? 0xf0 : 0x00);
		if (dc != 0xff)
			m_port_out_cb[offset](0, data, ~dc);
	}
}

u8 riscii_series_device::stbcon_r()
{
	return m_stbcon;
}

void riscii_series_device::stbcon_w(u8 data)
{
	m_stbcon = data;
}

u8 riscii_series_device::painten_r()
{
	return m_painten;
}

void riscii_series_device::painten_w(u8 data)
{
	m_painten = data;
}

u8 riscii_series_device::paintsta_r()
{
	return m_paintsta;
}

void riscii_series_device::paintsta_w(u8 data)
{
	m_paintsta = data;
}

u8 riscii_series_device::pawake_r()
{
	return m_pawake;
}

void riscii_series_device::pawake_w(u8 data)
{
	m_pawake = data;
}

u8 riscii_series_device::portjk_r(offs_t offset)
{
	return port_r(offset + 8);
}

void riscii_series_device::portjk_w(offs_t offset, u8 data)
{
	port_w(offset + 8, data);
}

u8 riscii_series_device::dcr_r(offs_t offset)
{
	return m_port_dcr[offset];
}

void riscii_series_device::dcr_w(offs_t offset, u8 data)
{
	m_port_dcr[offset] = data;
}

u8 riscii_series_device::pcon_r(offs_t offset)
{
	return m_port_control[offset];
}

void riscii_series_device::pcon_w(offs_t offset, u8 data)
{
	m_port_control[offset] = data;
}


//**************************************************************************
//  UART HANDLERS
//**************************************************************************

void riscii_series_device::uarttx_w(u8 data)
{
	logerror("%05X: UARTTX = %02X\n", m_ppc, data);
}

u8 riscii_series_device::uartrx_r()
{
	return 0;
}

u8 riscii_series_device::uartcon_r()
{
	// TB8 is write-only
	return m_uartcon & 0x7f;
}

void riscii_series_device::uartcon_w(u8 data)
{
	// UTBE is read-only
	m_uartcon = (data & 0xfd) | (m_uartcon & 0x02);
}

u8 riscii_series_device::uartsta_r()
{
	return m_uartsta;
}

void riscii_series_device::uartsta_w(u8 data)
{
	// RB8 and URBF are read-only
	m_uartsta = (data & 0x7d) | (m_uartsta & 0x82);
}


//**************************************************************************
//  SPI HANDLERS
//**************************************************************************

u8 riscii_series_device::spicon_r()
{
	return m_spicon;
}

void riscii_series_device::spicon_w(u8 data)
{
	m_spicon = data;
}

u8 riscii_series_device::spista_r()
{
	return m_spista;
}

void riscii_series_device::spista_w(u8 data)
{
	// RBF is read-only
	m_spista = (data & 0xbe) | (m_spista & 0x01);
}

u8 riscii_series_device::sprl_r()
{
	return 0;
}

void riscii_series_device::sprl_w(u8 data)
{
	logerror("%05X: SPRL = %02X\n", m_ppc, data);
}

u8 riscii_series_device::sprm_r()
{
	return 0;
}

void riscii_series_device::sprm_w(u8 data)
{
	logerror("%05X: SPRM = %02X\n", m_ppc, data);
}

u8 riscii_series_device::sprh_r()
{
	return 0;
}

void riscii_series_device::sprh_w(u8 data)
{
	logerror("%05X: SPRH = %02X\n", m_ppc, data);
}


//**************************************************************************
//  TIMER HANDLERS
//**************************************************************************

u8 riscii_series_device::trl0l_r()
{
	return m_trl0 & 0x00ff;
}

void riscii_series_device::trl0l_w(u8 data)
{
	m_trl0 = (m_trl0 & 0xff00) | data;
}

u8 riscii_series_device::trl0h_r()
{
	return (m_trl0 & 0xff00) >> 8;
}

void riscii_series_device::trl0h_w(u8 data)
{
	m_trl0 = u8(data) << 8 | (m_trl0 & 0x00ff);
}

u8 riscii_series_device::trl1_r()
{
	return m_trl1;
}

void riscii_series_device::trl1_w(u8 data)
{
	m_trl1 = data;
}

u8 riscii_series_device::trl2_r()
{
	return m_trl2;
}

void riscii_series_device::trl2_w(u8 data)
{
	m_trl2 = data;
}

u8 riscii_series_device::tr01con_r()
{
	return m_tr01con;
}

void riscii_series_device::tr01con_w(u8 data)
{
	m_tr01con = data;
}

u8 riscii_series_device::tr2con_r()
{
	return m_tr2con;
}

void riscii_series_device::tr2con_w(u8 data)
{
	m_tr2con = data;
}

u8 riscii_series_device::trlir_r()
{
	return m_trlir;
}

void riscii_series_device::trlir_w(u8 data)
{
	m_trlir = data;
}

u8 riscii_series_device::t0cl_r()
{
	return 0x00;
}

u8 riscii_series_device::t0ch_r()
{
	return 0x00;
}

u8 riscii_series_device::tr1c_r()
{
	return 0xff;
}

u8 riscii_series_device::tr2c_r()
{
	return 0xff;
}

u8 riscii_series_device::sfcr_r()
{
	return m_sfcr;
}

void riscii_series_device::sfcr_w(u8 data)
{
	m_sfcr = data;
}


//**************************************************************************
//  MUSIC/SPEECH SYNTHESIZER
//**************************************************************************

u8 riscii_series_device::addl_r()
{
	return m_add[BIT(m_sfcr, 2) ? 3 : m_sfcr & 0x03] & 0x0000ff;
}

void riscii_series_device::addl_w(u8 data)
{
	u32 &add = m_add[BIT(m_sfcr, 2) ? 3 : m_sfcr & 0x03];
	add = (add & 0xffff00) | data;
}

u8 riscii_series_device::addm_r()
{
	return (m_add[BIT(m_sfcr, 2) ? 3 : m_sfcr & 0x03] & 0x00ff00) >> 8;
}

void riscii_series_device::addm_w(u8 data)
{
	u32 &add = m_add[BIT(m_sfcr, 2) ? 3 : m_sfcr & 0x03];
	add = (add & 0xff00ff) | u32(data) << 8;
}

u8 riscii_series_device::addh_r()
{
	return (m_add[BIT(m_sfcr, 2) ? 3 : m_sfcr & 0x03] & 0xff0000) >> 8;
}

void riscii_series_device::addh_w(u8 data)
{
	u32 &add = m_add[BIT(m_sfcr, 2) ? 3 : m_sfcr & 0x03];
	add = (add & 0x00ffff) | u32(data) << 16;
}

u8 riscii_series_device::env_sphdr_r()
{
	if (BIT(m_sfcr, 2))
		return m_sphdr;
	else
		return m_env[m_sfcr & 0x03];
}

void riscii_series_device::env_sphdr_w(u8 data)
{
	if (BIT(m_sfcr, 2))
		m_sphdr = data;
	else
		m_env[m_sfcr & 0x03] = data;
}

u8 riscii_series_device::mtcon_sphtcon_r()
{
	if (BIT(m_sfcr, 2))
		return m_sphtcon;
	else
		return m_mtcon[m_sfcr & 0x03];
}

void riscii_series_device::mtcon_sphtcon_w(u8 data)
{
	if (BIT(m_sfcr, 2))
		m_sphtcon = data;
	else
		m_mtcon[m_sfcr & 0x03] = data;
}

u8 riscii_series_device::mtrl_sphtrl_r()
{
	if (BIT(m_sfcr, 2))
		return m_sphtrl;
	else
		return m_mtrl[m_sfcr & 0x03];
}

void riscii_series_device::mtrl_sphtrl_w(u8 data)
{
	if (BIT(m_sfcr, 2))
		m_sphtrl = data;
	else
		m_mtrl[m_sfcr & 0x03] = data;
}

u8 riscii_series_device::vocon_r()
{
	return m_vocon;
}

void riscii_series_device::vocon_w(u8 data)
{
	m_vocon = data;
}


//**************************************************************************
//  MEMORY HELPERS
//**************************************************************************

u16 riscii_series_device::fetch_program_word()
{
	return m_cache->read_word(std::exchange(m_pc, (m_pc + 1) & m_pcmask));
}

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
					m_bsr[1] = (m_bsr[1] + 1) & m_bankmask;
			}
			else
			{
				m_fsr[1] = (m_fsr[1] - 1) | 0x80;
				if (m_fsr[1] == 0xff)
					m_bsr[1] = (m_bsr[1] - 1) & m_bankmask;
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
	return (m_tabptr + offset) & m_tbptmask;
}

void riscii_series_device::multi_byte_carry(u16 addr, bool cy)
{
	if (addr == 0x0002)
		m_exec_state = EXEC_ADCPCM;
	else if (addr == 0x0009 && cy)
	{
		// FSR1 can carry into BSR1
		bsr1_w(bsr1_r() + 1);
	}
	else if (addr == 0x000b && cy)
	{
		// TABPTRL can carry into TABPTRM and TABPTRH
		u8 midbyte = tabptrm_r() + 1;
		tabptrm_w(midbyte);
		if (midbyte == 0)
			tabptrh_w(tabptrh_r() + 1);
	}
	else if (addr == 0x001c && cy)
	{
		// PORTF can carry into PORTE and PORTD
		u8 midbyte = port_r(3);
		port_w(3, midbyte);
		if (midbyte == 0)
			port_w(2, port_r(2) + 1);
	}
}

void riscii_series_device::multi_byte_borrow(u16 addr, bool cy)
{
	if (addr == 0x0002)
		m_exec_state = EXEC_SBCPCM;
	else if (addr == 0x0009 && !cy)
	{
		// FSR1 can borrow from BSR1
		bsr1_w(bsr1_r() - 1);
	}
	else if (addr == 0x000b && !cy)
	{
		// TABPTRL can borrow from TABPTRM and TABPTRH
		u8 midbyte = tabptrm_r() - 1;
		tabptrm_w(midbyte);
		if (midbyte == 0xff)
			tabptrh_w(tabptrh_r() - 1);
	}
	else if (addr == 0x001c && !cy)
	{
		// PORTF can borrow from PORTE and PORTD
		u8 midbyte = port_r(3) - 1;
		port_w(3, midbyte);
		if (midbyte == 0xff)
			port_w(2, port_r(2) + 1);
	}
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
	bool cy = u16(data) + m_acc + (c ? m_status & 0x01 : 0) >= 0x100;
	bool dc = (data & 0x0f) + (m_acc & 0x0f) + (c ? m_status & 0x01 : 0) >= 0x10;
	if (a)
		acc_w(tmp & 0xff);
	else
	{
		m_regs->write_byte(addr, tmp & 0xff);
		multi_byte_carry(addr, cy);
	}
	m_status = (m_status & 0xc0)
		| (cy ? 0x01 : 0x00)
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
	bool cy = u8(data) >= m_acc + (c ? m_status & 0x01 : 0); // borrow is inverted
	bool dc = (data & 0x0f) >= (m_acc & 0x0f) + (c ? ~m_status & 0x01 : 0);
	if (a)
		acc_w(tmp & 0xff);
	else
	{
		m_regs->write_byte(addr, tmp & 0xff);
		multi_byte_borrow(addr, cy);
	}
	m_status = (m_status & 0xc0)
		| (cy ? 0x01 : 0x00)
		| (dc ? 0x02 : 0x00)
		| ((tmp & 0xff) == 0 ? 0x04 : 0x00)
		| ((tmp < 0) != BIT(tmp, 7) ? 0x08 : 0x00)
		| (tmp <= 0 ? 0x10 : 0x00)
		| (tmp >= 0 ? 0x20 : 0x00);
}

void riscii_series_device::execute_add_imm(u8 data, bool c)
{
	s16 tmp = s16(s8(data)) + s8(m_acc) + (c ? m_status & 0x01 : 0);
	bool cy = u16(data) + m_acc + (c ? m_status & 0x01 : 0) >= 0x100;
	bool dc = (data & 0x0f) + (m_acc & 0x0f) + (c ? m_status & 0x01 : 0) >= 0x10;
	acc_w(tmp & 0xff);
	m_status = (m_status & 0xc0)
		| (cy ? 0x01 : 0x00)
		| (dc ? 0x02 : 0x00)
		| ((tmp & 0xff) == 0 ? 0x04 : 0x00)
		| ((tmp < 0) != BIT(tmp, 7) ? 0x08 : 0x00)
		| (tmp <= 0 ? 0x10 : 0x00)
		| (tmp >= 0 ? 0x20 : 0x00);
}

void riscii_series_device::execute_sub_imm(u8 data, bool c)
{
	s16 tmp = s16(s8(data)) - s8(m_acc) - (c ? ~m_status & 0x01 : 0);
	bool cy = u8(data) >= m_acc + (c ? m_status & 0x01 : 0); // borrow is inverted
	bool dc = (data & 0x0f) + (m_acc & 0x0f) + (c ? ~m_status & 0x01 : 0) >= 0x10;
	acc_w(tmp & 0xff);
	m_status = (m_status & 0xc0)
		| (cy ? 0x01 : 0x00)
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
	execute_mul_imm(m_regs->read_byte(get_banked_address(reg)));
}

void riscii_series_device::execute_mul_imm(u8 data)
{
	int mier = BIT(m_cpucon, 3) ? int(s8(m_acc)) : int(m_acc);
	int mcand = BIT(m_cpucon, 4) ? int(s8(data)) : int(data);
	m_prod = u16(mier * mcand);
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
	// Push PC to the stack region at the end of banked RAM
	m_stkptr -= 2;
	u16 stkaddr = u16(m_maxbank - (BIT(m_stkptr, 7) ? 0 : 1)) << 8 | 0x80 | (m_stkptr & 0x7e);
	m_regs->write_word(stkaddr, swapendian_int16(m_pc & 0xffff));

	// PCH (on relevant models) must be saved somewhere. This implementation assumes a private buffer is used.
	if (m_pcmask > 0xffff)
		m_pchstack[m_stkptr >> 1] = (m_pc & 0xff0000) >> 16;

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
	{
		m_regs->write_byte(addr, tmp);
		multi_byte_borrow(addr, tmp != 0xff);
	}
	execute_jcc(tmp != 0);
}

void riscii_series_device::execute_jinz(u8 reg, bool a)
{
	u16 addr = get_banked_address(reg);
	u8 tmp = m_regs->read_byte(addr) + 1;
	if (a)
		acc_w(tmp);
	else
	{
		m_regs->write_byte(addr, tmp);
		multi_byte_carry(addr, tmp == 0);
	}
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
	{
		m_regs->write_byte(addr, tmp & 0xff);
		multi_byte_carry(addr, (tmp >> 8) != 0);
	}
	m_status = (m_status & 0xfa) | ((tmp & 0xff) == 0 ? 0x04 : 0x00) | (tmp >> 8);
}

void riscii_series_device::execute_dec(u8 reg, bool a)
{
	u16 addr = get_banked_address(reg);
	u16 tmp = (u16(m_regs->read_byte(addr)) - 1) & 0x1ff;
	if (a)
		acc_w(tmp & 0xff);
	else
	{
		m_regs->write_byte(addr, tmp & 0xff);
		multi_byte_borrow(addr, (tmp >> 8) != 0);
	}
	m_status = (m_status & 0xfa) | ((tmp & 0xff) == 0 ? 0x04 : 0x00) | (tmp >> 8);
}

void riscii_series_device::execute_rpt(u8 reg)
{
	m_repeat = m_regs->read_byte(get_banked_address(reg)) - 1;
}

void riscii_series_device::execute_ret(bool inte)
{
	// Pop PC from the stack region at the end of banked RAM
	u16 stkaddr = u16(m_maxbank - (BIT(m_stkptr, 7) ? 0 : 1)) << 8 | 0x80 | (m_stkptr & 0x7e);
	u32 dest = swapendian_int16(m_regs->read_word(stkaddr));
	if (m_pcmask > 0xffff)
		dest |= u32(m_pchstack[m_stkptr >> 1]) << 16;
	execute_jump(dest);
	m_stkptr += 2;

	// RETI also sets the global interrupt enable flag
	if (inte)
		m_cpucon |= 0x04;
}

void riscii_series_device::execute_wdtc()
{
	logerror("Watchdog timer reset (PC = %05X)\n", m_ppc);
}

void riscii_series_device::execute_slep()
{
	logerror("%s mode entered (PC = %05X)\n", BIT(m_cpucon, 1) ? "Idle" : "Sleep", m_ppc);
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
	u16 addr = get_banked_address(m_curreg);
	u32 memaddr = (ptr & 0x7ffffe) >> 1;
	if (BIT(ptr, 23))
		memaddr += m_datastart;
	else
		memaddr &= m_pcmask;
	u16 data = m_program->read_word(memaddr);
	if (BIT(ptr, 0))
		m_regs->write_byte(addr, data >> 8);
	else
		m_regs->write_byte(addr, data);
	if (m_repeat != 0)
		--m_repeat;
	else
		m_exec_state = EXEC_CYCLE1;
}

bool riscii_series_device::interrupt_active() const
{
	if (!BIT(m_cpucon, 2))
		return false;
	else
		return (m_intcon & m_intsta) != 0
			|| (m_painten & m_paintsta) != 0
			|| (m_spista & 0x30) == 0x30
			|| (m_sphtcon & 0x30) == 0x30;
}

u32 riscii_series_device::vector_interrupt() const
{
	// Input port A
	if ((m_painten & m_paintsta) != 0)
		return 0x00002;

	// Capture
	if ((m_intcon & m_intsta & 0x80) != 0)
		return 0x00004;

	// Speech timer
	if ((m_sphtcon & 0x30) == 0x30)
		return 0x00006;

	// Timers 0–2
	if ((m_intcon & m_intsta & 0x07) != 0)
		return 0x00008;

	// Peripheral
	if ((m_intcon & m_intsta & 0x78) != 0 || (m_spista & 0x30) == 0x30)
		return 0x0000a;

	// Should not reach here
	return 0x00000;
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
				execute_cycle1(fetch_program_word());
				if (m_exec_state == EXEC_CYCLE1)
				{
					--m_repeat;
					m_pc = m_ppc;
				}
			}
			else if (interrupt_active())
			{
				// Disable interrupts and call handler
				m_cpucon &= 0xfb;
				execute_call(vector_interrupt());
			}
			else
				execute_cycle1(fetch_program_word());
			break;

		case EXEC_ADCPCM:
			if (BIT(m_status, 0))
			{
				u8 pcm = pcm_r() + 1;
				pcm_w(pcm);
				if (pcm == 0x00 && m_pcmask > 0xffff)
					pch_w(pch_r() + 1);
			}
			m_exec_state = EXEC_CYCLE1;
			break;

		case EXEC_SBCPCM:
			if (!BIT(m_status, 0))
			{
				u8 pcm = pcm_r() - 1;
				pcm_w(pcm);
				if (pcm == 0xff && m_pcmask > 0xffff)
					pch_w(pch_r() - 1);
			}
			m_exec_state = EXEC_CYCLE1;
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
			execute_jump(u32(m_exec_state - EXEC_L0JMP) << 16 | fetch_program_word());
			m_exec_state = EXEC_CYCLE1;
			break;

		case EXEC_L0CALL: case EXEC_L1CALL: case EXEC_L2CALL: case EXEC_L3CALL:
		case EXEC_L4CALL: case EXEC_L5CALL: case EXEC_L6CALL: case EXEC_L7CALL:
		case EXEC_L8CALL: case EXEC_L9CALL: case EXEC_LACALL: case EXEC_LBCALL:
		case EXEC_LCCALL: case EXEC_LDCALL: case EXEC_LECALL: case EXEC_LFCALL:
			execute_call(u32(m_exec_state - EXEC_L0CALL) << 16 | fetch_program_word());
			m_exec_state = EXEC_CYCLE1;
			break;

		case EXEC_NOJMP:
			(void)fetch_program_word();
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

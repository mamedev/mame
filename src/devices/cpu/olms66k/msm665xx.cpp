// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Oki MSM665xx 16-bit microcontroller family (nX-8/500S core)

    Currently this is just one stub device with no actual execution core.

****************************************************************************/

#include "emu.h"
#include "msm665xx.h"
#include "nx8dasm.h"

// device type definition
DEFINE_DEVICE_TYPE(MSM66573, msm66573_device, "msm66573", "Oki MSM66573")


msm665xx_device::msm665xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor mem_map, address_map_constructor data_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 20, 0, mem_map)
	, m_data_config("iram", ENDIANNESS_LITTLE, 16, 20, 0, data_map)
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
	state_add(MSM665XX_PC, "PC", m_pc);
	state_add<u32>(STATE_GENPC, "GENPC",
		[this]() { return u32(m_csr) << 16 | m_pc; },
		[this](u32 data) { m_csr = (data >> 16) & 0x0f; m_pc = data & 0xffff; m_ppc = data & 0xfffff; }
	).mask(0xfffff).noshow();
	state_add<u32>(STATE_GENPCBASE, "GENPCBASE", m_ppc,
		[this](u32 data) { m_csr = (data >> 16) & 0x0f; m_pc = data & 0xffff; m_ppc = data & 0xfffff; }
	).mask(0xfffff).noshow();
	state_add(MSM665XX_PSW, "PSW", m_psw);
	//state_add(STATE_GENFLAGS, "FLAGS", m_psw).formatstr("%9s").noshow(); // TODO
	state_add(MSM665XX_LRB, "LRB", m_lrb);
	state_add(MSM665XX_SSP, "SSP", m_ssp);
	u16 *fixed = static_cast<u16 *>(memshare("internal")->ptr());
	for (int n = 0; n < 4; n++)
		state_add<u16>(MSM665XX_PR0 + n, std::array<const char *, 4>{{"X1", "X2", "DP", "USP"}}[n],
			[this, fixed, n]() { return fixed[(m_psw & 0x07) << 2 | n]; },
			[this, fixed, n](u16 data) { fixed[(m_psw & 0x07) << 2 | n] = data; }
		);
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
	state_add(MSM665XX_CSR, "CSR", m_csr).mask(0x0f);
	state_add(MSM665XX_DSR, "DSR", m_dsr).mask(0x0f);
	state_add(MSM665XX_TSR, "TSR", m_tsr).mask(0x0f);
	state_add(MSM665XX_ROMWIN, "ROMWIN", m_romwin);

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
}

void msm665xx_device::device_reset()
{
	// PC will be initialized from vector following reset; LRB and ROMWIN are undefined at reset
	m_acc = 0;
	m_psw = 0;
	m_ssp = 0xffff;
	m_csr = 0;
	m_dsr = 0;
	m_tsr = 0;
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
	m_dsr = data & 0x0f;
}

u8 msm665xx_device::tsr_r()
{
	return m_tsr;
}

void msm665xx_device::tsr_w(u8 data)
{
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


void msm665xx_device::execute_run()
{
	m_pc = m_program_cache.read_word(0);
	m_ppc = m_pc;
	debugger_instruction_hook(m_pc);

	// TODO
	m_icount = 0;
}


void msm665xx_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		// TODO
		break;
	}
}

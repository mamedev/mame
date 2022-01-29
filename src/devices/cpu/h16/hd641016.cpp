// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Hitachi HD641016 16-bit MPU

    Currently this device is just a stub with no actual execution core.

***************************************************************************/

#include "emu.h"
#include "hd641016.h"
#include "h16dasm.h"

// device type definition
DEFINE_DEVICE_TYPE(HD641016, hd641016_device, "hd641016", "Hitachi HD641016")

hd641016_device::hd641016_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, HD641016, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 16, 24, 0)
	, m_data_config("data", ENDIANNESS_BIG, 32, 10, 0, address_map_constructor(FUNC(hd641016_device::ram_map), this))
	, m_io_config("io", ENDIANNESS_BIG, 32, 9, 0, address_map_constructor(FUNC(hd641016_device::io_map), this))
	, m_pc(0)
	, m_ssp(0)
	, m_bsp(0)
	, m_ebr(0)
	, m_rbr(0)
	, m_ibr(0)
	, m_cbnr(0)
	, m_sr(0)
	, m_bmr(0)
	, m_gbnr(0)
	, m_vbnr(0)
	, m_icount(0)
{
}

void hd641016_device::ram_map(address_map &map)
{
	map(0x000, 0x3ff).ram().share("ram"); // 1K on-chip high-speed RAM
}

void hd641016_device::io_map(address_map &map)
{
	map.unmap_value_low();
	//map(0x128, 0x129).rw(FUNC(hd641016_device::abr0_r), FUNC(hd641016_device::abr0_w));
	//map(0x12a, 0x12b).rw(FUNC(hd641016_device::arr0_r), FUNC(hd641016_device::arr0_w));
	//map(0x12c, 0x12d).rw(FUNC(hd641016_device::awcr0_r), FUNC(hd641016_device::awcr0_w));
	//map(0x12e, 0x12f).rw(FUNC(hd641016_device::abr1_r), FUNC(hd641016_device::abr1_w));
	//map(0x130, 0x131).rw(FUNC(hd641016_device::arr1_r), FUNC(hd641016_device::arr1_w));
	//map(0x132, 0x133).rw(FUNC(hd641016_device::awcr1_r), FUNC(hd641016_device::awcr1_w));
	//map(0x134, 0x135).rw(FUNC(hd641016_device::abr2_r), FUNC(hd641016_device::abr2_w));
	//map(0x136, 0x137).rw(FUNC(hd641016_device::arr2_r), FUNC(hd641016_device::arr2_w));
	//map(0x138, 0x139).rw(FUNC(hd641016_device::awcr2_r), FUNC(hd641016_device::awcr2_w));
	//map(0x13a, 0x13b).rw(FUNC(hd641016_device::abr3_r), FUNC(hd641016_device::abr3_w));
	//map(0x13c, 0x13d).rw(FUNC(hd641016_device::arr3_r), FUNC(hd641016_device::arr3_w));
	//map(0x13e, 0x13f).rw(FUNC(hd641016_device::awcr3_r), FUNC(hd641016_device::awcr3_w));
	//map(0x140, 0x141).rw(FUNC(hd641016_device::ipr0_r), FUNC(hd641016_device::ipr0_w));
	//map(0x142, 0x143).rw(FUNC(hd641016_device::ipr1_r), FUNC(hd641016_device::ipr1_w));
	//map(0x144, 0x145).rw(FUNC(hd641016_device::ipr2_r), FUNC(hd641016_device::ipr2_w));
	//map(0x146, 0x147).rw(FUNC(hd641016_device::icr_r), FUNC(hd641016_device::icr_w));
	//map(0x14e, 0x14f).rw(FUNC(hd641016_device::pcr0_r), FUNC(hd641016_device::pcr0_w));
	//map(0x158, 0x158).rw(FUNC(hd641016_device::asci0_rxb_r), FUNC(hd641016_device::asci0_txb_w));
	//map(0x159, 0x159).r(FUNC(hd641016_device::asci0_st0_r));
	//map(0x15a, 0x15a).rw(FUNC(hd641016_device::asci0_st1_r), FUNC(hd641016_device::asci0_st1_w));
	//map(0x15b, 0x15b).rw(FUNC(hd641016_device::asci0_st2_r), FUNC(hd641016_device::asci0_st2_w));
	//map(0x15e, 0x15e).rw(FUNC(hd641016_device::asci0_ie0_r), FUNC(hd641016_device::asci0_ie0_w));
	//map(0x15f, 0x15f).rw(FUNC(hd641016_device::asci0_ie1_r), FUNC(hd641016_device::asci0_ie1_w));
	//map(0x160, 0x160).rw(FUNC(hd641016_device::asci0_ie2_r), FUNC(hd641016_device::asci0_ie2_w));
	//map(0x162, 0x162).w(FUNC(hd641016_device::asci0_cmd_w));
	//map(0x163, 0x163).rw(FUNC(hd641016_device::asci0_md0_r), FUNC(hd641016_device::asci0_md0_w));
	//map(0x164, 0x164).rw(FUNC(hd641016_device::asci0_md1_r), FUNC(hd641016_device::asci0_md1_w));
	//map(0x165, 0x165).rw(FUNC(hd641016_device::asci0_md2_r), FUNC(hd641016_device::asci0_md2_w));
	//map(0x166, 0x166).rw(FUNC(hd641016_device::asci0_ctl_r), FUNC(hd641016_device::asci0_ctl_w));
	//map(0x16a, 0x16a).rw(FUNC(hd641016_device::asci0_tmc_r), FUNC(hd641016_device::asci0_tmc_w));
	//map(0x16b, 0x16b).rw(FUNC(hd641016_device::asci0_rxs_r), FUNC(hd641016_device::asci0_rxs_w));
	//map(0x16c, 0x16c).rw(FUNC(hd641016_device::asci0_txs_r), FUNC(hd641016_device::asci0_txs_w));
	//map(0x170, 0x170).rw(FUNC(hd641016_device::asci1_rxb_r), FUNC(hd641016_device::asci1_txb_w));
	//map(0x171, 0x171).r(FUNC(hd641016_device::asci1_st0_r));
	//map(0x172, 0x172).rw(FUNC(hd641016_device::asci1_st1_r), FUNC(hd641016_device::asci1_st1_w));
	//map(0x173, 0x173).rw(FUNC(hd641016_device::asci1_st2_r), FUNC(hd641016_device::asci1_st2_w));
	//map(0x176, 0x176).rw(FUNC(hd641016_device::asci1_ie0_r), FUNC(hd641016_device::asci1_ie0_w));
	//map(0x177, 0x177).rw(FUNC(hd641016_device::asci1_ie1_r), FUNC(hd641016_device::asci1_ie1_w));
	//map(0x178, 0x178).rw(FUNC(hd641016_device::asci1_ie2_r), FUNC(hd641016_device::asci1_ie2_w));
	//map(0x17a, 0x17a).w(FUNC(hd641016_device::asci1_cmd_w));
	//map(0x17b, 0x17b).rw(FUNC(hd641016_device::asci1_md0_r), FUNC(hd641016_device::asci1_md0_w));
	//map(0x17c, 0x17c).rw(FUNC(hd641016_device::asci1_md1_r), FUNC(hd641016_device::asci1_md1_w));
	//map(0x17d, 0x17d).rw(FUNC(hd641016_device::asci1_md2_r), FUNC(hd641016_device::asci1_md2_w));
	//map(0x17e, 0x17e).rw(FUNC(hd641016_device::asci1_ctl_r), FUNC(hd641016_device::asci1_ctl_w));
	//map(0x182, 0x182).rw(FUNC(hd641016_device::asci1_tmc_r), FUNC(hd641016_device::asci1_tmc_w));
	//map(0x183, 0x183).rw(FUNC(hd641016_device::asci1_rxs_r), FUNC(hd641016_device::asci1_rxs_w));
	//map(0x184, 0x184).rw(FUNC(hd641016_device::asci1_txs_r), FUNC(hd641016_device::asci1_txs_w));
	//map(0x18e, 0x18f).rw(FUNC(hd641016_device::tmr1_ucr_r), FUNC(hd641016_device::tmr1_ucr_w));
	//map(0x190, 0x191).rw(FUNC(hd641016_device::tmr1_ccra_r), FUNC(hd641016_device::tmr1_ccra_w));
	//map(0x192, 0x193).rw(FUNC(hd641016_device::tmr1_ccrb_r), FUNC(hd641016_device::tmr1_ccrb_w));
	//map(0x194, 0x195).rw(FUNC(hd641016_device::tmr1_cntr_r), FUNC(hd641016_device::tmr1_cntr_w));
	//map(0x196, 0x197).r(FUNC(hd641016_device::tmr1_str_r));
	//map(0x198, 0x189).rw(FUNC(hd641016_device::tmr2_ucr_r), FUNC(hd641016_device::tmr2_ucr_w));
	//map(0x19a, 0x19b).rw(FUNC(hd641016_device::tmr2_ccra_r), FUNC(hd641016_device::tmr2_ccra_w));
	//map(0x19c, 0x19d).rw(FUNC(hd641016_device::tmr2_ccrb_r), FUNC(hd641016_device::tmr2_ccrb_w));
	//map(0x19e, 0x19f).rw(FUNC(hd641016_device::tmr2_cntr_r), FUNC(hd641016_device::tmr2_cntr_w));
	//map(0x1a0, 0x1a1).r(FUNC(hd641016_device::tmr2_str_r));
	//map(0x1b0, 0x1b3).rw(FUNC(hd641016_device::dmac0_madr_r), FUNC(hd641016_device::dmac0_madr_w));
	//map(0x1b4, 0x1b7).rw(FUNC(hd641016_device::dmac0_dadr_r), FUNC(hd641016_device::dmac0_dadr_w));
	//map(0x1b8, 0x1b9).rw(FUNC(hd641016_device::dmac0_etcr_r), FUNC(hd641016_device::dmac0_etcr_w));
	//map(0x1ba, 0x1bb).rw(FUNC(hd641016_device::dmac0_btcr_r), FUNC(hd641016_device::dmac0_btcr_w));
	//map(0x1bc, 0x1bd).rw(FUNC(hd641016_device::dmac0_chcra_r), FUNC(hd641016_device::dmac0_chcra_w));
	//map(0x1be, 0x1bf).rw(FUNC(hd641016_device::dmac0_chcrb_r), FUNC(hd641016_device::dmac0_chcrb_w));
	//map(0x1c0, 0x1c3).rw(FUNC(hd641016_device::dmac1_madr_r), FUNC(hd641016_device::dmac1_madr_w));
	//map(0x1c4, 0x1c7).rw(FUNC(hd641016_device::dmac1_dadr_r), FUNC(hd641016_device::dmac1_dadr_w));
	//map(0x1c8, 0x1c9).rw(FUNC(hd641016_device::dmac1_etcr_r), FUNC(hd641016_device::dmac1_etcr_w));
	//map(0x1ca, 0x1cb).rw(FUNC(hd641016_device::dmac1_btcr_r), FUNC(hd641016_device::dmac1_btcr_w));
	//map(0x1cc, 0x1cd).rw(FUNC(hd641016_device::dmac1_chcra_r), FUNC(hd641016_device::dmac1_chcra_w));
	//map(0x1ce, 0x1cf).rw(FUNC(hd641016_device::dmac1_chcrb_r), FUNC(hd641016_device::dmac1_chcrb_w));
	//map(0x1d0, 0x1d3).rw(FUNC(hd641016_device::dmac2_madr_r), FUNC(hd641016_device::dmac2_madr_w));
	//map(0x1d4, 0x1d7).rw(FUNC(hd641016_device::dmac2_dadr_r), FUNC(hd641016_device::dmac2_dadr_w));
	//map(0x1d8, 0x1d9).rw(FUNC(hd641016_device::dmac2_etcr_r), FUNC(hd641016_device::dmac2_etcr_w));
	//map(0x1da, 0x1db).rw(FUNC(hd641016_device::dmac2_btcr_r), FUNC(hd641016_device::dmac2_btcr_w));
	//map(0x1dc, 0x1dd).rw(FUNC(hd641016_device::dmac2_chcra_r), FUNC(hd641016_device::dmac2_chcra_w));
	//map(0x1de, 0x1df).rw(FUNC(hd641016_device::dmac2_chcrb_r), FUNC(hd641016_device::dmac2_chcrb_w));
	//map(0x1e0, 0x1e3).rw(FUNC(hd641016_device::dmac3_madr_r), FUNC(hd641016_device::dmac3_madr_w));
	//map(0x1e4, 0x1e7).rw(FUNC(hd641016_device::dmac3_dadr_r), FUNC(hd641016_device::dmac3_dadr_w));
	//map(0x1e8, 0x1e9).rw(FUNC(hd641016_device::dmac3_etcr_r), FUNC(hd641016_device::dmac3_etcr_w));
	//map(0x1ea, 0x1eb).rw(FUNC(hd641016_device::dmac3_btcr_r), FUNC(hd641016_device::dmac3_btcr_w));
	//map(0x1ec, 0x1ed).rw(FUNC(hd641016_device::dmac3_chcra_r), FUNC(hd641016_device::dmac3_chcra_w));
	//map(0x1ee, 0x1ef).rw(FUNC(hd641016_device::dmac3_chcrb_r), FUNC(hd641016_device::dmac3_chcrb_w));
	//map(0x1f0, 0x1f1).rw(FUNC(hd641016_device::dmac_opcr_r), FUNC(hd641016_device::dmac_opcr_w));
	//map(0x1f8, 0x1f9).rw(FUNC(hd641016_device::mcr_r), FUNC(hd641016_device::mcr_w));
}

device_memory_interface::space_config_vector hd641016_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_data_config),
		std::make_pair(AS_IO, &m_io_config)
	};
}

std::unique_ptr<util::disasm_interface> hd641016_device::create_disassembler()
{
	return std::make_unique<h16_disassembler>();
}

void hd641016_device::device_start()
{
	space(AS_PROGRAM).specific(m_program);
	space(AS_PROGRAM).cache(m_cache);
	space(AS_DATA).cache(m_data);
	space(AS_IO).specific(m_io);

	set_icountptr(m_icount);

	// register state
	state_add(H16_PC, "PC", m_pc).mask(0xffffff);
	state_add(STATE_GENPC, "GENPC", m_pc).mask(0xffffff).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).mask(0xffffff).noshow();
	state_add(H16_SSP, "SSP", m_ssp);
	state_add(H16_BSP, "BSP", m_bsp);
	state_add(H16_EBR, "EBR", m_ebr);
	state_add(H16_RBR, "RBR", m_rbr);
	state_add(H16_IBR, "IBR", m_ibr);
	state_add(H16_CBNR, "CBNR", m_cbnr);
	state_add(H16_SR, "SR", m_sr).mask(0xb71f);
	state_add<u16>(H16_CCR, "CCR",
		[this]() { return m_sr & 0x001f; },
		[this](u16 data) { m_sr = (m_sr & 0xb700) | data; }
	).mask(0x001f).noshow();
	state_add(STATE_GENFLAGS, "FLAGS", m_sr).mask(0xb71f).formatstr("%14s").noshow();
	state_add(H16_BMR, "BMR", m_bmr);
	state_add(H16_GBNR, "GBNR", m_gbnr);
	state_add(H16_VBNR, "VBNR", m_vbnr);
	u32 *const internal_ram = static_cast<u32 *>(memshare("ram")->ptr());
	for (int n = 0; n < 16; n++)
	{
		std::string rname = n == 15 ? "USP" : string_format("R%d", n);
		state_add<u32>(H16_R0 + n, rname.c_str(),
			[this, n, internal_ram]() { return internal_ram[0xf0 - ((m_gbnr & global_bank_mask()) << 4) + n]; },
			[this, n, internal_ram](u32 data) { internal_ram[0xf0 - ((m_gbnr & global_bank_mask()) << 4) + n] = data; }
		);
	}

	// save state
	save_item(NAME(m_pc));
	save_item(NAME(m_ssp));
	save_item(NAME(m_bsp));
	save_item(NAME(m_ebr));
	save_item(NAME(m_rbr));
	save_item(NAME(m_ibr));
	save_item(NAME(m_cbnr));
	save_item(NAME(m_sr));
	save_item(NAME(m_bmr));
	save_item(NAME(m_gbnr));
	save_item(NAME(m_vbnr));
}

void hd641016_device::device_reset()
{
	m_ebr &= 0xff000000;
	m_rbr = 0x00fff800 | (m_rbr & 0xff0003ff);
	m_ibr = 0x00ff0000 | (m_ibr & 0xff00ffff);
	m_cbnr = 0x00000000;
	m_sr = 0x2700 | (m_sr & 0x001f);
	m_bmr = 0xa4;
	m_gbnr &= 0xf0;
	m_vbnr &= 0xf7;
}

void hd641016_device::execute_run()
{
	m_ssp = m_program.read_dword(0x000000);
	m_pc = m_program.read_dword(0x000004);
	debugger_instruction_hook(m_pc);

	m_icount = 0;
}

void hd641016_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c-%c%c-<%d> %c%c%c%c%c",
				BIT(m_sr, 15) ? 'T' : '.',
				BIT(m_sr, 13) ? 'S' : 'U',
				BIT(m_sr, 12) ? 'I' : '.',
				BIT(m_sr, 8, 3),
				BIT(m_sr, 4) ? 'X' : '.',
				BIT(m_sr, 3) ? 'N' : '.',
				BIT(m_sr, 2) ? 'Z' : '.',
				BIT(m_sr, 1) ? 'V' : '.',
				BIT(m_sr, 0) ? 'C' : '.');
		break;
	}
}

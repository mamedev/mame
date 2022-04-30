// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "nes_vt369_vtunknown_soc.h"


// this has a new RGB555 mode
DEFINE_DEVICE_TYPE(NES_VT369_SOC, nes_vt369_alt_soc_device, "nes_vt369_soc", "VT369 series System on a Chip")
DEFINE_DEVICE_TYPE(NES_VT369_SOC_SWAP, nes_vt369_alt_swap_d5_d6_soc_device, "nes_vt369_soc_swap", "VT369 series System on a Chip (with opcode swapping)")

// uncertain
DEFINE_DEVICE_TYPE(NES_VTUNKNOWN_SOC_CY, nes_vt369_soc_device, "nes_vtunknown_soc_cy", "VTxx series System on a Chip (CY)")
DEFINE_DEVICE_TYPE(NES_VTUNKNOWN_SOC_BT, nes_vtunknown_soc_bt_device, "nes_vtunknown_soc_bt", "VTxx series System on a Chip (BT)")

DEFINE_DEVICE_TYPE(NES_VTUNKNOWN_SOC_DG, nes_vtunknown_soc_dg_device, "nes_vtunknown_soc_dg", "VTxx series System on a Chip (DG)")
DEFINE_DEVICE_TYPE(NES_VTUNKNOWN_SOC_FA, nes_vtunknown_soc_fa_device, "nes_vtunknown_soc_fa", "VTxx series System on a Chip (Family Pocket)")


nes_vt369_soc_device::nes_vt369_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt09_soc_device(mconfig, NES_VTUNKNOWN_SOC_CY, tag, owner, clock),
	m_alu(*this, "alu"),
	m_soundcpu(*this, "soundcpu")
{
}

nes_vtunknown_soc_bt_device::nes_vtunknown_soc_bt_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt09_soc_device(mconfig, NES_VTUNKNOWN_SOC_BT, tag, owner, clock)
{
}


nes_vt369_alt_soc_device::nes_vt369_alt_soc_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt09_soc_device(mconfig, type, tag, owner, clock)
{
}

nes_vt369_alt_soc_device::nes_vt369_alt_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt369_alt_soc_device(mconfig, NES_VT369_SOC, tag, owner, clock)
{
}

nes_vt369_alt_swap_d5_d6_soc_device::nes_vt369_alt_swap_d5_d6_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt369_alt_soc_device(mconfig, NES_VT369_SOC_SWAP, tag, owner, clock)
{
}


nes_vtunknown_soc_dg_device::nes_vtunknown_soc_dg_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt09_soc_device(mconfig, type, tag, owner, clock)
{
}

nes_vtunknown_soc_dg_device::nes_vtunknown_soc_dg_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	nes_vtunknown_soc_dg_device(mconfig, NES_VTUNKNOWN_SOC_DG, tag, owner, clock)
{
}

nes_vtunknown_soc_fa_device::nes_vtunknown_soc_fa_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	nes_vtunknown_soc_dg_device(mconfig, NES_VTUNKNOWN_SOC_FA, tag, owner, clock)
{
}

/***********************************************************************************************************************************************************/
/* VT369? */
/***********************************************************************************************************************************************************/

void nes_vt369_soc_device::device_add_mconfig(machine_config& config)
{
	nes_vt02_vt03_soc_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt369_soc_device::nes_vt369_map);

	VT_VT1682_ALU(config, m_alu, 0);

	M6502(config, m_soundcpu, N2A03_NTSC_XTAL);
	m_soundcpu->set_addrmap(AS_PROGRAM, &nes_vt369_soc_device::vt369_sound_map);
}

void nes_vt369_soc_device::vt369_soundcpu_control_w(offs_t offset, uint8_t data)
{
	logerror("%s: write to sound cpu control reg (reset etc.) %02x\n", machine().describe_context(), data);

	if (data == 0x0d)
		m_soundcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	else
		m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

void nes_vt369_soc_device::vt369_relative_w(offs_t offset, uint8_t data)
{
	logerror("%s: vt369_relative_w %02x %02x\n", machine().describe_context(), offset,  data);
	m_relative[offset] = data;
}


void nes_vt369_soc_device::nes_vt369_map(address_map &map)
{
	map(0x0000, 0x1fff).ram(); // 8k RAM?

	// ddrdismx relies on the mirroring, later SoCs have different mirroring?
	map(0x2000, 0x2007).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));                      // standard PPU registers
	map(0x2010, 0x201f).rw(m_ppu, FUNC(ppu_vt03_device::read_extended), FUNC(ppu_vt03_device::write_extended));  //  extra VT PPU registers

	map(0x4000, 0x4013).rw(m_apu, FUNC(nesapu_device::read), FUNC(nesapu_device::write));

	map(0x4014, 0x4014).r(FUNC(nes_vt369_soc_device::psg1_4014_r)).w(FUNC(nes_vt369_soc_device::vt_dma_w));
	map(0x4015, 0x4015).rw(FUNC(nes_vt369_soc_device::psg1_4015_r), FUNC(nes_vt369_soc_device::psg1_4015_w)); // PSG status / first control register
	map(0x4016, 0x4016).rw(FUNC(nes_vt369_soc_device::in0_r), FUNC(nes_vt369_soc_device::in0_w));
	map(0x4017, 0x4017).r(FUNC(nes_vt369_soc_device::in1_r)).w(FUNC(nes_vt369_soc_device::psg1_4017_w));

	map(0x4034, 0x4034).w(FUNC(nes_vt369_soc_device::vt03_4034_w));  // secondary DMA

	map(0x4100, 0x410b).r(FUNC(nes_vt369_soc_device::vt03_410x_r)).w(FUNC(nes_vt369_soc_device::vt03_410x_w));
	// 0x410c unused
	map(0x410d, 0x410d).w(FUNC(nes_vt369_soc_device::extra_io_control_w));
	map(0x410e, 0x410e).rw(FUNC(nes_vt369_soc_device::extrain_01_r), FUNC(nes_vt369_soc_device::extraout_01_w));
	map(0x410f, 0x410f).rw(FUNC(nes_vt369_soc_device::extrain_23_r), FUNC(nes_vt369_soc_device::extraout_23_w));

	map(0x4112, 0x4112).w(FUNC(nes_vt369_soc_device::vt369_4112_bank6000_select_w));

	// 0x4114 RS232 timer (low)
	// 0x4115 RS232 timer (high)
	// 0x4116 unused
	// 0x4117 unused
	// 0x4118 unused
	map(0x4119, 0x4119).r(FUNC(nes_vt369_soc_device::rs232flags_region_r));
	// 0x411a RS232 TX data
	// 0x411b RS232 RX data
	map(0x411c, 0x411c).w(FUNC(nes_vt369_soc_device::vt369_411c_bank6000_enable_w));

	map(0x4130, 0x4130).rw(m_alu, FUNC(vrt_vt1682_alu_device::alu_out_1_r), FUNC(vrt_vt1682_alu_device::alu_oprand_1_w));
	map(0x4131, 0x4131).rw(m_alu, FUNC(vrt_vt1682_alu_device::alu_out_2_r), FUNC(vrt_vt1682_alu_device::alu_oprand_2_w));
	map(0x4132, 0x4132).rw(m_alu, FUNC(vrt_vt1682_alu_device::alu_out_3_r), FUNC(vrt_vt1682_alu_device::alu_oprand_3_w));
	map(0x4133, 0x4133).rw(m_alu, FUNC(vrt_vt1682_alu_device::alu_out_4_r), FUNC(vrt_vt1682_alu_device::alu_oprand_4_w));
	map(0x4134, 0x4134).rw(m_alu, FUNC(vrt_vt1682_alu_device::alu_out_5_r), FUNC(vrt_vt1682_alu_device::alu_oprand_5_mult_w));
	map(0x4135, 0x4135).rw(m_alu, FUNC(vrt_vt1682_alu_device::alu_out_6_r), FUNC(vrt_vt1682_alu_device::alu_oprand_6_mult_w));
	map(0x4136, 0x4136).w(m_alu, FUNC(vrt_vt1682_alu_device::alu_oprand_5_div_w));
	map(0x4137, 0x4137).w(m_alu, FUNC(vrt_vt1682_alu_device::alu_oprand_6_div_w));

	map(0x414f, 0x414f).r(FUNC(nes_vt369_soc_device::vt369_414f_r));
	map(0x415c, 0x415c).r(FUNC(nes_vt369_soc_device::vt369_415c_r));

	map(0x4160, 0x4161).w(FUNC(nes_vt369_soc_device::vt369_relative_w));
	map(0x4162, 0x4162).w(FUNC(nes_vt369_soc_device::vt369_soundcpu_control_w));

	map(0x41b0, 0x41bf).r(FUNC(nes_vt369_soc_device::vt369_41bx_r)).w(FUNC(nes_vt369_soc_device::vt369_41bx_w));

//  map(0x48a0, 0x48af).r(FUNC(nes_vt369_soc_device::vt369_48ax_r)).w(FUNC(nes_vt369_soc_device::vt369_48ax_w));
	map(0x4800, 0x4fff).ram().share("soundram"); // sound program for 2nd CPU is uploaded here, but some sets aren't uploading anything, do they rely on an internal ROM? other DMA? possibility to map ROM?

	map(0x6000, 0x7fff).r(FUNC(nes_vt369_soc_device::vt369_6000_r)).w(FUNC(nes_vt369_soc_device::vt369_6000_w));

	map(0x8000, 0xffff).rw(FUNC(nes_vt369_soc_device::external_space_read), FUNC(nes_vt369_soc_device::external_space_write));

}


void nes_vt369_soc_device::vt369_sound_map(address_map &map)
{
	map(0x0000, 0x17ff).ram();
	map(0x1800, 0x1fff).ram().share("soundram");

	map(0xf800, 0xffff).ram().share("soundram"); // doesn't actually map here, the CPU fetches vectors from lower addressse
}



void nes_vt369_soc_device::vt369_411c_bank6000_enable_w(offs_t offset, uint8_t data)
{
	logerror("enable bank at 0x6000 (%02x)\n", data);
	m_bank6000_enable = data;
}


void nes_vt369_soc_device::vt369_4112_bank6000_select_w(offs_t offset, uint8_t data)
{
	logerror("set bank at 0x6000 to %02x\n", data);
	m_bank6000 = data;

	// 0x3c = 0x78000
}


uint8_t nes_vt369_soc_device::vt369_6000_r(offs_t offset)
{
	if (m_bank6000_enable & 0x40)
	{
		address_space& spc = this->space(AS_PROGRAM);
		int address = (m_bank6000 * 0x2000) + (offset & 0x1fff);
		return spc.read_byte(address);
	}
	else
	{
		return m_6000_ram[offset];
	}
}

void nes_vt369_soc_device::vt369_6000_w(offs_t offset, uint8_t data)
{
	if (m_bank6000_enable & 0x40)
	{
		logerror("%s: write to 0x6xxx with ROM enabled? %04x %02x\n", machine().describe_context(), offset, data);
	}
	else
	{
		m_6000_ram[offset] = data;
	}
}


void nes_vt369_soc_device::device_start()
{
	nes_vt02_vt03_soc_device::device_start();

	m_6000_ram.resize(0x2000);
	m_bank6000 = 0;
	m_bank6000_enable = 0;
	m_relative[0] = m_relative[1] = 0x00;

	save_item(NAME(m_6000_ram));
	save_item(NAME(m_bank6000));
	save_item(NAME(m_bank6000_enable));
	save_item(NAME(m_relative));
}

void nes_vt369_soc_device::device_reset()
{
	nes_vt02_vt03_soc_device::device_reset();
	m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}


uint8_t nes_vt369_soc_device::vt369_41bx_r(offs_t offset)
{
	switch (offset)
	{
	case 0x07:
		return 0x04;
	default:
		return 0x00;
	}
}

void nes_vt369_soc_device::vt369_41bx_w(offs_t offset, uint8_t data)
{
	logerror("vt369_41bx_w %02x %02x\n", offset, data);
}


uint8_t nes_vt369_soc_device::vt369_414f_r()
{
	return 0xff;
}

uint8_t nes_vt369_soc_device::vt369_415c_r()
{
	return 0xff;
}


void nes_vt369_soc_device::vt369_48ax_w(offs_t offset, uint8_t data)
{
	logerror("vt369_48ax_w %02x %02x\n", offset, data);
}

uint8_t nes_vt369_soc_device::vt369_48ax_r(offs_t offset)
{
	switch (offset)
	{
	case 0x04:
		return 0x01;
	case 0x05:
		return 0x01;
	default:
		return 0x00;
	}
}

/***********************************************************************************************************************************************************/
/* this might just be the same as vt369 but with the games not using all features */
/***********************************************************************************************************************************************************/

void nes_vt369_alt_soc_device::device_add_mconfig(machine_config& config)
{
	nes_vt02_vt03_soc_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt369_alt_soc_device::nes_vt_hh_map);
}

void nes_vt369_alt_soc_device::vtfp_411d_w(uint8_t data)
{
	// controls chram access and mapper emulation modes in later models
	logerror("vtfp_411d_w  %02x\n", data);
	m_411d = data;
	update_banks();
}

uint8_t nes_vt369_alt_soc_device::vthh_414a_r()
{
	return 0x80;
}

uint8_t nes_vt369_alt_soc_device::extra_rom_r()
{
	// this reads from the 'extra ROM' area (serial style protocol) and code is copied on gtct885 to e00 in RAM, jumps to it at EDF9: jsr $0e1c
	return machine().rand();
}


void nes_vt369_alt_soc_device::nes_vt_hh_map(address_map &map)
{
	nes_vt02_vt03_soc_device::nes_vt_map(map);

	map(0x0000, 0x1fff).mask(0x0fff).ram();

	map(0x414a, 0x414a).r(FUNC(nes_vt369_alt_soc_device::vthh_414a_r));
	map(0x411d, 0x411d).w(FUNC(nes_vt369_alt_soc_device::vtfp_411d_w));

	map(0x4153, 0x4153).r(FUNC(nes_vt369_alt_soc_device::extra_rom_r)); // extra SPI? / SEEPROM port?
}


void nes_vt369_alt_swap_d5_d6_soc_device::encryption_4169_w(uint8_t data)
{
	if (data == 0x01)
		downcast<n2a03_core_swap_op_d5_d6 &>(*m_maincpu).set_encryption_state(false);
	else if (data == 0x00)
		downcast<n2a03_core_swap_op_d5_d6 &>(*m_maincpu).set_encryption_state(true);
	else
		logerror("%s: encryption_4169_w %02x\n", machine().describe_context(), data);
}

void nes_vt369_alt_swap_d5_d6_soc_device::nes_vt_hh_swap_map(address_map &map)
{
	nes_vt369_alt_soc_device::nes_vt_hh_map(map);

	map(0x4169, 0x4169).w(FUNC(nes_vt369_alt_swap_d5_d6_soc_device::encryption_4169_w));
}



void nes_vt369_alt_swap_d5_d6_soc_device::device_add_mconfig(machine_config& config)
{
	nes_vt02_vt03_soc_device::device_add_mconfig(config);

	N2A03_CORE_SWAP_OP_D5_D6(config.replace(), m_maincpu, NTSC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt369_alt_swap_d5_d6_soc_device::nes_vt_hh_swap_map);
}

/***********************************************************************************************************************************************************/
/* this might also just be the same as vt369 but with the games not using all features */
/***********************************************************************************************************************************************************/

void nes_vtunknown_soc_dg_device::device_add_mconfig(machine_config& config)
{
	nes_vt02_vt03_soc_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vtunknown_soc_dg_device::nes_vt_dg_map);
}

void nes_vtunknown_soc_dg_device::vt03_411c_w(uint8_t data)
{
	logerror("vt03_411c_w  %02x\n", data);
	m_411c = data;
	update_banks();
}

void nes_vtunknown_soc_dg_device::nes_vt_dg_map(address_map &map)
{
	nes_vt02_vt03_soc_device::nes_vt_map(map);

	map(0x0000, 0x1fff).ram();
	map(0x411c, 0x411c).w(FUNC(nes_vtunknown_soc_dg_device::vt03_411c_w));
}

/***********************************************************************************************************************************************************/
/* 'BT' specifics (base = '4K') */
/***********************************************************************************************************************************************************/

void nes_vtunknown_soc_bt_device::device_add_mconfig(machine_config& config)
{
	nes_vt02_vt03_soc_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vtunknown_soc_bt_device::nes_vt_bt_map);
}

void nes_vtunknown_soc_bt_device::nes_vt_bt_map(address_map &map)
{
	nes_vt_4k_ram_map(map);
	map(0x412c, 0x412c).w(FUNC(nes_vtunknown_soc_bt_device::vt03_412c_extbank_w));
}

void nes_vtunknown_soc_bt_device::vt03_412c_extbank_w(uint8_t data)
{
	m_upper_write_412c_callback(data);
}


/***********************************************************************************************************************************************************/
/* 'FA' specifics (base = 'DG') */ // used by fapocket
/***********************************************************************************************************************************************************/

void nes_vtunknown_soc_fa_device::device_add_mconfig(machine_config& config)
{
	nes_vt02_vt03_soc_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vtunknown_soc_fa_device::nes_vt_fa_map);
}

uint8_t nes_vtunknown_soc_fa_device::vtfa_412c_r()
{
	return m_upper_read_412c_callback();
}

void nes_vtunknown_soc_fa_device::vtfa_412c_extbank_w(uint8_t data)
{
	m_upper_write_412c_callback(data);

}

void nes_vtunknown_soc_fa_device::vtfp_4242_w(uint8_t data)
{
	logerror("vtfp_4242_w %02x\n", data);
	m_4242 = data;
}

void nes_vtunknown_soc_fa_device::nes_vt_fa_map(address_map &map)
{
	nes_vtunknown_soc_dg_device::nes_vt_dg_map(map);

	map(0x412c, 0x412c).r(FUNC(nes_vtunknown_soc_fa_device::vtfa_412c_r)).w(FUNC(nes_vtunknown_soc_fa_device::vtfa_412c_extbank_w));
	map(0x4242, 0x4242).w(FUNC(nes_vtunknown_soc_fa_device::vtfp_4242_w));
}


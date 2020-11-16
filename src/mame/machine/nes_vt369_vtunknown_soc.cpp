// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "nes_vt369_vtunknown_soc.h"


// this has a new RGB555 mode
DEFINE_DEVICE_TYPE(NES_VT369_SOC, nes_vt369_soc_device, "nes_vt369_soc", "VT369 series System on a Chip")

// uncertain
DEFINE_DEVICE_TYPE(NES_VTUNKNOWN_SOC_CY, nes_vtunknown_soc_cy_device, "nes_vtunknown_soc_cy", "VTxx series System on a Chip (CY)")
DEFINE_DEVICE_TYPE(NES_VTUNKNOWN_SOC_BT, nes_vtunknown_soc_bt_device, "nes_vtunknown_soc_bt", "VTxx series System on a Chip (BT)")

DEFINE_DEVICE_TYPE(NES_VTUNKNOWN_SOC_DG, nes_vtunknown_soc_dg_device, "nes_vtunknown_soc_dg", "VTxx series System on a Chip (DG)")
DEFINE_DEVICE_TYPE(NES_VTUNKNOWN_SOC_FA, nes_vtunknown_soc_fa_device, "nes_vtunknown_soc_fa", "VTxx series System on a Chip (Family Pocket)")


nes_vtunknown_soc_cy_device::nes_vtunknown_soc_cy_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt09_soc_device(mconfig, NES_VTUNKNOWN_SOC_CY, tag, owner, clock)
{
}

nes_vtunknown_soc_bt_device::nes_vtunknown_soc_bt_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt09_soc_device(mconfig, NES_VTUNKNOWN_SOC_BT, tag, owner, clock)
{
}


nes_vt369_soc_device::nes_vt369_soc_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt09_soc_device(mconfig, type, tag, owner, clock)
{
}

nes_vt369_soc_device::nes_vt369_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt369_soc_device(mconfig, NES_VT369_SOC, tag, owner, clock)
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
/* 'CY' specifics (base = '4K') */
/***********************************************************************************************************************************************************/

void nes_vtunknown_soc_cy_device::device_add_mconfig(machine_config& config)
{
	nes_vt02_vt03_soc_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vtunknown_soc_cy_device::nes_vt_cy_map);
}

void nes_vtunknown_soc_cy_device::nes_vt_cy_map(address_map &map)
{
	nes_vt_4k_ram_map(map);
	map(0x41b0, 0x41bf).r(FUNC(nes_vtunknown_soc_cy_device::vt03_41bx_r)).w(FUNC(nes_vtunknown_soc_cy_device::vt03_41bx_w));
	map(0x4130, 0x4136).r(FUNC(nes_vtunknown_soc_cy_device::vt03_413x_r)).w(FUNC(nes_vtunknown_soc_cy_device::vt03_413x_w));
	map(0x414f, 0x414f).r(FUNC(nes_vtunknown_soc_cy_device::vt03_414f_r));
	map(0x415c, 0x415c).r(FUNC(nes_vtunknown_soc_cy_device::vt03_415c_r));

	map(0x48a0, 0x48af).r(FUNC(nes_vtunknown_soc_cy_device::vt03_48ax_r)).w(FUNC(nes_vtunknown_soc_cy_device::vt03_48ax_w));
}

void nes_vtunknown_soc_cy_device::device_start()
{
	nes_vt02_vt03_soc_device::device_start();
	save_item(NAME(m_413x));
}


uint8_t nes_vtunknown_soc_cy_device::vt03_41bx_r(offs_t offset)
{
	switch (offset)
	{
	case 0x07:
		return 0x04;
	default:
		return 0x00;
	}
}

void nes_vtunknown_soc_cy_device::vt03_41bx_w(offs_t offset, uint8_t data)
{
	logerror("vt03_41bx_w %02x %02x\n", offset, data);
}

uint8_t nes_vtunknown_soc_cy_device::vt03_413x_r(offs_t offset)
{
	logerror("vt03_413x_r %02x\n", offset);
	return m_413x[offset];
}

void nes_vtunknown_soc_cy_device::vt03_413x_w(offs_t offset, uint8_t data)
{
	logerror("vt03_413x_w %02x %02x\n", offset, data);
	// VT168 style ALU ??
	m_413x[offset] = data;
	if (offset == 0x5)
	{
		uint32_t res = uint32_t((m_413x[5] << 8) | m_413x[4]) * uint32_t((m_413x[1] << 8) | m_413x[0]);
		m_413x[0] = res & 0xFF;
		m_413x[1] = (res >> 8) & 0xFF;
		m_413x[2] = (res >> 16) & 0xFF;
		m_413x[3] = (res >> 24) & 0xFF;
		m_413x[6] = 0x00;

	}
	else if (offset == 0x6)
	{
		/*uint32_t res = uint32_t((m_413x[5] << 8) | m_413x[4]) * uint32_t((m_413x[1] << 8) | m_413x[0]);
		m_413x[0] = res & 0xFF;
		m_413x[1] = (res >> 8) & 0xFF;
		m_413x[2] = (res >> 16) & 0xFF;
		m_413x[3] = (res >> 24) & 0xFF;*/
		m_413x[6] = 0x00;
	}
}

uint8_t nes_vtunknown_soc_cy_device::vt03_414f_r()
{
	return 0xff;
}

uint8_t nes_vtunknown_soc_cy_device::vt03_415c_r()
{
	return 0xff;
}


void nes_vtunknown_soc_cy_device::vt03_48ax_w(offs_t offset, uint8_t data)
{
	logerror("vt03_48ax_w %02x %02x\n", offset, data);
}

uint8_t nes_vtunknown_soc_cy_device::vt03_48ax_r(offs_t offset)
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
/* 'HH' specifics  (base = '4K') */
/***********************************************************************************************************************************************************/

void nes_vt369_soc_device::device_add_mconfig(machine_config& config)
{
	nes_vt02_vt03_soc_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt369_soc_device::nes_vt_hh_map);
}

void nes_vt369_soc_device::vtfp_411d_w(uint8_t data)
{
	// controls chram access and mapper emulation modes in later models
	logerror("vtfp_411d_w  %02x\n", data);
	m_411d = data;
	update_banks();
}

uint8_t nes_vt369_soc_device::vthh_414a_r()
{
	return 0x80;
}

void nes_vt369_soc_device::nes_vt_hh_map(address_map &map)
{
	nes_vt02_vt03_soc_device::nes_vt_map(map);

	map(0x0000, 0x1fff).mask(0x0fff).ram();

	map(0x414a, 0x414a).r(FUNC(nes_vt369_soc_device::vthh_414a_r));
	map(0x411d, 0x411d).w(FUNC(nes_vt369_soc_device::vtfp_411d_w));
}

/***********************************************************************************************************************************************************/
/* 'DG' specifics  (base = '4K') */
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


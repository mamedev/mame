// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "nes_vt09_soc.h"


DEFINE_DEVICE_TYPE(NES_VT09_SOC,    nes_vt09_soc_device,    "nes_vt09_soc",    "VT09 series System on a Chip (NTSC)")

nes_vt09_soc_device::nes_vt09_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt09_soc_device(mconfig, NES_VT09_SOC, tag, owner, clock)
{
}

nes_vt09_soc_device::nes_vt09_soc_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	nes_vt02_vt03_soc_device(mconfig, type, tag, owner, clock),
	m_upper_write_412c_callback(*this),
	m_upper_read_412c_callback(*this),
	m_upper_read_412d_callback(*this)
{
}

/***********************************************************************************************************************************************************/
/* '4K' specifics */
/***********************************************************************************************************************************************************/

void nes_vt09_soc_device::device_add_mconfig(machine_config& config)
{
	nes_vt02_vt03_soc_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt09_soc_device::nes_vt_4k_ram_map);
}

void nes_vt09_soc_device::device_start()
{
	nes_vt02_vt03_soc_device::device_start();

	m_upper_write_412c_callback.resolve_safe();
	m_upper_read_412c_callback.resolve_safe(0xff);
	m_upper_read_412d_callback.resolve_safe(0xff);
}

void nes_vt09_soc_device::nes_vt_4k_ram_map(address_map &map)
{
	nes_vt02_vt03_soc_device::nes_vt_map(map);
	map(0x0800, 0x0fff).ram();

//  map(0x412c, 0x412c).rw(FUNC(nes_vt09_soc_device::vtfp_412c_r, FUNC(nes_vt09_soc_device::vtfp_412c_extbank_w)); // GPIO
//  map(0x412d, 0x412d).r(FUNC(nes_vt09_soc_device::vtfp_412d_r)); // GPIO

}

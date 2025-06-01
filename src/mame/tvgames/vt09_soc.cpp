// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "vt09_soc.h"


DEFINE_DEVICE_TYPE(VT09_SOC,    vt09_soc_device,    "vt09_soc",    "VT09 series System on a Chip (NTSC)")

vt09_soc_device::vt09_soc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	vt09_soc_device(mconfig, VT09_SOC, tag, owner, clock)
{
}

vt09_soc_device::vt09_soc_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	vt02_vt03_soc_device(mconfig, type, tag, owner, clock),
	m_upper_write_412c_callback(*this),
	m_upper_read_412c_callback(*this, 0xff),
	m_upper_read_412d_callback(*this, 0xff)
{
}

/***********************************************************************************************************************************************************/
/* '4K' specifics */
/***********************************************************************************************************************************************************/

void vt09_soc_device::device_add_mconfig(machine_config& config)
{
	vt02_vt03_soc_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &vt09_soc_device::vt_4k_ram_map);
}

void vt09_soc_device::vt_4k_ram_map(address_map &map)
{
	vt02_vt03_soc_device::vt_map(map);
	map(0x0800, 0x0fff).ram();

//  map(0x412c, 0x412c).rw(FUNC(vt09_soc_device::vtfp_412c_r, FUNC(vt09_soc_device::vtfp_412c_extbank_w)); // GPIO
//  map(0x412d, 0x412d).r(FUNC(vt09_soc_device::vtfp_412d_r)); // GPIO

}

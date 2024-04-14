// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Opcode Super Game Module

***************************************************************************/

#include "emu.h"
#include "sgm.h"
#include "sound/ay8910.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COLECO_SGM, coleco_sgm_device, "coleco_sgm", "Opcode Super Game Module")

coleco_sgm_device::coleco_sgm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, COLECO_SGM, tag, owner, clock),
	device_coleco_expansion_interface(mconfig, *this),
	m_view_lower(*this, "view_lower"),
	m_view_upper(*this, "view_upper")
{
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void coleco_sgm_device::io_map(address_map &map)
{
	map(0x50, 0x50).w("ay", FUNC(ay8910_device::address_w));
	map(0x51, 0x51).w("ay", FUNC(ay8910_device::data_w));
	map(0x52, 0x52).r("ay", FUNC(ay8910_device::data_r));
	map(0x53, 0x53).w(FUNC(coleco_sgm_device::upper_enable_w));
	map(0x7f, 0x7f).w(FUNC(coleco_sgm_device::lower_enable_w));
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void coleco_sgm_device::device_add_mconfig(machine_config &config)
{
	ay8910_device &ay(AY8910(config, "ay", 7.15909_MHz_XTAL / 4));
	ay.add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.0);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void coleco_sgm_device::device_start()
{
	// allocate memory for ram
	m_ram_lower = std::make_unique<uint8_t []>(0x2000);
	m_ram_upper = std::make_unique<uint8_t []>(0x6000);

	// register for save states
	save_pointer(NAME(m_ram_lower), 0x2000);
	save_pointer(NAME(m_ram_upper), 0x6000);

	// install device into memory map
	m_expansion->io_space().install_device(0x00, 0x7f, *this, &coleco_sgm_device::io_map);

	m_expansion->mem_space().install_view(0x0000, 0x1fff, m_view_lower);
	m_view_lower[0].install_ram(0x0000, 0x1fff, &m_ram_lower[0]);

	m_expansion->mem_space().install_view(0x2000, 0x7fff, m_view_upper);
	m_view_upper[0].install_ram(0x2000, 0x7fff, &m_ram_upper[0]);
}

void coleco_sgm_device::device_reset()
{
	m_view_lower.disable();
	m_view_upper.disable();
}

void coleco_sgm_device::upper_enable_w(uint8_t data)
{
	if (BIT(data, 0))
		m_view_upper.select(0);
	else
		m_view_upper.disable();
}

void coleco_sgm_device::lower_enable_w(uint8_t data)
{
	if (BIT(data, 1))
		m_view_lower.disable();
	else
		m_view_lower.select(0);
}

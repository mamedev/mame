// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    IBM 2-8MB Memory Expansion Adapter
    FRU 90X9556
	MCA ID $FCFF

    32-bit memory expansion card. 4 72-pin SIMM slots, 8MB max capacity.

	The ADP entry point is at 064220h linear, the function parameters are at 05FFD8h

***************************************************************************/

#include "emu.h"
#include "ibm_memory_exp_32.h"

#define VERBOSE 1
#include "logmacro.h"

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MCA32_IBM_MEMORY_EXP, mca32_ibm_memory_exp_device, "mca32_ibm_memory_exp", "IBM 2-8MB Memory Expansion Adapter (@FCFF)")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mca32_ibm_memory_exp_device::device_add_mconfig(machine_config &config)
{
	RAM(config, m_expansion_ram).set_default_size("8M");
}

//-------------------------------------------------
//  mca32_planar_lpt_device - constructor
//-------------------------------------------------

mca32_ibm_memory_exp_device::mca32_ibm_memory_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca32_ibm_memory_exp_device(mconfig, MCA32_IBM_MEMORY_EXP, tag, owner, clock)
{
}

mca32_ibm_memory_exp_device::mca32_ibm_memory_exp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_mca32_card_interface(mconfig, *this, 0xfcff),
	m_expansion_ram(*this, "expansion_ram")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mca32_ibm_memory_exp_device::device_start()
{
    m_is_mapped = 0;

	// POS data 3 is the memory present bits, see https://github.com/86Box/86Box/commit/8e6497f01dcf1abe72aea1f317bbba6165e7d026
	set_mca_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mca32_ibm_memory_exp_device::device_reset()
{
    m_is_mapped = 0;
	if(m_is_mapped) unmap();

	reset_option_select();
	m_option_select[MCABus::POS::OPTION_SELECT_DATA_3] = 0xaa;	// 8MB present
}

void mca32_ibm_memory_exp_device::remap()
{
    // LOG("%s\n", FUNCNAME);
}

void mca32_ibm_memory_exp_device::unmap()
{
	//m_is_mapped = false;
}

bool mca32_ibm_memory_exp_device::map_has_changed()
{
	//if(m_is_mapped) unmap();

	// TODO: What do the bits in option select data 4 mean, exactly?

	LOG("%s: Updating map. XMS base = %06X\n", FUNCNAME, m_option_select[MCABus::POS::OPTION_SELECT_DATA_2] * 0x100000);

	offs_t xms_base = m_option_select[MCABus::POS::OPTION_SELECT_DATA_2] * 0x100000;

	if((m_option_select[MCABus::POS::OPTION_SELECT_DATA_4] & 0x3f) == 0x3f)
	{
		if(xms_base > 0)
		{
			LOG("%s: Enabling XMS card\n", FUNCNAME);
		
			m_mca->install_memory(xms_base, xms_base+0x7fffff,
				read8sm_delegate(*m_expansion_ram, FUNC(ram_device::read)), write8sm_delegate(*m_expansion_ram, FUNC(ram_device::write)));
				m_is_mapped = true;
		}
		else
		{
			LOG("%s: XMS base not set, not enabling card\n", FUNCNAME);
		}

	}
	else
	{
		unmap();
	}
	
	return false;
}

void mca32_ibm_memory_exp_device::update_pos_data_1(uint8_t data)
{
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] = data;
	map_has_changed();
}

void mca32_ibm_memory_exp_device::update_pos_data_2(uint8_t data)
{
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_2] = data;
	map_has_changed();
}

void mca32_ibm_memory_exp_device::update_pos_data_3(uint8_t data)
{
	// I think this one's read-only, it shows the memory present.
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_3] = data;
	map_has_changed();
}

void mca32_ibm_memory_exp_device::update_pos_data_4(uint8_t data)
{
	// What do the bits do, exactly? 0x3F means enable, I think.

	m_option_select[MCABus::POS::OPTION_SELECT_DATA_4] = data;
	LOG("%s: D:%02X\n", FUNCNAME, data);
	map_has_changed();
}

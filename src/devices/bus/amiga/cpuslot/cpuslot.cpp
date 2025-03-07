// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    86-pin expansion slot (A500, A1000)
    Coprocessor slot (A2000, B2000)

***************************************************************************/

#include "emu.h"
#include "cpuslot.h"


// type definitions
DEFINE_DEVICE_TYPE(AMIGA_CPUSLOT, amiga_cpuslot_device, "amiga_cpuslot", "Amiga CPU Expansion Slot")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

amiga_cpuslot_device::amiga_cpuslot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_CPUSLOT, tag, owner, clock),
	device_single_card_slot_interface<device_amiga_cpuslot_interface>(mconfig, *this),
	m_space(*this, finder_base::DUMMY_TAG, -1),
	m_cfgout_cb(*this),
	m_ovr_cb(*this),
	m_int2_cb(*this),
	m_int6_cb(*this),
	m_ipl7_cb(*this)
{
}

void amiga_cpuslot_device::device_start()
{
	m_card = get_card_device();
}

void amiga_cpuslot_device::cfgin_w(int state)
{
	if (m_card)
		m_card->cfgin_w(state);
}

void amiga_cpuslot_device::rst_w(int state)
{
	if (m_card)
		m_card->rst_w(state);
}


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

device_amiga_cpuslot_interface::device_amiga_cpuslot_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "amiga_cpuslot")
{
	m_host = dynamic_cast<amiga_cpuslot_device *>(device.owner());
}

// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VP551 Super Sound 4-channel Expander Package emulation

**********************************************************************/

#include "emu.h"
#include "vp550.h"
#include "vp576.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VP576, vp576_device, "vp576", "VP-576 System Expansion")


//-------------------------------------------------
//  VIP_EXPANSION_INTERFACE( expansion_intf )
//-------------------------------------------------

void vp576_device::update_interrupts()
{
	int interrupt = CLEAR_LINE;

	for (int i = 0; i < MAX_SLOTS; i++)
	{
		interrupt |= m_int[i];
	}

	m_slot->interrupt_w(interrupt);
}

static void expansion_cards(device_slot_interface &device)
{
	device.option_add("vp550", VP550);
}


//-------------------------------------------------
//  machine_config( vp576 )
//-------------------------------------------------

void vp576_device::device_add_mconfig(machine_config &config)
{
	VIP_EXPANSION_SLOT(config, m_expansion_slot[0], XTAL(3'521'280)/2, expansion_cards, "vp550").set_fixed(true);
	m_expansion_slot[0]->int_wr_callback().set(FUNC(vp576_device::exp1_int_w));

	VIP_EXPANSION_SLOT(config, m_expansion_slot[1], XTAL(3'521'280)/2, expansion_cards, "vp550").set_fixed(true);
	m_expansion_slot[1]->int_wr_callback().set(FUNC(vp576_device::exp2_int_w));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vp576_device - constructor
//-------------------------------------------------

vp576_device::vp576_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VP576, tag, owner, clock),
	device_vip_expansion_card_interface(mconfig, *this),
	m_expansion_slot(*this, "exp%u", 1)
{
	for (int i = 0; i < MAX_SLOTS; i++)
	{
		m_int[i] = CLEAR_LINE;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vp576_device::device_start()
{
}


//-------------------------------------------------
//  vip_program_w - program write
//-------------------------------------------------

void vp576_device::vip_program_w(offs_t offset, uint8_t data, int cdef, int *minh)
{
	switch (offset & 0xc000)
	{
		case 0x8000:
			m_expansion_slot[0]->program_w(offset, data, cdef, minh);
			break;

		case 0x4000:
			m_expansion_slot[1]->program_w(0x8000 | offset, data, cdef, minh);
			break;
	}
}


//-------------------------------------------------
//  vip_sc_w - status code write
//-------------------------------------------------

void vp576_device::vip_sc_w(int n, int sc)
{
	for (auto & elem : m_expansion_slot)
	{
		elem->sc_w(n, sc);
	}
}


//-------------------------------------------------
//  vip_q_w - Q write
//-------------------------------------------------

void vp576_device::vip_q_w(int state)
{
	for (auto & elem : m_expansion_slot)
	{
		elem->q_w(state);
	}
}


//-------------------------------------------------
//  vip_run_w - RUN write
//-------------------------------------------------

void vp576_device::vip_run_w(int state)
{
	for (auto & elem : m_expansion_slot)
	{
		elem->run_w(state);
	}
}

// license:BSD-3-Clause
// copyright-holders:Dirk Best, hap
/***************************************************************************

Saitek OSA Expansion Slot

Used by Saitek(SciSys) chess computers Leonardo, Galileo, Renaissance.

***************************************************************************/

#include "emu.h"
#include "expansion.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SAITEKOSA_EXPANSION, saitekosa_expansion_device, "saitekosa_expansion", "Saitek OSA Expansion Bus")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  saitekosa_expansion_device - constructor
//-------------------------------------------------

saitekosa_expansion_device::saitekosa_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SAITEKOSA_EXPANSION, tag, owner, clock),
	device_single_card_slot_interface<device_saitekosa_expansion_interface>(mconfig, *this),
	m_stb_handler(*this),
	m_rts_handler(*this),
	m_module(nullptr)
{ }

//-------------------------------------------------
//  saitekosa_expansion_device - destructor
//-------------------------------------------------

saitekosa_expansion_device::~saitekosa_expansion_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void saitekosa_expansion_device::device_start()
{
	// get inserted module
	m_module = get_card_device();

	// resolve callbacks
	m_stb_handler.resolve_safe();
	m_rts_handler.resolve_safe();

	// register for savestates
	save_item(NAME(m_data));
	save_item(NAME(m_nmi));
	save_item(NAME(m_ack));
}

//-------------------------------------------------
//  host to module interface
//-------------------------------------------------

u8 saitekosa_expansion_device::data_r()
{
	if (m_module)
		return m_module->data_r();

	return 0xff;
}

void saitekosa_expansion_device::data_w(u8 data)
{
	if (m_module)
		m_module->data_w(data);

	m_data = data;
}

void saitekosa_expansion_device::nmi_w(int state)
{
	state = (state) ? 1 : 0;

	if (m_module)
		m_module->nmi_w(state);

	m_nmi = state;
}

void saitekosa_expansion_device::ack_w(int state)
{
	state = (state) ? 1 : 0;

	if (m_module)
		m_module->ack_w(state);

	m_ack = state;
}


//**************************************************************************
//  MODULE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_saitekosa_expansion_interface - constructor
//-------------------------------------------------

device_saitekosa_expansion_interface::device_saitekosa_expansion_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "saitekosaexp")
{
	m_expansion = dynamic_cast<saitekosa_expansion_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_saitekosa_expansion_interface - destructor
//-------------------------------------------------

device_saitekosa_expansion_interface::~device_saitekosa_expansion_interface()
{
}

// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    SAM Coupe Expansion Slot

    64-pin slot

***************************************************************************/

#include "emu.h"
#include "expansion.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SAMCOUPE_EXPANSION, samcoupe_expansion_device, "samcoupe_expansion", "SAM Coupe Expansion Bus")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  samcoupe_expansion_device - constructor
//-------------------------------------------------

samcoupe_expansion_device::samcoupe_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SAMCOUPE_EXPANSION, tag, owner, clock),
	device_single_card_slot_interface<device_samcoupe_expansion_interface>(mconfig, *this),
	m_int_handler(*this),
	m_module(nullptr)
{
}

//-------------------------------------------------
//  samcoupe_expansion_device - destructor
//-------------------------------------------------

samcoupe_expansion_device::~samcoupe_expansion_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void samcoupe_expansion_device::device_start()
{
	// get inserted module
	m_module = get_card_device();

	// resolve callbacks
	m_int_handler.resolve_safe();
}

//-------------------------------------------------
//  host to module interface
//-------------------------------------------------

uint8_t samcoupe_expansion_device::mreq_r(offs_t offset)
{
	if (m_module)
		return m_module->mreq_r(offset);

	return 0xff;
}

void samcoupe_expansion_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_module)
		m_module->mreq_w(offset, data);
}

uint8_t samcoupe_expansion_device::iorq_r(offs_t offset)
{
	if (m_module)
		return m_module->iorq_r(offset);

	return 0xff;
}

void samcoupe_expansion_device::iorq_w(offs_t offset, uint8_t data)
{
	if (m_module)
		m_module->iorq_w(offset, data);
}

WRITE_LINE_MEMBER( samcoupe_expansion_device::xmem_w )
{
	if (m_module)
		m_module->xmem_w(state);
}

WRITE_LINE_MEMBER( samcoupe_expansion_device::print_w )
{
	if (m_module)
		m_module->print_w(state);
}


//**************************************************************************
//  MODULE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_samcoupe_expansion_interface - constructor
//-------------------------------------------------

device_samcoupe_expansion_interface::device_samcoupe_expansion_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "samcoupeexp")
{
	m_expansion = dynamic_cast<samcoupe_expansion_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_samcoupe_expansion_interface - destructor
//-------------------------------------------------

device_samcoupe_expansion_interface::~device_samcoupe_expansion_interface()
{
}

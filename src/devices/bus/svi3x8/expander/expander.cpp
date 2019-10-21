// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SVI 318/328 Expansion Slot

    50-pin slot

***************************************************************************/

#include "emu.h"
#include "expander.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SVI_EXPANDER, svi_expander_device, "svi_expander", "SVI 318/328 Expander Bus")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  svi_expander_device - constructor
//-------------------------------------------------

svi_expander_device::svi_expander_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SVI_EXPANDER, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_module(nullptr),
	m_int_handler(*this),
	m_romdis_handler(*this),
	m_ramdis_handler(*this),
	m_ctrl1_handler(*this),
	m_ctrl2_handler(*this),
	m_excsr_handler(*this),
	m_excsw_handler(*this)
{
}

//-------------------------------------------------
//  svi_expander_device - destructor
//-------------------------------------------------

svi_expander_device::~svi_expander_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void svi_expander_device::device_start()
{
	// get inserted module
	m_module = dynamic_cast<device_svi_expander_interface *>(get_card_device());

	// resolve callbacks
	m_int_handler.resolve_safe();
	m_romdis_handler.resolve_safe();
	m_ramdis_handler.resolve_safe();
	m_ctrl1_handler.resolve_safe();
	m_ctrl2_handler.resolve_safe();
	m_excsr_handler.resolve_safe(0xff);
	m_excsw_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void svi_expander_device::device_reset()
{
}

//-------------------------------------------------
//  host to module interface
//-------------------------------------------------

uint8_t svi_expander_device::mreq_r(offs_t offset)
{
	romdis_w(1);
	ramdis_w(1);

	if (m_module)
		return m_module->mreq_r(offset);

	return 0xff;
}

void svi_expander_device::mreq_w(offs_t offset, uint8_t data)
{
	romdis_w(1);
	ramdis_w(1);

	if (m_module)
		m_module->mreq_w(offset, data);
}

uint8_t svi_expander_device::iorq_r(offs_t offset)
{
	if (m_module)
		return m_module->iorq_r(offset);

	return 0xff;
}

void svi_expander_device::iorq_w(offs_t offset, uint8_t data)
{
	if (m_module)
		m_module->iorq_w(offset, data);
}

WRITE_LINE_MEMBER( svi_expander_device::bk21_w )
{
		if (m_module)
			m_module->bk21_w(state);
}

WRITE_LINE_MEMBER( svi_expander_device::bk22_w )
{
		if (m_module)
			m_module->bk22_w(state);
}

WRITE_LINE_MEMBER( svi_expander_device::bk31_w )
{
		if (m_module)
			m_module->bk31_w(state);
}

WRITE_LINE_MEMBER( svi_expander_device::bk32_w )
{
		if (m_module)
			m_module->bk32_w(state);
}


//**************************************************************************
//  CARTRIDGE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_svi_expander_interface - constructor
//-------------------------------------------------

device_svi_expander_interface::device_svi_expander_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_expander = dynamic_cast<svi_expander_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_expansion_interface - destructor
//-------------------------------------------------

device_svi_expander_interface::~device_svi_expander_interface()
{
}

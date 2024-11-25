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
	device_single_card_slot_interface<device_svi_expander_interface>(mconfig, *this),
	device_mixer_interface(mconfig, *this),
	m_module(nullptr),
	m_int_handler(*this),
	m_romdis_handler(*this),
	m_ramdis_handler(*this),
	m_ctrl1_handler(*this),
	m_ctrl2_handler(*this),
	m_excsr_handler(*this, 0xff),
	m_excsw_handler(*this),
	m_dummy(0)
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
	m_module = get_card_device();

	// register for save states
	save_item(NAME(m_dummy));
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

void svi_expander_device::bk21_w(int state)
{
	if (m_module)
		m_module->bk21_w(state);
}

void svi_expander_device::bk22_w(int state)
{
	if (m_module)
		m_module->bk22_w(state);
}

void svi_expander_device::bk31_w(int state)
{
	if (m_module)
		m_module->bk31_w(state);
}

void svi_expander_device::bk32_w(int state)
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
	device_interface(device, "svi3x8exp")
{
	m_expander = dynamic_cast<svi_expander_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_expansion_interface - destructor
//-------------------------------------------------

device_svi_expander_interface::~device_svi_expander_interface()
{
}

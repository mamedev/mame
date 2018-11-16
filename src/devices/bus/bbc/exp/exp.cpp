// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Master Compact Expansion slot emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_EXP_SLOT, bbc_exp_slot_device, "bbc_exp_slot", "BBC Master Compact Expansion port")



//**************************************************************************
//  DEVICE BBC_EXP PORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_exp_interface - constructor
//-------------------------------------------------

device_bbc_exp_interface::device_bbc_exp_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<bbc_exp_slot_device *>(device.owner());
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_exp_slot_device - constructor
//-------------------------------------------------

bbc_exp_slot_device::bbc_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BBC_EXP_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_card(nullptr),
	m_irq_handler(*this),
	m_nmi_handler(*this),
	m_cb1_handler(*this),
	m_cb2_handler(*this)
{
}


//-------------------------------------------------
//  device_validity_check -
//-------------------------------------------------

void bbc_exp_slot_device::device_validity_check(validity_checker &valid) const
{
	device_t *const carddev = get_card_device();
	if (carddev && !dynamic_cast<device_bbc_exp_interface *>(carddev))
		osd_printf_error("Card device %s (%s) does not implement device_bbc_exp_interface\n", carddev->tag(), carddev->name());
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_exp_slot_device::device_start()
{
	device_t *const carddev = get_card_device();
	m_card = dynamic_cast<device_bbc_exp_interface *>(carddev);
	if (carddev && !m_card)
		fatalerror("Card device %s (%s) does not implement device_bbc_exp_interface\n", carddev->tag(), carddev->name());

	// resolve callbacks
	m_irq_handler.resolve_safe();
	m_nmi_handler.resolve_safe();

	m_cb1_handler.resolve_safe();
	m_cb2_handler.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_exp_slot_device::device_reset()
{
}

//-------------------------------------------------
//  read
//-------------------------------------------------

READ8_MEMBER(bbc_exp_slot_device::fred_r)
{
	if (m_card)
		return m_card->fred_r(space, offset);
	else
		return 0xff;
}

READ8_MEMBER(bbc_exp_slot_device::jim_r)
{
	if (m_card)
		return m_card->jim_r(space, offset);
	else
		return 0xff;
}

READ8_MEMBER(bbc_exp_slot_device::sheila_r)
{
	if (m_card)
		return m_card->sheila_r(space, offset);
	else
		return 0xfe;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

WRITE8_MEMBER(bbc_exp_slot_device::fred_w)
{
	if (m_card)
		m_card->fred_w(space, offset, data);
}

WRITE8_MEMBER(bbc_exp_slot_device::jim_w)
{
	if (m_card)
		m_card->jim_w(space, offset, data);
}

WRITE8_MEMBER(bbc_exp_slot_device::sheila_w)
{
	if (m_card)
		m_card->sheila_w(space, offset, data);
}

//-------------------------------------------------
//  pb_r
//-------------------------------------------------

READ8_MEMBER(bbc_exp_slot_device::pb_r)
{
	if (m_card)
		return 0x1f | m_card->pb_r(space, 0);
	else
		return 0xff;
}


//-------------------------------------------------
//  pb_w
//-------------------------------------------------

WRITE8_MEMBER(bbc_exp_slot_device::pb_w)
{
	if (m_card)
		m_card->pb_w(space, 0, data);
}

//-------------------------------------------------
//  SLOT_INTERFACE( bbc_exp_devices )
//-------------------------------------------------


// slot devices
//#include "autocue.h"
#include "mertec.h"


void bbc_exp_devices(device_slot_interface &device)
{
	//device.option_add("autocue", BBC_AUTOCUE);       /* Autocue RAM disk board */
	device.option_add("mertec",  BBC_MERTEC);        /* Mertec Compact Companion */
}

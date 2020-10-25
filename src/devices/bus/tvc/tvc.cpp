// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*********************************************************************

    tvc.c

*********************************************************************/

#include "emu.h"
#include "tvc.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(TVCEXP_SLOT, tvcexp_slot_device, "tvcexp_slot", "TVC64 Expansion Slot")


//**************************************************************************
//    TVC Expansion Interface
//**************************************************************************

//-------------------------------------------------
//  device_tvcexp_interface - constructor
//-------------------------------------------------

device_tvcexp_interface::device_tvcexp_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "tvc64exp")
{
}


//-------------------------------------------------
//  ~device_tvcexp_interface - destructor
//-------------------------------------------------

device_tvcexp_interface::~device_tvcexp_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tvcexp_slot_device - constructor
//-------------------------------------------------
tvcexp_slot_device::tvcexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TVCEXP_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_tvcexp_interface>(mconfig, *this),
	m_out_irq_cb(*this),
	m_out_nmi_cb(*this),
	m_cart(nullptr)
{
}

//-------------------------------------------------
//  tvcexp_slot_device - destructor
//-------------------------------------------------

tvcexp_slot_device::~tvcexp_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tvcexp_slot_device::device_start()
{
	m_cart = get_card_device();

	// resolve callbacks
	m_out_irq_cb.resolve_safe();
	m_out_nmi_cb.resolve_safe();
}

/*-------------------------------------------------
    module id read
-------------------------------------------------*/

uint8_t tvcexp_slot_device::id_r()
{
	uint8_t result = 0x00;

	if (m_cart)
		result = m_cart->id_r() & 0x03;

	return result;
}

/*-------------------------------------------------
    module interrupt ack
-------------------------------------------------*/

void tvcexp_slot_device::int_ack()
{
	if (m_cart)
		m_cart->int_ack();
}

/*-------------------------------------------------
    module int read
-------------------------------------------------*/

uint8_t tvcexp_slot_device::int_r()
{
	uint8_t result = 1;

	if (m_cart)
		result = m_cart->int_r() & 0x01;

	return result;
}


/*-------------------------------------------------
    read
-------------------------------------------------*/

uint8_t tvcexp_slot_device::read(offs_t offset)
{
	if (m_cart)
		return m_cart->read(offset);
	else
		return 0x00;
}


/*-------------------------------------------------
    write
-------------------------------------------------*/

void tvcexp_slot_device::write(offs_t offset, uint8_t data)
{
	if (m_cart)
		m_cart->write(offset, data);
}

/*-------------------------------------------------
    IO read
-------------------------------------------------*/

uint8_t tvcexp_slot_device::io_read(offs_t offset)
{
	if (m_cart)
		return m_cart->io_read(offset);
	else
		return 0x00;
}


/*-------------------------------------------------
   IO write
-------------------------------------------------*/

void tvcexp_slot_device::io_write(offs_t offset, uint8_t data)
{
	if (m_cart)
		m_cart->io_write(offset, data);
}

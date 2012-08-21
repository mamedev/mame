/*********************************************************************

    tvcexp.c

*********************************************************************/

#include "emu.h"
#include "tvcexp.h"
#include "emuopts.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type TVCEXP_SLOT  = &device_creator<tvcexp_slot_device>;


//**************************************************************************
//    TVC Expansion Interface
//**************************************************************************

//-------------------------------------------------
//  device_tvcexp_interface - constructor
//-------------------------------------------------

device_tvcexp_interface::device_tvcexp_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
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
tvcexp_slot_device::tvcexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, TVCEXP_SLOT, "TVC64 Expansion Slot", tag, owner, clock),
		device_slot_interface(mconfig, *this)
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
	m_cart = dynamic_cast<device_tvcexp_interface *>(get_card_device());

	// resolve callbacks
	m_out_irq_func.resolve(m_out_irq_cb, *this);
	m_out_nmi_func.resolve(m_out_nmi_cb, *this);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void tvcexp_slot_device::device_config_complete()
{
	// inherit a copy of the static data
	const tvcexp_interface *intf = reinterpret_cast<const tvcexp_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<tvcexp_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
    	memset(&m_out_irq_cb, 0, sizeof(m_out_irq_cb));
    	memset(&m_out_nmi_cb, 0, sizeof(m_out_nmi_cb));
	}
}


/*-------------------------------------------------
    module id read
-------------------------------------------------*/

UINT8 tvcexp_slot_device::id_r()
{
	UINT8 result = 0x00;

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

UINT8 tvcexp_slot_device::int_r()
{
	UINT8 result = 1;

	if (m_cart)
		result = m_cart->int_r() & 0x01;

	return result;
}


/*-------------------------------------------------
    read
-------------------------------------------------*/

READ8_MEMBER(tvcexp_slot_device::read)
{
	if (m_cart)
		return m_cart->read(space, offset);
	else
		return 0x00;
}


/*-------------------------------------------------
    write
-------------------------------------------------*/

WRITE8_MEMBER(tvcexp_slot_device::write)
{
	if (m_cart)
		m_cart->write(space, offset, data);
}

/*-------------------------------------------------
    IO read
-------------------------------------------------*/

READ8_MEMBER(tvcexp_slot_device::io_read)
{
	if (m_cart)
		return m_cart->io_read(space, offset);
	else
		return 0x00;
}


/*-------------------------------------------------
   IO write
-------------------------------------------------*/

WRITE8_MEMBER(tvcexp_slot_device::io_write)
{
	if (m_cart)
		m_cart->io_write(space, offset, data);
}

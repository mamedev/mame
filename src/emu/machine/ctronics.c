/***************************************************************************

    Centronics printer interface

***************************************************************************/

#include "emu.h"
#include "ctronics.h"

//**************************************************************************
//  CENTRONICS SLOT DEVICE
//**************************************************************************

// device type definition
const device_type CENTRONICS = &device_creator<centronics_device>;

//-------------------------------------------------
//  centronics_device - constructor
//-------------------------------------------------

centronics_device::centronics_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CENTRONICS, "Centronics", tag, owner, clock),
		device_slot_interface(mconfig, *this),
		m_dev(NULL)
{

}

//-------------------------------------------------
//  centronics_device - destructor
//-------------------------------------------------

centronics_device::~centronics_device()
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void centronics_device::device_config_complete()
{
	// inherit a copy of the static data
	const centronics_interface *intf = reinterpret_cast<const centronics_interface *>(static_config());
	if (intf != NULL)
		*static_cast<centronics_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_ack_func, 0, sizeof(m_out_ack_func));
		memset(&m_out_busy_func, 0, sizeof(m_out_busy_func));
		memset(&m_out_not_busy_func, 0, sizeof(m_out_not_busy_func));
	}

	m_dev = dynamic_cast<device_centronics_peripheral_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void centronics_device::device_start()
{
	/* resolve callbacks */
	m_out_ack_func.resolve(m_out_ack_cb, *this);
	m_out_busy_func.resolve(m_out_busy_cb, *this);
	m_out_not_busy_func.resolve(m_out_not_busy_cb, *this);
}

/*****************************************************************************
    GLOBAL VARIABLES
*****************************************************************************/

const centronics_interface standard_centronics =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

//**************************************************************************
//  DEVICE CENTRONICS PERIPHERAL INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_centronics_peripheral_interface - constructor
//-------------------------------------------------

device_centronics_peripheral_interface::device_centronics_peripheral_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	/* set some initial values */
	m_pe = FALSE;
	m_fault = FALSE;
	m_busy = TRUE;
	m_strobe = TRUE;
	m_ack = FALSE;
	m_data = 0x00;
}


//-------------------------------------------------
//  ~device_centronics_peripheral_interface - destructor
//-------------------------------------------------

device_centronics_peripheral_interface::~device_centronics_peripheral_interface()
{
}

//**************************************************************************
//  CENTRONICS PRINTER DEVICE
//**************************************************************************

// device type definition
const device_type CENTRONICS_PRINTER = &device_creator<centronics_printer_device>;

/*****************************************************************************
    PRINTER INTERFACE
*****************************************************************************/
const struct printer_interface centronics_printer_config =
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, centronics_printer_device, printer_online)
};

static MACHINE_CONFIG_FRAGMENT( centronics_printer )
	MCFG_PRINTER_ADD("printer")
	MCFG_DEVICE_CONFIG(centronics_printer_config)
MACHINE_CONFIG_END


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/
//-------------------------------------------------
//  centronics_printer_device - constructor
//-------------------------------------------------

centronics_printer_device::centronics_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CENTRONICS_PRINTER, "Centronics Printer", tag, owner, clock),
		device_centronics_peripheral_interface( mconfig, *this )
{
}
//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor centronics_printer_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( centronics_printer );
}


/*-------------------------------------------------
    printer_online - callback that
    sets us busy when the printer goes offline
-------------------------------------------------*/

WRITE_LINE_MEMBER(centronics_printer_device::printer_online)
{
	/* when going online, set PE and FAULT high and BUSY low */
	m_pe = state;
	m_fault = state;
	m_busy = !state;
}

static TIMER_CALLBACK( timer_ack_callback )
{
	centronics_printer_device *printer = reinterpret_cast<centronics_printer_device *>(ptr);
	printer->ack_callback(param);
}

static TIMER_CALLBACK( timer_busy_callback )
{
	centronics_printer_device *printer = reinterpret_cast<centronics_printer_device *>(ptr);
	printer->busy_callback(param);
}

void centronics_printer_device::ack_callback(UINT8 param)
{
	/* signal change */
	m_owner->out_ack(param);
	m_ack = param;

	if (param == FALSE)
	{
		/* data is now ready, output it */
		m_printer->output(m_data);

		/* ready to receive more data, return BUSY to low */
		machine().scheduler().timer_set(attotime::from_usec(7), FUNC(timer_busy_callback), FALSE, this);
	}
}


void centronics_printer_device::busy_callback(UINT8 param)
{
	/* signal change */
	m_owner->out_busy(param);
	m_owner->out_not_busy(!param);
	m_busy = param;

	if (param == TRUE)
	{
		/* timer to turn ACK low to receive data */
		machine().scheduler().timer_set(attotime::from_usec(10), FUNC(timer_ack_callback), FALSE, this);
	}
	else
	{
		/* timer to return ACK to high state */
		machine().scheduler().timer_set(attotime::from_usec(5), FUNC(timer_ack_callback), TRUE, this);
	}
}

void centronics_printer_device::device_start()
{
	m_owner = dynamic_cast<centronics_device *>(owner());

	/* get printer device */
	m_printer = subdevice<printer_image_device>("printer");

	/* register for state saving */
	save_item(NAME(m_auto_fd));
	save_item(NAME(m_strobe));
	save_item(NAME(m_busy));
	save_item(NAME(m_ack));
	save_item(NAME(m_data));
}

void centronics_printer_device::device_reset()
{

}

/*-------------------------------------------------
    centronics_strobe_w - signal that data is
    ready
-------------------------------------------------*/

void centronics_printer_device::strobe_w(UINT8 state)
{
	/* look for a high -> low transition */
	if (m_strobe == TRUE && state == FALSE && m_busy == FALSE)
	{
		/* STROBE has gone low, data is ready */
		machine().scheduler().timer_set(attotime::zero, FUNC(timer_busy_callback), TRUE, this);
	}

	m_strobe = state;
}


/*-------------------------------------------------
    centronics_prime_w - initialize and reset
    printer (centronics mode)
-------------------------------------------------*/

void centronics_printer_device::init_prime_w(UINT8 state)
{
	/* reset printer if line is low */
	if (state == FALSE)
		device_reset();
}


SLOT_INTERFACE_START(centronics_printer)
	SLOT_INTERFACE("printer", CENTRONICS_PRINTER)
SLOT_INTERFACE_END

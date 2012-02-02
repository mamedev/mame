/***************************************************************************

    Centronics printer interface

***************************************************************************/

#include "emu.h"
#include "ctronics.h"


// device type definition
const device_type CENTRONICS = &device_creator<centronics_device>;

//-------------------------------------------------
//  centronics_device - constructor
//-------------------------------------------------

centronics_device::centronics_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, CENTRONICS, "Centronics", tag, owner, clock)
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
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void centronics_device::device_start()
{
	/* set some initial values */
	m_pe = FALSE;
	m_fault = FALSE;
	m_busy = TRUE;
	m_strobe = TRUE;

	/* get printer device */
	m_printer = subdevice<printer_image_device>("printer");

	/* resolve callbacks */
	m_out_ack_func.resolve(m_out_ack_cb, *this);
	m_out_busy_func.resolve(m_out_busy_cb, *this);
	m_out_not_busy_func.resolve(m_out_not_busy_cb, *this);

	/* register for state saving */
	save_item(NAME(m_auto_fd));
	save_item(NAME(m_strobe));
	save_item(NAME(m_busy));
	save_item(NAME(m_ack));
	save_item(NAME(m_data));

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


/*****************************************************************************
    PRINTER INTERFACE
*****************************************************************************/
const struct printer_interface centronics_printer_config =
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, centronics_device, printer_online)
};

static MACHINE_CONFIG_FRAGMENT( centronics )
	MCFG_PRINTER_ADD("printer")
	MCFG_DEVICE_CONFIG(centronics_printer_config)
MACHINE_CONFIG_END


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor centronics_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( centronics );
}


/*-------------------------------------------------
    printer_online - callback that
    sets us busy when the printer goes offline
-------------------------------------------------*/

WRITE_LINE_MEMBER(centronics_device::printer_online)
{
	/* when going online, set PE and FAULT high and BUSY low */
	m_pe = state;
	m_fault = state;
	m_busy = !state;
}

static TIMER_CALLBACK( timer_ack_callback )
{
    centronics_device *cent = reinterpret_cast<centronics_device *>(ptr);
	cent->ack_callback(param);
}

static TIMER_CALLBACK( timer_busy_callback )
{
    centronics_device *cent = reinterpret_cast<centronics_device *>(ptr);
	cent->busy_callback(param);
}

void centronics_device::ack_callback(UINT8 param)
{
	/* signal change */
	m_out_ack_func(param);
	m_ack = param;

	if (param == FALSE)
	{
		/* data is now ready, output it */
		m_printer->output(m_data);

		/* ready to receive more data, return BUSY to low */
		machine().scheduler().timer_set(attotime::from_usec(7), FUNC(timer_busy_callback), FALSE, this);
	}
}


void centronics_device::busy_callback(UINT8 param)
{
	/* signal change */
	m_out_busy_func(param);
	m_out_not_busy_func(!param);
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


/*-------------------------------------------------
    set_line - helper to set individual bits
-------------------------------------------------*/

void centronics_device::set_line(int line, int state)
{
	if (state)
		m_data |= 1 << line;
	else
		m_data &= ~(1 << line);
}


/*-------------------------------------------------
    centronics_strobe_w - signal that data is
    ready
-------------------------------------------------*/

WRITE_LINE_MEMBER( centronics_device::strobe_w )
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

WRITE_LINE_MEMBER( centronics_device::init_prime_w )
{
	/* reset printer if line is low */
	if (state == FALSE)
		device_reset();
}

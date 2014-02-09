#include "emu.h"
#include "image.h"

//**************************************************************************
//  CENTRONICS PRINTER DEVICE
//**************************************************************************

// device type definition
const device_type CENTRONICS_PRINTER_IMAGE = &device_creator<centronics_printer_image_device>;

/*****************************************************************************
    PRINTER INTERFACE
*****************************************************************************/
const struct printer_interface centronics_printer_config =
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, centronics_printer_image_device, printer_online)
};

static MACHINE_CONFIG_FRAGMENT( centronics_printer )
	MCFG_PRINTER_ADD("printer")
	MCFG_DEVICE_CONFIG(centronics_printer_config)
MACHINE_CONFIG_END


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/
//-------------------------------------------------
//  centronics_printer_image_device - constructor
//-------------------------------------------------

centronics_printer_image_device::centronics_printer_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CENTRONICS_PRINTER_IMAGE, "Centronics Printer", tag, owner, clock, "centronics_printer", __FILE__),
		device_centronics_peripheral_interface( mconfig, *this )
{
}
//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor centronics_printer_image_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( centronics_printer );
}

/*-------------------------------------------------
    printer_online - callback that
    sets us busy when the printer goes offline
-------------------------------------------------*/

WRITE_LINE_MEMBER(centronics_printer_image_device::printer_online)
{
	output_perror(!state);
}

void centronics_printer_image_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_ACK:
		output_ack(param);

		if (param == FALSE)
		{
			/* data is now ready, output it */
			m_printer->output(m_data);

			/* ready to receive more data, return BUSY to low */
			timer_set(attotime::from_usec(7), TIMER_BUSY, FALSE);
		}
		break;

	case TIMER_BUSY:
		m_busy = param;
		output_busy(m_busy);

		if (param == TRUE)
		{
			/* timer to turn ACK low to receive data */
			timer_set(attotime::from_usec(10), TIMER_ACK, FALSE);
		}
		else
		{
			/* timer to return ACK to high state */
			timer_set(attotime::from_usec(5), TIMER_ACK, TRUE);
		}
	}
}

void centronics_printer_image_device::device_start()
{
	m_owner = dynamic_cast<centronics_device *>(owner());

	/* get printer device */
	m_printer = subdevice<printer_image_device>("printer");

	/* register for state saving */
	save_item(NAME(m_strobe));
	save_item(NAME(m_data));
	save_item(NAME(m_busy));
}

void centronics_printer_image_device::device_reset()
{
	m_busy = FALSE;
	output_busy(m_busy);
	output_fault(1);
	output_ack(1);
}

/*-------------------------------------------------
    centronics_strobe_w - signal that data is
    ready
-------------------------------------------------*/

WRITE_LINE_MEMBER( centronics_printer_image_device::input_strobe )
{
	/* look for a high -> low transition */
	if (m_strobe == TRUE && state == FALSE && m_busy == FALSE)
	{
		/* STROBE has gone low, data is ready */
		timer_set(attotime::zero, TIMER_BUSY, TRUE);
	}

	m_strobe = state;
}


/*-------------------------------------------------
    centronics_prime_w - initialize and reset
    printer (centronics mode)
-------------------------------------------------*/

WRITE_LINE_MEMBER(centronics_printer_image_device::input_init)
{
	/* reset printer if line is low */
	if (state == FALSE)
		device_reset();
}

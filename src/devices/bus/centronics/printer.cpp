// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "printer.h"

//**************************************************************************
//  CENTRONICS PRINTER DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(CENTRONICS_PRINTER, centronics_printer_device, "centronics_printer", "Centronics Printer")


static MACHINE_CONFIG_START( centronics_printer )
	MCFG_DEVICE_ADD("printer", PRINTER, 0)
	MCFG_PRINTER_ONLINE_CB(WRITELINE(centronics_printer_device, printer_online))
MACHINE_CONFIG_END


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/
//-------------------------------------------------
//  centronics_printer_device - constructor
//-------------------------------------------------

centronics_printer_device::centronics_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CENTRONICS_PRINTER, tag, owner, clock),
	device_centronics_peripheral_interface( mconfig, *this ),
	m_strobe(0),
	m_data(0),
	m_busy(0),
	m_printer(*this, "printer")
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
	output_perror(!state);
}

void centronics_printer_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_ACK:
		output_ack(param);

		if (param == false)
		{
			/* data is now ready, output it */
			m_printer->output(m_data);

			/* ready to receive more data, return BUSY to low */
			timer_set(attotime::from_usec(7), TIMER_BUSY, false);
		}
		break;

	case TIMER_BUSY:
		m_busy = param;
		output_busy(m_busy);

		if (param == true)
		{
			/* timer to turn ACK low to receive data */
			timer_set(attotime::from_usec(10), TIMER_ACK, false);
		}
		else
		{
			/* timer to return ACK to high state */
			timer_set(attotime::from_usec(5), TIMER_ACK, true);
		}
	}
}

void centronics_printer_device::device_start()
{
	/* register for state saving */
	save_item(NAME(m_strobe));
	save_item(NAME(m_data));
	save_item(NAME(m_busy));
}

void centronics_printer_device::device_reset()
{
	m_busy = false;
	output_busy(m_busy);
	output_fault(1);
	output_ack(1);
	output_select(1);
}

/*-------------------------------------------------
    centronics_strobe_w - signal that data is
    ready
-------------------------------------------------*/

WRITE_LINE_MEMBER( centronics_printer_device::input_strobe )
{
	/* look for a high -> low transition */
	if (m_strobe == true && state == false && m_busy == false)
	{
		/* STROBE has gone low, data is ready */
		timer_set(attotime::zero, TIMER_BUSY, true);
	}

	m_strobe = state;
}


/*-------------------------------------------------
    centronics_prime_w - initialize and reset
    printer (centronics mode)
-------------------------------------------------*/

WRITE_LINE_MEMBER(centronics_printer_device::input_init)
{
	/* reset printer if line is low */
	if (state == false)
		device_reset();
}

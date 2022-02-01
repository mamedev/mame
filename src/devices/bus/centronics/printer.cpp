// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "printer.h"

//**************************************************************************
//  CENTRONICS PRINTER DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(CENTRONICS_PRINTER, centronics_printer_device, "centronics_printer", "Centronics Printer")


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
//  device_add_mconfig - add device configuration
//-------------------------------------------------


void centronics_printer_device::device_add_mconfig(machine_config &config)
{
	PRINTER(config, m_printer, 0);
	m_printer->online_callback().set(FUNC(centronics_printer_device::printer_online));
}

/*-------------------------------------------------
    printer_online - callback that
    sets us busy when the printer goes offline
-------------------------------------------------*/

WRITE_LINE_MEMBER(centronics_printer_device::printer_online)
{
	output_perror(!state);
}

void centronics_printer_device::device_timer(emu_timer &timer, device_timer_id id, int param)
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
			m_busy_timer->adjust(attotime::from_usec(7), false);
		}
		break;

	case TIMER_BUSY:
		m_busy = param;
		output_busy(m_busy);

		if (param == true)
		{
			/* timer to turn ACK low to receive data */
			m_ack_timer->adjust(attotime::from_usec(10), false);
		}
		else
		{
			/* timer to return ACK to high state */
			m_ack_timer->adjust(attotime::from_usec(5), true);
		}
	}
}

void centronics_printer_device::device_start()
{
	m_ack_timer = timer_alloc(TIMER_ACK);
	m_busy_timer = timer_alloc(TIMER_BUSY);

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
		m_busy_timer->adjust(attotime::zero, true);
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

// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "cenluaprinter.h"
#include "screen.h"


//**************************************************************************
//  CENTRONICS PRINTER DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(CENTRONICS_LUAPRINTER, centronics_luaprinter_device, "centronics_luaprinter", "Centronics Lua Printer")


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/
//-------------------------------------------------
//  centronics_luaprinter_device - constructor
//-------------------------------------------------

centronics_luaprinter_device::centronics_luaprinter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CENTRONICS_LUAPRINTER, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	device_luaprinter_interface(mconfig, *this),
	m_strobe(0),
	m_data(0),
	m_busy(0),
	m_printer(*this, "printer"),
	m_screen(*this, "screen")
{
}
//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void centronics_luaprinter_device::device_add_mconfig(machine_config &config)
{
	PRINTER(config, m_printer, 0);
	m_printer->online_callback().set(FUNC(centronics_luaprinter_device::printer_online));

	/* video hardware (simulates paper) */
	screen_device &screen(SCREEN(config, m_screen, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(PAPER_WIDTH, PAPER_SCREEN_HEIGHT);
	screen.set_visarea(0, PAPER_WIDTH-1, 0, PAPER_SCREEN_HEIGHT-1);
	screen.set_screen_update(FUNC(device_luaprinter_interface::lp_screen_update));

}

/*-------------------------------------------------
    printer_online - callback that
    sets us busy when the printer goes offline
-------------------------------------------------*/

WRITE_LINE_MEMBER(centronics_luaprinter_device::printer_online)
{
	output_perror(!state);
}

void centronics_luaprinter_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_ACK:
		output_ack(param);

		if (param == false)
		{
			/* data is now ready, output it */
			m_printer->output(m_data);
			putnextchar(m_data);

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

void centronics_luaprinter_device::device_start()
{
	m_ack_timer = timer_alloc(TIMER_ACK);
	m_busy_timer = timer_alloc(TIMER_BUSY);

	m_bitmap.allocate(PAPER_WIDTH, PAPER_HEIGHT);
	m_bitmap.fill(0xffffff);

	/* register for state saving */
	save_item(NAME(m_strobe));
	save_item(NAME(m_data));
	save_item(NAME(m_busy));

	initluaprinter(m_bitmap);
}

void centronics_luaprinter_device::device_stop()
{
	if (m_lp_pagedirty)
		savepage();
}

void centronics_luaprinter_device::device_reset()
{
	m_busy = false;
	output_busy(m_busy);
	output_fault(1);
	output_ack(1);
	output_ack(0);
	output_select(1);
	output_perror(0); // added paper out
}


/*-------------------------------------------------
    centronics_strobe_w - signal that data is
    ready
-------------------------------------------------*/

WRITE_LINE_MEMBER( centronics_luaprinter_device::input_strobe )
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

WRITE_LINE_MEMBER(centronics_luaprinter_device::input_init)
{
	/* reset printer if line is low */
	if (state == false)
		device_reset();
}

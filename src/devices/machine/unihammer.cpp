// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Seikosha uni-hammer dot matrix print mechanism

**********************************************************************/

#include "emu.h"
#include "unihammer.h"

#include <algorithm>


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

namespace {

// The dot sensor emits a group of 27 evenly spaced pulses and then stays low
// for about 1.9 ms before the next group. Those 27 pulses make up three dot
// columns: seven dots followed by three idle pulses, three times over. So a
// group spans 30 pulse slots, of which the last three are the long gap the
// firmware resynchronises on, and one dot column takes ten slots -- the
// 5.56 ms column period.
constexpr int DOTS_PER_COLUMN = 7;      // pulses that carry a dot row
constexpr int SLOTS_PER_COLUMN = 10;    // dot rows plus three idle pulses
constexpr int COLUMNS_PER_GROUP = 3;
constexpr int PULSES_PER_GROUP = 27;    // slots that actually pulse
constexpr int SLOTS_PER_GROUP = SLOTS_PER_COLUMN * COLUMNS_PER_GROUP;

constexpr int COLUMN_PERIOD_NS = 5556000;               // 5.56 ms
constexpr attotime DOT_PULSE_PERIOD = attotime::from_nsec(COLUMN_PERIOD_NS / SLOTS_PER_COLUMN);
constexpr attotime DOT_PULSE_WIDTH = attotime::from_usec(290);

// The carrier returns home under the recovery spring in about 66 ms from
// the far end of its travel.
constexpr double RETURN_TIME_FULL = 0.066;

// 144 dpi horizontal = 2px per dot column (72 columns/inch, from 12 cpi and
// 6 dot columns per character); 126 dpi vertical = 2px per dot row and 7px
// per 1/18" line feed pulse.
constexpr int PAPER_HDPI = 144;
constexpr int PAPER_VDPI = 126;
constexpr int LF_QUANTUM = PAPER_VDPI / 18;
constexpr int DOT_COLUMNS_PER_INCH = 72;
constexpr int DOT_PX = PAPER_HDPI / DOT_COLUMNS_PER_INCH;

} // anonymous namespace



//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

DEFINE_DEVICE_TYPE(UNIHAMMER_PRINTER, unihammer_printer_device, "unihammer", "Uni-hammer Print Mechanism")


//-------------------------------------------------
//  unihammer_printer_device - constructor
//-------------------------------------------------

unihammer_printer_device::unihammer_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, UNIHAMMER_PRINTER, tag, owner, clock),
	m_bitmap_printer(*this, "printer"),
	m_dot_cb(*this),
	m_home_cb(*this)
{
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void unihammer_printer_device::device_add_mconfig(machine_config &config)
{
	BITMAP_PRINTER(config, m_bitmap_printer,
		m_paper_width_in * PAPER_HDPI, m_paper_height_in * PAPER_VDPI,
		PAPER_HDPI, PAPER_VDPI);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void unihammer_printer_device::device_start()
{
	m_dot_timer = timer_alloc(FUNC(unihammer_printer_device::dot_tick), this);
	m_return_timer = timer_alloc(FUNC(unihammer_printer_device::return_done), this);

	save_item(NAME(m_mot));
	save_item(NAME(m_hc));
	save_item(NAME(m_lfc));
	save_item(NAME(m_pin));
	save_item(NAME(m_motor_on));
	save_item(NAME(m_clutch_engaged));
	save_item(NAME(m_dot));
	save_item(NAME(m_home));
	save_item(NAME(m_slot));
	save_item(NAME(m_column));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void unihammer_printer_device::device_reset()
{
	m_mot = m_hc = m_lfc = m_pin = 1;
	m_motor_on = false;
	m_clutch_engaged = false;
	m_slot = 0;
	m_column = 0;

	m_dot_timer->reset();
	m_return_timer->reset();

	set_dot(0);
	set_home(1);
}


//-------------------------------------------------
//  mot_w - _MOT, motor drive
//-------------------------------------------------

void unihammer_printer_device::mot_w(int state)
{
	if (m_mot == state)
		return;

	m_mot = state;

	if (!state)
	{
		// motor starts turning: the platen, the hammer cam and the dot
		// sensor all run off the same shaft, so this is what starts the
		// dot pulse train
		m_motor_on = true;
		m_slot = 0;
		start_slot();
	}
	else
	{
		// motor stops: no more rotation, so no more dot pulses, and an
		// engaged carrier just stops wherever it is
		m_motor_on = false;
		m_dot_timer->reset();
		set_dot(0);
	}
}


//-------------------------------------------------
//  hc_w - _HC, H solenoid / jaw clutch
//-------------------------------------------------

void unihammer_printer_device::hc_w(int state)
{
	if (m_hc == state)
		return;

	m_hc = state;

	if (state)
	{
		// solenoid released: the jaw clutch couples the carrier to the
		// motor shaft again, so it starts advancing with the dot columns
		m_clutch_engaged = true;
		m_return_timer->reset();
	}
	else
	{
		// solenoid pulled: the clutch is released and the recovery spring
		// drags the carrier back to the home position
		m_clutch_engaged = false;

		attotime const travel = attotime::from_double(
			RETURN_TIME_FULL * double(m_column) / double(m_paper_width_in * DOT_COLUMNS_PER_INCH));
		m_return_timer->adjust(travel);
	}
}


//-------------------------------------------------
//  lfc_w - _LFC, line feed solenoid
//-------------------------------------------------

void unihammer_printer_device::lfc_w(int state)
{
	if (m_lfc == state)
		return;

	m_lfc = state;

	if (!state)
		do_linefeed();
}


//-------------------------------------------------
//  pin_w - _PIN, hammer solenoid
//-------------------------------------------------

void unihammer_printer_device::pin_w(int state)
{
	if (m_pin == state)
		return;

	m_pin = state;

	if (!state)
		print_dot();
}


//-------------------------------------------------
//  start_slot - begin a pulse slot, stepping the
//  carrier at each column boundary
//-------------------------------------------------

void unihammer_printer_device::start_slot()
{
	if ((m_slot % SLOTS_PER_COLUMN) == 0 && m_clutch_engaged)
	{
		// the carrier is geared to the same shaft, so it steps exactly one
		// dot column per ten pulse slots
		m_column++;
		set_home(0);
	}

	if (m_slot < PULSES_PER_GROUP)
	{
		set_dot(1);
		m_dot_timer->adjust(DOT_PULSE_WIDTH);
	}
	else
	{
		// the tail of the group: three silent slots make up the long low
		// period that tells the firmware a new group is about to start
		set_dot(0);
		m_dot_timer->adjust(DOT_PULSE_PERIOD);
	}
}


//-------------------------------------------------
//  dot_tick - drive the dot sensor waveform
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(unihammer_printer_device::dot_tick)
{
	if (!m_motor_on)
		return;

	if (m_dot)
	{
		// end of a pulse: idle for the rest of this slot
		set_dot(0);
		m_dot_timer->adjust(DOT_PULSE_PERIOD - DOT_PULSE_WIDTH);
	}
	else
	{
		m_slot = (m_slot + 1) % SLOTS_PER_GROUP;
		start_slot();
	}
}


//-------------------------------------------------
//  return_done - carrier has reached home
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(unihammer_printer_device::return_done)
{
	m_column = 0;
	set_home(1);
}


//-------------------------------------------------
//  set_dot / set_home - sensor outputs
//-------------------------------------------------

void unihammer_printer_device::set_dot(int state)
{
	if (m_dot != state)
	{
		m_dot = state;
		m_dot_cb(state);
	}
}

void unihammer_printer_device::set_home(int state)
{
	if (m_home != state)
	{
		m_home = state;
		m_home_cb(state);
	}
}


//-------------------------------------------------
//  print_dot - strike the hammer at the current
//  carrier column and dot row
//-------------------------------------------------

void unihammer_printer_device::print_dot()
{
	// the bitmap printer treats everything ahead of its clear frontier as
	// blank paper and wipes it on the next feed, so make sure the frontier
	// has passed the rows about to be struck before drawing into them
	m_bitmap_printer->check_new_page();

	int const row = m_slot % SLOTS_PER_COLUMN;
	if (row >= DOTS_PER_COLUMN)
		return; // one of the three idle pulses between columns, no dot row

	int const x = m_column * DOT_PX;
	int const y = m_bitmap_printer->m_ypos + row * DOT_PX;

	for (int dy = 0; dy < DOT_PX; dy++)
		for (int dx = 0; dx < DOT_PX; dx++)
			m_bitmap_printer->draw_pixel(x + dx, y + dy, 0x000000);

	m_bitmap_printer->setheadpos(x, m_bitmap_printer->m_ypos);
}


//-------------------------------------------------
//  do_linefeed - advance the paper by one 1/18"
//  line feed pulse
//-------------------------------------------------

void unihammer_printer_device::do_linefeed()
{
	m_bitmap_printer->setheadpos(m_bitmap_printer->m_xpos, m_bitmap_printer->m_ypos + LF_QUANTUM);
	m_bitmap_printer->check_new_page();
}

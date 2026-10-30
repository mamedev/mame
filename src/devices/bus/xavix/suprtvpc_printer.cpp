// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Epoch Super TV-PC dedicated thermal printer. Receives inverted, LSB-first
    serial data in 49-byte packets and saves completed pages as PNG images.
    A page starts with 00 48 3d, a little-endian row count and zero padding.
    Each raster packet contains 00 and 384 pixels, leftmost first; one is black.

    TODO:
    - Dump the printer controller and establish the paper controls.
    - Measure packet-ready pulse duration and the cancelled-transfer
      timeout.

***************************************************************************/

#include "emu.h"
#include "suprtvpc_printer.h"

#include "emuopts.h"
#include "fileio.h"
#include "video.h"

#include "png.h"

#include <algorithm>
#include <iterator>

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SUPRTVPC_PRINTER, suprtvpc_printer_device, "suprtvpc_printer", "Super TV-PC Printer")

suprtvpc_printer_device::suprtvpc_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SUPRTVPC_PRINTER, tag, owner, clock)
	, device_suprtvpc_exp_interface(mconfig, *this)
{
}

void suprtvpc_printer_device::device_start()
{
	m_busy_timer = timer_alloc(FUNC(suprtvpc_printer_device::ready), this);
	m_idle_timer = timer_alloc(FUNC(suprtvpc_printer_device::abort_page), this);
	// Job heights are 16-bit.  Packed monochrome rows preserve unfinished jobs
	// in saves without the fixed RGB32 paper buffer used by bitmap_printer_device.
	m_page = std::make_unique<u8[]>(ROW_BYTES * MAX_ROWS);

	save_pointer(NAME(m_page), ROW_BYTES * MAX_ROWS);
	save_item(NAME(m_packet));
	save_item(NAME(m_bit_count));
	save_item(NAME(m_height));
	save_item(NAME(m_row));
	save_item(NAME(m_data));
	save_item(NAME(m_clock));
	save_item(NAME(m_busy));
}

void suprtvpc_printer_device::device_reset()
{
	m_busy_timer->adjust(attotime::never);
	m_idle_timer->adjust(attotime::never);
	std::fill_n(m_page.get(), ROW_BYTES * MAX_ROWS, 0);
	std::fill(std::begin(m_packet), std::end(m_packet), 0);
	m_bit_count = 0;
	m_height = 0;
	m_row = 0;
	m_data = false;
	m_clock = false;
	m_busy = false;
}

void suprtvpc_printer_device::data_w(int state)
{
	if (bool(state) != m_data)
		m_idle_timer->adjust(state ? attotime::never : attotime::from_msec(20));
	m_data = bool(state);
}

int suprtvpc_printer_device::ready_r()
{
	return !m_busy;
}

void suprtvpc_printer_device::clock_w(int state)
{
	if (state && !m_clock && !m_busy)
	{
		m_idle_timer->adjust(m_data ? attotime::never : attotime::from_msec(20));
		m_packet[m_bit_count / 8] |= u8(!m_data) << (m_bit_count % 8);
		if (++m_bit_count == std::size(m_packet) * 8)
		{
			receive_packet();
			m_bit_count = 0;
			std::fill(std::begin(m_packet), std::end(m_packet), 0);
			m_busy = true;
			// A low ready pulse acknowledges each packet.  Its physical duration is unmeasured.
			m_busy_timer->adjust(attotime::from_msec(1));
		}
	}
	m_clock = bool(state);
}

TIMER_CALLBACK_MEMBER(suprtvpc_printer_device::ready)
{
	m_busy = false;
}

TIMER_CALLBACK_MEMBER(suprtvpc_printer_device::abort_page)
{
	// The host ends cancelled transfers with data held low, including midway through
	// a byte.  Normal inter-row gaps are under 2 ms; the actual timeout is unknown.
	clear_rows();
	std::fill(std::begin(m_packet), std::end(m_packet), 0);
	m_bit_count = 0;
	m_height = 0;
	m_row = 0;
}

void suprtvpc_printer_device::receive_packet()
{
	if (!m_height)
	{
		if (!m_packet[0] && m_packet[1] == 0x48 && m_packet[2] == 0x3d
			&& std::all_of(std::begin(m_packet) + 5, std::end(m_packet), [] (u8 data) { return !data; }))
		{
			m_height = u16(m_packet[3]) | (u16(m_packet[4]) << 8);
			m_row = 0;
			LOG("Page height %u\n", m_height);
		}
		else
			LOG("Unknown page header\n");
	}
	else if (!m_packet[0])
	{
		std::copy_n(m_packet + 1, ROW_BYTES, m_page.get() + m_row * ROW_BYTES);
		if (++m_row == m_height)
		{
			print_page();
			clear_rows();
			m_height = 0;
		}
	}
	else
	{
		LOG("Unknown raster command %02x\n", m_packet[0]);
		clear_rows();
		m_height = 0;
	}
}

void suprtvpc_printer_device::clear_rows()
{
	std::fill_n(m_page.get(), m_row * ROW_BYTES, 0);
}

void suprtvpc_printer_device::print_page()
{
	bitmap_ind16 bitmap(ROW_BYTES * 8, m_height);
	for (unsigned y = 0; y < m_height; ++y)
	{
		for (unsigned x = 0; x < ROW_BYTES * 8; ++x)
			bitmap.pix(y, x) = BIT(m_page[y * ROW_BYTES + x / 8], x % 8);
	}

	emu_file file(machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	std::error_condition error = machine().video().open_next(file, "png");
	if (!error)
		error = util::png_write_bitmap(file, nullptr, bitmap, std::size(PALETTE), PALETTE);
	if (error)
		machine().popmessage("Unable to save printout: %s", error.message());
	else
		machine().popmessage("Printout saved to %s", file.fullpath());
}

// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    µPD7004

    10-bit 8 Channel A/D Converter

    TODO:
    - Serial modes
    - 2s complement output

***************************************************************************/

#include "emu.h"
#include "upd7004.h"

#define VERBOSE 0
#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(UPD7004, upd7004_device, "upd7004", "UPD7004 A/D Converter")

// permit our enum to be saved
ALLOW_SAVE_TYPE(upd7004_device::state);


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  upd7004_device - constructor
//-------------------------------------------------

upd7004_device::upd7004_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, UPD7004, tag, owner, clock),
	m_eoc_cb(*this), m_eoc_ff_cb(*this),
	m_in_cb(*this),
	m_state(STATE_IDLE),
	m_cycle_timer(nullptr),
	m_div(1), m_code(false), m_address(0), m_sar(0x3ff)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd7004_device::device_start()
{
	// resolve callbacks
	m_eoc_cb.resolve_safe();
	m_eoc_ff_cb.resolve_safe();
	m_in_cb.resolve_all_safe(0x3ff);

	// allocate timers
	m_cycle_timer = timer_alloc(FUNC(upd7004_device::update_state), this);
	m_cycle_timer->adjust(attotime::never);

	// register for save states
	save_item(NAME(m_state));
	save_item(NAME(m_div));
	save_item(NAME(m_code));
	save_item(NAME(m_address));
	save_item(NAME(m_sar));
}

//-------------------------------------------------
//  update_state -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(upd7004_device::update_state)
{
	switch (m_state)
	{
		case STATE_IDLE:
			m_cycle_timer->adjust(attotime::never);
			break;

		case STATE_CONVERSION_START:
			m_sar = m_in_cb[m_address](0);

			// the conversion takes 96 to 104 cycles
			m_state = STATE_CONVERSION_DONE;
			m_cycle_timer->adjust(attotime::from_ticks(100, clock() / m_div));

			break;

		case STATE_CONVERSION_DONE:
		{
			// set eoc
			m_eoc_cb(1);
			m_eoc_ff_cb(1);

			m_state = STATE_IDLE;
			m_cycle_timer->adjust(attotime::never);

			LOG("Conversion finished, result: %03x\n", m_sar);

			break;
		}
	}
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

uint8_t upd7004_device::read(offs_t offset)
{
	uint8_t data = 0xff;

	// connected to a flip-flop, cleared on read
	m_eoc_ff_cb(0);

	switch (offset)
	{
	case 0:
		// low-byte data output
		data = (m_sar & 0x03) << 6;
		break;

	case 1:
		// high-byte data output
		data = (m_sar >> 2) & 0xff;
		break;
	}

	return data;
}

void upd7004_device::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		// address set
		m_address = data & 0x07;

		// reset eoc
		m_eoc_cb(0);

		// start conversion timer
		m_state = STATE_CONVERSION_START;
		m_cycle_timer->adjust(attotime::from_hz(clock() / m_div));

		LOG("Select address %d\n", m_address);

		break;

	case 1:
		// initialize
		m_div = 1 << (data & 0x03);
		m_code = data & 0x04;

		logerror("Initialize with clock division ratio %d, %s data\n", m_div, m_code ? "2s complement" : "binary");

		if (m_code)
			logerror("Warning: 2s complement data unimplemented\n");

		break;
	}
}

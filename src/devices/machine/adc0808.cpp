// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    ADC0808/ADC0809

    A/D Converter with 8 Channel-Multiplexer

***************************************************************************/

#include "emu.h"
#include "adc0808.h"


//**************************************************************************
//  CONSTANTS/MACROS
//**************************************************************************

#define VERBOSE 0


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ADC0808, adc0808_device, "adc0808", "ADC0808 A/D Converter")
DEFINE_DEVICE_TYPE(ADC0809, adc0809_device, "adc0809", "ADC0809 A/D Converter")
DEFINE_DEVICE_TYPE(M58990, m58990_device, "m58990", "M58990 A/D Converter")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// permit our enum to be saved
ALLOW_SAVE_TYPE(adc0808_device::state);

//-------------------------------------------------
//  adc0808_device - constructor
//-------------------------------------------------

adc0808_device::adc0808_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_eoc_cb(*this), m_eoc_ff_cb(*this),
	m_in_cb(*this),
	m_state(STATE_IDLE),
	m_cycle_timer(nullptr),
	m_start(0), m_address(0), m_sar(0xff), m_eoc(1)
{
}

adc0808_device::adc0808_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	adc0808_device(mconfig, ADC0808, tag, owner, clock)
{
}

//-------------------------------------------------
//  adc0809_device - constructor
//-------------------------------------------------

adc0809_device::adc0809_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	adc0808_device(mconfig, ADC0809, tag, owner, clock)
{
}

//-------------------------------------------------
//  m58990_device - constructor
//-------------------------------------------------

m58990_device::m58990_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	adc0808_device(mconfig, M58990, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adc0808_device::device_start()
{
	// resolve callbacks
	m_eoc_cb.resolve_safe();
	m_eoc_ff_cb.resolve_safe();
	m_in_cb.resolve_all_safe(0xff);

	// allocate timers
	m_cycle_timer = timer_alloc(FUNC(adc0808_device::update_state), this);
	m_cycle_timer->adjust(attotime::zero, 0, attotime::from_hz(clock()));

	// register for save states
	save_item(NAME(m_state));
	save_item(NAME(m_start));
	save_item(NAME(m_address));
	save_item(NAME(m_sar));
	save_item(NAME(m_eoc));
}

//-------------------------------------------------
//  update_state
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(adc0808_device::update_state)
{
	switch (m_state)
	{
		case STATE_IDLE:
			m_cycle_timer->adjust(attotime::never);
			return;

		// ready; beginning to run conversion cycle
		case STATE_CONVERSION_READY:
			m_state = STATE_CONVERSION_RUNNING;

			m_sar = m_in_cb[m_address](0);
			m_eoc = 0;
			m_eoc_cb(m_eoc);

			// the conversion takes 8 steps per 8 cycles
			m_cycle_timer->adjust(attotime::from_ticks(64, clock()));
			return;

		// start; mark ourselves as ready for conversion 1 cycle later
		case STATE_CONVERSION_START:
			m_state = STATE_CONVERSION_READY;
			m_cycle_timer->adjust(attotime::from_hz(clock()));
			return;

		// end of conversion cycle
		case STATE_CONVERSION_RUNNING:
		{
			m_state = STATE_IDLE;
			const uint8_t start_sar = m_sar;
			m_sar = m_in_cb[m_address](0);

			m_eoc = 1;
			m_eoc_cb(m_eoc);
			m_eoc_ff_cb(1);

			if (VERBOSE)
				logerror("Conversion finished, result %02x\n", m_sar);

			if (m_sar != start_sar)
				logerror("Conversion finished, should fail - starting value %02x, ending value %02x", start_sar, m_sar);

			// eoc is delayed by one cycle
			m_cycle_timer->adjust(attotime::never);
			break;
		}
	}
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

u8 adc0808_device::data_r()
{
	if (!machine().side_effects_disabled())
	{
		if (VERBOSE)
			logerror("data_r: %02x\n", m_sar);

		// oe connected to flip-flop clear
		m_eoc_ff_cb(0);
	}

	return m_sar;
}

void adc0808_device::address_w(u8 data)
{
	m_address = data & 7;
}

WRITE_LINE_MEMBER( adc0808_device::start_w )
{
	if (m_start == state)
		return;

	if (state && !m_start)
	{
		m_state = STATE_CONVERSION_START;
		m_cycle_timer->adjust(attotime::from_hz(clock()));
	}
	else if (!state && m_start)
	{
		m_cycle_timer->adjust(attotime::from_hz(clock()));
	}

	m_start = state;
}

READ_LINE_MEMBER( adc0808_device::eoc_r )
{
	return m_eoc;
}

void adc0808_device::address_offset_start_w(offs_t offset, u8 data)
{
	if (VERBOSE)
		logerror("address_offset_start_w %02x %02x\n", offset, data);

	start_w(1);
	address_w(offset);
	start_w(0);
}

void adc0808_device::address_data_start_w(u8 data)
{
	if (VERBOSE)
		logerror("address_data_start_w %02x\n", data);

	start_w(1);
	address_w(data);
	start_w(0);
}

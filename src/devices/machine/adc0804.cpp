// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    National Semiconductor ADC0804 8-bit µP Compatible A/D Converter

    The ADC0804 is the most widely used member of a family of five
    CMOS 8-bit ADCs with microprocessor-compatible interfaces, which
    differ from each other only in error specifications:

      ADC0801       ±¼ LSB      Full-scale adjusted
      ADC0802       ±½ LSB      Vref/2 = 2.500 Vdc
      ADC0803       ±½ LSB      Full-scale adjusted
      ADC0804       ±1 LSB      Vref/2 = 2.500 Vdc
      ADC0805       ±1 LSB      Vref/2 = NC

    Though these devices are designed to convert differential analog
    inputs, single-phase conversion can be achieved by tying pin 7
    (negative Vin) to pin 8 (analog GND). A CD4051B or other analog
    switch IC is often used to multiplex several input sources.

    Pin 4 should be connected to either an externally generated clock
    signal (640 kHz typical, 1460 kHz maximum) or a timing capacitor
    plus a resistor bridged to pin 19. The resistor is typically 10 kΩ
    but higher values may be needed due to loading conditions when
    multiple other devices are also clocked through pin 19, which
    yields the output of the internal Schmitt trigger clock circuit.

    The ADC0804's INTR semaphore output reflects not the actual busy
    state of the converter (as EOC does on the ADC0808) but an
    internal flip-flop attached to it. This flip-flop is reset by
    pulling either RD or WR active low; therefore, INTR will never
    toggle if RD is tied to GND to keep the data outputs continuously
    active.

**********************************************************************/

#include "emu.h"
#include "adc0804.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ADC0803, adc0803_device, "adc0803", "ADC0803 A/D Converter")
DEFINE_DEVICE_TYPE(ADC0804, adc0804_device, "adc0804", "ADC0804 A/D Converter")

ALLOW_SAVE_TYPE(adc0804_device::read_mode);


//**************************************************************************
//  DEVICE CONSTRUCTION AND INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  adc0804_device - constructor
//-------------------------------------------------

adc0804_device::adc0804_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_vin_callback(*this)
	, m_intr_callback(*this)
	, m_res(0.0)
	, m_cap(0.0)
	, m_fclk_rc(attotime::zero)
	, m_timer(nullptr)
	, m_rd_mode(RD_STROBED)
	, m_rd_active(false)
	, m_wr_active(false)
	, m_result(0)
	, m_intr_active(false)
{
}

adc0804_device::adc0804_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: adc0804_device(mconfig, ADC0804, tag, owner, clock)
{
}


//-------------------------------------------------
//  adc0803_device - constructor
//-------------------------------------------------

adc0803_device::adc0803_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: adc0804_device(mconfig, ADC0803, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void adc0804_device::device_resolve_objects()
{
	m_vin_callback.resolve_safe(0);
	m_intr_callback.resolve_safe();

	if (m_rd_mode == RD_GROUNDED)
		m_rd_active = true;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adc0804_device::device_start()
{
	// calculate RC timing
	if (m_res == 0.0 || m_cap == 0.0)
		m_fclk_rc = attotime::zero;
	else
		m_fclk_rc = attotime::from_double(m_res * m_cap / 1.1);

	// create timer
	m_timer = timer_alloc(FUNC(adc0804_device::conversion_done), this);

	// save state
	if (m_rd_mode == RD_BITBANGED)
		save_item(NAME(m_rd_active));
	save_item(NAME(m_wr_active));
	save_item(NAME(m_result));
	save_item(NAME(m_intr_active));
}


//**************************************************************************
//  CONVERSION SEQUENCE
//**************************************************************************

// Conversion occurs over a busy period of 66–73 cycles, after 1–8 cycles of latency
const int adc0804_device::s_conversion_cycles = 74;


//-------------------------------------------------
//  set_interrupt - set or clear INTR output
//-------------------------------------------------

void adc0804_device::set_interrupt(bool state)
{
	if (m_intr_active != state)
	{
		m_intr_active = state;
		m_intr_callback(state ? ASSERT_LINE : CLEAR_LINE);
	}
}


//-------------------------------------------------
//  conversion_start - begin the busy period
//-------------------------------------------------

void adc0804_device::conversion_start()
{
	if (!m_timer->enabled())
		m_timer->adjust(m_fclk_rc != attotime::zero ? m_fclk_rc * s_conversion_cycles : clocks_to_attotime(s_conversion_cycles));
	else
		logerror("%s: Tried to start conversion when already in progress\n", machine().describe_context());
}


//-------------------------------------------------
//  conversion_done - load result and signal
//  interrupt
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(adc0804_device::conversion_done)
{
	m_result = m_vin_callback();

	if (!m_rd_active && !m_wr_active)
		set_interrupt(true);
}


//**************************************************************************
//  BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  read - read the 8-bit conversion result
//-------------------------------------------------

u8 adc0804_device::read()
{
	switch (m_rd_mode)
	{
	case RD_STROBED:
		if (!machine().side_effects_disabled())
			set_interrupt(false);
		break;

	case RD_BITBANGED:
		if (!m_rd_active)
			return 0xff; // open bus
		break;

	case RD_GROUNDED:
		break;
	}

	return m_result;
}


//-------------------------------------------------
//  write - begin a new conversion
//-------------------------------------------------

void adc0804_device::write(u8 data)
{
	// data is ignored
	set_interrupt(false);
	conversion_start();
}


//-------------------------------------------------
//  read_and_write - read the result of the last
//  conversion and begin a new one
//-------------------------------------------------

u8 adc0804_device::read_and_write()
{
	if (!machine().side_effects_disabled())
	{
		set_interrupt(false);
		conversion_start();
	}

	if (m_rd_mode == RD_BITBANGED && !m_rd_active)
		return 0xff; // open bus
	else
		return m_result;
}


//-------------------------------------------------
//  rd_w - enable data bus by line write
//-------------------------------------------------

WRITE_LINE_MEMBER(adc0804_device::rd_w)
{
	assert(m_rd_mode == RD_BITBANGED);

	// RD input is active low
	if (!state && !m_rd_active)
	{
		m_rd_active = true;
		set_interrupt(false);
	}
	else if (state && m_rd_active)
		m_rd_active = false;
}


//-------------------------------------------------
//  wr_w - begin conversion by line write
//-------------------------------------------------

WRITE_LINE_MEMBER(adc0804_device::wr_w)
{
	// WR input is active low
	if (!state && !m_wr_active)
	{
		m_wr_active = true;
		set_interrupt(false);
	}
	else if (state && m_wr_active)
	{
		m_wr_active = false;
		conversion_start();
	}
}

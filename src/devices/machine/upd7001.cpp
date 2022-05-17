// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    NEC µPD7001 CMOS Serial I/O Analog-to-Digital Converter

    This ADC takes 56 clock cycles to convert one of its four
    multiplexed input channels by successive approximation. The
    8-bit result is shifted out MSB first on the SO line. (SO and
    EOC are open-drain outputs, making them convenient to tie
    together since CS disables EOC but enables SO.) If only one
    input is used, it may be connected to A0, with DL tied to Vdd
    and SI to Vss.

    The internal clock (fCK) is normally generated from a resistor
    bridging the CL0 and CL1 pins and a capacitor between CL1 and
    ground. The typical values R = 27 kΩ and C = 47 pF produce an
    operating frequency of about 400 kHz. (500 kHz is the maximum
    according to the datasheet.)

*********************************************************************/

#include "emu.h"
#include "upd7001.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(UPD7001, upd7001_device, "upd7001", "NEC uPD7001 A/D Converter")

//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  upd7001_device - constructor
//-------------------------------------------------

upd7001_device::upd7001_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, UPD7001, tag, owner, clock)
	, m_an_callback(*this)
	, m_eoc_callback(*this)
	, m_res(0.0)
	, m_cap(0.0)
	, m_fck_rc(attotime::attotime::zero)
	, m_conv_timer(nullptr)
	, m_scsk_timer(nullptr)
	, m_cs_active(false)
	, m_eoc_active(false)
	, m_oe(false)
	, m_sck(true)
	, m_si(true)
	, m_so(false)
	, m_dl(false)
	, m_sr(0)
	, m_mpx(0)
{
}


//-------------------------------------------------
//  device_resolve_objects -
//-------------------------------------------------

void upd7001_device::device_resolve_objects()
{
	// resolve callbacks
	m_an_callback.resolve_all_safe(0);
	m_eoc_callback.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd7001_device::device_start()
{
	// calculate RC timing (FIXME: math is not very accurate)
	if (m_res == 0.0 || m_cap == 0.0)
		m_fck_rc = attotime::zero;
	else
		m_fck_rc = attotime::from_double(m_res * m_cap * 1.97);

	// initialize timers
	m_conv_timer = timer_alloc(FUNC(upd7001_device::conversion_done), this);
	m_scsk_timer = timer_alloc(FUNC(upd7001_device::output_enabled), this);

	// save state
	save_item(NAME(m_cs_active));
	save_item(NAME(m_eoc_active));
	save_item(NAME(m_oe));
	save_item(NAME(m_sck));
	save_item(NAME(m_si));
	save_item(NAME(m_so));
	save_item(NAME(m_dl));
	save_item(NAME(m_sr));
	save_item(NAME(m_mpx));
}


//-------------------------------------------------
//  cs_w - active-low chip select
//-------------------------------------------------

WRITE_LINE_MEMBER(upd7001_device::cs_w)
{
	if (!state && !m_cs_active)
	{
		if (m_conv_timer->enabled())
		{
			logerror("%s: CS lowered while conversion still in progress\n", machine().describe_context());
			m_conv_timer->enable(false);
		}

		// EOC deasserted on falling edge
		m_eoc_active = false;
		m_eoc_callback(1);

		// Output is enabled after 5 internal clock pulses
		m_scsk_timer->adjust(m_fck_rc != attotime::zero ? m_fck_rc * 5 : clocks_to_attotime(5));
	}
	else if (state && m_cs_active)
	{
		// Disable data output
		m_oe = false;
		m_scsk_timer->enable(false);

		// Begin conversion on rising edge
		m_conv_timer->adjust(m_fck_rc != attotime::zero ? m_fck_rc * 56 : clocks_to_attotime(56));
	}

	m_cs_active = !state;
}


//-------------------------------------------------
//  conversion_done - finish converting the
//  selected analog input
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(upd7001_device::conversion_done)
{
	// Load conversion result into shift register
	m_sr = m_an_callback[m_mpx]();

	// Activate EOC output
	m_eoc_active = true;
	m_eoc_callback(0);
}


//-------------------------------------------------
//  output_enabled - enable data output on SO
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(upd7001_device::output_enabled)
{
	m_oe = true;
}


//-------------------------------------------------
//  sck_w - shift data out of and into register
//-------------------------------------------------

WRITE_LINE_MEMBER(upd7001_device::sck_w)
{
	if (m_cs_active)
	{
		if (!state && m_sck)
		{
			if (m_scsk_timer->enabled() && m_scsk_timer->remaining() > attotime::zero)
				logerror("%s: SCK lowered %.2f microseconds too early\n", machine().describe_context(), m_scsk_timer->remaining().as_double() * 1.0E+6);

			// SO updates on falling edge
			m_so = BIT(m_sr, 7);
		}
		else if (state && !m_sck)
		{
			// SI shifted in on rising edge
			m_sr = (m_sr << 1) | m_si;
			if (m_dl)
				m_mpx = m_sr & 3;
		}
	}

	m_sck = state;
}


//-------------------------------------------------
//  dl_w - latch input data for multiplexer to
//  select input to convert
//-------------------------------------------------

WRITE_LINE_MEMBER(upd7001_device::dl_w)
{
	if (state && !m_dl && m_cs_active)
		m_mpx = m_sr & 3;

	m_dl = state;
}

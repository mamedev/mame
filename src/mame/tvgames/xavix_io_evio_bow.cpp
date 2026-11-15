// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    eVio violin bow sensor interface. Samples host movement and generates
    quadrature phases for the console's two optical-sensor inputs, retaining
    direction and unconsumed movement between samples.

***************************************************************************/

#include "emu.h"
#include "xavix_io_evio_bow.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(XAVIX_EVIO_BOW, xavix_evio_bow_device, "xavix_evio_bow", "XaviX eVio Bow Sensor")

xavix_evio_bow_device::xavix_evio_bow_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_EVIO_BOW, tag, owner, clock)
	, m_motion(*this, "BOW")
	, m_player(0)
{
}

static INPUT_PORTS_START( xavix_evio_bow )
	PORT_START("BOW")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_NAME("P1 Bow") PORT_SENSITIVITY(16) PORT_KEYDELTA(8) PORT_CODE(MOUSECODE_X_INDEXED(0)) PORT_CODE_DEC(INPUT_CODE_INVALID) PORT_CODE_INC(INPUT_CODE_INVALID) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_evio_bow2 )
	PORT_INCLUDE(xavix_evio_bow)

	PORT_MODIFY("BOW")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_NAME("P2 Bow") PORT_SENSITIVITY(16) PORT_KEYDELTA(8) PORT_CODE(MOUSECODE_X_INDEXED(1)) PORT_CODE_DEC(INPUT_CODE_INVALID) PORT_CODE_INC(INPUT_CODE_INVALID) PORT_PLAYER(2)
INPUT_PORTS_END

ioport_constructor xavix_evio_bow_device::device_input_ports() const
{
	return m_player ? INPUT_PORTS_NAME(xavix_evio_bow2) : INPUT_PORTS_NAME(xavix_evio_bow);
}

void xavix_evio_bow_device::device_start()
{
	assert(m_player < 2);
	m_sample_timer = timer_alloc(FUNC(xavix_evio_bow_device::sample_input), this);
	m_phase_timer = timer_alloc(FUNC(xavix_evio_bow_device::step_phase), this);

	save_item(NAME(m_pending));
	save_item(NAME(m_last));
	save_item(NAME(m_phase));
}

void xavix_evio_bow_device::device_reset()
{
	m_pending = 0;
	m_last = m_motion->read();
	m_phase = 0;
	m_sample_timer->adjust(attotime::from_hz(1'000), 0, attotime::from_hz(1'000));
	m_phase_timer->adjust(attotime::from_usec(250), 0, attotime::from_usec(250));
}

TIMER_CALLBACK_MEMBER(xavix_evio_bow_device::sample_input)
{
	u8 const current = m_motion->read();
	s16 const delta = s8(current - m_last);
	m_last = current;
	m_pending = std::clamp<s16>(m_pending + delta, -255, 255);
}

TIMER_CALLBACK_MEMBER(xavix_evio_bow_device::step_phase)
{
	if (m_pending > 0)
	{
		m_phase = (m_phase + 1) & 3;
		m_pending--;
	}
	else if (m_pending < 0)
	{
		m_phase = (m_phase - 1) & 3;
		m_pending++;
	}
}

u8 xavix_evio_bow_device::quadrature_r() const
{
	// The two optical sensors produce the firmware's packed A/B Gray sequence.
	return GRAY[m_phase];
}

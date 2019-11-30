// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/******************************************************************************
* FILE
*   Yamaha 3812 emulator interface - MAME VERSION
*
* CREATED BY
*   Ernesto Corvi
*
* UPDATE LOG
*   JB  28-04-2002  Fixed simultaneous usage of all three different chip types.
*                       Used real sample rate when resample filter is active.
*       AAT 12-28-2001  Protected Y8950 from accessing unmapped port and keyboard handlers.
*   CHS 1999-01-09  Fixes new ym3812 emulation interface.
*   CHS 1998-10-23  Mame streaming sound chip update
*   EC  1998        Created Interface
*
* NOTES
*
******************************************************************************/
#include "emu.h"
#include "3812intf.h"
#include "sound/fmopl.h"


void ym3812_device::irq_handler(int irq)
{
	m_timer[2]->adjust(attotime::zero, irq);
}

/* Timer overflow callback from timer.c */
void ym3812_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_A:
		ym3812_timer_over(m_chip,0);
		break;

	case TIMER_B:
		ym3812_timer_over(m_chip,1);
		break;

	case TIMER_IRQ_SYNC:
		if (!m_irq_handler.isnull())
			m_irq_handler(param);
		break;
	}
}

void ym3812_device::timer_handler(int c, const attotime &period)
{
	if( period == attotime::zero )
	{   /* Reset FM Timer */
		m_timer[c]->enable(false);
	}
	else
	{   /* Start FM Timer */
		m_timer[c]->adjust(period);
	}
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ym3812_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	ym3812_update_one(m_chip, outputs[0], samples);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ym3812_device::device_start()
{
	int rate = clock() / 72;

	m_irq_handler.resolve();

	/* stream system initialize */
	m_chip = ym3812_init(this, clock(), rate);
	if (!m_chip)
		throw emu_fatalerror("ym3812_device(%s): Error creating YM3812 chip", tag());

	calculate_rates();

	/* YM3812 setup */
	ym3812_set_timer_handler (m_chip, ym3812_device::static_timer_handler, this);
	ym3812_set_irq_handler   (m_chip, ym3812_device::static_irq_handler, this);
	ym3812_set_update_handler(m_chip, ym3812_device::static_update_request, this);

	m_timer[0] = timer_alloc(TIMER_A);
	m_timer[1] = timer_alloc(TIMER_B);
	m_timer[2] = timer_alloc(TIMER_IRQ_SYNC);
}

void ym3812_device::device_clock_changed()
{
	calculate_rates();
	ym3812_clock_changed(m_chip, clock(), clock() / 72);
}

void ym3812_device::calculate_rates()
{
	int rate = clock() / 72;

	if (m_stream != nullptr)
		m_stream->set_sample_rate(rate);
	else
		m_stream = machine().sound().stream_alloc(*this, 0, 1, rate);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ym3812_device::device_stop()
{
	ym3812_shutdown(m_chip);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ym3812_device::device_reset()
{
	ym3812_reset_chip(m_chip);
}


u8 ym3812_device::read(offs_t offset)
{
	return ym3812_read(m_chip, offset & 1);
}

void ym3812_device::write(offs_t offset, u8 data)
{
	ym3812_write(m_chip, offset & 1, data);
}

u8 ym3812_device::status_port_r() { return read(0); }
u8 ym3812_device::read_port_r() { return read(1); }
void ym3812_device::control_port_w(u8 data) { write(0, data); }
void ym3812_device::write_port_w(u8 data) { write(1, data); }


DEFINE_DEVICE_TYPE(YM3812, ym3812_device, "ym3812", "YM3812 OPL2")

ym3812_device::ym3812_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, YM3812, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_timer{ nullptr, nullptr }
	, m_chip(nullptr)
	, m_irq_handler(*this)
{
}

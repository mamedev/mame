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
#include "3526intf.h"
#include "fmopl.h"


/* IRQ Handler */
void ym3526_device::irq_handler(int irq)
{
	if (!m_irq_handler.isnull())
		m_irq_handler(irq);
}

/* Timer overflow callback from timer.c */
void ym3526_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case 0:
		ym3526_timer_over(m_chip,0);
		break;

	case 1:
		ym3526_timer_over(m_chip,1);
		break;
	}
}

void ym3526_device::timer_handler(int c,const attotime &period)
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

void ym3526_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	ym3526_update_one(m_chip, outputs[0], samples);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ym3526_device::device_start()
{
	int rate = clock() / 72;

	// resolve callbacks
	m_irq_handler.resolve();

	/* stream system initialize */
	m_chip = ym3526_init(this, clock(), rate);
	assert_always(m_chip != nullptr, "Error creating YM3526 chip");

	calculate_rates();

	/* YM3526 setup */
	ym3526_set_timer_handler (m_chip, &ym3526_device::static_timer_handler, this);
	ym3526_set_irq_handler   (m_chip, &ym3526_device::static_irq_handler, this);
	ym3526_set_update_handler(m_chip, &ym3526_device::static_update_request, this);

	m_timer[0] = timer_alloc(0);
	m_timer[1] = timer_alloc(1);
}

void ym3526_device::device_clock_changed()
{
	calculate_rates();
	ym3526_clock_changed(m_chip, clock(), clock() / 72);
}

void ym3526_device::calculate_rates()
{
	int rate = clock()/72; /* ??? */

	if (m_stream != nullptr)
		m_stream->set_sample_rate(rate);
	else
		m_stream = machine().sound().stream_alloc(*this,0,1,rate);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ym3526_device::device_stop()
{
	ym3526_shutdown(m_chip);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ym3526_device::device_reset()
{
	ym3526_reset_chip(m_chip);
}


u8 ym3526_device::read(offs_t offset)
{
	return ym3526_read(m_chip, offset & 1);
}

void ym3526_device::write(offs_t offset, u8 data)
{
	ym3526_write(m_chip, offset & 1, data);
}

u8 ym3526_device::status_port_r() { return read(0); }
u8 ym3526_device::read_port_r() { return read(1); }
void ym3526_device::control_port_w(u8 data) { write(0, data); }
void ym3526_device::write_port_w(u8 data) { write(1, data); }


DEFINE_DEVICE_TYPE(YM3526, ym3526_device, "ym3526", "YM3526 OPL")

ym3526_device::ym3526_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, YM3526, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_timer{ nullptr, nullptr }
	, m_chip(nullptr)
	, m_irq_handler(*this)
{
}

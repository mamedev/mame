// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "emu.h"
#include "2203intf.h"
#include "fm.h"

const ssg_callbacks ym2203_device::psgintf =
{
	&ym2203_device::psg_set_clock,
	&ym2203_device::psg_write,
	&ym2203_device::psg_read,
	&ym2203_device::psg_reset
};

/* IRQ Handler */
void ym2203_device::irq_handler(int irq)
{
	m_timer[2]->adjust(attotime::zero, irq);
}

/* Timer overflow callback from timer.c */
void ym2203_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_A:
		ym2203_timer_over(m_chip,0);
		break;

	case TIMER_B:
		ym2203_timer_over(m_chip,1);
		break;

	case TIMER_IRQ_SYNC:
		if (!m_irq_handler.isnull())
			m_irq_handler(param);
		break;
	}
}

void ym2203_device::timer_handler(int c, int count, int clock)
{
	if( count == 0 || clock == 0)
	{   /* Reset FM Timer */
		m_timer[c]->enable(false);
	}
	else
	{   /* Start FM Timer */
		attotime period = attotime::from_hz(clock) * count;

		if (!m_timer[c]->enable(true))
			m_timer[c]->adjust(period);
	}
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------


void ym2203_device::stream_generate(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	ym2203_update_one(m_chip, outputs[0], samples);
}


void ym2203_device::device_post_load()
{
	ym2203_postload(m_chip);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ym2203_device::device_start()
{
	ay8910_device::device_start();

	m_irq_handler.resolve();

	/* Timer Handler set */
	m_timer[0] = timer_alloc(TIMER_A);
	m_timer[1] = timer_alloc(TIMER_B);
	m_timer[2] = timer_alloc(TIMER_IRQ_SYNC);

	/* stream system initialize */
	calculate_rates();

	/* Initialize FM emurator */
	int rate = clock()/72; /* ??? */
	m_chip = ym2203_init(this,clock(),rate,&ym2203_device::static_timer_handler,&ym2203_device::static_irq_handler,&psgintf);
	assert_always(m_chip != nullptr, "Error creating YM2203 chip");
}

void ym2203_device::device_clock_changed()
{
	calculate_rates();
	ym2203_clock_changed(m_chip, clock(), clock() / 72);
}

void ym2203_device::calculate_rates()
{
	int rate = clock()/72; /* ??? */

	if (m_stream != nullptr)
		m_stream->set_sample_rate(rate);
	else
		m_stream = machine().sound().stream_alloc(*this,0,1,rate, stream_update_delegate(&ym2203_device::stream_generate,this));
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ym2203_device::device_stop()
{
	if (m_chip)
		ym2203_shutdown(m_chip);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ym2203_device::device_reset()
{
	ym2203_reset_chip(m_chip);
}


u8 ym2203_device::read(offs_t offset)
{
	return ym2203_read(m_chip, offset & 1);
}

void ym2203_device::write(offs_t offset, u8 data)
{
	ym2203_write(m_chip, offset & 1, data);
}

u8 ym2203_device::status_port_r()
{
	return read(0);
}

u8 ym2203_device::read_port_r()
{
	return read(1);
}

void ym2203_device::control_port_w(u8 data)
{
	write(0, data);
}

void ym2203_device::write_port_w(u8 data)
{
	write(1, data);
}

DEFINE_DEVICE_TYPE(YM2203, ym2203_device, "ym2203", "YM2203 OPN")

ym2203_device::ym2203_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ay8910_device(mconfig, YM2203, tag, owner, clock, PSG_TYPE_YM, 3, 2)
	, m_stream(nullptr)
	, m_timer{ nullptr, nullptr }
	, m_chip(nullptr)
	, m_irq_handler(*this)
{
}

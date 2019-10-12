// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

  262intf.c

  MAME interface for YMF262 (OPL3) emulator

***************************************************************************/
#include "emu.h"
#include "262intf.h"
#include "ymf262.h"


/* IRQ Handler */
void ymf262_device::irq_handler(int irq)
{
	if (!m_irq_handler.isnull())
		m_irq_handler(irq);
}

/* Timer overflow callback from timer.c */
void ymf262_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case 0:
		ymf262_timer_over(m_chip,0);
		break;

	case 1:
		ymf262_timer_over(m_chip,1);
		break;
	}
}


void ymf262_device::timer_handler(int c, const attotime &period)
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

void ymf262_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	ymf262_update_one(m_chip, outputs, samples);
}

//-------------------------------------------------
//  device_post_load - device-specific post load
//-------------------------------------------------
void ymf262_device::device_post_load()
{
	ymf262_post_load(m_chip);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ymf262_device::device_start()
{
	int rate = clock()/288;

	m_irq_handler.resolve();

	/* stream system initialize */
	m_chip = ymf262_init(this,clock(),rate);
	if (!m_chip)
		throw emu_fatalerror("ymf262_device(%s): Error creating YMF262 chip", tag());

	m_stream = machine().sound().stream_alloc(*this,0,4,rate);

	/* YMF262 setup */
	ymf262_set_timer_handler (m_chip, &ymf262_device::static_timer_handler, this);
	ymf262_set_irq_handler   (m_chip, &ymf262_device::static_irq_handler, this);
	ymf262_set_update_handler(m_chip, &ymf262_device::static_update_request, this);

	m_timer[0] = timer_alloc(0);
	m_timer[1] = timer_alloc(1);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ymf262_device::device_stop()
{
	ymf262_shutdown(m_chip);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ymf262_device::device_reset()
{
	ymf262_reset_chip(m_chip);
}

//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void ymf262_device::device_clock_changed()
{
	int rate = clock()/288;
	ymf262_clock_changed(m_chip,clock(),rate);
	m_stream->set_sample_rate(rate);
}

u8 ymf262_device::read(offs_t offset)
{
	return ymf262_read(m_chip, offset & 3);
}

void ymf262_device::write(offs_t offset, u8 data)
{
	ymf262_write(m_chip, offset & 3, data);
}

DEFINE_DEVICE_TYPE(YMF262, ymf262_device, "ymf262", "YMF262 OPL3")

ymf262_device::ymf262_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, YMF262, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_timer{ nullptr, nullptr }
	, m_chip(nullptr)
	, m_irq_handler(*this)
{
}

// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

  2612intf.c

  The YM2612 emulator supports up to 2 chips.
  Each chip has the following connections:
  - Status Read / Control Write A
  - Port Read / Data Write A
  - Control Write B
  - Data Write B

***************************************************************************/

#include "emu.h"
#include "2612intf.h"
#include "fm.h"

/*------------------------- YM2612 -------------------------------*/
/* IRQ Handler */
void ym2612_device::irq_handler(int irq)
{
	if (!m_irq_handler.isnull())
		m_irq_handler(irq);
}

/* Timer overflow callback from timer.c */
void ym2612_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case 0:
		ym2612_timer_over(m_chip,0);
		break;

	case 1:
		ym2612_timer_over(m_chip,1);
		break;
	}
}

void ym2612_device::timer_handler(int c,int count,int clock)
{
	if( count == 0 || clock == 0 )
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

void ym2612_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	ym2612_update_one(m_chip, outputs, samples);
}


void ym2612_device::device_post_load()
{
	ym2612_postload(m_chip);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ym2612_device::device_start()
{
	int rate = clock()/72;

	m_irq_handler.resolve();

	/* FM init */
	/* Timer Handler set */
	m_timer[0] = timer_alloc(0);
	m_timer[1] = timer_alloc(1);

	/* stream system initialize */
	m_stream = machine().sound().stream_alloc(*this,0,2,rate);

	/**** initialize YM2612 ****/
	m_chip = ym2612_init(this,clock(),rate,&ym2612_device::static_timer_handler,&ym2612_device::static_irq_handler);
	if (!m_chip)
		throw emu_fatalerror("ym2612_device(%s): Error creating YM2612 chip", tag());
}

void ym2612_device::device_clock_changed()
{
	calculate_rates();
	ym2612_clock_changed(m_chip, clock(), clock() / 72);
}

void ym2612_device::calculate_rates()
{
	int rate = clock() / 72;

	if (m_stream != nullptr)
		m_stream->set_sample_rate(rate);
	else
		m_stream = machine().sound().stream_alloc(*this,0,2,rate);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ym2612_device::device_stop()
{
	ym2612_shutdown(m_chip);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ym2612_device::device_reset()
{
	ym2612_reset_chip(m_chip);
}


u8 ym2612_device::read(offs_t offset)
{
	return ym2612_read(m_chip, offset & 3);
}

void ym2612_device::write(offs_t offset, u8 data)
{
	ym2612_write(m_chip, offset & 3, data);
}


DEFINE_DEVICE_TYPE(YM2612, ym2612_device, "ym2612", "YM2612 OPN2")

ym2612_device::ym2612_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ym2612_device(mconfig, YM2612, tag, owner, clock)
{
}

ym2612_device::ym2612_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_timer{ nullptr, nullptr }
	, m_chip(nullptr)
	, m_irq_handler(*this)
{
}


DEFINE_DEVICE_TYPE(YM3438, ym3438_device, "ym3438", "YM3438 OPN2C")

ym3438_device::ym3438_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ym2612_device(mconfig, YM3438, tag, owner, clock)
{
}

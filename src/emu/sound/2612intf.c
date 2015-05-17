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

#include "2612intf.h"
#include "fm.h"

/*------------------------- TM2612 -------------------------------*/
/* IRQ Handler */
static void IRQHandler(void *param,int irq)
{
	ym2612_device *ym2612 = (ym2612_device *) param;
	ym2612->_IRQHandler(irq);
}

void ym2612_device::_IRQHandler(int irq)
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

static void timer_handler(void *param,int c,int count,int clock)
{
	ym2612_device *ym2612 = (ym2612_device *) param;
	ym2612->_timer_handler(c, count, clock);
}

void ym2612_device::_timer_handler(int c,int count,int clock)
{
	if( count == 0 )
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

/* update request from fm.c */
void ym2612_update_request(void *param)
{
	ym2612_device *ym2612 = (ym2612_device *) param;
	ym2612->_ym2612_update_request();
}

void ym2612_device::_ym2612_update_request()
{
	m_stream->update();
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
	m_chip = ym2612_init(this,this,clock(),rate,timer_handler,IRQHandler);
	assert_always(m_chip != NULL, "Error creating YM2612 chip");
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


READ8_MEMBER( ym2612_device::read )
{
	return ym2612_read(m_chip, offset & 3);
}

WRITE8_MEMBER( ym2612_device::write )
{
	ym2612_write(m_chip, offset & 3, data);
}


const device_type YM2612 = &device_creator<ym2612_device>;

ym2612_device::ym2612_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, YM2612, "YM2612", tag, owner, clock, "ym2612", __FILE__),
		device_sound_interface(mconfig, *this),
		m_irq_handler(*this)
{
}

ym2612_device::ym2612_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_sound_interface(mconfig, *this),
		m_irq_handler(*this)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ym2612_device::device_config_complete()
{
}


const device_type YM3438 = &device_creator<ym3438_device>;

ym3438_device::ym3438_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ym2612_device(mconfig, YM3438, "YM3438", tag, owner, clock, "ym3438", __FILE__)
{
}

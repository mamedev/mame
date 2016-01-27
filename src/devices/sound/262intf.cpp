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
static void IRQHandler(void *param,int irq)
{
	ymf262_device *ymf262 = (ymf262_device *) param;
	ymf262->_IRQHandler(irq);
}

void ymf262_device::_IRQHandler(int irq)
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


static void timer_handler(void *param, int c, const attotime &period)
{
	ymf262_device *ymf262 = (ymf262_device *) param;
	ymf262->_timer_handler(c, period);
}

void ymf262_device::_timer_handler(int c, const attotime &period)
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

void ymf262_update_request(void *param, int interval)
{
	ymf262_device *ymf262 = (ymf262_device *) param;
	ymf262->_ymf262_update_request();
}

void ymf262_device::_ymf262_update_request()
{
	m_stream->update();
}



//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ymf262_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	ymf262_update_one(m_chip, outputs, samples);
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
	assert_always(m_chip != nullptr, "Error creating YMF262 chip");

	m_stream = machine().sound().stream_alloc(*this,0,4,rate);

	/* YMF262 setup */
	ymf262_set_timer_handler (m_chip, timer_handler, this);
	ymf262_set_irq_handler   (m_chip, IRQHandler, this);
	ymf262_set_update_handler(m_chip, ymf262_update_request, this);

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


READ8_MEMBER( ymf262_device::read )
{
	return ymf262_read(m_chip, offset & 3);
}

WRITE8_MEMBER( ymf262_device::write )
{
	ymf262_write(m_chip, offset & 3, data);
}

const device_type YMF262 = &device_creator<ymf262_device>;

ymf262_device::ymf262_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, YMF262, "YMF262", tag, owner, clock, "ymf262", __FILE__),
		device_sound_interface(mconfig, *this),
		m_irq_handler(*this)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ymf262_device::device_config_complete()
{
}

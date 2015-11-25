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
#include "3526intf.h"
#include "fmopl.h"


/* IRQ Handler */
static void IRQHandler(void *param,int irq)
{
	ym3526_device *ym3526 = (ym3526_device *) param;
	ym3526->_IRQHandler(irq);
}

void ym3526_device::_IRQHandler(int irq)
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

static void timer_handler(void *param,int c,const attotime &period)
{
	ym3526_device *ym3526 = (ym3526_device *) param;
	ym3526->_timer_handler(c, period);
}

void ym3526_device::_timer_handler(int c,const attotime &period)
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

/* update request from fm.c */
void ym3526_update_request(void *param, int interval)
{
	ym3526_device *ym3526 = (ym3526_device *) param;
	ym3526->_ym3526_update_request();
}

void ym3526_device::_ym3526_update_request()
{
	m_stream->update();
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
	int rate = clock()/72;

	// resolve callbacks
	m_irq_handler.resolve();

	/* stream system initialize */
	m_chip = ym3526_init(this,clock(),rate);
	assert_always(m_chip != NULL, "Error creating YM3526 chip");

	m_stream = machine().sound().stream_alloc(*this,0,1,rate);
	/* YM3526 setup */
	ym3526_set_timer_handler (m_chip, timer_handler, this);
	ym3526_set_irq_handler   (m_chip, IRQHandler, this);
	ym3526_set_update_handler(m_chip, ym3526_update_request, this);

	m_timer[0] = timer_alloc(0);
	m_timer[1] = timer_alloc(1);
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


READ8_MEMBER( ym3526_device::read )
{
	return ym3526_read(m_chip, offset & 1);
}

WRITE8_MEMBER( ym3526_device::write )
{
	ym3526_write(m_chip, offset & 1, data);
}

READ8_MEMBER( ym3526_device::status_port_r ) { return read(space, 0); }
READ8_MEMBER( ym3526_device::read_port_r ) { return read(space, 1); }
WRITE8_MEMBER( ym3526_device::control_port_w ) { write(space, 0, data); }
WRITE8_MEMBER( ym3526_device::write_port_w ) { write(space, 1, data); }


const device_type YM3526 = &device_creator<ym3526_device>;

ym3526_device::ym3526_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, YM3526, "YM3526", tag, owner, clock, "ym3526", __FILE__),
		device_sound_interface(mconfig, *this),
		m_stream(NULL),
		m_chip(NULL),
		m_irq_handler(*this)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ym3526_device::device_config_complete()
{
}

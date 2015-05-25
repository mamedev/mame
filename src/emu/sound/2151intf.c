// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

    2151intf.c

    Support interface YM2151(OPM)

***************************************************************************/

#include "emu.h"
#include "fm.h"
#include "2151intf.h"
#include "ym2151.h"



const device_type YM2151 = &device_creator<ym2151_device>;


//-------------------------------------------------
//  ym2151_device - constructor
//-------------------------------------------------

ym2151_device::ym2151_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, YM2151, "YM2151", tag, owner, clock, "ym2151", __FILE__),
		device_sound_interface(mconfig, *this),
		m_irqhandler(*this),
		m_portwritehandler(*this)
{
}


//-------------------------------------------------
//  read - read from the device
//-------------------------------------------------

READ8_MEMBER( ym2151_device::read )
{
	if (offset & 1)
	{
		m_stream->update();
		return ym2151_read_status(m_chip);
	}
	else
		return 0xff;    /* confirmed on a real YM2151 */
}


//-------------------------------------------------
//  write - write from the device
//-------------------------------------------------

WRITE8_MEMBER( ym2151_device::write )
{
	if (offset & 1)
	{
		m_stream->update();
		ym2151_write_reg(m_chip, m_lastreg, data);
	}
	else
		m_lastreg = data;
}


READ8_MEMBER( ym2151_device::status_r ) { return read(space, 1); }

WRITE8_MEMBER( ym2151_device::register_w ) { write(space, 0, data); }
WRITE8_MEMBER( ym2151_device::data_w ) { write(space, 1, data); }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ym2151_device::device_start()
{
	m_irqhandler.resolve_safe();
	m_portwritehandler.resolve_safe();

	// stream setup
	int rate = clock() / 64;
	m_stream = stream_alloc(0, 2, rate);

	m_chip = ym2151_init(this, clock(), rate);
	assert_always(m_chip != NULL, "Error creating YM2151 chip");

	ym2151_set_irq_handler(m_chip, irq_frontend);
	ym2151_set_port_write_handler(m_chip, port_write_frontend);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ym2151_device::device_reset()
{
	ym2151_reset_chip(m_chip);
}


//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ym2151_device::device_stop()
{
	ym2151_shutdown(m_chip);
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ym2151_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	ym2151_update_one(m_chip, outputs, samples);
}


void ym2151_device::irq_frontend(device_t *device, int irq)
{
	downcast<ym2151_device *>(device)->m_irqhandler(irq);
}

void ym2151_device::port_write_frontend(device_t *device, offs_t offset, UINT8 data)
{
	downcast<ym2151_device *>(device)->m_portwritehandler(offset, data);
}

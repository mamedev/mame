// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    midiin.h

    MIDI In image device

*********************************************************************/

#ifndef MAME_IMAGEDEV_MIDIIN_H
#define MAME_IMAGEDEV_MIDIIN_H

#pragma once

#include "diserial.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class midiin_device :    public device_t,
						public device_image_interface,
						public device_serial_interface
{
public:
	// construction/destruction
	midiin_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto input_callback() { return m_input_cb.bind(); }

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;

	// image device
	virtual iodevice_t image_type() const noexcept override { return IO_MIDIIN; }
	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool must_be_loaded() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "mid"; }
	virtual bool core_opens_image_file() const noexcept override { return false; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// serial overrides
	virtual void tra_complete() override;    // Tx completed sending byte
	virtual void tra_callback() override;    // Tx send bit

private:
	static const int XMIT_RING_SIZE = (8192*4*4);

	void xmit_char(uint8_t data);

	osd_midi_device *m_midi;
	emu_timer *m_timer;
	devcb_write_line        m_input_cb;
	uint8_t m_xmitring[XMIT_RING_SIZE];
	int m_xmit_read, m_xmit_write;
	bool m_tx_busy;
};

// device type definition
DECLARE_DEVICE_TYPE(MIDIIN, midiin_device)

// device iterator
typedef device_type_iterator<midiin_device> midiin_device_iterator;

#endif // MAME_IMAGEDEV_MIDIIN_H

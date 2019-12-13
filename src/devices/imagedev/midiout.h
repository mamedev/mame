// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    midiout.h

    MIDI Out image device

*********************************************************************/

#ifndef MAME_IMAGEDEV_MIDIOUT_H
#define MAME_IMAGEDEV_MIDIOUT_H

#pragma once

#include "diserial.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class midiout_device :    public device_t,
						public device_image_interface,
						public device_serial_interface
{
public:
	// construction/destruction
	midiout_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;

	// image device
	virtual iodevice_t image_type() const noexcept override { return IO_MIDIOUT; }
	virtual bool is_readable()  const noexcept override { return false; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool must_be_loaded() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "mid"; }
	virtual bool core_opens_image_file() const noexcept override { return false; }

	virtual void tx(uint8_t state) { rx_w(state); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// serial overrides
	virtual void rcv_complete() override;    // Rx completed receiving byte

private:
	osd_midi_device *m_midi;
};

// device type definition
DECLARE_DEVICE_TYPE(MIDIOUT, midiout_device)

// device iterator
typedef device_type_iterator<midiout_device> midiout_device_iterator;

#endif // MAME_IMAGEDEV_MIDIOUT_H

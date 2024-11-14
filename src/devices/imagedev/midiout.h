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

#include "interface/midiport.h"

#include <memory>
#include <string>
#include <system_error>
#include <utility>


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
	~midiout_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	// image device
	virtual bool is_readable()  const noexcept override { return false; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "mid"; }
	virtual bool core_opens_image_file() const noexcept override { return false; }
	virtual const char *image_type_name() const noexcept override { return "midiout"; }
	virtual const char *image_brief_type_name() const noexcept override { return "mout"; }

	virtual void tx(uint8_t state) { rx_w(state); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial_interface implementation
	virtual void rcv_complete() override;    // Rx completed receiving byte

private:
	std::unique_ptr<osd::midi_output_port> m_midi;
};

// device type definition
DECLARE_DEVICE_TYPE(MIDIOUT, midiout_device)

// device iterator
typedef device_type_enumerator<midiout_device> midiout_device_enumerator;

#endif // MAME_IMAGEDEV_MIDIOUT_H

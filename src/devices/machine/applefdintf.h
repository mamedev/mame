// license:BSD-3-Clause
// copyright-holders:O. Galibert
/*********************************************************************

    applefdintf.h

    Common interface of various Apple Floppy Disk Controllers, implemented
    in the classic Apple controller, the IWM and the SWIM variants


*********************************************************************/

#ifndef MAME_MACHINE_APPLEFDINTF_H
#define MAME_MACHINE_APPLEFDINTF_H

#pragma once

#include "imagedev/floppy.h"

class applefdintf_device : public device_t
{
public:
	// read/write handlers for cpu access
	virtual uint8_t read(offs_t offset) = 0;
	virtual void write(offs_t offset, u8 data) = 0;

	// phases line management
	//   Input from the exterior for the devices where they are bidirectional
	void phases_w(u8 phases);
	//   Push output for when driven by the chip
	auto phases_cb() { return m_phases_cb.bind(); }

	// sel35 line when present (active high)
	auto sel35_cb() { return m_sel35_cb.bind(); }

	// drive select lines (two of them in the low bits, active high)
	auto devsel_cb() { return m_devsel_cb.bind(); }

	// hdsel line when present and in output mode (active high)
	auto hdsel_cb() { return m_hdsel_cb.bind(); }

	// dat1byte line when present (active high)
	auto dat1byte_cb() { return m_dat1byte_cb.bind(); }

	// floppy selection input, to be updated according to the previous callbacks,
	// nullptr if none selected
	virtual void set_floppy(floppy_image_device *floppy) = 0;

	// get the currently selected floppy
	virtual floppy_image_device *get_floppy() const = 0;

	// Floppy formats
	//   13-sector 5.25
	static void formats_525_13(format_registration &fr);

	//   5.25
	static void formats_525(format_registration &fr);

	//   3.5
	static void formats_35(format_registration &fr);

	// Floppy drives slot-level
	//   5.25
	static void floppies_525(device_slot_interface &device);
	template<typename T> static void add_525_13(machine_config &config, T &floppy) {
		FLOPPY_CONNECTOR(config, floppy, floppies_525, "525", formats_525_13).enable_sound(true);
	}
	template<typename T> static void add_525_13_nc(machine_config &config, T &floppy) {
		FLOPPY_CONNECTOR(config, floppy, floppies_525, nullptr, formats_525_13).enable_sound(true);
	}

	template<typename T> static void add_525(machine_config &config, T &floppy) {
		FLOPPY_CONNECTOR(config, floppy, floppies_525, "525", formats_525).enable_sound(true);
	}
	template<typename T> static void add_525_nc(machine_config &config, T &floppy) {
		FLOPPY_CONNECTOR(config, floppy, floppies_525, nullptr, formats_525).enable_sound(true);
	}

	//   3.5
	static void floppies_35(device_slot_interface &device);
	template<typename T> static void add_35(machine_config &config, T &floppy) {
		FLOPPY_CONNECTOR(config, floppy, floppies_35, "35dd", formats_35).enable_sound(true);
	}
	template<typename T> static void add_35_sd(machine_config &config, T &floppy) {
		FLOPPY_CONNECTOR(config, floppy, floppies_35, "35sd", formats_35).enable_sound(true);
	}
	template<typename T> static void add_35_hd(machine_config &config, T &floppy) {
		FLOPPY_CONNECTOR(config, floppy, floppies_35, "35hd", formats_35).enable_sound(true);
	}
	template<typename T> static void add_35_nc(machine_config &config, T &floppy) {
		FLOPPY_CONNECTOR(config, floppy, floppies_35, nullptr, formats_35).enable_sound(true);
	}

	// Sync the state when something external is going to change, like
	// the floppy side or rpm.
	virtual void sync() = 0;

protected:
	devcb_write8 m_phases_cb, m_devsel_cb;
	devcb_write_line m_sel35_cb, m_hdsel_cb, m_dat1byte_cb;

	// Current phase value in the bottom bits, input/output flags in the top bits
	u8 m_phases;

	// Current value of the driven external phases input
	u8 m_phases_input;

	applefdintf_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void update_phases();
};

#endif // MAME_MACHINE_APPLEFDINTF_H

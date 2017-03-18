// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    dinvram.h

    Device NVRAM interfaces.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DINVRAM
#define MAME_EMU_DINVRAM

//**************************************************************************
//  CONFIGURATION MACROS
//**************************************************************************

#define MCFG_DEVICE_HAS_BATTERY(_batt) \
	device_optional_nvram_interface::static_set_has_battery(*device, _batt);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> device_nvram_interface

// class representing interface-specific live nvram
class device_nvram_interface : public device_interface
{
public:
	// construction/destruction
	device_nvram_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_nvram_interface();

	// public accessors... for now
	void nvram_reset() { nvram_default(); }
	void nvram_load(emu_file &file) { nvram_read(file); }
	void nvram_save(emu_file &file) { nvram_write(file); }

	bool has_battery_backup() const { return device_has_battery_backup(); }

	memory_array &memarray() { return m_memarray; }

protected:
	// derived class overrides
	virtual void nvram_default() = 0;
	virtual void nvram_read(emu_file &file);
	virtual void nvram_write(emu_file &file);
	virtual bool device_has_battery_backup() const { return true; }

private:
	memory_array m_memarray;
};

// iterator
typedef device_interface_iterator<device_nvram_interface> nvram_interface_iterator;

// ======================> device_optional_nvram_interface

// interface subclass for memories that may or may not be battery-backed
class device_optional_nvram_interface : public device_nvram_interface
{
public:
	// construction/destruction
	device_optional_nvram_interface(const machine_config &mconfig, device_t &device, bool has_battery_by_default);

	// static configuration
	static void static_set_has_battery(device_t &device, bool has_battery);

protected:
	// device_nvram_interface overrides
	virtual bool device_has_battery_backup() const override { return m_has_battery; }

private:
	bool m_has_battery;
};


#endif  /* MAME_EMU_DINVRAM */

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
	bool nvram_can_save() { return nvram_can_write(); }

protected:
	// derived class overrides
	virtual void nvram_default() = 0;
	virtual void nvram_read(emu_file &file) = 0;
	virtual void nvram_write(emu_file &file) = 0;
	virtual bool nvram_can_write() { return true; }
};

// iterator
typedef device_interface_enumerator<device_nvram_interface> nvram_interface_enumerator;


#endif  /* MAME_EMU_DINVRAM */

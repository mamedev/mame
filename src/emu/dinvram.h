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
	device_nvram_interface(const machine_config &mconfig, device_t &device, bool backup_enabled = true);
	virtual ~device_nvram_interface();

	// configuration
	void nvram_enable_backup(bool enabled) { assert(!device().started()); m_backup_enabled = enabled; }

	// public accessors... for now
	void nvram_reset() { nvram_default(); }
	bool nvram_load(util::read_stream &file) { return nvram_read(file); }
	bool nvram_save(util::write_stream &file) { return nvram_write(file); }
	bool nvram_backup_enabled() const noexcept { return m_backup_enabled; }
	bool nvram_can_save() const { return m_backup_enabled && nvram_can_write(); }

protected:
	// derived class overrides
	virtual void nvram_default() = 0;
	virtual bool nvram_read(util::read_stream &file) = 0;
	virtual bool nvram_write(util::write_stream &file) = 0;
	virtual bool nvram_can_write() const { return true; }

private:
	bool m_backup_enabled;
};

// iterator
typedef device_interface_enumerator<device_nvram_interface> nvram_interface_enumerator;


#endif  /* MAME_EMU_DINVRAM */

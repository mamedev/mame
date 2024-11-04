// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

    NEC PC8801-31 CD-ROM I/F

**************************************************************************************************/

#ifndef MAME_BUS_PC8801_31_H
#define MAME_BUS_PC8801_31_H

#pragma once

#include "machine/nscsi_bus.h"
#include "bus/nscsi/cd.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pc8801_31_device : public device_t
{
public:
	// construction/destruction
	pc8801_31_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

	auto rom_bank_cb() { return m_rom_bank_cb.bind(); }

	// I/O operations
	void amap(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(select_off);

private:
	required_device<nscsi_bus_device> m_scsibus;

	devcb_write_line m_rom_bank_cb;

	emu_timer *m_sel_off_timer;

	u8 scsi_status_r();
	void scsi_sel_w(u8 data);
	void scsi_reset_w(u8 data);
	u8 clock_r();
	void volume_control_w(u8 data);
	u8 id_r();
	void rom_bank_w(u8 data);
	u8 volume_meter_r();

	bool m_clock_hb;
	bool m_cddrive_enable;
};


// device type definition
DECLARE_DEVICE_TYPE(PC8801_31, pc8801_31_device)

#endif // MAME_BUS_PC8801_31_H

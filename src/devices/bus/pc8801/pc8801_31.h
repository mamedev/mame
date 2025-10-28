// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

    NEC PC8801-31 CD-ROM I/F

**************************************************************************************************/

#ifndef MAME_BUS_PC8801_PC8801_31_H
#define MAME_BUS_PC8801_PC8801_31_H

#pragma once

#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "bus/nscsi/pc8801_30.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cb.h"

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
	auto drq_cb() { return m_drq_cb.bind(); }
	auto dma_r() { return data_r(); }

	// I/O operations
	void amap(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(select_off_cb);

private:
	required_device<nscsi_bus_device> m_sasibus;
	required_device<nscsi_cdrom_pc8801_30_device> m_cddrive;
	required_device<nscsi_callback_device> m_sasi;

	devcb_write_line m_rom_bank_cb;
	devcb_write_line m_drq_cb;

	emu_timer *m_sel_off_timer;

	u8 status_r();
	void select_w(u8 data);
	u8 data_r();
	void data_w(u8 data);
	void scsi_reset_w(u8 data);
	u8 clock_r();
	u8 id_r();
	void rom_bank_w(u8 data);
	template <unsigned N> u8 volume_meter_r();

	bool m_clock_hb;
	bool m_cddrive_enable;
	bool m_dma_enable;
	int m_sasi_sel;
	int m_sasi_req;

	void sasi_req_w(int state);
	void sasi_sel_w(int state);
};


// device type definition
DECLARE_DEVICE_TYPE(PC8801_31, pc8801_31_device)

#endif // MAME_BUS_PC8801_PC8801_31_H

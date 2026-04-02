// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor X37 SASI hard disk controller emulation

*********************************************************************/

#ifndef MAME_LUXOR_X37_SASI_H
#define MAME_LUXOR_X37_SASI_H

#pragma once


#include "bus/nscsi/devices.h"
#include "machine/nscsi_bus.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> luxor_x37_sasi_device

class luxor_x37_sasi_device :  public device_t, public nscsi_device_interface
{
public:
	// construction/destruction
	luxor_x37_sasi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_callback() { return m_write_int.bind(); }
	auto req0_callback() { return m_write_req0.bind(); }

	uint16_t stat_r(offs_t offset, uint16_t mem_mask);
	void ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t tre_r(offs_t offset, uint16_t mem_mask);
	void tre_w(offs_t offset, uint16_t data, uint16_t mem_mask);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// nscsi_device_interface overrides
	virtual void scsi_ctrl_changed() override;

private:
	devcb_write_line m_write_int;
	devcb_write_line m_write_req0;

	memory_share_creator<u16> m_buffer;

	bool m_int = 1;
	bool m_req0 = 1;
	u8 m_data_out = 0;
	offs_t m_a = 0;
	bool m_hlc = 0;
	bool m_dir = 0;
	bool m_rc = 1;
};


// device type definition
DECLARE_DEVICE_TYPE(LUXOR_X37_SASI, luxor_x37_sasi_device)


#endif // MAME_LUXOR_X37_SASI_H

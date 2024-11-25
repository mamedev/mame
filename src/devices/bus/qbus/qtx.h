// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_QBUS_QTX_H
#define MAME_BUS_QBUS_QTX_H

#pragma once

#include "qbus.h"
#include "machine/mc68901.h"
#include "machine/ncr53c90.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> qts1_device

class qts1_device : public device_t, public device_qbus_card_interface
{
public:
	// device type constructor
	qts1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	u8 asc_r(offs_t offset);
	void asc_w(offs_t offset, u8 data);
	void dma_address_w(offs_t offset, u8 data);
	u8 io_status_r();
	void channel_w(u8 data);

	void asc_config(device_t *device);

	void prg_map(address_map &map) ATTR_COLD;
	void fc7_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_localcpu;
	required_device<mc68901_device> m_mfp;
	required_device<ncr53c90a_device> m_asc;

	u32 m_dma_address;
};

// device type declaration
DECLARE_DEVICE_TYPE(TTI_QTS1, qts1_device)

#endif // MAME_BUS_QBUS_QTX_H

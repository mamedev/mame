// license:BSD-3-Clause
// copyright-holders:Roberto Fernandez,Nigel Barnes
#ifndef MAME_BUS_COCO_COCO_PSG_H
#define MAME_BUS_COCO_COCO_PSG_H

#pragma once

#include "cococart.h"
#include "machine/intelfsh.h"
#include "sound/ay8910.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> coco_psg_device

class coco_psg_device :
	public device_t,
	public device_cococart_interface
{
public:
	// construction/destruction
	coco_psg_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u8 cts_read(offs_t offset) override;
	virtual void cts_write(offs_t offset, u8 data) override;
	virtual u8 scs_read(offs_t offset) override;
	virtual void scs_write(offs_t offset, u8 data) override;

private:
	required_device<ay8910_device> m_psg;
	required_device<sst_39sf040_device> m_flash;

	void flash2aaa_w(offs_t offset, u8 data);
	void flash5555_w(offs_t offset, u8 data);

	std::unique_ptr<u8[]> m_sram;
	u8 m_bank[2];
	u8 m_control;
};


// device type definition
DECLARE_DEVICE_TYPE(COCO_PSG, device_cococart_interface)

#endif // MAME_BUS_COCO_COCO_PSG_H

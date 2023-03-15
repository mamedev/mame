// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
#ifndef MAME_BUS_COCO_COCO_IDE_H
#define MAME_BUS_COCO_COCO_IDE_H

#pragma once

#include "cococart.h"
#include "bus/ata/ataintf.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> coco_ide_device

class coco_ide_device :
		public device_t,
		public device_cococart_interface,
		public device_cococart_host_interface
{
public:
	// construction/destruction
	coco_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual u8 cts_read(offs_t offset) override;
	virtual void cts_write(offs_t offset, u8 data) override;
	virtual u8 scs_read(offs_t offset) override;
	virtual void scs_write(offs_t offset, u8 data) override;
	virtual void set_sound_enable(bool sound_enable) override;

	virtual u8 *get_cart_base() override;
	virtual u32 get_cart_size() override;

	virtual address_space &cartridge_space() override;

private:
	required_device<ata_interface_device> m_ata;
	required_device<cococart_slot_device> m_slot;
	required_ioport m_jumpers;

	u8 ide_read(offs_t offset);
	void ide_write(offs_t offset, u8 data);

	u8 m_latch;
};


// device type definition
DECLARE_DEVICE_TYPE(COCO_IDE, coco_ide_device)

#endif // MAME_BUS_COCO_COCO_IDE_H

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
	coco_psg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(cts_read) override;
	virtual DECLARE_WRITE8_MEMBER(cts_write) override;
	virtual DECLARE_READ8_MEMBER(scs_read) override;
	virtual DECLARE_WRITE8_MEMBER(scs_write) override;

private:
	required_device<ay8910_device> m_psg;
	required_device<sst_39sf040_device> m_flash;

	void flash2aaa_w(offs_t offset, uint8_t data);
	void flash5555_w(offs_t offset, uint8_t data);

	std::unique_ptr<uint8_t[]> m_sram;
	uint8_t m_bank[2];
	uint8_t m_control;
};


// device type definition
DECLARE_DEVICE_TYPE(COCO_PSG, device_cococart_interface)

#endif // MAME_BUS_COCO_COCO_PSG_H

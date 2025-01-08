// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_QBUS_QG640_H
#define MAME_BUS_QBUS_QG640_H

#pragma once

#include "qbus.h"
#include "machine/7200fifo.h"
#include "video/bt47x.h"
#include "video/hd63484.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> qg640_device

class qg640_device : public device_t, public device_qbus_card_interface
{
public:
	// device type constructor
	qg640_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 status_r();
	void unknown_w(u8 data);
	void clut_w(offs_t offset, u8 data);
	u16 acrtc_r(offs_t offset);
	void acrtc_w(offs_t offset, u16 data);

	void mem_map(address_map &map) ATTR_COLD;
	void videoram_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_qgcpu;
	required_device<idt7201_device> m_fifo;
	required_device<hd63484_device> m_acrtc;
	required_device_array<bt47x_device_base, 2> m_clut;
};

// device type declaration
DECLARE_DEVICE_TYPE(MATROX_QG640, qg640_device)

#endif // MAME_BUS_QBUS_QG640_H

// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Morrow Designs Disk Jockey/DMA floppy controller board emulation

**********************************************************************/

#ifndef MAME_BUS_S100_DJDMA_H
#define MAME_BUS_S100_DJDMA_H

#pragma once

#include "s100.h"
#include "cpu/z80/z80.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> s100_djdma_device

class s100_djdma_device : public device_t, public device_s100_card_interface
{
public:
	// construction/destruction
	s100_djdma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// bus-level overrides;
	virtual void s100_sout_w(offs_t offset, uint8_t data) override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void djdma_io(address_map &map) ATTR_COLD;
	void djdma_mem(address_map &map) ATTR_COLD;

	void reset_int_w(u8 data);
	u8 disk_di_r();
	void disk_do_w(u8 data);
	u8 bus_di_r();
	void bus_hi_addr_w(u8 data);
	void bus_status_w(u8 data);
	u8 bus_request_r();
	u8 bus_release_r();
	void bus_stb_w(offs_t offset, u8 data);
	u8 disk_status_r();
	void dro0_w(u8 data);
	void dro1_w(u8 data);
	void dro2_w(u8 data);
	void dro3_w(u8 data);

	required_device<cpu_device> m_diskcpu;
	required_region_ptr<u8> m_cmdaddr;

	bool m_bus_hold;
};


// device type definition
DECLARE_DEVICE_TYPE(S100_DJDMA, s100_djdma_device)

#endif // MAME_BUS_S100_DJDMA_H

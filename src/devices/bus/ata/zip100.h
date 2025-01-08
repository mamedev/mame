// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_ATA_ZIP100_H
#define MAME_BUS_ATA_ZIP100_H

#pragma once

#include "atadev.h"

class zip100_ide_device : public device_t, public device_ata_interface
{
public:
	zip100_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual u16 read_dma() override;
	virtual u16 read_cs0(offs_t offset, u16 mem_mask = 0xffff) override;
	virtual u16 read_cs1(offs_t offset, u16 mem_mask = 0xffff) override;
	virtual void write_dma(u16 data) override;
	virtual void write_cs0(offs_t offset, u16 data, u16 mem_mask = 0xffff) override;
	virtual void write_cs1(offs_t offset, u16 data, u16 mem_mask = 0xffff) override;
	virtual void write_dmack(int state) override;
	virtual void write_csel(int state) override;
	virtual void write_dasp(int state) override;
	virtual void write_pdiag(int state) override;

private:
	void mem_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(ZIP100_IDE, zip100_ide_device)

#endif // MAME_BUS_ATA_ZIP100_H

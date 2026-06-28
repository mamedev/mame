// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_NEC_PC88_SDIP_H
#define MAME_NEC_PC88_SDIP_H

#pragma once

#include "machine/eepromser.h"


class pc88_sdip_device : public eeprom_serial_93c06_16bit_device
{
public:
	pc88_sdip_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	ioport_value dsw1_r();
	ioport_value dsw2_r();
	ioport_value auto_boot_floppy_r();
//	ioport_value built_in_fdd_r();
	ioport_value memory_weight_r();
};

class pc8801mc_memsw_device : public device_t,
						  public device_nvram_interface
{
public:
	pc8801mc_memsw_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	ioport_value dsw1_r();
	ioport_value dsw2_r();
	ioport_value auto_boot_floppy_r();
	ioport_value boot_mode_r();
	ioport_value cpu_clock_r();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	u8 m_bram[0x80];
	const u8 m_bram_size = 0x80;
};


// device type definition
DECLARE_DEVICE_TYPE(PC88_SDIP, pc88_sdip_device)
DECLARE_DEVICE_TYPE(PC8801MC_MEMSW, pc8801mc_memsw_device)

#endif // MAME_NEC_PC88_SDIP_H

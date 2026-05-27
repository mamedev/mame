// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_NEC_PC98_SDIP_H
#define MAME_NEC_PC98_SDIP_H

#pragma once

#include "machine/nvram.h"


class pc98_sdip_device : public device_t,
						  public device_nvram_interface
{
public:
	pc98_sdip_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	void bank_w(int state);

	// legacy i/f
	// TODO: not necessarily linear, bit 7 is parity bit
	ioport_value dsw1_r() { return m_sdip_ram[0]; }
	ioport_value dsw2_r() { return m_sdip_ram[1]; }
	ioport_value dsw3_r() {
		const u8 fdc_setting = (m_sdip_ram[2] & 3) ^ 2;
		//popmessage("%s %s", BIT(fdc_setting, 1) ? "2HD" : "2DD", BIT(fdc_setting, 0) ? "fixed" : "auto-detect");
		return (fdc_setting) | (m_sdip_ram[2] & 0x7c) | (0 << 7);
	}

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	u8 m_sdip_ram[24];
	const u8 m_sdip_size = 24;
	bool m_bank = 0;
};


// device type definition
DECLARE_DEVICE_TYPE(PC98_SDIP, pc98_sdip_device)

#endif // MAME_NEC_PC98_SDIP_H

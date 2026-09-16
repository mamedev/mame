// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_NEC_PC88VA_MEMSW_H
#define MAME_NEC_PC88VA_MEMSW_H

#pragma once

#include "machine/nvram.h"


class pc88va_memsw_device : public device_t,
						  public device_nvram_interface
{
public:
	pc88va_memsw_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	// $198, byte/word
	void write_disable_w(u16 data) { (void)data; m_write_protected = true; }
	// $19a, byte/word
	void write_enable_w(u16 data) { (void)data; m_write_protected = false; }

	// $30-$31
	u8 dsw_r(offs_t offset) { return m_bram[2 + offset * 4]; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	u8 m_bram[0x20];
	const u8 m_bram_size = 0x20;
	bool m_write_protected;
};


// device type definition
DECLARE_DEVICE_TYPE(PC88VA_MEMSW, pc88va_memsw_device)

#endif // MAME_NEC_PC88VA_MEMSW_H

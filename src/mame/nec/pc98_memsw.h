// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_NEC_PC98_MEMSW_H
#define MAME_NEC_PC98_MEMSW_H

#pragma once

#include "machine/nvram.h"


class pc98_memsw_device : public device_t,
						  public device_nvram_interface
{
public:
	pc98_memsw_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	uint8_t m_bram[0x10];
	const uint8_t m_bram_size = 0x10;
};


// device type definition
DECLARE_DEVICE_TYPE(PC98_MEMSW, pc98_memsw_device)

#endif // MAME_NEC_PC98_MEMSW_H

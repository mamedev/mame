// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_EFO_CEDAR_MAGNET_FLOP_H
#define MAME_EFO_CEDAR_MAGNET_FLOP_H

#pragma once


DECLARE_DEVICE_TYPE(CEDAR_MAGNET_FLOP, cedar_magnet_flop_device)

#include "machine/nvram.h"

class cedar_magnet_flop_device : public device_t
{
public:
	// construction/destruction
	cedar_magnet_flop_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 port60_r();
	u8 port61_r();
	u8 port63_r();

	void port60_w(u8 data);
	void port62_w(u8 data);
	void port63_w(u8 data);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_region_ptr<u8> m_disk;
	u8 m_flopdat = 0;
	u8 m_flopcmd = 0;
	u8 m_flopsec = 0;
	u8 m_flopstat = 0;
	u8 m_floptrk = 0;

	u8 m_curtrack = 0;
	int m_secoffs = 0;

};

#endif // MAME_EFO_CEDAR_MAGNET_FLOP_H

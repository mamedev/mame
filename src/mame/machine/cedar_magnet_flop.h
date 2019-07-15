// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_CEDAR_MAGNET_FLOP_H
#define MAME_MACHINE_CEDAR_MAGNET_FLOP_H

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

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_region_ptr<u8> m_disk;
	u8 m_flopdat;
	u8 m_flopcmd;
	u8 m_flopsec;
	u8 m_flopstat;
	u8 m_floptrk;

	u8 m_curtrack;
	int m_secoffs;

};

#endif // MAME_MACHINE_CEDAR_MAGNET_FLOP_H

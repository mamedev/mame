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

	DECLARE_READ8_MEMBER(port60_r);
	DECLARE_READ8_MEMBER(port61_r);
	DECLARE_READ8_MEMBER(port63_r);

	DECLARE_WRITE8_MEMBER(port60_w);
	DECLARE_WRITE8_MEMBER(port62_w);
	DECLARE_WRITE8_MEMBER(port63_w);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t m_flopdat;
	uint8_t m_flopcmd;
	uint8_t m_flopsec;
	uint8_t m_flopstat;
	uint8_t m_floptrk;

	uint8_t m_curtrack;
	int m_secoffs;

};

#endif // MAME_MACHINE_CEDAR_MAGNET_FLOP_H

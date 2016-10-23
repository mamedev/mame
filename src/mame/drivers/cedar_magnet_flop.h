// license:BSD-3-Clause
// copyright-holders:David Haywood

#pragma once

#ifndef CEDAR_MAGNET_FLOP_DEF
#define CEDAR_MAGNET_FLOP_DEF

extern const device_type CEDAR_MAGNET_FLOP;

#define MCFG_CEDAR_MAGNET_FLOP_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, CEDAR_MAGNET_FLOP, 0)

#include "machine/nvram.h"

class cedar_magnet_flop_device :  public device_t
{
public:
	// construction/destruction
	cedar_magnet_flop_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t m_flopdat;
	uint8_t m_flopcmd;
	uint8_t m_flopsec;
	uint8_t m_flopstat;
	uint8_t m_floptrk;

	uint8_t m_curtrack;
	int m_secoffs;

	uint8_t port60_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t port61_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t port63_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void port60_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port62_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port63_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
private:
};

#endif

// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#pragma once

#ifndef CAMMU_H_
#define CAMMU_H_

#include "emu.h"

class cammu_device : public device_t, public device_memory_interface
{
public:
	cammu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_ADDRESS_MAP(map, 8);

	DECLARE_READ32_MEMBER(mmu_r);
	DECLARE_WRITE32_MEMBER(mmu_w);

	DECLARE_READ32_MEMBER(cammu_r) { return m_cammu[offset]; }
	DECLARE_WRITE32_MEMBER(cammu_w) { m_cammu[offset] = data; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config (address_spacenum spacenum) const override;

private:
	address_space_config m_main_space_config;
	address_space_config m_io_space_config;
	address_space_config m_boot_space_config;

	address_space *m_main_space;
	address_space *m_io_space;
	address_space *m_boot_space;

	u32 m_cammu[1024];
};

// device type definition
extern const device_type CAMMU;

#endif
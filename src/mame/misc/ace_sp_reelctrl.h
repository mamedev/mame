// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MISC_ACE_SP_REELCTRL_H
#define MAME_MISC_ACE_SP_REELCTRL_H

#pragma once

#include "cpu/m6805/m68705.h"

class ace_sp_reelctrl_base_device : public device_t
{
public:
protected:
	ace_sp_reelctrl_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<m68705p_device> m_mcu;
};

class ace_sp_reelctrl_device : public ace_sp_reelctrl_base_device
{
public:
	ace_sp_reelctrl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
};

class ace_sp_reelctrl_pcp_device : public ace_sp_reelctrl_base_device
{
public:
	ace_sp_reelctrl_pcp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
};

DECLARE_DEVICE_TYPE(ACE_SP_REELCTRL, ace_sp_reelctrl_device)
DECLARE_DEVICE_TYPE(ACE_SP_REELCTRL_PCP, ace_sp_reelctrl_pcp_device)


#endif // #define MAME_MISC_ACE_SP_REELCTRL_H


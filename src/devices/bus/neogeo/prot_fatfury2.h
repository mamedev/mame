// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#ifndef MAME_BUS_NEOGEO_PROT_FATFURY2_H
#define MAME_BUS_NEOGEO_PROT_FATFURY2_H

#pragma once

#include "machine/alpha_8921.h"

DECLARE_DEVICE_TYPE(NG_FATFURY2_PROT, fatfury2_prot_device)


class fatfury2_prot_device :  public device_t
{
public:
	// construction/destruction
	fatfury2_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint16_t protection_r(offs_t offset);
	void protection_w(offs_t offset, uint16_t data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<alpha_8921_device> m_pro_ct0; // PRO-CT0 or SNK-9201
};

#endif // MAME_BUS_NEOGEO_PROT_FATFURY2_H

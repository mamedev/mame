// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#ifndef MAME_BUS_NEOGEO_PROT_FATFURY2_H
#define MAME_BUS_NEOGEO_PROT_FATFURY2_H

#pragma once

DECLARE_DEVICE_TYPE(NG_FATFURY2_PROT, fatfury2_prot_device)


class fatfury2_prot_device :  public device_t
{
public:
	// construction/destruction
	fatfury2_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint16_t protection_r(offs_t offset);
	void protection_w(offs_t offset, uint16_t data);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	uint32_t     m_prot_data;
};

#endif // MAME_BUS_NEOGEO_PROT_FATFURY2_H

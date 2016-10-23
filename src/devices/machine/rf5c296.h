// license:BSD-3-Clause
// copyright-holders:smf
#pragma once

#ifndef __RF5C296_H__
#define __RF5C296_H__

#include "pccard.h"

#define MCFG_RF5C296_SLOT(name) \
	rf5c296_device::set_pccard_name(*device, name);

class rf5c296_device : public device_t
{
public:
	rf5c296_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	static void set_pccard_name(device_t &device, const char *name) { downcast<rf5c296_device &>(device).m_pccard_name = name; }

	void io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t mem_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mem_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void reg_w(ATTR_UNUSED uint8_t reg, uint8_t data);
	uint8_t reg_r(ATTR_UNUSED uint8_t reg);

	unsigned char m_rf5c296_reg;
	pccard_slot_device *m_pccard;
	const char *m_pccard_name;
};

extern const device_type RF5C296;

#endif

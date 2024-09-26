// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_MACHINE_RF5C296_H
#define MAME_MACHINE_RF5C296_H

#pragma once

#include "bus/pccard/pccard.h"


class rf5c296_device : public device_t
{
public:
	rf5c296_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	template <typename T> void set_pccard(T &&tag) { m_pccard.set_tag(std::forward<T>(tag)); }

	void io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t io_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t mem_r(offs_t offset, uint16_t mem_mask = ~0);
	void mem_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void reg_w(uint8_t reg, uint8_t data);
	uint8_t reg_r(uint8_t reg);

	unsigned char m_rf5c296_reg;
	required_device<pccard_slot_device> m_pccard;
};

DECLARE_DEVICE_TYPE(RF5C296, rf5c296_device)

#endif // MAME_MACHINE_RF5C296_H

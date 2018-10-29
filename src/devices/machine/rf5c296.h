// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_MACHINE_RF5C296_H
#define MAME_MACHINE_RF5C296_H

#pragma once

#include "pccard.h"


class rf5c296_device : public device_t
{
public:
	rf5c296_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	template <typename T> void set_pccard(T &&tag) { m_pccard.set_tag(std::forward<T>(tag)); }

	DECLARE_WRITE16_MEMBER(io_w);
	DECLARE_READ16_MEMBER(io_r);
	DECLARE_READ16_MEMBER(mem_r);
	DECLARE_WRITE16_MEMBER(mem_w);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void reg_w(ATTR_UNUSED uint8_t reg, uint8_t data);
	uint8_t reg_r(ATTR_UNUSED uint8_t reg);

	unsigned char m_rf5c296_reg;
	required_device<pccard_slot_device> m_pccard;
};

DECLARE_DEVICE_TYPE(RF5C296, rf5c296_device)

#endif // MAME_MACHINE_RF5C296_H

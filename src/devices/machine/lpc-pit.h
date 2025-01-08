// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_LPC_PIT_H
#define MAME_MACHINE_LPC_PIT_H

#pragma once

#include "lpc.h"

class lpc_pit_device : public lpc_device {
public:
	lpc_pit_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

private:
	void map(address_map &map) ATTR_COLD;

	uint8_t status_r(offs_t offset);
	void access_w(offs_t offset, uint8_t data);
	void control_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(LPC_PIT, lpc_pit_device)

#endif // MAME_MACHINE_LPC_PIT_H

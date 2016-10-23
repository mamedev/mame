// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef LPC_PIT_H
#define LPC_PIT_H

#include "lpc.h"

#define MCFG_LPC_PIT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, LPC_PIT, 0)

class lpc_pit_device : public lpc_device {
public:
	lpc_pit_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	uint8_t status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void access_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	void device_start() override;
	void device_reset() override;

private:
	DECLARE_ADDRESS_MAP(map, 32);
};

extern const device_type LPC_PIT;

#endif

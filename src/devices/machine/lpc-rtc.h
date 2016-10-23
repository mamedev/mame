// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef LPC_RTC_H
#define LPC_RTC_H

#include "lpc.h"

#define MCFG_LPC_RTC_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, LPC_RTC, 0)

class lpc_rtc_device : public lpc_device {
public:
	lpc_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void map_extdevice(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space);

	uint8_t index_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void index_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t target_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void target_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t extindex_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void extindex_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t exttarget_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void exttarget_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	void device_start() override;
	void device_reset() override;

private:
	DECLARE_ADDRESS_MAP(map, 32);
	DECLARE_ADDRESS_MAP(extmap, 32);

	uint8_t cur_index, cur_extindex;
	uint8_t ram[256];
};

extern const device_type LPC_RTC;

#endif

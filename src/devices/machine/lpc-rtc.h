// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_LPC_RTC_H
#define MAME_MACHINE_LPC_RTC_H

#pragma once

#include "lpc.h"

class lpc_rtc_device : public lpc_device {
public:
	lpc_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void map_extdevice(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space);

protected:
	void device_start() override;
	void device_reset() override;

private:
	void map(address_map &map);
	void extmap(address_map &map);

	uint8_t cur_index, cur_extindex;
	uint8_t ram[256];

	DECLARE_READ8_MEMBER(  index_r);
	DECLARE_WRITE8_MEMBER( index_w);
	DECLARE_READ8_MEMBER(  target_r);
	DECLARE_WRITE8_MEMBER( target_w);
	DECLARE_READ8_MEMBER(  extindex_r);
	DECLARE_WRITE8_MEMBER( extindex_w);
	DECLARE_READ8_MEMBER(  exttarget_r);
	DECLARE_WRITE8_MEMBER( exttarget_w);
};

DECLARE_DEVICE_TYPE(LPC_RTC, lpc_rtc_device)

#endif // MAME_MACHINE_LPC_RTC_H

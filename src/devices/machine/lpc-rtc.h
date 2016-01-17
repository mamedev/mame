// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef LPC_RTC_H
#define LPC_RTC_H

#include "lpc.h"

#define MCFG_LPC_RTC_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, LPC_RTC, 0)

class lpc_rtc_device : public lpc_device {
public:
	lpc_rtc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual void map_device(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
							UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space) override;

	virtual void map_extdevice(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
									UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space);

	DECLARE_READ8_MEMBER(  index_r);
	DECLARE_WRITE8_MEMBER( index_w);
	DECLARE_READ8_MEMBER(  target_r);
	DECLARE_WRITE8_MEMBER( target_w);
	DECLARE_READ8_MEMBER(  extindex_r);
	DECLARE_WRITE8_MEMBER( extindex_w);
	DECLARE_READ8_MEMBER(  exttarget_r);
	DECLARE_WRITE8_MEMBER( exttarget_w);

protected:
	void device_start() override;
	void device_reset() override;

private:
	DECLARE_ADDRESS_MAP(map, 32);
	DECLARE_ADDRESS_MAP(extmap, 32);

	UINT8 cur_index, cur_extindex;
	UINT8 ram[256];
};

extern const device_type LPC_RTC;

#endif

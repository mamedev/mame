// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_I82371EB_ISA_H
#define MAME_MACHINE_I82371EB_ISA_H

#pragma once

#include "machine/i82371sb.h"
#include "machine/mc146818.h"

class i82371eb_isa_device : public i82371sb_isa_device
{
public:
	template <typename T>
	i82371eb_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: i82371eb_isa_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
	}

	i82371eb_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config & config) override;
	virtual void config_map(address_map &map) override ATTR_COLD;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void internal_io_map(address_map &map) override ATTR_COLD;
private:
	required_device<mc146818_device> m_rtc;

	u8 m_rtccfg;

	u8 m_rtc_index;
	template <unsigned E> u8 rtc_index_r(offs_t offset);
	template <unsigned E> void rtc_index_w(offs_t offset, u8 data);

	template <unsigned E> u8 rtc_data_r(offs_t offset);
	template <unsigned E> void rtc_data_w(offs_t offset, u8 data);
};

DECLARE_DEVICE_TYPE(I82371EB_ISA, i82371eb_isa_device)

#endif // MAME_MACHINE_I82371EB_ISA_H

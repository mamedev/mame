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
	i82371eb_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, bool internal_rtc = false)
		: i82371eb_isa_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
		m_has_internal_rtc = internal_rtc;
	}

	auto a20m() { return m_a20m_callback.bind(); }
	void a20gate_w(int state);

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
	devcb_write_line m_a20m_callback;

	u8 m_serirqc;
	u8 m_pdmacfg[2], m_pdma_ch[8];
	u8 m_ddmabp[2];
	u8 m_gencfg[4];
	u8 m_rtccfg;
	u8 pdmacfg_r(offs_t offset);
	void pdmacfg_w(offs_t offset, u8 data);

	bool m_has_internal_rtc;
	u8 m_rtc_index;
	template <unsigned E> u8 rtc_index_r(offs_t offset);
	template <unsigned E> void rtc_index_w(offs_t offset, u8 data);

	template <unsigned E> u8 rtc_data_r(offs_t offset);
	template <unsigned E> void rtc_data_w(offs_t offset, u8 data);

	u8 port92_r(offs_t offset);
	void port92_w(offs_t offset, u8 data);

	void fast_gatea20(int state);
	int m_ext_gatea20, m_fast_gatea20;
	u8 m_port92;
};

DECLARE_DEVICE_TYPE(I82371EB_ISA, i82371eb_isa_device)

#endif // MAME_MACHINE_I82371EB_ISA_H

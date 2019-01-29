// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_SPG110_H
#define MAME_MACHINE_SPG110_H

#pragma once

//#include "spg2xx.h"
#include "cpu/unsp/unsp.h"

class spg110_device : public device_t
{
public:
	spg110_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	spg110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T>
	spg110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: spg110_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
	}

	void map(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<unsp_device> m_cpu;
	//TIMER_CALLBACK_MEMBER(test_timer);
	//emu_timer *m_test_timer;
	DECLARE_READ16_MEMBER(spg110_2062_r);
	DECLARE_READ16_MEMBER(spg110_2063_r);
	DECLARE_WRITE16_MEMBER(spg110_2063_w);

	READ16_MEMBER(datasegment_r);
	WRITE16_MEMBER(datasegment_w);
};

DECLARE_DEVICE_TYPE(SPG110, spg110_device)

#endif // MAME_MACHINE_SPG110_H

// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_SEGA_SATURN_DCC_H
#define MAME_SEGA_SATURN_DCC_H

#pragma once

#include "cpu/sh/sh7604.h"

class saturn_dcc_device : public device_t
{
public:
	// construction/destruction
	saturn_dcc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_master_cpu(T &&tag) { m_master_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_slave_cpu(T &&tag) { m_slave_cpu.set_tag(std::forward<T>(tag)); }

	void minit_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sinit_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	IRQ_CALLBACK_MEMBER(irq_ack_cb);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<sh7604_device> m_master_cpu;
	required_device<sh7604_device> m_slave_cpu;

	void handle_frt_cb(s32 param);

	// amount of slices vs. SH-2 reference clock
	static constexpr int INTERLEAVE_DIV = 128;
	// quantum interleave duration, in usec
	static constexpr int INTERLEAVE_DURATION = 1666;
};

DECLARE_DEVICE_TYPE(SATURN_DCC, saturn_dcc_device)


#endif // MAME_SEGA_SATURN_DCC_H

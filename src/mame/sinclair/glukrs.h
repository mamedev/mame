// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub

#ifndef MAME_SINCLAIR_GLUKRS_H
#define MAME_SINCLAIR_GLUKRS_H

#pragma once

#include "machine/mc146818.h"


class glukrs_device : public mc146818_device
{
public:
	glukrs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void enable() { m_glukrs_active = true; }
	void disable() { m_glukrs_active = false; }
	bool is_active() { return m_glukrs_active; }

	u8 address_r() { return m_glukrs_active ? mc146818_device::get_address() : 0xff; }
	virtual void address_w(u8 address) override { if (m_glukrs_active) mc146818_device::address_w(address); }
	virtual u8 data_r() override { return m_glukrs_active ? mc146818_device::data_r() : 0xff; }
	virtual void data_w(u8 data) override { if (m_glukrs_active) { mc146818_device::data_w(data); } }

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

	virtual int data_size() const override { return 0xff; }

private:
	bool m_glukrs_active;
};

DECLARE_DEVICE_TYPE(GLUKRS, glukrs_device)
#endif // MAME_SINCLAIR_GLUKRS_H

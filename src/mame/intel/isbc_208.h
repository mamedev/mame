// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_INTEL_ISBC_208_H
#define MAME_INTEL_ISBC_208_H

#pragma once

#include "imagedev/floppy.h"
#include "machine/upd765.h"
#include "machine/am9517a.h"

class isbc_208_device : public device_t
{
public:
	template <typename T>
	isbc_208_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: isbc_208_device(mconfig, tag, owner, clock)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
	}

	isbc_208_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map) ATTR_COLD;
	uint8_t stat_r(offs_t offset);
	void aux_w(offs_t offset, uint8_t data);

	auto irq_callback() { return m_out_irq_func.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<am9517a_device> m_dmac;
	required_device<i8272a_device> m_fdc;

	devcb_write_line m_out_irq_func;
	u8 m_aux;
	u16 m_seg;
	address_space *m_maincpu_mem;

	void out_eop_w(int state);
	void hreq_w(int state);
	uint8_t dma_read_byte(offs_t offset);
	void dma_write_byte(offs_t offset, uint8_t data);
	void irq_w(int state);
};

DECLARE_DEVICE_TYPE(ISBC_208, isbc_208_device)

#endif // MAME_INTEL_ISBC_208_H

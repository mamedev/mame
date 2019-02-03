// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_MACHINE_ISBC_208_H
#define MAME_MACHINE_ISBC_208_H

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

	void map(address_map &map);
	DECLARE_READ8_MEMBER(stat_r);
	DECLARE_WRITE8_MEMBER(aux_w);

	auto irq_callback() { return m_out_irq_func.bind(); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<am9517a_device> m_dmac;
	required_device<i8272a_device> m_fdc;

	devcb_write_line m_out_irq_func;
	u8 m_aux;
	u16 m_seg;
	address_space *m_maincpu_mem;

	DECLARE_WRITE_LINE_MEMBER(out_eop_w);
	DECLARE_WRITE_LINE_MEMBER(hreq_w);
	DECLARE_READ8_MEMBER(dma_read_byte);
	DECLARE_WRITE8_MEMBER(dma_write_byte);
	DECLARE_WRITE_LINE_MEMBER(irq_w);
	DECLARE_FLOPPY_FORMATS(floppy_formats);
};

DECLARE_DEVICE_TYPE(ISBC_208, isbc_208_device)

#endif // MAME_MACHINE_ISBC_208_H

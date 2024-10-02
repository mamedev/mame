// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_FP1060IO_H
#define MAME_BUS_FP1060IO_H

#pragma once

#include "fp1000_exp.h"
#include "fp1060io_exp.h"
#include "machine/input_merger.h"

class fp1060io_device : public fp1000_exp_device
//, public device_single_card_slot_interface<device_fp1060io_slot_interface>
{
public:
	fp1060io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cs_w(offs_t offset, u8 data) override;
	virtual u8 id_r(offs_t offset) override;
	virtual void remap_cb() override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	required_device_array<fp1060io_exp_slot_device, 4> m_subslot;
	required_device_array<input_merger_device, 4> m_irqs_int;

	u8 m_slot_select = 0;
};


DECLARE_DEVICE_TYPE(FP1060IO, fp1060io_device)


#endif // MAME_BUS_FP1060IO_H

// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    PC-style floppy disk controller emulation

**********************************************************************/

#ifndef MAME_MACHINE_PC_FDC_H
#define MAME_MACHINE_PC_FDC_H

#pragma once

#include "machine/upd765.h"


class pc_fdc_family_device : public device_t {
public:
	auto intrq_wr_callback() { return intrq_cb.bind(); }
	auto drq_wr_callback() { return drq_cb.bind(); }

	virtual void map(address_map &map) = 0;

	void tc_w(bool state);
	uint8_t dma_r();
	void dma_w(uint8_t data);

	uint8_t dor_r();
	void dor_w(uint8_t data);
	void ccr_w(uint8_t data);

	required_device<upd765a_device> fdc;

protected:
	pc_fdc_family_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void irq_w(int state);
	void drq_w(int state);

	bool irq, drq, fdc_drq, fdc_irq;
	devcb_write_line intrq_cb, drq_cb;
	uint8_t dor;

	floppy_image_device *floppy[4];

	void check_irq();
	void check_drq();
};

class pc_fdc_xt_device : public pc_fdc_family_device {
public:
	pc_fdc_xt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override ATTR_COLD;
	void dor_fifo_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(PC_FDC_XT, pc_fdc_xt_device)

#endif // MAME_MACHINE_PC_FDC_H

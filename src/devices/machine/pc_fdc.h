// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    PC-style floppy disk controller emulation

**********************************************************************/

#ifndef MAME_MACHINE_PC_FDC_H
#define MAME_MACHINE_PC_FDC_H

#pragma once

#include "machine/upd765.h"


class pc_fdc_family_device : public pc_fdc_interface {
public:
	auto intrq_wr_callback() { return intrq_cb.bind(); }
	auto drq_wr_callback() { return drq_cb.bind(); }

	virtual void map(address_map &map) override;

	virtual void tc_w(bool state) override;
	virtual uint8_t dma_r() override;
	virtual void dma_w(uint8_t data) override;
	virtual uint8_t do_dir_r() override;

	READ8_MEMBER(dor_r);
	WRITE8_MEMBER(dor_w);
	READ8_MEMBER(dir_r);
	WRITE8_MEMBER(ccr_w);

	required_device<upd765a_device> fdc;

protected:
	pc_fdc_family_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( drq_w );

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

	virtual void map(address_map &map) override;
	WRITE8_MEMBER(dor_fifo_w);
};

class pc_fdc_at_device : public pc_fdc_family_device {
public:
	pc_fdc_at_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;
};

DECLARE_DEVICE_TYPE(PC_FDC_XT, pc_fdc_xt_device)
DECLARE_DEVICE_TYPE(PC_FDC_AT, pc_fdc_at_device)

#endif // MAME_MACHINE_PC_FDC_H

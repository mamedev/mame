// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_TEKTRONIX_TEK_MSU_FDC_H
#define MAME_TEKTRONIX_TEK_MSU_FDC_H

#pragma once

#include "cpu/m6502/m6502.h"
#include "imagedev/floppy.h"
#include "machine/nscsi_bus.h"
#include "machine/upd765.h"

class tek_msu_fdc_device
	: public nscsi_device
	, public nscsi_slot_card_interface
{
public:
	tek_msu_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void scsi_ctrl_changed() override;

private:
	void mem_map(address_map &map) ATTR_COLD;

	u8 status_r();
	u8 data_r();
	void control_w(u8 data);
	void data_w(u8 data);

	u8 fdc_r(offs_t offset);
	void fdc_w(offs_t offset, u8 data);
	void motor(int param);

	required_device<m6502_device> m_cpu;
	required_device<i8272a_device> m_fdc;
	required_device<floppy_connector> m_fdd;

	emu_timer *m_motor;

	bool m_minisel;
};

DECLARE_DEVICE_TYPE(TEK_MSU_FDC, tek_msu_fdc_device)

#endif // MAME_TEKTRONIX_TEK_MSU_FDC_H

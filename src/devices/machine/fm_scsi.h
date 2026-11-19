// license:BSD-3-Clause
// copyright-holders:Barry Rodewald, R. Belmont
/*
 * fm_scsi.h
 *
 * SCSI controller used in Fujitsu FMR-50, FMR-60, and FM-Towns
 *
 */

#ifndef MAME_MACHINE_FM_SCSI_H
#define MAME_MACHINE_FM_SCSI_H

#pragma once

#include "machine/nscsi_bus.h"


class fmscsi_device : public device_t, public nscsi_device_interface
{
public:
	fmscsi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto irq_handler() { return m_irq_handler.bind(); }
	auto drq_handler() { return m_drq_handler.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	// data register, also used by DMA
	uint8_t data_r();
	void data_w(uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void scsi_ctrl_changed() override;

private:
	uint8_t status_r();
	void control_w(uint8_t data);

	void ack_w(bool state);
	void set_int(bool state);
	void update_data();
	void update_irq();
	void update_drq();

	devcb_write_line m_irq_handler;
	devcb_write_line m_drq_handler;

	uint8_t m_data;
	uint8_t m_control;
	uint32_t m_ctrl;
	bool m_ack;
	bool m_int;
	bool m_irq;
	bool m_drq;
};

DECLARE_DEVICE_TYPE(FMSCSI, fmscsi_device)

#endif // MAME_MACHINE_FM_SCSI_H

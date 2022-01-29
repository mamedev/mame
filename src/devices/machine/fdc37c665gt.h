// license:BSD-3-Clause
// copyright-holders: Samuele Zannoli, windyfairy
/***************************************************************************

FDC37C665GT.h

SMSC FDC37C665GT High Performance Multi-Mode Parallel Port Super I/O Floppy Disk Controllers

***************************************************************************/

#ifndef MAME_MACHINE_FDC37C665GT_H
#define MAME_MACHINE_FDC37C665GT_H

#pragma once

// floppy disk controller
#include "machine/upd765.h"
#include "imagedev/floppy.h"
#include "formats/pc_dsk.h"
// parallel port
#include "machine/pc_lpt.h"
// serial port
#include "machine/ins8250.h"

class fdc37c665gt_device : public device_t
{
public:
	fdc37c665gt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: fdc37c665gt_device(mconfig, tag, owner, clock, upd765_family_device::mode_t::AT)
	{ }

	fdc37c665gt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, upd765_family_device::mode_t floppy_mode);

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	// to access io ports
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	auto fintr() { return m_fintr_callback.bind(); }
	auto fdrq() { return m_fdrq_callback.bind(); }
	auto txd1() { return m_txd1_callback.bind(); }
	auto ndtr1() { return m_ndtr1_callback.bind(); }
	auto nrts1() { return m_nrts1_callback.bind(); }
	auto txd2() { return m_txd2_callback.bind(); }
	auto ndtr2() { return m_ndtr2_callback.bind(); }
	auto nrts2() { return m_nrts2_callback.bind(); }

	// chip pins for uarts
	DECLARE_WRITE_LINE_MEMBER(rxd1_w);
	DECLARE_WRITE_LINE_MEMBER(ndcd1_w);
	DECLARE_WRITE_LINE_MEMBER(ndsr1_w);
	DECLARE_WRITE_LINE_MEMBER(nri1_w);
	DECLARE_WRITE_LINE_MEMBER(ncts1_w);
	DECLARE_WRITE_LINE_MEMBER(rxd2_w);
	DECLARE_WRITE_LINE_MEMBER(ndcd2_w);
	DECLARE_WRITE_LINE_MEMBER(ndsr2_w);
	DECLARE_WRITE_LINE_MEMBER(nri2_w);
	DECLARE_WRITE_LINE_MEMBER(ncts2_w);

protected:
	// device-level overrides
	virtual void device_start() override;

	// for the internal floppy controller
	DECLARE_WRITE_LINE_MEMBER(irq_floppy_w);

	// for the internal parallel port
	DECLARE_WRITE_LINE_MEMBER(irq_parallel_w);

	// for the internal uarts
	DECLARE_WRITE_LINE_MEMBER(irq_serial1_w);
	DECLARE_WRITE_LINE_MEMBER(txd_serial1_w);
	DECLARE_WRITE_LINE_MEMBER(dtr_serial1_w);
	DECLARE_WRITE_LINE_MEMBER(rts_serial1_w);
	DECLARE_WRITE_LINE_MEMBER(irq_serial2_w);
	DECLARE_WRITE_LINE_MEMBER(txd_serial2_w);
	DECLARE_WRITE_LINE_MEMBER(dtr_serial2_w);
	DECLARE_WRITE_LINE_MEMBER(rts_serial2_w);

private:
	// put your private members here
	enum OperatingMode
	{
		Run = 0,
		Configuration = 1
	} mode;

	enum LogicalDevice
	{
		FDC = 0,
		IDE,
		Parallel,
		Serial1,
		Serial2,

		LogicalDeviceEnd
	};

	int config_key_step;
	int config_index;
	bool enabled_logical[LogicalDevice::LogicalDeviceEnd];
	int device_addresses[LogicalDevice::LogicalDeviceEnd];
	int com_addresses[4];

	upd765_family_device::mode_t m_floppy_mode;

	uint8_t configuration_registers[16];

	devcb_write_line m_fintr_callback;
	devcb_write_line m_fdrq_callback;
	devcb_write_line m_pintr1_callback; // Parallel
	devcb_write_line m_irq3_callback; // Serial Port COM1/COM3
	devcb_write_line m_irq4_callback; // Serial Port COM2/COM4

	devcb_write_line m_txd1_callback;
	devcb_write_line m_ndtr1_callback;
	devcb_write_line m_nrts1_callback;
	devcb_write_line m_txd2_callback;
	devcb_write_line m_ndtr2_callback;
	devcb_write_line m_nrts2_callback;

	required_device<n82077aa_device> m_fdc;
	required_device_array<ns16550_device, 2> m_serial;
	required_device<pc_lpt_device> m_lpt;

	void write_configuration_register(int index, int data);
};

DECLARE_DEVICE_TYPE(FDC37C665GT, fdc37c665gt_device);

#endif // MAME_MACHINE_FDC37C665GT_H

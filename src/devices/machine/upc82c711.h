// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Universal Peripheral Controller 82C711 emulation

**********************************************************************/

#ifndef MAME_MACHINE_UPC82C711_H
#define MAME_MACHINE_UPC82C711_H

#pragma once

#include "machine/ins8250.h"
#include "machine/pc_lpt.h"
#include "machine/upd765.h"
#include "bus/ata/ataintf.h"
#include "imagedev/floppy.h"


// ======================> upc82c710_device

class upc82c711_device : public device_t
{
public:
	// construction/destruction
	upc82c711_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u16 io_r(offs_t offset, u16 mem_mask = ~0);
	void io_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u8 dack_r();
	void dack_w(u8 data);
	void tc_w(bool state);

	u8 dack_tc0_r() { tc_w(false); return dack_r(); }
	void dack_tc0_w(u8 data) { tc_w(false); dack_w(data); }
	u8 dack_tc1_r() { tc_w(true); return dack_r(); }
	void dack_tc1_w(u8 data) { tc_w(true); dack_w(data); }

	auto fintr() { return m_fintr_callback.bind(); }
	auto fdrq() { return m_fdrq_callback.bind(); }
	auto pintr() { return m_pintr_callback.bind(); }
	auto irq3() { return m_irq3_callback.bind(); }
	auto irq4() { return m_irq4_callback.bind(); }
	auto txd1() { return m_txd1_callback.bind(); }
	auto dtr1() { return m_dtr1_callback.bind(); }
	auto rts1() { return m_rts1_callback.bind(); }
	auto txd2() { return m_txd2_callback.bind(); }
	auto dtr2() { return m_dtr2_callback.bind(); }
	auto rts2() { return m_rts2_callback.bind(); }

	// chip pins for uarts
	void rxd1_w(int state);
	void dcd1_w(int state);
	void dsr1_w(int state);
	void ri1_w(int state);
	void cts1_w(int state);
	void rxd2_w(int state);
	void dcd2_w(int state);
	void dsr2_w(int state);
	void ri2_w(int state);
	void cts2_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<ata_interface_device> m_ide;
	required_device<upd765_family_device> m_fdc;
	required_device<pc_lpt_device> m_lpt;
	required_device_array<ns16450_device, 2> m_serial;

	void dor_w(u8 data);
	void fdc_irq_w(int state);
	void fdc_drq_w(int state);

	bool irq, drq, fdc_drq, fdc_irq;
	u8 dor;

	floppy_image_device *floppy[4];

	void check_irq();
	void check_drq();

	devcb_write_line m_fintr_callback;
	devcb_write_line m_fdrq_callback;
	devcb_write_line m_pintr_callback;
	devcb_write_line m_irq3_callback; // Serial Port COM1/COM3
	devcb_write_line m_irq4_callback; // Serial Port COM2/COM4

	devcb_write_line m_txd1_callback;
	devcb_write_line m_dtr1_callback;
	devcb_write_line m_rts1_callback;
	devcb_write_line m_txd2_callback;
	devcb_write_line m_dtr2_callback;
	devcb_write_line m_rts2_callback;

	void write_cfg(int index, u8 data);

	enum
	{
		DEVICE_FDC = 0,
		DEVICE_IDE,
		DEVICE_LPT,
		DEVICE_SER1,
		DEVICE_SER2,
		DEVICE_TOTAL
	};

	bool device_enabled[DEVICE_TOTAL];
	u16 device_address[DEVICE_TOTAL];

	u16 com_address[4];

	int m_cfg_mode;
	u8 m_cfg_regs[16];
	u8 m_cfg_indx;
};


DECLARE_DEVICE_TYPE(UPC82C711, upc82c711_device)

#endif // MAME_MACHINE_UPC82C711_H

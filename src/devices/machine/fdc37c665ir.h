// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_FDC37C665IR_H
#define MAME_MACHINE_FDC37C665IR_H

#pragma once

#include "bus/isa/isa.h"
#include "imagedev/floppy.h"
#include "machine/ins8250.h"
#include "machine/pc_lpt.h"
#include "machine/upd765.h"

class fdc37c665ir_device : public device_t,
						   public device_isa16_card_interface,
						   public device_memory_interface
{
public:
	fdc37c665ir_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~fdc37c665ir_device() {}

	void remap(int space_id, offs_t start, offs_t end) override;

	auto fintr() { return m_fintr_callback.bind(); }
	auto pintr1() { return m_pintr1_callback.bind(); }
	auto irq3() { return m_irq3_callback.bind(); }
	auto irq4() { return m_irq4_callback.bind(); }
	auto txd1() { return m_txd1_callback.bind(); }
	auto ndtr1() { return m_ndtr1_callback.bind(); }
	auto nrts1() { return m_nrts1_callback.bind(); }
	auto txd2() { return m_txd2_callback.bind(); }
	auto ndtr2() { return m_ndtr2_callback.bind(); }
	auto nrts2() { return m_nrts2_callback.bind(); }

	void rxd1_w(int state);
	void ndcd1_w(int state);
	void ndsr1_w(int state);
	void nri1_w(int state);
	void ncts1_w(int state);
	void rxd2_w(int state);
	void ndcd2_w(int state);
	void ndsr2_w(int state);
	void nri2_w(int state);
	void ncts2_w(int state);

	static void floppy_formats(format_registration &fr);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	const address_space_config m_space_config;

	required_device<n82077aa_device> m_fdc;
	required_device<pc_lpt_device> m_lpt;
	required_device_array<ns16550_device, 2> m_com;

	devcb_write_line m_fintr_callback;
//	devcb_write_line m_fdrq_callback;
	devcb_write_line m_pintr1_callback; // Parallel
	devcb_write_line m_irq3_callback; // Serial Port COM2/COM4
	devcb_write_line m_irq4_callback; // Serial Port COM1/COM3

	devcb_write_line m_txd1_callback;
	devcb_write_line m_ndtr1_callback;
	devcb_write_line m_nrts1_callback;
	devcb_write_line m_txd2_callback;
	devcb_write_line m_ndtr2_callback;
	devcb_write_line m_nrts2_callback;

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	void config_map(address_map &map) ATTR_COLD;

	void irq_floppy_w(int state);
	void drq_floppy_w(int state);

	void irq_parallel_w(int state);

	void irq_serial1_w(int state);
	void txd_serial1_w(int state);
	void dtr_serial1_w(int state);
	void rts_serial1_w(int state);
	void irq_serial2_w(int state);
	void txd_serial2_w(int state);
	void dtr_serial2_w(int state);
	void rts_serial2_w(int state);

	u8 m_index = 0;

	enum config_phase_t : u8 {
		LOCK_55_0,
		UNLOCK_55_1,
		UNLOCK_DATA,
		LOCK_CR
	};

	config_phase_t m_config_phase;
	u8 m_cr[0xd];

	u16 get_com_address(u8 setting);
};


DECLARE_DEVICE_TYPE(FDC37C665IR, fdc37c665ir_device);

#endif // MAME_MACHINE_FDC37C665IR_H

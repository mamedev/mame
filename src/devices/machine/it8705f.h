// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_IT8705F_H
#define MAME_MACHINE_IT8705F_H

#pragma once

#include "bus/isa/isa.h"
#include "imagedev/floppy.h"
#include "machine/8042kbdc.h"
#include "machine/ds128x.h"
#include "machine/ins8250.h"
#include "machine/pc_lpt.h"
#include "machine/upd765.h"

class it8705f_device : public device_t,
						 public device_isa16_card_interface,
						 public device_memory_interface
{
public:
	it8705f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~it8705f_device();

	void remap(int space_id, offs_t start, offs_t end) override;

	auto irq1() { return m_irq1_callback.bind(); }
	auto irq8() { return m_irq8_callback.bind(); }
	auto irq9() { return m_irq9_callback.bind(); }
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

	required_device<n82077aa_device> m_pc_fdc;
	required_device_array<ns16550_device, 2> m_pc_com;
	required_device<pc_lpt_device> m_pc_lpt;
	memory_view m_logical_view;

	devcb_write_line m_irq1_callback;
	devcb_write_line m_irq8_callback;
	devcb_write_line m_irq9_callback;
	devcb_write_line m_txd1_callback;
	devcb_write_line m_ndtr1_callback;
	devcb_write_line m_nrts1_callback;
	devcb_write_line m_txd2_callback;
	devcb_write_line m_ndtr2_callback;
	devcb_write_line m_nrts2_callback;

	u8 m_index = 0;
	u8 m_logical_index = 0;
	bool m_activate[9]{};

	u8 m_lock_sequence_index = 0;

	uint8_t read(offs_t offset);
	void write(offs_t offset, u8 data);

	void config_map(address_map &map) ATTR_COLD;

	void logical_device_select_w(offs_t offset, u8 data);
	template <unsigned N> u8 activate_r(offs_t offset);
	template <unsigned N> void activate_w(offs_t offset, u8 data);

	void request_irq(int irq, int state);
	void request_dma(int dreq, int state);

	u8 m_pc_fdc_irq_line = 6;
	u8 m_pc_fdc_drq_line = 2;
//  u8 m_pc_fdc_mode;
	u16 m_pc_fdc_address = 0x3f0;

	void irq_floppy_w(int state);
	void drq_floppy_w(int state);

	u8 m_pc_lpt_irq_line = 7;
	u8 m_pc_lpt_drq_line = 4;
//  u8 m_pc_lpt_mode;
	u16 m_pc_lpt_address = 0x378;

	void irq_parallel_w(int state);

	u16 m_pc_com_address[2]{};
	u8 m_pc_com_irq_line[2]{};
	u8 m_pc_com_control[2]{};

	void irq_serial1_w(int state);
	void txd_serial1_w(int state);
	void dtr_serial1_w(int state);
	void rts_serial1_w(int state);
	void irq_serial2_w(int state);
	void txd_serial2_w(int state);
	void dtr_serial2_w(int state);
	void rts_serial2_w(int state);

	template <unsigned N> u8 uart_address_r(offs_t offset);
	template <unsigned N> void uart_address_w(offs_t offset, u8 data);
	template <unsigned N> u8 uart_irq_r(offs_t offset);
	template <unsigned N> void uart_irq_w(offs_t offset, u8 data);
	template <unsigned N> u8 uart_config_r(offs_t offset);
	template <unsigned N> void uart_config_w(offs_t offset, u8 data);
};

DECLARE_DEVICE_TYPE(IT8705F, it8705f_device);

#endif // MAME_MACHINE_IT8705F_H

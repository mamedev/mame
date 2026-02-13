// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_IT8671F_H
#define MAME_MACHINE_IT8671F_H

#pragma once

#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/isa/isa.h"
#include "imagedev/floppy.h"
#include "machine/at_keybc.h"
#include "machine/ds128x.h"
#include "machine/ins8250.h"
#include "machine/pc_lpt.h"
#include "machine/upd765.h"

class it8671f_device : public device_t,
						 public device_isa16_card_interface,
						 public device_memory_interface
{
public:
	it8671f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~it8671f_device();

	void remap(int space_id, offs_t start, offs_t end) override;

	auto krst_gpio2() { return m_krst_callback.bind(); }
	auto ga20_gpio6() { return m_ga20_callback.bind(); }

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

	virtual uint8_t dack_r(int line) override;
	virtual void dack_w(int line, uint8_t data) override;
	virtual void eop_w(int state) override;
	void update_dreq_mapping(int dreq, int logical);

private:
	const address_space_config m_space_config;

	required_device<n82077aa_device> m_fdc;
	required_device_array<ns16550_device, 2> m_com;
	required_device<pc_lpt_device> m_lpt;
	required_device<ps2_keyboard_controller_device> m_keybc;
	required_device<pc_kbdc_device> m_ps2_con;
	required_device<pc_kbdc_device> m_aux_con;

	memory_view m_logical_view;

	devcb_write_line m_krst_callback; // GPIO2
	devcb_write_line m_ga20_callback; // GPIO6
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
	bool m_activate[8]{};
	int m_dreq_mapping[4];
	int m_last_dma_line;

	enum config_phase_t : u8 {
		WAIT_FOR_KEY,
		UNLOCK_PNP,
		MB_PNP_MODE
	};

	u8 m_lock_port_index = 0;
	u8 m_lock_sequence_index = 0;
	u16 m_port_select_index = 0;
	u16 m_port_select_data = 0;
	config_phase_t m_config_phase;
	u8 m_port_config[4];
	u8 m_unlock_sequence[32];

	uint8_t read(offs_t offset);
	void write(offs_t offset, u8 data);
	void port_select_w(offs_t offset, u8 data);
	void port_map(address_map &map);

	void config_map(address_map &map) ATTR_COLD;

	void logical_device_select_w(offs_t offset, u8 data);
	template <unsigned N> u8 activate_r(offs_t offset);
	template <unsigned N> void activate_w(offs_t offset, u8 data);

	void request_irq(int irq, int state);
	void request_dma(int dreq, int state);

	u8 m_fdc_irq_line = 6;
	u8 m_fdc_drq_line = 2;
//  u8 m_fdc_mode;
	u16 m_fdc_address = 0x3f0;
	u8 m_fdc_f0, m_fdc_f1;

	void irq_floppy_w(int state);
	void drq_floppy_w(int state);

	u8 m_lpt_irq_line = 7;
	u8 m_lpt_drq_line = 4;
//  u8 m_lpt_mode;
	u16 m_lpt_address = 0x378;

	void irq_parallel_w(int state);

	u16 m_com_address[2]{};
	u8 m_com_irq_line[2]{};
	u8 m_com_control[2]{};

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

	void cpu_reset_w(int state);
	void cpu_a20_w(int state);
	void irq_keyboard_w(int state);
	void irq_mouse_w(int state);

	u8 m_key_irq_line;
	u8 m_aux_irq_line;
};

DECLARE_DEVICE_TYPE(IT8671F, it8671f_device);

#endif // MAME_MACHINE_IT8671F_H

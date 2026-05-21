// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_I82091AA_H
#define MAME_MACHINE_I82091AA_H

#pragma once

#include "bus/isa/isa.h"
#include "imagedev/floppy.h"
#include "machine/ins8250.h"
#include "machine/pc_lpt.h"
#include "machine/upd765.h"

class i82091aa_device : public device_t,
						public device_isa16_card_interface,
						public device_memory_interface
{
public:
	i82091aa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual ~i82091aa_device();

	void remap(int space_id, offs_t start, offs_t end) override;

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
	address_space_config m_space_config;
	required_device<n82077aa_device> m_fdc;
	required_device_array<ns16550_device, 2> m_com;
	required_device<pc_lpt_device> m_lpt;

	devcb_write_line m_txd1_callback;
	devcb_write_line m_ndtr1_callback;
	devcb_write_line m_nrts1_callback;
	devcb_write_line m_txd2_callback;
	devcb_write_line m_ndtr2_callback;
	devcb_write_line m_nrts2_callback;

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	void config_map(address_map &map) ATTR_COLD;
	void request_irq(int irq, int state);
	void request_dma(int dreq, int state);

	u8 m_index;
	u8 m_fcfg1, m_fcfg2;
	u8 m_pcfg1, m_pcfg2;
	u8 m_sacfg1, m_sacfg2;
	u8 m_sbcfg1, m_sbcfg2;
	u8 m_idecfg;

	void irq_floppy_w(int state);
	void drq_floppy_w(int state);

	void irq_serial1_w(int state);
	void txd_serial1_w(int state);
	void dtr_serial1_w(int state);
	void rts_serial1_w(int state);
	void irq_serial2_w(int state);
	void txd_serial2_w(int state);
	void dtr_serial2_w(int state);
	void rts_serial2_w(int state);

	void irq_parallel_w(int state);
};

DECLARE_DEVICE_TYPE(I82091AA, i82091aa_device);

#endif // MAME_MACHINE_I82091AA_H

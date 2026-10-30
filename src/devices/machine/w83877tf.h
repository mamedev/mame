// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_W83877TF_H
#define MAME_MACHINE_W83877TF_H

#pragma once

#include "bus/isa/isa.h"
#include "imagedev/floppy.h"
#include "machine/ins8250.h"
#include "machine/pc_lpt.h"
#include "machine/upd765.h"

class w83877tf_device : public device_t,
						public device_isa16_card_interface,
						public device_memory_interface
{
public:
	w83877tf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~w83877tf_device();

	void remap(int space_id, offs_t start, offs_t end) override;

	void set_por_hefras(u8 value) { m_hefras = value; }
	void set_por_hefere(u8 value) { m_hefere = value; }

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

	devcb_write_line m_irq1_callback;
	devcb_write_line m_irq8_callback;
	devcb_write_line m_irq9_callback;
	devcb_write_line m_txd1_callback;
	devcb_write_line m_ndtr1_callback;
	devcb_write_line m_nrts1_callback;
	devcb_write_line m_txd2_callback;
	devcb_write_line m_ndtr2_callback;
	devcb_write_line m_nrts2_callback;

	u8 m_index;
	int m_dreq_mapping[4];
	int m_last_dma_line;

	u8 m_hefras, m_hefere;
	u8 m_lock_sequence;
	u8 m_ipd, m_pnpcvs;
	u8 m_clkinsel;

	u16 m_fdc_ad;
	u8 m_fdc_iqs;
	u8 m_fdc_dqs;
	u8 m_fdctri;
	u8 m_fdcpwd;
	u8 m_fipurdwm;
	u8 m_sel4fdd;
	u8 m_fddmode;
	u8 m_floppy_boot;
	u8 m_floppy_mediaid;
	u8 m_swwp;
	u8 m_disfddwr;
	u8 m_en3mode;
	u8 m_invertz;
	u8 m_fdd_mode;
	u8 m_abchg;

	u16 m_prt_ad;
	u8 m_prt_iqs, m_ecpirq;
	u8 m_prt_dqs;
	u8 m_prtmods;
	u8 m_prtpwd;
	u8 m_ecpfthr;
	u8 m_eppver;
	u8 m_prttri;

	u16 m_uart_ad[2];
	u8 m_uart_iqs[2];
	u8 m_irqin_iqs;
	u8 m_suamidi;
	u8 m_submidi;
	u8 m_uratri;
	u8 m_urbtri;
	u8 m_urapwd;
	u8 m_urbpwd;
	u8 m_rxw4c;
	u8 m_txw4c;
	u8 m_urirsel;
	u8 m_tura;
	u8 m_turb;
	u8 m_tx2inv;
	u8 m_rx2inv;
	u8 m_ir_mode;
	u8 m_hduplx;
	u8 m_sirrx;
	u8 m_sirtx;
	u8 m_fasta, m_fastb;

	u16 m_pm1_ad;

	u16 m_gpe_ad;

	uint8_t read(offs_t offset);
	void write(offs_t offset, u8 data);

	void config_map(address_map &map) ATTR_COLD;

	void pnp_init();

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

	void request_irq(int irq, int state);
	void request_dma(int dreq, int state);
};

DECLARE_DEVICE_TYPE(W83877TF, w83877tf_device);

#endif // MAME_MACHINE_W83877TF_H

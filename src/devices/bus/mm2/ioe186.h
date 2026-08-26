// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Nokia MikroMikko 2 IOE186 emulation

*********************************************************************/

#ifndef MAME_BUS_MM2_IOE186_H
#define MAME_BUS_MM2_IOE186_H

#pragma once

#include "exp.h"
#include "bus/rs232/rs232.h"
#include "machine/74259.h"
#include "machine/am9517a.h"
#include "machine/input_merger.h"
#include "machine/pit8253.h"
#include "machine/z80sio.h"

DECLARE_DEVICE_TYPE(NOKIA_IOE186, ioe186_device)

class ioe186_device : public device_t, public device_mikromikko2_expansion_bus_card_interface
{
public:
	ioe186_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void bhlda_w(int state, int bcas) override;

private:
	void map(address_map &map) ATTR_COLD;

	required_device<am9517a_device> m_dmac;
	required_device<i8274_device> m_mpsc;
	required_device<pit8253_device> m_pit;
	required_device<input_merger_device> m_irqs;
	required_device<ls259_device> m_ctrl;
	required_device<ls259_device> m_out;
	required_device<rs232_port_device> m_rs232a;
	required_device<rs232_port_device> m_rs232b;
	required_ioport m_sid;
	required_ioport m_sic;
	required_ioport m_s1;

	uint8_t input_r() { return !m_rs232a->dsr_r() | (!m_rs232b->dsr_r() << 1); }
	void page_w(uint8_t data) { m_page = data & 0x0f; }

	uint8_t dmac_mem_r(offs_t offset) { return m_bus->memspace().read_byte((m_page << 16) | offset); }
	void dmac_mem_w(offs_t offset, uint8_t data) { m_bus->memspace().write_byte((m_page << 16) | offset, data); }

	uint8_t mpsc_rxa_r(offs_t offset) { return m_mpsc->cd_ba_r(0); }
	void mpsc_txa_w(offs_t offset, uint8_t data) { m_mpsc->da_w(data); }
	uint8_t mpsc_rxb_r(offs_t offset) { return m_mpsc->cd_ba_r(1); }
	void mpsc_txb_w(offs_t offset, uint8_t data) { m_mpsc->db_w(data); }

	void hold_w(int state);
	void int_w(int state);

	void llba_w(int state) { m_llba = state; }
	void llbb_w(int state) { m_llbb = state; }
	void mpsc_txda_w(int state) { if (m_llba) m_mpsc->rxa_w(state); else m_rs232a->write_txd(state); }
	void mpsc_txdb_w(int state) { if (m_llbb) m_mpsc->rxb_w(state); else m_rs232b->write_txd(state); }
	void mpsc_rtsa_w(int state) { if (m_llba) m_mpsc->ctsa_w(state); else m_rs232a->write_rts(state); }
	void mpsc_rtsb_w(int state) { if (m_llbb) m_mpsc->ctsb_w(state); else m_rs232b->write_rts(state); }

	void cla0_w(int state) { m_cla0 = state; }
	void cla1_w(int state) { m_cla1 = state; }
	void clb0_w(int state) { m_clb0 = state; }
	void clb1_w(int state) { m_clb1 = state; }
	void tmr0_w(int state);
	void tmr1_w(int state);
	void tmr2_w(int state);

	void dack0_w(int state) { if (!state) m_dmac->dreq0_w(CLEAR_LINE); }
	void dack1_w(int state) { if (!state) m_dmac->dreq1_w(CLEAR_LINE); }
	void dack2_w(int state) { if (!state) m_dmac->dreq2_w(CLEAR_LINE); }
	void dack3_w(int state) { if (!state) m_dmac->dreq3_w(CLEAR_LINE); }

	u8 m_page;
	int m_hold_level;
	int m_int_level;

	bool m_llba;
	bool m_llbb;
	bool m_cla0;
	bool m_cla1;
	bool m_clb0;
	bool m_clb1;
};

#endif // MAME_BUS_MM2_IOE186_H

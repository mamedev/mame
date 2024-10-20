// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_DEC_DC7061_H
#define MAME_DEC_DC7061_H

#pragma once

#include "machine/nscsi_bus.h"

class dc7061_device
	: public nscsi_device
	, public nscsi_slot_card_interface
{
public:
	dc7061_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// device configuration
	auto sys_int() { return m_sys_int.bind(); }
	auto dma_r() { return m_dma_r.bind(); }
	auto dma_w() { return m_dma_w.bind(); }

	// register access
	void map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// ncsci_device implementation
	virtual void scsi_ctrl_changed() override;

	// register read handlers
	u16 sdb_r();
	u16 sc1_r();
	u16 sc2_r();
	u16 csr_r();
	u16 id_r();
	u16 slcsr_r();
	u16 destat_r();
	u16 data_r();
	u16 dmctrl_r();
	u16 dmlotc_r();
	u16 dmaddrl_r();
	u16 dmaddrh_r();
	u16 dmabyte_r();
	u16 cstat_r();
	u16 dstat_r();
	u16 comm_r();
	u16 dictrl_r();

	// register write handlers
	void sdb_w(u16 data);
	void sc1_w(u16 data);
	void sc2_w(u16 data);
	void csr_w(u16 data);
	void id_w(u16 data);
	void slcsr_w(u16 data);
	void data_w(u16 data);
	void dmctrl_w(u16 data);
	void dmlotc_w(u16 data);
	void dmaddrl_w(u16 data);
	void dmaddrh_w(u16 data);
	void dmabyte_w(u16 data);
	void cstat_w(u16 data);
	void dstat_w(u16 data);
	void comm_w(u16 data);
	void dictrl_w(u16 data);

	// state machine
	void state_timer(s32 param);
	int state_step();

	// other helpers
	void set_cstat(u16 data, u16 mask);
	void set_dstat(u16 data, u16 mask);
	void set_irq(bool irq_state);

private:
	devcb_write_line m_sys_int;
	devcb_read16 m_dma_r;
	devcb_write16 m_dma_w;

	// state machine
	emu_timer *m_state_timer;
	u32 m_state;

	// registers
	u16 m_sdb;
	u16 m_sc1;
	u16 m_sc2;
	u16 m_csr;
	u16 m_id;
	u16 m_slcsr;
	u16 m_destat;
	u16 m_data;
	u16 m_dmctrl;
	u16 m_dmlotc;
	u16 m_dmaddrl;
	u16 m_dmaddrh;
	u16 m_dmabyte;
	u16 m_cstat;
	u16 m_dstat;
	u16 m_comm;
	u16 m_dictrl;

	// line state
	u32 m_scsi_ctrl;
	bool m_sys_int_state;
};

DECLARE_DEVICE_TYPE(DC7061, dc7061_device)

#endif // MAME_DEC_DC7061_H

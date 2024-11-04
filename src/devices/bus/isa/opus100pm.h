// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_ISA_OPUS100PM_H
#define MAME_BUS_ISA_OPUS100PM_H

#pragma once

#include "cpu/ns32000/ns32000.h"
#include "machine/ns32081.h"
#include "machine/ns32082.h"
#include "machine/ram.h"

#include "bus/isa/isa.h"

class isa8_opus108pm_device
	: public device_t
	, public device_isa8_card_interface
{
public:
	isa8_opus108pm_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// device_t overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override;

	void map_cpu(address_map &map) ATTR_COLD;
	void map_isa(address_map &map) ATTR_COLD;

	// adapter register helpers
	u8 card_wait_r() { return 0xb2; }
	u8 card_stat_r();
	void card_rste_w(u8 data);
	void card_ack_w(u8 data);
	void card_irq_w(u8 data);

	// host register helpers
	u8 host_stat_r();
	void host_eirq_w(u8 data);
	void host_ack_w(u8 data);
	void host_int_w(u8 data);
	void host_nmi_w(u8 data);
	void host_go_w(u8 data);
	void host_run_w(u8 data);
	void host_rst_w(u8 data);

	void update_isa_irq(int state);

private:
	required_device<ns32016_device> m_cpu;
	required_device<ns32081_device> m_fpu;
	required_device<ns32082_device> m_mmu;
	required_device<ram_device> m_ram;

	required_ioport m_base;
	required_ioport m_irq;

	bool m_installed;

	u8 m_card_stat;
	u8 m_host_stat;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA8_OPUS108PM, isa8_opus108pm_device)

#endif // MAME_BUS_ISA_OPUS100PM_H

// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    VIA VT8231 South Bridge - PCI to ISA Bridge

***************************************************************************/

#ifndef MAME_MACHINE_VT8231_ISA_H
#define MAME_MACHINE_VT8231_ISA_H

#pragma once

#include "pci.h"
#include "machine/ins8250.h"
#include "machine/pic8259.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vt8231_isa_device

class vt8231_isa_device : public pci_device
{
public:
	vt8231_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto intr_cb() { return m_intr_cb.bind(); }
	auto com1_txd_cb() { return m_com1_txd_cb.bind(); }
	auto com1_dtr_cb() { return m_com1_dtr_cb.bind(); }
	auto com1_rts_cb() { return m_com1_rts_cb.bind(); }

	void com1_rxd_w(int state) { m_com1->rx_w(state); };
	void com1_dcd_w(int state) { m_com1->dcd_w(state); };
	void com1_dsr_w(int state) { m_com1->dsr_w(state); };
	void com1_ri_w(int state) { m_com1->ri_w(state); };
	void com1_cts_w(int state) { m_com1->cts_w(state); };

	// access from other pci functions
	void ide_irq_w(unsigned channel, int state);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	uint8_t isa_pnp_routing_1_r();
	void isa_pnp_routing_1_w(uint8_t data);
	uint8_t isa_pnp_routing_2_r();
	void isa_pnp_routing_2_w(uint8_t data);
	uint8_t isa_pnp_routing_3_r();
	void isa_pnp_routing_3_w(uint8_t data);
	uint8_t function_control_1_r();
	void function_control_1_w(uint8_t data);

	void io_map(address_map &map) ATTR_COLD;

	uint8_t misc_functions_r();
	void misc_functions_w(uint8_t data);

	uint8_t pic_slave_ack_r(offs_t offset);
	void irq_to_pic(int irq, int state);

	uint8_t superio_cfg_idx_r();
	void superio_cfg_idx_w(uint8_t data);
	uint8_t superio_cfg_data_r();
	void superio_cfg_data_w(uint8_t data);

	void com1_map(address_map &map) ATTR_COLD;
	uint16_t com1_baud_r();
	void com1_baud_w(uint16_t data);

	void com1_int_w(int state);
	void com1_txd_w(int state) { m_com1_txd_cb(state); };
	void com1_dtr_w(int state) { m_com1_dtr_cb(state); };
	void com1_rts_w(int state) { m_com1_rts_cb(state); };

	required_device_array<pic8259_device, 2> m_pic;
	required_device<ns16550_device> m_com1;

	devcb_write_line m_intr_cb;
	devcb_write_line m_com1_txd_cb;
	devcb_write_line m_com1_dtr_cb;
	devcb_write_line m_com1_rts_cb;

	uint8_t m_isa_pnp_routing_1;
	uint8_t m_isa_pnp_routing_2;
	uint8_t m_isa_pnp_routing_3;
	uint8_t m_function_control_1;

	uint8_t m_misc_functions;
	uint8_t m_superio_cfg[0x10];
	uint8_t m_superio_cfg_idx;
	uint16_t m_baud_divisor;

	bool m_initialized;
};

// device type definition
DECLARE_DEVICE_TYPE(VT8231_ISA, vt8231_isa_device)

#endif // MAME_MACHINE_VT8231_ISA_H

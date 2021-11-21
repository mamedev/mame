// license:BSD-3-Clause
// copyright-holders:Brice Onken
// thanks-to:Patrick Mackinlay

/*
 * Sony CXD8452AQ WSC-SONIC3 APbus interface and Ethernet controller controller
 *
 * The SONIC3 is an APbus controller designed for interfacing the SONIC Ethernet
 * controller to the APbus while providing DMA capabilities. It also manages
 * RAM access for the SONIC. The host can write to SONIC's RAM, but not the other
 * way around.
 */

#ifndef MAME_MACHINE_CXD8452AQ_H
#define MAME_MACHINE_CXD8452AQ_H

#pragma once

#include "emu.h"

class cxd8452aq_device : public device_t, public device_memory_interface
{
public:
	cxd8452aq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map);

	template <typename... T>
	void set_apbus_address_translator(T &&...args) { m_apbus_virt_to_phys_callback.set(std::forward<T>(args)...); }

	auto irq_out() { return m_irq_handler.bind(); }
	void irq_w(int state)
	{
		if (state)
		{
			m_sonic3_reg.control |= 0x1;
		}
		else
		{
			m_sonic3_reg.control &= ~0x1;
		}
		m_irq_check->adjust(attotime::zero);
	}

	template <typename T>
	void set_bus(T &&tag, int spacenum) { m_bus.set_tag(std::forward<T>(tag), spacenum); }

protected:
	// overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual space_config_vector memory_space_config() const override;

	// Address maps
	void sonic_bus_map(address_map &map);
	address_space_config main_bus_config;
	address_space_config sonic_config;
	uint8_t sonic_r(offs_t offset, uint8_t mem_mask);
	void sonic_w(offs_t offset, uint8_t data, uint8_t mem_mask);

	// Register accessors
	uint32_t control_r(offs_t offset);
	void control_w(offs_t offset, uint32_t data);
	uint32_t tx_count_r(offs_t offset);
	void tx_count_w(offs_t offset, uint32_t data);
	uint32_t rx_count_r(offs_t offset);
	void rx_count_w(offs_t offset, uint32_t data);

	// Interrupt handling
	devcb_write_line m_irq_handler;
	bool m_irq = false;
	emu_timer *m_irq_check;
	TIMER_CALLBACK_MEMBER(irq_check);

	// APbus DMA
	// TODO: Actual frequency, since we don't have DRQ to implictly rate limit
	//       Might be the APbus frequency.
	const int DMA_TIMER = 100;
	device_delegate<uint32_t(uint32_t)> m_apbus_virt_to_phys_callback;
	required_address_space m_bus;
	emu_timer *m_dma_check;
	TIMER_CALLBACK_MEMBER(dma_check);

	struct sonic3_register_file
	{
		// General registers
		uint32_t control = 0x80000000;
		uint32_t config = 0x0;
		uint32_t revision = 0x3;

		// DMA registers
		uint32_t rx_sonic_address = 0x0;
		uint32_t rx_host_address = 0x0;
		uint32_t rx_count = 0x0;
		uint32_t tx_sonic_address = 0x0;
		uint32_t tx_host_address = 0x0;
		uint32_t tx_count = 0x0;
	} m_sonic3_reg;

	// control register constants
	const uint32_t INT_EN_MASK = 0x7f00;
	// TODO: determine if external interrupt should be included when cleared
	const uint32_t INT_CLR_MASK = 0xf0;
	const uint32_t RX_DMA_COMPLETE = 0x40;
	const uint32_t TX_DMA_COMPLETE = 0x20;
	const uint32_t EXT_INT = 0x1;

	// count register constants
	const uint32_t DMA_START = 0x80000000;
};

DECLARE_DEVICE_TYPE(CXD8452AQ, cxd8452aq_device)

#endif // MAME_MACHINE_CXD8452AQ_H

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

#ifndef MAME_SONY_CXD8452AQ_H
#define MAME_SONY_CXD8452AQ_H

#pragma once

class cxd8452aq_device : public device_t, public device_memory_interface
{
public:
	cxd8452aq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map) ATTR_COLD;

	template <typename... T>
	void set_apbus_address_translator(T &&...args) { m_apbus_virt_to_phys_callback.set(std::forward<T>(args)...); }

	template <typename T>
	void set_bus(T &&tag, int spacenum) { m_bus.set_tag(std::forward<T>(tag), spacenum); }

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

protected:
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

	// Address maps
	address_space_config main_bus_config;
	address_space_config sonic_config;

	// Interrupt handling
	devcb_write_line m_irq_handler;
	bool m_irq = false;
	emu_timer *m_irq_check;

	// Bus/DMA related members
	device_delegate<uint32_t(uint32_t)> m_apbus_virt_to_phys_callback;
	required_address_space m_bus;
	memory_access<64, 3, 0, ENDIANNESS_BIG>::cache m_main_cache;
	memory_access<32, 2, 0, ENDIANNESS_BIG>::cache m_net_cache;
	emu_timer *m_dma_check;

	// Bus configuration
	void sonic_bus_map(address_map &map) ATTR_COLD;

	// overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

	// Bus access
	uint8_t sonic_r(offs_t offset);
	void sonic_w(offs_t offset, uint8_t data);

	// Register accessors
	uint32_t control_r(offs_t offset);
	void control_w(offs_t offset, uint32_t data);
	uint32_t revision_r(offs_t offset);
	uint32_t config_r(offs_t offset);
	void config_w(offs_t offset, uint32_t data);
	uint32_t rx_sonic_addr_r(offs_t offset);
	void rx_sonic_addr_w(offs_t offset, uint32_t data);
	uint32_t rx_host_addr_r(offs_t offset);
	void rx_host_addr_w(offs_t offset, uint32_t data);
	uint32_t tx_sonic_addr_r(offs_t offset);
	void tx_sonic_addr_w(offs_t offset, uint32_t data);
	uint32_t tx_host_addr_r(offs_t offset);
	void tx_host_addr_w(offs_t offset, uint32_t data);
	uint32_t tx_count_r(offs_t offset);
	void tx_count_w(offs_t offset, uint32_t data);
	uint32_t rx_count_r(offs_t offset);
	void rx_count_w(offs_t offset, uint32_t data);

	// Callback methods
	TIMER_CALLBACK_MEMBER(irq_check);
	TIMER_CALLBACK_MEMBER(dma_check);
};

DECLARE_DEVICE_TYPE(CXD8452AQ, cxd8452aq_device)

#endif // MAME_SONY_CXD8452AQ_H

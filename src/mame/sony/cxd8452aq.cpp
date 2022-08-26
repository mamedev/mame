// license:BSD-3-Clause
// copyright-holders:Brice Onken
// thanks-to:Patrick Mackinlay

/*
 * Sony CXD8452AQ WSC-SONIC3 APbus interface and Ethernet controller controller
 *
 * TODO:
 *  - Error-related interrupts
 */

#include "emu.h"
#include "cxd8452aq.h"

#define LOG_REGISTERS (1U << 1)
#define LOG_INTERRUPT (1U << 2)
#define LOG_DATA (1U << 3)

#define SONIC3_DEBUG (LOG_GENERAL | LOG_REGISTERS | LOG_INTERRUPT)
#define SONIC3_TRACE (SONIC3_DEBUG | LOG_DATA)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(CXD8452AQ, cxd8452aq_device, "cxd8452aq", "Sony CXD8452AQ WSC-SONIC3")

namespace
{
	// control register constants
	static constexpr uint32_t INT_EN_MASK = 0x7f00;

	// TODO: determine if external interrupt should be included when cleared
	static constexpr uint32_t INT_CLR_MASK = 0xf0;
	static constexpr uint32_t RX_DMA_COMPLETE = 0x40;
	static constexpr uint32_t TX_DMA_COMPLETE = 0x20;
	static constexpr uint32_t EXT_INT = 0x1;

	// count register constants
	static constexpr uint32_t DMA_START = 0x80000000;

	// DMA update timer
	// TODO: Actual frequency, since we don't have DRQ to implictly rate limit
	//       Might be the APbus frequency.
	static constexpr int DMA_TIMER = 100;
}

cxd8452aq_device::cxd8452aq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CXD8452AQ, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	main_bus_config("main_bus", ENDIANNESS_BIG, 32, 32, 0),
	sonic_config("sonic", ENDIANNESS_BIG, 32, 32, 0, address_map_constructor(FUNC(cxd8452aq_device::sonic_bus_map), this)),
	m_irq_handler(*this),
	m_apbus_virt_to_phys_callback(*this),
	m_bus(*this, finder_base::DUMMY_TAG, -1, 64)
{
}

void cxd8452aq_device::map(address_map &map)
{
	map(0x00, 0x03).rw(FUNC(cxd8452aq_device::control_r), FUNC(cxd8452aq_device::control_w));
	map(0x04, 0x07).rw(FUNC(cxd8452aq_device::config_r), FUNC(cxd8452aq_device::config_w));
	map(0x08, 0x0b).r(FUNC(cxd8452aq_device::revision_r));
	map(0x0c, 0x0f).rw(FUNC(cxd8452aq_device::rx_sonic_addr_r), FUNC(cxd8452aq_device::rx_sonic_addr_w));
	map(0x10, 0x13).rw(FUNC(cxd8452aq_device::rx_host_addr_r), FUNC(cxd8452aq_device::rx_host_addr_w));
	map(0x14, 0x17).rw(FUNC(cxd8452aq_device::rx_count_r), FUNC(cxd8452aq_device::rx_count_w));
	map(0x18, 0x1b).rw(FUNC(cxd8452aq_device::tx_sonic_addr_r), FUNC(cxd8452aq_device::tx_sonic_addr_w));
	map(0x1c, 0x1f).rw(FUNC(cxd8452aq_device::tx_host_addr_r), FUNC(cxd8452aq_device::tx_host_addr_w));
	map(0x20, 0x23).rw(FUNC(cxd8452aq_device::tx_count_r), FUNC(cxd8452aq_device::tx_count_w));
}

void cxd8452aq_device::sonic_bus_map(address_map &map)
{
	map(0x0, 0xfffff).mirror(0xfff00000).rw(FUNC(cxd8452aq_device::sonic_r), FUNC(cxd8452aq_device::sonic_w));
}

void cxd8452aq_device::device_start()
{
	m_bus->cache(m_main_cache);
	space(0).cache(m_net_cache);
	m_irq_handler.resolve_safe();
	m_apbus_virt_to_phys_callback.resolve();
	m_irq_check = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cxd8452aq_device::irq_check), this));
	m_dma_check = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cxd8452aq_device::dma_check), this));

	save_item(NAME(m_sonic3_reg.control));
	save_item(NAME(m_sonic3_reg.config));
	save_item(NAME(m_sonic3_reg.revision));
	save_item(NAME(m_sonic3_reg.rx_sonic_address));
	save_item(NAME(m_sonic3_reg.rx_host_address));
	save_item(NAME(m_sonic3_reg.rx_count));
	save_item(NAME(m_sonic3_reg.tx_sonic_address));
	save_item(NAME(m_sonic3_reg.tx_host_address));
	save_item(NAME(m_sonic3_reg.tx_count));
	save_item(NAME(m_irq));
}

void cxd8452aq_device::device_reset() 
{
	m_sonic3_reg = {};
}

device_memory_interface::space_config_vector cxd8452aq_device::memory_space_config() const
{
	// Uses a similar trick that the Jazz MCT-ADR driver uses to translate accesses from the SONIC to the system bus
	return space_config_vector{
		std::make_pair(0, &main_bus_config),
		std::make_pair(1, &sonic_config)};
}

uint8_t cxd8452aq_device::sonic_r(offs_t offset)
{
	return m_net_cache.read_byte(offset);
}

void cxd8452aq_device::sonic_w(offs_t offset, uint8_t data)
{
	m_net_cache.write_byte(offset, data);
}

uint32_t cxd8452aq_device::control_r(offs_t offset)
{
	return m_sonic3_reg.control;
}

void cxd8452aq_device::control_w(offs_t offset, uint32_t data)
{
	LOGMASKED(LOG_REGISTERS, "set sonic3.control = 0x%x\n", data);

	// Clear and reprogram interrupt enable bits
	m_sonic3_reg.control &= ~INT_EN_MASK;
	m_sonic3_reg.control |= data & INT_EN_MASK;

	// Clear specified interrupts by getting 0s on interrupt status bits to clear
	m_sonic3_reg.control &= (~INT_CLR_MASK | (data & INT_CLR_MASK));
	m_irq_check->adjust(attotime::zero);
}

uint32_t cxd8452aq_device::revision_r(offs_t offset)
{
	return m_sonic3_reg.revision;
}

uint32_t cxd8452aq_device::config_r(offs_t offset)
{
	return m_sonic3_reg.config;
}

void cxd8452aq_device::config_w(offs_t offset, uint32_t data)
{
	LOGMASKED(LOG_REGISTERS, "set sonic3.config = 0x%x\n", data);
	m_sonic3_reg.config = data;
}

uint32_t cxd8452aq_device::rx_sonic_addr_r(offs_t offset)
{
	return m_sonic3_reg.rx_sonic_address;
}

void cxd8452aq_device::rx_sonic_addr_w(offs_t offset, uint32_t data)
{
	LOGMASKED(LOG_REGISTERS, "set sonic3.rx_sonic_address = 0x%x\n", data);
	m_sonic3_reg.rx_sonic_address = data;
}

uint32_t cxd8452aq_device::rx_host_addr_r(offs_t offset)
{
	return m_sonic3_reg.rx_host_address;
}

void cxd8452aq_device::rx_host_addr_w(offs_t offset, uint32_t data)
{
	LOGMASKED(LOG_REGISTERS, "set sonic3.rx_host_address = 0x%x\n", data);
	m_sonic3_reg.rx_host_address = data;
}

uint32_t cxd8452aq_device::tx_sonic_addr_r(offs_t offset)
{
	return m_sonic3_reg.tx_sonic_address;
}

void cxd8452aq_device::tx_sonic_addr_w(offs_t offset, uint32_t data)
{
	LOGMASKED(LOG_REGISTERS, "set sonic3.tx_sonic_address = 0x%x\n", data);
	m_sonic3_reg.tx_sonic_address = data;
}

uint32_t cxd8452aq_device::tx_host_addr_r(offs_t offset)
{
	return m_sonic3_reg.tx_host_address;
}

void cxd8452aq_device::tx_host_addr_w(offs_t offset, uint32_t data)
{
	LOGMASKED(LOG_REGISTERS, "set sonic3.tx_host_address = 0x%x\n", data);
	m_sonic3_reg.tx_host_address = data;
}

uint32_t cxd8452aq_device::tx_count_r(offs_t offset)
{
	return m_sonic3_reg.tx_count;
}

void cxd8452aq_device::tx_count_w(offs_t offset, uint32_t data)
{
	LOGMASKED(LOG_REGISTERS, "set sonic3.tx_count = 0x%x\n", data);

	m_sonic3_reg.tx_count = data;
	if (data & DMA_START)
	{
		m_dma_check->adjust(attotime::zero);
	}
	m_irq_check->adjust(attotime::zero);
}

uint32_t cxd8452aq_device::rx_count_r(offs_t offset)
{
	return m_sonic3_reg.rx_count;
}

void cxd8452aq_device::rx_count_w(offs_t offset, uint32_t data)
{
	LOGMASKED(LOG_REGISTERS, "set sonic3.rx_count = 0x%x\n", data);

	m_sonic3_reg.rx_count = data;
	if (data & DMA_START)
	{
		m_dma_check->adjust(attotime::zero);
	}
	m_irq_check->adjust(attotime::zero);
}

TIMER_CALLBACK_MEMBER(cxd8452aq_device::irq_check)
{
	bool newIrq = m_sonic3_reg.control & (EXT_INT | RX_DMA_COMPLETE | TX_DMA_COMPLETE);
	if (m_irq != newIrq)
	{
		LOGMASKED(LOG_INTERRUPT, "sonic3 interrupt changed to %d!\n", newIrq);
		m_irq = newIrq;
		m_irq_handler(newIrq);
	}
}

TIMER_CALLBACK_MEMBER(cxd8452aq_device::dma_check)
{
	bool rxDmaActive = m_sonic3_reg.rx_count > DMA_START;
	bool txDmaActive = m_sonic3_reg.tx_count > DMA_START;

	if (rxDmaActive && txDmaActive)
	{
		// TODO: is this an error? allow it for now, but log it.
		LOGMASKED(LOG_GENERAL, "WARNING: RX and TX DMA active at the same time, this may not be a valid condition.\n");
	}

	if (rxDmaActive)
	{
		// Move byte from SONIC RAM to main memory
		uint8_t data = m_net_cache.read_byte(m_sonic3_reg.rx_sonic_address & 0xffff);
		LOGMASKED(LOG_DATA, "sonic3.rx(0x%x -> 0x%x, data=0x%x)\n", m_sonic3_reg.rx_sonic_address, m_sonic3_reg.rx_host_address, data);
		m_main_cache.write_byte(m_apbus_virt_to_phys_callback(m_sonic3_reg.rx_host_address), data);
		m_sonic3_reg.rx_count -= 1;
		m_sonic3_reg.rx_sonic_address += 1;
		m_sonic3_reg.rx_host_address += 1;
		if (m_sonic3_reg.rx_count == DMA_START)
		{
			m_sonic3_reg.control |= RX_DMA_COMPLETE;

			// TODO: determine if rx_count is cleared here
			m_sonic3_reg.rx_count = 0;
			m_irq_check->adjust(attotime::zero);
		}
	}

	if (txDmaActive)
	{
		// Move byte from main memory to SONIC RAM
		uint8_t data = m_main_cache.read_byte(m_apbus_virt_to_phys_callback(m_sonic3_reg.tx_host_address));
		LOGMASKED(LOG_DATA, "sonic3.tx(0x%x -> 0x%x, data=0x%x)\n", m_sonic3_reg.tx_host_address, m_sonic3_reg.tx_sonic_address, data);
		m_net_cache.write_byte(m_sonic3_reg.tx_sonic_address & 0xffff, data);
		m_sonic3_reg.tx_count -= 1;
		m_sonic3_reg.tx_host_address += 1;
		m_sonic3_reg.tx_sonic_address += 1;
		if (m_sonic3_reg.tx_count == DMA_START)
		{
			m_sonic3_reg.control |= TX_DMA_COMPLETE;

			// TODO: determine if tx_count is cleared here
			m_sonic3_reg.tx_count = 0;
			m_irq_check->adjust(attotime::zero);
		}
	}

	if (m_sonic3_reg.rx_count > DMA_START || m_sonic3_reg.tx_count > DMA_START)
	{
		// Schedule another run if needed
		m_dma_check->adjust(attotime::from_nsec(DMA_TIMER));
	}
}

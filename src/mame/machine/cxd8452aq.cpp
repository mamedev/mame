// license:BSD-3-Clause
// copyright-holders:Brice Onken
// thanks-to:Patrick Mackinlay

/*
 * Sony CXD8452AQ WSC-SONIC3 APbus interface and Ethernet controller controller
 *
 * The SONIC3 is an APbus controller designed for interfacing the SONIC Ethernet
 * controller to the APbus while providing DMA capabilities.
 *
 * TODO:
 *  - Error-related interrupts
 */

#include "cxd8452aq.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(CXD8452AQ, cxd8452aq_device, "cxd8452aq", "Sony CXD8452AQ WSC-SONIC3")

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

device_memory_interface::space_config_vector cxd8452aq_device::memory_space_config() const
{
    // Uses the same trick that the Jazz MCT-ADR driver uses to translate accesses from the SONIC to the system bus
	return space_config_vector{
		std::make_pair(0, &main_bus_config),
		std::make_pair(1, &sonic_config)
	};
}

void cxd8452aq_device::map(address_map &map)
{
    map(0x00, 0x03).rw(FUNC(cxd8452aq_device::sonic3_r), FUNC(cxd8452aq_device::sonic3_w));
	map(0x04, 0x07).lrw32(NAME([this]() { LOG("read sonic3.config = 0x%x\n", sonic3_reg.config); return sonic3_reg.config; }), NAME([this](uint32_t data) { LOG("write sonic3.config = 0x%x\n", data); sonic3_reg.config = data; }));
	map(0x08, 0x0b).lrw32(NAME([this]() { LOG("read sonic3.revision = 0x%x\n", sonic3_reg.revision); return sonic3_reg.revision; }), NAME([this](uint32_t data) { LOG("write sonic3.revision = 0x%x, but it is probably write only?\n", data); /* sonic3_reg.revision = data; */ }));
    map(0x0c, 0x0f).lrw32(NAME([this]() { return sonic3_reg.rx_sonic_address; }), NAME([this](uint32_t data) { LOG("write sonic3.rx_sonic_address = 0x%x\n", data); sonic3_reg.rx_sonic_address = data; }));
    map(0x10, 0x13).lrw32(NAME([this]() { return sonic3_reg.rx_host_address; }), NAME([this](uint32_t data) { LOG("write sonic3.rx_host_address = 0x%x\n", data); sonic3_reg.rx_host_address = data; }));
    map(0x14, 0x17).rw(FUNC(cxd8452aq_device::rx_count_r), FUNC(cxd8452aq_device::rx_count_w));
    map(0x18, 0x1b).lrw32(NAME([this]() { return sonic3_reg.tx_sonic_address; }), NAME([this](uint32_t data) { LOG("write sonic3.tx_sonic_address = 0x%x\n", data); sonic3_reg.tx_sonic_address = data; }));
    map(0x1c, 0x1f).lrw32(NAME([this]() { return sonic3_reg.tx_host_address; }), NAME([this](uint32_t data) { LOG("write sonic3.tx_host_address = 0x%x\n", data); sonic3_reg.tx_host_address = data; }));
    map(0x20, 0x23).rw(FUNC(cxd8452aq_device::tx_count_r), FUNC(cxd8452aq_device::tx_count_w));
}

uint32_t cxd8452aq_device::sonic3_r(offs_t offset)
{
    LOG("read sonic3.sonic  = 0x%x\n", sonic3_reg.sonic);
    return sonic3_reg.sonic;
}

void cxd8452aq_device::sonic3_w(offs_t offset, uint32_t data)
{
    LOG("write sonic3.sonic = 0x%x\n", data);
    sonic3_reg.sonic &= ~INT_EN_MASK; // Clear interrupt enable bits
    sonic3_reg.sonic |= data & INT_EN_MASK;   // program interrupt enable bits
    sonic3_reg.sonic &= (~INT_CLR_MASK | (data & INT_CLR_MASK)); // 0s on interrupt status bits to clear
    m_irq_check->adjust(attotime::zero);
}

uint32_t cxd8452aq_device::tx_count_r(offs_t offset)
{
    LOG("read sonic3.tx_count = 0x%x\n", sonic3_reg.tx_count);
    return sonic3_reg.tx_count;
}

void cxd8452aq_device::tx_count_w(offs_t offset, uint32_t data)
{
    LOG("write sonic3.tx_count = 0x%x\n", data);
    sonic3_reg.tx_count = data;
    if(data & DMA_START)
    {
        m_dma_check->adjust(attotime::zero);
    }
    m_irq_check->adjust(attotime::zero);
}

uint32_t cxd8452aq_device::rx_count_r(offs_t offset)
{
    LOG("read sonic3.rx_count = 0x%x\n", sonic3_reg.rx_count);
    return sonic3_reg.rx_count;
}   

void cxd8452aq_device::rx_count_w(offs_t offset, uint32_t data)
{
    LOG("write sonic3.rx_count = 0x%x\n", data);
    sonic3_reg.rx_count = data;
    if(data & DMA_START)
    {
        m_dma_check->adjust(attotime::zero);
    }
    m_irq_check->adjust(attotime::zero);
}

void cxd8452aq_device::sonic_bus_map(address_map &map)
{
	map(0x00000000U, 0xffffffffU).rw(FUNC(cxd8452aq_device::sonic_r), FUNC(cxd8452aq_device::sonic_w));
}

uint8_t cxd8452aq_device::sonic_r(offs_t offset, uint8_t mem_mask)
{
    auto result = space(0).read_byte(offset & 0xfffff);
    // LOG("sonic_r[0x%x (0x%x) | 0x%x] = 0x%x\n", offset, offset & 0xfffff, mem_mask, result);
    return result;
}

void cxd8452aq_device::sonic_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	// LOG("sonic_w[0x%x (0x%x) | 0x%x]\n", offset, offset & 0xfffff, mem_mask);
    space(0).write_byte(offset & 0xfffff, data);
}

TIMER_CALLBACK_MEMBER(cxd8452aq_device::irq_check)
{
    bool newIrq = sonic3_reg.sonic & (EXT_INT | RX_DMA_COMPLETE | TX_DMA_COMPLETE);
    if (m_irq != newIrq)
    {
		LOG("WSC-SONIC3 interrupt changed to %d!\n", newIrq);
		m_irq = newIrq;
		m_irq_handler(newIrq);
	}
}

TIMER_CALLBACK_MEMBER(cxd8452aq_device::dma_check)
{
    // Check to see if we should be doing DMA
    bool rxDmaActive = sonic3_reg.rx_count > DMA_START;
    bool txDmaActive = sonic3_reg.tx_count > DMA_START;
    if (rxDmaActive && txDmaActive) // TODO: is this an error? allow it for now, but log it.
    {
        LOG("WARNING: RX and TX DMA active at the same time, this may not be a valid condition.\n");
    }

    if (rxDmaActive)
    {
        // Move byte from the SONIC RAM region to main memory
        uint8_t data = space(0).read_byte(sonic3_reg.rx_sonic_address & 0xffff);
        LOG("sonic3.rx(0x%x -> 0x%x, data=0x%x)\n", sonic3_reg.rx_sonic_address, sonic3_reg.rx_host_address, data);
        m_bus->write_byte(m_apbus_virt_to_phys_callback(sonic3_reg.rx_host_address), data);
        sonic3_reg.rx_count -= 1;
        sonic3_reg.rx_sonic_address += 1;
        sonic3_reg.rx_host_address += 1;
        if(sonic3_reg.rx_count == DMA_START)
        {
            sonic3_reg.sonic |= RX_DMA_COMPLETE;
            sonic3_reg.rx_count = 0; // XXX
            m_irq_check->adjust(attotime::zero);
        }
    }
    if (txDmaActive)
    {
        // Move byte from main memory to the SONIC RAM
        uint8_t data = m_bus->read_byte(m_apbus_virt_to_phys_callback(sonic3_reg.tx_host_address));
        LOG("sonic3.tx(0x%x -> 0x%x, data=0x%x)\n", sonic3_reg.tx_host_address, sonic3_reg.tx_sonic_address, data);
        space(0).write_byte(sonic3_reg.tx_sonic_address & 0xffff, data);
        sonic3_reg.tx_count -= 1;
        sonic3_reg.tx_host_address += 1;
        sonic3_reg.tx_sonic_address += 1;
        if(sonic3_reg.tx_count == DMA_START)
        {
            sonic3_reg.sonic |= TX_DMA_COMPLETE;
            sonic3_reg.tx_count = 0; // XXX
            m_irq_check->adjust(attotime::zero);
        }
    }

    if(sonic3_reg.rx_count > DMA_START || sonic3_reg.tx_count > DMA_START)
    {
        // Schedule another run if needed
        m_dma_check->adjust(attotime::from_usec(DMA_TIMER));
    }
}

void cxd8452aq_device::device_start()
{
    m_irq_handler.resolve_safe();
    m_apbus_virt_to_phys_callback.resolve();
    m_irq_check = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cxd8452aq_device::irq_check), this));
    m_dma_check = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cxd8452aq_device::dma_check), this));
}

void cxd8452aq_device::device_add_mconfig(machine_config &config) { }

void cxd8452aq_device::device_reset() { }

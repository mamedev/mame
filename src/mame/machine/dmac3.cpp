// license:BSD-3-Clause
// copyright-holders:Brice Onken,Tsubai Masanari
// thanks-to:Patrick Mackinlay

#include "emu.h"
#include "dmac3.h"

#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(DMAC3, dmac3_device, "dmac3", "Sony CXD8403Q DMA Controller")

dmac3_device::dmac3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DMAC3, tag, owner, clock), m_bus(*this, finder_base::DUMMY_TAG, -1, 64), m_irq_handler(*this), m_dma_r(*this), m_dma_w(*this)
{
}

void dmac3_device::device_start() 
{
	m_irq_handler.resolve_safe();
	m_dma_r.resolve_all_safe(0);
	m_dma_w.resolve_all_safe();

	m_irq_check = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dmac3_device::irq_check), this));
	m_dma_check = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dmac3_device::dma_check), this));
}

void dmac3_device::map_dma_ram(address_map &map)
{
	// Host platform configures the use of RAM at device attach
	map(0x0, MAP_RAM_SIZE - 1).ram();
}

void dmac3_device::set_base_map_address(uint32_t base_map_address)
{
	BASE_MAP_ADDRESS = base_map_address;
}

uint32_t dmac3_device::csr_r(DMAC3_Controller controller)
{
	uint32_t val = m_controllers[controller].csr;
	LOG("dmac3-%d csr_r: 0x%x\n", controller, val);
	return val;
}
uint32_t dmac3_device::intr_r(DMAC3_Controller controller)
{
	uint32_t val = m_controllers[controller].intr;
	LOG("dmac3-%d intr_r: 0x%x\n", controller, val);
	return val;
}
uint32_t dmac3_device::length_r(DMAC3_Controller controller)
{
	uint32_t val = m_controllers[controller].length;
	LOG("dmac3-%d length_r: 0x%x\n", controller, val);
	return val;
}
uint32_t dmac3_device::address_r(DMAC3_Controller controller)
{
	uint32_t val = m_controllers[controller].address;
	LOG("dmac3-%d address_r: 0x%x\n", controller, val);
	return val;
}
uint32_t dmac3_device::conf_r(DMAC3_Controller controller)
{
	uint32_t val = m_controllers[controller].conf;
	LOG("dmac3-%d conf_r: 0x%x\n", controller, val);
	return val;
}

void dmac3_device::csr_w(DMAC3_Controller controller, uint32_t data)
{
	LOG("dmac3-%d csr_w: 0x%x\n", controller, data);
	if(data & CSR_RESET)
	{
		LOG("dmac3-%d chip reset\n", controller);
		reset_controller(controller);
	}
	else
	{
		m_controllers[controller].csr = data;
	}
}

void dmac3_device::intr_w(DMAC3_Controller controller, uint32_t data)
{
	LOG("dmac3-%d intr_w: 0x%x\n", controller, data);
	auto intr_clear_bits = data & INTR_CLR_MASK; // Get 1s on bits to clear
	auto intr_enable_bits = data & INTR_EN_MASK; // Get 1s on bits to set
	m_controllers[controller].intr &= ~intr_clear_bits; // Clear requested interrupt flags
	m_controllers[controller].intr &= ~INTR_EN_MASK;	// Clear all mask bits
	m_controllers[controller].intr |= intr_enable_bits; // Set mask bits to new mask
}

void dmac3_device::length_w(DMAC3_Controller controller, uint32_t data)
{
	LOG("dmac3-%d length_w: 0x%x\n", controller, data);
	m_controllers[controller].length = data;
}

void dmac3_device::address_w(DMAC3_Controller controller, uint32_t data)
{
	LOG("dmac3-%d address_w: 0x%x\n", controller, data);
	m_controllers[controller].address = data;
}

void dmac3_device::conf_w(DMAC3_Controller controller, uint32_t data)
{
	// Log is polluted with switching between SPIFI3 and regular mode
	// Will probably remove the if at some point, but we can mostly trust all 3
	// DMAC+SPIFI3 users (MROM, NEWS-OS, and NetBSD) to follow this correctly
	if(data != CONF_FASTACCESS && data != CONF_SLOWACCESS)
	{
		LOG("dmac3-%d conf_w: 0x%x\n", controller, data);
	}
	m_controllers[controller].conf = data;
}

void dmac3_device::device_reset()
{
	// Reset both controllers
	reset_controller(DMAC3_Controller::CTRL0);
	reset_controller(DMAC3_Controller::CTRL1);
}

void dmac3_device::reset_controller(DMAC3_Controller controller)
{
	m_controllers[controller].csr = 0;
	m_controllers[controller].intr &= INTR_INT; // TODO: is the external interrupt bit preserved? I assume so...
	m_controllers[controller].length = 0;
	m_controllers[controller].address = 0;
	m_controllers[controller].conf = 0;
	m_irq_check->adjust(attotime::zero);
}

uint32_t dmac3_device::dmac3_virt_to_phys(uint32_t v_address)
{
	uint32_t dmac_page_address = BASE_MAP_ADDRESS + 8 * (v_address >> 12); // Convert page number to PTE address
	uint64_t raw_pte = m_bus->read_qword(dmac_page_address);
	dmac3_pte pte;
	pte.valid = (raw_pte & ENTRY_VALID) > 0;
	pte.coherent = (raw_pte & ENTRY_COHERENT) > 0;
	pte.pad2 = (raw_pte & ENTRY_PAD) >> ENTRY_PAD_SHIFT;
	pte.pfnum = raw_pte & ENTRY_PFNUM;

	if(!pte.valid)
	{
		fatalerror("DMAC3 TLB: out of universe!"); // TODO: is there an interrupt or something we can signal to the host instead?
	}

	return (pte.pfnum << 12) + (v_address & 0xFFF);
}

TIMER_CALLBACK_MEMBER(dmac3_device::irq_check)
{
	bool newIrq = false;
	
	// Scan each controller for an interrupt condition - if any of these are true, set IRQ.
	// If both controllers have no interrupt conditions, IRQ can be cleared.
	for (int controller = 0; controller < 2; ++controller)
	{
		uint32_t intr = m_controllers[controller].intr;
		newIrq |= ((intr & INTR_INT) > 0) && ((intr & INTR_INTEN) > 0); // External interrupt (SPIFI)
		newIrq |= ((intr & INTR_EOPI) > 0) && ((intr & INTR_EOPIE) > 0); // End-of-operation interrupt
		newIrq |= ((intr & INTR_DRQI) > 0) && ((intr & INTR_DRQIE) > 0); // DRQ interrupt (?)
		newIrq |= ((intr & INTR_TCI) > 0) && ((intr & INTR_TCIE) > 0); // Transfer count interrupt (?)
		newIrq |= (intr & (INTR_PERR)) > 0; // XXX DREQ, EOP?
	}

	if (m_irq != newIrq)
	{
		LOG("DMAC3 interrupt changed to %d!\n", newIrq);
		m_irq = newIrq;
		m_irq_handler(newIrq);
	}
}

TIMER_CALLBACK_MEMBER(dmac3_device::dma_check)
{
	bool active = false;
	for (int controller = 0; controller < 2; ++controller)
	{
		// Check if controller is active and has something to do
		if(!(m_controllers[controller].csr & CSR_ENABLE) || !(m_controllers[controller].length) || !m_controllers[controller].drq) { continue; }

		// We have data to send or receive, so we need to do the
		// virtual->physical address mapping to know where to go
		uint32_t address = dmac3_virt_to_phys(m_controllers[controller].address); // Bus physical address

		if(m_controllers[controller].csr & CSR_RECV)
		{
			// Device to memory
			uint8_t data = m_dma_r[controller]();
			LOG("dma_r data 0x%02x address 0x%08x count %d -> %d\n", data, address, m_controllers[controller].length, m_controllers[controller].length - 1);
			m_bus->write_byte(address, data);
		}
		else
		{
			// Memory to device
			uint8_t data = m_bus->read_byte(address);
			LOG("dma_w data 0x%02x address 0x%08x\n", data, address);
			m_dma_w[controller](data);
		}

		// Increment byte offset
		++m_controllers[controller].address;
		if ((m_controllers[controller].address & DMAC_PAGE_MASK) + 0x1000 > MAP_RAM_SIZE)
		{
			// XXX Is there an interrupt or some other way to handle this besides killing the emulation?
			fatalerror("DMAC3 address overflow! Ran beyond the bounds of the MAP RAM!");
		}

		// Decrement transfer count
		--m_controllers[controller].length;
		if(!m_controllers[controller].length)
		{
			m_controllers[controller].intr |= INTR_EOPI; // TODO: is this the right interrupt flag? Does TC also need to be set?
			m_irq_check->adjust(attotime::zero);
		}

		// If DRQ is still active, do it again
		if(m_controllers[controller].drq)
		{
			active = true;
		}
	}

	// If at least one controller wants to go again, do so
	if (active)
	{
		m_dma_check->adjust(attotime::zero);
	}
}

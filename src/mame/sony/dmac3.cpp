// license:BSD-3-Clause
// copyright-holders:Brice Onken,Tsubai Masanari
// thanks-to:Patrick Mackinlay

#include "emu.h"
#include "dmac3.h"

#define LOG_GENERAL (1U << 0)
#define LOG_REGISTER (1U << 1)
#define LOG_INTERRUPT (1U << 2)
#define LOG_DATA (1U << 3)

#define DMAC3_DEBUG (LOG_GENERAL | LOG_REGISTER | LOG_INTERRUPT)
#define DMAC3_TRACE (DMAC3_DEBUG | LOG_DATA)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(DMAC3, dmac3_device, "dmac3", "Sony CXD8403Q DMAC3 DMA Controller")

dmac3_device::dmac3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, DMAC3, tag, owner, clock), m_bus(*this, finder_base::DUMMY_TAG, -1, 64),
	m_irq_handler(*this),
	m_dma_r(*this),
	m_dma_w(*this),
	m_apbus_virt_to_phys_callback(*this)
{
}

void dmac3_device::device_start()
{
	m_apbus_virt_to_phys_callback.resolve();
	m_irq_handler.resolve_safe();
	m_dma_r.resolve_all_safe(0);
	m_dma_w.resolve_all_safe();

	m_irq_check = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dmac3_device::irq_check), this));
	m_dma_check = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dmac3_device::dma_check), this));

	save_item(STRUCT_MEMBER(m_controllers, csr));
	save_item(STRUCT_MEMBER(m_controllers, intr));
	save_item(STRUCT_MEMBER(m_controllers, length));
	save_item(STRUCT_MEMBER(m_controllers, address));
	save_item(STRUCT_MEMBER(m_controllers, conf));
	save_item(STRUCT_MEMBER(m_controllers, drq));
	save_item(NAME(m_irq));
}

void dmac3_device::device_reset()
{
	// Reset both controllers
	reset_controller(dmac3_controller::CTRL0);
	reset_controller(dmac3_controller::CTRL1);
}

void dmac3_device::reset_controller(dmac3_controller controller)
{
	m_controllers[controller].csr = 0;
	m_controllers[controller].intr &= INTR_INT; // TODO: is the external interrupt bit preserved?
	m_controllers[controller].length = 0;
	m_controllers[controller].address = 0;
	m_controllers[controller].conf = 0;
	m_irq_check->adjust(attotime::zero);
}

uint32_t dmac3_device::csr_r(dmac3_controller controller)
{
	return m_controllers[controller].csr;
}

uint32_t dmac3_device::intr_r(dmac3_controller controller)
{
	return m_controllers[controller].intr;
}

uint32_t dmac3_device::length_r(dmac3_controller controller)
{
	return m_controllers[controller].length;
}

uint32_t dmac3_device::address_r(dmac3_controller controller)
{
	return m_controllers[controller].address;
}

uint32_t dmac3_device::conf_r(dmac3_controller controller)
{
	return m_controllers[controller].conf;
}

void dmac3_device::csr_w(dmac3_controller controller, uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "dmac3-%d csr_w: 0x%x\n", controller, data);
	if (data & CSR_RESET)
	{
		LOGMASKED(LOG_GENERAL, "dmac3-%d chip reset\n", controller);
		reset_controller(controller);
	}
	else
	{
		m_controllers[controller].csr = data;
	}
}

void dmac3_device::intr_w(dmac3_controller controller, uint32_t data)
{
	static constexpr uint32_t INTR_CLR_MASK = (INTR_INT | INTR_TCI | INTR_EOP | INTR_EOPI | INTR_DREQ | INTR_DRQI | INTR_PERR);
	static constexpr uint32_t INTR_EN_MASK = (INTR_INTEN | INTR_TCIE | INTR_EOPIE | INTR_DRQIE);

	LOGMASKED(LOG_REGISTER, "dmac3-%d intr_w: 0x%x\n", controller, data);
	const auto intr_clear_bits = data & INTR_CLR_MASK;  // Get 1s on bits to clear
	const auto intr_enable_bits = data & INTR_EN_MASK;  // Get 1s on bits to set
	m_controllers[controller].intr &= ~intr_clear_bits; // Clear requested interrupt flags
	m_controllers[controller].intr &= ~INTR_EN_MASK;    // Clear all mask bits
	m_controllers[controller].intr |= intr_enable_bits; // Set mask bits to new mask
	m_irq_check->adjust(attotime::zero);
}

void dmac3_device::length_w(dmac3_controller controller, uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "dmac3-%d length_w: 0x%x (%s)\n", controller, data, machine().describe_context());
	m_controllers[controller].length = data;
}

void dmac3_device::address_w(dmac3_controller controller, uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "dmac3-%d address_w: 0x%x (%s)\n", controller, data, machine().describe_context());
	m_controllers[controller].address = data;
}

void dmac3_device::conf_w(dmac3_controller controller, uint32_t data)
{
	if ((data & ~CONF_WIDTH) != (m_controllers[controller].conf & ~CONF_WIDTH))
	{
		LOGMASKED(LOG_REGISTER, "dmac3-%d conf_w: 0x%x (%s)\n", controller, data, machine().describe_context());
	}

	m_controllers[controller].conf = data;
}

TIMER_CALLBACK_MEMBER(dmac3_device::irq_check)
{
	bool newIrq = false;

	// Scan each controller for an interrupt condition - if any of these are true, set IRQ.
	// If both controllers have no interrupt conditions, IRQ can be cleared.
	for (const auto &controller : m_controllers)
	{
		const uint32_t intr = controller.intr;
		newIrq |= (intr & INTR_INT) && (intr & INTR_INTEN); // External interrupt (SPIFI)
		newIrq |= (intr & INTR_EOPI) && (intr & INTR_EOPIE); // End-of-operation interrupt
		newIrq |= (intr & INTR_DRQI) && (intr & INTR_DRQIE); // DRQ interrupt (?)
		newIrq |= (intr & INTR_TCI) && (intr & INTR_TCIE);   // Transfer count interrupt (?)
		newIrq |= (intr & INTR_PERR);                        // TODO: DREQ, EOP?
	}

	if (m_irq != newIrq)
	{
		LOGMASKED(LOG_INTERRUPT, "Interrupt set to %d\n", newIrq);
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
		if (!(m_controllers[controller].csr & CSR_ENABLE) || !(m_controllers[controller].length) || !m_controllers[controller].drq)
		{
			continue;
		}

		// If the MSB is set, use the address register as a physical address with no address mapping.
		// Otherwise, we must translate the address from an APbus virtual address to the physical address.
		// In this thread, one of the NetBSD port developers mentions that DMAC3 does support direct access (from 0xA0000000 up)
		// http://www.jp.netbsd.org/ja/JP/ml/port-mips-ja/200005/msg00005.html
		// Based on what NEWS-OS does, it seems like anything above 0x80000000 bypasses the DMA map.
		uint32_t address = m_controllers[controller].address;
		if (!(address & 0x80000000))
		{
			address = m_apbus_virt_to_phys_callback(address);
		}
		else
		{
			address = address & ~0x80000000;
		}

		if (m_controllers[controller].csr & CSR_RECV)
		{
			// Device to memory
			const uint8_t data = m_dma_r[controller]();
			LOGMASKED(LOG_DATA, "dma_r data 0x%02x address 0x%08x count %d\n", data, address, m_controllers[controller].length);
			m_bus->write_byte(address, data);
		}
		else
		{
			// Memory to device
			const uint8_t data = m_bus->read_byte(address);
			LOGMASKED(LOG_DATA, "dma_w data 0x%02x address 0x%08x count %d\n", data, address, m_controllers[controller].length);
			m_dma_w[controller](data);
		}

		// Increment byte offset
		++m_controllers[controller].address;

		// Decrement transfer count
		--m_controllers[controller].length;
		if (!m_controllers[controller].length)
		{
			// Neither NEWS-OS nor NetBSD seem to expect an EOPI to be the main trigger of a DMA completion
			// In fact, NEWS-OS gets confused if EOPI is set but the SPIFI hasn't actually completed the transfer.
			// As such, for now at least, this driver doesn't set this flag to avoid disturbing the interrupt
			// handlers. More investigation and experimentation will be required for accurate emulation of this bit.
			// TODO: m_controllers[controller].intr |= INTR_EOPI;
			m_irq_check->adjust(attotime::zero);
		}

		// If DRQ is still active, do it again
		if (m_controllers[controller].drq)
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

// license:BSD-3-Clause
// copyright-holders:Brice Onken,Tsubai Masanari
// thanks-to:Patrick Mackinlay

#include "emu.h"
#include "dmac3.h"

#define LOG_GENERAL (1U << 0)
#define LOG_REGISTER (1U << 1)
#define LOG_INTERRUPT (1U << 2)
#define LOG_DATA  (1U << 3)

#define DMAC3_DEBUG (LOG_GENERAL|LOG_REGISTER|LOG_INTERRUPT)
#define DMAC3_TRACE (DMAC3_DEBUG|LOG_DATA)

// #define VERBOSE DMAC3_DEBUG
#include "logmacro.h"

DEFINE_DEVICE_TYPE(DMAC3, dmac3_device, "dmac3", "Sony CXD8403Q DMA Controller")

dmac3_device::dmac3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
    : device_t(mconfig, DMAC3, tag, owner, clock), m_bus(*this, finder_base::DUMMY_TAG, -1, 64), m_irq_handler(*this), m_dma_r(*this), m_dma_w(*this), m_apbus_virt_to_phys_callback(*this)
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
}

uint32_t dmac3_device::csr_r(DMAC3_Controller controller)
{
    uint32_t val = m_controllers[controller].csr;
    return val;
}
uint32_t dmac3_device::intr_r(DMAC3_Controller controller)
{
    uint32_t val = m_controllers[controller].intr;
    return val;
}
uint32_t dmac3_device::length_r(DMAC3_Controller controller)
{
    uint32_t val = m_controllers[controller].length;
    return val;
}
uint32_t dmac3_device::address_r(DMAC3_Controller controller)
{
    uint32_t val = m_controllers[controller].address;
    return val;
}
uint32_t dmac3_device::conf_r(DMAC3_Controller controller)
{
    uint32_t val = m_controllers[controller].conf;
    return val;
}

void dmac3_device::csr_w(DMAC3_Controller controller, uint32_t data)
{
    LOGMASKED(LOG_REGISTER, "dmac3-%d csr_w: 0x%x\n", controller, data);
    if(data & CSR_RESET)
    {
        LOGMASKED(LOG_GENERAL, "dmac3-%d chip reset\n", controller);
        reset_controller(controller);
    }
    else
    {
        m_controllers[controller].csr = data;
    }
}

void dmac3_device::intr_w(DMAC3_Controller controller, uint32_t data)
{
    LOGMASKED(LOG_REGISTER, "dmac3-%d intr_w: 0x%x\n", controller, data);
    auto intr_clear_bits = data & INTR_CLR_MASK; // Get 1s on bits to clear
    auto intr_enable_bits = data & INTR_EN_MASK; // Get 1s on bits to set
    m_controllers[controller].intr &= ~intr_clear_bits; // Clear requested interrupt flags
    m_controllers[controller].intr &= ~INTR_EN_MASK;	// Clear all mask bits
    m_controllers[controller].intr |= intr_enable_bits; // Set mask bits to new mask
    m_irq_check->adjust(attotime::zero);
    // TODO: does it make sense to clear INTR_INT??
}

void dmac3_device::length_w(DMAC3_Controller controller, uint32_t data)
{
    LOGMASKED(LOG_REGISTER, "dmac3-%d length_w: 0x%x\n", controller, data);
    m_controllers[controller].length = data;
}

void dmac3_device::address_w(DMAC3_Controller controller, uint32_t data)
{
    LOGMASKED(LOG_REGISTER, "dmac3-%d address_w: 0x%x (%s)\n", controller, data, machine().describe_context());
    m_controllers[controller].address = data;
}

void dmac3_device::conf_w(DMAC3_Controller controller, uint32_t data)
{
#if (VERBOSE & LOG_REGISTER) > 0 // No need for the extra comparison if logging isn't enabled
    // Only log if something other than the access mode changed
    // since, at least for now, DMAC and SPIFI accesses will go through regardless
    // of this setting.

    if((data & ~CONF_WIDTH) != (m_controllers[controller].conf & ~CONF_WIDTH))
    {
        LOGMASKED(LOG_REGISTER, "dmac3-%d conf_w: 0x%x\n", controller, data);
    }
#endif

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
    m_controllers[controller].intr &= INTR_INT; // TODO: is the external interrupt bit preserved?
    m_controllers[controller].length = 0;
    m_controllers[controller].address = 0;
    m_controllers[controller].conf = 0;
    m_irq_check->adjust(attotime::zero);
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
        if(!(m_controllers[controller].csr & CSR_ENABLE) || !(m_controllers[controller].length) || !m_controllers[controller].drq) { continue; }

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

        if(m_controllers[controller].csr & CSR_RECV)
        {
            // Device to memory
            uint8_t data = m_dma_r[controller]();
            LOGMASKED(LOG_DATA, "dma_r data 0x%02x address 0x%08x count %d\n", data, address, m_controllers[controller].length);
            m_bus->write_byte(address, data);
        }
        else
        {
            // Memory to device
            uint8_t data = m_bus->read_byte(address);
            LOGMASKED(LOG_DATA, "dma_w data 0x%02x address 0x%08x count %d\n", data, address, m_controllers[controller].length);
            m_dma_w[controller](data);
        }

        // Increment byte offset
        ++m_controllers[controller].address;

        // Decrement transfer count
        --m_controllers[controller].length;
        if(!m_controllers[controller].length)
        {
            // Neither NEWS-OS nor NetBSD seem to expect an EOPI to be the main trigger of a DMA completion
            // In fact, NEWS-OS gets confused if EOPI is set but the SPIFI hasn't actually completed the transfer.
            // As such, for now at least, this driver doesn't set this flag to avoid disturbing the interrupt
            // handlers. More investigation and experimentation will be required for accurate emulation of this bit.
            // m_controllers[controller].intr |= INTR_EOPI;
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

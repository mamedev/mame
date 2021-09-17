// license:BSD-3-Clause
// copyright-holders:Brice Onken,Tsubai Masanari
// thanks-to:Patrick Mackinlay

/*
 * Sony CXD8403Q DMAC3 DMA controller
 *
 * Register definitions were derived from the NetBSD source code, copyright (c) 2000 Tsubai Masanari.
 *
 * References:
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/dmac3reg.h
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/dmac3var.h
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/dmac3.c
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/spifi.c
 */

#ifndef MAME_MACHINE_DMAC3_H
#define MAME_MACHINE_DMAC3_H

#pragma once

class dmac3_device : public device_t
{
public:
    dmac3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

    // DMAC3 has two controllers on-chip
    // The 5000X uses controllers 0 and 1 for SPIFI/SCSI bus 0 and 1 respectively
    enum DMAC3_Controller
    {
        CTRL0 = 0,
        CTRL1 = 1,
    };

    // Address map setup
    void map_dma_ram(address_map &map);
    void set_base_map_address(uint32_t base_map_address);
    template <DMAC3_Controller controller> void map(address_map &map)
    {
        map(0x0, 0x3).rw(FUNC(dmac3_device::csr_r<controller>), FUNC(dmac3_device::csr_w<controller>));
        map(0x4, 0x7).rw(FUNC(dmac3_device::intr_r<controller>), FUNC(dmac3_device::intr_w<controller>));
        map(0x8, 0xb).rw(FUNC(dmac3_device::length_r<controller>), FUNC(dmac3_device::length_w<controller>));
        map(0xc, 0xf).rw(FUNC(dmac3_device::address_r<controller>), FUNC(dmac3_device::address_w<controller>));
        map(0x10, 0x13).rw(FUNC(dmac3_device::conf_r<controller>), FUNC(dmac3_device::conf_w<controller>));
    }

    // Signal routing
    template <typename T> void set_bus(T &&tag, int spacenum) { m_bus.set_tag(std::forward<T>(tag), spacenum); }
    template <DMAC3_Controller controller> auto dma_r_cb() { return m_dma_r[controller].bind(); }
	template <DMAC3_Controller controller> auto dma_w_cb() { return m_dma_w[controller].bind(); }
    auto irq_out() { return m_irq_handler.bind(); } // XXX Seems to be one IRQ pin for both controllers?

    template <DMAC3_Controller controller> void irq_w(int state)
    {
        if (state)
        {
            m_controllers[controller].intr |= INTR_INT;
        }
        else
        {
            m_controllers[controller].intr &= ~INTR_INT;
        }
        m_irq_check->adjust(attotime::zero);
    }

    template <DMAC3_Controller controller> void drq_w(int state)
    {
        m_controllers[controller].drq = (state != 0);
        m_dma_check->adjust(attotime::zero);
    }

protected:
    // DMAC3 requires off-board RAM to be allocated for the DMA map
    // The platform host controls this configuration.
    const uint32_t MAP_RAM_SIZE = 0x20000; // 128 kibibytes
    uint32_t BASE_MAP_ADDRESS; // Where do we start on the bus?

    // Overrides from device_t
    virtual void device_start() override;
    virtual void device_reset() override;

    // Connections to other devices and associated state
    required_address_space m_bus;
    devcb_write_line m_irq_handler;
    devcb_read8::array<2> m_dma_r; // XXX 32b? 64b?
	devcb_write8::array<2> m_dma_w; // XXX 32b? 64b?
    emu_timer *m_irq_check;
	emu_timer *m_dma_check;
    bool m_irq = false; // Chip-level IRQ

    // Misc functions
    void reset_controller(DMAC3_Controller controller);
    uint32_t dmac3_virt_to_phys(uint32_t v_address);
    TIMER_CALLBACK_MEMBER(irq_check);
    TIMER_CALLBACK_MEMBER(dma_check);

    /*
     * DMAC3 memory addressing
     * 
     * The DMAC3 has its own virtual->physical addressing scheme. Like the CPU's TLB, the host OS
     * is responsible for populating the DMAC's TLB. The `dmac3_pte` struct defines what each entry looks like. Note that to avoid
     * bit-order dependencies and other platform-specific stuff, in this implementation, valid, coherent, pad2, and pfnum are
     * separate variables, but the actual register is packed like this:
     * 
     * 0x xxxx xxxx xxxx xxxx
     *   |   pad   |  entry  |
     * 
     * Entry is packed as follows:
     * 0b    x      x     xxxxxxxxxx xxxxxxxxxxxxxxxxxxxx
     *    |valid|coherent|    pad2  |       pfnum        |
     * 
     * The DMAC3 requires RAM to hold the TLB. On the NWS-5000X, this is 128KiB starting at physical address 0x14c20000 (and goes to 0x14c3ffff)
     * 
     * The monitor ROM populates the PTEs as follows in response to a `dl` command.
     * Addr       PTE1             PTE2
     * 0x14c20000 0000000080103ff5 0000000080103ff6
     * 
     * It also loads the `address` register with 0xd60. 
     * 
     * This will cause the DMAC to start mapping from virtual address 0xd60 to physical address 0x3ff5d60. 
     * If the address register goes beyond 0xFFF, bit 12 will increment. This will increase the page number so the virtual address will be
     * 0x1000, and will cause the DMAC to use the next PTE (in this case, the next sequential page, 0x3ff6000).
     * 
     * NetBSD splits the mapping RAM into two sections, one for each DMAC controller. If the OS does not keep track, the DMACs
     * could end up in a configuration that would cause them to overwrite each other's data.
     * 
     * Another note: NetBSD mentions that the `pad2` section of the register is 10 bits. However, this might not be fully accurate.
     * On the NWS-5000X, the physical address bus is 36 bits because it has an R4400SC. The 32nd bit is sometimes set, depending
     * on the virtual address being used (maybe it goes to the memory controller). It doesn't impact the normal operation of the
     * computer, but does mean that the `pad2` section might only be 6 bits, not 10 bits.
     */
    
    // Page table entry structure
    struct dmac3_pte
    {
        uint32_t pad; // unused??
        bool valid;   // Entry is OK to use
        bool coherent;  // DMA coherence enabled (don't care for emulation??)
        uint32_t pad2;
        uint32_t pfnum; // Page number (upper 20 bits of physical address)
    };

    enum DMAC3_PTE_MASKS
    {
        ENTRY_VALID = 0x80000000,
        ENTRY_COHERENT = 0x40000000,
        ENTRY_PAD = 0x3FF00000,
        ENTRY_PAD_SHIFT = 20,
        ENTRY_PFNUM = 0xFFFFF
    };

    const uint32_t DMAC_PAGE_MASK = 0xFFF000; // Index into page map XXX might need to be adjusted

    // DMAC3 has two controllers on-chip
    struct dmac3_register_file
    {
        uint32_t csr = 0; // Status register
        uint32_t intr = 0; // Interrupt status register
        uint32_t length = 0; // Transfer count register
        uint32_t address = 0; // Starting byte offset XXX why is this incremented by 200000 for CTRL1 in NetBSD???
        uint32_t conf = 0; // Transaction configuration register
        bool drq = false; // XXX Is this something different from DREQ?
    } m_controllers[2];

     // Bitmasks for DMAC3 registers
    enum DMAC3_CSR_MASKS : uint32_t
    {
        CSR_SEND = 0x0000,
        CSR_ENABLE = 0x0001,
        CSR_RECV = 0x0002,
        CSR_RESET = 0x0004,
        CSR_APAD = 0x0008,
        CSR_MBURST = 0x0010,
        CSR_DBURST = 0x0020,
    };

    const uint32_t INTR_CLR_MASK = (/*INTR_INT |*/ INTR_TCI | INTR_EOP | INTR_EOPI | INTR_DREQ | INTR_DRQI | INTR_PERR); // TODO: are EOP, DREQ, PERR actually intr?
    const uint32_t INTR_EN_MASK = (INTR_INTEN | INTR_TCIE | INTR_EOPIE | INTR_DRQIE);
    enum DMAC3_INTR_MASKS : uint32_t
    {
        INTR_INT = 0x0001,
        INTR_INTEN = 0x0002,
        INTR_TCIE = 0x0020,
        INTR_TCI = 0x0040,
        INTR_EOP = 0x0100,
        INTR_EOPIE = 0x0200, // End of operation interrupt enable (guess)
        INTR_EOPI = 0x0400,
        INTR_DREQ = 0x1000, // Is this just DRQ? Or is this for triggering DMA requests to the host?
        INTR_DRQIE = 0x2000, // Interrupt on DRQ enable?
        INTR_DRQI = 0x4000,
        INTR_PERR = 0x8000,
    };

    // I'm not clear yet on what IPER, DERR, MPER are signalling
    // NetBSD ignores IPER and MPER, but resets the DMAC if DERR is asserted during the interrupt routine
    // DCEN and PCEN are set by NetBSD during attach (along with FASTACCESS)
    enum DMAC3_CONF_MASKS : uint32_t
    {
        CONF_IPER = 0x8000,
        CONF_MPER = 0x4000,
        CONF_PCEN = 0x2000, // parity check enable?
        CONF_DERR = 0x1000,
        CONF_DCEN = 0x0800,
        CONF_ODDP = 0x0200, // if I had to guess, odd parity?
        CONF_WIDTH = 0x00ff,
        CONF_SLOWACCESS = 0x0020, // SPIFI access mode (see NetBSD source code)
        CONF_FASTACCESS = 0x0001, // DMAC3 access mode (see NetBSD source code)
    };

    // Register file accessors
    uint32_t csr_r(DMAC3_Controller controller);
    uint32_t intr_r(DMAC3_Controller controller);
    uint32_t length_r(DMAC3_Controller controller);
    uint32_t address_r(DMAC3_Controller controller);
    uint32_t conf_r(DMAC3_Controller controller);

    void csr_w(DMAC3_Controller controller, uint32_t data);
    void intr_w(DMAC3_Controller controller, uint32_t data);
    void length_w(DMAC3_Controller controller, uint32_t data);
    void address_w(DMAC3_Controller controller, uint32_t data);
    void conf_w(DMAC3_Controller controller, uint32_t data);

    // Templates as partial functions for register file accessors since they can be bound at compile time
    template <DMAC3_Controller controller> uint32_t csr_r() { return csr_r(controller); }
    template <DMAC3_Controller controller> uint32_t intr_r() { return intr_r(controller); }
    template <DMAC3_Controller controller> uint32_t length_r() { return length_r(controller); }
    template <DMAC3_Controller controller> uint32_t address_r() { return address_r(controller); }
    template <DMAC3_Controller controller> uint32_t conf_r() { return conf_r(controller); }

    template <DMAC3_Controller controller> void csr_w(uint32_t data) { csr_w(controller, data); }
    template <DMAC3_Controller controller> void intr_w(uint32_t data) { intr_w(controller, data); }
    template <DMAC3_Controller controller> void length_w(uint32_t data) { length_w(controller, data); }
    template <DMAC3_Controller controller> void address_w(uint32_t data) { address_w(controller, data); }
    template <DMAC3_Controller controller> void conf_w(uint32_t data) { conf_w(controller, data); }
};

DECLARE_DEVICE_TYPE(DMAC3, dmac3_device)

#endif // MAME_MACHINE_DMAC3

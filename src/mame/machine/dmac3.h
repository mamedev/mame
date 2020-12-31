// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay, Brice Onken

/*
 * Sony DMAC3 DMA controller
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
    // Haven't determined yet if the DMA capabilities of the APBus and FIFO chips also use logic on the DMAC chip
    enum DMAC3_Controller
    {
        CTRL0 = 0,
        CTRL1 = 1,
    };

    // configuration
    template <typename T>
    void set_bus(T &&tag, int spacenum) { m_bus.set_tag(std::forward<T>(tag), spacenum); }
    auto out_int_cb() { return m_out_int.bind(); }

    // TODO: mechanism for DMAC3
    // template <unsigned Channel> auto dma_r_cb() { return m_dma_r[Channel].bind(); }
    // template <unsigned Channel> auto dma_w_cb() { return m_dma_w[Channel].bind(); }

    // TODO: mechanism for DMAC3
    // template <unsigned IRQ> void irq(int state) { set_irq_line(IRQ, state); }
    // template <unsigned DRQ> void drq(int state) { set_drq_line(DRQ, state); }

    // Address maps
    void map_dma_ram(address_map &map);
    template <DMAC3_Controller controller>
    void map(address_map &map)
    {
        map(0x0, 0x3).rw(FUNC(dmac3_device::cstat_r<controller>), FUNC(dmac3_device::cstat_w<controller>));
        map(0x4, 0x7).rw(FUNC(dmac3_device::ictl_r<controller>), FUNC(dmac3_device::ictl_w<controller>));
        map(0x8, 0xb).rw(FUNC(dmac3_device::trc_r<controller>), FUNC(dmac3_device::trc_w<controller>));
        map(0xc, 0xf).rw(FUNC(dmac3_device::tra_r<controller>), FUNC(dmac3_device::tra_w<controller>));
        map(0x10, 0x13).rw(FUNC(dmac3_device::cnf_r<controller>), FUNC(dmac3_device::cnf_w<controller>));
    }

protected:
    // device_t overrides
    virtual void device_start() override;
    virtual void device_reset() override;

    void set_irq_line(int number, int state);
    void set_drq_line(int channel, int state);

    void irq_check(void *ptr = nullptr, s32 param = 0);
    void dma_check(void *ptr = nullptr, s32 param = 0);

    required_address_space m_bus;

    devcb_write_line m_out_int;
    devcb_read32::array<2> m_dma_r;
    devcb_write32::array<2> m_dma_w;

    emu_timer *m_irq_check;
    emu_timer *m_dma_check;

    // DMAC3 requires off-board RAM to be allocated for the DMA map
    // The platform host controls this configuration.
    const uint32_t map_ram_size = 0x20000; // 128 kibibytes

    // DMAC3 has two controllers on-chip
    struct dmac3_register_file
    {
        uint32_t cstat = 0;
        uint32_t ictl = 0;
        uint32_t trc = 0;
        uint32_t tra = 0;
        uint32_t cnf = 0;
    } m_controllers[2];

    // Register file accessors
    uint32_t cstat_r(DMAC3_Controller controller);
    uint32_t ictl_r(DMAC3_Controller controller);
    uint32_t trc_r(DMAC3_Controller controller);
    uint32_t tra_r(DMAC3_Controller controller);
    uint32_t cnf_r(DMAC3_Controller controller);

    void cstat_w(DMAC3_Controller controller, uint32_t data);
    void ictl_w(DMAC3_Controller controller, uint32_t data);
    void trc_w(DMAC3_Controller controller, uint32_t data);
    void tra_w(DMAC3_Controller controller, uint32_t data);
    void cnf_w(DMAC3_Controller controller, uint32_t data);

    // Templates as partial functions for register file accessors
    template <DMAC3_Controller controller>
    uint32_t cstat_r() { return cstat_r(controller); }
    template <DMAC3_Controller controller>
    uint32_t ictl_r() { return ictl_r(controller); }
    template <DMAC3_Controller controller>
    uint32_t trc_r() { return trc_r(controller); }
    template <DMAC3_Controller controller>
    uint32_t tra_r() { return tra_r(controller); }
    template <DMAC3_Controller controller>
    uint32_t cnf_r() { return cnf_r(controller); }

    template <DMAC3_Controller controller>
    void cstat_w(uint32_t data) { cstat_w(controller, data); }
    template <DMAC3_Controller controller>
    void ictl_w(uint32_t data) { ictl_w(controller, data); }
    template <DMAC3_Controller controller>
    void trc_w(uint32_t data) { trc_w(controller, data); }
    template <DMAC3_Controller controller>
    void tra_w(uint32_t data) { tra_w(controller, data); }
    template <DMAC3_Controller controller>
    void cnf_w(uint32_t data) { cnf_w(controller, data); }

    // Bitmasks for DMAC3 registers
    enum cstat_mask : uint32_t
    {
        CSTAT_SEND = 0x0000,
        CSTAT_ENABLE = 0x0001,
        CSTAT_RECV = 0x0002,
        CSTAT_RESET = 0x0004,
        CSTAT_APAD = 0x0008,
        CSTAT_MBURST = 0x0010,
        CSTAT_DBURST = 0x0020,
    };

    enum ictl_mask : uint32_t
    {
        ICTL_INT = 0x0001,
        ICTL_INTEN = 0x0002,
        ICTL_TCIE = 0x0020,
        ICTL_TCI = 0x0040,
        ICTL_EOP = 0x0100,
        ICTL_EOPIE = 0x0200, // End of operation interrupt enable (guess)
        ICTL_EOPI = 0x0400,
        ICTL_DREQ = 0x1000,
        ICTL_DRQIE = 0x2000, // Interrupt on DRQ enable?
        ICTL_DRQI = 0x4000,
        ICTL_PERR = 0x8000,
    };

    // I'm not clear yet on what IPER, DERR, MPER are signalling
    // NetBSD ignores IPER and MPER, but resets the dMAC if DERR is asserted during the interrupt routine
    // DCEN and PCEN are set by NetBSD during attach (along with FASTACCESS)
    enum cnf_mask : uint32_t
    {
        CNF_IPER = 0x8000,
        CNF_MPER = 0x4000,
        CNF_PCEN = 0x2000,
        CNF_DERR = 0x1000,
        CNF_DCEN = 0x0800,
        CNF_ODDP = 0x0200, // if I had to guess, odd parity?
        CNF_WIDTH = 0x00ff,
        CNF_SLOWACCESS = 0x0020, // SPIFI access? -> see spifi code in NetBSD
        CNF_FASTACCESS = 0x0001, // DEFAULT access? -> see spifi code in NetBSD
    };

    // Composite interrupt state
    bool m_out_int_state;
};

DECLARE_DEVICE_TYPE(DMAC3, dmac3_device)

#endif // MAME_MACHINE_DMAC3

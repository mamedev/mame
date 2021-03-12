// license:BSD-3-Clause
// copyright-holders:Brice Onken,Tsubai Masanari
// thanks-to:Patrick Mackinlay

/*
 * Sony DMAC3 DMA controller
 *
 * This is just a skeleton that logs accesses to inspect SCSI transactions.
 *
 * Register definitions were derived from the NetBSD source code, copyright (c) 2000 Tsubai Masanari.
 *
 * References:
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/dmac3reg.h
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/dmac3var.h
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/dmac3.c
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/spifi.c
 *
 *  TODO: FIFO functionality
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

    // Address maps
    void map_dma_ram(address_map &map);
    template <DMAC3_Controller controller>
    void map(address_map &map)
    {
        map(0x0, 0x3).rw(FUNC(dmac3_device::csr_r<controller>), FUNC(dmac3_device::csr_w<controller>));
        map(0x4, 0x7).rw(FUNC(dmac3_device::intr_r<controller>), FUNC(dmac3_device::intr_w<controller>));
        map(0x8, 0xb).rw(FUNC(dmac3_device::length_r<controller>), FUNC(dmac3_device::length_w<controller>));
        map(0xc, 0xf).rw(FUNC(dmac3_device::address_r<controller>), FUNC(dmac3_device::address_w<controller>));
        map(0x10, 0x13).rw(FUNC(dmac3_device::conf_r<controller>), FUNC(dmac3_device::conf_w<controller>));
    }

protected:
	virtual void device_start() override {};

    // DMAC3 requires off-board RAM to be allocated for the DMA map
    // The platform host controls this configuration.
    const uint32_t map_ram_size = 0x20000; // 128 kibibytes

    // DMAC3 has two controllers on-chip
    struct dmac3_register_file
    {
        uint32_t csr = 0;
        uint32_t intr = 0;
        uint32_t length = 0;
        uint32_t address = 0;
        uint32_t conf = 0;
    } m_controllers[2];

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

    // Templates as partial functions for register file accessors
    template <DMAC3_Controller controller>
    uint32_t csr_r() { return csr_r(controller); }
    template <DMAC3_Controller controller>
    uint32_t intr_r() { return intr_r(controller); }
    template <DMAC3_Controller controller>
    uint32_t length_r() { return length_r(controller); }
    template <DMAC3_Controller controller>
    uint32_t address_r() { return address_r(controller); }
    template <DMAC3_Controller controller>
    uint32_t conf_r() { return conf_r(controller); }

    template <DMAC3_Controller controller>
    void csr_w(uint32_t data) { csr_w(controller, data); }
    template <DMAC3_Controller controller>
    void intr_w(uint32_t data) { intr_w(controller, data); }
    template <DMAC3_Controller controller>
    void length_w(uint32_t data) { length_w(controller, data); }
    template <DMAC3_Controller controller>
    void address_w(uint32_t data) { address_w(controller, data); }
    template <DMAC3_Controller controller>
    void conf_w(uint32_t data) { conf_w(controller, data); }

    // Bitmasks for DMAC3 registers
    enum csr_mask : uint32_t
    {
        CSR_SEND = 0x0000,
        CSR_ENABLE = 0x0001,
        CSR_RECV = 0x0002,
        CSR_RESET = 0x0004,
        CSR_APAD = 0x0008,
        CSR_MBURST = 0x0010,
        CSR_DBURST = 0x0020,
    };

    enum intr_mask : uint32_t
    {
        INTR_INT = 0x0001,
        INTR_INTEN = 0x0002,
        INTR_TCIE = 0x0020,
        INTR_TCI = 0x0040,
        INTR_EOP = 0x0100,
        INTR_EOPIE = 0x0200, // End of operation interrupt enable (guess)
        INTR_EOPI = 0x0400,
        INTR_DREQ = 0x1000,
        INTR_DRQIE = 0x2000, // Interrupt on DRQ enable?
        INTR_DRQI = 0x4000,
        INTR_PERR = 0x8000,
    };

    // I'm not clear yet on what IPER, DERR, MPER are signalling
    // NetBSD ignores IPER and MPER, but resets the DMAC if DERR is asserted during the interrupt routine
    // DCEN and PCEN are set by NetBSD during attach (along with FASTACCESS)
    enum conf_mask : uint32_t
    {
        CONF_IPER = 0x8000,
        CONF_MPER = 0x4000,
        CONF_PCEN = 0x2000,
        CONF_DERR = 0x1000,
        CONF_DCEN = 0x0800,
        CONF_ODDP = 0x0200, // if I had to guess, odd parity?
        CONF_WIDTH = 0x00ff,
        CONF_SLOWACCESS = 0x0020, // SPIFI access mode (see NetBSD source code)
        CONF_FASTACCESS = 0x0001, // DMAC3 access mode (see NetBSD source code)
    };
};

DECLARE_DEVICE_TYPE(DMAC3, dmac3_device)

#endif // MAME_MACHINE_DMAC3

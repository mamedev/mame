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
 *  - Determine if address bus translation actually goes through this chip
 *  - Interrupts
 *  - Other functionality
 */

#ifndef MAME_MACHINE_CXD8452AQ_H
#define MAME_MACHINE_CXD8452AQ_H

#pragma once

#include "emu.h"
#include "device.h"
#include "devfind.h"
#include "mconfig.h"

class cxd8452aq_device : public device_t, public device_memory_interface
{
public:
    cxd8452aq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

    void map(address_map &map);

    template <typename... T> void set_apbus_address_translator(T &&... args) { m_apbus_virt_to_phys_callback.set(std::forward<T>(args)...); }

    // CPU-side accessors for the WSC-SONIC3 registers
    uint32_t cpu_r(offs_t offset, uint32_t mem_mask) { return space(1).read_dword(offset, mem_mask); }
	void cpu_w(offs_t offset, uint32_t data, uint32_t mem_mask) { space(1).write_dword(offset, data, mem_mask); }

    auto irq_out() { return m_irq_handler.bind(); }
    void irq_w(int state)
    {
        if (state)
        {
            sonic3_reg.sonic |= 0x1;
        }
        else
        {
            sonic3_reg.sonic &= ~0x1;
        }
        m_irq_check->adjust(attotime::zero);
    }

    template <typename T> void set_bus(T &&tag, int spacenum) { m_bus.set_tag(std::forward<T>(tag), spacenum); }

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
    uint32_t sonic3_r(offs_t offset);
    void sonic3_w(offs_t offset, uint32_t data);
    uint32_t tx_count_r(offs_t offset);
    void tx_count_w(offs_t offset, uint32_t data);
    uint32_t rx_count_r(offs_t offset);
    void rx_count_w(offs_t offset, uint32_t data);

    // Interrupt handling
    devcb_write_line m_irq_handler;
    bool m_irq = false; // Chip-level IRQ
    emu_timer *m_irq_check;
    TIMER_CALLBACK_MEMBER(irq_check);

    // APbus DMA
    device_delegate<uint32_t(uint32_t)> m_apbus_virt_to_phys_callback;
    const int DMA_TIMER = 1;      // No idea what this should be - maybe it would run at the APbus frequency?
    required_address_space m_bus; // direct bus access for DMA
    emu_timer *m_dma_check;
    TIMER_CALLBACK_MEMBER(dma_check);

    struct WscSonicRegisters
	{
        // General registers
        uint32_t sonic = 0x80000000;
        uint32_t config = 0x0;
        uint32_t revision = 0x3;

        // DMA registers
        uint32_t rx_sonic_address = 0x0;
        uint32_t rx_host_address = 0x0;
        uint32_t rx_count = 0x0;
        uint32_t tx_sonic_address = 0x0;
        uint32_t tx_host_address = 0x0;
        uint32_t tx_count = 0x0;
    } sonic3_reg;

    // Register-related constants
    // SONIC register
    const uint32_t INT_EN_MASK = 0x7f00;
    const uint32_t INT_CLR_MASK = 0xf0; // XXX external interrupt too?
    const uint32_t RX_DMA_COMPLETE = 0x40;
    const uint32_t TX_DMA_COMPLETE = 0x20;
    const uint32_t EXT_INT = 0x1;

    // Count registers
    const uint32_t DMA_START = 0x80000000;
};

DECLARE_DEVICE_TYPE(CXD8452AQ, cxd8452aq_device)

#endif // MAME_MACHINE_CXD8452AQ_H

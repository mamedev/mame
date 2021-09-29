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

    // CPU-side accessors for the WSC-SONIC3 registers
    uint32_t cpu_r(offs_t offset, uint32_t mem_mask) { return space(1).read_dword(offset, mem_mask); }
	void cpu_w(offs_t offset, uint32_t data, uint32_t mem_mask) { space(1).write_dword(offset, data, mem_mask); }

    auto irq_out() { return m_irq_handler.bind(); }
    void irq_w(int state)
    {
        // NEWS-OS seems to check the LSB of the SONIC register when starting the interrupt processing
        // This doesn't line up with what is in the NetBSD source code, which seems to check the SONIC
        // directly. However, other APbus devices share the same relationship (see: DMAC3 and SPIFI)
        // so this is likely "legit". If the DMAC3 is anything to go by, there are probably other interrupt
        // conditions that this chip detects or sets. This chip might have DMA functions of some kind too.
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

    // Other I/O
    devcb_write_line m_irq_handler;
    bool m_irq = false; // Chip-level IRQ
    emu_timer *m_irq_check;
    TIMER_CALLBACK_MEMBER(irq_check);

    struct WscSonicRegisters
	{
        uint32_t sonic = 0x80000000;
        uint32_t config = 0x0;
        uint32_t revision = 0x3;
        // TODO: rest of the registers
	} sonic3_reg;

};

DECLARE_DEVICE_TYPE(CXD8452AQ, cxd8452aq_device)

#endif // MAME_MACHINE_CXD8452AQ_H

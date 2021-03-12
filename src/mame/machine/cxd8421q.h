// license:BSD-3-Clause
// copyright-holders:Brice Onken

/*
 * Sony CXD8421Q WSC-ESCC1 Serial Controller AP-Bus Interface
 *
 * Only the direct passthrough to the ESCC is working. However, the monitor ROM only uses the
 * passthrough capability, so this actually is all that is needed to get to the monitor ROM prompt.
 *
 * TODO:
 *  - ESCC1 control and status registers
 *  - FIFOs, timers, etc.
 *  - Clock handling for the ESCC1 (the ESCC itself has a 9.8304MHz crystal)
 */

#ifndef MAME_MACHINE_CXD8421Q_H
#define MAME_MACHINE_CXD8421Q_H

#pragma once

#include "emu.h"
#include "device.h"
#include "devfind.h"
#include "mconfig.h"
#include "machine/z80scc.h"
#include "bus/rs232/rs232.h"

class cxd8421q_device : public device_t
{
public:
    cxd8421q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

    // avaliable ESCC channels
    enum ESCC_Channel
    {
        CHA,
        CHB
    };

    // Direct channel access
    template <ESCC_Channel channel>
    uint32_t ch_read(offs_t offset) { return ch_read(channel, offset); }
    template <ESCC_Channel channel>
    void ch_write(offs_t offset, uint32_t data) { ch_write(channel, offset, data); }
    // TODO: ESCC1 FIFO, control registers, etc.

    // Interrupts
    void escc_irq_w(int state);
    auto out_int_callback() { return out_irq.bind(); }

protected:
    // device_t overrides
    virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

    // Devices owned by this chip
    required_device<z80scc_device> m_escc;
    required_device_array<rs232_port_device, 2> m_serial;

    // ESCC direct channel access
    uint32_t ch_read(ESCC_Channel channel, offs_t offset);
	void ch_write(ESCC_Channel channel, offs_t offset, uint32_t data);

    // Interrupts
    bool escc_irq_state = false;
    devcb_write_line out_irq;
    void device_resolve_objects() override { out_irq.resolve_safe(); }
};

DECLARE_DEVICE_TYPE(CXD8421Q, cxd8421q_device)

#endif // MAME_MACHINE_CXD8421Q_H

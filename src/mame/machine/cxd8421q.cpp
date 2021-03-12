// license:BSD-3-Clause
// copyright-holders:Brice Onken

/*
 * Sony CXD8421Q WSC-ESCC1 Serial Controller AP-Bus Interface
 *
 * Only the direct passthrough to the ESCC is working. However, the monitor ROM only uses the
 * passthrough capability, so this actually is all that is needed to get to the monitor ROM prompt.
 *
 * Reference:
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/zs_ap.c
 *
 * TODO:
 *  - ESCC1 control and status registers
 *  - FIFOs, timers, etc.
 *  - Clock handling for the ESCC1 (the ESCC itself has a 9.8304MHz crystal)
 */

#include "cxd8421q.h"

#include "logmacro.h"

DEFINE_DEVICE_TYPE(CXD8421Q, cxd8421q_device, "cxd8421q", "Sony CXD8421Q WSC-ESCC1")

cxd8421q_device::cxd8421q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
    device_t(mconfig, CXD8421Q, tag, owner, clock),
    m_escc(*this, "escc"),
    m_serial(*this, "serial%u", 0U),
    out_irq(*this) {}

void cxd8421q_device::device_add_mconfig(machine_config &config)
{
    // General ESCC setup
    SCC85230(config, m_escc, 9.8304_MHz_XTAL); // 9.8304MHz per NetBSD source
    m_escc->out_int_callback().set(FUNC(cxd8421q_device::escc_irq_w));

    RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
    m_serial[0]->cts_handler().set(m_escc, FUNC(z80scc_device::ctsa_w));
    m_serial[0]->dcd_handler().set(m_escc, FUNC(z80scc_device::dcda_w));
    m_serial[0]->rxd_handler().set(m_escc, FUNC(z80scc_device::rxa_w));
    m_escc->out_rtsa_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
    m_escc->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));
    m_escc->out_dtra_callback().set(m_serial[0], FUNC(rs232_port_device::write_dtr));

    RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
    m_serial[1]->cts_handler().set(m_escc, FUNC(z80scc_device::ctsb_w));
    m_serial[1]->dcd_handler().set(m_escc, FUNC(z80scc_device::dcdb_w));
    m_serial[1]->rxd_handler().set(m_escc, FUNC(z80scc_device::rxb_w));
    m_escc->out_rtsb_callback().set(m_serial[1], FUNC(rs232_port_device::write_rts));
    m_escc->out_txdb_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));
    m_escc->out_dtrb_callback().set(m_serial[1], FUNC(rs232_port_device::write_dtr));
}

uint32_t cxd8421q_device::ch_read(ESCC_Channel channel, offs_t offset)
{
    if (offset < 2) {
        offset |= (channel == CHA ? 0x2 : 0x0);
        return m_escc->ab_dc_r(offset);
    } else {
        // Placeholder for ESCC1 extport and ctl registers
        LOG("escc1 ch%d r non-passthrough: 0x%x\n", channel, offset);
        return 0x0;
    }
}

void cxd8421q_device::ch_write(ESCC_Channel channel, offs_t offset, uint32_t data)
{
    if (offset < 2) {
        offset |= (channel == CHA ? 0x2 : 0x0);
        m_escc->ab_dc_w(offset, data);
    } else {
        // Placeholder for ESCC1 extport and ctl registers
        LOG("escc1 ch%d w non-passthrough: 0x%x = 0x%x\n", channel, offset, data);
        return;
    }
}

void cxd8421q_device::escc_irq_w(int state)
{
    if ( ((bool)state) != escc_irq_state)
    {
        escc_irq_state = state;
        out_irq(state);
    }
}

void cxd8421q_device::device_start() { }

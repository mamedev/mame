// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// DSP 563xx HI08 host interface

#include "emu.h"
#include "hi08.h"
#include "dsp563xx.h"

DEFINE_DEVICE_TYPE(HI08, hi08_device, "hi08", "DSP563xx HI08 host interface")

hi08_device::hi08_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
    device_t(mconfig, HI08, tag, owner, clock),
    m_cpu(*this, DEVICE_SELF_OWNER)
{
}

void hi08_device::device_start()
{
    save_item(NAME(m_hcr));
    save_item(NAME(m_hpcr));
    save_item(NAME(m_hbar));
    save_item(NAME(m_hrx));
    save_item(NAME(m_htx));
    save_item(NAME(m_tx));
    save_item(NAME(m_icr));
    save_item(NAME(m_cvr));
    save_item(NAME(m_isr));
    save_item(NAME(m_ivr));
}

void hi08_device::device_reset()
{
    m_hcr = 0;
    m_hpcr = 0;
    m_hbar = 0x80;
    m_hrx = 0;
    m_htx = 0;
    m_tx = 0;
    m_icr = 0;
    m_cvr = 0x32;
    m_isr = 3;
    m_ivr = 15;

}

void hi08_device::write(offs_t offset, u8 data)
{
    switch(offset) {
    case 0:
	m_icr = data & 0xbf;
	logerror("host write isr %02x (%s)\n", m_isr, machine().describe_context());
	break;
    case 1:
	m_cvr = data;
	logerror("host write cvr %02x (%s)\n", m_cvr, machine().describe_context());
	break;
    case 2:
	logerror("host rejected write isr %02x (%s)\n", data, machine().describe_context());
	break;
    case 3:
	m_ivr = data;
	logerror("host write ivr %02x (%s)\n", m_ivr, machine().describe_context());
	break;
    case 4:
	logerror("host rejected write unused %02x (%s)\n", data, machine().describe_context());
	break;
    case 5:
	if(m_icr & ICR_HLEND)
	    m_tx = (m_tx & 0xffff00) | data;
	else
	    m_tx = (m_tx & 0x00ffff) | (data << 16);
	break;
    case 6:
	m_tx = (m_tx & 0xff00ff) | (data << 8);
	break;
    case 7:
	if(m_icr & ICR_HLEND)
	    m_tx = (m_tx & 0x00ffff) | (data << 16);
	else
	    m_tx = (m_tx & 0xffff00) | data;
	m_hrx = m_tx;
	logerror("received %06x (%s)\n", m_hrx, machine().describe_context());
	m_isr &= ~ISR_TXDE;
	if(!(m_isr & ISR_RXDF))
	    m_isr |= ISR_TRDY;
	machine().scheduler().synchronize();
	break;
    }
}

u8 hi08_device::read(offs_t offset)
{
    switch(offset) {
    case 0:
	logerror("host read icr (%s)\n", machine().describe_context());
	return m_icr;
    case 1:
	logerror("host read cvr (%s)\n", machine().describe_context());
	return m_cvr;
    case 2:
	logerror("host read isr (%s)\n", machine().describe_context());
	return m_isr;
    case 3:
	logerror("host read ivr (%s)\n", machine().describe_context());
	return m_ivr;
    case 4:
	logerror("host read unused (%s)\n", machine().describe_context());
	return 0x00;
    case 5:
	return m_htx >> (m_icr & ICR_HLEND ? 0 : 16);
    case 6:
	return m_htx >> 8;
    case 7:
	return m_htx >> (m_icr & ICR_HLEND ? 16 : 0);
    }
    return 0;
}

void hi08_device::map(address_map &map)
{
    map(0*4, 0*4+3).rw(FUNC(hi08_device::hcr_r), FUNC(hi08_device::hcr_w));
    map(1*4, 1*4+3).r(FUNC(hi08_device::hsr_r));
    map(2*4, 2*4+3).rw(FUNC(hi08_device::hpcr_r), FUNC(hi08_device::hpcr_w));
    map(3*4, 3*4+3).rw(FUNC(hi08_device::hbar_r), FUNC(hi08_device::hbar_w));
    map(4*4, 4*4+3).r(FUNC(hi08_device::hrx_r));
    map(5*4, 5*4+3).w(FUNC(hi08_device::htx_w));
}

void hi08_device::hcr_w(u32 data)
{
    m_hcr = data & 0x1f;
    logerror("hcr_w %02x (%s)\n", m_hcr, machine().describe_context());
}

u32 hi08_device::hcr_r()
{
    logerror("hcr_r %02x (%s)\n", m_hcr, machine().describe_context());
    return m_hcr;
}

u32 hi08_device::hsr_r()
{
    u16 hsr = 0;
    if(!(m_isr & ISR_TXDE))
	hsr |= HSR_HRDF;
    if(!(m_isr & ISR_RXDF))
	hsr |= HSR_HTDE;
    if(m_cvr & CVR_HC)
	hsr |= HSR_HCP;
    hsr |= m_icr & (ICR_HF1 | ICR_HF0);
    return hsr;
}

void hi08_device::hpcr_w(u32 data)
{
    m_hpcr = data & 0xff7f;
    logerror("hpcr_w %04x (%s)\n", m_hpcr, machine().describe_context());
}

u32 hi08_device::hpcr_r()
{
    logerror("hpcr_r %04x (%s)\n", m_hpcr, machine().describe_context());
    return m_hpcr;
}

void hi08_device::hbar_w(u32 data)
{
    m_hbar = data;
    logerror("hbar_w %02x (%s)\n", m_hbar, machine().describe_context());
}

u32 hi08_device::hbar_r()
{
    logerror("hbar_r (%s)\n", machine().describe_context());
    return m_hbar;
}

u32 hi08_device::hrx_r()
{
    logerror("hrx_r %06x (%s)\n", m_hrx, machine().describe_context());
    m_isr = (m_isr & ~ISR_TRDY) | ISR_TXDE;
    machine().scheduler().synchronize();
    return m_hrx;
}

void hi08_device::htx_w(u32 data)
{
    logerror("htx_w %06x (%s)\n", data, machine().describe_context());
}

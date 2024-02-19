// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// SH7042, sh2 variant

#include "emu.h"
#include "sh7042.h"

DEFINE_DEVICE_TYPE(SH7042, sh7042_device, "sh7042", "Hitachi SH-2 (SH7042)")
DEFINE_DEVICE_TYPE(SH7043, sh7043_device, "sh7043", "Hitachi SH-2 (SH7043)")

sh7042_device::sh7042_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sh7042_device(mconfig, SH7042, tag, owner, clock)
{
}

sh7043_device::sh7043_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sh7042_device(mconfig, SH7043, tag, owner, clock)
{
}

sh7042_device::sh7042_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	sh2_device(mconfig, type, tag, owner, clock, CPU_TYPE_SH2, address_map_constructor(FUNC(sh7042_device::map), this), 32, 0xffffffff),
	m_read_adc(*this, 0),
	m_sci_tx(*this)
{
	for(unsigned int i=0; i != m_read_adc.size(); i++)
		m_read_adc[i].bind().set([this, i]() { return adc_default(i); });
}

u16 sh7042_device::adc_default(int adc)
{
	logerror("read of un-hooked adc %d\n", adc);
	return 0;
}

void sh7042_device::device_start()
{
	sh2_device::device_start();

	save_item(NAME(m_addr));
	save_item(NAME(m_adcsr));
	save_item(NAME(m_adcr));
}

void sh7042_device::device_reset()
{
	sh2_device::device_reset();

	memset(m_addr, 0, sizeof(m_addr));
	m_adcsr = m_adcr = 0;
}

void sh7042_device::map(address_map &map)
{
	map(0xffff83e0, 0xffff83e0).rw(FUNC(sh7042_device::adcsr_r), FUNC(sh7042_device::adcsr_w));
	map(0xffff83e1, 0xffff83e1).rw(FUNC(sh7042_device::adcr_r), FUNC(sh7042_device::adcr_w));
	map(0xffff83f0, 0xffff83ff).r(FUNC(sh7042_device::addr_r));

	map(0xfffff000, 0xffffffff).ram();
}


// ADC section

u16 sh7042_device::addr_r(offs_t offset)
{
	logerror("addr16_r %d %03x\n", offset, m_addr[offset]);
	return m_addr[offset];
}

u8 sh7042_device::adcsr_r()
{
	logerror("adcsr_r %02x\n", m_adcsr);
	return m_adcsr;
}

u8 sh7042_device::adcr_r()
{
	logerror("adcr_r %02x\n", m_adcr);
	return m_adcr;
}

void sh7042_device::adcsr_w(u8 data)
{
	logerror("adcsr_w %02x\n", data);
	//	u8 prev = m_adcsr;
	m_adcsr = (data & 0x7f) | (m_adcsr & data & CSR_ADF);
}

void sh7042_device::adcr_w(u8 data)
{
	static const char *const tg_modes[4] = { "soft", "mtu", "?", "external" };
	static const char *const buf_modes[4] = { "normal", "a->b", "a,b->c,d", "a->b->c->d" };
	logerror("adcr_w speed=%d trigger=%s mode=%s sampling=%s buffering=%s\n",
			 BIT(data, 6) ? "high" : "low",
			 tg_modes[(data >> 4) & 3],
			 BIT(data, 3) ? "scan" : "single",
			 BIT(data, 2) ? "simultaneous" : "normal",
			 buf_modes[data & 3]);
	m_adcr = data;
}

void sh7042_device::do_sci_w(int sci, int state)
{
	logerror("sci %d %d\n", sci, state);
}

// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "sh_adc.h"
#include "sh7042.h"

DEFINE_DEVICE_TYPE(SH_ADC, sh_adc_device, "sh_adc", "SH7042 ADC")

sh_adc_device::sh_adc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SH_ADC, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_adcsr(0), m_adcr(0)
{
}

u16 sh_adc_device::addr_r(offs_t offset)
{
	logerror("addr16_r %d %03x\n", offset, m_addr[offset]);
	return m_addr[offset];
}

u8 sh_adc_device::adcsr_r()
{
	logerror("adcsr_r %02x\n", m_adcsr);
	return m_adcsr;
}

u8 sh_adc_device::adcr_r()
{
	logerror("adcr_r %02x\n", m_adcr);
	return m_adcr;
}

void sh_adc_device::adcsr_w(u8 data)
{
	logerror("adcsr_w %02x\n", data);
	//	u8 prev = m_adcsr;
	m_adcsr = (data & 0x7f) | (m_adcsr & data & CSR_ADF);
}

void sh_adc_device::adcr_w(u8 data)
{
	logerror("adcr_w %02x\n", data);
	m_adcr = data;
}

void sh_adc_device::device_start()
{
	save_item(NAME(m_addr));
	save_item(NAME(m_adcsr));
	save_item(NAME(m_adcr));
}

void sh_adc_device::device_reset()
{
	memset(m_addr, 0, sizeof(m_addr));
	m_adcsr = m_adcr = 0;
}

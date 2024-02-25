// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// SH7042, sh2 variant

#include "emu.h"
#include "sh7042.h"

DEFINE_DEVICE_TYPE(SH7042,  sh7042_device,  "sh7042",  "Hitachi SH-2 (SH7042)")

sh7042_device::sh7042_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sh2_device(mconfig, SH7042, tag, owner, clock, CPU_TYPE_SH2, address_map_constructor(FUNC(sh7042_device::map), this), 32, 0xffffffff),
	m_adc(*this, "adc"),
	m_read_adc(*this, 0)
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
}

void sh7042_device::device_reset()
{
	sh2_device::device_reset();
}

void sh7042_device::map(address_map &map)
{
	map(0xffff83e0, 0xffff83e0).rw(m_adc, FUNC(sh_adc_device::adcsr_r), FUNC(sh_adc_device::adcsr_w));
	map(0xffff83e1, 0xffff83e1).rw(m_adc, FUNC(sh_adc_device::adcr_r), FUNC(sh_adc_device::adcr_w));
	map(0xffff83f0, 0xffff83ff).r(m_adc, FUNC(sh_adc_device::addr_r));

	map(0xfffff000, 0xffffffff).ram();
}

void sh7042_device::device_add_mconfig(machine_config &config)
{
	SH_ADC(config, m_adc, *this);
}

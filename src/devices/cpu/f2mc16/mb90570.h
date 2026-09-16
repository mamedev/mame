// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_CPU_F2MC16_MB90570_H
#define MAME_CPU_F2MC16_MB90570_H

#pragma once

#include "f2mc16.h"
#include "f2mc16_adc.h"
#include "f2mc16_clock.h"
#include "f2mc16_intc.h"
#include "f2mc16_port.h"
#include "f2mc16_ppg.h"
#include "f2mc16_uart.h"

class mb90570_device :
	public f2mc16_device
{
public:
	mb90570_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto &adc() const { if (m_adc) return *m_adc; return *subdevice<f2mc16_adc_device>(m_adc.finder_tag()); }
	template<unsigned N> typename std::enable_if<N < 13, f2mc16_port_device>::type &port() const { if (m_port[N]) return *m_port[N]; return *subdevice<f2mc16_port_device>(m_port[N].finder_tag()); }
	auto &ppg() const { if (m_ppg) return *m_ppg; return *subdevice<f2mc16_ppg_device>(m_ppg.finder_tag()); }
	template<unsigned N> typename std::enable_if<N < 2, f2mc16_uart_device>::type &uart() const { if (m_uart[N]) return *m_uart[N]; return *subdevice<f2mc16_uart_device>(m_uart[N].finder_tag()); }

protected:
	mb90570_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint16_t internal_ram_end);

	// device_t
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	void internal_map(address_map &map) ATTR_COLD;

	const address_space_config m_program_config;
	const uint16_t m_internal_ram_end;

	required_device<f2mc16_adc_device> m_adc;
	required_device<f2mc16_clock_generator_device> m_clock_generator;
	required_device<f2mc16_intc_device> m_intc;
	required_device_array<f2mc16_port_device, 13> m_port;
	required_device<f2mc16_ppg_device> m_ppg;
	required_device_array<f2mc16_uart_device, 2> m_uart;
};

class mb90f574_device : public mb90570_device
{
public:
	mb90f574_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(MB90F574, mb90f574_device)

#endif // MAME_CPU_F2MC16_MB90570_H

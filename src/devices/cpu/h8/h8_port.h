// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_port.h

    H8 8 bits digital port


***************************************************************************/

#ifndef MAME_CPU_H8_H8_PORT_H
#define MAME_CPU_H8_H8_PORT_H

#pragma once

#include "h8.h"

class h8_port_device : public device_t {
public:
	h8_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template<typename T> h8_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, int address, uint8_t default_ddr, uint8_t mask)
		: h8_port_device(mconfig, tag, owner, 0)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
		m_address = address;
		m_default_ddr = default_ddr;
		m_mask = mask;
	}

	void ddr_w(uint8_t data);
	uint8_t ddr_r();
	void dr_w(uint8_t data);
	uint8_t dr_r();
	uint8_t port_r();
	void pcr_w(uint8_t data);
	uint8_t pcr_r();
	void odr_w(uint8_t data);
	uint8_t odr_r();

	bool nvram_read(util::read_stream &file);
	bool nvram_write(util::write_stream &file);

protected:
	required_device<h8_device> m_cpu;

	int m_address;
	uint8_t m_default_ddr, m_ddr, m_pcr, m_odr;
	uint8_t m_mask;
	uint8_t m_dr;
	int32_t m_last_output;

	virtual void device_start() override;
	virtual void device_reset() override;
	void update_output();
};

DECLARE_DEVICE_TYPE(H8_PORT, h8_port_device)

typedef device_type_enumerator<h8_port_device> h8_port_device_enumerator;

#endif // MAME_CPU_H8_H8_PORT_H

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
	h8_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template<typename T> h8_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, int address, u8 default_ddr, u8 mask)
		: h8_port_device(mconfig, tag, owner, 0)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
		m_address = address;
		m_default_ddr = default_ddr;
		m_mask = mask;
	}

	void ddr_w(u8 data);
	u8 ddr_r();
	u8 ff_r() { return 0xff; }
	void dr_w(u8 data);
	u8 dr_r();
	u8 port_r();
	void pcr_w(u8 data);
	u8 pcr_r();
	void odr_w(u8 data);
	u8 odr_r();

	bool nvram_read(util::read_stream &file);
	bool nvram_write(util::write_stream &file);

protected:
	required_device<h8_device> m_cpu;

	int m_address;
	u8 m_default_ddr, m_ddr, m_pcr, m_odr;
	u8 m_mask;
	u8 m_dr;
	s32 m_last_output;

	virtual void device_start() override;
	virtual void device_reset() override;
	void update_output();
};

DECLARE_DEVICE_TYPE(H8_PORT, h8_port_device)

typedef device_type_enumerator<h8_port_device> h8_port_device_enumerator;

#endif // MAME_CPU_H8_H8_PORT_H

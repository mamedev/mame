// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Fujitsu Micro F2MC-16 series Port

***************************************************************************/

#ifndef MAME_CPU_F2MC16_F2MC16_PORT_H
#define MAME_CPU_F2MC16_F2MC16_PORT_H

#pragma once

#include "f2mc16.h"

class f2mc16_port_device :
	public device_t
{
public:
	f2mc16_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint8_t defval, uint8_t mask);
	f2mc16_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto read() { return m_read_cb.bind(); }
	auto write() { return m_write_cb.bind(); }

	uint8_t pdr_r();
	uint8_t pdr_adc_r();
	void pdr_w(uint8_t data);

	uint8_t ddr_r();
	void ddr_w(uint8_t data);

	uint8_t ader_r();
	void ader_w(uint8_t data);

protected:
	// device_t
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void update_output(bool initial = false);

	f2mc16_device *m_cpu;
	uint8_t m_defval;
	uint8_t m_mask;
	devcb_read8 m_read_cb;
	devcb_write8 m_write_cb;

	uint8_t m_pdr;
	uint8_t m_ddr;
	uint8_t m_ader;
	uint8_t m_output;
};

DECLARE_DEVICE_TYPE(F2MC16_PORT, f2mc16_port_device)

#endif

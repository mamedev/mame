// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Midway X-unit games.

**************************************************************************/
#ifndef MAME_INCLUDES_MIDXUNIT_H
#define MAME_INCLUDES_MIDXUNIT_H

#pragma once

#include "midtunit.h"
#include "machine/midwayic.h"


class midxunit_state : public midtunit_state
{
public:
	midxunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: midtunit_state(mconfig, type, tag)
		, m_nvram(*this, "nvram")
		, m_midway_serial_pic(*this, "serial_pic")
		, m_gun_recoil(*this, "Player%u_Gun_Recoil", 1U)
		, m_gun_led(*this, "Player%u_Gun_LED", 1U)
	{ }

	void midxunit(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	uint16_t midxunit_cmos_r(offs_t offset);
	void midxunit_cmos_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void midxunit_io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void midxunit_unknown_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	DECLARE_WRITE_LINE_MEMBER(adc_int_w);
	uint16_t midxunit_status_r();
	uint16_t midxunit_uart_r(offs_t offset);
	void midxunit_uart_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t midxunit_security_r();
	void midxunit_security_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void midxunit_security_clock_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t midxunit_sound_r();
	uint16_t midxunit_sound_state_r();
	void midxunit_sound_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	DECLARE_WRITE_LINE_MEMBER(midxunit_dcs_output_full);

	void main_map(address_map &map);

	required_shared_ptr<uint16_t> m_nvram;
	required_device<midway_serial_pic_device> m_midway_serial_pic;
	output_finder<3> m_gun_recoil;
	output_finder<3> m_gun_led;

	uint8_t m_cmos_write_enable;
	uint16_t m_iodata[8];
	uint8_t m_ioshuffle[16];
	uint8_t m_uart[8];
	uint8_t m_security_bits;
	bool m_adc_int;
};

#endif // MAME_INCLUDES_MIDXUNIT_H

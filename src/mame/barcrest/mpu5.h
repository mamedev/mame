// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_BARCREST_MPU5_H
#define MAME_BARCREST_MPU5_H

#pragma once

#include "sec.h"

#include "machine/68340.h"


class mpu5_state : public driver_device
{
public:
	mpu5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sec(*this, "sec")
		, m_statuslamp(*this, "statuslamp%u", 1U)
	{ }

	void mpu5(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	uint16_t mpu5_mem_r(offs_t offset, uint16_t mem_mask = ~0);
	void mpu5_mem_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t asic_r16(offs_t offset, uint16_t mem_mask = ~0);
	uint8_t asic_r8(offs_t offset);
	void asic_w16(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void asic_w8(offs_t offset, uint8_t data);

	uint16_t pic_r(offs_t offset);
	void pic_w(offs_t offset, uint16_t data);

	void mpu5_map(address_map &map) ATTR_COLD;

	uint16_t* m_cpuregion = nullptr;
	std::unique_ptr<uint16_t[]> m_mainram;

	uint8_t m_led_strobe_temp = 0;
	uint8_t m_led_strobe = 0;
	uint8_t m_pic_clk = 0;
	bool  m_pic_transfer_in_progress = false;
	uint8_t m_pic_bit1 = 0;
	uint8_t m_pic_data = 0;
	uint8_t m_pic_clocked_bits = 0;
	uint8_t m_pic_stored_input = 0;
	uint8_t m_pic_output_bit = 0;
	uint8_t m_input_strobe = 0;

	// devices
	required_device<m68340_cpu_device> m_maincpu;
	required_device<sec_device> m_sec;

	output_finder<2> m_statuslamp;
};

INPUT_PORTS_EXTERN( mpu5 );

#endif // MAME_BARCREST_MPU5_H

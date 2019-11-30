// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_MPU5_H
#define MAME_INCLUDES_MPU5_H

#pragma once

#include "machine/68340.h"
#include "machine/sec.h"


class mpu5_state : public driver_device
{
public:
	mpu5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sec(*this, "sec")
	{ }

	void mpu5(machine_config &config);

private:
	DECLARE_READ32_MEMBER(mpu5_mem_r);
	DECLARE_WRITE32_MEMBER(mpu5_mem_w);

	DECLARE_READ32_MEMBER(asic_r32);
	DECLARE_READ8_MEMBER(asic_r8);
	DECLARE_WRITE32_MEMBER(asic_w32);
	DECLARE_WRITE8_MEMBER(asic_w8);

	DECLARE_READ32_MEMBER(pic_r);
	DECLARE_WRITE32_MEMBER(pic_w);

	virtual void machine_start() override;
	void mpu5_map(address_map &map);

	uint32_t* m_cpuregion;
	std::unique_ptr<uint32_t[]> m_mainram;

	uint8_t m_led_strobe_temp;
	uint8_t m_led_strobe;
	uint8_t m_pic_clk;
	bool  m_pic_transfer_in_progress;
	uint8_t m_pic_bit1;
	uint8_t m_pic_data;
	uint8_t m_pic_clocked_bits;
	uint8_t m_pic_stored_input;
	uint8_t m_pic_output_bit;
	uint8_t m_input_strobe;

	// devices
	required_device<m68340_cpu_device> m_maincpu;
	required_device<sec_device> m_sec;
};

INPUT_PORTS_EXTERN( mpu5 );

#endif // MAME_INCLUDES_MPU5_H

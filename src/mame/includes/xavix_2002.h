// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_XAVIX_2002_H
#define MAME_INCLUDES_XAVIX_2002_H

#include "xavix_2000.h"

class xavix_i2c_jmat_state : public xavix_i2c_state
{
public:
	xavix_i2c_jmat_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
	{ }

	void xavix2002_i2c_jmat(machine_config &config);

private:
	uint8_t read_extended_io0();
	uint8_t read_extended_io1();
	uint8_t read_extended_io2();
	void write_extended_io0(uint8_t data);
	void write_extended_io1(uint8_t data);
	void write_extended_io2(uint8_t data);
};

class xavix2002_superpttv_state : public xavix_state
{
public:
	xavix2002_superpttv_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
	{ }

	void xavix2002_superpctv(machine_config &config);

private:
	uint8_t read_extended_io0() { return 0x00; }
	uint8_t read_extended_io1() { return 0x00; }
	uint8_t read_extended_io2() { return 0x00; }
	//void write_extended_io0(uint8_t data);
	//void write_extended_io1(uint8_t data);
	//void write_extended_io2(uint8_t data);
};

class xavix_i2c_bowl_state : public xavix_i2c_state
{
public:
	xavix_i2c_bowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
	{ }

	DECLARE_READ_LINE_MEMBER(camera_r);
};


#endif // MAME_INCLUDES_XAVIX_2002_H

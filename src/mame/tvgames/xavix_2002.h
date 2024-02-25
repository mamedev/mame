// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_TVGAMES_XAVIX_2002_H
#define MAME_TVGAMES_XAVIX_2002_H

#include "xavix_2000.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "softlist_dev.h"

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

class xavix2002_super_tv_pc_state : public xavix_state
{
public:
	xavix2002_super_tv_pc_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
		, m_cart(*this, "cartslot")
	{ }

	void xavix2002_super_tv_pc(machine_config &config);

private:
	uint8_t read_extended_io0() { return 0x00; }
	uint8_t read_extended_io1() { return 0x00; }
	uint8_t read_extended_io2() { return 0x00; }
	//void write_extended_io0(uint8_t data);
	//void write_extended_io1(uint8_t data);
	//void write_extended_io2(uint8_t data);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<generic_slot_device> m_cart;
};

class xavix_i2c_bowl_state : public xavix_i2c_state
{
public:
	xavix_i2c_bowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
	{ }

	int camera_r();
};


#endif // MAME_TVGAMES_XAVIX_2002_H

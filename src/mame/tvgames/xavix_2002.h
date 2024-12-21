// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_TVGAMES_XAVIX_2002_H
#define MAME_TVGAMES_XAVIX_2002_H

#include "xavix_2000.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "softlist_dev.h"

class superxavix_i2c_jmat_state : public superxavix_i2c_state
{
public:
	superxavix_i2c_jmat_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_state(mconfig, type, tag)
	{ }

	void superxavix_i2c_jmat(machine_config &config);

	void init_xavmusic();

private:
	uint8_t read_extended_io0();
	uint8_t read_extended_io1();
	uint8_t read_extended_io2();
	void write_extended_io0(uint8_t data);
	void write_extended_io1(uint8_t data);
	void write_extended_io2(uint8_t data);
};

class superxavix_super_tv_pc_state : public superxavix_state
{
public:
	superxavix_super_tv_pc_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_state(mconfig, type, tag)
		, m_cart(*this, "cartslot")
		, m_rombank(*this, "rombank")
	{ }

	void superxavix_super_tv_pc(machine_config &config);

	void init_stvpc();

private:
	virtual void machine_reset() override;

	uint8_t read_extended_io0() { return 0x00; }
	uint8_t read_extended_io1() { return 0x00; }
	uint8_t read_extended_io2() { return 0x00; }
	void write_extended_io0(uint8_t data) { logerror("%s: extio0_w %02x\n", machine().describe_context(), data); }
	void write_extended_io1(uint8_t data) { logerror("%s: extio1_w %02x\n", machine().describe_context(), data); }
	void write_extended_io2(uint8_t data)
	{
		logerror("%s: extio2_w %02x\n", machine().describe_context(), data);
		if (data & 0x04)
			m_rombank->set_entry(1);
		else
			m_rombank->set_entry(0);
	}

	virtual void xavix_extbus_map(address_map &map) override;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<generic_slot_device> m_cart;
	required_memory_bank m_rombank;
};

class superxavix_i2c_bowl_state : public superxavix_i2c_state
{
public:
	superxavix_i2c_bowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: superxavix_i2c_state(mconfig, type, tag)
	{ }

	int camera_r();
};


#endif // MAME_TVGAMES_XAVIX_2002_H

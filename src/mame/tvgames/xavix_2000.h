// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_TVGAMES_XAVIX_2000_H
#define MAME_TVGAMES_XAVIX_2000_H

#include "xavix.h"


class xavix_2000_nv_sdb_state : public xavix_state
{
public:
	xavix_2000_nv_sdb_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_state(mconfig, type, tag)
	{ }

	void xavix2000_nv_sdb(machine_config &config);

protected:

private:
	uint8_t sdb_anport0_r() { return m_mouse0x->read()^0x7f; }
	uint8_t sdb_anport1_r() { return m_mouse0y->read()^0x7f; }
	uint8_t sdb_anport2_r() { return m_mouse1x->read()^0x7f; }
	uint8_t sdb_anport3_r() { return m_mouse1y->read()^0x7f; }
};

class xavix_i2c_lotr_state : public xavix_i2c_state
{
public:
	xavix_i2c_lotr_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
	{ }

	int camera_r();

protected:
	//virtual void write_io1(uint8_t data, uint8_t direction) override;
};

class xavix_duelmast_state : public xavix_i2c_state
{
public:
	xavix_duelmast_state(const machine_config &mconfig, device_type type, const char *tag)
		: xavix_i2c_state(mconfig, type, tag)
	{ }

protected:
	virtual uint8_t read_io1(uint8_t direction) override;
};


#endif // MAME_TVGAMES_XAVIX_2000_H

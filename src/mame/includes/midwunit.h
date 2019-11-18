// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Ernesto Corvi
/*************************************************************************

    Driver for Midway Wolf-unit games.

**************************************************************************/
#ifndef MAME_INCLUDES_MIDWUNIT_H
#define MAME_INCLUDES_MIDWUNIT_H

#pragma once

#include "machine/midwayic.h"

class midwunit_state : public midtunit_state
{
public:
	midwunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: midtunit_state(mconfig, type, tag)
		, m_midway_serial_pic(*this, "serial_security_sim")
		, m_midway_serial_pic_emu(*this, "serial_security")
		, m_nvram(*this, "nvram")
		, m_mainram(*this, "mainram")
		, m_ports(*this, { { "IN0", "IN1", "DSW", "IN2" } })
	{ }

	void wunit(machine_config &config);
	void wunit_picemu(machine_config &config);
	void wunit_picsim(machine_config &config);

	void init_mk3r10();
	void init_nbahangt();
	void init_wwfmania();
	void init_umk3();
	void init_mk3();
	void init_openice();
	void init_rmpgwt();
	void init_umk3r11();
	void init_mk3r20();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	DECLARE_WRITE16_MEMBER(midwunit_cmos_enable_w);
	DECLARE_WRITE16_MEMBER(midwunit_cmos_w);
	DECLARE_READ16_MEMBER(midwunit_cmos_r);
	DECLARE_WRITE16_MEMBER(midwunit_io_w);
	DECLARE_READ16_MEMBER(midwunit_io_r);
	DECLARE_READ16_MEMBER(midwunit_security_r);
	DECLARE_WRITE16_MEMBER(midwunit_security_w);
	DECLARE_READ16_MEMBER(midwunit_sound_r);
	DECLARE_READ16_MEMBER(midwunit_sound_state_r);
	DECLARE_WRITE16_MEMBER(midwunit_sound_w);
	DECLARE_WRITE16_MEMBER(umk3_palette_hack_w);
	DECLARE_WRITE16_MEMBER(wwfmania_io_0_w);

	void init_mk3_common();
	void main_map(address_map &map);

	optional_device<midway_serial_pic_device> m_midway_serial_pic;
	optional_device<midway_serial_pic_emu_device> m_midway_serial_pic_emu;
	required_shared_ptr<uint16_t> m_nvram;
	required_shared_ptr<uint16_t> m_mainram;
	required_ioport_array<4> m_ports;

	uint8_t m_cmos_write_enable;
	uint16_t m_iodata[8];
	uint8_t m_ioshuffle[16];
	uint8_t m_uart[8];
	uint8_t m_security_bits;
	uint16_t *m_umk3_palette;
};

#endif // MAME_INCLUDES_MIDWUNIT_H

// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Ernesto Corvi
/*************************************************************************

    Driver for Midway Wolf-unit games.

**************************************************************************/

#include "machine/midwayic.h"

class midwunit_state : public midtunit_state
{
public:
	midwunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: midtunit_state(mconfig, type, tag),
			m_midway_serial_pic(*this, "serial_pic"),
			m_nvram(*this, "nvram"),
			m_mainram(*this, "mainram"),
			m_ports(*this, { { "IN0", "IN1", "DSW", "IN2" } })
			{ }

	required_device<midway_serial_pic_device> m_midway_serial_pic;
	required_shared_ptr<uint16_t> m_nvram;
	required_shared_ptr<uint16_t> m_mainram;
	required_ioport_array<4> m_ports;

	uint8_t m_cmos_write_enable;
	uint16_t m_iodata[8];
	uint8_t m_ioshuffle[16];
	uint8_t m_uart[8];
	uint8_t m_security_bits;
	uint16_t *m_umk3_palette;

	void midwunit_cmos_enable_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void midwunit_cmos_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t midwunit_cmos_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void midwunit_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t midwunit_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t midwunit_security_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void midwunit_security_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t midwunit_sound_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t midwunit_sound_state_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void midwunit_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void umk3_palette_hack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wwfmania_io_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void init_mk3r10();
	void init_nbahangt();
	void init_wwfmania();
	void init_umk3();
	void init_mk3();
	void init_openice();
	void init_rmpgwt();
	void init_umk3r11();
	void init_mk3r20();

	void machine_reset_midwunit();
	void video_start_midwunit();

	void register_state_saving();
	void init_wunit_generic();
	void init_mk3_common();
};

// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Midway X-unit games.

**************************************************************************/

#include "machine/midwayic.h"

class midxunit_state : public midtunit_state
{
public:
	midxunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: midtunit_state(mconfig, type, tag),
			m_nvram(*this, "nvram"),
			m_midway_serial_pic(*this, "serial_pic") { }

	required_shared_ptr<uint16_t> m_nvram;
	required_device<midway_serial_pic_device> m_midway_serial_pic;
	uint8_t m_cmos_write_enable;
	uint16_t m_iodata[8];
	uint8_t m_ioshuffle[16];
	uint8_t m_analog_port;
	uint8_t m_uart[8];
	uint8_t m_security_bits;
	uint16_t midxunit_cmos_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void midxunit_cmos_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void midxunit_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void midxunit_unknown_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t midxunit_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t midxunit_analog_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void midxunit_analog_select_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t midxunit_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t midxunit_uart_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void midxunit_uart_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t midxunit_security_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void midxunit_security_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void midxunit_security_clock_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t midxunit_sound_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t midxunit_sound_state_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void midxunit_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void midxunit_dcs_output_full(int state);
	void init_revx();
	void machine_reset_midxunit();
	void video_start_midxunit();
	void register_state_saving();
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);
};

// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn,Zsolt Vasvari,Aaron Giles
/*************************************************************************

    Gottlieb Exterminator hardware

*************************************************************************/
#include "cpu/tms32010/tms32010.h"
#include "cpu/tms34010/tms34010.h"

class exterm_state : public driver_device
{
public:
	exterm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_audioslave(*this, "audioslave"),
		m_slave(*this, "slave"),
		m_master_videoram(*this, "master_videoram"),
		m_slave_videoram(*this, "slave_videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_audioslave;
	required_device<tms34010_device> m_slave;

	required_shared_ptr<uint16_t> m_master_videoram;
	required_shared_ptr<uint16_t> m_slave_videoram;

	uint8_t m_aimpos[2];
	uint8_t m_trackball_old[2];
	uint8_t m_master_sound_latch;
	uint8_t m_slave_sound_latch;
	uint8_t m_sound_control;
	uint16_t m_last;

	void exterm_host_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t exterm_host_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t exterm_input_port_0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t exterm_input_port_1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void exterm_output_port_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_latch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_nmi_rate_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sound_master_latch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sound_slave_latch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sound_nmi_to_slave_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ym2151_data_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void palette_init_exterm(palette_device &palette);
	void sound_delayed_w(void *ptr, int32_t param);
	void master_sound_nmi_callback(timer_device &timer, void *ptr, int32_t param);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg_master);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg_master);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg_slave);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg_slave);
	uint16_t exterm_trackball_port_r(int which, uint16_t mem_mask);
};

// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/mc80.h
 *
 ****************************************************************************/

#ifndef MC80_H_
#define MC80_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"

class mc80_state : public driver_device
{
public:
	mc80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_p_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu") { }

	void mc8030_zve_write_protect_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mc8030_vis_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mc8030_eprom_prog_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ctc_z0_w(int state);
	void ctc_z1_w(int state);
	uint8_t mc80_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mc80_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mc80_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mc80_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t zve_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t zve_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void zve_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void zve_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t asp_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t asp_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void asp_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void asp_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	optional_shared_ptr<uint8_t> m_p_videoram;
	void machine_reset_mc8020();
	void video_start_mc8020();
	void machine_reset_mc8030();
	void video_start_mc8030();
	uint32_t screen_update_mc8020(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mc8030(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mc8020_kbd(timer_device &timer, void *ptr, int32_t param);
	void ctc_z2_w(int state);
	int mc8020_irq_callback(device_t &device, int irqline);
	int mc8030_irq_callback(device_t &device, int irqline);
	required_device<cpu_device> m_maincpu;
};

#endif

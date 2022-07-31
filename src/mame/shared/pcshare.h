// license:BSD-3-Clause
// copyright-holders:Peter Trauner
#ifndef MAME_MACHINE_PCSHARE_H
#define MAME_MACHINE_PCSHARE_H

#pragma once

#include "machine/am9517a.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/mc146818.h"
#include "machine/8042kbdc.h"

class pcat_base_state : public driver_device
{
public:
	pcat_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dma8237_1(*this, "dma8237_1"),
		m_dma8237_2(*this, "dma8237_2"),
		m_pic8259_1(*this, "pic8259_1"),
		m_pic8259_2(*this, "pic8259_2"),
		m_pit8254(*this, "pit8254"),
		m_mc146818(*this, "rtc"),
		m_kbdc(*this, "kbdc")
	{
	}

	DECLARE_WRITE_LINE_MEMBER(pc_dma_hrq_changed);
	uint8_t pc_dma_read_byte(offs_t offset);
	void pc_dma_write_byte(offs_t offset, uint8_t data);
	uint8_t dma_page_select_r(offs_t offset);
	void dma_page_select_w(offs_t offset, uint8_t data);
	void set_dma_channel(int channel, int state);
	DECLARE_WRITE_LINE_MEMBER( pc_dack0_w );
	DECLARE_WRITE_LINE_MEMBER( pc_dack1_w );
	DECLARE_WRITE_LINE_MEMBER( pc_dack2_w );
	DECLARE_WRITE_LINE_MEMBER( pc_dack3_w );
	uint8_t get_slave_ack(offs_t offset);
	DECLARE_WRITE_LINE_MEMBER( at_pit8254_out2_changed );

protected:
	void pcat_common(machine_config &config);
	void pcvideo_vga(machine_config &config);
	void pcvideo_trident_vga(machine_config &config);
	void pcvideo_s3_vga(machine_config &config);
	void pcvideo_cirrus_gd5428(machine_config &config);
	void pcvideo_cirrus_gd5430(machine_config &config);

	void pcat32_io_common(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<am9517a_device> m_dma8237_1;
	required_device<am9517a_device> m_dma8237_2;
	required_device<pic8259_device> m_pic8259_1;
	required_device<pic8259_device> m_pic8259_2;
	required_device<pit8254_device> m_pit8254;
	optional_device<mc146818_device> m_mc146818;
	required_device<kbdc8042_device> m_kbdc;

	int m_dma_channel = 0;
	uint8_t m_dma_offset[2][4]{};
	uint8_t m_at_pages[0x10]{};
	int m_pit_out2 = 0;
};

#endif // MAME_MACHINE_PCSHARE_H

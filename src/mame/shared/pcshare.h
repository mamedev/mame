// license:BSD-3-Clause
// copyright-holders:Peter Trauner
#ifndef MAME_SHARED_PCSHARE_H
#define MAME_SHARED_PCSHARE_H

#pragma once

#include "machine/8042kbdc.h"
#include "machine/am9517a.h"
#include "machine/mc146818.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"


class pcat_base_state : public driver_device
{
public:
	// cfr. https://github.com/mamedev/mame/issues/391
	[[deprecated("Leaky abstraction of a southbridge, to be replaced with actual chipset emulation.")]]
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

	void pc_dma_hrq_changed(int state);
	uint8_t pc_dma_read_byte(offs_t offset);
	void pc_dma_write_byte(offs_t offset, uint8_t data);
	uint8_t dma_page_select_r(offs_t offset);
	void dma_page_select_w(offs_t offset, uint8_t data);
	void set_dma_channel(int channel, int state);
	void pc_dack0_w(int state);
	void pc_dack1_w(int state);
	void pc_dack2_w(int state);
	void pc_dack3_w(int state);
	uint8_t get_slave_ack(offs_t offset);
	void at_pit8254_out2_changed(int state);

protected:
	void pcat_common(machine_config &config);
	void pcat_common_nokeyboard(machine_config &config);

	void pcat32_io_common(address_map &map) ATTR_COLD;

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

#endif // MAME_SHARED_PCSHARE_H

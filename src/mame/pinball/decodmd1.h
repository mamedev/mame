// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * Data East Pinball DMD Type 1 display
 */

#ifndef MAME_PINBALL_DECODMD1_H
#define MAME_PINBALL_DECODMD1_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/timer.h"


class decodmd_type1_device : public device_t
{
public:
	decodmd_type1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void data_w(uint8_t data);
	uint8_t busy_r();
	void ctrl_w(uint8_t data);
	uint8_t status_r();

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr unsigned B_CLR = 0x01;
	static constexpr unsigned B_SET = 0x02;
	static constexpr unsigned B_CLK = 0x04;

	required_device<cpu_device> m_cpu;
	required_memory_bank m_rombank;
	required_shared_ptr<uint8_t> m_ram;
	required_device<hc259_device> m_bitlatch;
	required_region_ptr<uint8_t> m_rom;

	uint8_t m_latch;
	uint8_t m_status;
	uint8_t m_ctrl;
	uint8_t m_busy;
	uint8_t m_command;
	uint8_t m_rowclock;
	uint8_t m_rowdata;
	uint32_t m_rowselect;
	uint8_t m_blank;
	uint32_t m_pxdata1;
	uint32_t m_pxdata2;
	uint32_t m_pxdata1_latched;
	uint32_t m_pxdata2_latched;
	bool m_frameswap;
	uint32_t m_pixels[0x200];
	uint8_t m_busy_lines;
	uint32_t m_prevrow;

	uint8_t latch_r();
	uint8_t ctrl_r();
	uint8_t dmd_port_r(offs_t offset);
	void dmd_port_w(offs_t offset, uint8_t data);

	void blank_w(int state);
	void status_w(int state);
	void rowdata_w(int state);
	void rowclock_w(int state);
	void test_w(int state);

	void output_data();
	void set_busy(uint8_t input, uint8_t val);
	uint32_t screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect );
	TIMER_DEVICE_CALLBACK_MEMBER(dmd_nmi);

	void decodmd1_map(address_map &map) ATTR_COLD;
	void decodmd1_io_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(DECODMD1, decodmd_type1_device)

#endif // MAME_PINBALL_DECODMD1_H

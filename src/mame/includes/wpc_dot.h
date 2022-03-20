// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * wpc_dot.h
 *
 *  Created on: 18/10/2013
 *      Author: bsr
 */

#ifndef MAME_INCLUDES_WPC_DOT_H
#define MAME_INCLUDES_WPC_DOT_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "audio/wpcsnd.h"
#include "audio/dcs.h"
#include "machine/wpc.h"

class wpc_dot_state : public driver_device
{
public:
	wpc_dot_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_wpcsnd(*this,"wpcsnd")
		, m_wpc(*this,"wpc")
		, m_cpubank(*this, "cpubank")
		, m_fixedbank(*this, "fixedbank")
		, m_dmdbanks(*this, "dmdbank%u", 1U)
	{ }

	void init_wpc_dot();
	void wpc_dot(machine_config &config);

protected:
	void wpc_dot_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;
	optional_device<wpcsnd_device> m_wpcsnd;
	required_device<wpc_device> m_wpc;
	required_memory_bank m_cpubank;
	required_memory_bank m_fixedbank;
	required_memory_bank_array<6> m_dmdbanks;

	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
	static const device_timer_id TIMER_VBLANK = 0;
	static const device_timer_id TIMER_IRQ = 1;

	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(wpcsnd_reply_w);
	DECLARE_WRITE_LINE_MEMBER(wpc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(wpc_firq_w);
	void wpc_rombank_w(uint8_t data);
	void wpc_dmdbank_w(offs_t offset, uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	uint16_t m_vblank_count = 0U;
	uint32_t m_irq_count = 0U;
	uint8_t m_bankmask = 0U;
	uint8_t m_ram[0x3000]{};
	uint8_t m_dmdram[0x2000]{};
	emu_timer* m_vblank_timer;
	emu_timer* m_irq_timer;
};

class wpc_flip1_state : public wpc_dot_state
{
public:
	wpc_flip1_state(const machine_config &mconfig, device_type type, const char *tag)
		: wpc_dot_state(mconfig, type, tag)
	{ }

	void init_wpc_flip1();
	void wpc_flip1(machine_config &config);

protected:
	void wpc_flip1_map(address_map &map);
};

class wpc_flip2_state : public wpc_flip1_state
{
public:
	wpc_flip2_state(const machine_config &mconfig, device_type type, const char *tag)
		: wpc_flip1_state(mconfig, type, tag)
	{ }

	void init_wpc_flip2();
	void wpc_flip2(machine_config &config);

protected:
	void wpc_flip2_map(address_map &map);
};

#endif // MAME_INCLUDES_WPC_DOT_H

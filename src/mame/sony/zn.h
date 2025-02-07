// license:BSD-3-Clause
// copyright-holders:smf, R. Belmont

#ifndef MAME_SONY_ZN_H
#define MAME_SONY_ZN_H

#pragma once

#include "cpu/psx/psx.h"
#include "machine/at28c16.h"
#include "machine/cat702.h"
#include "machine/ram.h"
#include "sound/spu.h"
#include "video/psx.h"
#include "screen.h"
#include "speaker.h"
#include "znmcu.h"

class zn_state : public driver_device
{
public:
	zn_state(const machine_config &mconfig, device_type type, const char *tag);

	void zn1_1mb_vram(machine_config &config) ATTR_COLD;
	void zn1_2mb_vram(machine_config &config) ATTR_COLD;
	void zn2(machine_config &config) ATTR_COLD;

protected:
	virtual void driver_start() override ATTR_COLD;

	void zn_base(machine_config &config) ATTR_COLD;
	template<unsigned N> void cat702(machine_config &config) ATTR_COLD;

	virtual void maincpu_program_map(address_map &map) ATTR_COLD;
	void zn2_maincpu_program_map(address_map &map) ATTR_COLD;

	template<unsigned N> void cat702_dataout(int state) { m_cat702_dataout[N] = state; update_sio0_rxd(); }
	void znmcu_dataout(int state) { m_znmcu_dataout = state; update_sio0_rxd(); }
	void update_sio0_rxd() { m_sio0->write_rxd(m_cat702_dataout[0] && m_cat702_dataout[1] && m_znmcu_dataout); }

	uint8_t boardconfig_r();
	uint8_t coin_r();
	virtual void coin_w(uint8_t data);
	uint16_t unknown_r(offs_t offset, uint16_t mem_mask = ~0);
	uint8_t znsecsel_r();
	virtual void znsecsel_w(uint8_t data);
	uint16_t zn2_spu_hack_r();

	required_device<psxcpu_device> m_maincpu;
	required_device<psxirq_device> m_irq;
	required_device<psxsio0_device> m_sio0;
	required_device<spu_device> m_spu;
	required_device<psxgpu_device> m_gpu;
	required_device<screen_device> m_screen;
	required_device_array<speaker_device, 2> m_speaker;
	required_device<at28c16_device> m_at28c16;
	optional_device_array<cat702_device, 2> m_cat702;
	required_device<ram_device> m_ram;
	required_device<znmcu_device> m_znmcu;

	std::array<int, 2> m_cat702_dataout;
	uint8_t m_coin;
	int m_znmcu_dataout;
	uint8_t m_znsecsel;
	uint16_t m_zn2_spu_hack;
};

#endif // MAME_SONY_ZN_H

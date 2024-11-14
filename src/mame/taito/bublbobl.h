// license:BSD-3-Clause
// copyright-holders:Chris Moore, Nicola Salmoria
#ifndef MAME_TAITO_BUBLBOBL_H
#define MAME_TAITO_BUBLBOBL_H

#pragma once

#include "cpu/m6800/m6801.h"
#include "cpu/m6805/m68705.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/input_merger.h"
#include "machine/gen_latch.h"
#include "sound/ymopn.h"
#include "sound/ymopl.h"
#include "emupal.h"
#include "screen.h"


class bublbobl_state : public driver_device
{
public:
	bublbobl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_videoram(*this, "videoram")
		, m_objectram(*this, "objectram")
		, m_mcu_sharedram(*this, "mcu_sharedram")
		, m_maincpu(*this, "maincpu")
		, m_mcu(*this, "mcu")
		, m_audiocpu(*this, "audiocpu")
		, m_subcpu(*this, "subcpu")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_ym2203(*this, "ym2203")
		, m_ym3526(*this, "ym3526")
		, m_soundirq(*this, "soundirq")
		, m_soundnmi(*this, "soundnmi")
		, m_main_to_sound(*this, "main_to_sound")
		, m_sound_to_main(*this, "sound_to_main")
	{ }

	void init_dland();
	void init_common();
	void tokio(machine_config &config);
	void boblbobl(machine_config &config);
	void bublbobl(machine_config &config);
	void bublbobl_nomcu(machine_config &config);
	void bublboblp(machine_config &config);
	void tokiob(machine_config &config);

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_objectram;
	optional_shared_ptr<uint8_t> m_mcu_sharedram;

	/* video-related */
	bool     m_video_enable = false;

	/* sound-related */
	int      m_sreset_old = 0;

	/* mcu-related */

	/* Bubble Bobble MCU */
	uint8_t    m_port3_in = 0U;
	uint8_t    m_port1_out = 0U;
	uint8_t    m_port2_out = 0U;
	uint8_t    m_port3_out = 0U;
	uint8_t    m_port4_out = 0U;
	emu_timer *m_irq_ack_timer = nullptr;
	/* Bobble Bobble */
	int      m_ic43_a = 0;
	int      m_ic43_b = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_mcu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<ym2203_device> m_ym2203;
	optional_device<ym3526_device> m_ym3526;
	optional_device<input_merger_device> m_soundirq;
	required_device<input_merger_device> m_soundnmi;
	required_device<generic_latch_8_device> m_main_to_sound;
	required_device<generic_latch_8_device> m_sound_to_main;

	void common_sreset(int state);
	void bublbobl_bankswitch_w(uint8_t data);
	void tokio_bankswitch_w(uint8_t data);
	void tokio_videoctrl_w(uint8_t data);
	void bublbobl_nmitrigger_w(uint8_t data);
	uint8_t tokiob_mcu_r();
	void bublbobl_soundcpu_reset_w(uint8_t data);
	uint8_t common_sound_semaphores_r();
	IRQ_CALLBACK_MEMBER(mcram_vect_r);
	void bublbobl_mcu_port1_w(uint8_t data);
	void bublbobl_mcu_port2_w(uint8_t data);
	uint8_t bublbobl_mcu_port3_r();
	void bublbobl_mcu_port3_w(uint8_t data);
	void bublbobl_mcu_port4_w(uint8_t data);
	uint8_t boblbobl_ic43_a_r(offs_t offset);
	void boblbobl_ic43_a_w(offs_t offset, uint8_t data);
	void boblbobl_ic43_b_w(offs_t offset, uint8_t data);
	uint8_t boblbobl_ic43_b_r(offs_t offset);

	DECLARE_MACHINE_START(tokio);
	DECLARE_MACHINE_RESET(tokio);
	DECLARE_MACHINE_START(bublbobl);
	DECLARE_MACHINE_RESET(bublbobl);
	DECLARE_MACHINE_START(boblbobl);
	DECLARE_MACHINE_RESET(boblbobl);
	DECLARE_MACHINE_START(common);
	DECLARE_MACHINE_RESET(common);
	uint32_t screen_update_bublbobl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void configure_banks();

	void bootleg_map(address_map &map) ATTR_COLD;
	void bublbobl_maincpu_map(address_map &map) ATTR_COLD;
	void common_maincpu_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void subcpu_map(address_map &map) ATTR_COLD;
	void tokio_map(address_map &map) ATTR_COLD;
	void tokio_map_bootleg(address_map &map) ATTR_COLD;
	void tokio_map_mcu(address_map &map) ATTR_COLD;
	void tokio_sound_map(address_map &map) ATTR_COLD;
	void tokio_subcpu_map(address_map &map) ATTR_COLD;

protected:
	TIMER_CALLBACK_MEMBER(irq_ack);
};


class bub68705_state : public bublbobl_state
{
public:
	bub68705_state(const machine_config &mconfig, device_type type, const char *tag)
		: bublbobl_state(mconfig, type, tag)
		, m_mcu(*this, "mcu")
		, m_mux_ports(*this, { "DSW0", "DSW1", "IN1", "IN2" })
		, m_port_a_out(0xff)
		, m_port_b_out(0xff)
		, m_address(0)
		, m_latch(0)
	{
	}

	void port_a_w(uint8_t data);
	void port_b_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);

	INTERRUPT_GEN_MEMBER(bublbobl_m68705_interrupt);

	DECLARE_MACHINE_START(bub68705);
	DECLARE_MACHINE_RESET(bub68705);

	void bub68705(machine_config &config);
protected:
	required_device<m68705p_device> m_mcu;
	required_ioport_array<4>        m_mux_ports;

	uint8_t     m_port_a_out = 0U;
	uint8_t     m_port_b_out = 0U;
	uint16_t    m_address = 0U;
	uint8_t     m_latch = 0U;
};


class bub8749_state : public bublbobl_state
{
public:
	bub8749_state(const machine_config &mconfig, device_type type, const char *tag)
		: bublbobl_state(mconfig, type, tag)
		, m_mcu(*this, "mcu")
	{
	}

	void bub8749(machine_config &config);

protected:
	required_device<i8749_device> m_mcu;
};

#endif // MAME_TAITO_BUBLBOBL_H

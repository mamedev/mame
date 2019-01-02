// license:BSD-3-Clause
// copyright-holders:Chris Moore, Nicola Salmoria
#ifndef MAME_INCLUDES_BUBLBOBL_H
#define MAME_INCLUDES_BUBLBOBL_H

#pragma once

#include "cpu/m6805/m68705.h"
#include "machine/input_merger.h"
#include "machine/gen_latch.h"
#include "sound/2203intf.h"
#include "sound/3526intf.h"
#include "emupal.h"
#include "screen.h"

#include "machine/taito68705interface.h"


class bublbobl_state : public driver_device
{
public:
	enum
	{
		TIMER_M68705_IRQ_ACK
	};

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

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_objectram;
	optional_shared_ptr<uint8_t> m_mcu_sharedram;

	/* video-related */
	bool     m_video_enable;

	/* sound-related */
	int      m_sreset_old;

	/* mcu-related */

	/* Bubble Bobble MCU */
	uint8_t    m_ddr1;
	uint8_t    m_ddr2;
	uint8_t    m_ddr3;
	uint8_t    m_ddr4;
	uint8_t    m_port1_in;
	uint8_t    m_port2_in;
	uint8_t    m_port3_in;
	uint8_t    m_port4_in;
	uint8_t    m_port1_out;
	uint8_t    m_port2_out;
	uint8_t    m_port3_out;
	uint8_t    m_port4_out;
	/* Bobble Bobble */
	int      m_ic43_a;
	int      m_ic43_b;

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
	DECLARE_WRITE8_MEMBER(bublbobl_bankswitch_w);
	DECLARE_WRITE8_MEMBER(tokio_bankswitch_w);
	DECLARE_WRITE8_MEMBER(tokio_videoctrl_w);
	DECLARE_WRITE8_MEMBER(bublbobl_nmitrigger_w);
	DECLARE_READ8_MEMBER(tokiob_mcu_r);
	DECLARE_WRITE8_MEMBER(bublbobl_soundcpu_reset_w);
	DECLARE_READ8_MEMBER(common_sound_semaphores_r);
	DECLARE_READ8_MEMBER(bublbobl_mcu_ddr1_r);
	DECLARE_WRITE8_MEMBER(bublbobl_mcu_ddr1_w);
	DECLARE_READ8_MEMBER(bublbobl_mcu_ddr2_r);
	DECLARE_WRITE8_MEMBER(bublbobl_mcu_ddr2_w);
	DECLARE_READ8_MEMBER(bublbobl_mcu_ddr3_r);
	DECLARE_WRITE8_MEMBER(bublbobl_mcu_ddr3_w);
	DECLARE_READ8_MEMBER(bublbobl_mcu_ddr4_r);
	DECLARE_WRITE8_MEMBER(bublbobl_mcu_ddr4_w);
	DECLARE_READ8_MEMBER(bublbobl_mcu_port1_r);
	DECLARE_WRITE8_MEMBER(bublbobl_mcu_port1_w);
	DECLARE_READ8_MEMBER(bublbobl_mcu_port2_r);
	DECLARE_WRITE8_MEMBER(bublbobl_mcu_port2_w);
	DECLARE_READ8_MEMBER(bublbobl_mcu_port3_r);
	DECLARE_WRITE8_MEMBER(bublbobl_mcu_port3_w);
	DECLARE_READ8_MEMBER(bublbobl_mcu_port4_r);
	DECLARE_WRITE8_MEMBER(bublbobl_mcu_port4_w);
	DECLARE_READ8_MEMBER(boblbobl_ic43_a_r);
	DECLARE_WRITE8_MEMBER(boblbobl_ic43_a_w);
	DECLARE_WRITE8_MEMBER(boblbobl_ic43_b_w);
	DECLARE_READ8_MEMBER(boblbobl_ic43_b_r);

	void init_dland();
	void init_common();
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

	void tokio(machine_config &config);
	void boblbobl(machine_config &config);
	void bublbobl(machine_config &config);
	void bublbobl_nomcu(machine_config &config);
	void bublboblp(machine_config &config);
	void tokiob(machine_config &config);
	void bootleg_map(address_map &map);
	void bublbobl_maincpu_map(address_map &map);
	void common_maincpu_map(address_map &map);
	void mcu_map(address_map &map);
	void sound_map(address_map &map);
	void subcpu_map(address_map &map);
	void tokio_map(address_map &map);
	void tokio_map_bootleg(address_map &map);
	void tokio_map_mcu(address_map &map);
	void tokio_sound_map(address_map &map);
	void tokio_subcpu_map(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
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

	DECLARE_WRITE8_MEMBER(port_a_w);
	DECLARE_WRITE8_MEMBER(port_b_w);

	INTERRUPT_GEN_MEMBER(bublbobl_m68705_interrupt);

	DECLARE_MACHINE_START(bub68705);
	DECLARE_MACHINE_RESET(bub68705);

	void bub68705(machine_config &config);
protected:
	required_device<m68705p_device> m_mcu;
	required_ioport_array<4>        m_mux_ports;

	uint8_t     m_port_a_out;
	uint8_t     m_port_b_out;
	uint16_t    m_address;
	uint8_t     m_latch;
};

#endif // MAME_INCLUDES_BUBLBOBL_H

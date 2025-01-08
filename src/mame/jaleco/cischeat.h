// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_JALECO_CISCHEAT_H
#define MAME_JALECO_CISCHEAT_H

#pragma once

// TODO: better inheritance, eventually split individual driver files

#include "sound/okim6295.h"
#include "machine/gen_latch.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "ms1_gatearray.h"
#include "ms1_tmap.h"
#include "emupal.h"
#include "screen.h"

class cischeat_state : public driver_device
{
public:
	cischeat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_tmap(*this, "scroll%u", 0)
		, m_ram(*this, "ram")
		, m_roadram(*this, "roadram.%u", 0)
		, m_f1gpstr2_ioready(*this, "ioready")
		, m_maincpu(*this, "maincpu")
		, m_cpu1(*this, "cpu1")
		, m_cpu2(*this, "cpu2")
		, m_cpu3(*this, "cpu3")
		, m_cpu5(*this, "cpu5")
		, m_soundcpu(*this, "soundcpu")
		, m_screen(*this, "screen")
		, m_watchdog(*this, "watchdog")
		, m_oki1(*this, "oki1")
		, m_oki2(*this, "oki2")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_soundlatch2(*this, "soundlatch2")
		, m_gatearray(*this, "gatearray")
		, m_leds(*this, "led%u", 0U)
	{}

	void scudhamm_motor_command_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void scudhamm_leds_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void scudhamm_enable_w(uint16_t data);
	void scudhamm_oki_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bigrun_soundbank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t scudhamm_motor_status_r();
	uint16_t scudhamm_motor_pos_r();
	uint8_t scudhamm_analog_r();
	uint16_t bigrun_ip_select_r();
	void leds_out_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void unknown_out_w(uint16_t data);
	void motor_out_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void wheel_out_w(uint16_t data);
	void ip_select_w(uint16_t data);
	void ip_select_plus1_w(uint16_t data);
	void bigrun_comms_w(uint16_t data);
	void active_layers_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t cischeat_ip_select_r();
	void cischeat_soundlatch_w(uint16_t data);
	void cischeat_comms_w(uint16_t data);
	uint16_t f1gpstar_wheel_r();
	uint16_t f1gpstr2_ioready_r();
	uint16_t wildplt_xy_r();
	uint16_t wildplt_mux_r();
	void wildplt_mux_w(uint16_t data);
	void f1gpstar_motor_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void f1gpstar_soundint_w(uint16_t data);
	void f1gpstar_comms_w(uint16_t data);
	void f1gpstr2_io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cischeat_soundbank_1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cischeat_soundbank_2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_irq(int state);
	void init_cischeat();
	void init_bigrun();
	void init_f1gpstar();
	uint32_t screen_update_bigrun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_scudhamm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cischeat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_f1gpstar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(bigrun_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(scudhamm_scanline);
	void prepare_shadows();
	void cischeat_draw_road(bitmap_ind16 &bitmap, const rectangle &cliprect, int road_num, int priority1, int priority2, int transparency);
	void f1gpstar_draw_road(bitmap_ind16 &bitmap, const rectangle &cliprect, int road_num, int priority1, int priority2, int transparency);
	void cischeat_draw_sprites(bitmap_ind16 &bitmap , const rectangle &cliprect, int priority1, int priority2);
	void bigrun_draw_sprites(bitmap_ind16 &bitmap , const rectangle &cliprect, int priority1, int priority2);
	void cischeat_untangle_sprites(const char *region);

	void scudhamm(machine_config &config);
	void cischeat(machine_config &config);
	void f1gpstr2(machine_config &config);
	void f1gpstar(machine_config &config);
	void bigrun(machine_config &config);
	void bigrun_d65006(machine_config &config);
	void cischeat_gs88000(machine_config &config);

	void bigrun_map(address_map &map) ATTR_COLD;
	void bigrun_map2(address_map &map) ATTR_COLD;
	void bigrun_map3(address_map &map) ATTR_COLD;
	void bigrun_sound_map(address_map &map) ATTR_COLD;
	void cischeat_map(address_map &map) ATTR_COLD;
	void cischeat_map2(address_map &map) ATTR_COLD;
	void cischeat_map3(address_map &map) ATTR_COLD;
	void cischeat_sound_map(address_map &map) ATTR_COLD;
	void f1gpstar_map(address_map &map) ATTR_COLD;
	void f1gpstar_map2(address_map &map) ATTR_COLD;
	void f1gpstar_map3(address_map &map) ATTR_COLD;
	void f1gpstar_sound_map(address_map &map) ATTR_COLD;
	void f1gpstr2_io_map(address_map &map) ATTR_COLD;
	void f1gpstr2_map(address_map &map) ATTR_COLD;
	void f1gpstr2_sound_map(address_map &map) ATTR_COLD;
	void scudhamm_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override
	{
		m_leds.resolve(); m_scudhamm_motor_command = 0;
	}

	virtual void video_start() override ATTR_COLD;

	optional_device_array<megasys1_tilemap_device, 3> m_tmap;
	required_shared_ptr<uint16_t> m_ram;
	optional_shared_ptr_array<uint16_t,2> m_roadram;
	optional_shared_ptr<uint16_t> m_f1gpstr2_ioready;

	uint16_t m_active_layers = 0U;

	int m_prev = 0;
	uint16_t m_scudhamm_motor_command = 0U;
	int m_ip_select = 0;
	uint16_t m_wildplt_output = 0U;
	uint8_t m_drawmode_table[16]{};
#ifdef MAME_DEBUG
	int m_debugsprites = 0;
#endif
	int m_show_unknown = 0;
	uint16_t *m_spriteram = nullptr;

	uint8_t m_motor_value = 0U;
	uint8_t m_io_value = 0U;

	// TODO: make these to have a more meaningful name
	optional_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_cpu1;
	optional_device<cpu_device> m_cpu2;
	optional_device<cpu_device> m_cpu3;
	optional_device<cpu_device> m_cpu5;
	optional_device<cpu_device> m_soundcpu;
	required_device<screen_device> m_screen;
	optional_device<watchdog_timer_device> m_watchdog;
	required_device<okim6295_device> m_oki1;
	required_device<okim6295_device> m_oki2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_16_device> m_soundlatch;
	optional_device<generic_latch_16_device> m_soundlatch2;
	optional_device<megasys1_gatearray_device> m_gatearray;
	output_finder<5> m_leds;
};

class armchamp2_state : public cischeat_state
{
public:
	armchamp2_state(const machine_config &mconfig, device_type type, const char *tag)
		: cischeat_state(mconfig, type, tag)
	{
		m_arm_motor_command = 0;
		m_armold = 0;
	}

	uint16_t motor_status_r();
	void motor_command_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t analog_r();
	void output_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void armchmp2(machine_config &config);
	void armchmp2_map(address_map &map) ATTR_COLD;
	TIMER_DEVICE_CALLBACK_MEMBER(armchamp2_scanline);
	ioport_value left_sensor_r();
	ioport_value right_sensor_r();
	ioport_value center_sensor_r();

private:
	u16 m_arm_motor_command;
	int m_armold;
};

class wildplt_state : public cischeat_state
{
public:
	wildplt_state(const machine_config &mconfig, device_type type, const char *tag)
		: cischeat_state(mconfig, type, tag)
	{}

	uint16_t *m_buffer_spriteram = nullptr;
	std::unique_ptr<uint16_t[]> m_allocated_spriteram;
	void wildplt_map(address_map &map) ATTR_COLD;
	void wildplt(machine_config &config);
	void sprite_dma_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	uint16_t m_sprite_dma_reg = 0U;
};

class captflag_state : public cischeat_state
{
public:
	captflag_state(const machine_config &mconfig, device_type type, const char *tag)
		: cischeat_state(mconfig, type, tag)
		, m_hopper(*this, "hopper")
		, m_motor_left(*this, "motor_left")
		, m_motor_right(*this, "motor_right")
		, m_oki1_bank(*this, "oki1_bank")
		, m_oki2_bank(*this, "oki2_bank")
		, m_motor_left_output(*this, "left")
		, m_motor_right_output(*this, "right")
	{
		for (int side = 0; side < 2; ++side)
			m_motor_command[side] = m_motor_pos[side] = 0;
		m_captflag_leds = 0;
	}

	void captflag(machine_config &config);
	template <int N> int motor_busy_r();
	template <int N> ioport_value motor_pos_r();
	void init_captflag();
	void init_vscaptfl();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void motor_command_right_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void motor_command_left_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void motor_move(int side, uint16_t data);
	void oki_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void leds_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TIMER_DEVICE_CALLBACK_MEMBER(captflag_scanline);

	void captflag_map(address_map &map) ATTR_COLD;
	void oki1_map(address_map &map) ATTR_COLD;
	void oki2_map(address_map &map) ATTR_COLD;

	required_device<ticket_dispenser_device> m_hopper;

	required_device<timer_device> m_motor_left;
	required_device<timer_device> m_motor_right;

	required_memory_bank m_oki1_bank;
	required_memory_bank m_oki2_bank;

	output_finder<> m_motor_left_output;
	output_finder<> m_motor_right_output;

	uint16_t m_captflag_leds;
	uint16_t m_motor_command[2];
	uint16_t m_motor_pos[2];
};

#endif // MAME_JALECO_CISCHEAT_H

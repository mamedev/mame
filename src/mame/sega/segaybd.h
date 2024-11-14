// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega Y-Board hardware

***************************************************************************/
#ifndef MAME_SEGA_SEGAYBD_H
#define MAME_SEGA_SEGAYBD_H

#pragma once

#include "cpu/m68000/m68000musashi.h"
#include "cpu/z80/z80.h"
#include "machine/mb3773.h"
#include "segaic16.h"
#include "sega16sp.h"
#include "screen.h"


// ======================> segaybd_state

class segaybd_state : public sega_16bit_common_base
{
public:
	// construction/destruction
	segaybd_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_16bit_common_base(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subx(*this, "subx")
		, m_suby(*this, "suby")
		, m_soundcpu(*this, "soundcpu")
		, m_linkcpu(*this, "linkcpu")
		, m_watchdog(*this, "watchdog")
		, m_screen(*this, "screen")
		, m_bsprites(*this, "bsprites")
		, m_ysprites(*this, "ysprites")
		, m_segaic16vid(*this, "segaic16vid")
		, m_adc_ports(*this, "ADC.%u", 0)
		, m_start_lamp(*this, "start_lamp")
		, m_right_motor_position(*this, "right_motor_position")
		, m_right_motor_position_nor(*this, "right_motor_position_nor")
		, m_right_motor_speed(*this, "right_motor_speed")
		, m_left_motor_position(*this, "left_motor_position")
		, m_left_motor_position_nor(*this, "left_motor_position_nor")
		, m_left_motor_speed(*this, "left_motor_speed")
		, m_danger_lamp(*this, "danger_lamp")
		, m_crash_lamp(*this, "crash_lamp")
		, m_emergency_stop_lamp(*this, "emergency_stop_lamp")
		, m_bank_data_raw(*this, "bank_data_raw")
		, m_vibration_motor(*this, "vibration_motor")
		, m_bank_motor_position(*this, "bank_motor_position")
		, m_upright_wheel_motor(*this, "upright_wheel_motor")
		, m_left_start_lamp(*this, "left_start_lamp")
		, m_right_start_lamp(*this, "right_start_lamp")
		, m_gun_recoil(*this, "P%_Gun_Recoil", 1U)
	{
	}

	void yboard_deluxe(machine_config &config);
	void yboard_link(machine_config &config);
	void yboard(machine_config &config);

	// game-specific driver init
	void init_generic();
	void init_pdrift();
	void init_r360();
	void init_gforce2();
	void init_rchase();
	void init_gloc();

protected:
	// device overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// main CPU read/write handlers
	void output1_w(uint8_t data);
	void misc_output_w(uint8_t data);
	void output2_w(uint8_t data);

	// linked cabinet specific handlers
	void mb8421_intl(int state);
	void mb8421_intr(int state);
	uint16_t link_r();
	uint16_t link2_r();
	void link2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	//uint8_t link_portc0_r();

	// input helpers
	ioport_value analog_mux();

	// game-specific output handlers
	void gforce2_output_cb1(uint16_t data);
	void gforce2_output_cb2(uint16_t data);
	void gloc_output_cb1(uint16_t data);
	void gloc_output_cb2(uint16_t data);
	void r360_output_cb2(uint16_t data);
	void pdrift_output_cb1(uint16_t data);
	void pdrift_output_cb2(uint16_t data);
	void rchase_output_cb2(uint16_t data);

	// video updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void link_map(address_map &map) ATTR_COLD;
	void link_portmap(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void main_map_link(address_map &map) ATTR_COLD;
	void motor_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_portmap(address_map &map) ATTR_COLD;
	void subx_map(address_map &map) ATTR_COLD;
	void suby_map(address_map &map) ATTR_COLD;

	// internal types
	typedef delegate<void (uint16_t)> output_delegate;

	// internal helpers
	TIMER_CALLBACK_MEMBER(irq2_gen_tick);
	void update_irqs();

	// devices
	required_device<m68000msh_device> m_maincpu;
	required_device<m68000msh_device> m_subx;
	required_device<m68000msh_device> m_suby;
	required_device<z80_device> m_soundcpu;
	optional_device<z80_device> m_linkcpu;
	required_device<mb3773_device> m_watchdog;
	required_device<screen_device> m_screen;
	required_device<sega_sys16b_sprite_device> m_bsprites;
	required_device<sega_yboard_sprite_device> m_ysprites;
	required_device<segaic16_video_device> m_segaic16vid;

	// input ports
	optional_ioport_array<6> m_adc_ports;

	// outputs
	output_finder<> m_start_lamp;
	output_finder<> m_right_motor_position;
	output_finder<> m_right_motor_position_nor;
	output_finder<> m_right_motor_speed;
	output_finder<> m_left_motor_position;
	output_finder<> m_left_motor_position_nor;
	output_finder<> m_left_motor_speed;
	output_finder<> m_danger_lamp;
	output_finder<> m_crash_lamp;
	output_finder<> m_emergency_stop_lamp;
	output_finder<> m_bank_data_raw;
	output_finder<> m_vibration_motor;
	output_finder<> m_bank_motor_position;
	output_finder<> m_upright_wheel_motor;
	output_finder<> m_left_start_lamp;
	output_finder<> m_right_start_lamp;
	output_finder<2> m_gun_recoil;

	// configuration
	output_delegate m_output_cb1;
	output_delegate m_output_cb2;

	// internal state
	uint16_t m_pdrift_bank = 0;
	emu_timer *m_scanline_timer = nullptr;
	int m_irq2_scanline = 0;
	uint8_t m_timer_irq_state = 0;
	uint8_t m_vblank_irq_state = 0;
	uint8_t m_misc_io_data = 0;
};

#endif // MAME_SEGA_SEGAYBD_H

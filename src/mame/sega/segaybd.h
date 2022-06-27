// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega Y-Board hardware

***************************************************************************/
#ifndef MAME_INCLUDES_SEGAYBD_H
#define MAME_INCLUDES_SEGAYBD_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/mb3773.h"
#include "video/segaic16.h"
#include "video/sega16sp.h"
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
		, m_pdrift_bank(0)
		, m_scanline_timer(nullptr)
		, m_irq2_scanline(0)
		, m_timer_irq_state(0)
		, m_vblank_irq_state(0)
		, m_misc_io_data(0)
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

private:
	// main CPU read/write handlers
	void output1_w(uint8_t data);
	void misc_output_w(uint8_t data);
	void output2_w(uint8_t data);

	// linked cabinet specific handlers
	DECLARE_WRITE_LINE_MEMBER(mb8421_intl);
	DECLARE_WRITE_LINE_MEMBER(mb8421_intr);
	uint16_t link_r();
	uint16_t link2_r();
	void link2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
//  uint8_t link_portc0_r();

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

	void link_map(address_map &map);
	void link_portmap(address_map &map);
	void main_map(address_map &map);
	void main_map_link(address_map &map);
	void motor_map(address_map &map);
	void sound_map(address_map &map);
	void sound_portmap(address_map &map);
	void subx_map(address_map &map);
	void suby_map(address_map &map);

	// internal types
	typedef delegate<void (uint16_t)> output_delegate;

	// device overrides
	virtual void machine_reset() override;
	virtual void video_start() override;

	// internal helpers
	TIMER_CALLBACK_MEMBER(irq2_gen_tick);
	void update_irqs();

	// devices
	required_device<m68000_device> m_maincpu;
	required_device<m68000_device> m_subx;
	required_device<m68000_device> m_suby;
	required_device<z80_device> m_soundcpu;
	optional_device<z80_device> m_linkcpu;
	required_device<mb3773_device> m_watchdog;
	required_device<screen_device> m_screen;
	required_device<sega_sys16b_sprite_device> m_bsprites;
	required_device<sega_yboard_sprite_device> m_ysprites;
	required_device<segaic16_video_device> m_segaic16vid;

	// input ports
	optional_ioport_array<6> m_adc_ports;

	// configuration
	output_delegate m_output_cb1;
	output_delegate m_output_cb2;

	// internal state
	uint16_t          m_pdrift_bank = 0;
	emu_timer *     m_scanline_timer = nullptr;
	int             m_irq2_scanline = 0;
	uint8_t           m_timer_irq_state = 0;
	uint8_t           m_vblank_irq_state = 0;
	uint8_t           m_misc_io_data = 0;
};

#endif // MAME_INCLUDES_SEGAYBD_H

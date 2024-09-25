// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Midway Zeus games

**************************************************************************/
#ifndef MAME_MIDWAY_MIDZEUS_H
#define MAME_MIDWAY_MIDZEUS_H

#pragma once

#include "midwayic.h"

#include "machine/timekpr.h"
#include "video/poly.h"

#include "emupal.h"
#include "screen.h"

/*************************************
 *
 *  Type definitions
 *
 *************************************/

struct mz_poly_extra_data
{
	const void      *palbase;
	const void      *texbase;
	uint16_t        solidcolor = 0;
	uint16_t        voffset = 0;
	int16_t         zoffset = 0;
	uint16_t        transcolor = 0;
	uint16_t        texwidth = 0;
	uint16_t        color = 0;
	uint32_t        alpha = 0;
	uint32_t        ctrl_word = 0;
	bool            blend_enable = false;
	bool            depth_test_enable = false;
	bool            depth_write_enable = false;
	uint32_t        blend = 0;
	uint8_t         (*get_texel)(const void *, int, int, int);
};


class midzeus_state;

class midzeus_renderer : public poly_manager<float, mz_poly_extra_data, 4>
{
public:
	midzeus_renderer(midzeus_state &state);

	void render_poly(int32_t scanline, const extent_t& extent, const mz_poly_extra_data& object, int threadid);
	void render_poly_solid_fixedz(int32_t scanline, const extent_t& extent, const mz_poly_extra_data& object, int threadid);

	void zeus_draw_quad(int long_fmt, const uint32_t *databuffer, uint32_t texdata, bool logit);
	void zeus_draw_debug_quad(const rectangle& rect, const vertex_t* vert);

private:
	midzeus_state& m_state;
};

typedef midzeus_renderer::vertex_t poly_vertex;


class midzeus_state : public driver_device
{
	friend class midzeus_renderer;

public:
	midzeus_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_zeusbase(*this, "zeusbase"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_ioasic(*this, "ioasic"),
		m_ram_base(*this, "ram_base"),
		m_nvram(*this, "nvram"),
		m_tms32032_control(*this, "tms32032_ctl"),
		m_mainbank(*this, "mainbank")
	{ }

	void mk4(machine_config &config);

	optional_shared_ptr<uint32_t> m_zeusbase;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void cmos_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cmos_r(offs_t offset);
	void cmos_protect_w(uint32_t data);
	uint32_t disk_asic_jr_r(offs_t offset);
	void disk_asic_jr_w(offs_t offset, uint32_t data);
	uint32_t tms32032_control_r(offs_t offset);
	void tms32032_control_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t zeus_r(offs_t offset);
	void zeus_w(offs_t offset, uint32_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(display_irq);
	TIMER_CALLBACK_MEMBER(display_irq_off);

	void zeus_map(address_map &map) ATTR_COLD;
	void midzeus(machine_config &config);

	emu_timer *     m_display_irq_off_timer = nullptr;
	uint32_t        m_disk_asic_jr[0x10]{};

	uint8_t         m_cmos_protected = 0;

	emu_timer *     m_timer[2]{};

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	required_device<midway_ioasic_device> m_ioasic;
	required_shared_ptr<uint32_t> m_ram_base;
	required_shared_ptr<uint32_t> m_nvram;
	required_shared_ptr<uint32_t> m_tms32032_control;
	optional_memory_bank m_mainbank;

private:
	static inline constexpr XTAL MIDZEUS_VIDEO_CLOCK = 66.6667_MHz_XTAL;

	void exit_handler();
	void zeus_pointer_w(uint32_t which, uint32_t data, bool logit);
	void zeus_register16_w(offs_t offset, uint16_t data, bool logit);
	void zeus_register32_w(offs_t offset, uint32_t data, bool logit);
	void zeus_register_update(offs_t offset);
	int zeus_fifo_process(const uint32_t *data, int numwords);
	void zeus_draw_model(uint32_t texdata, bool logit);

	void log_fifo_command(const uint32_t *data, int numwords, const char *suffix);
	void log_waveram(uint32_t length_and_base);

	void *waveram0_ptr_from_block_addr(uint32_t addr);
	void *waveram0_ptr_from_expanded_addr(uint32_t addr);
	void *waveram1_ptr_from_expanded_addr(uint32_t addr);
	void *waveram0_ptr_from_texture_addr(uint32_t addr, int width);
	void waveram_plot_depth(int y, int x, uint16_t color, uint16_t depth);
	void waveram_plot(int y, int x, uint16_t color);
	void waveram_plot_check_depth(int y, int x, uint16_t color, uint16_t depth);
	void waveram_plot_check_depth_nowrite(int y, int x, uint16_t color, uint16_t depth);

	std::unique_ptr<midzeus_renderer> m_poly;
	uint8_t     m_log_fifo = 0;

	uint32_t    m_zeus_fifo[20]{};
	uint8_t     m_zeus_fifo_words = 0;
	int16_t     m_zeus_matrix[3][3]{};
	int32_t     m_zeus_point[3]{};
	int16_t     m_zeus_light[3]{};
	void *      m_zeus_renderbase = 0;
	uint32_t    m_zeus_palbase = 0;
	uint32_t    m_zeus_unkbase = 0;
	int         m_zeus_enable_logging = 0;
	uint32_t    m_zeus_objdata = 0;
	rectangle   m_zeus_cliprect;

	std::unique_ptr<uint32_t[]> m_waveram[2];
	int         m_yoffs = 0;
	int         m_texel_width = 0;
	int         m_is_mk4b = 0;
};

class invasnab_state : public midzeus_state
{
public:
	invasnab_state(const machine_config &mconfig, device_type type, const char *tag) :
		midzeus_state(mconfig, type, tag),
		m_io_gun_x(*this, "GUNX%u", 1U),
		m_io_gun_y(*this, "GUNY%u", 1U)
	{ }

	void invasn(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void invasn_gun_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t invasn_gun_r();

	void update_gun_irq();
	TIMER_CALLBACK_MEMBER(invasn_gun_callback);

	void invasnab_map(address_map &map) ATTR_COLD;

	uint32_t        m_gun_control = 0;
	uint8_t         m_gun_irq_state = 0;
	emu_timer *     m_gun_timer[2]{};
	int32_t         m_gun_x[2]{}, m_gun_y[2]{};

	required_ioport_array<2> m_io_gun_x;
	required_ioport_array<2> m_io_gun_y;
};

#endif // MAME_MIDWAY_MIDZEUS_H

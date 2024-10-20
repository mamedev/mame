// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_SEGA_MODEL1_H
#define MAME_SEGA_MODEL1_H

#pragma once

#include "dsbz80.h"
#include "m1comm.h"
#include "segaic24.h"

#include "segam1audio.h"

#include "cpu/mb86233/mb86233.h"
#include "cpu/v60/v60.h"
#include "machine/i8251.h"
#include "machine/gen_fifo.h"
#include "machine/mb8421.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"

#include <glm/vec3.hpp>

#include <functional>

class model1_state : public driver_device
{
public:
	model1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dpram(*this, "dpram")
		, m_m1audio(*this, M1AUDIO_TAG)
		, m_m1uart(*this, "m1uart")
		, m_m1comm(*this, "m1comm")
		, m_dsbz80(*this, "dsbz80")
		, m_tgp_copro(*this, "tgp_copro")
		, m_screen(*this, "screen")
		, m_copro_fifo_in(*this, "copro_fifo_in")
		, m_copro_fifo_out(*this, "copro_fifo_out")
		, m_poly_rom(*this, "polygons")
		, m_copro_tables(*this, "copro_tables")
		, m_copro_data(*this, "copro_data")
		, m_display_list0(*this, "display_list0")
		, m_display_list1(*this, "display_list1")
		, m_color_xlat(*this, "color_xlat")
		, m_paletteram16(*this, "palette")
		, m_palette(*this, "palette")
		, m_tiles(*this, "tile")
		, m_digits(*this, "digit%u", 0U)
		, m_outs(*this, "out%u", 0U)
		, m_throttle(*this, "THROTTLE")
	{
	}

	void model1(machine_config &config);
	void vf(machine_config &config);
	void vr(machine_config &config);
	void vformula(machine_config &config);
	void swa(machine_config &config);
	void wingwar(machine_config &config);
	void wingwar360(machine_config &config);
	void netmerc(machine_config &config);

	struct spoint_t
	{
		int32_t x = 0, y = 0;
	};

	struct point_t
	{
		float x = 0, y = 0, z = 0;
		float xx = 0, yy = 0;
		spoint_t s;
	};

	class quad_t
	{
	public:
		quad_t() { }
		quad_t(int ccol, float cz, point_t* p0, point_t* p1, point_t* p2, point_t* p3)
			: p{ p0, p1, p2, p3 }
			, z(cz)
			, col(ccol)
		{
		}

		int compare(const quad_t* other) const;

		point_t *p[4] = { nullptr, nullptr, nullptr, nullptr };
		float z = 0;
		int col = 0;
	};

private:
	// Machine
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void bank_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	TIMER_DEVICE_CALLBACK_MEMBER(model1_interrupt);
	IRQ_CALLBACK_MEMBER(irq_callback);

	// TGP
	u16 fifoin_status_r();

	u16 v60_copro_fifo_r(offs_t offset);
	void v60_copro_fifo_w(offs_t offset, u16 data);
	u16 v60_copro_ram_adr_r();
	void v60_copro_ram_adr_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 v60_copro_ram_r(offs_t offset);
	void v60_copro_ram_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void copro_sincos_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 copro_sincos_r(offs_t offset);
	void copro_inv_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 copro_inv_r(offs_t offset);
	void copro_isqrt_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 copro_isqrt_r(offs_t offset);
	void copro_atan_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 copro_atan_r();
	void copro_data_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 copro_data_r(offs_t offset);
	void copro_ramadr_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 copro_ramadr_r(offs_t offset);
	void copro_ramdata_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 copro_ramdata_r(offs_t offset);

	void copro_reset();

	u32 m_copro_sincos_base = 0;
	u32 m_copro_inv_base = 0;
	u32 m_copro_isqrt_base = 0;
	u32 m_copro_atan_base[4]{};
	u32 m_copro_data_base = 0;
	u32 m_copro_ram_adr[4]{};

	uint16_t m_r360_state = 0;
	uint8_t r360_r();
	void r360_w(uint8_t data);

	// Rendering
	virtual void video_start() override ATTR_COLD;
	u16 model1_listctl_r(offs_t offset);
	void model1_listctl_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	uint32_t screen_update_model1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank_model1(int state);

	struct lightparam_t
	{
		float a = 0;
		float d = 0;
		float s = 0;
		int p = 0;
	};

	class view_t
	{
	public:
		view_t() {
			light.x = 0;
			light.y = 0;
			light.z = 0;
		}

		void init_translation_matrix();

		void set_viewport(float xcenter, float ycenter, float xl, float xr, float yb, float yt);
		void set_lightparam(int index, float diffuse, float ambient, float specular, int power);
		void set_zoom(float x, float y);
		void set_light_direction(float x, float y, float z);
		void set_translation_matrix(float* mat);
		void set_view_translation(float x, float y);

		void project_point(point_t *p) const;
		void project_point_direct(point_t *p) const;

		void transform_vector(glm::vec3& p) const;
		void transform_point(point_t *p) const;

		void recompute_frustum();

		int xc = 0, yc = 0, x1 = 0, y1 = 0, x2 = 0, y2 = 0;
		float zoomx = 0, zoomy = 0, viewx = 0, viewy = 0;
		float a_bottom = 0, a_top = 0, a_left = 0, a_right = 0;
		float vxx = 0, vyy = 0, vzz = 0, ayy = 0, ayyc = 0, ayys = 0;
		float translation[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		glm::vec3 light;
		lightparam_t lightparams[32];
	};

	void model1_io(address_map &map) ATTR_COLD;
	void model1_mem(address_map &map) ATTR_COLD;
	void model1_comm_mem(address_map &map) ATTR_COLD;

	void copro_prog_map(address_map &map) ATTR_COLD;
	void copro_data_map(address_map &map) ATTR_COLD;
	void copro_external_map(address_map &map) ATTR_COLD;
	void copro_io_map(address_map &map) ATTR_COLD;
	void copro_rf_map(address_map &map) ATTR_COLD;

	void polhemus_map(address_map &map) ATTR_COLD;

	// Machine
	void irq_raise(int level);
	void irq_init();
	void irq_control_w(u8 data);

	uint8_t m_irq_status = 0;
	int m_last_irq = 0;

	// Devices
	required_device<v60_device> m_maincpu;          // V60
	required_device<mb8421_device> m_dpram;
	required_device<segam1audio_device> m_m1audio;  // Model 1 standard sound board
	required_device<i8251_device> m_m1uart;
	optional_device<m1comm_device> m_m1comm;        // Model 1 communication board
	optional_device<dsbz80_device> m_dsbz80;        // Digital Sound Board
	optional_device<mb86233_device> m_tgp_copro;
	required_device<screen_device> m_screen;
	required_device<generic_fifo_u32_device> m_copro_fifo_in, m_copro_fifo_out;

	required_region_ptr<uint32_t> m_poly_rom;
	required_region_ptr<uint32_t> m_copro_tables;
	optional_memory_region        m_copro_data;

	required_shared_ptr<uint16_t> m_display_list0;
	required_shared_ptr<uint16_t> m_display_list1;
	required_shared_ptr<uint16_t> m_color_xlat;

	// Sound
	int m_sound_irq = 0;

	// TGP FIFO
	void    fifoout_push(uint32_t data);
	void    fifoout_push_f(float data);
	uint32_t  fifoin_pop();
	float   fifoin_pop_f();
	uint16_t  ram_get_i();
	float   ram_get_f();
	u32 m_v60_copro_fifo_r = 0, m_v60_copro_fifo_w = 0;

	// TGP
	void    tgp_reset();
	class clipper_t
	{
	public:
		clipper_t()
			: m_isclipped(nullptr)
			, m_clip(nullptr)
		{
		}

		clipper_t(std::function<bool(view_t*, point_t*)> isclipped, std::function<void(view_t*, point_t*, point_t*, point_t*)> clip)
			: m_isclipped(isclipped)
			, m_clip(clip)
		{
		}

		std::function<bool(view_t*, point_t*)> m_isclipped;
		std::function<void(view_t*, point_t*, point_t*, point_t*)> m_clip;
	};

	std::unique_ptr<view_t> m_view;
	std::unique_ptr<point_t[]> m_pointdb;
	point_t *m_pointpt;
	std::unique_ptr<quad_t[]> m_quaddb;
	quad_t      *m_quadpt;
	std::unique_ptr<quad_t *[]> m_quadind;

	uint16_t  m_v60_copro_ram_adr = 0;
	uint16_t  m_v60_copro_ram_latch[2]{};
	std::unique_ptr<uint32_t[]> m_copro_ram_data;
	uint16_t  m_listctl[2]{};
	uint16_t  *m_glist = nullptr;
	bool    m_render_done = false;

	std::unique_ptr<uint16_t[]> m_tgp_ram;
	std::unique_ptr<uint32_t[]> m_poly_ram;

	// Rendering helper functions
	uint32_t  readi(int adr) const;
	int16_t   readi16(int adr) const;
	float   readf(int adr) const;
	void    cross_product(point_t* o, const point_t* p, const point_t* q) const;
	float   view_determinant(const point_t *p1, const point_t *p2, const point_t *p3) const;

	static bool fclip_isc_bottom(view_t*, point_t*);
	static bool fclip_isc_top(view_t*, point_t*);
	static bool fclip_isc_left(view_t*, point_t*);
	static bool fclip_isc_right(view_t*, point_t*);
	static void fclip_clip_bottom(view_t*, point_t*, point_t*, point_t*);
	static void fclip_clip_top(view_t*, point_t*, point_t*, point_t*);
	static void fclip_clip_left(view_t*, point_t*, point_t*, point_t*);
	static void fclip_clip_right(view_t*, point_t*, point_t*, point_t*);

	// Rendering
	void    tgp_render(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void    tgp_scan();

	void        sort_quads() const;
	void        unsort_quads() const;
	void        draw_quads(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	static void recompute_frustum(view_t *view);
	static void draw_hline(bitmap_rgb32 &bitmap, int x1, int x2, int y, int color);
	static void draw_hline_moired(bitmap_rgb32 &bitmap, int x1, int x2, int y, int color);
	static void fill_slope(bitmap_rgb32 &bitmap, view_t *view, int color, int32_t x1, int32_t x2, int32_t sl1, int32_t sl2, int32_t y1, int32_t y2, int32_t *nx1, int32_t *nx2);
	static void fill_line(bitmap_rgb32 &bitmap, view_t *view, int color, int32_t y, int32_t x1, int32_t x2);
	void        fill_quad(bitmap_rgb32 &bitmap, view_t *view, const quad_t& q) const;

	void    fclip_push_quad_next(int level, quad_t& q, point_t *p1, point_t *p2, point_t *p3, point_t *p4);
	void    fclip_push_quad(int level, quad_t& q);

	static float    min4f(float a, float b, float c, float d);
	static float    max4f(float a, float b, float c, float d);
	static float    compute_specular(glm::vec3& normal, glm::vec3& light, float diffuse,int lmode);

	int push_direct(int list_offset);
	int draw_direct(bitmap_rgb32 &bitmap, const rectangle &cliprect, int list_offset);
	int skip_direct(int list_offset) const;
	void    push_object(uint32_t tex_adr, uint32_t poly_adr, uint32_t size);
	void    draw_objects(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void set_current_render_list();
	int     get_list_number();
	void    end_frame();

	clipper_t m_clipfn[4];

	// run-time rendering
	uint16_t* m_display_list_current = nullptr;

	optional_shared_ptr<uint16_t> m_paletteram16;
	required_device<palette_device> m_palette;
	required_device<segas24_tile_device> m_tiles;

	// I/O related
	output_finder<2> m_digits;
	output_finder<8> m_outs;
	optional_ioport m_throttle;
	u8 dpram_r(offs_t offset);
	void gen_outputs_w(uint8_t data);
	void vf_outputs_w(uint8_t data);
	void vr_outputs_w(uint8_t data);
	void swa_outputs_w(uint8_t data);
	void wingwar_outputs_w(uint8_t data);
	void wingwar360_outputs_w(uint8_t data);
	void netmerc_outputs_w(uint8_t data);
	void drive_board_w(uint8_t data);
};

#endif // MAME_SEGA_MODEL1_H

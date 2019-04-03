// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_INCLUDES_MODEL1_H
#define MAME_INCLUDES_MODEL1_H

#pragma once

#include "audio/dsbz80.h"
#include "audio/segam1audio.h"

#include "cpu/mb86233/mb86233.h"
#include "cpu/v60/v60.h"
#include "machine/i8251.h"
#include "machine/gen_fifo.h"
#include "machine/mb8421.h"
#include "machine/m1comm.h"
#include "machine/timer.h"
#include "video/segaic24.h"

#include "emupal.h"
#include "screen.h"

#include <glm/vec3.hpp>

#include <functional>


#define DECLARE_TGP_FUNCTION(name) void name()

enum {FIFO_SIZE = 256};
enum {MAT_STACK_SIZE = 32};

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
		, m_dsbz80(*this, DSBZ80_TAG)
		, m_tgp_copro(*this, "tgp_copro")
		, m_screen(*this, "screen")
		, m_copro_fifo_in(*this, "copro_fifo_in")
		, m_copro_fifo_out(*this, "copro_fifo_out")
		, m_poly_rom(*this, "polygons")
		, m_copro_tables(*this, "copro_tables")
		, m_copro_data(*this, "copro_data")
		, m_mr2(*this, "mr2")
		, m_mr(*this, "mr")
		, m_display_list0(*this, "display_list0")
		, m_display_list1(*this, "display_list1")
		, m_color_xlat(*this, "color_xlat")
		, m_paletteram16(*this, "palette")
		, m_palette(*this, "palette")
		, m_tiles(*this, "tile")
		, m_digits(*this, "digit%u", 0U)
	{
	}

	void model1(machine_config &config);
	void model1_hle(machine_config &config);

	void vf(machine_config &config);
	void vr(machine_config &config);
	void vformula(machine_config &config);
	void swa(machine_config &config);
	void wingwar(machine_config &config);
	void wingwar360(machine_config &config);
	void netmerc(machine_config &config);

	struct spoint_t
	{
		int32_t x, y;
	};

	struct point_t
	{
		float x, y, z;
		float xx, yy;
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
	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(io_w);

	DECLARE_WRITE16_MEMBER(bank_w);

	TIMER_DEVICE_CALLBACK_MEMBER(model1_interrupt);
	IRQ_CALLBACK_MEMBER(irq_callback);

	// TGP
	DECLARE_READ16_MEMBER(fifoin_status_r);
	DECLARE_WRITE16_MEMBER(md1_w);
	DECLARE_WRITE16_MEMBER(md0_w);
	DECLARE_WRITE16_MEMBER(p_w);
	DECLARE_WRITE16_MEMBER(mr_w);
	DECLARE_WRITE16_MEMBER(mr2_w);

	DECLARE_READ16_MEMBER(v60_copro_fifo_r);
	DECLARE_WRITE16_MEMBER(v60_copro_fifo_w);
	DECLARE_READ16_MEMBER(v60_copro_ram_adr_r);
	DECLARE_WRITE16_MEMBER(v60_copro_ram_adr_w);
	DECLARE_READ16_MEMBER(v60_copro_ram_r);
	DECLARE_WRITE16_MEMBER(v60_copro_ram_w);

	DECLARE_READ32_MEMBER(copro_ram_r);
	DECLARE_WRITE32_MEMBER(copro_ram_w);
	DECLARE_READ32_MEMBER(copro_fifoin_pop);
	DECLARE_WRITE32_MEMBER(copro_fifoout_push);

	DECLARE_WRITE32_MEMBER(copro_sincos_w);
	DECLARE_READ32_MEMBER(copro_sincos_r);
	DECLARE_WRITE32_MEMBER(copro_inv_w);
	DECLARE_READ32_MEMBER(copro_inv_r);
	DECLARE_WRITE32_MEMBER(copro_isqrt_w);
	DECLARE_READ32_MEMBER(copro_isqrt_r);
	DECLARE_WRITE32_MEMBER(copro_atan_w);
	DECLARE_READ32_MEMBER(copro_atan_r);
	DECLARE_WRITE32_MEMBER(copro_data_w);
	DECLARE_READ32_MEMBER(copro_data_r);
	DECLARE_WRITE32_MEMBER(copro_ramadr_w);
	DECLARE_READ32_MEMBER(copro_ramadr_r);
	DECLARE_WRITE32_MEMBER(copro_ramdata_w);
	DECLARE_READ32_MEMBER(copro_ramdata_r);

	void copro_hle_vf();
	void copro_hle_swa();
	void copro_reset();

	u32 m_copro_sincos_base;
	u32 m_copro_inv_base;
	u32 m_copro_isqrt_base;
	u32 m_copro_atan_base[4];
	u32 m_copro_data_base;
	u32 m_copro_ram_adr;

	uint16_t m_r360_state;
	DECLARE_READ8_MEMBER(r360_r);
	DECLARE_WRITE8_MEMBER(r360_w);

	// Rendering
	virtual void video_start() override;
	DECLARE_READ16_MEMBER(model1_listctl_r);
	DECLARE_WRITE16_MEMBER(model1_listctl_w);

	uint32_t screen_update_model1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_model1);

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

	void model1_io(address_map &map);
	void model1_mem(address_map &map);
	void model1_comm_mem(address_map &map);

	void copro_prog_map(address_map &map);
	void copro_data_map(address_map &map);
	void copro_external_map(address_map &map);
	void copro_io_map(address_map &map);
	void copro_rf_map(address_map &map);

	void polhemus_map(address_map &map);

	// Machine
	void irq_raise(int level);
	void irq_init();
	DECLARE_WRITE8_MEMBER(irq_control_w);

	uint8_t m_irq_status;
	int m_last_irq;

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

	required_shared_ptr<uint16_t> m_mr2;
	required_shared_ptr<uint16_t> m_mr;
	required_shared_ptr<uint16_t> m_display_list0;
	required_shared_ptr<uint16_t> m_display_list1;
	required_shared_ptr<uint16_t> m_color_xlat;

	// Sound
	int m_sound_irq;

	// TGP FIFO
	void    fifoout_push(uint32_t data);
	void    fifoout_push_f(float data);
	uint32_t  fifoin_pop();
	float   fifoin_pop_f();
	uint16_t  ram_get_i();
	float   ram_get_f();
	u32 m_v60_copro_fifo_r, m_v60_copro_fifo_w;

	// TGP
	void    tgp_reset();

	DECLARE_TGP_FUNCTION( fadd );
	DECLARE_TGP_FUNCTION( fsub );
	DECLARE_TGP_FUNCTION( fmul );
	DECLARE_TGP_FUNCTION( fdiv );
	DECLARE_TGP_FUNCTION( matrix_push );
	DECLARE_TGP_FUNCTION( matrix_pop );
	DECLARE_TGP_FUNCTION( matrix_write );
	DECLARE_TGP_FUNCTION( clear_stack );
	DECLARE_TGP_FUNCTION( matrix_mul );
	DECLARE_TGP_FUNCTION( anglev );
	DECLARE_TGP_FUNCTION( triangle_normal );
	DECLARE_TGP_FUNCTION( normalize );
	DECLARE_TGP_FUNCTION( acc_seti );
	DECLARE_TGP_FUNCTION( track_select );
	DECLARE_TGP_FUNCTION( load_base );
	DECLARE_TGP_FUNCTION( transpose );
	DECLARE_TGP_FUNCTION( anglep );
	DECLARE_TGP_FUNCTION( matrix_ident );
	DECLARE_TGP_FUNCTION( matrix_read );
	DECLARE_TGP_FUNCTION( matrix_trans );
	DECLARE_TGP_FUNCTION( matrix_scale );
	DECLARE_TGP_FUNCTION( matrix_rotx );
	DECLARE_TGP_FUNCTION( matrix_roty );
	DECLARE_TGP_FUNCTION( matrix_rotz );
	DECLARE_TGP_FUNCTION( track_read_quad );
	DECLARE_TGP_FUNCTION( intercept );
	DECLARE_TGP_FUNCTION( transform_point );
	DECLARE_TGP_FUNCTION( fcos_m1 );
	DECLARE_TGP_FUNCTION( fsin_m1 );
	DECLARE_TGP_FUNCTION( fcosm_m1 );
	DECLARE_TGP_FUNCTION( fsinm_m1 );
	DECLARE_TGP_FUNCTION( distance3 );
	DECLARE_TGP_FUNCTION( ftoi );
	DECLARE_TGP_FUNCTION( itof );
	DECLARE_TGP_FUNCTION( acc_set );
	DECLARE_TGP_FUNCTION( acc_get );
	DECLARE_TGP_FUNCTION( acc_add );
	DECLARE_TGP_FUNCTION( acc_sub );
	DECLARE_TGP_FUNCTION( acc_mul );
	DECLARE_TGP_FUNCTION( acc_div );
	DECLARE_TGP_FUNCTION( f42 );
	DECLARE_TGP_FUNCTION( xyz2rqf );
	DECLARE_TGP_FUNCTION( f43 );
	DECLARE_TGP_FUNCTION( f43_swa );
	DECLARE_TGP_FUNCTION( track_read_tri );
	DECLARE_TGP_FUNCTION( matrix_sdir );
	DECLARE_TGP_FUNCTION( fsqrt );
	DECLARE_TGP_FUNCTION( vlength );
	DECLARE_TGP_FUNCTION( f47 );
	DECLARE_TGP_FUNCTION( track_read_info );
	DECLARE_TGP_FUNCTION( colbox_set );
	DECLARE_TGP_FUNCTION( colbox_test );
	DECLARE_TGP_FUNCTION( f49_swa );
	DECLARE_TGP_FUNCTION( f50_swa );
	DECLARE_TGP_FUNCTION( f52 );
	DECLARE_TGP_FUNCTION( matrix_rdir );
	DECLARE_TGP_FUNCTION( track_lookup );
	DECLARE_TGP_FUNCTION( f56 );
	DECLARE_TGP_FUNCTION( int_normal );
	DECLARE_TGP_FUNCTION( matrix_readt );
	DECLARE_TGP_FUNCTION( acc_geti );
	DECLARE_TGP_FUNCTION( int_point );
	DECLARE_TGP_FUNCTION( col_setcirc );
	DECLARE_TGP_FUNCTION( col_testpt );
	DECLARE_TGP_FUNCTION( push_and_ident );
	DECLARE_TGP_FUNCTION( catmull_rom );
	DECLARE_TGP_FUNCTION( distance );
	DECLARE_TGP_FUNCTION( car_move );
	DECLARE_TGP_FUNCTION( cpa );
	DECLARE_TGP_FUNCTION( vmat_store );
	DECLARE_TGP_FUNCTION( vmat_restore );
	DECLARE_TGP_FUNCTION( vmat_mul );
	DECLARE_TGP_FUNCTION( vmat_read );
	DECLARE_TGP_FUNCTION( matrix_rtrans );
	DECLARE_TGP_FUNCTION( matrix_unrot );
	DECLARE_TGP_FUNCTION( f80 );
	DECLARE_TGP_FUNCTION( vmat_save );
	DECLARE_TGP_FUNCTION( vmat_load );
	DECLARE_TGP_FUNCTION( ram_setadr );
	DECLARE_TGP_FUNCTION( groundbox_test );
	DECLARE_TGP_FUNCTION( f89 );
	DECLARE_TGP_FUNCTION( f92 );
	DECLARE_TGP_FUNCTION( f93 );
	DECLARE_TGP_FUNCTION( f94 );
	DECLARE_TGP_FUNCTION( vmat_flatten );
	DECLARE_TGP_FUNCTION( vmat_load1 );
	DECLARE_TGP_FUNCTION( ram_trans );
	DECLARE_TGP_FUNCTION( f98_load );
	DECLARE_TGP_FUNCTION( f98 );
	DECLARE_TGP_FUNCTION( f99 );
	DECLARE_TGP_FUNCTION( f100 );
	DECLARE_TGP_FUNCTION( groundbox_set );
	DECLARE_TGP_FUNCTION( f102 );
	DECLARE_TGP_FUNCTION( f103 );
	DECLARE_TGP_FUNCTION( dump );
	DECLARE_TGP_FUNCTION( function_get_vf );
	DECLARE_TGP_FUNCTION( function_get_swa );

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
	point_t *m_pointdb;
	point_t *m_pointpt;
	quad_t      *m_quaddb;
	quad_t      *m_quadpt;
	quad_t      **m_quadind;
	offs_t      m_pushpc;
	u32 m_copro_hle_active_list_pos, m_copro_hle_active_list_length;
	typedef void (model1_state::*tgp_func)();

	struct function
	{
		tgp_func cb;
		int count;
	};

	static float tsin(s16 angle);
	static float tcos(s16 angle);

	static const struct function ftab_vf[];
	static const struct function ftab_swa[];
	uint32_t  m_copro_hle_list_length;
	float   m_cmat[12];
	float   m_mat_stack[MAT_STACK_SIZE][12];
	float   m_mat_vector[21][12];
	int32_t   m_mat_stack_pos;
	float   m_acc;
	float   m_tgp_vf_xmin;
	float   m_tgp_vf_xmax;
	float   m_tgp_vf_zmin;
	float   m_tgp_vf_zmax;
	float   m_tgp_vf_ygnd;
	float   m_tgp_vf_yflr;
	float   m_tgp_vf_yjmp;
	float   m_tgp_vr_circx;
	float   m_tgp_vr_circy;
	float   m_tgp_vr_circrad;
	float   m_tgp_vr_cbox[12];
	int     m_tgp_vr_select;

	float   m_tgp_int_px;
	float   m_tgp_int_py;
	float   m_tgp_int_pz;
	uint32_t  m_tgp_int_adr;
	uint16_t  m_v60_copro_ram_adr;
	uint16_t  m_v60_copro_ram_latch[2];
	uint16_t  m_copro_hle_ram_scan_adr;
	std::unique_ptr<uint32_t[]> m_copro_ram_data;
	float   m_tgp_vr_base[4];
	int     m_ccount;
	uint16_t  m_listctl[2];
	uint16_t  *m_glist;
	bool    m_render_done;

	std::unique_ptr<uint16_t[]> m_tgp_ram;
	std::unique_ptr<uint32_t[]> m_poly_ram;

	void configure_fifos();

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
	uint16_t* m_display_list_current;

	optional_shared_ptr<uint16_t> m_paletteram16;
	required_device<palette_device> m_palette;
	required_device<segas24_tile_device> m_tiles;

	// I/O related
	output_finder<2> m_digits;
	DECLARE_READ8_MEMBER(dpram_r);
	DECLARE_WRITE8_MEMBER(vf_outputs_w);
	DECLARE_WRITE8_MEMBER(vr_outputs_w);
	DECLARE_WRITE8_MEMBER(swa_outputs_w);
	DECLARE_WRITE8_MEMBER(wingwar_outputs_w);
	DECLARE_WRITE8_MEMBER(wingwar360_outputs_w);
	DECLARE_WRITE8_MEMBER(netmerc_outputs_w);
	DECLARE_WRITE8_MEMBER(drive_board_w);
};


/*----------- defined in machine/model1.c -----------*/

#endif // MAME_INCLUDES_MODEL1_H

// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "audio/dsbz80.h"
#include "audio/segam1audio.h"
#include "machine/m1comm.h"
#include "cpu/v60/v60.h"

#define DECLARE_TGP_FUNCTION(name) void name()

enum {FIFO_SIZE = 256};
enum {MAT_STACK_SIZE = 32};

class model1_state : public driver_device
{
public:
	model1_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_m1audio(*this, "m1audio"),
		m_m1comm(*this, "m1comm"),
		m_dsbz80(*this, DSBZ80_TAG),
		m_tgp(*this, "tgp"),
		m_screen(*this, "screen"),
		m_mr2(*this, "mr2"),
		m_mr(*this, "mr"),
		m_display_list0(*this, "display_list0"),
		m_display_list1(*this, "display_list1"),
		m_color_xlat(*this, "color_xlat"),
		m_paletteram16(*this, "palette"),
		m_palette(*this, "palette") { }

	required_device<v60_device> m_maincpu;      // V60
	required_device<segam1audio_device> m_m1audio;  // Model 1 standard sound board
	optional_device<m1comm_device> m_m1comm;    // Model 1 communication board
	optional_device<dsbz80_device> m_dsbz80;    // Digital Sound Board
	optional_device<mb86233_cpu_device> m_tgp;
	required_device<screen_device> m_screen;

	required_shared_ptr<UINT16> m_mr2;
	required_shared_ptr<UINT16> m_mr;
	required_shared_ptr<UINT16> m_display_list0;
	required_shared_ptr<UINT16> m_display_list1;
	required_shared_ptr<UINT16> m_color_xlat;

	struct view *m_view;
	struct m1_point *m_pointdb;
	struct m1_point *m_pointpt;
	struct quad_m1 *m_quaddb;
	struct quad_m1 *m_quadpt;
	struct quad_m1 **m_quadind;
	int m_sound_irq;
	UINT8 m_last_snd_cmd;
	int m_snd_cmd_state;
	int m_last_irq;
	int m_dump;
	offs_t m_pushpc;
	int m_fifoin_rpos;
	int m_fifoin_wpos;
	UINT32 m_fifoin_data[FIFO_SIZE];
	int m_swa;
	int m_fifoin_cbcount;
	typedef void (model1_state::*tgp_func)();
	tgp_func m_fifoin_cb;
	struct function
	{
		tgp_func cb;
		int count;
	};
	static const struct function ftab_vf[];
	static const struct function ftab_swa[];
	INT32 m_fifoout_rpos;
	INT32 m_fifoout_wpos;
	UINT32 m_fifoout_data[FIFO_SIZE];
	UINT32 m_list_length;
	float m_cmat[12];
	float m_mat_stack[MAT_STACK_SIZE][12];
	float m_mat_vector[21][12];
	INT32 m_mat_stack_pos;
	float m_acc;
	float m_tgp_vf_xmin;
	float m_tgp_vf_xmax;
	float m_tgp_vf_zmin;
	float m_tgp_vf_zmax;
	float m_tgp_vf_ygnd;
	float m_tgp_vf_yflr;
	float m_tgp_vf_yjmp;
	float m_tgp_vr_circx;
	float m_tgp_vr_circy;
	float m_tgp_vr_circrad;
	float m_tgp_vr_cbox[12];
	int m_tgp_vr_select;
	UINT16 m_ram_adr;
	UINT16 m_ram_latch[2];
	UINT16 m_ram_scanadr;
	std::unique_ptr<UINT32[]> m_ram_data;
	float m_tgp_vr_base[4];
	int m_puuu;
	int m_ccount;
	UINT32 m_copro_r;
	UINT32 m_copro_w;
	int m_copro_fifoout_rpos;
	int m_copro_fifoout_wpos;
	UINT32 m_copro_fifoout_data[FIFO_SIZE];
	int m_copro_fifoout_num;
	int m_copro_fifoin_rpos;
	int m_copro_fifoin_wpos;
	UINT32 m_copro_fifoin_data[FIFO_SIZE];
	int m_copro_fifoin_num;
	UINT32 m_vr_r;
	UINT32 m_vr_w;
	UINT16 m_listctl[2];
	UINT16 *m_glist;
	int m_render_done;
	std::unique_ptr<UINT16[]> m_tgp_ram;
	optional_shared_ptr<UINT16> m_paletteram16;
	required_device<palette_device> m_palette;
	UINT32 *m_poly_rom;
	std::unique_ptr<UINT32[]> m_poly_ram;
	UINT16 m_lamp_state;
	DECLARE_READ16_MEMBER(io_r);
	DECLARE_WRITE16_MEMBER(io_w);
	DECLARE_READ16_MEMBER(fifoin_status_r);
	DECLARE_WRITE16_MEMBER(bank_w);
	DECLARE_READ16_MEMBER(network_ctl_r);
	DECLARE_WRITE16_MEMBER(network_ctl_w);
	DECLARE_WRITE16_MEMBER(md1_w);
	DECLARE_WRITE16_MEMBER(md0_w);
	DECLARE_WRITE16_MEMBER(p_w);
	DECLARE_WRITE16_MEMBER(mr_w);
	DECLARE_WRITE16_MEMBER(mr2_w);
	DECLARE_READ16_MEMBER(snd_68k_ready_r);
	DECLARE_WRITE16_MEMBER(snd_latch_to_68k_w);
	DECLARE_READ16_MEMBER(model1_tgp_copro_r);
	DECLARE_WRITE16_MEMBER(model1_tgp_copro_w);
	DECLARE_READ16_MEMBER(model1_tgp_copro_adr_r);
	DECLARE_WRITE16_MEMBER(model1_tgp_copro_adr_w);
	DECLARE_READ16_MEMBER(model1_tgp_copro_ram_r);
	DECLARE_WRITE16_MEMBER(model1_tgp_copro_ram_w);
	DECLARE_READ16_MEMBER(model1_tgp_vr_adr_r);
	DECLARE_WRITE16_MEMBER(model1_tgp_vr_adr_w);
	DECLARE_READ16_MEMBER(model1_vr_tgp_ram_r);
	DECLARE_WRITE16_MEMBER(model1_vr_tgp_ram_w);
	DECLARE_READ16_MEMBER(model1_vr_tgp_r);
	DECLARE_WRITE16_MEMBER(model1_vr_tgp_w);
	DECLARE_READ32_MEMBER(copro_ram_r);
	DECLARE_WRITE32_MEMBER(copro_ram_w);
	DECLARE_READ16_MEMBER(model1_listctl_r);
	DECLARE_WRITE16_MEMBER(model1_listctl_w);
	DECLARE_MACHINE_START(model1);
	DECLARE_MACHINE_RESET(model1);
	DECLARE_VIDEO_START(model1);
	DECLARE_MACHINE_RESET(model1_vr);
	UINT32 screen_update_model1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_model1(screen_device &screen, bool state);
	TIMER_DEVICE_CALLBACK_MEMBER(model1_interrupt);
	void irq_raise(int level);
	void irq_init();
	IRQ_CALLBACK_MEMBER(irq_callback);
	DECLARE_READ_LINE_MEMBER(copro_fifoin_pop_ok);
	DECLARE_READ32_MEMBER(copro_fifoin_pop);
	DECLARE_WRITE32_MEMBER(copro_fifoout_push);
	void vr_tgp_reset();
	void tgp_reset(int swa);
	void next_fn();
	UINT32 fifoout_pop();
	void fifoout_push(UINT32 data);
	void fifoout_push_f(float data);
	UINT32 fifoin_pop();
	void fifoin_push(UINT32 data);
	float fifoin_pop_f();
	UINT16 ram_get_i();
	float ram_get_f();
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
	DECLARE_TGP_FUNCTION( f11 );
	DECLARE_TGP_FUNCTION( normalize );
	DECLARE_TGP_FUNCTION( acc_seti );
	DECLARE_TGP_FUNCTION( track_select );
	DECLARE_TGP_FUNCTION( f14 );
	DECLARE_TGP_FUNCTION( f15_swa );
	DECLARE_TGP_FUNCTION( anglep );
	DECLARE_TGP_FUNCTION( matrix_ident );
	DECLARE_TGP_FUNCTION( matrix_read );
	DECLARE_TGP_FUNCTION( matrix_trans );
	DECLARE_TGP_FUNCTION( matrix_scale );
	DECLARE_TGP_FUNCTION( matrix_rotx );
	DECLARE_TGP_FUNCTION( matrix_roty );
	DECLARE_TGP_FUNCTION( matrix_rotz );
	DECLARE_TGP_FUNCTION( track_read_quad );
	DECLARE_TGP_FUNCTION( f24_swa );
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
	DECLARE_TGP_FUNCTION( f44 );
	DECLARE_TGP_FUNCTION( matrix_sdir );
	DECLARE_TGP_FUNCTION( f45 );
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
	DECLARE_TGP_FUNCTION( f57 );
	DECLARE_TGP_FUNCTION( matrix_readt );
	DECLARE_TGP_FUNCTION( acc_geti );
	DECLARE_TGP_FUNCTION( f60 );
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
	void copro_fifoin_push(UINT32 data);
	UINT32 copro_fifoout_pop();
	void sort_quads();
	void unsort_quads();
	void draw_quads(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void fclip_push_quad_next(int level, struct quad_m1 *q, struct m1_point *p1, struct m1_point *p2, struct m1_point *p3, struct m1_point *p4);
	void fclip_push_quad(int level, struct quad_m1 *q);
	void push_object(UINT32 tex_adr, UINT32 poly_adr, UINT32 size);
	UINT16 *push_direct(UINT16 *list);
	void draw_objects(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT16 *draw_direct(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT16 *list);
	UINT16 *get_list();
	int get_list_number();
	void end_frame();
	void tgp_render(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void tgp_scan();
};


/*----------- defined in machine/model1.c -----------*/

ADDRESS_MAP_EXTERN( model1_vr_tgp_map, 32 );

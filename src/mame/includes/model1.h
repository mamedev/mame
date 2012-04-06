#include "audio/dsbz80.h"

typedef void (*tgp_func)(running_machine &machine);

enum {FIFO_SIZE = 256};
enum {MAT_STACK_SIZE = 32};

class model1_state : public driver_device
{
public:
	model1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
        m_maincpu(*this, "maincpu"),
        m_audiocpu(*this, "maincpu"),
        m_dsbz80(*this, DSBZ80_TAG)
        { }

    required_device<cpu_device> m_maincpu;      // V60
    required_device<cpu_device> m_audiocpu;     // sound 68000
    optional_device<dsbz80_device> m_dsbz80;    // Digital Sound Board

	struct view *m_view;
	struct point *m_pointdb;
	struct point *m_pointpt;
	struct quad_m1 *m_quaddb;
	struct quad_m1 *m_quadpt;
	struct quad_m1 **m_quadind;
	int m_sound_irq;
	int m_to_68k[8];
	int m_fifo_wptr;
	int m_fifo_rptr;
	int m_last_irq;
	UINT16 *m_mr;
	UINT16 *m_mr2;
	int m_dump;
	UINT16 *m_display_list0;
	UINT16 *m_display_list1;
	UINT16 *m_color_xlat;
	offs_t m_pushpc;
	int m_fifoin_rpos;
	int m_fifoin_wpos;
	UINT32 m_fifoin_data[FIFO_SIZE];
	int m_swa;
	int m_fifoin_cbcount;
	tgp_func m_fifoin_cb;
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
	UINT32 *m_ram_data;
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
	UINT16 *m_tgp_ram;
	UINT16 *m_paletteram16;
	UINT32 *m_poly_rom;
	UINT32 *m_poly_ram;
	DECLARE_READ16_MEMBER(io_r);
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
	DECLARE_READ16_MEMBER(m1_snd_68k_latch_r);
	DECLARE_READ16_MEMBER(m1_snd_v60_ready_r);
	DECLARE_WRITE16_MEMBER(m1_snd_68k_latch1_w);
	DECLARE_WRITE16_MEMBER(m1_snd_68k_latch2_w);
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
};


/*----------- defined in machine/model1.c -----------*/

extern const mb86233_cpu_core model1_vr_tgp_config;



ADDRESS_MAP_EXTERN( model1_vr_tgp_map, 32 );

MACHINE_START( model1 );

void model1_vr_tgp_reset( running_machine &machine );
void model1_tgp_reset(running_machine &machine, int swa);


/*----------- defined in video/model1.c -----------*/

VIDEO_START(model1);
SCREEN_UPDATE_RGB32(model1);
SCREEN_VBLANK(model1);


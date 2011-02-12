typedef void (*tgp_func)(running_machine *machine);

enum {FIFO_SIZE = 256};
enum {MAT_STACK_SIZE = 32};

class model1_state : public driver_device
{
public:
	model1_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	struct view *view;
	struct point *pointdb, *pointpt;
	struct quad_m1 *quaddb, *quadpt;
	struct quad_m1 **quadind;
	int sound_irq;
	int to_68k[8];
	int fifo_wptr;
	int fifo_rptr;
	int last_irq;
	UINT16 *mr;
	UINT16 *mr2;
	int dump;
	UINT16 *display_list0;
	UINT16 *display_list1;
	UINT16 *color_xlat;
	offs_t pushpc;
	int fifoin_rpos;
	int fifoin_wpos;
	UINT32 fifoin_data[FIFO_SIZE];
	int swa;
	int fifoin_cbcount;
	tgp_func fifoin_cb;
	INT32 fifoout_rpos;
	INT32 fifoout_wpos;
	UINT32 fifoout_data[FIFO_SIZE];
	UINT32 list_length;
	float cmat[12];
	float mat_stack[MAT_STACK_SIZE][12];
	float mat_vector[21][12];
	INT32 mat_stack_pos;
	float acc;
	float tgp_vf_xmin;
	float tgp_vf_xmax;
	float tgp_vf_zmin;
	float tgp_vf_zmax;
	float tgp_vf_ygnd;
	float tgp_vf_yflr;
	float tgp_vf_yjmp;
	float tgp_vr_circx;
	float tgp_vr_circy;
	float tgp_vr_circrad;
	float tgp_vr_cbox[12];
	int tgp_vr_select;
	UINT16 ram_adr;
	UINT16 ram_latch[2];
	UINT16 ram_scanadr;
	UINT32 *ram_data;
	float tgp_vr_base[4];
	int puuu;
	int ccount;
	UINT32 copro_r;
	UINT32 copro_w;
	int copro_fifoout_rpos;
	int copro_fifoout_wpos;
	UINT32 copro_fifoout_data[FIFO_SIZE];
	int copro_fifoout_num;
	int copro_fifoin_rpos;
	int copro_fifoin_wpos;
	UINT32 copro_fifoin_data[FIFO_SIZE];
	int copro_fifoin_num;
	UINT32 vr_r;
	UINT32 vr_w;
	UINT16 listctl[2];
	UINT16 *glist;
	int render_done;
	UINT16 *tgp_ram;
	UINT16 *paletteram16;
	UINT32 *poly_rom;
	UINT32 *poly_ram;
};


/*----------- defined in machine/model1.c -----------*/

extern const mb86233_cpu_core model1_vr_tgp_config;

READ16_HANDLER( model1_tgp_copro_r );
WRITE16_HANDLER( model1_tgp_copro_w );
READ16_HANDLER( model1_tgp_copro_adr_r );
WRITE16_HANDLER( model1_tgp_copro_adr_w );
READ16_HANDLER( model1_tgp_copro_ram_r );
WRITE16_HANDLER( model1_tgp_copro_ram_w );

READ16_HANDLER( model1_vr_tgp_r );
WRITE16_HANDLER( model1_vr_tgp_w );
READ16_HANDLER( model1_tgp_vr_adr_r );
WRITE16_HANDLER( model1_tgp_vr_adr_w );
READ16_HANDLER( model1_vr_tgp_ram_r );
WRITE16_HANDLER( model1_vr_tgp_ram_w );

ADDRESS_MAP_EXTERN( model1_vr_tgp_map, 32 );

MACHINE_START( model1 );

void model1_vr_tgp_reset( running_machine *machine );
void model1_tgp_reset(running_machine *machine, int swa);


/*----------- defined in video/model1.c -----------*/

VIDEO_START(model1);
VIDEO_UPDATE(model1);
VIDEO_EOF(model1);

READ16_HANDLER( model1_listctl_r );
WRITE16_HANDLER( model1_listctl_w );

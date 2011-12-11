#include "video/poly.h"

typedef float MATRIX[4][4];
typedef float VECTOR[4];
typedef float VECTOR3[3];

typedef struct {
	float x,y,z,d;
} PLANE;

typedef struct _cached_texture cached_texture;

class model3_state : public driver_device
{
public:
	model3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

    int m_sound_irq_enable;
    emu_timer *m_sound_timer;
	UINT8 m_irq_enable;
	UINT8 m_irq_state;
	UINT8 m_scsi_irq_state;
	UINT64 *m_work_ram;
	int m_crom_bank;
	int m_controls_bank;
	UINT32 m_real3d_device_id;
	UINT16 *m_soundram;
	UINT32 m_mpc105_regs[0x40];
	UINT32 m_mpc105_addr;
	int m_pci_bus;
	int m_pci_device;
	int m_pci_function;
	int m_pci_reg;
	UINT32 m_mpc106_regs[0x40];
	UINT32 m_mpc106_addr;
	UINT32 m_dma_data;
	UINT32 m_dma_status;
	UINT32 m_dma_source;
	UINT32 m_dma_dest;
	UINT32 m_dma_endian;
	UINT32 m_dma_irq;
	UINT64 m_controls_2;
	UINT64 m_controls_3;
	UINT8 m_serial_fifo1;
	UINT8 m_serial_fifo2;
	int m_lightgun_reg_sel;
	int m_adc_channel;
	UINT64 m_real3d_status;
	UINT64 *m_network_ram;
	int m_prot_data_ptr;
	int m_scsp_last_line;
	UINT32 *m_vrom;
	int m_step;
	UINT64 *m_paletteram64;
	int m_m3_step;
	INT32 m_tap_state;
	UINT64 m_ir;
	UINT8 m_id_data[32];
	INT32 m_id_size;
	int m_tdo;
	UINT8 m_layer_enable;
	UINT32 m_layer_modulate_r;
	UINT32 m_layer_modulate_g;
	UINT32 m_layer_modulate_b;
	UINT32 m_layer_modulate1;
	UINT32 m_layer_modulate2;
	UINT64 m_layer_scroll[2];
	UINT64 *m_m3_char_ram;
	UINT64 *m_m3_tile_ram;
	UINT32 *m_texture_fifo;
	int m_texture_fifo_pos;
	UINT16 *m_texture_ram[2];
	UINT32 *m_display_list_ram;
	UINT32 *m_culling_ram;
	UINT32 *m_polygon_ram;
	UINT16 *m_pal_lookup;
	int m_real3d_display_list;
	bitmap_t *m_bitmap3d;
	bitmap_t *m_zbuffer;
	rectangle m_clip3d;
	rectangle *m_screen_clip;
	VECTOR3 m_parallel_light;
	float m_parallel_light_intensity;
	float m_ambient_light_intensity;
	poly_manager *m_poly;
	int m_list_depth;
	int m_tick;
	int m_debug_layer_disable;
	UINT64 m_vid_reg0;
	int m_matrix_stack_ptr;
	MATRIX *m_matrix_stack;
	MATRIX m_coordinate_system;
	float m_viewport_focal_length;
	int m_viewport_region_x;
	int m_viewport_region_y;
	int m_viewport_region_width;
	int m_viewport_region_height;
	PLANE m_clip_plane[5];
	UINT32 m_matrix_base_address;
	cached_texture *m_texcache[2][1024/32][2048/32];

	required_device<cpu_device> m_maincpu;
};


/*----------- defined in drivers/model3.c -----------*/

void model3_set_irq_line(running_machine &machine, UINT8 bit, int state);


/*----------- defined in machine/model3.c -----------*/

void model3_machine_init(running_machine &machine, int step);
int model3_tap_read(running_machine &machine);
void model3_tap_write(running_machine &machine, int tck, int tms, int tdi, int trst);
void model3_tap_reset(running_machine &machine);
READ32_HANDLER(rtc72421_r);
WRITE32_HANDLER(rtc72421_w);


/*----------- defined in video/model3.c -----------*/

READ64_HANDLER(model3_char_r);
WRITE64_HANDLER(model3_char_w);
READ64_HANDLER(model3_tile_r);
WRITE64_HANDLER(model3_tile_w);
READ64_HANDLER(model3_vid_reg_r);
WRITE64_HANDLER(model3_vid_reg_w);
READ64_HANDLER(model3_palette_r);
WRITE64_HANDLER(model3_palette_w);

VIDEO_START(model3);
SCREEN_UPDATE(model3);

WRITE64_HANDLER(real3d_cmd_w);
WRITE64_HANDLER(real3d_display_list_w);
WRITE64_HANDLER(real3d_polygon_ram_w);
void real3d_display_list_end(running_machine &machine);
void real3d_display_list1_dma(address_space *space, UINT32 src, UINT32 dst, int length, int byteswap);
void real3d_display_list2_dma(address_space *space, UINT32 src, UINT32 dst, int length, int byteswap);
void real3d_vrom_texture_dma(address_space *space, UINT32 src, UINT32 dst, int length, int byteswap);
void real3d_texture_fifo_dma(address_space *space, UINT32 src, int length, int byteswap);
void real3d_polygon_ram_dma(address_space *space, UINT32 src, UINT32 dst, int length, int byteswap);

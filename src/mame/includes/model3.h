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
	model3_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 irq_enable;
	UINT8 irq_state;
	UINT8 scsi_irq_state;
	UINT64 *work_ram;
	int crom_bank;
	int controls_bank;
	UINT32 real3d_device_id;
	UINT16 *soundram;
	UINT32 mpc105_regs[0x40];
	UINT32 mpc105_addr;
	int pci_bus;
	int pci_device;
	int pci_function;
	int pci_reg;
	UINT32 mpc106_regs[0x40];
	UINT32 mpc106_addr;
	UINT32 dma_data;
	UINT32 dma_status;
	UINT32 dma_source;
	UINT32 dma_dest;
	UINT32 dma_endian;
	UINT32 dma_irq;
	UINT64 controls_2;
	UINT64 controls_3;
	UINT8 serial_fifo1;
	UINT8 serial_fifo2;
	int lightgun_reg_sel;
	int adc_channel;
	UINT64 real3d_status;
	UINT64 *network_ram;
	int prot_data_ptr;
	int scsp_last_line;
	int vblank;
	UINT32 *vrom;
	int step;
	UINT64 *paletteram64;
	int m3_step;
	INT32 tap_state;
	UINT64 ir;
	UINT8 id_data[32];
	INT32 id_size;
	int tdo;
	UINT8 layer_enable;
	UINT32 layer_modulate_r;
	UINT32 layer_modulate_g;
	UINT32 layer_modulate_b;
	UINT32 layer_modulate1;
	UINT32 layer_modulate2;
	UINT64 layer_scroll[2];
	UINT64 *m3_char_ram;
	UINT64 *m3_tile_ram;
	UINT32 *texture_fifo;
	int texture_fifo_pos;
	UINT16 *texture_ram[2];
	UINT32 *display_list_ram;
	UINT32 *culling_ram;
	UINT32 *polygon_ram;
	UINT16 *pal_lookup;
	int real3d_display_list;
	bitmap_t *bitmap3d;
	bitmap_t *zbuffer;
	rectangle clip3d;
	rectangle *screen_clip;
	VECTOR3 parallel_light;
	float parallel_light_intensity;
	float ambient_light_intensity;
	poly_manager *poly;
	int list_depth;
	int tick;
	int debug_layer_disable;
	UINT64 vid_reg0;
	int matrix_stack_ptr;
	MATRIX *matrix_stack;
	MATRIX coordinate_system;
	float viewport_focal_length;
	int viewport_region_x;
	int viewport_region_y;
	int viewport_region_width;
	int viewport_region_height;
	PLANE clip_plane[5];
	UINT32 matrix_base_address;
	cached_texture *texcache[2][1024/32][2048/32];
};


/*----------- defined in drivers/model3.c -----------*/

void model3_set_irq_line(running_machine *machine, UINT8 bit, int state);


/*----------- defined in machine/model3.c -----------*/

extern void model3_machine_init(running_machine *machine, int step);
extern int model3_tap_read(running_machine *machine);
extern void model3_tap_write(running_machine *machine, int tck, int tms, int tdi, int trst);
extern void model3_tap_reset(running_machine *machine);
extern READ32_HANDLER(rtc72421_r);
extern WRITE32_HANDLER(rtc72421_w);


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
VIDEO_UPDATE(model3);

WRITE64_HANDLER(real3d_cmd_w);
WRITE64_HANDLER(real3d_display_list_w);
WRITE64_HANDLER(real3d_polygon_ram_w);
void real3d_display_list_end(running_machine *machine);
void real3d_display_list1_dma(address_space *space, UINT32 src, UINT32 dst, int length, int byteswap);
void real3d_display_list2_dma(address_space *space, UINT32 src, UINT32 dst, int length, int byteswap);
void real3d_vrom_texture_dma(address_space *space, UINT32 src, UINT32 dst, int length, int byteswap);
void real3d_texture_fifo_dma(address_space *space, UINT32 src, int length, int byteswap);
void real3d_polygon_ram_dma(address_space *space, UINT32 src, UINT32 dst, int length, int byteswap);

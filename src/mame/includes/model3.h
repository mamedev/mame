// license:BSD-3-Clause
// copyright-holders:R. Belmont, Ville Linde
#include "video/poly.h"
#include "bus/scsi/scsi.h"
#include "machine/53c810.h"
#include "audio/dsbz80.h"
#include "machine/eepromser.h"
#include "sound/scsp.h"
#include "machine/315-5881_crypt.h"

typedef float MATRIX[4][4];
typedef float VECTOR[4];
typedef float VECTOR3[3];

struct cached_texture
{
	cached_texture *next;
	UINT8       width;
	UINT8       height;
	UINT8       format;
	UINT8       alpha;
	rgb_t       data[1];
};

struct m3_vertex
{
	float x;
	float y;
	float z;
	float u;
	float v;
	float nx;
	float ny;
	float nz;
};

typedef frustum_clip_vertex<float, 4> m3_clip_vertex;

struct m3_triangle
{
	m3_clip_vertex v[3];

	cached_texture *texture;
	int param;
	int transparency;
	int color;
};

class model3_renderer;

class model3_state : public driver_device
{
public:
	model3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_lsi53c810(*this, "lsi53c810"),
		m_audiocpu(*this, "audiocpu"),
		m_scsp1(*this, "scsp1"),
		m_eeprom(*this, "eeprom"),
		m_screen(*this, "screen"),
		m_work_ram(*this, "work_ram"),
		m_paletteram64(*this, "paletteram64"),
		m_dsbz80(*this, DSBZ80_TAG),
		m_soundram(*this, "soundram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_cryptdevice(*this, "315_5881")
	{
		m_step15_with_mpc106 = false;
		m_step20_with_old_real3d = false;
	}

	required_device<cpu_device> m_maincpu;
	optional_device<lsi53c810_device> m_lsi53c810;
	required_device<cpu_device> m_audiocpu;
	required_device<scsp_device> m_scsp1;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<screen_device> m_screen;

	required_shared_ptr<UINT64> m_work_ram;
	required_shared_ptr<UINT64> m_paletteram64;
	optional_device<dsbz80_device> m_dsbz80;    // Z80-based MPEG Digital Sound Board
	required_shared_ptr<UINT16> m_soundram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<sega_315_5881_crypt_device> m_cryptdevice;

	tilemap_t *m_layer4[4];
	tilemap_t *m_layer8[4];

	int m_sound_irq_enable;
	emu_timer *m_sound_timer;
	emu_timer *m_real3d_dma_timer;
	UINT8 m_irq_enable;
	UINT8 m_irq_state;
	UINT8 m_scsi_irq_state;
	int m_crom_bank;
	int m_controls_bank;
	bool m_step15_with_mpc106;
	bool m_step20_with_old_real3d;
	UINT32 m_real3d_device_id;
	int m_pci_bus;
	int m_pci_device;
	int m_pci_function;
	int m_pci_reg;
	UINT32 m_mpc105_regs[0x40];
	UINT32 m_mpc105_addr;
	UINT32 m_mpc106_regs[0x40];
	UINT32 m_mpc106_addr;
	UINT32 m_dma_data;
	UINT32 m_dma_status;
	UINT32 m_dma_source;
	UINT32 m_dma_dest;
	UINT32 m_dma_endian;
	UINT32 m_dma_irq;
	UINT32 m_dma_busy;
	UINT64 m_controls_2;
	UINT64 m_controls_3;
	UINT8 m_serial_fifo1;
	UINT8 m_serial_fifo2;
	int m_lightgun_reg_sel;
	int m_adc_channel;
	UINT64 m_real3d_status;
	UINT64 *m_network_ram;
	int m_prot_data_ptr;
	UINT32 *m_vrom;
	int m_step;
	int m_m3_step;
	INT32 m_tap_state;
	UINT64 m_ir;
	UINT8 m_id_data[32];
	INT32 m_id_size;
	int m_tdo;
	UINT16 m_layer_priority;
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
	int m_real3d_display_list;
	rectangle m_clip3d;
	rectangle *m_screen_clip;
	VECTOR3 m_parallel_light;
	float m_parallel_light_intensity;
	float m_ambient_light_intensity;
	UINT64 m_vid_reg0;
	int m_matrix_stack_ptr;
	int m_list_depth;
	MATRIX *m_matrix_stack;
	MATRIX m_coordinate_system;
	MATRIX m_projection_matrix;
	float m_viewport_x;
	float m_viewport_y;
	float m_viewport_width;
	float m_viewport_height;
	float m_viewport_near;
	float m_viewport_far;
	UINT32 m_matrix_base_address;
	cached_texture *m_texcache[2][1024/32][2048/32];

	model3_renderer *m_renderer;
	m3_triangle* m_tri_buffer;
	m3_triangle* m_tri_alpha_buffer;
	int m_tri_buffer_ptr;
	int m_tri_alpha_buffer_ptr;
	int m_viewport_tri_index[4];
	int m_viewport_tri_alpha_index[4];

	DECLARE_READ32_MEMBER(rtc72421_r);
	DECLARE_WRITE32_MEMBER(rtc72421_w);
	DECLARE_READ64_MEMBER(model3_char_r);
	DECLARE_WRITE64_MEMBER(model3_char_w);
	DECLARE_READ64_MEMBER(model3_tile_r);
	DECLARE_WRITE64_MEMBER(model3_tile_w);
	DECLARE_READ64_MEMBER(model3_vid_reg_r);
	DECLARE_WRITE64_MEMBER(model3_vid_reg_w);
	DECLARE_WRITE64_MEMBER(model3_palette_w);
	DECLARE_READ64_MEMBER(model3_palette_r);
	DECLARE_WRITE64_MEMBER(real3d_display_list_w);
	DECLARE_WRITE64_MEMBER(real3d_polygon_ram_w);
	DECLARE_WRITE64_MEMBER(real3d_cmd_w);
	DECLARE_READ64_MEMBER(mpc105_addr_r);
	DECLARE_WRITE64_MEMBER(mpc105_addr_w);
	DECLARE_READ64_MEMBER(mpc105_data_r);
	DECLARE_WRITE64_MEMBER(mpc105_data_w);
	DECLARE_READ64_MEMBER(mpc105_reg_r);
	DECLARE_WRITE64_MEMBER(mpc105_reg_w);
	DECLARE_READ64_MEMBER(mpc106_addr_r);
	DECLARE_WRITE64_MEMBER(mpc106_addr_w);
	DECLARE_READ64_MEMBER(mpc106_data_r);
	DECLARE_WRITE64_MEMBER(mpc106_data_w);
	DECLARE_READ64_MEMBER(mpc106_reg_r);
	DECLARE_WRITE64_MEMBER(mpc106_reg_w);
	DECLARE_READ64_MEMBER(scsi_r);
	DECLARE_WRITE64_MEMBER(scsi_w);
	DECLARE_READ64_MEMBER(real3d_dma_r);
	DECLARE_WRITE64_MEMBER(real3d_dma_w);
	DECLARE_READ64_MEMBER(model3_ctrl_r);
	DECLARE_WRITE64_MEMBER(model3_ctrl_w);
	DECLARE_READ64_MEMBER(model3_sys_r);
	DECLARE_WRITE64_MEMBER(model3_sys_w);
	DECLARE_READ64_MEMBER(model3_rtc_r);
	DECLARE_WRITE64_MEMBER(model3_rtc_w);
	DECLARE_READ64_MEMBER(real3d_status_r);
	DECLARE_READ8_MEMBER(model3_sound_r);
	DECLARE_WRITE8_MEMBER(model3_sound_w);
	DECLARE_READ64_MEMBER(network_r);
	DECLARE_WRITE64_MEMBER(network_w);

	DECLARE_WRITE64_MEMBER(daytona2_rombank_w);
	DECLARE_WRITE16_MEMBER(model3snd_ctrl);
	UINT32 pci_device_get_reg();
	void pci_device_set_reg(UINT32 value);
	DECLARE_DRIVER_INIT(genprot);
	DECLARE_DRIVER_INIT(lemans24);
	DECLARE_DRIVER_INIT(vs298);
	DECLARE_DRIVER_INIT(vs299);
	DECLARE_DRIVER_INIT(swtrilgy);
	DECLARE_DRIVER_INIT(scudplus);
	DECLARE_DRIVER_INIT(model3_20);
	DECLARE_DRIVER_INIT(bass);
	DECLARE_DRIVER_INIT(vs2);
	DECLARE_DRIVER_INIT(daytona2);
	DECLARE_DRIVER_INIT(eca);
	DECLARE_DRIVER_INIT(srally2);
	DECLARE_DRIVER_INIT(harleya);
	DECLARE_DRIVER_INIT(skichamp);
	DECLARE_DRIVER_INIT(spikeofe);
	DECLARE_DRIVER_INIT(scud);
	DECLARE_DRIVER_INIT(harley);
	DECLARE_DRIVER_INIT(swtrilga);
	DECLARE_DRIVER_INIT(vs29815);
	DECLARE_DRIVER_INIT(model3_10);
	DECLARE_DRIVER_INIT(vs215);
	DECLARE_DRIVER_INIT(getbass);
	DECLARE_DRIVER_INIT(scudplusa);
	DECLARE_DRIVER_INIT(dirtdvls);
	DECLARE_DRIVER_INIT(vf3);
	DECLARE_DRIVER_INIT(von2);
	DECLARE_DRIVER_INIT(lostwsga);
	DECLARE_DRIVER_INIT(oceanhun);
	DECLARE_DRIVER_INIT(dayto2pe);
	DECLARE_DRIVER_INIT(spikeout);
	DECLARE_DRIVER_INIT(magtruck);
	DECLARE_DRIVER_INIT(lamachin);
	DECLARE_DRIVER_INIT(model3_15);
	DECLARE_MACHINE_START(model3_10);
	DECLARE_MACHINE_RESET(model3_10);
	DECLARE_MACHINE_START(model3_15);
	DECLARE_MACHINE_RESET(model3_15);
	DECLARE_MACHINE_START(model3_20);
	DECLARE_MACHINE_RESET(model3_20);
	DECLARE_MACHINE_START(model3_21);
	DECLARE_MACHINE_RESET(model3_21);
	TIMER_CALLBACK_MEMBER(model3_sound_timer_tick);
	TIMER_CALLBACK_MEMBER(real3d_dma_timer_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(model3_interrupt);
	void model3_exit();
	DECLARE_WRITE8_MEMBER(scsp_irq);
	LSI53C810_DMA_CB(real3d_dma_callback);
	LSI53C810_FETCH_CB(scsi_fetch);
	LSI53C810_IRQ_CB(scsi_irq_callback);
	void update_irq_state();
	void set_irq_line(UINT8 bit, int line);
	void model3_init(int step);
	// video
	virtual void video_start() override;
	UINT32 screen_update_model3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(tile_info_layer0_4bit);
	TILE_GET_INFO_MEMBER(tile_info_layer1_4bit);
	TILE_GET_INFO_MEMBER(tile_info_layer2_4bit);
	TILE_GET_INFO_MEMBER(tile_info_layer3_4bit);
	TILE_GET_INFO_MEMBER(tile_info_layer0_8bit);
	TILE_GET_INFO_MEMBER(tile_info_layer1_8bit);
	TILE_GET_INFO_MEMBER(tile_info_layer2_8bit);
	TILE_GET_INFO_MEMBER(tile_info_layer3_8bit);
	void reset_triangle_buffers();
	m3_triangle* push_triangle(bool alpha);
	void draw_layers(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_layer(bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, int sx, int sy, int prio);
	void draw_3d_layer(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void invalidate_texture(int page, int texx, int texy, int texwidth, int texheight);
	cached_texture *get_texture(int page, int texx, int texy, int texwidth, int texheight, int format);
	inline void write_texture16(int xpos, int ypos, int width, int height, int page, UINT16 *data);
	inline void write_texture8(int xpos, int ypos, int width, int height, int page, int upper, int lower, UINT16 *data);
	void real3d_upload_texture(UINT32 header, UINT32 *data);
	void init_matrix_stack();
	void get_top_matrix(MATRIX *out);
	void push_matrix_stack();
	void pop_matrix_stack();
	void multiply_matrix_stack(MATRIX matrix);
	void translate_matrix_stack(float x, float y, float z);
	void draw_model(UINT32 addr);
	UINT32 *get_memory_pointer(UINT32 address);
	void set_projection(float left, float right, float top, float bottom, float near, float far);
	void load_matrix(int matrix_num, MATRIX *out);
	void traverse_list4(int lod_num, UINT32 address);
	void traverse_list(UINT32 address);
	inline void process_link(UINT32 address, UINT32 link);
	void draw_block(UINT32 address);
	void draw_viewport(int pri, UINT32 address);
	void real3d_traverse_display_list();
	void real3d_display_list_end();
	void real3d_display_list1_dma(UINT32 src, UINT32 dst, int length, int byteswap);
	void real3d_display_list2_dma(UINT32 src, UINT32 dst, int length, int byteswap);
	void real3d_vrom_texture_dma(UINT32 src, UINT32 dst, int length, int byteswap);
	void real3d_texture_fifo_dma(UINT32 src, int length, int byteswap);
	void real3d_polygon_ram_dma(UINT32 src, UINT32 dst, int length, int byteswap);
	// machine
	void insert_id(UINT32 id, INT32 start_bit);
	int tap_read();
	void tap_write(int tck, int tms, int tdi, int trst);
	void tap_reset();
	void tap_set_asic_ids();

	DECLARE_READ64_MEMBER(model3_5881prot_r);
	DECLARE_WRITE64_MEMBER(model3_5881prot_w);
	int first_read;
	UINT16 crypt_read_callback(UINT32 addr);

};

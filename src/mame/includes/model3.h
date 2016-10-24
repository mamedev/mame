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
	uint8_t       width;
	uint8_t       height;
	uint8_t       format;
	uint8_t       alpha;
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
		m_adc_ports(*this, {"AN0", "AN1", "AN2", "AN3", "AN4", "AN5", "AN6", "AN7"}),
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

	optional_ioport_array<8> m_adc_ports;

	required_shared_ptr<uint64_t> m_work_ram;
	required_shared_ptr<uint64_t> m_paletteram64;
	optional_device<dsbz80_device> m_dsbz80;    // Z80-based MPEG Digital Sound Board
	required_shared_ptr<uint16_t> m_soundram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<sega_315_5881_crypt_device> m_cryptdevice;

	tilemap_t *m_layer4[4];
	tilemap_t *m_layer8[4];

	int m_sound_irq_enable;
	emu_timer *m_sound_timer;
	emu_timer *m_real3d_dma_timer;
	uint8_t m_irq_enable;
	uint8_t m_irq_state;
	uint8_t m_scsi_irq_state;
	int m_crom_bank;
	int m_controls_bank;
	bool m_step15_with_mpc106;
	bool m_step20_with_old_real3d;
	uint32_t m_real3d_device_id;
	int m_pci_bus;
	int m_pci_device;
	int m_pci_function;
	int m_pci_reg;
	uint32_t m_mpc105_regs[0x40];
	uint32_t m_mpc105_addr;
	uint32_t m_mpc106_regs[0x40];
	uint32_t m_mpc106_addr;
	uint32_t m_dma_data;
	uint32_t m_dma_status;
	uint32_t m_dma_source;
	uint32_t m_dma_dest;
	uint32_t m_dma_endian;
	uint32_t m_dma_irq;
	uint32_t m_dma_busy;
	uint64_t m_controls_2;
	uint64_t m_controls_3;
	uint8_t m_serial_fifo1;
	uint8_t m_serial_fifo2;
	int m_lightgun_reg_sel;
	int m_adc_channel;
	uint64_t m_real3d_status;
	int m_prot_data_ptr;
	std::unique_ptr<uint32_t[]> m_vrom;
	int m_step;
	int m_m3_step;
	int32_t m_tap_state;
	uint64_t m_ir;
	uint8_t m_id_data[32];
	int32_t m_id_size;
	int m_tdo;
	uint16_t m_layer_priority;
	uint32_t m_layer_modulate_r;
	uint32_t m_layer_modulate_g;
	uint32_t m_layer_modulate_b;
	uint32_t m_layer_modulate1;
	uint32_t m_layer_modulate2;
	uint64_t m_layer_scroll[2];
	std::unique_ptr<uint64_t[]> m_m3_char_ram;
	std::unique_ptr<uint64_t[]> m_m3_tile_ram;
	std::unique_ptr<uint32_t[]> m_texture_fifo;
	int m_texture_fifo_pos;
	std::unique_ptr<uint16_t[]> m_texture_ram[2];
	std::unique_ptr<uint32_t[]> m_display_list_ram;
	std::unique_ptr<uint32_t[]> m_culling_ram;
	std::unique_ptr<uint32_t[]> m_polygon_ram;
	int m_real3d_display_list;
	rectangle m_clip3d;
	rectangle *m_screen_clip;
	VECTOR3 m_parallel_light;
	float m_parallel_light_intensity;
	float m_ambient_light_intensity;
	uint64_t m_vid_reg0;
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
	uint32_t m_matrix_base_address;
	cached_texture *m_texcache[2][1024/32][2048/32];

	model3_renderer *m_renderer;
	m3_triangle* m_tri_buffer;
	m3_triangle* m_tri_alpha_buffer;
	int m_tri_buffer_ptr;
	int m_tri_alpha_buffer_ptr;
	int m_viewport_tri_index[4];
	int m_viewport_tri_alpha_index[4];

	uint32_t rtc72421_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void rtc72421_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint64_t model3_char_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void model3_char_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t model3_tile_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void model3_tile_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t model3_vid_reg_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void model3_vid_reg_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	void model3_palette_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t model3_palette_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void real3d_display_list_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	void real3d_polygon_ram_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	void real3d_cmd_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t mpc105_addr_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void mpc105_addr_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t mpc105_data_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void mpc105_data_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t mpc105_reg_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void mpc105_reg_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t mpc106_addr_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void mpc106_addr_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t mpc106_data_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void mpc106_data_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t mpc106_reg_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void mpc106_reg_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t scsi_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void scsi_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t real3d_dma_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void real3d_dma_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t model3_ctrl_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void model3_ctrl_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t model3_sys_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void model3_sys_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t model3_rtc_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void model3_rtc_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t real3d_status_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint8_t model3_sound_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void model3_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void daytona2_rombank_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	void model3snd_ctrl(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t pci_device_get_reg();
	void pci_device_set_reg(uint32_t value);
	void init_genprot();
	void init_lemans24();
	void init_vs298();
	void init_vs299();
	void init_swtrilgy();
	void init_scudplus();
	void init_model3_20();
	void init_bass();
	void init_vs2();
	void init_daytona2();
	void init_eca();
	void init_srally2();
	void init_harleya();
	void init_skichamp();
	void init_spikeofe();
	void init_scud();
	void init_harley();
	void init_swtrilga();
	void init_vs29815();
	void init_model3_10();
	void init_vs215();
	void init_getbass();
	void init_scudplusa();
	void init_dirtdvls();
	void init_vf3();
	void init_von2();
	void init_lostwsga();
	void init_oceanhun();
	void init_dayto2pe();
	void init_spikeout();
	void init_magtruck();
	void init_lamachin();
	void init_model3_15();
	void machine_start_model3_10();
	void machine_reset_model3_10();
	void machine_start_model3_15();
	void machine_reset_model3_15();
	void machine_start_model3_20();
	void machine_reset_model3_20();
	void machine_start_model3_21();
	void machine_reset_model3_21();
	void model3_sound_timer_tick(void *ptr, int32_t param);
	void real3d_dma_timer_callback(void *ptr, int32_t param);
	void model3_interrupt(timer_device &timer, void *ptr, int32_t param);
	void model3_exit();
	void scsp_irq(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	LSI53C810_DMA_CB(real3d_dma_callback);
	LSI53C810_FETCH_CB(scsi_fetch);
	LSI53C810_IRQ_CB(scsi_irq_callback);
	void update_irq_state();
	void set_irq_line(uint8_t bit, int line);
	void model3_init(int step);
	// video
	virtual void video_start() override;
	uint32_t screen_update_model3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void tile_info_layer0_4bit(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tile_info_layer1_4bit(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tile_info_layer2_4bit(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tile_info_layer3_4bit(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tile_info_layer0_8bit(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tile_info_layer1_8bit(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tile_info_layer2_8bit(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tile_info_layer3_8bit(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void reset_triangle_buffers();
	m3_triangle* push_triangle(bool alpha);
	void draw_layer(bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, int sx, int sy, int prio);
	void invalidate_texture(int page, int texx, int texy, int texwidth, int texheight);
	cached_texture *get_texture(int page, int texx, int texy, int texwidth, int texheight, int format);
	inline void write_texture16(int xpos, int ypos, int width, int height, int page, uint16_t *data);
	inline void write_texture8(int xpos, int ypos, int width, int height, int page, int upper, int lower, uint16_t *data);
	void real3d_upload_texture(uint32_t header, uint32_t *data);
	void init_matrix_stack();
	void get_top_matrix(MATRIX *out);
	void push_matrix_stack();
	void pop_matrix_stack();
	void multiply_matrix_stack(MATRIX matrix);
	void translate_matrix_stack(float x, float y, float z);
	void draw_model(uint32_t addr);
	uint32_t *get_memory_pointer(uint32_t address);
	void set_projection(float left, float right, float top, float bottom, float near, float far);
	void load_matrix(int matrix_num, MATRIX *out);
	void traverse_list4(int lod_num, uint32_t address);
	void traverse_list(uint32_t address);
	inline void process_link(uint32_t address, uint32_t link);
	void draw_block(uint32_t address);
	void draw_viewport(int pri, uint32_t address);
	void real3d_traverse_display_list();
	void real3d_display_list_end();
	void real3d_display_list1_dma(uint32_t src, uint32_t dst, int length, int byteswap);
	void real3d_display_list2_dma(uint32_t src, uint32_t dst, int length, int byteswap);
	void real3d_vrom_texture_dma(uint32_t src, uint32_t dst, int length, int byteswap);
	void real3d_texture_fifo_dma(uint32_t src, int length, int byteswap);
	void real3d_polygon_ram_dma(uint32_t src, uint32_t dst, int length, int byteswap);
	// machine
	void insert_id(uint32_t id, int32_t start_bit);
	int tap_read();
	void tap_write(int tck, int tms, int tdi, int trst);
	void tap_reset();
	void tap_set_asic_ids();

	uint64_t model3_5881prot_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void model3_5881prot_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	int first_read;
	uint16_t crypt_read_callback(uint32_t addr);

};

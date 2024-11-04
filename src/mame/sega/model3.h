// license:BSD-3-Clause
// copyright-holders:R. Belmont, Ville Linde
#ifndef MAME_SEGA_MODEL3_H
#define MAME_SEGA_MODEL3_H

#pragma once

#include "cpu/powerpc/ppc.h"
#include "video/poly.h"
#include "bus/scsi/scsi.h"
#include "machine/53c810.h"
#include "dsbz80.h"
#include "machine/eepromser.h"
#include "machine/i8251.h"
#include "sound/scsp.h"
#include "315_5649.h"
#include "315-5881_crypt.h"
#include "segabill.h"
#include "machine/msm6242.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

typedef float MATRIX[4][4];
typedef float VECTOR[4];
typedef float VECTOR3[3];

struct cached_texture
{
	cached_texture(int texwidth, int texheight, int texformat) :
		next(nullptr),
		width(texwidth),
		height(texheight),
		format(texformat),
		alpha(~0),
		data(new rgb_t[(32 << texwidth) * (32 << texheight) * 4]) { }
	cached_texture *next;
	uint8_t       width;
	uint8_t       height;
	uint8_t       format;
	uint8_t       alpha;
	std::unique_ptr<rgb_t[]> data;
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

struct model3_polydata
{
	cached_texture *texture;
	uint32_t color;
	uint32_t texture_param;
	int transparency;
	int intensity;
};

class model3_state;

class model3_renderer : public poly_manager<float, model3_polydata, 6>
{
public:
	model3_renderer(running_machine &machine, int width, int height)
		: poly_manager<float, model3_polydata, 6>(machine)
	{
		m_fb = std::make_unique<bitmap_rgb32>(width, height);
		m_zb = std::make_unique<bitmap_ind32>(width, height);
	}

	void draw(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_opaque_triangles(const m3_triangle* tris, int num_tris);
	void draw_alpha_triangles(const m3_triangle* tris, int num_tris);
	void clear_fb();
	void clear_zb();
	void draw_scanline_solid(int32_t scanline, const extent_t &extent, const model3_polydata &extradata, int threadid);
	void draw_scanline_solid_trans(int32_t scanline, const extent_t &extent, const model3_polydata &extradata, int threadid);
	void draw_scanline_tex(int32_t scanline, const extent_t &extent, const model3_polydata &extradata, int threadid);
	void draw_scanline_tex_colormod(int32_t scanline, const extent_t &extent, const model3_polydata &extradata, int threadid);
	void draw_scanline_tex_contour(int32_t scanline, const extent_t &extent, const model3_polydata &extradata, int threadid);
	void draw_scanline_tex_trans(int32_t scanline, const extent_t &extent, const model3_polydata &extradata, int threadid);
	void draw_scanline_tex_alpha(int32_t scanline, const extent_t &extent, const model3_polydata &extradata, int threadid);
	void wait_for_polys();

private:
	std::unique_ptr<bitmap_rgb32> m_fb;
	std::unique_ptr<bitmap_ind32> m_zb;
};

class model3_state : public driver_device
{
public:
	model3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_lsi53c810(*this, "lsi53c810"),
		m_audiocpu(*this, "audiocpu"),
		m_scsp1(*this, "scsp1"),
		m_eeprom(*this, "eeprom"),
		m_screen(*this, "screen"),
		m_rtc(*this, "rtc"),
		m_io(*this, "io"),
		m_work_ram(*this, "work_ram"),
		m_bank_crom(*this, "bank_crom"),
		m_paletteram64(*this, "paletteram64"),
		m_dsbz80(*this, "dsbz80"),
		m_uart(*this, "uart"),
		m_soundram(*this, "soundram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_cryptdevice(*this, "315_5881"),
		m_billboard(*this, "billboard"),
		m_bank2(*this, "bank2")
	{
		m_step15_with_mpc106 = false;
		m_step20_with_old_real3d = false;
	}

	void add_cpu_66mhz(machine_config &config);
	void add_cpu_100mhz(machine_config &config);
	void add_cpu_166mhz(machine_config &config);

	void add_base_devices(machine_config &config);
	void add_scsi_devices(machine_config &config);
	void add_crypt_devices(machine_config &config);

	void model3_21_5881(machine_config &config);
	void model3_20_5881(machine_config &config);
	void model3_15(machine_config &config);
	void model3_10(machine_config &config);
	void model3_20(machine_config &config);
	void model3_21(machine_config &config);

	void getbass(machine_config &config);
	void scud(machine_config &config);
	void lostwsga(machine_config &config);

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
	void init_swtrilgp();
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

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<ppc_device> m_maincpu;
	optional_device<lsi53c810_device> m_lsi53c810;
	required_device<cpu_device> m_audiocpu;
	required_device<scsp_device> m_scsp1;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<screen_device> m_screen;
	required_device<rtc72421_device> m_rtc;
	required_device<sega_315_5649_device> m_io;

	required_shared_ptr<uint64_t> m_work_ram;
	memory_bank_creator m_bank_crom;
	required_shared_ptr<uint64_t> m_paletteram64;
	optional_device<dsbz80_device> m_dsbz80;    // Z80-based MPEG Digital Sound Board
	optional_device<i8251_device> m_uart;
	required_shared_ptr<uint16_t> m_soundram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<sega_315_5881_crypt_device> m_cryptdevice;

	required_device<sega_billboard_device> m_billboard;
	memory_bank_creator m_bank2;

	tilemap_t *m_layer4[4]{};
	tilemap_t *m_layer8[4]{};

	int m_sound_irq_enable = 0;
	emu_timer *m_sound_timer = nullptr;
	emu_timer *m_real3d_dma_timer = nullptr;
	emu_timer *m_scan_timer = nullptr;
	uint8_t m_irq_enable = 0;
	uint8_t m_irq_state = 0;
	uint8_t m_scsi_irq_state = 0;
	int m_crom_bank = 0;
	int m_controls_bank = 0;
	bool m_step15_with_mpc106 = false;
	bool m_step20_with_old_real3d = false;
	uint32_t m_real3d_device_id = 0;
	int m_pci_bus = 0;
	int m_pci_device = 0;
	int m_pci_function = 0;
	int m_pci_reg = 0;
	uint32_t m_mpc105_regs[0x40]{};
	uint32_t m_mpc105_addr = 0;
	uint32_t m_mpc106_regs[0x40]{};
	uint32_t m_mpc106_addr = 0;
	uint32_t m_dma_data = 0;
	uint32_t m_dma_status = 0;
	uint32_t m_dma_source = 0;
	uint32_t m_dma_dest = 0;
	uint32_t m_dma_endian = 0;
	uint32_t m_dma_irq = 0;
	uint32_t m_dma_busy = 0;
	uint64_t m_controls_2 = 0;
	uint64_t m_controls_3 = 0;
	uint8_t m_serial_fifo1 = 0;
	uint8_t m_serial_fifo2 = 0;
	int m_lightgun_reg_sel = 0;
	int m_adc_channel = 0;
	uint64_t m_real3d_status = 0;
	int m_prot_data_ptr = 0;
	std::unique_ptr<uint32_t[]> m_vrom;
	int m_step = 0;
	int m_m3_step = 0;
	int32_t m_tap_state = 0;
	uint64_t m_ir = 0;
	uint8_t m_id_data[32]{};
	int32_t m_id_size = 0;
	int m_tdo = 0;
	uint16_t m_layer_priority = 0;
	uint32_t m_layer_modulate_r = 0;
	uint32_t m_layer_modulate_g = 0;
	uint32_t m_layer_modulate_b = 0;
	uint32_t m_layer_modulate1 = 0;
	uint32_t m_layer_modulate2 = 0;
	uint64_t m_layer_scroll[2];
	std::unique_ptr<uint64_t[]> m_m3_char_ram;
	std::unique_ptr<uint64_t[]> m_m3_tile_ram;
	std::unique_ptr<uint32_t[]> m_texture_fifo;
	int m_texture_fifo_pos = 0;
	std::unique_ptr<uint16_t[]> m_texture_ram[2];
	std::unique_ptr<uint32_t[]> m_display_list_ram;
	std::unique_ptr<uint32_t[]> m_culling_ram;
	std::unique_ptr<uint32_t[]> m_polygon_ram;
	int m_real3d_display_list = 0;
	rectangle m_clip3d;
	rectangle *m_screen_clip = nullptr;
	VECTOR3 m_parallel_light;
	float m_parallel_light_intensity = 0;
	float m_ambient_light_intensity = 0;
	uint64_t m_vid_reg0 = 0;
	int m_matrix_stack_ptr = 0;
	int m_list_depth = 0;
	std::unique_ptr<MATRIX[]> m_matrix_stack;
	MATRIX m_coordinate_system;
	MATRIX m_projection_matrix;
	float m_viewport_x = 0;
	float m_viewport_y = 0;
	float m_viewport_width = 0;
	float m_viewport_height = 0;
	float m_viewport_near = 0;
	float m_viewport_far = 0;
	uint32_t m_matrix_base_address = 0;
	cached_texture *m_texcache[2][1024/32][2048/32]{};

	std::unique_ptr<model3_renderer> m_renderer;
	std::unique_ptr<m3_triangle[]> m_tri_buffer;
	std::unique_ptr<m3_triangle[]> m_tri_alpha_buffer;
	int m_tri_buffer_ptr = 0;
	int m_tri_alpha_buffer_ptr = 0;
	int m_viewport_tri_index[4]{};
	int m_viewport_tri_alpha_index[4]{};

	uint32_t rtc72421_r(offs_t offset);
	void rtc72421_w(offs_t offset, uint32_t data);
	uint64_t model3_char_r(offs_t offset);
	void model3_char_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t model3_tile_r(offs_t offset);
	void model3_tile_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t model3_vid_reg_r(offs_t offset);
	void model3_vid_reg_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void model3_palette_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t model3_palette_r(offs_t offset);
	void real3d_display_list_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void real3d_polygon_ram_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void real3d_cmd_w(uint64_t data);
	uint64_t mpc105_addr_r(offs_t offset, uint64_t mem_mask = ~0);
	void mpc105_addr_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t mpc105_data_r();
	void mpc105_data_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t mpc105_reg_r(offs_t offset);
	void mpc105_reg_w(offs_t offset, uint64_t data);
	void mpc105_init();
	uint64_t mpc106_addr_r(offs_t offset, uint64_t mem_mask = ~0);
	void mpc106_addr_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t mpc106_data_r(offs_t offset, uint64_t mem_mask = ~0);
	void mpc106_data_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t mpc106_reg_r(offs_t offset);
	void mpc106_reg_w(offs_t offset, uint64_t data);
	void mpc106_init();
	uint64_t scsi_r(offs_t offset, uint64_t mem_mask = ~0);
	void scsi_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t real3d_dma_r(offs_t offset, uint64_t mem_mask = ~0);
	void real3d_dma_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);

	void eeprom_w(uint8_t data);
	uint8_t input_r();
	void lostwsga_ser1_w(uint8_t data);
	uint8_t lostwsga_ser2_r();
	void lostwsga_ser2_w(uint8_t data);

	uint64_t model3_sys_r(offs_t offset, uint64_t mem_mask = ~0);
	void model3_sys_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t model3_rtc_r(offs_t offset, uint64_t mem_mask = ~0);
	void model3_rtc_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t real3d_status_r(offs_t offset);
	uint8_t model3_sound_r(offs_t offset);
	void model3_sound_w(offs_t offset, uint8_t data);

	void daytona2_rombank_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void model3snd_ctrl(uint16_t data);
	uint32_t pci_device_get_reg();
	void pci_device_set_reg(uint32_t value);
	void configure_fast_ram();
	void interleave_vroms();

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
	TIMER_CALLBACK_MEMBER(model3_scan_timer_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(model3_interrupt);
	void model3_exit();
	void scsp_irq(offs_t offset, uint8_t data);
	void real3d_dma_callback(uint32_t src, uint32_t dst, int length, int byteswap);
	uint32_t scsi_fetch(uint32_t dsp);
	void scsi_irq_callback(int state);
	void update_irq_state();
	void set_irq_line(uint8_t bit, int line);
	void model3_init(int step);
	// video
	uint32_t screen_update_model3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
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

	uint16_t crypt_read_callback(uint32_t addr);

	void model3_5881_mem(address_map &map) ATTR_COLD;
	void model3_10_mem(address_map &map) ATTR_COLD;
	void model3_mem(address_map &map) ATTR_COLD;
	void model3_snd(address_map &map) ATTR_COLD;
	void scsp1_map(address_map &map) ATTR_COLD;
	void scsp2_map(address_map &map) ATTR_COLD;
	void getbass_iocpu_mem(address_map &map) ATTR_COLD;
	void getbass_iocpu_io(address_map &map) ATTR_COLD;
};

#endif // MAME_SEGA_MODEL3_H

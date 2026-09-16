// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Brad Oliver
/******************************************************************************

    Nintendo 2C0x PPU emulation.

    Written by Ernesto Corvi.
    This code is heavily based on Brad Oliver's MESS implementation.

******************************************************************************/

#ifndef MAME_VIDEO_PPU2C0X_H
#define MAME_VIDEO_PPU2C0X_H

#pragma once

//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

// Mirroring types.
#define PPU_MIRROR_NONE       0
#define PPU_MIRROR_VERT       1
#define PPU_MIRROR_HORZ       2
#define PPU_MIRROR_HIGH       3
#define PPU_MIRROR_LOW        4
#define PPU_MIRROR_4SCREEN    5 // Same effect as NONE, but signals that we should never mirror.

#define VISIBLE_SCREEN_WIDTH         (32*8) /* Visible screen width */
#define VISIBLE_SCREEN_HEIGHT        (30*8) /* Visible screen height */

#define NTH_BIT(x, n) (((x) >> (n)) & 1)


//**************************************************************************
//  FORWARD DECLARATIONS
//**************************************************************************

extern bool g_nes_p1_a_pressed_edge;

//class nes_exrom_device;
//class nes_txrom_device;
//class nes_sxrom_device;
//class nes_tengen032_device;
//class nes_sc127_device;
class m6502_device;
//class nes_batmap_srrx_device;


//**************************************************************************
//  ppu2c0x_device
//**************************************************************************

class ppu2c0x_device : public device_t,
						public device_memory_interface,
						public device_video_interface,
						public device_palette_interface
{
public:
	typedef device_delegate<void (int scanline, bool vblank, bool blanked)> scanline_delegate;
	typedef device_delegate<void (int scanline, bool vblank, bool blanked)> hblank_delegate;
	typedef device_delegate<int (int address, int data)> vidaccess_delegate;
	typedef device_delegate<void (offs_t offset)> latch_delegate;
	typedef device_delegate<void (int scanline, unsigned dot, int ppu_tick, uint16_t ppu_address)> ppu_to_mapper_delegate;
	typedef device_delegate<void (uint16_t address, uint64_t ppu_cycle, int ppu_tick, bool odd_frame)> ppu_bus_address_delegate;
	typedef device_delegate<void ()> ppu_odd_frame_skip_delegate;
	typedef device_delegate<void (bool upper_chr, uint16_t ppu_address)> mmc1_ppu_phase_delegate;
	typedef device_delegate<void (uint16_t address)> mmc5_ppu_read_delegate;
	typedef device_delegate<void ()> mmc5_reset_scanline_irq_delegate;
	
	enum
	{
		NTSC_SCANLINES_PER_FRAME     = 262,
		PAL_SCANLINES_PER_FRAME      = 312,
		VS_CLONE_SCANLINES_PER_FRAME = 280,

		BOTTOM_VISIBLE_SCANLINE        = 239,
		VBLANK_FIRST_SCANLINE          = 241,
		VBLANK_FIRST_SCANLINE_PALC     = 291,
		VBLANK_FIRST_SCANLINE_VS_CLONE = 240,
		VBLANK_LAST_SCANLINE_NTSC      = 260,
		VBLANK_LAST_SCANLINE_PAL       = 310,
		VBLANK_LAST_SCANLINE_VS_CLONE  = 279

		// Both the scanline immediately before and immediately after VBLANK
		// are non-rendering and non-vblank.
	};
	
	enum class ppu_bus_source : uint8_t
	{
		PPU,
		CPU_ACCESS
	};

	// CPU-visible PPU register interface.
	virtual uint8_t read(offs_t offset);
	virtual void write(offs_t offset, uint8_t val);
	virtual uint8_t palette_read(offs_t offset);
	virtual void palette_write(offs_t offset, uint8_t data);

	template <typename T>
	void set_cpu_tag(T &&tag)
	{
		m_cpu.set_tag(std::forward<T>(tag));
	}

	auto int_callback()
	{
		return m_int_callback.bind();
	}

	// ---------------------------------------------------------------------
	// Legacy renderer / compatibility surface.
	//
	// These are still declared because derived PPUs and old MAME rendering
	// paths may depend on the virtual API even though the current NES path is
	// driven by the cycle-accurate renderer below.
	// ---------------------------------------------------------------------
	void apply_color_emphasis_and_clamp(bool is_pal_or_dendy, int color_emphasis, double& R, double& G, double& B);
	rgb_t nespal_to_RGB(int color_intensity, int color_num, int color_emphasis, bool is_pal_or_dendy);
	virtual void init_palette_tables();

	virtual void read_tile_plane_data(int address, int color);
	virtual void shift_tile_plane_data(uint8_t &pix);
	virtual void draw_tile_pixel(uint8_t pix, int color, uint32_t back_pen, uint32_t *&dest);
	virtual void draw_tile(uint8_t *line_priority, int color_byte, int color_bits, int address, int start_x, uint32_t back_pen, uint32_t *&dest);
	virtual void draw_background(uint8_t *line_priority);
	virtual void draw_back_pen(uint32_t* dst, int back_pen);
	void draw_background_pen();

	virtual void read_sprite_plane_data(int address);
	virtual void make_sprite_pixel_data(uint8_t &pixel_data, bool flipx);
	virtual void write_to_spriteram_with_increment(uint8_t data);
	void reload_refresh_data();
	virtual void draw_sprite_pixel(int sprite_xpos, int color, int pixel, uint8_t pixel_data, bitmap_rgb32 &bitmap);
	virtual bool is_spritepixel_opaque(int pixel_data, int color);
	virtual void draw_sprite_pixel_low(bitmap_rgb32& bitmap, int pixel_data, int pixel, int sprite_xpos, int color, int sprite_index, uint8_t* line_priority);
	virtual void draw_sprite_pixel_high(bitmap_rgb32& bitmap, int pixel_data, int pixel, int sprite_xpos, int color, int sprite_index, uint8_t* line_priority);
	virtual void read_extra_sprite_bits(int sprite_index);
	virtual int apply_sprite_pattern_page(int index1, int size);
	virtual void draw_sprites(uint8_t *line_priority);

	void render_scanline();
	virtual void scanline_increment_fine_ycounter();
	void update_visible_enabled_scanline();
	void update_visible_disabled_scanline();
	void update_visible_scanline();
	void update_scanline();
	void render(bitmap_rgb32 &bitmap, int flipx, int flipy, int sx, int sy, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	int get_current_scanline() { return scanline; }
	int get_current_dot() { return dot; }

	template <typename... T>
	void set_scanline_callback(T &&... args)
	{
		m_scanline_callback_proc.set(std::forward<T>(args)...);
		m_scanline_callback_proc.resolve();
	}

	template <typename... T>
	void set_hblank_callback(T &&... args)
	{
		m_hblank_callback_proc.set(std::forward<T>(args)...);
		m_hblank_callback_proc.resolve();
	}

	template <typename... T>
	void set_vidaccess_callback(T &&... args)
	{
		m_vidaccess_callback_proc.set(std::forward<T>(args)...);
		m_vidaccess_callback_proc.resolve();
	}

	void set_scanlines_per_frame(int scanlines) { m_scanlines_per_frame = scanlines; }
	uint16_t get_vram_dest();
	void set_vram_dest(uint16_t dest);
	void ppu2c0x(address_map &map);

	// ---------------------------------------------------------------------
	// Cycle-accurate NES PPU path.
	// ---------------------------------------------------------------------
	void run_prerender_scanline_dot();
	void run_scanline_241_dot();
	void run_visible_scanline_dot();
	void run_render_pipeline_dot();
	void run_bg_fetch_dot();
	void reload_bg_shift_registers();
	void do_pixel_output_and_sprite_zero();

	void do_sprite_evaluation();
	bool calc_sprite_tile_addr(uint8_t y, uint8_t index, uint8_t attrib, bool is_high);
	void do_sprite_loading();
	void do_sprite_loading_oam2addr_only();
	unsigned get_sprite_pixel(unsigned &spr_pal, bool &spr_behind_bg, bool &spr_is_s0);

	void copy_vert();
	void copy_horiz();
	void bump_vert();
	void bump_horiz();
	void apply_scroll_ops();

	void do_2007_post_access_bump(ppu_bus_source source);
	void write_oam_data_reg(uint8_t val);
	void write_oam_dma_byte(uint8_t val);

	void set_nmi(bool s);
	void set_mapper(int mapper_number);
	void reset();
	void tick(int x);
	
	bool is_visible_scanline() const { return scanline <= BOTTOM_VISIBLE_SCANLINE; }
	bool is_prerender_scanline() const { return scanline == (m_scanlines_per_frame - 1); }
	bool is_render_scanline() const { return is_visible_scanline() || is_prerender_scanline(); }
	bool is_vblank_start_scanline() const { return scanline == m_vblank_first_scanline; }
	bool is_ntsc_timing() const { return m_scanlines_per_frame == NTSC_SCANLINES_PER_FRAME; }
	
	void retro_fix_previous_pixel_after_ppumask_write();

	template <typename... T>
	void set_latch(T &&... args)
	{
		m_latch.set(std::forward<T>(args)...);
		m_latch.resolve();
	}

	template <typename... T>
	void set_ppu_to_mapper(T &&... args)
	{
		m_ppu_to_mapper.set(std::forward<T>(args)...);
		m_ppu_to_mapper.resolve();
	}
	
	template <typename... T>
	void set_ppu_bus_address(T &&... args)
	{
		m_ppu_bus_address_callback.set(std::forward<T>(args)...);
		m_ppu_bus_address_callback.resolve();
	}
	
	template <typename... T>
	void set_ppu_odd_frame_skip(T &&... args)
	{
		m_ppu_odd_frame_skip.set(std::forward<T>(args)...);
		m_ppu_odd_frame_skip.resolve();
	}
	
	template <typename... T>
	void set_mmc1_ppu_phase(T &&... args)
	{
		m_mmc1_ppu_phase.set(std::forward<T>(args)...);
		m_mmc1_ppu_phase.resolve();
	}

	template <typename... T>
	void set_mmc5_ppu_read(T &&... args) {
		m_mmc5_ppu_read.set(std::forward<T>(args)...);
		m_mmc5_ppu_read.resolve();
	}

	template <typename... T>
	void set_mmc5_reset_scanline_irq(T &&... args) {
		m_mmc5_reset_scanline_irq.set(std::forward<T>(args)...);
		m_mmc5_reset_scanline_irq.resolve();
	}

protected:
	ppu2c0x_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock, address_map_constructor internal_map);
	ppu2c0x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	// ---------------------------------------------------------------------
	// PPU register indexes.
	// ---------------------------------------------------------------------
	enum
	{
		PPU_CONTROL0 = 0,
		PPU_CONTROL1,
		PPU_STATUS,
		PPU_SPRITE_ADDRESS,
		PPU_SPRITE_DATA,
		PPU_SCROLL,
		PPU_ADDRESS,
		PPU_DATA,
		PPU_MAX_REG
	};

	// ---------------------------------------------------------------------
	// PPU register bit definitions.
	// ---------------------------------------------------------------------
	enum
	{
		PPU_CONTROL0_INC         = 0x04,
		PPU_CONTROL0_SPR_SELECT  = 0x08,
		PPU_CONTROL0_CHR_SELECT  = 0x10,
		PPU_CONTROL0_SPRITE_SIZE = 0x20,
		PPU_CONTROL0_NMI         = 0x80,

		PPU_CONTROL1_DISPLAY_MONO   = 0x01,
		PPU_CONTROL1_BACKGROUND_L8  = 0x02,
		PPU_CONTROL1_SPRITES_L8     = 0x04,
		PPU_CONTROL1_BACKGROUND     = 0x08,
		PPU_CONTROL1_SPRITES        = 0x10,
		PPU_CONTROL1_COLOR_EMPHASIS = 0xe0,

		PPU_STATUS_8SPRITES    = 0x20,
		PPU_STATUS_SPRITE0_HIT = 0x40,
		PPU_STATUS_VBLANK      = 0x80
	};

	enum Sprite_size
	{
		EIGHT_BY_EIGHT = 0,
		EIGHT_BY_SIXTEEN
	};

	enum class ppu_fetch_phase : uint8_t
	{
		NONE,
		NT,
		AT,
		PTL,
		PTH,
		SPR_NT,
		SPR_AT,
		SPR_PTL,
		SPR_PTH,
		DUMMY338,
		DUMMY340
	};

	// ---------------------------------------------------------------------
	// Delayed register write/read latches.
	// ---------------------------------------------------------------------
	struct PPUDelayedLatch
	{
		bool has_pending;
		uint8_t value;
		uint16_t value16;
		int apply_dot;
		int apply_scanline;
		int apply_ppu;
	};

	PPUDelayedLatch pending_2000;
	PPUDelayedLatch pending_2001;
	//PPUDelayedLatch pending_2004;
	PPUDelayedLatch pending_2006;

	void apply_delayed_2000(uint8_t val);
	void apply_delayed_2001(uint8_t val);

	// ---------------------------------------------------------------------
	// Device overrides.
	// ---------------------------------------------------------------------
	virtual void device_start() override;
	virtual u32 palette_entries() const noexcept override { return 0x40 * 8; }
	virtual void device_config_complete() override;
	virtual space_config_vector memory_space_config() const override;

	// ---------------------------------------------------------------------
	// Internal memory/bus helpers.
	// ---------------------------------------------------------------------
	uint8_t readbyte(offs_t address);
	inline uint8_t readbyte(uint16_t bus_addr, uint16_t mem_addr);
	inline void writebyte(offs_t address, uint8_t data);

	void ppu_bus_address_drive(uint16_t addr, ppu_bus_source source); 
	void ppu_cart_address_drive(uint16_t addr);
	void ppu_bus_a12_observe(uint16_t addr);
	uint8_t ppu_bus_read(uint16_t addr, ppu_fetch_phase phase);

	inline void clock_bg_shifters_only();
	void do_prerender_oam_sweep();
	void latch_sec_oam_for_2004();

	void init_startup_only_state();
	void init_runtime_reset_state();
	void resolve_mapper_ppu_devices();
	void start_nopalram();

	// ---------------------------------------------------------------------
	// Address space / attached CPU.
	// ---------------------------------------------------------------------
	const address_space_config m_space_config;
	required_device<cpu_device> m_cpu;

	// ---------------------------------------------------------------------
	// Legacy/common PPU configuration and compatibility state.
	// ---------------------------------------------------------------------
	int m_scanlines_per_frame;       // Number of scanlines per frame.
	int m_security_value;            // 2C05 protection.
	int m_vblank_first_scanline;     // First scanline where VBLANK occurs.
	int m_mapper_number;

	uint16_t bg_pat_addr_tile;       // Latched background pattern table base used by BG fetches.
	uint8_t m_planebuf[16];
	int m_scanline;

	// Used by ppu2c0x_vt.cpp / derived legacy paths.
	std::unique_ptr<uint8_t[]> m_spriteram;

	int m_videoram_addr_mask;
	int m_global_refresh_mask;
	int m_line_write_increment_large;
	bool m_paletteram_in_ppuspace;

	std::vector<uint8_t> m_palette_ram;
	bitmap_rgb32 m_bitmap;
	uint16_t m_spriteramsize = 0x100;
	int m_regs[PPU_MAX_REG];

	// Legacy renderer / sh6578-related state.
	int m_tile_page;
	int m_back_color;
	int m_refresh_data;
	int m_x_fine;
	int m_toggle;
	int m_tilecount;

	latch_delegate m_latch;
	ppu_to_mapper_delegate m_ppu_to_mapper;
	ppu_bus_address_delegate m_ppu_bus_address_callback;
	ppu_odd_frame_skip_delegate m_ppu_odd_frame_skip;
	mmc1_ppu_phase_delegate m_mmc1_ppu_phase;
	mmc5_ppu_read_delegate m_mmc5_ppu_read;
	mmc5_reset_scanline_irq_delegate m_mmc5_reset_scanline_irq;

	// ---------------------------------------------------------------------
	// Core PPU scroll/address/register state.
	// ---------------------------------------------------------------------
	unsigned m_prerender_line;

	// VRAM address/scroll registers. 15 bits are meaningful.
	unsigned t;
	unsigned v;
	uint8_t fine_x;
	unsigned ppuctrl_vram_increment;                 // $2000 bit 2.
	uint16_t ppuctrl_sprite_pattern_base;       // $2000 bit 3.
	uint16_t ppuctrl_bg_pattern_base;           // $2000 bit 4.
	Sprite_size ppuctrl_sprite_size;        // $2000 bit 5.
	bool ppuctrl_nmi_enable;             // $2000 bit 7.
	bool nmi_pending;
	int nmi_delay;
	bool suppress_vblank_flag;

	// $2001 decoded state.
	uint8_t ppumask_grayscale_mask;   // 0x30 if grayscale is enabled, otherwise 0x3f.
	bool ppumask_show_bg_left;
	bool ppumask_show_spr_left;
	uint16_t ppumask_emphasis_bits;
	unsigned bg_left_clip;
	unsigned spr_left_clip;

	// $2002 decoded state.
	bool ppustatus_sprite_overflow;
	bool ppustatus_sprite0_hit;
	bool ppustatus_vblank;

	// $2003/$2004/OAM register state.
	uint8_t primary_oam[0x100];
	uint8_t secondary_oam[0x20];

	uint8_t oamaddr;
	unsigned oam2addr;
	uint8_t oamdata_read_buffer;
	uint8_t oam_eval_addr;
	
	uint8_t spr_pt_addr_h;
	uint8_t spr_pt_addr_l;
	bool oam_copy_done;
	int overflow_bug_counter;
	bool sprite_eval_in_range;

	// $2005/$2006 write toggle.
	bool w;

	// $2007 read buffer.
	uint8_t ppudata_read_buffer;

	// ---------------------------------------------------------------------
	// PPU timing counters.
	// ---------------------------------------------------------------------
	bool odd_frame;
	int scanline;
	int dot;
	uint64_t frame;
	bool skip_dot;
	int ppu_tick_in_cpu_cycle;
	uint8_t frame_start_ppu_phase;

	// Current PPU address bus value.
	unsigned ppu_address_bus;
	uint8_t ppu_ad_latch = 0;
	bool ppu2007_ale_read_addr_latch_poison = false;
	uint8_t ppu2007_ale_read_low_latch = 0;
	uint8_t ppu_ale_low_latch;

	// ---------------------------------------------------------------------
	// Background fetch pipeline.
	// ---------------------------------------------------------------------
	uint8_t bg_nt_latch;
	uint8_t bg_at_latch;
	uint8_t bg_pt_l_latch;
	uint8_t bg_pt_h_latch;

	uint16_t bg_pt_l_shift;
	uint16_t bg_pt_h_shift;
	uint16_t bg_at_l_shift;
	uint16_t bg_at_h_shift;

	uint8_t bg_at_latch_l;
	uint8_t bg_at_latch_h;

	bool skip_bg_reload_once;

	uint16_t bg_fetch_v_nt = 0;
	uint16_t bg_fetch_v_at = 0;
	uint16_t bg_fetch_v_pt = 0;
	uint16_t bg_fetch_pt_base = 0;
	uint16_t bg_fetch_nt_addr;
	uint16_t bg_fetch_at_addr;

	// ---------------------------------------------------------------------
	// Sprite evaluation / sprite output pipeline.
	// ---------------------------------------------------------------------
	uint8_t spr_attr_latch[8];
	uint8_t spr_x_latch[8];
	uint8_t spr_x_counter[8];
	uint8_t spr_pt_l_shift[8];
	uint8_t spr_pt_h_shift[8];
	uint8_t spr_shift_count[8];
		
	//bool spr_pt_l_addr_valid[8];
	//bool spr_pt_h_addr_valid[8];

	bool sprite0_in_oam2_next;
	bool sprite0_in_oam2_current;

	uint8_t eval_sprite_y;
	uint8_t eval_sprite_tile;
	bool eval_candidate_in_range;

	uint8_t sprite0_eval_addr;
	uint8_t sprite0_pat;
	bool sprite0_hit_pending;
	uint8_t sprite0_hit_delay;

	bool sprite_go_this_line;
	bool sprite_go_next_line;
	bool sprite_sl0_early_shift_pending;

	uint8_t oam_latch_addr = 0;
	bool oam2_full;
	uint8_t oamdata_latch;
	uint8_t oam2_last_write;
	bool oam2_increment_frozen;
	bool sprite0_in_oam2_next_valid;

	bool s_after_wrap;
	bool sl0_stale_s0_loaded;
	bool sl0_stale_sprite0_identity;

	uint8_t overflow_eval_phase = 0;
	uint8_t overflow_finish_bytes = 0;

	bool sprite_eval_initialized;
	bool m_oam_eval_realigned = false;

	// ---------------------------------------------------------------------
	// Decoded render-enable state.
	//
	// *_output_enabled follows the immediate/current PPUMASK output-visible state.
	// *_pipeline_enabled follows the delayed/internal render pipeline state.
	// ---------------------------------------------------------------------
	bool bg_output_enabled;
	bool spr_output_enabled;
	bool bg_pipeline_enabled;
	bool spr_pipeline_enabled;
	
	// ---------------------------------------------------------------------
	// OAM corruption / sprite eval edge cases.
	// ---------------------------------------------------------------------
	bool oam_corrupt_pending;
	uint8_t oam_corrupt_seed;
	
	uint8_t corrupt_resume_high_lane;
	uint8_t corrupt_resume_oam2addr;
	bool corrupt_resume_fifo_pending;

	// Primary-OAM row that supplied each secondary-OAM/render-unit slot.
	uint8_t sec_oam_source[8];
	uint8_t sprite_oam_source[8];
	
	// ---------------------------------------------------------------------
	// Previous visible pixel state for retroactive $2001 edge behavior.
	// ---------------------------------------------------------------------
	bool prev_pixel_valid = false;
	int prev_pixel_scanline = 0;
	unsigned prev_pixel_x = 0;

	unsigned prev_bg_pixel_pat = 0;
	unsigned prev_attr_bits = 0;

	unsigned prev_spr_pat = 0;
	unsigned prev_spr_pal = 0;
	bool prev_spr_behind_bg = false;
	bool prev_spr_is_s0 = false;

	uint8_t prev_sprite0_pat = 0;
	uint8_t prev_backdrop_pal_index = 0;

	bool retro_ppumask_color = false;
	bool retro_ppumask_render = false;

	// ---------------------------------------------------------------------
	// Delayed $2007 read/write state.
	// ---------------------------------------------------------------------
	struct ppu2007_delayed_write
	{
		bool pending = false;
		int delay = 0;
		uint16_t addr = 0;
		uint8_t data = 0;
	};

	struct ppu2007_delayed_read
	{
		bool pending = false;
		int delay = 0;
		uint16_t addr = 0;

		// false = delayed direct refill from addr
		// true  = delayed rendering refill from the next real PPU bus read
		bool use_next_ppu_read_for_refill = false;

		// Rendering $2007 reads mature first, then wait here until an allowed
		// PPU fetch supplies the refill value.
		bool waiting_for_refill_bus_read = false;
	};

	ppu2007_delayed_write m_2007_write;
	ppu2007_delayed_read m_2007_read;

	bool ppu2007_buffer_fill_armed;
	bool ppu2007_buffer_fill_arm_pending = false;
	bool ppu_bus_read_can_fill_2007;

	bool ppu2007_post_bump_pending;
	int ppu2007_post_bump_delay;

	uint16_t spr_fetch_v_old;
	uint16_t spr_fetch_v_new;

	void schedule_2007_write(uint16_t addr, uint8_t data, int delay);
	void schedule_2007_read(uint16_t addr, int delay, bool use_next_ppu_read_for_refill);
	void schedule_2007_post_access_bump();
	
	// ---------------------------------------------------------------------
	// Mapper hooks / cached mapper capability flags.
	// ---------------------------------------------------------------------
	//nes_exrom_device *m_mmc5 = nullptr;
	//nes_txrom_device *m_mmc3 = nullptr;
	//nes_sxrom_device *m_mmc1_sxrom = nullptr;
	//nes_tengen032_device *m_rambo1 = nullptr;
	//nes_sc127_device *m_sc127 = nullptr;
	//nes_batmap_srrx_device *m_batmap_srrx = nullptr;

	//bool m_has_mmc3_a12 = false;
	//bool m_has_rambo1_a12 = false;
	//bool m_has_sc127_a12 = false;
	//bool m_has_mmc5_ppu = false;
	//bool m_has_mmc1_phase = false;
	//bool m_has_chr_latch = false;
	//bool m_has_batmap_srrx_a12 = false;

	// ---------------------------------------------------------------------
	// Save-state replacements for old function-local statics.
	// ---------------------------------------------------------------------
	uint8_t m_eval_wrap_byte = 0;
	uint8_t m_eval_prev_oam_latch_addr = 0x00;

	int m_save_sprite_size = 0;
	void presave();
	void postload();

	// ---------------------------------------------------------------------
	// PPU internal register/open-bus latch.
	//
	// This models the PPU-side I/O latch. It is not CPU open bus.
	// ---------------------------------------------------------------------
	uint8_t m_ppu_io_db = 0x00;
	uint64_t m_ppu_io_db_decay_at[8] = {};

	// Counts chosen to match the screenshot profile. These are hardware-profile
	// constants, not universal NES constants.
	static constexpr uint16_t ppudecay_count_for_bit[8] =
	{
		0x04f4,
		0x0398,
		0x02e3,
		0x030b,
		0x39e1,
		0xbdca,
		0x39e1,
		0x02a6
	};

	uint64_t ppudecay_now() const;
	uint64_t ppudecay_count_to_cpu_cycles(uint16_t count) const;
	void ppu_open_bus_drive(uint8_t data);
	uint8_t ppu_open_bus_peek();
	void ppu_open_bus_drive_masked(uint8_t data, uint8_t mask);

	// ---------------------------------------------------------------------
	// Palette / utility helpers.
	// ---------------------------------------------------------------------
	uint32_t m_nespens[0x40 * 8];

	uint8_t rev_byte(uint8_t n)
	{
		static uint8_t const rev_table[] =
		{
			0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
			0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
			0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
			0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
			0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
			0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
			0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
			0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
			0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
			0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
			0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
			0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
			0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
			0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
			0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
			0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
		};

		return rev_table[n];
	}

private:
	m6502_device* m_maincpu6502 = nullptr;

	inline uint16_t apply_grayscale_and_emphasis(uint8_t color);

	scanline_delegate m_scanline_callback_proc;
	hblank_delegate m_hblank_callback_proc;
	vidaccess_delegate m_vidaccess_callback_proc;
	devcb_write_line m_int_callback;

	// Legacy MAME render/helper state.
	int m_refresh_latch;
	int m_add;
	int m_videomem_addr;
	int m_data_latch;
	int m_buffered_data;
	int m_sprite_page;
	int m_scan_scale;
};


//**************************************************************************
//  RGB PPUs
//**************************************************************************

class ppu2c0x_rgb_device : public ppu2c0x_device
{
protected:
	ppu2c0x_rgb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void init_palette_tables() override;

private:
	required_region_ptr<uint8_t> m_palette_data;
};


//**************************************************************************
//  Concrete PPU device types
//**************************************************************************

class ppu2c02_device : public ppu2c0x_device
{
public:
	ppu2c02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class ppu2c03b_device : public ppu2c0x_rgb_device
{
public:
	ppu2c03b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class ppu2c04_device : public ppu2c0x_rgb_device
{
public:
	ppu2c04_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class ppu2c07_device : public ppu2c0x_device
{
public:
	ppu2c07_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class ppupalc_device : public ppu2c0x_device
{
public:
	ppupalc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class ppu2c05_01_device : public ppu2c0x_rgb_device
{
public:
	ppu2c05_01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class ppu2c05_02_device : public ppu2c0x_rgb_device
{
public:
	ppu2c05_02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class ppu2c05_03_device : public ppu2c0x_rgb_device
{
public:
	ppu2c05_03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class ppu2c05_04_device : public ppu2c0x_rgb_device
{
public:
	ppu2c05_04_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class ppu2c04_clone_device : public ppu2c0x_device
{
public:
	ppu2c04_clone_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

	virtual void draw_background(uint8_t *line_priority) override;
	virtual void draw_sprite_pixel(int sprite_xpos, int color, int pixel, uint8_t pixel_data, bitmap_rgb32 &bitmap) override;
	virtual void draw_sprites(uint8_t *line_priority) override;

	virtual void init_palette_tables() override;

protected:
	virtual void device_start() override;

private:
	required_region_ptr<uint8_t> m_palette_data;

	// Buffered sprite RAM for next frame.
	std::unique_ptr<uint8_t[]> m_spritebuf;
};


//**************************************************************************
//  Device type declarations
//**************************************************************************

DECLARE_DEVICE_TYPE(PPU_2C02,    ppu2c02_device)       // NTSC NES
DECLARE_DEVICE_TYPE(PPU_2C03B,   ppu2c03b_device)      // Playchoice 10
DECLARE_DEVICE_TYPE(PPU_2C04,    ppu2c04_device)       // Vs. Unisystem
DECLARE_DEVICE_TYPE(PPU_2C07,    ppu2c07_device)       // PAL NES
DECLARE_DEVICE_TYPE(PPU_PALC,    ppupalc_device)       // PAL Clones
DECLARE_DEVICE_TYPE(PPU_2C05_01, ppu2c05_01_device)    // Vs. Unisystem (Ninja Jajamaru Kun)
DECLARE_DEVICE_TYPE(PPU_2C05_02, ppu2c05_02_device)    // Vs. Unisystem (Mighty Bomb Jack)
DECLARE_DEVICE_TYPE(PPU_2C05_03, ppu2c05_03_device)    // Vs. Unisystem (Gumshoe)
DECLARE_DEVICE_TYPE(PPU_2C05_04, ppu2c05_04_device)    // Vs. Unisystem (Top Gun)
DECLARE_DEVICE_TYPE(PPU_2C04C,   ppu2c04_clone_device) // Vs. Unisystem (Super Mario Bros. bootlegs)

#endif // MAME_VIDEO_PPU2C0X_H

// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi,Brad Oliver,Brian H. Gilbert
/******************************************************************************

    Nintendo 2C0x PPU emulation.

    Written by Ernesto Corvi.
    This code is heavily based on Brad Oliver's MESS implementation.

******************************************************************************/

#ifndef MAME_VIDEO_PPU2C0X_H
#define MAME_VIDEO_PPU2C0X_H

#pragma once

#include <functional>

///*************************************************************************
//  MACROS / CONSTANTS
///*************************************************************************

// mirroring types
#define PPU_MIRROR_NONE       0
#define PPU_MIRROR_VERT       1
#define PPU_MIRROR_HORZ       2
#define PPU_MIRROR_HIGH       3
#define PPU_MIRROR_LOW        4
#define PPU_MIRROR_4SCREEN    5 // Same effect as NONE, but signals that we should never mirror

#define PPU_DRAW_BG       0
#define PPU_DRAW_OAM      1

/* constant definitions */
#define VISIBLE_SCREEN_WIDTH         (32*8) /* Visible screen width */
#define VISIBLE_SCREEN_HEIGHT        (30*8) /* Visible screen height */

///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> ppu2c0x_device

class ppu2c0x_device :  public device_t,
						public device_memory_interface,
						public device_video_interface,
						public device_palette_interface
{
public:
	// /NMI as the PPU drives it at time when (true: low), from the VBL flag and NMI enable as functions of time (PPUs
	// with their own timing). A 2A03 the PPU times against samples the pin itself and reads it here (the PPU connects it
	// when it starts); the pin is also driven, for any other listener.
	bool nmi_asserted_at(const attotime &when) const;

	typedef device_delegate<void (int scanline, bool vblank, bool blanked)> scanline_delegate;
	typedef device_delegate<void (int scanline, bool vblank, bool blanked)> hblank_delegate;
	typedef device_delegate<void ()> a12_delegate;
	typedef device_delegate<void (int)> a12_level_delegate;
	typedef device_delegate<void (u16)> addr_bus_delegate;
	typedef device_delegate<bool (const attotime &, const attotime &)> a12_filter_delegate;
	typedef device_delegate<void (int *ppu_regs)> nmi_delegate;
	typedef device_delegate<void (offs_t offset)> latch_delegate;
	using ppu_write_delegate = device_delegate<void (offs_t, u8)>;

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

	virtual u8 read(offs_t offset);
	virtual void write(offs_t offset, u8 data);
	virtual u8 palette_read(offs_t offset);
	virtual void palette_write(offs_t offset, u8 data);

	template <typename T> void set_cpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	auto int_callback() { return m_int_callback.bind(); }
	// EXT0-3 pins (NESdev PPUCTRL, master/slave): with PPUCTRL bit 6 clear they are inputs that give the backdrop's palette
	// index (a stock console grounds them: index 0); with bit 6 set the PPU drives the low four bits of each pixel's
	// palette index on them (offset = x). Unbound, the input reads 0 and nothing is output.
	auto ext_in() { return m_ext_in_cb.bind(); }
	auto ext_out() { return m_ext_out_cb.bind(); }

	void spriteram_dma(address_space &space, const u8 page);
	void set_spriteram_value(offs_t offset, u8 data) { m_spriteram[offset] = data; }

	void render(bitmap_rgb32 &bitmap, bool flipx, bool flipy, int sx, int sy, const rectangle &cliprect);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	int get_current_scanline() { return m_scanline; }
	bool dot_renderer() const { return use_dot_renderer(); }   // whether the PPU draws (and reports A12) dot by dot
	// the hblank callback at the start of HSYNC (dot 277) instead of the dot an MMC3 would see A12 rise
	void set_hblank_at_hsync(bool on) { m_hblank_at_hsync = on; }
	// callbacks can be set at configuration time (bound when the device starts) or later (bound at once)
	template <typename... T> void set_scanline_callback(T &&... args) { m_scanline_callback_proc.set(std::forward<T>(args)...); if (started()) m_scanline_callback_proc.resolve(); }
	template <typename... T> void set_hblank_callback(T &&... args) { m_hblank_callback_proc.set(std::forward<T>(args)...); if (started()) m_hblank_callback_proc.resolve(); }
	// dot renderer: called on every filtered rising edge of PPU A12 (MMC3-style scanline counters)
	template <typename... T> void set_a12_callback(T &&... args) { m_a12_callback_proc.set(std::forward<T>(args)...); if (started()) m_a12_callback_proc.resolve(); }
	// whether this PPU runs the dot renderer with its own timing (MMC5 uses it to pick its IRQ/in-frame model)
	bool rendering_now() const;   // public: cartridge mappers (MMC5) use it to tell when the PPU stops fetching
	bool dot_timing() const { return m_own_timing; }
	// Cartridge interface: the sprite whose bytes the PPU is fetching (NESdev PPU rendering: dots 257-320 of a rendered
	// line, eight dots per sprite). sprite_fetch_slot() is its secondary OAM slot (0-7), sprite_fetch_oam_index() the
	// primary OAM entry (0-63) that sprite evaluation copied into that slot, or -1 when evaluation left the slot empty;
	// both are -1 outside those dots, when rendering is off and on PPUs without their own dot timing. They describe the
	// dot being rendered when the PPU calls the cartridge (ppu_latch, the A12 and address bus callbacks).
	int sprite_fetch_slot() const;

	// NTSC 2C02 revision (NESdev "PPU variants"): RP2C02 to RP2C02D, RP2C02E, RP2C02G (default), RP2C02H. Before G,
	// OAMDATA and palette RAM are not readable; from E on, OAMADDR >= 8 at the start of rendering corrupts OAM. H behaves as G (its fixes are not
	// documented). Only the PPU_2C02 type uses it.
	enum : u8 { REV_2C02_EARLY = 0, REV_2C02E, REV_2C02G, REV_2C02H };
	void set_revision(u8 rev) { m_revision = rev; }
	// decode the picture from the PPU's composite signal (NESdev "NTSC video", "PAL video") instead of the palette:
	// 0 palette, 1 composite, 2 composite with delay-line chroma (PAL: each line's U and V averaged with the line above)
	void set_composite(u8 mode) { m_composite = mode; }
	int sprite_fetch_oam_index() const;
	// unfiltered PPU A12 level, reported on every change (MMC1: in 4K CHR mode, A12 selects which CHR register drives its upper outputs)
	template <typename... T> void set_a12_level_callback(T &&... args) { m_a12_level_proc.set(std::forward<T>(args)...); m_a12_level_proc.resolve(); }
	// PPU address bus, reported whenever A13/A12 change (Oeka Kids latches A8-A9 when they become 1/0); unlike the read
	// latch this sees every bus change, including $2006 writes outside rendering, which change the bus without an access
	template <typename... T> void set_addr_bus_callback(T &&... args) { m_addr_bus_proc.set(std::forward<T>(args)...); m_addr_bus_proc.resolve(); }
	// Acclaim MC-ACC: the counter is clocked when A12 falls after a counted rise (NESdev: "typically four pixels later than Sharp MMC3")
	void set_a12_on_fall(bool on_fall) { m_a12_on_fall = on_fall; }
	// NES: the console reset line also resets the PPU; Famicom: it does not (NESdev PPU power up state)
	void set_console_reset(bool resets_ppu) { m_console_reset = resets_ppu; }
	void set_power_on_warmup(bool warmup) { m_power_warmup = warmup; }
	// Where the PPU dot grid sits relative to the start of a CPU cycle, in master clocks (NESdev "CPU/PPU alignment":
	// the CPU divides the master clock by 12 and the PPU by 4, so on NTSC there are four possible power-on alignments,
	// 0-3; the 2C07 divides by 5, so 0-4).  It is fixed for the life of the machine.  Must be set before start.
	void set_cpu_ppu_alignment(u8 master_clocks) { m_cpu_ppu_align = master_clocks; }
	// optional board filter on the rising edges of PPU A12 reported to the A12 callback: it is given the time A12 went low and
	// the time it rose and returns whether the edge counts (MMC3: three falling edges of M2); without it, A12 has to have
	// been low for 10 PPU clocks
	template <typename... T> void set_a12_filter_callback(T &&... args) { m_a12_filter_proc.set(std::forward<T>(args)...); m_a12_filter_proc.resolve(); }
	void set_scanlines_per_frame(int scanlines) { m_scanlines_per_frame = scanlines; }

	// MMC5 has to be able to check this
	int is_sprite_8x16() { return m_regs[PPU_CONTROL0] & PPU_CONTROL0_SPRITE_SIZE; }
	int get_draw_phase() { return m_draw_phase; }
	int get_tilenum() { return m_tilecount; }

	// scanline renderer: called before each background tile fetch with the tile's pattern table address (MMC2/MMC4 latches)
	template <typename... T> void set_latch(T &&... args) { m_latch.set(std::forward<T>(args)...); if (started()) m_latch.resolve(); }
	// dot renderer: called after every pattern table read (background, sprites, PPUDATA) with the exact address read
	// (MMC2/MMC4 latches); when set, the before-read latch call above is not used by the dot renderer
	template <typename... T> void set_read_latch(T &&... args) { m_read_latch.set(std::forward<T>(args)...); if (started()) m_read_latch.resolve(); }
	// PPU bus strobes (dot renderer), called at the dot each happens, before the memory access: ALE latching an address,
	// /RD reading it (rendering fetches and PPUDATA reads), /WR writing VRAM (PPUDATA writes; palette writes stay inside)
	template <typename... T> void set_ale_callback(T &&... args) { m_ale_cb.set(std::forward<T>(args)...); if (started()) m_ale_cb.resolve(); }
	template <typename... T> void set_rd_callback(T &&... args) { m_rd_cb.set(std::forward<T>(args)...); if (started()) m_rd_cb.resolve(); }
	template <typename... T> void set_wr_callback(T &&... args) { m_wr_cb.set(std::forward<T>(args)...); if (started()) m_wr_cb.resolve(); }

	//  void update_screen(bitmap_t &bitmap, const rectangle &cliprect);

	u16 get_vram_dest();
	void set_vram_dest(u16 dest);
	void reload_refresh_data();

	u8 ppu_vram_direct_read(offs_t address);
	void ppu_vram_direct_write(offs_t address, u8 data);

	bool in_vblanking() { return (m_scanline >= m_vblank_first_scanline - 1); }
	// Forces the dot renderer to finish all fetches for real time that has already elapsed, using whatever
	// state (CHR/PRG banks, scroll, etc.) was in effect at the moment each dot was supposed to occur. Call this
	// before applying any mapper-side state change that the dot renderer's pending fetches should not retroactively
	// see (e.g. CHR bank switches) - otherwise already-elapsed dots can be rendered with the new value instead
	// of the one that was actually active when real time reached them.
	void catch_up() { apply_due_events(); if (use_dot_renderer()) dot_run_to(current_line_dot()); }
	// the same, up to a time later in the current CPU cycle (a cartridge register that only takes its value when M2 falls)
	void catch_up_to(const attotime &when) { apply_due_events(); if (use_dot_renderer()) dot_run_to(m_own_timing ? line_dot_at(when) : current_line_dot()); }
protected:
	ppu2c0x_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, u32 clock, address_map_constructor internal_map);

	// registers definition
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

	// bit definitions for (some of) the registers
	enum
	{
		PPU_CONTROL0_INC               = 0x04,
		PPU_CONTROL0_SPR_SELECT        = 0x08,
		PPU_CONTROL0_CHR_SELECT        = 0x10,
		PPU_CONTROL0_SPRITE_SIZE       = 0x20,
		PPU_CONTROL0_EXT_OUTPUT        = 0x40,   // master/slave select: EXT pins are outputs
		PPU_CONTROL0_NMI               = 0x80,

		PPU_CONTROL1_DISPLAY_MONO      = 0x01,
		PPU_CONTROL1_BACKGROUND_L8     = 0x02,
		PPU_CONTROL1_SPRITES_L8        = 0x04,
		PPU_CONTROL1_BACKGROUND        = 0x08,
		PPU_CONTROL1_SPRITES           = 0x10,
		PPU_CONTROL1_COLOR_EMPHASIS    = 0xe0,

		PPU_STATUS_8SPRITES            = 0x20,
		PPU_STATUS_SPRITE0_HIT         = 0x40,
		PPU_STATUS_VBLANK              = 0x80
	};

	// construction/destruction
	ppu2c0x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock = 0);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_config_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	virtual u32 palette_entries() const noexcept override { return 0x40 * 8; }

	TIMER_CALLBACK_MEMBER(hblank_tick);
	TIMER_CALLBACK_MEMBER(nmi_tick);
	u8 m_idle_nt = 0;               // nametable byte of the two unused fetches at the end of a line
	bool m_idle_dot_skipped = false; // the odd frame's short pre-render line left scanline 0's idle dot out
	void apply_due_events();
	bool m_applying_events = false;
	TIMER_CALLBACK_MEMBER(scanline_tick);
	TIMER_CALLBACK_MEMBER(vbl_tick);

	/* routines */
	rgb_t nespal_to_RGB(int color_intensity, int color_num, int color_emphasis, bool is_pal_or_dendy);
	virtual void init_palette_tables();

	virtual void write_to_spriteram_with_increment(u8 data);

	virtual void read_tile_plane_data(int address, int color);
	virtual void shift_tile_plane_data(u8 &pix);
	virtual void draw_tile_pixel(u8 pix, int color, u32 back_pen, u32 *&dest);
	virtual void draw_tile(u8 *line_priority, int color_byte, int color_bits, int address, int start_x, u32 back_pen, u32 *&dest);
	virtual void draw_background( u8 *line_priority );
	virtual void draw_back_pen(u32* dst, int back_pen);
	void draw_background_pen();

	virtual void read_sprite_plane_data(int address);
	virtual void make_sprite_pixel_data(u8 &pixel_data, bool flipx);
	virtual void draw_sprite_pixel(int sprite_xpos, int color, int pixel, u8 pixel_data, bitmap_rgb32 &bitmap);
	virtual bool is_spritepixel_opaque(int pixel_data, int color);
	virtual void draw_sprite_pixel_low(bitmap_rgb32& bitmap, int pixel_data, int pixel, int sprite_xpos, int color, int sprite_index, u8* line_priority);
	virtual void draw_sprite_pixel_high(bitmap_rgb32& bitmap, int pixel_data, int pixel, int sprite_xpos, int color, int sprite_index, u8* line_priority);
	virtual void read_extra_sprite_bits(int sprite_index);

	virtual int apply_sprite_pattern_page(int index1, int size);
	virtual void draw_sprites(u8 *line_priority);
	void render_scanline();
	virtual void scanline_increment_fine_ycounter();
	void update_visible_enabled_scanline();
	void update_visible_disabled_scanline();
	void update_visible_scanline();
	void update_scanline();

	// dot-based renderer: the standard 2C0x PPUs emulate every dot of a rendered scanline, and CPU register
	// accesses first bring the PPU up to the current beam position; clones with their own drawing keep the
	// scanline renderer above
	virtual bool use_dot_renderer() const { return true; }
	// whether the PPU keeps its own dot clock: the dot renderer always; the scanline renderer where the chip's timing is
	// known to be the 2C02's (the NTSC VT03 family)
	virtual bool keeps_own_timing() const { return use_dot_renderer(); }
	int current_line_dot();
	int line_dot_at(const attotime &when);
	attotime event_fire_time(const attotime &when);
	void arm_event(emu_timer *timer, const attotime &when, int param = 0);
	attotime m_next_line_start;   // logical start of the next line (the scanline timer may fire up to a CPU cycle later)
	attotime m_vbl_event_time;
	bool m_render_prev = false;    // m_render_eff as it was on the previous dot
	bool m_s0hit_pending = false;  // sprite 0 hit detected, flag not visible yet
	attotime m_s0hit_time;        // when it becomes visible in $2002    // logical time of the pending VBL-timer event
	u8 sprite_flags_at_m2_fall();
	void dot_run_to(int target);
	void dot_step(int dot);
	void dot_oam_row_corruption(int dot, bool apply);
	void dot_reload_shifters();
	void dot_fetch_nt(int dot);
	void dot_fetch_at();
	void dot_fetch_pattern(bool high);
	void dot_read_nt();
	void dot_read_at();
	void dot_read_pattern(bool high);
	void dot_increment_x();
	void dot_evaluate_sprites();
	void dot_fetch_sprite_slot(int slot);
	void bus_ale(u16 addr);
	u8 bus_read(u16 addr_now);
	void dot_oam_step(int dot, bool rendering);
	void oam2_increment();
	void dot_sprite_evaluation_step(int dot);
	void dot_output_pixel(int x, bool rendering);
	void dot_output_pixel_inner(int x, bool rendering);
	int hblank_dot();
	attotime dot_time(int dots);
	void schedule_next_line();
	u8 dot_oam_bus(int dot);
	u8 io_latch();
	void ppu_bus(u16 addr, attotime when);
	void ppu_bus_idle();
	void data_port_increment();
	attotime dot_when(int dot);
	void io_latch_refresh(u8 value, u8 mask);
	void update_nmi_line();

	// address space configurations
	const address_space_config      m_space_config;

	required_device<cpu_device> m_cpu;

	void start_nopalram();

	int                         m_scanlines_per_frame;  /* number of scanlines per frame */
	int                         m_security_value;       /* 2C05 protection */
	int                         m_vblank_first_scanline;  /* the very first scanline where VBLANK occurs */

	// used in rendering
	u8 m_planebuf[16]; // temp buffer used for fetching tile data
	s32                    m_scanline;         /* scanline count */
	std::unique_ptr<u8[]>  m_spriteram;           /* sprite ram */

	int m_videoram_addr_mask;
	int m_global_refresh_mask;
	int m_line_write_increment_large;
	bool m_paletteram_in_ppuspace; // sh6578 doesn't have the palette in PPU space, so various side-effects don't apply
	std::vector<u8>        m_palette_ram;          /* shouldn't be in main memory! */
	bitmap_rgb32                m_bitmap;          /* target bitmap */
	u8                          m_regs[PPU_MAX_REG];        /* registers */
	u32                         m_tile_page;            /* current tile page */
	u8                          m_back_color;           /* background color */
	s32                         m_refresh_data;         /* refresh-related */
	u8                          m_x_fine;               /* refresh-related */
	bool                        m_toggle;               /* used to latch hi-lo scroll */
	bool                        m_render_enabled_hblank; /* BG/sprite rendering state sampled in hblank, used for the scroll increment */
	s32                         m_tilecount;            /* MMC5 can change attributes to subsets of the 34 visible tiles */
	int                         m_line_dot;             /* dot renderer: next dot of the current scanline still to be emulated */
	u16                         m_bg_shift_lo, m_bg_shift_hi, m_at_shift_lo, m_at_shift_hi; /* dot renderer: background shifters */
	u8                          m_bg_next_nt, m_bg_next_at, m_bg_next_lo, m_bg_next_hi;     /* dot renderer: fetched tile latches */
	u8                          m_spr_count;            /* dot renderer: sprites found for the current line */
	u8                          m_spr_x[8], m_spr_attr[8], m_spr_lo[8], m_spr_hi[8];
	// the eight sprite output units (NESdev PPU rendering; TriCNES): X down-counter, attribute latch and the two pattern
	// shift registers, loaded during the sprite fetches of dots 257-320
	u8                          m_sp_ctr[8], m_sp_attr[8], m_sp_lo[8], m_sp_hi[8];
	// extra pattern planes 2 and 3 (VT03 family 4bpp; always 0 on the 2C02): background latches and shift registers, and
	// per sprite output unit; a unit in 16-pixel mode draws them as a second 8-pixel half (m_sp_half counts its pixels)
	u8                          m_bg_next_x0 = 0, m_bg_next_x1 = 0;
	u16                         m_bg_shift_x0 = 0, m_bg_shift_x1 = 0;
	u16                         m_at_shift_x0 = 0, m_at_shift_x1 = 0;   // palette bits 2-3 (SH6578; always 0 on the 2C02)
	u8                          m_sp_x0[8] = { }, m_sp_x1[8] = { }, m_sp_half[8] = { };

	// dot renderer hooks for PPUs with pattern buses of their own or more pattern planes (VT03 family); the defaults are
	// the 2C02's: a pattern byte is what the PPU bus read returned, there are no extra planes, the palette gives the colour
	virtual u8 dot_bg_pattern(u16 addr, bool high, u8 bus_data, u8 attr, u8 &extra) { extra = 0; return bus_data; }
	virtual u8 dot_sp_pattern(u16 addr, bool high, u8 bus_data, u8 attr, u8 &extra) { extra = 0; return bus_data; }
	virtual bool dot_sprite_16px() const { return false; }
	// the bits of a sprite's attribute byte that exist in OAM: $E3 on the 2C02 (bits 2-4 read back as 0); the VT03 family
	// keeps all eight (its bits 2-4 select a sprite CHR bank)
	virtual u8 oam_attr_mask() const { return 0xe3; }
	// the 2C02's PPUDATA access pipeline (TriCNES): a PPU without that internal path (the VT03 family, whose VRAM DMA
	// writes PPUDATA every CPU cycle) reads and writes VRAM and palette RAM directly instead
	virtual bool has_data_pipeline() const { return true; }
	virtual bool dot_pixel_rgb(int x, u8 bg_pix, u8 bg_pal, bool bg_opaque, u8 sp_pix, u8 sp_pal, bool sprite_wins, u32 *dest) { return false; }
	// background fetch addressing and opacity (SH6578: two-byte nametable entries, a per-tile palette, palette-based
	// transparency); the defaults are the 2C02's
	virtual u16 dot_nt_addr() const { return 0x2000 | (m_refresh_data & 0x0fff); }
	virtual u16 dot_at_addr() const { const u16 v = m_refresh_data; return 0x23c0 | (v & 0x0c00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07); }
	virtual u8 dot_at_decode(u8 data) { if (m_refresh_data & 0x0040) data >>= 4; if (m_refresh_data & 0x0002) data >>= 2; return data & 0x03; }
	virtual u16 dot_pattern_addr(bool high) const { return ((m_tile_page ? 0x1000 : 0) + (m_bg_next_nt * 16) + ((m_refresh_data >> 12) & 7) + (high ? 8 : 0)) & 0x1fff; }
	virtual bool dot_bg_opaque(u8 bg_pix, u8 bg_pal) const { return bg_pix != 0; }
	// called after a visible line's last pixel; a PPU with a background/sprite engine the dot renderer does not model
	// (VT3xx extended video) redraws the line there, from m_line_v: v as it is for that line's first tile (captured on
	// dot 320 of the line before, after the horizontal and vertical copies and before the two prefetched tiles)
	virtual void dot_line_done() { }
	// called on dot 320, before the two tiles of the next line are prefetched (2C04 clone: name table bits)
	virtual void dot_before_prefetch() { }
	// the VRAM address of PPUDATA is v, the scroll register (2C02); the 2C04 clone keeps a separate one ($2006 does not
	// change its scroll), which PPUDATA uses and increments
	virtual bool vram_addr_is_v() const { return true; }
	u16 m_line_v = 0, m_line_v_next = 0;
	u8                          m_at_latch;
	bool                        m_spr_in_range[8] = { };
	// sprite evaluation as the 2C02 does it (TriCNES): it walks OAMADDR itself, writes secondary OAM through its own 5-bit
	// address, which freezes once it wraps until reset on dots 63/255/339, and passes every byte through the OAM buffer
	// PPU memory bus (TriCNES): an access takes two dots, an address-latch dot that puts the address on the bus and
	// captures its low 8 bits in the external octal latch, then a read dot whose address is the high bits as they are then
	// with the latched low byte
	u8                          m_line_pal[256] = { };   // palette value of each pixel of the line, before greyscale/emphasis
	u16                         m_line_idx[256] = { };   // composite: each pixel's 9-bit colour index (emphasis, level, colour)
	u8                          m_composite = 0;
	double                      m_comp_prev_u[256] = { }, m_comp_prev_v[256] = { };   // delay-line PAL decoding
	void composite_decode_line();
	int                         m_line_pal_y = -1;
	u8                          m_out_mask = 0, m_out_mask_next = 0, m_out_mask_delay = 0;   // PPUMASK greyscale/emphasis as the pixel output sees them
	u16                         m_bus_addr = 0;
	u8                          m_octal = 0;
	u16                         m_pat_addr = 0;     // pattern address register: the background pattern fetch being made
	// PPUDATA access state machines (TriCNES PPU_DATA_StateMachine*): a CPU read/write sets an SR latch at the end of the
	// access, which ripples through a chain of latches stepped on full and half dots and produces an address-latch
	// strobe, then the buffer read / VRAM write and the v increment
	bool m_r_sr = false, m_r_req = false, m_rl[5] = { }, m_pd_rb = false, m_r_ale = false;
	bool m_w_sr = false, m_w_req = false, m_wl[5] = { }, m_w_ale = false, m_db_par = false, m_tstep_latch = false;
	bool m_blnk_latch = true, m_pal_en = false, m_bus_read = false, m_bus_ale = false;
	u8 m_w_data = 0;
	attotime m_r_start, m_w_start;   // end of the CPU access that starts the read/write machine (never: none pending)
	u8 m_2006_delay = 0;             // dots until a $2006 second write reaches v and the bus
	u16 m_2006_new = 0, m_2006_old = 0;
	bool m_copyv = false;           // v was loaded from $2006 on this dot (a Y increment on the same dot ANDs the two)
	bool use_data_pipeline() const { return use_dot_renderer() && m_own_timing && has_data_pipeline(); }
	// NESdev "PPU variants": on the RGB PPUs (2C03, 2C04, 2C05) OAMDATA and palette RAM are not readable
	bool rgb_ppu() const;
	// OAMDATA and palette RAM readable: not on the RGB PPUs, not on 2C02 revisions before G
	bool oam_palette_readable() const;
	// OAMADDR >= 8 when rendering starts corrupts OAM: 2C02 revisions E, G and H
	bool oam_corruption() const;
	// 2C07: the forced OAM refresh of scanlines 265-310, 24 lines after the NMI (NESdev "PPU OAM")
	bool pal_oam_refresh_line() const;
	u8 bus_fetch();
	u16 m_fetch_addr = 0;
	void dot_step_main(int dot);
	void bus_write_vram(u8 data);
	void data_sm_full(int dot);
	void data_sm_full_end();
	void data_sm_half(int dot);
	void data_increment_y();
	// the lazily run PPU must still reach the dots where a PPUADDR/PPUDATA access takes effect when nothing else makes
	// it catch up (its bus changes drive A12 for the cartridge)
	emu_timer *m_pipe_timer = nullptr;
	TIMER_CALLBACK_MEMBER(pipe_tick);
	void pipe_catchup(int dots);
	u8                          m_oam_buf = 0xff;   // OAM buffer (what $2004 reads while rendering)
	u8                          m_oam2_addr = 0;
	bool                        m_oam2_ovf = false;
	u8                          m_oam2_reset = 0;   // dots until a pending secondary-OAM address reset
	u8                          m_eval_tick = 0;    // 0: Y range check, 1-3: tile, attribute, X
	bool                        m_eval_nine = false;
	bool                        m_eval_addr_ovf = false;
	u8                          m_fetch_y = 0xff, m_fetch_tile = 0xff, m_fetch_attr = 0xff, m_fetch_x = 0xff;   // attribute bits feeding the attribute shifters' serial inputs
	bool                        m_spr_zero_in_line;     /* dot renderer: slot 0 holds OAM sprite 0 */
	latch_delegate              m_latch;
	latch_delegate              m_read_latch;
	latch_delegate              m_ale_cb;
	latch_delegate              m_rd_cb;
	ppu_write_delegate          m_wr_cb;

	u16                         m_spriteramsize;

	u8 readbyte(offs_t address);

	void ppu2c0x(address_map &map) ATTR_COLD;

private:
	inline void writebyte(offs_t address, u8 data);
	inline u16 apply_grayscale_and_emphasis(u8 color);

	scanline_delegate           m_scanline_callback_proc;   /* optional scanline callback */
	hblank_delegate             m_hblank_callback_proc; /* optional hblank callback */
	a12_delegate                m_a12_callback_proc;    /* optional A12 rising-edge callback (dot renderer) */
	a12_level_delegate          m_a12_level_proc;       /* optional unfiltered A12 level callback (dot renderer) */
	addr_bus_delegate           m_addr_bus_proc;        /* optional address bus callback on A13/A12 changes (dot renderer) */
	u8                          m_bus_a13a12;           /* A13/A12 last driven on the PPU address bus */
	a12_filter_delegate         m_a12_filter_proc;      /* optional board filter on A12 rising edges (dot renderer) */
	emu_timer                   *m_a12_timer;
	emu_timer                   *m_a12_catchup_timer;
	TIMER_CALLBACK_MEMBER(a12_tick);
	TIMER_CALLBACK_MEMBER(a12_catchup_tick);
	bool                        m_a12;                  /* level of PPU A12 on the address bus */
	bool                        m_a12_on_fall = false;  /* MC-ACC: report on the fall after a counted rise */
	bool                        m_a12_armed = false;    /* MC-ACC: a counted rise is waiting for its fall */
	bool                        m_console_reset = true; /* the console reset reaches the PPU (NES yes, Famicom no) */
	u8                          m_cpu_ppu_align = 0;    /* master clocks the PPU dot grid leads the CPU cycle grid by */
	bool                        m_powered = false;      /* the first reset is the cold power-on */
	bool                        m_power_warmup = false; /* CPU and PPU leave reset together at power-on (NES-001): the warm-up applies then too */
	bool                        m_render_eff = false;   /* internal rendering-enabled state (follows PPUMASK 2 dots late) */
	bool                        m_render_pending = false;
	int                         m_render_delay = 0;     /* dots until m_render_pending takes effect (0 = none) */
	u8                          m_rdbuf_value = 0;      /* PPUDATA read: byte fetched at the read, entering the read buffer later */
	int                         m_rdbuf_delay = 0;      /* dots until that refill happens (0 = none pending) */
	u8                          m_eval_start = 0;
	bool                        m_oamw_pending = false; /* OAMDATA write waiting to reach OAM (it lands a few dots after the CPU write) */
	u8                          m_oamw_data = 0;
	int                         m_oamw_delay = 0;       /* dots until it lands (0 = none pending) */
	int                         m_oamw_latency = 3;
	int                         m_mask_latency = 3;     /* dots after a PPUMASK write until rendering turns on/off */     /* how many dots after the CPU write an OAMDATA write lands */
	void oam_data_write_apply(u8 data);
	void oam_data_write_flush();       /* OAMADDR as of dot 65: where this line's sprite evaluation starts */
	u32                         m_oam_corrupt_rows = 0; /* pending rendering-transition OAM row copies */
	bool                        m_oam_start_copied = false; /* the render-start OAM copy has been done on this pre-render line */
	attotime                    m_warmup_until;         /* writes to $2000/$2001/$2005/$2006 are ignored until then */
	void a12_report(attotime when);
	attotime                    m_a12_low_since;        /* when A12 last went low */
	u16                         m_spr_addr[8];          /* sprite pattern addresses fetched on dots 257-320 */
	int                         m_ovf_dot;              /* dot of the current line on which sprite evaluation sets the overflow flag (-1: none) */
	devcb_write_line            m_int_callback;         /* nmi access callback from interface */
	devcb_read8                 m_ext_in_cb;            /* EXT0-3 as inputs: backdrop palette index */
	devcb_write8                m_ext_out_cb;           /* EXT0-3 as outputs: each pixel's palette index */

	s32                         m_refresh_latch;        /* refresh-related */
	s32                         m_add;              /* vram increment amount */
	u32                         m_videomem_addr;        /* videomem address pointer */
	u8                          m_data_latch;           /* latched videomem data */
	u8                          m_buffered_data;
	u32                         m_sprite_page;          /* current sprite page */
	s32                         m_scan_scale;           /* scan scale */
	s32                         m_draw_phase;           /* MMC5 uses different regs for BG and OAM */

	// timers
	emu_timer                   *m_hblank_timer;        /* hblank period at end of each scanline */
	emu_timer                   *m_hsync_timer = nullptr; /* start of HSYNC, for a device that counts it (own timing) */
	TIMER_CALLBACK_MEMBER(hsync_tick);
	emu_timer                   *m_nmi_timer;           /* NMI timer */
	emu_timer                   *m_scanline_timer;      /* scanline timer */
	emu_timer                   *m_vbl_timer;           /* dot renderer: VBL flag set/clear at dot 1 */
	bool                        m_vbl_suppress;         /* dot renderer: $2002 read one clock early suppresses this frame's VBL */
	bool                        m_vbl_set_pending;      /* dot renderer: the VBL flag for this frame has not been set yet */
	bool                        m_nmi_line;             /* dot renderer: current level of /NMI (VBL && NMI enable) */
	bool                        m_own_timing;           /* PPU keeps its own dot clock (any renderer) instead of following the screen refresh */
	attotime                    m_dot_period;           /* dot renderer: one PPU clock */
	attotime                    m_line_start;           /* dot renderer: when the current line began */
	bool                        m_odd_frame;            /* dot renderer: NTSC odd frames skip one clock while rendering */
	bool                        m_nmi_vbl;              /* dot renderer: vblank as seen by the NMI logic (ends one clock before the readable flag) */
	// /NMI sampled by the CPU (own timing): the VBL flag and the NMI enable as functions of time
	bool                        m_nmi_sampled = false;
	std::function<void (const attotime &)> m_irq_assert_time;
	attotime                    m_vbl_set_at;           // start of this frame's set pulse (dot 1 of the first vblank line, first half)
	bool                        m_vbl_set_blocked = false;   // a $2002 read held the flag clear through the end of that pulse
	attotime                    m_vbl_clr_at;           // when the flag was last cleared (a $2002 read, or the pre-render line)
	attotime                    m_nmi_en_at;            // when PPUCTRL's NMI enable last changed
	attotime                    m_vbl_nmi_end_at;       // start of the pre-render line: where vblank ends for /NMI
	bool                        m_nmi_en_prev = false;  // the NMI enable before that change
	attotime                    m_nmi_update_last;      // latest time the pin is due to follow the model
	bool vbl_flag_at(const attotime &when) const;
	void nmi_line_update_due(const attotime &event);
	attotime nmi_output_delay() const;   // 3 master clocks; none on the single-chip Dendy-style clones
	// the master clock grid the CPU and PPU share: master clocks per CPU cycle (12 NTSC, 16 PAL 2C07, 15 Dendy-style clones)
	// and per PPU dot (4, 5, 5)
	u32 cpu_master_clocks() const;
	attotime m2_rise_offset() const;
	attotime access_end_offset() const;
	u32 dot_master_clocks() const;
	u8                          m_revision = REV_2C02G;
	bool                        m_hblank_at_hsync = false;
	u8                          m_sec_oam_src[8];       /* dot renderer: primary OAM entry copied into each secondary OAM slot, $ff if none */
	u8                          m_sec_oam[32];          /* dot renderer: secondary OAM for the next line (exposed through $2004 during rendering) */
	u8                          m_eval_n, m_eval_m;     /* dot renderer: primary OAM sprite/byte indices */
	u8                          m_eval_sec;             /* dot renderer: next secondary OAM byte */
	u8                          m_eval_latch;           /* dot renderer: primary OAM read latch */
	u8                          m_eval_found;           /* dot renderer: sprites copied to secondary OAM */
	u8                          m_eval_sprite0_mask;    /* dot renderer: selected slots originating from OAM sprite 0 */
	bool                        m_eval_copying;         /* dot renderer: copying tile/attribute/X of an in-range sprite */
	bool                        m_eval_done;            /* dot renderer: primary OAM scan wrapped/completed */
	attotime                    m_decay_refresh[8];     /* PPU open bus: when each bit of the I/O latch was last refreshed (bits decay after ~600 ms) */
};

class ppu2c0x_rgb_device : public ppu2c0x_device {
protected:
	ppu2c0x_rgb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock = 0);

	virtual void init_palette_tables() override;

private:
	required_region_ptr<u8> m_palette_data;
};

class ppu2c02_device : public ppu2c0x_device {
public:
	ppu2c02_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ppu2c03b_device : public ppu2c0x_rgb_device {
public:
	ppu2c03b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ppu2c04_device : public ppu2c0x_rgb_device {
public:
	ppu2c04_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ppu2c07_device : public ppu2c0x_device {
public:
	ppu2c07_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ppupalc_device : public ppu2c0x_device {
public:
	ppupalc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ppu2c05_01_device : public ppu2c0x_rgb_device {
public:
	ppu2c05_01_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ppu2c05_02_device : public ppu2c0x_rgb_device {
public:
	ppu2c05_02_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ppu2c05_03_device : public ppu2c0x_rgb_device {
public:
	ppu2c05_03_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ppu2c05_04_device : public ppu2c0x_rgb_device {
public:
	ppu2c05_04_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ppu2c04_clone_device : public ppu2c0x_device {
public:
	ppu2c04_clone_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;

protected:
	virtual void device_start() override ATTR_COLD;

	virtual u32 palette_entries() const noexcept override { return 0x40 * 2; }
	virtual void device_reset() override ATTR_COLD;
	// drawn by the dot renderer, with the 2C02's fetch timing (assumed: nothing documents this clone); its timing still
	// follows the screen (its frame is not the 2C02's) and its quirks come in through the hooks below
	virtual bool keeps_own_timing() const override { return false; }
	virtual bool vram_addr_is_v() const override { return false; }
	virtual void dot_before_prefetch() override;
	virtual void dot_line_done() override;
	virtual bool dot_pixel_rgb(int x, u8 bg_pix, u8 bg_pal, bool bg_opaque, u8 sp_pix, u8 sp_pal, bool sprite_wins, u32 *dest) override;

	virtual void draw_background(u8 *line_priority) override;
	virtual void draw_sprite_pixel(int sprite_xpos, int color, int pixel, u8 pixel_data, bitmap_rgb32 &bitmap) override;
	virtual void draw_sprites(u8 *line_priority) override;

	virtual void init_palette_tables() override;

private:
	required_region_ptr<u8> m_palette_data;

	std::unique_ptr<u8[]>   m_spritebuf; /* buffered sprite ram for next frame */
};

// device type definition
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

// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn

#ifndef MAME_MACHINE_XAVIX_VIDEO_H
#define MAME_MACHINE_XAVIX_VIDEO_H

#pragma once

#include "emupal.h"
#include "screen.h"

#include <array>
#include <memory>
#include <utility>

class xavix_video_device : public device_t, public device_video_interface, public device_gfx_interface
{
public:
	static constexpr u32 NTSC_LINE_CLOCKS = 2730;
	static constexpr u32 PAL_LINE_CLOCKS = 2724;
	static constexpr unsigned NTSC_FIELD_HALF_LINES = 525;
	static constexpr unsigned PAL_FIELD_HALF_LINES = 625;
	static constexpr int PICTURE_PIXEL_CLOCKS = 8;

	// TODO: Confirm PAL-M and PAL-N selection and status encoding on applicable hardware.
	enum class video_standard : u8 { NTSC, PAL_M, PAL_N, PAL };

	xavix_video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_video_standard(video_standard standard) { m_video_standard = standard; }
	template <typename T> void set_gfx_region(T &&tag) { m_gfx_region.set_tag(std::forward<T>(tag)); }
	template <typename... T> void set_video_read_callback(T &&... args) { m_video_read_cb.set(std::forward<T>(args)...); }
	auto irq_callback() { return m_irq_cb.bind(); }
	auto nmi_callback() { return m_nmi_cb.bind(); }
	auto vblank_callback() { return m_vblank_cb.bind(); }
	auto sprite_dma_source_callback() { return m_sprite_dma_source_cb.bind(); }

	virtual void map(address_map &map) ATTR_COLD;
	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u8 read_low_bus_bypass(u32 address);
	virtual void vblank();

protected:
	static constexpr int RASTER_REFERENCE_DELAY_CLOCKS = 8;
	static constexpr int PICTURE_FIRST_ROW = 16;
	static constexpr int PICTURE_END_ROW = 240;
	static constexpr int SPRITE_COUNT = 0x100;
	static constexpr int SPRITE_RAM_PLANES = 8;
	static constexpr int SPRITE_RAM_SIZE = SPRITE_COUNT * SPRITE_RAM_PLANES;
	static constexpr int PALETTE_BANK_ENTRIES = 0x100;
	static constexpr int TILEMAP_WRAP_WIDTH = 0x100;
	static constexpr int TILEMAP_WRAP_HEIGHT = 0x100;
	static constexpr u8 TRANSPARENT_HUE_MIN = 0x18;
	static constexpr u8 INLINE_HEADER_END = 0x06;
	static constexpr unsigned COLOR_EFFECT_NOISE_BITS = 3;
	static constexpr unsigned COLOR_EFFECT_NOISE_LEVELS = 1U << COLOR_EFFECT_NOISE_BITS;

	enum class palette_plane { SH, L };

	enum tilemap_register : unsigned
	{
		TILEMAP_PLANE_LOW = 0x00,
		TILEMAP_PLANE_MIDDLE = 0x01,
		TILEMAP_PLANE_HIGH = 0x02, // address high byte or per-tile attributes
		TILEMAP_FORMAT = 0x03,
		TILEMAP_SCROLL_X = 0x04,
		TILEMAP_SCROLL_Y = 0x05,
		TILEMAP_PALETTE_PRIORITY = 0x06,
		TILEMAP_CTRL = 0x07,
		TILEMAP_REGISTER_COUNT = 0x08
	};

	enum tilemap_ctrl : u8
	{
		TILEMAP_ATTRIBUTES = 0x08,
		TILEMAP_INLINE_HEADER = 0x10,
		TILEMAP_ENABLE = 0x80
	};

	enum sprite_ram_plane : unsigned
	{
		SPRITE_ATTR0 = 0 * SPRITE_COUNT,
		SPRITE_ATTR1 = 1 * SPRITE_COUNT,
		SPRITE_Y = 2 * SPRITE_COUNT,
		SPRITE_X = 3 * SPRITE_COUNT,
		SPRITE_X_HIGH = 4 * SPRITE_COUNT,
		SPRITE_ADDRESS_LOW = 5 * SPRITE_COUNT,
		SPRITE_ADDRESS_MIDDLE = 6 * SPRITE_COUNT,
		SPRITE_ADDRESS_HIGH = 7 * SPRITE_COUNT
	};

	enum sprite_dma_ctrl : u8
	{
		SPRITE_DMA_BLOCK_COUNT_MASK = 0x07,
		SPRITE_DMA_IRQ_ENABLE = 0x20,
		SPRITE_DMA_RUN = 0x40,
		SPRITE_DMA_COMPLETE = 0x80
	};

	enum video_ctrl : u8
	{
		VIDEO_RASTER_IRQ_ENABLE = 0x10,
		VIDEO_VBLANK_NMI_ENABLE = 0x20,
		VIDEO_RASTER_IRQ_PENDING = 0x40,
		VIDEO_VBLANK_NMI_PENDING = 0x80
	};

	enum arena_ctrl : u8
	{
		ARENA_SIDE = 0x01,
		ARENA_IRQ_ENABLE = 0x02,
		ARENA_STATUS_ACTIVE = 0x40,
		ARENA_STATUS_PENDING = 0x80
	};

	enum color_effect_ctrl : u8
	{
		COLOR_EFFECT_HUE = 0x01,
		COLOR_EFFECT_SATURATION = 0x02,
		COLOR_EFFECT_LIGHTNESS = 0x04,
		COLOR_EFFECT_DIM = 0x08,
		COLOR_EFFECT_INVERT = 0x10
	};

	struct arena_state
	{
		bool flag = false;
		bool output = false;
		bool compare = false;
	};

	struct video_counters
	{
		u16 horizontal = 0;
		u16 vertical = 0;
		u16 picture_x = 0;
		u16 picture_y = 0;
	};

	struct counter_config
	{
		u16 horizontal_period;
		u16 vertical_period;
		u16 horizontal_reference;
		u16 vertical_reference;
		u16 left;
		u16 top;
	};

	enum : unsigned
	{
		PREDICT_FIELD = 0x01,
		PREDICT_NMI = 0x02,
		PREDICT_RASTER = 0x04,
		PREDICT_ARENA = 0x08
	};

	struct picture_event_delays
	{
		attotime field = attotime::never;
		attotime nmi = attotime::never;
		attotime raster = attotime::never;
		attotime arena = attotime::never;
	};

	struct pixel_layout
	{
		bool flip_x;
		bool flip_y;
		int pixel_phase;
		int pixel_width;
	};

	class packed_reader
	{
	public:
		packed_reader(device_delegate<u8 (u32)> &read, offs_t address) : m_read(read), m_address(address) { }
		u8 read(unsigned bits);

	private:
		device_delegate<u8 (u32)> &m_read;
		offs_t m_address;
		u32 m_data = 0;
		unsigned m_bits = 0;
	};

	xavix_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int video_hres_scale);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	template <palette_plane Plane> u8 graphics_palram_r(offs_t offset);
	template <palette_plane Plane> void graphics_palram_w(offs_t offset, u8 data);
	virtual u16 palette_data(int pen) const;
	virtual void set_palette_data(int pen, u16 data);
	u8 spriteram_r(offs_t offset);
	void spriteram_cpu_w(offs_t offset, u8 data);
	void spriteram_w(offs_t offset, u8 data);
	static u16 replace_pen_lightness(u16 palette_data, u16 lightness);
	static u16 replace_pen_saturation(u16 palette_data, u16 saturation);
	static u16 replace_pen_hue(u16 palette_data, u16 hue);
	static u8 pen_lightness(u16 palette_data);
	static u8 pen_saturation(u16 palette_data);
	static u8 pen_hue(u16 palette_data);
	static rgb_t pen_color(u16 palette_data);
	void update_pen(int pen, u16 data);
	rectangle arena_effect_area() const;
	u16 apply_color_effect(u16 data, u8 noise) const;
	static int aligned_output_position(int clocks, int alignment_clocks, int sample_clocks);
	virtual std::pair<int, int> counter_offset() const;
	virtual std::pair<int, int> picture_offset() const;
	virtual std::pair<int, int> render_offset() const;
	virtual auto decode_pixel_layout(u8 flags) const -> pixel_layout;
	virtual device_delegate<u8 (u32)> &tile_read_callback();
	u32 classic_segment_base(unsigned segment) const;
	virtual u32 graphics_segment_base(unsigned segment) const;
	virtual u32 extended_graphics_segment_base(unsigned segment) const;
	void decode_graphics(memory_region *extra_region = nullptr) ATTR_COLD;
	virtual void mix_bitmap_layer(screen_device &screen, const rectangle &cliprect) { }
	void sprite_dma_ctrl_w(u8 data);
	TIMER_CALLBACK_MEMBER(sprite_dma_tick);
	virtual u8 video_status_r();
	template <unsigned Layer> void tilemap_regs_w(offs_t offset, u8 data, u8 mem_mask);
	void sprite_address_mode_w(u8 data);
	u8 text_array_r(offs_t offset);
	void text_array_w(offs_t offset, u8 data);
	u32 advance_video_noise_state(u32 state, u64 clocks) const;
	void sync_video_noise();
	void video_noise_feedback_w(offs_t offset, u8 data);
	void color_effect_w(offs_t offset, u8 data);
	u8 video_noise_state_r(offs_t offset);
	u8 video_ctrl_r();
	void video_ctrl_w(u8 data);
	void update_video_irq_source();
	TIMER_CALLBACK_MEMBER(raster_irq_gen);
	virtual void schedule_raster_irq();
	void raster_irq_x_w(u8 data);
	void raster_irq_y_w(u8 data);
	u8 arena_start_r();
	void arena_start_w(u8 data);
	u8 arena_end_r();
	void arena_end_w(u8 data);
	u8 arena_ctrl_r();
	void arena_ctrl_w(u8 data);
	TIMER_CALLBACK_MEMBER(arena_irq_gen);
	void schedule_arena_irq();
	void sync_video_counters();
	virtual counter_config counter_parameters() const;
	virtual void reset_video_timing();
	void configure_raster(int width, int height, rectangle const &visible, attotime period);
	void reset_raster_origin(int row);
	virtual rectangle picture_area() const;
	virtual void raster_end() { }
	static u32 counter_interval(counter_config const &config, video_counters const &counters);
	static void advance_counters(counter_config const &config, video_counters &counters, u64 ticks);
	u32 arena_interval(counter_config const &config, video_counters const &counters, arena_state const &arena) const;
	bool advance_arena(counter_config const &config, video_counters &counters, arena_state &arena, u64 ticks) const;
	picture_event_delays predict_picture_delays(unsigned events) const;
	virtual void schedule_picture_irqs();
	TIMER_CALLBACK_MEMBER(counter_field_start);

	device_delegate<u8 (u32)> m_video_read_cb;
	devcb_write_line m_irq_cb;
	devcb_write_line m_nmi_cb;
	devcb_write_line m_vblank_cb;
	devcb_read8 m_sprite_dma_source_cb;
	required_shared_ptr<u8> m_spriteram;
	required_shared_ptr<u8> m_tilemap1_regs;
	required_shared_ptr<u8> m_tilemap2_regs;
	required_shared_ptr<u8> m_graphics_palram_sh;
	required_shared_ptr<u8> m_graphics_palram_l;
	required_shared_ptr<u8> m_raster_irq_x;
	required_shared_ptr<u8> m_raster_irq_y;
	required_shared_ptr<u8> m_segment_regs;

	u8 m_text_array[3];
	u8 m_sprite_read_latch;
	u8 m_graphics_palette_read_latch;
	u8 m_sprite_dma_src[2];
	u8 m_sprite_dma_dst;
	u8 m_sprite_dma_len;
	u8 m_sprite_dma_count;
	u8 m_sprite_dma_row;
	u8 m_sprite_dma_ctrl;
	emu_timer *m_sprite_dma_timer;
	u8 m_arena_start;
	u8 m_arena_end;
	u8 m_arena_ctrl;
	bool m_arena_irq_pending;
	emu_timer *m_arena_irq_timer;
	arena_state m_arena;
	video_standard m_video_standard;
	video_counters m_video_counters;
	u64 m_video_counter_ticks;
	emu_timer *m_video_counter_timer;
	u8 m_color_effect[3];
	u32 m_video_noise_state;
	u8 m_video_noise_feedback[3];
	u64 m_video_noise_ticks;
	u8 m_video_ctrl;
	emu_timer *m_raster_irq_timer;
	bitmap_ind16 m_pen_bitmap;
	u8 m_sprite_address_mode;
	int m_video_hres_scale;

private:
	rectangle m_raster_visible;
	TIMER_CALLBACK_MEMBER(raster_vblank);
	emu_timer *m_vblank_start_timer;
	emu_timer *m_vblank_end_timer;
	static constexpr int ORIGINAL_PICTURE_DELAY_CLOCKS = 24;
	// Uncontended master clocks per byte; earlier-chip timing remains unmeasured.
	static constexpr unsigned SPRITE_DMA_BYTE_CLOCKS = 2;
	static const gfx_layout TILE_16X16_2BPP_LAYOUT;

	static constexpr std::array<u8, TILEMAP_REGISTER_COUNT> TILEMAP_REGISTER_MASK =
			{ 0x7f, 0x7f, 0x7f, 0xfe, 0xff, 0xff, 0xff, 0x9f };
	static const std::array<std::array<double, 2>, 48> HUE_IQ;

	struct tile_data
	{
		offs_t address;
		int bpp;
		int width;
		int height;
		int palette;
		int priority;
		pixel_layout layout;
	};

	struct sprite_data
	{
		tile_data tile;
		int x;
		int y;
	};

	enum class graphics_source { SPRITE, TILEMAP };

	std::pair<int, int> fixed_raster_offset(int delay_clocks) const;
	void pixel_decoder_ctrl_w(u8 data);
	void draw_output(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_tilemap(const rectangle &cliprect, int layer);
	u32 graphics_address(graphics_source source, u8 mode, u32 entry, unsigned tile_bytes) const;
	void decode_inline_header(tile_data &tile);
	void draw_sprites(const rectangle &cliprect);
	void draw_tile_line(const rectangle &cliprect, const tile_data &tile, int xpos, int ypos, int line, int xoffset);

	required_memory_region m_gfx_region;
	std::unique_ptr<pen_t[]> m_color_effect_palette;
	bool m_color_effect_palette_valid;
	std::array<s16, PALETTE_BANK_ENTRIES / 16> m_transparent_pens;
};

class xavix2000_video_device : public xavix_video_device
{
public:
	xavix2000_video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual std::pair<int, int> picture_offset() const override;
};

class superxavix_video_device : public xavix_video_device
{
public:
	superxavix_video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	static void configure_screen(screen_device &screen, u32 master_clock, video_standard standard) ATTR_COLD;
	void decode_cartridge_graphics(memory_region &region) { decode_graphics(&region); }
	template <typename... T> void set_graphics_read_callback(T &&... args) { m_graphics_read_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_bitmap_read_callback(T &&... args) { m_bitmap_read_cb.set(std::forward<T>(args)...); }

	virtual void map(address_map &map) override ATTR_COLD;
	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
	// The picture NMI is timed independently of the presentation screen.
	virtual void vblank() override { }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// xavix_video_device implementation
	virtual u16 palette_data(int pen) const override;
	virtual void set_palette_data(int pen, u16 data) override;
	virtual void reset_video_timing() override;
	virtual rectangle picture_area() const override;
	virtual void raster_end() override;
	virtual counter_config counter_parameters() const override;
	virtual void schedule_picture_irqs() override;
	virtual void schedule_raster_irq() override;
	virtual auto decode_pixel_layout(u8 flags) const -> pixel_layout override;
	virtual std::pair<int, int> counter_offset() const override;
	virtual std::pair<int, int> picture_offset() const override;
	virtual std::pair<int, int> render_offset() const override;
	virtual device_delegate<u8 (u32)> &tile_read_callback() override;
	virtual u32 graphics_segment_base(unsigned segment) const override;
	virtual u32 extended_graphics_segment_base(unsigned segment) const override;
	virtual void mix_bitmap_layer(screen_device &screen, const rectangle &cliprect) override;
	virtual u8 video_status_r() override;

private:
	static constexpr int OUTPUT_PIXEL_CLOCKS = 2;
	static constexpr int STREAM_ORIGIN_ALIGNMENT_CLOCKS = PICTURE_PIXEL_CLOCKS;
	static constexpr int PAIRED_PIXEL_CLOCKS = PICTURE_PIXEL_CLOCKS / 2;
	static constexpr int MAX_ACTIVE_WIDTH = TILEMAP_WRAP_WIDTH * PICTURE_PIXEL_CLOCKS / OUTPUT_PIXEL_CLOCKS;
	static constexpr int BITMAP_OUTPUT_DELAY_PIXELS = 2;
	// TODO: Verify the standard-dependent bitmap counter origins on hardware.
	static constexpr int BITMAP_VERTICAL_ORIGIN_NTSC = 12;
	static constexpr int BITMAP_VERTICAL_ORIGIN_PAL = 39;
	static const std::array<u8, 0x1a> TIMING_MASK;
	static const std::array<u8, 8> POSITION_MASK;
	static const std::array<u8, 0x1a> TIMING_NTSC_DEFAULT;
	static const std::array<u8, 0x1a> TIMING_PAL_DEFAULT;

	enum class palette_bank { GRAPHICS = 0, BITMAP = 1 };
	enum class palette_component { HUE, SATURATION, LIGHTNESS };

	struct palette_port_state
	{
		u8 index = 0;
		u16 color_latch = 0;
	};

	struct bitmap_row
	{
		u32 offset = 0;
		bool enabled = false;
	};

	enum bitmap_register : unsigned
	{
		BITMAP_TOP = 0x00,
		BITMAP_BOTTOM = 0x02,
		BITMAP_LEFT = 0x04,
		BITMAP_RIGHT = 0x06,
		BITMAP_ADDRESS_TOP = 0x08,
		BITMAP_ADDRESS_LEFT = 0x0c,
		BITMAP_ADDRESS_RIGHT = 0x0e,
		BITMAP_BASE = 0x10,
		BITMAP_STRIDE = 0x12,
		BITMAP_RESOLUTION = 0x13,
		BITMAP_CTRL = 0x14
	};

	enum timing_register : unsigned
	{
		TIMING_HORIZONTAL_TOTAL = 0x00,
		TIMING_FIELD_LEFT = 0x0a,
		TIMING_FIELD_RIGHT = 0x0e,
		TIMING_HALF_LINES = 0x10,
		TIMING_FIELD_BOTTOM = 0x12,
		TIMING_FIELD_TOP = 0x19
	};

	struct video_timing
	{
		u16 horizontal_total = 0;
		u16 half_lines = 0;
		u16 active_width = 0;
		u16 active_height = 0;
		u16 horizontal_start = 0;
		u16 vertical_origin = 0;

		bool operator==(video_timing const &) const = default;
		int width() const { return (horizontal_total + OUTPUT_PIXEL_CLOCKS - 1) / OUTPUT_PIXEL_CLOCKS; }
		int height() const { return (half_lines + 1) / 2; }
		rectangle visible_area() const;
	};

	template <palette_plane Plane> u8 graphics_palram_r(offs_t offset);
	template <palette_plane Plane> u8 bitmap_palram_r(offs_t offset);
	template <palette_plane Plane> void bitmap_palram_w(offs_t offset, u8 data);
	void video_timing_regs_w(offs_t offset, u8 data);
	static video_timing calculate_video_timing(u8 const *registers, int output_sample_phase_clocks);
	video_timing calculate_video_timing() const;
	void apply_video_timing();
	TIMER_CALLBACK_MEMBER(picture_nmi);
	TIMER_CALLBACK_MEMBER(video_field_start);
	void bitmap_params_w(offs_t offset, u8 data);
	void update_bitmap_rows(int scanline);
	void advance_bitmap_row();
	bool horizontal_reload_in_blanking(counter_config const &config) const;
	int output_sample_phase_clocks() const;
	void video_position_regs_w(offs_t offset, u8 data);
	void extended_segment_regs_w(offs_t offset, u8 data);
	template <palette_bank Bank> u8 palette_index_r();
	template <palette_bank Bank> void palette_index_w(u8 data);
	template <palette_bank Bank, palette_component Component> u8 palette_component_r();
	template <palette_bank Bank, palette_component Component> void palette_component_w(u8 data);

	device_delegate<u8 (u32)> m_graphics_read_cb;
	device_delegate<u8 (u32)> m_bitmap_read_cb;
	required_shared_ptr<u8> m_extended_segment_regs;
	required_shared_ptr<u8> m_video_timing_regs;
	required_shared_ptr<u8> m_video_position_regs;
	required_shared_ptr<u8> m_bitmap_palram_sh;
	required_shared_ptr<u8> m_bitmap_palram_l;
	required_shared_ptr<u8> m_bitmap_params;

	std::array<palette_port_state, 2> m_palette_ports;
	u8 m_bitmap_palette_read_latch;
	s16 m_bitmap_scanline;
	s16 m_bitmap_reference;
	u16 m_bitmap_position;
	u32 m_bitmap_source_offset;
	bool m_bitmap_vertical_gate;
	bool m_bitmap_rendered;
	std::array<bitmap_row, 0x200> m_bitmap_rows;
	video_timing m_video_timing;
	bitmap_rgb32 m_output_bitmap;
	emu_timer *m_picture_nmi_timer;
	emu_timer *m_video_field_timer;
	bool m_video_field_phase;
	u8 m_graphics_mode;
};

DECLARE_DEVICE_TYPE(XAVIX_VIDEO, xavix_video_device)
DECLARE_DEVICE_TYPE(XAVIX2000_VIDEO, xavix2000_video_device)
DECLARE_DEVICE_TYPE(SUPERXAVIX_VIDEO, superxavix_video_device)

#endif // MAME_MACHINE_XAVIX_VIDEO_H

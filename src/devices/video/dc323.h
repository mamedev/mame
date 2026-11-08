// license:BSD-3-Clause
// copyright-holders:Eric Harley
/***************************************************************************

    Digital Equipment Corporation DC323 Adder

    Models the Adder command engine, processor/bitmap transfers, retained
    per-plane Viper state, bitmap memory, and raster-synchronous scrolling. The
    Viper datapaths remain candidates for a later, separate extraction.

***************************************************************************/

#ifndef MAME_VIDEO_DC323_H
#define MAME_VIDEO_DC323_H

#pragma once


class dc323_device : public device_t
{
public:
	enum register_number : u8
	{
		REG_INTERRUPT_ENABLE = 0x02,
		REG_STATUS = 0x03,
		REG_ID_DATA = 0x07,
		REG_COMMAND = 0x08,
		REG_X_CONTROL = 0x09,
		REG_SCROLL_X_MIN = 0x0e,
		REG_SCROLL_X_MAX = 0x0f,
		REG_SCROLL_Y_MIN = 0x10,
		REG_SCROLL_Y_MAX = 0x11,
		REG_PAUSE = 0x12,
		REG_SCROLL_Y_OFFSET = 0x13,
		REG_SCROLL_Y = 0x14,
		REG_PENDING_X_INDEX = 0x15,
		REG_PENDING_Y_INDEX = 0x16,
		REG_NEW_X_INDEX = 0x17,
		REG_NEW_Y_INDEX = 0x18,
		REG_OLD_X_INDEX = 0x19,
		REG_OLD_Y_INDEX = 0x1a,
		REG_SOURCE_FAST_DX = 0x20,
		REG_SOURCE_SLOW_DY = 0x21,
		REG_SOURCE_X = 0x22,
		REG_SOURCE_Y = 0x23,
		REG_DESTINATION_X = 0x24,
		REG_DESTINATION_Y = 0x25,
		REG_WIDTH = 0x26,
		REG_DESTINATION_FAST_DY = 0x27,
		REG_DESTINATION_SLOW_DX = 0x28,
		REG_HEIGHT = 0x29,
		REG_FAST_SCALE = 0x2a,
		REG_SLOW_SCALE = 0x2b,
		REG_SOURCE2_X = 0x2c,
		REG_SOURCE2_Y = 0x2d,
		REG_SOURCE2_SIZE = 0x2e,
		REG_ERROR_1 = 0x2f,
		REG_ERROR_2 = 0x30,
		REG_Y_LIMIT = 0x37
	};

	enum status_bit : u16
	{
		STATUS_PAUSE_COMPLETE = 0x0001,
		STATUS_SCROLL_SERVICE = 0x0002,
		STATUS_RASTEROP_INITIALIZED = 0x0004,
		STATUS_RASTEROP_COMPLETE = 0x0008,
		STATUS_ADDRESS_COMPLETE = 0x0010,
		STATUS_ID_DATA_READY = 0x0020,
		STATUS_ID_DATA_TRANSMIT_READY = 0x0040,
		STATUS_RASTEROP_MASK = STATUS_RASTEROP_INITIALIZED |
							   STATUS_RASTEROP_COMPLETE |
							   STATUS_ADDRESS_COMPLETE,
		STATUS_COMMAND_MASK = STATUS_RASTEROP_MASK | STATUS_ID_DATA_READY
	};

	static constexpr unsigned BITMAP_WIDTH = 1024;
	static constexpr unsigned BITMAP_HEIGHT = 2048;
	static constexpr unsigned BITMAP_PLANES = 4;
	static constexpr unsigned BITMAP_PLANE_SIZE = BITMAP_WIDTH * BITMAP_HEIGHT;

	dc323_device(machine_config const &mconfig, char const *tag,
				 device_t *owner, u32 clock);

	// Bind the level-sensitive interrupt output.
	auto irq_handler() { return m_irq_handler.bind(); }
	// Bind the current display-raster input used for old/new index selection.
	auto raster_line_handler() { return m_raster_line_handler.bind(); }
	// Select the ROM region that supplies E49 character-generator data.
	void set_chargen_tag(char const *tag) { m_chargen_rom.set_tag(tag); }
	// Select the shared storage that represents the four bitmap planes.
	void set_bitmap_tag(char const *tag) { m_bitmap_planes.set_tag(tag); }

	u8 read(offs_t offset);
	void write(offs_t offset, u16 data);
	void operational_w(int state);
	void viper_select_w(u8 data);
	void frame_event();
	void scanline_event(u16 line);

	// Board compatibility and video-backend accessors.  These expose state,
	// but never CPU identity or firmware addresses, so policy remains outside.
	// Read a retained Adder register without host-bus side effects.
	u16 register_r(offs_t offset) const
	{
		return m_registers[offset & REGISTER_MASK];
	}
	// Read the internal status latch without operational gating.
	u16 status_r() const { return m_status; }
	// Report whether firmware has queued a scroll for the next frame boundary.
	bool scroll_pending() const { return m_scroll_pending; }
	// Read the active four-plane Viper selection mask.
	u8 viper_select_r() const { return m_viper_select; }
	// Read retained state from one Viper register bank.
	u16 viper_register_r(unsigned plane, unsigned reg) const
	{
		return m_viper_registers[plane][reg & VIPER_REGISTER_MASK];
	}
	u8 pixel_r(unsigned plane, unsigned x, unsigned y) const;
	u8 display_pixel_r(unsigned plane, unsigned x, unsigned y) const;
	// Consume the accumulated bitmap/view-change notification used by a host
	// display backend. Command timing and raster synchronization remain local.
	bool consume_display_dirty();
	void bitmap_clear(u8 data = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	struct raster_point
	{
		int x;
		int y;
	};

	struct axis_sample
	{
		unsigned destination;
		unsigned source;
	};

	struct source2_format
	{
		bool tiled;
		unsigned width;
		unsigned height;
	};

	enum : u8
	{
		VIPER_REG_BUS_WIDTH = 0x01,
		VIPER_REG_Z_ADDRESS = 0x03,
		VIPER_REG_LOGIC_BASE = 0x04,
		VIPER_REG_MASK_1 = 0x08,
		VIPER_REG_MASK_2 = 0x09,
		VIPER_REG_SOURCE = 0x0a,
		VIPER_REG_FILL = 0x0b,
		VIPER_REG_SCROLL_LEFT = 0x0c,
		VIPER_REG_SCROLL_RIGHT = 0x0d,
		VIPER_REG_BACKGROUND = 0x0e,
		VIPER_REG_FOREGROUND = 0x0f,
		VIPER_REG_CSR_BASE = 0x10
	};
	enum : u8
	{
		COMMAND_RASTER_OPERATION = 2,
		COMMAND_PROCESSOR_TRANSFER = 3
	};
	enum : u8
	{
		X_MODE_NORMAL = 0,
		X_MODE_LINEAR_PATTERN = 2,
		X_MODE_FILL = 3
	};
	static constexpr unsigned REGISTER_MASK = 0x3f;
	static constexpr unsigned VIPER_REGISTER_MASK = 0x1f;
	static constexpr u16 COORDINATE_MASK = 0x3fff;
	static constexpr u16 FIELD_13_BIT_MASK = 0x1fff;
	static constexpr u16 SCROLL_X_MASK = 0x3ffc;
	static constexpr u16 UNITY_SCALE = 0x1fff;
	static constexpr u16 Z_ADDRESS_MASK = 0x003f;
	static constexpr unsigned Z_BLOCK_SHIFT = 4;

	// Convert plane and physical X/Y coordinates to a linear bitmap address.
	static constexpr u32 bitmap_plane_address(unsigned plane, unsigned x,
											  unsigned y)
	{
		return plane * BITMAP_PLANE_SIZE + y * BITMAP_WIDTH + x;
	}
	// Apply PYOF within the refresh ring before forming a bitmap address.
	u32 device_bitmap_plane_address(unsigned plane, unsigned x, int y) const
	{
		// PYOF translates coordinates in the display portion of bitmap memory.
		// Higher addresses are independent backing/scratch storage and must not
		// alias into the refresh ring at YL.
		int const memory_y =
				(y >= 0 && y < int(m_active_y_limit))
						? (y + int(m_active_y_offset)) % int(m_active_y_limit)
						: y;
		return bitmap_plane_address(plane, x, memory_y);
	}
	// Test coordinates against the full physical bitmap allocation.
	static constexpr bool bitmap_contains(int x, int y)
	{
		return (unsigned(x) < BITMAP_WIDTH) && (unsigned(y) < BITMAP_HEIGHT);
	}

	// Raster geometry and register decoding.
	static int signed_14(u16 value);
	static unsigned viper_bus_width(u16 value);
	static int source2_coordinate(u16 origin, int coordinate,
								  unsigned tile_size, bool tiled);
	source2_format source2_format_r() const;
	static std::vector<axis_sample> scaled_axis(unsigned destination_length,
												u16 scale, unsigned &source,
												unsigned &accumulator);
	static std::vector<axis_sample> scaled_axis(unsigned destination_length,
												u16 scale);
	static std::vector<raster_point>
	raster_vector(int dx, int dy, int error_adjustment, unsigned limit);
	std::pair<int, int> raster_index(int unindexed_y);

	// Processor/bitmap transfer data path and its six-word I/D FIFO.
	u8 id_data_r();
	void id_data_w(u16 data);
	u16 pbt_read_word(u32 index);
	void pbt_write_word(u32 index, u16 data);
	void fifo_clear();
	bool fifo_push(u16 data);
	u16 fifo_pop();
	void update_fifo_status();
	void schedule_pbt_service();

	// Command lifecycle and interrupt delivery.
	void stop_command_activity();
	TIMER_CALLBACK_MEMBER(command_initialize_tick);
	TIMER_CALLBACK_MEMBER(command_complete_tick);
	TIMER_CALLBACK_MEMBER(pbt_service_tick);
	void command_w(u16 command);
	void unsupported_transfer(u16 command);
	void update_interrupt(char const *event);

	// Raster command execution and Viper source routing.
	void execute_rasterop(u16 command);
	void execute_linear_pattern(u16 command);
	void execute_fill(u16 command);
	void route_source_cycle(unsigned csr, unsigned destination_x,
							std::array<bool, BITMAP_PLANES> const &source_bits);
	void load_viper_bit(unsigned plane, unsigned target, unsigned word_bit,
						bool data);
	void write_viper_pixel(unsigned plane, unsigned x, unsigned y, u16 command,
						   bool source_bit);

	// Raster-synchronous scrolling and display address translation.
	bool scroll_mask_allows(unsigned plane, u16 x) const;
	void begin_scroll();
	void finish_scroll();
	void execute_scroll_line(u16 device_y);
	void execute_down_scroll(u16 distance);

	required_region_ptr<u8> m_chargen_rom;
	memory_share_creator<u8> m_bitmap_planes;
	devcb_write_line m_irq_handler;
	devcb_read16 m_raster_line_handler;

	std::array<u16, 0x40> m_registers{};
	std::array<std::array<u16, 0x20>, BITMAP_PLANES> m_viper_registers{};
	u8 m_viper_select = 0;
	u16 m_status = 0;
	// The board release input gates command/status activity.
	bool m_operational = false;
	u16 m_active_pause = 0;
	u8 m_pause_events = 0;
	bool m_pause_fired = false;
	bool m_scroll_pending = false;
	bool m_scroll_active = false;
	u16 m_active_scroll = 0;
	u16 m_active_scroll_x_min = 0;
	u16 m_active_scroll_x_max = 0;
	u16 m_active_scroll_y_min = 0;
	u16 m_active_scroll_y_max = 0;
	u16 m_active_scroll_y_offset = 0;
	u16 m_active_y_offset = 0;
	u16 m_active_y_limit = BITMAP_HEIGHT;
	u16 m_active_scroll_line = 0;
	std::array<u16, BITMAP_PLANES> m_active_scroll_fill{};
	std::array<u16, BITMAP_PLANES> m_active_scroll_left{};
	std::array<u16, BITMAP_PLANES> m_active_scroll_right{};
	std::array<u8, BITMAP_PLANES> m_active_scroll_word_width{};
	bool m_index_phase_active = false;
	bool m_pbt_read_active = false;
	bool m_pbt_read_z_mode = false;
	u16 m_pbt_read_command = 0;
	u32 m_pbt_read_index = 0;
	u32 m_pbt_read_count = 0;
	bool m_pbt_write_active = false;
	bool m_pbt_write_z_mode = false;
	u16 m_pbt_write_command = 0;
	u16 m_pbt_write_bus_width = 0;
	u16 m_pbt_write_words_per_row = 0;
	u32 m_pbt_write_index = 0;
	u32 m_pbt_write_accepted = 0;
	u32 m_pbt_write_count = 0;
	std::array<u16, 6> m_id_fifo{};
	u8 m_id_fifo_head = 0;
	u8 m_id_fifo_tail = 0;
	u8 m_id_fifo_count = 0;
	emu_timer *m_command_initialize_timer = nullptr;
	emu_timer *m_command_complete_timer = nullptr;
	emu_timer *m_pbt_service_timer = nullptr;
	u8 m_diagnostic_data = 0;
	bool m_raster_diagnostic = false;
	u16 m_linear_pattern_slow_position = 0;
	u16 m_linear_pattern_slow_accumulator = 0;
	bool m_fill_position_valid = false;
	s32 m_fill_a_x = 0;
	s32 m_fill_a_y = 0;
	s32 m_fill_b_x = 0;
	s32 m_fill_b_y = 0;
	bool m_fill_a_vector_valid = false;
	bool m_fill_b_vector_valid = false;
	bool m_fill_a_vector_dirty = false;
	bool m_fill_b_vector_dirty = false;
	s32 m_fill_a_origin_x = 0;
	s32 m_fill_a_origin_y = 0;
	s32 m_fill_b_origin_x = 0;
	s32 m_fill_b_origin_y = 0;
	s32 m_fill_a_dx = 0;
	s32 m_fill_a_dy = 0;
	s32 m_fill_b_dx = 0;
	s32 m_fill_b_dy = 0;
	s32 m_fill_a_error = 0;
	s32 m_fill_b_error = 0;
	u32 m_fill_a_progress = 0;
	u32 m_fill_b_progress = 0;
	bool m_irq_state = false;
	bool m_display_dirty = true;
	u32 m_log_events = 0;
};


DECLARE_DEVICE_TYPE(DC323, dc323_device)

#endif // MAME_VIDEO_DC323_H

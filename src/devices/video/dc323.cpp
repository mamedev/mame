// license:BSD-3-Clause
// copyright-holders:Eric Harley
/***************************************************************************

    Digital Equipment Corporation DC323 Adder

    The DC323 (78690, "Adder") is the common address and control processor in
    DEC's Adder/Viper bitmap architecture.  It generates screen-refresh and
    update addresses, executes raster geometry, controls processor/bitmap
    transfers, and coordinates smooth scrolling.  Plane-local masking, source
    routing, Boolean operations, and color selection are performed by Viper
    processors.  The VT340 control module contains one DC323-CA Adder II at E70
    and four DC322 Vipers at E85 through E88, one for each bitmap plane.

    This device models the host register interface, command/status timing and
    interrupts, the six-entry I/D FIFO, X- and Z-mode processor transfers,
    normal/linear-pattern/fill raster operations, retained per-plane Viper
    state, four-plane bitmap memory, and raster-synchronous scrolling.  Video
    timing remains external and enters through the frame and scanline events.

    Exact memory-cycle arbitration and some Viper control-store, pipeline, and
    edge-mask behaviour remain approximate.

    Implementation status

    Implemented:
      * Register/status interface, command abort, and per-plane Viper state
      * Normal constant, bitmap, and E49 character-ROM raster sources
      * Linear patterns, Source 2 routing, and two-edge/chained polygon fills
      * X- and Z-mode processor/bitmap transfers with a six-word I/D FIFO
      * Raster indexing, smooth scrolling, bitmap planes, and display access

    Partial:
      * Hole-fill/complement edge cases and exact fill-vertex corrections
      * Viper word-pipeline delays, multiple-driver resolution, and arbitration
      * Transfer cycle cadence, overlapping traversal, and scroll queue timing
      * Low-resolution Z subplanes and the board-inaccessible upper I/D byte

    Deliberate boundaries:
      * Reserved/illegal raster encodings do not draw and may be diagnosed
      * Unidentified operation-3 source/destination forms are logged only;
        no transfer results or completion semantics are invented

    Keep unsupported command boundaries explicit until documentation,
    firmware traces, or physical-hardware comparisons establish semantics.

    Raster programming and pixel flow

    CPU2 first loads geometry and source state, then writes register 08 to
    launch the operation.  WIDTH and HEIGHT are conventional names used by
    rectangular operations; in the Adder geometry they are also the X part of
    the destination fast vector and Y part of its slow vector respectively.

       Adder register group                 Function
      +---------------------+----------------------------------------------+
      | 08 COMMAND          | operation, D/S1/S2 enables, logic selection |
      | 09 X CONTROL        | mode, pen, hole fill, source/dest indexing   |
      | 20-23 SOURCE        | pattern vector and Source 1 origin           |
      | 24-29 DESTINATION   | origin, fast vector, and slow vector         |
      | 2A-2B SCALE         | fast/slow source sampling rate               |
      | 2C-2E SOURCE 2      | origin, tiling enable, and tile dimensions   |
      | 2F-30 ERROR 1/2     | slow/fast Bresenham initial-error offsets    |
      | 15-1A INDEX         | pending/new/old display translation          |
      +---------------------+----------------------------------------------+
                         |
                         | write COMMAND (08)
                         v
      +---------------------- DC323 geometry -----------------------+
      | destination origin (24,25)                                  |
      |   + fast vector (26,27), ERROR 2 (30), FAST SCALE (2A)      |
      |   + slow vector (28,29), ERROR 1 (2F), SLOW SCALE (2B)      |
      |   + optional destination index selected by X CONTROL (09)   |
      +----------------------------+--------------------------------+
                                   | one destination coordinate
                 +-----------------+------------------+
                 |                                    |
                 v                                    v
      +--------------------------+       +--------------------------+
      | Source 1                 |       | Source 2                 |
      | origin 22/23             |       | origin/format 2C-2E      |
      | bitmap or E49 char ROM   |       | bitmap/tiled pattern     |
      | optional source index    |       | destination-relative     |
      +-------------+------------+       +-------------+------------+
                    +--------------------+-------------+
                                         |
                                         v
      +-------------------- each selected DC322 Viper -------------------+
      | CSR 10-17 routes sources -> SOURCE 0A / MASK 1 08 / MASK 2 09   |
      | LOGIC 04-07 combines source and old destination under the masks  |
      | BACKGROUND 0E / FOREGROUND 0F supplies the resulting plane bit   |
      +-------------------------------+----------------------------------+
                                      | write one bit in each plane
                                      v
                bitmap planes 0-3 -> color index -> E51/E52 palette

    X CONTROL mode 00 walks the fast and slow destination vectors as a
    raster rectangle.  Source 1 may be a bitmap or E49 character-ROM sample;
    with neither bitmap source enabled, the retained Viper SOURCE register is
    used instead.  Source 2 is addressed relative to the unindexed destination
    and may repeat as a power-of-two tile.

    Mode 10 uses the same destination geometry but walks and scales a retained
    Source 1 pattern; its slow position and fractional phase survive command
    boundaries.  Mode 11 interprets the destination slow vector as polygon
    edge A and the Source 1 vector registers as edge B, then writes inclusive
    spans between them.  Operation 3 bypasses ordinary raster dispatch and
    moves packed X- or Z-mode data through register 07 and the six-word FIFO.

                              VT340 board

        CPU2 / host bus                          DC7051 timing
    registers, commands, I/D data              frame and scanline
              |                                       |
              |                                       |
              v                                       v
    +--------------------------------------------------------------+
    |                     dc323_device boundary                    |
    |                                                              |
    |  +------------------- DC323 Adder ----------------------+    |
    |  | register/status interface     command timers and IRQ |    |
    |  | six-word I/D FIFO and PBT     raster/address/scroll  |    |
    |  +--------------------------+----------------------------+   |
    |                             |                                |
    |          +-------------+-------------+-------------+         |
    |          |             |             |             |         |
    |     +----v----+   +----v----+   +----v----+   +----v----+    |
    |     | DC322   |   | DC322   |   | DC322   |   | DC322   |    |
    |     | Viper 0 |   | Viper 1 |   | Viper 2 |   | Viper 3 |    |
    |     +----+----+   +----+----+   +----+----+   +----+----+    |
    |          |             |             |             |         |
    |      bitmap 0      bitmap 1      bitmap 2      bitmap 3      |
    +----------+-------------+-------------+-------------+---------+
               +-------------+-------------+-------------+
                                  | four-bit pixel index
                                  v
                 +---------------------------------------+
                 |    16-entry, 12-bit RGB output map    |
                 | E51 74AS870: 4-bit red                |
                 | E52 74AS870: 4-bit green + 4-bit blue |
                 +-----------------+---------------------+
                                   |
                                   v
                               video / CRT

    References:

    * Digital Semiconductor Databook 1986, 78690 Video Control (Adder)
      and 78660/78680 Video Processor (Viper)
      https://bitsavers.org/pdf/dec/semiconductor/
      1986_Digital_Semiconductor_Databook.pdf

    * Dragon Video System Hardware Specification, revision 4.2, 25 June 1985
      https://archive.computerhistory.org/resources/access/text/2021/02/
      102739869-05-01-acc.pdf

    * VT340 Field Maintenance Print Set, MP-02412-01, February 1987
      https://bitsavers.org/pdf/dec/terminal/vt340/
      MP02412_VT340_EngrDrws_Feb87.pdf

***************************************************************************/

#include "emu.h"
#include "dc323.h"

#define LOG_REGISTERS  (1U << 1)
#define LOG_COMMANDS   (1U << 2)
#define LOG_SCROLL     (1U << 3)
#define LOG_INTERRUPTS (1U << 4)
#define LOG_TRANSFERS  (1U << 5)
#define LOG_RASTEROPS  (1U << 6)

#define VERBOSE (0)
#include "logmacro.h"

// A deliberately enabled diagnostic build must still remain usable during a
// long terminal session.  Emit one truncation notice after this many events.
#define DC323_LOG_LIMIT 100000U
#define DC323_LOGMASKED(mask, ...) \
	do { \
		if (VERBOSE & (mask)) \
		{ \
			if (m_log_events < DC323_LOG_LIMIT) \
				(LOG_OUTPUT_FUNC)(__VA_ARGS__); \
			else if (m_log_events == DC323_LOG_LIMIT) \
				(LOG_OUTPUT_FUNC)( \
						"%s %s: diagnostic log limit reached\n", \
						machine().time().to_string(), tag()); \
			if (m_log_events <= DC323_LOG_LIMIT) \
				++m_log_events; \
		} \
	} while (false)


DEFINE_DEVICE_TYPE(DC323, dc323_device, "dc323", "DEC DC323 Adder")


// Resolve the character ROM, bitmap share, and board-facing callbacks.
dc323_device::dc323_device(machine_config const &mconfig, char const *tag,
						   device_t *owner, u32 clock) :
	device_t(mconfig, DC323, tag, owner, clock),
	m_chargen_rom(*this, finder_base::DUMMY_TAG),
	m_bitmap_planes(*this, "bitmap_planes", BITMAP_PLANES * BITMAP_PLANE_SIZE,
					ENDIANNESS_LITTLE),
	m_irq_handler(*this),
	m_raster_line_handler(*this, 0)
{
}


// Allocate asynchronous command timers and register all mutable device state.
void dc323_device::device_start()
{
	m_command_initialize_timer =
			timer_alloc(FUNC(dc323_device::command_initialize_tick), this);
	m_command_complete_timer =
			timer_alloc(FUNC(dc323_device::command_complete_tick), this);
	m_pbt_service_timer =
			timer_alloc(FUNC(dc323_device::pbt_service_tick), this);
	save_item(NAME(m_registers));
	save_item(NAME(m_viper_registers));
	save_item(NAME(m_viper_select));
	save_item(NAME(m_status));
	save_item(NAME(m_operational));
	save_item(NAME(m_active_pause));
	save_item(NAME(m_pause_events));
	save_item(NAME(m_pause_fired));
	save_item(NAME(m_scroll_pending));
	save_item(NAME(m_scroll_active));
	save_item(NAME(m_active_scroll));
	save_item(NAME(m_active_scroll_x_min));
	save_item(NAME(m_active_scroll_x_max));
	save_item(NAME(m_active_scroll_y_min));
	save_item(NAME(m_active_scroll_y_max));
	save_item(NAME(m_active_scroll_y_offset));
	save_item(NAME(m_active_y_offset));
	save_item(NAME(m_active_y_limit));
	save_item(NAME(m_active_scroll_line));
	save_item(NAME(m_active_scroll_fill));
	save_item(NAME(m_active_scroll_left));
	save_item(NAME(m_active_scroll_right));
	save_item(NAME(m_active_scroll_word_width));
	save_item(NAME(m_index_phase_active));
	save_item(NAME(m_pbt_read_active));
	save_item(NAME(m_pbt_read_z_mode));
	save_item(NAME(m_pbt_read_command));
	save_item(NAME(m_pbt_read_index));
	save_item(NAME(m_pbt_read_count));
	save_item(NAME(m_pbt_write_active));
	save_item(NAME(m_pbt_write_z_mode));
	save_item(NAME(m_pbt_write_command));
	save_item(NAME(m_pbt_write_bus_width));
	save_item(NAME(m_pbt_write_words_per_row));
	save_item(NAME(m_pbt_write_index));
	save_item(NAME(m_pbt_write_accepted));
	save_item(NAME(m_pbt_write_count));
	save_item(NAME(m_id_fifo));
	save_item(NAME(m_id_fifo_head));
	save_item(NAME(m_id_fifo_tail));
	save_item(NAME(m_id_fifo_count));
	save_item(NAME(m_diagnostic_data));
	save_item(NAME(m_raster_diagnostic));
	save_item(NAME(m_linear_pattern_slow_position));
	save_item(NAME(m_linear_pattern_slow_accumulator));
	save_item(NAME(m_fill_position_valid));
	save_item(NAME(m_fill_a_x));
	save_item(NAME(m_fill_a_y));
	save_item(NAME(m_fill_b_x));
	save_item(NAME(m_fill_b_y));
	save_item(NAME(m_fill_a_vector_valid));
	save_item(NAME(m_fill_b_vector_valid));
	save_item(NAME(m_fill_a_vector_dirty));
	save_item(NAME(m_fill_b_vector_dirty));
	save_item(NAME(m_fill_a_origin_x));
	save_item(NAME(m_fill_a_origin_y));
	save_item(NAME(m_fill_b_origin_x));
	save_item(NAME(m_fill_b_origin_y));
	save_item(NAME(m_fill_a_dx));
	save_item(NAME(m_fill_a_dy));
	save_item(NAME(m_fill_b_dx));
	save_item(NAME(m_fill_b_dy));
	save_item(NAME(m_fill_a_error));
	save_item(NAME(m_fill_b_error));
	save_item(NAME(m_fill_a_progress));
	save_item(NAME(m_fill_b_progress));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_display_dirty));
}


// Return the command engine, raster state, and bitmap memory to power-on state.
void dc323_device::device_reset()
{
	m_registers.fill(0);
	for (auto &registers : m_viper_registers)
		registers.fill(0);
	m_viper_select = 0;
	m_status = 0;
	m_operational = false;
	m_active_pause = 0;
	m_pause_events = 0;
	m_pause_fired = false;
	m_scroll_pending = false;
	m_scroll_active = false;
	m_active_scroll = 0;
	m_active_scroll_x_min = 0;
	m_active_scroll_x_max = 0;
	m_active_scroll_y_min = 0;
	m_active_scroll_y_max = 0;
	m_active_scroll_y_offset = 0;
	m_active_y_offset = 0;
	m_active_y_limit = BITMAP_HEIGHT;
	m_active_scroll_line = 0;
	m_active_scroll_fill.fill(0);
	m_active_scroll_left.fill(0);
	m_active_scroll_right.fill(0);
	m_active_scroll_word_width.fill(0);
	m_index_phase_active = false;
	m_pbt_read_active = false;
	m_pbt_read_z_mode = false;
	m_pbt_read_command = 0;
	m_pbt_read_index = 0;
	m_pbt_read_count = 0;
	m_pbt_write_active = false;
	m_pbt_write_z_mode = false;
	m_pbt_write_command = 0;
	m_pbt_write_bus_width = 0;
	m_pbt_write_words_per_row = 0;
	m_pbt_write_index = 0;
	m_pbt_write_accepted = 0;
	m_pbt_write_count = 0;
	m_id_fifo.fill(0);
	m_id_fifo_head = 0;
	m_id_fifo_tail = 0;
	m_id_fifo_count = 0;
	m_diagnostic_data = 0;
	m_raster_diagnostic = false;
	m_linear_pattern_slow_position = 0;
	m_linear_pattern_slow_accumulator = 0;
	m_fill_position_valid = false;
	m_fill_a_x = 0;
	m_fill_a_y = 0;
	m_fill_b_x = 0;
	m_fill_b_y = 0;
	m_fill_a_vector_valid = false;
	m_fill_b_vector_valid = false;
	m_fill_a_vector_dirty = false;
	m_fill_b_vector_dirty = false;
	m_fill_a_origin_x = 0;
	m_fill_a_origin_y = 0;
	m_fill_b_origin_x = 0;
	m_fill_b_origin_y = 0;
	m_fill_a_dx = 0;
	m_fill_a_dy = 0;
	m_fill_b_dx = 0;
	m_fill_b_dy = 0;
	m_fill_a_error = 0;
	m_fill_b_error = 0;
	m_fill_a_progress = 0;
	m_fill_b_progress = 0;
	m_irq_state = false;
	m_display_dirty = true;
	m_log_events = 0;
	bitmap_clear();
	m_irq_handler(CLEAR_LINE);
}


// Read the byte-wide host interface, including status and transfer FIFO data.
u8 dc323_device::read(offs_t offset)
{
	unsigned const reg = offset & REGISTER_MASK;
	u8 data;
	if ((reg == REG_ID_DATA) && m_pbt_read_active)
		data = id_data_r();
	else if ((reg == REG_STATUS) && m_operational)
		data = u8(m_status);
	else
		data = u8(m_registers[reg]);

	DC323_LOGMASKED(LOG_REGISTERS,
					"%s %s: R %02X=%02X status=%04X enable=%04X\n",
					machine().time().to_string(), tag(), reg, data, m_status,
					m_registers[REG_INTERRUPT_ENABLE]);
	return data;
}


// Load a host register and apply any command, status, or scroll side effects.
void dc323_device::write(offs_t offset, u16 data)
{
	unsigned const reg = offset & REGISTER_MASK;
	u16 const old_data = m_registers[reg];
	DC323_LOGMASKED(LOG_REGISTERS,
					"%s %s: W %02X=%04X status=%04X enable=%04X\n",
					machine().time().to_string(), tag(), reg, data, m_status,
					m_registers[REG_INTERRUPT_ENABLE]);
	m_registers[reg] = data;
	if ((reg == REG_DESTINATION_X) || (reg == REG_DESTINATION_Y) ||
		((reg == REG_X_CONTROL) && ((old_data ^ data) & 0x000f)))
	{
		// Loading either destination origin starts a new fill position.  A
		// mode/axis/baseline change also terminates any active vector string.
		m_fill_position_valid = false;
	}
	if ((reg == REG_SOURCE_X) || (reg == REG_SOURCE_Y))
	{
		// In linear-pattern mode either source-origin component restarts both
		// the source slow-vector scan and its fractional scale phase.
		m_linear_pattern_slow_position = 0;
		m_linear_pattern_slow_accumulator = 0;
	}
	if ((reg == REG_DESTINATION_SLOW_DX) || (reg == REG_HEIGHT) ||
		(reg == REG_ERROR_1))
		m_fill_a_vector_dirty = true;
	if ((reg == REG_SOURCE_FAST_DX) || (reg == REG_SOURCE_SLOW_DY) ||
		(reg == REG_ERROR_2))
		m_fill_b_vector_dirty = true;
	if (reg == REG_ID_DATA)
	{
		// The board diagnostics use the DC323 I/D path as a retained test-data
		// source. Equal-byte words test ordinary bus retention; 00CF selects
		// the all-ones result and 00C6 selects the raster-address pattern
		// returned by diagnostic bitmap-to-processor command 0BC0.
		if (data == 0x00cf)
		{
			m_diagnostic_data = 0xff;
			m_raster_diagnostic = false;
		}
		else if (data == 0x00c6)
			m_raster_diagnostic = true;
		else if (u8(data >> 8) == u8(data))
		{
			m_diagnostic_data = u8(data);
			m_raster_diagnostic = false;
		}
		id_data_w(data);
	}

	if ((reg == REG_INTERRUPT_ENABLE) && m_operational)
		update_interrupt("interrupt enable write");
	if ((reg == REG_STATUS) && m_operational)
	{
		u16 const old_status = m_status;
		m_status &= data;
		// Pause complete is a two-event queue, not a simple sticky bit.  A
		// zero in bit zero acknowledges one queued raster crossing; the bit
		// remains asserted until both crossings have been acknowledged.
		if (!BIT(data, 0) && m_pause_events)
			--m_pause_events;
		if (m_pause_events)
			m_status |= STATUS_PAUSE_COMPLETE;
		DC323_LOGMASKED(
				LOG_INTERRUPTS,
				"%s %s: status clear data=%04X old=%04X new=%04X enable=%04X\n",
				machine().time().to_string(), tag(), data, old_status, m_status,
				m_registers[REG_INTERRUPT_ENABLE]);
		update_interrupt("status acknowledge");
	}
	if ((reg == REG_SCROLL_Y) && m_operational)
	{
		DC323_LOGMASKED(LOG_SCROLL,
						"%s %s: schedule scroll command=%04X pending=%u "
						"status=%04X enable=%04X\n",
						machine().time().to_string(), tag(), data,
						m_scroll_pending, m_status,
						m_registers[REG_INTERRUPT_ENABLE]);
		m_scroll_pending = true;
		m_display_dirty = true;
	}
	if (reg == REG_COMMAND)
		command_w(data);
}


// Gate command and interrupt activity with the board's operational input.
void dc323_device::operational_w(int state)
{
	bool const operational = bool(state);
	if (m_operational == operational)
		return;
	m_operational = operational;
	if (m_operational)
	{
		// Reset leaves the command and address engines idle.  Once the board
		// releases the DC323 for normal accesses, their three completion
		// outputs are therefore asserted until a command starts new work.  FIFO
		// receive ready remains clear until a bitmap-to-processor transfer
		// supplies data.
		m_status |= STATUS_RASTEROP_MASK;
	}
	DC323_LOGMASKED(LOG_REGISTERS, "%s %s: operational=%u\n",
					machine().time().to_string(), tag(), m_operational);
	update_interrupt("operational input");
}


// Select which of the four Viper planes participate in subsequent operations.
void dc323_device::viper_select_w(u8 data)
{
	u8 const select = data & 0x0f;
	if (m_viper_select == select)
		return;
	m_viper_select = select;
	DC323_LOGMASKED(LOG_REGISTERS, "%s %s: Viper select=%X\n",
					machine().time().to_string(), tag(), m_viper_select);
}


// Latch frame-buffered registers and start any pending raster-synchronous
// scroll.
void dc323_device::frame_event()
{
	// Video timing runs continuously, but reset holds the command/status logic
	// inactive.  Do not latch stale frame service before the board releases the
	// DC323 into operational mode.
	if (!m_operational)
		return;

	u16 const old_status = m_status;
	finish_scroll();
	bool const was_pending = m_scroll_pending;
	if (was_pending)
		m_display_dirty = true;
	// At the frame boundary the indexes for the region already moved become
	// the old indexes, and the scroll process's pending values become the new
	// indexes.  Upward scrolling then moves bitmap rows behind screen refresh,
	// allowing update rasters to select old/new indexes at the same boundary.
	m_registers[REG_OLD_X_INDEX] = m_registers[REG_NEW_X_INDEX];
	m_registers[REG_OLD_Y_INDEX] = m_registers[REG_NEW_Y_INDEX];
	m_registers[REG_NEW_X_INDEX] = m_registers[REG_PENDING_X_INDEX];
	m_registers[REG_NEW_Y_INDEX] = m_registers[REG_PENDING_Y_INDEX];
	// PYOF is the screen-to-memory offset and YL is the height of the refresh
	// portion of bitmap memory, including any extra down-scroll scanlines. Both
	// are frame-buffered.  A zero/unprogrammed YL retains the backing-store
	// extent for device tests and reset-time operation.
	u16 const old_y_offset = m_active_y_offset;
	u16 const old_y_limit = m_active_y_limit;
	u16 const programmed_y_limit = m_registers[REG_Y_LIMIT] & FIELD_13_BIT_MASK;
	m_active_y_limit = programmed_y_limit ? std::min<u16>(programmed_y_limit,
														  BITMAP_HEIGHT)
										  : BITMAP_HEIGHT;
	m_active_y_offset = (m_registers[REG_SCROLL_Y_OFFSET] & FIELD_13_BIT_MASK) %
						m_active_y_limit;
	if ((m_active_y_offset != old_y_offset) ||
		(m_active_y_limit != old_y_limit))
		m_display_dirty = true;
	m_active_pause = m_registers[REG_PAUSE] & 0x07ff;
	m_pause_fired = false;
	m_index_phase_active =
			was_pending &&
			((m_registers[REG_OLD_X_INDEX] != m_registers[REG_NEW_X_INDEX]) ||
			 (m_registers[REG_OLD_Y_INDEX] != m_registers[REG_NEW_Y_INDEX]));
	begin_scroll();
	m_status |= STATUS_SCROLL_SERVICE;
	DC323_LOGMASKED(
			LOG_SCROLL,
			"%s %s: frame pending=%u->%u status=%04X->%04X enable=%04X\n",
			machine().time().to_string(), tag(), was_pending, m_scroll_pending,
			old_status, m_status, m_registers[REG_INTERRUPT_ENABLE]);
	update_interrupt("vertical frame");
}


// Service pause crossings and incremental scroll work at the current scanline.
void dc323_device::scanline_event(u16 line)
{
	if (!m_operational)
		return;

	// The pause value is double buffered at the frame boundary.  Crossing the
	// programmed device-coordinate scan queues one of up to two independently
	// acknowledgeable pause events.
	if (!m_pause_fired && (line == m_active_pause))
	{
		m_pause_fired = true;
		m_pause_events = std::min<u8>(m_pause_events + 1, 2);
		m_status |= STATUS_PAUSE_COMPLETE;
		update_interrupt("pause raster");
	}

	if (!m_scroll_active)
		return;
	u16 const last_line = std::min<u16>(line, m_active_scroll_y_max - 1);
	while (m_active_scroll_line <= last_line)
		execute_scroll_line(m_active_scroll_line++);
	if (m_active_scroll_line >= m_active_scroll_y_max)
		m_scroll_active = false;
}


// Sign-extend a coordinate from the DC323's 14-bit coordinate space.
int dc323_device::signed_14(u16 value)
{
	value &= COORDINATE_MASK;
	return BIT(value, 13) ? int(value) - 0x4000 : int(value);
}


// Decode the Viper bus-width field into its physical 4-, 8-, or 16-bit width.
unsigned dc323_device::viper_bus_width(u16 value)
{
	switch ((value >> 2) & 3)
	{
	case 0: return 4;
	case 3: return 16;
	default: return 8;
	}
}


// Generate a Source 2 address using either linear or power-of-two tiled motion.
int dc323_device::source2_coordinate(u16 origin, int coordinate,
									 unsigned tile_size, bool tiled)
{
	return signed_14(origin +
					 (tiled ? (coordinate & (tile_size - 1)) : coordinate));
}


// Decode the Source 2 tiling flag and encoded tile dimensions.
dc323_device::source2_format dc323_device::source2_format_r() const
{
	u16 const value = m_registers[REG_SOURCE2_SIZE];
	return { !BIT(value, 7), 1U << ((value & 0x0007) + 2),
			 1U << (((value >> 4) & 0x0007) + 2) };
}


// Expand a fixed-point scale into destination/source sample pairs, retaining
// phase.
std::vector<dc323_device::axis_sample>
dc323_device::scaled_axis(unsigned destination_length, u16 scale,
						  unsigned &source, unsigned &accumulator)
{
	std::vector<axis_sample> result;
	unsigned const step = (scale & FIELD_13_BIT_MASK) + 1;
	unsigned destination = 0;
	if (!BIT(scale, 13))
	{
		result.reserve(destination_length);
		while (destination < destination_length)
		{
			result.push_back({ destination++, source });
			accumulator += step;
			if (accumulator >= 0x2000)
			{
				accumulator -= 0x2000;
				source++;
			}
		}
	}
	else
	{
		// During down scaling the source advances on every computation and
		// the destination advances only when the scale accumulator carries.
		while (destination < destination_length)
		{
			result.push_back({ destination, source++ });
			accumulator += step;
			if (accumulator >= 0x2000)
			{
				accumulator -= 0x2000;
				destination++;
			}
		}
	}
	return result;
}


// Generate scaled samples from a fresh source position and fractional phase.
std::vector<dc323_device::axis_sample>
dc323_device::scaled_axis(unsigned destination_length, u16 scale)
{
	unsigned source = 0;
	unsigned accumulator = 0;
	return scaled_axis(destination_length, scale, source, accumulator);
}


// Generate the bounded Bresenham path used by raster and chained-fill vectors.
std::vector<dc323_device::raster_point>
dc323_device::raster_vector(int dx, int dy, int error_adjustment,
							unsigned limit)
{
	unsigned const abs_dx = unsigned(std::abs(dx));
	unsigned const abs_dy = unsigned(std::abs(dy));
	unsigned const length = std::min(std::max(abs_dx, abs_dy), limit);
	std::vector<raster_point> result;
	result.reserve(length);
	if (!length)
		return result;

	bool const x_major = abs_dx >= abs_dy;
	unsigned const major = x_major ? abs_dx : abs_dy;
	unsigned const minor = x_major ? abs_dy : abs_dx;
	int const x_step = (dx > 0) - (dx < 0);
	int const y_step = (dy > 0) - (dy < 0);
	int error = -int(major) + error_adjustment;
	int x = 0;
	int y = 0;
	for (unsigned position = 0; position < length; position++)
	{
		result.push_back({ x, y });
		if (x_major)
			x += x_step;
		else
			y += y_step;
		error += int(2 * minor);
		if (error >= 0)
		{
			if (x_major)
				y += y_step;
			else
				x += x_step;
			error -= int(2 * major);
		}
	}
	return result;
}


// Select the old or new raster index according to the live scroll phase.
std::pair<int, int> dc323_device::raster_index(int unindexed_y)
{
	// Up-scroll moves rows behind refresh, so updates below the current raster
	// still need the old index.  Down-scroll rotates the refresh store and PYOF
	// together at the frame boundary; its entire frame therefore uses the new
	// index rather than splitting a multi-scanline glyph at the live raster.
	bool const use_new = !m_index_phase_active || BIT(m_active_scroll, 12) ||
						 (unindexed_y < int(m_raster_line_handler()));
	unsigned const x_reg = use_new ? REG_NEW_X_INDEX : REG_OLD_X_INDEX;
	unsigned const y_reg = use_new ? REG_NEW_Y_INDEX : REG_OLD_Y_INDEX;
	return { signed_14(m_registers[x_reg]), signed_14(m_registers[y_reg]) };
}


// Dequeue processor-bound data and refill the FIFO from an active bitmap read.
u8 dc323_device::id_data_r()
{
	if (!m_pbt_read_active || !m_id_fifo_count)
		return u8(m_id_fifo_count ? m_id_fifo[m_id_fifo_head]
								  : m_registers[REG_ID_DATA]);
	u16 const data = fifo_pop();
	while ((m_id_fifo_count < m_id_fifo.size()) &&
		   (m_pbt_read_index < m_pbt_read_count))
		fifo_push(pbt_read_word(m_pbt_read_index++));
	if ((m_pbt_read_index >= m_pbt_read_count) && !m_id_fifo_count)
	{
		m_pbt_read_active = false;
		m_status |= STATUS_ADDRESS_COMPLETE;
	}
	update_fifo_status();
	schedule_pbt_service();
	DC323_LOGMASKED(LOG_TRANSFERS,
					"%s %s: I/D dequeue generated=%u/%u data=%04X fifo=%u "
					"active=%u status=%04X\n",
					machine().time().to_string(), tag(), m_pbt_read_index,
					m_pbt_read_count, data, m_id_fifo_count, m_pbt_read_active,
					m_status);
	return u8(data);
}


// Assemble one processor-bound transfer word from bitmap, Z, ROM, or test data.
u16 dc323_device::pbt_read_word(u32 index)
{
	u16 const width = m_registers[REG_WIDTH] & FIELD_13_BIT_MASK;
	if (m_pbt_read_z_mode)
	{
		u32 const row = width ? index / width : 0;
		u32 const column = width ? index % width : 0;
		int const source_x_step =
				BIT(m_registers[REG_SOURCE_FAST_DX], 13) ? -1 : 1;
		int const source_y_step =
				BIT(m_registers[REG_SOURCE_SLOW_DY], 13) ? -1 : 1;
		int const unindexed_source_x =
				int(m_registers[REG_SOURCE_X] & COORDINATE_MASK) +
				int(column) * source_x_step;
		int const unindexed_source_y =
				int(m_registers[REG_SOURCE_Y] & COORDINATE_MASK) +
				int(row) * source_y_step;
		auto const raster_offset = BIT(m_registers[REG_X_CONTROL], 5)
										   ? raster_index(unindexed_source_y)
										   : std::pair<int, int>{ 0, 0 };
		int const source_x = unindexed_source_x + raster_offset.first;
		int const source_y = unindexed_source_y + raster_offset.second;
		unsigned const z_block = m_pbt_read_command & 3;
		u16 data = 0;
		if (bitmap_contains(source_x, source_y))
			for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
			{
				if (!BIT(m_viper_select, plane))
					continue;
				unsigned const plane_address =
						m_viper_registers[plane][VIPER_REG_Z_ADDRESS] &
						Z_ADDRESS_MASK;
				if ((plane_address >> Z_BLOCK_SHIFT) == z_block)
					data |= u16(m_bitmap_planes[device_bitmap_plane_address(
									plane, source_x, source_y)])
							<< (plane_address & 15);
			}
		return data;
	}

	u32 const bytes_per_row = (width + 7) / 8;
	u32 const row = bytes_per_row ? index / bytes_per_row : 0;
	u32 const column = bytes_per_row ? index % bytes_per_row : 0;
	u16 const source_origin_x = m_registers[REG_SOURCE_X] & COORDINATE_MASK;
	u16 const source_origin_y = m_registers[REG_SOURCE_Y] & FIELD_13_BIT_MASK;
	u16 const source_x = source_origin_x + column * 8;
	u16 const source_y = source_origin_y + row;
	u8 data = 0;
	if (m_raster_diagnostic && (m_pbt_read_command == 0x0bc0))
		data = BIT(source_origin_x, 3) ? 0x55 : 0xaa;
	else if (!m_raster_diagnostic && (m_pbt_read_command == 0x0bc0))
		data = m_diagnostic_data;
	else if (BIT(source_origin_y, 12))
	{
		u16 const rom_x = source_x & 0x00ff;
		u16 const rom_y = source_y & 0x03ff;
		data = m_chargen_rom[(rom_y << 5) | (rom_x >> 3)];
	}
	else if ((source_x < BITMAP_WIDTH) && (source_y < BITMAP_HEIGHT))
	{
		unsigned output_plane = 0;
		bool output_enabled = false;
		unsigned const csr = VIPER_REG_CSR_BASE + (m_pbt_read_command & 7);
		for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
			if (BIT(m_viper_select, plane) &&
				BIT(m_viper_registers[plane][csr], 4))
			{
				output_plane = plane;
				output_enabled = true;
				break;
			}
		if (output_enabled)
			for (unsigned bit = 0; bit < 8 && (source_x + bit) < BITMAP_WIDTH;
				 bit++)
				data |= (m_bitmap_planes[device_bitmap_plane_address(
								 output_plane, source_x + bit, source_y)] &
						 1)
						<< bit;
	}
	return data;
}


// Enqueue one bitmap-bound word while enforcing the physical six-word FIFO
// depth.
void dc323_device::id_data_w(u16 data)
{
	if (!m_pbt_write_active)
	{
		fifo_clear();
		fifo_push(data);
		return;
	}
	if ((m_pbt_write_accepted >= m_pbt_write_count) || !fifo_push(data))
		return;
	m_pbt_write_accepted++;
	if (m_pbt_write_accepted >= m_pbt_write_count)
	{
		// Once the complete short transfer is resident, finish it without
		// forcing the processor to yield solely for the regression harness.
		// Longer streams still back-pressure at the physical six-word depth.
		while (m_id_fifo_count && (m_pbt_write_index < m_pbt_write_count))
			pbt_write_word(m_pbt_write_index++, fifo_pop());
		if (m_pbt_write_index >= m_pbt_write_count)
		{
			m_pbt_write_active = false;
			m_status |= STATUS_ADDRESS_COMPLETE;
		}
	}
	else
		schedule_pbt_service();
	update_fifo_status();
	DC323_LOGMASKED(LOG_TRANSFERS,
					"%s %s: I/D enqueue accepted=%u index=%u/%u data=%04X "
					"fifo=%u active=%u status=%04X\n",
					machine().time().to_string(), tag(), m_pbt_write_accepted,
					m_pbt_write_index, m_pbt_write_count, data, m_id_fifo_count,
					m_pbt_write_active, m_status);
}


// Apply one processor-to-bitmap transfer word in packed or Z-addressed mode.
void dc323_device::pbt_write_word(u32 index, u16 data)
{
	u16 const destination_x = m_registers[REG_DESTINATION_X] & COORDINATE_MASK;
	u16 const destination_y = m_registers[REG_DESTINATION_Y] & COORDINATE_MASK;
	u16 const width = m_registers[REG_WIDTH] & FIELD_13_BIT_MASK;
	u16 const height = m_registers[REG_HEIGHT] & FIELD_13_BIT_MASK;
	if (m_pbt_write_z_mode)
	{
		u32 const row = width ? index / width : 0;
		u32 const column = width ? index % width : 0;
		int const unindexed_x = destination_x + column;
		int const unindexed_y = destination_y + row;
		auto const raster_offset = BIT(m_registers[REG_X_CONTROL], 6)
										   ? raster_index(unindexed_y)
										   : std::pair<int, int>{ 0, 0 };
		int const x = unindexed_x + raster_offset.first;
		int const y = unindexed_y + raster_offset.second;
		unsigned const z_block = m_pbt_write_command & 3;
		unsigned const color_register = BIT(m_pbt_write_command, 3)
												? VIPER_REG_BACKGROUND
												: VIPER_REG_FOREGROUND;
		if (bitmap_contains(x, y))
			for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
			{
				if (!BIT(m_viper_select, plane))
					continue;
				auto &viper = m_viper_registers[plane];
				unsigned const plane_address =
						viper[VIPER_REG_Z_ADDRESS] & Z_ADDRESS_MASK;
				if ((plane_address >> Z_BLOCK_SHIFT) != z_block)
					continue;
				bool const plane_data = BIT(data, plane_address & 15);
				viper[color_register] = plane_data ? 0xffff : 0x0000;
				write_viper_pixel(plane, x, y, m_pbt_write_command, false);
			}
		return;
	}

	u32 const row = index / m_pbt_write_words_per_row;
	u32 const column = index % m_pbt_write_words_per_row;
	u16 const word_x = (destination_x & ~(m_pbt_write_bus_width - 1)) +
					   column * m_pbt_write_bus_width;
	if (row >= height || (destination_y + row) >= BITMAP_HEIGHT)
		return;
	for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
	{
		if (!BIT(m_viper_select, plane))
			continue;
		auto &viper = m_viper_registers[plane];
		switch (viper[VIPER_REG_CSR_BASE + (m_pbt_write_command & 7)] & 3)
		{
		case 1: viper[VIPER_REG_SOURCE] = data; break;
		case 2:
			viper[VIPER_REG_MASK_1] = data;
			viper[VIPER_REG_MASK_2] = data;
			break;
		case 3: viper[VIPER_REG_MASK_2] = data; break;
		}
		for (unsigned bit = 0; bit < m_pbt_write_bus_width; bit++)
		{
			u16 const x = word_x + bit;
			if (x < destination_x || x >= destination_x + width ||
				x >= BITMAP_WIDTH)
				continue;
			write_viper_pixel(plane, x, destination_y + row,
							  m_pbt_write_command,
							  BIT(viper[VIPER_REG_SOURCE], bit));
		}
	}
}


// Empty the I/D FIFO and refresh its externally visible status bits.
void dc323_device::fifo_clear()
{
	m_id_fifo_head = 0;
	m_id_fifo_tail = 0;
	m_id_fifo_count = 0;
	update_fifo_status();
}


// Append one I/D word when FIFO capacity is available.
bool dc323_device::fifo_push(u16 data)
{
	if (m_id_fifo_count >= m_id_fifo.size())
		return false;
	m_id_fifo[m_id_fifo_tail] = data;
	m_id_fifo_tail = (m_id_fifo_tail + 1) % m_id_fifo.size();
	m_id_fifo_count++;
	update_fifo_status();
	return true;
}


// Remove the oldest I/D word, returning zero for an empty FIFO.
u16 dc323_device::fifo_pop()
{
	if (!m_id_fifo_count)
		return 0;
	u16 const data = m_id_fifo[m_id_fifo_head];
	m_id_fifo_head = (m_id_fifo_head + 1) % m_id_fifo.size();
	m_id_fifo_count--;
	update_fifo_status();
	return data;
}


// Derive receive/transmit-ready status and IRQ state from FIFO occupancy.
void dc323_device::update_fifo_status()
{
	if (m_id_fifo_count)
		m_status |= STATUS_ID_DATA_READY;
	else
		m_status &= ~STATUS_ID_DATA_READY;
	if (m_id_fifo_count < m_id_fifo.size())
		m_status |= STATUS_ID_DATA_TRANSMIT_READY;
	else
		m_status &= ~STATUS_ID_DATA_TRANSMIT_READY;
	update_interrupt("I/D FIFO state");
}


// Schedule the next asynchronous transfer service step while either path is
// active.
void dc323_device::schedule_pbt_service()
{
	if (m_pbt_read_active || m_pbt_write_active)
		m_pbt_service_timer->adjust(attotime::from_usec(1));
}


// Cancel timers and transfer state when a command is aborted or superseded.
void dc323_device::stop_command_activity()
{
	m_command_initialize_timer->adjust(attotime::never);
	m_command_complete_timer->adjust(attotime::never);
	m_pbt_service_timer->adjust(attotime::never);
	m_pbt_read_active = false;
	m_pbt_read_z_mode = false;
	m_pbt_write_active = false;
	m_pbt_write_z_mode = false;
	fifo_clear();
}


// Assert initialization-complete after the command decoder's modeled latency.
TIMER_CALLBACK_MEMBER(dc323_device::command_initialize_tick)
{
	m_status |= STATUS_RASTEROP_INITIALIZED;
	update_interrupt("command initialization complete");
}


// Assert raster and address completion after the operation's modeled latency.
TIMER_CALLBACK_MEMBER(dc323_device::command_complete_tick)
{
	m_status |= STATUS_RASTEROP_COMPLETE;
	if (param == 0)
		m_status |= STATUS_ADDRESS_COMPLETE;
	update_interrupt(param == 0 ? "raster operation complete"
								: "PBT address calculation complete");
}


// Move one service quantum between bitmap memory and the transfer FIFO.
TIMER_CALLBACK_MEMBER(dc323_device::pbt_service_tick)
{
	if (m_pbt_write_active && m_id_fifo_count &&
		(m_pbt_write_index < m_pbt_write_count))
	{
		pbt_write_word(m_pbt_write_index++, fifo_pop());
		if (m_pbt_write_index >= m_pbt_write_count)
		{
			m_pbt_write_active = false;
			m_status |= STATUS_ADDRESS_COMPLETE;
		}
	}
	if (m_pbt_read_active && (m_id_fifo_count < m_id_fifo.size()) &&
		(m_pbt_read_index < m_pbt_read_count))
		fifo_push(pbt_read_word(m_pbt_read_index++));
	if (m_pbt_read_active && (m_pbt_read_index >= m_pbt_read_count) &&
		!m_id_fifo_count)
	{
		m_pbt_read_active = false;
		m_status |= STATUS_ADDRESS_COMPLETE;
	}
	update_fifo_status();
	if ((m_pbt_write_active && m_id_fifo_count) ||
		(m_pbt_read_active && (m_pbt_read_index < m_pbt_read_count) &&
		 (m_id_fifo_count < m_id_fifo.size())))
		schedule_pbt_service();
}


// Decode a command, initialize its data path, and schedule completion events.
void dc323_device::command_w(u16 command)
{
	unsigned const operation = (command >> 8) & 3;
	bool const bitmap_to_processor =
			(operation == COMMAND_PROCESSOR_TRANSFER) && BIT(command, 11) &&
			!BIT(command, 10);
	bool const processor_to_bitmap =
			(operation == COMMAND_PROCESSOR_TRANSFER) && !BIT(command, 11) &&
			BIT(command, 10);
	u16 const old_status = m_status;
	DC323_LOGMASKED(
			LOG_COMMANDS,
			"%s %s: command=%04X operation=%u status=%04X operational=%u\n",
			machine().time().to_string(), tag(), command, operation, m_status,
			m_operational);

	if (m_operational && ((operation == COMMAND_RASTER_OPERATION) ||
						  (operation == COMMAND_PROCESSOR_TRANSFER)))
	{
		m_status &= ~STATUS_COMMAND_MASK;
		stop_command_activity();
		m_command_initialize_timer->adjust(attotime::from_usec(4));
	}
	else if (m_operational && (command == 0x0000))
	{
		// Command zero returns the command path to its idle state.  Firmware
		// uses it to terminate a diagnostic I/D sequence before polling Address
		// Output Complete.
		stop_command_activity();
		m_status |= STATUS_RASTEROP_MASK;
		update_interrupt("command abort");
	}

	if ((command & 0xffe0) == 0x0180)
	{
		unsigned const reg = command & VIPER_REGISTER_MASK;
		for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
			if (BIT(m_viper_select, plane))
				m_viper_registers[plane][reg] = m_registers[REG_ID_DATA];
	}

	if (bitmap_to_processor)
	{
		u32 const width = m_registers[REG_WIDTH] & FIELD_13_BIT_MASK;
		u32 const height = m_registers[REG_HEIGHT] & FIELD_13_BIT_MASK;
		m_pbt_read_z_mode = (command & 0xfffc) == 0x0bd8;
		m_pbt_read_index = 0;
		m_pbt_read_command = command;
		m_pbt_read_count =
				(m_pbt_read_z_mode ? width : ((width + 7) / 8)) * height;
		m_pbt_read_active = m_pbt_read_count != 0;
		while ((m_id_fifo_count < m_id_fifo.size()) &&
			   (m_pbt_read_index < m_pbt_read_count))
			fifo_push(pbt_read_word(m_pbt_read_index++));
		if (m_operational)
		{
			if (!m_pbt_read_active)
				m_status |= STATUS_ADDRESS_COMPLETE;
			m_command_complete_timer->adjust(attotime::from_usec(8), 1);
		}
		schedule_pbt_service();
		DC323_LOGMASKED(LOG_TRANSFERS,
						"%s %s: PBT start width=%u height=%u count=%u "
						"active=%u status=%04X\n",
						machine().time().to_string(), tag(), width, height,
						m_pbt_read_count, m_pbt_read_active, m_status);
	}
	else if (processor_to_bitmap)
	{
		u16 const destination_x =
				m_registers[REG_DESTINATION_X] & COORDINATE_MASK;
		u16 const width = m_registers[REG_WIDTH] & FIELD_13_BIT_MASK;
		u16 const height = m_registers[REG_HEIGHT] & FIELD_13_BIT_MASK;
		m_pbt_write_z_mode = (command & 0xfff4) == 0x07a4;
		m_pbt_write_bus_width =
				viper_bus_width(m_viper_registers[0][VIPER_REG_BUS_WIDTH]);
		m_pbt_write_words_per_row =
				(width != 0) ? ((destination_x & (m_pbt_write_bus_width - 1)) +
								width + m_pbt_write_bus_width - 1) /
									   m_pbt_write_bus_width
							 : 0;
		m_pbt_write_index = 0;
		m_pbt_write_accepted = 0;
		m_pbt_write_count =
				(m_pbt_write_z_mode ? width : m_pbt_write_words_per_row) *
				height;
		m_pbt_write_command = command;
		m_pbt_write_active = m_pbt_write_count != 0;
		if (m_operational)
		{
			if (!m_pbt_write_active)
				m_status |= STATUS_ADDRESS_COMPLETE;
			m_command_complete_timer->adjust(attotime::from_usec(8), 1);
		}
		DC323_LOGMASKED(LOG_TRANSFERS,
						"%s %s: PTB start width=%u height=%u bus=%u words=%u "
						"active=%u status=%04X\n",
						machine().time().to_string(), tag(), width, height,
						m_pbt_write_bus_width, m_pbt_write_count,
						m_pbt_write_active, m_status);
	}
	else if (operation == COMMAND_PROCESSOR_TRANSFER)
		unsupported_transfer(command);

	execute_rasterop(command);
	if (m_operational && (operation == COMMAND_RASTER_OPERATION))
		m_command_complete_timer->adjust(attotime::from_usec(8), 0);
	DC323_LOGMASKED(
			LOG_COMMANDS, "%s %s: command complete=%04X status=%04X->%04X\n",
			machine().time().to_string(), tag(), command, old_status, m_status);
}


// Preserve an unidentified operation-3 encoding without fabricating transfer
// data or completion semantics.  Command initialization still follows the
// common command path above, matching the behavior before this boundary was
// made explicit.
void dc323_device::unsupported_transfer(u16 command)
{
	DC323_LOGMASKED(
			LOG_COMMANDS,
			"%s %s: unsupported operation-3 encoding=%04X D=%u S1=%u S2=%u\n",
			machine().time().to_string(), tag(), command, BIT(command, 10),
			BIT(command, 11), BIT(command, 12));
}


// Drive the board IRQ whenever enabled status sources change state.
void dc323_device::update_interrupt(char const *event)
{
	bool const state =
			m_operational && bool(m_status & m_registers[REG_INTERRUPT_ENABLE]);
	if (state != m_irq_state)
	{
		DC323_LOGMASKED(LOG_INTERRUPTS,
						"%s %s: IRQ %u->%u event=%s status=%04X enable=%04X\n",
						machine().time().to_string(), tag(), m_irq_state, state,
						event, m_status, m_registers[REG_INTERRUPT_ENABLE]);
		m_irq_state = state;
		m_irq_handler(state ? ASSERT_LINE : CLEAR_LINE);
	}
}


// Dispatch and execute a normal, linear-pattern, or fill raster operation.
void dc323_device::execute_rasterop(u16 command)
{
	bool const rasterop = ((command >> 8) & 3) == COMMAND_RASTER_OPERATION;
	bool const destination_enabled = BIT(command, 10);
	bool const source1_enabled = BIT(command, 11);
	bool const source2_enabled = BIT(command, 12);
	u16 const x_control = m_registers[REG_X_CONTROL];
	unsigned const x_mode = x_control & 0x0003;
	bool const normal_mode = (x_mode == X_MODE_NORMAL) && BIT(x_control, 7);
	bool const linear_pattern_mode = x_mode == X_MODE_LINEAR_PATTERN;
	bool const fill_mode = (x_mode == X_MODE_FILL) && BIT(x_control, 7);
	if (!rasterop)
		return;
	if (!destination_enabled)
	{
		DC323_LOGMASKED(LOG_RASTEROPS,
						"%s %s: raster no-op=%04X destination disabled\n",
						machine().time().to_string(), tag(), command);
		return;
	}
	if (linear_pattern_mode)
	{
		if (source1_enabled || source2_enabled)
			execute_linear_pattern(command);
		else
			DC323_LOGMASKED(
					LOG_RASTEROPS,
					"%s %s: raster no-op=%04X source-free linear mode\n",
					machine().time().to_string(), tag(), command);
		return;
	}
	if (fill_mode)
	{
		if (!source1_enabled)
			execute_fill(command);
		else
			DC323_LOGMASKED(
					LOG_RASTEROPS,
					"%s %s: unsupported fill encoding=%04X with Source 1\n",
					machine().time().to_string(), tag(), command);
		return;
	}
	if (!normal_mode)
	{
		DC323_LOGMASKED(
				LOG_RASTEROPS,
				"%s %s: raster no-op=%04X X-control=%04X reserved/pen-up\n",
				machine().time().to_string(), tag(), command, x_control);
		return;
	}

	int const source_x = signed_14(m_registers[REG_SOURCE_X]);
	int const source_y = signed_14(m_registers[REG_SOURCE_Y]);
	int const unindexed_destination_x =
			m_registers[REG_DESTINATION_X] & COORDINATE_MASK;
	int const unindexed_destination_y =
			m_registers[REG_DESTINATION_Y] & COORDINATE_MASK;
	auto const origin_destination_index =
			BIT(x_control, 6) ? raster_index(unindexed_destination_y)
							  : std::pair<int, int>{ 0, 0 };
	int const destination_x =
			signed_14(unindexed_destination_x + origin_destination_index.first);
	int const destination_y = signed_14(unindexed_destination_y +
										origin_destination_index.second);
	auto const origin_source_index = source1_enabled && BIT(x_control, 5)
											 ? raster_index(source_y)
											 : std::pair<int, int>{ 0, 0 };
	int const indexed_source_y =
			signed_14(source_y + origin_source_index.second);
	bool const source_is_e49 = source1_enabled && BIT(indexed_source_y, 12);
	int const destination_fast_dx = signed_14(m_registers[REG_WIDTH]);
	int const destination_fast_dy =
			signed_14(m_registers[REG_DESTINATION_FAST_DY]);
	int const destination_slow_dx =
			signed_14(m_registers[REG_DESTINATION_SLOW_DX]);
	int const destination_slow_dy = signed_14(m_registers[REG_HEIGHT]);
	auto const fast_path =
			raster_vector(destination_fast_dx, destination_fast_dy,
						  signed_14(m_registers[REG_ERROR_2]), BITMAP_HEIGHT);
	auto const slow_path =
			raster_vector(destination_slow_dx, destination_slow_dy,
						  signed_14(m_registers[REG_ERROR_1]), BITMAP_HEIGHT);
	unsigned const width = fast_path.size();
	unsigned const height = slow_path.size();
	int const source_x_step = BIT(m_registers[REG_SOURCE_FAST_DX], 13) ? -1 : 1;
	int const source_y_step = BIT(m_registers[REG_SOURCE_SLOW_DY], 13) ? -1 : 1;
	auto const fast_samples = scaled_axis(width, m_registers[REG_FAST_SCALE]);
	auto const slow_samples = scaled_axis(height, m_registers[REG_SLOW_SCALE]);
	bool const fill_holes =
			BIT(x_control, 4) && !source1_enabled && !source2_enabled;
	source2_format const source2 = source2_enabled
										   ? source2_format_r()
										   : source2_format{ false, 1, 1 };
	std::vector<bool> selected;
	std::array<int, BITMAP_HEIGHT> row_min;
	std::array<int, BITMAP_HEIGHT> row_max;
	unsigned first_occupied_row = BITMAP_HEIGHT;
	unsigned last_occupied_row = 0;
	if (fill_holes)
	{
		selected.resize(BITMAP_PLANE_SIZE);
		row_min.fill(BITMAP_WIDTH);
		row_max.fill(-1);
	}
	unsigned const function = (command >> 4) & 3;
	u8 const logic0 = u8(m_viper_registers[0][VIPER_REG_LOGIC_BASE + function]);
	DC323_LOGMASKED(
			LOG_RASTEROPS,
			"%s %s: raster command=%04X src=%d,%d s2=%04X,%04X tile=%u/%ux%u "
			"dst=%d,%d size=%ux%u select=%X logic0=%02X s1=%u s2=%u\n",
			machine().time().to_string(), tag(), command, source_x, source_y,
			m_registers[REG_SOURCE2_X], m_registers[REG_SOURCE2_Y],
			source2.tiled, source2.width, source2.height, destination_x,
			destination_y, width, height, m_viper_select, logic0,
			source1_enabled, source2_enabled);
	for (axis_sample const slow : slow_samples)
	{
		raster_point const slow_point = slow_path[slow.destination];
		int const unindexed_source_pixel_y =
				source1_enabled ? source_y + slow.source * source_y_step : 0;
		for (axis_sample const fast : fast_samples)
		{
			raster_point const fast_point = fast_path[fast.destination];
			int const unindexed_pixel_x =
					unindexed_destination_x + slow_point.x + fast_point.x;
			int const unindexed_pixel_y =
					unindexed_destination_y + slow_point.y + fast_point.y;
			auto const destination_index =
					BIT(x_control, 6) ? raster_index(unindexed_pixel_y)
									  : std::pair<int, int>{ 0, 0 };
			int const pixel_x =
					signed_14(unindexed_pixel_x + destination_index.first);
			int const pixel_y =
					signed_14(unindexed_pixel_y + destination_index.second);
			if (!bitmap_contains(pixel_x, pixel_y))
				continue;
			if (fill_holes)
			{
				selected[pixel_y * BITMAP_WIDTH + pixel_x] = true;
				row_min[pixel_y] = std::min(row_min[pixel_y], pixel_x);
				row_max[pixel_y] = std::max(row_max[pixel_y], pixel_x);
				first_occupied_row =
						std::min(first_occupied_row, unsigned(pixel_y));
				last_occupied_row =
						std::max(last_occupied_row, unsigned(pixel_y));
			}
			unsigned const word_bit = pixel_x & 15;
			std::array<bool, BITMAP_PLANES> source1_bits{};
			std::array<bool, BITMAP_PLANES> source2_bits{};
			if (source1_enabled)
			{
				auto const source_index =
						BIT(x_control, 5)
								? raster_index(unindexed_source_pixel_y)
								: std::pair<int, int>{ 0, 0 };
				int const source_pixel_x =
						signed_14(source_x + fast.source * source_x_step +
								  source_index.first);
				int const source_pixel_y = signed_14(unindexed_source_pixel_y +
													 source_index.second);
				if (source_is_e49)
				{
					u16 const rom_x = source_pixel_x & 0x00ff;
					u16 const rom_y = source_pixel_y & 0x03ff;
					source1_bits.fill(
							BIT(m_chargen_rom[(rom_y << 5) | (rom_x >> 3)],
								rom_x & 7));
				}
				else if (bitmap_contains(source_pixel_x, source_pixel_y))
					for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
						if (BIT(m_viper_select, plane))
							source1_bits[plane] =
									m_bitmap_planes[device_bitmap_plane_address(
											plane, source_pixel_x,
											source_pixel_y)];
			}
			if (source2_enabled)
			{
				int const source2_pixel_x = source2_coordinate(
						m_registers[REG_SOURCE2_X], unindexed_pixel_x,
						source2.width, source2.tiled);
				int const source2_pixel_y = source2_coordinate(
						m_registers[REG_SOURCE2_Y], unindexed_pixel_y,
						source2.height, source2.tiled);
				if (bitmap_contains(source2_pixel_x, source2_pixel_y))
					for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
						if (BIT(m_viper_select, plane))
							source2_bits[plane] =
									m_bitmap_planes[device_bitmap_plane_address(
											plane, source2_pixel_x,
											source2_pixel_y)];
			}
			if (source1_enabled && source2_enabled)
			{
				unsigned const csr_bank = command & 4;
				route_source_cycle(csr_bank + 0, pixel_x, source1_bits);
				route_source_cycle(csr_bank + 1, pixel_x, source2_bits);
			}
			for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
			{
				if (!BIT(m_viper_select, plane))
					continue;
				auto const &viper = m_viper_registers[plane];
				unsigned const bus_width =
						viper_bus_width(viper[VIPER_REG_BUS_WIDTH]);
				unsigned const viper_word_bit =
						unsigned(pixel_x) & (bus_width - 1);
				bool const source_bit =
						(source1_enabled && source2_enabled)
								? BIT(viper[VIPER_REG_SOURCE], viper_word_bit)
						: source2_enabled ? source2_bits[plane]
						: source1_enabled
								? source1_bits[plane]
								: BIT(viper[VIPER_REG_SOURCE], word_bit);
				write_viper_pixel(plane, pixel_x, pixel_y, command, source_bit);
			}
		}
	}

	// Hole fill modifies destination scanning so a broad parallelogram does
	// not retain gaps between independently generated fast-vector scans.  Keep
	// every original write (including duplicates important to complement
	// operations), then add only the missing pixels in each occupied scanline.
	// Source-coupled hole pixels require an address-generation rule that has
	// not yet been established, so this first bounded implementation applies
	// only to destination-only/constant-source operations.
	if (fill_holes)
	{
		for (unsigned y = first_occupied_row;
			 (y < BITMAP_HEIGHT) && (y <= last_occupied_row); y++)
			for (int x = row_min[y]; x <= row_max[y]; x++)
			{
				if (selected[y * BITMAP_WIDTH + x])
					continue;
				for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
				{
					if (!BIT(m_viper_select, plane))
						continue;
					auto const &viper = m_viper_registers[plane];
					write_viper_pixel(
							plane, x, y, command,
							BIT(viper[VIPER_REG_SOURCE], unsigned(x) & 15));
				}
			}
	}
}


// Repeat and scale a retained bitmap pattern along the destination vectors.
void dc323_device::execute_linear_pattern(u16 command)
{
	int const source_fast_dx = signed_14(m_registers[REG_SOURCE_FAST_DX]);
	int const source_slow_dy = signed_14(m_registers[REG_SOURCE_SLOW_DY]);
	int const destination_fast_dx = signed_14(m_registers[REG_WIDTH]);
	int const destination_fast_dy =
			signed_14(m_registers[REG_DESTINATION_FAST_DY]);
	int const destination_slow_dx =
			signed_14(m_registers[REG_DESTINATION_SLOW_DX]);
	int const destination_slow_dy = signed_14(m_registers[REG_HEIGHT]);
	bool const source1_enabled = BIT(command, 11);
	bool const source2_enabled = BIT(command, 12);
	if (source1_enabled && (!source_fast_dx || !source_slow_dy))
		return;

	unsigned const source_width =
			source1_enabled ? unsigned(std::abs(source_fast_dx)) : 1;
	unsigned const source_height =
			source1_enabled ? unsigned(std::abs(source_slow_dy)) : 1;
	auto const fast_path =
			raster_vector(destination_fast_dx, destination_fast_dy,
						  signed_14(m_registers[REG_ERROR_2]), BITMAP_HEIGHT);
	auto const slow_path =
			raster_vector(destination_slow_dx, destination_slow_dy,
						  signed_14(m_registers[REG_ERROR_1]), BITMAP_HEIGHT);
	unsigned const destination_width = fast_path.size();
	unsigned const destination_height = slow_path.size();
	if (!destination_width || !destination_height)
		return;
	unsigned slow_source =
			source1_enabled ? m_linear_pattern_slow_position % source_height
							: 0;
	unsigned slow_accumulator =
			source1_enabled ? m_linear_pattern_slow_accumulator : 0;
	auto const slow_samples = scaled_axis(
			destination_height,
			source1_enabled ? m_registers[REG_SLOW_SCALE] : UNITY_SCALE,
			slow_source, slow_accumulator);
	if (source1_enabled)
	{
		m_linear_pattern_slow_position = slow_source % source_height;
		m_linear_pattern_slow_accumulator = slow_accumulator;
	}
	int const source_x_step = source_fast_dx < 0 ? -1 : 1;
	int const source_y_step = source_slow_dy < 0 ? -1 : 1;
	int const source_origin_x = signed_14(m_registers[REG_SOURCE_X]);
	int const source_origin_y = signed_14(m_registers[REG_SOURCE_Y]);
	int const unindexed_destination_origin_x =
			m_registers[REG_DESTINATION_X] & COORDINATE_MASK;
	int const unindexed_destination_origin_y =
			m_registers[REG_DESTINATION_Y] & COORDINATE_MASK;
	u16 const x_control = m_registers[REG_X_CONTROL];
	auto const destination_origin_index =
			BIT(x_control, 6) ? raster_index(unindexed_destination_origin_y)
							  : std::pair<int, int>{ 0, 0 };
	int const destination_origin_x = signed_14(m_registers[REG_DESTINATION_X]) +
									 destination_origin_index.first;
	int const destination_origin_y = signed_14(m_registers[REG_DESTINATION_Y]) +
									 destination_origin_index.second;
	auto const source_origin_index = source1_enabled && BIT(x_control, 5)
											 ? raster_index(source_origin_y)
											 : std::pair<int, int>{ 0, 0 };
	source2_format const source2 = source2_enabled
										   ? source2_format_r()
										   : source2_format{ false, 1, 1 };
	unsigned fast_source = 0;
	unsigned fast_accumulator = 0;
	auto const fast_samples = scaled_axis(
			destination_width,
			source1_enabled ? m_registers[REG_FAST_SCALE] : UNITY_SCALE,
			fast_source, fast_accumulator);
	DC323_LOGMASKED(LOG_RASTEROPS,
					"%s %s: linear pattern command=%04X src=%d,%d "
					"pattern=%ux%u dst=%d,%d size=%ux%u select=%X\n",
					machine().time().to_string(), tag(), command,
					source_origin_x + source_origin_index.first,
					source_origin_y + source_origin_index.second, source_width,
					source_height, destination_origin_x, destination_origin_y,
					destination_width, destination_height, m_viper_select);

	for (axis_sample const slow : slow_samples)
	{
		raster_point const slow_point = slow_path[slow.destination];
		int source_y = 0;
		std::pair<int, int> source_index{ 0, 0 };
		if (source1_enabled)
		{
			int const unindexed_source_y =
					source_origin_y +
					int(slow.source % source_height) * source_y_step;
			if (BIT(x_control, 5))
				source_index = raster_index(unindexed_source_y);
			source_y = unindexed_source_y + source_index.second;
			if (unsigned(source_y) >= BITMAP_HEIGHT)
				continue;
		}
		for (axis_sample const fast : fast_samples)
		{
			raster_point const fast_point = fast_path[fast.destination];
			int const unindexed_destination_x = unindexed_destination_origin_x +
												slow_point.x + fast_point.x;
			int const unindexed_destination_y = unindexed_destination_origin_y +
												slow_point.y + fast_point.y;
			auto const destination_index =
					BIT(x_control, 6) ? raster_index(unindexed_destination_y)
									  : std::pair<int, int>{ 0, 0 };
			int const destination_x =
					unindexed_destination_x + destination_index.first;
			int const destination_y =
					unindexed_destination_y + destination_index.second;
			if (!bitmap_contains(destination_x, destination_y))
				continue;
			std::array<bool, BITMAP_PLANES> source1_bits{};
			std::array<bool, BITMAP_PLANES> source2_bits{};
			if (source1_enabled)
			{
				int const source_x =
						source_origin_x +
						int(fast.source % source_width) * source_x_step +
						source_index.first;
				if (unsigned(source_x) >= BITMAP_WIDTH)
					continue;
				for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
					if (BIT(m_viper_select, plane))
						source1_bits[plane] =
								m_bitmap_planes[device_bitmap_plane_address(
										plane, source_x, source_y)];
			}
			if (source2_enabled)
			{
				int const source2_x = source2_coordinate(
						m_registers[REG_SOURCE2_X], unindexed_destination_x,
						source2.width, source2.tiled);
				int const source2_y = source2_coordinate(
						m_registers[REG_SOURCE2_Y], unindexed_destination_y,
						source2.height, source2.tiled);
				if (bitmap_contains(source2_x, source2_y))
					for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
						if (BIT(m_viper_select, plane))
							source2_bits[plane] =
									m_bitmap_planes[device_bitmap_plane_address(
											plane, source2_x, source2_y)];
			}
			if (source1_enabled && source2_enabled)
			{
				unsigned const csr_bank = command & 4;
				route_source_cycle(csr_bank + 0, destination_x, source1_bits);
				route_source_cycle(csr_bank + 1, destination_x, source2_bits);
			}
			for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
			{
				if (!BIT(m_viper_select, plane))
					continue;
				auto const &viper = m_viper_registers[plane];
				unsigned const bus_width =
						viper_bus_width(viper[VIPER_REG_BUS_WIDTH]);
				unsigned const word_bit =
						unsigned(destination_x) & (bus_width - 1);
				bool const source_bit =
						(source1_enabled && source2_enabled)
								? BIT(viper[VIPER_REG_SOURCE], word_bit)
						: source2_enabled ? source2_bits[plane]
										  : source1_bits[plane];
				if (BIT(x_control, 7))
					write_viper_pixel(plane, destination_x, destination_y,
									  command, source_bit);
			}
		}
	}
}


// Advance chained edge vectors and write the inclusive span between them.
void dc323_device::execute_fill(u16 command)
{
	u16 const x_control = m_registers[REG_X_CONTROL];
	bool const y_fill = BIT(x_control, 2);
	bool const baseline = BIT(x_control, 3);
	bool const source2_enabled = BIT(command, 12);
	source2_format const source2 = source2_enabled
										   ? source2_format_r()
										   : source2_format{ false, 1, 1 };
	if (!m_fill_position_valid)
	{
		m_fill_a_x = m_registers[REG_DESTINATION_X] & COORDINATE_MASK;
		m_fill_a_y = m_registers[REG_DESTINATION_Y] & FIELD_13_BIT_MASK;
		m_fill_b_x = y_fill ? m_fill_a_x
							: int(m_registers[REG_SOURCE_X] & COORDINATE_MASK);
		m_fill_b_y = y_fill ? int(m_registers[REG_SOURCE_Y] & FIELD_13_BIT_MASK)
							: m_fill_a_y;
		m_fill_position_valid = true;
		m_fill_a_vector_valid = false;
		m_fill_b_vector_valid = false;
	}
	int const a_origin_x = m_fill_a_x;
	int const a_origin_y = m_fill_a_y;
	int const b_origin_x = m_fill_b_x;
	int const b_origin_y = m_fill_b_y;
	int const programmed_a_dx = signed_14(m_registers[REG_DESTINATION_SLOW_DX]);
	int const programmed_a_dy = signed_14(m_registers[REG_HEIGHT]);
	int const programmed_b_dx = signed_14(m_registers[REG_SOURCE_FAST_DX]);
	int const programmed_b_dy = signed_14(m_registers[REG_SOURCE_SLOW_DY]);

	DC323_LOGMASKED(
			LOG_RASTEROPS,
			"%s %s: fill command=%04X axis=%c baseline=%u A=%d,%d points=%zu "
			"B=%d,%d points=%zu s2=%u/%04X,%04X tile=%u/%ux%u select=%X\n",
			machine().time().to_string(), tag(), command, y_fill ? 'Y' : 'X',
			baseline, a_origin_x, a_origin_y,
			raster_vector(programmed_a_dx, programmed_a_dy,
						  signed_14(m_registers[REG_ERROR_1]), BITMAP_HEIGHT)
					.size(),
			b_origin_x, b_origin_y,
			raster_vector(programmed_b_dx, programmed_b_dy,
						  signed_14(m_registers[REG_ERROR_2]), BITMAP_HEIGHT)
					.size(),
			source2_enabled, m_registers[REG_SOURCE2_X],
			m_registers[REG_SOURCE2_Y], source2.tiled, source2.width,
			source2.height, m_viper_select);

	auto const write_span = [this, command, y_fill, source2_enabled,
							 source2](int fixed, int first, int last)
	{
		if (first > last)
			std::swap(first, last);
		for (int position = first; position <= last; position++)
		{
			int const x = y_fill ? fixed : position;
			int const y = y_fill ? position : fixed;
			if (!bitmap_contains(x, y))
				continue;
			int source2_x = 0;
			int source2_y = 0;
			bool source2_valid = false;
			if (source2_enabled)
			{
				source2_x = source2_coordinate(m_registers[REG_SOURCE2_X], x,
											   source2.width, source2.tiled);
				source2_y = source2_coordinate(m_registers[REG_SOURCE2_Y], y,
											   source2.height, source2.tiled);
				source2_valid = bitmap_contains(source2_x, source2_y);
			}
			for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
			{
				if (!BIT(m_viper_select, plane))
					continue;
				auto const &viper = m_viper_registers[plane];
				bool const source_bit =
						source2_enabled
								? (source2_valid &&
								   m_bitmap_planes[device_bitmap_plane_address(
										   plane, source2_x, source2_y)])
								: BIT(viper[VIPER_REG_SOURCE],
									  unsigned(x) & 15);
				write_viper_pixel(plane, x, y, command, source_bit);
			}
		}
	};

	if (baseline)
	{
		auto const a_path = raster_vector(programmed_a_dx, programmed_a_dy,
										  signed_14(m_registers[REG_ERROR_1]),
										  BITMAP_HEIGHT);
		int const fixed_baseline = y_fill ? b_origin_y : b_origin_x;
		for (raster_point const a : a_path)
		{
			int const a_x = a_origin_x + a.x;
			int const a_y = a_origin_y + a.y;
			write_span(y_fill ? a_x : a_y, y_fill ? a_y : a_x, fixed_baseline);
		}
		m_fill_a_x += programmed_a_dx;
		m_fill_a_y += programmed_a_dy;
		return;
	}

	// Each edge generator retains its partial vector when the other edge
	// expires. Firmware can then load only the expired edge and resume the
	// still-active one on the next command.
	if (!m_fill_a_vector_valid || m_fill_a_vector_dirty)
	{
		m_fill_a_origin_x = m_fill_a_x;
		m_fill_a_origin_y = m_fill_a_y;
		m_fill_a_dx = programmed_a_dx;
		m_fill_a_dy = programmed_a_dy;
		m_fill_a_error = signed_14(m_registers[REG_ERROR_1]);
		m_fill_a_progress = 0;
		m_fill_a_vector_valid = true;
		m_fill_a_vector_dirty = false;
	}
	if (!m_fill_b_vector_valid || m_fill_b_vector_dirty)
	{
		m_fill_b_origin_x = m_fill_b_x;
		m_fill_b_origin_y = m_fill_b_y;
		m_fill_b_dx = programmed_b_dx;
		m_fill_b_dy = programmed_b_dy;
		m_fill_b_error = signed_14(m_registers[REG_ERROR_2]);
		m_fill_b_progress = 0;
		m_fill_b_vector_valid = true;
		m_fill_b_vector_dirty = false;
	}
	auto const a_path = raster_vector(m_fill_a_dx, m_fill_a_dy, m_fill_a_error,
									  BITMAP_HEIGHT);
	auto const b_path = raster_vector(m_fill_b_dx, m_fill_b_dy, m_fill_b_error,
									  BITMAP_HEIGHT);
	unsigned const a_remaining = (m_fill_a_progress < a_path.size())
										 ? a_path.size() - m_fill_a_progress
										 : 0;
	unsigned const b_remaining = (m_fill_b_progress < b_path.size())
										 ? b_path.size() - m_fill_b_progress
										 : 0;
	unsigned const points = std::min(a_remaining, b_remaining);
	for (unsigned position = 0; position < points; position++)
	{
		raster_point const a = a_path[m_fill_a_progress + position];
		raster_point const b = b_path[m_fill_b_progress + position];
		int const a_x = m_fill_a_origin_x + a.x;
		int const a_y = m_fill_a_origin_y + a.y;
		int const b_x = m_fill_b_origin_x + b.x;
		int const b_y = m_fill_b_origin_y + b.y;
		write_span(y_fill ? a_x : a_y, y_fill ? a_y : a_x, y_fill ? b_y : b_x);
	}
	m_fill_a_progress += points;
	m_fill_b_progress += points;
	if (m_fill_a_progress < a_path.size())
	{
		m_fill_a_x = m_fill_a_origin_x + a_path[m_fill_a_progress].x;
		m_fill_a_y = m_fill_a_origin_y + a_path[m_fill_a_progress].y;
	}
	else
	{
		m_fill_a_x = m_fill_a_origin_x + m_fill_a_dx;
		m_fill_a_y = m_fill_a_origin_y + m_fill_a_dy;
	}
	if (m_fill_b_progress < b_path.size())
	{
		m_fill_b_x = m_fill_b_origin_x + b_path[m_fill_b_progress].x;
		m_fill_b_y = m_fill_b_origin_y + b_path[m_fill_b_progress].y;
	}
	else
	{
		m_fill_b_x = m_fill_b_origin_x + m_fill_b_dx;
		m_fill_b_y = m_fill_b_origin_y + m_fill_b_dy;
	}
}


// Load one routed bus bit into a Viper source or mask register.
void dc323_device::load_viper_bit(unsigned plane, unsigned target,
								  unsigned word_bit, bool data)
{
	auto set_register_bit = [this, plane, word_bit, data](unsigned reg)
	{
		u16 &value = m_viper_registers[plane][reg];
		value = (value & ~u16(1U << word_bit)) | (u16(data) << word_bit);
	};

	switch (target & 3)
	{
	case 1: set_register_bit(VIPER_REG_SOURCE); break;
	case 2:
		set_register_bit(VIPER_REG_MASK_1);
		set_register_bit(VIPER_REG_MASK_2);
		break;
	case 3: set_register_bit(VIPER_REG_MASK_2); break;
	}
}


// Route a Source 1 or Source 2 cycle through the selected Viper CSR controls.
void dc323_device::route_source_cycle(
		unsigned csr, unsigned destination_x,
		std::array<bool, BITMAP_PLANES> const &source_bits)
{
	bool bus_valid = false;
	bool bus_data = false;
	for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
	{
		if (!BIT(m_viper_select, plane))
			continue;
		u16 const control =
				m_viper_registers[plane][VIPER_REG_CSR_BASE + (csr & 7)];
		if (BIT(control, 4) && !bus_valid)
		{
			bus_valid = true;
			bus_data = source_bits[plane];
		}
	}

	for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
	{
		if (!BIT(m_viper_select, plane))
			continue;
		auto const &viper = m_viper_registers[plane];
		u16 const control = viper[VIPER_REG_CSR_BASE + (csr & 7)];
		unsigned const bus_width = viper_bus_width(viper[VIPER_REG_BUS_WIDTH]);
		unsigned const word_bit = destination_x & (bus_width - 1);
		load_viper_bit(plane, (control >> 2) & 3, word_bit, source_bits[plane]);
		if (bus_valid)
			load_viper_bit(plane, control & 3, word_bit, bus_data);
	}
}


// Apply Viper masks and logic, then write a single destination-plane pixel.
void dc323_device::write_viper_pixel(unsigned plane, unsigned x, unsigned y,
									 u16 command, bool source_bit)
{
	auto const &viper = m_viper_registers[plane];
	unsigned const bus_width = viper_bus_width(viper[VIPER_REG_BUS_WIDTH]);
	unsigned const word_bit = x & (bus_width - 1);
	u8 const logic = u8(viper[VIPER_REG_LOGIC_BASE + ((command >> 4) & 3)]);
	bool mask1 = BIT(viper[VIPER_REG_MASK_1], word_bit);
	bool mask2 = BIT(viper[VIPER_REG_MASK_2], word_bit);
	if (BIT(logic, 4))
		mask1 = !mask1;
	if (BIT(logic, 5))
		mask2 = !mask2;
	if (!mask1 || !mask2)
		return;
	if (!BIT(logic, 6))
		source_bit = !source_bit;
	u32 const destination = device_bitmap_plane_address(plane, x, y);
	bool const destination_bit = m_bitmap_planes[destination];
	unsigned const logic_input = (destination_bit << 1) | source_bit;
	bool const select_foreground = BIT(logic, logic_input);
	unsigned const color_register =
			select_foreground ? VIPER_REG_FOREGROUND : VIPER_REG_BACKGROUND;
	bool const result = BIT(viper[color_register], word_bit);
	if (destination_bit != result)
	{
		m_bitmap_planes[destination] = result;
		m_display_dirty = true;
	}
}


// Apply the Viper edge masks to a pixel inside the active scroll words.
bool dc323_device::scroll_mask_allows(unsigned plane, u16 x) const
{
	u16 const word_width = m_active_scroll_word_width[plane];
	u16 const left_word = m_active_scroll_x_min & ~(word_width - 1);
	u16 const right_word = m_active_scroll_x_max & ~(word_width - 1);
	if ((x < left_word) || (x >= right_word + word_width))
		return false;

	unsigned const bit = x & (word_width - 1);
	if (x < left_word + word_width)
		return !BIT(m_active_scroll_left[plane], bit);
	if (x >= right_word)
		return !BIT(m_active_scroll_right[plane], bit);
	return true;
}


// Snapshot a pending scroll command and choose its frame or scanline execution
// path.
void dc323_device::begin_scroll()
{
	if (!m_scroll_pending)
		return;
	m_scroll_pending = false;
	m_active_scroll = m_registers[REG_SCROLL_Y];
	m_active_scroll_x_min = std::min<u16>(
			m_registers[REG_SCROLL_X_MIN] & SCROLL_X_MASK, BITMAP_WIDTH);
	m_active_scroll_x_max = std::min<u16>(
			m_registers[REG_SCROLL_X_MAX] & SCROLL_X_MASK, BITMAP_WIDTH);
	m_active_scroll_y_min = std::min<u16>(
			m_registers[REG_SCROLL_Y_MIN] & FIELD_13_BIT_MASK, BITMAP_HEIGHT);
	m_active_scroll_y_max = std::min<u16>(
			m_registers[REG_SCROLL_Y_MAX] & FIELD_13_BIT_MASK, BITMAP_HEIGHT);
	m_active_scroll_y_offset = m_active_y_offset;
	m_active_scroll_line = m_active_scroll_y_min;
	for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
	{
		auto const &viper = m_viper_registers[plane];
		m_active_scroll_fill[plane] = viper[VIPER_REG_FILL];
		m_active_scroll_left[plane] = viper[VIPER_REG_SCROLL_LEFT];
		m_active_scroll_right[plane] = viper[VIPER_REG_SCROLL_RIGHT];
		m_active_scroll_word_width[plane] =
				viper_bus_width(viper[VIPER_REG_BUS_WIDTH]);
	}
	u16 const distance = (m_active_scroll_y_min < m_active_scroll_y_max)
								 ? std::min<u16>(m_active_scroll & 0x0fff,
												 m_active_scroll_y_max -
														 m_active_scroll_y_min)
								 : 0;
	bool const erase = BIT(m_active_scroll, 13);
	DC323_LOGMASKED(
			LOG_SCROLL,
			"%s %s: execute command=%04X down=%u distance=%u erase=%u x=%u..%u "
			"y=%u..%u offset=%u pending=0 status=%04X enable=%04X\n",
			machine().time().to_string(), tag(), m_active_scroll,
			BIT(m_active_scroll, 12), distance, erase, m_active_scroll_x_min,
			m_active_scroll_x_max, m_active_scroll_y_min, m_active_scroll_y_max,
			m_active_scroll_y_offset, m_status,
			m_registers[REG_INTERRUPT_ENABLE]);
	if ((m_active_scroll_x_min >= m_active_scroll_x_max) ||
		(m_active_scroll_y_min >= m_active_scroll_y_max) ||
		(!erase && !distance))
		return;
	m_scroll_active = true;
	if (BIT(m_active_scroll, 12) && !erase)
	{
		execute_down_scroll(distance);
		m_scroll_active = false;
	}
}


// Complete any scanlines left when a new frame boundary is reached.
void dc323_device::finish_scroll()
{
	while (m_scroll_active && (m_active_scroll_line < m_active_scroll_y_max))
		execute_scroll_line(m_active_scroll_line++);
	m_scroll_active = false;
}


// Move and optionally erase one upward-scroll bitmap row behind the raster.
void dc323_device::execute_scroll_line(u16 device_y)
{
	m_display_dirty = true;
	bool const erase = BIT(m_active_scroll, 13);
	u16 const distance =
			std::min<u16>(m_active_scroll & 0x0fff,
						  m_active_scroll_y_max - m_active_scroll_y_min);

	for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
	{
		u16 const word_width = m_active_scroll_word_width[plane];
		u16 const left_word = m_active_scroll_x_min & ~(word_width - 1);
		u16 const right_word = m_active_scroll_x_max & ~(word_width - 1);
		u16 const operation_x_min = erase ? m_active_scroll_x_min : left_word;
		u16 const operation_x_max =
				erase ? m_active_scroll_x_max
					  : std::min<u16>(right_word + word_width, BITMAP_WIDTH);

		u16 const destination_bitmap_y =
				(device_y + m_active_scroll_y_offset) % m_active_y_limit;
		bool const fill =
				erase || (device_y >= m_active_scroll_y_max - distance);
		u16 const source_y = device_y + distance;
		u16 const source_bitmap_y =
				(source_y + m_active_scroll_y_offset) % m_active_y_limit;
		for (u16 x = operation_x_min; x < operation_x_max; x++)
			if (erase || scroll_mask_allows(plane, x))
				m_bitmap_planes[bitmap_plane_address(plane, x,
													 destination_bitmap_y)] =
						fill ? BIT(m_active_scroll_fill[plane], x & 15)
							 : m_bitmap_planes[bitmap_plane_address(
									   plane, x, source_bitmap_y)];
	}
}


// Rotate the refresh ring and backing rows for a downward scroll.
void dc323_device::execute_down_scroll(u16 distance)
{
	m_display_dirty = true;
	// A down-scroll cannot copy the region from top to bottom without replacing
	// data before refresh has read it.  The Adder instead advances all bitmap
	// data outside the region by the scroll distance and lowers PYOF by the
	// same amount.  With refresh starting at the new offset, the untouched data
	// in the region appears to move down while everything outside it stays
	// fixed.
	//
	// Firmware has already supplied the new PYOF.  Save the rows that wrap
	// while rotating the rest of the refresh store toward lower addresses,
	// matching the hardware's reserved down-scroll scanlines.
	std::vector<u8> wrapped(BITMAP_PLANES * distance * BITMAP_WIDTH);
	for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
		for (u16 y = 0; y < distance; y++)
			std::copy_n(&m_bitmap_planes[bitmap_plane_address(plane, 0, y)],
						BITMAP_WIDTH,
						&wrapped[(plane * distance + y) * BITMAP_WIDTH]);

	for (unsigned plane = 0; plane < BITMAP_PLANES; plane++)
	{
		for (u16 destination_y = 0; destination_y < m_active_y_limit;
			 destination_y++)
		{
			u16 const device_y = (destination_y + m_active_y_limit -
								  m_active_scroll_y_offset) %
								 m_active_y_limit;
			u16 const source_y = (destination_y + distance) % m_active_y_limit;
			for (u16 x = 0; x < BITMAP_WIDTH; x++)
			{
				bool const in_scroll_region =
						(device_y >= m_active_scroll_y_min) &&
						(device_y < m_active_scroll_y_max) &&
						scroll_mask_allows(plane, x);
				u32 const destination =
						bitmap_plane_address(plane, x, destination_y);
				if (in_scroll_region)
				{
					if (device_y < m_active_scroll_y_min + distance)
						m_bitmap_planes[destination] =
								BIT(m_active_scroll_fill[plane], x & 15);
				}
				else if (source_y < distance)
				{
					m_bitmap_planes[destination] =
							wrapped[(plane * distance + source_y) *
											BITMAP_WIDTH +
									x];
				}
				else
				{
					m_bitmap_planes[destination] =
							m_bitmap_planes[bitmap_plane_address(plane, x,
																 source_y)];
				}
			}
		}
	}
}


// Read a pixel by physical bitmap-memory coordinates.
u8 dc323_device::pixel_r(unsigned plane, unsigned x, unsigned y) const
{
	return m_bitmap_planes[bitmap_plane_address(plane, x, y)];
}


// Read a displayed pixel after applying the active PYOF address translation.
u8 dc323_device::display_pixel_r(unsigned plane, unsigned x, unsigned y) const
{
	u16 const memory_y = (y + m_active_y_offset) % m_active_y_limit;
	return m_bitmap_planes[bitmap_plane_address(plane, x, memory_y)];
}


// Return and clear the accumulated display-backend invalidation flag.
bool dc323_device::consume_display_dirty()
{
	return std::exchange(m_display_dirty, false);
}


// Initialize every bitmap plane to a uniform value and invalidate the display.
void dc323_device::bitmap_clear(u8 data)
{
	std::fill_n(&m_bitmap_planes[0], BITMAP_PLANES * BITMAP_PLANE_SIZE, data);
	m_display_dirty = true;
}

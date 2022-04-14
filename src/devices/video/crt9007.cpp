// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    SMC CRT9007 CRT Video Processor and Controller (VPAC) emulation

**********************************************************************/

/*

    TODO:

    - cursor timer
    - light pen
    - non-DMA mode
    - DMA mode
    - cursor/blank skew
    - sequential breaks
    - interlaced mode
    - smooth scroll
    - page blank
    - double height cursor
    - row attributes
    - pin configuration
    - operation modes 0,4,7
    - address modes 1,2,3
    - light pen
    - state saving

*/

#include "emu.h"
#include "crt9007.h"

#include "screen.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CRT9007, crt9007_device, "crt9007", "SMC CRT9007 VPAC")



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define HAS_VALID_PARAMETERS \
	(m_reg[0x00] && m_reg[0x01] && m_reg[0x07] && m_reg[0x08] && m_reg[0x09])

#define CHARACTERS_PER_HORIZONTAL_PERIOD \
	m_reg[0x00]

#define CHARACTERS_PER_DATA_ROW \
	(m_reg[0x01] + 1)

#define HORIZONTAL_DELAY \
	m_reg[0x02]

#define HORIZONTAL_SYNC_WIDTH \
	m_reg[0x03]

#define VERTICAL_SYNC_WIDTH \
	m_reg[0x04]

#define VERTICAL_DELAY \
	(m_reg[0x05] - 1)

#define PIN_CONFIGURATION \
	(m_reg[0x06] >> 6)

#define CURSOR_SKEW \
	((m_reg[0x06] >> 3) & 0x07)

#define BLANK_SKEW \
	(m_reg[0x06] & 0x07)

#define VISIBLE_DATA_ROWS_PER_FRAME \
	(m_reg[0x07] + 1)

#define SCAN_LINES_PER_DATA_ROW \
	((m_reg[0x08] & 0x1f) + 1)

#define SCAN_LINES_PER_FRAME \
	(((m_reg[0x08] << 3) & 0x0700) | m_reg[0x09])

#define DMA_BURST_COUNT \
	((m_reg[0x0a] & 0x0f) + 1)

#define DMA_BURST_DELAY \
	((((m_reg[0x0a] >> 4) & 0x07) + 1) % 8)

#define DMA_DISABLE \
	BIT(m_reg[0x0a], 7)

#define SINGLE_HEIGHT_CURSOR \
	BIT(m_reg[0x0b], 0)

#define OPERATION_MODE \
	((m_reg[0x0b] >> 1) & 0x07)

#define INTERLACE_MODE \
	((m_reg[0x0b] >> 4) & 0x03)

#define PAGE_BLANK \
	BIT(m_reg[0x0b], 6)

#define TABLE_START \
	(((m_reg[0x0d] << 8) & 0x3f00) | m_reg[0x0c])

#define ADDRESS_MODE \
	((m_reg[0x0d] >> 6) & 0x03)

#define AUXILIARY_ADDRESS_1 \
	(((m_reg[0x0f] << 8) & 0x3f00) | m_reg[0x0e])

#define ROW_ATTRIBUTES_1 \
	((m_reg[0x0f] >> 6) & 0x03)

#define SEQUENTIAL_BREAK_1 \
	m_reg[0x10]

#define SEQUENTIAL_BREAK_2 \
	m_reg[0x12]

#define DATA_ROW_START \
	m_reg[0x11]

#define DATA_ROW_END \
	m_reg[0x12]

#define AUXILIARY_ADDRESS_2 \
	(((m_reg[0x14] << 8) & 0x3f00) | m_reg[0x13])

#define ROW_ATTRIBUTES_2 \
	((m_reg[0x14] >> 6) & 0x03)

#define SMOOTH_SCROLL_OFFSET \
	((m_reg[0x17] >> 1) & 0x3f)

#define SMOOTH_SCROLL_OFFSET_OVERFLOW \
	BIT(m_reg[0x17], 7)

#define VERTICAL_CURSOR \
	m_reg[0x18]

#define HORIZONTAL_CURSOR \
	m_reg[0x19]

#define INTERRUPT_ENABLE \
	m_reg[0x1a]

#define FRAME_TIMER \
	BIT(m_reg[0x1a], 0)

#define LIGHT_PEN_INTERRUPT \
	BIT(m_reg[0x1a], 5)

#define VERTICAL_RETRACE_INTERRUPT \
	BIT(m_reg[0x1a], 6)

#define VERTICAL_LIGHT_PEN \
	m_reg[0x3b]

#define HORIZONTAL_LIGHT_PEN \
	m_reg[0x3c]


// interlace
enum
{
	NON_INTERLACED = 0,
	ENHANCED_VIDEO_INTERFACE,
	NORMAL_VIDEO_INTERFACE
};


// operation modes
enum
{
	OPERATION_MODE_REPETITIVE_MEMORY_ADDRESSING = 0,    // not implemented
	OPERATION_MODE_DOUBLE_ROW_BUFFER = 1,
	OPERATION_MODE_SINGLE_ROW_BUFFER = 4,               // not implemented
	OPERATION_MODE_ATTRIBUTE_ASSEMBLE = 7               // not implemented
};


// addressing modes
enum
{
	ADDRESS_MODE_SEQUENTIAL_ADDRESSING = 0,
	ADDRESS_MODE_SEQUENTIAL_ROLL_ADDRESSING,            // not implemented
	ADDRESS_MODE_CONTIGUOUS_ROW_TABLE,                  // not implemented
	ADDRESS_MODE_LINKED_LIST_ROW_TABLE                  // not implemented
};


// interrupt enable register bits
const int IE_VERTICAL_RETRACE           = 0x40;
//const int IE_LIGHT_PEN                  = 0x20;
const int IE_FRAME_TIMER                = 0x01;

// status register bits
const int STATUS_INTERRUPT_PENDING      = 0x80;
//const int STATUS_VERTICAL_RETRACE       = 0x40;
const int STATUS_LIGHT_PEN_UPDATE       = 0x20;
//const int STATUS_ODD_EVEN               = 0x04;
//const int STATUS_FRAME_TIMER_OCCURRED   = 0x01;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// default address map
void crt9007_device::crt9007(address_map &map)
{
	if (!has_configured_map(0))
		map(0x0000, 0x3fff).ram();
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  readbyte - read a byte at the given address
//-------------------------------------------------

inline uint8_t crt9007_device::readbyte(offs_t address)
{
	return space().read_byte(address);
}


//-------------------------------------------------
//  trigger_interrupt -
//-------------------------------------------------

inline void crt9007_device::trigger_interrupt(int line)
{
	int status = m_status;

	m_status |= line;

	if (INTERRUPT_ENABLE & line)
	{
		m_status |= STATUS_INTERRUPT_PENDING;

		if (!(status & STATUS_INTERRUPT_PENDING))
		{
			LOG("CRT9007 INT 1\n");
			m_write_int(ASSERT_LINE);
		}
	}
}


//-------------------------------------------------
//  update_cblank_line -
//-------------------------------------------------

inline void crt9007_device::update_cblank_line()
{
	int x = screen().hpos();
	int y = screen().vpos();

	// composite blank
	bool cblank = !(m_hs && m_vs);

	if (m_cblank != cblank)
	{
		m_cblank = cblank;

		LOG("CRT9007 y %03u x %04u : CBLANK %u\n", y, x, m_cblank);

		m_write_cblank(m_cblank);
	}
}


//-------------------------------------------------
//  update_hsync_timer -
//-------------------------------------------------

inline void crt9007_device::update_hsync_timer(bool state)
{
	int y = screen().vpos();

	int next_x = state ? m_hsync_start : m_hsync_end;
	int next_y = state ? (y + 1) % SCAN_LINES_PER_FRAME : y;

	attotime duration = screen().time_until_pos(next_y, next_x);

	m_hsync_timer->adjust(duration, !state);
}


//-------------------------------------------------
//  update_vsync_timer -
//-------------------------------------------------

inline void crt9007_device::update_vsync_timer(bool state)
{
	int next_y = state ? m_vsync_start : m_vsync_end;

	attotime duration = screen().time_until_pos(next_y, 0);

	m_vsync_timer->adjust(duration, !state);
}


//-------------------------------------------------
//  update_vlt_timer -
//-------------------------------------------------

inline void crt9007_device::update_vlt_timer(bool state)
{
	// this signal is active during all visible scan lines and during the horizontal trace at vertical retrace
	int y = screen().vpos();

	int next_x = state ? m_vlt_end : m_vlt_start;
	int next_y = state ? y : ((y == m_vlt_bottom) ? 0 : (y + 1));

	attotime duration = screen().time_until_pos(next_y, next_x);

	m_vlt_timer->adjust(duration, !state);
}


//-------------------------------------------------
//  update_curs_timer -
//-------------------------------------------------

inline void crt9007_device::update_curs_timer(bool state)
{
	// this signal is active for 1 character time for all scanlines within the data row
	// TODO
}


//-------------------------------------------------
//  update_drb_timer -
//-------------------------------------------------

inline void crt9007_device::update_drb_timer(bool state)
{
	// this signal is active for 1 full scan line (VLT edge to edge) at the top scan line of each new row
	// there is 1 extra DRB signal during the 1st scanline of the vertical retrace interval
	int y = screen().vpos();

	int next_x = m_vlt_end;
	int next_y = y ? y + 1 : y;

	if (state)
	{
		if (y == 0)
		{
			next_y = VERTICAL_DELAY - 1;
		}
		else if (y == m_drb_bottom)
		{
			next_x = 0;
			next_y = 0;
		}
		else
		{
			next_y = y + SCAN_LINES_PER_DATA_ROW - 1;
		}
	}

	attotime duration = screen().time_until_pos(next_y, next_x);

	m_drb_timer->adjust(duration, !state);
}


//-------------------------------------------------
//  update_dma_timer -
//-------------------------------------------------

inline void crt9007_device::update_dma_timer()
{
	// TODO
}


//-------------------------------------------------
//  recompute_parameters -
//-------------------------------------------------

inline void crt9007_device::recompute_parameters()
{
	// check that necessary registers have been loaded
	if (!HAS_VALID_PARAMETERS) return;

	// screen dimensions
	int horiz_pix_total = CHARACTERS_PER_HORIZONTAL_PERIOD * m_hpixels_per_column;
	int vert_pix_total = SCAN_LINES_PER_FRAME;

	// refresh rate
	attotime refresh = clocks_to_attotime(CHARACTERS_PER_HORIZONTAL_PERIOD * vert_pix_total);

	// horizontal sync
	m_hsync_start = 0;
	m_hsync_end = HORIZONTAL_SYNC_WIDTH * m_hpixels_per_column;

	// visible line time
	m_vlt_start = HORIZONTAL_DELAY * m_hpixels_per_column;
	m_vlt_end = (HORIZONTAL_DELAY + CHARACTERS_PER_DATA_ROW) * m_hpixels_per_column;
	m_vlt_bottom = VERTICAL_DELAY + (VISIBLE_DATA_ROWS_PER_FRAME * SCAN_LINES_PER_DATA_ROW) - 1;

	// data row boundary
	m_drb_bottom = VERTICAL_DELAY + (VISIBLE_DATA_ROWS_PER_FRAME * SCAN_LINES_PER_DATA_ROW) - SCAN_LINES_PER_DATA_ROW;

	// vertical sync
	m_vsync_start = 0;
	m_vsync_end = VERTICAL_SYNC_WIDTH;

	// visible area
	rectangle visarea(m_hsync_end, horiz_pix_total - 1, m_vsync_end, vert_pix_total - 1);

	LOG("CRT9007 Screen: %u x %u @ %f Hz\n", horiz_pix_total, vert_pix_total, refresh.as_hz());
	LOG("CRT9007 Visible Area: (%u, %u) - (%u, %u)\n", visarea.min_x, visarea.min_y, visarea.max_x, visarea.max_y);

	//screen().configure(horiz_pix_total, vert_pix_total, visarea, refresh.as_attoseconds());
	(void)visarea;

	m_hsync_timer->adjust(screen().time_until_pos(0, 0));
	m_vsync_timer->adjust(screen().time_until_pos(0, 0));
	m_vlt_timer->adjust(screen().time_until_pos(0, m_vlt_start), 1);
	m_drb_timer->adjust(screen().time_until_pos(0, 0));

	int frame_timer_line = m_drb_bottom - (OPERATION_MODE == OPERATION_MODE_DOUBLE_ROW_BUFFER ? SCAN_LINES_PER_DATA_ROW : 0);
	m_frame_timer->adjust(screen().time_until_pos(frame_timer_line, 0), 0, refresh);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  crt9007_device - constructor
//-------------------------------------------------

crt9007_device::crt9007_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CRT9007, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_space_config("videoram", ENDIANNESS_LITTLE, 8, 14, 0, address_map_constructor(FUNC(crt9007_device::crt9007), this)),
	m_write_int(*this),
	m_write_dmar(*this),
	m_write_hs(*this),
	m_write_vs(*this),
	m_write_vlt(*this),
	m_write_curs(*this),
	m_write_drb(*this),
	m_write_wben(*this),
	m_write_cblank(*this),
	m_write_slg(*this),
	m_write_sld(*this)
{
	for (auto & elem : m_reg)
		elem = 0;
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void crt9007_device::device_resolve_objects()
{
	// resolve callbacks
	m_write_int.resolve_safe();
	m_write_dmar.resolve_safe();
	m_write_hs.resolve_safe();
	m_write_vs.resolve_safe();
	m_write_vlt.resolve_safe();
	m_write_curs.resolve_safe();
	m_write_drb.resolve_safe();
	m_write_wben.resolve_safe();
	m_write_cblank.resolve_safe();
	m_write_slg.resolve_safe();
	m_write_sld.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void crt9007_device::device_start()
{
	// allocate timers
	m_hsync_timer = timer_alloc(TIMER_HSYNC);
	m_vsync_timer = timer_alloc(TIMER_VSYNC);
	m_vlt_timer = timer_alloc(TIMER_VLT);
	m_curs_timer = timer_alloc(TIMER_CURS);
	m_drb_timer = timer_alloc(TIMER_DRB);
	m_dma_timer = timer_alloc(TIMER_DMA);
	m_frame_timer = timer_alloc(TIMER_FRAME);

	// save state
	save_item(NAME(m_reg));
	save_item(NAME(m_status));
	save_item(NAME(m_hpixels_per_column));
	save_item(NAME(m_disp));
	save_item(NAME(m_hs));
	save_item(NAME(m_vs));
	save_item(NAME(m_cblank));
	save_item(NAME(m_vlt));
	save_item(NAME(m_drb));
	save_item(NAME(m_lpstb));
	save_item(NAME(m_dmar));
	save_item(NAME(m_ack));
	save_item(NAME(m_dma_count));
	save_item(NAME(m_dma_burst));
	save_item(NAME(m_dma_delay));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void crt9007_device::device_reset()
{
	m_disp = false;
	m_cblank = false;
	m_status = 0;

	// HS = 1
	m_hs = true;
	m_write_hs(1);

	// VS = 1
	m_vs = true;
	m_write_vs(1);

	// CBLANK = 1
	m_write_cblank(0);

	// CURS = 0
	m_write_curs(0);

	// VLT = 0
	m_write_vlt(0);

	// DRB = 1
	m_write_drb(1);

	// INT = 0
	m_write_int(CLEAR_LINE);

	// 28 (DMAR) = 0
	m_dmar = false;
	m_write_dmar(CLEAR_LINE);

	// 29 (WBEN) = 0
	m_write_wben(1); // HACK

	// 30 (SLG) = 0
	m_write_slg(0);

	// 31 (SLD) = 0
	m_write_sld(0);

	// 32 (LPSTB) = 0
}


//-------------------------------------------------
//  device_post_load - called after the loading a
//  saved state, so that registered variables can
//  be expanded as necessary
//-------------------------------------------------

void crt9007_device::device_post_load()
{
	recompute_parameters();
}


//-------------------------------------------------
//  device_clock_changed - handle clock change
//-------------------------------------------------

void crt9007_device::device_clock_changed()
{
	recompute_parameters();
}


//-------------------------------------------------
//  device_timer - handle timer events
//-------------------------------------------------

void crt9007_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	int x = screen().hpos();
	int y = screen().vpos();

	switch (id)
	{
	case TIMER_HSYNC:
		m_hs = bool(param);

		LOG("CRT9007 y %03u x %04u : HS %u\n", y, x, m_hs);

		m_write_hs(m_hs);

		update_cblank_line();

		update_hsync_timer(m_hs);
		break;

	case TIMER_VSYNC:
		m_vs = bool(param);

		LOG("CRT9007 y %03u x %04u : VS %u\n", y, x, m_vs);

		m_write_vs(m_vs);

		if (m_vs)
		{
			// reset all other bits except Light Pen Update to logic 0
			m_status &= (STATUS_LIGHT_PEN_UPDATE | STATUS_INTERRUPT_PENDING);
		}
		else
		{
			trigger_interrupt(IE_VERTICAL_RETRACE);

			update_cblank_line();
		}

		update_vsync_timer(m_vs);
		break;

	case TIMER_VLT:
		m_vlt = bool(param);

		LOG("CRT9007 y %03u x %04u : VLT %u\n", y, x, m_vlt);

		m_write_vlt(m_vlt);

		update_vlt_timer(m_vlt);
		break;

	case TIMER_CURS:
		LOG("CRT9007 y %03u x %04u : CURS %u\n", y, x, param);

		m_write_curs(param);

		update_curs_timer(param);
		break;

	case TIMER_DRB:
		m_drb = bool(param);

		LOG("CRT9007 y %03u x %04u : DRB %u\n", y, x, m_drb);

		m_write_drb(m_drb);

		if (!m_drb && !DMA_DISABLE)
		{
			// start DMA burst sequence
			m_dma_count = CHARACTERS_PER_DATA_ROW;
			m_dma_burst = DMA_BURST_COUNT ? (DMA_BURST_COUNT * 4) : CHARACTERS_PER_DATA_ROW;
			m_dma_delay = DMA_BURST_DELAY;
			m_dmar = true;

			LOG("CRT9007 DMAR 1\n");
			m_write_dmar(ASSERT_LINE);
		}

		update_drb_timer(m_drb);
		break;

	case TIMER_DMA:
		readbyte(AUXILIARY_ADDRESS_2);

		update_dma_timer();
		break;

	case TIMER_FRAME:
		trigger_interrupt(IE_FRAME_TIMER);
		break;
	}
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector crt9007_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

uint8_t crt9007_device::read(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0x15:
		if (!machine().side_effects_disabled())
		{
			LOG("CRT9007 Start\n");
			m_disp = true;
		}
		break;

	case 0x16:
		if (!machine().side_effects_disabled())
		{
			LOG("CRT9007 Reset\n");
			device_reset();
		}
		break;

	case 0x38:
		data = VERTICAL_CURSOR;
		break;

	case 0x39:
		data = HORIZONTAL_CURSOR;
		break;

	case 0x3a:
		data = m_status;

		if (!machine().side_effects_disabled() && (m_status & STATUS_INTERRUPT_PENDING))
		{
			// reset interrupt pending bit
			m_status &= ~STATUS_INTERRUPT_PENDING;
			LOG("CRT9007 INT 0\n");
			m_write_int(CLEAR_LINE);
		}
		break;

	case 0x3b:
		data = VERTICAL_LIGHT_PEN;
		break;

	case 0x3c:
		data = HORIZONTAL_LIGHT_PEN;

		if (!machine().side_effects_disabled())
		{
			// reset light pen update bit
			m_status &= ~STATUS_LIGHT_PEN_UPDATE;
		}
		break;

	default:
		if (!machine().side_effects_disabled())
			logerror("CRT9007 Read from Invalid Register: %02x!\n", offset);
	}

	return data;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void crt9007_device::write(offs_t offset, uint8_t data)
{
	m_reg[offset] = data;

	switch (offset)
	{
	case 0x00:
		recompute_parameters();
		LOG("CRT9007 Characters per Horizontal Period: %u\n", CHARACTERS_PER_HORIZONTAL_PERIOD);
		break;

	case 0x01:
		recompute_parameters();
		LOG("CRT9007 Characters per Data Row: %u\n", CHARACTERS_PER_DATA_ROW);
		break;

	case 0x02:
		recompute_parameters();
		LOG("CRT9007 Horizontal Delay: %u\n", HORIZONTAL_DELAY);
		break;

	case 0x03:
		recompute_parameters();
		LOG("CRT9007 Horizontal Sync Width: %u\n", HORIZONTAL_SYNC_WIDTH);
		break;

	case 0x04:
		recompute_parameters();
		LOG("CRT9007 Vertical Sync Width: %u\n", VERTICAL_SYNC_WIDTH);
		break;

	case 0x05:
		recompute_parameters();
		LOG("CRT9007 Vertical Delay: %u\n", VERTICAL_DELAY);
		break;

	case 0x06:
		recompute_parameters();
		LOG("CRT9007 Pin Configuration: %u\n", PIN_CONFIGURATION);
		LOG("CRT9007 Cursor Skew: %u\n", CURSOR_SKEW);
		LOG("CRT9007 Blank Skew: %u\n", BLANK_SKEW);
		break;

	case 0x07:
		recompute_parameters();
		LOG("CRT9007 Visible Data Rows per Frame: %u\n", VISIBLE_DATA_ROWS_PER_FRAME);
		break;

	case 0x08:
		recompute_parameters();
		LOG("CRT9007 Scan Lines per Data Row: %u\n", SCAN_LINES_PER_DATA_ROW);
		break;

	case 0x09:
		recompute_parameters();
		LOG("CRT9007 Scan Lines per Frame: %u\n", SCAN_LINES_PER_FRAME);
		break;

	case 0x0a:
		LOG("CRT9007 DMA Burst Count: %u\n", DMA_BURST_COUNT);
		LOG("CRT9007 DMA Burst Delay: %u\n", DMA_BURST_DELAY);
		LOG("CRT9007 DMA Disable: %u\n", DMA_DISABLE);
		break;

	case 0x0b:
		LOG("CRT9007 %s Height Cursor\n", SINGLE_HEIGHT_CURSOR ? "Single" : "Double");
		LOG("CRT9007 Operation Mode: %u\n", OPERATION_MODE);
		LOG("CRT9007 Interlace Mode: %u\n", INTERLACE_MODE);
		LOG("CRT9007 %s Mechanism\n", PAGE_BLANK ? "Page Blank" : "Smooth Scroll");
		break;

	case 0x0c:
		break;

	case 0x0d:
		LOG("CRT9007 Table Start Register: %04x\n", TABLE_START);
		LOG("CRT9007 Address Mode: %u\n", ADDRESS_MODE);
		break;

	case 0x0e:
		break;

	case 0x0f:
		LOG("CRT9007 Auxialiary Address Register 1: %04x\n", AUXILIARY_ADDRESS_1);
		LOG("CRT9007 Row Attributes: %u\n", ROW_ATTRIBUTES_1);
		break;

	case 0x10:
		LOG("CRT9007 Sequential Break Register 1: %u\n", SEQUENTIAL_BREAK_1);
		break;

	case 0x11:
		LOG("CRT9007 Data Row Start Register: %u\n", DATA_ROW_START);
		break;

	case 0x12:
		LOG("CRT9007 Data Row End/Sequential Break Register 2: %u\n", SEQUENTIAL_BREAK_2);
		break;

	case 0x13:
		break;

	case 0x14:
		LOG("CRT9007 Auxiliary Address Register 2: %04x\n", AUXILIARY_ADDRESS_2);
		LOG("CRT9007 Row Attributes: %u\n", ROW_ATTRIBUTES_2);
		break;

	case 0x15:
		LOG("CRT9007 Start\n");
		m_disp = true;
		break;

	case 0x16:
		LOG("CRT9007 Reset\n");
		device_reset();
		break;

	case 0x17:
		LOG("CRT9007 Smooth Scroll Offset: %u\n", SMOOTH_SCROLL_OFFSET);
		LOG("CRT9007 Smooth Scroll Offset Overflow: %u\n", SMOOTH_SCROLL_OFFSET_OVERFLOW);
		break;

	case 0x18:
		LOG("CRT9007 Vertical Cursor Register: %u\n", VERTICAL_CURSOR);
		break;

	case 0x19:
		LOG("CRT9007 Horizontal Cursor Register: %u\n", HORIZONTAL_CURSOR);
		break;

	case 0x1a:
		LOG("CRT9007 Frame Timer: %u\n", FRAME_TIMER);
		LOG("CRT9007 Light Pen Interrupt: %u\n", LIGHT_PEN_INTERRUPT);
		LOG("CRT9007 Vertical Retrace Interrupt: %u\n", VERTICAL_RETRACE_INTERRUPT);
		break;

	default:
		logerror("CRT9007 Write to Invalid Register: %02x!\n", offset);
	}
}


//-------------------------------------------------
//  ack_w - DMA acknowledge
//-------------------------------------------------

void crt9007_device::ack_w(int state)
{
	LOG("CRT9007 ACK: %u\n", state);

	if (m_dmar && !m_ack && state)
	{
		// start DMA transfer
		m_dma_timer->adjust(attotime::from_hz(clock()));
	}

	m_ack = bool(state);
}


//-------------------------------------------------
//  lpstb_w - light pen strobe
//-------------------------------------------------

void crt9007_device::lpstb_w(int state)
{
	LOG("CRT9007 LPSTB: %u\n", state);

	if (!m_lpstb && state)
	{
		// TODO latch current row/column position
	}

	m_lpstb = bool(state);
}


//-------------------------------------------------
//  set_character_width -
//-------------------------------------------------

void crt9007_device::set_character_width(unsigned value)
{
	m_hpixels_per_column = value;

	if (started())
		recompute_parameters();
}

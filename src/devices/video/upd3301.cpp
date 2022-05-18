// license:BSD-3-Clause
// copyright-holders:Curt Coder, Angelo Salese
/**********************************************************************

    NEC uPD3301 Programmable CRT Controller emulation

**********************************************************************/

/*

    TODO:

    - pinpoint how much of pc8001/pc8801 drawing functions should actually be inherited
      here;
    - N interrupt (special control character);
    - light pen;
    - reset counters;
    - proper DMA timing (now the whole screen is transferred at the end of the frame,
        accurate timing requires CCLK timer which kills performance)
    - DMA burst mode (reportedly not working in pc8801 arch,
        should return a blank screen for reasons);
    - DMA underrun (sorcerml in pc8801?). Should throw a status U irq;
    - cleanup: variable namings should be more verbose
        (i.e. not be a single letter like m_y, m_z, m_b ...);
    - jettermi (pc8801) expects to colorize its underlying 400 b&w mode by masking with the
        text color attributes here;
    - xak2 (pc8801) throws text garbage on legacy renderer (verify);

*/

#include "emu.h"
#include "upd3301.h"

#include "screen.h"

#define LOG_WARN    (1U << 1) // Illegal setup attempts
#define LOG_CMD     (1U << 2) // Command r/w
#define LOG_CURSOR  (1U << 3) // Cursor position (verbose)
#define LOG_CRTC    (1U << 4) // CRTC parameters
#define LOG_INT     (1U << 5) // INT, VRTC and DRQ lines
#define LOG_HRTC    (1U << 6) // HRTC (verbose)

#define VERBOSE (LOG_WARN)
//#define VERBOSE (LOG_WARN|LOG_CMD)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

#define LOGWARN(...)      LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGCMD(...)       LOGMASKED(LOG_CMD, __VA_ARGS__)
#define LOGCURSOR(...)    LOGMASKED(LOG_CURSOR, __VA_ARGS__)
#define LOGCRTC(...)      LOGMASKED(LOG_CRTC, __VA_ARGS__)
#define LOGINT(...)       LOGMASKED(LOG_INT, __VA_ARGS__)
#define LOGHRTC(...)      LOGMASKED(LOG_HRTC, __VA_ARGS__)


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define COMMAND_MASK                    0xe0
#define COMMAND_RESET                   0x00
#define COMMAND_START_DISPLAY           0x20
#define COMMAND_SET_INTERRUPT_MASK      0x40
#define COMMAND_READ_LIGHT_PEN          0x60    // not supported
#define COMMAND_LOAD_CURSOR_POSITION    0x80
#define COMMAND_RESET_INTERRUPT         0xa0
#define COMMAND_RESET_COUNTERS          0xc0    // not supported


#define STATUS_VE                       0x10
#define STATUS_U                        0x08    // not supported
#define STATUS_N                        0x04    // not supported
#define STATUS_E                        0x02
#define STATUS_LP                       0x01    // not supported


enum
{
	MODE_NONE,
	MODE_RESET,
	MODE_READ_LIGHT_PEN,
	MODE_LOAD_CURSOR_POSITION,
	MODE_RESET_COUNTERS
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(UPD3301, upd3301_device, "upd3301", "NEC uPD3301")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  upd3301_device - constructor
//-------------------------------------------------

upd3301_device::upd3301_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, UPD3301, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_write_int(*this)
	, m_write_drq(*this)
	, m_write_hrtc(*this)
	, m_write_vrtc(*this)
	, m_write_rvv(*this)
	, m_display_cb(*this)
	, m_attr_fetch_cb(*this)
	, m_width(0)
	, m_status(0)
	, m_param_count(0)
	, m_data_fifo_pos(0)
	, m_attr_fifo_pos(0)
	, m_input_fifo(0)
	, m_me(0)
	, m_h(80)
	, m_l(20)
	, m_r(10)
	, m_v(6)
	, m_z(32)
	, m_attr_blink(0)
	, m_attr_frame(0)
	, m_cm(0)
	, m_cx(0)
	, m_cy(0)
	, m_cursor_blink(0)
	, m_cursor_frame(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd3301_device::device_start()
{
	screen().register_screen_bitmap(m_bitmap);
	// resolve callbacks
	m_write_drq.resolve_safe();
	m_write_int.resolve_safe();
	m_write_hrtc.resolve_safe();
	m_write_vrtc.resolve_safe();
	m_write_rvv.resolve();
	m_display_cb.resolve();
	m_attr_fetch_cb.resolve();

	// allocate timers
	m_hrtc_timer = timer_alloc(TIMER_HRTC);
	m_vrtc_timer = timer_alloc(TIMER_VRTC);
	m_drq_timer = timer_alloc(TIMER_DRQ);

	// state saving
	save_item(NAME(m_y));
	save_item(NAME(m_hrtc));
	save_item(NAME(m_vrtc));
	save_item(NAME(m_mode));
	save_item(NAME(m_status));
	save_item(NAME(m_param_count));
	save_item(NAME(m_data_fifo_pos));
	save_item(NAME(m_attr_fifo_pos));
	save_item(NAME(m_input_fifo));
	save_item(NAME(m_mn));
	save_item(NAME(m_me));
	save_item(NAME(m_dma_mode));
	save_item(NAME(m_h));
	save_item(NAME(m_b));
	save_item(NAME(m_l));
	save_item(NAME(m_s));
	save_item(NAME(m_c));
	save_item(NAME(m_r));
	save_item(NAME(m_v));
	save_item(NAME(m_z));
	save_item(NAME(m_gfx_mode));
	save_item(NAME(m_attr));
	save_item(NAME(m_attr_blink));
	save_item(NAME(m_attr_frame));
	save_item(NAME(m_cm));
	save_item(NAME(m_cx));
	save_item(NAME(m_cy));
	save_item(NAME(m_cursor_blink));
	save_item(NAME(m_cursor_frame));
	save_item(NAME(m_data_fifo));
	save_item(NAME(m_attr_fifo));
	save_item(NAME(m_reverse_display));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd3301_device::device_reset()
{
	set_display(0);
	set_interrupt(0);
	set_drq(0);

	m_cm = 0;
	m_b = 48;
	m_reverse_display = false;
	if (!m_write_rvv.isnull())
		m_write_rvv(m_reverse_display);

	recompute_parameters();
}


//-------------------------------------------------
//  device_clock_changed - handle clock change
//-------------------------------------------------

void upd3301_device::device_clock_changed()
{
	recompute_parameters();
}


//-------------------------------------------------
//  device_timer - handle timer events
//-------------------------------------------------

// this snipped was inside screen_update fn
// bad idea: it causes all sort of desync glitches when emulation unthrottles
// TODO: verify if FIFO clear-out happens on vblank-in or -out
inline void upd3301_device::reset_fifo_vrtc()
{
	m_y = 0;
	m_data_fifo_pos = 0;
	m_attr_fifo_pos = 0;

	m_cursor_frame++;

	if (m_cursor_frame == m_b)
	{
		m_cursor_frame = 0;
		m_cursor_blink = !m_cursor_blink;
	}

	m_attr_frame++;
	if (m_attr_frame == (m_b << 1))
	{
		m_attr_frame = 0;
		m_attr_blink = !m_attr_blink;
	}
	// Clear the buffer here. Needed in particular by PC-88, which usually disables
	// DMA transfer in order to show Graphic layer only (i.e. bugattac).
	m_bitmap.fill(0, screen().visible_area());
}

void upd3301_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
		case TIMER_HRTC:
			LOGHRTC("HRTC: %u at beam %d x %d\n", param, screen().hpos(), screen().vpos());

			m_write_hrtc(param);
			m_hrtc = param;

			update_hrtc_timer(param);
			break;

		case TIMER_VRTC:
			LOGINT("VRTC: %u at y %d\n", param, screen().vpos());

			m_write_vrtc(param);
			m_vrtc = param;

			update_vrtc_timer(param);
			if(!get_display_status())
				break;

			if (!param)
			{
				reset_fifo_vrtc();
				set_drq(1);
			}

			if (param && !m_me)
			{
				m_status |= STATUS_E;
				set_interrupt(1);
			}

			break;

		case TIMER_DRQ:
			break;
	}
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t upd3301_device::read(offs_t offset)
{
	uint8_t data = 0;

	switch (offset & 0x01)
	{
		case 0: // data
			if (!machine().side_effects_disabled())
				popmessage("light pen reading?");
			break;

		case 1: // status
			data = m_status;
			if (!machine().side_effects_disabled())
				m_status &= ~(STATUS_LP | STATUS_E |STATUS_N | STATUS_U);
			break;
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void upd3301_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 0x01)
	{
		case 0: // data
			switch (m_mode)
			{
			case MODE_RESET:
				switch (m_param_count)
				{
				case 0:
					m_dma_mode = BIT(data, 7);
					// number of characters per line -2
					// TODO: doesn't seem to like anything beyond 80
					m_h = (data & 0x7f) + 2;
					if (m_h > 80)
						popmessage("Illegal width set %d", m_h);
					LOGCMD("DMA Mode: %s\n", m_dma_mode ? "character" : "burst");
					LOGCMD("H: %u (characters per line)\n", m_h);
					break;

				case 1:
					// cursor/attribute blink rate
					m_b = ((data >> 6) + 1) * 16;
					// number of lines displayed -1
					// (or in other words, tilemap y size)
					m_l = (data & 0x3f) + 1;
					LOGCMD("B: %u (cursor blink rate)\n", m_b);
					LOGCMD("L: %u (number of lines displayed)\n", m_l);
					break;

				case 2:
					// skip line (pseudo-interlace?)
					m_s = BIT(data, 7);
					if (m_s)
						popmessage("skip line enable");
					// cursor mode
					// (00) not blinking underline cursor
					// (01) blinking underline cursor
					// (10) not blinking solid cursor
					// (11) blinking solid cursor
					// NB: there must be at least 14 lines per char to make underline valid
					m_c = (data >> 5) & 0x03;
					if (m_c != 3)
						popmessage("cursor mode %02x", m_c);
					// Number of lines per character -1
					m_r = (data & 0x1f) + 1;
					LOGCMD("S: %u (skip line)\n", m_s);
					LOGCMD("C: %u (cursor mode)\n", m_c);
					LOGCMD("R: %u (number of lines per character)\n", m_r);
					break;

				case 3:
					// vblank lines -1 (1 to 8)
					m_v = (data >> 5) + 1;
					// hblank width -2 (6 to 33)
					m_z = (data & 0x1f) + 2;
					LOGCMD("V: %u (vblank lines)\n", m_v);
					LOGCMD("Z: %u (hblank width)\n", m_z);
					recompute_parameters();
					break;

				case 4:
					//  AT|SC
					// (00|0) transparent b&w with special control character
					// (00|1) no attributes, no special control
					// (01|0) transparent color
					// (10|0) non-transparent b&w, special control
					// (10|1) non-transparent b&w, no special control
					// any other setting are invalid
					//m_at1 = BIT(data, 7);
					//m_at0 = BIT(data, 6);
					//m_sc = BIT(data, 5);
					m_gfx_mode = (data & 0xe0) >> 5;
					if (m_gfx_mode & 0x4)
						popmessage("attr mode %02x", m_gfx_mode);

					// Max number of attributes per line -1
					// can't be higher than 20
					// overriden to 0 in no attribute/no sc mode
					m_attr = (m_gfx_mode == 1) ? 0 : std::min((data & 0x1f) + 1, 20);
					LOGCMD("AT1: %u AT0: %u SC: %u (gfx mode = %02x)\n",
						BIT(data, 7), BIT(data, 6), BIT(data, 5), m_gfx_mode
					);
					LOGCMD("ATTR: %u (num attributes per line)\n", m_attr);

					m_mode = MODE_NONE;
					break;
				}

				m_param_count++;
				break;

			case MODE_LOAD_CURSOR_POSITION:
				switch (m_param_count)
				{
				case 0:
					m_cx = data & 0x7f;
					LOGCURSOR("CX: %u\n", m_cx);
					break;

				case 1:
					m_cy = data & 0x3f;
					LOGCURSOR("CY: %u\n", m_cy);

					m_mode = MODE_NONE;
					break;
				}

				m_param_count++;
				break;

			default:
				LOGWARN("Invalid Parameter Byte %02x!\n", data);
			}
			break;

		case 1: // command
			m_mode = MODE_NONE;
			m_param_count = 0;

			switch (data & 0xe0)
			{
			case COMMAND_RESET:
				LOGCMD("Reset\n");
				m_mode = MODE_RESET;
				// This should also disable external display such as Graphic layer in PC-8801
				// sorcerml contradicts with it tho: it just issue a reset command without caring about
				// re-enabling display.
				set_display(0);
				set_interrupt(0);
				break;

			case COMMAND_START_DISPLAY:
			{
				LOGCMD("Start Display\n");
				bool new_rvv = bool(BIT(data, 0));
				LOGCMD("RVV: %u (reverse display)\n", new_rvv);
				// misscmd (pc8001) enables this
				if (m_reverse_display != new_rvv)
				{
					m_reverse_display = new_rvv;
					if (!m_write_rvv.isnull())
						m_write_rvv(m_reverse_display);
					else if (m_reverse_display == true)
						logerror("%s: RVV reverse display enabled (warning)\n", machine().describe_context());
				}
				set_display(1);
				reset_counters();
				break;
			}

			case COMMAND_SET_INTERRUPT_MASK:
				LOGCMD("Set Interrupt Mask\n");
				// vblank irq mask
				m_me = BIT(data, 0);
				// special control character irq mask
				m_mn = BIT(data, 1);
				// TODO: Apparently unmasking ME should be reflected in undocumented status bit 7
				LOGCMD("ME: %u (vblank irq mask)\n", m_me);
				LOGCMD("MN: %u (special control char irq mask)\n", m_mn);
				break;

			case COMMAND_READ_LIGHT_PEN:
				LOGCMD("Read Light Pen\n");
				// TODO: similar to cursor parameters except on read
				// (plus an HR to bit 7 param [0])
				m_mode = MODE_READ_LIGHT_PEN;
				break;

			case COMMAND_LOAD_CURSOR_POSITION:
				LOGCURSOR("Load Cursor Position\n");
				m_mode = MODE_LOAD_CURSOR_POSITION;
				// (1) show cursor (0) disable cursor
				m_cm = BIT(data, 0);
				LOGCURSOR("CM: %u\n", m_cm);
				break;

			case COMMAND_RESET_INTERRUPT:
				LOGCMD("Reset Interrupt\n");
				set_interrupt(0);
				break;

			case COMMAND_RESET_COUNTERS:
				LOGCMD("Reset Counters\n");
				m_mode = MODE_RESET_COUNTERS;
				reset_counters();
				break;
			}
			break;
	}
}


//-------------------------------------------------
//  dack_w -
//-------------------------------------------------

void upd3301_device::dack_w(uint8_t data)
{
	// TODO: underrun condition
	if (m_y >= (m_l * m_r))
	{
		return;
	}

	if (m_data_fifo_pos < m_h)
	{
		m_data_fifo[m_input_fifo][m_data_fifo_pos] = data;
		m_data_fifo_pos++;
	}
	else
	{
		m_attr_fifo[m_input_fifo][m_attr_fifo_pos] = data;
		m_attr_fifo_pos++;
	}

	if ((m_data_fifo_pos == m_h) && (m_attr_fifo_pos == (m_attr << 1)))
	{
		const u8 attr_max_size = 80;
		// first attribute start is always overwritten with a 0
		m_attr_fifo[m_input_fifo][0] = 0;
		// last parameter always extends up to the end of the row
		// (7narabe (pc8001) fills last row value with white when exausting available slots)
		m_attr_fifo[m_input_fifo][40] = attr_max_size;
		m_input_fifo = !m_input_fifo;

		m_data_fifo_pos = 0;
		m_attr_fifo_pos = 0;

		draw_scanline();

		if (m_y == (m_l * m_r))
		{
			// end DMA transfer
			set_drq(0);
		}
	}
}


//-------------------------------------------------
//  lpen_w -
//-------------------------------------------------

void upd3301_device::lpen_w(int state)
{
}


//-------------------------------------------------
//  hrtc_r - Horizontal ReTraCe
//-------------------------------------------------

int upd3301_device::hrtc_r()
{
	return m_hrtc;
}


//-------------------------------------------------
//  vrtc_r - Vertical ReTraCe
//-------------------------------------------------

int upd3301_device::vrtc_r()
{
	return m_vrtc;
}


//-------------------------------------------------
//  draw_scanline -
//-------------------------------------------------

UPD3301_FETCH_ATTRIBUTE( upd3301_device::default_attr_fetch )
{
	const u8 attr_max_size = 80;
	std::array<u16, attr_max_size> attr_extend_info = {0};

	// elthlead (pc8801) uses b&w no attributes/no special control mode
	if (m_gfx_mode == 1)
		return attr_extend_info;

	// TODO: may actually fetch in LIFO order
	// Some edge cases in pc8801 N88 Basic (status on bottom), jettermi and play6lim backs up this theory.
	for (int ex = 0; ex < attr_fifo_size; ex+=2)
	{
		u8 attr_start = std::min(attr_row[ex], attr_max_size);
		u8 attr_value = attr_row[ex+1];
		u8 attr_end = std::min(attr_row[ex+2], attr_max_size);
		// if the target is == 0 then just consider max size instead
		// (starfire (pc8001) wants this otherwise will black screen on gameplay)
		if (attr_end == 0)
			attr_end = attr_max_size;

		//printf("%04x %d %d [%02x]\n", ex, attr_start, attr_end, attr_value);

		for (int i = attr_start; i < attr_end; i++)
			attr_extend_info[i] = attr_value;

		if (attr_end == attr_max_size)
			break;
	}

	return attr_extend_info;
}

void upd3301_device::draw_scanline()
{
	// Olympia Boss never bothers in writing a correct attribute table for rows on resident OS,
	// it just extends the full attribute RAM with a start: 0 end: 0xff value: 0.
	// According to doc notes anything beyond width 80 is puked by the CRTC, therefore we clamp.
	const u8 attr_max_size = 80;
	const std::array<u8, 41> attr_fifo = m_attr_fifo[!m_input_fifo];

	// expose attribute handling to our client
	// PC-8801 schematics definitely shows extra TTL connections for handling its "8 to 16-bit" attribute conversion.
	// It also practically needs to read the attribute mapping for various extra side-effects such as colorized 400 line 1bpp
	// cfr. "その他 / other" section at http://mydocuments.g2.xrea.com/html/p8/vraminfo.html
	std::array<u16, attr_max_size> extend_attr = m_attr_fetch_cb(attr_fifo, m_gfx_mode, m_y, m_attr << 1, m_h);

	for (int lc = 0; lc < m_r; lc++)
	{
		bool is_lowestline = lc == m_r - 1;
		for (int sx = 0; sx < m_h; sx++)
		{
			int y = m_y + lc;
			uint8_t cc = m_data_fifo[!m_input_fifo][sx];
			int csr = m_cm && m_cursor_blink && ((y / m_r) == m_cy) && (sx == m_cx);

			// datasheet mentions these but I find zero unambiguous information for PC-8001/PC-8801, i.e.:
			// - "highlight" should be attribute blinking?
			// - is "gpa" actually NEC-ese for attr bus?
			// - is sl0 / sl12 NEC names for upper/lower line?
//          int hlgt = 0;
//          int rvv = 0;
//          int vsp = 0;
//          int sl0 = 0;
//          int sl12 = 0;
//          int gpa = 0;

//          m_display_cb(m_bitmap, y, sx, cc, lc, hlgt, rvv, vsp, sl0, sl12, csr, gpa);
			m_display_cb(m_bitmap, y, sx, cc, lc, csr, m_attr_blink, extend_attr[sx], m_gfx_mode, is_lowestline);
		}
	}

	m_y += m_r;
}


//-------------------------------------------------
//  update_screen -
//-------------------------------------------------

uint32_t upd3301_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// TODO: non-transparent modes
	bitmap.fill(0, cliprect);

	if (!get_display_status())
		return 0;

	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}


//-------------------------------------------------
//  set_interrupt -
//-------------------------------------------------

void upd3301_device::set_interrupt(int state)
{
	LOGINT("Interrupt: %u\n", state);

	m_write_int(state);

	if (!state)
	{
		m_status &= ~(STATUS_N | STATUS_E);
	}
}


//-------------------------------------------------
//  set_drq -
//-------------------------------------------------

void upd3301_device::set_drq(int state)
{
	LOGINT("DRQ: %u %d\n", state, screen().vpos());

	m_write_drq(state);
}


//-------------------------------------------------
//  set_display -
//-------------------------------------------------

bool upd3301_device::get_display_status()
{
	return bool(m_status & STATUS_VE);
}


void upd3301_device::set_display(int state)
{
	if (state)
	{
		m_status |= STATUS_VE;
	}
	else
	{
		m_status &= ~STATUS_VE;
	}
}


//-------------------------------------------------
//  reset_counters -
//-------------------------------------------------

void upd3301_device::reset_counters()
{
	set_interrupt(0);
	set_drq(0);
}


//-------------------------------------------------
//  update_hrtc_timer -
//-------------------------------------------------

void upd3301_device::update_hrtc_timer(int state)
{
	int y = screen().vpos();

	int next_x = state ? m_h : 0;
	int next_y = state ? y : ((y + 1) % ((m_l + m_v) * m_width));

	attotime duration = screen().time_until_pos(next_y, next_x);

	m_hrtc_timer->adjust(duration, !state);
}


//-------------------------------------------------
//  update_vrtc_timer -
//-------------------------------------------------

void upd3301_device::update_vrtc_timer(int state)
{
	const bool next_state = !state;
	int next_y = next_state ? (m_l * m_r) : 0;

	attotime duration = screen().time_until_pos(next_y, 0);

	m_vrtc_timer->adjust(duration, next_state);
}


//-------------------------------------------------
//  recompute_parameters -
//-------------------------------------------------

void upd3301_device::recompute_parameters()
{
	int horiz_pix_total = (m_h + m_z) * m_width;
	int vert_pix_total = (m_l + m_v) * m_r;

	attoseconds_t refresh = HZ_TO_ATTOSECONDS(clock()) * horiz_pix_total * vert_pix_total;

	rectangle visarea;

	visarea.set(0, (m_h * m_width) - 1, 0, (m_l * m_r) - 1);

	LOGCRTC("Screen: %u x %u @ %f Hz\n", horiz_pix_total, vert_pix_total, 1 / ATTOSECONDS_TO_DOUBLE(refresh));
	LOGCRTC("Visible Area: (%u, %u) - (%u, %u)\n", visarea.min_x, visarea.min_y, visarea.max_x, visarea.max_y);

	screen().configure(horiz_pix_total, vert_pix_total, visarea, refresh);

	update_hrtc_timer(0);
	update_vrtc_timer(0);
}

// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    hp9845_printer.cpp

    HP9845 internal printer HLE

    The internal printer of HP9845-series machines is a thermal
    printer. It prints by heating the resistors in a fixed line
    of 560 elements. A stepper motor advances the paper (and in some
    cases backs it up).
    A HP nanoprocessor controls the operation of the printer.
    The NP runs firmware from a 2 kB ROM and it has 256 bytes of
    RAM. A second 2 kB ROM acts as character generator.
    Character cells are 7 pixels wide and 12 pixels high.
    Shape of characters in the character generator is "compressed"
    in a peculiar way (see patent) so that the 7x12 cell fits in 8
    bytes. These 8 bytes are addressed according to a Gray code, not
    a binary code, to speed up access.
    Each step of the motor moves the paper by 1/154 of inch. Each
    line in a normal sized character is 2 steps high. Lines of big
    sized characters are 3 steps high. The horizontal size of
    pixels is equal to 2 steps.
    HLE is necessary because it's very difficult to dump ROMs (they're
    non-standard HP parts and they're soldered on the PCB).
    Content of char.gen. was reconstructed by looking at the shape
    of each character on paper.
    Thanks to Ansgar Kueckes for letting me run tests "remotely" on
    his printer.

    Output of printer:
    - bitbanger 1: raw dump of all bytes from PPU to printer
    - bitbanger 2: output of printer pixels. Each line has this form:
    <line_no>:<pixels>, where line_no is the line number (counted in
    motor steps, see above) and <pixels> is the 560-pixel line. Each
    character of <pixels> is either a space (no dot) or '*'. This
    stream can be post-processed to produce a bitmapped graphic.
    Each '*' is to be translated to a 2x2 black square to preserve
    the 1:1 aspect-ratio of pixels.

    What's not implemented:
    - A realistic simulation of printer speed
    - Form Feed
    - Handling of page length & top/bottom margins
    - ESC & l <octal> [ST] commands (set vertical size of lines &
      top margin of paper)

    References:
    - US Patent 4,180,854
    - HP 09845-93000, sep 81, HP9845 BASIC programming (appendix A)

*********************************************************************/

#include "emu.h"
#include "hp9845_printer.h"

// Debugging
#define VERBOSE 0
#include "logmacro.h"

// Bit manipulation
namespace {
	template<typename T> constexpr T BIT_MASK(unsigned n)
	{
		return (T)1U << n;
	}

	template<typename T> void BIT_CLR(T& w , unsigned n)
	{
		w &= ~BIT_MASK<T>(n);
	}

	template<typename T> void BIT_SET(T& w , unsigned n)
	{
		w |= BIT_MASK<T>(n);
	}

	template<typename T> void COPY_BIT(bool bit , T& w , unsigned n)
	{
		if (bit) {
			BIT_SET(w , n);
		} else {
			BIT_CLR(w , n);
		}
	}
}

// Character constants
constexpr uint8_t CH_BS     = 0x08;
constexpr uint8_t CH_TAB    = 0x09;
constexpr uint8_t CH_LF     = 0x0a;
constexpr uint8_t CH_CR     = 0x0d;
constexpr uint8_t CH_SH_OUT = 0x0e;
constexpr uint8_t CH_SH_IN  = 0x0f;
constexpr uint8_t CH_ESC    = 0x1b;

// Bits in m_attrs
constexpr unsigned ATTRS_NEW_BIT    = 7;    // Redefined character
constexpr unsigned ATTRS_TAB_BIT    = 6;    // Tab position
constexpr unsigned ATTRS_U_L_BIT    = 5;    // Underlined
constexpr unsigned ATTRS_BIG_BIT    = 4;    // Big character
constexpr uint8_t ATTRS_REDEF_NO_MASK = 0x0f;   // Redefined character number
constexpr uint8_t ATTRS_NEW_MASK    = BIT_MASK<uint8_t>(ATTRS_NEW_BIT);
constexpr uint8_t ATTRS_BIG_MASK    = BIT_MASK<uint8_t>(ATTRS_BIG_BIT);
constexpr uint8_t ATTRS_U_L_MASK    = BIT_MASK<uint8_t>(ATTRS_U_L_BIT);

// Various timing constants
constexpr unsigned MAX_PIXELS_BURNED    = 62;   // Maximum number of pixels burned in parallel
constexpr unsigned USEC_LT_16_PIXELS    = 4960; // microseconds to burn < 16 pixels
constexpr unsigned USEC_LT_32_PIXELS    = 5984; // microseconds to burn < 32 pixels
constexpr unsigned USEC_LT_48_PIXELS    = 6464; // microseconds to burn < 48 pixels
constexpr unsigned USEC_GE_48_PIXELS    = 6976; // microseconds to burn >= 48 pixels
constexpr unsigned USEC_1_STEP          = 3456; // microseconds for 1 motor step

// Device type definition
DEFINE_DEVICE_TYPE(HP9845_PRINTER, hp9845_printer_device, "hp9845_prt", "HP9845 internal printer")

hp9845_printer_device::hp9845_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
: device_t(mconfig , HP9845_PRINTER , tag , owner , clock),
	m_irl_handler(*this),
	m_flg_handler(*this),
	m_sts_handler(*this),
	m_prt_graph_out(*this , "prt_graphic"),
	m_prt_alpha_out(*this , "prt_alpha"),
	m_prt_chargen(*this , "prt_chargen")
{
}

ROM_START(hp9845_printer)
	ROM_REGION(0x800, "prt_chargen", 0)
	ROM_LOAD("1818-2687.bin", 0, 0x800, CRC(cce64de8) SHA1(1dfabd32f4bdef85f88a514bd1978b80b25a6e80))
ROM_END

const tiny_rom_entry *hp9845_printer_device::device_rom_region() const
{
	return ROM_NAME(hp9845_printer);
}

void hp9845_printer_device::device_add_mconfig(machine_config &config)
{
	BITBANGER(config, m_prt_alpha_out, 0);
	BITBANGER(config, m_prt_graph_out, 0);
}

void hp9845_printer_device::device_start()
{
	m_irl_handler.resolve_safe();
	m_flg_handler.resolve_safe();
	m_sts_handler.resolve_safe();

	save_item(NAME(m_display_mode));
	save_item(NAME(m_shifted));
	save_item(NAME(m_current_u_l));
	save_item(NAME(m_current_big));
	save_item(NAME(m_ibf));
	save_item(NAME(m_inten));
	save_item(NAME(m_busy));
	save_item(NAME(m_ib));
	save_item(NAME(m_pos));
	save_item(NAME(m_line));
	save_item(NAME(m_attrs));
	save_item(NAME(m_redef_count));
	save_item(NAME(m_redef_idx));
	save_item(NAME(m_redef_chars));
	save_item(NAME(m_replace_count));
	save_item(NAME(m_redef_buff));
	save_item(NAME(m_next_replace));
	save_item(NAME(m_rep_str_len));
	save_item(NAME(m_rep_str_ptr));
	save_item(NAME(m_octal_accum));
	save_item(NAME(m_fsm_state));
	save_item(NAME(m_cur_line));

	m_timer = timer_alloc(0);
}

void hp9845_printer_device::device_reset()
{
	state_reset();
	m_cur_line = 0;
	m_inten = false;
	m_ibf = false;
	m_busy = false;
	update_flg();
	m_sts_handler(true);
}

void hp9845_printer_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_busy = false;
	update_fsm();
	update_flg();
}

READ16_MEMBER(hp9845_printer_device::printer_r)
{
	uint16_t res = 0;

	switch (offset) {
	case 0:
	case 1:
		// Bit 7: Interrupt enabled
		// Bit 5: 1
		// Bit 1: Printer powered (1)
		// Bit 0: Interrupt pending
		if (m_inten) {
			BIT_SET(res , 7);
		}
		BIT_SET(res , 5);
		BIT_SET(res , 1);
		if (m_inten && !m_ibf && !m_busy) {
			BIT_SET(res , 0);
		}
		break;

	default:
		break;
	}

	return res;
}

WRITE16_MEMBER(hp9845_printer_device::printer_w)
{
	switch (offset) {
	case 0:
		// New character
		m_ib = (uint8_t)data;
		m_ibf = true;
		update_fsm();
		update_flg();
		break;

	case 1:
		// Control word
		m_inten = BIT(data , 7);
		update_flg();
		break;

	default:
		break;
	}
}

void hp9845_printer_device::state_reset()
{
	m_display_mode = false;
	m_shifted = false;
	m_current_u_l = false;
	m_current_big = false;
	memset(m_attrs , 0 , sizeof(m_attrs));
	memset(m_redef_buff, 0, sizeof(m_redef_buff));
	m_redef_count = 0;
	m_replace_count = 0;
	m_next_replace = REDEF_BUFF_LEN - 1;
	m_rep_str_len = 0;
	m_rep_str_ptr = 0;
	start_new_line();
	m_fsm_state = FSM_NORMAL_TEXT;
}

void hp9845_printer_device::insert_char(uint8_t ch)
{
	if (m_pos == 80) {
		crlf();
	}
	COPY_BIT(m_current_big , m_attrs[ m_pos ] , ATTRS_BIG_BIT);
	COPY_BIT(m_current_u_l , m_attrs[ m_pos ] , ATTRS_U_L_BIT);
	if (ch == '_') {
		BIT_SET(m_attrs[ m_pos ] , ATTRS_U_L_BIT);
	} else {
		m_line[ m_pos ] = ch;
		// Check for redefined characters
		unsigned redef_idx;
		if (is_ch_redef(ch , redef_idx)) {
			m_attrs[ m_pos ] = (m_attrs[ m_pos ] & ~ATTRS_REDEF_NO_MASK) | (uint8_t)redef_idx | ATTRS_NEW_MASK;
		} else {
			BIT_CLR(m_attrs[ m_pos ] , ATTRS_NEW_BIT);
		}
	}
	m_pos++;
}

void hp9845_printer_device::start_new_line()
{
	m_pos = 0;
	memset(m_line, ' ', sizeof(m_line));
	for (auto& x : m_attrs) {
		x &= ~(ATTRS_NEW_MASK | ATTRS_BIG_MASK | ATTRS_U_L_MASK);
	}
}

uint8_t hp9845_printer_device::get_ch_matrix_line(const uint8_t *matrix_base , unsigned line_no , const uint8_t *seq)
{
	uint8_t res = 0;

	switch (line_no) {
	case 0:
		{
			uint8_t b0 = matrix_base[ seq[ 0 ] ];
			if (BIT(b0 , 0)) {
				res = b0 >> 1;
			}
		}
		break;

	case 1:
		{
			uint8_t b3 = matrix_base[ seq[ 3 ] ];
			if (BIT(b3 , 0)) {
				res = matrix_base[ seq[ 0 ] ] >> 1;
			}
		}
		break;

	case 2:
		{
			uint8_t b1 = matrix_base[ seq[ 1 ] ];
			if (!BIT(b1 , 0)) {
				res = b1 >> 1;
			}
		}
		break;

	case 3:
		{
			uint8_t b2 = matrix_base[ seq[ 2 ] ];
			if (!BIT(b2 , 0)) {
				res = b2 >> 1;
			}
		}
		break;

	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		res = matrix_base[ seq[ line_no - 1 ] ] >> 1;
		break;

	case 9:
		{
			uint8_t b1 = matrix_base[ seq[ 1 ] ];
			if (BIT(b1 , 0)) {
				res = b1 >> 1;
			} else {
				uint8_t b5 = matrix_base[ seq[ 5 ] ];
				if (BIT(b5 , 0)) {
					res = matrix_base[ seq[ 0 ] ] >> 1;
				}
			}
		}
		break;

	case 10:
		{
			uint8_t b2 = matrix_base[ seq[ 2 ] ];
			if (BIT(b2 , 0)) {
				res = b2 >> 1;
			} else {
				uint8_t b6 = matrix_base[ seq[ 6 ] ];
				if (BIT(b6 , 0)) {
					res = matrix_base[ seq[ 0 ] ] >> 1;
				}
			}
		}
		break;

	case 11:
		{
			uint8_t b7 = matrix_base[ seq[ 7 ] ];
			if (BIT(b7 , 0)) {
				res = matrix_base[ seq[ 0 ] ] >> 1;
			}
		}
		break;
	}

	return res;
}

unsigned hp9845_printer_device::print_560_pixels(unsigned line_no , const uint8_t *pixels)
{
	std::ostringstream buff;
	buff << line_no;
	buff << ':';
	unsigned pixel_count = 0;
	for (unsigned i = 0; i < 70; i++) {
		uint8_t pop_count = pixels[ i ];
		pop_count = (pop_count & 0x55) + ((pop_count >> 1) & 0x55);
		pop_count = (pop_count & 0x33) + ((pop_count >> 2) & 0x33);
		pop_count = (pop_count & 0x0f) + ((pop_count >> 4) & 0x0f);
		pixel_count += pop_count;
		for (uint8_t mask = 0x80; mask; mask >>= 1) {
			buff << ((mask & pixels[ i ]) ? '*' : ' ');
		}
	}
	if (pixel_count) {
		buff << '\n';
		std::string out{buff.str()};
		for (auto c : out) {
			m_prt_graph_out->output(c);
		}
	}
	LOG("pc=%u\n" , pixel_count);
	return pixel_count;
}

static const uint8_t linear_seq[] = { 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 };
static const uint8_t gray_seq[]   = { 0 , 1 , 3 , 2 , 6 , 7 , 5 , 4 };

uint8_t hp9845_printer_device::get_ch_pixels(uint8_t ch , uint8_t attrs , unsigned matrix_line) const
{
	uint8_t pixels;
	bool new_ch = BIT(attrs , ATTRS_NEW_BIT);
	if (new_ch) {
		const uint8_t *matrix = &m_redef_buff[ (attrs & ATTRS_REDEF_NO_MASK) * 8 ];
		pixels = get_ch_matrix_line(matrix, matrix_line, linear_seq);
	} else {
		const uint8_t *matrix = &m_prt_chargen[ ch * 8 ];
		pixels = get_ch_matrix_line(matrix, matrix_line, gray_seq);
	}
	return pixels;
}

attotime hp9845_printer_device::burn_time(unsigned pixel_count)
{
	if (pixel_count == 0) {
		return attotime::zero;
	}
	unsigned full_burns = pixel_count / MAX_PIXELS_BURNED;
	unsigned rem_pixels = pixel_count - full_burns * MAX_PIXELS_BURNED;
	unsigned usec = full_burns * USEC_GE_48_PIXELS;
	if (rem_pixels < 16) {
		usec += USEC_LT_16_PIXELS;
	} else if (rem_pixels < 32) {
		usec += USEC_LT_32_PIXELS;
	} else if (rem_pixels < 48) {
		usec += USEC_LT_48_PIXELS;
	} else {
		usec += USEC_GE_48_PIXELS;
	}
	return attotime::from_usec(usec);
}

void hp9845_printer_device::print_line()
{
	attotime print_time;
	bool any_big = false;
	// Normal-sized character range is [0..22], every other line
	// Big-sized characters range is [-8..22], one line every 3
	for (int line_off = -8; line_off < 23; line_off++) {
		if ((line_off + int(m_cur_line)) < 0) {
			continue;
		}
		bool norm_line = line_off >= 0 && line_off % 2 == 0;
		bool big_line = (line_off + 8) % 3 == 0;

		if (norm_line || big_line) {
			uint8_t pixel_line[ 70 ];
			memset(pixel_line, 0, sizeof(pixel_line));

			unsigned pixel_line_idx = 0;
			uint8_t pixel_line_mask = 0x80;

			for (unsigned pos = 0; pos < m_pos; pos++) {
				uint8_t ch = m_line[ pos ];
				uint8_t attrs = m_attrs[ pos ];
				uint8_t pixels = 0;

				if (line_off == 22 && BIT(attrs , ATTRS_U_L_BIT)) {
					// Underline
					pixels = 0x7f;
				} else if (BIT(attrs , ATTRS_BIG_BIT) && big_line) {
					any_big = true;
					unsigned matrix_line = (line_off + 8) / 3;
					pixels = get_ch_pixels(ch, attrs, matrix_line);
				} else if (!BIT(attrs , ATTRS_BIG_BIT) && norm_line) {
					unsigned matrix_line = line_off / 2;
					pixels = get_ch_pixels(ch, attrs, matrix_line);
				}
				for (uint8_t mask = 0x40; mask; mask >>= 1) {
					if (pixels & mask) {
						pixel_line[ pixel_line_idx ] |= pixel_line_mask;
					}
					pixel_line_mask >>= 1;
					if (pixel_line_mask == 0) {
						pixel_line_mask = 0x80;
						pixel_line_idx++;
					}
				}
			}
			unsigned pixel_count = print_560_pixels(unsigned(line_off + int(m_cur_line)) , pixel_line);
			print_time += burn_time(pixel_count);
		}
	}
	m_cur_line += 24;
	// Computing an accurate printing time is a lot more complex than this.
	// This should be a reasonable estimate.
	if (any_big) {
		print_time += attotime::from_usec(USEC_1_STEP * 32);
	} else {
		print_time += attotime::from_usec(USEC_1_STEP * 24);
	}
	LOG("A time=%.6f\n" , print_time.as_double());
	m_timer->adjust(print_time);
	m_busy = true;
}

void hp9845_printer_device::print_graphic_line()
{
	unsigned pixel_count = print_560_pixels(m_cur_line , m_line);
	attotime print_time{burn_time(pixel_count)};
	print_time += attotime::from_usec(USEC_1_STEP * 2);
	m_cur_line += 2;
	LOG("G time=%.6f\n" , print_time.as_double());
	m_timer->adjust(print_time);
	m_busy = true;
}

void hp9845_printer_device::crlf()
{
	print_line();
	start_new_line();
}

void hp9845_printer_device::set_tab()
{
	if (m_pos > 0) {
		BIT_SET(m_attrs[ m_pos - 1 ] , ATTRS_TAB_BIT);
	}
}

void hp9845_printer_device::clear_tabs()
{
	for (auto& x : m_attrs) {
		BIT_CLR(x , ATTRS_TAB_BIT);
	}
}

void hp9845_printer_device::move_to_next_tab()
{
	for (unsigned i = m_pos; i < 80; i++) {
		if (BIT(m_attrs[ i ] , ATTRS_TAB_BIT)) {
			m_pos = i + 1;
			return;
		}
	}
	crlf();
}

void hp9845_printer_device::update_flg()
{
	m_flg_handler(!m_ibf && !m_busy);
	m_irl_handler(!m_ibf && !m_busy && m_inten);
}

bool hp9845_printer_device::is_ch_redef(uint8_t ch , unsigned& redef_number) const
{
	for (unsigned i = 0; i < m_redef_count; i++) {
		if (m_redef_chars[ i ] == ch) {
			redef_number = i;
			return true;
		}
	}
	return false;
}

uint8_t hp9845_printer_device::allocate_ch_redef(uint8_t& idx)
{
	if (m_redef_count >= REDEF_CH_COUNT) {
		idx = REDEF_CH_COUNT - 1;
		return true;
	} else if (m_next_replace < 8) {
		return false;
	} else {
		uint8_t max_idx = (m_next_replace - 8) / 8;
		if (max_idx < m_redef_count) {
			idx = max_idx;
		} else {
			idx = m_redef_count++;
		}
		return true;
	}
}

bool hp9845_printer_device::is_ch_replaced(uint8_t ch , uint8_t& len , uint8_t& ptr) const
{
	uint8_t idx = REDEF_BUFF_LEN - 1;
	for (unsigned i = 0; i < m_replace_count; i++) {
		if (m_redef_buff[ idx-- ] == ch) {
			len = m_redef_buff[ idx-- ];
			ptr = idx;
			return true;
		} else {
			idx -= (m_redef_buff[ idx ] + 1);
		}
	}
	return false;
}

uint8_t hp9845_printer_device::free_redef_space() const
{
	return m_next_replace - m_redef_count * 8;
}

uint8_t hp9845_printer_device::apply_shifting(uint8_t ch) const
{
	if ((ch & 0x60) == 0 || !m_shifted) {
		return ch;
	} else {
		return ch | 0x80;
	}
}

bool hp9845_printer_device::parse_octal(uint8_t ch)
{
	if (ch >= '0' && ch <= '7') {
		m_octal_accum = (m_octal_accum * 8) + (ch - '0');
		return true;
	} else {
		return false;
	}
}

bool hp9845_printer_device::parse_ch(uint8_t ch)
{
	bool consume = true;

	switch (m_fsm_state) {
	case FSM_NORMAL_TEXT:
		if (m_display_mode) {
			insert_char(apply_shifting(ch));
			if (ch == CH_CR) {
				crlf();
			} else if (ch == CH_ESC) {
				m_fsm_state = FSM_WAIT_ESC_Z;
			}
		} else {
			switch (ch) {
			case CH_BS:
				if (m_pos > 0) {
					m_pos--;
				}
				break;

			case CH_TAB:
				move_to_next_tab();
				break;

			case CH_LF:
				{
					auto save_pos = m_pos;
					crlf();
					m_pos = save_pos;
				}
				break;

			case CH_CR:
				m_fsm_state = FSM_AFTER_CR;
				break;

			case CH_SH_OUT:
				m_shifted = true;
				break;

			case CH_SH_IN:
				m_shifted = false;
				break;

			case CH_ESC:
				m_fsm_state = FSM_AFTER_ESC;
				break;

			default:
				if ((ch & 0xe4) == 0x80) {
					m_current_u_l = false;
				} else if ((ch & 0xe4) == 0x84) {
					m_current_u_l = true;
				} else if ((ch & 0x60) != 0) {
					insert_char(apply_shifting(ch));
				}
				break;
			}
		}
		break;

	case FSM_AFTER_CR:
		if (ch == CH_LF) {
			// CR + LF sequence
			crlf();
		} else {
			// CR + something not LF
			auto save_cur_line = m_cur_line;
			crlf();
			m_cur_line = save_cur_line;
			// Ch is pushed back for next iteration of FSM
			consume = false;
		}
		m_fsm_state = FSM_NORMAL_TEXT;
		break;

	case FSM_AFTER_ESC:
		m_fsm_state = FSM_NORMAL_TEXT;
		switch (ch) {
		case '1':
			set_tab();
			break;

		case '3':
			clear_tabs();
			break;

		case 'E':
			LOG("State reset\n");
			state_reset();
			break;

		case 'Y':
			m_display_mode = true;
			break;

		case '&':
			m_fsm_state = FSM_AFTER_ESC_AMP;
			break;

		case '?':
			if (m_pos > 0) {
				crlf();
			}
			m_pos = 0;
			m_fsm_state = FSM_COLLECT_ESC_QMARK;
			break;

		default:
			// Unknown ESC sequence
			break;
		}
		break;

	case FSM_AFTER_ESC_AMP:
		switch (ch) {
		case 'd':
			m_fsm_state = FSM_AFTER_ESC_AMP_D;
			break;

		case 'k':
			m_fsm_state = FSM_AFTER_ESC_AMP_K;
			break;

		case 'l':
			m_fsm_state = FSM_AFTER_ESC_AMP_L;
			m_octal_accum = 0;
			break;

		case 'n':
			m_fsm_state = FSM_AFTER_ESC_AMP_N;
			m_octal_accum = 0;
			break;

		case 'o':
			m_fsm_state = FSM_AFTER_ESC_AMP_O;
			m_octal_accum = 0;
			break;

		default:
			// Unknown ESC & sequence
			m_fsm_state = FSM_NORMAL_TEXT;
			break;
		}
		break;

	case FSM_COLLECT_ESC_QMARK:
		m_line[ m_pos++ ] = ch;
		if (m_pos == 70) {
			print_graphic_line();
			start_new_line();
			m_fsm_state = FSM_NORMAL_TEXT;
		}
		break;

	case FSM_AFTER_ESC_AMP_K:
		switch (ch) {
		case '0':
		case '1':
			m_octal_accum = ch != '0';
			m_fsm_state = FSM_AFTER_ESC_AMP_K_01;
			break;

		default:
			m_fsm_state = FSM_NORMAL_TEXT;
			break;
		}
		break;

	case FSM_AFTER_ESC_AMP_K_01:
		if (ch == 'S') {
			m_current_big = m_octal_accum;
		}
		m_fsm_state = FSM_NORMAL_TEXT;
		break;

	case FSM_AFTER_ESC_AMP_D:
		switch (ch) {
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
			m_current_u_l = true;
			break;

		case '@':
		case 'A':
		case 'B':
		case 'C':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
			m_current_u_l = false;
			break;

		default:
			break;
		}
		m_fsm_state = FSM_NORMAL_TEXT;
		break;

	case FSM_AFTER_ESC_AMP_N:
		if (!parse_octal(ch)) {
			if (ch == 'c') {
				unsigned dummy;
				if (is_ch_redef(m_octal_accum, dummy) || !allocate_ch_redef(m_redef_idx)) {
					// Character redefined twice or space exhausted
					m_redef_idx = 0xff;
				}
				LOG("Redef ch %02x @%u\n" , m_octal_accum , m_redef_idx);
				if (m_redef_idx < REDEF_CH_COUNT) {
					m_redef_chars[ m_redef_idx ] = m_octal_accum;
				}
				m_octal_accum = 0;
				m_fsm_state = FSM_AFTER_ESC_AMP_N_C;
			} else {
				m_fsm_state = FSM_NORMAL_TEXT;
			}
		}
		break;

	case FSM_AFTER_ESC_AMP_O:
		if (!parse_octal(ch)) {
			if (ch == 'c') {
				uint8_t dummy1;
				uint8_t dummy2;
				if (is_ch_replaced(m_octal_accum, dummy1, dummy2) || free_redef_space() < 2) {
					// Character replaced twice or space exhausted
					m_redef_idx = 0xff;
				} else {
					m_redef_idx = m_next_replace;
					m_redef_buff[ m_redef_idx-- ] = m_octal_accum;
					m_redef_buff[ m_redef_idx ] = 0;
					m_next_replace -= 2;
					m_replace_count++;
				}
				m_octal_accum = 0;
				m_fsm_state = FSM_AFTER_ESC_AMP_O_C;
			} else {
				m_fsm_state = FSM_NORMAL_TEXT;
			}
		}
		break;

	case FSM_AFTER_ESC_AMP_O_C:
		if (!parse_octal(ch)) {
			if (ch == 'L') {
				if (m_redef_idx < REDEF_BUFF_LEN) {
					uint8_t max_len = std::min(m_octal_accum , free_redef_space());
					m_next_replace -= max_len;
					m_redef_buff[ m_redef_idx-- ] = max_len;
					LOG("Repl ch %02x with str @%u, len=%u\n" , m_redef_buff[ m_redef_idx + 2 ] , m_redef_idx , max_len);
				}
				if (m_octal_accum) {
					m_fsm_state = FSM_AFTER_ESC_AMP_O_C_L;
				} else {
					m_fsm_state = FSM_NORMAL_TEXT;
				}
			} else {
				m_fsm_state = FSM_NORMAL_TEXT;
			}
		}
		break;

	case FSM_AFTER_ESC_AMP_O_C_L:
		if (m_redef_idx < REDEF_BUFF_LEN && m_redef_idx >= m_next_replace) {
			m_redef_buff[ m_redef_idx-- ] = ch;
		}
		if (--m_octal_accum == 0) {
			m_fsm_state = FSM_NORMAL_TEXT;
		}
		break;

	case FSM_AFTER_ESC_AMP_L:
		if (!parse_octal(ch)) {
			// TODO: ESC & l <octal> [ST]
			m_fsm_state = FSM_NORMAL_TEXT;
		}
		break;

	case FSM_AFTER_ESC_AMP_N_C:
		if (!parse_octal(ch)) {
			if (ch >= 'p' && ch <= 'w') {
				if (m_redef_idx < REDEF_CH_COUNT) {
					m_redef_buff[ m_redef_idx * 8 + ch - 'p' ] = m_octal_accum;
				}
				m_octal_accum = 0;
			} else if (ch >= 'P' && ch <= 'W') {
				if (m_redef_idx < REDEF_CH_COUNT) {
					m_redef_buff[ m_redef_idx * 8 + ch - 'P' ] = m_octal_accum;
				}
				m_fsm_state = FSM_NORMAL_TEXT;
			} else {
				m_fsm_state = FSM_NORMAL_TEXT;
			}
		}
		break;

	case FSM_WAIT_ESC_Z:
		insert_char(ch);
		if (ch == 'Z') {
			m_display_mode = false;
			m_fsm_state = FSM_NORMAL_TEXT;
		} else if (ch != CH_ESC) {
			m_fsm_state = FSM_NORMAL_TEXT;
		}
		break;
	}
	return consume;
}

void hp9845_printer_device::update_fsm()
{
	while (!m_busy && m_ibf) {
		bool consume = false;
		if (m_rep_str_len != 0) {
			consume = parse_ch(m_redef_buff[ m_rep_str_ptr ]);
		} else if ((m_fsm_state == FSM_NORMAL_TEXT || m_fsm_state == FSM_AFTER_CR || m_fsm_state == FSM_COLLECT_ESC_QMARK) &&
				   is_ch_replaced(apply_shifting(m_ib), m_rep_str_len, m_rep_str_ptr)) {
			LOG("Subst ch %02x with str @%u (len = %u)\n" , m_ib , m_rep_str_ptr , m_rep_str_len);
			if (m_rep_str_len == 0) {
				consume = true;
			}
		} else {
			consume = parse_ch(m_ib);
		}
		if (consume) {
			if (m_rep_str_len) {
				m_rep_str_len--;
				m_rep_str_ptr--;
			}
			if (m_rep_str_len == 0) {
				m_prt_alpha_out->output(m_ib);
				m_ibf = false;
				update_flg();
			}
		}
	}
}

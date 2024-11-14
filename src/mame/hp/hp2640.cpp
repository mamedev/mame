// license:BSD-3-Clause
// copyright-holders:F. Ulivi, Gavin Scott
//
// **********************************
// Driver for HP2640-series terminals
// **********************************
//
// HP264x terminals are built around a few common elements:
// - Chassis with a passive card cage and power supply. Cards share a common bus
//   with a daisy-chained priority order (i.e. cards closer to power supply have
//   higher priority in accessing the bus). The bus also distributes a 4.9152 MHz
//   system clock.
// - White phosphor CRT (with its unusual 2:1 ratio) and the related drive board
// - Keyboard
// The "personality" in each version is in the set of cards installed in
// the card cage (and in a few minor differences of the common elements).
//
// *********************************************
// Driver for HP2641A APL terminal (Gavin Scott)
// *********************************************
//
// This driver implements the HP2641A APL Terminal which
// was used with APL\3000 on the HP 3000 Series II/III computers from 1976
// into the 1980s. I recently got APL\3000 running under simulation so this
// is an attempt to recreate the whole experience with the APL characters
// on the terminal device designed for it. The HP2641A is basically a 2645
// with a few altered ROMs (many are identical to the 2645) and one
// additional 2K ROM for which a second CTL PCA was required that was not
// needed in the 2645.
//
// This driver emulates the HP2641A model, as composed of the following cards:
// - 02640-60123    Keyboard interface
// - 02640-60112    Display control
// - 02640-60088    Display timing
// - 02640-60124    Display extended DMA
// - 02640-60024    Display enhancement
// - 02640-60209    CPU
// - 02640-60192    Control storage (firmware ROMs & 256-byte SRAM)
// - 02640-60192    Second CTL PCA with one additional 2K ROM and 256b SRAM.
// - 02640-60065    4k DRAM (4 of these for a 16k total)
// - 02640-60086    Asynchronous data comm
// - 02640-60137    CTU interface
// - 02640-60032    Read/write PCA
//
// The following table summarizes the emulated character sets. The 2641A only
// had room for a single optional character set (line drawing here) after the
// required 192 APL characters were accounted for.
//
// | Char. set | Description    | No. of chars |      ROMs | Type              |
// |-----------+----------------+--------------+-----------+-------------------|
// |         0 | Std ASCII set  |          128 | 1816-0612 | Alphanumeric      |
// |           |                |              | 1816-0613 | Alphanumeric      |
// |         1 | APL Symbols    |          128 | 1816-0984 | Alphanumeric      |
// |           | APL UC and sym |              | 1816-0985 | Alphanumeric      |
// |         2 | APL Overstruck |           64 | 1816-0986 | Alphanumeric      |
// |         3 | Line drawing   |           64 | 1816-1417 | 8-bit microvector |
//
// *************************************
// Driver for HP2645A terminal (F.Ulivi)
// *************************************
//
// This driver emulates the HP2645A model, as composed of the following cards:
// - 02640-60123    Keyboard interface
// - 02640-60112    Display control
// - 02640-60088    Display timing
// - 02640-60124    Display extended DMA
// - 02640-60024    Display enhancement
// - 02640-60209    CPU
// - 02640-60192    Control storage (firmware ROMs & 256-byte SRAM)
// - 02640-60065    4k DRAM (4 of these for a 16k total)
// - 02640-60086    Asynchronous data comm
// - 02640-60137    CTU interface
// - 02640-60032    Read/write PCA
//
// The following table summarizes the emulated character sets.
//
// | Char. set | Description    | No. of chars |      ROMs | Type              |
// |-----------+----------------+--------------+-----------+-------------------|
// |         0 | Std ASCII set  |          128 | 1816-0612 | Alphanumeric      |
// |           |                |              | 1816-0613 |                   |
// |         1 | Math symbols   |           64 | 1816-0642 | Alphanumeric      |
// |         2 | Line drawing   |           64 | 1816-1417 | 8-bit microvector |
// |         3 | Big characters |           64 | 1816-1425 | 8-bit microvector |
//
// All characters are rendered in a 9x15 cell.
// Fonts are rendered in different ways according to type: alphanumeric fonts
// take advantage of half-pixel shifting for aesthetic improvement, microvector fonts
// are rendered by mapping 8 bits to 9 pixels (bit 0 is displayed twice).
// Earlier versions of display hw used weird 9-bit PROMs for microvector fonts.
// Then, luckily, HP wised up and reverted to 8-bit memories.
//
// All non-volatile settings of this terminal are "stored" in mechanical switches
// and not in non-volatile memories (they were probably too expensive/limited/unreliable
// back then).
// There are 3 switches in a corner of the keyboard (data comm switches) and 24 option
// switches (named A to Z) on the keyboard interface PCB.
//
// Notes:
// - Speed of the CPU is not accurately emulated. The effects of bus sharing by
//   video DMA and CPU are not taken into account.
// - LEDs are not output.
// - RESET key is not implemented.
// - A few TODOs here & there.

#include "emu.h"
#include "screen.h"
#include "cpu/i8085/i8085.h"
#include "machine/timer.h"
#include "bus/rs232/rs232.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "hp2640_tape.h"
#include "sound/beep.h"
#include "emupal.h"
#include "speaker.h"

#include "hp2640.lh"

// Debugging
#define VERBOSE 0
#include "logmacro.h"

namespace {

// Bit manipulation
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

// **** Constants ****
constexpr auto SYS_CLOCK = XTAL(4'915'200);
constexpr auto VIDEO_DOT_CLOCK   = XTAL(21'060'000);
constexpr unsigned VIDEO_VIS_ROWS    = 24;
constexpr unsigned VIDEO_TOT_ROWS    = 25;
constexpr unsigned VIDEO_VIS_COLS    = 80;
constexpr unsigned VIDEO_TOT_COLS    = 104;
constexpr unsigned VIDEO_CHAR_WIDTH  = 9;
constexpr unsigned VIDEO_CHAR_HEIGHT = 15;
constexpr uint16_t START_DMA_ADDR   = 0xffff;
constexpr unsigned MAX_DMA_CYCLES   = 450;
constexpr unsigned CURSOR_BLINK_INH_MS  = 110;
constexpr unsigned BEEP_FREQUENCY   = 650;
constexpr unsigned BEEP_DURATION_MS = 100;

// ************
// hp2640_base
// ************
class hp2640_base_state : public driver_device
{
protected:
	hp2640_base_state(const machine_config &mconfig, device_type type, const char *tag , uint8_t m_cg_0 , uint8_t m_cg_1 , uint8_t m_cg_2 , uint8_t m_cg_3);

	void hp2640_base(machine_config &config);

	// Character generator settings
	enum : uint8_t {
		// OR-ENABLE (0) or AND-ENABLE (1)
		// *Ignored*
		CHARGEN_A = 0x01,
		// Alphanumeric (0) or Microvector (1)
		CHARGEN_B = 0x02,
		// 128 characters (0) or 64 characters (1)
		CHARGEN_C = 0x04,
		// D0 copy disabled (0) or enabled (1)
		// *Ignored as D0 copy is always enabled*
		CHARGEN_D = 0x08,
		// D0 copy on UC/LC (0) or LC only (1)
		// *Ignored, see CHARGEN_D*
		CHARGEN_E = 0x10
	};

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	IRQ_CALLBACK_MEMBER(irq_callback);

	void mode_byte_w(uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_10ms_exp);

	uint8_t kb_r(offs_t offset);
	void kb_prev_w(uint8_t data);
	void kb_reset_w(uint8_t data);
	uint8_t switches_ah_r();
	uint8_t switches_jr_r();
	uint8_t switches_sz_r();
	uint8_t datacomm_sw_r();
	void kb_led_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer);
	void cx_w(uint8_t data);
	void cy_w(uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_cursor_blink_inh);

	uint8_t async_status_r();
	void async_control_w(uint8_t data);
	void async_dav_w(int state);
	void async_txd_w(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(timer_beep_exp);

	void tape_irq_w(int state);

	uint8_t poll_r();

	void cpu_mem_map(address_map &map) ATTR_COLD;
	void cpu_io_map(address_map &map) ATTR_COLD;

	required_device<i8080a_cpu_device> m_cpu;
	required_device<timer_device> m_timer_10ms;
	required_ioport_array<4> m_io_key;
	required_ioport m_io_comm;
	required_ioport m_io_sw_ah;
	required_ioport m_io_sw_jr;
	required_ioport m_io_sw_sz;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<timer_device> m_timer_cursor_blink_inh;
	required_device<rs232_port_device> m_rs232;
	required_device<ay51013_device> m_uart;
	required_device<clock_device> m_uart_clock;
	required_device<beep_device> m_beep;
	required_device<timer_device> m_timer_beep;
	required_device<hp2640_tape_device> m_tapes;
	memory_view m_io_view;

	uint8_t m_mode_byte;
	bool m_timer_irq;
	bool m_datacom_irq;
	bool m_tape_irq;

	// Character generators
	required_region_ptr_array<uint8_t , 4> m_chargen;

	const uint8_t m_chargen_set[ 4 ];

	// Video DMA
	struct line_buffer {
		uint8_t m_chars[ VIDEO_VIS_COLS ];
		uint8_t m_attrs[ VIDEO_VIS_COLS ];
	};
	struct line_buffer m_buffers[ 2 ];
	bool m_even;
	bool m_dma_on;  // U310-9
	bool m_line_done;
	uint16_t m_dma_addr;
	uint8_t m_row_counter;
	bool m_row_clock;   // U21-5
	bool m_row_reset;   // U11-1
	bool m_eop; // U311-9
	bool m_eol; // U211-9
	bool m_skipeol; // U311-5
	bool m_en_skipeol;  // U211-5

	// Video
	bitmap_rgb32 m_bitmap;
	uint8_t m_cursor_x;
	uint8_t m_cursor_y;
	bool m_cursor_blink_inh;
	bool m_blanking;

	// Async line interface
	uint8_t m_async_control;

	void update_irq();
	uint8_t video_dma_get();
	void video_load_buffer(bool buff_idx , unsigned& idx , uint8_t ch , bool iv , uint8_t attrs);
	void video_fill_buffer(bool buff_idx , unsigned max_cycles);
	void video_render_buffer(unsigned video_scanline , unsigned line_in_row , bool buff_idx , bool cyen);
	void update_async_control(uint8_t new_control);
	void update_async_irq();
};

hp2640_base_state::hp2640_base_state(const machine_config &mconfig, device_type type, const char *tag , uint8_t m_cg_0 , uint8_t m_cg_1 , uint8_t m_cg_2 , uint8_t m_cg_3)
	: driver_device(mconfig , type , tag),
	  m_cpu(*this , "cpu"),
	  m_timer_10ms(*this , "timer_10ms"),
	  m_io_key(*this , "KEY%u" , 0),
	  m_io_comm(*this , "comm"),
	  m_io_sw_ah(*this , "sw_ah"),
	  m_io_sw_jr(*this , "sw_jr"),
	  m_io_sw_sz(*this , "sw_sz"),
	  m_screen(*this , "screen"),
	  m_palette(*this , "palette"),
	  m_timer_cursor_blink_inh(*this , "timer_cursor_blink_inh"),
	  m_rs232(*this , "rs232"),
	  m_uart(*this , "uart"),
	  m_uart_clock(*this , "uart_clock"),
	  m_beep(*this , "beep"),
	  m_timer_beep(*this , "timer_beep"),
	  m_tapes(*this , "tapes"),
	  m_io_view(*this , "io_view"),
	  m_chargen(*this , "chargen%u" , 0),
	  m_chargen_set{ m_cg_0 , m_cg_1 , m_cg_2 , m_cg_3}
{
}

void hp2640_base_state::machine_start()
{
	m_screen->register_screen_bitmap(m_bitmap);

	// these are set in machine_reset, but device reset callbacks end up reading them before then
	// (machine_reset is called after all child devices are reset)
	m_mode_byte = false;
	m_timer_irq = m_datacom_irq = m_tape_irq = false;
	m_async_control = 0;

	m_even = true;
	m_line_done = false;
	m_dma_addr = 0;
	m_row_counter = 0;
	m_row_reset = false;

	// TODO: save more state
	save_item(NAME(m_mode_byte));
	save_item(NAME(m_timer_irq));
	save_item(NAME(m_datacom_irq));
	save_item(NAME(m_tape_irq));
}

void hp2640_base_state::machine_reset()
{
	m_mode_byte = 0;
	m_timer_irq = false;
	m_datacom_irq = false;
	m_tape_irq = false;
	m_timer_10ms->reset();
	update_irq();
	m_blanking = true;
	m_dma_on = true;
	m_eop = true;
	m_uart->write_xr(1);
	m_uart->write_swe(0);
	m_uart->write_cs(1);
	m_uart->write_nb2(1);
	m_async_control = 0;
	m_uart_clock->set_unscaled_clock(0);
	update_async_control(0x00);
	m_rs232->write_dtr(0);
	async_txd_w(0);
	m_beep->set_state(0);
}

IRQ_CALLBACK_MEMBER(hp2640_base_state::irq_callback)
{
	uint8_t res;

	// Encode interrupts in restart instruction (in order of decreasing priority)
	if (m_tape_irq) {
		// RST 5
		res = 0xef;
	} else if (m_datacom_irq && !BIT(m_mode_byte , 4)) {
		// RST 4
		res = 0xe7;
	} else if (m_timer_irq && !BIT(m_mode_byte , 5)) {
		// RST 3
		res = 0xdf;
	} else {
		// RST 0: should never happen (TM)
		res = 0xc7;
	}
	LOG("IRQ ACK %02x\n" , res);
	return res;
}

void hp2640_base_state::mode_byte_w(uint8_t data)
{
	if (BIT(m_mode_byte , 0) && !BIT(data , 0)) {
		m_timer_10ms->reset();
	} else if (!BIT(m_mode_byte , 0) && BIT(data , 0)) {
		m_timer_10ms->adjust(attotime::from_msec(10) , 0 , attotime::from_msec(10));
	}
	m_mode_byte = data;

	if (!BIT(m_mode_byte , 1)) {
		m_timer_irq = false;
	}
	update_irq();

	m_io_view.select(BIT(m_mode_byte , 6));
}

TIMER_DEVICE_CALLBACK_MEMBER(hp2640_base_state::timer_10ms_exp)
{
	if (BIT(m_mode_byte , 1)) {
		m_timer_irq = true;
		update_irq();
	}
}

uint8_t hp2640_base_state::kb_r(offs_t offset)
{
	ioport_value k = m_io_key[ offset / 4 ]->read();

	return uint8_t(k >> (8 * (offset % 4)));
}

void hp2640_base_state::kb_prev_w(uint8_t data)
{
	// This port is used to set the threshold in key sense circuit for hysteresis.
	// We can safely ignore all writes.
}

void hp2640_base_state::kb_reset_w(uint8_t data)
{
	// TODO: enabled/disable CPU reset
}

uint8_t hp2640_base_state::switches_ah_r()
{
	uint8_t res = m_io_sw_ah->read();
	LOG("SW AH=%02x\n" , res);
	return res;
}

uint8_t hp2640_base_state::switches_jr_r()
{
	uint8_t res = m_io_sw_jr->read();
	LOG("SW JR=%02x\n" , res);
	return res;
}

uint8_t hp2640_base_state::switches_sz_r()
{
	uint8_t res = m_io_sw_sz->read();
	LOG("SW SZ=%02x\n" , res);
	return res;
}

uint8_t hp2640_base_state::datacomm_sw_r()
{
	uint8_t res = m_io_comm->read();
	LOG("COM SW=%02x\n" , res);
	return res;
}

void hp2640_base_state::kb_led_w(uint8_t data)
{
	if (BIT(data , 7)) {
		m_beep->set_state(1);
		m_timer_beep->adjust(attotime::from_msec(BEEP_DURATION_MS));
	}
	// TODO:
	LOG("LED = %02x\n" , data);
}

uint32_t hp2640_base_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(hp2640_base_state::scanline_timer)
{
	unsigned video_scanline = param;
	if (video_scanline < VIDEO_VIS_ROWS * VIDEO_CHAR_HEIGHT) {
		unsigned row = video_scanline / VIDEO_CHAR_HEIGHT;
		unsigned line_in_row = video_scanline - row * VIDEO_CHAR_HEIGHT;

		if (line_in_row == 0) {
			if (row == VIDEO_VIS_ROWS - 1) {
				// Start of V blank (GVS)
				m_even = true;
				m_row_reset = true;
				m_row_counter = 0;
				m_dma_addr = START_DMA_ADDR;
				m_eop = m_blanking;
				m_skipeol = false;
			} else {
				m_even = !m_even;
			}
			m_line_done = false;
			video_fill_buffer(m_even , MAX_DMA_CYCLES);
		};
		video_render_buffer(video_scanline , line_in_row , !m_even , row == m_cursor_y);
	}
}

void hp2640_base_state::cx_w(uint8_t data)
{
	m_cursor_x = data & 0x7f;
	m_cursor_blink_inh = true;
	m_timer_cursor_blink_inh->adjust(attotime::from_msec(CURSOR_BLINK_INH_MS));
}

void hp2640_base_state::cy_w(uint8_t data)
{
	m_blanking = BIT(data , 7);
	m_cursor_y = data & 0x1f;
	m_dma_on = m_blanking || !BIT(data , 6);
	if (m_cursor_y == m_row_counter) {
		LOG("ROW MATCH %02x\n" , data);
		if (!BIT(data , 6) && BIT(data , 5)) {
			LOG("EOP on cy\n");
			m_eop = true;
		}
		if (BIT(data , 6) && !BIT(data , 5) && m_en_skipeol) {
			LOG("SKIPEOL on cy\n");
			m_skipeol = true;
		}
	}
	if (m_dma_on) {
		// Fill buffer if DMA is enabled for the 1st time on the current line
		// TODO: compute correct max. no. of cycles
		video_fill_buffer(m_even , MAX_DMA_CYCLES);
	}
	m_cursor_blink_inh = true;
	m_timer_cursor_blink_inh->adjust(attotime::from_msec(CURSOR_BLINK_INH_MS));
}

TIMER_DEVICE_CALLBACK_MEMBER(hp2640_base_state::timer_cursor_blink_inh)
{
	m_cursor_blink_inh = false;
}

uint8_t hp2640_base_state::async_status_r()
{
	uint8_t res = 0;

	if (m_uart->dav_r()) {
		BIT_SET(res, 0);
	}
	if (m_uart->tbmt_r()) {
		BIT_SET(res, 1);
	}
	if (m_uart->or_r()) {
		BIT_SET(res, 2);
	}
	if (m_uart->pe_r()) {
		BIT_SET(res, 3);
	}
	if (m_rs232->dcd_r()) {
		BIT_SET(res, 4);
	}
	if (m_rs232->cts_r()) {
		BIT_SET(res, 5);
	}
	// Secondary ch. receive always 1
	BIT_SET(res, 6);
	LOG("ASYNC ST=%02x\n" , res);
	return res;
}

void hp2640_base_state::async_control_w(uint8_t data)
{
	update_async_control(data);
}

void hp2640_base_state::async_dav_w(int state)
{
	update_async_irq();
}

void hp2640_base_state::async_txd_w(int state)
{
	m_rs232->write_txd(!BIT(m_async_control , 6) && m_uart->so_r());
}

TIMER_DEVICE_CALLBACK_MEMBER(hp2640_base_state::timer_beep_exp)
{
	m_beep->set_state(0);
}

void hp2640_base_state::tape_irq_w(int state)
{
	m_tape_irq = state;
	update_irq();
}

uint8_t hp2640_base_state::poll_r()
{
	uint8_t res = m_tapes->poll_r();
	LOG("POLL %02x\n" , res);
	return res;
}

void hp2640_base_state::update_irq()
{
	bool state = m_tape_irq ||
		(m_datacom_irq && !BIT(m_mode_byte , 4)) ||
		(m_timer_irq && !BIT(m_mode_byte , 5));
	m_cpu->set_input_line(I8085_INTR_LINE , state);
}

uint8_t hp2640_base_state::video_dma_get()
{
	uint8_t b = m_cpu->space(AS_PROGRAM).read_byte(m_dma_addr | 0xc000);
	m_dma_addr--;
	return b;
}

void hp2640_base_state::video_load_buffer(bool buff_idx , unsigned& idx , uint8_t ch , bool iv , uint8_t attrs)
{
	COPY_BIT(iv, ch, 7);
	m_buffers[ buff_idx ].m_chars[ idx ] = ch;
	// Real hw only stores bits 5,4,3,2,0
	m_buffers[ buff_idx ].m_attrs[ idx ] = attrs;
	idx++;
	m_en_skipeol = false;
	m_row_reset = false;
}

void hp2640_base_state::video_fill_buffer(bool buff_idx , unsigned max_cycles)
{
	if (m_line_done || !m_dma_on) {
		return;
	}
	m_line_done = true;
	m_row_clock = true;
	m_eol = false;
	bool curr_iv = false;   // U310-5
	uint8_t curr_attrs = 0;
	bool link_lsb = false;
	uint8_t dma_byte = 0;

	for (unsigned i = 0 , mem_cycles = 0; i < VIDEO_VIS_COLS && mem_cycles < max_cycles; mem_cycles++) {
		if (link_lsb) {
			uint8_t lsb = video_dma_get();
			if ((lsb & 0xf) != 0xf) {
				// This link is a pointer to a new row
				if (m_row_clock) {
					m_row_clock = false;
					if (!m_row_reset) {
						m_row_counter++;
					}
				}
				m_skipeol = false;
			}
			m_dma_addr = (uint16_t(dma_byte) << 8) | lsb;
			link_lsb = false;
		} else if (m_eop || m_eol) {
			curr_iv = false;
			curr_attrs = 0;
			// Load spaces
			video_load_buffer(buff_idx, i, 0x20, false, 0);
		} else {
			dma_byte = video_dma_get();
			// Decode byte type
			if (!BIT(dma_byte , 7)) {
				// 0xxx-xxxx
				// DATA byte (ordinary character)
				if (!m_skipeol) {
					video_load_buffer(buff_idx, i, dma_byte, curr_iv, curr_attrs);
				}
			} else if (!BIT(dma_byte , 6)) {
				// 10xx-xxxx
				// CONTROL byte
				// Bit(s)   Content
				// ================
				// 5..4     Character set selection
				// 3        Half-brightness
				// 2        Underline
				// 1        Inverse video
				// 0        Blink
				curr_iv = BIT(dma_byte , 1);
				curr_attrs = dma_byte;
			} else if (BIT(dma_byte , 5) || BIT(dma_byte , 4)) {
				// 111x-xxxx or 1101-xxxx
				// Link MSB
				link_lsb = true;
			} else if (!BIT(dma_byte , 3) || !BIT(dma_byte , 2)) {
				// 1100-0xxx or 1100-10xx
				// Flags: ignored
			} else if (!BIT(dma_byte , 1)) {
				// 1100-110x
				// EOL
				if (!m_skipeol) {
					m_eol = true;
					m_en_skipeol = true;
				}
				m_skipeol = false;
			} else if (!BIT(dma_byte , 0)) {
				// 1100-1110
				// EOP
				m_eop = true;
			} else {
				// 1100-1111
				// "O" data byte
				if (!m_skipeol) {
					video_load_buffer(buff_idx, i, dma_byte, curr_iv, curr_attrs);
				}
			}
		}
	}
}

void hp2640_base_state::video_render_buffer(unsigned video_scanline , unsigned line_in_row , bool buff_idx , bool cyen)
{
	if (m_blanking) {
		m_bitmap.fill(rgb_t::black() , rectangle(0 , VIDEO_VIS_COLS * VIDEO_CHAR_WIDTH * 2 , video_scanline , video_scanline));
		return;
	}

	bool cursor_blink;
	unsigned fn = (unsigned)m_screen->frame_number();
	unsigned fn_12 = fn / 12;
	if (m_cursor_blink_inh) {
		cursor_blink = true;
		if (BIT(fn , 0)) {
			cyen = false;
		}
	} else {
		cursor_blink = BIT(fn_12 , 0);
	}

	for (unsigned i = 0 , x_left = 0; i < VIDEO_VIS_COLS; i++ , x_left += VIDEO_CHAR_WIDTH * 2) {
		uint8_t ch = m_buffers[ buff_idx ].m_chars[ i ];
		bool iv = BIT(ch , 7);
		BIT_CLR(ch, 7);
		uint8_t attrs = m_buffers[ buff_idx ].m_attrs[ i ];
		uint8_t char_set = (attrs >> 4) & 3;
		uint8_t chargen_set = m_chargen_set[ char_set ];
		uint16_t ch_addr = ch & 0x1f;
		if (BIT(ch , 6)) {
			BIT_SET(ch_addr, 5);
		}
		uint8_t byte;
		if ((ch <= 0x1f || ch >= 0x60) && (chargen_set & CHARGEN_C) == 0) {
			if (m_chargen[ char_set ].bytes() >= 0x800) {
				// Read from LC ROM
				byte = ~m_chargen[ char_set ][ (ch_addr << 4) + line_in_row + 0x400 ];
			} else {
				// LC ROM not installed
				byte = 0;
			}
		} else if (ch >= 0x20) {
			// Read from UC ROM
			byte = ~m_chargen[ char_set ][ (ch_addr << 4) + line_in_row ];
		} else {
			byte = 0;
		}
		uint16_t pixels_e;
		uint16_t pixels_o;
		bool microvector = chargen_set & CHARGEN_B;

		if (cyen && (line_in_row == 11 || line_in_row == 12) && i == m_cursor_x) {
			pixels_e = pixels_o = cursor_blink ? ~0 : 0;
		} else if (BIT(fn_12 , 1) && BIT(attrs , 0)) {
			pixels_e = pixels_o = 0;
		} else if (line_in_row == 11 && BIT(attrs , 2)) {
			pixels_e = pixels_o = ~0;
		} else if (microvector) {
			pixels_e = pixels_o = uint16_t(byte) << 1;
			// Copy b0
			if (BIT(pixels_e , 1)) {
				BIT_SET(pixels_e, 0);
				BIT_SET(pixels_o, 0);
			}
		} else {
			bool half_shift = BIT(byte , 0);
			pixels_e = pixels_o = uint16_t(byte & 0xfe);
			if (half_shift) {
				pixels_e <<= 1;
				if (BIT(pixels_o , 7)) {
					BIT_SET(pixels_o, 8);
				}
			}
		}
		if (iv) {
			pixels_e = ~pixels_e;
			pixels_o = ~pixels_o;
		}
		unsigned on_pen = BIT(attrs , 3) ? 1 : 2;
		for (unsigned x = 0; x < VIDEO_CHAR_WIDTH * 2; x += 2) {
			m_bitmap.pix(video_scanline , x_left + x) = m_palette->pen(BIT(pixels_e , 0) ? on_pen : 0);
			m_bitmap.pix(video_scanline , x_left + x + 1) = m_palette->pen(BIT(pixels_o , 0) ? on_pen : 0);
			pixels_e >>= 1;
			pixels_o >>= 1;
		}
	}
}

static const unsigned baud_rate_divisors[] = {
	0,      // 0: external clock (here: no clock)
	2800,   // 1: 110 baud
	2048,   // 2: 150 baud
	1024,   // 3: 300 baud
	256,    // 4: 1200 baud
	128,    // 5: 2400 baud
	64,     // 6: 4800 baud
	32      // 7: 9600 baud
};

void hp2640_base_state::update_async_control(uint8_t new_control)
{
	LOG("ASYNC CT=%02x\n" , new_control);
	uint8_t diff = m_async_control ^ new_control;
	m_async_control = new_control;
	m_rs232->write_rts(BIT(m_async_control , 0));
	if (diff & 0x0e) {
		unsigned new_rate_idx = (m_async_control >> 1) & 0x07;
		// Set baud rate
		double rxc_txc_freq;
		if (new_rate_idx == 0) {
			rxc_txc_freq = 0.0;
		} else {
			rxc_txc_freq = SYS_CLOCK.dvalue() / baud_rate_divisors[ new_rate_idx ];
		}
		m_uart_clock->set_unscaled_clock(rxc_txc_freq);
		m_uart->write_tsb(new_rate_idx == 1);
		LOG("ASYNC freq=%f\n" , rxc_txc_freq);
	}
	if (diff & 0x30) {
		m_uart->write_np(BIT(new_control , 5));
		m_uart->write_nb1(BIT(new_control , 5));
		m_uart->write_eps(BIT(new_control , 4));
	}
	// Update TxD
	async_txd_w(0);
}

void hp2640_base_state::update_async_irq()
{
	m_datacom_irq = m_uart->dav_r();
	LOG("ASYNC IRQ=%d\n" , m_datacom_irq);
	update_irq();
}

#define IOP_MASK(x) BIT_MASK<ioport_value>((x))

static INPUT_PORTS_START(hp2640_base)
	PORT_START("KEY0")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)        // 000 CONTROL
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))        // 001 ESC
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')                      // 002 TAB
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)          // 003 LSHIFT
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))  // 004 RSHIFT
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))    // 005 PAD1
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))    // 006 PAD4
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)                   // 007 BS
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_TOGGLE PORT_NAME("Remote")                             // 010 REMOTE
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')          // 011 1
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')          // 012 Q
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')          // 013 Z
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("RETURN")  // 014 RETURN
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))    // 015 PAD2
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))    // 016 PAD5
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_TILDE) PORT_CHAR('\\') PORT_CHAR('|')     // 017 BACKSLASH
	PORT_BIT(IOP_MASK(16) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_NAME("Caps lock")  // 020 CAPS LOCK
	PORT_BIT(IOP_MASK(17) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')          // 021 2
	PORT_BIT(IOP_MASK(18) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')          // 022 W
	PORT_BIT(IOP_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')          // 023 X
	PORT_BIT(IOP_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']') PORT_CHAR('}')  // 024 ]
	PORT_BIT(IOP_MASK(21) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))    // 025 PAD3
	PORT_BIT(IOP_MASK(22) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))    // 026 PAD6
	PORT_BIT(IOP_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_NAME("f5")  // 027 F5
	PORT_BIT(IOP_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_TOGGLE PORT_NAME("Memory lock")                        // 030 MEMORY LOCK
	PORT_BIT(IOP_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')          // 031 3
	PORT_BIT(IOP_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')          // 032 E
	PORT_BIT(IOP_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')          // 033 C
	PORT_BIT(IOP_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')      // 034 :
	PORT_BIT(IOP_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))      // 035 LEFT
	PORT_BIT(IOP_MASK(30) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Roll up")                                        // 036 ROLL UP
	PORT_BIT(IOP_MASK(31) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_NAME("f6")  // 037 F6
	PORT_START("KEY1")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_TOGGLE PORT_NAME("Auto LF")                            // 040 AUTO LF
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')          // 041 4
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')          // 042 R
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')          // 043 V
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')      // 044 ;
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))      // 045 HOME
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))          // 046 UP
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_NAME("f7")  // 047 F7
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_NAME("Test")                                           // 050 TEST
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')          // 051 5
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')          // 052 T
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')          // 053 B
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')          // 054 L
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))    // 055 RIGHT
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_PGDN) PORT_NAME("Next page")              // 056 NEXT PAGE
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_NAME("f8")  // 057 F8
	PORT_BIT(IOP_MASK(16) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                // 060
	PORT_BIT(IOP_MASK(17) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')          // 061 6
	PORT_BIT(IOP_MASK(18) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')          // 062 Y
	PORT_BIT(IOP_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')                     // 063 SPACE
	PORT_BIT(IOP_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')          // 064 K
	PORT_BIT(IOP_MASK(21) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_PGUP) PORT_NAME("Prev page")              // 065 PREV PAGE
	PORT_BIT(IOP_MASK(22) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Clear dsply")                                    // 066 CLEAR DSPLY
	PORT_BIT(IOP_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_INSERT) PORT_NAME("Insert char")          // 067 INSERT CHAR
	PORT_BIT(IOP_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Display functions")                              // 070 DISPLAY FUNCTIONS
	PORT_BIT(IOP_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')         // 071 7
	PORT_BIT(IOP_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')          // 072 U
	PORT_BIT(IOP_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')          // 073 N
	PORT_BIT(IOP_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')          // 074 J
	PORT_BIT(IOP_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))      // 075 DOWN
	PORT_BIT(IOP_MASK(30) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Set tab")                                        // 076 SET TAB
	PORT_BIT(IOP_MASK(31) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_DEL) PORT_NAME("Delete char")             // 077 DELETE CHAR
	PORT_START("KEY2")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_TOGGLE PORT_NAME("Block mode")                         // 100 BLOCK MODE
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')          // 101 8
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')          // 102 I
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')          // 103 M
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')          // 104 H
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_NAME("Roll down")                                      // 105 ROLL DOWN
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_NAME("Clear tab")                                      // 106 CLEAR TAB
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_NAME("Delete line")                                    // 107 DELETE LINE
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_NAME("Green")                                          // 110 GREEN
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_NAME("Enter")                                          // 111 ENTER
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')          // 112 O
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')      // 113 ,
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')          // 114 G
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR('.')                   // 115 PAD.
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))    // 116 PAD9
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Insert line")                                    // 117 INSERT LINE
	PORT_BIT(IOP_MASK(16) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Read")                                           // 120 READ
	PORT_BIT(IOP_MASK(17) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')          // 121 9
	PORT_BIT(IOP_MASK(18) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')          // 122 P
	PORT_BIT(IOP_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')       // 123 .
	PORT_BIT(IOP_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')          // 124 F
	PORT_BIT(IOP_MASK(21) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))    // 125 PAD0
	PORT_BIT(IOP_MASK(22) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))    // 126 PAD8
	PORT_BIT(IOP_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_NAME("f4")  // 127 F4
	PORT_BIT(IOP_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_NAME("Record")                                         // 130 RECORD
	PORT_BIT(IOP_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_0) PORT_CHAR('0')                         // 131 0
	PORT_BIT(IOP_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR('`')  // 132 @
	PORT_BIT(IOP_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')      // 133 /
	PORT_BIT(IOP_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')          // 134 D
	PORT_BIT(IOP_MASK(29) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                // 135
	PORT_BIT(IOP_MASK(30) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))    // 136 PAD7
	PORT_BIT(IOP_MASK(31) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_NAME("f3")  // 137 F3
	PORT_START("KEY3")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_NAME("Gold")                                           // 140 GOLD
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')      // 141 -
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{') // 142 [
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                 // 143
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')          // 144 S
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                 // 145
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                 // 146
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_NAME("f2")  // 147 F2
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_NAME("Transmit")                                       // 150 TRANSMIT
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')     // 151 ^
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('_')                // 152 _
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                // 153
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')          // 154 A
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                // 155
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                // 156
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD)  PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_NAME("f1")  // 157 F1

	// Default: Full duplex, no parity, 9600 bps
	PORT_START("comm")
	PORT_CONFNAME(0x80, 0x80, "Duplex")
	PORT_CONFSETTING(0x80, "Full")
	PORT_CONFSETTING(0x00, "Half")
	PORT_CONFNAME(0x30, 0x20, "Parity")
	PORT_CONFSETTING(0x00, "Odd")
	PORT_CONFSETTING(0x10, "Even")
	PORT_CONFSETTING(0x20, "None")
	PORT_CONFNAME(0x0e, 0x0e, "Baud rate")
	PORT_CONFSETTING(0x00, "Ext")
	PORT_CONFSETTING(0x02, "110")
	PORT_CONFSETTING(0x04, "150")
	PORT_CONFSETTING(0x06, "300")
	PORT_CONFSETTING(0x08, "1200")
	PORT_CONFSETTING(0x0a, "2400")
	PORT_CONFSETTING(0x0c, "4800")
	PORT_CONFSETTING(0x0e, "9600")

	// Default: all switches closed except R, U & V
	// This setting should work for standard character-oriented I/O
	PORT_START("sw_ah")
	PORT_CONFNAME(0x01, 0x00, "Switch A")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x01, DEF_STR(Off))
	PORT_CONFNAME(0x02, 0x00, "Switch B")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x02, DEF_STR(Off))
	PORT_CONFNAME(0x04, 0x00, "Switch C")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x04, DEF_STR(Off))
	PORT_CONFNAME(0x08, 0x00, "Switch D")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x08, DEF_STR(Off))
	PORT_CONFNAME(0x10, 0x00, "Switch E")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x10, DEF_STR(Off))
	PORT_CONFNAME(0x20, 0x00, "Switch F")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x20, DEF_STR(Off))
	PORT_CONFNAME(0x40, 0x00, "Switch G")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x40, DEF_STR(Off))
	PORT_CONFNAME(0x80, 0x00, "Switch H")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x80, DEF_STR(Off))
	PORT_START("sw_jr")
	PORT_CONFNAME(0x01, 0x00, "Switch J")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x01, DEF_STR(Off))
	PORT_CONFNAME(0x02, 0x00, "Switch K")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x02, DEF_STR(Off))
	PORT_CONFNAME(0x04, 0x00, "Switch L")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x04, DEF_STR(Off))
	PORT_CONFNAME(0x08, 0x00, "Switch M")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x08, DEF_STR(Off))
	PORT_CONFNAME(0x10, 0x00, "Switch N")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x10, DEF_STR(Off))
	PORT_CONFNAME(0x20, 0x00, "Switch P")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x20, DEF_STR(Off))
	PORT_CONFNAME(0x40, 0x00, "Switch Q")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x40, DEF_STR(Off))
	PORT_CONFNAME(0x80, 0x80, "Switch R")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x80, DEF_STR(Off))
	PORT_START("sw_sz")
	PORT_CONFNAME(0x01, 0x00, "Switch S")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x01, DEF_STR(Off))
	PORT_CONFNAME(0x02, 0x00, "Switch T")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x02, DEF_STR(Off))
	PORT_CONFNAME(0x04, 0x04, "Switch U")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x04, DEF_STR(Off))
	PORT_CONFNAME(0x08, 0x08, "Switch V")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x08, DEF_STR(Off))
	PORT_CONFNAME(0x10, 0x00, "Switch W")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x10, DEF_STR(Off))
	PORT_CONFNAME(0x20, 0x00, "Switch X")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x20, DEF_STR(Off))
	PORT_CONFNAME(0x40, 0x00, "Switch Y")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x40, DEF_STR(Off))
	PORT_CONFNAME(0x80, 0x00, "Switch Z")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x80, DEF_STR(Off))
INPUT_PORTS_END

void hp2640_base_state::cpu_mem_map(address_map &map)
{
	map.unmap_value_low();
	map(0x0000, 0x57ff).rom();
	map(0x8000, 0x8fff).view(m_io_view);

	// View 0 is for normal I/O
	// View 1 is for poll read
	// Writing is independent of poll state
	m_io_view[ 0 ](0x0100, 0x0100).r(m_uart, FUNC(ay51013_device::receive));
	m_io_view[ 0 ](0x0120, 0x0120).r(FUNC(hp2640_base_state::async_status_r));

	map(0x8140, 0x8140).w(FUNC(hp2640_base_state::async_control_w));
	map(0x8160, 0x8160).w(m_uart, FUNC(ay51013_device::transmit));
	map(0x8300, 0x8300).w(FUNC(hp2640_base_state::kb_led_w));

	m_io_view[ 0 ](0x0300, 0x030d).r(FUNC(hp2640_base_state::kb_r));
	m_io_view[ 0 ](0x030e, 0x030e).r(FUNC(hp2640_base_state::switches_ah_r));
	m_io_view[ 0 ](0x030f, 0x030f).r(FUNC(hp2640_base_state::datacomm_sw_r));

	map(0x8320, 0x8320).w(FUNC(hp2640_base_state::kb_prev_w));
	map(0x8380, 0x8380).w(FUNC(hp2640_base_state::kb_reset_w));

	m_io_view[ 0 ](0x0380, 0x0380).r(FUNC(hp2640_base_state::switches_jr_r));
	m_io_view[ 0 ](0x03a0, 0x03a0).r(FUNC(hp2640_base_state::switches_sz_r));

	map(0x8700, 0x8700).w(FUNC(hp2640_base_state::cx_w));
	map(0x8720, 0x8720).w(FUNC(hp2640_base_state::cy_w));
	map(0x8b00, 0x8b00).w(m_tapes, FUNC(hp2640_tape_device::command_w));

	m_io_view[ 0 ](0x0b00, 0x0b00).r(m_tapes, FUNC(hp2640_tape_device::status_r));

	map(0x8b20, 0x8b20).w(m_tapes, FUNC(hp2640_tape_device::data_w));

	m_io_view[ 0 ](0x0b20, 0x0b20).r(m_tapes, FUNC(hp2640_tape_device::data_r));

	m_io_view[ 1 ](0x0000, 0x0fff).r(FUNC(hp2640_base_state::poll_r));

	map(0x9100, 0x91ff).ram();
	map(0xc000, 0xffff).ram();
}

void hp2640_base_state::cpu_io_map(address_map &map)
{
	map.unmap_value_low();
	map(0x00, 0xff).w(FUNC(hp2640_base_state::mode_byte_w));
}

void hp2640_base_state::hp2640_base(machine_config &config)
{
	I8080A(config, m_cpu, SYS_CLOCK / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &hp2640_base_state::cpu_mem_map);
	m_cpu->set_addrmap(AS_IO, &hp2640_base_state::cpu_io_map);
	m_cpu->set_irq_acknowledge_callback(FUNC(hp2640_base_state::irq_callback));

	TIMER(config, m_timer_10ms).configure_generic(FUNC(hp2640_base_state::timer_10ms_exp));
	TIMER(config, m_timer_cursor_blink_inh).configure_generic(FUNC(hp2640_base_state::timer_cursor_blink_inh));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER, rgb_t::white());
	// Actual pixel clock is half this value: 21.06 MHz
	// We use the doubled value to be able to emulate the half-pixel shifting of the real hw
	// Each real-world half pixel is a whole MAME pixel
	m_screen->set_raw(VIDEO_DOT_CLOCK * 2 ,
						   VIDEO_TOT_COLS * VIDEO_CHAR_WIDTH * 2 , 0 , VIDEO_VIS_COLS * VIDEO_CHAR_WIDTH * 2 ,
						   VIDEO_TOT_ROWS * VIDEO_CHAR_HEIGHT , 0 , VIDEO_VIS_ROWS * VIDEO_CHAR_HEIGHT);
	m_screen->set_screen_update(FUNC(hp2640_base_state::screen_update));
	TIMER(config, "scantimer").configure_scanline(FUNC(hp2640_base_state::scanline_timer), "screen", 0, 1);
	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);
	config.set_default_layout(layout_hp2640);

	// RS232
	RS232_PORT(config, m_rs232, default_rs232_devices , nullptr);

	// UART (TR1602B)
	AY51013(config, m_uart);
	m_uart->read_si_callback().set(m_rs232, FUNC(rs232_port_device::rxd_r));
	m_uart->write_so_callback().set(FUNC(hp2640_base_state::async_txd_w));
	m_uart->write_dav_callback().set(FUNC(hp2640_base_state::async_dav_w));
	m_uart->set_auto_rdav(true);

	CLOCK(config, m_uart_clock, 19200 * 16);
	m_uart_clock->signal_handler().set(m_uart, FUNC(ay51013_device::write_rcp));
	m_uart_clock->signal_handler().append(m_uart, FUNC(ay51013_device::write_tcp));

	// Beep
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, BEEP_FREQUENCY).add_route(ALL_OUTPUTS, "mono", 1.00);
	TIMER(config, m_timer_beep).configure_generic(FUNC(hp2640_base_state::timer_beep_exp));

	// Tape drives
	HP2640_TAPE(config , m_tapes , SYS_CLOCK);
	m_tapes->irq().set(FUNC(hp2640_base_state::tape_irq_w));
}

// ************
// hp2641_state
// ************
class hp2641_state : public hp2640_base_state
{
public:
	hp2641_state(const machine_config &mconfig, device_type type, const char *tag);

	void hp2641(machine_config &config);

protected:
	void cpu_mem_map(address_map &map) ATTR_COLD;
};

hp2641_state::hp2641_state(const machine_config &mconfig, device_type type, const char *tag)
	: hp2640_base_state(mconfig, type, tag, 0, 0, 0, CHARGEN_B | CHARGEN_C)
{
}

void hp2641_state::hp2641(machine_config &config)
{
	hp2640_base(config);
	m_cpu->set_addrmap(AS_PROGRAM, &hp2641_state::cpu_mem_map);
}

void hp2641_state::cpu_mem_map(address_map &map)
{
	hp2640_base_state::cpu_mem_map(map);
	map(0x7000, 0x77ff).rom(); // The extra APL code ROM
	map(0x9000, 0x90ff).ram(); // 256 bytes of RAM on second CTL PCA board
}

static INPUT_PORTS_START(hp2641)
	PORT_INCLUDE(hp2640_base)

	PORT_MODIFY("KEY0")
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_NAME("LINE FEED") PORT_CHAR(0x0a)                      // 001 LINE FEED
	PORT_BIT(IOP_MASK(24), IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_NAME("APL")                                            // 030 APL
	PORT_MODIFY("KEY3")
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD)   PORT_NAME("BREAK")                                          // 150 BREAK
INPUT_PORTS_END

ROM_START(hp2641)
	ROM_REGION(0x7800, "cpu", 0) // ROMs on first CTL PCA
	ROM_LOAD("1818-0512.bin", 0x0000, 0x800, CRC(1796F4FC) SHA1(1BDEC49B3A937F9D52E143319E4EA3B42DD34FF2))
	ROM_LOAD("1818-0287.bin", 0x0800, 0x800, CRC(717C01A6) SHA1(2BA2362B658A095126A5BFD533A0B35B0EEFC6EC))
	ROM_LOAD("1818-0448.bin", 0x1000, 0x800, CRC(15B09E97) SHA1(7975d04fcf24490b3c16b2ac261754d2ad848017))
	ROM_LOAD("1818-0206.bin", 0x1800, 0x800, CRC(788f8464) SHA1(94c55390801c193bb395855d3a0f186c1d1fd498))
	ROM_LOAD("1818-0273.bin", 0x2000, 0x800, CRC(0DA3F328) SHA1(1FDD7391CBB64BAEE0D69DA147EE2AF123971A61))
	ROM_LOAD("1818-0208.bin", 0x2800, 0x800, CRC(1eca0ff8) SHA1(630a533efe53d3643f652a5d9d6503ab7f47d4e5))
	ROM_LOAD("1818-0209.bin", 0x3000, 0x800, CRC(82e02695) SHA1(eaa7010f4672320116a1f319f96aeb078ce79609))
	ROM_LOAD("1818-0210.bin", 0x3800, 0x800, CRC(A1EBB6AB) SHA1(71B2BEEFF3817D9C87870983BF1653F682B15D21))
	ROM_LOAD("1818-0426.bin", 0x4000, 0x800, CRC(9C49FB37) SHA1(15F2F236F567A6234F9DE8841FE9A6C246022127))
	ROM_LOAD("1818-0275.bin", 0x4800, 0x800, CRC(462175D1) SHA1(DD6974876177453D420201398F88C8389CA8240D))
	ROM_LOAD("1818-0513.bin", 0x5000, 0x800, CRC(416d1a4d) SHA1(6ce7136f36f06fc44716da1970b96a3ef29e5b13))
	// Second CTL PCA. Is this gap in the region a problem? Not so far!
	ROM_LOAD("1818-0276.bin", 0x7000, 0x800, CRC(D061D1B8) SHA1(91FA0541640DF6A0EB8A3DC2C7CAB0EE5967EAAE))

	ROM_REGION(0x800, "chargen0", 0) // HP Roman UC and LC ROMs
	ROM_LOAD("1816-0612.bin", 0x0000, 0x400, CRC(5d7befd6) SHA1(31357e7b8630f52698f1b6825e79c7a51ff3f245))
	ROM_LOAD("1816-0613.bin", 0x0400, 0x400, CRC(b6bac431) SHA1(42a557ecff769425d295ebbd1b73b26ddbfd3a09))

	ROM_REGION(0x800, "chargen1", 0) // APL 1 and APL 2 Character ROMs
	ROM_LOAD("1816-0984.bin", 0x0000, 0x400, CRC(0CEAE759) SHA1(04634EE6271C69D5AC399E547A8738B8AB22EA4A))
	ROM_LOAD("1816-0985.bin", 0x0400, 0x400, CRC(25FCC682) SHA1(E2160ACB34E972ECF59590550327EE4471982AAF))

	ROM_REGION(0x400, "chargen2", 0) // APL 3 Overstruck character ROM
	ROM_LOAD("1816-0986.bin", 0x0000, 0x400, CRC(21D68D26) SHA1(EC76FB234B97106AA2DB3BE163C75864C785C08F))

	ROM_REGION(0x400, "chargen3", 0) // Line Drawing (microvector format)
	ROM_LOAD("1816-1417.bin", 0x0000, 0x400, CRC(E91343A4) SHA1(B37BE2F3699BC8766435B5EE3775D36510DF8D1E))
ROM_END

// ************
// hp2645_state
// ************
class hp2645_state : public hp2640_base_state
{
public:
	hp2645_state(const machine_config &mconfig, device_type type, const char *tag);

	void hp2645(machine_config &config);
};

hp2645_state::hp2645_state(const machine_config &mconfig, device_type type, const char *tag)
	: hp2640_base_state(mconfig, type, tag, 0, CHARGEN_C, CHARGEN_B | CHARGEN_C, CHARGEN_B | CHARGEN_C)
{
}

void hp2645_state::hp2645(machine_config &config)
{
	hp2640_base(config);
}

ROM_START(hp2645)
	ROM_REGION(0x5800 , "cpu" , 0)
	ROM_LOAD("1818-0512.bin", 0x0000, 0x800, CRC(1796f4fc) SHA1(1bdec49b3a937f9d52e143319e4ea3b42dd34ff2))
	ROM_LOAD("1818-0287.bin", 0x0800, 0x800, CRC(717c01a6) SHA1(2ba2362b658a095126a5bfd533a0b35b0eefc6ec))
	ROM_LOAD("1818-0448.bin", 0x1000, 0x800, CRC(15b09e97) SHA1(7975d04fcf24490b3c16b2ac261754d2ad848017))
	ROM_LOAD("1818-0206.bin", 0x1800, 0x800, CRC(788f8464) SHA1(94c55390801c193bb395855d3a0f186c1d1fd498))
	ROM_LOAD("1818-0207.bin", 0x2000, 0x800, CRC(de7186a9) SHA1(aaf5d29f95e5417320e3af573fdcc7ab03922e3d))
	ROM_LOAD("1818-0208.bin", 0x2800, 0x800, CRC(1eca0ff8) SHA1(630a533efe53d3643f652a5d9d6503ab7f47d4e5))
	ROM_LOAD("1818-0209.bin", 0x3000, 0x800, CRC(82e02695) SHA1(eaa7010f4672320116a1f319f96aeb078ce79609))
	ROM_LOAD("1818-0210.bin", 0x3800, 0x800, CRC(a1ebb6ab) SHA1(71b2beeff3817d9c87870983bf1653f682b15d21))
	ROM_LOAD("1818-0426.bin", 0x4000, 0x800, CRC(9c49fb37) SHA1(15f2f236f567a6234f9de8841fe9a6c246022127))
	ROM_LOAD("1818-0212.bin", 0x4800, 0x800, CRC(50221ec0) SHA1(b3fb76da1210ed6eefec9c3fbd731970dd50f962))
	ROM_LOAD("1818-0513.bin", 0x5000, 0x800, CRC(416d1a4d) SHA1(6ce7136f36f06fc44716da1970b96a3ef29e5b13))

	ROM_REGION(0x800, "chargen0", 0)
	ROM_LOAD("1816-0612.bin", 0x0000, 0x400, CRC(5d7befd6) SHA1(31357e7b8630f52698f1b6825e79c7a51ff3f245))
	ROM_LOAD("1816-0613.bin", 0x0400, 0x400, CRC(b6bac431) SHA1(42a557ecff769425d295ebbd1b73b26ddbfd3a09))

	ROM_REGION(0x400, "chargen1", 0)
	ROM_LOAD("1816-0642.bin", 0x0000, 0x400, CRC(2b8d151d) SHA1(208ae3ec780eb8bbbe6ac39cc61141730eda7fdd))

	ROM_REGION(0x400, "chargen2", 0)
	ROM_LOAD("1816-1417.bin", 0x0000, 0x400, CRC(e91343a4) SHA1(b37be2f3699bc8766435b5ee3775d36510df8d1e))

	ROM_REGION(0x400, "chargen3", 0)
	ROM_LOAD("1816-1425.bin", 0x0000, 0x400, CRC(69a34fef) SHA1(816929cadd53c2fe42b3ca561c029cb1ccd4ca24))
ROM_END

} // anonymous namespace

COMP( 1976, hp2641, 0, 0, hp2641, hp2641     , hp2641_state, empty_init, "Hewlett-Packard", "HP 2641A", 0)
COMP( 1976, hp2645, 0, 0, hp2645, hp2640_base, hp2645_state, empty_init, "Hewlett-Packard", "HP 2645A", 0)

// license:BSD-3-Clause
// copyright-holders:F. Ulivi
//
// *******************************
// Driver for HP series 80 systems
// *******************************
//
// This is WIP: lot of things still missing

#include "emu.h"
#include "emupal.h"
#include "screen.h"
#include "cpu/capricorn/capricorn.h"
#include "speaker.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "machine/1ma6.h"
#include "bus/hp80_optroms/hp80_optrom.h"
#include "softlist.h"
#include "machine/bankdev.h"
#include "bus/hp80_io/hp80_io.h"
#include "imagedev/bitbngr.h"

// Debugging
#define VERBOSE 1
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

// **** Constants ****
static constexpr unsigned MASTER_CLOCK  = 9808000;
// Video memory is actually made of 16384 4-bit nibbles
static constexpr unsigned VIDEO_MEM_SIZE= 8192;
static constexpr unsigned ALPHA_MEM_SIZE= 4096;
static constexpr unsigned GRAPH_MEM_SIZE= 16384;
static constexpr unsigned CRT_STS_READY_BIT     = 0;
static constexpr unsigned CRT_STS_DISPLAY_BIT   = 1;
static constexpr unsigned CRT_STS_BUSY_BIT      = 7;
static constexpr unsigned CRT_CTL_RD_RQ_BIT     = 0;
static constexpr unsigned CRT_CTL_WIPEOUT_BIT   = 1;
static constexpr unsigned CRT_CTL_POWERDN_BIT   = 2;
static constexpr unsigned CRT_CTL_GRAPHICS_BIT  = 7;
// Time to read/write a byte in video memory (in master clock cycles)
static constexpr unsigned CRT_RW_TIME           = 96;
// Time taken by hw timer updating (semi-made up) (in usec)
static constexpr unsigned TIMER_BUSY_USEC   = 128;
static constexpr unsigned IRQ_KEYBOARD_BIT  = 0;
static constexpr unsigned IRQ_TIMER0_BIT    = 1;
static constexpr unsigned TIMER_COUNT       = 4;
static constexpr unsigned IRQ_IOP0_BIT      = IRQ_TIMER0_BIT + TIMER_COUNT;
// Maximum count of I/O processors (the same thing as count of I/O slots)
static constexpr unsigned IOP_COUNT         = 4;
static constexpr unsigned IRQ_BIT_COUNT     = IRQ_IOP0_BIT + IOP_COUNT;
static constexpr unsigned NO_IRQ            = IRQ_BIT_COUNT;

// Internal printer has a moving printhead with 8 vertically-arranged resistors that print dots
// by heating thermal paper. The horizontal span of the printhead covers 224 columns.
// In alpha mode, each sweep prints up to 32 characters. Each character has a 8x7 cell.
// 8 pixels of cell height are covered by the printhead height, whereas 7 pixels of width
// allow for 32 characters on a row (224 = 32 * 7).
// After an alpha line is printed the paper advances by 10 pixel lines, so that a space of
// 2 lines is left between alpha lines.
// In graphic mode, printing starts at column 16 and covers 192 columns. So on each side of
// the printed area there's a 16-column wide margin (224 = 192 + 2 * 16).
// Once a graphic line is printed, paper advances by 8 pixel lines so that no space is inserted
// between successive sweeps.
// A full image of the graphic screen (256 x 192) is printed rotated 90 degrees clockwise.
// The printer controller chip (1MA9) has an embedded character generator ROM that is used
// when printing alpha lines. This ROM is also read by the CPU when drawing text on the graphic
// screen (BASIC "LABEL" instruction).
constexpr unsigned PRT_BUFFER_SIZE      = 192;
constexpr unsigned PRTSTS_PAPER_OK_BIT  = 7;
constexpr unsigned PRTSTS_DATARDY_BIT   = 6;
constexpr unsigned PRTSTS_PRTRDY_BIT    = 0;
constexpr unsigned PRTCTL_GRAPHIC_BIT   = 7;
//constexpr unsigned PRTCTL_POWERUP_BIT = 6;
constexpr unsigned PRTCTL_READGEN_BIT   = 5;
// Time to print a line (nominal speed is 2 lines/s)
constexpr unsigned PRT_BUSY_MSEC        = 500;
// Horizontal start position of graphic print (16 columns from left-hand side)
constexpr unsigned PRT_GRAPH_OFFSET     = 16;
// Height of printhead
constexpr unsigned PRT_PH_HEIGHT        = 8;
// Height of alpha rows
constexpr unsigned PRT_ALPHA_HEIGHT     = 10;
// Width of character cells
constexpr unsigned PRT_CELL_WIDTH       = 7;
// Height of graphic rows
//constexpr unsigned PRT_GRAPH_HEIGHT   = 8;
// Width of graphic sweeps
constexpr unsigned PRT_GRAPH_WIDTH      = 192;
// Width of printhead sweeps
constexpr unsigned PRT_WIDTH            = 224;

// ************
//  hp85_state
// ************
class hp85_state : public driver_device
{
public:
	hp85_state(const machine_config &mconfig, device_type type, const char *tag);

	void hp85(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_w);

	IRQ_CALLBACK_MEMBER(irq_callback);

	DECLARE_WRITE8_MEMBER(ginten_w);
	DECLARE_WRITE8_MEMBER(gintdis_w);
	DECLARE_READ8_MEMBER(keysts_r);
	DECLARE_WRITE8_MEMBER(keysts_w);
	DECLARE_READ8_MEMBER(keycod_r);
	DECLARE_WRITE8_MEMBER(keycod_w);
	DECLARE_READ8_MEMBER(crtc_r);
	DECLARE_WRITE8_MEMBER(crtc_w);
	DECLARE_READ8_MEMBER(clksts_r);
	DECLARE_WRITE8_MEMBER(clksts_w);
	DECLARE_READ8_MEMBER(clkdat_r);
	DECLARE_WRITE8_MEMBER(clkdat_w);
	DECLARE_WRITE8_MEMBER(prtlen_w);
	DECLARE_READ8_MEMBER(prchar_r);
	DECLARE_WRITE8_MEMBER(prchar_w);
	DECLARE_READ8_MEMBER(prtsts_r);
	DECLARE_WRITE8_MEMBER(prtctl_w);
	DECLARE_WRITE8_MEMBER(prtdat_w);
	DECLARE_WRITE8_MEMBER(rselec_w);
	DECLARE_READ8_MEMBER(intrsc_r);
	DECLARE_WRITE8_MEMBER(intrsc_w);

	TIMER_DEVICE_CALLBACK_MEMBER(kb_scan);
	TIMER_DEVICE_CALLBACK_MEMBER(vm_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_update);
	TIMER_DEVICE_CALLBACK_MEMBER(clk_busy_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(prt_busy_timer);

	DECLARE_WRITE8_MEMBER(irl_w);
	DECLARE_WRITE8_MEMBER(halt_w);

	void cpu_mem_map(address_map &map);
	void rombank_mem_map(address_map &map);

	required_device<capricorn_cpu_device> m_cpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<timer_device> m_vm_timer;
	required_device<timer_device> m_clk_busy_timer;
	required_device<timer_device> m_prt_busy_timer;
	required_device<beep_device> m_beep;
	required_device<dac_1bit_device> m_dac;
	required_ioport m_io_key0;
	required_ioport m_io_key1;
	required_ioport m_io_key2;
	required_ioport m_io_modkeys;
	required_device_array<hp80_optrom_slot_device , 6> m_rom_drawers;
	required_device<address_map_bank_device> m_rombank;
	required_device_array<hp80_io_slot_device , IOP_COUNT> m_io_slots;
	required_device<bitbanger_device> m_prt_graph_out;
	required_device<bitbanger_device> m_prt_alpha_out;

	// Character generators
	required_region_ptr<uint8_t> m_chargen;
	required_region_ptr<uint8_t> m_prt_chargen;

	bitmap_rgb32 m_bitmap;
	std::vector<uint8_t> m_video_mem;
	uint16_t m_crt_sad;
	uint16_t m_crt_bad;
	uint8_t m_crt_sts;
	uint8_t m_crt_ctl;
	uint8_t m_crt_read_byte;
	uint8_t m_crt_write_byte;
	bool m_global_int_en;
	uint16_t m_int_serv;
	unsigned m_top_pending;
	uint16_t m_int_acked;
	uint16_t m_int_en;
	uint8_t m_halt_lines;

	// State of keyboard
	ioport_value m_kb_state[ 3 ];
	bool m_kb_enable;
	bool m_kb_pressed;
	bool m_kb_flipped;
	uint8_t m_kb_keycode;

	// Timers
	typedef struct {
		uint8_t m_timer_cnt[ 4 ];
		uint8_t m_timer_reg[ 4 ];
		bool m_timer_en;
		bool m_timer_clr;
		uint8_t m_digit_to_match;
	} hw_timer_t;
	hw_timer_t m_hw_timer[ TIMER_COUNT ];
	uint8_t m_timer_idx;
	bool m_clk_busy;

	// Printer
	uint8_t m_prtlen;
	uint8_t m_prt_idx;
	uint8_t m_prchar_r;
	uint8_t m_prchar_w;
	uint8_t m_prtsts;
	uint8_t m_prtctl;
	uint8_t m_prt_buffer[ PRT_BUFFER_SIZE ];

	attotime time_to_video_mem_availability() const;
	static void get_video_addr(uint16_t addr , uint16_t& byte_addr , bool& lsb_nibble);
	uint8_t video_mem_r(uint16_t addr , uint16_t addr_mask) const;
	void video_mem_w(uint16_t addr , uint16_t addr_mask , uint8_t data);
	void video_mem_read();
	void video_mem_write();

	bool kb_scan_ioport(ioport_value pressed , unsigned idx_base , uint8_t& keycode);

	void irq_w(unsigned n_irq , bool state);
	void irq_en_w(unsigned n_irq , bool state);
	void update_int_bits();
	void update_irl();

	uint8_t get_prt_font(uint8_t ch , unsigned col) const;
	void prt_format_alpha(unsigned row , uint8_t *pixel_row) const;
	void prt_format_graphic(unsigned row , uint8_t *pixel_row) const;
	void prt_output_row(const uint8_t *pixel_row);
	void prt_do_printing();
};

hp85_state::hp85_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig , type , tag),
	  m_cpu(*this , "cpu"),
	  m_screen(*this , "screen"),
	  m_palette(*this , "palette"),
	  m_vm_timer(*this , "vm_timer"),
	  m_clk_busy_timer(*this , "clk_busy_timer"),
	  m_prt_busy_timer(*this , "prt_busy_timer"),
	  m_beep(*this , "beeper"),
	  m_dac(*this , "dac"),
	  m_io_key0(*this , "KEY0"),
	  m_io_key1(*this , "KEY1"),
	  m_io_key2(*this , "KEY2"),
	  m_io_modkeys(*this, "MODKEYS"),
	  m_rom_drawers(*this , "drawer%u" , 1),
	  m_rombank(*this , "rombank"),
	  m_io_slots(*this , "slot%u" , 1),
	  m_prt_graph_out(*this , "prt_graphic"),
	  m_prt_alpha_out(*this , "prt_alpha"),
	  m_chargen(*this , "chargen"),
	  m_prt_chargen(*this , "prt_chargen")
{
}

void hp85_state::machine_start()
{
	m_screen->register_screen_bitmap(m_bitmap);
	m_video_mem.resize(VIDEO_MEM_SIZE);
}

void hp85_state::machine_reset()
{
	m_crt_sad = 0;
	m_crt_bad = 0;
	m_crt_sts = 0x7c;
	m_crt_ctl = BIT_MASK<uint8_t>(CRT_CTL_POWERDN_BIT) | BIT_MASK<uint8_t>(CRT_CTL_WIPEOUT_BIT);
	m_crt_read_byte = 0;
	m_crt_write_byte = 0;
	m_int_serv = 0;
	m_top_pending = NO_IRQ;
	m_int_acked = 0;
	m_int_en = 0;
	m_global_int_en = false;
	m_kb_state[ 0 ] = 0;
	m_kb_state[ 1 ] = 0;
	m_kb_state[ 2 ] = 0;
	m_kb_keycode = 0xff;
	m_kb_enable = true;
	m_kb_pressed = false;
	m_kb_flipped = false;
	for (auto& timer : m_hw_timer) {
		for (unsigned i = 0; i < 4; i++) {
			timer.m_timer_cnt[ i ] = 0;
			timer.m_timer_reg[ i ] = 0;
		}
		timer.m_timer_en = false;
		timer.m_timer_clr = false;
		timer.m_digit_to_match = 0;
	}
	m_timer_idx = 0;
	m_clk_busy = false;
	update_irl();
	m_halt_lines = 0;
	m_cpu->set_input_line(INPUT_LINE_HALT , CLEAR_LINE);
	m_prtlen = 0;
	m_prt_idx = PRT_BUFFER_SIZE;
	m_prchar_r = 0;
	m_prchar_w = 0;
	m_prtsts = BIT_MASK<uint8_t>(PRTSTS_PAPER_OK_BIT) | BIT_MASK<uint8_t>(PRTSTS_PRTRDY_BIT);
	m_prtctl = 0;

	// Load optional ROMs (if any)
	// All entries in rombanks [01..FF] initially not present
	m_rombank->space(AS_PROGRAM).unmap_read(HP80_OPTROM_SIZE * 1 , HP80_OPTROM_SIZE * 0x100 - 1);
	for (auto& draw : m_rom_drawers) {
		LOG("Loading opt ROM in drawer %s\n" , draw->tag());
		draw->install_read_handler(m_rombank->space(AS_PROGRAM));
	}
	// Clear RSELEC
	m_rombank->set_bank(0xff);

	// Mount I/O slots in address space
	m_cpu->space(AS_PROGRAM).unmap_readwrite(0xff50 , 0xff5f);
	for (auto& io : m_io_slots) {
		io->install_read_write_handlers(m_cpu->space(AS_PROGRAM));
	}
}

uint32_t hp85_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

WRITE_LINE_MEMBER(hp85_state::vblank_w)
{
	COPY_BIT(!state , m_crt_sts , CRT_STS_DISPLAY_BIT);
	if (state) {
		if (BIT(m_crt_ctl , CRT_CTL_WIPEOUT_BIT) || BIT(m_crt_ctl , CRT_CTL_POWERDN_BIT)) {
			// Blank video
			m_bitmap.fill(rgb_t::black());
		} else if (BIT(m_crt_ctl , CRT_CTL_GRAPHICS_BIT)) {
			// Render graphic video
			uint16_t video_start = m_crt_sad;
			for (unsigned y = 0; y < 192; y++) {
				for (unsigned x = 0; x < 256; x += 8) {
					uint8_t pixels = video_mem_r(video_start , GRAPH_MEM_SIZE / 2 - 1);
					video_start += 2;
					for (unsigned sub_x = 0; sub_x < 8; sub_x++) {
						m_bitmap.pix32(y , x + sub_x) = m_palette->pen(BIT(pixels , 7));
						pixels <<= 1;
					}
				}
			}
		} else {
			// Render alpha video
			uint16_t video_start = m_crt_sad;
			for (unsigned row = 0; row < 192; row += 12) {
				for (unsigned col = 0; col < 256; col += 8) {
					uint8_t ch = video_mem_r(video_start , ALPHA_MEM_SIZE / 2 - 1);
					video_start += 2;
					for (unsigned sub_row = 0; sub_row < 12; sub_row++) {
						uint8_t pixels;
						if (sub_row < 8) {
							pixels = m_chargen[ (ch & 0x7f) * 8 + sub_row ];
						} else if (BIT(ch , 7) && (sub_row == 9 || sub_row == 10)) {
							// Underline
							pixels = 0xfe;
						} else {
							pixels = 0;
						}
						for (unsigned sub_x = 0; sub_x < 8; sub_x++) {
							m_bitmap.pix32(row + sub_row , col + sub_x) = m_palette->pen(BIT(pixels , 7));
							pixels <<= 1;
						}
					}
				}
			}
		}
	}
}

// Vector table (indexed by bit no. in m_int_serv)
static const uint8_t vector_table[] = {
	0x04,   // Keyboard
	0x08,   // Timer 0
	0x0a,   // Timer 1
	0x0c,   // Timer 2
	0x0e,   // Timer 3
	0x10,   // Slot 1
	0x10,   // Slot 2
	0x10,   // Slot 3
	0x10,   // Slot 4
	0x00    // No IRQ
};

IRQ_CALLBACK_MEMBER(hp85_state::irq_callback)
{
	logerror("IRQ ACK %u\n" , m_top_pending);
	BIT_SET(m_int_acked , m_top_pending);
	if (m_top_pending > IRQ_IOP0_BIT && m_top_pending < IRQ_BIT_COUNT) {
		// Interrupts are disabled in all I/O translators of higher priority than
		// the one being serviced
		for (unsigned i = m_top_pending - 1; i >= IRQ_IOP0_BIT; i--) {
			irq_en_w(i , false);
		}
	}
	update_irl();
	return vector_table[ m_top_pending ];
}

WRITE8_MEMBER(hp85_state::ginten_w)
{
	m_global_int_en = true;
	update_irl();
}

WRITE8_MEMBER(hp85_state::gintdis_w)
{
	m_global_int_en = false;
	update_irl();
}

READ8_MEMBER(hp85_state::keysts_r)
{
	uint8_t res = 0;
	if (BIT(m_int_en , IRQ_KEYBOARD_BIT)) {
		BIT_SET(res , 0);
	}
	if (m_kb_pressed) {
		BIT_SET(res , 1);
	}
	if (BIT(m_io_modkeys->read() , 0)) {
		BIT_SET(res , 3);
	}
	if (m_global_int_en) {
		BIT_SET(res , 7);
	}
	return res;
}

WRITE8_MEMBER(hp85_state::keysts_w)
{
	if (BIT(data , 0)) {
		irq_en_w(IRQ_KEYBOARD_BIT , true);
	} else if (BIT(data , 1)) {
		irq_en_w(IRQ_KEYBOARD_BIT , false);
	}
	m_dac->write(BIT(data , 5));
	m_beep->set_state(BIT(data , 6));
	if (BIT(data , 7)) {
		m_kb_flipped = !m_kb_flipped;
	}
}

READ8_MEMBER(hp85_state::keycod_r)
{
	return m_kb_keycode;
}

WRITE8_MEMBER(hp85_state::keycod_w)
{
	if (data == 1) {
		irq_w(IRQ_KEYBOARD_BIT , false);
		m_kb_enable = true;
	}
}

READ8_MEMBER(hp85_state::crtc_r)
{
	uint8_t res = 0xff;

	// Read from CRT controller (1MA5)
	switch (offset) {
	case 0:
		// CRTSAD: write-only
		break;

	case 1:
		// CRTBAD: write-only
		break;

	case 2:
		// CRTSTS
		res = m_crt_sts;
		break;

	case 3:
		// CRTDAT
		res = m_crt_read_byte;
		break;
	}
	return res;
}

WRITE8_MEMBER(hp85_state::crtc_w)
{
	// Write to CRT controller (1MA5)
	uint8_t burst_idx = m_cpu->flatten_burst();
	switch (offset) {
	case 0:
		// CRTSAD
		if (burst_idx == 1) {
			m_crt_sad = ((uint16_t)data << 8) | (m_crt_sad & 0xff);
		} else if (burst_idx == 0) {
			m_crt_sad = (m_crt_sad & 0xff00) | data;
		}
		break;

	case 1:
		// CRTBAD
		if (burst_idx == 1) {
			m_crt_bad = ((uint16_t)data << 8) | (m_crt_bad & 0xff);
		} else if (burst_idx == 0) {
			m_crt_bad = (m_crt_bad & 0xff00) | data;
		}
		break;

	case 2:
		// CRTCTL
		m_crt_ctl = data;
		if (BIT(m_crt_ctl , CRT_CTL_RD_RQ_BIT)) {
			BIT_CLR(m_crt_sts , CRT_STS_READY_BIT);
			BIT_SET(m_crt_sts , CRT_STS_BUSY_BIT);
			attotime vm_av = time_to_video_mem_availability();
			m_vm_timer->adjust(vm_av + attotime::from_ticks(CRT_RW_TIME , MASTER_CLOCK));
		}
		break;

	case 3:
		// CRTDAT
		{
			m_crt_write_byte = data;
			BIT_CLR(m_crt_sts , CRT_STS_READY_BIT);
			BIT_SET(m_crt_sts , CRT_STS_BUSY_BIT);
			attotime vm_av = time_to_video_mem_availability();
			m_vm_timer->adjust(vm_av + attotime::from_ticks(CRT_RW_TIME , MASTER_CLOCK));
		}
		break;
	}
}

READ8_MEMBER(hp85_state::clksts_r)
{
	uint8_t res = 0;
	for (unsigned i = 0; i < TIMER_COUNT; i++) {
		if (BIT(m_int_en , IRQ_TIMER0_BIT + i)) {
			BIT_SET(res , i);
		}
	}
	if (!m_clk_busy) {
		BIT_SET(res , 7);
	}
	//logerror("CLKSTS R=%02x\n" , res);
	return res;
}

WRITE8_MEMBER(hp85_state::clksts_w)
{
	// logerror("CLKSTS W=%02x\n" , data);
	if (data == 0x0c) {
		// Set test mode (see timer_update)
		auto& timer = m_hw_timer[ m_timer_idx ];
		timer.m_digit_to_match = 1;
		timer.m_timer_cnt[ 0 ] = timer.m_timer_reg[ 0 ];
		timer.m_timer_cnt[ 1 ] = timer.m_timer_reg[ 1 ];
		timer.m_timer_cnt[ 2 ] = timer.m_timer_reg[ 2 ];
		timer.m_timer_cnt[ 3 ] = timer.m_timer_reg[ 3 ];
		logerror("Test mode enabled for timer %u\n" , m_timer_idx);
	} else {
		m_timer_idx = (data >> 6) & 3;
		auto& timer = m_hw_timer[ m_timer_idx ];
		if (BIT(data , 0)) {
			// Disable timer irq
			irq_en_w(IRQ_TIMER0_BIT + m_timer_idx , false);
		} else if (BIT(data , 1)) {
			// Enable timer irq
			irq_en_w(IRQ_TIMER0_BIT + m_timer_idx , true);
		}
		if (BIT(data , 2)) {
			// Stop timer
			timer.m_timer_en = false;
		} else if (BIT(data , 3)) {
			// Start timer
			timer.m_timer_en = true;
		}
		if (BIT(data , 4) || (BIT(data , 3) && timer.m_digit_to_match)) {
			// Clear timer
			timer.m_timer_clr = true;
			// Disable test mode
			timer.m_digit_to_match = 0;
		}
		if (BIT(data , 5)) {
			// Clear timer irq
			irq_w(IRQ_TIMER0_BIT + m_timer_idx , false);
		}
		update_int_bits();
	}
}

READ8_MEMBER(hp85_state::clkdat_r)
{
	uint8_t res;
	unsigned burst_idx = m_cpu->flatten_burst();
	if (burst_idx < 4) {
		res = m_hw_timer[ m_timer_idx ].m_timer_cnt[ burst_idx ];
	} else {
		// What happens when loading more than 4 bytes from timers?
		logerror("Reading more than 4 bytes from timer %u\n" , m_timer_idx);
		res = 0;
	}
	//logerror("CLKDAT R %u=%02x\n" , burst_idx , res);
	return res;
}

WRITE8_MEMBER(hp85_state::clkdat_w)
{
	unsigned burst_idx = m_cpu->flatten_burst();
	//logerror("CLKDAT W %u=%02x\n" , burst_idx , data);
	if (burst_idx < 4) {
		m_hw_timer[ m_timer_idx ].m_timer_reg[ burst_idx ] = data;
	} else {
		// What happens when storing more than 4 bytes into timers?
		logerror("Writing more than 4 bytes into timer %u\n" , m_timer_idx);
	}
}

WRITE8_MEMBER(hp85_state::prtlen_w)
{
	//LOG("PRTLEN=%u\n" , data);
	if (data == 0) {
		// Advance paper
		memset(m_prt_buffer , 0 , sizeof(m_prt_buffer));
		m_prt_idx = 0;
		prt_do_printing();
	} else {
		m_prtlen = data;
		if (!BIT(m_prtctl , PRTCTL_GRAPHIC_BIT)) {
			m_prt_idx = 0;
		}
	}
}

READ8_MEMBER(hp85_state::prchar_r)
{
	return m_prchar_r;
}

WRITE8_MEMBER(hp85_state::prchar_w)
{
	m_prchar_w = data;
}

READ8_MEMBER(hp85_state::prtsts_r)
{
	return m_prtsts;
}

WRITE8_MEMBER(hp85_state::prtctl_w)
{
	//LOG("PRTCTL=%02x\n" , data);
	m_prtctl = data;
	BIT_SET(m_prtsts , PRTSTS_PRTRDY_BIT);
	if (BIT(m_prtctl , PRTCTL_READGEN_BIT)) {
		// Reading printer char. gen.
		m_prchar_r = get_prt_font(m_prchar_w , m_prtctl & 7);
		BIT_SET(m_prtsts , PRTSTS_DATARDY_BIT);
	} else {
		BIT_CLR(m_prtsts , PRTSTS_DATARDY_BIT);
	}
	if (BIT(m_prtctl , PRTCTL_GRAPHIC_BIT)) {
		m_prt_idx = 0;
	}
}

WRITE8_MEMBER(hp85_state::prtdat_w)
{
	m_cpu->flatten_burst();
	//LOG("PRTDAT=%02x\n" , data);
	if (m_prt_idx < PRT_BUFFER_SIZE) {
		m_prt_buffer[ m_prt_idx++ ] = data;
		if (m_prt_idx == PRT_BUFFER_SIZE || (!BIT(m_prtctl , PRTCTL_GRAPHIC_BIT) && m_prt_idx >= m_prtlen)) {
			//LOG("Print\n");
			prt_do_printing();
			m_prt_idx = PRT_BUFFER_SIZE;
		}
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(hp85_state::prt_busy_timer)
{
	BIT_SET(m_prtsts , PRTSTS_PRTRDY_BIT);
}

WRITE8_MEMBER(hp85_state::rselec_w)
{
	m_rombank->set_bank(data);
}

READ8_MEMBER(hp85_state::intrsc_r)
{
	if (m_top_pending >= IRQ_IOP0_BIT && m_top_pending < IRQ_BIT_COUNT && BIT(m_int_acked , m_top_pending)) {
		return (uint8_t)m_io_slots[ m_top_pending - IRQ_IOP0_BIT ]->get_base_addr();
	} else {
		// Probably..
		return 0xff;
	}
}

WRITE8_MEMBER(hp85_state::intrsc_w)
{
	if (m_top_pending >= IRQ_IOP0_BIT && m_top_pending < IRQ_BIT_COUNT && BIT(m_int_acked , m_top_pending)) {
		// Clear interrupt request in the slot being serviced
		m_io_slots[ m_top_pending - IRQ_IOP0_BIT ]->clear_service();
	}
	for (auto& iop: m_io_slots) {
		iop->inten();
	}
	for (unsigned i = IRQ_IOP0_BIT; i < (IRQ_IOP0_BIT + IOP_COUNT); i++) {
		irq_en_w(i , true);
	}
}

// Outer index: key position [0..79] = r * 8 + c
// Inner index: SHIFT state (0 = no SHIFT, 1 = SHIFT)
static const uint8_t keyboard_table[ 80 ][ 2 ] = {
	// --    SHIFT
	{ 0xa2 , 0xac },    // 0,0: Down / Auto
	{ 0xa1 , 0xa5 },    // 0,1: Up / Home
	{ 0x83 , 0x87 },    // 0,2: k4 / k8
	{ 0x82 , 0x86 },    // 0,3: k3 / k7
	{ 0x81 , 0x85 },    // 0,4: k2 / k6
	{ 0x80 , 0x84 },    // 0,5: k1 / k5
	{ 0x96 , 0x60 },    // 0,6: LABEL KEY
	{ 0xff , 0xff },    // 0,7: N/U
	{ 0x38 , 0x2a },    // 1,0: 8
	{ 0x37 , 0x26 },    // 1,1: 7
	{ 0x36 , 0x5e },    // 1,2: 6
	{ 0x35 , 0x25 },    // 1,3: 5
	{ 0x34 , 0x24 },    // 1,4: 4
	{ 0x33 , 0x23 },    // 1,5: 3
	{ 0x32 , 0x40 },    // 1,6: 2
	{ 0x31 , 0x21 },    // 1,7: 1
	{ 0x49 , 0x69 },    // 2,0: I
	{ 0x55 , 0x75 },    // 2,1: U
	{ 0x59 , 0x79 },    // 2,2: Y
	{ 0x54 , 0x74 },    // 2,3: T
	{ 0x52 , 0x72 },    // 2,4: R
	{ 0x45 , 0x65 },    // 2,5: E
	{ 0x57 , 0x77 },    // 2,6: W
	{ 0x51 , 0x71 },    // 2,7: Q
	{ 0x4b , 0x6b },    // 3,0: K
	{ 0x4a , 0x6a },    // 3,1: J
	{ 0x48 , 0x68 },    // 3,2: H
	{ 0x47 , 0x67 },    // 3,3: G
	{ 0x46 , 0x66 },    // 3,4: F
	{ 0x44 , 0x64 },    // 3,5: D
	{ 0x53 , 0x73 },    // 3,6: S
	{ 0x41 , 0x61 },    // 3,7: A
	{ 0x4d , 0x6d },    // 4,0: M
	{ 0x4e , 0x6e },    // 4,1: N
	{ 0x42 , 0x62 },    // 4,2: B
	{ 0x56 , 0x76 },    // 4,3: V
	{ 0x43 , 0x63 },    // 4,4: C
	{ 0x58 , 0x78 },    // 4,5: X
	{ 0x5a , 0x7a },    // 4,6: Z
	{ 0x20 , 0x20 },    // 4,7: Space
	{ 0x2c , 0x3c },    // 5,0: ,
	{ 0x2e , 0x3e },    // 5,1: .
	{ 0x2f , 0x3f },    // 5,2: / ?
	{ 0x8e , 0x90 },    // 5,3: PAUSE / STEP
	{ 0x8d , 0x8d },    // 5,4: RUN
	{ 0x2b , 0x7f },    // 5,5: KP +
	{ 0x2d , 0x7d },    // 5,6: KP -
	{ 0x2a , 0x7e },    // 5,7: KP *
	{ 0x4c , 0x6c },    // 6,0: L
	{ 0x3b , 0x3a },    // 6,1: ;
	{ 0x27 , 0x22 },    // 6,2: ' "
	{ 0x9a , 0x9a },    // 6,3: END LINE
	{ 0x94 , 0x95 },    // 6,4: LIST / P LST
	{ 0xff , 0xff },    // 6,5: N/U
	{ 0xff , 0xff },    // 6,6: N/U
	{ 0x2f , 0x7b },    // 6,7: KP /
	{ 0x4f , 0x6f },    // 7,0: O
	{ 0x50 , 0x70 },    // 7,1: P
	{ 0x28 , 0x5b },    // 7,2: ( [
	{ 0x29 , 0x5d },    // 7,3: ) ]
	{ 0x8f , 0xad },    // 7,4: CONT / SCRATCH
	{ 0xa0 , 0x92 },    // 7,5: -LINE / CLEAR
	{ 0x29 , 0x8c },    // 7,6: ) INIT
	{ 0xff , 0xff },    // 7,7: N/U
	{ 0x39 , 0x28 },    // 8,0: 9
	{ 0x30 , 0x29 },    // 8,1: 0
	{ 0x2d , 0x5f },    // 8,2: - _
	{ 0x3d , 0x2b },    // 8,3: = +
	{ 0x5c , 0x7c },    // 8,4: \ |
	{ 0x99 , 0x9b },    // 8,5: BS
	{ 0x28 , 0x8b },    // 8,6: ( RESET
	{ 0x5e , 0xa6 },    // 8,7: ^ / RESLT
	{ 0x9c , 0x93 },    // 9,0: LEFT / GRAPH
	{ 0x9d , 0x89 },    // 9,1: RIGHT / COPY
	{ 0xa3 , 0xa3 },    // 9,2: RPL / INS
	{ 0xa4 , 0xa8 },    // 9,3: -CHAR / DEL
	{ 0x9f , 0x9e },    // 9,4: ROLL
	{ 0xaa , 0x88 },    // 9,5: LOAD / REW
	{ 0xa9 , 0x91 },    // 9,6: STORE / TEST
	{ 0x8a , 0x8a }     // 9,7: PAPER ADVANCE
};

bool hp85_state::kb_scan_ioport(ioport_value pressed , unsigned idx_base , uint8_t& keycode)
{
	while (pressed) {
		unsigned bit_no = 31 - count_leading_zeros(pressed);
		uint8_t unshifted = keyboard_table[ idx_base + bit_no ][ 0 ];
		bool isalpha = unshifted >= 'A' && unshifted <= 'Z';
		ioport_value modifiers = m_io_modkeys->read();
		bool shift = BIT(modifiers , 0);
		bool caps_lock = BIT(modifiers , 1);
		bool control = BIT(modifiers , 2);
		if (isalpha) {
			shift = shift ^ caps_lock ^ m_kb_flipped;
		}
		keycode = keyboard_table[ idx_base + bit_no ][ shift ];
		uint8_t tmp = isalpha ? unshifted : keycode;
		if (control && (tmp & 0xe0) == 0x40) {
			keycode &= ~0xe0;
		}
		if (keycode != 0xff) {
			return true;
		}
		ioport_value mask = BIT_MASK<ioport_value>(bit_no);
		pressed &= ~mask;
	}
	return false;
}

TIMER_DEVICE_CALLBACK_MEMBER(hp85_state::kb_scan)
{
	ioport_value input[ 3 ];
	input[ 0 ] = m_io_key0->read();
	input[ 1 ] = m_io_key1->read();
	input[ 2 ] = m_io_key2->read();

	if (m_kb_enable) {
		uint8_t keycode;

		bool got_key = kb_scan_ioport(input[ 0 ] & ~m_kb_state[ 0 ] , 0 , keycode) ||
			kb_scan_ioport(input[ 1 ] & ~m_kb_state[ 1 ] , 32 , keycode) ||
			kb_scan_ioport(input[ 2 ] & ~m_kb_state[ 2 ] , 64 , keycode);

		if (got_key) {
			m_kb_keycode = keycode;
			irq_w(IRQ_KEYBOARD_BIT , true);
			m_kb_enable = false;
		}
	}
	m_kb_pressed = input[ 0 ] != 0 ||
		input[ 1 ] != 0 ||
		input[ 2 ] != 0;

	m_kb_state[ 0 ] = input[ 0 ];
	m_kb_state[ 1 ] = input[ 1 ];
	m_kb_state[ 2 ] = input[ 2 ];
}

TIMER_DEVICE_CALLBACK_MEMBER(hp85_state::vm_timer)
{
	if (BIT(m_crt_ctl , CRT_CTL_RD_RQ_BIT)) {
		video_mem_read();
	} else {
		video_mem_write();
	}
	BIT_CLR(m_crt_sts , CRT_STS_BUSY_BIT);
}

TIMER_DEVICE_CALLBACK_MEMBER(hp85_state::timer_update)
{
	for (unsigned i = 0; i < TIMER_COUNT; i++) {
		auto& timer = m_hw_timer[ i ];
		if (timer.m_timer_clr) {
			timer.m_timer_clr = false;
			timer.m_timer_cnt[ 0 ] = 0;
			timer.m_timer_cnt[ 1 ] = 0;
			timer.m_timer_cnt[ 2 ] = 0;
			timer.m_timer_cnt[ 3 ] = 0;
		} else if (timer.m_timer_en) {
			if (timer.m_digit_to_match) {
				// Timers have an undocumented mode (used by test "J" of service ROM)
				// where the counter has to match in sequence all digits of register
				// in order to raise an interrupt. In other words interrupt is generated
				// after a number of updates that's equal to the sum of all digits in
				// register + 1. My opinion is that people at HP designed this mode to
				// allow all digits in a timer to be tested quickly. Without this special
				// mode it takes more than 27 hours to check that all digits increment
				// correctly and that there are no stuck bits.
				// From an operative point of view, we copy register into counter when
				// this special mode is activated (see clksts_w). Then, at each update,
				// we decrement the digit of counter pointed to by m_digit_to_match (1 =
				// least significant digit). Each time a digit "borrows" (i.e. it decrements
				// from 0 to 9), we move on to digit at left. When m_digit_to_match reaches
				// 9, interrupt is raised and the timer stops.
				// At this point counter is always "99999999".
				if (timer.m_digit_to_match < 9) {
					while (true) {
						bool borrow = false;
						uint8_t b = timer.m_timer_cnt[ (timer.m_digit_to_match - 1) / 2 ];
						if (BIT(timer.m_digit_to_match , 0)) {
							// Least significant digit in b
							if (b & 0x0f) {
								b--;
							} else {
								b = (b & 0xf0) | 9;
								borrow = true;
							}
						} else {
							// Most significant digit in b
							if (b & 0xf0) {
								b -= 0x10;
							} else {
								b = 0x99;
								borrow = true;
							}
						}
						timer.m_timer_cnt[ (timer.m_digit_to_match - 1) / 2 ] = b;
						if (borrow) {
							timer.m_digit_to_match++;
							if (timer.m_digit_to_match == 9) {
								irq_w(IRQ_TIMER0_BIT + i , true);
								break;
							}
						} else {
							break;
						}
					}
				}
			} else {
				// Standard timer mode
				// Increment all active timers by 1
				bool carry = true;
				for (unsigned idx = 0; idx < 4 && carry; idx++) {
					carry = false;
					uint8_t b = timer.m_timer_cnt[ idx ];
					b++;
					if ((b & 0xf) > 9) {
						b += 6;
						if (b >= 0xa0) {
							b += 0x60;
							carry = true;
						}
					}
					timer.m_timer_cnt[ idx ] = b;
				}
				if (timer.m_timer_cnt[ 0 ] == timer.m_timer_reg[ 0 ] &&
					timer.m_timer_cnt[ 1 ] == timer.m_timer_reg[ 1 ] &&
					timer.m_timer_cnt[ 2 ] == timer.m_timer_reg[ 2 ] &&
					timer.m_timer_cnt[ 3 ] == timer.m_timer_reg[ 3 ]) {
					timer.m_timer_cnt[ 0 ] = 0;
					timer.m_timer_cnt[ 1 ] = 0;
					timer.m_timer_cnt[ 2 ] = 0;
					timer.m_timer_cnt[ 3 ] = 0;
					irq_w(IRQ_TIMER0_BIT + i , true);
				}
			}
		}
	}
	m_clk_busy = true;
	m_clk_busy_timer->adjust(attotime::from_usec(TIMER_BUSY_USEC));
}

TIMER_DEVICE_CALLBACK_MEMBER(hp85_state::clk_busy_timer)
{
	m_clk_busy = false;
}

WRITE8_MEMBER(hp85_state::irl_w)
{
	//LOG("irl_w %u=%u\n" , offset , data);
	irq_w(offset + IRQ_IOP0_BIT , data != 0);
}

WRITE8_MEMBER(hp85_state::halt_w)
{
	//LOG("halt_w %u=%u\n" , offset , data);
	bool prev_halt = m_halt_lines != 0;
	COPY_BIT(data != 0 , m_halt_lines , offset);
	bool new_halt = m_halt_lines != 0;
	if (prev_halt != new_halt) {
		LOG("halt=%d hl=%x\n" , new_halt , m_halt_lines);
		m_cpu->set_input_line(INPUT_LINE_HALT , new_halt);
	}
}

attotime hp85_state::time_to_video_mem_availability() const
{
	if (BIT(m_crt_ctl , CRT_CTL_WIPEOUT_BIT) || BIT(m_crt_ctl , CRT_CTL_POWERDN_BIT)) {
		// Blank video, immediate access
		return attotime::zero;
	} else if (m_screen->vblank()) {
		// Vertical blanking, immediate access
		return attotime::zero;
	} else {
		// In the active part, wait until vertical blanking
		return m_screen->time_until_vblank_start();
	}
}

void hp85_state::get_video_addr(uint16_t addr , uint16_t& byte_addr , bool& lsb_nibble)
{
	byte_addr = (addr / 2) & (VIDEO_MEM_SIZE - 1);
	lsb_nibble = BIT(addr , 0);
}

uint8_t hp85_state::video_mem_r(uint16_t addr , uint16_t addr_mask) const
{
	uint16_t byte_addr;
	bool lsb_nibble;

	get_video_addr(addr , byte_addr , lsb_nibble);

	byte_addr &= addr_mask;

	uint8_t res;

	if (lsb_nibble) {
		res = (m_video_mem[ byte_addr ] & 0x0f) << 4;
		byte_addr = (byte_addr + 1) & addr_mask;
		res |= (m_video_mem[ byte_addr ] & 0xf0) >> 4;
	} else {
		res = m_video_mem[ byte_addr ];
	}

	return res;
}

void hp85_state::video_mem_w(uint16_t addr , uint16_t addr_mask , uint8_t data)
{
	uint16_t byte_addr;
	bool lsb_nibble;

	get_video_addr(addr , byte_addr , lsb_nibble);

	byte_addr &= addr_mask;

	if (lsb_nibble) {
		m_video_mem[ byte_addr ] = (m_video_mem[ byte_addr ] & 0xf0) | (data >> 4);
		byte_addr = (byte_addr + 1) & addr_mask;
		m_video_mem[ byte_addr ] = (m_video_mem[ byte_addr ] & 0x0f) | (data << 4);
	} else {
		m_video_mem[ byte_addr ] = data;
	}
}

void hp85_state::video_mem_read()
{
	uint16_t mask;

	if (BIT(m_crt_ctl , CRT_CTL_GRAPHICS_BIT)) {
		mask = GRAPH_MEM_SIZE / 2 - 1;
	} else {
		mask = ALPHA_MEM_SIZE / 2 - 1;
	}
	m_crt_read_byte = video_mem_r(m_crt_bad , mask);
	m_crt_bad += 2;
	BIT_CLR(m_crt_ctl , CRT_CTL_RD_RQ_BIT);
	BIT_SET(m_crt_sts , CRT_STS_READY_BIT);
}

void hp85_state::video_mem_write()
{
	uint16_t mask;

	if (BIT(m_crt_ctl , CRT_CTL_GRAPHICS_BIT)) {
		mask = GRAPH_MEM_SIZE / 2 - 1;
	} else {
		mask = ALPHA_MEM_SIZE / 2 - 1;
	}
	video_mem_w(m_crt_bad , mask , m_crt_write_byte);
	m_crt_bad += 2;
}

void hp85_state::irq_w(unsigned n_irq , bool state)
{
	//LOG("irq_w %u=%d GIE=%d SRV=%03x ACK=%03x IE=%03x\n" , n_irq , state , m_global_int_en , m_int_serv , m_int_acked , m_int_en);
	if (state && !BIT(m_int_serv , n_irq)) {
		// Set service request
		BIT_SET(m_int_serv , n_irq);
		BIT_CLR(m_int_acked , n_irq);
	} else if (!state && BIT(m_int_serv , n_irq)) {
		// Clear service request
		BIT_CLR(m_int_serv , n_irq);
		BIT_CLR(m_int_acked , n_irq);
	}
	update_int_bits();
}

void hp85_state::irq_en_w(unsigned n_irq , bool state)
{
	COPY_BIT(state , m_int_en , n_irq);
	update_int_bits();
}

void hp85_state::update_int_bits()
{
	uint16_t irqs = m_int_en & m_int_serv;
	for (m_top_pending = 0; m_top_pending < IRQ_BIT_COUNT && !BIT(irqs , m_top_pending); m_top_pending++) {
	}
	update_irl();
}

void hp85_state::update_irl()
{
	//LOG("irl GIE=%d top=%u ACK=%03x\n" , m_global_int_en , m_top_pending , m_int_acked);
	m_cpu->set_input_line(0 , m_global_int_en && m_top_pending < IRQ_BIT_COUNT && !BIT(m_int_acked , m_top_pending));
}

uint8_t hp85_state::get_prt_font(uint8_t ch , unsigned col) const
{
	// Bit 7: pixel @ top
	// Bit 0: pixel @ bottom
	uint8_t column = m_prt_chargen[ (((unsigned)ch & 0x7f) << 3) | col ];
	if (BIT(ch , 7)) {
		// Underline
		BIT_SET(column , 0);
	}
	return column;
}

void hp85_state::prt_format_alpha(unsigned row , uint8_t *pixel_row) const
{
	memset(pixel_row , 0 , PRT_WIDTH);
	for (unsigned i = 0; i < m_prt_idx; i++) {
		for (unsigned j = 0; j < PRT_CELL_WIDTH; j++) {
			uint8_t pixel_col = get_prt_font(m_prt_buffer[ i ] , j);
			*pixel_row++ = BIT(pixel_col , 7 - row);
		}
	}
}

void hp85_state::prt_format_graphic(unsigned row , uint8_t *pixel_row) const
{
	memset(pixel_row , 0 , PRT_WIDTH);
	pixel_row += PRT_GRAPH_OFFSET;
	for (unsigned i = 0; i < PRT_GRAPH_WIDTH; i++) {
		*pixel_row++ = BIT(m_prt_buffer[ i ] , 7 - row);
	}
}

void hp85_state::prt_output_row(const uint8_t *pixel_row)
{
	for (unsigned i = 0; i < PRT_WIDTH; i++) {
		m_prt_graph_out->output(*pixel_row++ != 0 ? '*' : ' ');
	}
	m_prt_graph_out->output('\n');
}

void hp85_state::prt_do_printing()
{
	uint8_t pixel_row[ PRT_WIDTH ];
	for (unsigned row = 0; row < PRT_PH_HEIGHT; row++) {
		if (BIT(m_prtctl , PRTCTL_GRAPHIC_BIT)) {
			prt_format_graphic(row , pixel_row);
		} else {
			prt_format_alpha(row , pixel_row);
		}
		prt_output_row(pixel_row);
	}
	if (!BIT(m_prtctl , PRTCTL_GRAPHIC_BIT)) {
		// Dump the text line to alpha bitbanger
		for (unsigned i = 0; i < m_prt_idx; i++) {
			m_prt_alpha_out->output(m_prt_buffer[ i ]);
		}
		m_prt_alpha_out->output('\n');
		// Add 2 empty lines
		memset(pixel_row , 0 , PRT_WIDTH);
		for (unsigned i = 0; i < (PRT_ALPHA_HEIGHT - PRT_PH_HEIGHT); i++) {
			prt_output_row(pixel_row);
		}
	}
	// Start busy timer
	BIT_CLR(m_prtsts , PRTSTS_PRTRDY_BIT);
	m_prt_busy_timer->adjust(attotime::from_msec(PRT_BUSY_MSEC));
}

#define IOP_MASK(x) BIT_MASK<ioport_value>((x))

static INPUT_PORTS_START(hp85)
	// Keyboard is arranged in a matrix of 10 rows and 8 columns. In addition there are 3 keys with
	// dedicated input lines: SHIFT, SHIFT LOCK & CONTROL.
	// A key on row "r"=[0..9] and column "c"=[0..7] is mapped to bit "b" of KEY"n" input, where
	// n = r / 4
	// b = (r % 4) * 8 + c
	PORT_START("KEY0")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_NAME("Down AUTO") // 0,0: Down / Auto
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_NAME("Up Home")       // 0,1: Up / Home
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_NAME("k4 k8")         // 0,2: k4 / k8
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_NAME("k3 k7")         // 0,3: k3 / k7
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_NAME("k2 k6")         // 0,4: k2 / k6
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_NAME("k1 k5")         // 0,5: k1 / k5
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("LABEL KEY")                                                        // 0,6: LABEL KEY
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                 // 0,7: N/U
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')                            // 1,0: 8
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')                            // 1,1: 7
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')                           // 1,2: 6
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')                           // 1,3: 5
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')                           // 1,4: 4
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')                           // 1,5: 3
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')                           // 1,6: 2
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')                           // 1,7: 1
	PORT_BIT(IOP_MASK(16) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')                           // 2,0: I
	PORT_BIT(IOP_MASK(17) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')                           // 2,1: U
	PORT_BIT(IOP_MASK(18) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')                           // 2,2: Y
	PORT_BIT(IOP_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')                           // 2,3: T
	PORT_BIT(IOP_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')                           // 2,4: R
	PORT_BIT(IOP_MASK(21) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')                           // 2,5: E
	PORT_BIT(IOP_MASK(22) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')                           // 2,6: W
	PORT_BIT(IOP_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')                           // 2,7: Q
	PORT_BIT(IOP_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')                           // 3,0: K
	PORT_BIT(IOP_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')                           // 3,1: J
	PORT_BIT(IOP_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')                           // 3,2: H
	PORT_BIT(IOP_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')                           // 3,3: G
	PORT_BIT(IOP_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')                           // 3,4: F
	PORT_BIT(IOP_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')                           // 3,5: D
	PORT_BIT(IOP_MASK(30) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')                           // 3,6: S
	PORT_BIT(IOP_MASK(31) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')                           // 3,7: A

	PORT_START("KEY1")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')                            // 4,0: M
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')                            // 4,1: N
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')                            // 4,2: B
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')                            // 4,3: V
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')                            // 4,4: C
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')                            // 4,5: X
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')                            // 4,6: Z
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')                                       // 4,7: Space
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')                        // 5,0: ,
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')                         // 5,1: .
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')                       // 5,2: / ?
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("PAUSE STEP")                                                      // 5,3: PAUSE / STEP
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("RUN")                                                             // 5,4: RUN
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD)) PORT_NAME("KP +") // 5,5: KP +
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_NAME("KP -")   // 5,6: KP -
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK)) PORT_NAME("KP *") // 5,7: KP * (not sure)
	PORT_BIT(IOP_MASK(16) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')                           // 6,0: L
	PORT_BIT(IOP_MASK(17) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')                       // 6,1: ;
	PORT_BIT(IOP_MASK(18) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')                      // 6,2: ' "
	PORT_BIT(IOP_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("END LINE")                 // 6,3: END LINE
	PORT_BIT(IOP_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("LIST P LST")                                                      // 6,4: LIST / P LST
	PORT_BIT(IOP_MASK(21) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                // 6,5: N/U
	PORT_BIT(IOP_MASK(22) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                // 6,6: N/U
	PORT_BIT(IOP_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) PORT_NAME("KP /")   // 6,7: KP /
	PORT_BIT(IOP_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')                           // 7,0: O
	PORT_BIT(IOP_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')                           // 7,1: P
	PORT_BIT(IOP_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('(') PORT_CHAR('[')                   // 7,2: ( [
	PORT_BIT(IOP_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(')') PORT_CHAR(']')                  // 7,3: ) ]
	PORT_BIT(IOP_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("CONT SCRATCH")                                                    // 7,4: CONT / SCRATCH
	PORT_BIT(IOP_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("-LINE CLEAR")                                                     // 7,5: -LINE / CLEAR
	PORT_BIT(IOP_MASK(30) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME(") INIT")                                                          // 7,6: ) INIT
	PORT_BIT(IOP_MASK(31) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                // 7,7: N/U

	PORT_START("KEY2")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')                            // 8,0: 9
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')                            // 8,1: 0
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')                        // 8,2: - _
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')                       // 8,3: = +
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('\\') PORT_CHAR('|')                       // 8,4: \ |
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)                                     // 8,5: BS
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("( RESET")                                                          // 8,6: ( RESET
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("^ RESLT")                                                          // 8,7: ^ / RESLT
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))    PORT_NAME("Left GRAPH") // 9,0: LEFT / GRAPH
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME("Right COPY")  // 9,1: RIGHT / COPY
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT) PORT_NAME("RPL INS")                               // 9,2: RPL / INS
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_NAME("-CHAR DEL")                                // 9,3: -CHAR / DEL
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("ROLL")                                    // 9,4: ROLL
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("LOAD REW")                                                        // 9,5: LOAD / REW
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("STORE TEST")                                                      // 9,6: STORE / TEST
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("PAPER ADVANCE")                                                   // 9,7: PAPER ADVANCE

	PORT_START("MODKEYS")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)                // Shift
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_NAME("Shift lock")   // Shift lock
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)              // Control

INPUT_PORTS_END

void hp85_state::cpu_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x7fff).m(m_rombank, FUNC(address_map_bank_device::amap8));
	map(0x8000, 0xbfff).ram();
	map(0xff00, 0xff00).w(FUNC(hp85_state::ginten_w));
	map(0xff01, 0xff01).w(FUNC(hp85_state::gintdis_w));
	map(0xff02, 0xff02).rw(FUNC(hp85_state::keysts_r), FUNC(hp85_state::keysts_w));
	map(0xff03, 0xff03).rw(FUNC(hp85_state::keycod_r), FUNC(hp85_state::keycod_w));
	map(0xff04, 0xff07).rw(FUNC(hp85_state::crtc_r), FUNC(hp85_state::crtc_w));
	map(0xff08, 0xff09).rw("tape", FUNC(hp_1ma6_device::reg_r), FUNC(hp_1ma6_device::reg_w));
	map(0xff0a, 0xff0a).rw(FUNC(hp85_state::clksts_r), FUNC(hp85_state::clksts_w));
	map(0xff0b, 0xff0b).rw(FUNC(hp85_state::clkdat_r), FUNC(hp85_state::clkdat_w));
	map(0xff0c, 0xff0c).w(FUNC(hp85_state::prtlen_w));
	map(0xff0d, 0xff0d).rw(FUNC(hp85_state::prchar_r), FUNC(hp85_state::prchar_w));
	map(0xff0e, 0xff0e).rw(FUNC(hp85_state::prtsts_r), FUNC(hp85_state::prtctl_w));
	map(0xff0f, 0xff0f).w(FUNC(hp85_state::prtdat_w));
	map(0xff18, 0xff18).w(FUNC(hp85_state::rselec_w));
	map(0xff40, 0xff40).rw(FUNC(hp85_state::intrsc_r), FUNC(hp85_state::intrsc_w));
}

void hp85_state::rombank_mem_map(address_map &map)
{
	map.unmap_value_high();
	// ROM in bank 0 is always present (it's part of system ROMs)
	map(0x0000, 0x1fff).rom();
}

void hp85_state::hp85(machine_config &config)
{
	HP_CAPRICORN(config, m_cpu, MASTER_CLOCK / 16);
	m_cpu->set_addrmap(AS_PROGRAM, &hp85_state::cpu_mem_map);
	m_cpu->set_irq_acknowledge_callback(FUNC(hp85_state::irq_callback));

	ADDRESS_MAP_BANK(config, "rombank").set_map(&hp85_state::rombank_mem_map).set_options(ENDIANNESS_LITTLE, 8, 21, HP80_OPTROM_SIZE);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK / 2 , 312 , 0 , 256 , 256 , 0 , 192);
	m_screen->set_screen_update(FUNC(hp85_state::screen_update));
	m_screen->screen_vblank().set(FUNC(hp85_state::vblank_w));
	PALETTE(config, m_palette, palette_device::MONOCHROME);
	TIMER(config, m_vm_timer).configure_generic(FUNC(hp85_state::vm_timer));

	// No idea at all about the actual keyboard scan frequency
	TIMER(config, "kb_timer").configure_periodic(FUNC(hp85_state::kb_scan), attotime::from_hz(100));

	// Hw timers are updated at 1 kHz rate
	TIMER(config, "hw_timer").configure_periodic(FUNC(hp85_state::timer_update), attotime::from_hz(1000));
	TIMER(config, m_clk_busy_timer).configure_generic(FUNC(hp85_state::clk_busy_timer));
	TIMER(config, m_prt_busy_timer).configure_generic(FUNC(hp85_state::prt_busy_timer));

	// Beeper
	SPEAKER(config, "mono").front_center();
	DAC_1BIT(config, m_dac , 0).add_route(ALL_OUTPUTS, "mono", 0.5, AUTO_ALLOC_INPUT, 0);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	BEEP(config, m_beep, MASTER_CLOCK / 8192).add_route(ALL_OUTPUTS, "mono", 0.5, AUTO_ALLOC_INPUT, 0);

	// Tape drive
	HP_1MA6(config, "tape", 0);

	// Optional ROMs
	HP80_OPTROM_SLOT(config, m_rom_drawers[0]);
	HP80_OPTROM_SLOT(config, m_rom_drawers[1]);
	HP80_OPTROM_SLOT(config, m_rom_drawers[2]);
	HP80_OPTROM_SLOT(config, m_rom_drawers[3]);
	HP80_OPTROM_SLOT(config, m_rom_drawers[4]);
	HP80_OPTROM_SLOT(config, m_rom_drawers[5]);

	// I/O slots
	HP80_IO_SLOT(config, m_io_slots[0]).set_slot_no(0);
	m_io_slots[0]->irl_cb().set(FUNC(hp85_state::irl_w));
	m_io_slots[0]->halt_cb().set(FUNC(hp85_state::halt_w));
	HP80_IO_SLOT(config, m_io_slots[1]).set_slot_no(1);
	m_io_slots[1]->irl_cb().set(FUNC(hp85_state::irl_w));
	m_io_slots[1]->halt_cb().set(FUNC(hp85_state::halt_w));
	HP80_IO_SLOT(config, m_io_slots[2]).set_slot_no(2);
	m_io_slots[2]->irl_cb().set(FUNC(hp85_state::irl_w));
	m_io_slots[2]->halt_cb().set(FUNC(hp85_state::halt_w));
	HP80_IO_SLOT(config, m_io_slots[3]).set_slot_no(3);
	m_io_slots[3]->irl_cb().set(FUNC(hp85_state::irl_w));
	m_io_slots[3]->halt_cb().set(FUNC(hp85_state::halt_w));

	// Printer output
	BITBANGER(config, m_prt_graph_out, 0);
	BITBANGER(config, m_prt_alpha_out, 0);

	SOFTWARE_LIST(config, "optrom_list").set_original("hp85_rom");
}

ROM_START(hp85)
	ROM_REGION(0x6000 , "cpu" , 0)
	ROM_LOAD("romsys1.bin" , 0x0000 , 0x2000 , CRC(7724b1e9) SHA1(7836195389de2ac0eab7199835f5dc8f7dc41729))
	ROM_LOAD("romsys2.bin" , 0x2000 , 0x2000 , CRC(50a85263) SHA1(3cf1d08749103ee245d572550ba1b053ffc7ef57))
	ROM_LOAD("romsys3.bin" , 0x4000 , 0x2000 , CRC(0df385f0) SHA1(4c5ce5afd28f6d776f16cabbbbcc09769ff306b7))

	ROM_REGION(0x2000 , "rombank" , 0)
	ROM_LOAD("rom000.bin" , 0 , 0x2000 , CRC(e13b8ae3) SHA1(2374618d25d1a000ddb534ae4f55ebd98ce0fff3))

	ROM_REGION(0x400 , "chargen" , 0)
	ROM_LOAD("chrgen.bin" , 0 , 0x400 , CRC(9c402544) SHA1(32634fc73c1544aeeefda62ebb10349c5b40729f))

	ROM_REGION(0x400 , "prt_chargen" , 0)
	ROM_LOAD("prt_chrgen.bin" , 0 , 0x400 , CRC(abeaba27) SHA1(fbf6bdd5d96df6aa5963f8cdfdeb180402b1cc85))
ROM_END

COMP( 1980, hp85, 0, 0, hp85, hp85, hp85_state, empty_init, "HP", "HP 85", 0)

// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Atari System IV hardware
    (Imagination Technology / Atari, Inc., 1984)

    Original preliminary driver by Phil Bennett.
    Video system rewritten from the "System IV Specification and User Guide" by Ken Van Mersbergen (Dutchman2000)
    (Imagination Technology, revision 8403.22, with UCODE.RND Graphic
    Controller instruction summaries 8404.03 - 8404.18).

    Games supported:
        * The Last Starfighter (prototype) - 1984 development programs
          - Cave sequence demo
          - Tube fly-through test
          - TESTAST / TESTOBJ object test programs
        * Air Race (prototype, 1985) - track 1 demo

    Hardware overview
    -----------------
    Processor Module (PM):
        CPU board    - 68000 @ 8MHz, Queue Controller, Queue Memory, timebase
        Update board - microcoded 16MHz Graphic Controller (GC), display
        2-4 Buffers  - W, X, Y, Z: 512x512x(4..8) Screen RAM each, plus a
                       1Kx12 Translation RAM (offset/expansion/replacement)
    Application Module (AM):
        Memory board - program PROM/RAM, Image Memory PROMs (up to 1MB)
        I/O board    - controls, sound, Mathbox (TMS32010 + shared RAM)

    68000 memory map (from the spec, plus the Last Starfighter I/O board)
        000000-0FFFFF  AM program memory (the 1984 programs ran from RAM,
                       downloaded by the VAX through the Debug Module)
        580000         Joystick / buttons
        588000         Analog flight yoke (hi byte yaw, lo byte pitch)
        598000         Sound board
        7C0000-7C4FFF  Mathbox #2 shared RAM (Air Race)
        7C6000         Mathbox #2 control/status
        7F0000-7F4FFF  Mathbox shared RAM
        7F6000         Mathbox control/status
        800000-DFFFFF  Screen Space (4096 x 1536, Screen Memory is the
                       512x512 area at X=400-5FF, Y=200-3FF)
        F00000-F07FFF  Queue Memory (up to 16K words; Air Race uses $F00100)
        FF8000-FF803F  Queue Registers (synchronous load)
        FF8040-FF8045  Control Registers (MCR, QLR, QAR)
        FF8400-FF843F  Queue Registers (asynchronous load)
        FF8800         DHR - display hold register (read only)
        FF8840         DLR - display load register (sample and read)

    To do:
        * Queue Processor timing: queued instructions execute instantly
          (except WVR, which waits for vertical blank).  Interrupt levels
          1/2 follow Air Race's usage (QLR underflow / completion jump).
        * Replacement (display-time read-modify-write) is not emulated.
        * Buffer Priority Register thresholds (BPR) - marked "not currently
          implemented" in the spec; a pixel is active when non-zero.
        * LDSR status register bit assignments are a guess.
        * Image Memory (HUD / alphanumerics) for Last Starfighter is missing:
          HDUP.ASM draws the cockpit frame, scanner globe and temperature
          gauges from bank 0 and two fonts ([morris.lsf]ALPHANUM.ASM) from
          bank 1, none of which survive.
        * Sound board.

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms320c1x/tms320c1x.h"
#include "machine/watchdog.h"
#include "screen.h"

#define LOG_GPU     (1U << 1)
#define LOG_UNKNOWN (1U << 2)

//#define VERBOSE (LOG_GPU | LOG_UNKNOWN)
#include "logmacro.h"


namespace {

class atarisy4_state : public driver_device
{
public:
	atarisy4_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dsp0(*this, "dsp0"),
		m_dsp0_bank1(*this, "dsp0_bank1"),
		m_screen(*this, "screen"),
		m_watchdog(*this, "watchdog"),
		m_m68k_ram(*this, "m68k_ram"),
		m_prog_rom(*this, "program"),
		m_dsp_rom(*this, "dsp"),
		m_image_rom(*this, "gfx"),
		m_stick(*this, "STICK%c", 'X'),
		m_config(*this, "CONFIG")
	{
	}

	void atarisy4(machine_config &config) ATTR_COLD;

	void init_laststar() ATTR_COLD;

protected:
	// Buffer Control Register bits
	static constexpr uint8_t BCR_UD  = 0x80;   // 1 = display, 0 = update
	static constexpr uint8_t BCR_LR  = 0x40;   // horizontal direction reversed
	static constexpr uint8_t BCR_SRW = 0x20;   // Screen RAM write enable
	static constexpr uint8_t BCR_TRW = 0x10;   // Translation RAM write enable

	// Master Control Register bits
	static constexpr uint8_t MCR_QP  = 0x80;   // Queue Processor enable
	static constexpr uint8_t MCR_WP  = 0x40;   // Screen Space (wrapped) write enable
	static constexpr uint8_t MCR_IE6 = 0x08;   // level 6 interrupt enable

	// Screen Space origin of Screen Memory
	static constexpr int SM_X0 = 0x400;
	static constexpr int SM_Y0 = 0x200;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank_w(int state);

	// 68000 side
	void main_map(address_map &map) ATTR_COLD;
	uint16_t screen_space_r(offs_t offset, uint16_t mem_mask);
	void screen_space_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t pm_reg_r(offs_t offset);
	void pm_reg_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t analog_r();

	// Mathbox
	uint16_t m68k_shared_0_r(offs_t offset);
	void m68k_shared_0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t m68k_shared_1_r(offs_t offset);
	void m68k_shared_1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp0_status_r();
	void dsp0_control_w(uint16_t data);
	int dsp0_bio_r();
	void dsp0_bank_w(uint16_t data);
	void dsp0_map(address_map &map) ATTR_COLD;
	void dsp0_io_map(address_map &map) ATTR_COLD;

	// Graphic Controller
	void queue_register_w(unsigned reg, uint16_t data);
	uint16_t queue_ram_r(offs_t offset);
	void queue_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void qp_run();
	void update_irqs();
	void execute_gc_command(uint8_t cmd);
	bool buffer_updating(int buf) const { return !(m_bcr[buf] & BCR_UD); }
	void write_update_pixel(int x, int y, uint8_t data, bool use_offset);
	uint8_t read_update_pixel(int x, int y, bool use_offset) const;
	void plot_screen_space(int sx, int sy, uint8_t data, bool clip);
	void image_transfer(bool clip);
	void draw_line(int x0, int y0, int x1, int y1, uint8_t data);
	void fill_polygon(uint8_t data);
	void fill_translation_ram(unsigned base, int start, int size, int data, int step, int limit);
	static unsigned expansion_address(uint8_t data) { return (data & 0x80) ? (0x380 | (data & 0x7f)) : (0x180 | data); }
	int display_priority(int *order) const;


	required_device<m68000_device> m_maincpu;
	required_device<tms320c10_device> m_dsp0;
	required_memory_bank m_dsp0_bank1;
	required_device<screen_device> m_screen;
	required_device<watchdog_timer_device> m_watchdog;

	required_shared_ptr<uint16_t> m_m68k_ram;
	required_region_ptr<uint16_t> m_prog_rom;
	required_region_ptr<uint8_t> m_dsp_rom;
	optional_region_ptr<uint8_t> m_image_rom;

	required_ioport_array<2> m_stick;
	required_ioport m_config;

	uint16_t m_dsp_bank[2]{};
	uint8_t m_csr[2]{};
	std::unique_ptr<uint16_t[]> m_shared_ram[2];

	// Buffers
	std::unique_ptr<uint8_t[]> m_screen_ram[4];  // 512 x 512 x 8 bits each
	std::unique_ptr<uint16_t[]> m_trans_ram[4];  // 1K x 12 bits each
	std::unique_ptr<uint8_t[]> m_color_ram[3];   // R, G, B: 4 tables x 256

	// PM registers
	uint16_t m_gr[8]{};
	uint8_t m_bcr[4]{};
	uint8_t m_psr[4]{};
	uint8_t m_bpr[4]{};
	uint8_t m_dpr = 0;
	uint8_t m_ctr = 0;
	uint8_t m_ifr = 0;
	uint8_t m_ecr = 0;
	uint16_t m_far = 0;
	uint8_t m_mcr = 0;
	uint16_t m_qlr = 0;
	uint16_t m_qar = 0;
	uint16_t m_shadow[0x23]{};

	// Queue Memory (16K words) and Queue Processor
	static constexpr unsigned QM_WORDS = 0x4000;
	static constexpr uint8_t QTAG_RAW = 0xff;
	std::unique_ptr<uint16_t[]> m_queue_ram;
	std::unique_ptr<uint8_t[]> m_queue_tag;     // register a hardware-queued word came from
	bool m_qp_wait_vblank = false;
	bool m_qp_halted = false;
	bool m_irq1 = false;                        // QLR underflow
	bool m_irq2 = false;                        // Queue Processor reached a completion jump
	// Graphic Controller state
	uint8_t m_idr = 0;          // image merge data
	uint8_t m_icd = 0;          // image compare data (partial transfer)
	bool m_expansion = false;
	bool m_offset = false;
	bool m_transpose = false;
	int m_clip_l = 0, m_clip_t = 0, m_clip_r = 0, m_clip_b = 0;
	int m_cur_x = 0, m_cur_y = 0;
	static constexpr int MAX_POLY_POINTS = 256;
	int m_poly_x[MAX_POLY_POINTS]{};
	int m_poly_y[MAX_POLY_POINTS]{};
	int m_poly_count = 0;
};


class airrace_state : public atarisy4_state
{
public:
	airrace_state(const machine_config &mconfig, device_type type, const char *tag) :
		atarisy4_state(mconfig, type, tag),
		m_dsp1(*this, "dsp1"),
		m_dsp1_bank1(*this, "dsp1_bank1")
	{
	}

	void init_airrace() ATTR_COLD;

	void airrace(machine_config &config) ATTR_COLD;

protected:
	uint16_t dsp1_status_r();
	void dsp1_control_w(uint16_t data);
	int dsp1_bio_r();
	void dsp1_bank_w(uint16_t data);

	virtual void machine_reset() override ATTR_COLD;

	void airrace_map(address_map &map) ATTR_COLD;
	void dsp1_map(address_map &map) ATTR_COLD;
	void dsp1_io_map(address_map &map) ATTR_COLD;

private:
	required_device<tms320c10_device> m_dsp1;
	required_memory_bank m_dsp1_bank1;
};



/*************************************
 *
 *  Video start / display
 *
 *************************************/

void atarisy4_state::video_start()
{
	for (int i = 0; i < 4; i++)
	{
		m_screen_ram[i] = make_unique_clear<uint8_t[]>(512 * 512);
		m_trans_ram[i] = make_unique_clear<uint16_t[]>(1024);
		save_pointer(NAME(m_screen_ram[i]), 512 * 512, i);
		save_pointer(NAME(m_trans_ram[i]), 1024, i);
	}
	for (int i = 0; i < 3; i++)
	{
		m_color_ram[i] = make_unique_clear<uint8_t[]>(1024);
		save_pointer(NAME(m_color_ram[i]), 1024, i);
	}

	m_queue_ram = make_unique_clear<uint16_t[]>(QM_WORDS);
	m_queue_tag = std::make_unique<uint8_t[]>(QM_WORDS);
	std::fill_n(m_queue_tag.get(), QM_WORDS, QTAG_RAW);
	save_pointer(NAME(m_queue_ram), QM_WORDS);
	save_pointer(NAME(m_queue_tag), QM_WORDS);
	save_item(NAME(m_qp_wait_vblank));
	save_item(NAME(m_qp_halted));
	save_item(NAME(m_irq1));
	save_item(NAME(m_irq2));

	save_item(NAME(m_gr));
	save_item(NAME(m_bcr));
	save_item(NAME(m_psr));
	save_item(NAME(m_bpr));
	save_item(NAME(m_dpr));
	save_item(NAME(m_ctr));
	save_item(NAME(m_ifr));
	save_item(NAME(m_ecr));
	save_item(NAME(m_far));
	save_item(NAME(m_mcr));
	save_item(NAME(m_qlr));
	save_item(NAME(m_qar));
	save_item(NAME(m_shadow));
	save_item(NAME(m_idr));
	save_item(NAME(m_icd));
	save_item(NAME(m_expansion));
	save_item(NAME(m_offset));
	save_item(NAME(m_transpose));
	save_item(NAME(m_clip_l));
	save_item(NAME(m_clip_t));
	save_item(NAME(m_clip_r));
	save_item(NAME(m_clip_b));
	save_item(NAME(m_cur_x));
	save_item(NAME(m_cur_y));
	save_item(NAME(m_poly_x));
	save_item(NAME(m_poly_y));
	save_item(NAME(m_poly_count));
}

/*
    Decode the 6-bit Display Priority Register priority select code
    (spec Appendix B) into a list of Buffers, highest priority first.
    Returns the number of Buffers listed.
*/
int atarisy4_state::display_priority(int *order) const
{
	const int code = m_dpr & 0x3f;
	const int first = code >> 4;
	const int sub = code & 0x0f;

	int rest[3], n = 0;
	for (int b = 0; b < 4; b++)
		if (b != first)
			rest[n++] = b;

	order[0] = first;

	if (sub == 0)
		return 1;

	if (sub <= 3)
	{
		order[1] = rest[sub - 1];
		return 2;
	}

	const int idx = (sub <= 9) ? (sub - 4) : (sub - 10);
	const int second = idx >> 1;
	const int third = idx & 1;

	order[1] = rest[second];
	int rest2[2], m = 0;
	for (int i = 0; i < 3; i++)
		if (i != second)
			rest2[m++] = rest[i];
	order[2] = rest2[third];

	if (sub <= 9)
		return 3;

	order[3] = rest2[third ^ 1];
	return 4;
}

uint32_t atarisy4_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int order[4];
	const int count = display_priority(order);

	// Only Buffers assigned to display take part.
	// Air Race (1985) drives the display through "channels": the displayed
	// buffer gets BCR=$84 (channel 1 in bits 3-2), the update buffer $30, and
	// DPR stays at $10.  When the buffer named by DPR isn't displaying, use a
	// displaying buffer whose BCR channel field matches instead.
	int active[4], nactive = 0;
	for (int i = 0; i < count; i++)
	{
		const int want = order[i];
		int sel = -1;
		if (m_bcr[want] & BCR_UD)
			sel = want;
		else
			for (int b = 0; b < 4 && sel < 0; b++)
				if ((m_bcr[b] & BCR_UD) && ((m_bcr[b] >> 2) & 3) == want)
					sel = b;
		if (sel >= 0 && std::find(active, active + nactive, sel) == active + nactive)
			active[nactive++] = sel;
	}

	if (nactive == 0)
	{
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint32_t *dest = &bitmap.pix(y);

		// Per-line offsets (always applied during display)
		int src_y[4], x_off[4];
		for (int i = 0; i < nactive; i++)
		{
			const int b = active[i];
			src_y[i] = (y + (m_trans_ram[b][y] & 0x1ff)) & 0x1ff;
			x_off[i] = m_trans_ram[b][0x200 + y] & 0x1ff;
		}

		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			int sel = 0;
			uint8_t pix = 0;
			bool found = false;

			for (int i = 0; i < nactive; i++)
			{
				const int b = active[i];
				int sx = x;
				if (m_bcr[b] & BCR_LR)
					sx = 511 - sx;
				sx = (sx + x_off[i]) & 0x1ff;
				const uint8_t p = m_screen_ram[b][(src_y[i] << 9) | sx];
				if (p != 0)
				{
					sel = b;
					pix = p;
					found = true;
					break;
				}
			}

			if (!found)
			{
				// All active buffers zero: highest priority buffer is output
				sel = active[0];
				pix = 0;
			}

			const unsigned addr = (((m_ctr >> (sel * 2)) & 3) << 8) | pix;
			dest[x] = rgb_t(m_color_ram[0][addr], m_color_ram[1][addr], m_color_ram[2][addr]);
		}
	}

	return 0;
}

void atarisy4_state::vblank_w(int state)
{
	// The Debug Module development setup could hold off the watchdog
	if (state && !BIT(m_config->read(), 0))
		m_watchdog->watchdog_reset();

	if (state && (m_mcr & MCR_IE6))
		m_maincpu->set_input_line(6, ASSERT_LINE);

	if (state && m_qp_wait_vblank)
	{
		m_qp_wait_vblank = false;
		qp_run();
	}
}


/*************************************
 *
 *  Screen Memory access helpers
 *
 *************************************/

// x, y are Screen Memory coordinates (0-511)
void atarisy4_state::write_update_pixel(int x, int y, uint8_t data, bool use_offset)
{
	for (int b = 0; b < 4; b++)
	{
		if (!buffer_updating(b) || !(m_bcr[b] & BCR_SRW))
			continue;

		int px = x, py = y;
		if (use_offset && y < 384)
		{
			px = (px + (m_trans_ram[b][0x200 + y] & 0x1ff)) & 0x1ff;
			py = (py + (m_trans_ram[b][y] & 0x1ff)) & 0x1ff;
		}

		uint8_t &dst = m_screen_ram[b][(py << 9) | px];
		const uint8_t protect = m_psr[b];   // active-low plane enables
		dst = (dst & protect) | (data & ~protect);
	}
}

uint8_t atarisy4_state::read_update_pixel(int x, int y, bool use_offset) const
{
	const int b = (m_mcr >> 4) & 3;
	int px = x & 0x1ff, py = y & 0x1ff;
	if (use_offset && py < 384)
	{
		px = (px + (m_trans_ram[b][0x200 + py] & 0x1ff)) & 0x1ff;
		py = (py + (m_trans_ram[b][py] & 0x1ff)) & 0x1ff;
	}
	return m_screen_ram[b][(py << 9) | px];
}

// sx, sy in Screen Space; writes outside Screen Memory do nothing
void atarisy4_state::plot_screen_space(int sx, int sy, uint8_t data, bool clip)
{
	if (clip && (sx < m_clip_l || sx > m_clip_r || sy < m_clip_t || sy > m_clip_b))
		return;

	const int x = sx - SM_X0;
	const int y = sy - SM_Y0;
	if (x < 0 || x > 511 || y < 0 || y > 511)
		return;

	write_update_pixel(x, y, data, m_offset);
}


/*************************************
 *
 *  68000 Screen Space (bit-mapped) access
 *
 *  A1-A11 + UDS/LDS = X (0-FFF), A12-A22 = Y.
 *
 *************************************/

uint16_t atarisy4_state::screen_space_r(offs_t offset, uint16_t mem_mask)
{
	const int x = ((offset << 1) & 0xffe) - SM_X0;
	const int y = (offset >> 11) - SM_Y0;

	// Outside Screen Memory reads wrap modulo 512 on each axis
	const uint8_t hi = read_update_pixel(x, y, m_offset);
	const uint8_t lo = read_update_pixel(x + 1, y, m_offset);
	return (hi << 8) | lo;
}

void atarisy4_state::screen_space_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int x = ((offset << 1) & 0xffe) - SM_X0;
	int y = (offset >> 11) - SM_Y0;

	const bool inside = x >= 0 && x <= 510 && y >= 0 && y <= 511;
	if (!inside)
	{
		if (!(m_mcr & MCR_WP))
			return;
		x &= 0x1ff;
		y &= 0x1ff;
	}

	if (ACCESSING_BITS_8_15)
		write_update_pixel(x, y, data >> 8, m_offset);
	if (ACCESSING_BITS_0_7)
		write_update_pixel((x + 1) & 0x1ff, y, data & 0xff, m_offset);
}


/*************************************
 *
 *  PM I/O registers (FF8000-FF8FFF)
 *
 *************************************/

uint16_t atarisy4_state::pm_reg_r(offs_t offset)
{
	const offs_t reg = offset & 0x3ff;

	if (offset == 0x400 || offset == 0x420)
	{
		// DHR / DLR: current display address (line count)
		return m_screen->vpos() & 0x0fff;
	}

	if ((reg & 0x1ff) < 0x23)
	{
		// Shadow read of Queue/Control Registers
		return m_shadow[reg & 0x1ff];
	}

	LOGMASKED(LOG_UNKNOWN, "PM register read %06x\n", 0xff8000 + offset * 2);
	return 0;
}

void atarisy4_state::pm_reg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// FF8000 (synchronous) and FF8400 (asynchronous) are equivalent here
	const offs_t reg = offset & 0x1ff;

	if (offset >= 0x400)
	{
		LOGMASKED(LOG_UNKNOWN, "PM status register write %06x = %04x\n", 0xff8000 + offset * 2, data);
		return;
	}

	if (reg < 0x23)
	{
		COMBINE_DATA(&m_shadow[reg]);
		data = m_shadow[reg];
	}

	if (reg < 0x19)
	{
		if (m_mcr & MCR_QP)
		{
			// Queue Register Deferred: assemble opcode + data into Queue Memory at QAR
			uint16_t word;
			if (reg < 7)
				word = 0x8000 | (reg << 12) | (data & 0x0fff);
			else if (reg == 0x18)
				word = data & 0x7fff;                        // FAR: queued jump
			else
				word = 0xf000 | ((reg - 8) << 8) | (data & 0xff);  // tag disambiguates GR7

			const unsigned addr = (m_qar >> 1) & (QM_WORDS - 1);
			m_queue_ram[addr] = word;
			m_queue_tag[addr] = reg;
			m_qar = (m_qar + 2) & 0x7ffe;

			// QLR counts queued loads; underflow requests level 1 (Air Race QBUFFULL)
			if ((m_qlr & 0x7fff) == 0)
				m_irq1 = true;
			m_qlr = (m_qlr & 0x8000) | ((m_qlr - 1) & 0x7fff);
			update_irqs();
		}
		else
		{
			queue_register_w(reg, data);
		}
		return;
	}

	switch (reg)
	{
		case 0x20: // MCR
		{
			const uint8_t old = m_mcr;
			m_mcr = data & 0xff;
			if (!(m_mcr & MCR_IE6))
				m_maincpu->set_input_line(6, CLEAR_LINE);
			if (!(m_mcr & MCR_QP))
				m_irq2 = false;     // DPDONE turns the queue off to acknowledge
			update_irqs();
			if ((m_mcr & MCR_QP) && !(old & MCR_QP))
			{
				m_qp_halted = false;
				qp_run();
			}
			break;
		}

		case 0x21: // QLR
			m_qlr = data;
			m_irq1 = false;
			update_irqs();
			break;

		case 0x22: // QAR
			m_qar = data & 0x7ffe;
			break;

		default:
			LOGMASKED(LOG_UNKNOWN, "PM register write %06x = %04x\n", 0xff8000 + offset * 2, data);
			break;
	}
}

uint16_t atarisy4_state::queue_ram_r(offs_t offset)
{
	return m_queue_ram[offset & (QM_WORDS - 1)];
}

void atarisy4_state::queue_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset &= QM_WORDS - 1;
	COMBINE_DATA(&m_queue_ram[offset]);
	m_queue_tag[offset] = QTAG_RAW;

	// The CPU may be patching the word the Queue Processor is parked on
	if ((m_mcr & MCR_QP) && m_qp_halted && offset == ((m_far >> 1) & (QM_WORDS - 1)))
	{
		m_qp_halted = false;
		qp_run();
	}
}

void atarisy4_state::update_irqs()
{
	// Air Race: level 1 = QLR underflow (INTL1/QBUFFULL), level 2 = QP done (INTL2/DPDONE)
	m_maincpu->set_input_line(1, (m_irq1 && BIT(m_mcr, 1)) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(2, (m_irq2 && BIT(m_mcr, 2)) ? ASSERT_LINE : CLEAR_LINE);
}

/*
    Queue Processor.  Fetches words from Queue Memory at FAR:
        0aaa aaaa aaaa aaac   jump to byte address a (c = completion/interrupt flag)
        1rrr dddd dddd dddd   load GR0-GR6 with 12-bit data
        1111 rrrr dddd dddd   load BCRW..ECR (F0-FF) with 8-bit data
    A jump to itself parks the processor (Air Race QIDLE); a jump with the c bit
    set raises the completion interrupt (Air Race QDONE).  ECR=WVR (0B) stalls
    until the next vertical blank.  Execution is otherwise instantaneous.
*/
void atarisy4_state::qp_run()
{
	int budget = 0x20000;

	while ((m_mcr & MCR_QP) && !m_qp_halted && !m_qp_wait_vblank && budget-- > 0)
	{
		const unsigned addr = (m_far >> 1) & (QM_WORDS - 1);
		const uint16_t word = m_queue_ram[addr];
		const uint8_t tag = m_queue_tag[addr];

		unsigned reg;
		uint16_t data;

		if (tag != QTAG_RAW)
		{
			reg = tag;
			data = (reg < 7) ? (word & 0x0fff) : (reg == 0x18) ? (word & 0x7fff) : (word & 0xff);
		}
		else if (!BIT(word, 15))
		{
			reg = 0x18;
			data = word;
		}
		else if ((word & 0xf000) != 0xf000)
		{
			reg = (word >> 12) & 7;
			data = word & 0x0fff;
		}
		else
		{
			reg = 8 + ((word >> 8) & 0x0f);
			data = word & 0xff;
		}

		m_far = (m_far + 2) & 0x7ffe;

		if (reg == 0x18)
		{
			const uint16_t target = data & 0x7ffe;
			if (BIT(data, 0))
			{
				m_irq2 = true;
				update_irqs();
				m_far = target;
				m_qp_halted = true;
			}
			else if (target == ((addr << 1) & 0x7ffe))
			{
				m_far = target;          // idle loop
				m_qp_halted = true;
			}
			else
			{
				m_far = target;
			}
			continue;
		}

		if (reg == 0x17 && (data & 0xff) == 0x0b)
		{
			m_watchdog->watchdog_reset();
			m_qp_wait_vblank = true;
			continue;
		}

		queue_register_w(reg, data);
	}

	if (budget <= 0)
		LOGMASKED(LOG_UNKNOWN, "Queue Processor instruction budget exhausted at %04x\n", m_far);
}

void atarisy4_state::queue_register_w(unsigned reg, uint16_t data)
{
	switch (reg)
	{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
			m_gr[reg] = data;
			break;

		case 0x08: case 0x09: case 0x0a: case 0x0b:
			m_bcr[reg & 3] = data & 0xff;
			break;

		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			m_psr[reg & 3] = data & 0xff;
			break;

		case 0x10: case 0x11: case 0x12: case 0x13:
			m_bpr[reg & 3] = data & 0xff;
			break;

		case 0x14: m_dpr = data & 0xff; break;
		case 0x15: m_ctr = data & 0xff; break;
		case 0x16: m_ifr = data & 0xff; break;

		case 0x17:
			m_ecr = data & 0xff;
			m_watchdog->watchdog_reset();
			execute_gc_command(m_ecr);
			break;

		case 0x18:
			m_far = data & 0x7ffe;
			m_qp_halted = false;
			break;

		default:
			LOGMASKED(LOG_UNKNOWN, "Write to reserved queue register %02x = %04x\n", reg, data);
			break;
	}
}


/*************************************
 *
 *  Graphic Controller
 *
 *  UCODE.RND instruction summary:
 *
 *    00 REXP   reset expansion          01 SEXP  set expansion
 *    02 ROFF   reset offset             03 SOFF  set offset
 *    04 RTPS   reset transposition      05 STPS  set transposition
 *    06 SIMD   data                     07 SICD  data
 *    08 RCW    reset clip window        09 SCW   LM TM RM BM
 *    0E LDSR   data                     0F POKE  A data
 *    10 FXOT   start size data step     11 FYOT  start size data step
 *    12 SMER   X Y start size           13 ERSM  X Y start size
 *    14 FERT   start size data step     15 FCT   start size data step BGR..CT
 *    16 SMCT   X Y start size BGR..CT   17 CTSM  X Y start size CT
 *    20 ITSN   X Y W H IL IH IO         21 ITSC  X Y W H IL IH IO (clip)
 *    22 ITSF   X Y W H IL IH IO
 *    24 PPA    X Y data                 25 PPR   DX DY data
 *    26 VPA    X Y data                 27 VPR   DX DY data
 *    28 PFOA   X Y                      29 PFOR  DX DY
 *    2A PFVA   X Y                      2B PFVR  DX DY
 *    2C PFC    data (GR2)
 *
 *************************************/

static inline int sext12(uint16_t v) { return util::sext(v, 12); }

void atarisy4_state::fill_translation_ram(unsigned base, int start, int size, int data, int step, int limit)
{
	for (int b = 0; b < 4; b++)
	{
		if (!buffer_updating(b) || !(m_bcr[b] & BCR_TRW))
			continue;

		int value = data;
		for (int i = 0; i < size; i++)
		{
			const int idx = start + i;
			if (idx >= limit)
				break;
			const unsigned addr = (base == 0x180) ? expansion_address(idx & 0xff) : (base + idx);
			m_trans_ram[b][addr & 0x3ff] = value & 0xfff;
			value += step;
		}
	}
}

void atarisy4_state::image_transfer(bool clip)
{
	const int x0 = m_gr[0] & 0xfff;
	const int y0 = m_gr[1] & 0xfff;
	const int w = sext12(m_gr[2]);
	const int h = sext12(m_gr[3]);
	const uint32_t base = ((m_gr[5] & 0xff) << 12) | (m_gr[4] & 0xfff);
	const int io = m_gr[6] & 0xfff;

	const int aw = std::abs(w);
	const int ah = std::abs(h);

	// Image Format Register
	const int mask_code = m_ifr & 0x0f;
	const int shift = (m_ifr >> 4) & 7;
	// The spec (rev 8403.22) describes IFR bit 7 as 1 = partial transfer, but
	// every Last Starfighter routine relies on the opposite: SCREENFILL uses
	// IFR=$80 for a solid rectangle, while HDUP.ASM/AN10 load SICD=1 ("TX
	// everything but 0") with bit 7 clear, and the temperature gauges use SICD
	// as a fill threshold.  So bit 7 set = full transfer, clear = partial.
	const bool partial = !BIT(m_ifr, 7);

	uint8_t mask;
	if (mask_code <= 8)
		mask = (1 << mask_code) - 1;
	else
		mask = 0xff << (mask_code - 8);

	const uint32_t rom_mask = m_image_rom ? (m_image_rom.bytes() - 1) : 0;

	for (int r = 0; r < ah; r++)
	{
		const int dy = (h < 0) ? (ah - 1 - r) : r;

		for (int c = 0; c < aw; c++)
		{
			const int dx = (w < 0) ? (aw - 1 - c) : c;

			// Transposition swaps the Image Memory horizontal/vertical addressing
			const uint32_t src = m_transpose ? (base + c * io + r) : (base + r * io + c);
			uint8_t data = m_image_rom ? m_image_rom[src & rom_mask] : 0;

			data = (data & mask) >> shift;

			if (partial && data < m_icd)
				continue;

			// Merge with the Image Data Register.  OR (not add) matches the
			// Last Starfighter HUD colour usage: SIMD 3 "make green" with
			// 1-bit data gives colour 3 (green), SIMD 5 "white" gives 5.
			data |= m_idr;

			const int sx = x0 + dx;
			const int sy = y0 + dy;

			if (clip && (sx < m_clip_l || sx > m_clip_r || sy < m_clip_t || sy > m_clip_b))
				continue;

			const int mx = sx - SM_X0;
			const int my = sy - SM_Y0;
			if (mx < 0 || mx > 511 || my < 0 || my > 511)
				continue;

			if (!m_expansion)
			{
				write_update_pixel(mx, my, data, m_offset);
			}
			else
			{
				// Each update Buffer converts through its own Expansion Table
				for (int b = 0; b < 4; b++)
				{
					if (!buffer_updating(b) || !(m_bcr[b] & BCR_SRW))
						continue;

					const uint8_t saved[4] = { m_bcr[0], m_bcr[1], m_bcr[2], m_bcr[3] };
					for (int o = 0; o < 4; o++)
						if (o != b)
							m_bcr[o] &= ~BCR_SRW;
					write_update_pixel(mx, my, m_trans_ram[b][expansion_address(data)] & 0xff, m_offset);
					for (int o = 0; o < 4; o++)
						m_bcr[o] = saved[o];
				}
			}
		}
	}
}

void atarisy4_state::draw_line(int x0, int y0, int x1, int y1, uint8_t data)
{
	const int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	const int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy;

	for (;;)
	{
		plot_screen_space(x0, y0, data, false);
		if (x0 == x1 && y0 == y1)
			break;
		const int e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; }
		if (e2 <= dx) { err += dx; y0 += sy; }
	}
}

void atarisy4_state::fill_polygon(uint8_t data)
{
	const int n = m_poly_count;
	if (n == 0)
		return;

	int miny = m_poly_y[0], maxy = m_poly_y[0];
	for (int i = 1; i < n; i++)
	{
		miny = std::min(miny, m_poly_y[i]);
		maxy = std::max(maxy, m_poly_y[i]);
	}

	// Restrict to Screen Memory rows
	miny = std::max(miny, SM_Y0);
	maxy = std::min(maxy, SM_Y0 + 511);

	// Even-odd scanline fill, sampled at pixel centres
	int xs[MAX_POLY_POINTS];
	for (int y = miny; y <= maxy; y++)
	{
		const double fy = y + 0.5;
		int count = 0;

		for (int i = 0; i < n; i++)
		{
			const int j = (i + 1) % n;
			const int ya = m_poly_y[i], yb = m_poly_y[j];
			if (ya == yb)
				continue;
			if ((fy >= ya && fy < yb) || (fy >= yb && fy < ya))
			{
				const double t = (fy - ya) / double(yb - ya);
				const double fx = m_poly_x[i] + t * (m_poly_x[j] - m_poly_x[i]);
				xs[count++] = int(std::floor(fx + 0.5));
			}
		}

		std::sort(xs, xs + count);
		for (int k = 0; k + 1 < count; k += 2)
		{
			const int xa = std::max(xs[k], SM_X0);
			const int xb = std::min(xs[k + 1], SM_X0 + 511);
			for (int x = xa; x <= xb; x++)
				plot_screen_space(x, y, data, false);
		}
	}

	// Edges (ensures degenerate / sub-pixel polygons still appear)
	for (int i = 0; i < n; i++)
	{
		const int j = (i + 1) % n;
		draw_line(m_poly_x[i], m_poly_y[i], m_poly_x[j], m_poly_y[j], data);
	}
}

void atarisy4_state::execute_gc_command(uint8_t cmd)
{
	switch (cmd)
	{
		case 0x00: m_expansion = false; break;  // REXP
		case 0x01: m_expansion = true;  break;  // SEXP
		case 0x02: m_offset = false;    break;  // ROFF
		case 0x03: m_offset = true;     break;  // SOFF
		case 0x04: m_transpose = false; break;  // RTPS
		case 0x05: m_transpose = true;  break;  // STPS

		case 0x06: // SIMD - image merge data
			m_idr = m_gr[0] & 0xff;
			break;

		case 0x07: // SICD - image compare data
			m_icd = m_gr[0] & 0xff;
			break;

		case 0x08: // RCW - clip to visible Screen Memory
			m_clip_l = SM_X0;
			m_clip_r = SM_X0 + 511;
			m_clip_t = SM_Y0;
			m_clip_b = SM_Y0 + 383;
			break;

		case 0x09: // SCW
			m_clip_l = m_gr[0] & 0xfff;
			m_clip_t = m_gr[1] & 0xfff;
			m_clip_r = m_gr[2] & 0xfff;
			m_clip_b = m_gr[3] & 0xfff;
			break;

		case 0x0b: // WVR - wait for vertical retrace (not in the spec; used by Air Race)
			if (!(m_mcr & MCR_QP) && !m_screen->vblank())
				m_maincpu->spin_until_time(m_screen->time_until_vblank_start());
			break;

		case 0x0e: // LDSR - load GC status register (bit assignment unverified)
			m_expansion = BIT(m_gr[0], 0);
			m_offset = BIT(m_gr[0], 1);
			m_transpose = BIT(m_gr[0], 2);
			break;

		case 0x0f: // POKE - GC internal RAM
			LOGMASKED(LOG_GPU, "POKE %03x = %03x\n", m_gr[0] & 0x3ff, m_gr[1]);
			break;

		case 0x10: // FXOT - horizontal offset table
			fill_translation_ram(0x200, m_gr[0] & 0x1ff, m_gr[1] & 0x3ff, m_gr[2], sext12(m_gr[3]), 384);
			break;

		case 0x11: // FYOT - vertical offset table
			fill_translation_ram(0x000, m_gr[0] & 0x1ff, m_gr[1] & 0x3ff, m_gr[2], sext12(m_gr[3]), 384);
			break;

		case 0x14: // FERT - expansion/replacement table
			fill_translation_ram(0x180, m_gr[0] & 0xff, m_gr[1] & 0x1ff, m_gr[2], sext12(m_gr[3]), 256);
			break;

		case 0x15: // FCT - fill color table
		{
			const unsigned table = (m_gr[4] & 3) << 8;
			int value = m_gr[2];
			for (int i = 0; i < (m_gr[1] & 0x1ff); i++)
			{
				const unsigned addr = table | ((m_gr[0] + i) & 0xff);
				for (int c = 0; c < 3; c++)
					if (BIT(m_gr[4], 4 + c))
						m_color_ram[c][addr] = value;
				value += sext12(m_gr[3]);
			}
			break;
		}

		case 0x16: // SMCT - Screen Memory to color table
		{
			int order[4];
			display_priority(order);
			const int b = order[0];
			const int x = (m_gr[0] & 0xfff) - SM_X0;
			const int y = (m_gr[1] & 0xfff) - SM_Y0;
			const unsigned table = (m_gr[4] & 3) << 8;
			for (int i = 0; i < (m_gr[3] & 0x1ff); i++)
			{
				const uint8_t val = m_screen_ram[b][((y & 0x1ff) << 9) | ((x + i) & 0x1ff)];
				const unsigned addr = table | ((m_gr[2] + i) & 0xff);
				for (int c = 0; c < 3; c++)
					if (BIT(m_gr[4], 4 + c))
						m_color_ram[c][addr] = val;
			}
			break;
		}

		case 0x17: // CTSM - color table to Screen Memory (R, G, B on consecutive lines)
		{
			const int x = (m_gr[0] & 0xfff) - SM_X0;
			const int y = (m_gr[1] & 0xfff) - SM_Y0;
			const unsigned table = (m_gr[4] & 3) << 8;
			for (int i = 0; i < (m_gr[3] & 0x1ff); i++)
				for (int c = 0; c < 3; c++)
					write_update_pixel((x + i) & 0x1ff, (y + c) & 0x1ff, m_color_ram[c][table | ((m_gr[2] + i) & 0xff)], false);
			break;
		}

		case 0x20: // ITSN
		case 0x22: // ITSF
			image_transfer(true);
			break;

		case 0x21: // ITSC
			image_transfer(true);
			break;

		case 0x24: // PPA
			m_cur_x = m_gr[0] & 0xfff;
			m_cur_y = m_gr[1] & 0xfff;
			plot_screen_space(m_cur_x, m_cur_y, m_gr[2] & 0xff, false);
			break;

		case 0x25: // PPR
			m_cur_x += sext12(m_gr[0]);
			m_cur_y += sext12(m_gr[1]);
			plot_screen_space(m_cur_x, m_cur_y, m_gr[2] & 0xff, false);
			break;

		case 0x26: // VPA
		{
			const int x1 = m_gr[0] & 0xfff, y1 = m_gr[1] & 0xfff;
			draw_line(m_cur_x, m_cur_y, x1, y1, m_gr[2] & 0xff);
			m_cur_x = x1; m_cur_y = y1;
			break;
		}

		case 0x27: // VPR
		{
			const int x1 = m_cur_x + sext12(m_gr[0]), y1 = m_cur_y + sext12(m_gr[1]);
			draw_line(m_cur_x, m_cur_y, x1, y1, m_gr[2] & 0xff);
			m_cur_x = x1; m_cur_y = y1;
			break;
		}

		case 0x28: // PFOA
			m_cur_x = m_gr[0] & 0xfff;
			m_cur_y = m_gr[1] & 0xfff;
			m_poly_x[0] = m_cur_x;
			m_poly_y[0] = m_cur_y;
			m_poly_count = 1;
			break;

		case 0x29: // PFOR
			m_cur_x += sext12(m_gr[0]);
			m_cur_y += sext12(m_gr[1]);
			m_poly_x[0] = m_cur_x;
			m_poly_y[0] = m_cur_y;
			m_poly_count = 1;
			break;

		case 0x2a: // PFVA
		case 0x2b: // PFVR
			if (cmd == 0x2a)
			{
				m_cur_x = m_gr[0] & 0xfff;
				m_cur_y = m_gr[1] & 0xfff;
			}
			else
			{
				m_cur_x += sext12(m_gr[0]);
				m_cur_y += sext12(m_gr[1]);
			}
			if (m_poly_count < MAX_POLY_POINTS)
			{
				m_poly_x[m_poly_count] = m_cur_x;
				m_poly_y[m_poly_count] = m_cur_y;
				m_poly_count++;
			}
			break;

		case 0x2c: // PFC - close and fill
			fill_polygon(m_gr[2] & 0xff);
			m_poly_count = 0;
			break;

		default:
			LOGMASKED(LOG_UNKNOWN, "Unimplemented GC instruction %02x\n", cmd);
			break;
	}
}


/*************************************
 *
 *  68000<->TMS comms
 *
 *************************************/

uint16_t atarisy4_state::m68k_shared_0_r(offs_t offset)
{
	if (!BIT(m_csr[0], 3))
		return (m_shared_ram[0][offset]);
	else
		return 0xffff;
}

void atarisy4_state::m68k_shared_0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!BIT(m_csr[0], 3))
		COMBINE_DATA(&m_shared_ram[0][offset]);
}

uint16_t atarisy4_state::m68k_shared_1_r(offs_t offset)
{
	if (!BIT(m_csr[1], 3))
		return (m_shared_ram[1][offset]);
	else
		return 0xffff;
}

void atarisy4_state::m68k_shared_1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!BIT(m_csr[1], 3))
		COMBINE_DATA(&m_shared_ram[1][offset]);
}

/*
    Mathbox control/status register (from MBINTERC.C)
        bit 0  FIREUPTI   0 = reset TMS, 1 = run
        bit 1  INTERTI    interrupt TMS
        bit 2  BIO        TMS BIO pin
        bit 3  TIOWNSMEM  0 = 68000 owns shared memory, 1 = TMS
        bit 4  TIDONE     set by TMS "OUT h4000,0"
        bit 5  INTER68    TIDONE also interrupts the 68000
*/
uint16_t atarisy4_state::dsp0_status_r()
{
	return m_csr[0];
}

void atarisy4_state::dsp0_control_w(uint16_t data)
{
	m_dsp0->set_input_line(INPUT_LINE_RESET, data & 0x01 ? CLEAR_LINE : ASSERT_LINE);
	m_dsp0->set_input_line(0, data & 0x02 ? ASSERT_LINE : CLEAR_LINE);

	m_csr[0] = data;
}

int atarisy4_state::dsp0_bio_r()
{
	return BIT(m_csr[0], 2);
}

void atarisy4_state::dsp0_bank_w(uint16_t data)
{
	if (data & 0x4000)
	{
		/* Set TIDONE bit */
		m_csr[0] |= 0x10;

		if (BIT(m_csr[0], 5))
			LOGMASKED(LOG_UNKNOWN, "Mathbox 68000 interrupt requested (not hooked up)\n");
	}

	data &= 0x3800;
	m_dsp0_bank1->set_base(&m_shared_ram[0][data]);
	m_dsp_bank[0] = data;
}

uint16_t airrace_state::dsp1_status_r()
{
	return m_csr[1];
}

void airrace_state::dsp1_control_w(uint16_t data)
{
	m_dsp1->set_input_line(INPUT_LINE_RESET, data & 0x01 ? CLEAR_LINE : ASSERT_LINE);
	m_dsp1->set_input_line(0, data & 0x02 ? ASSERT_LINE : CLEAR_LINE);

	m_csr[1] = data;
}

int airrace_state::dsp1_bio_r()
{
	return BIT(m_csr[1], 2);
}

void airrace_state::dsp1_bank_w(uint16_t data)
{
	if (data & 0x4000)
	{
		/* Set TIDONE bit */
		m_csr[1] |= 0x10;

		if (BIT(m_csr[1], 5))
			LOGMASKED(LOG_UNKNOWN, "Mathbox 2 68000 interrupt requested (not hooked up)\n");
	}

	data &= 0x3800;
	m_dsp1_bank1->set_base(&m_shared_ram[1][data]);
	m_dsp_bank[1] = data;
}


/*************************************
 *
 *  Main CPU memory map
 *
 *************************************/

void atarisy4_state::main_map(address_map &map)
{
	map(0x000000, 0x01ffff).ram().share("m68k_ram");
	map(0x580000, 0x580001).portr("JOYSTICK");
	map(0x588000, 0x588001).r(FUNC(atarisy4_state::analog_r));
	map(0x598000, 0x598001).noprw(); /* Sound board */
	map(0x7c0000, 0x7c4fff).rw(FUNC(atarisy4_state::m68k_shared_1_r), FUNC(atarisy4_state::m68k_shared_1_w));
	map(0x7f0000, 0x7f4fff).rw(FUNC(atarisy4_state::m68k_shared_0_r), FUNC(atarisy4_state::m68k_shared_0_w));
	map(0x7f6000, 0x7f6001).rw(FUNC(atarisy4_state::dsp0_status_r), FUNC(atarisy4_state::dsp0_control_w));
	map(0x800000, 0xdfffff).rw(FUNC(atarisy4_state::screen_space_r), FUNC(atarisy4_state::screen_space_w));
	map(0xf00000, 0xf07fff).rw(FUNC(atarisy4_state::queue_ram_r), FUNC(atarisy4_state::queue_ram_w));
	map(0xff8000, 0xff8fff).rw(FUNC(atarisy4_state::pm_reg_r), FUNC(atarisy4_state::pm_reg_w));
}

void airrace_state::airrace_map(address_map &map)
{
	main_map(map);

	map(0x7c6000, 0x7c6001).rw(FUNC(airrace_state::dsp1_status_r), FUNC(airrace_state::dsp1_control_w));
}


/*************************************
 *
 *  Mathbox DSP memory map
 *
 *************************************/

void atarisy4_state::dsp0_map(address_map &map)
{
	map.global_mask(0xfff);
	map(0x0000, 0x07ff).bankrw("dsp0_bank0");
	map(0x0800, 0x0fff).bankrw("dsp0_bank1");
}

void atarisy4_state::dsp0_io_map(address_map &map)
{
	map(0x00, 0x01).w(FUNC(atarisy4_state::dsp0_bank_w));
}

void airrace_state::dsp1_map(address_map &map)
{
	map.global_mask(0xfff);
	map(0x0000, 0x07ff).bankrw("dsp1_bank0");
	map(0x0800, 0x0fff).bankrw("dsp1_bank1");
}

void airrace_state::dsp1_io_map(address_map &map)
{
	map(0x00, 0x01).w(FUNC(airrace_state::dsp1_bank_w));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

uint16_t atarisy4_state::analog_r()
{
	return (m_stick[0]->read() << 8) | m_stick[1]->read();
}

/*
    Both games use the Star Wars / Firefox flight yoke: an analog X/Y yoke on
    $588000 (high byte left/right, low byte up/down) and its four buttons on
    $580000 D12-D15.

    Last Starfighter: struct joybits in LSF.INC (Whitesmiths C, LSB-first bitfields).
    D0-D7 are two digital debug joysticks (rotate() in MISCC.C), D8/D9 debug buttons.
*/
static INPUT_PORTS_START( atarisy4 )
	PORT_START("JOYSTICK")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT )  PORT_NAME("Debug Right Stick Left (Roll)")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_NAME("Debug Right Stick Right (Roll)")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN )  PORT_NAME("Debug Right Stick Down (Pitch)")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP )    PORT_NAME("Debug Right Stick Up (Pitch)")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )   PORT_NAME("Debug Left Stick Left (Yaw)")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT )  PORT_NAME("Debug Left Stick Right (Yaw)")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )   PORT_NAME("Debug Left Stick Down (Back)")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )     PORT_NAME("Debug Left Stick Up (Ahead)")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Debug Top Button (Reset Speed)")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Debug Bottom Button (Reset Speed)")
	PORT_BIT( 0x0c00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Yoke Right Trigger (Right Laser)")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Yoke Left Trigger (Left Laser)")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Yoke Left Thumb (Slower)")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Yoke Right Thumb (Faster)")

	PORT_START("STICKX")    // high byte: yaw
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30)

	PORT_START("STICKY")    // low byte: pitch
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30)

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "Watchdog" )
	PORT_CONFSETTING(    0x00, "Disabled (Debug Module)" )
	PORT_CONFSETTING(    0x01, "Enabled (8 frames)" )
INPUT_PORTS_END

/*
    Air Race: struct joybits in FLY.H (gcc, MSB-first bitfields).
    D0-D7 foot pedal (throttle), D8 1P start, D9 2P start, D10/D11 spare,
    D12-D15 yoke triggers.  Any trigger starts a race.
*/
static INPUT_PORTS_START( airrace )
	PORT_INCLUDE( atarisy4 )

	PORT_MODIFY("JOYSTICK")
	PORT_BIT( 0x00ff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(20) PORT_NAME("Foot Pedal")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0c00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Yoke Front Left Trigger")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Yoke Front Right Trigger")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Yoke Back Left Trigger")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Yoke Back Right Trigger")
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void atarisy4_state::atarisy4(machine_config &config)
{
	M68000(config, m_maincpu, 32_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &atarisy4_state::main_map);

	TMS320C10(config, m_dsp0, 16'000'000);
	m_dsp0->set_addrmap(AS_PROGRAM, &atarisy4_state::dsp0_map);
	m_dsp0->set_addrmap(AS_IO, &atarisy4_state::dsp0_io_map);
	m_dsp0->bio().set(FUNC(atarisy4_state::dsp0_bio_r));

	// Spec: 32MHz master, 16MHz pixel clock, 660 x 404 total, 512 x 384 visible, 60Hz
	SCREEN(config, m_screen);
	m_screen->set_raw(32_MHz_XTAL / 2, 660, 0, 512, 404, 0, 384);
	m_screen->set_screen_update(FUNC(atarisy4_state::screen_update));
	m_screen->screen_vblank().set(FUNC(atarisy4_state::vblank_w));

	// "Power up and watchdog timer reset is provided after 8 frames"; cleared by ECR loads
	WATCHDOG_TIMER(config, m_watchdog).set_vblank_count(m_screen, 8);
}

void airrace_state::airrace(machine_config &config)
{
	atarisy4(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &airrace_state::airrace_map);

	TMS320C10(config, m_dsp1, 16'000'000);
	m_dsp1->set_addrmap(AS_PROGRAM, &airrace_state::dsp1_map);
	m_dsp1->set_addrmap(AS_IO, &airrace_state::dsp1_io_map);
	m_dsp1->bio().set(FUNC(airrace_state::dsp1_bio_r));
}


/*************************************
 *
 *  ROM definition(s)
 *
 *  The 1984-85 Last Starfighter and Air Race programs were developed on a VAX and
 *  downloaded into RAM through the Debug Module; these sets are flat
 *  images of 68000 space $00000-$1FFFF built from the original Tektronix
 *  hex files (even/odd bytes, 64KB each), plus the Mathbox TMS32010
 *  program (MATHBS.LDA) as big-endian words.
 *
 *************************************/

#define LASTSTAR_COMMON \
	ROM_REGION( 0x1000, "dsp", 0 ) \
	ROM_LOAD( "mathbs.bin", 0x0000, 0x1000, CRC(06335375) SHA1(5a8faccade52f00ccf6676c0b79dfa5d638ca4d3) ) \
	ROM_REGION( 0x100000, "gfx", ROMREGION_ERASE00 ) \
	ROM_LOAD( "image_memory", 0x00000, 0x20000, NO_DUMP )   /* HUD art (bank 0) and fonts (bank 1) */

// Cave sequence demo - DEMO.HEX (CAVESEQ + DATA1 + Mathbox code at $400), 27-Aug-1984
ROM_START( laststar )
	ROM_REGION16_BE( 0x20000, "program", 0 )
	ROM_LOAD16_BYTE( "laststar.even", 0x00000, 0x10000, CRC(a1e8b088) SHA1(899e10e3fc04f3b7f82a7f6fd4632da4c5d7bff8) )
	ROM_LOAD16_BYTE( "laststar.odd",  0x00001, 0x10000, CRC(d59a2966) SHA1(97e314a1bf48e27f727e7671ecf41302c914d30e) )
	LASTSTAR_COMMON
ROM_END

// Same program with a RECONSTRUCTED font Image Memory: layout from [morris.lsf]ALPHANUM.ASM,
// glyph shapes from Atari's 6x10 set (DATA\AN6X10.TXT).  Not original data.
ROM_START( laststarf )
	ROM_REGION16_BE( 0x20000, "program", 0 )
	ROM_LOAD16_BYTE( "laststar.even", 0x00000, 0x10000, CRC(a1e8b088) SHA1(899e10e3fc04f3b7f82a7f6fd4632da4c5d7bff8) )
	ROM_LOAD16_BYTE( "laststar.odd",  0x00001, 0x10000, CRC(d59a2966) SHA1(97e314a1bf48e27f727e7671ecf41302c914d30e) )

	ROM_REGION( 0x1000, "dsp", 0 )
	ROM_LOAD( "mathbs.bin", 0x0000, 0x1000, CRC(06335375) SHA1(5a8faccade52f00ccf6676c0b79dfa5d638ca4d3) )

	ROM_REGION( 0x100000, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "lsf_fonts_recon.bin", 0x00000, 0x20000, CRC(d87fe439) SHA1(47588465f1173a103c2b89e6559b21a85c8f09ec) )
ROM_END

// Tube fly-through test - CAVE\TUBEDEMMB.HEX + DATA.HEX, 11-Aug-1984
ROM_START( laststart )
	ROM_REGION16_BE( 0x20000, "program", 0 )
	ROM_LOAD16_BYTE( "laststart.even", 0x00000, 0x10000, CRC(cc012298) SHA1(3cc20b0ad90418829fc65405ee3918b319e9414a) )
	ROM_LOAD16_BYTE( "laststart.odd",  0x00001, 0x10000, CRC(3ed59016) SHA1(e7e8174699e4a2655d038cb225e472e75a98b45d) )
	LASTSTAR_COMMON
ROM_END

// TESTAST - TEST\TESTAST.HEX + TEST\DATA1.HEX, Aug-1984
ROM_START( laststara )
	ROM_REGION16_BE( 0x20000, "program", 0 )
	ROM_LOAD16_BYTE( "laststara.even", 0x00000, 0x10000, CRC(5d9caab4) SHA1(3a9012d62953ef58fa74f3c17fd34f68ddaceade) )
	ROM_LOAD16_BYTE( "laststara.odd",  0x00001, 0x10000, CRC(52059eea) SHA1(8c02bcc36f8b9a28ca306c924c543af6b58107bd) )
	LASTSTAR_COMMON
ROM_END

// TESTOBJ - TEST\TESTOBJ.HEX + TEST\DATA1.HEX, Aug-1984
ROM_START( laststaro )
	ROM_REGION16_BE( 0x20000, "program", 0 )
	ROM_LOAD16_BYTE( "laststaro.even", 0x00000, 0x10000, CRC(027f622f) SHA1(79b07728a36ce3f2df1b9375f23b9b06ef511859) )
	ROM_LOAD16_BYTE( "laststaro.odd",  0x00001, 0x10000, CRC(7cb75e98) SHA1(4e07ac582aa4bb03da8509518084177f68947bc4) )
	LASTSTAR_COMMON
ROM_END


// Air Race, track 1 - MORRIS 4\AIR\DEMO.HEX (TRK1 + Mathbox code at $400 + DATA\DEMO track data), Dec-1985
ROM_START( airrace )
	ROM_REGION16_BE( 0x20000, "program", 0 )
	ROM_LOAD16_BYTE( "airrace.even", 0x00000, 0x10000, CRC(fe98469f) SHA1(90d61b70c3bd8c42048d3a54a32610e2a009cc5f) )
	ROM_LOAD16_BYTE( "airrace.odd",  0x00001, 0x10000, CRC(a0219aa7) SHA1(d3271cb20cfbfb8e3ddf4af369a9196dfc4de33d) )

	ROM_REGION( 0x1000, "dsp", 0 )  // MATHBS.LDA, runs on both Mathboxes
	ROM_LOAD( "mathbs.bin", 0x0000, 0x1000, CRC(228ee717) SHA1(8b94707edd875b8dc02f6e9624b7a3c51e986934) )

	ROM_REGION( 0x100000, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "image_memory", 0x00000, 0x20000, NO_DUMP )   // shared with Last Starfighter (same HDUP gauge / ALPHANUM fonts)
ROM_END

// Same program with the reconstructed font Image Memory (not original data)
ROM_START( airracef )
	ROM_REGION16_BE( 0x20000, "program", 0 )
	ROM_LOAD16_BYTE( "airrace.even", 0x00000, 0x10000, CRC(fe98469f) SHA1(90d61b70c3bd8c42048d3a54a32610e2a009cc5f) )
	ROM_LOAD16_BYTE( "airrace.odd",  0x00001, 0x10000, CRC(a0219aa7) SHA1(d3271cb20cfbfb8e3ddf4af369a9196dfc4de33d) )

	ROM_REGION( 0x1000, "dsp", 0 )
	ROM_LOAD( "mathbs.bin", 0x0000, 0x1000, CRC(228ee717) SHA1(8b94707edd875b8dc02f6e9624b7a3c51e986934) )

	ROM_REGION( 0x100000, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "lsf_fonts_recon.bin", 0x00000, 0x20000, CRC(d87fe439) SHA1(47588465f1173a103c2b89e6559b21a85c8f09ec) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void atarisy4_state::init_laststar()
{
	/* 16K words of Mathbox shared RAM */
	m_shared_ram[0] = make_unique_clear<uint16_t[]>(0x4000);

	membank("dsp0_bank0")->set_base(m_shared_ram[0].get());
	m_dsp0_bank1->set_base(&m_shared_ram[0][0x800]);
}

void airrace_state::init_airrace()
{
	init_laststar();

	m_shared_ram[1] = make_unique_clear<uint16_t[]>(0x4000);
	membank("dsp1_bank0")->set_base(m_shared_ram[1].get());
	m_dsp1_bank1->set_base(&m_shared_ram[1][0x800]);
}

void atarisy4_state::machine_start()
{
	// Program image -> RAM (as downloaded by the development system)
	std::copy_n(&m_prog_rom[0], std::min<size_t>(m_prog_rom.length(), m_m68k_ram.length()), &m_m68k_ram[0]);

	// Mathbox program -> shared RAM bank 0 of each Mathbox
	for (auto &shared : m_shared_ram)
		if (shared)
			for (size_t i = 0; i < m_dsp_rom.bytes() / 2; i++)
				shared[i] = (m_dsp_rom[i * 2] << 8) | m_dsp_rom[i * 2 + 1];

	save_item(NAME(m_dsp_bank));
	save_item(NAME(m_csr));
	save_pointer(NAME(m_shared_ram[0]), 0x4000);
	if (m_shared_ram[1])
		save_pointer(NAME(m_shared_ram[1]), 0x4000);
}

void atarisy4_state::machine_reset()
{
	m_dsp0->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_csr[0] = m_csr[1] = 0;

	// Watchdog/power-up reset clears all MCR bits
	m_mcr = 0;
	m_irq1 = m_irq2 = false;
	m_qp_wait_vblank = m_qp_halted = false;
	m_maincpu->set_input_line(6, CLEAR_LINE);
	update_irqs();

	execute_gc_command(0x08); // RCW
}

void airrace_state::machine_reset()
{
	atarisy4_state::machine_reset();

	m_dsp1->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1984, laststar,  0,        atarisy4, atarisy4, atarisy4_state, init_laststar, ROT0, "Atari, Inc.", "The Last Starfighter (prototype, cave sequence demo)", MACHINE_IS_INCOMPLETE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND_HW )
GAME( 1984, laststarf, laststar, atarisy4, atarisy4, atarisy4_state, init_laststar, ROT0, "Atari, Inc.", "The Last Starfighter (prototype, cave sequence demo, reconstructed fonts)", MACHINE_IS_INCOMPLETE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND_HW )
GAME( 1984, laststart, laststar, atarisy4, atarisy4, atarisy4_state, init_laststar, ROT0, "Atari, Inc.", "The Last Starfighter (prototype, tube fly-through test)", MACHINE_IS_INCOMPLETE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND_HW )
GAME( 1984, laststara, laststar, atarisy4, atarisy4, atarisy4_state, init_laststar, ROT0, "Atari, Inc.", "The Last Starfighter (prototype, TESTAST asteroid test)", MACHINE_IS_INCOMPLETE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND_HW )
GAME( 1984, laststaro, laststar, atarisy4, atarisy4, atarisy4_state, init_laststar, ROT0, "Atari, Inc.", "The Last Starfighter (prototype, TESTOBJ cave-piece test)", MACHINE_IS_INCOMPLETE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND_HW )
GAME( 1985, airrace,   0,        airrace,  airrace,  airrace_state,  init_airrace,  ROT0, "Atari Games", "Air Race (prototype, track 1)", MACHINE_IS_INCOMPLETE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND_HW )
GAME( 1985, airracef,  airrace,  airrace,  airrace,  airrace_state,  init_airrace,  ROT0, "Atari Games", "Air Race (prototype, track 1, reconstructed fonts)", MACHINE_IS_INCOMPLETE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND_HW )

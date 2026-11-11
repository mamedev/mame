// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Jonathan Gevaryahu
/***************************************************************************

    DEC VT220

    vt220a (Version 2.1, models A-C) follows the Technical Manual and the
    schematic: CRT9007 with the line buffer, character generator and soft
    characters, X2212 NVR, host and printer ports, and the LK201.  The
    CRT9007 has no smooth scroll yet, so smooth scroll moves a row at a time.
    vt220 (Version 2.3, probably models D-F) shows nothing: its video, NVR
    and memory map beyond the RAM and DUART are not known.

    TODO (vt220a):
    - smooth scroll
    - the 20 mA host loop
    - CTS gating (MC68681 MR2 bit 4)

    The VT220 exists in two major hardware revisions. The Pocket Service
    Guide has separate lists of components for earlier and later models,
    with the following being those that differ:

                                    A, B, and C     D, E, and F
        Terminal controller         70-20814-02     70-23363-01
        Power supply/monitor board  70-20624-02     70-23361-01
        Transformer                 70-20772-02     70-23362-02
        CRT/bezel/yoke (white)      70-20662-04     70-23360-01
                       (green)      70-20662-05     70-23360-02
                       (amber)      70-20662-06     70-23360-03
        Danish keyboard             LK201-AD        LK201-ED
        British keyboard            LK201-AE        LK201-EE
        Norwegian keyboard          LK201-AN        LK201-EN

    The original VT220 Technical Manual (EK-VT220-TM) covers Models A, B,
    and C only. Though it was later reprinted with an addendum for Models
    D, E, and F, this has not been found. The available schematics are
    inapplicable to later models, which have an altogether different memory
    map and, evidently, use some different IC types: an ER5911 rather than
    X2212 as non-volatile memory and, in place of the CRT9007, some sort
    of custom video gate array (which might even be clocked differently).

****************************************************************************/

#include "emu.h"

#include "lk201.h"

#include "bus/rs232/rs232.h"
#include "cpu/mcs51/i8051.h"
#include "machine/mc68681.h"
#include "machine/x2212.h"
#include "video/crt9007.h"

#include "emupal.h"
#include "screen.h"

#include <algorithm>


namespace {

// vt220a video timing as the firmware programs the CRT9007, for 80 and 132 columns: dot clock
// (Y3, Y4), dots per character, horizontal total and delay in characters, visible characters
struct col_timing { XTAL dot; int width, htotal, hdelay, cols; };
constexpr col_timing TIMING_80 { 15.288_MHz_XTAL, 10, 98, 12, 80 };
constexpr col_timing TIMING_132 { 22.896_MHz_XTAL, 9, 163, 22, 132 };
constexpr int VTOTAL = 260, VSTART = 16, VVIS = 240; // vertical delay (R05 - 1), then 24 rows of 10 scan lines

class vt220_state : public driver_device
{
public:
	vt220_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_duart(*this, "duart"),
		m_kbd(*this, "lk201")
	{ }

	void vt220(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	void vt220_common(machine_config &config) ATTR_COLD;

	required_device<i8051_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<scn2681_device> m_duart;
	required_device<lk201_device> m_kbd;

private:
	u8 m_kbd_rxd = 1;

	uint32_t screen_update_vt220(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vt220_data(address_map &map) ATTR_COLD;
	void vt220_mem(address_map &map) ATTR_COLD;
};

class vt220a_state : public vt220_state
{
public:
	vt220a_state(const machine_config &mconfig, device_type type, const char *tag) :
		vt220_state(mconfig, type, tag),
		m_crtc(*this, "crtc"),
		m_nvr(*this, "nvr"),
		m_host(*this, "host"),
		m_printer(*this, "printer"),
		m_chargen(*this, "chargen"),
		m_screen_ram(*this, "screen_ram"),
		m_attr_ram(*this, "attr_ram")
	{ }

	void vt220a(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<crt9007_device> m_crtc;
	required_device<x2212_device> m_nvr;
	required_device<rs232_port_device> m_host;
	required_device<rs232_port_device> m_printer;
	required_region_ptr<u8> m_chargen;
	required_shared_ptr<u8> m_screen_ram;
	required_shared_ptr<u8> m_attr_ram;

	u8 m_p1 = 0xff;
	u8 m_conf = 0;
	u8 m_duart_op = 0xff;
	u8 m_char_addr = 0;
	u8 m_atr_latch = 0;
	u8 m_lba = 0;
	bool m_132col = false;
	s16 m_vlt_x = 0;
	u8 m_sl = 0;
	bool m_dw = false; // cursor skew F/F: double width scan line
	u8 m_line_sl[VTOTAL]; // per scan line: SL3-SL0 (0xff without VLT last frame) and double width
	u8 m_line_dw[VTOTAL] = { };
	bool m_line_vlt[VTOTAL] = { }; // VLT seen this frame
	bool m_vlt = false;
	bool m_drb = true;
	bool m_row_loading = false;
	bool m_cblank = true;
	u8 m_host_txd = 1;
	u8 m_host_rxd = 1;
	u8 m_softchar[0x800] = { };
	u8 m_lbf[0x800] = { };

	void vt220a_palette(palette_device &palette) const ATTR_COLD;
	u32 screen_update_vt220a(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	const col_timing &timing() const { return m_132col ? TIMING_132 : TIMING_80; }
	void set_columns();
	offs_t lbf_addr(bool chr, u8 lba) const;
	u8 lbf_out(bool chr, u8 lba) const;
	int video_col(int x, bool dw) const;
	int video_dot(int x, bool dw) const;
	bool in_row(int x, bool dw) const;
	static offs_t soft_addr(u8 chr, int line) { return ((chr & 0x7f) << 4) | line; }
	u8 char_row(bool alt, u8 chr, int line) const;
	u8 dot(int col, int x, int line, bool &bold, bool &blink_n) const;
	u8 video_out(int x, int y, bool &bold, bool &blink_n) const;
	void write_host_txd(u8 conf) { m_host->write_txd(BIT(conf, 6) ? 1 : m_host_txd); }

	u8 p1_r();
	void p1_w(u8 data);
	void conf_w(u8 data);
	void char_addr_w(u8 data);
	u8 chargen_r(offs_t offset);
	void chargen_w(offs_t offset, u8 data);
	u8 duart_r(offs_t offset);
	void duart_out_w(u8 data);
	void host_txd_w(int state);
	void host_rxd_w(int state);
	u8 vram_r(offs_t offset);
	void vs_w(int state);
	void vlt_w(int state);
	void sl_w(u8 data);
	void curs_w(int state);
	void drb_w(int state);
	void cblank_w(int state);

	void vt220a_data(address_map &map) ATTR_COLD;
	void vt220a_mem(address_map &map) ATTR_COLD;
	void vt220a_vram(address_map &map) ATTR_COLD;
};


void vt220_state::vt220_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
}

void vt220a_state::vt220a_mem(address_map &map)
{
	map(0x0000, 0xffff).rom().region("maincpu", 0);
}

void vt220_state::vt220_data(address_map &map)
{
	map.unmap_value_high();
	map(0x2000, 0x2fff).mirror(0xc000).ram();
	map(0x3800, 0x380f).mirror(0xc7f0).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
}

void vt220a_state::vt220a_data(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0000).mirror(0x47ff).w(FUNC(vt220a_state::conf_w)); // 07ff in the technical manual, as the rest
	map(0x0800, 0x0800).mirror(0x47ff).w(FUNC(vt220a_state::char_addr_w)); // 0fff
	map(0x1000, 0x103f).mirror(0x47c0).rw(m_crtc, FUNC(crt9007_device::read), FUNC(crt9007_device::write)); // 1700-17ff
	map(0x1800, 0x1800).mirror(0x47ff).nopw(); // 1fff RESET TIM L: strobe resetting the timing generator, data ignored
	map(0x2000, 0x2fff).mirror(0xc000).ram().share(m_screen_ram); // RAM 0/1, a000-afff
	map(0x3000, 0x300f).mirror(0xc7f0).r(FUNC(vt220a_state::duart_r)).w(m_duart, FUNC(scn2681_device::write)); // b7f0-b7ff
	map(0x8000, 0x8fff).mirror(0x4000).ram().share(m_attr_ram); // attribute RAM 0/1
	map(0x9000, 0x90ff).mirror(0x4700).rw(m_nvr, FUNC(x2212_device::read), FUNC(x2212_device::write)); // 9700-97ff
	map(0x9800, 0x980f).mirror(0x47f0).rw(FUNC(vt220a_state::chargen_r), FUNC(vt220a_state::chargen_w)); // 9ff0-9fff
}

offs_t vt220a_state::lbf_addr(bool chr, u8 lba) const
{
	// E14 (2016): A10/A9 BNKS1/BNKS0, A8 ATR/CHR (high for character), A7-A0 LBA
	return (BIT(m_duart_op, 5, 2) << 9) | (chr ? 0x100 : 0) | lba; // OP5/OP6 BNK S0/S1
}

u8 vt220a_state::lbf_out(bool chr, u8 lba) const
{
	// CHRG LATCH2 (E20) takes the character address latch (E19), and the attribute path the attribute
	// latch (E21), while LBF DISABLE H (P1.3) is high, otherwise the line buffer output
	if (BIT(m_p1, 3))
		return chr ? m_char_addr : m_atr_latch;
	return m_lbf[lbf_addr(chr, lba)];
}

void vt220a_state::vt220a_vram(address_map &map)
{
	// CRT9007 VA0-VA13 drive the CPU address bus during DMA
	map(0x0000, 0x3fff).r(FUNC(vt220a_state::vram_r));
}

int vt220a_state::video_col(int x, bool dw) const
{
	// the line buffer address counter is cleared by VLT and clocked by T5, -1 before VLT; on double
	// width lines the INH gate holds off every other character load, so it advances at half rate
	const int dx = x - m_vlt_x;
	return (dx < 0) ? -1 : (dx >> (dw ? 1 : 0)) / timing().width;
}

int vt220a_state::video_dot(int x, bool dw) const
{
	// and the INH F/F shifts each dot twice
	return ((x - m_vlt_x) >> (dw ? 1 : 0)) % timing().width;
}

bool vt220a_state::in_row(int x, bool dw) const
{
	// CBLANK H blanks the video (TM 6.2.3) for the whole retrace (CRT9007 datasheet, pin 35), but the
	// CRT9007 model's CBLANK covers only sync: blank outside the row's characters here
	const int col = video_col(x, dw);
	return col >= 0 && col < (timing().cols >> (dw ? 1 : 0));
}

u8 vt220a_state::vram_r(offs_t offset)
{
	// screen and attribute RAM see the same address; during VLT of a data row's top scan line the
	// row goes into the line buffer, if the CPU has granted the bus (DMA L, P1.0, drives TSC high)
	const u8 chr = m_screen_ram[offset & 0x0fff];
	if (m_vlt && !BIT(m_p1, 0) && !BIT(m_p1, 3) && !BIT(m_conf, 2) && !machine().side_effects_disabled())
	{
		const int col = video_col(m_screen->hpos(), m_dw);
		if (!m_drb && !m_row_loading)
		{
			// top scan line of a data row: it is painted live as the line buffer loads
			m_screen->update_partial(m_screen->vpos() - 1);
			m_row_loading = true;
		}
		if (col >= 0 && col < 0x100)
		{
			m_lbf[lbf_addr(true, col)] = chr;
			m_lbf[lbf_addr(false, col)] = m_attr_ram[offset & 0x0fff];
		}
	}
	return chr;
}

void vt220a_state::vs_w(int state)
{
	// a scan line that had no VLT in the frame now ending shows nothing; the others keep their SL and
	// double width until their next VLT, as a mid-line partial update may draw them before it
	if (!state)
	{
		for (int y = 0; y < VTOTAL; y++)
		{
			if (!m_line_vlt[y])
			{
				m_line_sl[y] = 0xff;
				m_line_dw[y] = 0;
			}
		}
		std::fill_n(m_line_vlt, VTOTAL, false);
	}
}

void vt220a_state::vlt_w(int state)
{
	if (state && !m_vlt)
	{
		// the chargen scan line comes from the CRT9007's SL3-SL0, set during the retrace before
		const int y = m_screen->vpos();
		m_vlt_x = m_screen->hpos();
		m_line_sl[y] = m_sl;
		m_line_dw[y] = m_dw;
		m_line_vlt[y] = true;
	}
	else if (!state && m_vlt)
	{
		// CGBLANK resets the cursor skew F/F at the start of each retrace
		m_dw = false;
	}
	m_vlt = state;
}

void vt220a_state::sl_w(u8 data)
{
	m_sl = data;
}

void vt220a_state::curs_w(int state)
{
	// CURS H outside VLT sets the cursor skew F/F for a double width line (TM 6.2.6.1)
	if (state && !m_vlt)
		m_dw = true;
}

void vt220a_state::drb_w(int state)
{
	// DRB is active low for the scan line that loads a data row
	m_drb = state;
	if (state)
		m_row_loading = false;
}

void vt220a_state::cblank_w(int state)
{
	m_cblank = state;
}

u8 vt220a_state::char_row(bool alt, u8 chr, int line) const
{
	// ALT SEL H selects the soft character RAM; otherwise CHRG SEL H (P1.5) is the ROM's MSB
	return alt ? m_softchar[soft_addr(chr, line)] : m_chargen[(BIT(m_p1, 5) << 12) | (chr << 4) | line];
}

u8 vt220a_state::dot(int col, int x, int line, bool &bold, bool &blink_n) const
{
	// one dot of the video stream: E30 gives the CPU the line buffer address and ALT SEL while
	// EN CHR GEN RW H is set, otherwise the video side walks the row
	const bool cpu = BIT(m_conf, 2);
	const u8 lba = cpu ? m_lba : u8(col);
	const u8 chr = lbf_out(true, lba);
	const u8 atr = lbf_out(false, lba);
	const bool alt = cpu ? BIT(m_conf, 3) : BIT(atr, 3);

	// AD7 BLINK (E38 NAND with DUART BLINK H), AD6 BOLD, AD5 REV (low for reverse), AD4 UL, AD3 ALT SEL
	bold = BIT(atr, 6);
	blink_n = !(BIT(m_duart_op, 3) && BIT(atr, 7));

	// past the character cell, or 0xff without VLT
	if (line >= 10)
		return 0;
	const u8 bits = char_row(alt, chr, line);

	// MSB first; each lit dot is stretched into the next (TM 6.2.4), so a lit dot 7 lights the first fill dot
	bool on = (x < 8 && BIT(bits, 7 - x)) || (x > 0 && x <= 8 && BIT(bits, 8 - x));
	if (BIT(atr, 4) && line >= 8) // UL ATR gated with SC3: scan lines 9 and 10
		on = true;
	// reverse: NORM FIELD H XOR REV ATR (E46)
	return (on ? 1 : 0) ^ BIT(m_conf, 0) ^ (BIT(atr, 5) ? 0 : 1);
}

u8 vt220a_state::video_out(int x, int y, bool &bold, bool &blink_n) const
{
	// the video stream at beam position x, y: nothing outside the row or with DIS VIDEO H (OP7)
	bold = blink_n = false;
	const bool dw = m_line_dw[y];
	if (!in_row(x, dw) || BIT(m_duart_op, 7))
		return 0;
	return dot(video_col(x, dw), video_dot(x, dw), m_line_sl[y], bold, blink_n);
}

u8 vt220a_state::duart_r(offs_t offset)
{
	if (offset == 0x0d && !machine().side_effects_disabled())
	{
		// self-test status taps on the video output (sheet 5): TBLK on IP4 is low while blanked
		// (CBLANK or DIS VIDEO), TVB H on IP3 is E45 NOR of the video and the two intensity lines.
		// Sampled when the CPU reads the input port, so the IPCR change bits only update then.
		const int x = m_screen->hpos(), y = m_screen->vpos();
		const bool blank = m_cblank || BIT(m_duart_op, 7) || !in_row(x, m_line_dw[y]); // OP7 DIS VIDEO H
		bool bold = false, blink_n = false;
		const bool video = !blank && video_out(x, y, bold, blink_n);
		m_duart->ip3_w(!(video || bold || blink_n));
		m_duart->ip4_w(!blank);
	}
	return m_duart->read(offset);
}

void vt220a_state::p1_w(u8 data)
{
	// LBF DISABLE H (P1.3) and CHRG SEL H (P1.5) change the video; DMA L (P1.0) changes with every row
	// load, whose top scan line is painted as it loads
	if ((m_p1 ^ data) & 0x28)
		m_screen->update_partial(m_screen->vpos());
	// DMA L (P1.0) falling edge loads the character address latch into the line buffer, only while
	// EN CHR GEN RW H gives the access mux to the CPU (TM 6.2.4.1); a row load's DMA grant doesn't
	if (BIT(m_p1, 0) && !BIT(data, 0) && !BIT(data, 3) && BIT(m_conf, 2))
	{
		m_lbf[lbf_addr(true, m_lba)] = m_char_addr;
		m_lbf[lbf_addr(false, m_lba)] = m_atr_latch;
	}
	// NVR RECALL L (P1.1) and NVR STORE L (P1.2); the X2212 core takes them active high, and a
	// STORE edge has no effect during RECALL (TM 4.2.9), so RECALL ending doesn't start one
	m_nvr->recall(!BIT(data, 1));
	if (BIT(m_p1, 2) != BIT(data, 2))
		m_nvr->store(!BIT(data, 2) && BIT(data, 1));
	// RESET CRT L (P1.6)
	if (BIT(m_p1, 6) && !BIT(data, 6))
		m_crtc->reset();
	m_p1 = data;
}

void vt220a_state::conf_w(u8 data)
{
	// E26 (LS174) latches BAD0-5, bit order per firmware self-test (reverse of the sheet 2 Q labels):
	// 0 NORM FIELD, 1 LBA CLRS, 2 EN CHR GEN RW, 3 ALT SEL, 4 RESET CMM, 5 LBA CLKS
	// E32 F/F latches BAD6: set for ENA 20mA, reset for ENA EIA
	// RESET CMM H resets the DUART
	if (BIT(data, 4) && !BIT(m_conf, 4))
		m_duart->reset();
	// E32 selects the host interface: E33 passes HOST TXD to the EIA driver only with ENA EIA,
	// and the receive gating takes EIA RXD only then (20 mA loop not emulated: idles marking)
	if (BIT(data, 6) != BIT(m_conf, 6))
	{
		if (BIT(data, 6))
			logerror("%s: 20 mA host loop selected, not emulated\n", machine().describe_context());
		write_host_txd(data);
		m_duart->rx_a_w(BIT(data, 6) ? 1 : m_host_rxd);
	}
	// with EN CHR GEN RW H set, E30 gives the CPU the line buffer address counter (E7, LS393)
	if (BIT(data, 2))
	{
		if (BIT(data, 1))
			m_lba = 0;
		else if (BIT(m_conf, 5) && !BIT(data, 5))
			m_lba++;
	}
	m_screen->update_partial(m_screen->vpos());
	m_conf = data;
}

void vt220a_state::host_txd_w(int state)
{
	m_host_txd = state;
	write_host_txd(m_conf);
}

void vt220a_state::host_rxd_w(int state)
{
	m_host_rxd = state;
	if (!BIT(m_conf, 6))
		m_duart->rx_a_w(state);
}

u8 vt220a_state::p1_r()
{
	// P1.7 CAR DET from the EIA receiver, low when asserted; the rest are outputs or the PROD TEST pull-up
	return 0x7f | (m_host->dcd_r() << 7);
}

void vt220a_state::duart_out_w(u8 data)
{
	m_screen->update_partial(m_screen->vpos());
	m_duart_op = data;
	if (BIT(data, 4) != m_132col)
	{
		m_132col = BIT(data, 4); // OP4 132 COL H
		set_columns();
	}
}

void vt220a_state::set_columns()
{
	const col_timing &t = timing();
	const int htotal = t.htotal * t.width;
	const int hstart = t.hdelay * t.width;
	m_screen->configure(htotal, VTOTAL, rectangle(hstart, hstart + t.cols * t.width - 1, VSTART, VSTART + VVIS - 1), attotime::from_ticks(htotal * VTOTAL, t.dot));
	m_crtc->set_unscaled_clock(t.dot / t.width);
	m_crtc->set_character_width(t.width);
}

void vt220a_state::char_addr_w(u8 data)
{
	// E19 latches the character address from BAD0-7 and E21 the attribute from AD3-7, only with
	// CPU access of the video logic enabled by EN CHR GEN RW H (TM table 4-2)
	if (BIT(m_conf, 2))
	{
		m_char_addr = data;
		m_atr_latch = data & 0xf8;
	}
}

u8 vt220a_state::chargen_r(offs_t offset)
{
	// enabled by EN CHR GEN RW H (TM table 4-2), otherwise nothing drives the bus
	if (!BIT(m_conf, 2))
		return 0xff;
	return char_row(BIT(m_conf, 3), lbf_out(true, m_lba), offset);
}

void vt220a_state::chargen_w(offs_t offset, u8 data)
{
	if (BIT(m_conf, 3) && BIT(m_conf, 2))
	{
		m_screen->update_partial(m_screen->vpos());
		m_softchar[soft_addr(lbf_out(true, m_lba), offset)] = data;
	}
}

/* Input ports */
static INPUT_PORTS_START( vt220 )
INPUT_PORTS_END

void vt220_state::machine_start()
{
	save_item(NAME(m_kbd_rxd));
}

void vt220a_state::machine_start()
{
	vt220_state::machine_start();

	std::fill_n(m_line_sl, VTOTAL, 0xff);

	save_item(NAME(m_p1));
	save_item(NAME(m_conf));
	save_item(NAME(m_duart_op));
	save_item(NAME(m_char_addr));
	save_item(NAME(m_atr_latch));
	save_item(NAME(m_lba));
	save_item(NAME(m_132col));
	save_item(NAME(m_vlt_x));
	save_item(NAME(m_sl));
	save_item(NAME(m_dw));
	save_item(NAME(m_line_sl));
	save_item(NAME(m_line_dw));
	save_item(NAME(m_line_vlt));
	save_item(NAME(m_vlt));
	save_item(NAME(m_drb));
	save_item(NAME(m_row_loading));
	save_item(NAME(m_cblank));
	save_item(NAME(m_host_txd));
	save_item(NAME(m_host_rxd));
	save_item(NAME(m_softchar));
	save_item(NAME(m_lbf));

	// TM table 5-2: the printer port DTR and RTS are on whenever the terminal is powered
	m_printer->write_dtr(0);
	m_printer->write_rts(0);
}

void vt220a_state::machine_reset()
{
	m_dw = false;
	m_row_loading = false;
}

uint32_t vt220_state::screen_update_vt220(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

u32 vt220a_state::screen_update_vt220a(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		u16 *const pix = &bitmap.pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			bool bold, blink_n;
			pix[x] = video_out(x, y, bold, blink_n) ? 1 + (blink_n ? 1 : 0) + (bold ? 2 : 0) : 0;
		}
	}
	return 0;
}

void vt220a_state::vt220a_palette(palette_device &palette) const
{
	// E49 open-collector NANDs of the video with TBLK L into R17 1.8K, with BLINK ATR into R15 2.7K and
	// with BOLD ATR into R16 2K (sheet 5); intensity taken as the summed conductance
	constexpr double G[4] = { 1 / 1.8, 1 / 1.8 + 1 / 2.7, 1 / 1.8 + 1 / 2.0, 1 / 1.8 + 1 / 2.7 + 1 / 2.0 };
	palette.set_pen_color(0, rgb_t::black());
	for (int i = 0; i < 4; i++)
	{
		const u8 v = u8(255 * G[i] / G[3]);
		palette.set_pen_color(1 + i, rgb_t(v, v, v));
	}
}


void vt220_state::vt220_common(machine_config &config)
{
	I8051(config, m_maincpu, 11.0592_MHz_XTAL); // from schematic for earlier version

	// LK201 on the 8051 UART, through EIA drivers and receivers at both
	// ends (TM 4.2.8 for models A-C, schematic 54-17036 for D-F)
	m_maincpu->port_in_cb<3>().set([this] () -> u8 { return 0xfe | m_kbd_rxd; }); // P3.0 RXD from KBR
	m_maincpu->port_out_cb<3>().set(m_kbd, FUNC(lk201_device::rx_w)).bit(1); // P3.1 TXD to KBT
	LK201(config, m_kbd);
	m_kbd->tx_handler().set([this] (int state) { m_kbd_rxd = state; });

	SCN2681(config, m_duart, 3.6864_MHz_XTAL);
	m_duart->irq_cb().set_inputline(m_maincpu, MCS51_INT1_LINE);
}

void vt220_state::vt220(machine_config &config)
{
	vt220_common(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &vt220_state::vt220_mem);
	m_maincpu->set_addrmap(AS_DATA, &vt220_state::vt220_data);
	m_maincpu->port_in_cb<1>().set_constant(0); // ???

	screen_device &screen(SCREEN(config, m_screen));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(vt220_state::screen_update_vt220));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);
}

void vt220a_state::vt220a(machine_config &config)
{
	vt220_common(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &vt220a_state::vt220a_mem);
	m_maincpu->set_addrmap(AS_DATA, &vt220a_state::vt220a_data);

	// P1.4 PROD TEST is low only with the factory fixture on J1-1
	m_maincpu->port_in_cb<1>().set(FUNC(vt220a_state::p1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(vt220a_state::p1_w));

	// T5 character clock: dot clock / dots per character; set_columns switches on 132 COL H (OP4), which the
	// DUART's reset sets high (OPR cleared) until the firmware writes OPR
	constexpr col_timing t = TIMING_80;
	CRT9007(config, m_crtc, t.dot / t.width);
	m_crtc->set_addrmap(0, &vt220a_state::vt220a_vram);
	m_crtc->set_character_width(t.width);
	m_crtc->set_screen(m_screen);
	SCREEN(config, m_screen);
	m_screen->set_raw(t.dot, t.htotal * t.width, t.hdelay * t.width, (t.hdelay + t.cols) * t.width, VTOTAL, VSTART, VSTART + VVIS);
	// VS and HS are active low (CRT9007 pins 12, 13) and wired straight to T0 and T1; the 8051 counts their
	// falling edges, and an asserted mcs51 input line is a low pin
	m_crtc->vs_callback().set_inputline(m_maincpu, MCS51_T0_LINE).invert();
	m_crtc->vs_callback().append(FUNC(vt220a_state::vs_w));
	m_crtc->hs_callback().set_inputline(m_maincpu, MCS51_T1_LINE).invert();
	m_crtc->drb_callback().set(FUNC(vt220a_state::drb_w));
	m_crtc->drb_callback().append_inputline(m_maincpu, MCS51_INT0_LINE).invert(); // DMA REQ L
	m_crtc->vlt_callback().set(FUNC(vt220a_state::vlt_w));
	m_crtc->sl_callback().set(FUNC(vt220a_state::sl_w));
	m_crtc->curs_callback().set(FUNC(vt220a_state::curs_w));
	m_crtc->cblank_callback().set(FUNC(vt220a_state::cblank_w));
	X2212(config, m_nvr); // E17

	// TM figure 5-4: channel A host (J3, EIA or 20 mA), channel B printer.  Modem signals are low when
	// asserted at the DUART pins, as on the RS-232 port: the firmware drops DTR by driving OP1 high, and
	// reads DSR, SPD IND and PTR DSR asserted as input port bits low (opposite to the TM's H signal
	// names; the sense here comes from the firmware)
	m_duart->outport_cb().set(FUNC(vt220a_state::duart_out_w));
	m_duart->outport_cb().append(m_host, FUNC(rs232_port_device::write_rts)).bit(0); // OP0 RTS
	m_duart->outport_cb().append(m_host, FUNC(rs232_port_device::write_dtr)).bit(1); // OP1 DTR
	m_duart->outport_cb().append(m_host, FUNC(rs232_port_device::write_spds)).bit(2); // OP2 SPD SEL
	m_duart->a_tx_cb().set(FUNC(vt220a_state::host_txd_w));
	m_duart->b_tx_cb().set(m_printer, FUNC(rs232_port_device::write_txd));

	RS232_PORT(config, m_host, default_rs232_devices, nullptr);
	m_host->rxd_handler().set(FUNC(vt220a_state::host_rxd_w));
	m_host->cts_handler().set(m_duart, FUNC(scn2681_device::ip0_w)); // CTS
	m_host->dsr_handler().set(m_duart, FUNC(scn2681_device::ip1_w)); // DSR
	m_host->si_handler().set(m_duart, FUNC(scn2681_device::ip2_w)); // SPD IND

	RS232_PORT(config, m_printer, default_rs232_devices, nullptr);
	m_printer->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_b_w));
	m_printer->dsr_handler().set(m_duart, FUNC(scn2681_device::ip5_w)); // PTR DSR

	m_screen->set_screen_update(FUNC(vt220a_state::screen_update_vt220a));
	m_screen->set_palette("palette");
	PALETTE(config, "palette", FUNC(vt220a_state::vt220a_palette), 5);
}

/* ROM definitions */
ROM_START(vt220)
	ROM_REGION(0x8000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("23-178e6.bin", 0x0000, 0x8000, CRC(cce5088c) SHA1(4638304729d1213658a96bb22c5211322b74d8fc) )

	ROM_REGION(0x4000, "chargen", ROMREGION_ERASEFF)
	ROM_LOAD("23-348e4.e13", 0x0000, 0x2000, CRC(994f3e37) SHA1(fe72a9fe9adb3a24743a6288d88ae07570cfea9a) )
ROM_END

ROM_START(vt220a)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
#if 0
	// these ROMs are listed in the schematics; it is unknown whether or not they were actually shipped
	ROM_LOAD("23-012e5.e3", 0x0000, 0x4000, NO_DUMP) // location e3 with jumper w7 present and w6 cut
	ROM_RELOAD(0x4000, 0x4000) // the rom also maps to 4000-7fff as a14 is unconnected
	ROM_LOAD("23-248e4.e4", 0x8000, 0x2000, NO_DUMP)
	// a000-bfff may be open bus or may be a mirror of above depending on whether the rom uses PGM/A13 as a secondary enable or not
	ROM_RELOAD(0xc000, 0x2000)
	// e000-ffff may be open bus or may be a mirror of above depending on whether the rom uses PGM/A13 as a secondary enable or not
#endif
	ROM_LOAD("23-183e5.e3", 0x8000, 0x4000, CRC(2848db40) SHA1(b3b029ef964c86ede68ad1b2854cfad766f20af0))
	ROM_RELOAD(0xc000, 0x4000)
	ROM_LOAD("23-182e5.e4", 0x0000, 0x4000, CRC(c759bf9f) SHA1(6fe21e8eb9576fbcda76d7909b07859db0793c4e))
	ROM_RELOAD(0x4000, 0x4000)
	ROM_LOAD("23-011m1.e1", 0x0000, 0x1000, CRC(6c4930a9) SHA1(1200a5dbf431017a11a51b5c4c9b4e7952b0a2bb)) // 8051 internal code

	ROM_REGION(0x4000, "chargen", ROMREGION_ERASEFF)
#if 0
	// this ROM is listed in the schematics
	ROM_LOAD("23-247e4.e13", 0x0000, 0x2000, NO_DUMP)
#endif
	ROM_LOAD("23-348e4.e13", 0x0000, 0x2000, CRC(994f3e37) SHA1(fe72a9fe9adb3a24743a6288d88ae07570cfea9a)) // CPU reads at 9800 (chargen_r), the soft character RAM there with ALT SEL
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  STATE         INIT        COMPANY                          FULLNAME               FLAGS */
COMP( 1983, vt220,  0,      0,      vt220,   vt220, vt220_state,  empty_init, "Digital Equipment Corporation", "VT220 (Version 2.3)", MACHINE_NOT_WORKING )
COMP( 1983, vt220a, vt220,  0,      vt220a,  vt220, vt220a_state, empty_init, "Digital Equipment Corporation", "VT220 (Version 2.1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

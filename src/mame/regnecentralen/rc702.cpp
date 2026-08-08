// license:BSD-3-Clause
// copyright-holders:Robbbert, Thorbjørn Ravn Andersen
/******************************************************************************************************************

Regnecentralen Piccolo RC702/RC703

2016-09-10 Skeleton driver
2026-07-30 RC702 flavor boots to CP/M and ID-COMAL.

Undumped prom at IC55 type 74S287 (address decoder for PROM0/PROM1 mapping)
Keyboard has 8048 and 2758, both undumped.

Machine variants:
  rc702      - RC702, 8" DSDD (maxi), 8 MHz FDC
  rc702mini  - RC702, 5.25" DD (mini), 4 MHz FDC
  rc703      - RC703, 5.25" QD (80-track), 4 MHz FDC

ToDo:
- Hard drive for RC703, ports 0x60-0x67. Extra CTC on HD board, ports 0x44-0x47
- Keyboard MCU (8048 + 2758) -- currently using generic_keyboard

SIO peripherals (J1 / J2):
- SIO-A (J1): data/reader port, mapped to RDR:/PUN: in CP/M.
- SIO-B (J2): traditionally the printer port; also usable for CP/NET traffic to
  an MP/M server on the host (the alternative to PIO-B).
  Both channels are RS232 (null_modem slot).  No baud-rate default is set here:
  the firmware programs the CTC baud generator at cold boot (typically 38400
  8-N-1), so match the null_modem slot rate to the firmware in MAME's options.
  Each SIO channel's TxC and RxC are tied together to share one CTC output, so
  TX and RX on a channel always run at the same rate (see clock tree below).
  The pins for synchronous operation are not exposed on the RC702 connectors,
  so synchronous mode is only possible on the RC703.

PIO peripherals (J3 / J4):
- PIO-A (J4): keyboard, wired direct.
- PIO-B (J3): configurable slot device (pio_port/), default empty.
  J3 was an unpopulated expansion connector. Not the printer port, that was SIO-B.

Z80 PIO modeling: PIO-A is wired direct and PIO-B is the slot ("Einstein
topology", after src/mame/tatung/einstein.cpp).  Making both ports of one
z80pio_device slot devices breaks IM2 interrupt delivery: the flat
z80pio_device has no per-channel device_t subdevices, and slots have only
been validated against per-channel-subdevice chips (z80sio_channel etc.).

prom1 (line program ROM) is undumped; region filled with 0xff.  A typical use
was a test PROM in the RC703.  A CP/NOS boot prom1 for diskless operation is in
progress.

References:

*  Latest technical manual for both RC702 and RC703:  https://github.com/ravn/rc700-gensmedet/blob/main/docs/RC702-RC703_Microcomputer_technical_manual.pdf

*  Manuals and emulator: https://www.jbox.dk/rc702/manuals.sht
   (https://web.archive.org/web/20250814205631/https://www.jbox.dk/rc702/manuals.shtm)

*   Diskette image archive: https://ddhf.dk/wiki/Bits:Keyword/RC/RC700

*   Refactoring firmware and bios: https://github.com/ravn/rc700-gensmedet


****************************************************************************************************************/

#include "emu.h"

#include "pio_port/pio_port.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/z80daisy.h"
#include "machine/7474.h"
#include "machine/am9517a.h"
#include "machine/clock.h"
#include "machine/keyboard.h"
#include "machine/upd765.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "sound/beep.h"
#include "video/i8275.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "rc702.lh"


namespace {

// Background-colour margin (palette pen 0 = dark amber) added around the
// active i8275 raster on all four sides.  The margin is baked into the
// screen bitmap via bitmap.fill(0) + coordinate offset in display_pixels,
// so it is renderer-independent (SDL and BGFX both show the correct colour
// without relying on .lay artwork).
static constexpr int CRTC_BORDER = 8;

// RC702 clock tree (see RC702_HARDWARE_TECHNICAL_REFERENCE.md):
//   * 8 MHz crystal          -> Z80 CPU and the CTC/SIO/PIO + Am9517A DMA all
//     run at 4 MHz (/2); the uPD765 FDC runs at the full 8 MHz on the 8" drive
//     and at 4 MHz (/2) on the 5.25" drives.
//   * 19.6608 MHz memory (DRAM) clock -> baud rate generator: two cascaded
//     74LS393 dividers give /32 = 0.6144 MHz into CTC ch0/ch1, so the maximum
//     async rate is 614400 / 16 = 38400 baud.
//   * 11.64 MHz dot clock    -> 8275 CRTC character clock (/7 = 7 px/char).
//     This is a PLL output, not a crystal: a phase-locked loop regenerates it
//     locked to the 50 Hz field rate, giving 108 chars x 7 px x 15.4 kHz
//     (308 lines x 50 Hz) = 11.6424 MHz.  So it is a plain derived frequency
//     here, not an XTAL.
// The 4 MHz peripheral clock (8 MHz / 2) is the value repeated below.
static constexpr XTAL MAIN_XTAL  = 8_MHz_XTAL;        // CPU + peripherals + FDC
static constexpr XTAL MEM_CLOCK  = 19.6608_MHz_XTAL;  // DRAM clock; baud = /32
static constexpr int  DOT_CLOCK  = 11'640'000;        // 8275 PLL dot clock; char clock = /7

class rc702_state : public driver_device
{
public:
	rc702_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_rom_prom1(*this, "prom1")
		, m_ram(*this, "mainram")
		, m_bank1(*this, "bank1")
		, m_bank1h(*this, "bank1h")
		, m_bank2(*this, "bank2")
		, m_bank2h(*this, "bank2h")
		, m_p_chargen(*this, "chargen")
		, m_crtc(*this, "crtc")
		, m_ctc1(*this, "ctc1")
		, m_sio(*this, "sio1")
		, m_pio(*this, "pio")
		, m_dma(*this, "dma")
		, m_beep(*this, "beeper")
		, m_7474(*this, "7474")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_rs232a(*this, "rs232a")
		, m_rs232b(*this, "rs232b")
		, m_pio_b(*this, "piob")
		, m_screen(*this, "screen")
		, m_keyboard(*this, "keyboard")
		, m_dsw(*this, "DSW")
		, m_promcfg(*this, "PROMCFG")
	{ }

	void rc700_base(machine_config &config) ATTR_COLD;
	void rc702(machine_config &config) ATTR_COLD;
	void rc702mini(machine_config &config) ATTR_COLD;
	void rc703(machine_config &config) ATTR_COLD;
	void rc702sem702(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void add_fdc_dma(machine_config &config);
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);
	void port14_w(uint8_t data);
	void port1c_w(uint8_t data);
	void sem702_char_w(uint8_t data);
	void sem702_dot_w(uint8_t data);
	void sem702_data_w(uint8_t data);
	void crtc_drq_w(int state);
	void hreq_w(int state);
	void clock_w(int state);
	void eop_w(int state);
	void q_w(int state);
	void qbar_w(int state);
	void dack1_w(int state);
	void dack2_w(int state);
	I8275_DRAW_CHARACTER_MEMBER(display_pixels);
	uint32_t screen_update_with_border(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void rc702_palette(palette_device &palette) const;
	void kbd_put(u8 data);
	uint8_t kbd_r();

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	uint8_t m_kbd_data = 0U;
	bool m_q_state = false;
	bool m_qbar_state = false;
	bool m_drq_state = false;
	uint16_t m_beepcnt = 0U;
	bool m_eop = false;
	bool m_dack1 = false;
	// 8237 DACK2 real-line voltage; HIGH (=1) means ch2 inactive.  Fed with
	// m_eop into the 74LS32/74LS74 roll-function logic (see eop_w).
	bool m_dack2 = true;
	// SEM702 RAM-based character generator (IC82 swap-in for ROA327),
	// 128 chars * 16 lines = 2 KB, programmed via ports 0xD1/0xD2/0xD3.
	uint8_t m_sem702_ram[0x800] = {};
	uint8_t m_sem702_char_latch = 0U;
	uint8_t m_sem702_dot_latch = 0U;
	bool m_has_sem702 = false;

	required_device<palette_device> m_palette;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_region_ptr<u8> m_rom_prom1;
	required_shared_ptr<u8> m_ram;
	required_memory_bank    m_bank1;
	required_memory_bank    m_bank1h;
	required_memory_bank    m_bank2;
	required_memory_bank    m_bank2h;
	required_region_ptr<u8> m_p_chargen;
	required_device<i8275_device> m_crtc;
	required_device<z80ctc_device> m_ctc1;
	required_device<z80sio_device> m_sio;
	required_device<z80pio_device> m_pio;
	required_device<am9517a_device> m_dma;
	required_device<beep_device> m_beep;
	required_device<ttl7474_device> m_7474;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<rs232_port_device> m_rs232a;
	required_device<rs232_port_device> m_rs232b;
	required_device<rc702_pio_port_device> m_pio_b;
	required_device<screen_device> m_screen;
	required_device<generic_keyboard_device> m_keyboard;
	required_ioport m_dsw;
	required_ioport m_promcfg;
};


void rc702_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share(m_ram);
	map(0x0000, 0x07ff).bankr(m_bank1);
	map(0x0800, 0x0fff).bankr(m_bank1h);
	map(0x2000, 0x27ff).bankr(m_bank2);
	map(0x2800, 0x2fff).bankr(m_bank2h);
}

void rc702_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x01).rw(m_crtc, FUNC(i8275_device::read), FUNC(i8275_device::write));
	map(0x04, 0x05).m(m_fdc, FUNC(upd765a_device::map));
	map(0x08, 0x0b).rw(m_sio, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w)); // boot sequence doesn't program this
	map(0x0c, 0x0f).rw(m_ctc1, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x10, 0x13).rw(m_pio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x14, 0x17).portr(m_dsw).w(FUNC(rc702_state::port14_w)); // motors
	map(0x18, 0x1b).lw8(NAME([this] (u8 data) { m_bank1->set_entry(0); m_bank1h->set_entry(0); m_bank2->set_entry(0); m_bank2h->set_entry(0); })); // replace roms with ram
	map(0x1c, 0x1f).w(FUNC(rc702_state::port1c_w)); // sound
	// SEM702 chargen (IC82) ports; only present on the SEM702 variant, where
	// they land on a RAM board fitted in place of the ROA327 font ROM.
	if (m_has_sem702)
	{
		map(0xd1, 0xd1).w(FUNC(rc702_state::sem702_char_w));
		map(0xd2, 0xd2).w(FUNC(rc702_state::sem702_dot_w));
		map(0xd3, 0xd3).w(FUNC(rc702_state::sem702_data_w));
	}
	map(0xf0, 0xff).rw(m_dma, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
}

// PROM socket type: 2716 (2KB) or 2732 (4KB), selected by a socket pin-21
// jumper (+5V Vpp vs A11).  The 2732 option is only on later board
// revisions.  (RC702 tech manual p.63.)
static INPUT_PORTS_START( rc702_promcfg )
	PORT_START("PROMCFG")
	PORT_CONFNAME( 0x01, 0x00, "PROM0 (IC66) Type")
	PORT_CONFSETTING(    0x00, "2716 (2KB)")
	PORT_CONFSETTING(    0x01, "2732 (4KB)")
	PORT_CONFNAME( 0x02, 0x02, "PROM1 (IC65) Type")
	PORT_CONFSETTING(    0x00, "2716 (2KB)")
	PORT_CONFSETTING(    0x02, "2732 (4KB)")
INPUT_PORTS_END

/* Input ports - PROM reads port 0x14 bit 7: set=mini, clear=maxi. */
static INPUT_PORTS_START( rc702_maxi )
	PORT_INCLUDE( rc702_promcfg )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "S01")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S02")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S03")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S04")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S05")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S06")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S07")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "S08 Minifloppy")
	PORT_DIPSETTING(    0x80, DEF_STR( On ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
INPUT_PORTS_END

/* Same as rc702_maxi but S08 (Minifloppy) defaults On. */
static INPUT_PORTS_START( rc702_mini )
	PORT_INCLUDE( rc702_maxi )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x80, 0x80, "S08 Minifloppy")
	PORT_DIPSETTING(    0x80, DEF_STR( On ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
INPUT_PORTS_END

void rc702_state::machine_reset()
{
	// PROM lower halves always map to ROM
	m_bank1->set_entry(1);
	m_bank2->set_entry(1);

	// PROM upper halves depend on jumper setting:
	// 2716 (2KB): A11 not connected -- upper half mirrors lower half
	// 2732 (4KB): A11 active -- upper half is distinct ROM data
	uint8_t promcfg = m_promcfg->read();
	m_bank1h->set_entry(BIT(promcfg, 0) ? 2 : 1);  // 4K: ROM+0x800, 2K: mirror
	m_bank2h->set_entry(BIT(promcfg, 1) ? 2 : 1);  // 4K: ROM+0x800, 2K: mirror

	m_beepcnt = 0xffff;
	m_dack1 = 0;
	m_eop = 0;
	// ch2 DACK starts inactive (real line HIGH); along with m_eop=0
	// (TC inactive after init) keeps the 74LS32 OR output HIGH at reset.
	m_dack2 = true;
	m_7474->preset_w(1);
	m_7474->d_w(1);          // D input tied to LOGICAL ONE (R35 pull-up)
	m_7474->clock_w(1);      // OR-gate output is HIGH at reset

	// Set FDC data rate: 8" maxi drives use 500 kbps, 5.25" mini use 250 kbps.
	// DIP switch S08 bit 7: clear = maxi (8"), set = mini (5.25").
	m_fdc->set_rate(BIT(m_dsw->read(), 7) ? 250'000 : 500'000);

	m_maincpu->reset();
}

void rc702_state::machine_start()
{
	// Lower halves: entry 0 = RAM, entry 1 = ROM
	m_bank1->configure_entry(0, m_ram);
	m_bank1->configure_entry(1, m_rom);
	m_bank2->configure_entry(0, &m_ram[0x2000]);
	m_bank2->configure_entry(1, m_rom_prom1);

	// Upper halves: entry 0 = RAM, entry 1 = mirror of lower ROM (2716), entry 2 = upper ROM (2732)
	m_bank1h->configure_entry(0, &m_ram[0x0800]);
	m_bank1h->configure_entry(1, &m_rom[0x0000]);        // 2KB mirror
	m_bank1h->configure_entry(2, &m_rom[0x0800]);        // 4KB upper half
	m_bank2h->configure_entry(0, &m_ram[0x2800]);
	m_bank2h->configure_entry(1, &m_rom_prom1[0x0000]);  // 2KB mirror
	m_bank2h->configure_entry(2, &m_rom_prom1[0x0800]);  // 4KB upper half
	save_item(NAME(m_q_state));
	save_item(NAME(m_qbar_state));
	save_item(NAME(m_drq_state));
	save_item(NAME(m_beepcnt));
	save_item(NAME(m_eop));
	save_item(NAME(m_dack1));
	save_item(NAME(m_dack2));
	save_item(NAME(m_kbd_data));

	if (m_has_sem702)
	{
		// Power-on SEM702 RAM is undefined; fill with 0xFF so an unprogrammed
		// boot shows solid blocks rather than a blank (working-looking) screen.
		// Software must load a font before enabling the CRT.
		std::memset(m_sem702_ram, 0xff, sizeof(m_sem702_ram));
		save_item(NAME(m_sem702_ram));
		save_item(NAME(m_sem702_char_latch));
		save_item(NAME(m_sem702_dot_latch));
	}
}

void rc702_state::q_w(int state)
{
	m_q_state = state;

	if (m_q_state && m_drq_state)
		m_dma->dreq3_w(1);
	else
		m_dma->dreq3_w(0);
}

void rc702_state::qbar_w(int state)
{
	m_qbar_state = state;

	if (m_qbar_state && m_drq_state)
		m_dma->dreq2_w(1);
	else
		m_dma->dreq2_w(0);
}

void rc702_state::crtc_drq_w(int state)
{
	m_drq_state = state;

	if (m_q_state && m_drq_state)
		m_dma->dreq3_w(1);
	else
		m_dma->dreq3_w(0);

	if (m_qbar_state && m_drq_state)
		m_dma->dreq2_w(1);
	else
		m_dma->dreq2_w(0);
}

void rc702_state::eop_w(int state)
{
	if (state == m_eop)
		return;

	m_eop = state;

	if (!m_eop && !m_dack1)
		m_fdc->tc_w(1);
	else
		m_fdc->tc_w(0);

	// 74LS32 OR gate (MIC 11): inputs /DACK2 + /TC (active-low).  Rising edge
	// at end of TC clocks the 74LS74 (D=1), switching CRTC DRQ routing from
	// ch2 to ch3 -- the "roll function".  See
	// https://github.com/ravn/rc700-gensmedet/blob/main/docs/dma_ch3_8275_roll_function.md
	m_7474->clock_w(m_dack2 || m_eop);
}

void rc702_state::dack1_w(int state)
{
	if (state == m_dack1)
		return;

	m_dack1 = state;

	if (!m_eop && !m_dack1)
		m_fdc->tc_w(1);
	else
		m_fdc->tc_w(0);

	//m_fdc->dack_w = state;  // pin not emulated
}

void rc702_state::dack2_w(int state)
{
	if (state == m_dack2)
		return;

	m_dack2 = state;

	// 74LS32 OR gate on MIC 11 -- see comment in eop_w() above.
	m_7474->clock_w(m_dack2 || m_eop);
}

void rc702_state::port14_w(uint8_t data)
{
	// Mini floppy motor: bit 0 = 1 starts, 0 stops (no-op on always-spinning
	// 8" maxi drives).  Do NOT call set_floppy() -- the connector already binds
	// flopi[0]; set_floppy() would bind it to all 4 slots and deadlock the PROM
	// with 4 spurious ready-change interrupts per drive event.
	floppy_image_device *floppy = m_floppy0->get_device();
	if (floppy)
		floppy->mon_w(!BIT(data, 0));
}

void rc702_state::port1c_w(uint8_t data)
{
	m_beep->set_state(1);
	m_beepcnt = 0x3000;
}

// SEM702 chargen (IC82).  The handlers are only installed on the SEM702
// variant (see io_map).  Three pure latches: 0xD1 = 7-bit char address
// (ACHAR), 0xD2 = 4-bit line address (ALINE), 0xD3 writes
// RAM[(ACHAR << 4) | ALINE].  No auto-increment is modelled -- all known
// software sets ALINE before every byte, so real behaviour is unobserved
// and the strict model avoids relying on side effects.
void rc702_state::sem702_char_w(uint8_t data)
{
	m_sem702_char_latch = data & 0x7f;
}

void rc702_state::sem702_dot_w(uint8_t data)
{
	m_sem702_dot_latch = data & 0x0f;
}

void rc702_state::sem702_data_w(uint8_t data)
{
	uint16_t addr = (uint16_t(m_sem702_char_latch) << 4) | m_sem702_dot_latch;
	m_sem702_ram[addr & 0x7ff] = data;
}

// monitor is orange even when powered off
void rc702_state::rc702_palette(palette_device &palette) const
{
	// RC752 (NEC JB-1201M(A)) amber monitor: dark warm-brown background, soft
	// amber foreground.  Colors sampled from the jbox (Ringgaard) emulator.
	palette.set_pen_color(0, rgb_t(0x4f, 0x25, 0x09));  // background: dark brown
	palette.set_pen_color(1, rgb_t(0xc4, 0x9b, 0x47));  // foreground: soft amber
}

I8275_DRAW_CHARACTER_MEMBER( rc702_state::display_pixels )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint8_t gfx = 0;

	using namespace i8275_attributes;

	if (!BIT(attrcode, VSP))
	{
		// GPA0 from field attribute selects ROA327 (semigraphics) vs ROA296 (main chargen).
		// On the SEM702 variant, the ROA327 half of the chargen address
		// space is backed by RAM that the host programs via 0xD1/0xD2/0xD3.
		uint16_t offset = (linecount & 15) | (charcode << 4);
		if (BIT(attrcode, GPA0))
		{
			if (m_has_sem702)
				gfx = m_sem702_ram[offset & 0x7ff];
			else
				gfx = m_p_chargen[offset | 0x800];
		}
		else
		{
			gfx = m_p_chargen[offset];
		}
	}

	if (BIT(attrcode, LTEN))
		gfx = 0xff;

	if (BIT(attrcode, RVV))
		gfx ^= 0xff;

	// Highlight not used in RC702, possibly fixed in RC703.
	// Bits 0-6 are the 7 visible pixels (bit 7 unused in both ROA296 and ROA327 ROMs).
	// Offset by CRTC_BORDER so the outer margin stays as palette[0] (background).
	x += CRTC_BORDER;
	int const py = y + CRTC_BORDER;
	bitmap.pix(py, x++) = palette[BIT(gfx, 0) ? 1 : 0];
	bitmap.pix(py, x++) = palette[BIT(gfx, 1) ? 1 : 0];
	bitmap.pix(py, x++) = palette[BIT(gfx, 2) ? 1 : 0];
	bitmap.pix(py, x++) = palette[BIT(gfx, 3) ? 1 : 0];
	bitmap.pix(py, x++) = palette[BIT(gfx, 4) ? 1 : 0];
	bitmap.pix(py, x++) = palette[BIT(gfx, 5) ? 1 : 0];
	bitmap.pix(py, x++) = palette[BIT(gfx, 6) ? 1 : 0];
}

uint32_t rc702_state::screen_update_with_border(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->palette()->entry_list_raw()[0], cliprect);
	return m_crtc->screen_update(screen, bitmap, cliprect);
}

// Baud rate generator.  The 0.6144 MHz CTC clock is derived from the
// 19.6608 MHz DRAM (memory) clock by a /32 divider chain on MIC 11 (see the
// clock tree at the top).  With the CTC in /16 mode the maximum baud rate is
// 38400 bps.
void rc702_state::clock_w(int state)
{
	m_ctc1->trg0(state);
	m_ctc1->trg1(state);
	if (m_beepcnt == 0)
		m_beep->set_state(0);
	if (m_beepcnt < 0xfe00)
		m_beepcnt--;
}

void rc702_state::hreq_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	m_dma->hack_w(state); // tell dma that bus has been granted
}

uint8_t rc702_state::memory_read_byte(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void rc702_state::memory_write_byte(offs_t offset, uint8_t data)
{
	m_maincpu->space(AS_PROGRAM).write_byte(offset,data);
}

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "ctc1" },
	{ "sio1" },
	{ "pio" },
	{ nullptr }
};

void rc702_state::kbd_put(u8 data)
{
	m_kbd_data = data;
	m_pio->strobe_a(0);
	m_pio->strobe_a(1);
}

uint8_t rc702_state::kbd_r()
{
	return m_kbd_data;
}


static void rc702_floppies(device_slot_interface &device)
{
	device.option_add("8dsdd", FLOPPY_8_DSDD);
}

static void rc702mini_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

static void rc703_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void rc702_state::rc700_base(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MAIN_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &rc702_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &rc702_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain_intf);

	CLOCK(config, "ctc_clock", MEM_CLOCK / 32).signal_handler().set(FUNC(rc702_state::clock_w));

	Z80CTC(config, m_ctc1, MAIN_XTAL / 2);
	m_ctc1->zc_callback<0>().set(m_sio, FUNC(z80sio_device::txca_w));
	m_ctc1->zc_callback<0>().append(m_sio, FUNC(z80sio_device::rxca_w));
	m_ctc1->zc_callback<1>().set(m_sio, FUNC(z80sio_device::rxtxcb_w));
	m_ctc1->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80SIO(config, m_sio, MAIN_XTAL / 2);
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_sio->out_txda_callback().set(m_rs232a, FUNC(rs232_port_device::write_txd));
	m_sio->out_rtsa_callback().set(m_rs232a, FUNC(rs232_port_device::write_rts));
	m_sio->out_dtra_callback().set(m_rs232a, FUNC(rs232_port_device::write_dtr));
	m_sio->out_txdb_callback().set(m_rs232b, FUNC(rs232_port_device::write_txd));
	m_sio->out_rtsb_callback().set(m_rs232b, FUNC(rs232_port_device::write_rts));
	m_sio->out_dtrb_callback().set(m_rs232b, FUNC(rs232_port_device::write_dtr));

	// SIO-A (J1): data/reader port - mapped to RDR:/PUN: in CP/M.
	RS232_PORT(config, m_rs232a, default_rs232_devices, "null_modem");
	m_rs232a->rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
	m_rs232a->cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));
	m_rs232a->dcd_handler().set(m_sio, FUNC(z80sio_device::dcda_w));

	// SIO-B (J2): traditionally a printer port.  Here it is wired to a
	// null_modem so the guest can talk to an MP/M server running on the
	// host -- CP/NET traffic can go over either this SIO channel or PIO-B.
	// No baud-rate default is set; the firmware programs the CTC baud
	// generator at cold boot, so match the null_modem slot to the firmware
	// rate (typically 38400 8-N-1) in MAME's slot options.
	RS232_PORT(config, m_rs232b, default_rs232_devices, "null_modem");
	m_rs232b->rxd_handler().set(m_sio, FUNC(z80sio_device::rxb_w));
	m_rs232b->cts_handler().set(m_sio, FUNC(z80sio_device::ctsb_w));
	m_rs232b->dcd_handler().set(m_sio, FUNC(z80sio_device::dcdb_w));

	Z80PIO(config, m_pio, MAIN_XTAL / 2);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	// PIO-A: keyboard, wired directly for now (see Einstein-topology note at top).
	m_pio->in_pa_callback().set(FUNC(rc702_state::kbd_r));
	// PIO-B: configurable slot device, default empty (J3 expansion connector).
	m_pio->in_pb_callback().set(m_pio_b, FUNC(rc702_pio_port_device::read));
	m_pio->out_pb_callback().set(m_pio_b, FUNC(rc702_pio_port_device::write));
	m_pio->out_brdy_callback().set(m_pio_b, FUNC(rc702_pio_port_device::rdy_w));

	// PIO-B slot: out_strobe_handler lets cards pulse the chip's STB input.
	// The 3-arg (no-clock) ctor registers the card list, mirroring
	// EINSTEIN_USERPORT in src/mame/tatung/einstein.cpp.
	RC702_PIO_PORT(config, m_pio_b);
	m_pio_b->out_strobe_handler().set(m_pio, FUNC(z80pio_device::strobe_b));

	// generic_keyboard -> kbd_put latches m_kbd_data and pulses strobe_a.
	GENERIC_KEYBOARD(config, m_keyboard);
	m_keyboard->set_keyboard_callback(FUNC(rc702_state::kbd_put));

	AM9517A(config, m_dma, MAIN_XTAL / 2);
	m_dma->out_hreq_callback().set(FUNC(rc702_state::hreq_w));
	m_dma->out_eop_callback().set(FUNC(rc702_state::eop_w)).invert();   // real line is active low, mame has it backwards
	m_dma->in_memr_callback().set(FUNC(rc702_state::memory_read_byte));
	m_dma->out_memw_callback().set(FUNC(rc702_state::memory_write_byte));
	// The 8275 can be fed from DMA ch2 then ch3 in sequence (the "roll
	// function"), allowing a split screen; the 74LS74 hands DRQ from ch2 to
	// ch3 at ch2's TC (see eop_w).  No known RC702 software uses this -- all
	// firmware drives one full-screen segment via ch2.
	m_dma->out_iow_callback<2>().set(m_crtc, FUNC(i8275_device::dack_w));
	m_dma->out_iow_callback<3>().set(m_crtc, FUNC(i8275_device::dack_w));
	// out_dack_callback<1> (FDC) is wired per-variant in add_fdc_dma() since
	// each variant uses a different UPD765A clock and floppy geometry.
	m_dma->out_dack_callback<2>().set(FUNC(rc702_state::dack2_w));

	TTL7474(config, m_7474);
	m_7474->output_cb().set(FUNC(rc702_state::q_w));
	m_7474->comp_output_cb().set(FUNC(rc702_state::qbar_w));

	/* video hardware */
	SCREEN(config, m_screen);
	m_screen->set_refresh_hz(50);
	// 80 chars × 7 px = 560 visible columns, 25 rows × 8 lines = 200 visible
	// lines, plus CRTC_BORDER pixels of background colour on all four sides.
	// The margin is baked into the bitmap (bitmap.fill + coordinate offset in
	// display_pixels) rather than .lay artwork, so SDL and BGFX both show the
	// correct amber background colour without special-casing.
	m_screen->set_size(560 + 2 * CRTC_BORDER, 200 + 4*8 + 2 * CRTC_BORDER);
	m_screen->set_visarea(0, 559 + 2 * CRTC_BORDER - 1, 0, 199 + 2 * CRTC_BORDER - 1);
	m_screen->set_screen_update(FUNC(rc702_state::screen_update_with_border));

	I8275(config, m_crtc, DOT_CLOCK / 7);
	m_crtc->set_character_width(7);
	m_crtc->set_display_callback(FUNC(rc702_state::display_pixels));
	m_crtc->irq_wr_callback().set(m_7474, FUNC(ttl7474_device::clear_w)).invert();
	m_crtc->irq_wr_callback().append(m_ctc1, FUNC(z80ctc_device::trg2));
	m_crtc->drq_wr_callback().set(FUNC(rc702_state::crtc_drq_w));

	PALETTE(config, m_palette, FUNC(rc702_state::rc702_palette), 2);

	config.set_default_layout(layout_rc702);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 1000).add_route(ALL_OUTPUTS, "mono", 0.50);
}

void rc702_state::add_fdc_dma(machine_config &config)
{
	m_fdc->intrq_wr_callback().set(m_ctc1, FUNC(z80ctc_device::trg3));
	m_fdc->drq_wr_callback().set(m_dma, FUNC(am9517a_device::dreq1_w));
	m_dma->in_ior_callback<1>().set(m_fdc, FUNC(upd765a_device::dma_r));
	m_dma->out_iow_callback<1>().set(m_fdc, FUNC(upd765a_device::dma_w));
	m_dma->out_dack_callback<1>().set(FUNC(rc702_state::dack1_w));
}

void rc702_state::rc702(machine_config &config)
{
	rc700_base(config);

	UPD765A(config, m_fdc, MAIN_XTAL, true, true);        // 8 MHz for 8" drives, SSSD not tested.
	add_fdc_dma(config);

	FLOPPY_CONNECTOR(config, "fdc:0", rc702_floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(false);
	FLOPPY_CONNECTOR(config, "fdc:1", rc702_floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(false);
}

void rc702_state::rc702mini(machine_config &config)
{
	rc700_base(config);

	UPD765A(config, m_fdc, MAIN_XTAL / 2, true, true);    // 4 MHz for 5.25" drives
	add_fdc_dma(config);

	FLOPPY_CONNECTOR(config, "fdc:0", rc702mini_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(false);
	FLOPPY_CONNECTOR(config, "fdc:1", rc702mini_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(false);
}

void rc702_state::rc703(machine_config &config)
{
	rc700_base(config);

	UPD765A(config, m_fdc, MAIN_XTAL / 2, true, true);    // 4 MHz for 5.25" QD drives
	add_fdc_dma(config);

	FLOPPY_CONNECTOR(config, "fdc:0", rc703_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(false);
	FLOPPY_CONNECTOR(config, "fdc:1", rc703_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(false);
	// TODO: Hard disk ports 0x60-0x67, CTC2 ports (on hard disk board) 0x44-0x47
}

// RC702 8" with a SEM702 RAM chargen board in IC82: identical to rc702 but
// the ROA327 half of the chargen space is RAM (ports 0xD1/0xD2/0xD3).
// m_has_sem702 routes display_pixels() to m_sem702_ram at run time.
void rc702_state::rc702sem702(machine_config &config)
{
	rc702(config);
	m_has_sem702 = true;
}


/* ROM definition */

// TODO: revisit the PROM-to-machine assignment.  roa375 (RC702), rob357
// (RC703) and rob358 (RC700/RC703) are three fully separate autoload
// codebases (~99% different).  rob358 relocates to 0xA000 with a 64K
// layout and adds colour-CRT, hard-disk and ID-COMAL support that roa375
// lacks, so it is really an RC700/RC703 PROM.  For now: rc702 boots
// roa375 only; rc703 defaults to rob357 with rob358 as an RC700 option.

ROM_START( rc702 )
	// IC66: 2716 (2KB) or 2732 (4KB) EPROM.  Region is 0x1000 with ERASEFF;
	// a 2KB dump fills the low half and the rest stays 0xff.
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "roa375.ic66", 0x0000, 0x0800, CRC(034cf9ea) SHA1(306af9fc779e3d4f51645ba04f8a99b11b5e6084) )

	// IC65 line-program ROM (ROB388 on MIC705), undumped.  Optional: drop a
	// prom1.ic65 into the rom path to supply one (e.g. the CP/NOS line
	// program); otherwise the region stays 0xff.
	ROM_REGION( 0x1000, "prom1", ROMREGION_ERASEFF )
	ROM_LOAD_OPTIONAL( "prom1.ic65", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "roa296.rom", 0x0000, 0x0800, CRC(7d7e4548) SHA1(efb8b1ece5f9eeca948202a6396865f26134ff2f) ) // char
	ROM_LOAD( "roa327.rom", 0x0800, 0x0800, CRC(bed7ddb0) SHA1(201ae9e4ac3812577244b9c9044fadd04fb2b82f) ) // semi_gfx
ROM_END

// rc702mini and rc702sem702 are RC702 machines -> roa375
#define rom_rc702mini    rom_rc702
#define rom_rc702sem702  rom_rc702


// RC703: defaults to its own rob357 PROM; rob358 (RC700/RC703) selectable.
ROM_START( rc703 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "rc703", "RC703")
	ROMX_LOAD( "rob357.rom", 0x0000, 0x0800, CRC(dcf84a48) SHA1(7190d3a898bcbfa212178a4d36afc32bbbc166ef), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "rc700", "RC700")
	ROMX_LOAD( "rob358.rom", 0x0000, 0x0800, CRC(254aa89e) SHA1(5fb1eb8df1b853b931e670a2ff8d062c1bd8d6bc), ROM_BIOS(1))

	ROM_REGION( 0x1000, "prom1", ROMREGION_ERASEFF )
	ROM_LOAD_OPTIONAL( "prom1.ic65", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "roa296.rom", 0x0000, 0x0800, CRC(7d7e4548) SHA1(efb8b1ece5f9eeca948202a6396865f26134ff2f) )
	ROM_LOAD( "roa327.rom", 0x0800, 0x0800, CRC(bed7ddb0) SHA1(201ae9e4ac3812577244b9c9044fadd04fb2b82f) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME         PARENT  COMPAT  MACHINE      INPUT        CLASS        INIT        COMPANY           FULLNAME                          FLAGS
COMP( 1979, rc702,       0,      0,      rc702,       rc702_maxi,  rc702_state, empty_init, "Regnecentralen", "RC702 Piccolo (8\")",            MACHINE_SUPPORTS_SAVE )
COMP( 1979, rc702mini,   rc702,  0,      rc702mini,   rc702_mini,  rc702_state, empty_init, "Regnecentralen", "RC702 Piccolo (5.25\")",         MACHINE_SUPPORTS_SAVE )
COMP( 1979, rc702sem702, rc702,  0,      rc702sem702, rc702_maxi,  rc702_state, empty_init, "Regnecentralen", "RC702 Piccolo (8\", SEM702)",    MACHINE_SUPPORTS_SAVE )
COMP( 1982, rc703,       rc702,  0,      rc703,       rc702_mini,  rc702_state, empty_init, "Regnecentralen", "RC703 Piccolo (5.25\")",         MACHINE_SUPPORTS_SAVE )

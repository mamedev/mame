// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/***************************************************************************

    Technics SX-KN7000 and related MN10300-based keyboards

    All five machines are built around a Panasonic MN103002A (MN1030 series),
    running Panasonic's "MILK" object framework. LCD, control panel and floppy
    are emulated. The tone generators' registers are decoded, but they produce
    no sound: the wave ROMs are undumped and the ADSP-21065L effects DSP is not
    emulated.

    Design notes: https://arqueologiadigital.github.io/technics-docs/kn7000-driver-internals/

    Hardware inventory below is taken from the manufacturer's service manuals:

        SX-KN7000  EMID0207013C0 (2002)
        SX-KN6000  EMID9908016C0 (1999)
        SX-KN6500  EMID0101001C0 (2001)
        SX-KN2400, SX-KN2600

    Each manual covers only its own model, except that the SX-KN2400 book's
    parts list reproduces the SX-KN2600's list of main-board integrated
    circuits verbatim - it names devices that appear on no SX-KN2400 schematic
    sheet or board silkscreen, and omits the floppy controller that all three
    SX-KN2400 sources show. The SX-KN2400 devices below are therefore taken
    from its schematics, block diagram and board assembly drawings only.

    Capacities were read from the schematics and parts lists, and confirmed by
    counting address pins. The manuals write ROM sizes in megabits: the KN6000
    manual prints "(64M BIT MASK ROM)" next to parts numbered QSIGX3C64004 and
    up, which fixes both the unit and the meaning of the digits in the part
    number.

    Clocking: the KN6000 and KN6500 manuals print a 32 MHz oscillator at X1,
    feeding a spread-spectrum clock generator at IC6 whose output drives the
    CPU. The KN7000, KN2400 and KN2600 use the same topology, but their
    manuals do not give the frequency of X1, so their core clocks are inferred
    from the KN6000 and KN6500 rather than documented.

    The KN7000's program and table ROMs are not chip reads. They are payloads
    from Panasonic's own firmware update disks, and they validate against the
    checksums that Panasonic ships alongside them: the update descriptor files
    carry a 32-bit sum over the whole payload plus 16-bit sums of each 256 KiB
    block, and every block matches.

    A note on the KN7000's IC16/IC17: these are one pair of 4 MiB flash devices
    on CPU address lines A2-A22, so together they span 8 MiB. Address line A22
    selects between the two regions declared below - the table data occupies
    the half where A22 is low, and the program the half where it is high.

    TODO:
      - dump the wave, rhythm and picture ROMs listed as NO_DUMP below

***************************************************************************/

#include "emu.h"

#include "kn6000_cpanel.h"
#include "kn7000_cpanel.h"
#include "kn_tonegen.h"

#include "cpu/mn10300/mn10300.h"
#include "imagedev/floppy.h"
#include "machine/input_merger.h"
#include "machine/intelfsh.h"
#include "machine/upd765.h"

#include "screen.h"
#include "speaker.h"

#include "multibyte.h"

#include <algorithm>
#include <iterator>

#include "kn7000.lh"

#define LOG_UNEMULATED (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

// Common to all five models
class kn_state : public driver_device
{
public:
	kn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_cpanel(*this, "cpanel")
		, m_tonegen(*this, "tonegen")
		, m_lcdbuf(*this, "lcdbuf")
		, m_customflash(*this, "custom_data")
		, m_fdc(*this, "fdc")
		, m_wave_main_y(*this, "waveform_main_y")
		, m_wave_main_x(*this, "waveform_main_x")
		, m_wave_sub_y(*this, "waveform_sub_y")
		, m_wave_sub_x(*this, "waveform_sub_x")
		, m_dial(*this, "DIAL")
		, m_rearsw(*this, "REARSW")
		, m_volmain(*this, "VOL_MAIN")
		, m_volapcseq(*this, "VOL_APCSEQ")
		, m_tempoknob(*this, "TEMPO_KNOB")
	{ }

	void kn7000(machine_config &config) ATTR_COLD;
	void kn2400(machine_config &config) ATTR_COLD;
	void kn2600(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(kbd_key);
	DECLARE_INPUT_CHANGED_MEMBER(volume_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void kn_common(machine_config &config) ATTR_COLD;
	void kn24_common(machine_config &config) ATTR_COLD;
	void custom_flash_add(machine_config &config) ATTR_COLD;
	void fdc_add(machine_config &config) ATTR_COLD;
	void configure_cpanel() ATTR_COLD;
	void configure_tonegen() ATTR_COLD;
	void common_map(address_map &map) ATTR_COLD;
	void table_map(address_map &map) ATTR_COLD;
	void fdc_map(address_map &map) ATTR_COLD;
	template <int Tg> void tg_map(address_map &map, offs_t base) ATTR_COLD;
	void single_tg_map(address_map &map) ATTR_COLD;
	void kn24_map(address_map &map) ATTR_COLD;
	template <u16 Straps> u16 strap_r();

	u32 screen_update_rgb565(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_rgb555(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<mn10300_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<kn_cpanel_base_device> m_cpanel;
	required_device<kn_tonegen_base_device> m_tonegen;

	u16 m_sdmbx_out = 0xff;

private:
	enum { SIO_PANEL = 0 };

	required_shared_ptr<u32> m_lcdbuf;
	optional_device<fujitsu_29lv160b_device> m_customflash;
	optional_device<n82077aa_device> m_fdc;
	optional_memory_region m_wave_main_y;
	optional_memory_region m_wave_main_x;
	optional_memory_region m_wave_sub_y;
	optional_memory_region m_wave_sub_x;
	required_ioport m_dial;
	required_ioport m_rearsw;
	required_ioport m_volmain;
	required_ioport m_volapcseq;
	required_ioport m_tempoknob;

	u16 m_sdspi_rate = 0;
	u16 m_kbd_fifo[64];
	u8 m_kbd_head = 0;
	u8 m_kbd_tail = 0;
	u16 m_tg_addr[2];
	u16 m_tg_wave_bank[2];
	u16 m_tg_wave_addr[2];

	void kn2400_map(address_map &map) ATTR_COLD;
	void kn7000_map(address_map &map) ATTR_COLD;

	u32 screen_update_gray(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	template <u32 Base> u16 io_r(offs_t offset, u16 mem_mask = ~0);
	template <u32 Base> void io_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template <int Tg> void tg_addr_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template <int Tg> void tg_data_w(u16 data);
	template <int Tg> void tg_wave_bank_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template <int Tg> void tg_wave_addr_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template <int Tg> u16 tg_wave_data_r();
	u16 kbd_fifo_r();
	u16 sdmbx_r();
	void sdmbx_w(u16 data);
	void update_volume();
};


// SX-KN6000 and SX-KN6500
class kn6000_state : public kn_state
{
public:
	kn6000_state(const machine_config &mconfig, device_type type, const char *tag)
		: kn_state(mconfig, type, tag)
		, m_program(*this, "program")
		, m_libram(*this, "libram")
	{ }

	void kn6000(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_region_ptr<u32> m_program;
	required_shared_ptr<u32> m_libram;

	void kn6000_map(address_map &map) ATTR_COLD;
};


//**************************************************************************
//  Address maps
//**************************************************************************

void kn_state::common_map(address_map &map)
{
	map(0x48400000, 0x487fffff).rom().region("program", 0);

	// External memory appears twice, cachable at 0x40000000-0x7fffffff and
	// uncachable 0x40000000 above that.
	map(0x44000000, 0x44ffffff).ram().share("ram44");
	map(0x84000000, 0x84ffffff).ram().share("ram44");
	map(0x4c000000, 0x4cffffff).ram().share("libram");
	map(0x8c000000, 0x8cffffff).ram().share("libram");
	map(0x50000000, 0x503fffff).ram().share("workram");
	map(0x90000000, 0x903fffff).ram().share("workram");
	map(0x9c000000, 0x9cffffff).ram().share(m_lcdbuf);

	// Expansion ROM windows the firmware probes; nothing is fitted
	map(0x41000000, 0x41ffffff).lr8(NAME([] () -> u8 { return 0x00; })).nopw();
	map(0x56000000, 0x577fffff).lr8(NAME([] () -> u8 { return 0x00; })).nopw();

	// On-chip registers the CPU core does not decode yet. The core's internal
	// map takes precedence for the ones it does.
	map(0x20000000, 0x2000ffff).rw(FUNC(kn_state::io_r<0x20000000>), FUNC(kn_state::io_w<0x20000000>));
	map(0x32000000, 0x3200ffff).rw(FUNC(kn_state::io_r<0x32000000>), FUNC(kn_state::io_w<0x32000000>));
	map(0x34000000, 0x3400ffff).rw(FUNC(kn_state::io_r<0x34000000>), FUNC(kn_state::io_w<0x34000000>));
	map(0x36008000, 0x360080ff).rw(FUNC(kn_state::io_r<0x36008000>), FUNC(kn_state::io_w<0x36008000>));
	map(0x36008084, 0x36008085).lr16(NAME([] () -> u16 { return 0x0001; }));   // bit 0: panel link present

	map(0x98000000, 0x9807ffff).rw(FUNC(kn_state::io_r<0x98000000>), FUNC(kn_state::io_w<0x98000000>));
	map(0x98020000, 0x9802000f).lr8(NAME([] () -> u8 { return 0xff; })).nopw();
	map(0x98050004, 0x98050005).r(FUNC(kn_state::kbd_fifo_r));
	map(0x9805000c, 0x9805000d).rw(FUNC(kn_state::sdmbx_r), FUNC(kn_state::sdmbx_w));
	map(0x9805000e, 0x9805000f).lrw16(
			NAME([this] () -> u16 { return m_sdspi_rate; }),
			NAME([this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_sdspi_rate); }));
	map(0x98070000, 0x98070001).r(FUNC(kn_state::strap_r<0x8006>));
}

// A tone generator's window: register address and data, and the wave ROM
// bank, address and data that the service mode's wave ROM test uses
template <int Tg>
void kn_state::tg_map(address_map &map, offs_t base)
{
	map(base + 0x0, base + 0x1).w(FUNC(kn_state::tg_addr_w<Tg>));
	map(base + 0x2, base + 0x3).w(FUNC(kn_state::tg_data_w<Tg>));
	map(base + 0x4, base + 0x7).nopw();     // initialisation writes, not decoded
	map(base + 0x6, base + 0x7).w(FUNC(kn_state::tg_wave_bank_w<Tg>));
	map(base + 0x10, base + 0x13).nopw();
	map(base + 0x8, base + 0x9).w(FUNC(kn_state::tg_wave_addr_w<Tg>));
	map(base + 0xa, base + 0xb).r(FUNC(kn_state::tg_wave_data_r<Tg>));
}

// The models with one tone generator have it at 0x98050000
void kn_state::single_tg_map(address_map &map)
{
	tg_map<0>(map, 0x98050000);
}

// The KN6000 and KN7000 boards: table data below the program. On the KN7000 both
// are halves of the program flash pair, selected by A22; on the KN6000 and KN6500
// the table data are IC13 and IC14.
void kn_state::table_map(address_map &map)
{
	common_map(map);
	map(0x48000000, 0x483fffff).rom().region("table_data", 0);
	map(0x96800000, 0x969fffff).rw(m_customflash, FUNC(fujitsu_29lv160b_device::read), FUNC(fujitsu_29lv160b_device::write));
}

// The KN2400 and KN2600 board. The firmware also reads the half of the program
// flash pair that is not dumped.
void kn_state::kn24_map(address_map &map)
{
	common_map(map);
	map(0x48000000, 0x483fffff).rom().region("table_data", 0);
	single_tg_map(map);
}

void kn_state::fdc_map(address_map &map)
{
	map(0x98010000, 0x98010003).rw(m_fdc, FUNC(n82077aa_device::dma_r), FUNC(n82077aa_device::dma_w));
	map(0x98020004, 0x98020004).rw(m_fdc, FUNC(n82077aa_device::dor_r), FUNC(n82077aa_device::dor_w));
	map(0x98020008, 0x98020008).rw(m_fdc, FUNC(n82077aa_device::msr_r), FUNC(n82077aa_device::dsr_w));
	map(0x9802000a, 0x9802000a).rw(m_fdc, FUNC(n82077aa_device::fifo_r), FUNC(n82077aa_device::fifo_w));
	map(0x9802000e, 0x9802000e).rw(m_fdc, FUNC(n82077aa_device::dir_r), FUNC(n82077aa_device::ccr_w));
}

void kn_state::kn2400_map(address_map &map)
{
	kn24_map(map);
	fdc_map(map);
}

void kn6000_state::kn6000_map(address_map &map)
{
	table_map(map);
	single_tg_map(map);
	fdc_map(map);
}

void kn_state::kn7000_map(address_map &map)
{
	table_map(map);
	tg_map<0>(map, 0x98040000);
	tg_map<1>(map, 0x98050000);
	fdc_map(map);
}


//**************************************************************************
//  Handlers
//**************************************************************************

// Registers that are not emulated
template <u32 Base>
u16 kn_state::io_r(offs_t offset, u16 mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_UNEMULATED, "%s: unemulated read %08x & %04x\n", machine().describe_context(), Base + (offset << 1), mem_mask);
	return 0;
}

template <u32 Base>
void kn_state::io_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGMASKED(LOG_UNEMULATED, "%s: unemulated write %08x = %04x & %04x\n", machine().describe_context(), Base + (offset << 1), data, mem_mask);
}

// Each tone generator takes a register address, then its data
template <int Tg>
void kn_state::tg_addr_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tg_addr[Tg]);
}

template <int Tg>
void kn_state::tg_data_w(u16 data)
{
	m_tonegen->tg_write(Tg, m_tg_addr[Tg], data);
}

// The service mode's wave ROM test latches a bank at base+6 and a word address
// at base+8, then reads the sample word at base+a.
template <int Tg>
void kn_state::tg_wave_bank_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tg_wave_bank[Tg]);
}

template <int Tg>
void kn_state::tg_wave_addr_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tg_wave_addr[Tg]);
}

template <int Tg>
u16 kn_state::tg_wave_data_r()
{
	const u32 bank = m_tg_wave_bank[Tg] & 0x7fff;           // bit 15 is an enable
	const u32 word = (m_tg_wave_addr[Tg] >> 1) & 0x3fff;
	memory_region *const rgn = BIT(m_tg_wave_addr[Tg], 0)
			? (Tg ? m_wave_sub_x.target() : m_wave_main_x.target())
			: (Tg ? m_wave_sub_y.target() : m_wave_main_y.target());
	if (!rgn)
		return 0xffff;
	const u32 a = (bank * 0x4000 + word) * 2;
	if (a + 1 >= rgn->bytes())
		return 0xffff;
	return get_u16le(rgn->base() + a);
}

// The key bed's event FIFO: the key index in the low byte, bit 7 set on
// release, and the velocity in the high byte
u16 kn_state::kbd_fifo_r()
{
	if (m_kbd_head == m_kbd_tail)
		return 0xffff;
	const u16 data = m_kbd_fifo[m_kbd_tail % std::size(m_kbd_fifo)];
	if (!machine().side_effects_disabled())
		m_kbd_tail++;
	return data;
}

// The SD mailbox in the sub tone generator's window. With nothing attached, a
// write still raises the sub tone generator's interrupt.
// FIXME: what clears TGS.INT2 is unknown; it is held until the CPU accepts it
u16 kn_state::sdmbx_r()
{
	return m_sdmbx_out;
}

void kn_state::sdmbx_w(u16 data)
{
	m_maincpu->set_input_line(mn10300_device::IRQ5, HOLD_LINE);
}

// Board straps. Bit 12 is the rear panel's MIDI IN / BASS PEDAL switch; the
// KN2400/KN2600 firmware tells the models apart by bits 1-0 (11: KN2600,
// 10: KN2400).
template <u16 Straps>
u16 kn_state::strap_r()
{
	return Straps | (m_rearsw->read() & 0x1000);
}


INPUT_CHANGED_MEMBER(kn_state::kbd_key)
{
	if (u8(m_kbd_head - m_kbd_tail) >= std::size(m_kbd_fifo))
		return;
	m_kbd_fifo[m_kbd_head % std::size(m_kbd_fifo)] = newval ? (u16(param) | 0x6400) : (u16(param) | 0xff80);
	m_kbd_head++;
}

// MAIN VOLUME sets the output gain; the squared taper is an approximation
INPUT_CHANGED_MEMBER(kn_state::volume_changed)
{
	update_volume();
}

void kn_state::update_volume()
{
	const float v = float(m_volmain->read()) / 100.0f;
	m_tonegen->set_output_gain(ALL_OUTPUTS, v * v);
}

//**************************************************************************
//  Video
//**************************************************************************

// The firmware composites the LCD image itself. The KN7000's is 640x240 RGB565
// at 0x9ce00000.
u32 kn_state::screen_update_rgb565(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		u32 *const dst = &bitmap.pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			const offs_t k = offs_t(y) * 640 + x;
			const u32 w = m_lcdbuf[(0x00e00000 >> 2) + (k >> 1)];
			const u16 v = BIT(k, 0) ? u16(w >> 16) : u16(w);
			dst[x] = rgb_t(pal5bit(BIT(v, 11, 5)), pal6bit(BIT(v, 5, 6)), pal5bit(BIT(v, 0, 5)));
		}
	}
	return 0;
}

// The KN6000's is RGB555, and the panel is mounted upside down (ROT180)
u32 kn_state::screen_update_rgb555(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		u32 *const dst = &bitmap.pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			const offs_t k = offs_t(y) * 640 + x;
			const u32 w = m_lcdbuf[(0x00e00000 >> 2) + (k >> 1)];
			const u16 v = BIT(k, 0) ? u16(w >> 16) : u16(w);
			dst[x] = rgb_t(pal5bit(BIT(v, 10, 5)), pal5bit(BIT(v, 5, 5)), pal5bit(BIT(v, 0, 5)));
		}
	}
	return 0;
}

// The KN2400's and KN2600's is a 320x240 panel with four grey levels, 2 bits per
// pixel at 0x9c800000, the first pixel in the top bits and 0 the lightest
u32 kn_state::screen_update_gray(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		u32 *const dst = &bitmap.pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			const offs_t k = offs_t(y) * 320 + x;
			const u32 w = m_lcdbuf[(0x00800000 >> 2) + (k >> 4)];
			const u8 byte = u8(w >> ((k & 0xc) << 1));
			const u8 g = 0xff - BIT(byte, 6 - 2 * (k & 3), 2) * 0x55;
			dst[x] = rgb_t(g, g, g);
		}
	}
	return 0;
}


//**************************************************************************
//  Machine
//**************************************************************************

void kn_state::machine_start()
{
	std::fill(std::begin(m_kbd_fifo), std::end(m_kbd_fifo), 0);
	std::fill(std::begin(m_tg_addr), std::end(m_tg_addr), 0);
	std::fill(std::begin(m_tg_wave_bank), std::end(m_tg_wave_bank), 0);
	std::fill(std::begin(m_tg_wave_addr), std::end(m_tg_wave_addr), 0);

	save_item(NAME(m_kbd_fifo));
	save_item(NAME(m_kbd_head));
	save_item(NAME(m_kbd_tail));
	save_item(NAME(m_tg_addr));
	save_item(NAME(m_tg_wave_bank));
	save_item(NAME(m_tg_wave_addr));
	save_item(NAME(m_sdmbx_out));
	save_item(NAME(m_sdspi_rate));
}

void kn_state::machine_reset()
{
	update_volume();
}

// The KN6000 and KN6500 read their library at 0x4c000000 without writing it
// first. What answers there on the board is not established; a copy of the
// program flash is what the firmware finds. It is RAM: the KN6000 writes into it.
void kn6000_state::machine_start()
{
	kn_state::machine_start();
	std::copy_n(&m_program[0], m_program.length(), &m_libram[0]);
}


//**************************************************************************
//  Input ports
//**************************************************************************

INPUT_PORTS_START(kn)
	PORT_START("DIAL")
	PORT_BIT(0xff, 0x00, IPT_DIAL) PORT_SENSITIVITY(30) PORT_KEYDELTA(1) PORT_NAME("Data Dial")

	PORT_START("VOL_MAIN")   PORT_ADJUSTER(80, "Main Volume") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::volume_changed), 0)
	PORT_START("VOL_APCSEQ") PORT_ADJUSTER(80, "APC/SEQ Volume")
	PORT_START("VOL_MIC")    PORT_ADJUSTER(50, "Mic Volume")
	PORT_START("VOL_LINEIN") PORT_ADJUSTER(50, "Line In Volume")
	PORT_START("TEMPO_KNOB") PORT_ADJUSTER(50, "Tempo/Program Knob")

	PORT_START("REARSW")
	PORT_CONFNAME(0x1000, 0x1000, "Rear Panel MIDI IN/Bass Pedal Switch (SW701)")
	PORT_CONFSETTING(0x1000, "MIDI IN")
	PORT_CONFSETTING(0x0000, "Bass Pedals")

	// The key bed; each key's parameter is its index, the GM note minus 36
	PORT_START("KEYS0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_C2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x00)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_CS2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x01)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_D2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x02)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_DS2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x03)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_E2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x04)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_F2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x05)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_FS2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x06)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_G2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x07)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_GS2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x08)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_A2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x09)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_AS2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x0a)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_B2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x0b)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_C3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x0c)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_CS3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x0d)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_D3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x0e)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_DS3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x0f)
	PORT_START("KEYS1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_E3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x10)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_F3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x11)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_FS3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x12)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_G3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x13)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_GS3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x14)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_A3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x15)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_AS3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x16)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_B3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x17)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_C4 PORT_CODE(KEYCODE_Z) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x18)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_CS4 PORT_CODE(KEYCODE_S) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x19)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_D4 PORT_CODE(KEYCODE_X) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x1a)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_DS4 PORT_CODE(KEYCODE_D) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x1b)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_E4 PORT_CODE(KEYCODE_C) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x1c)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_F4 PORT_CODE(KEYCODE_V) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x1d)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_FS4 PORT_CODE(KEYCODE_G) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x1e)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_G4 PORT_CODE(KEYCODE_B) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x1f)
	PORT_START("KEYS2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_GS4 PORT_CODE(KEYCODE_H) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x20)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_A4 PORT_CODE(KEYCODE_N) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x21)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_AS4 PORT_CODE(KEYCODE_J) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x22)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_B4 PORT_CODE(KEYCODE_M) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x23)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_C5 PORT_CODE(KEYCODE_Q) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x24)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_CS5 PORT_CODE(KEYCODE_2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x25)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_D5 PORT_CODE(KEYCODE_W) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x26)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_DS5 PORT_CODE(KEYCODE_3) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x27)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_E5 PORT_CODE(KEYCODE_E) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x28)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_F5 PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x29)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_FS5 PORT_CODE(KEYCODE_5) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x2a)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_G5 PORT_CODE(KEYCODE_T) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x2b)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_GS5 PORT_CODE(KEYCODE_6) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x2c)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_A5 PORT_CODE(KEYCODE_Y) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x2d)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_AS5 PORT_CODE(KEYCODE_7) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x2e)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_B5 PORT_CODE(KEYCODE_U) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x2f)
	PORT_START("KEYS3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_C6 PORT_CODE(KEYCODE_I) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x30)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_CS6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x31)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_D6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x32)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_DS6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x33)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_E6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x34)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_F6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x35)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_FS6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x36)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_G6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x37)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_GS6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x38)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_A6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x39)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_AS6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x3a)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_B6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x3b)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_C7 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn_state::kbd_key), 0x3c)
INPUT_PORTS_END


//**************************************************************************
//  Machine configurations
//**************************************************************************

void kn_floppies(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("35dd", FLOPPY_35_DD);
}

void kn_state::kn_common(machine_config &config)
{
	MN103002A(config, m_maincpu, 32_MHz_XTAL);
	// The MMODE and BMODE straps (R30, R31) select the boot configuration. The
	// firmware has to start at the program flash's own address: its reset code
	// is position-independent, but the first call into the library RAM at
	// 0x4c000000 is PC-relative from the flash, and nothing on the way jumps
	// there absolutely. Interrupts enter through the "nop ; jmp handler" thunk
	// the firmware builds at the base of work RAM, with every IVAR cleared.
	m_maincpu->set_reset_pc(0x48400000);
	m_maincpu->set_vector_base(0x50000000);
	m_maincpu->set_sio_bit_rate<SIO_PANEL>(200'000, false);
	m_maincpu->sio_tx_cb<SIO_PANEL>().set(m_cpanel, FUNC(kn_cpanel_base_device::tx_byte));
	m_maincpu->sio_rx_enable_cb<SIO_PANEL>().set(m_cpanel, FUNC(kn_cpanel_base_device::rx_enable));

	SCREEN(config, m_screen).set_lcd();
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(640, 240);
	m_screen->set_visarea_full();

	SPEAKER(config, "speaker", 2).front();

	// TODO: the effects DSP and its SDRAM (IC307, IC308); USB
}

void kn_state::custom_flash_add(machine_config &config)
{
	// The custom-data flash: IC21 on the KN7000, IC18 on the KN6000 and KN6500. The
	// KN7000 firmware drives it with the AMD command set and checks its autoselect
	// ID against a table of accepted parts at 0x485cf9e0 before it will program it:
	//
	//   maker  device  sectors  name
	//   0x04   0x2249  35       Fujitsu MBM29LV160B
	//   0xc2   0x2249  35       Macronix MX29LV160B
	//   0x1f   0x00c0  40       Atmel AT49BV16X4
	//
	// All are 2 MiB bottom boot parts. Which one is fitted is not recorded: IC21 is
	// marked with the house code C3FBMD000050. The KN6000 and KN6500 accept the
	// MBM29LV160B or the AT49BV16X4.
	// FIXME: the KN6000's A49BV161490T and the KN6500's M29LV160B8TN are modelled
	// by the Fujitsu part, whose ID their firmware accepts
	FUJITSU_29LV160B(config, m_customflash);
}

void kn_state::fdc_add(machine_config &config)
{
	// IC103: a custom part (C1DB00000607) compatible with the N82077AA
	N82077AA(config, m_fdc, 24'000'000);
	// INTRQ and DRQ share IRQ1: the firmware moves each sector byte through the
	// DACK slot at 0x98010000 from its interrupt handler
	input_merger_device &fdc_irq(INPUT_MERGER_ANY_HIGH(config, "fdc_irq"));
	fdc_irq.output_handler().set_inputline(m_maincpu, mn10300_device::IRQ1);
	m_fdc->intrq_wr_callback().set(fdc_irq, FUNC(input_merger_device::in_w<0>));
	m_fdc->drq_wr_callback().set(fdc_irq, FUNC(input_merger_device::in_w<1>));
	FLOPPY_CONNECTOR(config, "fdc:0", kn_floppies, "35hd", floppy_image_device::default_pc_floppy_formats).enable_sound(true);
}

void kn_state::configure_cpanel()
{
	m_cpanel->atn().set_inputline(m_maincpu, mn10300_device::IRQ3);
	m_cpanel->rxd().set(m_maincpu, FUNC(mn10300_device::sio_rx_w<SIO_PANEL>));
	m_cpanel->set_dial_port(m_dial);
	m_cpanel->set_volapcseq_port(m_volapcseq);
	m_cpanel->set_tempoknob_port(m_tempoknob);
}

// The effects DSP between the tone generators and the outputs is not emulated
void kn_state::configure_tonegen()
{
	m_tonegen->add_route(0, "speaker", 1.0, 0);
	m_tonegen->add_route(1, "speaker", 1.0, 1);
}

void kn_state::kn7000(machine_config &config)
{
	kn_common(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &kn_state::kn7000_map);
	m_screen->set_screen_update(FUNC(kn_state::screen_update_rgb565));
	KN7000_CPANEL(config, m_cpanel);
	configure_cpanel();
	KN7000_TONEGEN(config, m_tonegen);
	configure_tonegen();
	custom_flash_add(config);
	fdc_add(config);
	config.set_default_layout(layout_kn7000);
}

// There is no artwork for the KN6000 and KN6500 panel yet
void kn6000_state::kn6000(machine_config &config)
{
	kn_common(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &kn6000_state::kn6000_map);
	m_screen->set_screen_update(FUNC(kn6000_state::screen_update_rgb555));
	KN6000_CPANEL(config, m_cpanel);
	configure_cpanel();
	KN6000_TONEGEN(config, m_tonegen);
	configure_tonegen();
	custom_flash_add(config);
	fdc_add(config);
}

void kn_state::kn24_common(machine_config &config)
{
	kn_common(config);
	m_screen->set_size(320, 240);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(kn_state::screen_update_gray));
	// FIXME: the KN2400 and KN2600 panel is not emulated; the KN7000's stands in
	KN7000_CPANEL(config, m_cpanel);
	configure_cpanel();
	KN2400_TONEGEN(config, m_tonegen);
	configure_tonegen();
}

// The KN2400 has the floppy drive
void kn_state::kn2400(machine_config &config)
{
	kn24_common(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &kn_state::kn2400_map);
	fdc_add(config);
}

void kn_state::kn2600(machine_config &config)
{
	kn24_common(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &kn_state::kn24_map);
}


//**************************************************************************
//  ROM definitions
//**************************************************************************

/***************************************************************************

    SX-KN7000

    IC16, IC17   C3FBNG000016   32 Mbit flash, program + table (see note above)
    IC18         C3CBND000046   64 Mbit mask ROM, rhythm  (later production)
    IC20         C3FBMD000050   32 Mbit flash, rhythm      (earlier production,
                                same site, and half the capacity; the manual
                                states IC20 is not supplied as a spare part)
    IC19         C3CBMD000098   64 Mbit picture ROM
    IC21         C3FBMD000050   16 Mbit custom flash (user data).  The service
                                manual gives it IC20's part code and caption
                                "32M FLASH", but that is copied from IC20: the
                                firmware's flash device table (0x485cf9e0)
                                accepts only 16 Mbit parts, so a 32 Mbit
                                device would fail its autoselect check.  It
                                also builds a 0x200000 sector map, and the
                                board decodes a 2 MB window at 0x96800000.
    IC203        C3CBQD000002  128 Mbit mask ROM, wave, main TG bank Y (AWAY)
    IC204        C3CBQD000001  128 Mbit mask ROM, wave, main TG bank X (AWAX)
    IC207        C3CBQD000004  128 Mbit mask ROM, wave, sub TG bank Y (BWAY)
    IC208        C3CBQD000003  128 Mbit mask ROM, wave, sub TG bank X (BWAX)

    The wave devices sit on two independent buses per tone generator, so they
    are declared as one region per bank rather than concatenated. The block
    diagram shows IC207 and IC208 the other way round, but the schematic gives
    BWAY on IC207 and BWAX on IC208 at pin level, the chip-enable groups agree
    with it, and the part numbers pair as Y = 000002/000004 against
    X = 000001/000003.
    IC414        C3FBKD000162    4 Mbit flash, SD card sub-CPU program

***************************************************************************/

ROM_START(kn7000)
	ROM_REGION32_LE(0x400000, "program", 0)
	ROM_LOAD32_WORD("kn7000_program_even.ic17", 0x000000, 0x200000, CRC(529b87ce) SHA1(f198fd9a9ea31a454acfe7be0eb935beca6771b1))
	ROM_LOAD32_WORD("kn7000_program_odd.ic16",  0x000002, 0x200000, CRC(a36e6222) SHA1(721d4469dc5f692f7a2c16c556b2e21115df19f6))

	ROM_REGION32_LE(0x400000, "table_data", 0)
	ROM_LOAD32_WORD("kn7000_table_even.ic17", 0x000000, 0x200000, CRC(005a6db2) SHA1(2f4112ea9b039b17b5ada6952b7646adae8d9dd6))
	ROM_LOAD32_WORD("kn7000_table_odd.ic16",  0x000002, 0x200000, CRC(7e1a312e) SHA1(435b597b926ebac56d4710bcae25b635a59a9ce5))

	ROM_REGION(0x1000000, "waveform_main_y", 0)
	ROM_LOAD("c3cbqd000002.ic203", 0x000000, 0x1000000, NO_DUMP)

	ROM_REGION(0x1000000, "waveform_main_x", 0)
	ROM_LOAD("c3cbqd000001.ic204", 0x000000, 0x1000000, NO_DUMP)

	ROM_REGION(0x1000000, "waveform_sub_y", 0)
	ROM_LOAD("c3cbqd000004.ic207", 0x000000, 0x1000000, NO_DUMP)

	ROM_REGION(0x1000000, "waveform_sub_x", 0)
	ROM_LOAD("c3cbqd000003.ic208", 0x000000, 0x1000000, NO_DUMP)

	ROM_REGION(0x800000, "rhythm_data", 0)
	ROM_LOAD("c3cbnd000046.ic18", 0x000000, 0x800000, NO_DUMP)

	ROM_REGION(0x800000, "picture", 0)
	ROM_LOAD("c3cbmd000098.ic19", 0x000000, 0x800000, NO_DUMP)

	// The custom flash holds user data, and is populated from a floppy rather than
	// programmed at the factory: the firmware inflates the CTMINI payload from an
	// "Initial Data Disk" and writes it verbatim to offset 0x20000, which is the top
	// 30 of the 64 KiB sectors.  Nothing is written below that, so the boot sectors
	// are left erased here.
	//
	// The images below are therefore not chip dumps.  Each is the exact content the
	// firmware places in the device for one published data set, so a part programmed
	// from that floppy reads back as declared.  They are offered as a BIOS choice
	// because a real instrument holds exactly one of them at a time.
	ROM_REGION(0x200000, "custom_data", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "ctmini",  "Initial Data Disk (factory default)")
	ROMX_LOAD("01ctmini.ic21", 0x020000, 0x1e0000, BAD_DUMP CRC(2a133ea7) SHA1(67b2a0fe8154c4d15557399a86bf0d0b49813ced), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "custm1",  "Custom Data: Blue Bayou")
	ROMX_LOAD("01custm1.ic21", 0x020000, 0x1e0000, BAD_DUMP CRC(c1c69b67) SHA1(4cd5fe37871984a4b470b1a12b835c9bc41f37f5), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "custm2",  "Custom Data: Piano Player")
	ROMX_LOAD("02custm2.ic21", 0x020000, 0x1e0000, BAD_DUMP CRC(8170c2d3) SHA1(6c85b616a9acc3a7ecc1b49a70f138c0d9442086), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "custm3",  "Custom Data: Jazz Organ Soloist")
	ROMX_LOAD("03custm3.ic21", 0x020000, 0x1e0000, BAD_DUMP CRC(5a7cc504) SHA1(d5722a1129134383bf643d20b4e0866c3089fc36), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "custm4",  "Custom Data: Bob's Band")
	ROMX_LOAD("04custm4.ic21", 0x020000, 0x1e0000, BAD_DUMP CRC(1f1c195e) SHA1(25822a3ad726df49882cdd0066424600fb70ea14), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "custm5",  "Custom Data: Traditional Melody")
	ROMX_LOAD("01custm5.ic21", 0x020000, 0x1e0000, BAD_DUMP CRC(ac264469) SHA1(11d32c3d293a23a511a76297647758046cd8ccc9), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "custm6",  "Custom Data: Jogeh 1")
	ROMX_LOAD("02custm6.ic21", 0x020000, 0x1e0000, BAD_DUMP CRC(174cea4c) SHA1(54f2bac0551a8d43dc1a87fe24a13c545169a326), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(7, "custm7",  "Custom Data: Vibraphone")
	ROMX_LOAD("03custm7.ic21", 0x020000, 0x1e0000, BAD_DUMP CRC(aa14a0ed) SHA1(e1bdfbbbe78353003af5ed01866be87d799523bb), ROM_BIOS(7))
	ROM_SYSTEM_BIOS(8, "custm8",  "Custom Data: Italian Accordion 7")
	ROMX_LOAD("04custm8.ic21", 0x020000, 0x1e0000, BAD_DUMP CRC(18753233) SHA1(f4722d93ff3f31c6d8c25f757968d796e4eb57ca), ROM_BIOS(8))

	ROM_REGION(0x80000, "sdcard_cpu", 0)
	ROM_LOAD("c3fbkd000162.ic414", 0x000000, 0x80000, NO_DUMP)
ROM_END

/***************************************************************************

    SX-KN6000

    IC11, IC12   M29LV160B8TN   16 Mbit flash, program
    IC13         QSIGX3C16008   16 Mbit mask ROM, table data
    IC14         QSIGX3C16007   16 Mbit mask ROM, table data
    IC15         QSIGX3C32021   32 Mbit mask ROM, rhythm data
    IC18         A49BV161490T   16 Mbit flash, custom rhythm (user data)
    IC205        QSIGX3C64004   64 Mbit mask ROM, wave, bank Y (WAY)
    IC206        QSIGX3C64005   64 Mbit mask ROM, wave, bank X (WAX)
    IC207        QSIGX3C64006   64 Mbit mask ROM, wave, bank Y (WAY)
    IC208        QSIGX3C64007   64 Mbit mask ROM, wave, bank X (WAX)

***************************************************************************/

ROM_START(kn6000)
	ROM_REGION32_LE(0x400000, "program", 0)
	ROM_LOAD32_WORD("kn6000_program_even.ic12", 0x000000, 0x200000, CRC(56c2cfe3) SHA1(e15a4c73440f1dcdf06457f9956c96bf20d68b16))
	ROM_LOAD32_WORD("kn6000_program_odd.ic11",  0x000002, 0x200000, CRC(9d94da6c) SHA1(d73b4c8ebf0c67b6a2eeb5571d0273fc6efbfe4c))

	ROM_REGION32_LE(0x400000, "table_data", 0)
	ROM_LOAD32_WORD("qsigx3c16008.ic13", 0x000000, 0x200000, NO_DUMP)
	ROM_LOAD32_WORD("qsigx3c16007.ic14", 0x000002, 0x200000, NO_DUMP)

	ROM_REGION(0x1000000, "waveform_main_y", 0)
	ROM_LOAD("qsigx3c64004.ic205", 0x000000, 0x800000, NO_DUMP)
	ROM_LOAD("qsigx3c64006.ic207", 0x800000, 0x800000, NO_DUMP)

	ROM_REGION(0x1000000, "waveform_main_x", 0)
	ROM_LOAD("qsigx3c64005.ic206", 0x000000, 0x800000, NO_DUMP)
	ROM_LOAD("qsigx3c64007.ic208", 0x800000, 0x800000, NO_DUMP)

	ROM_REGION(0x400000, "rhythm_data", 0)
	ROM_LOAD("qsigx3c32021.ic15", 0x000000, 0x400000, NO_DUMP)

	// Populated from the IDD6000 Initial Data Disk, which serves both the KN6000 and
	// the KN6500; see the note in the KN7000 set above.  Not a chip dump.
	ROM_REGION(0x200000, "custom_data", ROMREGION_ERASEFF)
	ROM_LOAD("01ctmini.ic18", 0x020000, 0x1e0000, BAD_DUMP CRC(f108e4c7) SHA1(8c6d62a8afab717a2b59e9242bbf897b01369416))
ROM_END

/***************************************************************************

    SX-KN6500

    IC11, IC12   M29LV160B8TN   16 Mbit flash, program
    IC13         C3FBMD000069   16 Mbit table data, supplied pre-programmed
    IC14         C3FBMD000068   16 Mbit table data, supplied pre-programmed
                                (the schematic legend reads "PROGRAMMED MASK
                                ROM", copied from the KN6000, but the pinout
                                drawn beside it - RESET, RY/BY, VPP, WE - is a
                                NOR flash, so no device type is asserted here)
    IC15         QSIGX3C32021   32 Mbit mask ROM, rhythm data
    IC18         M29LV160B8TN   16 Mbit flash, custom rhythm (user data).  The
                                schematic labels IC11, IC12 and IC18 with this
                                same part but three different descriptors, so
                                it is fitted at all three flash sites rather
                                than being a repeated parts-list row.  The
                                firmware will only program a device whose
                                autoselect response is in its table, and that
                                table holds MBM29LV160B and AT49BV16X4 - both
                                16 Mbit bottom boot, which is what fixes the
                                geometry here.  Which vendor's 29LV160B this
                                designation refers to is not established.
    IC205        QSIGX3C64004   64 Mbit mask ROM, wave, bank Y (WAY)
    IC206        QSIGX3C64005   64 Mbit mask ROM, wave, bank X (WAX)
    IC207        QSIGX3C64006   64 Mbit mask ROM, wave, bank Y (WAY)
    IC208        QSIGX3C64007   64 Mbit mask ROM, wave, bank X (WAX)
    IC209        QSIGX3C64020   64 Mbit mask ROM, wave, bank Y (WAY)
    IC210        QSIGX3C64019   64 Mbit mask ROM, wave, bank X (WAX)

***************************************************************************/

ROM_START(kn6500)
	ROM_REGION32_LE(0x400000, "program", 0)
	ROM_LOAD32_WORD("kn6500_program_even.ic12", 0x000000, 0x200000, CRC(f42a2fcf) SHA1(7cebf73bf623fd714ca455ed50b80da1d2186414))
	ROM_LOAD32_WORD("kn6500_program_odd.ic11",  0x000002, 0x200000, CRC(ca2a733f) SHA1(2484d3b76b62b05ded39e4194cdc74fd3c01bcbe))

	ROM_REGION32_LE(0x400000, "table_data", 0)
	ROM_LOAD32_WORD("c3fbmd000069.ic13", 0x000000, 0x200000, NO_DUMP)
	ROM_LOAD32_WORD("c3fbmd000068.ic14", 0x000002, 0x200000, NO_DUMP)

	ROM_REGION(0x1800000, "waveform_main_y", 0)
	ROM_LOAD("qsigx3c64004.ic205", 0x0000000, 0x800000, NO_DUMP)
	ROM_LOAD("qsigx3c64006.ic207", 0x0800000, 0x800000, NO_DUMP)
	ROM_LOAD("qsigx3c64020.ic209", 0x1000000, 0x800000, NO_DUMP)

	ROM_REGION(0x1800000, "waveform_main_x", 0)
	ROM_LOAD("qsigx3c64005.ic206", 0x0000000, 0x800000, NO_DUMP)
	ROM_LOAD("qsigx3c64007.ic208", 0x0800000, 0x800000, NO_DUMP)
	ROM_LOAD("qsigx3c64019.ic210", 0x1000000, 0x800000, NO_DUMP)

	ROM_REGION(0x400000, "rhythm_data", 0)
	ROM_LOAD("qsigx3c32021.ic15", 0x000000, 0x400000, NO_DUMP)

	// Same IDD6000 payload as the KN6000; not a chip dump.
	ROM_REGION(0x200000, "custom_data", ROMREGION_ERASEFF)
	ROM_LOAD("01ctmini.ic18", 0x020000, 0x1e0000, BAD_DUMP CRC(f108e4c7) SHA1(8c6d62a8afab717a2b59e9242bbf897b01369416))
ROM_END

/***************************************************************************

    SX-KN2400 and SX-KN2600

    One firmware image serves both models, selecting between them at run time,
    and the two boards carry an identical set of memory devices: the schematic
    sheet holding the tone generator and both wave ROMs is the same drawing in
    both manuals, and the board assembly drawings list the same designators.
    The models differ in storage and I/O only - the SX-KN2400 has a floppy
    drive and controller, the SX-KN2600 an SD card interface. They therefore
    share every ROM listed below except the SD sub-processor's program flash,
    which is fitted only on the SX-KN2600.

    IC12, IC13   C3FBNG000007   32 Mbit flash, program (see note below)
    IC14         C3ZBNG000023   64 Mbit flash, rhythm and other data
    IC302        C3ZBP0000003   64 Mbit flash, wave bank Y (AWAY bus)
    IC303        C3ZBP0000004   64 Mbit flash, wave bank X (AWAX bus)
    IC404        C3ZBK0000020    4 Mbit flash, SD sub-CPU program (KN2600 only)

    Neither board carries a table or font ROM: the schematics, the board
    assembly drawings and the block diagrams agree that the devices listed
    above are the only memories present, and the LCD controller at IC104 has
    no external memory attached.

    The block diagrams describe IC302 and IC303 as 128 Mbit parts addressed by
    WAY0-WAY22, but the schematics show only 22 address inputs, and the tone
    generator's WAY22 and WAY23 pins terminate unconnected. The 64 Mbit figure
    from the schematics is used here.

    A note on IC12/IC13: the schematics show 21 address inputs driven from CPU
    address lines A2-A22, making these 32 Mbit devices that together span
    8 MiB. The dumps below cover 4 MiB of that pair. The firmware also reads the
    other half, at 0x48000000; it has not been read, so it is mapped erased. On
    the KN7000 the equivalent pair holds the table data in that half.

***************************************************************************/

#define KN2400_ROM_COMMON \
	ROM_REGION32_LE(0x400000, "program", 0) \
	ROM_LOAD32_WORD("kn2400_program_even.ic13", 0x000000, 0x200000, CRC(b94fc8a8) SHA1(86d5d9916afdb90f82de78064b1d76fce3a21d7b)) \
	ROM_LOAD32_WORD("kn2400_program_odd.ic12",  0x000002, 0x200000, CRC(73781cbc) SHA1(d90a3560561efd94322dca1a6710f2d5d3837cd2)) \
	\
	ROM_REGION32_LE(0x400000, "table_data", ROMREGION_ERASEFF) \
	\
	ROM_REGION(0x800000, "rhythm_data", 0) \
	ROM_LOAD("c3zbng000023.ic14", 0x000000, 0x800000, NO_DUMP) \
	\
	ROM_REGION(0x800000, "waveform_main_y", 0) \
	ROM_LOAD("c3zbp0000003.ic302", 0x000000, 0x800000, NO_DUMP) \
	\
	ROM_REGION(0x800000, "waveform_main_x", 0) \
	ROM_LOAD("c3zbp0000004.ic303", 0x000000, 0x800000, NO_DUMP)

ROM_START(kn2400)
	KN2400_ROM_COMMON
ROM_END

ROM_START(kn2600)
	KN2400_ROM_COMMON

	// The SD card interface is fitted only on the SX-KN2600.
	ROM_REGION(0x80000, "sdcard_cpu", 0)
	ROM_LOAD("c3zbk0000020.ic404", 0x000000, 0x80000, NO_DUMP)
ROM_END

} // anonymous namespace


//   YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY     FULLNAME      FLAGS
SYST(2002, kn7000, 0,      0,      kn7000,  kn,    kn_state,     empty_init, "Technics", "SX-KN7000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
SYST(1999, kn6000, 0,      0,      kn6000,  kn,    kn6000_state, empty_init, "Technics", "SX-KN6000", ROT180 | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
SYST(2001, kn6500, 0,      0,      kn6000,  kn,    kn6000_state, empty_init, "Technics", "SX-KN6500", ROT180 | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
SYST(2000, kn2400, 0,      0,      kn2400,  kn,    kn_state,     empty_init, "Technics", "SX-KN2400", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
SYST(2000, kn2600, kn2400, 0,      kn2600,  kn,    kn_state,     empty_init, "Technics", "SX-KN2600", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

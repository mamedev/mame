// license:GPL2+
// copyright-holders:Felipe Sanches
/***************************************************************************

    Technics SX-KN7000 and related MN10300-based keyboards

    Panasonic MN103002A (MN10300 family, AM33 core) running the "MILK"
    object framework. LCD, control panel, floppy, SD, MIDI and an
    ADSP-21065L effects DSP.

    The wave ROMs are undumped, so the tone generator is driven correctly
    but its timbre is a placeholder.

    Design notes: https://arqueologiadigital.github.io/technics-docs/kn7000-driver-internals/

***************************************************************************/

#include "emu.h"

#include "kn6000_cpanel.h"
#include "kn6000_tonegen.h"
#include "kn7000_cpanel.h"
#include "kn7000_tonegen.h"

#include "cpu/mn10300/mn10300.h"
#include "imagedev/floppy.h"
#include "machine/intelfsh.h"
#include "machine/spi_sdcard.h"
#include "machine/upd765.h"

#include "screen.h"
#include "speaker.h"

#include <atomic>

#include "kn7000.lh"

namespace {

constexpr offs_t IRQ_VECTOR_BASE = 0x50000000;

class kn7000_state : public driver_device
{
public:
	kn7000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_workram(*this, "workram")
		, m_customflash(*this, "custom_data")
		, m_lcdbuf(*this, "lcdbuf")
		, m_progrom(*this, "program")
		, m_tonegen(*this, "tonegen")
		, m_dial(*this, "DIAL")
		, m_rearsw(*this, "REARSW")
		, m_sdsw(*this, "CPSD_SDSW")
		, m_sdcard(*this, "sdcard")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0")
		, m_sdcover(*this, "SDCOVER")
		, m_volmain(*this, "VOL_MAIN")
		, m_volapcseq(*this, "VOL_APCSEQ")
		, m_tempoknob(*this, "TEMPO_KNOB")
		, m_cpanel(*this, "cpanel")
		, m_sd_leds(*this, "sd_led%u", 0U)
	{ }

	void kn7000_base(machine_config &config) ATTR_COLD;
	void kn7000(machine_config &config) ATTR_COLD;
	void kn6000(machine_config &config) ATTR_COLD;
	void kn6500(machine_config &config) ATTR_COLD;
	void kn24_base(machine_config &config) ATTR_COLD;
	void kn2400(machine_config &config) ATTR_COLD;
	void kn2600(machine_config &config) ATTR_COLD;
	DECLARE_INPUT_CHANGED_MEMBER(kbd_key);     // PC-key note -> voice-event FIFO (public: PORT_CHANGED_MEMBER)
	DECLARE_INPUT_CHANGED_MEMBER(sd_cover_changed);   // SD slot cover toggle (public: PORT_CHANGED_MEMBER)

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<mn10300_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint32_t> m_workram;
	optional_device<fujitsu_29lv160b_device> m_customflash;  // IC21 @ 0x96800000
	uint32_t customflash_r(offs_t offset, uint32_t mem_mask = ~0);
	void customflash_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	required_shared_ptr<uint32_t> m_lcdbuf;      // firmware's composited RGB565 LCD image @0x9CE00000
	bool m_lib_mirror = false;                   // KN6000/KN6500: library @0x4C/0x8C mirrors the program ROM
	bool m_lcd_kn6 = false;                      // KN6000/KN6500: LCD framebuffer is RGB555 and mounted rotated 180deg (vs the KN7000's upright RGB565)
	bool m_lcd_kn24 = false;                     // KN2400/KN2600: 320x240 4-level grayscale panel, 2bpp framebuffer at 0x9C800000
	required_region_ptr<uint32_t> m_progrom;     // program flash (holds the CLUT)
	required_device<kn_tonegen_base_device> m_tonegen;   // first-cut audio (Phase C Stage 0)

	// Control panel button ports and LEDs (CPL = 8 cols, CPC = 5 cols; CPR + the
	// serial HLE device that reads these / drives the LEDs are still to come).
	required_ioport m_dial;
	required_ioport m_rearsw;           // rear-panel MIDI IN / BASS PEDAL selector SW701 (strap bit12 = data-bus D28)
	required_ioport m_sdsw;               // SD front-panel switches (byte 0x9CC00008, active-low)
	optional_device<spi_sdcard_device> m_sdcard;   // the SD card (SPI protocol via the 0x9805000C byte mailbox)
	optional_device<n82077aa_device> m_fdc;        // IC103 floppy disk controller (C1DB00000607, N82077AA/PC-AT-compatible)
	optional_device<floppy_connector> m_floppy;    // the 3.5" floppy drive
	uint8_t fdc_r(offs_t off);                     // FDC (IC103) PC/AT registers at 0x98020000 (schematic-confirmed)
	void    fdc_w(offs_t off, uint8_t data);
	uint8_t fdc_dma_r(offs_t off);                 // FDC.DACK byte slot at 0x98010000 (software-DMA transfer)
	void    fdc_dma_w(offs_t off, uint8_t data);
	void    fdc_irq_w(int state);                  // FDC INTRQ -> INTC group 0x18
	void    fdc_drq_w(int state);                  // FDC DRQ  -> INTC group 0x18 (per-byte software-DMA)
	required_ioport m_sdcover;             // SD slot cover switch (open/closed)
	required_ioport m_volmain;             // front-panel MAIN VOLUME slider (0-100 adjuster)
	required_ioport m_volapcseq;           // front-panel APC/SEQ VOLUME slider (0-100 adjuster)
	required_ioport m_tempoknob;           // front-panel TEMPO/PROGRAM knob (0-100 adjuster; a RELATIVE encoder)
	// Held by BASE type: kn7000() installs the KN7000 panel, kn6000() swaps in the
	// KN6000/KN6500 one (same tag "cpanel", same protocol, different matrix + LEDs).
	required_device<kn_cpanel_base_device> m_cpanel;   // control-panel HLE (buttons, LEDs, analog controls)

	void maincpu_mem(address_map &map) ATTR_COLD;

	// Logging handlers for the I/O banks that are not decoded yet.
	// The 0x98000000 sound/peripheral window, whose offsets these decode.
	uint16_t snd_r(offs_t offset, uint16_t mem_mask = ~0);
	void snd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t io_r(offs_t offset, uint16_t mem_mask = ~0);
	void io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	enum { IRQGRP_TIMER = 0x06, IRQGRP_PANEL = 0x1A, IRQGRP_MIDI1 = 0x12, IRQGRP_MIDI2 = 0x14 };
	void intc_assert(int group) { m_maincpu->intc_assert(group); }
	bool m_c11_unserviced = false;             // a panel transfer-complete not yet accepted
	uint16_t m_extmd_prev = 0;                 // previous EXTMD value (edge decode across intc_extmd_cb)
	uint16_t m_sdspi_rate = 0;                 // SD SPI clock-rate latch (0x9805000E), read back to verify
	uint16_t m_kbd_fifo[64] = { };
	uint8_t  m_kbd_head = 0, m_kbd_tail = 0;
	void kbd_push(uint8_t note, uint8_t vel)
	{
		m_kbd_fifo[m_kbd_head & 63] = uint16_t(note) | (uint16_t(vel) << 8); m_kbd_head++;
		if (note & 0x80) m_tonegen->key_break(note & 0x7F);
		else             m_tonegen->key_context(note);
	}

	uint16_t m_tg_addr[2] = { 0, 0 };          // latched register address, [0]=main [1]=sub

	uint16_t m_tg_wave_bank[2] = { 0, 0 };     // latched bank register  (base+6), [0]=main [1]=sub
	uint16_t m_tg_wave_addr[2] = { 0, 0 };     // latched word-address   (base+8)
	memory_region *m_waverom[4] = { nullptr, nullptr, nullptr, nullptr };
	uint16_t tg_wave_read(int tg);             // compose the address and return the sample word

	emu_timer *m_sys_timer = nullptr;
	TIMER_CALLBACK_MEMBER(sys_tick);

	// --- On-chip 16-bit TEMPO timer (mode 0x34001082 / base 0x34001092 / count 0x340010A2)

	emu_timer *m_sd_insert_timer = nullptr;
	TIMER_CALLBACK_MEMBER(sd_insert);

	output_finder<2> m_sd_leds;              // sd_led0 = SD in use, sd_led1 = SD play/pause
	emu_timer *m_sd_inuse_off = nullptr;     // one-shot: clear SD-in-use after the last SPI byte
	TIMER_CALLBACK_MEMBER(sd_inuse_off);

	// The SD slot has a hinged COVER; the firmware reads the cover switch as the
	// card-detect line: closed with a card in means accessible, open means absent.
	void sd_update_carddetect()
	{
		if (m_lib_mirror) return;
		const bool cover_open = (m_sdcover->read() & 1) != 0;
		const bool card = m_sdcard && m_sdcard->get_card_present();
		if (!cover_open && card)
			m_maincpu->intc_icr_clear(0x1B, 0x001F);   // bit4=0: present (closed + card)
		else
			m_maincpu->intc_icr_set(0x1B, 0x0012);     // bit4=1: no card / lid open
	}

	uint16_t m_sdmbx_out = 0xFF;               // last MISO byte (mailbox read value)
	uint16_t m_gpio8004 = 0xFFFF;              // GPIO latch 0x36008004 (bit1 = SD SPI CS, active-low)
	uint8_t  m_sdmbx_miso = 0;                 // MISO bit collector (spi_miso callback)
	void cpsd_mbx_write(uint16_t data);
	void sd_miso_w(int state) { m_sdmbx_miso = uint8_t(m_sdmbx_miso << 1) | (state & 1); }

	// --- SIO: three on-chip USART channels at 0x34000800 / 0x810 / 0x820 ----
	// ch0 = control panel, ch1 = MIDI port 1, ch2 = MIDI port 2.
	enum { SIO_PANEL = 0, SIO_MIDI1 = 1, SIO_MIDI2 = 2 };

	// --- Control-panel HLE (the sub-CPU side of the panel serial link) ------
	// LED command bytes arrive as 2-byte [ADDR][DATA] frames on the panel TX;
	TIMER_CALLBACK_MEMBER(panel_txdone_cb); // one-shot: SIO ch0 sync-transfer complete -> group 0x11
	emu_timer *m_panel_txdone = nullptr;
	TIMER_CALLBACK_MEMBER(volume_scan);     // periodic MAIN VOLUME slider -> DSP master gain
	emu_timer *m_vol_timer = nullptr;

	// --- CPSD (SD sub-CPU MN102H60) HLE on SIO channel 2 --------------------
	// The SD card is reached over SIO channel 2.

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

void kn7000_state::maincpu_mem(address_map &map)
{
	// The MN10300/AM33 has a flat 32-bit little-endian address space.

	// --- Table / rhythm flash -------------------------------------------
	// kn7000_table.rom, decompressed size 0x3E94D4 bytes (~4 MiB).
	map(0x48000000, 0x483fffff).rom().region("table_data", 0);

	// --- Program flash --------------------------------------------------
	map(0x48400000, 0x487fffff).rom().region("program", 0);

	// --- Work RAM -------------------------------------------------------
	map(0x50000000, 0x503fffff).ram().share("workram");

	map(0x4c000000, 0x4cffffff).ram().share("libram");
	map(0x8c000000, 0x8cffffff).ram().share("libram");
	map(0x90000000, 0x903fffff).ram().share("workram");

	// Further windows the boot reaches only after the library ROM loads and runs.
	// 0x44000000 is a heavily read and written ~1 MB block, mirrored at 0x84000000.
	map(0x44000000, 0x44ffffff).ram().share("ram44");
	map(0x84000000, 0x84ffffff).ram().share("ram44");
	map(0x9c000000, 0x9cffffff).ram().share("lcdbuf");   // firmware's composited LCD image (RGB565) lives at 0x9CE00000
	// SD front-panel switch register (byte 0x9CC00008, active-low: bits 0-5 are the
	// six CPSD-side transport switches, 1 = released).
	if (!m_lib_mirror)
		map(0x9cc00008, 0x9cc0000b).lr32(NAME([this](offs_t) -> uint32_t
		{
			return (m_lcdbuf[0x00C00008 >> 2] & 0xFFFFFFC0) | (~m_sdsw->read() & 0x3F);
		}));

	// --- Stubs for regions whose behavior is still unknown --------------

	map(0x41000000, 0x41ffffff).lr8(NAME([](offs_t) -> uint8_t { return 0x00; })).nopw();
	map(0x56000000, 0x577fffff).lr8(NAME([](offs_t) -> uint8_t { return 0x00; })).nopw();

	// TODO: Picture flash (splash / bitmap graphics), separate device.
	// 0x57800000 is the picture flash; it has no dump, so it is left unmapped.

	// These handlers only log. The CPU core's internal map is appended after this
	// one and decodes what it models -- interrupt vectors, the INTC, the serial
	// channels and the TM4/TM5 timers -- so only the rest arrives here.
	// TODO: model the rest; the on-chip registers belong in the CPU core.
	map(0x20000000, 0x2000ffff).rw(FUNC(kn7000_state::io_r), FUNC(kn7000_state::io_w));
	map(0x32000000, 0x3200ffff).rw(FUNC(kn7000_state::io_r), FUNC(kn7000_state::io_w));
	map(0x34000000, 0x3400ffff).rw(FUNC(kn7000_state::io_r), FUNC(kn7000_state::io_w));
	// KN6000/KN6500: the firmware polls further on-chip 16-bit timer counters
	// (TMnBC at 0x340010a4+, beyond the TM4/TM5 pair the core models) as plain RAM.
	if (m_lib_mirror)
		map(0x340010a4, 0x340010af).lr16(NAME([this](offs_t o) { return uint16_t(-(m_maincpu->total_cycles() >> 4)); }));
	map(0x36008000, 0x360080ff).rw(FUNC(kn7000_state::io_r), FUNC(kn7000_state::io_w));
	// GPIO input port 0x36008084: bit 0 = panel-link ready/presence, held asserted.
	map(0x36008084, 0x36008085).lr16(NAME([]() -> uint16_t { return 0x0001; }));
	// GPIO output latch 0x36008004: bit1 = the SD card's SPI chip select, active-low
	// (bclr asserts, bset releases).
	if (!m_lib_mirror)
		map(0x36008004, 0x36008005).lrw16(
			NAME([this](offs_t) -> uint16_t { return m_gpio8004; }),
			NAME([this](offs_t, uint16_t data, uint16_t mem_mask)
			{
				COMBINE_DATA(&m_gpio8004);
				if (m_sdcard)
					m_sdcard->spi_ss_w((m_gpio8004 & 0x0002) ? 0 : 1);   // active-low CS
			}));
	map(0x98000000, 0x9807ffff).rw(FUNC(kn7000_state::snd_r), FUNC(kn7000_state::snd_w));
	map(0x98020000, 0x9802000f).rw(FUNC(kn7000_state::fdc_r), FUNC(kn7000_state::fdc_w));
	// FDC.DACK byte slot at 0x98010000 (decoder Y1). The software-DMA handler, invoked
	// per FDC.DRQ on INTC group 0x18, moves one FIFO byte through here per request.
	map(0x98010000, 0x98010003).rw(FUNC(kn7000_state::fdc_dma_r), FUNC(kn7000_state::fdc_dma_w));
}

// Read one sample word out of a wave ROM through the tone generator's own port.
uint16_t kn7000_state::tg_wave_read(int tg)
{
	const uint32_t bank = m_tg_wave_bank[tg] & 0x1FF;        // base+6: 512 banks (bit15 = enable, masked off)
	const uint32_t chip = m_tg_wave_addr[tg] & 1;            // base+8 bit0: which of the two ROMs on this side
	const uint32_t word = (m_tg_wave_addr[tg] >> 1) & 0x3FFF;// base+8 bits1..14: word within the bank
	memory_region *const rgn = m_waverom[(unsigned(tg) << 1) | chip];
	if (!rgn)
		return 0xFFFF;
	const uint32_t a = (bank * 0x4000u + word) * 2u;
	if (a + 1 >= rgn->bytes())
		return 0xFFFF;
	return uint16_t(rgn->base()[a] | (rgn->base()[a + 1] << 8));
}

uint16_t kn7000_state::io_r(offs_t offset, uint16_t mem_mask)
{
	if (!machine().side_effects_disabled())
		logerror("%s: io_r  +%06X mask %04X\n", machine().describe_context(), offset << 1, mem_mask);
	return 0;
}

void kn7000_state::io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: io_w  +%06X = %04X mask %04X\n", machine().describe_context(),
			offset << 1, data, mem_mask);
}

uint16_t kn7000_state::snd_r(offs_t offset, uint16_t mem_mask)
{
	// 0x98070000 (offset 0x38000 within the 0x98000000 window): a status/strap word.
	if (offset == 0x38000)
	{
		return 0x8000 | 0x0006 | (m_rearsw->read() & 0x1000);
	}
	// 0x98050004 (offset 0x28002): the VOICE-EVENT / keyboard FIFO -- the interface
	// what the KN5000 firmware calls "keyboard input".
	if (offset == 0x28002)
	{
		if (!machine().side_effects_disabled() && m_kbd_head != m_kbd_tail)
			return m_kbd_fifo[m_kbd_tail++ & 63];
		return 0xFFFF;
	}
	if (offset == 0x28006)                            // 0x9805000C: SD mailbox data latch
		return m_sdmbx_out;
	if (offset == 0x28007)
		return m_sdspi_rate;
	// Wave-memory read port DATA (main 0x9804000A / sub 0x9805000A): return the
	// sample word for the latched (bank, address). See tg_wave_read().
	if (offset == 0x20005) return tg_wave_read(0);
	if (offset == 0x28005) return tg_wave_read(1);
	if (!machine().side_effects_disabled())
		logerror("%s: snd_r +%06X mask %04X\n", machine().describe_context(),
			offset << 1, mem_mask);
	return 0;
}

void kn7000_state::snd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 0x28007)                        // 0x9805000E: SD SPI clock-rate latch
	{
		COMBINE_DATA(&m_sdspi_rate);
		return;
	}
	switch (offset)
	{
	case 0x28006:                                                 // 0x9805000C: SD mailbox data latch
		cpsd_mbx_write(data);
		return;
	case 0x20000: m_tg_addr[0] = data; return;                    // main TG: address latch (0x98040000)
	case 0x20001:                                                 // main TG: data (0x98040002) -> reg[addr]
		m_tonegen->tg_write(0, m_tg_addr[0], data);                // Stage 2: feed the real TG voice engine
		return;
	case 0x28000: m_tg_addr[1] = data; return;                    // sub TG: address latch (0x98050000)
	case 0x28001:                                                 // sub TG: data (0x98050002) -> reg[addr]
		m_tonegen->tg_write(1, m_tg_addr[1], data);
		return;
	case 0x20002: case 0x20008:                                   // main TG control (0x98040004 / 0x98040010)
		return;
	// Wave-memory read port: the service-mode WAVE ROM test latches a bank at
	// base+6 and a word address at base+8, then reads the sample word at base+A.
	case 0x20003: m_tg_wave_bank[0] = data; return;              // main TG wave bank    (0x98040006)
	case 0x20004: m_tg_wave_addr[0] = data; return;              // main TG wave address (0x98040008)
	case 0x28003: m_tg_wave_bank[1] = data; return;              // sub  TG wave bank    (0x98050006)
	case 0x28004: m_tg_wave_addr[1] = data; return;              // sub  TG wave address (0x98050008)
	}
	// (The FDC registers at 0x98020000-0f are carved out of this window -> fdc_r/fdc_w; see io_r note.)
	logerror("%s: snd_w +%06X = %04X mask %04X\n", machine().describe_context(),
		offset << 1, data, mem_mask);
}

TIMER_CALLBACK_MEMBER(kn7000_state::sys_tick)
{
	intc_assert(IRQGRP_TIMER);
}

// (The on-chip TEMPO timer -- TM5, the clock behind all sequenced playback --
// is modeled in the MN10300 core now: mn10300.cpp tm5_*.)

void kn7000_state::cpsd_mbx_write(uint16_t data)
{
	// SD "in use" lamp: every mailbox byte is a live SPI transfer, so the card is being accessed. Light
	// the LED and (re)arm the one-shot; a burst of bytes keeps it steady, and it drops ~250 ms after the last.
	m_sd_leds[0] = 1;
	m_sd_inuse_off->adjust(attotime::from_msec(250));

	if (m_sdcard)
	{
		m_sdmbx_miso = 0;
		for (uint8_t bit = 0x80; bit; bit >>= 1)
		{
			m_sdcard->spi_clock_w(CLEAR_LINE);
			m_sdcard->spi_mosi_w((data & bit) ? 1 : 0);
			m_sdcard->spi_clock_w(ASSERT_LINE);
		}
		m_sdmbx_out = m_sdmbx_miso;
	}
	intc_assert(0x1C);
}

TIMER_CALLBACK_MEMBER(kn7000_state::sd_insert)
{
	sd_update_carddetect();
}

// SD "in use" one-shot expiry: no SPI byte for ~250 ms -> the card is idle, clear the lamp.
TIMER_CALLBACK_MEMBER(kn7000_state::sd_inuse_off)
{
	m_sd_leds[0] = 0;
}

INPUT_CHANGED_MEMBER(kn7000_state::sd_cover_changed)
{
	sd_update_carddetect();
}

// A PC-key note press or release: push a voice event into the key-bed FIFO the
// firmware polls. param carries the key index; velocity is fixed for host keys.
INPUT_CHANGED_MEMBER(kn7000_state::kbd_key)
{
	kbd_push(newval ? uint8_t(param) : uint8_t(param | 0x80), newval ? 0x64 : 0xff);
}

//  SIO -- three on-chip USART channels (panel + two MIDI ports)
//

// The main-CPU SIO channel-0 sync-transfer completion, INTC group 0x11. Deferred
// ~40us so the panel's reply cannot overtake the ISR that acknowledges it.
TIMER_CALLBACK_MEMBER(kn7000_state::panel_txdone_cb)
{
	m_c11_unserviced = true;
	intc_assert(0x11);
}

// Front-panel MAIN VOLUME slider -> master output gain on the final mix. A squared
// taper approximates a natural volume law (the exact analog-slider taper is unknown);
TIMER_CALLBACK_MEMBER(kn7000_state::volume_scan)
{
	const float v = float(m_volmain->read()) / 100.0f;
	m_tonegen->set_output_gain(ALL_OUTPUTS, v * v);

	m_sd_leds[1] = (m_lcdbuf[0x00C00008 >> 2] & 0xC0) ? 1 : 0;
}

static INPUT_PORTS_START(kn7000)

	PORT_START("DIAL")
	PORT_BIT(0xff, 0x00, IPT_DIAL) PORT_SENSITIVITY(30) PORT_KEYDELTA(1) PORT_NAME("DATA DIAL")

	PORT_START("VOL_MAIN")   PORT_ADJUSTER(80, "Main Volume")
	PORT_START("VOL_APCSEQ") PORT_ADJUSTER(80, "APC / SEQ Volume")
	PORT_START("VOL_MIC")    PORT_ADJUSTER(50, "Mic Volume")
	PORT_START("VOL_LINEIN") PORT_ADJUSTER(50, "Line-In Volume")
	PORT_START("TEMPO_KNOB") PORT_ADJUSTER(50, "Tempo / Program Knob")

	// Rear-panel MIDI IN / BASS PEDAL selector switch, SW701 on the JACK board.
	PORT_START("REARSW")
	PORT_CONFNAME(0x1000, 0x1000, "Rear panel: MIDI IN / BASS PEDAL selector (SW701)")
	PORT_CONFSETTING(0x1000, "MIDI IN")
	PORT_CONFSETTING(0x0000, "Bass Pedals")

	// SD front-panel switches (CPSD-side matrix, byte 0x9CC00008 ACTIVE-LOW,
	// bits 0-5 -> panel events 0x7020B5..0x7020BA per descriptor SEG1D @0x48613fc4).
	PORT_START("CPSD_SDSW")   // SD play/vol board -- GPIO 0x9CC00008, not on the CP serial link
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SD VOLUME -")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SD VOLUME +")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SD STOP")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SD PLAY/PAUSE")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SD SKIP/SEARCH <<")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SD SKIP/SEARCH >>")

	// SD slot cover switch. The slot has a hinged cover and the firmware only looks
	// for a card while it is closed.
	PORT_START("SDCOVER")
	PORT_CONFNAME(0x01, 0x00, "SD slot cover") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::sd_cover_changed), 0)
	PORT_CONFSETTING(   0x00, "Closed")
	PORT_CONFSETTING(   0x01, "Open")

	// Key bed: the PORT_CHANGED_MEMBER parameter is the key index, which is the
	// GM note minus 36.
	PORT_START("KEYS0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_C2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x00)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_CS2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x01)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_D2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x02)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_DS2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x03)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_E2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x04)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_F2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x05)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_FS2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x06)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_G2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x07)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_GS2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x08)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_A2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x09)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_AS2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x0A)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_B2 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x0B)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_C3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x0C)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_CS3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x0D)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_D3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x0E)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_DS3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x0F)
	PORT_START("KEYS1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_E3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x10)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_F3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x11)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_FS3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x12)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_G3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x13)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_GS3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x14)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_A3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x15)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_AS3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x16)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_B3 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x17)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_C4 PORT_CODE(KEYCODE_Z) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x18)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_CS4 PORT_CODE(KEYCODE_S) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x19)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_D4 PORT_CODE(KEYCODE_X) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x1A)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_DS4 PORT_CODE(KEYCODE_D) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x1B)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_E4 PORT_CODE(KEYCODE_C) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x1C)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_F4 PORT_CODE(KEYCODE_V) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x1D)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_FS4 PORT_CODE(KEYCODE_G) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x1E)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_G4 PORT_CODE(KEYCODE_B) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x1F)
	PORT_START("KEYS2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_GS4 PORT_CODE(KEYCODE_H) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x20)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_A4 PORT_CODE(KEYCODE_N) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x21)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_AS4 PORT_CODE(KEYCODE_J) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x22)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_B4 PORT_CODE(KEYCODE_M) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x23)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_C5 PORT_CODE(KEYCODE_Q) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x24)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_CS5 PORT_CODE(KEYCODE_2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x25)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_D5 PORT_CODE(KEYCODE_W) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x26)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_DS5 PORT_CODE(KEYCODE_3) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x27)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_E5 PORT_CODE(KEYCODE_E) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x28)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_F5 PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x29)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_FS5 PORT_CODE(KEYCODE_5) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x2A)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_G5 PORT_CODE(KEYCODE_T) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x2B)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_GS5 PORT_CODE(KEYCODE_6) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x2C)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_A5 PORT_CODE(KEYCODE_Y) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x2D)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_AS5 PORT_CODE(KEYCODE_7) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x2E)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_B5 PORT_CODE(KEYCODE_U) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x2F)
	PORT_START("KEYS3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_C6 PORT_CODE(KEYCODE_I) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x30)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_CS6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x31)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_D6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x32)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_DS6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x33)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_E6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x34)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_F6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x35)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_FS6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x36)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_G6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x37)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_GS6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x38)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_A6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x39)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_AS6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x3A)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_B6 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x3B)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_GM_C7 PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kn7000_state::kbd_key), 0x3C)
INPUT_PORTS_END

uint32_t kn7000_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// Present the firmware's OWN composited image -- the exact bytes the real LCD
	// controller scans; the firmware composites the planes in software.
	constexpr offs_t LCD = (0x9ce00000 - 0x9c000000) / 4;      // word offset of the framebuffer in the 0x9c RAM

	// KN2400/KN2600: a 320x240 FOUR-LEVEL GRAYSCALE panel. The firmware composites into a
// 2bpp buffer at 0x9C800000 (stride 80 bytes, MSB-first pixel pairs, 0 = lightest).
	if (m_lcd_kn24)
	{
		constexpr offs_t LCD24 = (0x9c800000 - 0x9c000000) / 4;
		for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		{
			uint32_t *const dst = &bitmap.pix(y);
			for (int x = cliprect.left(); x <= cliprect.right(); x++)
			{
				const offs_t k = offs_t(y) * 320 + x;                        // linear pixel index
				const uint32_t w = m_lcdbuf[LCD24 + (k >> 4)];               // 16 px per 32-bit word
				const uint8_t byte = uint8_t(w >> ((k & 0xc) << 1));         // little-endian byte within the word
				const uint8_t v = (byte >> (6 - 2 * (k & 3))) & 3;           // MSB-first 2-bit pixel
				const uint8_t g = 0xff - v * 0x55;
				dst[x] = rgb_t(g, g, g);
			}
		}
		return 0;
	}
	const bool kn6 = m_lcd_kn6;
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint32_t *const dst = &bitmap.pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			const offs_t k = kn6 ? offs_t(239 - y) * 640 + (639 - x) : offs_t(y) * 640 + x;   // linear pixel index
			const uint32_t w = m_lcdbuf[LCD + (k >> 1)];
			const uint16_t v = (k & 1) ? uint16_t(w >> 16) : uint16_t(w);   // little-endian
			dst[x] = kn6
				? rgb_t(((v >> 10) & 0x1f) << 3, ((v >> 5) & 0x1f) << 3, (v & 0x1f) << 3)    // RGB555
				: rgb_t(((v >> 11) & 0x1f) << 3, ((v >> 5) & 0x3f) << 2, (v & 0x1f) << 3);   // RGB565
		}
	}
	return 0;
}

uint32_t kn7000_state::customflash_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0;
	if (ACCESSING_BITS_0_15)
		data |= m_customflash->read(offset * 2);
	if (ACCESSING_BITS_16_31)
		data |= uint32_t(m_customflash->read(offset * 2 + 1)) << 16;
	return data;
}

void kn7000_state::customflash_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_15)
		m_customflash->write(offset * 2, data & 0xffff);
	if (ACCESSING_BITS_16_31)
		m_customflash->write(offset * 2 + 1, (data >> 16) & 0xffff);
}

void kn7000_state::machine_start()
{
	// output_finders auto-resolve in this MAME version (see kn5000_cpanel) --
	// no explicit resolve() call is needed or available.

	if (m_customflash)
	{
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x96800000, 0x969fffff,
				read32s_delegate(*this, FUNC(kn7000_state::customflash_r)),
				write32s_delegate(*this, FUNC(kn7000_state::customflash_w)));
		logerror("custom-data flash: IC21 mapped at 0x96800000\n");
	}

	m_waverom[0] = memregion("waveform_main_y");
	m_waverom[1] = memregion("waveform_main_x");
	m_waverom[2] = memregion("waveform_sub_y");
	m_waverom[3] = memregion("waveform_sub_x");

	save_item(NAME(m_tg_wave_bank));
	save_item(NAME(m_tg_wave_addr));

	// Periodic control-panel button scan (the real sub-CPUs poll their matrices
	// continuously and report changes over the serial link).
	m_panel_txdone = timer_alloc(FUNC(kn7000_state::panel_txdone_cb), this);
	m_vol_timer = timer_alloc(FUNC(kn7000_state::volume_scan), this);

	// (The AM33 maskable-interrupt vectors are configured per level on the core
	// in kn7000/kn6000(machine_config) -- set_maskable_vector.)

	if (m_lib_mirror)
		memcpy(memshare("libram")->ptr(), memregion("program")->base(), memregion("program")->bytes());
	m_sys_timer = timer_alloc(FUNC(kn7000_state::sys_tick), this);
	m_sd_insert_timer = timer_alloc(FUNC(kn7000_state::sd_insert), this);
	m_sd_inuse_off = timer_alloc(FUNC(kn7000_state::sd_inuse_off), this);

	// (INTC + TM5 timer state is save_item'd by the MN10300 core now.)
	save_item(NAME(m_c11_unserviced));
	save_item(NAME(m_extmd_prev));
	// (SIO channel state is save_item'd by the MN10300 core now.)
	save_item(NAME(m_sdspi_rate));
	save_item(NAME(m_tg_addr));
}

void kn7000_state::machine_reset()
{

	// Poll the MAIN VOLUME slider -> output gain at ~250 Hz.
	m_vol_timer->adjust(attotime::from_hz(250), 0, attotime::from_hz(250));

	// System tick, ~1 kHz. The real rate is unknown; the input clock has not been
	// identified. The interrupt dispatches to the firmware's RTOS handler.
	if (m_lib_mirror)
		// KN6000/KN6500: delay the tick past the single-threaded part of the boot, so
		// the scheduler exists by the time the first one arrives.
		m_sys_timer->adjust(attotime::from_seconds(2), 0, attotime::from_hz(1000));
	else
		m_sys_timer->adjust(attotime::from_hz(1000), 0, attotime::from_hz(1000));

	// (TM5 mode/base/countdown are reset by the core's device_reset.)
	if (!m_lib_mirror)
	{
		// SD card-detect: the polled group-0x1B ICR (0x3400016C) bit4 reads
		// 1 = no card, 0 = card present.
		m_maincpu->intc_icr_set(0x1B, 0x0012);
		const bool cover_open = (m_sdcover->read() & 1) != 0;
		if (!cover_open && m_sdcard && m_sdcard->get_card_present())
			m_sd_insert_timer->adjust(attotime::from_seconds(6));
		else
			m_sd_insert_timer->adjust(attotime::never);
		m_gpio8004 = 0xFFFF;                            // CS released (bit1=1) at reset
		if (m_sdcard)
			m_sdcard->spi_ss_w(0);                      // deselected until the firmware asserts CS (0x36008004 bit1)
	}
}

// ================= FDC (IC103, C1DB00000607, N82077AA-compatible) at 0x98020000 =================
// Confirmed by the SX-KN7000 service-manual schematic (chip-select decoder IC1 TC74VHC138F, page 101:
uint8_t kn7000_state::fdc_r(offs_t off)
{
	if (!m_fdc) return 0xff;
	switch (off)
	{
	case 0x4: return m_fdc->dor_r();    // reg2 DOR
	case 0x8: return m_fdc->msr_r();    // reg4 Main Status Register
	case 0xa: return m_fdc->fifo_r();   // reg5 data FIFO
	case 0xe: return m_fdc->dir_r();    // reg7 DIR (bit7 = disk-change)
	default:  return 0xff;
	}
}

void kn7000_state::fdc_w(offs_t off, uint8_t data)
{
	if (!m_fdc) return;
	switch (off)
	{
	case 0x4: m_fdc->dor_w(data);  break;   // reg2 DOR (motor / drive-select / /RESET / DMA gate)
	case 0x8: m_fdc->dsr_w(data);  break;   // reg4 Data-rate Select Register
	case 0xa: m_fdc->fifo_w(data); break;   // reg5 data FIFO
	case 0xe: m_fdc->ccr_w(data);  break;   // reg7 Configuration Control Register (data rate)
	default: break;
	}
}

// FDC interrupt and DMA-request lines -> on-chip INTC group 0x18. The sector-data
// phase is software DMA: the FDC asserts DRQ and the handler moves one byte.
void kn7000_state::fdc_irq_w(int state)
{
	if (state)
		intc_assert(0x18);
}
void kn7000_state::fdc_drq_w(int state)
{
	if (state)
		intc_assert(0x18);
}
// FDC.DACK byte slot at 0x98010000 (decoder Y1). A read/write here transfers one FIFO byte to/from the
// FDC in the current DMA operation (asserts DACK); the software-DMA ISR uses it per FDC.DRQ.
uint8_t kn7000_state::fdc_dma_r(offs_t)
{
	return m_fdc ? m_fdc->dma_r() : 0xff;
}
void kn7000_state::fdc_dma_w(offs_t, uint8_t data)
{
	if (m_fdc)
		m_fdc->dma_w(data);
}

static void kn7000_floppies(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);   // 3.5" high-density (1.44 MB, the KN7000 default)
	device.option_add("35dd", FLOPPY_35_DD);   // 3.5" double-density (720 KB, 2DD media)
}

void kn7000_state::kn7000_base(machine_config &config)
{
	// IC21, the custom-data flash. The firmware identifies the part with a JEDEC
	// autoselect before writing to it.
	FUJITSU_29LV160B(config, m_customflash);

	MN103002A(config, m_maincpu, 16_MHz_XTAL * 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &kn7000_state::maincpu_mem);
	// The MN10300 always resets to 0x40000000; this is where the board answers
	// that, and the flash does begin with the vector table it implies.
	m_maincpu->set_reset_pc(0x48400000);

	m_maincpu->set_vector_base(IRQ_VECTOR_BASE);

	// Group 0x11 (panel transfer complete) is level-like until serviced: if a
	// completion landed before the ISR's ack wiped it, re-deliver it after.
	m_maincpu->intc_ack_cb().set([this](uint8_t group) {
		if (group == 0x11 && m_c11_unserviced)
			m_panel_txdone->adjust(attotime::from_usec(40), 3);
	});
	// group 0x11 latched into the IAGR at accept -> it has been serviced.
	m_maincpu->intc_accept_cb().set([this](uint8_t group) {
		if (group == 0x11)
			m_c11_unserviced = false;
	});
	// EXTMD written: decode the panel-ATN edge re-arm transition (bits 7:6
	// 11b -> 10b) against our previous-value shadow.
	m_maincpu->intc_extmd_cb().set([this](uint16_t data) {
		const uint16_t prev = m_extmd_prev;
		m_extmd_prev = data;
		if (((prev & 0x00c0) == 0x00c0) && ((data & 0x00c0) == 0x0080))
			m_cpanel->atn_rearm();
	});

	// On-chip SIO routing; the register model itself lives in the CPU core.
	m_maincpu->sio_tx_done_cb<0>().set([this](int state) {
		// Sync-transfer completion -> group 0x11, always deferred (see panel_txdone_cb).
		m_panel_txdone->adjust(attotime::from_usec(40), 3);
	});
	// The main CPU transmits 7-byte frames with interleaved line syncs; the
	// panel HLE parses them, decodes LED writes, and queues replies.
	m_maincpu->sio_tx_cb<0>().set([this](uint8_t data) { m_cpanel->tx_byte(data); });
	// One group-0x10 interrupt per reply byte the panel delivers into the ch0
	// RX ring (the state-8 handler reads 0x34000809 per interrupt).
	m_maincpu->sio_rx_rdy_cb<0>().set([this](int state) { intc_assert(0x10); });
	// Config bit14 written set (group-0x1A ISR pass 2): the panel may now send
	// its queued reply.
	m_maincpu->sio_rx_enable_cb<0>().set([this](int state) { m_cpanel->rx_enable(); });
	SCREEN(config, m_screen).set_lcd();
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	// 640x240 8bpp LCD (proven from the blitter stride 0x280 and height 0xF0).
	m_screen->set_size(640, 240);
	m_screen->set_visarea(0, 640 - 1, 0, 240 - 1);
	m_screen->set_screen_update(FUNC(kn7000_state::screen_update));

	KN7000_CPANEL(config, m_cpanel);
	m_cpanel->atn().set([this](int state) { if (state) intc_assert(0x1a); });
	m_cpanel->rxd().set([this](uint8_t data) { m_maincpu->sio_rx_push(SIO_PANEL, data); });
	m_cpanel->set_dial_port(m_dial);
	m_cpanel->set_volapcseq_port(m_volapcseq);
	m_cpanel->set_tempoknob_port(m_tempoknob);

	// --- Sound. Shared by every model that reuses this config.
	SPEAKER(config, "speaker", 2).front();
	// Every model reusing this config has the floppy drive; the ones without an
	// SD slot remove it below.
	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->set_prefer_sd();
	m_sdcard->spi_miso_callback().set(FUNC(kn7000_state::sd_miso_w));

	// IC103 floppy disk controller (custom C1DB00000607, N82077AA/PC-AT-compatible) + 3.5" drive.
	N82077AA(config, m_fdc, 24'000'000);
	m_fdc->intrq_wr_callback().set(FUNC(kn7000_state::fdc_irq_w));   // FDC INTRQ (logging stub -- firmware polls MSR)
	m_fdc->drq_wr_callback().set(FUNC(kn7000_state::fdc_drq_w));     // FDC DRQ  (logging stub -- DMA not yet modelled)
	FLOPPY_CONNECTOR(config, "fdc:0", kn7000_floppies, "35hd", floppy_image_device::default_pc_floppy_formats).enable_sound(true);

	KN7000_TONEGEN(config, m_tonegen, 0);
	m_tonegen->add_route(0, "lspeaker", 1.0);
	m_tonegen->add_route(1, "rspeaker", 1.0);

	// TODO: real tone generators IC201/IC205; the effects DSP and its SDRAM IC307/8; USB.
}

void kn7000_state::kn7000(machine_config &config)
{
	kn7000_base(config);
	// Clickable front-panel artwork: buttons bound to the ioports, LEDs bound to
	// the cpl_led/cpc_led/cpr_led outputs.
	config.set_default_layout(layout_kn7000);
}

void kn7000_state::kn6000(machine_config &config)
{
	kn7000_base(config);
	m_lib_mirror = true;
	m_lcd_kn6 = true;
	// The KN6000/KN6500 front panel speaks the same CP wire protocol as the KN7000,
	// so only the matrix geometry and the LED decode differ.
	config.device_remove("tonegen");
	KN6000_TONEGEN(config, m_tonegen, 0);
	m_tonegen->add_route(0, "lspeaker", 1.0);
	m_tonegen->add_route(1, "rspeaker", 1.0);
	// The KN6000 and KN6500 have the floppy drive but no SD slot.
	config.device_remove("sdcard");

	config.device_remove("cpanel");
	KN6000_CPANEL(config, m_cpanel);
	m_cpanel->atn().set([this](int state) { if (state) intc_assert(0x1a); });
	m_cpanel->rxd().set([this](uint8_t data) { m_maincpu->sio_rx_push(SIO_PANEL, data); });
	m_cpanel->set_dial_port(m_dial);
	m_cpanel->set_volapcseq_port(m_volapcseq);
	m_cpanel->set_tempoknob_port(m_tempoknob);

	// No artwork: the KN6000/KN6500 panel layout is not finished, and the KN7000's
	// would put every clickable element at the wrong place with the wrong binding.
}

void kn7000_state::kn6500(machine_config &config)
{
	kn6000(config);
}

void kn7000_state::kn24_base(machine_config &config)
{
	kn7000_base(config);
	m_lcd_kn24 = true;
	// 320x240 4-level grayscale LCD (2bpp framebuffer at 0x9C800000).
	m_screen->set_size(320, 240);
	m_screen->set_visarea(0, 320 - 1, 0, 240 - 1);
}

// IC404, the SD sub-CPU program, is listed "KN2600 only": the KN2400 has the
// floppy drive and no SD slot, the KN2600 has both.
void kn7000_state::kn2400(machine_config &config)
{
	kn24_base(config);
	config.device_remove("sdcard");
}

void kn7000_state::kn2600(machine_config &config)
{
	kn24_base(config);
}

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

	ROM_REGION16_BE(0x200000, "custom_data", ROMREGION_ERASEFF)
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
	ROM_REGION16_BE(0x200000, "custom_data", ROMREGION_ERASEFF)
	ROM_LOAD("01ctmini.ic18", 0x020000, 0x1e0000, BAD_DUMP CRC(f108e4c7) SHA1(8c6d62a8afab717a2b59e9242bbf897b01369416))
ROM_END

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
	ROM_REGION16_BE(0x200000, "custom_data", ROMREGION_ERASEFF)
	ROM_LOAD("01ctmini.ic18", 0x020000, 0x1e0000, BAD_DUMP CRC(f108e4c7) SHA1(8c6d62a8afab717a2b59e9242bbf897b01369416))
ROM_END

#define KN2400_ROM_COMMON \
	ROM_REGION32_LE(0x400000, "table_data", ROMREGION_ERASEFF) \
 \
	ROM_REGION32_LE(0x400000, "program", 0) \
	ROM_LOAD32_WORD("kn2400_program_even.ic13", 0x000000, 0x200000, CRC(b94fc8a8) SHA1(86d5d9916afdb90f82de78064b1d76fce3a21d7b)) \
	ROM_LOAD32_WORD("kn2400_program_odd.ic12",  0x000002, 0x200000, CRC(73781cbc) SHA1(d90a3560561efd94322dca1a6710f2d5d3837cd2)) \
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

//   YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY     FULLNAME      FLAGS
SYST(2002, kn7000, 0,      0,      kn7000,  kn7000, kn7000_state, empty_init, "Technics", "SX-KN7000", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// KN6000 / KN6500 reuse the KN7000 machine config: same MN10300, same ROM base.
SYST(1999, kn6000, 0,      0,      kn6000,  kn7000, kn7000_state, empty_init, "Technics", "SX-KN6000", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
// The KN6500 is the KN6000 with one constant changed, tone generator included.
SYST(2001, kn6500, 0,      0,      kn6500,  kn7000, kn7000_state, empty_init, "Technics", "SX-KN6500", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// KN2400 / KN2600 -- MN10300/MILK siblings sharing one firmware image (kn2600 = clone of kn2400).
SYST(2000, kn2400, 0,      0,      kn2400,  kn7000, kn7000_state, empty_init, "Technics", "SX-KN2400", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
SYST(2000, kn2600, kn2400, 0,      kn2600,  kn7000, kn7000_state, empty_init, "Technics", "SX-KN2600", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

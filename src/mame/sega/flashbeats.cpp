// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    drivers/flashbeats.cpp
    Sega "Flash Beats" game

    Hardware:
        - H8/3007 CPU
        - 68000 + SCSP for sound effects
        - DSB2 MPEG board with another 68000
        - Two Sega I/O chips: 315-5338A and 315-5296
        - Pinball-style 64x16 dot-matrix display
        - 5 display tubes containing an unknown number of RGB LEDs behind
          a diffuser

****************************************************************************/

#include "emu.h"

#include "315_5296.h"
#include "315_5338a.h"
#include "dsb2.h"

#include "cpu/h8/h83006.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "machine/te7750.h"
#include "machine/timer.h"
#include "sound/scsp.h"

#include "speaker.h"

#include "endianness.h"

#include <algorithm>
#include <cmath>
#include <numbers>

#include "flsbeats.lh"


namespace {

// Audio spectrum-analyzer feedback tap.
//
// The cabinet loops its own audio back into the H8/3007's on-chip A/D
// converter: an analog mux (select latch at 0xa18000, low 3 bits) feeds one
// of five stereo level signals to AN0/AN1 (left/right pair). The firmware's
// sampler (jump table at 0x2302 -> ADC poll at 0x2418) round-robins the five
// mux selects (2,1,3,7,4) at ~94Hz each, storing the AN0/AN1 conversions
// into five per-source slots at 0xfff380..0xfff3c8. The attract-mode meter
// renderer (0xf9d8) maps one slot pair per lane tube -- five *different*
// stereo signals -- and the real cabinet's lanes visibly track different
// parts of the music (owner-confirmed), so the analog stage is a five-band
// filter bank on the amp output: the lane show is a spectrum analyzer.
// With nothing bound to the ADC it converts a constant and the meters stay
// dead.
//
// The per-band sample rate is far below audio rate, so each band must be
// rectified and smoothed to a DC level before the mux: modeled as a biquad
// band-pass into an ideal peak rectifier with an RC release. Band centers
// were fitted by correlating per-tube bar levels in real-cabinet attract
// footage against band-filtered soundtrack energy; Q and ballistics are
// guesses (no schematics), affecting only meter feel.
class flashbeats_spectrum_device : public device_t, public device_sound_interface
{
public:
	static constexpr int BANDS = 5;

	flashbeats_spectrum_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// current rectified band level as the 10-bit unsigned value the H8 ADC
	// expects (0 = silence, 0x3ff = full scale). The rectifier's output gain
	// stands in for the filter board's op-amp stage: the meter firmware uses
	// the ADC result's top byte directly as an LED count and saturates a
	// half-bar at 21, so full deflection is only ~8% of ADC full scale.
	// 4.22 was calibrated by matching the emulated byte distribution to
	// bar-length measurements from cabinet footage of the same attract
	// segment (distributions align at the 90th percentile).
	u16 adc_r(int band, int channel)
	{
		m_stream->update();
		return u16(std::clamp(m_env[band][channel] * 4.22, 0.0, 1.0) * 0x3ff);
	}

protected:
	virtual void device_start() override ATTR_COLD
	{
		save_item(NAME(m_env));
		save_item(NAME(m_z1));
		save_item(NAME(m_z2));
		m_stream = stream_alloc(2, 0, SAMPLE_RATE_INPUT_ADAPTIVE);
	}

	virtual void sound_stream_update(sound_stream &stream) override
	{
		if (stream.sample_rate() != m_rate)
		{
			m_rate = stream.sample_rate();
			recompute_filters();
		}

		const double release = std::exp(-1.0 / (0.050 * m_rate));   // ~50ms RC release
		for (int ch = 0; ch < 2; ch++)
			for (int b = 0; b < BANDS; b++)
			{
				double z1 = m_z1[b][ch], z2 = m_z2[b][ch], env = m_env[b][ch];
				for (int i = 0; i < stream.samples(); i++)
				{
					const double x = stream.get(ch, i);
					const double y = m_b0[b] * x + z1;
					z1 = -m_a1[b] * y + z2;
					z2 = m_b2[b] * x - m_a2[b] * y;
					const double s = std::fabs(y);
					env = (s > env) ? s : env * release;
				}
				m_z1[b][ch] = z1;
				m_z2[b][ch] = z2;
				m_env[b][ch] = env;
			}
	}

private:
	// RBJ-cookbook 0dB-peak band-pass biquads (transposed direct form II,
	// b1 = 0), centers spread low->high across the lanes
	void recompute_filters()
	{
		static constexpr double CENTER[BANDS] = { 63.0, 250.0, 1000.0, 2500.0, 10000.0 };
		static constexpr double Q = 1.0;
		for (int b = 0; b < BANDS; b++)
		{
			const double w0 = 2.0 * std::numbers::pi * std::min(CENTER[b] / m_rate, 0.45);
			const double alpha = std::sin(w0) / (2.0 * Q);
			const double norm = 1.0 / (1.0 + alpha);
			m_b0[b] = alpha * norm;
			m_b2[b] = -alpha * norm;
			m_a1[b] = -2.0 * std::cos(w0) * norm;
			m_a2[b] = (1.0 - alpha) * norm;
		}
	}

	sound_stream *m_stream;
	u32 m_rate;
	double m_b0[BANDS], m_b2[BANDS], m_a1[BANDS], m_a2[BANDS];
	double m_z1[BANDS][2], m_z2[BANDS][2];
	double m_env[BANDS][2];
};

} // anonymous namespace

// device_finder explicit instantiations require global scope
DEFINE_DEVICE_TYPE(FLASHBEATS_SPECTRUM, flashbeats_spectrum_device, "flashbeats_spectrum", "Flash Beats spectrum display filter board")

namespace {

flashbeats_spectrum_device::flashbeats_spectrum_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, FLASHBEATS_SPECTRUM, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_stream(nullptr),
	m_rate(0),
	m_b0{}, m_b2{}, m_a1{}, m_a2{},
	m_z1{}, m_z2{},
	m_env{}
{
}


class flashbeats_state : public driver_device
{
	// Flash Beats display geometry (recovered by RE; see notes at update_dmd).
	// The DMD is natively 64x16 -- exactly matching the display RAM -- NOT the
	// 128x32 the physical panel's dot count might suggest. Each dot is one byte
	// (low nibble = brightness 0..15) on a 2-byte stride ([value][pad]).
	static constexpr int DMD_W = 64;
	static constexpr int DMD_H = 16;
	static constexpr int LANE_COUNT = 5;
	static constexpr int LANE_LEN = 47;        // LEDs per lane (even bytes 0..92 of each 0x60 row at 0xa0c000)

	// DMD framebuffer: sub 0x75ec copies shifting slices of the 0xa02800 staging
	// canvas (itself filled by the sprite-blit at 0x749e) into 0xa00000, one row
	// per DMD_PITCH bytes, dispatched by 0x7642(r5=0) from the scene handler's
	// per-tick scroll loop.
	static constexpr offs_t DMD_BASE  = 0xa00000;
	static constexpr offs_t DMD_PITCH = 0x80;    // bytes/row = DMD_W * 2

public:
	flashbeats_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_scspcpu(*this, "scspcpu"),
		m_scsp(*this, "scsp"),
		m_spectrum(*this, "spectrum"),
		m_sound_ram(*this, "sound_ram"),
		m_dsb2(*this, "dsb2"),
		m_eeprom(*this, "eeprom"),
		m_315_5296(*this, "segaio1"),
		m_315_5338a(*this, "segaio2"),
		m_dispram(*this, "dispram"),
		m_dmd(*this, "dmddot%u", 0U),
		m_lane_red(*this, "lane%u_%u_r", 0U, 0U),
		m_lane_green(*this, "lane%u_%u_g", 0U, 0U),
		m_btn_lamp(*this, "btnlamp%u", 0U),
		m_lamp_flash(*this, "lamp_flash"),
		m_lamp_up(*this, "lamp_up"),
		m_lamp_dn(*this, "lamp_dn"),
		m_lamp_start(*this, "lamp_start")
	{
	}

	void flashbeats(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void flashbeats_map(address_map &map) ATTR_COLD;
	void main_scsp_map(address_map &map) ATTR_COLD;
	void scsp_mem(address_map &map) ATTR_COLD;

	void scsp_irq(offs_t offset, uint8_t data);

	// H8 SCI serial -> sound boards
	// The H8 drives two on-chip SCI channels in internal-async mode (~31250 bps,
	// MIDI rate) as queue-driven transmitters (RE by disasm, routines @0x2540 /
	// 0x25e0). Channel roles, confirmed by disasm + queue-fill correlation:
	//   * SCI1 -> SCSP MIDI-in : sound EFFECTS. Fires per game event (every menu
	//     blip, every rally hit). The upstream SCSP is itself a serial device
	//     (midi_in(int state) -> rx_w), so SCI1's per-bit TX line wires straight
	//     into it in machine_config -- no reassembly (model2/3 do the same).
	//   * SCI0 -> DSB2 (MPEG music). Fires rarely (track changes). DSB2's
	//     write_txd() is already bit-level (feeds an internal i8251), so SCI0's
	//     TX line connects to it directly in machine_config -- no reassembly.
	// Without these wires both sound CPUs sit in wait-loops forever (the SCSP
	// 68000 spins on sound_ram+0x300 play-flags its own MIDI ISR would set), and
	// the H8's TX queues fill but never drain (TDRE never recycles with the TX
	// pin unconnected)

	uint8_t p6_r();
	void p6_w(uint8_t data);
	// H8 ADC channels AN0/AN1 = left/right level of the filter-bank band
	// currently selected by the 0xa18000 mux latch. Band assignment measured
	// by correlating per-tube bar levels in real-cabinet attract footage
	// against band-filtered energy of its soundtrack: the tubes run low band
	// (top) to high band (bottom), and the firmware's sampler pairs lanes
	// 0..4 with mux selects 4,2,1,3,7 respectively.
	int spectrum_band() const
	{
		static constexpr int8_t BAND_FOR_MUX[8] = { 2, 2, 1, 3, 0, 2, 2, 4 };
		return BAND_FOR_MUX[m_spectrum_mux & 7];
	}
	uint16_t adc0_r() { return m_spectrum->adc_r(spectrum_band(), 0); }
	uint16_t adc1_r() { return m_spectrum->adc_r(spectrum_band(), 1); }
	uint8_t spectrum_mux_r();
	void spectrum_mux_w(uint8_t data);
	void update_lanes();
	void update_dmd();
	TIMER_DEVICE_CALLBACK_MEMBER(lane_update_timer);
	void te7752_port3_w(uint8_t data);
	void te7752_port4_w(uint8_t data);
	void te7752_port5_w(uint8_t data);
	void te7752_port6_w(uint8_t data);

	required_device<h83007_device> m_maincpu;
	required_device<m68000_device> m_scspcpu;
	required_device<scsp_device> m_scsp;
	required_device<flashbeats_spectrum_device> m_spectrum;
	required_shared_ptr<uint16_t> m_sound_ram;
	required_device<dsb2_device> m_dsb2;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<sega_315_5296_device> m_315_5296;
	required_device<sega_315_5338a_device> m_315_5338a;
	required_shared_ptr<uint16_t> m_dispram;   // 0xa00000 display work RAM (incl. lanes @0xa0c000)
	output_finder<DMD_W * DMD_H> m_dmd;   // 64x16 DMD dots (dmddot0..1023) -> .lay artwork
	output_finder<LANE_COUNT, LANE_LEN> m_lane_red;  // red plane -> .lay artwork
	output_finder<LANE_COUNT, LANE_LEN> m_lane_green;  // green plane -> .lay artwork
	output_finder<10> m_btn_lamp;   // btnlamp0-9 -> .lay (0-4 = P1 lanes 1-5, 5-9 = P2 lanes 1-5)
	output_finder<> m_lamp_flash;   // lamp_flash -> .lay (gameplay cue: return on the FLASH)
	output_finder<> m_lamp_up, m_lamp_dn, m_lamp_start;   // lamp_up/dn/start -> .lay

	uint8_t m_spectrum_mux = 0;   // analog spectrum-display source select latch @0xa18000
};


void flashbeats_state::machine_start()
{
	save_item(NAME(m_spectrum_mux));
}

void flashbeats_state::machine_reset()
{
	uint8_t *ROM = memregion("scspcpu")->base();
	memcpy(m_sound_ram, ROM, 0x400);
	m_scspcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_scspcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

// Publish the 5 LED lanes as named artwork outputs, consumed by the .lay. The
// lanes are built from TWO on/off planes in display RAM, both 5 rows of 
// 0x60 (96) bytes (one row per lane), cells at even byte offsets (2-byte stride
// [value][pad]):
//   * P0 (0xa0c000): the RED content
//   * P1 (0xa0c1e0): the GREEN content
// The two planes are independent; the game routinely sets both for the same LED.
// Each LED is published as TWO independent on/off outputs -- the red die
// (lane<r>_<c>_r) and the green die (lane<r>_<c>_g) -- and the .lay overlays them
// with blend="add", so an LED with both dies lit sums to yellow the same way the
// real diffused bi-color package does.
//
// PLANE WIDTHS DIFFER -> 47 LEDs, adjacent-OR green:
// Measured across all attract/meter frames, the RED plane lights cells 0..46 (47
// cells, 1:1 with the 47 physical LEDs) but the GREEN plane lights cells 0..47
// (48 cells). The 48-cell green source maps onto the 47 LEDs by OR-ing each cell
// with its neighbour: LED_green[i] = green_cell[i] | green_cell[i+1].
//
// Big-endian H8: byte at even address = high byte of the 16-bit display word.
void flashbeats_state::update_lanes()
{
	auto const disp8 = util::big_endian_cast<uint8_t const>(m_dispram.target());

	for (int lane = 0; lane < LANE_COUNT; lane++)
	{
		for (int i = 0; i < LANE_LEN; i++)
		{
			const offs_t off = lane * 0x60 + i * 2;
			const bool red   = disp8[(0xa0c000 + off) & 0xffff] != 0;   // P0: 47 cells, 1:1 per LED
			// P1: 48 green cells OR'd down to 47 LEDs (this cell OR the next).
			const bool green = disp8[(0xa0c1e0 + off) & 0xffff] != 0 || disp8[(0xa0c1e0 + off + 2) & 0xffff] != 0;
			m_lane_red[lane][i]   = red   ? 1 : 0;
			m_lane_green[lane][i] = green ? 1 : 0;
		}
	}
}

// Flash Beats has no conventional raster screen; it has a 64x16 pinball-style
// dot-matrix display and 5 RGB-LED "tube" lanes, both built by the H8 in work
// RAM and both published as named artwork outputs, drawn by the .lay.
//
// The main-loop scene handlers compose source art into a staging canvas at
// 0xa02800 via the sprite blitter at 0x749e, but that canvas only updates when
// a scene loads fresh art -- it is NOT the live panel. The real,
// continuously-scanned panel is 0xa00000: sub 0x75ec copies shifting slices of
// 0xa02800 into 0xa00000 every scene tick (dispatched by 0x7642(r5=0) from the
// scroll-position loop at 0x2cd8), producing genuine frame-by-frame animation.
//
// The DMD is natively 64x16, one byte per dot (low nibble = brightness 0..15)
// on a 2-byte stride ([value][pad]). It is a genuine 16-level display, not
// on/off: verified by tapping the live dot values -- the boot self-test writes
// a full 0..15 gradient, and gameplay/attract scenes each use their own subset
// of shades (a 1/15 dim-glow-plus-bright logo, an even 0/3/7/11/15 ramp, etc.).
// Only a few levels are lit in any one scene (so it reads as ~2-5), but the
// value used spans the whole nibble across scenes. Each dot is published raw as
// an output (dmddot<n>, row-major n = y*64 + x) carrying its 0..15 brightness;
// the .lay maps that to a 16-state amber/red element.
void flashbeats_state::update_dmd()
{
	auto const disp8 = util::big_endian_cast<uint8_t const>(m_dispram.target());

	for (int y = 0; y < DMD_H; y++)
		for (int x = 0; x < DMD_W; x++)
			m_dmd[y * DMD_W + x] = disp8[(DMD_BASE + y * DMD_PITCH + x * 2) & 0xffff] & 0x0f;
}

// Polls display RAM on a fixed-rate timer. Both the DMD and the lanes are pure
// artwork outputs (no raster screen); screen updates aren't a reliable
// game-state clock, so both are driven from this timer instead.
TIMER_DEVICE_CALLBACK_MEMBER(flashbeats_state::lane_update_timer)
{
	update_dmd();
	update_lanes();
}

// TE7752 P3 lamp outputs
// Player 1 lane5 isn't on this port at all; see port4_w below.
void flashbeats_state::te7752_port3_w(uint8_t data)
{
	m_btn_lamp[0] = BIT(data, 4);   // P1 lane1 (top)
	m_btn_lamp[1] = BIT(data, 5);   // P1 lane2
	m_btn_lamp[2] = BIT(data, 6);   // P1 lane3 (middle)
	m_btn_lamp[3] = BIT(data, 7);   // P1 lane4
}

// TE7752 P4 lamp outputs
// (out_port4, P2 side): bits 1-5 = lanes 1-5 directly
void flashbeats_state::te7752_port4_w(uint8_t data)
{
	m_btn_lamp[4]     = BIT(data, 0);  // P1 lane5 (bottom)
	m_btn_lamp[5 + 0] = BIT(data, 1);  // P2 lane1 (top)
	m_btn_lamp[5 + 1] = BIT(data, 2);  // P2 lane2
	m_btn_lamp[5 + 2] = BIT(data, 3);  // P2 lane3 (middle)
	m_btn_lamp[5 + 3] = BIT(data, 4);  // P2 lane4
	m_btn_lamp[5 + 4] = BIT(data, 5);  // P2 lane5 (bottom)
	m_lamp_up         = BIT(data, 6);  // Select1 (up)
	m_lamp_dn         = BIT(data, 7);  // Select2 (down)
}

// TE7752 P5 lamp outputs
void flashbeats_state::te7752_port5_w(uint8_t data)
{
	m_lamp_flash = BIT(data, 0);   // 'flash' lighting
}

// TE7752 P6 lamp outputs
void flashbeats_state::te7752_port6_w(uint8_t data)
{
	m_lamp_start = BIT(data, 0);   // Start
}

uint8_t flashbeats_state::p6_r()
{
	return (m_eeprom->do_read() << 3);
}

void flashbeats_state::p6_w(uint8_t data)
{
	m_eeprom->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write((data >> 2) & 1);
	m_eeprom->cs_write((data & 0x01) ? ASSERT_LINE : CLEAR_LINE);
}

// Analog spectrum-display source-select latch: the ADC sampler
// RMWs the low 3 bits (sequence 2,1,3,7,4), so reads must return the latch.
uint8_t flashbeats_state::spectrum_mux_r()
{
	return m_spectrum_mux;
}

void flashbeats_state::spectrum_mux_w(uint8_t data)
{
	m_spectrum_mux = data;
}

void flashbeats_state::flashbeats_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);
	map(0x200000, 0x20ffff).ram();
	map(0x400000, 0x40007f).rw(m_315_5296, FUNC(sega_315_5296_device::read), FUNC(sega_315_5296_device::write)).umask16(0xff00);
	map(0x600000, 0x60001f).rw("telio", FUNC(te7752_device::read), FUNC(te7752_device::write)).umask16(0xff00);
	map(0x800000, 0x80001f).rw(m_315_5338a, FUNC(sega_315_5338a_device::read), FUNC(sega_315_5338a_device::write)).umask16(0xff00);
	map(0xa00000, 0xa0ffff).ram().share("dispram");   // display work RAM; lanes built at 0xa0c000
	map(0xa10000, 0xa10fff).ram();
	map(0xa18000, 0xa18001).rw(FUNC(flashbeats_state::spectrum_mux_r), FUNC(flashbeats_state::spectrum_mux_w)).umask16(0xff00);
}

void flashbeats_state::main_scsp_map(address_map &map)
{
	map(0x000000, 0x0fffff).ram().share("sound_ram");
	map(0x100000, 0x100fff).rw("scsp", FUNC(scsp_device::read), FUNC(scsp_device::write));
	// The sound 68000 writes a status/handshake nibble here (values 0x08/0x0c/
	// 0x0e/0x0f/0x0d... from PC 0x6001b0/0x6005e0) very early in init; it was
	// hitting unmapped space and faulting. Back it with RAM so init proceeds.
	map(0x400000, 0x400001).ram();
	map(0x600000, 0x67ffff).rom().region("scspcpu", 0);
	// Sample ROM (rom3) exposed to the sound-68000, Model 2 style (see model2_snd:
	// 0x800000-0x9fffff = "samples"). rom3 lives at offset 0x80000 in the "scspcpu"
	// region (after rom4's 0x80000 of code). The firmware stages PCM from here into
	// sound_ram (the SCSP wave/DSP DRAM) itself; the SCSP then plays it from RAM, so
	// the DSP keeps its writable work area (mapping rom3 straight into scsp_mem as
	// ROM droned because the DSP read/writes that same space).
	map(0x800000, 0x9fffff).rom().region("scspcpu", 0x80000);
}

void flashbeats_state::scsp_mem(address_map &map)
{
	// SCSP wave/DSP address space
	map(0x000000, 0x0fffff).ram().share("sound_ram");
}

void flashbeats_state::flashbeats(machine_config &config)
{
	/* basic machine hardware */
	H83007(config, m_maincpu, 16_MHz_XTAL); // 16 MHz oscillator next to chip, also 16 MHz causes SCI0 and 1 rates to be 31250 (MIDI)
	m_maincpu->set_addrmap(AS_PROGRAM, &flashbeats_state::flashbeats_map);
	m_maincpu->read_port6().set(FUNC(flashbeats_state::p6_r));
	m_maincpu->write_port6().set(FUNC(flashbeats_state::p6_w));
	// SCI1 TX -> SCSP MIDI-in (sound effects). The upstream SCSP is a
	// device_serial_interface: midi_in(int state) feeds one raw line bit into its
	// own async receiver (31250 bps, 8N1 -- matches the H8 SCI1 rate above), so we
	// wire the H8's per-bit TX line straight in, exactly like model2/3 route their
	// i8251 TXD to scsp_device::midi_in. No byte reassembly needed. SCI0 TX -> DSB2
	// is wired below, next to the DSB2 device.
	m_maincpu->write_sci_tx<1>().set(m_scsp, FUNC(scsp_device::midi_in));
	// On-chip A/D converter inputs: the cabinet feeds its own audio levels
	// back in through the 0xa18000 mux.
	m_maincpu->read_adc<0>().set(FUNC(flashbeats_state::adc0_r));
	m_maincpu->read_adc<1>().set(FUNC(flashbeats_state::adc1_r));

	M68000(config, m_scspcpu, 11289600);
	m_scspcpu->set_addrmap(AS_PROGRAM, &flashbeats_state::main_scsp_map);

	EEPROM_93C46_16BIT(config, "eeprom");

	SEGA_315_5296(config, m_315_5296, 8_MHz_XTAL);
	m_315_5296->in_pa_callback().set_ioport("IN_A");
	m_315_5296->in_pb_callback().set_ioport("IN_B");
	m_315_5296->in_pd_callback().set_ioport("IN_D");
	// DIP banks live on ports G/H, read once at boot (PC 0x622/0x650): the H8
	// reads 0x40000c/0x40000e, inverts (active-low), then splits DSW1 low nibble
	// into the coin-setting table and DSW2 into start/continue credit counts +
	// bit7 advertise sound.
	m_315_5296->in_pg_callback().set_ioport("DSW1");
	m_315_5296->in_ph_callback().set_ioport("DSW2");

	SEGA_315_5338A(config, m_315_5338a, 32_MHz_XTAL);

	// Neither the DMD nor the LED lanes is a real raster device on the hardware:
	// the 64x16 DMD and the 5 LED lanes are all built by the H8 in work RAM and
	// published as named artwork outputs (update_dmd / update_lanes), drawn by
	// the .lay. There is no MAME screen -- both are polled by the timer below.

	// Artwork output state is not screen-clocked; poll it on its own timer so
	// frameskip/throttle can't affect its update rate. Rate chosen from live
	// measurement (write-tap directly on 0xa0c000/0xa0c1e0, counting actual byte
	// VALUE changes, not just writes -- the composer rewrites the whole buffer
	// every cycle regardless of content change, ~595 writes/frame, so raw write
	// count isn't useful): baseline content-change rate tracks the RTCOR system
	// tick (~248Hz, matches h8_refresh.h's ~4.03ms tick almost exactly), with
	// bursts up to ~433Hz measured during busier attract-mode animation. 480Hz
	// stays above the observed burst ceiling with margin.
	TIMER(config, "lane_timer").configure_periodic(FUNC(flashbeats_state::lane_update_timer), attotime::from_hz(480));

	config.set_default_layout(layout_flsbeats);

	te7752_device &te7752(TE7752(config, "telio"));
	te7752.ios_cb().set_constant(1);
	//te7752.out_port2_cb().set(FUNC(flashbeats_state::te7752_port2_w));
	te7752.out_port3_cb().set(FUNC(flashbeats_state::te7752_port3_w));   // P1 lane lamps (see impl comment)
	te7752.out_port4_cb().set(FUNC(flashbeats_state::te7752_port4_w));   // P2 lane lamps + up/dn arrows (see impl comment)
	te7752.out_port5_cb().set(FUNC(flashbeats_state::te7752_port5_w));   // flash light (see impl comment)
	te7752.out_port6_cb().set(FUNC(flashbeats_state::te7752_port6_w));   // start lamp (see impl comment)
	// out_port7 deliberately left unhooked: confirmed continuous clock/strobe
	// toggle unrelated to lamps.

	SPEAKER(config, "speaker", 2).front();

	SCSP(config, m_scsp, 22579200); // TODO : Unknown clock, divider
	m_scsp->set_addrmap(0, &flashbeats_state::scsp_mem);
	m_scsp->irq_cb().set(FUNC(flashbeats_state::scsp_irq));
	// The game programs its SCSP SFX voices around -21..-33 dB full-scale
	// (TL=0x18 with IMXL=4/5 on the DSP path, DISDL=3/4 direct) and relies on the
	// cabinet's analog mix stage to bring them up level with the DSB2 MPEG music
	// (real cabinets have the two balanced; the music never enters the SCSP
	// digitally -- the game leaves EFSDL=0 on slots 16/17, so an EXTS hookup would
	// mute it). Only the 16:1 SCSP:DSB2 ratio is ground truth; the absolute
	// level is calibrated against another Sega arcade machine, daytona (MAME
	// 0.280): 180s attract captures give daytona -15.3 dBFS active RMS vs
	// -16.1 dBFS here at these gains.
	m_scsp->add_route(0, "speaker", 16.0, 0);
	m_scsp->add_route(1, "speaker", 16.0, 1);

	DSB2(config, m_dsb2, 0);
	m_dsb2->add_route(0, "speaker", 1.0, 0);
	m_dsb2->add_route(1, "speaker", 1.0, 1);

	// Audio feedback into the H8 ADC (drives the lane spectrum-analyzer show):
	// the filter board taps the line-level signal ahead of the power amp, so
	// these gains are 1/16 the speaker routes (same 16:1 balance); the board's
	// output-stage gain is adc_r's calibrated 4.22.
	FLASHBEATS_SPECTRUM(config, m_spectrum);
	m_scsp->add_route(0, "spectrum", 1.0, 0);
	m_scsp->add_route(1, "spectrum", 1.0, 1);
	m_dsb2->add_route(0, "spectrum", 0.0625, 0);
	m_dsb2->add_route(1, "spectrum", 0.0625, 1);

	// SCI0 TX -> DSB2 (MPEG music) serial in. DSB2::write_txd is bit-level (it
	// feeds an internal i8251 clocked by DSB2's own uart_clock), so the H8's
	// SCI0 TX line connects straight through -- no byte reassembly needed.
	// The DSB2's serial reply (rxd) returns to the H8's SCI0 RX pin.
	m_maincpu->write_sci_tx<0>().set(m_dsb2, FUNC(dsb2_device::write_txd));
	m_dsb2->rxd_handler().set(m_maincpu, FUNC(h83007_device::sci_rx_w<0>));
}

void flashbeats_state::scsp_irq(offs_t offset, uint8_t data)
{
	m_scspcpu->set_input_line(offset, data);
}

// Input mappings
// 3 bits seem genuinely dead (scanner never reads them) and left PORT_BIT(..., IPT_UNUSED).
static INPUT_PORTS_START( flashbeats )
	// 315-5296 port A -- @0x400000.		
	PORT_START("IN_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2 Attack 1") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 Attack 2") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Attack 3") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Attack 4") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Attack 5") PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )  // dead lines (scanner never reads)

	// 315-5296 port B -- @0x400002.
	// bit 2 is the cabinet's TEST/diagnostics button (opens the ">Exit" test
	// menu from attract, and doubles as select/confirm once inside it). Bit 3
	// is the coin-door SERVICE button (credits up, and advances the test-menu
	// cursor exactly what a Sega service switch does; it was mislabeled COIN1
	// at first). Bits 0-1 are the real coin chutes 1/2. Bit 6 = start. Bits 4-5
	// are Select1 (up) and Select2 (down); bit 7 is unknown (unconfirmed, not
	// dead -- scanner does read it).
	PORT_START("IN_B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW ) PORT_NAME("Test")  // TEST button (menu open/select)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service")  // SERVICE button (credit + menu advance)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Select 1 (Up)") PORT_PLAYER(1) 
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Select 2 (Down)") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unconfirmed, not a dead line

	// 315-5296 port D -- @0x400006.
	// Bits 0-4 quiet in attract - P2's 5 lane buttons (mirrors port A's P1 layout).
	// Bit 7 is unknown (unconfirmed, not dead). Bits 5-6 are dead lines (scanner
	// never reads them).
	PORT_START("IN_D")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Attack 1") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Attack 2") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Attack 3") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Attack 4") PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 Attack 5") PORT_PLAYER(1)
	PORT_BIT( 0x60, IP_ACTIVE_LOW, IPT_UNUSED )  // dead lines (scanner never reads)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unconfirmed, not a dead line

	// DIP SW1 on 315-5296 port G (read @boot, PC 0x622, active-low: ON = 0).
	// Only the low nibble (sw1-4) is consumed; setting #N reads back as
	// (~N & 0x0f).
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x05, DEF_STR( 6C_1C ) )     // #A
	PORT_DIPSETTING(    0x06, DEF_STR( 5C_1C ) )     // #9
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )     // #8
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )     // #6
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )     // #3
	PORT_DIPSETTING(    0x08, "5 Coins/2 Credits" )  // #7
	PORT_DIPSETTING(    0x0a, DEF_STR( 5C_3C ) )     // #5
	PORT_DIPSETTING(    0x0b, DEF_STR( 3C_2C ) )     // #4
	PORT_DIPSETTING(    0x0d, "5 Coins/6 Credits" )  // #2
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )     // #0
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_3C ) )     // #1
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )     // #B
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )     // #C
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_4C ) )     // #D
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )     // #E
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) ) // #F
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )

	// DIP SW2 on 315-5296 port H (read @boot, PC 0x650-0x69e, active-low). The
	// H8 inverts, masks each field and adds 1 -> credit counts 1-4. Bit 7 is
	// cabinet-documented "ADVERTISE SOUND" (Japanese label: 内部/外部, "internal/
	// external"). This is a sound-source selector, not a demo-sounds mute -
	// confirmed by a full 24-bit input sweep during attract mode that found no
	// input bit (DIP or otherwise) silences advertise sound.
	// Traced the boot read itself (PC 0x69C-0x6A8): bit 7 does gate a conditional
	// write, but its target (0xfff438) sits above the H8/3007's on-chip RAM
	// ceiling (0xfff1f per the Hitachi H8/300H family memory map) and below the
	// internal I/O register block (0xfff20+) -- i.e. unmapped space in the
	// current emulation, so the write is a no-op here and has no observable
	// effect either way. Left as the literal cabinet-documented labels rather
	// than DEF_STR( Demo_Sounds ) pending further hardware RE.
	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Credits To Start (1P)" )  PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x0c, 0x0c, "Credits To Start (VS)" )  PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x30, 0x30, "Credits To Continue" )    PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x80, "Advertise Sound" )        PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "Internal" )
	PORT_DIPSETTING(    0x00, "External" )
INPUT_PORTS_END

/***************************************************************************

  ROM definition(s)

***************************************************************************/

ROM_START( flsbeats )
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "epr-21609_rom1.ic18", 0x000000, 0x080000, CRC(130a0a62) SHA1(400f24304959547b188ed874653ae2e1e77092fe) )

	ROM_REGION(0x280000, "scspcpu", 0)
	ROM_LOAD16_WORD_SWAP( "epr-21610_rom4.ic14", 0x000000, 0x080000, CRC(c877e0e6) SHA1(595f143fb3789852a4af9d2920cbaefabecfa45c) )
	ROM_LOAD16_WORD_SWAP( "epr-21611_rom3.ic4", 0x080000, 0x200000, CRC(2f5dc574) SHA1(f0b8d076b0fc8e94582de0ca17ecd5c8b90bedc4) )

	ROM_REGION(0x20000, "dsb2:mpegcpu", 0)
	ROM_LOAD16_WORD_SWAP( "epr-21612.ic2", 0x000000, 0x020000, CRC(6912e1cb) SHA1(3497d6ae0b9be00116a3278f46d738c4c6f26d20) )

	ROM_REGION(0x2000000, "dsb2:mpeg", 0)
	ROM_LOAD( "mpr-21601_n26_9852k7016.ic18", 0x0000000, 0x400000, CRC(d23e2b7b) SHA1(8c26a740fee0adc4d45d34786b0c28abb105324d) )
	ROM_LOAD( "mpr-21602_n27_9852k7017.ic19", 0x0400000, 0x400000, CRC(e143960b) SHA1(7ace5cae6f2a8868d74d4397a9c1b0a0f6f26c9f) )
	ROM_LOAD( "mpr-21603_n28_9852k7018.ic20", 0x0800000, 0x400000, CRC(136b69d8) SHA1(ad81be6f0383f29306c8d9f21d1e7440172ebf97) )
	ROM_LOAD( "mpr-21604_n29_9852k7019.ic21", 0x0c00000, 0x400000, CRC(87b15ea4) SHA1(eacc0f575926e68a195090370cb9e9e06b38e404) )
	ROM_LOAD( "mpr-21605_n30_9852k7023.ic22", 0x1000000, 0x400000, CRC(fb1a802e) SHA1(6fa99694973fd3cdd1a0f74a655583aadf5adc8f) )
	ROM_LOAD( "mpr-21606_n31_9852k7020.ic23", 0x1400000, 0x400000, CRC(d9aba5e0) SHA1(6f3f1b01174d771a56f92aeead998827da97ac22) )
	ROM_LOAD( "mpr-21607_n32_9852k7021.ic24", 0x1800000, 0x400000, CRC(f3dd07c6) SHA1(aa9d056e8ff5d2282917a09c42711062b8df989a) )
	ROM_LOAD( "mpr-21608_n33_9852k7022.ic25", 0x1c00000, 0x400000, CRC(be4db836) SHA1(93d4cbb3bb299e3cf1dda105670e3923751c28ad) )
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT   MACHINE       INPUT       CLASS              INIT     MONITOR   COMPANY  FULLNAME      FLAGS
GAME( 1998, flsbeats, 0,    flashbeats,   flashbeats, flashbeats_state,  empty_init, ROT0,    "Sega", "Flash Beats", 0 )

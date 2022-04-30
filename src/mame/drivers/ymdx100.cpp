// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Yamaha DX100 & DX27 FM synthesizers.

    The main differences between DX100 and DX27 are that DX27 has 61 keys
    rather than 49, but DX100 is portable and can run on battery power. Their
    tone generation capabilities are identical.

*******************************************************************************/

#include "emu.h"

#include "bus/midi/midi.h"
#include "cpu/m6800/m6801.h"
#include "machine/adc0808.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "sound/ymopm.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class yamaha_dx100_state : public driver_device
{
public:
	yamaha_dx100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_midi_in(true)
	{
	}

	void dx100(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	HD44780_PIXEL_UPDATE(lcd_pixel_update);
	void palette_init(palette_device &palette);

	DECLARE_WRITE_LINE_MEMBER(p22_w);

	void mem_map(address_map &map);

	required_device<hd6303x_cpu_device> m_maincpu;

	bool m_midi_in;
};

void yamaha_dx100_state::machine_start()
{
	save_item(NAME(m_midi_in));
}

HD44780_PIXEL_UPDATE(yamaha_dx100_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 16)
		bitmap.pix(line * 10 + y + 1 + ((y == 7) ? 1 : 0), pos * 6 + x + 1) = state ? 1 : 2;
}

void yamaha_dx100_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(0x00, 0x00, 0x00)); // background
	palette.set_pen_color(1, rgb_t(0xff, 0xff, 0xff)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(0x18, 0x18, 0x18)); // lcd pixel off
}

WRITE_LINE_MEMBER(yamaha_dx100_state::p22_w)
{
	if (state)
		m_maincpu->m6801_clock_serial();
}

void yamaha_dx100_state::mem_map(address_map &map)
{
	map(0x0000, 0x001f).m(m_maincpu, FUNC(hd6303x_cpu_device::hd6301x_io));
	map(0x0040, 0x00ff).ram(); // internal RAM
	map(0x0800, 0x0fff).ram().share("nvram");
	map(0x1000, 0x17ff).ram();
	map(0x2000, 0x2001).rw("lcdc", FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	map(0x2800, 0x2800).r("adc", FUNC(m58990_device::data_r));
	map(0x3000, 0x3000).w("adc", FUNC(m58990_device::address_data_start_w));
	map(0x3800, 0x3801).rw("ymsnd", FUNC(ym2164_device::read), FUNC(ym2164_device::write));
	map(0x8000, 0xffff).rom().region("program", 0);
}

static INPUT_PORTS_START(dx100)
INPUT_PORTS_END

void yamaha_dx100_state::dx100(machine_config &config)
{
	HD6303X(config, m_maincpu, 7.15909_MHz_XTAL / 2); // HD6303XP
	m_maincpu->set_addrmap(AS_PROGRAM, &yamaha_dx100_state::mem_map);
	m_maincpu->in_p2_cb().set([this]() -> u8 { return m_midi_in ? 0x04 : 0; });
	m_maincpu->in_p6_cb().set("adc", FUNC(m58990_device::eoc_r)).lshift(4);
	m_maincpu->out_ser_tx_cb().set("mdout", FUNC(midi_port_device::write_txd));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC5518BPL + CR2032T battery

	m58990_device &adc(M58990(config, "adc", 7.15909_MHz_XTAL / 8)); // M58990P-1 (clocked by E)
	adc.in_callback<5>().set_constant(0x80); // "CNG RAM BATTERY!" displayed unless value is between 0x70 and 0xCC

	CLOCK(config, "subclock", 500_kHz_XTAL).signal_handler().set(FUNC(yamaha_dx100_state::p22_w));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set([this](int state) { m_midi_in = state; });
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*16+1, 10*2+1);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(yamaha_dx100_state::palette_init), 3);

	hd44780_device &lcdc(HD44780(config, "lcdc", 0)); // HD44780RA00
	lcdc.set_lcd_size(2, 16);
	lcdc.set_pixel_update_cb(FUNC(yamaha_dx100_state::lcd_pixel_update));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2164_device &ymsnd(YM2164(config, "ymsnd", 7.15909_MHz_XTAL / 2)); // with YM3014 DAC
	ymsnd.add_route(0, "lspeaker", 0.60);
	ymsnd.add_route(1, "rspeaker", 0.60);
}

ROM_START(dx100)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("dx100 v1.1.bin", 0x0000, 0x8000, CRC(c3ed7c86) SHA1(5b003db1bb5c1909907153f6446b63b07f5b41d6))
ROM_END

} // anonymous namespace

SYST(1985, dx100, 0, 0, dx100, dx100, yamaha_dx100_state, empty_init, "Yamaha", "DX100 Digital Programmable Algorithm Synthesizer", MACHINE_IS_SKELETON)

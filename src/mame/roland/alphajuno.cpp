// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland αJuno/SynthPlus 10 synthesizers.

****************************************************************************/

#include "emu.h"
//#include "bus/midi/midi.h"
#include "cpu/mcs51/mcs51.h"
#include "mb62h195.h"
#include "mb63h149.h"
#include "machine/nvram.h"
#include "machine/rescap.h"
#include "machine/upd7001.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"


namespace {

class alphajuno_state : public driver_device
{
public:
	alphajuno_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "lcdc")
	{
	}

	void ajuno1(machine_config &config);
	void ajuno2(machine_config &config);
	void mks50(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	void lcd_w(offs_t offset, u8 data);

	void prog_map(address_map &map) ATTR_COLD;
	void ajuno1_ext_map(address_map &map) ATTR_COLD;
	void ajuno2_ext_map(address_map &map) ATTR_COLD;
	void mks50_ext_map(address_map &map) ATTR_COLD;

	void palette_init(palette_device &palette);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
};

void alphajuno_state::machine_reset()
{
}

HD44780_PIXEL_UPDATE(alphajuno_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 8)
		bitmap.pix(y, (line * 8 + pos) * 6 + x) = state;
}


void alphajuno_state::lcd_w(offs_t offset, u8 data)
{
	if (offset == 0)
		m_lcdc->control_w(data);
	else
		m_lcdc->data_w(data);
}

void alphajuno_state::prog_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("program", 0);
}

void alphajuno_state::ajuno1_ext_map(address_map &map)
{
	map(0x2000, 0x27ff).mirror(0xd800).ram().share("nvram");
	map(0x8000, 0x8008).w(FUNC(alphajuno_state::lcd_w));
}

void alphajuno_state::ajuno2_ext_map(address_map &map)
{
	map(0x2000, 0x27ff).mirror(0xd800).ram().share("nvram");
	map(0x5000, 0x57ff).mirror(0x800).rw("keyscan", FUNC(mb63h149_device::read), FUNC(mb63h149_device::write));
	map(0x8000, 0x8008).w(FUNC(alphajuno_state::lcd_w));
}

void alphajuno_state::mks50_ext_map(address_map &map)
{
	map(0x8000, 0x8008).w(FUNC(alphajuno_state::lcd_w));
	map(0xe000, 0xffff).ram().share("nvram");
}

static INPUT_PORTS_START(ajuno1)
INPUT_PORTS_END

static INPUT_PORTS_START(ajuno2)
INPUT_PORTS_END

static INPUT_PORTS_START(mks50)
INPUT_PORTS_END

void alphajuno_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(131, 136, 139));
	palette.set_pen_color(1, rgb_t( 92,  83,  88));
}

void alphajuno_state::ajuno1(machine_config &config)
{
	I8032(config, m_maincpu, 12_MHz_XTAL); // P8032AH
	m_maincpu->set_addrmap(AS_PROGRAM, &alphajuno_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &alphajuno_state::ajuno1_ext_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC5517APL + battery

	mb62h195_device &io(MB62H195(config, "io"));
	io.lc_callback().set(m_lcdc, FUNC(hd44780_device::write));
	io.sout_callback().set("adc", FUNC(upd7001_device::si_w));
	io.sck_callback().set("adc", FUNC(upd7001_device::sck_w));
	io.sin_callback().set("adc", FUNC(upd7001_device::so_r));
	io.adc_callback().set("adc", FUNC(upd7001_device::cs_w));

	//MB87123(config, "dco", 12_MHz_XTAL);

	UPD7001(config, "adc", RES_K(27), CAP_P(47));

	// LCD: LM16155A or LM16155B
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*16, 8*1);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(alphajuno_state::palette_init), 2);

	HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 8);
	m_lcdc->set_pixel_update_cb(FUNC(alphajuno_state::lcd_pixel_update));
	m_lcdc->set_busy_factor(0.005f);
}

void alphajuno_state::ajuno2(machine_config &config)
{
	ajuno1(config);
	m_maincpu->set_addrmap(AS_IO, &alphajuno_state::ajuno2_ext_map);

	mb63h149_device &keyscan(MB63H149(config, "keyscan", 12_MHz_XTAL));
	keyscan.int_callback().set_inputline(m_maincpu, MCS51_INT0_LINE);
}

void alphajuno_state::mks50(machine_config &config)
{
	I80C31(config, m_maincpu, 12_MHz_XTAL); // MSM80C31P
	m_maincpu->set_addrmap(AS_PROGRAM, &alphajuno_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &alphajuno_state::mks50_ext_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC5564PL-20 + battery

	mb62h195_device &io(MB62H195(config, "io"));
	io.lc_callback().set(m_lcdc, FUNC(hd44780_device::write));

	//MB87123(config, "dco", 12_MHz_XTAL);

	// LCD Unit: DM011Z-1DL3
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*16, 8*1);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(alphajuno_state::palette_init), 2);

	HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 8);
	m_lcdc->set_pixel_update_cb(FUNC(alphajuno_state::lcd_pixel_update));
	m_lcdc->set_busy_factor(0.05f);
}

// Original EPROM labels specify major and minor revisions with punch grids; "U" (update?) tag is separate.
// Version strings may be inconsistent with EPROM labels.
ROM_START(ajuno1)
	ROM_REGION(0x4000, "program", 0)
	//ROM_SYSTEM_BIOS(0, "v26", "Version 2.6")
	ROM_LOAD("u__ju-1_2_6.ic10", 0x0000, 0x4000, CRC(9797fd5b) SHA1(0d2e24f8c5f646279985a34ac8bf7c0b9354d32b)) // M5L27128K-2
ROM_END

ROM_START(ajuno2)
	ROM_REGION(0x4000, "program", 0)
	ROM_SYSTEM_BIOS(0, "v25", "Version 2.5")
	ROMX_LOAD("ju-2_2_5__u.ic24", 0x0000, 0x4000, CRC(13b9e68e) SHA1(28a8207a5cd63ababd61d7a46df102ea7116a898), ROM_BIOS(0)) // NEC D27128D-2
	ROM_SYSTEM_BIOS(1, "v25oled", "Version 2.5 (OLED Display Mod)") // http://wp.visuanetics.nl/oled-display-for-alpha-juno-2/
	ROMX_LOAD("ju2-2_5-modified-for-oled-final.bin", 0x0000, 0x4000, CRC(1bca5bc6) SHA1(22e9c71af4b5f3e185f767740e61e5332c0a979f), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v24", "Version 2.4")
	ROMX_LOAD("ju-2_2_4.ic24", 0x0000, 0x4000, CRC(bfedda17) SHA1(27eee472befdbc7d7ed0caaf359775d8ff3c836a), ROM_BIOS(2)) // M5M27C128
ROM_END

ROM_START(hs80)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("roland hs-80 v5.0 eprom -dom- - 27128", 0x0000, 0x4000, CRC(94e85807) SHA1(128eec37fb6dcac6ec73d3ca544c28dbf7dbf9b2))
ROM_END

ROM_START(mks50)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("mks-50_v1.02.ic7", 0x0000, 0x4000, CRC(a342f90e) SHA1(8eed986051abfdf55167c179dc7c7f0822a3ba0c))
ROM_END

} // anonymous namespace


SYST(1985, ajuno1, 0, 0, ajuno1, ajuno1, alphajuno_state, empty_init, "Roland", "Alpha Juno-1 (JU-1) Programmable Polyphonic Synthesizer", MACHINE_IS_SKELETON)
//SYST(1985, hs10, ajuno1, 0, ajuno1, ajuno1, alphajuno_state, empty_init, "Roland", "SynthPlus 10 (HS-10) Programmable Polyphonic Synthesizer", MACHINE_IS_SKELETON)
SYST(1986, ajuno2, 0, 0, ajuno2, ajuno2, alphajuno_state, empty_init, "Roland", "Alpha Juno-2 (JU-2) Programmable Polyphonic Synthesizer", MACHINE_IS_SKELETON)
SYST(1986, hs80, ajuno2, 0, ajuno2, ajuno2, alphajuno_state, empty_init, "Roland", "HS-80 Programmable Polyphonic Synthesizer", MACHINE_IS_SKELETON)
SYST(1987, mks50, 0, 0, mks50, mks50, alphajuno_state, empty_init, "Roland", "MKS-50 Synthesizer Module", MACHINE_IS_SKELETON)

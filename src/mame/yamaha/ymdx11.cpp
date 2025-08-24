// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Yamaha DX11 synthesizer.

****************************************************************************/

#include "emu.h"
//#include "bus/midi/midi.h"
#include "cpu/m6800/m6801.h"
#include "machine/adc0808.h"
#include "machine/nvram.h"
#include "sound/ymopz.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class yamaha_dx11_state : public driver_device
{
public:
	yamaha_dx11_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_adc(*this, "adc")
		, m_rombank(*this, "rombank")
	{
	}

	void dx11(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	HD44780_PIXEL_UPDATE(lcd_pixel_update);
	void palette_init(palette_device &palette);

	void cartridge_bank_w(u8 data);

	void main_map(address_map &map) ATTR_COLD;

	required_device<hd6301y_cpu_device> m_maincpu;
	required_device<hd6301y_cpu_device> m_subcpu;
	required_device<adc0808_device> m_adc;
	required_memory_bank m_rombank;
};

void yamaha_dx11_state::machine_start()
{
	m_rombank->configure_entries(0, 2, memregion("firmware")->base(), 0x8000);
}


HD44780_PIXEL_UPDATE(yamaha_dx11_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 16)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

void yamaha_dx11_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(131, 136, 139));
	palette.set_pen_color(1, rgb_t( 92,  83,  88));
}


void yamaha_dx11_state::cartridge_bank_w(u8 data)
{
}

void yamaha_dx11_state::main_map(address_map &map)
{
	map(0x1400, 0x1400).mirror(0x3ff).w(FUNC(yamaha_dx11_state::cartridge_bank_w));
	map(0x1800, 0x1801).mirror(0x3fe).rw("opz", FUNC(ym2414_device::read), FUNC(ym2414_device::write));
	map(0x1c00, 0x1c01).mirror(0x3fe).rw("lcdc", FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	map(0x2000, 0x3fff).ram().share("ram1");
	map(0x4000, 0x5fff).ram().share("ram2");
	//map(0x6000, 0x7fff).rw(FUNC(yamaha_dx11_state::cartridge_r), FUNC(yamaha_dx11_state::cartridge_w));
	map(0x8000, 0xffff).bankr("rombank");
}


static INPUT_PORTS_START(dx11)
INPUT_PORTS_END

void yamaha_dx11_state::dx11(machine_config &config)
{
	HD6303Y(config, m_maincpu, 8_MHz_XTAL); // HD63B03YP-N
	m_maincpu->set_addrmap(AS_PROGRAM, &yamaha_dx11_state::main_map);
	m_maincpu->out_p2_cb().set_membank("rombank").bit(6);

	HD6301Y0(config, m_subcpu, 8_MHz_XTAL).set_disable(); // HD63B01Y0D60P

	NVRAM(config, "ram1", nvram_device::DEFAULT_ALL_0); // TC5564APL-15 + lithium battery
	NVRAM(config, "ram2", nvram_device::DEFAULT_ALL_0); // TC5564APL-15 + lithium battery

	M58990(config, m_adc, 8_MHz_XTAL / 16); // M58990P-1; divider not verified (actually clocked by P26 output of sub CPU)

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_color(rgb_t::green());
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*16, 8*2);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(yamaha_dx11_state::palette_init), 2);

	hd44780_device &lcdc(HD44780(config, "lcdc", 270'000)); // TODO: clock not measured, datasheet typical clock used
	lcdc.set_lcd_size(2, 16);
	lcdc.set_pixel_update_cb(FUNC(yamaha_dx11_state::lcd_pixel_update));

	SPEAKER(config, "speaker", 2).front();

	ym2414_device &opz(YM2414(config, "opz", 3.579545_MHz_XTAL));
	//opz.irq_handler().set_inputline(m_maincpu, hd6301y_cpu_device::IRQ2_LINE); // IRQ = P51
	opz.add_route(0, "speaker", 0.60, 0);
	opz.add_route(1, "speaker", 0.60, 1);
}

ROM_START(dx11)
	ROM_REGION(0x10000, "firmware", 0)
	ROM_LOAD("dx11_xd820_bo.ic15", 0x00000, 0x10000, CRC(d5355df7) SHA1(a5376c32fcce9e93a22b71ec476b14ff3091153f)) // 27C512

	ROM_REGION(0x4000, "subcpu", 0)
	ROM_LOAD("hd63b01y0d60p.ic18", 0x0000, 0x4000, NO_DUMP)
ROM_END

} // anonymous namespace


SYST(1988, dx11, 0, 0, dx11, dx11, yamaha_dx11_state, empty_init, "Yamaha", "DX11 Digital Programmable Algorithm Synthesizer", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

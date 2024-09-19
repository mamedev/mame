// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Quasimidi Quasar & TechnoX synthesizer modules.

***************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/6850acia.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

namespace {

class qmquasar_state : public driver_device
{
public:
	qmquasar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void quasar(machine_config &config);
	void technox(machine_config &config);

protected:
	virtual void driver_start() override;

private:
	HD44780_PIXEL_UPDATE(quasar_pixel_update);
	HD44780_PIXEL_UPDATE(technox_pixel_update);

	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
};

HD44780_PIXEL_UPDATE(qmquasar_state::quasar_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 40)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

HD44780_PIXEL_UPDATE(qmquasar_state::technox_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 16)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}


void qmquasar_state::prog_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("program", 0); // TODO: banking?
}

void qmquasar_state::ext_map(address_map &map)
{
	map(0x0000, 0x38ff).ram();
	map(0x3900, 0x3903).nopr();
	map(0x390b, 0x390b).nopr();
	map(0x7c00, 0x7fff).noprw();
	map(0x8000, 0x8003).noprw();
	map(0xa000, 0xa001).w("acia", FUNC(acia6850_device::write));
	map(0xa002, 0xa003).r("acia", FUNC(acia6850_device::read));
	map(0xb000, 0xb001).w("lcdc", FUNC(hd44780_device::write));
	map(0xb002, 0xb003).r("lcdc", FUNC(hd44780_device::read));
	map(0xc000, 0xc0ff).nopw();
	map(0xff10, 0xff10).noprw();
}


static INPUT_PORTS_START(qmquasar)
INPUT_PORTS_END

void qmquasar_state::quasar(machine_config &config)
{
	I8032(config, m_maincpu, 12'000'000); // exact type and clock unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &qmquasar_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &qmquasar_state::ext_map);

	acia6850_device &acia(ACIA6850(config, "acia"));
	acia.irq_handler().set_inputline(m_maincpu, MCS51_INT1_LINE);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*40, 8*2);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	hd44780_device &lcdc(HD44780(config, "lcdc", 270'000)); // TODO: clock not measured, datasheet typical clock used
	lcdc.set_lcd_size(2, 40);
	lcdc.set_pixel_update_cb(FUNC(qmquasar_state::quasar_pixel_update));
}

void qmquasar_state::technox(machine_config &config)
{
	quasar(config);

	subdevice<screen_device>("screen")->set_size(6*16, 8*2);
	subdevice<screen_device>("screen")->set_visarea_full();

	subdevice<hd44780_device>("lcdc")->set_lcd_size(2, 16);
	subdevice<hd44780_device>("lcdc")->set_pixel_update_cb(FUNC(qmquasar_state::technox_pixel_update));
}


ROM_START(qmquasar)
	ROM_REGION(0x20000, "program", 0)
	ROM_LOAD("27c1000_v200.ic4", 0x00000, 0x20000, CRC(951363bd) SHA1(6dfb2fa362afd8da0fb764108b6263d9da45c676)) // "Version No: 2.00"

	ROM_REGION(0x40000, "samples", 0)
	ROM_LOAD("27c020_v200d.ic51", 0x00000, 0x40000, CRC(ef318407) SHA1(e147413d5df2f9489c2886587ae60f6807b7dbb9))
ROM_END

ROM_START(technox)
	ROM_REGION(0x20000, "program", 0)
	ROM_LOAD("mx27c1000_v105.ic4", 0x00000, 0x20000, CRC(4d393cd9) SHA1(9ee853f1f2e88fbfc86589c4d0f91003d0ff146f)) // "Version No 1.05b"

	ROM_REGION(0x20000, "samples", 0)
	ROM_LOAD("nm27c010_v105.ic51", 0x00000, 0x20000, CRC(d669570e) SHA1(0645a1b022ed25a836152933e6df32da4448f49a))
ROM_END

void qmquasar_state::driver_start()
{
	memory_region *rgn = memregion("program");
	u8 *rom = rgn->base();

	for (offs_t base = 0x00000; base < rgn->bytes(); base += 0x100)
	{
		// Copy 256-byte block before descrambling addresses
		std::vector<uint8_t> orig(&rom[base], &rom[base + 0x100]);

		for (offs_t offset = 0; offset < 0x100; offset++)
			rom[base + offset] = orig[bitswap<8>(offset, 1, 2, 3, 6, 5, 7, 1, 0) ^ 0x40 ^ ((offset & 0x11) << 3)];
	}
}

} // anonymous namespace

SYST(1993, qmquasar, 0, 0, quasar,  qmquasar, qmquasar_state, empty_init, "Quasimidi Musikelektronik GmbH", "Quasimidi Quasar", MACHINE_IS_SKELETON)
SYST(1995, technox,  0, 0, technox, qmquasar, qmquasar_state, empty_init, "Quasimidi Musikelektronik GmbH", "TechnoX",          MACHINE_IS_SKELETON)

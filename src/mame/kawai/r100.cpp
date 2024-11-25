// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Kawai R-100 drum machine.

***************************************************************************/

#include "emu.h"
#include "mb63h158.h"
#include "cpu/m6502/m50734.h"
#include "machine/nvram.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"


namespace {

class kawai_r100_state : public driver_device
{
public:
	kawai_r100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "lcdc")
	{
	}

	void r100(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	HD44780_PIXEL_UPDATE(pixel_update);

	void p0_w(u8 data);
	void p1_w(u8 data);
	void p2_w(u8 data);
	void p3_w(u8 data);
	void buffer_w(u8 data);

	void main_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	required_device<m50734_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
};


void kawai_r100_state::machine_start()
{
	m_lcdc->rw_w(0); // write only
}

HD44780_PIXEL_UPDATE(kawai_r100_state::pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 16)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

void kawai_r100_state::p0_w(u8 data)
{
	m_lcdc->rs_w(BIT(data, 3));
	m_lcdc->e_w(BIT(data, 4));
}

void kawai_r100_state::p1_w(u8 data)
{
}

void kawai_r100_state::p2_w(u8 data)
{
}

void kawai_r100_state::p3_w(u8 data)
{
}

void kawai_r100_state::buffer_w(u8 data)
{
}

void kawai_r100_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram1");
	map(0x2000, 0x3fff).ram().share("nvram2");
	map(0x4100, 0x41ff).r("sensor", FUNC(mb63h158_device::read));
	map(0x4200, 0x4200).mirror(0xff).w(m_lcdc, FUNC(hd44780_device::db_w));
	map(0x4300, 0x4300).mirror(0xff).w(FUNC(kawai_r100_state::buffer_w));
	map(0x4400, 0xffff).rom().region("program", 0x4400);
}

void kawai_r100_state::data_map(address_map &map)
{
	// TODO: 0x0000-0x1fff and 0x2000-0x3fff mapped to cartridge slot
}


static INPUT_PORTS_START(r100)
INPUT_PORTS_END

void kawai_r100_state::r100(machine_config &config)
{
	M50734(config, m_maincpu, 16_MHz_XTAL / 2); // M50734SP
	m_maincpu->set_addrmap(AS_PROGRAM, &kawai_r100_state::main_map);
	m_maincpu->set_addrmap(AS_DATA, &kawai_r100_state::data_map);
	m_maincpu->set_p0_3state(0x20);
	m_maincpu->set_p3_3state(0x02);
	m_maincpu->p0_out_cb().set(FUNC(kawai_r100_state::p0_w));
	m_maincpu->p1_out_cb().set(FUNC(kawai_r100_state::p1_w));
	m_maincpu->p2_out_cb().set(FUNC(kawai_r100_state::p2_w));
	m_maincpu->p3_out_cb().set(FUNC(kawai_r100_state::p3_w));
	m_maincpu->p4_in_cb().set_constant(0x0f);

	NVRAM(config, "nvram1", nvram_device::DEFAULT_ALL_0); // MB8464-15LL-SK + battery
	NVRAM(config, "nvram2", nvram_device::DEFAULT_ALL_0); // MB8464-15LL-SK + battery

	MB63H158(config, "sensor", 16_MHz_XTAL / 4);

	//M60009_AGU_DGU(config, "pcm", 5_MHz_XTAL);

	// LCD unit
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*16, 8*2);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 16);
	m_lcdc->set_pixel_update_cb(FUNC(kawai_r100_state::pixel_update));
}

ROM_START(r100)
	ROM_REGION(0x10000, "program", 0)
	ROM_SYSTEM_BIOS(0, "c", "Revision C")
	ROMX_LOAD("kawai_6p13c.u18", 0x00000, 0x08000, CRC(3d7ed644) SHA1(d47d6c866d19390d781287ae7a69962c4503ce8e), ROM_BIOS(0))
	ROMX_LOAD("kawai_6p14c.u16", 0x08000, 0x08000, CRC(61757eca) SHA1(24331ed8d5842d65ad1b2a472fb708e8c5a140c7), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "b", "Revision B")
	ROMX_LOAD("kawai_6p13b.u18", 0x00000, 0x08000, CRC(53d288a0) SHA1(25d5bcaf4b5146d13c9d75313d3110776dd4e237), ROM_BIOS(1)) // D27C256AD-20
	ROMX_LOAD("kawai_6p14b.u16", 0x08000, 0x08000, CRC(777889bb) SHA1(30fb93afcc2e8738bcc6f991081f269953e49b15), ROM_BIOS(1)) // D27C256AD-20

	ROM_REGION(0x80000, "pcm", 0)
	ROM_LOAD("kawai_mn234001kaa.u20", 0x00000, 0x80000, CRC(aaf1805e) SHA1(5894b4cb03e17a5aa8c2b0c9b1b3d9285009a1c3))
ROM_END

} // anonymous namespace


SYST(1987, r100, 0, 0, r100, r100, kawai_r100_state, empty_init, "Kawai Musical Instrument Manufacturing", "R-100 Digital Drum Machine", MACHINE_IS_SKELETON)

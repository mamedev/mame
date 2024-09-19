// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Sony DPS-V55/V55M effects processor.

    V55 and V55M are identical except for the power supply (120 V, 60 Hz for
    the former, 230 V, 50/60 Hz for the latter).

****************************************************************************/

#include "emu.h"
#include "cpu/f2mc16/mb9061x.h"
#include "machine/nvram.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

namespace {

class dpsv55_state : public driver_device
{
public:
	dpsv55_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "lcdc")
		, m_p5(0)
	{
	}

	void dpsv55(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	HD44780_PIXEL_UPDATE(pixel_update);
	u8 p5_r();
	void p5_w(u8 data);
	u8 sci_ssr0_r();

	void mem_map(address_map &map) ATTR_COLD;

	required_device<mb90641_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;

	u8 m_p5;
};

void dpsv55_state::machine_start()
{
	save_item(NAME(m_p5));
}

HD44780_PIXEL_UPDATE(dpsv55_state::pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos >= 4 && pos < 20)
		bitmap.pix(line * 8 + y, (pos - 4) * 6 + x) = state;
}

u8 dpsv55_state::p5_r()
{
	return m_p5;
}

void dpsv55_state::p5_w(u8 data)
{
	m_lcdc->e_w(BIT(data, 3));
	m_lcdc->rw_w(BIT(data, 2));
	m_lcdc->rs_w(BIT(data, 1));

	m_p5 = data;
}

u8 dpsv55_state::sci_ssr0_r()
{
	return 0x18;
}


void dpsv55_state::mem_map(address_map &map)
{
	map(0x000011, 0x000011).nopw();
	map(0x000022, 0x000022).noprw();
	map(0x000023, 0x000023).r(FUNC(dpsv55_state::sci_ssr0_r));
	map(0xfc0000, 0xfc7fff).mirror(0x18000).ram().share("nvram"); // CS1
	map(0xfe0000, 0xffffff).rom().region("eprom", 0); // CS0
}


static INPUT_PORTS_START(dpsv55)
INPUT_PORTS_END

void dpsv55_state::dpsv55(machine_config &config)
{
	MB90641A(config, m_maincpu, 4_MHz_XTAL); // MB90641APF-G-105BND
	m_maincpu->set_addrmap(AS_PROGRAM, &dpsv55_state::mem_map);
	m_maincpu->read_pdr1().set(m_lcdc, FUNC(hd44780_device::db_r));
	m_maincpu->write_pdr1().set(m_lcdc, FUNC(hd44780_device::db_w));
	m_maincpu->read_pdr5().set(FUNC(dpsv55_state::p5_r));
	m_maincpu->write_pdr5().set(FUNC(dpsv55_state::p5_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // CY62256LL-70SNC-T2 + 3V lithium battery + M62021FP-600C reset generator + M5239L voltage detector

	// LCD unit (LC0801)
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*16, 8*2);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 20);
	m_lcdc->set_pixel_update_cb(FUNC(dpsv55_state::pixel_update));

	//CXD2707(config, "dsp", 49.152_MHz_XTAL);
}

/*
Sony DPS-V55

Version 1.02

Chip: MX 27C1000DC-90

Sticker:
~~~~~~~~~~~~
 DPS-V55/M
 Ver.1.02
 759-499074
~~~~~~~~~~~~

Multi Processor  DPS-V55
Sony Corporation (c)1998
*/

ROM_START(dpsv55)
	ROM_REGION(0x20000, "eprom", 0)
	ROM_LOAD("dps-v55_m__ver.1.02__759-499-74.ic704", 0x00000, 0x20000, CRC(138c2fe0) SHA1(0916ccb1d7567639b382a19240a56274c5c2fa4a))
ROM_END

} // anonymous namespace

SYST(1998, dpsv55, 0, 0, dpsv55, dpsv55, dpsv55_state, empty_init, "Sony", "DPS-V55 Multi-Effect Processor", MACHINE_IS_SKELETON)

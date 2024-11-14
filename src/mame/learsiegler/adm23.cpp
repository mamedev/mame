// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for ADM 23 terminal.

***************************************************************************/

#include "emu.h"
#include "cpu/z8/z8.h"
#include "video/mc6845.h"
#include "screen.h"


namespace {

class adm23_state : public driver_device
{
public:
	adm23_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_chargen(*this, "chargen")
	{
	}

	void adm23(machine_config &config);

private:
	MC6845_UPDATE_ROW(update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(addr_changed);

	void mem_map(address_map &map) ATTR_COLD;

	required_region_ptr<u8> m_chargen;
};

MC6845_UPDATE_ROW(adm23_state::update_row)
{
}

MC6845_ON_UPDATE_ADDR_CHANGED(adm23_state::addr_changed)
{
}

void adm23_state::mem_map(address_map &map)
{
	map(0x0800, 0x1fff).rom().region("program", 0x0800);
	map(0x2020, 0x2020).nopr();
	map(0x2080, 0x2080).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x2081, 0x2081).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xc080, 0xc080).nopw();
	map(0xd080, 0xd080).nopr();
}


static INPUT_PORTS_START(adm23)
INPUT_PORTS_END


void adm23_state::adm23(machine_config &config)
{
	z8_device &maincpu(Z8682(config, "maincpu", 14.7428_MHz_XTAL / 2));
	maincpu.set_addrmap(AS_PROGRAM, &adm23_state::mem_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14.7428_MHz_XTAL, 873, 0, 720, 281, 0, 264);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	mc6845_device &crtc(SY6545_1(config, "crtc", 14.7428_MHz_XTAL / 9));
	crtc.set_char_width(9);
	crtc.set_show_border_area(false);
	crtc.set_screen("screen");
	crtc.set_update_row_callback(FUNC(adm23_state::update_row));
	crtc.set_on_update_addr_change_callback(FUNC(adm23_state::addr_changed));
}

ROM_START(adm23)
	ROM_REGION(0x2000, "program", 0) // CPU is clearly a Z8682, though its location is labeled "Z8 8681"
	ROM_LOAD("136261-083_u9.bin", 0x0000, 0x2000, CRC(85da07e7) SHA1(a1305d0f06c7e3c6075b05ca0f11c53a901b5013))

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("chr_u10.bin", 0x0000, 0x0800, CRC(cd053232) SHA1(6b4136a91d0dcd9cb5df92c54c9c30b1cb5f1974))

	ROM_REGION(0x0020, "proms", 0)
	ROM_LOAD("129253-61_u3.bin", 0x0000, 0x0020, NO_DUMP) // N82S123N at location labeled "S288"
ROM_END

} // anonymous namespace


COMP(1982, adm23, 0, 0, adm23, adm23, adm23_state, empty_init, "Lear Siegler", "ADM 23 Smart Terminal", MACHINE_IS_SKELETON)

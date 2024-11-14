// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    Skeleton driver for Decision Data IS-48x/LM-48xC IBM-compatible coax workstation displays.

***********************************************************************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "cpu/bcp/dp8344.h"
#include "machine/eeprompar.h"
#include "video/mc6845.h"
#include "screen.h"


namespace {

class is48x_state : public driver_device
{
public:
	is48x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bcp(*this, "bcp")
		, m_crtc(*this, "crtc")
	{ }

	void is482(machine_config &config);

private:
	MC6845_UPDATE_ROW(update_row);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void bcp_inst_map(address_map &map) ATTR_COLD;
	void bcp_data_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<dp8344_device> m_bcp;
	required_device<mc6845_device> m_crtc;
};

MC6845_UPDATE_ROW(is48x_state::update_row)
{
}

void is48x_state::mem_map(address_map &map)
{
	map(0x00000, 0x07fff).ram(); // W24257S-70LL
	map(0x40000, 0x47fff).rw(m_bcp, FUNC(dp8344_device::remote_read), FUNC(dp8344_device::remote_write));
	map(0x50000, 0x51fff).ram(); // CY7C185-35VC
	map(0x54000, 0x55fff).ram(); // CY7C185-35VC
	map(0x60000, 0x67fff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write));
	map(0x80000, 0xfffff).rom().region("program", 0);
}

void is48x_state::io_map(address_map &map)
{
	map(0x8005, 0x8005).nopw();
	map(0x8080, 0x8080).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x8081, 0x8081).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x8101, 0x8101).nopr();
	map(0x8180, 0x8180).rw(m_bcp, FUNC(dp8344_device::cmd_r), FUNC(dp8344_device::cmd_w));
}

void is48x_state::bcp_inst_map(address_map &map)
{
	map(0x0000, 0x1fff).ram(); // CY7C185-35VC x2
}

void is48x_state::bcp_data_map(address_map &map)
{
	map(0x0000, 0x7fff).ram(); // W24257S-70LL
	map(0xc000, 0xffff).ram();
}

static INPUT_PORTS_START(is482)
INPUT_PORTS_END

void is48x_state::is482(machine_config &config)
{
	I80188(config, m_maincpu, 16_MHz_XTAL); // N80C188-16
	m_maincpu->set_addrmap(AS_PROGRAM, &is48x_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &is48x_state::io_map);

	DP8344B(config, m_bcp, 18.867_MHz_XTAL); // DP8344BV
	m_bcp->set_auto_start(false);
	m_bcp->set_addrmap(AS_PROGRAM, &is48x_state::bcp_inst_map);
	m_bcp->set_addrmap(AS_DATA, &is48x_state::bcp_data_map);

	EEPROM_28256(config, "eeprom"); // AT28C256

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(60_MHz_XTAL / 2, 770, 0, 560, 532, 0, 475); // FIXME: vertical rate is supposed to be 75 Hz
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	HD6845S(config, m_crtc, 60_MHz_XTAL / 28); // HD46505SP-1
	m_crtc->set_char_width(14); // guess
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_update_row_callback(FUNC(is48x_state::update_row));
}

ROM_START(is482) // "IS-488-A" on case
	ROM_REGION(0x80000, "program", 0)
	ROM_LOAD("is-482_u67_s008533243.bin", 0x00000, 0x80000, CRC(1e23ac17) SHA1(aadc73bc0454c5b1c33d440dc511009dc6b7f9e0)) // M27C4001-10FI
ROM_END

} // anonymous namespace


COMP(199?, is482, 0, 0, is482, is482, is48x_state, empty_init, "Decision Data", "IS-482 Workstation", MACHINE_IS_SKELETON)

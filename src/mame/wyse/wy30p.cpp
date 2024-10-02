// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Wyse WY-30+ terminal.

    This low-end video terminal is the successor to the WY-30, whose control
    board features an entirely different hardware configuration.

*******************************************************************************/

#include "emu.h"
#include "bus/wysekbd/wysekbd.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/eepromser.h"
#include "screen.h"


namespace {

class wy30p_state : public driver_device
{
public:
	wy30p_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_eeprom(*this, "eeprom")
		, m_keyboard(*this, "keyboard")
		, m_screen(*this, "screen")
	{
	}

	void wy30p(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void driver_start() override;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 p1_r();
	u8 p3_r();
	void keyboard_clock_w(u8 data);
	void keyboard_reset_w(u8 data);
	u8 de00_r();

	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	required_device<eeprom_serial_er5911_device> m_eeprom;
	required_device<wyse_keyboard_port_device> m_keyboard;
	required_device<screen_device> m_screen;
};


void wy30p_state::machine_start()
{
}

u32 wy30p_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

u8 wy30p_state::p1_r()
{
	return 0x7f | (!m_keyboard->data_r() << 7);
}

u8 wy30p_state::p3_r()
{
	return 0xdf | (m_eeprom->do_read() << 5);
}

void wy30p_state::keyboard_clock_w(u8 data)
{
	m_keyboard->cmd_w(0);
	m_keyboard->cmd_w(1);
}

void wy30p_state::keyboard_reset_w(u8 data)
{
	m_keyboard->cmd_w(0);
}

u8 wy30p_state::de00_r()
{
	return m_screen->vblank() << 7; // probably not correct
}

void wy30p_state::prog_map(address_map &map)
{
	map(0x0000, 0x3fff).mirror(0xc000).rom().region("program", 0);
}

void wy30p_state::ext_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0xa000, 0xa7ff).ram();
	map(0xc300, 0xc3ff).nopw(); // ?
	map(0xc700, 0xc7ff).nopw(); // ?
	map(0xc900, 0xc9ff).nopw(); // ?
	map(0xdc00, 0xdc00).mirror(0xff).w(FUNC(wy30p_state::keyboard_clock_w));
	map(0xdd00, 0xdd00).mirror(0xff).w(FUNC(wy30p_state::keyboard_reset_w));
	map(0xde00, 0xde00).mirror(0xff).r(FUNC(wy30p_state::de00_r));
}


static INPUT_PORTS_START(wy30p)
INPUT_PORTS_END


void wy30p_state::wy30p(machine_config &config)
{
	i8031_device &maincpu(I8031(config, "maincpu", 7.3728_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &wy30p_state::prog_map);
	maincpu.set_addrmap(AS_IO, &wy30p_state::ext_map);
	maincpu.port_in_cb<1>().set(FUNC(wy30p_state::p1_r));
	maincpu.port_in_cb<3>().set(FUNC(wy30p_state::p3_r));
	maincpu.port_out_cb<3>().set(m_eeprom, FUNC(eeprom_serial_er5911_device::cs_write)).bit(3);
	maincpu.port_out_cb<3>().append(m_eeprom, FUNC(eeprom_serial_er5911_device::clk_write)).bit(4);
	maincpu.port_out_cb<3>().append(m_eeprom, FUNC(eeprom_serial_er5911_device::di_write)).bit(5);

	EEPROM_ER5911_8BIT(config, m_eeprom);

	WYSE_KEYBOARD(config, m_keyboard, wy30_keyboards, "wy30");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(31.2795_MHz_XTAL * 2 / 3, 1050, 0, 800, 331, 0, 312); // divider and dimensions guessed
	m_screen->set_screen_update(FUNC(wy30p_state::screen_update));
	m_screen->screen_vblank().set_inputline("maincpu", MCS51_INT0_LINE);
}


// PCB: "© 1989 990710-01 REV.B1"
// CPU: Intel P8031AH
// Program EPROM: 250971-02 (M27128MFI) "© WYSE TECH 92' REV.A"
// Nonvolatile memory: CSI CAT59C11P
// RAM: TC5565APL-12, HY6116AP-15
// QFP gate array: 211019-02 (Oki M76V020) "© WYSE,1989"
// Jumper near gate array: "FONT SIZE" = "8Kx8" or "2Kx8" (J3)
// XTALs: 31.2795 (dot clock?), 7.3728 (for CPU)
// DB-25 connectors: "MDM" & "AUX"
ROM_START(wy30p)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("250971-02.u4", 0x0000, 0x4000, CRC(3666549c) SHA1(23c432da2083df4b355daf566dd6514d1f9a7690))
ROM_END

void wy30p_state::driver_start()
{
	uint8_t *rom = memregion("program")->base();
	for (offs_t base = 0x0000; base < 0x4000; base += 0x2000)
	{
		std::vector<uint8_t> orig(&rom[base], &rom[base + 0x2000]);

		// Line swaps can be confusing...
		for (offs_t offset = 0; offset < 0x2000; offset++)
			rom[base | offset] = bitswap<8>(orig[bitswap<13>(offset, 8, 5, 0, 4, 3, 9, 7, 10, 11, 12, 2, 6, 1)], 3, 4, 5, 6, 7, 2, 1, 0);
	}
}

} // anonymous namespace


COMP(1992, wy30p, 0, 0, wy30p, wy30p, wy30p_state, empty_init, "Wyse Technology", "WY-30+ (v1.8)", MACHINE_IS_SKELETON)

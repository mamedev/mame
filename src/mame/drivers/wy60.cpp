// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    Skeleton driver for Wyse WY-60 terminal.

***********************************************************************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
//#include "machine/i2cmem.h"
#include "machine/mc2661.h"
#include "video/scn2674.h"
#include "screen.h"

class wy60_state : public driver_device
{
public:
	wy60_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_pvtc(*this, "pvtc")
		, m_sio(*this, "sio")
	{
	}

	void wy60(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void driver_start() override;

private:
	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);

	u8 pvtc_r(offs_t offset);
	void pvtc_w(offs_t offset, u8 data);
	u8 sio_r(offs_t offset);
	void sio_w(offs_t offset, u8 data);

	void prog_map(address_map &map);
	void ext_map(address_map &map);
	void row_buffer_map(address_map &map);

	required_device<scn2672_device> m_pvtc;
	required_device<mc2661_device> m_sio;
};


void wy60_state::machine_start()
{
}

SCN2672_DRAW_CHARACTER_MEMBER(wy60_state::draw_character)
{
}

u8 wy60_state::pvtc_r(offs_t offset)
{
	return m_pvtc->read(offset >> 8);
}

void wy60_state::pvtc_w(offs_t offset, u8 data)
{
	m_pvtc->write(offset >> 8, data);
}

u8 wy60_state::sio_r(offs_t offset)
{
	return m_sio->read(offset >> 8);
}

void wy60_state::sio_w(offs_t offset, u8 data)
{
	m_sio->write(offset >> 8, data);
}

void wy60_state::prog_map(address_map &map)
{
	// TODO: EA banking?
	map(0x0000, 0xffff).rom().region("coderom", 0);
}

void wy60_state::ext_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("charram");
	map(0x2000, 0x3fff).ram().share("attrram");
	map(0xa000, 0xa7ff).mirror(0x1800).rw(FUNC(wy60_state::pvtc_r), FUNC(wy60_state::pvtc_w));
	map(0xc000, 0xc3ff).mirror(0xc00).r(FUNC(wy60_state::sio_r));
	map(0xd000, 0xd3ff).mirror(0xc00).w(FUNC(wy60_state::sio_w));
}

void wy60_state::row_buffer_map(address_map &map)
{
	map.global_mask(0x0ff);
	map(0x000, 0x0ff).ram();
}


static INPUT_PORTS_START(wy60)
INPUT_PORTS_END

void wy60_state::wy60(machine_config &config)
{
	i8051_device &maincpu(I8051(config, "maincpu", 11_MHz_XTAL)); // P8051AN-40196
	maincpu.set_addrmap(AS_PROGRAM, &wy60_state::prog_map);
	maincpu.set_addrmap(AS_IO, &wy60_state::ext_map);

	//X2404(config, m_eeprom);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(26.58_MHz_XTAL, 1000, 0, 800, 443, 0, 416); // 26.580 kHz horizontal
	//screen.set_raw(39.71_MHz_XTAL, 1494, 0, 1188, 443, 0, 416);
	screen.set_screen_update("pvtc", FUNC(scn2672_device::screen_update));

	SCN2672(config, m_pvtc, 26.58_MHz_XTAL / 10); // custom-marked as Motorola SC67336P (205001-02)
	m_pvtc->set_screen("screen");
	m_pvtc->set_character_width(10); // 9 in 132-column mode
	m_pvtc->set_addrmap(0, &wy60_state::row_buffer_map);
	m_pvtc->set_addrmap(1, &wy60_state::row_buffer_map);
	m_pvtc->set_display_callback(FUNC(wy60_state::draw_character));
	//m_pvtc->intr_callback().set_inputline("maincpu", MCS51_T0_LINE);
	//m_pvtc->breq_callback().set_inputline("maincpu", MCS51_INT0_LINE);

	MC2661(config, m_sio, 4.9152_MHz_XTAL); // SCN2661B
	//m_sio->rxrdy_handler().set_inputline("maincpu", MCS51_INT1_LINE);
}

// CPU:   8051(202008-03)
// EPROM: 27512(193003-01)
// Video: 211003-02/205001-02
// RAM:   2064 (2064/2016/2016/2064)
// NVRAM: X2404
// UART:  2661
// XTALs: 39.710, 26.580, 11.000, 4.9152

ROM_START(wy60)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("202008-03.bin", 0x0000, 0x1000, NO_DUMP)

	ROM_REGION(0x10000, "coderom", 0)
	ROM_LOAD("193003-01.u9", 0x00000, 0x10000, CRC(26de0ea4) SHA1(91409f98a3990b514fbcb7de2eb45944bf5b95bc))
ROM_END

ROM_START(wy60a)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("202008-03.bin", 0x0000, 0x1000, NO_DUMP)

	ROM_REGION(0x10000, "coderom", 0)
	ROM_LOAD("wy-60_4k.u6", 0x00000, 0x10000, CRC(6daf2824) SHA1(23cd039ec7ae71b0742e8eebf75be8cd5992e3fd))
ROM_END

void wy60_state::driver_start()
{
	uint8_t *rom = memregion("coderom")->base();
	for (offs_t base = 0x00000; base < 0x10000; base += 0x2000)
	{
		std::vector<uint8_t> orig(&rom[base], &rom[base + 0x2000]);

		for (offs_t offset = 0; offset < 0x2000; offset++)
			rom[base | offset] = bitswap<8>(orig[bitswap<13>(offset, 0, 6, 9, 4, 2, 1, 3, 5, 7, 8, 10, 11, 12)], 6, 0, 5, 1, 4, 2, 3, 7);
	}

	// FIXME: remove once internal ROM is dumped
	std::copy_n(rom, 0x1000, memregion("maincpu")->base());
}

COMP(1986, wy60,  0,    0, wy60, wy60, wy60_state, empty_init, "Wyse Technology", "WY-60 (set 1)", MACHINE_IS_SKELETON)
COMP(1986, wy60a, wy60, 0, wy60, wy60, wy60_state, empty_init, "Wyse Technology", "WY-60 (set 2)", MACHINE_IS_SKELETON)

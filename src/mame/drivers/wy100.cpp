// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Wyse WY-100 video terminal.

    The WY-100 was Wyse Technology's first product. An unusual feature of
    this terminal's keyboard is three banks of user-accessible DIP switches.

*******************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/bankdev.h"
#include "video/i8275.h"
#include "screen.h"

class wy100_state : public driver_device
{
public:
	wy100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bankdev(*this, "bankdev")
		, m_crtc(*this, "crtc%u", 1U)
		, m_chargen(*this, "chargen")
	{
	}

	void wy100(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(crtc_w);
	DECLARE_WRITE8_MEMBER(p2_w);

	void prg_map(address_map &map);
	void io_map(address_map &map);
	void bank_map(address_map &map);

	required_device<mcs48_cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_bankdev;
	required_device_array<i8276_device, 2> m_crtc;

	required_region_ptr<u8> m_chargen;
};

WRITE8_MEMBER(wy100_state::crtc_w)
{
	m_crtc[0]->write(space, offset >> 8, data);
	m_crtc[1]->write(space, offset >> 8, data);
}

WRITE8_MEMBER(wy100_state::p2_w)
{
	m_bankdev->set_bank(data & 0x1f);
}

void wy100_state::prg_map(address_map &map)
{
	map(0x000, 0xfff).rom().region("maincpu", 0);
}

void wy100_state::io_map(address_map &map)
{
	map(0x00, 0xff).m(m_bankdev, FUNC(address_map_bank_device::amap8));
}

void wy100_state::bank_map(address_map &map)
{
	map(0x0000, 0x01ff).w(FUNC(wy100_state::crtc_w));
	map(0x0c00, 0x1fff).ram();
}


static INPUT_PORTS_START(wy100)
INPUT_PORTS_END


void wy100_state::wy100(machine_config &config)
{
	I8039(config, m_maincpu, 10.1376_MHz_XTAL); // INS8039N-11
	m_maincpu->set_addrmap(AS_PROGRAM, &wy100_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &wy100_state::io_map);
	m_maincpu->p2_out_cb().set(FUNC(wy100_state::p2_w));

	ADDRESS_MAP_BANK(config, m_bankdev);
	m_bankdev->set_addrmap(0, &wy100_state::bank_map);
	m_bankdev->set_data_width(8);
	m_bankdev->set_addr_width(13);
	m_bankdev->set_stride(0x100);

	//SCN2651(config, "pci", 10.1376_MHz_XTAL / 2); // INS2651N

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(18.48_MHz_XTAL, 1000, 0, 800, 308, 0, 286);
	screen.set_screen_update("crtc1", FUNC(i8276_device::screen_update));

	I8276(config, m_crtc[0], 18.48_MHz_XTAL / 10).set_character_width(10);
	I8276(config, m_crtc[1], 18.48_MHz_XTAL / 10).set_character_width(10);
}


ROM_START(wy100)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("wy100_00401f.bin", 0x0000, 0x1000, CRC(1f71de8f) SHA1(2bd9f712aba8b44823ce0b3e111da7b472a1ab38))

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("wy100_23-002-01c.bin", 0x0000, 0x0800, CRC(93c31537) SHA1(085e5ad110a76bee83e819a718a7d4cbfb8e07e7))
ROM_END


COMP(1981, wy100, 0, 0, wy100, wy100, wy100_state, empty_init, "Wyse Technology", "WY-100", MACHINE_IS_SKELETON)

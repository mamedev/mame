// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    PEVM Byte

Refs:
    https://web.archive.org/web/20241003085933/https://zxbyte.ru/

TODO:
- Sound / KR580VI53 == i8253
- Keyboard Ext keys
- Byte-01?

**********************************************************************/

#include "emu.h"

#include "spectrum.h"

namespace {

class byte_state : public spectrum_state
{
public:
	byte_state(const machine_config &mconfig, device_type type, const char *tag)
        : spectrum_state(mconfig, type, tag)
        , m_io_comp(*this, "COMP")
    { }

	void byte(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void map_io(address_map &map) ATTR_COLD;

	virtual u8 spectrum_rom_r(offs_t offset) override;

private:
    required_ioport m_io_comp;

	bool m_1f_gate;
};

u8 byte_state::spectrum_rom_r(offs_t offset)
{
	if (m_1f_gate)
	{
		const u16 adr66 = ((offset >> 7) & 0xff) | ((m_io_comp->read() & 1) << 8);
		const u8 dat66 = memregion("dd66")->base()[adr66];
		if (~dat66 & 0x10)
		{
			u16 adr71 = ((dat66 & 0x0f) << 7) | (offset & 0x7f);
			return memregion("dd71")->base()[adr71];
		}
	}

	return spectrum_state::spectrum_rom_r(offset);
}

void byte_state::map_io(address_map &map)
{
	spectrum_state::spectrum_clone_io(map);
	map(0x0004, 0x0004).mirror(0xff8a).unmapw(); // #8e - ch0
	map(0x0024, 0x0024).mirror(0xff8a).unmapw(); // #ae - ch1
	map(0x0044, 0x0044).mirror(0xff8a).unmapw(); // #ce - ch2
	map(0x0064, 0x0064).mirror(0xff8a).unmapw(); // #ee - ctrl

	map(0x0015, 0x0015).mirror(0xff8a).lr8(NAME([this]() { m_1f_gate = true; return 0xff; })); // #1f
}

INPUT_PORTS_START(byte)
	PORT_INCLUDE( spectrum )

	PORT_START("COMP")
	PORT_CONFNAME( 0x01, 0x01, "Compatibility")
	PORT_CONFSETTING(    0x00, "On: Sinclair")
	PORT_CONFSETTING(    0x01, "Off: Byte")
INPUT_PORTS_END

void byte_state::machine_start()
{
    spectrum_state::machine_start();

	// Save
	save_item(NAME(m_1f_gate));
}

void byte_state::machine_reset()
{
    spectrum_state::machine_reset();

	m_1f_gate = false;
}


void byte_state::byte(machine_config &config)
{
    spectrum_state::spectrum_clone(config);

	m_maincpu->set_io_map(&byte_state::map_io);
	m_exp->fb_r_handler().set([]() { return 0xff; });
}

ROM_START(byte)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_DEFAULT_BIOS("prusak")

	ROM_SYSTEM_BIOS(0, "v1", "V1")
	ROMX_LOAD("byte.rom", 0x0000, 0x4000, CRC(c13ba473) SHA1(99f40727185abbb2413f218d69df021ae2e99e45), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "prusak", "Prusak's")
	ROMX_LOAD("dd72.bin", 0x0000, 0x2000, CRC(2464d537) SHA1(e8b4a468e6f254f090fc5b2c59ea573c2a4f0455), ROM_BIOS(1))
	ROMX_LOAD("dd73.bin", 0x2000, 0x2000, CRC(bd430288) SHA1(b4c67a6213b1ecfa37cc476bc483e8a8deef6149), ROM_BIOS(1))

    ROM_REGION(0x800, "dd71", ROMREGION_ERASEFF)
	ROM_LOAD("dd71_rt7.bin", 0x000, 0x800, CRC(c91b07c2) SHA1(2365d45b028b1e91dffb5bcdc87bd26ca9a7c26f))

    ROM_REGION(0x200, "dd66", ROMREGION_ERASEFF)
	ROM_LOAD("dd66_rt5.bin", 0x000, 0x200, CRC(f8f9766a) SHA1(3f5345763a30e5370199c454301de655e7f1a1da))

    ROM_REGION(0x600, "tbd", ROMREGION_ERASEFF)
	ROM_LOAD("dd10_rt5.reva.bin", 0x000, 0x200, CRC(aae13e3e) SHA1(46f0ca97ceee0c591277aaac8b0cecc445927690)) // SN 1..7599 - 1989..1990
	ROM_LOAD("dd10_rt5.revb.bin", 0x200, 0x200, CRC(b649b5d1) SHA1(2d067962b08aee8cdf1bc4f5ce337815dd9d6c66)) // SN 7600..  - 1991..1996
	ROM_LOAD("dd11_rt5.bin", 0x400, 0x200, CRC(0f32b304) SHA1(d7adf9861c332510ff3682a1b06e6d9898343b6d))
ROM_END

} // Anonymous namespace

//    YEAR  NAME      PARENT    COMPAT  MACHINE INPUT CLASS       INIT           COMPANY FULLNAME     FLAGS
COMP( 1990, byte,     spectrum, 0,      byte,   byte, byte_state, init_spectrum, "BEMZ", "PEVM Byte", 0 )
//COMP( 1993, byte01,   ...
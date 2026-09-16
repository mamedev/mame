// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
    Aquaplus P/ECE
*/
#include "emu.h"

#include "cpu/c33/s1c33209.h"


namespace {

class piece_state : public driver_device

{
public:
	piece_state(machine_config const &mconfig, device_type type, char const *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void piece512k(machine_config &config) ATTR_COLD;
	void piece2m(machine_config &config) ATTR_COLD;

private:
	void mem_base(address_map &map) ATTR_COLD;
	void mem_512k(address_map &map) ATTR_COLD;
	void mem_2m(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};


void piece_state::piece512k(machine_config &config)
{
	S1C33209(config, m_maincpu, 24'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &piece_state::mem_512k);
}

void piece_state::piece2m(machine_config &config)
{
	piece512k(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &piece_state::mem_2m);
}


void piece_state::mem_base(address_map &map)
{
	map(0x010'0000, 0x013'ffff).mirror(0x00c'0000).ram(); // SRAM
	// USB controller at 0x040'0000
}

void piece_state::mem_512k(address_map &map)
{
	mem_base(map);

	map(0x0c0'0000, 0x0c7'ffff).mirror(0x038'0000).rom().region("flash", 0); // load Flash as ROM just to show disassembly
}

void piece_state::mem_2m(address_map &map)
{
	mem_base(map);

	map(0x0c0'0000, 0x0df'ffff).mirror(0x020'0000).rom().region("flash", 0); // load Flash as ROM just to show disassembly
}


INPUT_PORTS_START(piece)
INPUT_PORTS_END


ROM_START(piece512k)
	ROM_DEFAULT_BIOS("v1.20")
	ROM_SYSTEM_BIOS(0, "v1.18", "PieceSystem Ver1.18")
	ROM_SYSTEM_BIOS(1, "v1.20", "PieceSystem Ver1.20")

	ROM_REGION(0x08'0000, "flash", ROMREGION_16BIT | ROMREGION_LE)
	ROMX_LOAD("ver1.18_512k.bin", 0x00'0000, 0x08'0000, CRC(c1298d20) SHA1(ca905d825f5c422b63b0cb622de779183e25bb9a), ROM_BIOS(0))
	ROMX_LOAD("ver1.20_512k.bin", 0x00'0000, 0x08'0000, CRC(0bb58345) SHA1(43b6bae56f89e3fe12aef9f82d55776fb7f16870), ROM_BIOS(1))
ROM_END

ROM_START(piece2m)
	ROM_DEFAULT_BIOS("v1.20")
	ROM_SYSTEM_BIOS(0, "v1.20", "PieceSystem Ver1.20")

	ROM_REGION(0x20'0000, "flash", ROMREGION_16BIT | ROMREGION_LE)
	ROMX_LOAD("ver1.20_2m.bin",   0x00'0000, 0x20'0000, CRC(9d83bb57) SHA1(efecdaba7837fe0fc215f5f1e9c00c050cc5e910), ROM_BIOS(0))
ROM_END

} // anonymous namespace


//   YEAR  NAME       PARENT     COMPAT  MACHINE    INPUT  CLASS        INIT        COMPANY     FULLNAME                FLAGS
SYST(2001, piece512k, 0,         0,      piece512k, piece, piece_state, empty_init, "Aquaplus", "P/ECE (512 kB Flash)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
SYST(2001, piece2m,   piece512k, 0,      piece2m,   piece, piece_state, empty_init, "Aquaplus", "P/ECE (2 MB Flash)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

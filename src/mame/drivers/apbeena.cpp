// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  apbeena.cpp

  Skeleton driver for the Sega Advanced Pico BEENA

  H/W is custom Sega SoC with ARM7TDMI core at 81 MHz.

  TODO:
            Everything!
            Needs the internal BIOS dumped.

***************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"

class beena_state : public driver_device
{
public:
	beena_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "arm7")
	{ }

	void beena(machine_config &config);

private:
	void machine_start() override;
	void machine_reset() override;

	void beena_arm7_map(address_map &map);

	required_device<arm7_cpu_device> m_maincpu;
};

void beena_state::beena_arm7_map(address_map &map)
{
	map(0x00000000, 0x000001ff).rom().region("cart", 0);
	map(0x80000000, 0x807fffff).rom().region("cart", 0);
}

static INPUT_PORTS_START( beena )
INPUT_PORTS_END

void beena_state::machine_reset()
{
}

void beena_state::machine_start()
{
}

void beena_state::beena(machine_config &config)
{
	ARM7_BE(config, m_maincpu, 81'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &beena_state::beena_arm7_map);
}

ROM_START( apb_frpc )
	ROM_REGION32_BE( 0x80000, "bios", 0 )   // SoC internal BIOS
	ROM_LOAD16_WORD_SWAP( "beenabios.bin", 0x000000, 0x080000, NO_DUMP )

	ROM_REGION32_BE( 0x800000, "cart", 0 )  // cartridge ROM
	ROM_LOAD16_WORD_SWAP( "freshprettycure.bin", 0x000000, 0x800000, CRC(afa3466c) SHA1(b49914f97db95f611489487f558d13d7cceb3299) )

	ROM_REGION( 0x2000000, "pages", 0 ) // book pages
	ROM_LOAD( "0 - 0010.png", 0x000000, 0x186546b, CRC(6d3d8c91) SHA1(9909322c09277ddd306f10f9a04a897b0fe74961) )
	ROM_LOAD( "0 - 0011.png", 0x000000, 0x179d3a8, CRC(c3911254) SHA1(2d2932e97a0b20a9b5e5fd78a3bbad0f1e146813) )
	ROM_LOAD( "0 - 0005.png", 0x000000, 0x18998f4, CRC(16837cc0) SHA1(eb86b2943cf603a63cd5b94faa0d06ee8a3139d6) )
	ROM_LOAD( "0 - 0007.png", 0x000000, 0x195df07, CRC(efccbb5b) SHA1(83de26d52cb6578d8323551a289223c8a305e37d) )
	ROM_LOAD( "0 - 0008.png", 0x000000, 0x18a9dd8, CRC(7455ebb2) SHA1(34576eee067534535cda838a9e94794e2cfee585) )
	ROM_LOAD( "0 - 0002.png", 0x000000, 0x1e37a3d, CRC(62ae3535) SHA1(d8f6c19c466715df5a568ac7cac8dacb53f2e04a) )
	ROM_LOAD( "0 - 0004.png", 0x000000, 0x19a864d, CRC(edede3ea) SHA1(81914894358c74064aa5f9e980ac37f7140f80c3) )
	ROM_LOAD( "0 - 0003.png", 0x000000, 0x1ce4f16, CRC(fe6c72d9) SHA1(2c4412eb518fbac2e8519c87e53270d960ecab53) )
	ROM_LOAD( "0 - 0009.png", 0x000000, 0x17a5c57, CRC(59ead843) SHA1(29c4cd7929cfd68f41d65024ee5ca3cc62fa4a8b) )
	ROM_LOAD( "0 - 0006.png", 0x000000, 0x195b7e5, CRC(3e307e6d) SHA1(23ed9ab2a4122d7cb33d8d6174ca901b88647b4d) )
	ROM_LOAD( "0 - 0012.png", 0x000000, 0x187a9df, CRC(f1cdc95d) SHA1(aa926c9360edcecdaa3a9ce1b5e17ee61048cee4) )
	ROM_LOAD( "0 - 0001.png", 0x000000, 0x1ca26ec, CRC(d8e18a54) SHA1(887a96931bf622cb673f2550323ad3923eff337e) )
ROM_END

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY     FULLNAME  FLAGS
CONS( 2009, apb_frpc,  0,      0,      beena,     beena,   beena_state, empty_init, "Sega", "Advanced Pico BEENA: Issho ni Henshin Fresh Pretty Cure", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

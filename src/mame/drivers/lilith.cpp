// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-10-27 Skeleton of Lilith, a Modula-2 single-user workstation.

CPU consists of various parts including AM2901 and AM2911.

************************************************************************************************************************************/

#include "emu.h"

class lilith_state : public driver_device
{
public:
	lilith_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		//      , m_maincpu(*this, "maincpu")
	{ }

	void lilith(machine_config &config);
private:
	//  required_device<cpu_device> m_maincpu;
};

static INPUT_PORTS_START( lilith )
INPUT_PORTS_END

void lilith_state::lilith(machine_config &config)
{
}

ROM_START( lilith )
	ROM_REGION( 0x10000, "maincpu", 0 )
	// main?
	ROM_LOAD( "150-0001-01a.bin", 0x000000, 0x000100, CRC(2991dbc6) SHA1(89a4f9f4f45495547062efcb035fb87906410ab9) )
	ROM_LOAD( "150-0001-02a.bin", 0x000000, 0x000100, CRC(2be08757) SHA1(19ad7fbea742953c10ba3da652e560547bc92032) )
	ROM_LOAD( "150-0001-03a.bin", 0x000000, 0x000100, CRC(c2103f8a) SHA1(b970f4b05d74b6d2a6477247b7f50acd024452f4) )
	ROM_LOAD( "150-0001-04a.bin", 0x000000, 0x000100, CRC(d5315408) SHA1(a52d2e535bff141e9976b6684b0aaf5d0089f6e6) )
	ROM_LOAD( "150-0002-01a.bin", 0x000000, 0x000800, CRC(2235b7ba) SHA1(bca0843bc61ba39f16f582eda9aa6c2188b7115a) )
	ROM_LOAD( "150-0002-02a.bin", 0x000000, 0x000800, CRC(c56b64cf) SHA1(42d6b2a394e0360905b30231eaa1b257602fe696) )
	ROM_LOAD( "150-0002-03a.bin", 0x000000, 0x000800, CRC(2c3ac942) SHA1(08c6e0f307e7eec3cfe9a55ed0a8c8eab4bd2931) )
	ROM_LOAD( "150-0002-04a.bin", 0x000000, 0x000800, CRC(f297e2e0) SHA1(f323b5fda7b4ce13ce5da5267b5375e257806410) )
	ROM_LOAD( "150-0002-05a.bin", 0x000000, 0x000800, CRC(009a46d3) SHA1(20050a0cc0084b10f6892161cb43dc6be286423b) )
	// fdc?
	ROM_LOAD( "fd6502u1.bin", 0x000000, 0x000800, CRC(24ab745a) SHA1(6229f7d08e5a9adbba045ce3db232c704d87ba0d) )
	ROM_LOAD( "fd6502u2.bin", 0x000000, 0x000800, CRC(a8cffbc3) SHA1(384d56cdc7d383cf9f160086827b9e0fb310cfdf) )
	// proms?
	ROM_LOAD( "hortim.bin",   0x000000, 0x000800, CRC(4f9a596a) SHA1(b2cec32f6b7bc23e6f83065cb20e0eae55ecc53a) )
	ROM_LOAD( "pd_u38.bin",   0x000000, 0x000100, CRC(724bf954) SHA1(5c8554d3a0f341516fe050fe09ed8e232af7e89e) )
	ROM_LOAD( "rx_u20.bin",   0x000000, 0x000100, CRC(658b523e) SHA1(070b5bf424e9f1fc3bc3dfc6126501c25f3b9ef3) )
	ROM_LOAD( "tx_u23.bin",   0x000000, 0x000100, CRC(4c3e2e9f) SHA1(9a88a016f3f74a15bcb635dd73d2129512ca53e8) )
	ROM_LOAD( "vertim.bin",   0x000000, 0x000800, CRC(304a8714) SHA1(dcd3542b8722144b80a638dda0f6df740a92c941) )
	ROM_LOAD( "xx_u40.bin",   0x000000, 0x000100, CRC(434e3456) SHA1(7679b3841b105345582e9bda758a134a2578cfbd) )
	// keyboard
	ROM_LOAD( "kbd_eprom.bin", 0x000000, 0x000800, CRC(40fa5230) SHA1(473ca714959fc35aa2a0ab9310aee2aedffa2163) )
ROM_END

COMP( 1984, lilith, 0, 0, lilith, lilith, lilith_state, empty_init, "DISER", "Lilith", MACHINE_IS_SKELETON )

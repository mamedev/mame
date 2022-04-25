// license:BSD-3-Clause
// copyright-holders:

/*
Dangerous Bar by Namco (1994)
https://www.youtube.com/watch?v=XwZoXtkZ9qo

Same Same Panic by Namco (1993)
https://www.youtube.com/watch?v=YDJW7ch45Yw

Hardware notes:
Main PCB named Hi-Pric P41 B 8813960102 (8813970102)
- MC68HC11K1 (main CPU)
- HD68B09P (audio CPU)
- CY7C132 DPRAM
- C140 (custom Namco audio chip)
- C121 (custom Namco glue logic for the 6809)
- OSC1 49.1520MHz
- 1 4-dip bank

For Dangerous Bar:
Led display PCB named Namco ST-M4
 CPU: Sharp LZ8415M
 Xtal: Marked "D122C5"
 Display controller: Seiko-Epson SED1351F-0A
*/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "sound/c140.h"
#include "speaker.h"


namespace {

class dangbar_state : public driver_device
{
public:
	dangbar_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void dangbar(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<mc68hc11_cpu_device> m_maincpu;

	void main_map(address_map &map);
	void audio_map(address_map &map);
};


void dangbar_state::machine_start()
{
}


void dangbar_state::main_map(address_map &map)
{
	map(0x8000, 0xffff).rom().region("maincpu", 0x8000);
}

void dangbar_state::audio_map(address_map &map) // TODO: audio section seems similar to namcos2.cpp / namcos21.cpp
{
	map(0xd000, 0xffff).rom().region("audiocpu", 0x01000);
}


static INPUT_PORTS_START( dangbar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW:4")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED ) // only 4 dips
INPUT_PORTS_END


void dangbar_state::dangbar(machine_config &config)
{
	// basic machine hardware
	MC68HC11K1(config, m_maincpu, 49.152_MHz_XTAL / 4); // divider guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &dangbar_state::main_map);

	mc6809_device &audiocpu(MC6809(config, "audiocpu", 49.152_MHz_XTAL / 24)); // HD68B09P, divider guessed from other Namco drivers
	audiocpu.set_addrmap(AS_PROGRAM, &dangbar_state::audio_map);

	// video hardware
	// TODO: LED screen

	// sound hardware
	SPEAKER(config, "mono").front_center(); // TODO: verify if stereo

	C140(config, "c140", 49.152_MHz_XTAL / 384 / 6).add_route(ALL_OUTPUTS, "mono", 0.75); // 21.333kHz, copied from other Namco drivers
}


ROM_START( dangbar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "drb1_mprod_mpr.2c", 0x00000, 0x10000, CRC(0f197b71) SHA1(acd7ae6a843fd963d9c0aedfe18183b77c797da3) ) // 1st half is 0xff filled

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "drb1_snd0_snd.8a", 0x00000, 0x20000, CRC(8d424d04) SHA1(12dfd7b8bed22460634c34e57c18c31e38e30b4d) ) // mostly 0xff filled

	ROM_REGION16_BE( 0x200000, "c140", 0 )
	ROM_LOAD16_BYTE( "drb1_voi1.13a",  0x000000, 0x080000, CRC(3891186e) SHA1(459e68a2549b946788e8070c7ff4eeb92ad6f5c8) )
	ROM_LOAD16_BYTE( "drb1_voi2.14a",  0x100000, 0x080000, CRC(ba704115) SHA1(0d027bf7cd9cf0b9d0b5dff7b8ae88ad6b82e45f) )

	ROM_REGION( 0x20000, "ledcpu", 0 )
	ROM_LOAD( "drb1_dot0.ic13", 0x00000, 0x20000, CRC(aeaeb246) SHA1(b470f3450e763411ced795abcc4c8c810dd9b956) )
ROM_END

ROM_START( sspanic )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jp1_mpr0.2c", 0x00000, 0x10000, CRC(04e16a89) SHA1(6211d901cf723ed03a583b38813ee77329e15eea) ) // 0xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "jp1_snd.8a", 0x00000, 0x20000, CRC(46e2401c) SHA1(0edddd42e17da67c57f9c778c5fbf7a76ed287f5) ) // x11xxxxxxxxxxxxxx = 0xFF

	ROM_REGION16_BE( 0x200000, "c140", 0 )
	ROM_LOAD16_BYTE( "jp0_voi1.14a", 0x000000, 0x080000, CRC(49a943ee) SHA1(fae4bdc2812a8f90de980845185161c14608ca5a) )
	ROM_LOAD16_BYTE( "jp0_voi2.13a", 0x100000, 0x080000, CRC(55bb4550) SHA1(961e50366159afd25cd38e3a37c3a06fcfdff1a7) )
ROM_END

} // Anonymous namespace


GAME( 1993, sspanic, 0, dangbar, dangbar, dangbar_state, empty_init, ROT0, "Namco", "Same Same Panic", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1994, dangbar, 0, dangbar, dangbar, dangbar_state, empty_init, ROT0, "Namco", "Dangerous Bar",   MACHINE_IS_SKELETON_MECHANICAL )

// license:BSD-3-Clause
// copyright-holders:

/*
Trio de Bingo (GS470)
(c) Konami

CPU   : 68000
Sound : 054539

Main board (with sound)

470A02      ; 27C4001
470A03      ;  |
470A04      ;  /
470G01      ; 27C4096 main prg

Unfortunately there are no PCB pics or more detailed hardware info available.
Should be the main board for a multi board set, where the satellite boards handle the video part.
A full cabinet pic is available on Gaming History.
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/k054539.h"
#include "speaker.h"


namespace {

class kongs470_state : public driver_device
{
public:
	kongs470_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void kongs470(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void main_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};


void kongs470_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0x100000, 0x10ffff).ram();
}


static INPUT_PORTS_START( tdbingo )
	PORT_START("IN0")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void kongs470_state::machine_start()
{
}


void kongs470_state::kongs470(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 10'000'000); // clock unverified
	m_maincpu->set_addrmap(AS_PROGRAM, &kongs470_state::main_map);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	k054539_device &k054539(K054539(config, "k054539", 18'432'000)); // clock unverified
	k054539.add_route(0, "speaker", 0.75, 1);
	k054539.add_route(1, "speaker", 0.75, 0);
}


ROM_START( tdbingo )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "470g01.bin", 0x00000, 0x80000, CRC(c0015cfb) SHA1(633e1d6c12691ddd31ccdd41b213b6ed33ae4554) )

	ROM_REGION( 0x180000, "k054539", 0 )
	ROM_LOAD( "470a02.bin", 0x000000, 0x080000, CRC(5b68522d) SHA1(2f14857d315b524ec27238f8e33e650843bda256) )
	ROM_LOAD( "470a03.bin", 0x080000, 0x080000, CRC(25ee73f3) SHA1(119c99af839f5def342930dccf45d205ad83aef7) )
	ROM_LOAD( "470a04.bin", 0x100000, 0x080000, CRC(9b432c4b) SHA1(9ab0396579aba215c8adc81b6568b2b94c226589) )
ROM_END

} // Anonymous namespace


GAME( 1995, tdbingo,  0, kongs470, tdbingo, kongs470_state, empty_init, ROT0, "Konami", "Trio de Bingo", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

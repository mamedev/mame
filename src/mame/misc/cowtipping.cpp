// license:BSD-3-Clause
// copyright-holders:

/*

Cow Tipping
Game Refuge / Team Play 2004

TODO:
- Unemulated MC68SZ328 + AMD_29LV640MB;
- Lengthy debug strings in dump hints at a blitter/texture based video section;

PCB sputnik rev. 1.1

- Motorola Super VZ DragonBall MC68SZ328AVH66
- XTAL: 80 MHz (near Cyclone), 32.768k (near SoC and PIC)
- Altera Cyclone EP1C6Q240C7
- TDA8771AH DAC
- PIC12C508 Microcontroller

Two identical looking PCBs were dumped. The dumps are different (even taking into account a suspected bad dump), but it
isn't currently known if it's because they're different sets or because of the saved scores and settings.

The PIC is undumped, but on PCB the game seems to run without it, hanging when checking the ticket dispenser.

*/



#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "machine/mc68328.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class cowtipping_state : public driver_device
{
public:
	cowtipping_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void cowtipping(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};


void cowtipping_state::machine_start()
{
}

uint32_t cowtipping_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void cowtipping_state::main_map(address_map &map)
{
	map(0x00000000, 0x008000ff).rom().region("flash", 0);
	map(0x05000000, 0x0501ffff).ram();
}


static INPUT_PORTS_START( cowtipping )

INPUT_PORTS_END

void cowtipping_state::cowtipping(machine_config &config)
{
	MC68EZ328(config, m_maincpu, 32.768_kHz_XTAL * 506); // 16.580608 MHz, multiplier unknown, actually MC68SZ328
	m_maincpu->set_addrmap(AS_PROGRAM, &cowtipping_state::main_map);

	PIC16C56(config, "pic", 4000000);  // Actually PIC12C508/P, clock not verified

//  TODO: AMD_29LV640MB (64 MBit with Boot Sector)

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));  // wrong
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(cowtipping_state::screen_update));
	screen.set_size(1024, 256);
	screen.set_visarea_full();

	PALETTE(config, "palette").set_entries(65536); // wrong

	SPEAKER(config, "speaker", 2).front();
}


ROM_START( cowtipp )
	ROM_REGION16_BE(0x800100, "flash", 0)
	ROM_LOAD16_WORD_SWAP( "am29lv640mb-u5-3287.u5", 0x000000, 0x800100, CRC(5aa4ac7c) SHA1(4b882008e13e581c0131875a1845c7e78696087e) )
	// empty space at u6

	ROM_REGION( 0x1000, "pic", 0 )
	ROM_LOAD( "56_pic12c508-04p.u3", 0x000, 0x09db, NO_DUMP )
ROM_END

ROM_START( cowtippa )
	ROM_REGION16_BE(0x800100, "flash", 0)
	ROM_LOAD16_WORD_SWAP( "am29lv640mb-u5-3276.u5", 0x000000, 0x800100, BAD_DUMP CRC(05ce9f7f) SHA1(c27772c9a6a1feaf4d83ae0902759fa25642ad65) ) // suspected bad, seems to have some problems with bit 5
	// empty space at u6

	ROM_REGION( 0x1000, "pic", 0 )
	ROM_LOAD( "54_pic12c508-04p.u3", 0x000, 0x09db, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 2004, cowtipp,  0,       cowtipping, cowtipping, cowtipping_state, empty_init, ROT270, "Game Refuge / Team Play", "Cow Tipping - Shake Cattle & Roll (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2004, cowtippa, cowtipp, cowtipping, cowtipping, cowtipping_state, empty_init, ROT270, "Game Refuge / Team Play", "Cow Tipping - Shake Cattle & Roll (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

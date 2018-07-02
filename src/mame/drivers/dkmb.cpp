// license:BSD-3-Clause
// copyright-holders:

/*
Donkey Kong / Donkey Kong Jr / Mario Bros
Namco / Nintendo / Cosmodog 2003

PCB LUNA REV. 1.0 (C) 2003 COSMODOG, LTD.

Maincpu: Motorola MPC603RRX266LC
XTAL: SG-8002CA @ 75 MHz (not clearly readable)
Video: Xilinx Spartan XC2S200 FPGA
Memory: 2 x 48LC2M32B2 SDR SDRAM
PIC12C508 Microcontroller

Some interesting notes from the dumper:
* Donkey Kong and Donkey Kong Jr are vertical games, while Mario Bros is originally Horizontal.
  This is achieved on this board by cropping Mario Bros slightly on the sides, while the life bar BG area is compressed slightly.
* It appears that the program ROM is responsible for programming the FPGA bitstream.
  Sound effects are supported as samples and triggers are caught by the emulator.
* The continue features are not implemented through modifications of the game, but rather through hacks applied with the emulator.
  I've had it glitch out and give me a 'Continue?' screen while I still had remaining lives in Donkey Kong.
*/



#include "emu.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "cpu/powerpc/ppc.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class dkmb_state : public driver_device
{
public:
	dkmb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void dkmb(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);
};


void dkmb_state::machine_start()
{
}

uint32_t dkmb_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void dkmb_state::main_map(address_map &map)
{
	map(0x00ff0000, 0x00ffffff).ram();
	map(0xffe00000, 0xffffffff).rom().region("maincpu", 0);
}


static INPUT_PORTS_START( dkmb )

INPUT_PORTS_END

MACHINE_CONFIG_START(dkmb_state::dkmb)

	MCFG_DEVICE_ADD("maincpu", PPC603R, 75'000'000) // Actually MPC603RRX266LC
	MCFG_DEVICE_PROGRAM_MAP(main_map)

	MCFG_DEVICE_ADD("pic", PIC16C56, 4'000'000)  // Actually PIC12C508, clock not verified

	MCFG_SCREEN_ADD("screen", RASTER)  // wrong
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DRIVER(dkmb_state, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 256, 0, 240)

	MCFG_PALETTE_ADD("palette", 65536)

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
MACHINE_CONFIG_END


ROM_START( dkmb )
	ROM_REGION64_BE(0x200000, "maincpu", 0)
	ROM_LOAD("donkeykong.u14", 0x00000, 0x200000, CRC(7895fe2d) SHA1(134174c02ce3c5d28a890ca57dc36677873b1dc8) )

	ROM_REGION( 0x1000, "pic", 0 )
	ROM_LOAD("12c508.u12", 0x000, 0x09db, CRC(3adb3e33) SHA1(36a96886d83b64633eea83e57bdfa8a20c6d4f6a) )
ROM_END

GAME( 2003, dkmb, 0, dkmb, dkmb, dkmb_state, empty_init, ROT270, "Namco / Nintendo / Cosmodog", "Donkey Kong / Donkey Kong Jr / Mario Bros", MACHINE_IS_SKELETON )

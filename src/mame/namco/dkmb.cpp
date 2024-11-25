// license:BSD-3-Clause
// copyright-holders:David Haywood

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


namespace {

class dkmb_state : public driver_device
{
public:
	dkmb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_framebuffer(*this, "framebuffer")
	{ }

	void dkmb(machine_config &config);

	u64 unk_2060000_r();
	u64 unk_20c0010_r();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint64_t> m_framebuffer;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};


void dkmb_state::machine_start()
{
}

uint32_t dkmb_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int count = 0;
	for (int y = 0; y < 256; y++)
	{
		uint32_t *const dst = &bitmap.pix(y);

		for (int x = 0; x < 1024 / 2; x++)
		{
			uint64_t const val = m_framebuffer[count];

			dst[(x * 2) + 0] = (val >> 32) & 0x00ffffff;
			dst[(x * 2) + 1] = (val >> 0)  & 0x00ffffff;

			count++;
		}
	}

	return 0;
}


u64 dkmb_state::unk_2060000_r()
{
	return ((u64)machine().rand()) << 56;
}

u64 dkmb_state::unk_20c0010_r()
{
	return ((u64)machine().rand()) << 56;
}

void dkmb_state::main_map(address_map &map)
{
	map(0x00000000, 0x000fffff).ram(); // copies code to RAM and runs it from here
	map(0x00100000, 0x001fffff).ram().share("framebuffer"); // seems to be a 1024x256 ARGB bitmap, with 320 entries used for each line and 224 lines, so framebuffer? (but then gets overwritten with data not of that format?)

	map(0x00e00000, 0x00ffffff).ram(); // size uncertain

	map(0x02080000, 0x02080007).ram(); // write
	map(0x02060000, 0x02060007).r(FUNC(dkmb_state::unk_2060000_r)); // read
	map(0x020c0000, 0x020c0007).ram(); // write
	map(0x020c0010, 0x020c0017).r(FUNC(dkmb_state::unk_20c0010_r)); // read

	map(0x04090000, 0x0409ffff).ram(); // size uncertain

	// map(0xff000000, 0xff0fffff) // looks like flash ROM access, mirror of main ROM or just checking if extra exist?
	// map(0xff800000, 0xff8fffff) // looks like flash ROM access, mirror of main ROM or just checking if extra exist?

	map(0xffe00000, 0xffffffff).rom().region("maincpu", 0);
}


static INPUT_PORTS_START( dkmb )

INPUT_PORTS_END

void dkmb_state::dkmb(machine_config &config)
{
	PPC603R(config, m_maincpu, 75'000'000); // Actually MPC603RRX266LC
	m_maincpu->set_addrmap(AS_PROGRAM, &dkmb_state::main_map);

	PIC16C56(config, "pic", 4'000'000);  // Actually PIC12C508, clock not verified

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));  // wrong
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(dkmb_state::screen_update));
	screen.set_size(1024, 256);
	screen.set_visarea_full();

	PALETTE(config, "palette").set_entries(65536);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}


ROM_START( dkmb )
	ROM_REGION64_BE(0x200000, "maincpu", 0)
	ROM_LOAD("donkeykong.u14", 0x00000, 0x200000, CRC(7895fe2d) SHA1(134174c02ce3c5d28a890ca57dc36677873b1dc8) ) // what type of ROM is this?

	ROM_REGION( 0x1000, "pic", 0 )
	/*
	    Intel HEX format dump.  When converted to binary using
	    "srec_cat.exe 12c508.u12 -intel -o 12c508.bin -binary"
	    this contains only the string
	    "Copyright 2001 Core Technologies why are you looking in here?"
	    in the first 0x80 bytes, which is the unprotected area of the PIC.
	    the rest is blank, therefore protected and a bad dump
	*/
	ROM_LOAD("12c508.u12", 0x000, 0x09db, BAD_DUMP CRC(3adb3e33) SHA1(36a96886d83b64633eea83e57bdfa8a20c6d4f6a) )
ROM_END

} // anonymous namespace


GAME( 2003, dkmb, 0, dkmb, dkmb, dkmb_state, empty_init, ROT270, "Namco / Nintendo / Cosmodog", "Donkey Kong / Donkey Kong Jr / Mario Bros", MACHINE_IS_SKELETON )

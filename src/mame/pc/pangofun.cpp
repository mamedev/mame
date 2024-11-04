// license:BSD-3-Clause
// copyright-holders:David Haywood
/**************************************************************************************************

  It's a standard 486 PC motherboard, gfx card etc. with expansion ROM board

  probably impossible to emulate right now due to the bad / missing (blank when read) rom
  although it would be a good idea if somebody checked for sure

TODO:
- Doesn't detect proper CPU type;
- "Memory test fail" at POST, needs um8498f "memory controller" emulation
  (non-PCI with shadow RAM?), also cfr. correlated at.cpp romsets;
- Sometimes POST will trip an invalid groupFF_16 modrm FF at 000F00B2, causing an hang;
- bp 932d1, ROM banking that reads at 0xffffe? Is um8498f ROM boot capable?

===================================================================================================
readme by f205v

Game: Pango Fun
Anno: 1995
Produttore: InfoCube (Pisa-Italy)
N.revisione: rl00rv00

CPUs:-----------

Main PCB (it's a standard 486 motherboard):

1x 80486
1x oscillator 14.31818 MHz

Video PCB (it's a standard VGA ISA board):

1x CIRRUS CLGD5401-42QC-B-31063-198AC

Sound PCB (missing, it's a standard ISA 16bit sound card):
?

ROMs PCB (it's a custom PCB, with ISA connector to motherboard on one side and JAMMA connetcor on the other side):
1x NE555P


ROMs:-----------

Main PCB (it's a standard 486 motherboard):
1x 27C512 (bios)

Video PCB (it's a standard VGA ISA board):
1x maskrom (28pin) (VGAbios)(not dumped)

Sound PCB (missing, it's a standard ISA 16bit sound card):

ROMs PCB (it's a custom PCB, with ISA connector to motherboard on one side and JAMMA connector on the other side):
5x AM27C040 (u11,u12,u31,u32,u33)
1x TMS27C040 (u13)(probably corrupted)
1x INTEL27C010A (u39)
4x PALCE16V8H (u5,u25,u26,u28)(not dumped yet)
1x PALCE20V8H (u42)
1x PALCE20V8H (u44)(not dumped yet)
2x PALCE22V10H (u45,u49)(not dumped yet)


Notes:----------

Main PCB (it's a standard 486 motherboard):
1x Keyboard DIN connector (not used)

Video PCB (it's a standard VGA ISA board):
1x VGA connetctor (to ROMs PCB)
1x red/black cable (to ROMs PCB)

Sound PCB (missing, it's a standard ISA 16bit sound card):
1x stereo audio out (to ROMs PCB)

ROMs PCB (it's a custom PCB, with ISA connector to motherboard on one side and JAMMA connetcor on the other side):
1x ISA connector (into motherboard)
1x JAMMA edge connector
1x VGA in connector (from Video PCB)
1x stereo audio jack (from sound card)
1x 13x2 legs connector(only 2 legs used for red/black cable from Video PCB)
1x trimmer (volume)
2x 8 switches dip

Info:----------

------------------------------------------
I have:
3 Main PCBs (all of them without 80486)
11 Video PCBs
4 ROMs PCBs (only one has EPROMs)
------------------------------------------

Game was programmed by Giovanni Tummarello and Roberto Molinelli in 1990 for Amiga;
it was an action/strategy videogame published by Proxxima Software (Rome, Italy) and
later ported to PC machines and published in 1994 by AIM Games software (US) and in
Arcade Version (Coin-Op) by InfoCube (Pisa, Italy)

**************************************************************************************************/

#include "emu.h"

#include "pcshare.h"

#include "cpu/i386/i386.h"
#include "video/pc_vga.h"
#include "screen.h"


namespace {

class pangofun_state : public pcat_base_state
{
public:
	pangofun_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag) { }

	void pangofun(machine_config &config);

	void init_pangofun();

private:
	virtual void machine_start() override ATTR_COLD;
	void pcat_io(address_map &map) ATTR_COLD;
	void pcat_map(address_map &map) ATTR_COLD;
};


void pangofun_state::pcat_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000a0000, 0x000bffff).rw("vga", FUNC(vga_device::mem_r), FUNC(vga_device::mem_w));
	map(0x000c0000, 0x000c7fff).rom().region("video_bios", 0);
	map(0x000e0000, 0x000effff).rom().region("game_prg", 0);
	map(0x000f0000, 0x000fffff).rom().region("bios", 0);
	/* TODO: correct RAM mapping/size? */
	map(0x00100000, 0x01ffffff).ram();
	map(0x02000000, 0xfffeffff).noprw();
	map(0xffff0000, 0xffffffff).rom().region("bios", 0);
}

void pangofun_state::pcat_io(address_map &map)
{
	pcat32_io_common(map);
	map(0x00e0, 0x00e3).nopw();
	map(0x03b0, 0x03df).m("vga", FUNC(vga_device::io_map));
}

static INPUT_PORTS_START( pangofun )
INPUT_PORTS_END

void pangofun_state::machine_start()
{
}

void pangofun_state::pangofun(machine_config &config)
{
	/* basic machine hardware */
	I486(config, m_maincpu, XTAL(40'000'000)); // um486sxlc-40
	m_maincpu->set_addrmap(AS_PROGRAM, &pangofun_state::pcat_map);
	m_maincpu->set_addrmap(AS_IO, &pangofun_state::pcat_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	/* video hardware */
	// TODO: map to ISA bus, CLGD5401
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.1748_MHz_XTAL, 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(vga_device::screen_update));

	vga_device &vga(VGA(config, "vga", 0));
	vga.set_screen("screen");
	vga.set_vram_size(0x100000);

	pcat_common(config);

	// TODO: um8498f
}


ROM_START(pangofun)
	ROM_REGION32_LE(0x10000, "bios", 0) /* motherboard bios */
	ROM_LOAD("bios.bin", 0x000000, 0x10000, CRC(e70168ff) SHA1(4a0d985c218209b7db2b2d33f606068aae539020) )

	ROM_REGION32_LE(0x20000, "video_bios", 0)    /* Trident TVGA9000 BIOS */
	ROM_LOAD16_BYTE("prom.vid", 0x00000, 0x04000, CRC(ad7eadaf) SHA1(ab379187914a832284944e81e7652046c7d938cc) )
	ROM_CONTINUE(               0x00001, 0x04000 )

	/* this is what was on the rom board, mapping unknown */
	ROM_REGION32_LE(0xa00000, "game_prg", 0)    /* rom board */
	ROM_LOAD("bank8.u39", 0x000000, 0x20000, CRC(72422c66) SHA1(40b8cca3f99925cf019053921165f6a4a30d784d) )
	ROM_LOAD16_BYTE("bank0.u11", 0x100001, 0x80000, CRC(6ce951d7) SHA1(1dd09491c651920a8a507bdc6584400367e5a292) )
	ROM_LOAD16_BYTE("bank0.u31", 0x100000, 0x80000, CRC(b6c06baf) SHA1(79074b086d24737d629272d98f17de6e1e650485) )
	/* Following two references to a SB Pro clone sound card. */
	ROM_LOAD16_BYTE("bank1.u12", 0x200001, 0x80000, CRC(5adc1f2e) SHA1(17abde7a2836d042a698661339eefe242dd9af0d) )
	ROM_LOAD16_BYTE("bank1.u32", 0x200000, 0x80000, CRC(5647cbf6) SHA1(2e53a74b5939b297fa1a77441017cadc8a19ddef) )
	ROM_LOAD16_BYTE("bank2.u13", 0x300001, 0x80000, BAD_DUMP CRC(504bf849) SHA1(13a184ec9e176371808938015111f8918cb4df7d) ) // EMPTY! (Definitely Bad, interleaves with the next ROM)
	ROM_LOAD16_BYTE("bank2.u33", 0x300000, 0x80000, CRC(272ecfb6) SHA1(6e1b6bdef62d953de102784ba0148fb20182fa87) )
					/*bank3.u14 , NOT POPULATED */
					/*bank3.u34 , NOT POPULATED */
					/*bank4.u15 , NOT POPULATED */
					/*bank4.u35 , NOT POPULATED */
					/*bank5.u16 , NOT POPULATED */
					/*bank5.u36 , NOT POPULATED */
					/*bank6.u17 , NOT POPULATED */
					/*bank6.u37 , NOT POPULATED */
					/*bank7.u18 , NOT POPULATED */
					/*bank7.u37 , NOT POPULATED */
					/*bank8.u19 , NOT POPULATED */
ROM_END

void pangofun_state::init_pangofun()
{
}

} // anonymous namespace


GAME( 1995, pangofun,  0,   pangofun, pangofun, pangofun_state, init_pangofun, ROT0, "InfoCube", "Pango Fun (Italy)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )

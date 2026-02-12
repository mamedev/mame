// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Pango Fun

MS-DOS 5.0 based

TODO:
- Needs um8498f chipset improvements;
- Move stuff to ISA bus (4 ISA16 + 1 ISA8 + 1 ISA16);
- Hookup GD5401 "AVGA1" properly;
- Understand what's the use of the goofy rig between AVGA1 DCLK and EVIDEO pins to feature
  connector of the romdisk card, just a bypass to avoid showing text mode GFXs?
  Also note an extra VGA connector on romdisk card.
- Hookup jadarom romdisk, understand why it "cannot load program";

===================================================================================================
readme by f205v

Game: Pango Fun
Year: 1995
Publisher: InfoCube (Pisa-Italy)
Revision number: rl00rv00

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
1x VGA connector (to ROMs PCB)
1x red/black cable (to ROMs PCB)

Sound PCB (missing, it's a standard ISA 16bit sound card):
1x stereo audio out (to ROMs PCB)

ROMs PCB (it's a custom PCB, with ISA connector to motherboard on one side and JAMMA connector on the other side):
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

#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/i386/i386.h"
#include "machine/at_keybc.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/um8498f.h"
#include "sound/spkrdev.h"
#include "video/pc_vga.h"

#include "screen.h"
#include "speaker.h"


namespace {

class pangofun_state : public driver_device
{
public:
	pangofun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_chipset(*this, "chipset")
		, m_isabus(*this, "isabus")
		, m_speaker(*this, "speaker")
	{ }

	void pangofun(machine_config &config);

private:
	required_device<i486_device> m_maincpu;
	required_device<um8498f_device> m_chipset;
	required_device<isa16_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;

	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


void pangofun_state::main_map(address_map &map)
{
//	map(0x00000000, 0x0009ffff).ram();
//	map(0x000a0000, 0x000bffff).rw("vga", FUNC(vga_device::mem_r), FUNC(vga_device::mem_w));
//	map(0x000c0000, 0x000c7fff).rom().region("video_bios", 0);
	// boot ROM has four 0xaa55 headers
	// $00000 has CON/AUX/LPT/COM refs
	// $08000 contains COMMAND.COM
	// $10000 and $18000 are mostly similar copies, with shifted pointers.
	// former looks involved in copying stuff of latter, wants reading at $e0000,
	// moves in conventional memory and jumps to PC=700
	map(0x000d0000, 0x000d7fff).rom().region("romdisk", 0x10000);
	map(0x000d8000, 0x000dffff).rom().region("romdisk", 0x18000);
//	map(0x000e0000, 0x000e7fff).rom().region("romdisk", 0x00000);
//	map(0x000e8000, 0x000effff).rom().region("romdisk", 0x08000);
//	map(0x000f0000, 0x000fffff).rom().region("bios", 0);
//	map(0x00100000, 0x01ffffff).ram();
	map(0x00100000, 0x03ffffff).noprw();
//	map(0x02000000, 0xfffeffff).noprw();
	map(0xfffe0000, 0xffffffff).rom().region("bios", 0);
}

void pangofun_state::main_io(address_map &map)
{
	map(0x00e0, 0x00e3).nopw(); // timestamp stuff?
//	map(0x03b0, 0x03df).m("vga", FUNC(vga_device::io_map));
}


void pangofun_state::pangofun(machine_config &config)
{
	/* basic machine hardware */
	I486(config, m_maincpu, XTAL(40'000'000)); // um486sxlc-40
	m_maincpu->set_addrmap(AS_PROGRAM, &pangofun_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &pangofun_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("chipset", FUNC(um8498f_device::int_ack_r));

	UM8498F(config, m_chipset, XTAL(25'000'000), "maincpu", "bios", "keybc", "ram", "isabus");
	m_chipset->hold().set([this] (int state) {
		// halt cpu
		m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

		// and acknowledge hold
		m_chipset->hlda_w(state);
	});
	m_chipset->nmi().set_inputline("maincpu", INPUT_LINE_NMI);
	m_chipset->intr().set_inputline("maincpu", INPUT_LINE_IRQ0);
	m_chipset->cpureset().set_inputline("maincpu", INPUT_LINE_RESET);
	m_chipset->a20m().set_inputline("maincpu", INPUT_LINE_A20);
	// isa dma
	m_chipset->ior().set([this] (offs_t offset) -> u16 {
		if (offset < 4)
			return m_isabus->dack_r(offset);
		else
			return m_isabus->dack16_r(offset);
	});
	m_chipset->iow().set([this] (offs_t offset, u16 data) {
		if (offset < 4)
			m_isabus->dack_w(offset, data);
		else
			m_isabus->dack16_w(offset, data);
	});
	m_chipset->tc().set([this] (offs_t offset, u8 data) { m_isabus->eop_w(offset, data); });
	// speaker
	m_chipset->spkr().set([this] (int state) { m_speaker->level_w(state); });

	// unknown, not provided. Chipset can go up to 32M, but will go "memory fail" with that (?)
	RAM(config, "ram").set_default_size("64M");

	ISA16(config, m_isabus, 0);
	m_isabus->set_memspace("maincpu", AS_PROGRAM);
	m_isabus->set_iospace("maincpu", AS_IO);
	m_isabus->iochck_callback().set(m_chipset, FUNC(um8498f_device::iochck_w));
	m_isabus->irq2_callback().set(m_chipset, FUNC(um8498f_device::irq09_w));
	m_isabus->irq3_callback().set(m_chipset, FUNC(um8498f_device::irq03_w));
	m_isabus->irq4_callback().set(m_chipset, FUNC(um8498f_device::irq04_w));
	m_isabus->irq5_callback().set(m_chipset, FUNC(um8498f_device::irq05_w));
	m_isabus->irq6_callback().set(m_chipset, FUNC(um8498f_device::irq06_w));
	m_isabus->irq7_callback().set(m_chipset, FUNC(um8498f_device::irq07_w));
	m_isabus->irq10_callback().set(m_chipset, FUNC(um8498f_device::irq10_w));
	m_isabus->irq11_callback().set(m_chipset, FUNC(um8498f_device::irq11_w));
	m_isabus->irq12_callback().set(m_chipset, FUNC(um8498f_device::irq12_w));
	m_isabus->irq14_callback().set(m_chipset, FUNC(um8498f_device::irq14_w));
	m_isabus->irq15_callback().set(m_chipset, FUNC(um8498f_device::irq15_w));
	m_isabus->drq0_callback().set(m_chipset, FUNC(um8498f_device::dreq0_w));
	m_isabus->drq1_callback().set(m_chipset, FUNC(um8498f_device::dreq1_w));
	m_isabus->drq2_callback().set(m_chipset, FUNC(um8498f_device::dreq2_w));
	m_isabus->drq3_callback().set(m_chipset, FUNC(um8498f_device::dreq3_w));
	m_isabus->drq5_callback().set(m_chipset, FUNC(um8498f_device::dreq5_w));
	m_isabus->drq6_callback().set(m_chipset, FUNC(um8498f_device::dreq6_w));
	m_isabus->drq7_callback().set(m_chipset, FUNC(um8498f_device::dreq7_w));

	ISA16_SLOT(config, "isa1", 0, "isabus", pc_isa16_cards, "avga1", false);
	ISA16_SLOT(config, "isa2", 0, "isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "isabus", pc_isa8_cards,  nullptr, false);
	// TODO: this space will be reserved to the romdisk
	ISA16_SLOT(config, "isa6", 0, "isabus", pc_isa16_cards, nullptr, false);

	at_kbc_device_base &keybc(AT_KEYBOARD_CONTROLLER(config, "keybc", XTAL(12'000'000)));
	keybc.hot_res().set(m_chipset, FUNC(um8498f_device::kbrst_w));
	keybc.gate_a20().set(m_chipset, FUNC(um8498f_device::gatea20_w));
	keybc.kbd_irq().set(m_chipset, FUNC(um8498f_device::irq01_w));
	keybc.kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	keybc.kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	// Temporary default
	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, "ms_naturl"));
	pc_kbdc.out_clock_cb().set(keybc, FUNC(at_kbc_device_base::kbd_clk_w));
	pc_kbdc.out_data_cb().set(keybc, FUNC(at_kbc_device_base::kbd_data_w));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START(pangofun)
	ROM_REGION32_LE(0x20000, "romdisk", 0 )
	ROM_LOAD("bank8.u39", 0x000000, 0x20000, CRC(72422c66) SHA1(40b8cca3f99925cf019053921165f6a4a30d784d) )
	/*bank8.u19 , NOT POPULATED */

	ROM_REGION32_LE(0x20000, "bios", 0 ) /* motherboard bios */
	ROM_COPY( "romdisk",  0x000000, 0x00000, 0x10000 )
	ROM_LOAD( "bios.bin", 0x010000, 0x10000, CRC(e70168ff) SHA1(4a0d985c218209b7db2b2d33f606068aae539020) )

	/* this is what was on the rom board, mapping unknown */
	ROM_REGION32_LE(0x800000, "game_prg", ROMREGION_ERASEFF )    /* rom board */
	ROM_LOAD16_BYTE( "bank0.u11", 0x000001, 0x80000, CRC(6ce951d7) SHA1(1dd09491c651920a8a507bdc6584400367e5a292) )
	ROM_LOAD16_BYTE( "bank0.u31", 0x000000, 0x80000, CRC(b6c06baf) SHA1(79074b086d24737d629272d98f17de6e1e650485) )
	/* Following two  references to a SB Pro clone sound card. */
	ROM_LOAD16_BYTE( "bank1.u12", 0x100001, 0x80000, CRC(5adc1f2e) SHA1(17abde7a2836d042a698661339eefe242dd9af0d) )
	ROM_LOAD16_BYTE( "bank1.u32", 0x100000, 0x80000, CRC(5647cbf6) SHA1(2e53a74b5939b297fa1a77441017cadc8a19ddef) )
	ROM_LOAD16_BYTE( "bank2.u13", 0x200001, 0x80000, BAD_DUMP CRC(504bf849) SHA1(13a184ec9e176371808938015111f8918cb4df7d) ) // EMPTY! (Definitely Bad, interleaves with the next ROM)
	ROM_LOAD16_BYTE( "bank2.u33", 0x200000, 0x80000, CRC(272ecfb6) SHA1(6e1b6bdef62d953de102784ba0148fb20182fa87) )
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
ROM_END

} // anonymous namespace


GAME( 1995, pangofun,  0,   pangofun, 0, pangofun_state, empty_init, ROT0, "InfoCube", "Pango Fun (Italy)", MACHINE_NOT_WORKING )

// license:BSD-3-Clause
// copyright-holders:David Haywood
/* It's a standard 486 PC motherboard, gfx card etc. with expansion ROM board

 probably impossible to emulate right now due to the bad / missing (blank when read) rom
 although it would be a good idea if somebody checked for sure

TODO:
- bp 932d1, ROM banking that reads at 0xffffe???

*/

/*

readme by f205v

Game: Pango Fun
Anno: 1995
Produttore: InfoCube (Pisa-Italy)
N.revisione: rl00rv00

CPUs:-----------

Main PCB (it's a standard 486 motherboard):

1x 80486
1x oscillator 14.31818 MHz

Video PCB (it's a standard VGA EISA board):

1x CIRRUS CLGD5401-42QC-B-31063-198AC

Sound PCB (missing, it's a standard ISA 16bit sound card):
?

ROMs PCB (it's a custom PCB, with EISA connector to motherboard on one side and JAMMA connetcor on the other side):
1x NE555P


ROMs:-----------

Main PCB (it's a standard 486 motherboard):
1x 27C512 (bios)

Video PCB (it's a standard VGA EISA board):
1x maskrom (28pin) (VGAbios)(not dumped)

Sound PCB (missing, it's a standard ISA 16bit sound card):

ROMs PCB (it's a custom PCB, with EISA connector to motherboard on one side and JAMMA connetcor on the other side):
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

Video PCB (it's a standard VGA EISA board):
1x VGA connetctor (to ROMs PCB)
1x red/black cable (to ROMs PCB)

Sound PCB (missing, it's a standard ISA 16bit sound card):
1x stereo audio out (to ROMs PCB)

ROMs PCB (it's a custom PCB, with EISA connector to motherboard on one side and JAMMA connetcor on the other side):
1x EISA connector (into motherboard)
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

*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pcshare.h"
#include "video/pc_vga.h"


class pangofun_state : public pcat_base_state
{
public:
	pangofun_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag) { }

	DECLARE_DRIVER_INIT(pangofun);
	virtual void machine_start() override;
};


static ADDRESS_MAP_START( pcat_map, AS_PROGRAM, 32, pangofun_state )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_DEVREADWRITE8("vga", vga_device, mem_r, mem_w, 0xffffffff)
	AM_RANGE(0x000c0000, 0x000c7fff) AM_ROM AM_REGION("video_bios", 0)
	AM_RANGE(0x000e0000, 0x000effff) AM_ROM AM_REGION("game_prg", 0)
	AM_RANGE(0x000f0000, 0x000fffff) AM_ROM AM_REGION("bios", 0 )
	/* TODO: correct RAM mapping/size? */
	AM_RANGE(0x00100000, 0x00ffffff) AM_NOP
	AM_RANGE(0x01000000, 0x01ffffff) AM_RAM
	AM_RANGE(0x02000000, 0xfffeffff) AM_NOP
	AM_RANGE(0xffff0000, 0xffffffff) AM_ROM AM_REGION("bios", 0 )
ADDRESS_MAP_END

static ADDRESS_MAP_START( pcat_io, AS_IO, 32, pangofun_state )
	AM_IMPORT_FROM(pcat32_io_common)
	AM_RANGE(0x00e0, 0x00e3) AM_WRITENOP
	AM_RANGE(0x03b0, 0x03bf) AM_DEVREADWRITE8("vga", vga_device, port_03b0_r, port_03b0_w, 0xffffffff)
	AM_RANGE(0x03c0, 0x03cf) AM_DEVREADWRITE8("vga", vga_device, port_03c0_r, port_03c0_w, 0xffffffff)
	AM_RANGE(0x03d0, 0x03df) AM_DEVREADWRITE8("vga", vga_device, port_03d0_r, port_03d0_w, 0xffffffff)
ADDRESS_MAP_END

#define AT_KEYB_HELPER(bit, text, key1) \
	PORT_BIT( bit, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(text) PORT_CODE(key1)

static INPUT_PORTS_START( pangofun )
	PORT_START("pc_keyboard_0")
	PORT_BIT ( 0x0001, 0x0000, IPT_UNUSED )     /* unused scancode 0 */
	AT_KEYB_HELPER( 0x0002, "Esc",          KEYCODE_Q           ) /* Esc                         01  81 */

	PORT_START("pc_keyboard_1")
	AT_KEYB_HELPER( 0x0020, "Y",            KEYCODE_Y           ) /* Y                           15  95 */
	AT_KEYB_HELPER( 0x1000, "Enter",        KEYCODE_ENTER       ) /* Enter                       1C  9C */

	PORT_START("pc_keyboard_2")

	PORT_START("pc_keyboard_3")
	AT_KEYB_HELPER( 0x0002, "N",            KEYCODE_N           ) /* N                           31  B1 */
	AT_KEYB_HELPER( 0x0800, "F1",           KEYCODE_S           ) /* F1                          3B  BB */

	PORT_START("pc_keyboard_4")

	PORT_START("pc_keyboard_5")

	PORT_START("pc_keyboard_6")
	AT_KEYB_HELPER( 0x0040, "(MF2)Cursor Up",       KEYCODE_UP          ) /* Up                          67  e7 */
	AT_KEYB_HELPER( 0x0080, "(MF2)Page Up",         KEYCODE_PGUP        ) /* Page Up                     68  e8 */
	AT_KEYB_HELPER( 0x0100, "(MF2)Cursor Left",     KEYCODE_LEFT        ) /* Left                        69  e9 */
	AT_KEYB_HELPER( 0x0200, "(MF2)Cursor Right",    KEYCODE_RIGHT       ) /* Right                       6a  ea */
	AT_KEYB_HELPER( 0x0800, "(MF2)Cursor Down",     KEYCODE_DOWN        ) /* Down                        6c  ec */
	AT_KEYB_HELPER( 0x1000, "(MF2)Page Down",       KEYCODE_PGDN        ) /* Page Down                   6d  ed */
	AT_KEYB_HELPER( 0x4000, "Del",                  KEYCODE_A           ) /* Delete                      6f  ef */

	PORT_START("pc_keyboard_7")
INPUT_PORTS_END

void pangofun_state::machine_start()
{
}

static MACHINE_CONFIG_START( pangofun, pangofun_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I486, 25000000 )    /* I486 ?? Mhz (25 according to POST) */
	MCFG_CPU_PROGRAM_MAP(pcat_map)
	MCFG_CPU_IO_MAP(pcat_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_1", pic8259_device, inta_cb)

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_vga )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */

	MCFG_FRAGMENT_ADD( pcat_common )
MACHINE_CONFIG_END


ROM_START(pangofun)
	ROM_REGION32_LE(0x20000, "bios", 0) /* motherboard bios */
	ROM_LOAD("bios.bin", 0x000000, 0x10000, CRC(e70168ff) SHA1(4a0d985c218209b7db2b2d33f606068aae539020) )

	ROM_REGION(0x20000, "video_bios", 0)    /* Trident TVGA9000 BIOS */
	ROM_LOAD16_BYTE("prom.vid", 0x00000, 0x04000, CRC(ad7eadaf) SHA1(ab379187914a832284944e81e7652046c7d938cc) )
	ROM_CONTINUE(               0x00001, 0x04000 )

	/* this is what was on the rom board, mapping unknown */
	ROM_REGION(0xa00000, "game_prg", 0)    /* rom board */
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

DRIVER_INIT_MEMBER(pangofun_state,pangofun)
{
}

GAME( 1995, pangofun,  0,   pangofun, pangofun, pangofun_state, pangofun, ROT0, "InfoCube", "Pango Fun (Italy)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )

/********************************************************************************************************************

Tournament Solitaire (c) 1995 Dynamo

Unmodified 486 PC-AT HW. Input uses a trackball device that isn't PC standard afaik.

Jet Way Information Co. OP495SLC motherboard
 - AMD Am486-DX40 CPU
 - Trident TVGA9000i video card

preliminary driver by Angelo Salese

TODO:
- Returns CMOS checksum error, can't enter into BIOS setup screens to set that up ... it's certainly a MESS-to-MAME
  conversion bug or a keyboard device issue, since it works fine in MESS. (Update: it's the keyboard device)

keyboard trick;
- Set 0x41c to zero then set the scancode accordingly:
- bp f1699 ah = 0x3b
- bp f53b9 al = scancode
- bp f08d9 ah = scancode

0x48 is up 0x4d is down 0x50 is right 0x4b is left
0x3c/0x3d is pageup/pagedown
0x01 is esc
0x0d is enter

********************************************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/mc146818.h"
#include "machine/8042kbdc.h"
#include "machine/pcshare.h"
#include "video/pc_vga.h"


class pcat_dyn_state : public driver_device
{
public:
	pcat_dyn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

};


//ce9b8
/* TODO: understand the proper ROM loading.*/
static ADDRESS_MAP_START( pcat_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_RAM
	AM_RANGE(0x000c0000, 0x000c7fff) AM_ROM AM_REGION("video_bios", 0)
	AM_RANGE(0x000c8000, 0x000cffff) AM_RAM
//  AM_RANGE(0x000d0000, 0x000d7fff) AM_RAM AM_REGION("disk_bios", 0)
//  AM_RANGE(0x000d8000, 0x000dffff) AM_RAM AM_REGION("disk_bios", 0)
//  AM_RANGE(0x000e0000, 0x000effff) AM_ROM AM_REGION("game_prg", 0)
	AM_RANGE(0x000f0000, 0x000fffff) AM_ROM AM_REGION("bios", 0 )
	AM_RANGE(0x00100000, 0x001fffff) AM_RAM //AM_REGION("game_prg", 0)
//  AM_RANGE(0x00200000, 0x00ffffff) AM_RAM
	//AM_RANGE(0x01000000, 0x01ffffff) AM_RAM
	AM_RANGE(0xffff0000, 0xffffffff) AM_ROM AM_REGION("bios", 0 )
ADDRESS_MAP_END

static ADDRESS_MAP_START( pcat_io, AS_IO, 32 )
	AM_IMPORT_FROM(pcat32_io_common)
	AM_RANGE(0x0070, 0x007f) AM_DEVREADWRITE8_MODERN("rtc", mc146818_device, read, write, 0xffffffff)
ADDRESS_MAP_END

#define AT_KEYB_HELPER(bit, text, key1) \
	PORT_BIT( bit, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(text) PORT_CODE(key1)

static INPUT_PORTS_START( pcat_dyn )
	PORT_START("pc_keyboard_0")
	PORT_BIT ( 0x0001, 0x0000, IPT_UNUSED ) 	/* unused scancode 0 */
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
	AT_KEYB_HELPER( 0x0040, "(MF2)Cursor Up",		KEYCODE_UP          ) /* Up                          67  e7 */
	AT_KEYB_HELPER( 0x0080, "(MF2)Page Up",			KEYCODE_PGUP        ) /* Page Up                     68  e8 */
	AT_KEYB_HELPER( 0x0100, "(MF2)Cursor Left",		KEYCODE_LEFT        ) /* Left                        69  e9 */
	AT_KEYB_HELPER( 0x0200, "(MF2)Cursor Right",	KEYCODE_RIGHT       ) /* Right                       6a  ea */
	AT_KEYB_HELPER( 0x0800, "(MF2)Cursor Down",		KEYCODE_DOWN        ) /* Down                        6c  ec */
	AT_KEYB_HELPER( 0x1000, "(MF2)Page Down",		KEYCODE_PGDN        ) /* Page Down                   6d  ed */
	AT_KEYB_HELPER( 0x4000, "Del",      		    KEYCODE_A           ) /* Delete                      6f  ef */

	PORT_START("pc_keyboard_7")
INPUT_PORTS_END

static void pcat_dyn_set_keyb_int(running_machine &machine, int state)
{
	pic8259_ir1_w(machine.device("pic8259_1"), state);
}

static READ8_HANDLER( vga_setting ) { return 0xff; } // hard-code to color

static const struct pc_vga_interface vga_interface =
{
	NULL,
	NULL,
	vga_setting,
	AS_PROGRAM,
	0xa0000,
	AS_IO,
	0x0000
};

static void set_gate_a20(running_machine &machine, int a20)
{
	cputag_set_input_line(machine, "maincpu", INPUT_LINE_A20, a20);
}

static void keyboard_interrupt(running_machine &machine, int state)
{
	pic8259_ir1_w(machine.device("pic8259_1"), state);
}

static int pcat_dyn_get_out2(running_machine &machine) {
	return pit8253_get_output(machine.device("pit8254"), 2 );
}


static const struct kbdc8042_interface at8042 =
{
	KBDC8042_AT386, set_gate_a20, keyboard_interrupt, pcat_dyn_get_out2
};

static MACHINE_START( pcat_dyn )
{

	device_set_irq_callback(machine.device("maincpu"), pcat_irq_callback);
	init_pc_common(machine, PCCOMMON_KEYBOARD_AT, pcat_dyn_set_keyb_int);
	kbdc8042_init(machine, &at8042);
}

static MACHINE_CONFIG_START( pcat_dyn, pcat_dyn_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I486, 40000000)	/* Am486 DX-40 */
	MCFG_CPU_PROGRAM_MAP(pcat_map)
	MCFG_CPU_IO_MAP(pcat_io)

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_vga )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_MACHINE_START(pcat_dyn)
	MCFG_MC146818_ADD( "rtc", MC146818_STANDARD )

	MCFG_FRAGMENT_ADD( pcat_common )
MACHINE_CONFIG_END

/***************************************
*
* ROM definitions
*
***************************************/

ROM_START(toursol)
	ROM_REGION32_LE(0x10000, "bios", 0)	/* Motherboard BIOS */
	ROM_LOAD("prom.mb", 0x000000, 0x10000, CRC(e44bfd3c) SHA1(c07ec94e11efa30e001f39560010112f73cc0016) )

	ROM_REGION(0x20000, "video_bios", 0)	/* Trident TVGA9000 BIOS */
	ROM_LOAD16_BYTE("prom.vid", 0x00000, 0x04000, CRC(ad7eadaf) SHA1(ab379187914a832284944e81e7652046c7d938cc) )
	ROM_CONTINUE(				0x00001, 0x04000 )

	ROM_REGION32_LE(0x100000, "game_prg", 0)	/* PromStor 32, mapping unknown */
	ROM_LOAD("sol.u21", 0x00000, 0x40000, CRC(e97724d9) SHA1(995b89d129c371b815c6b498093bd1bbf9fd8755))
	ROM_LOAD("sol.u22", 0x40000, 0x40000, CRC(69d42f50) SHA1(737fe62f3827b00b4f6f3b72ef6c7b6740947e95))
	ROM_LOAD("sol.u23", 0x80000, 0x40000, CRC(d1e39bd4) SHA1(39c7ee43cddb53fba0f7c0572ddc40289c4edd07))
	ROM_LOAD("sol.u24", 0xa0000, 0x40000, CRC(555341e0) SHA1(81fee576728855e234ff7aae06f54ae9705c3ab5))
	ROM_LOAD("sol.u28", 0xe0000, 0x02000, CRC(c9374d50) SHA1(49173bc69f70bb2a7e8af9d03e2538b34aa881d8))
ROM_END


ROM_START(toursol1)
	ROM_REGION32_LE(0x10000, "bios", 0)	/* Motherboard BIOS */
	ROM_LOAD("prom.mb", 0x000000, 0x10000, CRC(e44bfd3c) SHA1(c07ec94e11efa30e001f39560010112f73cc0016) )

	ROM_REGION(0x20000, "video_bios", 0)	/* Trident TVGA9000 BIOS */
	ROM_LOAD16_BYTE("prom.vid", 0x00000, 0x04000, CRC(ad7eadaf) SHA1(ab379187914a832284944e81e7652046c7d938cc) )
	ROM_CONTINUE(				0x00001, 0x04000 )

	ROM_REGION32_LE(0x100000, "game_prg", 0)	/* PromStor 32, mapping unknown */
	ROM_LOAD("prom.0", 0x00000, 0x40000, CRC(f26ce73f) SHA1(5516c31aa18716a47f46e412fc273ae8784d2061))
	ROM_LOAD("prom.1", 0x40000, 0x40000, CRC(8f96e2a8) SHA1(bc3ce8b99e6ff40e355df2c3f797f1fe88b3b219))
	ROM_LOAD("prom.2", 0x80000, 0x40000, CRC(8b0ac5cf) SHA1(1c2b6a53c9ff4d18a5227d899facbbc719f40205))
	ROM_LOAD("prom.3", 0xa0000, 0x40000, CRC(9352e965) SHA1(2bfb647ec27c60a8c821fdf7483199e1a444cea8))
	ROM_LOAD("prom.7", 0xe0000, 0x02000, CRC(154c8092) SHA1(4439ee82f36d5d5c334494ba7bb4848e839213a7))
ROM_END

static DRIVER_INIT(pcat_dyn)
{
	pc_vga_init(machine, &vga_interface, NULL);
}

GAME( 1995, toursol,  0,       pcat_dyn, pcat_dyn, pcat_dyn, ROT0, "Dynamo", "Tournament Solitaire (V1.06, 08/03/95)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1995, toursol1, toursol, pcat_dyn, pcat_dyn, pcat_dyn, ROT0, "Dynamo", "Tournament Solitaire (V1.04, 06/22/95)", GAME_NOT_WORKING|GAME_NO_SOUND )

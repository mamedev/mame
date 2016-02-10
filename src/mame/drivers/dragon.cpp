// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    dragon.c

    Dragon

***************************************************************************/

#include "includes/dragon.h"
#include "includes/dgnalpha.h"
#include "imagedev/cassette.h"
#include "formats/coco_cas.h"
#include "cpu/m6809/m6809.h"
#include "bus/coco/coco_232.h"
#include "bus/coco/coco_orch90.h"
#include "bus/coco/coco_pak.h"
#include "bus/coco/coco_fdc.h"
#include "bus/coco/coco_multi.h"
#include "formats/vdk_dsk.h"
#include "formats/dmk_dsk.h"
#include "imagedev/flopdrv.h"


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( dragon_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( dragon_mem, AS_PROGRAM, 8, dragon_state )
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

/* Dragon keyboard

       PB0 PB1 PB2 PB3 PB4 PB5 PB6 PB7
  PA6: Ent Clr Brk N/c N/c N/c N/c Shift
  PA5: X   Y   Z   Up  Dwn Lft Rgt Space
  PA4: P   Q   R   S   T   U   V   W
  PA3: H   I   J   K   L   M   N   O
  PA2: @   A   B   C   D   E   F   G
  PA1: 8   9   :   ;   ,   -   .   /
  PA0: 0   1   2   3   4   5   6   7
 */
static INPUT_PORTS_START( dragon32 )
	PORT_START("row0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("row1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_COLON) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("row2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_G) PORT_CHAR('G')

	PORT_START("row3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("row4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("row5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(10)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(9)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("row6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_HOME) PORT_CHAR(12)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_NAME("BREAK") PORT_CODE(KEYCODE_END) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x78, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, NULL) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
INPUT_PORTS_END

static SLOT_INTERFACE_START(dragon_cart)
	SLOT_INTERFACE("dragon_fdc", DRAGON_FDC)
	SLOT_INTERFACE("sdtandy_fdc", SDTANDY_FDC)
	SLOT_INTERFACE("pak", COCO_PAK)
SLOT_INTERFACE_END

FLOPPY_FORMATS_MEMBER( dragon_alpha_state::dragon_formats )
	FLOPPY_VDK_FORMAT,
	FLOPPY_DMK_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( dragon_floppies )
	SLOT_INTERFACE("sssd", FLOPPY_525_SSSD)
	SLOT_INTERFACE("sd",   FLOPPY_525_SD)
	SLOT_INTERFACE("ssdd", FLOPPY_525_SSDD)
	SLOT_INTERFACE("dd",   FLOPPY_525_DD)
	SLOT_INTERFACE("ssqd", FLOPPY_525_SSQD)
	SLOT_INTERFACE("qd",   FLOPPY_525_QD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( dragon_base, dragon_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", M6809E, XTAL_4_433619MHz)
	MCFG_CPU_PROGRAM_MAP(dragon_mem)

	// devices
	MCFG_DEVICE_ADD(PIA0_TAG, PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(coco_state, pia0_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(coco_state, pia0_pb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(coco_state, pia0_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(coco_state, pia0_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(coco_state, pia0_irq_a))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(coco_state, pia0_irq_b))

	MCFG_DEVICE_ADD(PIA1_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(coco_state, pia1_pa_r))
	MCFG_PIA_READPB_HANDLER(READ8(coco_state, pia1_pb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(coco_state, pia1_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(coco_state, pia1_pb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(coco_state, pia1_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(coco_state, pia1_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(coco_state, pia1_firq_a))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(coco_state, pia1_firq_b))

	MCFG_SAM6883_ADD(SAM_TAG, XTAL_4_433619MHz, MAINCPU_TAG, AS_PROGRAM)
	MCFG_SAM6883_RES_CALLBACK(READ8(dragon_state, sam_read))
	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(coco_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED)

	MCFG_DEVICE_ADD(PRINTER_TAG, PRINTER, 0)

	// video hardware
	MCFG_SCREEN_MC6847_PAL_ADD(SCREEN_TAG, VDG_TAG)

	MCFG_DEVICE_ADD(VDG_TAG, MC6847_PAL, XTAL_4_433619MHz)
	MCFG_MC6847_HSYNC_CALLBACK(WRITELINE(dragon_state, horizontal_sync))
	MCFG_MC6847_FSYNC_CALLBACK(WRITELINE(dragon_state, field_sync))
	MCFG_MC6847_INPUT_CALLBACK(DEVREAD8(SAM_TAG, sam6883_device, display_read))

	// sound hardware
	MCFG_FRAGMENT_ADD( coco_sound )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dragon32, dragon_base )
	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_EXTRA_OPTIONS("64K")

	// cartridge
	MCFG_COCO_CARTRIDGE_ADD(CARTRIDGE_TAG, dragon_cart, "dragon_fdc")
	MCFG_COCO_CARTRIDGE_CART_CB(WRITELINE(coco_state, cart_w))
	MCFG_COCO_CARTRIDGE_NMI_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_NMI))
	MCFG_COCO_CARTRIDGE_HALT_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_HALT))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( dragon64, dragon_base, dragon64_state )
	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")

	// cartridge
	MCFG_COCO_CARTRIDGE_ADD(CARTRIDGE_TAG, dragon_cart, "dragon_fdc")
	MCFG_COCO_CARTRIDGE_CART_CB(WRITELINE(coco_state, cart_w))
	MCFG_COCO_CARTRIDGE_NMI_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_NMI))
	MCFG_COCO_CARTRIDGE_HALT_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_HALT))

	// acia
	MCFG_DEVICE_ADD("acia", MOS6551, 0)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( d64plus, dragon_base, dragon64_state )
	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")

	// cartridge
	MCFG_COCO_CARTRIDGE_ADD(CARTRIDGE_TAG, dragon_cart, "dragon_fdc")
	MCFG_COCO_CARTRIDGE_CART_CB(WRITELINE(coco_state, cart_w))
	MCFG_COCO_CARTRIDGE_NMI_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_NMI))
	MCFG_COCO_CARTRIDGE_HALT_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_HALT))

	// acia
	MCFG_DEVICE_ADD("acia", MOS6551, 0)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( dgnalpha, dragon_base, dragon_alpha_state )
	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")

	// cartridge
	MCFG_COCO_CARTRIDGE_ADD(CARTRIDGE_TAG, dragon_cart, nullptr)
	MCFG_COCO_CARTRIDGE_CART_CB(WRITELINE(coco_state, cart_w))
	MCFG_COCO_CARTRIDGE_NMI_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_NMI))
	MCFG_COCO_CARTRIDGE_HALT_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_HALT))

	// acia
	MCFG_DEVICE_ADD("acia", MOS6551, 0)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)

	// floppy
	MCFG_WD2797_ADD(WD2797_TAG, XTAL_1MHz)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(dragon_alpha_state, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(dragon_alpha_state, fdc_drq_w))

	MCFG_FLOPPY_DRIVE_ADD(WD2797_TAG ":0", dragon_floppies, "qd", dragon_alpha_state::dragon_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD2797_TAG ":1", dragon_floppies, "qd", dragon_alpha_state::dragon_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD2797_TAG ":2", dragon_floppies, "qd", dragon_alpha_state::dragon_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD2797_TAG ":3", dragon_floppies, "qd", dragon_alpha_state::dragon_formats)

	// sound hardware
	MCFG_SOUND_ADD(AY8912_TAG, AY8912, 1000000)
	MCFG_AY8910_PORT_A_READ_CB(READ8(dragon_alpha_state, psg_porta_read))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(dragon_alpha_state, psg_porta_write))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)

	// pia 2
	MCFG_DEVICE_ADD( PIA2_TAG, PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(dragon_alpha_state, pia2_pa_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(dragon_alpha_state, pia2_firq_a))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(dragon_alpha_state, pia2_firq_b))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tanodr64, dragon64 )
	// cartridge
	MCFG_DEVICE_MODIFY(CARTRIDGE_TAG)
	MCFG_DEVICE_SLOT_INTERFACE(dragon_cart, "sdtandy_fdc", false)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(dragon32)
	ROM_REGION(0xC000, "maincpu",0)
	ROM_LOAD("d32.rom",      0x0000,  0x4000, CRC(e3879310) SHA1(f2dab125673e653995a83bf6b793e3390ec7f65a))
ROM_END

ROM_START(dragon64)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("d64_1.rom",    0x0000,  0x4000, CRC(60a4634c) SHA1(f119506eaa3b4b70b9aa0dd83761e8cbe043d042))
	ROM_LOAD("d64_2.rom",    0x8000,  0x4000, CRC(17893a42) SHA1(e3c8986bb1d44269c4587b04f1ca27a70b0aaa2e))
ROM_END

ROM_START(dragon200)
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ic18.rom",    0x0000, 0x4000, CRC(84f68bf9) SHA1(1983b4fb398e3dd9668d424c666c5a0b3f1e2b69))
	ROM_LOAD( "ic17.rom",    0x8000, 0x4000, CRC(17893a42) SHA1(e3c8986bb1d44269c4587b04f1ca27a70b0aaa2e))
ROM_END

ROM_START(dragon200e)
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ic18_v1.4e.ic34",    0x0000, 0x4000, CRC(95af0a0a) SHA1(628543ee8b47a56df2b2175cfb763c0051517b90))
	ROM_LOAD( "ic17_v1.4e.ic37",    0x8000, 0x4000, CRC(48b985df) SHA1(c25632f3c2cfd1af3ee26b2f233a1ce1eccc365d))
	ROM_REGION( 0x1000, "gfx", ROMREGION_ERASEFF )
	ROM_LOAD( "rom26.ic1",          0x0000, 0x1000, CRC(565724bc) SHA1(da5b756ba2a9c9ecebaa7daa8ba8bfd984d56a6f))
ROM_END

ROM_START(d64plus)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("d64_1.rom",    0x0000,  0x4000, CRC(60a4634c) SHA1(f119506eaa3b4b70b9aa0dd83761e8cbe043d042))
	ROM_LOAD("d64_2.rom",    0x8000,  0x4000, CRC(17893a42) SHA1(e3c8986bb1d44269c4587b04f1ca27a70b0aaa2e))
ROM_END

ROM_START(tanodr64)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("d64_1.rom",    0x0000,  0x4000, CRC(60a4634c) SHA1(f119506eaa3b4b70b9aa0dd83761e8cbe043d042))
	ROM_LOAD("d64_2.rom",    0x8000,  0x4000, CRC(17893a42) SHA1(e3c8986bb1d44269c4587b04f1ca27a70b0aaa2e))
ROM_END

ROM_START(dgnalpha)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("alpha_bt.rom",    0x2000,  0x2000, CRC(c3dab585) SHA1(4a5851aa66eb426e9bb0bba196f1e02d48156068))
	ROM_LOAD("alpha_ba.rom",    0x8000,  0x4000, CRC(84f68bf9) SHA1(1983b4fb398e3dd9668d424c666c5a0b3f1e2b69))
ROM_END


COMP(  1982,    dragon32,   coco,   0,      dragon32,  dragon32, driver_device,  0,      "Dragon Data Ltd",            "Dragon 32", 0)
COMP(  1983,    dragon64,   coco,   0,      dragon64,  dragon32, driver_device,  0,      "Dragon Data Ltd",            "Dragon 64", 0)
COMP(  1983,    dragon200,  coco,   0,      dragon64,  dragon32, driver_device,  0,      "Dragon Data Ltd",            "Dragon 200", 0)
COMP(  1983,    dragon200e, coco,   0,      dragon64,  dragon32, driver_device,  0,      "Dragon Data Ltd",            "Dragon 200-E", 0)
COMP(  1983,    d64plus,    coco,   0,      d64plus,   dragon32, driver_device,  0,      "Dragon Data Ltd",            "Dragon 64 Plus", 0)
COMP(  1983,    tanodr64,   coco,   0,      tanodr64,  dragon32, driver_device,  0,      "Dragon Data Ltd / Tano Ltd", "Tano Dragon 64 (NTSC)", 0)
COMP(  1984,    dgnalpha,   coco,   0,      dgnalpha,  dragon32, driver_device,  0,      "Dragon Data Ltd",            "Dragon Alpha Prototype", 0)

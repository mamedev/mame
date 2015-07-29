// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/*****************************************************************

GCE Vectrex

Mathis Rosenhauer
Christopher Salomon (technical advice)
Bruce Tomlin (hardware info)

*****************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/vector.h"
#include "machine/6522via.h"
#include "includes/vectrex.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "machine/nvram.h"


static ADDRESS_MAP_START(vectrex_map, AS_PROGRAM, 8, vectrex_state )
	AM_RANGE(0x0000, 0x7fff) AM_NOP // cart area, handled at machine_start
	AM_RANGE(0xc800, 0xcbff) AM_RAM AM_MIRROR(0x0400) AM_SHARE("gce_vectorram")
	AM_RANGE(0xd000, 0xd7ff) AM_READWRITE(vectrex_via_r, vectrex_via_w)
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START(vectrex)
	PORT_START("CONTR1X")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_MINMAX(0,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(30)

	PORT_START("CONTR1Y")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_MINMAX(0,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_REVERSE

	PORT_START("CONTR2X")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_MINMAX(0,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_PLAYER(2)

	PORT_START("CONTR2Y")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_MINMAX(0,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(2)

	PORT_START("3DCONF")
	PORT_CONFNAME(0x01, 0x00, "3D Imager")
	PORT_CONFSETTING(0x00, DEF_STR(Off))
	PORT_CONFSETTING(0x01, DEF_STR(On))
	PORT_CONFNAME(0x02, 0x00, "Separate images")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x02, DEF_STR(Yes))
	PORT_CONFNAME(0x1c, 0x10, "Left eye")
	PORT_CONFSETTING(0x00, "Black")
	PORT_CONFSETTING(0x04, "Red")
	PORT_CONFSETTING(0x08, "Green")
	PORT_CONFSETTING(0x0c, "Blue")
	PORT_CONFSETTING(0x10, "Color")
	PORT_CONFNAME(0xe0, 0x80, "Right eye")
	PORT_CONFSETTING(0x00, "Black")
	PORT_CONFSETTING(0x20, "Red")
	PORT_CONFSETTING(0x40, "Green")
	PORT_CONFSETTING(0x60, "Blue")
	PORT_CONFSETTING(0x80, "Color")

	PORT_START("LPENCONF")
	PORT_CONFNAME(0x03, 0x00, "Lightpen")
	PORT_CONFSETTING(0x00, DEF_STR(Off))
	PORT_CONFSETTING(0x01, "left port")
	PORT_CONFSETTING(0x02, "right port")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_CODE(MOUSECODE_BUTTON1)

	PORT_START("LPENY")
	PORT_BIT(0xff, 0x80, IPT_LIGHTGUN_X)  PORT_CROSSHAIR(Y, 1, 0, 0) PORT_MINMAX(0,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(1) PORT_PLAYER(1)

	PORT_START("LPENX")
	PORT_BIT(0xff, 0x80, IPT_LIGHTGUN_Y)  PORT_CROSSHAIR(X, 1, 0, 0) PORT_MINMAX(0,0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(1) PORT_REVERSE PORT_PLAYER(1)

INPUT_PORTS_END

static SLOT_INTERFACE_START(vectrex_cart)
	SLOT_INTERFACE_INTERNAL("vec_rom",    VECTREX_ROM_STD)
	SLOT_INTERFACE_INTERNAL("vec_rom64k", VECTREX_ROM_64K)
	SLOT_INTERFACE_INTERNAL("vec_sram",   VECTREX_ROM_SRAM)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( vectrex, vectrex_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, XTAL_6MHz / 4)
	MCFG_CPU_PROGRAM_MAP(vectrex_map)

	/* video hardware */
	MCFG_VECTOR_ADD("vector")
	MCFG_SCREEN_ADD("screen", VECTOR)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(400, 300)
	MCFG_SCREEN_VISIBLE_AREA(0, 399, 0, 299)
	MCFG_SCREEN_UPDATE_DRIVER(vectrex_state, screen_update_vectrex)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_ADD("ay8912", AY8912, 1500000)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("BUTTONS"))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(vectrex_state, vectrex_psg_port_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	/* via */
	MCFG_DEVICE_ADD("via6522_0", VIA6522, 0)
	MCFG_VIA6522_READPA_HANDLER(READ8(vectrex_state, vectrex_via_pa_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(vectrex_state, vectrex_via_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(vectrex_state, v_via_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(vectrex_state, v_via_pb_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(vectrex_state, v_via_ca2_w))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(vectrex_state, v_via_cb2_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(vectrex_state, vectrex_via_irq))

	/* cartridge */
	MCFG_VECTREX_CARTRIDGE_ADD("cartslot", vectrex_cart, NULL)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","vectrex")
MACHINE_CONFIG_END

ROM_START(vectrex)
	ROM_REGION(0x2000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "bios0", "exec rom")
	ROMX_LOAD("exec_rom.bin", 0x0000, 0x2000, CRC(ba13fb57) SHA1(65d07426b520ddd3115d40f255511e0fd2e20ae7), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "bios1", "exec rom intl 284001-1")
	ROMX_LOAD("exec_rom_intl_284001-1.bin", 0x0000, 0x2000, CRC(6d2bd167) SHA1(77a220d5d98846b606dff608f7b5d00183ec3bab), ROM_BIOS(2) )

//  The following fastboots are listed here for reference and documentation
//  ROM_SYSTEM_BIOS(2, "bios2", "us-fastboot hack")
//  ROMX_LOAD("us-fastboot.bin", 0x0000, 0x2000, CRa6e4dac4) SHA1(e0900be6d6858b985fd7f0999d864b2fceaf01a1), ROM_BIOS(3) )
//  ROM_SYSTEM_BIOS(3, "bios3", "intl-fastboot hack")
//  ROMX_LOAD("intl-fastboot.bin", 0x0000, 0x2000, CRC(71dcf0f4) SHA1(2a257c5111f5cee841bd14acaa9df6496aaf3d8b), ROM_BIOS(4) )

ROM_END


/*****************************************************************

  RA+A Spectrum I+

  The Spectrum I+ was a modified Vectrex. It had a 32K ROM cart
  and 2K additional battery backed RAM (0x8000 - 0x87ff). PB6
  was used to signal inserted coins to the VIA. The unit was
  controlled by 8 buttons (2x4 buttons of controller 1 and 2).
  Each button had a LED which were mapped to 0xa000.
  The srvice mode can be accessed by pressing button
  8 during startup. As soon as all LEDs light up,
  press 2 and 3 without releasing 8. Then release 8 and
  after that 2 and 3. You can leave the screen where you enter
  ads by pressing 8 several times.

  Character matrix is:

  btn| 1  2  3  4  5  6  7  8
  ---+------------------------
  1  | 0  1  2  3  4  5  6  7
  2  | 8  9  A  B  C  D  E  F
  3  | G  H  I  J  K  L  M  N
  4  | O  P  Q  R  S  T  U  V
  5  | W  X  Y  Z  sp !  "  #
  6  | $  %  &  '  (  )  *  +
  7  | ,  -  _  /  :  ;  ?  =
  8  |bs ret up dn l  r hom esc

  The first page of ads is shown with the "result" of the
  test. Remaining pages are shown in attract mode. If no extra
  ram is present, the word COLOR is scrolled in big vector!
  letters in attract mode.

*****************************************************************/

static ADDRESS_MAP_START(raaspec_map , AS_PROGRAM, 8, vectrex_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xa000, 0xa000) AM_WRITE(raaspec_led_w)
	AM_RANGE(0xc800, 0xcbff) AM_RAM AM_MIRROR(0x0400) AM_SHARE("gce_vectorram")
	AM_RANGE(0xd000, 0xd7ff) AM_READWRITE(vectrex_via_r, vectrex_via_w)
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START(raaspec)
	PORT_START("LPENCONF")
	PORT_START("LPENY")
	PORT_START("LPENX")
	PORT_START("3DCONF")
	PORT_START("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON8)
	PORT_START("COIN")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN1)
INPUT_PORTS_END


static MACHINE_CONFIG_DERIVED( raaspec, vectrex )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(raaspec_map)
	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_VIDEO_START_OVERRIDE(vectrex_state,raaspec)

	/* via */
	MCFG_DEVICE_MODIFY("via6522_0")
	MCFG_VIA6522_READPB_HANDLER(READ8(vectrex_state, vectrex_s1_via_pb_r))

	MCFG_DEVICE_REMOVE("cartslot")
MACHINE_CONFIG_END

ROM_START(raaspec)
	ROM_REGION(0x10000,"maincpu", 0)
	ROM_LOAD("spectrum.bin", 0x0000, 0x8000, CRC(20af7f3f) SHA1(7ce85db8dd32687ad7629631ae113820371faf7c))
	ROM_LOAD("exec_rom.bin", 0xe000, 0x2000, CRC(ba13fb57) SHA1(65d07426b520ddd3115d40f255511e0fd2e20ae7))
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     INIT       COMPANY FULLNAME */
CONS(1982, vectrex,  0,        0,      vectrex,  vectrex,  vectrex_state, vectrex,    "General Consumer Electronics",   "Vectrex" , ROT270)

GAME(1984, raaspec,  0,        raaspec,  raaspec, vectrex_state,  vectrex, ROT270,    "Roy Abel & Associates",   "Spectrum I+", MACHINE_NOT_WORKING ) //TODO: button labels & timings, a mandatory artwork too?

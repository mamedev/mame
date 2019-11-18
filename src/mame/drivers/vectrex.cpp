// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/*****************************************************************

GCE Vectrex

Mathis Rosenhauer
Christopher Salomon (technical advice)
Bruce Tomlin (hardware info)

*****************************************************************/

#include "emu.h"
#include "includes/vectrex.h"

#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/volt_reg.h"
#include "video/vector.h"

#include "softlist.h"
#include "speaker.h"


void vectrex_state::vectrex_map(address_map &map)
{
	map(0x0000, 0x7fff).noprw(); // cart area, handled at machine_start
	map(0xc800, 0xcbff).ram().mirror(0x0400).share("gce_vectorram");
	map(0xd000, 0xd7ff).rw(FUNC(vectrex_state::vectrex_via_r), FUNC(vectrex_state::vectrex_via_w));
	map(0xe000, 0xffff).rom().region("maincpu", 0);
}

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

void vectrex_base_state::vectrex_cart(device_slot_interface &device)
{
	device.option_add_internal("vec_rom",    VECTREX_ROM_STD);
	device.option_add_internal("vec_rom64k", VECTREX_ROM_64K);
	device.option_add_internal("vec_sram",   VECTREX_ROM_SRAM);
}

void vectrex_base_state::vectrex_base(machine_config &config)
{
	MC6809(config, m_maincpu, 6_MHz_XTAL); // 68A09

	/* video hardware */
	VECTOR(config, m_vector, 0);
	SCREEN(config, m_screen, SCREEN_TYPE_VECTOR);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(400, 300);
	m_screen->set_visarea(0, 399, 0, 299);
	m_screen->set_screen_update(FUNC(vectrex_base_state::screen_update_vectrex));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	MC1408(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // mc1408.ic301 (also used for vector generation)
	voltage_regulator_device &vreg(VOLTAGE_REGULATOR(config, "vref", 0));
	vreg.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vreg.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);

	AY8912(config, m_ay8912, 6_MHz_XTAL / 4);
	m_ay8912->port_a_read_callback().set_ioport("BUTTONS");
	m_ay8912->port_a_write_callback().set(FUNC(vectrex_base_state::vectrex_psg_port_w));
	m_ay8912->add_route(ALL_OUTPUTS, "speaker", 0.2);

	/* via */
	VIA6522(config, m_via6522_0, 6_MHz_XTAL / 4);
	m_via6522_0->readpa_handler().set(FUNC(vectrex_base_state::vectrex_via_pa_r));
	m_via6522_0->readpb_handler().set(FUNC(vectrex_base_state::vectrex_via_pb_r));
	m_via6522_0->writepa_handler().set(FUNC(vectrex_base_state::v_via_pa_w));
	m_via6522_0->writepb_handler().set(FUNC(vectrex_base_state::v_via_pb_w));
	m_via6522_0->ca2_handler().set(FUNC(vectrex_base_state::v_via_ca2_w));
	m_via6522_0->cb2_handler().set(FUNC(vectrex_base_state::v_via_cb2_w));
	m_via6522_0->irq_handler().set(FUNC(vectrex_base_state::vectrex_via_irq));
}

void vectrex_state::vectrex(machine_config &config)
{
	vectrex_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &vectrex_state::vectrex_map);

	vectrex_cart_slot_device &slot(VECTREX_CART_SLOT(config, "cartslot", 0));
	vectrex_cart(slot);

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_type("vectrex", SOFTWARE_LIST_ORIGINAL_SYSTEM);
}

ROM_START(vectrex)
	ROM_REGION(0x2000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "bios0", "exec rom")
	ROMX_LOAD("exec_rom.bin", 0x0000, 0x2000, CRC(ba13fb57) SHA1(65d07426b520ddd3115d40f255511e0fd2e20ae7), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "bios1", "exec rom intl 284001-1")
	ROMX_LOAD("exec_rom_intl_284001-1.bin", 0x0000, 0x2000, CRC(6d2bd167) SHA1(77a220d5d98846b606dff608f7b5d00183ec3bab), ROM_BIOS(1) )

//  The following fastboots are listed here for reference and documentation
//  ROM_SYSTEM_BIOS(2, "bios2", "us-fastboot hack")
//  ROMX_LOAD("us-fastboot.bin", 0x0000, 0x2000, CRa6e4dac4) SHA1(e0900be6d6858b985fd7f0999d864b2fceaf01a1), ROM_BIOS(2) )
//  ROM_SYSTEM_BIOS(3, "bios3", "intl-fastboot hack")
//  ROMX_LOAD("intl-fastboot.bin", 0x0000, 0x2000, CRC(71dcf0f4) SHA1(2a257c5111f5cee841bd14acaa9df6496aaf3d8b), ROM_BIOS(3) )

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

void raaspec_state::raaspec_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0xa000, 0xa000).w(FUNC(raaspec_state::raaspec_led_w));
	map(0xc800, 0xcbff).ram().mirror(0x0400).share("gce_vectorram");
	map(0xd000, 0xd7ff).rw(FUNC(raaspec_state::vectrex_via_r), FUNC(raaspec_state::vectrex_via_w));
	map(0xe000, 0xffff).rom();
}

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


void raaspec_state::raaspec(machine_config &config)
{
	vectrex_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &raaspec_state::raaspec_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	m_via6522_0->readpb_handler().set(FUNC(raaspec_state::vectrex_s1_via_pb_r));
}

ROM_START(raaspec)
	ROM_REGION(0x10000,"maincpu", 0)
	ROM_LOAD("spectrum.bin", 0x0000, 0x8000, CRC(20af7f3f) SHA1(7ce85db8dd32687ad7629631ae113820371faf7c))
	ROM_LOAD("exec_rom.bin", 0xe000, 0x2000, CRC(ba13fb57) SHA1(65d07426b520ddd3115d40f255511e0fd2e20ae7))
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

//   YEAR  NAME       PARENT    COMPAT   MACHINE   INPUT     STATE          INIT        MONITOR  COMPANY                         FULLNAME
CONS( 1982, vectrex,  0,        0,       vectrex,  vectrex,  vectrex_state, empty_init,          "General Consumer Electronics", "Vectrex" , ROT270)

GAME( 1984, raaspec,  0,                 raaspec,  raaspec,  raaspec_state, empty_init, ROT270,  "Roy Abel & Associates",        "Spectrum I+", MACHINE_NOT_WORKING ) //TODO: button labels & timings, a mandatory artwork too?

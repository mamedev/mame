// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

UKNC (Educational Computer by Scientific Centre) PDP-11 clone.
Also known as Elektronika MS-0511.
RAM = 192K (CPU 1 = 64K, CPU 2 = 32K, Videoram = 96K), ROM = 32K.
Graphics 640x288 pixels.

2009-05-12 Skeleton driver.

Status: both CPUs start in the weeds.

****************************************************************************/

#include "emu.h"
#include "bus/qbus/qbus.h"
#include "cpu/t11/t11.h"
#include "imagedev/cassette.h"

#include "emupal.h"
#include "screen.h"


namespace {

class uknc_state : public driver_device
{
public:
	uknc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_qbus(*this, "qbus")
		, m_cart(*this, "cart")
		, m_cassette(*this, "cassette")
	{ }

	void uknc(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<k1801vm2_device> m_maincpu;
	required_device<k1801vm2_device> m_subcpu;
	required_device<qbus_device> m_qbus;
	required_device<qbus_device> m_cart;
	required_device<cassette_image_device> m_cassette;

	void uknc_mem(address_map &map) ATTR_COLD;
	void uknc_sub_mem(address_map &map) ATTR_COLD;
};


void uknc_state::uknc_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).ram();
}

void uknc_state::uknc_sub_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).ram();
	map(0x8000, 0xffff).rom().region("subcpu",0);
}

/* Input ports */
static INPUT_PORTS_START( uknc )
INPUT_PORTS_END


void uknc_state::machine_reset()
{
}

void uknc_state::machine_start()
{
}

uint32_t uknc_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void uknc_state::uknc(machine_config &config)
{
	/* basic machine hardware */
	K1801VM2(config, m_maincpu, XTAL(16'000'000)/4); // external clock is /2 + internal divider /2
	m_maincpu->set_initial_mode(0x8000);
	m_maincpu->set_addrmap(AS_PROGRAM, &uknc_state::uknc_mem);
	m_maincpu->set_disable();

	QBUS(config, m_qbus, 0);
	m_qbus->set_space(m_maincpu, AS_PROGRAM);
	m_qbus->birq4().set_inputline(m_maincpu, t11_device::VEC_LINE);
	QBUS_SLOT(config, "qbus" ":1", qbus_cards, nullptr);

	K1801VM2(config, m_subcpu, XTAL(12'500'000)/4);
	m_subcpu->set_addrmap(AS_PROGRAM, &uknc_state::uknc_sub_mem);
	m_subcpu->set_initial_mode(0x8000);

	QBUS(config, m_cart, 0);
	m_cart->set_space(m_subcpu, AS_PROGRAM);
	m_cart->birq4().set_inputline(m_subcpu, t11_device::VEC_LINE);
	QBUS_SLOT(config, "cart" ":1", qbus_cards, "mz");
	QBUS_SLOT(config, "cart" ":2", qbus_cards, nullptr);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(uknc_state::screen_update));

	PALETTE(config, "palette", palette_device::MONOCHROME);
}

/* ROM definition */
ROM_START( uknc )
	ROM_REGION16_LE(0100000, "subcpu", ROMREGION_ERASE00)
	ROM_LOAD("uknc.rom", 0x0000, 0x8000, CRC(a1536994) SHA1(b3c7c678c41ffa9b37f654fbf20fef7d19e6407b))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  STATE       INIT        COMPANY             FULLNAME  FLAGS */
COMP( 1987, uknc, 0,      0,      uknc,    uknc,  uknc_state, empty_init, "Elektronika", "UKNC / MS-0511",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

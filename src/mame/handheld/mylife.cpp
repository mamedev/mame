// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "cpu/edsp/edsp.h"

#include "machine/i2cmem.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class mylife_state : public driver_device
{
public:
	mylife_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_screen(*this, "screen")
		, m_seeprom(*this, "seeprom")
		, m_cart(*this, "cartslot")
	{ }

	void mylife(machine_config &config) ATTR_COLD;

protected:

private:
	required_device<screen_device> m_screen;
	required_device<i2cmem_device> m_seeprom;
	required_device<generic_slot_device> m_cart;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u16 pa_r();
	void pa_w(u16 data);

	void prog_map(address_map &map) ATTR_COLD;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load) ATTR_COLD;
};

uint32_t mylife_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

u16 mylife_state::pa_r()
{
	return m_seeprom->read_sda() << 9;
}

void mylife_state::pa_w(u16 data)
{
	m_seeprom->write_scl(BIT(data, 8));
	m_seeprom->write_sda(BIT(data, 9));
}

void mylife_state::prog_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
}

static INPUT_PORTS_START( mylife )
INPUT_PORTS_END

void mylife_state::mylife(machine_config &config)
{
	emg2000a_device &maincpu(EMG2000A(config, "maincpu", 27_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &mylife_state::prog_map);
	maincpu.in_pa_callback().set(FUNC(mylife_state::pa_r));
	maincpu.out_pa_callback().set(FUNC(mylife_state::pa_w));

	I2C_24C02(config, m_seeprom);

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(256, 256); // unknown resolution
	m_screen->set_visarea(0, 256-1, 0, 256-1);
	m_screen->set_screen_update(FUNC(mylife_state::screen_update));

	SPEAKER(config, "mono").front_center();

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "mylife_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(mylife_state::cart_load));
	SOFTWARE_LIST(config, "cart_list").set_original("mylife_cart");
}

DEVICE_IMAGE_LOAD_MEMBER(mylife_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

ROM_START( mylife )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF ) // no TSOP pads
	ROM_LOAD( "mylife_ml-01.u1", 0x000000, 0x800000, CRC(48f98fd2) SHA1(15911b30a0702f28d016b6452303e3c5ee9e14ab) )

	ROM_REGION( 0x100, "seeprom", ROMREGION_ERASEFF ) // probably just settings
	ROM_LOAD( "af24bc02.u8", 0x000, 0x100, CRC(bec6d965) SHA1(80a7b385735a5d31388798fe301e6e019c93f84a) )
ROM_END

ROM_START( mylifei )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF ) // dumped via TSOP pads
	ROM_LOAD( "mylife36.u7", 0x000000, 0x800000, CRC(bfc9b801) SHA1(97612a8c3eb9e76190f59363efa59e5cd72d2be6) )

	ROM_REGION( 0x100, "seeprom", ROMREGION_ERASEFF ) // probably just settings
	ROM_LOAD( "af24bc02.u8", 0x000, 0x100, CRC(17c6eb00) SHA1(2b30cb3a924f4905f98cc8220edaee156eaf59ae) )
ROM_END

ROM_START( mylifes )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF ) // no TSOP pads
	ROM_LOAD( "mylife_spain.bin", 0x000000, 0x800000, CRC(83bb90bf) SHA1(44d3e62577e03766af5a92f421ed3a96616a5857) )
ROM_END

} // anonymous namespace

CONS( 200?, mylife,       0,              0,      mylife,  mylife, mylife_state, empty_init, "Giochi Preziosi / Playmates", "My Life - Another life in your hands (UK)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // maybe US, found in UK
CONS( 200?, mylifei,      mylife,         0,      mylife,  mylife, mylife_state, empty_init, "Giochi Preziosi",             "My Life - Un'altra vita nelle tue mani (Italy)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
// who was the Spanish distributor, what's the Spanish subtitle?
CONS( 200?, mylifes,      mylife,         0,      mylife,  mylife, mylife_state, empty_init, "Giochi Preziosi",             "My Life (Spain)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// there was a follow-up system 'My Real Life' featuring a camera, unknown if it's the same tech

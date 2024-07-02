// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************************************************

Similar to https://www.youtube.com/watch?v=FmyR-kL-QWo

base unit contains

1x Toshiba TMP90C845AF

1x SANYO LC21003 BLA5

3x SEC C941A KS0108B

1x Toshiba T9842B

(system has no bios ROM)

Cart sizes: 1MB, 2MB, 4MB

********************************************************************/

#include "emu.h"

#include "cpu/tlcs90/tlcs90.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"


namespace {

class pockchalv1_state : public driver_device
{
public:
	pockchalv1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
	{ }

	void pockchalv1(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	uint32_t screen_update_pockchalv1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	void pockchalv1_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	uint32_t m_rom_size = 0;
};


DEVICE_IMAGE_LOAD_MEMBER( pockchalv1_state::cart_load )
{
	m_rom_size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(m_rom_size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), m_rom_size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

void pockchalv1_state::video_start()
{
}

uint32_t pockchalv1_state::screen_update_pockchalv1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void pockchalv1_state::pockchalv1_map(address_map &map)
{
	map(0xc000, 0xffff).ram();
}


static INPUT_PORTS_START( pockchalv1 )
INPUT_PORTS_END



void pockchalv1_state::machine_start()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	if (m_cart->exists())
		space.install_read_handler(0x0000, 0x7fff, read8sm_delegate(*m_cart, FUNC(generic_slot_device::read_rom)));
}

void pockchalv1_state::machine_reset()
{
}


void pockchalv1_state::pockchalv1(machine_config &config)
{
	/* basic machine hardware */
	TMP90845(config, m_maincpu, 8000000);         /* ? MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &pockchalv1_state::pockchalv1_map);
//  m_maincpu->->set_vblank_int("screen", FUNC(pockchalv1_state::irq0_line_hold));

	// wrong, it's a b&w / greyscale thing
	PALETTE(config, "palette").set_format(palette_device::xRGB_444, 0x100).set_endianness(ENDIANNESS_BIG);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 256-16-1);
	screen.set_screen_update(FUNC(pockchalv1_state::screen_update_pockchalv1));
	screen.set_palette("palette");

	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "pockchalw_cart", "bin"));
	cartslot.set_device_load(FUNC(pockchalv1_state::cart_load));
	cartslot.set_must_be_loaded(true);

	SOFTWARE_LIST(config, "pc1_list").set_compatible("pockchalw");
}



ROM_START( pockchal )
ROM_END

} // Anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE     INPUT       CLASS             INIT        COMPANY                FULLNAME                      FLAGS
CONS( 199?, pockchal, 0,      0,      pockchalv1, pockchalv1, pockchalv1_state, empty_init, "Benesse Corporation", "Pocket Challenge W (Japan)", MACHINE_IS_SKELETON )

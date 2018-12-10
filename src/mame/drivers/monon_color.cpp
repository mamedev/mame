// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************
   Monon Color - Chinese handheld

   see
   https://gbatemp.net/threads/the-monon-color-a-new-video-game-console-from-china.467788/
   https://twitter.com/Splatoon2weird/status/1072182093206052864

   uses AX208 CPU (8051 @ 96Mhz with single cycle instructions + integrated video, jpeg decoder etc.)
   https://docplayer.net/52724058-Ax208-product-specification.html

***************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

class monon_color_state : public driver_device
{
public:
	monon_color_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cart(*this, "cartslot"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void monon_color(machine_config &config);
protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
private:
	required_device<generic_slot_device> m_cart;
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(monon_color_cart);
};

void monon_color_state::machine_start()
{
}

void monon_color_state::machine_reset()
{
}

void monon_color_state::video_start()
{
}


uint32_t monon_color_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( monon_color )
INPUT_PORTS_END


MACHINE_CONFIG_START(monon_color_state::monon_color)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", I8051, 96000000) // AX208! (needs custom core)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(monon_color_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 256)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "monon_color_cart")
	MCFG_GENERIC_EXTENSIONS("bin")
	MCFG_GENERIC_WIDTH(GENERIC_ROM8_WIDTH)
	MCFG_GENERIC_LOAD(monon_color_state, monon_color_cart)
	MCFG_GENERIC_MANDATORY

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","monon_color")
MACHINE_CONFIG_END

DEVICE_IMAGE_LOAD_MEMBER( monon_color_state, monon_color_cart )
{
	uint32_t size = m_cart->common_get_size("rom");
	std::vector<uint8_t> temp;
	temp.resize(size);
	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(&temp[0], size, "rom");

	memcpy(memregion("maincpu")->base(), &temp[0], size);

	return image_init_result::PASS;
}

ROM_START( mononcol )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
ROM_END

CONS( 2011, mononcol,    0,          0,  monon_color,  monon_color,    monon_color_state, empty_init,    "M&D",   "Monon Color", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    Easy Karaoke (c)IVL Technologies

    A version of this was also released in France by Lexibook, with French songs


    This uses

    Clarity 4.3 ARM
    SVI1186
    NV0165  0317
    Sound Vision Inc.

    an overview for 4.1 and 4.2 can be found at
    http://web.archive.org/web/20031212120255fw_/http://www.soundvisioninc.com/OEMProducts/C4datasheet072401.pdf
    Amusingly this datasheet advertises 'MAME Game emulation' as one of the capabilities despite the chip
    clocking in at only 72Mhz

    Support chip is

    IVL
    Technologies
    ICS0253R1.0
    UA1068ABK-RD
    0327 A01491F

    RAM chip is

    IC42S16400-7T

    ROM is

    IVL
    Technologies
    ICS0303-B
    (c)1985-1986
    3415BAI THAI

    --------------

    Cartridges contain:

    1x MX 29LV040TC-90 (Flash ROM)

    1x HC573A

    1x ICSI IC89LV52A-24PQ (80C52 MCU with 8KBytes Flash memory, can be read protected)

    presumably manages a serial protocol to send data to the main unit

*******************************************************************************/

#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

class easy_karaoke_state : public driver_device
{
public:
	easy_karaoke_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_cart(*this, "cartslot")
		, m_cart_region(nullptr)
	{ }

	void easy_karaoke(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void easy_karaoke_base(machine_config &config);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<cpu_device> m_maincpu;

	required_device<screen_device> m_screen;
	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint32_t a000004_r();

	void arm_map(address_map &map);
};

uint32_t easy_karaoke_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void easy_karaoke_state::machine_start()
{
	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		m_cart_region = memregion(std::string(m_cart->tag()) + GENERIC_ROM_REGION_TAG);
	}
}

void easy_karaoke_state::machine_reset()
{
	m_maincpu->set_state_int(ARM7_R15, 0x04000000);
}

DEVICE_IMAGE_LOAD_MEMBER(easy_karaoke_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

static INPUT_PORTS_START( easy_karaoke )
INPUT_PORTS_END

uint32_t easy_karaoke_state::a000004_r()
{
	return machine().rand();
}

void easy_karaoke_state::arm_map(address_map &map)
{
	map(0x00000000, 0x007fffff).ram();
	map(0x04000000, 0x043fffff).rom().region("maincpu", 0);
	map(0x0a000004, 0x0a000007).r(FUNC(easy_karaoke_state::a000004_r));
}


void easy_karaoke_state::easy_karaoke_base(machine_config &config)
{
	ARM9(config, m_maincpu, 72000000); // ARM 720 core
	m_maincpu->set_addrmap(AS_PROGRAM, &easy_karaoke_state::arm_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(easy_karaoke_state::screen_update));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "easy_karaoke_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(easy_karaoke_state::cart_load));

}

void easy_karaoke_state::easy_karaoke(machine_config &config)
{
	easy_karaoke_base(config);
	SOFTWARE_LIST(config, "cart_list").set_original("easy_karaoke_cart");
}

ROM_START( easykara )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ics0303-b.bin", 0x000000, 0x400000, CRC(43d86ae8) SHA1(219dcbf72b92d1b7e00f78f237194ab47dc08f1b) )
ROM_END

CONS( 2004, easykara,      0,       0,      easy_karaoke, easy_karaoke, easy_karaoke_state, empty_init, "IVL Technologies", "Easy Karaoke Groove Station", MACHINE_IS_SKELETON )

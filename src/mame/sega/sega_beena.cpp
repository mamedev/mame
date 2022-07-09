// license:BSD-3-Clause
// copyright-holders:R. Belmont
/******************************************************************************

    Sega Beena

    apbeena.cpp

    Skeleton driver for the Sega Advanced Pico BEENA

    H/W is custom Sega SoC with ARM7TDMI core at 81 MHz.

    TODO:
            Everything!
            Needs the internal BIOS dumped.
            Component list / PCB diagram

    Cartridge ROM has 'edinburgh' in the header, maybe a system codename?
    ROM is also full of OGG files containing the string 'Encoded with Speex speex-1.0.4'
    as well as .mid files for music.

    Cartridges pinout:

        Glob down, PCB cut corner at upper right
        Top row of pins A25 on left to A1 on right.
        Bottom row of pins B25 on left to B1 on right.

        A1  /CE (tied high with resistor)
        A2  D11
        A3  D3
        A4  D10
        A5  D2
        A6  D9
        A7  D1
        A8  D8
        A9  D0
        A10 /OE
        A11 N/C
        A12 A0
        A13 A1
        A14 A2
        A15 A3
        A16 A4
        A17 A5
        A18 A6
        A19 A7
        A20 A17
        A21 A18
        A22 N/C
        A23 VCC
        A24 N/C
        A25 GND

        B1  N/C
        B2  D4
        B3  D12
        B4  D5
        B5  D13
        B6  D6
        B7  D14
        B8  D7
        B9  D15
        B10 A16
        B11 A15
        B12 A14
        B13 A13
        B14 A12
        B15 A11
        B16 A10
        B17 A9
        B18 A8
        B19 A19
        B20 A20
        B21 N/C
        B22 A21
        B23 VCC
        B24 N/C
        B25 GND

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/arm7/arm7.h"

#include "softlist_dev.h"
#include "speaker.h"
#include "screen.h"


class sega_beena_state : public driver_device
{
public:
	sega_beena_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "arm7")
		, m_cart(*this, "cartslot")
		, m_cart_region(nullptr)
		, m_bank(*this, "cartbank")
	{ }

	void sega_beena(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	void beena_arm7_map(address_map &map);

	required_device<arm7_cpu_device> m_maincpu;
	optional_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;
	optional_memory_bank m_bank;
};

void sega_beena_state::beena_arm7_map(address_map &map)
{
	map(0x00000000, 0x000001ff).rom().bankr("cartbank");
	map(0x80000000, 0x807fffff).rom().bankr("cartbank");
}

void sega_beena_state::machine_start()
{
	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_region = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

		m_bank->configure_entries(0, (m_cart_region->bytes() + 0x7fffff) / 0x800000, m_cart_region->base(), 0x800000);
	}
	else
		m_bank->configure_entries(0, 1, memregion("bios")->base(), 0x800000);

	m_bank->set_entry(0);
}

void sega_beena_state::machine_reset()
{
}

DEVICE_IMAGE_LOAD_MEMBER(sega_beena_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");


	return image_init_result::PASS;
}

static INPUT_PORTS_START( sega_beena )
INPUT_PORTS_END


uint32_t sega_beena_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void sega_beena_state::sega_beena(machine_config &config)
{
	ARM7_BE(config, m_maincpu, 81'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sega_beena_state::beena_arm7_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(sega_beena_state::screen_update));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "sega_beena_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(sega_beena_state::cart_load));
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("sega_beena_cart");
}

ROM_START( beena )
	ROM_REGION32_BE( 0x80000, "bios", 0 )   // SoC internal BIOS
	ROM_LOAD16_WORD_SWAP( "beenabios.bin", 0x000000, 0x080000, NO_DUMP )
ROM_END


//    year, name,         parent,  compat, machine,      input,        class,              init,       company,  fullname,                             flags
CONS( 2009, beena,      0,       0,      sega_beena, sega_beena, sega_beena_state, empty_init, "Sega", "Advanced Pico BEENA", MACHINE_IS_SKELETON )

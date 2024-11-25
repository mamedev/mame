// license:BSD-3-Clause
// copyright-holders:
/******************************************************************************

"Zeebo", or "Genie" or "Longcheer W800", is a video game developed in a partnership
between Qualcomm and Zeebo Inc., the North American arm of the Brazilian company Tectoy.

A large amount of information can be found on
https://www.tripleoxygen.net/wiki/console/zeebo/start

----

Zeebo comes equipped with an ARM11 processor running at 528 MHZ. It is a chip
with low energy consumption and widely used in smartphones, such as the iPhone 3G.

Its graphics core is called Adreno 130, and was developed by Qualcomm with technology
from ATI.

----

System was launched in 2009, but discontinued shortly after.

Games were downloaded to NAND, but once the online store was closed the games could
no longer be downloaded, leaving no way to get them back onto the system after
doing a factory restore or similar.

----

The information from the site above should be transferred into the driver, there
are details such as the memory map, and dumps of RAM from a running system.

Is there a bootstrap ROM, or does this happen transparently to the CPU? how is
memory configuration determined by default etc?

This driver needs a proper owner.

*******************************************************************************/

#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "speaker.h"
#include "screen.h"


namespace {

class zeebo_game_state : public driver_device
{
public:
	zeebo_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "arm11")
	{ }

	void zeebo(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void zeebo_arm11_map(address_map &map) ATTR_COLD;

	void copy_block(int i, int blocksize, int blocksize_stripped, uint8_t* nandrom, int dest);
	void bootstrap();

	required_device<arm11_cpu_device> m_maincpu;
};

void zeebo_game_state::zeebo_arm11_map(address_map &map)
{
	map(0x00000000, 0x03ffffff).ram();
}


void zeebo_game_state::copy_block(int i, int blocksize, int blocksize_stripped, uint8_t* nandrom, int dest)
{
	const int base = i * blocksize;
	address_space& mem = m_maincpu->space(AS_PROGRAM);

	for (int j = 0; j < blocksize_stripped; j++)
	{
		uint8_t data = nandrom[base + j];
		//printf("writing to %08x : %02x", dest + j, data);
		mem.write_byte((dest+j)^3, data);
	}
}

void zeebo_game_state::bootstrap()
{
	uint8_t* rom = memregion("nand")->base();

	int j = 0;
	for (int i = 0xB700; i < 0xB800; i++) // how much is copied?
	{
		copy_block(i, 0x210, 0x200, rom, j * 0x200);
		j++;
	}
}


void zeebo_game_state::machine_start()
{
}

void zeebo_game_state::machine_reset()
{
	bootstrap();
}


static INPUT_PORTS_START( zeebo )
INPUT_PORTS_END


uint32_t zeebo_game_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void zeebo_game_state::zeebo(machine_config &config)
{
	ARM11(config, m_maincpu, 528000000); // 528 MHz ARM11 based SoC
	m_maincpu->set_addrmap(AS_PROGRAM, &zeebo_game_state::zeebo_arm11_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(zeebo_game_state::screen_update));

	SPEAKER(config, "speaker").front_center();
}

ROM_START( zeebo )
	ROM_REGION32_BE( 0x8400000, "nand", 0 )
	// older versions should be dumped too if possible
	ROM_LOAD( "1.1.2_spare.bin", 0x000000, 0x8400000, CRC(64bd6faa) SHA1(da0db9585d15cf7f1f127e39b0a5fa47f3c13cc0) )
ROM_END

} // anonymous namespace


CONS( 2009, zeebo,      0,       0,      zeebo, zeebo, zeebo_game_state, empty_init, "Zeebo Inc.", "Zeebo (Brazil)", MACHINE_IS_SKELETON )

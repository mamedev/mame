// license:BSD-3-Clause
// copyright-holders:R. Belmont
/************************************************************************************************************

    Sega electromechanical fruit machines (834-6912 PCB)

    Z80 x2
    RF5C68 - sound

    TODO: decryption isn't right, or there's more to it.  M3001 has somewhat plausible initial startup code
    but quickly goes off into the weeds.  M4001 isn't plausible even with the fill at the end cleared.

************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/rf5c68.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class segafruit_state : public driver_device
{
public:
	segafruit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soundlatch(*this, "soundlatch")
		, m_rf5c68(*this, "rf5c68")
	{
	}

	void segafruit(machine_config &config);

	void decrypt_m3001();
	void decrypt_m4001();

private:
	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<z80_device> m_maincpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<rf5c68_device> m_rf5c68;
};

void segafruit_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
}

void segafruit_state::io_map(address_map &map)
{
}

static INPUT_PORTS_START(segafruit)
INPUT_PORTS_END

void segafruit_state::decrypt_m3001()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x8000; i++)
	{
		rom[i] ^= 0x28;

		if (BIT(i, 12))
		{
			if (BIT(i, 4))
			{
				if (BIT(i, 8))
				{
					rom[i] ^= 0x80;
				}
				else
				{
					if (BIT(i, 0))
					{
						rom[i] ^= 0x80;
					}
				}
			}
			else
			{
				if (!(BIT(i, 0)))
				{
					rom[i] ^= 0x80;
				}
			}
		}
		else
		{
			if (BIT(i, 4))
			{
				if (BIT(i, 0))
				{
					rom[i] ^= 0x80;
				}
			}
			else
			{
				rom[i] ^= 0x80;
			}
		}
	}
}

void segafruit_state::decrypt_m4001()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x8000; i++)
	{
		if (BIT(i, 12))
		{
			if (BIT(i, 8))
			{
				if (BIT(i, 4))
				{
					rom[i] ^= 0x20;
				}
				else
				{
					if (BIT(i, 0))
					{
						rom[i] ^= 0x80;
					}
					else
					{
						rom[i] ^= 0x08;
					}
				}
			}
			else
			{
				if (BIT(i, 4))
				{
					rom[i] ^= 0x20;
				}
				else
				{
					if (BIT(i, 0))
					{
						rom[i] ^= 0x28;
					}
					else
					{
						rom[i] ^= 0x08;
					}
				}
			}
		}
		else
		{
			if (BIT(i, 8))
			{
				if (BIT(i, 4))
				{
					if (BIT(i, 0))
					{
						rom[i] ^= 0x88;
					}
				}
				else
				{
					if (BIT(i, 0))
					{
						rom[i] ^= 0x80;
					}
					else
					{
						rom[i] ^= 0x08;
					}
				}
			}
			else
			{
				if (BIT(i, 0))
				{
					if (BIT(i, 4))
					{
						rom[i] ^= 0x88;
					}
					else
					{
						rom[i] ^= 0x28;
					}
				}
			}
		}
	}
}

void segafruit_state::segafruit(machine_config & config)
{
	Z80(config, m_maincpu, 4'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &segafruit_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &segafruit_state::io_map);

	GENERIC_LATCH_8(config, m_soundlatch);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	RF5C68(config, m_rf5c68, XTAL(8'000'000));
	m_rf5c68->add_route(0, "lspeaker", 0.40);
	m_rf5c68->add_route(1, "rspeaker", 0.40);
}

ROM_START(m4001)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("epr-a12334.ic46", 0x000000, 0x008000, CRC(d56dbf57) SHA1(94f0ca78c14852a0fb12ad9ac9ce35fe0c19f113))

	ROM_REGION(0x4000, "soundcpu", 0)
	ROM_LOAD("epr-12335.ic51", 0x000000, 0x004000, CRC(d5ee7239) SHA1(9dad92623c8d732d26fffab8e50cd74616e2ac24))

	ROM_REGION(0x20000, "rf5c68", 0)
	ROM_LOAD("epr-12332.ic38", 0x000000, 0x010000, CRC(26b8a8ff) SHA1(2f0aa625d4162d63265618691701dc40593bdbdf))
	ROM_LOAD("epr-12333.ic24", 0x010000, 0x010000, CRC(b9db6c6e) SHA1(fc4610abd593e7f54422f4dcd2ff0170d6221d65))
ROM_END

ROM_START(m3001)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("epr-12602.ic46", 0x000000, 0x008000, CRC(00445ca5) SHA1(8aaa2117b5a3e36fcb5ef9b24258390ab4585163))

	ROM_REGION(0x4000, "soundcpu", 0)
	ROM_LOAD("epr-12603.ic51", 0x000000, 0x004000, CRC(90663368) SHA1(f5134a0146fabb09c165055b8da542cbb83d717a))

	ROM_REGION(0x20000, "rf5c68", 0)
	ROM_LOAD("epr-12600.ic38", 0x000000, 0x010000, CRC(99179df6) SHA1(2c26bea6724ded2bef90738fbb4b38d21a1f0906))
	ROM_LOAD("epr-12601.ic24", 0x010000, 0x010000, CRC(b1cb4265) SHA1(ce55bd47b0db8ad99233d98d4afc686ab2c66d63))
ROM_END

} // anonymous namespace

// Standalone M1 games
GAME(1990, m3001, 0, segafruit, segafruit, segafruit_state, decrypt_m3001, ROT0, "Sega", "M3001", MACHINE_NOT_WORKING)
GAME(1990, m4001, 0, segafruit, segafruit, segafruit_state, decrypt_m4001, ROT0, "Sega", "M4001", MACHINE_NOT_WORKING)

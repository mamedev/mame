// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Nintendo GameCube

    Skeleton driver, just to document the available firmware dumps
    for now.

***************************************************************************/

#include "emu.h"
#include "cpu/powerpc/ppc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class gamecube_state : public driver_device
{
public:
	gamecube_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_cpu(*this, "maincpu")
	{ }

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void decrypt(uint8_t *data, unsigned size);

	required_device<cpu_device> m_cpu;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

static ADDRESS_MAP_START( ppc_mem, AS_PROGRAM, 64, gamecube_state )
	AM_RANGE(0x00000000, 0x017fffff) AM_RAM // 24 MB main memory
	AM_RANGE(0x08000000, 0x081fffff) AM_RAM //  2 MB embedded framebuffer
	AM_RANGE(0xfff00000, 0xffffffff) AM_ROMBANK("boot")
ADDRESS_MAP_END


//**************************************************************************
//  INPUTS
//**************************************************************************

INPUT_PORTS_START( gc )
INPUT_PORTS_END


//**************************************************************************
//  MACHINE
//**************************************************************************

// bootrom descrambler reversed by segher
// Copyright 2008 Segher Boessenkool <segher@kernel.crashing.org>
void gamecube_state::decrypt(uint8_t *data, unsigned size)
{
	uint8_t acc = 0;
	uint8_t nacc = 0;

	uint16_t t = 0x2953;
	uint16_t u = 0xd9c2;
	uint16_t v = 0x3ff1;

	uint8_t x = 1;

	for (unsigned it = 0; it < size;)
	{
		int t0 = t & 1;
		int t1 = (t >> 1) & 1;
		int u0 = u & 1;
		int u1 = (u >> 1) & 1;
		int v0 = v & 1;

		x ^= t1 ^ v0;
		x ^= (u0 | u1);
		x ^= (t0 ^ u1 ^ v0) & (t0 ^ u0);

		if (t0 == u0)
		{
			v >>= 1;
			if (v0)
				v ^= 0xb3d0;
		}

		if (t0 == 0)
		{
			u >>= 1;
			if (u0)
				u ^= 0xfb10;
		}

		t >>= 1;
		if (t0)
			t ^= 0xa740;

		nacc++;
		acc = 2 * acc + x;
		if (nacc == 8)
		{
			data[it++] ^= acc;
			nacc = 0;
		}
	}
}

void gamecube_state::machine_start()
{
	decrypt(memregion("ipl")->base() + 0x100, 0x1afe00);

	// swap endianess after decryption
	uint8_t *base;
	int i, j;
	for (i = 0, base = memregion("ipl")->base(); i < memregion("ipl")->bytes(); i += 8)
	{
		uint8_t temp[8];
		memcpy(temp, base, 8);
		for (j = 8 - 1; j >= 0; j--)
			*base++ = temp[j];
	}

	membank("boot")->set_base(memregion("ipl")->base());
}

void gamecube_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

static MACHINE_CONFIG_START( gc )
	MCFG_CPU_ADD("maincpu", PPC603, 485000000 / 100) // 485 MHz IBM "Gekko" (750CXe/750FX based)
	MCFG_CPU_PROGRAM_MAP(ppc_mem)
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( gcjp )
	ROM_REGION(0x200000, "ipl", 0)
	ROM_DEFAULT_BIOS("v12")
	ROM_SYSTEM_BIOS(0, "v10", "IPL 1.0")
	ROMX_LOAD("jpn_v10.bin", 0x000000, 0x200000, CRC(6dac1f2a) SHA1(a1837968288253ed541f2b11440b68f5a9b33875), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v11", "IPL 1.1")
	ROMX_LOAD("jpn_v11.bin", 0x000000, 0x200000, CRC(d235e3f9) SHA1(96f69a21645de73a5ba61e57951ef303d55788c5), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v12", "IPL 1.2")
	ROMX_LOAD("jpn_v12.bin", 0x000000, 0x200000, CRC(8bdabbd4) SHA1(f1b0ef434cd74fd8fe23698e2fc911d945b45bf1), ROM_BIOS(3))

	ROM_REGION(0x20000, "dvd", 0)
	ROM_LOAD("20010608.bin", 0x00000, 0x20000, CRC(c047465a) SHA1(27872c201e87b06a19bf85d36c796ef383f8d52d))

	ROM_REGION(0x1000, "dsp_coef", 0)
	ROM_LOAD("dsp_coef.bin", 0x0000, 0x1000, CRC(d2777c90) SHA1(c116d867ba001dcd6bf6d399ff4bf38d340f556c))

	ROM_REGION(0x2000, "dsp_rom", 0)
	ROM_LOAD("dsp_rom.bin", 0x0000, 0x2000, CRC(47daaa65) SHA1(3c6cc6e04fdd0b2a392d7a6ed769455444846be7))
ROM_END

ROM_START( gcus )
	ROM_REGION(0x200000, "ipl", 0)
	ROM_DEFAULT_BIOS("v12")
	ROM_SYSTEM_BIOS(0, "v10", "IPL 1.0")
	ROMX_LOAD("usa_v10.bin", 0x000000, 0x200000, CRC(6d740ae7) SHA1(015808f637a984acde6a06efa7546e278293c6ee), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v11", "IPL 1.1")
	ROMX_LOAD("usa_v11.bin", 0x000000, 0x200000, CRC(d5e6feea) SHA1(239eacd86527ff9a75aeb7282da65797baeef010), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v12", "IPL 1.2")
	ROMX_LOAD("usa_v12.bin", 0x000000, 0x200000, CRC(86573808) SHA1(ef9194ab4804aa0aa8540d846caf291b28331165), ROM_BIOS(3))

	ROM_REGION(0x20000, "dvd", 0)
	ROM_LOAD("20010608.bin", 0x00000, 0x20000, CRC(c047465a) SHA1(27872c201e87b06a19bf85d36c796ef383f8d52d))

	ROM_REGION(0x1000, "dsp_coef", 0)
	ROM_LOAD("dsp_coef.bin", 0x0000, 0x1000, CRC(d2777c90) SHA1(c116d867ba001dcd6bf6d399ff4bf38d340f556c))

	ROM_REGION(0x2000, "dsp_rom", 0)
	ROM_LOAD("dsp_rom.bin", 0x0000, 0x2000, CRC(47daaa65) SHA1(3c6cc6e04fdd0b2a392d7a6ed769455444846be7))
ROM_END

ROM_START( gceu )
	ROM_REGION(0x200000, "ipl", 0)
	ROM_DEFAULT_BIOS("v12")
	ROM_SYSTEM_BIOS(0, "v10", "IPL 1.0")
	ROMX_LOAD("pal_v10.bin", 0x000000, 0x200000, CRC(4f319f43) SHA1(f27c63e5394e2fd1606f70df004c4fc2d6027700), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v11", "IPL 1.1")
	ROMX_LOAD("pal_v11.bin", 0x000000, 0x200000, CRC(dd8cab7c) SHA1(6f305c37dc1fbe332883bb8153eee26d3d325629), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v12", "IPL 1.2")
	ROMX_LOAD("pal_v12.bin", 0x000000, 0x200000, CRC(ad1b7f16) SHA1(80b8744ff5e43585392f55546bd03a673d11ef5f), ROM_BIOS(3))

	ROM_REGION(0x20000, "dvd", 0)
	ROM_LOAD("20010608.bin", 0x00000, 0x20000, CRC(c047465a) SHA1(27872c201e87b06a19bf85d36c796ef383f8d52d))

	ROM_REGION(0x1000, "dsp_coef", 0)
	ROM_LOAD("dsp_coef.bin", 0x0000, 0x1000, CRC(d2777c90) SHA1(c116d867ba001dcd6bf6d399ff4bf38d340f556c))

	ROM_REGION(0x2000, "dsp_rom", 0)
	ROM_LOAD("dsp_rom.bin", 0x0000, 0x2000, CRC(47daaa65) SHA1(3c6cc6e04fdd0b2a392d7a6ed769455444846be7))
ROM_END

ROM_START( gcbr )
	ROM_REGION(0x200000, "ipl", 0)
	ROM_LOAD("bra_v10.bin", 0x000000, 0x200000, CRC(667d0b64) SHA1(f3cd0c7c61cbcefa85e7de3aff4cfa50bc508714))

	ROM_REGION(0x20000, "dvd", 0)
	ROM_LOAD("20010608.bin", 0x00000, 0x20000, CRC(c047465a) SHA1(27872c201e87b06a19bf85d36c796ef383f8d52d))

	ROM_REGION(0x1000, "dsp_coef", 0)
	ROM_LOAD("dsp_coef.bin", 0x0000, 0x1000, CRC(d2777c90) SHA1(c116d867ba001dcd6bf6d399ff4bf38d340f556c))

	ROM_REGION(0x2000, "dsp_rom", 0)
	ROM_LOAD("dsp_rom.bin", 0x0000, 0x2000, CRC(47daaa65) SHA1(3c6cc6e04fdd0b2a392d7a6ed769455444846be7))
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  MACHINE  INPUT  CLASS           INIT  ROTATION  COMPANY    FULLNAME  FLAGS
GAME( 2001, gcjp,  0,      gc,      gc,    gamecube_state, 0,    ROT0,     "Nintendo", "GameCube (Japan)",  MACHINE_IS_SKELETON )
GAME( 2001, gcus,  gcjp,   gc,      gc,    gamecube_state, 0,    ROT0,     "Nintendo", "GameCube (US)",     MACHINE_IS_SKELETON )
GAME( 2002, gceu,  gcjp,   gc,      gc,    gamecube_state, 0,    ROT0,     "Nintendo", "GameCube (EU)",     MACHINE_IS_SKELETON )
GAME( 2002, gcbr,  gcjp,   gc,      gc,    gamecube_state, 0,    ROT0,     "Nintendo", "GameCube (Brazil)", MACHINE_IS_SKELETON )

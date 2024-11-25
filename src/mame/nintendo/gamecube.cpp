// license: BSD-3-Clause
// copyright-holders: Dirk Best, Segher Boessenkool
/***************************************************************************

    Nintendo GameCube

    Skeleton driver, just to document the available firmware dumps
    for now.

***************************************************************************/

#include "emu.h"
#include "cpu/powerpc/ppc.h"


namespace {

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

	void gc(machine_config &config);
	void ppc_mem(address_map &map) ATTR_COLD;
protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void decrypt(uint8_t *data, unsigned size);

	required_device<cpu_device> m_cpu;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void gamecube_state::ppc_mem(address_map &map)
{
	map(0x00000000, 0x017fffff).ram(); // 24 MB main memory
	map(0x08000000, 0x081fffff).ram(); //  2 MB embedded framebuffer
	map(0xfff00000, 0xffffffff).bankr("boot");
}


//**************************************************************************
//  INPUTS
//**************************************************************************

INPUT_PORTS_START( gc )
INPUT_PORTS_END


//**************************************************************************
//  MACHINE
//**************************************************************************

// bootrom descrambler reversed by segher
// Copyright 2008-2017 Segher Boessenkool <segher@kernel.crashing.org>
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

void gamecube_state::gc(machine_config &config)
{
	PPC603(config, m_cpu, 485000000 / 100); // 485 MHz IBM "Gekko" (750CXe/750FX based)
	m_cpu->set_addrmap(AS_PROGRAM, &gamecube_state::ppc_mem);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

/*  There are a lot of bad dumps of the gamecube IPLs floating around.
    The following IPL dumps are known bad, with RAM/MAC address garbage in the last 0x8000 bytes:
    ROM_LOAD("ipl_bad_ntsc_v10.bin",  0x000000, 0x200000, CRC(6d740ae7) SHA1(015808f637a984acde6a06efa7546e278293c6ee))
    ROM_LOAD("ipl_bad2_ntsc_v10.bin", 0x000000, 0x200000, CRC(8bdabbd4) SHA1(f1b0ef434cd74fd8fe23698e2fc911d945b45bf1))
    ROM_LOAD("ipl_bad_pal_v10.bin",   0x000000, 0x200000, CRC(dd8cab7c) SHA1(6f305c37dc1fbe332883bb8153eee26d3d325629))
    The following rom is flat out unknown and unseen in the wild, except for its checksums:
    ROM_LOAD("ipl_unknown.bin",       0x000000, 0x200000, CRC(d235e3f9) SHA1(96f69a21645de73a5ba61e57951ef303d55788c5))
*/

ROM_START( gcjp ) // DOL-001(JPN) and DOL-101(JPN); NTSC gamecube board, outputs NTSC color, NTSC timings; JPN Region jumper set
	ROM_REGION(0x200000, "ipl", 0)
	ROM_DEFAULT_BIOS("v12")
	ROM_SYSTEM_BIOS(0, "v10", "NTSC Revision 1.0") // Internal version 36  Mar 22 2001 22:38:31
	ROMX_LOAD("ipl_ntsc_v10.bin", 0x000000, 0x200000, CRC(6dac1f2a) SHA1(a1837968288253ed541f2b11440b68f5a9b33875), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v11", "NTSC Revision 1.1") // Internal version 47  Sep 27 2001 15:15:22
	ROMX_LOAD("ipl_ntsc_v11.bin", 0x000000, 0x200000, CRC(d5e6feea) SHA1(239eacd86527ff9a75aeb7282da65797baeef010), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v12", "NTSC Revision 1.2") // Internal version 0x2301  Jun 16 2003 04:27:06
	ROMX_LOAD("ipl_ntsc_v12.bin", 0x000000, 0x200000, CRC(86573808) SHA1(ef9194ab4804aa0aa8540d846caf291b28331165), ROM_BIOS(2)) // not verified from console yet but seems good
	// There may be another IPL with the same "NTSC Revision 1.2" string as above but not the same code. If so, it is undumped.

	ROM_REGION(0x20000, "dvd", 0)
	ROM_LOAD("20010608.bin", 0x00000, 0x20000, CRC(c047465a) SHA1(27872c201e87b06a19bf85d36c796ef383f8d52d))

	ROM_REGION(0x1000, "dsp_coef", 0)
	ROM_LOAD("dsp_coef.bin", 0x0000, 0x1000, CRC(d2777c90) SHA1(c116d867ba001dcd6bf6d399ff4bf38d340f556c))

	ROM_REGION(0x2000, "dsp_rom", 0)
	ROM_LOAD("dsp_rom.bin", 0x0000, 0x2000, CRC(47daaa65) SHA1(3c6cc6e04fdd0b2a392d7a6ed769455444846be7))
ROM_END

ROM_START( gcus ) // DOL-001(USA) and DOL-101(USA); NTSC gamecube board, outputs NTSC color, NTSC timings; USA region jumper set
	ROM_REGION(0x200000, "ipl", 0)
	ROM_DEFAULT_BIOS("v12")
	ROM_SYSTEM_BIOS(0, "v10", "NTSC Revision 1.0") // Internal version 36  Mar 22 2001 22:38:31
	ROMX_LOAD("ipl_ntsc_v10.bin", 0x000000, 0x200000, CRC(6dac1f2a) SHA1(a1837968288253ed541f2b11440b68f5a9b33875), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v11", "NTSC Revision 1.1") // Internal version 47  Sep 27 2001 15:15:22
	ROMX_LOAD("ipl_ntsc_v11.bin", 0x000000, 0x200000, CRC(d5e6feea) SHA1(239eacd86527ff9a75aeb7282da65797baeef010), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v12", "NTSC Revision 1.2") // Internal version 0x2301  Jun 16 2003 04:27:06
	ROMX_LOAD("ipl_ntsc_v12.bin", 0x000000, 0x200000, CRC(86573808) SHA1(ef9194ab4804aa0aa8540d846caf291b28331165), ROM_BIOS(2)) // not verified from console yet but seems good
	// There may be another IPL with the same "NTSC Revision 1.2" string as above but not the same code. If so, it is undumped.

	ROM_REGION(0x20000, "dvd", 0)
	ROM_LOAD("20010608.bin", 0x00000, 0x20000, CRC(c047465a) SHA1(27872c201e87b06a19bf85d36c796ef383f8d52d))

	ROM_REGION(0x1000, "dsp_coef", 0)
	ROM_LOAD("dsp_coef.bin", 0x0000, 0x1000, CRC(d2777c90) SHA1(c116d867ba001dcd6bf6d399ff4bf38d340f556c))

	ROM_REGION(0x2000, "dsp_rom", 0)
	ROM_LOAD("dsp_rom.bin", 0x0000, 0x2000, CRC(47daaa65) SHA1(3c6cc6e04fdd0b2a392d7a6ed769455444846be7))
ROM_END

ROM_START( gceu ) // DOL-001(EUR) and DOL-101(EUR); PAL gamecube board, outputs PAL-E color, PAL-E timings; EUR region (not sure if there is a separate jumper for this?)
	ROM_REGION(0x200000, "ipl", 0)
	ROM_DEFAULT_BIOS("v12")
	ROM_SYSTEM_BIOS(0, "v10", "PAL Revision 1.0") // Internal version 47  Sep 27 2001 15:15:22
	ROMX_LOAD("ipl_pal_v10.bin", 0x000000, 0x200000, CRC(4f319f43) SHA1(f27c63e5394e2fd1606f70df004c4fc2d6027700), ROM_BIOS(0))
	// "PAL Revision 1.1" IPL probably doesn't exist; the internal version of "PAL Revision 1.0" lines up with "NTSC Revision 1.1"
	ROM_SYSTEM_BIOS(1, "v12", "PAL Revision 1.2") // Internal version 0x2301  Jun 16 2003 04:27:06
	ROMX_LOAD("ipl_pal_v12.bin", 0x000000, 0x200000, CRC(ad1b7f16) SHA1(80b8744ff5e43585392f55546bd03a673d11ef5f), ROM_BIOS(1)) // not verified from console yet but seems good

	ROM_REGION(0x20000, "dvd", 0)
	ROM_LOAD("20010608.bin", 0x00000, 0x20000, CRC(c047465a) SHA1(27872c201e87b06a19bf85d36c796ef383f8d52d))

	ROM_REGION(0x1000, "dsp_coef", 0)
	ROM_LOAD("dsp_coef.bin", 0x0000, 0x1000, CRC(d2777c90) SHA1(c116d867ba001dcd6bf6d399ff4bf38d340f556c))

	ROM_REGION(0x2000, "dsp_rom", 0)
	ROM_LOAD("dsp_rom.bin", 0x0000, 0x2000, CRC(47daaa65) SHA1(3c6cc6e04fdd0b2a392d7a6ed769455444846be7))
ROM_END

ROM_START( gcbr ) // DOL-002(BRA); NTSC gamecube board, outputs video with PAL-M color, and either PAL-E (for the IPL) or PAL-M (for games) timings; region jumper is unknown but probably USA
	ROM_REGION(0x200000, "ipl", 0) // "MPAL Revision 1.1", Internal version 47  Sep 27 2001 15:15:22
	ROM_LOAD("ipl_mpal_v11.bin", 0x000000, 0x200000, CRC(667d0b64) SHA1(f3cd0c7c61cbcefa85e7de3aff4cfa50bc508714))

	ROM_REGION(0x20000, "dvd", 0)
	ROM_LOAD("20010608.bin", 0x00000, 0x20000, CRC(c047465a) SHA1(27872c201e87b06a19bf85d36c796ef383f8d52d))

	ROM_REGION(0x1000, "dsp_coef", 0)
	ROM_LOAD("dsp_coef.bin", 0x0000, 0x1000, CRC(d2777c90) SHA1(c116d867ba001dcd6bf6d399ff4bf38d340f556c))

	ROM_REGION(0x2000, "dsp_rom", 0)
	ROM_LOAD("dsp_rom.bin", 0x0000, 0x2000, CRC(47daaa65) SHA1(3c6cc6e04fdd0b2a392d7a6ed769455444846be7))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS           INIT        COMPANY     FULLNAME             FLAGS
CONS( 2001, gcjp,  0,      0,      gc,      gc,    gamecube_state, empty_init, "Nintendo", "GameCube (Japan)",  MACHINE_IS_SKELETON )
CONS( 2001, gcus,  gcjp,   0,      gc,      gc,    gamecube_state, empty_init, "Nintendo", "GameCube (USA)",    MACHINE_IS_SKELETON )
CONS( 2002, gceu,  gcjp,   0,      gc,      gc,    gamecube_state, empty_init, "Nintendo", "GameCube (EUR)",    MACHINE_IS_SKELETON )
CONS( 2002, gcbr,  gcjp,   0,      gc,      gc,    gamecube_state, empty_init, "Nintendo", "GameCube (Brazil)", MACHINE_IS_SKELETON )

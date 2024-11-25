// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

 IGS PC based hardware

TODO:
- Checks CPUID pretty soon that is a Geode processor at PC=f604d, jumps to lalaland
  on next CPUID with EAX=1 if ecx is equal to 0x540/0x551/0x552 ...
- "NV440" listed below doesn't make sense, NV[S]440 is a nVidia Quadro card.
  More likely it's either a GeForce MX 440 (NV17) or a GeForce 6200/6500 (NV43/NV44)

 Speed Driver
 -------------

 4 boards
    1x NV440 gfx card (?)
    1x sound card
    1x CF2IDE card
    1x proteection card with a IGS027A (ARM7 with internal ROM)

 The CF dump contains no MBR, it is thought that it is stored in either the main bios
 or provided by the IGS027A.  If you disconnect the protection card the bios will hang
 when detecting the HDDs which further backs up the theory that it is provided or
 decrypted by the 027A

 EZTouch
 --------------

 Custom board

 AMD Geode Processor (MediaGX derived?)
  marked
 AGXD533AAXFDCC
    0452EQA
    (c)2001 2.1
    TAIWAN

 IGS027 security chip (ARM with internal ROM) (dumped!)

 IGS035? (maybe..)

 CF card

*/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class speeddrv_state : public driver_device
{
public:
	speeddrv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void speeddrv(machine_config &config);

	void init_speeddrv();

private:
	void speeddrv_map(address_map &map) ATTR_COLD;

	required_device<mediagx_device> m_maincpu;
};

void speeddrv_state::speeddrv_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x20000);
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0);
}

static INPUT_PORTS_START( speeddrv )
INPUT_PORTS_END



void speeddrv_state::speeddrv(machine_config &config)
{
	// TODO: AMD Geode, superset of MediaGX, clock is probably higher
	MEDIAGX(config, m_maincpu, 40'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &speeddrv_state::speeddrv_map);

	PCI_ROOT(config, "pci", 0);
	// ...
}


ROM_START( speeddrv )
	ROM_REGION32_LE(0x40000, "bios", 0) /* motherboard bios */
	ROM_LOAD( "mainbios", 0x0000, 0x040000, NO_DUMP )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "speed_driver_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	DISK_REGION( "disks" )
	DISK_IMAGE( "speeddrv", 0, SHA1(88712a37b75d84cf9b5a4bee9386285d1b3760b3) )
ROM_END

ROM_START( eztouch )
	ROM_REGION32_LE(0x40000, "bios", 0) /* motherboard bios */
	ROM_LOAD( "szz_bios.rom", 0x0000, 0x040000, CRC(9b09f094) SHA1(a9c533afa29218339bbd4f12075f34f9574e3bf6) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "szz_027a.rom", 0x000000, 0x04000, CRC(f05dae69) SHA1(fa64e73cf045cda64aa0b702de3bd032a44d2c5c) )

	ROM_REGION( 0x80000, "extarm", 0 ) /* external ROM for ARM? (encrypted) */
	ROM_LOAD( "szz_v116cn.rom", 0x000000, 0x80000, CRC(8c443a89) SHA1(efdbaa832def812e0786cab95ebf60cdc226d3c4))

	DISK_REGION( "disks" )
	DISK_IMAGE( "szz_cf", 0, SHA1(d02cb1af0f03ce83719870d8a66244dde9795b2e) )
ROM_END


void speeddrv_state::init_speeddrv()
{
}

} // anonymous namespace


GAME( 2004, speeddrv, 0, speeddrv, speeddrv, speeddrv_state, init_speeddrv, ROT0, "IGS", "Speed Driver",          MACHINE_IS_SKELETON )
GAME( 200?, eztouch,  0, speeddrv, speeddrv, speeddrv_state, init_speeddrv, ROT0, "IGS", "EZ Touch (v116 China)", MACHINE_IS_SKELETON )

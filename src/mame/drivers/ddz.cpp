// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/****************************************************************************

    招级疯斗 - "Zhaoji Fengdou" - "Crazy Class" HW

    driver by Angelo Salese, based off original ddz.cpp by ElSemi

    TODO:
    - Decryption;

=============================================================================

Haze's notes:

fwiw, it's probably same PCB as the non-working 'ddz' in MAME, but different game.

there's some kind of encryption/scrambling going on, at the very least

Code:


Offset      0  1  2  3  4  5  6  7   8  9  A  B  C  D  E  F

0007BE60   00 00 00 99 03 AD AF 00  00 00 82 00 03 AD 64 63      ™ ­¯   ‚  ­dc
0007BE70   62 61 39 38 37 36 35 34  33 32 31 30 00 4E 61 4E   ba9876543210 NaN
0007BE80   00 66 6E 49 02 0E 85 06  02 0E 84 04 02 0E 83 EA    fnI  …   „   ƒê
0007BE90   02 0E 83 D6 02 0E 83 C8  02 0E 84 58 02 0E 84 12     ƒÖ  ƒÈ  „X  „
0007BEA0   66 65 28 00 30 00 65 73  61 62 20 64 61 62 20 3A   fe( 0 esab dab :
0007BEB0   66 74 6E 69 72 70 66 76  20 6E 69 20 67 75 62 00   ftnirpfv ni gub
0007BEC0   46 45 44 43 42 41 39 38  37 36 35 34 33 32 31 30   FEDCBA9876543210
0007BED0   00 29 6C 6C 75 6E 2E 00  00 00 8F 8E 02 0E 89 DC    )llun.    Ž  ‰Ü


if you reverse the letters you get 'bug in vfprintf : bad base'

so I suspect the data is in reverse order and maybe some blocks scrambled about.

****************************************************************************/

#include "emu.h"
#include "cpu/se3208/se3208.h"
#include "machine/nvram.h"
#include "machine/eepromser.h"
#include "machine/vrender0.h"
#include "sound/vrender0.h"
#include "video/vrender0.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>

#define IDLE_LOOP_SPEEDUP

class ddz_state : public driver_device
{
public:
	ddz_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_ipl(*this, "ipl"),
		m_encdata(*this, "enc_data"),
		m_maincpu(*this, "maincpu"),
		m_vr0soc(*this, "vr0soc")
	{ }


	void init_ddz();
	void ddz(machine_config &config);

private:

	/* memory pointers */
	required_shared_ptr<uint32_t> m_workram;
	required_region_ptr<uint8_t> m_ipl;
	required_region_ptr<uint8_t> m_encdata;

	/* devices */
	required_device<se3208_device> m_maincpu;
	required_device<vrender0soc_device> m_vr0soc;

	IRQ_CALLBACK_MEMBER(icallback);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	void ddz_mem(address_map &map);
};

IRQ_CALLBACK_MEMBER(ddz_state::icallback)
{
	return m_vr0soc->irq_callback();
}

void ddz_state::ddz_mem(address_map &map)
{
	map(0x00000000, 0x00ffffff).rom().nopw().region("ipl", 0);

//  map(0x01500000, 0x01500003).portr("IN0");
//  map(0x01500004, 0x01500007).portr("IN1");
//  map(0x01500008, 0x0150000b).portr("IN2");

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));

	map(0x02000000, 0x027fffff).ram().share("workram");

	map(0x03000000, 0x04ffffff).m(m_vr0soc, FUNC(vrender0soc_device::audiovideo_map));
}

static INPUT_PORTS_START( ddz )
	PORT_START("IN0")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

void ddz_state::machine_start()
{
	// ...
}

void ddz_state::machine_reset()
{
	// ...
}

void ddz_state::ddz(machine_config &config)
{
	SE3208(config, m_maincpu, 14318180 * 3); // TODO : different between each PCBs
	m_maincpu->set_addrmap(AS_PROGRAM, &ddz_state::ddz_mem);
	m_maincpu->set_irq_acknowledge_callback(FUNC(ddz_state::icallback));

	VRENDER0_SOC(config, m_vr0soc, 14318180 * 3);
	m_vr0soc->set_host_cpu_tag(m_maincpu);
}

ROM_START( ddz )
	ROM_REGION32_LE( 0x1000000, "ipl", ROMREGION_ERASEFF )

	ROM_REGION( 0x1000000, "enc_data", ROMREGION_ERASEFF )
	ROM_LOAD("ddz.001.rom",  0x000000, 0x400000, CRC(b379f823) SHA1(531885b35d668d22c75a9759994f4aca6eacb046) )
	ROM_LOAD("ddz.002.rom",  0x400000, 0x400000, CRC(285c744d) SHA1(2f8bc70825e55e3114015cb263e786df35cde275) )
	ROM_LOAD("ddz.003.rom",  0x800000, 0x400000, CRC(61c9b5c9) SHA1(0438417398403456a1c49408881797a94aa86f49) )
ROM_END


ROM_START( crzclass ) // PCB marked MAH-JONG
	ROM_REGION32_LE( 0x1000000, "ipl", ROMREGION_ERASEFF )

	ROM_REGION( 0x1000000, "enc_data", ROMREGION_ERASEFF )
	ROM_LOAD("tjf-mahjong-rom1.bin",  0x000000, 0x400000, CRC(0a8af816) SHA1(9f292e847873078ed2b7584f463633cf9086c7e8) ) // SHARP LH28F320BJD-TTL80
	ROM_LOAD("tjf-mahjong-rom2.bin",  0x400000, 0x400000, CRC(2a04e84a) SHA1(189b16fd4314fd2a5f8a1214618b5db83f8ac59a) ) // SHARP LH28F320BJD-TTL80
	ROM_LOAD("tjf-mahjong-rom3.bin",  0x800000, 0x400000, CRC(1cacf3f9) SHA1(e6c88c98aeb7df4098f8e20f412018617005724d) ) // SHARP LH28F320BJD-TTL80
	// rom4 not populated
ROM_END

void ddz_state::init_ddz()
{
	for(uint32_t x=0;x<m_encdata.bytes();x+=16)
	{
		// TBD
		for(int y=0;y<16;y++)
			m_ipl[x+(y)] = m_encdata[x+y];
//          m_ipl[x+(15-y)] = m_encdata[x+y];
	}
}

GAME( 200?, ddz,      0,        ddz,  ddz,  ddz_state, init_ddz,    ROT0, "IGS?",                "Dou Di Zhu", MACHINE_IS_SKELETON )
GAME( 200?, crzclass, 0,        ddz,  ddz,  ddz_state, init_ddz,    ROT0, "TJF",                 "Zhaoji Fengdou", MACHINE_IS_SKELETON ) // 'Crazy Class'


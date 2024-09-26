// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Kawai K4 synthesizer.

***************************************************************************/

#include "emu.h"
#include "cpu/upd78k/upd78k3.h"
//#include "machine/nvram.h"


namespace {

class kawai_k4_state : public driver_device
{
public:
	kawai_k4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_mpu(*this, "mpu")
	{
	}

	void k4(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<upd78310_device> m_mpu;
};


void kawai_k4_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("coderom", 0);
	map(0x8000, 0xbfff).rom().region("coderom", 0x8000); // TODO: banked with other stuff
}


static INPUT_PORTS_START(k4)
INPUT_PORTS_END

void kawai_k4_state::k4(machine_config &config)
{
	UPD78310(config, m_mpu, 12_MHz_XTAL); // µPD78310G-36
	m_mpu->set_addrmap(AS_PROGRAM, &kawai_k4_state::mem_map);

	//NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC51832SP-12 + battery
}

ROM_START(k4)
	ROM_REGION(0x10000, "coderom", 0)
	ROM_SYSTEM_BIOS(0, "v1.4", "Version 1.4")
	ROMX_LOAD("k4_v14.u8",    0x00000, 0x10000, CRC(bf463780) SHA1(acb391fc8d074420a0dcdf0cbea523f6e7601e66), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1.3", "Version 1.3")
	ROMX_LOAD("p206b.u8",     0x00000, 0x10000, CRC(4108dec4) SHA1(8666583bc2f7a6792586325d8b98ffb684e51259), ROM_BIOS(1)) // 27C512
	ROM_SYSTEM_BIOS(2, "v1.0", "Version 1.0")
	ROMX_LOAD("p206_e5dp.u8", 0x00000, 0x10000, CRC(fdf47a8e) SHA1(2a074da55746cc51f1e06d3f88afc296750f3ea6), ROM_BIOS(2)) // 27C512

	ROM_REGION(0x180000, "waverom", 0)
	ROM_LOAD("d941_p202-m8dw.u30", 0x000000, 0x80000, NO_DUMP) // TC534000P
	ROM_LOAD("d942_p203-m8dw.u29", 0x080000, 0x80000, NO_DUMP) // TC534000P
	ROM_LOAD("d943_p204-m8dw.u31", 0x100000, 0x80000, NO_DUMP) // TC534000P
ROM_END

ROM_START(k4r)
	ROM_REGION(0x10000, "coderom", 0)
	ROM_SYSTEM_BIOS(0, "v1.4", "Version 1.4")
	ROMX_LOAD("p207c.u7",    0x00000, 0x10000, CRC(17bdaaf6) SHA1(a1220fd479e154be679596ab18a1e76ae722a39d), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1.3", "Version 1.3")
	ROMX_LOAD("k4r_v1.3.u7", 0x00000, 0x10000, CRC(1b36d5ce) SHA1(afe25260e3ada1646d6c04cebb4c6dec41f742c0), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v1.2", "Version 1.2")
	ROMX_LOAD("k4r_v1.2.u7", 0x00000, 0x10000, CRC(8b4953bc) SHA1(7bdb0d15bfe396fe6c2499c75137d71aef6c2ca8), ROM_BIOS(2))

	ROM_REGION(0x180000, "waverom", 0)
	ROM_LOAD("d941_p202-m8dw.u40", 0x000000, 0x80000, NO_DUMP) // TC534000P
	ROM_LOAD("d942_p203-m8dw.u39", 0x080000, 0x80000, NO_DUMP) // TC534000P
	ROM_LOAD("d943_p204-m8dw.u38", 0x100000, 0x80000, NO_DUMP) // TC534000P
ROM_END

} // anonymous namespace


SYST(1989, k4,  0,  0, k4, k4, kawai_k4_state, empty_init, "Kawai Musical Instrument Manufacturing", "K4 16-bit Digital Synthesizer",         MACHINE_IS_SKELETON)
SYST(1989, k4r, k4, 0, k4, k4, kawai_k4_state, empty_init, "Kawai Musical Instrument Manufacturing", "K4r 16-bit Digital Synthesizer Module", MACHINE_IS_SKELETON)

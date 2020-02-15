// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland D-50/D-550.

****************************************************************************/

#include "emu.h"
#include "cpu/upd78k/upd78k3.h"

class roland_d50_state : public driver_device
{
public:
	roland_d50_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void d50(machine_config &config);

private:
	void mem_map(address_map &map);

	required_device<upd78312_device> m_maincpu;
};


void roland_d50_state::mem_map(address_map &map)
{
	// Internal ROM is enabled at 0000–1FFF (+5V pullup on EA pin)
	map(0x2000, 0xfdff).rom().region("progrom", 0x2000); // TODO: banking
}


static INPUT_PORTS_START(d50)
INPUT_PORTS_END


void roland_d50_state::d50(machine_config &config)
{
	UPD78312(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_d50_state::mem_map);

	// LCD unit is LM402802 (D-50) or LM402551 (D-550)

	//MB87136(config, "synthe", 32.768_MHz_XTAL);
}


ROM_START(d50)
	ROM_REGION(0x10000, "progrom", 0)
	ROM_SYSTEM_BIOS(0, "v2.22", "ROM Version 2.22")
	ROMX_LOAD("d-50_222.bin", 0x00000, 0x10000, CRC(e92c69f9) SHA1(fd783abc7fbd4abe2dedfe89d59e396b5e687a27), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v2.20", "ROM Version 2.20 (CFW, patched)")
	ROMX_LOAD("roland_d50_220rw_cfw_patched.bin", 0x00000, 0x10000, CRC(8901640e) SHA1(abbc0026c64c55a6cbaca5e3e5dcb1a7536e91b3), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v2.10", "ROM Version 2.10")
	ROMX_LOAD("d50 - v.2.10 - ic22 - 27c512.bin", 0x00000, 0x10000, CRC(d1387c54) SHA1(904cf9daf296a66d3c4969266541983081e19471), ROM_BIOS(2))

	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("d78312g-022_15179266.ic25", 0x0000, 0x2000, NO_DUMP) // 8-digit Roland part number not printed on IC
	ROM_COPY("progrom", 0x0000, 0x0000, 0x2000)

	ROM_REGION(0x40000, "pcma", 0)
	ROM_LOAD("tc532000p-7469.ic30", 0x00000, 0x40000, NO_DUMP)

	ROM_REGION(0x40000, "pcmb", 0)
	ROM_LOAD("tc532000p-7470.ic29", 0x00000, 0x40000, NO_DUMP)
ROM_END

ROM_START(d50a)
	ROM_REGION(0x10000, "progrom", 0)
	ROM_SYSTEM_BIOS(0, "v1.06", "ROM Version 1.06")
	ROMX_LOAD("d50-v1.06.bin", 0x00000, 0x10000, CRC(ccba4e46) SHA1(ce56321226dbbf7dbfac2ad344da447ad6448ee8), ROM_BIOS(0))

	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("d78312g-017_15179261.ic25", 0x0000, 0x2000, NO_DUMP) // not compatible with newer firmware
	ROM_COPY("progrom", 0x0000, 0x0000, 0x2000)

	ROM_REGION(0x40000, "pcma", 0)
	ROM_LOAD("tc532000p-7469.ic30", 0x00000, 0x40000, NO_DUMP)

	ROM_REGION(0x40000, "pcmb", 0)
	ROM_LOAD("tc532000p-7470.ic29", 0x00000, 0x40000, NO_DUMP)
ROM_END

ROM_START(d550)
	ROM_REGION(0x10000, "progrom", 0)
	ROM_SYSTEM_BIOS(0, "v1.02", "ROM Version 1.02")
	ROMX_LOAD("roland d-550 v1.02 eprom firmware.bin", 0x00000, 0x10000, CRC(11b54d24) SHA1(d11bdb50c49edababec73a936346a6f918dd0949), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1.01", "ROM Version 1.01")
	ROMX_LOAD("roland_d-550_ver_1.01.bin", 0x00000, 0x10000, CRC(c267019f) SHA1(314616d14b91e8c8733aee5dd64736fda8ff6904), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v1.0.0", "ROM Version 1.0.0")
	ROMX_LOAD("d-550__1.0.0.mbm27c512.ic22.bin", 0x00000, 0x10000, CRC(f8ec84c3) SHA1(00b2b74009bdc0747e11bf57d4dc6c39424c7669), ROM_BIOS(2))

	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("d78312g-022_15179266.ic25", 0x0000, 0x2000, NO_DUMP)
	ROM_COPY("progrom", 0x0000, 0x0000, 0x2000)

	ROM_REGION(0x40000, "pcma", 0)
	ROM_LOAD("tc532000p-7469.ic30", 0x00000, 0x40000, NO_DUMP)

	ROM_REGION(0x40000, "pcmb", 0)
	ROM_LOAD("tc532000p-7470.ic29", 0x00000, 0x40000, NO_DUMP)
ROM_END

SYST(1987, d50,  0,   0, d50, d50, roland_d50_state, empty_init, "Roland", "D-50 (Ver. 2.xx)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
SYST(1987, d50a, d50, 0, d50, d50, roland_d50_state, empty_init, "Roland", "D-50 (Ver. 1.xx)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
SYST(1987, d550, d50, 0, d50, d50, roland_d50_state, empty_init, "Roland", "D-550", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

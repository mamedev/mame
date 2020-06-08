// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for CP/M system by GNAT Computers.

    For more information, visit the following link:
    https://classictech.wordpress.com/computer-companies/gnat-computers-san-diego-calif/

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/wd_fdc.h"
#include "machine/z80sio.h"

class gnat10_state : public driver_device
{
public:
	gnat10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "fdc")
	{
	}

	void gnat10(machine_config &config);

private:
	u8 floppy_status_r();

	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<fd1797_device> m_fdc;
};


u8 gnat10_state::floppy_status_r()
{
	return m_fdc->drq_r() << 7 | m_fdc->intrq_r();
}

void gnat10_state::mem_map(address_map &map)
{
	map(0x0000, 0x0002).rom().region("boot", 0);
	map(0x0003, 0xf7ff).ram();
	map(0xf800, 0xffff).rom().region("boot", 0);
}

void gnat10_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x60, 0x63).rw("sio1", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x70, 0x73).rw("sio2", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x80, 0x83).rw(m_fdc, FUNC(fd1797_device::read), FUNC(fd1797_device::write));
	map(0xa0, 0xa0).r(FUNC(gnat10_state::floppy_status_r));
}


static INPUT_PORTS_START(gnat10)
INPUT_PORTS_END

void gnat10_state::gnat10(machine_config &config)
{
	Z80(config, m_maincpu, 4'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &gnat10_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &gnat10_state::io_map);

	Z80SIO(config, "sio1", 4'000'000); // type unknown
	Z80SIO(config, "sio2", 4'000'000); // type unknown

	FD1797(config, m_fdc, 1'000'000); // type unknown

	// TODO: video screen, peripheral ports
}

ROM_START(gnat10)
	ROM_REGION(0x800, "boot", 0)
	ROM_LOAD("gnat-507", 0x000, 0x800, CRC(72baa750) SHA1(7b78324b90b8c6f78c88a7dde8d53ea612ea1f7f)) // LF patched back to CR/LF in four instances
ROM_END

COMP(1980, gnat10, 0, 0, gnat10, gnat10, gnat10_state, empty_init, "GNAT Computers", "GNAT System 10", MACHINE_IS_SKELETON)

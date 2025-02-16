// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Commodore A570

    DMAC based CD-ROM controller for the A500

    Notes:
    - Essentially turns the A500 into a CDTV
    - A prototype version is called A690
    - ROM label for the A690: "391298-01 V1.0 Copyright ©1991 CBM C480"
    - The ROM P/N 391298-01 seems to have been used for multiple versions
    - There are expansion slots for a 2 MB RAM expansion and a SCSI module

    TODO:
    - DMAC/CD-ROM drive hookup (needs DMAC rev 2)

***************************************************************************/

#include "emu.h"
#include "a570.h"

#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AMIGA_CPUSLOT_A570, bus::amiga::cpuslot::a570_device, "amiga_a570", "Commodore A570")

namespace bus::amiga::cpuslot {

a570_device::a570_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_CPUSLOT_A570, tag, owner, clock),
	device_amiga_cpuslot_interface(mconfig, *this),
	m_dmac(*this, "dmac"),
	m_config(*this, "config")
{
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void a570_device::map(address_map &map)
{
	map(0xdc8000, 0xdc87ff).mirror(0x07800).rw("nvram0", FUNC(at28c16_device::read), FUNC(at28c16_device::write)).umask16(0x00ff);
	map(0xdc8000, 0xdc87ff).mirror(0x07800).rw("nvram1", FUNC(at28c16_device::read), FUNC(at28c16_device::write)).umask16(0xff00);
	map(0xf00000, 0xf3ffff).mirror(0x40000).rom().region("bootrom", 0);
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( a570 )
	PORT_START("config")
	PORT_CONFNAME(0x01, 0x00, "2 MB RAM Expansion")
	PORT_CONFSETTING(0x00, "Disabled")
	PORT_CONFSETTING(0x01, "Enabled")
INPUT_PORTS_END

ioport_constructor a570_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a570 );
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( firmware )
	ROM_REGION16_BE(0x40000, "bootrom", 0)
	// COMMODORE-AMIGA  391298-01  ©1992 V2.30
	ROM_LOAD("391298-01_v230.u20", 0x00000, 0x40000, CRC(30b54232) SHA1(ed7e461d1fff3cda321631ae42b80e3cd4fa5ebb))
ROM_END

const tiny_rom_entry *a570_device::device_rom_region() const
{
	return ROM_NAME( firmware );
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void a570_device::device_add_mconfig(machine_config &config)
{
	AMIGA_DMAC_REV2(config, m_dmac, 28.37516_MHz_XTAL / 4); // 7M
	m_dmac->cfgout_cb().set([this] (int state) { m_host->cfgout_w(state); });
	m_dmac->int_cb().set([this] (int state) { m_host->int2_w(state); });

	AT28C16(config, "nvram0", 0);
	AT28C16(config, "nvram1", 0);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void a570_device::device_start()
{
	m_ram = make_unique_clear<uint16_t[]>(0x200000/2);

	m_dmac->set_address_space(&m_host->space());
	m_dmac->set_ram(m_ram.get());

	m_host->space().install_device(0x000000, 0xffffff, *this, &a570_device::map);

	// register for save states
	save_pointer(NAME(m_ram), 0x200000/2);
}

// the dmac handles this
void a570_device::cfgin_w(int state) { m_dmac->configin_w(state); }

void a570_device::rst_w(int state)
{
	// call rst first as it will unmap memory
	m_dmac->rst_w(state);

	if (state == 0)
		m_dmac->ramsz_w(m_config->read() ? 3 : 0);
}

} // namespace bus::amiga::cpuslot

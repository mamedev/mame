// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Arcturus sidecar cartridge
    Michael Zapf

*****************************************************************************/

#include "emu.h"
#include "arcturus.h"

#define LOG_WARN        (1U << 1)   // Warnings

// #define VERBOSE (LOG_GENERAL | LOG_WARN)

#include "logmacro.h"

#define TI99_CARTSC_4000 "mem4000"
#define TI99_CARTSC_A000 "mema000"
#define TI99_CARTSC_C000 "memc000"
#define TI99_CARTSC_RAM "ram"

DEFINE_DEVICE_TYPE(TI99_ARCTURUS, bus::ti99::sidecar::arcturus_device, "ti99_arcturus", "Arcturus sidecar cartridge")

namespace bus::ti99::sidecar {

/*
    Constructor called from subclasses.
*/
arcturus_device::arcturus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bus::ti99::internal::ioport_attached_device(mconfig, TI99_ARCTURUS, tag, owner, clock),
	m_ram(*this, TI99_CARTSC_RAM)
{
}

void arcturus_device::readz(offs_t offset, uint8_t *value)
{
	switch ((offset >> 13)&0x07)
	{
	case 0:
	case 3:
	case 4:
	case 7:
		break;
	case 1:
		*value = m_ram->pointer()[offset & 0x07ff];
		break;
	case 2:
		*value = m_rom4[offset & 0x1fff];
		break;
	case 5:
		*value = m_roma[offset & 0x1fff];
		break;
	case 6:
		*value = m_romc[offset & 0x1fff];
		break;
	}
}

void arcturus_device::write(offs_t offset, uint8_t data)
{
	if (((offset >> 13)&0x07)==1)
		m_ram->pointer()[offset & 0x07ff] = data;
}

void arcturus_device::device_start()
{
	m_rom4 = memregion(TI99_CARTSC_4000)->base();
	m_roma = memregion(TI99_CARTSC_A000)->base();
	m_romc = memregion(TI99_CARTSC_C000)->base();
}

ROM_START( ti99_arcturus )
	ROM_REGION(0x2000, TI99_CARTSC_4000, 0)
	ROM_LOAD("arcturus.u1", 0x0000, 0x2000, CRC(28ba65ec) SHA1(63329930c71ef776e8598a8d4c580c41bd52a339))
	ROM_REGION(0x2000, TI99_CARTSC_A000, 0)
	ROM_LOAD("arcturus.u2", 0x0000, 0x2000, CRC(91e6910a) SHA1(51c1f75d4e9d74af21c0b43188fc0ffc4a3ad4c0))
	ROM_REGION(0x2000, TI99_CARTSC_C000, 0)
	ROM_LOAD("arcturus.u3", 0x0000, 0x2000, CRC(e9ce9f4e) SHA1(65850c57e9480a5d8bdd852692f18d0162ca1406))
ROM_END

void arcturus_device::device_add_mconfig(machine_config& config)
{
	RAM(config, TI99_CARTSC_RAM).set_default_size("2K").set_default_value(0);
}

const tiny_rom_entry *arcturus_device::device_rom_region() const
{
	return ROM_NAME( ti99_arcturus );
}

} // end namespace bus::ti99::peb

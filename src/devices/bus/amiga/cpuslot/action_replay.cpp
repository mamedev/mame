// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Datel Electronics Action Replay

    Freezer cartridge for the A500 and A2000

    Notes:
    - The first four bytes cannot be read if you read the ROM using
      the cartridge. This the reason for the BAD_DUMP flags.

    TODO:
    - MK1: Unimplemented (ROM: 0xf00000-0xf0ffff, RAM: 0x9fc000-0x9fffff)
    - MK2: Partially works, but entering/exiting from cartridge isn't stable
    - MK3: Not working (needs different reset handling)
    - Unmapping ROM/RAM
    - Reset behaviour: Real cartridge measures reset pulse length to
      differentiate between keyboard reset and CPU RESET instruction
    - Breakpoints
    - A2000 version

***************************************************************************/

#include "emu.h"
#include "action_replay.h"

#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AMIGA_CPUSLOT_ACTION_REPLAY_MK1, bus::amiga::cpuslot::action_replay_mk1_device, "amiga_ar1", "Action Replay")
DEFINE_DEVICE_TYPE(AMIGA_CPUSLOT_ACTION_REPLAY_MK2, bus::amiga::cpuslot::action_replay_mk2_device, "amiga_ar2", "Action Replay MK II")
DEFINE_DEVICE_TYPE(AMIGA_CPUSLOT_ACTION_REPLAY_MK3, bus::amiga::cpuslot::action_replay_mk3_device, "amiga_ar3", "Action Replay MK III")

namespace bus::amiga::cpuslot {

action_replay_device_base::action_replay_device_base(const machine_config &mconfig, device_type type, size_t ram_size, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_amiga_cpuslot_interface(mconfig, *this),
	m_rom(*this, "rom"),
	m_ram(*this, "ram", ram_size, ENDIANNESS_BIG)
{
}

action_replay_mk1_device::action_replay_mk1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	action_replay_device_base(mconfig, AMIGA_CPUSLOT_ACTION_REPLAY_MK1, 0x4000, tag, owner, clock)
{
}

action_replay_mk2_device::action_replay_mk2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	action_replay_device_base(mconfig, AMIGA_CPUSLOT_ACTION_REPLAY_MK2, 0x10000, tag, owner, clock)
{
}

action_replay_mk2_device::action_replay_mk2_device(const machine_config &mconfig, device_type type, size_t ram_size, const char *tag, device_t *owner, uint32_t clock) :
	action_replay_device_base(mconfig, type, ram_size, tag, owner, clock)
{
}

action_replay_mk3_device::action_replay_mk3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	action_replay_mk2_device(mconfig, AMIGA_CPUSLOT_ACTION_REPLAY_MK3, 0x10000, tag, owner, clock)
{
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void action_replay_mk2_device::regs_map(address_map &map)
{
	// base address 0x400000
	map(0x000000, 0x03ffff).rom().region("rom", 0);
	map(0x000000, 0x000001).w(FUNC(action_replay_mk2_device::mode_w));
	map(0x000000, 0x000001).mirror(0x02).r(FUNC(action_replay_mk2_device::status_r));
	map(0x000006, 0x000007).w(FUNC(action_replay_mk2_device::restore_w));
	map(0x040000, 0x04ffff).mirror(0x30000).ram().share(m_ram);
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( action_replay )
	PORT_START("button")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Freeze") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(action_replay_device_base::freeze), 0)
INPUT_PORTS_END

ioport_constructor action_replay_device_base::device_input_ports() const
{
	return INPUT_PORTS_NAME( action_replay );
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( rom_mk1 )
	ROM_REGION(0x10000, "rom", 0)
	ROM_DEFAULT_BIOS("v150")
	ROM_SYSTEM_BIOS(0, "v100", "Version 1.00")
	ROMX_LOAD("ar1_v100.bin", 0x0000, 0x10000, BAD_DUMP CRC(2d921771) SHA1(1ead9dda2dad29146441f5ef7218375022e01248), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v150", "Version 1.50")
	ROMX_LOAD("ar1_v150.bin", 0x0000, 0x10000, BAD_DUMP CRC(f82c4258) SHA1(843b433b2c56640e045d5fdc854dc6b1a4964e7c), ROM_BIOS(1))
ROM_END

const tiny_rom_entry *action_replay_mk1_device::device_rom_region() const
{
	return ROM_NAME( rom_mk1 );
}

ROM_START( rom_mk2 )
	ROM_REGION16_BE(0x40000, "rom", 0)
	ROM_DEFAULT_BIOS("v214")
	ROM_SYSTEM_BIOS(0, "v205", "Version 2.05")
	ROMX_LOAD("ar2_v205.bin", 0x00000, 0x20000, BAD_DUMP CRC(4051eef8) SHA1(9df22b1d3285b522c223697c83d144d04e961a4a), ROM_BIOS(0))
	ROM_RELOAD(               0x20000, 0x20000)
	ROM_SYSTEM_BIOS(1, "v212", "Version 2.12")
	ROMX_LOAD("ar2_v212.bin", 0x00000, 0x20000, BAD_DUMP CRC(d29bdd86) SHA1(76c2900457badf22b742f0af48b78937e8b67694), ROM_BIOS(1))
	ROM_RELOAD(               0x20000, 0x20000)
	ROM_SYSTEM_BIOS(2, "v214", "Version 2.14")
	ROMX_LOAD("ar2_v214.bin", 0x00000, 0x20000, BAD_DUMP CRC(1bb3d0a8) SHA1(14b1f5a69efb6f4e2331970e6ca0f33c0f04ac91), ROM_BIOS(2))
	ROM_RELOAD(               0x20000, 0x20000)
ROM_END

const tiny_rom_entry *action_replay_mk2_device::device_rom_region() const
{
	return ROM_NAME( rom_mk2 );
}

ROM_START( rom_mk3 )
	ROM_REGION16_BE(0x40000, "rom", 0)
	ROM_DEFAULT_BIOS("v317")
	ROM_SYSTEM_BIOS(0, "v309", "Version 3.09")
	ROMX_LOAD("ar3_v309.evn", 0x00000, 0x20000, CRC(2b84519f) SHA1(7841873bf009d8341dfa2794b3751bacf86adcc8), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("ar3_v309.odd", 0x00001, 0x20000, CRC(1d35bd56) SHA1(6464be1626b519499e76e4e3409e8016515d48b6), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v317", "Version 3.17")
	ROMX_LOAD("ar3_v317.evn", 0x00000, 0x20000, CRC(5fb69a10) SHA1(2ce641d1e2e254769be7d1ba41aace079cb04e27), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("ar3_v317.odd", 0x00001, 0x20000, CRC(f767d072) SHA1(a7431d22bfdb746a8bb6073c9edf7a2b65c6d307), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

const tiny_rom_entry *action_replay_mk3_device::device_rom_region() const
{
	return ROM_NAME( rom_mk3 );
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void action_replay_mk1_device::device_start()
{
}

void action_replay_mk1_device::freeze_w(int state)
{
	LOG("freeze_w: %d\n", state);
}

void action_replay_mk2_device::install_chipmem_taps()
{
	m_chipmem_read_tap = m_host->space().install_read_tap
	(
		0x000000, 0x1fffff,
		"chipmem_r",
		[this] (offs_t offset, uint16_t &data, uint16_t mem_mask)
		{
			if (m_nmi_active)
			{
				// disable chip memory
				m_host->ovr_w(0);

				// map action replay
				m_host->space().install_rom(0x000000, 0x03ffff, 0x040000, m_rom);
				m_host->space().install_device(0x400000, 0x47ffff, *this, &action_replay_mk2_device::regs_map);

				// the rom is mapped too late for this read, so adjust it with the data that would have been read
				data = m_rom[offset >> 1];

				m_nmi_active = false;
			}
		},
		&m_chipmem_read_tap
	);

	m_chipmem_write_tap = m_host->space().install_write_tap
	(
		0x000000, 0x1fffff,
		"chipmem_w",
		[this] (offs_t offset, uint16_t &data, uint16_t mem_mask)
		{
			if (m_reset)
			{
				// trigger nmi
				LOG("trigger nmi\n");

				m_nmi_active = true;
				m_host->ipl7_w(1);
				m_host->ipl7_w(0);

				m_reset = false;
			}
		},
		&m_chipmem_write_tap
	);
}

void action_replay_mk2_device::device_start()
{
	m_status = STATUS_RESET;
	m_reset = true;

	install_chipmem_taps();

	// custom chip writes (by the cpu) get mirrored to internal ram
	// TODO: exact range (probably supports mirrors)
	m_custom_chip_tap = m_host->space().install_write_tap
	(
		0xdff000, 0xdff1ff,
		"custom_chip_w",
		[this] (offs_t offset, uint16_t &data, uint16_t mem_mask)
		{
			m_ram[(0xf000 | (offset & 0x1ff)) >> 1] = data;
		},
		&m_custom_chip_tap
	);

}

void action_replay_mk2_device::freeze_w(int state)
{
	LOG("freeze_w: %d\n", state);

	if (state)
	{
		m_status = STATUS_BUTTON;

		m_nmi_active = true;
		m_host->ipl7_w(1);
		m_host->ipl7_w(0);
	}
}

void action_replay_mk2_device::rst_w(int state)
{
	LOG("rst_w: %d\n", state);

	if (state == 0)
	{
		m_status = STATUS_RESET;
		m_reset = true;
	}
}

uint16_t action_replay_mk2_device::status_r(offs_t offset, uint16_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOG("status_r: %04x & %04x\n", m_status, mem_mask);

	return m_status;
}

void action_replay_mk2_device::mode_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// 1-  enable freeze on read of 0xbfe001
	// -0  enable freeze on write of 0xbfd100

	LOG("mode_w: %04x & %04x\n", data, mem_mask);

	m_mode = data & 0x03;
}

void action_replay_mk2_device::restore_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("restore_w: %04x & %04x\n", data, mem_mask);

	// map chip memory again
	m_host->ovr_w(1);

	// remapping chip memory has wiped out our tap, reinstall it
	install_chipmem_taps();
}

} // namespace bus::amiga::cpuslot

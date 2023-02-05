// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    powermacg3.cpp
    PowerMac G3 (original beige hardware)
    Preliminary driver by R. Belmont

    The last desktop Old World Mac, with hardware very similar to the first
    New World machines.

    CPU: PowerPC 750 "G3" @ 233 MHz
    Memory controller/PCI bridge: Motorola MPC106 "Grackle"
    Video: ATI Rage II+, ATI Rage Pro on rev. B, ATI Rage Pro Turbo on rev. C
    I/O: Heathrow PCI I/O ASIC (see heathrow.cpp for details)

****************************************************************************/

#include "emu.h"
#include "cpu/powerpc/ppc.h"
#include "machine/pci.h"
#include "machine/pci-ide.h"
#include "machine/mpc106.h"
#include "heathrow.h"

class pwrmacg3_state : public driver_device
{
public:
	void pwrmacg3(machine_config &config);

	pwrmacg3_state(const machine_config &mconfig, device_type type, const char *tag);

	required_device<cpu_device> m_maincpu;

private:
	void pwrmacg3_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;
};

pwrmacg3_state::pwrmacg3_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
{
}

void pwrmacg3_state::machine_start()
{
}

void pwrmacg3_state::machine_reset()
{
}

void pwrmacg3_state::pwrmacg3_map(address_map &map)
{
	map.unmap_value_high();
}

void pwrmacg3_state::pwrmacg3(machine_config &config)
{
	PPC604(config, m_maincpu, 66000000);    // actually PPC750
	m_maincpu->set_addrmap(AS_PROGRAM, &pwrmacg3_state::pwrmacg3_map);

	PCI_ROOT(config, "pci", 0);
	MPC106(config, "pci:00.0", 0, mpc106_host_device::MAP_TYPE_B, "maincpu", "bootrom", 32 * 1024 * 1024);

	heathrow_device &heathrow(HEATHROW(config, "pci:10.0", 0));
	heathrow.set_maincpu_tag("maincpu");
}

/*
    Config register for Gossamer beige G3 and all-in-one
    bit 15: 1 = SWIM3, 0 = PC style FDC
    bit 14: 1 = slow ROM, 0 = burstable ROM
    bit 12 = PCI slot C card present
    bit 10 = PCI slot B card present
    bit 8  = PCI slot A card present
    bits 7-5 = bus to CPU clock ratio (1 for 2:1)
    bit 4:  0 = all-in-one "Molar Mac", 1 = Desktop beige G3
    bits 3-1: bus speed (0=75 MHz, 1=70, 2=78.75, 3=invalid, 4=75, 5=60, 6=66.82, 7=83)
    bit 0:  must be 1 (burn-in diagnostics?)

    desktop = 0b1001010100111101;
    AIO     = 0b1001010100101101;
*/
ROM_START(pwrmacg3)
	ROM_REGION(0x1000000, "bootrom", ROMREGION_64BIT | ROMREGION_BE | ROMREGION_ERASEFF)
	ROM_LOAD( "pmacg3_79d68d63.bin", 0xc00000, 0x400000, CRC(74a3badf) SHA1(e7fc183f62addc6499350c727252d3348184955e) )

	// The Gossamer machine config register is at 0xFF000000, which is in the MPC106's ROM space.
	// So we're hacking it like this.  Hardware is assumed to operate similarly.
	ROM_FILL(0, 1, 0b10010101)
	ROM_FILL(1, 1, 0b00111101)
	ROM_FILL(2, 1, 0b10010101)
	ROM_FILL(3, 1, 0b00111101)
ROM_END

static INPUT_PORTS_START(pwrmacg3)
INPUT_PORTS_END

COMP(1997, pwrmacg3, 0, 0, pwrmacg3, pwrmacg3, pwrmacg3_state, empty_init, "Apple Computer", "Power Macintosh G3", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

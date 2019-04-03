// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/***************************************************************************

    Final Chess Card by TASC

    TODO:
    - skeleton, just boots the CPU

***************************************************************************/

#include "emu.h"
#include "finalchs.h"
#include "cpu/m6502/m65c02.h"

WRITE8_MEMBER( isa8_finalchs_device::io7ff8_write )
{
	m_FCH_latch_data = data;
}

READ8_MEMBER( isa8_finalchs_device::io7ff8_read )
{
	static unsigned char table[] = { 0xff, 0xfd, 0xfe };
	static int i = -1;
	i++;
	if (i == 3) i = 0;
	return table[i];  // exercise the NMI handler for now with known commands
}

READ8_MEMBER( isa8_finalchs_device::io6000_read )
{
	return 0x55;
}

WRITE8_MEMBER( isa8_finalchs_device::io6000_write )
{
	m_FCH_latch_data = data;
}

void isa8_finalchs_device::finalchs_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x7ff8, 0x7ff8).r(FUNC(isa8_finalchs_device::io7ff8_read));
	map(0x7ff8, 0x7ff8).w(FUNC(isa8_finalchs_device::io7ff8_write));
	map(0x6000, 0x6000).r(FUNC(isa8_finalchs_device::io6000_read));
	map(0x6000, 0x6000).w(FUNC(isa8_finalchs_device::io6000_write));
	map(0x8000, 0xffff).rom();
}

ROM_START(finalchs)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("finalchs.bin", 0x8000, 0x8000, CRC(c8e72dff) SHA1(f422b19a806cef4fadd580caefaaf8c32b644098))
ROM_END

READ8_MEMBER( isa8_finalchs_device::finalchs_r )
{
	return 0;
}

WRITE8_MEMBER( isa8_finalchs_device::finalchs_w )
{
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_FINALCHS, isa8_finalchs_device, "isa_finalchs", "Final Chess Card")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_finalchs_device::device_add_mconfig(machine_config &config)
{
	m65c02_device &cpu(M65C02(config, "maincpu", 5000000));
	cpu.set_addrmap(AS_PROGRAM, &isa8_finalchs_device::finalchs_mem);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_finalchs_device - constructor
//-------------------------------------------------

isa8_finalchs_device::isa8_finalchs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA8_FINALCHS, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_FCH_latch_data(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_finalchs_device::device_start()
{
	set_isa_device();
	//the included setup program allows any port from 0x100 to 0x1F0 to be selected, at increments of 0x10
	//picked the following at random until we get dips hooked up
	m_isa->install_device(0x160, 0x0161, read8_delegate(FUNC(isa8_finalchs_device::finalchs_r), this), write8_delegate(FUNC(isa8_finalchs_device::finalchs_w), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_finalchs_device::device_reset()
{
	m_FCH_latch_data = 0;
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_finalchs_device::device_rom_region() const
{
	return ROM_NAME( finalchs );
}

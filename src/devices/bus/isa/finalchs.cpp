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

static ADDRESS_MAP_START(finalchs_mem , AS_PROGRAM, 8, isa8_finalchs_device)
	AM_RANGE( 0x0000, 0x1fff ) AM_RAM
	AM_RANGE( 0x7ff8, 0x7ff8 ) AM_READ(io7ff8_read)
	AM_RANGE( 0x7ff8, 0x7ff8 ) AM_WRITE(io7ff8_write)
	AM_RANGE( 0x6000, 0x6000 ) AM_READ(io6000_read)
	AM_RANGE( 0x6000, 0x6000 ) AM_WRITE(io6000_write)
	AM_RANGE( 0x8000, 0xffff ) AM_ROM
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( finalchs_config )
	MCFG_CPU_ADD("maincpu",M65C02,5000000)
	MCFG_CPU_PROGRAM_MAP(finalchs_mem)
MACHINE_CONFIG_END

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

const device_type ISA8_FINALCHS = &device_creator<isa8_finalchs_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa8_finalchs_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( finalchs_config );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_finalchs_device - constructor
//-------------------------------------------------

isa8_finalchs_device::isa8_finalchs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, ISA8_FINALCHS, "Final Chess Card", tag, owner, clock, "finalchs", __FILE__),
		device_isa8_card_interface( mconfig, *this ), m_FCH_latch_data(0)
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
	m_isa->install_device(0x160, 0x0161, 0, 0, read8_delegate(FUNC(isa8_finalchs_device::finalchs_r), this), write8_delegate(FUNC(isa8_finalchs_device::finalchs_w), this));
//  timer_pulse(machine, ATTOTIME_IN_HZ(1), NULL, 0, cause_M6502_irq);
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

const rom_entry *isa8_finalchs_device::device_rom_region() const
{
	return ROM_NAME( finalchs );
}

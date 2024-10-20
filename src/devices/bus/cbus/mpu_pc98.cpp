// license:BSD-3-Clause
// copyright-holders:R. Belmont,Kevin Horton
/***************************************************************************

    MPU-401 MIDI device interface

***************************************************************************/

#include "emu.h"
#include "mpu_pc98.h"

#define MPU_CORE_TAG "mpu401"


/*
DIP-SWs
1-2-3-4
         0xc0d0
      1  0xc4d0
    1    0xc8d0
...
1        0xe0d0 (default)
...
1 1 1 1  0xfcd0

5-6-7-8
1        irq12
  1      irq6 (default)
    1    irq5
      1  irq3
*/

void mpu_pc98_device::mpu_irq_out(int state)
{
	m_bus->int_w<2>(state);
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MPU_PC98, mpu_pc98_device, "mpu_pc98", "Roland MPU-401 MIDI Interface (C-bus)")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mpu_pc98_device::device_add_mconfig(machine_config &config)
{
	MPU401(config, m_mpu401).irq_cb().set(FUNC(mpu_pc98_device::mpu_irq_out));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

mpu_pc98_device::mpu_pc98_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MPU_PC98, tag, owner, clock)
	, m_bus(*this, DEVICE_SELF_OWNER)
	, m_mpu401(*this, MPU_CORE_TAG)
{
}

void mpu_pc98_device::map(address_map &map)
{
	map(0x0, 0x3).rw(MPU_CORE_TAG, FUNC(mpu401_device::mpu_r), FUNC(mpu401_device::mpu_w)).umask16(0x00ff);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mpu_pc98_device::device_start()
{
	m_bus->io_space().install_device(0xe0d0, 0xe0d3, *this, &mpu_pc98_device::map);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mpu_pc98_device::device_reset()
{
}

// license:BSD-3-Clause
// copyright-holders:R. Belmont,Kevin Horton
/***************************************************************************

    MPU-401 MIDI device interface

    TODO:
    - skeleton, doesn't do anything

***************************************************************************/

#include "emu.h"
#include "mpu401.h"
#include "machine/pic8259.h"

#define MPU_CORE_TAG "mpu401"

MACHINE_CONFIG_FRAGMENT( isa8mpu401 )
	MCFG_MPU401_ADD(MPU_CORE_TAG, WRITELINE(isa8_mpu401_device, mpu_irq_out))
MACHINE_CONFIG_END

/*
DIP-SWs
1-2-3-4
         0x200
      1  0x210
    1    0x220
...
1   1 1  0x330 (default)
...
1 1 1 1  0x370

5-6-7-8
1        irq2 (default)
  1      irq3
    1    irq5
      1  irq7
*/

WRITE_LINE_MEMBER( isa8_mpu401_device::mpu_irq_out )
{
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_MPU401 = &device_creator<isa8_mpu401_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa8_mpu401_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( isa8mpu401 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_adlib_device - constructor
//-------------------------------------------------

isa8_mpu401_device::isa8_mpu401_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, ISA8_MPU401, "Roland MPU-401 MIDI Interface", tag, owner, clock, "isa_mpu401", __FILE__),
		device_isa8_card_interface( mconfig, *this ),
		m_mpu401(*this, MPU_CORE_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_mpu401_device::device_start()
{
	set_isa_device();

	m_isa->install_device(0x330, 0x0331, 0, 0, READ8_DEVICE_DELEGATE(m_mpu401, mpu401_device, mpu_r), WRITE8_DEVICE_DELEGATE(m_mpu401, mpu401_device, mpu_w));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_mpu401_device::device_reset()
{
}

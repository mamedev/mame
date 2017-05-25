// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * mach32.c
 *
 *  Implementation of the ATi mach32 and mach64 video chips
 *  Based on ati_vga and mach8
 *
 *  Created on: 16/05/2014
 */

#include "emu.h"
#include "mach32.h"

#include "screen.h"


DEFINE_DEVICE_TYPE(ATIMACH32,       mach32_device,       "mach32",       "ATi mach32")
DEFINE_DEVICE_TYPE(ATIMACH32_8514A, mach32_8514a_device, "mach32_8514a", "ATi mach32 (2D acceleration module)")
DEFINE_DEVICE_TYPE(ATIMACH64,       mach64_device,       "mach64",       "ATi mach64")
DEFINE_DEVICE_TYPE(ATIMACH64_8514A, mach64_8514a_device, "mach64_8514a", "ATi mach64 (2D acceleration module)")


/*
 *  mach32
 */

// 8514/A device
mach32_8514a_device::mach32_8514a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mach32_8514a_device(mconfig, ATIMACH32_8514A, tag, owner, clock)
{
}

mach32_8514a_device::mach32_8514a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: mach8_device(mconfig, type, tag, owner, clock), m_chip_ID(0), m_membounds(0)
{
}


// SVGA device
mach32_device::mach32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mach32_device(mconfig, ATIMACH32, tag, owner, clock)
{
}

mach32_device::mach32_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: ati_vga_device(mconfig, type, tag, owner, clock), m_8514a(*this,"8514a")
{
}

static MACHINE_CONFIG_START( mach32_8514a )
	MCFG_DEVICE_ADD("8514a", ATIMACH32_8514A, 0)
	MCFG_EEPROM_SERIAL_93C56_ADD("ati_eeprom")
MACHINE_CONFIG_END

machine_config_constructor mach32_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mach32_8514a );
}

void mach32_8514a_device::device_config_complete()
{
	m_vga = dynamic_cast<vga_device*>(owner());
}

void mach32_8514a_device::device_start()
{
	mach8_device::device_start();
	//    017h  68800-AX
	//    177h  68800-LX
	//    2F7h  68800-6
	//  The 68800-3 appears to return 0 for this field (undocumented)
	m_chip_ID = 0x000;
	m_membounds = 0;
}

void mach32_8514a_device::device_reset()
{
}

void mach32_device::device_start()
{
	ati_vga_device::device_start();
}

void mach32_device::device_reset()
{
	ati_vga_device::device_reset();
}

/*
 *   mach64
 */

// 8514/A device
mach64_8514a_device::mach64_8514a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mach64_8514a_device(mconfig, ATIMACH64_8514A, tag, owner, clock)
{
}

mach64_8514a_device::mach64_8514a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: mach32_8514a_device(mconfig, type, tag, owner, clock)
{
}


// SVGA device
mach64_device::mach64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mach64_device(mconfig, ATIMACH64, tag, owner, clock)
{
}

mach64_device::mach64_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: mach32_device(mconfig, type, tag, owner, clock), m_8514a(*this, "8514a")
{
}

static MACHINE_CONFIG_START( mach64_8514a )
	MCFG_DEVICE_ADD("8514a", ATIMACH64_8514A, 0)
	MCFG_EEPROM_SERIAL_93C56_ADD("ati_eeprom")
MACHINE_CONFIG_END

machine_config_constructor mach64_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mach64_8514a );
}

void mach64_8514a_device::device_config_complete()
{
	m_vga = dynamic_cast<vga_device*>(owner());
}

void mach64_8514a_device::device_start()
{
	mach32_8514a_device::device_start();
	//    017h  68800-AX
	//    177h  68800-LX
	//    2F7h  68800-6
	//  The 68800-3 appears to return 0 for this field (undocumented)
	m_chip_ID = 0x0000;  // value is unknown for mach64
	m_membounds = 0;
}

void mach64_8514a_device::device_reset()
{
}

void mach64_device::device_start()
{
	mach32_device::device_start();
}

void mach64_device::device_reset()
{
	mach32_device::device_reset();
}

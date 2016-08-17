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

#include "mach32.h"

const device_type ATIMACH32 = &device_creator<mach32_device>;
const device_type ATIMACH32_8514A = &device_creator<mach32_8514a_device>;
const device_type ATIMACH64 = &device_creator<mach64_device>;
const device_type ATIMACH64_8514A = &device_creator<mach64_8514a_device>;


/*
 *  mach32
 */

// 8514/A device
mach32_8514a_device::mach32_8514a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: mach8_device(mconfig, ATIMACH32_8514A, "ATi mach32 (2D acceleration module)", tag, owner, clock, "mach32_8514a", __FILE__),
	m_chip_ID(0),
	m_membounds(0)
{
}

mach32_8514a_device::mach32_8514a_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: mach8_device(mconfig, type, name, tag, owner, clock, shortname, source),
	m_chip_ID(0),
	m_membounds(0)
{
}


// SVGA device
mach32_device::mach32_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ati_vga_device(mconfig, ATIMACH32, "ATi mach32", tag, owner, clock, "mach32", __FILE__),
		m_8514a(*this,"8514a")
{
}

mach32_device::mach32_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: ati_vga_device(mconfig, type, name, tag, owner, clock, shortname, source),
		m_8514a(*this,"8514a")
{
}

static MACHINE_CONFIG_FRAGMENT( mach32_8514a )
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
mach64_8514a_device::mach64_8514a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: mach32_8514a_device(mconfig, ATIMACH64_8514A, "ATi mach64 (2D acceleration module)", tag, owner, clock, "mach64_8514a", __FILE__)
{
}

mach64_8514a_device::mach64_8514a_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: mach32_8514a_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}


// SVGA device
mach64_device::mach64_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: mach32_device(mconfig, ATIMACH64, "ATi mach64", tag, owner, clock, "mach64", __FILE__),
		m_8514a(*this,"8514a")
{
}

mach64_device::mach64_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: mach32_device(mconfig, type, name, tag, owner, clock, shortname, source),
		m_8514a(*this,"8514a")
{
}

static MACHINE_CONFIG_FRAGMENT( mach64_8514a )
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

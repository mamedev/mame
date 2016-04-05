// license:BSD-3-Clause
// copyright-holders:MetalliC

/*

MODEL3 COMMUNICATION BOARD
837-11861  171-7053B
SEGA 1995
|---------------------------------------------------------------------------------------------------|
|                                                                                                   |
|   LATTICE                     NKK N341256SJ-20     NKK N341256SJ-20     NKK N341256SJ-20    40MHz |
|   PLSI 2032   JP3                                                                      JP1        |
|   (315-5958)  JP4             NKK N341256SJ-20     NKK N341256SJ-20     NKK N341256SJ-20          |
|                                                                                             JP2   |
|                                                                                                   |
|   PALCE16V8                                                                                       |
|   315-6075       68000FN12       315-5804             315-5917              315-5917              |
|                                  (QFP144)             (QFP80)               (QFP80)               |
|   PALCE16V8                                                                                       |
|   315-6074                                                                                        |
|  LEDx7                                                                                            |
|---------------------------------------------------------------------------------------------------|
JP1: 1-2
JP2: 2-3
JP3: not shorted
JP4: shorted


HIKARU COMMUNICATION BOARD
837-13404  171-7641B
|-----------------------------------|
| 40MHz      LATTICE     315-5917   |
|            PLSI2032               |
|            (315-5958A)    315-5917|
|   PAL                             |
|                          62256    |
|             315-5804     62256    |
|                          62256    |
|                          62256    |
|  68000          PAL               |
|  62256                            |
|  62256          PAL               |
|-----------------------------------|
Notes:
      62256 - 32k x8 SRAM


NAOMI COMMUNICATION BOARD
840-0001E
837-13489  171-7704B
SEGA 1998
|--------------------------------------------------------------------|
|         CN1                                  CN3                   |
| 40MHz        256KbSRAM  315-5917    256KbSRAM  315-5917  256KbSRAM |
|              256KbSRAM  (QFP80)     256KbSRAM  (QFP80)   256KbSRAM |
|                                                                    |
|                                                                    |
|    315-5804        68000FN12        LATTICE                        |
|    (QFP144)                         M5-128/104                     |
|                                     12YC/1-15YI/1                  |
|                                     (315-6194A)                    |
|          CN2                                                       |
|--------------------------------------------------------------------|

*/


#include "emu.h"
#include "emuopts.h"
#include "machine/m3comm.h"

#define M68K_TAG     "m3commcpu"

/*************************************
 *  M3COMM Memory Map
 *************************************/
static ADDRESS_MAP_START( m3comm_mem, AS_PROGRAM, 16, m3comm_device )
	AM_RANGE(0x0000000, 0x000ffff) AM_RAM AM_SHARE("m68k_ram")
	AM_RANGE(0x0080000, 0x008ffff) AM_RAMBANK("comm_ram")
	AM_RANGE(0x0040000, 0x0040001) AM_READWRITE(commbank_r, commbank_w)
	AM_RANGE(0x00C0088, 0x00C0089) AM_READWRITE(status0_r, status0_w)
	AM_RANGE(0x00C008A, 0x00C008B) AM_READWRITE(status1_r, status1_w)
ADDRESS_MAP_END


MACHINE_CONFIG_FRAGMENT( m3comm )
	MCFG_CPU_ADD(M68K_TAG, M68000, 10000000) // random
	MCFG_CPU_PROGRAM_MAP(m3comm_mem)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
MACHINE_CONFIG_END


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type M3COMM = &device_creator<m3comm_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor m3comm_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( m3comm );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m3comm_device - constructor
//-------------------------------------------------

m3comm_device::m3comm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, M3COMM, "MODEL-3 COMMUNICATION BD", tag, owner, clock, "m3comm", __FILE__),
	m68k_ram(*this, "m68k_ram"),
	m_commcpu(*this, M68K_TAG),
	m_ram(*this, RAM_TAG),
	m_line_rx(OPEN_FLAG_WRITE | OPEN_FLAG_CREATE ),
	m_line_tx(OPEN_FLAG_READ)
{
	// prepare localhost "filename"
	m_localhost[0] = 0;
	strcat(m_localhost, "socket.");
	strcat(m_localhost, mconfig.options().comm_localhost());
	strcat(m_localhost, ":");
	strcat(m_localhost, mconfig.options().comm_localport());

	// prepare remotehost "filename"
	m_remotehost[0] = 0;
	strcat(m_remotehost, "socket.");
	strcat(m_remotehost, mconfig.options().comm_remotehost());
	strcat(m_remotehost, ":");
	strcat(m_remotehost, mconfig.options().comm_remoteport());
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m3comm_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m3comm_device::device_reset()
{
	m_control = 0xC000;
	m_offset = 0;
	m_status0 = 0;
	m_status1 = 0;
	m_commbank = 0;
	membank("comm_ram")->set_base(m_ram->pointer());
}

void m3comm_device::device_reset_after_children()
{
	m_commcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}


READ16_MEMBER(m3comm_device::commbank_r)
{
	return m_commbank;
}
WRITE16_MEMBER(m3comm_device::commbank_w)
{
	m_commbank = data;
	membank("comm_ram")->set_base(m_ram->pointer() + (m_commbank ? 0x10000 : 0));
}
READ16_MEMBER(m3comm_device::status0_r)
{
	return m_status0;
}
WRITE16_MEMBER(m3comm_device::status0_w)
{
	m_status0 = data;
}
READ16_MEMBER(m3comm_device::status1_r)
{
	return m_status1;
}
WRITE16_MEMBER(m3comm_device::status1_w)
{
	m_status1 = data;
}

READ16_MEMBER(m3comm_device::naomi_r)
{
	switch (offset)
	{
	case 0:			// 5F7018
		return m_control;
	case 1:			// 5F701C
		return m_offset;
	case 2:			// 5F7020
	{
		logerror("M3COMM read @ %08x\n", (m_control << 16) | m_offset);
		UINT16 value;
		if (m_control & 1)
			value = m68k_ram[m_offset / 2];
		else {
			UINT16 *commram = (UINT16*)membank("comm_ram")->base();

			value = commram[m_offset / 2];
		}
		m_offset+= 2;
		return value;
	}
	case 3:			// 5F7024
		return m_status0;
	case 4:			// 5F7028
		return m_status1;
	default:
		return 0;
	}
}

WRITE16_MEMBER(m3comm_device::naomi_w)
{
	switch (offset)
	{
	case 0:			// 5F7018
		logerror("M3COMM control write %04x\n", data);
		m_control = data;
		m_commcpu->set_input_line(INPUT_LINE_RESET, (m_control & 0x20) ? CLEAR_LINE : ASSERT_LINE);
		break;
	case 1:			// 5F701C
		m_offset = data;
		break;
	case 2:			// 5F7020
		logerror("M3COMM write @ %08x %04x\n", (m_control << 16) | m_offset, data);
		if (m_control & 1)
			m68k_ram[m_offset / 2] = data;
		else {
			UINT16 *commram = (UINT16*)membank("comm_ram")->base();
			commram[m_offset / 2] = data;
		}
		m_offset += 2;
		break;
	case 3:			// 5F7024
		m_status0 = data;
		break;
	case 4:			// 5F7028
		m_status1 = data;
		break;
	}
}

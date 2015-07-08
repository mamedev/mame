// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Template for skeleton device

***************************************************************************/

#include "emu.h"
#include "machine/segacdblock.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

READ16_MEMBER(segacdblock_device::hirq_r){	return m_hirq; }
WRITE16_MEMBER(segacdblock_device::hirq_w){	COMBINE_DATA(&m_hirq); }

READ16_MEMBER(segacdblock_device::hirq_mask_r){	return m_hirq_mask; }
WRITE16_MEMBER(segacdblock_device::hirq_mask_w) {	COMBINE_DATA(&m_hirq_mask); }

READ16_MEMBER(segacdblock_device::cr0_r){	return m_cr[0]; }
WRITE16_MEMBER(segacdblock_device::cr0_w){ COMBINE_DATA(&m_cr[0]);}

READ16_MEMBER(segacdblock_device::cr1_r){	return m_cr[1]; }
WRITE16_MEMBER(segacdblock_device::cr1_w){ COMBINE_DATA(&m_cr[1]);}

READ16_MEMBER(segacdblock_device::cr2_r){	return m_cr[2]; }
WRITE16_MEMBER(segacdblock_device::cr2_w){ COMBINE_DATA(&m_cr[2]);}

READ16_MEMBER(segacdblock_device::cr3_r){	return m_cr[3]; }
WRITE16_MEMBER(segacdblock_device::cr3_w){ COMBINE_DATA(&m_cr[3]);}


// device type definition
const device_type SEGACDBLOCK = &device_creator<segacdblock_device>;

/*
 	DTR 	Data Transfer Register
0x25890008 	HIRQ 	Interrupt Status Register
0x2589000C 	HIRQ Mask 	Interrupt Mask Register
0x25890018 	CR1 	Command Register 1
0x2589001C 	CR2 	Command Register 2
0x25890020 	CR3 	Command Register 3
0x25890024 	CR4 	Command Register 4
0x25890028 	MPEGRGB 	MPEG RGB Data Transfer Register 
*/
static ADDRESS_MAP_START( map, AS_0, 32, segacdblock_device )
	AM_RANGE(0x08, 0x0b) AM_READWRITE16(hirq_r,hirq_w,0xffffffff)
	AM_RANGE(0x0c, 0x0f) AM_READWRITE16(hirq_mask_r,hirq_mask_w,0xffffffff)
	AM_RANGE(0x18, 0x1b) AM_READWRITE16(cr0_r,cr0_w,0xffffffff)
	AM_RANGE(0x1c, 0x1f) AM_READWRITE16(cr1_r,cr1_w,0xffffffff)
	AM_RANGE(0x20, 0x23) AM_READWRITE16(cr2_r,cr2_w,0xffffffff)
	AM_RANGE(0x24, 0x27) AM_READWRITE16(cr3_r,cr3_w,0xffffffff)

ADDRESS_MAP_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  segacdblock_device - constructor
//-------------------------------------------------

segacdblock_device::segacdblock_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEGACDBLOCK, "Sega Saturn CD-Block (HLE)", tag, owner, clock, "segacdblock", __FILE__),
		device_memory_interface(mconfig, *this),
		m_space_config("segacdblock", ENDIANNESS_BIG, 32,6, 0, NULL, *ADDRESS_MAP_NAME(map))
{
}


const address_space_config *segacdblock_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void segacdblock_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void segacdblock_device::device_start()
{
	m_space = &space(AS_0);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void segacdblock_device::device_reset()
{
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ32_MEMBER( segacdblock_device::read )
{
	return m_space->read_dword(offset+0x05800000);
}

WRITE32_MEMBER( segacdblock_device::write )
{
	m_space->write_dword(offset+0x05800000,data);
}

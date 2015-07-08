// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

Template for skeleton device

***************************************************************************/

#include "emu.h"
#include "machine/segacdblock.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// HIRQ definitions
#define CMOK 0x0001 // command dispatch possible
#define DRDY 0x0002 // data transfer preparations complete
#define CSCT 0x0004 // finished reading 1 sector
#define BFUL 0x0008 // CD buffer full
#define PEND 0x0010 // CD playback completed
#define DCHG 0x0020 // disc change / tray open
#define ESEL 0x0040 // selector settings processing complete
#define EHST 0x0080 // host input/output processing complete
#define ECPY 0x0100 // duplication/move processing complete
#define EFLS 0x0200 // file system processing complete
#define SCDQ 0x0400 // subcode Q update completed
#define MPED 0x0800 // MPEG-related processing complete
#define MPCM 0x1000 // MPEG action uncertain
#define MPST 0x2000 // MPEG interrupt status report

// CD status (hi byte of CR1) definitions:
// (these defines are shifted up 8)
#define CD_STAT_BUSY     0x0000     // status change in progress
#define CD_STAT_PAUSE    0x0100     // CD block paused (temporary stop)
#define CD_STAT_STANDBY  0x0200     // CD drive stopped
#define CD_STAT_PLAY     0x0300     // CD play in progress
#define CD_STAT_SEEK     0x0400     // drive seeking
#define CD_STAT_SCAN     0x0500     // drive scanning
#define CD_STAT_OPEN     0x0600     // tray is open
#define CD_STAT_NODISC   0x0700     // no disc present
#define CD_STAT_RETRY    0x0800     // read retry in progress
#define CD_STAT_ERROR    0x0900     // read data error occurred
#define CD_STAT_FATAL    0x0a00     // fatal error (hard reset required)
#define CD_STAT_PERI     0x2000     // periodic response if set, else command response
#define CD_STAT_TRANS    0x4000     // data transfer request if set
#define CD_STAT_WAIT     0x8000     // waiting for command if set, else executed immediately
#define CD_STAT_REJECT   0xff00     // ultra-fatal error.


void segacdblock_device::sh1_writes_registers(UINT16 r1, UINT16 r2, UINT16 r3, UINT16 r4)
{
	m_cr[0] = r1;
	m_cr[1] = r2;
	m_cr[2] = r3;
	m_cr[3] = r4;
}

void segacdblock_device::set_flag(UINT16 which)
{
	m_hirq |= which;
}

void segacdblock_device::clear_flag(UINT16 which)
{
	m_hirq &= ~which;
}

READ16_MEMBER(segacdblock_device::hirq_r){	return m_hirq; }
WRITE16_MEMBER(segacdblock_device::hirq_w)
{
	COMBINE_DATA(&m_hirq);
	 
	if(m_hirq & CMOK) /**< @todo needs fucntion irq_mask too */
	{
		m_hs = true;

		clear_flag(CMOK);
		sh1_writes_registers(CD_STAT_BUSY,0,0,0);
	}
}

READ16_MEMBER(segacdblock_device::hirq_mask_r){	return m_hirq_mask; }
WRITE16_MEMBER(segacdblock_device::hirq_mask_w) { COMBINE_DATA(&m_hirq_mask); }

// 0x6001fd8 cd-block string check
READ16_MEMBER(segacdblock_device::cr0_r){ return m_cr[0]; }
WRITE16_MEMBER(segacdblock_device::cr0_w){ printf("%04x 0\n",data); COMBINE_DATA(&m_cr[0]);}

READ16_MEMBER(segacdblock_device::cr1_r){ return m_cr[1]; }
WRITE16_MEMBER(segacdblock_device::cr1_w){ printf("%04x 1\n",data); COMBINE_DATA(&m_cr[1]);}

READ16_MEMBER(segacdblock_device::cr2_r){ return m_cr[2]; }
WRITE16_MEMBER(segacdblock_device::cr2_w){ printf("%04x 2\n",data); COMBINE_DATA(&m_cr[2]);}

READ16_MEMBER(segacdblock_device::cr3_r){ return m_cr[3]; }
WRITE16_MEMBER(segacdblock_device::cr3_w){ printf("%04x 3\n",data); COMBINE_DATA(&m_cr[3]);}


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
	AM_RANGE(0x08, 0x0b) AM_MIRROR(0xf000) AM_READWRITE16(hirq_r,hirq_w,0xffffffff)
	AM_RANGE(0x0c, 0x0f) AM_MIRROR(0xf000) AM_READWRITE16(hirq_mask_r,hirq_mask_w,0xffffffff)
	AM_RANGE(0x18, 0x1b) AM_MIRROR(0xf000) AM_READWRITE16(cr0_r,cr0_w,0xffffffff)
	AM_RANGE(0x1c, 0x1f) AM_MIRROR(0xf000) AM_READWRITE16(cr1_r,cr1_w,0xffffffff)
	AM_RANGE(0x20, 0x23) AM_MIRROR(0xf000) AM_READWRITE16(cr2_r,cr2_w,0xffffffff)
	AM_RANGE(0x24, 0x27) AM_MIRROR(0xf000) AM_READWRITE16(cr3_r,cr3_w,0xffffffff)
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
		m_space_config("segacdblock", ENDIANNESS_BIG, 32,16, 0, NULL, *ADDRESS_MAP_NAME(map))
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
	// init_command_regs
	m_cr[0] = 'C';
	m_cr[1] = 'D' << 8 | 'B';
	m_cr[2] = 'L' << 8 | 'O';
	m_cr[3] = 'C' << 8 | 'K';
	m_hs = false;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ32_MEMBER( segacdblock_device::read )
{
	return m_space->read_dword(offset*4);
}

WRITE32_MEMBER( segacdblock_device::write )
{
	m_space->write_dword(offset*4,data);
}

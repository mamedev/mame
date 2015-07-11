// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

Template for skeleton device

4c68
3bba reject routine -> 0xf8 r14
3bbc peri routine -> 0 r14
33cc open, nodisc, fatal -> r0
***************************************************************************/

#include "emu.h"
#include "machine/segacdblock.h"
#include "debugger.h"


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
// 0xbe1: clears DRDY, CSCT, BFUL, PEND,

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

void segacdblock_device::write_fad()
{
	m_cr[3] = 1 | m_fad >> 16; /**< @todo track number */
	m_cr[4] = m_fad & 0xffff;
}

void segacdblock_device::set_flag(UINT16 which)
{
	m_hirq |= which;
}

/*!
 @todo: wrong and not needed
 */
void segacdblock_device::clear_flag(UINT16 which)
{
	m_hirq &= ~which;
}

UINT16 segacdblock_device::read_cd_state()
{
	return m_cr[0] & 0xff00;
}

void segacdblock_device::write_cd_state(UINT16 which)
{
	m_cr[0] = (m_cr[0] & 0xff) | which;
}


READ16_MEMBER(segacdblock_device::hirq_r){	return m_hirq; }
WRITE16_MEMBER(segacdblock_device::hirq_w)
{
	m_hirq &= data;

	if((m_hirq & CMOK) == 0) /**< @todo needs fucntion irq_mask too */
	{
		//sh1_writes_registers(CD_STAT_BUSY,0,0,0); /**< @todo it's of course faster than 150 Hz, but how much? */
		//m_cd_timer->adjust(attotime::from_hz(clock()));
		//debugger_break(machine());
	}
}

READ16_MEMBER(segacdblock_device::hirq_mask_r){	return m_hirq_mask; }
WRITE16_MEMBER(segacdblock_device::hirq_mask_w) { COMBINE_DATA(&m_hirq_mask); }

// 0x6001fd8 cd-block string check
READ16_MEMBER(segacdblock_device::cr0_r){ return m_dr[0]; }
WRITE16_MEMBER(segacdblock_device::cr0_w){ m_cmd_issued |= 1; COMBINE_DATA(&m_cr[0]);}

READ16_MEMBER(segacdblock_device::cr1_r){ return m_dr[1]; }
WRITE16_MEMBER(segacdblock_device::cr1_w){ m_cmd_issued |= 2; COMBINE_DATA(&m_cr[1]);}

READ16_MEMBER(segacdblock_device::cr2_r){ return m_dr[2]; }
WRITE16_MEMBER(segacdblock_device::cr2_w){ m_cmd_issued |= 4; COMBINE_DATA(&m_cr[2]);}

READ16_MEMBER(segacdblock_device::cr3_r){ return m_dr[3]; }
WRITE16_MEMBER(segacdblock_device::cr3_w){ m_cmd_issued |= 8; COMBINE_DATA(&m_cr[3]);}


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

void segacdblock_device::cd_cmd_status()
{
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_get_hw_info()
{
	m_dr[0] = CD_STAT_NODISC;
	m_dr[1] = 0x0201;
	m_dr[2] = 0x0000;
	m_dr[3] = 0x0400;
	set_flag(CMOK);
}


void segacdblock_device::cd_cmd_init(UINT8 init_flags)
{
	m_dr[0] = CD_STAT_NODISC;
	m_dr[1] = 0x0000;
	m_dr[2] = 0x0000;
	m_dr[3] = 0x0000;

	if(init_flags & 1)
		set_flag(ESEL|EHST|ECPY|EFLS|SCDQ);
//	sh1_writes_registers(CD_STAT_NODISC,0,0,0);
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_end_transfer()
{
	m_dr[0] = CD_STAT_NODISC | 0xff;
	m_dr[1] = 0xffff;
	m_dr[2] = 0;
	m_dr[3] = 0;
	// EHST
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_get_copy_error()
{
	m_dr[0] = CD_STAT_NODISC | 0;
	m_dr[1] = 0;
	m_dr[2] = 0;
	m_dr[3] = 0;
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_abort()
{
	set_flag(EFLS);
	// ...
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_auth_device(bool isMPEGauth)
{
	if(isMPEGauth == true)
		set_flag(MPED);
	else
		set_flag(EFLS|CSCT);

	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_device_auth_status(bool isMPEGauth)
{
	if(isMPEGauth == true)
	{
		m_dr[0] = CD_STAT_NODISC | 0;
		m_dr[1] = 2; /**< @todo: check if card present */
		m_dr[2] = 0;
		m_dr[3] = 0;
	}
	else
	{
		m_dr[0] = CD_STAT_NODISC | 0;
		m_dr[1] = 4; /**< @todo: various auth states */
		m_dr[2] = 0;
		m_dr[3] = 0;
	}
	set_flag(CMOK);
}

void segacdblock_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	assert(id == SH1_TIMER);

	m_sh1_ticks ++;
	//m_sh1_ticks &= 0xff;
	//if(m_sh1_ticks == 0)

	if(m_sh1_inited == false)
	{
		if(m_sh1_ticks >= 0x4000)
		{
			m_sh1_inited = true;
			set_flag(0xffff);
		}
	}
	else
	{
		if((m_sh1_ticks & 0xff) == 0)
		{
			m_dr[0] = CD_STAT_NODISC;
			m_dr[1] = 0x0000;
			m_dr[2] = 0x0000;
			m_dr[3] = 0x0000;
			set_flag(SCDQ);

			if(m_cmd_issued == 0xf)
			{
				switch(m_cr[0] >> 8)
				{
					case 0x00:
						cd_cmd_status();
						break;
					case 0x01:
						cd_cmd_get_hw_info();
						break;
					case 0x04:
						cd_cmd_init(m_cr[0] & 0xff);
						break;
					case 0x06:
						cd_cmd_end_transfer();
						break;
					case 0x67:
						cd_cmd_get_copy_error();
						break;
					case 0x75:
						cd_cmd_abort();
						break;
					case 0xe0:
						if(m_cr[1] > 1)
							printf("%04x auth command\n",m_cr[1]);
						cd_cmd_auth_device(m_cr[1] == 1);
						break;
					case 0xe1:
						if(m_cr[1] > 1)
							printf("%04x auth command\n",m_cr[1]);
						cd_cmd_device_auth_status(m_cr[1] == 1);
						break;
					default:
						printf("%04x %04x %04x %04x\n",m_cr[0],m_cr[1],m_cr[2],m_cr[3]);
				}
				m_cmd_issued = 0;
			}
		}
	}

#if 0
	if(m_hirq & CMOK)
		return;

	if(read_cd_state() & CD_STAT_PERI) // waiting for command
	{
		//write_cd_state(CD_STAT_PERI); /**< @todo command */
		//write_fad();
		m_sh1_timer->adjust(attotime::from_hz(clock()));
		return;
	}

	if(read_cd_state() == CD_STAT_BUSY)
	{
		switch(m_cr[0] & 0xff)
		{
			case 0x00:
				sh1_writes_registers(CD_STAT_NODISC,0,0,0);
				break;
			case 0x04:
				cmd_init();
				break;
		}

		set_flag(CMOK);
		//write_fad();
		//m_cd_timer->adjust(attotime::from_hz(clock()));
		return;
	}
#endif
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void segacdblock_device::device_start()
{
	m_space = &space(AS_0);
	m_sh1_timer = timer_alloc(SH1_TIMER);

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void segacdblock_device::device_reset()
{
	// init_command_regs
	m_dr[0] = 'C';
	m_dr[1] = 'D' << 8 | 'B';
	m_dr[2] = 'L' << 8 | 'O';
	m_dr[3] = 'C' << 8 | 'K';
	m_fad = 150;
	m_sh1_timer->reset();
	m_sh1_timer->adjust(attotime::from_hz(clock()*256), 0, attotime::from_hz(clock()*256));
	m_sh1_ticks = 0;
	m_sh1_inited = false;
	m_cmd_issued = 0;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ32_MEMBER( segacdblock_device::read )
{
	return m_space->read_word(offset*4)|(m_space->read_word(offset*4)<<16);
}

WRITE32_MEMBER( segacdblock_device::write )
{
	if(mem_mask == 0xffff0000)
		m_space->write_word(offset*4,data>>16);
}

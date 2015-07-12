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
#include "imagedev/chd_cd.h"
#include "cdrom.h"
#include "sound/cdda.h"

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

// device type definition
const device_type SEGACDBLOCK = &device_creator<segacdblock_device>;


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

READ16_MEMBER(segacdblock_device::datatrns_r)
{
	UINT16 res;

	res = 0xffff;

	switch(xfertype)
	{
		case XFERTYPE_TOC:
			res = tocbuf[m_dma_src]<<8 | tocbuf[m_dma_src+1];
			m_dma_src += 2;

			//printf("%04x\n",res);

			if (m_dma_src > m_dma_size)
			{
				m_dma_src = 0;
				xfertype = XFERTYPE_INVALID;
			}
			break;
		default:
			debugger_break(machine());
			break;
	}

	return res;
}


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
	AM_RANGE(0x00, 0x03) AM_MIRROR(0xf000) AM_READ16(datatrns_r,0xffffffff)
	AM_RANGE(0x08, 0x0b) AM_MIRROR(0xf000) AM_READWRITE16(hirq_r,hirq_w,0xffffffff)
	AM_RANGE(0x0c, 0x0f) AM_MIRROR(0xf000) AM_READWRITE16(hirq_mask_r,hirq_mask_w,0xffffffff)
	AM_RANGE(0x18, 0x1b) AM_MIRROR(0xf000) AM_READWRITE16(cr0_r,cr0_w,0xffffffff)
	AM_RANGE(0x1c, 0x1f) AM_MIRROR(0xf000) AM_READWRITE16(cr1_r,cr1_w,0xffffffff)
	AM_RANGE(0x20, 0x23) AM_MIRROR(0xf000) AM_READWRITE16(cr2_r,cr2_w,0xffffffff)
	AM_RANGE(0x24, 0x27) AM_MIRROR(0xf000) AM_READWRITE16(cr3_r,cr3_w,0xffffffff)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( segacdblock_config )
	MCFG_CDROM_ADD( "cdrom" )
	MCFG_CDROM_INTERFACE("sat_cdrom")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker") /**< @todo: reference to main sound routing? */
	MCFG_SOUND_ADD("cdda", CDDA, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

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

machine_config_constructor segacdblock_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( segacdblock_config );
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void segacdblock_device::device_validity_check(validity_checker &valid) const
{
}



void segacdblock_device::cd_standard_return(bool isPeri)
{
	m_dr[0] = (isPeri == true ? CD_STAT_PERI : 0) | m_cd_state;
	m_dr[1] = 0x0000;
	m_dr[2] = 0x0000;
	m_dr[3] = 150;
}


void segacdblock_device::cd_cmd_status()
{
	cd_standard_return(false);
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_get_hw_info()
{
	m_dr[0] = m_cd_state;
	m_dr[1] = 0x0201;
	m_dr[2] = 0x0000;
	m_dr[3] = 0x0400;
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_get_toc()
{
	m_TransferActive = true;
	m_dr[0] = CD_STAT_TRANS | m_cd_state;
	m_dr[1] = 102*2;
	m_dr[2] = 0;
	m_dr[3] = 0;
	m_TOCPhase = true;
	//set_flag(DRDY); // ...
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_get_session_info(UINT8 param)
{
	m_cd_state = CD_STAT_PAUSE;
	m_dr[0] = m_cd_state;
	m_dr[1] = 0;
	switch(param)
	{
		case 0: // lead out
			m_dr[2] = 0x0100 | ((m_DiscLeadOut >> 16) & 0xff);
			m_dr[3] = m_DiscLeadOut & 0xffff;
			break;

		case 1: // lead in
			m_dr[2] = 0x0100 | 0;
			m_dr[3] = 0;
			break;

		default: // unknown
			popmessage("Get Session Info used with param %02x, contact MAMEdev",param);
			m_dr[2] = 0;
			m_dr[3] = 0;
			break;
	}

	printf("Returns: %04x %04x %04x %04x\n",m_dr[0],m_dr[1],m_dr[2],m_dr[3]);
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_init(UINT8 init_flags)
{
	cd_standard_return(false);

	if(init_flags & 1)
		set_flag(ESEL|EHST|ECPY|EFLS|SCDQ);

	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_end_transfer()
{
	m_TransferActive = false;
	m_cd_state &= ~CD_STAT_TRANS;

	if(m_dma_size != 0)
	{
		m_dr[0] = m_cd_state | ((m_dma_size >> 17) & 0xff);
		m_dr[1] = (m_dma_size>>1) & 0xffff;
	}
	else
	{
		m_dr[0] = m_cd_state | 0xff;
		m_dr[1] = 0xffff;
	}
	m_dr[2] = 0;
	m_dr[3] = 0;


	m_dma_size = 0;
	// EHST
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_reset_selector()
{
	// ...
	cd_standard_return(false);

	set_flag(ESEL);
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_set_sector_length()
{
	// ...
	cd_standard_return(false);

	set_flag(ESEL);
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_get_copy_error()
{
	m_dr[0] = m_cd_state | 0;
	m_dr[1] = 0;
	m_dr[2] = 0;
	m_dr[3] = 0;
	set_flag(CMOK);
}


void segacdblock_device::cd_cmd_abort()
{
	cd_standard_return(false);

	set_flag(EFLS);
	// ...
	xfertype = XFERTYPE_INVALID;
	m_dma_size = 0;
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_auth_device(bool isMPEGauth)
{
	if(isMPEGauth == true)
	{
		//set_flag(MPED);
	}
	else
		set_flag(EFLS|CSCT);

	cd_standard_return(false);

	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_device_auth_status(bool isMPEGauth)
{
	if(isMPEGauth == true)
	{
		m_dr[0] = m_cd_state | 0;
		m_dr[1] = 0; /**< @todo: 2 if card present */
		m_dr[2] = 0;
		m_dr[3] = 0;
	}
	else
	{
		m_dr[0] = m_cd_state | 0;
		m_dr[1] = 4; /**< @todo: various auth states */
		m_dr[2] = 0;
		m_dr[3] = 0;
	}
	set_flag(CMOK);
}

void segacdblock_device::sh1_TOCRetrieve()
{
	int i, ntrks, tocptr, fad;

	//xfertype = XFERTYPE_TOC;
	m_dma_src = 0;
	m_dma_size = 102*4;

	if (cdrom)
		ntrks = cdrom_get_last_track(cdrom);
	else
		ntrks = 0;

	// data format for Saturn TOC:
	// no header.
	// 4 bytes per track
	// top nibble of first byte is CTRL info
	// low nibble is ADR
	// next 3 bytes are FAD address (LBA + 150)
	// there are always 99 track entries (0-98)
	// unused tracks are ffffffff.
	// entries 99-101 are metadata

	tocptr = 0; // starting point of toc entries

	for (i = 0; i < ntrks; i++)
	{
		if (cdrom)
		{
			//tocbuf[tocptr] = sega_cdrom_get_adr_control(cdrom, i);
			//HACK: ddsom does not enter ingame with the line above!
			tocbuf[tocptr] = cdrom_get_adr_control(cdrom, i)<<4 | 0x01;

			fad = cdrom_get_track_start(cdrom, i) + 150;
			tocbuf[tocptr+1] = (fad>>16)&0xff;
			tocbuf[tocptr+2] = (fad>>8)&0xff;
			tocbuf[tocptr+3] = fad&0xff;
		}
		else
		{
			tocbuf[tocptr] = 0xff;
			tocbuf[tocptr+1] = 0xff;
			tocbuf[tocptr+2] = 0xff;
			tocbuf[tocptr+3] = 0xff;
		}

		tocptr += 4;
	}

	// fill in the rest
	for ( ; i < 99; i++)
	{
		tocbuf[tocptr] = 0xff;
		tocbuf[tocptr+1] = 0xff;
		tocbuf[tocptr+2] = 0xff;
		tocbuf[tocptr+3] = 0xff;

		tocptr += 4;
	}

	// tracks 99-101 are special metadata
	// $$$FIXME: what to do with the address info for these?
	tocptr = 99 * 4;
	tocbuf[tocptr] = tocbuf[0]; // get ctrl/adr from first track
	tocbuf[tocptr+1] = 1;   // first track's track #
	tocbuf[tocptr+2] = 0;
	tocbuf[tocptr+3] = 0;

	tocbuf[tocptr+4] = tocbuf[(ntrks-1)*4]; // ditto for last track
	tocbuf[tocptr+5] = ntrks;   // last track's track #
	tocbuf[tocptr+6] = 0;
	tocbuf[tocptr+7] = 0;

	// get total disc length (start of lead-out)
	fad = cdrom_get_track_start(cdrom, 0xaa) + 150;

	m_DiscLeadOut = fad;
	tocbuf[tocptr+8] = tocbuf[0];
	tocbuf[tocptr+9]  = (fad>>16)&0xff;
	tocbuf[tocptr+10] = (fad>>8)&0xff;
	tocbuf[tocptr+11] = fad&0xff;

	xfertype = XFERTYPE_TOC;
	m_TOCPhase = false;
	set_flag(DRDY);
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

		}
	}
	else
	{
		if((m_sh1_ticks & 0xff) != 0)
		{
			if(m_TOCPhase == true)
				sh1_TOCRetrieve();

			if(m_isDiscInTray == true)
				m_cd_state = CD_STAT_PAUSE;
			else
				m_cd_state = CD_STAT_NODISC;

			if(m_TransferActive == true)
				m_cd_state|= CD_STAT_TRANS;

			cd_standard_return(true);
			set_flag(SCDQ);
		}
		else
		{
			if(m_cmd_issued == 0xf)
			{
				if(m_cr[0] != 0)
				printf("CD CMD: %04x %04x %04x %04x\n",m_cr[0],m_cr[1],m_cr[2],m_cr[3]);

				switch(m_cr[0] >> 8)
				{
					case 0x00:
						cd_cmd_status();
						break;

					case 0x01:
						cd_cmd_get_hw_info();
						break;

					case 0x02:
						cd_cmd_get_toc();
						break;

					case 0x03:
						cd_cmd_get_session_info(m_cr[0] & 0xff);
						break;

					case 0x04:
						cd_cmd_init(m_cr[0] & 0xff);
						break;

					case 0x06:
						cd_cmd_end_transfer();
						break;

					case 0x48:
						cd_cmd_reset_selector();
						break;

					case 0x60:
						cd_cmd_set_sector_length();
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
						printf("Unhandled %04x %04x %04x %04x\n",m_cr[0],m_cr[1],m_cr[2],m_cr[3]);
						//set_flag(CMOK);
				}
				m_cmd_issued = 0;
			}
		}
	}
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
	m_hirq = 0xffff;

	cdrom = subdevice<cdrom_image_device>("cdrom")->get_cdrom_file();
	m_isDiscInTray = cdrom != NULL;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ32_MEMBER( segacdblock_device::read )
{
	UINT32 res;

	res = 0;

	if(mem_mask == 0xffff0000)
		res|= m_space->read_word(offset*4)<<16;

	if(mem_mask == 0x0000ffff)
		res|= m_space->read_word(offset*4)<<0;

	if(mem_mask == 0xffffffff)
		debugger_break(machine());

	return res;
}

WRITE32_MEMBER( segacdblock_device::write )
{
	if(mem_mask == 0xffff0000)
		m_space->write_word(offset*4,data>>16);

	if(mem_mask == 0x0000ffff)
		m_space->write_word(offset*4,data);

	if(mem_mask == 0xffffffff)
		debugger_break(machine());
}

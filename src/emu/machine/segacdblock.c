// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont
/*!
@brief Sega Saturn CD-Block HLE device implementation
@discussion 
  (Yet) Another tilt at the windmill in 2015 by Angelo Salese, 
  based off old stvcd.c driver started in 2011 by R.Belmont.

  Not *yet* decapped SH-1 more or less executes this code.
  Information sources:
  - Tyranid's document
  - A commented disassembly I made of the Saturn BIOS's CD code
  - Yabuse's cs2.c
  - The ISO/IEC "Yellow Book" CD-ROM standard, 1995 version

  Address is mostly in terms of FAD (Frame ADdress).
  FAD is absolute number of frames from the start of the disc.
  In other words, FAD = LBA + 150; FAD is the same units as
  LBA except it counts starting at absolute zero instead of
  the first sector (00:02:00 in MSF format).
 
@see http://wiki.yabause.org/index.php5?title=CDBlock

@asmnotes
4c68
3bba reject routine -> 0xf8 r14
3bbc peri routine -> 0 r14
33cc open, nodisc, fatal -> r0

@todo
- (fill up this section once pull request these files);

*/

#define DUMP_DIR_ENTRIES 0

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
	m_cr[3] = 0x100 | m_FAD >> 16; /**< @todo track number */
	m_cr[4] = m_FAD & 0xffff;
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

void segacdblock_device::SH2SendsCommand()
{
	if(m_cmd_issued != 0xf)
		fatalerror("SH-2 attempted to write a command in-the-middle %04x\n",m_cmd_issued);
	
	m_cmd_timer->adjust(attotime::from_hz(16667));
	m_peri_timer->adjust(attotime::never);
}

// 0x6001fd8 cd-block string check
READ16_MEMBER(segacdblock_device::cr0_r){ return m_dr[0]; }
WRITE16_MEMBER(segacdblock_device::cr0_w){ m_cmd_issued |= 1; COMBINE_DATA(&m_cr[0]);}

READ16_MEMBER(segacdblock_device::cr1_r){ return m_dr[1]; }
WRITE16_MEMBER(segacdblock_device::cr1_w){ m_cmd_issued |= 2; COMBINE_DATA(&m_cr[1]);}

READ16_MEMBER(segacdblock_device::cr2_r){ return m_dr[2]; }
WRITE16_MEMBER(segacdblock_device::cr2_w){ m_cmd_issued |= 4; COMBINE_DATA(&m_cr[2]);}

READ16_MEMBER(segacdblock_device::cr3_r){ return m_dr[3]; }
WRITE16_MEMBER(segacdblock_device::cr3_w){ m_cmd_issued |= 8; COMBINE_DATA(&m_cr[3]); SH2SendsCommand(); }

void segacdblock_device::cd_defragblocks(partitionT *part)
{
	UINT32 i, j;
	blockT *temp;
	UINT8 temp2;

	for (i = 0; i < (MAX_BLOCKS-1); i++)
	{
		for (j = i+1; j < MAX_BLOCKS; j++)
		{
			if ((part->blocks[i] == (blockT *)NULL) && (part->blocks[j] != (blockT *)NULL))
			{
				temp = part->blocks[i];
				part->blocks[i] = part->blocks[j];
				part->blocks[j] = temp;

				temp2 = part->bnum[i];
				part->bnum[i] = part->bnum[j];
				part->bnum[j] = temp2;
			}
		}
	}
}

READ32_MEMBER(segacdblock_device::datatrns32_r)
{
	UINT32 res;
	
	res = -1;
	if(xfertype == CDDMA_INPROGRESS)
	{
		if (xfersect < xfersectnum)
		{
			// get next longword
			res = (transpart->blocks[xfersectpos+xfersect]->data[xferoffs + 0]<<24) |
				  (transpart->blocks[xfersectpos+xfersect]->data[xferoffs + 1]<<16) |
				  (transpart->blocks[xfersectpos+xfersect]->data[xferoffs + 2]<<8)  |
				  (transpart->blocks[xfersectpos+xfersect]->data[xferoffs + 3]<<0);

			m_dma_size += 4;
			xferoffs += 4;

			// did we run out of sector?
			if (xferoffs >= transpart->blocks[xfersect]->size)
			{
				//CDROM_LOG(("CD: finished xfer of block %d of %d\n", xfersect+1, xfersectnum))

				xferoffs = 0;
				xfersect++;
			}
		}
		else    // sectors are done, kill 'em all if we can
		{
			if (DeleteSectorMode == true)
			{
				INT32 i;

				//CDROM_LOG(("Killing sectors in done\n"))

				// deallocate the blocks
				for (i = xfersectpos; i < xfersectpos+xfersectnum; i++)
				{
					cd_free_block(transpart->blocks[i]);
					transpart->blocks[i] = (blockT *)NULL;
					transpart->bnum[i] = 0xff;
				}

				// defrag what's left
				cd_defragblocks(transpart);

				// clean up our state
				transpart->size -= m_dma_size;
				transpart->numblks -= xfersectnum;

				/* @todo check this */
				//xfertype = CDDMA_STOPPED;
			}
		}
	}
	
	return res;
}

READ16_MEMBER(segacdblock_device::datatrns_r)
{
	UINT16 res;

	res = 0xffff;

	if(xfertype == CDDMA_INPROGRESS)
	{
		res = m_DMABuffer[m_dma_src]<<8 | m_DMABuffer[m_dma_src+1]; // @todo Make this 16-bits
		m_dma_src += 2;
		if(m_dma_src > m_dma_size)
		{
			m_dma_src = 0;
			xfertype = CDDMA_STOPPED;
		}
	}
	else
		debugger_break(machine());

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
	AM_RANGE(0x18000, 0x18003) AM_READ(datatrns32_r)
	AM_RANGE(0x90000, 0x90003) AM_MIRROR(0xf000) AM_READ16(datatrns_r,0xffffffff)
	AM_RANGE(0x90008, 0x9000b) AM_MIRROR(0xf000) AM_READWRITE16(hirq_r,hirq_w,0xffffffff)
	AM_RANGE(0x9000c, 0x9000f) AM_MIRROR(0xf000) AM_READWRITE16(hirq_mask_r,hirq_mask_w,0xffffffff)
	AM_RANGE(0x90018, 0x9001b) AM_MIRROR(0xf000) AM_READWRITE16(cr0_r,cr0_w,0xffffffff)
	AM_RANGE(0x9001c, 0x9001f) AM_MIRROR(0xf000) AM_READWRITE16(cr1_r,cr1_w,0xffffffff)
	AM_RANGE(0x90020, 0x90023) AM_MIRROR(0xf000) AM_READWRITE16(cr2_r,cr2_w,0xffffffff)
	AM_RANGE(0x90024, 0x90027) AM_MIRROR(0xf000) AM_READWRITE16(cr3_r,cr3_w,0xffffffff)
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
		m_space_config("segacdblock", ENDIANNESS_BIG, 32,20, 0, NULL, *ADDRESS_MAP_NAME(map))
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

void segacdblock_device::SH1CommandExecute()
{
	if(m_cr[0] != 0)
		printf("CD CMD: %04x %04x %04x %04x\n",m_cr[0],m_cr[1],m_cr[2],m_cr[3]);

	
	switch(m_cr[0] >> 8)
	{
		case 0x00:	cd_cmd_status(); break;
		case 0x01: 	cd_cmd_get_hw_info(); break;
		case 0x02:	cd_cmd_get_toc(); break;
		case 0x03:	cd_cmd_get_session_info(m_cr[0] & 0xff); break;
		case 0x04:	cd_cmd_init(m_cr[0] & 0xff); break;
		case 0x06:	cd_cmd_end_transfer(); break;
		
		case 0x10:  cd_cmd_play_disc(); break;

		case 0x30:	cd_cmd_set_device_connection(m_cr[2] >> 8); break;
		
		case 0x48:	cd_cmd_reset_selector(m_cr[0] & 0xff, m_cr[2] >> 8); break;

		case 0x51:  cd_cmd_get_sector_number(m_cr[2] >> 8); break;
		
		case 0x60:  cd_cmd_set_sector_length(m_cr[0] & 0xff, m_cr[1] >> 8); break;
		case 0x63:  cd_cmd_get_then_delete_sector(); break;
		case 0x67:	cd_cmd_get_copy_error(); break;

		case 0x70:	cd_cmd_change_dir(((m_cr[2] & 0xff)<<16) | (m_cr[3] & 0xffff) ); break;
		case 0x72:	cd_cmd_get_file_system_scope(); break;
		case 0x74:  cd_cmd_read_file(); break;
		case 0x75:	cd_cmd_abort(); break;

		case 0xe0:	cd_cmd_auth_device(m_cr[1]); break;
		case 0xe1:  cd_cmd_device_auth_status(m_cr[1]);  break;
		default:
			printf("Unhandled %04x %04x %04x %04x\n",m_cr[0],m_cr[1],m_cr[2],m_cr[3]);
						//set_flag(CMOK);
	}
	m_cmd_issued = 0;
	// @todo: timer changes with cd timer, maybe we could disable this during data/audio playback and enable that instead?
	m_peri_timer->adjust(attotime::from_usec(16667), 0, attotime::from_usec(16667));	
}

segacdblock_device::blockT *segacdblock_device::cd_alloc_block(UINT8 *blknum)
{
	INT32 i;

	// search the 200 available blocks for a free one
	for (i = 0; i < 200; i++)
	{
		if (blocks[i].size == -1)
		{
			//freeblocks--;
			//if (freeblocks <= 0)
			{
				//buffull = 1;
			//	fatalerror("buffull in cd_alloc_block\n");
			}

			blocks[i].size = m_SectorLengthIn;
			*blknum = i;

			//CDROM_LOG(("CD: allocating block %d, size %x\n", i, m_SectorLengthIn))

			return &blocks[i];
		}
	}

	set_flag(BFUL);
	return (blockT *)NULL;
}

void segacdblock_device::cd_free_block(blockT *blktofree)
{
	INT32 i;

	if(blktofree == NULL)
	{
		return;
	}

	for (i = 0; i < 200; i++)
	{
		if (&blocks[i] == blktofree)
		{
//			CDROM_LOG(("CD: freeing block %d\n", i))
		}
	}

	blktofree->size = -1;
	//freeblocks++;
	//buffull = 0;
	//hirqreg &= ~BFUL;
}

void segacdblock_device::cd_readblock(UINT32 fad, UINT8 *dat)
{
	if (cdrom)
	{
		cdrom_read_data(cdrom, fad-150, dat, CD_TRACK_MODE1);
	}
}

// iso9660 parsing
void segacdblock_device::read_new_dir(UINT32 fileno)
{
	int foundpd, i;
	UINT32 cfad;//, dirfad;
	UINT8 sect[2048];

	if (fileno == 0xffffff)
	{
		cfad = 166;     // first sector of directory as per iso9660 specs

		foundpd = 0;    // search for primary vol. desc
		while ((!foundpd) && (cfad < 200))
		{
			if(m_SectorLengthIn != 2048)
				popmessage("Sector Length %d, contact MAMEdev (0)",m_SectorLengthIn);

			memset(sect, 0, 2048);
			cd_readblock(cfad++, sect);

			if ((sect[1] == 'C') && (sect[2] == 'D') && (sect[3] == '0') && (sect[4] == '0') && (sect[5] == '1'))
			{
				switch (sect[0])
				{
					case 0: // boot record
						break;

					case 1: // primary vol. desc
						foundpd = 1;
						break;

					case 2: // secondary vol desc
						break;

					case 3: // vol. section descriptor
						break;

					case 0xff:
						cfad = 200;
						break;
				}
			}
		}

		// got primary vol. desc.
		if (foundpd)
		{
			//dirfad = sect[140] | (sect[141]<<8) | (sect[142]<<16) | (sect[143]<<24);
			//dirfad += 150;

			// parse root entry
			curroot.firstfad = sect[158] | (sect[159]<<8) | (sect[160]<<16) | (sect[161]<<24);
			curroot.firstfad += 150;
			curroot.length = sect[166] | (sect[167]<<8) | (sect[168]<<16) | (sect[169]<<24);
			curroot.flags = sect[181];
			for (i = 0; i < sect[188]; i++)
			{
				curroot.name[i] = sect[189+i];
			}
			curroot.name[i] = '\0'; // terminate

			// easy to fix, but make sure we *need* to first
			if (curroot.length > MAX_DIR_SIZE)
			{
				fatalerror("ERROR: root directory too big (%d)\n", curroot.length);
			}

			// done with all that, read the root directory now
			make_dir_current(curroot.firstfad);
		}
	}
	else
	{
		if (curdir[fileno].length > MAX_DIR_SIZE)
		{
			osd_printf_error("ERROR: new directory too big (%d)!\n", curdir[fileno].length);
		}
		make_dir_current(curdir[fileno].firstfad);
	}
}

// makes the directory pointed to by FAD current
void segacdblock_device::make_dir_current(UINT32 fad)
{
	int i;
	UINT32 nextent, numentries;
	dynamic_buffer sect(MAX_DIR_SIZE);
	direntryT *curentry;

	memset(&sect[0], 0, MAX_DIR_SIZE);
	if(m_SectorLengthIn != 2048)
		popmessage("Sector Length %d, contact MAMEdev (1)",m_SectorLengthIn);

	for (i = 0; i < (curroot.length/2048); i++)
	{
		cd_readblock(fad+i, &sect[2048*i]);
	}

	nextent = 0;
	numentries = 0;
	while (nextent < MAX_DIR_SIZE)
	{
		if (sect[nextent])
		{
			nextent += sect[nextent];
			numentries++;
		}
		else
		{
			nextent = MAX_DIR_SIZE;
		}
	}

	curdir.resize(numentries);
	curentry = &curdir[0];
	numfiles = numentries;

	nextent = 0;
	while (numentries)
	{
		// [0] record size
		// [1] xa record size
		// [2-5] lba
		// [6-9] (lba?)
		// [10-13] size
		// [14-17] (size?)
		// [18] year
		// [19] month
		// [20] day
		// [21] hour
		// [22] minute
		// [23] second
		// [24] gmt offset
		// [25] flags
		// [26] file unit size
		// [27] interleave gap size
		// [28-29] volume sequencer number
		// [30-31] (volume sequencer number?)
		// [32] name character size
		// [33+ ...] file name

		curentry->record_size = sect[nextent+0];
		curentry->xa_record_size = sect[nextent+1];
		curentry->firstfad = sect[nextent+2] | (sect[nextent+3]<<8) | (sect[nextent+4]<<16) | (sect[nextent+5]<<24);
		curentry->firstfad += 150;
		curentry->length = sect[nextent+10] | (sect[nextent+11]<<8) | (sect[nextent+12]<<16) | (sect[nextent+13]<<24);
		curentry->year = sect[nextent+18];
		curentry->month = sect[nextent+19];
		curentry->day = sect[nextent+20];
		curentry->hour = sect[nextent+21];
		curentry->minute = sect[nextent+22];
		curentry->second = sect[nextent+23];
		curentry->gmt_offset = sect[nextent+24];
		curentry->flags = sect[nextent+25];
		curentry->file_unit_size = sect[nextent+26];
		curentry->interleave_gap_size = sect[nextent+27];
		curentry->volume_sequencer_number = sect[nextent+28] | (sect[nextent+29] << 8);

		for (i = 0; i < sect[nextent+32]; i++)
		{
			curentry->name[i] = sect[nextent+33+i];
		}
		curentry->name[i] = '\0';   // terminate
		#if DUMP_DIR_ENTRIES
		printf("%08x %08x %s %d/%d/%d\n",curentry->firstfad,curentry->length,curentry->name,curentry->year,curentry->month,curentry->day);
		#endif
		
		nextent += sect[nextent];
		curentry++;
		numentries--;
	}

	for (i = 0; i < numfiles; i++)
	{
		if (!(curdir[i].flags & 0x02))
		{
			firstfile = i;
			i = numfiles;
		}
	}
}

void segacdblock_device::cd_getsectoroffsetnum(UINT32 bufnum, UINT32 *sectoffs, UINT32 *sectnum)
{
	if (*sectoffs == 0xffff)
	{
		// last sector
		printf("CD: Don't know how to handle offset ffff\n");
	}
	else if (*sectnum == 0xffff)
	{
		*sectnum = partitions[bufnum].numblks - *sectoffs;
	}
}

void segacdblock_device::cd_standard_return(bool isPeri)
{
	m_dr[0] = (isPeri == true ? CD_STAT_PERI : 0) | m_cd_state | ((m_playtype == true) ? 0x80 : 0);
	m_dr[1] = 0x0000;
	m_dr[2] = 0x0100 | ((m_FAD >> 16) & 0xff);
	m_dr[3] = m_FAD & 0xffff;
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
	m_dma_src = 0;
	m_dma_size = 0;
	sourcetype = SOURCE_TOC;
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

void segacdblock_device::cd_cmd_play_disc()
{
	UINT8 play_mode = m_cr[2] >> 8;
	if(play_mode)
		return; // @todo intentional for debugging
	
	UINT32 start_pos = ((m_cr[0] & 0xff) << 16) | (m_cr[1] & 0xffff);
	UINT32 end_pos = ((m_cr[2] & 0xff) << 16) | (m_cr[3] & 0xffff);
	if(start_pos & 0x800000)
	{
		if(start_pos == 0xffffff)
			return; // @todo intentional for debugging
	
		m_FAD = start_pos & 0xfffff;
	}
	else 
		return; // @todo intentional for debugging

	if(end_pos & 0x800000)
	{
		if(end_pos == 0xffffff)
			return; // @todo intentional for debugging
	
		m_FADEnd = end_pos & 0xfffff;
	}
	else 
		return; // @todo intentional for debugging

	m_cd_state = CD_STAT_PLAY;
	m_cd_timer->adjust(attotime::from_hz(150)); // @todo use the clock Luke
	m_playtype = false;
	
	cd_standard_return(false);
	set_flag(EHST);
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_set_device_connection(UINT8 param)
{
	if(param >= MAX_FILTERS) // mostly 0xff, expect that anything above this disables anyway
		CDDeviceConnection = (filterT *)NULL;
	else
		CDDeviceConnection = &CDFilters[param];

	cd_standard_return(false);

	set_flag(ESEL);
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_reset_selector(UINT8 reset_flags, UINT8 buffer_number)
{
	int i;
	if(reset_flags == 0)
	{
		if(buffer_number < MAX_FILTERS)
		{
			for (i = 0; i < MAX_BLOCKS; i++)
			{
				cd_free_block(partitions[buffer_number].blocks[i]);
				partitions[buffer_number].blocks[i] = (blockT *)NULL;
				partitions[buffer_number].bnum[i] = 0xff;
			}

			partitions[buffer_number].size = -1;
			partitions[buffer_number].numblks = 0;
		}
		else
			return; // @todo intentional for debugging
	}
	else
	{
		/* reset false filter output conditions */
		/* TODO: check these two. */
		if(reset_flags & 0x80)
		{
			for(i=0;i<MAX_FILTERS;i++)
				CDFilters[i].condfalse = 0;
		}

		/* reset true filter output conditions */
		if(reset_flags & 0x40)
		{
			for(i=0;i<MAX_FILTERS;i++)
				CDFilters[i].condtrue = 0;
		}

		/* reset filter conditions*/
		if(reset_flags & 0x10)
		{
			for(i=0;i<MAX_FILTERS;i++)
			{
				CDFilters[i].fad = 0;
				CDFilters[i].range = 0xffffffff;
				CDFilters[i].mode = 0;
				CDFilters[i].chan = 0;
				CDFilters[i].smmask = 0;
				CDFilters[i].cimask = 0;
				CDFilters[i].fid = 0;
				CDFilters[i].smval = 0;
				CDFilters[i].cival = 0;
			}
		}
		/* reset partition buffer data */
		if(reset_flags & 0x4)
		{
			for(i=0;i<MAX_FILTERS;i++)
			{
				for (int j = 0; j < MAX_BLOCKS; j++)
				{
					cd_free_block(partitions[i].blocks[j]);
					partitions[i].blocks[j] = (blockT *)NULL;
					partitions[i].bnum[j] = 0xff;
				}

				partitions[i].size = -1;
				partitions[i].numblks = 0;
			}
//				buffull = sectorstore = 0;
		}
	}
	cd_standard_return(false);

	set_flag(ESEL);
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_get_sector_number(UINT8 buffer_number)
{
	m_dr[0] = m_cd_state;
	m_dr[1] = 0;
	m_dr[2] = 0;
	if (partitions[buffer_number].size == -1)
		m_dr[3] = 0;
	else
		m_dr[3] = partitions[buffer_number].numblks;

	set_flag(DRDY);
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_set_sector_length(UINT8 length_in, UINT8 length_out)
{
	const int sectorSizes[4] = { 2048, 2336, 2340, 2352 };
	assert(length_in < 4);
	assert(length_out < 4);
	m_SectorLengthIn = sectorSizes[length_in];
	m_SectorLengthOut = sectorSizes[length_out];
	
	cd_standard_return(false);

	set_flag(ESEL);
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_get_then_delete_sector()
{
	UINT32 sectnum = m_cr[3];
	UINT32 sectofs = m_cr[1];
	UINT32 bufnum = m_cr[2]>>8;
	
	// @todo reject states
	cd_getsectoroffsetnum(bufnum, &sectofs, &sectnum);
//	m_dma_src = 0;
//	m_dma_size = 0;
	transpart = &partitions[bufnum];

	m_TransferActive = true;
	cd_standard_return(false); // cheap hack
	
	m_dr[0] = CD_STAT_TRANS | m_cd_state;
	xferoffs = 0;
	xfersect = 0;
	m_dma_size = 0;
	xfersectpos = sectofs;
	xfersectnum = sectnum;
	xfertype = CDDMA_INPROGRESS;
//	sourcetype = SOURCE_DATA;
	set_flag(EHST);
	set_flag(DRDY);
	set_flag(CMOK);
	DeleteSectorMode = true;
}

void segacdblock_device::cd_cmd_get_copy_error()
{
	m_dr[0] = m_cd_state | 0;
	m_dr[1] = 0;
	m_dr[2] = 0;
	m_dr[3] = 0;
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_change_dir(UINT32 dir_entry)
{
	read_new_dir(dir_entry);
	set_flag(EFLS);
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_get_file_system_scope()
{
	m_dr[0] = m_cd_state;
	m_dr[1] = numfiles; // # of files in directory
	m_dr[2] = 0x0100;   // report directory held
	m_dr[3] = firstfile;    // first file id
	set_flag(EFLS);
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_read_file()
{
	UINT16 file_offset,file_filter,file_id,file_size;

	file_offset = ((m_cr[0] & 0xff)<<8)|(m_cr[1] & 0xff); /* correct? */
	file_filter = m_cr[2] >> 8;
	file_id = ((m_cr[2] & 0xff) << 16)|(m_cr[3]);
	file_size = ((curdir[file_id].length + m_SectorLengthIn - 1) / m_SectorLengthIn) - file_offset;

	m_FAD = (curdir[file_id].firstfad + file_offset);
	m_FADEnd = file_size;
	//		cd_stat = CD_STAT_PLAY|0x80;    // set "cd-rom" bit
	//		cd_curfad = (curdir[file_id].firstfad + file_offset);
	//		fadstoplay = file_size;
	if(file_filter < 0x24)
		CDDeviceConnection = &CDFilters[file_filter];
	else
		CDDeviceConnection = (filterT *)NULL;
	
	m_cd_state = CD_STAT_PLAY;
	m_playtype = true;
	cd_standard_return(false);
	m_cd_timer->adjust(attotime::from_hz(150)); // @todo use the clock Luke
	
//	printf("Read file %08x (%08x %08x) %02x %d\n",curdir[file_id].firstfad,cd_curfad,fadstoplay,file_filter,sectlenin);
}

void segacdblock_device::cd_cmd_abort()
{
	cd_standard_return(false);

	set_flag(EFLS);
	// ...
	xfertype = CDDMA_STOPPED;
	sourcetype = SOURCE_NONE;
	m_dma_size = 0;
	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_auth_device(UINT16 AuthType)
{
	switch(AuthType)
	{
		case 0: // CD-Rom
			set_flag(EFLS|CSCT);
			break;
		case 1: // MPEG
			//set_flag(MPED);
			break;
	}

	cd_standard_return(false);

	set_flag(CMOK);
}

void segacdblock_device::cd_cmd_device_auth_status(UINT16 AuthType)
{
	switch(AuthType)
	{
		case 0: // CD-Rom
			m_dr[0] = m_cd_state | 0;
			m_dr[1] = 4; /**< @todo: various auth states */
			m_dr[2] = 0;
			m_dr[3] = 0;
			break;
		case 1:
			m_dr[0] = m_cd_state | 0;
			m_dr[1] = 0; /**< @todo: 2 if card present */
			m_dr[2] = 0;
			m_dr[3] = 0;
			break;
	}
	
	set_flag(CMOK);
}

void segacdblock_device::TOCRetrieve()
{
	int i, ntrks, tocptr, fad;

	//xfertype = XFERTYPE_TOC;
	//m_dma_src = 0;
	//m_dma_size = 102*4;

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

//	xfertype = XFERTYPE_TOC;
//	m_TOCPhase = false;
//	set_flag(DRDY);
}

/*!
 @todo Of course DMA needs steps ...
 */
void segacdblock_device::dma_setup()
{
	switch(sourcetype)
	{
		case SOURCE_NONE:
			// ...
			break;
		case SOURCE_TOC:
			memcpy(m_DMABuffer,tocbuf,102*4);
			m_dma_size = 102*4;
			m_dma_src = 0;
			xfertype = CDDMA_INPROGRESS;
			set_flag(DRDY);
			sourcetype = SOURCE_NONE;
			break;
		case SOURCE_DATA:
			xfertype = CDDMA_INPROGRESS;
			set_flag(DRDY);
			sourcetype = SOURCE_NONE;
			break;
		case SOURCE_AUDIO:
			break;
	}
}

segacdblock_device::partitionT *segacdblock_device::cd_filterdata(filterT *flt, int trktype, UINT8 *p_ok)
{
	int match, keepgoing;
	partitionT *filterprt = (partitionT *)NULL;

	//CDROM_LOG(("cd_filterdata, trktype %d\n", trktype))
	match = 1;
	keepgoing = 2;
	m_LastBuffer = flt->condtrue;

	// loop on the filters
	do
	{
		// FAD range check?
		/* according to an obscure document note, this switches the filter connector to be false if the range fails ... I think ... */
		if (flt->mode & 0x40)
		{
			if ((m_FAD < flt->fad) || (m_FAD > (flt->fad + flt->range)))
			{
				//printf("curfad reject %08x %08x %08x %08x\n",cd_curfad,fadstoplay,flt->fad,flt->fad+flt->range);
				match = 0;
				//m_LastBuffer = flt->condfalse;
				//flt = &filters[m_LastBuffer];
			}
		}

		if ((trktype != CD_TRACK_AUDIO) && (curblock.data[15] == 2))
		{
			if (flt->mode & 1)  // file number
			{
				if (curblock.fnum != flt->fid)
				{
					printf("fnum reject\n");
					match = 0;
				}
			}

			if (flt->mode & 2)  // channel number
			{
				if (curblock.chan != flt->chan)
				{
					printf("channel number reject\n");
					match = 0;
				}
			}

			if (flt->mode & 4)  // sub mode
			{
				if((curblock.subm & flt->smmask) != flt->smval)
				{
					printf("sub mode reject\n");
					match = 0;
				}
			}

			if (flt->mode & 8)  // coding information
			{
				if((curblock.cinf & flt->cimask) != flt->cival)
				{
					printf("coding information reject\n");
					match = 0;
				}
			}

			if (flt->mode & 0x10)   // reverse subheader conditions
			{
				match ^= 1;
			}
		}

		if (match)
		{
			//m_LastBuffer = flt->condtrue;
			//filterprt = &partitions[m_LastBuffer];
			// we're done
			keepgoing = 0;
		}
		else
		{
			m_LastBuffer = flt->condfalse;

			// reject sector if no match on either connector
			if ((m_LastBuffer == 0xff) || (keepgoing == 0))
			{
				*p_ok = 0;
				return (partitionT *)NULL;
			}

			// try again using the filter that was on the "false" connector
			flt = &CDFilters[m_LastBuffer];
			match = 1;

			// and exit if we fail
			keepgoing--;
		}
	} while (keepgoing);

	filterprt = &partitions[m_LastBuffer];

	// try to allocate a block
	filterprt->blocks[filterprt->numblks] = cd_alloc_block(&filterprt->bnum[filterprt->numblks]);

	// did the allocation succeed?
	if (filterprt->blocks[filterprt->numblks] == (blockT *)NULL)
	{
		*p_ok = 0;
		return (partitionT *)NULL;
	}

	// copy working block to the newly allocated one
	memcpy(filterprt->blocks[filterprt->numblks], &curblock, sizeof(blockT));

	// and massage the data format a bit
	switch  (curblock.size)
	{
		case 2048:  // user data
			if (curblock.data[15] == 2)
			{
				// mode 2
				memcpy(&filterprt->blocks[filterprt->numblks]->data[0], &curblock.data[24], curblock.size);
			}
			else
			{
				// mode 1
				memcpy(&filterprt->blocks[filterprt->numblks]->data[0], &curblock.data[16], curblock.size);
			}
			break;

		case 2324:  // Mode 2 Form 2 data
			memcpy(&filterprt->blocks[filterprt->numblks]->data[0], &curblock.data[24], curblock.size);
			break;

		case 2336:  // Mode 2 Form 2 skip sync/header
			memcpy(&filterprt->blocks[filterprt->numblks]->data[0], &curblock.data[16], curblock.size);
			break;

		case 2340:  // Mode 2 Form 2 skip sync only
			memcpy(&filterprt->blocks[filterprt->numblks]->data[0], &curblock.data[12], curblock.size);
			break;

		case 2352:  // want all data, it's already done, so don't do it again :)
			break;
	}

	// update the status of the partition
	if (filterprt->size == -1)
		filterprt->size = 0;

	filterprt->size += filterprt->blocks[filterprt->numblks]->size;
	filterprt->numblks++;

	*p_ok = 1;
	return filterprt;
}

// read a single sector off the CD, applying the current filter(s) as necessary
segacdblock_device::partitionT *segacdblock_device::cd_read_filtered_sector(INT32 fad, UINT8 *p_ok)
{
	int trktype;

	//if ((cddevice != NULL) && (!buffull))
	if (CDDeviceConnection != NULL)
	{
		// find out the track's type
		trktype = cdrom_get_track_type(cdrom, cdrom_get_track(cdrom, fad-150));

		// now get a raw 2352 byte sector - if it's mode 1, get mode1_raw
		if ((trktype == CD_TRACK_MODE1) || (trktype == CD_TRACK_MODE1_RAW))
		{
			cdrom_read_data(cdrom, fad-150, curblock.data, CD_TRACK_MODE1_RAW);
		}
		else if (trktype != CD_TRACK_AUDIO) // if not audio it must be mode 2 so get mode2_raw
		{
			cdrom_read_data(cdrom, fad-150, curblock.data, CD_TRACK_MODE2_RAW);
		}
		else
		{
			cdrom_read_data(cdrom, fad-150, curblock.data, CD_TRACK_AUDIO);
		}

		curblock.size = m_SectorLengthIn;
		curblock.FAD = fad;

		// if track is Mode 2, get the subheader values
		if ((trktype != CD_TRACK_AUDIO) && (curblock.data[15] == 2))
		{
			curblock.chan = curblock.data[17];
			curblock.fnum = curblock.data[16];
			curblock.subm = curblock.data[18];
			curblock.cinf = curblock.data[19];

			// if it's Form 2, the length is actually 2324 bytes
			if (curblock.subm & 0x20)
			{
				curblock.size = 2324;
			}
		}

		return cd_filterdata(CDDeviceConnection, trktype, &*p_ok);
	}

	*p_ok = 0;
	return (partitionT *)NULL;
}


void segacdblock_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	assert(id < CD_TIMER+1);

	if(id == CMD_TIMER)
		SH1CommandExecute();
	else if(id == PERI_TIMER)
	{
		dma_setup();

		if(m_isDiscInTray == true)
			m_cd_state = CD_STAT_PAUSE;
		else
			m_cd_state = CD_STAT_NODISC;

		if(m_TransferActive == true)
			m_cd_state|= CD_STAT_TRANS;

		cd_standard_return(true);
		set_flag(SCDQ);
	}
	else if(id == CD_TIMER)
	{
		if((m_cd_state & 0x0f00) == CD_STAT_PAUSE)
		{
			if(!(m_hirq & BFUL))
				m_cd_state = CD_STAT_PLAY;
			
			m_cd_timer->adjust(attotime::from_hz(150));
			
		}
		else if((m_cd_state & 0x0f00) == CD_STAT_PLAY)
		{
			UINT8 p_ok;
			
			popmessage("%08x %08x",m_FAD,m_FADEnd);

			if (cdrom)
			{
				if(cdrom_get_track_type(cdrom, cdrom_get_track(cdrom, m_FAD)) != CD_TRACK_AUDIO)
				{
					cd_read_filtered_sector(m_FAD,&p_ok);
					//machine().device<cdda_device>("cdda")->stop_audio(); //stop any pending CD-DA
				}
				else
				{
					p_ok = 1; // TODO
					//machine().device<cdda_device>("cdda")->start_audio(cd_curfad, 1);
				}
			}
			
			if(p_ok)
			{
				m_FAD ++;
				m_FADEnd --;
				set_flag(CSCT);
				
				if(!m_FADEnd)
				{
					m_cd_state = CD_STAT_PAUSE;
					set_flag(PEND);
					
					if(m_playtype == true)
						set_flag(EFLS);
					return;
				}
			}
		
			if(m_hirq & BFUL)
				m_cd_state = CD_STAT_PAUSE;
			
			m_cd_timer->adjust(attotime::from_hz(150));
		}
	}
	
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void segacdblock_device::device_start()
{
	m_space = &space(AS_0);
	//m_sh1_timer = timer_alloc(SH1_TIMER);
	m_peri_timer = timer_alloc(PERI_TIMER);
	m_cmd_timer = timer_alloc(CMD_TIMER);
	m_cd_timer = timer_alloc(CD_TIMER);
	m_DMABuffer = auto_alloc_array_clear(machine(), UINT8, 2352);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void segacdblock_device::device_reset()
{
	INT32 i,j;
	// init_command_regs
	m_dr[0] = 'C';
	m_dr[1] = 'D' << 8 | 'B';
	m_dr[2] = 'L' << 8 | 'O';
	m_dr[3] = 'C' << 8 | 'K';
	m_FAD = 150;
	m_peri_timer->reset();
	m_cmd_timer->reset();
	m_cd_timer->reset();
	
	xfertype = CDDMA_STOPPED;
	sourcetype = SOURCE_NONE;
	m_dma_size = 0;
	m_dma_src = 0;
	//m_sh1_timer->adjust(attotime::from_hz(clock()*256), 0, attotime::from_hz(clock()*256));
	m_sh1_inited = false;
	m_cmd_issued = 0;
	m_hirq = 0xffff;

	m_LastBuffer = 0xff;

	// reset buffer partitions
	for (i = 0; i < MAX_FILTERS; i++)
	{
		partitions[i].size = -1;
		partitions[i].numblks = 0;

		for (j = 0; j < MAX_BLOCKS; j++)
		{
			partitions[i].blocks[j] = (blockT *)NULL;
			partitions[i].bnum[j] = 0xff;
		}
	}

	// reset blocks
	for (i = 0; i < MAX_BLOCKS; i++)
	{
		blocks[i].size = -1;
		memset(&blocks[i].data, 0, CD_MAX_SECTOR_DATA);
	}

	
	cdrom = subdevice<cdrom_image_device>("cdrom")->get_cdrom_file();
	if(cdrom != NULL)
		TOCRetrieve();
	
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
		res = m_space->read_dword(offset*4);
		
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

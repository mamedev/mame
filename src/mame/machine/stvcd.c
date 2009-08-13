/***************************************************************************

  machine/stvcd.c - Sega Saturn and ST-V CD-ROM handling

  Rewritten (again) 2007 by R. Belmont.

  Status: All known discs at least load their executable, and many load
          some data files successfully, but there are other problems.

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

***************************************************************************/

#include "driver.h"
#ifdef MESS
#include "devices/chd_cd.h"
#endif
#include "cdrom.h"
#include "stvcd.h"
#include <stdio.h>

// super-verbose
#if 0
#define CDROM_LOG(x) printf x;
#else
#define CDROM_LOG(x)
#endif

static cdrom_file *cdrom = (cdrom_file *)NULL;

static void cd_readTOC(void);
static void cd_readblock(UINT32 fad, UINT8 *dat);
static void cd_playdata(void);

#define MAX_FILTERS	(24)
#define MAX_BLOCKS	(200)
#define MAX_DIR_SIZE	(16384)

typedef struct
{
	UINT8 flags;		// iso9660 flags
	UINT32 length;		// length of file
	UINT32 firstfad;		// first sector of file
	UINT8 name[128];
} direntryT;

typedef struct
{
   UINT8 mode;
   UINT8 chan;
   UINT8 smmask;
   UINT8 cimask;
   UINT8 fid;
   UINT8 smval;
   UINT8 cival;
   UINT8 condtrue;
   UINT8 condfalse;
   UINT32 fad;
   UINT32 range;
} filterT;

typedef struct
{
   INT32 size;	// size of block
   INT32 FAD;	// FAD on disc
   UINT8 data[CD_MAX_SECTOR_DATA];
   UINT8 chan;	// channel
   UINT8 fnum;	// file number
   UINT8 subm;	// subchannel mode
   UINT8 cinf;	// coding information
} blockT;

typedef struct
{
   INT32 size;
   blockT *blocks[MAX_BLOCKS];
   UINT8 bnum[MAX_BLOCKS];
   UINT8 numblks;
} partitionT;

// 16-bit transfer types
typedef enum
{
	XFERTYPE_INVALID,
	XFERTYPE_TOC,
	XFERTYPE_FILEINFO_1,
	XFERTYPE_FILEINFO_254
} transT;

// 32-bit transfer types
typedef enum
{
	XFERTYPE32_INVALID,
	XFERTYPE32_GETSECTOR,
	XFERTYPE32_GETDELETESECTOR
} trans32T;

// local variables
static emu_timer *sector_timer;
static partitionT partitions[MAX_FILTERS];
static partitionT *transpart;

static blockT blocks[MAX_BLOCKS];
static blockT curblock;

static UINT8 tocbuf[102*4];
static UINT8 finfbuf[256];
static UINT8 onesectorstored;

static INT32 sectlenin, sectlenout;

static UINT8 lastbuf, playtype;

static transT xfertype;
static trans32T xfertype32;
static UINT32 xfercount, calcsize;
static UINT32 xferoffs, xfersect, xfersectpos, xfersectnum, xferdnum;

static filterT filters[MAX_FILTERS];
static filterT *cddevice;
static int cddevicenum;

static UINT16 cr1, cr2, cr3, cr4;
static UINT16 hirqmask, hirqreg;
static UINT16 cd_stat;
static UINT32 cd_curfad = 0;
static UINT32 in_buffer = 0;	// amount of data in the buffer
static int oddframe = 0;
static UINT32 fadstoplay = 0;
static int buffull, sectorstore, freeblocks;

// iso9660 utilities
static void read_new_dir(UINT32 fileno);
static void make_dir_current(UINT32 fad);

static direntryT curroot;		// root entry of current filesystem
static direntryT *curdir;		// current directory
static int numfiles;			// # of entries in current directory
static int firstfile;			// first non-directory file

// HIRQ definitions
#define CMOK 0x0001 // command ok / ready for new command
#define DRDY 0x0002 // drive ready
#define CSCT 0x0004 // sector ready (?)
#define BFUL 0x0008 // buffer full
#define PEND 0x0010 // command pending
#define DCHG 0x0020 // disc change / tray open
#define ESEL 0x0040 // soft reset, end of blah
#define EHST 0x0080 //
#define ECPY 0x0100 //
#define EFLS 0x0200 // stop execution of cd block filesystem
#define SCDQ 0x0400 // subcode Q renewal complete
#define MPED 0x0800 // MPEG
#define MPCM 0x1000 // MPEG
#define MPST 0x2000 // MPEG

// CD status (hi byte of CR1) definitions:
// (these defines are shifted up 8)
#define CD_STAT_BUSY     0x0000		// status change in progress
#define CD_STAT_PAUSE    0x0100		// CD block paused (temporary stop)
#define CD_STAT_STANDBY  0x0200		// CD drive stopped
#define CD_STAT_PLAY     0x0300		// CD play in progress
#define CD_STAT_SEEK     0x0400		// drive seeking
#define CD_STAT_SCAN     0x0500		// drive scanning
#define CD_STAT_OPEN     0x0600		// tray is open
#define CD_STAT_NODISC   0x0700		// no disc present
#define CD_STAT_RETRY    0x0800		// read retry in progress
#define CD_STAT_ERROR    0x0900		// read data error occured
#define CD_STAT_FATAL    0x0a00		// fatal error (hard reset required)
#define CD_STAT_PERI     0x2000		// periodic response if set, else command response
#define CD_STAT_TRANS    0x4000		// data transfer request if set
#define CD_STAT_WAIT     0x8000		// waiting for command if set, else executed immediately
#define CD_STAT_REJECT   0xff00		// ultra-fatal error.

static TIMER_CALLBACK( sector_cb )
{
	if (fadstoplay)
	{
		cd_playdata();
	}
	else
	{
		hirqreg |= SCDQ;
	}

	cd_stat |= CD_STAT_PERI;
	cr1 = cd_stat;
	cr2 = 0x4101;
	cr3 = (cd_curfad>>16)&0xff;
	cr4 = cd_curfad;

	timer_adjust_oneshot(sector_timer, ATTOTIME_IN_HZ(150), 0);
}

// global functions
void stvcd_reset(running_machine *machine)
{
	INT32 i, j;

	hirqmask = 0xffff;
	hirqreg = 0xffff;
	cr1 = 'C';
	cr2 = ('D'<<8) | 'B';
	cr3 = ('L'<<8) | 'O';
	cr4 = ('C'<<8) | 'K';
	cd_stat = CD_STAT_PAUSE;

	if (curdir != (direntryT *)NULL)
		free((void *)curdir);
	curdir = (direntryT *)NULL;		// no directory yet

	xfertype = XFERTYPE_INVALID;
	xfertype32 = XFERTYPE32_INVALID;

	// reset flag vars
	buffull = sectorstore = 0;

	freeblocks = 200;

	sectlenin = sectlenout = 2048;

	lastbuf = 0xff;

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

	// open device
	if (cdrom)
	{
		cdrom_close(cdrom);
		cdrom = (cdrom_file *)NULL;
	}

	#ifdef MESS
	cdrom = mess_cd_get_cdrom_file(devtag_get_device( machine, "cdrom" ));
	#else
	cdrom = cdrom_open(get_disk_handle(machine, "cdrom"));
	#endif

	if (cdrom)
	{
		CDROM_LOG(("Opened CD-ROM successfully, reading root directory\n"))
		read_new_dir(0xffffff);	// read root directory
	}
	else
	{
		cd_stat = CD_STAT_OPEN;
	}

	sector_timer = timer_alloc(machine, sector_cb, NULL);
	timer_adjust_oneshot(sector_timer, ATTOTIME_IN_HZ(150), 0);	// 150 sectors / second = 300kBytes/second
}

static blockT *cd_alloc_block(UINT8 *blknum)
{
	INT32 i;

	// search the 200 available blocks for a free one
	for (i = 0; i < 200; i++)
	{
		if (blocks[i].size == -1)
		{
			freeblocks--;
			if (freeblocks <= 0)
			{
				buffull = 1;
			}

			blocks[i].size = sectlenin;
			*blknum = i;

			CDROM_LOG(("CD: allocating block %d, size %x\n", i, sectlenin))

			return &blocks[i];
		}
	}

	buffull = 1;
	return (blockT *)NULL;
}

static void cd_free_block(blockT *blktofree)
{
	INT32 i;

	CDROM_LOG(("cd_free_block: %x\n", (UINT32)(FPTR)blktofree))

	for (i = 0; i < 200; i++)
	{
		if (&blocks[i] == blktofree)
		{
			CDROM_LOG(("CD: freeing block %d\n", i))
		}
	}

	blktofree->size = -1;
	freeblocks++;
	buffull = 0;
	hirqreg &= ~BFUL;
}

static void cd_getsectoroffsetnum(UINT32 bufnum, UINT32 *sectoffs, UINT32 *sectnum)
{
	if (*sectoffs == 0xffff)
	{
		// last sector
		CDROM_LOG(("CD: Don't know how to handle offset ffff\n"))
	}
	else if (*sectnum == 0xffff)
	{
		*sectnum = partitions[bufnum].numblks - *sectoffs;
	}
}

static void cd_defragblocks(partitionT *part)
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

static UINT16 cd_readWord(UINT32 addr)
{
	UINT16 rv;

	switch (addr & 0xffff)
	{
		case 0x0008:	// read HIRQ register
		case 0x000a:
			rv = hirqreg;

			rv &= ~DCHG;	// always clear bit 6 (tray open)

			if (buffull) rv |= BFUL; else rv &= ~BFUL;
			if (sectorstore) rv |= CSCT; else rv &= ~CSCT;

			hirqreg = rv;

//          CDROM_LOG(("%s:RW HIRQ: %04x\n", cpuexec_describe_context(Machine), rv))

			return rv;

		case 0x000c:
		case 0x000e:
			CDROM_LOG(("RW HIRM: %04x\n", hirqmask))
			return hirqmask;

		case 0x0018:
		case 0x001a:
//          CDROM_LOG(("RW CR1: %04x\n", cr1))
			return cr1;

		case 0x001c:
		case 0x001e:
//          CDROM_LOG(("RW CR2: %04x\n", cr2))
			return cr2;

		case 0x0020:
		case 0x0022:
//          CDROM_LOG(("RW CR3: %04x\n", cr3))
			return cr3;

		case 0x0024:
		case 0x0026:
//          CDROM_LOG(("RW CR4: %04x\n", cr4))
			return cr4;

		case 0x8000:
			rv = 0xffff;
			switch (xfertype)
			{
				case XFERTYPE_TOC:
					rv = tocbuf[xfercount]<<8 | tocbuf[xfercount+1];
					xfercount += 2;
					xferdnum += 2;

					if (xfercount > 102*4)
					{
						xfercount = 0;
						xfertype = XFERTYPE_INVALID;
					}
					break;

				case XFERTYPE_FILEINFO_1:
					rv = finfbuf[xfercount]<<8 | finfbuf[xfercount+1];
					xfercount += 2;
					xferdnum += 2;

					if (xfercount > 6*2)
					{
						xfercount = 0;
						xfertype = XFERTYPE_INVALID;
					}
					break;

				default:
					CDROM_LOG(("STVCD: Unhandled xfer type %d\n", (int)xfertype))
					rv = 0xffff;
					break;
			}

			return rv;

		default:
			CDROM_LOG(("%s:CD: RW %08x\n", cpuexec_describe_context(machine), addr))
			return 0xffff;
	}

}

static UINT32 cd_readLong(UINT32 addr)
{
	UINT32 rv = 0;

	switch (addr & 0xffff)
	{
		case 0x8000:
			switch (xfertype32)
			{
				case XFERTYPE32_GETSECTOR:
				case XFERTYPE32_GETDELETESECTOR:
					// make sure we have sectors left
					if (xfersect < xfersectnum)
					{
						// get next longword
						rv = transpart->blocks[xfersectpos+xfersect]->data[xferoffs]<<24 |
						     transpart->blocks[xfersectpos+xfersect]->data[xferoffs + 1]<<16 |
						     transpart->blocks[xfersectpos+xfersect]->data[xferoffs + 2]<<8 |
						     transpart->blocks[xfersectpos+xfersect]->data[xferoffs + 3];

						xferdnum += 4;
						xferoffs += 4;

						// did we run out of sector?
						if (xferoffs >= transpart->blocks[xfersect]->size)
						{
							CDROM_LOG(("CD: finished xfer of block %d of %d\n", xfersect+1, xfersectnum))

							xferoffs = 0;
							xfersect++;
						}
					}
					else	// sectors are done, kill 'em all if we can
					{
						if (xfertype32 == XFERTYPE32_GETDELETESECTOR)
						{
							INT32 i;

							CDROM_LOG(("Killing sectors in done\n"))

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
							transpart->size -= xferdnum;
							transpart->numblks -= xfersectnum;

							xfertype32 = XFERTYPE32_INVALID;
						}
					}
					break;

				default:
					CDROM_LOG(("CD: unhandled 32-bit transfer type\n"))
					break;
			}

			return rv;

		default:
			CDROM_LOG(("%s:CD: RL %08x\n", cpuexec_describe_context(machine), addr))
			return 0xffff;
	}
}

static void cd_writeWord(UINT32 addr, UINT16 data)
{
	UINT32 temp;

	switch(addr & 0xffff)
	{
	case 0x0008:
	case 0x000a:
//              CDROM_LOG(("%s:WW HIRQ: %04x & %04x => %04x\n", cpuexec_describe_context(machine), hirqreg, data, hirqreg & data))
		hirqreg &= data;
		return;
	case 0x000c:
	case 0x000e:
      		CDROM_LOG(("WW HIRM: %04x => %04x\n", hirqmask, data))
		hirqmask = data;
		return;
	case 0x0018:
	case 0x001a:
//              CDROM_LOG(("WW CR1: %04x\n", data))
		cr1 = data;
		cd_stat &= ~CD_STAT_PERI;
		break;
	case 0x001c:
	case 0x001e:
//              CDROM_LOG(("WW CR2: %04x\n", data))
		cr2 = data;
		break;
	case 0x0020:
	case 0x0022:
//              CDROM_LOG(("WW CR3: %04x\n", data))
		cr3 = data;
		break;
	case 0x0024:
	case 0x0026:
//              CDROM_LOG(("WW CR4: %04x\n", data))
		cr4 = data;
//      CDROM_LOG(("CD: command exec %02x %02x %02x %02x %02x (stat %04x)\n", hirqreg, cr1, cr2, cr3, cr4, cd_stat))

		if (!cdrom)
		{
			cd_stat = CD_STAT_OPEN;
			cr1 = cd_stat | 0xff;
			cr2 = 0xffff;
			cr3 = 0xffff;
			cr4 = 0xffff;
			return;
		}

		switch (cr1 & 0xff00)
		{
		case 0x0000:
			CDROM_LOG(("%s:CD: Get Status\n", cpuexec_describe_context(machine)))

			// values taken from a US saturn with a disc in and the lid closed
			hirqreg |= CMOK;
			cr1 = cd_stat;
			cr2 = 0x4101;
			cr3 = 0x100 | (cd_curfad>>16);
			cr4 = cd_curfad;
			CDROM_LOG(("   = %04x %04x %04x %04x %04x\n", hirqreg, cr1, cr2, cr3, cr4))
			break;

		case 0x0100:
			CDROM_LOG(("%s:CD: Get Hardware Info\n", cpuexec_describe_context(machine)))
			hirqreg |= CMOK;
			cr1 = cd_stat;
			cr2 = 0x0201;
			cr3 = 0x0000;
			cr4 = 0x0400;
			break;

		case 0x200:	// Get TOC
			CDROM_LOG(("%s:CD: Get TOC\n", cpuexec_describe_context(machine)))
			cd_readTOC();
			cd_stat = CD_STAT_TRANS|CD_STAT_PAUSE;
			cr2 = 102*2;	// TOC length in words (102 entries @ 2 words/4bytes each)
			cr3 = 0;
			cr4 = 0;
			xferdnum = 0;
			hirqreg |= (CMOK|DRDY);
			break;

		case 0x0300:	// get session info (lower byte = session # to get?)
		            	// bios is interested in returns in cr3 and cr4
						// cr3 should be data track #
						// cr4 must be > 1 and < 100 or bios gets angry.
			CDROM_LOG(("%s:CD: Get Session Info\n", cpuexec_describe_context(machine)))
			cd_readTOC();
			switch (cr1 & 0xff)
			{
				case 0:	// get total session info / disc end
					cr2 = 0;
					cr3 = 0x0100 | tocbuf[(101*4)+1];
					cr4 = tocbuf[(101*4)+2]<<8 | tocbuf[(101*4)+3];
					break;

				case 1:	// get total session info / disc start
					cr2 = 0;
					cr3 = 0x0100;	// sessions in high byte, session start in lower
					cr4 = 0;
					break;

				default:
					mame_printf_error("CD: Unknown request to Get Session Info %x\n", cr1 & 0xff);
					break;
			}
			cd_stat = CD_STAT_PAUSE;
			cr1 = cd_stat << 8;
			hirqreg |= (CMOK);
			break;

		case 0x400:	// initialize CD system
				// CR1 & 1 = reset software
				// CR1 & 2 = decode RW subcode
				// CR1 & 4 = don't confirm mode 2 subheader
				// CR1 & 8 = retry reading mode 2 sectors
				// CR1 & 10 = force single-speed
			CDROM_LOG(("%s:CD: Initialize CD system\n", cpuexec_describe_context(machine)))
			hirqreg |= (CMOK|DRDY|ESEL);
			cd_stat = CD_STAT_PAUSE;
			cd_curfad = 150;
			in_buffer = 0;
			buffull = 0;
			cr2 = 0x4101;
			cr3 = 0x100 | ((cd_curfad>>16)&0xff);
			cr4 = cd_curfad;
			break;

		case 0x0600:	// end data transfer
				// returns # of bytes transfered (24 bits) in
				// low byte of cr1 (MSB) and cr2 (middle byte, LSB)
			CDROM_LOG(("%s:CD: End data transfer (%d bytes xfer'd)\n", cpuexec_describe_context(machine), xferdnum))

			// clear the "transfer" flag
			cd_stat &= ~CD_STAT_TRANS;

			if (xferdnum)
			{
				cr1 = (cd_stat) | ((xferdnum>>17) & 0xff);
				cr2 = (xferdnum>>1)&0xffff;
				cr3 = 0;
				cr4 = 0;
			}
			else
			{
				cr1 = (cd_stat) | (0xff);	// is this right?
				cr2 = 0xffff;
				cr3 = 0;
				cr4 = 0;
			}

			// try to clean up any transfers still in progress
			switch (xfertype32)
			{
				case XFERTYPE32_GETSECTOR:
					hirqreg |= EHST;
					break;

				case XFERTYPE32_GETDELETESECTOR:
					if (transpart->size > 0)
					{
						INT32 i;

						xfertype32 = XFERTYPE32_INVALID;

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
						transpart->size -= xferdnum;
						transpart->numblks -= xfersectnum;

						if (freeblocks == 200)
						{
							onesectorstored = 0;
						}

						hirqreg |= EHST;
					}
					break;

				default:
					break;
			}


			// hack for the bootloader
			cd_stat |= CD_STAT_PERI;
			cr1 = cd_stat;

			// and kick the CD if there's more to read
			cd_playdata();

			xferdnum = 0;
			hirqreg |= CMOK;

			CDROM_LOG(("   = %04x %04x %04x %04x %04x\n", hirqreg, cr1, cr2, cr3, cr4))
			break;

		case 0x1000: // Play Disk.  FAD is in lowest 7 bits of cr1 and all of cr2.
			CDROM_LOG(("%s:CD: Play Disk\n",   cpuexec_describe_context(machine)))
			cd_stat = CD_STAT_PLAY; //|0x80;    // set "cd-rom" bit?
			cd_curfad = ((cr1&0xff)<<16) | cr2;
			fadstoplay = ((cr3&0xff)<<16) | cr4;

			if (cd_curfad & 0x800000)
			{
				if (cd_curfad != 0xffffff)
				{
					// fad mode
					cd_curfad &= 0xfffff;
					fadstoplay &= 0xfffff;
				}
			}
			else
			{
				// track mode
				mame_printf_error("CD: Play Disk track mode, not yet implemented\n");
			}

			CDROM_LOG(("CD: Play Disk: start %x length %x\n", cd_curfad, fadstoplay))

			cr2 = 0x4101;	// ctrl/adr in hi byte, track # in low byte
			cr3 = (0x100) | ((cd_curfad>>16)&0xff);	// index of subcode in hi byte, frame address
			cr4 = cd_curfad & 0xffff;
			hirqreg |= (CMOK|DRDY);
			oddframe = 0;
			in_buffer = 0;

			playtype = 0;

			// and do the disc I/O
			// make sure it doesn't come in too early
			timer_adjust_oneshot(sector_timer, attotime_never, 0);
			timer_adjust_oneshot(sector_timer, ATTOTIME_IN_HZ(150), 0);	// 150 sectors / second = 300kBytes/second
			break;

		case 0x1100: // disk seek
			CDROM_LOG(("%s:CD: Disk seek\n",   cpuexec_describe_context(machine)))
			if (cr1 & 0x80)
			{
				temp = (cr1&0x7f)<<16;	// get FAD to seek to
				temp |= cr2;

				if (temp == 0xffffff)
				{
					cd_stat = CD_STAT_PAUSE;
				}
				else
				{
					CDROM_LOG(("CD: not clear how to handle FAD seek\n"))
					cd_curfad = temp;
				}
			}
			else
			{
				// is it a valid track?
				if (cr2 >> 8)
				{
				 	cd_stat = CD_STAT_PAUSE;
					// (index is cr2 low byte)
				}
				else
				{
					cd_stat = CD_STAT_STANDBY;
					cd_curfad = 0xffffffff;
				}
			}


			hirqreg |= CMOK;
			cr1 = cd_stat;
			cr2 = 0x4101;
			cr3 = (cd_curfad>>16)&0xff;
			cr4 = cd_curfad;
			break;

		case 0x3000:	// Set CD Device connection
			{
				UINT8 parm;

				// get operation
				parm = cr3>>8;

				CDROM_LOG(("%s:CD: Set CD Device Connection filter # %x\n",   cpuexec_describe_context(machine), parm))

				cddevicenum = parm;

				if (parm == 0xff)
				{
					cddevice = (filterT *)NULL;
				}
				else
				{
					if (parm < 0x24)
					{
						cddevice = &filters[(cr3>>8)];
					}
				}

				hirqreg |= (CMOK|ESEL);
			}
			break;

		case 0x4000:	// Set Filter Range
						// cr1 low + cr2 = FAD0, cr3 low + cr4 = FAD1
						// cr3 hi = filter num.
			{
				UINT8 fnum = (cr3>>8)&0xff;

				CDROM_LOG(("%s:CD: Set Filter Range\n",   cpuexec_describe_context(machine)))

				filters[fnum].fad = ((cr1 & 0xff)<<16) | cr2;
				filters[fnum].range = ((cr3 & 0xff)<<16) | cr4;

				hirqreg |= (CMOK|ESEL|DRDY);
				cr2 = 0x4101;	// ctrl/adr in hi byte, track # in low byte
				cr3 = 0x0100|((cd_curfad>>16)&0xff);
				cr4 = (cd_curfad & 0xffff);
			}
			break;

		case 0x4200:	// Set Filter Subheader conditions
			{
				UINT8 fnum = (cr3>>8)&0xff;

				CDROM_LOG(("%s:CD: Set Filter Subheader conditions %x => chan %x masks %x fid %x vals %x\n", cpuexec_describe_context(machine), fnum, cr1&0xff, cr2, cr3&0xff, cr4))

				filters[fnum].chan = cr1 & 0xff;
				filters[fnum].smmask = (cr2>>8)&0xff;
				filters[fnum].cimask = cr2&0xff;
				filters[fnum].fid = cr3&0xff;
				filters[fnum].smval = (cr4>>8)&0xff;
				filters[fnum].cival = cr4&0xff;

				hirqreg |= (CMOK|ESEL);
				cr2 = 0x4101;	// ctrl/adr in hi byte, track # in low byte
				cr3 = 0x0100|((cd_curfad>>16)&0xff);
				cr4 = (cd_curfad & 0xffff);
			}
			break;

		case 0x4400:	// Set Filter Mode
			{
				UINT8 fnum = (cr3>>8)&0xff;
				UINT8 mode = (cr1 & 0xff);

				// initialize filter?
				if (mode & 0x80)
				{
					memset(&filters[fnum], 0, sizeof(filterT));
				}
				else
				{
					filters[fnum].mode = mode;
				}

				CDROM_LOG(("%s:CD: Set Filter Mode filt %x mode %x\n", cpuexec_describe_context(machine), fnum, mode))
				hirqreg |= (CMOK|ESEL|DRDY);
				cr2 = 0x4101;	// ctrl/adr in hi byte, track # in low byte
				cr3 = 0x0100|((cd_curfad>>16)&0xff);
				cr4 = (cd_curfad & 0xffff);
			}
			break;

		case 0x4600:	// Set Filter Connection
			{
				UINT8 fnum = (cr3>>8)&0xff;

				CDROM_LOG(("%s:CD: Set Filter Connection %x => mode %x parm %04x\n", cpuexec_describe_context(machine), fnum, cr1 & 0xf, cr2))

				// set true condition?
				if (cr1 & 1)
				{
					filters[fnum].condtrue = (cr2>>8)&0xff;
				}
				else if (cr1 & 2)	// set false condition
				{
					filters[fnum].condfalse = cr2&0xff;
				}

				hirqreg |= (CMOK|ESEL);
				cr2 = 0x4101;	// ctrl/adr in hi byte, track # in low byte
				cr3 = 0x0100|((cd_curfad>>16)&0xff);
				cr4 = (cd_curfad & 0xffff);
			}
			break;

		case 0x4800:	// Reset Selector
			CDROM_LOG(("%s:CD: Reset Selector\n",   cpuexec_describe_context(machine)))
			hirqreg |= (CMOK|ESEL|DRDY);
			cr2 = 0x4101;	// ctrl/adr in hi byte, track # in low byte
			cr3 = 0x0100|((cd_curfad>>16)&0xff);
			cr4 = (cd_curfad & 0xffff);
			break;

		case 0x5000:	// get Buffer Size
			cr1 = cd_stat;
			cr2 = freeblocks;
			if (cr2 > 200) cr2 = 200;	// ???

			cr3 = 0x1800;
			cr4 = 200;
			CDROM_LOG(("%s:CD: Get Buffer Size = %d\n", cr2, cpuexec_describe_context(machine)))
			hirqreg |= (CMOK|ESEL|DRDY);	// DRDY is probably wrong
			break;

		case 0x5100:	// get # sectors used in a buffer
			{
				UINT32 bufnum = cr3>>8;

				// is the partition empty?
				if (partitions[bufnum].size == -1)
				{
					cr4 = 0;
				}
				else
				{
					cr4 = partitions[bufnum].numblks;
				}

				CDROM_LOG(("%s:CD: Get Sector Number (bufno %d) = %d blocks\n",   cpuexec_describe_context(machine), bufnum, cr4))

				cr2 = 0;
				cr3 = 0;
				hirqreg |= (CMOK|DRDY);
			}
			break;

		case 0x5200:	// calculate acutal size
			{
				UINT32 bufnum = cr3>>8;
				UINT32 sectoffs = cr2;
				UINT32 numsect = cr4;

				CDROM_LOG(("%s:CD: Calculate actual size: buf %x offs %x numsect %x\n", cpuexec_describe_context(machine), bufnum, sectoffs, numsect))

				calcsize = 0;
				if (partitions[bufnum].size != -1)
				{
					INT32 i;

					for (i = 0; i < numsect; i++)
					{
						if (partitions[bufnum].blocks[sectoffs+i])
						{
							calcsize += (partitions[bufnum].blocks[sectoffs+i]->size / 2);
						}
					}
				}

				hirqreg |= (CMOK|ESEL);
				cr1 = cd_stat;
				cr2 = 0x4101;	// CTRL/track
				cr3 = (cd_curfad>>16)&0xff;
				cr4 = (cd_curfad & 0xffff);
			}
			break;

		case 0x5300:	// get actual block size
			CDROM_LOG(("%s:CD: Get actual block size\n", cpuexec_describe_context(machine)))
			hirqreg |= (CMOK|ESEL);
			cr1 = cd_stat | ((calcsize>>16)&0xff);
			cr2 = (calcsize & 0xffff);
			cr3 = 0;
			cr4 = 0;
			break;

		case 0x6000:	// set sector length
			CDROM_LOG(("%s:CD: Set sector length\n",   cpuexec_describe_context(machine)))
			hirqreg |= (CMOK|ESEL|EFLS|SCDQ|DRDY);

			switch (cr1 & 0xff)
			{
				case 0:
					sectlenin = 2048;
					break;
				case 1:
					sectlenin = 2336;
					break;
				case 2:
					sectlenin = 2340;
					break;
				case 3:
					sectlenin = 2352;
					break;
			}

			switch ((cr2>>8) & 0xff)
			{
				case 0:
					sectlenout = 2048;
					break;
				case 1:
					sectlenout = 2336;
					break;
				case 2:
					sectlenout = 2340;
					break;
				case 3:
					sectlenout = 2352;
					break;
			}
			break;

		case 0x6100:	// get sector data
			{
				UINT32 sectnum = cr4;
				UINT32 sectofs = cr2;
				UINT32 bufnum = cr3>>8;

				CDROM_LOG(("%s:CD: Get sector data (SN %d SO %d BN %d)\n",   cpuexec_describe_context(machine), sectnum, sectofs, bufnum))

				if (bufnum >= MAX_FILTERS)
				{
					CDROM_LOG(("CD: invalid buffer number\n"));
					cd_stat = 0xff;	// ERROR
					hirqreg |= (CMOK|EHST);
					return;
				}

				if (partitions[bufnum].numblks == 0)
				{
					CDROM_LOG(("CD: buffer is empty\n"))
					cd_stat = 0xff;	// ERROR
					hirqreg |= (CMOK|EHST);
					return;
				}

				cd_getsectoroffsetnum(bufnum, &sectofs, &sectnum);

				xfertype32 = XFERTYPE32_GETSECTOR;
				xferoffs = 0;
				xfersect = 0;
				xferdnum = 0;
				xfersectpos = sectofs;
				xfersectnum = sectnum;
				transpart = &partitions[bufnum];

				cd_stat |= CD_STAT_TRANS;
				hirqreg |= (CMOK|EHST|DRDY);
			}
			break;

		case 0x6200:	// delete sector data
			{
				UINT32 sectnum = cr4;
				UINT32 sectofs = cr2;
				UINT32 bufnum = cr3>>8;
				INT32 i;

				CDROM_LOG(("%s:CD: Delete sector data (SN %d SO %d BN %d)\n",   cpuexec_describe_context(machine), sectnum, sectofs, bufnum))

				if (bufnum >= MAX_FILTERS)
				{
					CDROM_LOG(("CD: invalid buffer number\n"))
					cd_stat = 0xff;	// ERROR
					hirqreg |= (CMOK|EHST);
					return;
				}

				if (partitions[bufnum].numblks == 0)
				{
					CDROM_LOG(("CD: buffer is empty\n"))
					cd_stat = 0xff;	// ERROR
					hirqreg |= (CMOK|EHST);
					return;
				}

				cd_getsectoroffsetnum(bufnum, &sectofs, &sectnum);

				for (i = sectofs; i < (sectofs + sectnum); i++)
				{
					partitions[bufnum].size -= partitions[bufnum].blocks[i]->size;
					cd_free_block(partitions[bufnum].blocks[i]);
					partitions[bufnum].blocks[i] = (blockT *)NULL;
					partitions[bufnum].bnum[i] = 0xff;
				}

				cd_defragblocks(&partitions[bufnum]);

				partitions[bufnum].numblks -= sectnum;

				cd_stat &= ~CD_STAT_TRANS;
				hirqreg |= (CMOK|EHST);
			}
			break;

		case 0x6300:	// get then delete sector data
			{
				UINT32 sectnum = cr4;
				UINT32 sectofs = cr2;
				UINT32 bufnum = cr3>>8;

				CDROM_LOG(("%s:CD: Get and delete sector data (SN %d SO %d BN %d)\n",   cpuexec_describe_context(machine), sectnum, sectofs, bufnum))

				if (bufnum >= MAX_FILTERS)
				{
					CDROM_LOG(("CD: invalid buffer number\n"))
					cd_stat = 0xff;	// ERROR
					hirqreg |= (CMOK|EHST);
					return;
				}

				if (partitions[bufnum].numblks == 0)
				{
					CDROM_LOG(("CD: buffer is empty\n"))
					cd_stat = 0xff;	// ERROR
					hirqreg |= (CMOK|EHST);
					return;
				}

				cd_getsectoroffsetnum(bufnum, &sectofs, &sectnum);

				xfertype32 = XFERTYPE32_GETDELETESECTOR;
				xferoffs = 0;
				xfersect = 0;
				xferdnum = 0;
				xfersectpos = sectofs;
				xfersectnum = sectnum;
				transpart = &partitions[bufnum];

				cd_stat &= ~CD_STAT_TRANS;
				hirqreg |= (CMOK|EHST|DRDY);
			}
			break;

		case 0x6700:	// get copy error
			CDROM_LOG(("%s:CD: Get copy error\n",   cpuexec_describe_context(machine)))
			hirqreg |= (CMOK|ESEL|EFLS|SCDQ|DRDY);
			break;

		case 0x7000:	// change directory
			CDROM_LOG(("%s:CD: Change Directory\n",   cpuexec_describe_context(machine)))
			hirqreg |= (CMOK|EFLS);

			temp = (cr3&0xff)<<16;
			temp |= cr4;
			read_new_dir(temp);
			break;

		case 0x7100:	// Read directory entry
			CDROM_LOG(("%s:CD: Read Directory Entry\n",   cpuexec_describe_context(machine)))
			hirqreg |= (CMOK|DRDY);

			temp = (cr3&0xff)<<16;
			temp |= cr4;
			cr2 = 0x4101;	// CTRL/track
			cr3 = (curdir[temp].firstfad>>16)&0xff;
			cr4 = (curdir[temp].firstfad&0xffff);
			break;

		case 0x7200:	// Get file system scope
			CDROM_LOG(("%s:CD: Get file system scope(PC=%x)\n",   cpuexec_describe_context(machine)))
			hirqreg |= (CMOK|DRDY);
			cr2 = numfiles;	// # of files in directory
			cr3 = 0x0100;	// report directory held
			cr4 = firstfile;	// first file id
			break;

		case 0x7300:	// Get File Info
			CDROM_LOG(("%s:CD: Get File Info\n",   cpuexec_describe_context(machine)))
			cd_stat |= CD_STAT_TRANS;
			cd_stat &= 0xff00;		// clear top byte of return value
			hirqreg |= (CMOK|DRDY);

			temp = (cr3&0xff)<<16;
			temp |= cr4;

			if (temp == 0xffffff)	// special
			{
				xfertype = XFERTYPE_FILEINFO_254;
				xfercount = 0;

				cr1 = cd_stat;
				cr2 = 0x5f4;
				cr3 = 0;
				cr4 = 0;
			}
			else
			{
				cr1 = cd_stat;
				cr2 = 6;	// 6 words for single file
							// first 4 bytes = FAD address
							// second 4 bytes = length
							// last 4 bytes:
							// - unit size
							// - gap size
							// - file #
							// attributes flags

				cr3 = cr4 = 0;

				// first 4 bytes = FAD
				finfbuf[0] = (curdir[temp].firstfad>>24)&0xff;
				finfbuf[1] = (curdir[temp].firstfad>>16)&0xff;
				finfbuf[2] = (curdir[temp].firstfad>>8)&0xff;
				finfbuf[3] = (curdir[temp].firstfad&0xff);
		 		// second 4 bytes = length of file
				finfbuf[4] = (curdir[temp].length>>24)&0xff;
				finfbuf[5] = (curdir[temp].length>>16)&0xff;
				finfbuf[6] = (curdir[temp].length>>8)&0xff;
				finfbuf[7] = (curdir[temp].length&0xff);
				finfbuf[8] = 0x00;
				finfbuf[9] = 0x00;
				finfbuf[10] = temp;
				finfbuf[11] = curdir[temp].flags;

				xfertype = XFERTYPE_FILEINFO_1;
				xfercount = 0;
			}
			CDROM_LOG(("   = %04x %04x %04x %04x %04x\n", hirqreg, cr1, cr2, cr3, cr4))
			break;

		case 0x7400:	// Read File
			CDROM_LOG(("%s:CD: Read File\n",   cpuexec_describe_context(machine)))
			temp = (cr3&0xff)<<16;
			temp |= cr4;

			cd_stat = CD_STAT_PLAY|0x80;	// set "cd-rom" bit
			cr2 = 0x4101;	// CTRL/track
			cr3 = (curdir[temp].firstfad>>16)&0xff;
			cr4 = (curdir[temp].firstfad&0xffff);

			cd_curfad = curdir[temp].firstfad;
			if (curdir[temp].length / 2048)
			{
				fadstoplay = curdir[temp].length/2048;
				fadstoplay++;
			}
			else
			{
				fadstoplay = curdir[temp].length/2048;
			}

			hirqreg |= (CMOK|DRDY);
			oddframe = 0;
			in_buffer = 0;

			playtype = 1;

			// and do the disc I/O
//          timer_adjust_oneshot(sector_timer, ATTOTIME_IN_HZ(150), 0);  // 150 sectors / second = 300kBytes/second
			break;

		case 0x7500:
			CDROM_LOG(("%s:CD: Abort File\n",   cpuexec_describe_context(machine)))
			// bios expects "2bc" mask to work against this
			hirqreg |= (CMOK|EFLS|EHST|ESEL|DCHG|PEND|BFUL|CSCT|DRDY);
			cd_stat = CD_STAT_PERI|CD_STAT_PAUSE;	// force to pause
			break;

		case 0xe000:	// appears to be copy protection check.  needs only to return OK.
			CDROM_LOG(("%s:CD: Verify copy protection\n",   cpuexec_describe_context(machine)))
			cd_stat = CD_STAT_PAUSE;
			cr1 = cd_stat;	// necessary to pass
			cr2 = 0x4;
//          hirqreg |= (CMOK|EFLS|CSCT);
			sectorstore = 1;
			hirqreg = 0xfc5;
			break;

		case 0xe100:	// get disc region
			CDROM_LOG(("%s:CD: Get disc region\n",   cpuexec_describe_context(machine)))
			cd_stat = CD_STAT_PAUSE;
			cr1 = cd_stat;	// necessary to pass
			cr2 = 0x4;		// (must return this value to pass bios checks)
			hirqreg |= (CMOK);
			break;

		default:
			CDROM_LOG(("%s:CD: Unknown command %04x\n", cr1,   cpuexec_describe_context(machine)))
			hirqreg |= (CMOK);
			break;
		}
//      CDROM_LOG(("ret: %04x %04x %04x %04x %04x\n", hirqreg, cr1, cr2, cr3, cr4))
		break;
	default:
		CDROM_LOG(("%s:CD: WW %08x %04x\n", addr, data, cpuexec_describe_context(machine)))
		break;
	}
}

READ32_HANDLER( stvcd_r )
{
	UINT32 rv = 0;

	offset <<= 2;

	switch (offset)
	{
		case 0x90008:
		case 0x9000a:
		case 0x9000c:
		case 0x9000e:
		case 0x90018:
		case 0x9001a:
		case 0x9001c:
		case 0x9001e:
		case 0x90020:
		case 0x90022:
		case 0x90024:
		case 0x90026:
			rv = cd_readWord(offset);
			return rv<<16;

		case 0x98000:
		case 0x18000:
			if (mem_mask == 0xffffffff)
			{
				rv = cd_readLong(offset);
			}
			else if (mem_mask == 0xffff0000)
			{
				rv = cd_readWord(offset)<<16;
			}
			else if (mem_mask == 0x0000ffff)
			{
				rv = cd_readWord(offset);
			}
			else
			{
				mame_printf_error("CD: Unknown data buffer read @ mask = %08x\n", mem_mask);
			}

			break;

		default:
			CDROM_LOG(("Unknown CD read @ %x\n", offset))
			break;
	}

	return rv;
}

WRITE32_HANDLER( stvcd_w )
{
	offset <<= 2;

	switch (offset)
	{
		case 0x90008:
		case 0x9000a:
		case 0x9000c:
		case 0x9000e:
		case 0x90018:
		case 0x9001a:
		case 0x9001c:
		case 0x9001e:
		case 0x90020:
		case 0x90022:
		case 0x90024:
		case 0x90026:
			cd_writeWord(offset, data>>16);
			break;

		default:
			CDROM_LOG(("Unknown CD write %x @ %x\n", data, offset))
			break;
	}
}

// iso9660 parsing
static void read_new_dir(UINT32 fileno)
{
	int foundpd, i;
	UINT32 cfad, dirfad;
	UINT8 sect[2048];

	if (fileno == 0xffffff)
	{
		cfad = 166;		// first sector of directory as per iso9660 specs

		foundpd = 0;	// search for primary vol. desc
		while ((!foundpd) && (cfad < 200))
		{
			memset(sect, 0, 2048);
			cd_readblock(cfad++, sect);

			if ((sect[1] == 'C') && (sect[2] == 'D') && (sect[3] == '0') && (sect[4] == '0') && (sect[5] == '1'))
			{
			 	switch (sect[0])
				{
					case 0:	// boot record
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
			dirfad = sect[140] | (sect[141]<<8) | (sect[142]<<16) | (sect[143]<<24);
			dirfad += 150;

			// parse root entry
			curroot.firstfad = sect[158] | (sect[159]<<8) | (sect[160]<<16) | (sect[161]<<24);
			curroot.firstfad += 150;
			curroot.length = sect[166] | (sect[167]<<8) | (sect[168]<<16) | (sect[169]<<24);
			curroot.flags = sect[181];
			for (i = 0; i < sect[188]; i++)
			{
				curroot.name[i] = sect[189+i];
			}
			curroot.name[i] = '\0';	// terminate

			// easy to fix, but make sure we *need* to first
			if (curroot.length > MAX_DIR_SIZE)
			{
				mame_printf_error("ERROR: root directory too big (%d)\n", curroot.length);
			}

			// done with all that, read the root directory now
			make_dir_current(curroot.firstfad);
		}
	}
	else
	{
		if (curdir[fileno].length > MAX_DIR_SIZE)
		{
			mame_printf_error("ERROR: new directory too big (%d)!\n", curdir[fileno].length);
		}
		make_dir_current(curdir[fileno].firstfad);
	}
}

// makes the directory pointed to by FAD current
static void make_dir_current(UINT32 fad)
{
	int i;
	UINT32 nextent, numentries;
	UINT8 sect[MAX_DIR_SIZE];
	direntryT *curentry;

	memset(sect, 0, MAX_DIR_SIZE);
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

	if (curdir != (direntryT *)NULL)
	{
		free((void *)curdir);
	}

	curdir = alloc_array_or_die(direntryT, numentries);
	curentry = curdir;
	numfiles = numentries;

	nextent = 0;
	while (numentries)
	{
		curentry->firstfad = sect[nextent+2] | (sect[nextent+3]<<8) | (sect[nextent+4]<<16) | (sect[nextent+5]<<24);
		curentry->firstfad += 150;
		curentry->length = sect[nextent+10] | (sect[nextent+11]<<8) | (sect[nextent+12]<<16) | (sect[nextent+13]<<24);
		curentry->flags = sect[nextent+25];
		for (i = 0; i < sect[nextent+32]; i++)
		{
			curentry->name[i] = sect[nextent+33+i];
		}
		curentry->name[i] = '\0';	// terminate

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

void stvcd_exit(running_machine* machine)
{
	if (curdir != (direntryT *)NULL)
	{
		free((void *)curdir);
		curdir = (direntryT *)NULL;
	}

	if (cdrom)
	{
		#ifndef MESS
		cdrom_close(cdrom);
		#endif
		cdrom = (cdrom_file *)NULL;
	}
}

static void cd_readTOC(void)
{
	int i, ntrks, toclen, tocptr, fad;

	xfertype = XFERTYPE_TOC;
	xfercount = 0;

	if (cdrom)
	{
		ntrks = cdrom_get_last_track(cdrom);
	}
	else
	{
		ntrks = 0;
	}

	toclen = (4 * ntrks);	// toclen header entry

	// data format for Saturn TOC:
	// no header.
	// 4 bytes per track
	// top nibble of first byte is CTRL info
	// low nibble is ADR
	// next 3 bytes are FAD address (LBA + 150)
	// there are always 99 track entries (0-98)
	// unused tracks are ffffffff.
	// entries 99-101 are metadata

	tocptr = 0;	// starting point of toc entries

	for (i = 0; i < ntrks; i++)
	{
		if (cdrom)
		{
			tocbuf[tocptr] = cdrom_get_adr_control(cdrom, i)<<4 | 0x01;
		}

		if (cdrom)
		{
			fad = cdrom_get_track_start(cdrom, i) + 150;
		}
		else
		{
			fad = 150;
		}

		tocbuf[tocptr+1] = (fad>>16)&0xff;
		tocbuf[tocptr+2] = (fad>>8)&0xff;
		tocbuf[tocptr+3] = fad&0xff;

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
	tocbuf[tocptr] = tocbuf[0];	// get ctrl/adr from first track
	tocbuf[tocptr+1] = 1;	// first track's track #
	tocbuf[tocptr+2] = 0;
	tocbuf[tocptr+3] = 0;

	tocbuf[tocptr+4] = tocbuf[(ntrks-1)*4];	// ditto for last track
	tocbuf[tocptr+5] = ntrks;	// last track's track #
	tocbuf[tocptr+6] = 0;
	tocbuf[tocptr+7] = 0;

	// get total disc length (start of lead-out)
	fad = cdrom_get_track_start(cdrom, 0xaa) + 150;

	tocbuf[tocptr+8] = tocbuf[0];
	tocbuf[tocptr+9]  = (fad>>16)&0xff;
	tocbuf[tocptr+10] = (fad>>8)&0xff;
	tocbuf[tocptr+11] = fad&0xff;
}

static partitionT *cd_filterdata(filterT *flt, int trktype)
{
	int match = 1, keepgoing = 2;
	partitionT *filterprt = (partitionT *)NULL;

	CDROM_LOG(("cd_filterdata, trktype %d\n", trktype))

	// loop on the filters
	do
	{
		if ((trktype != CD_TRACK_AUDIO) && (curblock.data[15] == 2))
		{
			if (flt->mode & 1)	// file number
			{
				if (curblock.fnum != flt->fid)
				{
					logerror("fnum reject\n");
					match = 0;
				}
			}

			if (flt->mode & 2)	// channel number
			{
				mame_printf_error("STVCD: unimplemented channel number match\n");
			}

			if (flt->mode & 4)	// sub mode
			{
				mame_printf_error("STVCD: unimplemented sub mode match\n");
			}

			if (flt->mode & 8)	// coding information
			{
				mame_printf_error("STVCD: unimplemented coding information match\n");
			}

			if (flt->mode & 0x10)	// reverse subheader conditions
			{
				match ^= 1;
			}
		}

		// FAD range check?
		if (flt->mode & 0x40)
		{
			if ((cd_curfad < flt->fad) || (cd_curfad > (flt->fad + flt->range)))
			{
				logerror("curfad reject\n");
				match = 0;
			}
		}

		if (match)
		{
			lastbuf = flt->condtrue;
			filterprt = &partitions[lastbuf];
			// we're done
			keepgoing = 0;
		}
		else
		{
			lastbuf = flt->condfalse;

			// reject sector if no match on either connector
			if ((lastbuf == 0xff) || (keepgoing < 2))
			{
				return (partitionT *)NULL;
			}

			// try again using the filter that was on the "false" connector
			flt = &filters[lastbuf];

			// and exit if we fail
			keepgoing--;
		}
	} while (keepgoing);

	// try to allocate a block
	filterprt->blocks[filterprt->numblks] = cd_alloc_block(&filterprt->bnum[filterprt->numblks]);

	// did the allocation succeed?
	if (filterprt->blocks[filterprt->numblks] == (blockT *)NULL)
	{
		return (partitionT *)NULL;
	}

	// copy working block to the newly allocated one
	memcpy(filterprt->blocks[filterprt->numblks], &curblock, sizeof(blockT));

	// and massage the data format a bit
	switch  (curblock.size)
	{
		case 2048:	// user data
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

		case 2324:	// Mode 2 Form 2 data
			memcpy(&filterprt->blocks[filterprt->numblks]->data[0], &curblock.data[24], curblock.size);
			break;

		case 2336:	// Mode 2 Form 2 skip sync/header
			memcpy(&filterprt->blocks[filterprt->numblks]->data[0], &curblock.data[16], curblock.size);
			break;

		case 2340:	// Mode 2 Form 2 skip sync only
			memcpy(&filterprt->blocks[filterprt->numblks]->data[0], &curblock.data[12], curblock.size);
			break;

		case 2352:	// want all data, it's already done, so don't do it again :)
			break;
	}

	// update the status of the partition
	if (filterprt->size == -1)
	{
		filterprt->size = 0;
	}
	filterprt->size += filterprt->blocks[filterprt->numblks]->size;
	filterprt->numblks++;

	return filterprt;
}

// read a single sector off the CD, applying the current filter(s) as necessary
static partitionT *cd_read_filtered_sector(INT32 fad)
{
	int trktype;

	if ((cddevice != NULL) && (!buffull))
	{
		// find out the track's type
		trktype = cdrom_get_track_type(cdrom, cdrom_get_track(cdrom, fad-150));

		// now get a raw 2352 byte sector
		cdrom_read_data(cdrom, fad-150, curblock.data, CD_TRACK_RAW_DONTCARE);
		cr3 = 0x100 | (fad>>16);	// update cr3/4 with the current fad
		cr4 = fad;
		curblock.size = sectlenin;
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

		return cd_filterdata(cddevice, trktype);
	}

	return (partitionT *)NULL;
}

// loads in data set up by a CD-block PLAY command
static void cd_playdata(void)
{
	if ((cd_stat & 0x0f00) == CD_STAT_PLAY)
	{
		if (fadstoplay)
		{
          		logerror("STVCD: Reading FAD %d\n", cd_curfad);

			if (cdrom)
			{
				partitionT *playpart;

				playpart = cd_read_filtered_sector(cd_curfad);

				cd_curfad++;
				fadstoplay--;

				hirqreg |= CSCT;

				if (!fadstoplay)
				{
					CDROM_LOG(("cd_playdata: playback ended\n"))
					cd_stat = CD_STAT_PAUSE;

					hirqreg |= PEND;

					if (playtype == 1)
					{
						CDROM_LOG(("cd_playdata: setting EFLS\n"))
						hirqreg |= EFLS;
					}
				}
			}
		}
	}
}

// loads a single sector off the CD, anywhere from FAD 150 on up
static void cd_readblock(UINT32 fad, UINT8 *dat)
{
	if (cdrom)
	{
		cdrom_read_data(cdrom, fad-150, dat, CD_TRACK_MODE1);
	}
}



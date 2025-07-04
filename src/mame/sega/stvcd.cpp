// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont
/***************************************************************************

  sega/stvcd.cpp - Sega Saturn and ST-V CD-ROM handling

  Another tilt at the windmill in 2011 by R. Belmont.

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

============================================================================
TODO:
- finish off code cleanups (repetition etc.);
- improve debugging;
- fix DRQ behaviour, several softwares gets to the point of filling
  the buffer (and probably don't know what to do);
- fix startup, cfr. cdblock branch;
- merge common components with lle version via superclass (i.e. comms);
- derive MPEG commands in a subdevice;

DASM notes:
* whizzj:
- wpset 0x605e4b8,4,w,wpdata==0x4e
  (second trigger)
- write to 0x605e498 -> 1
  (PUBLISH.CPK tries to playback a few frames then keeps looping)

***************************************************************************/

#include "emu.h"
#include "stvcd.h"

#include "coreutil.h"
#include "multibyte.h"

#define LOG_WARN           (1U << 1)
#define LOG_CMD            (1U << 2)
#define LOG_SEEK           (1U << 3)
#define LOG_XFER           (1U << 4)

#define VERBOSE (LOG_CMD | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

#define LOGWARN(...)         LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGCMD(...)          LOGMASKED(LOG_CMD, __VA_ARGS__)
#define LOGSEEK(...)         LOGMASKED(LOG_SEEK, __VA_ARGS__)
#define LOGXFER(...)         LOGMASKED(LOG_XFER, __VA_ARGS__)

#define LIVE_CD_VIEW    0

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

DEFINE_DEVICE_TYPE(STVCD, stvcd_device, "stvcd", "Sega Saturn/ST-V CD Block HLE")

stvcd_device::stvcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, STVCD, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_space_config("regs", ENDIANNESS_LITTLE, 32, 20, 0, address_map_constructor(FUNC(stvcd_device::io_regs), this))
	, m_cdrom_image(*this, "cdrom")
	, m_sector_timer(*this, "sector_timer")
	, m_sh1_timer(*this, "sh1_cmd")
	, m_cdda(*this, "cdda")
{
}


void stvcd_device::device_add_mconfig(machine_config &config)
{
	CDROM(config, "cdrom").set_interface("cdrom");

	TIMER(config, m_sector_timer).configure_generic(FUNC(stvcd_device::stv_sector_cb));
	TIMER(config, m_sh1_timer).configure_generic(FUNC(stvcd_device::stv_sh1_sim));

	CDDA(config, m_cdda);
	m_cdda->add_route(0, *this, 1.0, 0);
	m_cdda->add_route(1, *this, 1.0, 1);
	m_cdda->set_cdrom_tag("cdrom");
}

void stvcd_device::device_start()
{
}

device_memory_interface::space_config_vector stvcd_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

/*
 * Block interface
 */

void stvcd_device::io_regs(address_map &map)
{
	map(0x18000, 0x18003).rw(FUNC(stvcd_device::datatrns_r), FUNC(stvcd_device::datatrns_w));
	map(0x90000, 0x90003).mirror(0x08000).rw(FUNC(stvcd_device::datatrns_r), FUNC(stvcd_device::datatrns_w));
	map(0x90008, 0x9000b).mirror(0x08000).rw(FUNC(stvcd_device::hirq_r), FUNC(stvcd_device::hirq_w)).umask32(0xffffffff);
	map(0x9000c, 0x9000f).mirror(0x08000).rw(FUNC(stvcd_device::hirqmask_r), FUNC(stvcd_device::hirqmask_w)).umask32(0xffffffff);
	map(0x90018, 0x9001b).mirror(0x08000).rw(FUNC(stvcd_device::cr1_r), FUNC(stvcd_device::cr1_w)).umask32(0xffffffff);
	map(0x9001c, 0x9001f).mirror(0x08000).rw(FUNC(stvcd_device::cr2_r), FUNC(stvcd_device::cr2_w)).umask32(0xffffffff);
	map(0x90020, 0x90023).mirror(0x08000).rw(FUNC(stvcd_device::cr3_r), FUNC(stvcd_device::cr3_w)).umask32(0xffffffff);
	map(0x90024, 0x90027).mirror(0x08000).rw(FUNC(stvcd_device::cr4_r), FUNC(stvcd_device::cr4_w)).umask32(0xffffffff);

	// NetLink access
	// dragndrm expects this value, most likely for status
	map(0x8502a, 0x8502a).lr8(NAME([] () -> u8 { return 0x11; }));
}

u32 stvcd_device::datatrns_r(offs_t offset, uint32_t mem_mask)
{
	u32 rv;

	if (mem_mask == 0xffffffff)
	{
		rv = dataxfer_long_r();
	}
	else if (mem_mask == 0xffff0000)
	{
		rv = dataxfer_word_r()<<16;
	}
	else if (mem_mask == 0x0000ffff)
	{
		rv = dataxfer_word_r();
	}
	else
	{
		LOGWARN("CD: Unknown data buffer read with mask = %08x\n", mem_mask);
		rv = 0;
	}
	return rv;
}

void stvcd_device::datatrns_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (mem_mask == 0xffffffff)
		dataxfer_long_w(data);
	else
		LOGWARN("CD: Unknown data buffer write with mask = %08x\n", mem_mask);
}

inline u32 stvcd_device::dataxfer_long_r()
{
	uint32_t rv = 0;

	switch (xfertype32)
	{
		case XFERTYPE32_GETSECTOR:
		case XFERTYPE32_GETDELETESECTOR:
			// make sure we have sectors left
			if (xfersect < xfersectnum)
			{
				// get next longword
				rv = get_u32be(&transpart->blocks[xfersectpos+xfersect]->data[xferoffs]);

				xferdnum += 4;
				xferoffs += 4;

				// did we run out of sector?
				if (xferoffs >= transpart->blocks[xfersect]->size)
				{
					LOG("Finished xfer of block %d of %d\n", xfersect+1, xfersectnum);

					xferoffs = 0;
					xfersect++;
				}
			}
			else    // sectors are done, kill 'em all if we can
			{
				if (xfertype32 == XFERTYPE32_GETDELETESECTOR)
				{
					int32_t i;

					LOG("Killing sectors in done\n");

					// deallocate the blocks
					for (i = xfersectpos; i < xfersectpos+xfersectnum; i++)
					{
						cd_free_block(transpart->blocks[i]);
						transpart->blocks[i] = (blockT *)nullptr;
						transpart->bnum[i] = 0xff;
					}

					// defrag what's left
					cd_defragblocks(transpart);

					// clean up our state
					transpart->size -= xferdnum;
					transpart->numblks -= xfersectnum;

					// TODO: is this correct?
					xfertype32 = XFERTYPE32_INVALID;
				}
			}
			break;

		default:
			LOGWARN("CD: unhandled 32-bit transfer type %d\n", (int)xfertype32);
			break;
	}

	return rv;
}

inline void stvcd_device::dataxfer_long_w(u32 data)
{
	switch (xfertype32)
	{
		case XFERTYPE32_PUTSECTOR:
			// make sure we have sectors left
			if (xfersect < xfersectnum)
			{
				// get next longword
				put_u32be(&transpart->blocks[xfersectpos+xfersect]->data[xferoffs], data);

				xferdnum += 4;
				xferoffs += 4;

				// did we run out of sector?
				if (xferoffs >= transpart->blocks[xfersectpos+xfersect]->size)
				{
					LOG("Finished xfer of block %d of %d\n", xfersect+1, xfersectnum);

					xferoffs = 0;
					xfersect++;
				}
			}
			else    // sectors are done
			{
				/* Virtual On doesnt want this to be resetted. */
				//xfertype32 = XFERTYPE32_INVALID;
			}
			break;

		default:
			LOGWARN("CD: unhandled 32-bit transfer type write %d\n", (int)xfertype32);
			break;
	}
}

inline u16 stvcd_device::dataxfer_word_r()
{
	u16 rv;

	rv = 0xffff;
	switch (xfertype)
	{
		case XFERTYPE_TOC:
			rv = get_u16be(&tocbuf[xfercount]);

			xfercount += 2;
			xferdnum += 2;

			if (xfercount > 102*4)
			{
				xfercount = 0;
				xfertype = XFERTYPE_INVALID;
			}
			break;

		case XFERTYPE_FILEINFO_1:
			rv = get_u16be(&finfbuf[xfercount]);
			xfercount += 2;
			xferdnum += 2;

			if (xfercount > 6*2)
			{
				xfercount = 0;
				xfertype = XFERTYPE_INVALID;
			}
			break;

		case XFERTYPE_FILEINFO_254: // Lunar 2
			if((xfercount % (6 * 2)) == 0)
			{
				uint32_t temp = 2 + (xfercount / (0x6 * 2));

				// first 4 bytes = FAD
				put_u32be(&finfbuf[0], curdir[temp].firstfad);
				// second 4 bytes = length of file
				put_u32be(&finfbuf[4], curdir[temp].length);
				finfbuf[8] = curdir[temp].interleave_gap_size;
				finfbuf[9] = curdir[temp].file_unit_size;
				finfbuf[10] = temp;
				finfbuf[11] = curdir[temp].flags;
			}

			rv = get_u16be(&finfbuf[xfercount % (6 * 2)]);

			xfercount += 2;
			xferdnum += 2;

			if (xfercount > (254 * 6 * 2))
			{
				xfercount = 0;
				xfertype = XFERTYPE_INVALID;
			}
			break;

		case XFERTYPE_SUBQ:
			rv = get_u16be(&subqbuf[xfercount]);

			xfercount += 2;
			xferdnum += 2;

			if (xfercount > 5*2)
			{
				xfercount = 0;
				xfertype = XFERTYPE_INVALID;
			}
			break;


		case XFERTYPE_SUBRW:
			rv = get_u16be(&subrwbuf[xfercount]);

			xfercount += 2;
			xferdnum += 2;

			if (xfercount > 12*2)
			{
				xfercount = 0;
				xfertype = XFERTYPE_INVALID;
			}
			break;

		default:
			LOGWARN("STVCD: Unhandled xfer type %d\n", (int)xfertype);
			rv = 0;
			break;
	}

	return rv;
}

uint16_t stvcd_device::hirq_r()
{
	// TODO: this member must return the register only
	u16 rv;

//  LOG("RW HIRQ: %04x\n", rv);

	rv = hirqreg;

	rv &= ~DCHG;    // always clear bit 6 (tray open)

	if (buffull) rv |= BFUL; else rv &= ~BFUL;
	if (sectorstore) rv |= CSCT; else rv &= ~CSCT;

	hirqreg = rv;

	return rv;
}

void stvcd_device::hirq_w(uint16_t data) { hirqreg &= data; }

// TODO: these two are actually never read or written to by host?
uint16_t stvcd_device::hirqmask_r()
{
	LOGWARN("RW HIRM: %04x\n", hirqmask);
	return hirqmask;
}

void stvcd_device::hirqmask_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGWARN("WW HIRM: %04x => %04x\n", hirqmask, data);
	COMBINE_DATA(&hirqmask);
}

uint16_t stvcd_device::cr1_r() { return cr1; }
uint16_t stvcd_device::cr2_r() { return cr2; }
uint16_t stvcd_device::cr3_r() { return cr3; }
uint16_t stvcd_device::cr4_r()
{
	cmd_pending = 0;
	cd_stat |= CD_STAT_PERI;
	return cr4;
}

// TODO: understand how dual-port interface really works out
void stvcd_device::cr1_w(uint16_t data)
{
	cr1 = data;
	cd_stat &= ~CD_STAT_PERI;
	cmd_pending |= 1;
	m_sh1_timer->adjust(attotime::never);
}

void stvcd_device::cr2_w(uint16_t data)
{
	cr2 = data;
	cmd_pending |= 2;
}

void stvcd_device::cr3_w(uint16_t data)
{
	cr3 = data;
	cmd_pending |= 4;
}

void stvcd_device::cr4_w(uint16_t data)
{
	cr4 = data;
	cmd_pending |= 8;
	m_sh1_timer->adjust(attotime::from_hz(get_timing_command()));
}

uint32_t stvcd_device::stvcd_r(offs_t offset, uint32_t mem_mask)
{
	return this->space().read_dword(offset<<2, mem_mask);
}

void stvcd_device::stvcd_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	this->space().write_dword(offset<<2,data, mem_mask);
}

/*
 * CDC command helpers
 */
int stvcd_device::get_timing_command(void)
{
	// TODO: calculate timings based off command params
	// given the CMOK returns it looks like SH2 expects way slower responses
	// (loops for 0x7xx times at most, max number of iterations is 0x240000)
	return 16667;
}

/* FIXME: assume Saturn CD-ROMs to have a 2 secs pre-gap for now. */
int stvcd_device::get_track_index(uint32_t fad)
{
	uint32_t rel_fad;
	uint8_t track;

	if(m_cdrom_image->get_track_type(m_cdrom_image->get_track(fad)) != cdrom_file::CD_TRACK_AUDIO)
		return 1;

	track = m_cdrom_image->get_track( fad );

	rel_fad = fad - m_cdrom_image->get_track_start( track );

	if(rel_fad < 150)
		return 0;

	return 1;
}

int stvcd_device::sega_cdrom_get_adr_control(int track)
{
	return bitswap<8>(m_cdrom_image->get_adr_control(cur_track),3,2,1,0,7,6,5,4);
}

void stvcd_device::cr_standard_return(uint16_t cur_status)
{
	if (!m_cdrom_image->exists())
	{
		cr1 = cur_status;
		cr2 = 0;
		cr3 = 0;
		cr4 = 0;
	}
	else if ((cd_stat & 0x0f00) == CD_STAT_SEEK)
	{
		/* During seek state, values returned are from the target position */
		uint8_t seek_track = m_cdrom_image->get_track(cd_fad_seek-150);

		cr1 = cur_status | (playtype << 7) | 0x00 | (cdda_repeat_count & 0xf);
		cr2 =  (seek_track == 0xff) ? 0xffff : ((sega_cdrom_get_adr_control(seek_track)<<8) | seek_track);
		cr3 = (get_track_index(cd_fad_seek)<<8) | (cd_fad_seek>>16); //index & 0xff00
		cr4 = cd_fad_seek;
	}
	else
	{
		cr1 = cur_status | (playtype << 7) | 0x00 | (cdda_repeat_count & 0xf); //options << 4 | repeat & 0xf
		cr2 = (cur_track == 0xff) ? 0xffff : ((sega_cdrom_get_adr_control(cur_track)<<8) | (m_cdrom_image->get_track(cd_curfad-150)+1));
		cr3 = (get_track_index(cd_curfad)<<8) | (cd_curfad>>16); //index & 0xff00
		cr4 = cd_curfad;
	}
}

void stvcd_device::mpeg_standard_return(uint16_t cur_status)
{
	cr1 = cur_status | 0x01;
	cr2 = 0; // V-Counter
	cr3 = (0 << 8) | 0x10; // Picture Info | audio status
	cr4 = 0x1000; // video status
}

/*
 * CDC commands
 */

void stvcd_device::cmd_get_status()
{
	//LOGCMD("%s: Get Status\n", machine().describe_context();
	hirqreg |= CMOK;
	if(status_type == 0)
		cr_standard_return(cd_stat);
	else
	{
		cr1 = (cd_stat) | (prev_cr1 & 0xff);
		cr2 = prev_cr2;
		cr3 = prev_cr3;
		cr4 = prev_cr4;
		status_type = 0; /* Road Blaster and friends needs this otherwise they won't boot. */
	}
	//LOG("   = %04x %04x %04x %04x %04x\n", hirqreg, cr1, cr2, cr3, cr4);
}

void stvcd_device::cmd_get_hw_info()
{
	LOGCMD("%s: Get Hardware Info\n", machine().describe_context());
	hirqreg |= CMOK;
	cr1 = cd_stat;
	cr2 = 0x0201;
	cr3 = 0x0000;
	cr4 = 0x0400;
	status_type = 0;
}

void stvcd_device::cmd_get_toc()
{
	LOGCMD("%s: Get TOC\n", machine().describe_context());
	cd_readTOC();
	cd_stat = CD_STAT_TRANS|CD_STAT_PAUSE;
	cr1 = cd_stat;
	cr2 = 102*2;    // TOC length in words (102 entries @ 2 words/4bytes each)
	cr3 = 0;
	cr4 = 0;
	xferdnum = 0;
	hirqreg |= (CMOK|DRDY);
	status_type = 0;
}

void stvcd_device::cmd_get_session_info()
{
	// get session info (lower byte = session # to get?)
	// bios is interested in returns in cr3 and cr4
	// cr3 should be data track #
	// cr4 must be > 1 and < 100 or bios gets angry.
	LOGCMD("%s: Get Session Info\n", machine().describe_context());
	// TODO: shouldn't really read from TOC
	cd_readTOC();
	switch (cr1 & 0xff)
	{
		case 0: // get total session info / disc end
			cd_stat = CD_STAT_PAUSE;
			cr1 = cd_stat;
			cr2 = 0;
			cr3 = 0x0100 | tocbuf[(101*4)+1];
			cr4 = get_u16be(&tocbuf[(101*4)+2]);
			break;

		case 1: // get total session info / disc start
			cd_stat = CD_STAT_PAUSE;
			cr1 = cd_stat;
			cr2 = 0;
			cr3 = 0x0100;   // sessions in high byte, session start in lower
			cr4 = 0;
			break;

		default:
			LOGWARN("CD: Unknown request to Get Session Info %x\n", cr1 & 0xff);
			cr1 = cd_stat;
			cr2 = 0;
			cr3 = 0;
			cr4 = 0;
			break;
	}

	hirqreg |= (CMOK);
	status_type = 0;
}

void stvcd_device::cmd_init_cdsystem()
{
	// TODO: double check this
	// initialize CD system
	// CR1 & 1 = reset software
	// CR1 & 2 = decode RW subcode
	// CR1 & 4 = don't confirm mode 2 subheader
	// CR1 & 8 = retry reading mode 2 sectors
	// CR1 & 10 = force single-speed
	// CR1 & 80 = no change flag (done by Assault Suit Leynos 2)
	LOGCMD("%s: Initialize CD system\n", machine().describe_context());
	//if((cr1 & 0x81) == 0x00) //guess TODO: nope, Choice Cuts doesn't like it, it crashes if you try to skip the FMV otherwise.
	{
		if(((cd_stat & 0x0f00) != CD_STAT_NODISC) && ((cd_stat & 0x0f00) != CD_STAT_OPEN))
		{
			cd_stat = CD_STAT_PAUSE;
			cd_curfad = 150;
			//cur_track = 1;
			fadstoplay = 0;
		}
		in_buffer = 0;
		buffull = 0;
		hirqreg &= 0xffe5;
		cd_speed = (cr1 & 0x10) ? 1 : 2;

		/* reset filter connections */
		/* Guess: X-Men COTA sequence is 0x48->0x48->0x04(01)->0x04(00)->0x30 then 0x10, without this game throws a FAD reject error */
		/* X-Men vs. SF is even fussier, sequence is  0x04 (1) 0x04 (0) 0x03 (0) 0x03 (1) 0x30 */
		#if 0
		for(int i=0;i<MAX_FILTERS;i++)
		{
			filters[i].fad = 0;
			filters[i].range = 0xffffffff;
			filters[i].mode = 0;
			filters[i].chan = 0;
			filters[i].smmask = 0;
			filters[i].cimask = 0;
			filters[i].fid = 0;
			filters[i].smval = 0;
			filters[i].cival = 0;
		}
		#endif

		/* reset CD device connection */
		//cddevice = (filterT *)nullptr;
	}

	hirqreg |= (CMOK|ESEL);
	cr_standard_return(cd_stat);
	status_type = 0;
}

void stvcd_device::cmd_end_data_transfer()
{
	// end data transfer (TODO: needs to be worked on!)
	// returns # of bytes transferred (24 bits) in
	// low byte of cr1 (MSB) and cr2 (middle byte, LSB)
	LOGXFER("%s: End data transfer (%d bytes xfer'd)\n", machine().describe_context(), xferdnum);

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
		LOGWARN("No xferdnum error\n");
		cr1 = (cd_stat) | (0xff);   // is this right?
		cr2 = 0xffff;
		cr3 = 0;
		cr4 = 0;
	}

	// try to clean up any transfers still in progress
	switch (xfertype32)
	{
		case XFERTYPE32_GETSECTOR:
		case XFERTYPE32_PUTSECTOR:
			hirqreg |= EHST;
			break;

		case XFERTYPE32_GETDELETESECTOR:
			if (transpart->size > 0)
			{
				int32_t i;

				xfertype32 = XFERTYPE32_INVALID;

				// deallocate the blocks
				for (i = xfersectpos; i < xfersectpos+xfersectnum; i++)
				{
					cd_free_block(transpart->blocks[i]);
					transpart->blocks[i] = (blockT *)nullptr;
					transpart->bnum[i] = 0xff;
				}

				// defrag what's left
				cd_defragblocks(transpart);

				// clean up our state
				transpart->size -= xferdnum;
				transpart->numblks -= xfersectnum;

				if (freeblocks == 200)
				{
					sectorstore = 0;
				}

				hirqreg |= EHST;
			}
			break;

		default:
			break;
	}


	xferdnum = 0;
	hirqreg |= CMOK;

	LOGXFER("\t%04x %04x %04x %04x %04x\n", hirqreg, cr1, cr2, cr3, cr4);
	status_type = 1;
}

void stvcd_device::cmd_play_disc()
{
	// Play Disc. FAD is in lowest 7 bits of cr1 and all of cr2.
	uint32_t start_pos, end_pos;
	uint8_t play_mode;

	LOGCMD("%s: Play Disc\n",   machine().describe_context());
	cd_stat = CD_STAT_PLAY;

	play_mode = (cr3 >> 8) & 0x7f;

	if (!(cr3 & 0x8000))    // preserve current position if bit 7 set
	{
		start_pos = ((cr1&0xff)<<16) | cr2;
		end_pos = ((cr3&0xff)<<16) | cr4;

		if (start_pos & 0x800000)
		{
			if (start_pos != 0xffffff)
				cd_curfad = start_pos & 0xfffff;

			LOGCMD("\tFAD mode\n");
			cur_track = m_cdrom_image->get_track(cd_curfad-150);
		}
		else
		{
			// track mode
			if(((start_pos)>>8) != 0)
			{
				cur_track = (start_pos)>>8;
				cd_fad_seek = m_cdrom_image->get_track_start(cur_track-1);
				cd_stat = CD_STAT_SEEK;
				m_cdda->pause_audio(0);
			}
			else
			{
				// FIXME: Waku Waku 7 sets up track 0, that basically doesn't make any sense. Just skip it for now.
				popmessage("Warning: track mode == 0, contact MAMEdev");
				cr_standard_return(cd_stat);
				hirqreg |= (CMOK);
				return;
			}

			LOGCMD("\ttrack mode %d\n",cur_track);
		}

		if (end_pos & 0x800000)
		{
			if (end_pos != 0xffffff)
				fadstoplay = end_pos & 0xfffff;
		}
		else
		{
			uint8_t end_track;

			end_track = (end_pos)>>8;
			fadstoplay = m_cdrom_image->get_track_start(end_track) - cd_fad_seek;
		}
	}
	else    // play until the end of the disc
	{
		start_pos = ((cr1&0xff)<<16) | cr2;
		end_pos = ((cr3&0xff)<<16) | cr4;

		if(start_pos != 0xffffff)
		{
			/* Madou Monogatari sets 0xff80xxxx as end position, needs investigation ... */
			if(end_pos & 0x800000)
				fadstoplay = end_pos & 0xfffff;
			else
			{
				if(end_pos == 0)
					fadstoplay = (m_cdrom_image->get_track_start(0xaa)) - cd_curfad;
				else
					fadstoplay = (m_cdrom_image->get_track_start((end_pos & 0xff00) >> 8)) - cd_curfad;
			}
			LOGCMD("\ttrack mode %08x %08x\n", cd_curfad, fadstoplay);
		}
		else
		{
			/* resume from a pause state */
			// FIXME: verify implementation with Galaxy Fight
			// it calls 10ff ffff ffff ffff, but then it follows up with
			// 0x04->0x02->0x06->0x11->0x04->0x02->0x06 command sequence
			// (and current implementation nukes start/end FAD addresses at 0x04).
			// I'm sure that this doesn't work like this, but there could
			// be countless possible combinations ...
			if(fadstoplay == 0)
			{
				cd_curfad = m_cdrom_image->get_track_start(cur_track-1);
				fadstoplay = m_cdrom_image->get_track_start(cur_track) - cd_curfad;
			}
			LOGCMD("\ttrack resume %08x %08x\n",cd_curfad,fadstoplay);
		}
	}

	LOGCMD("\tPlay Disc: start %x length %x\n", cd_curfad, fadstoplay);

	cr_standard_return(cd_stat);
	hirqreg |= (CMOK);
	oddframe = 0;
	in_buffer = 0;

	playtype = 0;

	// cdda
	if(m_cdrom_image->get_track_type(m_cdrom_image->get_track(cd_curfad)) == cdrom_file::CD_TRACK_AUDIO)
	{
		m_cdda->pause_audio(0);
		//m_cdda->start_audio(cd_curfad, fadstoplay);
		//cdda_repeat_count = 0;
	}

	if(play_mode != 0x7f)
		cdda_maxrepeat = play_mode & 0xf;
	else
		cdda_maxrepeat = 0;

	cdda_repeat_count = 0;
	status_type = 0;
}

void stvcd_device::cmd_seek_disc()
{
	uint32_t temp;

	LOGCMD("%s: Disc seek\n",   machine().describe_context());
	LOGCMD("\t%08x %08x %08x %08x\n",cr1,cr2,cr3,cr4);
	if (cr1 & 0x80)
	{
		temp = (cr1&0xff)<<16;  // get FAD to seek to
		temp |= cr2;

		//cd_curfad = temp;

		if (temp == 0xffffff)
		{
			cd_stat = CD_STAT_PAUSE;
			m_cdda->pause_audio(1);
		}
		else
		{
			cd_curfad = ((cr1&0x7f)<<16) | cr2;
			LOGCMD("\tdisc seek with params %04x %04x\n",cr1,cr2); //Area 51 sets this up
		}
	}
	else
	{
		// is it a valid track?
		if (cr2 >> 8)
		{
			cd_stat = CD_STAT_PAUSE;
			cur_track = cr2>>8;
			cd_curfad = m_cdrom_image->get_track_start(cur_track-1);
			m_cdda->pause_audio(1);
			// (index is cr2 low byte)
		}
		else // error!
		{
			cd_stat = CD_STAT_STANDBY;
			cd_curfad = 0xffffffff;
			cur_track = 0xff;
			m_cdda->stop_audio(); //stop any pending CD-DA
		}
	}


	hirqreg |= CMOK;
	cr_standard_return(cd_stat);
	status_type = 0;
}

void stvcd_device::cmd_ffwd_rew_disc()
{
	// FFWD / REW
	// cr1 bit 0 determines if this is a Fast Forward (0) or a Rewind (1) command
	// TODO: unemulated, can be triggered thru BIOS player
	// ...
}

void stvcd_device::cmd_get_subcode_q_rw_channel()
{
	// Get SubCode Q / RW Channel
	switch(cr1 & 0xff)
	{
		case 0: // Get Q
		{
			uint32_t msf_abs,msf_rel;
			uint8_t track;
			cr1 = cd_stat | 0;
			cr2 = 10/2;
			cr3 = 0;
			cr4 = 0;

			/*
			Subcode Q info should be:
			---- --x- S0
			---- ---x S1
			xxxx ---- [0] Control (bit 7 Pre-emphasis, bit 6: copy permitted, bit 5 undefined, bit 4 number of channels)
			---- xxxx [0] address (0x0001 Mode 1)
			xxxx xxxx [1] track number (1-99, AA lead-out), BCD format
			xxxx xxxx [2] index (01 lead-out), BCD format
			xxxx xxxx [3] Time within' track M
			xxxx xxxx [4] Time within' track S
			xxxx xxxx [5] Time within' track F
			xxxx xxxx [6] Zero
			xxxx xxxx [7] Absolute M
			xxxx xxxx [8] Absolute S
			xxxx xxxx [9] Absolute F
			xxxx xxxx [10] CRCC
			xxxx xxxx [11] CRCC
			*/

			msf_abs = cdrom_file::lba_to_msf_alt( cd_curfad - 150 );
			track = m_cdrom_image->get_track( cd_curfad );
			msf_rel = cdrom_file::lba_to_msf_alt( cd_curfad - 150 - m_cdrom_image->get_track_start( track ) );

			xfertype = XFERTYPE_SUBQ;
			xfercount = 0;
			subqbuf[0] = 0x01 | ((m_cdrom_image->get_track_type(m_cdrom_image->get_track(track+1)) == cdrom_file::CD_TRACK_AUDIO) ? 0x00 : 0x40);
			subqbuf[1] = dec_2_bcd(track+1);
			subqbuf[2] = dec_2_bcd(get_track_index(cd_curfad));
			subqbuf[3] = dec_2_bcd((msf_rel >> 16) & 0xff);
			subqbuf[4] = dec_2_bcd((msf_rel >> 8) & 0xff);
			subqbuf[5] = dec_2_bcd((msf_rel >> 0) & 0xff);
			subqbuf[6] = 0;
			subqbuf[7] = dec_2_bcd((msf_abs >> 16) & 0xff);
			subqbuf[8] = dec_2_bcd((msf_abs >> 8) & 0xff);
			subqbuf[9] = dec_2_bcd((msf_abs >> 0) & 0xff);
		}
		break;

		case 1: // Get RW
			cr1 = cd_stat | 0;
			cr2 = 12;
			cr3 = 0;
			cr4 = 0;

			xfertype = XFERTYPE_SUBRW;
			xfercount = 0;

			/* return null data for now */
			{
				int i;

				for(i=0;i<12*2;i++)
					subrwbuf[i] = 0;
			}
			break;
	}
	hirqreg |= CMOK|DRDY;
	status_type = 0;
}

void stvcd_device::cmd_set_cddevice_connection()
{
	// Set CD Device connection
	uint8_t parm;

	// get operation
	parm = cr3>>8;

	LOGCMD("%s: Set CD Device Connection filter # %x\n",   machine().describe_context(), parm);

	cddevicenum = parm;

	if (parm == 0xff)
	{
		cddevice = (filterT *)nullptr;
	}
	else
	{
		if (parm < MAX_FILTERS)
		{
			cddevice = &filters[parm];
		}
	}

	hirqreg |= (CMOK|ESEL);
	cr_standard_return(cd_stat);
	status_type = 0;
}

void stvcd_device::cmd_get_cddevice_connection()
{
	popmessage("Get CD Device Connection, contact MAMEdev");
	hirqreg |= CMOK;
}

void stvcd_device::cmd_last_buffer_destination()
{
	// Last Buffer Destination
	cr1 = cd_stat | 0;
	cr2 = 0;
	cr3 = lastbuf << 8;
	cr4 = 0;
	hirqreg |= (CMOK);
	status_type = 0;
}

void stvcd_device::cmd_set_filter_range()
{
	// Set Filter Range
	// cr1 low + cr2 = FAD0, cr3 low + cr4 = FAD1
	// cr3 hi = filter num.
	uint8_t fnum = (cr3>>8)&0xff;

	LOGCMD("%s: Set Filter Range\n",   machine().describe_context());

	filters[fnum].fad = ((cr1 & 0xff)<<16) | cr2;
	filters[fnum].range = ((cr3 & 0xff)<<16) | cr4;

	LOGCMD("\t%08x %08x %d\n",filters[fnum].fad,filters[fnum].range,fnum);

	hirqreg |= (CMOK|ESEL);
	cr_standard_return(cd_stat);
	status_type = 0;
}

void stvcd_device::cmd_get_filter_range()
{
	popmessage("Get Filter Range, contact MAMEdev");
	hirqreg |= CMOK;
}

void stvcd_device::cmd_set_filter_subheader_conditions()
{
	// Set Filter Subheader conditions
	uint8_t fnum = (cr3>>8)&0xff;

	LOGCMD("%s: Set Filter Subheader conditions %x => chan %x masks %x fid %x vals %x\n", machine().describe_context(), fnum, cr1&0xff, cr2, cr3&0xff, cr4);

	filters[fnum].chan = cr1 & 0xff;
	filters[fnum].smmask = (cr2>>8)&0xff;
	filters[fnum].cimask = cr2&0xff;
	filters[fnum].fid = cr3&0xff;
	filters[fnum].smval = (cr4>>8)&0xff;
	filters[fnum].cival = cr4&0xff;

	hirqreg |= (CMOK|ESEL);
	cr_standard_return(cd_stat);
	status_type = 0;
}


void stvcd_device::cmd_get_filter_subheader_conditions()
{
	// Get Filter Subheader conditions
	uint8_t fnum = (cr3>>8)&0xff;

	LOGCMD("%s: Set Filter Subheader conditions %x => chan %x masks %x fid %x vals %x\n", machine().describe_context(), fnum, cr1&0xff, cr2, cr3&0xff, cr4);

	cr1 = cd_stat | (filters[fnum].chan & 0xff);
	cr2 = (filters[fnum].smmask << 8) | (filters[fnum].cimask & 0xff);
	cr3 = filters[fnum].fid;
	cr4 = (filters[fnum].smval << 8) | (filters[fnum].cival & 0xff);

	hirqreg |= (CMOK|ESEL);
	status_type = 0;
}

void stvcd_device::cmd_set_filter_mode()
{
	// Set Filter Mode
	uint8_t fnum = (cr3>>8)&0xff;
	uint8_t mode = (cr1 & 0xff);

	// initialize filter?
	if (mode & 0x80)
	{
		memset(&filters[fnum], 0, sizeof(filterT));
	}
	else
	{
		filters[fnum].mode = mode;
	}

	LOGCMD("%s: Set Filter Mode filt %x mode %x\n", machine().describe_context(), fnum, mode);
	hirqreg |= (CMOK|ESEL);
	cr_standard_return(cd_stat);
	status_type = 0;
}

void stvcd_device::cmd_get_filter_mode()
{
	// Get Filter Mode
	uint8_t fnum = (cr3>>8)&0xff;

	cr1 = cd_stat | (filters[fnum].mode & 0xff);
	cr2 = 0;
	cr3 = 0;
	cr4 = 0;

	hirqreg |= (CMOK|ESEL);
	status_type = 0;
}

void stvcd_device::cmd_set_filter_connection()
{
	// Set Filter Connection
	// FIXME: verify usage of cr3 LSB
	// (false condition?)
	uint8_t fnum = (cr3>>8)&0xff;

	LOGCMD("%s: Set Filter Connection %x => mode %x parm %04x\n", machine().describe_context(), fnum, cr1 & 0xf, cr2);

	if (cr1 & 1)    // set true condition
		filters[fnum].condtrue = (cr2>>8)&0xff;

	if (cr1 & 2)    // set false condition
		filters[fnum].condfalse = cr2&0xff;

	hirqreg |= (CMOK|ESEL);
	cr_standard_return(cd_stat);
	status_type = 0;
}

void stvcd_device::cmd_reset_selector()
{
	int i,j;
	// Reset Selector

	LOGCMD("%s: Reset Selector\n",   machine().describe_context());

	if((cr1 & 0xff) == 0x00)
	{
		uint8_t bufnum = cr3>>8;

		if(bufnum < MAX_FILTERS)
		{
			for (i = 0; i < MAX_BLOCKS; i++)
			{
				cd_free_block(partitions[bufnum].blocks[i]);
				partitions[bufnum].blocks[i] = (blockT *)nullptr;
				partitions[bufnum].bnum[i] = 0xff;
			}

			partitions[bufnum].size = -1;
			partitions[bufnum].numblks = 0;
		}

		// TODO: buffer full flag

		if (freeblocks == 200) { sectorstore = 0; }

		hirqreg |= (CMOK|ESEL);
		cr_standard_return(cd_stat);
		status_type = 0;
		return;
	}

	/* reset false filter output conditions */
	/// TODO: verify default value for these two
	if(cr1 & 0x80)
	{
		for(i=0;i<MAX_FILTERS;i++)
			filters[i].condfalse = 0;
	}

	/* reset true filter output conditions */
	if(cr1 & 0x40)
	{
		for(i=0;i<MAX_FILTERS;i++)
			filters[i].condtrue = 0;
	}

	/* reset filter conditions*/
	if(cr1 & 0x10)
	{
		for(i=0;i<MAX_FILTERS;i++)
		{
			filters[i].fad = 0;
			filters[i].range = 0xffffffff;
			filters[i].mode = 0;
			filters[i].chan = 0;
			filters[i].smmask = 0;
			filters[i].cimask = 0;
			filters[i].fid = 0;
			filters[i].smval = 0;
			filters[i].cival = 0;
		}
	}

	/* reset partition buffer data */
	if(cr1 & 0x4)
	{
		for(i=0;i<MAX_FILTERS;i++)
		{
			for (j = 0; j < MAX_BLOCKS; j++)
			{
				cd_free_block(partitions[i].blocks[j]);
				partitions[i].blocks[j] = (blockT *)nullptr;
				partitions[i].bnum[j] = 0xff;
			}

			partitions[i].size = -1;
			partitions[i].numblks = 0;
		}

		buffull = sectorstore = 0;
	}

	hirqreg |= (CMOK|ESEL);
	cr_standard_return(cd_stat);
	status_type = 0;
}

void stvcd_device::cmd_get_buffer_size()
{
	// get Buffer Size
	cr1 = cd_stat;
	cr2 = (freeblocks > 200) ? 200 : freeblocks;
	cr3 = 0x1800;
	cr4 = 200;
	LOG("Get Buffer Size = %d\n", cr2);
	hirqreg |= (CMOK);
	status_type = 0;
}

void stvcd_device::cmd_get_buffer_partition_sector_number()
{
	// get # sectors used in a buffer

	uint32_t bufnum = cr3>>8;

	LOGCMD("%s: Get Sector Number (bufno %d) = %d blocks\n",   machine().describe_context(), bufnum, cr4);
	cr1 = cd_stat;
	cr2 = 0;
	cr3 = 0;
	if(cr1 & 0xff || cr2 || cr3 & 0xff || cr4)
		LOGWARN("Get # sectors used with params %04x %04x %04x %04x\n",cr1,cr2,cr3,cr4);

	// is the partition empty?
	if (partitions[bufnum].size == -1)
	{
		cr4 = 0;
	}
	else
	{
		cr4 = partitions[bufnum].numblks;
		//LOGWARN("Partition %08x %04x\n",bufnum,cr4);
	}

	//LOGWARN("%04x\n",cr4);
	if(cr4 == 0)
		hirqreg |= (CMOK);
	else
		hirqreg |= (CMOK|DRDY);
	status_type = 1;
}

void stvcd_device::cmd_calculate_actual_data_size()
{
	// calculate actual size
	uint32_t bufnum = cr3>>8;
	uint32_t sectoffs = cr2;
	uint32_t numsect = cr4;

	LOGCMD("%s: Calculate actual size: buf %x offs %x numsect %x\n", machine().describe_context(), bufnum, sectoffs, numsect);

	calcsize = 0;
	if (partitions[bufnum].size != -1)
	{
		int32_t i;

		for (i = 0; i < numsect; i++)
		{
			if (partitions[bufnum].blocks[sectoffs+i])
			{
				calcsize += (partitions[bufnum].blocks[sectoffs+i]->size / 2);
			}
		}
	}

	hirqreg |= (CMOK|ESEL);
	cr_standard_return(cd_stat);
	status_type = 0;
}

void stvcd_device::cmd_get_actual_data_size()
{
	// get actual block size
	LOGCMD("%s: Get actual block size\n", machine().describe_context());
	cr1 = cd_stat | ((calcsize>>16)&0xff);
	cr2 = (calcsize & 0xffff);
	cr3 = 0;
	cr4 = 0;
	hirqreg |= (CMOK|ESEL);
	status_type = 1;
}

void stvcd_device::cmd_get_sector_information()
{
	// get sector info
	uint32_t sectoffs = cr2 & 0xff;
	uint32_t bufnum = cr3>>8;

	if (bufnum >= MAX_FILTERS || !partitions[bufnum].blocks[sectoffs])
	{
		cr1 |= CD_STAT_REJECT & 0xff00;
		hirqreg |= (CMOK|ESEL);
		LOGWARN("Get sector info reject\n");
	}
	else
	{
		cr1 = cd_stat | ((partitions[bufnum].blocks[sectoffs]->FAD >> 16) & 0xff);
		cr2 = partitions[bufnum].blocks[sectoffs]->FAD & 0xffff;
		cr3 = ((partitions[bufnum].blocks[sectoffs]->fnum & 0xff) << 8) | (partitions[bufnum].blocks[sectoffs]->chan & 0xff);
		cr4 = ((partitions[bufnum].blocks[sectoffs]->subm & 0xff) << 8) | (partitions[bufnum].blocks[sectoffs]->cinf & 0xff);
		hirqreg |= (CMOK|ESEL);
	}

	status_type = 0;
}

void stvcd_device::cmd_set_sector_length()
{
	// set sector length
	LOGCMD("%s: Set sector length\n",   machine().describe_context());

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
	hirqreg |= (CMOK|ESEL);
	cr_standard_return(cd_stat);
	status_type = 0;
}

void stvcd_device::cmd_get_sector_data()
{
	// get sector data
	uint32_t sectnum = cr4;
	uint32_t sectofs = cr2;
	uint32_t bufnum = cr3>>8;

	LOGCMD("%s: Get sector data (SN %d SO %d BN %d)\n",   machine().describe_context(), sectnum, sectofs, bufnum);

	if (bufnum >= MAX_FILTERS)
	{
		// TODO: find actual SW that does this
		// (may conceal a bigger issue)
		LOGWARN("CD: invalid buffer number\n");
		cr_standard_return(CD_STAT_REJECT);
		hirqreg |= (CMOK|EHST);
		return;
	}

	if (partitions[bufnum].numblks < sectnum)
	{
		LOGWARN("CD: buffer is not full %08x %08x\n",partitions[bufnum].numblks,sectnum);
		cr_standard_return(CD_STAT_REJECT);
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
	cr_standard_return(cd_stat);
	hirqreg |= (CMOK|EHST|DRDY);
	status_type = 0;
}

void stvcd_device::cmd_delete_sector_data()
{
	// delete sector data
	uint32_t sectnum = cr4;
	uint32_t sectofs = cr2;
	uint32_t bufnum = cr3>>8;
	int32_t i;

	LOGCMD("%s: Delete sector data (SN %d SO %d BN %d)\n",   machine().describe_context(), sectnum, sectofs, bufnum);

	if (bufnum >= MAX_FILTERS)
	{
		// TODO: mustn't happen
		LOGWARN("CD: invalid buffer number\n");
		cr_standard_return(CD_STAT_REJECT);
		hirqreg |= (CMOK|EHST);
		return;
	}

	// pstarcol PS2 does this
	// TODO: verify if implementation is correct
	if (partitions[bufnum].numblks == 0)
	{
		LOGWARN("CD: buffer is already empty\n");
		cr_standard_return(CD_STAT_REJECT);
		hirqreg |= (CMOK|EHST);
		return;
	}

	cd_getsectoroffsetnum(bufnum, &sectofs, &sectnum);

	for (i = sectofs; i < (sectofs + sectnum); i++)
	{
		// pstarcol PS2 tries to delete partial partitions,
		// need to guard against it (otherwise it would crash after first attract cycle)
		if (partitions[bufnum].size > 0)
		{
			partitions[bufnum].size -= partitions[bufnum].blocks[i]->size;
			cd_free_block(partitions[bufnum].blocks[i]);
			partitions[bufnum].blocks[i] = (blockT *)nullptr;
			partitions[bufnum].bnum[i] = 0xff;
		}
	}

	cd_defragblocks(&partitions[bufnum]);

	partitions[bufnum].numblks -= sectnum;

	if (freeblocks == 200)
	{
		sectorstore = 0;
	}

	cd_stat &= ~CD_STAT_TRANS;
	cr_standard_return(cd_stat);
	hirqreg |= (CMOK|EHST);
	status_type = 0;
}

void stvcd_device::cmd_get_and_delete_sector_data()
{
	// get then delete sector data

	uint32_t sectnum = cr4;
	uint32_t sectofs = cr2;
	uint32_t bufnum = cr3>>8;

	LOGCMD("%s: Get and delete sector data (SN %d SO %d BN %d)\n",   machine().describe_context(), sectnum, sectofs, bufnum);

	if (bufnum >= MAX_FILTERS)
	{
		// TODO: mustn't happen
		LOGWARN("CD: invalid buffer number\n");
		cr_standard_return(CD_STAT_REJECT);
		hirqreg |= (CMOK|EHST);
		return;
	}

	/* Yoshimoto Mahjong uses the REJECT status to verify when the data is ready. */
	// TODO: verify again if it's really REJECT or something else
	if (partitions[bufnum].numblks < sectnum)
	{
		LOGWARN("CD: buffer is not full %08x %08x\n",partitions[bufnum].numblks,sectnum);
		cr_standard_return(CD_STAT_REJECT);
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

	cd_stat |= CD_STAT_TRANS;
	cr_standard_return(cd_stat);
	hirqreg |= (CMOK|EHST|DRDY);
	status_type = 0;
}

void stvcd_device::cmd_put_sector_data()
{
	// put sector data
	/* After Burner 2, Out Run, Fantasy Zone and Dungeon Master Nexus trips this */

	uint32_t sectnum = cr4 & 0xff;
	uint32_t sectofs = cr2;
	uint32_t bufnum = cr3>>8;

	xfertype32 = XFERTYPE32_PUTSECTOR;

	/*TODO: eventual errors? */

	cd_getsectoroffsetnum(bufnum, &sectofs, &sectnum);

	cd_stat |= CD_STAT_TRANS;

	xferoffs = 0;
	xfersect = 0;
	xferdnum = 0;
	xfersectpos = sectofs;
	xfersectnum = sectnum;
	transpart = &partitions[bufnum];

	// allocate the blocks
	for (int i = xfersectpos; i < xfersectpos+xfersectnum; i++)
	{
		transpart->blocks[i] = cd_alloc_block(&transpart->bnum[i]);
		if(transpart->size == -1)
			transpart->size = 0;
		transpart->size += transpart->blocks[i]->size;
		transpart->numblks++;
	}

	hirqreg |= (CMOK|DRDY);
	cr_standard_return(cd_stat);
	status_type = 0;
}

void stvcd_device::cmd_move_sector_data()
{
	popmessage("Move Sector data, contact MAMEdev");
	hirqreg |= (CMOK);
}

void stvcd_device::cmd_copy_sector_data()
{
	// copy sector data
	/* Sword & Sorcery / Riglord Saga 2 uses this */
	// TODO: incomplete
	uint32_t src_filter = (cr3>>8)&0xff;
	uint32_t dst_filter = cr1&0xff;
	uint32_t sectnum = cr4 & 0xff;

	//cd_stat |= CD_STAT_TRANS;
	//transpart = &partitions[dst_filter];

	for (int i = 0; i < sectnum; i++)
	{
		// allocate the dst blocks
		partitions[dst_filter].blocks[i] = cd_alloc_block(&partitions[dst_filter].bnum[i]);
		if(partitions[dst_filter].size == -1)
			partitions[dst_filter].size = 0;
		partitions[dst_filter].size += partitions[dst_filter].blocks[i]->size;
		partitions[dst_filter].numblks++;

		//copy data
		for(int j = 0; j < sectlenin; j++)
			partitions[dst_filter].blocks[i]->data[j] = partitions[src_filter].blocks[i]->data[j];

		//deallocate the src blocks
		//partitions[src_filter].size -= partitions[src_filter].blocks[i]->size;
		//cd_free_block(partitions[src_filter].blocks[i]);
		//partitions[src_filter].blocks[i] = (blockT *)nullptr;
		//partitions[src_filter].bnum[i] = 0xff;
	}

	hirqreg |= (CMOK|ECPY);
	cr_standard_return(cd_stat);
	status_type = 0;
}

void stvcd_device::cmd_get_sector_data_copy_or_move_error()
{
	// get copy error
	LOGCMD("%s: Get copy error\n",   machine().describe_context());
	logerror("Get copy error\n");
	cr1 = cd_stat;
	cr2 = 0;
	cr3 = 0;
	cr4 = 0;
	hirqreg |= (CMOK);
	status_type = 0;
}

void stvcd_device::cmd_change_directory()
{
	uint32_t temp;
	// change directory
	LOGCMD("%s: Change Directory\n",   machine().describe_context());
	hirqreg |= (CMOK|EFLS);

	temp = (cr3&0xff)<<16;
	temp |= cr4;

	read_new_dir(temp);
	cr_standard_return(cd_stat);
	status_type = 0;
}

void stvcd_device::cmd_read_directory()
{
	// Read directory entry
	LOGCMD("%s: Read Directory Entry\n",   machine().describe_context());
//  uint32_t read_dir;

//  read_dir = ((cr3&0xff)<<16)|cr4;

	if((cr3 >> 8) < MAX_FILTERS)
		cddevice = &filters[cr3 >> 8];
	else
		cddevice = (filterT *)nullptr;

	// TODO: how to actually read?
	//read_new_dir(read_dir - 2);

	cr_standard_return(cd_stat);
	hirqreg |= (CMOK|EFLS);
	status_type = 0;
}

void stvcd_device::cmd_get_file_scope()
{
	// Get file system scope
	LOGCMD("%s: Get file system scope\n", machine().describe_context());
	hirqreg |= (CMOK|EFLS);
	cr1 = cd_stat;
	cr2 = numfiles; // # of files in directory
	cr3 = 0x0100;   // report directory held
	cr4 = firstfile;    // first file id
	LOGWARN("%04x %04x %04x %04x\n",cr1,cr2,cr3,cr4);
	status_type = 0;
}

void stvcd_device::cmd_get_target_file_info()
{
	uint32_t temp;

	// Get File Info
	LOGCMD("%s: Get File Info\n",   machine().describe_context());
	cd_stat |= CD_STAT_TRANS;
	cd_stat &= 0xff00;      // clear top byte of return value
	playtype = 0;
	cdda_repeat_count = 0;
	hirqreg |= (CMOK|DRDY);

	temp = (cr3&0xff)<<16;
	temp |= cr4;

	if (temp == 0xffffff)   // special
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
		cr2 = 6;    // 6 words for single file
					// first 4 bytes = FAD address
					// second 4 bytes = length
					// last 4 bytes:
					// - unit size
					// - gap size
					// - file #
					// attributes flags

		cr3 = 0;
		cr4 = 0;

		// TODO: chaossd and sengblad does this
		// (iso9660 parsing doesn't read beyond the first sector)
		if (curdir[temp].firstfad == 0 || curdir[temp].length == 0)
			throw emu_fatalerror("File ID not found in XFERTYPE_FILEINFO_1");
//      LOGWARN("%08x %08x\n",curdir[temp].firstfad,curdir[temp].length);
		// first 4 bytes = FAD
		put_u32be(&finfbuf[0], curdir[temp].firstfad);
		// second 4 bytes = length of file
		put_u32be(&finfbuf[4], curdir[temp].length);
		finfbuf[8] = curdir[temp].interleave_gap_size;
		finfbuf[9] = curdir[temp].file_unit_size;
		finfbuf[10] = temp;
		finfbuf[11] = curdir[temp].flags;


		xfertype = XFERTYPE_FILEINFO_1;
		xfercount = 0;
	}
	LOG("   = %04x %04x %04x %04x %04x\n", hirqreg, cr1, cr2, cr3, cr4);
	status_type = 0;
}

void stvcd_device::cmd_read_file()
{
	// Read File
	LOGCMD("%s: Read File\n",   machine().describe_context());
	uint16_t file_offset,file_filter,file_id,file_size;

	file_offset = ((cr1 & 0xff)<<8)|(cr2 & 0xff); /* correct? */
	file_filter = cr3 >> 8;
	file_id = ((cr3 & 0xff) << 16)|(cr4);
	file_size = ((curdir[file_id].length + sectlenin - 1) / sectlenin) - file_offset;

	cd_stat = CD_STAT_PLAY|0x80;    // set "cd-rom" bit
	cd_curfad = (curdir[file_id].firstfad + file_offset);
	fadstoplay = file_size;
	if(file_filter < MAX_FILTERS)
		cddevice = &filters[file_filter];
	else
		cddevice = (filterT *)nullptr;

	LOGWARN("Read file %08x (%08x %08x) %02x %d\n",curdir[file_id].firstfad,cd_curfad,fadstoplay,file_filter,sectlenin);

	cr_standard_return(cd_stat);

	oddframe = 0;
	in_buffer = 0;

	playtype = 1;

	hirqreg |= (CMOK|EHST);
	status_type = 0;
}

void stvcd_device::cmd_abort_file()
{
	LOGCMD("%s: Abort File\n",   machine().describe_context());
	// bios expects "2bc" mask to work against this
	hirqreg |= (CMOK|EFLS);
	sectorstore = 0;
	xfertype32 = XFERTYPE32_INVALID;
	xferdnum = 0;
	if(((cd_stat & 0x0f00) != CD_STAT_NODISC) && ((cd_stat & 0x0f00) != CD_STAT_OPEN))
		cd_stat = CD_STAT_PAUSE;    // force to pause
	cr_standard_return(cd_stat);
	status_type = 0;
}

void stvcd_device::cmd_check_copy_protection()
{
	// appears to be copy protection check.  needs only to return OK.
	LOGCMD("%s: Verify copy protection\n",   machine().describe_context());
	if(((cd_stat & 0x0f00) != CD_STAT_NODISC) && ((cd_stat & 0x0f00) != CD_STAT_OPEN))
		cd_stat = CD_STAT_PAUSE;

//   cr1 = cd_stat;  // necessary to pass
//   cr2 = 0x4;
//   hirqreg |= (CMOK|EFLS|CSCT);
	if(cr2 == 0x0001) // MPEG card
	{
		hirqreg |= (CMOK|MPED);
	}
	else
	{
		sectorstore = 1;
		hirqreg = 0x7c5;
	}
	cr_standard_return(cd_stat);
	status_type = 0;
}

void stvcd_device::cmd_get_disc_region()
{
	// get disc region
	LOGCMD("%s: Get disc region\n",   machine().describe_context());
	if(cd_stat != CD_STAT_NODISC && cd_stat != CD_STAT_OPEN)
		cd_stat = CD_STAT_PAUSE;
	cr1 = cd_stat;  // necessary to pass
	if(cr2 == 0x0001) // MPEG card
		cr2 = 0x2;
	else
		cr2 = 0x4;    // 0 = No CD, 1 = Audio CD, 2 Regular Data disk (not Saturn), 3 pirate disc, 4 Saturn disc
	cr3 = 0;
	cr4 = 0;
	hirqreg |= (CMOK);
//  cr_standard_return(cd_stat);
	status_type = 0;

}

void stvcd_device::cmd_get_mpeg_card_boot_rom()
{
	// Get MPEG Card Boot ROM
	// TODO: incomplete, needs to actually retrieve from MPEG ROM, just silence popmessage for now.

	cr_standard_return(cd_stat);
	hirqreg |= (CMOK|MPED);
}

void stvcd_device::cmd_mpeg_get_status()
{
	// MPEG Get Status
	// ...
	mpeg_standard_return(cd_stat);
	hirqreg |= (CMOK);
}

void stvcd_device::cmd_mpeg_get_irq()
{
	// MPEG get IRQ
	// ...
	cr1 = cd_stat | 0;
	cr2 = 5;
	cr3 = 0;
	cr4 = 0;
	hirqreg |= (CMOK);
}


void stvcd_device::cmd_mpeg_set_irq_mask()
{
	// MPEG Set IRQ Mask
	// ...
	mpeg_standard_return(cd_stat);
	hirqreg |= (CMOK);
}

void stvcd_device::cmd_mpeg_set_mode()
{
	// MPEG Set Mode
	// ...
	mpeg_standard_return(cd_stat);
	hirqreg |= (CMOK);
}


void stvcd_device::cmd_mpeg_init()
{
	// MPEG init
	// ...
	hirqreg |= (CMOK|MPED);
	if(cr2 == 0x0001)
		hirqreg |= (MPCM);

	cr1 = cd_stat;
	cr2 = 0;
	cr3 = 0;
	cr4 = 0;

}

void stvcd_device::cd_exec_command()
{
	if(cr1 != 0 &&
		((cr1 & 0xff00) != 0x5100) &&
		((cr1 & 0xff00) != 0x5200) &&
		((cr1 & 0xff00) != 0x5300) &&
		1)
		logerror("Command exec %04x %04x %04x %04x %04x (stat %04x)\n", hirqreg, cr1, cr2, cr3, cr4, cd_stat);

	if(!m_cdrom_image->exists() && ((cr1 >> 8) & 0xff) != 0x00) {
		hirqreg |= (CMOK);
		return;
	}

	switch ((cr1 >> 8) & 0xff)
	{
		case 0x00: cmd_get_status(); break;
		case 0x01: cmd_get_hw_info(); break;
		case 0x02: cmd_get_toc(); break;
		case 0x03: cmd_get_session_info(); break;
		case 0x04: cmd_init_cdsystem(); break;
		case 0x06: cmd_end_data_transfer(); break;

		case 0x10: cmd_play_disc(); break;
		case 0x11: cmd_seek_disc(); break;
		case 0x12: cmd_ffwd_rew_disc(); break;

		case 0x20: cmd_get_subcode_q_rw_channel(); break;

		case 0x30: cmd_set_cddevice_connection(); break;
		case 0x31: cmd_get_cddevice_connection(); break;
		case 0x32: cmd_last_buffer_destination(); break;

		case 0x40: cmd_set_filter_range(); break;
		case 0x41: cmd_get_filter_range(); break;
		case 0x42: cmd_set_filter_subheader_conditions(); break;
		case 0x43: cmd_get_filter_subheader_conditions(); break;
		case 0x44: cmd_set_filter_mode(); break;
		case 0x45: cmd_get_filter_mode(); break;
		case 0x46: cmd_set_filter_connection(); break;
		case 0x48: cmd_reset_selector(); break;

		case 0x50: cmd_get_buffer_size(); break;
		case 0x51: cmd_get_buffer_partition_sector_number(); break;
		case 0x52: cmd_calculate_actual_data_size(); break;
		case 0x53: cmd_get_actual_data_size(); break;
		case 0x54: cmd_get_sector_information(); break;
//      case 0x55: cmd_execute_frame_address_search()
//      case 0x56: cmd_get_frame_address_search_results()

		case 0x60: cmd_set_sector_length(); break;
		case 0x61: cmd_get_sector_data(); break;
		case 0x62: cmd_delete_sector_data(); break;
		case 0x63: cmd_get_and_delete_sector_data(); break;
		case 0x64: cmd_put_sector_data(); break;
		case 0x65: cmd_move_sector_data(); break;
		case 0x66: cmd_copy_sector_data(); break;
		case 0x67: cmd_get_sector_data_copy_or_move_error(); break;

		case 0x70: cmd_change_directory(); break;
		case 0x71: cmd_read_directory(); break;
		case 0x72: cmd_get_file_scope(); break;
		case 0x73: cmd_get_target_file_info(); break;
		case 0x74: cmd_read_file(); break;
		case 0x75: cmd_abort_file(); break;

		// following are MPEG commands, enough to get Sport Fishing to do something
		case 0x90: cmd_mpeg_get_status(); break;
		case 0x91: cmd_mpeg_get_irq(); break;
		case 0x92: cmd_mpeg_set_irq_mask(); break;
		case 0x93: cmd_mpeg_init(); break;
		case 0x94: cmd_mpeg_set_mode(); break;

		case 0xe0: cmd_check_copy_protection(); break;
		case 0xe1: cmd_get_disc_region(); break;
		case 0xe2: cmd_get_mpeg_card_boot_rom(); break;

		default:
			LOG("Unknown command %04x\n", cr1>>8);
			popmessage("CD Block unknown command %02x, contact MAMEdev",cr1>>8);

			hirqreg |= (CMOK);
			break;
	}

	if(status_type == 1)
	{
		prev_cr1 = cr1;
		prev_cr2 = cr2;
		prev_cr3 = cr3;
		prev_cr4 = cr4;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER( stvcd_device::stv_sh1_sim )
{
	if((cmd_pending == 0xf) && (!(hirqreg & CMOK)))
		cd_exec_command();
}

TIMER_DEVICE_CALLBACK_MEMBER( stvcd_device::stv_sector_cb )
{
	if(!m_cdrom_image->exists())
		return;

	//m_sector_timer->reset();

	//popmessage("%08x %08x %d %d",cd_curfad,fadstoplay,cmd_pending,cd_speed);

	cd_playdata();

	if(m_cdrom_image->get_track_type(m_cdrom_image->get_track(cd_curfad)) == cdrom_file::CD_TRACK_AUDIO)
		m_sector_timer->adjust(attotime::from_hz(75));    // 75 sectors / second = 150kBytes/second (cdda track ignores cd_speed setting)
	else
		m_sector_timer->adjust(attotime::from_hz(75*cd_speed));   // 75 / 150 sectors / second = 150 / 300kBytes/second

	// TODO: Saturn refuses to boot with this if a disk isn't in and condition is applied!?
	// TODO: Check out actual timing of SCDQ acquisition.
	// (Daytona USA original version definitely wants it to be on).
	//if(((cd_stat & 0x0f00) != CD_STAT_NODISC) && ((cd_stat & 0x0f00) != CD_STAT_OPEN))
	hirqreg |= SCDQ;

	if(cd_stat & CD_STAT_PERI)
	{
		cr_standard_return(cd_stat);
	}
}

// global functions
void stvcd_device::device_reset()
{
	int32_t i, j;

	hirqmask = 0xffff;
	hirqreg = 0xffff;
	cr1 = 'C';
	cr2 = ('D'<<8) | 'B';
	cr3 = ('L'<<8) | 'O';
	cr4 = ('C'<<8) | 'K';
	cd_stat = CD_STAT_PAUSE;
	cd_stat |= CD_STAT_PERI;
	cur_track = 0xff;

	curdir.clear();

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
			partitions[i].blocks[j] = (blockT *)nullptr;
			partitions[i].bnum[j] = 0xff;
		}
	}

	// reset blocks
	for (i = 0; i < MAX_BLOCKS; i++)
	{
		blocks[i].size = -1;
		memset(&blocks[i].data, 0, cdrom_file::MAX_SECTOR_DATA);
	}

	// open device
	if (m_cdrom_image->exists())
	{
		LOG("Opened CD-ROM successfully, reading root directory\n");
		read_new_dir(0xffffff);    // read root directory
	}
	else
	{
		cd_stat = CD_STAT_NODISC;
	}

	cd_speed = 2;
	cdda_repeat_count = 0;
	tray_is_closed = 1;

	m_sector_timer->adjust(attotime::from_hz(150));   // 150 sectors / second = 300kBytes/second
}

stvcd_device::blockT *stvcd_device::cd_alloc_block(uint8_t *blknum)
{
	int32_t i;

	// search the 200 available blocks for a free one
	for (i = 0; i < 200; i++)
	{
		if (blocks[i].size == -1)
		{
			freeblocks--;
			if (freeblocks <= 0)
			{
				buffull = 1;
				LOGWARN("buffull in cd_alloc_block\n");
			}

			blocks[i].size = sectlenin;
			*blknum = i;

			LOG("Allocating block %d, size %x\n", i, sectlenin);

			return &blocks[i];
		}
	}

	buffull = 1;
	return (blockT *)nullptr;
}

void stvcd_device::cd_free_block(blockT *blktofree)
{
	int32_t i;

	LOG("cd_free_block: %x\n", (uint32_t)(uintptr_t)blktofree);

	if(blktofree == nullptr)
	{
		return;
	}

	for (i = 0; i < 200; i++)
	{
		if (&blocks[i] == blktofree)
		{
			LOG("Freeing block %d\n", i);
		}
	}

	blktofree->size = -1;
	freeblocks++;
	buffull = 0;
	hirqreg &= ~BFUL;
}

void stvcd_device::cd_getsectoroffsetnum(uint32_t bufnum, uint32_t *sectoffs, uint32_t *sectnum)
{
	if (*sectoffs == 0xffff)
	{
		// last sector
		LOGWARN("CD: Don't know how to handle offset ffff\n");
	}
	else if (*sectnum == 0xffff)
	{
		*sectnum = partitions[bufnum].numblks - *sectoffs;
	}
}

void stvcd_device::cd_defragblocks(partitionT *part)
{
	uint32_t i, j;
	blockT *temp;
	uint8_t temp2;

	for (i = 0; i < (MAX_BLOCKS-1); i++)
	{
		for (j = i+1; j < MAX_BLOCKS; j++)
		{
			if ((part->blocks[i] == (blockT *)nullptr) && (part->blocks[j] != (blockT *)nullptr))
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

// iso9660 parsing
void stvcd_device::read_new_dir(uint32_t fileno)
{
	int foundpd, i;
	uint32_t cfad;//, dirfad;
	uint8_t sect[2048];

	if (fileno == 0xffffff)
	{
		cfad = 166;     // first sector of directory as per iso9660 specs

		foundpd = 0;    // search for primary vol. desc
		while ((!foundpd) && (cfad < 200))
		{
			if(sectlenin != 2048)
				popmessage("Sector Length %d, contact MAMEdev (0)",sectlenin);

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
			//dirfad = get_u32le(&sect[140]);
			//dirfad += 150;

			// parse root entry
			curroot.firstfad = get_u32le(&sect[158]);
			curroot.firstfad += 150;
			curroot.length = get_u32le(&sect[166]);
			curroot.flags = sect[181];
			for (i = 0; i < sect[188]; i++)
			{
				curroot.name[i] = sect[189+i];
			}
			curroot.name[i] = '\0'; // terminate

			// easy to fix, but make sure we *need* to first
			if (curroot.length > MAX_DIR_SIZE)
			{
				LOGWARN("ERROR: root directory too big (%d)\n", curroot.length);
			}

			// done with all that, read the root directory now
			make_dir_current(curroot.firstfad);
		}
	}
	else
	{
		if (curdir[fileno].length > MAX_DIR_SIZE)
		{
			LOGWARN("ERROR: new directory too big (%d)!\n", curdir[fileno].length);
		}
		make_dir_current(curdir[fileno].firstfad);
	}
}

// makes the directory pointed to by FAD current
void stvcd_device::make_dir_current(uint32_t fad)
{
	uint32_t i;
	uint32_t nextent, numentries;
	std::vector<uint8_t> sect(MAX_DIR_SIZE);
	direntryT *curentry;

	memset(&sect[0], 0, MAX_DIR_SIZE);
	if(sectlenin != 2048)
		popmessage("Sector Length %d, contact MAMEdev (1)",sectlenin);

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
		curentry->firstfad = get_u32le(&sect[nextent+2]);
		curentry->firstfad += 150;
		curentry->length = get_u32le(&sect[nextent+10]);
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
		curentry->volume_sequencer_number = get_u16le(&sect[nextent+28]);

		for (i = 0; i < sect[nextent+32]; i++)
		{
			curentry->name[i] = sect[nextent+33+i];
		}
		curentry->name[i] = '\0';   // terminate
		//LOGWARN("%08x %08x %s %d/%d/%d\n",curentry->firstfad,curentry->length,curentry->name,curentry->year,curentry->month,curentry->day);

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

void stvcd_device::device_stop()
{
	curdir.clear();
}

void stvcd_device::cd_readTOC(void)
{
	int i, ntrks, tocptr, fad;

	xfertype = XFERTYPE_TOC;
	xfercount = 0;

	if (m_cdrom_image->exists())
	{
		ntrks = m_cdrom_image->get_last_track();
	}
	else
	{
		ntrks = 0;
	}

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
		if (m_cdrom_image->exists())
		{
			//tocbuf[tocptr] = sega_cdrom_get_adr_control(cdrom, i);
			//HACK: ddsom does not enter ingame with the line above!
			tocbuf[tocptr] = m_cdrom_image->get_adr_control(i)<<4 | 0x01;
		}
		else
		{
			tocbuf[tocptr] = 0xff;
		}

		if (m_cdrom_image->exists())
		{
			fad = m_cdrom_image->get_track_start(i) + 150;

			put_u24be(&tocbuf[tocptr+1], fad);
		}
		else
		{
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
	fad = m_cdrom_image->get_track_start(0xaa) + 150;

	tocbuf[tocptr+8] = tocbuf[0];
	put_u24be(&tocbuf[tocptr+9], fad);
}

stvcd_device::partitionT *stvcd_device::cd_filterdata(filterT *flt, int trktype, uint8_t *p_ok)
{
	int match, keepgoing;
	partitionT *filterprt;

	LOG("cd_filterdata, trktype %d\n", trktype);
	match = 1;
	keepgoing = 2;
	lastbuf = flt->condtrue;

	// loop on the filters
	do
	{
		// FAD range check?
		/* according to an obscure document note, this switches the filter connector to be false if the range fails ... I think ... */
		if (flt->mode & 0x40)
		{
			if ((cd_curfad < flt->fad) || (cd_curfad > (flt->fad + flt->range)))
			{
				LOGWARN("curfad reject %08x %08x %08x %08x\n",cd_curfad,fadstoplay,flt->fad,flt->fad+flt->range);
				match = 0;
				//lastbuf = flt->condfalse;
				//flt = &filters[lastbuf];
			}
		}

		if ((trktype != cdrom_file::CD_TRACK_AUDIO) && (curblock.data[15] == 2))
		{
			if (flt->mode & 1)  // file number
			{
				if (curblock.fnum != flt->fid)
				{
					LOGWARN("fnum reject\n");
					match = 0;
				}
			}

			if (flt->mode & 2)  // channel number
			{
				if (curblock.chan != flt->chan)
				{
					LOGWARN("channel number reject\n");
					match = 0;
				}
			}

			if (flt->mode & 4)  // sub mode
			{
				if((curblock.subm & flt->smmask) != flt->smval)
				{
					LOGWARN("sub mode reject\n");
					match = 0;
				}
			}

			if (flt->mode & 8)  // coding information
			{
				if((curblock.cinf & flt->cimask) != flt->cival)
				{
					LOGWARN("coding information reject\n");
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
			//lastbuf = flt->condtrue;
			//filterprt = &partitions[lastbuf];
			// we're done
			keepgoing = 0;
		}
		else
		{
			lastbuf = flt->condfalse;

			// reject sector if no match on either connector
			if ((lastbuf == 0xff) || (keepgoing == 0))
			{
				*p_ok = 0;
				return (partitionT *)nullptr;
			}

			// try again using the filter that was on the "false" connector
			flt = &filters[lastbuf];
			match = 1;

			// and exit if we fail
			keepgoing--;
		}
	} while (keepgoing);

	filterprt = &partitions[lastbuf];

	// try to allocate a block
	filterprt->blocks[filterprt->numblks] = cd_alloc_block(&filterprt->bnum[filterprt->numblks]);

	// did the allocation succeed?
	if (filterprt->blocks[filterprt->numblks] == (blockT *)nullptr)
	{
		*p_ok = 0;
		return (partitionT *)nullptr;
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
stvcd_device::partitionT *stvcd_device::cd_read_filtered_sector(int32_t fad, uint8_t *p_ok)
{
	int trktype;

	if ((cddevice != nullptr) && (!buffull))
	{
		// find out the track's type
		trktype = m_cdrom_image->get_track_type(m_cdrom_image->get_track(fad-150));

		// now get a raw 2352 byte sector - if it's mode 1, get mode1_raw
		if ((trktype == cdrom_file::CD_TRACK_MODE1) || (trktype == cdrom_file::CD_TRACK_MODE1_RAW))
		{
			m_cdrom_image->read_data(fad-150, curblock.data, cdrom_file::CD_TRACK_MODE1_RAW);
		}
		else if (trktype != cdrom_file::CD_TRACK_AUDIO) // if not audio it must be mode 2 so get mode2_raw
		{
			m_cdrom_image->read_data(fad-150, curblock.data, cdrom_file::CD_TRACK_MODE2_RAW);
		}
		else
		{
			m_cdrom_image->read_data(fad-150, curblock.data, cdrom_file::CD_TRACK_AUDIO);
		}

		curblock.size = sectlenin;
		curblock.FAD = fad;

		// if track is Mode 2, get the subheader values
		if ((trktype != cdrom_file::CD_TRACK_AUDIO) && (curblock.data[15] == 2))
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

		return cd_filterdata(cddevice, trktype, &*p_ok);
	}

	*p_ok = 0;
	return (partitionT *)nullptr;
}

// loads in data set up by a CD-block PLAY command
void stvcd_device::cd_playdata()
{
	if ((cd_stat & 0x0f00) == CD_STAT_SEEK)
	{
		int32_t fad_diff;
		LOGSEEK("PRE %08x %08x %08x %d\n",cd_curfad,cd_fad_seek,cd_stat,cd_fad_seek - cd_curfad);

		fad_diff = (cd_fad_seek - cd_curfad);

		/* Zero Divide wants this TODO: timings. */
		if(fad_diff > (750*cd_speed))
		{
			LOGSEEK("PRE FFWD %08x %08x %08x %d %d\n",cd_curfad,cd_fad_seek,cd_stat,cd_fad_seek - cd_curfad,750*cd_speed);
			cd_curfad += (750*cd_speed);
			LOGSEEK("POST FFWD %08x %08x %08x %d %d\n",cd_curfad,cd_fad_seek,cd_stat,cd_fad_seek - cd_curfad, 750*cd_speed);
		}
		else if(fad_diff < (-750*cd_speed))
		{
			LOGSEEK("PRE REW %08x %08x %08x %d %d\n",cd_curfad,cd_fad_seek,cd_stat,cd_fad_seek - cd_curfad, -750*cd_speed);
			cd_curfad -= (750*cd_speed);
			LOGSEEK("POST REW %08x %08x %08x %d %d\n",cd_curfad,cd_fad_seek,cd_stat,cd_fad_seek - cd_curfad, -750*cd_speed);
		}
		else
		{
			LOGSEEK("Ready\n");
			cd_curfad = cd_fad_seek;
			cd_stat = CD_STAT_PLAY;
		}

		return;
	}

	if (LIVE_CD_VIEW)
		popmessage("%04x %d %d", cd_stat, cd_curfad, fadstoplay);

	if ((cd_stat & 0x0f00) == CD_STAT_PLAY)
	{
		if (fadstoplay)
		{
			LOGXFER("STVCD: Reading FAD %d\n", cd_curfad);

			if (m_cdrom_image->exists())
			{
				uint8_t p_ok;

				if(m_cdrom_image->get_track_type(m_cdrom_image->get_track(cd_curfad)) != cdrom_file::CD_TRACK_AUDIO)
				{
					cd_read_filtered_sector(cd_curfad,&p_ok);
					m_cdda->stop_audio(); //stop any pending CD-DA
				}
				else
				{
					 // TODO: pinpoint cases when this isn't okay
					 // (out of bounds disc for example)
					p_ok = 1;
					m_cdda->start_audio(cd_curfad, 1);
				}

				if(p_ok)
				{
					cd_curfad++;
					fadstoplay--;
					hirqreg |= CSCT;
					sectorstore = 1;

					if (!fadstoplay)
					{
						if(cdda_repeat_count >= cdda_maxrepeat)
						{
							LOG("cd_playdata: playback ended\n");
							cd_stat = CD_STAT_PAUSE;

							hirqreg |= PEND;

							if (playtype == 1)
							{
								LOG("cd_playdata: setting EFLS\n");
								hirqreg |= EFLS;
							}
						}
						else
						{
							if(cdda_repeat_count < 0xe)
								cdda_repeat_count++;

							cd_curfad = m_cdrom_image->get_track_start(cur_track-1) + 150;
							fadstoplay = m_cdrom_image->get_track_start(cur_track) - cd_curfad;
						}
					}
				}
			}
		}
	}
}

// loads a single sector off the CD, anywhere from FAD 150 on up
void stvcd_device::cd_readblock(uint32_t fad, uint8_t *dat)
{
	if (m_cdrom_image->exists())
	{
		m_cdrom_image->read_data(fad-150, dat, cdrom_file::CD_TRACK_MODE1);
	}
}

void stvcd_device::set_tray_open()
{
	if(!tray_is_closed)
		return;

	hirqreg |= DCHG;
	cd_stat = CD_STAT_OPEN;

	// unmount the existing image, pretend that's what user wants if we are there.
	m_cdrom_image->unload();

	tray_is_closed = 0;

	popmessage("Tray Open");
}

void stvcd_device::set_tray_close()
{
	/* avoid user attempts to load a CD-ROM without opening the tray first (emulation asserts anyway with current framework) */
	if(tray_is_closed)
		return;

	hirqreg |= DCHG;

	if (m_cdrom_image->exists())
	{
		LOG("Opened CD-ROM successfully, reading root directory\n");
		//read_new_dir(0xffffff);  // read root directory
		cd_stat = CD_STAT_PAUSE;
	}
	else
	{
		cd_stat = CD_STAT_NODISC;
	}

	cd_speed = 2;
	cdda_repeat_count = 0;
	tray_is_closed = 1;

	popmessage("Tray Close");
}

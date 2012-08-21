#include "emu.h"
#include "includes/psx.h"
#include "psxcd.h"
#include "psxcddrv.h"

//#define debug_mess_driver

//
//
//

static const unsigned int	num_pf_sectors=32, num_pf_buffers=16;

//
//
//

cdrom_driver::cdrom_driver()
{
	pf_head = 0;
	pf_tail = 0;
	num_pf = 0;
	pf_head_sector = 0;
	pf_status = NULL;
	native_sector_size = raw_sector_size;
	last_pf_status = NULL;
	m_cd = NULL;
	m_machine = NULL;

//  printf("cdrom_driver base class init, pf_buffer size is %d\n", num_pf_sectors*num_pf_buffers*raw_sector_size);

	pf_buffer=new unsigned char [num_pf_sectors*num_pf_buffers*raw_sector_size];

	timestamp_frequency = 1;
}

cdrom_driver::~cdrom_driver()
{
	global_free(pf_buffer);
}

//
//
//

void cdrom_driver::set_native_sector_size(const unsigned int sz)
{
	native_sector_size=sz;
}

//
//
//

void cdrom_driver::cancel_io()
{
	if (pf_status)
	{
		pf_status->cancel();
		pf_status->release();
	}

	pf_status=NULL;
}

//
//
//

void cdrom_driver::prefetch_sector(const unsigned int sec)
{
	int numpfsec=num_pf*num_pf_sectors,
			pfsec=sec-pf_head_sector;

//  printf("prefetch_sector: %d\n", sec);

	if ((pfsec<0) || (pfsec>numpfsec))
	{
		// Sector is not in prefetch buffer, abort current prefetch

		if (pf_status)
		{
			pf_status->cancel();
			pf_status->release();
		}

		// Reset the buffer and begin a new prefetch

		pf_head=0;
		pf_tail=0;
		num_pf=1;
		pf_head_sector=sec;

		last_pf_status=NULL;
		pf_status=read_sectors(sec, num_pf_sectors, pf_buffer);
	}
}

//
//
//

bool cdrom_driver::is_prefetch_sector_loaded(const unsigned int sec)
{
	int numpfsec=num_pf*num_pf_sectors,
			pfsec=sec-pf_head_sector;

	if ((pfsec<0) || (pfsec>=numpfsec))
	{
		// Requested sector is not in prefetch buffer, begin a new prefetch

		prefetch_sector(sec);
		pfsec=sec-pf_head_sector;
		numpfsec=num_pf*num_pf_sectors;
	}

	int blk=pfsec/num_pf_sectors;

	if (blk>0)
	{
		// Discard blocks below the prefetch point

		pf_head=(pf_head+blk)%num_pf_buffers;
		pf_head_sector+=num_pf_sectors*blk;
		pfsec-=num_pf_sectors*blk;
		num_pf-=blk;
		blk=0;
	}

	bool comp=((! pf_status) || (pf_status->complete()));

	if (! comp)
	{
		INT64 curtime=m_machine->device<cpu_device>("maincpu")->total_cycles();

		if (last_pf_status!=pf_status)
		{
			last_pf_status=pf_status;
			pf_timeout_begin=curtime;
		}

		INT64 timeout=curtime-pf_timeout_begin;
		int timeout_sec=(int)(timeout/timestamp_frequency);
		if (timeout_sec>20)
		{
			printf("cdrom: prefetch timed out, trying again...\n");
			num_pf=0;
			pf_head_sector=-1;
			prefetch_sector(sec);
			return is_prefetch_sector_loaded(sec);
		}
	}

	if ((num_pf<num_pf_buffers) && (comp))
	{
		// The prefetch buffer is not full and we are not waiting on IO,
		// prefetch the next block

		pf_tail=(pf_tail+1)%num_pf_buffers;
		num_pf++;

		int nxtsec=pf_head_sector+((num_pf-1)*num_pf_sectors);
		unsigned char *ptr=pf_buffer+((pf_tail*num_pf_sectors)*native_sector_size);

		if (pf_status)
		{
			pf_status->release();
			pf_status=last_pf_status=NULL;
		}

		pf_status=read_sectors(nxtsec, num_pf_sectors, ptr);
	}

	if (blk==(num_pf-1))
	{
		// The sector we want is in the last block in the prefetch buffer
		// which might still be loading, check if the sector we want is loaded
#if 0 // we do not load async in MESS
		INT64 trans=pf_status->bytes_transferred();
		unsigned int secmod=pfsec%num_pf_sectors,
								 needbytes=(secmod+1)*native_sector_size;

		if (trans<needbytes)
		{
			// The sector is not loaded yet

			return false;
		} else
#endif
		{
			// The sector is loaded

			return true;
		}
	} else
	{
		// The sector is not in the last block so it must be loaded

		return true;
	}
}

//
//
//

unsigned char *cdrom_driver::get_prefetch_sector(const unsigned int sec, unsigned int *sz)
{
	int numpfsec=num_pf*num_pf_sectors,
			pfsec=sec-pf_head_sector;
	if ((pfsec>=0) && (pfsec<numpfsec))
	{
		int	blk=(pf_head+(pfsec/num_pf_sectors))%num_pf_buffers,
				off=((blk*num_pf_sectors)+(pfsec%num_pf_sectors))*native_sector_size;
		*sz=native_sector_size;
		return pf_buffer+off;
	} else
	{
		return NULL;
	}
}

//
//
//

bool cdrom_driver::read_sector(const unsigned int sec, unsigned char *buf, const bool block)
{
	bool loaded=is_prefetch_sector_loaded(sec);

//  printf("read_sector: %d (loaded=%c)\n", sec, loaded ? 'Y' : 'N');

	if ((! loaded) && (block))
	{
		pf_status->block_until_complete();
		loaded=true;
	}

	if (loaded)
	{
		unsigned int secsz=0;
		unsigned char *ptr=get_prefetch_sector(sec,&secsz);
		assert(ptr);

//      printf("got sector %d @ %p, size %d = %02x %02x | %02x %02x\n", sec, ptr, secsz, ptr[0], ptr[1], ptr[0x20], ptr[0x21]);

		if (secsz<2352)
		{
			// Add sector header

			buf[0]=0;
			memset(buf+1,0xff,10);
			buf[11]=0;
			sector_to_msf(sec,buf+12);
			buf[12]=decimal_to_bcd(buf[12]);
			buf[13]=decimal_to_bcd(buf[13]);
			buf[14]=decimal_to_bcd(buf[14]);
			buf[15]=2;

			if (secsz<2336)
			{
				memset(buf+16,0xff,8);
			}
		}

		switch (secsz)
		{
			case 2048:
				memcpy(buf+24,ptr,2048);
				break;

			case 2336:
				memcpy(buf+16,ptr,2336);
				break;

			case 2352:
				memcpy(buf,ptr,raw_sector_size);
				break;
		}

		return true;
	} else
	{
		return false;
	}
}

void cdrom_driver::set_machine(const running_machine &machine)
{
	m_machine = (running_machine *)&machine;

	timestamp_frequency = m_machine->device<cpu_device>("maincpu")->clock();

//  printf("cdrom_driver::set_machine: timestamp frequency = %d\n", timestamp_frequency);
}

/*
    MAME/MESS driver implementation
*/

class mess_cdrom_driver : public cdrom_driver
{
	enum track_type
	{
		track_illegal=-1,
		track_mode1_2048=0,
		track_mode2_2352,
		track_mode2_2336,
		track_audio
	};

	struct toc_entry
	{
		unsigned int type;
		unsigned char address[3];
	};

	int num_sectors, num_tracks;
	toc_entry toc[100];
	int bin_sector_size;
	const char *err;

	io_status *read_sectors(const unsigned int startsec, const unsigned int numsec, unsigned char *buf);

public:
	mess_cdrom_driver();
	~mess_cdrom_driver();

	bool is_usable(char *error_msg, const int msglen) const
	{
		return true;
	}

	bool read_toc();
	unsigned int get_first_track() const;
	unsigned int get_num_tracks() const;
	bool get_track_address(const unsigned int track,unsigned char *address) const;
	tracktype get_track_type(const unsigned int track) const;
	unsigned int find_track(const unsigned int sector, unsigned int *start_sector, unsigned int *end_sector) const;
	cdromtype get_type() const;
};

mess_cdrom_driver::mess_cdrom_driver()
	: err(NULL)
{
	for (int i=0; i<100; i++)
		toc[i].type=track_illegal;
	num_tracks=0;
	num_sectors=0;
}

//
//
//

mess_cdrom_driver::~mess_cdrom_driver()
{
	cancel_io();
}

//
//
//

bool mess_cdrom_driver::read_toc()
{
	if (m_cd)
	{
		const cdrom_toc *toc = cdrom_get_toc(m_cd);

		num_tracks = cdrom_get_last_track(m_cd);

		switch (toc->tracks[0].trktype)
		{
			case CD_TRACK_MODE1: bin_sector_size=2048; break;
			case CD_TRACK_MODE2_FORM1: bin_sector_size=2048; break;
			case CD_TRACK_MODE2: bin_sector_size=2336; break;
			case CD_TRACK_MODE2_FORM_MIX: bin_sector_size=2336; break;
			default: bin_sector_size=2352; break;
		}

		set_native_sector_size(bin_sector_size);
		num_sectors = cdrom_get_track_start(m_cd, num_tracks) + toc->tracks[num_tracks].frames;

//      printf("mess_cdrom_driver: %d sectors, native size %d\n",num_sectors, bin_sector_size);

		return true;
	}

	return false;
}

//
//
//

io_status *mess_cdrom_driver::read_sectors(const unsigned int startsec, const unsigned int numsec, unsigned char *buf)
{
	const cdrom_toc *toc = cdrom_get_toc(m_cd);
	UINT32 track = cdrom_get_track(m_cd, startsec);
	UINT32 secsize = toc->tracks[track].datasize;

	#ifdef debug_mess_driver
	printf("mess: read %d sectors from %d (secsize %d)\n",numsec,startsec,secsize);
	#endif

	if (1) // && ((int)startsec<num_sectors))
	{
//      fin->seek((INT64)startsec*(INT64)bin_sector_size);

//      io_status *ios=fin->async_read(buf,numsec*bin_sector_size);
//      if (! ios)
		for (int i = 0; i < numsec; i++)
		{
//          printf("[%d/%d] Reading to pf_buffer %p at offset %d, size %d\n", i, numsec, &buf[secsize*i], secsize*i, secsize);
			cdrom_read_data(m_cd, startsec+i, &buf[secsize*i], CD_TRACK_RAW_DONTCARE);
		}

		return NULL;

//      return ios;
	}
	else
	{
		printf("mess: read sector out of range (%d, max=%d)\n",
					 startsec,
					 num_sectors);

		memset(buf,0xff,numsec*bin_sector_size);
		return NULL;
	}
}

//
//
//

unsigned int mess_cdrom_driver::get_first_track() const
{
	return 1;
}

unsigned int mess_cdrom_driver::get_num_tracks() const
{
//  printf("get_num_tracks = %d\n", num_tracks);
	return num_tracks;
}

bool mess_cdrom_driver::get_track_address(const unsigned int track, unsigned char *address) const
{
	if ((track>=1) && ((int)track<=num_tracks))
	{
		UINT32 trkstart = cdrom_get_track_start(m_cd, track);

		sector_to_msf(trkstart, address);

		address[0] = address[0];
		address[1] = address[1];
		address[2] = address[2];

//      printf("get_track_address %d = %02x:%02x:%02x\n", track, address[0], address[1], address[2]);

		return true;
	}
	else
	{
		address[0]=address[1]=address[2]=0;
		return false;
	}
}

unsigned int mess_cdrom_driver::find_track(const unsigned int sector, unsigned int *start_sector, unsigned int *end_sector) const
{
	const cdrom_toc *toc = cdrom_get_toc(m_cd);
	UINT32 track = cdrom_get_track(m_cd, sector);
	int start;

	start = cdrom_get_track_start(m_cd, track);

	if (start_sector != NULL)
	{
		*start_sector = start;
	}

	if (end_sector != NULL)
	{
		*end_sector = start + toc->tracks[track].frames;
	}

	printf("find_track: track %d start %d end %d\n", track, start, start + toc->tracks[track].frames);

	return -1;
}

//
//
//

cdromtype mess_cdrom_driver::get_type() const
{
//  printf("get_type\n");
	return cdromtype_cd;
}

//
//
//

tracktype mess_cdrom_driver::get_track_type(const unsigned int track) const
{
	const cdrom_toc *toc = cdrom_get_toc(m_cd);

//  printf("get_track_type %d = %d\n", track, toc->tracks[track].trktype);

	switch (toc->tracks[track].trktype)
	{
		case CD_TRACK_MODE1: return tracktype_mode1;
		case CD_TRACK_MODE2_RAW: return tracktype_mode2;
		case CD_TRACK_MODE2_FORM_MIX: return tracktype_mode2;
		case CD_TRACK_AUDIO: return tracktype_audio;
		default: return tracktype_unknown;
	}
}

//
//
//

cdrom_driver *open_mess_drv()
{
	return new mess_cdrom_driver();
}


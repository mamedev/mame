// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/***************************************************************************

    Atari 400/800

    Floppy Disk Controller code

    Juergen Buchmueller, June 1998

***************************************************************************/

#include <ctype.h>

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/atari.h"
#include "atarifdc.h"
#include "sound/pokey.h"
#include "machine/6821pia.h"
#include "formats/atari_dsk.h"

#define VERBOSE_SERIAL  0
#define VERBOSE_CHKSUM  0

/*************************************
 *
 *  Disk stuff
 *
 *************************************/

#define FORMAT_XFD  0
#define FORMAT_ATR  1
#define FORMAT_DSK  2

/*****************************************************************************
 * This is the structure I used in my own Atari 800 emulator some years ago
 * Though it's a bit overloaded, I used it as it is the maximum of all
 * supported formats:
 * XFD no header at all
 * ATR 16 bytes header
 * DSK this struct
 * It is used to determine the format of a XFD image by it's size only
 *****************************************************************************/

struct atari_dsk_format
{
	UINT8 density;
	UINT8 tracks;
	UINT8 door;
	UINT8 sta1;
	UINT8 spt;
	UINT8 doublesided;
	UINT8 highdensity;
	UINT8 seclen_hi;
	UINT8 seclen_lo;
	UINT8 status;
	UINT8 sta2;
	UINT8 sta3;
	UINT8 sta4;
	UINT8 cr;
	UINT8 info[65+1];
};

/* combined with the size the image should have */
struct xfd_format
{
	int size;
	atari_dsk_format dsk;
};

/* here's a table of known xfd formats */
static const xfd_format xfd_formats[] =
{
	{35 * 18 * 1 * 128,                 {0,35,1,0,18,0,0,0,128,255,0,0,0,13,"35 SS/SD"}},
	{35 * 26 * 1 * 128,                 {1,35,1,0,26,0,4,0,128,255,0,0,0,13,"35 SS/MD"}},
	{(35 * 18 * 1 - 3) * 256 + 3 * 128, {2,35,1,0,18,0,4,1,  0,255,0,0,0,13,"35 SS/DD"}},
	{40 * 18 * 1 * 128,                 {0,40,1,0,18,0,0,0,128,255,0,0,0,13,"40 SS/SD"}},
	{40 * 26 * 1 * 128,                 {1,40,1,0,26,0,4,0,128,255,0,0,0,13,"40 SS/MD"}},
	{(40 * 18 * 1 - 3) * 256 + 3 * 128, {2,40,1,0,18,0,4,1,  0,255,0,0,0,13,"40 SS/DD"}},
	{40 * 18 * 2 * 128,                 {0,40,1,0,18,1,0,0,128,255,0,0,0,13,"40 DS/SD"}},
	{40 * 26 * 2 * 128,                 {1,40,1,0,26,1,4,0,128,255,0,0,0,13,"40 DS/MD"}},
	{(40 * 18 * 2 - 3) * 256 + 3 * 128, {2,40,1,0,18,1,4,1,  0,255,0,0,0,13,"40 DS/DD"}},
	{77 * 18 * 1 * 128,                 {0,77,1,0,18,0,0,0,128,255,0,0,0,13,"77 SS/SD"}},
	{77 * 26 * 1 * 128,                 {1,77,1,0,26,0,4,0,128,255,0,0,0,13,"77 SS/MD"}},
	{(77 * 18 * 1 - 3) * 256 + 3 * 128, {2,77,1,0,18,0,4,1,  0,255,0,0,0,13,"77 SS/DD"}},
	{77 * 18 * 2 * 128,                 {0,77,1,0,18,1,0,0,128,255,0,0,0,13,"77 DS/SD"}},
	{77 * 26 * 2 * 128,                 {1,77,1,0,26,1,4,0,128,255,0,0,0,13,"77 DS/MD"}},
	{(77 * 18 * 2 - 3) * 256 + 3 * 128, {2,77,1,0,18,1,4,1,  0,255,0,0,0,13,"77 DS/DD"}},
	{80 * 18 * 2 * 128,                 {0,80,1,0,18,1,0,0,128,255,0,0,0,13,"80 DS/SD"}},
	{80 * 26 * 2 * 128,                 {1,80,1,0,26,1,4,0,128,255,0,0,0,13,"80 DS/MD"}},
	{(80 * 18 * 2 - 3) * 256 + 3 * 128, {2,80,1,0,18,1,4,1,  0,255,0,0,0,13,"80 DS/DD"}},
	{0, {0,}}
};

/*****************************************************************************
 *
 * Open a floppy image for drive 'drive' if it is not yet openend
 * and a name was given. Determine the image geometry depending on the
 * type of image and store the results into the global drv[] structure
 *
 *****************************************************************************/

#define MAXSIZE 5760 * 256 + 80
static void _atari_load_proc(device_image_interface &image)
{
	atari_fdc_device *atarifdc = static_cast<atari_fdc_device *>(image.device().owner());
	atarifdc->atari_load_proc(image);
}

void atari_fdc_device::atari_load_proc(device_image_interface &image)
{
	int id = floppy_get_drive(image);
	int size, i;
	const char *ext;

	m_drv[id].image = std::make_unique<UINT8[]>(MAXSIZE);
	if (!m_drv[id].image)
		return;

	/* tell whether the image is writable */
	m_drv[id].mode = !image.is_readonly();
	/* set up image if it has been created */
	if (image.has_been_created())
	{
		int sector;
		char buff[256];
		memset(buff, 0, sizeof(buff));
		/* default to 720 sectors */
		for( sector = 0; sector < 720; sector++ )
			image.fwrite(buff, 256);
		image.fseek(0, SEEK_SET);
	}

	size = image.fread(m_drv[id].image.get(), MAXSIZE);

	if( size <= 0 )
	{
		m_drv[id].image = nullptr;
		return;
	}


	/* re allocate the buffer; we don't want to be too lazy ;) */
	//m_drv[id].image = (UINT8*)image.image_realloc(m_drv[id].image, size);

	ext = image.filetype();

	// hack alert, this means we can only load ATR via the softlist at the moment, image.filetype reutrns NULL :/
	if (image.software_entry() != nullptr) ext="ATR";

	/* no extension: assume XFD format (no header) */
	if (!ext)
	{
		m_drv[id].type = FORMAT_XFD;
		m_drv[id].header_skip = 0;
	}
	else
	/* XFD extension */
	if( toupper(ext[0])=='X' && toupper(ext[1])=='F' && toupper(ext[2])=='D' )
	{
		m_drv[id].type = FORMAT_XFD;
		m_drv[id].header_skip = 0;
	}
	else
	/* ATR extension */
	if( toupper(ext[0])=='A' && toupper(ext[1])=='T' && toupper(ext[2])=='R' )
	{
		m_drv[id].type = FORMAT_ATR;
		m_drv[id].header_skip = 16;
	}
	else
	/* DSK extension */
	if( toupper(ext[0])=='D' && toupper(ext[1])=='S' && toupper(ext[2])=='K' )
	{
		m_drv[id].type = FORMAT_DSK;
		m_drv[id].header_skip = sizeof(atari_dsk_format);
	}
	else
	{
		m_drv[id].type = FORMAT_XFD;
		m_drv[id].header_skip = 0;
	}

	if( m_drv[id].type == FORMAT_ATR &&
		(m_drv[id].image[0] != 0x96 || m_drv[id].image[1] != 0x02) )
	{
		m_drv[id].type = FORMAT_XFD;
		m_drv[id].header_skip = 0;
	}


	switch (m_drv[id].type)
	{
	/* XFD or unknown format: find a matching size from the table */
	case FORMAT_XFD:
		for( i = 0; xfd_formats[i].size; i++ )
		{
			if( size == xfd_formats[i].size )
			{
				m_drv[id].density = xfd_formats[i].dsk.density;
				m_drv[id].tracks = xfd_formats[i].dsk.tracks;
				m_drv[id].spt = xfd_formats[i].dsk.spt;
				m_drv[id].heads = (xfd_formats[i].dsk.doublesided) ? 2 : 1;
				m_drv[id].bseclen = 128;
				m_drv[id].seclen = 256 * xfd_formats[i].dsk.seclen_hi + xfd_formats[i].dsk.seclen_lo;
				m_drv[id].sectors = m_drv[id].tracks * m_drv[id].heads * m_drv[id].spt;
				break;
			}
		}
		break;
	/* ATR format: find a size including the 16 bytes header */
	case FORMAT_ATR:
		{
			int s;
			m_drv[id].bseclen = 128;
			/* get sectors from ATR header */
			s = (size - 16) / 128;
			/* 3 + odd number of sectors ? */
			if ( m_drv[id].image[4] == 128 || (s % 18) == 0 || (s % 26) == 0 || ((s - 3) % 1) != 0 )
			{
				m_drv[id].sectors = s;
				m_drv[id].seclen = 128;
				/* sector size 128 or count not evenly dividable by 26 ? */
				if( m_drv[id].seclen == 128 || (s % 26) != 0 )
				{
					/* yup! single density */
					m_drv[id].density = 0;
					m_drv[id].spt = 18;
					m_drv[id].heads = 1;
					m_drv[id].tracks = s / 18;
					if( s % 18 != 0 )
						m_drv[id].tracks += 1;
					if( m_drv[id].tracks % 2 == 0 && m_drv[id].tracks > 80 )
					{
						m_drv[id].heads = 2;
						m_drv[id].tracks /= 2;
					}
				}
				else
				{
					/* yes: medium density */
					m_drv[id].density = 0;
					m_drv[id].spt = 26;
					m_drv[id].heads = 1;
					m_drv[id].tracks = s / 26;
					if( s % 26 != 0 )
						m_drv[id].tracks += 1;
					if( m_drv[id].tracks % 2 == 0 && m_drv[id].tracks > 80 )
					{
						m_drv[id].heads = 2;
						m_drv[id].tracks /= 2;
					}
				}
			}
			else
			{
				/* it's double density */
				s = (s - 3) / 2 + 3;
				m_drv[id].sectors = s;
				m_drv[id].density = 2;
				m_drv[id].seclen = 256;
				m_drv[id].spt = 18;
				m_drv[id].heads = 1;
				m_drv[id].tracks = s / 18;
				if( s % 18 != 0 )
					m_drv[id].tracks += 1;
				if( m_drv[id].tracks % 2 == 0 && m_drv[id].tracks > 80 )
				{
					m_drv[id].heads = 2;
					m_drv[id].tracks /= 2;
				}
			}
		}
		break;
	/* DSK format: it's all in the header */
	case FORMAT_DSK:
		{
			atari_dsk_format *dsk = (atari_dsk_format *) m_drv[id].image.get();

			m_drv[id].tracks = dsk->tracks;
			m_drv[id].spt = dsk->spt;
			m_drv[id].heads = (dsk->doublesided) ? 2 : 1;
			m_drv[id].seclen = 256 * dsk->seclen_hi + dsk->seclen_lo;
			m_drv[id].bseclen = m_drv[id].seclen;
			m_drv[id].sectors = m_drv[id].tracks * m_drv[id].heads * m_drv[id].spt;
		}
		break;
	}
	logerror("atari opened floppy '%s', %d sectors (%d %s%s) %d bytes/sector\n",
			image.filename(),
			m_drv[id].sectors,
			m_drv[id].tracks,
			(m_drv[id].heads == 1) ? "SS" : "DS",
			(m_drv[id].density == 0) ? "SD" : (m_drv[id].density == 1) ? "MD" : "DD",
			m_drv[id].seclen);
	return;
}



/*****************************************************************************
 *
 * This is a description of the data flow between Atari (A) and the
 * Floppy (F) for the supported commands.
 *
 * A->F     DEV  CMD  AUX1 AUX2 CKS
 *          '1'  'S'  00   00                 get status
 * F->A     ACK  CPL  04   FF   E0   00   CKS
 *                     ^    ^
 *                     |    |
 *                     |    bit 7 : door closed
 *                     |
 *                     bit7  : MD 128 bytes/sector, 26 sectors/track
 *                     bit5  : DD 256 bytes/sector, 18 sectors/track
 *                     else  : SD 128 bytes/sector, 18 sectors/track
 *
 * A->F     DEV  CMD  AUX1 AUX2 CKS
 *          '1'  'R'  SECL SECH               read sector
 * F->A     ACK                               command acknowledge
 *               ***                          now read the sector
 * F->A              CPL                      complete: sector read
 * F->A                  128/256 byte CKS
 *
 * A->F     DEV  CMD  AUX1 AUX2 CKS
 *          '1'  'W'  SECL SECH               write with verify
 * F->A     ACK                               command acknowledge
 * A->F          128/256 data CKS
 * F->A                            CPL        complete: CKS okay
 *          execute writing the sector
 * F->A                                 CPL   complete: sector written
 *
 * A->F     DEV  CMD  AUX1 AUX2 CKS
 *          '1'  'P'  SECL SECH               put sector
 * F->A     ACK                               command acknowledge
 * A->F          128/256 data CKS
 * F->A                            CPL        complete: CKS okay
 *          execute writing the sector
 * F->A                                 CPL   complete: sector written
 *
 * A->F     DEV  CMD  AUX1 AUX2 CKS
 *           '1' '!'  xx   xx                 single density format
 * F->A     ACK                               command acknowledge
 *          execute formatting
 * F->A               CPL                     complete: format
 * F->A                    128/256 byte CKS   bad sector table
 *
 *
 * A->F     DEV  CMD  AUX1 AUX2 CKS
 *          '1'  '"'  xx   xx                 double density format
 * F->A     ACK                               command acknowledge
 *          execute formatting
 * F->A               CPL                     complete: format
 * F->A                    128/256 byte CKS   bad sector table
 *
 *****************************************************************************/
static void make_chksum(device_t *device,UINT8 * chksum, UINT8 data)
{
	UINT8 newone;
	newone= *chksum + data;
	if (newone < *chksum)
		newone++;

	if (VERBOSE_CHKSUM)
		device->logerror("atari chksum old $%02x + data $%02x -> new $%02x\n", *chksum, data, newone);

	*chksum = newone;
}

void atari_fdc_device::clr_serout(int expect_data)
{
	m_serout_chksum = 0;
	m_serout_offs = 0;
	m_serout_count = expect_data + 1;
}

void atari_fdc_device::add_serout(int expect_data)
{
	m_serout_chksum = 0;
	m_serout_count = expect_data + 1;
}

void atari_fdc_device::clr_serin(int ser_delay)
{
	pokey_device *pokey = machine().device<pokey_device>("pokey");
	m_serin_chksum = 0;
	m_serin_offs = 0;
	m_serin_count = 0;
	pokey->serin_ready(ser_delay * 40);
}

void atari_fdc_device::add_serin(UINT8 data, int with_checksum)
{
	m_serin_buff[m_serin_count++] = data;
	if (with_checksum)
		make_chksum(this,&m_serin_chksum, data);
}

static void ATTR_PRINTF(1,2) atari_set_frame_message(const char *fmt, ...)
{
	//va_list arg;
	//va_start(arg, fmt);

	//vsprintf(atari_frame_message, fmt, arg);
	//atari_frame_counter = 30; /* FIXME */

	//va_end(arg);
}

void atari_fdc_device::a800_serial_command()
{
	int i, drive, sector, offset;

	if( !m_serout_offs )
	{
		if (VERBOSE_SERIAL)
			logerror("atari serout command offset = 0\n");
		return;
	}
	clr_serin(10);

	if (VERBOSE_SERIAL)
	{
		logerror("atari serout command %d: %02X %02X %02X %02X %02X : %02X ",
			m_serout_offs,
			m_serout_buff[0], m_serout_buff[1], m_serout_buff[2],
			m_serout_buff[3], m_serout_buff[4], m_serout_chksum);
	}

	if (m_serout_chksum == 0)
	{
		if (VERBOSE_SERIAL)
			logerror("OK\n");

		drive = m_serout_buff[0] - '1';   /* drive # */
		/* sector # */
		if (drive < 0 || drive > 3)             /* ignore unknown drives */
		{
			logerror("atari unsupported drive #%d\n", drive+1);
			atari_set_frame_message("DRIVE #%d not supported", drive+1);
			return;
		}

		/* extract sector number from the command buffer */
		sector = m_serout_buff[2] + 256 * m_serout_buff[3];

		switch (m_serout_buff[1]) /* command ? */
		{
			case 'S':   /* status */
				atari_set_frame_message("DRIVE #%d STATUS", drive+1);

				if (VERBOSE_SERIAL)
					logerror("atari status\n");

				add_serin('A',0);
				add_serin('C',0);
				if (!m_drv[drive].mode) /* read only mode ? */
				{
					if (m_drv[drive].spt == 26)
						add_serin(0x80,1);   /* MD: 0x80 */
					else
					if (m_drv[drive].seclen == 128)
						add_serin(0x00,1);   /* SD: 0x00 */
					else
						add_serin(0x20,1);   /* DD: 0x20 */
				}
				else
				{
					if (m_drv[drive].spt == 26)
						add_serin(0x84,1);   /* MD: 0x84 */
					else
					if (m_drv[drive].seclen == 128)
						add_serin(0x04,1);   /* SD: 0x04 */
					else
						add_serin(0x24,1);   /* DD: 0x24 */
				}
				if (m_drv[drive].image)
					add_serin(0xff,1);   /* door closed: 0xff */
				else
					add_serin(0x7f,1);   /* door open: 0x7f */
				add_serin(0xe0,1);   /* dunno */
				add_serin(0x00,1);   /* dunno */
				add_serin(m_serin_chksum,0);
				break;

			case 'R':   /* read sector */
				if (VERBOSE_SERIAL)
					logerror("atari read sector #%d\n", sector);

				if( sector < 1 || sector > m_drv[drive].sectors )
				{
					atari_set_frame_message("DRIVE #%d READ SECTOR #%3d - ERR", drive+1, sector);

					if (VERBOSE_SERIAL)
						logerror("atari bad sector #\n");

					add_serin('E',0);
					break;
				}
				add_serin('A',0);   /* acknowledge */
				add_serin('C',0);   /* completed */
				if (sector < 4)     /* sector 1 .. 3 might be different length */
				{
					atari_set_frame_message("DRIVE #%d READ SECTOR #%3d - SD", drive+1, sector);
					offset = (sector - 1) * m_drv[drive].bseclen + m_drv[drive].header_skip;
					for (i = 0; i < 128; i++)
						add_serin(m_drv[drive].image[offset++],1);
				}
				else
				{
					atari_set_frame_message("DRIVE #%d READ SECTOR #%3d - %cD", drive+1, sector, (m_drv[drive].seclen == 128) ? 'S' : 'D');
					offset = (sector - 1) * m_drv[drive].seclen + m_drv[drive].header_skip;
					for (i = 0; i < m_drv[drive].seclen; i++)
						add_serin(m_drv[drive].image[offset++],1);
				}
				add_serin(m_serin_chksum,0);
				break;

			case 'W':   /* write sector with verify */
				if (VERBOSE_SERIAL)
					logerror("atari write sector #%d\n", sector);

				add_serin('A',0);
				if (sector < 4)     /* sector 1 .. 3 might be different length */
				{
					add_serout(m_drv[drive].bseclen);
					atari_set_frame_message("DRIVE #%d WRITE SECTOR #%3d - SD", drive+1, sector);
				}
				else
				{
					add_serout(m_drv[drive].seclen);
					atari_set_frame_message("DRIVE #%d WRITE SECTOR #%3d - %cD", drive+1, sector, (m_drv[drive].seclen == 128) ? 'S' : 'D');
				}
				break;

			case 'P':   /* put sector (no verify) */
				if (VERBOSE_SERIAL)
					logerror("atari put sector #%d\n", sector);

				add_serin('A',0);
				if (sector < 4)     /* sector 1 .. 3 might be different length */
				{
					add_serout(m_drv[drive].bseclen);
					atari_set_frame_message("DRIVE #%d PUT SECTOR #%3d - SD", drive+1, sector);
				}
				else
				{
					add_serout(m_drv[drive].seclen);
					atari_set_frame_message("DRIVE #%d PUT SECTOR #%3d - %cD", drive+1, sector, (m_drv[drive].seclen == 128) ? 'S' : 'D');
				}
				break;

			case '!':   /* SD format */
				if (VERBOSE_SERIAL)
					logerror("atari format SD drive #%d\n", drive+1);

				atari_set_frame_message("DRIVE #%d FORMAT SD", drive+1);
				add_serin('A',0);   /* acknowledge */
				add_serin('C',0);   /* completed */
				for (i = 0; i < 128; i++)
					add_serin(0,1);
				add_serin(m_serin_chksum,0);
				break;

			case '"':   /* DD format */
				if (VERBOSE_SERIAL)
					logerror("atari format DD drive #%d\n", drive+1);

				atari_set_frame_message("DRIVE #%d FORMAT DD", drive+1);
				add_serin('A',0);   /* acknowledge */
				add_serin('C',0);   /* completed */
				for (i = 0; i < 256; i++)
					add_serin(0,1);
				add_serin(m_serin_chksum,0);
				break;

			default:
				if (VERBOSE_SERIAL)
					logerror("atari unknown command #%c\n", m_serout_buff[1]);

				atari_set_frame_message("DRIVE #%d UNKNOWN CMD '%c'", drive+1, m_serout_buff[1]);
				add_serin('N',0);   /* negative acknowledge */
		}
	}
	else
	{
		atari_set_frame_message("serial cmd chksum error");
		if (VERBOSE_SERIAL)
			logerror("BAD\n");

		add_serin('E',0);
	}
	if (VERBOSE_SERIAL)
		logerror("atari %d bytes to read\n", m_serin_count);
}

void atari_fdc_device::a800_serial_write()
{
	int i, drive, sector, offset;

	if (VERBOSE_SERIAL)
	{
		logerror("atari serout %d bytes written : %02X ",
			m_serout_offs, m_serout_chksum);
	}

	clr_serin(80);
	if (m_serout_chksum == 0)
	{
		if (VERBOSE_SERIAL)
			logerror("OK\n");

		add_serin('C',0);
		/* write the sector */
		drive = m_serout_buff[0] - '1';   /* drive # */
		/* not write protected and image available ? */
		if (m_drv[drive].mode && m_drv[drive].image)
		{
			/* extract sector number from the command buffer */
			sector = m_serout_buff[2] + 256 * m_serout_buff[3];
			if (sector < 4)     /* sector 1 .. 3 might be different length */
			{
				offset = (sector - 1) * m_drv[drive].bseclen + m_drv[drive].header_skip;

				if (VERBOSE_SERIAL)
					logerror("atari storing 128 byte sector %d at offset 0x%08X", sector, offset );

				for (i = 0; i < 128; i++)
					m_drv[drive].image[offset++] = m_serout_buff[5+i];
				atari_set_frame_message("DRIVE #%d WROTE SECTOR #%3d - SD", drive+1, sector);
			}
			else
			{
				offset = (sector - 1) * m_drv[drive].seclen + m_drv[drive].header_skip;

				if (VERBOSE_SERIAL)
					logerror("atari storing %d byte sector %d at offset 0x%08X", m_drv[drive].seclen, sector, offset );

				for (i = 0; i < m_drv[drive].seclen; i++)
					m_drv[drive].image[offset++] = m_serout_buff[5+i];
				atari_set_frame_message("DRIVE #%d WROTE SECTOR #%3d - %cD", drive+1, sector, (m_drv[drive].seclen == 128) ? 'S' : 'D');
			}
			add_serin('C',0);
		}
		else
		{
			add_serin('E',0);
		}
	}
	else
	{
		if (VERBOSE_SERIAL)
			logerror("BAD\n");

		add_serin('E',0);
	}
}

READ8_MEMBER( atari_fdc_device::serin_r )
{
	int data = 0x00;
	int ser_delay = 0;

	if (m_serin_count)
	{
		pokey_device *pokey = machine().device<pokey_device>("pokey");

		data = m_serin_buff[m_serin_offs];
		ser_delay = 2 * 40;
		if (m_serin_offs < 3)
		{
			ser_delay = 4 * 40;
			if (m_serin_offs < 2)
				ser_delay = 200 * 40;
		}
		m_serin_offs++;
		if (--m_serin_count == 0)
			m_serin_offs = 0;
		else
			pokey->serin_ready(ser_delay);
	}

	if (VERBOSE_SERIAL)
		logerror("atari serin[$%04x] -> $%02x; delay %d\n", m_serin_offs, data, ser_delay);

	return data;
}

WRITE8_MEMBER( atari_fdc_device::serout_w )
{
	pia6821_device *pia = machine().device<pia6821_device>( "pia" );

	/* ignore serial commands if no floppy image is specified */
	if( !m_drv[0].image )
		return;
	if (m_serout_count)
	{
		/* store data */
		m_serout_buff[m_serout_offs] = data;

		if (VERBOSE_SERIAL)
			logerror("atari serout[$%04x] <- $%02x; count %d\n", m_serout_offs, data, m_serout_count);

		m_serout_offs++;
		if (--m_serout_count == 0)
		{
			/* exclusive or written checksum with calculated */
			m_serout_chksum ^= data;
			/* if the attention line is high, this should be data */
			if (pia->irq_b_state())
				a800_serial_write();
		}
		else
		{
			make_chksum(this,&m_serout_chksum, data);
		}
	}
}

WRITE_LINE_MEMBER(atari_fdc_device::pia_cb2_w)
{
	if (!state)
	{
		clr_serout(4); /* expect 4 command bytes + checksum */
	}
	else
	{
		m_serin_delay = 0;
		a800_serial_command();
	}
}

static const floppy_interface atari_floppy_interface =
{
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(atari_only),
	"floppy_5_25"
};

static MACHINE_CONFIG_FRAGMENT( atari_fdc )
	MCFG_LEGACY_FLOPPY_4_DRIVES_ADD(atari_floppy_interface)
MACHINE_CONFIG_END

legacy_floppy_image_device *atari_fdc_device::atari_floppy_get_device_child(int drive)
{
	switch(drive) {
		case 0 : return subdevice<legacy_floppy_image_device>(FLOPPY_0);
		case 1 : return subdevice<legacy_floppy_image_device>(FLOPPY_1);
		case 2 : return subdevice<legacy_floppy_image_device>(FLOPPY_2);
		case 3 : return subdevice<legacy_floppy_image_device>(FLOPPY_3);
	}
	return nullptr;
}

const device_type ATARI_FDC = &device_creator<atari_fdc_device>;

atari_fdc_device::atari_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ATARI_FDC, "Atari FDC", tag, owner, clock, "atari_fdc", __FILE__),
	m_serout_count(0),
	m_serout_offs(0),
	m_serout_chksum(0),
//  m_serout_delay(0),
	m_serin_count(0),
	m_serin_offs(0),
	m_serin_chksum(0),
	m_serin_delay(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void atari_fdc_device::device_start()
{
	int id;

	memset(m_serout_buff, 0, sizeof(m_serout_buff));
	memset(m_serin_buff, 0, sizeof(m_serin_buff));
	memset(m_drv, 0, sizeof(m_drv));

	for(id=0;id<4;id++)
	{
		atari_floppy_get_device_child(id)->floppy_install_load_proc(_atari_load_proc);
	}
}

//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor atari_fdc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( atari_fdc  );
}

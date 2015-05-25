// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Kevin Thacker, Phill Harvey-Smith, Robbbert, Curt Coder
/***************************************************************************

    !!! DEPRECATED, USE src/emu/wd_fdc.h FOR NEW DRIVERS !!!

    wd17xx.c

    Implementations of the Western Digital 17xx and 27xx families of
    floppy disk controllers


    Models:

              DAL   DD   Side   Clock       Remark
      ---------------------------------------------------------
      FD1771                    1 or 2 MHz  First model
      FD1781         x          1 or 2 MHz
      FD1791         x          1 or 2 MHz
      FD1792                    1 or 2 MHz
      FD1793   x     x          1 or 2 MHz
      FD1794   x                1 or 2 MHz
      FD1795         x     x    1 or 2 MHz
      FD1797   x     x     x    1 or 2 MHz
      FD1761         x          1 MHz
      FD1762                    1 MHz       ?
      FD1763   x     x          1 MHz
      FD1764   x                1 MHz       ?
      FD1765         x     x    1 MHz
      FD1767   x     x     x    1 MHz
      WD2791         x          1 or 2 MHz  Internal data separator
      WD2793   x     x          1 or 2 MHz  Internal data separator
      WD2795         x     x    1 or 2 MHz  Internal data separator
      WD2797   x     x     x    1 or 2 MHz  Internal data separator
      WD1770   x     x          8 MHz       Motor On signal
      WD1772   x     x          8 MHz       Motor On signal, Faster stepping rates
      WD1773   x     x          8 MHz       Enable precomp line

      Notes: - In general, later models include all features of earlier models
             - DAL: Data access lines, x = TRUE; otherwise inverted
             - DD: Double density support
             - Side: Has side select support
             - ?: Unknown if it really exists

    Clones:

      - SMC FD179x
      - Fujitsu MB8876 -> FD1791, MB8877 -> FD1793
      - VLSI VL177x


    Changelog:

    Kevin Thacker
        - Removed disk image code and replaced it with floppy drive functions.
          Any disc image is now useable with this code.
        - Fixed write protect

    2005-Apr-16 P.Harvey-Smith:
        - Increased the delay in wd17xx_timed_read_sector_request and
          wd17xx_timed_write_sector_request, to 40us to more closely match
          what happens on the real hardware, this has fixed the NitrOS9 boot
          problems.

    2007-Nov-01 Wilbert Pol:
        Needed these changes to get the MB8877 for Osborne-1 to work:
        - Added support for multiple record read
        - Changed the wd17xx_read_id to not return after DATADONEDELAY, but
          the host should read the id data through the data register. This
          was accomplished by making this change in the wd17xx_read_id
          function:
            -               wd17xx_complete_command(device, DELAY_DATADONE);
            +               wd17xx_set_data_request();

    2009-May-10 Robbbert:
        Further change to get the Kaypro II to work
        - When wd17xx_read_id has generated the 6 data bytes, it should make
          an IRQ and turn off the busy status. The timing for Osborne is
          critical, it must be between 300 and 700 usec, I've settled on 400.
          The Kaypro doesn't care timewise, but the busy flag must turn off
          sometime or it hangs.
            -       w->status |= STA_2_BUSY;
            +       wd17xx_set_busy(device, attotime::from_usec(400));

    2009-June-4 Roberto Lavarone:
        - Initial support for wd1771
        - Added simulation of head loaded feedback from drive
        - Bugfix: busy flag was cleared too early

    2009-June-21 Robbbert:
    - The Bugfix above, while valid, caused the Osborne1 to fail. This
      is because the delay must not exceed 14usec (found by extensive testing).
    - The minimum delay is 1usec, need by z80netf while formatting a disk.
    - http://www.bannister.org/forums/ubbthreads.php?ubb=showflat&Number=50889#Post50889
      explains the problems, testing done, and the test procedure for the z80netf.
    - Thus, the delay is set to 10usec, and all the disks I have (not many)
      seem to work.
    - Note to anyone who wants to change something: Make sure that the
      Osborne1 boots up! It is extremely sensitive to timing!
    - For testing only: The osborne1 rom can be patched to make it much
      more stable, by changing the byte at 0x0da7 from 0x28 to 0x18.

    2009-June-25 Robbbert:
    - Turns out kayproii not working, 10usec is too short.. but 11usec is ok.
      Setting it to 12usec.
      Really, this whole thing needs a complete rewrite.

    2009-July-08 Roberto Lavarone:
    - Fixed a bug in head load flag handling: einstein and samcoupe now working again

    2009-September-30 Robbbert:
    - Modified what status flags are returned after a Forced Interrupt,
      to allow Microbee to boot CP/M.

    2010-Jan-31 Phill Harvey-Smith
    - The above bugfixes for the Kaypro/Osborne1 have borked the booting on the Dragon
      Alpha. The Alpha it seems needs a delay of ay least 17us or the NMI generated by
      INTRQ happens too early and doen't break out of the read/write bytes loops.

      I have made the delay settable by calling wd17xx_set_complete_command_delay, and
      let it default to 12us, as required above, so that the Dragon Alpha works again.
      This hopefully should not break the other machines.

      This should probably be considdered a minor hack but it does seem to work !

    2010-02-04 Phill Harvey-Smith
    - Added multiple sector write as the RM Nimbus needs it.

    2010-March-22 Curt Coder:
    - Implemented immediate and index pulse interrupts.

    2010-Dec-31 Phill Harvey-Smith
    - Copied multi-sector write code from r7263, for some reason this had been
      silently removed, but is required for the rmnimbus driver.

    2011-Mar-08 Phill Harvey-Smith
    - Triggering intrq now clears the DRQ bit in the status as well as the busy bit.
      Execution of the READ_DAM command now correctly sets w->command.

    2011-Apr-01 Curt Coder
    - Set complete command delay to 16 usec (DD) / 32 usec (SD) and removed
      the external delay setting hack.

    2011-Jun-24 Curt Coder
    - Added device types for all known variants, and enforced inverted DAL lines.

    2011-Sep-18 Curt Coder
    - Connected Side Select Output for variants that support it.

    TODO:
        - What happens if a track is read that doesn't have any id's on it?
         (e.g. unformatted disc)
        - Rewrite into a C++ device

***************************************************************************/


#include "emu.h"
#include "formats/imageutl.h"
#include "machine/wd17xx.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define VERBOSE         0   /* General logging */
#define VERBOSE_DATA    0   /* Logging of each byte during read and write */

#define DELAY_ERROR     3
#define DELAY_NOTREADY  1
#define DELAY_DATADONE  3

#define TYPE_I          1
#define TYPE_II         2
#define TYPE_III        3
#define TYPE_IV         4

#define FDC_STEP_RATE   0x03    /* Type I additional flags */
#define FDC_STEP_VERIFY 0x04    /* verify track number */
#define FDC_STEP_HDLOAD 0x08    /* load head */
#define FDC_STEP_UPDATE 0x10    /* update track register */

#define FDC_RESTORE     0x00    /* Type I commands */
#define FDC_SEEK        0x10
#define FDC_STEP        0x20
#define FDC_STEP_IN     0x40
#define FDC_STEP_OUT    0x60

#define FDC_MASK_TYPE_I         (FDC_STEP_HDLOAD|FDC_STEP_VERIFY|FDC_STEP_RATE)

/* Type I commands status */
#define STA_1_BUSY      0x01    /* controller is busy */
#define STA_1_IPL       0x02    /* index pulse */
#define STA_1_TRACK0    0x04    /* track 0 detected */
#define STA_1_CRC_ERR   0x08    /* CRC error */
#define STA_1_SEEK_ERR  0x10    /* seek error */
#define STA_1_HD_LOADED 0x20    /* head loaded */
#define STA_1_WRITE_PRO 0x40    /* floppy is write protected */
#define STA_1_NOT_READY 0x80    /* drive not ready */
#define STA_1_MOTOR_ON  0x80    /* status of the Motor On output (WD1770 and WD1772 only) */

/* Type II and III additional flags */
#define FDC_DELETED_AM  0x01    /* read/write deleted address mark */
#define FDC_SIDE_CMP_T  0x02    /* side compare track data */
#define FDC_15MS_DELAY  0x04    /* delay 15ms before command */
#define FDC_SIDE_CMP_S  0x08    /* side compare sector data */
#define FDC_MULTI_REC   0x10    /* only for type II commands */

/* Type II commands */
#define FDC_READ_SEC    0x80    /* read sector */
#define FDC_WRITE_SEC   0xA0    /* write sector */

#define FDC_MASK_TYPE_II        (FDC_MULTI_REC|FDC_SIDE_CMP_S|FDC_15MS_DELAY|FDC_SIDE_CMP_T|FDC_DELETED_AM)

/* Type II commands status */
#define STA_2_BUSY      0x01
#define STA_2_DRQ       0x02
#define STA_2_LOST_DAT  0x04
#define STA_2_CRC_ERR   0x08
#define STA_2_REC_N_FND 0x10
#define STA_2_REC_TYPE  0x20
#define STA_2_WRITE_PRO 0x40
#define STA_2_NOT_READY 0x80

#define FDC_MASK_TYPE_III       (FDC_SIDE_CMP_S|FDC_15MS_DELAY|FDC_SIDE_CMP_T|FDC_DELETED_AM)

/* Type III commands */
#define FDC_READ_DAM    0xc0    /* read data address mark */
#define FDC_READ_TRK    0xe0    /* read track */
#define FDC_WRITE_TRK   0xf0    /* write track (format) */

/* Type IV additional flags */
#define FDC_IM0         0x01    /* interrupt mode 0 */
#define FDC_IM1         0x02    /* interrupt mode 1 */
#define FDC_IM2         0x04    /* interrupt mode 2 */
#define FDC_IM3         0x08    /* interrupt mode 3 */

#define FDC_MASK_TYPE_IV        (FDC_IM3|FDC_IM2|FDC_IM1|FDC_IM0)

/* Type IV commands */
#define FDC_FORCE_INT   0xd0    /* force interrupt */

/* structure describing a double density track */
#define TRKSIZE_DD      6144
#if 0
static const UINT8 track_DD[][2] = {
	{16, 0x4e},     /* 16 * 4E (track lead in)               */
	{ 8, 0x00},     /*  8 * 00 (pre DAM)                     */
	{ 3, 0xf5},     /*  3 * F5 (clear CRC)                   */

	{ 1, 0xfe},     /* *** sector *** FE (DAM)               */
	{ 1, 0x80},     /*  4 bytes track,head,sector,seclen     */
	{ 1, 0xf7},     /*  1 * F7 (CRC)                         */
	{22, 0x4e},     /* 22 * 4E (sector lead in)              */
	{12, 0x00},     /* 12 * 00 (pre AM)                      */
	{ 3, 0xf5},     /*  3 * F5 (clear CRC)                   */
	{ 1, 0xfb},     /*  1 * FB (AM)                          */
	{ 1, 0x81},     /*  x bytes sector data                  */
	{ 1, 0xf7},     /*  1 * F7 (CRC)                         */
	{16, 0x4e},     /* 16 * 4E (sector lead out)             */
	{ 8, 0x00},     /*  8 * 00 (post sector)                 */
	{ 0, 0x00},     /* end of data                           */
};
#endif

/* structure describing a single density track */
#define TRKSIZE_SD      3172
#if 0
static const UINT8 track_SD[][2] = {
	{16, 0xff},     /* 16 * FF (track lead in)               */
	{ 8, 0x00},     /*  8 * 00 (pre DAM)                     */
	{ 1, 0xfc},     /*  1 * FC (clear CRC)                   */

	{11, 0xff},     /* *** sector *** 11 * FF                */
	{ 6, 0x00},     /*  6 * 00 (pre DAM)                     */
	{ 1, 0xfe},     /*  1 * FE (DAM)                         */
	{ 1, 0x80},     /*  4 bytes track,head,sector,seclen     */
	{ 1, 0xf7},     /*  1 * F7 (CRC)                         */
	{10, 0xff},     /* 10 * FF (sector lead in)              */
	{ 4, 0x00},     /*  4 * 00 (pre AM)                      */
	{ 1, 0xfb},     /*  1 * FB (AM)                          */
	{ 1, 0x81},     /*  x bytes sector data                  */
	{ 1, 0xf7},     /*  1 * F7 (CRC)                         */
	{ 0, 0x00},     /* end of data                           */
};
#endif


/***************************************************************************
    HELPER FUNCTIONS
***************************************************************************/

static int wd17xx_has_dal(device_t *device)
{
	return (device->type() == FD1793 || device->type() == FD1794 || device->type() == FD1797 ||
			device->type() == FD1763 || device->type() == FD1764 || device->type() == FD1767 ||
			device->type() == WD1770 || device->type() == WD1772 || device->type() == WD1773 ||
			device->type() == WD2793 || device->type() == WD2797 ||
			device->type() == MB8877);
}

static int wd17xx_is_sd_only(device_t *device)
{
	return (device->type() == FD1771 || device->type() == FD1792 || device->type() == FD1794 || device->type() == FD1762 || device->type() == FD1764);
}

static int wd17xx_has_side_select(device_t *device)
{
	return (device->type() == FD1795 || device->type() == FD1797 ||
			device->type() == FD1765 || device->type() == FD1767 ||
			device->type() == WD2795 || device->type() == WD2797);
}

int wd1770_device::wd17xx_dden()
{
	if (!m_in_dden_func.isnull())
		return m_in_dden_func();
	else
		return m_dden;
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/* clear a data request */
void wd1770_device::wd17xx_clear_drq()
{
	m_status &= ~STA_2_DRQ;

	m_drq = CLEAR_LINE;
	m_out_drq_func(m_drq);
}

/* set data request */
void wd1770_device::wd17xx_set_drq()
{
	if (m_status & STA_2_DRQ)
		m_status |= STA_2_LOST_DAT;

	m_status |= STA_2_DRQ;

	m_drq = ASSERT_LINE;
	m_out_drq_func(m_drq);
}

/* clear interrupt request */
void wd1770_device::wd17xx_clear_intrq()
{
	m_intrq = CLEAR_LINE;
	m_out_intrq_func(m_intrq);
}

/* set interrupt request */
void wd1770_device::wd17xx_set_intrq()
{
	m_status &= ~STA_2_BUSY;
	m_status &= ~STA_2_DRQ;

	m_intrq = ASSERT_LINE;
	m_out_intrq_func(m_intrq);
}

/* set intrq after delay */
TIMER_CALLBACK_MEMBER( wd1770_device::wd17xx_command_callback )
{
	if (m_last_command_data != FDC_FORCE_INT)
	{
		wd17xx_set_intrq();
	}
}

/* write next byte to data register and set drq */
TIMER_CALLBACK_MEMBER( wd1770_device::wd17xx_data_callback )
{
	/* check if this is a write command */
	if( (m_command_type == TYPE_II && m_command == FDC_WRITE_SEC) ||
			(m_command_type == TYPE_III && m_command == FDC_WRITE_TRK) )
	{
		/* we are ready for new data */
		wd17xx_set_drq();

		return;
	}

	/* any bytes remaining? */
	if (m_data_count >= 1)
	{
		/* yes */
		m_data = m_buffer[m_data_offset++];

		if (VERBOSE_DATA)
			logerror("wd17xx_data_callback: $%02X (data_count %d)\n", m_data, m_data_count);

		wd17xx_set_drq();

		/* any bytes remaining? */
		if (--m_data_count < 1)
		{
			/* no */
			m_data_offset = 0;

			/* clear ddam type */
			m_status &=~STA_2_REC_TYPE;

			/* read a sector with ddam set? */
			if (m_command_type == TYPE_II && m_ddam != 0)
			{
				/* set it */
				m_status |= STA_2_REC_TYPE;
			}

			/* check if we should handle the next sector for a multi record read */
			if (m_command_type == TYPE_II && m_command == FDC_READ_SEC && (m_read_cmd & 0x10))
			{
				if (VERBOSE)
					logerror("wd17xx_data_callback: multi sector read\n");

				if (m_sector == 0xff)
					m_sector = 0x01;
				else
					m_sector++;

				wd17xx_timed_read_sector_request();
			}
			else
			{
				/* Delay the INTRQ 3 byte times because we need to read two CRC bytes and
				   compare them with a calculated CRC */
				wd17xx_complete_command(DELAY_DATADONE);

				if (VERBOSE)
					logerror("wd17xx_data_callback: data read completed\n");
			}
		}
		else
		{
			/* requeue us for more data */
			m_timer_data->adjust(attotime::from_usec(wd17xx_dden() ? 128 : 32));
		}
	}
	else
	{
		logerror("wd17xx_data_callback: (no new data) $%02X (data_count %d)\n", m_data, m_data_count);
	}
}


void wd1770_device::wd17xx_set_busy(const attotime &duration)
{
	m_status |= STA_1_BUSY;

	m_timer_cmd->adjust(duration);
}


/* BUSY COUNT DOESN'T WORK PROPERLY! */

void wd1770_device::wd17xx_command_restore()
{
	UINT8 step_counter;

	if (m_drive == NULL)
		return;

	step_counter = 255;

#if 0
	m_status |= STA_1_BUSY;
#endif

	/* setup step direction */
	m_direction = -1;

	m_command_type = TYPE_I;

	/* reset busy count */
	m_busy_count = 0;

	if (1) // image_slotexists(m_drive) : FIXME
	{
		/* keep stepping until track 0 is received or 255 steps have been done */
		while (m_drive->floppy_tk00_r() && (step_counter != 0))
		{
			/* update time to simulate seek time busy signal */
			m_busy_count++;
			m_drive->floppy_drive_seek(m_direction);
			step_counter--;
		}
	}

	/* update track reg */
	m_track = 0;
#if 0
	/* simulate seek time busy signal */
	m_busy_count = 0;  //m_busy_count * ((m_data & FDC_STEP_RATE) + 1);

	/* when command completes set irq */
	wd17xx_set_intrq();
#endif
	wd17xx_set_busy(attotime::from_usec(100));
}

/*
    Write an entire track. Formats which do not define a write_track
    function pointer will cause a silent return.
    What is written to the image depends on the selected format. Sector
    dumps have to extract the sectors in the track, while track dumps
    may directly write the bytes.
    (The if-part below may thus be removed.)
*/
void wd1770_device::write_track()
{
	floppy_image_legacy *floppy;
#if 0
	int i;
	for (i=0;i+4<m_data_offset;)
	{
		if (m_buffer[i]==0xfe)
		{
			/* got address mark */
			int track   = m_buffer[i+1];
			int side    = m_buffer[i+2];
			int sector  = m_buffer[i+3];
			//int len     = m_buffer[i+4];
			int filler  = 0xe5; /* IBM and Thomson */
			int density = m_density;
			m_drive->floppy_drive_format_sector(side,sector,track,
						m_hd,sector,density?1:0,filler);
			i += 128; /* at least... */
		}
		else
			i++;
	}
#endif

	/* Get the size in bytes of the current track. For real hardware this
	may vary per system in small degree, and there even for each track
	and head, so we should not assume a fixed value here.
	As we are using a buffered track writing, we have to find out how long
	the track will become. The only object which can tell us is the
	selected format.
	*/
	m_data_count = 0;
	floppy = m_drive->flopimg_get_image();
	if (floppy != NULL)
		m_data_count = floppy_get_track_size(floppy, m_hd, m_track);

		if (m_data_count==0)
		{
			if (wd17xx_is_sd_only(this))
				m_data_count = TRKSIZE_SD;
			else
				m_data_count = wd17xx_dden() ? TRKSIZE_SD : TRKSIZE_DD;
		}

	m_drive->floppy_drive_write_track_data_info_buffer( m_hd, (char *)m_buffer, &(m_data_count) );

	m_data_offset = 0;

	wd17xx_set_drq();
	m_status |= STA_2_BUSY;
	m_busy_count = 0;
}

/*
    Read an entire track. It is up to the format to deliver the data. Sector
    dumps may be required to fantasize the missing track bytes, while track
    dumps can directly deliver them.
    (The if-part below may thus be removed.)
*/
void wd1770_device::read_track()
{
	floppy_image_legacy *floppy;
#if 0
	UINT8 *psrc;        /* pointer to track format structure */
	UINT8 *pdst;        /* pointer to track buffer */
	int cnt;            /* number of bytes to fill in */
	UINT16 crc;         /* id or data CRC */
	UINT8 d;            /* data */
	UINT8 t = m_track; /* track of DAM */
	UINT8 h = m_head;  /* head of DAM */
	UINT8 s = m_sector_dam;        /* sector of DAM */
	UINT16 l = m_sector_length;    /* sector length of DAM */
	int i;

	for (i = 0; i < m_sec_per_track; i++)
	{
		m_dam_list[i][0] = t;
		m_dam_list[i][1] = h;
		m_dam_list[i][2] = i;
		m_dam_list[i][3] = l >> 8;
	}

	pdst = m_buffer;

	if (m_density)
	{
		psrc = track_DD[0];    /* double density track format */
		cnt = TRKSIZE_DD;
	}
	else
	{
		psrc = track_SD[0];    /* single density track format */
		cnt = TRKSIZE_SD;
	}

	while (cnt > 0)
	{
		if (psrc[0] == 0)      /* no more track format info ? */
		{
			if (m_dam_cnt < m_sec_per_track) /* but more DAM info ? */
			{
				if (m_density)/* DD track ? */
					psrc = track_DD[3];
				else
					psrc = track_SD[3];
			}
		}

		if (psrc[0] != 0)      /* more track format info ? */
		{
			cnt -= psrc[0];    /* subtract size */
			d = psrc[1];

			if (d == 0xf5)     /* clear CRC ? */
			{
				crc = 0xffff;
				d = 0xa1;      /* store A1 */
			}

			for (i = 0; i < *psrc; i++)
				*pdst++ = d;   /* fill data */

			if (d == 0xf7)     /* store CRC ? */
			{
				pdst--;        /* go back one byte */
				*pdst++ = crc & 255;    /* put CRC low */
				*pdst++ = crc / 256;    /* put CRC high */
				cnt -= 1;      /* count one more byte */
			}
			else if (d == 0xfe)/* address mark ? */
			{
				crc = 0xffff;   /* reset CRC */
			}
			else if (d == 0x80)/* sector ID ? */
			{
				pdst--;        /* go back one byte */
				t = *pdst++ = m_dam_list[m_dam_cnt][0]; /* track number */
				h = *pdst++ = m_dam_list[m_dam_cnt][1]; /* head number */
				s = *pdst++ = m_dam_list[m_dam_cnt][2]; /* sector number */
				l = *pdst++ = m_dam_list[m_dam_cnt][3]; /* sector length code */
				m_dam_cnt++;
				crc = ccitt_crc16_one(crc, t);  /* build CRC */
				crc = ccitt_crc16_one(crc, h);  /* build CRC */
				crc = ccitt_crc16_one(crc, s);  /* build CRC */
				crc = ccitt_crc16_one(crc, l);  /* build CRC */
				l = (l == 0) ? 128 : l << 8;
			}
			else if (d == 0xfb)// data address mark ?
			{
				crc = 0xffff;   // reset CRC
			}
			else if (d == 0x81)// sector DATA ?
			{
				pdst--;        /* go back one byte */
				if (seek(w, t, h, s) == 0)
				{
					if (mame_fread(m_image_file, pdst, l) != l)
					{
						m_status = STA_2_CRC_ERR;
						return;
					}
				}
				else
				{
					m_status = STA_2_REC_N_FND;
					return;
				}
				for (i = 0; i < l; i++) // build CRC of all data
					crc = ccitt_crc16_one(crc, *pdst++);
				cnt -= l;
			}
			psrc += 2;
		}
		else
		{
			*pdst++ = 0xff;    /* fill track */
			cnt--;             /* until end */
		}
	}
#endif
	/* Determine the track size. We cannot allow different sizes in this
	design (see above, write_track). */
	m_data_count = 0;
	floppy = m_drive->flopimg_get_image();
	if (floppy != NULL)
		m_data_count = floppy_get_track_size(floppy, m_hd, m_track);

		if (m_data_count==0)
		{
			if (wd17xx_is_sd_only(this))
				m_data_count = TRKSIZE_SD;
			else
				m_data_count = wd17xx_dden() ? TRKSIZE_SD : TRKSIZE_DD;
		}

	m_drive->floppy_drive_read_track_data_info_buffer( m_hd, (char *)m_buffer, &(m_data_count) );

	m_data_offset = 0;

	wd17xx_set_drq();
	m_status |= STA_2_BUSY;
	m_busy_count = 0;
}


/* read the next data address mark */
void wd1770_device::wd17xx_read_id()
{
	chrn_id id;
	m_status &= ~(STA_2_CRC_ERR | STA_2_REC_N_FND);

	/* get next id from disc */
	if (m_drive->floppy_drive_get_next_id(m_hd, &id))
	{
		UINT16 crc = 0xffff;

		m_data_offset = 0;
		m_data_count = 6;

		/* for MFM */
		/* crc includes 3x0x0a1, and 1x0x0fe (id mark) */
		crc = ccitt_crc16_one(crc,0x0a1);
		crc = ccitt_crc16_one(crc,0x0a1);
		crc = ccitt_crc16_one(crc,0x0a1);
		crc = ccitt_crc16_one(crc,0x0fe);

		m_buffer[0] = id.C;
		m_buffer[1] = id.H;
		m_buffer[2] = id.R;
		m_buffer[3] = id.N;
		crc = ccitt_crc16_one(crc, m_buffer[0]);
		crc = ccitt_crc16_one(crc, m_buffer[1]);
		crc = ccitt_crc16_one(crc, m_buffer[2]);
		crc = ccitt_crc16_one(crc, m_buffer[3]);
		/* crc is stored hi-byte followed by lo-byte */
		m_buffer[4] = crc>>8;
		m_buffer[5] = crc & 255;

		m_sector = id.C;

		if (VERBOSE)
			logerror("wd17xx_read_id: read id succeeded.\n");

		wd17xx_timed_data_request();
	}
	else
	{
		/* record not found */
		m_status |= STA_2_REC_N_FND;
		//m_sector = m_track;
		if (VERBOSE)
			logerror("wd17xx_read_id: read id failed\n");

		wd17xx_complete_command(DELAY_ERROR);
	}
}



void wd1770_device::index_pulse_callback(device_t *img, int state)
{
	if (img != m_drive)
		return;

	m_idx = state;

	if (!state && m_idx && BIT(m_interrupt, 2))
		wd17xx_set_intrq();

	if (m_hld_count)
		m_hld_count--;
}



int wd1770_device::wd17xx_locate_sector()
{
	UINT8 revolution_count;
	chrn_id id;
	revolution_count = 0;

	m_status &= ~STA_2_REC_N_FND;

	while (revolution_count!=4)
	{
		if (m_drive->floppy_drive_get_next_id(m_hd, &id))
		{
			/* compare track */
			if (id.C == m_track)
			{
				/* compare head, if we were asked to */
				if (!wd17xx_has_side_select(this) || (id.H == m_head) || (m_head == (UINT8) ~0))
				{
					/* compare id */
					if (id.R == m_sector)
					{
						m_sector_length = 1<<(id.N+7);
						m_sector_data_id = id.data_id;
						/* get ddam status */
						m_ddam = id.flags & ID_FLAG_DELETED_DATA;
						/* got record type here */
						if (VERBOSE)
							logerror("sector found! C:$%02x H:$%02x R:$%02x N:$%02x%s\n", id.C, id.H, id.R, id.N, m_ddam ? " DDAM" : "");
						return 1;
					}
				}
			}
		}

			/* index set? */
		if (m_drive->floppy_drive_get_flag_state(FLOPPY_DRIVE_INDEX))
		{
			/* update revolution count */
			revolution_count++;
		}
	}
	return 0;
}


int wd1770_device::wd17xx_find_sector()
{
	if ( wd17xx_locate_sector() )
	{
		return 1;
	}

	/* record not found */
	m_status |= STA_2_REC_N_FND;

	if (VERBOSE)
		logerror("track %d sector %d not found!\n", m_track, m_sector);

	wd17xx_complete_command(DELAY_ERROR);

	return 0;
}

void wd1770_device::wd17xx_side_compare(UINT8 command)
{
	if (wd17xx_has_side_select(this))
		set_side((command & FDC_SIDE_CMP_T) ? 1 : 0);

	if (command & FDC_SIDE_CMP_T)
		m_head = (command & FDC_SIDE_CMP_S) ? 1 : 0;
	else
		m_head = ~0;
}

/* read a sector */
void wd1770_device::wd17xx_read_sector()
{
	m_data_offset = 0;

	/* side compare? */
	wd17xx_side_compare(m_read_cmd);

	if (wd17xx_find_sector())
	{
		m_data_count = m_sector_length;

		/* read data */
		m_drive->floppy_drive_read_sector_data(m_hd, m_sector_data_id, (char *)m_buffer, m_sector_length);

		wd17xx_timed_data_request();

		m_status |= STA_2_BUSY;
		m_busy_count = 0;
	}
}


/* called on error, or when command is actually completed */
/* KT - I have used a timer for systems that use interrupt driven transfers.
A interrupt occurs after the last byte has been read. If it occurs at the time
when the last byte has been read it causes problems - same byte read again
or bytes missed */
/* TJL - I have add a parameter to allow the emulation to specify the delay
*/
void wd1770_device::wd17xx_complete_command(int delay)
{
	m_data_count = 0;

	m_hld_count = 2;

	/* set new timer */
	int usecs = wd17xx_dden() ? 32 : 16;
	m_timer_cmd->adjust(attotime::from_usec(usecs));

	/* Kill onshot read/write sector timers */
	m_timer_rs->adjust(attotime::never);
	m_timer_ws->adjust(attotime::never);
}



void wd1770_device::wd17xx_write_sector()
{
	/* at this point, the disc is write enabled, and data
	 * has been transfered into our buffer - now write it to
	 * the disc image or to the real disc
	 */

	/* side compare? */
	wd17xx_side_compare(m_write_cmd);

	/* find sector */
	if (wd17xx_find_sector())
	{
		m_data_count = m_sector_length;

		/* write data */
		m_drive->floppy_drive_write_sector_data(m_hd, m_sector_data_id, (char *)m_buffer, m_sector_length, m_write_cmd & 0x01);
	}
}



/* verify the seek operation by looking for a id that has a matching track value */
void wd1770_device::wd17xx_verify_seek()
{
	UINT8 revolution_count;
	chrn_id id;
	revolution_count = 0;

	if (VERBOSE)
		logerror("doing seek verify\n");

	m_status &= ~STA_1_SEEK_ERR;

	/* must be found within 5 revolutions otherwise error */
	while (revolution_count!=5)
	{
		if (m_drive->floppy_drive_get_next_id( m_hd, &id))
		{
			/* compare track */
			if (id.C == m_track)
			{
				if (VERBOSE)
					logerror("seek verify succeeded!\n");
				return;
			}
		}

			/* index set? */
		if (m_drive->floppy_drive_get_flag_state(FLOPPY_DRIVE_INDEX))
		{
			/* update revolution count */
			revolution_count++;
		}
	}

	m_status |= STA_1_SEEK_ERR;

	if (VERBOSE)
		logerror("failed seek verify!\n");
}



/* callback to initiate read sector */
TIMER_CALLBACK_MEMBER( wd1770_device::wd17xx_read_sector_callback )
{
	/* ok, start that read! */

	if (VERBOSE)
		logerror("wd179x: Read Sector callback.\n");

	if (!m_drive->floppy_drive_get_flag_state(FLOPPY_DRIVE_READY))
		wd17xx_complete_command(DELAY_NOTREADY);
	else
		wd17xx_read_sector();
}



/* callback to initiate write sector */
TIMER_CALLBACK_MEMBER( wd1770_device::wd17xx_write_sector_callback )
{
	/* ok, start that write! */

	if (VERBOSE)
		logerror("wd179x: Write Sector callback.\n");

	if (!m_drive->floppy_drive_get_flag_state(FLOPPY_DRIVE_READY))
		wd17xx_complete_command(DELAY_NOTREADY);
	else
	{
		/* drive write protected? */
		if (m_drive->floppy_wpt_r() == CLEAR_LINE)
		{
			m_status |= STA_2_WRITE_PRO;

			wd17xx_complete_command(DELAY_ERROR);
		}
		else
		{
			/* side compare? */
			wd17xx_side_compare(m_write_cmd);

			/* attempt to find it first before getting data from cpu */
			if (wd17xx_find_sector())
			{
				/* request data */
				m_data_offset = 0;
				m_data_count = m_sector_length;

				wd17xx_set_drq();

				m_status |= STA_2_BUSY;
				m_busy_count = 0;
			}
		}
	}
}



/* setup a timed data request - data request will be triggered in a few usecs time */
void wd1770_device::wd17xx_timed_data_request()
{
	/* set new timer */
	m_timer_data->adjust(attotime::from_usec(wd17xx_dden() ? 128 : 32));
}



/* setup a timed read sector - read sector will be triggered in a few usecs time */
void wd1770_device::wd17xx_timed_read_sector_request()
{
	/* set new timer */
	m_timer_rs->adjust(attotime::from_usec(m_pause_time));
}



/* setup a timed write sector - write sector will be triggered in a few usecs time */
void wd1770_device::wd17xx_timed_write_sector_request()
{
	/* set new timer */
	m_timer_ws->adjust(attotime::from_usec(m_pause_time));
}


/***************************************************************************
    INTERFACE
***************************************************************************/

/* use this to determine which drive is controlled by WD */
void wd1770_device::set_drive(UINT8 drive)
{
	if (VERBOSE)
		logerror("wd17xx_set_drive: $%02x\n", drive);

	if (m_floppy_drive_tags[drive] != NULL)
	{
		m_drive = siblingdevice<legacy_floppy_image_device>(m_floppy_drive_tags[drive]);
	}
}

void wd1770_device::set_side(UINT8 head)
{
	if (VERBOSE)
	{
		if (head != m_hd)
			logerror("wd17xx_set_side: $%02x\n", head);
	}

	m_hd = head;
}

void wd1770_device::set_pause_time(int usec)
{
	m_pause_time = usec;
}


/***************************************************************************
    DEVICE HANDLERS
***************************************************************************/

/* master reset */
WRITE_LINE_MEMBER( wd1770_device::mr_w )
{
	/* reset device when going from high to low */
	if (m_mr && state == CLEAR_LINE)
	{
		m_command = 0x03;
		m_status &= ~STA_1_NOT_READY; /* ? */
	}

	/* execute restore command when going from low to high */
	if (m_mr == CLEAR_LINE && state)
	{
		wd17xx_command_restore();
		m_sector = 0x01;
	}

	m_mr = state;
}

/* ready and enable precomp (1773 only) */
WRITE_LINE_MEMBER( wd1770_device::rdy_w )
{
	m_rdy = state;
}

/* motor on, 1770 and 1772 only */
READ_LINE_MEMBER( wd1770_device::mo_r )
{
	return m_mo;
}

/* track zero */
WRITE_LINE_MEMBER( wd1770_device::tr00_w )
{
	m_tr00 = state;
}

/* index pulse */
WRITE_LINE_MEMBER( wd1770_device::idx_w )
{
	m_idx = state;

	if (!state && m_idx && BIT(m_interrupt, 2))
		wd17xx_set_intrq();
}

/* write protect status */
WRITE_LINE_MEMBER( wd1770_device::wprt_w )
{
	m_wprt = state;
}

/* double density enable */
WRITE_LINE_MEMBER( wd1770_device::dden_w )
{
	/* not supported on FD1771, FD1792, FD1794, FD1762 and FD1764 */
	if (wd17xx_is_sd_only(this))
		fatalerror("wd17xx_dden_w: double density input not supported on this model!\n");
	else if (!m_in_dden_func.isnull())
		logerror("wd17xx_dden_w: write has no effect because a read handler is already defined!\n");
	else
		m_dden = state;
}

/* data request */
READ_LINE_MEMBER( wd1770_device::drq_r )
{
	return m_drq;
}

/* interrupt request */
READ_LINE_MEMBER( wd1770_device::intrq_r )
{
	return m_intrq;
}

/* read the FDC status register. This clears IRQ line too */
READ8_MEMBER( wd1770_device::status_r )
{
	int result;

	if (!BIT(m_interrupt, 3))
	{
		wd17xx_clear_intrq();
	}

	/* bit 7, 'not ready' or 'motor on' */
	if (type() == WD1770 || type() == WD1772)
	{
		m_status &= ~STA_1_MOTOR_ON;
		m_status |= m_mo << 7;
	}
	else
	{
		m_status &= ~STA_1_NOT_READY;
		if (!m_drive->floppy_drive_get_flag_state(FLOPPY_DRIVE_READY))
			m_status |= STA_1_NOT_READY;
	}

	result = m_status;

	/* type 1 command or force int command? */
	if ((m_command_type==TYPE_I) || (m_command_type==TYPE_IV && ! m_was_busy))
	{
		result &= ~(STA_1_IPL | STA_1_TRACK0);

		/* bit 1, index pulse */
		result |= m_idx << 1;

		/* bit 2, track 0 state, inverted */
		result |= !m_drive->floppy_tk00_r() << 2;

		if (m_command_type==TYPE_I)
		{
			if (m_hld_count)
				m_status |= STA_1_HD_LOADED;
			else
				m_status &= ~ STA_1_HD_LOADED;
		}

		/* bit 6, write protect, inverted */
		result |= !m_drive->floppy_wpt_r() << 6;
	}

	/* eventually set data request bit */
//  m_status |= m_status_drq;

	if (VERBOSE)
	{
		if (m_data_count < 4)
			logerror("%s: wd17xx_status_r: $%02X (data_count %d)\n", machine().describe_context(), result, m_data_count);
	}

	return result ^ (wd17xx_has_dal(this) ? 0 : 0xff);
}

/* read the FDC track register */
READ8_MEMBER( wd1770_device::track_r )
{
	if (VERBOSE)
		logerror("%s: wd17xx_track_r: $%02X\n", machine().describe_context(), m_track);

	return m_track ^ (wd17xx_has_dal(this) ? 0 : 0xff);
}

/* read the FDC sector register */
READ8_MEMBER( wd1770_device::sector_r )
{
	if (VERBOSE)
		logerror("%s: wd17xx_sector_r: $%02X\n", machine().describe_context(), m_sector);

	return m_sector ^ (wd17xx_has_dal(this) ? 0 : 0xff);
}

/* read the FDC data register */
READ8_MEMBER( wd1770_device::data_r )
{
	if (VERBOSE_DATA)
		logerror("%s: wd17xx_data_r: %02x\n", machine().describe_context(), m_data);

	/* clear data request */
	wd17xx_clear_drq();

	return m_data ^ (wd17xx_has_dal(this) ? 0 : 0xff);
}

/* write the FDC command register */
WRITE8_MEMBER( wd1770_device::command_w )
{
	if (!wd17xx_has_dal(this)) data ^= 0xff;

	m_last_command_data = data;

	/* only the WD1770 and WD1772 have a 'motor on' line */
	if (type() == WD1770 || type() == WD1772)
	{
		m_mo = ASSERT_LINE;
		m_drive->floppy_mon_w(CLEAR_LINE);
	}

	m_drive->floppy_drive_set_ready_state(1,0);

	if (!BIT(m_interrupt, 3))
	{
		wd17xx_clear_intrq();
	}

	/* clear write protected. On read sector, read track and read dam, write protected bit is clear */
	m_status &= ~((1<<6) | (1<<5) | (1<<4));

	if ((data & ~FDC_MASK_TYPE_IV) == FDC_FORCE_INT)
	{
		if (VERBOSE)
			logerror("%s: wd17xx_command_w $%02X FORCE_INT (data_count %d)\n", machine().describe_context(), data, m_data_count);

		m_data_count = 0;
		m_data_offset = 0;
		m_was_busy = m_status & STA_2_BUSY;
		m_status &= ~STA_2_BUSY;

		wd17xx_clear_drq();

		if (!BIT(m_interrupt, 3) && BIT(data, 3))
		{
			/* set immediate interrupt */
			wd17xx_set_intrq();
		}

		if (BIT(m_interrupt, 3))
		{
			if (data == FDC_FORCE_INT)
			{
				/* clear immediate interrupt */
				m_interrupt = data & 0x0f;
			}
			else
			{
				/* keep immediate interrupt */
				m_interrupt = 0x08 | (data & 0x07);
			}
		}
		else
		{
			m_interrupt = data & 0x0f;
		}

		/* terminate command */
		wd17xx_complete_command(DELAY_ERROR);

		m_busy_count = 0;
		m_command_type = TYPE_IV;
		return;
	}

	if (data & 0x80)
	{
		/*m_status_ipl = 0;*/

		if ((data & ~FDC_MASK_TYPE_II) == FDC_READ_SEC)
		{
			if (VERBOSE)
			{
				logerror("%s: wd17xx_command_w $%02X READ_SEC (", machine().describe_context(), data);
				logerror("cmd=%02X, trk=%02X, sec=%02X, dat=%02X)\n",m_command,m_track,m_sector,m_data);
			}
			m_read_cmd = data;
			m_command = data & ~FDC_MASK_TYPE_II;
			m_command_type = TYPE_II;
			m_status &= ~STA_2_LOST_DAT;
			m_status |= STA_2_BUSY;
			wd17xx_clear_drq();

			wd17xx_timed_read_sector_request();

			return;
		}

		if ((data & ~FDC_MASK_TYPE_II) == FDC_WRITE_SEC)
		{
			if (VERBOSE)
			{
				logerror("%s: wd17xx_command_w $%02X WRITE_SEC (", machine().describe_context(), data);
				logerror("cmd=%02X, trk=%02X, sec=%02X, dat=%02X)\n",m_command,m_track,m_sector,m_data);
			}

			m_write_cmd = data;
			m_command = data & ~FDC_MASK_TYPE_II;
			m_command_type = TYPE_II;
			m_status &= ~STA_2_LOST_DAT;
			m_status |= STA_2_BUSY;
			wd17xx_clear_drq();

			wd17xx_timed_write_sector_request();

			return;
		}

		if ((data & ~FDC_MASK_TYPE_III) == FDC_READ_TRK)
		{
			if (VERBOSE)
				logerror("%s: wd17xx_command_w $%02X READ_TRK\n", machine().describe_context(), data);

			m_command = data & ~FDC_MASK_TYPE_III;
			m_command_type = TYPE_III;
			m_status &= ~STA_2_LOST_DAT;
			wd17xx_clear_drq();
#if 1
//          m_status = seek(w, m_track, m_head, m_sector);
			if (m_status == 0)
				read_track();
#endif
			return;
		}

		if ((data & ~FDC_MASK_TYPE_III) == FDC_WRITE_TRK)
		{
			if (VERBOSE)
				logerror("%s: wd17xx_command_w $%02X WRITE_TRK\n", machine().describe_context(), data);

			m_command_type = TYPE_III;
			m_status &= ~STA_2_LOST_DAT;
			wd17xx_clear_drq();

			if (!m_drive->floppy_drive_get_flag_state(FLOPPY_DRIVE_READY))
			{
				wd17xx_complete_command(DELAY_NOTREADY);
			}
			else
			{
				/* drive write protected? */
				if (m_drive->floppy_wpt_r() == CLEAR_LINE)
				{
				/* yes */
					m_status |= STA_2_WRITE_PRO;
				/* quit command */
					wd17xx_complete_command(DELAY_ERROR);
				}
				else
				{
				m_command = data & ~FDC_MASK_TYPE_III;
				m_data_offset = 0;
				if (wd17xx_is_sd_only(this))
					m_data_count = TRKSIZE_SD;
				else
					m_data_count = wd17xx_dden() ? TRKSIZE_SD : TRKSIZE_DD;

				wd17xx_set_drq();
				m_status |= STA_2_BUSY;
				m_busy_count = 0;
				}
			}
			return;
		}

		if ((data & ~FDC_MASK_TYPE_III) == FDC_READ_DAM)
		{
			if (VERBOSE)
				logerror("%s: wd17xx_command_w $%02X READ_DAM\n", machine().describe_context(), data);

			m_command_type = TYPE_III;
			m_command = data & ~FDC_MASK_TYPE_III;
			m_status &= ~STA_2_LOST_DAT;
			m_status |= STA_2_BUSY;

			wd17xx_clear_drq();

			if (m_drive->floppy_drive_get_flag_state(FLOPPY_DRIVE_READY))
				wd17xx_read_id();
			else
				wd17xx_complete_command(DELAY_NOTREADY);

			return;
		}

		if (VERBOSE)
			logerror("%s: wd17xx_command_w $%02X unknown\n", machine().describe_context(), data);

		return;
	}

	m_status |= STA_1_BUSY;

	/* clear CRC error */
	m_status &=~STA_1_CRC_ERR;

	if ((data & ~FDC_MASK_TYPE_I) == FDC_RESTORE)
	{
		if (VERBOSE)
			logerror("%s: wd17xx_command_w $%02X RESTORE\n", machine().describe_context(), data);

		wd17xx_command_restore();
	}

	if ((data & ~FDC_MASK_TYPE_I) == FDC_SEEK)
	{
		UINT8 newtrack;

		if (VERBOSE)
			logerror("old track: $%02x new track: $%02x\n", m_track, m_data);
		m_command_type = TYPE_I;

		/* setup step direction */
		if (m_track < m_data)
		{
			if (VERBOSE)
				logerror("direction: +1\n");

			m_direction = 1;
		}
		else if (m_track > m_data)
		{
			if (VERBOSE)
				logerror("direction: -1\n");

			m_direction = -1;
		}

		newtrack = m_data;
		if (VERBOSE)
			logerror("%s: wd17xx_command_w $%02X SEEK (data_reg is $%02X)\n", machine().describe_context(), data, newtrack);

		/* reset busy count */
		m_busy_count = 0;

		/* keep stepping until reached track programmed */
		while (m_track != newtrack)
		{
			/* update time to simulate seek time busy signal */
			m_busy_count++;

			/* update track reg */
			m_track += m_direction;

			m_drive->floppy_drive_seek(m_direction);
		}

		/* simulate seek time busy signal */
		m_busy_count = 0;  //m_busy_count * ((data & FDC_STEP_RATE) + 1);
#if 0
		wd17xx_set_intrq();
#endif
		wd17xx_set_busy(attotime::from_usec(100));

	}

	if ((data & ~(FDC_STEP_UPDATE | FDC_MASK_TYPE_I)) == FDC_STEP)
	{
		if (VERBOSE)
			logerror("%s: wd17xx_command_w $%02X STEP dir %+d\n", machine().describe_context(), data, m_direction);

		m_command_type = TYPE_I;
		/* if it is a real floppy, issue a step command */
		/* simulate seek time busy signal */
		m_busy_count = 0;  //((data & FDC_STEP_RATE) + 1);

		m_drive->floppy_drive_seek(m_direction);

		if (data & FDC_STEP_UPDATE)
			m_track += m_direction;

#if 0
		wd17xx_set_intrq();
#endif
		wd17xx_set_busy(attotime::from_usec(100));


	}

	if ((data & ~(FDC_STEP_UPDATE | FDC_MASK_TYPE_I)) == FDC_STEP_IN)
	{
		if (VERBOSE)
			logerror("%s: wd17xx_command_w $%02X STEP_IN\n", machine().describe_context(), data);

		m_command_type = TYPE_I;
		m_direction = +1;
		/* simulate seek time busy signal */
		m_busy_count = 0;  //((data & FDC_STEP_RATE) + 1);

		m_drive->floppy_drive_seek(m_direction);

		if (data & FDC_STEP_UPDATE)
			m_track += m_direction;
#if 0
		wd17xx_set_intrq();
#endif
		wd17xx_set_busy(attotime::from_usec(100));

	}

	if ((data & ~(FDC_STEP_UPDATE | FDC_MASK_TYPE_I)) == FDC_STEP_OUT)
	{
		if (VERBOSE)
			logerror("%s: wd17xx_command_w $%02X STEP_OUT\n", machine().describe_context(), data);

		m_command_type = TYPE_I;
		m_direction = -1;
		/* simulate seek time busy signal */
		m_busy_count = 0;  //((data & FDC_STEP_RATE) + 1);

		/* for now only allows a single drive to be selected */
		m_drive->floppy_drive_seek(m_direction);

		if (data & FDC_STEP_UPDATE)
			m_track += m_direction;

#if 0
		wd17xx_set_intrq();
#endif
		wd17xx_set_busy(attotime::from_usec(100));
	}

	if (m_command_type == TYPE_I)
	{
		/* 0 is enable spin up sequence, 1 is disable spin up sequence */
		if ((data & FDC_STEP_HDLOAD)==0)
		{
			m_status |= STA_1_HD_LOADED;
			m_hld_count = 2;
		}
		else
			m_status &= ~STA_1_HD_LOADED;

		if (data & FDC_STEP_VERIFY)
		{
			/* verify seek */
			wd17xx_verify_seek();
		}
	}
}

/* write the FDC track register */
WRITE8_MEMBER( wd1770_device::track_w )
{
	if (!wd17xx_has_dal(this)) data ^= 0xff;

	m_track = data;

	if (VERBOSE)
		logerror("%s: wd17xx_track_w $%02X\n", machine().describe_context(), data);
}

/* write the FDC sector register */
WRITE8_MEMBER( wd1770_device::sector_w )
{
	if (!wd17xx_has_dal(this)) data ^= 0xff;

	m_sector = data;

	if (VERBOSE)
		logerror("%s: wd17xx_sector_w $%02X\n", machine().describe_context(), data);
}

/* write the FDC data register */
WRITE8_MEMBER( wd1770_device::data_w )
{
	if (!wd17xx_has_dal(this)) data ^= 0xff;

	if (m_data_count > 0)
	{
		wd17xx_clear_drq();

		/* put byte into buffer */
		if (VERBOSE_DATA)
			logerror("wd17xx_info buffered data: $%02X at offset %d.\n", data, m_data_offset);

		m_buffer[m_data_offset++] = data;

				if (--m_data_count < 1)
				{
						if (m_command == FDC_WRITE_TRK)
								write_track();
						else
								wd17xx_write_sector();

						m_data_offset = 0;

						/* Check we should handle the next sector for a multi record write */
						if ( m_command_type == TYPE_II && m_command == FDC_WRITE_SEC && ( m_write_cmd & FDC_MULTI_REC ) )
						{
							m_sector++;
							if (wd17xx_locate_sector())
							{
								m_data_count = m_sector_length;

								m_status |= STA_2_BUSY;
								m_busy_count = 0;

								wd17xx_timed_data_request();
							}
						}
						else
						{
							wd17xx_complete_command(DELAY_DATADONE);
							if (VERBOSE)
								logerror("wd17xx_data_w(): multi data write completed\n");
						}
//                       wd17xx_complete_command( DELAY_DATADONE);
				}
				else
				{
						if (m_command == FDC_WRITE_TRK)
						{
								/* Process special data values according to WD17xx specification.
					Note that as CRC values take up two bytes which are written on
					every 0xf7 byte, this will cause the actual image to
					grow larger than what was written from the system. So we need
					to take the value of data_offset when writing the track.
								*/
								if (wd17xx_dden())
								{
										switch (data)
										{
										case 0xf5:
										case 0xf6:
												/* not allowed in FM. */
												/* Take back the last write. */
												m_data_offset--;
												break;
										case 0xf7:
												/* Take back the last write. */
												m_data_offset--;
												/* write two crc bytes */
												m_buffer[m_data_offset++] = (m_crc>>8)&0xff;
												m_buffer[m_data_offset++] = (m_crc&0xff);
												m_crc_active = FALSE;
												break;
										case 0xf8:
										case 0xf9:
										case 0xfa:
										case 0xfb:
										case 0xfe:
												/* Preset crc */
												m_crc = 0xffff;
						/* AM is included in the CRC */
						m_crc = ccitt_crc16_one(m_crc, data);
												m_crc_active = TRUE;
												break;
										case 0xfc:
												/* Write index mark. No effect here as we do not store clock patterns.
							Maybe later. */
												break;
										case 0xfd:
												/* Just write, don't use for CRC. */
												break;
										default:
												/* Byte already written. */
												if (m_crc_active)
														m_crc = ccitt_crc16_one(m_crc, data);
										}
								}
								else  /* MFM */
								{
										switch (data)
										{
										case 0xf5:
												/* Take back the last write. */
												m_data_offset--;
												/* Write a1 */
												m_buffer[m_data_offset++] = 0xa1;
												/* Preset CRC */
												m_crc = 0xffff;
												m_crc_active = TRUE;
												break;
										case 0xf6:
												/* Take back the last write. */
												m_data_offset--;
												/* Write c2 */
												m_buffer[m_data_offset++] = 0xc2;
												break;
										case 0xf7:
												/* Take back the last write. */
												m_data_offset--;
												/* write two crc bytes */
												m_buffer[m_data_offset++] = (m_crc>>8)&0xff;
												m_buffer[m_data_offset++] = (m_crc&0xff);
												m_crc_active = FALSE;
												break;
										case 0xf8:
										case 0xf9:
										case 0xfa:
										case 0xfb:
										case 0xfc:
										case 0xfd:
												/* Just write, don't use for CRC. */
												break;
										case 0xfe:
						/* AM is included in the CRC */
						if (m_crc_active)
							m_crc = ccitt_crc16_one(m_crc, data);
												break;
										default:
												/* Byte already written. */
												if (m_crc_active)
														m_crc = ccitt_crc16_one(m_crc, data);
										}
								}
						}
						/* yes... setup a timed data request */
						wd17xx_timed_data_request();
				}
	}
	else
	{
		if (VERBOSE)
			logerror("%s: wd17xx_data_w $%02X\n", machine().describe_context(), data);
	}
	m_data = data;
}

READ8_MEMBER( wd1770_device::read )
{
	UINT8 data = 0;

	switch (offset & 0x03)
	{
	case 0: data = status_r(space, 0); break;
	case 1: data = track_r(space, 0); break;
	case 2: data = sector_r(space, 0); break;
	case 3: data = data_r(space, 0); break;
	}

	return data;
}

WRITE8_MEMBER( wd1770_device::write )
{
	switch (offset & 0x03)
	{
	case 0: command_w(space, 0, data); break;
	case 1: track_w(space, 0, data);   break;
	case 2: sector_w(space, 0, data);  break;
	case 3: data_w(space, 0, data);    break;
	}
}


/***************************************************************************
    MAME DEVICE INTERFACE
***************************************************************************/

const device_type FD1771 = &device_creator<fd1771_device>;

fd1771_device::fd1771_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, FD1771, "FD1771", tag, owner, clock, "fd1771", __FILE__)
{
}


const device_type FD1781 = &device_creator<fd1781_device>;

fd1781_device::fd1781_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, FD1781, "FD1781", tag, owner, clock, "fd1781", __FILE__)
{
}


const device_type FD1791 = &device_creator<fd1791_device>;

fd1791_device::fd1791_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, FD1791, "FD1791", tag, owner, clock, "fd1791", __FILE__)
{
}


const device_type FD1792 = &device_creator<fd1792_device>;

fd1792_device::fd1792_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, FD1792, "FD1792", tag, owner, clock, "fd1792", __FILE__)
{
}


const device_type FD1793 = &device_creator<fd1793_device>;

fd1793_device::fd1793_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, FD1793, "FD1793", tag, owner, clock, "fd1793", __FILE__)
{
}


const device_type FD1794 = &device_creator<fd1794_device>;

fd1794_device::fd1794_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, FD1794, "FD1794", tag, owner, clock, "fd1794", __FILE__)
{
}


const device_type FD1795 = &device_creator<fd1795_device>;

fd1795_device::fd1795_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, FD1795, "FD1795", tag, owner, clock, "fd1795", __FILE__)
{
}


const device_type FD1797 = &device_creator<fd1797_device>;

fd1797_device::fd1797_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, FD1797, "FD1797", tag, owner, clock, "fd1797", __FILE__)
{
}


const device_type FD1761 = &device_creator<fd1761_device>;

fd1761_device::fd1761_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, FD1761, "FD1761", tag, owner, clock, "fd1761", __FILE__)
{
}


const device_type FD1762 = &device_creator<fd1762_device>;

fd1762_device::fd1762_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, FD1762, "FD1762", tag, owner, clock, "fd1762", __FILE__)
{
}


const device_type FD1763 = &device_creator<fd1763_device>;

fd1763_device::fd1763_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, FD1763, "FD1763", tag, owner, clock, "fd1763", __FILE__)
{
}


const device_type FD1764 = &device_creator<fd1764_device>;

fd1764_device::fd1764_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, FD1764, "FD1764", tag, owner, clock, "fd1764", __FILE__)
{
}


const device_type FD1765 = &device_creator<fd1765_device>;

fd1765_device::fd1765_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, FD1765, "FD1765", tag, owner, clock, "fd1765", __FILE__)
{
}


const device_type FD1767 = &device_creator<fd1767_device>;

fd1767_device::fd1767_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, FD1767, "FD1767", tag, owner, clock, "fd1767", __FILE__)
{
}


const device_type WD2791 = &device_creator<wd2791_device>;

wd2791_device::wd2791_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, WD2791, "WD2791", tag, owner, clock, "wd2791", __FILE__)
{
}


const device_type WD2793 = &device_creator<wd2793_device>;

wd2793_device::wd2793_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, WD2793, "WD2793", tag, owner, clock, "wd2793", __FILE__)
{
}


const device_type WD2795 = &device_creator<wd2795_device>;

wd2795_device::wd2795_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, WD2795, "WD2795", tag, owner, clock, "wd2795", __FILE__)
{
}


const device_type WD2797 = &device_creator<wd2797_device>;

wd2797_device::wd2797_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, WD2797, "WD2797_LEGACY", tag, owner, clock, "wd2797_l", __FILE__)
{
}


const device_type WD1770 = &device_creator<wd1770_device>;

wd1770_device::wd1770_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, WD1770, "WD1770_LEGACY", tag, owner, clock, "wd1770_l", __FILE__),
	m_out_intrq_func(*this),
	m_out_drq_func(*this),
	m_in_dden_func(*this)
{
	for (int i = 0; i < 4; i++)
		m_floppy_drive_tags[i] = NULL;
}

wd1770_device::wd1770_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_out_intrq_func(*this),
	m_out_drq_func(*this),
	m_in_dden_func(*this)
{
	for (int i = 0; i < 4; i++)
		m_floppy_drive_tags[i] = NULL;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wd1770_device::device_start()
{
	m_status = STA_1_TRACK0;
	m_pause_time = 1000;

	/* allocate timers */
	m_timer_cmd = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(wd1770_device::wd17xx_command_callback),this));
	m_timer_data = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(wd1770_device::wd17xx_data_callback),this));
	m_timer_rs = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(wd1770_device::wd17xx_read_sector_callback),this));
	m_timer_ws = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(wd1770_device::wd17xx_write_sector_callback),this));

	/* resolve callbacks */
	m_in_dden_func.resolve();
	m_out_intrq_func.resolve_safe();
	m_out_drq_func.resolve_safe();

	/* stepping rate depends on the clock */
	m_stepping_rate[0] = 6;
	m_stepping_rate[1] = 12;
	m_stepping_rate[2] = 20;
	m_stepping_rate[3] = 30;

}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

static void wd17xx_index_pulse_callback(device_t *controller, device_t *img, int state)
{
	wd1770_device *wd = downcast<wd1770_device *>(controller);
	wd->index_pulse_callback(img,state);
}

void wd1770_device::device_reset()
{
	/* set the default state of some input lines */
	m_mr = ASSERT_LINE;
	m_wprt = ASSERT_LINE;
	m_dden = ASSERT_LINE;

	for (int i = 0; i < 4; i++)
	{
		if (m_floppy_drive_tags[i])
		{
			legacy_floppy_image_device *img = siblingdevice<legacy_floppy_image_device>(m_floppy_drive_tags[i]);

			if (img)
			{
				img->floppy_drive_set_controller(this);
				img->floppy_drive_set_index_pulse_callback(wd17xx_index_pulse_callback);
				img->floppy_drive_set_rpm(300.);
			}
		}
	}

	set_drive(0);

	m_hd = 0;
	m_hld_count = 0;
	m_sector = 1;
	m_last_command_data = 0;
	m_interrupt = 0;
	m_data_count = 0;
	m_idx = 0;
	wd17xx_command_restore();
}


const device_type WD1772 = &device_creator<wd1772_device>;

wd1772_device::wd1772_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, WD1772, "WD1772_LEGACY", tag, owner, clock, "wd1772_l", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wd1772_device::device_start()
{
	wd1770_device::device_start();

	/* the 1772 has faster track stepping rates */
	m_stepping_rate[0] = 6;
	m_stepping_rate[1] = 12;
	m_stepping_rate[2] = 2;
	m_stepping_rate[3] = 3;
}


const device_type WD1773 = &device_creator<wd1773_device>;

wd1773_device::wd1773_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, WD1773, "WD1773_LEGACY", tag, owner, clock, "wd1773_l", __FILE__)
{
}


const device_type MB8866 = &device_creator<mb8866_device>;

mb8866_device::mb8866_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, MB8866, "MB8866", tag, owner, clock, "mb8866", __FILE__)
{
}


const device_type MB8876 = &device_creator<mb8876_device>;

mb8876_device::mb8876_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, MB8876, "MB8876", tag, owner, clock, "mb8876", __FILE__)
{
}


const device_type MB8877 = &device_creator<mb8877_device>;

mb8877_device::mb8877_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: wd1770_device(mconfig, MB8877, "MB8877_LEGACY", tag, owner, clock, "mb8877_l", __FILE__)
{
}

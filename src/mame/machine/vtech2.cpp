// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/***************************************************************************
    vtech2.c

    machine driver
    Juergen Buchmueller <pullmoll@t-online.de> MESS driver, Jan 2000
    Davide Moretti <dave@rimini.com> ROM dump and hardware description

    TODO:
        Add loading images from WAV files.
        Printer and RS232 support.
        Check if the FDC is really the same as in the
        Laser 210/310 (aka VZ200/300) series.

****************************************************************************/

#include "emu.h"
#include "includes/vtech2.h"
#include "imagedev/flopdrv.h"
#include "imagedev/cassette.h"
#include "sound/speaker.h"

/* public */

/* static */

#define TRKSIZE_VZ  0x9a0   /* arbitrary (actually from analyzing format) */

static const UINT8 laser_fdc_wrprot[2] = {0x80, 0x80};

/* wrappers for bank #1 to #4 */
WRITE8_MEMBER(vtech2_state::mwa_bank1 ) { mwa_bank(0,offset,data); }
WRITE8_MEMBER(vtech2_state::mwa_bank2 ) { mwa_bank(1,offset,data); }
WRITE8_MEMBER(vtech2_state::mwa_bank3 ) { mwa_bank(2,offset,data); }
WRITE8_MEMBER(vtech2_state::mwa_bank4 ) { mwa_bank(3,offset,data); }

/* wrappers for bank #1 to #4 */
READ8_MEMBER(vtech2_state::mra_bank1 ) { return mra_bank(0,offset); }
READ8_MEMBER(vtech2_state::mra_bank2 ) { return mra_bank(1,offset); }
READ8_MEMBER(vtech2_state::mra_bank3 ) { return mra_bank(2,offset); }
READ8_MEMBER(vtech2_state::mra_bank4 ) { return mra_bank(3,offset); }

/* read banked memory (plain ROM/RAM) */
static const char *const mra_bank_hard[4] =
{
	"bank1",  /* mapped in 0000-3fff */
	"bank2",  /* mapped in 4000-7fff */
	"bank3",  /* mapped in 8000-bfff */
	"bank4"   /* mapped in c000-ffff */
};

/* write banked memory (plain ROM/RAM) */
static const char *const mwa_bank_hard[4] =
{
	"bank1",  /* mapped in 0000-3fff */
	"bank2",  /* mapped in 4000-7fff */
	"bank3",  /* mapped in 8000-bfff */
	"bank4"   /* mapped in c000-ffff */
};

DRIVER_INIT_MEMBER(vtech2_state,laser)
{
	UINT8 *gfx = memregion("gfx2")->base();
	int i;

	m_laser_track_x2[0] = m_laser_track_x2[1] = 80;
	m_laser_fdc_bits = 8;
	m_laser_drive = -1;

	for (i = 0; i < 256; i++)
		gfx[i] = i;

	m_laser_latch = -1;
	m_mem = memregion("maincpu")->base();

	// check ROM expansion
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	for (i = 0; i < ARRAY_LENGTH(m_laser_bank); i++)
		m_laser_bank[i] = -1;
}



void vtech2_state::laser_machine_init(int bank_mask, int video_mask)
{
	m_laser_bank_mask = bank_mask;
	m_laser_video_bank = video_mask;
	m_videoram = m_mem + m_laser_video_bank * 0x04000;
	logerror("laser_machine_init(): bank mask $%04X, video %d [$%05X]\n", m_laser_bank_mask, m_laser_video_bank, m_laser_video_bank * 0x04000);

	for (int i = 0; i < ARRAY_LENGTH(m_laser_bank); i++)
		laser_bank_select_w(m_maincpu->space(AS_PROGRAM), i, 0);
}

void vtech2_state::machine_reset()
{
	/* banks 0 to 3 only, optional ROM extension */
	laser_machine_init(0x00f, 3);
}

MACHINE_RESET_MEMBER(vtech2_state,laser500)
{
	/* banks 0 to 2, and 4-7 only, optional ROM extension */
	laser_machine_init(0x0f7, 7);
}

MACHINE_RESET_MEMBER(vtech2_state,laser700)
{
	/* all banks except #3 */
	laser_machine_init(0xff7, 7);
}


WRITE8_MEMBER(vtech2_state::laser_bank_select_w)
{
	static const char *const bank_name[16] = {
		"ROM lo","ROM hi","MM I/O","Video RAM lo",
		"RAM #0","RAM #1","RAM #2","RAM #3",
		"RAM #4","RAM #5","RAM #6","RAM #7/Video RAM hi",
		"ext ROM #0","ext ROM #1","ext ROM #2","ext ROM #3"
	};
	char bank[10];
	offset %= 4;
	data &= 15;

	if( data != m_laser_bank[offset] )
	{
		m_laser_bank[offset] = data;
		logerror("select bank #%d $%02X [$%05X] %s\n", offset+1, data, 0x4000 * (data & 15), bank_name[data]);

		/* memory mapped I/O bank selected? */
		if (data == 2)
		{
			static read8_delegate mra_bank_soft[] =
			{
				read8_delegate(FUNC(vtech2_state::mra_bank1), this),
				read8_delegate(FUNC(vtech2_state::mra_bank2), this),
				read8_delegate(FUNC(vtech2_state::mra_bank3), this),
				read8_delegate(FUNC(vtech2_state::mra_bank4), this),
			};

			static write8_delegate mwa_bank_soft[] =
			{
				write8_delegate(FUNC(vtech2_state::mwa_bank1), this),
				write8_delegate(FUNC(vtech2_state::mwa_bank2), this),
				write8_delegate(FUNC(vtech2_state::mwa_bank3), this),
				write8_delegate(FUNC(vtech2_state::mwa_bank4), this),
			};

			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(offset * 0x4000, offset * 0x4000 + 0x3fff, mra_bank_soft[offset], mwa_bank_soft[offset]);
		}
		else
		{
			sprintf(bank, "bank%d", offset + 1);
			if (data >= 12 && m_cart_rom && (m_cart_rom->bytes() > (data % 12) * 0x4000))   // Expansion ROM banks
			{
				membank(bank)->set_base(m_cart_rom->base()+ (data % 12) * 0x4000);
				m_maincpu->space(AS_PROGRAM).install_read_bank(offset * 0x4000, offset * 0x4000 + 0x3fff, mra_bank_hard[offset]);
				m_maincpu->space(AS_PROGRAM).install_write_bank(offset * 0x4000, offset * 0x4000 + 0x3fff, mwa_bank_hard[offset]);
			}
			else if (data < 12 && (m_laser_bank_mask & (1 << data)))    // ROM/RAM banks
			{
				// video RAM bank selected?
				if (data == m_laser_video_bank)
					logerror("select bank #%d VIDEO!\n", offset + 1);

				membank(bank)->set_base(&m_mem[0x4000 * m_laser_bank[offset]]);
				m_maincpu->space(AS_PROGRAM).install_read_bank(offset * 0x4000, offset * 0x4000 + 0x3fff, mra_bank_hard[offset]);
				m_maincpu->space(AS_PROGRAM).install_write_bank(offset * 0x4000, offset * 0x4000 + 0x3fff, mwa_bank_hard[offset]);
			}
			else
			{
				logerror("select bank #%d MASKED!\n", offset + 1);
				m_maincpu->space(AS_PROGRAM).nop_readwrite(offset * 0x4000, offset * 0x4000 + 0x3fff);
			}
		}

	}
}

/*************************************************
 * memory mapped I/O read
 * bit  function
 * 7    not assigned
 * 6    column 6
 * 5    column 5
 * 4    column 4
 * 3    column 3
 * 2    column 2
 * 1    column 1
 * 0    column 0
 ************************************************/
int vtech2_state::mra_bank(int bank, int offs)
{
	UINT8 data = 0x7f;

	/* Laser 500/700 only: keyboard rows A through D */
	if( (offs & 0x00ff) == 0x00ff )
	{
		if( (offs & 0x0300) == 0x0000 ) /* keyboard row A */
		{
			if( ioport("ROWA")->read() != m_row_a )
			{
				m_row_a = ioport("ROWA")->read();
				data &= m_row_a;
			}
		}
		if( (offs & 0x0300) == 0x0100 ) /* keyboard row B */
		{
			if( ioport("ROWB")->read() != m_row_b )
			{
				m_row_b = ioport("ROWB")->read();
				data &= m_row_b;
			}
		}
		if( (offs & 0x0300) == 0x0200 ) /* keyboard row C */
		{
			if( ioport("ROWC")->read() != m_row_c )
			{
				m_row_c = ioport("ROWC")->read();
				data &= m_row_c;
			}
		}
		if( (offs & 0x0300) == 0x0300 ) /* keyboard row D */
		{
			if( ioport("ROWD")->read() != m_row_d )
			{
				m_row_d = ioport("ROWD")->read();
				data &= m_row_d;
			}
		}
	}
	else
	{
		/* All Lasers keyboard rows 0 through 7 */
		if( !(offs & 0x01) )
			data &= ioport("ROW0")->read();
		if( !(offs & 0x02) )
			data &= ioport("ROW1")->read();
		if( !(offs & 0x04) )
			data &= ioport("ROW2")->read();
		if( !(offs & 0x08) )
			data &= ioport("ROW3")->read();
		if( !(offs & 0x10) )
			data &= ioport("ROW4")->read();
		if( !(offs & 0x20) )
			data &= ioport("ROW5")->read();
		if( !(offs & 0x40) )
			data &= ioport("ROW6")->read();
		if( !(offs & 0x80) )
			data &= ioport("ROW7")->read();
	}

	/* BIT 7 - tape input */

	data |= (m_cassette->input() > +0.02) ? 0x80 : 0;

	return data;
}

/*************************************************
 * memory mapped I/O write
 * bit  function
 * 7-6  not assigned
 * 5    speaker B ???
 * 4    ???
 * 3    mode: 1 graphics, 0 text
 * 2    cassette out (MSB)
 * 1    cassette out (LSB)
 * 0    speaker A
 ************************************************/
void vtech2_state::mwa_bank(int bank, int offs, int data)
{
	offs += 0x4000 * m_laser_bank[bank];
	switch (m_laser_bank[bank])
	{
	case  0:    /* ROM lower 16K */
	case  1:    /* ROM upper 16K */
		logerror("bank #%d write to ROM [$%05X] $%02X\n", bank+1, offs, data);
		break;
	case  2:    /* memory mapped output */
		if (data != m_laser_latch)
		{
			logerror("bank #%d write to I/O [$%05X] $%02X\n", bank+1, offs, data);
			/* Toggle between graphics and text modes? */
			if ((data ^ m_laser_latch) & 0x01)
				m_speaker->level_w(data & 1);
			m_laser_latch = data;
		}
		m_cassette->output( BIT(data, 2) ? -1.0 : +1.0);
		break;
	case 12:    /* ext. ROM #1 */
	case 13:    /* ext. ROM #2 */
	case 14:    /* ext. ROM #3 */
	case 15:    /* ext. ROM #4 */
		logerror("bank #%d write to ROM [$%05X] $%02X\n", bank+1, offs, data);
		break;
	default:    /* RAM */
		if( m_laser_bank[bank] == m_laser_video_bank && m_mem[offs] != data )
		{
			logerror("bank #%d write to videoram [$%05X] $%02X\n", bank+1, offs, data);
		}
		m_mem[offs] = data;
		break;
	}
}


device_t *vtech2_state::laser_file()
{
	return machine().device(m_laser_drive ? FLOPPY_1 : FLOPPY_0);
}

void vtech2_state::laser_get_track()
{
	sprintf(m_laser_frame_message, "#%d get track %02d", m_laser_drive, m_laser_track_x2[m_laser_drive]/2);
	m_laser_frame_time = 30;
	/* drive selected or and image file ok? */
	if( m_laser_drive >= 0 && laser_file() != nullptr )
	{
		int size, offs;
		device_image_interface *image = dynamic_cast<device_image_interface *>(laser_file());
		size = TRKSIZE_VZ;
		offs = TRKSIZE_VZ * m_laser_track_x2[m_laser_drive]/2;
		image->fseek(offs, SEEK_SET);
		size = image->fread(m_laser_fdc_data, size);
		logerror("get track @$%05x $%04x bytes\n", offs, size);
	}
	m_laser_fdc_offs = 0;
	m_laser_fdc_write = 0;
}

void vtech2_state::laser_put_track()
{
	device_image_interface *image = dynamic_cast<device_image_interface *>(laser_file());
	/* drive selected and image file ok? */
	if( m_laser_drive >= 0 && laser_file() != nullptr )
	{
		int size, offs;
		offs = TRKSIZE_VZ * m_laser_track_x2[m_laser_drive]/2;
		image->fseek(offs + m_laser_fdc_start, SEEK_SET);
		size = image->fwrite(&m_laser_fdc_data[m_laser_fdc_start], m_laser_fdc_write);
		logerror("put track @$%05X+$%X $%04X/$%04X bytes\n", offs, m_laser_fdc_start, size, m_laser_fdc_write);
	}
}

#define PHI0(n) (((n)>>0)&1)
#define PHI1(n) (((n)>>1)&1)
#define PHI2(n) (((n)>>2)&1)
#define PHI3(n) (((n)>>3)&1)

READ8_MEMBER(vtech2_state::laser_fdc_r)
{
	int data = 0xff;
	switch( offset )
	{
	case 1: /* data (read-only) */
		if( m_laser_fdc_bits > 0 )
		{
			if( m_laser_fdc_status & 0x80 )
				m_laser_fdc_bits--;
			data = (m_laser_data >> m_laser_fdc_bits) & 0xff;
#if 0
			logerror("laser_fdc_r bits %d%d%d%d%d%d%d%d\n",
				(data>>7)&1,(data>>6)&1,(data>>5)&1,(data>>4)&1,
				(data>>3)&1,(data>>2)&1,(data>>1)&1,(data>>0)&1 );
#endif
		}
		if( m_laser_fdc_bits == 0 )
		{
			m_laser_data = m_laser_fdc_data[m_laser_fdc_offs];
			logerror("laser_fdc_r %d : data ($%04X) $%02X\n", offset, m_laser_fdc_offs, m_laser_data);
			if( m_laser_fdc_status & 0x80 )
			{
				m_laser_fdc_bits = 8;
				m_laser_fdc_offs = (m_laser_fdc_offs + 1) % TRKSIZE_FM;
			}
			m_laser_fdc_status &= ~0x80;
		}
		break;
	case 2: /* polling (read-only) */
		/* fake */
		if( m_laser_drive >= 0 )
			m_laser_fdc_status |= 0x80;
		data = m_laser_fdc_status;
		break;
	case 3: /* write protect status (read-only) */
		if( m_laser_drive >= 0 )
			data = laser_fdc_wrprot[m_laser_drive];
		logerror("laser_fdc_r %d : write_protect $%02X\n", offset, data);
		break;
	}
	return data;
}

WRITE8_MEMBER(vtech2_state::laser_fdc_w)
{
	int drive;

	switch( offset )
	{
	case 0: /* latch (write-only) */
		drive = (data & 0x10) ? 0 : (data & 0x80) ? 1 : -1;
		if( drive != m_laser_drive )
		{
			m_laser_drive = drive;
			if( m_laser_drive >= 0 )
				laser_get_track();
		}
		if( m_laser_drive >= 0 )
		{
			if( (PHI0(data) && !(PHI1(data) || PHI2(data) || PHI3(data)) && PHI1(m_laser_fdc_latch)) ||
				(PHI1(data) && !(PHI0(data) || PHI2(data) || PHI3(data)) && PHI2(m_laser_fdc_latch)) ||
				(PHI2(data) && !(PHI0(data) || PHI1(data) || PHI3(data)) && PHI3(m_laser_fdc_latch)) ||
				(PHI3(data) && !(PHI0(data) || PHI1(data) || PHI2(data)) && PHI0(m_laser_fdc_latch)) )
			{
				if( m_laser_track_x2[m_laser_drive] > 0 )
					m_laser_track_x2[m_laser_drive]--;
				logerror("laser_fdc_w(%d) $%02X drive %d: stepout track #%2d.%d\n", offset, data, m_laser_drive, m_laser_track_x2[m_laser_drive]/2,5*(m_laser_track_x2[m_laser_drive]&1));
				if( (m_laser_track_x2[m_laser_drive] & 1) == 0 )
					laser_get_track();
			}
			else
			if( (PHI0(data) && !(PHI1(data) || PHI2(data) || PHI3(data)) && PHI3(m_laser_fdc_latch)) ||
				(PHI1(data) && !(PHI0(data) || PHI2(data) || PHI3(data)) && PHI0(m_laser_fdc_latch)) ||
				(PHI2(data) && !(PHI0(data) || PHI1(data) || PHI3(data)) && PHI1(m_laser_fdc_latch)) ||
				(PHI3(data) && !(PHI0(data) || PHI1(data) || PHI2(data)) && PHI2(m_laser_fdc_latch)) )
			{
				if( m_laser_track_x2[m_laser_drive] < 2*40 )
					m_laser_track_x2[m_laser_drive]++;
				logerror("laser_fdc_w(%d) $%02X drive %d: stepin track #%2d.%d\n", offset, data, m_laser_drive, m_laser_track_x2[m_laser_drive]/2,5*(m_laser_track_x2[m_laser_drive]&1));
				if( (m_laser_track_x2[m_laser_drive] & 1) == 0 )
					laser_get_track();
			}
			if( (data & 0x40) == 0 )
			{
				m_laser_data <<= 1;
				if( (m_laser_fdc_latch ^ data) & 0x20 )
					m_laser_data |= 1;
				if( (m_laser_fdc_edge ^= 1) == 0 )
				{
					if( --m_laser_fdc_bits == 0 )
					{
						UINT8 value = 0;
						m_laser_data &= 0xffff;
						if( m_laser_data & 0x4000 ) value |= 0x80;
						if( m_laser_data & 0x1000 ) value |= 0x40;
						if( m_laser_data & 0x0400 ) value |= 0x20;
						if( m_laser_data & 0x0100 ) value |= 0x10;
						if( m_laser_data & 0x0040 ) value |= 0x08;
						if( m_laser_data & 0x0010 ) value |= 0x04;
						if( m_laser_data & 0x0004 ) value |= 0x02;
						if( m_laser_data & 0x0001 ) value |= 0x01;
						logerror("laser_fdc_w(%d) data($%04X) $%02X <- $%02X ($%04X)\n", offset, m_laser_fdc_offs, m_laser_fdc_data[m_laser_fdc_offs], value, m_laser_data);
						m_laser_fdc_data[m_laser_fdc_offs] = value;
						m_laser_fdc_offs = (m_laser_fdc_offs + 1) % TRKSIZE_FM;
						m_laser_fdc_write++;
						m_laser_fdc_bits = 8;
					}
				}
			}
			/* change of write signal? */
			if( (m_laser_fdc_latch ^ data) & 0x40 )
			{
				/* falling edge? */
				if ( m_laser_fdc_latch & 0x40 )
				{
					sprintf(m_laser_frame_message, "#%d put track %02d", m_laser_drive, m_laser_track_x2[m_laser_drive]/2);
					m_laser_frame_time = 30;
					m_laser_fdc_start = m_laser_fdc_offs;
					m_laser_fdc_edge = 0;
				}
				else
				{
					/* data written to track before? */
					if( m_laser_fdc_write )
						laser_put_track();
				}
				m_laser_fdc_bits = 8;
				m_laser_fdc_write = 0;
			}
		}
		m_laser_fdc_latch = data;
		break;
	}
}

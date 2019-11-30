// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/***************************************************************************
    vtech2.c

    machine driver
    Juergen Buchmueller <pullmoll@t-online.de> MESS driver, Jan 2000
    Davide Moretti <dave@rimini.com> ROM dump and hardware description

    TODO:
        RS232 support.
        Check if the FDC is really the same as in the
        Laser 210/310 (aka VZ200/300) series.

****************************************************************************/

#include "emu.h"
#include "includes/vtech2.h"

/* public */

/* static */

#define TRKSIZE_VZ  0x9a0   /* arbitrary (actually from analyzing format) */

static const uint8_t laser_fdc_wrprot[2] = {0x80, 0x80};

void vtech2_state::init_laser()
{
	uint8_t *gfx = memregion("gfx2")->base();
	int i;

	m_laser_track_x2[0] = m_laser_track_x2[1] = 80;
	m_laser_fdc_bits = 8;
	m_laser_drive = -1;
	m_cart_size = 0;

	for (i = 0; i < 256; i++)
		gfx[i] = i;

	m_laser_latch = -1;

	// check ROM expansion
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
}


DEVICE_IMAGE_LOAD_MEMBER( vtech2_state::cart_load )
{
	m_cart_size = m_cart->common_get_size("rom");

	if (m_cart_size > 0x10000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Cartridge bigger than 64k");
		m_cart_size = 0;
		return image_init_result::FAIL;
	}

	m_cart->rom_alloc(m_cart_size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), m_cart_size, "rom");

	return image_init_result::PASS;
}

void vtech2_state::machine_reset()
{
	m_language = m_io_keyboard[5]->read() & 0x30;
}


READ8_MEMBER( vtech2_state::cart_r )
{
	if (offset >= m_cart_size)
		return 0xff;
	return m_cart->read_rom(offset);
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
READ8_MEMBER( vtech2_state::mmio_r )
{
	u8 data = 0x7f;

	offset = ~offset & 0x7ff;
	if (BIT(offset, 10))
	{
		offset = (offset >> 8) & 3;
		data &= m_io_keyboard[offset + 8]->read();     // ROW A-D
	}
	else
	for (u8 i = 0; i < 8; i++)
		if (BIT(offset, i))
			data &= m_io_keyboard[i]->read();    // ROW 0-7

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
WRITE8_MEMBER( vtech2_state::mmio_w )
{
	m_speaker->level_w(data & 1);
	m_laser_latch = data;
	m_cassette->output( BIT(data, 2) ? -1.0 : +1.0);
}


void vtech2_state::laser_get_track()
{
	sprintf(m_laser_frame_message, "#%d get track %02d", m_laser_drive, m_laser_track_x2[m_laser_drive]/2);
	m_laser_frame_time = 30;
	/* drive selected or and image file ok? */
	if( m_laser_drive >= 0 && m_laser_file[m_laser_drive].found() )
	{
		device_image_interface &image = *m_laser_file[m_laser_drive];
		int size = TRKSIZE_VZ;
		int offs = TRKSIZE_VZ * m_laser_track_x2[m_laser_drive]/2;
		image.fseek(offs, SEEK_SET);
		size = image.fread(m_laser_fdc_data, size);
		logerror("get track @$%05x $%04x bytes\n", offs, size);
	}
	m_laser_fdc_offs = 0;
	m_laser_fdc_write = 0;
}

void vtech2_state::laser_put_track()
{
	/* drive selected and image file ok? */
	if( m_laser_drive >= 0 && m_laser_file[m_laser_drive].found() )
	{
		device_image_interface &image = *m_laser_file[m_laser_drive];
		int offs = TRKSIZE_VZ * m_laser_track_x2[m_laser_drive]/2;
		image.fseek(offs + m_laser_fdc_start, SEEK_SET);
		int size = image.fwrite(&m_laser_fdc_data[m_laser_fdc_start], m_laser_fdc_write);
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
						uint8_t value = 0;
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

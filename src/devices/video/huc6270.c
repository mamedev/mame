// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    NEC HuC6270 Video Display Controller

    The HuC6270 basically outputs a 9-bit stream of pixel data which
    holds a color index, a palette index, and an indication whether
    the pixel contains background data or from sprite data.

    This data can be used by a colour encoder to output graphics.

    A regular screen is displayed as follows:

        |<- HDS ->|<--       HDW       -->|<- HDE ->|<- HSW ->|
        |---------|-----------------------|---------|---------|
    VSW |                                                     |
        |---------|-----------------------|---------|---------|
    VDS |                                                     |
        |                  overscan                           |
        |---------|-----------------------|---------|---------|
        |         |                       |                   |
        |         |                       |                   |
        |         |                       |                   |
        |         |                       |                   |
    VDW | overscan|    active display     |      overscan     |
        |         |                       |                   |
        |         |                       |                   |
        |         |                       |                   |
        |         |                       |                   |
        |---------|-----------------------|---------|---------|
    VCR |                  overscan                           |
        |                                                     |
        |---------|-----------------------|---------|---------|
        ^end hsync
         ^start vsync (30 cycles after hsync)


KNOWN ISSUES
  - Violent Soldier (probably connected):
    - In the intro some artefacts appear at the top of the
      screen every now and then.
  - In ccovell's splitres test not all sections seem to be aligned properly.
  - Side Arms: Seems to be totally broken.


TODO
  - Fix timing of VRAM-SATB DMA
  - Implement VRAM-VRAM DMA
  - DMA speeds differ depending on the dot clock selected in the huc6270

**********************************************************************/

#include "emu.h"
#include "huc6270.h"

#define LOG 0

enum {
	MAWR = 0x00,
	MARR = 0x01,
	VxR = 0x02,
	CR = 0x05,
	RCR = 0x06,
	BXR = 0x07,
	BYR = 0x08,
	MWR = 0x09,
	HSR = 0x0A,
	HDR = 0x0B,
	VPR = 0x0C,
	VDW = 0x0D,
	VCR = 0x0E,
	DCR = 0x0F,
	SOUR = 0x10,
	DESR = 0x11,
	LENR = 0x12,
	DVSSR = 0x13
};

ALLOW_SAVE_TYPE(huc6270_device::huc6270_v_state);
ALLOW_SAVE_TYPE(huc6270_device::huc6270_h_state);


/* Bits in the VDC status register */
#define HUC6270_BSY         0x40    /* Set when the VDC accesses VRAM */
#define HUC6270_VD          0x20    /* Set when in the vertical blanking period */
#define HUC6270_DV          0x10    /* Set when a VRAM > VRAM DMA transfer is done */
#define HUC6270_DS          0x08    /* Set when a VRAM > SATB DMA transfer is done */
#define HUC6270_RR          0x04    /* Set when the current scanline equals the RCR register */
#define HUC6270_OR          0x02    /* Set when there are more than 16 sprites on a line */
#define HUC6270_CR          0x01    /* Set when sprite #0 overlaps with another sprite */


const device_type HUC6270 = &device_creator<huc6270_device>;


const UINT8 huc6270_device::vram_increments[4] = { 1, 32, 64, 128 };

huc6270_device::huc6270_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, HUC6270, "HuC6270 VDC", tag, owner, clock, "huc6270", __FILE__),
	m_vram_size(0),
	m_irq_changed_cb(*this)
{
}


/*
  Read one row of tile data from video ram
*/
inline void huc6270_device::fetch_bat_tile_row()
{
	UINT16 bat_data, data1, data2, data3, data4, tile_palette;
	int i;

	bat_data = m_vram[ m_bat_address & m_vram_mask ];
	tile_palette = ( bat_data >> 8 ) & 0xF0;
	data1 = m_vram[ ( ( ( bat_data & 0x0FFF ) << 4 ) + m_bat_row + 0 ) & m_vram_mask ];
	data2 = ( data1 >> 7 ) & 0x1FE;
	data3 = m_vram[ ( ( ( bat_data & 0x0FFF ) << 4 ) + m_bat_row + 8 ) & m_vram_mask ];
	data4 = ( data3 >> 5 ) & 0x7F8;
	data3 <<= 2;

	for ( i = 7; i >= 0; i-- )
	{
		UINT16 c = ( data1 & 0x01 ) | ( data2 & 0x02 ) | ( data3 & 0x04 ) | ( data4 & 0x08 );

		/* Colour 0 for background tiles is always taken from palette 0 */
		if ( c )
			c |= tile_palette;

		m_bat_tile_row[i] = c;

		data1 >>= 1;
		data2 >>= 1;
		data3 >>= 1;
		data4 >>= 1;
	}
}


void huc6270_device::add_sprite( int index, int x, int pattern, int line, int flip_x, int palette, int priority, int sat_lsb )
{
	int i = m_sprites_this_line;

	if ( i < 16 )
	{
		UINT32 b0, b1, b2, b3;
		int j;

		if ( flip_x )
			flip_x = 0x0F;

		pattern += ( ( line >> 4 ) << 1 );

		if ( ( m_mwr & 0x0c ) == 0x04 )
		{
			if ( ! sat_lsb )
			{
				b0 = m_vram[ ( ( pattern * 0x40 ) + ( line & 0x0F ) + 0x00 ) & m_vram_mask ];
				b1 = m_vram[ ( ( pattern * 0x40 ) + ( line & 0x0F ) + 0x10 ) & m_vram_mask ] << 1;
			}
			else
			{
				b0 = m_vram[ ( ( pattern * 0x40 ) + ( line & 0x0F ) + 0x20 ) & m_vram_mask ];
				b1 = m_vram[ ( ( pattern * 0x40 ) + ( line & 0x0F ) + 0x30 ) & m_vram_mask ] << 1;
			}
			b2 = 0;
			b3 = 0;
		}
		else
		{
			b0 = m_vram[ ( ( pattern * 0x40 ) + ( line & 0x0F ) + 0x00 ) & m_vram_mask ];
			b1 = m_vram[ ( ( pattern * 0x40 ) + ( line & 0x0F ) + 0x10 ) & m_vram_mask ] << 1;
			b2 = m_vram[ ( ( pattern * 0x40 ) + ( line & 0x0F ) + 0x20 ) & m_vram_mask ] << 2;
			b3 = m_vram[ ( ( pattern * 0x40 ) + ( line & 0x0F ) + 0x30 ) & m_vram_mask ] << 3;
		}

		for ( j = 15; j >= 0; j-- )
		{
			UINT8 data = ( b3 & 0x08 ) | ( b2 & 0x04 ) | ( b1 & 0x02 ) | ( b0 & 0x01 );

			if ( data )
			{
				data |= palette << 4;

				if ( x + ( j ^ flip_x ) < 1024 )
				{
					if (! m_sprite_row[ x + ( j ^ flip_x ) ] )
					{
						m_sprite_row[ x + ( j ^ flip_x ) ] = ( priority ? 0x4000 : 0x0000 ) | ( index << 8 ) | data;
					}
					else
					{
						if ( ! ( m_sprite_row[ x + ( j ^ flip_x ) ] & 0xFF00 ) )
						{
							/* Sprite 0 collission */
							m_sprite_row[ x + ( j ^ flip_x ) ] |= 0x8000;
						}
					}
				}
			}

			b0 >>= 1;
			b1 >>= 1;
			b2 >>= 1;
			b3 >>= 1;
		}

		m_sprites_this_line += 1;
	}
}


void huc6270_device::select_sprites()
{
	int i;

	m_sprites_this_line = 0;
	memset( m_sprite_row, 0, sizeof( m_sprite_row ) );
	m_sprite_row_index = 0x20;

	for ( i = 0; i < 4 * 64; i += 4 )
	{
		static const int cgy_table[4] = { 16, 32, 64, 64 };
		int cgy = ( m_sat[i+3] >> 12 ) & 0x03;
		int height = cgy_table[ cgy ];
		int sprite_line = m_raster_count - m_sat[i];

		if ( sprite_line >= 0 && sprite_line < height )
		{
			int pattern = m_sat[i+2] >> 1;
			int sat_lsb = m_sat[i+2] & 0x01;
			int palette = m_sat[i+3] & 0x0F;
			int priority = m_sat[i+3] & 0x80;
			int cgx = m_sat[i+3] & 0x0100;

			/* If CGY is set to 1, bit 1 of the sprite pattern index is forced to 0 */
			if ( cgy & 1 )
				pattern &= ~0x0002;

			/* If CGY is set to 2 or 3, bits 1 and 2 of the sprite pattern index are forced to 0 */
			if ( cgy & 2 )
				pattern &= ~0x0006;

			/* Recalculate line index when sprite is flipped vertically */
			if ( m_sat[i+3] & 0x8000 )
				sprite_line = ( height - 1 ) - sprite_line;

			/* Is the sprite 32 pixels wide */
			if ( cgx )
			{
				/* If CGX is set, bit 0 of the sprite pattern index is forced to 0 */
				pattern &= ~0x0001;

				/* Check for horizontal flip */
				if ( m_sat[i+3] & 0x0800 )
				{
					/* Add to our list of sprites for this line */
					add_sprite( i/4, m_sat[i+1], pattern + 1, sprite_line, 1, palette, priority, sat_lsb );
					add_sprite( i/4, m_sat[i+1] + 16, pattern, sprite_line, 1, palette, priority, sat_lsb );
				}
				else
				{
					/* Add to our list of sprites for this line */
					add_sprite( i/4, m_sat[i+1], pattern, sprite_line, 0, palette, priority, sat_lsb );
					add_sprite( i/4, m_sat[i+1] + 16, pattern + 1, sprite_line, 0, palette, priority, sat_lsb );
				}
			}
			else
			{
				/* Add to our list of sprites for this line */
				add_sprite( i/4, m_sat[i+1], pattern, sprite_line, m_sat[i+3] & 0x0800, palette, priority, sat_lsb );
			}
		}
	}

	/* Check for sprite overflow */
	if ( m_sprites_this_line >= 16 )
	{
		/* note: flag is set only if irq is taken, Mizubaku Daibouken relies on this behaviour */
		if ( m_cr & 0x02 )
		{
			m_status |= HUC6270_OR;
			m_irq_changed_cb( ASSERT_LINE );
		}
	}
}


inline void huc6270_device::handle_vblank()
{
	if ( ! m_vd_triggered )
	{
		if ( m_cr & 0x08 )
		{
			m_status |= HUC6270_VD;
			m_irq_changed_cb( ASSERT_LINE );
		}

		/* Should we initiate a VRAM->SATB DMA transfer.
		   The timing for this is incorrect.
		 */
		if ( m_dvssr_written || ( m_dcr & 0x10 ) )
		{
			int i;

			if (LOG) logerror("SATB transfer from %05x\n", m_dvssr << 1 );
			for ( i = 0; i < 4 * 64; i += 4 )
			{
				m_sat[i + 0] = m_vram[ ( m_dvssr + i + 0 ) & m_vram_mask ] & 0x03FF;
				m_sat[i + 1] = m_vram[ ( m_dvssr + i + 1 ) & m_vram_mask ] & 0x03FF;
				m_sat[i + 2] = m_vram[ ( m_dvssr + i + 2 ) & m_vram_mask ] & 0x07FF;
				m_sat[i + 3] = m_vram[ ( m_dvssr + i + 3 ) & m_vram_mask ];
			}
			m_dvssr_written = 0;

			/* Generate SATB interrupt if requested */
			if ( m_dcr & 0x01 )
			{
				m_satb_countdown = 4;
//                  m_status |= HUC6270_DS;
//                  m_irq_changed_cb( ASSERT_LINE );
			}
		}

		m_vd_triggered = 1;
	}
}


inline void huc6270_device::next_vert_state()
{
	switch ( m_vert_state )
	{
	case HUC6270_VSW:
		m_vert_state = HUC6270_VDS;
		m_vert_to_go = ( ( m_vpr >> 8 ) & 0xFF ) + 2;
		break;

	case HUC6270_VDS:
		m_vert_state = HUC6270_VDW;
		m_vert_to_go = ( m_vdw & 0x1FF ) + 1;
		m_byr_latched = m_byr;
		m_vd_triggered = 0;
		break;

	case HUC6270_VDW:
		m_vert_state = HUC6270_VCR;
		m_vert_to_go = ( m_vcr & 0xFF );
		handle_vblank();
		break;

	case HUC6270_VCR:
		m_vert_state = HUC6270_VSW;
		m_vert_to_go = ( m_vpr & 0x1F ) + 1;
		break;
	}
}


inline void huc6270_device::next_horz_state()
{
	switch ( m_horz_state )
	{
	case HUC6270_HDS:
		m_bxr_latched = m_bxr;
//if (LOG) printf("latched bxr vpos=%d, hpos=%d\n", video_screen_get_vpos(device->machine->first_screen()), video_screen_get_hpos(device->machine->first_screen()));
		m_horz_state = HUC6270_HDW;
		m_horz_to_go = ( m_hdr & 0x7F ) + 1;
		{
			static const int width_shift[4] = { 5, 6, 7, 7 };
			UINT16 v;

			v = ( m_byr_latched ) & ( ( m_mwr & 0x40 ) ? 0x1FF : 0xFF );
			m_bat_row = v & 7;
			m_bat_address_mask = ( 1 << width_shift[ ( m_mwr >> 4 ) & 0x03 ] ) - 1;
			m_bat_address = ( ( v >> 3 ) << ( width_shift[ ( m_mwr >> 4 ) & 0x03 ] ) )
				| ( ( m_bxr_latched >> 3 ) & m_bat_address_mask );
			m_bat_column = m_bxr & 7;
			fetch_bat_tile_row();
		}
		break;

	case HUC6270_HDW:
		m_horz_state = HUC6270_HDE;
		m_horz_to_go = ( ( m_hdr >> 8 ) & 0x7F ) + 1;
		break;

	case HUC6270_HDE:
		m_horz_state = HUC6270_HSW;
		m_horz_to_go = ( m_hsr & 0x1F ) + 1;
		break;

	case HUC6270_HSW:
		m_horz_state = HUC6270_HDS;
		m_horz_to_go = MAX( ( ( m_hsr >> 8 ) & 0x7F ), 2 ) + 1;

		/* If section has ended, advance to next vertical state */
		while ( m_vert_to_go == 0 )
			next_vert_state();

		/* Select sprites for the coming line */
		select_sprites();
		break;
	}
	m_horz_steps = 0;
}


READ16_MEMBER( huc6270_device::next_pixel )
{
	UINT16 data = HUC6270_SPRITE;

	/* Check if we're on an active display line */
	if ( m_vert_state == HUC6270_VDW )
	{
		/* Check if we're in active display area */
		if ( m_horz_state == HUC6270_HDW )
		{
			UINT8 sprite_data = m_sprite_row[ m_sprite_row_index ] & 0x00FF;
			int collission = ( m_sprite_row[ m_sprite_row_index ] & 0x8000 ) ? 1 : 0;

			if ( m_cr & 0x80 )
			{
				data = HUC6270_BACKGROUND | m_bat_tile_row[ m_bat_column ];
				if ( sprite_data && ( m_cr & 0x40 ) )
				{
					if ( m_sprite_row[ m_sprite_row_index ] & 0x4000 )
					{
						data = HUC6270_SPRITE | sprite_data;
					}
					else
					{
						if ( data == HUC6270_BACKGROUND )
						{
							data = HUC6270_SPRITE | sprite_data;
						}
					}
				}
			}
			else
			{
				if ( m_cr & 0x40 )
				{
					data = HUC6270_SPRITE | sprite_data;
				}
			}

			m_sprite_row_index = m_sprite_row_index + 1;
			m_bat_column += 1;
			if ( m_bat_column >= 8 )
			{
				m_bat_address = ( m_bat_address & ~m_bat_address_mask )
					| ( ( m_bat_address + 1 ) & m_bat_address_mask );
				m_bat_column = 0;
				fetch_bat_tile_row();
			}

			if ( collission && ( m_cr & 0x01 ) )
			{
				m_status |= HUC6270_CR;
				m_irq_changed_cb( ASSERT_LINE );
			}
		}
	}

	m_horz_steps++;
	if ( m_horz_steps == 8 )
	{
		m_horz_to_go -= 1;
		m_horz_steps = 0;
		while ( m_horz_to_go == 0 )
			next_horz_state();
	}
	return data;
}


//inline READ16_MEMBER( huc6270_device::time_until_next_event )
//{
//  return m_horz_to_go * 8 + m_horz_steps;
//}


WRITE_LINE_MEMBER( huc6270_device::vsync_changed )
{
	state &= 0x01;
	if ( m_vsync != state )
	{
		/* Check for low->high VSYNC transition */
		if ( state )
		{
			m_vert_state = HUC6270_VCR;
			m_vert_to_go = 0;

			while ( m_vert_to_go == 0 )
				next_vert_state();
		}
		else
		/* High->low transition */
		{
			handle_vblank();

			/* Should we perform VRAM-VRAM dma.
			   The timing for this is incorrect.
			 */
			if ( m_dma_enabled )
			{
				int desr_inc = ( m_dcr & 0x0008 ) ? -1 : +1;
				int sour_inc = ( m_dcr & 0x0004 ) ? -1 : +1;

				if (LOG) logerror("doing dma sour = %04x, desr = %04x, lenr = %04x\n", m_sour, m_desr, m_lenr );
				do {
					UINT16 data = m_vram[ m_sour & m_vram_mask ];
					m_vram[ m_desr & m_vram_mask ] = data;
					m_sour += sour_inc;
					m_desr += desr_inc;
					m_lenr -= 1;
				} while ( m_lenr != 0xFFFF );

				if ( m_dcr & 0x0002 )
				{
					m_status |= HUC6270_DV;
					m_irq_changed_cb( ASSERT_LINE );
				}
				m_dma_enabled = 0;
			}
		}
	}

	m_vsync = state;
}


WRITE_LINE_MEMBER( huc6270_device::hsync_changed )
{
	state &= 0x01;

	/* Check for high->low HSYNC transition */
	/* Check for low->high HSYNC transition */
	if( ! m_hsync && state )
	{
		if ( m_satb_countdown )
		{
			m_satb_countdown--;

			if ( m_satb_countdown == 0 )
			{
				m_status |= HUC6270_DS;
				m_irq_changed_cb( ASSERT_LINE );
			}
		}

		m_horz_state = HUC6270_HSW;
		m_horz_to_go = 0;
		m_horz_steps = 0;
		m_byr_latched += 1;
		m_raster_count += 1;
		if ( m_vert_to_go == 1 && m_vert_state == HUC6270_VDS )
		{
			m_raster_count = 0x40;
		}

		m_vert_to_go -= 1;

		while ( m_horz_to_go == 0 )
			next_horz_state();

		if ( m_raster_count == m_rcr && ( m_cr & 0x04 ) )
		{
			m_status |= HUC6270_RR;
			m_irq_changed_cb( ASSERT_LINE );
		}
	}

	m_hsync = state;
}


READ8_MEMBER( huc6270_device::read )
{
	UINT8 data = 0x00;

	switch ( offset & 3 )
	{
		case 0x00:  /* status */
			data = m_status;
			m_status &= ~( HUC6270_VD | HUC6270_DV | HUC6270_RR | HUC6270_CR | HUC6270_OR | HUC6270_DS );
			m_irq_changed_cb( CLEAR_LINE );
			break;

		case 0x02:
			data = m_vrr & 0xFF;
			break;

		case 0x03:
			data = m_vrr >> 8;
			if ( m_register_index == VxR )
			{
				m_marr += vram_increments[ ( m_cr >> 11 ) & 3 ];
				m_vrr = m_vram[ m_marr & m_vram_mask ];
			}
			break;
	}
	return data;
}


WRITE8_MEMBER( huc6270_device::write )
{
	if (LOG) logerror("%s: huc6270 write %02x <- %02x ", machine().describe_context(), offset, data);

	switch ( offset & 3 )
	{
		case 0x00:  /* VDC register select */
			m_register_index = data & 0x1F;
			break;

		case 0x02:  /* VDC data LSB */
			switch ( m_register_index )
			{
				case MAWR:      /* memory address write register LSB */
					m_mawr = ( m_mawr & 0xFF00 ) | data;
					break;

				case MARR:      /* memory address read register LSB */
					m_marr = ( m_marr & 0xFF00 ) | data;
					m_vrr = m_vram[ m_marr & m_vram_mask ];
					break;

				case VxR:       /* vram write data LSB */
					m_vwr = ( m_vwr & 0xFF00 ) | data;
					break;

				case CR:        /* control register LSB */
					m_cr = ( m_cr & 0xFF00 ) | data;
					break;

				case RCR:       /* raster compare register LSB */
					m_rcr = ( m_rcr & 0x0300 ) | data;
//printf("%s: RCR set to %03x\n", machine().describe_context(), m_rcr);
//                  if ( m_raster_count == m_rcr && m_cr & 0x04 )
//                  {
//                      m_status |= HUC6270_RR;
//                      m_irq_changed_cb( ASSERT_LINE );
//                  }
//if (LOG) printf("%04x: RCR (%03x) written at %d,%d\n", activecpu_get_pc(), huc6270->m_rcr, video_screen_get_vpos(device->machine->first_screen()), video_screen_get_hpos(device->machine->first_screen()) );
					break;

				case BXR:       /* background x-scroll register LSB */
					m_bxr = ( m_bxr & 0x0300 ) | data;
//if (LOG) printf("*********************** BXR written %d at %d,%d\n", m_bxr, video_screen_get_vpos(device->machine->first_screen()), video_screen_get_hpos(device->machine->first_screen()) );
					break;

				case BYR:       /* background y-scroll register LSB */
					m_byr = ( m_byr & 0x0100 ) | data;
					m_byr_latched = m_byr;
//if (LOG) printf("******************** BYR written %d at %d,%d\n", huc6270->m_byr, video_screen_get_vpos(device->machine->first_screen()), video_screen_get_hpos(device->machine->first_screen()) );
					break;

				case MWR:       /* memory width register LSB */
					m_mwr = ( m_mwr & 0xFF00 ) | data;
					break;

				case HSR:       /* horizontal sync register LSB */
					m_hsr = ( m_hsr & 0xFF00 ) | data;
					break;

				case HDR:       /* horizontal display register LSB */
					m_hdr = ( m_hdr & 0xFF00 ) | data;
					break;

				case VPR:       /* vertical sync register LSB */
					m_vpr = ( m_vpr & 0xFF00 ) | data;
					break;

				case VDW:       /* vertical display register LSB */
					m_vdw = ( m_vdw & 0xFF00 ) | data;
					break;

				case VCR:       /* vertical display end position register LSB */
					m_vcr = ( m_vcr & 0xFF00 ) | data;
					break;

				case DCR:       /* DMA control register LSB */
					m_dcr = ( m_dcr & 0xFF00 ) | data;
					break;

				case SOUR:      /* DMA source address register LSB */
					m_sour = ( m_sour & 0xFF00 ) | data;
					break;

				case DESR:      /* DMA destination address register LSB */
					m_desr = ( m_desr & 0xFF00 ) | data;
					break;

				case LENR:      /* DMA length register LSB */
					m_lenr = ( m_lenr & 0xFF00 ) | data;
					break;

				case DVSSR:     /* Sprite attribute table LSB */
					m_dvssr = ( m_dvssr & 0xFF00 ) | data;
					m_dvssr_written = 1;
					break;
			}
			break;

		case 0x03:  /* VDC data MSB */
			switch ( m_register_index )
			{
				case MAWR:      /* memory address write register MSB */
					m_mawr = ( m_mawr & 0x00FF ) | ( data << 8 );
					break;

				case MARR:      /* memory address read register MSB */
					m_marr = ( m_marr & 0x00FF ) | ( data << 8 );
					m_vrr = m_vram[ m_marr & m_vram_mask ];
					break;

				case VxR:       /* vram write data MSB */
					m_vwr = ( m_vwr & 0x00FF ) | ( data << 8 );
					m_vram[ m_mawr & m_vram_mask ] = m_vwr;
					m_mawr += vram_increments[ ( m_cr >> 11 ) & 3 ];
					break;

				case CR:        /* control register MSB */
					m_cr = ( m_cr & 0x00FF ) | ( data << 8 );
					break;

				case RCR:       /* raster compare register MSB */
					m_rcr = ( m_rcr & 0x00FF ) | ( ( data & 0x03 ) << 8 );
//printf("%s: RCR set to %03x\n", machine().describe_context(), m_rcr);
//                  if ( m_raster_count == m_rcr && m_cr & 0x04 )
//                  {
//                      m_status |= HUC6270_RR;
//                      m_irq_changed_cb( ASSERT_LINE );
//                  }
					break;

				case BXR:       /* background x-scroll register MSB */
					m_bxr = ( m_bxr & 0x00FF ) | ( ( data & 0x03 ) << 8 );
					break;

				case BYR:       /* background y-scroll register MSB */
					m_byr = ( m_byr & 0x00FF ) | ( ( data & 0x01 ) << 8 );
					m_byr_latched = m_byr;
					break;

				case MWR:       /* memory width register MSB */
					m_mwr = ( m_mwr & 0x00FF ) | ( data << 8 );
					break;

				case HSR:       /* horizontal sync register MSB */
					m_hsr = ( m_hsr & 0x00FF ) | ( data << 8 );
					break;

				case HDR:       /* horizontal display register MSB */
					m_hdr = ( m_hdr & 0x00FF ) | ( data << 8 );
					break;

				case VPR:       /* vertical sync register MSB */
					m_vpr = ( m_vpr & 0x00FF ) | ( data << 8 );
					break;

				case VDW:       /* vertical display register MSB */
					m_vdw = ( m_vdw & 0x00FF ) | ( data << 8 );
					break;

				case VCR:       /* vertical display end position register MSB */
					m_vcr = ( m_vcr & 0x00FF ) | ( data << 8 );
					break;

				case DCR:       /* DMA control register MSB */
					m_dcr = ( m_dcr & 0x00FF ) | ( data << 8 );
					break;

				case SOUR:      /* DMA source address register MSB */
					m_sour = ( m_sour & 0x00FF ) | ( data << 8 );
					break;

				case DESR:      /* DMA destination address register MSB */
					m_desr = ( m_desr & 0x00FF ) | ( data << 8 );
					break;

				case LENR:      /* DMA length register MSB */
					m_lenr = ( m_lenr & 0x00FF ) | ( data << 8 );
					m_dma_enabled = 1;
//logerror("DMA is not supported yet.\n");
					break;

				case DVSSR:     /* Sprite attribute table MSB */
					m_dvssr = ( m_dvssr & 0x00FF ) | ( data << 8 );
					m_dvssr_written = 1;
					break;
			}
			break;
	}
	if (LOG) logerror("\n");
}


void huc6270_device::device_start()
{
	/* Resolve callbacks */
	m_irq_changed_cb.resolve_safe();

	m_vram = auto_alloc_array_clear(machine(), UINT16, m_vram_size/sizeof(UINT16));
	m_vram_mask = (m_vram_size >> 1) - 1;

	save_pointer(NAME(m_vram), m_vram_size/sizeof(UINT16));

	save_item(NAME(m_register_index));
	save_item(NAME(m_mawr));
	save_item(NAME(m_marr));
	save_item(NAME(m_vrr));
	save_item(NAME(m_vwr));
	save_item(NAME(m_cr));
	save_item(NAME(m_rcr));
	save_item(NAME(m_bxr));
	save_item(NAME(m_byr));
	save_item(NAME(m_mwr));
	save_item(NAME(m_hsr));
	save_item(NAME(m_hdr));
	save_item(NAME(m_vpr));
	save_item(NAME(m_vdw));
	save_item(NAME(m_vcr));
	save_item(NAME(m_dcr));
	save_item(NAME(m_sour));
	save_item(NAME(m_desr));
	save_item(NAME(m_lenr));
	save_item(NAME(m_dvssr));
	save_item(NAME(m_status));
	save_item(NAME(m_hsync));
	save_item(NAME(m_vsync));
	save_item(NAME(m_vert_state));
	save_item(NAME(m_horz_state));
	save_item(NAME(m_vd_triggered));
	save_item(NAME(m_vert_to_go));
	save_item(NAME(m_horz_to_go));
	save_item(NAME(m_horz_steps));
	save_item(NAME(m_raster_count));
	save_item(NAME(m_dvssr_written));
	save_item(NAME(m_satb_countdown));
	save_item(NAME(m_dma_enabled));
	save_item(NAME(m_byr_latched));
	save_item(NAME(m_bxr_latched));
	save_item(NAME(m_bat_address));
	save_item(NAME(m_bat_address_mask));
	save_item(NAME(m_bat_row));
	save_item(NAME(m_bat_column));
	save_item(NAME(m_bat_tile_row));
	save_item(NAME(m_sat));
	save_item(NAME(m_sprites_this_line));
	save_item(NAME(m_sprite_row_index));
	save_item(NAME(m_sprite_row));
}


void huc6270_device::device_reset()
{
	m_mawr = 0;
	m_marr = 0;
	m_vrr = 0;
	m_vwr = 0;
	m_cr = 0;
	m_rcr = 0;
	m_bxr = 0;
	m_byr = 0;
	m_mwr = 0;
	m_hsr = 0x0202;     /* Take some defaults for horizontal timing */
	m_hdr = 0x041f;
	m_vpr = 0x0f02;     /* Take some defaults for vertical timing */
	m_vdw = 0x00ef;
	m_vcr = 0x0004;
	m_dcr = 0;
	m_sour = 0;
	m_lenr = 0;
	m_dvssr = 0;
	m_status = 0;
	m_vd_triggered = 0;
	m_dvssr_written = 0;
	m_satb_countdown = 0;
	m_raster_count = 0x4000;
	m_vert_to_go = 0;
	m_vert_state = HUC6270_VSW;
	m_horz_steps = 0;
	m_horz_to_go = 0;
	m_horz_state = HUC6270_HDS;
	m_hsync = 0;
	m_vsync = 0;
	m_dma_enabled = 0;
	m_byr_latched = 0;

	memset(m_sat, 0, sizeof(m_sat));
}

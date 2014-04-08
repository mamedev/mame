/***************************************************************************

  video/a7800.c

  Routines to control the Atari 7800 video hardware

    2014-03-24 Mike Saarna Fixed DMA regarding startup, shutdown and
                            cycle stealing.

    2013-05-08 huygens rewrite to emulate line ram buffers (mostly fixes Kung-Fu Master
                            Started DMA cycle stealing implementation

    2003-06-23 ericball Kangaroo mode & 320 mode & other stuff

    2002-05-14 kubecj vblank dma stop fix

    2002-05-13 kubecj   fixed 320C mode (displayed 2 pixels instead of one!)
                            noticed that Jinks uses 0x02-320D mode
                            implemented the mode - completely unsure if good!
                            implemented some Maria CTRL variables

    2002-05-12 kubecj added cases for 0x01-160A, 0x05-160B as stated by docs

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"

#include "includes/a7800.h"


#define TRIGGER_HSYNC   64717

#define READ_MEM(x) space.read_byte(x)

/********** Maria ***********/

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
void a7800_state::video_start()
{
	int i,j;
	for(i=0; i<32; i++)
	{
		m_maria_palette[i]=0;
	}

	for(i=0; i<2; i++)
	{
		for(j=0; j<160; j++)
			m_line_ram[i][j] = 0;
	}

	m_active_buffer = 0;

	m_maria_write_mode=0;
	m_maria_dmaon=0;
	m_maria_vblank=0x80;
	m_maria_dll=0;
	m_maria_wsync=0;

	m_maria_color_kill = 0;
	m_maria_cwidth = 0;
	m_maria_bcntl = 0;
	m_maria_kangaroo = 0;
	m_maria_rm = 0;

	machine().first_screen()->register_screen_bitmap(m_bitmap);
}

/***************************************************************************

  Stop the video hardware emulation.

***************************************************************************/

int a7800_state::is_holey(unsigned int addr)
{
	if (((m_maria_holey & 0x02) && ((addr & 0x9000) == 0x9000)) || ( (m_maria_holey & 0x01) && ((addr & 0x8800) == 0x8800)))
		return 1;
	else
		return 0;
}

int a7800_state::write_line_ram(int addr, UINT8 offset, int pal)
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	int data, c;

	data = READ_MEM(addr);
	pal = pal << 2;

	if (m_maria_write_mode)
	{
		c = (pal & 0x10) | (data & 0x0C) | (data >> 6); // P2 D3 D2 D7 D6
		if (((c & 3) || m_maria_kangaroo) && (offset < 160))
			m_line_ram[m_active_buffer][offset] = c;
		offset++;
		c = (pal & 0x10) | ((data & 0x03) << 2) | ((data & 0x30) >> 4); // P2 D1 D0 D5 D4
		if (((c & 3) || m_maria_kangaroo) && (offset < 160))
			m_line_ram[m_active_buffer][offset] = c;
	}
	else
	{
		for (int i=0; i<4; i++, offset++)
		{
			c = pal | ((data >> (6 - 2*i)) & 0x03);
			if (((c & 3) || m_maria_kangaroo) && (offset < 160))
				m_line_ram[m_active_buffer][offset] = c;
		}
	}
	return m_maria_write_mode ? 2 : 4;
}


void a7800_state::maria_draw_scanline()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	unsigned int graph_adr,data_addr;
	int width,pal,ind;
	UINT8 hpos;
	unsigned int dl;
	int x, d, c, i, pixel_cell, cells;
	int maria_cycles;

		if ( m_maria_offset == 0 )
			maria_cycles = 5+21; // DMA startup + last line shutdown
		else
			maria_cycles = 5+16; // DMA startup + other line shutdown

	cells = 0;

	/* Process this DLL entry */
	dl = m_maria_dl;

	/* DMA */
	/* Step through DL's */
	while ((READ_MEM(dl + 1) & 0x5F) != 0)
	{
		/* Extended header */
		if (!(READ_MEM(dl+1) & 0x1F))
		{
			graph_adr = (READ_MEM(dl+2) << 8) | READ_MEM(dl);
			width = ((READ_MEM(dl+3) ^ 0xff) & 0x1F) + 1;
			hpos = READ_MEM(dl+4);
			pal = READ_MEM(dl+3) >> 5;
			m_maria_write_mode = (READ_MEM(dl+1) & 0x80) >> 5;
			ind = READ_MEM(dl+1) & 0x20;
			dl+=5;
			maria_cycles += 10;
		}
		/* Normal header */
		else
		{
			graph_adr = (READ_MEM(dl+2) << 8) | READ_MEM(dl);
			width = ((READ_MEM(dl+1) ^ 0xff) & 0x1F) + 1;
			hpos = READ_MEM(dl+3);
			pal = READ_MEM(dl+1) >> 5;
			ind = 0x00;
			dl+=4;
			maria_cycles += 8;
		}

		/*logerror("%x DL: ADR=%x  width=%x  hpos=%x  pal=%x  mode=%x  ind=%x\n",m_screen->vpos(),graph_adr,width,hpos,pal,m_maria_write_mode,ind );*/

		for (x=0; x<width; x++)
		{
			/* Do indirect mode */
			if (ind)
			{
				c = READ_MEM(graph_adr + x) & 0xFF;
				data_addr = (m_maria_charbase | c) + (m_maria_offset << 8);
				if (is_holey(data_addr))
					continue;
				maria_cycles += 3;
				if( m_maria_cwidth ) // two data bytes per map byte
				{
					cells = write_line_ram(data_addr, hpos, pal);
					hpos += cells;
					cells = write_line_ram(data_addr+1, hpos, pal);
					hpos += cells;
					maria_cycles += 6;
				}
				else
				{
					cells = write_line_ram(data_addr, hpos, pal);
					hpos += cells;
					maria_cycles += 3;
				}
			}
			else // direct mode
			{
				data_addr = graph_adr + x + (m_maria_offset  << 8);
				if (is_holey(data_addr))
					continue;
				cells = write_line_ram(data_addr, hpos, pal);
				hpos += cells;
				maria_cycles += 3;
			}
		}
	}
			// spin the CPU for Maria DMA, if it's not already spinning for WSYNC
		if ( ! m_maria_wsync )
				m_maincpu->spin_until_time(m_maincpu->cycles_to_attotime(maria_cycles/4)); // Maria clock rate is 4 times that of the CPU

	// draw line buffer to screen
	m_active_buffer = !m_active_buffer; // switch buffers
	UINT16 *scanline;
	scanline = &m_bitmap.pix16(m_screen->vpos());


	for ( i = 0; i<160; i++ )
	{
		switch (m_maria_rm)
		{
			case 0x00:  /* 160A, 160B */
			case 0x01:  /* 160A, 160B */
				pixel_cell =  m_line_ram[m_active_buffer][i];
				scanline[2*i] = m_maria_palette[pixel_cell];
				scanline[2*i+1] = m_maria_palette[pixel_cell];
				break;

			case 0x02: /* 320B, 320D */
				pixel_cell = m_line_ram[m_active_buffer][i];
				d = (pixel_cell & 0x10) | (pixel_cell & 0x02) | ((pixel_cell >> 3) & 1); // b4 0 0 b1 b3
				scanline[2*i] = m_maria_palette[d];
				d = (pixel_cell & 0x10) | ((pixel_cell << 1) & 0x02) | ((pixel_cell >> 2) & 1); // b4 0 0 b0 b2
				scanline[2*i+1] = m_maria_palette[d];
				break;

			case 0x03:  /* 320A, 320C */
				pixel_cell = m_line_ram[m_active_buffer][i];
				d = (pixel_cell & 0x1C) | (pixel_cell & 0x02); // b4 b3 b2 b1 0
				scanline[2*i] = m_maria_palette[d];
				d = (pixel_cell & 0x1C) | ((pixel_cell << 1) & 0x02); // b4 b3 b2 b0 0
				scanline[2*i+1] = m_maria_palette[d];
				break;
		}
	}

	for(i=0; i<160; i++) // buffer automaticaly cleared once displayed
		m_line_ram[m_active_buffer][i] = 0;
}



TIMER_DEVICE_CALLBACK_MEMBER(a7800_state::a7800_interrupt)
{
	// DMA Begins 7 cycles after hblank
	machine().scheduler().timer_set(m_maincpu->cycles_to_attotime(7), timer_expired_delegate(FUNC(a7800_state::a7800_maria_startdma),this));
	if( m_maria_wsync )
	{
		machine().scheduler().trigger( TRIGGER_HSYNC );
		m_maria_wsync = 0;
	}

	int frame_scanline = m_screen->vpos() % ( m_lines + 1 );
	if (frame_scanline == 16)
		m_maria_vblank = 0x00;

	if ( frame_scanline == ( m_lines - 5 ) )
	{
		m_maria_vblank = 0x80;
	}
}


TIMER_CALLBACK_MEMBER(a7800_state::a7800_maria_startdma)
{
	int frame_scanline;
	address_space& space = m_maincpu->space(AS_PROGRAM);
	int maria_scanline = m_screen->vpos();

	frame_scanline = maria_scanline % ( m_lines + 1 );

	if( (frame_scanline == 16) && m_maria_dmaon )
	{
		/* end of vblank */

		m_maria_dll = m_maria_dpp; // currently only handle changes to dll during vblank
		m_maria_dl = (READ_MEM(m_maria_dll+1) << 8) | READ_MEM(m_maria_dll+2);
		m_maria_offset = READ_MEM(m_maria_dll) & 0x0f;
		m_maria_holey = (READ_MEM(m_maria_dll) & 0x60) >> 5;
		m_maria_nmi = READ_MEM(m_maria_dll) & 0x80;
		/*  logerror("DLL=%x\n",m_maria_dll); */
		maria_draw_scanline();
	}


	if( ( frame_scanline > 16 ) && (frame_scanline < (m_lines - 5)) && m_maria_dmaon )
	{
		maria_draw_scanline();

		if( m_maria_offset == 0 )
		{
			m_maria_dll+=3;
			m_maria_dl = (READ_MEM(m_maria_dll+1) << 8) | READ_MEM(m_maria_dll+2);
			m_maria_offset = READ_MEM(m_maria_dll) & 0x0f;
			m_maria_holey = (READ_MEM(m_maria_dll) & 0x60) >> 5;
			if ( READ_MEM(m_maria_dll & 0x10) )
				logerror("dll bit 5 set!\n");
			m_maria_nmi = READ_MEM(m_maria_dll) & 0x80;
		}
		else
		{
			m_maria_offset--;
		}
	}

	if( m_maria_nmi )
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		m_maria_nmi = 0;
	}
}

/***************************************************************************

  Refresh the video screen

***************************************************************************/
/* This routine is called at the start of vblank to refresh the screen */
UINT32 a7800_state::screen_update_a7800(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


/****** MARIA ***************************************/

READ8_MEMBER(a7800_state::a7800_MARIA_r)
{
	switch (offset)
	{
		case 0x08:
			return m_maria_vblank;

		default:
			logerror("undefined MARIA read %x\n",offset);
			return 0x00; // don't know if this should be 0x00 or 0xff
	}
}

WRITE8_MEMBER(a7800_state::a7800_MARIA_w)
{
	int i;

	if ((offset & 3) != 0)
		m_maria_palette[offset] = data;

	switch (offset)
	{
		case 0x00:
			// all color ram addresses with 00 for their least significant bits point to the background color
			for (i = 0; i<8; i++)
				m_maria_palette[4*i] = data;
			break;
		case 0x04:
			m_maincpu->spin_until_trigger(TRIGGER_HSYNC);
			m_maria_wsync=1;
			break;
		case 0x0C: // DPPH
			m_maria_dpp = (m_maria_dpp & 0x00ff) | (data << 8);
			break;
		case 0x10: // DPPL
			m_maria_dpp = (m_maria_dpp & 0xff00) | data;
			break;
		case 0x14:
			m_maria_charbase = (data << 8);
			break;
		case 0x1C:
			/*logerror("MARIA CTRL=%x\n",data);*/
			m_maria_color_kill = data & 0x80;
			switch ((data >> 5) & 3)
			{
				case 0x00: case 01:
				logerror("dma test mode, do not use.\n");
				break;
				case 0x02:
				m_maria_dmaon = 1;
				break;
				case 0x03:
				m_maria_dmaon = 0;
				break;
			}
			m_maria_cwidth = data & 0x10;
			m_maria_bcntl = data & 0x08; // Currently unimplemented as we don't display the border
			m_maria_kangaroo = data & 0x04;
			m_maria_rm = data & 0x03;

			/*logerror( "MARIA CTRL: CK:%d DMA:%d CW:%d BC:%d KM:%d RM:%d\n",
			        m_maria_color_kill ? 1 : 0,
			        ( data & 0x60 ) >> 5,
			        m_maria_cwidth ? 1 : 0,
			        m_maria_bcntl ? 1 : 0,
			        m_maria_kangaroo ? 1 : 0,
			        m_maria_rm );*/

			break;
	}
}

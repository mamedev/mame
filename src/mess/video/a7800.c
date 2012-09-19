/***************************************************************************

  video/a7800.c

  Routines to control the Atari 7800 video hardware

  TODO:
    precise DMA cycle stealing

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


#define TRIGGER_HSYNC	64717

#define READ_MEM(x) space.read_byte(x)

/********** Maria ***********/

#define DPPH 0x2c
#define DPPL 0x30
#define CTRL 0x3c




// 20030621 ericball define using logical operations
#define inc_hpos() { hpos = (hpos + 1) & 0x1FF; }
#define inc_hpos_by_2() { hpos = (hpos + 2) & 0x1FF; }

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
void a7800_state::video_start()
{
	int i;

	for(i=0; i<8; i++)
	{
		m_maria_palette[i][0]=0;
		m_maria_palette[i][1]=0;
		m_maria_palette[i][2]=0;
		m_maria_palette[i][3]=0;
	}

	m_maria_write_mode=0;
	m_maria_scanline=0;
	m_maria_dmaon=0;
	m_maria_vblank=0x80;
	m_maria_dll=0;
	m_maria_dodma=0;
	m_maria_wsync=0;

	m_maria_color_kill = 0;
	m_maria_cwidth = 0;
	m_maria_bcntl = 0;
	m_maria_kangaroo = 0;
	m_maria_rm = 0;

	machine().primary_screen->register_screen_bitmap(m_bitmap);
}

/***************************************************************************

  Stop the video hardware emulation.

***************************************************************************/

static void maria_draw_scanline(running_machine &machine)
{
	a7800_state *state = machine.driver_data<a7800_state>();
	address_space& space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	unsigned int graph_adr,data_addr;
	int width,hpos,pal,mode,ind;
	unsigned int dl;
	int x, d, c, i;
	int ind_bytes;
	UINT16 *scanline;

	/* set up scanline */
	scanline = &state->m_bitmap.pix16(state->m_maria_scanline);
	for (i = 0; i < 320; i++)
		scanline[i] = state->m_maria_backcolor;

	/* Process this DLL entry */
	dl = state->m_maria_dl;

	/* Step through DL's */
	while ((READ_MEM(dl + 1) & 0x5F) != 0)
	{

		/* Extended header */
		if (!(READ_MEM(dl+1) & 0x1F))
		{
			graph_adr = (READ_MEM(dl+2) << 8) | READ_MEM(dl);
			width = ((READ_MEM(dl+3) ^ 0xff) & 0x1F) + 1;
			hpos = READ_MEM(dl+4)*2;
			pal = READ_MEM(dl+3) >> 5;
			state->m_maria_write_mode = (READ_MEM(dl+1) & 0x80) >> 5;
			ind = READ_MEM(dl+1) & 0x20;
			dl+=5;
		}
		/* Normal header */
		else
		{
			graph_adr = (READ_MEM(dl+2) << 8) | READ_MEM(dl);
			width = ((READ_MEM(dl+1) ^ 0xff) & 0x1F) + 1;
			hpos = READ_MEM(dl+3)*2;
			pal = READ_MEM(dl+1) >> 5;
			ind = 0x00;
			dl+=4;
		}

		mode = state->m_maria_rm | state->m_maria_write_mode;

		/*logerror("%x DL: ADR=%x  width=%x  hpos=%x  pal=%x  mode=%x  ind=%x\n",state->m_maria_scanline,graph_adr,width,hpos,pal,mode,ind );*/

		for (x=0; x<width; x++) // 20030621 ericball get graphic data first, then switch (mode)
		{
			ind_bytes = 1;

			/* Do indirect mode */
			if (ind)
			{
				c = READ_MEM(graph_adr + x) & 0xFF;
				data_addr= (state->m_maria_charbase | c) + (state->m_maria_offset << 8);
				if( state->m_maria_cwidth )
					ind_bytes = 2;
			}
			else
			{
				data_addr = graph_adr + x + (state->m_maria_offset  << 8);
			}

			if ( (state->m_maria_holey & 0x02) && ((data_addr & 0x9000) == 0x9000))
				continue;
			if ( (state->m_maria_holey & 0x01) && ((data_addr & 0x8800) == 0x8800))
				continue;

			while (ind_bytes > 0)
			{
				ind_bytes--;
				d = READ_MEM(data_addr++);

				switch (mode)
				{
					case 0x00:  /* 160A (160x2) */
					case 0x01:  /* 160A (160x2) */
						c = (d & 0xC0) >> 6;
						if (c || state->m_maria_kangaroo)
						{
							scanline[hpos + 0] = state->m_maria_palette[pal][c];
							scanline[hpos + 1] = state->m_maria_palette[pal][c];
						}
						inc_hpos_by_2();

						c = (d & 0x30) >> 4;
						if (c || state->m_maria_kangaroo)
						{
							scanline[hpos + 0] = state->m_maria_palette[pal][c];
							scanline[hpos + 1] = state->m_maria_palette[pal][c];
						}
						inc_hpos_by_2();

						c = (d & 0x0C) >> 2;
						if (c || state->m_maria_kangaroo)
						{
							scanline[hpos + 0] = state->m_maria_palette[pal][c];
							scanline[hpos + 1] = state->m_maria_palette[pal][c];
						}
						inc_hpos_by_2();

						c = (d & 0x03);
						if (c || state->m_maria_kangaroo)
						{
							scanline[hpos + 0] = state->m_maria_palette[pal][c];
							scanline[hpos + 1] = state->m_maria_palette[pal][c];
						}
						inc_hpos_by_2();
						break;

					case 0x02: /* 320D used by Jinks! */
						c = pal & 0x04;
						if ( d & 0xC0 || pal & 0x03 || state->m_maria_kangaroo )
						{
							scanline[hpos + 0] = state->m_maria_palette[c][((d & 0x80) >> 6) | ((pal & 2) >> 1)];
							scanline[hpos + 1] = state->m_maria_palette[c][((d & 0x40) >> 5) | ((pal & 1) >> 0)];
						}
						inc_hpos_by_2();

						if ( d & 0x30 || pal & 0x03 || state->m_maria_kangaroo )
						{
							scanline[hpos + 0] = state->m_maria_palette[c][((d & 0x20) >> 4) | ((pal & 2) >> 1)];
							scanline[hpos + 1] = state->m_maria_palette[c][((d & 0x10) >> 3) | ((pal & 1) >> 0)];
						}
						inc_hpos_by_2();

						if ( d & 0x0C || pal & 0x03 || state->m_maria_kangaroo )
						{
							scanline[hpos + 0] = state->m_maria_palette[c][((d & 0x08) >> 2) | ((pal & 2) >> 1)];
							scanline[hpos + 1] = state->m_maria_palette[c][((d & 0x04) >> 1) | ((pal & 1) >> 0)];
						}
						inc_hpos_by_2();

						if ( d & 0x03 || pal & 0x03 || state->m_maria_kangaroo )
						{
							scanline[hpos + 0] = state->m_maria_palette[c][((d & 0x02) << 0) | ((pal & 2) >> 1)];
							scanline[hpos + 1] = state->m_maria_palette[c][((d & 0x01) << 1) | ((pal & 1) >> 0)];
						}
						inc_hpos_by_2();

						break;

					case 0x03:  /* MODE 320A */
						if (d & 0xC0 || state->m_maria_kangaroo)
						{
							scanline[hpos + 0] = state->m_maria_palette[pal][(d & 0x80) >> 6];
							scanline[hpos + 1] = state->m_maria_palette[pal][(d & 0x40) >> 5];
						}
						inc_hpos_by_2();

						if ( d & 0x30 || state->m_maria_kangaroo)
						{
							scanline[hpos + 0] = state->m_maria_palette[pal][(d & 0x20) >> 4];
							scanline[hpos + 1] = state->m_maria_palette[pal][(d & 0x10) >> 3];
						}
						inc_hpos_by_2();

						if (d & 0x0C || state->m_maria_kangaroo)
						{
							scanline[hpos + 0] = state->m_maria_palette[pal][(d & 0x08) >> 2];
							scanline[hpos + 1] = state->m_maria_palette[pal][(d & 0x04) >> 1];
						}
						inc_hpos_by_2();

						if (d & 0x03 || state->m_maria_kangaroo)
						{
							scanline[hpos + 0] = state->m_maria_palette[pal][(d & 0x02)];
							scanline[hpos + 1] = state->m_maria_palette[pal][(d & 0x01) << 1];
						}
						inc_hpos_by_2();
						break;

					case 0x04:  /* 160B (160x4) */
					case 0x05:  /* 160B (160x4) */
						c = (d & 0xC0) >> 6;
						if (c || state->m_maria_kangaroo)
						{
							scanline[hpos + 0] = state->m_maria_palette[(pal & 0x04) | ((d & 0x0C) >> 2)][c];
							scanline[hpos + 1] = state->m_maria_palette[(pal & 0x04) | ((d & 0x0C) >> 2)][c];
						}
						inc_hpos_by_2();

						c = (d & 0x30) >> 4;
						if (c || state->m_maria_kangaroo)
						{
							scanline[hpos + 0] = state->m_maria_palette[(pal & 0x04) | (d & 0x03)][c];
							scanline[hpos + 1] = state->m_maria_palette[(pal & 0x04) | (d & 0x03)][c];
						}
						inc_hpos_by_2();
						break;

					case 0x06:  /* MODE 320B */
						if (d & 0xCC || state->m_maria_kangaroo)
						{
							scanline[hpos + 0] = state->m_maria_palette[pal][((d & 0x80) >> 6) | ((d & 0x08) >> 3)];
							scanline[hpos + 1] = state->m_maria_palette[pal][((d & 0x40) >> 5) | ((d & 0x04) >> 2)];
						}
						inc_hpos_by_2();

						if ( d & 0x33 || state->m_maria_kangaroo)
						{
							scanline[hpos + 0] = state->m_maria_palette[pal][((d & 0x20) >> 4) | ((d & 0x02) >> 1)];
							scanline[hpos + 1] = state->m_maria_palette[pal][((d & 0x10) >> 3) | (d & 0x01)];
						}
						inc_hpos_by_2();
						break;

					case 0x07: /* (320C mode) */
						if (d & 0xC0 || state->m_maria_kangaroo)
						{
							scanline[hpos + 0] = state->m_maria_palette[(pal & 0x04) | ((d & 0x0C) >> 2)][(d & 0x80) >> 6];
							scanline[hpos + 1] = state->m_maria_palette[(pal & 0x04) | ((d & 0x0C) >> 2)][(d & 0x40) >> 5];
						}
						inc_hpos_by_2();

						if ( d & 0x30 || state->m_maria_kangaroo)
						{
							scanline[hpos + 0] = state->m_maria_palette[(pal & 0x04) | (d & 0x03)][(d & 0x20) >> 4];
							scanline[hpos + 1] = state->m_maria_palette[(pal & 0x04) | (d & 0x03)][(d & 0x10) >> 3];
						}
						inc_hpos_by_2();
						break;

				}	/* endswitch (mode) */
			}	/* endwhile (ind_bytes > 0)*/
		}	/* endfor (x=0; x<width; x++) */
	}	/* endwhile (READ_MEM(dl + 1) != 0) */
}


TIMER_DEVICE_CALLBACK( a7800_interrupt )
{
	a7800_state *state = timer.machine().driver_data<a7800_state>();
	int frame_scanline;
	UINT8 *ROM = timer.machine().root_device().memregion("maincpu")->base();
	address_space& space = timer.machine().device("maincpu")->memory().space(AS_PROGRAM);

	state->m_maria_scanline++;

	/* why + 1? */
	frame_scanline = state->m_maria_scanline % ( state->m_lines + 1 );

	if( frame_scanline == 1 )
	{
		/*logerror( "frame beg\n" );*/
	}

	if( state->m_maria_wsync )
	{
		timer.machine().scheduler().trigger( TRIGGER_HSYNC );
		state->m_maria_wsync = 0;
	}

	if( frame_scanline == 16 )
	{
		/* end of vblank */

		state->m_maria_vblank=0;
		if( state->m_maria_dmaon || state->m_maria_dodma )
		{
			state->m_maria_dodma = 1;	/* if dma allowed, start it */

			state->m_maria_dll = (ROM[DPPH] << 8) | ROM[DPPL];
			state->m_maria_dl = (READ_MEM(state->m_maria_dll+1) << 8) | READ_MEM(state->m_maria_dll+2);
			state->m_maria_offset = READ_MEM(state->m_maria_dll) & 0x0f;
			state->m_maria_holey = (READ_MEM(state->m_maria_dll) & 0x60) >> 5;
			state->m_maria_dli = READ_MEM(state->m_maria_dll) & 0x80;
			/*  logerror("DLL=%x\n",state->m_maria_dll); */
			/*  logerror("DLL: DL = %x  dllctrl = %x\n",state->m_maria_dl,ROM[state->m_maria_dll]); */
		}

		/*logerror( "vblank end on line %d\n", frame_scanline );*/
	}

	/*  moved start of vblank up (to prevent dma/dli happen on line -4)
        this fix made PR Baseball happy
        Kung-Fu Master looks worse
        don't know about others yet */
	if( frame_scanline == ( state->m_lines - 4 ) )
	{
		/* vblank starts 4 scanlines before end of screen */

		state->m_maria_vblank = 0x80;

		/* fixed 2002/05/14 kubecj
                when going vblank, dma must be stopped
                otherwise system tries to read past end of dll
                causing false dlis to occur, mainly causing wild
                screen flickering

                games fixed:
                Ace of Aces
                Mean 18
                Ninja Golf (end of levels)
                Choplifter
                Impossible Mission
                Jinks
        */

		state->m_maria_dodma = 0;
		/*logerror( "vblank on line %d\n\n", frame_scanline );*/
	}


	if( ( frame_scanline > 15 ) && state->m_maria_dodma )
	{
		if (state->m_maria_scanline < ( state->m_lines - 4 ) )
			maria_draw_scanline(timer.machine());

		if( state->m_maria_offset == 0 )
		{
			state->m_maria_dll+=3;
			state->m_maria_dl = (READ_MEM(state->m_maria_dll+1) << 8) | READ_MEM(state->m_maria_dll+2);
			state->m_maria_offset = READ_MEM(state->m_maria_dll) & 0x0f;
			state->m_maria_holey = (READ_MEM(state->m_maria_dll) & 0x60) >> 5;
			state->m_maria_dli = READ_MEM(state->m_maria_dll) & 0x80;
		}
		else
		{
			state->m_maria_offset--;
		}
	}

	if( state->m_maria_dli )
	{
		/*logerror( "dli on line %d [%02X] [%02X] [%02X]\n", frame_scanline, ROM[0x7E], ROM[0x7C], ROM[0x7D] );*/
	}

	if( state->m_maria_dli )
	{
		state->m_maria_dli = 0;
		state->m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}

}

/***************************************************************************

  Refresh the video screen

***************************************************************************/
/* This routine is called at the start of vblank to refresh the screen */
UINT32 a7800_state::screen_update_a7800(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_maria_scanline = 0;
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


/****** MARIA ***************************************/

READ8_MEMBER(a7800_state::a7800_MARIA_r)
{
	UINT8 *ROM = memregion("maincpu")->base();
	switch (offset)
	{
		case 0x08:
			return m_maria_vblank;

		default:
			logerror("undefined MARIA read %x\n",offset);
			return ROM[0x20 + offset];
	}
}

WRITE8_MEMBER(a7800_state::a7800_MARIA_w)
{
	UINT8 *ROM = memregion("maincpu")->base();
	switch (offset)
	{
		case 0x00:
			m_maria_backcolor = data;
			// 20030621 ericball added m_maria_palette[pal][0] to make kanagroo mode easier
			m_maria_palette[0][0]=m_maria_backcolor;
			m_maria_palette[1][0]=m_maria_backcolor;
			m_maria_palette[2][0]=m_maria_backcolor;
			m_maria_palette[3][0]=m_maria_backcolor;
			m_maria_palette[4][0]=m_maria_backcolor;
			m_maria_palette[5][0]=m_maria_backcolor;
			m_maria_palette[6][0]=m_maria_backcolor;
			m_maria_palette[7][0]=m_maria_backcolor;
			break;
		case 0x01:
			m_maria_palette[0][1] = data;
			break;
		case 0x02:
			m_maria_palette[0][2] = data;
			break;
		case 0x03:
			m_maria_palette[0][3] = data;
			break;
		case 0x04:
			machine().device("maincpu")->execute().spin_until_trigger(TRIGGER_HSYNC);
			m_maria_wsync=1;
			break;

		case 0x05:
			m_maria_palette[1][1] = data;
			break;
		case 0x06:
			m_maria_palette[1][2] = data;
			break;
		case 0x07:
			m_maria_palette[1][3] = data;
			break;

		case 0x09:
			m_maria_palette[2][1] = data;
			break;
		case 0x0A:
			m_maria_palette[2][2] = data;
			break;
		case 0x0B:
			m_maria_palette[2][3] = data;
			break;

		case 0x0D:
			m_maria_palette[3][1] = data;
			break;
		case 0x0E:
			m_maria_palette[3][2] = data;
			break;
		case 0x0F:
			m_maria_palette[3][3] = data;
			break;

		case 0x11:
			m_maria_palette[4][1] = data;
			break;
		case 0x12:
			m_maria_palette[4][2] = data;
			break;
		case 0x13:
			m_maria_palette[4][3] = data;
			break;
		case 0x14:
			m_maria_charbase = (data << 8);
			break;
		case 0x15:
			m_maria_palette[5][1] = data;
			break;
		case 0x16:
			m_maria_palette[5][2] = data;
			break;
		case 0x17:
			m_maria_palette[5][3] = data;
			break;

		case 0x19:
			m_maria_palette[6][1] = data;
			break;
		case 0x1A:
			m_maria_palette[6][2] = data;
			break;
		case 0x1B:
			m_maria_palette[6][3] = data;
			break;

		case 0x1C:
			/*logerror("MARIA CTRL=%x\n",data);*/
			m_maria_color_kill = data & 0x80;
			if ((data & 0x60) == 0x40)
				m_maria_dmaon = 1;
			else
				m_maria_dmaon = m_maria_dodma = 0;

			m_maria_cwidth = data & 0x10;
			m_maria_bcntl = data & 0x08;
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
		case 0x1D:
			m_maria_palette[7][1] = data;
			break;
		case 0x1E:
			m_maria_palette[7][2] = data;
			break;
		case 0x1F:
			m_maria_palette[7][3] = data;
			break;
	}
	ROM[ 0x20 + offset ] = data;
}


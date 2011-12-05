#include "emu.h"
#include "video/vdc.h"

/* VDC segments */
#define STATE_VSW		0
#define STATE_VDS		1
#define STATE_VDW		2
#define STATE_VCR		3

/* todo: replace this with the PAIR structure */
typedef union
{
#ifdef LSB_FIRST
	struct { unsigned char l,h; } b;
#else
	struct { unsigned char h,l; } b;
#endif
	unsigned short int w;
}pair;

/* the VDC context */

typedef struct
{
	int dvssr_write;			/* Set when the DVSSR register has been written to */
	int physical_width;			/* Width of the display */
	int physical_height;		/* Height of the display */
	UINT16 sprite_ram[64*4];	/* Sprite RAM */
	int curline;				/* the current scanline we're on */
	int current_segment;		/* current segment of display */
	int current_segment_line;	/* current line inside a segment of display */
	int vblank_triggered;		/* to indicate whether vblank has been triggered */
	int raster_count;			/* counter to compare RCR against */
	int satb_countdown;			/* scanlines to wait to trigger the SATB irq */
	UINT8 *vram;
	UINT8   inc;
	UINT8 vdc_register;
	UINT8 vdc_latch;
	pair vdc_data[32];
	int status;
	int y_scroll;
}VDC;

typedef struct {
	UINT8	vce_control;			/* VCE control register */
	pair	vce_address;			/* Current address in the palette */
	pair	vce_data[512];			/* Palette data */
	int		current_bitmap_line;	/* The current line in the display we are on */
	bitmap_t	*bmp;
}VCE;

typedef struct {
	UINT8 prio;
	UINT8 vdc0_enabled;
	UINT8 vdc1_enabled;
} VPC_PRIO;

typedef struct {
	VPC_PRIO vpc_prio[4];
	UINT8	prio_map[512];		/* Pre-calculated priority map */
	pair	priority;			/* Priority settings registers */
	pair	window1;			/* Window 1 setting */
	pair	window2;			/* Window 2 setting */
	UINT8	vdc_select;			/* Which VDC do the ST0, ST1, and ST2 instructions write to */
}VPC;

static VDC vdc[2];
static VCE vce;
static VPC vpc;

/* Function prototypes */

static void vdc_advance_line(running_machine &machine, int which);
static void draw_black_line(running_machine &machine, int line);
static void draw_overscan_line(int line);
static void draw_sgx_overscan_line(int line);
static void pce_refresh_line(int which, int line, int external_input, UINT8 *drawn, UINT16 *line_buffer);
static void pce_refresh_sprites(running_machine &machine, int which, int line, UINT8 *drawn, UINT16 *line_buffer);
static void vdc_do_dma(running_machine &machine, int which);
static void vpc_init( running_machine &machine );

TIMER_DEVICE_CALLBACK( pce_interrupt )
{
	/* Draw the last scanline */
	if ( vce.current_bitmap_line >= 14 && vce.current_bitmap_line < 14 + 242 )
	{
		/* We are in the active display area */
		/* First fill the line with the overscan color */
		draw_overscan_line(vce.current_bitmap_line );

		/* Check if we need to draw more just the overscan color */
		if ( vdc[0].current_segment == STATE_VDW )
		{
			/* 0 - no sprite and background pixels drawn
               1 - background pixel drawn
               otherwise is 2 + sprite# */
			UINT8 drawn[VDC_WPF];
			/* our line buffer */
			UINT16 *line_buffer = BITMAP_ADDR16( vce.bmp, vce.current_bitmap_line, 86 );

			/* clear our priority/sprite collision detection buffer. */
			memset(drawn, 0, VDC_WPF);

			vdc[0].y_scroll = ( vdc[0].current_segment_line == 0 ) ? vdc[0].vdc_data[BYR].w : ( vdc[0].y_scroll + 1 );

			/* Draw VDC #0 background layer */
			pce_refresh_line( 0, vdc[0].current_segment_line, 0, drawn, line_buffer);

			/* Draw VDC #0 sprite layer */
			if(vdc[0].vdc_data[CR].w & CR_SB)
			{
				pce_refresh_sprites(timer.machine(), 0, vdc[0].current_segment_line, drawn, line_buffer);
			}
		}
	}
	else
	{
		/* We are in one of the blanking areas */
		draw_black_line(timer.machine(), vce.current_bitmap_line );
	}

	/* bump current scanline */
	vce.current_bitmap_line = ( vce.current_bitmap_line + 1 ) % VDC_LPF;
	vdc_advance_line(timer.machine(), 0 );
}

TIMER_DEVICE_CALLBACK( sgx_interrupt )
{
	/* Draw the last scanline */
	if ( vce.current_bitmap_line >= 14 && vce.current_bitmap_line < 14 + 242 )
	{
		/* We are in the active display area */
		/* First fill the line with the overscan color */
		draw_sgx_overscan_line(vce.current_bitmap_line );

		/* Check if we need to draw more just the overscan color */
		if ( vdc[0].current_segment == STATE_VDW )
		{
			/* 0 - no sprite and background pixels drawn
               1 - background pixel drawn
               otherwise is 2 + sprite# */
			UINT8 drawn[2][512];
			UINT16 *line_buffer;
			UINT16 temp_buffer[2][512];
			int i;

			/* clear our priority/sprite collision detection buffer. */
			memset( drawn, 0, sizeof(drawn) );

			vdc[0].y_scroll = ( vdc[0].current_segment_line == 0 ) ? vdc[0].vdc_data[BYR].w : ( vdc[0].y_scroll + 1 );
			vdc[1].y_scroll = ( vdc[1].current_segment_line == 0 ) ? vdc[1].vdc_data[BYR].w : ( vdc[1].y_scroll + 1 );

			/* Draw VDC #0 background layer */
			pce_refresh_line( 0, vdc[0].current_segment_line, 0, drawn[0], temp_buffer[0]);

			/* Draw VDC #0 sprite layer */
			if(vdc[0].vdc_data[CR].w & CR_SB)
			{
				pce_refresh_sprites(timer.machine(), 0, vdc[0].current_segment_line, drawn[0], temp_buffer[0]);
			}

			/* Draw VDC #1 background layer */
			pce_refresh_line( 1, vdc[1].current_segment_line, 1, drawn[1], temp_buffer[1]);

			/* Draw VDC #1 sprite layer */
			if ( vdc[1].vdc_data[CR].w & CR_SB )
			{
				pce_refresh_sprites(timer.machine(), 1, vdc[1].current_segment_line, drawn[1], temp_buffer[1]);
			}

			line_buffer = BITMAP_ADDR16( vce.bmp, vce.current_bitmap_line, 86 );
			/* Combine the output of both VDCs */
			for( i = 0; i < 512; i++ )
			{
				int cur_prio = vpc.prio_map[i];

				if ( vpc.vpc_prio[cur_prio].vdc0_enabled )
				{
					if ( vpc.vpc_prio[cur_prio].vdc1_enabled )
					{
						switch( vpc.vpc_prio[cur_prio].prio )
						{
						case 0:	/* BG1 SP1 BG0 SP0 */
							if ( drawn[0][i] )
							{
								line_buffer[i] = temp_buffer[0][i];
							}
							else if ( drawn[1][i] )
							{
								line_buffer[i] = temp_buffer[1][i];
							}
							break;
						case 1:	/* BG1 BG0 SP1 SP0 */
							if ( drawn[0][i] )
							{
								if ( drawn[0][i] > 1 )
								{
									line_buffer[i] = temp_buffer[0][i];
								}
								else
								{
									if ( drawn[1][i] > 1 )
									{
										line_buffer[i] = temp_buffer[1][i];
									}
									else
									{
										line_buffer[i] = temp_buffer[0][i];
									}
								}
							}
							else if ( drawn[1][i] )
							{
								line_buffer[i] = temp_buffer[1][i];
							}
							break;
						case 2: /* BG1 + SP1 => SP1
                                   BG0 + SP1 => BG0
                                   BG0 + BG1 => BG0
                                   BG0 + SP0 => SP0
                                   BG1 + SP0 => BG1
                                   SP0 + SP1 => SP0 */
							if ( drawn[0][i] )
							{
								if ( drawn[0][i] > 1 )
								{
									if ( drawn[1][i] == 1 )
									{
										line_buffer[i] = temp_buffer[1][i];
									}
									else
									{
										line_buffer[i] = temp_buffer[0][i];
									}
								}
								else
								{
									line_buffer[i] = temp_buffer[0][i];
								}
							}
							else if ( drawn[1][i] )
							{
								line_buffer[i] = temp_buffer[1][i];
							}
							break;
						}
					}
					else
					{
						if ( drawn[0][i] )
						{
							line_buffer[i] = temp_buffer[0][i];
						}
					}
				}
				else
				{
					if ( vpc.vpc_prio[cur_prio].vdc1_enabled )
					{
						if ( drawn[1][i] )
						{
							line_buffer[i] = temp_buffer[1][i];
						}
					}
				}
			}
		}
	}
	else
	{
		/* We are in one of the blanking areas */
		draw_black_line(timer.machine(), vce.current_bitmap_line );
	}

	/* bump current scanline */
	vce.current_bitmap_line = ( vce.current_bitmap_line + 1 ) % VDC_LPF;
	vdc_advance_line(timer.machine(), 0 );
	vdc_advance_line(timer.machine(), 1 );
}

static void vdc_advance_line(running_machine &machine, int which)
{
	int ret = 0;

	vdc[which].curline += 1;
	vdc[which].current_segment_line += 1;
	vdc[which].raster_count += 1;

	if ( vdc[which].satb_countdown )
	{
		vdc[which].satb_countdown -= 1;
		if ( vdc[which].satb_countdown == 0 )
		{
			if ( vdc[which].vdc_data[DCR].w & DCR_DSC )
			{
				vdc[which].status |= VDC_DS;	/* set satb done flag */
				ret = 1;
			}
		}
	}

	if ( vce.current_bitmap_line == 0 )
	{
		vdc[which].current_segment = STATE_VSW;
		vdc[which].current_segment_line = 0;
		vdc[which].vblank_triggered = 0;
		vdc[which].curline = 0;
	}

	if ( STATE_VSW == vdc[which].current_segment && vdc[which].current_segment_line >= ( vdc[which].vdc_data[VPR].b.l & 0x1F ) )
	{
		vdc[which].current_segment = STATE_VDS;
		vdc[which].current_segment_line = 0;
	}

	if ( STATE_VDS == vdc[which].current_segment && vdc[which].current_segment_line >= vdc[which].vdc_data[VPR].b.h )
	{
		vdc[which].current_segment = STATE_VDW;
		vdc[which].current_segment_line = 0;
		vdc[which].raster_count = 0x40;
	}

	if ( STATE_VDW == vdc[which].current_segment && vdc[which].current_segment_line > ( vdc[which].vdc_data[VDW].w & 0x01FF ) )
	{
		vdc[which].current_segment = STATE_VCR;
		vdc[which].current_segment_line = 0;

		/* Generate VBlank interrupt, sprite DMA */
		vdc[which].vblank_triggered = 1;
		if ( vdc[which].vdc_data[CR].w & CR_VR )
		{
			vdc[which].status |= VDC_VD;
			ret = 1;
		}

		/* do VRAM > SATB DMA if the enable bit is set or the DVSSR reg. was written to */
		if( ( vdc[which].vdc_data[DCR].w & DCR_DSR ) || vdc[which].dvssr_write )
		{
			int i;

			vdc[which].dvssr_write = 0;

			for( i = 0; i < 256; i++ )
			{
				vdc[which].sprite_ram[i] = ( vdc[which].vram[ ( vdc[which].vdc_data[DVSSR].w << 1 ) + i * 2 + 1 ] << 8 ) | vdc[which].vram[ ( vdc[which].vdc_data[DVSSR].w << 1 ) + i * 2 ];
			}

			/* generate interrupt if needed */
			if ( vdc[which].vdc_data[DCR].w & DCR_DSC )
			{
				vdc[which].satb_countdown = 4;
			}
		}
	}

	if ( STATE_VCR == vdc[which].current_segment )
	{
		if ( vdc[which].current_segment_line >= 3 && vdc[which].current_segment_line >= vdc[which].vdc_data[VCR].b.l )
		{
			vdc[which].current_segment = STATE_VSW;
			vdc[which].current_segment_line = 0;
			vdc[which].curline = 0;
		}
	}

	/* generate interrupt on line compare if necessary */
	if ( vdc[which].raster_count == vdc[which].vdc_data[RCR].w && vdc[which].vdc_data[CR].w & CR_RC )
	{
		vdc[which].status |= VDC_RR;
		ret = 1;
	}

	/* handle frame events */
	if(vdc[which].curline == 261 && ! vdc[which].vblank_triggered )
	{

		vdc[which].vblank_triggered = 1;
		if(vdc[which].vdc_data[CR].w & CR_VR)
		{	/* generate IRQ1 if enabled */
			vdc[which].status |= VDC_VD;	/* set vblank flag */
			ret = 1;
		}

		/* do VRAM > SATB DMA if the enable bit is set or the DVSSR reg. was written to */
		if ( ( vdc[which].vdc_data[DCR].w & DCR_DSR ) || vdc[which].dvssr_write )
		{
			int i;

			vdc[which].dvssr_write = 0;
#ifdef MAME_DEBUG
			assert(((vdc[which].vdc_data[DVSSR].w<<1) + 512) <= 0x10000);
#endif
			for( i = 0; i < 256; i++ )
			{
				vdc[which].sprite_ram[i] = ( vdc[which].vram[ ( vdc[which].vdc_data[DVSSR].w << 1 ) + i * 2 + 1 ] << 8 ) | vdc[which].vram[ ( vdc[which].vdc_data[DVSSR].w << 1 ) + i * 2 ];
			}

			/* generate interrupt if needed */
			if(vdc[which].vdc_data[DCR].w & DCR_DSC)
			{
				vdc[which].satb_countdown = 4;
			}
		}
	}

	if (ret)
		cputag_set_input_line(machine, "maincpu", 0, HOLD_LINE);
}

VIDEO_START( pce )
{
	logerror("*** pce_vh_start\n");

	/* clear context */
	memset(&vdc, 0, sizeof(vdc));
	memset(&vce, 0, sizeof(vce));
	memset(&vpc, 0, sizeof(vpc));

	/* allocate VRAM */
	vdc[0].vram = auto_alloc_array(machine, UINT8, 0x10000);
	vdc[1].vram = auto_alloc_array(machine, UINT8, 0x10000);
	memset(vdc[0].vram, 0, 0x10000);
	memset(vdc[1].vram, 0, 0x10000);

	/* create display bitmap */
	vce.bmp = machine.primary_screen->alloc_compatible_bitmap();

	vdc[0].inc = 1;
	vdc[1].inc = 1;

	vpc_init(machine);
}


SCREEN_UPDATE( pce )
{
	/* copy our rendering buffer to the display */
	copybitmap (bitmap,vce.bmp,0,0,0,0,cliprect);
	return 0;
}

static void draw_black_line(running_machine &machine, int line)
{
	int i;

	/* our line buffer */
	UINT16 *line_buffer = BITMAP_ADDR16( vce.bmp, line, 0 );

	for( i=0; i< VDC_WPF; i++ )
		line_buffer[i] = get_black_pen( machine );
}

static void draw_overscan_line(int line)
{
	int i;

	/* Are we in greyscale mode or in color mode? */
	int color_base = vce.vce_control & 0x80 ? 512 : 0;

	/* our line buffer */
	UINT16 *line_buffer = BITMAP_ADDR16( vce.bmp, line, 0 );

	for ( i = 0; i < VDC_WPF; i++ )
		line_buffer[i] = color_base + vce.vce_data[0x100].w;
}

static void draw_sgx_overscan_line(int line)
{
	int i;

	/* Are we in greyscale mode or in color mode? */
	int color_base = vce.vce_control & 0x80 ? 512 : 0;

	/* our line buffer */
	UINT16 *line_buffer = BITMAP_ADDR16( vce.bmp, line, 0 );

	for ( i = 0; i < VDC_WPF; i++ )
		line_buffer[i] = color_base + vce.vce_data[0].w;
}

static void vram_write(int which, offs_t offset, UINT8 data)
{
	if(offset & 0x10000)
	{
		logerror("VDC #%d: Write to VRAM offset %05X\n", which, offset);
		return;
	}
	else
	{
		vdc[which].vram[offset] = data;
	}
}

static UINT8 vram_read(int which, offs_t offset)
{
	UINT8 temp;

	if(offset & 0x10000)
	{
		temp = vdc[which].vram[offset & 0xFFFF];
	}
	else
	{
		temp = vdc[which].vram[offset];
	}

	return temp;
}


static void vdc_w( running_machine &machine, int which, offs_t offset, UINT8 data )
{
	switch(offset&3)
	{
		case 0x00:	/* VDC register select */
			vdc[which].vdc_register = (data & 0x1F);
			break;

		case 0x02:	/* VDC data (LSB) */
			vdc[which].vdc_data[vdc[which].vdc_register].b.l = data;
			switch(vdc[which].vdc_register)
			{
				case VxR:	/* LSB of data to write to VRAM */
					vdc[which].vdc_latch = data;
					break;

			    case BYR:
					vdc[which].y_scroll=vdc[which].vdc_data[BYR].w;
					break;

				case HDR:
					vdc[which].physical_width = ((data & 0x003F) + 1) << 3;
					break;

				case VDW:
					vdc[which].physical_height &= 0xFF00;
					vdc[which].physical_height |= (data & 0xFF);
					vdc[which].physical_height &= 0x01FF;
					break;

				case LENR:
//                  logerror("LENR LSB = %02X\n", data);
					break;
				case SOUR:
//                  logerror("SOUR LSB = %02X\n", data);
					break;
				case DESR:
//                  logerror("DESR LSB = %02X\n", data);
					break;
			}
			break;

		case 0x03:	/* VDC data (MSB) */
			vdc[which].vdc_data[vdc[which].vdc_register].b.h = data;
			switch(vdc[which].vdc_register)
			{
				case VxR:	/* MSB of data to write to VRAM */
					vram_write(which, vdc[which].vdc_data[MAWR].w*2+0, vdc[which].vdc_latch);
					vram_write(which, vdc[which].vdc_data[MAWR].w*2+1, data);
					vdc[which].vdc_data[MAWR].w += vdc[which].inc;
					break;

				case CR:
					{
						static const unsigned char inctab[] = {1, 32, 64, 128};
						vdc[which].inc = inctab[(data >> 3) & 3];
					}
					break;

				case VDW:
					vdc[which].physical_height &= 0x00FF;
					vdc[which].physical_height |= (data << 8);
					vdc[which].physical_height &= 0x01FF;
					break;

				case DVSSR:
					/* Force VRAM <> SATB DMA for this frame */
					vdc[which].dvssr_write = 1;
					break;

				case BYR:
					vdc[which].y_scroll=vdc[which].vdc_data[BYR].w;
					break;

				case LENR:
					vdc_do_dma( machine, which );
					break;
				case SOUR:
//                  logerror("SOUR MSB = %02X\n", data);
					break;
				case DESR:
//                  logerror("DESR MSB = %02X\n", data);
					break;
			}
			break;
	}
}

static UINT8 vdc_r( running_machine &machine, int which, offs_t offset )
{
	int temp = 0;
	switch(offset & 3)
	{
		case 0x00:
			temp = vdc[which].status;
			vdc[which].status &= ~(VDC_VD | VDC_DV | VDC_DS | VDC_RR | VDC_OR | VDC_CR);
			cputag_set_input_line(machine, "maincpu", 0, CLEAR_LINE);
			break;

		case 0x02:
			temp = vram_read(which, vdc[which].vdc_data[MARR].w * 2 + 0);
			break;

		case 0x03:
			temp = vram_read(which, vdc[which].vdc_data[MARR].w * 2 + 1);
			if ( vdc[which].vdc_register == VxR )
			{
				vdc[which].vdc_data[MARR].w += vdc[which].inc;
			}
			break;
	}
	return (temp);
}

WRITE8_HANDLER( vdc_0_w ) {	vdc_w( space->machine(), 0, offset, data ); }
WRITE8_HANDLER( vdc_1_w ) {	vdc_w( space->machine(), 1, offset, data ); }
READ8_HANDLER( vdc_0_r ) {	return vdc_r( space->machine(), 0, offset ); }
READ8_HANDLER( vdc_1_r ) {	return vdc_r( space->machine(), 1, offset ); }

PALETTE_INIT( vce )
{
	int i;

	for( i = 0; i < 512; i++ )
	{
		int r = (( i >> 3) & 7) << 5;
		int g = (( i >> 6) & 7) << 5;
		int b = (( i     ) & 7) << 5;
		int y = ( (  66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
		palette_set_color_rgb(machine, i, r, g, b);
		palette_set_color_rgb(machine, 512+i, y, y, y);
	}
}

 READ8_HANDLER ( vce_r )
{
	int temp = 0xFF;
	switch(offset & 7)
	{
		case 0x04:	/* color table data (LSB) */
			temp = vce.vce_data[vce.vce_address.w].b.l;
			break;

		case 0x05:	/* color table data (MSB) */
			temp = vce.vce_data[vce.vce_address.w].b.h;
			temp |= 0xFE;
			vce.vce_address.w = (vce.vce_address.w + 1) & 0x01FF;
			break;
	}
	return (temp);
}


WRITE8_HANDLER ( vce_w )
{
	switch(offset & 7)
	{
		case 0x00:	/* control reg. */
			vce.vce_control = data;
			break;

		case 0x02:	/* color table address (LSB) */
			vce.vce_address.b.l = data;
			vce.vce_address.w &= 0x1FF;
			break;

		case 0x03:	/* color table address (MSB) */
			vce.vce_address.b.h = data;
			vce.vce_address.w &= 0x1FF;
			break;

		case 0x04:	/* color table data (LSB) */
			vce.vce_data[vce.vce_address.w].b.l = data;
			break;

		case 0x05:	/* color table data (MSB) */
			vce.vce_data[vce.vce_address.w].b.h = data & 0x01;

			/* bump internal address */
			vce.vce_address.w = (vce.vce_address.w + 1) & 0x01FF;
			break;
	}
}


static void pce_refresh_line(int which, int line, int external_input, UINT8 *drawn, UINT16 *line_buffer)
{
    static const int width_table[4] = {5, 6, 7, 7};

    int scroll_y = ( vdc[which].y_scroll & 0x01FF);
    int scroll_x = (vdc[which].vdc_data[BXR].w & 0x03FF);
    int nt_index;

    /* is virtual map 32 or 64 characters tall ? (256 or 512 pixels) */
    int v_line = (scroll_y) & (vdc[which].vdc_data[MWR].w & 0x0040 ? 0x1FF : 0x0FF);

    /* row within character */
    int v_row = (v_line & 7);

    /* row of characters in BAT */
    int nt_row = (v_line >> 3);

    /* virtual X size (# bits to shift) */
    int v_width =        width_table[(vdc[which].vdc_data[MWR].w >> 4) & 3];

    /* pointer to the name table (Background Attribute Table) in VRAM */
    UINT8 *bat = &(vdc[which].vram[nt_row << (v_width+1)]);

	/* Are we in greyscale mode or in color mode? */
	int color_base = vce.vce_control & 0x80 ? 512 : 0;

    int b0, b1, b2, b3;
    int i0, i1, i2, i3;
    int cell_pattern_index;
    int cell_palette;
    int x, c, i;

    /* character blanking bit */
    if(!(vdc[which].vdc_data[CR].w & CR_BB))
    {
		return;
    }
	else
	{
		int	pixel = 0;
		int phys_x = - ( scroll_x & 0x07 );

		for(i=0;i<(vdc[which].physical_width >> 3) + 1;i++)
		{
			nt_index = (i + (scroll_x >> 3)) & ((2 << (v_width-1))-1);
			nt_index *= 2;

			/* get name table data: */

			/* palette # = index from 0-15 */
			cell_palette = ( bat[nt_index + 1] >> 4 ) & 0x0F;

			/* This is the 'character number', from 0-0x0FFF         */
			/* then it is shifted left 4 bits to form a VRAM address */
			/* and one more bit to convert VRAM word offset to a     */
			/* byte-offset within the VRAM space                     */
			cell_pattern_index = ( ( ( bat[nt_index + 1] << 8 ) | bat[nt_index] ) & 0x0FFF) << 5;

			b0 = vram_read(which, (cell_pattern_index) + (v_row << 1) + 0x00);
			b1 = vram_read(which, (cell_pattern_index) + (v_row << 1) + 0x01);
			b2 = vram_read(which, (cell_pattern_index) + (v_row << 1) + 0x10);
			b3 = vram_read(which, (cell_pattern_index) + (v_row << 1) + 0x11);

			for(x=0;x<8;x++)
			{
				i0 = (b0 >> (7-x)) & 1;
				i1 = (b1 >> (7-x)) & 1;
				i2 = (b2 >> (7-x)) & 1;
				i3 = (b3 >> (7-x)) & 1;
				c = (cell_palette << 4 | i3 << 3 | i2 << 2 | i1 << 1 | i0);

				/* colour #0 always comes from palette #0 */
				if ( ! ( c & 0x0F ) )
					c &= 0x0F;

				if ( phys_x >= 0 && phys_x < vdc[which].physical_width )
				{
					drawn[ pixel ] = c ? 1 : 0;
					if ( c || ! external_input )
						line_buffer[ pixel ] = color_base + vce.vce_data[c].w;
					pixel++;
					if ( vdc[which].physical_width != 512 )
					{
						while ( pixel < ( ( ( phys_x + 1 ) * 512 ) / vdc[which].physical_width ) )
						{
							drawn[ pixel ] = c ? 1 : 0;
							if ( c || ! external_input )
								line_buffer[ pixel ] = color_base + vce.vce_data[c].w;
							pixel++;
						}
					}
				}
				phys_x += 1;
			}
		}
	}
}



static void conv_obj(int which, int i, int l, int hf, int vf, char *buf)
{
	int b0, b1, b2, b3, i0, i1, i2, i3, x;
	int xi;
	int tmp;

	l &= 0x0F;
	if(vf) l = (15 - l);

	tmp = l + ( i << 5);

	b0 = vram_read(which, (tmp + 0x00)<<1);
	b0 |= vram_read(which, ((tmp + 0x00)<<1)+1)<<8;
	b1 = vram_read(which, (tmp + 0x10)<<1);
	b1 |= vram_read(which, ((tmp + 0x10)<<1)+1)<<8;
	b2 = vram_read(which, (tmp + 0x20)<<1);
	b2 |= vram_read(which, ((tmp + 0x20)<<1)+1)<<8;
	b3 = vram_read(which, (tmp + 0x30)<<1);
	b3 |= vram_read(which, ((tmp + 0x30)<<1)+1)<<8;

	for(x=0;x<16;x++)
	{
		if(hf) xi = x; else xi = (15 - x);
		i0 = (b0 >> xi) & 1;
		i1 = (b1 >> xi) & 1;
		i2 = (b2 >> xi) & 1;
		i3 = (b3 >> xi) & 1;
		buf[x] = (i3 << 3 | i2 << 2 | i1 << 1 | i0);
	}
}

static void pce_refresh_sprites(running_machine &machine, int which, int line, UINT8 *drawn, UINT16 *line_buffer)
{
    int i;
	UINT8 sprites_drawn = 0;

	/* Are we in greyscale mode or in color mode? */
	int color_base = vce.vce_control & 0x80 ? 512 : 0;

	/* count up: Highest priority is Sprite 0 */
	for(i = 0; i < 64; i++)
	{
		static const int cgy_table[] = {16, 32, 64, 64};

		int obj_y = (vdc[which].sprite_ram[(i << 2) + 0] & 0x03FF) - 64;
		int obj_x = (vdc[which].sprite_ram[(i << 2) + 1] & 0x03FF) - 32;
		int obj_i = (vdc[which].sprite_ram[(i << 2) + 2] & 0x07FE);
		int obj_a = (vdc[which].sprite_ram[(i << 2) + 3]);
		int cgx   = (obj_a >> 8) & 1;   /* sprite width */
		int cgy   = (obj_a >> 12) & 3;  /* sprite height */
		int hf    = (obj_a >> 11) & 1;  /* horizontal flip */
		int vf    = (obj_a >> 15) & 1;  /* vertical flip */
		int palette = (obj_a & 0x000F);
		int priority = (obj_a >> 7) & 1;
		int obj_h = cgy_table[cgy];
		int obj_l = (line - obj_y);
		int cgypos;
		char buf[16];

		if ((obj_y == -64) || (obj_y > line)) continue;
		if ((obj_x == -32) || (obj_x >= vdc[which].physical_width)) continue;

		/* no need to draw an object that's ABOVE where we are. */
		if((obj_y + obj_h) < line) continue;

		/* If CGX is set, bit 0 of sprite pattern index is forced to 0 */
		if ( cgx )
			obj_i &= ~2;

		/* If CGY is set to 1, bit 1 of the sprite pattern index is forced to 0. */
		if ( cgy & 1 )
			obj_i &= ~4;

		/* If CGY is set to 2 or 3, bit 1 and 2 of the sprite pattern index are forced to 0. */
		if ( cgy & 2 )
			obj_i &= ~12;

		if (obj_l < obj_h)
		{

			sprites_drawn++;
			if(sprites_drawn > 16)
			{
				if(vdc[which].vdc_data[CR].w & CR_OV)
				{
					/* note: flag is set only if irq is taken, Mizubaku Daibouken relies on this behaviour */
					vdc[which].status |= VDC_OR;
					cputag_set_input_line(machine, "maincpu", 0, ASSERT_LINE);
				}
				continue;  /* Should cause an interrupt */
			}

			cgypos = (obj_l >> 4);
			if(vf) cgypos = ((obj_h - 1) >> 4) - cgypos;

			if(cgx == 0)
			{
				int x;
				int pixel_x = ( ( obj_x * 512 ) / vdc[which].physical_width );

				conv_obj(which, obj_i + (cgypos << 2), obj_l, hf, vf, buf);

				for(x = 0; x < 16; x++)
				{
					if(((obj_x + x) < (vdc[which].physical_width)) && ((obj_x + x) >= 0))
					{
						if ( buf[x] )
						{
							if( drawn[pixel_x] < 2 )
							{
								if( priority || drawn[pixel_x] == 0 )
								{
									line_buffer[pixel_x] = color_base + vce.vce_data[0x100 + (palette << 4) + buf[x]].w;
									if ( vdc[which].physical_width != 512 )
									{
										int dp = 1;
										while ( pixel_x + dp < ( ( ( obj_x + x + 1 ) * 512 ) / vdc[which].physical_width ) )
										{
											drawn[pixel_x + dp] = i + 2;
											line_buffer[pixel_x + dp] = color_base + vce.vce_data[0x100 + (palette << 4) + buf[x]].w;
											dp++;
										}
									}
								}
								drawn[pixel_x] = i + 2;
							}
							/* Check for sprite #0 collision */
							else if (drawn[pixel_x] == 2)
							{
								if(vdc[which].vdc_data[CR].w & CR_CC)
									cputag_set_input_line(machine, "maincpu", 0, ASSERT_LINE);
								vdc[which].status |= VDC_CR;
							}
						}
					}
					if ( vdc[which].physical_width != 512 )
					{
						pixel_x = ( ( obj_x + x + 1 ) * 512 ) / vdc[which].physical_width;
					}
					else
					{
						pixel_x += 1;
					}
				}
			}
			else
			{
				int x;
				int pixel_x = ( ( obj_x * 512 ) / vdc[which].physical_width );

				conv_obj(which, obj_i + (cgypos << 2) + (hf ? 2 : 0), obj_l, hf, vf, buf);

				for(x = 0; x < 16; x++)
				{
					if(((obj_x + x) < (vdc[which].physical_width)) && ((obj_x + x) >= 0))
					{
						if ( buf[x] )
						{
							if( drawn[pixel_x] < 2 )
							{
								if ( priority || drawn[pixel_x] == 0 )
								{
									line_buffer[pixel_x] = color_base + vce.vce_data[0x100 + (palette << 4) + buf[x]].w;
									if ( vdc[which].physical_width != 512 )
									{
										int dp = 1;
										while ( pixel_x + dp < ( ( ( obj_x + x + 1 ) * 512 ) / vdc[which].physical_width ) )
										{
											drawn[pixel_x + dp] = i + 2;
											line_buffer[pixel_x + dp] = color_base + vce.vce_data[0x100 + (palette << 4) + buf[x]].w;
											dp++;
										}
									}
								}
								drawn[pixel_x] = i + 2;
							}
							/* Check for sprite #0 collision */
							else if ( drawn[pixel_x] == 2 )
							{
								if(vdc[which].vdc_data[CR].w & CR_CC)
									cputag_set_input_line(machine, "maincpu", 0, ASSERT_LINE);
								vdc[which].status |= VDC_CR;
							}
						}
					}
					if ( vdc[which].physical_width != 512 )
					{
						pixel_x = ( ( obj_x + x + 1 ) * 512 ) / vdc[which].physical_width;
					}
					else
					{
						pixel_x += 1;
					}
				}

				/* 32 pixel wide sprites are counted as 2 sprites and the right half
                   is only drawn if there are 2 open slots.
                */
				sprites_drawn++;
				if( sprites_drawn > 16 )
				{
					if(vdc[which].vdc_data[CR].w&CR_OV)
					{
						/* note: flag is set only if irq is taken, Mizubaku Daibouken relies on this behaviour */
						vdc[which].status |= VDC_OR;
						cputag_set_input_line(machine, "maincpu", 0, ASSERT_LINE);
					}
				}
				else
				{
					conv_obj(which, obj_i + (cgypos << 2) + (hf ? 0 : 2), obj_l, hf, vf, buf);
					for(x = 0; x < 16; x++)
					{
						if(((obj_x + 0x10 + x) < (vdc[which].physical_width)) && ((obj_x + 0x10 + x) >= 0))
						{
							if ( buf[x] )
							{
								if( drawn[pixel_x] < 2 )
								{
									if( priority || drawn[pixel_x] == 0 )
									{
										line_buffer[pixel_x] = color_base + vce.vce_data[0x100 + (palette << 4) + buf[x]].w;
										if ( vdc[which].physical_width != 512 )
										{
											int dp = 1;
											while ( pixel_x + dp < ( ( ( obj_x + x + 17 ) * 512 ) / vdc[which].physical_width ) )
											{
												drawn[pixel_x + dp] = i + 2;
												line_buffer[pixel_x + dp] = color_base + vce.vce_data[0x100 + (palette << 4) + buf[x]].w;
												dp++;
											}
										}
									}
									drawn[pixel_x] = i + 2;
								}
								/* Check for sprite #0 collision */
								else if ( drawn[pixel_x] == 2 )
								{
									if(vdc[which].vdc_data[CR].w & CR_CC)
										cputag_set_input_line(machine, "maincpu", 0, ASSERT_LINE);
									vdc[which].status |= VDC_CR;
								}
							}
						}
						if ( vdc[which].physical_width != 512 )
						{
							pixel_x = ( ( obj_x + x + 17 ) * 512 ) / vdc[which].physical_width;
						}
						else
						{
							pixel_x += 1;
						}
					}
				}
			}
		}
	}
}

static void vdc_do_dma(running_machine &machine, int which)
{
	int src = vdc[which].vdc_data[SOUR].w;
	int dst = vdc[which].vdc_data[DESR].w;
	int len = vdc[which].vdc_data[LENR].w;

	int did = (vdc[which].vdc_data[DCR].w >> 3) & 1;
	int sid = (vdc[which].vdc_data[DCR].w >> 2) & 1;
	int dvc = (vdc[which].vdc_data[DCR].w >> 1) & 1;

	do {
		UINT8 l, h;

		l = vram_read(which, src<<1);
		h = vram_read(which, (src<<1) + 1);

		vram_write(which, dst<<1,l);
		vram_write(which, 1+(dst<<1),h);

		if(sid) src = (src - 1) & 0xFFFF;
		else	src = (src + 1) & 0xFFFF;

		if(did) dst = (dst - 1) & 0xFFFF;
		else	dst = (dst + 1) & 0xFFFF;

		len = (len - 1) & 0xFFFF;

	} while (len != 0xFFFF);

	vdc[which].status |= VDC_DV;
	vdc[which].vdc_data[SOUR].w = src;
	vdc[which].vdc_data[DESR].w = dst;
	vdc[which].vdc_data[LENR].w = len;
	if(dvc)
	{
		cputag_set_input_line(machine, "maincpu", 0, ASSERT_LINE);
	}

}

static void vpc_update_prio_map( void )
{
	int i;

	for( i = 0; i < 512; i++ )
	{
		vpc.prio_map[i] = 0;
		if ( vpc.window1.w < 0x40 || i > vpc.window1.w )
		{
			vpc.prio_map[i] |= 1;
		}
		if ( vpc.window2.w < 0x40 || i > vpc.window2.w )
		{
			vpc.prio_map[i] |= 2;
		}
	}
}

WRITE8_HANDLER( vpc_w )
{
//if ( offset < 2 )
//printf("VPC write offset %02X, data %02X\n", offset, data );
	switch( offset & 0x07 )
	{
	case 0x00:	/* Priority register #0 */
		vpc.priority.b.l = data;
		vpc.vpc_prio[0].prio = ( data >> 2 ) & 3;
		vpc.vpc_prio[0].vdc0_enabled = data & 1;
		vpc.vpc_prio[0].vdc1_enabled = data & 2;
		vpc.vpc_prio[1].prio = ( data >> 6 ) & 3;
		vpc.vpc_prio[1].vdc0_enabled = data & 0x10;
		vpc.vpc_prio[1].vdc1_enabled = data & 0x20;
		break;
	case 0x01:	/* Priority register #1 */
		vpc.priority.b.h = data;
		vpc.vpc_prio[2].prio = ( data >> 2 ) & 3;
		vpc.vpc_prio[2].vdc0_enabled = data & 1;
		vpc.vpc_prio[2].vdc1_enabled = data & 2;
		vpc.vpc_prio[3].prio = ( data >> 6 ) & 3;
		vpc.vpc_prio[3].vdc0_enabled = data & 0x10;
		vpc.vpc_prio[3].vdc1_enabled = data & 0x20;
		break;
	case 0x02:	/* Window 1 LSB */
		vpc.window1.b.l = data;
		vpc_update_prio_map();
		break;
	case 0x03:	/* Window 1 MSB */
		vpc.window1.b.h = data & 3;
		vpc_update_prio_map();
		break;
	case 0x04:	/* Window 2 LSB */
		vpc.window2.b.l = data;
		vpc_update_prio_map();
		break;
	case 0x05:	/* Window 2 MSB */
		vpc.window2.b.h = data & 3;
		vpc_update_prio_map();
		break;
	case 0x06:	/* VDC I/O select */
		vpc.vdc_select = data & 1;
		break;
	}
}

READ8_HANDLER( vpc_r )
{
	UINT8 data = 0;
	switch( offset & 0x07 )
	{
	case 0x00:  /* Priority register #0 */
		data = vpc.priority.b.l;
		break;
	case 0x01:  /* Priority register #1 */
		data = vpc.priority.b.h;
		break;
	case 0x02:  /* Window 1 LSB */
		data = vpc.window1.b.l;
		break;
	case 0x03:  /* Window 1 MSB; high bits are 0 or 1? */
		data = vpc.window1.b.h;
		break;
	case 0x04:  /* Window 2 LSB */
		data = vpc.window2.b.l;
		break;
	case 0x05:  /* Window 2 MSB; high bits are 0 or 1? */
		data = vpc.window2.b.h;
		break;
	}
	return data;
}

static void vpc_init( running_machine &machine )
{
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	vpc_w( space, 0, 0x11 );
	vpc_w( space, 1, 0x11 );
	vpc.window1.w = 0;
	vpc.window2.w = 0;
	vpc.vdc_select = 0;
}

WRITE8_HANDLER( sgx_vdc_w )
{
	if ( vpc.vdc_select )
	{
		vdc_1_w( space, offset, data );
	}
	else
	{
		vdc_0_w( space, offset, data );
	}
}

READ8_HANDLER( sgx_vdc_r )
{
	return ( vpc.vdc_select ) ? vdc_1_r( space, offset ) : vdc_0_r( space, offset );
}


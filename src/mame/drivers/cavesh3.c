/* Cave SH3 ( CAVE CV1000-B ) */
/* skeleton placeholder driver */

/*
ToDo:

Eeprom
NAND/Flash Writing and Saving! (DeathSmiles uses it for Unlock data)
Improve Blending precision?
Sound
Why does mmmbnk write to 0 on startup, is it related to the broken GFX you see?
What is mmpork checking when it reports 'ERROR'
General SH3 cleanups, verify dividers and such
Speedups? (without breaking overall timing)
Solid White BG on DS title screen? Lack of BG / GFX clear in MMP boot/test mode?

*/

#include "emu.h"
#include "cpu/sh4/sh4.h"
#include "cpu/sh4/sh3comn.h"
#include "profiler.h"

class cavesh3_state : public driver_device
{
public:
	cavesh3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
};

/***************************************************************************
                                Video Hardware
***************************************************************************/

UINT16* cavesh3_ram16;

struct _clr_t
{
	INT8 r,g,b;
};
typedef struct _clr_t clr_t;

// r5g5b5 ro clr_t
INLINE void pen_to_clr(UINT16 pen, clr_t *clr)
{
	clr->r = (pen >> 10) & 0x1f;
	clr->g = (pen >>  5) & 0x1f;
	clr->b = (pen      ) & 0x1f;
}

// convert separate r,g,b biases (0..80..ff) to clr_t (-1f..0..1f)
INLINE void tint_to_clr(UINT8 r, UINT8 g, UINT8 b, clr_t *clr)
{
	clr->r	=	(r - 0x80) / 4;
	clr->g	=	(g - 0x80) / 4;
	clr->b	=	(b - 0x80) / 4;

	if (clr->r < -0x1f)	clr->r = -0x1f;
	if (clr->g < -0x1f)	clr->g = -0x1f;
	if (clr->b < -0x1f)	clr->b = -0x1f;
}

// convert alpha factor (0..ff) to clr_t (0..1f)
INLINE void alpha_to_clr(UINT8 alpha, clr_t *clr)
{
	clr->r	=	alpha / 8;
	clr->g	=	alpha / 8;
	clr->b	=	alpha / 8;
}

// clamp to 0..1f
INLINE INT8 clamp(INT8 comp)
{
	if (comp > 0x1f)	return 0x1f;
	else if (comp < 0)	return 0;
	else				return comp;
}

// clr_t to r5g5b5
INLINE UINT16 clr_to_pen(const clr_t *clr)
{
	return (clr->r << 10) | (clr->g << 5) | clr->b;
//return 0x8000 | (clr->r << 10) | (clr->g << 5) | clr->b;
}

// add clrs
INLINE void clr_add(const clr_t *clr0, const clr_t *clr1, clr_t *clr)
{
	clr->r = clamp(clr0->r + clr1->r);
	clr->g = clamp(clr0->g + clr1->g);
	clr->b = clamp(clr0->b + clr1->b);
}

// multiply clrs
INLINE void clr_mul(const clr_t *clr0, const clr_t *clr1, clr_t *clr)
{
	clr->r = clamp(clr0->r * clr1->r / 0x1f);
	clr->g = clamp(clr0->g * clr1->g / 0x1f);
	clr->b = clamp(clr0->b * clr1->b / 0x1f);
}

INLINE char mode_name(UINT8 mode)
{
	switch( mode )
	{
		case 0:	return 'A';	// +alpha
		case 1:	return 'S';	// +source
		case 2:	return 'D';	// +dest
		case 3:	return '*';	// *
		case 4:	return 'a';	// -alpha
		case 5:	return 's';	// -source
		case 6:	return 'd';	// -dest
		case 7:	return '-';	// *
	}
	return '?';
}

// (1|s|d) * s_factor * s + (1|s|d) * d_factor * d
// 0: +alpha
// 1: +source
// 2: +dest
// 3: *
// 4: -alpha
// 5: -source
// 6: -dest
// 7: *
INLINE void cavesh_clr_select(const clr_t *s_clr, const clr_t *d_clr, const clr_t *a_clr, UINT8 mode, clr_t *clr)
{
	switch( mode )
	{
		case 0:	// +alpha
			clr->r = a_clr->r;
			clr->g = a_clr->g;
			clr->b = a_clr->b;
			return;

		case 1:	// +source
			clr->r = s_clr->r;
			clr->g = s_clr->g;
			clr->b = s_clr->b;
			return;

		case 2:	// +dest
			clr->r = d_clr->r;
			clr->g = d_clr->g;
			clr->b = d_clr->b;
			return;

		case 3:	// *
			clr->r = 0x1f;
			clr->g = 0x1f;
			clr->b = 0x1f;
			return;

		case 4:	// -alpha
			clr->r = a_clr->r^0x1f;
			clr->g = a_clr->g^0x1f;
			clr->b = a_clr->b^0x1f;
			return;

		case 5:	// -source
			clr->r = s_clr->r^0x1f;
			clr->g = s_clr->g^0x1f;
			clr->b = s_clr->b^0x1f;
			return;

		case 6:	// -dest
			clr->r = d_clr->r^0x1f;
			clr->g = d_clr->g^0x1f;
			clr->b = d_clr->b^0x1f;
			return;

		default:
		case 7:	// *
			clr->r = 0x1f;
			clr->g = 0x1f;
			clr->b = 0x1f;
			return;
	}
}


static UINT32 cavesh_gfx_addr;
static UINT32 cavesh_gfx_scroll_0_x, cavesh_gfx_scroll_0_y;
static UINT32 cavesh_gfx_scroll_1_x, cavesh_gfx_scroll_1_y;

static int cavesh_gfx_size;
static bitmap_t *cavesh_bitmaps[1];

static VIDEO_START( cavesh3 )
{
	cavesh_gfx_size	= 0x2000 * 0x1000;
	cavesh_bitmaps[0]	=	auto_bitmap_alloc(machine, 0x2000, 0x1000, BITMAP_FORMAT_INDEXED16);
}

INLINE UINT32 GFX_OFFSET( UINT32 x0, UINT32 y0, UINT32 x, UINT32 y )
{
	// correct
	return	((x0 + x) & 0x1fff) +
			((y0 + y) & 0x0fff) * 0x2000;
}

INLINE void draw_sprite(
	bitmap_t *bitmap, const rectangle *clip, UINT16 *gfx, int gfx_size,

	int src_p,int src_x,int src_y, int dst_x,int dst_y, int dimx,int dimy, int flipx,int flipy,

	int blend, clr_t *s_alpha_clr, int s_mode, clr_t *d_alpha_clr, int d_mode,

	int tint, clr_t *tint_clr
)
{

	//logerror("draw sprite %04x %04x - %04x %04x\n", dst_x, dst_y, dimx, dimy);

	int x,y, xf,yf;
	clr_t s_clr, d_clr, clr0, clr1;
	UINT16 pen;
	UINT16 *bmp;

	if (flipx)	{	xf = -1;	src_x += (dimx-1);	}
	else		{	xf = +1;						}

	if (flipy)	{	yf = -1;	src_y += (dimy-1);	}
	else		{	yf = +1;						}

	int starty = 0;

	if (dst_y < clip->min_y)
		starty = clip->min_y - dst_y;

	for (y = starty; y < dimy; y++)
	{
		if ((dst_y + y) > clip->max_y)
			return;

		bmp = BITMAP_ADDR16(bitmap, dst_y + y, 0);

		int startx = 0;

		if (dst_x < clip->min_x)
			startx = clip->min_x - dst_x;

		for (x = startx; x < dimx; x++)
		{
			if ((dst_x + x) > clip->max_x)
				break;

			pen = gfx[GFX_OFFSET(src_x,src_y, xf * x, yf * y) & (gfx_size-1)];

			if ((tint) ||(pen & 0x8000)) // (tint) not quite right but improves deathsml
			{
				// convert source to clr
				pen_to_clr(pen, &s_clr);

				// apply clr bias to source
//              if (tint)
					clr_add(&s_clr, tint_clr, &s_clr);

				if (blend)
				{
					// convert destination to clr
					pen_to_clr(bmp[dst_x + x], &d_clr);

					// transform source
					cavesh_clr_select(&s_clr, &d_clr, s_alpha_clr, s_mode, &clr0);
					clr_mul(&clr0, &s_clr, &clr0);

					// transform destination
					cavesh_clr_select(&s_clr, &d_clr, d_alpha_clr, d_mode, &clr1);
					clr_mul(&clr1, &d_clr, &clr1);

					// blend (add) into source
					clr_add(&clr0, &clr1, &s_clr);
				}

				// write result
				bmp[dst_x + x] = clr_to_pen(&s_clr)|(pen&0x8000);
			}

		}
	}
}



INLINE UINT16 READ_NEXT_WORD(address_space &space, offs_t *addr)
{
//  UINT16 data = space.read_word(*addr); // going through the memory system is 'more correct' but noticably slower
	UINT16 data =  cavesh3_ram16[((*addr&(0x7fffff))>>1)^3]; // this probably needs to be made endian safe tho
	*addr += 2;

//  printf("data %04x\n", data);
	return data;
}

INLINE void cavesh_gfx_copy(address_space &space, offs_t *addr)
{
	UINT32 x,y, dst_p,dst_x,dst_y, dimx,dimy;
	UINT16 *dst;

	// 0x20000000
	READ_NEXT_WORD(space, addr);
	READ_NEXT_WORD(space, addr);

	// 0x99999999
	READ_NEXT_WORD(space, addr);
	READ_NEXT_WORD(space, addr);

	dst_x = READ_NEXT_WORD(space, addr);
	dst_y = READ_NEXT_WORD(space, addr);

	dst_p = 0;
	dst_x &= 0x1fff;
	dst_y &= 0x0fff;

	dimx = (READ_NEXT_WORD(space, addr) & 0x1fff) + 1;
	dimy = (READ_NEXT_WORD(space, addr) & 0x0fff) + 1;

	logerror("GFX COPY: DST %02X,%02X,%03X DIM %02X,%03X\n", dst_p,dst_x,dst_y, dimx,dimy);

	for (y = 0; y < dimy; y++)
	{
		dst = BITMAP_ADDR16(cavesh_bitmaps[0], dst_y + y, 0);

		for (x = 0; x < dimx; x++)
		{
			 dst[dst_x + x] = READ_NEXT_WORD(space, addr);
		}
	}
}

INLINE void cavesh_gfx_draw(address_space &space, offs_t *addr)
{
	int	x,y, dimx,dimy, flipx,flipy, src_p;
	int tint,blend, s_alpha,s_mode, d_alpha,d_mode;
	clr_t tint_clr, s_alpha_clr, d_alpha_clr;

	UINT16 attr		=	READ_NEXT_WORD(space, addr);
	UINT16 alpha	=	READ_NEXT_WORD(space, addr);
	UINT16 src_x	=	READ_NEXT_WORD(space, addr);
	UINT16 src_y	=	READ_NEXT_WORD(space, addr);
	UINT16 dst_x	=	READ_NEXT_WORD(space, addr);
	UINT16 dst_y	=	READ_NEXT_WORD(space, addr);
	UINT16 w		=	READ_NEXT_WORD(space, addr);
	UINT16 h		=	READ_NEXT_WORD(space, addr);
	UINT16 tint_r	=	READ_NEXT_WORD(space, addr);
	UINT16 tint_gb	=	READ_NEXT_WORD(space, addr);

	// 0: +alpha
	// 1: +source
	// 2: +dest
	// 3: *
	// 4: -alpha
	// 5: -source
	// 6: -dest
	// 7: *

	d_mode	=	 attr & 0x0007;
	s_mode	=	(attr & 0x0070) >> 4;

	tint	=	 !(attr & 0x0100);
	blend	=	   attr & 0x0200;

	flipy	=	 attr & 0x0400;
	flipx	=	 attr & 0x0800;

	d_alpha	=	 alpha & 0x00ff;
	s_alpha	=	(alpha & 0xff00) >> 8;

	src_p	=	0;
	src_x	=	src_x & 0x1fff;
	src_y	=	src_y & 0x0fff;


	x		=	(dst_x & 0x7fff) - (dst_x & 0x8000);
	y		=	(dst_y & 0x7fff) - (dst_y & 0x8000);

	dimx	=	(w & 0x1fff) + 1;
	dimy	=	(h & 0x0fff) + 1;

	// convert parameters to clr

	tint_to_clr(tint_r & 0x00ff, (tint_gb >>  8) & 0xff, tint_gb & 0xff, &tint_clr);

	alpha_to_clr(s_alpha, &s_alpha_clr);
	alpha_to_clr(d_alpha, &d_alpha_clr);

	// draw
	draw_sprite(
		cavesh_bitmaps[0], &cavesh_bitmaps[0]->cliprect, BITMAP_ADDR16(cavesh_bitmaps[0], 0,0),cavesh_gfx_size,
		src_p,src_x,src_y, x,y, dimx,dimy, flipx,flipy,
		blend, &s_alpha_clr, s_mode, &d_alpha_clr, d_mode,
		tint, &tint_clr
	);

}

// Death Smiles has bad text with wrong clip sizes, must clip to screen size.
static void cavesh_gfx_exec(address_space &space)
{
	UINT16 layer = 0;

	offs_t addr = cavesh_gfx_addr & 0x1fffffff;

//  logerror("GFX EXEC: %08X\n", addr);

	cavesh_bitmaps[0]->cliprect.min_x = cavesh_gfx_scroll_1_x;
	cavesh_bitmaps[0]->cliprect.min_y = cavesh_gfx_scroll_1_y;
	cavesh_bitmaps[0]->cliprect.max_x = cavesh_bitmaps[0]->cliprect.min_x + 320-1;
	cavesh_bitmaps[0]->cliprect.max_y = cavesh_bitmaps[0]->cliprect.min_y + 240-1;

	while (1)
	{
		UINT16 data = READ_NEXT_WORD(space, &addr);

		switch( data & 0xf000 )
		{
			case 0x0000:
			case 0xf000:
				return;

			case 0xc000:
				data = READ_NEXT_WORD(space, &addr);
				layer = data ? 1 : 0;

				if (layer)
				{
					cavesh_bitmaps[0]->cliprect.min_x = cavesh_gfx_scroll_1_x;
					cavesh_bitmaps[0]->cliprect.min_y = cavesh_gfx_scroll_1_y;
					cavesh_bitmaps[0]->cliprect.max_x = cavesh_bitmaps[0]->cliprect.min_x + 320-1;
					cavesh_bitmaps[0]->cliprect.max_y = cavesh_bitmaps[0]->cliprect.min_y + 240-1;
				}
				else
				{
					cavesh_bitmaps[0]->cliprect.min_x = 0;
					cavesh_bitmaps[0]->cliprect.min_y = 0;
					cavesh_bitmaps[0]->cliprect.max_x = 0x2000-1;
					cavesh_bitmaps[0]->cliprect.max_y = 0x1000-1;
				}
				break;

			case 0x2000:
				addr -= 2;
				cavesh_gfx_copy(space, &addr);
				break;

			case 0x1000:
				addr -= 2;
				cavesh_gfx_draw(space, &addr);
				break;

			default:
				popmessage("GFX op = %04X", data);
				return;
		}
	}
}


static READ32_HANDLER( cavesh_gfx_ready_r )
{
	return 0x00000010;
}

static WRITE32_HANDLER( cavesh_gfx_exec_w )
{
	if ( ACCESSING_BITS_0_7 )
	{
		if (data & 1)
		{
			g_profiler.start(PROFILER_USER1);
			cavesh_gfx_exec(*space);
			g_profiler.stop();
		}
	}
}



static SCREEN_UPDATE( cavesh3 )
{
	int scroll_0_x, scroll_0_y;
	//int scroll_1_x, scroll_1_y;

	bitmap_fill(bitmap, cliprect, 0);

	scroll_0_x = -cavesh_gfx_scroll_0_x;
	scroll_0_y = -cavesh_gfx_scroll_0_y;
	//scroll_1_x = -cavesh_gfx_scroll_1_x;
	//scroll_1_y = -cavesh_gfx_scroll_1_y;

	//logerror("SCREEN UPDATE\n");

	copyscrollbitmap_trans(bitmap, cavesh_bitmaps[0], 1,&scroll_0_x, 1,&scroll_0_y, cliprect, 0x8000);

	return 0;
}


static READ32_HANDLER( cavesh3_blitter_r )
{
	switch (offset*4)
	{
		case 0x10:
			return cavesh_gfx_ready_r(space,offset,mem_mask);

		case 0x24:
			return 0xffffffff;

		case 0x28:
			return 0xffffffff;

		case 0x50:
			return input_port_read(space->machine(), "DSW");

		default:
			logerror("unknowncavesh3_blitter_r %08x %08x\n", offset*4, mem_mask);
			break;

	}
	return 0;
}

static WRITE32_HANDLER( cavesh3_blitter_w )
{
	switch (offset*4)
	{
		case 0x04:
			cavesh_gfx_exec_w(space,offset,data,mem_mask);
			break;

		case 0x08:
			COMBINE_DATA(&cavesh_gfx_addr);
			break;

		case 0x14:
			COMBINE_DATA(&cavesh_gfx_scroll_0_x);
			break;

		case 0x18:
			COMBINE_DATA(&cavesh_gfx_scroll_0_y);
			break;

		case 0x40:
			COMBINE_DATA(&cavesh_gfx_scroll_1_x);
			break;

		case 0x44:
			COMBINE_DATA(&cavesh_gfx_scroll_1_y);
			break;

	}
}



static READ64_HANDLER( ymz2770c_z_r )
{
	UINT64 ret = space->machine().rand();

	return ret ^ (ret<<32);
}

static WRITE64_HANDLER( ymz2770c_z_w )
{

}



// FLASH

#define FLASH_PAGE_SIZE	(2048+64)

UINT8 flash_page_data[FLASH_PAGE_SIZE];

typedef enum							{ STATE_IDLE = 0,	STATE_READ,		STATE_READ_ID,	STATE_READ_STATUS, STATE_BLOCK_ERASE, STATE_PAGE_PROGRAM	} flash_state_t;
static const char *flash_state_name[] =	{ "IDLE",			"READ",			"READ_ID",		"READ_STATUS",     "BLOCK ERASE",     "PAGE PROGRAM"		};

static flash_state_t flash_state;

static UINT8 flash_enab;

static UINT8 flash_cmd_seq;
static UINT32 flash_cmd_prev;

static UINT8 flash_addr_seq;
static UINT8 flash_read_seq;

static UINT16 flash_row, flash_col;
static UINT16 flash_page_addr;
static UINT16 flash_page_index;

static void flash_hard_reset(running_machine &machine)
{
//  logerror("%08x FLASH: RESET\n", cpuexec_describe_context(machine));

	flash_state = STATE_READ;

	flash_cmd_prev = -1;
	flash_cmd_seq = 0;

	flash_addr_seq = 0;
	flash_read_seq = 0;

	flash_row = 0;
	flash_col = 0;

	memset(flash_page_data, 0, FLASH_PAGE_SIZE);
	flash_page_addr = 0;
	flash_page_index = 0;
}

static WRITE8_HANDLER( flash_enab_w )
{
	logerror("%08x FLASH: enab = %02X\n", cpu_get_pc(&space->device()), data);
	//flash_enab = data;
	flash_enab = 1; // todo, why does it get turned off again instantly?
}

static void flash_change_state(running_machine &machine, flash_state_t state)
{
	flash_state = state;

	flash_cmd_prev = -1;
	flash_cmd_seq = 0;

	flash_read_seq = 0;
	flash_addr_seq = 0;

	logerror("flash_change_state - FLASH: state = %s\n", flash_state_name[state]);
}

static WRITE8_HANDLER( flash_cmd_w )
{
	if (!flash_enab)
		return;

	logerror("%08x FLASH: cmd = %02X (prev = %02X)\n", cpu_get_pc(&space->device()), data, flash_cmd_prev);

	if (flash_cmd_prev == -1)
	{
		flash_cmd_prev = data;

		switch (data)
		{
			case 0x00:	// READ
				flash_addr_seq = 0;
				break;

			case 0x60:  // BLOCK ERASE
				flash_addr_seq = 0;
				break;

			case 0x70:	// READ STATUS
				flash_change_state( space->machine(), STATE_READ_STATUS );
				break;

			case 0x80:	// PAGE / CACHE PROGRAM
				flash_addr_seq = 0;
				flash_page_addr = 0;// flash_col;
				//flash_page_index = flash_row;


				break;

			case 0x90:	// READ ID
				flash_change_state( space->machine(), STATE_READ_ID );
				break;

			case 0xff:	// RESET
				flash_change_state( space->machine(), STATE_IDLE );
				break;

			default:
			{
				logerror("%08x FLASH: unknown cmd1 = %02X\n", cpu_get_pc(&space->device()), data);
			}
		}
	}
	else
	{
		switch (flash_cmd_prev)
		{
			case 0x00:	// READ
				if (data == 0x30)
				{
					UINT8 *region = space->machine().region( "game" )->base();

					memcpy(flash_page_data, region + flash_row * FLASH_PAGE_SIZE, FLASH_PAGE_SIZE);
					flash_page_addr = flash_col;
					flash_page_index = flash_row;

					flash_change_state( space->machine(), STATE_READ );

					logerror("%08x FLASH: caching page = %04X\n", cpu_get_pc(&space->device()), flash_row);
				}
				break;

			case 0x60: // BLOCK ERASE
				if (data==0xd0)
				{

					 flash_change_state( space->machine(), STATE_BLOCK_ERASE );
					 //logerror("%08x FLASH: caching page = %04X\n", cpu_get_pc(&space->device()), flash_row);
				}
				else
				{
					logerror("unexpected 2nd command after BLOCK ERASE\n");
				}
				break;
			case 0x80:
				if (data==0x10)
				{
				//  UINT8 *region = space->machine().region( "game" )->base();
					flash_change_state( space->machine(), STATE_PAGE_PROGRAM );
					flash_page_addr = flash_col;
					flash_page_index = flash_row;
					// don't do this until it's verified as OK
					//memcpy(region + flash_row * FLASH_PAGE_SIZE, flash_page_data, FLASH_PAGE_SIZE);

				}
				else
				{
					logerror("unexpected 2nd command after SPAGE PROGRAM\n");
				}
				break;


			default:
			{
				logerror("%08x FLASH: unknown cmd2 = %02X (cmd1 = %02X)\n", cpu_get_pc(&space->device()), data, flash_cmd_prev);
			}
		}
	}
}

static WRITE8_HANDLER( flash_data_w ) // death smiles
{
	if (!flash_enab)
		return;

	logerror("flash data write %04x\n", flash_page_addr);
	flash_page_data[flash_page_addr] = data;
	flash_page_addr++;
}

static WRITE8_HANDLER( flash_addr_w )
{
	if (!flash_enab)
		return;

	logerror("%08x FLASH: addr = %02X (seq = %02X)\n", cpu_get_pc(&space->device()), data, flash_addr_seq);

	switch( flash_addr_seq++ )
	{
		case 0:
			flash_col = (flash_col & 0xff00) | data;
			break;
		case 1:
			flash_col = (flash_col & 0x00ff) | (data << 8);
			break;
		case 2:
			flash_row = (flash_row & 0xff00) | data;
			break;
		case 3:
			flash_row = (flash_row & 0x00ff) | (data << 8);
			flash_addr_seq = 0;
			break;
	}
}

static READ8_HANDLER( flash_io_r )
{
	UINT8 data = 0x00;
	UINT32 old;

	if (!flash_enab)
		return 0xff;

	switch (flash_state)
	{
		case STATE_READ_ID:
			old = flash_read_seq;

			switch( flash_read_seq++ )
			{
				case 0:
					data = 0xEC;	// Manufacturer
					break;
				case 1:
					data = 0xF1;	// Device
					break;
				case 2:
					data = 0x00;	// XX
					break;
				case 3:
					data = 0x15;	// Flags
					flash_read_seq = 0;
					break;
			}

			logerror("%08x FLASH: read %02X from id(%02X)\n", cpu_get_pc(&space->device()), data, old);
			break;

		case STATE_READ:
			if (flash_page_addr > FLASH_PAGE_SIZE-1)
				flash_page_addr = FLASH_PAGE_SIZE-1;

			old = flash_page_addr;

			data = flash_page_data[flash_page_addr++];

			//logerror("%08x FLASH: read data %02X from addr %03X (page %04X)\n", cpu_get_pc(&space->device()), data, old, flash_page_index);
			break;

		case STATE_READ_STATUS:
			// bit 7 = writeable, bit 6 = ready, bit 5 = ready/true ready, bit 1 = fail(N-1), bit 0 = fail
			data = 0xe0;
			logerror("%08x FLASH: read status %02X\n", cpu_get_pc(&space->device()), data);
			break;

		default:
		{
			logerror("%08x FLASH: unknown read in state %s\n", cpu_get_pc(&space->device()), flash_state_name[flash_state]);
		}
	}

	return data;
}

static READ8_HANDLER( flash_ready_r )
{
	return 1;
}

// FLASH interface

static READ64_HANDLER( ibara_flash_port_e_r )
{
	return	((flash_ready_r(space, offset) ? 0x20 : 0x00)) |
			input_port_read(space->machine(), "PORT_E");
}


static READ8_HANDLER( ibara_flash_io_r )
{
	switch (offset)
	{
		default:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:

		//  logerror("ibara_flash_io_r offset %04x\n", offset);
			return 0xff;

		case 0x00:
			return flash_io_r(space,offset);
	}
}

static WRITE8_HANDLER( ibara_flash_io_w )
{
	switch (offset)
	{
		default:
		case 0x03:
			logerror("unknown ibara_flash_io_w offset %04x data %02x\n", offset, data); // 03 enable/disable fgpa access?
			break;

		case 0x00:
			flash_data_w(space, offset, data);
			break;

		case 0x01:
			flash_cmd_w(space, offset, data);
			break;

		case 0x2:
			flash_addr_w(space, offset, data);
			break;
	}
}



// ibarablk uses the rtc to render the clock in the first attract demo
// if this code returns bad values it has gfx corruption.  the ibarablka set doesn't do this?!
static READ8_HANDLER( serial_rtc_eeprom_r )
{
	switch (offset)
	{
		default:
			return 0;
		//logerror("unknown serial_rtc_eeprom_r access offset %02x\n", offset);
		case 1:
			return 0xfe;
	}
}

static WRITE8_HANDLER( serial_rtc_eeprom_w )
{
	switch (offset)
	{
		case 0x01:
		// data & 0x00010000 = DATA
		// data & 0x00020000 = CLK
		// data & 0x00040000 = CE
		break;

		case 0x03:
			flash_enab_w(space,offset,data);
			return;

		default:
		logerror("unknown serial_rtc_eeprom_w access offset %02x data %02x\n",offset, data);
		break;
	}

}

static UINT64*cavesh3_ram;

static WRITE64_HANDLER( cavesh3_nop_write )
{

}

static ADDRESS_MAP_START( cavesh3_map, AS_PROGRAM, 64 )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM AM_REGION("maincpu", 0) AM_WRITE(cavesh3_nop_write) // mmmbnk writes here on startup for some reason..
	AM_RANGE(0x00200000, 0x003fffff) AM_ROM AM_REGION("maincpu", 0)

	/*       0x04000000, 0x07ffffff  SH3 Internal Regs (including ports) */

	AM_RANGE(0x0c000000, 0x0c7fffff) AM_RAM AM_BASE(&cavesh3_ram)//  AM_SHARE("mainram")// work RAM
	AM_RANGE(0x0c800000, 0x0cffffff) AM_RAM// AM_SHARE("mainram") // mirror of above on type B boards, extra ram on type D

	AM_RANGE(0x10000000, 0x10000007) AM_READWRITE8(ibara_flash_io_r, ibara_flash_io_w, U64(0xffffffffffffffff))
	AM_RANGE(0x10400000, 0x10400007) AM_READWRITE(ymz2770c_z_r, ymz2770c_z_w)
	AM_RANGE(0x10C00000, 0x10C00007) AM_READWRITE8(serial_rtc_eeprom_r, serial_rtc_eeprom_w, U64(0xffffffffffffffff))
	AM_RANGE(0x18000000, 0x18000057) AM_READWRITE32(cavesh3_blitter_r, cavesh3_blitter_w, U64(0xffffffffffffffff))

	AM_RANGE(0xf0000000, 0xf0ffffff) AM_RAM // mem mapped cache (sh3 internal?)
	/*       0xffffe000, 0xffffffff  SH3 Internal Regs 2 */
ADDRESS_MAP_END

static READ64_HANDLER( ibara_fpga_r )
{
	return 0xff;
}

static WRITE64_HANDLER( ibara_fpga_w )
{
	if (ACCESSING_BITS_24_31)
	{
		// data & 0x08 = CE
		// data & 0x10 = CLK
		// data & 0x20 = DATA
	}
}


static ADDRESS_MAP_START( cavesh3_port, AS_IO, 64 )
	AM_RANGE(SH3_PORT_C, SH3_PORT_C+7) AM_READ_PORT("PORT_C")
	AM_RANGE(SH3_PORT_D, SH3_PORT_D+7) AM_READ_PORT("PORT_D")
	AM_RANGE(SH3_PORT_E, SH3_PORT_E+7) AM_READ( ibara_flash_port_e_r )
	AM_RANGE(SH3_PORT_F, SH3_PORT_F+7) AM_READ_PORT("PORT_F")
	AM_RANGE(SH3_PORT_L, SH3_PORT_L+7) AM_READ_PORT("PORT_L")
	AM_RANGE(SH3_PORT_J, SH3_PORT_J+7) AM_READWRITE( ibara_fpga_r, ibara_fpga_w )
ADDRESS_MAP_END


static INPUT_PORTS_START( cavesh3 )
	PORT_START("DSW")		// 18000050.l (18000050.b + 3 i.e. MSB + 3, is shown as DIPSW)
//  PORT_BIT(        0xfcfffffc, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME(    0x00000002, 0x00000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000002, DEF_STR( On ) )
	PORT_SERVICE(    0x00000001, IP_ACTIVE_HIGH )

	PORT_START("PORT_C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )	// Service coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )	// Test button copied here
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1  )	// IMPLEMENT COIN ERROR!
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PORT_D")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4        ) PORT_PLAYER(1)

	PORT_START("PORT_E")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL )	// FLASH ready
	PORT_BIT( 0xdf, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("PORT_F")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE2 )	// Test Push Button
	PORT_BIT( 0xfd, IP_ACTIVE_LOW,  IPT_UNKNOWN )


	PORT_START("PORT_L")	// 4000134.b, 4000136.b
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4        ) PORT_PLAYER(2)
INPUT_PORTS_END


// apparently correct (but where is the OSC?)
#define CAVE_CPU_CLOCK 12800000 * 8

// none of this is verified for cave sh3
static const struct sh4_config sh4cpu_config = {
	0, // md2 (clock divders)
	0, // md1 (clock divders)
	0, // md0 (clock divders)
	0,
	0,
	0,
	1,
	1, // md7 (master?)
	0,
	CAVE_CPU_CLOCK
};




static INTERRUPT_GEN(cavesh3_interrupt)
{
	device_set_input_line(device, 2, HOLD_LINE);
}

static MACHINE_RESET( cavesh3 )
{
	flash_enab = 0;
	flash_hard_reset(machine);
	cavesh3_ram16 = (UINT16*)cavesh3_ram;
}

static PALETTE_INIT( cavesh_RRRRR_GGGGG_BBBBB )
{
	int i;
	for (i = 0; i < 0x10000; i++)
		palette_set_color(machine, i, MAKE_RGB(pal5bit(i >> 10), pal5bit(i >> 5), pal5bit(i >> 0)));
}


static MACHINE_CONFIG_START( cavesh3, cavesh3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH3BE, CAVE_CPU_CLOCK)
	MCFG_CPU_CONFIG(sh4cpu_config)
	MCFG_CPU_PROGRAM_MAP(cavesh3_map)
	MCFG_CPU_IO_MAP(cavesh3_port)
	MCFG_CPU_VBLANK_INT("screen", cavesh3_interrupt)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(0x200, 0x200)
	MCFG_SCREEN_VISIBLE_AREA(0, 0x140-1, 0, 0xf0-1)

	MCFG_PALETTE_INIT(cavesh_RRRRR_GGGGG_BBBBB)
	MCFG_PALETTE_LENGTH(0x10000)


	MCFG_SCREEN_UPDATE(cavesh3)
	MCFG_MACHINE_RESET(cavesh3)

	MCFG_VIDEO_START(cavesh3)
MACHINE_CONFIG_END

/**************************************************

All roms are flash roms with no lables, so keep the
 version numbers attached to the roms that differ

**************************************************/

ROM_START( mushisam )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u4", 0x000000, 0x200000, CRC(0b5b30b2) SHA1(35fd1bb1561c30b311b4325bc8f4628f2fccd20b) ) /* (2004/10/12 MASTER VER.) */

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD("u2", 0x000000, 0x8400000, CRC(b1f826dc) SHA1(c287bd9f571d0df03d7fcbcf3c57c74ce564ab05) ) /* (2004/10/12 MASTER VER.) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(138e2050) SHA1(9e86489a4e65af5efb5495adf6d4b3e01d5b2816) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(e3d05c9f) SHA1(130c3d62317da1729c85bd178bd51500edd73ada) )
ROM_END

ROM_START( mushisama )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u4", 0x000000, 0x200000, CRC(9f1c7f51) SHA1(f82ae72ec03687904ca7516887080be92365a5f3) ) /* (2004/10/12 MASTER VER) */

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD("u2", 0x000000, 0x8400000, CRC(2cd13810) SHA1(40e45e201b60e63a060b68d4cc767eb64cfb99c2) ) /* (2004/10/12 MASTER VER) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(138e2050) SHA1(9e86489a4e65af5efb5495adf6d4b3e01d5b2816) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(e3d05c9f) SHA1(130c3d62317da1729c85bd178bd51500edd73ada) )
ROM_END

ROM_START( espgal2 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(09c908bb) SHA1(7d6031fd3542b3e1d296ff218feb40502fd78694) ) /* (2005/11/14 MASTER VER) */

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(222f58c7) SHA1(d47a5085a1debd9cb8c61d88cd39e4f5036d1797) ) /* (2005/11/14 MASTER VER) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(b9a10c22) SHA1(4561f95c6018c9716077224bfe9660e61fb84681) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(c76b1ec4) SHA1(b98a53d41a995d968e0432ed824b0b06d93dcea8) )
ROM_END

ROM_START( mushitam )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u4", 0x000000, 0x200000, CRC(4a23e6c8) SHA1(d44c287bb88e6d413a8d35d75bc1b4928ad52cdf) ) /* (2005/09/09 MASTER VER) */

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD("u2", 0x000000, 0x8400000, CRC(3f93ff82) SHA1(6f6c250aa7134016ffb288d056bc937ea311f538) ) /* (2005/09/09 MASTER VER) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(701a912a) SHA1(85c198946fb693d99928ea2595c84ba4d9dc8157) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(6feeb9a1) SHA1(992711c80e660c32f97b343c2ce8184fddd7364e) )
ROM_END

ROM_START( futari15 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u4", 0x000000, 0x200000, CRC(e8c5f128) SHA1(45fb8066fdbecb83fdc2e14555c460d0c652cd5f) ) /* (2006/12/8.MAST VER. 1.54.) */

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD("u2", 0x000000, 0x8400000, CRC(b9eae1fc) SHA1(410f8e7cfcbfd271b41fb4f8d049a13a3191a1f9) ) /* (2006/12/8.MAST VER. 1.54.) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(39f1e1f4) SHA1(53d12f59a56df35c705408c76e6e02118da656f1) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(c631a766) SHA1(8bb6934a2f5b8a9841c3dcf85192b1743773dd8b) )
ROM_END

ROM_START( futari15a )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u4", 0x000000, 0x200000, CRC(a609cf89) SHA1(56752fae9f42fa852af8ee2eae79e25ec7f17953) ) /* (2006/12/8 MAST VER 1.54) */

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD("u2", 0x000000, 0x8400000, CRC(b9d815f9) SHA1(6b6f668b0bbb087ffac65e4f0d8bd9d5b28eeb28) ) /* (2006/12/8 MAST VER 1.54) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(39f1e1f4) SHA1(53d12f59a56df35c705408c76e6e02118da656f1) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(c631a766) SHA1(8bb6934a2f5b8a9841c3dcf85192b1743773dd8b) )
ROM_END

ROM_START( futari10 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(b127dca7) SHA1(e1f518bc72fc1cdf69aefa89eafa4edaf4e84778) ) /* (2006/10/23 MASTER VER.) */

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(78ffcd0c) SHA1(0e2937edec15ce3f5741b72ebd3bbaaefffb556e) ) /* (2006/10/23 MASTER VER.) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(39f1e1f4) SHA1(53d12f59a56df35c705408c76e6e02118da656f1) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(c631a766) SHA1(8bb6934a2f5b8a9841c3dcf85192b1743773dd8b) )
ROM_END

ROM_START( futariblk )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(6db13c62) SHA1(6a53ce7f70b754936ccbb3a4674d4b2f03979644) ) /* (2007/12/11 BLACK LABEL VER) */

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(08c6fd62) SHA1(e1fc386b2b0e41906c724287cbf82304297e0150) ) /* (2007/12/11 BLACK LABEL VER) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(39f1e1f4) SHA1(53d12f59a56df35c705408c76e6e02118da656f1) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(c631a766) SHA1(8bb6934a2f5b8a9841c3dcf85192b1743773dd8b) )
ROM_END

ROM_START( ibara )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(8e6c155d) SHA1(38ac2107dc7824836e2b4e04c7180d5ae43c9b79) ) /* (2005/03/22 MASTER VER..) */

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(55840976) SHA1(4982bdce84f9603adfed7a618f18bc80359ab81e) ) /* (2005/03/22 MASTER VER..) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(ee5e585d) SHA1(7eeba4ee693060e927f8c46b16e39227c6a62392) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(f0aa3cb6) SHA1(f9d137cd879e718811b2d21a0af2a9c6b7dca2f9) )
ROM_END

ROM_START( ibarablk )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(ee1f1f77) SHA1(ac276f3955aa4dde2544af4912819a7ae6bcf8dd) ) /* (2006/02/06. MASTER VER.) */

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(5e46be44) SHA1(bed5f1bf452f2cac58747ecabec3c4392566a3a7) ) /* (2006/02/06. MASTER VER.) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(a436bb22) SHA1(0556e771cc02638bf8814315ba671c2d442594f1) ) /* (2006/02/06 MASTER VER.) */
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(d11ab6b6) SHA1(2132191cbe847e2560423e4545c969f21f8ff825) ) /* (2006/02/06 MASTER VER.) */
ROM_END

ROM_START( ibarablka )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(a9d43839) SHA1(507696e616608c05893c7ac2814b3365e9cb0720) ) /* (2006/02/06 MASTER VER.) */

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(33400d96) SHA1(09c22b5431ac3726bf88c56efd970f56793f825a) ) /* (2006/02/06 MASTER VER.) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(a436bb22) SHA1(0556e771cc02638bf8814315ba671c2d442594f1) ) /* (2006/02/06 MASTER VER.) */
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(d11ab6b6) SHA1(2132191cbe847e2560423e4545c969f21f8ff825) ) /* (2006/02/06 MASTER VER.) */
ROM_END

ROM_START( deathsml )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(1a7b98bf) SHA1(07798a4a846e5802756396b34df47d106895c1f1) ) /* (2007/10/09 MASTER VER) */

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(d45b0698) SHA1(7077b9445f5ed4749c7f683191ccd312180fac38) ) /* (2007/10/09 MASTER VER) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(aab718c8) SHA1(0e636c46d06151abd6f73232bc479dafcafe5327) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(83881d84) SHA1(6e2294b247dfcbf0ced155dc45c706f29052775d) )
ROM_END

ROM_START( mmpork )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(d06cfa42) SHA1(5707feb4b3e5265daf5926f38c38612b24106f1f) ) /* (2007/ 4/17 MASTER VER.) */

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(1ee961b8) SHA1(81a2eba704ac1cf7fc44fa7c6a3f50e3570c104f) ) /* (2007/ 4/17 MASTER VER.) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(4a4b36df) SHA1(5db5ce6fa47e5ca3263d4bd19315890c6d29df66) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(ce83d07b) SHA1(a5947467c8f5b7c4b0ad8e32df2ee29b787e355f) )
ROM_END

ROM_START( mmmbnk )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x0000, 0x200000, CRC(5589d8c6) SHA1(43fbdb0effe2bc0e7135698757b6ee50200aecde) ) /* (2007/06/05 MASTER VER.) */

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x0000, 0x8400000, CRC(f3b50c30) SHA1(962327798081b292b2d3fd3b7845c0197f9f2d8a) ) /* (2007/06/05 MASTER VER.) */

	ROM_REGION( 0x800000, "samples", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(4caaa1bf) SHA1(9b92c13eac05601da4d9bb3eb727c156974e9f0c) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(8e3a51ba) SHA1(e34cf9acb13c3d8ca6cd1306b060b1d429872abd) )
ROM_END




static READ64_HANDLER( mushisam_speedup_r )
{
	int pc = cpu_get_pc(&space->device());
	if ( pc == 0xc04a0aa ) device_spin_until_time(&space->device(), attotime::from_usec(10)); // mushisam
	else if (pc == 0xc04a0da)  device_spin_until_time(&space->device(), attotime::from_usec(10)); // mushitam
//  else printf("read %08x\n", cpu_get_pc(&space->device()));
	return cavesh3_ram[0x0022f0/8];
}

DRIVER_INIT( mushisam )
{
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xc0022f0, 0xc0022f7, FUNC(mushisam_speedup_r) );
}

static READ64_HANDLER( mushisama_speedup_r )
{
	if ( cpu_get_pc(&space->device())== 0xc04a2aa ) device_spin_until_time(&space->device(), attotime::from_usec(10)); // mushisam
//  else printf("read %08x\n", cpu_get_pc(&space->device()));
	return cavesh3_ram[0x00024d8/8];
}

DRIVER_INIT( mushisama )
{
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xc0024d8, 0xc0024df, FUNC(mushisama_speedup_r) );
}

static READ64_HANDLER( espgal2_speedup_r )
{
	int pc = cpu_get_pc(&space->device());

	if ( pc == 0xc05177a ) device_spin_until_time(&space->device(), attotime::from_usec(10)); // espgal2
	if ( pc == 0xc05176a ) device_spin_until_time(&space->device(), attotime::from_usec(10)); // futari15 / futari15a / futari10 / futariblk / ibarablk / ibarablka / mmpork
	if ( pc == 0xc0519a2 ) device_spin_until_time(&space->device(), attotime::from_usec(10)); // deathsml
	//else printf("read %08x\n", cpu_get_pc(&space->device()));
	return cavesh3_ram[0x002310/8];
}

DRIVER_INIT( espgal2 )
{
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xc002310, 0xc002317, FUNC(espgal2_speedup_r) );
}



/*
espgal2 c002310
futari15 c002310
futari15a c002310
futari10 c002310
futariblk c002310
ibarablk c002310
ibarablka c002310
deathsml c002310
mmpork c002310
*/


GAME( 2004, mushisam,  0,          cavesh3,    cavesh3,  mushisam,  ROT270, "Cave", "Mushihime Sama (2004/10/12 MASTER VER.)",                           GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2004, mushisama, mushisam,   cavesh3,    cavesh3,  mushisama, ROT270, "Cave", "Mushihime Sama (2004/10/12 MASTER VER)",                            GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2005, espgal2,   0,          cavesh3,    cavesh3,  espgal2, ROT270, "Cave", "EspGaluda II (2005/11/14 MASTER VER)",                              GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2005, mushitam,  0,          cavesh3,    cavesh3,  mushisam, ROT0, "Cave", "Mushihime Tama (2005/09/09 MASTER VER)",                            GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2006, futari15,  0,          cavesh3,    cavesh3,  espgal2, ROT270, "Cave", "Mushihime Sama Futari Ver 1.5 (2006/12/8.MASTER VER. 1.54.)",       GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2006, futari15a, futari15,   cavesh3,    cavesh3,  espgal2, ROT270, "Cave", "Mushihime Sama Futari Ver 1.5 (2006/12/8 MASTER VER 1.54)",         GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2006, futari10,  futari15,   cavesh3,    cavesh3,  espgal2, ROT270, "Cave", "Mushihime Sama Futari Ver 1.0 (2006/10/23 MASTER VER.)",            GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2007, futariblk, futari15,   cavesh3,    cavesh3,  espgal2, ROT270, "Cave", "Mushihime Sama Futari Black Label (2007/12/11 BLACK LABEL VER)",    GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2006, ibara,     0,          cavesh3,    cavesh3,  mushisam, ROT270, "Cave", "Ibara (2005/03/22 MASTER VER..)",                                   GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2006, ibarablk,  0,          cavesh3,    cavesh3,  0, ROT270, "Cave", "Ibara Kuro - Black Label (2006/02/06. MASTER VER.)",                GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2006, ibarablka, ibarablk,   cavesh3,    cavesh3,  espgal2, ROT270, "Cave", "Ibara Kuro - Black Label (2006/02/06 MASTER VER.)",                 GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2007, deathsml,  0,          cavesh3,    cavesh3,  espgal2, ROT0, "Cave", "Death Smiles (2007/10/09 MASTER VER)",                              GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2007, mmpork,    0,          cavesh3,    cavesh3,  espgal2, ROT270, "Cave", "Muchi Muchi Pork (2007/ 4/17 MASTER VER.)",                         GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2007, mmmbnk,    0,          cavesh3,    cavesh3,  0, ROT0, "Cave", "Medal Mahjong Moukari Bancho no Kiban (2007/06/05 MASTER VER.)",   GAME_NOT_WORKING | GAME_NO_SOUND )

/*

Known versions of games on this hardware (* denotes undumped):

MUSHIHIME SAMA
  "2004/10/12 MASTER VER"  - broken
  "2004/10/12 MASTER VER." - fixed 1
* "2004/10/12.MASTER VER." - fixed 2

MUSHIHIME TAMA
  "2005/09/09 MASTER VER"

ESPGALUDA II
  "2005/11/14 MASTER VER"

IBARA
  "2005/03/22 MASTER VER.."

IBARA BLACK LABEL
  "2006/02/06 MASTER VER."
  "2006/02/06. MASTER VER."

PINK SWEETS
* "2006/04/06 MASTER VER."
* "2006/04/06 MASTER VER..."
* "2006/04/06 MASTER VER...."
* "2006/05/18 MASTER VER."
* "2006/xx/xx MASTER VER"

MUSHIHIME SAMA FUTARI 1.0
* "2006/10/23 MASTER VER"  - Ultra unlockable
  "2006/10/23 MASTER VER." - Ultra unlockable
* "2006/10/23.MASTER VER." - Cannot unlock ultra

MUSHIHIME SAMA FUTARI 1.5
  "2006/12/8 MASTER VER 1.54"
  "2006/12/8.MASTER VER.1.54."

MUSHIHIME SAMA FUTARI BLACK LABEL
  "2007/12/11 BLACK LABEL VER"
* "2009/11/17 INTERNATIONAL BL"  ("Another Ver" on title screen)

MUCHI MUCHI PORK
  "2007/ 4/17 MASTER VER."
* 2 "period" ver, location of the periods unknown

MEDAL MAHJONG MOKUARI BANCHO NO KIBAN
  "2007/06/05 MASTER VER."

DEATH SMILES
  "2007/10/09 MASTER VER"

DEATH SMILES MEGA BLACK LABEL
* "2008/10/06 MEGABLACK LABEL VER"

DODONPACHI FUKKATSU 1.0
* "2008/05/16 MASTER VER"

DODONPACHI FUKKATSU 1.5
* "2008/06/23 MASTER VER 1.5"

DODONPACHI DAIFUKKATSU BLACK LABEL
* "2010/1/18 BLACK LABEL"

AKAI KATANA
* "2010/ 8/13 MASTER VER."
*  Home/Limited version, unknown date line, different gameplay from regular version, doesn't accept coins - permanent freeplay

MUSHIHIMESAMA 1.5 MATSURI VERSION
* 2011/5/23 CAVEMATSURI VER 1.5

*/

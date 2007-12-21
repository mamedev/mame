/*************************************************************************

    Driver for Midway Zeus games

**************************************************************************/

#include "driver.h"
#include "eminline.h"
#include "includes/midzeus.h"
#include "video/poly.h"
#include "video/rgbutil.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define DUMP_WAVE_RAM	0

#define WAVERAM_WIDTH	1024
#define WAVERAM_HEIGHT	2048



/*************************************
 *
 *  Type definitions
 *
 *************************************/

typedef struct _poly_extra_data poly_extra_data;
struct _poly_extra_data
{
	const void *	palbase;
	const void *	texbase;
	UINT16			solidcolor;
	INT16			zoffset;
	UINT16			transcolor;
	UINT16			texwidth;
	UINT16			color;
	UINT32			alpha;
};



/*************************************
 *
 *  Global variables
 *
 *************************************/

extern UINT32 *zeusbase;

static poly_manager *poly;
static UINT8 log_fifo;

static UINT32 zeus_fifo[20];
static UINT8 zeus_fifo_words;
static void *zeus_renderbase;
static rectangle zeus_cliprect;

static void *waveram[2];



/*************************************
 *
 *  Function prototypes
 *
 *************************************/

static void exit_handler(running_machine *machine);

static void zeus_register32_w(offs_t offset, UINT32 data, int logit);
static void zeus_register_update(offs_t offset, UINT32 oldval);
static int zeus_fifo_process(const UINT32 *data, int numwords);

static void log_fifo_command(const UINT32 *data, int numwords, const char *suffix);



/*************************************
 *
 *  Macros
 *
 *************************************/

#define WAVERAM_BLOCK(bank, blocknum)			((void *)((UINT8 *)waveram[bank] + 16 * (blocknum)))

#define WAVERAM_PTR8(base, bytenum)				((UINT8 *)(base) + BYTE4_XOR_LE(bytenum))
#define WAVERAM_READ8(base, bytenum)			(*WAVERAM_PTR8(base, bytenum))
#define WAVERAM_WRITE8(base, bytenum, data)		do { *WAVERAM_PTR8(base, bytenum) = (data); } while (0)

#define WAVERAM_PTR16(base, wordnum)			((UINT16 *)(base) + BYTE_XOR_LE(wordnum))
#define WAVERAM_READ16(base, wordnum)			(*WAVERAM_PTR16(base, wordnum))
#define WAVERAM_WRITE16(base, wordnum, data)	do { *WAVERAM_PTR16(base, wordnum) = (data); } while (0)

#define WAVERAM_PTR32(base, dwordnum)			((UINT32 *)(base) + (dwordnum))
#define WAVERAM_READ32(base, dwordnum)			(*WAVERAM_PTR32(base, dwordnum))
#define WAVERAM_WRITE32(base, dwordnum, data)	do { *WAVERAM_PTR32(base, dwordnum) = (data); } while (0)

#define PIXYX_TO_DWORDNUM(y, x)					((((y) & 0x1fe) << 11) | (((y) & 1) << 10) | (((x) & 0x1fe) << 1) | ((x) & 1))
#define DEPTHYX_TO_DWORDNUM(y, x)				(PIXYX_TO_DWORDNUM(y, x) | 2)

#define WAVERAM_PTRPIX(base, y, x)				WAVERAM_PTR32(base, PIXYX_TO_DWORDNUM(y, x))
#define WAVERAM_READPIX(base, y, x)				(*WAVERAM_PTRPIX(base, y, x))
#define WAVERAM_WRITEPIX(base, y, x, color)		do { *WAVERAM_PTRPIX(base, y, x) = (color);  } while (0)

#define WAVERAM_PTRDEPTH(base, y, x)			WAVERAM_PTR16(base, DEPTHYX_TO_DWORDNUM(y, x & ~1) * 2 + (x & 1))
#define WAVERAM_READDEPTH(base, y, x)			(*WAVERAM_PTRDEPTH(base, y, x))
#define WAVERAM_WRITEDEPTH(base, y, x, color)	do { *WAVERAM_PTRDEPTH(base, y, x) = (color);  } while (0)



/*************************************
 *
 *  Inlines for block addressing
 *
 *************************************/

INLINE void *waveram_ptr_from_block_addr(UINT8 bank, UINT32 addr)
{
	UINT32 blocknum = (addr % WAVERAM_WIDTH) + ((addr >> 12) % WAVERAM_HEIGHT) * WAVERAM_WIDTH;
	return WAVERAM_BLOCK(bank, blocknum);
}

INLINE void *waveram_ptr_from_expanded_addr(UINT8 bank, UINT32 addr)
{
	UINT32 blocknum = (addr % WAVERAM_WIDTH) + ((addr >> 16) % WAVERAM_HEIGHT) * WAVERAM_WIDTH;
	return WAVERAM_BLOCK(bank, blocknum);
}

INLINE void *waveram_ptr_from_texture_addr(UINT8 bank, UINT32 addr, int width)
{
	UINT32 blocknum = ((addr & ~1) * width) / 8;
	return WAVERAM_BLOCK(bank, blocknum);
}



/*************************************
 *
 *  Inlines for rendering
 *
 *************************************/

INLINE void waveram_plot(int y, int x, UINT32 color)
{
	if (x >= 0 && x <= zeus_cliprect.max_x && y >= 0 && y < zeus_cliprect.max_y)
		WAVERAM_WRITEPIX(zeus_renderbase, y, x, color);
}

INLINE void waveram_plot_depth(int y, int x, UINT32 color, UINT16 depth)
{
	if (x >= 0 && x <= zeus_cliprect.max_x && y >= 0 && y < zeus_cliprect.max_y)
	{
		WAVERAM_WRITEPIX(zeus_renderbase, y, x, color);
		WAVERAM_WRITEDEPTH(zeus_renderbase, y, x, depth);
	}
}

INLINE void waveram_plot_check_depth(int y, int x, UINT32 color, UINT16 depth)
{
	if (x >= 0 && x <= zeus_cliprect.max_x && y >= 0 && y < zeus_cliprect.max_y)
	{
		UINT16 *depthptr = WAVERAM_PTRDEPTH(zeus_renderbase, y, x);
		if (depth <= *depthptr)
		{
			WAVERAM_WRITEPIX(zeus_renderbase, y, x, color);
			*depthptr = depth;
		}
	}
}

INLINE void waveram_plot_check_depth_nowrite(int y, int x, UINT32 color, UINT16 depth)
{
	if (x >= 0 && x <= zeus_cliprect.max_x && y >= 0 && y < zeus_cliprect.max_y)
	{
		UINT16 *depthptr = WAVERAM_PTRDEPTH(zeus_renderbase, y, x);
		if (depth <= *depthptr)
			WAVERAM_WRITEPIX(zeus_renderbase, y, x, color);
	}
}



/*************************************
 *
 *  Inlines for texel accesses
 *
 *************************************/

INLINE UINT8 get_texel_8bit(const void *base, int y, int x, int width)
{
	UINT32 byteoffs = (y / 2) * (width * 2) + ((x / 4) << 3) + ((y & 1) << 2) + (x & 3);
	return WAVERAM_READ8(base, byteoffs);
}


INLINE UINT8 get_texel_4bit(const void *base, int y, int x, int width)
{
	UINT32 byteoffs = (y / 2) * (width * 2) + ((x / 8) << 3) + ((y & 1) << 2) + ((x / 2) & 3);
	return (WAVERAM_READ8(base, byteoffs) >> (4 * (x & 1))) & 0x0f;
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( midzeus2 )
{
	/* allocate memory for "wave" RAM */
	waveram[0] = auto_malloc(WAVERAM_WIDTH * WAVERAM_HEIGHT * 16);
	waveram[1] = auto_malloc(WAVERAM_WIDTH * WAVERAM_HEIGHT * 16);
	state_save_register_global_pointer(((UINT32 *)waveram[0]), WAVERAM_WIDTH * WAVERAM_HEIGHT * 16/4);
	state_save_register_global_pointer(((UINT32 *)waveram[1]), WAVERAM_WIDTH * WAVERAM_HEIGHT * 16/4);

	/* initialize polygon engine */
	poly = poly_alloc(10000, sizeof(poly_extra_data), POLYFLAG_ALLOW_QUADS);

	/* we need to cleanup on exit */
	add_exit_callback(machine, exit_handler);

	zeus_renderbase = waveram[1];
}


static void exit_handler(running_machine *machine)
{
#if DUMP_WAVE_RAM
	FILE *f = fopen("waveram.dmp", "w");
	int bank;
	int i;

	for (bank = 0; bank < 2; bank++)
		for (i = 0; i < WAVERAM_WIDTH * WAVERAM_HEIGHT; i++)
		{
			if (i % 4 == 0) fprintf(f, "%03X%03X: ", i / WAVERAM_WIDTH, i % WAVERAM_WIDTH);
			fprintf(f, " %08X %08X ", (UINT32)(waveram[bank][i] >> 32), (UINT32)waveram[bank][i]);
			if (i % 4 == 3) fprintf(f, "\n");
		}
	fclose(f);
#endif

	poly_free(poly);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

VIDEO_UPDATE( midzeus2 )
{
	int x, y;

	poly_wait(poly, "VIDEO_UPDATE");

	/* normal update case */
	if (!input_code_pressed(KEYCODE_W))
	{
		const void *base = waveram_ptr_from_expanded_addr(1, 0);//zeusbase[0xcc]);
		int xoffs = machine->screen[screen].visarea.min_x;
		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			UINT32 *dest = (UINT32 *)bitmap->base + y * bitmap->rowpixels;
			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
				dest[x] = WAVERAM_READPIX(base, y, x - xoffs);
		}
	}

	/* waveram drawing case */
	else
	{
		static int yoffs = 0;
		static int width = 256;
		const UINT64 *base;

		if (input_code_pressed(KEYCODE_DOWN)) yoffs += input_code_pressed(KEYCODE_LSHIFT) ? 0x40 : 1;
		if (input_code_pressed(KEYCODE_UP)) yoffs -= input_code_pressed(KEYCODE_LSHIFT) ? 0x40 : 1;
		if (input_code_pressed(KEYCODE_LEFT) && width > 4) { width >>= 1; while (input_code_pressed(KEYCODE_LEFT)) ; }
		if (input_code_pressed(KEYCODE_RIGHT) && width < 512) { width <<= 1; while (input_code_pressed(KEYCODE_RIGHT)) ; }

		if (yoffs < 0) yoffs = 0;
		base = waveram_ptr_from_block_addr(0, yoffs << 12);

		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			UINT16 *dest = (UINT16 *)bitmap->base + y * bitmap->rowpixels;
			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			{
				UINT8 tex = get_texel_8bit(base, y, x, width);
				dest[x] = (tex << 8) | tex;
			}
		}
		popmessage("offs = %06X", yoffs << 12);
	}

	return 0;
}



/*************************************
 *
 *  Core read handler
 *
 *************************************/

READ32_HANDLER( zeus2_r )
{
	UINT32 result = zeusbase[offset];

	logerror("%06X:zeus2_r(%02X)\n", activecpu_get_pc(), offset);
	
	switch (offset)
	{
		case 0x00:
			result = 0x20;
			break;
		
		case 0x01:
			/* bit  $000C0070 are tested in a loop until 0 */
			/* bits $00080000 is tested in a loop until 0 */
			/* bit  $00000004 is tested for toggling; probably VBLANK */
			result = 0x00;
			if (video_screen_get_vblank(0))
				result |= 0x04;
			break;

		case 0x54:
			/* upper 16 bits are masked when read -- is that the hpos? */
			result = video_screen_get_vpos(0);
			break;
	}
	
	return result;
}



/*************************************
 *
 *  Core write handler
 *
 *************************************/

WRITE32_HANDLER( zeus2_w )
{
	logerror("%06X:zeus2_w", activecpu_get_pc());
	zeus_register32_w(offset, data, TRUE);
}



	/*************************************
 *
 *  Handle register writes
 *
 *************************************/

static void zeus_register32_w(offs_t offset, UINT32 data, int logit)
{
	UINT32 oldval = zeusbase[offset];
	
	/* writes to register $CC need to force a partial update */
//	if ((offset & ~1) == 0xcc)
//		video_screen_update_partial(0, video_screen_get_vpos(0));

	/* always write to low word? */
	zeusbase[offset] = data;

	/* log appropriately */
	if (logit)
		logerror("(%02X) = %08X\n", offset, data);

	/* handle the update */
	zeus_register_update(offset, oldval);
}



/*************************************
 *
 *  Update state after a register write
 *
 *************************************/

static void zeus_register_update(offs_t offset, UINT32 oldval)
{
	/* handle the writes; only trigger on low accesses */
	switch (offset)
	{
		case 0x08:
			zeus_fifo[zeus_fifo_words++] = zeusbase[0x08];
			if (zeus_fifo_process(zeus_fifo, zeus_fifo_words))
				zeus_fifo_words = 0;
			break;

		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
			video_screen_update_partial(0, video_screen_get_vpos(0));
			{
				int vtotal = zeusbase[0x37] & 0xffff;
				int htotal = zeusbase[0x34] >> 16;
				rectangle visarea;

				visarea.min_x = zeusbase[0x33] >> 16;
				visarea.max_x = (zeusbase[0x34] & 0xffff) - 1;
				visarea.min_y = 0;
				visarea.max_y = zeusbase[0x35] & 0xffff;
				if (htotal > 0 && vtotal > 0 && visarea.min_x < visarea.max_x && visarea.max_y < vtotal)
				{
					video_screen_configure(0, htotal, vtotal, &visarea, HZ_TO_ATTOSECONDS((double)MIDZEUS_VIDEO_CLOCK / 4.0 / (htotal * vtotal)));
					zeus_cliprect = visarea;
					zeus_cliprect.max_x -= zeus_cliprect.min_x;
					zeus_cliprect.min_x = 0;
				}
			}
			break;
		
		case 0x41:
			/* this is the address, except in read mode, where it latches values */
			if (zeusbase[0x4e] & 0x10)
			{
				const void *src = waveram_ptr_from_expanded_addr(0, oldval);
				zeusbase[0x41] = oldval;
				zeusbase[0x48] = WAVERAM_READ32(src, 0);
				zeusbase[0x49] = WAVERAM_READ32(src, 1);

				if (zeusbase[0x4e] & 0x40)
				{
					zeusbase[0x41]++;
					zeusbase[0x41] += (zeusbase[0x41] & 0x400) << 6;
					zeusbase[0x41] &= ~0xfc00;
				}
			}
			break;
		
		case 0x48:
		case 0x49:
			/* if we're in write mode, process it */
			if (zeusbase[0x40] == 0x00890000)
			{
				/*
					zeusbase[0x4e]:
						bit 0-1: which register triggers write through
						bit 3:   enable write through via these registers
						bit 4:   seems to be set during reads, when 0x41 is used for latching
						bit 6:   enable autoincrement on write through
				*/
				if ((zeusbase[0x4e] & 0x08) && (offset & 3) == (zeusbase[0x4e] & 3))
				{
					void *dest = waveram_ptr_from_expanded_addr(0, zeusbase[0x41]);
					WAVERAM_WRITE32(dest, 0, zeusbase[0x48]);
					WAVERAM_WRITE32(dest, 1, zeusbase[0x49]);
					
					if (zeusbase[0x4e] & 0x40)
					{
						zeusbase[0x41]++;
						zeusbase[0x41] += (zeusbase[0x41] & 0x400) << 6;
						zeusbase[0x41] &= ~0xfc00;
					}
				}
			}
			
			/* make sure we log anything else */
			else
				logerror("\t[40]=%08X [4E]=%08X\n", zeusbase[0x40], zeusbase[0x4e]);
			break;

		case 0x51:
			
			/* in this mode, crusnexo expects the reads to immediately latch */
			if (zeusbase[0x50] == 0x00a20000)
				oldval = zeusbase[0x51];

			/* this is the address, except in read mode, where it latches values */
			if ((zeusbase[0x5e] & 0x10) || (zeusbase[0x50] == 0x00a20000))
			{
				const void *src = waveram_ptr_from_expanded_addr(1, oldval);
				zeusbase[0x51] = oldval;
				zeusbase[0x58] = WAVERAM_READ32(src, 0);
				zeusbase[0x59] = WAVERAM_READ32(src, 1);
				zeusbase[0x5a] = WAVERAM_READ32(src, 2);

				if (zeusbase[0x5e] & 0x40)
				{
					zeusbase[0x51]++;
					zeusbase[0x51] += (zeusbase[0x51] & 0x200) << 7;
					zeusbase[0x51] &= ~0xfe00;
				}
			}
			break;
		
		case 0x57:
			/* thegrid uses this to write either left or right halves of pixels */
			if (zeusbase[0x50] == 0x00e90000)
			{
				void *dest = waveram_ptr_from_expanded_addr(1, zeusbase[0x51]);
				if (zeusbase[0x57] & 1)
					WAVERAM_WRITE32(dest, 0, zeusbase[0x58]);
				if (zeusbase[0x57] & 4)
					WAVERAM_WRITE32(dest, 1, zeusbase[0x59]);
			}
			
			/* make sure we log anything else */
			else
				logerror("\t[50]=%08X [5E]=%08X\n", zeusbase[0x50], zeusbase[0x5e]);
			break;
		
		case 0x58:
		case 0x59:
		case 0x5a:
			/* if we're in write mode, process it */
			if (zeusbase[0x50] == 0x00890000)
			{
				/*
					zeusbase[0x5e]:
						bit 0-1: which register triggers write through
						bit 3:   enable write through via these registers
						bit 4:   seems to be set during reads, when 0x51 is used for latching
						bit 5:   unknown, currently used to specify ordering, but this is suspect
						bit 6:   enable autoincrement on write through
				*/
				if ((zeusbase[0x5e] & 0x08) && (offset & 3) == (zeusbase[0x5e] & 3))
				{
					void *dest = waveram_ptr_from_expanded_addr(1, zeusbase[0x51]);
					WAVERAM_WRITE32(dest, 0, zeusbase[0x58]);
					if (zeusbase[0x5e] & 0x20)
						WAVERAM_WRITE32(dest, 1, zeusbase[0x5a]);
					else
					{
						WAVERAM_WRITE32(dest, 1, zeusbase[0x59]);
						WAVERAM_WRITE32(dest, 2, zeusbase[0x5a]);
					}
					
					if (zeusbase[0x5e] & 0x40)
					{
						zeusbase[0x51]++;
						zeusbase[0x51] += (zeusbase[0x51] & 0x200) << 7;
						zeusbase[0x51] &= ~0xfe00;
					}
				}
			}
			
			/* make sure we log anything else */
			else
				logerror("\t[50]=%08X [5E]=%08X\n", zeusbase[0x50], zeusbase[0x5e]);
			break;
	}
}



/*************************************
 *
 *  Process the FIFO
 *
 *************************************/

static int zeus_fifo_process(const UINT32 *data, int numwords)
{
	/* handle logging */
	switch (data[0] >> 24)
	{
		/* 0x05: write 32-bit value to low registers */
		/* in model data, this is 0x19 */
		case 0x05:
			if (numwords < 2)
				return FALSE;
			if (log_fifo)
				log_fifo_command(data, numwords, " -- reg32");
			zeus_register32_w((data[0] >> 16) & 0x7f, data[1], log_fifo);
			break;

		default:
			printf("Unknown command %08X\n", data[0]);
			if (log_fifo)
				log_fifo_command(data, numwords, "\n");
			break;
	}
	return TRUE;
}



/*************************************
 *
 *  Debugging tools
 *
 *************************************/

static void log_fifo_command(const UINT32 *data, int numwords, const char *suffix)
{
	int wordnum;

	logerror("Zeus cmd %02X :", data[0] >> 24);
	for (wordnum = 0; wordnum < numwords; wordnum++)
		logerror(" %08X", data[wordnum]);
	logerror("%s", suffix);
}

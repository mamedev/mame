/***********************************************************************************************

    Old Genesis VDP emulation still used by System 18
    -------------------------------------------------

    (originally used for Sega System C/C2 Driver, source version 0.54 - 02 Feb 2003)

***********************************************************************************************/

#include "emu.h"
#include "sound/sn76496.h"
#include "includes/genesis.h"



/******************************************************************************
    Macros
******************************************************************************/

#define BITMAP_WIDTH		320
#define BITMAP_HEIGHT		240

#define VRAM_SIZE			0x10000
#define VRAM_MASK			(VRAM_SIZE - 1)
#define VSRAM_SIZE			0x80
#define VSRAM_MASK			(VSRAM_SIZE - 1)
#define CRAM_SIZE			0x40
#define CRAM_MASK			(CRAM_SIZE - 1)

#define VDP_VRAM_BYTE(x)	(vdp_vram[(x) & VRAM_MASK])
#define VDP_VSRAM_BYTE(x)	(vdp_vsram[(x) & VSRAM_MASK])
#define VDP_VRAM_WORD(x)	((VDP_VRAM_BYTE(x) << 8) | VDP_VRAM_BYTE((x) + 1))
#define VDP_VSRAM_WORD(x)	((VDP_VSRAM_BYTE(x) << 8) | VDP_VSRAM_BYTE((x) + 1))

#define EXTRACT_PIXEL(x,i)	(((x) >> (((i) ^ NATIVE_ENDIAN_VALUE_LE_BE(1,7)) * 4)) & 0x0f)



/******************************************************************************
    Function Prototypes
******************************************************************************/

static int  vdp_data_r(running_machine *machine);
static void vdp_data_w(running_machine *machine, int data);
static int  vdp_control_r(running_machine *machine);
static void vdp_control_w(address_space *space, int data);
static void vdp_register_w(running_machine *machine, int data, int vblank);
static void vdp_control_dma(address_space *space, int data);
static void vdp_dma_68k(address_space *space);
static void vdp_dma_fill(int);
static void vdp_dma_copy(void);

static void drawline(UINT16 *bitmap, int line, int bgfill);
static void get_scroll_tiles(int line, int scrollnum, UINT32 scrollbase, UINT32 *tiles, int *offset);
static void get_window_tiles(int line, UINT32 scrollbase, UINT32 *tiles);
static void drawline_tiles(UINT32 *tiles, UINT16 *bmap, int pri, int offset, int lclip, int rclip);
static void drawline_sprite(int line, UINT16 *bmap, int priority, UINT8 *spritebase);



/******************************************************************************
    Global variables
******************************************************************************/

/* EXTERNALLY ACCESSIBLE */
       UINT16		genesis_bg_pal_lookup[4];	/* lookup table for background tiles */
       UINT16		genesis_sp_pal_lookup[4];	/* lookup table for sprites */
       UINT8		genesis_vdp_regs[32];		/* VDP registers */

/* LOCAL */
static screen_device *genesis_screen;

static UINT8 *		vdp_vram;					/* VDP video RAM */
static UINT8 *		vdp_vsram;					/* VDP vertical scroll RAM */
static UINT8		display_enable;				/* is the display enabled? */
static int			genesis_palette_base;		/* offset to use when doing palette writes */

/* updates */
static UINT16 *		transparent_lookup;			/* fast transparent mapping table */

/* vram bases */
static UINT32		vdp_scrollabase;			/* base address of scroll A tiles */
static UINT32		vdp_scrollbbase;			/* base address of scroll B tiles */
static UINT32		vdp_windowbase;				/* base address of window tiles */
static UINT32		vdp_spritebase;				/* base address of sprite data */
static UINT32		vdp_hscrollbase;			/* base address of H scroll values */

/* other vdp variables */
static INT32		vdp_hscrollmask;			/* mask for H scrolling */
static UINT32		vdp_hscrollsize;			/* size of active H scroll values */
static UINT8		vdp_vscrollmode;			/* current V scrolling mode */

static UINT8		vdp_cmdpart;				/* partial command flag */
static UINT8		vdp_code;					/* command code value */
static UINT32		vdp_address;				/* current I/O address */
static UINT8		vdp_dmafill;				/* DMA filling flag */
static UINT8		scrollheight;				/* height of the scroll area in tiles */
static UINT8		scrollwidth;				/* width of the scroll area in tiles */
static UINT8		bgcol;						/* current background color */
static UINT8		window_down;				/* window Y direction */
static UINT32		window_vpos;				/* window Y position */
static UINT8		window_right;				/* window X direction */
static UINT32		window_hpos;				/* window X position */
static UINT8		window_width;				/* window width */


/******************************************************************************
    Video Start / Stop Functions
*******************************************************************************

    Here we allocate memory used by the VDP and various other video related parts
    of the System C/C2 hardware such as the Palette RAM. Here is what is needed

    64kb of VRAM (multi-purpose, storing tiles, tilemaps, hscroll data,
                    spritelist etc.)

    80bytes of VSRAM (used exclusively for storing Vertical Scroll values)

******************************************************************************/


static void start_genesis_vdp(screen_device *screen)
{
	static const UINT8 vdp_init[24] =
	{
		0x04, 0x44, 0x30, 0x3C, 0x07, 0x6C, 0x00, 0x00,
		0x00, 0x00, 0xFF, 0x00, 0x01, 0x37, 0x00, 0x02,
		0x01, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x80,
	};
	int i;

	genesis_screen = screen;

	/* allocate memory for the VDP, the lookup table, and the buffer bitmap */
	vdp_vram			= auto_alloc_array(screen->machine, UINT8, VRAM_SIZE);
	vdp_vsram			= auto_alloc_array(screen->machine, UINT8, VSRAM_SIZE);
	transparent_lookup	= auto_alloc_array(screen->machine, UINT16, 0x1000);

	/* clear the VDP memory, prevents corrupt tile in Puyo Puyo 2 */
	memset(vdp_vram, 0, VRAM_SIZE);
	memset(vdp_vsram, 0, VSRAM_SIZE);

	/* init transparency table */
	for (i = 0; i < 0x1000; i++)
	{
		int orig_color = i & 0x7ff;
		int half_bright = i & 0x800;

		if (orig_color & 0x100)
			transparent_lookup[i] = orig_color;
		else if (half_bright)
			transparent_lookup[i] = orig_color | 0x800;
		else
			transparent_lookup[i] = orig_color | 0x1000;
	}

	/* reset the palettes */
	genesis_palette_base = 0;
	genesis_bg_pal_lookup[0] = genesis_sp_pal_lookup[0] = 0x00;
	genesis_bg_pal_lookup[1] = genesis_sp_pal_lookup[1] = 0x10;
	genesis_bg_pal_lookup[2] = genesis_sp_pal_lookup[2] = 0x20;
	genesis_bg_pal_lookup[3] = genesis_sp_pal_lookup[3] = 0x30;

	/* reset VDP */
    for (i = 0; i < 24; i++)
        vdp_register_w(screen->machine, 0x8000 | (i << 8) | vdp_init[i], 1);
	vdp_cmdpart = 0;
	vdp_code    = 0;
	vdp_address = 0;

	/* Save State Stuff */
	state_save_register_global_array(screen->machine, genesis_vdp_regs);
	state_save_register_global_pointer(screen->machine, vdp_vram, 0x10000);
	state_save_register_global_pointer(screen->machine, vdp_vsram, 0x80);
	state_save_register_global_array(screen->machine, genesis_bg_pal_lookup);
	state_save_register_global_array(screen->machine, genesis_sp_pal_lookup);
	state_save_register_global(screen->machine, display_enable);
	state_save_register_global(screen->machine, vdp_scrollabase);
	state_save_register_global(screen->machine, vdp_scrollbbase);
	state_save_register_global(screen->machine, vdp_windowbase);
	state_save_register_global(screen->machine, vdp_spritebase);
	state_save_register_global(screen->machine, vdp_hscrollbase);
	state_save_register_global(screen->machine, vdp_hscrollmask);
	state_save_register_global(screen->machine, vdp_hscrollsize);
	state_save_register_global(screen->machine, vdp_vscrollmode);
	state_save_register_global(screen->machine, vdp_cmdpart);
	state_save_register_global(screen->machine, vdp_code);
	state_save_register_global(screen->machine, vdp_address);
	state_save_register_global(screen->machine, vdp_dmafill);
	state_save_register_global(screen->machine, scrollheight);
	state_save_register_global(screen->machine, scrollwidth);
	state_save_register_global(screen->machine, bgcol);
	state_save_register_global(screen->machine, window_down);
	state_save_register_global(screen->machine, window_vpos);
}


void system18_vdp_start(running_machine *machine)
{
	start_genesis_vdp(machine->primary_screen);

	genesis_palette_base = 0x1800;
	genesis_bg_pal_lookup[0] = genesis_sp_pal_lookup[0] = 0x1800;
	genesis_bg_pal_lookup[1] = genesis_sp_pal_lookup[1] = 0x1810;
	genesis_bg_pal_lookup[2] = genesis_sp_pal_lookup[2] = 0x1820;
	genesis_bg_pal_lookup[3] = genesis_sp_pal_lookup[3] = 0x1830;
}



/******************************************************************************
    Screen Refresh Functions
*******************************************************************************

    These are responsible for the core drawing. The update_display function
    can be called under several circumstances to cache all the currently
    displayed lines before a significant palette or scrolling change is
    set to occur. The actual refresh routine marks the accumulated palette
    entries and then converts the raw pens in the cache bitmap to their
    final remapped values.

******************************************************************************/

void system18_vdp_update( bitmap_t *bitmap, const rectangle *cliprect )
{
	int y;

	/* generate the final screen */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		drawline(BITMAP_ADDR16(bitmap, y, 0), y, 0xffff);
}

/******************************************************************************
    VDP Read & Write Handlers
*******************************************************************************

    The VDP is accessed via 8 & 16 bit reads / writes at the addresses 0xC00000 -
    0xC0001F.  Different locations have different functions in the following
    layout.  (based on Information in Charles MacDonalds Document)

    0xC00000 -   DATA port (8 or 16 bit, Read or Write)
    0xC00002 -   mirror of above
    0xC00004 -   CTRL port (8 or 16 bit, Read or Write)
    0xC00006 -   mirror of above
    0xC00008 -   HV counter (8 or 16 bit Read Only)
    0xC0000A -   mirror of above
    0xC0000C -   mirror of above
    0xC0000E -   mirror of above
    0xC00010 -   SN76489 (8 bit Write Only)
    0xC00012 -   mirror of above
    0xC00014 -   mirror of above
    0xC00016 -   mirror of above

    The SN76489 PSG Writes are intercepted before we get here and handled
    directly.

******************************************************************************/

READ16_HANDLER( genesis_vdp_r )
{
	switch (offset)
	{
		case 0x00:	/* Read Data */
		case 0x01:
			return vdp_data_r(space->machine);

		case 0x02:	/* Status Register */
		case 0x03:
			return vdp_control_r(space->machine);

		case 0x04:	/* HV counter */
		case 0x05:
		case 0x06:
		case 0x07:
		{
			int xpos = genesis_screen->hpos();
			int ypos = genesis_screen->vpos();

			/* adjust for the weird counting rules */
			if (xpos > 0xe9) xpos -= (342 - 0x100);
			if (ypos > 0xea) ypos -= (262 - 0x100);

			/* kludge for ichidant: it sets the H counter to 160, then */
			/* expects that a read from here will return 159 */
			if (ypos > 0) ypos -= 2;
			return (ypos << 8) | xpos;
		}
	}
	return 0;
}


WRITE16_HANDLER( genesis_vdp_w )
{
	running_device *device;
	switch (offset)
	{
		case 0x00:	/* Write data */
		case 0x01:
			if (mem_mask != 0xffff)
			{
				data &= mem_mask;
				 if (ACCESSING_BITS_8_15)
					data |= data >> 8;
				 else
					data |= data << 8;
			}
			vdp_data_w(space->machine, data);
			break;

		case 0x02:	/* Control Write */
		case 0x03:
			if (mem_mask != 0xffff)
			{
				data &= mem_mask;
				 if (ACCESSING_BITS_8_15)
					data |= data >> 8;
				 else
					data |= data << 8;
			}
			vdp_control_w(space, data);
			break;

		case 0x08:	/* SN76489 Write */
		case 0x09:
		case 0x0a:
		case 0x0b:
			device = space->machine->device("snsnd");
			if (device != NULL && ACCESSING_BITS_0_7)
				sn76496_w(device, 0, data & 0xff);
			break;
	}
}



/******************************************************************************
    VDP DATA Reads / Writes (Accesses to 0xC00000 - 0xC00003)
*******************************************************************************

    The functions here are called by the read / write handlers for the VDP.
    They deal with Data Reads and Writes to and from the VDP Chip.

    Read / write mode is set by Writes to VDP Control, Can't Read when in
    write mode or vice versa.

******************************************************************************/

/* Games needing Read to Work .. bloxeed (attract) .. puyo puyo .. probably more */
static int vdp_data_r(running_machine *machine)
{
	int read = 0;

	/* kill 2nd write pending flag */
	vdp_cmdpart = 0;

	/* which memory is based on the code */
	switch (vdp_code & 0x0f)
	{
		case 0x00:		/* VRAM read */
			read = VDP_VRAM_WORD(vdp_address & ~1);
			break;

		case 0x04:		/* VSRAM read */
			read = VDP_VSRAM_WORD(vdp_address & ~1);
			break;

		default:		/* Illegal read attempt */
			logerror("%s: VDP illegal read type %02x\n", cpuexec_describe_context(machine), vdp_code);
			read = 0x00;
			break;
	}

	/* advance the address */
	vdp_address += genesis_vdp_regs[15];
	return read;
}


static void vdp_data_w(running_machine *machine, int data)
{
	/* kill 2nd write pending flag */
	vdp_cmdpart = 0;

	/* handle the fill case */
	if (vdp_dmafill)
	{
		vdp_dma_fill(data);
		vdp_dmafill = 0;
		return;
	}

	/* which memory is based on the code */
	switch (vdp_code & 0x0f)
	{
		case 0x01:		/* VRAM write */

			/* if the hscroll RAM is changing, force an update */
			if (vdp_address >= vdp_hscrollbase &&
				vdp_address < vdp_hscrollbase + vdp_hscrollsize)
				machine->primary_screen->update_partial(machine->primary_screen->vpos());

			/* write to VRAM */
			if (vdp_address & 1)
				data = ((data & 0xff) << 8) | ((data >> 8) & 0xff);
			VDP_VRAM_BYTE(vdp_address & ~1) = data >> 8;
			VDP_VRAM_BYTE(vdp_address |  1) = data;
			break;

		case 0x03:		/* Palette write */
			{
				int offset = (vdp_address/2) % CRAM_SIZE;
				palette_set_color_rgb(machine, offset + genesis_palette_base, pal3bit(data >> 1), pal3bit(data >> 5), pal3bit(data >> 9));
			}
			break;

		case 0x05:		/* VSRAM write */

			/* vscroll RAM is changing, force an update */
			machine->primary_screen->update_partial(machine->primary_screen->vpos());

			/* write to VSRAM */
			if (vdp_address & 1)
				data = ((data & 0xff) << 8) | ((data >> 8) & 0xff);
			VDP_VSRAM_BYTE(vdp_address & ~1) = data >> 8;
			VDP_VSRAM_BYTE(vdp_address |  1) = data;
			break;

		default:		/* Illegal write attempt */
			logerror("%s: VDP illegal write type %02x data %04x\n", cpuexec_describe_context(machine), vdp_code, data);
			break;
	}

	/* advance the address */
	vdp_address += genesis_vdp_regs[15];
}



/******************************************************************************
    VDP CTRL Reads / Writes (Accesses to 0xC00004 - 0xC00007)
*******************************************************************************

    A Read from the Control Port will return the Status Register Value.
    16-bits are used to report the VDP status
    |  0     |  0     |  1     |  1     |  0     |  1     |  FIFE  |  FIFF  |
    |  VIP   |  SOF   |  SCL   |  ODD   |  VBLK  |  HBLK  |  DMA   |  PAL   |

        0,1 = Set Values
        FIFE = FIFO Empty
        FIFF = FIFO Full
        VIP  = Vertical Interrupt Pending
        SOF  = Sprite Overflow  (Not used in C2 afaik)
        SCL  = Sprite Collision (Not used in C2 afaik)
        ODD  = Odd Frame (Interlace Mode) (Not used in C2 afaik)
        VBLK = In Vertical Blank
        HBLK = In Horizontal Blank
        DMA  = DMA In Progress
        PAL  = Pal Mode Flag

    Control Writes are used for setting VDP Registers, setting up DMA Transfers
    etc.

    A Write to the Control port can consist of 2 16-bit Words.
    When the VDP _isn't_ expecting the 2nd part of a command the highest 2 bits
    of the data written will determine what it is we want to do.
    10xxxxxx xxxxxxxx will cause a register to be set
    anything else will trigger the 1st half of a 2 Part Mode Setting Command
    If the VDP is already expecting the 2nd half of a command the data written
    will be the 2nd half of the Mode setup.

******************************************************************************/

static int vdp_control_r(running_machine *machine)
{
	int status = 0x3600; // wwally needs fifo empty set

	/* kill 2nd write pending flag */
	vdp_cmdpart = 0;

	/* set the VBLANK bit */
	if (machine->primary_screen->vblank())
		status |= 0x0008;

	/* set the HBLANK bit */
	if (machine->primary_screen->hblank())
		status |= 0x0004;

	return (status);
}


static void vdp_control_w(address_space *space, int data)
{
	/* case 1: we're not expecting the 2nd half of a command */
	if (!vdp_cmdpart)
	{
		/* if 10xxxxxx xxxxxxxx this is a register setting command */
		if ((data & 0xc000) == 0x8000)
			vdp_register_w(space->machine, data, space->machine->primary_screen->vblank());

		/* otherwise this is the First part of a mode setting command */
		else
		{
			vdp_code    = (vdp_code & 0x3c) | ((data >> 14) & 0x03);
			vdp_address = (vdp_address & 0xc000) | (data & 0x3fff);
			vdp_cmdpart = 1;
		}
	}

	/* case 2: this is the 2nd part of a mode setting command */
	else
	{
		vdp_code    = (vdp_code & 0x03) | ((data >> 2) & 0x3c);
		vdp_address = (vdp_address & 0x3fff) | ((data << 14) & 0xc000);
		vdp_cmdpart = 0;
		vdp_control_dma(space, data);
	}
}


static void vdp_register_w(running_machine *machine, int data, int vblank)
{
	int scrwidth = 0;
	static const UINT8 is_important[32] = { 0,0,1,1,1,1,0,1,0,0,0,1,0,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0 };

	UINT8 regnum = (data & 0x1f00) >> 8; /* ---R RRRR ---- ---- */
	UINT8 regdat = (data & 0x00ff);      /* ---- ---- DDDD DDDD */

	genesis_vdp_regs[regnum] = regdat;

	/* these are mostly important writes; force an update if they written */
	if (is_important[regnum])
		machine->primary_screen->update_partial(machine->primary_screen->vpos());

	/* For quite a few of the registers its a good idea to set a couple of variable based
       upon the writes here */
	switch (regnum)
	{
		case 0x01: /* video modes */
			if (regdat & 8)
				popmessage("Video height = 240!");
			break;

		case 0x02: /* Scroll A Name Table Base */
			vdp_scrollabase = (regdat & 0x38) << 10;
			break;

		case 0x03: /* Window Name Table Base */
			vdp_windowbase = (regdat & 0x3e) << 10;
			break;

		case 0x04: /* Scroll B Name Table Base */
			vdp_scrollbbase = (regdat & 0x07) << 13;
			break;

		case 0x05: /* Sprite Table Base */
			vdp_spritebase = (regdat & 0x7e) << 9;
			break;

		case 0x07: /* BG Colour */
			bgcol = regdat & 0x3f;
			break;

		case 0x0b: /* Scroll Modes */
		{
			static const UINT16 mask_table[4] = { 0x000, 0x007, 0xff8, 0xfff };
			vdp_vscrollmode = (regdat & 0x04) >> 2;

			vdp_hscrollmask = mask_table[regdat & 3];
			vdp_hscrollsize = 4 * ((vdp_hscrollmask < 224) ? (vdp_hscrollmask + 1) : 224);
			break;
		}

		case 0x0c: /* video modes */
			/* HDG: taken from mess/machine/genesis.c */
			switch (regdat&0x81)
			{
				case 0x00: // 32 cell
					scrwidth=32;
					window_width=32;
				break;
				case 0x01: // 40 cell corrupted
					scrwidth=40;
					window_width=64;
				break;
				case 0x80: // illegal!
					scrwidth=40;
					window_width=64;
				break;
				case 0x81: // 40 cell
					scrwidth=40;
					window_width=64;
				break;
			}
			{
				int height = genesis_screen->height();
				rectangle visarea = genesis_screen->visible_area();
				attoseconds_t refresh = genesis_screen->frame_period().attoseconds;

				/* this gets called from the init! */
				visarea.max_x = scrwidth*8-1;
				genesis_screen->configure(scrwidth*8, height, visarea, refresh);
			}
			break;

		case 0x0d: /* HScroll Base */
			vdp_hscrollbase = (regdat & 0x3f) << 10;
			break;

		case 0x10: /* Scroll Size */
		{
			static const UINT8 size_table[4] = { 32, 64, 128, 128 };
			scrollwidth = size_table[regdat & 0x03];
			scrollheight = size_table[(regdat & 0x30) >> 4];
			break;
		}

		case 0x11: /* Window H Position .. Doesn't Matter for any of the C2 Games */
			window_right = regdat & 0x80;
			window_hpos = (regdat & 0x1f) << 4;
			break;

		case 0x12: /* Window V Position */
			window_down = regdat & 0x80;
			window_vpos = (regdat & 0x1f) << 3;
			break;
	}
}


static void vdp_control_dma(address_space *space, int data)
{
	if ((vdp_code & 0x20) && (genesis_vdp_regs[1] & 0x10))
	{
		switch(genesis_vdp_regs[23] & 0xc0)
		{
			case 0x00:
			case 0x40:		/* 68k -> VRAM Tranfser */
				vdp_dma_68k(space);
				break;

			case 0x80:		/* VRAM fill, can't be done here, requires data write */
				vdp_dmafill = 1;
				break;

			case 0xC0:		/* VRAM copy */
				vdp_dma_copy();
				break;
		}
	}
}



/******************************************************************************
    DMA handling
*******************************************************************************

    These are currently Pretty much directly from the C2 emu

******************************************************************************/

static void vdp_dma_68k(address_space *space)
{
	int length = genesis_vdp_regs[19] | (genesis_vdp_regs[20] << 8);
	int source = (genesis_vdp_regs[21] << 1) | (genesis_vdp_regs[22] << 9) | ((genesis_vdp_regs[23] & 0x7f) << 17);
	int count;

	/* length of 0 means 64k likely */
	if (!length)
		length = 0xffff;

	/* handle the DMA */
	for (count = 0; count < length; count++)
	{
		vdp_data_w(space->machine, space->read_word(source));
		source += 2;
	}
}


static void vdp_dma_copy(void)
{
	int length = genesis_vdp_regs[19] | (genesis_vdp_regs[20] << 8);
	int source = genesis_vdp_regs[21] | (genesis_vdp_regs[22] << 8);
	int count;

	/* length of 0 means 64k likely */
	if (!length)
		length = 0xffff;

	/* handle the DMA */
	for (count = 0; count < length; count++)
	{
		VDP_VRAM_BYTE(vdp_address) = VDP_VRAM_BYTE(source++);
		vdp_address += genesis_vdp_regs[15];
	}
}


static void vdp_dma_fill(int data)
{
	int length = genesis_vdp_regs[19] | (genesis_vdp_regs[20] << 8);
	int count;

	/* length of 0 means 64k likely */
	if (!length)
		length = 0xffff;

	/* handle the fill */
	VDP_VRAM_BYTE(vdp_address) = data;
	data >>= 8;
	for(count = 0; count < length; count++)
	{
		VDP_VRAM_BYTE(vdp_address ^ 1) = data;
		vdp_address += genesis_vdp_regs[15];
	}
}



/******************************************************************************
    Scroll calculations
*******************************************************************************

    These special routines handle the transparency effects in sprites.

******************************************************************************/

/*** Useful Little Functions **************************************************/

/* Note: We Expect plane = 0 for Scroll A, plane = 2 for Scroll B */
INLINE int vdp_gethscroll(int plane, int line)
{
	line &= vdp_hscrollmask;
	return 0x400 - (VDP_VRAM_WORD(vdp_hscrollbase + (4 * line) + plane) & 0x3ff);
}


/* Note: We expect plane = 0 for Scroll A, plane = 2 for Scroll B
   A Column is 8 Pixels Wide                                     */
static int vdp_getvscroll(int plane, int column)
{
	UINT32 vsramoffset;

	switch (vdp_vscrollmode)
	{
		case 0x00: /* Overall Scroll */
			return VDP_VSRAM_WORD(plane) & 0x7ff;

		case 0x01: /* Column Scroll */
			if (column == 40) column = 39; /* Fix Minor Innacuracy Only affects PotoPoto */
			vsramoffset = (4 * (column >> 1)) + plane;
			return VDP_VSRAM_WORD(vsramoffset) & 0x7ff;
	}
	return 0;
}



/******************************************************************************
    Drawing Functions
*******************************************************************************

    These are used by the Screen Refresh functions to do the actual rendering
    of a screenline to the bitmap.

    Draw Planes in Order
        Scroll B Low
        Scroll A Low / Window Low
        Sprites Low
        Scroll B High
        Scroll A High / Window High
        Sprites High

    NOTE: Low Sprites _can_ overlap High sprites, however none of the C2
            games do this ever so its safe to draw them in this order.

******************************************************************************/

static void drawline(UINT16 *bitmap, int line, int bgfill)
{
	int lowsprites, highsprites, link;
	UINT32 scrolla_tiles[41], scrollb_tiles[41], window_tiles[41];
	int scrolla_offset, scrollb_offset;
	UINT8 *lowlist[81], *highlist[81];
	int bgcolor = bgfill ? bgfill : genesis_bg_pal_lookup[0];
	int window_lclip, window_rclip;
	int scrolla_lclip, scrolla_rclip;
	int column, sprite;

	/* clear to the background color */
	for (column = 0; column < BITMAP_WIDTH; column++)
		bitmap[column] = bgcolor;

	/* if display is disabled, stop */
	if (!(genesis_vdp_regs[1] & 0x40))
		return;

	/* Sprites need to be Drawn in Reverse order .. may as well sort them here */
	link = lowsprites = highsprites = 0;
	for (sprite = 0; sprite < 80; sprite++)
	{
		UINT8 *spritebase = &VDP_VRAM_BYTE(vdp_spritebase + 8 * link);

		/* sort into high/low priorities */
		if (spritebase[4] & 0x0080)
			highlist[++highsprites] = spritebase;
		else
			lowlist[++lowsprites] = spritebase;

		/* get the link; if 0, stop processing */
		link = spritebase[3] & 0x7F;
		if (!link)
			break;
	}

	/* get tiles for the B scroll layer */
	get_scroll_tiles(line, 2, vdp_scrollbbase, scrollb_tiles, &scrollb_offset);

	/* get tiles for the A scroll layer */
	get_scroll_tiles(line, 0, vdp_scrollabase, scrolla_tiles, &scrolla_offset);

	/* get tiles for the window layer */
	get_window_tiles(line, vdp_windowbase, window_tiles);

	/* compute the windowing for this line */
	if ((window_down && line >= window_vpos) || (!window_down && line < window_vpos))
		window_lclip = 0, window_rclip = BITMAP_WIDTH - 1;
	else if (window_right)
		window_lclip = window_hpos, window_rclip = BITMAP_WIDTH - 1;
	else
		window_lclip = 0, window_rclip = window_hpos - 1;

	/* compute the clipping of the scroll A layer */
	if (window_lclip == 0)
	{
		scrolla_lclip = window_rclip + 1;
		scrolla_rclip = BITMAP_WIDTH - 1;
	}
	else
	{
		scrolla_lclip = 0;
		scrolla_rclip = window_lclip - 1;
	}

	/* Scroll B Low */
	drawline_tiles(scrollb_tiles, bitmap, 0, scrollb_offset, 0, BITMAP_WIDTH - 1);

	/* Scroll A Low */
	drawline_tiles(scrolla_tiles, bitmap, 0, scrolla_offset, scrolla_lclip, scrolla_rclip);

	/* Window Low */
	drawline_tiles(window_tiles, bitmap, 0, 0, window_lclip, window_rclip);

	/* Sprites Low */
	for (sprite = lowsprites; sprite > 0; sprite--)
		drawline_sprite(line, bitmap, 0, lowlist[sprite]);

	/* Scroll B High */
	drawline_tiles(scrollb_tiles, bitmap, 1, scrollb_offset, 0, BITMAP_WIDTH - 1);

	/* Scroll A High */
	drawline_tiles(scrolla_tiles, bitmap, 1, scrolla_offset, scrolla_lclip, scrolla_rclip);

	/* Window High */
	drawline_tiles(window_tiles, bitmap, 1, 0, window_lclip, window_rclip);

	/* Sprites High */
	for (sprite = highsprites; sprite > 0; sprite--)
		drawline_sprite(line, bitmap, 1, highlist[sprite]);
}



/******************************************************************************
    Background rendering
*******************************************************************************

    This code handles all the scroll calculations and rendering of the
    background layers.

******************************************************************************/

/* determine the tiles we will draw on a scrolling layer */
static void get_scroll_tiles(int line, int scrollnum, UINT32 scrollbase, UINT32 *tiles, int *offset)
{
	int linehscroll = vdp_gethscroll(scrollnum, line);
	int column;

	/* adjust for partial tiles and then pre-divide hscroll to get the tile offset */
	*offset = -(linehscroll % 8);
	linehscroll /= 8;

	/* loop over columns */
	for (column = 0; column < 41; column++)
	{
		int columnvscroll = vdp_getvscroll(scrollnum, (column - (linehscroll & 1)) & 0x3f) + line;

		/* determine the base of the tilemap row */
		int temp = ((columnvscroll / 8) & (scrollheight - 1)) * scrollwidth;
		int tilebase = scrollbase + 2 * temp;

		/* offset into the tilemap based on the column */
		temp = (linehscroll + column) & (scrollwidth - 1);
		tilebase += 2 * temp;

		/* get the tile info */
		*tiles++ = ((columnvscroll % 8) << 16) | VDP_VRAM_WORD(tilebase);
	}
}


/* determine the tiles we will draw on a non-scrolling window layer */
static void get_window_tiles(int line, UINT32 scrollbase, UINT32 *tiles)
{
	int column;

	/* loop over columns */
	for (column = 0; column < 40; column++)
	{
		/* determine the base of the tilemap row */
		int temp = (line / 8) * window_width + column;
		int tilebase = scrollbase + 2 * temp;

		/* get the tile info */
		*tiles++ = ((line % 8) << 16) | VDP_VRAM_WORD(tilebase);
	}
}


/* draw a line of tiles */
static void drawline_tiles(UINT32 *tiles, UINT16 *bmap, int pri, int offset, int lclip, int rclip)
{
	/* adjust for the 8-pixel slop */
	bmap += offset;
	if (lclip > rclip)
		return;

	/* loop over columns */
	for ( ; offset < BITMAP_WIDTH; offset += 8, bmap += 8)
	{
		UINT32 tile = *tiles++;

		/* if the tile is the correct priority, draw it */
		if (((tile >> 15) & 1) == pri && offset < BITMAP_WIDTH)
		{
			int colbase = genesis_bg_pal_lookup[(tile & 0x6000) >> 13];
			UINT32 *tp = (UINT32 *)&VDP_VRAM_BYTE((tile & 0x7ff) * 32);
			UINT32 mytile;
			int col;

			/* vertical flipping */
			if (!(tile & 0x1000))
				mytile = tp[tile >> 16];
			else
				mytile = tp[(tile >> 16) ^ 7];

			/* skip if all-transparent */
			if (!mytile)
				continue;

			/* non-clipped */
			if (offset >= lclip && offset <= rclip - 7)
			{
				/* non-flipped */
				if (!(tile & 0x0800))
				{
					col = EXTRACT_PIXEL(mytile, 0); if (col) bmap[0] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 1); if (col) bmap[1] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 2); if (col) bmap[2] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 3); if (col) bmap[3] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 4); if (col) bmap[4] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 5); if (col) bmap[5] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 6); if (col) bmap[6] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 7); if (col) bmap[7] = colbase + col;
				}

				/* horizontal flip */
				else
				{
					col = EXTRACT_PIXEL(mytile, 7); if (col) bmap[0] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 6); if (col) bmap[1] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 5); if (col) bmap[2] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 4); if (col) bmap[3] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 3); if (col) bmap[4] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 2); if (col) bmap[5] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 1); if (col) bmap[6] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 0); if (col) bmap[7] = colbase + col;
				}
			}

			/* clipped */
			else if (offset >= lclip - 8 && offset <= rclip)
			{
				/* non-flipped */
				if (!(tile & 0x0800))
				{
					col = EXTRACT_PIXEL(mytile, 0); if (col && (offset + 0) >= lclip && (offset + 0) <= rclip) bmap[0] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 1); if (col && (offset + 1) >= lclip && (offset + 1) <= rclip) bmap[1] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 2); if (col && (offset + 2) >= lclip && (offset + 2) <= rclip) bmap[2] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 3); if (col && (offset + 3) >= lclip && (offset + 3) <= rclip) bmap[3] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 4); if (col && (offset + 4) >= lclip && (offset + 4) <= rclip) bmap[4] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 5); if (col && (offset + 5) >= lclip && (offset + 5) <= rclip) bmap[5] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 6); if (col && (offset + 6) >= lclip && (offset + 6) <= rclip) bmap[6] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 7); if (col && (offset + 7) >= lclip && (offset + 7) <= rclip) bmap[7] = colbase + col;
				}

				/* horizontal flip */
				else
				{
					col = EXTRACT_PIXEL(mytile, 7); if (col && (offset + 0) >= lclip && (offset + 0) <= rclip) bmap[0] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 6); if (col && (offset + 1) >= lclip && (offset + 1) <= rclip) bmap[1] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 5); if (col && (offset + 2) >= lclip && (offset + 2) <= rclip) bmap[2] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 4); if (col && (offset + 3) >= lclip && (offset + 3) <= rclip) bmap[3] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 3); if (col && (offset + 4) >= lclip && (offset + 4) <= rclip) bmap[4] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 2); if (col && (offset + 5) >= lclip && (offset + 5) <= rclip) bmap[5] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 1); if (col && (offset + 6) >= lclip && (offset + 6) <= rclip) bmap[6] = colbase + col;
					col = EXTRACT_PIXEL(mytile, 0); if (col && (offset + 7) >= lclip && (offset + 7) <= rclip) bmap[7] = colbase + col;
				}
			}
		}
	}
}



/******************************************************************************
    Sprite drawing
*******************************************************************************

    These special routines handle the transparency effects in sprites.

******************************************************************************/

/* draw a non-horizontally-flipped section of a sprite */
INLINE void draw8pixs(UINT16 *bmap, int sxpos, int patno, int priority, int colbase, int patline)
{
	UINT32 tile = *(UINT32 *)&VDP_VRAM_BYTE(patno * 32 + 4 * patline);
	int col;

	/* skip if all-transparent */
	if (!tile)
		return;

	/* non-transparent */
	if ((colbase & 0x30) != 0x30 || !(genesis_vdp_regs[12] & 0x08))
	{
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 0); if (col) bmap[sxpos] = colbase + col; } sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 1); if (col) bmap[sxpos] = colbase + col; } sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 2); if (col) bmap[sxpos] = colbase + col; } sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 3); if (col) bmap[sxpos] = colbase + col; } sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 4); if (col) bmap[sxpos] = colbase + col; } sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 5); if (col) bmap[sxpos] = colbase + col; } sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 6); if (col) bmap[sxpos] = colbase + col; } sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 7); if (col) bmap[sxpos] = colbase + col; } sxpos++;
	}

	/* transparent */
	else
	{
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 0); if (col) { if (col < 0x0e) bmap[sxpos] = colbase + col; else bmap[sxpos] = transparent_lookup[((col & 1) << 11) | (bmap[sxpos] & 0x7ff)]; }} sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 1); if (col) { if (col < 0x0e) bmap[sxpos] = colbase + col; else bmap[sxpos] = transparent_lookup[((col & 1) << 11) | (bmap[sxpos] & 0x7ff)]; }} sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 2); if (col) { if (col < 0x0e) bmap[sxpos] = colbase + col; else bmap[sxpos] = transparent_lookup[((col & 1) << 11) | (bmap[sxpos] & 0x7ff)]; }} sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 3); if (col) { if (col < 0x0e) bmap[sxpos] = colbase + col; else bmap[sxpos] = transparent_lookup[((col & 1) << 11) | (bmap[sxpos] & 0x7ff)]; }} sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 4); if (col) { if (col < 0x0e) bmap[sxpos] = colbase + col; else bmap[sxpos] = transparent_lookup[((col & 1) << 11) | (bmap[sxpos] & 0x7ff)]; }} sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 5); if (col) { if (col < 0x0e) bmap[sxpos] = colbase + col; else bmap[sxpos] = transparent_lookup[((col & 1) << 11) | (bmap[sxpos] & 0x7ff)]; }} sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 6); if (col) { if (col < 0x0e) bmap[sxpos] = colbase + col; else bmap[sxpos] = transparent_lookup[((col & 1) << 11) | (bmap[sxpos] & 0x7ff)]; }} sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 7); if (col) { if (col < 0x0e) bmap[sxpos] = colbase + col; else bmap[sxpos] = transparent_lookup[((col & 1) << 11) | (bmap[sxpos] & 0x7ff)]; }} sxpos++;
	}
}


/* draw a horizontally-flipped section of a sprite */
INLINE void draw8pixs_hflip(UINT16 *bmap, int sxpos, int patno, int priority, int colbase, int patline)
{
	UINT32 tile = *(UINT32 *)&VDP_VRAM_BYTE(patno * 32 + 4 * patline);
	int col;

	/* skip if all-transparent */
	if (!tile)
		return;

	/* non-transparent */
	if ((colbase & 0x30) != 0x30 || !(genesis_vdp_regs[12] & 0x08))
	{
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 7); if (col) bmap[sxpos] = colbase + col; } sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 6); if (col) bmap[sxpos] = colbase + col; } sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 5); if (col) bmap[sxpos] = colbase + col; } sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 4); if (col) bmap[sxpos] = colbase + col; } sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 3); if (col) bmap[sxpos] = colbase + col; } sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 2); if (col) bmap[sxpos] = colbase + col; } sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 1); if (col) bmap[sxpos] = colbase + col; } sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 0); if (col) bmap[sxpos] = colbase + col; } sxpos++;
	}

	/* transparent */
	else
	{
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 7); if (col) { if (col < 0x0e) bmap[sxpos] = colbase + col; else bmap[sxpos] = transparent_lookup[((col & 1) << 11) | (bmap[sxpos] & 0x7ff)]; }} sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 6); if (col) { if (col < 0x0e) bmap[sxpos] = colbase + col; else bmap[sxpos] = transparent_lookup[((col & 1) << 11) | (bmap[sxpos] & 0x7ff)]; }} sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 5); if (col) { if (col < 0x0e) bmap[sxpos] = colbase + col; else bmap[sxpos] = transparent_lookup[((col & 1) << 11) | (bmap[sxpos] & 0x7ff)]; }} sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 4); if (col) { if (col < 0x0e) bmap[sxpos] = colbase + col; else bmap[sxpos] = transparent_lookup[((col & 1) << 11) | (bmap[sxpos] & 0x7ff)]; }} sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 3); if (col) { if (col < 0x0e) bmap[sxpos] = colbase + col; else bmap[sxpos] = transparent_lookup[((col & 1) << 11) | (bmap[sxpos] & 0x7ff)]; }} sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 2); if (col) { if (col < 0x0e) bmap[sxpos] = colbase + col; else bmap[sxpos] = transparent_lookup[((col & 1) << 11) | (bmap[sxpos] & 0x7ff)]; }} sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 1); if (col) { if (col < 0x0e) bmap[sxpos] = colbase + col; else bmap[sxpos] = transparent_lookup[((col & 1) << 11) | (bmap[sxpos] & 0x7ff)]; }} sxpos++;
		if (sxpos>=0 && sxpos<BITMAP_WIDTH) { col = EXTRACT_PIXEL(tile, 0); if (col) { if (col < 0x0e) bmap[sxpos] = colbase + col; else bmap[sxpos] = transparent_lookup[((col & 1) << 11) | (bmap[sxpos] & 0x7ff)]; }} sxpos++;
	}
}


static void drawline_sprite(int line, UINT16 *bmap, int priority, UINT8 *spritebase)
{
	int spriteypos   = (((spritebase[0] & 0x01) << 8) | spritebase[1]) - 0x80;
	int spritexpos   = (((spritebase[6] & 0x01) << 8) | spritebase[7]) - 0x80;
	int spriteheight = ((spritebase[2] & 0x03) + 1) * 8;
	int spritewidth  = (((spritebase[2] & 0x0c) >> 2) + 1) * 8;
	int spriteattr, patno, patflip, patline, colbase, x;

	/* skip if out of range */
	if (line < spriteypos || line >= spriteypos + spriteheight)
		return;
	if (spritexpos + spritewidth < 0 || spritexpos >= BITMAP_WIDTH)
		return;

	/* extract the remaining data */
	spriteattr = (spritebase[4] << 8) | spritebase[5];
	patno      = spriteattr & 0x07FF;
	patflip    = (spriteattr & 0x1800) >> 11;
	patline    = line - spriteypos;

	/* determine the color base */
	colbase = genesis_sp_pal_lookup[(spriteattr & 0x6000) >> 13];

	/* adjust for the X position */
	spritewidth >>= 3;
	spriteheight >>= 3;

	/* switch off the flip mode */
	switch (patflip)
	{
		case 0x00: /* No Flip */
			for (x = 0; x < spritewidth; x++)
			{
				if (spritexpos >= -7 && spritexpos < BITMAP_WIDTH)
					draw8pixs(bmap, spritexpos, patno, priority, colbase, patline);
				spritexpos += 8;
				patno += spriteheight;
			}
			break;

		case 0x01: /* Horizontal Flip */
			patno += spriteheight * (spritewidth - 1);
			for (x = 0; x < spritewidth; x++)
			{
				if (spritexpos >= -7 && spritexpos < BITMAP_WIDTH)
					draw8pixs_hflip(bmap, spritexpos, patno, priority, colbase, patline);
				spritexpos += 8;
				patno -= spriteheight;
			}
			break;

		case 0x02: /* Vertical Flip */
			patline = 8 * spriteheight - patline - 1;
			for (x = 0; x < spritewidth; x++)
			{
				if (spritexpos >= -7 && spritexpos < BITMAP_WIDTH)
					draw8pixs(bmap, spritexpos, patno, priority, colbase, patline);
				spritexpos += 8;
				patno += spriteheight;
			}
			break;

		case 0x03: /* Both Flip */
			patno += spriteheight * (spritewidth - 1);
			patline = 8 * spriteheight - patline - 1;
			for (x = 0; x < spritewidth; x++)
			{
				if (spritexpos >= -7 && spritexpos < BITMAP_WIDTH)
					draw8pixs_hflip(bmap, spritexpos, patno, priority, colbase, patline);
				spritexpos += 8;
				patno -= spriteheight;
			}
			break;
	}
}



/******************************************************************************/
/* General Information (Mainly Genesis Related                                */
/******************************************************************************/
/* Genesis VDP Registers (from sega2.doc)

Reg# : |  Bit7  |  Bit6  |  Bit5  |  Bit4  |  Bit3  |  Bit2  |  Bit1  |  Bit0  |    General Function
--------------------------------------------------------------------------------
0x00 : |  0     |  0     |  0     |  IE1   |  0     |  1     |  M3    |  0     |    Mode Set Register #1
IE = Enabled H Interrupt (Lev 4), M3 = Enable HV Counter Read / Stopped
0x01 : |  0     |  DISP  |  IE0   |  M1    |  M2    |  1     |  0     |  0     |    Mode Set Register #2
DISP = Display Enable, IE0 = Enabled V Interrupt (Lev 6), M1 = DMA Enabled, M2 = 30 Cell Mode
0x02 : |  0     |  0     |  SA15  |  SA14  |  SA13  |  0     |  0     |  0     |    Scroll A Base Address
SA13-15 = Bits 13-15 of the Scroll A Name Table Base Address in VRAM
0x03 : |  0     |  0     |  WD15  |  WD14  |  WD13  |  WD12  |  WD11  |  0     |    Window Base Address
WD11-15 = Bits 11-15 of the Window Name Table Base Address in VRAM
0x04 : |  0     |  0     |  0     |   0    |  0     |  SB15  |  SB14  |  SB13  |    Scroll B Base Address
SB13-15 = Bits 13-15 of the Scroll B Name Table Base Address in VRAM
0x05 : |  0     |  AT15  |  AT14  |  AT13  |  AT12  |  AT11  |  AT10  |  AT9   |    Sprite Table Base Address
AT9=15 = Bits 9-15 of the Sprite Name Table Base Address in VRAM
0x06 : |  0     |  0     |  0     |  0     |  0     |  0     |  0     |  0     |    Unused
0x07 : |  0     |  0     |  CPT1  |  CPT0  |  COL3  |  COL2  |  COL1  |  COL0  |    Background Colour Select
CPT0-1 = Palette Number, COL = Colour in Palette
0x08 : |  0     |  0     |  0     |  0     |  0     |  0     |  0     |  0     |    Unused
0x09 : |  0     |  0     |  0     |  0     |  0     |  0     |  0     |  0     |    Unused
0x0A : |  BIT7  |  BIT6  |  BIT5  |  BIT4  |  BIT3  |  BIT2  |  BIT1  |  BIT0  |    H-Interrupt Register
BIT0-7 = Controls Level 4 Interrupt Timing
0x0B : |  0     |  0     |  0     |  0     |  IE2   |  VSCR  |  HSCR  |  LSCR  |    Mode Set Register #3
IE2 = Enable E Interrupt (Lev 2), VSCR = Vertical Scroll Mode, HSCR / LSCR = Horizontal Scroll Mode
0x0C : |  RS0   |  0     |  0     |  0     |  S/TE  |  LSM1  |  LSM0  |  RS1   |    Mode Set Register #4
RS0 / RS1 = Cell Mode, S/TE = Shadow/Hilight Enable, LSM0 / LSM1 = Interlace Mode Setting
0x0D : |  0     |  0     |  HS15  |  HS14  |  HS13  |  HS12  |  HS11  |  HS10  |    HScroll Base Address
HS10-15 = Bits 10-15 of the HScroll Name Table Base Address in VRAM
0x0E : |  0     |  0     |  0     |  0     |  0     |  0     |  0     |  0     |    Unused
0x0F : |  INC7  |  INC6  |  INC5  |  INC4  |  INC3  |  INC2  |  INC1  |  INC0  |    Auto-Increment
INC0-7 = Auto Increment Value (after VRam Access)
0x10 : |  0     |  0     |  VSZ1  |  VSZ0  |  0     |  0     |  HSZ1  |  HSZ0  |    Scroll Size
VSZ0-1 = Vertical Plane Size, HSZ0-1 = Horizontal Plane Size
0x11 : |  RIGT  |  0     |  0     |  WHP5  |  WHP4  |  WHP3  |  WHP2  |  WHP1  |    Window H Position
RIGT = Window Right Side of Base Point, WHP1-5 = Bits1-5 of Window H Point
0x12 : |  DOWN  |  0     |  0     |  WVP4  |  WVP3  |  WVP2  |  WVP1  |  WVP0  |    Window V Position
DOWN = Window Below Base Point, WVP0-4 = Bits0-4 of Window V Point
0x13 : |  LG7   |  LG6   |  LG5   |  LG4   |  LG3   |  LG2   |  LG1   |  LG0   |    DMA Length Counter LOW
0x14 : |  LG15  |  LG14  |  LG13  |  LG12  |  LG11  |  LG10  |  LG9   |  LG8   |    DMA Length Counter HIGH
LG0-15 = Bits 0-15 of DMA Length Counter
0x15 : |  SA8   |  SA7   |  SA6   |  SA5   |  SA4   |  SA3   |  SA2   |  SA1   |    DMA Source Address LOW
0x16 : |  SA16  |  SA15  |  SA14  |  SA13  |  SA12  |  SA11  |  SA10  |  SA9   |    DMA Source Address MID
0x17 : |  DMD1  |  DMD0  |  SA22  |  SA21  |  SA20  |  SA19  |  SA18  |  S17   |    DMA Source Address HIGH
LG0-15 = Bits 1-22 of DMA Source Address
DMD0-1 = DMA Mode

Memory Layouts ...

Scroll Name Table
16-bits are used to Define a Tile in the Scroll Plane
|  PRI   |  CP1   |  CP0   |  VF    |  HF    |  PT10  |  PT9   |  PT8   |
|  PT7   |  PT6   |  PT5   |  PT4   |  PT3   |  PT2   |  PT1   |  PT0   |
PRI = Priority, CP = Colour Palette, VF = VFlip, HF = HFlip, PT0-9 = Tile # in VRAM

HScroll Data  (in VRAM)
0x00 |
0x01 / H Scroll of Plane A (Used in Overall, Cell & Line Modes)
0x02 |
0x03 / H Scroll of Plane B (Used in Overall, Cell & Line Modes)
0x04 |
0x05 / H Scroll of Plane A (Used in Line Mode)
0x06 |
0x07 / H Scroll of Plane B (Used in Line Mode)
...
0x20 |
0x21 / H Scroll of Plane A (Used in Cell & Line Mode)
0x22 |
0x23 / H Scroll of Plane B (Used in Cell & Line Mode)
etc.. That kinda thing :)
Data is in Format ..
|  x     |  x     |  x     |  x     |  x     |  x     |  HS9   |  HS8   |
|  HS7   |  HS6   |  HS5   |  HS4   |  HS3   |  HS2   |  HS1   |  HS0   |
HS = HScroll Amount for Overall / Cell / Line depending on Mode.

VScroll Data (in VSRAM)
0x00 |
0x01 / V Scroll of Plane A (Used in Overall & Cell Modes)
0x02 |
0x03 / V Scroll of Plane B (Used in Overall & Cell Modes)
0x04 |
0x05 / V Scroll of Plane A (Used in Cell Mode)
0x06 |
0x07 / V Scroll of Plane B (Used in Cell Modes)
etc..
Data is in Format ..
|  x     |  x     |  x     |  x     |  x     |  VS10  |  VS9   |  VS8   |
|  VS7   |  VS6   |  VS5   |  VS4   |  VS3   |  VS2   |  VS1   |  VS0   |
VS = HScroll Amount for Overall / Cell / Line depending on Mode.

Sprite Attributes Table (in VRAM)
Each Sprite uses 64-bits of Information (8 bytes)
|  x     |  x     |  x     |  x     |  x     |  x     |  YP9   |  YP8   |
|  YP7   |  YP6   |  YP5   |  YP4   |  YP3   |  YP2   |  YP1   |  YP0   |
|  x     |  x     |  x     |  x     |  HS1   |  HS0   |  VS1   |  VS0   |
|  x     |  LN6   |  LN5   |  LN4   |  LN3   |  LN2   |  LN1   |  LN0   |
|  PRI   |  CP1   |  CP0   |  VF    |  HF    |  PT10  |  PT9   |  PT8   |
|  PT7   |  PT6   |  PT5   |  PT4   |  PT3   |  PT2   |  PT1   |  PT0   |
|  x     |  x     |  x     |  x     |  x     |  x     |  XP9   |  XP8   |
|  XP7   |  XP6   |  XP5   |  XP4   |  XP3   |  XP2   |  XP1   |  XP0   |
YP = Y Position of Sprite
HS = Horizontal Size (Blocks)
VS = Vertical Size (Blocks)
LN = Link Field
PRI = Priority
CP = Colour Palette
VF = VFlip
HF = HFlip
PT = Pattern Number
XP = X Position of Sprite



*/


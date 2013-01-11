/***************************************************************************

    atarirle.c

    RLE sprite handling for early-to-mid 90's Atari raster games.

****************************************************************************

    Description:

    Beginning with Hydra, and continuing through to Primal Rage, Atari used
    RLE-compressed sprites. These sprites were decoded, colored, and scaled
    on the fly using an AMD 29C101 ALU unit. The instructions for the ALU
    were read from 3 512-byte PROMs and fed into the instruction input.

    See the bottom of the source for more details on the operation of these
    components.

***************************************************************************/

#include "emu.h"
#include "atarirle.h"


/***************************************************************************
    TYPES & STRUCTURES
***************************************************************************/

/* internal structure containing a word index, shift and mask */
struct atarirle_mask
{
	int             word;               /* word index */
	int             shift;              /* shift amount */
	int             mask;               /* final mask */
};

/* internal structure for sorting the motion objects */
struct mo_sort_entry
{
	mo_sort_entry * next;
	int             entry;
};

/* internal structure describing each object in the ROMs */
struct atarirle_info
{
	INT16           width;
	INT16           height;
	INT16           xoffs;
	INT16           yoffs;
	UINT8           bpp;
	const UINT16 *  table;
	const UINT16 *  data;
};

/* internal structure containing the state of the motion objects */
struct atarirle_data
{
	int             bitmapwidth;        /* width of the full playfield bitmap */
	int             bitmapheight;       /* height of the full playfield bitmap */
	int             bitmapxmask;        /* x coordinate mask for the playfield bitmap */
	int             bitmapymask;        /* y coordinate mask for the playfield bitmap */

	int             spriterammask;      /* combined mask when accessing sprite RAM with raw addresses */
	int             spriteramsize;      /* total size of sprite RAM, in entries */

	int             palettebase;        /* base palette entry */
	int             maxcolors;          /* maximum number of colors */

	rectangle       cliprect;           /* clipping rectangle */

	atarirle_mask   codemask;           /* mask for the code index */
	atarirle_mask   colormask;          /* mask for the color */
	atarirle_mask   xposmask;           /* mask for the X position */
	atarirle_mask   yposmask;           /* mask for the Y position */
	atarirle_mask   scalemask;          /* mask for the scale factor */
	atarirle_mask   hflipmask;          /* mask for the horizontal flip */
	atarirle_mask   ordermask;          /* mask for the order */
	atarirle_mask   prioritymask;       /* mask for the priority */
	atarirle_mask   vrammask;           /* mask for the VRAM target */

	const UINT16 *  rombase;            /* pointer to the base of the GFX ROM */
	int             romlength;          /* length of the GFX ROM */
	int             objectcount;        /* number of objects in the ROM */
	atarirle_info * info;               /* list of info records */
	atarirle_entry *spriteram;          /* pointer to sprite RAM */

	bitmap_ind16 *      vram[2][2];         /* pointers to VRAM bitmaps and backbuffers */
	int             partial_scanline;   /* partial update scanline */

	UINT8           control_bits;       /* current control bits */
	UINT8           command;            /* current command */
	UINT8           is32bit;            /* 32-bit or 16-bit? */
	UINT16          checksums[256];     /* checksums for each 0x40000 bytes */
	UINT16          ram[0x1000/2];
};



/***************************************************************************
    MACROS
***************************************************************************/

/* data extraction */
#define EXTRACT_DATA(_input, _mask) (((_input)->data[(_mask).word] >> (_mask).shift) & (_mask).mask)


enum { atarirle_hilite_index = -1 };



/***************************************************************************
    STATIC VARIABLES
***************************************************************************/

static UINT8 rle_bpp[8];
static UINT16 *rle_table[8];



/***************************************************************************
    STATIC FUNCTION DECLARATIONS
***************************************************************************/

static void build_rle_tables(running_machine &machine);
static int count_objects(const UINT16 *base, int length);
static void prescan_rle(const atarirle_data *mo, int which);
static void sort_and_render(running_machine &machine, atarirle_data *mo);
static void compute_checksum(atarirle_data *mo);
static void draw_rle(atarirle_data *mo, bitmap_ind16 &bitmap, int code, int color, int hflip, int vflip,
		int x, int y, int xscale, int yscale, const rectangle &clip);
static void draw_rle_zoom(bitmap_ind16 &bitmap, const atarirle_info *gfx,
		UINT32 palette, int sx, int sy, int scalex, int scaley,
		const rectangle &clip);
static void draw_rle_zoom_hflip(bitmap_ind16 &bitmap, const atarirle_info *gfx,
		UINT32 palette, int sx, int sy, int scalex, int scaley,
		const rectangle &clip);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE atarirle_data *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == ATARIRLE);

	return (atarirle_data *)downcast<atarirle_device *>(device)->token();
}


/*---------------------------------------------------------------
    compute_log: Computes the number of bits necessary to
    hold a given value. The input must be an even power of
    two.
---------------------------------------------------------------*/

#ifdef UNUSED_FUNCTION
INLINE int compute_log(int value)
{
	int log = 0;

	if (value == 0)
		return -1;
	while (!(value & 1))
		log++, value >>= 1;
	if (value != 1)
		return -1;
	return log;
}
#endif

/*---------------------------------------------------------------
    round_to_powerof2: Rounds a number up to the nearest
    power of 2. Even powers of 2 are rounded up to the
    next greatest power (e.g., 4 returns 8).
---------------------------------------------------------------*/

INLINE int round_to_powerof2(int value)
{
	int log = 0;

	if (value == 0)
		return 1;
	while ((value >>= 1) != 0)
		log++;
	return 1 << (log + 1);
}


/*---------------------------------------------------------------
    collapse_bits: Moving right-to-left, for each 1 bit in
    the mask, copy the corresponding bit from the input
    value into the result, packing the bits along the way.
---------------------------------------------------------------*/

#ifdef UNUSED_FUNCTION
INLINE int collapse_bits(int value, int mask)
{
	int testmask, ormask;
	int result = 0;

	for (testmask = ormask = 1; testmask != 0; testmask <<= 1)
		if (mask & testmask)
		{
			if (value & testmask)
				result |= ormask;
			ormask <<= 1;
		}
	return result;
}
#endif

/*---------------------------------------------------------------
    convert_mask: Converts a 4-word mask into a word index,
    shift, and adjusted mask. Returns 0 if invalid.
---------------------------------------------------------------*/

INLINE int convert_mask(const atarirle_entry *input, atarirle_mask *result)
{
	int i, temp;

	/* determine the word and make sure it's only 1 */
	result->word = -1;
	for (i = 0; i < 8; i++)
		if (input->data[i])
		{
			if (result->word == -1)
				result->word = i;
			else
				return 0;
		}

	/* if all-zero, it's valid */
	if (result->word == -1)
	{
		result->word = result->shift = result->mask = 0;
		return 1;
	}

	/* determine the shift and final mask */
	result->shift = 0;
	temp = input->data[result->word];
	while (!(temp & 1))
	{
		result->shift++;
		temp >>= 1;
	}
	result->mask = temp;
	return 1;
}



/***************************************************************************
    GLOBAL FUNCTIONS
***************************************************************************/

/*---------------------------------------------------------------
    atarirle_init: Configures the motion objects using the input
    description. Allocates all memory necessary and generates
    the attribute lookup table.
---------------------------------------------------------------*/

static DEVICE_START( atarirle )
{
	atarirle_data *mo = get_safe_token(device);
	running_machine &machine = device->machine();
	const atarirle_desc *desc = (const atarirle_desc *)device->static_config();
	const UINT16 *base = (const UINT16 *)machine.root_device().memregion(desc->region)->base();
	int i, width, height;

	/* build and allocate the generic tables */
	build_rle_tables(machine);

	/* determine the masks first */
	convert_mask(&desc->codemask,     &mo->codemask);
	convert_mask(&desc->colormask,    &mo->colormask);
	convert_mask(&desc->xposmask,     &mo->xposmask);
	convert_mask(&desc->yposmask,     &mo->yposmask);
	convert_mask(&desc->scalemask,    &mo->scalemask);
	convert_mask(&desc->hflipmask,    &mo->hflipmask);
	convert_mask(&desc->ordermask,    &mo->ordermask);
	convert_mask(&desc->prioritymask, &mo->prioritymask);
	convert_mask(&desc->vrammask,     &mo->vrammask);

	/* copy in the basic data */
	mo->bitmapwidth   = round_to_powerof2(mo->xposmask.mask);
	mo->bitmapheight  = round_to_powerof2(mo->yposmask.mask);
	mo->bitmapxmask   = mo->bitmapwidth - 1;
	mo->bitmapymask   = mo->bitmapheight - 1;

	mo->spriteramsize = desc->spriteramentries;
	mo->spriterammask = desc->spriteramentries - 1;

	mo->palettebase   = desc->palettebase;
	mo->maxcolors     = desc->maxcolors / 16;

	mo->rombase       = base;
	mo->romlength     = machine.root_device().memregion(desc->region)->bytes();
	mo->objectcount   = count_objects(base, mo->romlength);

	mo->cliprect      = machine.primary_screen->visible_area();
	if (desc->rightclip)
	{
		mo->cliprect.min_x = desc->leftclip;
		mo->cliprect.max_x = desc->rightclip;
	}

	/* compute the checksums */
	memset(mo->checksums, 0, sizeof(mo->checksums));
	for (i = 0; i < mo->romlength / 0x20000; i++)
	{
		const UINT16 *csbase = &mo->rombase[0x10000 * i];
		int cursum = 0, j;
		for (j = 0; j < 0x10000; j++)
			cursum += *csbase++;
		mo->checksums[i] = cursum;
	}

	/* allocate the object info */
	mo->info = auto_alloc_array_clear(machine, atarirle_info, mo->objectcount);

	/* fill in the data */
	for (i = 0; i < mo->objectcount; i++)
		prescan_rle(mo, i);

	/* allocate the spriteram */
	mo->spriteram = auto_alloc_array_clear(machine, atarirle_entry, mo->spriteramsize);

	/* allocate bitmaps */
	width = machine.primary_screen->width();
	height = machine.primary_screen->height();

	mo->vram[0][0] = auto_bitmap_ind16_alloc(machine, width, height);
	mo->vram[0][1] = auto_bitmap_ind16_alloc(machine, width, height);
	mo->vram[0][0]->fill(0);
	mo->vram[0][1]->fill(0);

	/* allocate alternate bitmaps if needed */
	if (mo->vrammask.mask != 0)
	{
		mo->vram[1][0] = auto_bitmap_ind16_alloc(machine, width, height);
		mo->vram[1][1] = auto_bitmap_ind16_alloc(machine, width, height);
		mo->vram[1][0]->fill(0);
		mo->vram[1][1]->fill(0);
	}

	mo->partial_scanline = -1;

	/* register for save states */
	device->save_pointer(NAME(mo->spriteram[0].data), ARRAY_LENGTH(mo->spriteram[0].data) * mo->spriteramsize);
	device->save_item(NAME(*mo->vram[0][0]));
	device->save_item(NAME(*mo->vram[0][1]));
	if (mo->vrammask.mask != 0)
	{
		device->save_item(NAME(*mo->vram[1][0]));
		device->save_item(NAME(*mo->vram[1][1]));
	}
	device->save_item(NAME(mo->partial_scanline));
	device->save_item(NAME(mo->control_bits));
	device->save_item(NAME(mo->command));
	device->save_item(NAME(mo->is32bit));
	device->save_item(NAME(mo->checksums));
}


const device_type ATARIRLE = &device_creator<atarirle_device>;

atarirle_device::atarirle_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ATARIRLE, "Atari RLE", tag, owner, clock)
{
	m_token = global_alloc_clear(atarirle_data);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void atarirle_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void atarirle_device::device_start()
{
	DEVICE_START_NAME( atarirle )(this);
}



/*---------------------------------------------------------------
    atarirle_control_w: Write handler for MO control bits.
---------------------------------------------------------------*/

void atarirle_control_w(device_t *device, UINT8 bits)
{
	atarirle_data *mo = get_safe_token(device);
	int scanline = device->machine().primary_screen->vpos();
	int oldbits = mo->control_bits;

//logerror("atarirle_control_w(%d)\n", bits);

	/* do nothing if nothing changed */
	if (oldbits == bits)
		return;

	/* force a partial update first */
	device->machine().primary_screen->update_partial(scanline);

	/* if the erase flag was set, erase the front map */
	if (oldbits & ATARIRLE_CONTROL_ERASE)
	{
		rectangle cliprect = mo->cliprect;

		/* compute the top and bottom of the rect */
		if (mo->partial_scanline + 1 > cliprect.min_y)
			cliprect.min_y = mo->partial_scanline + 1;
		if (scanline < cliprect.max_y)
			cliprect.max_y = scanline;

//logerror("  partial erase %d-%d (frame %d)\n", cliprect.min_y, cliprect.max_y, (oldbits & ATARIRLE_CONTROL_FRAME) >> 2);

		/* erase the bitmap */
		mo->vram[0][(oldbits & ATARIRLE_CONTROL_FRAME) >> 2]->fill(0, cliprect);
		if (mo->vrammask.mask != 0)
			mo->vram[1][(oldbits & ATARIRLE_CONTROL_FRAME) >> 2]->fill(0, cliprect);
	}

	/* update the bits */
	mo->control_bits = bits;

	/* if mogo is set, do a render on the rising edge */
	if (!(oldbits & ATARIRLE_CONTROL_MOGO) && (bits & ATARIRLE_CONTROL_MOGO))
	{
		if (mo->command == ATARIRLE_COMMAND_DRAW)
			sort_and_render(device->machine(), mo);
		else if (mo->command == ATARIRLE_COMMAND_CHECKSUM)
			compute_checksum(mo);
	}

	/* remember where we left off */
	mo->partial_scanline = scanline;
}



/*---------------------------------------------------------------
    atarirle_command_w: Write handler for MO command bits.
---------------------------------------------------------------*/

void atarirle_command_w(device_t *device, UINT8 command)
{
	atarirle_data *mo = get_safe_token(device);
	mo->command = command;
}



/*---------------------------------------------------------------
    atarirle_eof: Flush remaining changes.
---------------------------------------------------------------*/

void atarirle_eof(device_t *device)
{
	{
		atarirle_data *mo = get_safe_token(device);

		/* if the erase flag is set, erase to the end of the screen */
		if (mo->control_bits & ATARIRLE_CONTROL_ERASE)
		{
			rectangle cliprect = mo->cliprect;

			/* compute top only; bottom is equal to visible_area */
			if (mo->partial_scanline + 1 > cliprect.min_y)
				cliprect.min_y = mo->partial_scanline + 1;

//logerror("  partial erase %d-%d (frame %d)\n", cliprect.min_y, cliprect.max_y, (mo->control_bits & ATARIRLE_CONTROL_FRAME) >> 2);

			/* erase the bitmap */
			mo->vram[0][(mo->control_bits & ATARIRLE_CONTROL_FRAME) >> 2]->fill(0, cliprect);
			if (mo->vrammask.mask != 0)
				mo->vram[1][(mo->control_bits & ATARIRLE_CONTROL_FRAME) >> 2]->fill(0, cliprect);
		}

		/* reset the partial scanline to -1 so we can detect full updates */
		mo->partial_scanline = -1;
	}
}



/*---------------------------------------------------------------
    atarirle_spriteram_r: Read handler for the spriteram.
---------------------------------------------------------------*/

READ16_DEVICE_HANDLER( atarirle_spriteram_r )
{
	atarirle_data *mo = get_safe_token(device);

	return mo->ram[offset];
}



/*---------------------------------------------------------------
    atarirle_spriteram_w: Write handler for the spriteram.
---------------------------------------------------------------*/

WRITE16_DEVICE_HANDLER( atarirle_spriteram_w )
{
	atarirle_data *mo = get_safe_token(device);
	int entry = (offset >> 3) & mo->spriterammask;
	int idx = offset & 7;

	/* combine raw data */
	COMBINE_DATA(&mo->ram[offset]);

	/* store a copy in our local spriteram */
	mo->spriteram[entry].data[idx] = mo->ram[offset];
	mo->is32bit = 0;
}



/*---------------------------------------------------------------
    atarirle_spriteram32_r: Read handler for the spriteram.
---------------------------------------------------------------*/

READ32_DEVICE_HANDLER( atarirle_spriteram32_r )
{
	atarirle_data *mo = get_safe_token(device);
	UINT32 *ram = (UINT32 *)mo->ram;

	return ram[offset];
}



/*---------------------------------------------------------------
    atarirle_spriteram32_w: Write handler for the spriteram.
---------------------------------------------------------------*/

WRITE32_DEVICE_HANDLER( atarirle_spriteram32_w )
{
	atarirle_data *mo = get_safe_token(device);
	UINT32 *ram = (UINT32 *)mo->ram;
	int entry = (offset >> 2) & mo->spriterammask;
	int idx = 2 * (offset & 3);

	/* combine raw data */
	COMBINE_DATA(&ram[offset]);

	/* store a copy in our local spriteram */
	mo->spriteram[entry].data[idx+0] = ram[offset] >> 16;
	mo->spriteram[entry].data[idx+1] = ram[offset];
	mo->is32bit = 1;
}



/*---------------------------------------------------------------
    atarirle_get_vram: Return the VRAM bitmap.
---------------------------------------------------------------*/

bitmap_ind16 *atarirle_get_vram(device_t *device, int idx)
{
	atarirle_data *mo = get_safe_token(device);
//logerror("atarirle_get_vram (frame %d)\n", (mo->control_bits & ATARIRLE_CONTROL_FRAME) >> 2);
	return mo->vram[idx][(mo->control_bits & ATARIRLE_CONTROL_FRAME) >> 2];
}



/*---------------------------------------------------------------
    build_rle_tables: Builds internal table for RLE mapping.
---------------------------------------------------------------*/

static void build_rle_tables(running_machine &machine)
{
	UINT16 *base;
	int i;

	/* allocate all 5 tables */
	base = auto_alloc_array(machine, UINT16, 0x500);

	/* assign the tables */
	rle_table[0] = &base[0x000];
	rle_table[1] = &base[0x100];
	rle_table[2] = rle_table[3] = &base[0x200];
	rle_table[4] = rle_table[6] = &base[0x300];
	rle_table[5] = rle_table[7] = &base[0x400];

	/* set the bpps */
	rle_bpp[0] = 4;
	rle_bpp[1] = rle_bpp[2] = rle_bpp[3] = 5;
	rle_bpp[4] = rle_bpp[5] = rle_bpp[6] = rle_bpp[7] = 6;

	/* build the 4bpp table */
	for (i = 0; i < 256; i++)
		rle_table[0][i] = (((i & 0xf0) + 0x10) << 4) | (i & 0x0f);

	/* build the 5bpp table */
	for (i = 0; i < 256; i++)
		rle_table[2][i] = (((i & 0xe0) + 0x20) << 3) | (i & 0x1f);

	/* build the special 5bpp table */
	for (i = 0; i < 256; i++)
	{
		if ((i & 0x0f) == 0)
			rle_table[1][i] = (((i & 0xf0) + 0x10) << 4) | (i & 0x0f);
		else
			rle_table[1][i] = (((i & 0xe0) + 0x20) << 3) | (i & 0x1f);
	}

	/* build the 6bpp table */
	for (i = 0; i < 256; i++)
		rle_table[5][i] = (((i & 0xc0) + 0x40) << 2) | (i & 0x3f);

	/* build the special 6bpp table */
	for (i = 0; i < 256; i++)
	{
		if ((i & 0x0f) == 0)
			rle_table[4][i] = (((i & 0xf0) + 0x10) << 4) | (i & 0x0f);
		else
			rle_table[4][i] = (((i & 0xc0) + 0x40) << 2) | (i & 0x3f);
	}
}



/*---------------------------------------------------------------
    count_objects: Determines the number of objects in the
    motion object ROM.
---------------------------------------------------------------*/

int count_objects(const UINT16 *base, int length)
{
	int lowest_address = length;
	int i;

	/* first determine the lowest address of all objects */
	for (i = 0; i < lowest_address; i += 4)
	{
		int offset = ((base[i + 2] & 0xff) << 16) | base[i + 3];
//logerror("count_objects: i=%d offset=%08X\n", i, offset);
		if (offset > i && offset < lowest_address)
			lowest_address = offset;
	}

	/* that determines how many objects */
	return lowest_address / 4;
}



/*---------------------------------------------------------------
    prescan_rle: Prescans an RLE object, computing the
    width, height, and other goodies.
---------------------------------------------------------------*/

static void prescan_rle(const atarirle_data *mo, int which)
{
	atarirle_info *rledata = &mo->info[which];
	UINT16 *base = (UINT16 *)&mo->rombase[which * 4];
	const UINT16 *end = mo->rombase + mo->romlength / 2;
	int width = 0, height, flags, offset;
	const UINT16 *table;

	/* look up the offset */
	rledata->xoffs = (INT16)base[0];
	rledata->yoffs = (INT16)base[1];

	/* determine the depth and table */
	flags = base[2];
	rledata->bpp = rle_bpp[(flags >> 8) & 7];
	table = rledata->table = rle_table[(flags >> 8) & 7];

	/* determine the starting offset */
	offset = ((base[2] & 0xff) << 16) | base[3];
	rledata->data = base = (UINT16 *)&mo->rombase[offset];

	/* make sure it's valid */
	if (offset < which * 4 || offset >= mo->romlength)
	{
		memset(rledata, 0, sizeof(*rledata));
		return;
	}

	/* first pre-scan to determine the width and height */
	for (height = 0; height < 1024 && base < end; height++)
	{
		int tempwidth = 0;
		int entry_count = *base++;

		/* if the high bit is set, assume we're inverted */
		if (entry_count & 0x8000)
		{
			entry_count ^= 0xffff;

			/* also change the ROM data so we don't have to do this again at runtime */
			base[-1] ^= 0xffff;
		}

		/* we're done when we hit 0 */
		if (entry_count == 0)
			break;

		/* track the width */
		while (entry_count-- && base < end)
		{
			int word = *base++;
			int count/*, value*/;

			/* decode the low byte first */
			count = table[word & 0xff];
			//value = count & 0xff;
			tempwidth += count >> 8;

			/* decode the upper byte second */
			count = table[word >> 8];
			//value = count & 0xff;
			tempwidth += count >> 8;
		}

		/* only remember the max */
		if (tempwidth > width)
			width = tempwidth;
	}

	/* fill in the data */
	rledata->width = width;
	rledata->height = height;
}



/*---------------------------------------------------------------
    compute_checksum: Compute the checksum values on the ROMs.
---------------------------------------------------------------*/

static void compute_checksum(atarirle_data *mo)
{
	int reqsums = mo->spriteram[0].data[0] + 1;
	int i;

	/* number of checksums is in the first word */
	if (reqsums > 256)
		reqsums = 256;

	/* stuff them back */
	if (!mo->is32bit)
	{
		for (i = 0; i < reqsums; i++)
			mo->ram[i] = mo->checksums[i];
	}
	else
	{
		UINT32 *ram = (UINT32 *)mo->ram;
		for (i = 0; i < reqsums; i++)
			if (i & 1)
				ram[i/2] = (ram[i/2] & 0xffff0000) | mo->checksums[i];
			else
				ram[i/2] = (ram[i/2] & 0x0000ffff) | (mo->checksums[i] << 16);
	}
}


/*---------------------------------------------------------------
    sort_and_render: Render all motion objects in order.
---------------------------------------------------------------*/

static void sort_and_render(running_machine &machine, atarirle_data *mo)
{
	bitmap_ind16 *bitmap1 = mo->vram[0][(~mo->control_bits & ATARIRLE_CONTROL_FRAME) >> 2];
	bitmap_ind16 *bitmap2 = mo->vram[1][(~mo->control_bits & ATARIRLE_CONTROL_FRAME) >> 2];
	atarirle_entry *obj = mo->spriteram;
	mo_sort_entry sort_entry[256];
	mo_sort_entry *list_head[256];
	mo_sort_entry *current;
	int i;

atarirle_entry *hilite = NULL;
int count = 0;

	/* sort the motion objects into their proper priorities */
	memset(list_head, 0, sizeof(list_head));
	for (i = 0; i < 256; i++, obj++)
	{
		int order = EXTRACT_DATA(obj, mo->ordermask);
		sort_entry[i].entry = i;
		sort_entry[i].next = list_head[order];
		list_head[order] = &sort_entry[i];
	}

	/* now loop back and process */
	count = 0;
	for (i = 1; i < 256; i++)
		for (current = list_head[i]; current; current = current->next)
		{
			int scale, code;

			/* extract scale and code */
			obj = &mo->spriteram[current->entry];
			scale = EXTRACT_DATA(obj, mo->scalemask);
			code = EXTRACT_DATA(obj, mo->codemask);

			/* make sure they are in range */
			if (scale > 0 && code < mo->objectcount)
			{
				int hflip = EXTRACT_DATA(obj, mo->hflipmask);
				int color = EXTRACT_DATA(obj, mo->colormask);
				int priority = EXTRACT_DATA(obj, mo->prioritymask);
				int x = EXTRACT_DATA(obj, mo->xposmask);
				int y = EXTRACT_DATA(obj, mo->yposmask);
				int which = EXTRACT_DATA(obj, mo->vrammask);

if (count++ == atarirle_hilite_index)
	hilite = obj;

				if (x & ((mo->xposmask.mask + 1) >> 1))
					x = (INT16)(x | ~mo->xposmask.mask);
				if (y & ((mo->yposmask.mask + 1) >> 1))
					y = (INT16)(y | ~mo->yposmask.mask);
				x += mo->cliprect.min_x;

				/* merge priority and color */
				color = (color << 4) | (priority << ATARIRLE_PRIORITY_SHIFT);

				/* render to one or both bitmaps */
				if (which == 0)
					draw_rle(mo, *bitmap1, code, color, hflip, 0, x, y, scale, scale, mo->cliprect);
				if (bitmap2 && which != 0)
					draw_rle(mo, *bitmap2, code, color, hflip, 0, x, y, scale, scale, mo->cliprect);
			}
		}

if (hilite)
{
	int scale, code/*, which*/;

	/* extract scale and code */
	obj = hilite;
	scale = EXTRACT_DATA(obj, mo->scalemask);
	code = EXTRACT_DATA(obj, mo->codemask);
	//which = EXTRACT_DATA(obj, mo->vrammask);

	/* make sure they are in range */
	if (scale > 0 && code < mo->objectcount)
	{
		int hflip = EXTRACT_DATA(obj, mo->hflipmask);
		int color = EXTRACT_DATA(obj, mo->colormask);
		int priority = EXTRACT_DATA(obj, mo->prioritymask);
		int x = EXTRACT_DATA(obj, mo->xposmask);
		int y = EXTRACT_DATA(obj, mo->yposmask);
		int scaled_xoffs, scaled_yoffs;
		const atarirle_info *info;

		if (x & ((mo->xposmask.mask + 1) >> 1))
			x = (INT16)(x | ~mo->xposmask.mask);
		if (y & ((mo->yposmask.mask + 1) >> 1))
			y = (INT16)(y | ~mo->yposmask.mask);
		x += mo->cliprect.min_x;

		/* merge priority and color */
		color = (color << 4) | (priority << ATARIRLE_PRIORITY_SHIFT);

		info = &mo->info[code];
		scaled_xoffs = (scale * info->xoffs) >> 12;
		scaled_yoffs = (scale * info->yoffs) >> 12;

		/* we're hflipped, account for it */
		if (hflip)
			scaled_xoffs = ((scale * info->width) >> 12) - scaled_xoffs;

		/* adjust for the x and y offsets */
		x -= scaled_xoffs;
		y -= scaled_yoffs;

		do
		{
			const rectangle &visarea = machine.primary_screen->visible_area();
			int scaled_width = (scale * info->width + 0x7fff) >> 12;
			int scaled_height = (scale * info->height + 0x7fff) >> 12;
			int ex, ey, sx = x, sy = y, tx, ty;

			/* make sure we didn't end up with 0 */
			if (scaled_width == 0) scaled_width = 1;
			if (scaled_height == 0) scaled_height = 1;

			/* compute the remaining parameters */
			ex = sx + scaled_width - 1;
			ey = sy + scaled_height - 1;

			/* left edge clip */
			if (sx < visarea.min_x)
				sx = visarea.min_x;
			if (sx > visarea.max_x)
				break;

			/* right edge clip */
			if (ex > visarea.max_x)
				ex = visarea.max_x;
			else if (ex < visarea.min_x)
				break;

			/* top edge clip */
			if (sy < visarea.min_y)
				sy = visarea.min_y;
			else if (sy > visarea.max_y)
				break;

			/* bottom edge clip */
			if (ey > visarea.max_y)
				ey = visarea.max_y;
			else if (ey < visarea.min_y)
				break;

			for (ty = sy; ty <= ey; ty++)
			{
				bitmap1->pix16(ty, sx) = machine.rand() & 0xff;
				bitmap1->pix16(ty, ex) = machine.rand() & 0xff;
			}
			for (tx = sx; tx <= ex; tx++)
			{
				bitmap1->pix16(sy, tx) = machine.rand() & 0xff;
				bitmap1->pix16(ey, tx) = machine.rand() & 0xff;
			}
		} while (0);
fprintf(stderr, "   Sprite: c=%04X l=%04X h=%d X=%4d (o=%4d w=%3d) Y=%4d (o=%4d h=%d) s=%04X\n",
	code, color, hflip,
	x, -scaled_xoffs, (scale * info->width) >> 12,
	y, -scaled_yoffs, (scale * info->height) >> 12, scale);
	}

}
}



/*---------------------------------------------------------------
    draw_rle: Render a single RLE-compressed motion
    object.
---------------------------------------------------------------*/

void draw_rle(atarirle_data *mo, bitmap_ind16 &bitmap, int code, int color, int hflip, int vflip,
	int x, int y, int xscale, int yscale, const rectangle &clip)
{
	UINT32 palettebase = mo->palettebase + color;
	const atarirle_info *info = &mo->info[code];
	int scaled_xoffs = (xscale * info->xoffs) >> 12;
	int scaled_yoffs = (yscale * info->yoffs) >> 12;

	/* we're hflipped, account for it */
	if (hflip)
		scaled_xoffs = ((xscale * info->width) >> 12) - scaled_xoffs;

//if (clip.min_y == Machine->primary_screen->visible_area().min_y)
//logerror("   Sprite: c=%04X l=%04X h=%d X=%4d (o=%4d w=%3d) Y=%4d (o=%4d h=%d) s=%04X\n",
//  code, color, hflip,
//  x, -scaled_xoffs, (xscale * info->width) >> 12,
//  y, -scaled_yoffs, (yscale * info->height) >> 12, xscale);

	/* adjust for the x and y offsets */
	x -= scaled_xoffs;
	y -= scaled_yoffs;

	/* bail on a NULL object */
	if (!info->data)
		return;

	/* 16-bit case */
	assert(bitmap.bpp() == 16);
	if (!hflip)
		draw_rle_zoom(bitmap, info, palettebase, x, y, xscale << 4, yscale << 4, clip);
	else
		draw_rle_zoom_hflip(bitmap, info, palettebase, x, y, xscale << 4, yscale << 4, clip);
}



/*---------------------------------------------------------------
    draw_rle_zoom: Draw an RLE-compressed object to a 16-bit
    bitmap.
---------------------------------------------------------------*/

void draw_rle_zoom(bitmap_ind16 &bitmap, const atarirle_info *gfx,
		UINT32 palette, int sx, int sy, int scalex, int scaley,
		const rectangle &clip)
{
	const UINT16 *row_start = gfx->data;
	const UINT16 *table = gfx->table;
	volatile int current_row = 0;

	int scaled_width = (scalex * gfx->width + 0x7fff) >> 16;
	int scaled_height = (scaley * gfx->height + 0x7fff) >> 16;

	int pixels_to_skip = 0, xclipped = 0;
	int dx, dy, ex, ey;
	int y, sourcey;

	/* make sure we didn't end up with 0 */
	if (scaled_width == 0) scaled_width = 1;
	if (scaled_height == 0) scaled_height = 1;

	/* compute the remaining parameters */
	dx = (gfx->width << 16) / scaled_width;
	dy = (gfx->height << 16) / scaled_height;
	ex = sx + scaled_width - 1;
	ey = sy + scaled_height - 1;
	sourcey = dy / 2;

	/* left edge clip */
	if (sx < clip.min_x)
		pixels_to_skip = clip.min_x - sx, xclipped = 1;
	if (sx > clip.max_x)
		return;

	/* right edge clip */
	if (ex > clip.max_x)
		ex = clip.max_x, xclipped = 1;
	else if (ex < clip.min_x)
		return;

	/* top edge clip */
	if (sy < clip.min_y)
	{
		sourcey += (clip.min_y - sy) * dy;
		sy = clip.min_y;
	}
	else if (sy > clip.max_y)
		return;

	/* bottom edge clip */
	if (ey > clip.max_y)
		ey = clip.max_y;
	else if (ey < clip.min_y)
		return;

	/* loop top to bottom */
	for (y = sy; y <= ey; y++, sourcey += dy)
	{
		UINT16 *dest = &bitmap.pix16(y, sx);
		int j, sourcex = dx / 2, rle_end = 0;
		const UINT16 *base;
		int entry_count;

		/* loop until we hit the row we're on */
		for ( ; current_row != (sourcey >> 16); current_row++)
			row_start += 1 + *row_start;

		/* grab our starting parameters from this row */
		base = row_start;
		entry_count = *base++;

		/* non-clipped case */
		if (!xclipped)
		{
			/* decode the pixels */
			for (j = 0; j < entry_count; j++)
			{
				int word = *base++;
				int count, value;

				/* decode the low byte first */
				count = table[word & 0xff];
				value = count & 0xff;
				rle_end += (count & 0xff00) << 8;

				/* store copies of the value until we pass the end of this chunk */
				if (value)
				{
					value += palette;
					while (sourcex < rle_end)
						*dest++ = value, sourcex += dx;
				}
				else
				{
					while (sourcex < rle_end)
						dest++, sourcex += dx;
				}

				/* decode the upper byte second */
				count = table[word >> 8];
				value = count & 0xff;
				rle_end += (count & 0xff00) << 8;

				/* store copies of the value until we pass the end of this chunk */
				if (value)
				{
					value += palette;
					while (sourcex < rle_end)
						*dest++ = value, sourcex += dx;
				}
				else
				{
					while (sourcex < rle_end)
						dest++, sourcex += dx;
				}
			}
		}

		/* clipped case */
		else
		{
			const UINT16 *end = &bitmap.pix16(y, ex);
			int to_be_skipped = pixels_to_skip;

			/* decode the pixels */
			for (j = 0; j < entry_count && dest <= end; j++)
			{
				int word = *base++;
				int count, value;

				/* decode the low byte first */
				count = table[word & 0xff];
				value = count & 0xff;
				rle_end += (count & 0xff00) << 8;

				/* store copies of the value until we pass the end of this chunk */
				if (to_be_skipped)
				{
					while (to_be_skipped && sourcex < rle_end)
						dest++, sourcex += dx, to_be_skipped--;
					if (to_be_skipped) goto next3;
				}
				if (value)
				{
					value += palette;
					while (sourcex < rle_end && dest <= end)
						*dest++ = value, sourcex += dx;
				}
				else
				{
					while (sourcex < rle_end)
						dest++, sourcex += dx;
				}

			next3:
				/* decode the upper byte second */
				count = table[word >> 8];
				value = count & 0xff;
				rle_end += (count & 0xff00) << 8;

				/* store copies of the value until we pass the end of this chunk */
				if (to_be_skipped)
				{
					while (to_be_skipped && sourcex < rle_end)
						dest++, sourcex += dx, to_be_skipped--;
					if (to_be_skipped) goto next4;
				}
				if (value)
				{
					value += palette;
					while (sourcex < rle_end && dest <= end)
						*dest++ = value, sourcex += dx;
				}
				else
				{
					while (sourcex < rle_end)
						dest++, sourcex += dx;
				}
			next4:
				;
			}
		}
	}
}



/*---------------------------------------------------------------
    draw_rle_zoom_hflip: Draw an RLE-compressed object to a
    16-bit bitmap with horizontal flip.
---------------------------------------------------------------*/

void draw_rle_zoom_hflip(bitmap_ind16 &bitmap, const atarirle_info *gfx,
		UINT32 palette, int sx, int sy, int scalex, int scaley,
		const rectangle &clip)
{
	const UINT16 *row_start = gfx->data;
	const UINT16 *table = gfx->table;
	volatile int current_row = 0;

	int scaled_width = (scalex * gfx->width + 0x7fff) >> 16;
	int scaled_height = (scaley * gfx->height + 0x7fff) >> 16;
	int pixels_to_skip = 0, xclipped = 0;
	int dx, dy, ex, ey;
	int y, sourcey;

	/* make sure we didn't end up with 0 */
	if (scaled_width == 0) scaled_width = 1;
	if (scaled_height == 0) scaled_height = 1;

	/* compute the remaining parameters */
	dx = (gfx->width << 16) / scaled_width;
	dy = (gfx->height << 16) / scaled_height;
	ex = sx + scaled_width - 1;
	ey = sy + scaled_height - 1;
	sourcey = dy / 2;

	/* left edge clip */
	if (sx < clip.min_x)
		sx = clip.min_x, xclipped = 1;
	if (sx > clip.max_x)
		return;

	/* right edge clip */
	if (ex > clip.max_x)
		pixels_to_skip = ex - clip.max_x, xclipped = 1;
	else if (ex < clip.min_x)
		return;

	/* top edge clip */
	if (sy < clip.min_y)
	{
		sourcey += (clip.min_y - sy) * dy;
		sy = clip.min_y;
	}
	else if (sy > clip.max_y)
		return;

	/* bottom edge clip */
	if (ey > clip.max_y)
		ey = clip.max_y;
	else if (ey < clip.min_y)
		return;

	/* loop top to bottom */
	for (y = sy; y <= ey; y++, sourcey += dy)
	{
		UINT16 *dest = &bitmap.pix16(y, ex);
		int j, sourcex = dx / 2, rle_end = 0;
		const UINT16 *base;
		int entry_count;

		/* loop until we hit the row we're on */
		for ( ; current_row != (sourcey >> 16); current_row++)
			row_start += 1 + *row_start;

		/* grab our starting parameters from this row */
		base = row_start;
		entry_count = *base++;

		/* non-clipped case */
		if (!xclipped)
		{
			/* decode the pixels */
			for (j = 0; j < entry_count; j++)
			{
				int word = *base++;
				int count, value;

				/* decode the low byte first */
				count = table[word & 0xff];
				value = count & 0xff;
				rle_end += (count & 0xff00) << 8;

				/* store copies of the value until we pass the end of this chunk */
				if (value)
				{
					value += palette;
					while (sourcex < rle_end)
						*dest-- = value, sourcex += dx;
				}
				else
				{
					while (sourcex < rle_end)
						dest--, sourcex += dx;
				}

				/* decode the upper byte second */
				count = table[word >> 8];
				value = count & 0xff;
				rle_end += (count & 0xff00) << 8;

				/* store copies of the value until we pass the end of this chunk */
				if (value)
				{
					value += palette;
					while (sourcex < rle_end)
						*dest-- = value, sourcex += dx;
				}
				else
				{
					while (sourcex < rle_end)
						dest--, sourcex += dx;
				}
			}
		}

		/* clipped case */
		else
		{
			const UINT16 *start = &bitmap.pix16(y, sx);
			int to_be_skipped = pixels_to_skip;

			/* decode the pixels */
			for (j = 0; j < entry_count && dest >= start; j++)
			{
				int word = *base++;
				int count, value;

				/* decode the low byte first */
				count = table[word & 0xff];
				value = count & 0xff;
				rle_end += (count & 0xff00) << 8;

				/* store copies of the value until we pass the end of this chunk */
				if (to_be_skipped)
				{
					while (to_be_skipped && sourcex < rle_end)
						dest--, sourcex += dx, to_be_skipped--;
					if (to_be_skipped) goto next3;
				}
				if (value)
				{
					value += palette;
					while (sourcex < rle_end && dest >= start)
						*dest-- = value, sourcex += dx;
				}
				else
				{
					while (sourcex < rle_end)
						dest--, sourcex += dx;
				}

			next3:
				/* decode the upper byte second */
				count = table[word >> 8];
				value = count & 0xff;
				rle_end += (count & 0xff00) << 8;

				/* store copies of the value until we pass the end of this chunk */
				if (to_be_skipped)
				{
					while (to_be_skipped && sourcex < rle_end)
						dest--, sourcex += dx, to_be_skipped--;
					if (to_be_skipped) goto next4;
				}
				if (value)
				{
					value += palette;
					while (sourcex < rle_end && dest >= start)
						*dest-- = value, sourcex += dx;
				}
				else
				{
					while (sourcex < rle_end)
						dest--, sourcex += dx;
				}
			next4:
				;
			}
		}
	}
}


/***************************************************************************

    The mapping of the bits from the PROMs is like this:

        D23 ->
        D22 ->
        D21 ->
        D20 ->
        D19 ->
        D18 ->
        D17 -> instruction bit 8
        D16 -> instruction bit 7

        D15 -> instruction bit 6
        D14 -> instruction bit 5
        D13 -> instruction bit 4
        D12 -> instruction bit 3
        D11 -> instruction bit 2
        D10 -> instruction bit 1 (modified via a PAL)
        D9  -> instruction bit 0
        D8  -> carry in

        D7  -> A bit 3
        D6  -> A bit 2
        D5  -> A bit 1
        D4  -> A bit 0
        D3  -> B bit 3
        D2  -> B bit 2
        D1  -> B bit 1
        D0  -> B bit 0

    Although much of the logic is contained in the ALU, the program counter
    is fed externally. Jumps are decoded like this:

        if (D13 && D14)
        {
            switch (D11 | D10 | D9)
            {
                case 0: condition = true;
                case 1: condition = ALU.LT;
                case 2: condition = ALU.Z;
                case 3: condition = ALU.N;
                case 4: condition = BLT.HFLIP;
                case 5: condition = /BLT.SCAN;
                case 6: condition = ALU.C;
                case 7: condition = MOTIMEP;
            }
            condition ^= D12;
            if (condition)
                PC = D8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0;
        }

    Here is the code from Guardians of the Hood:

                      I  C A B
                     --- - - -
    000: A0 C8 00 -> 064 0 0 0 (latch UC.RASCAS)
    001: 01 C8 66 -> 0E4 0 6 6
    002: 01 89 66 -> 0C4 1 6 6
    003: 03 82 66 -> 1C1 0 6 6
    004: 01 B8 65 -> 0DC 0 6 5
    005: 01 82 66 -> 0C1 0 6 6
    006: 01 89 55 -> 0C4 1 5 5
    007: 01 C8 77 -> 0E4 0 7 7
    008: 01 89 77 -> 0C4 1 7 7
    009: 03 82 77 -> 1C1 0 7 7
    00A: 03 82 77 -> 1C1 0 7 7
    00B: 03 82 77 -> 1C1 0 7 7
    00C: 03 82 77 -> 1C1 0 7 7
    00D: 01 C8 88 -> 0E4 0 8 8
    00E: 01 B8 79 -> 0DC 0 7 9
    00F: 03 82 99 -> 1C1 0 9 9
    010: 01 82 99 -> 0C1 0 9 9
    011: 01 B8 61 -> 0DC 0 6 1
    012: 01 82 11 -> 0C1 0 1 1
    013: 01 96 11 -> 0CB 0 1 1
    014: 03 82 11 -> 1C1 0 1 1
    015: 03 82 11 -> 1C1 0 1 1
    016: 03 82 11 -> 1C1 0 1 1
    017: 03 82 11 -> 1C1 0 1 1
    018: 00 EE 18 -> 077 0 1 8 if /MOTIMEP JUMP 018
    019: 84 B8 11 -> 05C 0 1 1
    01A: 01 B8 72 -> 0DC 0 7 2
    01B: 00 C8 00 -> 064 0 0 0
    01C: 9C B8 88 -> 05C 0 8 8
    01D: 98 B8 88 -> 05C 0 8 8
    01E: 01 96 22 -> 0CB 0 2 2
    01F: 00 F4 1C -> 07A 0 1 C if NZ JUMP 01C
    020: 00 38 11 -> 01C 0 1 1
    021: 03 34 00 -> 19A 0 0 0
    022: 03 34 00 -> 19A 0 0 0
    023: 03 34 00 -> 19A 0 0 0
    024: 00 11 99 -> 008 1 9 9
    025: 02 34 00 -> 11A 0 0 0
    026: 84 B4 00 -> 05A 0 0 0
    027: 00 C8 00 -> 064 0 0 0
    028: 00 C8 00 -> 064 0 0 0
    029: 10 3E 00 -> 01F 0 0 0
    02A: 00 E5 3B -> 072 1 3 B if Z JUMP 13B
    02B: 00 05 00 -> 002 1 0 0
    02C: 9C B4 00 -> 05A 0 0 0
    02D: 98 B4 00 -> 05A 0 0 0
    02E: 00 EE 2E -> 077 0 2 E if /MOTIMEP JUMP 02E
    02F: 10 3E 00 -> 01F 0 0 0
    030: 00 05 00 -> 002 1 0 0
    031: 00 F4 35 -> 07A 0 3 5 if NZ JUMP 035
    032: A8 B8 55 -> 05C 0 5 5 (latch UC.HFLIP)
    033: A8 FE 35 -> 07F 0 3 5 if MOTIMEP JUMP 035 (latch UC.HFLIP)
    034: 00 E1 47 -> 070 1 4 7 JUMP 147
    035: 01 B8 72 -> 0DC 0 7 2
    036: 01 B8 8F -> 0DC 0 8 F
    037: 01 83 5F -> 0C1 1 5 F
    038: 84 B8 FF -> 05C 0 F F
    039: 00 EE 39 -> 077 0 3 9 if /MOTIMEP JUMP 039
    03A: 19 BE FF -> 0DF 0 F F
    03B: 00 E4 68 -> 072 0 6 8 if Z JUMP 068
    03C: 84 B8 88 -> 05C 0 8 8
    03D: 00 EE 3D -> 077 0 3 D if /MOTIMEP JUMP 03D
    03E: 19 BE AA -> 0DF 0 A A
    03F: 00 EE 3F -> 077 0 3 F if /MOTIMEP JUMP 03F
    040: 19 BE BB -> 0DF 0 B B
    041: 00 EE 41 -> 077 0 4 1 if /MOTIMEP JUMP 041
    042: 19 BE CC -> 0DF 0 C C
    043: 00 EE 43 -> 077 0 4 3 if /MOTIMEP JUMP 043
    044: 19 BE DD -> 0DF 0 D D
    045: 00 EE 45 -> 077 0 4 5 if /MOTIMEP JUMP 045
    046: 19 BE EE -> 0DF 0 E E
    047: 00 C8 00 -> 064 0 0 0
    048: 01 82 1F -> 0C1 0 1 F
    049: 84 B8 FF -> 05C 0 F F
    04A: 11 BE FF -> 0DF 0 F F
    04B: 00 C8 00 -> 064 0 0 0
    04C: 00 C8 00 -> 064 0 0 0
    04D: 9C B8 99 -> 05C 0 9 9
    04E: 90 B8 99 -> 05C 0 9 9
    04F: 00 C8 00 -> 064 0 0 0
    050: 84 B8 99 -> 05C 0 9 9
    051: 9C B8 AA -> 05C 0 A A
    052: 98 B8 AA -> 05C 0 A A
    053: 00 C8 00 -> 064 0 0 0
    054: 00 C8 00 -> 064 0 0 0
    055: 9C B8 BB -> 05C 0 B B
    056: 98 B8 BB -> 05C 0 B B
    057: 00 C8 00 -> 064 0 0 0
    058: 00 C8 00 -> 064 0 0 0
    059: 9C B8 CC -> 05C 0 C C
    05A: 98 B8 CC -> 05C 0 C C
    05B: 00 C8 00 -> 064 0 0 0
    05C: 00 C8 00 -> 064 0 0 0
    05D: 9C B8 DD -> 05C 0 D D
    05E: 98 B8 DD -> 05C 0 D D
    05F: 00 C8 00 -> 064 0 0 0
    060: 00 C8 00 -> 064 0 0 0
    061: 9C B8 EE -> 05C 0 E E
    062: 98 B8 EE -> 05C 0 E E
    063: 00 C8 00 -> 064 0 0 0
    064: 00 C8 00 -> 064 0 0 0
    065: 9C B8 FF -> 05C 0 F F
    066: 98 B8 FF -> 05C 0 F F
    067: 01 82 69 -> 0C1 0 6 9
    068: 01 82 68 -> 0C1 0 6 8
    069: 01 96 22 -> 0CB 0 2 2
    06A: 00 F4 36 -> 07A 0 3 6 if NZ JUMP 036
    06B: 01 B8 72 -> 0DC 0 7 2
    06C: 01 96 22 -> 0CB 0 2 2
    06D: 01 C8 44 -> 0E4 0 4 4
    06E: 01 96 44 -> 0CB 0 4 4
    06F: 01 B8 43 -> 0DC 0 4 3
    070: 02 B8 44 -> 15C 0 4 4
    071: 02 B8 44 -> 15C 0 4 4
    072: 02 B8 44 -> 15C 0 4 4
    073: 02 B8 44 -> 15C 0 4 4
    074: 02 B8 33 -> 15C 0 3 3
    075: 01 93 43 -> 0C9 1 4 3
    076: 02 B8 33 -> 15C 0 3 3
    077: 00 38 33 -> 01C 0 3 3
    078: 01 C8 33 -> 0E4 0 3 3
    079: 01 89 33 -> 0C4 1 3 3
    07A: 01 82 33 -> 0C1 0 3 3
    07B: 01 C8 00 -> 0E4 0 0 0
    07C: 00 B8 00 -> 05C 0 0 0
    07D: 00 F4 87 -> 07A 0 8 7 if NZ JUMP 087
    07E: 01 89 11 -> 0C4 1 1 1
    07F: 84 B8 11 -> 05C 0 1 1
    080: 00 EE 80 -> 077 0 8 0 if /MOTIMEP JUMP 080
    081: 11 BE 00 -> 0DF 0 0 0
    082: A4 B8 22 -> 05C 0 2 2 (latch UC.COLOR)
    083: 01 96 22 -> 0CB 0 2 2
    084: 00 F6 7C -> 07B 0 7 C if P JUMP 07C
    085: 8C B8 22 -> 05C 0 2 2
    086: 00 E0 86 -> 070 0 8 6 JUMP 086
    087: 84 B8 00 -> 05C 0 0 0
    088: 00 EE 88 -> 077 0 8 8 if /MOTIMEP JUMP 088
    089: 19 BE 55 -> 0DF 0 5 5
    08A: A8 B8 55 -> 05C 0 5 5 (latch UC.HFLIP)
    08B: 03 82 55 -> 1C1 0 5 5
    08C: 00 C8 00 -> 064 0 0 0
    08D: 19 BE FF -> 0DF 0 F F
    08E: A4 B8 FF -> 05C 0 F F (latch UC.COLOR)
    08F: 00 C8 00 -> 064 0 0 0
    090: 00 C8 00 -> 064 0 0 0
    091: 19 BE 66 -> 0DF 0 6 6
    092: B0 C8 00 -> 064 0 0 0 (latch UC.FORMAT)
    093: 00 C8 00 -> 064 0 0 0
    094: 00 C8 00 -> 064 0 0 0
    095: 19 BE 77 -> 0DF 0 7 7
    096: 00 C8 00 -> 064 0 0 0
    097: 00 C8 00 -> 064 0 0 0
    098: 00 C8 00 -> 064 0 0 0
    099: 19 BE 88 -> 0DF 0 8 8
    09A: 00 C8 00 -> 064 0 0 0
    09B: 00 C8 00 -> 064 0 0 0
    09C: 00 C8 00 -> 064 0 0 0
    09D: 19 BE 00 -> 0DF 0 0 0
    09E: 84 B8 55 -> 05C 0 5 5
    09F: 00 C8 00 -> 064 0 0 0
    0A0: 00 C8 00 -> 064 0 0 0
    0A1: 09 BE DD -> 0DF 0 D D
    0A2: 00 C8 00 -> 064 0 0 0
    0A3: 00 C8 00 -> 064 0 0 0
    0A4: 09 BE EE -> 0DF 0 E E
    0A5: 01 C8 AA -> 0E4 0 A A
    0A6: 01 C8 BB -> 0E4 0 B B
    0A7: 09 BE 99 -> 0DF 0 9 9
    0A8: 01 B8 8F -> 0DC 0 8 F
    0A9: 01 82 FF -> 0C1 0 F F
    0AA: 09 BE 55 -> 0DF 0 5 5
    0AB: 01 C8 CC -> 0E4 0 C C
    0AC: 00 B8 DD -> 05C 0 D D
    0AD: 00 F6 B1 -> 07B 0 B 1 if P JUMP 0B1
    0AE: 01 A9 DD -> 0D4 1 D D
    0AF: 01 82 4C -> 0C1 0 4 C
    0B0: 01 89 CC -> 0C4 1 C C
    0B1: 01 82 CC -> 0C1 0 C C
    0B2: 00 B8 EE -> 05C 0 E E
    0B3: 00 F6 B7 -> 07B 0 B 7 if P JUMP 0B7
    0B4: 01 A9 EE -> 0D4 1 E E
    0B5: 01 82 4C -> 0C1 0 4 C
    0B6: 01 89 CC -> 0C4 1 C C
    0B7: 03 82 CC -> 1C1 0 C C
    0B8: 01 82 FF -> 0C1 0 F F
    0B9: 43 86 DA -> 1C3 0 D A
    0BA: 43 86 EB -> 1C3 0 E B
    0BB: 01 82 FF -> 0C1 0 F F
    0BC: 43 86 DA -> 1C3 0 D A
    0BD: 43 86 EB -> 1C3 0 E B
    0BE: 01 82 FF -> 0C1 0 F F
    0BF: 43 86 DA -> 1C3 0 D A
    0C0: 43 86 EB -> 1C3 0 E B
    0C1: 01 82 FF -> 0C1 0 F F
    0C2: 43 86 DA -> 1C3 0 D A
    0C3: 43 86 EB -> 1C3 0 E B
    0C4: 01 82 FF -> 0C1 0 F F
    0C5: 43 86 DA -> 1C3 0 D A
    0C6: 43 86 EB -> 1C3 0 E B
    0C7: 01 82 FF -> 0C1 0 F F
    0C8: 43 86 DA -> 1C3 0 D A
    0C9: 43 86 EB -> 1C3 0 E B
    0CA: 01 82 FF -> 0C1 0 F F
    0CB: 43 86 DA -> 1C3 0 D A
    0CC: 43 86 EB -> 1C3 0 E B
    0CD: 01 82 FF -> 0C1 0 F F
    0CE: 41 86 DA -> 0C3 0 D A
    0CF: 41 86 EB -> 0C3 0 E B
    0D0: 02 B8 DD -> 15C 0 D D
    0D1: 02 B8 EE -> 15C 0 E E
    0D2: 01 82 FF -> 0C1 0 F F
    0D3: 41 86 DA -> 0C3 0 D A
    0D4: 41 86 EB -> 0C3 0 E B
    0D5: 02 B8 DD -> 15C 0 D D
    0D6: 02 B8 EE -> 15C 0 E E
    0D7: 01 82 FF -> 0C1 0 F F
    0D8: 41 86 DA -> 0C3 0 D A
    0D9: 41 86 EB -> 0C3 0 E B
    0DA: 02 B8 DD -> 15C 0 D D
    0DB: 02 B8 EE -> 15C 0 E E
    0DC: 01 82 FF -> 0C1 0 F F
    0DD: 41 86 DA -> 0C3 0 D A
    0DE: 41 86 EB -> 0C3 0 E B
    0DF: 00 B8 CC -> 05C 0 C C
    0E0: 00 F6 E2 -> 07B 0 E 2 if P JUMP 0E2
    0E1: 01 A9 AA -> 0D4 1 A A
    0E2: 01 82 CC -> 0C1 0 C C
    0E3: 00 F6 E5 -> 07B 0 E 5 if P JUMP 0E5
    0E4: 01 A9 BB -> 0D4 1 B B
    0E5: 01 C8 CC -> 0E4 0 C C
    0E6: 01 89 CC -> 0C4 1 C C
    0E7: 03 82 CC -> 1C1 0 C C
    0E8: 03 82 CC -> 1C1 0 C C
    0E9: 03 82 CC -> 1C1 0 C C
    0EA: 01 B8 4F -> 0DC 0 4 F
    0EB: 03 82 FF -> 1C1 0 F F
    0EC: 00 D2 48 -> 069 0 4 8
    0ED: 00 E5 19 -> 072 1 1 9 if Z JUMP 119
    0EE: 02 B8 88 -> 15C 0 8 8
    0EF: 02 B8 88 -> 15C 0 8 8
    0F0: 02 B8 88 -> 15C 0 8 8
    0F1: 02 B8 88 -> 15C 0 8 8
    0F2: 01 80 88 -> 0C0 0 8 8
    0F3: 84 B8 88 -> 05C 0 8 8
    0F4: 00 EE F4 -> 077 0 F 4 if /MOTIMEP JUMP 0F4
    0F5: 19 BE 88 -> 0DF 0 8 8
    0F6: AC B8 88 -> 05C 0 8 8 (latch UC.RATE)
    0F7: 01 82 88 -> 0C1 0 8 8
    0F8: 02 B8 88 -> 15C 0 8 8
    0F9: 00 F8 FC -> 07C 0 F C if /BLT.HFLIP JUMP 0FC
    0FA: 01 82 A6 -> 0C1 0 A 6
    0FB: 00 E0 FD -> 070 0 F D JUMP 0FD
    0FC: 01 93 A6 -> 0C9 1 A 6
    0FD: 01 93 B7 -> 0C9 1 B 7
    0FE: B0 B8 99 -> 05C 0 9 9 (latch UC.FORMAT)
    0FF: 84 B8 55 -> 05C 0 5 5
    100: 01 C8 EE -> 0E4 0 E E
    101: 00 C8 00 -> 064 0 0 0
    102: 09 BE DD -> 0DF 0 D D
    103: 00 E6 7C -> 073 0 7 C if N JUMP 07C
    104: 00 93 F7 -> 049 1 F 7
    105: 00 ED 0B -> 076 1 0 B if C JUMP 10B
    106: B4 B8 DD -> 05C 0 D D (latch UC.TRANS)
    107: B8 B8 77 -> 05C 0 7 7 (latch UC.XYPOS)
    108: A0 B8 33 -> 05C 0 3 3 (latch UC.RASCAS)
    109: B8 B8 66 -> 05C 0 6 6 (latch UC.XYPOS)
    10A: BC C8 00 -> 064 0 0 0 (latch UC.SCAN)
    10B: 01 82 C7 -> 0C1 0 C 7
    10C: 01 82 8E -> 0C1 0 8 E
    10D: 00 D2 4E -> 069 0 4 E
    10E: 00 E5 14 -> 072 1 1 4 if Z JUMP 114
    10F: 01 93 4E -> 0C9 1 4 E
    110: 01 96 EE -> 0CB 0 E E
    111: 01 83 D5 -> 0C1 1 D 5
    112: 00 FD 14 -> 07E 1 1 4 if NC JUMP 114
    113: 01 89 99 -> 0C4 1 9 9
    114: 00 FB 14 -> 07D 1 1 4 if BLT.SCAN JUMP 114
    115: A0 C8 00 -> 064 0 0 0 (latch UC.RASCAS)
    116: 84 B8 55 -> 05C 0 5 5
    117: B0 B8 99 -> 05C 0 9 9 (latch UC.FORMAT)
    118: 00 E1 02 -> 070 1 0 2 JUMP 102
    119: AC B8 88 -> 05C 0 8 8 (latch UC.RATE)
    11A: 00 F9 1D -> 07C 1 1 D if /BLT.HFLIP JUMP 11D
    11B: 01 82 A6 -> 0C1 0 A 6
    11C: 00 E1 1E -> 070 1 1 E JUMP 11E
    11D: 01 93 A6 -> 0C9 1 A 6
    11E: 01 93 B7 -> 0C9 1 B 7
    11F: B0 B8 99 -> 05C 0 9 9 (latch UC.FORMAT)
    120: 84 B8 55 -> 05C 0 5 5
    121: 01 B8 4E -> 0DC 0 4 E
    122: 01 89 EE -> 0C4 1 E E
    123: 02 B8 EE -> 15C 0 E E
    124: 09 BE DD -> 0DF 0 D D
    125: 00 E6 7C -> 073 0 7 C if N JUMP 07C
    126: 01 82 8E -> 0C1 0 8 E
    127: 00 D2 4E -> 069 0 4 E
    128: 00 E5 33 -> 072 1 3 3 if Z JUMP 133
    129: 01 93 4E -> 0C9 1 4 E
    12A: 01 96 EE -> 0CB 0 E E
    12B: 00 93 F7 -> 049 1 F 7
    12C: 00 ED 32 -> 076 1 3 2 if C JUMP 132
    12D: B4 B8 DD -> 05C 0 D D (latch UC.TRANS)
    12E: B8 B8 77 -> 05C 0 7 7 (latch UC.XYPOS)
    12F: A0 B8 33 -> 05C 0 3 3 (latch UC.RASCAS)
    130: B8 B8 66 -> 05C 0 6 6 (latch UC.XYPOS)
    131: BC C8 00 -> 064 0 0 0 (latch UC.SCAN)
    132: 01 82 C7 -> 0C1 0 C 7
    133: 01 83 D5 -> 0C1 1 D 5
    134: 00 FD 36 -> 07E 1 3 6 if NC JUMP 136
    135: 01 89 99 -> 0C4 1 9 9
    136: 00 FB 36 -> 07D 1 3 6 if BLT.SCAN JUMP 136
    137: A0 C8 00 -> 064 0 0 0 (latch UC.RASCAS)
    138: 84 B8 55 -> 05C 0 5 5
    139: B0 B8 99 -> 05C 0 9 9 (latch UC.FORMAT)
    13A: 00 E1 24 -> 070 1 2 4 JUMP 124
    13B: 08 3E 00 -> 01F 0 0 0
    13C: 00 EF 3C -> 077 1 3 C if /MOTIMEP JUMP 13C
    13D: 18 3E 00 -> 01F 0 0 0
    13E: 00 E5 75 -> 072 1 7 5 if Z JUMP 175
    13F: 00 C8 00 -> 064 0 0 0
    140: 9C B8 44 -> 05C 0 4 4
    141: 98 B8 44 -> 05C 0 4 4
    142: 00 C8 00 -> 064 0 0 0
    143: 00 C8 00 -> 064 0 0 0
    144: 9C B8 11 -> 05C 0 1 1
    145: 98 B8 11 -> 05C 0 1 1
    146: 00 E1 5A -> 070 1 5 A JUMP 15A
    147: 00 38 11 -> 01C 0 1 1
    148: 03 34 00 -> 19A 0 0 0
    149: 03 34 00 -> 19A 0 0 0
    14A: 03 34 00 -> 19A 0 0 0
    14B: 00 11 99 -> 008 1 9 9
    14C: 00 EF 4C -> 077 1 4 C if /MOTIMEP JUMP 14C
    14D: 02 34 00 -> 11A 0 0 0
    14E: 84 B4 00 -> 05A 0 0 0
    14F: 01 C8 00 -> 0E4 0 0 0
    150: 9C B8 00 -> 05C 0 0 0
    151: 98 B8 00 -> 05C 0 0 0
    152: 00 C8 00 -> 064 0 0 0
    153: 00 C8 00 -> 064 0 0 0
    154: 9C B8 11 -> 05C 0 1 1
    155: 98 B8 11 -> 05C 0 1 1
    156: 00 C8 00 -> 064 0 0 0
    157: 00 C8 00 -> 064 0 0 0
    158: 9C B8 44 -> 05C 0 4 4
    159: 98 B8 44 -> 05C 0 4 4
    15A: 09 BE DD -> 0DF 0 D D
    15B: 01 82 8E -> 0C1 0 8 E
    15C: 00 D2 4E -> 069 0 4 E
    15D: 00 E5 67 -> 072 1 6 7 if Z JUMP 167
    15E: 01 93 4E -> 0C9 1 4 E
    15F: 01 96 EE -> 0CB 0 E E
    160: 00 93 F7 -> 049 1 F 7
    161: 00 ED 67 -> 076 1 6 7 if C JUMP 167
    162: B4 B8 DD -> 05C 0 D D (latch UC.TRANS)
    163: B8 B8 77 -> 05C 0 7 7 (latch UC.XYPOS)
    164: A0 B8 33 -> 05C 0 3 3 (latch UC.RASCAS)
    165: B8 B8 66 -> 05C 0 6 6 (latch UC.XYPOS)
    166: BC C8 00 -> 064 0 0 0 (latch UC.SCAN)
    167: 01 82 C7 -> 0C1 0 C 7
    168: 01 83 D5 -> 0C1 1 D 5
    169: 00 FD 6C -> 07E 1 6 C if NC JUMP 16C
    16A: 01 89 99 -> 0C4 1 9 9
    16B: 01 C8 00 -> 0E4 0 0 0
    16C: 01 96 00 -> 0CB 0 0 0
    16D: 00 F5 6C -> 07A 1 6 C if NZ JUMP 16C
    16E: A0 C8 00 -> 064 0 0 0 (latch UC.RASCAS)
    16F: 84 B8 55 -> 05C 0 5 5
    170: B0 B8 99 -> 05C 0 9 9 (latch UC.FORMAT)
    171: 01 C8 00 -> 0E4 0 0 0
    172: 01 96 00 -> 0CB 0 0 0
    173: 00 F5 72 -> 07A 1 7 2 if NZ JUMP 172
    174: 00 E1 5A -> 070 1 5 A JUMP 15A
    175: 84 B8 88 -> 05C 0 8 8
    176: 00 EF 76 -> 077 1 7 6 if /MOTIMEP JUMP 176
    177: 19 BE AA -> 0DF 0 A A
    178: B0 B8 AA -> 05C 0 A A (latch UC.FORMAT)
    179: 01 C8 77 -> 0E4 0 7 7
    17A: 84 B8 77 -> 05C 0 7 7
    17B: 01 C8 BB -> 0E4 0 B B
    17C: 00 C8 00 -> 064 0 0 0
    17D: 00 C8 00 -> 064 0 0 0
    17E: 00 C8 00 -> 064 0 0 0
    17F: 08 3E 00 -> 01F 0 0 0
    180: 01 80 BB -> 0C0 0 B B
    181: 01 89 77 -> 0C4 1 7 7
    182: 00 F5 7F -> 07A 1 7 F if NZ JUMP 17F
    183: 84 B8 AA -> 05C 0 A A
    184: 00 C8 00 -> 064 0 0 0
    185: 00 C8 00 -> 064 0 0 0
    186: 9C B8 BB -> 05C 0 B B
    187: 98 B8 BB -> 05C 0 B B
    188: 01 96 AA -> 0CB 0 A A
    189: 00 F7 8B -> 07B 1 8 B if P JUMP 18B
    18A: 00 E1 8A -> 070 1 8 A JUMP 18A
    18B: 00 C8 00 -> 064 0 0 0
    18C: 00 E1 78 -> 070 1 7 8 JUMP 178

***************************************************************************/

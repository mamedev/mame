/* kan_pand.c Kaneko Pandora Sprite Chip
  GFX processor - PX79C480FP-3 (KANEKO, Pandora-Chip)

  This emulates the Kaneko Pandora Sprite Chip
   which is found on several Kaneko boards.

   there several bootleg variants of this chip,
   these are emulated in kan_panb.c instead.

   Original Games using this Chip

   Snow Bros
   Air Buster
   DJ Boy
   Heavy Unit
   Sand Scorpian
   Gals Panic (1st release)

   The SemiCom games are also using this because
   their bootleg chip appears to function in an
   identical way.

   Rendering appears to be done to a framebuffer
   and the video system can be instructed not to
   clear this, allowing for 'sprite trail' effects
   as used by Air Buster.

   The chip appears to be an 8-bit chip, and
   when used on 16-bit CPUs only the MSB or LSB
   of the data lines are connected.  Address Lines
   also appear to be swapped around on one of the
   hookups.

   to use this in a driver you must hook functions to
   VIDEO_START  (allocates ram used by chip)
   VIDEO_UPDATE  (copies framebuffer to screen)
   and
   VIDEO_EOF  (renders the sprites to the framebuffer)

   spriteram should be accessed only with the
   pandora_spriteram_r / pandora_spriteram_w or
   pandora_spriteram_LSB_r / pandora_spriteram_LSB_w
   handlers, depending on the CPU being used with it.

*/

#include "driver.h"
#include "kan_pand.h"

static UINT8* pandora_spriteram;
static UINT8 pandora_region;
static bitmap_t *pandora_sprites_bitmap; /* bitmap to render sprites to, Pandora seems to be frame'buffered' */
static int pandora_clear_bitmap;
static int pandora_xoffset, pandora_yoffset;

void pandora_set_clear_bitmap(int clear)
{
	pandora_clear_bitmap = clear;
}

void pandora_update(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	if (!pandora_sprites_bitmap)
	{
		printf("ERROR: pandora_update with no pandora_sprites_bitmap\n");
		return;
	}

	copybitmap_trans(bitmap,pandora_sprites_bitmap,0,0,0,0,cliprect,0);
}


static void pandora_draw(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{

	int sx=0, sy=0, x=0, y=0, offs;


	/*
     * Sprite Tile Format
     * ------------------
     *
     * Byte | Bit(s)   | Use
     * -----+-76543210-+----------------
     *  0-2 | -------- | unused
     *  3   | xxxx.... | Palette Bank
     *  3   | .......x | XPos - Sign Bit
     *  3   | ......x. | YPos - Sign Bit
     *  3   | .....x.. | Use Relative offsets
     *  4   | xxxxxxxx | XPos
     *  5   | xxxxxxxx | YPos
     *  6   | xxxxxxxx | Sprite Number (low 8 bits)
     *  7   | ....xxxx | Sprite Number (high 4 bits)
     *  7   | x....... | Flip Sprite Y-Axis
     *  7   | .x...... | Flip Sprite X-Axis
     */

	for (offs = 0;offs < 0x1000;offs += 8)
	{
		int dx = pandora_spriteram[offs+4];
		int dy = pandora_spriteram[offs+5];
		int tilecolour = pandora_spriteram[offs+3];
		int attr = pandora_spriteram[offs+7];
		int flipx =   attr & 0x80;
		int flipy =  (attr & 0x40) << 1;
		int tile  = ((attr & 0x3f) << 8) + (pandora_spriteram[offs+6] & 0xff);

		if (tilecolour & 1) dx |= 0x100;
		if (tilecolour & 2) dy |= 0x100;

		if (tilecolour & 4)
		{
			x += dx;
			y += dy;
		}
		else
		{
			x = dx;
			y = dy;
		}

		if (flip_screen_get(machine))
		{
			sx = 240 - x;
			sy = 240 - y;
			flipx = !flipx;
			flipy = !flipy;
		}
		else
		{
			sx = x;
			sy = y;
		}

		/* global offset */
		sx+=pandora_xoffset;
		sy+=pandora_yoffset;

		sx &=0x1ff;
		sy &=0x1ff;

		if (sx&0x100) sx-=0x200;
		if (sy&0x100) sy-=0x200;

		drawgfx_transpen(bitmap,cliprect,machine->gfx[pandora_region],
				tile,
				(tilecolour & 0xf0) >> 4,
				flipx, flipy,
				sx,sy,0);
	}
}

void pandora_eof(running_machine *machine)
{
	assert(pandora_spriteram != NULL);

	// the games can disable the clearing of the sprite bitmap, to leave sprite trails
	if (pandora_clear_bitmap) bitmap_fill(pandora_sprites_bitmap,video_screen_get_visible_area(machine->primary_screen),0);

	pandora_draw(machine, pandora_sprites_bitmap, video_screen_get_visible_area(machine->primary_screen));
}

void pandora_start(running_machine *machine, UINT8 region, int x, int y)
{
	pandora_region = region;
	pandora_xoffset = x;
	pandora_yoffset = y;
	pandora_spriteram = auto_alloc_array(machine, UINT8, 0x1000);
	memset(pandora_spriteram,0x00, 0x1000);

	pandora_sprites_bitmap = video_screen_auto_bitmap_alloc(machine->primary_screen);
	pandora_clear_bitmap = 1;

    state_save_register_global(machine, pandora_clear_bitmap);
    state_save_register_global_pointer(machine, pandora_spriteram, 0x1000);
    state_save_register_global_bitmap(machine, pandora_sprites_bitmap);
}


WRITE8_HANDLER ( pandora_spriteram_w )
{
	// it's either hooked up oddly on this, or on the 16-bit games
	// either way, we swap the address lines so that the spriteram is in the same format
	offset = BITSWAP16(offset,  15,14,13,12, 11,   7,6,5,4,3,2,1,0,   10,9,8  );

	if (!pandora_spriteram)
	{
		printf("ERROR: pandora_spriteram_w with no pandora_spriteram\n");
		return;
	}

	if (offset>=0x1000)
	{
		logerror("pandora_spriteram_w write past spriteram, offset %04x %02x\n",offset,data);
		return;
	}
	pandora_spriteram[offset] = data;
}

READ8_HANDLER( pandora_spriteram_r )
{
	// it's either hooked up oddly on this, or ont the 16-bit games
	// either way, we swap the address lines so that the spriteram is in the same format
	offset = BITSWAP16(offset,  15,14,13,12, 11,  7,6,5,4,3,2,1,0,  10,9,8  );

	if (!pandora_spriteram)
	{
		printf("ERROR: pandora_spriteram_r with no pandora_spriteram\n");
		return 0x00;
	}

	if (offset>=0x1000)
	{
		logerror("pandora_spriteram_r read past spriteram, offset %04x\n",offset );
		return 0x00;
	}
	return pandora_spriteram[offset];
}

/* I don't know if this MSB/LSB mirroring is correct, or if there is twice as much ram, with half of it unused */
WRITE16_HANDLER( pandora_spriteram_LSB_w )
{
	if (!pandora_spriteram)
	{
		printf("ERROR: pandora_spriteram_LSB_w with no pandora_spriteram\n");
		return;
	}

	if (ACCESSING_BITS_8_15)
	{
		pandora_spriteram[offset] = (data>>8)&0xff;
	}

	if (ACCESSING_BITS_0_7)
	{
		pandora_spriteram[offset] = data&0xff;
	}
}

READ16_HANDLER( pandora_spriteram_LSB_r )
{
	if (!pandora_spriteram)
	{
		printf("ERROR: pandora_spriteram_LSB_r with no pandora_spriteram\n");
		return 0x0000;
	}

	return pandora_spriteram[offset]|(pandora_spriteram[offset]<<8);
}

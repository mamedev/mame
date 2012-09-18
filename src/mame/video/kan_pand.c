/***********************************************************************

    Kaneko Pandora Sprite Chip
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
    Sand Scorpion
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
    VIDEO_UPDATE  (copies framebuffer to screen)
    and
    VIDEO_EOF  (renders the sprites to the framebuffer)

    also, you have to add the correspondent device in MACHINE_DRIVER

    spriteram should be accessed only with the
    pandora_spriteram_r / pandora_spriteram_w or
    pandora_spriteram_LSB_r / pandora_spriteram_LSB_w
    handlers, depending on the CPU being used with it.

***********************************************************************/

#include "emu.h"
#include "video/kan_pand.h"

struct kaneko_pandora_state
{
	screen_device *screen;
	UINT8 *      spriteram;
	bitmap_ind16     *sprites_bitmap; /* bitmap to render sprites to, Pandora seems to be frame'buffered' */
	int          clear_bitmap;
	UINT8        region;
	int          xoffset, yoffset;
	int			 bg_pen; // might work some other way..
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE kaneko_pandora_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == KANEKO_PANDORA);

	return (kaneko_pandora_state *)downcast<kaneko_pandora_device *>(device)->token();
}

INLINE const kaneko_pandora_interface *get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == KANEKO_PANDORA));
	return (const kaneko_pandora_interface *) device->static_config();
}

/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

void pandora_set_bg_pen( device_t *device, int pen )
{
	kaneko_pandora_state *pandora = get_safe_token(device);
	pandora->bg_pen = pen;
}

void pandora_set_clear_bitmap( device_t *device, int clear )
{
	kaneko_pandora_state *pandora = get_safe_token(device);
	pandora->clear_bitmap = clear;
}

void pandora_update( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	kaneko_pandora_state *pandora = get_safe_token(device);

	if (!pandora->sprites_bitmap)
	{
		printf("ERROR: pandora_update with no pandora_sprites_bitmap\n");
		return;
	}

	copybitmap_trans(bitmap, *pandora->sprites_bitmap, 0, 0, 0, 0, cliprect, 0);
}


static void pandora_draw( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	kaneko_pandora_state *pandora = get_safe_token(device);
	int sx = 0, sy = 0, x = 0, y = 0, offs;


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

	for (offs = 0; offs < 0x1000; offs += 8)
	{
		int dx = pandora->spriteram[offs + 4];
		int dy = pandora->spriteram[offs + 5];
		int tilecolour = pandora->spriteram[offs + 3];
		int attr = pandora->spriteram[offs + 7];
		int flipx =   attr & 0x80;
		int flipy =  (attr & 0x40) << 1;
		int tile  = ((attr & 0x3f) << 8) + (pandora->spriteram[offs + 6] & 0xff);

		if (tilecolour & 1)
			dx |= 0x100;
		if (tilecolour & 2)
			dy |= 0x100;

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

		if (device->machine().driver_data()->flip_screen())
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
		sx += pandora->xoffset;
		sy += pandora->yoffset;

		sx &= 0x1ff;
		sy &= 0x1ff;

		if (sx & 0x100)
			sx -= 0x200;
		if (sy & 0x100)
			sy -= 0x200;

		drawgfx_transpen(bitmap,cliprect,device->machine().gfx[pandora->region],
				tile,
				(tilecolour & 0xf0) >> 4,
				flipx, flipy,
				sx,sy,0);
	}
}

void pandora_eof( device_t *device )
{
	kaneko_pandora_state *pandora = get_safe_token(device);
	assert(pandora->spriteram != NULL);

	// the games can disable the clearing of the sprite bitmap, to leave sprite trails
	if (pandora->clear_bitmap)
		pandora->sprites_bitmap->fill(pandora->bg_pen, pandora->screen->visible_area());

	pandora_draw(device, *pandora->sprites_bitmap, pandora->screen->visible_area());
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

WRITE8_DEVICE_HANDLER ( pandora_spriteram_w )
{
	kaneko_pandora_state *pandora = get_safe_token(device);

	// it's either hooked up oddly on this, or on the 16-bit games
	// either way, we swap the address lines so that the spriteram is in the same format
	offset = BITSWAP16(offset,  15,14,13,12, 11,   7,6,5,4,3,2,1,0,   10,9,8  );

	if (!pandora->spriteram)
	{
		printf("ERROR: pandora->spriteram_w with no pandora_spriteram\n");
		return;
	}

	if (offset >= 0x1000)
	{
		logerror("pandora->spriteram_w write past spriteram, offset %04x %02x\n", offset, data);
		return;
	}

	pandora->spriteram[offset] = data;
}

READ8_DEVICE_HANDLER( pandora_spriteram_r )
{
	kaneko_pandora_state *pandora = get_safe_token(device);

	// it's either hooked up oddly on this, or ont the 16-bit games
	// either way, we swap the address lines so that the spriteram is in the same format
	offset = BITSWAP16(offset,  15,14,13,12, 11,  7,6,5,4,3,2,1,0,  10,9,8  );

	if (!pandora->spriteram)
	{
		printf("ERROR: pandora->spriteram_r with no pandora_spriteram\n");
		return 0x00;
	}

	if (offset >= 0x1000)
	{
		logerror("pandora->spriteram_r read past spriteram, offset %04x\n", offset);
		return 0x00;
	}
	return pandora->spriteram[offset];
}

/* I don't know if this MSB/LSB mirroring is correct, or if there is twice as much ram, with half of it unused */
WRITE16_DEVICE_HANDLER( pandora_spriteram_LSB_w )
{
	kaneko_pandora_state *pandora = get_safe_token(device);

	if (!pandora->spriteram)
	{
		printf("ERROR: pandora->spriteram_LSB_w with no pandora_spriteram\n");
		return;
	}

	if (ACCESSING_BITS_8_15)
	{
		pandora->spriteram[offset] = (data >> 8) & 0xff;
	}

	if (ACCESSING_BITS_0_7)
	{
		pandora->spriteram[offset] = data & 0xff;
	}
}

READ16_DEVICE_HANDLER( pandora_spriteram_LSB_r )
{
	kaneko_pandora_state *pandora = get_safe_token(device);

	if (!pandora->spriteram)
	{
		printf("ERROR: pandora_spriteram_LSB_r with no pandora_spriteram\n");
		return 0x0000;
	}

	return pandora->spriteram[offset] | (pandora->spriteram[offset] << 8);
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( kaneko_pandora )
{
	kaneko_pandora_state *pandora = get_safe_token(device);
	const kaneko_pandora_interface *intf = get_interface(device);

	pandora->screen = device->machine().device<screen_device>(intf->screen);
	pandora->region = intf->gfx_region;
	pandora->xoffset = intf->x;
	pandora->yoffset = intf->y;
	pandora->bg_pen = 0;

	pandora->spriteram = auto_alloc_array(device->machine(), UINT8, 0x1000);

	pandora->sprites_bitmap = auto_bitmap_ind16_alloc(device->machine(), pandora->screen->width(), pandora->screen->height());

	device->save_item(NAME(pandora->clear_bitmap));
	device->save_pointer(NAME(pandora->spriteram), 0x1000);
	device->save_item(NAME(*pandora->sprites_bitmap));
}

static DEVICE_RESET( kaneko_pandora )
{
	kaneko_pandora_state *pandora = get_safe_token(device);

	memset(pandora->spriteram, 0x00, 0x1000);

	pandora->clear_bitmap = 1;
}

const device_type KANEKO_PANDORA = &device_creator<kaneko_pandora_device>;

kaneko_pandora_device::kaneko_pandora_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, KANEKO_PANDORA, "Kaneko Pandora - PX79C480FP-3", tag, owner, clock)
{
	m_token = global_alloc_clear(kaneko_pandora_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void kaneko_pandora_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kaneko_pandora_device::device_start()
{
	DEVICE_START_NAME( kaneko_pandora )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kaneko_pandora_device::device_reset()
{
	DEVICE_RESET_NAME( kaneko_pandora )(this);
}



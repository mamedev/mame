// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega 16-bit sprite hardware

***************************************************************************/

#include "emu.h"
#include "sega16sp.h"
#include "segaic16.h"



//****************************************************************************
//  CONSTANTS
//****************************************************************************

// device type definition
const device_type SEGA_HANGON_SPRITES = &device_creator<sega_hangon_sprite_device>;
const device_type SEGA_SHARRIER_SPRITES = &device_creator<sega_sharrier_sprite_device>;
const device_type SEGA_OUTRUN_SPRITES = &device_creator<sega_outrun_sprite_device>;
const device_type SEGA_SYS16A_SPRITES = &device_creator<sega_sys16a_sprite_device>;
const device_type BOOTLEG_SYS16A_SPRITES = &device_creator<bootleg_sys16a_sprite_device>;
const device_type SEGA_SYS16B_SPRITES = &device_creator<sega_sys16b_sprite_device>;
const device_type SEGA_XBOARD_SPRITES = &device_creator<sega_xboard_sprite_device>;
const device_type SEGA_YBOARD_SPRITES = &device_creator<sega_yboard_sprite_device>;



//****************************************************************************
//  DEVICE INTERFACE
//****************************************************************************

//-------------------------------------------------
//  sega_16bit_sprite_device -- core constructor
//-------------------------------------------------

sega_16bit_sprite_device::sega_16bit_sprite_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, const char *shortname, const char *source)
	: sprite16_device_ind16(mconfig, type, name, tag, owner, shortname, source),
		m_flip(false)
{
	// default to 1:1 bank mapping
	for (int bank = 0; bank < ARRAY_LENGTH(m_bank); bank++)
		m_bank[bank] = bank;
}


//-------------------------------------------------
//  device_start -- device startup
//-------------------------------------------------

void sega_16bit_sprite_device::device_start()
{
	// let the parent do its work
	sprite16_device_ind16::device_start();

	// save states
	save_item(NAME(m_flip));
	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  draw_write -- trigger a buffer flip
//-------------------------------------------------

WRITE16_MEMBER( sega_16bit_sprite_device::draw_write )
{
	UINT32 *src = reinterpret_cast<UINT32 *>(spriteram());
	UINT32 *dst = reinterpret_cast<UINT32 *>(buffer());

	// swap the halves of the sprite RAM
	for (int i = 0; i < spriteram_bytes()/4; i++)
	{
		UINT32 temp = *src;
		*src++ = *dst;
		*dst++ = temp;
	}

	// hack for thunderblade
	*spriteram() = 0xffff;

	// we will render the sprites when the video update happens
}



//****************************************************************************
//  HANG ON-STYLE SPRITES
//****************************************************************************

//-------------------------------------------------
//  sega_hangon_sprite_device -- constructor
//-------------------------------------------------

sega_hangon_sprite_device::sega_hangon_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sega_16bit_sprite_device(mconfig, SEGA_HANGON_SPRITES, "Sega Custom Sprites (Hang On)", tag, owner, "sega_hangon_sprite", __FILE__)
{
	set_local_origin(189, -1);
}


//-------------------------------------------------
//  draw -- render the sprites within the cliprect
//-------------------------------------------------

void sega_hangon_sprite_device::draw(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//
	//  Hang On-style sprites
	//
	//      Offs  Bits               Usage
	//       +0   bbbbbbbb --------  Bottom scanline of sprite - 1
	//       +0   -------- tttttttt  Top scanline of sprite - 1
	//       +2   bbbb---- --------  Sprite bank
	//       +2   -------x xxxxxxxx  X position of sprite (position $BD is screen position 0)
	//       +4   pppppppp pppppppp  Signed 16-bit pitch value between scanlines
	//       +6   -ooooooo oooooooo  Offset within selected sprite bank
	//       +6   f------- --------  Horizontal flip: read the data backwards if set
	//       +8   --cccccc --------  Sprite color palette
	//       +8   -------- zzzzzz--  Zoom factor
	//       +8   -------- ------pp  Sprite priority
	//       +E   dddddddd dddddddd  Scratch space for current address
	//
	//  Final bitmap format:
	//
	//            ----pp-- --------  Sprite priority
	//            ------cc cccc----  Sprite color palette
	//            -------- ----llll  4-bit pixel data
	//
	//  Special notes:
	//
	//      There is an interaction between the horizonal flip bit and the offset.
	//      The offset is maintained as a 16-bit value, even though only the lower
	//      15 bits are used for the address. The top bit is used to control flipping.
	//      This means that if the low 15 bits overflow during rendering, the sprite
	//      data will be read backwards after the overflow. This is important to
	//      emulate correctly as many games make use of this feature to render sprites
	//      at the beginning of a bank.
	//

	// render the sprites in order
	const UINT16 *spritebase = reinterpret_cast<const UINT16 *>(region()->base());
	UINT8 numbanks = region()->bytes() / 0x10000;
	const UINT8 *zoom = memregion("zoom")->base();
	UINT16 *ramend = spriteram() + spriteram_elements();
	for (UINT16 *data = spriteram(); data < ramend; data += 8)
	{
		// fetch the bottom; stop when we get something out of range
		int bottom  = data[0] >> 8;
		if (bottom > 0xf0)
			break;

		// extract remaining parameters
		int top     = data[0] & 0xff;
		int bank    = m_bank[(data[1] >> 12) & 0xf];
		int xpos    = data[1] & 0x1ff;
		int pitch   = INT16(data[2]);
		UINT16 addr = data[3];
		int colpri  = (((data[4] >> 8) & 0x3f) << 4) | (((data[4] >> 0) & 0x3) << 10);
		int vzoom   = (data[4] >> 2) & 0x3f;
		int hzoom   = vzoom << 1;

		// initialize the end address to the start address
		data[7] = addr;

		// if top greater than/equal to bottom, or invalid bank, punt
		if (top >= bottom || bank == 255)
			continue;

		// clamp to within the memory region size
		if (numbanks)
			bank %= numbanks;
		const UINT16 *spritedata = spritebase + 0x8000 * bank;

		// determine the starting zoom address and mask
		int zaddr = (vzoom & 0x38) << 5;
		int zmask = 1 << (vzoom & 7);

		// loop from top to bottom
		int minx = xpos;
		int maxx = cliprect.min_x - 1;
		int miny = cliprect.max_y + 1;
		int maxy = cliprect.min_y - 1;
		for (int y = top; y < bottom; y++)
		{
			// advance a row
			addr += pitch;

			// if the zoom bit says so, add pitch a second time
			if (zoom[zaddr++] & zmask)
				addr += pitch;

			// skip drawing if not within the cliprect
			if (y >= cliprect.min_y && y <= cliprect.max_y)
			{
				UINT16 *dest = &bitmap.pix(y);
				int xacc = 0x00;
				int x;

				// note that the System 16A sprites have a design flaw that allows the address
				// to carry into the flip flag, which is the topmost bit -- it is very important
				// to emulate this as the games compensate for it

				// non-flipped case
				if (!(addr & 0x8000))
				{
					// start at the word before because we preincrement below
					data[7] = addr - 1;
					for (x = xpos; x <= cliprect.max_x; )
					{
						UINT16 pixels = spritedata[++data[7] & 0x7fff];

						// draw four pixels
						int pix;
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }

						// stop if the last pixel in the group was 0xf
						if (pix == 15)
							break;
					}
				}

				// flipped case
				else
				{
					// start at the word after because we predecrement below
					data[7] = addr + 1;
					for (x = xpos; x <= cliprect.max_x; )
					{
						UINT16 pixels = spritedata[--data[7] & 0x7fff];

						// draw four pixels
						int pix;
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }

						// stop if the last pixel in the group was 0xf
						if (pix == 15)
							break;
					}
				}

				// update bounds
				if (x > maxx) maxx = x;
				if (y < miny) miny = y;
				maxy = y;
			}
		}

		// mark dirty
		if (minx <= maxx && miny <= maxy)
			mark_dirty(minx, maxx, miny, maxy);
	}
}



//****************************************************************************
//  SPACE HARRIER-STYLE SPRITES
//****************************************************************************

//-------------------------------------------------
//  sega_sharrier_sprite_device -- constructor
//-------------------------------------------------

sega_sharrier_sprite_device::sega_sharrier_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sega_16bit_sprite_device(mconfig, SEGA_SHARRIER_SPRITES, "Sega Custom Sprites (Space Harrier)", tag, owner, "sega_sharrier_sprite", __FILE__)
{
	set_local_origin(189, -1);
}


//-------------------------------------------------
//  draw -- render the sprites within the cliprect
//-------------------------------------------------

void sega_sharrier_sprite_device::draw(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//
	//  Space Harrier-style sprites
	//
	//      Offs  Bits               Usage
	//       +0   bbbbbbbb --------  Bottom scanline of sprite - 1
	//       +0   -------- tttttttt  Top scanline of sprite - 1
	//       +2   bbbb---- --------  Sprite bank
	//       +2   -------x xxxxxxxx  X position of sprite (position $BD is screen position 0)
	//       +4   s------- --------  Sprite shadow disable (0=enable, 1=disable)
	//       +4   -p------ --------  Sprite priority
	//       +4   --cccccc --------  Sprite color palette
	//       +4   -------- -ppppppp  Signed 7-bit pitch value between scanlines
	//       +6   f------- --------  Horizontal flip: read the data backwards if set
	//       +6   -ooooooo oooooooo  Offset within selected sprite bank
	//       +8   --zzzzzz --------  Horizontal zoom factor
	//       +8   -------- --zzzzzz  Vertical zoom factor
	//       +E   dddddddd dddddddd  Scratch space for current address
	//
	//  Final bitmap format:
	//
	//            ----s--- --------  Sprite shadow disable
	//            -----p-- --------  Sprite priority
	//            ------cc cccc----  Sprite color palette
	//            -------- ----llll  4-bit pixel data
	//
	//  Special notes:
	//
	//      There is an interaction between the horizonal flip bit and the offset.
	//      The offset is maintained as a 16-bit value, even though only the lower
	//      15 bits are used for the address. The top bit is used to control flipping.
	//      This means that if the low 15 bits overflow during rendering, the sprite
	//      data will be read backwards after the overflow. This is important to
	//      emulate correctly as many games make use of this feature to render sprites
	//      at the beginning of a bank.
	//

	// render the sprites in order
	const UINT32 *spritebase = reinterpret_cast<const UINT32 *>(region()->base());
	UINT8 numbanks = region()->bytes() / 0x20000;
	const UINT8 *zoom = memregion("zoom")->base();
	UINT16 *ramend = spriteram() + spriteram_elements();
	for (UINT16 *data = spriteram(); data < ramend; data += 8)
	{
		// fetch the bottom; stop when we get something out of range
		int bottom  = data[0] >> 8;
		if (bottom > 0xf0)
			break;

		// extract remaining parameters
		int top     = data[0] & 0xff;
		int bank    = m_bank[(data[1] >> 12) & 0x7];
		int xpos    = data[1] & 0x1ff;
		int colpri  = ((data[2] >> 8) & 0xff) << 4;
		int pitch   = INT16(data[2] << 9) >> 9;
		UINT16 addr = data[3];
		int hzoom   = ((data[4] >> 8) & 0x3f) << 1;
		int vzoom   = (data[4] >> 0) & 0x3f;

		// initialize the end address to the start address
		data[7] = addr;

		// if top greater than/equal to bottom, or invalid bank, punt
		if (top >= bottom || bank == 255)
			continue;

		// clamp to within the memory region size
		if (numbanks)
			bank %= numbanks;
		const UINT32 *spritedata = spritebase + 0x8000 * bank;

		// determine the starting zoom address and mask
		int zaddr = (vzoom & 0x38) << 5;
		int zmask = 1 << (vzoom & 7);

		// loop from top to bottom
		int minx = xpos;
		int maxx = cliprect.min_x - 1;
		int miny = cliprect.max_y + 1;
		int maxy = cliprect.min_y - 1;
		for (int y = top; y < bottom; y++)
		{
			// advance a row
			addr += pitch;

			// if the zoom bit says so, add pitch a second time
			if (zoom[zaddr++] & zmask)
				addr += pitch;

			// skip drawing if not within the cliprect
			if (y >= cliprect.min_y && y <= cliprect.max_y)
			{
				UINT16 *dest = &bitmap.pix(y);
				int xacc = 0x00;
				int x;

				// note that the System 16A sprites have a design flaw that allows the address
				// to carry into the flip flag, which is the topmost bit -- it is very important
				// to emulate this as the games compensate for it

				// non-flipped case
				if (!(addr & 0x8000))
				{
					// start at the word before because we preincrement below
					data[7] = addr - 1;
					for (x = xpos; x <= cliprect.max_x; )
					{
						UINT32 pixels = spritedata[++data[7] & 0x7fff];

						// draw 8 pixels
						int pix;
						pix = (pixels >> 28) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >> 24) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >> 20) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >> 16) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }

						// stop if the last pixel in the group was 0xf
						if (pix == 15)
							break;
					}
				}

				// flipped case
				else
				{
					// start at the word after because we predecrement below
					data[7] = addr + 1;
					for (x = xpos; x <= cliprect.max_x; )
					{
						UINT32 pixels = spritedata[--data[7] & 0x7fff];

						// draw 8 pixels
						int pix;
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >> 16) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >> 20) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >> 24) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }
						pix = (pixels >> 28) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x++; }

						// stop if the last pixel in the group was 0xf
						if (pix == 15)
							break;
					}
				}

				// update bounds
				if (x > maxx) maxx = x;
				if (y < miny) miny = y;
				maxy = y;
			}
		}

		// mark dirty
		if (minx <= maxx && miny <= maxy)
			mark_dirty(minx, maxx, miny, maxy);
	}
}



//****************************************************************************
//  SYSTEM 16A-STYLE SPRITES
//****************************************************************************

//-------------------------------------------------
//  sega_sys16a_sprite_device -- constructor
//-------------------------------------------------

sega_sys16a_sprite_device::sega_sys16a_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sega_16bit_sprite_device(mconfig, SEGA_SYS16A_SPRITES, "Sega System 16A Sprites", tag, owner, "sega_sys16a_sprite", __FILE__)
{
	set_local_origin(189, -1, -189, -1);
}


//-------------------------------------------------
//  draw -- render the sprites within the cliprect
//-------------------------------------------------

void sega_sys16a_sprite_device::draw(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//
	//  System 16A-style sprites
	//
	//      Offs  Bits               Usage
	//       +0   bbbbbbbb --------  Bottom scanline of sprite - 1
	//       +0   -------- tttttttt  Top scanline of sprite - 1
	//       +2   -------x xxxxxxxx  X position of sprite (position $BD is screen position 0)
	//       +4   pppppppp pppppppp  Signed 16-bit pitch value between scanlines
	//       +6   -ooooooo oooooooo  Offset within selected sprite bank
	//       +6   f------- --------  Horizontal flip: read the data backwards if set
	//       +8   --cccccc --------  Sprite color palette
	//       +8   -------- -bbb----  Sprite bank
	//       +8   -------- ------pp  Sprite priority
	//       +E   dddddddd dddddddd  Scratch space for current address
	//
	//  Final bitmap format:
	//
	//            ----pp-- --------  Sprite priority
	//            ------cc cccc----  Sprite color palette
	//            -------- ----llll  4-bit pixel data
	//
	//  Special notes:
	//
	//      There is an interaction between the horizonal flip bit and the offset.
	//      The offset is maintained as a 16-bit value, even though only the lower
	//      15 bits are used for the address. The top bit is used to control flipping.
	//      This means that if the low 15 bits overflow during rendering, the sprite
	//      data will be read backwards after the overflow. This is important to
	//      emulate correctly as many games make use of this feature to render sprites
	//      at the beginning of a bank.
	//

	// render the sprites in order
	const UINT16 *spritebase = reinterpret_cast<const UINT16 *>(region()->base());
	UINT8 numbanks = region()->bytes() / 0x10000;
	UINT16 *ramend = spriteram() + spriteram_elements();
	for (UINT16 *data = spriteram(); data < ramend; data += 8)
	{
		// fetch the bottom; stop when we get something out of range
		int bottom  = data[0] >> 8;
		if (bottom > 0xf0)
			break;

		// extract remaining parameters
		int top     = data[0] & 0xff;
		int xpos    = data[1] & 0x1ff;
		int pitch   = INT16(data[2]);
		UINT16 addr = data[3];
		int colpri  = (((data[4] >> 8) & 0x3f) << 4) | (((data[4] >> 0) & 0x3) << 10);
		int bank    = m_bank[(data[4] >> 4) & 0x7];

		// initialize the end address to the start address
		data[7] = addr;

		// if top greater than/equal to bottom, or invalid bank, punt
		if (top >= bottom || bank == 255)
			continue;

		// clamp to within the memory region size
		if (numbanks)
			bank %= numbanks;
		const UINT16 *spritedata = spritebase + 0x8000 * bank;

		// adjust positions for screen flipping
		int xdelta = 1;
		if (m_flip)
		{
			int temp = top;
			top = 224 - bottom;
			bottom = 224 - temp;
			xpos = 320 - xpos;
			xdelta = -1;
			set_origin(m_xoffs_flipped, m_yoffs_flipped);
		}
		else
		{
			set_origin(m_xoffs, m_yoffs);
		}

		// loop from top to bottom
		int minx = xpos;
		int maxx = xpos;
		int miny = cliprect.max_y + 1;
		int maxy = cliprect.min_y - 1;
		for (int y = top; y < bottom; y++)
		{
			// advance a row
			addr += pitch;

			// skip drawing if not within the cliprect
			if (y >= cliprect.min_y && y <= cliprect.max_y)
			{
				UINT16 *dest = &bitmap.pix(y);
				int x;

				// note that the System 16A sprites have a design flaw that allows the address
				// to carry into the flip flag, which is the topmost bit -- it is very important
				// to emulate this as the games compensate for it

				// non-flipped case
				if (!(addr & 0x8000))
				{
					// start at the word before because we preincrement below
					data[7] = addr - 1;
					for (x = xpos; ((xpos - x) & 0x1ff) != 1; )
					{
						UINT16 pixels = spritedata[++data[7] & 0x7fff];

						// draw four pixels
						int pix;
						pix = (pixels >> 12) & 0xf; if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta;
						pix = (pixels >>  8) & 0xf; if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta;
						pix = (pixels >>  4) & 0xf; if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta;
						pix = (pixels >>  0) & 0xf; if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta;

						// stop if the last pixel in the group was 0xf
						if (pix == 15)
							break;
					}
				}

				// flipped case
				else
				{
					// start at the word after because we predecrement below
					data[7] = addr + 1;
					for (x = xpos; ((xpos - x) & 0x1ff) != 1; )
					{
						UINT16 pixels = spritedata[--data[7] & 0x7fff];

						// draw four pixels
						int pix;
						pix = (pixels >>  0) & 0xf; if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta;
						pix = (pixels >>  4) & 0xf; if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta;
						pix = (pixels >>  8) & 0xf; if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta;
						pix = (pixels >> 12) & 0xf; if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta;

						// stop if the last pixel in the group was 0xf
						if (pix == 15)
							break;
					}
				}

				// update bounds
				if (x > maxx) maxx = x;
				if (x < minx) minx = x;
				if (y < miny) miny = y;
				maxy = y;
			}
		}

		// mark dirty
		if (minx <= maxx && miny <= maxy)
			mark_dirty(minx, maxx, miny, maxy);
	}
}


//****************************************************************************
//  BOOTLEG SYSTEM 16A-STYLE SPRITES
//****************************************************************************

//-------------------------------------------------
//  bootleg_sys16a_sprite_device -- constructor
//-------------------------------------------------

bootleg_sys16a_sprite_device::bootleg_sys16a_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sega_16bit_sprite_device(mconfig, BOOTLEG_SYS16A_SPRITES, "Sega System 16A Sprites (Bootleg)", tag, owner, "bootleg_sys16a_sprite", __FILE__)
{
	m_addrmap[0] = 0;
	m_addrmap[1] = 1;
	m_addrmap[2] = 2;
	m_addrmap[3] = 3;
	m_addrmap[4] = 4;
	m_addrmap[5] = 5;
	m_addrmap[6] = 6;
	m_addrmap[7] = 7;
	set_local_origin(189, -1);
}


//-------------------------------------------------
//  static_set_remap -- configure sprite address
//  remapping
//-------------------------------------------------

void bootleg_sys16a_sprite_device::static_set_remap(device_t &device, UINT8 offs0, UINT8 offs1, UINT8 offs2, UINT8 offs3, UINT8 offs4, UINT8 offs5, UINT8 offs6, UINT8 offs7)
{
	bootleg_sys16a_sprite_device &target = downcast<bootleg_sys16a_sprite_device &>(device);
	target.m_addrmap[0] = offs0;
	target.m_addrmap[1] = offs1;
	target.m_addrmap[2] = offs2;
	target.m_addrmap[3] = offs3;
	target.m_addrmap[4] = offs4;
	target.m_addrmap[5] = offs5;
	target.m_addrmap[6] = offs6;
	target.m_addrmap[7] = offs7;
}


//-------------------------------------------------
//  draw -- render the sprites within the cliprect
//-------------------------------------------------

void bootleg_sys16a_sprite_device::draw(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//
	//  Bootleg System 16A-style sprites
	//
	//  These are identical to regular System 16A sprites (see above), with two exceptions:
	//
	//      1. Addresses within each sprite entry are generally shuffled relative
	//          to the original, and
	//
	//      2. The pitch increment happens at the end, not at the beginning of
	//          the loop.
	//

	// render the sprites in order
	const UINT16 *spritebase = reinterpret_cast<const UINT16 *>(region()->base());
	UINT8 numbanks = region()->bytes() / 0x10000;
	UINT16 *ramend = spriteram() + spriteram_elements();
	for (UINT16 *data = spriteram(); data < ramend; data += 8)
	{
		// fetch the bottom; stop when we get something out of range
		int bottom  = data[m_addrmap[0]] >> 8;
		if (bottom > 0xf0)
			break;

		// extract remaining parameters
		int top     = data[m_addrmap[0]] & 0xff;
		int xpos    = data[m_addrmap[1]] & 0x1ff;
		int pitch   = INT16(data[m_addrmap[2]]);
		UINT16 addr = data[m_addrmap[3]];
		int colpri  = (((data[m_addrmap[4]] >> 8) & 0x3f) << 4) | (((data[m_addrmap[4]] >> 0) & 0x3) << 10);
		int bank    = m_bank[(data[m_addrmap[4]] >> 4) & 0x7];

		// initialize the end address to the start address
		UINT16 &data7 = data[m_addrmap[7]];
		data7 = addr;

		// if top greater than/equal to bottom, or invalid bank, punt
		if (top >= bottom || bank == 255)
			continue;

		// clamp to within the memory region size
		if (numbanks)
			bank %= numbanks;
		const UINT16 *spritedata = spritebase + 0x8000 * bank;

		// adjust positions for screen flipping
		int xdelta = 1;
		if (m_flip)
		{
			int temp = top;
			top = 224 - bottom;
			bottom = 224 - temp;
			xpos = 320 - xpos;
			xdelta = -1;
			set_origin(m_xoffs_flipped, m_yoffs_flipped);
		}
		else
		{
			set_origin(m_xoffs, m_yoffs);
		}

		// loop from top to bottom
		int minx = xpos;
		int maxx = xpos;
		int miny = cliprect.max_y + 1;
		int maxy = cliprect.min_y - 1;
		for (int y = top; y < bottom; y++)
		{
			// skip drawing if not within the cliprect
			if (y >= cliprect.min_y && y <= cliprect.max_y)
			{
				UINT16 *dest = &bitmap.pix(y);
				int x;

				// note that the System 16A sprites have a design flaw that allows the address
				// to carry into the flip flag, which is the topmost bit -- it is very important
				// to emulate this as the games compensate for it

				// non-flipped case
				if (!(addr & 0x8000))
				{
					// start at the word before because we preincrement below
					data7 = addr - 1;
					for (x = xpos; ((xpos - x) & 0x1ff) != 1; )
					{
						UINT16 pixels = spritedata[++data7 & 0x7fff];

						// draw four pixels
						int pix;
						pix = (pixels >> 12) & 0xf; if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta;
						pix = (pixels >>  8) & 0xf; if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta;
						pix = (pixels >>  4) & 0xf; if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta;
						pix = (pixels >>  0) & 0xf; if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta;

						// stop if the last pixel in the group was 0xf
						if (pix == 15)
							break;
					}
				}

				// flipped case
				else
				{
					// start at the word after because we predecrement below
					data7 = addr + 1;
					for (x = xpos; ((xpos - x) & 0x1ff) != 1; )
					{
						UINT16 pixels = spritedata[--data7 & 0x7fff];

						// draw four pixels
						int pix;
						pix = (pixels >>  0) & 0xf; if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta;
						pix = (pixels >>  4) & 0xf; if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta;
						pix = (pixels >>  8) & 0xf; if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta;
						pix = (pixels >> 12) & 0xf; if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta;

						// stop if the last pixel in the group was 0xf
						if (pix == 15)
							break;
					}
				}

				// update bounds
				if (x > maxx) maxx = x;
				if (x < minx) minx = x;
				if (y < miny) miny = y;
				maxy = y;
			}

			// advance a row - must be done at the end on the bootlegs!
			addr += pitch;
		}

		// mark dirty
		if (minx <= maxx && miny <= maxy)
			mark_dirty(minx, maxx, miny, maxy);
	}
}



//****************************************************************************
//  SYSTEM 16B-STYLE SPRITES
//****************************************************************************

//-------------------------------------------------
//  sega_sys16b_sprite_device -- constructor
//-------------------------------------------------

sega_sys16b_sprite_device::sega_sys16b_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sega_16bit_sprite_device(mconfig, SEGA_SYS16B_SPRITES, "Sega System 16B Sprites", tag, owner, "sega_16bit_sprite", __FILE__)
{
	set_local_origin(184, 0x00, -184, 0);
}


//-------------------------------------------------
//  draw -- render the sprites within the cliprect
//-------------------------------------------------

void sega_sys16b_sprite_device::draw(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//
	//  System 16B-style sprites
	//
	//      Offs  Bits               Usage
	//       +0   bbbbbbbb --------  Bottom scanline of sprite - 1
	//       +0   -------- tttttttt  Top scanline of sprite - 1
	//       +2   -------x xxxxxxxx  X position of sprite (position $BD is screen position 0)
	//       +2   ---iiii- --------  Sprite/sprite priority for Y-board
	//       +4   e------- --------  Signify end of sprite list
	//       +4   -h------ --------  Hide this sprite
	//       +4   -------f --------  Horizontal flip: read the data backwards if set
	//       +4   -------- pppppppp  Signed 8-bit pitch value between scanlines
	//       +6   oooooooo oooooooo  Offset within selected sprite bank
	//       +8   ----bbbb --------  Sprite bank
	//       +8   -------- pp------  Sprite priority, relative to tilemaps
	//       +8   -------- --cccccc  Sprite color palette
	//       +A   ------vv vvv-----  Vertical zoom factor (0 = full size, 0x10 = half size)
	//       +A   -------- ---hhhhh  Horizontal zoom factor (0 = full size, 0x10 = half size)
	//       +E   dddddddd dddddddd  Scratch space for current address
	//
	//  Final bitmap format:
	//
	//            iiii---- --------  Sprite/sprite priority for Y-board
	//            ----pp-- --------  Sprite priority
	//            ------cc cccc----  Sprite color palette
	//            -------- ----llll  4-bit pixel data
	//
	//  Note that the zooming described below is 100% accurate to the real board.
	//

	// render the sprites in order
	const UINT16 *spritebase = reinterpret_cast<const UINT16 *>(region()->base());
	UINT8 numbanks = region()->bytes() / 0x20000;
	UINT16 *ramend = spriteram() + spriteram_elements();
	for (UINT16 *data = spriteram(); data < ramend; data += 8)
	{
		// stop when we hit the end of sprite list
		if (data[2] & 0x8000)
			break;

		// extract parameters
		int bottom  = data[0] >> 8;
		int top     = data[0] & 0xff;
		int xpos    = data[1] & 0x1ff;
		int hide    = data[2] & 0x4000;
		int flip    = data[2] & 0x100;
		int pitch   = INT8(data[2] & 0xff);
		UINT16 addr = data[3];
		int bank    = m_bank[(data[4] >> 8) & 0xf];
		int colpri  = ((data[4] & 0xff) << 4) | (((data[1] >> 9) & 0xf) << 12);
		int vzoom   = (data[5] >> 5) & 0x1f;
		int hzoom   = data[5] & 0x1f;
		const UINT16 *spritedata;

		xpos &= 0x1ff;

		// initialize the end address to the start address
		data[7] = addr;

		// if hidden, or top greater than/equal to bottom, or invalid bank, punt
		if (hide || top >= bottom || bank == 255)
			continue;

		// clamp to within the memory region size
		if (numbanks)
			bank %= numbanks;
		spritedata = spritebase + 0x10000 * bank;

		// reset the yzoom counter
		data[5] &= 0x03ff;

		// adjust positions for screen flipping
		int xdelta = 1;
		if (m_flip)
		{
			int temp = top;
			top = 224 - bottom;
			bottom = 224 - temp;
			xpos = 320 - xpos;
			xdelta = -1;
			set_origin(m_xoffs_flipped, m_yoffs_flipped);
		}
		else
		{
			set_origin(m_xoffs, m_yoffs);
		}

		// loop from top to bottom
		int minx = xpos;
		int maxx = xpos;
		int miny = cliprect.max_y + 1;
		int maxy = cliprect.min_y - 1;
		for (int y = top; y < bottom; y++)
		{
			// advance a row
			addr += pitch;

			// accumulate zoom factors; if we carry into the high bit, skip an extra row
			data[5] += vzoom << 10;
			if (data[5] & 0x8000)
			{
				addr += pitch;
				data[5] &= ~0x8000;
			}

			// skip drawing if not within the cliprect
			if (y >= cliprect.min_y && y <= cliprect.max_y)
			{
				UINT16 *dest = &bitmap.pix(y);
				int x;

				// compute the initial X zoom accumulator; this is verified on the real PCB
				int xacc = 4 * hzoom;

				// non-flipped case
				if (!flip)
				{
					// start at the word before because we preincrement below
					data[7] = addr - 1;
					for (x = xpos; ((xpos - x) & 0x1ff) != 1; )
					{
						UINT16 pixels = spritedata[++data[7]];

						// draw four pixels
						int pix;
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; }
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; }

						// stop if the last pixel in the group was 0xf
						if (pix == 15)
							break;
					}
				}

				// flipped case
				else
				{
					// start at the word after because we predecrement below
					data[7] = addr + 1;
					for (x = xpos; ((xpos - x) & 0x1ff) != 1; )
					{
						UINT16 pixels = spritedata[--data[7]];

						// draw four pixels
						int pix;
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; }
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; }

						// stop if the last pixel in the group was 0xf
						if (pix == 15)
							break;
					}
				}

				// update bounds
				if (x > maxx) maxx = x;
				if (x < minx) minx = x;
				if (y < miny) miny = y;
				maxy = y;
			}
		}

		// mark dirty
		if (minx <= maxx && miny <= maxy)
			mark_dirty(minx, maxx, miny, maxy);
	}
}



//****************************************************************************
//  OUT RUN/X-BOARD-STYLE SPRITES
//****************************************************************************

//-------------------------------------------------
//  sega_outrun_sprite_device -- constructor
//-------------------------------------------------

sega_outrun_sprite_device::sega_outrun_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sega_16bit_sprite_device(mconfig, SEGA_OUTRUN_SPRITES, "Sega Custom Sprites (Out Run)", tag, owner, "sega_outrun_sprite", __FILE__),
		m_is_xboard(false)
{
	set_local_origin(189, 0x00);
}

sega_outrun_sprite_device::sega_outrun_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock, bool xboard_variant, const char *shortname, const char *source)
	: sega_16bit_sprite_device(mconfig, SEGA_XBOARD_SPRITES, "Sega X-Board Sprites", tag, owner, shortname, source),
		m_is_xboard(true)
{
	set_local_origin(190, 0x00);
}


//-------------------------------------------------
//  sega_xboard_sprite_device -- constructor
//-------------------------------------------------

sega_xboard_sprite_device::sega_xboard_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sega_outrun_sprite_device(mconfig, tag, owner, clock, true, "sega_xboard_sprite", __FILE__)
{
}


//-------------------------------------------------
//  draw -- render the sprites within the cliprect
//-------------------------------------------------

void sega_outrun_sprite_device::draw(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//
	//  Out Run/X-Board-style sprites
	//
	//      Offs  Bits               Usage
	//       +0   e------- --------  Signify end of sprite list
	//       +0   -h-h---- --------  Hide this sprite if either bit is set
	//       +0   ----bbb- --------  Sprite bank
	//       +0   -------t tttttttt  Top scanline of sprite + 256
	//       +2   oooooooo oooooooo  Offset within selected sprite bank
	//       +4   ppppppp- --------  Signed 7-bit pitch value between scanlines
	//       +4   -------x xxxxxxxx  X position of sprite (position $BE is screen position 0)
	//       +6   -s------ --------  Enable shadows
	//       +6   --pp---- --------  Sprite priority, relative to tilemaps
	//       +6   ------vv vvvvvvvv  Vertical zoom factor (0x200 = full size, 0x100 = half size, 0x300 = 2x size)
	//       +8   y------- --------  Render from top-to-bottom (1) or bottom-to-top (0) on screen
	//       +8   -f------ --------  Horizontal flip: read the data backwards if set
	//       +8   --x----- --------  Render from left-to-right (1) or right-to-left (0) on screen
	//       +8   ------hh hhhhhhhh  Horizontal zoom factor (0x200 = full size, 0x100 = half size, 0x300 = 2x size)
	//       +E   dddddddd dddddddd  Scratch space for current address
	//
	//  Out Run only:
	//       +A   hhhhhhhh --------  Height in scanlines - 1
	//       +A   -------- -ccccccc  Sprite color palette
	//
	//  X-Board only:
	//       +A   ----hhhh hhhhhhhh  Height in scanlines - 1
	//       +C   -------- cccccccc  Sprite color palette
	//
	//  Final bitmap format:
	//
	//            -s------ --------  Shadow control
	//            --pp---- --------  Sprite priority
	//            ----cccc cccc----  Sprite color palette
	//            -------- ----llll  4-bit pixel data
	//

	set_origin(m_xoffs, m_yoffs);

	// render the sprites in order
	const UINT32 *spritebase = reinterpret_cast<const UINT32 *>(region()->base());
	UINT8 numbanks = region()->bytes() / 0x40000;
	UINT16 *ramend = buffer() + spriteram_elements();
	for (UINT16 *data = buffer(); data < ramend; data += 8)
	{
		// stop when we hit the end of sprite list
		if (data[0] & 0x8000)
			break;

		// extract parameters
		int hide    = (data[0] & 0x5000);
		int bank    = (data[0] >> 9) & 7;
		int top     = (data[0] & 0x1ff) - 0x100;
		UINT16 addr = data[1];
		int pitch   = INT16((data[2] >> 1) | ((data[4] & 0x1000) << 3)) >> 8;
		int xpos    = data[2] & 0x1ff;
		int vzoom   = data[3] & 0x7ff;
		int ydelta  = (data[4] & 0x8000) ? 1 : -1;
		int flip    = (~data[4] >> 14) & 1;
		int xdelta  = (data[4] & 0x2000) ? 1 : -1;
		int hzoom   = data[4] & 0x7ff;
		int height  = (m_is_xboard ? (data[5] & 0xfff) : (data[5] >> 8)) + 1;
		int colpri  = ((m_is_xboard ? (data[6] & 0xff) : (data[5] & 0x7f)) << 4) | (((data[3] >> 12) & 7) << 12);

		// adjust X coordinate
		// note: the threshhold below is a guess. If it is too high, rachero will draw garbage
		// If it is too low, smgp won't draw the bottom part of the road
		if (xpos < 0x80 && xdelta < 0)
			xpos += 0x200;

		// initialize the end address to the start address
		data[7] = addr;

		// if hidden, punt
		if (hide)
			continue;

		// clamp to within the memory region size
		if (numbanks)
			bank %= numbanks;
		const UINT32 *spritedata = spritebase + 0x10000 * bank;

		// clamp to a maximum of 8x (not 100% confirmed)
		if (vzoom < 0x40) vzoom = 0x40;
		if (hzoom < 0x40) hzoom = 0x40;

		// loop from top to bottom
		int minx = xpos;
		int maxx = xpos;
		int miny = cliprect.max_y + 1;
		int maxy = cliprect.min_y - 1;
		int yacc = 0;
		int ytarget = top + ydelta * height;
		for (int y = top; y != ytarget; y += ydelta)
		{
			// skip drawing if not within the cliprect
			if (y >= cliprect.min_y && y <= cliprect.max_y)
			{
				UINT16 *dest = &bitmap.pix(y);
				int xacc = 0;
				int x;

				// non-flipped case
				if (!flip)
				{
					// start at the word before because we preincrement below
					data[7] = addr - 1;
					for (x = xpos; (xdelta > 0 && x <= cliprect.max_x) || (xdelta < 0 && x >= cliprect.min_x); )
					{
						UINT32 pixels = spritedata[++data[7]];

						// draw four pixels
						int pix;
						pix = (pixels >> 28) & 0xf; while (xacc < 0x200) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 24) & 0xf; while (xacc < 0x200) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 20) & 0xf; while (xacc < 0x200) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 16) & 0xf; while (xacc < 0x200) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 12) & 0xf; while (xacc < 0x200) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  8) & 0xf; while (xacc < 0x200) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  4) & 0xf; while (xacc < 0x200) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  0) & 0xf; while (xacc < 0x200) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; xacc += hzoom; } xacc -= 0x200;

						// stop if the second-to-last pixel in the group was 0xf
						if ((pixels & 0x000000f0) == 0x000000f0)
							break;
					}
				}

				// flipped case
				else
				{
					// start at the word after because we predecrement below
					data[7] = addr + 1;
					for (x = xpos; (xdelta > 0 && x <= cliprect.max_x) || (xdelta < 0 && x >= cliprect.min_x); )
					{
						UINT32 pixels = spritedata[--data[7]];

						// draw four pixels
						int pix;
						pix = (pixels >>  0) & 0xf; while (xacc < 0x200) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  4) & 0xf; while (xacc < 0x200) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  8) & 0xf; while (xacc < 0x200) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 12) & 0xf; while (xacc < 0x200) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 16) & 0xf; while (xacc < 0x200) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 20) & 0xf; while (xacc < 0x200) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 24) & 0xf; while (xacc < 0x200) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 28) & 0xf; while (xacc < 0x200) { if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) dest[x] = colpri | pix; x += xdelta; xacc += hzoom; } xacc -= 0x200;

						// stop if the second-to-last pixel in the group was 0xf
						if ((pixels & 0x0f000000) == 0x0f000000)
							break;
					}
				}

				// update bounds
				if (x > maxx) maxx = x;
				if (x < minx) minx = x;
				if (y < miny) miny = y;
				if (y > maxy) maxy = y;
			}

			// accumulate zoom factors; if we carry into the high bit, skip an extra row
			yacc += vzoom;
			addr += pitch * (yacc >> 9);
			yacc &= 0x1ff;
		}

		// mark dirty
		if (minx <= maxx && miny <= maxy)
			mark_dirty(minx, maxx, miny, maxy);
	}
}



//****************************************************************************
//  Y BOARD-STYLE SPRITES
//****************************************************************************

//-------------------------------------------------
//  sega_yboard_sprite_device -- constructor
//-------------------------------------------------

sega_yboard_sprite_device::sega_yboard_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sega_16bit_sprite_device(mconfig, SEGA_YBOARD_SPRITES, "Sega Y-Board Sprites", tag, owner, "sega_yboard_sprite", __FILE__)
{
	set_local_origin(0x600, 0x600);
}


//-------------------------------------------------
//  draw -- render the sprites within the cliprect
//-------------------------------------------------

void sega_yboard_sprite_device::draw(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//
	//  Y-Board-style sprites
	//
	//      Offs  Bits               Usage
	//       +0   e------- --------  Signify end of sprite list
	//       +0   -----iii iiiiiiii  Address of indirection table (/16)
	//       +2   bbbb---- --------  Upper 4 bits of bank index
	//       +2   ----xxxx xxxxxxxx  X position of sprite (position $600 is screen position 0)
	//       +4   bbbb---- --------  Lower 4 bits of bank index
	//       +4   ----yyyy yyyyyyyy  Y position of sprite (position $600 is screen position 0)
	//       +6   oooooooo oooooooo  Offset within selected sprite bank
	//       +8   hhhhhhhh hhhhhhhh  Height of sprite
	//       +A   -y------ --------  Render from top-to-bottom (1) or bottom-to-top (0) on screen
	//       +A   --f----- --------  Horizontal flip: read the data backwards if set
	//       +A   ---x---- --------  Render from left-to-right (1) or right-to-left (0) on screen
	//       +A   -----zzz zzzzzzzz  Zoom factor
	//       +C   -ccc---- --------  Sprite color
	//       +C   ----rrrr --------  Sprite priority
	//       +C   -------- pppppppp  Signed 8-bit pitch value between scanlines
	//       +E   ----nnnn nnnnnnnn  Index of next sprite
	//
	//  Final bitmap format:
	//
	//            ccc----- --------  Sprite color
	//            ---rrrr- --------  Sprite priority
	//            -------i iiiiiiii  Indirected color data
	//
	//  In addition to these parameters, the sprite area is clipped using scanline extents
	//  stored for every pair of scanlines in the rotation RAM. It's a bit of a cheat for us
	//  to poke our nose into the rotation structure, but there are no known cases of Y-board
	//  sprites without rotation RAM.
	//

	set_origin(m_xoffs, m_yoffs);

	// clear out any scanlines we might be using
	const UINT16 *rotatebase = m_segaic16_rotate[0].buffer ? m_segaic16_rotate[0].buffer.get() : m_segaic16_rotate[0].rotateram;
	rotatebase -= yorigin();
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		if (!(rotatebase[y & ~1] & 0xc000))
			memset(&bitmap.pix(y, cliprect.min_x), 0xff, cliprect.width() * sizeof(UINT16));

	// reset the visited list
	UINT8 visited[0x1000];
	memset(visited, 0, sizeof(visited));

	// render the sprites in order
	const UINT64 *spritebase = reinterpret_cast<const UINT64 *>(region()->base());
	UINT8 numbanks = region()->bytes() / 0x80000;
	int next = 0;
	for (UINT16 *data = spriteram(); !(data[0] & 0x8000) && !visited[next]; data = spriteram() + next * 8)
	{
		int hide    = (data[0] & 0x5000);
		const UINT16 *indirect = spriteram() + ((data[0] & 0x7ff) << 4);
		int bank    = ((data[1] >> 8) & 0x10) | ((data[2] >> 12) & 0x0f);
		int xpos    = data[1] & 0xfff;
		int top     = data[2] & 0xfff;
		UINT16 addr = data[3];
		int height  = data[4];
		int ydelta  = (data[5] & 0x4000) ? 1 : -1;
		int flip    = (~data[5] >> 13) & 1;
		int xdelta  = (data[5] & 0x1000) ? 1 : -1;
		int zoom    = data[5] & 0x7ff;
		int colpri  = (data[6] << 1) & 0xfe00;
		int pitch   = INT8(data[6]);

		// note that we've visited this entry and get the offset of the next one
		visited[next] = 1;
		next = data[7] & 0xfff;

		// if hidden, or invalid height, punt
		if (hide || height == 0)
			continue;

		// clamp to within the memory region size
		if (numbanks)
			bank %= numbanks;
		const UINT64 *spritedata = spritebase + 0x10000 * bank;

		// clamp to a maximum of 8x (not 100% confirmed)
		if (zoom == 0) zoom = 1;

		// loop from top to bottom
		int dminx = xpos;
		int dmaxx = xpos;
		int dminy = cliprect.max_y + 1;
		int dmaxy = cliprect.min_y - 1;
		int ytarget = top + ydelta * height;
		int yacc = 0;
		for (int y = top; y != ytarget; y += ydelta)
		{
			// skip drawing if not within the cliprect
			if (y >= cliprect.min_y && y <= cliprect.max_y)
			{
				UINT16 *dest = &bitmap.pix(y);
				int minx = rotatebase[y & ~1];
				int maxx = rotatebase[y |  1];
				int xacc = 0;

				// bit 0x8000 from rotate RAM means that Y is above the top of the screen
				if ((minx & 0x8000) && ydelta < 0)
					break;

				// bit 0x4000 from rotate RAM means that Y is below the bottom of the screen
				if ((minx & 0x4000) && ydelta > 0)
					break;

				// if either bit is set, skip the rest for this scanline
				if (!(minx & 0xc000))
				{
					// clamp min/max to the cliprect
					if (minx < cliprect.min_x)
						minx = cliprect.min_x;
					if (maxx > cliprect.max_x)
						maxx = cliprect.max_x;

					// non-flipped case
					int x;
					if (!flip)
					{
						// start at the word before because we preincrement below
						UINT16 offs = addr - 1;
						for (x = xpos; (xdelta > 0 && x <= maxx) || (xdelta < 0 && x >= minx); )
						{
							UINT64 pixels = spritedata[++offs];

							// draw 16 pixels
							int pix, ind;
							pix = (pixels >> 60) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 56) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 52) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 48) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 44) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 40) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 36) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 32) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 28) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 24) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 20) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 16) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 12) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >>  8) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >>  4) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >>  0) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;

							// stop if the last pixel in the group was 0xf
							if (pix == 0x0f)
								break;
						}
					}

					// flipped case
					else
					{
						// start at the word after because we predecrement below
						UINT16 offs = addr + 1;
						for (x = xpos; (xdelta > 0 && x <= maxx) || (xdelta < 0 && x >= minx); )
						{
							UINT64 pixels = spritedata[--offs];

							// draw 16 pixels
							int pix, ind;
							pix = (pixels >>  0) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >>  4) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >>  8) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 12) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 16) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 20) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 24) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 28) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 32) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 36) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 40) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 44) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 48) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 52) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 56) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 60) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { if (x >= minx && x <= maxx && ind < 0x1fe) dest[x] = colpri | ind; x += xdelta; xacc += zoom; } xacc -= 0x200;

							// stop if the last pixel in the group was 0xf
							if (pix == 0x0f)
								break;
						}
					}

					// update bounds
					if (x > dmaxx) dmaxx = x;
					if (x < dminx) dminx = x;
					if (y < dminy) dminy = y;
					if (y > dmaxy) dmaxy = y;
				}
			}

			// accumulate zoom factors; if we carry into the high bit, skip an extra row
			yacc += zoom;
			addr += pitch * (yacc >> 9);
			yacc &= 0x1ff;
		}

		// mark dirty
		if (dminx <= dmaxx && dminy <= dmaxy)
			mark_dirty(dminx, dmaxx, dminy, dmaxy);
	}
}

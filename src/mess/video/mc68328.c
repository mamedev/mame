/**********************************************************************

    Motorola 68328 ("DragonBall") System-on-a-Chip LCD implementation

    By MooglyGuy
    contact mooglyguy@gmail.com with licensing and usage questions.

**********************************************************************/

#include "emu.h"
#include "includes/mc68328.h"
#include "machine/mc68328.h"
#include "machine/ram.h"

/* THIS IS PRETTY MUCH TOTALLY WRONG AND DOESN'T REFLECT THE MC68328'S INTERNAL FUNCTIONALITY AT ALL! */
PALETTE_INIT( mc68328 )
{
	palette_set_color_rgb(machine, 0, 0x7b, 0x8c, 0x5a);
	palette_set_color_rgb(machine, 1, 0x00, 0x00, 0x00);
}

VIDEO_START( mc68328 )
{
}

/* THIS IS PRETTY MUCH TOTALLY WRONG AND DOESN'T REFLECT THE MC68328'S INTERNAL FUNCTIONALITY AT ALL! */
SCREEN_UPDATE_IND16( mc68328 )
{
	device_t *mc68328_device = screen.machine().device(MC68328_TAG);
	mc68328_t* mc68328 = mc68328_get_safe_token( mc68328_device );

	const UINT16 *video_ram = (const UINT16 *)(screen.machine().device<ram_device>(RAM_TAG)->pointer() + (mc68328->regs.lssa & 0x00ffffff));
	UINT16 word;
	UINT16 *line;
	int y, x, b;

	if(mc68328->regs.lckcon & LCKCON_LCDC_EN)
	{
		for (y = 0; y < 160; y++)
		{
			line = &bitmap.pix16(y);

			for (x = 0; x < 160; x += 16)
			{
				word = *(video_ram++);
				for (b = 0; b < 16; b++)
				{
					line[x + b] = (word >> (15 - b)) & 0x0001;
				}
			}
		}
	}
	else
	{
		for (y = 0; y < 160; y++)
		{
			line = &bitmap.pix16(y);

			for (x = 0; x < 160; x++)
			{
				line[x] = 0;
			}
		}
	}
	return 0;
}

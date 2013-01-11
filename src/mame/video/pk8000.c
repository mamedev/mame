#include "emu.h"
#include "includes/pk8000.h"

static UINT8 pk8000_text_start;
static UINT8 pk8000_chargen_start;
static UINT8 pk8000_video_start;
static UINT8 pk8000_color_start;

UINT8 pk8000_video_mode;
static UINT8 pk8000_video_color;
static UINT8 pk8000_color[32];
UINT8 pk8000_video_enable;

READ8_HANDLER(pk8000_video_color_r)
{
	return pk8000_video_color;
}

WRITE8_HANDLER(pk8000_video_color_w)
{
	pk8000_video_color = data;
}

READ8_HANDLER(pk8000_text_start_r)
{
	return pk8000_text_start;
}

WRITE8_HANDLER(pk8000_text_start_w)
{
	pk8000_text_start = data;
}

READ8_HANDLER(pk8000_chargen_start_r)
{
	return pk8000_chargen_start;
}

WRITE8_HANDLER(pk8000_chargen_start_w)
{
	pk8000_chargen_start = data;
}

READ8_HANDLER(pk8000_video_start_r)
{
	return pk8000_video_start;
}

WRITE8_HANDLER(pk8000_video_start_w)
{
	pk8000_video_start = data;
}

READ8_HANDLER(pk8000_color_start_r)
{
	return pk8000_color_start;
}

WRITE8_HANDLER(pk8000_color_start_w)
{
	pk8000_color_start = data;
}

READ8_HANDLER(pk8000_color_r)
{
	return pk8000_color[offset];
}

WRITE8_HANDLER(pk8000_color_w)
{
	pk8000_color[offset] = data;
}

static const rgb_t pk8000_palette[16] = {
	MAKE_RGB(0x00, 0x00, 0x00), // 0
	MAKE_RGB(0x00, 0x00, 0x00), // 1
	MAKE_RGB(0x00, 0xc0, 0x00), // 2
	MAKE_RGB(0x00, 0xff, 0x00), // 3
	MAKE_RGB(0x00, 0x00, 0xc0), // 4
	MAKE_RGB(0x00, 0x00, 0xff), // 5
	MAKE_RGB(0x00, 0xc0, 0xc0), // 6
	MAKE_RGB(0x00, 0xff, 0xff), // 7
	MAKE_RGB(0xc0, 0x00, 0x00), // 8
	MAKE_RGB(0xff, 0x00, 0x00), // 9
	MAKE_RGB(0xc0, 0xc0, 0x00), // A
	MAKE_RGB(0xff, 0xff, 0x00), // B
	MAKE_RGB(0xc0, 0x00, 0xc0), // C
	MAKE_RGB(0xff, 0x00, 0xff), // D
	MAKE_RGB(0xc0, 0xc0, 0xc0), // E
	MAKE_RGB(0xff, 0xff, 0xff), // F
};

PALETTE_INIT( pk8000 )
{
	palette_set_colors(machine, 0, pk8000_palette, ARRAY_LENGTH(pk8000_palette));
}

UINT32 pk8000_video_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 *videomem)
{
	int x,y,j,b;
	UINT16 offset = (pk8000_video_mode & 0xc0) << 8;
	rectangle my_rect;
	my_rect.set(0, 256+32-1, 0, 192+32-1);

	if (pk8000_video_enable) {
		bitmap.fill((pk8000_video_color >> 4) & 0x0f, my_rect);

		if (BIT(pk8000_video_mode,4)==0){
			// Text mode
			if (BIT(pk8000_video_mode,5)==0){
				// 32 columns
				for (y = 0; y < 24; y++)
				{
					for (x = 0; x < 32; x++)
					{
						UINT8 chr  = videomem[x +(y*32) + ((pk8000_text_start & 0x0f) << 10)+offset] ;
						UINT8 color= pk8000_color[chr>>3];
						for (j = 0; j < 8; j++) {
							UINT8 code = videomem[((chr<<3) + j) + ((pk8000_chargen_start & 0x0e) << 10)+offset];

							for (b = 0; b < 8; b++)
							{
								UINT8 col = (code >> b) & 0x01 ? (color & 0x0f) : ((color>>4) & 0x0f);
								bitmap.pix16((y*8)+j+16, x*8+(7-b)+16) =  col;
							}
						}
					}
				}
			} else {
				// 40 columns
				for (y = 0; y < 24; y++)
				{
					for (x = 0; x < 42; x++)
					{
						UINT8 chr = videomem[x +(y*64) + ((pk8000_text_start & 0x0e) << 10)+offset] ;
						for (j = 0; j < 8; j++) {
							UINT8 code = videomem[((chr<<3) + j) + ((pk8000_chargen_start  & 0x0e) << 10)+offset];
							for (b = 2; b < 8; b++)
							{
								UINT8 col = ((code >> b) & 0x01) ? (pk8000_video_color) & 0x0f : (pk8000_video_color>>4) & 0x0f;
								bitmap.pix16((y*8)+j+16, x*6+(7-b)+16+8) =  col;
							}
						}
					}
				}
			}
		} else {
			//Graphics
			for (y = 0; y < 24; y++)
			{
				UINT16 off_color = (((~pk8000_color_start) & 0x08) << 10)+offset + ((y>>3)<<11);
				UINT16 off_code  = (((~pk8000_video_start) & 0x08) << 10)+offset + ((y>>3)<<11);
				for (x = 0; x < 32; x++)
				{
					UINT8 chr  = videomem[x +(y*32) + ((pk8000_chargen_start & 0x0e) << 10)+offset] ;
					for (j = 0; j < 8; j++) {
						UINT8 color= videomem[((chr<<3) + j)+off_color];
						UINT8 code = videomem[((chr<<3) + j)+off_code];

						for (b = 0; b < 8; b++)
						{
							UINT8 col = (code >> b) & 0x01 ? (color & 0x0f) : ((color>>4) & 0x0f);
							bitmap.pix16((y*8)+j+16, x*8+(7-b)+16) =  col;
						}
					}
				}
			}
		}
	} else {
		// Disabled video
		bitmap.fill(0, my_rect);
	}
	return 0;
}

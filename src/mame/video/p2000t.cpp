// license:BSD-3-Clause
// copyright-holders:Paul Daniels
/**********************************************************************

    p2000m.c

    Functions to emulate video hardware of the p2000m

**********************************************************************/

#include "emu.h"
#include "includes/p2000t.h"

void p2000m_state::video_start()
{
	m_frame_count = 0;
}


uint32_t p2000m_state::screen_update_p2000m(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const videoram = m_videoram;

	for (int offs = 0; offs < 80 * 24; offs++)
	{
		int sy = (offs / 80) * 20;
		int sx = (offs % 80) * 12;

		int code;
		if ((m_frame_count > 25) && (videoram[offs + 2048] & 0x40))
			code = 32;
		else
		{
			code = videoram[offs];
			if ((videoram[offs + 2048] & 0x01) && (code & 0x20))
			{
				code += (code & 0x40) ? 64 : 96;
			} else {
				code &= 0x7f;
			}
			if (code < 32) code = 32;
		}

		m_gfxdecode->gfx(0)->zoom_opaque(bitmap,cliprect, code,
			videoram[offs + 2048] & 0x08 ? 0 : 1, 0, 0, sx, sy, 0x20000, 0x20000);

		if (videoram[offs] & 0x80)
		{
			for (int loop = 0; loop < 12; loop++)
			{
				bitmap.pix(sy + 18, sx + loop) = 0;   /* cursor */
				bitmap.pix(sy + 19, sx + loop) = 0;   /* cursor */
			}
		}
	}

	return 0;
}

/*
 *  Implementation of the P2000T hires graphicscard which was sold as extention board
 *  This emulation was based on documentation from the P2000T presevation page
 *  https://github.com/p2000t/documentation/tree/master/hardware [HiRes.pdf]
 *  To work properly the GOD36.BIN rom and "Taaltje 1.1 32K.cas" is needed 
 *  https://github.com/p2000t/software/tree/master/cartridges
*/

#define HIRES_IMAGE_RED_ENALBE_BIT 0
#define HIRES_IMAGE_GREEN_ENABLE_BIT 1
#define HIRES_IMAGE_BLUE_ENABLE_BIT 2
#define HIRES_IMAGE_P2000T_ENABLE_BIT 3

#define HIRES_MODE_PAGE_4_BIT 0
#define HIRES_MODE_PAGE_5_BIT 1
#define HIRES_MODE_PAGE_6_BIT 2
#define HIRES_MODE_PAGE_7_BIT 3
#define HIRES_MODE_UP_DOWN_BIT 4
#define HIRES_MODE_512_BIT 5
#define HIRES_MODE_1_ON_1_BIT 6
#define HIRES_MODE_RESSH_BIT 7

uint32_t p2000h_state::screen_update_p2000h(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	float PIX_TRANS_X = (480 / 25.6);
	float PIX_TRANS_Y = (480 / 25.6);
	float PIX_TRANS_X_OFSET = 0;
	int PIX_TRANS_X_WIDTH = 0;

	hirespio_emulate_sync();
	
	/*
	 * m_hires_image_select, when set
	 * bit  0: Red channel visible
	 * bit  1: GREEN channel visible
	 * bit  2: BLUE channel visible
	 * bit  3: P2000T channel visible
	 */
	if (BIT(m_hires_image_select, HIRES_IMAGE_P2000T_ENABLE_BIT)) {
		m_saa5050->screen_update(screen, bitmap, cliprect);
	}

	if (BIT(m_hires_image_select, HIRES_IMAGE_RED_ENALBE_BIT) || 
		BIT(m_hires_image_select, HIRES_IMAGE_BLUE_ENABLE_BIT) || 
		BIT(m_hires_image_select, HIRES_IMAGE_GREEN_ENABLE_BIT)) {

		uint8_t  pixelByte0, pixelByte1, pixelByte2, pixelByte3;
		int ofsByte0, ofsByte1, ofsByte2, ofsByte3;

		if (BIT(m_hires_image_mode, HIRES_MODE_512_BIT)) {
			// Set tranlate parameters for 512 pixels per line
			if (BIT(m_hires_image_mode, HIRES_MODE_1_ON_1_BIT)) {
				PIX_TRANS_X = ((in_80char_mode() ? 800 : 400) / 51.2);
				PIX_TRANS_X_OFSET = (in_80char_mode() ? 60 : 30);
			} else {
				PIX_TRANS_X = ((in_80char_mode() ? 960 : 480) / 51.2);
				PIX_TRANS_X_OFSET = 0;
			}
			PIX_TRANS_X_WIDTH = (in_80char_mode() ? 2 : 1);

			ofsByte0 = (BIT(m_hires_image_mode, HIRES_MODE_PAGE_4_BIT) ? 4 : 0) * 0x2000;
			ofsByte1 = (BIT(m_hires_image_mode, HIRES_MODE_PAGE_5_BIT) ? 6 : 2) * 0x2000;
			ofsByte2 = (BIT(m_hires_image_mode, HIRES_MODE_PAGE_4_BIT) ? 5 : 1) * 0x2000;
			ofsByte3 = (BIT(m_hires_image_mode, HIRES_MODE_PAGE_5_BIT) ? 7 : 3) * 0x2000;
		} else {
			// Set tranlate parameters for 256 pixels per line
			if (BIT(m_hires_image_mode, HIRES_MODE_1_ON_1_BIT)) {
				PIX_TRANS_X = ((in_80char_mode() ? 800 : 400) / 25.6);
				PIX_TRANS_X_OFSET = (in_80char_mode() ? 60 : 30);
			} else {
				PIX_TRANS_X = ((in_80char_mode() ? 960 : 480) / 25.6);
				PIX_TRANS_X_OFSET = 0;
			}
			PIX_TRANS_X_WIDTH = (in_80char_mode() ? 4 : 2);

			ofsByte0 = (BIT(m_hires_image_mode, HIRES_MODE_PAGE_4_BIT) ? 4 : 0) * 0x2000;
			ofsByte1 = (BIT(m_hires_image_mode, HIRES_MODE_PAGE_5_BIT) ? 5 : 1) * 0x2000;
			ofsByte2 = (BIT(m_hires_image_mode, HIRES_MODE_PAGE_6_BIT) ? 6 : 2) * 0x2000;
			ofsByte3 = (BIT(m_hires_image_mode, HIRES_MODE_PAGE_7_BIT) ? 7 : 3) * 0x2000;
		}
		
		int pixel = 0; 
		uint32_t color = 0;
		int ypos = 0;
		for (int yposcnt = 0; yposcnt < 256; yposcnt++) {
			// Y-lines are stored reversed in memory also take scroll reg into account
			ypos = (m_hires_scroll_reg + yposcnt) % 256;
			// Bit 4 of image mode toggles up-side down
			if (!BIT(m_hires_image_mode, HIRES_MODE_UP_DOWN_BIT)) {
				ypos = 256 - ypos;
			}
			
			if (BIT(m_hires_image_mode, HIRES_MODE_512_BIT)) {
				// We are in 512 pixels per line
				for (int xpos = 0; xpos < (512 / 16); xpos++) {
					// Read per byte (representing 2 times 8 pixels of 2 bits)
					pixelByte0 = m_hiresram->read(ofsByte1 + (ypos * 32) + xpos);
					pixelByte1 = m_hiresram->read(ofsByte0 + (ypos * 32) + xpos);

					pixelByte2 = m_hiresram->read(ofsByte3 + (ypos * 32) + xpos);
					pixelByte3 = m_hiresram->read(ofsByte2 + (ypos * 32) + xpos);
					for (int xposb = 0; xposb < 8; xposb++) {
						// if ressh bit is set (bit 7) a black hires image is generated 
						if (!BIT(m_hires_image_mode, HIRES_MODE_RESSH_BIT) ) {
							// Each video line has 512 pixels (so 16 bit * 32 bytes)
							// Per pixel use 1 bit of the 2 video pages combined pages as 0-1, 3-4, 5-6, 7-8
							// In 512 mode the color LUTs are  0=0,1,2,4,5 1=2,3,6,7 2=8,9,c,d 3=a,b,e,f
							pixel = (BIT(pixelByte0, xposb)) << 3 |	(BIT(pixelByte1, xposb)) << 1;
							color = rgb_t(
										BIT(m_hires_image_select, 0) ? m_hires_LutRed[pixel] : 0, 
										BIT(m_hires_image_select, 1) ? m_hires_LutGreen[pixel] : 0,	
										BIT(m_hires_image_select, 2) ? m_hires_LutBlue[pixel] : 0
									);
							pixel = (BIT(pixelByte2, xposb)) << 3 |	(BIT(pixelByte3, xposb)) << 1;
							// Scale one pixel in 512*256 grid to multiple pixels in 480*480 grid (so we loose some pixels)
							screen_update_p2000h_draw_pixel(bitmap, 
												 	(PIX_TRANS_Y * yposcnt) / 10, 
												   ((PIX_TRANS_X * ((xpos * 16) + (15-(xposb * 2)))) / 10) + PIX_TRANS_X_OFSET, 
													color, 2, PIX_TRANS_X_WIDTH);

							color = rgb_t(
										BIT(m_hires_image_select, 0) ? m_hires_LutRed[pixel] : 0, 
										BIT(m_hires_image_select, 1) ? m_hires_LutGreen[pixel] : 0,	
										BIT(m_hires_image_select, 2) ? m_hires_LutBlue[pixel]  : 0
									);
							// Scale one pixel in 512*256 grid to multiple pixels in 480*480 grid (so we loose some pixels)
							screen_update_p2000h_draw_pixel(bitmap, 
											 		 (PIX_TRANS_Y * yposcnt) / 10,
													((PIX_TRANS_X * ((xpos * 16) + (16-(xposb * 2)))) / 10) + PIX_TRANS_X_OFSET, 
												 	 color, 2, PIX_TRANS_X_WIDTH);
						} else {
							// Scale one pixel in 512*256 grid to multiple pixels in 480*480 grid
							screen_update_p2000h_draw_pixel(bitmap, 
													 (PIX_TRANS_Y * yposcnt) / 10, 
													((PIX_TRANS_X * ((xpos * 8) + (16-xposb))) / 10) + PIX_TRANS_X_OFSET,
													 rgb_t::black(), 2, PIX_TRANS_X_WIDTH * 2);
						}
					}
				}
			} else {
				for (int xpos = 0; xpos < (256 / 8); xpos++) {
					// Read per byte (representing 8 pixels of 8 bit color)
					pixelByte0 = m_hiresram->read(ofsByte0 + (ypos * 32) + xpos);
					pixelByte1 = m_hiresram->read(ofsByte1 + (ypos * 32) + xpos);
					pixelByte2 = m_hiresram->read(ofsByte2 + (ypos * 32) + xpos);
					pixelByte3 = m_hiresram->read(ofsByte3 + (ypos * 32) + xpos);
					for (int xposb = 0; xposb < 8; xposb++) {

						// if ressh bit is set (bit 7) a black hires image is generated 
						if (!BIT(m_hires_image_mode, HIRES_MODE_RESSH_BIT) ) {
							// Each video line has 256 pixels (so 8 bit * 32 bytes)
							// Per pixel use 1 bit of the 4 video pages
							pixel = (BIT(pixelByte3, xposb)) << 3 |
									(BIT(pixelByte2, xposb)) << 2 |
									(BIT(pixelByte1, xposb)) << 1 |
									(BIT(pixelByte0, xposb));
							color = rgb_t(
										BIT(m_hires_image_select, 0) ? m_hires_LutRed[pixel] : 0, 
										BIT(m_hires_image_select, 1) ? m_hires_LutGreen[pixel] : 0,	
										BIT(m_hires_image_select, 2) ? m_hires_LutBlue[pixel] : 0
									);
						} else {
							color =  rgb_t::black();
						}
						// Scale one pixel in 256*256 grid to multiple pixels in 480*480 grid
						screen_update_p2000h_draw_pixel(bitmap, 
										 (PIX_TRANS_Y * yposcnt) / 10, 
										((PIX_TRANS_X * ((xpos * 8) + (8-xposb))) / 10) + PIX_TRANS_X_OFSET,
										 color, 2, PIX_TRANS_X_WIDTH);
					}
				}
			}
		}
	}
	return 0;
}

void p2000h_state::screen_update_p2000h_draw_pixel(bitmap_rgb32 &bitmap, int ypos, int xpos, uint32_t color, int ylen, int xlen )
{
	for (int absypos = ypos; absypos < ylen + ypos; absypos++ ) {
		for (int absxpos = xpos; absxpos < xlen + xpos; absxpos++ ) {
			if (absypos < 480 && absxpos < 960 ) {
				// Do not overwrite P2000T image pixel
				if (!BIT(m_hires_image_select, HIRES_IMAGE_P2000T_ENABLE_BIT) || (bitmap.pix(absypos, absxpos) & 0x00ffffff) == 0) {
					bitmap.pix(absypos, absxpos) = color;
				}
			}
		}
	}
	
}

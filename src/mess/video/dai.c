// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha, Nathan Woods
/***************************************************************************

  dai.c

  Functions to emulate the video hardware of DAI.

  Krzysztof Strzecha

  All video modes are emulated but not fully tested yet.
  dai_state::screen_update_dai() function needs strong cleanup and optimalisation.


***************************************************************************/

#include "emu.h"
#include "includes/dai.h"

#define DEBUG_DAI_VIDEO 0

#define LOG_DAI_VIDEO_LINE(_mode, _unit, _resolution, _repeat, _scan) do { if (DEBUG_DAI_VIDEO) logerror ("Mode: %02x, Unit: %02x, Resolution: %02x, Repeat: %d, Current line: %d\n", _mode, _unit, _resolution, _repeat, _scan); } while (0)

const unsigned char dai_palette[16*3] =
{
	0x00, 0x00, 0x00,   /*  0 Black     */
	0x00, 0x00, 0x8b,   /*  1 Dark Blue     */
	0xb1, 0x00, 0x95,   /*  2 Purple Red    */
	0xff, 0x00, 0x00,   /*  3 Red       */
	0x75, 0x2e, 0x50,   /*  4 Purple Brown  */
	0x00, 0xb2, 0x38,   /*  5 Emerald Green */
	0x98, 0x62, 0x00,   /*  6 Kakhi Brown   */
	0xae, 0x7a, 0x00,   /*  7 Mustard Brown */
	0x89, 0x89, 0x89,   /*  8 Grey      */
	0xa1, 0x6f, 0xff,   /*  9 Middle Blue   */
	0xff, 0xa5, 0x00,   /* 10 Orange        */
	0xff, 0x99, 0xff,   /* 11 Pink      */
	0x9e, 0xf4, 0xff,   /* 12 Light Blue    */
	0xb3, 0xff, 0xbb,   /* 13 Light Green   */
	0xff, 0xff, 0x28,   /* 14 Light Yellow  */
	0xff, 0xff, 0xff,   /* 15 White     */
};


PALETTE_INIT_MEMBER(dai_state, dai)
{
	int i;

	for ( i = 0; i < sizeof(dai_palette) / 3; i++ )
	{
		m_palette->set_pen_color(i, dai_palette[i * 3], dai_palette[i * 3 + 1], dai_palette[i * 3 + 2]);
	}
}


void dai_state::video_start()
{
}

UINT32 dai_state::screen_update_dai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int i, j, k, l;

	UINT8* char_rom = memregion("gfx1")->base();

	UINT16 dai_video_memory_start = 0xbfff;
	UINT16 dai_scan_lines = 604;    /* scan lines of PAL tv */

	UINT16 current_scan_line = 0;
	UINT16 current_video_memory_address = dai_video_memory_start;

	UINT8 mode;         /* mode byte of line
                       bits 0-3 - line repeat count
                       bits 4-5 - resolution control
                       bits 6-7 - display mode control */
	UINT8 colour;           /* colour byte of line
                       bits 0-3 - one of 16 colours
                       bits 4-5 - colour register for update
                       bit  6   - if unset force 'unit colour mode'
                       bit  7   - enable coulor change
                                  if unset bits 0-5 are ignored */
	UINT8 line_repeat_count;    /* number of horizontalraster scans
                       for which same data will be displayed
                       0000 - 2 lines
                       each additional repeat adds 2 scans */
	UINT8 horizontal_resolution;    /* number of blobs per line
                       00 - 88 (low resolution graphics)
                       01 - 176 (medium resolution graphics)
                       10 - 352 (high resolution graphics)
                       11 - 528 (text with 66 chars per line) */
	UINT8 display_mode;     /* determine how data will be used
                       to generate the picture
                       00 - four colour graphics
                       01 - four colour characters
                       10 - sixteen colour graphics
                       11 - sixteen colour characters */
	UINT8 unit_mode;

	UINT8 current_data_1, current_data_2;

	UINT8 current_colour;

	while (current_scan_line < dai_scan_lines)
	{
		mode = space.read_byte(current_video_memory_address--);
		colour = space.read_byte(current_video_memory_address--);
		line_repeat_count = mode & 0x0f;
		horizontal_resolution = (mode & 0x30) >> 4;
		display_mode = (mode & 0xc0) >> 6;
		unit_mode = (colour & 0x40) >> 6;

		if (colour & 0x80)
			m_4_colours_palette[(colour & 0x30) >> 4] = colour & 0x0f;

		switch (display_mode)
		{
		case 0x00:  /* 4 colour grahics modes */
			switch (horizontal_resolution)
			{
			case 0x00:  /* 88 pixels */
				switch (unit_mode)
				{
				case 0:
					current_data_1 = space.read_byte(current_video_memory_address--);
					current_data_2 = space.read_byte(current_video_memory_address--);

					for (i=0; i<11; i++)
					{
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = m_4_colours_palette[(((current_data_1>>(7-k)) & 0x01)<<1) | ((current_data_2>>(7-k)) & 0x01)];
								for (l=0; l<12; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*12+l) = current_colour;
							}
						}
					}
					current_video_memory_address-=2;
					break;
				case 1:
					for (i=0; i<11; i++)
					{
						current_data_1 = space.read_byte(current_video_memory_address--);
						current_data_2 = space.read_byte(current_video_memory_address--);
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = m_4_colours_palette[(((current_data_1>>(7-k)) & 0x01)<<1) | ((current_data_2>>(7-k)) & 0x01)];
								for (l=0; l<12; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*12+l) = current_colour;
							}
						}
					}
					break;
				}
				break;

			case 0x01:  /* 176 pixels */
				switch (unit_mode)
				{
				case 0:
					current_data_1 = space.read_byte(current_video_memory_address--);
					current_data_2 = space.read_byte(current_video_memory_address--);
					for (i=0; i<22; i++)
					{
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = m_4_colours_palette[(((current_data_1>>(7-k)) & 0x01)<<1) | ((current_data_2>>(7-k)) & 0x01)];
								for (l=0; l<6; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*6+l) = current_colour;
							}
						}
					}
					current_video_memory_address-=2;
					break;
				case 1:
					for (i=0; i<22; i++)
					{
						current_data_1 = space.read_byte(current_video_memory_address--);
						current_data_2 = space.read_byte(current_video_memory_address--);
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = m_4_colours_palette[(((current_data_1>>(7-k)) & 0x01)<<1) | ((current_data_2>>(7-k)) & 0x01)];
								for (l=0; l<6; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*6+l) = current_colour;
							}
						}
					}
					break;
				}
				break;

			case 0x02:  /* 352 pixels */
				switch (unit_mode)
				{
				case 0:
					current_data_1 = space.read_byte(current_video_memory_address--);
					current_data_2 = space.read_byte(current_video_memory_address--);
					for (i=0; i<44; i++)
					{
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = m_4_colours_palette[(((current_data_1>>(7-k)) & 0x01)<<1) | ((current_data_2>>(7-k)) & 0x01)];
								for (l=0; l<3; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*3+l) = current_colour;
							}
						}
					}
					break;
				case 1:
					for (i=0; i<44; i++)
					{
						current_data_1 = space.read_byte(current_video_memory_address--);
						current_data_2 = space.read_byte(current_video_memory_address--);
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = m_4_colours_palette[(((current_data_1>>(7-k)) & 0x01)<<1) | ((current_data_2>>(7-k)) & 0x01)];
								for (l=0; l<3; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*3+l) = current_colour;
							}
						}
					}
					break;
				}
				break;

			case 0x03:  /* 528 pixels */
				switch (unit_mode)
				{
				case 0:
					current_data_1 = space.read_byte(current_video_memory_address--);
					current_data_2 = space.read_byte(current_video_memory_address--);
					for (i=0; i<66; i++)
					{
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = m_4_colours_palette[(((current_data_1>>(7-k)) & 0x01)<<1) | ((current_data_2>>(7-k)) & 0x01)];
								for (l=0; l<2; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*2+l) = current_colour;
							}
						}
					}
					break;
				case 1:
					for (i=0; i<66; i++)
					{
						current_data_1 = space.read_byte(current_video_memory_address--);
						current_data_2 = space.read_byte(current_video_memory_address--);
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = m_4_colours_palette[(((current_data_1>>(7-k)) & 0x01)<<1) | ((current_data_2>>(7-k)) & 0x01)];
								for (l=0; l<2; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*2+l) = current_colour;
							}
						}
					}
					break;
				}
				break;
			}
			break;

		case 0x01:  /* 4 colour characters */
			switch (horizontal_resolution)
			{
			case 0x00:  /* 11 chars */
				switch (unit_mode)
				{
				case 0:
					current_data_1 = space.read_byte(current_video_memory_address);
					current_data_2 = space.read_byte(current_video_memory_address-1);
					current_video_memory_address-=2;
					for (i=0; i<11; i++)
					{
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = m_4_colours_palette[(((current_data_2 >> k)&0x01)<<1) | ((char_rom[current_data_1*16+j]>>k) & 0x01)];
								for (l=0; l<12; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*12+l) = current_colour;
							}
						}
					}
					current_video_memory_address-=2;
					break;
				case 1:
					for (i=0; i<11; i++)
					{
						current_data_1 = space.read_byte(current_video_memory_address);
						current_data_2 = space.read_byte(current_video_memory_address-1);
						current_video_memory_address-=2;
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = m_4_colours_palette[(((current_data_2 >> k)&0x01)<<1) | ((char_rom[current_data_1*16+j]>>k) & 0x01)];
								for (l=0; l<12; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*12+l) = current_colour;
							}
						}
					}
					current_video_memory_address-=2;
					break;
				}
				break;
			case 0x01:  /* 22 chars */
				switch (unit_mode)
				{
				case 0:
					current_data_1 = space.read_byte(current_video_memory_address);
					current_data_2 = space.read_byte(current_video_memory_address-1);
					current_video_memory_address-=2;
					for (i=0; i<22; i++)
					{
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = m_4_colours_palette[(((current_data_2 >> k)&0x01)<<1) | ((char_rom[current_data_1*16+j]>>k) & 0x01)];
								for (l=0; l<6; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*6+l) = current_colour;
							}
						}
					}
					current_video_memory_address-=2;
					break;
				case 1:
					for (i=0; i<22; i++)
					{
						current_data_1 = space.read_byte(current_video_memory_address);
						current_data_2 = space.read_byte(current_video_memory_address-1);
						current_video_memory_address-=2;
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = m_4_colours_palette[(((current_data_2 >> k)&0x01)<<1) | ((char_rom[current_data_1*16+j]>>k) & 0x01)];
								for (l=0; l<6; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*6+l) = current_colour;
							}
						}
					}
					current_video_memory_address-=2;
					break;
				}
				break;
			case 0x02:  /* 44 chars */
				switch (unit_mode)
				{
				case 0:
					current_data_1 = space.read_byte(current_video_memory_address);
					current_data_2 = space.read_byte(current_video_memory_address-1);
					current_video_memory_address-=2;
					for (i=0; i<44; i++)
					{
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = m_4_colours_palette[(((current_data_2 >> k)&0x01)<<1) | ((char_rom[current_data_1*16+j]>>k) & 0x01)];
								for (l=0; l<3; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*3+l) = current_colour;
							}
						}
					}
					break;
				case 1:
					for (i=0; i<44; i++)
					{
						current_data_1 = space.read_byte(current_video_memory_address);
						current_data_2 = space.read_byte(current_video_memory_address-1);
						current_video_memory_address-=2;
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = m_4_colours_palette[(((current_data_2 >> k)&0x01)<<1) | ((char_rom[current_data_1*16+j]>>k) & 0x01)];
								for (l=0; l<3; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*3+l) = current_colour;
							}
						}
					}
					break;
				}
				break;
			case 0x03:  /* 66 chars */
				switch (unit_mode)
				{
				case 0:
					current_data_1 = space.read_byte(current_video_memory_address);
					current_data_2 = space.read_byte(current_video_memory_address-1);
					current_video_memory_address-=2;
					for (i=0; i<66; i++)
					{
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = m_4_colours_palette[(((current_data_2 >> k)&0x01)<<1) | ((char_rom[current_data_1*16+j]>>k) & 0x01)];
								for (l=0; l<2; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*2+l) = current_colour;
							}
						}
					}
					break;
				case 1:
					for (i=0; i<66; i++)
					{
						current_data_1 = space.read_byte(current_video_memory_address);
						current_data_2 = space.read_byte(current_video_memory_address-1);
						current_video_memory_address-=2;
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = m_4_colours_palette[(((current_data_2 >> k)&0x01)<<1) | ((char_rom[current_data_1*16+j]>>k) & 0x01)];
								for (l=0; l<2; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*2+l) = current_colour;
							}
						}
					}
					break;
				}
				break;
			}
			break;
				case 0x02:  /* 16 colour graphics */
			switch (horizontal_resolution)
			{
			case 0x00:  /* 88 pixels */
				switch (unit_mode)
				{
				case 0:
					current_data_1 = space.read_byte(current_video_memory_address--);
					current_data_2 = space.read_byte(current_video_memory_address--);

					for (i=0; i<11; i++)
					{
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = ((current_data_1>>(7-k)) & 0x01) ? (current_data_2>>4)&0x0f : current_data_2&0x0f;
								for (l=0; l<12; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*12+l) = current_colour;
							}
						}
					}
					current_video_memory_address-=2;
					break;
				case 1:
					for (i=0; i<11; i++)
					{
						current_data_1 = space.read_byte(current_video_memory_address--);
						current_data_2 = space.read_byte(current_video_memory_address--);
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = ((current_data_1>>(7-k)) & 0x01) ? (current_data_2>>4)&0x0f : current_data_2&0x0f;
								for (l=0; l<12; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*12+l) = current_colour;
							}
						}
					}
					break;
				}
				break;

			case 0x01:  /* 176 pixels */
				switch (unit_mode)
				{
				case 0:
					current_data_1 = space.read_byte(current_video_memory_address--);
					current_data_2 = space.read_byte(current_video_memory_address--);
					for (i=0; i<22; i++)
					{
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = ((current_data_1>>(7-k)) & 0x01) ? (current_data_2>>4)&0x0f : current_data_2&0x0f;
								for (l=0; l<6; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*6+l) = current_colour;
							}
						}
					}
					current_video_memory_address-=2;
					break;
				case 1:
					for (i=0; i<22; i++)
					{
						current_data_1 = space.read_byte(current_video_memory_address--);
						current_data_2 = space.read_byte(current_video_memory_address--);
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = ((current_data_1>>(7-k)) & 0x01) ? (current_data_2>>4)&0x0f : current_data_2&0x0f;
								for (l=0; l<6; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*6+l) = current_colour;
							}
						}
					}
					break;
				}
				break;

			case 0x02:  /* 352 pixels */
				switch (unit_mode)
				{
				case 0:
					current_data_1 = space.read_byte(current_video_memory_address--);
					current_data_2 = space.read_byte(current_video_memory_address--);
					for (i=0; i<44; i++)
					{
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = ((current_data_1>>(7-k)) & 0x01) ? (current_data_2>>4)&0x0f : current_data_2&0x0f;
								for (l=0; l<3; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*3+l) = current_colour;
							}
						}
					}
					break;
				case 1:
					for (i=0; i<44; i++)
					{
						current_data_1 = space.read_byte(current_video_memory_address--);
						current_data_2 = space.read_byte(current_video_memory_address--);
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = ((current_data_1>>(7-k)) & 0x01) ? (current_data_2>>4)&0x0f : current_data_2&0x0f;
								for (l=0; l<3; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*3+l) = current_colour;
							}
						}
					}
					break;
				}
				break;

			case 0x03:  /* 528 pixels */
				switch (unit_mode)
				{
				case 0:
					current_data_1 = space.read_byte(current_video_memory_address--);
					current_data_2 = space.read_byte(current_video_memory_address--);
					for (i=0; i<66; i++)
					{
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = ((current_data_1>>(7-k)) & 0x01) ? (current_data_2>>4)&0x0f : current_data_2&0x0f;
								for (l=0; l<2; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*2+l) = current_colour;
							}
						}
					}
					break;
				case 1:
					for (i=0; i<66; i++)
					{
						current_data_1 = space.read_byte(current_video_memory_address--);
						current_data_2 = space.read_byte(current_video_memory_address--);
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = ((current_data_1>>(7-k)) & 0x01) ? (current_data_2>>4)&0x0f : current_data_2&0x0f;
								for (l=0; l<2; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*2+l) = current_colour;
							}
						}
					}
					break;
				}
				break;
			}
			break;
		case 0x03:  /* 16 colour characters */
			switch (horizontal_resolution)
			{
			case 0x00:  /* 11 chars */
				switch (unit_mode)
				{
				case 0:
					current_data_1 = space.read_byte(current_video_memory_address--);
					current_data_2 = space.read_byte(current_video_memory_address--);
					for (i=0; i<11; i++)
					{
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = ((char_rom[current_data_1*16+j]>>k) & 0x01) ? (current_data_2>>4)&0x0f : current_data_2&0x0f;
								for (l=0; l<12; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*12+l) = current_colour;
							}
						}
					}
					current_video_memory_address-=2;
					break;
				case 1:
					for (i=0; i<11; i++)
					{
						current_data_1 = space.read_byte(current_video_memory_address--);
						current_data_2 = space.read_byte(current_video_memory_address--);
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = ((char_rom[current_data_1*16+j]>>k) & 0x01) ? (current_data_2>>4)&0x0f : current_data_2&0x0f;
								for (l=0; l<12; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*12+l) = current_colour;
							}
						}
					}
					current_video_memory_address-=2;
					break;
				}
				break;
			case 0x01:  /* 22 chars */
				switch (unit_mode)
				{
				case 0:
					current_data_1 = space.read_byte(current_video_memory_address--);
					current_data_2 = space.read_byte(current_video_memory_address--);
					for (i=0; i<22; i++)
					{
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = ((char_rom[current_data_1*16+j]>>k) & 0x01) ? (current_data_2>>4)&0x0f : current_data_2&0x0f;
								for (l=0; l<6; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*6+l) = current_colour;
							}
						}
					}
					current_video_memory_address-=2;
					break;
				case 1:
					for (i=0; i<22; i++)
					{
						current_data_1 = space.read_byte(current_video_memory_address--);
						current_data_2 = space.read_byte(current_video_memory_address--);
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = ((char_rom[current_data_1*16+j]>>k) & 0x01) ? (current_data_2>>4)&0x0f : current_data_2&0x0f;
								for (l=0; l<6; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*6+l) = current_colour;
							}
						}
					}
					current_video_memory_address-=2;
					break;
				}
				break;
			case 0x02:  /* 44 chars */
				switch (unit_mode)
				{
				case 0:
					current_data_1 = space.read_byte(current_video_memory_address--);
					current_data_2 = space.read_byte(current_video_memory_address--);
					for (i=0; i<44; i++)
					{
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = ((char_rom[current_data_1*16+j]>>k) & 0x01) ? (current_data_2>>4)&0x0f : current_data_2&0x0f;
								for (l=0; l<3; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*3+l) = current_colour;
							}
						}
					}
					break;
				case 1:
					for (i=0; i<44; i++)
					{
						current_data_1 = space.read_byte(current_video_memory_address--);
						current_data_2 = space.read_byte(current_video_memory_address--);
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = ((char_rom[current_data_1*16+j]>>k) & 0x01) ? (current_data_2>>4)&0x0f : current_data_2&0x0f;
								for (l=0; l<3; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*3+l) = current_colour;
							}
						}
					}
					break;
				}
				break;
			case 0x03:  /* 66 chars */
				switch (unit_mode)
				{
				case 0:
					current_data_1 = space.read_byte(current_video_memory_address--);
					current_data_2 = space.read_byte(current_video_memory_address--);
					for (i=0; i<66; i++)
					{
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = ((char_rom[current_data_1*16+j]>>k) & 0x01) ? (current_data_2>>4)&0x0f : current_data_2&0x0f;
								for (l=0; l<2; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*2+l) = current_colour;
							}
						}
					}
					break;
				case 1:
					for (i=0; i<66; i++)
					{
						current_data_1 = space.read_byte(current_video_memory_address--);
						current_data_2 = space.read_byte(current_video_memory_address--);
						for (j=0; j<=line_repeat_count; j++)
						{
							for (k=0; k<8; k++)
							{
								current_colour = ((char_rom[current_data_1*16+j]>>k) & 0x01) ? (current_data_2>>4)&0x0f : current_data_2&0x0f;
								for (l=0; l<2; l++)
									bitmap.pix16(current_scan_line/2 + j, (i*8+k)*2+l) = current_colour;
							}
						}
					}
					break;
				}
				break;
			}
			break;
		}
		current_scan_line += line_repeat_count*2+2;
		LOG_DAI_VIDEO_LINE(display_mode, unit_mode, horizontal_resolution, line_repeat_count, current_scan_line);
	}
	return 0;
}

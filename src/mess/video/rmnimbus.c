// license:BSD-3-Clause
// copyright-holders:Phill Harvey-Smith, Carl
/*
    video/rmnimbus.c

    Research machines Nimbus.

    2009-11-14, P.Harvey-Smith.

    This is my best guess implementation of the operation of the Nimbus
    video system.

    On the real machine, the Video chip has a block of 64K of memory which is
    completely separate from the main 80186 memory.

    The main CPU write to the video chip via a series of registers in the
    0x0000 to 0x002F reigon, the video chip then manages all video memory
    from there.

    As I cannot find a datasheet for the vide chip marked
    MB61H201 Fujitsu RML 12835 GCV, I have had to determine most of its
    operation by disassembling the Nimbus bios and by writing experemental
    code on the real machine.
*/

#include "emu.h"
#include "debug/debugcpu.h"
#include "debug/debugcon.h"
#include "includes/rmnimbus.h"


#define WIDTH_MASK      0x07

#define FG_COLOUR       (m_colours&0x0F)
#define BG_COLOUR       ((m_colours&0xF0)>>4)
#define SELECT_COL(x,c) (IS_80COL ? ((((x) & 1) ? ((c) << 2) : (c)) & 0xC) : (c))
#define FILL_WORD(c)    (((c) << 12) | ((c) << 8) | ((c) << 4) | (c))

#define IS_80COL        (m_mode&0x10)
#define IS_XOR          (m_op&8)

#define DEBUG_TEXT  0x01
#define DEBUG_DB    0x02
#define DEBUG_PIXEL 0x04

#define DEBUG_SET(flags)    ((m_debug_video & (flags))==(flags))

static void video_debug(running_machine &machine, int ref, int params, const char *param[]);

READ16_MEMBER(rmnimbus_state::nimbus_video_io_r)
{
	UINT16 result = 0;

	switch (offset)
	{
		case 0x00:
		case 0x08:
			result = read_pixel_data(m_x, m_y);
			break;
		case 0x02:
		case 0x0A:
			result = read_pixel_data(m_x, ++m_y);
			break;
		case 0x05:
		case 0x0D:
			result = read_pixel_data(++m_x, m_y);
			break;

		case 0x10:
			result = m_yline;
			break;
		case 0x11:
			result = m_op;
			break;
		case 0x12:
			result = m_colours;
			break;
		case 0x13:
			result = m_mode;
			break;
		case 0x14:
			result = m_screen->vpos() % 0xb; // TODO: verify
			break;
		case 0x15:
			result = m_x;
			break;
		case 0x16:
			result = m_y;
			break;
		default:
			logerror("nimbus: unknown video reg read %02x\n", offset);
			break;
	}

	if(DEBUG_SET(DEBUG_TEXT))
		logerror("Nimbus video IOR at %05X from %04X mask=%04X, data=%04X\n",space.device().safe_pc(),(offset*2),mem_mask,result);

	return result;
}

UINT8 rmnimbus_state::get_pixel(UINT16 x, UINT16 y)
{
	UINT8   result = 0;

	if((x<640) && (y<250))
	{
		if(IS_80COL)
			result=m_video_mem.pix16(y, x) >> 2;
		else
			result=m_video_mem.pix16(y, x*2);
	}

	return result;
}

UINT16 rmnimbus_state::read_pixel_line(UINT16 x, UINT16 y, UINT8 pixels, UINT8 bpp)
{
	UINT16 colour = 0;
	int i;
	x *= pixels;

	if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
		logerror("read_pixel_line(x=%d, y=%d, width=%d, bpp=%d)\n",x,y,pixels,bpp);

	for(i = 0; i < pixels - 1; i++)
	{
		colour |= get_pixel(i + x, y);

		if(bpp==1)
			colour=((colour==SELECT_COL(x + i, FG_COLOUR)) ? 1 : 0) << 1;
		else
			colour <<= bpp;
	}
	return colour | get_pixel(x + i, y);
}

UINT16 rmnimbus_state::read_pixel_data(UINT16 x, UINT16 y)
{
	UINT16  result=0;

	if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
		logerror("read_pixel_data(x=%d, y=%d), reg022=%04X\n",x,y,m_op);

	if(IS_80COL)
	{
		switch (m_op & WIDTH_MASK)
		{
			case 0x00   : break;

			case 0x01   : break;

			case 0x02   : break;

			case 0x03   : break;

			case 0x04   :
					result=read_pixel_line(x,y,4,2);
					break;

			case 0x05   : break;

			case 0x06   :
					result=read_pixel_line(x,y,8,2);
					break;

			case 0x07   : break;
		}
	}
	else /* 40 Col */
	{
		switch (m_op & WIDTH_MASK)
		{
			case 0x00   : break;

			case 0x01   : break;

			case 0x02   : break;

			case 0x03   : break;

			case 0x04   : break;

			case 0x05   : break;

			case 0x06   :
					result=read_pixel_line(x,y,4,4);
					break;

			case 0x07   : break;
		}
	}

	return result;
}

/*
    Write to the video registers.

    Incase anyone wonders about the DEBUG_DB statement, this allows me to log which registers
    are being written to and then play them back at the real machine, this has helped greatly
    in figuring out what the video registers do.

*/

WRITE16_MEMBER(rmnimbus_state::nimbus_video_io_w)
{
	UINT16 colours = data;
	if(offset < 0x14)
	{
		if(DEBUG_SET(DEBUG_TEXT))
			logerror("Nimbus video IOW at %05X write of %04X to %04X mask=%04X\n",space.device().safe_pc(),data,(offset*2),mem_mask);

		if(DEBUG_SET(DEBUG_DB))
			logerror("dw %05X,%05X\n",(offset*2),data);
	}

	switch (offset)
	{
		case 0x00:
		case 0x08:
			break;

		case 0x09:
			colours = FILL_WORD(FG_COLOUR);
		case 0x01:
			m_x = data;
			break;

		case 0x02:
		case 0x0A:
			m_y++;
			break;

		case 0x0B:
			colours = FILL_WORD(FG_COLOUR);
		case 0x03:
			m_x = data;
			m_y++;
			break;

		case 0x05:
		case 0x0D:
			m_x++;
			break;

		case 0x0E:
			colours = FILL_WORD(FG_COLOUR);
		case 0x06:
			m_y = data;
			break;

		case 0x0F:
			colours = FILL_WORD(FG_COLOUR);
		case 0x07:
			m_y = data;
			m_x++;
			break;

		case 0x10:
			m_yline = data;
			return;
		case 0x11:
			m_op = data;
			return;
		case 0x12:
			m_colours = data;
			return;
		case 0x13:
			/*
			    bits 0..3 of reg026 contain the border colour.
			    bit 5 contains the 40/80 column (320/640 pixel) flag.
			*/
			m_mode = data;
			return;
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
			change_palette(offset - 0x14, data);
			return;

		default:
			logerror("nimbus: unknown video reg write %02x %04x\n", offset, data);
			return;
	}
	if(offset & 0x08)
		write_pixel_data(m_x, m_y, colours);
}

void rmnimbus_state::set_pixel(UINT16 x, UINT16 y, UINT8 colour)
{
	if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
		logerror("set_pixel(x=%d, y=%d, colour=%04X), IS_XOR=%02X\n",x,y,colour,IS_XOR);

	if((x<640) && (y<250))
	{
		if(IS_XOR)
			m_video_mem.pix16(y, x)^=colour;
		else
			m_video_mem.pix16(y, x)=colour;
	}
}

void rmnimbus_state::set_pixel40( UINT16 x, UINT16 y, UINT8 colour)
{
	set_pixel((x*2),y,colour);
	set_pixel((x*2)+1,y,colour);
}

void rmnimbus_state::write_pixel_line(UINT16 x, UINT16 y, UINT16 data, UINT8 pixels, UINT8 bpp)
{
	UINT8 colour;
	UINT8 mask = (1 << bpp) - 1;
	x *= pixels;

	if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
		logerror("write_pixel_line(x=%d, y=%d, data=%04X, width=%d, bpp=%d)\n",x,y,data,pixels,bpp);

	for(int i = (pixels - 1); i >= 0; i--)
	{
		if(bpp==1)
			colour = SELECT_COL(x + i, (data & 1) ? FG_COLOUR : BG_COLOUR);
		else if(IS_80COL)
			colour = (data & mask) << 2;
		else
			colour = (data & mask);

		if(IS_80COL)
			set_pixel(x + i,y,colour);
		else
			set_pixel40(x + i,y,colour);

		data >>= bpp;
	}
}

void rmnimbus_state::move_pixel_line(UINT16 x, UINT16 y, UINT8 pixels)
{
	x *= pixels;
	if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
		logerror("move_pixel_line(x=%d, y=%d, width=%d)\n",x,y,pixels);

	for(int i = 0; i < pixels; i++)
	{
		if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
			logerror("x=%d\n",x + i);
		m_video_mem.pix16(m_yline, x + i) = m_video_mem.pix16(y, x + i);
	}
}



/*
    The values in the bottom 3 bits of reg022 seem to determine the number of bits per pixel
    for following operations.

    The values that I have decoded so far are :

    000 1bpp, foreground and background colours taken from reg024
    001 2bpp, using the first 4 colours of the pallette
    010
    011
    100 4bpp, must be a 16 bit word, of which the upper byte is a mask anded with existing pixels then ored
              with the lower byte containing the pixel data for two pixels.
    101 Move pixel data at x,reg020 to x,y, used for scrolling.
    110 if 40 col
            4bpp, 16 bit word containing the pixel data for 4 pixels.
        else
            2bpp, 16 bit word containing the pixel data for 8 pixels.
    111

    Bit 3 of reg022 is as follows :

    0   pixels are written from supplied colour data
    1   pixels are xor'ed onto the screen
*/

void rmnimbus_state::write_pixel_data(UINT16 x, UINT16 y, UINT16    data)
{
	if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
		logerror("write_pixel_data(x=%d, y=%d, data=%04X), reg022=%04X\n",x,y,data,m_op);

	if(IS_80COL)
	{
		switch (m_op & WIDTH_MASK)
		{
			case 0x00:
				write_pixel_line(x,y,data,16,1);
				break;

			case 0x01:
				write_pixel_line(x,y,data,8,1);
				break;

			case 0x02:
				write_pixel_line(x,y,data,8,1);
				break;

			case 0x03:
				set_pixel(x,y,SELECT_COL(x, FG_COLOUR));
				break;

			case 0x04:
				write_pixel_line(x,y,(((data & 0xFF00)>>8) & (data & 0xFF)) | (~((data & 0xFF00)>>8) & read_pixel_line(x,y,4,2)),4,2);
				break;

			case 0x05:
				move_pixel_line(x,y,16);
				break;

			case 0x06:
				write_pixel_line(x,y,data,8,2);
				break;

			case 0x07:
				set_pixel(x,y,SELECT_COL(x, FG_COLOUR));
				break;
		}
	}
	else /* 40 Col */
	{
		switch (m_op & WIDTH_MASK)
		{
			case 0x00:
				write_pixel_line(x,y,data,8,1);
				break;

			case 0x01:
				write_pixel_line(x,y,data,4,2);
				break;

			case 0x02:
				set_pixel40(x,y,FG_COLOUR);
				break;

			case 0x03:
				set_pixel(x,y,FG_COLOUR);
				break;

			case 0x04:
				write_pixel_line(x,y,(((data & 0xFF00)>>8) & (data & 0xFF)) | (~((data & 0xFF00)>>8) & read_pixel_line(x,y,2,4)),2,4);
				break;

			case 0x05:
				move_pixel_line(x,y,16);
				break;

			case 0x06:
				write_pixel_line(x,y,data,4,4);
				break;

			case 0x07:
				set_pixel(x,y,FG_COLOUR);
				break;
		}
	}
}

void rmnimbus_state::change_palette(UINT8 bank, UINT16 colours)
{
	// loop over changing colours
	for(int colourno = (bank * 4); colourno < ((bank + 1) * 4); colourno++)
	{
		int i = (colours & 8) >> 3;
		m_palette->set_pen_color(colourno, pal2bit((colours & 2) | i), pal2bit(((colours & 4) >> 1) | i), pal2bit(((colours & 1) << 1) | i));

		if(DEBUG_SET(DEBUG_TEXT))
			logerror("set colourno[%02X], colour=%02X\n",colourno, colours & 0xf);
		colours >>= 4;
	}
}

static void video_debug(running_machine &machine, int ref, int params, const char *param[])
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	if(params>0)
	{
		int temp;
		sscanf(param[0],"%d",&temp); state->m_debug_video = temp;
	}
	else
	{
		debug_console_printf(machine,"Error usage : nimbus_vid_debug <debuglevel>\n");
		debug_console_printf(machine,"Current debuglevel=%02X\n",state->m_debug_video);
	}
}

void rmnimbus_state::video_start()
{
	m_debug_video=0;

	m_screen->register_screen_bitmap(m_video_mem);

	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		debug_console_register_command(machine(), "nimbus_vid_debug", CMDFLAG_NONE, 0, 0, 1, video_debug);
	}
}

void rmnimbus_state::video_reset()
{
	m_mode = 0;
	m_x = 0;
	m_y = 0;
	m_op = 0;
	m_yline = 0;
}

UINT32 rmnimbus_state::screen_update_nimbus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_video_mem, 0, 0, 0, 0, cliprect);

	return 0;
}

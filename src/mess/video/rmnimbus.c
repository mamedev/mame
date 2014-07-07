/*
    video/rmnimbus.c

    Research machines Nimbus.

    2009-11-14, P.Harvey-Smith.

    This is my best guess implementation of the operation of the Nimbus
    video system.

    On the real machine, the Video chip has a block of 64K of memory which is
    completely seperate from the main 80186 memory.

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

// Offsets of nimbus video registers within register array

#define reg000          0x00
#define reg002          0x01
#define reg004          0x02
#define reg006          0x03
#define reg008          0x04
#define reg00A          0x05
#define reg00C          0x06
#define reg00E          0x07
#define reg010          0x08
#define reg012          0x09
#define reg014          0x0A
#define reg016          0x0B
#define reg018          0x0C
#define reg01A          0x0D
#define reg01C          0x0E
#define reg01E          0x0F
#define reg020          0x10
#define reg022          0x11
#define reg024          0x12
#define reg026          0x13
#define reg028          0x14
#define reg02A          0x15
#define reg02C          0x16
#define reg02E          0x17

#define FG_COLOUR       (m_vidregs[reg024]&0x0F)
#define BG_COLOUR       ((m_vidregs[reg024]&0xF0)>>4)
#define SELECT_COL(x,c)	(IS_80COL ? ((((x) & 1) ? ((c) << 2) : (c)) & 0xC) : (c))
#define FILL_WORD(c)	(((c) << 12) | ((c) << 8) | ((c) << 4) | (c))

#define IS_80COL        (m_vidregs[reg026]&0x10)
#define IS_XOR          (m_vidregs[reg022]&8)

#define DEBUG_TEXT  0x01
#define DEBUG_DB    0x02
#define DEBUG_PIXEL 0x04

#define DEBUG_SET(flags)    ((m_debug_video & (flags))==(flags))

static void video_debug(running_machine &machine, int ref, int params, const char *param[]);

/*
    I'm not sure which of thes return values on a real machine, so for the time being I'm going
    to return the values for all of them, it doesn't seem to hurt !
*/

READ16_MEMBER(rmnimbus_state::nimbus_video_io_r)
{
	int     pc=space.device().safe_pc();
	UINT16  result;

	switch (offset)
	{
		case    reg000  : result=read_pixel_data(m_vidregs[reg002],m_vidregs[reg00C]); break;
		case    reg002  : result=m_vidregs[reg002]; break;
		case    reg004  : result=read_pixel_data(m_vidregs[reg002],++m_vidregs[reg00C]); break;
		case    reg006  : result=m_vidregs[reg006]; break;
		case    reg008  : result=m_vidregs[reg008]; break;
		case    reg00A  : result=read_pixel_data(++m_vidregs[reg002],m_vidregs[reg00C]); break;
		case    reg00C  : result=m_vidregs[reg00C]; break;
		case    reg00E  : result=m_vidregs[reg00E]; break;

		case    reg010  : result=read_pixel_data(m_vidregs[reg002],m_vidregs[reg00C]); break;
		case    reg012  : result=m_vidregs[reg012]; break;
		case    reg014  : result=m_vidregs[reg014]; break;
		case    reg016  : result=m_vidregs[reg016]; break;
		case    reg018  : result=m_vidregs[reg018]; break;
		case    reg01A  : result=read_pixel_data(++m_vidregs[reg002],m_vidregs[reg00C]); break;
		case    reg01C  : result=m_vidregs[reg01C]; break;
		case    reg01E  : result=m_vidregs[reg01E]; break;

		case    reg020  : result=m_vidregs[reg020]; break;
		case    reg022  : result=m_vidregs[reg022]; break;
		case    reg024  : result=m_vidregs[reg024]; break;
		case    reg026  : result=m_vidregs[reg026]; break;
		case    reg028  : result=m_screen->vpos() % 0xb; break; //result=m_vidregs[reg028]; break;
		case    reg02A  : result=m_vidregs[reg002]; break;
		case    reg02C  : result=m_vidregs[reg00C]; break;
		case    reg02E  : result=m_vidregs[reg02E]; break;
		default         : result=0; break;
	}

	if(DEBUG_SET(DEBUG_TEXT))
		logerror("Nimbus video IOR at %05X from %04X mask=%04X, data=%04X\n",pc,(offset*2),mem_mask,result);

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
		logerror("read_pixel_data(x=%d, y=%d), reg022=%04X\n",x,y,m_vidregs[reg022]);

	if(IS_80COL)
	{
		switch (m_vidregs[reg022] & WIDTH_MASK)
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
		switch (m_vidregs[reg022] & WIDTH_MASK)
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
    Write to the video registers, the default action is to write to the array of registers.
    If a register also needs some special action call the action function for that register.

    Incase anyone wonders about the DEBUG_DB statement, this allows me to log which registers
    are being written to and then play them back at the real machine, this has helped greatly
    in figuring out what the video registers do.

*/

WRITE16_MEMBER(rmnimbus_state::nimbus_video_io_w)
{
	int pc=space.device().safe_pc();

	if(offset<reg028)
	{
		if(DEBUG_SET(DEBUG_TEXT))
			logerror("Nimbus video IOW at %05X write of %04X to %04X mask=%04X\n",pc,data,(offset*2),mem_mask);

		if(DEBUG_SET(DEBUG_DB))
			logerror("dw %05X,%05X\n",(offset*2),data);
	}

	switch (offset)
	{
		case    reg000  : m_vidregs[reg000]=data; break;
		case    reg002  : m_vidregs[reg002]=data; break;
		case    reg004  : m_vidregs[reg004]=data; write_reg_004(); break;
		case    reg006  : m_vidregs[reg006]=data; write_reg_006(); break;
		case    reg008  : m_vidregs[reg008]=data; break;
		case    reg00A  : m_vidregs[reg00A]=data; write_reg_00A(); break;
		case    reg00C  : m_vidregs[reg00C]=data; break;
		case    reg00E  : m_vidregs[reg00E]=data; write_reg_00E(); break;

		case    reg010  : m_vidregs[reg010]=data; write_reg_010(); break;
		case    reg012  : m_vidregs[reg012]=data; write_reg_012(); break;
		case    reg014  : m_vidregs[reg014]=data; write_reg_014(); break;
		case    reg016  : m_vidregs[reg016]=data; write_reg_016(); break;
		case    reg018  : m_vidregs[reg018]=data; break;
		case    reg01A  : m_vidregs[reg01A]=data; write_reg_01A(); break;
		case    reg01C  : m_vidregs[reg01C]=data; write_reg_01C();break;
		case    reg01E  : m_vidregs[reg01E]=data; write_reg_01E();break;

		case    reg020  : m_vidregs[reg020]=data; break;
		case    reg022  : m_vidregs[reg022]=data; break;
		case    reg024  : m_vidregs[reg024]=data; break;
		case    reg026  : m_vidregs[reg026]=data; write_reg_026(); break;
		case    reg028  : change_palette(0,data); break;
		case    reg02A  : change_palette(1,data); break;
		case    reg02C  : change_palette(2,data); break;
		case    reg02E  : change_palette(3,data); break;

		default         : break;
	}
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
		m_video_mem.pix16(m_vidregs[reg020], x + i) = m_video_mem.pix16(y, x + i);
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
		logerror("write_pixel_data(x=%d, y=%d, data=%04X), reg022=%04X\n",x,y,data,m_vidregs[reg022]);

	if(IS_80COL)
	{
		switch (m_vidregs[reg022] & WIDTH_MASK)
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
		switch (m_vidregs[reg022] & WIDTH_MASK)
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

void rmnimbus_state::write_reg_004()
{
	//m_vidregs[reg002]=0;
	m_vidregs[reg00C]++;
}

void rmnimbus_state::write_reg_006()
{
	m_vidregs[reg00C]++;
	m_vidregs[reg002]=m_vidregs[reg006];
}

void rmnimbus_state::write_reg_00A()
{
	m_vidregs[reg002]++;
}

void rmnimbus_state::write_reg_00E()
{
	m_vidregs[reg002]++;
	m_vidregs[reg00C]=m_vidregs[reg00E];
}

void rmnimbus_state::write_reg_010()
{
	write_pixel_data(m_vidregs[reg002],m_vidregs[reg00C],m_vidregs[reg010]);
}

void rmnimbus_state::write_reg_012()
{
	// I dunno if this is actually what is happening as the regs seem to be write only....
	// doing this however does seem to make some programs (worms from the welcom disk)
	// work correctly.
	m_vidregs[reg002]=m_vidregs[reg012];

	write_pixel_data(m_vidregs[reg012],m_vidregs[reg00C],FILL_WORD(FG_COLOUR));
}

void rmnimbus_state::write_reg_014()
{
	write_pixel_data(m_vidregs[reg002],++m_vidregs[reg00C],m_vidregs[reg014]);
}

void rmnimbus_state::write_reg_016()
{
	m_vidregs[reg002]=m_vidregs[reg016];

	write_pixel_data(m_vidregs[reg002],++m_vidregs[reg00C],FILL_WORD(FG_COLOUR));
}


void rmnimbus_state::write_reg_01A()
{
	write_pixel_data(++m_vidregs[reg002],m_vidregs[reg00C],m_vidregs[reg01A]);
}

void rmnimbus_state::write_reg_01C()
{
	// I dunno if this is actually what is happening as the regs seem to be write only....
	// doing this however does seem to make some programs (welcome from the welcom disk,
	// and others using the standard RM box menus) work correctly.
	m_vidregs[reg00C]=m_vidregs[reg01C];

	write_pixel_data(m_vidregs[reg002],m_vidregs[reg01C],FILL_WORD(FG_COLOUR));
}

void rmnimbus_state::write_reg_01E()
{
	m_vidregs[reg00C]=m_vidregs[reg01E];

	write_pixel_data(++m_vidregs[reg002],m_vidregs[reg00C],FILL_WORD(FG_COLOUR));
}

/*
    bits 0..3 of reg026 contain the border colour.
    bit 5 contains the 40/80 column (320/640 pixel) flag.
*/

void rmnimbus_state::write_reg_026()
{
	if(DEBUG_SET(DEBUG_TEXT))
		logerror("reg 026 write, border_colour=%02X\n",m_vidregs[reg026] & 0x0F);
}

void rmnimbus_state::change_palette(UINT8 bank, UINT16 colours)
{
	// loop over changing colours
	for(int colourno = (bank * 4); colourno < ((bank + 1) * 4); colourno++)
	{
		int i = (colours & 8) >> 3;
		m_palette->set_pen_color(colourno, pal2bit((colours & 2) | i), pal2bit(((colours & 4) >> 1) | i), pal2bit(((colours & 1) << 1) | i));

		if(DEBUG_SET(DEBUG_TEXT))
			logerror("set colourno[%02X], colour=%02X\n",colourno, colours);
		colours >>= 4;
	}
}

static void video_debug(running_machine &machine, int ref, int params, const char *param[])
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	if(params>0)
	{
		sscanf(param[0],"%d",&state->m_debug_video);
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

	save_item(NAME(m_vidregs));

	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		debug_console_register_command(machine(), "nimbus_vid_debug", CMDFLAG_NONE, 0, 0, 1, video_debug);
	}
}

void rmnimbus_state::video_reset()
{
	// When we reset clear the video registers and video memory.
	memset(&m_vidregs,0x00,sizeof(m_vidregs));
}

UINT32 rmnimbus_state::screen_update_nimbus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_video_mem, 0, 0, 0, 0, cliprect);

	return 0;
}

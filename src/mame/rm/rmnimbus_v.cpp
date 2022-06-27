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

    2021-09-29, P.Harvey-Smith.

    I now have access to the service manual for the Nimbus, this documents to facilities provided
    by the video chip, which will hopefully allow a much more accurate implementation.
*/

#include "emu.h"
#include "rmnimbus.h"

#include "debugger.h"
#include "debug/debugcon.h"

#include <functional>

/*
    Acording to the service manual the Nimbus should be capable of the following modes :

    320 x 200 4bpp
    640 x 200 2bpp
    400 x 200 4bpp
    800 x 200 2bpp
    320 x 250 4bpp
    640 x 250 2bpp
    400 x 250 4bpp
    800 x 250 2bpp
*/

/*
    From the service manual the registers are defined as follows :

Ports 0x00-0x1E are the registers used to update the display RAM thus :

Addr    m_x     m_y     Update memory on write?
0x00    nop     nop     no
0x02    load    nop     no
0x04    nop     inc     no
0x06    load    inc     no
0x08    nop     nop     no
0x0A    inc     nop     no
0x0C    nop     load    no
0x0E    inc     load    no
0x10    nop     nop     yes
0x12    load    nop     yes
0x14    nop     inc     yes
0x16    load    inc     yes
0x18    nop     nop     yes
0x1A    inc     nop     yes
0x1C    nop     load    yes
0x1E    inc     load    yes

0x20    scroll port, contains 8 bit scroll address

0x22    Update mode control port (up_mode), controls how data is written to display ram.
        see UPMODE_ constants below

0x24h   Intensity port, provides current logical intensities for update operations
        bits 0..3 Foreground
        bits 4..7 Background

0x26    Display mode (m_mode) current display mode and border colour.
        see MODE_ constants below

For READ.
Ports 0x28, 0x2A, 0x2C and 0x2E have different read and write functions :

0x28    Timing / status, all bits active high
        bit 0   line blank
        bit 1   line display
        bit 2   frame blank
        bit 3   frame display

0x2A    X address status, returns current value of X counter (m_x)

0x2C    Y address status, returns current value of Y counter (m_y)

For Write

0x28, 0x2A, 0x2C, 0x2E Colour look up table :

                Logic colour
Port    Bits    Low res     High Res
0x28    0..3    0           0
0x28    4..7    1           0
0x28    8..11   2           0
0x28    12..15  3           0

0x2A    0..3    3           1
0x2A    4..7    5           1
0x2A    8..11   6           1
0x2A    12..15  7           1

0x2C    0..3    8           2
0x2C    4..7    9           2
0x2C    8..11   10          2
0x2C    12..15  11          2

0x2E    0..3    12          3
0x2E    4..7    13          3
0x2E    8..11   14          3
0x2E    12..15  15          3


*/

// In following definitions ports are the WORD offset, the RM manual
// lists them by the byte offset so they are 2* the value

#define P_SCROLL        0x10
#define P_UPDATE_MODE   0x11
#define P_INTENSITY     0x12
#define P_MODE          0x13
#define P_STATUS        0x14
#define P_X_COUNT       0x15
#define P_Y_COUNT       0x16

#define P_COLOUR03      0x14
#define P_COLOUR47      0x15
#define P_COLOUR8B      0x16
#define P_COLOURCF      0x17

// From the service manual, Reg022  update mode constants :
// The first 8 are NON XOR writes
#define UPMODE_40_TEXT      0x00        // 40 character text
#define UPMODE_80_TEXT      0x01        // 80 character text
#define UPMODE_LO_PIXEL     0x02        // Low res pixel
#define UPMODE_HI_PIXEL     0x03        // Hi res pixel
#define UPMODE_ANIMATION    0x04        // Animation (mask + data)
#define UPMODE_SCROLL       0x05        // Scroll mode
#define UPMODE_DIRECT       0x06        // Direct write to video ram
#define UPMODE_ILLEGAL7     0x07

// The second 8 are XOR writes
#define UPMODE_40_TEXT_X    0x08
#define UPMODE_80_TEXT_X    0x09
#define UPMODE_LO_PIXEL_X   0x0A
#define UPMODE_HI_PIXEL_X   0x0B
#define UPMODE_ANIMATION_X  0x0C
#define UPMODE_SCROLL_X     0x0D
#define UPMODE_DIRECT_X     0x0E
#define UPMODE_ILLEGALF     0x0F

#define UP_XOR_MASK         0x08

// port 026, display mode (m_mode)
#define MODE_BORDER         0x0F        // bits 0..3, Border colour number
#define MODE_RESOLUTION     0x10        // bit 4, 0=low res (40 col), high = high res (80 col)
#define MODE_WIDTH          0x20        // bit 5, 0=narrow, 1=wide
#define MODE_HEIGHT         0x40        // bit 6, 0=625 lines, 1=562

#define WIDTH_MASK      0x07

#define FG_COLOUR       (m_colours&0x0F)
#define BG_COLOUR       ((m_colours&0xF0)>>4)
#define SELECT_COL(x,c) (IS_80COL ? ((((x) & 1) ? ((c) << 2) : (c)) & 0xC) : (c))
#define FILL_WORD(c)    (((c) << 12) | ((c) << 8) | ((c) << 4) | (c))

#define IS_80COL        (m_mode & MODE_RESOLUTION)
#define IS_XOR          (m_upmode & UP_XOR_MASK)

#define DEBUG_TEXT  0x01
#define DEBUG_DB    0x02
#define DEBUG_PIXEL 0x04

#define DEBUG_SET(flags)    ((m_debug_video & (flags))==(flags))

uint16_t rmnimbus_state::nimbus_video_io_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t result = 0;

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

		case P_SCROLL:
			result = m_yline;
			break;
		case P_UPDATE_MODE:
			result = m_upmode;
			break;
		case P_INTENSITY:
			result = m_colours;
			break;
		case P_MODE:
			result = m_mode;
			break;
		case P_STATUS:
			result = m_screen->vpos() % 0xb; // TODO: verify
			break;
		case P_X_COUNT:
			result = m_x;
			break;
		case P_Y_COUNT:
			result = m_y;
			break;

		default:
			logerror("nimbus: unknown video reg read %02x\n", offset);
			break;
	}

	if(DEBUG_SET(DEBUG_TEXT))
		logerror("Nimbus video IOR at %05X from %04X mask=%04X, data=%04X\n",m_maincpu->pc(),(offset*2),mem_mask,result);

	return result;
}

uint8_t rmnimbus_state::get_pixel(uint16_t x, uint16_t y)
{
	uint8_t   result = 0;

	if((x<640) && (y<250))
	{
		if(IS_80COL)
			result=m_video_mem.pix(y, x) >> 2;
		else
			result=m_video_mem.pix(y, x*2);
	}

	return result;
}

uint16_t rmnimbus_state::read_pixel_line(uint16_t x, uint16_t y, uint8_t pixels, uint8_t bpp)
{
	uint16_t colour = 0;
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

uint16_t rmnimbus_state::read_pixel_data(uint16_t x, uint16_t y)
{
	uint16_t  result=0;

	if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
		logerror("read_pixel_data(x=%d, y=%d), reg022=%04X\n",x,y,m_upmode);

	if(IS_80COL)
	{
		switch (m_upmode & WIDTH_MASK)
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
		switch (m_upmode & WIDTH_MASK)
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

void rmnimbus_state::nimbus_video_io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t colours = data;
	if(offset < 0x14)
	{
		if(DEBUG_SET(DEBUG_TEXT))
			logerror("Nimbus video IOW at %05X write of %04X to %04X mask=%04X\n",m_maincpu->pc(),data,(offset*2),mem_mask);

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
			[[fallthrough]];
		case 0x01:
			m_x = data;
			break;

		case 0x02:
		case 0x0A:
			m_y++;
			break;

		case 0x0B:
			colours = FILL_WORD(FG_COLOUR);
			[[fallthrough]];
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
			[[fallthrough]];
		case 0x06:
			m_y = data;
			break;

		case 0x0F:
			colours = FILL_WORD(FG_COLOUR);
			[[fallthrough]];
		case 0x07:
			m_y = data;
			m_x++;
			break;

		case P_SCROLL:
			m_yline = data;
			return;
		case P_UPDATE_MODE:
			m_upmode = data;
			return;
		case P_INTENSITY:
			m_colours = data;
			return;
		case P_MODE:
			/*
			    bits 0..3 of reg026 contain the border colour.
			    bit 5 contains the 40/80 column (320/640 pixel) flag.
			*/
			m_mode = data;
			return;
		case P_COLOUR03:
		case P_COLOUR47:
		case P_COLOUR8B:
		case P_COLOURCF:
			change_palette(offset - P_COLOUR03, data);
			return;

// This register doesn't appear to be documented, but is written regually in setpc ibm mode
		case 0x18 :
			break;

		default:
			logerror("nimbus: unknown video reg write %02x %04x\n", offset, data);
			return;
	}
	if(offset & 0x08)
		write_pixel_data(m_x, m_y, colours);
}

void rmnimbus_state::set_pixel(uint16_t x, uint16_t y, uint8_t colour)
{
	if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
		logerror("set_pixel(x=%d, y=%d, colour=%04X), IS_XOR=%02X\n",x,y,colour,IS_XOR);

	if((x<640) && (y<250))
	{
		if(IS_XOR)
			m_video_mem.pix(y, x)^=colour;
		else
			m_video_mem.pix(y, x)=colour;
	}
}

void rmnimbus_state::set_pixel40( uint16_t x, uint16_t y, uint8_t colour)
{
	set_pixel((x*2),y,colour);
	set_pixel((x*2)+1,y,colour);
}

void rmnimbus_state::write_pixel_line(uint16_t x, uint16_t y, uint16_t data, uint8_t pixels, uint8_t bpp)
{
	uint8_t colour;
	uint8_t mask = (1 << bpp) - 1;
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

void rmnimbus_state::move_pixel_line(uint16_t x, uint16_t y, uint8_t pixels)
{
	x *= pixels;
	if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
		logerror("move_pixel_line(x=%d, y=%d, width=%d)\n",x,y,pixels);

	for(int i = 0; i < pixels; i++)
	{
		if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
			logerror("x=%d\n",x + i);
		m_video_mem.pix(m_yline, x + i) = m_video_mem.pix(y, x + i);
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

void rmnimbus_state::write_pixel_data(uint16_t x, uint16_t y, uint16_t    data)
{
	if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
		logerror("write_pixel_data(x=%d, y=%d, data=%04X), reg022=%04X\n",x,y,data,m_upmode);

	if(IS_80COL)
	{
		switch (m_upmode & WIDTH_MASK)
		{
			case UPMODE_40_TEXT:
				write_pixel_line(x,y,data,16,1);
				break;

			case UPMODE_80_TEXT:
				write_pixel_line(x,y,data,8,1);
				break;

			case UPMODE_LO_PIXEL:
				write_pixel_line(x,y,data,8,1);
				break;

			case UPMODE_HI_PIXEL:
				set_pixel(x,y,SELECT_COL(x, FG_COLOUR));
				break;

			case UPMODE_ANIMATION:
				write_pixel_line(x,y,(((data & 0xFF00)>>8) & (data & 0xFF)) | (~((data & 0xFF00)>>8) & read_pixel_line(x,y,4,2)),4,2);
				break;

			case UPMODE_SCROLL:
				move_pixel_line(x,y,16);
				break;

			case UPMODE_DIRECT:
				write_pixel_line(x,y,data,8,2);
				break;

			case UPMODE_ILLEGAL7:
				set_pixel(x,y,SELECT_COL(x, FG_COLOUR));
				break;
		}
	}
	else /* 40 Col */
	{
		switch (m_upmode & WIDTH_MASK)
		{
			case UPMODE_40_TEXT:
				write_pixel_line(x,y,data,8,1);
				break;

			case UPMODE_80_TEXT:
				write_pixel_line(x,y,data,4,2);
				break;

			case UPMODE_LO_PIXEL:
				set_pixel40(x,y,FG_COLOUR);
				break;

			case UPMODE_HI_PIXEL:
				set_pixel(x,y,FG_COLOUR);
				break;

			case UPMODE_ANIMATION:
				write_pixel_line(x,y,(((data & 0xFF00)>>8) & (data & 0xFF)) | (~((data & 0xFF00)>>8) & read_pixel_line(x,y,2,4)),2,4);
				break;

			case UPMODE_SCROLL:
				move_pixel_line(x,y,16);
				break;

			case UPMODE_DIRECT:
				write_pixel_line(x,y,data,4,4);
				break;

			case UPMODE_ILLEGAL7:
				set_pixel(x,y,FG_COLOUR);
				break;
		}
	}
}

// Colours are encoded as follows :
// Each nibble contains a colour encoded as igrb
// so we shift through the specified colours and extract the bits, to set the palette.
//
void rmnimbus_state::change_palette(uint8_t bank, uint16_t colours)
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

void rmnimbus_state::video_debug(const std::vector<std::string> &params)
{
	if (params.size() > 0)
	{
		int temp;
		sscanf(params[0].c_str(), "%d", &temp);
		m_debug_video = temp;
	}
	else
	{
		machine().debugger().console().printf("Error usage : nimbus_vid_debug <debuglevel>\n");
		machine().debugger().console().printf("Current debuglevel=%02X\n", m_debug_video);
	}
}

void rmnimbus_state::video_start()
{
	m_debug_video = 0;

	m_screen->register_screen_bitmap(m_video_mem);

	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		using namespace std::placeholders;
		machine().debugger().console().register_command("nimbus_vid_debug", CMDFLAG_NONE, 0, 1, std::bind(&rmnimbus_state::video_debug, this, _1));
	}
}

void rmnimbus_state::video_reset()
{
	m_mode = 0;
	m_x = 0;
	m_y = 0;
	m_upmode = 0;
	m_yline = 0;
}

uint32_t rmnimbus_state::screen_update_nimbus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_video_mem, 0, 0, 0, 0, cliprect);

	return 0;
}

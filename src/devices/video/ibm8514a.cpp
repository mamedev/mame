// license:BSD-3-Clause
// copyright-holders:Barry Rodewald

#include "emu.h"
#include "ibm8514a.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

enum
{
	IBM8514_IDLE = 0,
	IBM8514_DRAWING_RECT,
	IBM8514_DRAWING_LINE,
	IBM8514_DRAWING_BITBLT,
	IBM8514_DRAWING_PATTERN,
	IBM8514_DRAWING_SSV_1,
	IBM8514_DRAWING_SSV_2,
	MACH8_DRAWING_SCAN
};

#define IBM8514_LINE_LENGTH (m_vga->offset())

DEFINE_DEVICE_TYPE(IBM8514A,   ibm8514a_device,   "ibm8514a",   "IBM 8514/A Video")

ibm8514a_device::ibm8514a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ibm8514a_device(mconfig, IBM8514A, tag, owner, clock)
{
}

ibm8514a_device::ibm8514a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_vga(*this, finder_base::DUMMY_TAG)
{
}

void ibm8514a_device::device_start()
{
	memset(&ibm8514, 0, sizeof(ibm8514));
	ibm8514.read_mask = 0x00000000;
	ibm8514.write_mask = 0xffffffff;
}

void ibm8514a_device::ibm8514_write_fg(uint32_t offset)
{
	offset %= m_vga->vga.svga_intf.vram_size;
	uint8_t dst = m_vga->mem_linear_r(offset);
	uint8_t src = 0;

	// check clipping rectangle
	if((ibm8514.current_cmd & 0xe000) == 0xc000)  // BitBLT writes to the destination X/Y, so check that instead
	{
		if(ibm8514.dest_x < ibm8514.scissors_left || ibm8514.dest_x > ibm8514.scissors_right || ibm8514.dest_y < ibm8514.scissors_top || ibm8514.dest_y > ibm8514.scissors_bottom)
			return;  // do nothing
	}
	else
	{
		if(ibm8514.curr_x < ibm8514.scissors_left || ibm8514.curr_x > ibm8514.scissors_right || ibm8514.curr_y < ibm8514.scissors_top || ibm8514.curr_y > ibm8514.scissors_bottom)
			return;  // do nothing
	}

	// determine source
	switch(ibm8514.fgmix & 0x0060)
	{
	case 0x0000:
		src = ibm8514.bgcolour;
		break;
	case 0x0020:
		src = ibm8514.fgcolour;
		break;
	case 0x0040:
	{
		// Windows 95 in svga 8bpp mode wants this (start logo, moving icons around, games etc.)
		u32 shift_values[4] = { 0, 8, 16, 24 };
		src = (ibm8514.pixel_xfer >> shift_values[(ibm8514.curr_x - ibm8514.prev_x) & 3]) & 0xff;
		break;
	}
	case 0x0060:
		// video memory - presume the memory is sourced from the current X/Y co-ords
		src = m_vga->mem_linear_r(((ibm8514.curr_y * IBM8514_LINE_LENGTH) + ibm8514.curr_x));
		break;
	}

	// write the data
	switch(ibm8514.fgmix & 0x000f)
	{
	case 0x0000:
		m_vga->mem_linear_w(offset,~dst);
		break;
	case 0x0001:
		m_vga->mem_linear_w(offset,0x00);
		break;
	case 0x0002:
		m_vga->mem_linear_w(offset,0xff);
		break;
	case 0x0003:
		m_vga->mem_linear_w(offset,dst);
		break;
	case 0x0004:
		m_vga->mem_linear_w(offset,~src);
		break;
	case 0x0005:
		m_vga->mem_linear_w(offset,src ^ dst);
		break;
	case 0x0006:
		m_vga->mem_linear_w(offset,~(src ^ dst));
		break;
	case 0x0007:
		m_vga->mem_linear_w(offset,src);
		break;
	case 0x0008:
		m_vga->mem_linear_w(offset,~(src & dst));
		break;
	case 0x0009:
		m_vga->mem_linear_w(offset,(~src) | dst);
		break;
	case 0x000a:
		m_vga->mem_linear_w(offset,src | (~dst));
		break;
	case 0x000b:
		m_vga->mem_linear_w(offset,src | dst);
		break;
	case 0x000c:
		m_vga->mem_linear_w(offset,src & dst);
		break;
	case 0x000d:
		m_vga->mem_linear_w(offset,src & (~dst));
		break;
	case 0x000e:
		m_vga->mem_linear_w(offset,(~src) & dst);
		break;
	case 0x000f:
		m_vga->mem_linear_w(offset,~(src | dst));
		break;
	}
}

void ibm8514a_device::ibm8514_write_bg(uint32_t offset)
{
	offset %= m_vga->vga.svga_intf.vram_size;
	uint8_t dst = m_vga->mem_linear_r(offset);
	uint8_t src = 0;

	// check clipping rectangle
	if((ibm8514.current_cmd & 0xe000) == 0xc000)  // BitBLT writes to the destination X/Y, so check that instead
	{
		if(ibm8514.dest_x < ibm8514.scissors_left || ibm8514.dest_x > ibm8514.scissors_right || ibm8514.dest_y < ibm8514.scissors_top || ibm8514.dest_y > ibm8514.scissors_bottom)
			return;  // do nothing
	}
	else
		if(ibm8514.curr_x < ibm8514.scissors_left || ibm8514.curr_x > ibm8514.scissors_right || ibm8514.curr_y < ibm8514.scissors_top || ibm8514.curr_y > ibm8514.scissors_bottom)
			return;  // do nothing

	// determine source
	switch(ibm8514.bgmix & 0x0060)
	{
	case 0x0000:
		src = ibm8514.bgcolour;
		break;
	case 0x0020:
		src = ibm8514.fgcolour;
		break;
	case 0x0040:
		src = ibm8514.pixel_xfer;
		break;
	case 0x0060:
		// video memory - presume the memory is sourced from the current X/Y co-ords
		src = m_vga->mem_linear_r(((ibm8514.curr_y * IBM8514_LINE_LENGTH) + ibm8514.curr_x));
		break;
	}

	// write the data
	switch(ibm8514.bgmix & 0x000f)
	{
	case 0x0000:
		m_vga->mem_linear_w(offset,~dst);
		break;
	case 0x0001:
		m_vga->mem_linear_w(offset,0x00);
		break;
	case 0x0002:
		m_vga->mem_linear_w(offset,0xff);
		break;
	case 0x0003:
		m_vga->mem_linear_w(offset,dst);
		break;
	case 0x0004:
		m_vga->mem_linear_w(offset,~src);
		break;
	case 0x0005:
		m_vga->mem_linear_w(offset,src ^ dst);
		break;
	case 0x0006:
		m_vga->mem_linear_w(offset,~(src ^ dst));
		break;
	case 0x0007:
		m_vga->mem_linear_w(offset,src);
		break;
	case 0x0008:
		m_vga->mem_linear_w(offset,~(src & dst));
		break;
	case 0x0009:
		m_vga->mem_linear_w(offset,(~src) | dst);
		break;
	case 0x000a:
		m_vga->mem_linear_w(offset,src | (~dst));
		break;
	case 0x000b:
		m_vga->mem_linear_w(offset,src | dst);
		break;
	case 0x000c:
		m_vga->mem_linear_w(offset,src & dst);
		break;
	case 0x000d:
		m_vga->mem_linear_w(offset,src & (~dst));
		break;
	case 0x000e:
		m_vga->mem_linear_w(offset,(~src) & dst);
		break;
	case 0x000f:
		m_vga->mem_linear_w(offset,~(src | dst));
		break;
	}
}

void ibm8514a_device::ibm8514_write(uint32_t offset, uint32_t src)
{
	int data_size = 8;
	uint32_t xfer;

	switch(ibm8514.pixel_control & 0x00c0)
	{
	case 0x0000:  // Foreground Mix only
		ibm8514_write_fg(offset);
		break;
	case 0x0040:  // fixed pattern (?)
		// TODO
		break;
	case 0x0080:  // use pixel transfer register
		if(ibm8514.bus_size == 0)  // 8-bit
			data_size = 8;
		if(ibm8514.bus_size == 1)  // 16-bit
			data_size = 16;
		if(ibm8514.bus_size == 2)  // 32-bit
			data_size = 32;
		if((ibm8514.current_cmd & 0x1000) && (data_size != 8))
		{
			xfer = ((ibm8514.pixel_xfer & 0x000000ff) << 8) | ((ibm8514.pixel_xfer & 0x0000ff00) >> 8)
					| ((ibm8514.pixel_xfer & 0x00ff0000) << 8) | ((ibm8514.pixel_xfer & 0xff000000) >> 8);
		}
		else
			xfer = ibm8514.pixel_xfer;
		if(ibm8514.current_cmd & 0x0002)
		{
			if((xfer & ((1<<(data_size-1))>>ibm8514.src_x)) != 0)
				ibm8514_write_fg(offset);
			else
				ibm8514_write_bg(offset);
		}
		else
		{
			ibm8514_write_fg(offset);
		}
		ibm8514.src_x++;
		if(ibm8514.src_x >= data_size)
			ibm8514.src_x = 0;
		break;
	case 0x00c0:  // use source plane
		if (m_vga->mem_linear_r(src) != 0x00)
			ibm8514_write_fg(offset);
		else
			ibm8514_write_bg(offset);
		break;
	}
}

/*
92E8h W(R/W):  Line Error Term Read/Write Register (ERR_TERM).
bit  0-12  (911/924) LINE PARAMETER/ERROR TERM. For Line Drawing this is the
            Bresenham Initial Error Term 2*dminor-dmajor (one less if the
            starting X is less than the ending X) in two's complement format.
            (dminor is the length of the line projected onto the minor or
            dependent axis, dmajor is the length of the line projected onto
            the major or independent axis).
     0-13  (80x +) LINE PARAMETER/ERROR TERM. See above.
 */
uint16_t ibm8514a_device::ibm8514_line_error_r()
{
	return ibm8514.line_errorterm;
}

void ibm8514a_device::ibm8514_line_error_w(uint16_t data)
{
	ibm8514.line_errorterm = data;
	LOG("8514/A: Line Parameter/Error Term write %04x\n", data);
}

/*
  9AE8h W(R):  Graphics Processor Status Register (GP_STAT)
bit   0-7  Queue State.
             00h = 8 words available - queue is empty
             01h = 7 words available
             03h = 6 words available
             07h = 5 words available
             0Fh = 4 words available
             1Fh = 3 words available
             3Fh = 2 words available
             7Fh = 1 word  available
             FFh = 0 words available - queue is full
        8  (911-928) DTA AVA. Read Data Available. If set data is ready to be
            read from the PIX_TRANS register (E2E8h).
        9  HDW BSY. Hardware Graphics Processor Busy
           If set the Graphics Processor is busy.
       10  (928 +) AE. All FIFO Slots Empty. If set all FIFO slots are empty.
    11-15  (864/964) (R) Queue State bits 8-12. 1Fh if 8 words or less
            available, Fh for 9 words, 7 for 10 words, 3 for 11 words, 1 for
            12 words and 0 for 13 words available.
 */
uint16_t ibm8514a_device::ibm8514_gpstatus_r()
{
	uint16_t ret = 0x0000;

	//LOG("S3: 9AE8 read\n");
	if(ibm8514.gpbusy == true)
		ret |= 0x0200;
	if(ibm8514.data_avail == true)
		ret |= 0x0100;
	return ret;
}

void ibm8514a_device::ibm8514_draw_vector(uint8_t len, uint8_t dir, bool draw)
{
	uint32_t offset;
	int x = 0;

	while(x <= len)
	{
		offset = (ibm8514.curr_y * IBM8514_LINE_LENGTH) + ibm8514.curr_x;
		if(draw)
			ibm8514_write(offset,offset);
		switch(dir)
		{
		case 0:  // 0 degrees
			ibm8514.curr_x++;
			break;
		case 1:  // 45 degrees
			ibm8514.curr_x++;
			ibm8514.curr_y--;
			break;
		case 2:  // 90 degrees
			ibm8514.curr_y--;
			break;
		case 3:  // 135 degrees
			ibm8514.curr_y--;
			ibm8514.curr_x--;
			break;
		case 4:  // 180 degrees
			ibm8514.curr_x--;
			break;
		case 5:  // 225 degrees
			ibm8514.curr_x--;
			ibm8514.curr_y++;
			break;
		case 6:  // 270 degrees
			ibm8514.curr_y++;
			break;
		case 7:  // 315 degrees
			ibm8514.curr_y++;
			ibm8514.curr_x++;
			break;
		}
		x++;
	}
}

/*
9AE8h W(W):  Drawing Command Register (CMD)
bit     0  (911-928) ~RD/WT. Read/Write Data. If set VRAM write operations are
            enabled. If clear operations execute normally but writes are
            disabled.
        1  PX MD. Pixel Mode. Defines the orientation of the display bitmap.
             0 = Through plane mode (Single pixel transferred at a time)
             1 = Across plane mode (Multiple pixels transferred at a time).
        2  LAST PXOF. Last Pixel Off. If set the last pixel of a line command
           (CMD_LINE, SSV or LINEAF) is not drawn. This is used for mixes such
           as XOR where drawing the same pixel twice would give the wrong
           color.
        3  DIR TYP. Direction Type.
             0: Bresenham line drawing (X-Y Axial)
                  CMD_LINE draws a line using the Bresenham algorithm as
                  specified in the DESTY_AXSTP (8AE8h), DESTX_DIASTP (8EE8h),
                  ERR_TERM (92E8h) and MAJ_AXIS_PCNT (96E8h) registers
                  INC_X, INC_Y and YMAJAXIS determines the direction.
             1: Vector line draws (Radial).
                  CMD_NOP allows drawing of Short Stroke Vectors (SSVs) by
                  writing to the Short Stroke register (9EE8h).
                  CMD_LINE draws a vector of length MAJ_AXIS_PCNT (96E8h)
                  in the direction specified by LINEDIR (bits 5-7).
                  DRWG-DIR determines the direction of the line.
        4  DRAW YES. If clear the current position is moved, but no pixels
           are modified. This bit should be set when attempting read or
           write of bitmap data.
      5-7  DRWG-DIR. Drawing Direction. When a line draw command (CMD_LINE)
           with DIR TYP=1 (Radial) is issued, these bits define the direction
           of the line counter clockwise relative to the positive X-axis.
             0 = 000 degrees
             1 = 045 degrees
             2 = 090 degrees
             3 = 135 degrees
             4 = 180 degrees
             5 = 225 degrees
             6 = 270 degrees
             7 = 315 degrees
        5  INC_X. This bit together with INC_Y determines which quadrant
           the slope of a line lies within. They also determine the
           orientation of rectangle draw commands.
           If set lines are drawn in the positive X direction (left to right).
        6  YMAJAXIS. For Bresenham line drawing commands this bit determines
           which axis is the independent or major axis. INC_X and INC_Y
           determines which quadrant the slope falls within. This bit further
           defines the slope to within an octant.
           If set Y is the major (independent) axis.
        7  INC_Y. This bit together with INC_X determines which quadrant
           the slope of a line lies within. They also determine the
           orientation of rectangle draw commands.
           If set lines are drawn in the positive Y direction (down).
        8  WAIT YES. If set the drawing engine waits for read/write of the
           PIX_TRANS register (E2E8h) for each pixel during a draw operation.
        9  (911-928) BUS SIZE. If set the PIX_TRANS register (E2E8h) is
            processed internally as two bytes in the order specified by BYTE
            SWAP. If clear all accesses to E2E8h are 8bit.
     9-10  (864,964) BUS SIZE. Select System Bus Size. Controls the width of
            the Pixel Data Transfer registers (E2E8h,E2EAh) and the memory
            mapped I/O. 0: 8bit, 1: 16bit, 2: 32bit
       12  BYTE SWAP. Affects both reads and writes of SHORT_STROKE (9EE8h)
           and PIX_TRANS (E2E8h) when 16bit=1.
           If set take low byte first, if clear take high byte first.
    13-15  Draw Command:
            0 = NOP. Used for Short Stroke Vectors.
            1 = Draw Line. If bit 3 is set the line is drawn to the angle in
                bits 5-7 and the length in the Major Axis Pixel Count register
                (96E8h), if clear the line is drawn from the Bresenham
                constants in the Axial Step Constant register(8AE8h), Diagonal
                Step Constant register (8EE8h), Line Error Term register
               (92E8h) and bits 5-7 of this register.
            2 = Rectangle Fill. The Current X (86E8h) and Y (82E8h)
                registers holds the coordinates of the rectangle to fill and
                the Major Axis Pixel Count register (96E8h) holds the
                horizontal width (in pixels) fill and the Minor Axis Pixel
                Count register (BEE8h index 0) holds the height of the
                rectangle.
            6 = BitBLT. Copies the source rectangle specified by the Current X
                (86E8h) and Y (8AE8h) registers to the destination rectangle,
                specified as for the Rectangle Fills.
            7 = (80x +) Pattern Fill. The source rectangle is an 8x8 pattern
                rectangle, which is copied repeatably to the destination
                rectangle.
 */
void ibm8514a_device::ibm8514_cmd_w(uint16_t data)
{
	int x,y;
	int pattern_x,pattern_y;
	uint32_t off,src;
	uint8_t readmask;

	ibm8514.current_cmd = data;
	ibm8514.src_x = 0;
	ibm8514.src_y = 0;
	ibm8514.bus_size = (data & 0x0600) >> 9;
	switch(data & 0xe000)
	{
	case 0x0000:  // NOP (for "Short Stroke Vectors")
		ibm8514.state = IBM8514_IDLE;
		ibm8514.gpbusy = false;
		LOG("8514/A: Command (%04x) - NOP (Short Stroke Vector)\n", ibm8514.current_cmd);
		break;
	case 0x2000:  // Line
		ibm8514.state = IBM8514_IDLE;
		ibm8514.gpbusy = false;
		if(data & 0x0008)
		{
			if(data & 0x0100)
			{
				ibm8514.state = IBM8514_DRAWING_LINE;
				ibm8514.data_avail = true;
				LOG("8514/A: Command (%04x) - Vector Line (WAIT) %i,%i \n", ibm8514.current_cmd, ibm8514.curr_x, ibm8514.curr_y);
			}
			else
			{
				ibm8514_draw_vector(ibm8514.rect_width,(data & 0x00e0) >> 5,(data & 0010) ? true : false);
				LOG("8514/A: Command (%04x) - Vector Line - %i,%i \n", ibm8514.current_cmd, ibm8514.curr_x, ibm8514.curr_y);
			}
		}
		else
		{
			// Not perfect, but will do for now.
			int16_t dx = ibm8514.rect_width;
			int16_t dy = ibm8514.line_axial_step >> 1;
			int16_t err = ibm8514.line_errorterm;
			int sx = (data & 0x0020) ? 1 : -1;
			int sy = (data & 0x0080) ? 1 : -1;
			int count = 0;
			int16_t temp;

			LOG("8514/A: Command (%04x) - Line (Bresenham) - %i,%i  Axial %i, Diagonal %i, Error %i, Major Axis %i, Minor Axis %i\n",
					ibm8514.current_cmd, ibm8514.curr_x, ibm8514.curr_y, ibm8514.line_axial_step, ibm8514.line_diagonal_step, ibm8514.line_errorterm, ibm8514.rect_width, ibm8514.rect_height);

			if((data & 0x0040))
			{
				temp = dx; dx = dy; dy = temp;
			}
			for(;;)
			{
				ibm8514_write(ibm8514.curr_x + (ibm8514.curr_y * IBM8514_LINE_LENGTH),ibm8514.curr_x + (ibm8514.curr_y * IBM8514_LINE_LENGTH));
				if (count > ibm8514.rect_width) break;
				count++;
				if((err*2) > -dy)
				{
					err -= dy;
					ibm8514.curr_x += sx;
				}
				if((err*2) < dx)
				{
					err += dx;
					ibm8514.curr_y += sy;
				}
			}
		}
		break;
	case 0x4000:  // Rectangle Fill
		if(data & 0x0100)  // WAIT (for read/write of PIXEL TRANSFER (E2E8))
		{
			ibm8514.state = IBM8514_DRAWING_RECT;
			//ibm8514.gpbusy = true;  // DirectX 5 keeps waiting for the busy bit to be clear...
			ibm8514.bus_size = (data & 0x0600) >> 9;
			ibm8514.data_avail = true;
			LOG("8514/A: Command (%04x) - Rectangle Fill (WAIT) %i,%i Width: %i Height: %i Colour: %08x\n",
					ibm8514.current_cmd, ibm8514.curr_x, ibm8514.curr_y, ibm8514.rect_width, ibm8514.rect_height, ibm8514.fgcolour);
			break;
		}
		LOG("8514/A: Command (%04x) - Rectangle Fill %i,%i Width: %i Height: %i Colour: %08x\n",
				ibm8514.current_cmd, ibm8514.curr_x, ibm8514.curr_y, ibm8514.rect_width, ibm8514.rect_height, ibm8514.fgcolour);
		off = 0;
		off += (IBM8514_LINE_LENGTH * ibm8514.curr_y);
		off += ibm8514.curr_x;
		for(y=0;y<=ibm8514.rect_height;y++)
		{
			for(x=0;x<=ibm8514.rect_width;x++)
			{
				if(data & 0x0020)  // source pattern is always based on current X/Y?
					ibm8514_write((off+x) % m_vga->vga.svga_intf.vram_size,(off+x) % m_vga->vga.svga_intf.vram_size);
				else
					ibm8514_write((off-x) % m_vga->vga.svga_intf.vram_size,(off-x) % m_vga->vga.svga_intf.vram_size);
				if(ibm8514.current_cmd & 0x0020)
				{
					ibm8514.curr_x++;
					if(ibm8514.curr_x > ibm8514.prev_x + ibm8514.rect_width)
					{
						ibm8514.curr_x = ibm8514.prev_x;
						ibm8514.src_x = 0;
						if(ibm8514.current_cmd & 0x0080)
							ibm8514.curr_y++;
						else
							ibm8514.curr_y--;
					}
				}
				else
				{
					ibm8514.curr_x--;
					if(ibm8514.curr_x < ibm8514.prev_x - ibm8514.rect_width)
					{
						ibm8514.curr_x = ibm8514.prev_x;
						ibm8514.src_x = 0;
						if(ibm8514.current_cmd & 0x0080)
							ibm8514.curr_y++;
						else
							ibm8514.curr_y--;
					}
				}
			}
			if(data & 0x0080)
				off += IBM8514_LINE_LENGTH;
			else
				off -= IBM8514_LINE_LENGTH;
		}
		ibm8514.state = IBM8514_IDLE;
		ibm8514.gpbusy = false;
		break;
	case 0xc000:  // BitBLT
		// TODO: a10cuba sets up blantantly invalid parameters here, CPU core bug maybe?
		LOG("8514/A: Command (%04x) - BitBLT from %i,%i to %i,%i  Width: %i  Height: %i\n",
				ibm8514.current_cmd, ibm8514.curr_x, ibm8514.curr_y, ibm8514.dest_x, ibm8514.dest_y, ibm8514.rect_width, ibm8514.rect_height);
		off = 0;
		off += (IBM8514_LINE_LENGTH * ibm8514.dest_y);
		off += ibm8514.dest_x;
		src = 0;
		src += (IBM8514_LINE_LENGTH * ibm8514.curr_y);
		src += ibm8514.curr_x;
		readmask = ((ibm8514.read_mask & 0x01) << 7) | ((ibm8514.read_mask & 0xfe) >> 1);
		for(y=0;y<=ibm8514.rect_height;y++)
		{
			for(x=0;x<=ibm8514.rect_width;x++)
			{
				if((ibm8514.pixel_control & 0xc0) == 0xc0)
				{
					// only check read mask if Mix Select is set to 11 (VRAM determines mix)
					if(m_vga->mem_linear_r((src+x)) & ~readmask)
					{
						// presumably every program is going to be smart enough to set the FG mix to use VRAM (0x6x)
						if(data & 0x0020)
							ibm8514_write(off+x,src+x);
						else
							ibm8514_write(off-x,src-x);
					}
				}
				else
				{
					// presumably every program is going to be smart enough to set the FG mix to use VRAM (0x6x)
					if(data & 0x0020)
						ibm8514_write(off+x,src+x);
					else
						ibm8514_write(off-x,src-x);
				}
				if(ibm8514.current_cmd & 0x0020)
				{
					ibm8514.curr_x++;
					if(ibm8514.curr_x > ibm8514.prev_x + ibm8514.rect_width)
					{
						ibm8514.curr_x = ibm8514.prev_x;
						ibm8514.src_x = 0;
						if(ibm8514.current_cmd & 0x0080)
							ibm8514.curr_y++;
						else
							ibm8514.curr_y--;
					}
				}
				else
				{
					ibm8514.curr_x--;
					if(ibm8514.curr_x < ibm8514.prev_x - ibm8514.rect_width)
					{
						ibm8514.curr_x = ibm8514.prev_x;
						ibm8514.src_x = 0;
						if(ibm8514.current_cmd & 0x0080)
							ibm8514.curr_y++;
						else
							ibm8514.curr_y--;
					}
				}
			}
			if(data & 0x0080)
			{
				src += IBM8514_LINE_LENGTH;
				off += IBM8514_LINE_LENGTH;
			}
			else
			{
				src -= IBM8514_LINE_LENGTH;
				off -= IBM8514_LINE_LENGTH;
			}
		}
		ibm8514.state = IBM8514_IDLE;
		ibm8514.gpbusy = false;
		ibm8514.curr_x = ibm8514.prev_x;
		ibm8514.curr_y = ibm8514.prev_y;
		break;
	case 0xe000:  // Pattern Fill
		LOG("8514/A: Command (%04x) - Pattern Fill - source %i,%i  dest %i,%i  Width: %i Height: %i\n",
				ibm8514.current_cmd, ibm8514.curr_x, ibm8514.curr_y, ibm8514.dest_x, ibm8514.dest_y, ibm8514.rect_width, ibm8514.rect_height);
		off = 0;
		off += (IBM8514_LINE_LENGTH * ibm8514.dest_y);
		off += ibm8514.dest_x;
		src = 0;
		src += (IBM8514_LINE_LENGTH * ibm8514.curr_y);
		src += ibm8514.curr_x;
		if(data & 0x0020)
			pattern_x = 0;
		else
			pattern_x = 7;
		if(data & 0x0080)
			pattern_y = 0;
		else
			pattern_y = 7;

		for(y=0;y<=ibm8514.rect_height;y++)
		{
			for(x=0;x<=ibm8514.rect_width;x++)
			{
				if(data & 0x0020)
				{
					ibm8514_write(off+x,src+pattern_x);
					pattern_x++;
					if(pattern_x >= 8)
						pattern_x = 0;
				}
				else
				{
					ibm8514_write(off-x,src-pattern_x);
					pattern_x--;
					if(pattern_x < 0)
						pattern_x = 7;
				}
			}

			// for now, presume that INC_X and INC_Y affect both src and dest, at is would for a bitblt.
			if(data & 0x0020)
				pattern_x = 0;
			else
				pattern_x = 7;
			if(data & 0x0080)
			{
				pattern_y++;
				src += IBM8514_LINE_LENGTH;
				if(pattern_y >= 8)
				{
					pattern_y = 0;
					src -= (IBM8514_LINE_LENGTH * 8);  // move src pointer back to top of pattern
				}
				off += IBM8514_LINE_LENGTH;
			}
			else
			{
				pattern_y--;
				src -= IBM8514_LINE_LENGTH;
				if(pattern_y < 0)
				{
					pattern_y = 7;
					src += (IBM8514_LINE_LENGTH * 8);  // move src pointer back to bottom of pattern
				}
				off -= IBM8514_LINE_LENGTH;
			}
		}
		ibm8514.state = IBM8514_IDLE;
		ibm8514.gpbusy = false;
		break;
	default:
		ibm8514.state = IBM8514_IDLE;
		ibm8514.gpbusy = false;
		LOG("8514/A: Unknown command: %04x\n", data);
		break;
	}
}

/*
8AE8h W(R/W):  Destination Y Position & Axial Step Constant Register
               (DESTY_AXSTP)
bit  0-11  DESTINATION Y-POSITION. During BITBLT operations this is the Y
           co-ordinate of the destination in pixels.
     0-12  (911/924) LINE PARAMETER AXIAL STEP CONSTANT. During Line Drawing,
            this is the Bresenham constant 2*dminor in two's complement
            format. (dminor is the length of the line projected onto the minor
            or dependent axis).
     0-13  (80 x+) LINE PARAMETER AXIAL STEP CONSTANT. Se above

 */
uint16_t ibm8514a_device::ibm8514_desty_r()
{
	return ibm8514.line_axial_step;
}

void ibm8514a_device::ibm8514_desty_w(uint16_t data)
{
	ibm8514.line_axial_step = data;
	ibm8514.dest_y = data;
	LOG("8514/A: Line Axial Step / Destination Y write %04x\n", data);
}

/*
8EE8h W(R/W):  Destination X Position & Diagonal Step Constant Register
               (DESTX_DISTP)
bit  0-11  DESTINATION X-POSITION. During BITBLT operations this is the X
           co-ordinate of the destination in pixels.
     0-12  (911/924) LINE PARAMETER DIAGONAL STEP CONSTANT. During Line
            Drawing this is the Bresenham constant 2*dminor-2*dmajor in two's
            complement format. (dminor is the length of the line projected
            onto the minor or dependent axis, dmajor is the length of the line
            projected onto the major or independent axis)
     0-13  (80x +) LINE PARAMETER DIAGONAL STEP CONSTANT. Se above

 */
uint16_t ibm8514a_device::ibm8514_destx_r()
{
	return ibm8514.line_diagonal_step;
}

void ibm8514a_device::ibm8514_destx_w(uint16_t data)
{
	ibm8514.line_diagonal_step = data;
	ibm8514.dest_x = data;
	LOG("8514/A: Line Diagonal Step / Destination X write %04x\n", data);
}

/*
9EE8h W(R/W):  Short Stroke Vector Transfer Register (SHORT_STROKE)
bit   0-3  Length of vector projected onto the major axis.
           This is also the number of pixels drawn.
        4  Must be set for pixels to be written.
      5-7  VECDIR. The angle measured counter-clockwise from horizontal
           right) at which the line is drawn,
             0 = 000 degrees
             1 = 045 degrees
             2 = 090 degrees
             3 = 135 degrees
             4 = 180 degrees
             5 = 225 degrees
             6 = 270 degrees
             7 = 315 degrees
     8-15  The lower 8 bits are duplicated in the upper 8 bits so two
           short stroke vectors can be drawn with one command.
Note: The upper byte must be written for the SSV command to be executed.
      Thus if a byte is written to 9EE8h another byte must be written to
      9EE9h before execution starts. A single 16bit write will do.
      If only one SSV is desired the other byte can be set to 0.
 */
void ibm8514a_device::ibm8514_wait_draw_ssv()
{
	uint8_t len = ibm8514.wait_vector_len;
	uint8_t dir = ibm8514.wait_vector_dir;
	bool draw = ibm8514.wait_vector_draw;
	uint8_t count = ibm8514.wait_vector_count;
	uint32_t offset;
	int x;
	int data_size;

	switch(ibm8514.bus_size)
	{
	case 0:
		data_size = 8;
		break;
	case 1:
		data_size = 16;
		break;
	case 2:
		data_size = 32;
		break;
	default:
		data_size = 8;
		break;
	}

	for(x=0;x<data_size;x++)
	{
		if(len > count)
		{
			if(ibm8514.state == IBM8514_DRAWING_SSV_1)
			{
				ibm8514.state = IBM8514_DRAWING_SSV_2;
				ibm8514.wait_vector_len = (ibm8514.ssv & 0x0f00) >> 8;
				ibm8514.wait_vector_dir = (ibm8514.ssv & 0xe000) >> 13;
				ibm8514.wait_vector_draw = (ibm8514.ssv & 0x1000) ? true : false;
				ibm8514.wait_vector_count = 0;
				return;
			}
			else if(ibm8514.state == IBM8514_DRAWING_SSV_2)
			{
				ibm8514.state = IBM8514_IDLE;
				ibm8514.gpbusy = false;
				ibm8514.data_avail = false;
				return;
			}
		}

		if(ibm8514.state == IBM8514_DRAWING_SSV_1 || ibm8514.state == IBM8514_DRAWING_SSV_2)
		{
			offset = (ibm8514.curr_y * IBM8514_LINE_LENGTH) + ibm8514.curr_x;
			if(draw)
				ibm8514_write(offset,offset);
			switch(dir)
			{
			case 0:  // 0 degrees
				ibm8514.curr_x++;
				break;
			case 1:  // 45 degrees
				ibm8514.curr_x++;
				ibm8514.curr_y--;
				break;
			case 2:  // 90 degrees
				ibm8514.curr_y--;
				break;
			case 3:  // 135 degrees
				ibm8514.curr_y--;
				ibm8514.curr_x--;
				break;
			case 4:  // 180 degrees
				ibm8514.curr_x--;
				break;
			case 5:  // 225 degrees
				ibm8514.curr_x--;
				ibm8514.curr_y++;
				break;
			case 6:  // 270 degrees
				ibm8514.curr_y++;
				break;
			case 7:  // 315 degrees
				ibm8514.curr_y++;
				ibm8514.curr_x++;
				break;
			}
		}
	}
}

void ibm8514a_device::ibm8514_draw_ssv(uint8_t data)
{
	uint8_t len = data & 0x0f;
	uint8_t dir = (data & 0xe0) >> 5;
	bool draw = (data & 0x10) ? true : false;

	ibm8514_draw_vector(len,dir,draw);
}

uint16_t ibm8514a_device::ibm8514_ssv_r()
{
	return ibm8514.ssv;
}

void ibm8514a_device::ibm8514_ssv_w(uint16_t data)
{
	ibm8514.ssv = data;

	if(ibm8514.current_cmd & 0x0100)
	{
		ibm8514.state = IBM8514_DRAWING_SSV_1;
		ibm8514.data_avail = true;
		ibm8514.wait_vector_len = ibm8514.ssv & 0x0f;
		ibm8514.wait_vector_dir = (ibm8514.ssv & 0xe0) >> 5;
		ibm8514.wait_vector_draw = (ibm8514.ssv & 0x10) ? true : false;
		ibm8514.wait_vector_count = 0;
		return;
	}

	if(ibm8514.current_cmd & 0x1000)  // byte sequence
	{
		ibm8514_draw_ssv(data & 0xff);
		ibm8514_draw_ssv(data >> 8);
	}
	else
	{
		ibm8514_draw_ssv(data >> 8);
		ibm8514_draw_ssv(data & 0xff);
	}
	LOG("8514/A: Short Stroke Vector write %04x\n", data);
}

void ibm8514a_device::ibm8514_wait_draw_vector()
{
	uint8_t len = ibm8514.wait_vector_len;
	uint8_t dir = ibm8514.wait_vector_dir;
	bool draw = ibm8514.wait_vector_draw;
	uint8_t count = ibm8514.wait_vector_count;
	uint32_t offset;
	uint8_t data_size = 0;
	int x;

	if(ibm8514.bus_size == 0)  // 8-bit
		data_size = 8;
	if(ibm8514.bus_size == 1)  // 16-bit
		data_size = 16;
	if(ibm8514.bus_size == 2)  // 32-bit
		data_size = 32;

	for(x=0;x<data_size;x++)
	{
		if(len > count)
		{
			if(ibm8514.state == IBM8514_DRAWING_LINE)
			{
				ibm8514.state = IBM8514_IDLE;
				ibm8514.gpbusy = false;
				ibm8514.data_avail = false;
				return;
			}
		}

		if(ibm8514.state == IBM8514_DRAWING_LINE)
		{
			offset = (ibm8514.curr_y * IBM8514_LINE_LENGTH) + ibm8514.curr_x;
			if(draw)
				ibm8514_write(offset,offset);
			switch(dir)
			{
			case 0:  // 0 degrees
				ibm8514.curr_x++;
				break;
			case 1:  // 45 degrees
				ibm8514.curr_x++;
				ibm8514.curr_y--;
				break;
			case 2:  // 90 degrees
				ibm8514.curr_y--;
				break;
			case 3:  // 135 degrees
				ibm8514.curr_y--;
				ibm8514.curr_x--;
				break;
			case 4:  // 180 degrees
				ibm8514.curr_x--;
				break;
			case 5:  // 225 degrees
				ibm8514.curr_x--;
				ibm8514.curr_y++;
				break;
			case 6:  // 270 degrees
				ibm8514.curr_y++;
				break;
			case 7:  // 315 degrees
				ibm8514.curr_y++;
				ibm8514.curr_x++;
				break;
			}
		}
	}
}

/*
96E8h W(R/W):  Major Axis Pixel Count/Rectangle Width Register (MAJ_AXIS_PCNT)
bit  0-10  (911/924)  RECTANGLE WIDTH/LINE PARAMETER MAX. For BITBLT and
            rectangle commands this is the width of the area. For Line Drawing
            this is the Bresenham constant dmajor in two's complement format.
            (dmajor is the length of the line projected onto the major or
            independent axis). Must be positive.
     0-11  (80x +) RECTANGLE WIDTH/LINE PARAMETER MAX. See above
 */
uint16_t ibm8514a_device::ibm8514_width_r()
{
	return ibm8514.rect_width;
}

void ibm8514a_device::ibm8514_width_w(uint16_t data)
{
	ibm8514.rect_width = data & 0x1fff;
	LOG("8514/A: Major Axis Pixel Count / Rectangle Width write %04x\n", data);
}

uint16_t ibm8514a_device::ibm8514_currentx_r()
{
	return ibm8514.curr_x;
}

void ibm8514a_device::ibm8514_currentx_w(uint16_t data)
{
	ibm8514.curr_x = data;
	ibm8514.prev_x = data;
	LOG("8514/A: Current X set to %04x (%i)\n", data, ibm8514.curr_x);
}

uint16_t ibm8514a_device::ibm8514_currenty_r()
{
	return ibm8514.curr_y;
}

void ibm8514a_device::ibm8514_currenty_w(uint16_t data)
{
	ibm8514.curr_y = data;
	ibm8514.prev_y = data;
	LOG("8514/A: Current Y set to %04x (%i)\n", data, ibm8514.curr_y);
}

uint16_t ibm8514a_device::ibm8514_fgcolour_r()
{
	return ibm8514.fgcolour;
}

void ibm8514a_device::ibm8514_fgcolour_w(uint16_t data)
{
	ibm8514.fgcolour = data;
	LOG("8514/A: Foreground Colour write %04x\n", data);
}

uint16_t ibm8514a_device::ibm8514_bgcolour_r()
{
	return ibm8514.bgcolour;
}

void ibm8514a_device::ibm8514_bgcolour_w(uint16_t data)
{
	ibm8514.bgcolour = data;
	LOG("8514/A: Background Colour write %04x\n", data);
}

/*
AEE8h W(R/W):  Read Mask Register (RD_MASK)
bit   0-7  (911/924) Read Mask affects the following commands: CMD_RECT,
            CMD_BITBLT and reading data in Across Plane Mode.
            Each bit set prevents the plane from being read.
     0-15  (801/5) Readmask. See above.
     0-31  (928 +) Readmask. See above. In 32 bits per pixel modes there are
            two 16bit registers at this address. BEE8h index 0Eh bit 4 selects
            which 16 bits are accessible and each access toggles to the other
            16 bits.
 */
uint16_t ibm8514a_device::ibm8514_read_mask_r()
{
	return ibm8514.read_mask & 0xffff;
}

void ibm8514a_device::ibm8514_read_mask_w(uint16_t data)
{
	ibm8514.read_mask = (ibm8514.read_mask & 0xffff0000) | data;
	LOG("8514/A: Read Mask (Low) write = %08x\n", ibm8514.read_mask);
}

/*
AAE8h W(R/W):  Write Mask Register (WRT_MASK)
bit   0-7  (911/924) Writemask. A plane can only be modified if the
            corresponding bit is set.
     0-15  (801/5) Writemask. See above.
     0-31  (928 +) Writemask. See above. In 32 bits per pixel modes there are
            two 16bit registers at this address. BEE8h index 0Eh bit 4 selects
            which 16 bits are accessible and each access toggles to the other
            16 bits.
 */
uint16_t ibm8514a_device::ibm8514_write_mask_r()
{
	return ibm8514.write_mask & 0xffff;
}

void ibm8514a_device::ibm8514_write_mask_w(uint16_t data)
{
	ibm8514.write_mask = (ibm8514.write_mask & 0xffff0000) | data;
	LOG("8514/A: Write Mask (Low) write = %08x\n", ibm8514.write_mask);
}

uint16_t ibm8514a_device::ibm8514_multifunc_r()
{
	switch(ibm8514.multifunc_sel)
	{
	case 0:
		return ibm8514.rect_height;
	case 1:
		return ibm8514.scissors_top;
	case 2:
		return ibm8514.scissors_left;
	case 3:
		return ibm8514.scissors_bottom;
	case 4:
		return ibm8514.scissors_right;
		// TODO: remaining functions
	default:
		LOG("8514/A: Unimplemented multifunction register %i selected\n", ibm8514.multifunc_sel);
		return 0xff;
	}
}

void ibm8514a_device::ibm8514_multifunc_w(uint16_t data)
{
	switch(data & 0xf000)
	{
/*
BEE8h index 00h W(R/W): Minor Axis Pixel Count Register (MIN_AXIS_PCNT).
bit  0-10  (911/924) Rectangle Height. Height of BITBLT or rectangle command.
            Actual height is one larger.
     0-11  (80x +) Rectangle Height. See above
*/
	case 0x0000:
		ibm8514.rect_height = data & 0x0fff;
		LOG("8514/A: Minor Axis Pixel Count / Rectangle Height write %04x\n", data);
		break;
/*
BEE8h index 01h W(R/W):  Top Scissors Register (SCISSORS_T).
bit  0-10  (911/924) Clipping Top Limit. Defines the upper bound of the
            Clipping Rectangle (Lowest Y coordinate).
     0-11  (80x +) Clipping Top Limit. See above

BEE8h index 02h W(R/W):  Left Scissors Registers (SCISSORS_L).
bit  0-10  (911,924) Clipping Left Limit. Defines the left bound of the
            Clipping Rectangle (Lowest X coordinate).
     0-11  (80x +) Clipping Left Limit. See above.

BEE8h index 03h W(R/W):  Bottom Scissors Register (SCISSORS_B).
bit  0-10  (911,924) Clipping Bottom Limit. Defines the bottom bound of the
            Clipping Rectangle (Highest Y coordinate).
     0-11  (80x +) Clipping Bottom Limit. See above.

BEE8h index 04h W(R/W):  Right Scissors Register (SCISSORS_R).
bit  0-10  (911,924) Clipping Right Limit. Defines the right bound of the
            Clipping Rectangle (Highest X coordinate).
     0-11  (80x +) Clipping Bottom Limit. See above.
 */
	case 0x1000:
		ibm8514.scissors_top = data & 0x0fff;
		LOG("S3: Scissors Top write %04x\n", data);
		break;
	case 0x2000:
		ibm8514.scissors_left = data & 0x0fff;
		LOG("S3: Scissors Left write %04x\n", data);
		break;
	case 0x3000:
		ibm8514.scissors_bottom = data & 0x0fff;
		LOG("S3: Scissors Bottom write %04x\n", data);
		break;
	case 0x4000:
		ibm8514.scissors_right = data & 0x0fff;
		LOG("S3: Scissors Right write %04x\n", data);
		break;
/*
BEE8h index 0Ah W(R/W):  Pixel Control Register (PIX_CNTL).
BIT     2  (911-928) Pack Data. If set image read data is a monochrome bitmap,
            if clear it is a bitmap of the current pixel depth
      6-7  DT-EX-DRC. Select Mix Select.
             0  Foreground Mix is always used.
             1  use fixed pattern to decide which mix setting to use on a pixel
             2  CPU Data (Pixel Transfer register) determines the Mix register used.
             3  Video memory determines the Mix register used.
 */
	case 0xa000:
		ibm8514.pixel_control = data;
		LOG("S3: Pixel control write %04x\n", data);
		break;
	case 0xe000:
		ibm8514.multifunc_misc = data;
		LOG("S3: Multifunction Miscellaneous write %04x\n", data);
		break;
/*
BEE8h index 0Fh W(W):  Read Register Select Register (READ_SEL)    (801/5,928)
bit   0-2  (911-928) READ-REG-SEL. Read Register Select. Selects the register
            that is actually read when a read of BEE8h happens. Each read of
            BEE8h increments this register by one.
             0: Read will return contents of BEE8h index 0.
             1: Read will return contents of BEE8h index 1.
             2: Read will return contents of BEE8h index 2.
             3: Read will return contents of BEE8h index 3.
             4: Read will return contents of BEE8h index 4.
             5: Read will return contents of BEE8h index 0Ah.
             6: Read will return contents of BEE8h index 0Eh.
             7: Read will return contents of 9AE8h (Bits 13-15 will be 0).
      0-3  (864,964) READ-REG-SEL. See above plus:
             8: Read will return contents of 42E8h (Bits 12-15 will be 0)
             9: Read will return contents of 46E8h
            10: Read will return contents of BEE8h index 0Dh
 */
	case 0xf000:
		ibm8514.multifunc_sel = data & 0x000f;
		LOG("S3: Multifunction select write %04x\n", data);
		break;
	default:
		LOG("S3: Unimplemented multifunction register %i write %03x\n", data >> 12, data & 0x0fff);
		break;
	}
}

void ibm8514a_device::ibm8514_wait_draw()
{
	int x, data_size = 8;
	uint32_t off;

	// the data in the pixel transfer register or written to VRAM masks the rectangle output
	if(ibm8514.bus_size == 0)  // 8-bit
		data_size = 8;
	if(ibm8514.bus_size == 1)  // 16-bit
		data_size = 16;
	if(ibm8514.bus_size == 2)  // 32-bit
		data_size = 32;
	off = 0;
	off += (IBM8514_LINE_LENGTH * ibm8514.curr_y);
	off += ibm8514.curr_x;
	if(ibm8514.current_cmd & 0x02) // "across plane mode"
	{
		for(x=0;x<data_size;x++)
		{
			ibm8514_write(off,off);
			if(ibm8514.current_cmd & 0x0020)
			{
				off++;
				ibm8514.curr_x++;
				if(ibm8514.curr_x > ibm8514.prev_x + ibm8514.rect_width)
				{
					ibm8514.curr_x = ibm8514.prev_x;
					ibm8514.src_x = 0;
					if(ibm8514.current_cmd & 0x0080)
					{
						ibm8514.curr_y++;
						if(ibm8514.curr_y > ibm8514.prev_y + ibm8514.rect_height)
						{
							ibm8514.state = IBM8514_IDLE;
							ibm8514.data_avail = false;
							ibm8514.gpbusy = false;
						}
					}
					else
					{
						ibm8514.curr_y--;
						if(ibm8514.curr_y < ibm8514.prev_y - ibm8514.rect_height)
						{
							ibm8514.state = IBM8514_IDLE;
							ibm8514.data_avail = false;
							ibm8514.gpbusy = false;
						}
					}
					return;
				}
			}
			else
			{
				off--;
				ibm8514.curr_x--;
				if(ibm8514.curr_x < ibm8514.prev_x - ibm8514.rect_width)
				{
					ibm8514.curr_x = ibm8514.prev_x;
					ibm8514.src_x = 0;
					if(ibm8514.current_cmd & 0x0080)
					{
						ibm8514.curr_y++;
						if(ibm8514.curr_y > ibm8514.prev_y + ibm8514.rect_height)
						{
							ibm8514.state = IBM8514_IDLE;
							ibm8514.gpbusy = false;
							ibm8514.data_avail = false;
						}
					}
					else
					{
						ibm8514.curr_y--;
						if(ibm8514.curr_y < ibm8514.prev_y - ibm8514.rect_height)
						{
							ibm8514.state = IBM8514_IDLE;
							ibm8514.gpbusy = false;
							ibm8514.data_avail = false;
						}
					}
					return;
				}
			}
		}
	}
	else
	{
		// "through plane" mode (single pixel)
		for(x=0;x<data_size;x+=8)
		{
			ibm8514_write(off,off);

			if(ibm8514.current_cmd & 0x0020)
			{
				off++;
				ibm8514.curr_x++;
				if(ibm8514.curr_x > ibm8514.prev_x + ibm8514.rect_width)
				{
					ibm8514.curr_x = ibm8514.prev_x;
					ibm8514.src_x = 0;
					if(ibm8514.current_cmd & 0x0080)
					{
						ibm8514.curr_y++;
						if(ibm8514.curr_y > ibm8514.prev_y + ibm8514.rect_height)
						{
							ibm8514.state = IBM8514_IDLE;
							ibm8514.gpbusy = false;
							ibm8514.data_avail = false;
						}
					}
					else
					{
						ibm8514.curr_y--;
						if(ibm8514.curr_y < ibm8514.prev_y - ibm8514.rect_height)
						{
							ibm8514.state = IBM8514_IDLE;
							ibm8514.gpbusy = false;
							ibm8514.data_avail = false;
						}
					}
					return;
				}
			}
			else
			{
				off--;
				ibm8514.curr_x--;
				if(ibm8514.curr_x < ibm8514.prev_x - ibm8514.rect_width)
				{
					ibm8514.curr_x = ibm8514.prev_x;
					ibm8514.src_x = 0;
					if(ibm8514.current_cmd & 0x0080)
					{
						ibm8514.curr_y++;
						if(ibm8514.curr_y > ibm8514.prev_y + ibm8514.rect_height)
						{
							ibm8514.state = IBM8514_IDLE;
							ibm8514.gpbusy = false;
							ibm8514.data_avail = false;
						}
					}
					else
					{
						ibm8514.curr_y--;
						if(ibm8514.curr_y < ibm8514.prev_y - ibm8514.rect_height)
						{
							ibm8514.state = IBM8514_IDLE;
							ibm8514.gpbusy = false;
							ibm8514.data_avail = false;
						}
					}
					return;
				}
			}
		}
	}
}

/*
B6E8h W(R/W):  Background Mix Register (BKGD_MIX)
bit   0-3  Background MIX (BACKMIX).
            00  not DST
            01  0 (false)
            02  1 (true)
            03  2 DST
            04  not SRC
            05  SRC xor DST
            06  not (SRC xor DST)
            07  SRC
            08  not (SRC and DST)
            09  (not SRC) or DST
            0A  SRC or (not DST)
            0B  SRC or DST
            0C  SRC and DST
            0D  SRC and (not DST)
            0E  (not SRC) and DST
            0F  not (SRC or DST)
           DST is always the destination bitmap, bit SRC has four
           possible sources selected by the BSS bits.
      5-6  Background Source Select (BSS)
             0  BSS is Background Color
             1  BSS is Foreground Color
             2  BSS is Pixel Data from the PIX_TRANS register (E2E8h)
             3  BSS is Bitmap Data (Source data from display buffer).
 */
uint16_t ibm8514a_device::ibm8514_backmix_r()
{
	return ibm8514.bgmix;
}

void ibm8514a_device::ibm8514_backmix_w(uint16_t data)
{
	ibm8514.bgmix = data;
	LOG("8514/A: BG Mix write %04x\n", data);
}

uint16_t ibm8514a_device::ibm8514_foremix_r()
{
	return ibm8514.fgmix;
}

void ibm8514a_device::ibm8514_foremix_w(uint16_t data)
{
	ibm8514.fgmix = data;
	LOG("8514/A: FG Mix write %04x\n", data);
}

uint16_t ibm8514a_device::ibm8514_pixel_xfer_r(offs_t offset)
{
	if(offset == 1)
		return (ibm8514.pixel_xfer & 0xffff0000) >> 16;
	else
		return ibm8514.pixel_xfer & 0x0000ffff;
}

void ibm8514a_device::ibm8514_pixel_xfer_w(offs_t offset, uint16_t data)
{
	if(offset == 1)
		ibm8514.pixel_xfer = (ibm8514.pixel_xfer & 0x0000ffff) | (data << 16);
	else
		ibm8514.pixel_xfer = (ibm8514.pixel_xfer & 0xffff0000) | data;

	if(ibm8514.state == IBM8514_DRAWING_RECT)
		ibm8514_wait_draw();

	if(ibm8514.state == IBM8514_DRAWING_SSV_1 || ibm8514.state == IBM8514_DRAWING_SSV_2)
		ibm8514_wait_draw_ssv();

	if(ibm8514.state == IBM8514_DRAWING_LINE)
		ibm8514_wait_draw_vector();

	LOG("8514/A: Pixel Transfer = %08x\n", ibm8514.pixel_xfer);
}

/*
02E8h W(R):  Display Status Register
bit     0  SENSE is the result of a wired-OR of 3 comparators, one
           for each of the RGB video signal.
           By programming the RAMDAC for various values
           and patterns and then reading the SENSE, the monitor type
           (color, monochrome or none) can be determined.
        1  VBLANK. Vertical Blank State
           If Vertical Blank is active this bit is set.
        2  HORTOG. Horizontal Toggle
           This bit toggles every time a HSYNC pulse starts
     3-15  Reserved(0)
 */
uint8_t ibm8514a_device::ibm8514_status_r(offs_t offset)
{
	switch(offset)
	{
		case 0:
			return m_vga->vga_vblank() << 1;
		case 2:
			return m_vga->ramdac_mask_r(0);
		case 3:
			return m_vga->ramdac_state_r(0);
		case 4:
			return m_vga->ramdac_write_index_r(0);
		case 5:
			return m_vga->ramdac_data_r(0);
	}
	return 0;
}

void ibm8514a_device::ibm8514_htotal_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0:
			ibm8514.htotal = data & 0xff;
			break;
		case 2:
			m_vga->ramdac_mask_w(0, data);
			break;
		case 3:
			m_vga->ramdac_read_index_w(0, data);
			break;
		case 4:
			m_vga->ramdac_write_index_w(0, data);
			break;
		case 5:
			m_vga->ramdac_data_w(0, data);
			break;
	}
	//vga.crtc.horz_total = data & 0x01ff;
	LOG("8514/A: Horizontal total write %04x\n", data);
}

/*
42E8h W(R):  Subsystem Status Register (SUBSYS_STAT)
bit   0-3  Interrupt requests. These bits show the state of internal interrupt
           requests. An interrupt will only occur if the corresponding bit(s)
           in SUBSYS_CNTL is set. Interrupts can only be reset by writing a 1
           to the corresponding Interrupt Clear bit in SUBSYS_CNTL.
             Bit 0: VBLNKFLG
                 1: PICKFLAG
                 2: INVALIDIO
                 3: GPIDLE
      4-6  MONITORID.
              1: IBM 8507 (1024x768) Monochrome
              2: IBM 8514 (1024x768) Color
              5: IBM 8503 (640x480) Monochrome
              6: IBM 8512/13 (640x480) Color
        7  8PLANE.
           (CT82c480) This bit is latched on reset from pin P4D7.
     8-11  CHIP_REV. Chip revision number.
    12-15  (CT82c480) CHIP_ID. 0=CT 82c480.
 */
uint16_t ibm8514a_device::ibm8514_substatus_r()
{
	// TODO:
	if(m_vga->vga_vblank() != 0)  // not correct, but will do for now
		ibm8514.substatus |= 0x01;
	return ibm8514.substatus;
}

/*
42E8h W(W):  Subsystem Control Register (SUBSYS_CNTL)
bit   0-3  Interrupt Reset. Write 1 to a bit to reset the interrupt.
           Bit 0  RVBLNKFLG   Write 1 to reset Vertical Blank interrupt.
               1  RPICKFLAG   Write 1 to reset PICK interrupt.
               2  RINVALIDIO  Write 1 to reset Queue Overflow/Data
                              Underflow interrupt.
               3  RGPIDLE     Write 1 to reset GPIDLE interrupt.
      4-7  Reserved(0)
        8  IBLNKFLG.   If set Vertical Blank Interrupts are enabled.
        9  IPICKFLAG.  If set PICK Interrupts are enabled.
       10  IINVALIDIO. If set Queue Overflow/Data Underflow Interrupts are
                       enabled.
       11  IGPIDLE.    If set Graphics Engine Idle Interrupts are enabled.
    12-13  CHPTEST. Used for chip testing.
    14-15  Graphics Processor Control (GPCTRL).
 */
void ibm8514a_device::ibm8514_subcontrol_w(uint16_t data)
{
	ibm8514.subctrl = data;
	ibm8514.substatus &= ~(data & 0x0f);  // reset interrupts
//  LOG("8514/A: Subsystem control write %04x\n", data);
}

uint16_t ibm8514a_device::ibm8514_subcontrol_r()
{
	return ibm8514.subctrl;
}

/*  22E8 (W)
 * Display Control
 *  bits 1-2: Line skip control - 0=bits 1-2 skipped, 1=bit 2 skipped
 *  bit    3: Double scan
 *  bit    4: Interlace
 *  bits 5-6: Emable Display - 0=no change, 1=enable 8514/A, 2 or 3=8514/A reset
 */
void ibm8514a_device::ibm8514_display_ctrl_w(uint16_t data)
{
	ibm8514.display_ctrl = data & 0x7e;
	switch(data & 0x60)
	{
		case 0x00:
			break;  // do nothing
		case 0x20:
			ibm8514.enabled = true;  // enable 8514/A
			break;
		case 0x40:
		case 0x60:  // reset (does this disable the 8514/A?)
			ibm8514.enabled = false;
			break;
	}
}

void ibm8514a_device::ibm8514_advfunc_w(uint16_t data)
{
	ibm8514.advfunction_ctrl = data;
	ibm8514.passthrough = data & 0x0001;
}

uint16_t ibm8514a_device::ibm8514_htotal_r()
{
	return ibm8514.htotal;
}

uint16_t ibm8514a_device::ibm8514_vtotal_r()
{
	return ibm8514.vtotal;
}

void ibm8514a_device::ibm8514_vtotal_w(uint16_t data)
{
	ibm8514.vtotal = data;
//  vga.crtc.vert_total = data;
	LOG("8514/A: Vertical total write %04x\n", data);
}

uint16_t ibm8514a_device::ibm8514_vdisp_r()
{
	return ibm8514.vdisp;
}

void ibm8514a_device::ibm8514_vdisp_w(uint16_t data)
{
	ibm8514.vdisp = data;
//  vga.crtc.vert_disp_end = data >> 3;
	LOG("8514/A: Vertical Displayed write %04x\n", data);
}

uint16_t ibm8514a_device::ibm8514_vsync_r()
{
	return ibm8514.vsync;
}

void ibm8514a_device::ibm8514_vsync_w(uint16_t data)
{
	ibm8514.vsync = data;
	LOG("8514/A: Vertical Sync write %04x\n", data);
}

void ibm8514a_device::enabled()
{
	ibm8514.state = IBM8514_IDLE;
	ibm8514.gpbusy = false;
}



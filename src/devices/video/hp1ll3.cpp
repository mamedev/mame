// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev, F. Ulivi
/*
    HP 1LL3-0005 GPU emulation.

    Used by HP Integral PC, possibly other HP products.

    On IPC, memory is 4 16Kx4bit DRAM chips = 32KB total (16K words),
    but firmware probes memory size and can work with 128KB memory.
    Undocumented "_desktop" mode requires this.

    Capabilities:
    - up to 1024x1024 px on screen
    - lines
    - rectangles
    - area fill with user-defined pattern
    - 16x16 user-defined proportional font, with automatic cursor
    - 16x16 user-defined sprite for mouse cursor (not a sprite layer)
    - windows with blitter (copy, fill and scroll) and clipping

    To do:
    - interrupt generation
    - realistic timing?
    - &c.

    This table summarizes the GPU commands.
    "Code" is the hex code of command.
    "Impl" shows whether the command is implemented in this driver or not.
    "Parameter" reports the I/O data exchanged by the command. Each line
    is a 16-bit word which is read from/written to the GPU by a pair of
    8-bit accesses to data register (MSB first).
    "I" is an input parameter (sent before command code)
    "D" is an input data word (sent after command code)
    "O" is an output data word.

    | *Cmd*     | *Code* | *Impl.* | *Parameter*         | *Description*                     |
    |-----------+--------+---------+---------------------+-----------------------------------|
    | NOP       |     00 | Y       | -                   | No operation. Used to close       |
    |           |        |         |                     | RDMEM/WRMEM/RDWIN/WRWIN commands. |
    | CONF      |     02 | Y       | D@0: H. timing 0    | Configure GPU                     |
    |           |        |         | D@1: H. timing 1    |                                   |
    |           |        |         | D@2: H. timing 2    |                                   |
    |           |        |         | D@3: H. timing 3    |                                   |
    |           |        |         | D@4: V. timing 0    |                                   |
    |           |        |         | D@5: V. timing 1    |                                   |
    |           |        |         | D@6: V. timing 2    |                                   |
    |           |        |         | D@7: V. timing 3    |                                   |
    |           |        |         | D@8: Words per line |                                   |
    |           |        |         | D@9: Lines          |                                   |
    |           |        |         | D@10: Option bits   |                                   |
    | DISVID    |     03 | Y       | -                   | Disable video                     |
    | ENVID     |     04 | Y       | -                   | Enable video                      |
    | ?         |     06 | Y       | -                   | Unknown                           |
    |           |        |         |                     | My guess is interrupt enable      |
    |           |        |         |                     | 05 is probably int. disable       |
    |           |        |         |                     | It's only used by diagb ROM       |
    | WRMEM     |     07 | Y       | I@0: RAM address    | Write video RAM. Keep writing     |
    |           |        |         | D@0: RAM word       | words with auto-incremented addr  |
    |           |        |         | ...                 | until terminated by NOP.          |
    | RDMEM     |     08 | Y       | I@0: RAM address    | Read video RAM. Read words until  |
    |           |        |         | O@0: RAM word       | terminated by NOP.                |
    |           |        |         | ...                 |                                   |
    | WRSAD     |     09 | Y       | I@0: Start address  | Set screen start address          |
    | WRORG     |     0a | Y       | I@0: Start address  | Set raster start address          |
    | WRDAD     |     0b | Y       | I@0: Start address  | Set data start address            |
    | WRRR      |     0c | Y       | I@0: RR             | Set replacement rule              |
    | MOVEP     |     0d | Y       | I@0: X              | Set pen position                  |
    |           |        |         | I@1: Y              |                                   |
    | IMOVEP    |     0e | Y       | I@0: delta X        | Set pen position (incremental)    |
    |           |        |         | I@1: delta Y        |                                   |
    | DRAWP     |     0f | Y       | I@0: X              | Draw a line and move pen          |
    |           |        |         | I@1: Y              |                                   |
    | IDRAWP    |     10 | N       | I@0: delta X        | Draw a line (incremental)         |
    |           |        |         | I@1: delta Y        |                                   |
    | RDP       |     11 | Y       | O@0: pen X pos      | Read current pen position         |
    |           |        |         | O@1: pen Y pos      |                                   |
    | WRUDL     |     12 | Y       | I@0: UDL            | Set user-defined line pattern     |
    | WRWINSIZ  |     13 | Y       | I@0: Window width   | Set window size                   |
    |           |        |         | I@1: Window height  |                                   |
    | WRWINORG  |     14 | Y       | I@0: Origin X       | Set window origin (UL corner)     |
    |           |        |         | I@1: Origin Y       |                                   |
    | COPY      |     15 | Y       | I@0: Dest. origin X | Copy a window. Parameter sets     |
    |           |        |         | I@1: Dest. origin Y | the UL corner of the dest. win.   |
    | FILL      |     16 | Y       | I@0: Pattern no.    | Fill a window                     |
    | FRAME     |     17 | N       | -                   | Draw a frame (rectangle)          |
    | SCROLUP   |     18 | Y       | I@0: Fill pattern   | Scroll a window upwards           |
    |           |        |         | I@1: Scroll amount  |                                   |
    | SCROLDN   |     19 | Y       | I@0: Fill pattern   | Scroll a window downwards         |
    |           |        |         | I@1: Scroll amount  |                                   |
    | SCROLLF   |     1a | N       | I@0: Fill pattern   | Scroll a window to the left       |
    |           |        |         | I@1: Scroll amount  |                                   |
    | SCROLLF   |     1b | N       | I@0: Fill pattern   | Scroll a window to the right      |
    |           |        |         | I@1: Scroll amount  |                                   |
    | RDWIN     |     1c | Y       | I@0: bit offset     | Read a window. Terminated by NOP. |
    |           |        |         | O@0: data word      |                                   |
    |           |        |         | ...                 |                                   |
    | WRWIN     |     1d | Y       | I@0: bit offset     | Write a window. Terminated by NOP.|
    |           |        |         | D@0: data word      |                                   |
    |           |        |         | ...                 |                                   |
    | RDWINPARM |     1e | Y       | O@0: Win. origin X  | Read current window parameters    |
    |           |        |         | O@1: Win. origin Y  |                                   |
    |           |        |         | O@2: Window width   |                                   |
    |           |        |         | O@3: Window height  |                                   |
    | CR        |     1f | N       | -                   | Carriage return: move pen to      |
    |           |        |         |                     | start of line                     |
    | CRLF      |     20 | Y       | -                   | CRLF: move pen to start of next   |
    |           |        |         |                     | text row                          |
    | LABEL     |     24 | Y       | I@0: character code | Write a character and move pen    |
    | ENSP      |     26 | Y       | I@0: sprite pattern | Enable sprite                     |
    | DISSP     |     27 | Y       | -                   | Disable sprite                    |
    | MOVESP    |     28 | Y       | I@0: Sprite X pos   | Move sprite                       |
    |           |        |         | I@1: Sprite Y pos   |                                   |
    | IMOVESP   |     29 | N       | I@0: Sprite delta X | Move sprite (incremental)         |
    |           |        |         | I@1: Sprite delta Y |                                   |
    | RDSP      |     2a | Y       | O@0: Sprite X pos   | Read sprite position              |
    |           |        |         | O@1: Sprite Y pos   |                                   |
    | DRAWPX    |     2b | Y       | I@0: X              | Write a pixel                     |
    |           |        |         | I@1: Y              |                                   |
    | WRFAD     |     2c | Y       | I@0: Font address   | Set address of font data          |
    | ENCURS    |     2d | Y       | I@0: cursor pattern | Enable cursor                     |
    |           |        |         | I@1: cursor offset  |                                   |
    | DISCURS   |     2e | Y       | -                   | Disable cursor                    |
    | ID        |     3f | Y       | O@0: ID             | Read hw ID                        |
*/

#include "emu.h"
#include "hp1ll3.h"

#include "screen.h"


///*************************************************************************
//  MACROS / CONSTANTS
///*************************************************************************

/*
 * command types (send)
 *
 * 0 -- no data
 * 1 -- write 1 word of data, then command
 * 2 -- write 2 words of data, then command
 * 3 -- write command, then 12 words of data (= CONF only)
 * 4 -- write 1 word of data, then command, then write X words of data, then write NOP
 *
 * (read)
 *
 * 3 -- write command, then read 2 words of data (X & Y coordinates)
 * 4 -- write 1 word of data, then command, then read X words of data, then write NOP
 */
#define NOP         0   // type 0
#define CONF        2   // type 3, configure GPU (screen size, timings...).  11 words of data.
#define DISVID      3   // type 0, disable video
#define ENVID       4   // type 0, enable video
#define WRMEM       7   // type 4, write GPU memory at offset, terminate by NOP
#define RDMEM       8   // type 4, read GPU memory from offset, terminate by NOP
#define WRSAD       9   // type 1, set screen area start address
#define WRORG       10  // type 1, set raster start address
#define WRDAD       11  // type 1, set data area start address (16x16 area fill, sprite and cursor)
#define WRRR        12  // type 1, set replacement rule (rasterop)
#define MOVEP       13  // type 2, move pointer
#define IMOVEP      14  // type 2, incremental move pointer
#define DRAWP       15  // type 2, draw line
#define IDRAWP      16  // type 2, incremental draw line
#define RDP         17  // type 3, read pointer position
#define WRUDL       18  // type 1, set user-defined line pattern (16-bit)
#define WRWINSIZ    19  // type 2, set window size
#define WRWINORG    20  // type 2, set window origin
#define COPY        21  // type 2, block copy
#define FILL        22  // type 1, fill area
#define FRAME       23  // type 0, draw rectangle
#define SCROLUP     24  // type 2, scroll up
#define SCROLDN     25  // type 2, scroll down
#define SCROLLF     26  // type 2, scroll left
#define SCROLRT     27  // type 2, scroll right
#define RDWIN       28  // type 4, read window raster
#define WRWIN       29  // type 4, write window raster
#define RDWINPARM   30
#define CR          31  // type 0, carriage return
#define CRLFx       32  // type 0, CR + LF
#define LABEL       36  // type 1, draw text
#define ENSP        38  // type 1, enable sprite
#define DISSP       39  // type 0, disable sprite
#define MOVESP      40  // type 2, move sprite
#define IMOVESP     41  // type 2, incremental move sprite
#define RDSP        42  // type 3, read sprite position
#define DRAWPX      43  // type 2, draw single pixel
#define WRFAD       44  // type 1, set font area start address
#define ENCURS      45  // type 2, enable cursor
#define DISCURS     46  // type 0, disable cursor
#define ID          63


/*
 * Replacement Rules (rops).  sources:
 *
 * - NetBSD's diofbvar.h (definitions for Topcat chip)
 * - pdf/hp/9000_300/specs/A-5958-4362-9_Series_300_Display_Color_Card_Theory_of_Operation_Oct85.pdf
 *   refers to TOPCAT documentation p/n A-1FH2-2001-7 (not online)
 */
#define RR_FORCE_ZERO   0x0
#define RR_CLEAR        RR_FORCE_ZERO
#define RR_AND          0x1
#define RR_AND_NOT_OLD  0x2
#define RR_NEW          0x3
#define RR_COPY         RR_NEW
#define RR_AND_NOT_NEW  0x4
#define RR_OLD          0x5
#define RR_XOR          0x6
#define RR_OR           0x7
#define RR_NOR          0x8
#define RR_XNOR         0x9
#define RR_NOT_OLD      0xa
#define RR_INVERT       RR_NOT_OLD
#define RR_OR_NOT_OLD   0xb
#define RR_NOT_NEW      0xc
#define RR_COPYINVERTED RR_NOT_NEW
#define RR_OR_NOT_NEW   0xd
#define RR_NAND         0xe
#define RR_FORCE_ONE    0xf

constexpr unsigned WS = 16; // bits in a word
constexpr uint16_t MSB_MASK = 0x8000;   // Mask of MSB

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define VERBOSE_DBG 2       /* general debug messages */

#define DBG_LOG(N,M,A) \
	do { \
		if(VERBOSE_DBG>=N) \
		{ \
			if( M ) \
				logerror("%11.6f at %s: %-16s",machine().time().as_double(),machine().describe_context(),(char*)M ); \
			logerror A; \
		} \
	} while (0)

// Index of words in configuration vector
enum : unsigned
{
	CONF_HOR_0 = 0,     // Horizontal timing, 1st word
	CONF_HOR_1 = 1,     // Horizontal timing, 2nd word
	CONF_HOR_2 = 2,     // Horizontal timing, 3rd word
	CONF_HOR_3 = 3,     // Horizontal timing, 4th word
	CONF_VER_0 = 4,     // Vertical timing, 1st word
	CONF_VER_1 = 5,     // Vertical timing, 2nd word
	CONF_VER_2 = 6,     // Vertical timing, 3rd word
	CONF_VER_3 = 7,     // Vertical timing, 4th word
	CONF_WPL   = 8,     // Words per line
	CONF_LINES = 9,     // Lines
	CONF_OPTS  = 10     // RAM size or option bits (more likely)
};

// Fields of hor/ver timing words
enum : uint16_t
{
	HV_CNT_MASK  = 0x3ff,   // Mask of counter part
	HV_ZONE_MASK = 0xc00,   // Mask of zone part
	HV_ZONE_0    = 0x000,   // Value for zone 0
	HV_ZONE_1    = 0x400,   // Value for zone 1
	HV_ZONE_2    = 0x800,   // Value for zone 2
	HV_ZONE_3    = 0xc00    // Value for zone 3
};

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
DEFINE_DEVICE_TYPE(HP1LL3, hp1ll3_device, "hp1ll3", "Hewlett-Packard 1LL3-0005 GPU")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  hp1ll3_device - constructor
//-------------------------------------------------

hp1ll3_device::hp1ll3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HP1LL3, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_vram_size(16)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hp1ll3_device::device_start()
{
	const rectangle &visarea = screen().visible_area();

	// register for state saving
	save_item(NAME(m_conf));

	screen().register_screen_bitmap(m_bitmap);

	m_videoram = std::make_unique<uint16_t[]>(m_vram_size);

	save_pointer(NAME(m_videoram) , m_vram_size);
	m_ram_addr_mask = uint16_t(m_vram_size - 1);
	m_horiz_pix_total = visarea.max_x + 1;
	m_vert_pix_total = visarea.max_y + 1;
}

void hp1ll3_device::device_reset()
{
	m_rd_ptr = m_wr_ptr = m_command = 0;
	m_sad = m_fad = m_dad = m_org = m_rr = m_udl = 0;
	m_enable_video = m_enable_cursor = m_enable_sprite = m_busy = false;
}

uint16_t hp1ll3_device::get_pix_addr(uint16_t x, uint16_t y) const
{
	return m_org + y * m_conf[CONF_WPL] + x / WS;
}

inline void hp1ll3_device::point(int x, int y, bool pix, const uint16_t masks[])
{
	uint16_t offset = get_pix_addr(x, y);
	uint16_t pix_mask = MSB_MASK >> (x % WS);

	rmw_rop(offset, pix ? ~0 : 0, pix_mask, masks);
}

// Bresenham algorithm -- from ef9365.cpp
void hp1ll3_device::line(int x1, int y1, int x2, int y2)
{
	int dx;
	int dy,t;
	int e;
	int x,y;
	int incy;
	int diago,horiz;
	unsigned char c1;

	uint16_t rop_masks[4];
	get_rop_masks(m_rr, rop_masks);

	uint16_t udl_scan = MSB_MASK;

	c1 = 0;
	incy = 1;

	if (x2 > x1)
		dx = x2 - x1;
	else
		dx = x1 - x2;

	if (y2 > y1)
		dy = y2 - y1;
	else
		dy = y1 - y2;

	if (dy > dx)
	{
		t = y2;
		y2 = x2;
		x2 = t;

		t = y1;
		y1 = x1;
		x1 = t;

		t = dx;
		dx = dy;
		dy = t;

		c1 = 1;
	}

	if (x1 > x2)
	{
		t = y2;
		y2 = y1;
		y1 = t;

		t = x1;
		x1 = x2;
		x2 = t;
	}

	horiz = dy << 1;
	diago = (dy - dx) << 1;
	e = (dy << 1) - dx;

	if (y1 <= y2)
		incy = 1;
	else
		incy = -1;

	x = x1;
	y = y1;

	if (c1)
	{
		do
		{
			point(y, x, m_udl & udl_scan,rop_masks);
			udl_scan >>= 1;
			if (!udl_scan)
				udl_scan = MSB_MASK;

			if (e > 0)
			{
				y = y + incy;
				e = e + diago;
			}
			else
			{
				e = e + horiz;
			}

			x++;

		} while (x <= x2);
	}
	else
	{
		do
		{
			point(x, y, m_udl & udl_scan,rop_masks);
			udl_scan >>= 1;
			if (!udl_scan)
				udl_scan = MSB_MASK;

			if (e > 0)
			{
				y = y + incy;
				e = e + diago;
			}
			else
			{
				e = e + horiz;
			}

			x++;

		} while (x <= x2);
	}
}

void hp1ll3_device::get_font(uint16_t& font_data, uint16_t& font_height) const
{
	font_data = m_fad + rd_video(m_fad + 1) + 2;
	font_height = rd_video(m_fad);
}

void hp1ll3_device::label(uint8_t chr, int width)
{
	draw_cursor_sprite();

	uint16_t font_data;
	uint16_t font_height;
	get_font(font_data, font_height);

	Rectangle clip = get_window();
	Rectangle dst{{ m_cursor_x, m_cursor_y }, { uint16_t(width), font_height }};
	bitblt(font_data + chr * 16, 16, 16, Point { 0, 0 }, clip, dst, m_rr);
	m_cursor_x += width;
	draw_cursor_sprite();
}

void hp1ll3_device::wr_video(uint16_t addr, uint16_t v)
{
	m_videoram[addr & m_ram_addr_mask] = v;
}

uint16_t hp1ll3_device::rd_video(uint16_t addr) const
{
	return m_videoram[addr & m_ram_addr_mask];
}

void hp1ll3_device::get_rop_masks(uint16_t rop, uint16_t masks[])
{
	masks[0] = BIT(rop, 0) ? ~0 : 0;
	masks[1] = BIT(rop, 1) ? ~0 : 0;
	masks[2] = BIT(rop, 2) ? ~0 : 0;
	masks[3] = BIT(rop, 3) ? ~0 : 0;
}

uint16_t hp1ll3_device::apply_rop(uint16_t old_pix, uint16_t new_pix, uint16_t glob_mask, const uint16_t masks[])
{
	uint16_t diff0 = old_pix & new_pix;
	uint16_t diff1 = ~old_pix & new_pix;
	uint16_t diff2 = old_pix & ~new_pix;
	uint16_t diff3 = ~old_pix & ~new_pix;

	// The idea here is that ROP is forced to be "RR_OLD" for all zero bits in glob_mask
	// so that the corresponding bits in new_pix are ignored
	return
			((masks[0] | ~glob_mask) & diff0) |
			(masks[1] & glob_mask & diff1) |
			((masks[2] | ~glob_mask) & diff2) |
			(masks[3] & glob_mask & diff3);
}

void hp1ll3_device::rmw_rop(uint16_t addr, uint16_t new_pix, uint16_t glob_mask, const uint16_t masks[])
{
	wr_video(addr, apply_rop(rd_video(addr), new_pix, glob_mask, masks));
}

void hp1ll3_device::clip_coord(int size_1, int &p1, int origin_clip, int size_clip, int &origin_2, int &size_2) const
{
	// origin_1 is implicitly 0
	int corner_2 = origin_2 + size_2;
	// Clip origin_2 & p1 w.r.t. clip rectangle
	int t = origin_2 - origin_clip;
	if (t < 0)
	{
		p1 -= t;
		origin_2 = origin_clip;
	}
	// Clip size_2 w.r.t. clip rectangle
	int corner_clip = origin_clip + size_clip;
	if (corner_2 > corner_clip)
		size_2 = corner_clip - origin_2;

	// Clip size_2 w.r.t. rectangle 1 (whose origin is 0,0)
	t = p1 + size_2 - size_1;
	if (t > 0)
		size_2 -= t;
}

bool hp1ll3_device::bitblt(uint16_t src_base_addr, unsigned src_width, unsigned src_height, Point src_p,
						   const Rectangle &clip_rect, const Rectangle &dst_rect, uint16_t rop, bool use_m_org)
{
	DBG_LOG(3,0,("bitblt %04x,%u,%u,(%u,%u),(%u,%u,%u,%u),(%u,%u,%u,%u),%u\n", src_base_addr,
				 src_width,
				 src_height,
				 src_p.x,
				 src_p.y,
				 clip_rect.origin.x, clip_rect.origin.y, clip_rect.size.x, clip_rect.size.y,
				 dst_rect.origin.x, dst_rect.origin.y, dst_rect.size.x, dst_rect.size.y,
				 rop));
	int src_x = src_p.x;
	int dst_x = static_cast<int16_t>(dst_rect.origin.x);
	int dst_width = dst_rect.size.x;
	int src_y = src_p.y;
	int dst_y = static_cast<int16_t>(dst_rect.origin.y);
	int dst_height = dst_rect.size.y;
	// Clip x-coordinates
	clip_coord(src_width, src_x, clip_rect.origin.x, clip_rect.size.x, dst_x, dst_width);
	// Clip y-coordinates
	clip_coord(src_height, src_y, clip_rect.origin.y, clip_rect.size.y, dst_y, dst_height);

	DBG_LOG(3,0,("bitblt (%u,%u) (%u,%u,%u,%u)\n", src_x, src_y, dst_x, dst_y, dst_width, dst_height));
	if (dst_width <= 0 || dst_height <= 0)
		return false;

	unsigned dst_rounded_width = m_conf[CONF_WPL] * WS;

	// p1_pix & p2_pix point to a uniquely identified pixel in video RAM
	unsigned p1_pix = unsigned(src_base_addr) * WS + src_x + src_width * src_y;
	unsigned p2_pix = unsigned(use_m_org ? m_org : m_sad) * WS + dst_x + dst_rounded_width * dst_y;

	DBG_LOG(3,0,("bitblt p1_pix=%u p2_pix=%u\n", p1_pix, p2_pix));
	uint16_t rop_masks[4];
	get_rop_masks(rop, rop_masks);

	if (p1_pix < p2_pix)
	{
		// Move block going up (decrease y)
		p1_pix += dst_height * src_width + dst_width - 1;
		p2_pix += dst_height * dst_rounded_width + dst_width - 1;
		DBG_LOG(3,0,("bitblt rev p1_pix=%u p2_pix=%u\n", p1_pix, p2_pix));
		while (dst_height--)
		{
			p1_pix -= src_width;
			p2_pix -= dst_rounded_width;
			rowbltneg(p1_pix, p2_pix, dst_width, rop_masks);
		}
	}
	else
	{
		// Move block going down (increase y)
		DBG_LOG(3,0,("bitblt fwd\n"));
		while (dst_height--)
		{
			rowbltpos(p1_pix, p2_pix, dst_width, rop_masks);
			p1_pix += src_width;
			p2_pix += dst_rounded_width;
		}
	}

	return true;
}

void hp1ll3_device::rowbltpos(unsigned p1_pix, unsigned p2_pix, int width, const uint16_t masks[])
{
	// p1_pix and p2_pix point to the leftmost pixel of the row
	while (width > 0)
	{
		unsigned p1_word = p1_pix / WS;
		unsigned p1_bit = p1_pix % WS;
		// Get src pixels and align to MSB
		uint16_t new_pix = rd_video(p1_word) << p1_bit;
		if (p1_bit)
			new_pix |= rd_video(p1_word + 1) >> (WS - p1_bit);

		unsigned p2_word = p2_pix / WS;
		unsigned p2_bit = p2_pix % WS;
		uint16_t old_pix = rd_video(p2_word);
		unsigned w = p2_bit + width;
		uint16_t glob_mask = ~0;
		if (p2_bit)
		{
			// Left end
			new_pix >>= p2_bit;
			glob_mask >>= p2_bit;
		}
		if (w < WS)
		{
			// Right end
			glob_mask &= ~0 << (WS - w);
		}
		rmw_rop(p2_word, new_pix, glob_mask, masks);
		DBG_LOG(3,0,("rowbltpos %04x %04x %04x>%04x %u %u %u\n", old_pix, new_pix, glob_mask, rd_video(p2_word), p1_pix, p2_pix, width));
		w = WS - p2_bit;
		p1_pix += w;
		p2_pix += w;
		width -= w;
	}
}

void hp1ll3_device::rowbltneg(unsigned p1_pix, unsigned p2_pix, int width, const uint16_t masks[])
{
	// p1_pix and p2_pix point to the rightmost pixel of the row
	while (width > 0)
	{
		unsigned p1_word = p1_pix / WS;
		unsigned p1_bit = p1_pix % WS;
		// Get src pixels and align to LSB
		uint16_t new_pix = rd_video(p1_word) >> (WS - 1 - p1_bit);
		if (p1_bit != (WS - 1))
			new_pix |= rd_video(p1_word - 1) << (p1_bit + 1);

		unsigned p2_word = p2_pix / WS;
		unsigned p2_bit = p2_pix % WS;
		uint16_t old_pix = rd_video(p2_word);
		int w = p2_bit - width;
		uint16_t glob_mask = ~0;
		if (p2_bit != (WS - 1))
		{
			// Right end
			new_pix <<= (WS - 1 - p2_bit);
			glob_mask <<= (WS - 1 - p2_bit);
		}
		if (w >= 0)
		{
			// Left end
			uint16_t tmp = ~0;
			glob_mask &= tmp >> (w + 1);
		}
		rmw_rop(p2_word, new_pix, glob_mask, masks);
		DBG_LOG(3,0,("rowbltneg %04x %04x %04x>%04x %u %u %u\n", old_pix, new_pix, glob_mask, rd_video(p2_word), p1_pix, p2_pix, width));
		w = p2_bit + 1;
		p1_pix -= w;
		p2_pix -= w;
		width -= w;
	}
}

void hp1ll3_device::fill(const Rectangle &fill_rect, uint16_t pattern_no)
{
	uint16_t pattern_addr = get_pattern_addr(pattern_no);

	Point src_p{ 0, 0 };
	// Align to 16x16 tiles in absolute coordinates
	uint16_t start_x = fill_rect.origin.x & ~0xf;
	Rectangle dst_rect{{ start_x, uint16_t(fill_rect.origin.y & ~0xf) }, { 16, 16 }};

	// Iterate over vertical span
	while (bitblt(pattern_addr, 16, 16, src_p, fill_rect, dst_rect, m_rr))
	{
		// Iterate over horizontal span
		do {
			dst_rect.origin.x += 16;
		} while (bitblt(pattern_addr, 16, 16, src_p, fill_rect, dst_rect, m_rr));
		dst_rect.origin.x = start_x;
		dst_rect.origin.y += 16;
	}
}

uint16_t hp1ll3_device::get_pattern_addr(uint16_t pattern_no) const
{
	return m_dad + pattern_no * 16;
}

void hp1ll3_device::draw_cursor()
{
	if (m_enable_cursor)
	{
		Rectangle dst{{ uint16_t(m_cursor_x + m_cursor_offset), m_cursor_y }, { 16, 16 }};
		bitblt(get_pattern_addr(m_cursor_pattern), 16, 16, Point{ 0, 0 }, get_screen(), dst, RR_XOR, false);
	}
}

void hp1ll3_device::draw_sprite()
{
	if (m_enable_sprite)
	{
		Rectangle dst{{ m_sprite_x, m_sprite_y }, { 16, 16 }};
		bitblt(get_pattern_addr(m_sprite_pattern), 16, 16, Point{ 0, 0 }, get_screen(), dst, RR_XOR, false);
	}
}

void hp1ll3_device::draw_cursor_sprite()
{
	draw_cursor();
	draw_sprite();
}

void hp1ll3_device::set_pen_pos(Point p)
{
	if (p.x != m_cursor_x || p.y != m_cursor_y)
	{
		draw_cursor();
		m_cursor_x = p.x;
		m_cursor_y = p.y;
		draw_cursor();
	}
}

void hp1ll3_device::set_sprite_pos(Point p)
{
	if (p.x != m_sprite_x || p.y != m_sprite_y)
	{
		draw_sprite();
		m_sprite_x = p.x;
		m_sprite_y = p.y;
		draw_sprite();
	}
}

void hp1ll3_device::disable_cursor()
{
	draw_cursor();
	m_enable_cursor = false;
}

void hp1ll3_device::disable_sprite()
{
	draw_sprite();
	m_enable_sprite = false;
}

hp1ll3_device::Rectangle hp1ll3_device::get_window() const
{
	return Rectangle{{ m_window.org_x, m_window.org_y }, { m_window.width, m_window.height }};
}

hp1ll3_device::Rectangle hp1ll3_device::get_screen() const
{
	return Rectangle{{ 0, 0 }, { uint16_t(m_horiz_pix_total), uint16_t(m_vert_pix_total) }};
}

void hp1ll3_device::get_hv_timing(bool vertical, unsigned& total, unsigned& active) const
{
	unsigned idx = vertical ? CONF_VER_0 : CONF_HOR_0;
	// Look for active part (zone 1) & compute total time
	total = 0;
	active = 0;
	for (unsigned i = 0; i < 4; ++i)
	{
		unsigned cnt = (m_conf[idx + i] & HV_CNT_MASK) + 1;
		total += cnt;
		if ((m_conf[idx + i] & HV_ZONE_MASK) == HV_ZONE_1)
		{
			active = cnt;
			// It seems that if a zone 2 comes immediately before a zone 1, it's
			// counted in the active part
			unsigned idx_1 = (i + 3) % 4 + idx;
			if ((m_conf[idx_1] & HV_ZONE_MASK) == HV_ZONE_2)
				active += (m_conf[idx_1] & HV_CNT_MASK) + 1;
		}
	}
	DBG_LOG(2, 0, ("H/V=%d total=%u active=%u\n", vertical, total, active));
}

void hp1ll3_device::apply_conf()
{
	unsigned h_total;
	unsigned h_active;
	unsigned v_total;
	unsigned v_active;

	get_hv_timing(false, h_total, h_active);
	get_hv_timing(true, v_total, v_active);

	// Dot clock is 4 times the GPU clock, H timings are in units of half the GPU clock
	// I suspect that the GPU has a kind of "double clock" mode that is enabled by option bits in the last conf word.
	// Without clock doubling, the HP-9808A video has a strange frame rate (~32Hz).
	h_total *= 8;
	h_active *= 8;

	attotime frame_rate{clocks_to_attotime(h_total * v_total)};
	screen().configure(h_total, v_total, rectangle{0, int32_t(h_active - 1), 0, int32_t(v_active - 1)}, frame_rate.attoseconds() / 4);

	m_horiz_pix_total = h_active;
	m_vert_pix_total = v_active;
}

uint32_t hp1ll3_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_enable_video)
	{
		bitmap.fill(rgb_t::black());
		return 0;
	}

	for (int y = 0; y < m_vert_pix_total; y++)
	{
		int const offset = m_sad + y*m_conf[CONF_WPL];
		uint16_t *p = &m_bitmap.pix16(y);

		for (int x = offset; x < offset + m_horiz_pix_total / 16; x++)
		{
			uint16_t const gfx = m_videoram[x];

			for (int i = 15; i >= 0; i--)
			{
				*p++ = BIT(gfx, i);
			}
		}
	}

	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}

//-------------------------------------------------
//  read - register read
//-------------------------------------------------

/*
 *  offset 0: CSR
 *
 *  bit 0   gpu is busy
 *  bit 1   data is ready
 *  bit 3   vert blank time
 *  bit 7   out of window
 *
 *  offset 2: data
 */

READ8_MEMBER( hp1ll3_device::read )
{
	uint8_t data = 0;

	if (offset == 0)
	{
		data = m_busy ? 1 : 0;
		data |= 2;
		data |= (screen().vblank() ? 8 : 0);
	}
	else
	{
		if (!(m_rd_ptr & 1))
		{
			switch (m_command)
			{
			case RDMEM:
				m_io_word = rd_video(m_memory_ptr++);
				break;

			case RDWIN:
				{
					uint16_t addr = get_pix_addr(m_rw_win_x, m_rw_win_y);
					unsigned bit = m_rw_win_x % WS;
					int width = m_window.width + m_window.org_x - m_rw_win_x;
					if (width <= 0)
					{
						m_command = NOP;
					}
					else
					{
						unsigned discarded_bits = 0;
						// Insert "m_rw_win_off" dummy bits in the first word of each line
						if (m_rw_win_x ==  m_window.org_x)
							discarded_bits = m_rw_win_off;
						unsigned w = std::min(WS - discarded_bits, unsigned(width));

						m_io_word = (rd_video(addr) << bit) >> discarded_bits;
						if ((bit + w) > WS)
							m_io_word |= rd_video(addr + 1) >> (WS - bit + discarded_bits);

						m_io_word &= ~0 << (WS - w - discarded_bits);
						m_rw_win_x += w;
						if (m_rw_win_x >= (m_window.width + m_window.org_x))
						{
							m_rw_win_x = m_window.org_x;
							m_rw_win_y++;
							if (m_rw_win_y >= (m_window.height + m_window.org_y))
								m_command = NOP;
						}
					}
				}
				break;

			case RDP:
				if (m_rd_ptr == 0)
					m_io_word = m_cursor_x;
				else if (m_rd_ptr == 2)
					m_io_word = m_cursor_y;
				break;

			case RDSP:
				if (m_rd_ptr == 0)
					m_io_word = m_sprite_x;
				else if (m_rd_ptr == 2)
					m_io_word = m_sprite_y;
				break;

			case RDWINPARM:
				switch (m_rd_ptr)
				{
				case 0:
					m_io_word = m_window.org_x;
					break;
				case 2:
					m_io_word = m_window.org_y;
					break;
				case 4:
					m_io_word = m_window.width;
					break;
				case 6:
					m_io_word = m_window.height;
					break;
				default:
					break;
				}
				break;

			case ID:
				/*
				 * 'diagb' ROM accepts either of these ID values
				 * 0x3003, 0x4004, 0x5005, 0x6006
				 */
				m_io_word = 0x4004;
				break;
			}
			data = uint8_t(m_io_word >> 8);
		} else
		{
			data = m_io_word & 0xff;
		}
		m_rd_ptr++;
	}

	DBG_LOG(1,"HPGPU", ("R @ %d == %02x\n", offset, data));

	return data;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

WRITE8_MEMBER( hp1ll3_device::write )
{
	DBG_LOG(1,"HPGPU", ("W @ %d <- %02x\n", offset, data));

	if (offset == 0)
	{
		command(data);
	}
	else
	{
		if (!(m_wr_ptr & 1))
		{
			m_io_word = uint16_t(data) << 8;
		}
		else
		{
			m_io_word |= data;
			switch (m_command)
			{
			case CONF:
				m_conf[m_wr_ptr >> 1] = m_io_word;
				DBG_LOG(1,"HPGPU",("CONF data word %d received: %04X\n", m_wr_ptr >> 1, m_io_word));
				if ((m_wr_ptr >> 1) == 10)
				{
					apply_conf();
					m_wr_ptr = -1;
					m_command = NOP;
				}
				break;

			case WRMEM:
				wr_video(m_memory_ptr++, m_io_word);
				break;

			case WRWIN:
				{
					uint16_t rop_masks[4];
					get_rop_masks(m_rr, rop_masks);

					uint16_t addr = get_pix_addr(m_rw_win_x, m_rw_win_y);
					uint16_t glob_mask = ~0;
					unsigned bit = m_rw_win_x % WS;
					int width = m_window.width + m_window.org_x - m_rw_win_x;
					if (width <= 0)
						m_command = NOP;
					else
					{
						unsigned discarded_bits = 0;
						// Remove "m_rw_win_off" bits from the first word of each line
						if (m_rw_win_x ==  m_window.org_x)
							discarded_bits = m_rw_win_off;
						unsigned w = std::min(WS - discarded_bits, unsigned(width));

						glob_mask >>= bit;
						if ((bit + w) < WS)
							glob_mask &= ~0 << (WS - (bit + w));

						rmw_rop(addr, (m_io_word << discarded_bits) >> bit, glob_mask, rop_masks);
						DBG_LOG(3, 0, ("WRWIN (%u,%u) %d %04x %04x->%04x\n", m_rw_win_x, m_rw_win_y, width, (m_io_word << discarded_bits) >> bit, glob_mask, rd_video(addr)));
						if ((bit + w) > WS)
						{
							unsigned pad = WS - bit + discarded_bits;
							glob_mask = ~0 << (2 * WS - bit - w);
							rmw_rop(addr + 1, uint16_t(m_io_word << pad), glob_mask, rop_masks);
							DBG_LOG(3, 0, ("WRWIN %u %04x %04x->%04x\n", pad, m_io_word << pad, glob_mask, rd_video(addr + 1)));
						}
						m_rw_win_x += w;
						if (m_rw_win_x >= (m_window.width + m_window.org_x))
						{
							m_rw_win_x = m_window.org_x;
							m_rw_win_y++;
							if (m_rw_win_y >= (m_window.height + m_window.org_y))
								m_command = NOP;
						}
					}
				}
				break;

			default:
				if (m_wr_ptr <= 4)
					m_input[m_wr_ptr / 2] = m_io_word;
				DBG_LOG(2,"HPGPU",("wrote %02x at %d, input buffer is %04X %04X\n", data, m_wr_ptr, m_input[0], m_input[1]));
			}
		}
		m_wr_ptr++;
	}
}


void hp1ll3_device::command(int command)
{
	switch (command)
	{

	// type 0 commands -- no data

	case NOP:
		DBG_LOG(2,"HPGPU",("command: NOP [%d, 0x%x]\n", command, command));
		switch (m_command)
		{
		case RDMEM:
			DBG_LOG(1,"HPGPU",("RDMEM of %d words at %04X complete\n", m_memory_ptr - m_input[0], m_input[0]));
			break;

		case WRMEM:
			DBG_LOG(1,"HPGPU",("WRMEM of %d words to %04X complete\n", m_memory_ptr - m_input[0], m_input[0]));
			break;
		}
		break;

	case DISVID:
		DBG_LOG(2,"HPGPU",("command: DISVID [%d, 0x%x]\n", command, command));
		m_enable_video = false;
		break;

	case ENVID:
		DBG_LOG(2,"HPGPU",("command: ENVID [%d, 0x%x]\n", command, command));
		DBG_LOG(1,"HPGPU",("enable video; SAD %04x FAD %04x DAD %04x ORG %04x UDL %04x RR %04x\n",
			m_sad, m_fad, m_dad, m_org, m_udl, m_rr));
		m_enable_video = true;
		break;

	case DISSP:
		DBG_LOG(2,"HPGPU",("command: DISSP [%d, 0x%x]\n", command, command));
		disable_sprite();
		break;

	case ENSP:
		DBG_LOG(2,"HPGPU",("command: ENSP [%d, 0x%x]\n", command, command));
		draw_sprite();
		m_sprite_pattern = m_input[0];
		m_enable_sprite = true;
		draw_sprite();
		DBG_LOG(1,"HPGPU",("enable sprite; cursor %d,%d sprite %d,%d\n", m_cursor_x, m_cursor_y, m_sprite_x, m_sprite_y));
		break;

	case DISCURS:
		DBG_LOG(2,"HPGPU",("command: DISCURS [%d, 0x%x]\n", command, command));
		disable_cursor();
		break;

	// type 1 commands -- 1 word of data expected in the buffer

	// start of screen memory
	case WRSAD:
		DBG_LOG(2,"HPGPU",("command: WRSAD [%d, 0x%x] (0x%04x)\n", command, command, m_input[0]));
		m_sad = m_input[0];
		break;

	// start of font memory
	case WRFAD:
		DBG_LOG(2,"HPGPU",("command: WRFAD [%d, 0x%x] (0x%04x)\n", command, command, m_input[0]));
		m_fad = m_input[0];
		break;

	// ?? used by diagnostic ROM
	case 6:
		DBG_LOG(1,"HPGPU",("command: 6\n"));
		break;

	// start of data area
	case WRDAD:
		DBG_LOG(2,"HPGPU",("command: WRDAD [%d, 0x%x] (0x%04x)\n", command, command, m_input[0]));
		m_dad = m_input[0];
		break;

	// ??
	case WRORG:
		DBG_LOG(2,"HPGPU",("command: WRORG [%d, 0x%x] (0x%04x)\n", command, command, m_input[0]));
		m_org = m_input[0];
		break;

	// set replacement rule (raster op)
	case WRRR:
		DBG_LOG(2,"HPGPU",("command: WRRR [%d, 0x%x] (0x%04x)\n", command, command, m_input[0]));
		m_rr = m_input[0];
		break;

	// set user-defined line pattern
	case WRUDL:
		DBG_LOG(2,"HPGPU",("command: WRUDL [%d, 0x%x] (0x%04x)\n", command, command, m_input[0]));
		m_udl = m_input[0];
		break;

	// area fill
	case FILL:
		DBG_LOG(2,"HPGPU",("command: FILL [%d, 0x%x] (0x%04x) from (%d,%d) size (%d,%d) rop 0x%x\n",
						   command, command, m_input[0],
						   m_window.org_x, m_window.org_y, m_window.width, m_window.height, m_rr));
		draw_cursor_sprite();
		fill(get_window(), m_input[0]);
		draw_cursor_sprite();
		break;

	case LABEL:
		{
			DBG_LOG(2,"HPGPU",("command: LABEL [%d, 0x%x] (0x%04x, '%c') at %d,%d\n", command, command, m_input[0],
				(m_input[0]<32||m_input[0]>127) ? ' ' : m_input[0], m_cursor_x, m_cursor_y));
			int const c = m_input[0] & 255;
			int const w = (c & 1) ?
					(rd_video(m_fad + 2 + (c>>1)) & 255) :
					(rd_video(m_fad + 2 + (c>>1)) >> 8);
			label(c, w);
		}
		break;

		// Write window
	case WRWIN:
		// Read window
	case RDWIN:
		DBG_LOG(2,"HPGPU",("command: %s [%d, 0x%x] offset=%u size(%d,%d)\n",
						   command == RDWIN ? "RDWIN":"WRWIN", command, command, m_input[0], m_window.width, m_window.height));
		m_rw_win_x = m_window.org_x;
		m_rw_win_y = m_window.org_y;
		m_rw_win_off = m_input[0] % WS;
		disable_cursor();
		disable_sprite();
		break;

	// type 2 commands -- 2 words of data expected in the buffer

	case DRAWPX:
		{
			DBG_LOG(2,"HPGPU",("command: DRAWPX [%d, 0x%x] (%d, %d)\n", command, command, m_input[0], m_input[1]));
			uint16_t rop_masks[4];
			get_rop_masks(m_rr, rop_masks);
			point(m_input[0], m_input[1], true, rop_masks);
		}
		break;

	// set window size
	case WRWINSIZ:
		DBG_LOG(2,"HPGPU",("command: WRWINSIZ [%d, 0x%x] (%d, %d)\n", command, command, m_input[0], m_input[1]));
		m_window.width = m_input[0];
		m_window.height = m_input[1];
		break;

	// set window origin
	case WRWINORG:
		DBG_LOG(2,"HPGPU",("command: WRWINORG [%d, 0x%x] (%d, %d)\n", command, command, m_input[0], m_input[1]));
		m_window.org_x = m_input[0];
		m_window.org_y = m_input[1];
		break;

		// Copy
	case COPY:
		{
			DBG_LOG(2,"HPGPU",("command: COPY [%d, 0x%x] (%d, %d)\n", command, command, m_input[0], m_input[1]));
			draw_cursor_sprite();
			unsigned rounded_width = m_conf[CONF_WPL] * WS;
			Rectangle dst_rect{{ m_input[0], m_input[1] }, { m_window.width, m_window.height }};
			// COPY apparently doesn't clip anything
			bitblt(m_org, rounded_width, 0xffff, Point{ m_window.org_x, m_window.org_y },
				   Rectangle{{ 0, 0 }, { uint16_t(rounded_width), 0xffff }}, dst_rect, m_rr);
			draw_cursor_sprite();
		}
		break;

	// move pointer absolute
	case MOVEP:
		DBG_LOG(2,"HPGPU",("command: MOVEP [%d, 0x%x] (%d, %d)\n", command, command, m_input[0], m_input[1]));
		set_pen_pos(Point{ m_input[0], m_input[1] });
		m_saved_x = m_cursor_x;
		break;

	// move pointer incremental
	case IMOVEP:
		DBG_LOG(2,"HPGPU",("command: IMOVEP [%d, 0x%x] (%d, %d)\n", command, command, m_input[0], m_input[1]));
		set_pen_pos(Point{ uint16_t(m_cursor_x + m_input[0]), uint16_t(m_cursor_y + m_input[1]) });
		m_saved_x = m_cursor_x;
		break;

		// Scroll UP
	case SCROLUP:
		{
			DBG_LOG(2,"HPGPU",("command: SCROLUP [%d, 0x%x] (%d, %d)\n", command, command, m_input[0], m_input[1]));
			draw_cursor_sprite();
			Point src_p{ m_window.org_x, uint16_t(m_window.org_y + m_input[1]) };
			Rectangle dst_rect = get_window();
			dst_rect.size.y -= m_input[1];
			unsigned rounded_width = m_conf[CONF_WPL] * WS;
			bitblt(m_org, rounded_width, m_vert_pix_total, src_p, get_window(), dst_rect, m_rr);
			dst_rect.origin.y += dst_rect.size.y;
			dst_rect.size.y = m_input[1];
			fill(dst_rect, m_input[0]);
			draw_cursor_sprite();
		}
		break;

		// Scroll DOWN
	case SCROLDN:
		{
			DBG_LOG(2,"HPGPU",("command: SCROLDN [%d, 0x%x] (%d, %d)\n", command, command, m_input[0], m_input[1]));
			draw_cursor_sprite();
			Point src_p{ m_window.org_x, m_window.org_y };
			Rectangle dst_rect = get_window();
			dst_rect.origin.y += m_input[1];
			dst_rect.size.y -= m_input[1];
			unsigned rounded_width = m_conf[CONF_WPL] * WS;
			bitblt(m_org, rounded_width, m_vert_pix_total, src_p, get_window(), dst_rect, m_rr);
			dst_rect.origin.y = m_window.org_y;
			dst_rect.size.y = m_input[1];
			fill(dst_rect, m_input[0]);
			draw_cursor_sprite();
		}
		break;

	case ENCURS:
		DBG_LOG(2,"HPGPU",("command: ENCURS [%d, 0x%x]\n", command, command));
		draw_cursor();
		m_cursor_pattern = m_input[0];
		m_cursor_offset = m_input[1];
		m_enable_cursor = true;
		draw_cursor();
		DBG_LOG(1,"HPGPU",("enable cursor; cursor %d,%d sprite %d,%d\n", m_cursor_x, m_cursor_y, m_sprite_x, m_sprite_y));
		break;

	// carriage return, line feed
	case CRLFx:
		{
			DBG_LOG(2,"HPGPU",("command: CRLF [%d, 0x%x]\n", command, command));

			uint16_t font_data;
			uint16_t font_height;
			get_font(font_data, font_height);

			set_pen_pos(Point{ m_saved_x, uint16_t(m_cursor_y + font_height) });
		}
		break;

	// move sprite absolute
	case MOVESP:
		DBG_LOG(2,"HPGPU",("command: MOVESP [%d, 0x%x] (%d, %d)\n", command, command, m_input[0], m_input[1]));
		set_sprite_pos(Point{ m_input[0], m_input[1] });
		break;

	// draw to ...
	case DRAWP:
		DBG_LOG(2,"HPGPU",("command: DRAWP [%d, 0x%x] (%d, %d) to (%d, %d)\n",
			command, command, m_cursor_x, m_cursor_y, m_input[0], m_input[1]));
		line(m_cursor_x, m_cursor_y, m_input[0], m_input[1]);
		set_pen_pos(Point{ m_input[0], m_input[1] });
		break;

	// type 3 command -- CONF -- accept configuration parameters (11 words)

	case CONF:
		break;

	// type 4 commands -- like type 1 plus data is read or written after command, terminated by NOP

	case RDMEM:
	case WRMEM:
		DBG_LOG(2,"HPGPU",("command: %s [%d, 0x%x] (0x%04x)\n",
			command == RDMEM?"RDMEM":"WRMEM", command, command, m_input[0]));
		m_memory_ptr = m_input[0]; // memory is word-addressable
		disable_cursor();
		disable_sprite();
		break;

	// type 3 read commands
	case RDP:
		DBG_LOG(2,"HPGPU",("command: RDP [%d, 0x%x]\n", command, command));
		break;

	case RDSP:
		DBG_LOG(2,"HPGPU",("command: RDSP [%d, 0x%x]\n", command, command));
		break;

	case RDWINPARM:
		DBG_LOG(2,"HPGPU",("command: RDWINPARM [%d, 0x%x]\n", command, command));
		break;

	case ID:
		DBG_LOG(2,"HPGPU",("command: ID [%d, 0x%x]\n", command, command));
		break;

	default:
		DBG_LOG(1,"HPGPU",("command: UNKNOWN [%d, 0x%x]\n", command, command));
		break;
	}

	m_rd_ptr = 0;
	m_wr_ptr = 0;
	m_command = command;
}

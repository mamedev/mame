// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
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
    . proper cursor and mouse pointers [cursor can be offset from the pen location]
    + variable width fonts [?? placed relative to current window]
    + basic lines
    - patterned lines
    . bit blits & scroll
    . meaning of WRRR bits
    . meaning of CONF data [+ autoconfiguration]
    - interrupt generation
    - realistic timing?
    - &c.
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
 * 3 -- write command, then 12 words of data (= CONF only?)
 * 4 -- write 1 word of data, then command, then write X words of data, then write NOP
 *
 * (read)
 *
 * 3 -- ???
 * 4 -- write 1 word of data, then command, then read X words of data, then write NOP
 */
#define NOP         0   // type 0
#define CONF        2   // type 3, configure GPU (screen size, timings...).  11 words of data.
#define DISVID      3   // type 0, disable video
#define ENVID       4   // type 0, enable video
#define WRMEM       7   // type 4, write GPU memory at offset, terminate by NOP
#define RDMEM       8   // type 4, read GPU memory from offset, terminate by NOP
#define WRSAD       9   // type 1, set screen area start address
#define WRORG       10  // type 1, set ???
#define WRDAD       11  // type 1, set data area start address (16x16 area fill, sprite and cursor)
#define WRRR        12  // type 1, set replacement rule (rasterop)
#define MOVEP       13  // type 2, move pointer
#define IMOVEP      14
#define DRAWP       15  // type 2, draw line
#define IDRAWP      16
#define RDP         17
#define WRUDL       18  // type 1, set user-defined line pattern (16-bit)
#define WRWINSIZ    19  // type 2, set ???
#define WRWINORG    20  // type 2, set ???
#define COPY        21  // type 2
#define FILL        22  // type 1, fill area
#define FRAME       23  // type _, draw rectangle
#define SCROLUP     24  // type 2
#define SCROLDN     25  // type 2
#define SCROLLF     26  // type 2
#define SCROLRT     27  // type 2
#define RDWIN       28  // type 1
#define WRWIN       29  // type 1
#define RDWINPARM   30
#define CR          31
#define CRLFx       32
#define LABEL       36  // type 1, draw text
#define ENSP        38  // type 0, enable sprite
#define DISSP       39  // type 0, disable sprite
#define MOVESP      40  // type 2, move sprite
#define IMOVESP     41
#define RDSP        42
#define DRAWPX      43  // type _, draw single pixel
#define WRFAD       44  // type 1, set font area start address
#define ENCURS      45  // type 0
#define DISCURS     46  // type 0
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

#define WS 16 // bits in a word


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


// enough for 9807a and 9808a both.
#define HPGPU_VRAM_SIZE (16384 * 4)


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

	m_cursor.allocate(16, 16);
	m_sprite.allocate(16, 16);

	m_videoram = std::make_unique<uint16_t[]>(HPGPU_VRAM_SIZE*2);   // x2 size to make WRWIN/RDWIN easier

	m_horiz_pix_total = visarea.max_x + 1;
	m_vert_pix_total = visarea.max_y + 1;
}

void hp1ll3_device::device_reset()
{
	m_input_ptr = m_command = 0;
	m_sad = m_fad = m_dad = m_org = m_rr = m_udl = 0;
	m_enable_video = m_enable_cursor = m_enable_sprite = m_busy = false;
}


inline void hp1ll3_device::point(int x, int y, int px)
{
	uint16_t offset = m_sad;

	offset += y*(m_horiz_pix_total/16) + (x >> 4);

	if (px)
		m_videoram[offset] |=  (1 << (15-(x%16)));
	else
		m_videoram[offset] &= ~(1 << (15-(x%16)));
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

	c1=0;
	incy=1;

	if(x2>x1)
		dx = x2 - x1;
	else
		dx = x1 - x2;

	if(y2>y1)
		dy = y2 - y1;
	else
		dy = y1 - y2;

	if( dy > dx )
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

	if( x1 > x2 )
	{
		t = y2;
		y2 = y1;
		y1 = t;

		t = x1;
		x1 = x2;
		x2 = t;
	}

	horiz = dy<<1;
	diago = ( dy - dx )<<1;
	e = ( dy<<1 ) - dx;

	if( y1 <= y2 )
		incy = 1;
	else
		incy = -1;

	x = x1;
	y = y1;

	if(c1)
	{
		do
		{
			point(y,x,m_udl);

			if( e > 0 )
			{
				y = y + incy;
				e = e + diago;
			}
			else
			{
				e = e + horiz;
			}

			x++;

		} while( x <= x2 );
	}
	else
	{
		do
		{
			point(x,y,m_udl);

			if( e > 0 )
			{
				y = y + incy;
				e = e + diago;
			}
			else
			{
				e = e + horiz;
			}

			x++;

		} while( x <= x2 );
	}

	return;
}

void hp1ll3_device::fill(int org_x, int org_y, int w, int h, int arg)
{
	uint16_t gfx, offset, mask, max_x = org_x + w;

	if (m_enable_cursor) bitblt(m_cursor_x, m_cursor_y, m_dad, 16, 16, RR_XOR);
	if (m_enable_sprite) bitblt(m_sprite_x, m_sprite_y, m_dad + 16, 16, 16, RR_XOR);

	for (int y = org_y; y < org_y + h; y++) {
		gfx = m_videoram[m_dad + (0x10 * arg) + y%16];
		if (m_rr == RR_COPYINVERTED) gfx ^= 0xffff;
		for (int x = org_x; x < max_x;) {
			mask = 0xffff;
			offset = m_sad + m_org + y * (m_horiz_pix_total/16) + (x >> 4);

			if (offset >= m_sad + (m_vert_pix_total * m_horiz_pix_total/WS))
				DBG_LOG(0,"HPGPU",("buffer overflow in FILL: %04x (%d, %d)\n", offset, 16*x, y));

			// clipping
			if (x == org_x) {
				mask >>= (org_x % WS);
				x += (WS - (org_x % WS));
			} else {
				if ((max_x - x) < WS) {
					mask &= ~((1 << (WS - (max_x % WS))) - 1);
				}
				x += WS;
			}
			m_videoram[offset] &= ~mask;
			m_videoram[offset] |= gfx & mask;
		}
	}

	if (m_enable_cursor) bitblt(m_cursor_x, m_cursor_y, m_dad, 16, 16, RR_XOR);
	if (m_enable_sprite) bitblt(m_sprite_x, m_sprite_y, m_dad + 16, 16, 16, RR_XOR);
}

// sprite drawing -- source is 16x16 px max, no window clipping
void hp1ll3_device::bitblt(int dstx, int dsty, uint16_t srcaddr, int width, int height, int op)
{
	uint16_t gfx, offset, mask;
	int max_x, max_y;

	max_x = dstx + width;
	if (max_x >= m_horiz_pix_total)
		max_x = m_horiz_pix_total;

	max_y = dsty + height;
	if (max_y >= m_vert_pix_total)
		max_y = m_vert_pix_total;

	for (int y = dsty; y < max_y; y++) {
		mask = 0xffff;
		gfx = m_videoram[srcaddr + y - dsty] & mask;
		offset = m_sad + y * (m_horiz_pix_total/16) + (dstx >> 4);

		if (offset >= m_sad + (m_vert_pix_total * m_horiz_pix_total/16))
			DBG_LOG(0,"HPGPU",("buffer overflow in bitblt: %04x (%d, %d)\n", offset, 16*dstx, y));

		// are we crossing word boundary?
		if (dstx % 16) {
			if (op == RR_XOR)
			{
				m_videoram[offset    ] ^= gfx >> (dstx % 16);
				m_videoram[offset + 1] ^= gfx << (16 - (dstx % 16));
			}
		} else {
			if (op == RR_XOR)
			{
				m_videoram[offset] ^= (gfx & mask);
			}
		}
	}
}

void hp1ll3_device::label(uint8_t chr, int width)
{
	uint16_t x, gfx, bg = 0x3f, offset, font = m_fontdata + chr * 16;
	int max_y = m_cursor_y + m_fontheight;

	if (max_y >= m_window.org_y + m_window.height)
		max_y = m_window.org_y + m_window.height - 1 - m_cursor_y;
	else
		max_y -= m_cursor_y;

	if (m_enable_cursor) bitblt(m_cursor_x, m_cursor_y, m_dad, 16, 16, RR_XOR);
	if (m_enable_sprite) bitblt(m_sprite_x, m_sprite_y, m_dad + 16, 16, 16, RR_XOR);

	for (int y = 0; y < max_y; y++) {
		x = m_cursor_x;
		offset = m_sad + (m_cursor_y + y) * (m_horiz_pix_total/16) + (x >> 4);
		gfx = m_videoram[font + y] >> (16 - width);

		if (offset >= m_sad + (m_vert_pix_total * m_horiz_pix_total/16))
			DBG_LOG(0,"HPGPU",("buffer overflow in LABEL: %04x (%d, %d)\n", offset, 16*x, y));

		// are we crossing word boundary?
		if ((x % 16) > (16 - width)) {
			if (m_rr == RR_COPYINVERTED) {
				m_videoram[offset    ] |= bg  >> ((x % 16) - (16 - width));
				m_videoram[offset + 1] |= bg  << (16 - ((x + width) % 16));
			} else {
				m_videoram[offset    ] &= ~(bg>> ((x % 16) - (16 - width)));
				m_videoram[offset + 1] &= ~(bg<< (16 - ((x + width) % 16)));
			}
			m_videoram[offset    ] ^= gfx >> ((x % 16) - (16 - width));
			m_videoram[offset + 1] ^= gfx << (16 - ((x + width) % 16));
		} else {
			if (m_rr == RR_COPYINVERTED) {
				m_videoram[offset    ] |= bg  << ((16 - width) - (x % 16));
			} else {
				m_videoram[offset    ] &= ~(bg<< ((16 - width) - (x % 16)));
			}
			m_videoram[offset    ] ^= gfx << ((16 - width) - (x % 16));
		}
	}

	m_cursor_x += width;
	if (m_enable_cursor) bitblt(m_cursor_x, m_cursor_y, m_dad, 16, 16, RR_XOR);
	if (m_enable_sprite) bitblt(m_sprite_x, m_sprite_y, m_dad + 16, 16, 16, RR_XOR);
}


uint32_t hp1ll3_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y, offset;
	uint16_t gfx, *p;

	if (!m_enable_video) {
		bitmap.fill(rgb_t::black());
		return 0;
	}

	// XXX last line is not actually drawn on real hw
	for (y = 0; y < m_vert_pix_total-1; y++) {
		offset = m_sad + y*(m_horiz_pix_total/16);
		p = &m_bitmap.pix16(y);

		for (x = offset; x < offset + m_horiz_pix_total/16; x++)
		{
			gfx = m_videoram[x];

			*p++ = BIT(gfx, 15);
			*p++ = BIT(gfx, 14);
			*p++ = BIT(gfx, 13);
			*p++ = BIT(gfx, 12);
			*p++ = BIT(gfx, 11);
			*p++ = BIT(gfx, 10);
			*p++ = BIT(gfx, 9);
			*p++ = BIT(gfx, 8);
			*p++ = BIT(gfx, 7);
			*p++ = BIT(gfx, 6);
			*p++ = BIT(gfx, 5);
			*p++ = BIT(gfx, 4);
			*p++ = BIT(gfx, 3);
			*p++ = BIT(gfx, 2);
			*p++ = BIT(gfx, 1);
			*p++ = BIT(gfx, 0);
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

	switch (offset)
	{
	case 0:
		data = m_busy ? 1 : 0;
		data |= 2;
		data |= (screen().vblank() ? 8 : 0);
		break;

	case 2:
		switch (m_command)
		{
		case RDMEM:
			if (m_memory_ptr < HPGPU_VRAM_SIZE*2) {
				if (m_memory_ptr & 1) {
					data = m_videoram[m_memory_ptr >> 1] & 0xff;
				} else {
					data = m_videoram[m_memory_ptr >> 1] >> 8;
				}
				m_memory_ptr++;
			}
			break;

		/*
		 * 'diagb' ROM accepts either of these ID values
		 * 0x3003, 0x4004, 0x5005, 0x6006
		 */
		case ID:
			data = 0x40;
			break;
		}
		break;

	case 3:
		if (m_command == ID) data = 0x04;
		break;
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

	switch (offset)
	{
	case 0:
		command(data);
		break;

	case 2:
	case 3:
		switch (m_command)
		{
		case CONF:
			if (m_conf_ptr < 2 * ARRAY_LENGTH(m_conf)) {
				if (m_conf_ptr & 1) {
					m_conf[m_conf_ptr >> 1] |= data;
					DBG_LOG(1,"HPGPU",("CONF data word %d received: %04X\n", m_conf_ptr >> 1, m_conf[m_conf_ptr >> 1]));
				} else {
					m_conf[m_conf_ptr >> 1] = data << 8;
				}
			} else {
				if (m_conf_ptr & 1) {
					m_input[0] |= data;
					DBG_LOG(1,"HPGPU",("unexpected CONF data word %d discarded: %04X\n", m_conf_ptr >> 1, m_input[0]));
				} else {
					m_input[0] = data << 8;
				}
			}
			m_conf_ptr++;
			break;

		case WRMEM:
			if (m_memory_ptr < HPGPU_VRAM_SIZE*2) {
				if (m_memory_ptr & 1) {
					m_videoram[m_memory_ptr >> 1] |= data;
				} else {
					m_videoram[m_memory_ptr >> 1] = data << 8;
				}
				m_memory_ptr++;
			}
			break;

		default:
			switch (m_input_ptr)
			{
			case 0:
				m_input[0] = data << 8;
				break;

			case 1:
				m_input[0] |= data;
				break;

			case 2:
				m_input[1] = data << 8;
				break;

			case 3:
				m_input[1] |= data;
				break;
			}
			DBG_LOG(2,"HPGPU",("wrote %02x at %d, input buffer is %04X %04X\n", data, m_input_ptr, m_input[0], m_input[1]));
			m_input_ptr++;
		}
	}
}


void hp1ll3_device::command(int command)
{
	int c, w;

	switch (command)
	{

	// type 0 commands -- no data

	case NOP:
		DBG_LOG(2,"HPGPU",("command: NOP [%d, 0x%x]\n", command, command));
		switch (m_command)
		{
		case RDMEM:
			DBG_LOG(1,"HPGPU",("RDMEM of %d words at %04X complete\n", (m_memory_ptr >> 1) - m_input[0], m_input[0]));
			break;

		case WRMEM:
			DBG_LOG(1,"HPGPU",("WRMEM of %d words to %04X complete\n", (m_memory_ptr >> 1) - m_input[0], m_input[0]));
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
		if (m_enable_sprite) {
			bitblt(m_sprite_x, m_sprite_y, m_dad + 16, 16, 16, RR_XOR);
			m_enable_sprite = false;
		}
		break;

	case ENSP:
		DBG_LOG(2,"HPGPU",("command: ENSP [%d, 0x%x]\n", command, command));
		if (m_enable_sprite) {
			bitblt(m_sprite_x, m_sprite_y, m_dad + 16, 16, 16, RR_XOR);
		}
		bitblt(m_sprite_x, m_sprite_y, m_dad + 16, 16, 16, RR_XOR);
		m_enable_sprite = true;
		DBG_LOG(1,"HPGPU",("enable sprite; cursor %d,%d sprite %d,%d\n", m_cursor_x, m_cursor_y, m_sprite_x, m_sprite_y));
		break;

	case DISCURS:
		DBG_LOG(2,"HPGPU",("command: DISCURS [%d, 0x%x]\n", command, command));
		if (m_enable_cursor) {
			bitblt(m_cursor_x, m_cursor_y, m_dad, 16, 16, RR_XOR);
			m_enable_cursor = false;
		}
		break;

	case ENCURS:
		DBG_LOG(2,"HPGPU",("command: ENCURS [%d, 0x%x]\n", command, command));
		if (m_enable_cursor) {
			bitblt(m_cursor_x, m_cursor_y, m_dad, 16, 16, RR_XOR);
		}
		bitblt(m_cursor_x, m_cursor_y, m_dad, 16, 16, RR_XOR);
		m_enable_cursor = true;
		DBG_LOG(1,"HPGPU",("enable cursor; cursor %d,%d sprite %d,%d\n", m_cursor_x, m_cursor_y, m_sprite_x, m_sprite_y));
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

	// ?? used by diagnostic ROM
	case 6:
		m_fontheight = m_videoram[m_fad];
		m_fontdata = m_fad + m_videoram[m_fad + 1] + 2;
		DBG_LOG(1,"HPGPU",("font data set: FAD %04X header %d bitmaps %04X height %d\n",
			m_fad, m_videoram[m_fad + 1], m_fontdata, m_fontheight));
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
		fill(m_window.org_x, m_window.org_y, m_window.width, m_window.height, m_input[0]);
		break;

	case LABEL:
		DBG_LOG(2,"HPGPU",("command: LABEL [%d, 0x%x] (0x%04x, '%c') at %d,%d\n", command, command, m_input[0],
			(m_input[0]<32||m_input[0]>127) ? ' ' : m_input[0], m_cursor_x, m_cursor_y));
		c = m_input[0] & 255;
		w = (c & 1) ?
			(m_videoram[m_fad + 2 + (c>>1)] & 255) :
			(m_videoram[m_fad + 2 + (c>>1)] >> 8);
		label(c, w);
		break;

	// type 2 commands -- 2 words of data expected in the buffer

	case DRAWPX:
		DBG_LOG(2,"HPGPU",("command: DRAWPX [%d, 0x%x] (%d, %d)\n", command, command, m_input[0], m_input[1]));
		point(m_input[0], m_input[1],(m_rr != RR_COPYINVERTED));
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

	// move pointer absolute
	case MOVEP:
		DBG_LOG(2,"HPGPU",("command: MOVEP [%d, 0x%x] (%d, %d)\n", command, command, m_input[0], m_input[1]));
		if (m_enable_cursor) bitblt(m_cursor_x, m_cursor_y, m_dad, 16, 16, RR_XOR);
		if (m_enable_sprite) bitblt(m_sprite_x, m_sprite_y, m_dad + 16, 16, 16, RR_XOR);
		m_cursor_x = m_input[0];
		m_cursor_y = m_input[1];
		m_saved_x = m_cursor_x;
		if (m_enable_cursor) bitblt(m_cursor_x, m_cursor_y, m_dad, 16, 16, RR_XOR);
		if (m_enable_sprite) bitblt(m_sprite_x, m_sprite_y, m_dad + 16, 16, 16, RR_XOR);
		break;

	// carriage return, line feed
	case CRLFx:
		DBG_LOG(2,"HPGPU",("command: CRLF [%d, 0x%x] (%d, %d)\n", command, command, m_input[0], m_input[1]));
		if (m_enable_cursor) bitblt(m_cursor_x, m_cursor_y, m_dad, 16, 16, RR_XOR);
		if (m_enable_sprite) bitblt(m_sprite_x, m_sprite_y, m_dad + 16, 16, 16, RR_XOR);
		m_cursor_x = m_saved_x;
		m_cursor_y += m_fontheight;
		if (m_enable_cursor) bitblt(m_cursor_x, m_cursor_y, m_dad, 16, 16, RR_XOR);
		if (m_enable_sprite) bitblt(m_sprite_x, m_sprite_y, m_dad + 16, 16, 16, RR_XOR);
		break;

	// move sprite absolute
	case MOVESP:
		DBG_LOG(2,"HPGPU",("command: MOVESP [%d, 0x%x] (%d, %d)\n", command, command, m_input[0], m_input[1]));
		m_sprite_x = m_input[0];
		m_sprite_y = m_input[1];
		break;

	// draw to ...
	case DRAWP:
		DBG_LOG(2,"HPGPU",("command: DRAWP [%d, 0x%x] (%d, %d) to (%d, %d)\n",
			command, command, m_cursor_x, m_cursor_y, m_input[0], m_input[1]));
		line(m_cursor_x, m_cursor_y, m_input[0], m_input[1]);
		m_cursor_x = m_input[0];
		m_cursor_y = m_input[1];
		break;

	// type 3 command -- CONF -- accept configuration parameters (11 words)

	case CONF:
		m_conf_ptr = 0;
		break;

	// type 4 commands -- like type 1 plus data is read or written after command, terminated by NOP

	case RDMEM:
	case WRMEM:
		DBG_LOG(2,"HPGPU",("command: %s [%d, 0x%x] (0x%04x)\n",
			command == RDMEM?"RDMEM":"WRMEM", command, command, m_input[0]));
		m_memory_ptr = m_input[0] << 1; // memory is word-addressable
		break;

	default:
		DBG_LOG(1,"HPGPU",("command: UNKNOWN [%d, 0x%x]\n", command, command));
		break;
	}

	m_input_ptr = 0;
	m_command = command;
}

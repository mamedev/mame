// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    HP 1LL3-0005 GPU emulation.

 ***************************************************************************/


#pragma once

#ifndef __HP1LL3_H__
#define __HP1LL3_H__

///*************************************************************************
//  MACROS / CONSTANTS
///*************************************************************************

/*
 * command types (send)
 *
 * 0 -- no data
 * 1 -- write 1 word of data, then command
 * 2 -- write 2 words of data, then command
 * 3 -- write command, then 11 words of data (= CONF only?)
 * 4 -- write 1 word of data, then command, then write X words of data, then write NOP
 *
 * (read)
 *
 * 3 -- ???
 * 4 -- write 1 word of data, then command, then read X words of data, then write NOP
 */
#define NOP			0	// type 0
#define CONF		2	// type 3, configure GPU (screen size, timings...).  11 words of data.
#define DISVID		3	// type 0, disable video
#define ENVID		4	// type 0, enable video
#define WRMEM		7	// type 4, write GPU memory at offset, terminate by NOP
#define RDMEM		8	// type 4, read GPU memory from offset, terminate by NOP
#define WRSAD		9	// type 1, set screen area start address
#define WRORG		10	// type 1, set ???
#define WRDAD		11	// type 1, set data area start address (16x16 area fill, sprite and cursor)
#define WRRR		12	// type 1, set replacement rule (rasterop)
#define MOVEP		13	// type 2, move pointer
#define IMOVEP		14
#define DRAWP		15	// type 2, draw line
#define IDRAWP		16
#define RDP			17
#define WRUDL		18	// type 1, set user-defined line pattern (16-bit)
#define WRWINSIZ	19	// type 2, set ???
#define WRWINORG	20	// type 2, set ???
#define COPY		21	// type 2
#define FILL		22	// type 1, fill area
#define FRAME		23	// type _, draw rectangle
#define SCROLUP		24	// type 2
#define SCROLDN		25	// type 2
#define SCROLLF		26	// type 2
#define SCROLRT		27	// type 2
#define RDWIN		28	// type 1
#define WRWIN		29	// type 1
#define RDWINPARM	30
#define CR			31
#define CRLFx		32
#define LABEL		36	// type 1, draw text
#define ENSP		38	// type 0, enable sprite
#define DISSP		39	// type 0, disable sprite
#define MOVESP		40	// type 2, move sprite
#define IMOVESP		41
#define RDSP		42
#define DRAWPX		43	// type _, draw single pixel
#define WRFAD		44	// type 1, set font area start address
#define ENCURS		45	// type 0
#define DISCURS		46	// type 0
#define ID			63


/*
 * Replacement Rules (rops).  sources: 
 *
 * - NetBSD's diofbvar.h (definitions for Topcat chip)
 * - pdf/hp/9000_300/specs/A-5958-4362-9_Series_300_Display_Color_Card_Theory_of_Operation_Oct85.pdf
 *   refers to TOPCAT documentation p/n A-1FH2-2001-7 (not online)
 */
#define	RR_FORCE_ZERO	0x0
#define	RR_CLEAR		RR_FORCE_ZERO
#define	RR_AND			0x1
#define	RR_AND_NOT_OLD	0x2
#define	RR_NEW			0x3
#define	RR_COPY			RR_NEW
#define	RR_AND_NOT_NEW	0x4
#define	RR_OLD			0x5
#define	RR_XOR			0x6
#define	RR_OR			0x7
#define	RR_NOR			0x8
#define	RR_XNOR			0x9
#define	RR_NOT_OLD		0xa
#define	RR_INVERT		RR_NOT_OLD
#define	RR_OR_NOT_OLD	0xb
#define	RR_NOT_NEW  	0xc
#define	RR_COPYINVERTED	RR_NOT_NEW
#define	RR_OR_NOT_NEW  	0xd
#define	RR_NAND		  	0xe
#define	RR_FORCE_ONE	0xf

#define WS 16 // bits in a word


///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_HP1LL3_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, HP1LL3, 0)

///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************


// ======================> hp1ll3_device

class hp1ll3_device :  public device_t,
	public device_video_interface
{
public:
	// construction/destruction
	hp1ll3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void command(int command);

	inline void point(int x, int y, int px);
	void label(uint8_t chr, int width);
	void fill(int x, int y, int w, int h, int arg);
	void line(int x_from, int y_from, int x_to, int y_to);
	void bitblt(int dstx, int dsty, uint16_t srcaddr, int width, int height, int op);

	uint16_t m_conf[11], m_input[2];
	int m_input_ptr, m_memory_ptr, m_conf_ptr;
	int m_command;

	uint16_t m_sad;
	uint16_t m_org;
	uint16_t m_dad;
	uint16_t m_rr;
	uint16_t m_fad, m_fontdata, m_fontheight;
	uint16_t m_udl;

	bool m_enable_video, m_enable_cursor, m_enable_sprite;
	uint16_t m_cursor_x, m_cursor_y;
	uint16_t m_sprite_x, m_sprite_y;
	struct {
		uint16_t width, height, org_x, org_y, width_w;
	} m_window;
	std::unique_ptr<uint16_t[]> m_videoram;

	bool m_busy;

	bitmap_ind16 m_bitmap, m_cursor, m_sprite;
};


// device type definition
extern const device_type HP1LL3;


#endif

// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * attache.c
 *
 *  Created on: 17/05/2013
 *
 *  Driver by Barry Rodewald
 *
 *
 *  Otrona Attache
 *
 *  CPU: Zilog Z80-A, 4MHz
 *  RAM: 64kB
 *  DMA: AMD 9517A (or compatible)
 *  RTC: Oki MSM5832, Z80-PIO
 *  Sound: GI AY-3-8912
 *  FDC: NEC D765A, 5.25" floppies
 *  Video: CRT5027, 320x240
 *  Serial: Z80-SIO
 *
 *  Note:
 *  In terminal mode (when disk booting fails or no disk is inserted), press Ctrl+Linefeed (ctrl+pgdn by default)
 *  to enter monitor mode.  From here you can run a bunch of diagnostic tests.
 *
 *  G - Display Test Pattern
 *  H - Display RAM Test
 *  nnI - Input Test  (nn = port number)
 *  J - Jump
 *  K - Keyboard Test
 *  L - Loop Tests
 *  M - Map Test
 *  nnmmO - Output Test (nn = port number, mm = data to send)
 *  P - Format Diskette (P to format disk in Drive A, 1P for Drive B)
 *  Q - CMOS RAM Test
 *  nR - Main RAM Test (n = 16kB bank to test [0-3])
 *  bbpcS - Select Output Ports (first b = printer baud rate, second b = comm baud rate, p = printer port, c = comm port)
 *  T - Real Time Clock Test
 *  U - United Tests
 *  cchsV - Read a sector from disk (cc = cylinder, h = head [bit 0=drive, bit 2=side], s = sector)
 *  cchsW - Write a sector from disk
 *  nnnnmmmmX - I/O port transmit (nnnn = number of bytes to transmit, mmmm = start of data to transmit)
 *  nnnnY - I/O port receive (nnnn = address of data loaded)
 *  Z - Auto Disk Test (1Z for drive B)
 *
 *
 *  TODO:
 *    - Keyboard repeat
 *    - Get at least some of the system tests to pass
 *    - and probably lots more I've forgotten, too.
 *
 */

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "sound/ay8910.h"
#include "machine/msm5832.h"
#include "machine/z80dart.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "machine/am9517a.h"
#include "machine/upd765.h"
#include "video/tms9927.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "softlist.h"

class attache_state : public driver_device
{
public:
	attache_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this,"maincpu"),
			m_rom(*this,"boot"),
			m_ram(*this,RAM_TAG),
			m_char_rom(*this,"video"),
			m_rtc(*this,"rtc"),
			m_psg(*this,"psg"),
			m_fdc(*this,"fdc"),
			m_sio(*this,"sio"),
			m_pio(*this,"pio"),
			m_ctc(*this,"ctc"),
			m_crtc(*this,"crtc"),
			m_dma(*this, "dma"),
			m_palette(*this, "palette"),
			m_floppy0(*this, "fdc:0:525dd"),
			m_floppy1(*this, "fdc:1:525dd"),
			m_kb_row0(*this, "row0"),
			m_kb_row1(*this, "row1"),
			m_kb_row2(*this, "row2"),
			m_kb_row3(*this, "row3"),
			m_kb_row4(*this, "row4"),
			m_kb_row5(*this, "row5"),
			m_kb_row6(*this, "row6"),
			m_kb_row7(*this, "row7"),
			m_kb_mod(*this, "modifiers"),
			m_membank1(*this, "bank1"),
			m_membank2(*this, "bank2"),
			m_membank3(*this, "bank3"),
			m_membank4(*this, "bank4"),
			m_membank5(*this, "bank5"),
			m_membank6(*this, "bank6"),
			m_membank7(*this, "bank7"),
			m_membank8(*this, "bank8"),
			m_nvram(*this, "nvram"),
			m_rom_active(true),
			m_gfx_enabled(false),
			m_kb_clock(true),
			m_kb_empty(true)
	{ }

	// PIO port B operation select
	enum
	{
		PIO_SEL_8910_ADDR = 0,
		PIO_SEL_8910_DATA,
		PIO_SEL_5832_READ,
		PIO_SEL_5832_WRITE,
		PIO_SEL_5101_WRITE,
		PIO_SEL_5101_READ,
		PIO_SEL_LATCH,
		PIO_SEL_NOP
	};

	// Display controller operation select
	enum
	{
		DISP_GFX_0 = 0,
		DISP_GFX_1,
		DISP_GFX_2,
		DISP_GFX_3,
		DISP_GFX_4,
		DISP_CRTC,
		DISP_ATTR,
		DISP_CHAR
	};

	// overrides
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank_int(screen_device &screen, bool state);
	virtual void driver_start();
	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_READ8_MEMBER(rom_r);
	DECLARE_WRITE8_MEMBER(rom_w);
	DECLARE_READ8_MEMBER(pio_portA_r);
	DECLARE_READ8_MEMBER(pio_portB_r);
	DECLARE_WRITE8_MEMBER(pio_portA_w);
	DECLARE_WRITE8_MEMBER(pio_portB_w);
	DECLARE_WRITE8_MEMBER(display_command_w);
	DECLARE_READ8_MEMBER(display_data_r);
	DECLARE_WRITE8_MEMBER(display_data_w);
	DECLARE_READ8_MEMBER(dma_mask_r);
	DECLARE_WRITE8_MEMBER(dma_mask_w);
	DECLARE_READ8_MEMBER(fdc_dma_r);
	DECLARE_WRITE8_MEMBER(fdc_dma_w);
	DECLARE_READ8_MEMBER(memmap_r);
	DECLARE_WRITE8_MEMBER(memmap_w);
	DECLARE_READ8_MEMBER(dma_mem_r);
	DECLARE_WRITE8_MEMBER(dma_mem_w);
	DECLARE_WRITE_LINE_MEMBER(hreq_w);
	DECLARE_WRITE_LINE_MEMBER(eop_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_dack_w);
	void operation_strobe(address_space& space,UINT8 data);
	void keyboard_clock_w(bool state);
	UINT8 keyboard_data_r();
	UINT16 get_key();
private:
	required_device<cpu_device> m_maincpu;
	required_memory_region m_rom;
	required_device<ram_device> m_ram;
	required_memory_region m_char_rom;
	required_device<msm5832_device> m_rtc;
	required_device<ay8912_device> m_psg;
	required_device<upd765a_device> m_fdc;
	required_device<z80sio0_device> m_sio;
	required_device<z80pio_device> m_pio;
	required_device<z80ctc_device> m_ctc;
	required_device<tms9927_device> m_crtc;
	required_device<am9517a_device> m_dma;
	required_device<palette_device> m_palette;
	required_device<floppy_image_device> m_floppy0;
	required_device<floppy_image_device> m_floppy1;
	required_ioport m_kb_row0;
	required_ioport m_kb_row1;
	required_ioport m_kb_row2;
	required_ioport m_kb_row3;
	required_ioport m_kb_row4;
	required_ioport m_kb_row5;
	required_ioport m_kb_row6;
	required_ioport m_kb_row7;
	required_ioport m_kb_mod;
	required_memory_bank m_membank1;
	required_memory_bank m_membank2;
	required_memory_bank m_membank3;
	required_memory_bank m_membank4;
	required_memory_bank m_membank5;
	required_memory_bank m_membank6;
	required_memory_bank m_membank7;
	required_memory_bank m_membank8;
	required_device<nvram_device> m_nvram;

	bool m_rom_active;
	bool m_gfx_enabled;
	UINT8 m_pio_porta;
	UINT8 m_pio_portb;
	UINT8 m_pio_select;
	UINT8 m_pio_latch;
	UINT8 m_crtc_reg_select;
	UINT8 m_current_cmd;
	UINT8 m_char_ram[128*32];
	UINT8 m_attr_ram[128*32];
	UINT8 m_gfx_ram[128*32*5];
	UINT8 m_char_line;
	UINT8 m_attr_line;
	UINT8 m_gfx_line;
	UINT8 m_cmos_ram[64];
	UINT8 m_cmos_select;
	UINT16 m_kb_current_key;
	bool m_kb_clock;
	bool m_kb_empty;
	UINT8 m_kb_bitpos;
	UINT8 m_memmap;
};

// Attributes (based on schematics):
// bit 0 = ALT
// bit 1 = RW
// bit 2 = BKG (reverse?)
// bit 3 = brightness
// bit 4 = double-size (width)
// bit 5 = underline
// bit 6 = superscript
// bit 7 = subscript (superscript and subscript combined produces strikethrough)
UINT32 attache_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 x,y,bit,scan,data;
	UINT8 dbl_mode = 0;  // detemines which half of character to display when using double size attribute,
							// as it can start on either odd or even character cells.

	// Graphics output (if enabled)
	if(m_gfx_enabled)
	{
		const pen_t *pen = m_palette->pens();

		for(y=0;y<(bitmap.height()-1)/10;y++)
		{
			for(x=0;x<(bitmap.width()-1)/8;x++)
			{
				// graphics pixels use half the clock of text, so 4 graphics pixels per character
				for(scan=0;scan<10;scan+=2)
				{
					data = m_gfx_ram[(128*32*(scan/2))+(y*128+x)];
					bitmap.pix32(y*10+scan,x*8)   = pen[BIT(data,7)];
					bitmap.pix32(y*10+scan,x*8+1) = pen[BIT(data,7)];
					bitmap.pix32(y*10+scan,x*8+2) = pen[BIT(data,6)];
					bitmap.pix32(y*10+scan,x*8+3) = pen[BIT(data,6)];
					bitmap.pix32(y*10+scan,x*8+4) = pen[BIT(data,5)];
					bitmap.pix32(y*10+scan,x*8+5) = pen[BIT(data,5)];
					bitmap.pix32(y*10+scan,x*8+6) = pen[BIT(data,4)];
					bitmap.pix32(y*10+scan,x*8+7) = pen[BIT(data,4)];
					bitmap.pix32(y*10+scan+1,x*8)   = pen[BIT(data,3)];
					bitmap.pix32(y*10+scan+1,x*8+1) = pen[BIT(data,3)];
					bitmap.pix32(y*10+scan+1,x*8+2) = pen[BIT(data,2)];
					bitmap.pix32(y*10+scan+1,x*8+3) = pen[BIT(data,2)];
					bitmap.pix32(y*10+scan+1,x*8+4) = pen[BIT(data,1)];
					bitmap.pix32(y*10+scan+1,x*8+5) = pen[BIT(data,1)];
					bitmap.pix32(y*10+scan+1,x*8+6) = pen[BIT(data,0)];
					bitmap.pix32(y*10+scan+1,x*8+7) = pen[BIT(data,0)];
				}
			}
		}
	}
	else
		bitmap.fill(0);

	// Text output
	for(y=0;y<(bitmap.height()-1)/10;y++)  // lines
	{
		for(x=0;x<(bitmap.width()-1)/8;x++)  // columns
		{
			assert(((y*128)+x) >= 0 && ((y*128)+x) < ARRAY_LENGTH(m_char_ram));
			UINT8 ch = m_char_ram[(y*128)+x];
			pen_t fg = m_palette->pen(m_attr_ram[(y*128)+x] & 0x08 ? 2 : 1); // brightness
			if(m_attr_ram[(y*128)+x] & 0x10) // double-size
				dbl_mode++;
			else
				dbl_mode = 0;

			for(scan=0;scan<10;scan++)  // 10 scanlines per line
			{
				data = m_char_rom->base()[ch*16+scan];
				if((m_attr_ram[(y*128)+x] & 0xc0) != 0xc0)  // if not strikethrough
				{
					if(m_attr_ram[(y*128)+x] & 0x40)  // superscript
					{
						if(scan >= 5)
							data = 0;
						else
							data = m_char_rom->base()[ch*16+(scan*2)+1];
					}
					if(m_attr_ram[(y*128)+x] & 0x80)  // subscript
					{
						if(scan < 5)
							data = 0;
						else
							data = m_char_rom->base()[ch*16+((scan-5)*2)+1];
					}
				}
				if((m_attr_ram[(y*128)+x] & 0x20) && scan == 9)  // underline
					data = 0xff;
				if((m_attr_ram[(y*128)+x] & 0xc0) == 0xc0 && scan == 3)  // strikethrough
					data = 0xff;
				if(m_attr_ram[(y*128)+x] & 0x04)  // reverse
					data = ~data;
				if(m_attr_ram[(y*128)+x] & 0x10) // double-size
				{
					UINT8 newdata = 0;
					if(dbl_mode & 1)
					{
						newdata = (data & 0x80) | ((data & 0x80) >> 1)
								| ((data & 0x40) >> 1) | ((data & 0x40) >> 2)
								| ((data & 0x20) >> 2) | ((data & 0x20) >> 3)
								| ((data & 0x10) >> 3) | ((data & 0x10) >> 4);
					}
					else
					{
						newdata = ((data & 0x08) << 4) | ((data & 0x08) << 3)
								| ((data & 0x04) << 3) | ((data & 0x04) << 2)
								| ((data & 0x02) << 2) | ((data & 0x02) << 1)
								| ((data & 0x01) << 1) | (data & 0x01);
					}
					data = newdata;
				}

				for(bit=0;bit<8;bit++)  // 8 pixels per character
				{
					UINT16 xpos = x*8+bit;
					UINT16 ypos = y*10+scan;

					if(BIT(data,7-bit))
						bitmap.pix32(ypos,xpos) = fg;
				}
			}
		}
	}
	return 0;
}

void attache_state::vblank_int(screen_device &screen, bool state)
{
	m_ctc->trg2(state);
}

READ8_MEMBER(attache_state::rom_r)
{
	if(m_rom_active)
		return m_rom->base()[offset];
	else
		return m_ram->pointer()[m_membank1->entry()*0x2000 + offset];
}

WRITE8_MEMBER(attache_state::rom_w)
{
	m_ram->pointer()[m_membank1->entry()*0x2000 + offset] = data;
}

UINT16 attache_state::get_key()
{
	UINT8 row,bits,data;
	ioport_port* keys[8] = { m_kb_row0, m_kb_row1, m_kb_row2, m_kb_row3, m_kb_row4, m_kb_row5, m_kb_row6, m_kb_row7 };
	UINT8 res = 0;

	// scan input ports
	for(row=0;row<8;row++)
	{
		data = keys[row]->read();
		for(bits=0;bits<8;bits++)
		{
			if(BIT(data,bits))
			{
				res = bits & 0x07;
				res |= ((row & 0x07) << 3);
				m_kb_empty = false;
				data = m_kb_mod->read();
				if(~data & 0x01)
					res |= 0x80;  // shift
				if(data & 0x02)
					res |= 0x40;  // ctrl
				//logerror("KB: hit row %i, bit %i\n",row,bits);
				return res;
			}
		}
	}
	// no key pressed
	m_kb_empty = true;
	return res;
}

UINT8 attache_state::keyboard_data_r()
{
	UINT16 key;
	if(m_kb_bitpos == 1)  // start bit, if data is available
	{
		key = get_key();
		if(m_kb_current_key != key)
			m_kb_current_key = key;
		else
			return 0x00;
		//logerror("KB: bit position %i, key %02x, empty %i\n",m_kb_bitpos,m_kb_current_key,m_kb_empty);
		if(m_kb_empty)
			return 0x00;
		else
			return 0x40;
	}
	else
	{
		//logerror("KB: bit position %i, key %02x, empty %i\n",m_kb_bitpos,m_kb_current_key,m_kb_empty);
		if(m_kb_current_key & (1<<(m_kb_bitpos-2)))
			return 0x00;
		else
			return 0x40;
	}
}

void attache_state::keyboard_clock_w(bool state)
{
	if(!state && m_kb_clock) // high to low transition - advance bit position
	{
		m_kb_bitpos++;
		if(m_kb_bitpos > 9)
			m_kb_bitpos = 1;
	}
	m_kb_clock = state;
}

// TODO: Figure out exactly how the HLD, RD, WR and CS lines on the RTC are hooked up
READ8_MEMBER(attache_state::pio_portA_r)
{
	UINT8 ret = 0xff;
	UINT8 porta = m_pio_porta;

	switch(m_pio_select)
	{
	case PIO_SEL_8910_DATA:
		ret = m_psg->data_r(space,0);
		logerror("PSG: data read %02x\n",ret);
		break;
	case PIO_SEL_5832_WRITE:
		m_rtc->cs_w(1);
		m_rtc->write_w(0);
		m_rtc->read_w(1);
		m_rtc->address_w((porta & 0xf0) >> 4);
		ret = m_rtc->data_r(space,0);
		logerror("RTC: read %02x from %02x (write)\n",ret,(porta & 0xf0) >> 4);
		break;
	case PIO_SEL_5832_READ:
		m_rtc->cs_w(1);
		m_rtc->write_w(0);
		m_rtc->read_w(1);
		m_rtc->address_w((porta & 0xf0) >> 4);
		ret = m_rtc->data_r(space,0);
		logerror("RTC: read %02x from %02x\n",ret,(porta & 0xf0) >> 4);
		break;
	case PIO_SEL_5101_WRITE:
		m_cmos_select = (m_cmos_select & 0xf0) | ((porta & 0xf0) >> 4);
		ret = m_cmos_ram[m_cmos_select] & 0x0f;
		logerror("CMOS: read %02x from byte %02x (write)\n",ret, m_cmos_select);
		break;
	case PIO_SEL_5101_READ:
		m_cmos_select = (m_cmos_select & 0xf0) | ((porta & 0xf0) >> 4);
		ret = m_cmos_ram[m_cmos_select] & 0x0f;
		logerror("CMOS: read %02x from byte %02x\n",ret, m_cmos_select);
		break;
	case PIO_SEL_LATCH:
		ret = 0x00;  // Write-only?
		break;
	case PIO_SEL_NOP:
		logerror("PIO: NOP read\n");
		break;
	}
	//logerror("PIO: Port A read operation %i returning %02x\n",m_pio_select,ret);

	return ret;
}

READ8_MEMBER(attache_state::pio_portB_r)
{
	UINT8 ret = m_pio_portb & 0xbf;
	ret |= keyboard_data_r();
	return ret;
}

void attache_state::operation_strobe(address_space& space, UINT8 data)
{
	//logerror("PIO: Port A write operation %i, data %02x\n",m_pio_select,data);
	switch(m_pio_select)
	{
	case PIO_SEL_8910_ADDR:
		m_psg->address_w(space,0,data);
		break;
	case PIO_SEL_8910_DATA:
		m_psg->data_w(space,0,data);
		break;
	case PIO_SEL_5832_WRITE:
		m_rtc->cs_w(1);
		m_rtc->write_w(1);
		m_rtc->read_w(0);
		m_rtc->address_w((data & 0xf0) >> 4);
		m_rtc->data_w(space,0,data & 0x0f);
		logerror("RTC: write %01x to %01x\n",data & 0x0f,(data & 0xf0) >> 4);
		break;
	case PIO_SEL_5832_READ:
		m_rtc->cs_w(1);
		m_rtc->write_w(0);
		m_rtc->read_w(0);
		m_rtc->address_w((data & 0xf0) >> 4);
		logerror("RTC: write %01x to %01x (read)\n",data & 0x0f,(data & 0xf0) >> 4);
		break;
	case PIO_SEL_5101_WRITE:
		m_cmos_select = (m_cmos_select & 0xf0) | ((data & 0xf0) >> 4);
		m_cmos_ram[m_cmos_select] = data & 0x0f;
		logerror("CMOS: write %01x to byte %02x\n",data & 0x0f, m_cmos_select);
		break;
	case PIO_SEL_5101_READ:
		m_cmos_select = (m_cmos_select & 0xf0) | ((data & 0xf0) >> 4);
		logerror("CMOS: write %01x to byte %02x (read)\n",data & 0x0f, m_cmos_select);
		break;
	case PIO_SEL_LATCH:
		m_pio_latch = data;
		m_rom_active = ~data & 0x04;
		m_floppy0->mon_w((data & 0x01) ? 0 : 1);
		m_floppy1->mon_w((data & 0x01) ? 0 : 1);
		m_gfx_enabled = data & 0x02;
		// TODO: display brightness
		break;
	case PIO_SEL_NOP:
		logerror("PIO: NOP write\n");
		break;
	default:
		logerror("PIO: Invalid write operation %i, data %02x\n",m_pio_select,data);
	}
}

WRITE8_MEMBER(attache_state::pio_portA_w)
{
	//  AO-7 = LATCH DATA OUT:
	//  LO = MOTOR ON
	//  L1 = GRAPHICS ENABLE
	//  L2 = /EPROM ENABLE
	//  L3-7 = DISPLAY BRIGHTNESS
	//  AO-7 = 8910 DATA I/O:
	//  AO-3 = 5832 DO-3 I/O
	//  A4-7 = 5832 AO-3 OUT
	//  AO-3 = 5101 DO-3 I/O
	//  A4-7 = 5101 AO-3 OUT
	m_pio_porta = data;
}

WRITE8_MEMBER(attache_state::pio_portB_w)
{
	//  BO-1 = 5101 A4-5
	//  B2-4 = OPERATION SELECT
	//  0 = 8910 ADDR LOAD
	//  1 = 8910 DATA LOAD
	//  2 = 5832 WRITE  -- the CP/M BIOS dumped from an actual disc seems to switch the RTC operations around
	//  3 = 5832 READ      this differs from the BIOS source listings available for both CP/M 2.2.3 and 2.2.5
	//  4 = 5101 WRITE
	//  5 = 5101 READ
	//  6 = LATCH LOAD
	//  7 = NO-OP
	//B5 = /'138 OPERATION STROBE
	//B6 = /KEYBOARD DATA IN
	//B7 = /KEYBOARD CLOCK OUT
	m_cmos_select = ((data & 0x03) << 4) | (m_cmos_select & 0x0f);
	if(!(data & 0x20) && (m_pio_portb & 0x20))
	{
		m_pio_select = (data & 0x1c) >> 2;
		operation_strobe(space,m_pio_porta);
	}
	m_pio_portb = data;
	keyboard_clock_w(data & 0x80);
}

// Display uses A8-A15 placed on the bus by the OUT instruction as an extra parameter
READ8_MEMBER(attache_state::display_data_r)
{
	UINT8 ret = 0xff;
	UINT8 param = (offset & 0xff00) >> 8;

	switch(m_current_cmd)
	{
	case DISP_GFX_0:
		ret = m_gfx_ram[(m_gfx_line*128)+(param & 0x7f)];
		break;
	case DISP_GFX_1:
		ret = m_gfx_ram[(m_gfx_line*128)+(param & 0x7f)+(128*32)];
		break;
	case DISP_GFX_2:
		ret = m_gfx_ram[(m_gfx_line*128)+(param & 0x7f)+(128*32*2)];
		break;
	case DISP_GFX_3:
		ret = m_gfx_ram[(m_gfx_line*128)+(param & 0x7f)+(128*32*3)];
		break;
	case DISP_GFX_4:
		ret = m_gfx_ram[(m_gfx_line*128)+(param & 0x7f)+(128*32*4)];
		break;
	case DISP_CRTC:
		ret = m_crtc->read(space, m_crtc_reg_select);
		break;
	case DISP_ATTR:
		ret = m_attr_ram[(m_attr_line*128)+(param & 0x7f)];
		break;
	case DISP_CHAR:
		ret = m_char_ram[(m_char_line*128)+(param & 0x7f)];
		break;
	default:
		logerror("Unimplemented display operation %02x\n",m_current_cmd);
	}

	return ret;
}

WRITE8_MEMBER(attache_state::display_data_w)
{
	UINT8 param = (offset & 0xff00) >> 8;
	switch(m_current_cmd)
	{
	case DISP_GFX_0:
		m_gfx_ram[(m_gfx_line*128)+(param & 0x7f)] = data;
		break;
	case DISP_GFX_1:
		m_gfx_ram[(m_gfx_line*128)+(param & 0x7f)+(128*32)] = data;
		break;
	case DISP_GFX_2:
		m_gfx_ram[(m_gfx_line*128)+(param & 0x7f)+(128*32*2)] = data;
		break;
	case DISP_GFX_3:
		m_gfx_ram[(m_gfx_line*128)+(param & 0x7f)+(128*32*3)] = data;
		break;
	case DISP_GFX_4:
		m_gfx_ram[(m_gfx_line*128)+(param & 0x7f)+(128*32*4)] = data;
		break;
	case DISP_CRTC:
		m_crtc->write(space, m_crtc_reg_select, data);
		break;
	case DISP_ATTR:
		m_attr_ram[(m_attr_line*128)+(param & 0x7f)] = data;
		break;
	case DISP_CHAR:
		m_char_ram[(m_char_line*128)+(param & 0x7f)] = data;
		break;
//  default:
//      logerror("Unimplemented display operation %02x data %02x param %02x\n",m_current_cmd,data,param);
	}
}

WRITE8_MEMBER(attache_state::display_command_w)
{
	UINT8 cmd = (data & 0xe0) >> 5;

	m_current_cmd = cmd;

	switch(cmd)
	{
	case DISP_GFX_0:
	case DISP_GFX_1:
	case DISP_GFX_2:
	case DISP_GFX_3:
	case DISP_GFX_4:
		m_gfx_line = data & 0x1f;
		break;
	case DISP_CRTC:
		// CRT5027/TMS9927 registers
		m_crtc_reg_select = data & 0x0f;
		break;
	case DISP_ATTR:
		// Attribute RAM
		m_attr_line = data & 0x1f;
		break;
	case DISP_CHAR:
		// Character RAM
		m_char_line = data & 0x1f;
		break;
	}
}

READ8_MEMBER(attache_state::memmap_r)
{
	return m_memmap;
}

WRITE8_MEMBER(attache_state::memmap_w)
{
	// TODO: figure this out properly
	// Tech manual says that RAM is split into 8kB chunks.
	// Would seem that bit 4 is always 0 and bit 3 is always 1?
	UINT8 bank = (data & 0xe0) >> 5;
	UINT8 loc = data & 0x07;
	memory_bank* banknum[8] = { m_membank1, m_membank2, m_membank3, m_membank4, m_membank5, m_membank6, m_membank7, m_membank8 };
	m_memmap = data;

	banknum[bank]->set_entry(loc);

	logerror("MEM: write %02x - bank %i, location %i\n",data, bank, loc);
}

READ8_MEMBER(attache_state::dma_mask_r)
{
	return m_dma->read(space,0x0f);
}

WRITE8_MEMBER(attache_state::dma_mask_w)
{
	m_dma->write(space,0x0f,data);
}

READ8_MEMBER(attache_state::fdc_dma_r)
{
	UINT8 ret = m_fdc->dma_r();
	return ret;
}

WRITE8_MEMBER(attache_state::fdc_dma_w)
{
	m_fdc->dma_w(data);
}

READ8_MEMBER(attache_state::dma_mem_r)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

WRITE8_MEMBER(attache_state::dma_mem_w)
{
	m_maincpu->space(AS_PROGRAM).write_byte(offset,data);
}

WRITE_LINE_MEMBER( attache_state::hreq_w )
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	m_dma->hack_w(state);
}

WRITE_LINE_MEMBER(attache_state::eop_w)
{
	m_fdc->tc_w(state);
}

WRITE_LINE_MEMBER( attache_state::fdc_dack_w )
{
}

static ADDRESS_MAP_START( attache_map , AS_PROGRAM, 8, attache_state)
	AM_RANGE(0x0000,0x1fff) AM_RAMBANK("bank1")
	AM_RANGE(0x2000,0x3fff) AM_RAMBANK("bank2")
	AM_RANGE(0x4000,0x5fff) AM_RAMBANK("bank3")
	AM_RANGE(0x6000,0x7fff) AM_RAMBANK("bank4")
	AM_RANGE(0x8000,0x9fff) AM_RAMBANK("bank5")
	AM_RANGE(0xa000,0xbfff) AM_RAMBANK("bank6")
	AM_RANGE(0xc000,0xdfff) AM_RAMBANK("bank7")
	AM_RANGE(0xe000,0xffff) AM_RAMBANK("bank8")
ADDRESS_MAP_END

static ADDRESS_MAP_START( attache_io , AS_IO, 8, attache_state)
	AM_RANGE(0xe0, 0xed) AM_DEVREADWRITE("dma",am9517a_device,read,write) AM_MIRROR(0xff00)
	AM_RANGE(0xee, 0xee) AM_WRITE(display_command_w) AM_MIRROR(0xff00)
	AM_RANGE(0xef, 0xef) AM_READWRITE(dma_mask_r, dma_mask_w) AM_MIRROR(0xff00)
	AM_RANGE(0xf0, 0xf3) AM_DEVREADWRITE("sio",z80sio0_device,ba_cd_r, ba_cd_w) AM_MIRROR(0xff00)
	AM_RANGE(0xf4, 0xf7) AM_DEVREADWRITE("ctc",z80ctc_device,read,write) AM_MIRROR(0xff00)
	AM_RANGE(0xf8, 0xfb) AM_DEVREADWRITE("pio",z80pio_device,read_alt,write_alt) AM_MIRROR(0xff00)
	AM_RANGE(0xfc, 0xfd) AM_DEVICE("fdc",upd765a_device,map) AM_MIRROR(0xff00)
	AM_RANGE(0xfe, 0xfe) AM_READWRITE(display_data_r, display_data_w) AM_MIRROR(0xff00) AM_MASK(0xffff)
	AM_RANGE(0xff, 0xff) AM_READWRITE(memmap_r, memmap_w) AM_MIRROR(0xff00)
ADDRESS_MAP_END

static INPUT_PORTS_START(attache)
	PORT_START("row0")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("BS") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("LF") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x18,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(27)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("LOCK") PORT_CODE(KEYCODE_PGUP)

	PORT_START("row1")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x06,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)

	PORT_START("row2")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0 ^") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2 @") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7 *") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("row3")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8 (") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9 )") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("' \"") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("= +") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/')

	PORT_START("row4")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("` ~") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G')

	PORT_START("row5")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("row6")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("row7")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)

	PORT_START("modifiers")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL)

INPUT_PORTS_END

// IRQ daisy chain = CTC -> SIO -> Expansion
static const z80_daisy_config attache_daisy_chain[] =
{
	{ "ctc" },
	{ "sio" },
	// expansion
	{ NULL }
};

static SLOT_INTERFACE_START( attache_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

void attache_state::driver_start()
{
	UINT8 *RAM = m_ram->pointer();

	m_membank1->configure_entries(0, 8, &RAM[0x0000], 0x2000);
	m_membank2->configure_entries(0, 8, &RAM[0x0000], 0x2000);
	m_membank3->configure_entries(0, 8, &RAM[0x0000], 0x2000);
	m_membank4->configure_entries(0, 8, &RAM[0x0000], 0x2000);
	m_membank5->configure_entries(0, 8, &RAM[0x0000], 0x2000);
	m_membank6->configure_entries(0, 8, &RAM[0x0000], 0x2000);
	m_membank7->configure_entries(0, 8, &RAM[0x0000], 0x2000);
	m_membank8->configure_entries(0, 8, &RAM[0x0000], 0x2000);

	m_membank1->set_entry(0);
	m_membank2->set_entry(1);
	m_membank3->set_entry(2);
	m_membank4->set_entry(3);
	m_membank5->set_entry(4);
	m_membank6->set_entry(5);
	m_membank7->set_entry(6);
	m_membank8->set_entry(7);

	memset(RAM,0,65536);

	m_nvram->set_base(m_cmos_ram,64);

	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x0000,0x0fff,read8_delegate(FUNC(attache_state::rom_r),this),write8_delegate(FUNC(attache_state::rom_w),this));

	save_pointer(m_char_ram,"Character RAM",128*32);
	save_pointer(m_attr_ram,"Attribute RAM",128*32);
	save_pointer(m_gfx_ram,"Graphics RAM",128*32*5);
	save_pointer(m_cmos_ram,"CMOS RAM",64);
}

void attache_state::machine_start()
{
	// initialise RAM
	memset(m_cmos_ram,0,64);
	memset(m_attr_ram,0,128*32);
	memset(m_char_ram,0,128*32);
	memset(m_gfx_ram,0,128*32*5);
}

void attache_state::machine_reset()
{
	m_kb_bitpos = 0;
}

static MACHINE_CONFIG_START( attache, attache_state )
	MCFG_CPU_ADD("maincpu",Z80,XTAL_8MHz / 2)
	MCFG_CPU_PROGRAM_MAP(attache_map)
	MCFG_CPU_IO_MAP(attache_io)
	MCFG_CPU_CONFIG(attache_daisy_chain)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(64)) /* not accurate */
	MCFG_SCREEN_SIZE(640,240)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(attache_state, screen_update)
	MCFG_SCREEN_VBLANK_DRIVER(attache_state, vblank_int)

	MCFG_PALETTE_ADD_MONOCHROME_GREEN_HIGHLIGHT("palette")

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("psg", AY8912, XTAL_8MHz / 4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_MSM5832_ADD("rtc",XTAL_32_768kHz)

	MCFG_DEVICE_ADD("pio", Z80PIO, XTAL_8MHz/26)
	MCFG_Z80PIO_IN_PA_CB(READ8(attache_state, pio_portA_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(attache_state, pio_portA_w))
	MCFG_Z80PIO_IN_PB_CB(READ8(attache_state, pio_portB_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(attache_state, pio_portB_w))

	MCFG_Z80SIO0_ADD("sio",XTAL_8MHz / 26, 0, 0, 0, 0)

	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL_8MHz / 4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("dma", AM9517A, XTAL_8MHz / 4)
	MCFG_AM9517A_OUT_HREQ_CB(WRITELINE(attache_state, hreq_w))
	MCFG_AM9517A_OUT_EOP_CB(WRITELINE(attache_state, eop_w))
	MCFG_AM9517A_IN_MEMR_CB(READ8(attache_state, dma_mem_r))
	MCFG_AM9517A_OUT_MEMW_CB(WRITE8(attache_state, dma_mem_w))
	MCFG_AM9517A_IN_IOR_0_CB(READ8(attache_state, fdc_dma_r))
	MCFG_AM9517A_OUT_IOW_0_CB(WRITE8(attache_state, fdc_dma_w))
	// MCFG_AM9517A_OUT_DACK_0_CB(WRITELINE(attache_state, fdc_dack_w))

	MCFG_UPD765A_ADD("fdc", true, true)
	MCFG_UPD765_INTRQ_CALLBACK(DEVWRITELINE("ctc", z80ctc_device, trg3))
	MCFG_UPD765_DRQ_CALLBACK(DEVWRITELINE("dma", am9517a_device, dreq0_w)) MCFG_DEVCB_INVERT
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", attache_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", attache_floppies, "525dd", floppy_image_device::default_floppy_formats)

	MCFG_DEVICE_ADD("crtc", TMS9927, 12324000)
	MCFG_TMS9927_CHAR_WIDTH(8)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64k")

	MCFG_SOFTWARE_LIST_ADD("disk_list","attache")
MACHINE_CONFIG_END

ROM_START( attache )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_FILL(0x0000,0x10000,0x00)

	ROM_REGION(0x1000, "boot", 0)
	ROM_SYSTEM_BIOS(0, "u252revg", "Boot Rev.G")
	ROMX_LOAD("u252revg.bin", 0x0000, 0x1000, CRC(113136b7) SHA1(845afd9ed2fd2b28c39921d8f2ba99e5295e0330), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "u252revf", "Boot Rev.F")
	ROMX_LOAD("u252revf.bin", 0x0000, 0x1000, CRC(b49eb3b2) SHA1(5b1b348301b2f76b1f250ba68bb8733fc15d18c2), ROM_BIOS(2))

	ROM_REGION(0x1000, "video", 0)
	ROM_LOAD("u416vid.bin",  0x0000, 0x1000, CRC(e376ec59) SHA1(7b9e9db575e77ce2f479eb9ae913528e4f0d125d) )

	ROM_REGION(0x100, "attr", 0)
	ROM_LOAD("u413.bin",  0x0000, 0x0100, CRC(5b60e622) SHA1(43450c747db1394466eabe5c26a61bf75a4f3b52) )

	ROM_REGION(0x200, "iosel", 0)
	ROM_LOAD("u110.bin",  0x0000, 0x0200, CRC(70dd255a) SHA1(36dcce07a2c14eefc069433459c422341bd47efb) )

	ROM_REGION(0x100, "floppy", 0)
	ROM_LOAD("u630.bin",  0x0000, 0x0100, CRC(f7a5c821) SHA1(fea07d9ac7e4e5f4f72aa7b2159deaedbd662ead) )
ROM_END

/*    YEAR  NAME    PARENT  COMPAT      MACHINE     INPUT    DEVICE            INIT    COMPANY      FULLNAME     FLAGS */
COMP( 1982, attache, 0,      0,         attache,    attache, driver_device,    0,      "Otrona",   "Attach\xC3\xA9",    MACHINE_IMPERFECT_GRAPHICS|MACHINE_NOT_WORKING)

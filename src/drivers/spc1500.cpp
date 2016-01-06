// license:BSD-3-Clause
// copyright-holders:Miso Kim
/***************************************************************************

Samsung SPC-1500 driver by Miso Kim

  2015-12-16 preliminary driver initialized
  2015-12-18 cassette tape supported
  2015-12-26 80/40 column mode supported
  2015-12-28 double access mode supported for I/O 
  2016-01-02 Korean character input method and display enabled
  2016-01-03 user defined char (PCG, Programmable Character Generator) support	
  2016-01-05 detection of color palette initialization 
  2016-01-06 80x16 mode graphic mode support
	
TODO:
  - Verify PCG ram read
  - Support floppy disk drive with SD-1500A controller card
	
****************************************************************************/

/*
 * SAMSUNG SPC-1500 Series
 *
 * Brief history
 * ---------------------------------------------------------
 * First release on 02-26-1987
 * Press release on 03-14-1987
 * Market price 365,000 won ($430) on 04-01-1987

 Hardware Specification
 
 1) SPC-1500
 RAM 122KB
  - Main Memory: 64KB
  - Video RAM: 10KB (text 2KB, character attributes 2KB, custom letter: 6KB)
  - Graphics RAM: 48KB (16KB on three sides by assigning each of RGB)
 ROM 96KB
  - BIOS: 32KB (including IPL and a part of BASIC)
  - BASIC ROM: 32KB (Hangul Samsung BASIC - Korean character display)
  - English font: 8KB (sizes 8x8 and 8x16 size of each character in ROM)
  - Hangul fonts: 24KB (head 8KB, middle 8KB, last 8KB)

 Periperials
  - Built-in cassette deck
  - 1 composite video output to connect a monitor
  - 1 TTL output for RGB monitor connection
  - 1 RF output for TV connection
  - 1 printer port
  - 1 joystick port

 Expansion Slots
  - Built-in 50-pin 3 slot
  - One is using the memory expansion card by default

 Two external power connector for FDD connection
  - DIP switch settings for the screen
  - Volume control
 
 2) SPC-1500A
  July 1987 Release
  RF modulator only remove the product from an existing model
 
 3) SPC-1500V
  This product can not confirm the release date because of PCB level modification.
  It equiped SPC-1500V VLSI chip embedded products and removed a lot of TTLs and the memory expansion card.
  - IOCS ROM Version: 1.6
  - Two internal 50-pin expension slots
 
 Firmware
 
  IOCS ROM
    The various versions with 32KB of capacity existed to date has confirmed the final version number 1.8
  - Version 1.3:
  - Version 1.4:
  - Version 1.5:
  - Version 1.6:
  - Version 1.8: supports a variety of peripherals such as external hard disk, FM-Sound, RS-232C from Static soft (C)
                 various memu appears on the initial screen.
 
  BASIC ROM
   Capacity and the final version number of the currently identified 32KB 1.3
 
  English ROM
   The final version of the verification of the capacity 8KB SS150-1222
   The character set of a 8x8 size, and are stored with the size 8x16 8x16 is a part of the size of the font data are written differently and 8x8.
 
  Hangul ROM
   8KB each is divided by a consonant and consonant and neutral.
   - Inital (Choseong)  SS151-1223: 8 types of intial character (actual 6 types)
   - Middle (Jungseong) SS152-1224: 2 types of middle character
   - Final  (Jongseong) SS153-1225: 2 types of final character
  
  Periperials - Monitor
   - , high-resolution monitor SM, color monitor model was to distinguish it from CD.
  
  1) MD-1255H (Low resolution monitors MD)
   - 12 inches Composite 15.734KHz / 60Hz
   - N displayed after the model name in the model is non - CRT scanning products
   - Stand adopted: if you put the rest on the bottom that can be placed slightly tilted back.
 
  2) MD-9052H (Low resolution monitors MD)
   - 9 inches Composite 15.734KHz / 60Hz
   - N from model name means 'anti-glare' 
   - All parts except for the appearance and size is the same as the CRT 1255H.
 
  3) MD-2563 (color monitor SM)
  4) SM-1439A (high-resolution monitor SM)
  5) SM-1422 (high-resolution monitor SM)
   - RGB monitors
  6) SM-1231 (high-resolution monitor SM)
   - High-resolution monochrome monitor
  7) SM-1231A (high-resolution monitor SM)
   - The other part is other than the appearance of the stand is attached to the same as the model SM-1231
 
  8) CD-1451D (color monitor SM) 
   - Composite color monitors
  9) CD-1462X (color monitor SM)
  10)CD-1464W (color monitor SM)
  11)CW-4644 
 
  FDD (floppy disk drive)
 
  1) SD-1500A
   - 5.25 "floppy drive for 2D composed of external disk drives diskettes
  2) SD-1500B
   - Dual external disk drives
   - The two models are idential except the number of FDD. They need the expension controller card named by SFC-1500.
   - IBM PC XT compatible FDD can be quipped. SFD-5x0 model is a genuine FDD from Samsung Electronics.

  HDD (Hard Disk Drive)
 
  1) STH-20
   - External hard disk drive set having a capacity of 20MB SCSI controller and the way
   - The controller had not solved alone but the controller can be used to mount another hard disk products.
   - Release price: 450,000 won ($530).
 
  Joysticks
   - Joystick was limited to 1 as possible (The PCB was designed by supporting two joysticks. 
 
  1) SJ-1500
   - Release price: 8,000 won ($9.4)
   - SPC-1000A, MSX-compatible
 
  Printer
 
  1) SP-510S
   - Bitmap image output method Hangul support
   - Recommanded 80 columns dot-matrix printer
  2) SP-570H
   - Recommanded 132 columns dot-matrix printer
  3) SP-510L
  4) SP-510T
  5) SP-570B

  Expansion Cards
 
  1) SFC-1500
   - External FDD capacity of the floppy disk controller 5.25 inches / 320KB can connect up to two.
 
  2) Multi-controller
   - Floppy disk controllers and hard disk controllers on the same PCB.
 
  3) ST-PAC
   - FM sound card can play with up to 9 simultaneous sound or 5 simultaneous sound and 5 drum tones at the same time (FM-PAC compatible MSX)
   - Line output and speaker output volume, tone adjustment built-in volume
   - it can be used as a synthesizer by connecting the ST-KEY2 product 
   - Release price: 60,000won ($71)
 
  SPC-1500 VDP card
   - MSX game support 
   - Release price: 35,000won ($41) with composite output only
   - Release price: 60,000won ($71) with composite and RGB outputs simultaneously
 
  VDP UNIT I
   - Composite video output with built-in card expansion card using the same video chip and MSX (static soft)
   - Release price: 40,000won ($47).
 
  VDP UNIT II
   - Expansion using the same video chip and video card with built-in card MSX with an RGB output (static soft)
   - Release price: 55,000won ($59).
 
  LAN card (SAMNET-K)
   - It uses serial communication instead of an Ethernet network card has a way with two serial ports.
   - There are two kinds of host card without a DIP switch and the DIP switch is in the client card.
   - It was mainly supplied to the teacher / student in an educational institution.
 
  Super Pack Card
   - Expansion cards that enable the external expansion slot, etc.
 
  RS-232C card
   - At least 300bps, an external modem connected to the serial communication card that supports up to 19,200bps 
     additionally available communications services using the PSTN network (general switched telephone network) 
	 and may also be connected to a 9-pin serial mouse.
   - Support for common serial communications functions, 
     and if IOCS ROM version 1.8 or higher to connect an external modem to the PC communication card is available.
   - When used in this communication program is super soft static net programs (XMODEM protocol, FS 220-6 compatible 
     and supporting Samsung/Sambo combination korean character code, and an 8-bit code completion support Hangul) is used.
   - Release price: 60,000won ($71).
 
  SS-1 ROM pack unit
   - The VDP unit containing 1 cartridge slot card and ROM pack
   - ROM pack was not compatible with original MSX ROM pack
   - Release price: 49,900won ($58)
 
  Super Pack
   - External ROM cartrige from Static Soft (C)
   - 1 cartridge slot and 3 expansion slots (up to five expansion slots available)
   - It is available to use the MSX ROM packs without any modification with the static soft VDP card
   - Release price: 60,000won ($71)
 
  ST-KEY2
   - For synthesizer external keyboard
 
  * Compatiblity with X1 series of Sharp Electronics
   - Almost the key components is the same as X1 models of Sharp Electronics and except for the keyboard input.
   - To port the X1 software to SPC-1500, Text attribute, keyboard input and DMA related code should be modified
   
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "sound/wave.h"
#include "video/mc6845.h"
#include "imagedev/cassette.h"
#include "formats/spc1000_cas.h"
#include "bus/centronics/ctronics.h"
#define VDP_CLOCK  XTAL_42_9545MHz
#include "softlist.h"

class spc1500_state : public driver_device
{
public:
	spc1500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vdg(*this, "mc6845")
		, m_cass(*this, "cassette")
		, m_ram(*this, RAM_TAG)
		, m_p_videoram(*this, "videoram")
		, m_pcgram(*this, "pcgram")
		, m_io_kb(*this, "LINE")
		, m_io_joy(*this, "JOY")
		, m_dipsw(*this, "DIP_SWITCH")		
		, m_centronics(*this, "centronics")
		, m_pio(*this, "ppi8255")
		, m_sound(*this, "ay8910")
		, m_palette(*this, "palette")
	{}
	DECLARE_WRITE_LINE_MEMBER(irq_w);
	DECLARE_READ8_MEMBER(psga_r);	
	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_WRITE_LINE_MEMBER( centronics_busy_w ) { m_centronics_busy = state; }
	DECLARE_READ8_MEMBER(mc6845_videoram_r);
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(palet_w);
	DECLARE_WRITE8_MEMBER(priority_w);
	DECLARE_WRITE8_MEMBER(pcg_w);
	DECLARE_WRITE8_MEMBER(pcgg_w);
	DECLARE_WRITE8_MEMBER(pcgb_w);
	DECLARE_WRITE8_MEMBER(pcgr_w);
	DECLARE_READ8_MEMBER(pcg_r);
	DECLARE_READ8_MEMBER(pcgg_r);
	DECLARE_READ8_MEMBER(pcgb_r);
	DECLARE_READ8_MEMBER(pcgr_r);
	DECLARE_WRITE8_MEMBER(crtc_w);
	DECLARE_READ8_MEMBER(crtc_r);
	DECLARE_WRITE8_MEMBER(romsel);
	DECLARE_WRITE8_MEMBER(ramsel);
	DECLARE_WRITE8_MEMBER(portb_w);
	DECLARE_WRITE8_MEMBER(psgb_w);
	DECLARE_WRITE8_MEMBER(portc_w);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_WRITE8_MEMBER(double_w);
	DECLARE_READ8_MEMBER(io_r);
	DECLARE_PALETTE_INIT(spc);
	DECLARE_VIDEO_START(spc);
	MC6845_UPDATE_ROW(crtc_update_row); 
	MC6845_RECONFIGURE(crtc_reconfig);
private:
	UINT8 *m_p_ram;
	UINT8 m_ipl;
	UINT8 m_palet[3];
	UINT16 m_page;
	UINT16 m_pcg_addr;
	UINT8 m_pcg_char, m_pcg_attr;
	attotime m_time;
	bool m_romsel;
	bool m_double_mode;
	bool m_p5bit;
	bool m_motor;
	UINT8 m_crtc_vreg[0x100];
	bool m_centronics_busy;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<z80_device> m_maincpu;
	required_device<mc6845_device> m_vdg;
	required_device<cassette_image_device> m_cass;
	required_device<ram_device> m_ram;
	required_shared_ptr<UINT8> m_p_videoram;
	required_shared_ptr<UINT8> m_pcgram;
	required_ioport_array<10> m_io_kb;
	required_ioport m_io_joy;
	required_ioport m_dipsw;
	required_device<centronics_device> m_centronics;
	required_device<i8255_device> m_pio;
	required_device<ay8910_device> m_sound;
	required_device<palette_device> m_palette;	
	UINT8 *m_font;        
	UINT8 m_priority;
	void get_pcg_addr();
};

READ8_MEMBER( spc1500_state::keyboard_r )
{
	offset &= 0xf;

	if (offset <= 9)
		return m_io_kb[offset]->read();
	else
		return 0xff;
}

WRITE8_MEMBER( spc1500_state::romsel)
{
	m_romsel = 1;
	if (m_ipl)
		membank("bank1")->set_entry(0);
	else
		membank("bank1")->set_entry(1);		
}

WRITE8_MEMBER( spc1500_state::ramsel)
{
	m_romsel = 0;
	membank("bank1")->set_entry(2);
}

WRITE8_MEMBER( spc1500_state::portb_w)
{
//	m_ipl = data & (1 << 1);
}

WRITE8_MEMBER( spc1500_state::psgb_w)
{
	attotime time = machine().scheduler().time();
	if (m_ipl != ((data>>1)&1))
	{
		m_ipl = ((data>>1)&1);
		membank("bank1")->set_entry(m_ipl ? 0 : 1);
	}
	m_cass->set_state(BIT(data, 6) ? CASSETTE_SPEAKER_ENABLED : CASSETTE_SPEAKER_MUTED);
	if (!m_motor && BIT(data, 7) &&  ATTOSECONDS_IN_MSEC((time - m_time).as_attoseconds()) > 1000)
	{
		m_cass->change_state((m_cass->get_state() & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_DISABLED ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
		m_time = time;
	}
	m_motor = BIT(data, 7);
}

WRITE8_MEMBER( spc1500_state::portc_w)
{
	m_cass->output(BIT(data, 0) ? -1.0 : 1.0);
	m_centronics->write_strobe(BIT(data, 7));
	m_double_mode = (!m_p5bit && BIT(data, 5)); // double access I/O mode
	m_p5bit = BIT(data, 5);
}

READ8_MEMBER( spc1500_state::portb_r)
{
	UINT8 data = 0;
 	data |= ((m_cass->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_STOPPED) && ((m_cass->get_state() & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_ENABLED)  ? 0x0 : 0x1;
 	data |= (m_dipsw->read() & 1) << 4;
 	data |= (m_cass->input() > 0.0038)<<1;
 	data |= m_vdg->vsync_r()<<7;
 	data &= ~((m_centronics_busy==0)<<3);
 	return data;
}

WRITE8_MEMBER( spc1500_state::crtc_w)
{
	static int m_crtc_index;
	if((offset & 1) == 0)
	{
		m_crtc_index = data & 0x1f;
		m_vdg->address_w(space, 0, data);
	}
	else
	{
		m_crtc_vreg[m_crtc_index] = data;
		m_vdg->register_w(space, 0, data);
	}
}

READ8_MEMBER( spc1500_state::crtc_r)
{
	if (offset & 1)
	{
		return m_vdg->register_r(space, 0);
	}
	return 0;
}

READ8_MEMBER( spc1500_state::pcg_r)
{
	get_pcg_addr();
	if (BIT(m_pcg_attr,5)) // PCG font
	{
		if ((offset&0x1f00) == 0x1400) 
			offset += 0x300;
		return m_pcgram[m_pcg_addr+(offset&0xf)+(((offset & 0x1f00) - 0x1500)<<3)];
	}
	else // ROM font
	{
		return m_font[(m_crtc_vreg[0x9]==15?0x1000:0)+m_pcg_addr+(offset&0xf)];
	}
	return 0;
}
void spc1500_state::get_pcg_addr()
{
	UINT16 vaddr = 0;
	if(m_p_videoram[0x7ff] & 0x20) {
		vaddr = 0x7ff;
	} else if(m_p_videoram[0x3ff] & 0x20) {
		vaddr = 0x3ff;
	} else if(m_p_videoram[0x5ff] & 0x20) {
		vaddr = 0x5ff;
	} else if(m_p_videoram[0x1ff] & 0x20) {
		vaddr = 0x1ff;
	} else {
		vaddr = 0x3ff;
	}
	m_pcg_char = m_p_videoram[0x1000 + vaddr];
	m_pcg_attr = m_p_videoram[vaddr];
	m_pcg_addr = m_pcg_char * (m_crtc_vreg[0x9]+1);
}
WRITE8_MEMBER( spc1500_state::pcg_w)
{
	get_pcg_addr();
	m_pcg_addr=m_pcg_addr+(offset&0xf)+(((offset & 0x1f00) - 0x1500)<<3);
	m_pcgram[m_pcg_addr] = data;
//	printf("pcgram:0x%04x 0x%02x\n",m_pcg_addr, data);
}

WRITE8_MEMBER( spc1500_state::priority_w)
{
	m_priority = data;
}

WRITE8_MEMBER( spc1500_state::palet_w)
{
	m_palet[(offset>>8)&0x0f] = data;
//	printf("palet:0x%02x, 0x%02x, 0x%02x\n", m_palet[0], m_palet[1], m_palet[2]);
}

PALETTE_INIT_MEMBER(spc1500_state,spc)
{
	palette.set_pen_color(0,rgb_t(0x00,0x00,0x00));
	palette.set_pen_color(1,rgb_t(0x00,0x00,0xff));
	palette.set_pen_color(2,rgb_t(0xff,0x00,0x00));
	palette.set_pen_color(3,rgb_t(0xff,0x00,0xff));
	palette.set_pen_color(4,rgb_t(0x00,0xff,0x00));
	palette.set_pen_color(5,rgb_t(0x00,0xff,0xff));
	palette.set_pen_color(6,rgb_t(0xff,0xff,0x00));
	palette.set_pen_color(7,rgb_t(0xff,0xff,0xff));
}

VIDEO_START_MEMBER(spc1500_state, spc)
{
	
}

MC6845_RECONFIGURE(spc1500_state::crtc_reconfig)
{
//	printf("reconfig. w:%d, h:%d, %f (%d,%d,%d,%d)\n", width, height, (float)frame_period, visarea.left(), visarea.top(), visarea.right(), visarea.bottom());
//	printf("register. m_vert_disp:%d, m_horiz_disp:%d, m_max_ras_addr:%d, m_vert_char_total:%d\n", m_crtc_vreg[6], m_crtc_vreg[1],  m_crtc_vreg[9], m_crtc_vreg[0x4]);
}

MC6845_UPDATE_ROW(spc1500_state::crtc_update_row)
{
	UINT8 han2;
	UINT8 *pf;
	UINT16 hfnt;
	int i;
	int j;
	int h1, h2, h3;
	UINT32  *p = &bitmap.pix32(y);
	
	unsigned char cho[] ={1,1,1,1,1,1,1,1,0,0,1,1,1,3,5,5,0,0,5,3,3,5,5,5,0,0,3,3,5,1};
	unsigned char jong[]={0,0,0,1,1,1,1,1,0,0,1,1,1,2,2,2,0,0,2,2,2,2,2,2,0,0,2,2,1,1};
	bool inv = false;
	char hs = (m_crtc_vreg[0x9] < 15 ? 3 : 4);
	int n = y & (m_crtc_vreg[0x9]);
	bool ln400 = (hs == 4 && m_crtc_vreg[0x4] > 20);
	for (i = 0; i < x_count; i++)
	{
		UINT8 *pp = &m_p_videoram[0x2000+((y>>hs)*x_count+(((y)&7)<<11))+i+(((hs==4)&&(y&8))?0x400:0)];
		UINT8 *pv = &m_p_videoram[(y>>hs)*x_count + i];
		UINT8 ascii = *(pv+0x1000);
		UINT8 attr = *pv;
		inv = (attr & 0x8 ? true : false);
		UINT8 rgb = (attr & 0x7);
		UINT8 pal = 1<<rgb;
		UINT8 pixelb = *(pp+0);
		UINT8 pixelr = *(pp+0x4000);
		UINT8 pixelg = *(pp+0x8000);
		UINT8 pen = (((m_palet[0]&pal)>0)<<2)|(((m_palet[1]&pal)>0)<<1)|(((m_palet[2]&pal)>0));
		if ((m_palet[0] | m_palet[1] | m_palet[2])==0 || ln400)
			pen = rgb;
		UINT32 color = m_palette->pen(pen);
		UINT8 pixelpen = 0;
		if (hs == 4 && ascii & 0x80)
		{
			UINT16 wpixelb = (pixelb << 8) + (*(pp+1));
			UINT16 wpixelr = (pixelr << 8) + (*(pp+0x4001));
			UINT16 wpixelg = (pixelg << 8) + (*(pp+0x8001));
			han2 = *(pv+0x1001);
			h1 = (ascii>>2)&0x1f;
			h2 = ((ascii<<3)|(han2>>5))&0x1f;
			h3 = (han2)&0x1f;
			pf = &m_font[0x2000+(h1 * 32) + (cho[h2] + (h3 != 0) -1) * 16 * 2 * 32 + n];
			hfnt = (*pf << 8) | (*(pf+16));
			pf = &m_font[0x4000+(h2 * 32) + (h3 == 0 ? 0 : 1) * 16 * 2 * 32 + n];
			hfnt = hfnt & ((*pf << 8) | (*(pf+16)));
			pf = &m_font[0x6000+(h3 * 32) + (jong[h2]-1) * 16 * 2 * 32 + n];
			hfnt = hfnt & ((*pf << 8) | (*(pf+16)));
			hfnt = (inv ? 0xffff - hfnt : hfnt);
			//printf("0x%04x\n" , hfnt);
			for (j = 0; j < 16; j++)
			{
				pixelpen = (((wpixelg&(0x8000 >> j))>0 ? 4:0 )|((wpixelr&(0x8000 >> j))>0 ? 2:0)|((wpixelb&(0x8000 >> j))>0 ? 1:0));
				*p++ = (((hfnt & (0x8000 >> j)) || (m_priority & (1<<pixelpen))) ? m_palette->pen(pixelpen) : color);
			}
			i++;
		}
		else if (attr & 0x20)
		{
			UINT8 *pa = &m_pcgram[(ascii<<hs)+n];
			UINT8 b = *pa;
			UINT8 r = *(pa+0x800);
			UINT8 g = *(pa+0x1000);
			for (j = 0; j < 8; j++)
			{
				pen = ((g & (0x80 >> j))>0?4:0)|((r & (0x80>>j))>0?2:0)|((b & (0x80>>j))>0?1:0);
				pixelpen = (((pixelg&(0x80 >> j))>0 ? 4 : 0)|((pixelr&(0x80 >> j))>0 ? 2:0)|(((pixelb&(0x80 >> j) ? 1:0 ))));
				*p++ = ((pen == 0 || (m_priority & (1<<pixelpen))) ? m_palette->pen(pixelpen) : m_palette->pen(pen));
			}
		}
		else
		{
			//printf("%c", ascii);
			UINT8 fnt = m_font[(hs == 4 ? 0x1000 : (attr & (1<<6) ? 0x80<<4 : 0)) + (ascii<<4) + n];
			if (ascii == 0 && (attr & 0x08) && inv)
			{
				fnt = 0xff;
				color = m_palette->pen(7);
			}
			fnt = (inv ? 0xff - fnt : fnt);
			for (j = 0; j < 8; j++)
			{
				pixelpen = (((pixelg&(0x80 >> j))>0 ? 4 : 0)|((pixelr&(0x80 >> j))>0 ? 2:0)|(((pixelb&(0x80 >> j) ? 1:0 ))));
				if (ascii == 0 && attr == 0 && !inv)
					*p++ = m_palette->pen(pixelpen);
				else
					*p++ = (((fnt & (0x80 >> j)) || (m_priority & (1<<pixelpen))) ? m_palette->pen(pixelpen) : color);
			}
		}
	}
}

WRITE8_MEMBER( spc1500_state::double_w)
{
	if (m_double_mode)
	{
		if (offset < 0x4000) { offset += 0x2000; m_p_videoram[offset] = m_p_videoram[offset + 0x4000] = m_p_videoram[offset + 0x8000] = data; } else
		if (offset < 0x8000) { offset += 0x2000; m_p_videoram[offset + 0x8000] = m_p_videoram[offset + 0x4000] = data; } else
		if (offset < 0xc000) { offset += 0x2000; m_p_videoram[offset] = m_p_videoram[offset + 0x8000] = data; } else
		if (offset < 0x10000){ offset += 0x2000; m_p_videoram[offset] = m_p_videoram[offset - 0x4000] = data; }
	}
	else
	{
		if (offset < 0x1000) {} else
		if (offset < 0x1300) { palet_w(space, offset, data); } else
		if (offset < 0x1400) { priority_w(space, offset, data); } else
		if (offset < 0x1800) { pcg_w(space, offset, data); } else
		if (offset < 0x1900) { crtc_w(space, offset, data); } else
		if (offset < 0x1a00) {} else
		if (offset < 0x1b00) { m_pio->write(space, offset, data);} else
		if (offset < 0x1c00) { m_sound->data_w(space, offset, data);} else
		if (offset < 0x1d00) { m_sound->address_w(space, offset, data);} else
		if (offset < 0x1e00) { romsel(space, offset, data);} else
		if (offset < 0x1f00) { ramsel(space, offset, data);} else
		if (offset < 0x2000) {} else
		if (offset < 0x10000) 
		{ 
			if (offset < 0x4000) 
			{
				offset &= 0xf7ff;
				m_p_videoram[offset-0x1800] = m_p_videoram[offset-0x2000] = data;
			}
			else
				m_p_videoram[offset-0x2000] = data; 
		};
	}
}

READ8_MEMBER( spc1500_state::io_r)
{
	m_double_mode = false;
	if (offset < 0x1000) {} else 
	if (offset < 0x1400) {} else
	if (offset < 0x1800) { return pcg_r(space, offset); } else
	if (offset < 0x1900) { return crtc_r(space, offset); } else
	if (offset < 0x1a00) { return keyboard_r(space, offset); } else
	if (offset < 0x1b00) { return m_pio->read(space, offset); } else
	if (offset < 0x1c00) { return m_sound->data_r(space, offset); } else
	if (offset < 0x2000) {} else
	if (offset < 0x10000){ 
		if (offset < 0x4000)
			offset &= 0xf7ff;
		return m_p_videoram[offset - 0x2000]; }
	return 0xff;
}

static ADDRESS_MAP_START( spc1500_double_io , AS_IO, 8, spc1500_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(io_r, double_w)
	AM_RANGE(0x2000, 0xffff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x0000, 0x17ff) AM_RAM AM_SHARE("pcgram")
ADDRESS_MAP_END

#if 0
static ADDRESS_MAP_START( spc1500_io , AS_IO, 8, spc1500_state )
	ADDRESS_MAP_UNMAP_HIGH
//	AM_RANGE(0x0000, 0x03ff) AM_DEVREADWRITE("userio", user_device, userio_r, userio_w)
//	AM_RANGE(0x0400, 0x05ff) AM_DEVREADWRITE("lanio", lan_device, lanio_r, lanio_w)
//	AM_RANGE(0x0600, 0x07ff) AM_DEVREADWRITE("rs232c", rs232c_device, rs232c_r, rs232c_w)
//	AM_RANGE(0x0800, 0x09ff) AM_DEVREADWRITE("fdcx", fdcx_device, fdcx_r, fdcx_w)
//	AM_RANGE(0x0a00, 0x0bff) AM_DEVREADWRITE("userio", user_device, userio_r, userio_w)
//	AM_RANGE(0x0c00, 0x0dff) AM_DEVREADWRITE("fdc", fdc_device, fdc_r, fdc_w)
//	AM_RANGE(0x0e00, 0x0fff) AM_DEVREADWRITE("extram", extram_device, extram_r, extram_w)
	AM_RANGE(0x1000, 0x10ff) AM_WRITE(paletb_w)
	AM_RANGE(0x1100, 0x11ff) AM_WRITE(paletr_w)
	AM_RANGE(0x1200, 0x12ff) AM_WRITE(paletg_w)
	AM_RANGE(0x1300, 0x13ff) AM_WRITE(priority_w)
	AM_RANGE(0x1400, 0x14ff) AM_READ(pcgg_r)
	AM_RANGE(0x1500, 0x15ff) AM_READWRITE(pcgb_r, pcgb_w)
	AM_RANGE(0x1600, 0x16ff) AM_READWRITE(pcgr_r, pcgr_w)
	AM_RANGE(0x1700, 0x17ff) AM_WRITE(pcgg_w)
	AM_RANGE(0x1800, 0x18ff) AM_READWRITE(crtc_r, crtc_w)
//	AM_RANGE(0x1800, 0x1800) AM_DEVWRITE("mc6845", mc6845_device, address_w)
//	AM_RANGE(0x1801, 0x1801) AM_DEVREADWRITE("mc6845", mc6845_device, register_r, register_w)
//	AM_RANGE(0x1800, 0x1801) AM_READWRITE(crtc_r, crtc_w)
	AM_RANGE(0x1900, 0x1909) AM_READ(keyboard_r)
 	AM_RANGE(0x1a00, 0x1a03) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0x1b00, 0x1bff) AM_DEVREADWRITE("ay8910", ay8910_device, data_r, data_w)
	AM_RANGE(0x1c00, 0x1cff) AM_DEVWRITE("ay8910", ay8910_device, address_w)
	AM_RANGE(0x1d00, 0x1d00) AM_WRITE(romsel)
	AM_RANGE(0x1e00, 0x1e00) AM_WRITE(ramsel)
	AM_RANGE(0x2000, 0xffff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END
#endif 

/* Input ports */
static INPUT_PORTS_START( spc1500 )

 	PORT_START("DIP_SWITCH") 
 	PORT_DIPNAME( 0x01, 0x00, "40/80" )
 	PORT_DIPSETTING(    0x00, "40COL" )
 	PORT_DIPSETTING(    0x01, "80COL" )
 	PORT_DIPNAME( 0x02, 0x02, "Language" )
 	PORT_DIPSETTING(    0x02, "Korean" )
 	PORT_DIPSETTING(    0x00, "English" )
	PORT_DIPNAME( 0x04, 0x00, "V-Res" )
 	PORT_DIPSETTING(    0x04, "400" )
	PORT_DIPSETTING(    0x00, "200" )
	PORT_DIPNAME( 0x08, 0x08, "X1" )
 	PORT_DIPSETTING(    0x08, "Compatible Mode" )
	PORT_DIPSETTING(    0x00, "Non Compatible" )
 
	PORT_START("LINE.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) 
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_RSHIFT) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_RCONTROL) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED) 
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_PAUSE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|') PORT_CHAR(0x1c)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Graph") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("= +") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+') 
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x16)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("LINE.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}') PORT_CHAR(0x1d)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 @") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')

	PORT_START("LINE.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)  PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(0x1b)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("` ~") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("LINE.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Del Ins") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)   
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Home") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0e)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')

	PORT_START("LINE.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) 
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED) 
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR(0x0d)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("LINE.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) 
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{') PORT_CHAR(0x1b)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(0x08)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 ^") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')

	PORT_START("LINE.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) 
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED) 
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(0x0a)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(0x15)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 &") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')

	PORT_START("LINE.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) 
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED) 
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\' \"") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('\"')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x0b)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(0x09)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 *") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')

	PORT_START("LINE.9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Hangul") PORT_CODE(KEYCODE_RALT)    
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0c)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0e)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 (") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')

	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(0x50, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
INPUT_PORTS_END

static ADDRESS_MAP_START(spc1500_mem, AS_PROGRAM, 8, spc1500_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_READ_BANK("bank1") AM_WRITE_BANK("bank2") 
	AM_RANGE(0x8000, 0xffff) AM_READWRITE_BANK("bank4")
ADDRESS_MAP_END

void spc1500_state::machine_start()
{
	UINT8 *mem_basic = memregion("basic")->base();
	UINT8 *mem_ipl = memregion("ipl")->base();
	m_p_ram = m_ram->pointer();
	m_font = memregion("font1")->base();	
	// configure and intialize banks 1 (read banks)
	membank("bank1")->configure_entry(0, mem_ipl);
	membank("bank1")->configure_entry(1, mem_basic);
	membank("bank1")->configure_entry(2, m_p_ram);
	membank("bank1")->set_entry(0);
	m_romsel = 1;
	static_set_addrmap(m_maincpu, AS_IO, ADDRESS_MAP_NAME(spc1500_double_io));
	set_address_space(AS_IO, m_maincpu->space(AS_IO));
	// intialize banks 2, 3, 4 (write banks)
	membank("bank2")->set_base(m_p_ram);
	membank("bank4")->set_base(m_p_ram + 0x8000);	
}

void spc1500_state::machine_reset()
{
	m_motor = false;
   	m_time = machine().scheduler().time();	
	m_double_mode = false;
}

READ8_MEMBER(spc1500_state::mc6845_videoram_r)
{
	return m_p_videoram[offset];
}

READ8_MEMBER( spc1500_state::psga_r )
{
	UINT8 data = 0;
	data |= (BIT(m_dipsw->read(),1)<<4) | (BIT(m_dipsw->read(),2)<<7);
	return data;
}

READ8_MEMBER( spc1500_state::porta_r )
{
	UINT8 data = 0x3f;
	data |= (m_cass->input() > 0.0038) ? 0x80 : 0;
	data |= ((m_cass->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_STOPPED) && ((m_cass->get_state() & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_ENABLED)  ? 0x00 : 0x40;
	data |= (m_io_joy->read() & 0x6f);
	data &= ~((m_centronics_busy == 0)<< 5);
	return data;
}

WRITE_LINE_MEMBER( spc1500_state::irq_w )
{
	m_maincpu->set_input_line(0, state ? CLEAR_LINE : HOLD_LINE);
}

static MACHINE_CONFIG_START( spc1500, spc1500_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(spc1500_mem)
	//MCFG_CPU_IO_MAP(spc1500_io)
	MCFG_CPU_IO_MAP(spc1500_double_io)
	MCFG_CPU_PERIODIC_INT_DRIVER(spc1500_state, irq0_line_hold,  60)

	/* video hardware */
	
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0,640-1,0,400-1)
	//MCFG_MC6845_VISAREA_ADJUST(50,50,640-50,400-50)
	MCFG_SCREEN_UPDATE_DEVICE("mc6845", mc6845_device, screen_update )
	MCFG_PALETTE_ADD("palette", 8)	
	MCFG_PALETTE_INIT_OWNER(spc1500_state, spc)
	MCFG_MC6845_ADD("mc6845", MC6845, "screen", (VDP_CLOCK/48)) //unknown divider
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(spc1500_state, crtc_update_row)
	MCFG_MC6845_RECONFIGURE_CB(spc1500_state, crtc_reconfig)
	MCFG_VIDEO_START_OVERRIDE(spc1500_state, spc)	
	
	MCFG_DEVICE_ADD("ppi8255", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_I8255_IN_PORTB_CB(READ8(spc1500_state, portb_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(spc1500_state, portb_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(spc1500_state, portc_w))
	
	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8910", AY8910, XTAL_4MHz / 2)
	MCFG_AY8910_PORT_A_READ_CB(READ8(spc1500_state, psga_r))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(spc1500_state, psgb_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(spc1500_state, centronics_busy_w))
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")
	MCFG_DEVICE_ADD("cent_status_in", INPUT_BUFFER, 0)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(spc1000_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_DISABLED)

	MCFG_SOFTWARE_LIST_ADD("cass_list", "spc1500_cass")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( spc1500 )
	ROM_REGION(0x8000, "ipl", ROMREGION_ERASEFF)
	ROM_LOAD("ipl.rom", 0x0000, 0x8000, CRC(80d0704a) SHA1(01e4cbe8baad72effbbe01addd477c5b0ec85c16))
	ROM_REGION(0x8000, "basic", ROMREGION_ERASEFF)
	ROM_LOAD("basic.rom", 0x0000, 0x8000, CRC(f48328e1) SHA1(fb874ea7d20078726682f2d0e03ea0d1f8bdbb07))
	ROM_REGION(0x8000, "font1", 0) 
	ROM_LOAD( "ss150fnt.bin", 0x0000, 0x2000, CRC(affdc5c0) SHA1(2a93582fcccf9e40b99ae238ce585d189afe9a5a) )
	ROM_LOAD( "ss151fnt.bin", 0x2000, 0x2000, CRC(83c2eb8d) SHA1(2adf7816206dc74b9f0d32cb3b56cbab31fa6044) )
	ROM_LOAD( "ss152fnt.bin", 0x4000, 0x2000, CRC(f4a5a590) SHA1(c9a02756107083bf602ae7c90cfe29b8b964e0df) )
	ROM_LOAD( "ss153fnt.bin", 0x6000, 0x2000, CRC(8677d5fa) SHA1(34bfacc855c3846744cd586c150c72e5cbe948b0) )
	
ROM_END


/* Driver */

/*    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT    CLASS         INIT    COMPANY    FULLNAME       FLAGS */
COMP( 1987, spc1500,  0,      0,       spc1500,   spc1500, driver_device,  0,   "Samsung", "SPC-1500", 0 )

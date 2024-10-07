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
  2016-01-10 double character support
  2016-01-12 PCG addressing improved
  2016-01-13 Cassette tape motor improved

TODO:
  - Verify PCG ram read for Korean character (English character is fine)
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

 Peripherals
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
  It equipped SPC-1500V VLSI chip embedded products and removed a lot of TTLs and the memory expansion card.
  - IOCS ROM Version: 1.6
  - Two internal 50-pin expansion slots

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
   - Initial (Choseong)  SS151-1223: 8 types of initial character (actual 6 types)
   - Middle (Jungseong) SS152-1224: 2 types of middle character
   - Final  (Jongseong) SS153-1225: 2 types of final character

  Peripherals - Monitor
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
   - The two models are identical except the number of FDD. They need the expansion controller card named by SFC-1500.
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
   - Recommended 80 columns dot-matrix printer
  2) SP-570H
   - Recommended 132 columns dot-matrix printer
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
   - External ROM cartridge from Static Soft (C)
   - 1 cartridge slot and 3 expansion slots (up to five expansion slots available)
   - It is available to use the MSX ROM packs without any modification with the static soft VDP card
   - Release price: 60,000won ($71)

  ST-KEY2
   - For synthesizer external keyboard

  * Compatibility with X1 series of Sharp Electronics
   - Almost the key components is the same as X1 models of Sharp Electronics and except for the keyboard input.
   - To port the X1 software to SPC-1500, Text attribute, keyboard input and DMA related code should be modified

*/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/spc1000_cas.h"

#include "utf8.h"


namespace {

#define VDP_CLOCK  XTAL(42'954'545)

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
		, m_io_kb(*this, "LINE.%u", 0)
		, m_io_joy(*this, "JOY")
		, m_dipsw(*this, "DIP_SWITCH")
		, m_centronics(*this, "centronics")
		, m_pio(*this, "ppi8255")
		, m_sound(*this, "ay8910")
		, m_palette(*this, "palette")
	{ }

	void spc1500(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;


private:
	uint8_t psga_r();
	uint8_t porta_r();
	void centronics_busy_w(int state) { m_centronics_busy = state; }
	uint8_t mc6845_videoram_r(offs_t offset);
	uint8_t keyboard_r(offs_t offset);
	void palet_w(offs_t offset, uint8_t data);
	void priority_w(uint8_t data);
	void pcg_w(offs_t offset, uint8_t data);
	uint8_t pcg_r(offs_t offset);
	void crtc_w(offs_t offset, uint8_t data);
	uint8_t crtc_r(offs_t offset);
	void romsel(uint8_t data);
	void ramsel(uint8_t data);
	void portb_w(uint8_t data);
	void psgb_w(uint8_t data);
	void portc_w(uint8_t data);
	uint8_t portb_r();
	void double_w(offs_t offset, uint8_t data);
	uint8_t io_r(offs_t offset);
	void spc_palette(palette_device &palette) const;
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_RECONFIGURE(crtc_reconfig);
	TIMER_DEVICE_CALLBACK_MEMBER(timer);

	void spc1500_double_io(address_map &map) ATTR_COLD;
	void spc1500_mem(address_map &map) ATTR_COLD;

	uint8_t *m_p_ram = nullptr;
	uint8_t m_ipl = 0;
	uint8_t m_palet[3]{};
	uint8_t m_paltbl[8]{};
	uint8_t m_pcg_char = 0, m_pcg_attr = 0, m_char_change = 0;
	uint16_t m_pcg_offset[3]{};
	int m_char_count = 0;
	attotime m_time;
	bool m_romsel = false;
	bool m_double_mode = false;
	bool m_p5bit = false;
	bool m_motor = false;
	bool m_motor_toggle = false;
	uint8_t m_crtc_vreg[0x100]{};
	bool m_centronics_busy = false;
	required_device<z80_device> m_maincpu;
	required_device<mc6845_device> m_vdg;
	required_device<cassette_image_device> m_cass;
	required_device<ram_device> m_ram;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_shared_ptr<uint8_t> m_pcgram;
	required_ioport_array<10> m_io_kb;
	required_ioport m_io_joy;
	required_ioport m_dipsw;
	required_device<centronics_device> m_centronics;
	required_device<i8255_device> m_pio;
	required_device<ay8910_device> m_sound;
	required_device<palette_device> m_palette;
	uint8_t *m_font = nullptr;
	uint8_t m_priority = 0;
	emu_timer *m_timer = nullptr;
	void get_pcg_addr();
};

uint8_t spc1500_state::keyboard_r(offs_t offset)
{
	offset &= 0xf;

	if (offset <= 9)
		return m_io_kb[offset]->read();
	else
		return 0xff;
}

void spc1500_state::romsel(uint8_t data)
{
	m_romsel = 1;
	if (m_ipl)
		membank("bank1")->set_entry(0);
	else
		membank("bank1")->set_entry(1);
}

void spc1500_state::ramsel(uint8_t data)
{
	m_romsel = 0;
	membank("bank1")->set_entry(2);
}

void spc1500_state::portb_w(uint8_t data)
{
//  m_ipl = data & (1 << 1);
}

void spc1500_state::psgb_w(uint8_t data)
{
	int elapsed_time = m_timer->elapsed().as_attoseconds()/ATTOSECONDS_PER_MICROSECOND;
	if (m_ipl != ((data>>1)&1))
	{
		m_ipl = ((data>>1)&1);
		membank("bank1")->set_entry(m_ipl ? 0 : 1);
	}
	//m_cass->change_state(BIT(data, 6) ? CASSETTE_SPEAKER_ENABLED : CASSETTE_SPEAKER_MUTED, CASSETTE_MASK_SPEAKER);
	if (m_motor && !BIT(data, 7) && (elapsed_time > 100))
	{
		m_cass->change_state((m_cass->get_state() & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_DISABLED ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
		m_timer->reset();
	}
	m_motor = BIT(data, 7);
}

void spc1500_state::portc_w(uint8_t data)
{
	m_cass->output(BIT(data, 0) ? -1.0 : 1.0);
	m_centronics->write_strobe(BIT(data, 7));
	m_double_mode = (!m_p5bit && BIT(data, 5)); // double access I/O mode
	m_p5bit = BIT(data, 5);
	m_vdg->set_unscaled_clock(VDP_CLOCK/(BIT(data, 2) ? 48 : 24));
}

uint8_t spc1500_state::portb_r()
{
	uint8_t data = 0;
	data |= ((m_cass->get_state() & CASSETTE_MASK_UISTATE) == CASSETTE_STOPPED || ((m_cass->get_state() & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_DISABLED));
	data |= (m_dipsw->read() & 1) << 4;
	data |= (m_cass->input() > 0.0038)<<1;
	data |= m_vdg->vsync_r()<<7;
	data &= ~((m_centronics_busy==0)<<3);
	return data;
}

void spc1500_state::crtc_w(offs_t offset, uint8_t data)
{
	static int m_crtc_index;
	if((offset & 1) == 0)
	{
		m_crtc_index = data & 0x1f;
		m_vdg->address_w(data);
	}
	else
	{
		m_crtc_vreg[m_crtc_index] = data;
		m_vdg->register_w(data);
	}
}

uint8_t spc1500_state::crtc_r(offs_t offset)
{
	if (offset & 1)
	{
		return m_vdg->register_r();
	}
	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(spc1500_state::timer)
{
	if(m_motor_toggle == true)
	{
		m_cass->change_state((m_cass->get_state() & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_DISABLED ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
		m_motor_toggle = false;
	}
}

void spc1500_state::get_pcg_addr()
{
	uint16_t vaddr = 0;
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
	if (m_pcg_char != m_char_change)
	{
		m_char_change = m_pcg_char;
		m_pcg_offset[0] = 0;
		m_pcg_offset[1] = 0;
		m_pcg_offset[2] = 0;
	}
}

void spc1500_state::pcg_w(offs_t offset, uint8_t data)
{
	int reg = (offset>>8)-0x15;
	get_pcg_addr();

	m_pcgram[m_pcg_char * 8 + m_pcg_offset[reg] + (reg*0x800)] = data;
	if (m_pcg_offset[reg] == 7)
		m_pcg_offset[reg] = 0;
	else
		m_pcg_offset[reg]++;
}

uint8_t spc1500_state::pcg_r(offs_t offset)
{
	int reg = (offset>>8)-0x15;
	uint8_t data = 0;
	get_pcg_addr();
	if (reg < 0) reg = 2;
	if (BIT(m_pcg_attr,5)) // PCG font
	{
		data = m_pcgram[m_pcg_char * 8 + m_pcg_offset[reg]+(reg*0x800)];
	}
	else // ROM font
	{
		data = m_font[(m_crtc_vreg[0x9]==15?0x1000:0)+(m_pcg_char * 16)+m_pcg_offset[reg]];
	}
	if (m_pcg_offset[reg]++ > m_crtc_vreg[0x9]-1)
		m_pcg_offset[reg] = 0;
	return data;
}

void spc1500_state::priority_w(uint8_t data)
{
	m_priority = data;
}

void spc1500_state::palet_w(offs_t offset, uint8_t data)
{
	m_palet[(offset>>8)&0x0f] = data;
	for(int i=1, j=0; i < 0x100; i<<=1, j++)
	{
		m_paltbl[j] = (m_palet[0]&i?1:0)|(m_palet[1]&i?2:0)|(m_palet[2]&i?4:0);
	}
}

void spc1500_state::spc_palette(palette_device &palette) const
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


MC6845_RECONFIGURE(spc1500_state::crtc_reconfig)
{
//  printf("reconfig. w:%d, h:%d, %f (%d,%d,%d,%d)\n", width, height, (float)frame_period, visarea.left(), visarea.top(), visarea.right(), visarea.bottom());
//  printf("register. m_vert_disp:%d, m_horiz_disp:%d, m_max_ras_addr:%d, m_vert_char_total:%d\n", m_crtc_vreg[6], m_crtc_vreg[1],  m_crtc_vreg[9], m_crtc_vreg[0x4]);
}

MC6845_UPDATE_ROW(spc1500_state::crtc_update_row)
{
	uint8_t const *pf;
	uint16_t hfnt;
	uint32_t *p = &bitmap.pix(y);

	unsigned char cho[] ={1,1,1,1,1,1,1,1,0,0,1,1,1,3,5,5,0,0,5,3,3,5,5,5,0,0,3,3,5,1};
	unsigned char jong[]={0,0,0,1,1,1,1,1,0,0,1,1,1,2,2,2,0,0,2,2,2,2,2,2,0,0,2,2,1,1};
	char hs = (m_crtc_vreg[0x9] < 15 ? 3 : 4);
	int n = y & (m_crtc_vreg[0x9]);
	bool ln400 = (hs == 4 && m_crtc_vreg[0x4] > 20);
	uint8_t const *const vram = &m_p_videoram[0] + (m_crtc_vreg[12] << 8) + m_crtc_vreg[13];
	for (int i = 0; i < x_count; i++)
	{
		uint8_t const *const pp = &vram[0x2000+((y>>hs)*x_count+(((y)&7)<<11))+i+(((hs==4)&&(y&8))?0x400:0)];
		uint8_t const *const pv = &vram[(y>>hs)*x_count + i];
		uint8_t ascii = *(pv+0x1000);
		uint8_t attr = *pv;
		bool inv = (attr & 0x8 ? true : false);
		uint8_t color = attr & 0x7;
		uint8_t pixelb = *(pp+0);
		uint8_t pixelr = *(pp+0x4000);
		uint8_t pixelg = *(pp+0x8000);
		bool nopalet = ((m_palet[0] | m_palet[1] | m_palet[2])==0 || ln400);
		uint8_t pen = (nopalet ? color : m_paltbl[color]);
		if (hs == 4 && (ascii & 0x80))
		{
			uint16_t wpixelb = (pixelb << 8) + (*(pp+1));
			uint16_t wpixelr = (pixelr << 8) + (*(pp+0x4001));
			uint16_t wpixelg = (pixelg << 8) + (*(pp+0x8001));
			if (ascii != 0xfa)
			{
				uint8_t han2 = *(pv+0x1001);
				int h1 = (ascii>>2)&0x1f;
				int h2 = ((ascii<<3)|(han2>>5))&0x1f;
				int h3 = (han2)&0x1f;
				pf = &m_font[0x2000+(h1 * 32) + (cho[h2] + (h3 != 0) -1) * 16 * 2 * 32 + n];
				hfnt = (*pf << 8) | (*(pf+16));
				pf = &m_font[0x4000+(h2 * 32) + (h3 == 0 ? 0 : 1) * 16 * 2 * 32 + n];
				hfnt = hfnt & ((*pf << 8) | (*(pf+16)));
				pf = &m_font[0x6000+(h3 * 32) + (jong[h2]-1) * 16 * 2 * 32 + n];
				hfnt = hfnt & ((*pf << 8) | (*(pf+16)));
			}
			else
			{
				ascii = *(pv+0x1001);
				pf = &m_font[0x6000+(ascii*32) + n];
				hfnt = (*pf << 8) | (*(pf+16));
			}
			hfnt = (inv ? 0xffff - hfnt : hfnt);
			for (int j = 0x8000; j > 0; j>>=1)
			{
				uint8_t pixel = ((wpixelg&j ? 4:0 )|(wpixelr&j? 2:0)|(wpixelb&j ? 1:0));
				uint8_t pixelpen = (nopalet ? pixel : m_paltbl[pixel]);
				*p++ = m_palette->pen(((hfnt & j) || (m_priority & (1<<pixel))) ? pixelpen : pen);
			}
			i++;
		}
		else if (attr & 0x20)
		{
			uint8_t const *pa = &m_pcgram[(ascii*(m_crtc_vreg[0x9]+1))+n];
			uint8_t b = *pa;
			uint8_t r = *(pa+0x800);
			uint8_t g = *(pa+0x1000);
			for (int j = 0x80; j > 0; j>>=1)
			{
				uint8_t pixel = ((g & j)?4:0)|((r & j)?2:0)|((b & j)?1:0);
				pen = (pixel == 7 ? color : pixel);
				pixel = (pixelg&j ? 4 : 0)|(pixelr&j ? 2:0)|(pixelb&j ? 1:0 );
				uint8_t pixelpen = (nopalet ? pixel : m_paltbl[pixel]);
				*p++ = m_palette->pen((m_priority & (1<<pixel)) ? pixelpen : pen);
			}
		}
		else
		{
			uint8_t fnt = m_font[(hs == 4 ? 0x1000 : (attr & (1<<6) ? 0x80<<4 : 0)) + (ascii<<4) + n];
			if (ascii == 0 && (attr & 0x08) && inv)
			{
				fnt = 0xff;
			}
			fnt = (inv ? 0xff - fnt : fnt);
			for (int j = 0x80; j > 0; j>>=1)
			{
				uint8_t pixel = ((pixelg&j) ? 4 : 0)|(pixelr&j ? 2:0)|(pixelb&j ? 1:0 );
				uint8_t pixelpen = (nopalet ? pixel : m_paltbl[pixel]);
				if (ascii == 0 && attr == 0 && !inv)
					*p++ = m_palette->pen(pixelpen);
				else
					*p++ = m_palette->pen(((fnt & j) || (m_priority & (1<<pixel))) ? pixelpen : pen);
			}
		}
	}
}

void spc1500_state::double_w(offs_t offset, uint8_t data)
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
		if (offset < 0x1300) { palet_w(offset, data); } else
		if (offset < 0x1400) { priority_w(data); } else
		if (offset < 0x1800) { pcg_w(offset, data); } else
		if (offset < 0x1900) { crtc_w(offset, data); } else
		if (offset < 0x1a00) {} else
		if (offset < 0x1b00) { m_pio->write(offset, data); } else
		if (offset < 0x1c00) { m_sound->data_w(data);} else
		if (offset < 0x1d00) { m_sound->address_w(data);} else
		if (offset < 0x1e00) { romsel(data);} else
		if (offset < 0x1f00) { ramsel(data);} else
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

uint8_t spc1500_state::io_r(offs_t offset)
{
	m_double_mode = false;
	if (offset < 0x1000) {} else
	if (offset < 0x1400) {} else
	if (offset < 0x1800) { return pcg_r(offset); } else
	if (offset < 0x1900) { return crtc_r(offset); } else
	if (offset < 0x1a00) { return keyboard_r(offset); } else
	if (offset < 0x1b00) { return m_pio->read(offset); } else
	if (offset < 0x1c00) { return m_sound->data_r(); } else
	if (offset < 0x2000) {} else
	if (offset < 0x10000){
		if (offset < 0x4000)
			offset &= 0xf7ff;
		return m_p_videoram[offset - 0x2000]; }
	return 0xff;
}

void spc1500_state::spc1500_double_io(address_map &map)
{
	map.unmap_value_high();
	map(0x2000, 0xffff).ram().share("videoram");
	map(0x0000, 0x17ff).ram().share("pcgram");
	map(0x0000, 0xffff).rw(FUNC(spc1500_state::io_r), FUNC(spc1500_state::double_w));
}

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
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x16)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("LINE.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}') PORT_CHAR(0x1d)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 @") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')

	PORT_START("LINE.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_INSERT) PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)  PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(0x1b)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("` ~") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("LINE.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Del Ins") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
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
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_HIGH,IPT_UNUSED) // DIP SW2 for Korean/English
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_HIGH,IPT_UNUSED) // DIP SW3 for 200/400 line
INPUT_PORTS_END

void spc1500_state::spc1500_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).bankr("bank1").bankw("bank2");
	map(0x8000, 0xffff).bankrw("bank4");
}

void spc1500_state::machine_start()
{
	uint8_t *mem_basic = memregion("basic")->base();
	uint8_t *mem_ipl = memregion("ipl")->base();
	m_p_ram = m_ram->pointer();
	m_font = memregion("font1")->base();
	// configure and intialize banks 1 (read banks)
	membank("bank1")->configure_entry(0, mem_ipl);
	membank("bank1")->configure_entry(1, mem_basic);
	membank("bank1")->configure_entry(2, m_p_ram);
	membank("bank1")->set_entry(0);
	m_romsel = 1;
	// intialize banks 2, 3, 4 (write banks)
	membank("bank2")->set_base(m_p_ram);
	membank("bank4")->set_base(m_p_ram + 0x8000);
	m_timer = machine().scheduler().timer_alloc(timer_expired_delegate());
	m_timer->adjust(attotime::zero);
}

void spc1500_state::machine_reset()
{
	m_motor = false;
	m_time = machine().scheduler().time();
	m_double_mode = false;
	memset(&m_paltbl[0], 1, 8);
	m_char_count = 0;
}

[[maybe_unused]] uint8_t spc1500_state::mc6845_videoram_r(offs_t offset)
{
	return m_p_videoram[offset];
}

uint8_t spc1500_state::psga_r()
{
	uint8_t data = 0;
	data |= (BIT(m_dipsw->read(),1)<<4) | (BIT(m_dipsw->read(),2)<<7);
	data |= (m_io_joy->read() & 0x6f);
	return data;
}

[[maybe_unused]] uint8_t spc1500_state::porta_r()
{
	uint8_t data = 0x3f;
	data |= (m_cass->input() > 0.0038) ? 0x80 : 0;
	data |= ((m_cass->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_STOPPED) && ((m_cass->get_state() & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_ENABLED)  ? 0x00 : 0x40;
	data &= ~((m_centronics_busy == 0)<< 5);
	return data;
}

void spc1500_state::spc1500(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &spc1500_state::spc1500_mem);
	//m_maincpu->set_addrmap(AS_IO, &spc1500_state::spc1500_io);
	m_maincpu->set_addrmap(AS_IO, &spc1500_state::spc1500_double_io);
	m_maincpu->set_periodic_int(FUNC(spc1500_state::irq0_line_hold), attotime::from_hz(60));

	/* video hardware */

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 400);
	screen.set_visarea(0,640-1,0,400-1);
	screen.set_screen_update("mc6845", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, FUNC(spc1500_state::spc_palette), 8);

	MC6845(config, m_vdg, (VDP_CLOCK/48)); //unknown divider
	m_vdg->set_screen("screen");
	m_vdg->set_show_border_area(false);
	m_vdg->set_char_width(8);
	m_vdg->set_update_row_callback(FUNC(spc1500_state::crtc_update_row));
	m_vdg->set_reconfigure_callback(FUNC(spc1500_state::crtc_reconfig));

	I8255(config, m_pio);
	m_pio->out_pa_callback().set("cent_data_out", FUNC(output_latch_device::write));
	m_pio->in_pb_callback().set(FUNC(spc1500_state::portb_r));
	m_pio->out_pb_callback().set(FUNC(spc1500_state::portb_w));
	m_pio->out_pc_callback().set(FUNC(spc1500_state::portc_w));

	TIMER(config, "1hz").configure_periodic(FUNC(spc1500_state::timer), attotime::from_hz(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_sound, XTAL(4'000'000) / 2);
	m_sound->port_a_read_callback().set(FUNC(spc1500_state::psga_r));
	m_sound->port_b_write_callback().set(FUNC(spc1500_state::psgb_w));
	m_sound->add_route(ALL_OUTPUTS, "mono", 1.00);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(spc1500_state::centronics_busy_w));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	INPUT_BUFFER(config, "cent_status_in");

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_DISABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cass->set_formats(spc1000_cassette_formats);
	m_cass->set_interface("spc1500_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("spc1500_cass");

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K");
}

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

} // Anonymous namespace


/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY    FULLNAME    FLAGS
COMP( 1987, spc1500, 0,      0,      spc1500, spc1500, spc1500_state, empty_init, "Samsung", "SPC-1500", 0 )

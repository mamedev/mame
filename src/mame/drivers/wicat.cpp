// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/***************************************************************************

Wicat - various systems.

2013-09-01 Skeleton driver

****************************************************************************/

/*

    TODO:

    - video DMA is done line by line and needs to be in perfect sync

*/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/8x300/8x300.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z8000/z8000.h"
#include "imagedev/floppy.h"
#include "machine/74259.h"
#include "machine/6522via.h"
#include "machine/am9517a.h"
#include "machine/im6402.h"
#include "machine/input_merger.h"
#include "machine/mc2661.h"
#include "machine/mm58274c.h"
#include "machine/wd_fdc.h"
#include "machine/x2212.h"
#include "video/i8275.h"
#include "emupal.h"
#include "screen.h"

#include "wicat.lh"

class wicat_state : public driver_device
{
public:
	wicat_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_vram(*this, "vram"),
		m_maincpu(*this, "maincpu"),
		m_rtc(*this, "rtc"),
		m_via(*this, "via"),
		m_uart(*this, "uart%u", 0U),
		m_videocpu(*this, "videocpu"),
		m_videoctrl(*this, "videoctrl"),
		m_videoirq(*this, "videoirq"),
		m_crtc(*this, "video"),
		m_videodma(*this, "videodma"),
		m_videouart0(*this, "videouart0"),
		m_videouart1(*this, "videouart1"),
		m_videouart(*this, "videouart"),
		m_videosram(*this, "vsram"),
		m_palette(*this, "palette"),
		m_chargen(*this, "g2char"),
		m_fdc(*this,"fdc")
	{
	}

	void wicat(machine_config &config);

private:
	DECLARE_READ16_MEMBER(invalid_r);
	DECLARE_WRITE16_MEMBER(invalid_w);
	DECLARE_READ16_MEMBER(memmap_r);
	DECLARE_WRITE16_MEMBER(memmap_w);
	DECLARE_WRITE_LINE_MEMBER(adir_w);
	DECLARE_WRITE_LINE_MEMBER(bdir_w);
	DECLARE_WRITE8_MEMBER(via_a_w);
	DECLARE_WRITE8_MEMBER(via_b_w);
	DECLARE_READ8_MEMBER(video_r);
	DECLARE_WRITE8_MEMBER(video_w);
	DECLARE_READ8_MEMBER(video_dma_r);
	DECLARE_WRITE8_MEMBER(video_dma_w);
	DECLARE_READ8_MEMBER(video_uart0_r);
	DECLARE_WRITE8_MEMBER(video_uart0_w);
	DECLARE_READ8_MEMBER(video_uart1_r);
	DECLARE_WRITE8_MEMBER(video_uart1_w);
	DECLARE_READ8_MEMBER(videosram_r);
	DECLARE_WRITE8_MEMBER(videosram_w);
	DECLARE_WRITE8_MEMBER(videosram_store_w);
	DECLARE_WRITE8_MEMBER(videosram_recall_w);
	DECLARE_READ8_MEMBER(video_timer_r);
	DECLARE_WRITE8_MEMBER(video_timer_w);
	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_READ8_MEMBER(video_status_r);
	DECLARE_WRITE_LINE_MEMBER(dma_hrq_w);
	DECLARE_WRITE_LINE_MEMBER(crtc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(crtc_irq_clear_w);
	DECLARE_READ8_MEMBER(hdc_r);
	DECLARE_WRITE8_MEMBER(hdc_w);
	DECLARE_READ8_MEMBER(fdc_r);
	DECLARE_WRITE8_MEMBER(fdc_w);
	DECLARE_READ16_MEMBER(via_r);
	DECLARE_WRITE16_MEMBER(via_w);
	I8275_DRAW_CHARACTER_MEMBER(wicat_display_pixels);

	required_shared_ptr<uint8_t> m_vram;
	required_device<m68000_device> m_maincpu;
	required_device<mm58274c_device> m_rtc;
	required_device<via6522_device> m_via;
	required_device_array<mc2661_device, 7> m_uart;
	required_device<cpu_device> m_videocpu;
	required_device<ls259_device> m_videoctrl;
	required_device<input_merger_device> m_videoirq;
	required_device<i8275_device> m_crtc;
	required_device<am9517a_device> m_videodma;
	required_device<mc2661_device> m_videouart0;
	required_device<mc2661_device> m_videouart1;
	required_device<im6402_device> m_videouart;
	required_device<x2210_device> m_videosram;
	required_device<palette_device> m_palette;
	required_memory_region m_chargen;
	required_device<fd1795_device> m_fdc;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) { return 0; }
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void main_mem(address_map &map);
	void video_io(address_map &map);
	void video_mem(address_map &map);
	void wd1000_io(address_map &map);
	void wd1000_mem(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void driver_start() override;

	void poll_kb();
	void send_key(uint8_t val);

	emu_timer* m_kb_timer;
	emu_timer* m_kb_serial_timer;
	static const device_timer_id KB_TIMER = 1;
	static const device_timer_id KB_SERIAL_TIMER = 2;

	uint8_t m_portA;
	uint8_t m_portB;
	bool m_crtc_irq;
	uint16_t m_kb_data;
	uint8_t m_kb_bit;
	uint32_t m_kb_keys[8];
};


void wicat_state::main_mem(address_map &map)
{
	map.unmap_value_low();
	map.global_mask(0xffffff);
	map(0x000000, 0x001fff).rom().region("c2", 0x0000);
	map(0x020000, 0x1fffff).ram();
	map(0x200000, 0x2fffff).ram();
	map(0x300000, 0xdfffff).rw(FUNC(wicat_state::invalid_r), FUNC(wicat_state::invalid_w));
	map(0xeff800, 0xeffbff).ram();  // memory mapping SRAM, used during boot sequence for storing various data (TODO)
	map(0xeffc00, 0xeffc01).rw(FUNC(wicat_state::memmap_r), FUNC(wicat_state::memmap_w));
	map(0xf00000, 0xf00007).rw(m_uart[0], FUNC(mc2661_device::read), FUNC(mc2661_device::write)).umask16(0xff00);  // UARTs
	map(0xf00008, 0xf0000f).rw(m_uart[1], FUNC(mc2661_device::read), FUNC(mc2661_device::write)).umask16(0xff00);
	map(0xf00010, 0xf00017).rw(m_uart[2], FUNC(mc2661_device::read), FUNC(mc2661_device::write)).umask16(0xff00);
	map(0xf00018, 0xf0001f).rw(m_uart[3], FUNC(mc2661_device::read), FUNC(mc2661_device::write)).umask16(0xff00);
	map(0xf00020, 0xf00027).rw(m_uart[4], FUNC(mc2661_device::read), FUNC(mc2661_device::write)).umask16(0xff00);
	map(0xf00028, 0xf0002f).rw(m_uart[5], FUNC(mc2661_device::read), FUNC(mc2661_device::write)).umask16(0xff00);
	map(0xf00030, 0xf00037).rw(m_uart[6], FUNC(mc2661_device::read), FUNC(mc2661_device::write)).umask16(0xff00);
	map(0xf00040, 0xf0005f).rw(FUNC(wicat_state::via_r), FUNC(wicat_state::via_w));
	map(0xf00060, 0xf0007f).rw(m_rtc, FUNC(mm58274c_device::read), FUNC(mm58274c_device::write)).umask16(0xff00);
	map(0xf000d0, 0xf000d0).w("ledlatch", FUNC(ls259_device::write_nibble_d3));
	map(0xf00180, 0xf0018f).rw(FUNC(wicat_state::hdc_r), FUNC(wicat_state::hdc_w));  // WD1000
	map(0xf00190, 0xf0019f).rw(FUNC(wicat_state::fdc_r), FUNC(wicat_state::fdc_w));  // FD1795
	map(0xf00f00, 0xf00fff).rw(FUNC(wicat_state::invalid_r), FUNC(wicat_state::invalid_w));
}

void wicat_state::video_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("g1", 0x0000);
	map(0x8000, 0xffff).ram();
}

void wicat_state::video_io(address_map &map)
{
	// these are largely wild guesses...
	map(0x0000, 0x0003).rw(FUNC(wicat_state::video_timer_r), FUNC(wicat_state::video_timer_w));  // some sort of timer?
	map(0x0100, 0x0107).rw(FUNC(wicat_state::video_uart0_r), FUNC(wicat_state::video_uart0_w));  // INS2651 UART #1
	map(0x0200, 0x0207).rw(FUNC(wicat_state::video_uart1_r), FUNC(wicat_state::video_uart1_w));  // INS2651 UART #2
	map(0x0304, 0x0304).r(FUNC(wicat_state::video_status_r));
	map(0x0400, 0x047f).rw(FUNC(wicat_state::videosram_r), FUNC(wicat_state::videosram_w));  // XD2210  4-bit NOVRAM
	map(0x0500, 0x0500).w(FUNC(wicat_state::videosram_recall_w));
	map(0x0600, 0x0600).w(FUNC(wicat_state::videosram_store_w));
	map(0x0800, 0x0807).w("videoctrl", FUNC(ls259_device::write_d0)).umask16(0xffff);
	map(0x0a00, 0x0a1f).rw(FUNC(wicat_state::video_dma_r), FUNC(wicat_state::video_dma_w)); // AM9517A DMA
	map(0x0b00, 0x0b03).rw(FUNC(wicat_state::video_r), FUNC(wicat_state::video_w));  // i8275 CRTC
	map(0x0e00, 0x0eff).ram();
	map(0x4000, 0x5fff).ram().share("vram"); // video RAM?
	map(0x8000, 0x8fff).rom().region("g2char", 0x0000);
	map(0x9000, 0x9fff).rom().region("g2char", 0x0000);
}

void wicat_state::wd1000_mem(address_map &map)
{
	map(0x0000, 0x0bff).rom().region("wd3", 0x0000);
	map(0x0c00, 0x0fff).noprw();
}

void wicat_state::wd1000_io(address_map &map)
{
	map(0x0000, 0x00ff).ram();  // left bank  - RAM
	map(0x0100, 0x01ff).ram();  // right bank - I/O ports (TODO)
}


/* Input ports */
static INPUT_PORTS_START( wicat )
	PORT_START("kb0")
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(7)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Retrn") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

	PORT_START("kb1")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')

	PORT_START("kb2")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')

	PORT_START("kb3")
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Unknown (7F)") PORT_CODE(KEYCODE_F4)

	PORT_START("kb4")

	PORT_START("kb5")
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Unknown (AA)") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Unknown (B0)") PORT_CODE(KEYCODE_F3)

	PORT_START("kb6")

	PORT_START("kb7")
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Set-up (FE)") PORT_CODE(KEYCODE_F1)

INPUT_PORTS_END

static void wicat_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void wicat_state::driver_start()
{
	m_kb_timer = timer_alloc(KB_TIMER);
	m_kb_serial_timer = timer_alloc(KB_SERIAL_TIMER);
}

void wicat_state::machine_start()
{
}

void wicat_state::machine_reset()
{
	// on the terminal board /DCD on both INS2651s are tied to GND
	m_videouart0->dcd_w(0);
	m_videouart1->dcd_w(0);
	for (int i = 0; i < 6; i++)
		m_uart[i]->dcd_w(0);

	// initialise im6402 (terminal board)
	m_videouart->cls1_w(1);
	m_videouart->cls2_w(1);
	m_videouart->pi_w(1);
	m_videouart->sbs_w(0);
	m_videouart->crl_w(1);

	m_kb_timer->adjust(attotime::zero,0,attotime::from_msec(50));
	m_crtc_irq = false;
	for(auto & elem : m_kb_keys)
		elem = 0;
}

void wicat_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case KB_TIMER:
		poll_kb();
		break;
	case KB_SERIAL_TIMER:
		m_videouart->write_rri((m_kb_data >> (m_kb_bit)) & 0x01);
		m_kb_bit++;
		if(m_kb_bit > 10)
		{
			m_kb_serial_timer->reset();
		}
		break;
	}
}

void wicat_state::poll_kb()
{
	uint8_t line;
	uint8_t val = 0;
	uint8_t x;
	uint32_t data;
	char kbtag[8];

	for(line=0;line<8;line++)
	{
		sprintf(kbtag,"kb%i",line);
		data = ioport(kbtag)->read();
		for(x=0;x<32;x++)
		{
			if((data & (1<<x)) && !(m_kb_keys[line] & (1<<x)))
			{
				send_key(val);
				m_kb_keys[line] |= (1 << x);
				return;
			}
			m_kb_keys[line] &= ~(1 << x);
			val++;
		}
	}
}

void wicat_state::send_key(uint8_t val)
{
	// based on settings in the terminal NOVRAM, the keyboard is using 1200 baud, 7 bits, 2 stop bits
	logerror("Sending key %i\n",val);
	m_kb_data = 0x0001 | (val << 2);
	m_kb_bit = 0;
	m_kb_serial_timer->adjust(attotime::zero,0,attotime::from_hz(1200));
}

WRITE_LINE_MEMBER(wicat_state::adir_w)
{
	// parallel port A direction (0 = input, 1 = output)
}

WRITE_LINE_MEMBER(wicat_state::bdir_w)
{
	// parallel port B direction (0 = input, 1 = output)
}

WRITE8_MEMBER( wicat_state::via_a_w )
{
	m_portA = data;
	logerror("VIA: write %02x to port A\n",data);
}

WRITE8_MEMBER( wicat_state::via_b_w )
{
	m_portB = data;
	logerror("VIA: write %02x to port B\n",data);
}

READ16_MEMBER( wicat_state::invalid_r )
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->set_buserror_details(0x300000+offset*2-2,true,m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}
	return 0xff;
}

WRITE16_MEMBER( wicat_state::invalid_w )
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->set_buserror_details(0x300000+offset*2-2,false,m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}
}

// TODO
READ16_MEMBER(wicat_state::memmap_r)
{
	popmessage("Memory mapping register EFFC01 read!");
	return 0xff;
}

WRITE16_MEMBER(wicat_state::memmap_w)
{
	popmessage("Memory mapping register EFFC01 written!");
}

// WD1000 Winchester Disk controller (10MB 5 1/4" HD)
// for now, we'll just try to tell the system there is no HD
READ8_MEMBER(wicat_state::hdc_r)
{
	switch(offset)
	{
	case 0x00:  // Error register
		return 0x00;
	case 0x06:  // Status register
		return 0x05;
	}
	return 0x00;
}

WRITE8_MEMBER(wicat_state::hdc_w)
{
	switch(offset)
	{
	case 0x00:  // Write precomp / Error register
		logerror("HDC: Write precomp %02x\n",data);
		break;
	case 0x01:  // Data register
		logerror("HDC: Data %02x\n",data);
		break;
	case 0x02:  // Sector Number
		logerror("HDC: Sector Number %02x\n",data);
		break;
	case 0x03:  // Sector Count
		logerror("HDC: Sector Count %02x\n",data);
		break;
	case 0x04:  // Cylinder High
		logerror("HDC: Cylinder High %02x\n",data);
		break;
	case 0x05:  // Cylinder Low
		logerror("HDC: Cylinder Low %02x\n",data);
		break;
	case 0x06:  // Command register
		logerror("HDC: Command %1x\n",(data & 0xf0) >> 4);
		m_maincpu->set_input_line(M68K_IRQ_5,HOLD_LINE);
		break;
	case 0x07:  // Size / Drive / Head
		logerror("HDC: Size / Drive / Head %02x\n",data);
		break;
	case 0x0c:  // DMA bits 9-16
		logerror("HDC: DMA address mid %02x\n",data);
		break;
	case 0x0d:  // DMA bits 1-8  (bit 0 cannot be set)
		logerror("HDC: DMA address low %02x\n",data);
		break;
	case 0x0e:  // DMA R/W
		logerror("HDC: DMA R/W %02x\n",data);
		break;
	case 0x0f:  // DMA bits 17-23
		logerror("HDC: DMA address high %02x\n",data);
		break;
	default:
		logerror("HDC: Write to unknown register %02x\n",data);
	}
}

READ8_MEMBER(wicat_state::fdc_r)
{
	uint8_t ret = 0x00;

	popmessage("FDC: read offset %02x",offset);
	switch(offset)
	{
	case 0x00:
		ret = m_fdc->status_r();
		break;
	case 0x01:
		ret = m_fdc->track_r();
		break;
	case 0x02:
		ret = m_fdc->sector_r();
		break;
	case 0x03:
		ret = m_fdc->data_r();
		break;
	case 0x08:
		// Interrupt status (TODO, not part of the FD1795)
		break;
	}
	return ret;
}

WRITE8_MEMBER(wicat_state::fdc_w)
{
	popmessage("FDC: write offset %02x data %02x",offset,data);
	switch(offset)
	{
	case 0x00:
		m_fdc->cmd_w(data);
		break;
	case 0x01:
		m_fdc->track_w(data);
		break;
	case 0x02:
		m_fdc->sector_w(data);
		break;
	case 0x03:
		m_fdc->data_w(data);
		break;
	case 0x08:
		// Interrupt disable / Drive select (TODO, not part of the FD1795)
		break;
	}
}

READ16_MEMBER(wicat_state::via_r)
{
	if(ACCESSING_BITS_0_7)
		return m_via->read(offset);
	return 0x00;
}

WRITE16_MEMBER(wicat_state::via_w)
{
	if(ACCESSING_BITS_0_7)
		m_via->write(offset,data);
	else if(ACCESSING_BITS_8_15)
		m_via->write(offset,data>>8);
}

READ8_MEMBER(wicat_state::video_r)
{
	switch(offset)
	{
	case 0x00:
		return m_crtc->read(space,0);
	case 0x02:
		return m_crtc->read(space,1);
	default:
		return 0xff;
	}
}

WRITE8_MEMBER(wicat_state::video_w)
{
	switch(offset)
	{
	case 0x00:
		m_crtc->write(space,0,data);
		break;
	case 0x02:
		m_crtc->write(space,1,data);
		break;
	}
}

READ8_MEMBER( wicat_state::vram_r )
{
	return m_videocpu->space(AS_IO).read_byte(offset*2);
}

WRITE8_MEMBER( wicat_state::vram_w )
{
	m_videocpu->space(AS_IO).write_byte(offset*2,data);
}

READ8_MEMBER(wicat_state::video_dma_r)
{
	return m_videodma->read(offset/2);
}

WRITE8_MEMBER(wicat_state::video_dma_w)
{
	if(!(offset & 0x01))
		m_videodma->write(offset/2,data);
}

READ8_MEMBER(wicat_state::video_uart0_r)
{
	uint16_t noff = offset >> 1;
	return m_videouart0->read(noff);
}

WRITE8_MEMBER(wicat_state::video_uart0_w)
{
	uint16_t noff = offset >> 1;
	m_videouart0->write(noff,data);
}

READ8_MEMBER(wicat_state::video_uart1_r)
{
	uint16_t noff = offset >> 1;
	return m_videouart1->read(noff);
}

WRITE8_MEMBER(wicat_state::video_uart1_w)
{
	uint16_t noff = offset >> 1;
	m_videouart1->write(noff,data);
}

// XD2210 64 x 4bit NOVRAM
READ8_MEMBER(wicat_state::videosram_r)
{
	if(offset & 0x01)
		return 0xff;
	else
		return m_videosram->read(space,offset/2);
}

WRITE8_MEMBER(wicat_state::videosram_w)
{
	if(!(offset & 0x01))
		m_videosram->write(space,offset/2,data);
}

WRITE8_MEMBER(wicat_state::videosram_store_w)
{
	if(data & 0x01)  // unsure of the actual bit checked, the terminal code just writes 0xff
	{
		m_videosram->store(1);
		m_videosram->store(0);
		logerror("XD2210: Store triggered.\n");
	}
}

WRITE8_MEMBER(wicat_state::videosram_recall_w)
{
	if(data & 0x01)  // unsure of the actual bit checked, the terminal code just writes 0xff
	{
		m_videosram->recall(1);
		m_videosram->recall(0);
		logerror("XD2210: Store triggered.\n");
	}
}

READ8_MEMBER(wicat_state::video_timer_r)
{
	uint8_t ret = 0x00;

	if(offset == 0x00)
		return (m_videouart->dr_r() << 4) | (m_videouart->tbre_r() && m_videoctrl->q6_r() ? 0x08 : 0x00);
	if(offset == 0x02)
	{
		if (!machine().side_effects_disabled())
		{
			m_videouart->drr_w(1);
			m_videouart->drr_w(0);
		}
		return m_videouart->read(space,0);
	}
	return ret;
}

WRITE8_MEMBER(wicat_state::video_timer_w)
{
	logerror("I/O port 0x%04x write %02x\n",offset,data);
	if(offset == 0x02)
		m_videouart->write(space,0,data);
}

READ8_MEMBER(wicat_state::video_status_r)
{
	// this port is read in the NVI IRQ routine, which if bit 2 is set, will unmask DMA channel 0.  But no idea what triggers it...
	return m_crtc_irq ? 0x04 : 0x00;
}

WRITE_LINE_MEMBER(wicat_state::dma_hrq_w)
{
	m_videocpu->set_input_line(INPUT_LINE_HALT,state ? ASSERT_LINE : CLEAR_LINE);
	m_videodma->hack_w(state);
}

WRITE_LINE_MEMBER(wicat_state::crtc_irq_w)
{
	if (state && m_videoctrl->q0_r())
	{
		m_crtc_irq = true;
		m_videoirq->in_w<1>(1);
	}
}

WRITE_LINE_MEMBER(wicat_state::crtc_irq_clear_w)
{
	if (!state)
	{
		m_crtc_irq = false;
		m_videoirq->in_w<1>(0);
	}
}

I8275_DRAW_CHARACTER_MEMBER(wicat_state::wicat_display_pixels)
{
	uint8_t romdata = vsp ? 0 : m_chargen->base()[(charcode << 4) | linecount];
	const pen_t *pen = m_palette->pens();

	for (int i = 0; i < 10; i++)
	{
		int color = ((romdata & 0xc0) != 0) ^ rvv;

		bitmap.pix32(y, x + i) = pen[color];
		romdata <<= 1;
	}
}

void wicat_state::wicat(machine_config &config)
{
	M68000(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &wicat_state::main_mem);

	VIA6522(config, m_via, 8_MHz_XTAL);
	m_via->writepa_handler().set(FUNC(wicat_state::via_a_w));
	m_via->writepb_handler().set(FUNC(wicat_state::via_b_w));
	m_via->irq_handler().set_inputline(m_maincpu, M68K_IRQ_1);

	MM58274C(config, m_rtc, 0);  // actually an MM58174AN, but should be compatible
	m_rtc->set_mode24(0); // 12 hour
	m_rtc->set_day1(1);   // monday

	// internal terminal
	MC2661(config, m_uart[0], 5.0688_MHz_XTAL);  // connected to terminal board
	m_uart[0]->txd_handler().set(m_videouart0, FUNC(mc2661_device::rx_w));
	m_uart[0]->rxrdy_handler().set_inputline(m_maincpu, M68K_IRQ_2);
	m_uart[0]->rts_handler().set(m_videouart0, FUNC(mc2661_device::cts_w));
	m_uart[0]->dtr_handler().set(m_videouart0, FUNC(mc2661_device::dsr_w));

	// RS232C ports (x5)
	const char *serial_names[5] = { "serial1", "serial2", "serial3", "serial4", "serial5" };
	for (int i = 1; i <= 5; i++)
	{
		MC2661(config, m_uart[i], 5.0688_MHz_XTAL);
		m_uart[i]->txd_handler().set(serial_names[i - 1], FUNC(rs232_port_device::write_txd));
		m_uart[i]->rts_handler().set(serial_names[i - 1], FUNC(rs232_port_device::write_rts));
		m_uart[i]->dtr_handler().set(serial_names[i - 1], FUNC(rs232_port_device::write_dtr));
		m_uart[i]->rxrdy_handler().set_inputline(m_maincpu, M68K_IRQ_2);
		m_uart[i]->txemt_dschg_handler().set_inputline(m_maincpu, M68K_IRQ_2);
	}

	// modem
	MC2661(config, m_uart[6], 5.0688_MHz_XTAL);  // connected to modem port
	m_uart[6]->rxrdy_handler().set_inputline(m_maincpu, M68K_IRQ_2);
	m_uart[6]->txemt_dschg_handler().set_inputline(m_maincpu, M68K_IRQ_2);

	const char *uart_names[5] = { "uart1", "uart2", "uart3", "uart4", "uart5" };
	for (int i = 1; i <= 5; i++)
	{
		rs232_port_device &port(RS232_PORT(config, serial_names[i - 1], default_rs232_devices, nullptr));
		port.rxd_handler().set(uart_names[i - 1], FUNC(mc2661_device::rx_w));
		port.dcd_handler().set(uart_names[i - 1], FUNC(mc2661_device::dcd_w));
		port.dsr_handler().set(uart_names[i - 1], FUNC(mc2661_device::dsr_w));
		port.cts_handler().set(uart_names[i - 1], FUNC(mc2661_device::cts_w));
	}

	ls259_device &ledlatch(LS259(config, "ledlatch")); // U19 on I/O board
	ledlatch.q_out_cb<0>().set(FUNC(wicat_state::adir_w));
	ledlatch.q_out_cb<1>().set(FUNC(wicat_state::bdir_w));
	ledlatch.q_out_cb<2>().set_output("led1").invert(); // 0 = on, 1 = off
	ledlatch.q_out_cb<3>().set_output("led2").invert();
	ledlatch.q_out_cb<4>().set_output("led3").invert();
	ledlatch.q_out_cb<5>().set_output("led4").invert();
	ledlatch.q_out_cb<6>().set_output("led5").invert();
	ledlatch.q_out_cb<7>().set_output("led6").invert();

	/* video hardware */
	Z8002(config, m_videocpu, 8_MHz_XTAL/2);  // AMD AMZ8002DC
	m_videocpu->set_addrmap(AS_PROGRAM, &wicat_state::video_mem);
	m_videocpu->set_addrmap(AS_IO, &wicat_state::video_io);

	INPUT_MERGER_ANY_HIGH(config, m_videoirq).output_handler().set_inputline(m_videocpu, INPUT_LINE_IRQ0);

	LS259(config, m_videoctrl);
	m_videoctrl->q_out_cb<0>().set(FUNC(wicat_state::crtc_irq_clear_w));
	m_videoctrl->q_out_cb<6>().set("tbreirq", FUNC(input_merger_device::in_w<1>));
	m_videoctrl->q_out_cb<7>().set("dmairq", FUNC(input_merger_device::in_w<1>));
	// Q1-Q5 are all used but unknown

	AM9517A(config, m_videodma, 8_MHz_XTAL);  // clock is a bit of guess
	m_videodma->out_hreq_callback().set(FUNC(wicat_state::dma_hrq_w));
	m_videodma->out_eop_callback().set("dmairq", FUNC(input_merger_device::in_w<0>));
	m_videodma->in_memr_callback().set(FUNC(wicat_state::vram_r));
	m_videodma->out_memw_callback().set(FUNC(wicat_state::vram_w));
	m_videodma->out_iow_callback<0>().set(m_crtc, FUNC(i8275_device::dack_w));

	INPUT_MERGER_ALL_HIGH(config, "dmairq").output_handler().set_inputline(m_videocpu, INPUT_LINE_NMI);

	IM6402(config, m_videouart, 0);
	m_videouart->set_rrc(0);
	m_videouart->set_trc(1200);
	m_videouart->dr_callback().set(m_videoirq, FUNC(input_merger_device::in_w<2>));
	m_videouart->tbre_callback().set("tbreirq", FUNC(input_merger_device::in_w<0>));

	INPUT_MERGER_ALL_HIGH(config, "tbreirq").output_handler().set(m_videoirq, FUNC(input_merger_device::in_w<3>));

	// terminal (2x INS2651, 1x IM6042 - one of these is for the keyboard, another communicates with the main board, the third is unknown)
	MC2661(config, m_videouart0, 5.0688_MHz_XTAL);  // the INS2651 looks similar enough to the MC2661...
	m_videouart0->txd_handler().set(m_uart[0], FUNC(mc2661_device::rx_w));
	m_videouart0->rxrdy_handler().set(m_videoirq, FUNC(input_merger_device::in_w<0>));
	m_videouart0->rts_handler().set(m_uart[0], FUNC(mc2661_device::cts_w));
	m_videouart0->dtr_handler().set(m_uart[0], FUNC(mc2661_device::dsr_w));

	MC2661(config, m_videouart1, 5.0688_MHz_XTAL);
	m_videouart1->set_rxc(19200);
	m_videouart1->set_txc(19200);
	m_videouart1->rxrdy_handler().set(m_videoirq, FUNC(input_merger_device::in_w<4>));

	X2210(config, "vsram");  // XD2210

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw(19.6608_MHz_XTAL, 1020, 0, 800, 324, 0, 300);
	screen.set_screen_update("video", FUNC(i8275_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	I8275(config, m_crtc, 19.6608_MHz_XTAL/10);
	m_crtc->set_character_width(10);
	m_crtc->set_display_callback(FUNC(wicat_state::wicat_display_pixels), this);
	m_crtc->drq_wr_callback().set(m_videodma, FUNC(am9517a_device::dreq0_w));
	m_crtc->vrtc_wr_callback().set(FUNC(wicat_state::crtc_irq_w));
	m_crtc->set_screen("screen");

	config.set_default_layout(layout_wicat);

	/* Winchester Disk Controller (WD1000 + FD1795) */
	n8x300_cpu_device &wd1kcpu(N8X300(config, "wd1kcpu", 8_MHz_XTAL));
	wd1kcpu.set_addrmap(AS_PROGRAM, &wicat_state::wd1000_mem);
	wd1kcpu.set_addrmap(AS_IO, &wicat_state::wd1000_io);

	FD1795(config, m_fdc, 8_MHz_XTAL);
	FLOPPY_CONNECTOR(config, "fdc:0", wicat_floppies, "525qd", floppy_image_device::default_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", wicat_floppies, nullptr, floppy_image_device::default_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", wicat_floppies, nullptr, floppy_image_device::default_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", wicat_floppies, nullptr, floppy_image_device::default_floppy_formats).enable_sound(true);

	SOFTWARE_LIST(config, "flop_list").set_type("wicat", SOFTWARE_LIST_ORIGINAL_SYSTEM);
}

/* ROM definition */
ROM_START( wicat )
	ROM_REGION16_BE(0x4000, "c1", 0)
	ROM_LOAD16_BYTE("wiboot.e",   0x00000, 0x0800, CRC(6f0f73c6) SHA1(be635bf3ffa1301f844a3d5560e278de46740d19) )
	ROM_LOAD16_BYTE("wiboot.o",   0x00001, 0x0800, CRC(b9763bbd) SHA1(68f497be56ff69534e17b41a40737cd6f708d65e) )
	ROM_LOAD16_BYTE("tpcnif.e",   0x01000, 0x0800, CRC(fd1127ec) SHA1(7c6b436c0cea41dbb23cb6bd9b9a5c21fa61d232) )
	ROM_LOAD16_BYTE("tpcnif.o",   0x01001, 0x0800, CRC(caa16e2a) SHA1(b3e64b676f50b65b3e365fc5f17eb1759c1310df) )
	ROM_LOAD16_BYTE("tpcf.e",     0x02000, 0x0800, CRC(d34be25c) SHA1(1b167918cbc19c9364f020176f4cc3722cba8434) )
	ROM_LOAD16_BYTE("tpcf.o",     0x02001, 0x0800, CRC(7712c570) SHA1(8743b7c98190ecf3bf7e917e6143b47b3b36db8d) )
	ROM_REGION(0x0060, "c1proms", 0)
	ROM_LOAD       ("cpu.8b",     0x00000, 0x0020, CRC(99b90665) SHA1(8a4677ea814e1843001fe28b284226b7291cdf76) )
	ROM_LOAD       ("cpu.8c",     0x00020, 0x0020, CRC(190a55ad) SHA1(de8a847bff8c343d69b853a215e6ee775ef2ef96) )
	ROM_LOAD       ("cpu.15c",    0x00040, 0x0020, CRC(ba2dd77d) SHA1(eb693d6d30aa6a9dba61c6c41a75614ed4e9e69a) )

	// System 150 CPU/MU board
	ROM_REGION16_BE(0x2000, "c2", 0)
	ROM_SYSTEM_BIOS( 0, "cms", "CMS HD / Floppy Boot / Boot v1.56" )
	ROMX_LOAD("s156.a5", 0x00000, 0x0800, CRC(2c1e9542) SHA1(50184e04f0c881818e96e2162111d16304e8762f), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD("s156.a7", 0x00001, 0x0800, CRC(5a0cb30d) SHA1(aa106ad5a8b9e89613f7ea026d62832cfdb19fd0), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD("37m.b5",  0x01000, 0x0800, CRC(831571fb) SHA1(fcc647b3ef9f0cca3e8212f850f96676d24cf318), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD("37m.b7",  0x01001, 0x0800, CRC(3c346e8e) SHA1(d8ff8297d265b25655c854ed8515fa9e16c63f39), ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "wd3", "WD3 HD Boot / Boot v1.56" )
	ROMX_LOAD("boot156.a5", 0x00000, 0x0800, CRC(58510a52) SHA1(d2135b056a04ba830b0ae1cef539e4a9a1b58f82), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD("boot156.a7", 0x00001, 0x0800, CRC(e53999f1) SHA1(9c6c6a3a56b5c16a35e1fe824f37c8ae739ebcb9), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD("wd3_15.b5",  0x01000, 0x0800, CRC(a765899b) SHA1(8427c564029914b7dbc29768ce451604180e390f), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD("wd3_15.b7",  0x01001, 0x0800, CRC(9d986585) SHA1(1ac7579c692f827b121c56dac0a77b15400caba1), ROM_SKIP(1) | ROM_BIOS(1) )

	// Terminal CPU board (Graphical)
	// "MG8000 VERSION 3.0"
	ROM_REGION16_BE(0x8000, "g1", 0)
	ROM_LOAD16_BYTE("1term0.e",   0x00000, 0x0800, CRC(a9aade37) SHA1(644e9362d5a9523be5c6f39a650b574735dbd4a2) )
	ROM_LOAD16_BYTE("1term0.o",   0x00001, 0x0800, CRC(8026b5b7) SHA1(cb93e0595b321889694cbb87f497d244e6a2d648) )
	ROM_LOAD16_BYTE("1term1.e",   0x01000, 0x0800, CRC(e6ce8016) SHA1(fae987f1ac26d027ed176f8886832e87d1feae60) )
	ROM_LOAD16_BYTE("1term1.o",   0x01001, 0x0800, CRC(d71f763e) SHA1(b0a7f4cc90ce267aec7e72ad22a227f0c8c1f650) )
	ROM_LOAD16_BYTE("1term2.e",   0x02000, 0x0800, CRC(c0e82703) SHA1(7a17da13c01e15b61eea65b06d988ab8ba7eaaf3) )
	ROM_LOAD16_BYTE("1term2.o",   0x02001, 0x0800, CRC(aa0d5b4f) SHA1(b37c2e5220f4838a805b20a0ef21689067f1a759) )
	ROM_LOAD16_BYTE("1term3.e",   0x03000, 0x0800, CRC(cd33f4c8) SHA1(6603c5f2330a9a5ec1121a367cebe6e900a00cb0) )
	ROM_LOAD16_BYTE("1term3.o",   0x03001, 0x0800, CRC(05e56714) SHA1(0c31be3c9ec90a0858fe04a208e2627e4beb12b0) )
	ROM_LOAD16_BYTE("1term4.e",   0x04000, 0x0800, CRC(a157c61f) SHA1(59b7be6cd696b2508b5c1fd7b6e6f7cb5a9f12ab) )
	ROM_LOAD16_BYTE("1term4.o",   0x04001, 0x0800, CRC(364c1a95) SHA1(bfd62a71c9d8f83dc12a7dbbf362d18819380ef3) )
	ROM_LOAD16_BYTE("1term5.e",   0x05000, 0x0800, CRC(c2b8bc9e) SHA1(cd054988a9694b3a211e1993da1b3dc2c5e6fdc2) )
	ROM_LOAD16_BYTE("1term5.o",   0x05001, 0x0800, CRC(421e0521) SHA1(29b87938f5c25c05920ca2c14893700bc45a86c5) )
	ROM_LOAD16_BYTE("1term6.e",   0x06000, 0x0800, CRC(f0d14ed6) SHA1(840acc2b90e8d16df7e5d60c399b08ec0e126a88) )
	ROM_LOAD16_BYTE("1term6.o",   0x06001, 0x0800, CRC(e245ff49) SHA1(9a34e6cf6013b1044cccf26371cc3a000f17b58c) )
	ROM_LOAD16_BYTE("1term7.e",   0x07000, 0x0800, CRC(0c918550) SHA1(2ef6ce41cc2643d45c4bae31ce151d8b6c363471) )
	ROM_LOAD16_BYTE("1term7.o",   0x07001, 0x0800, CRC(71fdc692) SHA1(d6f12ec20ff2e4948f54b0c79f11ccbdc9db865c) )
	ROM_REGION(0x40, "vsram", 0)
	ROM_LOAD       ("ee8-82.bin",  0x00000, 0x0040, CRC(dfb4b0fb) SHA1(12304f5c5236791f5e931d9e49b4a70dcbba55c0) )

	// "MG8000 VERSION 1.1"
	ROM_REGION16_BE(0x8000, "g2", 0)
	ROM_LOAD16_BYTE("2term0.e",   0x00000, 0x0800, CRC(29e5dd68) SHA1(9023f53d554b9ef4f4efc731645ba42f728bcd2c) )
	ROM_LOAD16_BYTE("2term0.o",   0x00001, 0x0800, CRC(91edd05d) SHA1(378b06fc8316199b7c580a6e7f28368dacdac5a9) )
	ROM_LOAD16_BYTE("2term1.e",   0x01000, 0x0800, CRC(2b48abe4) SHA1(4c9b4db1c1408b6551d50172dda994b36a2ee4b1) )
	ROM_LOAD16_BYTE("2term1.o",   0x01001, 0x0800, CRC(4c0e4f95) SHA1(bd49bf71fea1acfd50781820f0a650411b6f996b) )
	ROM_LOAD16_BYTE("2term2.e",   0x02000, 0x0800, CRC(3251324b) SHA1(e8f52308c9cbb9bcb5adb2685609d6a69b9eec1d) )
	ROM_LOAD16_BYTE("2term2.o",   0x02001, 0x0800, CRC(3a49c9e7) SHA1(0718b029ed316bc8e7bf22b0e94b6b5628758580) )
	ROM_LOAD16_BYTE("2term3.e",   0x03000, 0x0800, CRC(0f17be85) SHA1(9c40b4d06f3fb8def88b87615a590bb03dcfc4f4) )
	ROM_LOAD16_BYTE("2term3.o",   0x03001, 0x0800, CRC(08ae31c5) SHA1(2e53f87b6a4e0b973f7918d97f57f6560c651ab6) )
	ROM_LOAD16_BYTE("2term4.e",   0x04000, 0x0800, CRC(413936e7) SHA1(ce9d8666ca4e6847514bcf4de5703f0845e72928) )
	ROM_LOAD16_BYTE("2term4.o",   0x04001, 0x0800, CRC(06deab4e) SHA1(af5be7105a24d81dcc539296631b4309f7b8cb3f) )
	ROM_LOAD16_BYTE("2term5.e",   0x05000, 0x0800, CRC(7979bf59) SHA1(1bc397c58ce026fb90a02714d42df8f179a4f50e) )
	ROM_LOAD16_BYTE("2term5.o",   0x05001, 0x0800, CRC(e1f738ca) SHA1(bd8d7f1acb243880fd364f71097b9711de496739) )
	ROM_LOAD16_BYTE("2term6.e",   0x06000, 0x0800, CRC(bb04d70c) SHA1(0b482c2f06fe5e042a5813f027f5cf034d72e0dd) )
	ROM_LOAD16_BYTE("2term6.o",   0x06001, 0x0800, CRC(0afb566c) SHA1(761455ced46b6fccd0be9c8fa920f7954a36972b) )
	ROM_LOAD16_BYTE("2term7.e",   0x07000, 0x0800, CRC(033ea830) SHA1(27c33eea2df812a1a96e2f47ba7993e2ca3675ad) )
	ROM_LOAD16_BYTE("2term7.o",   0x07001, 0x0800, CRC(e157c5d2) SHA1(3cd1ea0fb9df1358e8a358468a4df5e4eaaa86a2) )
	ROM_REGION(0x40, "g2novram", 0)
	ROM_LOAD       ("ee2-2.bin",  0x00000, 0x0040, CRC(8f265118) SHA1(6bd74e3d01cf85cca1abcc15cb229dbd63022978) )

	// Terminal Video board
	ROM_REGION(0x1000, "g2char", 0)
	ROM_LOAD       ("ascii.chr",  0x00000, 0x0800, CRC(43e26e37) SHA1(f3d5d16040c66f0e827f72a35d4694ca62950949) )
	ROM_LOAD       ("apl.chr",    0x00800, 0x0800, CRC(8c6d698e) SHA1(147dd9296fe2efc6140fa148a6edf673c33f9371) )

	// Winchester Disk Controller  (WD1000 (comprised of an 8X300 + some WD1100-xx bits), FD1795 (FDC))
	ROM_REGION16_BE(0x1800, "wd3", 0)
	ROM_LOAD16_BYTE("wd3.u96",    0x00000, 0x0800, CRC(52736e61) SHA1(71c7c9170c733c483393969cb1cb3798b3eb980c) )  // 8X300 code even
	ROM_LOAD16_BYTE("wd3.u97",    0x00001, 0x0800, CRC(a66619ec) SHA1(5d091ac7c88f2f45b4a05e78bfc7a16c206b31ff) )  // 8X300 code odd
	ROM_LOAD       ("wd3.u95",    0x01000, 0x0800, CRC(80bb0617) SHA1(ac0f3194fcbef77532571baa3fec78b3010528bf) )  // "Fast IO select" bytes
ROM_END


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY               FULLNAME            FLAGS
COMP( 1982, wicat, 0,      0,      wicat,   wicat, wicat_state, empty_init, "Millennium Systems", "Wicat System 150", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )

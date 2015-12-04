// license:BSD-3-Clause
// copyright-holders:Robbbert, Barry Rodewald
/***************************************************************************

Wicat - various systems.

2013-09-01 Skeleton driver

****************************************************************************/

/*

    TODO:

    - video DMA is done line by line and needs to be in perfect sync

*/

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z8000/z8000.h"
#include "cpu/8x300/8x300.h"
#include "machine/6522via.h"
#include "machine/mm58274c.h"
#include "machine/mc2661.h"
#include "machine/im6402.h"
#include "video/i8275.h"
#include "machine/am9517a.h"
#include "machine/x2212.h"
#include "machine/wd_fdc.h"
#include "wicat.lh"

class wicat_state : public driver_device
{
public:
	wicat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vram(*this, "vram"),
		m_maincpu(*this, "maincpu"),
		m_rtc(*this, "rtc"),
		m_via(*this, "via"),
		m_uart0(*this,"uart0"),
		m_uart1(*this,"uart1"),
		m_uart2(*this,"uart2"),
		m_uart3(*this,"uart3"),
		m_uart4(*this,"uart4"),
		m_uart5(*this,"uart5"),
		m_uart6(*this,"uart6"),
		m_videocpu(*this,"videocpu"),
		m_videoctrl(*this,"video"),
		m_videodma(*this,"videodma"),
		m_videouart0(*this,"videouart0"),
		m_videouart1(*this,"videouart1"),
		m_videouart(*this,"videouart"),
		m_videosram(*this,"vsram"),
		m_palette(*this, "palette"),
		m_chargen(*this,"g2char"),
		m_fdc(*this,"fdc")
	{
	}

	DECLARE_READ16_MEMBER(invalid_r);
	DECLARE_WRITE16_MEMBER(invalid_w);
	DECLARE_READ16_MEMBER(memmap_r);
	DECLARE_WRITE16_MEMBER(memmap_w);
	DECLARE_WRITE16_MEMBER(parallel_led_w);
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
	DECLARE_READ8_MEMBER(video_ctrl_r);
	DECLARE_WRITE8_MEMBER(video_ctrl_w);
	DECLARE_READ8_MEMBER(video_status_r);
	DECLARE_WRITE_LINE_MEMBER(dma_hrq_w);
	DECLARE_WRITE_LINE_MEMBER(dma_nmi_cb);
	DECLARE_WRITE_LINE_MEMBER(crtc_cb);
	DECLARE_READ8_MEMBER(hdc_r);
	DECLARE_WRITE8_MEMBER(hdc_w);
	DECLARE_READ8_MEMBER(fdc_r);
	DECLARE_WRITE8_MEMBER(fdc_w);
	DECLARE_READ16_MEMBER(via_r);
	DECLARE_WRITE16_MEMBER(via_w);
	DECLARE_WRITE_LINE_MEMBER(kb_data_ready);
	I8275_DRAW_CHARACTER_MEMBER(wicat_display_pixels);

	required_shared_ptr<UINT8> m_vram;
	required_device<m68000_device> m_maincpu;
	required_device<mm58274c_device> m_rtc;
	required_device<via6522_device> m_via;
	required_device<mc2661_device> m_uart0;
	required_device<mc2661_device> m_uart1;
	required_device<mc2661_device> m_uart2;
	required_device<mc2661_device> m_uart3;
	required_device<mc2661_device> m_uart4;
	required_device<mc2661_device> m_uart5;
	required_device<mc2661_device> m_uart6;
	required_device<cpu_device> m_videocpu;
	required_device<i8275_device> m_videoctrl;
	required_device<am9517a_device> m_videodma;
	required_device<mc2661_device> m_videouart0;
	required_device<mc2661_device> m_videouart1;
	required_device<im6402_device> m_videouart;
	required_device<x2210_device> m_videosram;
	required_device<palette_device> m_palette;
	required_memory_region m_chargen;
	required_device<fd1795_t> m_fdc;

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) { return 0; }
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);


private:
	virtual void machine_start();
	virtual void machine_reset();
	virtual void driver_start();

	void poll_kb();
	void send_key(UINT8 val);

	emu_timer* m_video_timer;
	emu_timer* m_kb_timer;
	emu_timer* m_kb_serial_timer;
	static const device_timer_id VIDEO_TIMER = 0;
	static const device_timer_id KB_TIMER = 1;
	static const device_timer_id KB_SERIAL_TIMER = 2;

	UINT8 m_portA;
	UINT8 m_portB;
	bool m_video_timer_irq;
	bool m_video_kb_irq;
	UINT8 m_nmi_enable;
	UINT8 m_crtc_irq;
	UINT16 m_kb_data;
	UINT8 m_kb_bit;
	UINT32 m_kb_keys[8];
};


static ADDRESS_MAP_START(wicat_mem, AS_PROGRAM, 16, wicat_state)
	ADDRESS_MAP_UNMAP_LOW
	ADDRESS_MAP_GLOBAL_MASK(0xffffff)
	AM_RANGE(0x000000, 0x001fff) AM_ROM AM_REGION("c2", 0x0000)
	AM_RANGE(0x020000, 0x1fffff) AM_RAM
	AM_RANGE(0x200000, 0x2fffff) AM_RAM
	AM_RANGE(0x300000, 0xdfffff) AM_READWRITE(invalid_r,invalid_w)
	AM_RANGE(0xeff800, 0xeffbff) AM_RAM  // memory mapping SRAM, used during boot sequence for storing various data (TODO)
	AM_RANGE(0xeffc00, 0xeffc01) AM_READWRITE(memmap_r,memmap_w)
	AM_RANGE(0xf00000, 0xf00007) AM_DEVREADWRITE8("uart0",mc2661_device,read,write,0xff00)  // UARTs
	AM_RANGE(0xf00008, 0xf0000f) AM_DEVREADWRITE8("uart1",mc2661_device,read,write,0xff00)
	AM_RANGE(0xf00010, 0xf00017) AM_DEVREADWRITE8("uart2",mc2661_device,read,write,0xff00)
	AM_RANGE(0xf00018, 0xf0001f) AM_DEVREADWRITE8("uart3",mc2661_device,read,write,0xff00)
	AM_RANGE(0xf00020, 0xf00027) AM_DEVREADWRITE8("uart4",mc2661_device,read,write,0xff00)
	AM_RANGE(0xf00028, 0xf0002f) AM_DEVREADWRITE8("uart5",mc2661_device,read,write,0xff00)
	AM_RANGE(0xf00030, 0xf00037) AM_DEVREADWRITE8("uart6",mc2661_device,read,write,0xff00)
	AM_RANGE(0xf00040, 0xf0005f) AM_READWRITE(via_r, via_w)
	AM_RANGE(0xf00060, 0xf0007f) AM_DEVREADWRITE8("rtc",mm58274c_device,read,write,0xff00)
	AM_RANGE(0xf000d0, 0xf000d1) AM_WRITE(parallel_led_w)
	AM_RANGE(0xf00180, 0xf0018f) AM_READWRITE8(hdc_r,hdc_w,0xffff)  // WD1000
	AM_RANGE(0xf00190, 0xf0019f) AM_READWRITE8(fdc_r,fdc_w,0xffff)  // FD1795
	AM_RANGE(0xf00f00, 0xf00fff) AM_READWRITE(invalid_r,invalid_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(wicat_video_mem, AS_PROGRAM, 16, wicat_state)
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("g1", 0x0000)
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(wicat_video_io, AS_IO, 8, wicat_state)
	// these are largely wild guesses...
	AM_RANGE(0x0000,0x0003) AM_READWRITE(video_timer_r,video_timer_w)  // some sort of timer?
	AM_RANGE(0x0100,0x0107) AM_READWRITE(video_uart0_r,video_uart0_w)  // INS2651 UART #1
	AM_RANGE(0x0200,0x0207) AM_READWRITE(video_uart1_r,video_uart1_w)  // INS2651 UART #2
	AM_RANGE(0x0304,0x0304) AM_READ(video_status_r)
	AM_RANGE(0x0400,0x047f) AM_READWRITE(videosram_r,videosram_w)  // XD2210  4-bit NOVRAM
	AM_RANGE(0x0500,0x0500) AM_WRITE(videosram_recall_w)
	AM_RANGE(0x0600,0x0600) AM_WRITE(videosram_store_w)
	AM_RANGE(0x0800,0x080f) AM_READWRITE(video_ctrl_r,video_ctrl_w)
	AM_RANGE(0x0a00,0x0a1f) AM_READWRITE(video_dma_r,video_dma_w) // AM9517A DMA
	AM_RANGE(0x0b00,0x0b03) AM_READWRITE(video_r,video_w)  // i8275 CRTC
	AM_RANGE(0x0e00,0x0eff) AM_RAM
	AM_RANGE(0x4000,0x5fff) AM_RAM AM_SHARE("vram") // video RAM?
	AM_RANGE(0x8000,0x8fff) AM_ROM AM_REGION("g2char",0x0000)
	AM_RANGE(0x9000,0x9fff) AM_ROM AM_REGION("g2char",0x0000)
ADDRESS_MAP_END

static ADDRESS_MAP_START(wicat_wd1000_mem, AS_PROGRAM, 16, wicat_state)
	AM_RANGE(0x0000, 0x17ff) AM_ROM AM_REGION("wd3", 0x0000)
	AM_RANGE(0x1800, 0x1fff) AM_NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START(wicat_wd1000_io, AS_IO, 8, wicat_state)
	AM_RANGE(0x0000, 0x00ff) AM_RAM  // left bank  - RAM
	AM_RANGE(0x0100, 0x01ff) AM_RAM  // right bank - I/O ports (TODO)
ADDRESS_MAP_END


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

void wicat_state::driver_start()
{
	m_video_timer = timer_alloc(VIDEO_TIMER);
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
	m_uart0->dcd_w(0);
	m_uart1->dcd_w(0);
	m_uart2->dcd_w(0);
	m_uart3->dcd_w(0);
	m_uart4->dcd_w(0);
	m_uart5->dcd_w(0);

	// initialise im6402 (terminal board)
	m_videouart->cls1_w(1);
	m_videouart->cls2_w(1);
	m_videouart->pi_w(1);
	m_videouart->sbs_w(0);
	m_videouart->crl_w(1);

	m_video_timer_irq = false;
	m_video_kb_irq = false;
	m_video_timer->adjust(attotime::zero,0,attotime::from_hz(60));
	m_kb_timer->adjust(attotime::zero,0,attotime::from_msec(50));
	m_nmi_enable = 0;
	m_crtc_irq = CLEAR_LINE;
	for(auto & elem : m_kb_keys)
		elem = 0;
}

void wicat_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case VIDEO_TIMER:
		m_video_timer_irq = true;
		m_videocpu->set_input_line(INPUT_LINE_IRQ0,ASSERT_LINE);
		break;
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
	UINT8 line;
	UINT8 val = 0;
	UINT8 x;
	UINT32 data;
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

void wicat_state::send_key(UINT8 val)
{
	// based on settings in the terminal NOVRAM, the keyboard is using 1200 baud, 7 bits, 2 stop bits
	logerror("Sending key %i\n",val);
	m_kb_data = 0x0001 | (val << 2);
	m_kb_bit = 0;
	m_kb_serial_timer->adjust(attotime::zero,0,attotime::from_hz(1200));
}

WRITE16_MEMBER( wicat_state::parallel_led_w )
{
	// bit 0 - parallel port A direction (0 = input)
	// bit 1 - parallel port B direction (0 = input)
	output_set_value("led1",data & 0x0400);
	output_set_value("led2",data & 0x0800);
	output_set_value("led3",data & 0x1000);
	output_set_value("led4",data & 0x2000);
	output_set_value("led5",data & 0x4000);
	output_set_value("led6",data & 0x8000);
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
	if(!space.debugger_access())
	{
		m_maincpu->set_buserror_details(0x300000+offset*2-2,0,m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}
	return 0xff;
}

WRITE16_MEMBER( wicat_state::invalid_w )
{
	if(!space.debugger_access())
	{
		m_maincpu->set_buserror_details(0x300000+offset*2-2,1,m_maincpu->get_fc());
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
	UINT8 ret = 0x00;

	popmessage("FDC: read offset %02x",offset);
	switch(offset)
	{
	case 0x00:
		ret = m_fdc->status_r(space,0);
		break;
	case 0x01:
		ret = m_fdc->track_r(space,0);
		break;
	case 0x02:
		ret = m_fdc->sector_r(space,0);
		break;
	case 0x03:
		ret = m_fdc->data_r(space,0);
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
		m_fdc->cmd_w(space,0,data);
		break;
	case 0x01:
		m_fdc->track_w(space,0,data);
		break;
	case 0x02:
		m_fdc->sector_w(space,0,data);
		break;
	case 0x03:
		m_fdc->data_w(space,0,data);
		break;
	case 0x08:
		// Interrupt disable / Drive select (TODO, not part of the FD1795)
		break;
	}
}

READ16_MEMBER(wicat_state::via_r)
{
	if(ACCESSING_BITS_0_7)
		return m_via->read(space,offset);
	return 0x00;
}

WRITE16_MEMBER(wicat_state::via_w)
{
	if(ACCESSING_BITS_0_7)
		m_via->write(space,offset,data);
	else if(ACCESSING_BITS_8_15)
		m_via->write(space,offset,data>>8);
}

READ8_MEMBER(wicat_state::video_r)
{
	switch(offset)
	{
	case 0x00:
		return m_videoctrl->read(space,0);
	case 0x02:
		return m_videoctrl->read(space,1);
	default:
		return 0xff;
	}
}

WRITE8_MEMBER(wicat_state::video_w)
{
	switch(offset)
	{
	case 0x00:
		m_videoctrl->write(space,0,data);
		break;
	case 0x02:
		m_videoctrl->write(space,1,data);
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
	return m_videodma->read(space,offset/2);
}

WRITE8_MEMBER(wicat_state::video_dma_w)
{
	if(!(offset & 0x01))
		m_videodma->write(space,offset/2,data);
}

READ8_MEMBER(wicat_state::video_uart0_r)
{
	UINT16 noff = offset >> 1;
	return m_videouart0->read(space,noff);
}

WRITE8_MEMBER(wicat_state::video_uart0_w)
{
	UINT16 noff = offset >> 1;
	m_videouart0->write(space,noff,data);
}

READ8_MEMBER(wicat_state::video_uart1_r)
{
	UINT16 noff = offset >> 1;
	return m_videouart1->read(space,noff);
}

WRITE8_MEMBER(wicat_state::video_uart1_w)
{
	UINT16 noff = offset >> 1;
	m_videouart1->write(space,noff,data);
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
	UINT8 ret = 0x00;

	if(offset == 0x00)
	{
		if(m_video_timer_irq)
		{
			ret |= 0x08;
			m_video_timer_irq = false;
			m_videocpu->set_input_line(INPUT_LINE_IRQ0,CLEAR_LINE);
		}
		if(m_video_kb_irq)
		{
			ret |= 0x10;
			m_video_kb_irq = false;
			m_videocpu->set_input_line(INPUT_LINE_IRQ0,CLEAR_LINE);
		}
	}
	if(offset == 0x02)
		return m_videouart->read(space,0);
	return ret;
}

WRITE8_MEMBER(wicat_state::video_timer_w)
{
	logerror("I/O port 0x%04x write %02x\n",offset,data);
	if(offset == 0x02)
		m_videouart->write(space,0,data);
}

READ8_MEMBER(wicat_state::video_ctrl_r)
{
	return 0x00;  // TODO
}

WRITE8_MEMBER(wicat_state::video_ctrl_w)
{
	if(offset == 0x07)
		m_nmi_enable = data;
}

READ8_MEMBER(wicat_state::video_status_r)
{
	// this port is read in the NVI IRQ routine, which if bit 2 is set, will unmask DMA channel 0.  But no idea what triggers it...
	if(m_crtc_irq == ASSERT_LINE)
	{
		m_crtc_irq = CLEAR_LINE;
		m_videocpu->set_input_line(INPUT_LINE_IRQ0,CLEAR_LINE);
		return 0x04;
	}
	else
		return 0x00;
}

WRITE_LINE_MEMBER(wicat_state::dma_hrq_w)
{
	m_videocpu->set_input_line(INPUT_LINE_HALT,state ? ASSERT_LINE : CLEAR_LINE);
	m_videodma->hack_w(state);
}

WRITE_LINE_MEMBER(wicat_state::dma_nmi_cb)
{
	if(state)
	{
		if(m_nmi_enable != 0)
			m_videocpu->set_input_line(INPUT_LINE_NMI,PULSE_LINE);
	}
}

WRITE_LINE_MEMBER(wicat_state::kb_data_ready)
{
	m_video_kb_irq = state ? ASSERT_LINE : CLEAR_LINE;
	m_videocpu->set_input_line(INPUT_LINE_IRQ0,m_video_kb_irq);
}

WRITE_LINE_MEMBER(wicat_state::crtc_cb)
{
	m_crtc_irq = state ? ASSERT_LINE : CLEAR_LINE;
	m_videocpu->set_input_line(INPUT_LINE_IRQ0,m_crtc_irq);
}

I8275_DRAW_CHARACTER_MEMBER(wicat_state::wicat_display_pixels)
{
	UINT8 romdata = m_chargen->base()[((charcode << 4) | linecount) + 1];
	const pen_t *pen = m_palette->pens();

	for (int i = 0; i < 8; i++)
	{
		int color = (romdata >> (7-i)) & 0x01;

		if(vsp || linecount > 9)
			color = 0;

		bitmap.pix32(y, x + i) = pen[color];
	}
}

static MACHINE_CONFIG_START( wicat, wicat_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(wicat_mem)

	MCFG_DEVICE_ADD("via", VIA6522, XTAL_8MHz)
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(wicat_state, via_a_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(wicat_state, via_b_w))
	MCFG_VIA6522_IRQ_HANDLER(INPUTLINE("maincpu", M68K_IRQ_1))

	MCFG_DEVICE_ADD("rtc", MM58274C, 0)  // actually an MM58174AN, but should be compatible
	MCFG_MM58274C_MODE24(0) // 12 hour
	MCFG_MM58274C_DAY1(1)   // monday

	// internal terminal
	MCFG_DEVICE_ADD("uart0", MC2661, XTAL_5_0688MHz)  // connected to terminal board
	MCFG_MC2661_TXD_HANDLER(DEVWRITELINE("videouart0", mc2661_device, rx_w))
	MCFG_MC2661_RXRDY_HANDLER(INPUTLINE("maincpu", M68K_IRQ_2))
	MCFG_MC2661_RTS_HANDLER(DEVWRITELINE("videouart0", mc2661_device, cts_w))
	MCFG_MC2661_DTR_HANDLER(DEVWRITELINE("videouart0", mc2661_device, dsr_w))

	// RS232C ports (x5)
	MCFG_DEVICE_ADD("uart1", MC2661, XTAL_5_0688MHz)
	MCFG_MC2661_TXD_HANDLER(DEVWRITELINE("serial1", rs232_port_device, write_txd))
	MCFG_MC2661_RXRDY_HANDLER(INPUTLINE("maincpu", M68K_IRQ_2))
	MCFG_MC2661_RTS_HANDLER(DEVWRITELINE("serial1", rs232_port_device, write_rts))
	MCFG_MC2661_DTR_HANDLER(DEVWRITELINE("serial1", rs232_port_device, write_dtr))
	MCFG_MC2661_TXEMT_DSCHG_HANDLER(INPUTLINE("maincpu", M68K_IRQ_2))

	MCFG_DEVICE_ADD("uart2", MC2661, XTAL_5_0688MHz)
	MCFG_MC2661_TXD_HANDLER(DEVWRITELINE("serial2", rs232_port_device, write_txd))
	MCFG_MC2661_RXRDY_HANDLER(INPUTLINE("maincpu", M68K_IRQ_2))
	MCFG_MC2661_RTS_HANDLER(DEVWRITELINE("serial2", rs232_port_device, write_rts))
	MCFG_MC2661_DTR_HANDLER(DEVWRITELINE("serial2", rs232_port_device, write_dtr))

	MCFG_DEVICE_ADD("uart3", MC2661, XTAL_5_0688MHz)
	MCFG_MC2661_TXD_HANDLER(DEVWRITELINE("serial3", rs232_port_device, write_txd))
	MCFG_MC2661_RXRDY_HANDLER(INPUTLINE("maincpu", M68K_IRQ_2))
	MCFG_MC2661_RTS_HANDLER(DEVWRITELINE("serial3", rs232_port_device, write_rts))
	MCFG_MC2661_DTR_HANDLER(DEVWRITELINE("serial3", rs232_port_device, write_dtr))
	MCFG_MC2661_TXEMT_DSCHG_HANDLER(INPUTLINE("maincpu", M68K_IRQ_2))

	MCFG_DEVICE_ADD("uart4", MC2661, XTAL_5_0688MHz)
	MCFG_MC2661_TXD_HANDLER(DEVWRITELINE("serial4", rs232_port_device, write_txd))
	MCFG_MC2661_RXRDY_HANDLER(INPUTLINE("maincpu", M68K_IRQ_2))
	MCFG_MC2661_RTS_HANDLER(DEVWRITELINE("serial4", rs232_port_device, write_rts))
	MCFG_MC2661_DTR_HANDLER(DEVWRITELINE("serial4", rs232_port_device, write_dtr))
	MCFG_MC2661_TXEMT_DSCHG_HANDLER(INPUTLINE("maincpu", M68K_IRQ_2))

	MCFG_DEVICE_ADD("uart5", MC2661, XTAL_5_0688MHz)
	MCFG_MC2661_TXD_HANDLER(DEVWRITELINE("serial5", rs232_port_device, write_txd))
	MCFG_MC2661_RXRDY_HANDLER(INPUTLINE("maincpu", M68K_IRQ_2))
	MCFG_MC2661_RTS_HANDLER(DEVWRITELINE("serial5", rs232_port_device, write_rts))
	MCFG_MC2661_DTR_HANDLER(DEVWRITELINE("serial5", rs232_port_device, write_dtr))
	MCFG_MC2661_TXEMT_DSCHG_HANDLER(INPUTLINE("maincpu", M68K_IRQ_2))

	// modem
	MCFG_DEVICE_ADD("uart6", MC2661, XTAL_5_0688MHz)  // connected to modem port
	MCFG_MC2661_RXRDY_HANDLER(INPUTLINE("maincpu", M68K_IRQ_2))
	MCFG_MC2661_TXEMT_DSCHG_HANDLER(INPUTLINE("maincpu", M68K_IRQ_2))

	MCFG_RS232_PORT_ADD("serial1",default_rs232_devices,nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart1",mc2661_device,rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("uart1",mc2661_device,dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart1",mc2661_device,dsr_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart1",mc2661_device,cts_w))

	MCFG_RS232_PORT_ADD("serial2",default_rs232_devices,nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart2",mc2661_device,rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("uart2",mc2661_device,dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart2",mc2661_device,dsr_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart2",mc2661_device,cts_w))

	MCFG_RS232_PORT_ADD("serial3",default_rs232_devices,nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart3",mc2661_device,rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("uart3",mc2661_device,dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart3",mc2661_device,dsr_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart3",mc2661_device,cts_w))

	MCFG_RS232_PORT_ADD("serial4",default_rs232_devices,nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart4",mc2661_device,rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("uart4",mc2661_device,dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart4",mc2661_device,dsr_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart4",mc2661_device,cts_w))

	MCFG_RS232_PORT_ADD("serial5",default_rs232_devices,nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart5",mc2661_device,rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("uart5",mc2661_device,dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart5",mc2661_device,dsr_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart5",mc2661_device,cts_w))

	/* video hardware */
	MCFG_CPU_ADD("videocpu",Z8002,XTAL_8MHz/2)  // AMD AMZ8002DC
	MCFG_CPU_PROGRAM_MAP(wicat_video_mem)
	MCFG_CPU_IO_MAP(wicat_video_io)

	MCFG_DEVICE_ADD("videodma", AM9517A, XTAL_8MHz)  // clock is a bit of guess
	MCFG_AM9517A_OUT_HREQ_CB(WRITELINE(wicat_state, dma_hrq_w))
	MCFG_AM9517A_OUT_EOP_CB(WRITELINE(wicat_state, dma_nmi_cb))
	MCFG_AM9517A_IN_MEMR_CB(READ8(wicat_state, vram_r))
	MCFG_AM9517A_OUT_MEMW_CB(WRITE8(wicat_state, vram_w))
	MCFG_AM9517A_OUT_IOW_0_CB(DEVWRITE8("video", i8275_device, dack_w))
	MCFG_IM6402_ADD("videouart", 0, 0)
	MCFG_IM6402_DR_CALLBACK(WRITELINE(wicat_state, kb_data_ready))

	// terminal (2x INS2651, 1x IM6042 - one of these is for the keyboard, another communicates with the main board, the third is unknown)
	MCFG_DEVICE_ADD("videouart0", MC2661, XTAL_5_0688MHz)  // the INS2651 looks similar enough to the MC2661...
	MCFG_MC2661_TXD_HANDLER(DEVWRITELINE("uart0", mc2661_device, rx_w))
	MCFG_MC2661_RXRDY_HANDLER(INPUTLINE("videocpu", INPUT_LINE_IRQ0))
	MCFG_MC2661_RTS_HANDLER(DEVWRITELINE("uart0", mc2661_device, cts_w))
	MCFG_MC2661_DTR_HANDLER(DEVWRITELINE("uart0", mc2661_device, dsr_w))

	MCFG_DEVICE_ADD("videouart1", MC2661, XTAL_5_0688MHz)
	MCFG_MC2661_RXC(19200)
	MCFG_MC2661_TXC(19200)
	MCFG_MC2661_RXRDY_HANDLER(INPUTLINE("videocpu", INPUT_LINE_IRQ0))

	MCFG_X2210_ADD("vsram")  // XD2210

	MCFG_SCREEN_ADD("screen",RASTER)
	MCFG_SCREEN_SIZE(720,300)
	MCFG_SCREEN_VISIBLE_AREA(0,720-1,0,300-1)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DEVICE("video",i8275_device,screen_update)

	MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")

	MCFG_DEVICE_ADD("video", I8275, XTAL_19_6608MHz/8)
	MCFG_I8275_CHARACTER_WIDTH(9)
	MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(wicat_state, wicat_display_pixels)
	MCFG_I8275_DRQ_CALLBACK(DEVWRITELINE("videodma",am9517a_device, dreq0_w))
	MCFG_I8275_IRQ_CALLBACK(WRITELINE(wicat_state,crtc_cb))
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_DEFAULT_LAYOUT(layout_wicat)

	/* Winchester Disk Controller (WD1000 + FD1795) */
	MCFG_CPU_ADD("wd1kcpu",N8X300,XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(wicat_wd1000_mem)
	MCFG_CPU_IO_MAP(wicat_wd1000_io)
	MCFG_FD1795_ADD("fdc",XTAL_8MHz)


MACHINE_CONFIG_END

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
	ROMX_LOAD("s156.a5", 0x00000, 0x0800, CRC(2c1e9542) SHA1(50184e04f0c881818e96e2162111d16304e8762f), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD("s156.a7", 0x00001, 0x0800, CRC(5a0cb30d) SHA1(aa106ad5a8b9e89613f7ea026d62832cfdb19fd0), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD("37m.b5",  0x01000, 0x0800, CRC(831571fb) SHA1(fcc647b3ef9f0cca3e8212f850f96676d24cf318), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD("37m.b7",  0x01001, 0x0800, CRC(3c346e8e) SHA1(d8ff8297d265b25655c854ed8515fa9e16c63f39), ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "wd3", "WD3 HD Boot / Boot v1.56" )
	ROMX_LOAD("boot156.a5", 0x00000, 0x0800, CRC(58510a52) SHA1(d2135b056a04ba830b0ae1cef539e4a9a1b58f82), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD("boot156.a7", 0x00001, 0x0800, CRC(e53999f1) SHA1(9c6c6a3a56b5c16a35e1fe824f37c8ae739ebcb9), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD("wd3_15.b5",  0x01000, 0x0800, CRC(a765899b) SHA1(8427c564029914b7dbc29768ce451604180e390f), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD("wd3_15.b7",  0x01001, 0x0800, CRC(9d986585) SHA1(1ac7579c692f827b121c56dac0a77b15400caba1), ROM_SKIP(1) | ROM_BIOS(2) )

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

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   CLASS         INIT    COMPANY          FULLNAME       FLAGS */
COMP( 1982, wicat, 0,       0,     wicat, wicat, driver_device, 0, "Millennium Systems", "Wicat System 150", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )

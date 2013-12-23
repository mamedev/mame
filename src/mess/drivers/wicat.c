// license:MAME
// copyright-holders:Robbbert
/***************************************************************************

Wicat - various systems.

2013-09-01 Skeleton driver

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z8000/z8000.h"
#include "cpu/8x300/8x300.h"
#include "machine/serial.h"
#include "machine/6522via.h"
#include "machine/mm58274c.h"
#include "machine/mc2661.h"
#include "machine/im6402.h"
#include "video/i8275x.h"
#include "machine/am9517a.h"
#include "machine/x2212.h"
#include "machine/wd17xx.h"
#include "wicat.lh"

class wicat_state : public driver_device
{
public:
	wicat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_vram(*this, "vram")
		, m_maincpu(*this, "maincpu")
		, m_rtc(*this, "rtc")
		, m_uart0(*this,"uart0")
		, m_uart1(*this,"uart1")
		, m_uart2(*this,"uart2")
		, m_uart3(*this,"uart3")
		, m_uart4(*this,"uart4")
		, m_uart5(*this,"uart5")
		, m_uart6(*this,"uart6")
		, m_videocpu(*this,"videocpu")
		, m_videoctrl(*this,"video")
		, m_videodma(*this,"videodma")
		, m_videouart0(*this,"videouart0")
		, m_videouart1(*this,"videouart1")
		, m_videouart(*this,"videouart")
		, m_videosram(*this,"videosram")
		, m_chargen(*this,"g2char")
	{ }

	DECLARE_READ16_MEMBER(invalid_r);
	DECLARE_WRITE16_MEMBER(invalid_w);
	DECLARE_READ16_MEMBER(memmap_r);
	DECLARE_WRITE16_MEMBER(memmap_w);
	DECLARE_WRITE16_MEMBER(parallel_led_w);
	DECLARE_READ8_MEMBER(via_a_r);
	DECLARE_READ8_MEMBER(via_b_r);
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

	required_shared_ptr<UINT8> m_vram;
	required_device<m68000_device> m_maincpu;
	required_device<mm58274c_device> m_rtc;
	required_device<mc2661_device> m_uart0;
	required_device<mc2661_device> m_uart1;
	required_device<mc2661_device> m_uart2;
	required_device<mc2661_device> m_uart3;
	required_device<mc2661_device> m_uart4;
	required_device<mc2661_device> m_uart5;
	required_device<mc2661_device> m_uart6;
	required_device<cpu_device> m_videocpu;
	required_device<i8275x_device> m_videoctrl;
	required_device<am9517a_device> m_videodma;
	required_device<mc2661_device> m_videouart0;
	required_device<mc2661_device> m_videouart1;
	required_device<im6402_device> m_videouart;
	required_device<x2212_device> m_videosram;
	required_memory_region m_chargen;

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) { return 0; }
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);


private:
	virtual void machine_start();
	virtual void machine_reset();
	virtual void driver_start();

	emu_timer* m_video_timer;
	static const device_timer_id VIDEO_TIMER = 0;

	UINT8 m_portA;
	UINT8 m_portB;
	bool m_video_timer_irq;
	UINT8 m_nmi_enable;
	UINT8 m_crtc_irq;
};


static ADDRESS_MAP_START(wicat_mem, AS_PROGRAM, 16, wicat_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xffffff)
	AM_RANGE(0x000000, 0x001fff) AM_ROM AM_REGION("c2", 0x0000)
	AM_RANGE(0x020000, 0x1fffff) AM_RAM
	AM_RANGE(0x200000, 0x2fffff) AM_RAM
	AM_RANGE(0x300000, 0xdfffff) AM_READWRITE(invalid_r,invalid_w)
	AM_RANGE(0xeff800, 0xeffbff) AM_RAM  // memory mapping SRAM, used during boot sequence for the stack (TODO)
	AM_RANGE(0xeffc00, 0xeffc01) AM_READWRITE(memmap_r,memmap_w)
	AM_RANGE(0xf00000, 0xf00007) AM_DEVREADWRITE8("uart0",mc2661_device,read,write,0xff00)  // UARTs
	AM_RANGE(0xf00008, 0xf0000f) AM_DEVREADWRITE8("uart1",mc2661_device,read,write,0xff00)
	AM_RANGE(0xf00010, 0xf00017) AM_DEVREADWRITE8("uart2",mc2661_device,read,write,0xff00)
	AM_RANGE(0xf00018, 0xf0001f) AM_DEVREADWRITE8("uart3",mc2661_device,read,write,0xff00)
	AM_RANGE(0xf00020, 0xf00027) AM_DEVREADWRITE8("uart4",mc2661_device,read,write,0xff00)
	AM_RANGE(0xf00028, 0xf0002f) AM_DEVREADWRITE8("uart5",mc2661_device,read,write,0xff00)
	AM_RANGE(0xf00030, 0xf00037) AM_DEVREADWRITE8("uart6",mc2661_device,read,write,0xff00)
	AM_RANGE(0xf00040, 0xf0005f) AM_DEVREADWRITE8("via",via6522_device,read,write,0xff00)
	AM_RANGE(0xf00060, 0xf0007f) AM_DEVREADWRITE8("rtc",mm58274c_device,read,write,0xff00)
	AM_RANGE(0xf000d0, 0xf000d1) AM_WRITE(parallel_led_w)
	AM_RANGE(0xf00f00, 0xf00fff) AM_READWRITE(invalid_r,invalid_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(wicat_video_mem, AS_PROGRAM, 16, wicat_state)
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("g2", 0x0000)
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(wicat_video_io, AS_IO, 8, wicat_state)
	// these are largely wild guesses...
	AM_RANGE(0x0000,0x0003) AM_READWRITE(video_timer_r,video_timer_w)  // some sort of timer?
	AM_RANGE(0x0100,0x0107) AM_READWRITE(video_uart0_r,video_uart0_w)  // INS2651 UART #1
	AM_RANGE(0x0200,0x0207) AM_READWRITE(video_uart1_r,video_uart1_w)  // INS2651 UART #2
	AM_RANGE(0x0304,0x0304) AM_READ(video_status_r)
	AM_RANGE(0x0400,0x047f) AM_READWRITE(videosram_r,videosram_w)  // XD2210  4-bit NOVRAM
	AM_RANGE(0x0700,0x0700) AM_DEVREADWRITE("videouart",im6402_device,read,write)  // UART?
	AM_RANGE(0x0800,0x080f) AM_READWRITE(video_ctrl_r,video_ctrl_w)
	AM_RANGE(0x0a00,0x0a1f) AM_READWRITE(video_dma_r,video_dma_w) // DMA
	AM_RANGE(0x0b00,0x0b03) AM_READWRITE(video_r,video_w)  // i8275 CRTC
	AM_RANGE(0x0e00,0x0eff) AM_RAM
	AM_RANGE(0x4000,0x5fff) AM_RAM AM_SHARE("vram") // video RAM?
	AM_RANGE(0x8000,0x8fff) AM_ROM AM_REGION("g2char",0x0000)
	AM_RANGE(0x9000,0x9fff) AM_ROM AM_REGION("g2char",0x0000)
ADDRESS_MAP_END

static ADDRESS_MAP_START(wicat_flop_mem, AS_PROGRAM, 16, wicat_state)
	AM_RANGE(0x0000, 0x17ff) AM_ROM AM_REGION("wd3", 0x0000)
	AM_RANGE(0x1800, 0x1fff) AM_NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START(wicat_flop_io, AS_IO, 8, wicat_state)
	AM_RANGE(0x0000, 0x00ff) AM_RAM  // left bank
	AM_RANGE(0x0100, 0x01ff) AM_RAM  // right bank  -- one of these probably is RAM...
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( wicat )
INPUT_PORTS_END

void wicat_state::driver_start()
{
	m_video_timer = timer_alloc(VIDEO_TIMER);
}

void wicat_state::machine_start()
{
}

void wicat_state::machine_reset()
{
	// on the terminal board /DCD on both INS2651s is tied to GND
	m_videouart0->dcd_w(0);
	m_videouart1->dcd_w(0);
	m_uart0->dcd_w(0);
	m_video_timer_irq = false;
	m_video_timer->adjust(attotime::zero,0,attotime::from_hz(60));
	m_nmi_enable = 0;
	m_crtc_irq = CLEAR_LINE;
}

void wicat_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case VIDEO_TIMER:
		m_video_timer_irq = true;
		m_videocpu->set_input_line(INPUT_LINE_IRQ0,ASSERT_LINE);
		break;
	}
}
WRITE16_MEMBER( wicat_state::parallel_led_w )
{
	// bit 0 - parallel port A direction (0 = input)
	// bit 1 - parallel port B direction (0 = input)
	output_set_value("led1",(~data) & 0x0400);
	output_set_value("led2",(~data) & 0x0800);
	output_set_value("led3",(~data) & 0x1000);
	output_set_value("led4",(~data) & 0x2000);
	output_set_value("led5",(~data) & 0x4000);
	output_set_value("led6",(~data) & 0x8000);
}

READ8_MEMBER( wicat_state::via_a_r )
{
	return m_portA;
}

READ8_MEMBER( wicat_state::via_b_r )
{
	return m_portB;
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
	m68k_set_buserror_details(m_maincpu,0x300000+offset*2-2,0,m68k_get_fc(m_maincpu));
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	return 0xff;
}

WRITE16_MEMBER( wicat_state::invalid_w )
{
	m68k_set_buserror_details(m_maincpu,0x300000+offset*2-2,1,m68k_get_fc(m_maincpu));
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
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

READ8_HANDLER(wicat_state::video_uart0_r)
{
	UINT16 noff = offset >> 1;
	return m_videouart0->read(space,noff);
}

WRITE8_HANDLER(wicat_state::video_uart0_w)
{
	UINT16 noff = offset >> 1;
	m_videouart0->write(space,noff,data);
}

READ8_HANDLER(wicat_state::video_uart1_r)
{
	UINT16 noff = offset >> 1;
	return m_videouart1->read(space,noff);
}

WRITE8_HANDLER(wicat_state::video_uart1_w)
{
	UINT16 noff = offset >> 1;
	m_videouart1->write(space,noff,data);
}

// XD2210 256 x 4bit NOVRAM
READ8_MEMBER(wicat_state::videosram_r)
{
	if(offset == 0x08 || offset == 0x0c)
		return 0x08;
	if(offset == 0x06 || offset == 0x0a)
		return 0x0e;

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
	}
	return ret;
}

WRITE8_MEMBER(wicat_state::video_timer_w)
{
	logerror("I/O port 0x%04x write %02x\n",offset,data);
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
		return 0x04;
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

WRITE_LINE_MEMBER(wicat_state::crtc_cb)
{
	m_crtc_irq = state ? ASSERT_LINE : CLEAR_LINE;
	m_videocpu->set_input_line(INPUT_LINE_IRQ0,m_crtc_irq);
}

I8275_DISPLAY_PIXELS(wicat_display_pixels)
{
	wicat_state *state = device->machine().driver_data<wicat_state>();

	UINT8 romdata = state->m_chargen->base()[((charcode << 4) | linecount) + 1];
	int i;

	for (i = 0; i < 8; i++)
	{
		int color = (romdata >> (7-i)) & 0x01;

		if(linecount > 9)
			color = 0;

		bitmap.pix32(y, x + i) = RGB_MONOCHROME_GREEN_HIGHLIGHT[color];
	}
}

// internal terminal
static mc2661_interface wicat_uart0_intf =
{
	19200,  // RXC
	19200,  // TXC
	DEVCB_DEVICE_LINE_MEMBER("videouart0",mc2661_device, rx_w),  // TXD out
	DEVCB_CPU_INPUT_LINE("maincpu",M68K_IRQ_2),  // RXRDY out
	DEVCB_NULL,  // TXRDY out
	DEVCB_DEVICE_LINE_MEMBER("videouart0", mc2661_device, cts_w),  // RTS out
	DEVCB_DEVICE_LINE_MEMBER("videouart0", mc2661_device, dsr_w),  // DTR out
	DEVCB_NULL,  // TXEMT out
	DEVCB_NULL,  // BKDET out
	DEVCB_NULL   // XSYNC out
};

// RS232C ports (x5)
static mc2661_interface wicat_uart1_intf =
{
	0,
	0,
	DEVCB_DEVICE_LINE_MEMBER("serial1", serial_port_device, tx),
	DEVCB_CPU_INPUT_LINE("maincpu",M68K_IRQ_2),  // RXRDY out
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER("serial1", rs232_port_device, rts_w),
	DEVCB_DEVICE_LINE_MEMBER("serial1", rs232_port_device, dtr_w),
	DEVCB_CPU_INPUT_LINE("maincpu",M68K_IRQ_2),  // TXEMT out
	DEVCB_NULL,
	DEVCB_NULL
};

static mc2661_interface wicat_uart2_intf =
{
	0,
	0,
	DEVCB_DEVICE_LINE_MEMBER("serial2", serial_port_device, tx),
	DEVCB_CPU_INPUT_LINE("maincpu",M68K_IRQ_2),  // RXRDY out
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER("serial2", rs232_port_device, rts_w),
	DEVCB_DEVICE_LINE_MEMBER("serial2", rs232_port_device, dtr_w),
	DEVCB_NULL,  // TXEMT out
	DEVCB_NULL,
	DEVCB_NULL
};

static mc2661_interface wicat_uart3_intf =
{
	0,
	0,
	DEVCB_DEVICE_LINE_MEMBER("serial3", serial_port_device, tx),
	DEVCB_CPU_INPUT_LINE("maincpu",M68K_IRQ_2),  // RXRDY out
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER("serial3", rs232_port_device, rts_w),
	DEVCB_DEVICE_LINE_MEMBER("serial3", rs232_port_device, dtr_w),
	DEVCB_CPU_INPUT_LINE("maincpu",M68K_IRQ_2),  // TXEMT out
	DEVCB_NULL,
	DEVCB_NULL
};

static mc2661_interface wicat_uart4_intf =
{
	0,
	0,
	DEVCB_DEVICE_LINE_MEMBER("serial4", serial_port_device, tx),
	DEVCB_CPU_INPUT_LINE("maincpu",M68K_IRQ_2),  // RXRDY out
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER("serial4", rs232_port_device, rts_w),
	DEVCB_DEVICE_LINE_MEMBER("serial4", rs232_port_device, dtr_w),
	DEVCB_CPU_INPUT_LINE("maincpu",M68K_IRQ_2),  // TXEMT out
	DEVCB_NULL,
	DEVCB_NULL
};

static mc2661_interface wicat_uart5_intf =
{
	0,
	0,
	DEVCB_DEVICE_LINE_MEMBER("serial5", serial_port_device, tx),
	DEVCB_CPU_INPUT_LINE("maincpu",M68K_IRQ_2),  // RXRDY out
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER("serial5", rs232_port_device, rts_w),
	DEVCB_DEVICE_LINE_MEMBER("serial5", rs232_port_device, dtr_w),
	DEVCB_CPU_INPUT_LINE("maincpu",M68K_IRQ_2),  // TXEMT out
	DEVCB_NULL,
	DEVCB_NULL
};

// modem
static mc2661_interface wicat_uart6_intf =
{
	0,  // RXC
	0,  // TXC
	DEVCB_NULL, //DEVCB_DEVICE_LINE_MEMBER(RS232_TAG, serial_port_device, tx),  // RXD out
	DEVCB_CPU_INPUT_LINE("maincpu",M68K_IRQ_2),  // RXRDY out
	DEVCB_NULL,  // TXRDY out
	DEVCB_NULL, //DEVCB_DEVICE_LINE_MEMBER(RS232_TAG, rs232_port_device, rts_w),  // RTS out
	DEVCB_NULL, //DEVCB_DEVICE_LINE_MEMBER(RS232_TAG, rs232_port_device, dtr_w),  // DTR out
	DEVCB_CPU_INPUT_LINE("maincpu",M68K_IRQ_2),  // TXEMT out
	DEVCB_NULL,  // BKDET out
	DEVCB_NULL   // XSYNC out
};

// terminal (2x INS2651, 1x IM6042 - one of these is for the keyboard, another communicates with the main board, the third is unknown)
static mc2661_interface wicat_video_uart0_intf =
{
	19200,  // RXC
	19200,  // TXC
	DEVCB_DEVICE_LINE_MEMBER("uart0",mc2661_device, rx_w),  // RXD out
	DEVCB_CPU_INPUT_LINE("videocpu",INPUT_LINE_IRQ0),  // RXRDY out
	DEVCB_NULL,  // TXRDY out
	DEVCB_DEVICE_LINE_MEMBER("uart0",mc2661_device, cts_w),  // RTS out
	DEVCB_DEVICE_LINE_MEMBER("uart0",mc2661_device, dsr_w),  // DTR out
	DEVCB_NULL,  // TXEMT out
	DEVCB_NULL,  // BKDET out
	DEVCB_NULL   // XSYNC out
};

static mc2661_interface wicat_video_uart1_intf =
{
	19200,  // RXC
	19200,  // TXC
	DEVCB_NULL,  // RXD out
	DEVCB_CPU_INPUT_LINE("videocpu",INPUT_LINE_IRQ0),  // RXRDY out
	DEVCB_NULL,  // TXRDY out
	DEVCB_NULL, //DEVCB_DEVICE_LINE_MEMBER(RS232_TAG, rs232_port_device, rts_w),  // RTS out
	DEVCB_NULL, //DEVCB_DEVICE_LINE_MEMBER(RS232_TAG, rs232_port_device, dtr_w),  // DTR out
	DEVCB_CPU_INPUT_LINE("videocpu",INPUT_LINE_IRQ0),  // TXEMT out
	DEVCB_NULL,  // BKDET out
	DEVCB_NULL   // XSYNC out
};

struct im6402_interface wicat_video_uart_intf =
{
	0,  // RRC
	0,  // TRC

	DEVCB_NULL, //m_out_tro_cb;
	DEVCB_NULL, //m_out_dr_cb;
	DEVCB_NULL, //m_out_tbre_cb;
	DEVCB_NULL, //m_out_tre_cb;
};

static via6522_interface wicat_via_intf =
{
	DEVCB_DRIVER_MEMBER(wicat_state,via_a_r),  // Port A in
	DEVCB_DRIVER_MEMBER(wicat_state,via_b_r),  // Port B in
	DEVCB_NULL,  // CA1 in
	DEVCB_NULL,  // CB1 in
	DEVCB_NULL,  // CA2 in
	DEVCB_NULL,  // CB2 in
	DEVCB_DRIVER_MEMBER(wicat_state,via_a_w),  // Port A out
	DEVCB_DRIVER_MEMBER(wicat_state,via_b_w),  // Port B out
	DEVCB_NULL,  // CA1 out
	DEVCB_NULL,  // CB1 out
	DEVCB_NULL,  // CA2 out
	DEVCB_NULL,  // CB2 out
	DEVCB_CPU_INPUT_LINE("maincpu", M68K_IRQ_1)  // IRQ
};

static mm58274c_interface wicat_rtc_intf =
{
	0,  // 12 hour
	1   // first day
};

AM9517A_INTERFACE( wicat_videodma_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(wicat_state,dma_hrq_w), // m_out_hreq_cb;
	DEVCB_DRIVER_LINE_MEMBER(wicat_state,dma_nmi_cb), // m_out_eop_cb;

	DEVCB_DRIVER_MEMBER(wicat_state,vram_r), // m_in_memr_cb;
	DEVCB_DRIVER_MEMBER(wicat_state,vram_w), // m_out_memw_cb;

	{ DEVCB_NULL,DEVCB_NULL,DEVCB_NULL,DEVCB_NULL }, // m_in_ior_cb[4];
	{ DEVCB_DEVICE_MEMBER("video", i8275x_device, dack_w),DEVCB_NULL,DEVCB_NULL,DEVCB_NULL }, // m_out_iow_cb[4];
	{ DEVCB_NULL,DEVCB_NULL,DEVCB_NULL,DEVCB_NULL }  // m_out_dack_cb[4];
};

static MACHINE_CONFIG_START( wicat, wicat_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(wicat_mem)

	MCFG_VIA6522_ADD("via",XTAL_4MHz,wicat_via_intf)

	MCFG_MM58274C_ADD("rtc",wicat_rtc_intf)  // actually an MM58174AN, but should be compatible

	MCFG_MC2661_ADD("uart0", XTAL_5_0688MHz, wicat_uart0_intf)  // connected to terminal board (TODO)
	MCFG_MC2661_ADD("uart1", XTAL_5_0688MHz, wicat_uart1_intf)
	MCFG_MC2661_ADD("uart2", XTAL_5_0688MHz, wicat_uart2_intf)
	MCFG_MC2661_ADD("uart3", XTAL_5_0688MHz, wicat_uart3_intf)
	MCFG_MC2661_ADD("uart4", XTAL_5_0688MHz, wicat_uart4_intf)
	MCFG_MC2661_ADD("uart5", XTAL_5_0688MHz, wicat_uart5_intf)
	MCFG_MC2661_ADD("uart6", XTAL_5_0688MHz, wicat_uart6_intf)  // connected to modem port

	MCFG_RS232_PORT_ADD("serial1",default_rs232_devices,NULL)
	MCFG_SERIAL_OUT_RX_HANDLER(DEVWRITELINE("uart1",mc2661_device,rx_w))
	MCFG_RS232_OUT_DCD_HANDLER(DEVWRITELINE("uart1",mc2661_device,dcd_w))
	MCFG_RS232_OUT_DSR_HANDLER(DEVWRITELINE("uart1",mc2661_device,dsr_w))
	MCFG_RS232_OUT_CTS_HANDLER(DEVWRITELINE("uart1",mc2661_device,cts_w))

	MCFG_RS232_PORT_ADD("serial2",default_rs232_devices,NULL)
	MCFG_SERIAL_OUT_RX_HANDLER(DEVWRITELINE("uart2",mc2661_device,rx_w))
	MCFG_RS232_OUT_DCD_HANDLER(DEVWRITELINE("uart2",mc2661_device,dcd_w))
	MCFG_RS232_OUT_DSR_HANDLER(DEVWRITELINE("uart2",mc2661_device,dsr_w))
	MCFG_RS232_OUT_CTS_HANDLER(DEVWRITELINE("uart2",mc2661_device,cts_w))

	MCFG_RS232_PORT_ADD("serial3",default_rs232_devices,NULL)
	MCFG_SERIAL_OUT_RX_HANDLER(DEVWRITELINE("uart3",mc2661_device,rx_w))
	MCFG_RS232_OUT_DCD_HANDLER(DEVWRITELINE("uart3",mc2661_device,dcd_w))
	MCFG_RS232_OUT_DSR_HANDLER(DEVWRITELINE("uart3",mc2661_device,dsr_w))
	MCFG_RS232_OUT_CTS_HANDLER(DEVWRITELINE("uart3",mc2661_device,cts_w))

	MCFG_RS232_PORT_ADD("serial4",default_rs232_devices,NULL)
	MCFG_SERIAL_OUT_RX_HANDLER(DEVWRITELINE("uart4",mc2661_device,rx_w))
	MCFG_RS232_OUT_DCD_HANDLER(DEVWRITELINE("uart4",mc2661_device,dcd_w))
	MCFG_RS232_OUT_DSR_HANDLER(DEVWRITELINE("uart4",mc2661_device,dsr_w))
	MCFG_RS232_OUT_CTS_HANDLER(DEVWRITELINE("uart4",mc2661_device,cts_w))

	MCFG_RS232_PORT_ADD("serial5",default_rs232_devices,NULL)
	MCFG_SERIAL_OUT_RX_HANDLER(DEVWRITELINE("uart5",mc2661_device,rx_w))
	MCFG_RS232_OUT_DCD_HANDLER(DEVWRITELINE("uart5",mc2661_device,dcd_w))
	MCFG_RS232_OUT_DSR_HANDLER(DEVWRITELINE("uart5",mc2661_device,dsr_w))
	MCFG_RS232_OUT_CTS_HANDLER(DEVWRITELINE("uart5",mc2661_device,cts_w))

	/* video hardware */
	MCFG_CPU_ADD("videocpu",Z8002,XTAL_8MHz/2)  // AMD AMZ8002DC
	MCFG_CPU_PROGRAM_MAP(wicat_video_mem)
	MCFG_CPU_IO_MAP(wicat_video_io)

	MCFG_AM9517A_ADD("videodma", XTAL_8MHz, wicat_videodma_intf)  // clock is a bit of guess
	MCFG_IM6402_ADD("videouart", wicat_video_uart_intf)
	MCFG_MC2661_ADD("videouart0", XTAL_5_0688MHz, wicat_video_uart0_intf)  // the INS2651 looks similar enough to the MC2661...
	MCFG_MC2661_ADD("videouart1", XTAL_5_0688MHz, wicat_video_uart1_intf)
	MCFG_X2212_ADD("videosram")  // XD2210

	MCFG_SCREEN_ADD("screen",RASTER)
	MCFG_SCREEN_SIZE(720,300)
	MCFG_SCREEN_VISIBLE_AREA(0,720-1,0,300-1)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DEVICE("video",i8275x_device,screen_update)

	MCFG_I8275_ADD("video",XTAL_19_6608MHz/8,9,wicat_display_pixels,DEVWRITELINE("videodma",am9517a_device, dreq0_w))
	MCFG_I8275_IRQ_CALLBACK(WRITELINE(wicat_state,crtc_cb))
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_DEFAULT_LAYOUT(layout_wicat)

	/* Winchester Floppy Controller */
	MCFG_CPU_ADD("floppycpu",N8X300,XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(wicat_flop_mem)
	MCFG_CPU_IO_MAP(wicat_flop_io)
//	MCFG_FD1795_ADD("fdc")

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
	ROM_LOAD16_BYTE("boot156.a5", 0x00000, 0x0800, CRC(58510a52) SHA1(d2135b056a04ba830b0ae1cef539e4a9a1b58f82) )
	ROM_LOAD16_BYTE("boot156.a7", 0x00001, 0x0800, CRC(e53999f1) SHA1(9c6c6a3a56b5c16a35e1fe824f37c8ae739ebcb9) )
	ROM_LOAD16_BYTE("wd3_15.b5",  0x01000, 0x0800, CRC(a765899b) SHA1(8427c564029914b7dbc29768ce451604180e390f) )
	ROM_LOAD16_BYTE("wd3_15.b7",  0x01001, 0x0800, CRC(9d986585) SHA1(1ac7579c692f827b121c56dac0a77b15400caba1) )

	// Terminal CPU board (Graphical)
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

	// Terminal Video board
	ROM_REGION(0x1000, "g2char", 0)
	ROM_LOAD       ("ascii.chr",  0x00000, 0x0800, CRC(43e26e37) SHA1(f3d5d16040c66f0e827f72a35d4694ca62950949) )
	ROM_LOAD       ("apl.chr",    0x00800, 0x0800, CRC(8c6d698e) SHA1(147dd9296fe2efc6140fa148a6edf673c33f9371) )

	// Winchester Floppy Controller  (Signetics N8X300I + FD1795)
	ROM_REGION16_BE(0x1800, "wd3", 0)
	ROM_LOAD16_BYTE("wd3.u96",    0x00000, 0x0800, CRC(52736e61) SHA1(71c7c9170c733c483393969cb1cb3798b3eb980c) )
	ROM_LOAD16_BYTE("wd3.u97",    0x00001, 0x0800, CRC(a66619ec) SHA1(5d091ac7c88f2f45b4a05e78bfc7a16c206b31ff) )
	ROM_LOAD       ("wd3.u95",    0x01000, 0x0800, CRC(80bb0617) SHA1(ac0f3194fcbef77532571baa3fec78b3010528bf) )
ROM_END


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   CLASS         INIT    COMPANY          FULLNAME       FLAGS */
COMP( 1982, wicat, 0,       0,     wicat, wicat, driver_device, 0, "Millennium Systems", "Wicat System 150", GAME_NOT_WORKING | GAME_NO_SOUND_HW )

/******************************************************************************

Pasogo Handheld Console
Koei 1996

This is a handheld console made by Koei in 1996. All of the games available on
this console are variations of the game 'Go'


Main PCB Layout
---------------
PT-GMAIN01D
|--------------|------------||
|POWER    VOL  |            ||
|       LM2937*|    CART    ||
|CE-0702       |    SLOT    ||
|              |    CN1     ||
|              |MC14071B*   ||
|              |            ||
| PIEZO_SPKR   |------------||
|CN5           74HC04*       |
|    X2                      |
|  |-------|              CN4|
|  |VADEM  |                 |
|  |VG230  |                 |
|  |       |     HM514800*   |
|  |       |              CN3|
|  |-------|     HM514800*   |
|CN2*  X1                    |
|----------------------------|
Notes: (all ICs shown)

       VG230    - Vadem VG230 single-chip PC platform. Contains 16 MHz NEC uPD70116H V30HL CPU
                  (which is a high-speed low-power 8086 variation), IBM PC/XT-compatible core
                  logic, LCD controller (CGA/AT&T640x400), keyboard matrix scanner, dual PCMCIA
                  2.1 card controller, EMS 4.0 hardware support for up to 64MB, built-in timer
                  PIC/DMA/UART/RTC controllers. The clock input is 32.2200MHz. An internal divider
                  creates a 16.11MHz clock for the V30HL CPU.
       HM514800 - Hitachi HM514800 512k x8-bit DRAM
       MC14071  - Motorola MC14071 Quad 2-input OR gate
       74HC04   - 74HC04 Hex inverter
       LM2937   - Texas Instruments LM2937ES-5 voltage regulator (Max 26V input, 5V output at 500mA)
       CE-0702  - TDK CE-0702 DC-DC converter for LCD in SIP9 package (5V input, -24V output at 25mA)
       POWER    - 9V DC power input from AC/DC adapter
       VOL      - Volume pot
       CN5      - 5 pin connector for 4-way control pad (up/down/left/right/ground)
       CN4      - 5 pin connector for on/off switch and 2 buttons
       CN3      - 2 pin power input from 6x AA-battery compartment (input voltage is 9V DC)
       CN2      - Flat cable connector for video out to LCD panel. When the LCD is powered on the pixels
                  are blue. The LCD panel PCB has part number 97-44264-8 LMG6912RPFC LMG6910RPGR
                  and contains the following parts.....
                  Matsushita 53008HEB-8
                  Sanyo LA6324N quad operational amplifier
                  Hitachi BD66285BFC LCD controller IC (x3)
                  Hitachi BD66284BFC LCD controller IC (x4)
                  The LCD flat cable has several wires but 2 of them have frequencies which measure
                  69.9161Hz and 16.7798kHz. These are assumed to be VSync and HSync
       CN1      - Cart slot
       X1       - Marked 322. Measures 32.21732MHz so this is a common 32.22MHz OSC.
       X2       - No markings. Measures 32.768kHz and used for the RTC
       *        - These parts are on the other side of the PCB


Carts
-----
All of the carts are identical. Most have only one surface mounted mask ROM. Either a
MX23C8100 (8M) or YRM0442 (4M). Some are populated with additional parts including a 62256
32kx8 SRAM, a 3v coin battery and a MM1081N reset chip plus a few resistors/capacitors and
a transistor. All parts are surface mounted.

PT-GMEM01B
PT-GMEM01C
|---------------|
|--     CR2016  |
|-- MM1081      |
|--             |
|--             |
|--       62256 |
|--             |
|--             |
|--    MX23C8100|
|-- or YRM0442  |
|---------------|
Notes:
      Carts containing just one mask ROM are KS-1002, KS-1003, KS1004 and KS-1009
      Carts containing RAM, battery and reset chip are KS-1001 and KS1010
      Carts KS-1005, KS-1006, KS-1007 and KS-1008 probably exist but are unknown.

===========================================================================================

 PeT mess@utanet.at march 2008

although it is very related to standard pc hardware, it is different enough
to make the standard pc driver one level more complex, so own driver

TODO:
- Make a separate device of the Vadem VG230 core (it is also used in the HP
  OmniGo 100 and 120).

******************************************************************************/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/am9517a.h"
#include "machine/i8255.h"
#include "sound/speaker.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"


/*
  rtc interrupt irq 2
 */

struct vg230_t
{
	UINT8 index;
	UINT8 data[0x100];
	struct {
		UINT16 data;
	} bios_timer; // 1.19 MHz tclk signal
	struct {
		int seconds, minutes, hours, days;
		int alarm_seconds, alarm_minutes, alarm_hours, alarm_days;

		int onehertz_interrupt_on;
		int onehertz_interrupt_request;
		int alarm_interrupt_on;
		int alarm_interrupt_request;
	} rtc;
	struct {
		int write_protected;
	} pmu;
};

struct ems_t
{
	UINT8 data;
	int index;
	struct {
		UINT8 data[2];
		int address;
		int type;
		int on;
	} mapper[26];
};

class pasogo_state : public driver_device
{
public:
	pasogo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pic8259(*this, "pic8259")
		, m_dma8237(*this, "dma8237")
		, m_pit8253(*this, "pit8254")
		, m_speaker(*this, "speaker")
		, m_cart(*this, "cartslot")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic8259;
	required_device<am9517a_device> m_dma8237;
	required_device<pit8254_device> m_pit8253;
	required_device<speaker_sound_device> m_speaker;
	required_device<generic_slot_device> m_cart;

	DECLARE_READ8_MEMBER(ems_r);
	DECLARE_WRITE8_MEMBER(ems_w);
	DECLARE_READ8_MEMBER(vg230_io_r);
	DECLARE_WRITE8_MEMBER(vg230_io_w);
	vg230_t m_vg230;
	ems_t m_ems;
	DECLARE_DRIVER_INIT(pasogo);
	virtual void machine_reset();
	DECLARE_PALETTE_INIT(pasogo);
	UINT32 screen_update_pasogo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(pasogo_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(vg230_timer);
	void vg230_reset();
	void vg230_init();
	DECLARE_READ8_MEMBER( page_r );
	DECLARE_WRITE8_MEMBER( page_w );
	DECLARE_WRITE_LINE_MEMBER( speaker_set_spkrdata );
	DECLARE_WRITE_LINE_MEMBER( pit8253_out1_changed );
	DECLARE_WRITE_LINE_MEMBER( pit8253_out2_changed );
	DECLARE_WRITE_LINE_MEMBER( dma_hrq_changed );
	DECLARE_WRITE_LINE_MEMBER( dma8237_out_eop );
	DECLARE_READ8_MEMBER( dma_read_byte );
	DECLARE_WRITE8_MEMBER( dma_write_byte );
	DECLARE_READ8_MEMBER( dma8237_1_dack_r );
	DECLARE_READ8_MEMBER( dma8237_2_dack_r );
	DECLARE_READ8_MEMBER( dma8237_3_dack_r );
	DECLARE_WRITE8_MEMBER( dma8237_0_dack_w );
	DECLARE_WRITE8_MEMBER( dma8237_1_dack_w );
	DECLARE_WRITE8_MEMBER( dma8237_2_dack_w );
	DECLARE_WRITE8_MEMBER( dma8237_3_dack_w );
	void select_dma_channel(int channel, bool state);
	DECLARE_WRITE_LINE_MEMBER( dack0_w ) { select_dma_channel(0, state); }
	DECLARE_WRITE_LINE_MEMBER( dack1_w ) { select_dma_channel(1, state); }
	DECLARE_WRITE_LINE_MEMBER( dack2_w ) { select_dma_channel(2, state); }
	DECLARE_WRITE_LINE_MEMBER( dack3_w ) { select_dma_channel(3, state); }
	DECLARE_READ8_MEMBER( ppi_porta_r );
	DECLARE_READ8_MEMBER( ppi_portc_r );
	DECLARE_WRITE8_MEMBER( ppi_portb_w );

protected:
	UINT8 m_u73_q2;
	UINT8 m_out1;
	int m_dma_channel;
	bool m_cur_eop;
	UINT8 m_dma_offset[4];
	UINT8 m_pc_spkrdata;
	UINT8 m_pit_out2;

	memory_region *m_maincpu_rom;
	memory_region *m_cart_rom;

	int m_ppi_portc_switch_high;
	int m_ppi_speaker;
	int m_ppi_keyboard_clear;
	UINT8 m_ppi_keyb_clock;
	UINT8 m_ppi_portb;
	UINT8 m_ppi_clock_signal;
	UINT8 m_ppi_data_signal;
	UINT8 m_ppi_shift_register;
	UINT8 m_ppi_shift_enable;

};


TIMER_DEVICE_CALLBACK_MEMBER(pasogo_state::vg230_timer)
{
	m_vg230.rtc.seconds += 1;
	if (m_vg230.rtc.seconds >= 60)
	{
		m_vg230.rtc.seconds = 0;
		m_vg230.rtc.minutes += 1;
		if (m_vg230.rtc.minutes >= 60)
		{
			m_vg230.rtc.minutes = 0;
			m_vg230.rtc.hours += 1;
			if (m_vg230.rtc.hours >= 24)
			{
				m_vg230.rtc.hours = 0;
				m_vg230.rtc.days = (m_vg230.rtc.days + 1) & 0xfff;
			}
		}
	}

	if (m_vg230.rtc.seconds == m_vg230.rtc.alarm_seconds
		&& m_vg230.rtc.minutes == m_vg230.rtc.alarm_minutes
		&& m_vg230.rtc.hours == m_vg230.rtc.alarm_hours
		&& (m_vg230.rtc.days & 0x1f) == m_vg230.rtc.alarm_hours)
	{
		// generate alarm
	}
}

void pasogo_state::vg230_reset()
{
	system_time systime;

	memset(&m_vg230, 0, sizeof(m_vg230));
	m_vg230.pmu.write_protected = TRUE;
	machine().base_datetime(systime);

	m_vg230.rtc.seconds = systime.local_time.second;
	m_vg230.rtc.minutes = systime.local_time.minute;
	m_vg230.rtc.hours = systime.local_time.hour;
	m_vg230.rtc.days = 0;

	m_vg230.bios_timer.data=0x7200; // HACK
}

void pasogo_state::vg230_init()
{
	vg230_reset();
}


READ8_MEMBER( pasogo_state::vg230_io_r )
{
	int log = TRUE;
	UINT8 data = 0;

	m_vg230.bios_timer.data += 0x100; //HACK
	if (offset&1)
	{
		data = m_vg230.data[m_vg230.index];
		switch (m_vg230.index)
		{
			case 0x09:
				break;

			case 0x0a:
				if (m_vg230.data[9] & 1)
					data=ioport("JOY")->read();
				else
					data = 0xff;
				break;

			case 0x30:
				data = m_vg230.bios_timer.data & 0xff;
				break;

			case 0x31:
				data = m_vg230.bios_timer.data >> 8;
				log = FALSE;
				break;

			case 0x70:
				data = m_vg230.rtc.seconds;
				log = FALSE;
				break;

			case 0x71:
				data = m_vg230.rtc.minutes;
				log = FALSE;
				break;

			case 0x72:
				data = m_vg230.rtc.hours;
				log = FALSE;
				break;

			case 0x73:
				data = m_vg230.rtc.days;
				break;

			case 0x74:
				data = m_vg230.rtc.days >> 8;
				break;

			case 0x79:
				/*rtc status*/
				log = FALSE;
				break;

			case 0x7a:
				data &= ~3;
				if (m_vg230.rtc.alarm_interrupt_request)
					data |= 1<<1;
				if (m_vg230.rtc.onehertz_interrupt_request)
					data |= 1<<0;
				break;

			case 0xc1:
				data &= ~1;
				if (m_vg230.pmu.write_protected)
					data |= 1;
				m_vg230.pmu.write_protected = FALSE;
				log = FALSE;
				break;
		}

		if (log)
			logerror("%.5x vg230 %02x read %.2x\n",(int) m_maincpu->pc(), m_vg230.index, data);
		//    data=machine.root_device().memregion("maincpu")->base()[0x4000+offset];
	}
	else
		data = m_vg230.index;

	return data;
}


WRITE8_MEMBER( pasogo_state::vg230_io_w )
{
	int log = TRUE;

	if (offset & 1)
	{
		//  machine.root_device().memregion("maincpu")->base()[0x4000+offset]=data;
		m_vg230.data[m_vg230.index] = data;
		switch (m_vg230.index)
		{
			case 0x09:
				break;

			case 0x70:
				m_vg230.rtc.seconds = data & 0x3f;
				break;

			case 0x71:
				m_vg230.rtc.minutes = data & 0x3f;
				break;

			case 0x72:
				m_vg230.rtc.hours = data & 0x1f;
				break;

			case 0x73:
				m_vg230.rtc.days = (m_vg230.rtc.days & ~0xff) | data;
				break;

			case 0x74:
				m_vg230.rtc.days = (m_vg230.rtc.days & 0xff) | ((data & 0xf) << 8);
				break;

			case 0x75:
				m_vg230.rtc.alarm_seconds = data & 0x3f;
				break;

			case 0x76:
				m_vg230.rtc.alarm_minutes = data & 0x3f;
				break;

			case 0x77:
				m_vg230.rtc.alarm_hours = data & 0x1f;
				break;

			case 0x78:
				m_vg230.rtc.days = data & 0x1f;
				break;

			case 0x79:
				m_vg230.rtc.onehertz_interrupt_on = data & 1;
				m_vg230.rtc.alarm_interrupt_on = data & 2;
				log = FALSE;
				break;

			case 0x7a:
				if (data & 2)
				{
					m_vg230.rtc.alarm_interrupt_request = FALSE;
					m_vg230.rtc.onehertz_interrupt_request = FALSE; /* update interrupt */
				}
				break;
		}

		if (log)
			logerror("%.5x vg230 %02x write %.2x\n", (int)m_maincpu->pc(), m_vg230.index, data);
	}
	else
		m_vg230.index = data;
}


READ8_MEMBER( pasogo_state::ems_r )
{
	UINT8 data = 0;

	switch (offset)
	{
		case 0:
			data = m_ems.data;
			break;

		case 2:
		case 3:
			data = m_ems.mapper[m_ems.index].data[offset & 1];
			break;
	}
	return data;
}


WRITE8_MEMBER( pasogo_state::ems_w )
{
	char bank[10];

	switch (offset)
	{
	case 0:
		m_ems.data = data;
		switch (data & ~3)
		{
			case 0x80: m_ems.index = 0; break;
			case 0x84: m_ems.index = 1; break;
			case 0x88: m_ems.index = 2; break;
			case 0x8c: m_ems.index = 3; break;
			case 0x90: m_ems.index = 4; break;
			case 0x94: m_ems.index = 5; break;
			case 0x98: m_ems.index = 6; break;
			case 0x9c: m_ems.index = 7; break;
			case 0xa0: m_ems.index = 8; break;
			case 0xa4: m_ems.index = 9; break;
			case 0xa8: m_ems.index = 10; break;
			case 0xac: m_ems.index = 11; break;
			case 0xb0: m_ems.index = 12; break;
			case 0xb4: m_ems.index = 13; break;
			//case 0xb8: m_ems.index = 14; break;
			//case 0xbc: m_ems.index = 15; break;
			case 0xc0: m_ems.index = 14; break;
			case 0xc4: m_ems.index = 15; break;
			case 0xc8: m_ems.index = 16; break;
			case 0xcc: m_ems.index = 17; break;
			case 0xd0: m_ems.index = 18; break;
			case 0xd4: m_ems.index = 19; break;
			case 0xd8: m_ems.index = 20; break;
			case 0xdc: m_ems.index = 21; break;
			case 0xe0: m_ems.index = 22; break;
			case 0xe4: m_ems.index = 23; break;
			case 0xe8: m_ems.index = 24; break;
			case 0xec: m_ems.index = 25; break;
		}
		break;

	case 2:
	case 3:
		m_ems.mapper[m_ems.index].data[offset & 1] = data;
		m_ems.mapper[m_ems.index].address = (m_ems.mapper[m_ems.index].data[0] << 14) | ((m_ems.mapper[m_ems.index].data[1] & 0xf) << 22);
		m_ems.mapper[m_ems.index].on = m_ems.mapper[m_ems.index].data[1] & 0x80;
		m_ems.mapper[m_ems.index].type = (m_ems.mapper[m_ems.index].data[1] & 0x70) >> 4;
		logerror("%.5x ems mapper %d(%05x)on:%d type:%d address:%07x\n", (int)m_maincpu->pc(), m_ems.index, m_ems.data << 12,
			m_ems.mapper[m_ems.index].on, m_ems.mapper[m_ems.index].type, m_ems.mapper[m_ems.index].address );

		switch (m_ems.mapper[m_ems.index].type)
		{
		case 0: /*external*/
		case 1: /*ram*/
			sprintf(bank, "bank%d", m_ems.index + 1);
			membank(bank)->set_base(m_maincpu_rom->base() + (m_ems.mapper[m_ems.index].address & 0xfffff));
			break;
		case 3: /* rom 1 */
		case 4: /* pc card a */
		case 5: /* pc card b */
		default:
			break;
		case 2:
			sprintf(bank, "bank%d", m_ems.index + 1);
			membank(bank)->set_base(m_cart_rom->base() + (m_ems.mapper[m_ems.index].address & 0xfffff));
			break;
		}
		break;
	}
}


static ADDRESS_MAP_START(pasogo_mem, AS_PROGRAM, 16, pasogo_state)
	ADDRESS_MAP_GLOBAL_MASK(0xffFFF)
	AM_RANGE(0x00000, 0x7ffff) AM_RAM
	AM_RANGE(0x80000, 0x83fff) AM_RAMBANK("bank1")
	AM_RANGE(0x84000, 0x87fff) AM_RAMBANK("bank2")
	AM_RANGE(0x88000, 0x8bfff) AM_RAMBANK("bank3")
	AM_RANGE(0x8c000, 0x8ffff) AM_RAMBANK("bank4")
	AM_RANGE(0x90000, 0x93fff) AM_RAMBANK("bank5")
	AM_RANGE(0x94000, 0x97fff) AM_RAMBANK("bank6")
	AM_RANGE(0x98000, 0x9bfff) AM_RAMBANK("bank7")
	AM_RANGE(0x9c000, 0x9ffff) AM_RAMBANK("bank8")
	AM_RANGE(0xa0000, 0xa3fff) AM_RAMBANK("bank9")
	AM_RANGE(0xa4000, 0xa7fff) AM_RAMBANK("bank10")
	AM_RANGE(0xa8000, 0xabfff) AM_RAMBANK("bank11")
	AM_RANGE(0xac000, 0xaffff) AM_RAMBANK("bank12")
	AM_RANGE(0xb0000, 0xb3fff) AM_RAMBANK("bank13")
	AM_RANGE(0xb4000, 0xb7fff) AM_RAMBANK("bank14")
//  AM_RANGE(0xb8000, 0xbffff) AM_RAM
	AM_RANGE(0xb8000, 0xbffff) AM_RAMBANK("bank28")
	AM_RANGE(0xc0000, 0xc3fff) AM_RAMBANK("bank15")
	AM_RANGE(0xc4000, 0xc7fff) AM_RAMBANK("bank16")
	AM_RANGE(0xc8000, 0xcbfff) AM_RAMBANK("bank17")
	AM_RANGE(0xcc000, 0xcffff) AM_RAMBANK("bank18")
	AM_RANGE(0xd0000, 0xd3fff) AM_RAMBANK("bank19")
	AM_RANGE(0xd4000, 0xd7fff) AM_RAMBANK("bank20")
	AM_RANGE(0xd8000, 0xdbfff) AM_RAMBANK("bank21")
	AM_RANGE(0xdc000, 0xdffff) AM_RAMBANK("bank22")
	AM_RANGE(0xe0000, 0xe3fff) AM_RAMBANK("bank23")
	AM_RANGE(0xe4000, 0xe7fff) AM_RAMBANK("bank24")
	AM_RANGE(0xe8000, 0xebfff) AM_RAMBANK("bank25")
	AM_RANGE(0xec000, 0xeffff) AM_RAMBANK("bank26")

	AM_RANGE(0xf0000, 0xfffff) AM_ROMBANK("bank27")
ADDRESS_MAP_END


static ADDRESS_MAP_START(pasogo_io, AS_IO, 16, pasogo_state)
//  ADDRESS_MAP_GLOBAL_MASK(0xfFFF)
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8("dma8237", am9517a_device, read, write, 0xffff)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE8("pic8259", pic8259_device, read, write, 0xffff)
	AM_RANGE(0x26, 0x27) AM_READWRITE8(vg230_io_r, vg230_io_w, 0xffff)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE8("pit8254", pit8254_device, read, write, 0xffff)
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE8("ppi8255", i8255_device, read, write, 0xffff)
	AM_RANGE(0x6c, 0x6f) AM_READWRITE8(ems_r, ems_w, 0xffff)
ADDRESS_MAP_END


static INPUT_PORTS_START( pasogo )
PORT_START("JOY")
//  PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SELECT)  PORT_NAME("select")
//  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START) PORT_NAME("start")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("O") /*?*/
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("X") /*?*/
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("a") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("b") PORT_CODE(KEYCODE_B)
INPUT_PORTS_END


/* palette in red, green, blue tribles */
static const unsigned char pasogo_palette[][3] =
{
	{ 0, 0, 0 },
	{ 45,45,43 },
	{ 130, 159, 166 },
	{ 255,255,255 }
};


PALETTE_INIT_MEMBER(pasogo_state, pasogo)
{
	int i;

	for ( i = 0; i < ARRAY_LENGTH(pasogo_palette); i++ )
	{
		palette.set_pen_color(i, pasogo_palette[i][0], pasogo_palette[i][1], pasogo_palette[i][2]);
	}
}


UINT32 pasogo_state::screen_update_pasogo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//static int width = -1, height = -1;
	UINT8 *rom = m_maincpu_rom->base() + 0xb8000;
	static const UINT16 c[] = { 3, 0 };
	int x,y;
//  plot_box(bitmap, 0, 0, 64/*bitmap.width*/, bitmap.height, 0);
	int w = 640;
	int h = 240;
	if (0)
	{
		w = 320;
		h = 240;
		for (y=0; y<h; y++)
		{
			for (x=0; x<w; x+=4)
			{
				int a = (y & 1) * 0x2000;
				UINT8 d = rom[a + (y & ~1) * 80/2 + x/4];
				UINT16 *line = &bitmap.pix16(y, x);
				*line++ = (d >> 6) & 3;
				*line++ = (d >> 4) & 3;
				*line++ = (d >> 2) & 3;
				*line++ = (d >> 0) & 3;
			}
		}
	}
	else
	{
		for (y=0; y<h; y++)
		{
			for (x=0; x<w; x+=8)
			{
				int a = (y & 3) * 0x2000;
				UINT8 d = rom[a + (y & ~3) * 80/4 + x/8];
				UINT16 *line = &bitmap.pix16(y, x);
				*line++ = c[(d >> 7) & 1];
				*line++ = c[(d >> 6) & 1];
				*line++ = c[(d >> 5) & 1];
				*line++ = c[(d >> 4) & 1];
				*line++ = c[(d >> 3) & 1];
				*line++ = c[(d >> 2) & 1];
				*line++ = c[(d >> 1) & 1];
				*line++ = c[(d >> 0) & 1];
			}
		}
	}
#if 0
	if (w!=width || h!=height)
	{
		width = w; height = h;
//      machine().first_screen()->set_visible_area(0, width - 1, 0, height - 1);
		screen.set_visible_area(0, width - 1, 0, height - 1);
	}
#endif
	return 0;
}

INTERRUPT_GEN_MEMBER(pasogo_state::pasogo_interrupt)
{
//  m_maincpu->set_input_line(UPD7810_INTFE1, PULSE_LINE);
}

void pasogo_state::machine_reset()
{
	astring region_tag;
	m_cart_rom = memregion(region_tag.cpy(m_cart->tag()).cat(GENERIC_ROM_REGION_TAG));
	m_maincpu_rom = memregion("maincpu");

	membank("bank27")->set_base(m_cart_rom->base());
	membank("bank28")->set_base(m_maincpu_rom->base() + 0xb8000/*?*/);

	m_u73_q2 = 0;
	m_out1 = 2; // initial state of pit output is undefined
	m_pc_spkrdata = 0;
	m_pit_out2 = 1;
	m_dma_channel = -1;
	m_cur_eop = false;
}


WRITE_LINE_MEMBER(pasogo_state::speaker_set_spkrdata)
{
	m_pc_spkrdata = state ? 1 : 0;
	m_speaker->level_w(m_pc_spkrdata & m_pit_out2);
}


WRITE_LINE_MEMBER( pasogo_state::pit8253_out1_changed )
{
	/* Trigger DMA channel #0 */
	if ( m_out1 == 0 && state == 1 && m_u73_q2 == 0 )
	{
		m_u73_q2 = 1;
		m_dma8237->dreq0_w( m_u73_q2 );
	}
	m_out1 = state;
}


WRITE_LINE_MEMBER( pasogo_state::pit8253_out2_changed )
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(m_pc_spkrdata & m_pit_out2);
}


READ8_MEMBER( pasogo_state::page_r )
{
	return 0xff;
}


WRITE8_MEMBER( pasogo_state::page_w )
{
	switch(offset % 4)
	{
		case 1:
			m_dma_offset[2] = data;
			break;
		case 2:
			m_dma_offset[3] = data;
			break;
		case 3:
			m_dma_offset[0] = m_dma_offset[1] = data;
			break;
	}
}


WRITE_LINE_MEMBER( pasogo_state::dma_hrq_changed )
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237->hack_w(state);
}

WRITE_LINE_MEMBER( pasogo_state::dma8237_out_eop )
{
	m_cur_eop = state == ASSERT_LINE;
	if(m_dma_channel != -1 && m_cur_eop)
	{
		//m_isabus->eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE );
	}
}

READ8_MEMBER( pasogo_state::dma_read_byte )
{
	if(m_dma_channel == -1)
		return 0xff;
	address_space &spaceio = m_maincpu->space(AS_PROGRAM);
	offs_t page_offset = (((offs_t) m_dma_offset[m_dma_channel]) << 16) & 0x0F0000;
	return spaceio.read_byte( page_offset + offset);
}

WRITE8_MEMBER( pasogo_state::dma_write_byte )
{
	if(m_dma_channel == -1)
		return;
	address_space &spaceio = m_maincpu->space(AS_PROGRAM);
	offs_t page_offset = (((offs_t) m_dma_offset[m_dma_channel]) << 16) & 0x0F0000;

	spaceio.write_byte( page_offset + offset, data);
}


READ8_MEMBER( pasogo_state::dma8237_1_dack_r )
{
	return 0;
	//return m_isabus->dack_r(1);
}


READ8_MEMBER( pasogo_state::dma8237_2_dack_r )
{
	return 0;
	//return m_isabus->dack_r(2);
}


READ8_MEMBER( pasogo_state::dma8237_3_dack_r )
{
	return 0;
	//return m_isabus->dack_r(3);
}


WRITE8_MEMBER( pasogo_state::dma8237_0_dack_w )
{
	m_u73_q2 = 0;
	m_dma8237->dreq0_w( m_u73_q2 );
}


WRITE8_MEMBER( pasogo_state::dma8237_1_dack_w )
{
	//m_isabus->dack_w(1,data);
}


WRITE8_MEMBER( pasogo_state::dma8237_2_dack_w )
{
	//m_isabus->dack_w(2,data);
}


WRITE8_MEMBER( pasogo_state::dma8237_3_dack_w )
{
	//m_isabus->dack_w(3,data);
}


void pasogo_state::select_dma_channel(int channel, bool state)
{
	if (!state)
	{
		m_dma_channel = channel;
		if(m_cur_eop)
		{
			//m_isabus->eop_w(channel, ASSERT_LINE );
		}
	}
	else if(m_dma_channel == channel)
	{
		m_dma_channel = -1;
		if(m_cur_eop)
		{
			//m_isabus->eop_w(channel, CLEAR_LINE );
		}
	}
}

READ8_MEMBER (pasogo_state::ppi_porta_r)
{
	int data = 0xFF;
	/* KB port A */
	if (m_ppi_keyboard_clear)
	{
		//data = ioport("DSW0")->read();
	}
	else
	{
		data = m_ppi_shift_register;
	}
	return data;
}


READ8_MEMBER ( pasogo_state::ppi_portc_r )
{
	int data=0xff;

	data&=~0x80; // no parity error
	data&=~0x40; // no error on expansion board
	/* KB port C: equipment flags */
	if (m_ppi_portc_switch_high)
	{
		/* read hi nibble of S2 */
		//data = (data & 0xf0) | ((ioport("DSW0")->read() >> 4) & 0x0f);
	}
	else
	{
		/* read lo nibble of S2 */
		//data = (data & 0xf0) | (ioport("DSW0")->read() & 0x0f);
	}

	if ( m_ppi_portb & 0x01 )
	{
		data = ( data & ~0x10 ) | ( m_pit_out2 ? 0x10 : 0x00 );
	}
	data = ( data & ~0x20 ) | ( m_pit_out2 ? 0x20 : 0x00 );

	return data;
}


WRITE8_MEMBER( pasogo_state::ppi_portb_w )
{
	/* PPI controller port B*/
	m_ppi_portb = data;
	m_ppi_portc_switch_high = data & 0x08;
	m_ppi_keyboard_clear = data & 0x80;
	m_ppi_keyb_clock = data & 0x40;
	m_pit8253->write_gate2(BIT(data, 0));
	speaker_set_spkrdata( data & 0x02 );

	m_ppi_clock_signal = ( m_ppi_keyb_clock ) ? 1 : 0;
	//m_pc_kbdc->clock_write_from_mb(m_ppi_clock_signal);

	/* If PB7 is set clear the shift register and reset the IRQ line */
	if ( m_ppi_keyboard_clear )
	{
		m_pic8259->ir1_w(0);
		m_ppi_shift_register = 0;
		m_ppi_shift_enable = 1;
	}
}


static MACHINE_CONFIG_START( pasogo, pasogo_state )

	MCFG_CPU_ADD("maincpu", V30, XTAL_32_22MHz/2)
	MCFG_CPU_PROGRAM_MAP(pasogo_mem)
	MCFG_CPU_IO_MAP( pasogo_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pasogo_state,  pasogo_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259", pic8259_device, inta_cb)

	MCFG_DEVICE_ADD("pit8254", PIT8254, 0)
	MCFG_PIT8253_CLK0(4772720/4) /* heartbeat IRQ */
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir0_w))
	MCFG_PIT8253_CLK1(4772720/4) /* dram refresh */
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(pasogo_state, pit8253_out1_changed))
	MCFG_PIT8253_CLK2(4772720/4) /* pio port c pin 4, and speaker polling enough */
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(pasogo_state, pit8253_out2_changed))

	MCFG_PIC8259_ADD("pic8259", INPUTLINE("maincpu", 0), VCC, NULL)

	MCFG_DEVICE_ADD("dma8237", AM9517A, XTAL_14_31818MHz/3)
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(pasogo_state, dma_hrq_changed))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(pasogo_state, dma8237_out_eop))
	MCFG_I8237_IN_MEMR_CB(READ8(pasogo_state, dma_read_byte))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(pasogo_state, dma_write_byte))
	MCFG_I8237_IN_IOR_1_CB(READ8(pasogo_state, dma8237_1_dack_r))
	MCFG_I8237_IN_IOR_2_CB(READ8(pasogo_state, dma8237_2_dack_r))
	MCFG_I8237_IN_IOR_3_CB(READ8(pasogo_state, dma8237_3_dack_r))
	MCFG_I8237_OUT_IOW_0_CB(WRITE8(pasogo_state, dma8237_0_dack_w))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(pasogo_state, dma8237_1_dack_w))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(pasogo_state, dma8237_2_dack_w))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(pasogo_state, dma8237_3_dack_w))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(pasogo_state, dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(pasogo_state, dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(pasogo_state, dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(pasogo_state, dack3_w))

	MCFG_DEVICE_ADD("ppi8255", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(pasogo_state, ppi_porta_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(pasogo_state, ppi_portb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(pasogo_state, ppi_portc_r))

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 400-1)
	MCFG_SCREEN_UPDATE_DRIVER(pasogo_state, screen_update_pasogo)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", ARRAY_LENGTH(pasogo_palette))
	MCFG_PALETTE_INIT_OWNER(pasogo_state, pasogo)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "pasogo_cart")
	MCFG_GENERIC_WIDTH(GENERIC_ROM16_WIDTH)
	MCFG_GENERIC_MANDATORY

	MCFG_SOFTWARE_LIST_ADD("cart_list","pasogo")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("vg230_timer", pasogo_state, vg230_timer, attotime::from_hz(1))
MACHINE_CONFIG_END


ROM_START(pasogo)
	ROM_REGION(0x100000,"maincpu", ROMREGION_ERASEFF) // 1 megabyte dram?
ROM_END


DRIVER_INIT_MEMBER(pasogo_state,pasogo)
{
	vg230_init();
	memset(&m_ems, 0, sizeof(m_ems));
}

//    YEAR   NAME    PARENT  COMPAT    MACHINE   INPUT     INIT      COMPANY  FULLNAME          FLAGS
CONS( 1996, pasogo,   0,      0,       pasogo,  pasogo, pasogo_state,    pasogo,   "KOEI", "PasoGo", GAME_NO_SOUND|GAME_NOT_WORKING)

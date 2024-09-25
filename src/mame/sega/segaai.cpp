// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Fabio Priuli
// thanks-to:Chris Covell
/*

 Sega AI driver


 Not much is known at this stage, except that the system was intended to be
 used for educational purposes in schools. Yet the audio chips seem much more
 powerful than what an educational computer requires...

 CPU : 16bit V20 @ 5MHz
 ROM : 128KB OS.with SEGA PROLOG
 RAM : 128KB
 VRAM : 64KB
 Video : V9938 Resolution 256x212
 Sound : SN76489
 Cassette Drive : 9600bps
 TV Output : RGB, Video, RF
 Keyboard : new JIS arrangement (Japanese input mapping)


TODO:
- Cassette, playback is controlled by the computer. Games with cassette
  spin up the cassette for about 2 seconds
  - The tape sometimes seeks back and forth for 5-15 seconds for the right
    voice clip, while the game and player (kids) wait
  "In these tapes, by the way, the left audio channel is the narration,
  and the right channel the data bursts right before each one."
- Keyboard?
- SEGA Prolog? How to enter?

- Sound box:
  Note from ccovell:
  "With the Sound Box attached, I connected the output line of the Sound
  Box's keyboard pins to one or more input pins, and it suddenly played an
  FM instrument and printed "piano" on the screen! From this, pressing U/D
  on the pad cycled through the various instruments, and the PL/PR buttons
  lowered and raised the volume."

===========================================================================

 Sega AI Computer quick PCB overview by Chris Covell

 Major ICs

 IC 1    D701080-5     (86/09?)  NEC V20 CPU       DIP40
 IC 2    315-5200      (86/25)   SEGA          QFP100
 IC 3    27C512-25     (86/15)   64K EPROM "E000  8/24"
 IC 4    27C512-25     (86/06)   64K EPROM "F000  7/21"
 IC 5    MPR-7689      (86/22)   SEGA "264 AA E79" (ROM) DIP28
 IC 10   V9938                   Yamaha MSX2 VDP
 IC 13   D7759C        (86/12)   NEC Speech Synthesizer   DIP40
 IC 14   MPR-7619      (86/23)   SEGA (ROM)      DIP28
 IC 15   MPR-7620      (86/23)   SEGA (ROM)      DIP28
 IC 16   SN76489AN               TI PSG         DIP16
 IC 17   D8251AFC      (86/09)   NEC Communications Interface DIP28
 IC 18   315-5201      (86/25)   SEGA (bodge wire on pins 9,10) DIP20
 IC 19   M5204A        (87?/01)  OKI
 IC 20   D8255AC-2     (86/08)   NEC Peripheral Interface DIP40

 IC 6,7,8,9,11,12   D41464C-12   NEC 32K DRAMs - 128K RAM, 64K VRAM

 Crystals, etc

 X1   20.000           "KDS 6D"
 X2   21.47727         "KDS"
 X3   640kHz           "CSB 640 P"

 Connectors

 CN1   6-pin DIN Power connector
 CN2   8-pin DIN "AUX" connector
 CN3   Video phono jack
 CN4   Audio phono jack
 CN5   35-pin Sega MyCard connector
 CN6   60-pin expansion connector A1..A30 Bottom, B1..B30 Top
 CN7   9-pin header connector to "Joy, Button, LED" unit
 CN8   13(?) pin flat flex connector to pressure pad
 CN9   9-pin header connector to tape drive motor, etc.
 CN10   13-pin header connector to tape heads
 JP2   2-wire header to SW2 button board
 PJ1   7-wire header to Keyboard / Mic connector board
 MIC   2-wire header to mic on KB/Mic board
 SW1   Reset Switch

 Power switch is on the AC Adaptor

 Joypad unit (by Mitsumi) has U/D/L/R, "PL" and "PR" buttons, and a power LED.

Power Connector Pinout (Seen from AC Adaptor plug):
   1     5        1  12V COM    5   5V COM
      6           2  12V OUT    6   5V OUT
   2     4        3   5V COM
      3           4   5V OUT

AUX Connector Pinout:
   7   6          1 +5V(?)      5 csync
  3  8  1         2 GND         6 green
   5   4          3 blue        7 Audio out
     2            4 +5V(?)      8 red

New JIS Keyboard Connector Pinout:
    1 2           1,2,3 data lines
  3 4   5         4 ??          5,8 data lines
   6 7 8          6 GND         7 +5V


*/

#include "emu.h"

#include "bus/segaai/segaai_exp.h"
#include "bus/segaai/segaai_slot.h"
#include "cpu/nec/nec.h"
#include "imagedev/cassette.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "sound/sn76496.h"
#include "sound/upd7759.h"
#include "video/v9938.h"

#include "crsshair.h"
#include "softlist.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "segaai.lh"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


namespace {

class segaai_state : public driver_device
{
public:
	segaai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_sound(*this, "sn76489a")
		, m_v9938(*this, "v9938")
		, m_upd7759(*this, "upd7759")
		, m_i8251(*this, "i8251")
		, m_i8255(*this, "i8255")
		, m_cassette(*this, "cassette")
		, m_cardslot(*this, "cardslot")
		, m_expslot(*this, "exp")
		, m_port4(*this, "PORT4")
		, m_port5(*this, "PORT5")
		, m_touch(*this, "TOUCH")
		, m_touchpadx(*this, "TOUCHPADX")
		, m_touchpady(*this, "TOUCHPADY")
		, m_vector(0)
	{ }

	void segaai(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	static constexpr u8 VECTOR_V9938 = 0xf8;
	static constexpr u8 VECTOR_I8251_SEND = 0xf9;
	static constexpr u8 VECTOR_I8251_RECEIVE = 0xfa;
	static constexpr u8 VECTOR_UPD7759 = 0xfb;

	static constexpr u8 IRQ_V9938 = 0x01;
	static constexpr u8 IRQ_UPD7759 = 0x08;

	// 8255 Port B bits
	static constexpr u8 TOUCH_PAD_PRESSED = 0x02;
	static constexpr u8 TOUCH_PAD_DATA_AVAILABLE = 0x04;

	static constexpr u8 UPD7759_MODE = 0x01;
	static constexpr u8 UPD7759_BANK = 0x02;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void update_irq_state();
	u32 get_vector() { return m_vector; }
	void vdp_interrupt(int state);
	void upd7759_drq_w(int state);
	IRQ_CALLBACK_MEMBER(irq_callback);
	u8 i8255_portb_r();
	u8 i8255_portc_r();
	void i8255_portc_w(u8 data);
	void upd7759_ctrl_w(u8 data);
	void upd7759_data_w(u8 data);
	void port1c_w(u8 data);
	void port1d_w(u8 data);
	void port1e_w(u8 data);
	u8 port1e_r();
	u8 irq_enable_r();
	void irq_enable_w(u8 data);
	void irq_select_w(u8 data);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<sn76489a_device> m_sound;
	required_device<v9938_device> m_v9938;
	required_device<upd7759_device> m_upd7759;
	required_device<i8251_device> m_i8251;
	required_device<i8255_device> m_i8255;
	required_device<cassette_image_device> m_cassette;
	required_device<segaai_card_slot_device> m_cardslot;
	required_device<segaai_exp_slot_device> m_expslot;
	required_ioport m_port4;
	required_ioport m_port5;
	required_ioport m_touch;
	required_ioport m_touchpadx;
	required_ioport m_touchpady;

	u8 m_i8255_portb;
	u8 m_upd7759_ctrl;
	u8 m_upd7759_bank_ff;
	u8 m_port_1c;
	u8 m_port_1d;
	u8 m_port_1e;
	u32 m_prev_v9938_irq;
	u32 m_prev_upd7759_irq;
	u8 m_touchpad_x;
	u8 m_touchpad_y;
	u8 m_irq_active;
	u8 m_irq_enabled;
	u32 m_vector;
};


void segaai_state::mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram();
	// 0x20000-0x3ffff - Dynamically mapped by expansion slot
	// 0x80000-0x8ffff - Dynamically mapped by expansion slot
	// 0xa0000-0xbffff - Dynamically mapped by cardslot
	map(0xc0000, 0xdffff).rom();
	map(0xe0000, 0xeffff).rom();
	map(0xf0000, 0xfffff).rom();
}


/*
Interesting combination of I/O actions from the BIOS:

EC267: B0 03                mov     al,3h
EC269: E6 17                out     17h,al
EC26B: B0 FC                mov     al,0FCh     ; 11111100
EC26D: E6 0F                out     0Fh,al
EC26F: B0 FF                mov     al,0FFh
EC271: E6 08                out     8h,al

same code at ECDBE, ED2FC
EC2D6: B0 05                mov     al,5h
EC2D8: E6 17                out     17h,al
EC2DA: B0 FA                mov     al,0FAh     ; 11111010
EC2DC: E6 0F                out     0Fh,al
EC2DE: B0 00                mov     al,0h
EC2E0: E4 08                in      al,8h

same code at ECE08, ECE1D, ED282, EDBA8, EDD78
EC319: B0 04                mov     al,4h
EC31B: E6 17                out     17h,al
EC31D: B0 FE                mov     al,0FEh     ; 11111110
EC31F: E6 0F                out     0Fh,al

ECB45: 80 FA 03             cmp     dl,3h
ECB48: 74 05                be      0ECB4Fh
ECB4A: B0 09                mov     al,9h
ECB4C: E9 02 00             br      0ECB51h
ECB4F: B0 08                mov     al,8h
ECB51: E6 17                out     17h,al

same code at ED02A, ED17E, ED1DC
ECEE5: B0 03                mov     al,3h
ECEE7: E6 17                out     17h,al
ECEE9: B0 FC                mov     al,0FCh     ; 11111100
ECEEB: E6 0F                out     0Fh,al
ECEED: B0 00                mov     al,0h
ECEEF: E6 08                out     8h,al

same code at ED0D9, ED120, EDB04, EDC8F
ECF0D: B0 02                mov     al,2h
ECF0F: E6 17                out     17h,al
ECF11: B0 FE                mov     al,0FEh     ; 11111110
ECF13: E6 0F                out     0Fh,al

*/

void segaai_state::io_map(address_map &map)
{
	map(0x00, 0x03).rw(m_v9938, FUNC(v9938_device::read), FUNC(v9938_device::write));
	map(0x04, 0x07).rw(m_i8255, FUNC(i8255_device::read), FUNC(i8255_device::write));
	// port 7, bit 7, engages tape?

	map(0x08, 0x08).rw(m_i8251, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x09, 0x09).rw(m_i8251, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));

	// 0x0a (w) - ??
	// 0a: 00 written during boot
	map(0x0b, 0x0b).w(FUNC(segaai_state::upd7759_ctrl_w));

	map(0x0c, 0x0c).w(m_sound, FUNC(sn76489a_device::write));

	// 0x0e (w) - ??
	// 0x0f (w) - ??
	// during boot:
	// 0e <- 13
	// 0f <- ff
	// 0f <- 07
	// 0e <- 07
	// 0e <- 08
	// 0f <- fe

	map(0x14, 0x14).mirror(0x01).w(FUNC(segaai_state::upd7759_data_w));

	// IRQ Enable
	map(0x16, 0x16).rw(FUNC(segaai_state::irq_enable_r), FUNC(segaai_state::irq_enable_w));
	// IRQ Enable (per IRQ source selection) Why 2 registers for IRQ enable?
	map(0x17, 0x17).w(FUNC(segaai_state::irq_select_w));

	// Touchpad
	map(0x1c, 0x1c).w(FUNC(segaai_state::port1c_w));
	map(0x1d, 0x1d).w(FUNC(segaai_state::port1d_w));
	map(0x1e, 0x1e).rw(FUNC(segaai_state::port1e_r), FUNC(segaai_state::port1e_w));

	// 0x1f (w) - ??

	// Expansion I/O
	// 0x20-0x3f - Dynamically mapped by expansion slot
}


static INPUT_PORTS_START(ai_kbd)
	PORT_START("PORT4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("PL")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("RL")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("upd7759", upd7759_device, busy_r)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED) // Microphone sensor

	PORT_START("PORT5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Grey Button")
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("TOUCH")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Press touchpad")

	PORT_START("TOUCHPADX")
	PORT_BIT(0xff, 0x80, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_MINMAX(0, 255) PORT_PLAYER(1) PORT_NAME("Touchpad X")

	PORT_START("TOUCHPADY")
	PORT_BIT(0xff, 0x80, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_MINMAX(0, 255) PORT_PLAYER(1) PORT_NAME("Touchpad Y")
INPUT_PORTS_END


void segaai_state::update_irq_state()
{
	int state = CLEAR_LINE;

	if (m_irq_active & m_irq_enabled)
	{
		state = ASSERT_LINE;
	}

	m_maincpu->set_input_line(0, state);
}


// Based on edge triggers level triggers are created
void segaai_state::vdp_interrupt(int state)
{
	if (state != CLEAR_LINE)
	{
		if (m_prev_v9938_irq == CLEAR_LINE)
		{
			m_irq_active |= IRQ_V9938;
		}
	}
	m_prev_v9938_irq = state;

	update_irq_state();
}


// Based on edge triggers level triggers are created
void segaai_state::upd7759_drq_w(int state)
{
	int upd7759_irq = state ? CLEAR_LINE : ASSERT_LINE;
	if (upd7759_irq != CLEAR_LINE)
	{
		if (m_prev_upd7759_irq == CLEAR_LINE)
		{
			m_irq_active |= IRQ_UPD7759;
		}
	}
	m_prev_upd7759_irq = upd7759_irq;

	update_irq_state();
}


IRQ_CALLBACK_MEMBER(segaai_state::irq_callback)
{
	if (m_irq_active & m_irq_enabled & IRQ_V9938)
	{
		m_vector = VECTOR_V9938;
		m_irq_active &= ~IRQ_V9938;
	}
	else if (m_irq_active & m_irq_enabled & IRQ_UPD7759)
	{
		m_vector = VECTOR_UPD7759;
		m_irq_active &= ~IRQ_UPD7759;
	}
	else
	{
		if (m_irq_active & m_irq_enabled)
		{
			fatalerror("Unknown irq triggered: $%02X active, $%02X enabled\n", m_irq_active, m_irq_enabled);
		}
		fatalerror("irq_callback called but no irq active and enabled: $%02X active, $%02X enabled\n", m_irq_active, m_irq_enabled);
	}

	update_irq_state();
	return m_vector;
}


/*
Mainboard 8255 port B

 76543210
 +-------- Tape input (right channel?), unknown if input is signal level or bit
  +------- Tape head engaged
   +------ Tape insertion sensor (0 - tape is inserted, 1 - no tape inserted)
    +----- Tape write enable sensor
     +---- keyboard connector pin 3
      +--- 0 = Touch pad data available
       +-- 0 = Touch pad pressed
        +- Trigger button near touch panel (active low)
*/
u8 segaai_state::i8255_portb_r()
{
	m_i8255_portb = (m_i8255_portb & 0xf8) | (m_port5->read() & 0x01);

	if (BIT(m_port_1d, 0))
	{
		if (!BIT(m_touch->read(), 0))
		{
			m_i8255_portb |= TOUCH_PAD_PRESSED;
		}

		m_i8255_portb |= TOUCH_PAD_DATA_AVAILABLE;
	}
	else
	{
		m_i8255_portb |= TOUCH_PAD_PRESSED;
	}

	if (m_cassette->get_image() != nullptr)
	{
		m_i8255_portb &= ~0x20;
	}
	else
	{
		m_i8255_portb |= 0x20;
	}

	// when checking whether the tape is running Popoland wants to see bit7 set and bit5 reset
	// toggling this stops eigogam2 from booting normally into a game.
	// For tape reading eigogam2 routines at A11EA and A120C
	// There is a whistle tone on the cassette before normal speech starts, the code there likely
	// checks for this whistle tone.
	//m_i8255_portb ^= 0x80;

	return m_i8255_portb;
}


/*
Mainboard 8255 port C

 76543210
 +-------- keyboard connector pin 5
  +------- keyboard connector pin 8
   +------ keyboard connector pin 2
    +----- keyboard connector pin 1
     +---- Output
      +--- Output
       +-- Output
        +- Output
*/
u8 segaai_state::i8255_portc_r()
{
	u8 data = 0xf0;

	return data;
}


/*
 pin 10-13 - PC0-PC3 -> RA41
 PC0-PC3 continue on underside of pcb
 pin 14-17 - PC4-PC7 -> RA42
 PC3 continues on underside of pcb
 PC2 - pin 6 upa2003c, drives upa2003c pin 11
 PC1 - pin 4 upa2003c, drives upa2003c pin 13
 PC0 - pin 2 upd2003c, drives upa2003c pin 15
 these go to 3 dots to the left of resistors below upa2003c?
 which end up on the 9 pin flat connector on the lower left side of the pcb
 PC4 - to pin 1 of 7 pin connector bottom right side of pcb -> jis keyboard connector ?
 CN9 - 9 pin connector goes to cassette interface
 pin 1 - red
 pin 2 - black
 pin 3 - blue
 pin 4 - yellow
 pin 5 - white
 pin 6 - black/grey
 pin 7 - blue
 pin 8 - grey
 pin 9 - brown
*/
void segaai_state::i8255_portc_w(u8 data)
{
	// Writes to bits 6,5,4, unknown what they mean
	// Bit 0 written by cosmictr
	LOG("i8255 port c write: %02x\n", data);
}


void segaai_state::upd7759_data_w(u8 data)
{
	m_upd7759->start_w(ASSERT_LINE);
	m_upd7759->port_w(data);
	m_upd7759->start_w(CLEAR_LINE);
}


void segaai_state::upd7759_ctrl_w(u8 data)
{
	LOG("I/O Port $0b write: $%02x\n", data);

	u8 prev_upd7759_ctrl = m_upd7759_ctrl;
	m_upd7759_ctrl = data;

	// bit0 is connected to /md line of the uPD7759
	m_upd7759->md_w((m_upd7759_ctrl & UPD7759_MODE) ? 0 : 1);

	if (BIT(prev_upd7759_ctrl, 0))
	{
		if (!BIT(m_upd7759_ctrl, 0))
		{
			m_upd7759_bank_ff = 0;
			m_upd7759->set_rom_bank(m_upd7759_bank_ff);
		}
	}
	else
	{
		m_upd7759_bank_ff ^= 0x01;
		m_upd7759->set_rom_bank(m_upd7759_bank_ff);
	}
}


// I/O Port 16 - IRQ Enable
u8 segaai_state::irq_enable_r()
{
	return m_irq_enabled;
}


// IRQ Enable
// 76543210
// +-------- ???
//  +------- ???
//   +------ ???
//    +----- ???
//     +---- D7759 IRQ enable
//      +--- ???
//       +-- ??? 8251 receive?
//        +- V9938 IRQ enable
void segaai_state::irq_enable_w(u8 data)
{
	m_irq_enabled = data;
	m_irq_active &= data;
	update_irq_state();
}

// I/O Port 17 - IRQ Enable selection
// This port seems to be used to set or reset specific bits in the IRQ enable register.
// Why 2 ways of setting/clearing irq enable bits?
void segaai_state::irq_select_w(u8 data)
{
	int pin = (data >> 1) & 0x07;
	if (BIT(data, 0))
	{
		m_irq_enabled |= (1 << pin);
	}
	else
	{
		m_irq_enabled &= ~(1 << pin);
		m_irq_active &= m_irq_enabled;
	}
	update_irq_state();
}


void segaai_state::port1c_w(u8 data)
{
	m_port_1c = data;
}


void segaai_state::port1d_w(u8 data)
{
	m_port_1d = data;
}


void segaai_state::port1e_w(u8 data)
{
	m_port_1e = data;
}


u8 segaai_state::port1e_r()
{
	if (BIT(m_port_1c, 0))
	{
		return m_touchpady->read();
	}
	else
	{
		return m_touchpadx->read();
	}
}


void segaai_state::machine_start()
{
	m_i8255_portb = 0x7f;
	m_upd7759_ctrl = 0;
	m_upd7759_bank_ff = 0;
	m_port_1c = 0;
	m_port_1d = 0;
	m_port_1e = 0;
	m_prev_v9938_irq = CLEAR_LINE;
	m_prev_upd7759_irq = CLEAR_LINE;
	m_touchpad_x = 0;
	m_touchpad_y = 0;
	m_vector = 0;
	m_irq_enabled = 0;
	m_irq_active = 0;

	save_item(NAME(m_i8255_portb));
	save_item(NAME(m_upd7759_ctrl));
	save_item(NAME(m_upd7759_bank_ff));
	save_item(NAME(m_port_1c));
	save_item(NAME(m_port_1d));
	save_item(NAME(m_port_1e));
	save_item(NAME(m_prev_v9938_irq));
	save_item(NAME(m_prev_upd7759_irq));
	save_item(NAME(m_touchpad_x));
	save_item(NAME(m_touchpad_y));
	save_item(NAME(m_irq_active));
	save_item(NAME(m_irq_enabled));
	save_item(NAME(m_vector));

	machine().crosshair().get_crosshair(0).set_screen(CROSSHAIR_SCREEN_NONE);
}


void segaai_state::segaai(machine_config &config)
{
	V20(config, m_maincpu, 20_MHz_XTAL/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &segaai_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &segaai_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(segaai_state::irq_callback));

	V9938(config, m_v9938, 21.477272_MHz_XTAL);
	m_v9938->set_screen_ntsc(m_screen);
	m_v9938->set_vram_size(0x10000);
	m_v9938->int_cb().set(FUNC(segaai_state::vdp_interrupt));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);

	I8255(config, m_i8255);
	m_i8255->in_pa_callback().set_ioport(m_port4);
	m_i8255->in_pb_callback().set(FUNC(segaai_state::i8255_portb_r));
	m_i8255->in_pc_callback().set(FUNC(segaai_state::i8255_portc_r));
	m_i8255->out_pc_callback().set(FUNC(segaai_state::i8255_portc_w));

	I8251(config, m_i8251, 0);

	SPEAKER(config, "mono").front_center();

	SN76489A(config, m_sound, 21.477272_MHz_XTAL/6); // not verified, but sounds close to real hw recordings
	m_sound->add_route(ALL_OUTPUTS, "mono", 1.00);

	UPD7759(config, m_upd7759);
	m_upd7759->add_route(ALL_OUTPUTS, "mono", 0.70);
	m_upd7759->drq().set(FUNC(segaai_state::upd7759_drq_w));

	// Card slot
	SEGAAI_CARD_SLOT(config, m_cardslot, segaai_cards, nullptr);
	m_cardslot->set_address_space(m_maincpu, AS_PROGRAM);
	SOFTWARE_LIST(config, "software").set_original("segaai");

	// Expansion slot
	SEGAAI_EXP_SLOT(config, m_expslot, 21'477'272/6, segaai_exp, nullptr);  // not verified, assuming 3.58MHz
	m_expslot->set_mem_space(m_maincpu, AS_PROGRAM);
	m_expslot->set_io_space(m_maincpu, AS_IO);

	// Built-in cassette
	CASSETTE(config, m_cassette).set_stereo();
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->set_interface("cass");
	m_cassette->add_route(0, "mono", 0.70); // Channel 0 contains regular recorded sound
	m_cassette->set_channel(1); // Channel 1 contains data

	config.set_default_layout(layout_segaai);
}


ROM_START(segaai)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("mpr-7689.ic5",  0xc0000, 0x20000, CRC(62402ac9) SHA1(bf52d22b119d54410dad4949b0687bb0edf3e143))
	ROM_LOAD("e000 8_24.ic3", 0xe0000, 0x10000, CRC(c8b6a539) SHA1(cbf8473d1e3d8037ea98e9ca8b9aafdc8d16ff23))   // actual label is "e000 8/24"
	ROM_LOAD("f000 7_21.ic4", 0xf0000, 0x10000, CRC(64d6cd8c) SHA1(68c130048f16d6a0abe1978e84440931470222d9))   // actual label is "f000 7/21"

	ROM_REGION(0x40000, "upd7759", 0)
	ROM_LOAD("mpr-7619.ic14", 0x00000, 0x20000, CRC(d1aea002) SHA1(c8d5408bba65b17301f19cf9ebd2b635d642525a))
	ROM_LOAD("mpr-7620.ic15", 0x20000, 0x20000, CRC(e042754b) SHA1(02aede7a3e2fda9cbca621b530afa4520cf16610))
ROM_END

} // anonymous namespace

COMP(1986, segaai,     0,         0,      segaai,   ai_kbd, segaai_state,   empty_init,    "Sega",   "AI", MACHINE_NOT_WORKING)

// license:BSD-3-Clause
// copyright-holders:David Haywood, Alice Shelton
/*
    LeapFrog - Leapster

    educational system from 2003, software is all developed in MXFlash

    hwspecs


    CPU:
      Custom ASIC (ARCTangent 5.1 CPU @ 96MHz)

    Memory:
      Leapster: 2MB onboard RAM, 256 bytes NVRAM.
      Leapster2: 16MB RAM, 128kbytes NVRAM

    Media type:
      Cartridges of 4-16MB with between 2 and 512kb NVRAM

    Graphics:
      4Mb ATI chip.

    Audio:
      Custom

    Screen:
      160x160 CSTN with touchscreen.


    The Leapster 2 also has
        USB 1.1 (client only) + full-sized SD slot.


    many magic numbers in the BIOS ROM match the [strings:VALID_FLAGS] table in
    https://github.com/tsbiberdorf/MqxSrc/blob/master/tools/tad/mqx.tad
    does this mean the System is running on the MQX RTOS?
    https://www.synopsys.com/dw/ipdir.php?ds=os_mqx_software
    indicates it was available for ARC processors

*/

/* Cartridge pinout - for games list see hash/leapster.xml

CARTRIDGE-PINOUT:
-----------------
Look at the Cartridge-Slot:

                   B        A
                  ------------
                  VCC |01| VCC
                   NC |02| VSS
                  D11 |03| D04
                  D03 |04| D12
                  D10 |05| D05
                  VSS |06| D02
                  D13 |07| D09
                   NC |08| NC
                  D06 |09| D01
                  D14 |10| D08
                  ----|--|----
                  D07 |11| VSS
                  D00 |12| D15
                 Byte |13| OE
                   NC |14| A22
                   NC |15| A23
                   CE |16| A16
                  A00 |17| A15
                  A01 |18| A14
                  A02 |19| A13
                  VSS |20| A03
                  A12 |21| A04
                  A11 |22| A05
                  A10 |23| A06
                  A09 |24| A07
                  A08 |25| A17
                  A19 |26| A18
                  A20 |27| A21
                   WE |28| VSS
PIN7 of 24LC02B <---| |29| |---> PIN7 of 24LC02B
                   NC |30| |---> PIN6 of 24LC02B
                  ----------




PCB - Handheld-Console:

               +-----------------------------+
               |                             |
  +------------|                             |------------+
  |            | C A R T R I D G E - S L O T |            |
  |            |                             |            |
  |            +-----------------------------+            |
  |                                                       |
  |ASY 310-10069    +-------------------+                 |
  |                 |                   |                 |
  |                 |                   |                 |
  |LEAPSTER MAIN    |                   |                 |
  |Leap Frog        |                 U3|                 |
  |(c) 2004         +-------------------+     +-----+     |
  |                                           |ISSI |     |
  |                                           |     |     |
  |                      +---------+          |IS42S|     |
  |                      |         |          |16100|     |
   \       +-+           | EPOXY   |          |AT-7T|    /
    \      |A|           |   BLOCK |          |     |   /
     \     +-+           |         |          |   U2|  /
      \                  |       U1|          +-----+ /
       \                 +---------+                 /
        \                                           /
         \                                         /
          \                                       /
           \                                     /
            \                                   /
             \                                 /
              \                               /
               \                             /
                \                           /
                 \-------------------------/


A = 24LC02B / SN0429


ETCHES ON THE BACK OF THE PCB:

"FAB-300-10069-C"

"702800254.01A"
"SW1208 Rev.5"



PCB - Cartridge:
FRONT:

+-------------------------------------+
| LEAPSTER ROM CARTRIDGE              |
|   +--+     +---------+    Leap Frog |
|   |B1|U2   |         |     (c) 2003 |
+-+ +--+     |E P O X Y|            +-+
  |          |       U3|            |
+-+          +---------+            +-+
|                       20232-003-1020|
| ASY 310-10028             REV:00    |
+-+                    +-+          +-+
  |||||||||||||||||||||| ||||||||||||
  +--------------------+ +----------+
 A30                                A01

B1: 24L002B

24L002B:

             +-----+
  (GND)<- A0-|     |-VCC
  (GND)<- A1-|     |-WP
  (GND)<- A2-|     |-SCL
         VSS-|     |-SDA
             +-----+



PCB - LEAPSTER-TV:

+-----------------------------------------------------------------------+
|                                                                       |
|                                                                       |
|           20300+003+0015                                              |
 \           REV:06                                                    /
  |                                                                   |
  |                                                                   |
  |                 +----+     +-------+                              |
   \                |    |     | EPOXY |                             /
    |   +----+      |    |     |       |                            |
    |   |EPOX|      |    |     |       |                            |
    |   |Y   |      |    |     |     U1|    +-+                     |
     \  +----+      |    |     +-------+    | |                    /
      |             |  U2|                  +-+U4                 |
      |             +----+                                        |
      |                     +--------------+   ROADRUNNER CONSOLE |
       \                    | AM29PL160CB  |   Leap Frog (c) 2006/
        |                   | -90SF        |   Asy 310-10378    |
        |                   |            U6|                    |
        |                   +--------------+                    |
         \                                                     /
          |          +-----------------------------+          |
          |          | C A R T R I D G E - S L O T |          |
          |          +-----------------------------+          |
           \                                                 /
            +-----------------------------------------------+

*/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/arcompact/arcompact.h"
#include "leapster_a.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class leapster_state : public driver_device
{
public:
	leapster_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot"),
		m_palette(*this, "palette"),
		m_sound(*this, "leapster_snd"),
		m_buttons(*this, "BUTTONS"),
		m_touch(*this, {"TOUCHX", "TOUCHY", "TOUCH"})
		{ }

	void leapster(machine_config &config);

	void init_leapster();

	DECLARE_INPUT_CHANGED_MEMBER(leapster_touch_down);

private:
	void fire_adc_interrupt()
	{
		m_int_fired_flags |= 0x100;
		m_maincpu->set_input_line(0x10, ASSERT_LINE);
	}

	void adc_fifo_push(int channel, int data)
	{
		if (m_adc_fifo_base == m_adc_fifo_head && !m_adc_fifo_empty)
		{
			fatalerror("ADC FIFO full!\n");
		}

		m_adc_fifo[m_adc_fifo_head] = (channel << 16) | (data << 5);
		m_adc_fifo_head += 1;
		m_adc_fifo_head %= sizeof(m_adc_fifo) / sizeof(*m_adc_fifo);
		m_adc_fifo_empty = false;
	}

	uint32_t adc_fifo_pop()
	{
		if(m_adc_fifo_empty)
		{
			return 0;
		}

		uint32_t ret = m_adc_fifo[m_adc_fifo_base];
		m_adc_fifo_base += 1;
		m_adc_fifo_base %= sizeof(m_adc_fifo) / sizeof(*m_adc_fifo);;
		m_adc_fifo_empty = m_adc_fifo_head == m_adc_fifo_base;

		return ret;
	}

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update_leapster(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	TIMER_CALLBACK_MEMBER(leapster_timer_overflow);
	TIMER_CALLBACK_MEMBER(leapster_touch_adc_update);
	uint32_t leapster_1801000_r();
	uint32_t leapster_1801004_r();
	uint32_t leapster_1801008_r();
	uint32_t leapster_180100c_r();
	uint32_t leapster_1801018_r();
	uint32_t leapster_1809004_r();
	uint32_t leapster_180b000_r();
	uint32_t leapster_180b004_r();
	uint32_t leapster_180b008_r();
	uint32_t leapster_180d514_r();

	uint32_t leapster_180004c_r();

	void leapster_1802070_w(uint32_t data);
	uint32_t leapster_1802078_r();

	uint32_t leapster_eeprom_r(uint32_t offset);
	void leapster_eeprom_w(uint32_t offset, uint32_t data);

	uint32_t leapster_cpu_clock_r();
	void leapster_cpu_clock_w(uint32_t data);

	uint32_t leapster_int_flag_r();
	void leapster_int_flag_w(uint32_t data);

	uint32_t leapster_adc_r(uint32_t offset);
	void leapster_adc_w(uint32_t offset, uint32_t data);
	uint32_t leapster_lcd_r(uint32_t offset);
	void leapster_lcd_w(uint32_t offset, uint32_t data);
	uint32_t leapster_dma_r(uint32_t offset);
	void leapster_dma_w(uint32_t offset, uint32_t data);
	uint32_t leapster_timer_r(uint32_t offset);
	void leapster_timer_w(uint32_t offset, uint32_t data);

	void leapster_aux0047_w(uint32_t data);
	uint32_t leapster_aux0048_r();
	void leapster_aux0048_w(uint32_t data);
	void leapster_aux004b_w(uint32_t data);

	void leapster_aux0010_w(uint32_t data);
	uint32_t leapster_aux0011_r();
	void leapster_aux0011_w(uint32_t data);
	void leapster_aux001a_w(uint32_t data);
	uint32_t leapster_aux001b_r();

	void leapster_aux(address_map &map) ATTR_COLD;
	void leapster_map(address_map &map) ATTR_COLD;

	uint16_t m_1a_data[0x800];
	int m_1a_pointer;

	uint32_t m_timer_ticks[3]{};
	uint32_t m_timer_control[3]{};
	uint32_t m_timer_max[3]{};
	emu_timer *m_overflow_timer[3]{};
	static constexpr int TIMER_IRQS[3] = {0x19, 0x1a, 0x1e};

	uint16_t m_framebuffer_base;
	uint32_t m_display_format;
	uint32_t m_display_stride;

	uint32_t m_dma_src_addr;
	uint32_t m_dma_scanline_count;
	uint32_t m_dma_start_offset;
	uint32_t m_dma_stride; // Counted in words

	uint32_t m_clock_div;

	emu_timer *m_adc_timer{};

	uint32_t m_adc_channel_control[4]{};

	uint32_t m_adc_fifo[0x10000]{}; // Actual size unknown
	uint32_t m_adc_fifo_base;
	uint32_t m_adc_fifo_head;
	bool m_adc_fifo_empty;

	bool m_touchscreen_initted;

	uint32_t m_int_fired_flags;

	uint32_t m_current_eeprom_command;

	uint8_t m_system_eeprom[512];
	uint8_t m_cartridge_eeprom[2048];

	required_device<arcompact_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_device<palette_device> m_palette;
	required_device<leapster_snd_device> m_sound;

	required_ioport m_buttons;
	required_ioport_array<3> m_touch;

	memory_region *m_cart_rom = nullptr;
	uint32_t m_cart_bit = 0x0400'0000;
};


static INPUT_PORTS_START( leapster )
	PORT_START("BUTTONS")
	PORT_BIT(0x0000'0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_NAME("Right")
	PORT_BIT(0x0000'0100, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_NAME("Down")
	PORT_BIT(0x0000'0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_NAME("Left")
	PORT_BIT(0x0000'2000, IP_ACTIVE_LOW, IPT_BUTTON3)        PORT_NAME("B")
	PORT_BIT(0x0000'4000, IP_ACTIVE_LOW, IPT_BUTTON2)        PORT_NAME("A")
	PORT_BIT(0x0000'8000, IP_ACTIVE_LOW, IPT_VOLUME_DOWN)    PORT_NAME("Volume Down")
	PORT_BIT(0x0001'0000, IP_ACTIVE_LOW, IPT_VOLUME_UP)      PORT_NAME("Volume Up")
	PORT_BIT(0x0400'0000, IP_ACTIVE_LOW, IPT_SELECT)         PORT_NAME("Pause")
	PORT_BIT(0x1000'0000, IP_ACTIVE_LOW, IPT_BUTTON4)        PORT_NAME("Hint")
	PORT_BIT(0x2000'0000, IP_ACTIVE_LOW, IPT_BUTTON5)        PORT_NAME("Home")
	PORT_BIT(0x8000'0000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_NAME("Up")

	// Also used to dump Flash debug info in software
	PORT_BIT(0x0002'0000, IP_ACTIVE_LOW, IPT_UNKNOWN)        PORT_NAME("Brightness Down")
	PORT_BIT(0x0100'0000, IP_ACTIVE_LOW, IPT_UNKNOWN)        PORT_NAME("Brightness Up")
	PORT_BIT(0x0200'0000, IP_ACTIVE_LOW, IPT_UNKNOWN)        PORT_NAME("Contrast")

	PORT_START("TOUCHX")
	PORT_BIT(0x7ff, 0x3df, IPT_LIGHTGUN_X) PORT_MINMAX(0x00, 0x7f0) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(leapster_state::leapster_touch_down), 1)

	PORT_START("TOUCHY")
	PORT_BIT(0x7ff, 0x3df, IPT_LIGHTGUN_Y) PORT_MINMAX(0x00, 0x7f0) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(leapster_state::leapster_touch_down), 1)

	PORT_START("TOUCH")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(leapster_state::leapster_touch_down), 0) PORT_NAME("Touchscreen Touch")
INPUT_PORTS_END

void leapster_state::leapster_aux0010_w(uint32_t data)
{
}

void leapster_state::leapster_aux0011_w(uint32_t data)
{
	// unknown, written with 1a
}

void leapster_state::leapster_aux001a_w(uint32_t data)
{
	// probably not palette, but it does load 0x1000 words of increasing value on startup, so could be?
	m_1a_data[m_1a_pointer & 0x7ff] = data;

	uint8_t r = (data >> 12) & 0x7;
	uint8_t g = (data >> 8) & 0xf;
	uint8_t b = (data >> 4) & 0xf;

	m_palette->set_pen_color(m_1a_pointer & 0x7ff, rgb_t(pal3bit(r), pal4bit(g), pal4bit(b)));
	m_1a_pointer++;
}

uint32_t leapster_state::leapster_aux0011_r()
{
	// unknown, read when 11/1a are being written
	logerror("%s: leapster_aux0011_r\n", machine().describe_context());
	return 0x00000000;
}

uint32_t leapster_state::leapster_aux001b_r()
{
	// unknown, read when 11/1a are being written
	logerror("%s: leapster_aux001b_r\n", machine().describe_context());
	return 0x00000000;
}

void leapster_state::leapster_aux0047_w(uint32_t data)
{
	logerror("%s: leapster_aux0047_w %08x\n", machine().describe_context(), data);
}

uint32_t leapster_state::leapster_aux0048_r()
{
	logerror("%s: leapster_aux0048_r\n", machine().describe_context());
	return 0x00000000;
}

void leapster_state::leapster_aux0048_w(uint32_t data)
{
	logerror("%s: leapster_aux0047_w %08x\n", machine().describe_context(), data);
}

void leapster_state::leapster_aux004b_w(uint32_t data)
{
	logerror("%s: leapster_aux004b_w %08x\n", machine().describe_context(), data);
}

uint32_t leapster_state::leapster_1801000_r()
{
	logerror("%s: leapster_1801000_r\n", machine().describe_context());
	return 0x00000000;
}

uint32_t leapster_state::leapster_1801004_r()
{
	logerror("%s: leapster_1801004_r\n", machine().describe_context());
	return 0x00000000;
}

uint32_t leapster_state::leapster_1801008_r()
{
	logerror("%s: leapster_1801004_r\n", machine().describe_context());
	return 0x00000000;
}

uint32_t leapster_state::leapster_180100c_r()
{
	logerror("%s: leapster_180100c_r\n", machine().describe_context());
	return 0x00000000;
}

uint32_t leapster_state::leapster_1801018_r()
{
	logerror("%s: leapster_1801018_r\n", machine().describe_context());
	return 0x00000000;
}

// Bits: UUUU UCSS SLDU UUUU FTUU UUUU UUUP UUUQ
// C: 0 if a cartridge is present, 1 otherwise
// S: Identifies the LCD the leapster was manufactured with? On the original Leapster, 4, 6, and 7 are valid possibilites.
// L: Controls logging level?
// D: If not set, there are many places where execution infinite loops on error rather than panic. Controls debug logging level
// F: If not set, Flash will run at 10x the SWF framerate (effectively uncapping the framerate)
// T: If not set, the system will boot to the touch calibration
// P: Checked for PEG engine games
// Q: Also checked for PEG engine games, but its effect is OR'ed with P
// U: Unknown
//   The Flash uncapping bit at least is known to be toggleable by shorting one of the cartridge pins
// Value taken from testing with a Leapster 2 with a retail cartridge
uint32_t leapster_state::leapster_1809004_r()
{
	logerror("%s: leapster_1809004_r (return usually checked against 0x00200000)\n", machine().describe_context());
	return 0x63FF'BFFF | m_cart_bit;
}

uint32_t leapster_state::leapster_eeprom_r(uint32_t offset)
{
	if(offset == 2) {
		// This is really hacky and needs a better defined solution to identify the target bank
		uint8_t *eepromBank = BIT(m_current_eeprom_command, 8, 8) == 0x26 ? m_system_eeprom : m_cartridge_eeprom;
		return eepromBank[m_current_eeprom_command >> 16];
	}

	return 0;
}

void leapster_state::leapster_eeprom_w(uint32_t offset, uint32_t data)
{
	switch(offset)
	{
		case 0:
			m_current_eeprom_command = data;
			break;
		case 1:
			if(data == 2)
			{
				uint8_t *eepromBank = BIT(m_current_eeprom_command, 8, 8) == 0x26 ? m_system_eeprom : m_cartridge_eeprom;
				eepromBank[m_current_eeprom_command >> 16] = m_current_eeprom_command & 0xff;
			}

			break;
	}
}

uint32_t leapster_state::leapster_cpu_clock_r()
{
	return m_clock_div;
}

// Lower 3 bits are always masked for after read, but upper 29 bits never set?
void leapster_state::leapster_cpu_clock_w(uint32_t data)
{
	m_clock_div = data;
	// Commented out for performance reasons while testing for my sanity
//  m_maincpu->set_unscaled_clock_int(96000000 / (1 << (data & 0x07)));
}

uint32_t leapster_state::leapster_180b000_r()
{
	logerror("%s: leapster_180b000_r\n", machine().describe_context());
	return 0x00000000;
}

uint32_t leapster_state::leapster_180b004_r()
{
	// leapster2 BIOS checks if this is 0
	logerror("%s: leapster_180b004_r\n", machine().describe_context());
	return 0xffffffff;
}

uint32_t leapster_state::leapster_180b008_r()
{
	// checks bit 1 (using BMSK instruction and AND instruction)
	// writes back to same address?
	logerror("%s: leapster_180b008_r\n", machine().describe_context());
	return 0x00000001;
}

// UART Status register (0x0180'd510) bits:
// 0x80 (R): 1 indicates ready for output
// 0x40 (W): 1 causes the current byte in the transfer register to be sent
// 0x20 (R): 0 indicates transfer register contains a received byte
// 0x04 (W): 1 sets IRQ line clear?
// 0x01..0x02 (R): Unknown, used when recieving bytes
// Interrupts are done on IRQ 0x1b when a byte is received or when the other end is ready for another transfer
uint32_t leapster_state::leapster_180d514_r()
{
	logerror("%s: leapster_180d514_r (return usually checked against 0x0030d400)\n", machine().describe_context());
	// leapster -bios 0 does a BRNE in a loop comparing with 0x80
	return 0x000000A0;
}

// Official name: GIODataIn
// The GIO bus seems to be composed as a bitfield combining:
//  1.) Power / docking info
//  2.) Button input state
//  3.) (On Leapster 2): Internal Flash data in
uint32_t leapster_state::leapster_180004c_r()
{
	// Set all non-button bits to 1 to avoid detecting a shutdown for now
	return (0xffff'ffff & ~(0xb703'e380)) | m_buttons->read();
}

// Seems to be used to commit a change to a hardware voice
// The lower 3 bits are the voice index and the upper 29 are a command
// After this data is received, the audio unit acks it by raising IRQ 0x1d
// The audio driver will wait for this before making any more voice changes
void leapster_state::leapster_1802070_w(uint32_t data)
{
	m_maincpu->set_input_line(0x1d, ASSERT_LINE);

	m_sound->do_voice_command(data >> 3, BIT(data, 0, 3));
}

// Returns a bitfield indicating which audio voices are currently active
//   In a somewhat weird order, see leapster_snd_device::get_triggered_voices
uint32_t leapster_state::leapster_1802078_r()
{
	return m_sound->get_triggered_voices();
}

// Seems to signal fired interrupts for cases where one line is used by 2 sources
// ADC in particular uses this
// EEPROM IO expects bit 7 to be set as a ready bit before doing anything
uint32_t leapster_state::leapster_int_flag_r()
{
	return m_int_fired_flags | 0x80;
}

// Writing a 1 to a flag bit seems to ACK and set it to 0
void leapster_state::leapster_int_flag_w(uint32_t data)
{
	m_int_fired_flags &= ~data;
}

// ADC: I/O registers 0x0180'0090 - 0x0180'00ab, Drives IRQ vector 0x10
// 0x0180'0090 - 0x0180'009f: ADC channel controls
// 0x0180'00a0: ADC in: UUUU UUUU UUUU UUCC DDDD DDDD DDDU UUUU
//   C: Channel index
//   D: Packet data
//   U: Unknown
// 0x0180'00a8: Seems to be a status register. Bit 8 being 0 indicates that there is waiting input data
//   Reading from 0x0180'00a0 (or maybe 0x0180'00a8 itself?) is taken as an ACK and can set bit 8

// ADC input is almost certainly a FIFO, but I'm not sure what the depth or exact behaviour is
//   and for MAME's purposes, only the touchscreen is emulated anyway

// The ADC takes in 4 channels as input
// Channel 0: Touchscreen
// Channel 1: Unused?
// Channel 2: Something to do with the LCD
// Channel 3: Power / Battery level info?

// After the touchscreen ADC channel is activated, the BIOS waits for it to send 17 packets.
// The actual data of the first 8 of these packets is completely ignored.
// The next 8 are summed and stored, but never used after that.
// The last one is ignored

uint32_t leapster_state::leapster_adc_r(uint32_t offset)
{
	switch (offset)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
			return m_adc_channel_control[offset];
		case 0x04:
			return adc_fifo_pop();
		case 0x05: // Unk
			break;
		case 0x06:
			return m_adc_fifo_empty << 8;
	}

	logerror("%s: Unknown ADC I/O read! Addr: %08x\n", machine().describe_context(), 0x0180'0090 + (offset * 4));
	return 0;
}

void leapster_state::leapster_adc_w(uint32_t offset, uint32_t data)
{
	switch (offset)
	{
		// Channel 0 (Touchscreen) control
		case 0x00:
			// Channel enable bit
			if (data & 0x8000)
			{
				m_adc_timer->enable();

//              // Begin init process
				if (!m_touchscreen_initted)
				{
					fire_adc_interrupt();

					// Whatever these are, they're never used
					for (int i = 0; i < 17; i++)
					{
						adc_fifo_push(0, 0);
					}

					m_touchscreen_initted = true;

					m_adc_timer->reset(attotime::from_hz(60));
				}
			}
			else
			{
				m_adc_timer->enable(false);
			}
			[[fallthrough]];
		case 0x01:
		case 0x02:
		case 0x03:
			m_adc_channel_control[offset] = data;
	}

	logerror("%s: Unknown ADC I/O write! Addr: %08x, data: %08x\n", machine().describe_context(), 0x0180'0090 + (offset * 4), data);
}

uint32_t leapster_state::leapster_lcd_r(uint32_t offset)
{
	switch (offset)
	{
		case 0: // Written once, never read
			break;
		case 1: // Framebuffer base
			return m_framebuffer_base;
		case 2: // Stride
			return m_display_stride;
		case 3: // Display format
			return m_display_format;
		case 4: // Unknown
			break;
		case 5: // Never written
			break;
		case 6: // Written once, never read
			break;
	}

	logerror("%s: Unknown LCD I/O read! Addr: %08x\n", machine().describe_context(), 0x0180'8084 + (offset * 4));

	return 0;
}

void leapster_state::leapster_lcd_w(uint32_t offset, uint32_t data)
{
	switch (offset)
	{
		case 1:
			m_framebuffer_base = data;
			return;
		case 2: // Stride
			m_display_stride = data;
			return;
		case 3: // Display format
			if (BIT(data, 31) && BIT(data, 0, 30) != 4)
			{
//              fatalerror("Unimplemented display mode\n");
			}

			m_display_format = data;
			return;
	}

	logerror("%s: Unknown LCD I/O write! Addr: %08x, data: %08x\n", machine().describe_context(), 0x0180'8084 + (offset * 4), data);
}

uint32_t leapster_state::leapster_dma_r(uint32_t offset)
{
	return 0;
}

void leapster_state::leapster_dma_w(uint32_t offset, uint32_t data)
{
	switch (offset)
	{
		case 0: {
			if (data == 0x1b)
			{
				uint32_t dmaDst = 0x0300'0000 + m_framebuffer_base + m_dma_start_offset;
				uint32_t dmaSrc = m_dma_src_addr;

				for (int i = 0; i < m_dma_scanline_count; i++)
				{
					for (int j = 0; j < m_dma_stride; j++)
					{
						m_maincpu->space().write_dword(dmaDst, m_maincpu->space().read_dword(dmaSrc));
						dmaDst += 4;
						dmaSrc += 4;
					}
				}

				// IRQ 0x12 when DMA is finished
				m_maincpu->set_input_line(0x12, ASSERT_LINE);
			}

			return;
		}
		case 1:
			m_dma_src_addr = data;
			return;
		case 2:
			m_dma_stride = data;
			return;
		case 3:
			m_dma_scanline_count = data;
			return;
		case 4:
			m_dma_start_offset = data;
			return;
	}
}

uint32_t leapster_state::leapster_timer_r(uint32_t offset)
{
	offset *= 4;

	int index = BIT(offset, 10, 2);

	// Normalize timer 0 address to be in line with timers 1 and 2
	if (index == 0)
	{
		offset -= 0x84;
	}

	switch (BIT(offset, 2, 2))
	{
		case 0: // Tick count
			return m_timer_ticks[index] + m_overflow_timer[index]->elapsed().as_ticks(16'000'000);
		case 1: // Control register
			return m_timer_control[index];
		case 2: // Max ticks
			return m_timer_max[index];
	}

	return 0;
}

void leapster_state::leapster_timer_w(uint32_t offset, uint32_t data)
{
	offset *= 4;

	if (offset == 0x0510)
	{
		printf("%c", data);
		return;
	}

	if (offset == 0x0514)
	{
		// MQX blocks when the output queue fills, so the UART IRQ must have at least bare-bones emulation for transfer
		if (data == 0x44)
		{
			m_maincpu->set_input_line(0x1b, ASSERT_LINE);
		}

		return;
	}

	if (offset & 0x0300)
	{
		return;
	}

	int index = BIT(offset, 10, 2);

	// Normalize timer 0 address to be in line with timers 1 and 2
	if (index == 0)
	{
		offset -= 0x84;
	}

	switch (BIT(offset, 2, 2))
	{
		case 0: { // Tick count
			m_timer_ticks[index] = data;
			uint32_t ticks_until_overflow = data >= m_timer_max[index] ? 0 : m_timer_max[index] - data;
			m_overflow_timer[index]->reset(attotime::from_ticks(ticks_until_overflow, 16'000'000));
			break;
		}
		case 1: // Control register
			m_timer_control[index] = data;
			break;
		case 2: { // Max ticks
			m_timer_max[index] = data;
			m_timer_ticks[index] += m_overflow_timer[index]->elapsed().as_ticks(16'000'000);
			uint32_t ticks_until_overflow = m_timer_ticks[index] >= data ? 0 : data - m_timer_ticks[index];
			m_overflow_timer[index]->reset(attotime::from_ticks(ticks_until_overflow, 16'000'000));
			break;
		}
	}
}

uint32_t leapster_state::screen_update_leapster(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!BIT(m_display_format, 31) || (BIT(m_display_format, 0, 30) != 4 && BIT(m_display_format, 0, 30) != 3))
	{
		return 0;
	}

	if (BIT(m_display_format, 0, 30) == 4)
	{
		for (int i = 0; i < 160; i++)
		{
			for (int j = 0; j < 80; j++)
			{
				uint8_t bytes[3];

				bytes[0] = m_maincpu->space().read_byte(0x0300'0000 + m_framebuffer_base + i * m_display_stride + j * 3);
				bytes[1] = m_maincpu->space().read_byte(0x0300'0000 + m_framebuffer_base + i * m_display_stride + j * 3 + 1);
				bytes[2] = m_maincpu->space().read_byte(0x0300'0000 + m_framebuffer_base + i * m_display_stride + j * 3 + 2);

				bitmap.pix(i, j * 2) = pal444(bytes[0] | ((bytes[1] & 0xF0) << 4), 8, 4, 0);
				bitmap.pix(i, j * 2 + 1) = pal444((bytes[2] << 4) | (bytes[1] & 0x0F), 0, 8, 4);
			}
		}
	}
	else if (BIT(m_display_format, 0, 30) == 3) // Used in touch cal
	{
		for (int i = 0; i < 160; i++)
		{
			for (int j = 0; j < 160; j++)
			{
				uint8_t byte;

				byte = m_maincpu->space().read_byte(0x0300'0000 + m_framebuffer_base + i * m_display_stride + j);

				bitmap.pix(i, j) = pal332(byte, 5, 2, 0);
			}
		}
	}

	return 0;
}

DEVICE_IMAGE_LOAD_MEMBER( leapster_state::cart_load )
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM32_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

void leapster_state::machine_start()
{
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	if (m_cart_rom)
	{
		m_maincpu->space(AS_PROGRAM).install_rom(0x8000'0000, 0x8000'0000 + m_cart_rom->bytes() - 1, m_cart_rom->base());
		m_cart_bit = 0;
	}

	for (uint32_t i = 0; i < 3; i++)
	{
		m_overflow_timer[i] = timer_alloc(FUNC(leapster_state::leapster_timer_overflow), this);
		m_overflow_timer[i]->set_param(i);
	}

	m_adc_timer = timer_alloc(FUNC(leapster_state::leapster_touch_adc_update), this);

	memset(m_system_eeprom, 0, sizeof(m_system_eeprom));
	memset(m_cartridge_eeprom, 0, sizeof(m_system_eeprom));

	save_item(NAME(m_1a_data));

	m_sound->set_address_space(&m_maincpu->space());
}

void leapster_state::machine_reset()
{
	m_1a_pointer = 0;
	for (int i = 0; i < 0x800; i++)
		m_1a_data[i] = 0;

	for (int i = 0; i < 3; i++)
	{
		m_timer_ticks[i] = 0;
		m_timer_control[i] = 0;
		m_timer_max[i] = 0xffffffff;
		m_overflow_timer[i]->reset(attotime::from_ticks(m_timer_max[i], 16'000'000));
	}

	m_display_format = 0;
	m_display_stride = 0;
	m_framebuffer_base = 0;

	m_dma_src_addr = 0;
	m_dma_scanline_count = 0;
	m_dma_start_offset = 0;
	m_dma_stride = 0;

	for (int i = 0; i < 4; i++)
	{
		m_adc_channel_control[i] = 0;
	}

	m_adc_fifo_base = 0;
	m_adc_fifo_head = 0;
	m_adc_fifo_empty = true;

	m_touchscreen_initted = false;
}

void leapster_state::leapster_map(address_map &map)
{
//  A vector table is copied from 0x00000000 to 0x3c000000, but it is unclear if that is a BIOS mirror
//  or if it should be copying a different table.
	map(0x0000'0000, 0x007f'ffff).mirror(0x40000000).rom().region("maincpu", 0);
//  map(0x4000'0000, 0x407f'ffff).rom().region("maincpu", 0);

	map(0x0180'0030, 0x0180'003f).rw(FUNC(leapster_state::leapster_eeprom_r), FUNC(leapster_state::leapster_eeprom_w));

	map(0x0180'0080, 0x0180'0083).rw(FUNC(leapster_state::leapster_int_flag_r), FUNC(leapster_state::leapster_int_flag_w));

	map(0x0180'0090, 0x0180'00ab).rw(FUNC(leapster_state::leapster_adc_r), FUNC(leapster_state::leapster_adc_w));

	map(0x01801000, 0x01801003).r(FUNC(leapster_state::leapster_1801000_r));
	map(0x01801004, 0x01801007).r(FUNC(leapster_state::leapster_1801004_r));
	map(0x01801008, 0x0180100b).r(FUNC(leapster_state::leapster_1801008_r));
	map(0x0180100c, 0x0180100f).r(FUNC(leapster_state::leapster_180100c_r));
	map(0x01801018, 0x0180101b).r(FUNC(leapster_state::leapster_1801018_r));

	map(0x0180'004c, 0x0180'004f).r(FUNC(leapster_state::leapster_180004c_r));

	map(0x0180'2070, 0x0180'2073).w(FUNC(leapster_state::leapster_1802070_w));
	map(0x0180'2078, 0x0180'207b).r(FUNC(leapster_state::leapster_1802078_r));

	map(0x0180'4000, 0x0180'4fff).m(m_sound, FUNC(leapster_snd_device::map));

	map(0x0180'8084, 0x0180'809b).rw(FUNC(leapster_state::leapster_lcd_r), FUNC(leapster_state::leapster_lcd_w));

	map(0x0180'8800, 0x0180'881b).rw(FUNC(leapster_state::leapster_dma_r), FUNC(leapster_state::leapster_dma_w));

	map(0x01809004, 0x01809007).r(FUNC(leapster_state::leapster_1809004_r));
	map(0x01809008, 0x0180900b).rw(FUNC(leapster_state::leapster_cpu_clock_r), FUNC(leapster_state::leapster_cpu_clock_w));

	map(0x0180b000, 0x0180b003).r(FUNC(leapster_state::leapster_180b000_r));
	map(0x0180b004, 0x0180b007).r(FUNC(leapster_state::leapster_180b004_r));
	map(0x0180b008, 0x0180b00b).r(FUNC(leapster_state::leapster_180b008_r));

	map(0x0180'd000, 0x0180'd8ff).rw(FUNC(leapster_state::leapster_timer_r), FUNC(leapster_state::leapster_timer_w));

	map(0x0180d514, 0x0180d517).r(FUNC(leapster_state::leapster_180d514_r));

	// VRAM, also used for stack prior to MQX boot
	map(0x0300'0000, 0x0300'ffff).ram();

	// The original Leapster BIOS can write past the end of VRAM in its clear screen function (caused by asl by 2 at 0x4007'bc32)
	// This seems to be a bug (pointer arithmetic done on the wrong size?) and doesn't appear in later BIOS
	map(0x0301'0000, 0x0302'ffff).nopw();

	map(0x3c00'0000, 0x3c1f'ffff).ram(); // Main memory
	map(0x3c200000, 0x3fffffff).ram();
	// map(0x80000000, 0x807fffff).bankr("cartrom"); // game ROM pointers are all to the 80xxxxxx region, so I assume it maps here - installed if a cart is present
}


void leapster_state::leapster_aux(address_map &map)
{
	// addresses used here aren't known internal ARC addresses, so are presumed to be Leapster specific
	map(0x000000010, 0x000000010).w(FUNC(leapster_state::leapster_aux0010_w));
	map(0x000000011, 0x000000011).rw(FUNC(leapster_state::leapster_aux0011_r), FUNC(leapster_state::leapster_aux0011_w));
	map(0x00000001a, 0x00000001a).w(FUNC(leapster_state::leapster_aux001a_w));
	map(0x00000001b, 0x00000001b).r(FUNC(leapster_state::leapster_aux001b_r));

	map(0x000000047, 0x000000047).w(FUNC(leapster_state::leapster_aux0047_w));
	map(0x000000048, 0x000000048).rw(FUNC(leapster_state::leapster_aux0048_r), FUNC(leapster_state::leapster_aux0048_w));
	map(0x00000004b, 0x00000004b).w(FUNC(leapster_state::leapster_aux004b_w));
}

TIMER_CALLBACK_MEMBER(leapster_state::leapster_timer_overflow)
{
	m_timer_ticks[param] = 0;
	m_overflow_timer[param]->reset(attotime::from_ticks(m_timer_max[param], 16'000'000));

	if (BIT(m_timer_control[param], 0))
	{
		m_maincpu->set_input_line(TIMER_IRQS[param], ASSERT_LINE);
	}
}

uint32_t releasedCounter = 30;

TIMER_CALLBACK_MEMBER(leapster_state::leapster_touch_adc_update)
{
	uint16_t pressure;
	uint16_t adc_x;
	uint16_t adc_y;

	adc_x = m_touch[0]->read();
	adc_y = m_touch[1]->read();

	if(!m_touch[2]->read()) {
		pressure = 0x7FF;
	}
	else
	{
		releasedCounter = 0;
		pressure = 0;
	}

	// Takes 2 groups of 6 samples and discards any inputs where
	//   The two groups differ significantly.
	// Not exactly sure what's up with samples 0, 1, 4, and 5, but if their average is
	//   greater than 0x7F0 it'll get interpreted as there not being a touch, so I'm assuming
	//   they're pressure or something?

	adc_fifo_push(0, pressure);
	adc_fifo_push(0, pressure);
	adc_fifo_push(0, adc_x);
	adc_fifo_push(0, adc_y);
	adc_fifo_push(0, pressure);
	adc_fifo_push(0, pressure);

	adc_fifo_push(0, pressure);
	adc_fifo_push(0, pressure);
	adc_fifo_push(0, adc_x);
	adc_fifo_push(0, adc_y);
	adc_fifo_push(0, pressure);
	adc_fifo_push(0, pressure);

	fire_adc_interrupt();

	// The leapster touch driver will wait 3 or 4 accepted inputs before confirming a touchscreen
	//   press and 6 or 7 before confirming a release. I'm not sure what the intended frequency is,
	//   so I've just set it at 60.

	m_adc_timer->reset(attotime::from_hz(60));
}

INPUT_CHANGED_MEMBER(leapster_state::leapster_touch_down)
{
	if (m_touchscreen_initted && (m_adc_channel_control[0] & 0x8000))
	{
//      uint16_t adc_x;
//      uint16_t adc_y;
//
//      // Movement without a touch down
//      if (param && !m_touch[2]->read())
//      {
//          return;
//      }
//
//      // Touch release
//      if (!param && !newval)
//      {
//          adc_x = 0x7ff;
//          adc_y = 0x7ff;
//      }
//      else
//      {
//          adc_x = m_touch[0]->read();
//          adc_y = m_touch[0]->read();
//      }
//
//      printf("Touch at %02X, %02X\n", m_touch[0]->read(), m_touch[1]->read());
//
//      adc_fifo_push(0, adc_x);
//      adc_fifo_push(0, adc_y);
//      adc_fifo_push(0, 0); // Unk
//      adc_fifo_push(0, 0); // Unk
//      adc_fifo_push(0, adc_x);
//      adc_fifo_push(0, adc_y);
//
//      adc_fifo_push(0, adc_x);
//      adc_fifo_push(0, adc_y);
//      adc_fifo_push(0, 0); // Unk
//      adc_fifo_push(0, 0); // Unk
//      adc_fifo_push(0, adc_x);
//      adc_fifo_push(0, adc_y);
//
//      fire_adc_interrupt();
	}
}

void leapster_state::leapster(machine_config &config)
{
	// Basic machine hardware
	// CPU is ArcTangent-A5 '5.1' (ARCompact core)
	ARCA5(config, m_maincpu, 96000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &leapster_state::leapster_map);
	m_maincpu->set_addrmap(AS_IO, &leapster_state::leapster_aux);
	m_maincpu->set_default_vector_base(0x40000000);

	// Video hardware
	screen_device &screen(SCREEN(config, "screen").set_lcd());
	screen.set_refresh_hz(60);
	screen.set_size(160, 160);
	screen.set_visarea(0, 160-1, 0, 160-1);
	screen.set_screen_update(FUNC(leapster_state::screen_update_leapster));

	SPEAKER(config, "mono").front_center();
	LEAPSTER_SOUND(config, m_sound, 96000000);
	m_sound->add_route(ALL_OUTPUTS, "mono", 1.00);

	PALETTE(config, "palette").set_format(palette_device::xRGB_444, 0x800).set_endianness(ENDIANNESS_BIG);

	// Cartridge
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "leapster_cart", "bin").set_device_load(FUNC(leapster_state::cart_load));

	// Software lists
	SOFTWARE_LIST(config, "cart_list").set_original("leapster");
}

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios))

/* There are various build dates and revisions for different parts of the code, the date listed is the newest on in each ROM.
   This is always in the same place relative to the rest of the data.

   V2.1 sets (except TV) are apparently larger because "Learning with Leap" was built in.
*/

ROM_START(leapster)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS( 0, "uni15", "Universal v1.5" ) // 152-10346 Leapster BaseROM Universal v1.5 - Sep 04 2003 10:46:47
	ROM_LOAD_BIOS( 0, "155-10072-a.bin"   , 0x00000, 0x200000, CRC(af05e5a0) SHA1(d4468d060543ba7e44785041093bc98bcd9afa07) )
	ROM_SYSTEM_BIOS( 1, "us21",  "USA v2.1" )       // 152-11265 Leapster BaseROM US v2.1        - Apr 13 2005 15:34:57
	ROM_LOAD_BIOS( 1, "152-11265_2.1.bin",  0x00000, 0x800000, CRC(9639b3ae) SHA1(002873b782e823c7a8159deed16c78c149f2afab) )
	ROM_SYSTEM_BIOS( 2, "uk21",  "UK v2.1" )        // 152-11452 Leapster BaseROM UK v2.1        - Aug 30 2005 16:01:46
	ROM_LOAD_BIOS( 2, "leapster2_1004.bin", 0x00000, 0x800000, CRC(b466e14d) SHA1(910c234f03e76b7de55b8aa0a0c62fd1daae4910) )
	ROM_SYSTEM_BIOS( 3, "ger21", "German v2.1" )    // 152-11435 Leapster BaseROM German v2.1    - Oct 21 2005 18:53:59
	ROM_LOAD_BIOS( 3, "leapster2_1006.bin", 0x00000, 0x800000, BAD_DUMP CRC(a69ed8ca) SHA1(e6aacba0c39b1465f344c2b07ff1cbd8a395adac) ) // BADADDR xxx-xxxxxxxxxxxxxxxxxxx
	ROM_SYSTEM_BIOS( 4, "sp10",  "Spanish v1.0" )   // 152-11546 Leapster Baserom SP v1.0        - Apr 03 2006 06:26:00
	ROM_LOAD_BIOS( 4, "leapster2_1008.bin", 0x00000, 0x800000, CRC(b43345e7) SHA1(31c27e79568115bf36e5ef668f528e3005054152) )
	ROM_DEFAULT_BIOS( "uni15" )
ROM_END

ROM_START(leapstertv)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS( 0, "uni2111", "Universal v2.1.11" ) // 152-11594 LeapsterTv Baserom Universal.v2.1.11 - Apr 13 2006 16:36:08
	ROM_LOAD_BIOS( 0, "am29pl160cb-90sf.bin", 0x00000, 0x200000, CRC(194cc724) SHA1(000a79d75c19f2e43532ce0b31f0dca0bed49eab) )
	ROM_DEFAULT_BIOS( "uni2111" )
ROM_END

ROM_START(leapster2)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS( 0, "2xcip3_m9", "2x CIP3 m9" ) // 152-12659 Leapster 2x CIP3 Baserom m9 - Mar 29 2011 14:13:45
	ROM_LOAD_BIOS( 0, "152-12659_m9.bin",  0x00000, 0x800000, CRC(57bde604) SHA1(4b5eaac1e40bc605eb4cf6d4ad212343334762fd) )
	ROM_SYSTEM_BIOS( 1, "2x_06",     "2x 0.6" )     // 152-12206 Leapster 2x Baserom 0.6     - Feb 02 2009 17:15:38
	ROM_LOAD_BIOS( 1, "152-12206_0.6.bin", 0x00000, 0x800000, CRC(fa94d9a7) SHA1(c5bd84146701dc4a7635b0e37adedb90747adf32) )
	ROM_SYSTEM_BIOS( 2, "connb5",  "Connected B5" )  // 152-12076 Leapster Connected Baserom B5   - Feb 29 2008 18:11:21
	ROM_LOAD_BIOS( 2, "152-12076_b5.bin",   0x00000, 0x800000, CRC(4d223022) SHA1(bdc10ad70aa7641716e16fbea16bd0ef35f6e85e) )
	ROM_DEFAULT_BIOS( "2xcip3_m9" )
ROM_END

ROM_START(leapsterlmx)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS( 0, "lmax_2_2",    "v2.2" )     // 152-11476 LMAX Baserom v2.2    - Jan 12 2006 11:22:50
	ROM_LOAD_BIOS( 0, "152-11476_v2.2.bin",    0x00000, 0x800000, CRC(e1140475) SHA1(42089165db67005b6a0180e894ff8f36b97a081e) )
	ROM_SYSTEM_BIOS( 1, "lmax_us_2_1", "USA v2.1" ) // 152-11238 LMAX BaseROM US v2.1 - Mar 04 2005 12:01:01
	ROM_LOAD_BIOS( 1, "152-11238_us_v2.1.bin", 0x00000, 0x800000, CRC(80bb4e58) SHA1(7d8b1c23d08ce76a89cff1112957377c6a1d4b63) )
	ROM_DEFAULT_BIOS( "lmax_2_2" )
ROM_END

void leapster_state::init_leapster()
{
}

} // anonymous namespace


CONS( 2003, leapster,    0,        0, leapster, leapster, leapster_state, init_leapster, "LeapFrog", "Leapster",       MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
CONS( 2005, leapstertv,  leapster, 0, leapster, leapster, leapster_state, init_leapster, "LeapFrog", "Leapster TV",    MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2005, leapsterlmx, leapster, 0, leapster, leapster, leapster_state, init_leapster, "LeapFrog", "Leapster L-MAX", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2009, leapster2,   leapster, 0, leapster, leapster, leapster_state, init_leapster, "LeapFrog", "Leapster 2",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

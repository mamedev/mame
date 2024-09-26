// license:GPL-2.0+
// copyright-holders:Peter Trauner
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
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "machine/bankdev.h"
#include "machine/genpc.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"

/*
  rtc interrupt irq 2
 */


namespace {

class pasogo_state : public driver_device
{
public:
	pasogo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
		, m_ems(*this, "ems")
		, m_vram(*this, "vram")
		, m_palette(*this, "palette")
	{ }

	void pasogo(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(contrast);

private:
	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_device<address_map_bank_device> m_ems;
	required_shared_ptr<uint16_t> m_vram;
	required_device<palette_device> m_palette;

	uint16_t ems_r(offs_t offset, uint16_t mem_mask);
	void ems_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t emsram_r(offs_t offset, uint16_t mem_mask);
	void emsram_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t vg230_io_r(offs_t offset);
	void vg230_io_w(offs_t offset, uint8_t data);

	struct
	{
		uint8_t index = 0;
		uint8_t data[0x100]{};
		struct {
			uint16_t data = 0;
		} bios_timer; // 1.19 MHz tclk signal
		struct {
			int seconds = 0, minutes = 0, hours = 0, days = 0;
			int alarm_seconds = 0, alarm_minutes = 0, alarm_hours = 0, alarm_days = 0;

			int onehertz_interrupt_on = 0;
			int onehertz_interrupt_request = 0;
			int alarm_interrupt_on = 0;
			int alarm_interrupt_request = 0;
		} rtc;
		struct {
			int write_protected = 0;
		} pmu;
	} m_vg230;

	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;

	uint32_t screen_update_pasogo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(pasogo_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(vg230_timer);

	memory_region *m_cart_rom = nullptr;
	uint8_t m_ems_index = 0;
	uint16_t m_ems_bank[28]{};
	void emsbank_map(address_map &map) ATTR_COLD;
	void pasogo_io(address_map &map) ATTR_COLD;
	void pasogo_mem(address_map &map) ATTR_COLD;
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

void pasogo_state::machine_start()
{
	system_time systime;

	m_vg230 = decltype(m_vg230)();
	m_vg230.pmu.write_protected = true;
	machine().base_datetime(systime);

	m_vg230.rtc.seconds = systime.local_time.second;
	m_vg230.rtc.minutes = systime.local_time.minute;
	m_vg230.rtc.hours = systime.local_time.hour;
	m_vg230.rtc.days = 0;

	m_vg230.bios_timer.data=0x7200; // HACK
}

uint8_t pasogo_state::vg230_io_r(offs_t offset)
{
	int log = true;
	uint8_t data = 0;

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
				log = false;
				break;

			case 0x70:
				data = m_vg230.rtc.seconds;
				log = false;
				break;

			case 0x71:
				data = m_vg230.rtc.minutes;
				log = false;
				break;

			case 0x72:
				data = m_vg230.rtc.hours;
				log = false;
				break;

			case 0x73:
				data = m_vg230.rtc.days;
				break;

			case 0x74:
				data = m_vg230.rtc.days >> 8;
				break;

			case 0x79:
				/*rtc status*/
				log = false;
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
				m_vg230.pmu.write_protected = false;
				log = false;
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


void pasogo_state::vg230_io_w(offs_t offset, uint8_t data)
{
	int log = true;

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
				log = false;
				break;

			case 0x7a:
				if (data & 2)
				{
					m_vg230.rtc.alarm_interrupt_request = false;
					m_vg230.rtc.onehertz_interrupt_request = false; /* update interrupt */
				}
				break;
		}

		if (log)
			logerror("%.5x vg230 %02x write %.2x\n", (int)m_maincpu->pc(), m_vg230.index, data);
	}
	else
		m_vg230.index = data;
}


uint16_t pasogo_state::ems_r(offs_t offset, uint16_t mem_mask)
{
	uint8_t data = 0;
	uint8_t index;

	switch (offset)
	{
		case 0:
			data = m_ems_index;
			break;

		case 1:
			index = (m_ems_index >> 2) & 0x1f;
			data = m_ems_bank[index];
			break;
	}
	return data;
}


void pasogo_state::ems_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint8_t index;

	switch (offset)
	{
	case 0:
		m_ems_index = data;
		break;

	case 1:
		index = (m_ems_index >> 2) & 0x1f;
		if((index & ~1) == 10)
		{
			logerror("EMS mapping of CGA framebuffer\n");
			break;
		}
		else if(index >= 28)
		{
			logerror("EMS index out of range\n");
			break;
		}
		COMBINE_DATA(&m_ems_bank[index]);
		break;
	}
}

uint16_t pasogo_state::emsram_r(offs_t offset, uint16_t mem_mask)
{
	m_ems->set_bank(m_ems_bank[(offset >> 13) & 0x1f] & 0x7fff);
	return m_ems->read16(offset & 0x1fff, mem_mask);
}

void pasogo_state::emsram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_ems->set_bank(m_ems_bank[(offset >> 13) & 0x1f] & 0x7fff);
	m_ems->write16(offset & 0x1fff, data, mem_mask);
}

void pasogo_state::emsbank_map(address_map &map)
{
	map(0x04080000, 0x040fffff).ram();
	map(0x08000000, 0x080fffff).bankr("bank27");
	map(0x10000000, 0x1000ffff).ram(); // cart ram?
}

void pasogo_state::pasogo_mem(address_map &map)
{
	map(0x80000, 0xeffff).rw(FUNC(pasogo_state::emsram_r), FUNC(pasogo_state::emsram_w));
	map(0xb8000, 0xbffff).ram().share("vram");
	map(0xf0000, 0xfffff).bankr("bank27");
}


void pasogo_state::pasogo_io(address_map &map)
{
	map(0x0000, 0x00ff).m("mb", FUNC(ibm5160_mb_device::map));
	map(0x0026, 0x0027).rw(FUNC(pasogo_state::vg230_io_r), FUNC(pasogo_state::vg230_io_w));
	map(0x006c, 0x006f).rw(FUNC(pasogo_state::ems_r), FUNC(pasogo_state::ems_w));
}


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
	PORT_START("COLOR")
	PORT_CONFNAME(0x01, 0x01, "Contrast") PORT_CHANGED_MEMBER(DEVICE_SELF, pasogo_state, contrast, 0)
	PORT_CONFSETTING(0x00, "Actual")
	PORT_CONFSETTING(0x01, "Enhanced")
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(pasogo_state::contrast)
{
	if(newval)
	{
		m_palette->set_pen_color(0, rgb_t(80, 130, 130));
		m_palette->set_pen_color(1, rgb_t(40, 60, 140));
	}
	else
	{
		m_palette->set_pen_color(0, rgb_t(100, 110, 100));
		m_palette->set_pen_color(1, rgb_t(90, 80, 110));
	}
}

uint32_t pasogo_state::screen_update_pasogo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const vram = (uint8_t *)m_vram.target();
	for (int y=0; y<240; y++)
	{
		for (int x=0; x<(320/8); x++)
		{
			int const a = (y & 3) * 0x2000;
			uint8_t const d1 = vram[a + (y >> 2) * 80 + x];
			uint16_t *line = &bitmap.pix(y, x << 3);
			*line++ = BIT(d1, 7);
			*line++ = BIT(d1, 6);
			*line++ = BIT(d1, 5);
			*line++ = BIT(d1, 4);
			*line++ = BIT(d1, 3);
			*line++ = BIT(d1, 2);
			*line++ = BIT(d1, 1);
			*line++ = BIT(d1, 0);
		}
	}
	return 0;
}

INTERRUPT_GEN_MEMBER(pasogo_state::pasogo_interrupt)
{
//  m_maincpu->pulse_input_line(UPD7810_INTFE1, attotime::zero);
}

void pasogo_state::machine_reset()
{
	std::string region_tag;
	ioport_port *color = ioport("COLOR");
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	if (!m_cart_rom) // even with mandatory carts, debug will crash without anything at the boot vector
		m_cart_rom = memregion("empty");

	membank("bank27")->set_base(m_cart_rom->base());
	m_ems_index = 0;
	memset(m_ems_bank, 0, sizeof(m_ems_bank));
	contrast(*color->fields().first(), 0, 0, color->read());
}

void pasogo_state::pasogo(machine_config &config)
{
	V30(config, m_maincpu, XTAL(32'220'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &pasogo_state::pasogo_mem);
	m_maincpu->set_addrmap(AS_IO, &pasogo_state::pasogo_io);
	m_maincpu->set_vblank_int("screen", FUNC(pasogo_state::pasogo_interrupt));
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259", FUNC(pic8259_device::inta_cb));

	ADDRESS_MAP_BANK(config, "ems").set_map(&pasogo_state::emsbank_map).set_options(ENDIANNESS_LITTLE, 16, 32, 0x4000);

	ibm5160_mb_device &mb(IBM5160_MOTHERBOARD(config, "mb", 0));
	mb.set_cputag(m_maincpu);
	mb.int_callback().set_inputline(m_maincpu, 0);
	mb.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	RAM(config, RAM_TAG).set_default_size("512K");

	// It's a CGA device right so lets use isa_cga!  Well, not so much.
	// The carts use vg230 specific registers and mostly ignore the mc6845.
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_size(320, 240);
	screen.set_visarea(0, 320-1, 0, 240-1);
	screen.set_screen_update(FUNC(pasogo_state::screen_update_pasogo));
	screen.set_palette(m_palette);
	PALETTE(config, m_palette).set_entries(2);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "pasogo_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("pasogo");

	TIMER(config, "vg230_timer").configure_periodic(FUNC(pasogo_state::vg230_timer), attotime::from_hz(1));
}

ROM_START( pasogo )
	ROM_REGION( 0x10000, "empty", ROMREGION_ERASEFF )
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME  FLAGS
CONS( 1996, pasogo, 0,      0,      pasogo,  pasogo, pasogo_state, empty_init, "KOEI",  "PasoGo", MACHINE_NO_SOUND|MACHINE_NOT_WORKING)

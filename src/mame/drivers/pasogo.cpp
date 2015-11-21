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
#include "includes/genpc.h"
#include "softlist.h"

/*
  rtc interrupt irq 2
 */


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

	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_device<address_map_bank_device> m_ems;
	required_shared_ptr<UINT16> m_vram;
	required_device<palette_device> m_palette;

	DECLARE_READ16_MEMBER(ems_r);
	DECLARE_WRITE16_MEMBER(ems_w);
	DECLARE_READ16_MEMBER(emsram_r);
	DECLARE_WRITE16_MEMBER(emsram_w);
	DECLARE_READ8_MEMBER(vg230_io_r);
	DECLARE_WRITE8_MEMBER(vg230_io_w);

	struct
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
	} m_vg230;

	void machine_reset();
	void machine_start();

	UINT32 screen_update_pasogo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(pasogo_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(vg230_timer);
	DECLARE_INPUT_CHANGED_MEMBER(contrast);

	memory_region *m_cart_rom;
	UINT8 m_ems_index;
	UINT16 m_ems_bank[28];
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

	memset(&m_vg230, 0, sizeof(m_vg230));
	m_vg230.pmu.write_protected = TRUE;
	machine().base_datetime(systime);

	m_vg230.rtc.seconds = systime.local_time.second;
	m_vg230.rtc.minutes = systime.local_time.minute;
	m_vg230.rtc.hours = systime.local_time.hour;
	m_vg230.rtc.days = 0;

	m_vg230.bios_timer.data=0x7200; // HACK
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


READ16_MEMBER( pasogo_state::ems_r )
{
	UINT8 data = 0;
	UINT8 index;

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


WRITE16_MEMBER( pasogo_state::ems_w )
{
	UINT8 index;

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

READ16_MEMBER( pasogo_state::emsram_r )
{
	m_ems->set_bank(m_ems_bank[(offset >> 13) & 0x1f] & 0x7fff);
	return m_ems->read16(space, offset & 0x1fff, mem_mask);
}

WRITE16_MEMBER( pasogo_state::emsram_w )
{
	m_ems->set_bank(m_ems_bank[(offset >> 13) & 0x1f] & 0x7fff);
	m_ems->write16(space, offset & 0x1fff, data, mem_mask);
}

static ADDRESS_MAP_START(emsbank_map, AS_PROGRAM, 16, pasogo_state)
	AM_RANGE(0x04080000, 0x040fffff) AM_RAM
	AM_RANGE(0x08000000, 0x080fffff) AM_ROMBANK("bank27")
	AM_RANGE(0x10000000, 0x1000ffff) AM_RAM // cart ram?
ADDRESS_MAP_END

static ADDRESS_MAP_START(pasogo_mem, AS_PROGRAM, 16, pasogo_state)
	AM_RANGE(0x00000, 0x7ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xb8000, 0xbffff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0x80000, 0xeffff) AM_READWRITE(emsram_r, emsram_w)
	AM_RANGE(0xf0000, 0xfffff) AM_ROMBANK("bank27")
ADDRESS_MAP_END


static ADDRESS_MAP_START(pasogo_io, AS_IO, 16, pasogo_state)
	AM_RANGE(0x0026, 0x0027) AM_READWRITE8(vg230_io_r, vg230_io_w, 0xffff)
	AM_RANGE(0x006c, 0x006f) AM_READWRITE(ems_r, ems_w)
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

UINT32 pasogo_state::screen_update_pasogo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *vram = (UINT8 *)m_vram.target();
	int x, y;
	for (y=0; y<240; y++)
	{
		for (x=0; x<(320/8); x++)
		{
			int a = (y & 3) * 0x2000;
			UINT8 d1 = vram[a + (y >> 2) * 80 + x];
			UINT16 *line = &bitmap.pix16(y, x << 3);
			*line++ = ((d1 >> 7) & 1);
			*line++ = ((d1 >> 6) & 1);
			*line++ = ((d1 >> 5) & 1);
			*line++ = ((d1 >> 4) & 1);
			*line++ = ((d1 >> 3) & 1);
			*line++ = ((d1 >> 2) & 1);
			*line++ = ((d1 >> 1) & 1);
			*line++ = ((d1 >> 0) & 1);
		}
	}
	return 0;
}

INTERRUPT_GEN_MEMBER(pasogo_state::pasogo_interrupt)
{
//  m_maincpu->set_input_line(UPD7810_INTFE1, PULSE_LINE);
}

void pasogo_state::machine_reset()
{
	std::string region_tag;
	ioport_port *color = ioport("COLOR");
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	if (!m_cart_rom)    // this should never happen, since we make carts mandatory!
		m_cart_rom = memregion("maincpu");

	membank("bank27")->set_base(m_cart_rom->base());
	m_ems_index = 0;
	memset(m_ems_bank, 0, sizeof(m_ems_bank));
	contrast(*color->first_field(), NULL, 0, color->read());
}

static MACHINE_CONFIG_START( pasogo, pasogo_state )

	MCFG_CPU_ADD("maincpu", V30, XTAL_32_22MHz/2)
	MCFG_CPU_PROGRAM_MAP(pasogo_mem)
	MCFG_CPU_IO_MAP(pasogo_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pasogo_state,  pasogo_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_DEVICE_ADD("ems", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(emsbank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(16)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x4000)

	MCFG_IBM5160_MOTHERBOARD_ADD("mb", "maincpu")

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")

	// It's a CGA device right so lets use isa_cga!  Well, not so much.
	// The carts use vg230 specific registers and mostly ignore the mc6845.
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(pasogo_state, screen_update_pasogo)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD("palette", 2)

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "pasogo_cart")
	MCFG_GENERIC_WIDTH(GENERIC_ROM16_WIDTH)
	MCFG_GENERIC_MANDATORY

	MCFG_SOFTWARE_LIST_ADD("cart_list","pasogo")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("vg230_timer", pasogo_state, vg230_timer, attotime::from_hz(1))
MACHINE_CONFIG_END

ROM_START( pasogo )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
ROM_END

//    YEAR   NAME    PARENT  COMPAT    MACHINE   INPUT     INIT      COMPANY  FULLNAME          FLAGS
CONS( 1996, pasogo,   0,      0,       pasogo,  pasogo, driver_device,    0,   "KOEI", "PasoGo", MACHINE_NO_SOUND|MACHINE_NOT_WORKING)

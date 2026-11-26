// license:BSD-3-Clause
// copyright-holders:R. Belmont,Curt Coder
/**********************************************************************

Commodore MPS-1230 Dot Matrix Printer
Commodore 1988/89
Hardware info by Guru

This is a 9-pin dot matrix printer manufactured by Commodore (possibly under license from Olivetti)
The printer is based on the Olivetti DM100 9-pin dot matrix printer with an added Commodore
serial port and other Commodore modifications. The service manual shows the PCB as 'DM10XC2',
however the actual PCB does not exactly match that shown in the service manual.
The firmware is modified by Commodore to support the CBM ASCII character and command set.

The printer comes standard with Centronics Parallel and Commodore Serial ports, sheet feed
and tractor feed and can be used with Commodore 64, 128/D, Amiga and IBM compatibles.

The ink ribbons are compatible with, and used in the following printers....
Commodore MPS 1230 printer
Olivetti DM 100, DM 90-1, DM 95, DM 99 & PR 98 printers
Birch BR-600 POS/cash register
Facit E 440 printer
Philips NMS 1016 & NMS 1432 printers
Senor Science K-700 & K-750 POS/cash register
Triumph Adler MPR 7120 printer


PCB Layout
----------

BA235
OLIVETTI OPE MADE IN HONGKONG
750746Y00-A-B9

    CENTRONICS         SERIAL
|-|------------|--------|---|----------|
|J899  J369   9306 7407 J02G      L387A|
|                                  J169|
|               D7810  6264 27512      |
|J708   MB624207                       |
|-----|                                |
      |      11.06MHz         J532     |
      |                                |
      |    555  L293  L6210 2803 2803  |
      |J404 74LS14  J477      2803 J821|
      |--------------------------------|
Notes: (all IC's shown)
      D7810    - NEC D7810HG ROM-less Microcontroller, clock input 11.06MHz  (64 pin 'spider' package)
      MB624207 - Fujitsu MB624207U MB62XXXX UHB Series 1.5um CMOS Gate Array marked 'OPE 200' (SDIP64)
      6264     - 8k x8 SRAM (DIP28)
      27512    - TMS27C512 64k x8 EPROM (DIP28)
                 S/W revisions dumped:
                                     'Release R - 1.1D 10/NOV/1988' with label 'PDL2'
                                     'Release R - 1.1E 04/APR/1989'
                                     'Release R - 2.1E 09/AUG/1989' with label 'PEEK'
      L387A    - ST L387A 5V @ 0.5A Voltage Regulator
      2803     - ULN2803A 8-Channel Darlinton Driver (DIP18)
      L6210    - ST L6210R Dual Schottky Diode Bridge (DIP16)
      L293     - ST L293ER Quad Push-Pull Driver (DIP20)
      74LS14   - National DM74LS14N Hex Inverter with Schmitt Trigger (DIP14)
      555      - Texas Instruments NE555P Timer (DIP8)
      9306     - National NMC9306N 32bytes Serial EEPROM (DIP8)
      7407     - Texas Instruments ON7404N Hex Buffer with High Voltage Open-Collector Outputs (DIP14)
      J169     - 3-pin 9v power input connector (from built-in transformer)
      J821     - 6-pin connector for paper feed motor
      J532     - 12-pin connector for print head
      J477     - 2-pin print head transport motor connector
      J404     - 4-pin connector for paper-out photosensor
      J708     - 6-pin connector for transport motor encoder
      J899     - 15-pin connector for ribbon cable for console buttons and leds (feed, online etc)
      J369     - 36-pin centronics parallel connector
      J02G     - 6-pin Commodore serial bus connector

**********************************************************************/

#include "emu.h"
#include "mps1230.h"

#include <algorithm>
#include <cmath>



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MPS1230, mps1230_device, "mps1230", "Commodore MPS-1230 NLQ Printer")


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

namespace {

// carriage transport, in 1/1440" encoder counts
constexpr double CARRIAGE_COUNTS_PER_INCH = 1440.0;
constexpr double CARRIAGE_TRAVEL = 11 * CARRIAGE_COUNTS_PER_INCH;
constexpr double CARRIAGE_SLOT = 4.0;
constexpr double CARRIAGE_STROBE = 400.0;
constexpr double CARRIAGE_SPEED_MAX = 60000.0;
constexpr double CARRIAGE_TAU_DRIVE = 0.004;
constexpr double CARRIAGE_TAU_BRAKE = 0.002;
constexpr double CARRIAGE_TAU_COAST = 0.020;
constexpr attotime CARRIAGE_TICK = attotime::from_usec(25);

// A4 paper, printed at 240 x 216 dpi, with its left edge 0.2" left of the first character position
constexpr int PAPER_HDPI = 240;
constexpr int PAPER_VDPI = 216;
constexpr int PAPER_WIDTH = 827 * PAPER_HDPI / 100;
constexpr int PAPER_LEFT = 157;
constexpr int PAPER_HEIGHT = 11 * PAPER_VDPI;
constexpr int NEEDLE_PITCH = PAPER_VDPI / 72;

// a needle's dot is roughly 1/60" across
constexpr int DOT_WIDTH = PAPER_HDPI / 60;
constexpr int DOT_HEIGHT = NEEDLE_PITCH;

// factory defaults, with word 10 bits 0-1 selecting the interface (1 = Commodore serial, 2 = parallel)
constexpr u16 EEPROM_DEFAULT_SERIAL[16] =
{
	0x4e45, 0x4e54, 0x0a99, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0x92a1, 0x0880, 0x802a, 0x6aa0, 0x4094, 0x8282
};

constexpr u16 EEPROM_DEFAULT_PARALLEL[16] =
{
	0x4e45, 0x4e54, 0x0a99, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0x92a2, 0x0880, 0x802a, 0x6aa0, 0x4094, 0x8282
};

} // anonymous namespace


//-------------------------------------------------
//  ROM( mps1230 )
//-------------------------------------------------

ROM_START( mps1230 )
	ROM_REGION( 0x10000, "firmware", 0 )
	ROM_DEFAULT_BIOS("peek")
	ROM_SYSTEM_BIOS( 0, "pdl2", "Release R - 1.1D 10/NOV/1988" )
	ROMX_LOAD( "pdl2.f03ee", 0x00000, 0x10000, CRC(f8a9f45c) SHA1(4e7bb0d382c55432665f5576b6a5cd3c4c33bd8e), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v11e", "Release R - 1.1E 04/APR/1989" )
	ROMX_LOAD( "mps1230-v11e-xxxxxx-xx.bin", 0x00000, 0x10000, CRC(29f055db) SHA1(423799cdd9a52a71ece21fb2061d2b1844f25f6d), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "peek", "Release R - 2.1E 09/AUG/1989" )
	ROMX_LOAD( "peek.f03ee", 0x00000, 0x10000, CRC(b5215f25) SHA1(dcfdd16942652447c472301392d9b39514547af1), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mps1230_device::device_rom_region() const
{
	return ROM_NAME( mps1230 );
}


//-------------------------------------------------
//  ADDRESS_MAP( mps1230_mem )
//-------------------------------------------------

void mps1230_device::mps1230_mem(address_map &map)
{
	map(0x0000, 0xbfff).rom().region("firmware", 0);
	map(0xc000, 0xdfff).ram(); // as per the service manual
	map(0xe000, 0xe000).lr8(NAME([this] () { return m_cent_latch; }));
	map(0xe001, 0xe001).lw8(NAME([this] (u8 data) { needle9_w(1); }));
	map(0xe004, 0xe004).w(FUNC(mps1230_device::busy_clear_w));
	map(0xe005, 0xe005).lw8(NAME([this] (u8 data) { m_riloc = 1; }));
	map(0xe006, 0xe006).lw8(NAME([this] (u8 data) { m_riloc = 0; }));
	map(0xe007, 0xe007).w(FUNC(mps1230_device::fire_w));
	map(0xe009, 0xe009).lw8(NAME([this] (u8 data) { needle9_w(0); }));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mps1230_device::device_add_mconfig(machine_config &config)
{
	UPD7810(config, m_maincpu, 11'060'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mps1230_device::mps1230_mem);
	m_maincpu->pa_in_cb().set(FUNC(mps1230_device::pa_r));
	m_maincpu->pa_out_cb().set(FUNC(mps1230_device::pa_w));
	m_maincpu->pb_in_cb().set(FUNC(mps1230_device::pb_r));
	m_maincpu->pb_out_cb().set(FUNC(mps1230_device::pb_w));
	m_maincpu->pc_in_cb().set(FUNC(mps1230_device::pc_r));
	m_maincpu->pc_out_cb().set(FUNC(mps1230_device::pc_w));
	m_maincpu->rxd_func().set([this] () { return has_iec() ? m_bus->data_r() : 1; });
	m_maincpu->an1_func().set_ioport("PAPER");
	m_maincpu->an2_func().set([this] () { return BIT(m_panel->read(), 0) ? 0xff : 0x00; });

	EEPROM_93C06_16BIT(config, m_eeprom);
	if (dynamic_cast<centronics_device *>(owner()))
		m_eeprom->default_data(EEPROM_DEFAULT_PARALLEL, sizeof(EEPROM_DEFAULT_PARALLEL));
	else
		m_eeprom->default_data(EEPROM_DEFAULT_SERIAL, sizeof(EEPROM_DEFAULT_SERIAL));

	BITMAP_PRINTER(config, m_bitmap_printer, PAPER_WIDTH, PAPER_HEIGHT, PAPER_HDPI, PAPER_VDPI);
	m_bitmap_printer->set_pf_stepper_ratio(1, 2);
}


//-------------------------------------------------
//  INPUT_PORTS( mps1230 )
//-------------------------------------------------

static INPUT_PORTS_START( mps1230 )
	PORT_START("PANEL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Local") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(mps1230_device::local_changed), 0)
	PORT_BIT( 0x3e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Line Feed")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Form Feed")

	PORT_START("PAPER")
	PORT_CONFNAME( 0xff, 0xff, "Paper" )
	PORT_CONFSETTING( 0x00, "Out" )
	PORT_CONFSETTING( 0xff, "Loaded" )
INPUT_PORTS_END


//-------------------------------------------------
//  local_changed - the gate array toggles RILOC
//  on each press of the LOCAL key
//-------------------------------------------------

INPUT_CHANGED_MEMBER(mps1230_device::local_changed)
{
	if (oldval && !newval)
		m_riloc ^= 1;
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor mps1230_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mps1230 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mps1230_device - constructor
//-------------------------------------------------

mps1230_device::mps1230_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MPS1230, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this),
	device_centronics_peripheral_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_eeprom(*this, "eeprom"),
	m_bitmap_printer(*this, "bitmap_printer"),
	m_pf_stepper(*this, "bitmap_printer:pf_stepper"),
	m_panel(*this, "PANEL"),
	m_carriage_timer(nullptr),
	m_riloc(0),
	m_pa(0),
	m_position(CARRIAGE_TRAVEL / 2),
	m_velocity(0.0),
	m_sens0(0),
	m_needle9(0),
	m_cent_data(0xff),
	m_cent_latch(0xff),
	m_cent_strobe(1),
	m_busin(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mps1230_device::device_start()
{
	m_carriage_timer = timer_alloc(FUNC(mps1230_device::carriage_tick), this);

	save_item(NAME(m_riloc));
	save_item(NAME(m_pa));
	save_item(NAME(m_position));
	save_item(NAME(m_velocity));
	save_item(NAME(m_sens0));
	save_item(NAME(m_needle9));
	save_item(NAME(m_cent_data));
	save_item(NAME(m_cent_latch));
	save_item(NAME(m_cent_strobe));
	save_item(NAME(m_busin));
}


//-------------------------------------------------
//  carriage_tick - carriage transport motor and
//  strobe disk encoder
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(mps1230_device::carriage_tick)
{
	double const dt = CARRIAGE_TICK.as_double();
	bool const enable = BIT(m_pa, 3);
	int const sens1 = BIT(m_pa, 0);
	int const sens2 = BIT(m_pa, 1);

	m_maincpu->set_input_line(UPD7810_INTF1, CLEAR_LINE);
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	if (enable && (sens1 != sens2))
	{
		double const target = sens1 ? -CARRIAGE_SPEED_MAX : CARRIAGE_SPEED_MAX;
		m_velocity += (target - m_velocity) * dt / CARRIAGE_TAU_DRIVE;
	}
	else
	{
		m_velocity -= m_velocity * dt / (enable ? CARRIAGE_TAU_BRAKE : CARRIAGE_TAU_COAST);
	}

	double const old_position = m_position;
	m_position = std::clamp(m_position + m_velocity * dt, 0.0, CARRIAGE_TRAVEL);

	if (m_position == 0.0 || m_position == CARRIAGE_TRAVEL)
		m_velocity = 0.0;

	if (m_velocity != 0.0)
		m_sens0 = (m_velocity < 0.0) ? 1 : 0;

	int const count = int(m_position);
	int const old_count = int(old_position);

	for (int i = std::abs(count - old_count); i > 0; i--)
	{
		m_maincpu->ci_w(0);
		m_maincpu->ci_w(1);
	}

	if (std::floor(m_position / CARRIAGE_SLOT) != std::floor(old_position / CARRIAGE_SLOT))
		m_maincpu->set_input_line(UPD7810_INTF1, ASSERT_LINE);

	if (std::floor(m_position / CARRIAGE_STROBE) != std::floor(old_position / CARRIAGE_STROBE))
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	m_bitmap_printer->setheadpos(int(m_position * PAPER_HDPI / CARRIAGE_COUNTS_PER_INCH) - PAPER_LEFT, m_bitmap_printer->m_ypos);

	if (!enable && std::abs(m_velocity) < 1.0)
	{
		m_velocity = 0.0;
		m_carriage_timer->enable(false);
	}
}


//-------------------------------------------------
//  pa_r -
//-------------------------------------------------

u8 mps1230_device::pa_r()
{
	return 0xfb | (m_sens0 << 2);
}


//-------------------------------------------------
//  pa_w -
//-------------------------------------------------

void mps1230_device::pa_w(u8 data)
{
	m_pa = data;

	m_bitmap_printer->update_pf_stepper(bitswap<4>(data, 7, 5, 4, 6));

	if (BIT(data, 3) && !m_carriage_timer->enabled())
		m_carriage_timer->adjust(CARRIAGE_TICK, 0, CARRIAGE_TICK);
}


//-------------------------------------------------
//  pb_r -
//-------------------------------------------------

u8 mps1230_device::pb_r()
{
	return (m_panel->read() & 0xc0) | 0x3e | m_eeprom->do_read();
}


//-------------------------------------------------
//  pb_w -
//-------------------------------------------------

void mps1230_device::pb_w(u8 data)
{
	m_eeprom->di_write(BIT(data, 0));
	m_eeprom->cs_write(BIT(data, 5));

	if (has_centronics())
	{
		output_ack(!BIT(data, 3));
		output_perror(!BIT(data, 4));
	}
}


//-------------------------------------------------
//  pc_r -
//-------------------------------------------------

u8 mps1230_device::pc_r()
{
	return (m_riloc << 7) | 0x6f | (m_busin << 4);
}


//-------------------------------------------------
//  pc_w -
//-------------------------------------------------

void mps1230_device::pc_w(u8 data)
{
	if (has_iec())
		m_bus->data_w(this, BIT(data, 0));

	if (has_centronics())
		output_select(!BIT(data, 6));
}


//-------------------------------------------------
//  busy_clear_w - release the gate array BUSY
//  latch
//-------------------------------------------------

void mps1230_device::busy_clear_w(u8 data)
{
	m_busin = 1;

	if (has_centronics())
		output_busy(0);
}


//-------------------------------------------------
//  input_strobe - latch data and assert BUSY
//-------------------------------------------------

void mps1230_device::input_strobe(int state)
{
	if (m_cent_strobe && !state)
	{
		m_cent_latch = m_cent_data;
		m_busin = 0;
		output_busy(1);
	}

	m_cent_strobe = state;
}


//-------------------------------------------------
//  input_init -
//-------------------------------------------------

void mps1230_device::input_init(int state)
{
	if (!state)
		m_busin = 0;

	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  needle9_w - needle 9 driver, shared with the
//  EEPROM clock
//-------------------------------------------------

void mps1230_device::needle9_w(int state)
{
	m_needle9 = !state;
	m_eeprom->clk_write(state);
}


//-------------------------------------------------
//  fire_w - latch needles 1-8 and fire the head
//-------------------------------------------------

void mps1230_device::fire_w(u8 data)
{
	int const x = int(m_position * PAPER_HDPI / CARRIAGE_COUNTS_PER_INCH) - PAPER_LEFT;
	int const y = m_bitmap_printer->m_ypos;

	if (x < 0 || x + DOT_WIDTH > PAPER_WIDTH)
		return;

	u16 const needles = data | (m_needle9 ? 0x100 : 0);

	for (int i = 0; i < 9; i++)
		if (BIT(needles, i))
			for (int dy = 0; dy < DOT_HEIGHT; dy++)
				for (int dx = 0; dx < DOT_WIDTH; dx++)
					m_bitmap_printer->draw_pixel(x + dx, y + i * NEEDLE_PITCH + dy, 0x000000);
}


//-------------------------------------------------
//  device_reset_after_children - device-specific
//  reset after children
//-------------------------------------------------

void mps1230_device::device_reset_after_children()
{
	m_pf_stepper->set_absolute_position(m_bitmap_printer->get_top_margin() * 2);

	m_busin = 0;

	if (has_iec())
	{
		m_maincpu->sck_w(m_bus->clk_r());
		m_maincpu->set_input_line(UPD7810_INTF2, m_bus->atn_r() ? CLEAR_LINE : ASSERT_LINE);
	}

	if (has_centronics())
	{
		output_busy(1);
		output_ack(1);
		output_fault(1);
	}
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void mps1230_device::cbm_iec_atn(int state)
{
	m_maincpu->set_input_line(UPD7810_INTF2, state ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  cbm_iec_clk -
//-------------------------------------------------

void mps1230_device::cbm_iec_clk(int state)
{
	m_maincpu->sck_w(state);
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void mps1230_device::cbm_iec_reset(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);
}

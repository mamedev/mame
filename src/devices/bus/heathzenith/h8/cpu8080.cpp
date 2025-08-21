// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit 8080A CPU card

  Original 8080A 2.048MHz CPU board from Heath Company.



  Onboard Jumpers

  Label    Default  Emulated   Description (if jumper is closed)
  -------------------------------------------------------------------------
  B1-B2     Open      Yes      Allow /INT1 (INT 10) from the BH Bus (pin 8).

  C1-C2     Open      Yes      Allow /INT2 (Int 20) from the BH Bus (pin 9).

  E1-E2     Close     No       Allow inverted signal from J jumper to HOLD/HLDA on BH Bus (pin 25)

  F1-F2     Close     No       Allow /HOLD from the BH Bus (pin 27) to CPU Hold line.

  H1-H3     Open      No       Tie Ready-in from IC212 Generator to CPU Wait line
  H2-H3     Close     No       Tie Ready-in from IC212 Generator to CPU Ready line

  J1-J2     Close     No       CPU HLDA out through flip-flop, use /Q output to E jumper
  J2-J3     Open      No       CPU HLDA out through flip-flop, use Q output to E jumper

  K1-K2     Close     No       Affects Data-in/out buffers enabled signals based on /MEMR & /IOR signals
  K1-K3     Open      No       Affects Data-in/out buffers enabled signals based on /MEMR & /IOR signals

  L1-L2     Close     No       Tie CPU /INTA to interrupt buffer
  L2-L3     Open      No       Tie CPU /INTA to +12V

  P1-P2     Open      No       A10 to pin 19 of IC204 ROM
  P2-P3     Close     No       R jumper to pin 19 of IC204 ROM

  R1-R2     Close     No       +12V selected
  R2-R3     Open      No       S jumper selection selected

  S1-S2     Open      No       +5V selected
  S2-S3     Close     No       GND selected

  T1-T2     Close     No       -5V to pin 21 of IC204 ROM
  T2-T3     Open      No       +5V to pin 21 of IC204 ROM

  X1-X2     Open      No       Allow /ROM_Disable from BH bus (pin 46) to disable ROM

  Z1-Z2     Close     No       Tie /A10 to ROM decoder select line
  Z2-Z3     Open      No       Tie +5V to ROM decoder select line

  RDYIN     Close     No       Allow RDYIN from BH Bus (pin 20) to IC212 Generator

****************************************************************************/

#include "emu.h"
#include "cpu8080.h"

#include "bus/heathzenith/intr_cntrl/intr_cntrl.h"
#include "cpu/i8085/i8085.h"


namespace {

class h_8_cpu_8080_device : public device_t, public device_h8bus_card_interface, public device_p201_p2_card_interface
{
public:

	h_8_cpu_8080_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void int1_w(int state) override;
	virtual void int2_w(int state) override;
	virtual void int3_w(int state) override;
	virtual void int4_w(int state) override;
	virtual void int5_w(int state) override;
	virtual void int6_w(int state) override;
	virtual void int7_w(int state) override;

	virtual void p201_reset_w(int state) override;
	virtual void p201_int1_w(int state) override;
	virtual void p201_int2_w(int state) override;

protected:

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void handle_int1();
	void handle_int2();

private:
	void h8_status_callback(u8 data);
	void h8_inte_callback(int state);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void bus_mem_w(offs_t offset, u8 data) { m_mem.write_byte(offset, data); }
	u8 bus_mem_r(offs_t offset) { return m_mem.read_byte(offset); }
	void bus_io_w(offs_t offset, u8 data) { m_io.write_byte(offset, data); }
	u8 bus_io_r(offs_t offset) { return m_io.read_byte(offset); }

	required_device<i8080_cpu_device>  m_maincpu;
	required_device<heath_intr_socket> m_intr_socket;
	required_ioport                    m_config;

	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_mem;
	memory_access<8, 0, 0, ENDIANNESS_LITTLE>::specific  m_io;

	bool m_m1_state;
	bool m_allow_bus_int1;
	bool m_allow_bus_int2;
	bool m_p201_int1;
	bool m_p201_int2;
	bool m_bus_int1;
	bool m_bus_int2;
};

h_8_cpu_8080_device::h_8_cpu_8080_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, H8BUS_CPU_8080, tag, owner, 0)
	, device_h8bus_card_interface(mconfig, *this)
	, device_p201_p2_card_interface(*this, tag)
	, m_maincpu(*this, "maincpu")
	, m_intr_socket(*this, "intr_socket")
	, m_config(*this, "CONFIG")
{
}

void h_8_cpu_8080_device::h8_inte_callback(int state)
{
	m_p201_inte(state);
}

void h_8_cpu_8080_device::h8_status_callback(u8 data)
{
	int state = (data & i8080_cpu_device::STATUS_M1) ? 1 : 0;

	if (state != m_m1_state)
	{
		set_slot_m1(state);

		m_m1_state = state;
	}
}

void h_8_cpu_8080_device::int1_w(int state)
{
	if (m_allow_bus_int1)
	{
		m_bus_int1 = bool(state);

		handle_int1();
	}
}

void h_8_cpu_8080_device::int2_w(int state)
{
	if (m_allow_bus_int2)
	{
		m_bus_int2 = bool(state);

		handle_int2();
	}
}

void h_8_cpu_8080_device::int3_w(int state)
{
	m_intr_socket->set_irq_level(3, state);
}

void h_8_cpu_8080_device::int4_w(int state)
{
	m_intr_socket->set_irq_level(4, state);
}

void h_8_cpu_8080_device::int5_w(int state)
{
	m_intr_socket->set_irq_level(5, state);
}

void h_8_cpu_8080_device::int6_w(int state)
{
	m_intr_socket->set_irq_level(6, state);
}

void h_8_cpu_8080_device::int7_w(int state)
{
	m_intr_socket->set_irq_level(7, state);
}

void h_8_cpu_8080_device::p201_reset_w(int state)
{
	if (state)
	{
		m_maincpu->reset();
	}

	set_slot_reset(state);
}

void h_8_cpu_8080_device::p201_int1_w(int state)
{
	m_p201_int1 = bool(state);

	handle_int1();
}

void h_8_cpu_8080_device::p201_int2_w(int state)
{
	m_p201_int2 = bool(state);

	handle_int2();
}

void h_8_cpu_8080_device::handle_int1()
{
	m_intr_socket->set_irq_level(1, (m_p201_int1 || m_bus_int1) ? ASSERT_LINE : CLEAR_LINE);
}

void h_8_cpu_8080_device::handle_int2()
{
	m_intr_socket->set_irq_level(2, (m_p201_int2 || m_bus_int2) ? ASSERT_LINE : CLEAR_LINE);
}


static void intr_ctrl_options(device_slot_interface &device)
{
	device.option_add("original", HEATH_INTR_CNTRL);
}

void h_8_cpu_8080_device::mem_map(address_map &map)
{
	map.unmap_value_high();

	// default mem access to h8bus
	map(0x0000, 0xffff).rw(FUNC(h_8_cpu_8080_device::bus_mem_r), FUNC(h_8_cpu_8080_device::bus_mem_w));

	// main rom
	map(0x0000, 0x0fff).rom().region("maincpu", 0).unmapw();
}

void h_8_cpu_8080_device::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);

	// default io access to h8bus
	map(0x0000, 0xff).rw(FUNC(h_8_cpu_8080_device::bus_io_r), FUNC(h_8_cpu_8080_device::bus_io_w));
}


// ROM definition
ROM_START( h8 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )

	// H17 fdc bios - needed by bios2&3 this is on a separate card, keeping it documented here until that
	// card is implemented.
	//ROM_LOAD( "2716_444-19_h17.rom", 0x1800, 0x0800,     CRC(26e80ae3) SHA1(0c0ee95d7cb1a760f924769e10c0db1678f2435c))

	ROM_SYSTEM_BIOS(0, "bios0", "Standard")
	ROMX_LOAD( "2708_444-13_pam8.rom", 0x0000, 0x0400,   CRC(e0745513) SHA1(0e170077b6086be4e5cd10c17e012c0647688c39), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS(1, "bios1", "Alternate")
	ROMX_LOAD( "2708_444-13_pam8go.rom", 0x0000, 0x0400, CRC(9dbad129) SHA1(72421102b881706877f50537625fc2ab0b507752), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(2, "bios2", "Disk OS")
	ROMX_LOAD( "2716_444-13_pam8at.rom", 0x0000, 0x0800, CRC(fd95ddc1) SHA1(eb1f272439877239f745521139402f654e5403af), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS(3, "bios3", "Disk OS Alt")
	ROMX_LOAD( "2732_444-70_xcon8.rom", 0x0000, 0x1000,  CRC(b04368f4) SHA1(965244277a3a8039a987e4c3593b52196e39b7e7), ROM_BIOS(3) )

	// this one runs off into the weeds, because it is for the HA-8-6 Z-80 replacement CPU card, keeping it documented here, until
	// that card is implemented.
	// ROM_SYSTEM_BIOS(4, "bios4", "not working")
	// ROMX_LOAD( "2732_444-140_pam37.rom", 0x0000, 0x1000, CRC(53a540db) SHA1(90082d02ffb1d27e8172b11fff465bd24343486e), ROM_BIOS(4) )
ROM_END

static INPUT_PORTS_START( cpu_8080_jumpers )

	PORT_START("CONFIG")
	PORT_CONFNAME(0x01, 0x00, "Allow INT1 signal on BH Bus - Jumper B1-B2")
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x01, DEF_STR( Yes ))
	PORT_CONFNAME(0x02, 0x00, "Allow INT2 signal on BH Bus - Jumper C1-C2")
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x02, DEF_STR( Yes ))

INPUT_PORTS_END

ioport_constructor h_8_cpu_8080_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(cpu_8080_jumpers);
}

const tiny_rom_entry *h_8_cpu_8080_device::device_rom_region() const
{
	return ROM_NAME(h8);
}

void h_8_cpu_8080_device::device_start()
{
	save_item(NAME(m_m1_state));
	save_item(NAME(m_p201_int1));
	save_item(NAME(m_p201_int2));
	save_item(NAME(m_bus_int1));
	save_item(NAME(m_bus_int2));

	h8bus().space(AS_PROGRAM).specific(m_mem);
	h8bus().space(AS_IO).specific(m_io);

	h8bus().set_clock(m_maincpu->clock());
}

void h_8_cpu_8080_device::device_reset()
{
	m_m1_state  = false;
	m_p201_int1 = false;
	m_p201_int2 = false;
	m_bus_int1  = false;
	m_bus_int2  = false;

	ioport_value const config(m_config->read());

	m_allow_bus_int1 = bool(BIT(config, 0));
	m_allow_bus_int2 = bool(BIT(config, 1));
}

void h_8_cpu_8080_device::device_add_mconfig(machine_config &config)
{
	constexpr XTAL H8_CLOCK = XTAL(18'432'000) / 9;  // 2.048 MHz

	I8080(config, m_maincpu, H8_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &h_8_cpu_8080_device::mem_map);
	m_maincpu->set_addrmap(AS_IO, &h_8_cpu_8080_device::io_map);
	m_maincpu->out_status_func().set(FUNC(h_8_cpu_8080_device::h8_status_callback));
	m_maincpu->out_inte_func().set(FUNC(h_8_cpu_8080_device::h8_inte_callback));
	m_maincpu->set_irq_acknowledge_callback(m_intr_socket, FUNC(heath_intr_socket::irq_callback));

	HEATH_INTR_SOCKET(config, m_intr_socket, intr_ctrl_options, nullptr);
	m_intr_socket->irq_line_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_intr_socket->set_default_option("original");
	m_intr_socket->set_fixed(true);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H8BUS_CPU_8080, device_h8bus_card_interface, h_8_cpu_8080_device, "h8_cpu_8080", "Heath 8080A CPU board");

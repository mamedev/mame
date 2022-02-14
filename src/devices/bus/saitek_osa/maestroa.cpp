// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/***************************************************************************

Saitek OSA Module: Kasparov Maestro A (SciSys, 1986)

The chess engine revision is in-between Kaplan's Stratos and Turbostar.

Hardware notes:
- PCB label: M6L-PE-012 REV.2
- R65C02P4 @ 4MHz / 5.67MHz / 6MHz
- 32KB ROM (D27C256)
- 8KB RAM (HM6264LP-15)
- 3 more sockets, one for KSO expansion ROM, 2 unused

The CPU is a 4MHz part, higher speed modules overclock it. The PCB is not
compatible for upgrading to newer Maestro versions.

TODO:
- does not work if cpu speed is 4MHz

***************************************************************************/

#include "emu.h"
#include "maestroa.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/m6502/r65c02.h"

#include "softlist_dev.h"


DEFINE_DEVICE_TYPE(OSA_MAESTROA, saitekosa_maestroa_device, "osa_maestroa", "Maestro A")


//-------------------------------------------------
//  initialization
//-------------------------------------------------

saitekosa_maestroa_device::saitekosa_maestroa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, OSA_MAESTROA, tag, owner, clock),
	device_saitekosa_expansion_interface(mconfig, *this),
	m_maincpu(*this, "maincpu")
{ }

void saitekosa_maestroa_device::device_start()
{
	save_item(NAME(m_latch_enable));
	save_item(NAME(m_latch));
}

void saitekosa_maestroa_device::device_reset()
{
	set_cpu_freq();
	control_w(0);
}

void saitekosa_maestroa_device::set_cpu_freq()
{
	static const XTAL xtal[3] = { 4_MHz_XTAL, 5.67_MHz_XTAL, 6_MHz_XTAL };
	m_maincpu->set_unscaled_clock(xtal[ioport("FAKE")->read() % 3]);
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( maestroa )
	ROM_REGION(0x10000, "maincpu", 0)

	ROM_DEFAULT_BIOS("a1")

	// A
	ROM_SYSTEM_BIOS(0, "a1", "Maestro A (set 1)")
	ROMX_LOAD("m6a_a29b.u6", 0x8000, 0x8000, CRC(6ee0197a) SHA1(61f201ca64576aca582bc9f2a427638bd79e1fee), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "a2", "Maestro A (set 2)")
	ROMX_LOAD("m6a_v14b.u6", 0x8000, 0x8000, CRC(d566a476) SHA1(ef81b9a0dcfbd8427025cfe9bf738d42a7a1139a), ROM_BIOS(1))
ROM_END

const tiny_rom_entry *saitekosa_maestroa_device::device_rom_region() const
{
	return ROM_NAME(maestroa);
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( maestroa )
	PORT_START("FAKE")
	PORT_CONFNAME( 0x03, 0x02, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, saitekosa_maestroa_device, switch_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "4MHz" )
	PORT_CONFSETTING(    0x01, "5.67MHz" )
	PORT_CONFSETTING(    0x02, "6MHz" )
INPUT_PORTS_END

ioport_constructor saitekosa_maestroa_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(maestroa);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void saitekosa_maestroa_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	R65C02(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &saitekosa_maestroa_device::main_map);

	// extension rom
	GENERIC_SOCKET(config, "extrom", generic_plain_slot, "saitek_kso");
	SOFTWARE_LIST(config, "cart_list").set_original("saitek_kso");
}


//-------------------------------------------------
//  internal i/o
//-------------------------------------------------

u8 saitekosa_maestroa_device::rts_r()
{
	if (!machine().side_effects_disabled())
	{
		// strobe RTS-P
		m_expansion->rts_w(1);
		m_expansion->rts_w(0);
	}

	return 0xff;
}

void saitekosa_maestroa_device::xdata_w(u8 data)
{
	// clock latch
	m_latch = data;
}

u8 saitekosa_maestroa_device::xdata_r()
{
	return m_expansion->data_state();
}

void saitekosa_maestroa_device::control_w(u8 data)
{
	// d3: enable latch output
	m_latch_enable = bool(data & 8);

	// d2: STB-P
	m_expansion->stb_w(BIT(data, 2));
}

u8 saitekosa_maestroa_device::ack_r()
{
	// d7: ACK-P
	return m_expansion->ack_state() ? 0x80 : 0;
}

void saitekosa_maestroa_device::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).ram();
	map(0x2200, 0x2200).mirror(0x01ff).r(FUNC(saitekosa_maestroa_device::rts_r));
	map(0x2400, 0x2400).mirror(0x01ff).rw(FUNC(saitekosa_maestroa_device::xdata_r), FUNC(saitekosa_maestroa_device::xdata_w));
	map(0x2600, 0x2600).mirror(0x01ff).rw(FUNC(saitekosa_maestroa_device::ack_r), FUNC(saitekosa_maestroa_device::control_w));
	map(0x4000, 0x5fff).r("extrom", FUNC(generic_slot_device::read_rom));
	map(0x8000, 0xffff).rom();
}


//-------------------------------------------------
//  host i/o
//-------------------------------------------------

u8 saitekosa_maestroa_device::data_r()
{
	return m_latch_enable ? m_latch : 0xff;
}

void saitekosa_maestroa_device::nmi_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, !state ? ASSERT_LINE : CLEAR_LINE);
}

void saitekosa_maestroa_device::ack_w(int state)
{
	if (state != m_expansion->ack_state())
		machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}

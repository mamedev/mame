// license:BSD-3-Clause
// copyright-holders:A. Lenard
/***************************************************************************

    Zilog System 8000 driver

Based on docs and code from Oliver Lehmann's site (http://www.pofo.de/S8000/)


Models
------

System 8000 is a backplane-based minicomputer with 6 or 10 card slots, and as such it can have a variety of configuratons, some of which are officially designated as Models.
The following System 8000 models are known:

               CPU  Segmented  Disk type   Exp. Slots   Mem. Slots
Model 11        A       N        5.25"          6            2
Model 11 Plus   A       Y        5.25"          6            2
Model 12        H       Y        5.25"          6            2
Model 20       A10      N         8"           10           2/4
Model 21        A       N         8"           10           2/4
Model 21 Plus   A       Y        5.25"         10           2/4
Model 22        H       Y        5.25"         10           2/4
Model 30       A10      N         8"           10           2/4
Model 31        A       N        SMD           10           2/4
Model 31 Plus   A       Y        SMD           10           2/4
Model 32        H       Y        SMD           10           2/4

* CPU A10 (CPU-A 1.0) = 5.6 Mhz, 8 serial ports + 1 printer port
* CPU A (CPU-A) = Same as CPU A10, but with a larger ROM
* CPU H (HPCPU) = 11.1 Mhz + 32 KB cache, 2 serial ports

Models x2 are referred to as "System 8000 Series Two".

(There is also a "System 8000/32" with a 32-bit WE32100 CPU board and support for Unix System V, sold as Models 110 and 130, but I haven't found any reference materials about it online.)


Monitor program (BIOS) version differences
------------------------------------------

Monitor 1.0 uses a CPU-A board with only 8 KB ROM and a different memory location for the on-board RAM than the other versions, which all have 16 KB Monitor ROMs.
Since there are no markings on this board to indicate these differences, I have arbitrarily named it "System 8000 CPU-A 1.0".

Segmented/non-segmented mode can be toggled on the CPU-A board by changing some jumpers (E1-E12), however segmented mode is not supported by Monitor versions below 2.2 (will error out during the MMU power-up tests). The HPCPU board is always in segmented mode.


Expansion cards
---------------

All expansion cards communicate through a bus interface called the ZBI (Z-Bus Backplane Interconnect).
The physical card interface consists of 2 DIN41612 96-pin connectors (P1 and P2). Only P1 is used for the bus itself, while P2 is specific to each card.
This means that some expansion slots have a fixed purpose and can only be used with certain cards (or cards that don't use P2).
The CPU board is always in the first slot, and the memory boards are in the last ones.

An expansion card other than the CPU can be one of the following:
    - SSB: Secondary Serial Board (8 serial ports + 1 printer port)
    - TCC: Tape Cartridge Controller (4x tape drives)
    - MTC: 9-track Tape Controller (2x tape drives)
    - WDC: Winchester Disk Controller (4x 8" disk drives)
    - MWDC: mini-Winchester Disk Controller (4x 5.25" disk drives)
    - SMDC: Storage Module Disk Controller (4x SMD drives) [2x boards]
    - FPP: Floating-Point Processor 8/01 [2x boards]
    - ICP: Intelligent Communication Processor 8/02 (network card)
    - ECC: ECC Memory Controller [requires at least 1 ECC Memory Array]
    - Parity Memory Board (256 KB to 4MB(?))
    - ECC Memory Array (256 KB to 1 MB) [requires the ECC Controller]

The system can use either an ECC Controller + Memory Array(s) configuration or the Parity Board(s) for RAM.
Although System 8000 cases only have space for 2 or 4 memory boards for a maximum of 4 MB of RAM, all Monitor versions are able to handle up to 7 MB according to my testing.

In order to simplify things, instead of using slot numbers, I have divided the expansion slots into the following categories:
    - CPU (fixed): CPU-A, CPU-A 1.0, HPCPU
    - Disk: WDC, MWDC, SMDC
    - Tape: TCC, MTC
    - Option1: SSB1, FPP
    - Option2: SSB2, ICP
    - Memory: ECC, Parity

This arrangement isn't fully accurate though, as it ignores the model numbers, so it might be changed later.

***************************************************************************/

#include "emu.h"

#include "bus/zbi/zbi.h"
#include "bus/zbi/zbi_cards.h"
#include "bus/zbi/s8k_cpu.h"
#include "machine/ram.h"

#include "s8k.lh"

//#define VERBOSE 1
#include "logmacro.h"

namespace {

class s8k_state : public driver_device
{
public:
	s8k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_bus(*this, "bus")
		, m_slot_cpu(*this, "slot_cpu")
		, m_ram(*this, "ram")
		, m_power_led(*this, "power_led")
		, m_normal_led(*this, "normal_led")
		, m_busack_led(*this, "busack_led")
	{
	}

	void s8k(machine_config &config);
	void s8k_v1(machine_config &config);
	void s8k_s2(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(start_btn_cb);

	void normal_led_w(int state);
	void busack_led_w(int state);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_config_complete() override ATTR_COLD;

	// Timer functions to catch rapid activity pulses
	TIMER_CALLBACK_MEMBER(normal_led_delay);
	TIMER_CALLBACK_MEMBER(busack_led_delay);

	required_device<zbi_bus_device> m_bus;
	required_device<zbi_slot_device> m_slot_cpu;
	required_device<ram_device> m_ram;

	output_finder<> m_power_led;
	output_finder<> m_normal_led;
	output_finder<> m_busack_led;

private:
	void init_ram(machine_config &config);

	emu_timer *m_normal_timer;
	emu_timer *m_busack_timer;

	s8k_cpu_base *m_cpu_device;
};

//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START( s8k )
	PORT_START("FRONTPANEL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Start")	PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(s8k_state::start_btn_cb), 0)
INPUT_PORTS_END


//**************************************************************************
//  INTERRUPT HANDLING
//**************************************************************************


//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

INPUT_CHANGED_MEMBER(s8k_state::start_btn_cb)
{
	m_cpu_device->nmi_switch_w(newval);
}

void s8k_state::normal_led_w(int state)
{
	if (state)
	{
		m_normal_led = state;
		m_normal_timer->reset();
	}
	else
		m_normal_timer->adjust(attotime::from_hz(30));
}

void s8k_state::busack_led_w(int state)
{
	if (state)
	{
		m_busack_led = state;
		m_busack_timer->reset();
	}
	else
		m_busack_timer->adjust(attotime::from_hz(30));
}

TIMER_CALLBACK_MEMBER(s8k_state::normal_led_delay)
{
	m_normal_led = 0;
}

TIMER_CALLBACK_MEMBER(s8k_state::busack_led_delay)
{
	m_busack_led = 0;
}

//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

void s8k_state::machine_start()
{
	m_normal_timer = timer_alloc(FUNC(s8k_state::normal_led_delay), this);
	m_busack_timer = timer_alloc(FUNC(s8k_state::busack_led_delay), this);

	m_power_led.resolve();
	m_normal_led.resolve();
	m_busack_led.resolve();

	m_power_led = 0;
	m_normal_led = 0;
	m_busack_led = 0;
}

void s8k_state::machine_reset()
{
	m_power_led = (m_cpu_device != nullptr);
}

void s8k_state::device_config_complete()
{
	device_t *cpu_lookup = m_slot_cpu.lookup()->get_card_device();

	if (cpu_lookup)
	{
		m_cpu_device = dynamic_cast<s8k_cpu_base*>(cpu_lookup);
		m_cpu_device->ns_cb().set(*this, FUNC(s8k_state::normal_led_w));
		m_cpu_device->busack_cb().set(*this, FUNC(s8k_state::busack_led_w));
	}
	else
		m_cpu_device = nullptr;
}

//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

void s8k_state::init_ram(machine_config &config)
{
	RAM(config, m_ram).set_default_size("1M").set_extra_options("256K,512K,768K,2M,3M,4M").set_default_value(0);
}

void s8k_state::s8k(machine_config &config)
{
	config.set_default_layout(layout_s8k);

	ZBI_BUS(config, m_bus, 0);
	ZBI_SLOT(config, "slot_cpu", m_bus, zbi_s8k_cpu_cards, "cpu_a", true);
	ZBI_SLOT(config, "slot_disk", m_bus, zbi_s8k_disk_cards, "smdc");
	ZBI_SLOT(config, "slot_tape", m_bus, zbi_s8k_tape_cards, nullptr);
	ZBI_SLOT(config, "slot_opt1", m_bus, zbi_s8k_option1_cards, nullptr);
	ZBI_SLOT(config, "slot_opt2", m_bus, zbi_s8k_option2_cards, nullptr);
	ZBI_SLOT(config, "slot_mem", m_bus, zbi_s8k_ram_cards, "parity");

	init_ram(config);
}

void s8k_state::s8k_v1(machine_config &config)
{
	config.set_default_layout(layout_s8k);

	ZBI_BUS(config, m_bus, 0);
	ZBI_SLOT(config, "slot_cpu", m_bus, zbi_s8k_cpu_cards, "cpu_a10", true);
	ZBI_SLOT(config, "slot_disk", m_bus, zbi_s8k_disk_cards, nullptr);
	ZBI_SLOT(config, "slot_tape", m_bus, zbi_s8k_tape_cards, nullptr); // WDC only
	ZBI_SLOT(config, "slot_opt1", m_bus, zbi_s8k_option1_cards, nullptr);
	ZBI_SLOT(config, "slot_opt2", m_bus, zbi_s8k_option2_cards, nullptr);
	ZBI_SLOT(config, "slot_mem", m_bus, zbi_s8k_ram_cards, "parity");

	init_ram(config);
}

void s8k_state::s8k_s2(machine_config &config)
{
	config.set_default_layout(layout_s8k);

	ZBI_BUS(config, m_bus, 0);
	ZBI_SLOT(config, "slot_cpu", m_bus, zbi_s8k_cpu_cards, "hpcpu", true);
	ZBI_SLOT(config, "slot_disk", m_bus, zbi_s8k_disk_cards, "smdc");
	ZBI_SLOT(config, "slot_tape", m_bus, zbi_s8k_tape_cards, nullptr);
	ZBI_SLOT(config, "slot_opt1", m_bus, zbi_s8k_option1_cards, nullptr);
	ZBI_SLOT(config, "slot_opt2", m_bus, zbi_s8k_option2_cards, nullptr);
	ZBI_SLOT(config, "slot_mem", m_bus, zbi_s8k_ram_cards, "parity");

	init_ram(config);
}

ROM_START(s8000)
ROM_END

ROM_START(s8000v1)
ROM_END

ROM_START(s8000s2)
ROM_END

} // anonymous namespace

//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY  FULLNAME                   FLAGS
COMP( 1982, s8000,   0,      0,      s8k,     s8k,   s8k_state, empty_init, "Zilog", "System 8000",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1982, s8000v1, s8000,  0,      s8k_v1,  s8k,   s8k_state, empty_init, "Zilog", "System 8000 Model 20/30", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1984, s8000s2, s8000,  0,      s8k_s2,  s8k,   s8k_state, empty_init, "Zilog", "System 8000 Series Two",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )

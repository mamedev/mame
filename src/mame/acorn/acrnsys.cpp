// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Acorn System 2, 3, 4, and 5

    Acorn System 2: 6502, VDU 40x25, 4K RAM, 4K ROM (BASIC), Cassette
    http://chrisacorns.computinghistory.org.uk/Computers/System2.html

    Acorn System 3: 6502, VDU 40x25, 16K RAM, 4K ROM (BASIC), FDC (1 disc)
    http://chrisacorns.computinghistory.org.uk/Computers/System3.html

    Acorn System 3: 6809, VDU 40x25, 16K RAM, FDC (1 disc)
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_6809_CPU.html

    Acorn System 4: 6502, VDU 40x25, 16K RAM, FDC (2 discs)
    http://chrisacorns.computinghistory.org.uk/Computers/System4.html

    Acorn System 5: 6502A, VDU 80x25, 32K RAM, FDC
    http://chrisacorns.computinghistory.org.uk/Computers/System5.html

  TODO:
    - 4K BASIC ROM is undumped for System 2/3


  6809 Monitor Commands:-
    Modify memory:
    M  - Modify starting at specified address
    MR - Modify registers
    MG - Modify from Go address
    MV - Modify from breakpoint address
    MP - Modify from saved Program counter

    Execute programs:
    G - Go to specified address
    P - Proceed from saved Program counter past specified number of breakpoints

    Debugging aids:
    R - Display registers
    V - Insert/delete breakpoint
    T - Trace one, or more, instructions
    . - Do trace, displaying register contents at each step

    Cassette interface:
    S - Save memory to named file
    L - Load named file, with optional offset
    F - Finish loading - no name search

    Printer interface:
    C - Copy to parallel printer

    Disk interface:
    D - Disk bootstrap

****************************************************************************/

#include "emu.h"

#include "acrnsys_kbd.h"

#include "bus/acorn/bus.h"
#include "bus/centronics/ctronics.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "machine/ins8154.h"

#include "softlist_dev.h"


namespace {

class acrnsys_state : public driver_device
{
public:
	acrnsys_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_irqs(*this, "irqs")
		, m_kbd(*this, "kbd")
		, m_bus(*this, "bus")
		, m_via6522(*this, "via6522")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);

	void a6502(machine_config &config);
	void a6809(machine_config &config);
	void a6502a(machine_config &config);

	void acrnsys2(machine_config &config);
	void acrnsys3(machine_config &config);
	void acrnsys3_6809(machine_config &config);
	void acrnsys4(machine_config &config);
	void acrnsys5(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void rst_w(int state);

	void a6502_mem(address_map &map) ATTR_COLD;
	void a6809_mem(address_map &map) ATTR_COLD;
	void a6502a_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_irqs;
	required_device<acrnsys_keyboard_device> m_kbd;
	required_device<acorn_bus_device> m_bus;
	optional_device<via6522_device> m_via6522;
};


void acrnsys_state::a6502_mem(address_map &map)
{
	map.unmap_value_low();
	map(0x0000, 0xffff).rw(m_bus, FUNC(acorn_bus_device::read), FUNC(acorn_bus_device::write));
	map(0x0000, 0x03ff).ram();
	map(0x0e00, 0x0e7f).mirror(0x100).rw("ins8154", FUNC(ins8154_device::read_io), FUNC(ins8154_device::write_io));
	map(0xf000, 0xffff).rom().region("maincpu", 0);
}

void acrnsys_state::a6809_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(m_bus, FUNC(acorn_bus_device::read), FUNC(acorn_bus_device::write));
	map(0x0000, 0x03ff).ram();
	map(0x0980, 0x098f).mirror(0x70).m(m_via6522, FUNC(via6522_device::map));
	map(0xf000, 0xffff).rom().region("maincpu", 0);
}

void acrnsys_state::a6502a_mem(address_map &map)
{
	map.unmap_value_low();
	map(0x0000, 0xffff).rw(m_bus, FUNC(acorn_bus_device::read), FUNC(acorn_bus_device::write));
	map(0x0000, 0x03ff).ram();
	map(0x0e00, 0x0e0f).mirror(0x1f0).m(m_via6522, FUNC(via6522_device::map));
	map(0xe000, 0xffff).rom().region("maincpu", 0);
}


void acrnsys_state::machine_start()
{
}


void acrnsys_state::rst_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);

	if (state)
	{
		machine().schedule_soft_reset();
	}
}


/***************************************************************************
    6502 CPU Board - Part No. 200,000
***************************************************************************/

void acrnsys_state::a6502(machine_config &config)
{
	M6502(config, m_maincpu, 24_MHz_XTAL / 24); /* 1MHz 6502 CPU */
	m_maincpu->set_addrmap(AS_PROGRAM, &acrnsys_state::a6502_mem);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	ins8154_device &ins8154(INS8154(config, "ins8154"));
	//ins8154.in_a().set(FUNC(acrnsys_state::ins8154_pa_r));
	//ins8154.out_a().set(FUNC(acrnsys_state::ins8154_pa_w));
	ins8154.in_b().set([this]() { return m_kbd->data_r() | (m_kbd->strobe_r() << 7); });

	ACRNSYS_KEYBOARD(config, m_kbd);
	m_kbd->rst_handler().set(FUNC(acrnsys_state::rst_w));
}


/***************************************************************************
    6502A CPU Board - Part No. 200,005
***************************************************************************/

void acrnsys_state::a6502a(machine_config &config)
{
	M6502(config, m_maincpu, 24_MHz_XTAL / 12); /* 2MHz 6502A CPU */
	m_maincpu->set_addrmap(AS_PROGRAM, &acrnsys_state::a6502a_mem);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	MOS6522(config, m_via6522, 4_MHz_XTAL / 4);
	m_via6522->readpa_handler().set([this]() { return m_kbd->data_r() | (m_kbd->strobe_r() << 7); });
	//m_via6522->cb2_handler().set(FUNC(acrnsys_state::cass_w));
	m_via6522->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	ACRNSYS_KEYBOARD(config, m_kbd);
	m_kbd->blank_handler().set("via6522", FUNC(via6522_device::write_ca1));
	m_kbd->rst_handler().set(FUNC(acrnsys_state::rst_w));
}


/***************************************************************************
    6809 CPU Board - Part No. 200,012
***************************************************************************/

void acrnsys_state::a6809(machine_config &config)
{
	MC6809(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &acrnsys_state::a6809_mem);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	MOS6522(config, m_via6522, 4_MHz_XTAL / 4);
	m_via6522->writepa_handler().set("cent_data_out", FUNC(output_latch_device::write));
	m_via6522->readpb_handler().set(m_kbd, FUNC(acrnsys_keyboard_device::data_r));
	m_via6522->ca2_handler().set("centronics", FUNC(centronics_device::write_strobe));
	//m_via6522->cb2_handler().set(FUNC(acrnsys_state::cass_w));
	m_via6522->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	centronics_device &centronics(CENTRONICS(config, "centronics", centronics_devices, "printer"));
	centronics.ack_handler().set(m_via6522, FUNC(via6522_device::write_ca1));
	centronics.busy_handler().set(m_via6522, FUNC(via6522_device::write_pa7));
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(latch);

	ACRNSYS_KEYBOARD(config, m_kbd);
	m_kbd->strobe_handler().set("via6522", FUNC(via6522_device::write_cb1));
	m_kbd->rst_handler().set(FUNC(acrnsys_state::rst_w));
}

/***************************************************************************
    DEFAULT CARD SETTINGS
***************************************************************************/

static DEVICE_INPUT_DEFAULTS_START(8k_ram0000)
	DEVICE_INPUT_DEFAULTS("LINKS", 0x07, 0x00)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(8k_ram2000)
	DEVICE_INPUT_DEFAULTS("LINKS", 0x07, 0x01)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(8k_ramc000)
	DEVICE_INPUT_DEFAULTS("LINKS", 0x07, 0x06)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(32k_ram32k)
	DEVICE_INPUT_DEFAULTS("LINKS", 0x07, 0x00)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(32k_ram16k)
	DEVICE_INPUT_DEFAULTS("LINKS", 0x07, 0x01)
DEVICE_INPUT_DEFAULTS_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void acrnsys_state::acrnsys2(machine_config &config)
{
	/* 6502 CPU Board */
	a6502(config);

	/* Acorn Bus - 8 Slot Backplane */
	ACORN_BUS(config, m_bus, 0);
	m_bus->out_irq_callback().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_bus->out_nmi_callback().set_inputline(m_maincpu, M6502_NMI_LINE);
	ACORN_BUS_SLOT(config, "bus1", m_bus, acorn_bus_devices, "8k").set_option_device_input_defaults("8k", DEVICE_INPUT_DEFAULTS_NAME(8k_ram2000)); // 0x2000-0x3fff
	ACORN_BUS_SLOT(config, "bus2", m_bus, acorn_bus_devices, "vdu40");
	ACORN_BUS_SLOT(config, "bus3", m_bus, acorn_bus_devices, "cass");
	ACORN_BUS_SLOT(config, "bus4", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus5", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus6", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus7", m_bus, acorn_bus_devices, nullptr);

	/* Software lists */
	SOFTWARE_LIST(config, "flop_list").set_original("acrnsys_flop");
	SOFTWARE_LIST(config, "rom_list").set_original("acrnsys_rom");
}


void acrnsys_state::acrnsys3(machine_config &config)
{
	/* 6502 CPU Board */
	a6502(config);

	/* Acorn Bus - 8 Slot Backplane */
	ACORN_BUS(config, m_bus, 0);
	m_bus->out_irq_callback().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_bus->out_nmi_callback().set_inputline(m_maincpu, M6502_NMI_LINE);
	ACORN_BUS_SLOT(config, "bus1", m_bus, acorn_bus_devices, "8k").set_option_device_input_defaults("8k", DEVICE_INPUT_DEFAULTS_NAME(8k_ram2000)); // 0x2000-0x3fff
	ACORN_BUS_SLOT(config, "bus2", m_bus, acorn_bus_devices, "8k").set_option_device_input_defaults("8k", DEVICE_INPUT_DEFAULTS_NAME(8k_ramc000)); // 0xc000-0xdfff
	ACORN_BUS_SLOT(config, "bus3", m_bus, acorn_bus_devices, "vdu40");
	ACORN_BUS_SLOT(config, "bus4", m_bus, acorn_bus_devices, "fdc");
	ACORN_BUS_SLOT(config, "bus5", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus6", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus7", m_bus, acorn_bus_devices, nullptr);

	/* Software lists */
	SOFTWARE_LIST(config, "flop_list").set_original("acrnsys_flop");
	SOFTWARE_LIST(config, "rom_list").set_original("acrnsys_rom");
}


void acrnsys_state::acrnsys3_6809(machine_config &config)
{
	/* 6809 CPU Board */
	a6809(config);

	/* Acorn Bus - 8 Slot Backplane */
	ACORN_BUS(config, m_bus, 0);
	m_bus->out_irq_callback().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_bus->out_nmi_callback().set_inputline(m_maincpu, M6502_NMI_LINE);
	ACORN_BUS_SLOT(config, "bus1", m_bus, acorn_bus_devices, "8k").set_option_device_input_defaults("8k", DEVICE_INPUT_DEFAULTS_NAME(8k_ram0000)); // 0x0000-0x1fff
	ACORN_BUS_SLOT(config, "bus2", m_bus, acorn_bus_devices, "8k").set_option_device_input_defaults("8k", DEVICE_INPUT_DEFAULTS_NAME(8k_ram2000)); // 0x2000-0x3fff
	ACORN_BUS_SLOT(config, "bus3", m_bus, acorn_bus_devices, "8k").set_option_device_input_defaults("8k", DEVICE_INPUT_DEFAULTS_NAME(8k_ramc000)); // 0xc000-0xdfff
	ACORN_BUS_SLOT(config, "bus4", m_bus, acorn_bus_devices, "vdu40");
	ACORN_BUS_SLOT(config, "bus5", m_bus, acorn_bus_devices, "cass");
	ACORN_BUS_SLOT(config, "bus6", m_bus, acorn_bus_devices, "fdc");
	ACORN_BUS_SLOT(config, "bus7", m_bus, acorn_bus_devices, nullptr);

	/* Software lists */
	SOFTWARE_LIST(config, "flop_list").set_original("acrnsys_flop");
}


void acrnsys_state::acrnsys4(machine_config &config)
{
	/* 6502 CPU Board */
	a6502(config);

	/* Acorn Bus - 14 Slot Backplane */
	ACORN_BUS(config, m_bus, 0);
	m_bus->out_irq_callback().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_bus->out_nmi_callback().set_inputline(m_maincpu, M6502_NMI_LINE);
	ACORN_BUS_SLOT(config, "bus1", m_bus, acorn_bus_devices, "8k").set_option_device_input_defaults("8k", DEVICE_INPUT_DEFAULTS_NAME(8k_ram2000)); // 0x2000-0x3fff
	ACORN_BUS_SLOT(config, "bus2", m_bus, acorn_bus_devices, "8k").set_option_device_input_defaults("8k", DEVICE_INPUT_DEFAULTS_NAME(8k_ramc000)); // 0xc000-0xdfff
	ACORN_BUS_SLOT(config, "bus3", m_bus, acorn_bus_devices, "vdu40");
	ACORN_BUS_SLOT(config, "bus4", m_bus, acorn_bus_devices, "fdc");
	ACORN_BUS_SLOT(config, "bus5", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus6", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus7", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus8", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus9", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus10", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus11", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus12", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus13", m_bus, acorn_bus_devices, nullptr);

	/* Software lists */
	SOFTWARE_LIST(config, "flop_list").set_original("acrnsys_flop");
	SOFTWARE_LIST(config, "rom_list").set_original("acrnsys_rom");
}


void acrnsys_state::acrnsys5(machine_config &config)
{
	/* 6502A CPU Board */
	a6502a(config);

	/* Acorn Bus - 7 Slot Backplane */
	ACORN_BUS(config, m_bus, 0);
	m_bus->out_irq_callback().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_bus->out_nmi_callback().set_inputline(m_maincpu, M6502_NMI_LINE);
	ACORN_BUS_SLOT(config, "bus1", m_bus, acorn_bus_devices, "32k").set_option_device_input_defaults("32k", DEVICE_INPUT_DEFAULTS_NAME(32k_ram32k)); // 32K
	ACORN_BUS_SLOT(config, "bus2", m_bus, acorn_bus_devices, "32k").set_option_device_input_defaults("32k", DEVICE_INPUT_DEFAULTS_NAME(32k_ram16k)); // 16K
	ACORN_BUS_SLOT(config, "bus3", m_bus, acorn_bus_devices, "vdu80");
	ACORN_BUS_SLOT(config, "bus4", m_bus, acorn_bus_devices, "fdc");
	ACORN_BUS_SLOT(config, "bus5", m_bus, acorn_bus_devices, "econet");
	ACORN_BUS_SLOT(config, "bus6", m_bus, acorn_bus_devices, nullptr);

	/* Software lists */
	SOFTWARE_LIST(config, "flop_list").set_original("acrnsys_flop");
	SOFTWARE_LIST(config, "rom_list").set_original("acrnsys_rom");
}


/***************************************************************************
    ROM definitions
***************************************************************************/

ROM_START( acrnsys2 )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("cos.ic7", 0x0800, 0x0800, CRC(38f59dc7) SHA1(c587da5dcc6878dcd0bc823c508472d38296003e)) // Acorn COS
ROM_END

ROM_START( acrnsys3 )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("tosdos-s3.ic7", 0x0000, 0x1000, CRC(9b1fbec4) SHA1(4cb322dadcfba9c452797d6cc2096f0c92e8792c)) // Acorn DOS
ROM_END

ROM_START( acrnsys3_6809 )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("acorn6809.ic4", 0x0800, 0x0800, CRC(5fa5b632) SHA1(b14a884bf82a7a8c23bc03c2e112728dd1a74896))

	ROM_REGION(0x100, "proms", 0)
	ROM_LOAD("acorn6809.ic11", 0x0000, 0x0100, CRC(7908317d) SHA1(e0f1e5bd3a8598d3b62bc432dd1f3892ed7e66d8)) // address decoder
ROM_END

#define rom_acrnsys4 rom_acrnsys3

ROM_START( acrnsys5 )
	/* 6502A CPU board can take 4K, 8K, 16K ROMs */
	ROM_REGION(0x2000, "maincpu", 0)
	/* References suggest models 5A-5E also exist, this is 5F */
	ROM_LOAD("sys5f_iss1.ic11", 0x0000, 0x2000, CRC(cd80418d) SHA1(e588298239b5360b5d1e15d5cd9f7fe2b1693e5d)) // 201,625
ROM_END

} // anonymous namespace


//    YEAR  NAME           PARENT    COMPAT  MACHINE        INPUT    CLASS          INIT        COMPANY            FULLNAME                     FLAGS
COMP( 1980, acrnsys2,      acrnsys3, 0,      acrnsys2,      0,       acrnsys_state, empty_init, "Acorn Computers", "Acorn System 2",            MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1980, acrnsys3,      0,        0,      acrnsys3,      0,       acrnsys_state, empty_init, "Acorn Computers", "Acorn System 3 (6502 CPU)", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1980, acrnsys3_6809, acrnsys3, 0,      acrnsys3_6809, 0,       acrnsys_state, empty_init, "Acorn Computers", "Acorn System 3 (6809 CPU)", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1980, acrnsys4,      acrnsys3, 0,      acrnsys4,      0,       acrnsys_state, empty_init, "Acorn Computers", "Acorn System 4",            MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1982, acrnsys5,      0,        0,      acrnsys5,      0,       acrnsys_state, empty_init, "Acorn Computers", "Acorn System 5",            MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )

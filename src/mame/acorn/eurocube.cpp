// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Control Universal EuroCUBE

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/CU_EuroBeeb.html

    TODO:
    - add configuration to run at either 1MHz or 2MHz.

****************************************************************************/

#include "emu.h"

#include "bus/acorn/bus.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "machine/m3002.h"
#include "machine/mos6551.h"

#include "softlist_dev.h"


namespace {

class eurocube_state : public driver_device
{
public:
	eurocube_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_view(*this, "view")
		, m_bus(*this, "bus")
		, m_map(*this, "map")
	{ }

	void eurocube65(machine_config &config);
	void eurocube09(machine_config &config);
	void eurobeeb2(machine_config &config);
	void eurobeeb3m(machine_config &config);
	void eurobeeb3c(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	memory_view m_view;
	required_device<acorn_bus_device> m_bus;
	required_ioport m_map;

	void eurocube65_mem(address_map &map) ATTR_COLD;
	void eurocube09_mem(address_map &map) ATTR_COLD;
	void eurocube_map(address_map &map) ATTR_COLD;
};


void eurocube_state::eurocube65_mem(address_map &map)
{
	map(0x0000, 0xffff).m(FUNC(eurocube_state::eurocube_map));
	map(0xfe00, 0xfe0f).rw("via", FUNC(via6522_device::read), FUNC(via6522_device::write));
	map(0xfe10, 0xfe17).rw("acia", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0xfe18, 0xfe1f).rw("rtc", FUNC(m3002_device::read), FUNC(m3002_device::write));
}


void eurocube_state::eurocube09_mem(address_map &map)
{
	map(0x0000, 0xffff).m(FUNC(eurocube_state::eurocube_map));
	map(0xee00, 0xee0f).rw("via", FUNC(via6522_device::read), FUNC(via6522_device::write));
	map(0xee10, 0xee17).rw("acia", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0xee18, 0xee1f).rw("rtc", FUNC(m3002_device::read), FUNC(m3002_device::write));
}


void eurocube_state::eurocube_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).view(m_view);

	// Map 0 - Development BASIC
	m_view[0](0x0000, 0x1fff).ram();                   // M3
	m_view[0](0x2000, 0x3fff).rom().region("m2", 0);   // M2
	m_view[0](0x8000, 0xbfff).rom().region("m1", 0);   // M1
	m_view[0](0xc000, 0xffff).rom().region("m0", 0);   // M0
	m_view[0](0xd000, 0xdfff).lrw8(                    // I/O Block
		NAME([this](offs_t offset) { return m_bus->read(offset | 0xd000); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xd000, data); })
	);
	m_view[0](0xfe00, 0xfeff).lrw8(                    // I/O Page
		NAME([this](offs_t offset) { return m_bus->read(offset | 0xfe00); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xfe00, data); })
	);

	// Map 1 - EPROM BASIC
	m_view[1](0x0000, 0x3fff).ram();                   // M3
	m_view[1](0x4000, 0x7fff).rom().region("m2", 0);   // M2
	m_view[1](0x8000, 0xbfff).rom().region("m1", 0);   // M1
	m_view[1](0xc000, 0xffff).rom().region("m0", 0);   // M0
	m_view[1](0xd000, 0xdfff).lrw8(                    // I/O Block
		NAME([this](offs_t offset) { return m_bus->read(offset | 0xd000); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xd000, data); })
	);
	m_view[1](0xfe00, 0xfeff).lrw8(                    // I/O Page
		NAME([this](offs_t offset) { return m_bus->read(offset | 0xfe00); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xfe00, data); })
	);

	// Map 2 - Minimum BASIC
	m_view[2](0x0000, 0x07ff).ram();                   // M3
	m_view[2](0x0800, 0x0fff).ram();                   // M2
	m_view[2](0x8000, 0xbfff).rom().region("m1", 0);   // M1
	m_view[2](0xc000, 0xffff).rom().region("m0", 0);   // M0
	m_view[2](0xd000, 0xdfff).lrw8(                    // I/O Block
		NAME([this](offs_t offset) { return m_bus->read(offset | 0xd000); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xd000, data); })
	);
	m_view[2](0xfe00, 0xfeff).lrw8(                    // I/O Page
		NAME([this](offs_t offset) { return m_bus->read(offset | 0xfe00); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xfe00, data); })
	);

	// Map 3 - Reserved for future use by Control Universal Ltd
	m_view[3](0x0000, 0xffff).noprw();

	// Map 4 - FLEX, low cost (6809 only)
	m_view[4](0x0000, 0x1fff).ram();                   // M3
	m_view[4](0x2000, 0x3fff).ram();                   // M2
	m_view[4](0xe000, 0xffff).rom().region("m0", 0);   // M0
	m_view[4](0xe000, 0xe7ff).ram();                   // M1
	//m_view[4](0xe000, 0xefff).lrw8(                    // I/O Block
	//	NAME([this](offs_t offset) { return m_bus->read(offset | 0xe800); }),
	//	NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xe800, data); })
	//);
	m_view[4](0xee00, 0xeeff).lrw8(                    // I/O Page
		NAME([this](offs_t offset) { return m_bus->read(offset | 0xee00); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xee00, data); })
	);

	// Map 5 - FLEX (6809 only)
	m_view[5](0x0000, 0x3fff).ram();                   // M3
	m_view[5](0x4000, 0x7fff).ram();                   // M2
	m_view[5](0xe000, 0xffff).rom().region("m0", 0);   // M0
	m_view[5](0xe000, 0xe7ff).ram();                   // M1
	//m_view[5](0xe000, 0xefff).lrw8(                    // I/O Block
	//	NAME([this](offs_t offset) { return m_bus->read(offset | 0xe800); }),
	//	NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xe800, data); })
	//);
	m_view[5](0xee00, 0xeeff).lrw8(                    // I/O Page
		NAME([this](offs_t offset) { return m_bus->read(offset | 0xee00); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xee00, data); })
	);

	// Map 6 - Reserved for future use by Control Universal Ltd
	m_view[6](0x0000, 0xffff).noprw();

	// Map 7 - Reserved for future use by Control Universal Ltd
	m_view[7](0x0000, 0xffff).noprw();

	// Map 8 - ATOM BASIC
	m_view[8](0x0000, 0x07ff).ram();                   // M3
	m_view[8](0xa000, 0xbfff).ram();                   // M2
	m_view[8](0xc000, 0xdfff).rom().region("m1", 0);   // M1
	m_view[8](0xe000, 0xffff).rom().region("m0", 0);   // M0
	m_view[8](0x0800, 0x0fff).lrw8(                    // I/O Block
		NAME([this](offs_t offset) { return m_bus->read(offset | 0x0800); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0x0800, data); })
	);
	m_view[8](0xfe00, 0xfeff).lrw8(                    // I/O Page
		NAME([this](offs_t offset) { return m_bus->read(offset | 0xfe00); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xfe00, data); })
	);

	// Map 9 - Low-cost EPROM
	m_view[9](0x0000, 0x3fff).ram();                   // M3
	m_view[9](0xd000, 0xdfff).ram();                   // M2
	m_view[9](0xe000, 0xefff).rom().region("m1", 0);   // M1
	m_view[9](0xf000, 0xffff).rom().region("m0", 0);   // M0
	m_view[9](0x7000, 0x7fff).lrw8(                    // I/O Block
		NAME([this](offs_t offset) { return m_bus->read(offset | 0x7000); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0x7000, data); })
	);
	m_view[9](0xfe00, 0xfeff).lrw8(                    // I/O Page
		NAME([this](offs_t offset) { return m_bus->read(offset | 0xfe00); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xfe00, data); })
	);

	// Map 10 - ROM intensive
	m_view[10](0x0000, 0x3fff).ram();                  // M3
	m_view[10](0x4000, 0x7fff).rom().region("m2", 0);  // M2
	m_view[10](0x8000, 0xbfff).rom().region("m1", 0);  // M1
	m_view[10](0xc000, 0xffff).rom().region("m0", 0);  // M0
	m_view[10](0xfe00, 0xfeff).lrw8(                   // I/O Page
		NAME([this](offs_t offset) { return m_bus->read(offset | 0xfe00); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xfe00, data); })
	);

	// Map 11 - Low-cost RAM
	m_view[11](0x0000, 0x07ff).ram();                  // M3
	m_view[11](0x0800, 0x0fff).ram();                  // M2
	m_view[11](0x1000, 0x17ff).ram();                  // M1
	m_view[11](0x8000, 0xffff).rom().region("m0", 0);  // M0
	m_view[11](0xfe00, 0xfeff).lrw8(                   // I/O Page
		NAME([this](offs_t offset) { return m_bus->read(offset | 0xfe00); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xfe00, data); })
	);

	// Map 12 - RAM intensive
	m_view[12](0x0000, 0x1fff).ram();                  // M3
	m_view[12](0x2000, 0x3fff).ram();                  // M2
	m_view[12](0x4000, 0x5fff).rom().region("m1", 0);  // M1
	m_view[12](0x8000, 0xffff).rom().region("m0", 0);  // M0
	m_view[12](0x7000, 0x7fff).lrw8(                   // I/O Block
		NAME([this](offs_t offset) { return m_bus->read(offset | 0x7000); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0x7000, data); })
	);
	m_view[12](0xfe00, 0xfeff).lrw8(                   // I/O Page
		NAME([this](offs_t offset) { return m_bus->read(offset | 0xfe00); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xfe00, data); })
	);

	// Map 13 - Minimum configuration
	m_view[13](0x0000, 0x07ff).ram();                  // M3
	m_view[13](0x0800, 0x0fff).ram();                  // M2
	m_view[13](0xe000, 0xefff).rom().region("m1", 0);  // M1
	m_view[13](0xf000, 0xffff).rom().region("m0", 0);  // M0
	m_view[13](0xd000, 0xdfff).lrw8(                   // I/O Block
		NAME([this](offs_t offset) { return m_bus->read(offset | 0xd000); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xd000, data); })
	);
	m_view[13](0xfe00, 0xfeff).lrw8(                   // I/O Page
		NAME([this](offs_t offset) { return m_bus->read(offset | 0xfe00); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xfe00, data); })
	);

	// Map 14 - General purpose
	m_view[14](0x0000, 0x1fff).ram();                  // M3
	m_view[14](0x2000, 0x3fff).ram();                  // M2
	m_view[14](0x8000, 0xbfff).rom().region("m1", 0);  // M1
	m_view[14](0xc000, 0xffff).rom().region("m0", 0);  // M0
	m_view[14](0x7000, 0x7fff).lrw8(                   // I/O Block
		NAME([this](offs_t offset) { return m_bus->read(offset | 0x7000); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0x7000, data); })
	);
	m_view[14](0xfe00, 0xfeff).lrw8(                   // I/O Page
		NAME([this](offs_t offset) { return m_bus->read(offset | 0xfe00); }),
		NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xfe00, data); })
	);

	// Map 15 - Spare for user purposes
	m_view[15](0x0000, 0xffff).noprw();
}


static INPUT_PORTS_START( eurocube65 )
	PORT_START("map")
	PORT_CONFNAME(0x0f, 0x01, "Memory Map")
	PORT_CONFSETTING(0x00, "Development BASIC")
	PORT_CONFSETTING(0x01, "EPROM BASIC")
	PORT_CONFSETTING(0x02, "Minimum BASIC")
	PORT_CONFSETTING(0x09, "Low-cost EPROM")
	PORT_CONFSETTING(0x0a, "ROM intensive")
	PORT_CONFSETTING(0x0b, "Low-cost RAM")
	PORT_CONFSETTING(0x0c, "RAM intensive")
	PORT_CONFSETTING(0x0d, "Minimum configuration")
	PORT_CONFSETTING(0x0e, "General purpose")
INPUT_PORTS_END

static INPUT_PORTS_START( eurocube09 )
	PORT_START("map")
	PORT_CONFNAME(0x0f, 0x05, "Memory Map")
	PORT_CONFSETTING(0x04, "FLEX, low cost 6809 only")
	PORT_CONFSETTING(0x05, "FLEX 6809 only")
INPUT_PORTS_END


void eurocube_state::machine_start()
{
}


void eurocube_state::machine_reset()
{
	m_view.select(m_map->read());
}


static DEVICE_INPUT_DEFAULTS_START(terminal)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END


void eurocube_state::eurocube65(machine_config &config)
{
	M6502(config, m_maincpu, 4_MHz_XTAL / 4); // M6502
	m_maincpu->set_addrmap(AS_PROGRAM, &eurocube_state::eurocube65_mem);

	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	via6522_device &via(MOS6522(config, "via", 4_MHz_XTAL / 4));
	via.irq_handler().set("irqs", FUNC(input_merger_device::in_w<0>));

	mos6551_device &acia(MOS6551(config, "acia", 0));
	acia.set_xtal(1.8432_MHz_XTAL);
	acia.irq_handler().set("irqs", FUNC(input_merger_device::in_w<1>));
	acia.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	acia.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	//acia.dtr_handler().set(m_rs232, FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("acia", FUNC(mos6551_device::write_rxd));
	rs232.dsr_handler().set("acia", FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set("acia", FUNC(mos6551_device::write_cts));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	M3002(config, "rtc", 32.768_kHz_XTAL);

	// 4 Slot mini-rack
	ACORN_BUS(config, m_bus, 4_MHz_XTAL / 4);
	m_bus->out_irq_callback().set("irqs", FUNC(input_merger_device::in_w<2>));
	m_bus->out_nmi_callback().set_inputline(m_maincpu, M6502_NMI_LINE);
	ACORN_BUS_SLOT(config, "slot1", m_bus, eurocube_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "slot2", m_bus, eurocube_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "slot3", m_bus, eurocube_bus_devices, nullptr);

	//SOFTWARE_LIST(config, "flop_list").set_original("eurocube_flop");
	//SOFTWARE_LIST(config, "rom_list").set_original("eurocube_rom");
}

void eurocube_state::eurobeeb2(machine_config &config)
{
	eurocube65(config);

	subdevice<rs232_port_device>("rs232")->set_default_option("keyboard");

	subdevice<acorn_bus_slot_device>("slot1")->set_default_option("teletext");
}

void eurocube_state::eurobeeb3m(machine_config &config)
{
	eurocube65(config);

	subdevice<rs232_port_device>("rs232")->set_default_option("terminal");

	subdevice<acorn_bus_slot_device>("slot1")->set_default_option("cugraphm");
}

void eurocube_state::eurobeeb3c(machine_config &config)
{
	eurocube65(config);

	subdevice<rs232_port_device>("rs232")->set_default_option("terminal");

	subdevice<acorn_bus_slot_device>("slot1")->set_default_option("cugraphc");
}


void eurocube_state::eurocube09(machine_config &config)
{
	MC6809(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &eurocube_state::eurocube09_mem);

	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	via6522_device &via(MOS6522(config, "via", 4_MHz_XTAL / 4));
	via.irq_handler().set("irqs", FUNC(input_merger_device::in_w<0>));

	mos6551_device &acia(MOS6551(config, "acia", 0));
	acia.set_xtal(1.8432_MHz_XTAL);
	acia.irq_handler().set("irqs", FUNC(input_merger_device::in_w<1>));
	acia.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	acia.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	//acia.dtr_handler().set(m_rs232, FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("acia", FUNC(mos6551_device::write_rxd));
	rs232.dsr_handler().set("acia", FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set("acia", FUNC(mos6551_device::write_cts));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	M3002(config, "rtc", 32.768_kHz_XTAL); // Issue 5 board

	// 4 Slot mini-rack
	ACORN_BUS(config, m_bus, 4_MHz_XTAL / 4);
	m_bus->out_irq_callback().set("irqs", FUNC(input_merger_device::in_w<2>));
	m_bus->out_nmi_callback().set_inputline(m_maincpu, M6809_FIRQ_LINE);
	ACORN_BUS_SLOT(config, "slot1", m_bus, eurocube_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "slot2", m_bus, eurocube_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "slot3", m_bus, eurocube_bus_devices, nullptr);

	//SOFTWARE_LIST(config, "flop_list").set_original("eurocube_flop");
}


ROM_START( eurocube65 )
	ROM_REGION(0x8000, "m0", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "mosb361ms", "MOSB.3.6-1MS (Monitor)") // 1MHz
	ROMX_LOAD("mosb36-1ms.m0", 0x0000, 0x4000, CRC(538a7a9d) SHA1(afb13540f290b9ab35f410d6e3be8bd8561a974a), ROM_BIOS(0))

	ROM_REGION(0x4000, "m1", ROMREGION_ERASE00)

	ROM_REGION(0x4000, "m2", ROMREGION_ERASE00)
ROM_END

ROM_START( eurobeeb1 )
	ROM_REGION(0x8000, "m0", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "mosb361ms", "MOSB.3.6-1MS (Monitor)") // 1MHz
	ROMX_LOAD("mosb36-1ms.m0", 0x0000, 0x4000, CRC(538a7a9d) SHA1(afb13540f290b9ab35f410d6e3be8bd8561a974a), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "mos4c", "C-MOS 1.00-2MSB (Monitor)") // 2MHz
	ROMX_LOAD("mos4c.m0", 0x0000, 0x4000, CRC(075229e6) SHA1(baa22a9aba43b05fba91ccb870eac519303665b3), ROM_BIOS(1))

	ROM_REGION(0x4000, "m1", ROMREGION_ERASE00)
	ROMX_LOAD("basic2.m1", 0x0000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(0))
	ROMX_LOAD("basic4.m1", 0x0000, 0x4000, CRC(135cd65a) SHA1(1d433252be8e8133d8eaa5855bed0f0a6786e5c1), ROM_BIOS(1))

	ROM_REGION(0x4000, "m2", ROMREGION_ERASE00)
ROM_END

ROM_START( eurobeeb2 )
	ROM_REGION(0x8000, "m0", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "mosb3x", "MOSB.3 Oct 84 issue 1 (Teletext)")
	ROMX_LOAD("mosb3t.m0", 0x0000, 0x4000, CRC(9adc03bc) SHA1(15553d53802fddbfdb92f609a4cf8bb73e78380f), ROM_BIOS(0))

	ROM_REGION(0x4000, "m1", ROMREGION_ERASE00)
	ROM_LOAD("basic2.m1", 0x0000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))

	ROM_REGION(0x4000, "m2", ROMREGION_ERASE00)
ROM_END

ROM_START( eurobeeb3m )
	ROM_REGION(0x8000, "m0", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "mosb361c", "MOSB.3.6-1CK53S (CU-Graph/CU-Key53)") // 1MHz
	ROMX_LOAD("mosb3c.m0", 0x0000, 0x4000, CRC(d16f9a4a) SHA1(f64c9ef16aa09b6a8f018d1666560d0e5a283c9b), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "mosb419c", "MOSB.4.19 (CU-Graph/CU-Key99)")
	ROMX_LOAD("mosb419c.m0", 0x0000, 0x4000, CRC(38a3b7b4) SHA1(269f008fa92345c8c82ff0bbeb41a0f74511757a), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "mosb1", "MOSB.1 Dec 83 CU-graph issue 6 ")
	ROMX_LOAD("mosb1.m0", 0x0000, 0x2000, CRC(d4dc55fb) SHA1(0155b78353ebe481a7f0cf08eecc069cee48cf3b), ROM_BIOS(2))
	ROM_RELOAD(0x2000, 0x2000)
	ROM_SYSTEM_BIOS(3, "mosb400c", "Map1:MOSB.4.00 (CU-Graph/CU-Key99)")
	ROMX_LOAD("mosb400c.m0", 0x0000, 0x4000, CRC(6776dcd4) SHA1(cb620efd02f8a4095f0968ead5b7a625431e1ccb), ROM_BIOS(3))

	ROM_REGION(0x4000, "m1", ROMREGION_ERASE00)
	ROM_LOAD("basic2.m1", 0x0000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))
	ROMX_LOAD("basic4.m1", 0x0000, 0x4000, CRC(135cd65a) SHA1(1d433252be8e8133d8eaa5855bed0f0a6786e5c1), ROM_BIOS(3))

	ROM_REGION(0x4000, "m2", ROMREGION_ERASE00)
	ROMX_LOAD("2287sl-iss-d.m2", 0x0000, 0x4000, CRC(3db9e748) SHA1(96941aeaac5115363218a6b1fd71e6eddf3e71d0), ROM_BIOS(3))
ROM_END

#define rom_eurobeeb3c rom_eurobeeb3m

ROM_START( eurocube09 )
	ROM_REGION(0x8000, "m0", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "mosf31", "MOSF.3:1") // 1MHz
	ROMX_LOAD("mosf31.m0", 0x0000, 0x2000, CRC(c9b5c6b5) SHA1(c520b6271d098ac0c3c5aa36baa985030bb0bc76), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "mosf24", "MOSF.2:4")
	ROMX_LOAD("mosf24.m0", 0x0000, 0x2000, CRC(76b4d5a7) SHA1(61341d37d62b4810896f4cf74561cdc09cd8eba8), ROM_BIOS(1))

	ROM_REGION(0x4000, "m1", ROMREGION_ERASE00)

	ROM_REGION(0x4000, "m2", ROMREGION_ERASE00)
ROM_END

} // anonymous namespace


//    YEAR  NAME         PARENT       COMPAT  MACHINE      INPUT        CLASS            INIT         COMPANY               FULLNAME                             FLAGS
COMP( 1982, eurocube65,  0,           0,      eurocube65,  eurocube65,  eurocube_state,  empty_init,  "Control Universal",  "EuroCUBE-65",                       MACHINE_NO_SOUND_HW )
COMP( 1984, eurobeeb1,   eurocube65,  0,      eurocube65,  eurocube65,  eurocube_state,  empty_init,  "Control Universal",  "EuroBEEB-1",                        MACHINE_NO_SOUND_HW )
COMP( 1984, eurobeeb2,   eurocube65,  0,      eurobeeb2,   eurocube65,  eurocube_state,  empty_init,  "Control Universal",  "EuroBEEB-2 (Teletext)",             MACHINE_NO_SOUND_HW )
COMP( 1984, eurobeeb3m,  eurocube65,  0,      eurobeeb3m,  eurocube65,  eurocube_state,  empty_init,  "Control Universal",  "EuroBEEB-3M (CU-Graph monochrome)", MACHINE_NO_SOUND_HW )
COMP( 1984, eurobeeb3c,  eurocube65,  0,      eurobeeb3c,  eurocube65,  eurocube_state,  empty_init,  "Control Universal",  "EuroBEEB-3C (CU-Graph colour)",     MACHINE_NO_SOUND_HW )
COMP( 1983, eurocube09,  0,           0,      eurocube09,  eurocube09,  eurocube_state,  empty_init,  "Control Universal",  "EuroCUBE-09",                       MACHINE_NO_SOUND_HW )

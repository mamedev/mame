// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    mupid M-Disk Comp.-A
    Grundig FL-100

    Floppy disk station, usually connected to a mupid C2D2/C2A2 or the
    Grundig PTC-100.

    Everything here is guessed based on the software and a PCB image. You
    can see garbled output when you connect the builtin terminal to the
    first serial port ('ser'). It does boot from floppy too, so presumely
    only needs an emulated main system to work.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/i8271.h"
#include "machine/ram.h"
#include "bus/rs232/rs232.h"
#include "imagedev/floppy.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mdisk_state : public driver_device
{
public:
	mdisk_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cpu(*this, "maincpu"),
		m_ram(*this, "ram"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%u", 0),
		m_rom_timer(nullptr),
		m_uart1_rxrdy(0), m_uart1_txrdy(0),
		m_fdc_irq(0)
		{ }

		void mdisk(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(rom_timer_callback);
	void uart1_rxrdy_w(int state);
	void uart1_txrdy_w(int state);
	void fdc_irq_w(int state);
	void fdc_motor_w(int state);
	void fdc_side_w(uint8_t data);

	void update_irq(uint8_t vector);

	required_device<z80_device> m_cpu;
	required_device<ram_device> m_ram;
	required_device<i8271_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;

	void mdisk_mem(address_map &map) ATTR_COLD;
	void mdisk_io(address_map &map) ATTR_COLD;

	emu_timer *m_rom_timer;

	int m_uart1_rxrdy, m_uart1_txrdy, m_fdc_irq;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void mdisk_state::mdisk_mem(address_map &map)
{
	map(0x0000, 0x1fff).bankr("bank1r").bankw("bank1w");
	map(0x2000, 0xffff).bankrw("bank2");
}

void mdisk_state::mdisk_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x20, 0x20).w(FUNC(mdisk_state::fdc_side_w));
	map(0x40, 0x40).rw("fdc", FUNC(i8271_device::data_r), FUNC(i8271_device::data_w));
	map(0x80, 0x81).rw("uart1", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x84, 0x85).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x9c, 0x9e).m(m_fdc, FUNC(i8271_device::map));
}


//**************************************************************************
//  INPUTS
//**************************************************************************

INPUT_PORTS_START( mdisk )
	PORT_START("front")
	PORT_DIPNAME(0x01, 0x01, "Baudrate")
	PORT_DIPSETTING(0x00, "4800")
	PORT_DIPSETTING(0x01, "19200")
INPUT_PORTS_END


//**************************************************************************
//  FLOPPY
//**************************************************************************

void mdisk_state::fdc_motor_w(int state)
{
	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->mon_w(!state);
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->mon_w(!state);
}

void mdisk_state::fdc_side_w(uint8_t data)
{
	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->ss_w(BIT(data, 0));
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->ss_w(BIT(data, 0));
}


//**************************************************************************
//  MACHINE
//**************************************************************************

void mdisk_state::machine_start()
{
	// timer to switch rom to ram
	m_rom_timer = timer_alloc(FUNC(mdisk_state::rom_timer_callback), this);

	// register for save states
	save_item(NAME(m_uart1_rxrdy));
	save_item(NAME(m_uart1_txrdy));
	save_item(NAME(m_fdc_irq));
}

void mdisk_state::machine_reset()
{
	// read from rom, write to ram
	membank("bank1r")->set_base(memregion("firmware")->base());
	membank("bank1w")->set_base(m_ram->pointer());
	membank("bank2")->set_base(m_ram->pointer() + 0x2000);

	// timing unknown
	m_rom_timer->adjust(attotime::from_msec(50));

	m_uart1_rxrdy = 0;
	m_uart1_txrdy = 0;
	m_fdc_irq = 0;
}

TIMER_CALLBACK_MEMBER(mdisk_state::rom_timer_callback)
{
	// switch in ram
	logerror("Disabling ROM\n");
	membank("bank1r")->set_base(m_ram->pointer());
}

void mdisk_state::update_irq(uint8_t vector)
{
	if (m_uart1_rxrdy || m_uart1_txrdy || m_fdc_irq)
		m_cpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, vector); // Z80
	else
		m_cpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

void mdisk_state::fdc_irq_w(int state)
{
	m_fdc_irq = state;
	update_irq(0x00);
}

void mdisk_state::uart1_rxrdy_w(int state)
{
	m_uart1_rxrdy = state;
	update_irq(0x18);
}

void mdisk_state::uart1_txrdy_w(int state)
{
	m_uart1_txrdy = state;
	update_irq(0x1c);
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

static void mdisk_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void mdisk_state::mdisk(machine_config &config)
{
	Z80(config, m_cpu, 4_MHz_XTAL);
	m_cpu->set_addrmap(AS_PROGRAM, &mdisk_state::mdisk_mem);
	m_cpu->set_addrmap(AS_IO, &mdisk_state::mdisk_io);

	// 64k internal ram
	RAM(config, m_ram).set_default_size("64K");

	// uart 1
	i8251_device &uart1(I8251(config, "uart1", 4_MHz_XTAL));
	uart1.rxrdy_handler().set(FUNC(mdisk_state::uart1_rxrdy_w));
	uart1.txrdy_handler().set(FUNC(mdisk_state::uart1_txrdy_w));
	uart1.txd_handler().set("ser", FUNC(rs232_port_device::write_txd));
	uart1.rts_handler().set("ser", FUNC(rs232_port_device::write_rts));
	uart1.dtr_handler().set("ser", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232_1(RS232_PORT(config, "ser", default_rs232_devices, nullptr));
	rs232_1.rxd_handler().set("uart1", FUNC(i8251_device::write_rxd));
	rs232_1.cts_handler().set("uart1", FUNC(i8251_device::write_cts));
	rs232_1.dsr_handler().set("uart1", FUNC(i8251_device::write_dsr));

	// uart 2
	i8251_device &uart2(I8251(config, "uart2", 4_MHz_XTAL));
	uart2.txd_handler().set("aux", FUNC(rs232_port_device::write_txd));
	uart2.rts_handler().set("aux", FUNC(rs232_port_device::write_rts));
	uart2.dtr_handler().set("aux", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232_2(RS232_PORT(config, "aux", default_rs232_devices, nullptr));
	rs232_2.rxd_handler().set("uart2", FUNC(i8251_device::write_rxd));
	rs232_2.cts_handler().set("uart2", FUNC(i8251_device::write_cts));
	rs232_2.dsr_handler().set("uart2", FUNC(i8251_device::write_dsr));

	// selectable 4800 or 19200
	clock_device &uart_clock(CLOCK(config, "uart_clock", 4.9152_MHz_XTAL / 32)); // just 9600 for now
	uart_clock.signal_handler().set("uart1", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("uart1", FUNC(i8251_device::write_rxc));
	uart_clock.signal_handler().append("uart2", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("uart2", FUNC(i8251_device::write_rxc));

	// floppy
	I8271(config, m_fdc, 4_MHz_XTAL);
	m_fdc->set_ready_line_connected(true);
	m_fdc->intrq_wr_callback().set(FUNC(mdisk_state::fdc_irq_w));
	m_fdc->drq_wr_callback().set_inputline(m_cpu, INPUT_LINE_NMI);
	m_fdc->hdl_wr_callback().set(FUNC(mdisk_state::fdc_motor_w));
	FLOPPY_CONNECTOR(config, m_floppy[0], mdisk_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], mdisk_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( mdisk )
	ROM_REGION(0x2000, "firmware", 0)
	ROM_LOAD("motronic_0090_6815_16.00.bin", 0x0000, 0x2000, CRC(931b3410) SHA1(4593708268d5ef7ffcb330f91218fc0c845abf5e))
ROM_END

ROM_START( fl100 )
	ROM_REGION(0x2000, "firmware", 0)
	ROM_LOAD("fl100.bin", 0x0000, 0x2000, CRC(68800982) SHA1(501c8877a18cef091476b780de605b2bea3853fb))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY    FULLNAME          FLAGS
COMP( 198?, mdisk, 0,      0,      mdisk,   mdisk, mdisk_state, empty_init, "mupid",   "M-Disk Comp.-A", MACHINE_NOT_WORKING )
COMP( 198?, fl100, mdisk,  0,      mdisk,   mdisk, mdisk_state, empty_init, "Grundig", "FL-100",         MACHINE_NOT_WORKING )

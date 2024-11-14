// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
// thanks-to:Andrew Trotman
/**********************************************************************

    Poly Proteus Computer

    http://www.cs.otago.ac.nz/homepages/andrew/poly/Poly.htm

    Specifications:

      Processors     6809,Z80

      RAM            64 Kbytes

      ROM            4 Kbytes

      Clock          Z80 4 MHz (1 wait state per machine cycle)
                     6809 4 MHz clock, 1 MHz cycle

      Disk Drives    One or two 8" half height drives (Shugart SA860)
                     Double sided single density
                     Capacity 630 Kbytes per disk (CP/M)
                              590 Kbytes (Flex,Polysys)
                     Single sided can be used for compatibility

      Ports          Standard One RS232
                              One Poly network
                     Optional Total 3 RS232
                              One Poly network
                              One parallel
                              One disk drive extension

      Operating systems
                     CP/M. The standard operating system for 8080/Z80 micros.
                     FLEX. The standard operating system for 6809 micros.
                     POLYSYS. The operating system for the POLY network.

**********************************************************************/


#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/mc6854.h"
#include "machine/ram.h"
#include "machine/input_merger.h"
#include "machine/clock.h"
#include "machine/wd_fdc.h"
#include "bus/rs232/rs232.h"
#include "bus/centronics/ctronics.h"
#include "formats/flex_dsk.h"
#include "formats/poly_dsk.h"
#include "softlist_dev.h"


namespace {

class proteus_state : public driver_device
{
public:
	proteus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_z80(*this, "z80")
		, m_adlc(*this, "mc6854")
		, m_ptm(*this, "ptm")
		, m_irqs(*this, "irqs")
		, m_pia(*this, "pia")
		, m_acia(*this, "acia%u", 0)
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_floppy(nullptr)
	{ }

	void proteus(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

	void ptm_o2_callback(int state);
	void ptm_o3_callback(int state);
	uint8_t network_r(offs_t offset);
	void network_w(offs_t offset, uint8_t data);

	uint8_t drive_register_r();
	void drive_register_w(uint8_t data);
	void motor_w(int state);
	uint8_t fdc_inv_r(offs_t offset);
	void fdc_inv_w(offs_t offset, uint8_t data);
	static void floppy_formats(format_registration &fr);

	void enable_z80_w(uint8_t data);
	void enable_6809_w(uint8_t data);

	void proteus_6809_mem(address_map &map) ATTR_COLD;
	void proteus_z80_mem(address_map &map) ATTR_COLD;
	void proteus_z80_io(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_z80;
	required_device<mc6854_device> m_adlc;
	required_device<ptm6840_device> m_ptm;
	required_device<input_merger_device> m_irqs;
	required_device<pia6821_device> m_pia;
	required_device_array<acia6850_device, 3> m_acia;
	required_device<fd1771_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy = nullptr;
};


void proteus_state::machine_start()
{
	uint8_t bitswapped[0x1000];
	uint8_t *rom;

	/* bios rom region */
	rom = memregion("bios")->base();

	/* decrypt rom data lines */
	for (int i = 0x0000; i<0x1000; i++)
		bitswapped[i] = bitswap<8>(rom[i], 3, 4, 2, 5, 1, 6, 0, 7);

	/* decrypt rom address lines */
	for (int i = 0x0000; i<0x1000; i++)
		rom[i] = bitswapped[bitswap<16>(i, 15, 14, 13, 12, 10, 8, 4, 2, 0, 1, 3, 5, 6, 7, 9, 11)];

	/*
	    The Proteus BIOS contains a serial ID for copy protection. Disks also contain the
	    OS serial ID which must match the BIOS serial ID. This can be negated by patching the
	    serial in the BIOS to 0x0000.
	*/
	rom[0x02] = 0x00;
	rom[0x03] = 0x00;
}


void proteus_state::machine_reset()
{
	/* system starts with 6809 running and the Z80 remains reset */
	m_z80->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}


void proteus_state::ptm_o2_callback(int state)
{
	m_ptm->set_c1(state);
}

void proteus_state::ptm_o3_callback(int state)
{
	m_adlc->txc_w(state);
	m_adlc->rxc_w(!state);
}


uint8_t proteus_state::network_r(offs_t offset)
{
	return m_adlc->read(offset >> 1);
}

void proteus_state::network_w(offs_t offset, uint8_t data)
{
	m_adlc->write(offset >> 1, data);
}


void proteus_state::drive_register_w(uint8_t data)
{
	/* drive select */
	switch (data & 0x03)
	{
	case 0:
		m_floppy = m_floppy0->get_device();
		break;

	case 1:
		m_floppy = m_floppy1->get_device();
		break;

	default:
		m_floppy = nullptr;
		break;
	}
	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		/* side select */
		m_floppy->ss_w(BIT(data, 6));
	}
}

uint8_t proteus_state::drive_register_r()
{
	/* disk change */
	return (m_floppy ? m_floppy->dskchg_r() : 1) << 1;
}

void proteus_state::motor_w(int state)
{
	if (m_floppy) m_floppy->mon_w(!state);
}

uint8_t proteus_state::fdc_inv_r(offs_t offset)
{
	return m_fdc->read(offset) ^ 0xff;
}

void proteus_state::fdc_inv_w(offs_t offset, uint8_t data)
{
	m_fdc->write(offset, data ^ 0xff);
}


void proteus_state::enable_z80_w(uint8_t data)
{
	logerror("%04X enable z80\n", m_maincpu->pc());
	// Enable Z80 (store an address at E060 and when the 6809 comes back it'll load PC with that address)
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_z80->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

void proteus_state::enable_6809_w(uint8_t data)
{
	logerror("%04X enable 6809\n", m_z80->pc());
	// TODO: this is untested, not aware of any software that uses it
	m_z80->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
}


void proteus_state::proteus_6809_mem(address_map &map)
{
	map(0x0000, 0xffff).ram().share(RAM_TAG);
	map(0xe000, 0xe003).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));       // Parallel port
	map(0xe004, 0xe005).rw(m_acia[0], FUNC(acia6850_device::read), FUNC(acia6850_device::write)); // Terminal
	map(0xe008, 0xe009).rw(m_acia[1], FUNC(acia6850_device::read), FUNC(acia6850_device::write)); // Printer
	map(0xe00c, 0xe00d).rw(m_acia[2], FUNC(acia6850_device::read), FUNC(acia6850_device::write)); // Modem
	map(0xe014, 0xe014).rw(FUNC(proteus_state::drive_register_r), FUNC(proteus_state::drive_register_w));
	map(0xe018, 0xe01b).rw(FUNC(proteus_state::fdc_inv_r), FUNC(proteus_state::fdc_inv_w)); // Floppy control
	map(0xe020, 0xe027).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));       // Timer
	map(0xe030, 0xe036).rw(FUNC(proteus_state::network_r), FUNC(proteus_state::network_w)); // Poly network
	map(0xe060, 0xe060).w(FUNC(proteus_state::enable_z80_w));
	map(0xf000, 0xffff).rom().region("bios", 0);
}


void proteus_state::proteus_z80_mem(address_map &map)
{
	map(0x0000, 0xffff).ram().share(RAM_TAG);
}


void proteus_state::proteus_z80_io(address_map &map)
{
	map(0x00, 0x03).mirror(0xff00).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));       // Parallel port
	map(0x04, 0x05).mirror(0xff00).rw(m_acia[0], FUNC(acia6850_device::read), FUNC(acia6850_device::write)); // Terminal
	map(0x08, 0x09).mirror(0xff00).rw(m_acia[1], FUNC(acia6850_device::read), FUNC(acia6850_device::write)); // Printer
	map(0x0c, 0x0d).mirror(0xff00).rw(m_acia[2], FUNC(acia6850_device::read), FUNC(acia6850_device::write)); // Modem
	map(0x14, 0x14).mirror(0xff00).rw(FUNC(proteus_state::drive_register_r), FUNC(proteus_state::drive_register_w));
	map(0x18, 0x1b).mirror(0xff00).rw(FUNC(proteus_state::fdc_inv_r), FUNC(proteus_state::fdc_inv_w)); // Floppy control
	map(0x20, 0x27).mirror(0xff00).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));       // Timer
	map(0x30, 0x36).mirror(0xff00).rw(FUNC(proteus_state::network_r), FUNC(proteus_state::network_w)); // Poly network
	map(0x50, 0x50).mirror(0xff00).w(FUNC(proteus_state::enable_6809_w));
}


//Parameter 1: Baud Rate from BIOS
//  00 = 9600
//  02 = 4800
//  04 = 2400
//  06 = 1200
//  08 = 600
//  0A = 300
static INPUT_PORTS_START(proteus)
	PORT_START("TERM_BAUD")
	PORT_CONFNAME(0x07, 0x00, "Terminal Baud Rate")
	PORT_CONFSETTING(0x00, "9600")
	PORT_CONFSETTING(0x01, "4800")
	PORT_CONFSETTING(0x02, "2400")
	PORT_CONFSETTING(0x03, "1200")
	PORT_CONFSETTING(0x04, "600")
	PORT_CONFSETTING(0x05, "300")
	PORT_CONFSETTING(0x06, "150")

	PORT_START("J1")
	PORT_CONFNAME(0x07, 0x00, "J1 Modem Baud Rate")
	PORT_CONFSETTING(0x00, "9600")
	PORT_CONFSETTING(0x01, "4800")
	PORT_CONFSETTING(0x02, "2400")
	PORT_CONFSETTING(0x03, "1200")
	PORT_CONFSETTING(0x04, "600")
	PORT_CONFSETTING(0x05, "300")

	PORT_START("J2")
	PORT_CONFNAME(0x07, 0x00, "J2 Printer Baud Rate")
	PORT_CONFSETTING(0x00, "9600")
	PORT_CONFSETTING(0x01, "4800")
	PORT_CONFSETTING(0x02, "2400")
	PORT_CONFSETTING(0x03, "1200")
	PORT_CONFSETTING(0x04, "600")
	PORT_CONFSETTING(0x05, "300")
INPUT_PORTS_END


void proteus_state::floppy_formats(format_registration &fr)
{
	fr.add_fm_containers();
	fr.add(FLOPPY_FLEX_FORMAT);
	fr.add(FLOPPY_POLY_CPM_FORMAT);
}

static void proteus_floppies(device_slot_interface &device)
{
	device.option_add("8dssd", FLOPPY_8_DSSD); // Shugart SA-860
}


void proteus_state::proteus(machine_config &config)
{
	/* basic machine hardware */
	MC6809(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &proteus_state::proteus_6809_mem);

	Z80(config, m_z80, 4_MHz_XTAL);
	m_z80->set_addrmap(AS_PROGRAM, &proteus_state::proteus_z80_mem);
	m_z80->set_addrmap(AS_IO, &proteus_state::proteus_z80_io);

	INPUT_MERGER_ANY_HIGH(config, m_irqs);
	m_irqs->output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);
	m_irqs->output_handler().append_inputline(m_z80, INPUT_LINE_IRQ0);

	/* fdc */
	FD1771(config, m_fdc, 4_MHz_XTAL / 2);
	m_fdc->hld_wr_callback().set(FUNC(proteus_state::motor_w));
	m_fdc->set_force_ready(true);

	FLOPPY_CONNECTOR(config, "fdc:0", proteus_floppies, "8dssd", proteus_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", proteus_floppies, "8dssd", proteus_state::floppy_formats).enable_sound(true);

	/* ram */
	RAM(config, RAM_TAG).set_default_size("64K");

	/* network */
	MC6854(config, m_adlc);
	m_adlc->out_irq_cb().set("irqs", FUNC(input_merger_device::in_w<0>));

	PTM6840(config, m_ptm, 4_MHz_XTAL / 2);
	m_ptm->set_external_clocks(0, 0, 0);
	m_ptm->o2_callback().set(FUNC(proteus_state::ptm_o2_callback));
	m_ptm->o3_callback().set(FUNC(proteus_state::ptm_o3_callback));
	m_ptm->irq_callback().set("irqs", FUNC(input_merger_device::in_w<1>));

	/* parallel port */
	PIA6821(config, m_pia);
	//m_pia->readpb_handler().set(FUNC(proteus_state::pia_pb_r));
	//m_pia->writepa_handler().set(FUNC(proteus_state::pia_pa_w));
	//m_pia->writepb_handler().set(FUNC(proteus_state::pia_pb_w));
	//m_pia->ca2_handler().set(CENTRONICS_TAG, FUNC(centronics_device::write_strobe));
	m_pia->irqa_handler().set("irqs", FUNC(input_merger_device::in_w<2>));
	m_pia->irqb_handler().set("irqs", FUNC(input_merger_device::in_w<3>));

	centronics_device &parallel(CENTRONICS(config, "parallel", centronics_devices, "printer"));
	parallel.ack_handler().set(m_pia, FUNC(pia6821_device::ca1_w));
	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	parallel.set_output_latch(cent_data_out);

	/* terminal port */
	ACIA6850(config, m_acia[0], 0);
	m_acia[0]->txd_handler().set("terminal", FUNC(rs232_port_device::write_txd));
	m_acia[0]->rts_handler().set("terminal", FUNC(rs232_port_device::write_rts));
	m_acia[0]->irq_handler().set("irqs", FUNC(input_merger_device::in_w<4>));

	rs232_port_device &terminal(RS232_PORT(config, "terminal", default_rs232_devices, "terminal")); // TODO: ADM-31 terminal required
	terminal.rxd_handler().set(m_acia[0], FUNC(acia6850_device::write_rxd));
	terminal.cts_handler().set(m_acia[0], FUNC(acia6850_device::write_cts));

	clock_device &acia0_clock(CLOCK(config, "acia0_clock", 4_MHz_XTAL / 2 / 13)); // TODO: this is set using jumpers
	acia0_clock.signal_handler().set(m_acia[0], FUNC(acia6850_device::write_txc));
	acia0_clock.signal_handler().append(m_acia[0], FUNC(acia6850_device::write_rxc));

	/* printer port */
	ACIA6850(config, m_acia[1], 0);
	m_acia[1]->txd_handler().set("printer", FUNC(rs232_port_device::write_txd));
	m_acia[1]->rts_handler().set("printer", FUNC(rs232_port_device::write_rts));
	m_acia[1]->irq_handler().set("irqs", FUNC(input_merger_device::in_w<5>));

	rs232_port_device &printer(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	printer.rxd_handler().set(m_acia[1], FUNC(acia6850_device::write_rxd));
	printer.cts_handler().set(m_acia[1], FUNC(acia6850_device::write_cts));

	clock_device &acia1_clock(CLOCK(config, "acia1_clock", 4_MHz_XTAL / 2 / 13)); // TODO: this is set using jumpers J2
	acia1_clock.signal_handler().set(m_acia[1], FUNC(acia6850_device::write_txc));
	acia1_clock.signal_handler().append(m_acia[1], FUNC(acia6850_device::write_rxc));

	/* modem port */
	ACIA6850(config, m_acia[2], 0);
	m_acia[2]->txd_handler().set("modem", FUNC(rs232_port_device::write_txd));
	m_acia[2]->rts_handler().set("modem", FUNC(rs232_port_device::write_rts));
	m_acia[2]->irq_handler().set("irqs", FUNC(input_merger_device::in_w<6>));

	rs232_port_device &modem(RS232_PORT(config, "modem", default_rs232_devices, nullptr));
	modem.rxd_handler().set(m_acia[2], FUNC(acia6850_device::write_rxd));
	modem.cts_handler().set(m_acia[2], FUNC(acia6850_device::write_cts));

	clock_device &acia2_clock(CLOCK(config, "acia2_clock", 4_MHz_XTAL / 2 / 13)); // TODO: this is set using jumpers J1
	acia2_clock.signal_handler().set(m_acia[2], FUNC(acia6850_device::write_txc));
	acia2_clock.signal_handler().append(m_acia[2], FUNC(acia6850_device::write_rxc));

	/* software lists */
	SOFTWARE_LIST(config, "flop_list").set_original("poly_flop").set_filter("PROTEUS");
}


ROM_START(proteus)
	ROM_REGION(0x1000, "bios", 0)
	ROM_DEFAULT_BIOS("82")
	ROM_SYSTEM_BIOS(0, "82", "030982")
	ROMX_LOAD("proteusv30.u28", 0x0000, 0x1000, CRC(ad02dc36) SHA1(f73aff8b3d85448f603a2fe807e3a7e02550db27), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "86", "300986")
	ROMX_LOAD("300986.u28", 0x0000, 0x1000, CRC(f607d396) SHA1(0b9d9d3178b5587e60da56885b9e8a83f7d5dd26), ROM_BIOS(1))
ROM_END

} // anonymous namespace


/*    YEAR  NAME     PARENT  COMPAT   MACHINE  INPUT    CLASS          INIT        COMPANY     FULLNAME                     FLAGS */
COMP( 1982, proteus, 0,      0,       proteus, proteus, proteus_state, empty_init, "Polycorp", "Poly Proteus (Standalone)", MACHINE_NOT_WORKING )

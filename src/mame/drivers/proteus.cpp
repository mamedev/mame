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
#include "softlist.h"


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

	virtual void machine_reset() override;
	virtual void machine_start() override;

	DECLARE_WRITE_LINE_MEMBER(ptm_o2_callback);
	DECLARE_WRITE_LINE_MEMBER(ptm_o3_callback);
	DECLARE_READ8_MEMBER(network_r);
	DECLARE_WRITE8_MEMBER(network_w);

	DECLARE_READ8_MEMBER(drive_register_r);
	DECLARE_WRITE8_MEMBER(drive_register_w);
	DECLARE_WRITE_LINE_MEMBER(motor_w);
	DECLARE_READ8_MEMBER(fdc_inv_r);
	DECLARE_WRITE8_MEMBER(fdc_inv_w);
	DECLARE_FLOPPY_FORMATS(floppy_formats);

	DECLARE_WRITE8_MEMBER(enable_z80_w);
	DECLARE_WRITE8_MEMBER(enable_6809_w);

	void proteus(machine_config &config);
	void proteus_6809_mem(address_map &map);
	void proteus_z80_mem(address_map &map);
	void proteus_z80_io(address_map &map);

protected:
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
	floppy_image_device *m_floppy;
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


WRITE_LINE_MEMBER(proteus_state::ptm_o2_callback)
{
	m_ptm->set_c1(state);
}

WRITE_LINE_MEMBER(proteus_state::ptm_o3_callback)
{
	m_adlc->txc_w(state);
	m_adlc->rxc_w(!state);
}


READ8_MEMBER(proteus_state::network_r)
{
	return m_adlc->read(space, offset >> 1);
}

WRITE8_MEMBER(proteus_state::network_w)
{
	m_adlc->write(space, offset >> 1, data);
}


WRITE8_MEMBER(proteus_state::drive_register_w)
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

READ8_MEMBER(proteus_state::drive_register_r)
{
	/* disk change */
	return (m_floppy ? m_floppy->dskchg_r() : 1) << 1;
}

WRITE_LINE_MEMBER(proteus_state::motor_w)
{
	if (m_floppy) m_floppy->mon_w(!state);
}

READ8_MEMBER(proteus_state::fdc_inv_r)
{
	return m_fdc->gen_r(offset) ^ 0xff;
}

WRITE8_MEMBER(proteus_state::fdc_inv_w)
{
	m_fdc->gen_w(offset, data ^ 0xff);
}


WRITE8_MEMBER(proteus_state::enable_z80_w)
{
	logerror("%04X enable z80\n", m_maincpu->pc());
	// Enable Z80 (store an address at E060 and when the 6809 comes back it'll load PC with that address)
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_z80->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

WRITE8_MEMBER(proteus_state::enable_6809_w)
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
	map(0xe014, 0xe014).rw(this, FUNC(proteus_state::drive_register_r), FUNC(proteus_state::drive_register_w));
	map(0xe018, 0xe01b).rw(this, FUNC(proteus_state::fdc_inv_r), FUNC(proteus_state::fdc_inv_w)); // Floppy control
	map(0xe020, 0xe027).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));       // Timer
	map(0xe030, 0xe036).rw(this, FUNC(proteus_state::network_r), FUNC(proteus_state::network_w)); // Poly network
	map(0xe060, 0xe060).w(this, FUNC(proteus_state::enable_z80_w));
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
	map(0x14, 0x14).mirror(0xff00).rw(this, FUNC(proteus_state::drive_register_r), FUNC(proteus_state::drive_register_w));
	map(0x18, 0x1b).mirror(0xff00).rw(this, FUNC(proteus_state::fdc_inv_r), FUNC(proteus_state::fdc_inv_w)); // Floppy control
	map(0x20, 0x27).mirror(0xff00).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));       // Timer
	map(0x30, 0x36).mirror(0xff00).rw(this, FUNC(proteus_state::network_r), FUNC(proteus_state::network_w)); // Poly network
	map(0x50, 0x50).mirror(0xff00).w(this, FUNC(proteus_state::enable_6809_w));
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


FLOPPY_FORMATS_MEMBER(proteus_state::floppy_formats)
	FLOPPY_FLEX_FORMAT,
	FLOPPY_POLY_CPM_FORMAT
FLOPPY_FORMATS_END

static void proteus_floppies(device_slot_interface &device)
{
	device.option_add("8dssd", FLOPPY_8_DSSD); // Shugart SA-860
}


MACHINE_CONFIG_START(proteus_state::proteus)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", MC6809, 4_MHz_XTAL)
	MCFG_DEVICE_PROGRAM_MAP(proteus_6809_mem)

	MCFG_DEVICE_ADD("z80", Z80, 4_MHz_XTAL)
	MCFG_DEVICE_PROGRAM_MAP(proteus_z80_mem)
	MCFG_DEVICE_IO_MAP(proteus_z80_io)

	MCFG_INPUT_MERGER_ANY_HIGH("irqs")
	MCFG_INPUT_MERGER_OUTPUT_HANDLER(INPUTLINE("maincpu", M6809_IRQ_LINE))
	MCFG_DEVCB_CHAIN_OUTPUT(INPUTLINE("z80", INPUT_LINE_IRQ0))

	/* fdc */
	MCFG_FD1771_ADD("fdc", 4_MHz_XTAL / 2)
	MCFG_WD_FDC_HLD_CALLBACK(WRITELINE(*this, proteus_state, motor_w))
	MCFG_WD_FDC_FORCE_READY

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", proteus_floppies, "8dssd", proteus_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", proteus_floppies, "8dssd", proteus_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	/* ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")

	/* network */
	MCFG_DEVICE_ADD("mc6854", MC6854, 0)
	MCFG_MC6854_OUT_IRQ_CB(WRITELINE("irqs", input_merger_device, in_w<0>))

	MCFG_DEVICE_ADD("ptm", PTM6840, 4_MHz_XTAL / 2)
	MCFG_PTM6840_EXTERNAL_CLOCKS(0, 0, 0)
	MCFG_PTM6840_O2_CB(WRITELINE(*this, proteus_state, ptm_o2_callback))
	MCFG_PTM6840_O3_CB(WRITELINE(*this, proteus_state, ptm_o3_callback))
	MCFG_PTM6840_IRQ_CB(WRITELINE("irqs", input_merger_device, in_w<1>))

	/* parallel port */
	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	//MCFG_PIA_READPB_HANDLER(READ8(*this, proteus_state, pia_pb_r))
	//MCFG_PIA_WRITEPA_HANDLER(WRITE8(*this, proteus_state, pia_pa_w))
	//MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, proteus_state, pia_pb_w))
	//MCFG_PIA_CA2_HANDLER(WRITELINE(CENTRONICS_TAG, centronics_device, write_strobe))
	MCFG_PIA_IRQA_HANDLER(WRITELINE("irqs", input_merger_device, in_w<2>))
	MCFG_PIA_IRQB_HANDLER(WRITELINE("irqs", input_merger_device, in_w<3>))

	MCFG_CENTRONICS_ADD("parallel", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE("pia", pia6821_device, ca1_w))
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "parallel")

	/* terminal port */
	MCFG_DEVICE_ADD("acia0", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE("terminal", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(WRITELINE("terminal", rs232_port_device, write_rts))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE("irqs", input_merger_device, in_w<4>))

	MCFG_DEVICE_ADD("terminal", RS232_PORT, default_rs232_devices, "terminal") // TODO: ADM-31 terminal required
	MCFG_RS232_RXD_HANDLER(WRITELINE("acia0", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(WRITELINE("acia0", acia6850_device, write_cts))

	MCFG_DEVICE_ADD("acia0_clock", CLOCK, 4_MHz_XTAL / 2 / 13) // TODO: this is set using jumpers
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE("acia0", acia6850_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("acia0", acia6850_device, write_rxc))

	/* printer port */
	MCFG_DEVICE_ADD("acia1", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE("printer", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(WRITELINE("printer", rs232_port_device, write_rts))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE("irqs", input_merger_device, in_w<5>))

	MCFG_DEVICE_ADD("printer", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("acia1", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(WRITELINE("acia1", acia6850_device, write_cts))

	MCFG_DEVICE_ADD("acia1_clock", CLOCK, 4_MHz_XTAL / 2 / 13) // TODO: this is set using jumpers J2
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE("acia1", acia6850_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("acia1", acia6850_device, write_rxc))

	/* modem port */
	MCFG_DEVICE_ADD("acia2", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE("modem", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(WRITELINE("modem", rs232_port_device, write_rts))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE("irqs", input_merger_device, in_w<6>))

	MCFG_DEVICE_ADD("modem", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("acia2", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(WRITELINE("acia2", acia6850_device, write_cts))

	MCFG_DEVICE_ADD("acia2_clock", CLOCK, 4_MHz_XTAL / 2 / 13) // TODO: this is set using jumpers J1
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE("acia2", acia6850_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("acia2", acia6850_device, write_rxc))

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_list", "poly_flop")
	MCFG_SOFTWARE_LIST_FILTER("flop_list", "PROTEUS")
MACHINE_CONFIG_END


ROM_START(proteus)
	ROM_REGION(0x1000, "bios", 0)
	ROM_DEFAULT_BIOS("82")
	ROM_SYSTEM_BIOS(0, "82", "030982")
	ROMX_LOAD("proteusv30.u28", 0x0000, 0x1000, CRC(ad02dc36) SHA1(f73aff8b3d85448f603a2fe807e3a7e02550db27), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "86", "300986")
	ROMX_LOAD("300986.u28", 0x0000, 0x1000, CRC(f607d396) SHA1(0b9d9d3178b5587e60da56885b9e8a83f7d5dd26), ROM_BIOS(2))
ROM_END


/*    YEAR  NAME     PARENT  COMPAT   MACHINE  INPUT    CLASS          INIT        COMPANY     FULLNAME                     FLAGS */
COMP( 1982, proteus, 0,      0,       proteus, proteus, proteus_state, empty_init, "Polycorp", "Poly Proteus (Standalone)", MACHINE_NOT_WORKING )

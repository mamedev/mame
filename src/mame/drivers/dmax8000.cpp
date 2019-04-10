// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************************************************

Datamax 8000

2016-07-24 Skeleton driver

This computer was designed and made by Datamax Pty Ltd, in Manly, Sydney, Australia.

It could have up to 4x 8 inch floppy drives, in the IBM format.
There were a bunch of optional extras such as 256kB RAM, numeric coprocessor, RTC, etc.


ToDo:
- Parallel port
- Centronics port
- AUX serial port
- FDC/FDD/HD setup - no schematics available of this section, so guessing...

What there is of the schematic shows no sign of a daisy chain or associated interrupts.


****************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "machine/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"
#include "machine/z80ctc.h"
#include "machine/mm58274c.h"
#include "bus/rs232/rs232.h"



class dmax8000_state : public driver_device
{
public:
	dmax8000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
	{ }

	void dmax8000(machine_config &config);

	void init_dmax8000();

private:
	DECLARE_MACHINE_RESET(dmax8000);
	DECLARE_WRITE8_MEMBER(port0c_w);
	DECLARE_WRITE8_MEMBER(port0d_w);
	DECLARE_WRITE8_MEMBER(port14_w);
	DECLARE_WRITE8_MEMBER(port40_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);

	void dmax8000_io(address_map &map);
	void dmax8000_mem(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<fd1793_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
};


WRITE_LINE_MEMBER( dmax8000_state::fdc_drq_w )
{
	if (state) printf("DRQ ");
}

WRITE8_MEMBER( dmax8000_state::port0c_w )
{
	printf("Port0c=%X\n", data);
	m_fdc->dden_w(BIT(data, 6));
	floppy_image_device *floppy = nullptr;
	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	m_fdc->set_floppy(floppy);
	if (floppy)
	{
		floppy->mon_w(0);
		floppy->ss_w(0); // no idea
	}
}

WRITE8_MEMBER( dmax8000_state::port0d_w )
{
	printf("Port0d=%X\n", data);
}

WRITE8_MEMBER( dmax8000_state::port14_w )
{
	printf("Port14=%X\n", data);
}

WRITE8_MEMBER( dmax8000_state::port40_w )
{
	membank("bankr0")->set_entry(BIT(data, 0));
}

void dmax8000_state::dmax8000_mem(address_map &map)
{
	map(0x0000, 0x0fff).bankr("bankr0").bankw("bankw0");
	map(0x1000, 0xffff).ram();
}

void dmax8000_state::dmax8000_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x03).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0x04, 0x07).rw("dart1", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0x08, 0x0b).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x0c, 0x0f).rw("pio1", FUNC(z80pio_device::read), FUNC(z80pio_device::write)); // fdd controls
	map(0x10, 0x13).rw("pio2", FUNC(z80pio_device::read), FUNC(z80pio_device::write)); // centronics & parallel ports
	map(0x14, 0x17).w(FUNC(dmax8000_state::port14_w)); // control lines for the centronics & parallel ports
	//AM_RANGE(0x18, 0x19) AM_MIRROR(2) AM_DEVREADWRITE("am9511", am9512_device, read, write) // optional numeric coprocessor
	//AM_RANGE(0x1c, 0x1d) AM_MIRROR(2)  // optional hard disk controller (1C=status, 1D=data)
	map(0x20, 0x23).rw("dart2", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0x40, 0x40).w(FUNC(dmax8000_state::port40_w)); // memory bank control
	//AM_RANGE(0x60, 0x67) // optional IEEE488 GPIB
	map(0x70, 0x7f).rw("rtc", FUNC(mm58274c_device::read), FUNC(mm58274c_device::write)); // optional RTC
}

/* Input ports */
static INPUT_PORTS_START( dmax8000 )
INPUT_PORTS_END

MACHINE_RESET_MEMBER( dmax8000_state, dmax8000 )
{
	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
	m_maincpu->reset();
	m_maincpu->set_input_line_vector(0, 0xee); // fdc vector
}

void dmax8000_state::init_dmax8000()
{
	uint8_t *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);
}

static void floppies(device_slot_interface &device)
{
	device.option_add("8dsdd", FLOPPY_8_DSDD);
}


void dmax8000_state::dmax8000(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4'000'000); // no idea what crystal is used, but 4MHz clock is confirmed
	m_maincpu->set_addrmap(AS_PROGRAM, &dmax8000_state::dmax8000_mem);
	m_maincpu->set_addrmap(AS_IO, &dmax8000_state::dmax8000_io);
	MCFG_MACHINE_RESET_OVERRIDE(dmax8000_state, dmax8000)

	z80ctc_device &ctc(Z80CTC(config, "ctc", 4_MHz_XTAL));
	ctc.set_clk<0>(4_MHz_XTAL / 2); // 2MHz
	ctc.zc_callback<0>().set("dart1", FUNC(z80dart_device::rxca_w));
	ctc.zc_callback<0>().append("dart1", FUNC(z80dart_device::txca_w));
	ctc.zc_callback<0>().append("dart2", FUNC(z80dart_device::rxca_w));
	ctc.zc_callback<0>().append("dart2", FUNC(z80dart_device::txca_w));
	ctc.set_clk<1>(4_MHz_XTAL / 2); // 2MHz
	ctc.zc_callback<1>().set("dart2", FUNC(z80dart_device::rxtxcb_w));
	ctc.set_clk<2>(4_MHz_XTAL / 2); // 2MHz
	ctc.zc_callback<2>().set("dart1", FUNC(z80dart_device::rxtxcb_w));

	z80dart_device& dart1(Z80DART(config, "dart1", 4'000'000)); // A = terminal; B = aux
	dart1.out_txda_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	dart1.out_dtra_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	dart1.out_rtsa_callback().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("dart1", FUNC(z80dart_device::rxa_w));
	rs232.dcd_handler().set("dart1", FUNC(z80dart_device::dcda_w));
	rs232.ri_handler().set("dart1", FUNC(z80dart_device::ria_w));
	rs232.cts_handler().set("dart1", FUNC(z80dart_device::ctsa_w));

	Z80DART(config, "dart2", 4'000'000); // RS232 ports

	z80pio_device& pio1(Z80PIO(config, "pio1", 4'000'000));
	pio1.out_pa_callback().set(FUNC(dmax8000_state::port0c_w));
	pio1.out_pb_callback().set(FUNC(dmax8000_state::port0d_w));

	Z80PIO(config, "pio2", 4'000'000);

	FD1793(config, m_fdc, 2'000'000); // no idea
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_fdc->drq_wr_callback().set(FUNC(dmax8000_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", floppies, "8dsdd", floppy_image_device::default_floppy_formats).enable_sound(true);

	mm58274c_device &rtc(MM58274C(config, "rtc", 0)); // MM58174
	// this is all guess
	rtc.set_mode24(0); // 12 hour
	rtc.set_day1(1);   // monday
}


/* ROM definition */
ROM_START( dmax8000 )
	ROM_REGION( 0x11000, "maincpu", 0 )
	ROM_LOAD( "rev1_0.rom", 0x10000, 0x001000, CRC(acbec83f) SHA1(fce0a4307a791250dbdc6bb6a190f7fec3619d82) )
	// ROM_LOAD( "rev1_1.rom", 0x10000, 0x001000, CRC(2eb98a61) SHA1(cdd9a58f63ee7e3d3dd1c4ae3fd4376b308fd10f) )  // this is a hacked rom to speed up the serial port
ROM_END

/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT           COMPANY    FULLNAME        FLAGS
COMP( 1981, dmax8000, 0,      0,      dmax8000, dmax8000, dmax8000_state, init_dmax8000, "Datamax", "Datamax 8000", MACHINE_NOT_WORKING )

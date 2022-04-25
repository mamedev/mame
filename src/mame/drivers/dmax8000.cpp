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
#include "machine/z80sio.h"
#include "machine/z80ctc.h"
#include "machine/mm58174.h"
#include "bus/rs232/rs232.h"

namespace {

class dmax8000_state : public driver_device
{
public:
	dmax8000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_bank1(*this, "bank1")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
	{ }

	void dmax8000(machine_config &config);

private:
	void machine_reset() override;
	void machine_start() override;
	void port0c_w(u8 data);
	void port0d_w(u8 data);
	void port14_w(u8 data);
	void port40_w(u8 data);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_memory_bank    m_bank1;
	required_device<fd1793_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
};


WRITE_LINE_MEMBER( dmax8000_state::fdc_drq_w )
{
	if (state) printf("DRQ ");
}

void dmax8000_state::port0c_w(u8 data)
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

void dmax8000_state::port0d_w(u8 data)
{
	printf("Port0d=%X\n", data);
}

void dmax8000_state::port14_w(u8 data)
{
	printf("Port14=%X\n", data);
}

void dmax8000_state::port40_w(u8 data)
{
	m_bank1->set_entry(BIT(~data, 0));
}

void dmax8000_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("mainram");
	map(0x0000, 0x0fff).bankr("bank1");
}

void dmax8000_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x03).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0x04, 0x07).rw("dart1", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0x08, 0x0b).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x0c, 0x0f).rw("pio1", FUNC(z80pio_device::read), FUNC(z80pio_device::write)); // fdd controls
	map(0x10, 0x13).rw("pio2", FUNC(z80pio_device::read), FUNC(z80pio_device::write)); // centronics & parallel ports
	map(0x14, 0x17).w(FUNC(dmax8000_state::port14_w)); // control lines for the centronics & parallel ports
	//map(0x18, 0x19).mirror(2).rw("am9511", FUNC(am9512_device::read), FUNC(am9512_device::write)); // optional numeric coprocessor
	//map(0x1c, 0x1d).mirror(2);  // optional hard disk controller (1C=status, 1D=data)
	map(0x20, 0x23).rw("dart2", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0x40, 0x40).w(FUNC(dmax8000_state::port40_w)); // memory bank control
	//map(0x60, 0x67) // optional IEEE488 GPIB
	map(0x70, 0x7f).rw("rtc", FUNC(mm58174_device::read), FUNC(mm58174_device::write)); // optional RTC
}

/* Input ports */
static INPUT_PORTS_START( dmax8000 )
INPUT_PORTS_END

void dmax8000_state::machine_reset()
{
	m_bank1->set_entry(1);
	m_maincpu->set_input_line_vector(0, 0xee); // Z80 - fdc vector
}

void dmax8000_state::machine_start()
{
	m_bank1->configure_entry(0, m_ram);
	m_bank1->configure_entry(1, m_rom);
}

static void floppies(device_slot_interface &device)
{
	device.option_add("8dsdd", FLOPPY_8_DSDD);
}


void dmax8000_state::dmax8000(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4'000'000); // no idea what crystal is used, but 4MHz clock is confirmed
	m_maincpu->set_addrmap(AS_PROGRAM, &dmax8000_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &dmax8000_state::io_map);

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
	FLOPPY_CONNECTOR(config, "fdc:0", floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	MM58174(config, "rtc", 0);
}


/* ROM definition */
ROM_START( dmax8000 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "rev1_0.rom", 0x0000, 0x001000, CRC(acbec83f) SHA1(fce0a4307a791250dbdc6bb6a190f7fec3619d82) )
	// ROM_LOAD( "rev1_1.rom", 0x0000, 0x001000, CRC(2eb98a61) SHA1(cdd9a58f63ee7e3d3dd1c4ae3fd4376b308fd10f) )  // this is a hacked rom to speed up the serial port
ROM_END

} // Anonymous namespace

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT           COMPANY    FULLNAME        FLAGS
COMP( 1981, dmax8000, 0,      0,      dmax8000, dmax8000, dmax8000_state, empty_init, "Datamax", "Datamax 8000", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

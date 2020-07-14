// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

mc-CP/M-Computer

2010-08-31 Skeleton driver.
2010-11-18 Connected to a terminal
2011-09-28 Added more bioses

Some Monitor commands (varies between versions):

B - lock keyboard (^N to regain control)
E - prints a number
I - Select boot drive/set parameters - then it attempts to boot
K,O - display version header
N - newline
Z - print 'EFFF'

URL for v3.4: http://www.hanshehl.de/mc-prog.htm (German language)

Although the manual specifies ports 40-44 for the FDC, all bios versions
support it at 30-34 as well.

No software to test with, so we'll never know if the FDC works.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "machine/clock.h"
#include "machine/bankdev.h"
#include "machine/wd_fdc.h"
#include "bus/rs232/rs232.h"
#include "imagedev/floppy.h"


class mccpm_state : public driver_device
{
public:
	mccpm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bank(*this, "bankdev_map")
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
	{ }

	void mccpm(machine_config &config);

private:
	void io_map(address_map &map);
	void mem_map(address_map &map);
	void bankdev_map(address_map &map);
	void port44_w(u8);
	u8 port44_r();
	void fdc_irq(bool);
	u8 m_fdc_status;
	floppy_image_device *m_floppy;
	void machine_reset() override;
	void machine_start() override;
	required_device<cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_bank;
	required_device<fd1797_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
};


void mccpm_state::bankdev_map(address_map &map)
{
	// bank 0
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x4000, 0x7fff).lr8(NAME([this] () { if (!machine().side_effects_disabled()) m_bank->set_bank(1); return 0xff; }));
	// bank 1
	map(0x8000, 0xffff).ram();
}

void mccpm_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).m(m_bank, FUNC(address_map_bank_device::amap8));
	map(0x8000, 0xffff).ram();
}

void mccpm_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x40, 0x43).rw(m_fdc, FUNC(fd1797_device::read), FUNC(fd1797_device::write));
	map(0x44, 0x44).rw(FUNC(mccpm_state::port44_r), FUNC(mccpm_state::port44_w));
	map(0xf0, 0xf3).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0xf4, 0xf7).rw("pio", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
}

/* Input ports */
static INPUT_PORTS_START( mccpm )
INPUT_PORTS_END

void mccpm_state::port44_w(u8 data)
{
	m_floppy = nullptr;
	if (BIT(data, 1))
		m_floppy = m_floppy1->get_device();
	else
	if (BIT(data, 0))
		m_floppy = m_floppy0->get_device();

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		m_floppy->mon_w(0);
		m_fdc->dden_w(!BIT(data, 4));   // 0 = FM; 1 = MFM
	}
	// side select comes from fdc pin 25
	m_fdc->set_unscaled_clock(BIT(data, 5) ? 1e6 : 2e6);  // 13 or 20cm clock select
	m_maincpu->set_input_line_vector(0, 0xD7 ); // Z80 - jump to 0x0010 upon interrupt acknowledge IM 0 (or should it say 0x10?)
}

u8 mccpm_state::port44_r()
{
	// bit 4 is floppy hld_r, not yet emulated.
	// So we assume the head is loaded if the drive is selected.
	if (m_floppy)
		return m_fdc_status | 4;
	else
		return m_fdc_status;
}

void mccpm_state::fdc_irq(bool state)
{
	m_fdc_status = (m_fdc_status & 0xfd) | (state ? 2 : 0);
	m_maincpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

void mccpm_state::machine_reset()
{
	m_bank->set_bank(0);
	m_fdc_status = 0xfb;
	m_floppy = nullptr;
}

void mccpm_state::machine_start()
{
	save_item(NAME(m_fdc_status));;
}

static void flop_types(device_slot_interface &device)
{
	device.option_add("flop", FLOPPY_525_QD);
}

void mccpm_state::mccpm(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &mccpm_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mccpm_state::io_map);

	ADDRESS_MAP_BANK(config, m_bank, 0);
	m_bank->set_addrmap(0, &mccpm_state::bankdev_map);
	m_bank->set_data_width(8);
	m_bank->set_addr_width(16);
	m_bank->set_stride(0x8000);

	/* Devices */
	// clock supplied by pair of HD4702 baud rate generators
	clock_device &uart_clock(CLOCK(config, "uart_clock", 153'600));
	uart_clock.signal_handler().set("sio", FUNC(z80sio_device::txca_w));
	uart_clock.signal_handler().append("sio", FUNC(z80sio_device::rxca_w));

	// Ch A: terminal; Ch B: printer
	z80sio_device& sio(Z80SIO(config, "sio", XTAL(4'000'000)));
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	sio.out_txda_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	sio.out_dtra_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsa_callback().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("sio", FUNC(z80sio_device::rxa_w));
	rs232.cts_handler().set("sio", FUNC(z80sio_device::ctsa_w));

	Z80PIO(config, "pio", XTAL(4'000'000));

	FD1797(config, m_fdc, 8_MHz_XTAL / 8);
	m_fdc->intrq_wr_callback().set([this] (bool state) { mccpm_state::fdc_irq(state); });
	m_fdc->drq_wr_callback().set([this] (u8 state) { m_fdc_status = (m_fdc_status & 0xfe) | (state ? 1 : 0); });
	FLOPPY_CONNECTOR(config, "fdc:0", flop_types, "flop", floppy_image_device::default_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", flop_types, "flop", floppy_image_device::default_floppy_formats).enable_sound(true);
}

/* ROM definition */
ROM_START( mccpm )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v36", "V3.6")
	ROMX_LOAD("mon36.j15",   0x0000, 0x1000, CRC(9c441537) SHA1(f95bad52d9392b8fc9d9b8779b7b861672a0022b), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v34", "V3.4")
	ROMX_LOAD("monhemc.bin", 0x0000, 0x1000, CRC(cae7b56e) SHA1(1f40be9491a595e6705099a452743cc0d49bfce8), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v34a", "V3.4 (alt)")
	ROMX_LOAD("mc01mon.bin", 0x0000, 0x0d00, CRC(d1c89043) SHA1(f52a0ed3793dde0de74596be7339233b6a1770af), ROM_BIOS(2))
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                         FULLNAME            FLAGS
COMP( 1981, mccpm, 0,      0,      mccpm,   mccpm, mccpm_state, empty_init, "GRAF Elektronik Systeme GmbH", "mc-CP/M-Computer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

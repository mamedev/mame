// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
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
#include "machine/f4702.h"
#include "machine/wd_fdc.h"
#include "bus/rs232/rs232.h"
#include "imagedev/floppy.h"


namespace {

class mccpm_state : public driver_device
{
public:
	mccpm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "fdc")
		, m_fdd(*this, "fdc:%u", 0U)
		, m_brg(*this, "brg%u", 1U)
		, m_view(*this, "view")
	{ }

	void mccpm(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	void port44_w(u8);
	u8 port44_r();
	void fdc_irq(bool);
	template <int N> void bd_q_w(offs_t offset, u8 data);

	u8 m_fdc_status = 0U;
	floppy_image_device *m_floppy = 0;

	required_device<cpu_device> m_maincpu;
	required_device<fd1797_device> m_fdc;
	required_device_array<floppy_connector, 2> m_fdd;
	required_device_array<f4702_device, 2> m_brg;
	memory_view m_view;
};



void mccpm_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).view(m_view);
	m_view[0](0x0000, 0x3fff).rom().region("maincpu", 0);
	m_view[0](0x4000, 0x7fff).lr8(NAME([this] () { if (!machine().side_effects_disabled()) m_view.select(1); return 0xff; }));
	m_view[1](0x0000, 0x7fff).ram();
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
	PORT_START("BAUD1")
	PORT_DIPNAME(0xf, 0x8, "Baud Rate B (Printer)") PORT_DIPLOCATION("S:5,6,7,8")
	PORT_DIPSETTING(0x2, "50")
	PORT_DIPSETTING(0x3, "75")
	PORT_DIPSETTING(0xf, "110")
	PORT_DIPSETTING(0x4, "134.5")
	PORT_DIPSETTING(0xe, "150")
	PORT_DIPSETTING(0x5, "200")
	PORT_DIPSETTING(0xd, "300")
	PORT_DIPSETTING(0x6, "600")
	PORT_DIPSETTING(0xb, "1200")
	PORT_DIPSETTING(0xa, "1800")
	PORT_DIPSETTING(0x7, "2400")
	PORT_DIPSETTING(0x9, "4800")
	PORT_DIPSETTING(0x8, "9600")
	PORT_DIPSETTING(0x0, "19200")

	PORT_START("BAUD2")
	PORT_DIPNAME(0xf, 0x8, "Baud Rate A (Terminal)") PORT_DIPLOCATION("S:1,2,3,4")
	PORT_DIPSETTING(0x2, "50")
	PORT_DIPSETTING(0x3, "75")
	PORT_DIPSETTING(0xf, "110")
	PORT_DIPSETTING(0x4, "134.5")
	PORT_DIPSETTING(0xe, "150")
	PORT_DIPSETTING(0x5, "200")
	PORT_DIPSETTING(0xd, "300")
	PORT_DIPSETTING(0x6, "600")
	PORT_DIPSETTING(0xb, "1200")
	PORT_DIPSETTING(0xa, "1800")
	PORT_DIPSETTING(0x7, "2400")
	PORT_DIPSETTING(0x9, "4800")
	PORT_DIPSETTING(0x8, "9600")
	PORT_DIPSETTING(0x0, "19200")
INPUT_PORTS_END

void mccpm_state::port44_w(u8 data)
{
	m_floppy = nullptr;
	if (BIT(data, 1))
		m_floppy = m_fdd[1]->get_device();
	else
	if (BIT(data, 0))
		m_floppy = m_fdd[0]->get_device();

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

template <int N> void mccpm_state::bd_q_w(offs_t offset, u8 data)
{
	// "19200 extern" obtained by connecting Q2 to Im
	m_brg[N]->im_w(BIT(offset, 2));
}

void mccpm_state::machine_reset()
{
	m_view.select(0);
	m_fdc_status = 0xfb;
	m_floppy = nullptr;
}

void mccpm_state::machine_start()
{
	save_item(NAME(m_fdc_status));
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

	/* Devices */
	// clock supplied by pair of HD4702 baud rate generators
	F4702(config, m_brg[0], 2.4576_MHz_XTAL); // XTAL connected to Ix/Ox
	m_brg[0]->s_callback().set_ioport("BAUD1");
	m_brg[0]->z_callback().set("sio", FUNC(z80sio_device::rxtxcb_w));
	m_brg[0]->z_callback().append(FUNC(mccpm_state::bd_q_w<0>));

	F4702(config, m_brg[1], 2.4576_MHz_XTAL); // Cp connected to first BRG's CO
	m_brg[1]->s_callback().set_ioport("BAUD2");
	m_brg[1]->z_callback().set("sio", FUNC(z80sio_device::txca_w));
	m_brg[1]->z_callback().append("sio", FUNC(z80sio_device::rxca_w));
	m_brg[1]->z_callback().append(FUNC(mccpm_state::bd_q_w<1>));

	// Ch A: terminal; Ch B: printer
	z80sio_device& sio(Z80SIO(config, "sio", XTAL(4'000'000)));
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	sio.out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	sio.out_dtra_callback().set("rs232a", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	sio.out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	sio.out_dtrb_callback().set("rs232b", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsb_callback().set("rs232b", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set("sio", FUNC(z80sio_device::rxa_w));
	rs232a.cts_handler().set("sio", FUNC(z80sio_device::ctsa_w));
	rs232a.dcd_handler().set("sio", FUNC(z80sio_device::dcda_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set("sio", FUNC(z80sio_device::rxb_w));
	rs232b.cts_handler().set("sio", FUNC(z80sio_device::ctsb_w));
	rs232b.dcd_handler().set("sio", FUNC(z80sio_device::dcdb_w));

	Z80PIO(config, "pio", XTAL(4'000'000));

	FD1797(config, m_fdc, 8_MHz_XTAL / 8);
	m_fdc->intrq_wr_callback().set([this] (bool state) { mccpm_state::fdc_irq(state); });
	m_fdc->drq_wr_callback().set([this] (u8 state) { m_fdc_status = (m_fdc_status & 0xfe) | (state ? 1 : 0); });
	FLOPPY_CONNECTOR(config, "fdc:0", flop_types, "flop", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", flop_types, "flop", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
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

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                         FULLNAME            FLAGS
COMP( 1981, mccpm, 0,      0,      mccpm,   mccpm, mccpm_state, empty_init, "GRAF Elektronik Systeme GmbH", "mc-CP/M-Computer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

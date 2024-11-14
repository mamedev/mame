// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/**********************************************************************************************

Morrow Designs Micro Decision

2009-12-10 Skeleton driver.

Although it looks like an ordinary CP/M computer, the monitor and keyboard are actually a
dumb terminal plugged into the base unit. Therefore the roms and details of the terminal are
needed.

Board design changes depending on bios version of base unit:
In earliest design, F7 sets up VFO and F8 selects motor on, while fdc does drive select.
Later version gets rid of these, and F7 now does motor on and drive select; also addition of
i8253 timer, a centronics port, and a diagnostic jumper. F8 is unused.

Currently (as at 2016-07-17), memory test works, rom banking works, disk does NOT boot.
Ver 1 roms say "Not found", Ver 2 roms hang after pressing enter, Ver 3 rom hangs after memory test.

ToDo:
- Make the floppy boot
- Add i8253 timer chip
- Add centronics parts
- Different hardware for different bios versions (only earliest design is coded)

***********************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "bus/rs232/rs232.h"
#include "softlist_dev.h"


namespace {

class microdec_state : public driver_device
{
public:
	microdec_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy(nullptr)
		, m_rom_view(*this, "rom")
	{ }

	void microdec(machine_config &config);

	void init_microdec();

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	uint8_t portf5_r();
	uint8_t portf6_r();
	void portf6_w(uint8_t data);
	uint8_t portf7_r();
	void portf7_w(uint8_t data);
	void portf8_w(uint8_t data);

	void microdec_io(address_map &map) ATTR_COLD;
	void microdec_mem(address_map &map) ATTR_COLD;

	uint8_t m_portf8 = 0U;
	bool m_fdc_rdy = 0;

	required_device<cpu_device> m_maincpu;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	floppy_image_device *m_floppy;
	memory_view m_rom_view;
};


/*
d0-2 : motor on signals from f8
d3   : ack (cent)
d4   : ready (fdd)
d5   : diag jumper (md3 only) */
uint8_t microdec_state::portf5_r()
{
	if (!machine().side_effects_disabled())
		m_fdc->set_ready_line_connected(m_fdc_rdy);

	uint8_t data = m_portf8 | ioport("DIAG")->read() | 0xc0;
	return data;
}

// disable eprom
uint8_t microdec_state::portf6_r()
{
	if (!machine().side_effects_disabled())
		m_rom_view.disable(); // point at ram
	return 0xff;
}

// TC pin on fdc
uint8_t microdec_state::portf7_r()
{
	if (!machine().side_effects_disabled())
	{
		m_fdc->tc_w(1);
		m_fdc->tc_w(0);
	}
	return 0xff;
}

// enable eprom
void microdec_state::portf6_w(uint8_t data)
{
	m_rom_view.select(0); // point at rom
}

// sets up VFO stuff
void microdec_state::portf7_w(uint8_t data)
{
	m_fdc_rdy = BIT(data,2);
}

/*
d0-2 : motor on for drive sockets
d3   : precomp */
void microdec_state::portf8_w(uint8_t data)
{
	m_portf8 = data & 7;
	/* code for motor on per drive goes here */
	m_floppy = m_floppy0->get_device();
	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
		m_floppy->mon_w(0);
}

void microdec_state::microdec_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).ram();
	map(0x0000, 0x0fff).view(m_rom_view);
	m_rom_view[0](0x0000, 0x0fff).rom().region("maincpu", 0);
}

void microdec_state::microdec_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xf5, 0xf5).r(FUNC(microdec_state::portf5_r));
	map(0xf6, 0xf6).rw(FUNC(microdec_state::portf6_r), FUNC(microdec_state::portf6_w));
	map(0xf7, 0xf7).rw(FUNC(microdec_state::portf7_r), FUNC(microdec_state::portf7_w));
	map(0xf8, 0xf8).w(FUNC(microdec_state::portf8_w));
	map(0xfa, 0xfb).m(m_fdc, FUNC(upd765a_device::map));
	map(0xfc, 0xfd).rw("uart1", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xfe, 0xff).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write));
	// map(0xf0, 0xf3) 8253 PIT (md3 only) used as a baud rate generator for serial ports
	// map(0xf4, 0xf4) Centronics data
	// map(0xf5, 0xf5) motor check (md1/2)
	// map(0xf5, 0xf5) Centronics status (md3) read bit 3 (ack=1); read bit 4 (busy=1); write bit 7 (stb=0)
	// map(0xf6, 0xf6) rom enable (w=enable; r=disable)
	// map(0xf7, 0xf7) VFO Count set
	// map(0xf8, 0xf8) Motor and Shift control
	// map(0xfa, 0xfb) uPD765C fdc FA=status; FB=data
	// map(0xfc, 0xfd) Serial Port 1 (terminal) FC=data FD=status
	// map(0xfe, 0xff) Serial Port 2 (printer) FE=data FF=status
}

/* Input ports */
static INPUT_PORTS_START( microdec )
	PORT_START("DIAG")
	PORT_DIPNAME( 0x20, 0x20, "Diagnostics" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void microdec_state::machine_start()
{
	save_item(NAME(m_portf8));
	save_item(NAME(m_fdc_rdy));
}

void microdec_state::machine_reset()
{
	m_rom_view.select(0); // point at rom
}

static void microdec_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
}

void microdec_state::init_microdec()
{
	m_fdc->set_ready_line_connected(1);
}

void microdec_state::microdec(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &microdec_state::microdec_mem);
	m_maincpu->set_addrmap(AS_IO, &microdec_state::microdec_io);
	m_maincpu->set_irq_acknowledge_callback(NAME([](device_t &, int) -> int { return 0x7f; })); // 7407 drives D7 low

	/* video hardware */
	clock_device &uart_clock(CLOCK(config, "uart_clock", 16_MHz_XTAL / 4 / 13 / 2)); // TODO: rate configured by SW1/SW1 or PIT
	uart_clock.signal_handler().set("uart1", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("uart1", FUNC(i8251_device::write_rxc));
	uart_clock.signal_handler().append("uart2", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("uart2", FUNC(i8251_device::write_rxc));

	i8251_device &uart1(I8251(config, "uart1", 16_MHz_XTAL / 8));
	uart1.txd_handler().set("rs232a", FUNC(rs232_port_device::write_txd));
	uart1.dtr_handler().set("rs232a", FUNC(rs232_port_device::write_dtr));
	uart1.rts_handler().set("rs232a", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set("uart1", FUNC(i8251_device::write_rxd));
	rs232a.dsr_handler().set("uart1", FUNC(i8251_device::write_dsr));
	rs232a.cts_handler().set("uart1", FUNC(i8251_device::write_cts));

	i8251_device &uart2(I8251(config, "uart2", 16_MHz_XTAL / 8));
	uart2.txd_handler().set("rs232b", FUNC(rs232_port_device::write_txd));
	uart2.dtr_handler().set("rs232b", FUNC(rs232_port_device::write_dtr));
	uart2.rts_handler().set("rs232b", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set("uart2", FUNC(i8251_device::write_rxd));
	rs232b.dsr_handler().set("uart2", FUNC(i8251_device::write_dsr));
	rs232b.cts_handler().set("uart2", FUNC(i8251_device::write_cts));

	UPD765A(config, m_fdc, 16_MHz_XTAL / 4, true, true);
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	FLOPPY_CONNECTOR(config, "fdc:0", microdec_floppies, "525hd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	//FLOPPY_CONNECTOR(config, "fdc:1", microdec_floppies, "525hd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("md2_flop");
}

/* ROM definition */
ROM_START( md2 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v13", "v1.3" )
	ROMX_LOAD("md2-13.bin",  0x0000, 0x0800, CRC(43f4c9ab) SHA1(48a35cbee4f341310e9cba5178c3fd6e74ef9748), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v13a", "v1.3a" )
	ROMX_LOAD("md2-13a.bin", 0x0000, 0x0800, CRC(d7fcddfd) SHA1(cae29232b737ebb36a27b8ad17bc69e9968f1309), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "v13b", "v1.3b" )
	ROMX_LOAD("md2-13b.bin", 0x0000, 0x1000, CRC(a8b96835) SHA1(c6b111939aa7e725da507da1915604656540b24e), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "v20", "v2.0" )
	ROMX_LOAD("md2-20.bin",  0x0000, 0x1000, CRC(a604735c) SHA1(db6e6e82a803f5cbf4f628f5778a93ae3e211fe1), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 4, "v23", "v2.3" )
	ROMX_LOAD("md2-23.bin",  0x0000, 0x1000, CRC(49bae273) SHA1(00381a226fe250aa3636b0b740df0af63efb0d18), ROM_BIOS(4))
ROM_END

ROM_START( md3 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v23a", "v2.3a" )
	ROMX_LOAD("md3-23a.bin", 0x0000, 0x1000, CRC(95d59980) SHA1(ae65a8e8e2823cf4cf6b1d74c0996248e290e9f1), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v25", "v2.5" )
	ROMX_LOAD("md3-25.bin",  0x0000, 0x1000, CRC(14f86bc5) SHA1(82fe022c85f678744bb0340ca3f88b18901fdfcb), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "v31", "v3.1" )
	ROMX_LOAD("md3-31.bin",  0x0000, 0x1000, CRC(bd4014f6) SHA1(5b33220af34c64676756177db4915f97840b2996), ROM_BIOS(2))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME  PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT           COMPANY           FULLNAME               FLAGS
COMP( 1982, md2,  0,      0,      microdec, microdec, microdec_state, init_microdec, "Morrow Designs", "Micro Decision MD-2", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1982, md3,  md2,    0,      microdec, microdec, microdec_state, init_microdec, "Morrow Designs", "Micro Decision MD-3", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )

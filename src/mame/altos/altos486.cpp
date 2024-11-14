// License: BSD-3-Clause
// copyright-holders:Carl

// Altos 486, very incomplete

#include "emu.h"
#include "cpu/i86/i186.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/z80sio.h"
#include "machine/pit8253.h"
#include "machine/upd765.h"
#include "machine/i8255.h"
#include "bus/rs232/rs232.h"


namespace {

class altos486_state : public driver_device
{
public:
	altos486_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, "main_ram"),
		m_rom(*this, "bios")
	{ }

	void altos486(machine_config &config);

private:
	uint8_t read_rmx_ack(offs_t offset);

	uint16_t mmu_ram_r(offs_t offset);
	uint16_t mmu_io_r(offs_t offset);
	void mmu_ram_w(offs_t offset, uint16_t data);
	void mmu_io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void altos486_io(address_map &map) ATTR_COLD;
	void altos486_mem(address_map &map) ATTR_COLD;
	void altos486_z80_io(address_map &map) ATTR_COLD;
	void altos486_z80_mem(address_map &map) ATTR_COLD;

	required_device<i80186_cpu_device> m_maincpu;
	required_shared_ptr<uint16_t> m_ram;
	required_memory_region m_rom;

	bool m_sys_mode;
	uint8_t m_prot[256];
};

uint8_t altos486_state::read_rmx_ack(offs_t offset)
{
	if(offset == 4)
		return m_maincpu->int_callback(*this, 0);

	return 0;
}

uint16_t altos486_state::mmu_ram_r(offs_t offset)
{
	if (offset < 0x7e000)
		return m_ram[offset]; // TODO
	else
		return m_rom->as_u16(offset - 0x7e000);
}

uint16_t altos486_state::mmu_io_r(offs_t offset)
{
	if (!m_sys_mode)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		return 0;
	}
	if(offset < 0x100)
		return m_prot[offset];
	return 0; // TODO
}

void altos486_state::mmu_ram_w(offs_t offset, uint16_t data)
{
	//uint16_t entry = m_prot[offset >> 11];
	//if(!m_sys_mode)
}

void altos486_state::mmu_io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(!m_sys_mode)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		return;
	}
	if(offset < 0x100)
	{
		if(mem_mask != 0xff00)
			m_prot[offset] = data & 0xf;
	}
}

static void altos486_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void altos486_state::altos486_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0xfffff).rw(FUNC(altos486_state::mmu_ram_r), FUNC(altos486_state::mmu_ram_w)).share("main_ram");
}

void altos486_state::altos486_io(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(altos486_state::mmu_io_r), FUNC(altos486_state::mmu_io_w));
}

void altos486_state::altos486_z80_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom().region("iocpu", 0);
	map(0x1000, 0x17ff).ram();
	//map(0x8000, 0xffff).rw(FUNC(altos486_state::z80_shared_r), FUNC(altos486_state::z80_shared_w)):
}

void altos486_state::altos486_z80_io(address_map &map)
{
	//map(0x00, 0x03).rw("sio0", FUNC(z80sio_device::read), FUNC(z80sio_device::write));
	//map(0x04, 0x07).rw("sio1", FUNC(z80sio_device::read), FUNC(z80sio_device::write));
	//map(0x08, 0x0b).rw("sio2", FUNC(z80sio_device::read), FUNC(z80sio_device::write));
}

void altos486_state::altos486(machine_config &config)
{
	I80186(config, m_maincpu, 32_MHz_XTAL / 2); // divided by 2 externally and by 2 again internally to operate at 8 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &altos486_state::altos486_mem);
	m_maincpu->set_addrmap(AS_IO, &altos486_state::altos486_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259", FUNC(pic8259_device::inta_cb)); // yes, really

	z80_device &iocpu(Z80(config, "iocpu", 32_MHz_XTAL / 8));
	iocpu.set_addrmap(AS_PROGRAM, &altos486_state::altos486_z80_mem);
	iocpu.set_addrmap(AS_IO, &altos486_state::altos486_z80_io);

	pic8259_device &pic8259(PIC8259(config, "pic8259", 0));
	pic8259.out_int_callback().set(m_maincpu, FUNC(i80186_cpu_device::int0_w));
	pic8259.in_sp_callback().set_constant(1);
	pic8259.read_slave_ack_callback().set(FUNC(altos486_state::read_rmx_ack));

	I8255(config, "ppi8255");

	UPD765A(config, "fdc", 32_MHz_XTAL / 4, false, false);
	FLOPPY_CONNECTOR(config, "fdc:0", altos486_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).set_fixed(true);

	z80sio_device& sio0(Z80SIO(config, "sio0", 32_MHz_XTAL / 8)); // Z8440APS
	sio0.out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	sio0.out_dtra_callback().set("rs232a", FUNC(rs232_port_device::write_dtr));
	sio0.out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	sio0.out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	sio0.out_dtrb_callback().set("rs232b", FUNC(rs232_port_device::write_dtr));
	sio0.out_rtsb_callback().set("rs232b", FUNC(rs232_port_device::write_rts));
	//sio0.out_int_callback().set(FUNC(altos486_state::sio_interrupt));

	z80sio_device& sio1(Z80SIO(config, "sio1", 32_MHz_XTAL / 8)); // Z8440APS
	sio1.out_txda_callback().set("rs232c", FUNC(rs232_port_device::write_txd));
	sio1.out_dtra_callback().set("rs232c", FUNC(rs232_port_device::write_dtr));
	sio1.out_rtsa_callback().set("rs232c", FUNC(rs232_port_device::write_rts));
	sio1.out_txdb_callback().set("rs232d", FUNC(rs232_port_device::write_txd));
	sio1.out_dtrb_callback().set("rs232d", FUNC(rs232_port_device::write_dtr));
	sio1.out_rtsb_callback().set("rs232d", FUNC(rs232_port_device::write_rts));
	//sio1.out_int_callback().set(FUNC(altos486_state::sio_interrupt));

	z80sio_device& sio2(Z80SIO(config, "sio2", 32_MHz_XTAL / 8)); // Z8440APS
	sio2.out_txda_callback().set("rs232_lp", FUNC(rs232_port_device::write_txd));
	sio2.out_dtra_callback().set("rs232_lp", FUNC(rs232_port_device::write_dtr));
	sio2.out_rtsa_callback().set("rs232_lp", FUNC(rs232_port_device::write_rts));
	//sio2.out_int_callback().set(FUNC(altos486_state::sio_interrupt));

	i8274_device& i8274(I8274(config, "i8274", 32_MHz_XTAL / 8));
	i8274.out_txda_callback().set("rs422_wn", FUNC(rs232_port_device::write_txd));
	i8274.out_dtra_callback().set("rs422_wn", FUNC(rs232_port_device::write_dtr));
	i8274.out_rtsa_callback().set("rs422_wn", FUNC(rs232_port_device::write_rts));
	//i8274.out_int_callback().set(FUNC(altos486_state::sio_interrupt));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set("sio0", FUNC(z80sio_device::rxa_w));
	rs232a.dcd_handler().set("sio0", FUNC(z80sio_device::dcda_w));
	rs232a.cts_handler().set("sio0", FUNC(z80sio_device::ctsa_w));
	//rs232a.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(altos486_terminal));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set("sio0", FUNC(z80sio_device::rxb_w));
	rs232b.dcd_handler().set("sio0", FUNC(z80sio_device::dcdb_w));
	rs232b.cts_handler().set("sio0", FUNC(z80sio_device::ctsb_w));

	rs232_port_device &rs232c(RS232_PORT(config, "rs232c", default_rs232_devices, nullptr));
	rs232c.rxd_handler().set("sio1", FUNC(z80sio_device::rxa_w));
	rs232c.dcd_handler().set("sio1", FUNC(z80sio_device::dcda_w));
	rs232c.cts_handler().set("sio1", FUNC(z80sio_device::ctsa_w));

	rs232_port_device &rs232d(RS232_PORT(config, "rs232d", default_rs232_devices, nullptr));
	rs232d.rxd_handler().set("sio1", FUNC(z80sio_device::rxb_w));
	rs232d.dcd_handler().set("sio1", FUNC(z80sio_device::dcdb_w));
	rs232d.cts_handler().set("sio1", FUNC(z80sio_device::ctsb_w));

	rs232_port_device &rs232_lp(RS232_PORT(config, "rs232_lp", default_rs232_devices, nullptr));
	rs232_lp.rxd_handler().set("sio2", FUNC(z80sio_device::rxa_w));
	rs232_lp.dcd_handler().set("sio2", FUNC(z80sio_device::dcda_w));
	rs232_lp.cts_handler().set("sio2", FUNC(z80sio_device::ctsa_w));

	rs232_port_device &rs422_wn(RS232_PORT(config, "rs422_wn", default_rs232_devices, nullptr));
	rs422_wn.rxd_handler().set("i8274", FUNC(i8274_device::rxa_w));
	rs422_wn.dcd_handler().set("i8274", FUNC(i8274_device::dcda_w));
	rs422_wn.cts_handler().set("i8274", FUNC(i8274_device::ctsa_w));

	pit8253_device &pit0(PIT8253(config, "pit0", 0));
	pit0.set_clk<0>(XTAL(22'118'400)/18); // FIXME
	pit0.set_clk<1>(XTAL(22'118'400)/144); // FIXME
	pit0.set_clk<2>(XTAL(22'118'400)/18); // FIXME

	pit8253_device &pit1(PIT8253(config, "pit1", 0));
	pit1.set_clk<0>(XTAL(22'118'400)/18); // FIXME
	pit1.set_clk<1>(XTAL(22'118'400)/144); // FIXME
	pit1.set_clk<2>(XTAL(22'118'400)/18); // FIXME
}


ROM_START( altos486 )
	ROM_REGION( 0x4000, "bios", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v11", "Altos 486 v1.1")
	ROMX_LOAD("16577_lo_v1.1.bin",   0x0000, 0x2000, CRC(65a9db18) SHA1(3ac2b87f1fc0b28ed4907c9b4091aaa170609674), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("16576_hi_v1.1.bin",   0x0001, 0x2000, CRC(cea4cd8d) SHA1(f9f49828bd5e3281bd7cc34d4460ca1b677530b0), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v12", "Altos 486 v1.2")
	ROMX_LOAD("16577-003_4d_v1.2.bin",   0x0000, 0x2000, CRC(e2ac806b) SHA1(9b358246e26b3e85a6dff418899a180370884537), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("16576-003_3d_v1.2.bin",   0x0001, 0x2000, CRC(912f4c12) SHA1(df5088e8610513b577926b0c752e3b54bc880167), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_REGION( 0x1000, "iocpu", 0 )
	ROM_LOAD("16019_z80.bin", 0x0000, 0x1000, CRC(68b1b2e1) SHA1(5d83609a465029212d5e3f72ac9c520b3dbed838))

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "15020.bin",    0x0000, 0x0020, CRC(6a2bd961) SHA1(e9a9ed235574c9871dc32a80ff5ca4df6bd531e1) )
ROM_END

} // anonymous namespace


COMP( 1984, altos486, 0, 0, altos486, 0, altos486_state, empty_init, "Altos Computer Systems", "Altos 486", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

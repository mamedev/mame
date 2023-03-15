// license:BSD-3-Clause
// copyright-holders:Carl

// TODO: multibus

#include "emu.h"
#include "acs8600_ics.h"
#include "machine/z80sio.h"
#include "machine/am9513.h"
#include "bus/rs232/rs232.h"

DEFINE_DEVICE_TYPE(ACS8600_ICS, acs8600_ics_device, "acs8600_ics", "Altos ACS8600 Intelligent Serial Concentrator")

acs8600_ics_device::acs8600_ics_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ACS8600_ICS, tag, owner, clock),
	m_icscpu(*this, "icscpu"),
	m_out_irq1_func(*this),
	m_out_irq2_func(*this),
	m_host_space(*this, finder_base::DUMMY_TAG, -1)
{
}

ROM_START(acs8600_ics)
	ROM_REGION(0x1000, "icscpu", 0)
	ROM_LOAD("162-13914-001.bin", 0x0000, 0x1000, CRC(ab41768a) SHA1(aee04574dea4c17112431536d3b7e3984b8afdc2))
ROM_END

const tiny_rom_entry *acs8600_ics_device::device_rom_region() const
{
	return ROM_NAME(acs8600_ics);
}

void acs8600_ics_device::hiaddr_w(u8 data)
{
	m_hiaddr = data;
}

void acs8600_ics_device::ctrl_w(u8 data)
{
	m_ctrl = data;
	m_out_irq1_func(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);
}

u8 acs8600_ics_device::hostram_r(offs_t offset)
{
	return m_host_space->read_byte((m_hiaddr << 16) | (BIT(m_ctrl, 0) << 15) | (offset & 0x7fff));
}

void acs8600_ics_device::hostram_w(offs_t offset, u8 data)
{
	m_host_space->write_byte((m_hiaddr << 16) | (BIT(m_ctrl, 0) << 15) | (offset & 0x7fff), data);
}

WRITE_LINE_MEMBER(acs8600_ics_device::attn_w)
{
	m_icscpu->set_input_line(INPUT_LINE_NMI, state);
}

static DEVICE_INPUT_DEFAULTS_START(altos8600_terminal)
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void acs8600_ics_device::ics_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("icscpu", 0);
	map(0x1000, 0x17ff).ram();
	map(0x8000, 0xffff).rw(FUNC(acs8600_ics_device::hostram_r), FUNC(acs8600_ics_device::hostram_w));
}

void acs8600_ics_device::ics_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("sio1", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x04, 0x07).rw("sio2", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x08, 0x0b).rw("sio3", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x0c, 0x0f).rw("sio4", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x10, 0x11).rw("stc1", FUNC(am9513_device::read8), FUNC(am9513_device::write8));
	map(0x14, 0x15).rw("stc2", FUNC(am9513_device::read8), FUNC(am9513_device::write8));
	map(0x18, 0x18).w(FUNC(acs8600_ics_device::ctrl_w));
	map(0x1c, 0x1c).w(FUNC(acs8600_ics_device::hiaddr_w));
}

static const z80_daisy_config ics_daisy_chain[] =
{
	{ "sio1" },
	{ "sio2" },
	{ "sio3" },
	{ "sio4" },
	{ nullptr }
};

void acs8600_ics_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_icscpu, 4_MHz_XTAL);
	m_icscpu->set_addrmap(AS_PROGRAM, &acs8600_ics_device::ics_mem);
	m_icscpu->set_addrmap(AS_IO, &acs8600_ics_device::ics_io);
	m_icscpu->set_daisy_config(ics_daisy_chain);

	am9513_device &stc1(AM9513(config, "stc1", 1.8432_MHz_XTAL));
	stc1.out1_cb().set("sio1", FUNC(z80sio_device::rxca_w));
	stc1.out1_cb().append("sio1", FUNC(z80sio_device::txca_w));
	stc1.out2_cb().set("sio1", FUNC(z80sio_device::rxtxcb_w));
	stc1.out3_cb().set("sio2", FUNC(z80sio_device::rxca_w));
	stc1.out3_cb().append("sio2", FUNC(z80sio_device::txca_w));
	stc1.out4_cb().set("sio2", FUNC(z80sio_device::rxtxcb_w));
	stc1.out5_cb().set("sio3", FUNC(z80sio_device::rxca_w));
	stc1.out5_cb().append("sio3", FUNC(z80sio_device::txca_w));

	am9513_device &stc2(AM9513(config, "stc2", 1.8432_MHz_XTAL));
	stc2.out1_cb().set("sio3", FUNC(z80sio_device::rxtxcb_w));
	stc2.out2_cb().set("sio4", FUNC(z80sio_device::rxca_w));
	stc2.out2_cb().append("sio4", FUNC(z80sio_device::txca_w));
	stc2.out3_cb().set("sio4", FUNC(z80sio_device::rxtxcb_w));

	z80sio_device &sio1(Z80SIO(config, "sio1", 8_MHz_XTAL/2));
	sio1.out_txda_callback().set("rs2321a", FUNC(rs232_port_device::write_txd));
	sio1.out_dtra_callback().set("rs2321a", FUNC(rs232_port_device::write_dtr));
	sio1.out_rtsa_callback().set("rs2321a", FUNC(rs232_port_device::write_rts));
	sio1.out_txdb_callback().set("rs2321b", FUNC(rs232_port_device::write_txd));
	sio1.out_dtrb_callback().set("rs2321b", FUNC(rs232_port_device::write_dtr));
	sio1.out_rtsb_callback().set("rs2321b", FUNC(rs232_port_device::write_rts));
	sio1.out_int_callback().set_inputline(m_icscpu, INPUT_LINE_IRQ0);
	sio1.set_cputag(m_icscpu);

	rs232_port_device &rs2321a(RS232_PORT(config, "rs2321a", default_rs232_devices, "terminal"));
	rs2321a.rxd_handler().set("sio1", FUNC(z80sio_device::rxa_w));
	rs2321a.dcd_handler().set("sio1", FUNC(z80sio_device::dcda_w));
	rs2321a.cts_handler().set("sio1", FUNC(z80sio_device::ctsa_w));
	rs2321a.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(altos8600_terminal));

	rs232_port_device &rs2321b(RS232_PORT(config, "rs2321b", default_rs232_devices, nullptr));
	rs2321b.rxd_handler().set("sio1", FUNC(z80sio_device::rxb_w));
	rs2321b.dcd_handler().set("sio1", FUNC(z80sio_device::dcdb_w));
	rs2321b.cts_handler().set("sio1", FUNC(z80sio_device::ctsb_w));

	z80sio_device &sio2(Z80SIO(config, "sio2", 8_MHz_XTAL/2));
	sio2.out_txda_callback().set("rs2322a", FUNC(rs232_port_device::write_txd));
	sio2.out_dtra_callback().set("rs2322a", FUNC(rs232_port_device::write_dtr));
	sio2.out_rtsa_callback().set("rs2322a", FUNC(rs232_port_device::write_rts));
	sio2.out_txdb_callback().set("rs2322b", FUNC(rs232_port_device::write_txd));
	sio2.out_dtrb_callback().set("rs2322b", FUNC(rs232_port_device::write_dtr));
	sio2.out_rtsb_callback().set("rs2322b", FUNC(rs232_port_device::write_rts));
	sio2.out_int_callback().set_inputline(m_icscpu, INPUT_LINE_IRQ0);
	sio2.set_cputag(m_icscpu);

	rs232_port_device &rs2322a(RS232_PORT(config, "rs2322a", default_rs232_devices, nullptr));
	rs2322a.rxd_handler().set("sio2", FUNC(z80sio_device::rxa_w));
	rs2322a.dcd_handler().set("sio2", FUNC(z80sio_device::dcda_w));
	rs2322a.cts_handler().set("sio2", FUNC(z80sio_device::ctsa_w));

	rs232_port_device &rs2322b(RS232_PORT(config, "rs2322b", default_rs232_devices, nullptr));
	rs2322b.rxd_handler().set("sio2", FUNC(z80sio_device::rxb_w));
	rs2322b.dcd_handler().set("sio2", FUNC(z80sio_device::dcdb_w));
	rs2322b.cts_handler().set("sio2", FUNC(z80sio_device::ctsb_w));

	z80sio_device &sio3(Z80SIO(config, "sio3", 8_MHz_XTAL/2));
	sio3.out_txda_callback().set("rs2323a", FUNC(rs232_port_device::write_txd));
	sio3.out_dtra_callback().set("rs2323a", FUNC(rs232_port_device::write_dtr));
	sio3.out_rtsa_callback().set("rs2323a", FUNC(rs232_port_device::write_rts));
	sio3.out_txdb_callback().set("rs2323b", FUNC(rs232_port_device::write_txd));
	sio3.out_dtrb_callback().set("rs2323b", FUNC(rs232_port_device::write_dtr));
	sio3.out_rtsb_callback().set("rs2323b", FUNC(rs232_port_device::write_rts));
	sio3.out_int_callback().set_inputline(m_icscpu, INPUT_LINE_IRQ0);
	sio3.set_cputag(m_icscpu);

	rs232_port_device &rs2323a(RS232_PORT(config, "rs2323a", default_rs232_devices, nullptr));
	rs2323a.rxd_handler().set("sio3", FUNC(z80sio_device::rxa_w));
	rs2323a.dcd_handler().set("sio3", FUNC(z80sio_device::dcda_w));
	rs2323a.cts_handler().set("sio3", FUNC(z80sio_device::ctsa_w));

	rs232_port_device &rs2323b(RS232_PORT(config, "rs2323b", default_rs232_devices, nullptr));
	rs2323b.rxd_handler().set("sio3", FUNC(z80sio_device::rxb_w));
	rs2323b.dcd_handler().set("sio3", FUNC(z80sio_device::dcdb_w));
	rs2323b.cts_handler().set("sio3", FUNC(z80sio_device::ctsb_w));

	z80sio_device &sio4(Z80SIO(config, "sio4", 8_MHz_XTAL/2));
	sio4.out_txda_callback().set("rs2324a", FUNC(rs232_port_device::write_txd));
	sio4.out_dtra_callback().set("rs2324a", FUNC(rs232_port_device::write_dtr));
	sio4.out_rtsa_callback().set("rs2324a", FUNC(rs232_port_device::write_rts));
	sio4.out_txdb_callback().set("rs2324b", FUNC(rs232_port_device::write_txd));
	sio4.out_dtrb_callback().set("rs2324b", FUNC(rs232_port_device::write_dtr));
	sio4.out_rtsb_callback().set("rs2324b", FUNC(rs232_port_device::write_rts));
	sio4.out_int_callback().set_inputline(m_icscpu, INPUT_LINE_IRQ0);
	sio4.set_cputag(m_icscpu);

	rs232_port_device &rs2324a(RS232_PORT(config, "rs2324a", default_rs232_devices, nullptr));
	rs2324a.rxd_handler().set("sio4", FUNC(z80sio_device::rxa_w));
	rs2324a.dcd_handler().set("sio4", FUNC(z80sio_device::dcda_w));
	rs2324a.cts_handler().set("sio4", FUNC(z80sio_device::ctsa_w));

	rs232_port_device &rs2324b(RS232_PORT(config, "rs2324b", default_rs232_devices, nullptr));
	rs2324b.rxd_handler().set("sio4", FUNC(z80sio_device::rxb_w));
	rs2324b.dcd_handler().set("sio4", FUNC(z80sio_device::dcdb_w));
	rs2324b.cts_handler().set("sio4", FUNC(z80sio_device::ctsb_w));
}

void acs8600_ics_device::device_resolve_objects()
{
	m_out_irq1_func.resolve_safe();
	m_out_irq2_func.resolve_safe();
}

void acs8600_ics_device::device_start()
{
}


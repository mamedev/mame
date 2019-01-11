// license:BSD-3-Clause
// copyright-holders:Carl

// TODO: multibus

#include "emu.h"
#include "machine/acs8600_ics.h"
#include "cpu/z80/z80.h"
#include "machine/z80sio.h"
#include "machine/am9513.h"
#include "bus/rs232/rs232.h"

DEFINE_DEVICE_TYPE(ACS8600_ICS, acs8600_ics_device, "acs8600_ics", "Altos ACS8600 Intelligent Serial Concentrator")

acs8600_ics_device::acs8600_ics_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ACS8600_ICS, tag, owner, clock),
	m_icscpu(*this, "icscpu"),
	m_out_irq1_func(*this),
	m_out_irq2_func(*this)
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

WRITE8_MEMBER(acs8600_ics_device::hiaddr_w)
{
	m_hiaddr = data;
}

WRITE8_MEMBER(acs8600_ics_device::ctrl_w)
{
	m_ctrl = data;
	m_out_irq1_func(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER(acs8600_ics_device::hostram_r)
{
	return m_maincpu_mem->read_byte((m_hiaddr << 16) | (BIT(m_ctrl, 0) << 15) | (offset & 0x7fff));
}

WRITE8_MEMBER(acs8600_ics_device::hostram_w)
{
	m_maincpu_mem->write_byte((m_hiaddr << 16) | (BIT(m_ctrl, 0) << 15) | (offset & 0x7fff), data);
}

WRITE_LINE_MEMBER(acs8600_ics_device::attn_w)
{
	m_icscpu->set_input_line(INPUT_LINE_NMI, state);
}

static DEVICE_INPUT_DEFAULTS_START(altos8600_terminal)
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void acs8600_ics_device::ics_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("icscpu", 0);
	map(0x1000, 0x17ff).ram();
	map(0x8000, 0xffff).rw(this, FUNC(acs8600_ics_device::hostram_r), FUNC(acs8600_ics_device::hostram_w));
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
	map(0x18, 0x18).w(this, FUNC(acs8600_ics_device::ctrl_w));
	map(0x1c, 0x1c).w(this, FUNC(acs8600_ics_device::hiaddr_w));
}

static const z80_daisy_config ics_daisy_chain[] =
{
	"sio1",
	"sio2",
	"sio3",
	"sio4",
	nullptr
};

MACHINE_CONFIG_START(acs8600_ics_device::device_add_mconfig)
	MCFG_DEVICE_ADD("icscpu", Z80, XTAL(4'000'000))
	MCFG_DEVICE_PROGRAM_MAP(ics_mem)
	MCFG_DEVICE_IO_MAP(ics_io)
	MCFG_Z80_DAISY_CHAIN(ics_daisy_chain)

	MCFG_DEVICE_ADD("stc1", AM9513, XTAL(1'843'200))
	MCFG_AM9513_OUT1_CALLBACK(WRITELINE("sio1", z80sio_device, rxca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("sio1", z80sio_device, txca_w))
	MCFG_AM9513_OUT2_CALLBACK(WRITELINE("sio1", z80sio_device, rxtxcb_w))
	MCFG_AM9513_OUT3_CALLBACK(WRITELINE("sio2", z80sio_device, rxca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("sio2", z80sio_device, txca_w))
	MCFG_AM9513_OUT4_CALLBACK(WRITELINE("sio2", z80sio_device, rxtxcb_w))
	MCFG_AM9513_OUT5_CALLBACK(WRITELINE("sio3", z80sio_device, rxca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("sio3", z80sio_device, txca_w))
	MCFG_DEVICE_ADD("stc2", AM9513, XTAL(1'843'200))
	MCFG_AM9513_OUT1_CALLBACK(WRITELINE("sio3", z80sio_device, rxtxcb_w))
	MCFG_AM9513_OUT2_CALLBACK(WRITELINE("sio4", z80sio_device, rxca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("sio4", z80sio_device, txca_w))
	MCFG_AM9513_OUT3_CALLBACK(WRITELINE("sio4", z80sio_device, rxtxcb_w))

	MCFG_DEVICE_ADD("sio1", Z80SIO, XTAL(8'000'000)/2)
	MCFG_Z80SIO_OUT_TXDA_CB(WRITELINE("rs2321a", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRA_CB(WRITELINE("rs2321a", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSA_CB(WRITELINE("rs2321a", rs232_port_device, write_rts))
	MCFG_Z80SIO_OUT_TXDB_CB(WRITELINE("rs2321b", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRB_CB(WRITELINE("rs2321b", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSB_CB(WRITELINE("rs2321b", rs232_port_device, write_rts))
	MCFG_Z80SIO_OUT_INT_CB(INPUTLINE("icscpu", INPUT_LINE_IRQ0))
	MCFG_Z80SIO_CPU("icscpu")
	MCFG_DEVICE_ADD("rs2321a", RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio1", z80sio_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio1", z80sio_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio1", z80sio_device, ctsa_w))
	MCFG_SLOT_OPTION_DEVICE_INPUT_DEFAULTS("terminal", altos8600_terminal)

	MCFG_DEVICE_ADD("rs2321b", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio1", z80sio_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio1", z80sio_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio1", z80sio_device, ctsb_w))

	MCFG_DEVICE_ADD("sio2", Z80SIO, XTAL(8'000'000)/2)
	MCFG_Z80SIO_OUT_TXDA_CB(WRITELINE("rs2322a", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRA_CB(WRITELINE("rs2322a", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSA_CB(WRITELINE("rs2322a", rs232_port_device, write_rts))
	MCFG_Z80SIO_OUT_TXDB_CB(WRITELINE("rs2322b", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRB_CB(WRITELINE("rs2322b", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSB_CB(WRITELINE("rs2322b", rs232_port_device, write_rts))
	MCFG_Z80SIO_OUT_INT_CB(INPUTLINE("icscpu", INPUT_LINE_IRQ0))
	MCFG_Z80SIO_CPU("icscpu")
	MCFG_DEVICE_ADD("rs2322a", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio2", z80sio_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio2", z80sio_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio2", z80sio_device, ctsa_w))

	MCFG_DEVICE_ADD("rs2322b", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio2", z80sio_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio2", z80sio_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio2", z80sio_device, ctsb_w))

	MCFG_DEVICE_ADD("sio3", Z80SIO, XTAL(8'000'000)/2)
	MCFG_Z80SIO_OUT_TXDA_CB(WRITELINE("rs2323a", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRA_CB(WRITELINE("rs2323a", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSA_CB(WRITELINE("rs2323a", rs232_port_device, write_rts))
	MCFG_Z80SIO_OUT_TXDB_CB(WRITELINE("rs2323b", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRB_CB(WRITELINE("rs2323b", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSB_CB(WRITELINE("rs2323b", rs232_port_device, write_rts))
	MCFG_Z80SIO_OUT_INT_CB(INPUTLINE("icscpu", INPUT_LINE_IRQ0))
	MCFG_Z80SIO_CPU("icscpu")
	MCFG_DEVICE_ADD("rs2323a", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio3", z80sio_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio3", z80sio_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio3", z80sio_device, ctsa_w))

	MCFG_DEVICE_ADD("rs2323b", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio3", z80sio_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio3", z80sio_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio3", z80sio_device, ctsb_w))

	MCFG_DEVICE_ADD("sio4", Z80SIO, XTAL(8'000'000)/2)
	MCFG_Z80SIO_OUT_TXDA_CB(WRITELINE("rs2324a", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRA_CB(WRITELINE("rs2324a", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSA_CB(WRITELINE("rs2324a", rs232_port_device, write_rts))
	MCFG_Z80SIO_OUT_TXDB_CB(WRITELINE("rs2324b", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRB_CB(WRITELINE("rs2324b", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSB_CB(WRITELINE("rs2324b", rs232_port_device, write_rts))
	MCFG_Z80SIO_OUT_INT_CB(INPUTLINE("icscpu", INPUT_LINE_IRQ0))
	MCFG_Z80SIO_CPU("icscpu")
	MCFG_DEVICE_ADD("rs2324a", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio4", z80sio_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio4", z80sio_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio4", z80sio_device, ctsa_w))

	MCFG_DEVICE_ADD("rs2324b", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio4", z80sio_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio4", z80sio_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio4", z80sio_device, ctsb_w))
MACHINE_CONFIG_END

void acs8600_ics_device::device_start()
{
	m_maincpu_mem = &machine().device<cpu_device>(m_maincpu_tag)->space(AS_PROGRAM);
	m_out_irq1_func.resolve_safe();
	m_out_irq2_func.resolve_safe();
}


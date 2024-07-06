// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Multitech I/O and Memory Board

    PIO Application Example  : G B000
    CTC Application Example  : G B100
    8251 Application Example : G B300

***************************************************************************/

#include "emu.h"
#include "iom.h"
#include "machine/i8251.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "bus/rs232/rs232.h"


namespace {

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START(mpf_iom_ip)
	ROM_REGION(0x1000, "rom", 0)
	ROM_LOAD("ip-iom.bin", 0x0000, 0x1000, CRC(bf789c58) SHA1(2c3c26290e041e913d00d7e0d65d53480e27187c))
ROM_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START(mpf_iom_ip)
	PORT_START("DIPSW")
	PORT_DIPNAME(0xf, 0xa, "Baud Rate")
	PORT_DIPSETTING(0x0, "50")
	PORT_DIPSETTING(0x1, "75")
	PORT_DIPSETTING(0x2, "110")
	PORT_DIPSETTING(0x3, "150")
	PORT_DIPSETTING(0x4, "200")
	PORT_DIPSETTING(0x5, "300")
	PORT_DIPSETTING(0x6, "600")
	PORT_DIPSETTING(0x7, "1200")
	PORT_DIPSETTING(0x8, "2400")
	PORT_DIPSETTING(0x9, "4800")
	PORT_DIPSETTING(0xa, "9600")
INPUT_PORTS_END


static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END


//-------------------------------------------------
//  mpf_iom_device - constructor
//-------------------------------------------------

class mpf_iom_device : public device_t, public device_mpf1_exp_interface
{
public:
	mpf_iom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, MPF_IOM_IP, tag, owner, clock)
		, device_mpf1_exp_interface(mconfig, *this)
		, m_rom(*this, "rom")
		, m_ctc(*this, "ctc")
		, m_pio(*this, "pio")
		, m_usart(*this, "uart")
	{
	}

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		Z80PIO(config, m_pio, DERIVED_CLOCK(1, 1));
		m_pio->out_int_callback().set(DEVICE_SELF_OWNER, FUNC(mpf1_exp_device::int_w));

		Z80CTC(config, m_ctc, DERIVED_CLOCK(1, 1));
		m_ctc->set_clk<2>(DERIVED_CLOCK(1, 1));
		m_ctc->zc_callback<2>().set(m_usart, FUNC(i8251_device::write_txc));
		m_ctc->zc_callback<2>().append(m_usart, FUNC(i8251_device::write_rxc));
		m_ctc->intr_callback().set(DEVICE_SELF_OWNER, FUNC(mpf1_exp_device::int_w));

		I8251(config, m_usart, DERIVED_CLOCK(1, 1));
		m_usart->rxrdy_handler().set(m_ctc, FUNC(z80ctc_device::trg3));
		m_usart->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
		m_usart->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
		m_usart->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

		rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
		rs232.rxd_handler().set(m_usart, FUNC(i8251_device::write_rxd));
		rs232.cts_handler().set(m_usart, FUNC(i8251_device::write_cts));
		rs232.dsr_handler().set(m_usart, FUNC(i8251_device::write_dsr));
		rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	}

	virtual const tiny_rom_entry *device_rom_region() const override
	{
		return ROM_NAME(mpf_iom_ip);
	}

	virtual ioport_constructor device_input_ports() const override
	{
		return INPUT_PORTS_NAME(mpf_iom_ip);
	}

	virtual void device_start() override
	{
		m_ram = make_unique_clear<uint8_t[]>(0x1800);

		save_pointer(NAME(m_ram), 0x1800);
	}

	virtual void device_reset() override
	{
		program_space().install_rom(0xb000, 0xbfff, m_rom->base());
		program_space().install_ram(0xd800, 0xefff, m_ram.get());

		io_space().install_readwrite_handler(0x60, 0x61, emu::rw_delegate(*m_usart, FUNC(i8251_device::read)), emu::rw_delegate(*m_usart, FUNC(i8251_device::write)));
		io_space().install_readwrite_handler(0x64, 0x67, emu::rw_delegate(*m_ctc, FUNC(z80ctc_device::read)), emu::rw_delegate(*m_ctc, FUNC(z80ctc_device::write)));
		io_space().install_readwrite_handler(0x68, 0x6b, emu::rw_delegate(*m_pio, FUNC(z80pio_device::read)), emu::rw_delegate(*m_pio, FUNC(z80pio_device::write)));
		io_space().install_read_port(0x6c, 0x6c, "DIPSW");
	}

private:
	required_memory_region m_rom;
	required_device<z80ctc_device> m_ctc;
	required_device<z80pio_device> m_pio;
	required_device<i8251_device> m_usart;

	std::unique_ptr<uint8_t[]> m_ram;
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MPF_IOM_IP, device_mpf1_exp_interface, mpf_iom_device, "mpf1_iom_ip", "Multitech IOM-MPF-IP (I/O and Memory Board)")

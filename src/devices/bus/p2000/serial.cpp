// license:BSD-3-Clause
// copyright-holders:Bart Eversdijk
/**********************************************************************

    P2000 V.24 Serial Cartridge(s)

**********************************************************************/

#include "emu.h"
#include "serial.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(P2000_P2174V24, p2000_p2174_serial_device,          "p2kp2174", "P2000 P2174 V.24 Serial Interface")
DEFINE_DEVICE_TYPE(P2000_PTCV24,   p2000_v24serial_device,             "p2kv24",   "P2000 PTC V.24 Serial Interface")
DEFINE_DEVICE_TYPE(P2000_M2001V24, p2000_m2001_serial_device,          "p2km2001", "P2000 Miniware M2001 V.24 Serial Interface")
DEFINE_DEVICE_TYPE(P2000_VIEWDATA, p2000_p2171_viewdata_serial_device, "p2kp2171", "P2000 P2171-1 Viewdata Communicator")

static DEVICE_INPUT_DEFAULTS_START( v24 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

//**************************************************************************
//  PTC V.24 Serial Interface Cartridge
//**************************************************************************
//-------------------------------------------------
//  p2000_v24serial_device - constructor
//-------------------------------------------------
p2000_v24serial_device::p2000_v24serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, P2000_PTCV24, tag, owner, clock)
    , device_p2000_expansion_slot_card_interface(mconfig, *this)
        , m_usart(*this, "usart")
        , m_clkdivider(*this, "clkdiv")
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
void p2000_v24serial_device::device_add_mconfig(machine_config &config)
{
    I8251(config, m_usart, 2.5_MHz_XTAL);
	m_usart->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_usart->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_usart->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));

    PIT8253(config, m_clkdivider, 0);
    m_clkdivider->set_clk<0>(2.5_MHz_XTAL / 2); /*  P2000 clock / 2*/
    m_clkdivider->set_clk<1>(2.5_MHz_XTAL / 2);
    m_clkdivider->out_handler<0>().set(m_usart, FUNC(i8251_device::write_rxc));
    m_clkdivider->out_handler<1>().set(m_usart, FUNC(i8251_device::write_txc));

    rs232_port_device &rs232c(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232c.rxd_handler().set(m_usart, FUNC(i8251_device::write_rxd));
	rs232c.dsr_handler().set(m_usart, FUNC(i8251_device::write_dsr));
	rs232c.cts_handler().set(m_usart, FUNC(i8251_device::write_cts));
    rs232c.set_option_device_input_defaults("printer", DEVICE_INPUT_DEFAULTS_NAME(v24));
    rs232c.set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(v24));
	rs232c.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(v24));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void p2000_v24serial_device::device_start()
{
    m_slot->io_space().install_readwrite_handler(0x60, 0x63, read8sm_delegate(*m_clkdivider, FUNC(pit8253_device::read)), write8sm_delegate(*m_clkdivider, FUNC(pit8253_device::write)));
    m_slot->io_space().install_readwrite_handler(0x44, 0x45, read8sm_delegate(*m_usart, FUNC(i8251_device::read)), write8sm_delegate(*m_usart, FUNC(i8251_device::write)));
}

//**************************************************************************
//  M2001 V.24 Serial Interface Cartridge
//**************************************************************************
p2000_m2001_serial_device::p2000_m2001_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, P2000_M2001V24, tag, owner, clock)
    , device_p2000_expansion_slot_card_interface(mconfig, *this)
        , m_usart(*this, "usart")
        , m_clkdivider(*this, "clkdiv")
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
void p2000_m2001_serial_device::device_add_mconfig(machine_config &config)
{
    I8251(config, m_usart, 2.5_MHz_XTAL);
	m_usart->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_usart->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_usart->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));

    PIT8253(config, m_clkdivider, 0);
    m_clkdivider->set_clk<0>(2.5_MHz_XTAL);  /* P2000 clck */
    m_clkdivider->set_clk<1>(0);
    m_clkdivider->set_clk<2>(0);
    m_clkdivider->out_handler<0>().set(m_clkdivider, FUNC(pit8253_device::write_clk1));
    m_clkdivider->out_handler<0>().append(m_clkdivider, FUNC(pit8253_device::write_clk2));
    m_clkdivider->out_handler<1>().set(m_usart, FUNC(i8251_device::write_rxc));
    m_clkdivider->out_handler<2>().set(m_usart, FUNC(i8251_device::write_txc));
    
    rs232_port_device &rs232c(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232c.rxd_handler().set(m_usart, FUNC(i8251_device::write_rxd));
	rs232c.dsr_handler().set(m_usart, FUNC(i8251_device::write_dsr));
	rs232c.cts_handler().set(m_usart, FUNC(i8251_device::write_cts));
    rs232c.set_option_device_input_defaults("printer", DEVICE_INPUT_DEFAULTS_NAME(v24));
    rs232c.set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(v24));
	rs232c.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(v24));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void p2000_m2001_serial_device::device_start()
{
    m_slot->io_space().install_readwrite_handler(0x40, 0x43, read8sm_delegate(*m_clkdivider, FUNC(pit8253_device::read)), write8sm_delegate(*m_clkdivider, FUNC(pit8253_device::write)));
    m_slot->io_space().install_readwrite_handler(0x44, 0x45, read8sm_delegate(*m_usart, FUNC(i8251_device::read)), write8sm_delegate(*m_usart, FUNC(i8251_device::write)));
}


//**************************************************************************
//  P2174 - Philips V.24/RS-232 Interface
//**************************************************************************
/*
    Dip switches
            SI-1     75
            SI-2    150
            SI-3    300
            SI-4    600
            SI-5   1200
            SI-6   2400
            SI-7   4800
            SI-7   9600
 */
INPUT_PORTS_START( p2000_p2471 )
    PORT_START("SI1")
	PORT_DIPNAME( 0xff, 0x08, "Baud rate" )  // 153600 pulses / baud => 16
    PORT_DIPSETTING(    0x01, "9600" ) // 153600 / (9600 * 16) = 1
    PORT_DIPSETTING(    0x02, "4800")  // 153600 / (4800 * 16) = 2
	PORT_DIPSETTING(    0x04, "2400")  // 153600 / (2400 * 16) = 4
	PORT_DIPSETTING(    0x08, "1200")  // 153600 / (1200 * 16) = 8
	PORT_DIPSETTING(    0x10, "600")   // 153600 / (600 * 16) = 16
	PORT_DIPSETTING(    0x20, "300")   // 153600 / (300 * 16) = 32
	PORT_DIPSETTING(    0x40, "150")   // 153600 / (150 * 16) = 64
	PORT_DIPSETTING(    0x80, "75")    // 153600 / (75 * 16) = 128
 
    PORT_START("SI2")
	PORT_DIPNAME(   0x01, 0x00, "SI2-8") PORT_DIPLOCATION("SI2:1")
	PORT_DIPSETTING(0x01, "Disable")
	PORT_DIPSETTING(0x00, "Enable")
    PORT_DIPNAME(   0x02, 0x00, "SI2-7") PORT_DIPLOCATION("SI2:2")
	PORT_DIPSETTING(0x02, "Disable")
	PORT_DIPSETTING(0x00, "Enable")
    PORT_DIPNAME(   0x04, 0x00, "SI2-6") PORT_DIPLOCATION("SI2:3")
	PORT_DIPSETTING(0x04, "Disable")
	PORT_DIPSETTING(0x00, "Enable")
    PORT_DIPNAME(   0x08, 0x00, "SI2-5") PORT_DIPLOCATION("SI2:4")
	PORT_DIPSETTING(0x08, "Disable")
	PORT_DIPSETTING(0x00, "Enable")
    PORT_DIPNAME(   0x10, 0x00, "SI2-4") PORT_DIPLOCATION("SI2:5")
	PORT_DIPSETTING(0x10, "Disable")
	PORT_DIPSETTING(0x00, "Enable")
    PORT_DIPNAME(   0x20, 0x00, "SI2-3") PORT_DIPLOCATION("SI2:6")
	PORT_DIPSETTING(0x20, "Disable")
	PORT_DIPSETTING(0x00, "Enable")
    PORT_DIPNAME(   0x40, 0x00, "SI2-2") PORT_DIPLOCATION("SI2:7")
	PORT_DIPSETTING(0x40, "Disable")
	PORT_DIPSETTING(0x00, "Enable")
    PORT_DIPNAME(   0x80, 0x00, "SI2-1") PORT_DIPLOCATION("SI2:8")
	PORT_DIPSETTING(0x80, "Disable")
	PORT_DIPSETTING(0x00, "Enable")
INPUT_PORTS_END


p2000_p2174_serial_device::p2000_p2174_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
   	: device_t(mconfig, P2000_P2174V24, tag, owner, clock)
    , device_p2000_expansion_slot_card_interface(mconfig, *this)
        , m_usart(*this, "usart")
        , m_rs232(*this, "rs232")
        , m_dsw1(*this, "SI1") 
    	, m_dsw2(*this, "SI2")
        , m_usart_divide_counter(0)
    	, m_usart_clock_state(0)
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
void p2000_p2174_serial_device::device_add_mconfig(machine_config &config)
{
    I8251(config, m_usart, 2.5_MHz_XTAL);
	m_usart->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_usart->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_usart->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));

    // feed usart with 16 times the baudrate max baud is 9600
    // = 9600 * 16 = 153600 clock, closest valid value =>  38.4_kHz_XTAL * 4 
    clock_device &usart_clock(CLOCK(config, "usart_clock", 38.4_kHz_XTAL * 4));
	usart_clock.signal_handler().set(FUNC(p2000_p2174_serial_device::usart_clock_tick));
    
    RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_usart, FUNC(i8251_device::write_rxd));
	m_rs232->dsr_handler().set(m_usart, FUNC(i8251_device::write_dsr));
	m_rs232->cts_handler().set(m_usart, FUNC(i8251_device::write_cts));
    m_rs232->set_option_device_input_defaults("printer", DEVICE_INPUT_DEFAULTS_NAME(v24));
    m_rs232->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(v24));
	m_rs232->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(v24));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void p2000_p2174_serial_device::device_start()
{
    m_slot->io_space().install_readwrite_handler(0x40, 0x41, read8sm_delegate(*m_usart, FUNC(i8251_device::read)), write8sm_delegate(*m_usart, FUNC(i8251_device::write)));
    
    m_slot->io_space().install_read_handler(0x60, 0x60, read8smo_delegate(*this, FUNC(p2000_p2174_serial_device::port_60_r)));
    m_slot->io_space().install_read_handler(0x61, 0x61, read8smo_delegate(*this, FUNC(p2000_p2174_serial_device::port_60_r))); // port 61 == 60
    m_slot->io_space().install_read_handler(0x62, 0x62, read8smo_delegate(*this, FUNC(p2000_p2174_serial_device::port_62_r)));
    m_slot->io_space().install_read_handler(0x63, 0x63, read8smo_delegate(*this, FUNC(p2000_p2174_serial_device::port_62_r))); // port 63 == 62
}

//-------------------------------------------------
//  generate uart clock pulse
//-------------------------------------------------
WRITE_LINE_MEMBER( p2000_p2174_serial_device::usart_clock_tick )
{
	uint8_t old_counter = m_usart_divide_counter;
	m_usart_divide_counter++;

	uint8_t transition = (old_counter ^ m_usart_divide_counter) & m_dsw1->read();
	if (transition)
	{
		m_usart->write_txc(m_usart_clock_state);
		m_usart->write_rxc(m_usart_clock_state);
		m_usart_clock_state ^= 1;
	}
}

//-------------------------------------------------
//  input_ports - device-specific dipswitch ports
//-------------------------------------------------
ioport_constructor p2000_p2174_serial_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( p2000_p2471 );
}

//-------------------------------------------------
//  Handle input lines
//  BIT 
//    2   RI 
//    3   DCD 
//-------------------------------------------------
uint8_t p2000_p2174_serial_device::port_60_r()
{
    uint8_t data = 0;
    data |= m_rs232->ri_r()  ? 0x04 : 0x00;
    data |= m_rs232->dcd_r() ? 0x08 : 0x00;
    return data;
}

//-------------------------------------------------
//  handle io port dip - switch 2
//-------------------------------------------------
uint8_t p2000_p2174_serial_device::port_62_r()
{
    return m_dsw2->read();
}

//**************************************************************************
//  P2171-1 Viewdadata V.24 Serial Interface Cartridge
//**************************************************************************

p2000_p2171_viewdata_serial_device::p2000_p2171_viewdata_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
   	: device_t(mconfig, P2000_VIEWDATA, tag, owner, clock)
    , device_p2000_expansion_slot_card_interface(mconfig, *this)
        , m_usart(*this, "usart")
        , m_rs232(*this, "rs232")
        , m_usart_divide_counter(0)
    	, m_usart_clock_state(0)
{       
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
void p2000_p2171_viewdata_serial_device::device_add_mconfig(machine_config &config)
{
    I8251(config, m_usart, 2.5_MHz_XTAL);
	m_usart->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_usart->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_usart->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
    
    // Feed usart with 16 times the baudrate = 1200 * 16 = 19200 clock pulses 
    clock_device &clock(CLOCK(config, "usart_clock",  38.4_kHz_XTAL / 2));
	clock.signal_handler().set(m_usart, FUNC(i8251_device::write_txc));
    clock.signal_handler().append(m_usart, FUNC(i8251_device::write_rxc));
    
    RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_usart, FUNC(i8251_device::write_rxd));
	m_rs232->dsr_handler().set(m_usart, FUNC(i8251_device::write_dsr));
	m_rs232->cts_handler().set(m_usart, FUNC(i8251_device::write_cts));
    m_rs232->set_option_device_input_defaults("printer", DEVICE_INPUT_DEFAULTS_NAME(v24));
    m_rs232->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(v24));
	m_rs232->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(v24));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void p2000_p2171_viewdata_serial_device::device_start()
{
    m_slot->io_space().install_readwrite_handler(0x40, 0x41, read8sm_delegate(*m_usart, FUNC(i8251_device::read)), write8sm_delegate(*m_usart, FUNC(i8251_device::write)));
    
    m_slot->io_space().install_write_handler(0x60, 0x6f, write8smo_delegate(*this, FUNC(p2000_p2171_viewdata_serial_device::port_606f_w)));
}

//-------------------------------------------------
//  Handle input lines
//  BIT
//    6   DRS  --> usart
//    7   RTS2 --> rs232 // not implemented in RS232 device
//-------------------------------------------------
void p2000_p2171_viewdata_serial_device::port_606f_w(uint8_t data)
{
    m_usart->write_dsr(BIT(data, 6));
}

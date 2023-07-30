// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

  IBM PS/2 Planar UART.

  16550 UART. Can be remapped to COM1/COM2 or disabled entirely.

***************************************************************************/

#include "emu.h"
#include "planar_uart.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"

#define VERBOSE 1
#include "logmacro.h"

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MCA16_PLANAR_UART, mca16_planar_uart_device, "mca16_planar_uart", "IBM PS/2 Planar UART")

static void mca_com(device_slot_interface &device)
{
    device.option_add("microsoft_mouse", MSFT_HLE_SERIAL_MOUSE);
	device.option_add("logitech_mouse", LOGITECH_HLE_SERIAL_MOUSE);
	device.option_add("wheel_mouse", WHEEL_HLE_SERIAL_MOUSE);
	device.option_add("msystems_mouse", MSYSTEMS_HLE_SERIAL_MOUSE);
	device.option_add("rotatable_mouse", ROTATABLE_HLE_SERIAL_MOUSE);
	device.option_add("terminal", SERIAL_TERMINAL);
	device.option_add("null_modem", NULL_MODEM);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mca16_planar_uart_device::device_add_mconfig(machine_config &config)
{
	NS16550(config, m_uart, XTAL(1'843'200));
	m_uart->out_tx_callback().set("serport0", FUNC(rs232_port_device::write_txd));
	m_uart->out_dtr_callback().set("serport0", FUNC(rs232_port_device::write_dtr));
	m_uart->out_rts_callback().set("serport0", FUNC(rs232_port_device::write_rts));
    m_uart->out_int_callback().set([this] (int state)
	{
        if (m_is_mapped)
        {
            if (m_cur_irq == 3) m_mca->ireq_w<3>(state);
            if (m_cur_irq == 4) m_mca->ireq_w<4>(state);
        }
	});

	rs232_port_device &serport0(RS232_PORT(config, "serport0", mca_com, nullptr));
	serport0.rxd_handler().set(m_uart, FUNC(ins8250_uart_device::rx_w));
	serport0.dcd_handler().set(m_uart, FUNC(ins8250_uart_device::dcd_w));
	serport0.dsr_handler().set(m_uart, FUNC(ins8250_uart_device::dsr_w));
	serport0.ri_handler().set(m_uart, FUNC(ins8250_uart_device::ri_w));
	serport0.cts_handler().set(m_uart, FUNC(ins8250_uart_device::cts_w));
}

//-------------------------------------------------
//  mca16_planar_uart_device - constructor
//-------------------------------------------------

mca16_planar_uart_device::mca16_planar_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca16_planar_uart_device(mconfig, MCA16_PLANAR_UART, tag, owner, clock)
{
}

mca16_planar_uart_device::mca16_planar_uart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_mca16_card_interface(mconfig, *this, 0xffff),
	m_uart(*this, "uart")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mca16_planar_uart_device::device_start()
{
	set_mca_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mca16_planar_uart_device::device_reset()
{
	m_cur_io_start = m_cur_io_end = 0;
    m_cur_irq = 0;
    m_is_mapped = 0;
}

void mca16_planar_uart_device::enable()
{
    LOG("%s\n", FUNCNAME);

    planar_remap(AS_IO, m_cur_io_start, m_cur_io_end);
    m_is_mapped = 1;
}

void mca16_planar_uart_device::disable()
{
    LOG("%s\n", FUNCNAME);

    m_mca->unmap_device(m_cur_io_start, m_cur_io_end);
    m_is_mapped = 0;
}

uint8_t mca16_planar_uart_device::io8_r(offs_t offset)
{
    //assert_card_feedback();
    return m_uart->ins8250_r(offset);
}

void mca16_planar_uart_device::io8_w(offs_t offset, uint8_t data)
{
    //assert_card_feedback();
    m_uart->ins8250_w(offset, data);
}

void mca16_planar_uart_device::planar_remap(int space_id, offs_t start, offs_t end)
{
    // LOG("%s\n", FUNCNAME);

    if(space_id == AS_IO)
    {
        LOG("%s mapped to %04X-%04X\n", FUNCNAME, start, end);
        if(m_is_mapped) m_mca->unmap_device(m_cur_io_start, m_cur_io_end);
        
        m_mca->install_device(start, end, 
            read8sm_delegate(*this, FUNC(mca16_planar_uart_device::io8_r)),
            write8sm_delegate(*this, FUNC(mca16_planar_uart_device::io8_w)));
        m_cur_io_start = start;
        m_cur_io_end = end;
        m_is_mapped = true;
    }
    else
    {
            fatalerror("Tried to unmap I/O device from RAM space\n");
    }
}

void mca16_planar_uart_device::planar_remap_irq(uint8_t new_irq)
{
    LOG("%s\n", FUNCNAME);
    m_cur_irq = new_irq;
}
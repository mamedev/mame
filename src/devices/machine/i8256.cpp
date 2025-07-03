#include "emu.h"
#include "i8256.h"

#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(I8256, i8256_device, "intel_8256", "Intel 8256AH MULTIFUNCTION MICROPROCESSOR SUPPORT CONTROLLER")

i8256_device::i8256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
    : device_t(mconfig, I8256, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_extint_cb(*this),
	m_inta_cb(*this),
	m_in_p1_cb(*this, 0),
	m_in_p2_cb(*this, 0),
	m_out_p1_cb(*this),	
	m_out_p2_cb(*this),
	m_cts(1),
	m_rxd(1),
	m_rxc(0),
	m_txc(0)
{
}

void i8256_device::device_start()
{

}

void i8256_device::device_reset()
{
    m_command1 = 0;
    m_command2 = 0;
    m_command3 = 0;
    m_mode = 0;
    m_port1_control = 0;
    m_interrupts = 0;
	m_status = 0x30;
}

uint8_t i8256_device::read(offs_t offset)
{
    // In the 8-bit mode, AD0-AD3 are used to select the proper register, while AD1-AD4 are used in the 16-bit mode.
    // AD4 in the 8-bit mote is ignored as an address, while AD0 in the 16-bit mode is used as a second chip select, active low.
    if (BIT(m_command1,CMD1_8086))
	{
		offset = offset >> 1;
	}	
    	
	u8 reg = offset & 0x0F;

	switch (reg)
	{
		case REG_CMD1:
            return m_command1;
		case REG_CMD2:
            return m_command2;
		case REG_CMD3:
			return m_command3 & 0x76; // When command Register 3 is read, bits 0, 3, and 7 will always be zero.
		case REG_MODE:
           return m_mode;
		case REG_INTEN:
			return m_interrupts;
		case REG_PORT1C:
            return m_port1_control;
		case REG_PORT1:
			return m_port1_int;
		case REG_PORT2:
			return m_port2_int;
		case REG_STATUS:
			return m_status;
		default:
			LOG("I8256 Read unmapped register: %u\n", reg);
			return 0xFF;
	};
}

void i8256_device::write(offs_t offset, u8 data)
{

	u8 reg = offset & 0x0F;

    // In the 8-bit mode, AD0-AD3 are used to select the proper register, while AD1-AD4 are used in the 16-bit mode.
    // AD4 in the 8-bit mote is ignored as an address.

    if (BIT(m_command1,CMD1_8086))
	{
		if (!BIT(offset,0)) // AD0 in the 16-bit mode is used as a second chip select, active low.
		{
			reg = (offset >> 1) & 0x0F;
		} else {
			return;
		}
	}
        
	switch (reg)
	{
		case REG_CMD1:
			if (m_command1 != data) {
					m_command1 = data;
				if (BIT(m_command1,CMD1_8086))
				{
					LOG("I8256 Enabled 8086 mode\n");
				}
				LOG("I8256 Character length: %u\n", 8-(m_command1 & 0xC0));
			}            
			break;
		case REG_CMD2:
			if (m_command2 != data) {
            	m_command2 = data;
				LOG("I8256 Baud rate: %u\n", baudRates[m_command2 & 0x0F]);
				LOG("I8256 Clock Scale: %u\n", sysclockDivider[(m_command2 & 0x30 >> 4)]);
				if((clock() / sysclockDivider[(m_command2 & 0x30 >> 4)])!=1024000)
				{				
					LOG("I8256 Internal Clock should be 1024000, calculated: %u\n", (clock() / sysclockDivider[(m_command2 & 0x30 >> 4)]));
				}
			}
			break;
		case REG_CMD3:
			m_command3 = data;
			if (BIT(m_command3,CMD3_RST)) {
				m_interrupts = 0;
				m_status = 0x30;
			}
            break;
		case REG_INTEN:
			m_interrupts = m_interrupts | data;
			LOG("I8256 Enabled interrupts: %u\n", m_interrupts);
			break;
		case REG_INTAD: // reset interrupt
			m_interrupts = m_interrupts & ~data;
			break;
		case REG_MODE:
            m_mode = data;
			break;
		case REG_PORT1C:
            m_port1_control = data;
			break;
		case REG_PORT1:
			m_port1_int = data;
			break;
		case REG_PORT2:
			m_port2_int = data;
			break;
		default:
			LOG("I8256 Unmapped write %02x to %02x\n", data, reg);
			break;
	};
}

uint8_t i8256_device::p1_r()
{
    // if control bit is 0 (input), read from callback else use output latch
    uint8_t input = m_in_p1_cb(0);
    uint8_t result = 0;
    for (int i = 0; i < 8; i++) {
        if (BIT(m_port1_control, i)) // output
            result |= (m_port1_int & (1 << i));
        else // input
            result |= (input & (1 << i));
    }
    return result;
}

void i8256_device::p1_w(uint8_t data)
{
    m_port1_int = (m_port1_int & ~m_port1_control) | (data & m_port1_control);
    m_out_p1_cb(0, m_port1_int & m_port1_control);
}

uint8_t i8256_device::p2_r()
{
    uint8_t p2c = m_mode & 0x03;
    if (p2c == PORT2C_II || p2c == PORT2C_IO)
        return m_in_p2_cb(0);
    else
        return m_port2_int;
}

void i8256_device::p2_w(uint8_t data)
{
    uint8_t p2c = m_mode & 0x03;
    m_port2_int = data;
    uint8_t port2_data = 0;
    switch (p2c)
    {
        case PORT2C_IO: port2_data = m_port2_int & 0x0F; break;
        case PORT2C_OI: port2_data = m_port2_int & 0xF0; break;
        case PORT2C_OO: port2_data = m_port2_int; break;
        default: port2_data = 0; break;
    }
    if (p2c == PORT2C_IO || p2c == PORT2C_OI || p2c == PORT2C_OO)
        m_out_p2_cb(0, port2_data);
}

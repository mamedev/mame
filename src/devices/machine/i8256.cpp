#include "emu.h"
#include "i8256.h"

#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(I8256, i8256_device, "intel_8256", "Intel 8256AH MULTIFUNCTION MICROPROCESSOR SUPPORT CONTROLLER")

i8256_device::i8256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
    : device_t(mconfig, I8256, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_in_inta_cb(*this, 0),
	m_out_int_cb(*this),
	m_in_extint_cb(*this, 0),	
	m_in_p1_cb(*this, 0),
	m_in_p2_cb(*this, 0),
	m_out_p1_cb(*this),
	m_out_p2_cb(*this),
	m_rxc(0),
	m_rxd(1),
	m_cts(1),	
	m_txc(0),
	m_timer(nullptr)
{
}

void i8256_device::device_start()
{
	save_item(NAME(m_command1));
	save_item(NAME(m_command2));
	save_item(NAME(m_command3));
	save_item(NAME(m_mode));
	save_item(NAME(m_port1_control));
	save_item(NAME(m_interrupts));
	save_item(NAME(m_status));

	m_timer = timer_alloc(FUNC(i8256_device::timer_check), this);
}

void i8256_device::device_reset()
{
    m_command1 = 0;
    m_command2 = 0;
    m_command3 = 0;
    m_mode = 0;
    m_port1_control = 0;
    m_interrupts = 0;

	m_tx_buffer = 0;
	m_rx_buffer = 0;

	m_timers[0] = 0;
	m_timers[1] = 0;
	m_timers[2] = 0;
	m_timers[3] = 0;
	m_timers[4] = 0;

	m_status = 0x30; // TRE and TBE
	m_port1_int = 0;
    m_port2_int = 0;

	m_timer->adjust(attotime::from_hz(16000), 0, attotime::from_hz(16000));
}

TIMER_CALLBACK_MEMBER(i8256_device::timer_check)
{
    for (int i = 0; i < 5; ++i) {
        if (m_timers[i] > 0) {
            m_timers[i]--;
			if (m_timers[i] == 0 && BIT(m_interrupts,timer_interrupt[i])) { // If the interrupt is enabled
				m_current_interrupt_level = timer_interrupt[i];
				m_out_int_cb(1); // it occurs when the counter changes from 1 to 0.
			}            	
        }
    }
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
		case REG_INTAD:
			m_out_int_cb(0);
			return m_current_interrupt_level*4;
		case REG_BUFFER:
			return m_rx_buffer;
		case REG_PORT1C:
            return m_port1_control;
		case REG_PORT1:
			return m_port1_int;
		case REG_PORT2:
			return m_port2_int;
		case REG_TIMER1:
		case REG_TIMER2:
		case REG_TIMER3:
		case REG_TIMER4:
		case REG_TIMER5:
			return m_timers[reg-10];
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
				
				if (BIT(m_command1,CMD1_FRQ))
				{
					m_timer->adjust(attotime::from_hz(1000), 0, attotime::from_hz(1000));
				} else {
					m_timer->adjust(attotime::from_hz(16000), 0, attotime::from_hz(16000));
				}
				
				if (BIT(m_command1,CMD1_8086))
				{
					LOG("I8256 Enabled 8086 mode\n");
				}

				m_data_bits_count = 8-((BIT(m_command1, CMD1_L0)) | (BIT(m_command1, CMD1_L1) << 1));
				m_stop_bits = stopBits[(BIT(m_command1, CMD1_S0)) | (BIT(m_command1, CMD1_S1) << 1)];

				set_data_frame(1, m_data_bits_count, m_parity, m_stop_bits);
			}            
			break;
		case REG_CMD2:
			if (m_command2 != data) {
            	m_command2 = data;

				set_rate(baudRates[m_command2 & 0x0F]);

				if (BIT(m_command2,CMD2_PEN))
					m_parity = BIT(m_command2,CMD2_EP) ? PARITY_EVEN : PARITY_ODD;
				else
					m_parity = PARITY_NONE;				

				set_data_frame(1, m_data_bits_count, m_parity, m_stop_bits);

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
		case REG_MODE:
            m_mode = data;
			break;
		case REG_PORT1C:
            m_port1_control = data;
			break;
		case REG_INTEN:
			m_interrupts = m_interrupts | data;
			LOG("I8256 Enabled interrupts: %u\n", m_interrupts);
			break;
		case REG_INTAD: // reset interrupt
			m_interrupts = m_interrupts & ~data;
			break;
		case REG_BUFFER:
			m_tx_buffer = data;
			break;
		case REG_PORT1:
			m_port1_int = data;
			break;
		case REG_PORT2:
			m_port2_int = data;
			break;
		case REG_TIMER1:
		case REG_TIMER2:
		case REG_TIMER3:
		case REG_TIMER4:
		case REG_TIMER5:
			m_timers[reg-10] = data;
			break;
		default:
			LOG("I8256 Unmapped write %02x to %02x\n", data, reg);
			break;
	};
}

void i8256_device::write_rxd(int state)
{
	m_rxd = state;
	LOG("8256: Presented a %d\n", m_rxd);
	//  device_serial_interface::rx_w(state);
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

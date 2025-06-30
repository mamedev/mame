
#include "emu.h"
#include "i8256.h"

DEFINE_DEVICE_TYPE(I8256, i8256_device, "intel_8256", "Intel 8256AH MULTIFUNCTION MICROPROCESSOR SUPPORT CONTROLLER")

i8256_device::i8256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
    : device_t(mconfig, I8256, tag, owner, clock),
	device_serial_interface(mconfig, *this),
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
}

void i8256_device::write(offs_t offset, u8 data)
{
    // In the 8-bit mode, AD0-AD3 are used to select the proper register, while AD1-AD4 are used in the 16-bit mode.
    // AD4 in the 8-bit mote is ignored as an address, while AD0 in the 16-bit mode is used as a second chip select, active low.
    if (BIT(m_command1,CMD1_8086))
        offset = offset >> 1;
	u8 reg = offset & 0x0F;

	switch (reg)
	{
		case I8256_REG_CMD1:
            m_command1 = data;
			break;
		case I8256_REG_CMD2:
            m_command2 = data;
			break;
		case I8256_REG_CMD3:
			m_command3 = data;
            break;
		case I8256_REG_MODE:
            m_mode = data;
			break;
		case I8256_REG_PORT1C:
            m_port1_control = data;
			break;
		case I8256_REG_INTEN:
			break;
		case I8256_REG_INTAD:
			break;
		case I8256_REG_BUFFER:
			break;
		case I8256_REG_PORT1:
			break;
		case I8256_REG_PORT2:
			break;
		case I8256_REG_TIMER1:
			break;
		case I8256_REG_TIMER2:
			break;
		case I8256_REG_TIMER3:
			break;
		case I8256_REG_TIMER4:
			break;
		case I8256_REG_TIMER5:
			break;
		case I8256_REG_STATUS:
			break;

	}
}

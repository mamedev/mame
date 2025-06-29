
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
    m_mode_byte = 0;
    m_port1_control = 0;
    m_interrupts = 0;
}

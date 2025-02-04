// license:BSD-2-Clause
// copyright-holders:Lubomir Rintel

/***************************************************************************

    H8 I2C controller emulation

    An I2C block present on H8/3337, H8/3437 and probably elsewhere.

    TODO:
    - Slave mode
    - Interrupts

***************************************************************************/

#include "emu.h"
#include "h8_i2c.h"

DEFINE_DEVICE_TYPE(H8_I2C, h8_i2c_device, "h8_i2c", "H8 I2C")

h8_i2c_device::h8_i2c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, H8_I2C, tag, owner, clock),
	m_scl_w(*this),
	m_sda_w(*this),
	m_sda_r(*this, 0)
{
}

void h8_i2c_device::iccr_w(u8 data)
{
	logerror("iccr_w %02x\n", data);
	m_iccr = data;
}

u8 h8_i2c_device::iccr_r()
{
	logerror("iccr_r %02x\n", m_iccr);
	return m_iccr;
}

void h8_i2c_device::icsr_w(u8 data)
{
	logerror("icsr_w %02x\n", data);

	if (BIT(m_iccr, 7)) { // I2C Enabled
		if (BIT(m_iccr, 5)) { // Master mode
			if (!BIT(data, 5)) { // Not suppressed
				if (BIT(data, 7)) {
					m_sda_w(0);
				} else {
					m_scl_w(1);
					m_sda_w(1);
				}
			}
		} else {
			logerror("Slave mode not implemented!\n");
		}
	}

	// Preserve these bits if we're writing 0 to them
	data |= m_icsr & (data & 0x4e);
	// Preserve these bits where they have not been read yet
	data |= m_icsr & (~m_icsr_read & 0x4e);
	// Always 1
	data |= 0x30;

	m_icsr = data;
}

u8 h8_i2c_device::icsr_r()
{
	m_icsr_read = m_icsr;
	if (BIT(m_iccr, 4)) {
		// If we're transmitting, ACK bit reflects current line state
		m_icsr_read &= 0xfe;
		m_icsr_read |= m_sda_r();
	}
	logerror("icsr_r %02x\n", m_icsr_read);

	return m_icsr_read;
}

void h8_i2c_device::icdr_w(u8 data)
{
	logerror("icdr_w %02x\n", data);

	if (BIT(m_iccr, 7)) { // I2C Enabled
		if (BIT(m_iccr, 5)) { // Master mode
			if (BIT(m_iccr, 4)) { // Not inhibited
				logerror("Transmitting [%02x].\n", m_icdr);

				for (int i = 0; i < 8; i++) {
					m_scl_w(0);
					m_sda_w((data & 0x80) ? 1 : 0);
					data = data << 1;
					m_scl_w(1);
				}

				// Receive the ACK bit.
				m_scl_w(0);
				m_sda_w(1);
				m_scl_w(1);
				m_icsr &= ~1;
				m_icsr |= m_sda_r();

				// Signal an interrupt
				m_icsr |= 1 << 6;
			} else {
				logerror("Data written but transmit not enabled!\n");
			}
		} else {
			logerror("Slave mode not implemented!\n");
		}
	}

	m_icdr = data;
}

u8 h8_i2c_device::icdr_r()
{
	u8 data = m_icdr;

	if (BIT(m_iccr, 7)) { // I2C Enabled
		if (BIT(m_iccr, 5)) { // Master mode
			if (!BIT(m_iccr, 4)) {	// Not inhibited
				m_icdr = 0;
				m_scl_w(0);
				for (int i = 0; i < 8; i++)
				{
					m_scl_w(1);
					m_icdr <<= 1;
					m_icdr |= m_sda_r();
					m_scl_w(0);
				}

				// ACK if enabled
				m_sda_w(m_icsr & 1);
				m_scl_w(1);

				// Signal an interrupt
				m_icsr |= 1 << 6;

				logerror("Received [%02x].\n", m_icdr);
			} else {
				// Do not receive more bytes, just return present one
				logerror("Not receiving more data.\n");
			}
		} else {
			logerror("Slave mode not implemented!\n");
		}
	}
	logerror("icdr_r %02x\n", data);

	return data;
}

void h8_i2c_device::icmr_sar_w(u8 data)
{
	logerror("icmr_sar_w %02x\n", data);

	// Pick register from the ICE bit
	if (BIT(m_iccr, 7))
		m_icmr = data;
	else
		m_sar = data;
}

u8 h8_i2c_device::icmr_sar_r()
{
	logerror("icmr_sar_r %02x\n", m_icdr);

	// Pick register from the ICE bit
	if (BIT(m_iccr, 7))
		return m_icmr;
	else
		return m_sar;
}

void h8_i2c_device::device_start()
{
	save_item(NAME(m_iccr));
	save_item(NAME(m_icsr));
	save_item(NAME(m_icdr));
	save_item(NAME(m_icmr));
	save_item(NAME(m_sar));
}

void h8_i2c_device::device_reset()
{
	m_icsr_read = 0;

	m_iccr = 0x00;
	m_icsr = 0x30;
	m_icdr = 0x00;
	m_icmr = 0x38;
	m_sar = 0x00;
}

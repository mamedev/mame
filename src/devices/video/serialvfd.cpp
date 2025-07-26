// license:BSD-3-Clause
// copyright-holders: NaokiS

#include "serialvfd.h"
#include "emu.h"

#define LOG_VFD     (1U << 1)
//#define VERBOSE     ( LOG_VFD )

#include "logmacro.h"

#define LOGVFD(...)     LOGMASKED(LOG_VFD,     __VA_ARGS__)

serial_vfd_device::serial_vfd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SERIAL_VFD, tag, owner, clock)
	, m_vfd(*this, "vfd%u", 0U)
{}

void serial_vfd_device::write_clock(bool state){
	if(m_reset){
		m_clock = state;
		if(m_clock != m_lastClock){
			if(!m_clock){
				m_cmd = (m_cmd << 1) | m_data;
				m_bitCount++;
				//LOGVFD("vfd clock in: %02x, %x, %d\n", m_cmd, m_data, m_bitCount);
				if(m_bitCount >= 8){
					m_bitCount = 0;
					run_command();
					m_cmd = 0x00;
				}
			}
			m_lastClock = m_clock;
		}
	}
}

void serial_vfd_device::device_start(){
	m_vfd.resolve();
}

void serial_vfd_device::write_data(bool state){
	if(m_reset) m_data = state;
}

void serial_vfd_device::write_reset(bool state){
	m_reset = state;
	if(!m_reset){
		LOGVFD("vfd reset\n");
		m_bitCount = 0;
		m_digits = 0;
		m_cursor = 0;
		m_bright = 0x00;
		m_cmd = 0x00;
		// Reset doesn't actually seem to clear the display.
		//for(int i = 0; i < 16; i++){
		//  m_vfd[i] = 0x0000;
		//}
	}
}

void serial_vfd_device::run_command(){
	uint8_t temp = 0x00;
	if(m_reset && m_cmd & 0x80){
		// Command
		switch(m_cmd & 0xe0){
			case 0xa0:
				temp = (m_cmd & 0x0f);
				if(temp == 0x0f) m_cursor = 0;
				else m_cursor = temp;
				LOGVFD("vfd cursor: %d\n", m_cursor);
				break;
			case 0xc0:
				m_digits = (m_cmd & 0x0f);
				LOGVFD("vfd digits: %d\n", m_digits);
				break;
			case 0xe0:
				m_bright = (m_cmd & 0x1f);
				LOGVFD("vfd brightness: %d\n", m_bright);
				break;
		}
	} else {
		// "ASCII"
		LOGVFD("vfd_text: %02x, %s, %d\n", (m_cmd & 0x3f), m_cmd, m_cursor);
		if(m_cmd != 0x2c && m_cmd != 0x2e){
			m_buff[m_cursor] = vfd_charMap[m_cmd & 0x3f];
			m_vfd[m_cursor] = vfd_charMap[m_cmd & 0x3f];
			m_cursor++;
		} else {
			// , or . so need to update last char instead
			m_vfd[m_cursor-1] = (m_buff[m_cursor-1] | vfd_charMap[m_cmd & 0x3f]);
		}
		if(!m_digits){
			if(m_cursor >= 16) m_cursor = 0;
		} else {
			if(m_cursor >= m_digits) m_cursor = 0;
		}
	}
}

DEFINE_DEVICE_TYPE(SERIAL_VFD, serial_vfd_device, "serial_vfd", "Serial VFD Device")

// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "sh_adc.h"
#include "sh7042.h"
#include "sh_intc.h"

// Verbosity level
// 0 = no messages
// 1 = everything
static constexpr int V = 0;

DEFINE_DEVICE_TYPE(SH_ADC_MS, sh_adc_ms_device, "sh_adc_ms", "SH2/704x ADC (medium speed)")
DEFINE_DEVICE_TYPE(SH_ADC_HS, sh_adc_hs_device, "sh_adc_hs", "SH2/704x ADC (high speed)")

sh_adc_device::sh_adc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_intc(*this, finder_base::DUMMY_TAG),
	m_intc_vector(0), m_adcsr(0), m_adcr(0), m_trigger(0), m_start_mode(0), m_start_channel(0),
	m_end_channel(0), m_start_count(0), m_mode(0), m_channel(0), m_count(0), m_analog_powered(false), m_adtrg(false), m_next_event(0)
{
}

sh_adc_ms_device::sh_adc_ms_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	sh_adc_device(mconfig, SH_ADC_MS, tag, owner, clock)
{
	m_is_hs = false;
	m_port_base = 0;
	m_port_mask = 3;
	m_port_shift = 6;
	m_analog_powered = true;
}

sh_adc_hs_device::sh_adc_hs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	sh_adc_device(mconfig, SH_ADC_HS, tag, owner, clock)
{
	m_is_hs = true;
	m_port_base = 0;
	m_port_mask = 7;
	m_port_shift = 0;
	m_analog_powered = false;
}


u16 sh_adc_device::addr_r(offs_t offset)
{
	return m_addr[offset] << m_port_shift;
}

u8 sh_adc_device::adcsr_r()
{
	if(V>=1) logerror("adcsr_r %02x\n", m_adcsr);
	return m_adcsr;
}

u8 sh_adc_device::adcr_r()
{
	if(V>=1) logerror("adcr_r %02x\n", m_adcr);
	return m_adcr;
}

void sh_adc_device::adcsr_w(u8 data)
{
	if(V>=1) logerror("adcsr_w %02x\n", data);
	u8 prev = m_adcsr;
	m_adcsr = (data & (0x70 | m_port_mask)) | (m_adcsr & data & F_ADF);
	mode_update();
	if((prev & F_ADF) && !(m_adcsr & F_ADF)) {
		if(m_mode & HALTED) {
			m_mode &= ~HALTED;
			if(!(m_adcsr & F_ADST)) {
				sampling();
				conversion_wait(false, false);
			} else
				done();
		}
	}

	if(!(prev & F_ADST) && (m_adcsr & F_ADST))
		start_conversion();
}

void sh_adc_device::adcr_w(u8 data)
{
	if(V>=1) logerror("adcr_w %02x\n", data);
	m_adcr = data;
	mode_update();
}

void sh_adc_device::adtrg_w(int state)
{
	if(state != m_adtrg) {
		m_adtrg = state;
		if(!m_adtrg && (m_trigger & T_EXT) && !(m_adcsr & F_ADST)) {
			m_adcsr |= F_ADST;
			start_conversion();
		}
	}
}

void sh_adc_device::set_suspend(bool suspend)
{
}

void sh_adc_device::device_start()
{
	save_item(NAME(m_addr));
	save_item(NAME(m_buf));
	save_item(NAME(m_adcsr));
	save_item(NAME(m_adcr));
	save_item(NAME(m_trigger));
	save_item(NAME(m_start_mode));
	save_item(NAME(m_start_channel));
	save_item(NAME(m_end_channel));
	save_item(NAME(m_start_count));
	save_item(NAME(m_suspend_on_interrupt));
	save_item(NAME(m_mode));
	save_item(NAME(m_channel));
	save_item(NAME(m_count));
	save_item(NAME(m_analog_powered));
	save_item(NAME(m_next_event));
	save_item(NAME(m_adtrg));
}

void sh_adc_device::device_reset()
{
	memset(m_addr, 0, sizeof(m_addr));
	memset(m_buf, 0, sizeof(m_buf));
	m_adcsr = m_adcr = 0;
	m_trigger = T_SOFT;
	m_start_mode = IDLE;
	m_start_channel = m_end_channel = 0;
	m_start_count = 1;
	m_mode = IDLE;
	m_channel = 0;
	m_count = 0;
	m_next_event = 0;
	mode_update();
	m_adtrg = true;
}

void sh_adc_device::done()
{
	m_mode = IDLE;
	m_adcsr &= ~F_ADST;
	if(m_analog_power_control)
		m_analog_powered = false;
}

u64 sh_adc_device::internal_update(u64 current_time)
{
	if(m_next_event && m_next_event <= current_time) {
		m_next_event = 0;
		timeout(current_time);
	}
	return m_next_event;
}

void sh_adc_device::conversion_wait(bool first, bool poweron, u64 current_time)
{
	if(current_time)
		m_next_event = current_time + conversion_time(first, poweron);
	else {
		m_next_event = m_cpu->total_cycles() + conversion_time(first, poweron);
		m_cpu->internal_update();
	}
}

void sh_adc_device::buffer_value(int port, int buffer)
{
	m_buf[buffer] = m_cpu->do_read_adc(port + m_port_base);
}

void sh_adc_device::commit_value(int reg, int buffer)
{
	if(V>=1) logerror("adc commit %d -> %d:%03x\n", buffer, reg, m_buf[buffer]);
	m_addr[reg] = m_buf[buffer];
}

void sh_adc_device::sampling()
{
	if(m_mode & COUNTED)
		m_channel = get_channel_index(m_start_count - m_count);
	if(m_mode & DUAL) {
		buffer_value(m_channel, 0);
		buffer_value(m_channel+1, 1);
	} else
		buffer_value(m_channel);
}

void sh_adc_device::start_conversion()
{
	m_mode = m_start_mode;
	m_channel = m_start_channel;
	m_count = m_start_count;
	sampling();
	conversion_wait(true, !m_analog_powered);
	m_analog_powered = true;
}

void sh_adc_device::timeout(u64 current_time)
{
	if(m_mode & BUFFER) {
		do_buffering((m_mode & DUAL) && (m_channel & 1));
		if((m_mode & DUAL) && !(m_channel & 1)) {
			m_channel++;
			conversion_wait(false, false, current_time);
			return;
		}
	} else {
		if(m_mode & DUAL) {
			if(m_channel & 1)
				commit_value(m_channel, 1);
			else {
				commit_value(m_channel, 0);
				m_channel++;
				conversion_wait(false, false, current_time);
				return;
			}
		} else
			commit_value(m_channel);
	}

	if(m_mode & ROTATE) {
		if(m_channel != m_end_channel) {
			m_channel++;
			sampling();
			conversion_wait(false, false, current_time);
			return;
		}
		m_channel = m_start_channel;
	}

	if(m_mode & COUNTED) {
		m_count--;
		if(m_count) {
			sampling();
			conversion_wait(false, false, current_time);
			return;
		}
	}

	m_adcsr |= F_ADF;
	if(m_adcsr & F_ADIE)
		m_intc->internal_interrupt(m_intc_vector);

	if(m_mode & REPEAT) {
		if(m_suspend_on_interrupt && (m_adcsr & F_ADIE)) {
			m_mode |= HALTED;
			return;
		}
		m_channel = m_start_channel;
		m_count = m_start_count;
		sampling();
		conversion_wait(false, false, current_time);
		return;
	}

	done();
}

int sh_adc_device::conversion_time(bool first, bool poweron)
{
	int tm = m_adcsr & 0x10 ? 44 : 24;
	if(first)
		tm += m_adcsr & 0x10 ? 20 : 10;
	if(poweron)
		tm += 200;
	return tm;
}

void sh_adc_device::mode_update()
{
	if(m_is_hs) {
		m_trigger = 1 << ((m_adcr >> 4) & 3);
		m_analog_power_control = !(m_adcr & 0x40);

		m_mode = ACTIVE | (m_adcr & 0x08 ? REPEAT : 0);

		if(m_adcr & 0x03) {
			m_mode |= BUFFER;

		}

		if(m_adcsr & 0x08) {
			m_mode |= ROTATE;
			m_start_channel = 0;
			if(m_adcr & 0x04) {
				m_mode |= DUAL;
				m_end_channel = (m_adcsr & 6)+1;
			} else
				m_end_channel = m_adcsr & 7;
		} else
			m_start_channel = m_end_channel = m_adcsr & 7;

	} else {
		m_trigger = T_SOFT;

		m_mode = ACTIVE;

		if(m_adcsr & 0x08) {
			m_mode |= ROTATE;
			m_start_channel = 0;
			m_end_channel = m_adcsr & 3;
		} else
			m_start_channel = m_end_channel = m_adcsr & 3;
		if(m_start_channel == 2)
			machine().debug_break();
	}
}

void sh_adc_device::do_buffering(int buffer)
{
	if((m_mode & COUNTED) && m_channel >= 2) {
		commit_value(m_channel, buffer);
		return;
	}
	switch(m_adcsr & 3) {
	case 0:
		commit_value(m_channel, buffer);
		break;
	case 1:
		m_addr[1] = m_addr[0];
		commit_value(0, buffer);
		break;
	case 2:
		m_addr[2+buffer] = m_addr[buffer];
		commit_value(buffer, buffer);
		break;
	case 3:
		m_addr[3] = m_addr[2];
		m_addr[2] = m_addr[1];
		m_addr[1] = m_addr[0];
		commit_value(0, buffer);
		break;
	}
}

int sh_adc_device::get_channel_index(int count)
{
	abort();
}

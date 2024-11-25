// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h8_adc.h"

// Verbosity level
// 0 = no messages
// 1 = everything
static constexpr int V = 0;

DEFINE_DEVICE_TYPE(H8_ADC_3337, h8_adc_3337_device, "h8_adc_3337", "H8/3337 ADC")
DEFINE_DEVICE_TYPE(H8_ADC_3006, h8_adc_3006_device, "h8_adc_3006", "H8/3006 ADC")
DEFINE_DEVICE_TYPE(H8_ADC_2245, h8_adc_2245_device, "h8_adc_2245", "H8/2245 ADC")
DEFINE_DEVICE_TYPE(H8_ADC_2319, h8_adc_2319_device, "h8_adc_2319", "H8/2319 ADC")
DEFINE_DEVICE_TYPE(H8_ADC_2357, h8_adc_2357_device, "h8_adc_2357", "H8/2357 ADC")
DEFINE_DEVICE_TYPE(H8_ADC_2655, h8_adc_2655_device, "h8_adc_2655", "H8/2655 ADC")

h8_adc_device::h8_adc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_intc(*this, finder_base::DUMMY_TAG),
	m_intc_vector(0), m_adcsr(0), m_adcr(0), m_register_mask(0), m_trigger(0), m_start_mode(0), m_start_channel(0),
	m_end_channel(0), m_start_count(0), m_mode(0), m_channel(0), m_count(0), m_analog_powered(false), m_adtrg(false), m_next_event(0)
{
	m_suspend_on_interrupt = false;
	m_analog_power_control = false;
}

u8 h8_adc_device::addr8_r(offs_t offset)
{
	if(V>=1) logerror("addr8_r %d %03x\n", offset, m_addr[offset >> 1]);
	return offset & 1 ? m_addr[offset >> 1] << 6 : m_addr[offset >> 1] >> 2;
}

u16 h8_adc_device::addr16_r(offs_t offset)
{
	if(V>=1) logerror("addr16_r %d %03x\n", offset, m_addr[offset]);
	return m_addr[offset];
}

u8 h8_adc_device::adcsr_r()
{
	if(V>=1) logerror("adcsr_r %02x\n", m_adcsr);
	return m_adcsr;
}

u8 h8_adc_device::adcr_r()
{
	if(V>=1) logerror("adcr_r %02x\n", m_adcr);
	return m_adcr;
}

void h8_adc_device::adcsr_w(u8 data)
{
	if(V>=1) logerror("adcsr_w %02x\n", data);
	u8 prev = m_adcsr;
	m_adcsr = (data & 0x7f) | (m_adcsr & data & F_ADF);
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

void h8_adc_device::adcr_w(u8 data)
{
	if(V>=1) logerror("adcr_w %02x\n", data);
	m_adcr = data;
	mode_update();
}

void h8_adc_device::adtrg_w(int state)
{
	if(state != m_adtrg) {
		m_adtrg = state;
		if(!m_adtrg && (m_trigger & T_EXT) && !(m_adcsr & F_ADST)) {
			m_adcsr |= F_ADST;
			start_conversion();
		}
	}
}

void h8_adc_device::set_suspend(bool suspend)
{
}

void h8_adc_device::device_start()
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
	save_item(NAME(m_analog_power_control));
	save_item(NAME(m_mode));
	save_item(NAME(m_channel));
	save_item(NAME(m_count));
	save_item(NAME(m_analog_powered));
	save_item(NAME(m_adtrg));
	save_item(NAME(m_next_event));
}

void h8_adc_device::device_reset()
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
	m_analog_powered = !m_analog_power_control;
	m_adtrg = true;
}

void h8_adc_device::done()
{
	m_mode = IDLE;
	m_adcsr &= ~F_ADST;
	if(m_analog_power_control)
		m_analog_powered = false;
}

u64 h8_adc_device::internal_update(u64 current_time)
{
	if(m_next_event && m_next_event <= current_time) {
		m_next_event = 0;
		timeout(current_time);
	}
	return m_next_event;
}

void h8_adc_device::notify_standby(int state)
{
	if(!state && m_next_event)
		m_next_event += m_cpu->total_cycles() - m_cpu->standby_time();
}

void h8_adc_device::conversion_wait(bool first, bool poweron, u64 current_time)
{
	if(current_time)
		m_next_event = current_time + conversion_time(first, poweron);
	else {
		m_next_event = m_cpu->total_cycles() + conversion_time(first, poweron);
		m_cpu->internal_update();
	}
}

void h8_adc_device::buffer_value(int port, int buffer)
{
	m_buf[buffer] = m_cpu->do_read_adc(port);
	if(V>=1) logerror("adc buffer %d -> %d:%03x\n", port, buffer, m_buf[buffer]);
}

void h8_adc_device::commit_value(int reg, int buffer)
{
	reg &= m_register_mask;
	if(V>=1) logerror("adc commit %d -> %d:%03x\n", buffer, reg, m_buf[buffer]);
	m_addr[reg] = m_buf[buffer];
}

void h8_adc_device::sampling()
{
	if(m_mode & COUNTED)
		m_channel = get_channel_index(m_start_count - m_count);
	if(m_mode & DUAL) {
		buffer_value(m_channel, 0);
		buffer_value(m_channel+1, 1);
	} else
		buffer_value(m_channel);
}

void h8_adc_device::start_conversion()
{
	m_mode = m_start_mode;
	m_channel = m_start_channel;
	m_count = m_start_count;
	sampling();
	conversion_wait(true, !m_analog_powered);
	m_analog_powered = true;
}

void h8_adc_device::timeout(u64 current_time)
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

void h8_adc_device::do_buffering(int buffer)
{
	throw emu_fatalerror("%s: Buffering requested but unimplemented for this device\n", tag());
}

int h8_adc_device::get_channel_index(int count)
{
	throw emu_fatalerror("%s: Indexing requested but unimplemented for this device\n", tag());
}


h8_adc_3337_device::h8_adc_3337_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8_adc_device(mconfig, H8_ADC_3337, tag, owner, clock)
{
	m_register_mask = 3;
}

int h8_adc_3337_device::conversion_time(bool first, bool poweron)
{
	int tm;
	if(first)
		tm = m_adcsr & 0x08 ? 134 : 266;
	else
		tm = m_adcsr & 0x08 ? 128 : 256;
	return tm;
}

void h8_adc_3337_device::mode_update()
{
	m_trigger = m_adcr & 0x80 ? T_EXT : T_SOFT;

	if(m_adcsr & 0x10) {
		m_start_mode = ACTIVE | ROTATE;
		m_start_channel = m_adcsr & 4;
		m_end_channel = m_adcsr & 7;
	} else {
		m_start_mode = ACTIVE;
		m_start_channel = m_end_channel = m_adcsr & 7;
	}
}


h8_adc_3006_device::h8_adc_3006_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8_adc_device(mconfig, H8_ADC_3006, tag, owner, clock)
{
	m_register_mask = 3;
}

int h8_adc_3006_device::conversion_time(bool first, bool poweron)
{
	int tm;
	if(first)
		tm = m_adcsr & 0x08 ? 70 : 134;
	else
		tm = m_adcsr & 0x08 ? 66 : 128;
	return tm;
}

void h8_adc_3006_device::mode_update()
{
	m_trigger = m_adcr & 0x80 ? T_EXT|T_TIMER : T_SOFT;

	if(m_adcsr & 0x10) {
		m_start_mode = ACTIVE | ROTATE;
		m_start_channel = m_adcsr & 4;
		m_end_channel = m_adcsr & 7;
	} else {
		m_start_mode = ACTIVE;
		m_start_channel = m_end_channel = m_adcsr & 7;
	}
}


h8_adc_2245_device::h8_adc_2245_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8_adc_device(mconfig, H8_ADC_2245, tag, owner, clock)
{
	m_register_mask = 3;
}

int h8_adc_2245_device::conversion_time(bool first, bool poweron)
{
	int tm;
	if(first)
		tm = m_adcsr & 0x08 ? 134 : 266;
	else
		tm = m_adcsr & 0x08 ? 128 : 256;
	return tm;
}

void h8_adc_2245_device::mode_update()
{
	m_trigger = 1 << ((m_adcr >> 6) & 3);

	if(m_adcsr & 0x10) {
		m_start_mode = ACTIVE | ROTATE;
		m_start_channel = 0;
		m_end_channel = m_adcsr & 3;
	} else {
		m_start_mode = ACTIVE;
		m_start_channel = m_end_channel = m_adcsr & 3;
	}
}


h8_adc_2319_device::h8_adc_2319_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8_adc_device(mconfig, H8_ADC_2319, tag, owner, clock)
{
	m_register_mask = 3;
}

int h8_adc_2319_device::conversion_time(bool first, bool poweron)
{
	int tm;
	if(first)
		if(m_adcr & 0x08)
			tm = m_adcsr & 0x08 ? 134 : 266;
		else
			tm = m_adcsr & 0x08 ? 68 : 530;
	else
		if(m_adcr & 0x08)
			tm = m_adcsr & 0x08 ? 128 : 256;
		else
			tm = m_adcsr & 0x08 ? 64 : 512;
	return tm;
}

void h8_adc_2319_device::mode_update()
{
	m_trigger = 1 << ((m_adcr >> 6) & 3);

	if(m_adcsr & 0x10) {
		m_start_mode = ACTIVE | ROTATE;
		m_start_channel = m_adcsr & 4;
		m_end_channel = m_adcsr & 7;
	} else {
		m_start_mode = ACTIVE;
		m_start_channel = m_end_channel = m_adcsr & 7;
	}
}


h8_adc_2357_device::h8_adc_2357_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8_adc_device(mconfig, H8_ADC_2357, tag, owner, clock)
{
	m_register_mask = 3;
}

int h8_adc_2357_device::conversion_time(bool first, bool poweron)
{
	int tm;
	if(first)
		tm = m_adcsr & 0x08 ? 134 : 266;
	else
		tm = m_adcsr & 0x08 ? 128 : 256;
	return tm;
}

void h8_adc_2357_device::mode_update()
{
	m_trigger = 1 << ((m_adcr >> 6) & 3);

	if(m_adcsr & 0x10) {
		m_start_mode = ACTIVE | ROTATE;
		m_start_channel = m_adcsr & 4;
		m_end_channel = m_adcsr & 7;
	} else {
		m_start_mode = ACTIVE;
		m_start_channel = m_end_channel = m_adcsr & 7;
	}
}


h8_adc_2655_device::h8_adc_2655_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8_adc_device(mconfig, H8_ADC_2655, tag, owner, clock)
{
	m_suspend_on_interrupt = true;
	m_register_mask = 7;
}

int h8_adc_2655_device::conversion_time(bool first, bool poweron)
{
	int tm = m_adcsr & 0x10 ? 44 : 24;
	if(first)
		tm += m_adcsr & 0x10 ? 20 : 10;
	if(poweron)
		tm += 200;
	return tm;
}

void h8_adc_2655_device::mode_update()
{
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
}

void h8_adc_2655_device::do_buffering(int buffer)
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

int h8_adc_2655_device::get_channel_index(int count)
{
	abort();
}

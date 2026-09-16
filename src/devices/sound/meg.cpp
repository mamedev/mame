// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha MEG - Multiple effects generator
//
// Audio dsp dedicated to effects generation

#include "emu.h"
#include "meg.h"

DEFINE_DEVICE_TYPE(MEG, meg_device, "meg", "Multiple Effects Generator (HD62098 / XM309A00)")

meg_device::meg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MEG, tag, owner, clock)
{
}

void meg_device::device_start()
{
}

void meg_device::device_reset()
{
}

//   vl70:
// 6d1e: write 1, r0l
// 6d26: write 2, r0l
// 6d2e: read 2
// 6d36: write 3, r0l
// 6d3e: write reg 4:r0h, r0l
// 6d52: write reg 5:r0h, r0l-1
// 6d68: write 7, r0l
// 6d70: write reg 8:r0h, r0l
// 6d84: write reg 9:r0h, r0l
// 6dac: write a, r0l
// 6db4: write reg cd:r1l, r0
// 6dd4: write reg e:r0h, r0l
// 6dee: write reg f:r0h, r0l
// 6e08: read 10,11
// 6e1c: write reg 1213:r1l, r0
// 6e3c: write reg 14:r0h, r0l
// 6e50: write 15, r0l
// 6e58: write reg 16:r0h, r0l
// 6e6c: write reg 17:r0h, r0l
// 6e80: write reg 18:e0h, e0l

void meg_device::map(address_map &map)
{
	map(0x00, 0x00).w(FUNC(meg_device::select_w));
	map(0x01, 0x01).w(FUNC(meg_device::s1_w));
	map(0x02, 0x02).rw(FUNC(meg_device::s2_r), FUNC(meg_device::s2_w));
	map(0x03, 0x03).w(FUNC(meg_device::s3_w));
	map(0x04, 0x04).w(FUNC(meg_device::s4_w));
	map(0x05, 0x05).w(FUNC(meg_device::s5_w));
	map(0x07, 0x07).w(FUNC(meg_device::s7_w));
	map(0x08, 0x08).w(FUNC(meg_device::s8_w));
	map(0x09, 0x09).w(FUNC(meg_device::s9_w));
	map(0x0a, 0x0a).w(FUNC(meg_device::sa_w));
	map(0x0c, 0x0c).w(FUNC(meg_device::consth_w));
	map(0x0d, 0x0d).w(FUNC(meg_device::constl_w));
	map(0x0e, 0x0e).w(FUNC(meg_device::se_w));
	map(0x0f, 0x0f).w(FUNC(meg_device::sf_w));
	map(0x10, 0x10).r(FUNC(meg_device::s10_r));
	map(0x11, 0x11).r(FUNC(meg_device::s11_r));
	map(0x12, 0x12).w(FUNC(meg_device::offseth_w));
	map(0x13, 0x13).w(FUNC(meg_device::offsetl_w));
	map(0x14, 0x14).w(FUNC(meg_device::s14_w));
	map(0x15, 0x15).w(FUNC(meg_device::s15_w));
	map(0x16, 0x16).w(FUNC(meg_device::s16_w));
	map(0x17, 0x17).w(FUNC(meg_device::s17_w));
	map(0x18, 0x18).w(FUNC(meg_device::s18_w));
}

u8 meg_device::s2_r()
{
	logerror("read r2 %s\n", machine().describe_context());
	return 0x00;
}

void meg_device::select_w(u8 data)
{
	m_reg = data;
}

void meg_device::s1_w(u8 data)
{
	logerror("r1 %02x %s\n", data, machine().describe_context());
}

void meg_device::s2_w(u8 data)
{
	logerror("r2 %02x %s\n", data, machine().describe_context());
}

void meg_device::s3_w(u8 data)
{
	logerror("r3 %02x %s\n", data, machine().describe_context());
}

void meg_device::s4_w(u8 data)
{
	if(m_r4[m_reg] != data) {
		m_r4[m_reg] = data;
		logerror("r4[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}

void meg_device::s5_w(u8 data)
{
	if(m_r5[m_reg] != data) {
		m_r5[m_reg] = data;
		logerror("r5[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}

void meg_device::s7_w(u8 data)
{
	logerror("r7 %02x %s\n", data, machine().describe_context());
}

void meg_device::s8_w(u8 data)
{
	if(m_r8[m_reg] != data) {
		m_r8[m_reg] = data;
		logerror("r8[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}


void meg_device::s9_w(u8 data)
{
	if(m_r9[m_reg] != data) {
		m_r9[m_reg] = data;
		logerror("r9[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}

void meg_device::sa_w(u8 data)
{
	logerror("ra %02x %s\n", data, machine().describe_context());
}

void meg_device::consth_w(u8 data)
{
	m_const[m_reg] = (m_const[m_reg] & 0x00ff) | (data << 8);
}


void meg_device::constl_w(u8 data)
{
	m_const[m_reg] = (m_const[m_reg] & 0xff00) | data;
}

void meg_device::se_w(u8 data)
{
	if(m_re[m_reg] != data) {
		m_re[m_reg] = data;
		logerror("re[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}


void meg_device::sf_w(u8 data)
{
	if(m_rf[m_reg] != data) {
		m_rf[m_reg] = data;
		logerror("rf[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}

u8 meg_device::s10_r()
{
	logerror("read r10 %s\n", machine().describe_context());
	return 0x00;
}

u8 meg_device::s11_r()
{
	logerror("read r11 %s\n", machine().describe_context());
	return 0x00;
}

void meg_device::offseth_w(u8 data)
{
	m_offset[m_reg] = (m_offset[m_reg] & 0x00ff) | (data << 8);
}

void meg_device::offsetl_w(u8 data)
{
	m_offset[m_reg] = (m_offset[m_reg] & 0xff00) | data;
}

void meg_device::s14_w(u8 data)
{
	if(m_r14[m_reg] != data) {
		m_r14[m_reg] = data;
		logerror("r14[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}

void meg_device::s15_w(u8 data)
{
	logerror("r15 %02x %s\n", data, machine().describe_context());
}

void meg_device::s16_w(u8 data)
{
	if(m_r16[m_reg] != data) {
		m_r16[m_reg] = data;
		logerror("r16[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}

void meg_device::s17_w(u8 data)
{
	if(m_r17[m_reg] != data) {
		m_r17[m_reg] = data;
		logerror("r17[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}

void meg_device::s18_w(u8 data)
{
	if(m_r18[m_reg] != data) {
		m_r18[m_reg] = data;
		logerror("r18[%02x] = %02x %s\n", m_reg, data, machine().describe_context());
	}
}
